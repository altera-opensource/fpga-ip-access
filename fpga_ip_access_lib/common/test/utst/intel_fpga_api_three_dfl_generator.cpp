// // Copyright(c) 2023, Intel Corporation
// //
// // Redistribution  and  use  in source  and  binary  forms,  with  or  without
// // modification, are permitted provided that the following conditions are met:
// //
// // * Redistributions of  source code  must retain the  above copyright notice,
// //   this list of conditions and the following disclaimer.
// // * Redistributions in binary form must reproduce the above copyright notice,
// //   this list of conditions and the following disclaimer in the documentation
// //   and/or other materials provided with the distribution.
// // * Neither the name  of Intel Corporation  nor the names of its contributors
// //   may be used to  endorse or promote  products derived  from this  software
// //   without specific prior written permission.
// //
// // THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// // AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED TO,  THE
// // IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// // ARE DISCLAIMED.  IN NO EVENT  SHALL THE COPYRIGHT OWNER  OR CONTRIBUTORS BE
// // LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR
// // CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT LIMITED  TO,  PROCUREMENT  OF
// // SUBSTITUTE GOODS OR SERVICES;  LOSS OF USE,  DATA, OR PROFITS;  OR BUSINESS
// // INTERRUPTION)  HOWEVER CAUSED  AND ON ANY THEORY  OF LIABILITY,  WHETHER IN
// // CONTRACT,  STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE  OR OTHERWISE)
// // ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  EVEN IF ADVISED OF THE
// // POSSIBILITY OF SUCH DAMAGE.


// #include <stdio.h>
// #include <stdarg.h>
// #include <sstream>
// #include <iostream>
// #include <fstream>

// using namespace std;

// #include "gtest/gtest.h"

// #include "intel_fpga_api_cmn_inf.h"
// //#include "intel_fpga_platform_uio.h"
// #include "intel_fpga_api_cmn_dfl.h"

// // This test is used to generate DFL ROM base on the four defined address below
// #define DFL_ENTRY_ADDRESS 0x4000
// #define RAM_ONE_BASE_ADDRESS 0x5000
// #define RAM_TWO_BASE_ADDRESS 0x6000
// #define RAM_THREE_BASE_ADDRESS 0x7000


// // The following macros are used to construct 64-bit inside dfl
// #define CONSTRUCT_DFH(FeatureType_0xF, DFHVersion_0xFF, FeatureMinorRev_0xF, Reserved_0xFE, EOL_0x1, NextDfhByteOffset_0xFFFFFF, FeatureRev_0xF, FeatureID_0xFFF) ((0xF & (uint64_t)FeatureType_0xF) << 60) | ((0xFF & (uint64_t)DFHVersion_0xFF) << 52) | ((0xF & (uint64_t)FeatureMinorRev_0xF) << 48) | ((0b1111111 & (uint64_t)Reserved_0xFE) << 41) | ((0x1 & (uint64_t)EOL_0x1) << 40) | ((0xFFFFFF & (uint64_t)NextDfhByteOffset_0xFFFFFF) << 16) | ((0xF & (uint64_t)FeatureRev_0xF) << 12) | (0xFFF & (uint64_t)FeatureID_0xFFF)
// #define CONSTRUCT_CSR_ADDR(csr_addr_0xFFFFFFFFFFFFFFFE, csr_rel_n_0b1) ((0xFFFFFFFFFFFFFFFE & (uint64_t)csr_addr_0xFFFFFFFFFFFFFFFE) << 1) | (0b1 & (uint64_t)csr_rel_n_0b1)
// #define CONSTRUCT_CSR_SIZE_GROUP(csr_size_0xFFFFFFFF, has_params_0b1, group_id_0x7FFF, instance_id_0xFFFF) ((0xFFFFFFFFFFFF & (uint64_t)csr_size_0xFFFFFFFF) << 32) | ((0b1 & (uint64_t)has_params_0b1) << 31) | ((0x7FFF & (uint64_t)group_id_0x7FFF) << 16) | (0xFFFF & (uint64_t)instance_id_0xFFFF)
// #define CONSTRUCT_PARAM_HEADER(next_0xFFFFFFFF, version_0xFFFF, param_id_0xFFFF) ((0xFFFFFFFF & (uint64_t)next_0xFFFFFFFF) << 32) | ((0xFFFF & (uint64_t)version_0xFFFF) << 16) | (0xFFFF & (uint64_t)param_id_0xFFFF)

