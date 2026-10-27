#pragma once

#include "ClockLimits.hpp"
#include "WaitBounds.hpp"
#include "atsam_common/NvmResult.hpp"
#include "peripherals/NVMCTRL.hpp"

// The D5x/E5x NVMCTRL is not the D21/C21 one: commands go through CTRLB, the erase unit is a
// block of 16 pages (the "row" of the shared driver), the ready flag is in STATUS, and there is
// no RWW EEPROM section (SAM D5x/E5x data sheet, DS60001507N, chapter 25).
namespace Kvasir { namespace NVMCTRL {
    struct Traits {
        using NVM = Kvasir::Peripheral::NVMCTRL::Registers<>;

        // no RWW EEPROM section: the shared driver refuses a store on Nvm::Array::rwwee at compile time
        static constexpr bool hasRwweeArray = false;

        static bool ready() { return 0 != apply(read(NVM::STATUS::ready)); }

        /// INTFLAG.DONE (bit 0), ADDRE (1), PROGE (2), LOCKE (3), NVME (6) are sticky and cleared by writing a one
        /// (DS60001507N 25.8.6); a set one does not block the next command but would be read back as its result. ECCSE /
        /// ECCDE are read-only, cleared by reading ECCERR: read errors, not command errors, so not touched here. Only
        /// what was read is written back. ADDRE counts as a program error (an address command "fails with PROGE while
        /// ADDRE is set", 25.6.6).
        static std::uint8_t takeErrors() {
            static constexpr std::uint16_t errors
              = NVM::INTFLAG::done.mask | NVM::INTFLAG::addre.mask | NVM::INTFLAG::proge.mask
              | NVM::INTFLAG::locke.mask | NVM::INTFLAG::nvme.mask;
            auto const flags = static_cast<std::uint16_t>(
              get<0>(apply(read(NVM::INTFLAG::FULLREGISTER))) & errors);
            if(flags != 0) { apply(write(NVM::INTFLAG::FULLREGISTER, flags)); }
            return Kvasir::Nvm::ErrorBits::bits(
              (flags & NVM::INTFLAG::locke.mask) != 0,
              (flags & (NVM::INTFLAG::addre.mask | NVM::INTFLAG::proge.mask)) != 0,
              (flags & NVM::INTFLAG::nvme.mask) != 0);
        }

        // tFEB (block erase) max 200 ms, tFPW (page write) max 3 ms (DS60001507N Table 54-40, md l.86663-86665; "based
        // on simulation"); x2, at the highest CPU clock (Table 54-6)
        using EraseBound = Kvasir::Register::MicrosecondsBound<400'000>;
        using WriteBound = Kvasir::Register::MicrosecondsBound<6'000>;

        static void commandWritePage() {
            apply(write(NVM::CTRLB::CMDValC::wp), write(NVM::CTRLB::CMDEXValC::key));
        }

        static void commandEraseRow() {
            apply(write(NVM::CTRLB::CMDValC::eb), write(NVM::CTRLB::CMDEXValC::key));
        }

        static void commandPageBufferClear() {
            apply(write(NVM::CTRLB::CMDValC::pbc), write(NVM::CTRLB::CMDEXValC::key));
        }

        /// CTRLA.CACHEDIS0/1, one per AHB interface. Off around page-buffer writes: "NVM reads could be corrupted when
        /// mixing NVM reads with Page Buffer writes", workaround "writing a one to CTRLA.CACHEDIS0 and CTRLA.CACHEDIS1"
        /// (errata DS80000748T 2.14.1, md l.1956-1962; revisions A and D, read on the PDF page 35).
        static void disableCache() {
            apply(set(NVM::CTRLA::cachedis0), set(NVM::CTRLA::cachedis1));
        }

        static void enableCache() {
            apply(clear(NVM::CTRLA::cachedis0), clear(NVM::CTRLA::cachedis1));
        }

        /// ADDR[23:0] is a byte address (25.8.8), not the D21's word address. All 24 bits: the user
        /// row at 0x00804000 has bit 23 set.
        template<typename T>
        static void setAddr(T addr) {
            apply(write(NVM::ADDR::addr,
                        reinterpret_cast<std::uint32_t>(addr)
                          & Kvasir::Register::maskFromRange(23, 0)));
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

        static constexpr std::size_t pageSize    = 512;
        static constexpr std::size_t pagesPerRow = 16;
    };
}}   // namespace Kvasir::NVMCTRL
