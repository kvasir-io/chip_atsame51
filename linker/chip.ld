/* Linker script for ATSAME51J18A cortex-m4 */

MEMORY{
    flash  (xr ) : ORIGIN = 0x00000000, LENGTH = 256K
    ram    (xrw) : ORIGIN = 0x20000000, LENGTH = 128K
}

INCLUDE common_flash.ld
INCLUDE common_ram.ld

INCLUDE common.ld
