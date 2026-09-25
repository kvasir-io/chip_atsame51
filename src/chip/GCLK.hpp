#pragma once

#include "kvasir/Register/Register.hpp"
#include "kvasir/Register/Utility.hpp"
#include "peripherals/GCLK.hpp"

#include <bit>

namespace Kvasir { namespace GCLK {

    /// GENCTRLn.SRC, SAM D5x/E5x data sheet (DS60001507N) Table 14-4.
    enum class GeneratorSource : unsigned {
        xosc0     = 0x00,
        xosc1     = 0x01,
        gclkin    = 0x02,
        gclkgen1  = 0x03,
        osculp32k = 0x04,
        xosc32k   = 0x05,
        dfll      = 0x06,
        dpll0     = 0x07,
        dpll1     = 0x08
    };

    /// The PCHCTRLm index, Table 14-9 "PCHCTRLm Mapping". Channel 3 is shared: both FDPLLs' 32 kHz
    /// lock-timer clock, SDHC0's slow clock and every SERCOM's slow clock. The channels of
    /// peripherals the 64-pin E51 does not have (SERCOM6/7, TC6/7, SDHC1) are left out.
    enum class Peripheral : unsigned {
        dfll48                          = 0,
        fdpll0                          = 1,
        fdpll1                          = 2,
        fdpll_32k_sdhc_slow_sercom_slow = 3,
        eic                             = 4,
        freqm_msr                       = 5,
        freqm_ref                       = 6,
        sercom0_core                    = 7,
        sercom1_core                    = 8,
        tc0_tc1                         = 9,
        usb                             = 10,
        evsys_ch0                       = 11,
        evsys_ch1                       = 12,
        evsys_ch2                       = 13,
        evsys_ch3                       = 14,
        evsys_ch4                       = 15,
        evsys_ch5                       = 16,
        evsys_ch6                       = 17,
        evsys_ch7                       = 18,
        evsys_ch8                       = 19,
        evsys_ch9                       = 20,
        evsys_ch10                      = 21,
        evsys_ch11                      = 22,
        sercom2_core                    = 23,
        sercom3_core                    = 24,
        tcc0_tcc1                       = 25,
        tc2_tc3                         = 26,
        can0                            = 27,
        can1                            = 28,
        tcc2_tcc3                       = 29,
        tc4_tc5                         = 30,
        pdec                            = 31,
        ac                              = 32,
        ccl                             = 33,
        sercom4_core                    = 34,
        sercom5_core                    = 35,
        tcc4                            = 38,
        adc0                            = 40,
        adc1                            = 41,
        dac                             = 42,
        i2s0                            = 43,
        i2s1                            = 44,
        sdhc0                           = 45,
        cm4_trace                       = 47
    };

    template<unsigned Generator, GeneratorSource Source, unsigned long long Div>
    struct GenericClockGenerator {
        using GC = Kvasir::Peripheral::GCLK::Registers<>::GENCTRL<Generator>;

        static constexpr unsigned maxDiv = (Generator == 1 ? (1 << 16) : (1 << 8)) - 1;

        static_assert((Div <= maxDiv) || (std::popcount(Div) == 1),
                      "must be power of 2 or lower then maxDiv");

        static constexpr auto divsel = []() {
            if constexpr(Div > maxDiv) {
                return GC::DIVSELValC::div2;
            } else {
                return GC::DIVSELValC::div1;
            }
        }();
        static constexpr auto div = []() {
            if constexpr(Div > maxDiv) {
                return std::countr_zero(Div) - 1;
            } else {
                return Div;
            }
        }();

        [[nodiscard]] static constexpr auto enable() {
            return list(Register::overrideDefaults<typename GC::default_values>::value(
              set(GC::genen),
              write(divsel),
              set(GC::runstdby),
              write(GC::div, Register::value<div>()),
              write(
                GC::src,
                Register::value<typename GC::SRCVal, static_cast<typename GC::SRCVal>(Source)>())));
        }
    };

    template<unsigned Generator, Peripheral peripheral>
    struct PeripheralChannelController {
        using PC
          = Kvasir::Peripheral::GCLK::Registers<>::PCHCTRL<static_cast<unsigned>(peripheral)>;

        [[nodiscard]] static constexpr auto enable() {
            return list(Register::overrideDefaults<typename PC::default_values>::value(
              set(PC::chen),
              write(PC::gen,
                    Register::value<typename PC::GENVal,
                                    static_cast<typename PC::GENVal>(Generator)>())));
        }
    };

}}   // namespace Kvasir::GCLK
