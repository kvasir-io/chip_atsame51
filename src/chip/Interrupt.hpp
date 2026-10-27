#pragma once
#include "core/CoreInterrupts.hpp"
#include "kvasir/Common/Interrupt.hpp"
#include "kvasir/Port/Interrupts.hpp"
#include "peripherals/Interrupts.hpp"

#include <array>
#include <type_traits>

namespace Kvasir {
// The core's exceptions (core/CoreInterrupts.hpp) and the device lines of this part's SVD (svd_converter:
// peripherals/Interrupts.hpp, the SVD's names lower-cased without IRQ).
// The gaps: 42 to 44 and 133 are reserved on the whole family (DS60001507N Table 10-1), 70 to 77 (SERCOM6/7), 84
// (GMAC), 113 and 114 (TC6/7) belong to peripherals the 64-pin E51 does not have (Table 1-2).
struct Interrupt
  : CoreInterrupts
  , Peripheral::DeviceInterrupts {
    // The vector that serves EXTINT line n: a line each, EXTINT 0 on 12 to EXTINT 15 on 27
    // (DS60001507N Table 10-1, md lines 2785-2800).
    template<unsigned Line>
    using EicExtIntVector
      = Type<std::remove_cvref_t<decltype(eic_extint_0)>::value + static_cast<int>(Line)>;
};

namespace Nvic {
    // the core's reserved slots (the Cortex-M4's, -4 the debug monitor unused here), then the gaps
    template<>
    struct InterruptOffsetTraits<Kvasir::Tag::User>
      : Core::ChipInterruptTraits<Core::InterruptRules,
                                  Peripheral::interruptEnd,
                                  Peripheral::interruptGaps> {};
}   // namespace Nvic
}   // namespace Kvasir
