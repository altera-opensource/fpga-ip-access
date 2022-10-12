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
#include <string.h>
using namespace std;

#include "gtest/gtest.h"

#include "intel_fpga_api_cmn_inf.h"
#include "intel_fpga_api_cmn_dfl.h"

#define NUM_INTERFACES 5
#define MAX_PARAM_BLOCK 6
#define NEXT_PARAM_OFFSET_ADJUSTER 8

#define CONSTRUCT_DFH(FeatureType_0xF, DFHVersion_0xFF, FeatureMinorRev_0xF, Reserved_0xFE, EOL_0x1, NextDfhByteOffset_0xFFFFFF, FeatureRev_0xF, FeatureID_0xFFF) ((0xF & (uint64_t)FeatureType_0xF) << 60) | ((0xFF & (uint64_t)DFHVersion_0xFF) << 52) | ((0xF & (uint64_t)FeatureMinorRev_0xF) << 48) | ((0b1111111 & (uint64_t)Reserved_0xFE) << 41) | ((0x1 & (uint64_t)EOL_0x1) << 40) | ((0xFFFFFF & (uint64_t)NextDfhByteOffset_0xFFFFFF) << 16) | ((0xF & (uint64_t)FeatureRev_0xF) << 12) | (0xFFF & (uint64_t)FeatureID_0xFFF)
#define CONSTRUCT_CSR_ADDR(csr_addr_0xFFFFFFFFFFFFFFFE, csr_rel_n_0b1) ((0xFFFFFFFFFFFFFFFE & (uint64_t)csr_addr_0xFFFFFFFFFFFFFFFE) << 1) | (0b1 & (uint64_t)csr_rel_n_0b1)
#define CONSTRUCT_CSR_SIZE_GROUP(csr_size_0xFFFFFFFF, has_params_0b1, group_id_0x7FFF, instance_id_0xFFFF) ((0xFFFFFFFFFFFF & (uint64_t)csr_size_0xFFFFFFFF) << 32) | ((0b1 & (uint64_t)has_params_0b1) << 31) | ((0x7FFF & (uint64_t)group_id_0x7FFF) << 16) | (0xFFFF & (uint64_t)instance_id_0xFFFF)
#define CONSTRUCT_PARAM_HEADER(next_0x1FFFFFFF, reserved_0b11, eop_0b1, version_0xFFFF, param_id_0xFFFF) ((0x1FFFFFFF & (uint64_t)(next_0x1FFFFFFF/NEXT_PARAM_OFFSET_ADJUSTER)) << 35) | ((0b11 & (uint64_t)reserved_0b11) << 33) | ((0b1 & (uint64_t)eop_0b1) << 32) | ((0xFFFF & (uint64_t)version_0xFFFF) << 16) | (0xFFFF & (uint64_t)param_id_0xFFFF)
#define CONSTRUCT_PARAM_DATA(dfl_addr, dfl_rel_n) (uint64_t)dfl_addr | (0b1 & (uint64_t)dfl_rel_n)

struct
{
    uint64_t level_info = 0;
    uint64_t interface_num = 0;
} guid_info;

struct
{
    // link l1 to l2
    bool absolute_0 = false;
    int absolute_0_interface_index = -1;
    int absolute_0_param_index = -1;

    bool absolute_1 = false;
    int absolute_1_interface_index = -1;
    int absolute_1_param_index = -1;

    bool relative_0 = false;
    int relative_0_interface_index = -1;
    int relative_0_param_index = -1;

    bool relative_1 = false;
    int relative_1_interface_index = -1;
    int relative_1_param_index = -1;
} link_l1_l2;

struct
{
    // link l2 to l3
    bool absolute_0 = false;
    int absolute_0_interface_index = -1;
    int absolute_0_param_index = -1;

    bool absolute_1 = false;
    int absolute_1_interface_index = -1;
    int absolute_1_param_index = -1;

    bool relative_0 = false;
    int relative_0_interface_index = -1;
    int relative_0_param_index = -1;

    bool relative_1 = false;
    int relative_1_interface_index = -1;
    int relative_1_param_index = -1;
} link_l2_l3;

struct
{
    // link l1 to l3
    bool relative = false;
    int relative_interface_index = -1;
    int relative_param_index = -1;

    bool absolute = false;
    int absolute_interface_index = -1;
    int absolute_param_index = -1;
} link_l1_l3;

struct
{
    // link l3 to l1
    bool relative = false;
    int relative_interface_index = -1;
    int relative_param_index = -1;
} link_l3_l1;

struct
{
    // link l3 to l2
    bool relative = false;
    int relative_interface_index = -1;
    int relative_param_index = -1;
} link_l3_l2;

struct
{
    // link l2 to l1
    bool relative = false;
    int relative_interface_index = -1;
    int relative_param_index = -1;
} link_l2_l1;

uint64_t mem_block[4 * 1024];

typedef struct
{
    uint64_t x_param_header = 0; //(optional) Parameter header
    uint64_t x_param_y = 0;      //(optional) Parameter data

} PARAM_BLOCK;

typedef struct
{
    uint64_t x_feature_dfh;            // Device Feature Header itself
    uint64_t x_feature_guid_l;         // Low 64-bit of GUID
    uint64_t x_feature_guid_h;         // High 64-bit of GUID
    uint64_t x_feature_csr_addr;       // Location of feature register block
    uint64_t x_feature_csr_size_group; // Location of feature register block
    PARAM_BLOCK param_block[MAX_PARAM_BLOCK];

} HIER_DFL_BLOCK;

