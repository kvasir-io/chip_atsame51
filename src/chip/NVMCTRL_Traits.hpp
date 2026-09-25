#pragma once

#include "peripherals/NVMCTRL.hpp"

// The D5x/E5x NVMCTRL is not the D21/C21 one: commands go through CTRLB, the erase unit is a
// block of 16 pages (the "row" of the shared driver), the ready flag is in STATUS, and there is
// no RWW EEPROM section (SAM D5x/E5x data sheet, DS60001507N, chapter 25).
namespace Kvasir { namespace NVMCTRL { namespace Traits {

    using NVM = Kvasir::Peripheral::NVMCTRL::Registers<>;

    static inline bool ready() { return 0 != apply(read(NVM::STATUS::ready)); }

    /// The error flags are sticky and a set one does not block the next command, but it would be
    /// read back as that command's result; INTFLAG bits are cleared by writing a one.
    static inline void clear_flags() {
        apply(set(NVM::INTFLAG::addre),
              set(NVM::INTFLAG::proge),
              set(NVM::INTFLAG::done),
              set(NVM::INTFLAG::eccse));
    }

    static inline void command_write_page() {
        clear_flags();
        apply(write(NVM::CTRLB::CMDValC::wp), write(NVM::CTRLB::CMDEXValC::key));
    }

    static inline void command_erase_row() {
        clear_flags();
        apply(write(NVM::CTRLB::CMDValC::eb), write(NVM::CTRLB::CMDEXValC::key));
    }

    static inline void command_page_buffer_clear() {
        clear_flags();
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
    static constexpr std::size_t PageSize    = 512;
    static constexpr std::size_t PagesPerRow = 16;
}}}   // namespace Kvasir::NVMCTRL::Traits
