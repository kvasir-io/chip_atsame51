// What the SAM E5x refuses at compile time, as must-fail cases (kvasir_conformance_must_fail, CMakeLists.txt): the
// clock tree (Kvasir::Clocks::Sam::Tree, chip/ClockTree.hpp over atsam_common/ClockTree.hpp), the crystal
// oscillators, the FDPLL solver and an FDPLL setting of the firmware's own. TREE_FAIL=0 must compile: the control
// names an 8 MHz crystal board's tree with an FDPLL setting of its own (XOSC0 -> FDPLL0 DIV 2 / LDR 89 -> 120 MHz), the
// same with RWS instead of AUTOWS, the solver's 120 MHz and the reset's open loop. Each of 1..n names one setting the
// chip must refuse, and its ctest passes only on that case's own message (the case list in CMakeLists.txt).

#include "kvasir_config.hpp"
//
#include <chip/ClockTree.hpp>
#include <chip/chip.hpp>

#ifndef TREE_FAIL
    #error "TREE_FAIL=<0..n> selects the case"
#endif

namespace {
namespace CS                    = Kvasir::Clocks::Sam;
inline constexpr auto crystalHz = 8'000'000;
inline constexpr auto Cpu       = 120'000'000;

using Board    = CS::Tree<{.cpu    = Cpu,
                           .source = {.hz = crystalHz},
                           .dpll   = Kvasir::DPLL::checkedSetting<crystalHz, 2, 89>(),
                           .xosc   = {.startup = 0xE}}>;
using Rws      = CS::Tree<{.cpu    = Cpu,
                           .source = {.hz = crystalHz},
                           .dpll   = Kvasir::DPLL::checkedSetting<crystalHz, 2, 89>(),
                           .flash  = {.automatic = false}}>;
using Solved   = CS::Tree<{.cpu = Cpu, .source = {.hz = crystalHz}}>;
using OpenLoop = CS::Tree<{.cpu = 48'000'000, .kind = CS::Source::dfllOpenLoop}>;
static_assert(Board::dpll.div == 2 && Board::dpll.ldr == 89 && Rws::waitStates == 5);
static_assert(Solved::dpll.div == 1 && Solved::dpll.ldr == 59);

[[maybe_unused]] void valid() {
    Board::coreClockInit();
    Rws::coreClockInit();
    Solved::coreClockInit();
    OpenLoop::coreClockInit();
}

#if TREE_FAIL == 1   // f_CPU above the part's 120 MHz (Table 54-6)
using Refused = CS::Tree<{.cpu = 121'000'000, .source = {.hz = crystalHz}}>;
#elif TREE_FAIL == 2   // the D21's USB recovery: the E5x Tree has no such kind
using Refused = CS::Tree<{.cpu = 48'000'000, .kind = CS::Source::dfllUsbRecovery}>;
#elif TREE_FAIL == 3   // a .dpll whose reference is outside Table 54-53 (DIV 0: 4 MHz > 3.2 MHz)
using Refused = CS::Tree<{.cpu    = Cpu,
                          .source = {.hz = crystalHz},
                          .dpll   = Kvasir::DPLL::checkedSetting<crystalHz, 0, 29>()}>;
#elif TREE_FAIL == 4   // a .dpll made for a 12 MHz crystal on the 8 MHz one
using Refused = CS::Tree<{.cpu    = Cpu,
                          .source = {.hz = crystalHz},
                          .dpll   = Kvasir::DPLL::checkedSetting<12'000'000, 2, 59>()}>;
#elif TREE_FAIL == 5   // the open loop is 48 MHz, generator 0 /1, as the reset leaves it
using Refused = CS::Tree<{.cpu = 24'000'000, .kind = CS::Source::dfllOpenLoop}>;
#elif TREE_FAIL == 6   // XOSC2: the E5x has two crystal oscillators
using Refused = CS::Tree<{.cpu = Cpu, .source = {.hz = crystalHz}, .xoscNumber = 2}>;
#elif TREE_FAIL == 7   // a crystal below the XOSC's range (Table 54-44: 8..48 MHz)
using Refused = CS::Tree<{.cpu = Cpu, .source = {.hz = 4'000'000}}>;
#elif TREE_FAIL == 8   // a clock on XIN above 48 MHz (Table 54-43)
using Refused = CS::Tree<{
  .cpu    = Cpu,
  .source = {.hz = 50'000'000, .crystal = false}
}>;
#elif TREE_FAIL                                                                          \
  == 9   // no FDPLL setting makes the CPU clock exactly from this crystal (tolerance 0)
using Refused = CS::Tree<{.cpu = 119'999'999, .source = {.hz = crystalHz}}>;
#endif
#if TREE_FAIL != 0
[[maybe_unused]] void refused() { Refused::coreClockInit(); }
#endif
}   // namespace