void create_one_level_dfl(HIER_DFL_BLOCK dfl[], unsigned int num_interfaces)
{
    // following variables can be used to craft your own dfl
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

    unsigned int i = 0;
    for (; i < num_interfaces - 1; i++)
    {

        FeatureType = i;
        DFHVersion = 0;
        FeatureMinorRev = 0;
        Reserved = 0;
        EOL = 0b0;
        NextDfhByteOffset = (&dfl[i + 1] - &dfl[i]) * sizeof(HIER_DFL_BLOCK);
        FeatureRev = 0;
        FeatureID = 0;
        dfl[i].x_feature_dfh = CONSTRUCT_DFH(FeatureType, DFHVersion, FeatureMinorRev, Reserved, EOL, NextDfhByteOffset, FeatureRev, FeatureID);

        guid_info.interface_num++;
        dfl[i].x_feature_guid_l = guid_info.interface_num; // indicating the interface number
        dfl[i].x_feature_guid_h = guid_info.level_info;    // indicating the interface level

        csr_addr = 0;
        csr_rel_n = 0;
        dfl[i].x_feature_csr_addr = CONSTRUCT_CSR_ADDR(csr_addr, csr_rel_n);

        csr_size = 0;
        has_params = 1;
        group_id = 0;
        instance_id = 0;
        dfl[i].x_feature_csr_size_group = CONSTRUCT_CSR_SIZE_GROUP(csr_size, has_params, group_id, instance_id);
    }

    // set up last interface
    FeatureType = num_interfaces;
    DFHVersion = 0xF;
    FeatureMinorRev = 0xF;
    Reserved = 0xF;
    EOL = 0b1;
    NextDfhByteOffset = 0;
    FeatureRev = 0xF;
    FeatureID = 0xF;
    dfl[num_interfaces - 1].x_feature_dfh = CONSTRUCT_DFH(FeatureType, DFHVersion, FeatureMinorRev, Reserved, EOL, NextDfhByteOffset, FeatureRev, FeatureID);

    guid_info.interface_num++;
    dfl[num_interfaces - 1].x_feature_guid_l = guid_info.interface_num;
    dfl[num_interfaces - 1].x_feature_guid_h = guid_info.level_info;

    csr_addr = 0;
    csr_rel_n = 0;
    dfl[num_interfaces - 1].x_feature_csr_addr = CONSTRUCT_CSR_ADDR(csr_addr, csr_rel_n);

    csr_size = 0;
    has_params = 1;
    group_id = 0;
    instance_id = 0;
    dfl[num_interfaces - 1].x_feature_csr_size_group = CONSTRUCT_CSR_SIZE_GROUP(csr_size, has_params, group_id, instance_id);
}

int get_l1_param_id(int i, int j)
{
    int param_id = -1;

    if (link_l1_l2.absolute_0 && i == link_l1_l2.absolute_0_interface_index && j == link_l1_l2.absolute_0_param_index)
    {
        param_id = 0xc;
    }
    else if (link_l1_l2.absolute_1 && i == link_l1_l2.absolute_1_interface_index && j == link_l1_l2.absolute_1_param_index)
    {
        param_id = 0xc;
    }
    else if (link_l1_l2.relative_0 && i == link_l1_l2.relative_0_interface_index && j == link_l1_l2.relative_0_param_index)
    {
        param_id = 0xc;
    }
    else if (link_l1_l2.relative_1 && i == link_l1_l2.relative_1_interface_index && j == link_l1_l2.relative_1_param_index)
    {
        param_id = 0xc;
    }
    else if (link_l1_l3.absolute && i == link_l1_l3.absolute_interface_index && j == link_l1_l3.absolute_param_index)
    {
        param_id = 0xc;
    }
    else if (link_l1_l3.relative && i == link_l1_l3.relative_interface_index && j == link_l1_l3.relative_param_index)
    {
        param_id = 0xc;
    }
    else
    {
        param_id = 1;
    }

    return param_id;
}

int get_l2_param_id(int i, int j)
{
    int param_id = -1;
    if (link_l2_l3.absolute_0 && i == link_l2_l3.absolute_0_interface_index && j == link_l2_l3.absolute_0_param_index)
    {
        param_id = 0xc;
    }
    else if (link_l2_l3.absolute_1 && i == link_l2_l3.absolute_1_interface_index && j == link_l2_l3.absolute_1_param_index)
    {
        param_id = 0xc;
    }
    else if (link_l2_l3.relative_0 && i == link_l2_l3.relative_0_interface_index && j == link_l2_l3.relative_0_param_index)
    {
        param_id = 0xc;
    }
    else if (link_l2_l3.relative_1 && i == link_l2_l3.relative_1_interface_index && j == link_l2_l3.relative_1_param_index)
    {
        param_id = 0xc;
    }
    else if (link_l2_l1.relative && i == link_l2_l1.relative_interface_index && j == link_l2_l1.relative_param_index)
    {
        param_id = 0xc;
    }
    else
    {
        param_id = 1;
    }

    return param_id;
}

int get_l3_param_id(int i, int j)
{
    int param_id = -1;
    if (link_l3_l2.relative && i == link_l3_l2.relative_interface_index && j == link_l3_l2.relative_param_index)
    {
        param_id = 0xc;
    }
    else if (link_l3_l1.relative && i == link_l3_l1.relative_interface_index && j == link_l3_l1.relative_param_index)
    {
        param_id = 0xc;
    }
    else
    {
        param_id = 1;
    }

    return param_id;
}

