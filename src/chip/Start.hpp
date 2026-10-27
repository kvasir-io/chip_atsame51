#pragma once
// The SAM D5x/E5x's start traits for atsam_common/StartUp.hpp's FirstInitStep: whether the chip looks as a reset leaves it
// (restartIfNotFromReset() restarts an image at address 0 that was jumped into), and GCLK0 parked on the clock a reset
// gives it before the clock tree is rewritten.

#include "GCLK.hpp"
#include "peripherals/MCLK.hpp"
#include "peripherals/NVMCTRL.hpp"

#include <cstdint>

namespace Kvasir { namespace Startup { namespace Sam {
    struct StartTraits {
        // Whether the chip looks as a reset leaves it (DS60001507N, "l." = line of the converted data sheet). A reset of
        // any kind but the backup ones (POR, external, WDT, SysResetReq: Table 16-1 "Others: Reset") leaves APBAMASK at
        // 0x000007FF (15.8.7, l.7546), APBCMASK at 0x00002000 (15.8.9, l.7813), APBDMASK at 0 (15.8.10, l.7939) - header
        // and bit table agree for those three - and GCLK0 on DFLL48M (GENCTRL0 reset 0x00000106, SRC 6, 14.8.3, l.6458).
        // APBBMASK's header (0x00018056, 15.8.8, l.7699) and its bit table (0x00010016) disagree on bits 6 and 15, so
        // only the bits both call 0 are compared there: USB, EVSYS, SERCOM2/3, TCC0/1, TC2/3. Datasheet values, not yet
        // read on a board. An image that used a SERCOM, TC, TCC, ADC, DAC, ... or ran GCLK0 on another source leaves a
        // difference behind, and a jump resets none of it. The masks are whole-word snapshots (runtime flag masks).
        static bool looksLikeAfterReset() {
            using GCLK::GeneratorSource;
            using MC                     = Kvasir::Peripheral::MCLK::Registers<>;
            auto const              apba = Kvasir::Register::readWhole(MC::APBAMASK::FULLREGISTER);
            auto const              apbb = Kvasir::Register::readWhole(MC::APBBMASK::FULLREGISTER);
            auto const              apbc = Kvasir::Register::readWhole(MC::APBCMASK::FULLREGISTER);
            auto const              apbd = Kvasir::Register::readWhole(MC::APBDMASK::FULLREGISTER);
            constexpr std::uint32_t apbbUsed
              = MC::APBBMASK::usb_.mask | MC::APBBMASK::evsys_.mask | MC::APBBMASK::sercom2_.mask
              | MC::APBBMASK::sercom3_.mask | MC::APBBMASK::tcc0_.mask | MC::APBBMASK::tcc1_.mask
              | MC::APBBMASK::tc2_.mask | MC::APBBMASK::tc3_.mask;
            return apba == 0x0000'07FFU && (apbb & apbbUsed) == 0U && apbc == 0x0000'2000U
                && apbd == 0U && GCLK::gclk0Source() == GeneratorSource::dfll;
        }