// // Creates a single dfl without using Heirarchical dfl
// //need to be 4k align so need 512 64-bits data

// typedef struct
// {
//     uint64_t x_feature_dfh;            // Device Feature Header itself
//     uint64_t x_feature_guid_l;         // Low 64-bit of GUID
//     uint64_t x_feature_guid_h;         // High 64-bit of GUID
//     uint64_t x_feature_csr_addr;       // Location of feature register block
//     uint64_t x_feature_csr_size_group; // Location of feature register block
//     uint64_t x_param_header;           //(optional) Parameter header
//     uint64_t x_param_y;                //(optional) Parameter data
//     uint64_t reserved[505] = {0};            //reserved to make 4k page aligned

// } INTERFACE;

// // Createss a dfl with specified number of interfaces for testing
// static void create_single_dfl(INTERFACE dfl[], unsigned int num_interfaces)
// {
//     // following variables can be used to craft your own dfl
//     uint64_t FeatureType;
//     uint64_t DFHVersion;
//     uint64_t FeatureMinorRev;
//     uint64_t Reserved;
//     uint64_t EOL;
//     uint64_t NextDfhByteOffset;
//     uint64_t FeatureRev;
//     uint64_t FeatureID;

//     uint64_t csr_addr;
//     uint64_t csr_rel_n;

//     uint64_t csr_size;
//     uint64_t has_params;
//     uint64_t group_id;
//     uint64_t instance_id;

//     uint64_t next;
//     uint64_t version;
//     uint64_t param_id;

//     for (unsigned int i = 0; i < num_interfaces; i++)
//     {
//         if (i == num_interfaces - 1)
//         {
//             // set up last interface
//             FeatureType = 0x3;
//             DFHVersion = i;
//             FeatureMinorRev = 0;
//             Reserved = 0;
//             EOL = 0b1;
//             NextDfhByteOffset = 0;
//             FeatureRev = 0;
//             FeatureID = 0x23;
//             dfl[i].x_feature_dfh = CONSTRUCT_DFH(FeatureType, DFHVersion, FeatureMinorRev, Reserved, EOL, NextDfhByteOffset, FeatureRev, FeatureID);

//             dfl[i].x_feature_guid_l = i;
//             dfl[i].x_feature_guid_h = 0;

//             csr_addr = 0;
//             csr_rel_n = 0;
//             dfl[i].x_feature_csr_addr = CONSTRUCT_CSR_ADDR(csr_addr, csr_rel_n);

//             csr_size = 0;
//             has_params = 0;
//             group_id = 0;
//             instance_id = 0;
//             dfl[i].x_feature_csr_size_group = CONSTRUCT_CSR_SIZE_GROUP(csr_size, has_params, group_id, instance_id);

//             next = 0;
//             version = 0;
//             param_id = 0;
//             dfl[i].x_param_header = CONSTRUCT_PARAM_HEADER(next, version, param_id);
//             dfl[i].x_param_y = 0;

//             break;
//         }

//         FeatureType = 0x3;
//         DFHVersion = i;
//         FeatureMinorRev = 0;
//         Reserved = 0;
//         EOL = 0b0;
//         NextDfhByteOffset = (&dfl[i + 1] - &dfl[i]) * sizeof(INTERFACE);
//         FeatureRev = 0;
//         FeatureID = 0x23;
//         dfl[i].x_feature_dfh = CONSTRUCT_DFH(FeatureType, DFHVersion, FeatureMinorRev, Reserved, EOL, NextDfhByteOffset, FeatureRev, FeatureID);