void create_hier_dfl(void *mem_block_start_addr, int num_interface_per_level)
{
    // create three level dfl
    HIER_DFL_BLOCK level_one[num_interface_per_level];
    guid_info.level_info++;
    create_one_level_dfl(level_one, num_interface_per_level);
    HIER_DFL_BLOCK level_two[num_interface_per_level];
    guid_info.level_info++;
    create_one_level_dfl(level_two, num_interface_per_level);
    HIER_DFL_BLOCK level_three[num_interface_per_level];
    guid_info.level_info++;
    create_one_level_dfl(level_three, num_interface_per_level);

    // initialize the param blocks
    uint64_t next = 0;
    uint64_t reserved = 0;
    uint64_t eop = 0;
    uint64_t version = 0;
    uint64_t param_id = 0;

    // initialize all the param_block at level_one
    int param_block_num = 0;
    for (int i = 0; i < num_interface_per_level; i++)
    {
        for (int j = 0; j < MAX_PARAM_BLOCK - 1; j++)
        {
            next = ((&(level_one[i].param_block[j + 1])) - (&(level_one[i].param_block[j]))) * sizeof(PARAM_BLOCK);
            param_block_num++;
            version = param_block_num;
            eop = 0;
            // branch to next level based on relative address or absolute address
            param_id = get_l1_param_id(i, j);
            level_one[i].param_block[j].x_param_header = CONSTRUCT_PARAM_HEADER(next, reserved, eop, version, param_id);
        }
        // param tail
        next = 0;
        eop = 1;
        param_block_num++;
        version = param_block_num;
        param_id = 0;

        level_one[i].param_block[MAX_PARAM_BLOCK - 1].x_param_header = CONSTRUCT_PARAM_HEADER(next, reserved, eop, version, param_id);
    }

    // initialize all the param_block at level_two
    for (int i = 0; i < num_interface_per_level; i++)
    {
        for (int j = 0; j < MAX_PARAM_BLOCK - 1; j++)
        {
            next = ((&(level_two[i].param_block[j + 1])) - (&(level_two[i].param_block[j]))) * sizeof(PARAM_BLOCK);
            param_block_num++;
            version = param_block_num;
            eop = 0;
            // branch to next level based on relative address or absolute address
            param_id = get_l2_param_id(i, j);

            level_two[i].param_block[j].x_param_header = CONSTRUCT_PARAM_HEADER(next, reserved, eop, version, param_id);
        }

        next = 0;
        eop = 1;
        param_block_num++;
        version = param_block_num;
        param_id = 0;

        level_two[i].param_block[MAX_PARAM_BLOCK - 1].x_param_header = CONSTRUCT_PARAM_HEADER(next, reserved, eop, version, param_id);
    }

    // initialize all the param_block at level_three
    for (int i = 0; i < num_interface_per_level; i++)
    {
        for (int j = 0; j < MAX_PARAM_BLOCK - 1; j++)
        {
            next = ((&(level_three[i].param_block[j + 1])) - (&(level_three[i].param_block[j]))) * sizeof(PARAM_BLOCK);
            param_block_num++;
            eop = 0;
            version = param_block_num;
            param_id = get_l3_param_id(i, j);

            level_three[i].param_block[j].x_param_header = CONSTRUCT_PARAM_HEADER(next, reserved, eop, version, param_id);
        }
        next = 0;
        eop = 1;
        param_block_num++;
        version = param_block_num;
        param_id = 0;

        level_three[i].param_block[MAX_PARAM_BLOCK - 1].x_param_header = CONSTRUCT_PARAM_HEADER(next, reserved, eop, version, param_id);
    }

    // set up branch
    uint64_t dfl_addr;
    uint64_t dfl_rel_n;
    // level one branch
    if (link_l1_l2.absolute_0)
    {
        dfl_addr = (uint64_t)((uint64_t *)mem_block_start_addr + sizeof(level_one) * 8 / 64);
        dfl_rel_n = 1;
        level_one[link_l1_l2.absolute_0_interface_index].param_block[link_l1_l2.absolute_0_param_index].x_param_y = CONSTRUCT_PARAM_DATA(dfl_addr, dfl_rel_n);
        level_one[link_l1_l2.absolute_0_interface_index].x_feature_guid_h = 0x30c45aea68f642e6;
        level_one[link_l1_l2.absolute_0_interface_index].x_feature_guid_l = 0xaefb15b4b5e28284;
    }

    if (link_l1_l2.absolute_1)
    {
        dfl_addr = (uint64_t)((uint64_t *)mem_block_start_addr + sizeof(level_one) * 8 / 64);
        dfl_rel_n = 1;
        level_one[link_l1_l2.absolute_1_interface_index].param_block[link_l1_l2.absolute_1_param_index].x_param_y = CONSTRUCT_PARAM_DATA(dfl_addr, dfl_rel_n);
        level_one[link_l1_l2.absolute_1_interface_index].x_feature_guid_h = 0x30c45aea68f642e6;
        level_one[link_l1_l2.absolute_1_interface_index].x_feature_guid_l = 0xaefb15b4b5e28284;
    }

    if (link_l1_l2.relative_0)
    {
        int64_t signed_dfl_addr = sizeof(level_one) - sizeof(HIER_DFL_BLOCK) * link_l1_l2.relative_0_interface_index;
        dfl_rel_n = 0;
        level_one[link_l1_l2.relative_0_interface_index].param_block[link_l1_l2.relative_0_param_index].x_param_y = CONSTRUCT_PARAM_DATA(signed_dfl_addr, dfl_rel_n);
        level_one[link_l1_l2.relative_0_interface_index].x_feature_guid_h = 0x30c45aea68f642e6;
        level_one[link_l1_l2.relative_0_interface_index].x_feature_guid_l = 0xaefb15b4b5e28284;
    }

    if (link_l1_l2.relative_1)
    {
        int64_t signed_dfl_addr = sizeof(level_one) - sizeof(HIER_DFL_BLOCK) * link_l1_l2.relative_1_interface_index;
        dfl_rel_n = 0;
        level_one[link_l1_l2.relative_1_interface_index].param_block[link_l1_l2.relative_1_param_index].x_param_y = CONSTRUCT_PARAM_DATA(signed_dfl_addr, dfl_rel_n);
        level_one[link_l1_l2.relative_1_interface_index].x_feature_guid_h = 0x30c45aea68f642e6;
        level_one[link_l1_l2.relative_1_interface_index].x_feature_guid_l = 0xaefb15b4b5e28284;
    }

    if (link_l1_l3.absolute)
    {
        dfl_addr = (uint64_t)((uint64_t *)mem_block_start_addr + sizeof(level_one) * 8 / 64 + sizeof(level_two) * 8 / 64);
        dfl_rel_n = 1;
        level_one[link_l1_l3.relative_interface_index].param_block[link_l1_l3.relative_param_index].x_param_y = CONSTRUCT_PARAM_DATA(dfl_addr, dfl_rel_n);
        level_one[link_l1_l3.relative_interface_index].x_feature_guid_h = 0x30c45aea68f642e6;
        level_one[link_l1_l3.relative_interface_index].x_feature_guid_l = 0xaefb15b4b5e28284;
    }

    if (link_l1_l3.relative)
    {
        int64_t signed_dfl_addr = sizeof(level_one) + sizeof(level_two) - sizeof(HIER_DFL_BLOCK) * link_l1_l3.relative_interface_index;
        dfl_rel_n = 0;
        level_one[link_l1_l3.relative_interface_index].param_block[link_l1_l3.relative_param_index].x_param_y = CONSTRUCT_PARAM_DATA(signed_dfl_addr, dfl_rel_n);
        level_one[link_l1_l3.relative_interface_index].x_feature_guid_h = 0x30c45aea68f642e6;
        level_one[link_l1_l3.relative_interface_index].x_feature_guid_l = 0xaefb15b4b5e28284;
    }

    // level two branch
    if (link_l2_l3.absolute_0)
    {
        dfl_addr = (uint64_t)((uint64_t *)mem_block_start_addr + sizeof(level_one) * 8 / 64 + sizeof(level_two) * 8 / 64);
        dfl_rel_n = 1;
        level_two[link_l2_l3.absolute_0_interface_index].param_block[link_l2_l3.absolute_0_param_index].x_param_y = CONSTRUCT_PARAM_DATA(dfl_addr, dfl_rel_n);
        level_two[link_l2_l3.absolute_0_interface_index].x_feature_guid_h = 0x30c45aea68f642e6;
        level_two[link_l2_l3.absolute_0_interface_index].x_feature_guid_l = 0xaefb15b4b5e28284;
    }

    if (link_l2_l3.absolute_1)
    {
        dfl_addr = (uint64_t)((uint64_t *)mem_block_start_addr + sizeof(level_one) * 8 / 64 + sizeof(level_two) * 8 / 64);
        dfl_rel_n = 1;
        level_two[link_l2_l3.absolute_1_interface_index].param_block[link_l2_l3.absolute_1_param_index].x_param_y = CONSTRUCT_PARAM_DATA(dfl_addr, dfl_rel_n);
        level_two[link_l2_l3.absolute_1_interface_index].x_feature_guid_h = 0x30c45aea68f642e6;
        level_two[link_l2_l3.absolute_1_interface_index].x_feature_guid_l = 0xaefb15b4b5e28284;
    }

    if (link_l2_l3.relative_0)
    {
        int64_t signed_dfl_addr = sizeof(level_two) - sizeof(HIER_DFL_BLOCK) * link_l2_l3.relative_0_interface_index;
        dfl_rel_n = 0;
        level_two[link_l2_l3.relative_0_interface_index].param_block[link_l2_l3.relative_0_param_index].x_param_y = CONSTRUCT_PARAM_DATA(signed_dfl_addr, dfl_rel_n);
        level_two[link_l2_l3.relative_0_interface_index].x_feature_guid_h = 0x30c45aea68f642e6;
        level_two[link_l2_l3.relative_0_interface_index].x_feature_guid_l = 0xaefb15b4b5e28284;
    }

    if (link_l2_l3.relative_1)
    {
        int64_t signed_dfl_addr = sizeof(level_two) - sizeof(HIER_DFL_BLOCK) * link_l2_l3.relative_1_interface_index;
        dfl_rel_n = 0;
        level_two[link_l2_l3.relative_1_interface_index].param_block[link_l2_l3.relative_1_param_index].x_param_y = CONSTRUCT_PARAM_DATA(signed_dfl_addr, dfl_rel_n);
        level_two[link_l2_l3.relative_1_interface_index].x_feature_guid_h = 0x30c45aea68f642e6;
        level_two[link_l2_l3.relative_1_interface_index].x_feature_guid_l = 0xaefb15b4b5e28284;
    }

    if (link_l2_l1.relative)
    {
        int64_t signed_dfl_addr = (-1) * sizeof(level_one) - sizeof(HIER_DFL_BLOCK) * link_l2_l1.relative_interface_index;
        dfl_rel_n = 0;
        level_two[link_l2_l1.relative_interface_index].param_block[link_l2_l1.relative_param_index].x_param_y = CONSTRUCT_PARAM_DATA(signed_dfl_addr, dfl_rel_n);
        level_two[link_l2_l1.relative_interface_index].x_feature_guid_h = 0x30c45aea68f642e6;
        level_two[link_l2_l1.relative_interface_index].x_feature_guid_l = 0xaefb15b4b5e28284;
    }

    // level three branch
    if (link_l3_l2.relative)
    {
        int64_t signed_dfl_addr = (-1) * sizeof(level_two) - sizeof(HIER_DFL_BLOCK) * link_l3_l2.relative_interface_index;
        dfl_rel_n = 0;
        level_three[link_l3_l2.relative_interface_index].param_block[link_l3_l2.relative_param_index].x_param_y = CONSTRUCT_PARAM_DATA(signed_dfl_addr, dfl_rel_n);
        level_three[link_l3_l2.relative_interface_index].x_feature_guid_h = 0x30c45aea68f642e6;
        level_three[link_l3_l2.relative_interface_index].x_feature_guid_l = 0xaefb15b4b5e28284;
    }

    if (link_l3_l1.relative)
    {
        int64_t signed_dfl_addr = (-1) * (sizeof(level_two) + sizeof(level_one)) - sizeof(HIER_DFL_BLOCK) * link_l3_l1.relative_interface_index;
        dfl_rel_n = 0;
        level_three[link_l3_l1.relative_interface_index].param_block[link_l3_l1.relative_param_index].x_param_y = CONSTRUCT_PARAM_DATA(signed_dfl_addr, dfl_rel_n);
        level_three[link_l3_l1.relative_interface_index].x_feature_guid_h = 0x30c45aea68f642e6;
        level_three[link_l3_l1.relative_interface_index].x_feature_guid_l = 0xaefb15b4b5e28284;
    }

    // copy the hier dfl to the mem_block
    memcpy((uint64_t *)mem_block_start_addr, level_one, sizeof(level_one));
    memcpy((uint64_t *)mem_block_start_addr + sizeof(level_one) * 8 / 64, level_two, sizeof(level_two));
    memcpy(((uint64_t *)mem_block_start_addr + sizeof(level_two) * 8 / 64 + sizeof(level_one) * 8 / 64), level_three, sizeof(level_three));

    return;
}

