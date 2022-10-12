
/**
 * @file intel_fpga_platform_api.h
 *
 * @brief This header file should be included in the application source code.  This file defines
 * the function prototypes that interface with Intel FPGA IP Access API library.
 *
 * @note The function defined may be implemented as macros or inline function depending on
 * the platform targeted. The interface remains the same.
 *
 * @warning This header file is created for the documentation purpose. Use the platform specific
 * header file.
 */

/**
* @defgroup application_api  Application API
* 
* This includes a set of functions used by application.
*
* @{
*/

/**
* \defgroup platform  Platform Initialization and Termination
*
* This defines a set of functions that initialize and terminate the FPGA IP Access API library environment of the given platform.
*
* This set of functions should be called from the application before or after any FPGA IP access driver function call.
*
* @{
*/

/**
* @brief The function initializes the platform.
*
* This function must be called before any other FPGA IP access driver function call, other than fpga_platform_register_printf().  
* The arguments are platform specific.  Please read the document for the specific platform on the argument values.
*
* @param[in] argc The number of valid strings in argv.
* @param[in] argv The array of null-terminated strings.  The array size is specified with argc.
*
* @return true if the platform is initialized successfully; otherwise, false.  Application cannot continue without a successful return.
*/
bool fpga_platform_init(unsigned int argc, const char *argv[]);

/**
* @brief The function terminates the platform.
* 
* This function is called to cleanup the platform resource before the application ends.
* 
*/
void fpga_platform_cleanup();


/**
* @brief This defines message types
*
* Without registering an application specific message printf function using fpga_platform_register_printf,
* each message with be prefixed with a type specific word.
*
* @note Each type of message can be removed from the code generation with its corresponding macro at the compile time.
* Alternatively, they can be disabled with one macro FPGA_MSG_PRINTF_ENABLE.
*
*/
typedef enum
{
    FPGA_MSG_PRINTF_INFO,         //!< In default printf, this type of message is prefixed with "Info: "
    FPGA_MSG_PRINTF_WARNING,      //!< In default printf, this type of message is prefixed with "Warning: "
    FPGA_MSG_PRINTF_ERROR,        //!< In default printf, this type of message is prefixed with "Error: "
    FPGA_MSG_PRINTF_DEBUG         //!< In default printf, this type of message is prefixed with "Debug: "
} FPGA_MSG_PRINTF_TYPE;

/** 
* @brief function pointer type of the message printf function.
*
* @note The format and the consequent variable argument list should match the C99 language specification.
*
*/
typedef int (*FPGA_MSG_PRINTF) (FPGA_MSG_PRINTF_TYPE type, const char * format, va_list args);

/**
* @brief The function registers an application specific function pointer to be used by fpga_msg_printf().
* 
* By default, the printf function from the C run-time library will be used.
*
* @note This function should be called before fpga_platform_init() as printf may be called within that function.
*
* @param[in] msg_printf The function pointer to the application specific printf function.
*/
void fpga_platform_register_printf(FPGA_MSG_PRINTF msg_printf);

/** 
* @brief function pointer type of the run-time exception handler function.
*
* @note The format and the consequent variable argument list should match the C99 language specification.
*
*/
typedef void (*FPGA_RUNTIME_EXCEPTION_HANDLER) (const char *function, const char *file, int lineno, const char * format, va_list args);


/**
* @brief The function registers an application specific function to handle run-time exception from FPGA IP access driver call to fpga_throw_runtime_exception().
* 
* By default, the exit() function from the C run-time library will be used.
*
* @note This function should be called before fpga_platform_init() as run-time exception may be generated within that function.
*
* @param[in] handler The function pointer to the application specific run-time exception handler.
*/
void fpga_platform_register_runtime_exception_handler(FPGA_RUNTIME_EXCEPTION_HANDLER handler);



/** @} */ // end of platform

