// src/chip/ClockLimits.hpp (SAM D5x/E5x) through chip_atsam_common's ClockSolver.hpp, as the firmwares
// use it: FDPLL200M, the EFP and non-EFP wait-state tables.
#include "ClockLimits.hpp"

#include <cstdio>

namespace {
using namespace Kvasir;
namespace L = ClockLimits::E5x;
using ClockLimits::Supply;
using L::Flash;

// smart_hive keeps its own numbers (DIV 2, LDR 89: 1.333 MHz x 90 = 120 MHz), inside every range
constexpr auto hive = DPLL::checkXosc<L::Fdpll200m>(8'000'000, 2, 89, 0);
static_assert(hive.ok() && hive.out.num == 120'000'000 * hive.out.den);
static_assert(DPLL::assertXoscSetting<L::Fdpll200m,
                                      L::DpllWhere,
                                      8'000'000,
                                      2,
                                      89,
                                      0>()
                .ok());
// the solver's choice differs: DIV 0 (4 MHz) is above 3.2 MHz, DIV 1 (2 MHz, whole hertz, not on
// a limit) x 60
constexpr auto d120 = DPLL::fromXosc<L::Fdpll200m>(8'000'000, 120'000'000);
static_assert(d120.found && d120.div == 1 && d120.ldr == 59 && d120.gclkDiv == 1);
// 48 MHz is below the DCO's 96 MHz: 96 / 2 by the generator
constexpr auto d48 = DPLL::fromXosc<L::Fdpll200m>(8'000'000, 48'000'000);
static_assert(d48.found && d48.ldr == 47 && d48.gclkDiv == 2 && d48.div == 1);
// LDRFRAC is 1/32 here
constexpr auto frac
  = DPLL::fromReference<L::Fdpll200m>(32'768, 120'000'000, Prescaler::Tolerance::ppm(10), true);
static_assert(frac.found && frac.ldr == 3661 && frac.ldrFrac == 4);

// Table 54-38 (EFP): 120 MHz takes 6; Table 54-39 (non-EFP): 5 above 2.7 V, 6 is not a column
static_assert(L::waitStates<120'000'000,
                            Supply::from1V71,
                            Flash::efp>()
              == 6);
static_assert(L::waitStates<100'000'000,
                            Supply::from1V71,
                            Flash::efp>()
              == 5);
static_assert(L::waitStates<120'000'000,
                            Supply::from2V7,
                            Flash::standard>()
              == 5);
static_assert(L::waitStates<119'000'000,
                            Supply::from2V7,
                            Flash::standard>()
              == 4);
static_assert(L::waitStates<24'000'000,
                            Supply::from2V7,
                            Flash::standard>()
              == 0);
static_assert(L::waitStates<22'000'001,
                            Supply::from1V71,
                            Flash::standard>()
              == 1);
static_assert(L::waitStates<120'000'000,
                            Supply::from1V71,
                            Flash::standard>()
              == 5);

static_assert(DFLL::closedLoop<L::Dfll48m>(32'768,
                                           48'000'000)
                .mul
              == 1465);
static_assert(DFLL::MaxCoarseStep<L::Dfll48m> == 31 && DFLL::MaxFineStep<L::Dfll48m> == 127);
static_assert(L::gclkMaxDivision(0) == 512 && L::gclkMaxDivision(11) == 512);
}   // namespace

int main() {
    std::puts("clock solver (SAM E5x): every check is a static_assert");
    return 0;
}
