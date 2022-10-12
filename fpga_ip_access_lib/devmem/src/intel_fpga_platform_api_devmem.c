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
#include "intel_fpga_api_devmem.h"
#include "intel_fpga_platform_devmem.h"
#include "intel_fpga_platform_api_devmem.h"
#include "intel_fpga_api_cmn_dfl.h"

static char *s_devmem_drv_path = "/dev/mem";
static size_t s_devmem_addr_span = 0;
static size_t s_dfl_entry_addr = 0;
static int s_devmem_single_component_mode = 1;
static size_t s_devmem_start_addr = 0;

static int s_devmem_drv_handle = -1;
static void *s_devmem_mmap_ptr = NULL;
static const uint64_t MASK_4K_ADDR = ~(4*1024-1);

static void devmem_parse_args(unsigned int argc, const char *argv[]);
static long devmem_parse_integer_arg(const char *name);
static bool devmem_validate_args();
static void devmem_print_configuration();
static bool devmem_open_driver();
static bool devmem_map_mmio();
static bool devmem_scan_interfaces();
static bool devmem_create_unit_test_sw_model();

bool fpga_platform_init(unsigned int argc, const char *argv[])
{
    bool is_args_valid;
    bool ret = false;

    devmem_parse_args(argc, argv);
    is_args_valid = devmem_validate_args();

    if (is_args_valid)
    {
        devmem_print_configuration();
#ifndef DEVMEM_UNIT_TEST_SW_MODEL_MODE
        if (devmem_open_driver() == false)
            goto err_open;

        if (devmem_map_mmio() == false)
            goto err_map;

        if (devmem_scan_interfaces() == false)
            goto err_scan;
#else
        if (devmem_create_unit_test_sw_model() == false)
            goto err_open;
#endif
        ret = true;
    }
    else
    {
        goto err_open;
    }

    return ret;

#ifndef DEVMEM_UNIT_TEST_SW_MODEL_MODE
err_scan:
    munmap(s_devmem_mmap_ptr, s_devmem_addr_span);

err_map:
    close(s_devmem_drv_handle);
#endif

err_open:
    return ret;
}

void fpga_platform_cleanup()
{
#ifndef DEVMEM_UNIT_TEST_SW_MODEL_MODE
    munmap(s_devmem_mmap_ptr, s_devmem_addr_span);
#endif

    if (s_devmem_drv_handle >= 0)
    {
        close(s_devmem_drv_handle);
    }

    // Re-initialize local variables.
    s_devmem_start_addr = 0;
    s_devmem_addr_span = 0;
    s_devmem_single_component_mode = 1;

    s_devmem_drv_handle = -1;
    s_devmem_mmap_ptr = NULL;

    if (common_fpga_interface_info_vec_size() > 0)
    {
#ifdef DEVMEM_UNIT_TEST_SW_MODEL_MODE
        free(common_fpga_interface_info_vec_at(0)->base_address);
#endif
        common_fpga_interface_info_vec_resize(0);
    }

    errno = -1; //  -1 is a valid return on success.  Some function, such as strtol(), doesn't set errno upon successful return.

    fpga_msg_printf(FPGA_MSG_PRINTF_DEBUG, "Completed fpga_platform_cleanup");
}

void devmem_parse_args(unsigned int argc, const char *argv[])
{
    static struct option long_options[] =
        {
            {"devmem-driver-path", required_argument, 0, 'p'},
            {"start-address", required_argument, 0, 'a'},
            {"address-span", required_argument, 0, 's'},
            {"dfl-entry-address", required_argument, 0, 'w'},
            {"show-dbg-msg", no_argument, &g_common_show_dbg_msg, 'd'},
            {"single-component-mode", no_argument, &s_devmem_single_component_mode, 'c'},
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
            s_devmem_drv_path = optarg;
            break;

        case 's':
            s_devmem_addr_span = devmem_parse_integer_arg("Address span");
            break;

        case 'a':
            s_devmem_start_addr = devmem_parse_integer_arg("Start address");
            break;

        case 'w':
            s_dfl_entry_addr = devmem_parse_integer_arg("DFL entry address");
            s_devmem_single_component_mode = false;
            break;
        }
    }
}

long devmem_parse_integer_arg(const char *name)
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


bool devmem_validate_args()
{
    bool ret = s_devmem_addr_span > 0 &&
               s_devmem_start_addr > 0;
    if (!ret)
    {
        if (s_devmem_addr_span == 0)
        {
            fpga_msg_printf(FPGA_MSG_PRINTF_ERROR, "No valid address span value is provided using the argument, --address-span.");
        }
        if (s_devmem_start_addr == 0)
        {
            fpga_msg_printf(FPGA_MSG_PRINTF_ERROR, "Start address is not provided using the argument, --start-address.");
        }
    }

    if ( !s_devmem_single_component_mode && 
         (s_dfl_entry_addr < s_devmem_start_addr || s_dfl_entry_addr >= (s_devmem_start_addr + s_devmem_addr_span)))
    {
        fpga_msg_printf(FPGA_MSG_PRINTF_ERROR, "DFL entry address specified is not withing the range based on the arguments --start-address and --address-span.");
        ret = false;
    }

    return ret;
}

