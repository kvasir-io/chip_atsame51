#pragma once
#include <array>

namespace Kvasir { namespace SerialNumber { namespace Traits {

    /// SAM D5x/E5x data sheet (DS60001507N) 9.6 "Serial Number", words 0 to 3.
    static constexpr std::array<std::uint32_t, 4> addresses{0x008061FC,
                                                            0x00806010,
                                                            0x00806014,
                                                            0x00806018};

}}}   // namespace Kvasir::SerialNumber::Traits
