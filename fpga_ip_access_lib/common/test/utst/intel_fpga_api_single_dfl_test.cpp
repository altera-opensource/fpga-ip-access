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

#include <stdio.h>
#include <stdarg.h>
#include <sstream>
#include <iostream>
using namespace std;

#include "gtest/gtest.h"

#include "intel_fpga_api_cmn_inf.h"
#include "intel_fpga_api_cmn_dfl.h"

#define NEXT_PARAM_OFFSET_ADJUSTER 8

// The following macros are used to construct 64-bit inside DFL
#define CONSTRUCT_DFH(FeatureType_0xF, DFHVersion_0xFF, FeatureMinorRev_0xF, Reserved_0xFE, EOL_0x1, NextDfhByteOffset_0xFFFFFF, FeatureRev_0xF, FeatureID_0xFFF) ((0xF & (uint64_t)FeatureType_0xF) << 60) | ((0xFF & (uint64_t)DFHVersion_0xFF) << 52) | ((0xF & (uint64_t)FeatureMinorRev_0xF) << 48) | ((0b1111111 & (uint64_t)Reserved_0xFE) << 41) | ((0x1 & (uint64_t)EOL_0x1) << 40) | ((0xFFFFFF & (uint64_t)NextDfhByteOffset_0xFFFFFF) << 16) | ((0xF & (uint64_t)FeatureRev_0xF) << 12) | (0xFFF & (uint64_t)FeatureID_0xFFF)
#define CONSTRUCT_CSR_ADDR(csr_addr_0xFFFFFFFFFFFFFFFE, csr_rel_n_0b1) ((0xFFFFFFFFFFFFFFFE & (uint64_t)csr_addr_0xFFFFFFFFFFFFFFFE) << 1) | (0b1 & (uint64_t)csr_rel_n_0b1)
#define CONSTRUCT_CSR_SIZE_GROUP(csr_size_0xFFFFFFFF, has_params_0b1, group_id_0x7FFF, instance_id_0xFFFF) ((0xFFFFFFFFFFFF & (uint64_t)csr_size_0xFFFFFFFF) << 32) | ((0b1 & (uint64_t)has_params_0b1) << 31) | ((0x7FFF & (uint64_t)group_id_0x7FFF) << 16) | (0xFFFF & (uint64_t)instance_id_0xFFFF)
#define CONSTRUCT_PARAM_HEADER(next_0x1FFFFFFF, reserved_0b11, eop_0b1, version_0xFFFF, param_id_0xFFFF) ((0x1FFFFFFF & (uint64_t)(next_0x1FFFFFFF/NEXT_PARAM_OFFSET_ADJUSTER)) << 35) | ((0b11 & (uint64_t)reserved_0b11) << 33) | ((0b1 & (uint64_t)eop_0b1) << 32) | ((0xFFFF & (uint64_t)version_0xFFFF) << 16) | (0xFFFF & (uint64_t)param_id_0xFFFF)

// Creates a single DFL without using Heirarchical DFL
typedef struct
{
    uint64_t x_feature_dfh;            // Device Feature Header itself
    uint64_t x_feature_guid_l;         // Low 64-bit of GUID
    uint64_t x_feature_guid_h;         // High 64-bit of GUID
    uint64_t x_feature_csr_addr;       // Location of feature register block
    uint64_t x_feature_csr_size_group; // Location of feature register block
    uint64_t x_param_header;           //(optional) Parameter header
    uint8_t x_param_y;                 //(optional) Parameter data

} INTERFACE;

