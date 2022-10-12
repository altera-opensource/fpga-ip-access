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
#include <stddef.h>
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "intel_fpga_api_cmn_msg.h"
#include "intel_fpga_api_cmn_dfl.h"
#include "intel_fpga_api_zephyr.h"
#include "intel_fpga_platform_zephyr.h"
#include "intel_fpga_platform_api_zephyr.h"

static size_t s_zephyr_addr_span = 0;
static int s_zephyr_single_component_mode = 0;
static size_t s_zephyr_start_addr = 0;
static void *s_zephyr_mmap_ptr = NULL;

static uint64_t s_zephyr_dfl_addr = 0;
static bool s_zephyr_dfl_found = false;

//static int8_t s_zephyr_irq = -1;
//static const struct device *s_zephyr_dev = NULL;
//static bool zephyr_open_driver();
#ifdef ZEPHYR_UNIT_TEST_SW_MODEL_MODE
static bool zephyr_create_unit_test_sw_model();
#endif
static bool zephyr_scan_interfaces();
static void zephyr_parse_args(unsigned int argc, const char *argv[]);
static bool zephyr_validate_args();
static long parse_integer_arg(const char *name);
extern char *optarg;
extern int optind, optopt;

struct dev_info {
    const struct device *dev;
    uint64_t base_addr;
    uint32_t size;
    int32_t irq;
	uint32_t type;
};

#define FPGA_IP_ACCESS_DEV(node_id) \
    { .dev = DEVICE_DT_GET(node_id), \
      .base_addr = (uint64_t)DT_REG_ADDR(node_id), \
      .size = DT_REG_SIZE(node_id), \
      COND_CODE_1(DT_IRQ_HAS_IDX(node_id, 0), \
       (.irq = DT_IRQN(node_id)), \
       (.irq = -1)), \
       .type = 0, \
       IF_ENABLED(DT_NODE_HAS_COMPAT(node_id, fpga_ip_access_dfl), (.type =2,)) \
       IF_ENABLED(DT_NODE_HAS_COMPAT(node_id, fpga_ip_access), (.type =1,)) \
},

static const struct dev_info fpga_ip_access_dev_table[] = {
    DT_FOREACH_STATUS_OKAY(syscon, FPGA_IP_ACCESS_DEV)
};

bool fpga_platform_init(unsigned int argc, const char *argv[])
{
    bool        is_args_valid;
    bool        ret = false;

    zephyr_parse_args(argc, argv);
    is_args_valid = zephyr_validate_args();

    if (is_args_valid)
    {
#ifndef ZEPHYR_UNIT_TEST_SW_MODEL_MODE
        if(zephyr_scan_interfaces() == false)
            goto all_ret;
#else
        if(zephyr_create_unit_test_sw_model() == false)
            goto all_ret;
#endif
        ret = true;
    }
all_ret:
    return ret;
}

void fpga_platform_cleanup()
{
    // Re-initialize local variables.
    s_zephyr_start_addr = 0;
    s_zephyr_addr_span = 0;
    s_zephyr_single_component_mode = 0;
    s_zephyr_mmap_ptr = NULL;

    if (common_fpga_interface_info_vec_size() > 0)
    {
#ifdef ZEPHYR_UNIT_TEST_SW_MODEL_MODE
        free(common_fpga_interface_info_vec_at(0)->base_address);
#endif
        common_fpga_interface_info_vec_resize(0);
    }

    errno = -1;  //  -1 is a valid return on success.  Some function, such as strtol(), doesn't set errno upon successful return.

    fpga_msg_printf( FPGA_MSG_PRINTF_DEBUG, "Completed fpga_platform_cleanup" );
}

void zephyr_parse_args(unsigned int argc, const char *argv[])
{
    static struct option long_options[] =
        {
            {"address-span",		required_argument, 	0, 				's'}, //For future UNIT_TEST_SW_MODEL_MODE purpose
            {0, 0, 0, 0}
	};

    int option_index = 0;
    int c;

    while(1)
    {
        c = getopt_long(argc, (char * const*)argv, "s:", long_options, &option_index);

        if (c == -1)
        {
            break;
        }

        switch(c)
        {
            case 's':
                s_zephyr_addr_span = parse_integer_arg("Address span");
                break;
        }
    }
}