static uint64_t dfl_base_addr_decoder_mock(uint64_t base_addr)
{
    return base_addr;
}

class scan_hier_dfl : public ::testing::Test
{
public:
    void SetUp()
    {

        // link l1 to l2
        link_l1_l2.absolute_0 = false;
        link_l1_l2.absolute_0_interface_index = -1;
        link_l1_l2.absolute_0_param_index = -1;

        link_l1_l2.absolute_1 = false;
        link_l1_l2.absolute_1_interface_index = -1;
        link_l1_l2.absolute_1_param_index = -1;

        link_l1_l2.relative_0 = false;
        link_l1_l2.relative_0_interface_index = -1;
        link_l1_l2.relative_0_param_index = -1;

        link_l1_l2.relative_1 = false;
        link_l1_l2.relative_1_interface_index = -1;
        link_l1_l2.relative_1_param_index = -1;

        // link l2 to l3
        link_l2_l3.absolute_0 = false;
        link_l2_l3.absolute_0_interface_index = -1;
        link_l2_l3.absolute_0_param_index = -1;

        link_l2_l3.absolute_1 = false;
        link_l2_l3.absolute_1_interface_index = -1;
        link_l2_l3.absolute_1_param_index = -1;

        link_l2_l3.relative_0 = false;
        link_l2_l3.relative_0_interface_index = -1;
        link_l2_l3.relative_0_param_index = -1;

        link_l2_l3.relative_1 = false;
        link_l2_l3.relative_1_interface_index = -1;
        link_l2_l3.relative_1_param_index = -1;

        // link l1 to l3
        link_l1_l3.relative = false;
        link_l1_l3.relative_interface_index = -1;
        link_l1_l3.relative_param_index = -1;

        link_l1_l3.absolute = false;
        link_l1_l3.absolute_interface_index = -1;
        link_l1_l3.absolute_param_index = -1;

        // link l3 to l1
        link_l3_l1.relative = false;
        link_l3_l1.relative_interface_index = -1;
        link_l3_l1.relative_param_index = -1;

        // link l3 to l2
        link_l3_l2.relative = false;
        link_l3_l2.relative_interface_index = -1;
        link_l3_l2.relative_param_index = -1;

        // link l2 to l1
        link_l2_l1.relative = false;
        link_l2_l1.relative_interface_index = -1;
        link_l2_l1.relative_param_index = -1;
    }