//         dfl[i].x_feature_guid_l = i;
//         dfl[i].x_feature_guid_h = 0;

//         csr_addr = 0;
//         csr_rel_n = 0;
//         dfl[i].x_feature_csr_addr = CONSTRUCT_CSR_ADDR(csr_addr, csr_rel_n);

//         csr_size = 0;
//         has_params = 0;
//         group_id = 0;
//         instance_id = 0;
//         dfl[i].x_feature_csr_size_group = CONSTRUCT_CSR_SIZE_GROUP(csr_size, has_params, group_id, instance_id);

//         next = 0;
//         version = 0;
//         param_id = 0;
//         dfl[i].x_param_header = CONSTRUCT_PARAM_HEADER(next, version, param_id);
//         dfl[i].x_param_y = 0;
//     }
//     uint64_t dfl_entry_addr = DFL_ENTRY_ADDRESS;
//     uint64_t ram_one_addr = RAM_ONE_BASE_ADDRESS;
//     dfl[0].x_feature_csr_addr = ram_one_addr - dfl_entry_addr;

//     uint64_t ram_two_addr = RAM_TWO_BASE_ADDRESS;
//     dfl[1].x_feature_csr_addr = ram_two_addr - (dfl_entry_addr + sizeof(dfl[0]));

//     uint64_t ram_three_addr = RAM_THREE_BASE_ADDRESS;
//     dfl[2].x_feature_csr_addr = ram_three_addr - (dfl_entry_addr + sizeof(dfl[0]) + sizeof(dfl[1]));
// }

// class create_single_dfh : public ::testing::Test
// {
// public:
//     void SetUp()
//     {
//     }

//     void TearDown()
//     {
//     }

// protected:
// };


// TEST_F(create_single_dfh, should_create_three_interfaces)
// {
//     size_t num_interface = 3;
//     INTERFACE dfl[num_interface];
//     create_single_dfl(dfl, num_interface);

//     common_dfl_scan_multi_interfaces(&DFL[0], dfl_base_addr_decoder_mock);
//     EXPECT_EQ(num_interface, common_fpga_interface_info_vec_size()) << "common_fpga_interface_info_vec_size() not match";

//     //check_interface_info(dfl, num_interface);

//     // save the dfl to file
//     ofstream myfile("/tmp/dfl_rom.txt");
//     if (myfile.is_open())
//     {
//         myfile << "This file contains three DFL entry points. Cureently three interfaces included \n";
//         for (int count = 0; count < 3; count++)
//         {
//             myfile << (void *)dfl[count].x_feature_dfh << endl;
//             myfile << (void *)dfl[count].x_feature_guid_l << endl;
//             myfile << (void *)dfl[count].x_feature_guid_h << endl;
//             myfile << (void *)dfl[count].x_feature_csr_addr << endl;
//             myfile << (void *)dfl[count].x_feature_csr_size_group << endl;
//             myfile << (void *)dfl[count].x_param_header << endl;
//             myfile << (void *)dfl[count].x_param_y << endl;
//             for (int i = 0; i < 505; i++)
//             {
//                 myfile << (void *)dfl[count].reserved[i] << endl;
//             }
//         }

//         myfile << "uint version \n";
//         for (int count = 0; count < 3; count++)
//         {
//             myfile << dfl[count].x_feature_dfh << endl;
//             myfile << dfl[count].x_feature_guid_l << endl;
//             myfile << dfl[count].x_feature_guid_h << endl;
//             myfile << dfl[count].x_feature_csr_addr << endl;
//             myfile << dfl[count].x_feature_csr_size_group << endl;
//             myfile << dfl[count].x_param_header << endl;
//             myfile << dfl[count].x_param_y << endl;
//             for (int i = 0; i < 505; i++)
//             {
//                 myfile << (void *)dfl[count].reserved[i] << endl;
//             }
//         }
//         myfile.close();
//     }
//     else
//         cout << "Unable to open file";
// }
