#pragma once
// The SAM D5x/E5x's core clock from a target (atsam_common/ClockTree.hpp's skeleton over this chip's builders), like
// the D21's and the C21's: crystal (or a clock on XIN) -> XOSC0/1 -> FDPLL0 -> generator 0, or the DFLL48M in open loop
// as a reset leaves it. DS60001507N, "l." = line of SAMD5x_E5x_Family_Datasheet.md; errata DS80000748T.
//
//     struct ClockSettings {
//         using Tree = Kvasir::Clocks::Sam::Tree<{.cpu = 120'000'000, .source = {.hz = 8'000'000}}>;
//         using Provides = brigand::append<Tree::Provides, brigand::list<... the SERCOM/CAN clocks routed ...>>;
//         static void coreClockInit() { Tree::coreClockInit(); }
//     };
//     static_assert(ClockSettings::Tree::dpll.div == 1 && ClockSettings::Tree::dpll.ldr == 59);
//
// A firmware's own FDPLL numbers instead of the solver's (asserted against Table 54-53 like the solver's):
//
//     Tree<{.cpu = 120'000'000, .source = {.hz = 8'000'000}, .dpll = DPLL::checkedSetting<8'000'000, 2, 89>()}>
//
// The flash: NVMCTRL.CTRLA.AUTOWS by default ("the module can operate at any frequency up to the device maximum
// frequency", 25.8.1 l.27326; one cycle of latency at least); `.flash = {.automatic = false, ...}` writes CTRLA.RWS
// from Table 54-38/54-39 (ClockLimits.hpp) with AUTOWS 0. The CMCC (instruction/data cache, CTRL.CEN, 11.10.3 l.3847) is on
// unless `.cache = false`.

#include "ClockLimits.hpp"
#include "Dpll.hpp"
#include "GCLK.hpp"
#include "WaitBounds.hpp"
#include "Xosc.hpp"
#include "atsam_common/ClockTree.hpp"
#include "kvasir/Register/Register.hpp"
#include "kvasir/Register/Utility.hpp"
#include "peripherals/CMCC.hpp"
#include "peripherals/NVMCTRL.hpp"
#include "peripherals/OSCCTRL.hpp"

#include <cstdint>
#include <type_traits>

namespace Kvasir { namespace Clocks { namespace Sam {
    struct FlashTiming {
        bool                    automatic = true;   // CTRLA.AUTOWS, else CTRLA.RWS
        ClockLimits::E5x::Flash table
          = ClockLimits::E5x::Flash::standard;   // the part's EFP or not (board fact)
        ClockLimits::Supply supply = ClockLimits::Supply::from2V7;   // board fact
    };

    struct TreeConfig {
        std::uint64_t cpu{};
        Source        kind = Source::xoscFdpll;
        Xosc          source{};         // xoscFdpll: the crystal or XIN clock
        unsigned      xoscNumber = 0;   // XOSC0 or XOSC1 as FDPLL0's reference
        DPLL::Setting dpll{};   // .found: these numbers (DPLL::checkedSetting), else the solver's
        std::uint64_t tolerancePpm   = 0;
        bool          dpllRunStandby = false;
        bool dpllLockBypass = true;   // errata 2.13.1 (Dpll.hpp): LBYPASS, and the 10 ms settle
        bool dpllWakeUpFast = true;   // errata 2.13.1: WUF
        XOSC::Options xosc  = {};     // .crystal comes from .source
        FlashTiming   flash = {};
        bool          cache = true;   // CMCC.CTRL.CEN (11.10.3)
    };

    namespace Detail {
        // CTRLA.AUTOWS, or CTRLA.RWS with AUTOWS 0 (25.8.1, l.27281/27326), then the cache - each its own apply(), and
        // all of it before the caller switches generator 0: the flash must be ready for the faster clock first
        template<TreeConfig C,
                 unsigned   WaitStates>
        inline void flashAndCache() {
            using NVM = Kvasir::Peripheral::NVMCTRL::Registers<>;
            if constexpr(C.flash.automatic) {
                apply(set(NVM::CTRLA::autows));
            } else {
                static_assert(WaitStates <= 15, "NVMCTRL.CTRLA.RWS is 4 bits (25.8.1)");
                using F = typename std::remove_cvref_t<decltype(NVM::CTRLA::rws)>::DataType;
                apply(write(NVM::CTRLA::rws, Kvasir::Register::value<F, WaitStates>()),
                      clear(NVM::CTRLA::autows));
            }
            if constexpr(C.cache) { apply(set(Kvasir::Peripheral::CMCC::Registers<>::CTRL::cen)); }
        }

        template<TreeConfig C>
        consteval DPLL::Setting dpllOf() {
            if constexpr(C.dpll.found) {
                return C.dpll;
            } else {
                return DPLL::fromXosc<C.source.hz, C.cpu, C.tolerancePpm>();
            }
        }
    }   // namespace Detail

