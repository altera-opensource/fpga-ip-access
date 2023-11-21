// Copyright(c) 2023, Intel Corporation
//
// Redistribution  and  use  in source  and  binary  forms,  with  or  without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of  source code  must retain the  above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name  of Intel Corporation  nor the names of its contributors
//   may be used to  endorse or promote  products derived  from this  software
//   without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED TO,  THE
// IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT  SHALL THE COPYRIGHT OWNER  OR CONTRIBUTORS BE
// LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR
// CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT LIMITED  TO,  PROCUREMENT  OF
// SUBSTITUTE GOODS OR SERVICES;  LOSS OF USE,  DATA, OR PROFITS;  OR BUSINESS
// INTERRUPTION)  HOWEVER CAUSED  AND ON ANY THEORY  OF LIABILITY,  WHETHER IN
// CONTRACT,  STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE  OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#include <errno.h>
#include <getopt.h>
#include <stddef.h>
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <pthread.h>
#include <poll.h>
#include <semaphore.h>

#include "intel_fpga_api_cmn_msg.h"
#include "intel_fpga_api_uio.h"
#include "intel_fpga_platform_uio.h"
#include "intel_fpga_platform_api_uio.h"
#include "intel_fpga_api_cmn_dfl.h"

sem_t g_intSem;

static char *s_uio_drv_path = "/dev/uio0";
static size_t s_uio_addr_span = 0;
static size_t s_dfl_entry_addr = 0;
static int s_uio_single_component_mode = 1;
static size_t s_uio_start_addr = 0;
static size_t s_uio_inThread_timeout = 0;

static int s_uio_drv_handle = -1;
static void *s_uio_mmap_ptr = NULL;
static pthread_t s_intThread_id = 0;
static pthread_rwlock_t s_intLock;
static int s_intFlags = 0;

static void uio_parse_args(unsigned int argc, const char *argv[]);
static long uio_parse_integer_arg(const char *name);
static void uio_update_based_on_sysfs();
static void uio_get_sysfs_map_path(char *path, int path_buf_size);
static uint64_t uio_get_sysfs_map_file_to_uint64(const char *path);
static bool uio_validate_args();
static void uio_print_configuration();
static bool uio_open_driver();
static bool uio_map_mmio();
static bool uio_scan_interfaces();
static bool uio_create_interrupt_thread();
static bool uio_create_unit_test_sw_model();

static void *uio_interrupt_thread();