// Createss a DFL with specified number of interfaces for testing
static void create_single_dfl(INTERFACE DFL[], unsigned int num_interfaces)
{
    // following variables can be used to craft your own DFL
    uint64_t FeatureType;
    uint64_t DFHVersion;
    uint64_t FeatureMinorRev;
    uint64_t Reserved;
    uint64_t EOL;
    uint64_t NextDfhByteOffset;
    uint64_t FeatureRev;
    uint64_t FeatureID;

    uint64_t csr_addr;
    uint64_t csr_rel_n;

    uint64_t csr_size;
    uint64_t has_params;
    uint64_t group_id;
    uint64_t instance_id;

    uint64_t next;
    uint64_t reserved = 0;
    uint64_t eop = 0 ;
    uint64_t version;
    uint64_t param_id;

    for (unsigned int i = 0; i < (num_interfaces - 1); i++)
    {
        FeatureType = (uint64_t)0x1 << i;
        DFHVersion = (uint64_t)0x1 << i;
        FeatureMinorRev = (uint64_t)0x1 << i;
        Reserved = (uint64_t)0x1 << i;
        EOL = 0b0;
        NextDfhByteOffset = (&DFL[i + 1] - &DFL[i]) * sizeof(INTERFACE);
        FeatureRev = (uint64_t)0x1 << i;
        FeatureID = (uint64_t)0x1 << i;
        DFL[i].x_feature_dfh = CONSTRUCT_DFH(FeatureType, DFHVersion, FeatureMinorRev, Reserved, EOL, NextDfhByteOffset, FeatureRev, FeatureID);

        DFL[i].x_feature_guid_l = 0x1 << i;
        DFL[i].x_feature_guid_h = 0x1 << i;

        csr_addr = (uint64_t)0x1 << i;
        csr_rel_n = (uint64_t)0x1 << i;
        DFL[i].x_feature_csr_addr = CONSTRUCT_CSR_ADDR(csr_addr, csr_rel_n);

        csr_size = (uint64_t)0x1 << i;
        has_params = 0;
        group_id = (uint64_t)0x1 << i;
        instance_id = (uint64_t)0x1 << i;
        DFL[i].x_feature_csr_size_group = CONSTRUCT_CSR_SIZE_GROUP(csr_size, has_params, group_id, instance_id);

        next = 0;
        eop = 1;
        version = 0;
        param_id = 0;
        DFL[i].x_param_header = CONSTRUCT_PARAM_HEADER(next, reserved, eop, version, param_id);
        DFL[i].x_param_y = 0;
    }

    // set up last interface
    FeatureType = (uint64_t)0x1 << (num_interfaces - 1);
    DFHVersion = (uint64_t)0x1 << (num_interfaces - 1);
    FeatureMinorRev = (uint64_t)0x1 << (num_interfaces - 1);
    Reserved = (uint64_t)0x1 << (num_interfaces - 1);
    EOL = 0b1;
    NextDfhByteOffset = 0;
    FeatureRev = (uint64_t)0x1 << (num_interfaces - 1);
    FeatureID = (uint64_t)0x1 << (num_interfaces - 1);
    DFL[num_interfaces - 1].x_feature_dfh = CONSTRUCT_DFH(FeatureType, DFHVersion, FeatureMinorRev, Reserved, EOL, NextDfhByteOffset, FeatureRev, FeatureID);

    DFL[num_interfaces - 1].x_feature_guid_l = 0x1 << (num_interfaces - 1);
    DFL[num_interfaces - 1].x_feature_guid_h = 0x1 << (num_interfaces - 1);

    csr_addr = (uint64_t)0x1 << (num_interfaces - 1);
    csr_rel_n = (uint64_t)0x1 << (num_interfaces - 1);
    DFL[num_interfaces - 1].x_feature_csr_addr = CONSTRUCT_CSR_ADDR(csr_addr, csr_rel_n);

    csr_size = (uint64_t)0x1 << (num_interfaces - 1);
    has_params = 0;
    group_id = (uint64_t)0x1 << (num_interfaces - 1);
    instance_id = (uint64_t)0x1 << (num_interfaces - 1);
    DFL[num_interfaces - 1].x_feature_csr_size_group = CONSTRUCT_CSR_SIZE_GROUP(csr_size, has_params, group_id, instance_id);

    next = 0;
    eop = 1;
    version = (uint64_t)0x1 << (num_interfaces - 1);
    param_id = 0;
    DFL[num_interfaces - 1].x_param_header = CONSTRUCT_PARAM_HEADER(next, reserved, eop, version, param_id);
    DFL[num_interfaces - 1].x_param_y = 0;
}

static uint64_t dfl_base_addr_decoder_mock(uint64_t base_addr)
{
    return base_addr;
}

class scan_single_dfh : public ::testing::Test
{
public:
    void SetUp()
    {
    }

    void TearDown()
    {
    }

protected:
};

static void check_interface_info(INTERFACE DFL[], size_t num_interface)
{
    for (unsigned int i = 0; i < num_interface; i++)
    {
        EXPECT_EQ((uint16_t)((DFL[i].x_feature_csr_size_group >> 16) & 0x7FFF), common_fpga_interface_info_vec_at(i)->group_id) << "group id differ at i = " << i;
        EXPECT_EQ((uint16_t)((DFL[i].x_feature_csr_size_group) & 0xFFFF), common_fpga_interface_info_vec_at(i)->instance_id) << "instance id differ at i = " << i;
        EXPECT_EQ((uint64_t)(DFL[i].x_feature_guid_l), common_fpga_interface_info_vec_at(i)->guid.guid_l) << "guid_l differ at i = " << i;
        EXPECT_EQ((uint64_t)(DFL[i].x_feature_guid_h), common_fpga_interface_info_vec_at(i)->guid.guid_h) << "guid_h differ at i = " << i;
    }
}

