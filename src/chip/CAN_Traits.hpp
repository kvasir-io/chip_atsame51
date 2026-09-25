#pragma once
#include "Interrupt.hpp"
#include "Io.hpp"
#include "chip/atsam_common/CAN_Common.hpp"
#include "kvasir/Io/Types.hpp"

#include <array>

namespace Kvasir { namespace CAN { namespace Traits {

    struct CanTraits : CanTraitsBase<CanTraits> {
        static constexpr int                canInstanceBegin  = 0;
        static constexpr int                canInstanceEnd    = 2;
        static constexpr std::array<int, 0> disabledInstances = {{}};

        template<unsigned Instance>
        struct IsrIndex;

        template<>
        struct IsrIndex<0> {
            using Type = brigand::list<decltype(Kvasir::Interrupt::can0)>;
        };

        template<>
        struct IsrIndex<1> {
            using Type = brigand::list<decltype(Kvasir::Interrupt::can1)>;
        };

        // SAM D5x/E5x data sheet (DS60001507N) Table 6-1: CAN0 is peripheral function I (mux 8),
        // CAN1 is H (mux 7).
        static constexpr std::array pinMuxInfos{
          // Instance 0
          PinInfo{0, 0, 22, 8, PinInfo::TX},
          PinInfo{0, 0, 23, 8, PinInfo::RX},
          PinInfo{0, 0, 24, 8, PinInfo::TX},
          PinInfo{0, 0, 25, 8, PinInfo::RX},
          // Instance 1
          PinInfo{1, 1, 12, 7, PinInfo::TX},
          PinInfo{1, 1, 13, 7, PinInfo::RX},
          PinInfo{1, 1, 14, 7, PinInfo::TX},
          PinInfo{1, 1, 15, 7, PinInfo::RX}
        };
    };

}}}   // namespace Kvasir::CAN::Traits