void *uio_interrupt_thread()
{
    while (1)
    {
        int rc;

        rc = pthread_rwlock_rdlock(&s_intLock);
        if (rc != 0)
        {
            fpga_msg_printf(FPGA_MSG_PRINTF_ERROR, "InterruptThread failed to acquire OS lock to handle interrupt.");
            break;
        }
        uint16_t flags = s_intFlags;
        rc = pthread_rwlock_unlock(&s_intLock);
        if (rc != 0)
        {
            fpga_msg_printf(FPGA_MSG_PRINTF_ERROR, "InterruptThread failed to release OS lock to handle interrupt.");
            break;
        }

        if (!(flags & FPGA_PLATFORM_INT_THREAD_EXIT))
        {
            // Loop for uio with interrupt enabled
            // Current implementaion support 1 vector
            if (common_fpga_interface_info_vec_at(0)->interrupt_enable)
            {
                int fd = 0;

                fd = open(s_uio_drv_path, O_RDWR);
                if (fd < 0)
                {
                    fpga_msg_printf(FPGA_MSG_PRINTF_ERROR, "InterruptThread failed to open UIO device");
                    break;
                }

                struct pollfd fds = {
                    .fd = fd,
                    .events = POLLIN,
                };

                uint32_t info = 1;
                int ret = write(fd, &info, sizeof(info));
                if (ret < 0)
                {
                    fpga_msg_printf(FPGA_MSG_PRINTF_ERROR, "InterruptThread failed to re-Arm UIO interrupt");
                    close(fd);
                    break;
                }

                // Polling with timeout to check thread exit flag
                // s_uio_inThread_timeout init in uio_parse_args()
                ret = poll(&fds, 1, s_uio_inThread_timeout);

                if (ret > 0)
                {
                    if (common_fpga_interface_info_vec_at(0)->isr_callback != NULL)
                    {
                        common_fpga_interface_info_vec_at(0)->isr_callback(common_fpga_interface_info_vec_at(0)->isr_context);
                    }
                    else
                    {
                        fpga_msg_printf(FPGA_MSG_PRINTF_ERROR, "InterruptThread ISR is NULL ptr");
                        close(fd);
                        break;
                    }
                }

                if (fd > 0)
                {
                    close(fd);
                }
            }
            else
            {

                // Interrupt Thread blocked until sem_post
                rc = sem_wait(&g_intSem);
                if (rc != 0)
                {
                    fpga_msg_printf(FPGA_MSG_PRINTF_ERROR, "InterruptThread failed to acquire OS lock to handle interrupt.");
                    break;
                }
            }
        }
        else
        {
            // Interrupt Thread exit
            fpga_msg_printf(FPGA_MSG_PRINTF_DEBUG, "InterruptThread exit");
            pthread_exit(NULL);
        }
    }

    // Interrupt Thread exit for break condition
    fpga_msg_printf(FPGA_MSG_PRINTF_ERROR, "InterruptThread exit with error");
    pthread_exit(NULL);
}

bool fpga_platform_init(unsigned int argc, const char *argv[])
{
    bool is_args_valid;
    bool ret = false;

    uio_parse_args(argc, argv);
    uio_update_based_on_sysfs();
    is_args_valid = uio_validate_args();

    // Interrupt Timeout is configured to 100ms
    s_uio_inThread_timeout = 100;

    if (is_args_valid)
    {
        uio_print_configuration();
#ifndef UIO_UNIT_TEST_SW_MODEL_MODE
        if (uio_open_driver() == false)
            goto err_open;

        if (uio_map_mmio() == false)
            goto err_map;

        if (uio_scan_interfaces() == false)
            goto err_scan;
#else
        if (uio_create_unit_test_sw_model() == false)
            goto err_open;
#endif
        ret = true;
    }
    else
    {
        goto err_open;
    }

    if (s_uio_single_component_mode)
    {
        // Interrupt Thread creation and sync init
        if (uio_create_interrupt_thread() == false)
        {
            goto err_open;
        }
    }

    return ret;

#ifndef UIO_UNIT_TEST_SW_MODEL_MODE
err_scan:
    munmap(s_uio_mmap_ptr, s_uio_addr_span);

err_map:
    close(s_uio_drv_handle);
#endif

err_open:
    return ret;
}

void fpga_platform_cleanup()
{
    void *ret;
    int retVal = 0;
    int status = 0;

    if (s_intThread_id != 0)
    {
        status = pthread_rwlock_wrlock(&s_intLock);
        if (status == 0)
        {
            s_intFlags = s_intFlags | FPGA_PLATFORM_INT_THREAD_EXIT;
            status = pthread_rwlock_unlock(&s_intLock);

            status = status == 0 ? 0 : sem_getvalue(&g_intSem, &retVal);
            if (status == 0)
            {
                if (retVal <= 0)
                    status = sem_post(&g_intSem);
            }
        }

        if (status != 0)
        {
            fpga_throw_runtime_exception("fpga_platform_cleanup", __FILE__, __LINE__, "System error upon cleanup.");
        }
        if (pthread_join(s_intThread_id, &ret) != 0)
        {
            fpga_msg_printf(FPGA_MSG_PRINTF_ERROR, "Interrupt Thread join failed");
        }
        else
        {
            fpga_msg_printf(FPGA_MSG_PRINTF_DEBUG, "Interrupt Thread join successfully");
        }
    }

    if (s_uio_drv_handle >= 0)
    {
        close(s_uio_drv_handle);
    }

    // Re-initialize local variables.
    s_uio_drv_path = NULL;
    s_uio_start_addr = 0;
    s_uio_addr_span = 0;
    s_uio_single_component_mode = 0;

    s_uio_drv_handle = -1;
    s_uio_mmap_ptr = NULL;

    if (common_fpga_interface_info_vec_size() > 0)
    {
#ifdef UIO_UNIT_TEST_SW_MODEL_MODE
        free(common_fpga_interface_info_vec_at(0)->base_address);
#endif
        common_fpga_interface_info_vec_resize(0);
    }

    errno = -1; //  -1 is a valid return on success.  Some function, such as strtol(), doesn't set errno upon successful return.

    fpga_msg_printf(FPGA_MSG_PRINTF_DEBUG, "Completed fpga_platform_cleanup");
}