    template<TreeConfig C, Source = C.kind>
    struct E5xKind;

    // XOSCn -> FDPLL0 (solver: lowest DCO, integer mode; or .dpll) -> generator 0, in this order: oscillator
    // up (XOSCRDY), FDPLL0 running (CLKRDY), errata 2.13.1's settle, the flash and the cache, then the switch.
    template<TreeConfig C>
    struct E5xKind<C, Source::xoscFdpll> {
        static_assert(C.xoscNumber < 2,
                      "the E5x has XOSC0 and XOSC1");
        static constexpr DPLL::Setting dpll = Detail::dpllOf<C>();

        // a .dpll of its own must make f_CPU from this crystal: f_ref = f_XOSC / (2 (DIV + 1)) (28.8.14, l.33535),
        // f_CPU = f_ref (LDR + 1 + LDRFRAC / 32) / gclkDiv (28.6.5) - what checkedSetting computed for its XoscHz
        static_assert(
          !C.dpll.found
            || (dpll.ref.num * 2 * (dpll.div + 1) == C.source.hz * dpll.ref.den
                && dpll.achieved.num == C.cpu * dpll.achieved.den),
          "the FDPLL setting does not make .cpu from .source.hz (DPLL::checkedSetting's XoscHz)");

        static constexpr XOSC::Options xoscOptions = [] {
            auto o    = C.xosc;
            o.crystal = C.source.crystal;
            return o;
        }();

        using ExtraProvides = brigand::list<>;

        template<unsigned WaitStates>
        static void start() {
            using Kvasir::Register::isSet;
            using Kvasir::Register::waitUntil;
            using OSCC = Kvasir::Peripheral::OSCCTRL::Registers<>;

            apply(XOSC::configure<C.xoscNumber, C.source.hz, xoscOptions>());
            waitUntil<Kvasir::Chip::Sam::XoscReadyBound<xoscOptions.startup>>(
              isSet(XOSC::readyFlag<C.xoscNumber>));

            apply(
              DPLL::configure<0,
                              dpll,
                              C.xoscNumber == 0 ? DPLL::Reference::xosc0 : DPLL::Reference::xosc1,
                              DPLL::Options{.lockBypass = C.dpllLockBypass,
                                            .wakeUpFast = C.dpllWakeUpFast,
                                            .runStandby = C.dpllRunStandby}>());
            waitUntil<Kvasir::Chip::DpllReadyBound>(isSet(OSCC::DPLL<0>::STATUS::clkrdy));
            if constexpr(C.dpllLockBypass) { DPLL::settle<0>(); }

            Detail::flashAndCache<C, WaitStates>();
            apply(
              GCLK::GenericClockGenerator<0, GCLK::GeneratorSource::dpll0, dpll.gclkDiv>::enable());
        }
    };

    // DFLL48M in open loop: where a reset leaves generator 0, and where Start.hpp's parkMainClock() puts it back
    // (GENCTRL0 reset SRC 6, 14.8.3): only the flash and the cache are set up
    template<TreeConfig C>
    struct E5xKind<C, Source::dfllOpenLoop> {
        static_assert(C.cpu == 48'000'000,
                      "the E5x DFLL48M open loop as the reset leaves it: generator 0 /1, 48 MHz");

        using ExtraProvides = brigand::list<>;

        template<unsigned WaitStates>
        static void start() {
            Detail::flashAndCache<C, WaitStates>();
        }
    };

    struct E5xBuilders {
        using Config = TreeConfig;

        static consteval bool has(Source s) {
            return s == Source::xoscFdpll || s == Source::dfllOpenLoop;
        }

        // f_CPU max 120 MHz (Table 54-6, l.85579)
        static consteval std::uint64_t cpuMax(TreeConfig const&) {
            return ClockLimits::E5x::cpuMax;
        }

        // 0 under AUTOWS (CTRLA.RWS is not used then); else the row of the clock the flash sees at worst: the open
        // loop up to ClockLimits::E5x::DfllOpenLoopMaxHz, an FDPLL output is locked to its crystal
        template<TreeConfig C>
        static consteval unsigned waitStates() {
            if constexpr(C.flash.automatic) {
                return 0;
            } else {
                constexpr std::uint64_t hz
                  = C.kind == Source::dfllOpenLoop ? ClockLimits::E5x::dfllOpenLoopMaxHz : C.cpu;
                return ClockLimits::E5x::waitStates<hz, C.flash.supply, C.flash.table>();
            }
        }

        template<TreeConfig C>
        using Kind = E5xKind<C>;
    };

    template<TreeConfig C>
    using Tree = TreeOf<E5xBuilders, C>;
}}}   // namespace Kvasir::Clocks::Sam
