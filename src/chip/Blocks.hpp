#pragma once

// The SAM E51's clock gates (chip_atsam_common ClockGate.hpp, kvasir/Hal/Block.hpp): every block PM::enable names,
// with the MCLK mask field that clocks it (DS60001507 15.8.6 AHBMASK, 15.8.7 APBAMASK, 15.8.8 APBBMASK, 15.8.9
// APBCMASK, 15.8.10 APBDMASK). Rows marked open at boot have their bit set out of reset - AHBMASK resets to
// 0x00FFFFFF, APBAMASK to 0x000007FF, APBBMASK to 0x00018056, APBCMASK to 0x00002000, APBDMASK to 0 - and claim
// nothing; a Startup-phase write into any other block needs a listed PM::enable of it (default states: Table 15-1,
// md l.6969-7066). "A module may be connected to several clock domains (for example, AHB and APB), in which case it
// will have several mask bits" (15.6.2.6, md l.7083): such a block has a row per bit - PAC (CLK_PAC_AHB/APB, Table
// 15-1), DSU (12.5.3 md l.4146), NVMCTRL ("two synchronous clocks", 25.5.2 md l.26005), ICM (26.5.2 md l.28218), QSPI
// (37.5.3 md l.44814-44824; CLK_QSPI2X_AHB "is not used" in SDR mode, the reset mode, 37.6.3 md l.44910: no row) and
// USB (CLK_USB_AHB 38.5.3 md l.46526, CLK_USB_APB Table 15-1). HMATRIX: only the SVD's APBBMASK bit 6 (set in the
// header's reset value 0x00018056, absent from 15.8.8's bit table, AHBMASK bit 5 "Reserved" in 15.8.6) - unchecked.
// NVMCTRL_CACHE/NVMCTRL_SMEEPROM (AHBMASK bits 23/22) are not among the two clocks 25.5.2 names: no row.
// The AHB-only ones (CMCC, DMAC, SDHC0, CAN0, CAN1) by their AHB bit, which is on.
//
// The SERCOM and CAN rows also name what the shared drivers read through Sam::Block<Regs>: the interrupt lines (Table
// 10-1 "Interrupt Line Mapping", md l.2856-2911 for SERCOM0-5, l.2926/2928 for CAN0/1) and a SERCOM's DMAC trigger
// sources (CHCTRLA.TRIGSRC, md l.18152-18167: SERCOMn RX = 4 + 2n, TX = 5 + 2n), as the generated enumerators. A
// SERCOM has four NVIC lines: interrupt flags 0, 1 and 2 have a line each, and 3, 4, 5 and 7 share the fourth. All
// four are listed, because which flag means what depends on the mode - the I2C master's ERROR is flag 7, so it
// arrives on the shared line, while its MB and SB are lines 0 and 1.

#include "Dmac_Traits.hpp"
#include "Interrupt.hpp"
#include "atsam_common/ClockGate.hpp"
#include "peripherals/AC.hpp"
#include "peripherals/ADC.hpp"
#include "peripherals/AES.hpp"
#include "peripherals/CAN.hpp"
#include "peripherals/CCL.hpp"
#include "peripherals/CMCC.hpp"
#include "peripherals/DAC.hpp"
#include "peripherals/DMAC.hpp"
#include "peripherals/DSU.hpp"
#include "peripherals/EIC.hpp"
#include "peripherals/EVSYS.hpp"
#include "peripherals/FREQM.hpp"
#include "peripherals/GCLK.hpp"
#include "peripherals/HMATRIX.hpp"
#include "peripherals/I2S.hpp"
#include "peripherals/ICM.hpp"
#include "peripherals/MCLK.hpp"
#include "peripherals/NVMCTRL.hpp"
#include "peripherals/OSC32KCTRL.hpp"
#include "peripherals/OSCCTRL.hpp"
#include "peripherals/PAC.hpp"
#include "peripherals/PCC.hpp"
#include "peripherals/PDEC.hpp"
#include "peripherals/PM.hpp"
#include "peripherals/PORT.hpp"
#include "peripherals/QSPI.hpp"
#include "peripherals/RAMECC.hpp"
#include "peripherals/RSTC.hpp"
#include "peripherals/RTC_MODE0.hpp"
#include "peripherals/SDHC0.hpp"
#include "peripherals/SERCOM_I2CM.hpp"
#include "peripherals/SUPC.hpp"
#include "peripherals/TCC.hpp"
#include "peripherals/TC_COUNT16.hpp"
#include "peripherals/TRNG.hpp"
#include "peripherals/USB_DEVICE.hpp"
#include "peripherals/WDT.hpp"

