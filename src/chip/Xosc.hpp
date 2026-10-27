#pragma once
// The SAM D5x/E5x's two crystal oscillators XOSC0/XOSC1 as a building block, in the D21/C21 packages' shape
// (`Kvasir::XOSC`): the register write and the ready bit (DS60001507N 28.8.6 XOSCCTRLn and 28.8.5 STATUS, l.33003 / l.32853, "l." = line
// of SAMD5x_E5x_Family_Datasheet.md), the current setting from the frequency (ClockLimits.hpp `xoscCurrentFor`).
//
//     apply(Kvasir::XOSC::configure<0, 8'000'000, {.startup = 0xE}>());
//     Kvasir::Register::waitUntil<Kvasir::Chip::Sam::XoscReadyBound<0xE>>([] { return Kvasir::XOSC::ready<0>(); });

#include "ClockLimits.hpp"
#include "kvasir/Register/Register.hpp"
#include "kvasir/Register/Utility.hpp"
#include "peripherals/OSCCTRL.hpp"

#include <cstdint>
#include <type_traits>

namespace Kvasir { namespace XOSC {
    struct Options {
        bool crystal    = true;    // XTALEN: a crystal between XIN and XOUT, else a clock on XIN
        bool runStandby = false;   // RUNSTDBY
        std::uint16_t startup
          = 6;   // STARTUP 0x6: 64 OSCULP32K cycles, ~1953 us (Table 28-6, l.33033 ff.)
        bool lowBufGain
          = false;            // LOWBUFGAIN: only with ENALC; "the default value ... should be used"
        bool enalc = false;   // ENALC: automatic amplitude loop control
    };

    // XOSCCTRLn with ENABLE, ONDEMAND off (it runs whether or not a generator asks for it), the clock failure
    // detector off; IMULT/IPTAT from Table 28-7 for a crystal, 0 (their reset value) for a clock on XIN, which drives
    // no crystal. The rest of the word at its reset value (0x00000080: ONDEMAND, cleared here).
    template<unsigned      N,
             std::uint64_t Hz,
             Options       O = {}>
    [[nodiscard]] constexpr auto configure() {
        static_assert(N < 2, "the E5x has XOSC0 and XOSC1");
        static_assert(O.startup <= 15, "XOSCCTRL.STARTUP is 4 bits (Table 28-6)");
        static_assert(O.crystal ? Hz >= ClockLimits::E5x::xoscCrystalMinHz
                                    && Hz <= ClockLimits::E5x::xoscCrystalMaxHz
                                : Hz != 0 && Hz <= ClockLimits::E5x::xoscXinMaxHz,
                      "SAM E5x XOSC: a crystal of 8..48 MHz (Table 54-44) or an XIN clock up to 48 "
                      "MHz (Table 54-43)");
        using X = typename Kvasir::Peripheral::OSCCTRL::Registers<>::template XOSCCTRL<N>;
        using Kvasir::Register::value;
        using F = typename std::remove_cvref_t<decltype(X::imult)>::DataType;
        using S = typename X::STARTUPVal;
        constexpr auto current
          = O.crystal ? ClockLimits::E5x::xoscCurrentFor(Hz) : ClockLimits::E5x::XoscCurrent{};
        constexpr auto bit = [](bool b) { return b ? 1U : 0U; };
        return X::overrideDefaults(write(X::startup, value<S, static_cast<S>(O.startup)>()),
                                   write(X::imult, value<F, current.imult>()),
                                   write(X::iptat, value<F, current.iptat>()),
                                   write(X::lowbufgain, value<F, bit(O.lowBufGain)>()),
                                   write(X::enalc, value<F, bit(O.enalc)>()),
                                   clear(X::ondemand),
                                   write(X::runstdby, value<F, bit(O.runStandby)>()),
                                   write(X::xtalen, value<F, bit(O.crystal)>()),
                                   set(X::enable));
    }

    // STATUS.XOSCRDYn (28.8.5, l.32853): the oscillator is stable and ready - the field, for waitUntil(isSet(...))
    template<unsigned N>
    inline constexpr auto readyFlag = [] {
        static_assert(N < 2, "the E5x has XOSC0 and XOSC1");
        using St = Kvasir::Peripheral::OSCCTRL::Registers<>::STATUS;
        if constexpr(N == 0) {
            return St::xoscrdy0;
        } else {
            return St::xoscrdy1;
        }
    }();

    template<unsigned N>
    [[nodiscard]] inline bool ready() {
        return static_cast<std::uint32_t>(get<0>(apply(read(readyFlag<N>)))) != 0;
    }
}}   // namespace Kvasir::XOSC
