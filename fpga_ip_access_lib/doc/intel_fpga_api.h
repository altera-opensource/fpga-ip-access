
/**
* @file intel_fpga_api.h
*
* @brief This header file should be included in the FPGA IP access driver source code.  This file defines
* the function prototypes that interface with Intel FPGA IP Access API library.
*
* @note The function defined may be implemented as macros or inline function depending on
* the library implemented for a specific platform. The interface remains the same syntactically.
*
* @warning This header file is created for the documentation purpose. Use the platform specific
* header file.
*/

/**
* @defgroup fpga_ip_access_api  FPGA IP Access API
* 
* This includes a set of functions used by FPGA IP access driver.
*
* @{
*/

/**
* @defgroup mmio_rw  Memory Mapped IO (MMIO) Read and Write Access
*
* This defines a set of MMIO access functions.
*
* @pre All address values should be aligned to the corresponding memory read/write unit size
* except the 512-bit memory read/write.  For example, 32-bit read address should be multiple
* of 4. The address for 512-bit read/write should be aligned with 64-bit word, i.e. multiple
* of 8.  Any read/write to an unaligned address is undefined.
*
* @warning No platform supports the full set of API natively. Check the platform capability at
* run-time or at the compile time using the macros defined.  MMIO is emulated with the
* closest unit size if that unit is not natively supported by the targeted platform. If the
* smallest unit size is bigger than the desired size, the smallest unit size available natively
* is used to emulate.  For example, if 32-bit read is smallest unit available natively,
* 8-bit and 16-bit read is emulated with 32-bit read at the closest aligned address.  For example,
* fpga_read_8() at address from 0 to 3 is emulated with fpga_read_32() at address 0.  Each function
* call will result in a separate MMIO transaction.  Transactions are never combined.
* If the biggest unit size is smaller than the desired size, the biggest unit size available natively
* is used to emulate.  For example, if 64-bit read is biggest unit available natively, the 512-bit
* read is emulated with 8 sequential 64-bit read.  For example, fpga_read_512() at address 0 is
* emulated as for(int i = 0; i < 512/32; ++i) fpga_read_32(myhandle, i*(32/8));
*
* @warning If emulated read and write may introduce side-effect, the driver should error out at the
* compile time, preferably, or at the run-time with exception.
*
* @warning For example,
* @code
* // Compile time exception report using the available macros.
* #ifndef FPGA_PLATFORM_HAS_NATIVE_MMIO_READ_8
* #error Current platform doesn't support native 8-bit MMIO read as required.
* #endif
*
* // Run-time exception using the available macros.
* #ifndef FPGA_PLATFORM_HAS_NATIVE_MMIO_READ_8
* fpga_throw_runtime_exception(__FUNC__, __FILE__, __LINE__, "Current platform doesn't support native 8-bit MMIO read as required.");
* #endif
*
* @endcode
*
*
* @{
*/

/**
* @brief The function provides the 8-bit MMIO read access.
* 
* Read one byte at the offset address relative to the MMIO interface.
*
* @note Use FPGA_PLATFORM_HAS_NATIVE_MMIO_READ_8 to determine whether this is natively supported.
*
* @param[in] handle The handle to the targeted MMIO interface.  The type is specific to the platform.  Obtained with fpga_open().
* @param[in] offset The address offset from the base address of the MMIO interface. The address offset specifies the byte address always, independent from the targeted hardware interface property.
* 
* @return The value read from the specified address.
*/
uint8_t fpga_read_8(FPGA_MMIO_INTERFACE_HANDLE handle, uint32_t offset);

