#pragma once
// The SAM D5x/E5x's two FDPLL200Ms as building blocks, in the C21 package's shape: the numbers come from the solver
// (atsam_common/ClockSolver.hpp on ClockLimits.hpp) or from a firmware's own checked numbers, the register writes are
// these (DS60001507N 28.8.12-28.8.16, "l." = line of the converted data sheet; errata DS80000748T).
//
//     constexpr auto dpll = Kvasir::DPLL::fromXosc<8'000'000, 120'000'000>();   // DIV 1, LDR 59
//     apply(Kvasir::DPLL::configure<0, dpll>());
//     Kvasir::Register::waitUntil<Kvasir::Chip::DpllReadyBound>(
//       Kvasir::Register::isSet(Kvasir::Peripheral::OSCCTRL::Registers<>::DPLL<0>::STATUS::clkrdy));
//     Kvasir::DPLL::settle<0>();   // errata 2.13.1's 10 ms, before a generator takes the clock
//     apply(Kvasir::GCLK::GenericClockGenerator<0, Kvasir::GCLK::GeneratorSource::dpll0, dpll.gclkDiv>::enable());

#include "DpllSetting.hpp"
#include "WaitBounds.hpp"
#include "kvasir/Register/Register.hpp"
#include "kvasir/Register/Utility.hpp"
#include "peripherals/OSCCTRL.hpp"

#include <cstdint>
#include <type_traits>

namespace Kvasir { namespace DPLL {
    // DPLLCTRLB.REFCLK (28.8.14, l.33593): the dedicated GCLK, XOSC32K (the reset value), XOSC0, XOSC1
    enum class Reference : unsigned { gclk = 0, xosc32k = 1, xosc0 = 2, xosc1 = 3 };

    struct Options {
        // Errata 2.13.1 "FDPLL unlocks could occur while the output frequency is stable" (revisions A and D, PDF page
        // 34; F and G not affected): LBYPASS = 1 and WUF = 1, "the clock is issued even if the FDPLL status shows
        // unlocked", then wait for DPLLSTATUS.CLKRDY and "a 10 ms delay" (settle()) before a generator takes the
        // DPLL. On by default, as on the C21/D21: the revision is only known at run time (DSU.DID), and the
        // workaround costs the settle once per boot. LBYPASS = 1 also rules out an on-the-fly RATIO update
        // (28.6.5.1 "Loop Divider Ratio Updates", l.32082): configure() writes RATIO before ENABLE only.
        bool lockBypass = true;
        bool wakeUpFast = true;
        bool runStandby = false;
    };

    // DPLLnRATIO and DPLLnCTRLB (Enable-Protected, 28.8.14, l.33513: written while the DPLL is off), then DPLLnCTRLA
    // with ENABLE and ONDEMAND off (28.8.12). The writes of one step go out by address, highest first (CTRLB 0x38,
    // RATIO 0x34), CTRLA (0x30) after the sequence point. The caller waits for DPLLSTATUS.CLKRDY (ready()) and, with
    // the errata options, settle()s before a generator takes the clock.
    template<unsigned  N,
             Setting   S,
             Reference R = Reference::xosc0,
             Options   O = {}>
    [[nodiscard]] constexpr auto configure() {
        static_assert(N < 2, "the E5x has FDPLL0 and FDPLL1");
        static_assert(S.found, "DPLL::configure: a setting the solver did not find");
        static_assert(S.presc == 0, "the E5x FDPLL200M has no output prescaler");
        static_assert(R == Reference::xosc0 || R == Reference::xosc1 || S.div == 0,
                      "DIV only divides an XOSC reference");
        using D = typename Kvasir::Peripheral::OSCCTRL::Registers<>::template DPLL<N>;
        using Kvasir::Register::value;
        using RV         = typename D::CTRLB::REFCLKVal;
        auto const ratio = list(write(D::RATIO::ldr, value<S.ldr>()),
                                write(D::RATIO::ldrfrac, value<S.ldrFrac>()));
        // an option bit is written 1 or 0; 0 is its reset value, so the word is the one a missing write gave
        using B = typename std::remove_cvref_t<decltype(D::CTRLB::lbypass)>::DataType;
        using A = typename std::remove_cvref_t<decltype(D::CTRLA::runstdby)>::DataType;
        auto const ctrlb
          = D::CTRLB::overrideDefaults(write(D::CTRLB::div, value<S.div>()),
                                       write(D::CTRLB::lbypass, value<B, O.lockBypass ? 1U : 0U>()),
                                       write(D::CTRLB::wuf, value<B, O.wakeUpFast ? 1U : 0U>()),
                                       write(D::CTRLB::refclk, value<RV, static_cast<RV>(R)>()));
        auto const ctrla = D::CTRLA::overrideDefaults(
          set(D::CTRLA::enable),
          write(D::CTRLA::runstdby, value<A, O.runStandby ? 1U : 0U>()),
          clear(D::CTRLA::ondemand));
        return list(ratio, ctrlb, Kvasir::Register::sequencePoint, ctrla);
    }

    // DPLLnSTATUS.CLKRDY (28.8.16, l.33680): the output clock is on. With LBYPASS it says nothing about lock or
    // frequency (errata 2.13.1).
    template<unsigned N>
    [[nodiscard]] inline bool ready() {
        using D = typename Kvasir::Peripheral::OSCCTRL::Registers<>::template DPLL<N>;
        return static_cast<std::uint32_t>(get<0>(apply(read(D::STATUS::clkrdy)))) != 0;
    }

    // Errata 2.13.1's "10 ms delay ... after the clock ready bit is set": a fixed count of DPLLnSTATUS reads
    // (Chip::DpllSettlePolls, at least 10 ms at any core clock up to 120 MHz), registers only - so it may run in
    // ClockSettings::coreClockInit(), before memory is initialised.
    template<unsigned N>
    inline void settle() {
        using D = typename Kvasir::Peripheral::OSCCTRL::Registers<>::template DPLL<N>;
        for(std::uint32_t n = 0; n != Kvasir::Chip::dpllSettlePolls; ++n) {
            static_cast<void>(apply(read(D::STATUS::clkrdy)));
        }
    }
}}   // namespace Kvasir::DPLL