/**
* @defgroup discovery  MMIO and Interrupt Interface Discovery and Registration
*
* This defines a set of functions enumerating the components with memory mapped interface within the
* targeted platform and registering the access to them by obtaining handle.
*
* DFL (Device Feature List) is used to build the interface information if available. Otherwise, only
* a single interface is available.
*
* @note The hardware interrupt sender interface is associated with a memory mapped agent/responder interface providing the mmio access.
*
* @code
*     // Example code that enumerates all interfaces and creates handle 
*     FPGA_INTERFACE_INFO info;
*     FPGA_MMIO_INTERFACE_HANDLE handle;
* 
*     for( int index = 0; index < num_interfaces; ++index )
*     {
*         fpga_msg_printf(FPGA_MSG_PRINTF_INFO, "   ---------- Interface %d ----------", index);
*         fpga_get_interface_at(index, &info);
*         fpga_msg_printf(FPGA_MSG_PRINTF_INFO, "   fpga_get_interface_at(%d, &info) -> info", index);
*         fpga_msg_printf(FPGA_MSG_PRINTF_INFO, "   128-bit GUID: 0x%016llX%016llX", info.guid.guid_h, info.guid.guid_l);
*         fpga_msg_printf(FPGA_MSG_PRINTF_INFO, "   Instance ID: %d", info.instance_id);
*         fpga_msg_printf(FPGA_MSG_PRINTF_INFO, "   Group ID: %d", info.group_id);
* 
*         for (int i = 0; i < info.num_of_parameters; i++)
*         {
*             fpga_msg_printf(FPGA_MSG_PRINTF_INFO, "      ------ Parameter %d ------   ", i);
*             fpga_msg_printf(FPGA_MSG_PRINTF_INFO, "      Version: %d", info.parameters[i].version);
*             fpga_msg_printf(FPGA_MSG_PRINTF_INFO, "      Param ID: %d", info.parameters[i].param_id);
*             fpga_msg_printf(FPGA_MSG_PRINTF_INFO, "      Param Data Size: %d", info.parameters[i].data_size);
* 
*             for (int j = 0; j < info.parameters[i].data_size / 8; j++)
*             {
*                 fpga_msg_printf(FPGA_MSG_PRINTF_INFO, "      64-bit Param Data[%d]: 0x%016llX", j, info.parameters[i].data[j]);
*             }
*         }
* 
*         handle = fpga_open(index);
* 
*         // Pass the handle to FPGA IP access driver and call the driver functions
* 
*         fpga_close(handle)
*     }
* 
* @endcode
* @{
*/

/**
* @brief The function provides the interface count in the platform.
* 
* Returns the total number of interfaces.  All interfaces can be enumerated with fpga_get_interface_at().
*
*/
unsigned int fpga_get_num_of_interfaces();


/**
* @brief The function gets information of a specific interface.
* 
* @pre A valid buffer must be created to be filled with the interface information.
*
* @param[in] index The interface index.
* @param[out] info The pointer to the interface information structure.  The information is populated into this structure.
* @return  true if the specified interface index is < fpga_get_num_of_interfaces() and >= 0.  
* 
*/
bool fpga_get_interface_at(unsigned int index, FPGA_INTERFACE_INFO *info);

/**
* @brief The function claims the exclusive usage of the interface.
* 
* @warning This claim applies within the process space only.
*
* @param[in] index The interface index.
* @return the handle to be used by other functions to target this interface; FPGA_MMIO_INTERFACE_INVALID_HANDLE, if index is wrong or interface has been opened.
*/
FPGA_MMIO_INTERFACE_HANDLE fpga_open(unsigned int index);

/**
* @brief The function releases the exclusive usage of the interface.
* 
* @param[in] index The interface index.
* 
*/
void fpga_close(unsigned int index);


/**
* @brief The function claims the exclusive usage of the interrupt interface associated to the interface.
* 
* @warning This claim applies within the process space only.
*
* @param[in] index The interface index.
* @return the handle to be used by other functions to target this interrupt interface; FPGA_INTERRUPT_INVALID_HANDLE, if index is wrong or interface has been opened.
*/
FPGA_INTERRUPT_HANDLE fpga_interrupt_open(unsigned int index);

/**
* @brief The function releases the exclusive usage of the interrupt interface associated to the interface.
* 
* @param[in] index The interface index.
* 
*/
void fpga_interrupt_close(unsigned int index);


/** @} */ // end of discovery