/**
* @brief The function provides the 8-bit MMIO write access.
* 
* Write one byte at the offset address relative to the MMIO interface.
*
* @note Use FPGA_PLATFORM_HAS_NATIVE_MMIO_WRITE_8 to determine whether this is natively supported.
*
* @param[in] handle The handle to the targeted MMIO interface.  The type is specific to the platform.  Obtained with fpga_open().
* @param[in] offset The address offset from the base address of the MMIO interface. The address offset specifies the byte address always, independent from the targeted hardware interface property.
* @param[in] value The value to write to the specified address.
* 
*/
void fpga_write_8(FPGA_MMIO_INTERFACE_HANDLE handle, uint32_t offset, uint8_t value);

/**
* @brief The function provides the 16-bit MMIO read access.
* 
* Read one 16-bit integer at the offset address relative to the MMIO interface.
*
* @note Use FPGA_PLATFORM_HAS_NATIVE_MMIO_READ_16 to determine whether this is natively supported.
*
* @param[in] handle The handle to the targeted MMIO interface.  The type is specific to the platform.  Obtained with fpga_open().
* @param[in] offset The address offset from the base address of the MMIO interface. The address offset specifies the byte address always, independent from the targeted hardware interface property.
* 
* @return The value read from the specified address.
*/
uint16_t fpga_read_16(FPGA_MMIO_INTERFACE_HANDLE handle, uint32_t offset);

/**
* @brief The function provides the 16-bit MMIO write access.
* 
* Write one 16-bit integer at the offset address relative to the MMIO interface.
*
* @note Use FPGA_PLATFORM_HAS_NATIVE_MMIO_WRITE_16 to determine whether this is natively supported.
*
* @param[in] handle The handle to the targeted MMIO interface.  The type is specific to the platform.  Obtained with fpga_open().
* @param[in] offset The address offset from the base address of the MMIO interface. The address offset specifies the byte address always, independent from the targeted hardware interface property.
* @param[in] value The value to write to the specified address.
* 
*/
void fpga_write_16(FPGA_MMIO_INTERFACE_HANDLE handle, uint32_t offset, uint16_t value);


/**
* @brief The function provides the 32-bit MMIO read access.
* 
* Read one 32-bit integer at the offset address relative to the MMIO interface.
*
* @note Use FPGA_PLATFORM_HAS_NATIVE_MMIO_READ_32 to determine whether this is natively supported.
*
* @param[in] handle The handle to the targeted MMIO interface.  The type is specific to the platform.  Obtained with fpga_open().
* @param[in] offset The address offset from the base address of the MMIO interface. The address offset specifies the byte address always, independent from the targeted hardware interface property.
* 
* @return The value read from the specified address.
*/
uint32_t fpga_read_32(FPGA_MMIO_INTERFACE_HANDLE handle, uint32_t offset);

/**
* @brief The function provides the 32-bit MMIO write access.
* 
* Write one 32-bit integer at the offset address relative to the MMIO interface.
*
* @note Use FPGA_PLATFORM_HAS_NATIVE_MMIO_WRITE_32 to determine whether this is natively supported.
*
* @param[in] handle The handle to the targeted MMIO interface.  The type is specific to the platform.  Obtained with fpga_open().
* @param[in] offset The address offset from the base address of the MMIO interface. The address offset specifies the byte address always, independent from the targeted hardware interface property.
* @param[in] value The value to write to the specified address.
* 
*/
void fpga_write_32(FPGA_MMIO_INTERFACE_HANDLE handle, uint32_t offset, uint32_t value);

/**
* @brief The function provides the 64-bit MMIO read access.
* 
* Read one 64-bit integer at the offset address relative to the MMIO interface.
*
* @note Use FPGA_PLATFORM_HAS_NATIVE_MMIO_READ_64 to determine whether this is natively supported.
*
* @param[in] handle The handle to the targeted MMIO interface.  The type is specific to the platform.  Obtained with fpga_open().
* @param[in] offset The address offset from the base address of the MMIO interface. The address offset specifies the byte address always, independent from the targeted hardware interface property.
* 
* @return The value read from the specified address.
*/
uint64_t fpga_read_64(FPGA_MMIO_INTERFACE_HANDLE handle, uint32_t offset);

