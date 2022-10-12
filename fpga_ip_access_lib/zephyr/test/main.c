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

#include <zephyr/kernel.h>
#include <zephyr/sys/device_mmio.h>

#include "intel_fpga_api_zephyr.h"
#include "intel_fpga_platform_api_zephyr.h"
#include "intel_fpga_api_cmn_inf.h"
#include "intel_fpga_api_cmn_msg.h"
#include "intel_fpga_api_cmn_version.h"
#include <stdio.h>
#include <stdarg.h>

/* register offsets */
#define PIO_DATA_REG_OFFSET		0x00
#define PIO_DIR_REG_OFFSET		0x04
#define PIO_INT_MASK_REG_OFFSET		0x08
#define PIO_EDGECAPTURE_REG_OFFSET	0x0c

/* This test cases performed under Nios V system with
 * the following configurations   
 * 
 * Name				Base    End        IRQ
 * timer_sw_agent	0x90000 0x9003f
 * dm_sw_agent		0x80000 0x8ffff
 * ram				0x00000 0x3ffff
 * jtag_uart		0x90078 0x9007f    0
 * pio				0xa0000 0xa000f    1
 * pio				0xb0000 0xb000f    2
 * pio				0xc0000 0xc000f    3
 * DFL rom			0x100000 0x1000000
 *   - ram1			0x300000 0x1000
 *   - ram2			0x400000 0x1000
 * Take note that Nios V started from 16. 
 */

#ifdef ZEPHYR_UNIT_TEST_INT
static void my_isr(const FPGA_MMIO_INTERFACE_HANDLE *args){

    printk("my_isr Interrupt triggered-api!!!.\n");
    if(args != NULL)
    {
        fpga_write_32(*args, PIO_EDGECAPTURE_REG_OFFSET, 0xffffffff);
    }
}

static void my_isr_2(const FPGA_MMIO_INTERFACE_HANDLE *args){

    printk("my_isr_2 Interrupt triggered-api!!!.\n");
    if(args != NULL)
    {
         fpga_write_32(*args, PIO_EDGECAPTURE_REG_OFFSET, 0xffffffff);
    }
}

static void my_isr_3(const FPGA_MMIO_INTERFACE_HANDLE *args){

    printk("my_isr_3 Interrupt triggered-api!!!.\n");
    if(args != NULL)
    {
        fpga_write_32(*args, PIO_EDGECAPTURE_REG_OFFSET, 0xffffffff);
    }
}

FPGA_MMIO_INTERFACE_HANDLE s_handle = FPGA_MMIO_INTERFACE_INVALID_HANDLE;
FPGA_MMIO_INTERFACE_HANDLE s_handle_2 = FPGA_MMIO_INTERFACE_INVALID_HANDLE;
FPGA_MMIO_INTERFACE_HANDLE s_handle_3 = FPGA_MMIO_INTERFACE_INVALID_HANDLE;
#endif


static int customize_print(FPGA_MSG_PRINTF_TYPE type, const char * format, va_list args)
{
    int ret;

    switch(type)
    {
        case FPGA_MSG_PRINTF_DEBUG:
            printf("DEBUG: ");
            break;
        case FPGA_MSG_PRINTF_WARNING:
            printf("WARNING: ");
            break;
        case FPGA_MSG_PRINTF_ERROR:
            printf("ERROR: ");
            break;
        case FPGA_MSG_PRINTF_INFO:
            printf("INFO: ");
            break;
    }

    ret = vprintf(format, args);
    return ret;
}

