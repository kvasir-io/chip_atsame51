#pragma once

#include <string_view>

// Which SAM E51 this firmware is built for: cmake/chip.cmake passes KVASIR_CHIP_ATSAME51J18A (with
// KVASIR_CHIP_ATSAME51 and KVASIR_CHIP_FAMILY_SAM) to every TU of the image (kvasir_chip DEFINITIONS). One part
// so far; chip.hpp includes this first.
namespace Kvasir { namespace Chip {
    enum class Part { atsame51j18a };

#ifdef KVASIR_CHIP_ATSAME51J18A
    inline constexpr Part part = Part::atsame51j18a;
#else
    #error \
      "chip_atsame51 without a KVASIR_CHIP_ATSAME51<part> define: not compiled with the package's kvasir_chip DEFINITIONS"
#endif

    // which part this image is for, as text
    inline constexpr std::string_view partName = "ATSAME51J18A";
}}   // namespace Kvasir::Chip