/**
* @brief The function provides the 64-bit MMIO write access.
* 
* Write one 64-bit integer at the offset address relative to the MMIO interface.
*
* @note Use FPGA_PLATFORM_HAS_NATIVE_MMIO_WRITE_64 to determine whether this is natively supported.
*
* @param[in] handle The handle to the targeted MMIO interface.  The type is specific to the platform.  Obtained with fpga_open().
* @param[in] offset The address offset from the base address of the MMIO interface. The address offset specifies the byte address always, independent from the targeted hardware interface property.
* @param[in] value The value to write to the specified address.
* 
*/
void fpga_write_64(FPGA_MMIO_INTERFACE_HANDLE handle, uint32_t offset, uint64_t value);

/**
* @brief The function provides the 512-bit MMIO read access.
* 
* Read one 512-bit integer at the offset address relative to the MMIO interface.
*
* @note Use FPGA_PLATFORM_HAS_NATIVE_MMIO_READ_512 to determine whether this is natively supported.
*
* @param[in] handle The handle to the targeted MMIO interface.  The type is specific to the platform.  Obtained with fpga_open().
* @param[in] offset The address offset from the base address of the MMIO interface. The address offset specifies the byte address always, independent from the targeted hardware interface property.
* @param[in,out] value The pointer to the buffer allocated to store the 512-bit value read.
*
*/
void fpga_read_512(FPGA_MMIO_INTERFACE_HANDLE handle, uint32_t offset, uint8_t *value);

/**
* @brief The function provides the 512-bit MMIO write access.
* 
* Write one 512-bit integer at the offset address relative to the MMIO interface.
*
* @note Use FPGA_PLATFORM_HAS_NATIVE_MMIO_WRITE_512 to determine whether this is natively supported.
*
* @param[in] handle The handle to the targeted MMIO interface.  The type is specific to the platform.  Obtained with fpga_open().
* @param[in] offset The address offset from the base address of the MMIO interface. The address offset specifies the byte address always, independent from the targeted hardware interface property.
* @param[in] value The pointer to the buffer holding the 512-bit value to write.
* 
*/
void fpga_write_512(FPGA_MMIO_INTERFACE_HANDLE handle, uint32_t offset, uint8_t *value);



/** @} */ // end of mmio_rw

/**
* @defgroup dma_mem  Memory Management for DMA
*
* This defines a set of memory allocation functions to facilitate the DMA data transfer.
*
* @warning As allocated memory will be locked/pinned to the physical memory location, this should not
* be used for general purpose memory allocation.
*
* @warning Not all platforms support DMA.  Check the platform capability at
* run-time or at the compile time using the macros defined.
*
* @warning For example, error out if the DMA capability is required.
* @code
* // Compile time exception report using the available macros.
* #ifndef FPGA_PLATFORM_HAS_NATIVE_MMIO_READ_8
* #error This driver is only usable with platform supporting DMA.
* #endif
*
* // Run-time exception
* #ifndef FPGA_PLATFORM_HAS_NATIVE_MMIO_READ_8
* fpga_throw_runtime_exception(__FUNC__, __FILE__, __LINE__, "Current platform doesn't support DMA as required.");
* #endif
*
* @endcode
*
* @warning For example, branch the code depending on the availability of the DMA capability.
* @code
* // Compile time switch using the available macro.
* #ifdef FPGA_PLATFORM_HAS_DMA_CAPABILITY
* // Perform DMA data transfer
* #else
* // Perform MMIO data transfer
* #endif
*
* @endcode
*
* This API is defined to support a user space DMA data transfer model as described below.
*
* @code
* dma_mem = fpga_malloc();
* if (dma_mem != nullptr) fpga_throw_runtime_exception();
* physical_dma_mem = fpga_get_physical_address(dma_mem);  // physical_dma_mem is used to setup descriptor in the following steps
*
* // data transfer from host mem to device
* foreach data transfer
* {
*     Either write descriptor into the dma_mem region or pump descriptor into the device using the MMIO functions depending on the DMA controller
*     Populate data into the dma_mem region
*     Start transfer if needed when the start event is not part of dma_mem memory write.
* }
*
* // data transfer from device to host mem
* foreach data transfer
* {
*     Either write descriptor into the dma_mem region or pump descriptor into the device using the MMIO functions depending on the DMA controller
*     Wait for data transfer occurs with one of possible implementation: hw interrupt, poll DMA controller CSR using MMIO, or poll data within the dma_region (CAUTION: this requires cache coherent write to that address from device.)
*     Process the transferred data into the dma_mem region
* }
*
* fpga_free(dma_mem);
* @endcode
*
* @note Memory used for DMA must be fully specified using this API even on the baremetal platform
* without a memory manager.  The layout should be defined in the Platform Designer. Memory allocation
* via this set of API ensures portability across multiple platforms even if it may not have any
* material effect in the baremetal system.
*
* @{
*/

