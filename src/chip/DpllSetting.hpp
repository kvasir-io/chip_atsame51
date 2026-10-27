#pragma once
// The numbers of an E5x FDPLL200M setting, without its registers (Dpll.hpp writes them): host-testable on its own
// (tests/clock_limits_test.cpp).
#include "ClockLimits.hpp"

#include <cstdint>

namespace Kvasir { namespace DPLL {
    template<std::uint64_t XoscHz,
             std::uint64_t Hz,
             std::uint64_t TolerancePpm = 0,
             bool          Fractional   = false>
    consteval Setting fromXosc() {
        return solveXosc<ClockLimits::E5x::fdpll200m,
                         ClockLimits::E5x::dpllWhere,
                         XoscHz,
                         Hz,
                         TolerancePpm,
                         Fractional>();
    }

    // A firmware's own numbers (a crystal through DIV), asserted against the FDPLL200M's limits like the solver's,
    // as a Setting for configure(): for a firmware that keeps numbers the solver would not pick.
    template<std::uint64_t XoscHz,
             std::uint32_t Div,
             std::uint32_t Ldr,
             std::uint32_t LdrFrac = 0,
             std::uint32_t GclkDiv = 1>
    consteval Setting checkedSetting() {
        constexpr auto c = assertXoscSetting<ClockLimits::E5x::fdpll200m,
                                             ClockLimits::E5x::dpllWhere,
                                             XoscHz,
                                             Div,
                                             Ldr,
                                             LdrFrac,
                                             0,
                                             GclkDiv>();
        return {Div, Ldr, LdrFrac, 0, GclkDiv, c.out, c.ref, c.dco, c.ok()};
    }
}}   // namespace Kvasir::DPLL
