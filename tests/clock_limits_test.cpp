// src/chip/ClockLimits.hpp (SAM D5x/E5x) through chip_atsam_common's ClockSolver.hpp, as the firmwares
// use it: FDPLL200M, the EFP and non-EFP wait-state tables.
#include "ClockLimits.hpp"
#include "DpllSetting.hpp"

#include <cstdio>

namespace {
using namespace Kvasir;
namespace L = ClockLimits::E5x;
using ClockLimits::Supply;
using L::Flash;

// a firmware's own hand-written numbers (DIV 2, LDR 89: 1.333 MHz x 90 = 120 MHz), inside every range
constexpr auto hive = DPLL::checkXosc<L::fdpll200m>(8'000'000, 2, 89, 0);
static_assert(hive.ok() && hive.out.num == 120'000'000 * hive.out.den);
static_assert(DPLL::assertXoscSetting<L::fdpll200m,
                                      L::dpllWhere,
                                      8'000'000,
                                      2,
                                      89,
                                      0>()
                .ok());
// the solver's choice differs: DIV 0 (4 MHz) is above 3.2 MHz, DIV 1 (2 MHz, whole hertz, not on
// a limit) x 60
constexpr auto d120 = DPLL::fromXosc<L::fdpll200m>(8'000'000, 120'000'000);
static_assert(d120.found && d120.div == 1 && d120.ldr == 59 && d120.gclkDiv == 1);
// 48 MHz is below the DCO's 96 MHz: 96 / 2 by the generator
constexpr auto d48 = DPLL::fromXosc<L::fdpll200m>(8'000'000, 48'000'000);
static_assert(d48.found && d48.ldr == 47 && d48.gclkDiv == 2 && d48.div == 1);
// LDRFRAC is 1/32 here
constexpr auto frac
  = DPLL::fromReference<L::fdpll200m>(32'768, 120'000'000, Prescaler::Tolerance::ppm(10), true);
static_assert(frac.found && frac.ldr == 3661 && frac.ldrFrac == 4);

// DpllSetting.hpp: the solver through the chip's wrapper, and a hand-written setting checked into a Setting
constexpr auto w120 = DPLL::fromXosc<8'000'000, 120'000'000>();
static_assert(w120.found && w120.div == 1 && w120.ldr == 59 && w120.ldrFrac == 0
              && w120.gclkDiv == 1);
constexpr auto own = DPLL::checkedSetting<8'000'000, 2, 89>();
static_assert(own.found && own.div == 2 && own.ldr == 89 && own.ldrFrac == 0 && own.presc == 0
              && own.gclkDiv == 1);
static_assert(own.achieved.num == 120'000'000 * own.achieved.den
              && own.ref.num * 3 == 4'000'000 * own.ref.den);

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

static_assert(DFLL::closedLoop<L::dfll48m>(32'768,
                                           48'000'000)
                .mul
              == 1465);
static_assert(DFLL::maxCoarseStep<L::dfll48m> == 31 && DFLL::maxFineStep<L::dfll48m> == 127);
static_assert(L::gclkMaxDivision(0) == 512 && L::gclkMaxDivision(11) == 512);

// XOSCCTRLn.IMULT / IPTAT by frequency (Table 28-7): 8 MHz 3/2, >8..16 4/3, >16..24 5/3, >24..48 6/3
consteval bool current(std::uint64_t hz,
                       unsigned      imult,
                       unsigned      iptat) {
    auto const c = L::xoscCurrentFor(hz);
    return c.imult == imult && c.iptat == iptat;
}

static_assert(current(8'000'000,
                      3,
                      2)
              && current(8'000'001,
                         4,
                         3)
              && current(16'000'000,
                         4,
                         3)
              && current(16'000'001,
                         5,
                         3)
              && current(24'000'000,
                         5,
                         3)
              && current(24'000'001,
                         6,
                         3)
              && current(48'000'000,
                         6,
                         3));
// the flash wait states of the DFLL48M open loop's worst case (50.63 MHz): 1 above 2.7 V, 2 at 1.71..2.7 V (44 MHz is
// the 1-wait-state limit there) and on the EFP table (38 MHz)
static_assert(L::waitStates<L::dfllOpenLoopMaxHz,
                            Supply::from2V7,
                            Flash::standard>()
                == 1
              && L::waitStates<L::dfllOpenLoopMaxHz,
                               Supply::from1V71,
                               Flash::standard>()
                   == 2
              && L::waitStates<L::dfllOpenLoopMaxHz,
                               Supply::from1V71,
                               Flash::efp>()
                   == 2);
}   // namespace

int main() {
    std::puts("clock solver (SAM E5x): every check is a static_assert");
    return 0;
}
