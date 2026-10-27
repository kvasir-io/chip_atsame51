#pragma once
// The bounds of the SAM D5x/E5x package's register waits (Kvasir_SDK kvasir/Register/Wait.hpp), each with its source
// (DS60001507N, "l." = line of SAMD5x_E5x_Family_Datasheet.md; errata DS80000748T). Under the default wait policy
// (Unbounded) every site is the loop it was.
#include "ClockLimits.hpp"
#include "kvasir/Register/Wait.hpp"

#include <cstdint>

// f_CPU max 120 MHz (Table 54-6, l.85579): the SDK's MicrosecondsBound converts at this, a slower clock only lengthens
// the real time. Specialised before any bound below names it.
template<>
struct Kvasir::Register::CpuHzCeiling<Kvasir::Tag::User> {
    static constexpr std::uint64_t value = Kvasir::ClockLimits::E5x::cpuMax;
};

namespace Kvasir::Chip {
// STATUS.DFLLRDY after DFLLCTRLA.ENABLE in open loop: TOpenSTARTUP max 7 us (Table 54-50, l.86909), x14
using DfllReadyBound = Kvasir::Register::MicrosecondsBound<100>;

// DPLLSTATUS.CLKRDY after DPLLCTRLA.ENABLE: tLOCK max 95 us at f_IN 3.2 MHz (Table 54-53, l.86960) is the only time
// the table gives - no start-up time, no row for a lower f_IN (unchecked). 5 ms leaves room for a 100x lower reference.
using DpllReadyBound = Kvasir::Register::MicrosecondsBound<5'000>;

// Errata 2.13.1 (revisions A and D, PDF page 34): "A 10 ms delay is also suggested after the clock ready bit is set".
// A minimum, not a bound: this many DPLLSTATUS reads take at least 10 ms at any core clock up to the ceiling (a poll is
// no fewer than minCyclesPerPoll cycles); at the 48 MHz of the DFLL after a reset they take longer.
inline constexpr std::uint32_t dpllSettlePolls
  = static_cast<std::uint32_t>(10'000 * Register::CpuHzCeiling<Tag::User>::value / 1'000'000
                               / Kvasir::Register::minCyclesPerPoll);
}   // namespace Kvasir::Chip

// the shared SAM drivers' bounds (Sam::SyncBound, ...): they need the ceiling above
#include "atsam_common/SamWaitBounds.hpp"
