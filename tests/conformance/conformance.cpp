// The chip contract of chip_atsame51 as it is today (Kvasir_SDK tests/conformance/ChipContract.hpp): every row the chip
// has is REQUIRE, every row it lacks is GAP - a GAP that the chip fills fails here and moves to REQUIRE.

// the config header first: the chip below needs its log backend and log clock (uc_log.hpp would read it too, but the
// chip headers before it need the same)
#include "kvasir_config.hpp"

// then the chip and the drivers
#include "chip/WaitBounds.hpp"
#include "chip/atsam_common/ImageCheck.hpp"
#include "chip/atsam_common/NVMCTRL.hpp"
#include "chip/atsam_common/Serial_Number.hpp"
#include "chip/chip.hpp"
// the core's debug query and feature macros
#include "core/Features.hpp"
#include "cortex_common/Debug.hpp"

// last: the contract rows see only what is declared before them
#include "ChipContract.hpp"

namespace Conformance {
// the last two rows of the smallest part's main array (ATSAME51J18A: 256 KB, 8 KB blocks of 16 pages)
using Region = Kvasir::Nvm::Region<0x3'C000, 2>;

// the watchdog, gated (the health supervisor's) and ungated (a Startup entry)
struct WatchdogConfig {
    static constexpr auto overrunTime = std::chrono::seconds{2};
    static constexpr bool gatedFeed   = true;
};

struct UngatedWatchdogConfig {
    static constexpr auto overrunTime = std::chrono::seconds{1};
};
}   // namespace Conformance

// Startup hooks the SDK's Startup calls
KVASIR_CONFORMANCE_REQUIRE(
  "Startup::FirstInitStep<Tag::User>",
  requires { Kvasir::Startup::FirstInitStep<Kvasir::Tag::User>{}(); });

// the interrupt table and the pins
KVASIR_CONFORMANCE_REQUIRE("Nvic::InterruptOffsetTraits<Tag::User>::end",
                           Kvasir::Nvic::InterruptOffsetTraits<Kvasir::Tag::User>::end > 0);
KVASIR_CONFORMANCE_REQUIRE("Io::PinLocationTraits<void>",
                           Kvasir::Io::PinLocationTraits<void>::pinEnd > 0);

// reset cause, wait bounds
KVASIR_CONFORMANCE_REQUIRE("PM::resetCause()",
                           Kvasir::Conformance::Row::resetCause);
KVASIR_CONFORMANCE_REQUIRE("PM::resetKind()",
                           Kvasir::Conformance::Row::resetKind);
KVASIR_CONFORMANCE_REQUIRE("Chip::part / partName, the identity constants",
                           Kvasir::Conformance::Row::chipPart);
KVASIR_CONFORMANCE_REQUIRE("Register::CpuHzCeiling<Tag::User>",
                           Kvasir::Conformance::Row::cpuHzCeiling);
// every name the shared drivers use (atsam_common/ChipTraits.hpp; StartUp.hpp asserts the same)
KVASIR_CONFORMANCE_REQUIRE("Sam::ChipPackage<Sam::Package>",
                           Kvasir::Sam::ChipPackage<Kvasir::Sam::Package>);
// the CellStore's flash, the image check's backend
KVASIR_CONFORMANCE_REQUIRE("Hal::NorRegion (Nvm::Region)",
                           Kvasir::Hal::NorRegion<Conformance::Region>);
KVASIR_CONFORMANCE_REQUIRE("ImageCheck::Backend (ImageCheck::SamDsu)",
                           Kvasir::ImageCheck::Backend<Kvasir::ImageCheck::SamDsu<>>);
// the core of this chip (core_cortex_m4)
KVASIR_CORE_CONFORMANCE_REQUIRE("Core::Debug::isDebuggerConnected()",
                                Kvasir::Conformance::Row::coreDebugIsDebuggerConnected);
// known gaps (each moves to REQUIRE when the chip gets it)
// a reboot every chip spells the same, "was a debugger seen" (DSU STATUSB.DBGPRES)
KVASIR_CONFORMANCE_REQUIRE("Chip::reboot()",
                           Kvasir::Conformance::Row::chipReboot);
KVASIR_CONFORMANCE_REQUIRE(
  "Port::Reboot<Tag::User>, the reboot hook",
  requires { Kvasir::Port::Reboot<Kvasir::Tag::User>{}(); });
KVASIR_CONFORMANCE_REQUIRE("Chip::debuggerPresent()",
                           Kvasir::Conformance::Row::chipDebuggerPresent);
// the health supervisor's watchdog (atsam_common/Wdt.hpp)
KVASIR_CONFORMANCE_REQUIRE("Kvasir::Watchdog",
                           Kvasir::Conformance::Row::kvasirWatchdog);
// the serial number, as bytes and as hex text (every chip's two functions)
KVASIR_CONFORMANCE_REQUIRE("Kvasir::serialNumber() / serialNumberString()",
                           Kvasir::Conformance::Row::serialNumber);
KVASIR_CONFORMANCE_REQUIRE(
  "health watchdog (gated feed/arm/disarm, Timeout)",
  Kvasir::Conformance::HealthWatchdog<Kvasir::Watchdog<Conformance::WatchdogConfig>>);
KVASIR_CONFORMANCE_REQUIRE(
  "health watchdog, ungated (feed, Timeout)",
  Kvasir::Health::WatchdogShape<Kvasir::Watchdog<Conformance::UngatedWatchdogConfig>>);