void devmem_print_configuration()
{
    fpga_msg_printf(FPGA_MSG_PRINTF_INFO, "Devmem Platform Configuration:");
    fpga_msg_printf(FPGA_MSG_PRINTF_INFO, "   Driver Path: %s", s_devmem_drv_path);
    fpga_msg_printf(FPGA_MSG_PRINTF_INFO, "   Address Span: %ld", s_devmem_addr_span);
    fpga_msg_printf(FPGA_MSG_PRINTF_INFO, "   Start Address: 0x%lX", s_devmem_start_addr);
    if(s_devmem_single_component_mode)
    {
        fpga_msg_printf(FPGA_MSG_PRINTF_INFO, "   Single Component Operation Model: Yes");
    }
    else
    {
        fpga_msg_printf(FPGA_MSG_PRINTF_INFO, "   DFL Operation Model: Yes");
        fpga_msg_printf(FPGA_MSG_PRINTF_INFO, "   DFL Entry Address: 0x%lX", s_dfl_entry_addr);
    }
}

bool devmem_open_driver()
{
    bool ret = true;

    s_devmem_drv_handle = open(s_devmem_drv_path, O_RDWR | O_SYNC);
    if (s_devmem_drv_handle == -1)
    {

#ifdef _BSD_SOURCE
        fpga_msg_printf(FPGA_MSG_PRINTF_ERROR, "Failed to open %s. (Error code %d: %s)", s_devmem_drv_path, sys_nerr, sys_errlist[sys_nerr]);
#else
        fpga_msg_printf(FPGA_MSG_PRINTF_ERROR, "Failed to open %s. (Error code %d)", s_devmem_drv_path, errno);
#endif

        ret = false;
    }

    return ret;
}

bool devmem_map_mmio()
{
    bool ret = true;

    s_devmem_mmap_ptr = mmap(0, s_devmem_addr_span, PROT_READ | PROT_WRITE, MAP_SHARED, s_devmem_drv_handle, (s_devmem_start_addr & MASK_4K_ADDR));
    if (s_devmem_mmap_ptr == MAP_FAILED)
    {
#ifdef _BSD_SOURCE
        fpga_msg_printf(FPGA_MSG_PRINTF_ERROR, "Failed to map the mmio inteface to the devmem driver provided.  (Error code %d: %s)", sys_nerr, sys_errlist[sys_nerr]);
#else
        fpga_msg_printf(FPGA_MSG_PRINTF_ERROR, "Failed to map the mmio inteface to the devmem driver provided.  (Error code %d)", errno);
#ifdef INTEL_FPGA_MSG_PRINTF_ENABLE_DEBUG
        perror("MMAP error");
#endif
#endif
        ret = false;
    }

    return ret;
}

uint64_t devmem_dfl_base_addr_decoder(uint64_t base_addr)
{
    uint64_t ret = base_addr - (int64_t)s_devmem_mmap_ptr - (s_devmem_start_addr & ~MASK_4K_ADDR) + s_devmem_start_addr;

    return ret;
} 

bool devmem_scan_interfaces()
{
    bool ret = true;

    if (s_devmem_single_component_mode)
    {
        common_fpga_interface_info_vec_resize(1);

        common_fpga_interface_info_vec_at(0)->base_address = (void *)s_devmem_mmap_ptr + (s_devmem_start_addr & ~MASK_4K_ADDR);
        common_fpga_interface_info_vec_at(0)->is_mmio_opened = false;
        common_fpga_interface_info_vec_at(0)->is_interrupt_opened = false;
    }
    else
    {
        void *first_dfh_addr = (void *)((char *)s_devmem_mmap_ptr + (s_dfl_entry_addr - s_devmem_start_addr) + (s_devmem_start_addr & ~MASK_4K_ADDR));
#ifdef DFL_WALKER_DEBUG_MODE
        fpga_msg_printf(FPGA_MSG_PRINTF_DEBUG, "Walking through multi-components mode with fisrt DFL address: 0x%lX", (size_t)first_dfh_addr);
#endif
        common_dfl_scan_multi_interfaces(first_dfh_addr, devmem_dfl_base_addr_decoder);
    }

    return ret;
}

bool devmem_create_unit_test_sw_model()
{
    bool ret = true;

    common_fpga_interface_info_vec_resize(1);

    common_fpga_interface_info_vec_at(0)->base_address = malloc(s_devmem_addr_span);
    // Preset mem with all 1s
    memset(common_fpga_interface_info_vec_at(0)->base_address, 0xFF, s_devmem_addr_span);

    return ret;
}
