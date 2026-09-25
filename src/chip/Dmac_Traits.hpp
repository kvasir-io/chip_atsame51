#pragma once
#include "peripherals/DMAC.hpp"

#include <cstdint>

namespace Kvasir { namespace DMAC { namespace Traits {
    struct DmacTraits {
        /// 32 channels (SAM D5x/E5x data sheet Table 1-2), and the later DMAC: each channel has
        /// its own CHCTRLA register block rather than one window selected through CHID.
        static constexpr std::size_t Channels = 32;
        static constexpr bool        OldImpl  = false;

        using TriggerSource
          = Kvasir::Peripheral::DMAC::Registers<>::CHANNEL<0>::CHCTRLA::TRIGSRCVal;
    };
}}}   // namespace Kvasir::DMAC::Traits
