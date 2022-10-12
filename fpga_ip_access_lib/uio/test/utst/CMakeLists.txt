file(GLOB cpp_FILES *.cpp)

add_executable(fpga_ip_access_api_uio_utst ${cpp_FILES})

target_link_libraries(fpga_ip_access_api_uio_utst LINK_PUBLIC fpga_ip_access_lib_sw_tst fpga_ip_access_lib_common gtest dl pthread)
target_include_directories(fpga_ip_access_api_uio_utst PUBLIC "$<TARGET_PROPERTY:fpga_ip_access_lib,INTERFACE_INCLUDE_DIRECTORIES>")