    void TearDown()
    {
    }

protected:
};

// TEST USAGE:
// the top level is default to level 1
// the dfh address passed to scan_hier_dfl() is the begin address of the mem_block in ROM
// currently three level are created in ROM: l1, l2, l3
// each level has five interfaces
// under each interface there are five param_block
// each param_block can branch to any levels, except the current level
// to create a branch: first set the 'link' to 'true' then specify the 'interface_index' and 'param_index' to satrt branch

/* general structure to create a link between levels:
    link_l<a>_l<b>_relative = true;
    link_l<a>_l<b>_relative_interface_index = <i>;
    link_l<a>_l<b>_relative_param_index = <j>;

    a = 1, 2, 3;
    b = 1, 2, 3;
    a != b;
    i = 0, 1, 2, 3, 4;
    j = 0, 1, 2, 3, 4;

    Note:
    1. 'relative' can be replaced by 'absolute' to specify abosulte address
    2. two branches can be created at same time from l1 to l2: link_l1_l2_relative_0 and link_l1_l2_relative_1 (link_l1_l2_absolute_0 and link_l1_l2_absolute_1)
    3. same for l2 to l3: link_l2_l3_relative_0 and link_l2_l3_relative_1 (link_l2_l3_absolute_0 and link_l2_l3_absolute_1)
*/

