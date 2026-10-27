// src/chip/ResetCause.hpp: RSTC.RCAUSE's causes (DS60001507N 16.8.1) mapped onto the SDK's ResetKind, every
// enumerator. All static_asserts: building is the test.
#include "ResetCause.hpp"

namespace {
using namespace Kvasir::PM;
using Kvasir::ResetKind;

static_assert(toResetKind(ResetCause::por) == ResetKind::powerOn);
static_assert(toResetKind(ResetCause::bod12) == ResetKind::brownOut);
static_assert(toResetKind(ResetCause::bod33) == ResetKind::brownOut);
static_assert(toResetKind(ResetCause::nvm) == ResetKind::unknown);
static_assert(toResetKind(ResetCause::ext) == ResetKind::external);
static_assert(toResetKind(ResetCause::wdt) == ResetKind::watchdogTimeout);
static_assert(toResetKind(ResetCause::syst) == ResetKind::softwareRequest);
static_assert(toResetKind(ResetCause::backup) == ResetKind::powerOn);
static_assert(toResetKind(ResetCause::unknown) == ResetKind::unknown);
}   // namespace

int main() { return 0; }