namespace Kvasir { namespace Chip { namespace Blocks {
    namespace P = Peripheral;
    using M     = P::MCLK::Registers<>;
    using I     = ::Kvasir::Interrupt;
    using T     = ::Kvasir::DMAC::Traits::DmacTraits::TriggerSource;
    template<typename... Lines>
    using Irqs = brigand::list<Lines...>;
    template<typename Regs, auto const& Field, bool OpenAtBoot = false, typename Lines = Irqs<>>
    using Row = ::Kvasir::Sam::GateRow<M::baseAddr, Regs::baseAddr, Field, OpenAtBoot, Lines>;
    // a SERCOM: its gate, its four interrupt lines and its DMAC trigger sources
    template<unsigned N, auto const& Field, typename Lines, T Rx, T Tx>
    using SercomRow = ::Kvasir::Sam::
      GateRow<M::baseAddr, P::SERCOM_I2CM::Registers<N>::baseAddr, Field, false, Lines, Rx, Tx>;

    // clang-format off
    using Table = ::Kvasir::Hal::BlockTable<
        Row<P::CMCC::Registers<>, M::AHBMASK::cmcc_, true>,
        Row<P::DMAC::Registers<>, M::AHBMASK::dmac_, true>,
        Row<P::SDHC0::Registers<>, M::AHBMASK::sdhc0_, true>,
        Row<P::CAN::Registers<0>, M::AHBMASK::can0_, true, Irqs<decltype(I::can0)>>,
        Row<P::CAN::Registers<1>, M::AHBMASK::can1_, true, Irqs<decltype(I::can1)>>,
        Row<P::PAC::Registers<>, M::AHBMASK::pac_, true>,
        Row<P::PAC::Registers<>, M::APBAMASK::pac_, true>,
        Row<P::PM::Registers<>, M::APBAMASK::pm_, true>,
        Row<P::MCLK::Registers<>, M::APBAMASK::mclk_, true>,
        Row<P::RSTC::Registers<>, M::APBAMASK::rstc_, true>,
        Row<P::OSCCTRL::Registers<>, M::APBAMASK::oscctrl_, true>,
        Row<P::OSC32KCTRL::Registers<>, M::APBAMASK::osc32kctrl_, true>,
        Row<P::SUPC::Registers<>, M::APBAMASK::supc_, true>,
        Row<P::GCLK::Registers<>, M::APBAMASK::gclk_, true>,
        Row<P::WDT::Registers<>, M::APBAMASK::wdt_, true>,
        Row<P::RTC_MODE0::Registers<>, M::APBAMASK::rtc_, true>,
        Row<P::EIC::Registers<>, M::APBAMASK::eic_, true>,
        Row<P::FREQM::Registers<>, M::APBAMASK::freqm_>,
        SercomRow<0,
                  M::APBAMASK::sercom0_,
                  Irqs<decltype(I::sercom0_0), decltype(I::sercom0_1),
                       decltype(I::sercom0_2), decltype(I::sercom0_other)>,
                  T::sercom0_rx,
                  T::sercom0_tx>,
        SercomRow<1,
                  M::APBAMASK::sercom1_,
                  Irqs<decltype(I::sercom1_0), decltype(I::sercom1_1),
                       decltype(I::sercom1_2), decltype(I::sercom1_other)>,
                  T::sercom1_rx,
                  T::sercom1_tx>,
        Row<P::TC_COUNT16::Registers<0>, M::APBAMASK::tc0_>,
        Row<P::TC_COUNT16::Registers<1>, M::APBAMASK::tc1_>,
        Row<P::USB_DEVICE::Registers<>, M::AHBMASK::usb_, true>,
        Row<P::USB_DEVICE::Registers<>, M::APBBMASK::usb_>,
        Row<P::DSU::Registers<>, M::AHBMASK::dsu_, true>,
        Row<P::DSU::Registers<>, M::APBBMASK::dsu_, true>,
        Row<P::NVMCTRL::Registers<>, M::AHBMASK::nvmctrl_, true>,
        Row<P::NVMCTRL::Registers<>, M::APBBMASK::nvmctrl_, true>,
        Row<P::PORT::Registers<>, M::APBBMASK::port_, true>,
        Row<P::HMATRIX::Registers<>, M::APBBMASK::hmatrix_, true>,
        Row<P::EVSYS::Registers<>, M::APBBMASK::evsys_>,
        SercomRow<2,
                  M::APBBMASK::sercom2_,
                  Irqs<decltype(I::sercom2_0), decltype(I::sercom2_1),
                       decltype(I::sercom2_2), decltype(I::sercom2_other)>,
                  T::sercom2_rx,
                  T::sercom2_tx>,
        SercomRow<3,
                  M::APBBMASK::sercom3_,
                  Irqs<decltype(I::sercom3_0), decltype(I::sercom3_1),
                       decltype(I::sercom3_2), decltype(I::sercom3_other)>,
                  T::sercom3_rx,
                  T::sercom3_tx>,
        Row<P::TCC::Registers<0>, M::APBBMASK::tcc0_>,
        Row<P::TCC::Registers<1>, M::APBBMASK::tcc1_>,
        Row<P::TC_COUNT16::Registers<2>, M::APBBMASK::tc2_>,
        Row<P::TC_COUNT16::Registers<3>, M::APBBMASK::tc3_>,
        Row<P::RAMECC::Registers<>, M::APBBMASK::ramecc_, true>,
        Row<P::TCC::Registers<2>, M::APBCMASK::tcc2_>,
        Row<P::TCC::Registers<3>, M::APBCMASK::tcc3_>,
        Row<P::TC_COUNT16::Registers<4>, M::APBCMASK::tc4_>,
        Row<P::TC_COUNT16::Registers<5>, M::APBCMASK::tc5_>,
        Row<P::PDEC::Registers<>, M::APBCMASK::pdec_>,
        Row<P::AC::Registers<>, M::APBCMASK::ac_>,
        Row<P::AES::Registers<>, M::APBCMASK::aes_>,
        Row<P::TRNG::Registers<>, M::APBCMASK::trng_>,
        Row<P::ICM::Registers<>, M::AHBMASK::icm_, true>,
        Row<P::ICM::Registers<>, M::APBCMASK::icm_>,
        Row<P::QSPI::Registers<>, M::AHBMASK::qspi_, true>,
        Row<P::QSPI::Registers<>, M::APBCMASK::qspi_, true>,
        Row<P::CCL::Registers<>, M::APBCMASK::ccl_>,
        SercomRow<4,
                  M::APBDMASK::sercom4_,
                  Irqs<decltype(I::sercom4_0), decltype(I::sercom4_1),
                       decltype(I::sercom4_2), decltype(I::sercom4_other)>,
                  T::sercom4_rx,
                  T::sercom4_tx>,
        SercomRow<5,
                  M::APBDMASK::sercom5_,
                  Irqs<decltype(I::sercom5_0), decltype(I::sercom5_1),
                       decltype(I::sercom5_2), decltype(I::sercom5_other)>,
                  T::sercom5_rx,
                  T::sercom5_tx>,
        Row<P::TCC::Registers<4>, M::APBDMASK::tcc4_>,
        Row<P::ADC::Registers<0>, M::APBDMASK::adc0_>,
        Row<P::ADC::Registers<1>, M::APBDMASK::adc1_>,
        Row<P::DAC::Registers<>, M::APBDMASK::dac_>,
        Row<P::I2S::Registers<>, M::APBDMASK::i2s_>,
        Row<P::PCC::Registers<>, M::APBDMASK::pcc_>
        >;
    // clang-format on
}}}   // namespace Kvasir::Chip::Blocks

template<>
struct Kvasir::Hal::BlockGates<Kvasir::Tag::User> {
    using type = Kvasir::Chip::Blocks::Table;
};