/* example of branch from level 1 to level 2 at interface 3 and param_block 4 using absolute address is:

    link_l1_l2_absolute_0 = true;
    link_l1_l2_absolute_0_interface_index = 3;
    link_l1_l2_absolute_0_param_index = 4;
*/

/* example of branch from level 2 to level 1 at interface 3 and param_block 4 using relative address is:

    link_l2_l1_relative_0 = true;
    link_l2_l1_relative_0_interface_index = 3;
    link_l2_l1_relative_0_param_index = 4;
*/

// l1 -> l2; signed positive, first param
TEST_F(scan_hier_dfl, should_deal_with_l1_to_l2_signed_positive_first_param)
{
    link_l1_l2.relative_0 = true;
    link_l1_l2.relative_0_interface_index = 0;
    link_l1_l2.relative_0_param_index = 0;

    create_hier_dfl(mem_block, NUM_INTERFACES);
    common_dfl_scan_multi_interfaces(mem_block, dfl_base_addr_decoder_mock);
    EXPECT_EQ((size_t)10, common_fpga_interface_info_vec_size());

    for (size_t i = 0; i < common_fpga_interface_info_vec_size(); i++)
    {
        if (i >= 1 && i <= 5)
        {
            EXPECT_EQ(0, common_fpga_interface_info_vec_at(i)->dfh_parent) << "differ at index " << i;
        }
        else
        {
            EXPECT_EQ(-1, common_fpga_interface_info_vec_at(i)->dfh_parent) << "differ at index " << i;
        }
    }
}

// l1 -> l2; signed positive, second param
TEST_F(scan_hier_dfl, should_deal_with_l1_to_l2_signed_positive_second_param)
{
    link_l1_l2.relative_0 = true;
    link_l1_l2.relative_0_interface_index = 0;
    link_l1_l2.relative_0_param_index = 1;

    create_hier_dfl(mem_block, NUM_INTERFACES);
    common_dfl_scan_multi_interfaces(mem_block, dfl_base_addr_decoder_mock);
    EXPECT_EQ((size_t)10, common_fpga_interface_info_vec_size());

    for (size_t i = 0; i < common_fpga_interface_info_vec_size(); i++)
    {
        if (i >= 1 && i <= 5)
        {
            EXPECT_EQ(0, common_fpga_interface_info_vec_at(i)->dfh_parent) << "differ at index " << i;
        }
        else
        {
            EXPECT_EQ(-1, common_fpga_interface_info_vec_at(i)->dfh_parent) << "differ at index " << i;
        }
    }
}

// l1 - > l3; signed positive first param
TEST_F(scan_hier_dfl, should_deal_with_l1_to_l3_signed_positive_first_param)
{
    link_l1_l3.relative = true;
    link_l1_l3.relative_interface_index = 0;
    link_l1_l3.relative_param_index = 0;

    create_hier_dfl(mem_block, NUM_INTERFACES);
    common_dfl_scan_multi_interfaces(mem_block, dfl_base_addr_decoder_mock);
    EXPECT_EQ((size_t)10, common_fpga_interface_info_vec_size());
}

// l1 - > l3; signed positive second param
TEST_F(scan_hier_dfl, should_deal_with_l1_to_l3_signed_positive_second_param)
{
    link_l1_l3.relative = true;
    link_l1_l3.relative_interface_index = 0;
    link_l1_l3.relative_param_index = 1;

    create_hier_dfl(mem_block, NUM_INTERFACES);
    common_dfl_scan_multi_interfaces(mem_block, dfl_base_addr_decoder_mock);
    EXPECT_EQ((size_t)10, common_fpga_interface_info_vec_size());
}

// l1 -> l2 -> l3; signed positive first param
TEST_F(scan_hier_dfl, should_deal_with_l1_to_l2_to_l3_signed_positive_first_param)
{
    link_l1_l2.relative_0 = true;
    link_l1_l2.relative_0_interface_index = 0;
    link_l1_l2.relative_0_param_index = 0;

    link_l2_l3.relative_0 = true;
    link_l2_l3.relative_0_interface_index = 0;
    link_l2_l3.relative_0_param_index = 0;

    create_hier_dfl(mem_block, NUM_INTERFACES);
    common_dfl_scan_multi_interfaces(mem_block, dfl_base_addr_decoder_mock);
    EXPECT_EQ((size_t)15, common_fpga_interface_info_vec_size());

    for (size_t i = 0; i < common_fpga_interface_info_vec_size(); i++)
    {
        if (i >= 2 && i <= 6)
        {
            EXPECT_EQ(1, common_fpga_interface_info_vec_at(i)->dfh_parent) << "differ at index " << i;
        }
        else if ((i >= 7 && i <= 10) || i == 1)
        {
            EXPECT_EQ(0, common_fpga_interface_info_vec_at(i)->dfh_parent) << "differ at index " << i;
        }
        else
        {
            EXPECT_EQ(-1, common_fpga_interface_info_vec_at(i)->dfh_parent) << "differ at index " << i;
        }
    }
}

