# Introduction

This library implements the IP Access API for Intel FPGA using the Linux UIO (Userspace I/O) driver.

Currently, this library only handles one UIO path.

# CMake Option

cmake -DBUILD_FPGA_IP_ACCESS_MODE=UIO

NOTE: This option is not strictly required as UIO is the default value of BUILD_FPGA_IP_ACCESS_MODE if the default is unchanged in the parent CMakeLists.txt.

# Arguments of fpga_platform_init() 

 --uio-driver-path=<path>, -u <path>           UIO driver path (default: /dev/uio0).
 --start-address=<address>, -a <address>       Starting address within this UIO driver (default: 0).
 --address-span=<size>, -s <size>              Address span of the UIO. The value is obtained from sysfs if available, for example, /sys/class/uio/uio0/maps/map0/size. Otherwise, this is a required argument.
 --show-dbg-msg, -d                            Show debug message.