/**
* @brief The function provides physical memory allocation for the DMA purpose.
*
* @warning This should not be used for general purpose data memory allocation.  This memory resource is be very limited.
*
* @warning Only one memory allocation can be made per MMIO interface.
*
* @note The memory layout may be static in the baremetal environment.  The layout should be defined in Platform Designer.
* This function should be called still for the portability reason.
*
* @param[in] handle The handle to the targeted MMIO interface.  The type is specific to the platform.  Obtained with fpga_open().
* @param[in] size The number of bytes to allocate.
* @return The application space virtual address pointing to the memory buffer allocated on the physical memory.
* The nullptr, i.e. 0, is returned if memory requested is not available.
*/
void *fpga_malloc(FPGA_MMIO_INTERFACE_HANDLE handle, uint32_t size);


/**
* @brief The function releases physical memory allocation previously acquired.
* 
* @pre The address input must be a valid value returned by fpga_malloc().  The behavior of providing an
* invalid address is undefined.
*
* @param[in] handle The handle to the targeted MMIO interface.  The type is specific to the platform.  Obtained with fpga_open().
* @param[in] address The address value returned by fpga_malloc().
* 
*/
void fpga_free(FPGA_MMIO_INTERFACE_HANDLE handle, void *address);


/**
* @brief The function provides physical memory address information.
*
* The physical memory address is used to set up DMA transfer.  The physical memory address is also known as IO memory address.
* 
* @param[in] address The pointer to the buffer as returned by fpga_malloc().
* @return the physical memory address for the application space virtual address returned from fpga_malloc().
*
*/
FPGA_PLATFORM_PHYSICAL_MEM_ADDR_TYPE fpga_get_physical_address(void *address);


/** @} */ // end of dma_mem



/**
* @defgroup application_utils  Application Utility Functions
* 
* This defines a set of functions that post information to the application linking to the FPGA IP access driver.
* 
*
* @{
*/

/**
* @brief The function posts message in an application specific way.
* 
* @param[in] type The message type.
* @param[in] format The format string of additional message that describes this exception in an application user readable way.  The format conforms to the one defined for printf in C99 standard.
* 
*/
int fpga_msg_printf(FPGA_MSG_PRINTF_TYPE type, const char * format, ...);


/**
* @brief The function reports an irrecoverable run-time exception.
* 
* This function informs the application that any code beyond this function call will result in undefined
* behavior.  This function call will result in the termination of the application.
*
* @param[in] function The function name where this function call is made from.
* @param[in] file The file name where this function call is made from.
* @param[in] lineno The line number where this function call is made from.
* @param[in] format The format string of additional message that describes this exception in an application user readable way.  The format conforms to the one defined for printf in C99 standard.
* 
*/
fpga_throw_runtime_exception(const char *function, const char *file, int lineno, const char * format, ...);


/** @} */ // end of application_utils

/** @} */ // end of fpga_ip_access_api
