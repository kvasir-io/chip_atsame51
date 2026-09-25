#pragma once

#include "kvasir/Register/Register.hpp"
#include "kvasir/Register/Utility.hpp"

#include <cstdint>
#include <string_view>

namespace Kvasir {
namespace MCLK {
    template<unsigned baseAddr = 0x40000800>
    struct Registers {
        static constexpr unsigned BaseAddr = baseAddr;
    };

    namespace Detail {
        template<unsigned Offset, int BitPos>
        using BitSet = Register::Action<
          Kvasir::Register::WOBitLocT<Register::Address<Registers<>::BaseAddr + Offset>, BitPos>,
          Kvasir::Register::WriteLiteralAction<(1U << unsigned(BitPos))>>;

        template<unsigned Offset, int BitPos>
        using BitClear = Register::Action<
          Kvasir::Register::WOBitLocT<Register::Address<Registers<>::BaseAddr + Offset>, BitPos>,
          Kvasir::Register::WriteLiteralAction<0>>;

        struct PeripheryEnableInfo {
            unsigned address;
            unsigned offset;
            unsigned bit;
        };

        // SAM D5x/E5x data sheet (DS60001507N), 15.8.6 to 15.8.10: AHBMASK at 0x10, APBAMASK at
        // 0x14, APBBMASK 0x18, APBCMASK 0x1C, APBDMASK 0x20; the addresses are chip.svd's. One
        // entry per peripheral: every AHB clock is on out of reset (AHBMASK resets to
        // 0x00FFFFFF), so a peripheral that has both an AHB and an APB bit (PAC, DSU, NVMCTRL,
        // USB, HMATRIX, ICM, QSPI) is listed with its APB bit, which is the one that is off. The
        // AHB-only ones (CMCC, DMAC, SDHC0, CAN0, CAN1) are listed so that enable<> names them
        // all the same; their action is a no-op unless something cleared the bit.
        static constexpr std::array peripheryEnableInfos{
          PeripheryEnableInfo{0x41006000, 0x10,  8}, // CMCC
          PeripheryEnableInfo{0x4100A000, 0x10,  9}, // DMAC
          PeripheryEnableInfo{0x45000000, 0x10, 15}, // SDHC0
          PeripheryEnableInfo{0x42000000, 0x10, 17}, // CAN0
          PeripheryEnableInfo{0x42000400, 0x10, 18}, // CAN1

          PeripheryEnableInfo{0x40000000, 0x14,  0}, // PAC
          PeripheryEnableInfo{0x40000400, 0x14,  1}, // PM
          PeripheryEnableInfo{0x40000800, 0x14,  2}, // MCLK
          PeripheryEnableInfo{0x40000C00, 0x14,  3}, // RSTC
          PeripheryEnableInfo{0x40001000, 0x14,  4}, // OSCCTRL
          PeripheryEnableInfo{0x40001400, 0x14,  5}, // OSC32KCTRL
          PeripheryEnableInfo{0x40001800, 0x14,  6}, // SUPC
          PeripheryEnableInfo{0x40001C00, 0x14,  7}, // GCLK
          PeripheryEnableInfo{0x40002000, 0x14,  8}, // WDT
          PeripheryEnableInfo{0x40002400, 0x14,  9}, // RTC
          PeripheryEnableInfo{0x40002800, 0x14, 10}, // EIC
          PeripheryEnableInfo{0x40002C00, 0x14, 11}, // FREQM
          PeripheryEnableInfo{0x40003000, 0x14, 12}, // SERCOM0
          PeripheryEnableInfo{0x40003400, 0x14, 13}, // SERCOM1
          PeripheryEnableInfo{0x40003800, 0x14, 14}, // TC0
          PeripheryEnableInfo{0x40003C00, 0x14, 15}, // TC1

          PeripheryEnableInfo{0x41000000, 0x18,  0}, // USB
          PeripheryEnableInfo{0x41002000, 0x18,  1}, // DSU
          PeripheryEnableInfo{0x41004000, 0x18,  2}, // NVMCTRL
          PeripheryEnableInfo{0x41008000, 0x18,  4}, // PORT
          PeripheryEnableInfo{0x4100C000, 0x18,  6}, // HMATRIX
          PeripheryEnableInfo{0x4100E000, 0x18,  7}, // EVSYS
          PeripheryEnableInfo{0x41012000, 0x18,  9}, // SERCOM2
          PeripheryEnableInfo{0x41014000, 0x18, 10}, // SERCOM3
          PeripheryEnableInfo{0x41016000, 0x18, 11}, // TCC0
          PeripheryEnableInfo{0x41018000, 0x18, 12}, // TCC1
          PeripheryEnableInfo{0x4101A000, 0x18, 13}, // TC2
          PeripheryEnableInfo{0x4101C000, 0x18, 14}, // TC3
          PeripheryEnableInfo{0x41020000, 0x18, 16}, // RAMECC

          PeripheryEnableInfo{0x42000C00, 0x1C,  3}, // TCC2
          PeripheryEnableInfo{0x42001000, 0x1C,  4}, // TCC3
          PeripheryEnableInfo{0x42001400, 0x1C,  5}, // TC4
          PeripheryEnableInfo{0x42001800, 0x1C,  6}, // TC5
          PeripheryEnableInfo{0x42001C00, 0x1C,  7}, // PDEC
          PeripheryEnableInfo{0x42002000, 0x1C,  8}, // AC
          PeripheryEnableInfo{0x42002400, 0x1C,  9}, // AES
          PeripheryEnableInfo{0x42002800, 0x1C, 10}, // TRNG
          PeripheryEnableInfo{0x42002C00, 0x1C, 11}, // ICM
          PeripheryEnableInfo{0x42003400, 0x1C, 13}, // QSPI
          PeripheryEnableInfo{0x42003800, 0x1C, 14}, // CCL

          PeripheryEnableInfo{0x43000000, 0x20,  0}, // SERCOM4
          PeripheryEnableInfo{0x43000400, 0x20,  1}, // SERCOM5
          PeripheryEnableInfo{0x43001000, 0x20,  4}, // TCC4
          PeripheryEnableInfo{0x43001C00, 0x20,  7}, // ADC0
          PeripheryEnableInfo{0x43002000, 0x20,  8}, // ADC1
          PeripheryEnableInfo{0x43002400, 0x20,  9}, // DAC
          PeripheryEnableInfo{0x43002800, 0x20, 10}, // I2S
          PeripheryEnableInfo{0x43002C00, 0x20, 11}, // PCC
        };