// Testing the macro
TEST_F(scan_single_dfh, construct_dfh_all_ones)
{
    uint64_t FeatureType = 0xFFFFFF;
    uint64_t DFHVersion = 0xFFFFFFF;
    uint64_t FeatureMinorRev = 0xFFFFFF;
    uint64_t Reserved = 0xFFFFFFF;
    uint64_t EOL = 0xFFFFFFF;
    uint64_t NextDfhByteOffset = 0xFFFFFFFFFF;
    uint64_t FeatureRev = 0xFFFFFFFFF;
    uint64_t FeatureID = 0xFFFFFFF;
    EXPECT_EQ((uint64_t)0xFFFFFFFFFFFFFFFF, CONSTRUCT_DFH(FeatureType, DFHVersion, FeatureMinorRev, Reserved, EOL, NextDfhByteOffset, FeatureRev, FeatureID));
}

TEST_F(scan_single_dfh, construct_dfh_all_zeros)
{
    uint64_t FeatureType = 0x0;
    uint64_t DFHVersion = 0x0;
    uint64_t FeatureMinorRev = 0x0;
    uint64_t Reserved = 0x0;
    uint64_t EOL = 0x0;
    uint64_t NextDfhByteOffset = 0x0;
    uint64_t FeatureRev = 0x0;
    uint64_t FeatureID = 0x0;

    EXPECT_EQ((uint64_t)0x0, CONSTRUCT_DFH(FeatureType, DFHVersion, FeatureMinorRev, Reserved, EOL, NextDfhByteOffset, FeatureRev, FeatureID));
}

TEST_F(scan_single_dfh, construct_dfh_case_one)
{
    uint64_t FeatureType = 0x5;
    uint64_t DFHVersion = 0x1;
    uint64_t FeatureMinorRev = 0x0;
    uint64_t Reserved = 0b1111111;
    uint64_t EOL = 0b0;
    uint64_t NextDfhByteOffset = 0x0;
    uint64_t FeatureRev = 0x0;
    uint64_t FeatureID = 0xF01;

    EXPECT_EQ((uint64_t)0x5010FE0000000F01, CONSTRUCT_DFH(FeatureType, DFHVersion, FeatureMinorRev, Reserved, EOL, NextDfhByteOffset, FeatureRev, FeatureID));
}

TEST_F(scan_single_dfh, construct_dfh_case_two)
{
    uint64_t FeatureType = 0x3;
    uint64_t DFHVersion = 0xF1;
    uint64_t FeatureMinorRev = 0xE;
    uint64_t Reserved = 0b1111111;
    uint64_t EOL = 0b1;
    uint64_t NextDfhByteOffset = 0x100000;
    uint64_t FeatureRev = 0xF;
    uint64_t FeatureID = 0xD1;

    EXPECT_EQ((uint64_t)0x3F1EFF100000F0D1, CONSTRUCT_DFH(FeatureType, DFHVersion, FeatureMinorRev, Reserved, EOL, NextDfhByteOffset, FeatureRev, FeatureID));
}

// Testing the scan single dfh
TEST_F(scan_single_dfh, should_deal_with_one_interface)
{
    size_t num_interface = 1;
    INTERFACE DFL[num_interface];
    create_single_dfl(DFL, num_interface);

    common_dfl_scan_multi_interfaces(&DFL[0], dfl_base_addr_decoder_mock);
    EXPECT_EQ(num_interface, common_fpga_interface_info_vec_size()) << "common_fpga_interface_info_vec_size() not match";

    check_interface_info(DFL, num_interface);
}

TEST_F(scan_single_dfh, should_deal_with_two_interfaces)
{
    size_t num_interface = 2;
    INTERFACE DFL[num_interface];
    create_single_dfl(DFL, num_interface);

    common_dfl_scan_multi_interfaces(&DFL[0], dfl_base_addr_decoder_mock);
    EXPECT_EQ(num_interface, common_fpga_interface_info_vec_size()) << "common_fpga_interface_info_vec_size() not match";

    check_interface_info(DFL, num_interface);
}

TEST_F(scan_single_dfh, should_deal_with_three_interfaces)
{
    size_t num_interface = 3;
    INTERFACE DFL[num_interface];
    create_single_dfl(DFL, num_interface);

    common_dfl_scan_multi_interfaces(&DFL[0], dfl_base_addr_decoder_mock);
    EXPECT_EQ(num_interface, common_fpga_interface_info_vec_size()) << "common_fpga_interface_info_vec_size() not match";

    check_interface_info(DFL, num_interface);
}

TEST_F(scan_single_dfh, should_deal_with_ten_interfaces)
{
    size_t num_interface = 10;
    INTERFACE DFL[num_interface];
    create_single_dfl(DFL, num_interface);

    common_dfl_scan_multi_interfaces(&DFL[0], dfl_base_addr_decoder_mock);
    EXPECT_EQ(num_interface, common_fpga_interface_info_vec_size()) << "common_fpga_interface_info_vec_size() not match";

    check_interface_info(DFL, num_interface);
}
