#pragma once
#include "kvasir/Common/Interrupt.hpp"

#include <array>

namespace Kvasir {
namespace Interrupt {
    template<int I>
    using Type = ::Kvasir::Nvic::Index<I>;

    static constexpr Type<-14> nonMaskableInt{};
    static constexpr Type<-13> hardFault{};
    static constexpr Type<-12> memoryManagement{};
    static constexpr Type<-11> busFault{};
    static constexpr Type<-10> usageFault{};
    static constexpr Type<-5>  sVCall{};
    static constexpr Type<-2>  pendSV{};
    static constexpr Type<-1>  systick{};

    // The peripheral lines, named as chip.svd names them and checked against the SAM D5x/E5x
    // data sheet (DS60001507N), Table 10-1 "Interrupt Line Mapping". A peripheral with several
    // lines gives its first flags a line each and ORs the rest onto the last one ("_other"):
    // a SERCOM's flags 0, 1 and 2 are lines n_0 to n_2, and 3, 4, 5 and 7 (ERROR) share n_other.
    static constexpr Type<0>   pm{};
    static constexpr Type<1>   mclk{};
    static constexpr Type<2>   oscctrl_xosc0{};
    static constexpr Type<3>   oscctrl_xosc1{};
    static constexpr Type<4>   oscctrl_dfll{};
    static constexpr Type<5>   oscctrl_dpll0{};
    static constexpr Type<6>   oscctrl_dpll1{};
    static constexpr Type<7>   osc32kctrl{};
    static constexpr Type<8>   supc_other{};
    static constexpr Type<9>   supc_boddet{};
    static constexpr Type<10>  wdt{};
    static constexpr Type<11>  rtc{};
    static constexpr Type<12>  eic_extint_0{};
    static constexpr Type<13>  eic_extint_1{};
    static constexpr Type<14>  eic_extint_2{};
    static constexpr Type<15>  eic_extint_3{};
    static constexpr Type<16>  eic_extint_4{};
    static constexpr Type<17>  eic_extint_5{};
    static constexpr Type<18>  eic_extint_6{};
    static constexpr Type<19>  eic_extint_7{};
    static constexpr Type<20>  eic_extint_8{};
    static constexpr Type<21>  eic_extint_9{};
    static constexpr Type<22>  eic_extint_10{};
    static constexpr Type<23>  eic_extint_11{};
    static constexpr Type<24>  eic_extint_12{};
    static constexpr Type<25>  eic_extint_13{};
    static constexpr Type<26>  eic_extint_14{};
    static constexpr Type<27>  eic_extint_15{};
    static constexpr Type<28>  freqm{};
    static constexpr Type<29>  nvmctrl_0{};
    static constexpr Type<30>  nvmctrl_1{};
    static constexpr Type<31>  dmac_0{};
    static constexpr Type<32>  dmac_1{};
    static constexpr Type<33>  dmac_2{};
    static constexpr Type<34>  dmac_3{};
    static constexpr Type<35>  dmac_other{};
    static constexpr Type<36>  evsys_0{};
    static constexpr Type<37>  evsys_1{};
    static constexpr Type<38>  evsys_2{};
    static constexpr Type<39>  evsys_3{};
    static constexpr Type<40>  evsys_other{};
    static constexpr Type<41>  pac{};
    static constexpr Type<45>  ramecc{};
    static constexpr Type<46>  sercom0_0{};
    static constexpr Type<47>  sercom0_1{};
    static constexpr Type<48>  sercom0_2{};
    static constexpr Type<49>  sercom0_other{};
    static constexpr Type<50>  sercom1_0{};
    static constexpr Type<51>  sercom1_1{};
    static constexpr Type<52>  sercom1_2{};
    static constexpr Type<53>  sercom1_other{};
    static constexpr Type<54>  sercom2_0{};
    static constexpr Type<55>  sercom2_1{};
    static constexpr Type<56>  sercom2_2{};
    static constexpr Type<57>  sercom2_other{};
    static constexpr Type<58>  sercom3_0{};
    static constexpr Type<59>  sercom3_1{};
    static constexpr Type<60>  sercom3_2{};
    static constexpr Type<61>  sercom3_other{};
    static constexpr Type<62>  sercom4_0{};
    static constexpr Type<63>  sercom4_1{};
    static constexpr Type<64>  sercom4_2{};
    static constexpr Type<65>  sercom4_other{};
    static constexpr Type<66>  sercom5_0{};
    static constexpr Type<67>  sercom5_1{};
    static constexpr Type<68>  sercom5_2{};
    static constexpr Type<69>  sercom5_other{};
    static constexpr Type<78>  can0{};
    static constexpr Type<79>  can1{};
    static constexpr Type<80>  usb_other{};
    static constexpr Type<81>  usb_sof_hsof{};
    static constexpr Type<82>  usb_trcpt0{};
    static constexpr Type<83>  usb_trcpt1{};
    static constexpr Type<85>  tcc0_other{};
    static constexpr Type<86>  tcc0_mc0{};
    static constexpr Type<87>  tcc0_mc1{};
    static constexpr Type<88>  tcc0_mc2{};
    static constexpr Type<89>  tcc0_mc3{};
    static constexpr Type<90>  tcc0_mc4{};
    static constexpr Type<91>  tcc0_mc5{};
    static constexpr Type<92>  tcc1_other{};
    static constexpr Type<93>  tcc1_mc0{};
    static constexpr Type<94>  tcc1_mc1{};
    static constexpr Type<95>  tcc1_mc2{};
    static constexpr Type<96>  tcc1_mc3{};
    static constexpr Type<97>  tcc2_other{};
    static constexpr Type<98>  tcc2_mc0{};
    static constexpr Type<99>  tcc2_mc1{};
    static constexpr Type<100> tcc2_mc2{};
    static constexpr Type<101> tcc3_other{};
    static constexpr Type<102> tcc3_mc0{};
    static constexpr Type<103> tcc3_mc1{};
    static constexpr Type<104> tcc4_other{};
    static constexpr Type<105> tcc4_mc0{};
    static constexpr Type<106> tcc4_mc1{};
    static constexpr Type<107> tc0{};
    static constexpr Type<108> tc1{};
    static constexpr Type<109> tc2{};
    static constexpr Type<110> tc3{};
    static constexpr Type<111> tc4{};
    static constexpr Type<112> tc5{};
    static constexpr Type<115> pdec_other{};
    static constexpr Type<116> pdec_mc0{};
    static constexpr Type<117> pdec_mc1{};
    static constexpr Type<118> adc0_other{};
    static constexpr Type<119> adc0_resrdy{};
    static constexpr Type<120> adc1_other{};
    static constexpr Type<121> adc1_resrdy{};
    static constexpr Type<122> ac{};
    static constexpr Type<123> dac_other{};
    static constexpr Type<124> dac_empty_0{};
    static constexpr Type<125> dac_empty_1{};
    static constexpr Type<126> dac_resrdy_0{};
    static constexpr Type<127> dac_resrdy_1{};
    static constexpr Type<128> i2s{};
    static constexpr Type<129> pcc{};
    static constexpr Type<130> aes{};
    static constexpr Type<131> trng{};
    static constexpr Type<132> icm{};
    static constexpr Type<134> qspi{};
    static constexpr Type<135> sdhc0{};
}   // namespace Interrupt

namespace Nvic {
    using namespace Kvasir::Interrupt;