// l1 -> l2 -> l3; signed positive second param
TEST_F(scan_hier_dfl, should_deal_with_l1_to_l2_to_l3_signed_positive_second_param)
{
    link_l1_l2.relative_0 = true;
    link_l1_l2.relative_0_interface_index = 0;
    link_l1_l2.relative_0_param_index = 1;

    link_l2_l3.relative_0 = true;
    link_l2_l3.relative_0_interface_index = 0;
    link_l2_l3.relative_0_param_index = 1;

    create_hier_dfl(mem_block, NUM_INTERFACES);
    common_dfl_scan_multi_interfaces(mem_block, dfl_base_addr_decoder_mock);
    EXPECT_EQ((size_t)15, common_fpga_interface_info_vec_size());
}

// l1 -> l2 -> l3; unsigned first param
TEST_F(scan_hier_dfl, should_deal_with_l1_to_l2_to_l3_unsigned_first_param)
{
    link_l1_l2.absolute_0 = true;
    link_l1_l2.absolute_0_interface_index = 0;
    link_l1_l2.absolute_0_param_index = 0;

    link_l2_l3.absolute_0 = true;
    link_l2_l3.absolute_0_interface_index = 0;
    link_l2_l3.absolute_0_param_index = 0;

    create_hier_dfl(mem_block, NUM_INTERFACES);
    common_dfl_scan_multi_interfaces(mem_block, dfl_base_addr_decoder_mock);
    EXPECT_EQ((size_t)15, common_fpga_interface_info_vec_size());
}

// l1 -> l2 -> l3; unsigned second param
TEST_F(scan_hier_dfl, should_deal_with_l1_to_l2_to_l3_unsigned_second_param)
{
    link_l1_l2.absolute_0 = true;
    link_l1_l2.absolute_0_interface_index = 0;
    link_l1_l2.absolute_0_param_index = 1;

    link_l2_l3.absolute_0 = true;
    link_l2_l3.absolute_0_interface_index = 0;
    link_l2_l3.absolute_0_param_index = 1;

    create_hier_dfl(mem_block, NUM_INTERFACES);
    common_dfl_scan_multi_interfaces(mem_block, dfl_base_addr_decoder_mock);
    EXPECT_EQ((size_t)15, common_fpga_interface_info_vec_size());
}

// l1 -> l3 - > l2; signed negative first param
TEST_F(scan_hier_dfl, should_deal_with_l1_to_l2_to_l3_signed_negative_first_param)
{
    link_l1_l3.relative = true;
    link_l1_l3.relative_interface_index = 0;
    link_l1_l3.relative_param_index = 0;

    link_l3_l2.relative = true;
    link_l3_l2.relative_interface_index = 0;
    link_l3_l2.relative_param_index = 0;

    create_hier_dfl(mem_block, NUM_INTERFACES);
    common_dfl_scan_multi_interfaces(mem_block, dfl_base_addr_decoder_mock);
    EXPECT_EQ((size_t)15, common_fpga_interface_info_vec_size());
}

// l1 -> l3 - > l2; signed negative second param
TEST_F(scan_hier_dfl, should_deal_with_l1_to_l2_to_l3_signed_negative_second_param)
{
    link_l1_l3.relative = true;
    link_l1_l3.relative_interface_index = 0;
    link_l1_l3.relative_param_index = 1;

    link_l3_l2.relative = true;
    link_l3_l2.relative_interface_index = 4;
    link_l3_l2.relative_param_index = 1;

    create_hier_dfl(mem_block, NUM_INTERFACES);
    common_dfl_scan_multi_interfaces(mem_block, dfl_base_addr_decoder_mock);
    EXPECT_EQ((size_t)15, common_fpga_interface_info_vec_size());

    for (size_t i = 0; i < common_fpga_interface_info_vec_size(); i++)
    {
        if (i >= 1 && i <= 5)
        {
            EXPECT_EQ(0, common_fpga_interface_info_vec_at(i)->dfh_parent) << "differ at index " << i;
        }
        else if ((i >= 6 && i <= 10) || i == 1)
        {
            EXPECT_EQ(5, common_fpga_interface_info_vec_at(i)->dfh_parent) << "differ at index " << i;
        }
        else
        {
            EXPECT_EQ(-1, common_fpga_interface_info_vec_at(i)->dfh_parent) << "differ at index " << i;
        }
    }
}

// l1 -> l2 -> l3; unsigned all param
TEST_F(scan_hier_dfl, should_deal_with_l1_to_l2_to_l3_all_param)
{
    for (int level_one_interface = 0; level_one_interface < NUM_INTERFACES; level_one_interface++)
    {
        for (int level_one_param = 0; level_one_param < MAX_PARAM_BLOCK - 1; level_one_param++)
        {
            for (int level_two_interface = 0; level_two_interface < NUM_INTERFACES; level_two_interface++)
            {
                for (int level_two_param = 0; level_two_param < MAX_PARAM_BLOCK - 1; level_two_param++)
                {

                    link_l1_l2.absolute_0 = true;
                    link_l1_l2.absolute_0_interface_index = level_one_interface;
                    link_l1_l2.absolute_0_param_index = level_one_param;

                    link_l2_l3.absolute_0 = true;
                    link_l2_l3.absolute_0_interface_index = level_two_interface;
                    link_l2_l3.absolute_0_param_index = level_two_param;
                    create_hier_dfl(mem_block, 5);
                    common_dfl_scan_multi_interfaces(mem_block, dfl_base_addr_decoder_mock);
                    EXPECT_EQ((size_t)15, common_fpga_interface_info_vec_size()) << "FAILED: at level_one_interface: " << level_one_interface << "; level_one_param: " << level_one_param << "; level_two_interface: " << level_two_interface << "; level_two_param: " << level_two_param;
                }
            }
        }
    }
}