void uio_parse_args(unsigned int argc, const char *argv[])
{
    static struct option long_options[] =
        {
            {"uio-driver-path", required_argument, 0, 'p'},
            {"start-address", required_argument, 0, 'a'},
            {"address-span", required_argument, 0, 's'},
            {"dfl-entry-address", required_argument, 0, 'w'},
            {"show-dbg-msg", no_argument, &g_common_show_dbg_msg, 'd'},
            {"single-component-mode", no_argument, &s_uio_single_component_mode, 'c'},
            {0, 0, 0, 0}};

    int option_index = 0;
    int c;

    opterr = 0; // Suppress stderr output from getopt_long upon unrecognized options
    optind = 0; // Reset getopt_long position.

    while (1)
    {
        c = getopt_long(argc, (char *const *)argv, "p:a:w:s:dc", long_options, &option_index);

        if (c == -1)
        {
            break;
        }

        switch (c)
        {
        case 'p':
            s_uio_drv_path = optarg;
            break;

        case 's':
            s_uio_addr_span = uio_parse_integer_arg("Address span");
            break;

        case 'a':
            s_uio_start_addr = uio_parse_integer_arg("Start address");
            break;

        case 'w':
            s_dfl_entry_addr = uio_parse_integer_arg("DFL entry address");
            break;
        }
    }
}

long uio_parse_integer_arg(const char *name)
{
    long ret = 0;

    bool is_all_digit = true;
    char *p;
    typedef int (*DIGIT_TEST_FN)(int c);
    DIGIT_TEST_FN is_acceptabl_digit;
    if (optarg[0] == '0' && (optarg[1] == 'x' || optarg[1] == 'X'))
    {
        is_acceptabl_digit = isxdigit;
        optarg += 2; // trim the "0x" portion
    }
    else
    {
        is_acceptabl_digit = isdigit;
    }

    for (p = optarg; (*p) != '\0'; ++p)
    {
        if (!is_acceptabl_digit(*p))
        {
            is_all_digit = false;
            break;
        }
    }

    if (is_acceptabl_digit == isxdigit)
    {
        optarg -= 2; // restore the "0x" portion
    }

    if (is_all_digit)
    {
        if (sizeof(size_t) <= sizeof(long))
        {
            ret = (size_t)strtol(optarg, NULL, 0);
            if (errno == ERANGE)
            {
                ret = 0;
                fpga_msg_printf(FPGA_MSG_PRINTF_ERROR, "%s value is too big. %s is provided; maximum accepted is %ld", name, optarg, LONG_MAX);
            }
        }
        else
        {
            long span, span_c;
            span = strtol(optarg, NULL, 0);
            if (errno == ERANGE)
            {
                ret = 0;
                fpga_msg_printf(FPGA_MSG_PRINTF_ERROR, "%s value is too big. %s is provided; maximum accepted is %ld", name, optarg, LONG_MAX);
            }
            else
            {
                ret = (size_t)span;
                span_c = ret;
                if (span != span_c)
                {
                    fpga_msg_printf(FPGA_MSG_PRINTF_ERROR, "%s value is too big. %s is provided; maximum accepted is %ld", name, optarg, (size_t)-1);
                    ret = 0;
                }
            }
        }
    }
    else
    {
        fpga_msg_printf(FPGA_MSG_PRINTF_ERROR, "Invalid argument value type is provided. A integer value is expected. %s is provided.", optarg);
    }

    return ret;
}

