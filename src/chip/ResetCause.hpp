#pragma once

#include "kvasir/Port/ResetKind.hpp"

#include <cstdint>

// The reset causes of RSTC.RCAUSE (SAM D5x/E5x data sheet DS60001507N 16.8.1) and their mapping onto the SDK's
// ResetKind, without registers (host-tested in tests/reset_cause_test.cpp); PM.hpp reads the register.
namespace Kvasir { namespace PM {
    /// Named as 16.8.1 names the bits: BOD12 (bit 1, md l.8293) and BOD33 (bit 2, md l.8291) - the SVD calls them
    /// BODCORE/BODVDD; `unknown`: no bit or more than one bit set (atsam_common/ResetCauseDecode.hpp). The names are
    /// printed by the boot report.
    enum class ResetCause : std::uint8_t {
        por,
        bod12,
        bod33,
        nvm,
        ext,
        wdt,
        syst,
        backup,
        unknown
    };

    // resetCause() as the SDK's boot guard sees it: RCAUSE holds "the latest Reset cause" (16.6.2.3). A debugger's
    // reset is SYST like a software request: without a record the guard counts neither.
    [[nodiscard]] constexpr ResetKind toResetKind(ResetCause c) {
        switch(c) {
        case ResetCause::por:
        // a Backup or Hibernate exit (16.8.1 bit 7): a power domain coming back
        case ResetCause::backup: return ResetKind::powerOn;
        case ResetCause::bod12:
        case ResetCause::bod33:  return ResetKind::brownOut;
        case ResetCause::ext:    return ResetKind::external;
        case ResetCause::wdt:    return ResetKind::watchdogTimeout;
        case ResetCause::syst:   return ResetKind::softwareRequest;
        // NVMCTRL's reset, e.g. a BKSWRST bank swap (16.6.2.3): no kind of its own
        case ResetCause::nvm:
        case ResetCause::unknown: return ResetKind::unknown;
        }
        return ResetKind::unknown;
    }
}}   // namespace Kvasir::PM