int main(void)
{
    bool rc;
	struct z_device_mmio_rom *rom;
    uint32_t interface_i;

    const char *argv_valid[] =
    {
        "program"
    };

    printk("\n\nThis is Zephyr fpga ip access test on %s platform.\n", CONFIG_BOARD);
    printf("######################################\n");
    printf("###Zephyr fpga ip access test started ###\n");
    printf("###Version %s-%s            ###\n", APP_VERSION_BASE,GIT_VERSION);
    printf("######################################\n\n");

    fpga_platform_register_printf(customize_print);
    rc = fpga_platform_init(1, argv_valid);
    if (rc != true){
        fpga_msg_printf(FPGA_MSG_PRINTF_ERROR, "Error!: fpga_platform_init\n");
        return -1;
    }
    fpga_msg_printf(FPGA_MSG_PRINTF_INFO, "Fpga platform init completed\n\n");

    const struct device *dev_1 = DEVICE_DT_GET(DT_NODELABEL(myip1));
	const struct device *dev_2 = DEVICE_DT_GET(DT_NODELABEL(myip2));
	const struct device *dev_3 = DEVICE_DT_GET(DT_NODELABEL(myip3));

	rom = NULL;
	rom = DEVICE_MMIO_ROM_PTR(dev_1);
    if(rom == NULL)
	{
        fpga_msg_printf(FPGA_MSG_PRINTF_ERROR, "Error!: DEVICE_MMIO_ROM_PTR(dev_1)\n");
        return -1;
	}
    if(rom->addr != 0xa0000)
    {
        fpga_msg_printf(FPGA_MSG_PRINTF_ERROR, "Error!: DEVICE_MMIO_ROM_PTR(dev_1)\n");
        return -1;
    }

	rom = NULL;
    rom = DEVICE_MMIO_ROM_PTR(dev_2);
    if(rom == NULL)
    {
        fpga_msg_printf(FPGA_MSG_PRINTF_ERROR, "Error!: DEVICE_MMIO_ROM_PTR(dev_2)\n");
        return -1;
    }
    if(rom->addr != 0xb0000)
    {
        fpga_msg_printf(FPGA_MSG_PRINTF_ERROR, "Error!: DEVICE_MMIO_ROM_PTR(dev_2)\n");
        return -1;
    }

	rom = NULL;
    rom = DEVICE_MMIO_ROM_PTR(dev_3);
    if(rom == NULL)
    {
        fpga_msg_printf(FPGA_MSG_PRINTF_ERROR, "Error!: DEVICE_MMIO_ROM_PTR(dev_3)\n");
        return -1;
    }
    if(rom->addr != 0xc0000)
    {
        fpga_msg_printf(FPGA_MSG_PRINTF_ERROR, "Error!: DEVICE_MMIO_ROM_PTR(dev_3)\n");
        return -1;
    }

    //5 interfaces
    if(fpga_get_num_of_interfaces() != 5)
    {
        fpga_msg_printf(FPGA_MSG_PRINTF_ERROR, "Error!: Number of interfaces test failed!\n");
        fpga_msg_printf(FPGA_MSG_PRINTF_ERROR, "interface number =%d, expected number = %d!\n", fpga_get_num_of_interfaces(), 5);
        return -1;
    }

    fpga_msg_printf(FPGA_MSG_PRINTF_INFO, "Number of devices discovery test passed.\n");

    for(interface_i=0; interface_i < common_fpga_interface_info_vec_size(); interface_i++)
	{
        fpga_msg_printf(FPGA_MSG_PRINTF_INFO, "common_fpga_interface_info_vec_at(%d), base_addr = %p\n", interface_i, common_fpga_interface_info_vec_at(interface_i)->base_address);
	}
    fpga_msg_printf(FPGA_MSG_PRINTF_INFO, "Please check the base address for each interface shown above\n\n");

    //GUID test
    //Device 1 GUID : 0xDF3F3B5A300B40878CE57F09B3459B69
    //Device 2 GUID : 0xE3A2A9843F0340D0B679F5A982E806B4
    FPGA_INTERFACE_INFO info;
    fpga_get_interface_at(0, &info);
    if ((info.guid.guid_h != 0xDF3F3B5A300B4087) && (info.guid.guid_l != 0x8CE57F09B3459B69))
    {
        fpga_msg_printf(FPGA_MSG_PRINTF_ERROR, "Error! Device 1 GUID test failed!.\n");
        return -1;
    }

    fpga_get_interface_at(1, &info);
    if ((info.guid.guid_h != 0xE3A2A9843F0340D0) && (info.guid.guid_l != 0xB679F5A982E806B4))
    {
        fpga_msg_printf(FPGA_MSG_PRINTF_ERROR, "Error! Device 2 GUID test failed!.\n");
        return -1;
    }
    fpga_msg_printf(FPGA_MSG_PRINTF_INFO, "Devices GUID test passed.\n\n");

    FPGA_MMIO_INTERFACE_HANDLE m_dfl_handle = fpga_open(0);
    if( m_dfl_handle == FPGA_MMIO_INTERFACE_INVALID_HANDLE)
    {
        fpga_msg_printf(FPGA_MSG_PRINTF_ERROR, "Error!: Invalid m_dfl_handle\n");
        return -1;
    }
    fpga_write_32(m_dfl_handle, 0x0, 0xABABABAB);
    fpga_write_32(m_dfl_handle, 0x4, 0xBEEFBEEF);
    if((fpga_read_32(m_dfl_handle, 0x0) != 0xABABABAB) || (fpga_read_32(m_dfl_handle, 0x4) != 0xBEEFBEEF) || (fpga_read_32(m_dfl_handle, 0x0) != *(uint32_t *)0x300000) || (fpga_read_32(m_dfl_handle, 0x4) != *(uint32_t *)0x300004))
    {
        fpga_msg_printf(FPGA_MSG_PRINTF_ERROR, "Error! Device 1 memory test failed!.\n");
    }

    FPGA_MMIO_INTERFACE_HANDLE m_dfl_handle_2 = fpga_open(1);
    if( m_dfl_handle_2 == FPGA_MMIO_INTERFACE_INVALID_HANDLE)
    {
        fpga_msg_printf(FPGA_MSG_PRINTF_ERROR, "Error!: Invalid m_dfl_handle_2\n");
        return -1;
    }
    fpga_write_32(m_dfl_handle_2, 0x10, 0xAAAABBBB);
    fpga_write_32(m_dfl_handle_2, 0x14, 0xDEEDDEED);
    if((fpga_read_32(m_dfl_handle_2, 0x10) != 0xAAAABBBB) || (fpga_read_32(m_dfl_handle_2, 0x14) != 0xDEEDDEED) || (fpga_read_32(m_dfl_handle_2, 0x10) != *(uint32_t *)0x400010) || (fpga_read_32(m_dfl_handle_2, 0x14) != *(uint32_t *)0x400014))
    {
        fpga_msg_printf(FPGA_MSG_PRINTF_ERROR, "Error! Device 2 memory test failed!.\n");
    }

    FPGA_MMIO_INTERFACE_HANDLE m_handle = fpga_open(2);
    FPGA_MMIO_INTERFACE_HANDLE m_handle_2 = fpga_open(3);
	FPGA_MMIO_INTERFACE_HANDLE m_handle_3 = fpga_open(4);

    if( m_handle == FPGA_MMIO_INTERFACE_INVALID_HANDLE)
	{
		fpga_msg_printf(FPGA_MSG_PRINTF_ERROR, "Error!: Invalid m_handle\n");
        return -1;
	}

	if( m_handle_2 == FPGA_MMIO_INTERFACE_INVALID_HANDLE)
    {
		fpga_msg_printf(FPGA_MSG_PRINTF_ERROR, "Error!: Invalid m_handle_2\n");
        return -1;
    }

	if( m_handle_3 == FPGA_MMIO_INTERFACE_INVALID_HANDLE)
    {
		fpga_msg_printf(FPGA_MSG_PRINTF_ERROR, "Error!: Invalid m_handle_3\n");
        return -1;
    }

    // Clear and set 1 to interrupts mask register
    // Only available input ports bit will be set
    fpga_write_32(m_handle, PIO_INT_MASK_REG_OFFSET, 0x0);
    fpga_write_32(m_handle, PIO_INT_MASK_REG_OFFSET, 0xffffffff);
	fpga_write_32(m_handle_2, PIO_INT_MASK_REG_OFFSET, 0x0);
	fpga_write_32(m_handle_2, PIO_INT_MASK_REG_OFFSET, 0xffffffff);
	fpga_write_32(m_handle_3, PIO_INT_MASK_REG_OFFSET, 0x0);
	fpga_write_32(m_handle_3, PIO_INT_MASK_REG_OFFSET, 0xffffffff);

	// Return 0x7ff as only 11 bits for device 1
	if(fpga_read_32(m_handle, PIO_INT_MASK_REG_OFFSET) != 0x7ff )
	{
		fpga_msg_printf(FPGA_MSG_PRINTF_ERROR, "Error!: m_handle read value %x mismatched with expected value 0x7ff.\n", fpga_read_32(m_handle, PIO_INT_MASK_REG_OFFSET));
        return -1;
	}

	// Return 0x3ff as only 10 bits for device 2
	if(fpga_read_32(m_handle_2, PIO_INT_MASK_REG_OFFSET) != 0x3ff )
    {
        fpga_msg_printf(FPGA_MSG_PRINTF_ERROR, "Error!: m_handle_2 read value %x mismatched with expected value 0x3ff.\n", fpga_read_32(m_handle_2, PIO_INT_MASK_REG_OFFSET));
        return -1;
    }

	// Return 0x3ff as only 10 bits for device 3
	if(fpga_read_32(m_handle_3, PIO_INT_MASK_REG_OFFSET) != 0x3ff )
    {
        fpga_msg_printf(FPGA_MSG_PRINTF_ERROR, "Error!: m_handle_3 read value %x mismatched with expected value 0x3ff.\n", fpga_read_32(m_handle_3, PIO_INT_MASK_REG_OFFSET));
        return -1;
    }
    fpga_msg_printf(FPGA_MSG_PRINTF_INFO, "Memory address test passed.\n\n");

#ifdef ZEPHYR_UNIT_TEST_INT
    fpga_msg_printf(FPGA_MSG_PRINTF_INFO, "Interrupt Manual Test started....\n");
    fpga_msg_printf(FPGA_MSG_PRINTF_INFO, "User expected to stop test manually\n");
    if( m_handle != FPGA_MMIO_INTERFACE_INVALID_HANDLE)
    {
		s_handle = m_handle;
		fpga_register_isr(m_handle, &my_isr, &s_handle);
		fpga_write_32(m_handle, PIO_INT_MASK_REG_OFFSET, 0xffffffff);

	}

    if( m_handle_2 != FPGA_MMIO_INTERFACE_INVALID_HANDLE)
    {
        s_handle_2 = m_handle_2;
        fpga_register_isr(m_handle_2, &my_isr_2, &s_handle_2);
        fpga_write_32(m_handle_2, PIO_INT_MASK_REG_OFFSET, 0xffffffff);
    }

    if( m_handle_3 != FPGA_MMIO_INTERFACE_INVALID_HANDLE)
    {
        s_handle_3 = m_handle_3;
        fpga_register_isr(m_handle_3, &my_isr_3, &s_handle_3);
        fpga_write_32(m_handle_3, PIO_INT_MASK_REG_OFFSET, 0xffffffff);
    }

    fpga_enable_interrupt(m_handle);

    if( m_handle_2 != FPGA_MMIO_INTERFACE_INVALID_HANDLE)
        fpga_enable_interrupt(m_handle_2);

    if( m_handle_3 != FPGA_MMIO_INTERFACE_INVALID_HANDLE)
        fpga_enable_interrupt(m_handle_3);

    fpga_msg_printf(FPGA_MSG_PRINTF_INFO, "Interrupt enabled\n");
    for(uint32_t i = 0; i < 30 ; i++)
    {
        fpga_msg_printf(FPGA_MSG_PRINTF_INFO, "Clock counting down... %d\n", 30-i);
        k_sleep(K_SECONDS(1));
    }

    fpga_disable_interrupt(m_handle);

    if( m_handle_2 != FPGA_MMIO_INTERFACE_INVALID_HANDLE)
        fpga_disable_interrupt(m_handle_2);

    if( m_handle_3 != FPGA_MMIO_INTERFACE_INVALID_HANDLE)
        fpga_disable_interrupt(m_handle_3);

    fpga_msg_printf(FPGA_MSG_PRINTF_INFO, "Interrupt disabled\n");
    for(uint32_t j = 0; j < 30 ; j++)
    {
        fpga_msg_printf(FPGA_MSG_PRINTF_INFO, "Clock counting down... %d\n", 30-j);
        k_sleep(K_SECONDS(1));
    }

    //Clear PIO edge capture register
    fpga_write_32(0, PIO_EDGECAPTURE_REG_OFFSET, 0xffffffff);
    fpga_enable_interrupt(m_handle);

    if( m_handle_2 != FPGA_MMIO_INTERFACE_INVALID_HANDLE)
    {
        fpga_write_32(m_handle_2, PIO_EDGECAPTURE_REG_OFFSET, 0xffffffff);
        fpga_enable_interrupt(m_handle_2);
    }

    if( m_handle_3 != FPGA_MMIO_INTERFACE_INVALID_HANDLE)
    {
        fpga_write_32(m_handle_3, PIO_EDGECAPTURE_REG_OFFSET, 0xffffffff);
        fpga_enable_interrupt(m_handle_3);
    }

    fpga_msg_printf(FPGA_MSG_PRINTF_INFO, "Interrupt enabled\n");
    while(1)
    {
        fpga_msg_printf(FPGA_MSG_PRINTF_INFO, "Clock ticking forever...\n");
        k_sleep(K_SECONDS(1));
    }
#endif

    fpga_msg_printf(FPGA_MSG_PRINTF_INFO, "###Test finished###\n");
	return 0;
}