        static constexpr bool isValidPeripheryAddress(unsigned peripheryAddress) {
            for(auto pei : peripheryEnableInfos) {
                if(pei.address == peripheryAddress) { return true; }
            }
            return false;
        }

        static constexpr unsigned getOffset(unsigned peripheryAddress) {
            for(auto pei : peripheryEnableInfos) {
                if(pei.address == peripheryAddress) { return pei.offset; }
            }
            return 0;
        }

        static constexpr unsigned getBit(unsigned peripheryAddress) {
            for(auto pei : peripheryEnableInfos) {
                if(pei.address == peripheryAddress) { return pei.bit; }
            }
            return 0;
        }

    }   // namespace Detail

    template<unsigned PeripheryAddress>
    struct enable {
        static_assert(Detail::isValidPeripheryAddress(PeripheryAddress),
                      "invalid PeripheryAddress to enable");
        using action
          = Detail::BitSet<Detail::getOffset(PeripheryAddress), Detail::getBit(PeripheryAddress)>;
    };

    template<unsigned PeripheryAddress>
    struct disable {
        static_assert(Detail::isValidPeripheryAddress(PeripheryAddress),
                      "invalid PeripheryAddress to disable");
        using action
          = Detail::BitClear<Detail::getOffset(PeripheryAddress), Detail::getBit(PeripheryAddress)>;
    };
}   // namespace MCLK

namespace PM {
    using MCLK::disable;
    using MCLK::enable;
}   // namespace PM

}   // namespace Kvasir