// "is halting debug on": off / on / unknown on every core (cortex_common/Debug.hpp)
KVASIR_CORE_CONFORMANCE_REQUIRE("Core::haltingDebug()",
                                Kvasir::Conformance::Row::coreHaltingDebug);

// the indexed GCLK generator (atsam_common/GclkIndexed.hpp): DIV is 16 bits on generator 1, 8 on the others; above
// that a power of two goes through DIVSEL (DIV = log2 - 1, 16.8.3 / 14.8.3)
namespace Conformance::Gclk {
using Kvasir::GCLK::GeneratorSource;
using Kvasir::GCLK::GenericClockGenerator;
constexpr auto src = GeneratorSource::gclkin;
static_assert(Kvasir::Sam::Gclk::directMaximum(Kvasir::ClockLimits::E5x::gclkDivBits(0)) == 255
              && Kvasir::Sam::Gclk::directMaximum(Kvasir::ClockLimits::E5x::gclkDivBits(1))
                   == 65535);
static_assert(GenericClockGenerator<2,
                                    src,
                                    255>::division.div
                == 255
              && GenericClockGenerator<2,
                                       src,
                                       256>::division.div
                   == 7);
static_assert(GenericClockGenerator<2,
                                    src,
                                    (1ULL << 20)>::division.div
              == 19);
static_assert(GenericClockGenerator<1,
                                    src,
                                    65535>::division.div
                == 65535
              && GenericClockGenerator<1,
                                       src,
                                       (1ULL << 16)>::division.div
                   == 15);
static_assert(GenericClockGenerator<2,
                                    src,
                                    512>::divisionInTable
              && !GenericClockGenerator<2,
                                        src,
                                        1024>::divisionInTable);
}   // namespace Conformance::Gclk

// the clock tree's kinds (chip/ClockTree.hpp): the numbers each one takes, no image needed
namespace Conformance::ClockTree {
namespace CS = Kvasir::Clocks::Sam;
// XOSC0 8 MHz -> FDPLL0 at 120 MHz: the solver's DIV 1 (2 MHz), LDR 59, generator 0 / 1; AUTOWS (no RWS)
using Fdpll = CS::Tree<{.cpu = 120'000'000, .source = {.hz = 8'000'000}}>;
static_assert(Fdpll::dpll.div == 1 && Fdpll::dpll.ldr == 59 && Fdpll::dpll.gclkDiv == 1
              && Fdpll::waitStates == 0);
// a firmware's own numbers: DIV 2 (1.33 MHz), LDR 89; RWS from Table 54-39 above 2.7 V: 5 at 120 MHz
using Own = CS::Tree<{.cpu    = 120'000'000,
                      .source = {.hz = 8'000'000},
                      .dpll   = Kvasir::DPLL::checkedSetting<8'000'000, 2, 89>(),
                      .flash  = {.automatic = false}}>;
static_assert(Own::dpll.div == 2 && Own::dpll.ldr == 89 && Own::waitStates == 5);
// the DFLL48M open loop, RWS: the open loop's worst 50.63 MHz (Table 54-50 and the 125 C chapter's) -> 1 wait state
// above 2.7 V (Table 54-39); on the EFP table (Table 54-38, one row above 1.71 V: 38 MHz for 1) 2
using Open
  = CS::Tree<{.cpu = 48'000'000, .kind = CS::Source::dfllOpenLoop, .flash = {.automatic = false}}>;
static_assert(Open::waitStates == 1);
using OpenEfp = CS::Tree<{
  .cpu   = 48'000'000,
  .kind  = CS::Source::dfllOpenLoop,
  .flash = {.automatic = false,
            .table     = Kvasir::ClockLimits::E5x::Flash::efp,
            .supply    = Kvasir::ClockLimits::Supply::from1V71}
}>;
static_assert(OpenEfp::waitStates == 2);
// XOSCCTRL's current setting by frequency (Table 28-7)
static_assert(Kvasir::ClockLimits::E5x::xoscCurrentFor(8'000'000).imult == 3
              && Kvasir::ClockLimits::E5x::xoscCurrentFor(8'000'000).iptat == 2);
}   // namespace Conformance::ClockTree

// The clock gates (chip/Blocks.hpp over atsam_common/ClockGate.hpp): a block on an AHB and an APB clock has a row per
// mask bit (DS60001507 15.6.2.6), so enable<> is a list of both; the USB's AHB bit is set out of reset, its APB bit not
// (Table 15-1), so a USB write claims one gate; the DSU's are both set, a DSU write claims none.
namespace Conformance { namespace Gates {
    namespace P = Kvasir::Peripheral;
    using M     = P::MCLK::Registers<>;
    using Table = Kvasir::Chip::Blocks::Table;
    static_assert(std::is_same_v<Kvasir::PM::Enable<P::DSU::Registers<>::baseAddr>::action,
                                 brigand::list<decltype(set(M::AHBMASK::dsu_)),
                                               decltype(set(M::APBBMASK::dsu_))>>);
    static_assert(Table::claimsAt(P::DSU::Registers<>::baseAddr) == 0
                  && Table::claimsAt(P::USB_DEVICE::Registers<>::baseAddr) == 1
                  && Table::rowsOf(P::USB_DEVICE::Registers<>::baseAddr) == 2
                  && Table::rowsOf(P::NVMCTRL::Registers<>::baseAddr) == 2
                  && Table::rowsOf(P::PAC::Registers<>::baseAddr) == 2
                  && Table::rowsOf(P::ICM::Registers<>::baseAddr) == 2
                  && Table::rowsOf(P::QSPI::Registers<>::baseAddr) == 2);
}}   // namespace Conformance::Gates