        // GCLK0 (the CPU's clock) onto DFLL48M in open loop before the clock tree is rewritten: an image a bootloader
        // jumps into, or one that jumps into itself, may find GCLK0 on a DPLL or an XOSC the firmware's tree is about to
        // reprogram. After a reset GCLK0 is on the DFLL already (GENCTRL0 reset SRC 6, 14.8.3, l.6458; the DFLL is in
        // open loop, DFLLCTRLB reset 0x00, 28.8.8, l.33229) and this only reads two registers. The switch is on the fly:
        // the generator keeps both requests until source B is ready, SYNCBUSY.GENCTRL0 stays 1 until it is done (14.6.2.6,
        // l.6137-6155) - no "ONDEMAND on source A first" step as on the C21. Before it:
        //   - closed loop (DFLLCTRLB.MODE = 1) goes back to open loop: in closed loop STATUS.DFLLRDY "does not rise before
        //     lock fine occurs" (errata 2.8.2, all revisions, PDF page 29), and the reference generator may be one the tree
        //     is about to rewrite. DFLLCTRLB is read directly: DFLLSYNC.DFLLCTRLB "only applies for write operations"
        //     (28.8.11, l.33411);
        //   - the DFLL enabled with ONDEMAND = 0: DFLLCTRLA resets to 0x82, ENABLE | ONDEMAND (28.8.7, l.33186), and an
        //     on-demand DFLL nobody requests "will be in a disabled state" - waiting for DFLLRDY then waits for ever (the
        //     D21's OSC8M trap). ONDEMAND goes back to 1, its reset state, once GCLK0 requests the DFLL;
        //   - the DFLL's flags (INTFLAG DFLLRDY, DFLLOOB, DFLLLCKF, DFLLLCKC, DFLLRCS, write-one-to-clear, 28.8.4) cleared
        //     before and after the enable (errata 2.8.6, all revisions, PDF page 30); clearing MODE also resets the lock
        //     bits a closed loop left (errata 2.8.5).
        //   - "on the DFLL in open loop" also means undivided: GENCTRL0 DIVSEL 0 with DIV 0 or 1 (14.8.3, l.6458; DIVSEL
        //     "must be 0 and the GENCTRLn.DIV value must be either 0 or 1" for no division, bit 12, l.6507-6509) and MCLK
        //     CPUDIV 1 (15.8.5, l.7302: reset 0x01). Both are the reset's; an earlier image may have left a divided clock
        //     (the dfllOpenLoop Tree writes neither), so a divided one is switched like any other source - the generator
        //     write is DIVSEL 0 / DIV 1 - and CPUDIV goes back to 1, then INTFLAG.CKRDY reads 1 again (15.6.2.5, l.6959;
        //     CPUDIV "must not be re-written while INTFLAG.CKRDY reads 0");
        //   - the park can raise the CPU clock (a divided or slower source before it): AUTOWS goes on first, its reset
        //     state (NVMCTRL CTRLA reset bit 2 = 1, 25.8.1, l.27243-27246; "can operate at any frequency up to the device
        //     maximum frequency", l.27326-27330) - an earlier image's RWS may be the 0 WS of a slower clock. The tree
        //     sets its own wait states after the park.
        // Open-loop start-up is at most 7 us (Table 54-50, l.86909). Every wait is bounded (WaitBounds.hpp).
        static void parkMainClock() {
            using GCLK::GeneratorSource;
            using Osc = Kvasir::Peripheral::OSCCTRL::Registers<>;
            using Kvasir::Register::isClear;
            using Kvasir::Register::isSet;
            using Kvasir::Register::waitUntil;
            using Gen0            = Kvasir::Peripheral::GCLK::Registers<>::GENCTRL<0>;
            using MC              = Kvasir::Peripheral::MCLK::Registers<>;
            using Nvm             = Kvasir::Peripheral::NVMCTRL::Registers<>;
            bool const closedLoop = get<0>(apply(read(Osc::DFLLCTRLB::mode))) != 0;
            auto const gen0       = apply(read(Gen0::src), read(Gen0::divsel), read(Gen0::div));
            bool const cpuDivided
              = get<0>(apply(read(MC::CPUDIV::div))) != MC::CPUDIV::DIVVal::div1;
            bool const gen0Divided = gen0[Gen0::divsel] != Gen0::DIVSELVal::div1
                                  || static_cast<std::uint32_t>(gen0[Gen0::div]) > 1U;
            if(gen0[Gen0::src] == GeneratorSource::dfll && !closedLoop && !gen0Divided
               && !cpuDivided)
            {
                return;
            }

            apply(set(Nvm::CTRLA::autows));

            auto const clearDfllFlags = [] {
                apply(reset(Osc::INTFLAG::dfllrdy,
                            Osc::INTFLAG::dflloob,
                            Osc::INTFLAG::dflllckf,
                            Osc::INTFLAG::dflllckc,
                            Osc::INTFLAG::dfllrcs));
            };
            if(closedLoop) {
                apply(clear(Osc::DFLLCTRLB::mode));
                waitUntil<Kvasir::Chip::Sam::SyncBound>(isClear(Osc::DFLLSYNC::dfllctrlb));
            }
            clearDfllFlags();
            apply(set(Osc::DFLLCTRLA::enable), clear(Osc::DFLLCTRLA::ondemand));
            waitUntil<Kvasir::Chip::Sam::SyncBound>(isClear(Osc::DFLLSYNC::enable));
            waitUntil<Kvasir::Chip::DfllReadyBound>(isSet(Osc::STATUS::dfllrdy));
            clearDfllFlags();
            apply(GCLK::GenericClockGenerator<0, GeneratorSource::dfll, 1>::enable());
            waitUntil<Kvasir::Chip::Sam::SyncBound>([] { return !GCLK::gclk0Syncing(); });
            if(cpuDivided) {
                apply(
                  write(MC::CPUDIV::div,
                        Kvasir::Register::value<MC::CPUDIV::DIVVal, MC::CPUDIV::DIVVal::div1>()));
                waitUntil<Kvasir::Chip::Sam::SyncBound>(isSet(MC::INTFLAG::ckrdy));
            }
            apply(set(Osc::DFLLCTRLA::ondemand));
        }
    };
}}}   // namespace Kvasir::Startup::Sam
