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

#include "intel_fpga_platform_api.h"
#include "intel_fpga_api.h"

void test_interfaces(int num_interfaces);

int main(int argc, const char *argv[])
{
    fpga_platform_init(argc, argv);

    unsigned int num_interfaces = fpga_get_num_of_interfaces();
    fpga_msg_printf(FPGA_MSG_PRINTF_INFO, "");
    fpga_msg_printf(FPGA_MSG_PRINTF_INFO, "");
    fpga_msg_printf(FPGA_MSG_PRINTF_INFO, "========API Calls==========", num_interfaces);
    fpga_msg_printf(FPGA_MSG_PRINTF_INFO, "fpga_get_num_interfaces() -> %d", num_interfaces);
    test_interfaces(num_interfaces);

    fpga_platform_cleanup();
}

void test_interfaces(int num_interfaces)
{
    FPGA_INTERFACE_INFO info;
    FPGA_MMIO_INTERFACE_HANDLE handle;
    
    for( int index = 0; index < num_interfaces; ++index )
    {
        fpga_msg_printf(FPGA_MSG_PRINTF_INFO, "   ---------- Interface %d ----------", index);
        fpga_get_interface_at(index, &info);
        fpga_msg_printf(FPGA_MSG_PRINTF_INFO, "   fpga_get_interface_at(%d, &info) -> info", index);
        fpga_msg_printf(FPGA_MSG_PRINTF_INFO, "   128-bit GUID: 0x%016llX%016llX", info.guid.guid_h, info.guid.guid_l);
        fpga_msg_printf(FPGA_MSG_PRINTF_INFO, "   Instance ID: %d", info.instance_id);
        fpga_msg_printf(FPGA_MSG_PRINTF_INFO, "   Group ID: %d", info.group_id);

        for (int i = 0; i < info.num_of_parameters; i++)
        {
            fpga_msg_printf(FPGA_MSG_PRINTF_INFO, "      ------ Parameter %d ------   ", i);
            fpga_msg_printf(FPGA_MSG_PRINTF_INFO, "      Version: %d", info.parameters[i].version);
            fpga_msg_printf(FPGA_MSG_PRINTF_INFO, "      Param ID: %d", info.parameters[i].param_id);
            fpga_msg_printf(FPGA_MSG_PRINTF_INFO, "      Param Data Size: %d", info.parameters[i].data_size);

            for (int j = 0; j < info.parameters[i].data_size / 8; j++)
            {
                fpga_msg_printf(FPGA_MSG_PRINTF_INFO, "      64-bit Param Data[%d]: 0x%016llX", j, info.parameters[i].data[j]);
            }
        }        
        
        handle = fpga_open(index);
        fpga_msg_printf(FPGA_MSG_PRINTF_INFO, "   fpga_open(%d) -> 0x%08X", index, handle);
    }
    
}


