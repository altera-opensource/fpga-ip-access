# Introduction

IP Access API for Intel FPGAs provides a fundament software access methods to interact with the 
IP component in FPGA with memory mapped interfaces and interrupt interfaces. The memory mapped 
intefaces include Avalone Memory Mapped, AXI, AXI-lite, etc. IP access driver code can be developed
using such API to demonstrates how hardware component can operate in a range of host environments. 
The IP access driver code can be compiled and linked into an executable to create a demonstration
and testing environment in various platforms.

This repository provides the library source code implementing the IP Access API for Intel FPGAs
in various platforms.

# Build Environment

CMake is used to describe the usage below.

In the application main CMakeLists.txt, the IP Access API library can be fetched from github directly. For example,

include(FetchContent)

set(FPGA_IP_ACCESS_API_LIB_GIT_URL "https://github.com/altera-opensource/fpga-ip-access.git" CACHE STRING "URL of the IP Access API for Intel FPGAs library repository")
set(FPGA_IP_ACCESS_API_LIB_GIT_TAG "main" CACHE STRING "Git tag to use for the IP Access API for Intel FPGAs library repository")
FetchContent_Declare(
    FPGA_IP_ACCESS_API_LIB
    GIT_REPOSITORY ${FPGA_IP_ACCESS_API_LIB_GIT_URL}
    GIT_TAG ${FPGA_IP_ACCESS_API_LIB_GIT_TAG}
)
FetchContent_MakeAvailable(FPGA_IP_ACCESS_API_LIB)


# FPGA IP Access Driver Developer Usage

This FPGA IP Access API library can be referenced by FPGA IP access driver using the CMake facility. 

The FPGA IP Access API include directory, "$<TARGET_PROPERTY:fpga_ip_access_lib,INTERFACE_INCLUDE_DIRECTORIES>", should be added.
For example,

```
target_include_directories(my_driver PRIVATE "$<TARGET_PROPERTY:fpga_ip_access_lib,INTERFACE_INCLUDE_DIRECTORIES>")
```

Three header files can be included into the driver code as 
#include "intel_fpga_platform.h"
#include "intel_fpga_platform_api.h"
#include "intel_fpga_api.h"

# Demo Application Developer Usage

This FPGA IP Access API library can be compiled and linked into the application together with ip access drivers. 

The FPGA IP Access API include directory, "$<TARGET_PROPERTY:fpga_ip_access_lib,INTERFACE_INCLUDE_DIRECTORIES>", should be added.
The FPGA IP Access static library, fpga_ip_access_lib fpga_ip_access_lib_common, should also be added. 
For example,

```
target_include_directories(my_demo_exe PRIVATE "$<TARGET_PROPERTY:fpga_ip_access_lib,INTERFACE_INCLUDE_DIRECTORIES>" ${CMAKE_BINARY_DIR})
target_link_libraries(my_demo_exe LINK_PUBLIC fpga_ip_access_lib fpga_ip_access_lib_common my_driver )
```

Three header files can be included into the driver code as 
#include "intel_fpga_platform.h"
#include "intel_fpga_platform_api.h"
#include "intel_fpga_api.h"

Select a specific target platform by name using the option BUILD_FPGA_IP_ACCESS_MODE. 
The platform name can be found in the CMakeLists.txt under each directory. For example, in uio/CMakeLists.txt,
you'll find this line:
```
if( BUILD_FPGA_IP_ACCESS_MODE STREQUAL UIO)
```
"UIO" is the platform name. This platform option can be specified either as part of the cmake command line or within the CMakelist.txt.

Example command line:
```
cmake -Bbuild -DBUILD_FPGA_IP_ACCESS_MODE=UIO
```

Without specifying BUILD_FPGA_IP_ACCESS_MODE option, UIO is the default.

# Documentation

Generate the API documents using the doxygen tool. Follow the method described in the README.md under the doc folder.
