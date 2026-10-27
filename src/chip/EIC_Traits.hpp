#pragma once
// The EXTINT line of a pad on the SAM D5x/E5x (atsam_common/EicLines.hpp has the rule and its table citation); EIC.hpp
// includes this header itself.
#include "atsam_common/EicLines.hpp"

namespace Kvasir { namespace EIC { namespace Traits {
    using Lines::none;
    using Lines::notEncoded;

    consteval unsigned extIntOf(int port,
                                int pin) {
        return Lines::samE5x(port, pin);
    }
}}}   // namespace Kvasir::EIC::Traits