    template<>
    struct InterruptOffsetTraits<void> {
        static constexpr int begin = -14;
        /// Half open: the last vector is SDHC0 at 135.
        static constexpr int end = 136;
        /// The Cortex-M4 vectors that are reserved (-9 to -6, -3) or unused here (-4, the debug
        /// monitor), and the gaps in this part's own table: 42 to 44 and 133 are reserved on the
        /// whole family (Table 10-1), and 70 to 77 (SERCOM6/7), 84 (GMAC), 113 and 114 (TC6/7)
        /// belong to peripherals the 64-pin E51 does not have (Table 1-2).
        static constexpr std::array disabled
          = {-9, -8, -7, -6, -4, -3, 42, 43, 44, 70, 71, 72, 73, 74, 75, 76, 77, 84, 113, 114, 133};
        static constexpr std::array noEnable
          = {nonMaskableInt.index(), sVCall.index(), pendSV.index()};
        static constexpr std::array noDisable
          = {nonMaskableInt.index(), sVCall.index(), pendSV.index()};
        static constexpr std::array noSetPending = {sVCall.index(), hardFault.index()};
        static constexpr std::array noClearPending
          = {nonMaskableInt.index(), sVCall.index(), hardFault.index()};
        static constexpr std::array noSetPriority = {nonMaskableInt.index(), hardFault.index()};

        /// The three configurable faults are enabled in SHCSR by Fault::Core::EarlyInitList
        /// rather than through the NVIC, so none of them needs an enable here.
        using FaultInterruptIndexs           = brigand::list<decltype(hardFault),
                                                             decltype(memoryManagement),
                                                             decltype(busFault),
                                                             decltype(usageFault)>;
        using FaultInterruptIndexsNeedEnable = brigand::list<>;
    };

}   // namespace Nvic
}   // namespace Kvasir
