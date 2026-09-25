#pragma once
#include "Interrupt.hpp"
#include "chip/atsam_common/DMAC.hpp"
#include "chip/atsam_common/Sercom_Common.hpp"
#include "kvasir/Io/Types.hpp"

#include <array>

namespace Kvasir { namespace Sercom { namespace Traits {

    namespace detail {

        // Four NVIC lines per SERCOM (data sheet Table 10-1): interrupt flags 0, 1 and 2 have a
        // line each, and 3, 4, 5 and 7 share the fourth. All four are listed, because which flag
        // means what depends on the mode - the I2C master's ERROR is flag 7, so it arrives on
        // the shared line, while its MB and SB are lines 0 and 1.
        template<unsigned Instance>
        struct IsrIndex;

        template<>
        struct IsrIndex<0> {
            using Type = brigand::list<decltype(Kvasir::Interrupt::sercom0_0),
                                       decltype(Kvasir::Interrupt::sercom0_1),
                                       decltype(Kvasir::Interrupt::sercom0_2),
                                       decltype(Kvasir::Interrupt::sercom0_other)>;
        };

        template<>
        struct IsrIndex<1> {
            using Type = brigand::list<decltype(Kvasir::Interrupt::sercom1_0),
                                       decltype(Kvasir::Interrupt::sercom1_1),
                                       decltype(Kvasir::Interrupt::sercom1_2),
                                       decltype(Kvasir::Interrupt::sercom1_other)>;
        };

        template<>
        struct IsrIndex<2> {
            using Type = brigand::list<decltype(Kvasir::Interrupt::sercom2_0),
                                       decltype(Kvasir::Interrupt::sercom2_1),
                                       decltype(Kvasir::Interrupt::sercom2_2),
                                       decltype(Kvasir::Interrupt::sercom2_other)>;
        };

        template<>
        struct IsrIndex<3> {
            using Type = brigand::list<decltype(Kvasir::Interrupt::sercom3_0),
                                       decltype(Kvasir::Interrupt::sercom3_1),
                                       decltype(Kvasir::Interrupt::sercom3_2),
                                       decltype(Kvasir::Interrupt::sercom3_other)>;
        };

        template<>
        struct IsrIndex<4> {
            using Type = brigand::list<decltype(Kvasir::Interrupt::sercom4_0),
                                       decltype(Kvasir::Interrupt::sercom4_1),
                                       decltype(Kvasir::Interrupt::sercom4_2),
                                       decltype(Kvasir::Interrupt::sercom4_other)>;
        };

        template<>
        struct IsrIndex<5> {
            using Type = brigand::list<decltype(Kvasir::Interrupt::sercom5_0),
                                       decltype(Kvasir::Interrupt::sercom5_1),
                                       decltype(Kvasir::Interrupt::sercom5_2),
                                       decltype(Kvasir::Interrupt::sercom5_other)>;
        };

    }   // namespace detail

    struct SercomTraits : SercomTraitsBase<SercomTraits> {
        static constexpr int                sercomInstanceBegin = 0;
        static constexpr int                sercomInstanceEnd   = 6;
        static constexpr std::array<int, 0> disabledInstances   = {{}};

        template<unsigned Instance>
        struct IsrIndex {
            using Type = typename detail::IsrIndex<Instance>::Type;
        };

        template<unsigned Instance>
        static constexpr auto DmaTriggers() {
            static_assert(5 >= Instance);
            if constexpr(Instance == 0) {
                return std::make_pair(Kvasir::DMAC::TriggerSource::sercom0_rx,
                                      DMAC::TriggerSource::sercom0_tx);
            } else if constexpr(Instance == 1) {
                return std::make_pair(Kvasir::DMAC::TriggerSource::sercom1_rx,
                                      DMAC::TriggerSource::sercom1_tx);
            } else if constexpr(Instance == 2) {
                return std::make_pair(Kvasir::DMAC::TriggerSource::sercom2_rx,
                                      DMAC::TriggerSource::sercom2_tx);
            } else if constexpr(Instance == 3) {
                return std::make_pair(Kvasir::DMAC::TriggerSource::sercom3_rx,
                                      DMAC::TriggerSource::sercom3_tx);
            } else if constexpr(Instance == 4) {
                return std::make_pair(Kvasir::DMAC::TriggerSource::sercom4_rx,
                                      DMAC::TriggerSource::sercom4_tx);
            } else if constexpr(Instance == 5) {
                return std::make_pair(Kvasir::DMAC::TriggerSource::sercom5_rx,
                                      DMAC::TriggerSource::sercom5_tx);
            }
        }

        template<unsigned Instance>
        static constexpr auto DmaRX_Trigger() {
            return DmaTriggers<Instance>().first;
        }

        template<unsigned Instance>
        static constexpr auto DmaTX_Trigger() {
            return DmaTriggers<Instance>().second;
        }

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
