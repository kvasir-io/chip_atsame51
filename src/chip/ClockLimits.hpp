#pragma once
// The SAM D5x/E5x's clock limits, for chip_atsam_common/ClockSolver.hpp. Each number from
// DS60001507N ("line" is the line of SAMD5x_E5x_Family_Datasheet.md) and its errata DS80000748T
// (SAMD5x_E5x_Errata.md).
//
// EFP ("Extended Flash Performance") is an ordering-code extension that "will not be printed onto
// the package marking" (Ordering Information, line 1014) and exists for grade U only (line 1012):
// neither the part name (chip.cmake's ATSAME51J18A) nor the chip says which table holds. A firmware
// that writes RWS names its flash; one that cannot know uses AUTOWS (smart_hive).
#include "atsam_common/ClockSolver.hpp"

namespace Kvasir::ClockLimits::E5x {
// FDPLL200Mn: f_IN 32..3200 kHz, f_OUT 96..200 MHz (Table 54-53, line 86950: "based on simulation",
// and "applicable with LDO regulator and a direct reference (i.e., REFCLK is XOSC or XOSC32K, not
// GCLK)"); f_GCLK_DPLLx max 3.2 MHz (Table 54-6, line 85579). LDR 13 bits, LDRFRAC 5 bits (28.8.13,
// lines 33501/33497); DIV 11 bits, f_DIV = f_XOSC / (2 (DIV + 1)) (28.8.14, line 33535);
// f = f_CKR (LDR + 1 + LDRFRAC / 32) (28.6.5, line 31933 ff.). No output prescaler.
inline constexpr Dpll                   Fdpll200m{.refMin   = 32'000,
                                                  .refMax   = 3'200'000,
                                                  .outMin   = 96'000'000,
                                                  .outMax   = 200'000'000,
                                                  .ldrBits  = 13,
                                                  .fracBits = 5,
                                                  .divBits  = 11,
                                                  .prescMax = 0};
inline constexpr Prescaler::FixedString DpllWhere = "SAM E5x FDPLL200M, Table 54-53";

// f_CPU and f_AHB max 120 MHz (Table 54-6, line 85579)
inline constexpr std::uint64_t CpuMax = 120'000'000;

// Errata 2.13.1 (line 1904, PDF page 34: revisions A and D): spurious FDPLL unlocks, workaround
// LBYPASS = 1 and WUF = 1; 2.19.1 (line 2773): FDPLL and DFLL not usable with the buck regulator.

enum class Flash : std::uint8_t {
    efp,        // Table 54-38 (EFP part numbers)
    standard,   // Table 54-39 (non-EFP)
};

// Table 54-38, line 86622: VDD > 1.71 V
inline constexpr WaitStateTable WaitStatesEfp{
  "SAM E5x Table 54-38 (EFP)",
  {{
    {Supply::from1V71,
     {19'000'000, 38'000'000, 57'000'000, 76'000'000, 95'000'000, 100'000'000, 120'000'000}},
    {},
  }}};
// Table 54-39, line 86641: note 1 VDD > 2.7 V, note 2 1.71 V < VDD <= 2.7 V
inline constexpr WaitStateTable WaitStatesStandard{
  "SAM E5x Table 54-39 (non-EFP)",
  {{
    {Supply::from2V7, {24'000'000, 51'000'000, 77'000'000, 101'000'000, 119'000'000, 120'000'000}},
    {Supply::from1V71, {22'000'000, 44'000'000, 67'000'000, 89'000'000, 111'000'000, 120'000'000}},
  }}};

// CTRLA.RWS (bits 11:8, line 27281) is used when AUTOWS = 0; AUTOWS = 1 works "at any frequency up
// to the device maximum frequency" with "a minimum of one cycle latency" (line 27326).
template<std::uint64_t Hz,
         Supply        S,
         Flash         F>
consteval unsigned waitStates() {
    if constexpr(F == Flash::efp) {
        static_assert(S == Supply::from1V71, "Table 54-38 has one row: VDD > 1.71 V");
        return Nvm::waitStatesFrom<Hz, S, WaitStatesEfp>();
    } else {
        return Nvm::waitStatesFrom<Hz, S, WaitStatesStandard>();
    }
}

// DFLL48M closed loop: f_REF 732..33000 Hz (Table 54-51, line 86917, simulation values; note 2 the
// same 2 % reference accuracy as the D21); DFLLMUL.MUL 16 bits, DFLLVAL.COARSE 6 bits, FINE 8 bits
// (28.8.10/28.8.9, lines 33373/33327/33331).
inline constexpr Dfll Dfll48m{.refMin     = 732,
                              .refMax     = 33'000,
                              .mulBits    = 16,
                              .coarseBits = 6,
                              .fineBits   = 8};

// GCLK generators: Table 14-3 "Division Factor Bits", line 6488: generator 0 8 bits / 512, 1 16
// bits / 131072, 2-11 8 bits / 512.
consteval unsigned gclkDivBits(unsigned generator) { return generator == 1 ? 16 : 8; }

consteval unsigned long long gclkMaxDivision(unsigned generator) {
    return 1ULL << (gclkDivBits(generator) + 1);
}
}   // namespace Kvasir::ClockLimits::E5x