/**
* @defgroup interrupt  Interrupt Support
* 
* This defines a set of functions that deal with hardware interrupt.
* 
* The interrupt is serviced by registering an interrupt service routine (isr). See #FPGA_ISR
* The isr calling behavior is different between a baremetal program and an OS based application.
* Read fpga_register_isr() for the behavior difference.  In order to develop a portable FPGA IP access driver,
* isr shouldn't be implemented within the FPGA IP access driver.  Instead, isr should be accepted by some
* high-level API of the FPGA IP access driver and documented to the FPGA IP access driver users on how isr should be 
* implemented depending on the application environment where the isr calling behavior is known.
*
* @{
*/

/**
* @brief Interrupt service routine function pointer type.
*
* @param isr_context A pointer to a data structure when the ISR is registered with fpga_register_isr()
*
*/
typedef void (*FPGA_ISR) ( void *isr_context );

/**
* @brief The function registers an interrupt service routine.
* 
* @warning Interrupt service routine (isr) does not interrupt the execution of the application main thread
* in many operating system based environments, such as Linux and Windows.  In such environment,
* isr is invoked from a different thread created by the FPGA IP Access API library.  Any
* data exchange between the isr running in one thread and function running in other thread must be properly 
* synchronized. In the baremental environment, the application main thread jumps to isr immediately and 
* returns back to the point where the interruption occurred at the end of isr.
*
* @warning Use FPGA_PLATFORM_IS_ISR_CALLED_IN_THREAD to decide the isr calling behavior. 
* 
* @param[in] handle The interrupt handle
* @param [in] isr A function pointer to isr.
* @param [in] isr_context A pointer to a data structure. This value is passed to the ISR when it is called.
* @return 0 if successful; 1 if an isr previously registered is overwritten; any other value if there is any other error.
* 
*/
int fpga_register_isr(FPGA_INTERRUPT_HANDLE handle, FPGA_ISR isr, void *isr_context);


/**
* @brief The function enables interrupt handling.
* 
* @pre Interrupt service routine must have been registered before calling this function.
*
* @param[in] handle The interrupt handle
* @return 0 if successful; -1 if isr hasn't been registered; -2 if there is hardware fault.
* 
*/
int fpga_enable_interrupt(FPGA_INTERRUPT_HANDLE handle);


/**
* @brief The function disables interrupt handling.
* 
* @pre Interrupt service routine should have been enabled before calling this function.
*
* @param[in] handle The interrupt handle
* @return 0 if successful; -1 if isr hasn't been registered; -2 if there is hardware fault; -3 if isr hasn't been enabled.
*/
int fpga_disable_interrupt(FPGA_INTERRUPT_HANDLE handle);


/** @} */ // end of interrupt


/**
* \defgroup compilation_switches  Macros Describing Platform Compilation Switches.
* 
* This set of macros allows application builder to control code generation. 
*
* It should be defined by application build environment.  They are not 
* specific to platform type.  They influence the code generation of platform or FPGA IP access driver.
*
* @warning By default, none of the macros in this group is defined.
* 
* @{
*/

/// @brief Switch to enable/disable any message printf.  By default, any message printf is disabled.
#define INTEL_FPGA_MSG_PRINTF_ENABLE  1

/// @brief Switch to enable/disable message printf of the INTEL_FPGA_MSG_PRINTF_INFO type.  By default, such type of message printf is disabled.
#define INTEL_FPGA_MSG_PRINTF_ENABLE_INFO  1

/// @brief Switch to enable/disable message printf of the INTEL_FPGA_MSG_PRINTF_WARNING type.  By default, such type of message printf is disabled.
#define INTEL_FPGA_MSG_PRINTF_ENABLE_WARNING  1

/// @brief Switch to enable/disable message printf of the INTEL_FPGA_MSG_PRINTF_ERROR type.  By default, such type of message printf is disabled.
#define INTEL_FPGA_MSG_PRINTF_ENABLE_ERROR  1

/// @brief Switch to enable/disable message printf of the INTEL_FPGA_MSG_PRINTF_DEBUG type.  By default, such type of message printf is disabled.
#define INTEL_FPGA_MSG_PRINTF_ENABLE_DEBUG  1

/** @} */ // end of compilation_switches

/** @} */ // end of application_api
