include(${CMAKE_CURRENT_LIST_DIR}/../core/cmake/core.cmake)

# Kvasir's write semantics (svd_convert's defaults): a new unclassified write-only field stops the build
kvasir_chip_svd(peripherals ${CMAKE_CURRENT_LIST_DIR}/../chip.svd)

# the chip's part of the contract (Kvasir_SDK cmake/kvasir_chip.cmake); DEVICES: chip/drivers.hpp builds on
# kvasir_devices, which the SDK then adds after project()
kvasir_chip(
    DEVICES
    MPU
    ATSAME51J18A
    # The SDK writes a UF2 next to every image with this family id. The registry (github.com/microsoft/uf2,
    # utils/uf2families.json) has no SAME51 entry; SAMD51 is the same silicon family and what the UF2 bootloaders for
    # the E51 identify as.
    UF2_FAMILY
    0x55114460
    # linker/chip.ld has no EEPROM region: the D5x/E5x has no RWW section. An application that wants emulated EEPROM
    # carves it out of main flash in a linker file of its own.
    LINKER_FILE
    ${CMAKE_CURRENT_LIST_DIR}/../linker/chip.ld
    INCLUDE_DIR
    ${CMAKE_CURRENT_LIST_DIR}/../src
    REGISTERS
    peripherals
    # What src/chip/Variant.hpp reads: the part, the series and the family, for every TU of the image.
    DEFINITIONS
    KVASIR_CHIP_ATSAME51J18A=1
    KVASIR_CHIP_ATSAME51=1
    KVASIR_CHIP_FAMILY_SAM=1)