void uio_update_based_on_sysfs()
{
#ifndef UIO_UNIT_TEST_SW_MODEL_MODE
    if (s_uio_addr_span == 0)
    {
        enum
        {
            UIO_MAP_PATH_SIZE = 1024
        };

        char map_path[UIO_MAP_PATH_SIZE + 1];

        uio_get_sysfs_map_path(map_path, UIO_MAP_PATH_SIZE);

        strncat(map_path, "size", UIO_MAP_PATH_SIZE);
        s_uio_addr_span = uio_get_sysfs_map_file_to_uint64(map_path);
    }
#endif
}

uint64_t uio_get_sysfs_map_file_to_uint64(const char *path)
{
    uint64_t ret = 0;

    FILE *fp;

    fp = fopen(path, "r");
    if (fp)
    {
        if (fscanf(fp, "%lx", &ret) != 1)
        {
            ret = 0;
        }
        fclose(fp);
    }

    return ret;
}
void uio_get_sysfs_map_path(char *path, int path_buf_size)
{
    uint32_t index = 0;
    char *p;
    char *endptr;

    path[0] = '\0';
    // The region index is encoded in the file name component.
    p = strrchr(s_uio_drv_path, '/');
    if (!p)
    {
        return;
    }

    // p + 4 because the string will look like:
    // 01234
    // /dev/uio3
    endptr = NULL;
    p += 4;
    index = strtoul(p, &endptr, 10);
    if (*endptr)
    {
        return;
    }

    // Example map path: /sys/class/uio/uio0/maps/map0
    // Only use map0!
    if (snprintf(path, path_buf_size, "/sys/class/uio/uio%d/maps/map0/", index) < 0)
    {
        path[0] = '\0';
        return;
    }
}

bool uio_validate_args()
{
    bool ret = s_uio_addr_span > 0 &&
               s_uio_drv_path != NULL;

    if (!ret)
    {
        if (s_uio_addr_span == 0)
        {
            fpga_msg_printf(FPGA_MSG_PRINTF_ERROR, "No valid address span value is provided using the argument, --address-span.");
        }
        if (s_uio_drv_path == NULL)
        {
            fpga_msg_printf(FPGA_MSG_PRINTF_ERROR, "UIO driver path is not provided using the argument, --uio-driver-path.");
        }
    }
    return ret;
}

void uio_print_configuration()
{
    fpga_msg_printf(FPGA_MSG_PRINTF_INFO, "UIO Platform Configuration:");
    fpga_msg_printf(FPGA_MSG_PRINTF_INFO, "   Driver Path: %s", s_uio_drv_path);
    fpga_msg_printf(FPGA_MSG_PRINTF_INFO, "   Address Span: %ld", s_uio_addr_span);
    fpga_msg_printf(FPGA_MSG_PRINTF_INFO, "   Start Address: 0x%lX", s_uio_start_addr);
    fpga_msg_printf( FPGA_MSG_PRINTF_INFO, "   Single Component Operation Model: %s", s_uio_single_component_mode ? "Yes" : "No" );
    if(!s_uio_single_component_mode)
    {
        fpga_msg_printf( FPGA_MSG_PRINTF_INFO, "   DFL Operation Model: %s", s_uio_single_component_mode ? "No" : "Yes");
        fpga_msg_printf(FPGA_MSG_PRINTF_INFO, "   DFL Entry Address: 0x%lX", s_dfl_entry_addr);
    }
}

