# Usage

Use /dev/mem to carry out MMIO read/write.

# Platform Arguments

## Required Argument
```
--start-address   Specify the start address
--address-span    Specify the address span
```
## Optional Argument
```
--dfl-entry-address   Scan DFL start from the specified address. Without DFL, only single interface is set up.
--devmem-driver-path  Override the default path, /dev/mem
--show-dbg-msg        Turn on debug message print. NOTE: Debug messages need to be added during compilation by defining macro INTEL_FPGA_MSG_PRINTF_ENABLE_DEBUG
```
