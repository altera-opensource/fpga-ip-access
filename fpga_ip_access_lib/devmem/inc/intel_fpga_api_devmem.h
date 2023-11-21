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

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include "intel_fpga_platform_devmem.h"
#include "intel_fpga_api_cmn_inf.h"


#ifdef __cplusplus
extern "C" {
#endif

static inline void *fpga_devmem_get_base_address(FPGA_MMIO_INTERFACE_HANDLE handle)
{
    return common_fpga_interface_info_vec_at(handle)->base_address;
}

static inline uint8_t fpga_read_8(FPGA_MMIO_INTERFACE_HANDLE handle, uint32_t offset)
{
    return *((volatile uint8_t *)fpga_devmem_get_base_address(handle) + offset);
}

static inline void fpga_write_8(FPGA_MMIO_INTERFACE_HANDLE handle, uint32_t offset, uint8_t value)
{
    *((volatile uint8_t *)fpga_devmem_get_base_address(handle) + offset) = value;
}

static inline uint16_t fpga_read_16(FPGA_MMIO_INTERFACE_HANDLE handle, uint32_t offset)
{
    return *((volatile uint16_t *)((volatile uint8_t *)fpga_devmem_get_base_address(handle) + offset));
}

static inline void fpga_write_16(FPGA_MMIO_INTERFACE_HANDLE handle, uint32_t offset, uint16_t value)
{
    *((volatile uint16_t *)((volatile uint8_t *)fpga_devmem_get_base_address(handle) + offset)) = value;
}

static inline uint32_t fpga_read_32(FPGA_MMIO_INTERFACE_HANDLE handle, uint32_t offset)
{
    return *((volatile uint32_t *)((volatile uint8_t *)fpga_devmem_get_base_address(handle) + offset));
}

static inline void fpga_write_32(FPGA_MMIO_INTERFACE_HANDLE handle, uint32_t offset, uint32_t value)
{
    *((volatile uint32_t *)((volatile uint8_t *)fpga_devmem_get_base_address(handle) + offset)) = value;
}

static inline uint64_t fpga_read_64(FPGA_MMIO_INTERFACE_HANDLE handle, uint32_t offset)
{
#ifndef FPGA_PLATFORM_FORCE_64BIT_MMIO_EMULATION_WITH_32BIT
    return *((volatile uint64_t *)((volatile uint8_t *)fpga_devmem_get_base_address(handle) + offset));
#else
    // This emulation is needed when Intel FPGA PCIe Memory Mapped Bridge IP is used to implement the PCIe function.
    // Little-endian system is assumed.
    uint64_t data = fpga_read_32(handle, offset);
    data |= (uint64_t)fpga_read_32(handle, offset + 4) << 32;

    return data;
#endif
}

static inline void fpga_write_64(FPGA_MMIO_INTERFACE_HANDLE handle, uint32_t offset, uint64_t value)
{
#ifndef FPGA_PLATFORM_FORCE_64BIT_MMIO_EMULATION_WITH_32BIT
    *((volatile uint64_t *)((volatile uint8_t *)fpga_devmem_get_base_address(handle) + offset)) = value;
#else
    // This emulation is needed when Intel FPGA PCIe Memory Mapped Bridge IP is used to implement the PCIe function.
    // Little-endian system is assumed.
    fpga_write_32(handle, offset, (uint32_t)value);
    fpga_write_32(handle, offset + 4, (uint32_t)(value >> 32));
#endif
}

static inline void fpga_read_512(FPGA_MMIO_INTERFACE_HANDLE handle, uint32_t offset, uint8_t *value)
{
    int     i;
    for(i = 0; i < (512/64); ++i)
    {
        *((volatile uint64_t *)value) = fpga_read_64(handle, offset);
        value += 64/8;
        offset += 64/8;
    }
}

static inline void fpga_write_512(FPGA_MMIO_INTERFACE_HANDLE handle, uint32_t offset, uint8_t *value)
{
    int     i;
    for(i = 0; i < (512/64); ++i)
    {
        fpga_write_64(handle, offset, *((volatile uint64_t *)value));
        value += 64/8;        
        offset += 64/8;
    }
}

void *fpga_malloc(FPGA_MMIO_INTERFACE_HANDLE handle, uint32_t size);
void fpga_free(FPGA_MMIO_INTERFACE_HANDLE handle, void *address);
FPGA_PLATFORM_PHYSICAL_MEM_ADDR_TYPE fpga_get_physical_address(void *address);

int fpga_register_isr(FPGA_INTERRUPT_HANDLE handle, FPGA_ISR isr, void *isr_context);
int fpga_enable_interrupt(FPGA_INTERRUPT_HANDLE handle);
int fpga_disable_interrupt(FPGA_INTERRUPT_HANDLE handle);

#ifdef __cplusplus
}
#endif
