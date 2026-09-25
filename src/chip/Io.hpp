#pragma once

#include "kvasir/Io/Io.hpp"
#include "kvasir/Mpl/Utility.hpp"
#include "kvasir/Register/Register.hpp"
#include "peripherals/PORT.hpp"

#include <array>

namespace Kvasir { namespace Io {
    template<typename>
    struct PinLocationTraits {
        static constexpr unsigned baseAddress = Kvasir::Peripheral::PORT::Registers<>::baseAddr;
        static constexpr int      portBegin   = 0;
        /// The J variant (64 pins) bonds PORTA and PORTB.
        static constexpr int portEnd          = 2;
        static constexpr int pinBegin         = 0;
        static constexpr int pinEnd           = 32;
        static constexpr int ListEndIndicator = 255;
        /// The pads the 64-pin package does not bond: the ones with no entry in the
        /// "TQFP/VQFN/WLCSP 64" column of the SAM D5x/E5x data sheet (DS60001507N) Table 6-1.
        static constexpr std::array<std::array<int, pinEnd - pinBegin>, portEnd - portBegin>
          PinsDisabled{
            {{{26, 28, 29, ListEndIndicator}},
             {{18, 19, 20, 21, 24, 25, 26, 27, 28, 29, ListEndIndicator}}}
        };
    };

}}   // namespace Kvasir::Io

#include "atsam_common/Io.hpp"
