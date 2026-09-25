include(${CMAKE_CURRENT_LIST_DIR}/../core/cmake/core.cmake)

set(TARGET_MPU ATSAME51J18A)
set(TARGET_FLASH_SIZE 262144)
set(TARGET_RAM_SIZE 131072)
# No separate EEPROM region: the D5x/E5x has no RWW section. An application that wants emulated EEPROM carves it out of
# main flash in a linker file of its own (see smart_hive's linker/*.ld.in).
set(TARGET_EEPROM_SIZE 0)

# The SDK writes a UF2 next to every image and passes this through as the family id. The registry
# (github.com/microsoft/uf2, utils/uf2families.json) has no SAME51 entry; SAMD51 is the same silicon family and what the
# UF2 bootloaders for the E51 identify as.
set(TARGET_UF2_CODE 0x55114460)

set(LINKER_FILE ${CMAKE_CURRENT_LIST_DIR}/../linker/chip.ld)

svd_convert(peripherals SVD_FILE ${CMAKE_CURRENT_LIST_DIR}/../chip.svd OUTPUT_DIRECTORY peripherals)

# kvasir_devices: chip.hpp includes its drivers unconditionally (Sercom_I2CQueued.hpp ->
# kvasir/Devices/I2C/LineRecovery.hpp), so every image needs it. Found like CHIP_ROOT (KVASIR_DEVICES_ROOT: variable,
# environment, else next to the SDK); the SDK adds it after project() unless the firmware has it already.
kvasir_resolve_root(KVASIR_DEVICES_ROOT kvasir_devices)
kvasir_add_package(${KVASIR_DEVICES_ROOT} kvasir_devices kvasir_devices)
target_link_libraries(peripherals INTERFACE kvasir::devices)
