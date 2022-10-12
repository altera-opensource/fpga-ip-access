# SPDX-License-Identifier: Apache-2.0

if( BUILD_FPGA_IP_ACCESS_MODE STREQUAL ZEPHYR)
    cmake_minimum_required(VERSION 3.20.0)
    find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

    add_compile_definitions(ZEPHYR_FPGA_IP_ACCESS)

    option(TEST_INT "Enable Interrupt Test" OFF)
    if (TEST_INT)
        add_compile_definitions(ZEPHYR_UNIT_TEST_INT)
    endif()

    zephyr_include_directories(
	    ../common/inc
	    inc
        ${CMAKE_BINARY_DIR}/include
    )

    FILE(GLOB api_sources src/*.c)
    FILE(GLOB cmn_sources ../common/src/*.c)

    zephyr_library_named(fpga_ip_access_cmn)
    zephyr_library_sources(${cmn_sources})
    zephyr_link_libraries(fpga_ip_access_cmn)

    zephyr_library_named(fpga_ip_access_api)
    zephyr_library_sources(${api_sources})
    zephyr_link_libraries(fpga_ip_access_api)

    add_custom_target(version_cmn
    ${CMAKE_COMMAND} -D SRC=${CMAKE_CURRENT_SOURCE_DIR}/../common/inc/intel_fpga_api_cmn_version.h.in
                   -D DST=${CMAKE_BINARY_DIR}/include/intel_fpga_api_cmn_version.h
                   -P ${CMAKE_CURRENT_SOURCE_DIR}/../common/cmake/version.cmake
    )
 
    add_dependencies(zephyr_interface version_cmn)

    if (TEST)
	    target_sources(app PRIVATE test/main.c ${api_sources})
    endif()

endif()

