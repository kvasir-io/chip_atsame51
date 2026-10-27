#pragma once
#include "peripherals/DMAC.hpp"

#include <cstddef>

namespace Kvasir { namespace DMAC { namespace Traits {
    struct DmacTraits {
        /// 32 channels (SAM D5x/E5x data sheet Table 1-2), and the later DMAC: each channel has
        /// its own CHCTRLA register block rather than one window selected through CHID
        /// (atsam_common/DmacChannelWindow.hpp is the D2x/C2x one).
        static constexpr std::size_t channels              = 32;
        static constexpr bool        channelSelectedByChid = false;
        // the D2x/C2x's CRCDATAIN erratum (DmacChannelWindow.hpp) is not in the D5x/E5x errata DS80000748T
        static constexpr bool crcDataInWritesNeedAGap = false;

        using TriggerSource
          = Kvasir::Peripheral::DMAC::Registers<>::CHANNEL<0>::CHCTRLA::TRIGSRCVal;
    };
}}}   // namespace Kvasir::DMAC::Traits