// two branch from l1 -> l2; signed positive; first and second param
TEST_F(scan_hier_dfl, should_deal_with_two_branches_from_l1_to_l2_signed_positive_address)
{
    link_l1_l2.relative_0 = true;
    link_l1_l2.relative_0_interface_index = 0;
    link_l1_l2.relative_0_param_index = 0;

    link_l1_l2.relative_1 = true;
    link_l1_l2.relative_1_interface_index = 3;
    link_l1_l2.relative_1_param_index = 2;

    create_hier_dfl(mem_block, 5);
    common_dfl_scan_multi_interfaces(mem_block, dfl_base_addr_decoder_mock);
    EXPECT_EQ((size_t)15, common_fpga_interface_info_vec_size());
}

// one branch from l1 -> l2; two branch from l2 -> l3; absolute; first and second param
TEST_F(scan_hier_dfl, should_deal_with_one_branches_from_l1_to_l2_two_branch_from_l2_l3_absolute_address)
{
    link_l1_l2.absolute_0 = true;
    link_l1_l2.absolute_0_interface_index = 0;
    link_l1_l2.absolute_0_param_index = 0;

    link_l2_l3.absolute_0 = true;
    link_l2_l3.absolute_0_interface_index = 0;
    link_l2_l3.absolute_0_param_index = 0;

    link_l2_l3.absolute_1 = true;
    link_l2_l3.absolute_1_interface_index = 1;
    link_l2_l3.absolute_1_param_index = 1;

    create_hier_dfl(mem_block, 5);
    common_dfl_scan_multi_interfaces(mem_block, dfl_base_addr_decoder_mock);
    EXPECT_EQ((size_t)20, common_fpga_interface_info_vec_size());
}

// two branch from l1 to l2 and one branch from l2 to l3 using absolute address
TEST_F(scan_hier_dfl, should_deal_with_two_branches_from_l1_to_l2_and_one_branch_from_l2_to_l3_using_absolute_address)
{
    link_l1_l2.absolute_0 = true;
    link_l1_l2.absolute_0_interface_index = 0;
    link_l1_l2.absolute_0_param_index = 0;

    link_l1_l2.absolute_1 = true;
    link_l1_l2.absolute_1_interface_index = 1;
    link_l1_l2.absolute_1_param_index = 1;

    link_l2_l3.absolute_0 = true;
    link_l2_l3.absolute_0_interface_index = 1;
    link_l2_l3.absolute_0_param_index = 1;

    create_hier_dfl(mem_block, 5);
    common_dfl_scan_multi_interfaces(mem_block, dfl_base_addr_decoder_mock);
    EXPECT_EQ((size_t)25, common_fpga_interface_info_vec_size());
}

// two branches from l1 to l2 and l2 to l3 using absolute address
TEST_F(scan_hier_dfl, should_deal_with_two_branches_from_l1_to_l2_and_two_branch_from_l2_to_l3_using_absolute_address)
{
    link_l1_l2.absolute_0 = true;
    link_l1_l2.absolute_0_interface_index = 0;
    link_l1_l2.absolute_0_param_index = 0;

    link_l1_l2.absolute_1 = true;
    link_l1_l2.absolute_1_interface_index = 1;
    link_l1_l2.absolute_1_param_index = 1;

    link_l2_l3.absolute_0 = true;
    link_l2_l3.absolute_0_interface_index = 1;
    link_l2_l3.absolute_0_param_index = 1;

    link_l2_l3.absolute_1 = true;
    link_l2_l3.absolute_1_interface_index = 2;
    link_l2_l3.absolute_1_param_index = 2;

    create_hier_dfl(mem_block, 5);
    common_dfl_scan_multi_interfaces(mem_block, dfl_base_addr_decoder_mock);
    EXPECT_EQ((size_t)35, common_fpga_interface_info_vec_size());
}

// test cases for base address
void *get_base_address(void *current_dfh_address, uint64_t csr_addr)
{
    bool is_absolute_addr = 0b1 & csr_addr;

    if (is_absolute_addr)
    {
        return (void *)(csr_addr & 0xFFFFFFFFFFFFFFFE);
    }

    return (void *)((uint64_t)current_dfh_address + (int64_t)csr_addr);
}

TEST_F(scan_hier_dfl, should_deal_with_absolute_csr_addr)
{
    uint64_t csr = 0x81;
    void *current_dfh_address = (void *)0x7ffcd0b8b760;
    EXPECT_EQ((uint64_t)0x80, (uint64_t)get_base_address(current_dfh_address, csr));
}

TEST_F(scan_hier_dfl, should_deal_with_negative_csr_addr)
{
    uint64_t csr = 0x8000000000000080;
    void *current_dfh_address = (void *)0x7ffcd0b8b760;
    EXPECT_EQ((uint64_t)0x80007ffcd0b8b7e0, (uint64_t)get_base_address(current_dfh_address, csr));
}

TEST_F(scan_hier_dfl, should_deal_with_positive_csr_addr)
{
    uint64_t csr = 0x80;
    void *current_dfh_address = (void *)0x7ffcd0b8b760;
    EXPECT_EQ((uint64_t)0x7ffcd0b8b7e0, (uint64_t)get_base_address(current_dfh_address, csr));
}
