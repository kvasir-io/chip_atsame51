#pragma once
#include "chip/atsam_common/Sercom_Common.hpp"
#include "kvasir/Io/Types.hpp"

#include <array>

namespace Kvasir { namespace Sercom { namespace Traits {

    struct SercomTraits : SercomTraitsBase<SercomTraits> {
        static constexpr std::array pinMuxInfos{
          // ATSAME51J18A, 64 pins: PORTA and PORTB, six SERCOMs. {instance, port, pin, mux, pad},
          // mux 2 is column C and 3 is column D of the SAM D5x/E5x data sheet (DS60001507N)
          // Table 6-1 "Multiplexed Peripheral Signals"; only pads the 64-pin package bonds.
          // What this table cannot express is the IOSET rule (6.2.8.1 "SERCOM IOSET
          // Configurations"): all pads of one SERCOM have to come from the same IOSET, so a new
          // board's pin choice wants checking there as well.
          // Instance 0
          PinInfo{0, 0,  4, 3, 0},
          PinInfo{0, 0,  5, 3, 1},
          PinInfo{0, 0,  6, 3, 2},
          PinInfo{0, 0,  7, 3, 3},
          PinInfo{0, 0,  8, 2, 0},
          PinInfo{0, 0,  9, 2, 1},
          PinInfo{0, 0, 10, 2, 2},
          PinInfo{0, 0, 11, 2, 3},
          // Instance 1
          PinInfo{1, 0,  0, 3, 0},
          PinInfo{1, 0,  1, 3, 1},
          PinInfo{1, 0, 16, 2, 0},
          PinInfo{1, 0, 17, 2, 1},
          PinInfo{1, 0, 18, 2, 2},
          PinInfo{1, 0, 19, 2, 3},
          PinInfo{1, 0, 30, 3, 2},
          PinInfo{1, 0, 31, 3, 3},
          PinInfo{1, 1, 22, 2, 2},
          PinInfo{1, 1, 23, 2, 3},
          // Instance 2
          PinInfo{2, 0,  8, 3, 1},
          PinInfo{2, 0,  9, 3, 0},
          PinInfo{2, 0, 10, 3, 2},
          PinInfo{2, 0, 11, 3, 3},
          PinInfo{2, 0, 12, 2, 0},
          PinInfo{2, 0, 13, 2, 1},
          PinInfo{2, 0, 14, 2, 2},
          PinInfo{2, 0, 15, 2, 3},
          // Instance 3
          PinInfo{3, 0, 16, 3, 1},
          PinInfo{3, 0, 17, 3, 0},
          PinInfo{3, 0, 18, 3, 2},
          PinInfo{3, 0, 19, 3, 3},
          PinInfo{3, 0, 20, 3, 2},
          PinInfo{3, 0, 21, 3, 3},
          PinInfo{3, 0, 22, 2, 0},
          PinInfo{3, 0, 23, 2, 1},
          PinInfo{3, 0, 24, 2, 2},
          PinInfo{3, 0, 25, 2, 3},
          // Instance 4
          PinInfo{4, 0, 12, 3, 1},
          PinInfo{4, 0, 13, 3, 0},
          PinInfo{4, 0, 14, 3, 2},
          PinInfo{4, 0, 15, 3, 3},
          PinInfo{4, 1,  8, 3, 0},
          PinInfo{4, 1,  9, 3, 1},
          PinInfo{4, 1, 10, 3, 2},
          PinInfo{4, 1, 11, 3, 3},
          PinInfo{4, 1, 12, 2, 0},
          PinInfo{4, 1, 13, 2, 1},
          PinInfo{4, 1, 14, 2, 2},
          PinInfo{4, 1, 15, 2, 3},
          // Instance 5
          PinInfo{5, 0, 20, 2, 2},
          PinInfo{5, 0, 21, 2, 3},
          PinInfo{5, 0, 22, 3, 1},
          PinInfo{5, 0, 23, 3, 0},
          PinInfo{5, 0, 24, 3, 2},
          PinInfo{5, 0, 25, 3, 3},
          PinInfo{5, 1,  0, 3, 2},
          PinInfo{5, 1,  1, 3, 3},
          PinInfo{5, 1,  2, 3, 0},
          PinInfo{5, 1,  3, 3, 1},
          PinInfo{5, 1, 16, 2, 0},
          PinInfo{5, 1, 17, 2, 1},
          PinInfo{5, 1, 22, 3, 2},
          PinInfo{5, 1, 23, 3, 3},
          PinInfo{5, 1, 30, 3, 1},
          PinInfo{5, 1, 31, 3, 0},
        };
    };

}}}   // namespace Kvasir::Sercom::Traits
