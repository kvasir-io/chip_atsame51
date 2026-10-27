#pragma once
// The chip's pin table (PinLocationTraits). Users include the family's pin front, chip/atsam_common/Io.hpp, which
// includes this.

#include "kvasir/Hal/Pins.hpp"
#include "kvasir/Io/Io.hpp"
#include "kvasir/Register/Register.hpp"
#include "peripherals/PORT.hpp"

#include <array>
#include <type_traits>

namespace Kvasir { namespace Io {
    template<typename>
    struct PinLocationTraits {
        static constexpr unsigned baseAddress = Kvasir::Peripheral::PORT::Registers<>::baseAddr;
        static constexpr int      portBegin   = 0;
        /// The J variant (64 pins) bonds PORTA and PORTB.
        static constexpr int portEnd  = 2;
        static constexpr int pinBegin = 0;
        static constexpr int pinEnd   = 32;
        /// The pads the 64-pin package bonds: the ones with an entry in the "TQFP/VQFN/WLCSP 64"
        /// column of the SAM D5x/E5x data sheet (DS60001507N) Table 6-1 - PA00-PA25, PA27, PA30,
        /// PA31; PB00-PB17, PB22, PB23, PB30, PB31.
        static constexpr std::array<std::uint64_t, portEnd - portBegin> bonded{0xCBFF'FFFFULL,
                                                                               0xC0C3'FFFFULL};
    };

}}   // namespace Kvasir::Io

namespace Kvasir { namespace Sam {
    // The PORT registers of one pin group (port), for chip_atsam_common Io.hpp: the SVD's GROUP<n> cluster (DS60001507N 32.7:
    // PINCFGy 0x40 + y and PMUXn 0x30 + n per group, group stride 0x80).
    template<int Port>
    struct PortGroup {
        using Group = Kvasir::Peripheral::PORT::Registers<>::GROUP<Port>;

        static constexpr auto dirclr = Group::DIRCLR::dirclr;
        static constexpr auto dirset = Group::DIRSET::dirset;
        static constexpr auto outclr = Group::OUTCLR::outclr;
        static constexpr auto outset = Group::OUTSET::outset;
        static constexpr auto outtgl = Group::OUTTGL::outtgl;
        static constexpr auto in     = Group::IN::in;

        template<int Pin>
        using Pincfg = typename Group::template PINCFG<Pin>;
        template<int Pin>
        using Pmux = typename Group::template PMUX<Pin / 2>;
    };
}}   // namespace Kvasir::Sam
