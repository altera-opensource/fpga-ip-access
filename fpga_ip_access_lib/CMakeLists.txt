IF((NOT ${BUILD_FPGA_IP_ACCESS_MODE} MATCHES "ZEPHYR") OR (NOT DEFINED BUILD_FPGA_IP_ACCESS_MODE))
    cmake_minimum_required(VERSION 3.0.0)

    project(fpga_ip_access_lib VERSION 0.1.0)

    # Create an option to specify which mode to use, defult to "UIO"
    set(BUILD_FPGA_IP_ACCESS_MODE UIO CACHE STRING "Which mode to use")
    set(FPGA_IP_ACCESS_OPTIONS "")

    # Get a list of all files and sub directories
    FILE(GLOB children RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)
    # Iterate all directories and add any which have a CMakeLists.txt
    FOREACH( child ${children} )

        # Exclude zephyr as zephyr using its own toolchain
        IF(${child} MATCHES "zephyr")
            continue()
        ENDIF()

        IF( IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${child} )
            IF( EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${child}/CMakeLists.txt )
            
                # It is assumed that any CMakeLists.txt follows the rules for a FPGA IP Access Library
                # The subdirectory needs to do the following:
                #   Added the OPTION name to FPGA_IP_ACCESS_OPTIONS
                #   Set up a library using the ${PROJECT_NAME} as this is inherited from this CMakeLists.txt
                add_subdirectory(${child})
            ENDIF()
        ENDIF()
    ENDFOREACH()

# each subdirectory should setup the library and add it's option name to FPGA_IP_ACCESS_OPTIONS
# Use "cmake -DBUILD_FPGA_IP_ACCESS_MODE=XXX" to change which library you want to build with
set_property(CACHE BUILD_FPGA_IP_ACCESS_MODE PROPERTY STRINGS ${FPGA_IP_ACCESS_OPTIONS})

else()
    # Add zephyr directory
    # Need to define enviroment variables such as ZEPHYR_BASE, ZEPHYR_SDK_INSTALL_DIR correctly
    # Cmake values: set BUILD_FPGA_IP_ACCESS_MODE=ZEPHYR, BOARD value is needed to compile Zephyr application
    # Example : cmake -DBUILD_FPGA_IP_ACCESS_MODE=ZEPHYR -DBOARD=niosv_g ..
    add_subdirectory(zephyr)

endif()
