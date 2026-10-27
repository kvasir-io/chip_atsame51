#pragma once
// The SAM D5x/E5x's names the shared drivers of atsam_common use, in one place: atsam_common/StartUp.hpp asserts
// Kvasir::Sam::ChipPackage (atsam_common/ChipTraits.hpp lists what each member must give) on it.

#include "CAN_Traits.hpp"
#include "Dmac_Traits.hpp"
#include "Interrupt.hpp"
#include "NVMCTRL_Traits.hpp"
#include "Sercom_Traits.hpp"
#include "Serial_Number_Traits.hpp"
#include "Start.hpp"
#include "chip/atsam_common/SercomPads.hpp"
#include "peripherals/SERCOM_SPI.hpp"
#include "peripherals/SERCOM_USART.hpp"

#include <cstdint>

namespace Kvasir { namespace Sam {
    struct Package {
        using Sercom = Kvasir::Sercom::Traits::SercomTraits;
        using Dmac   = Kvasir::DMAC::Traits::DmacTraits;
        using Nvm    = Kvasir::NVMCTRL::Traits;
        using Start  = Kvasir::Startup::Sam::StartTraits;
        template<unsigned Line>
        using EicVector = Kvasir::Interrupt::EicExtIntVector<Line>;
        template<unsigned I>
        using SercomSpi = Kvasir::Peripheral::SERCOM_SPI::Registers<I>;
        template<unsigned I>
        using SercomUsart = Kvasir::Peripheral::SERCOM_USART::Registers<I>;
        using SercomPads  = Kvasir::Sam::Pads::SamE5x;
        static constexpr auto const& serialNumberAddresses
          = Kvasir::SerialNumber::Traits::addresses;
        // The chip differences the shared drivers read (atsam_common/ChipTraits.hpp): a SYNCBUSY register for the EIC
        // (DS60001507N 23.8.4, md l.19614) and the WDT (20.8.7, md l.11576); the WDT counts the fixed 1.024 kHz
        // CLK_WDT_OSC (20.5.3); no CTRL.CRCENABLE - a source in CRCCTRL.CRCSRC turns the CRC on (22.6.2.2 md l.15899)
        static constexpr bool          eicSyncBusyInStatus = false;
        static constexpr bool          wdtSyncBusyInStatus = false;
        static constexpr std::uint32_t wdtFixedClockHz     = 1'024;
        static constexpr bool          dmacHasCrcEnable    = false;
        static constexpr bool          hasCan              = true;
        using Can                                          = Kvasir::CAN::Traits::CanTraits;
    };
}}   // namespace Kvasir::Sam
