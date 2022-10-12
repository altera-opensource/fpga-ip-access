cmake_minimum_required(VERSION 3.0.0)

# Set the FPGA_IP_ACCESS_OPTIONS name on the parent variable
# use "cmake -DBUILD_FPGA_IP_ACCESS_MODE=DEVMEM" to set the option (Note: This is assumed to be the default mode). Can also be set with ccmake and vscode cmake options.
set(FPGA_IP_ACCESS_OPTIONS ${FPGA_IP_ACCESS_OPTIONS} DEVMEM PARENT_SCOPE)

# Only include if the BUILD_FPGA_IP_ACCESS_MODE is set to DEVMEM (case sensitive)
if( BUILD_FPGA_IP_ACCESS_MODE STREQUAL DEVMEM)
    message("INFO: Target Platform: Linux devmem driver")
    file(GLOB c_FILES src/*.c)

    add_library(${PROJECT_NAME} ${c_FILES})
    add_library(${PROJECT_NAME}_sw_tst ${c_FILES})
    set_target_properties (${PROJECT_NAME}_sw_tst PROPERTIES COMPILE_DEFINITIONS "DEVMEM_UNIT_TEST_SW_MODEL_MODE")

    target_include_directories(${PROJECT_NAME} PUBLIC inc)
    target_include_directories(${PROJECT_NAME} PUBLIC "$<TARGET_PROPERTY:${PROJECT_NAME}_common,INTERFACE_INCLUDE_DIRECTORIES>")
    target_include_directories(${PROJECT_NAME}_sw_tst PUBLIC inc)
    target_include_directories(${PROJECT_NAME}_sw_tst PUBLIC "$<TARGET_PROPERTY:${PROJECT_NAME}_common,INTERFACE_INCLUDE_DIRECTORIES>")

    if(${TEST})
        add_subdirectory(test)
        add_subdirectory(test-dfl)
    endif()
endif()

