#pragma once
#include "atsam_common/Io.hpp"
#include "chip/atsam_common/CAN_Common.hpp"
#include "kvasir/Io/Types.hpp"

#include <array>

namespace Kvasir { namespace CAN { namespace Traits {

    struct CanTraits : CanTraitsBase<CanTraits> {
        // SAM D5x/E5x data sheet (DS60001507N) Table 6-1: CAN0 is peripheral function I (mux 8),
        // CAN1 is H (mux 7).
        static constexpr std::array pinMuxInfos{
          // Instance 0
          PinInfo{0, 0, 22, 8, PinInfo::tx},
          PinInfo{0, 0, 23, 8, PinInfo::rx},
          PinInfo{0, 0, 24, 8, PinInfo::tx},
          PinInfo{0, 0, 25, 8, PinInfo::rx},
          // Instance 1
          PinInfo{1, 1, 12, 7, PinInfo::tx},
          PinInfo{1, 1, 13, 7, PinInfo::rx},
          PinInfo{1, 1, 14, 7, PinInfo::tx},
          PinInfo{1, 1, 15, 7, PinInfo::rx}
        };
    };

}}}   // namespace Kvasir::CAN::Traits
