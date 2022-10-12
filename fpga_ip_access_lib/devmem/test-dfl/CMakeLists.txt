file(GLOB c_FILES *.c)

add_executable(dfl-scan-devmem ${c_FILES})
set_target_properties (dfl-scan-devmem PROPERTIES COMPILE_DEFINITIONS "DFL_WALKER_REPORT;DFL_WALKER_DEBUG_MODE")

target_link_libraries(dfl-scan-devmem LINK_PUBLIC fpga_ip_access_lib fpga_ip_access_lib_common)
target_include_directories(dfl-scan-devmem PUBLIC "$<TARGET_PROPERTY:fpga_ip_access_lib,INTERFACE_INCLUDE_DIRECTORIES>")
