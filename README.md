# chip_atsame51

Kvasir chip package for the **Microchip ATSAME51J18A**: Cortex-M4F at up to 120 MHz, 256 KiB
flash, 128 KiB RAM, 64 pins (PORTA and PORTB), two CAN-FD controllers, six SERCOMs and a
32-channel DMAC.

Nothing builds here. `cmake/chip.cmake` is an `include()` fragment the Kvasir SDK pulls in
through `CHIP_ROOT`, the peripheral headers are generated from `chip.svd` into the consumer's
binary directory, and `src/chip/*.hpp` is hand-written. To exercise the package, build
something that uses it:

    cd ../smart_hive && just chip_root=$PWD/../chip_atsame51 build

## Layout

| Path | What |
| --- | --- |
| `chip.svd` | the part's CMSIS SVD, the source of `peripherals/*.hpp` |
| `cmake/chip.cmake` | MPU, memory sizes, the linker script, `svd_convert()` |
| `linker/chip.ld` | flash at 0, RAM at 0x20000000; there is no EEPROM region on this family |
| `core/` | `core_cortex_m4` (below) |
| `src/chip/atsam_common/` | `chip_atsam_common`, shared with chip_atsamd21 and chip_atsamc21 |
| `src/chip/*.hpp` | what is specific to this part (below) |

`src/chip/` holds the part's own tables, each checked against the SAM D5x/E5x family data sheet
(DS60001507N) and citing the table it came from: `Interrupt.hpp` (the vector table, Table 10-1),
`Io.hpp` (which pads the 64-pin package bonds, Table 6-1), `GCLK.hpp` (generator sources and the
peripheral channel map, Tables 14-4 and 14-9), `MCLK.hpp` and `PM.hpp` (clock gating, 15.8.6 to
15.8.10, and the reset cause out of RSTC, 16.8.1), `Sercom_Traits.hpp` and `CAN_Traits.hpp` (pin
multiplexing, Table 6-1), `NVMCTRL_Traits.hpp` (chapter 25), `Dmac_Traits.hpp` and
`Serial_Number_Traits.hpp` (9.6).

## The core

`core/` is `core_cortex_m4`, written for this package: there was a Cortex-M0+ and a Cortex-M33
core, and no Armv7-M one. It follows `core_cortex_m33` - the same files, the shared
`core_cortex_common` underneath - minus what is Armv8-M only: no SecureFault, no MSPLIM, no
stack-overflow UsageFault. `Startup::Core::startup()` opens the FPU (CPACR cp10/cp11), because
the core is built for the hard-float ABI.

`core/core.svd` describes three implemented NVIC priority bits (`nvicPrioBits = 3`, eight
levels), which is this family's number and not a property of the Cortex-M4; `core_cortex_common`
takes the range of `isrPriority` from that field. Another Cortex-M4 part would need its own
value there.

One thing the SDK still assumes: `Kvasir::Fault::Handler` decides "has a stack limit register"
by "is not Thumb-1", which is true of the M33 and not of the M4. It does no harm, because the
entry points that touch MSPLIM are only instantiated for a secondary core's stack.

## What this package does not offer yet

- **EIC.** `atsam_common/EIC.hpp` expects one `Interrupt::eic` line; this part has sixteen, one
  per EXTINT. With it go `SamPushButton` and `SamRotaryEncoder`.
- **The event system.** `atsam_common/EVSYS.hpp` is written against the D21's `CTRL` and
  `CHANNEL_TRIG`; this part carries the later one with indexed `CHANNEL[n]`/`USER[n]`.
- **The fuses.** `atsam_common/Fuses.hpp` uses the D21's NVM user-row layout. The user row here
  is laid out differently (9.4) and is erased by page, not by block (25.6.2).
- **The bootloader, by itself.** `atsam_common/Bootloader.hpp` is not in `chip.hpp`: it includes
  `kvasir/Util/Can.hpp`, `kvasir/Util/Bootloader_td.hpp` and (for the bootloader proper)
  `kvasir/Util/Bootloader.hpp`, which the SDK no longer has. smart_hive keeps copies under its
  own `src/kvasir/Util/` and includes `chip/atsam_common/Bootloader.hpp` itself; with those on
  the include path it works as it is.

## Status

Compiles and links (smart_hive: both firmwares and their CAN bootloaders, all variants, warning-free). Nothing in this package has been run on hardware
since it was rewritten; the clock, pin and interrupt tables were checked against the data sheet,
not against a board. The old `kvasir_atsame51j18a` package this replaces had copied its SERCOM
pin table from the D21 - that table is new here.