bool uio_open_driver()
{
    bool ret = true;

    s_uio_drv_handle = open(s_uio_drv_path, O_RDWR);
    if (s_uio_drv_handle == -1)
    {

#ifdef _BSD_SOURCE
        fpga_msg_printf(FPGA_MSG_PRINTF_ERROR, "Failed to open %s. (Error code %d: %s)", s_uio_drv_path, sys_nerr, sys_errlist[sys_nerr]);
#else
        fpga_msg_printf(FPGA_MSG_PRINTF_ERROR, "Failed to open %s. (Error code %d)", s_uio_drv_path, errno);
#endif

        ret = false;
    }

    return ret;
}

bool uio_map_mmio()
{
    bool ret = true;

    s_uio_mmap_ptr = mmap(0, s_uio_addr_span, PROT_READ | PROT_WRITE, MAP_SHARED, s_uio_drv_handle, 0);
    if (s_uio_mmap_ptr == MAP_FAILED)
    {
#ifdef _BSD_SOURCE
        fpga_msg_printf(FPGA_MSG_PRINTF_ERROR, "Failed to map the mmio inteface to the UIO driver provided.  (Error code %d: %s)", sys_nerr, sys_errlist[sys_nerr]);
#else
        fpga_msg_printf(FPGA_MSG_PRINTF_ERROR, "Failed to map the mmio inteface to the UIO driver provided.  (Error code %d)", errno);
#ifdef INTEL_FPGA_MSG_PRINTF_ENABLE_DEBUG
        perror("MMAP error");
#endif
#endif
        ret = false;
    }

    return ret;
}

uint64_t uio_dfl_base_addr_decoder(uint64_t base_addr)
{
    uint64_t ret = base_addr - (int64_t)s_uio_mmap_ptr;

    return ret;
}

bool uio_scan_interfaces()
{
    bool ret = true;

    if (s_uio_single_component_mode)
    {
        common_fpga_interface_info_vec_resize(1);

        common_fpga_interface_info_vec_at(0)->base_address = (void *)((char *)s_uio_mmap_ptr + s_uio_start_addr);
        common_fpga_interface_info_vec_at(0)->is_mmio_opened = false;
        common_fpga_interface_info_vec_at(0)->is_interrupt_opened = false;
    }
    else
    {
        void *first_dfh_addr = (void *)((char *)s_uio_mmap_ptr + s_dfl_entry_addr);
#ifdef DFL_WALKER_DEBUG_MODE
        fpga_msg_printf(FPGA_MSG_PRINTF_DEBUG, "Walking through multi-components mode with fisrt DFL address: 0x%lX", (size_t)first_dfh_addr);
#endif
        common_dfl_scan_multi_interfaces(first_dfh_addr, uio_dfl_base_addr_decoder);
    }

    return ret;
}

bool uio_create_interrupt_thread()
{
    bool ret;
    int rc;

    rc = sem_init(&g_intSem, 0, 0);
    if (rc == 0)
    {
        rc = pthread_rwlock_init(&s_intLock, 0);
    }
    if (rc == 0)
    {
        rc = pthread_rwlock_wrlock(&s_intLock);
    }
    s_intFlags = 0;
    if (rc == 0)
    {
        rc = pthread_rwlock_unlock(&s_intLock);
    }
    if (rc == 0)
    {
        rc = pthread_create(&s_intThread_id, NULL, uio_interrupt_thread, NULL);
    }

    ret = rc == 0;

    return ret;
}

bool uio_create_unit_test_sw_model()
{
    bool ret = true;

    common_fpga_interface_info_vec_resize(1);

    common_fpga_interface_info_vec_at(0)->base_address = malloc(s_uio_addr_span);
    // Preset mem with all 1s
    memset(common_fpga_interface_info_vec_at(0)->base_address, 0xFF, s_uio_addr_span);

    return ret;
}
