file(GLOB cpp_FILES *.cpp)

add_executable(fpga_ip_access_api_dfl_walker_utst ${cpp_FILES})

target_link_libraries(fpga_ip_access_api_dfl_walker_utst LINK_PUBLIC fpga_ip_access_lib_common fpga_ip_access_lib gtest dl pthread)
target_include_directories(fpga_ip_access_api_dfl_walker_utst PUBLIC "$<TARGET_PROPERTY:fpga_ip_access_lib,INTERFACE_INCLUDE_DIRECTORIES>")

