#pragma once

#include "ClockLimits.hpp"
#include "atsam_common/NvmResult.hpp"
#include "kvasir/Register/Wait.hpp"
#include "peripherals/NVMCTRL.hpp"

// The D5x/E5x NVMCTRL is not the D21/C21 one: commands go through CTRLB, the erase unit is a
// block of 16 pages (the "row" of the shared driver), the ready flag is in STATUS, and there is
// no RWW EEPROM section (SAM D5x/E5x data sheet, DS60001507N, chapter 25).
namespace Kvasir { namespace NVMCTRL { namespace Traits {

    using NVM = Kvasir::Peripheral::NVMCTRL::Registers<>;

    static inline bool ready() { return 0 != apply(read(NVM::STATUS::ready)); }

    /// INTFLAG.DONE (bit 0), ADDRE (1), PROGE (2), LOCKE (3), NVME (6) are sticky and cleared by writing a one
    /// (DS60001507N 25.8.6); a set one does not block the next command but would be read back as its result. ECCSE /
    /// ECCDE are read-only, cleared by reading ECCERR: read errors, not command errors, so not touched here. Only
    /// what was read is written back. ADDRE counts as a program error (an address command "fails with PROGE while
    /// ADDRE is set", 25.6.6).
    static inline std::uint8_t takeErrors() {
        static constexpr std::uint16_t Errors = NVM::INTFLAG::done.Mask | NVM::INTFLAG::addre.Mask
                                              | NVM::INTFLAG::proge.Mask | NVM::INTFLAG::locke.Mask
                                              | NVM::INTFLAG::nvme.Mask;
        auto const                     flags
          = static_cast<std::uint16_t>(get<0>(apply(read(NVM::INTFLAG::FULLREGISTER))) & Errors);
        if(flags != 0) { apply(write(NVM::INTFLAG::FULLREGISTER, flags)); }
        return Kvasir::Nvm::Detail::bits(
          (flags & NVM::INTFLAG::locke.Mask) != 0,
          (flags & (NVM::INTFLAG::addre.Mask | NVM::INTFLAG::proge.Mask)) != 0,
          (flags & NVM::INTFLAG::nvme.Mask) != 0);
    }

    // tFEB (block erase) max 200 ms, tFPW (page write) max 3 ms (DS60001507N Table 54-40, md l.86663-86665; "based
    // on simulation"); x2, at the highest CPU clock (Table 54-6)
    using EraseBound = Kvasir::Register::Bound::Microseconds<400'000, ClockLimits::E5x::CpuMax>;
    using WriteBound = Kvasir::Register::Bound::Microseconds<6'000, ClockLimits::E5x::CpuMax>;

    /// Declared for the shared driver's RWWEE branch, never instantiated here (no RWW section).
    template<typename T>
    static inline void prepare_erase_rww(T addr);

    static inline void command_write_page() {
        apply(write(NVM::CTRLB::CMDValC::wp), write(NVM::CTRLB::CMDEXValC::key));
    }

    static inline void command_erase_row() {
        apply(write(NVM::CTRLB::CMDValC::eb), write(NVM::CTRLB::CMDEXValC::key));
    }

    static inline void command_page_buffer_clear() {
        apply(write(NVM::CTRLB::CMDValC::pbc), write(NVM::CTRLB::CMDEXValC::key));
    }

    /// Declared for the shared driver's isMainFlash == false branch, which is never instantiated
    /// here: this part has no RWW section, so there is nothing to define them with.
    static inline void command_write_rww();
    static inline void command_erase_rww();

    /// CTRLA.CACHEDIS0/1, one per AHB interface.
    static inline void disable_cache() {
        apply(set(NVM::CTRLA::cachedis0), set(NVM::CTRLA::cachedis1));
    }

    static inline void enable_cache() {
        apply(clear(NVM::CTRLA::cachedis0), clear(NVM::CTRLA::cachedis1));
    }

    /// ADDR[23:0] is a byte address (25.8.8), not the D21's word address. All 24 bits: the user
    /// row at 0x00804000 has bit 23 set.
    template<typename T>
    void set_addr(T addr) {
        apply(
          write(NVM::ADDR::addr,
                reinterpret_cast<std::uint32_t>(addr) & Kvasir::Register::maskFromRange(23, 0)));
    }

    /// 512-byte pages, 16 of them to the erase block (25.6.2: "sixteen pages and page size is
    /// 512 bytes").
    /// The shared driver asks before every command whether the range is inside the array (D21/C21 wrap an address
    /// past the end silently). The E5x checks it itself: INTFLAG.ADDRE, a program error to takeErrors() (25.6.6).
    static constexpr bool inArray(std::uint32_t,
                                  std::uint32_t,
                                  bool) {
        return true;
    }

    static constexpr std::size_t PageSize    = 512;
    static constexpr std::size_t PagesPerRow = 16;
}}}   // namespace Kvasir::NVMCTRL::Traits
