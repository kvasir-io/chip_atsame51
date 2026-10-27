#pragma once

#include "ClockLimits.hpp"
#include "WaitBounds.hpp"
#include "atsam_common/GclkIndexed.hpp"
#include "kvasir/Register/Register.hpp"
#include "kvasir/Register/Utility.hpp"
#include "kvasir/Register/Wait.hpp"
#include "peripherals/GCLK.hpp"
#include "peripherals/MCLK.hpp"
#include "peripherals/OSCCTRL.hpp"

#include <bit>
#include <cstdint>

namespace Kvasir { namespace GCLK {

    /// GENCTRLn.SRC (DS60001507N 14.8.3, Table 14-4): the generated enum of generator 0's register - Generator casts
    /// it to its own generator's (the same values; each GENCTRL<n> of the SVD's dim has its enum type).
    using GeneratorSource = Kvasir::Peripheral::GCLK::Registers<>::GENCTRL<0>::SRCVal;

    /// The PCHCTRLm index, Table 14-9 "PCHCTRLm Mapping". Channel 3 is shared: both FDPLLs' 32 kHz
    /// lock-timer clock, SDHC0's slow clock and every SERCOM's slow clock. The channels of
    /// peripherals the 64-pin E51 does not have (SERCOM6/7, TC6/7, SDHC1) are left out.
    enum class Peripheral : unsigned {
        dfll48                     = 0,
        fdpll0                     = 1,
        fdpll1                     = 2,
        fdpll32kSdhcSlowSercomSlow = 3,
        eic                        = 4,
        freqmMsr                   = 5,
        freqmRef                   = 6,
        sercom0Core                = 7,
        sercom1Core                = 8,
        tc0Tc1                     = 9,
        usb                        = 10,
        evsysCh0                   = 11,
        evsysCh1                   = 12,
        evsysCh2                   = 13,
        evsysCh3                   = 14,
        evsysCh4                   = 15,
        evsysCh5                   = 16,
        evsysCh6                   = 17,
        evsysCh7                   = 18,
        evsysCh8                   = 19,
        evsysCh9                   = 20,
        evsysCh10                  = 21,
        evsysCh11                  = 22,
        sercom2Core                = 23,
        sercom3Core                = 24,
        tcc0Tcc1                   = 25,
        tc2Tc3                     = 26,
        can0                       = 27,
        can1                       = 28,
        tcc2Tcc3                   = 29,
        tc4Tc5                     = 30,
        pdec                       = 31,
        ac                         = 32,
        ccl                        = 33,
        sercom4Core                = 34,
        sercom5Core                = 35,
        tcc4                       = 38,
        adc0                       = 40,
        adc1                       = 41,
        dac                        = 42,
        i2s0                       = 43,
        i2s1                       = 44,
        sdhc0                      = 45,
        cm4Trace                   = 47
    };

    template<unsigned Generator, GeneratorSource Source, unsigned long long Div>
    using GenericClockGenerator = Sam::Gclk::Generator<ClockLimits::E5x::gclkDivBits(Generator),
                                                       ClockLimits::E5x::gclkMaxDivision(Generator),
                                                       Generator,
                                                       Source,
                                                       Div>;

    template<unsigned Generator, Peripheral PeripheralId>
    using PeripheralChannelController = Sam::Gclk::Channel<Generator, PeripheralId>;

    /// GCLK0's source and whether a GENCTRL0 write is still synchronising (the start traits wait on both).
    inline GeneratorSource gclk0Source() {
        using GC = Kvasir::Peripheral::GCLK::Registers<>;
        return get<0>(apply(read(GC::GENCTRL<0>::src)));
    }

    // SYNCBUSY.GENCTRLn is one field over bits 13:2 (14.8.2, l.6427), an enum per generator: gclk0 = bit 0 of it
    inline bool gclk0Syncing() {
        using GC = Kvasir::Peripheral::GCLK::Registers<>;
        return (static_cast<std::uint32_t>(get<0>(apply(read(GC::SYNCBUSY::genctrl))))
                & static_cast<std::uint32_t>(GC::SYNCBUSY::GENCTRLVal::gclk0))
            != 0;
    }
}}   // namespace Kvasir::GCLK