long parse_integer_arg( const char *name )
{
    long ret = 0;

    bool is_all_digit = true;
    char *p;
    typedef int (*DIGIT_TEST_FN) ( int c );
    DIGIT_TEST_FN is_acceptabl_digit;
    if(optarg[0] == '0' && (optarg[1] == 'x' || optarg[1] == 'X'))
    {
        is_acceptabl_digit = isxdigit;
        optarg += 2;    // trim the "0x" portion
    }
    else
    {
        is_acceptabl_digit = isdigit;
    }

    for(p = optarg; (*p) != '\0'; ++p)
    {
        if(!is_acceptabl_digit(*p))
        {
            is_all_digit = false;
            break;
        }
    }

    if ( is_acceptabl_digit == isxdigit )
    {
        optarg -= 2;  // restore the "0x" portion
    }

    if (is_all_digit)
    {
        if (sizeof(size_t) <= sizeof(long))
        {
            ret = (size_t)strtol(optarg, NULL, 0);
            if (errno == ERANGE)
            {
                ret = 0;
                fpga_msg_printf( FPGA_MSG_PRINTF_ERROR, "%s value is too big. %s is provided; maximum accepted is %ld", name, optarg, LONG_MAX );
            }
        }
        else
        {
            long span, span_c;
            span = strtol(optarg, NULL, 0);
            if (errno == ERANGE)
            {
                ret = 0;
                fpga_msg_printf( FPGA_MSG_PRINTF_ERROR, "%s value is too big. %s is provided; maximum accepted is %ld", name, optarg, LONG_MAX );
            }
            else
            {
                ret = (size_t)span;
                span_c = ret;
                if (span != span_c)
                {
                    fpga_msg_printf( FPGA_MSG_PRINTF_ERROR, "%s value is too big. %s is provided; maximum accepted is %ld", name, optarg, (size_t)-1 );
                    ret = 0;
                }
            }
        }
    }
    else
    {
        fpga_msg_printf( FPGA_MSG_PRINTF_ERROR, "Invalid argument value type is provided. A integer value is expected. %s is provided.", optarg );
    }

    return ret;
}

bool zephyr_validate_args()
{
    bool ret =false;

#ifdef ZEPHYR_UNIT_TEST_SW_MODEL_MODE
    ret = s_zephyr_addr_span > 0;

    if (!ret)
    {
        if ( s_zephyr_addr_span == 0 )
        {
            fpga_msg_printf( FPGA_MSG_PRINTF_ERROR, "No valid address span value is provided using the argument, --address-span." );
        }
    }
#else
    ret = true;
#endif
    return ret;
}

uint64_t zephyr_dfl_base_addr_decoder(uint64_t base_addr)
{
    return(base_addr - s_zephyr_dfl_addr);
}

bool zephyr_scan_interfaces()
{
    bool ret = false;
    uint32_t index = 0;
    uint8_t  discovered_dev_n = ARRAY_SIZE(fpga_ip_access_dev_table);
    void *first_dfh_addr = NULL;

    if( discovered_dev_n > 0)
    {
        if(s_zephyr_single_component_mode)
        {
            common_fpga_interface_info_vec_resize(1);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
            common_fpga_interface_info_vec_at(0)->base_address = (void *)(fpga_ip_access_dev_table[0].base_addr + s_zephyr_start_addr);
#pragma GCC diagnostic pop
            common_fpga_interface_info_vec_at(0)->dev = fpga_ip_access_dev_table[0].dev;
            common_fpga_interface_info_vec_at(0)->irq = fpga_ip_access_dev_table[0].irq;
            common_fpga_interface_info_vec_at(0)->is_mmio_opened = false;
            common_fpga_interface_info_vec_at(0)->is_interrupt_opened = false;

            printk("SINGLE COMPONENT MODE : Only 1 device can be used!\n");
        }
        else
        {
            for (uint32_t n = 0; n < ARRAY_SIZE(fpga_ip_access_dev_table); n++) {

				switch (fpga_ip_access_dev_table[n].type)
				{
					case 1:
						// Add new index for new device
                        // Always after DFL rom
                        index = common_fpga_interface_info_vec_size();
						common_fpga_interface_info_vec_resize(index+1);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
						common_fpga_interface_info_vec_at(index)->base_address = (void *)(fpga_ip_access_dev_table[n].base_addr + s_zephyr_start_addr);
#pragma GCC diagnostic pop
						common_fpga_interface_info_vec_at(index)->dfl = false;
						common_fpga_interface_info_vec_at(index)->dev = fpga_ip_access_dev_table[n].dev;
						common_fpga_interface_info_vec_at(index)->irq = fpga_ip_access_dev_table[n].irq;
						common_fpga_interface_info_vec_at(index)->is_mmio_opened = false;
						common_fpga_interface_info_vec_at(index)->is_interrupt_opened = false;
						break;

					case 2:
                        // Assume DFL rom start at index 0
                        // Only support 1 DFL rom
                        if(s_zephyr_dfl_found == true)
                            return ret;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
                        first_dfh_addr = (void *)(fpga_ip_access_dev_table[n].base_addr + s_zephyr_start_addr);
#pragma GCC diagnostic pop
                        s_zephyr_dfl_addr = fpga_ip_access_dev_table[n].base_addr;
                        s_zephyr_dfl_found = true;
						common_dfl_scan_multi_interfaces(first_dfh_addr, zephyr_dfl_base_addr_decoder);
					break;

					default:
						break;

				}
            }
        }
        ret = true;
    }
    return ret;
}

bool zephyr_create_unit_test_sw_model()
{
    bool  ret = true;

    common_fpga_interface_info_vec_resize(1);

    common_fpga_interface_info_vec_at(0)->base_address = malloc(s_zephyr_addr_span);
    // Preset mem with all 1s
    memset(common_fpga_interface_info_vec_at(0)->base_address, 0xFF, s_zephyr_addr_span);

    return ret;
}

