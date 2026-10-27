# chip_atsame51

Kvasir chip package for the **Microchip ATSAME51J18A**: Cortex-M4F at up to 120 MHz, 256 KiB
flash, 128 KiB RAM, 64 pins (PORTA and PORTB), two CAN-FD controllers, six SERCOMs and a
32-channel DMAC.

Nothing builds here. `cmake/chip.cmake` is an `include()` fragment the Kvasir SDK pulls in
through `CHIP_ROOT`, the peripheral headers are generated from `chip.svd` into the consumer's
binary directory, and `src/chip/*.hpp` is hand-written. To exercise the package, build
something that uses it: a firmware that points its `CHIP_ROOT` at this checkout.

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
peripheral channel map, Tables 14-4 and 14-9), `Blocks.hpp` (the clock gates over atsam_common's `ClockGate.hpp`, 15.8.6 to
15.8.10) and `PM.hpp` (the reset cause out of RSTC, 16.8.1, with `PM::resetKind()` for the SDK's boot guard from
`ResetCause.hpp`), `Sercom_Traits.hpp` and `CAN_Traits.hpp` (pin
multiplexing, Table 6-1), `NVMCTRL_Traits.hpp` (chapter 25), `Dmac_Traits.hpp` and
`Serial_Number_Traits.hpp` (9.6).

- **EIC.** `drivers.hpp` includes `atsam_common/EIC.hpp`, `SamPushButton.hpp` and `SamRotaryEncoder.hpp`, as on the
  D21/C21. One vector per EXTINT line (`EicExtIntVector<x>`, Table 10-1). Which line a pad drives is
  `EIC_Traits.hpp`: the E5x rule of `atsam_common/EicLines.hpp` (Table 6-1: PA08 the NMI, PB26-PB29 -> 12-15,
  otherwise pin % 16 for ports A and B - PA24 is EXTINT8 here, 12 on the D21/C21; ports C/D are not encoded).
  Errata 2.12.1: an edge in the first three GCLK_EIC cycles after the EIC's enable is not seen.
- **Start traits.** `Start.hpp`'s `StartTraits::looksLikeAfterReset()` (APB masks and GCLK0's source against their reset values,
  15.8.7-15.8.10, 14.8.3) lets `atsam_common/StartUp.hpp` restart an image at address 0 that was entered by a jump;
  `parkMainClock()` puts GCLK0 back on the DFLL48M in open loop (DFLL enabled with ONDEMAND off for the switch,
  closed loop left first - errata 2.8.2, 2.8.5, 2.8.6) before a clock tree is rewritten. The reset path only reads.
  `WaitBounds.hpp` has the bounds of the waits, each with its table.
- **FDPLL.** `Dpll.hpp` (registers) and `DpllSetting.hpp` (numbers, host-tested): `fromXosc<XoscHz, Hz>()` (the
  solver) or `checkedSetting<XoscHz, Div, Ldr>()` (a firmware's own numbers, asserted), `configure<N, S, Reference,
  Options>()`, `ready<N>()`, `settle<N>()`. `Options` defaults to errata 2.13.1's workaround (LBYPASS = 1, WUF = 1,
  then 10 ms after CLKRDY before a generator takes the DPLL; revisions A and D).

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

One thing the SDK still assumes: `Kvasir::Fault::HandlerFor` decides "has a stack limit register"
by "is not Thumb-1", which is true of the M33 and not of the M4. It does no harm, because the
entry points that touch MSPLIM are only instantiated for a secondary core's stack.

## What this package does not offer yet

- **The event system.** chip_atsamd21's `Evsys.hpp` is written against the D21's `CTRL` and
  `CHANNEL_TRIG`; this part carries the later one with indexed `CHANNEL[n]`/`USER[n]`.
- **The fuses.** chip_atsamd21's `Fuses.hpp` uses the D21's NVM user-row layout. The user row here
  is laid out differently (9.4) and is erased by page, not by block (25.6.2).

## Status

Compiles and links (two firmwares and their CAN bootloaders, all variants, warning-free).
Nothing in this package has been run on hardware since it was rewritten - the EIC rule, the start traits and the
FDPLL block included; the clock, pin and interrupt tables were checked against the data sheet,
not against a board. The old `kvasir_atsame51j18a` package this replaces had copied its SERCOM
pin table from the D21 - that table is new here.
