
/**
* @file intel_fpga_platform.h
*
* @brief This header file should be included in the FPGA IP access driver source code.  This file defines
* the function prototypes that interface with Intel FPGA IP Access API library.
*
* @warning This header file is created for the documentation purpose. Use the platform specific 
* header file.
*
*/

/**
* @defgroup platform_macros  Platform Specific Macros and Types
* 
* This includes a set of macros and types used by FPGA IP access driver with values specific to the target platform.
*
* @{
*/

/**
* \defgroup version  Macros Defining Platform Version
* 
* This includes a set of macros that defines the Platform version.
*
* @{
*/

/**
* @brief This is a macro defining the major version.
*
* Platform in newer version should be backward compatible with the older versions.  
* For example, if FPGA IP access driver is developed with version 1 of API, the FPGA IP access driver will
* continue to work if API version >= 1 is used.  However,
* if FPGA IP access driver is developed with version 3 of API, the FPGA IP access driver should
* stop compilation or throw runtime exception if library of version < 3 is loaded.
*
*/
#define FPGA_PLATFORM_MAJOR_VERSION 0

/**
* @brief This is a macro defining the minor version.
*
* This version signals a change in the implementation of the platform middleware.
*
* There is no change in API interface and behavior.
*
*/
#define FPGA_PLATFORM_MINOR_VERSION 0

/** @}*/ // end of mmio_capability


/**
* \defgroup mmio_capability  Macros Defining Platform Capabilities on Native MMIO Access.
* 
* This includes a set of macros that defines the native MMIO access capability of the target platform.
*
* @{
*/

/// @brief This is a macro reporting whether the platform support 8-bit read natively
#define FPGA_PLATFORM_HAS_NATIVE_MMIO_READ_8
/// @brief This is a macro reporting whether the platform support 8-bit write natively
#define FPGA_PLATFORM_HAS_NATIVE_MMIO_WRITE_8
/// @brief This is a macro reporting whether the platform support 16-bit read natively
#define FPGA_PLATFORM_HAS_NATIVE_MMIO_READ_16
/// @brief This is a macro reporting whether the platform support 16-bit write natively
#define FPGA_PLATFORM_HAS_NATIVE_MMIO_WRITE_16
/// @brief This is a macro reporting whether the platform support 32-bit read natively
#define FPGA_PLATFORM_HAS_NATIVE_MMIO_READ_32
/// @brief This is a macro reporting whether the platform support 32-bit write natively
#define FPGA_PLATFORM_HAS_NATIVE_MMIO_WRITE_32
/// @brief This is a macro reporting whether the platform support 64-bit read natively
#define FPGA_PLATFORM_HAS_NATIVE_MMIO_READ_64
/// @brief This is a macro reporting whether the platform support 64-bit write natively
#define FPGA_PLATFORM_HAS_NATIVE_MMIO_WRITE_64
/// @brief This is a macro reporting whether the platform support 512-bit read natively
#define FPGA_PLATFORM_HAS_NATIVE_MMIO_READ_512
/// @brief This is a macro reporting whether the platform support 512-bit write natively
#define FPGA_PLATFORM_HAS_NATIVE_MMIO_WRITE_512

/** @}*/ // end of mmio_capability

/**
* \defgroup dma_capability  Macros Defining Platform Capabilities on DMA.
* 
* This includes a set of macros that defines the DMA capability of the target platform.
*
* @{
*/

/// @brief This is a macro reporting whether the platform supports any DMA capability
#define FPGA_PLATFORM_HAS_DMA_CAPABILITY

/** @}*/ // end of dma_capability

/**
* \defgroup isr_behavior  Macros Defining Platform Behavior on Hardware Interrupt.
* 
* This includes a set of macros that defines the hardware interrupt behavior of the target platform.
*
* @{
*/

/// @brief This is a macro reporting whether the interrupt service routine (isr) is called from a different thread, instead of interrupting the execution of the application.
#define FPGA_PLATFORM_IS_ISR_CALLED_IN_THREAD


/** @}*/ // end of isr_behavior


/**
* \defgroup fpga_ip_access_types  FPGA IP access driver types
* 
* This set of types used by FPGA IP access driver
*
* @{
*/

/**
* @brief MMIO interface handle type
* @note This type name is portable among all FPGA IP Access API libraries for different platforms.  The typedef definition is platform specific.
*/
typedef uint32_t FPGA_MMIO_INTERFACE_HANDLE;

/**
* @brief Invalid MMIO interface handle return value
* @note This macro is portable among all FPGA IP Access API libraries for different  platforms.  The macro definition is platform specific.
*/
#define FPGA_MMIO_INTERFACE_INVALID_HANDLE -1

/**
* @brief Hardware interrupt interface handle type
* @note This type name is portable among all FPGA IP Access API libraries for different  platforms.  The typedef definition is platform specific.
*/
typedef int FPGA_INTERRUPT_HANDLE;

/**
* @brief Invalid interrupt interface handle return value
* @note This macro is portable among all FPGA IP Access API libraries for different  platforms.  The macro definition is platform specific.
*/
#define FPGA_INTERRUPT_INVALID_HANDLE -1

/**
* @brief Interface parameter information
* @note This type name is portable among all FPGA IP Access API libraries for different  platforms.
*/
typedef struct {
    uint16_t version;       //!< Parameter version
    uint16_t param_id;      //!< Parameter ID
    size_t   data_size;     //!< Parameter data size in bytes. The size is mutiple of 8
    uint64_t *data;         //!< Pointer to parameter data
} FPGA_INTERFACE_PARAMETER;

/**
* @brief Interface GUID/UUID
* @note This type name is portable among all FPGA IP Access API libraries for different  platforms.
*/
typedef struct {
    uint64_t                     guid_l;        //!< Lower 64 bits of the GUID
    uint64_t                     guid_h;        //!< Upper 64 bits of the GUID
} FPGA_INTERFACE_GUID;

/**
* @brief Interface information
* @note This type name is portable among all FPGA IP Access API libraries for different  platforms.  The typedef definition is platform specific.  
* The public documented member variables in this struct are portable among all FPGA IP Access API libraries for different  platforms.
*/
typedef struct 
{
    FPGA_INTERFACE_GUID          guid;              //!< Interface GUID/UUID
    uint16_t                     instance_id;       //!< Identify an instance of interface when this interface is instantiated multiple times in the system.
    uint16_t                     group_id;          //!< Define a group of interfaces that support a high-level function.  One FPGA IP Access driver may be developed using such group of interfaces.
    size_t                       num_of_parameters; //!< Define the array size of parameters 
    FPGA_INTERFACE_PARAMETER     *parameters;       //!< Point to parameter array
    int                          dfh_parent;        //!< Index to the FPGA_INTERFACE_INFO of the parent; -1 if there is no parent
} FPGA_INTERFACE_INFO;

/**
* @brief Physical memory address value type
* @note This type name is portable among all FPGA IP Access API libraries for different  platforms.  The typedef definition is platform specific.
*/
typedef int64_t FPGA_PLATFORM_PHYSICAL_MEM_ADDR_TYPE;


/** @}*/ // end of fpga_ip_access_types

/** @}*/ // end of platform_macros
