// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Rupansh Sekar <rupanshsekar@hotmail.com>

#pragma once

#include <nce/guest_common.h>
#include <vector>
#include "backing.h"

namespace skyline::vfs {
    /**
     * @brief Nintendo Program Descriptor Metadata (https://switchbrew.org/wiki/NPDM)
     */
    class MetaData {
      private:
        /**
         * @brief This is the header of a NPDM File "META" (https://switchbrew.org/wiki/NPDM#META)
         */
        struct NpdmHeader {
            char magic[4];
            u8 _pad0[8];
            u8 _pad1[2];
            u8 threadPrio;
            u8 threadCore;
            u8 _pad2[4];
            u32 systemResourceSize;
            u32 processCategory;
            u32 threadStackSz;
            u8 appName[0x10];
            u8 _pad3[0x40];
            u32 aciOff;
            u32 aciSz;
            u32 acidOff;
            u32 acidSz;
        } npdmHeader{};

        static_assert(sizeof(NpdmHeader) == 0x80);

        /**
         * @brief ACID Header (https://switchbrew.org/wiki/NPDM#ACID)
         */
        struct AcidHeader {
            u8 sig[0x100];
            u8 ncaMods[0x100];
            char magic[4];
            u32 ncaSz;
            u8 _pad0[0x4];
            u32 flags;
            u64 titleIdL;
            u64 titleIdH;
            u8 _pad1[0x20]; // Fah, sac, kc, 8b pad
        } acidHeader{};

        static_assert(sizeof(AcidHeader) == 0x240);

        /**
         * @brief ACI0 Header (https://switchbrew.org/wiki/NPDM#ACI0)
         */
        struct AciHeader {
            char magic[4];
            u8 _pad0[0xC];
            u64 titleId;
            u8 _pad1[0x18]; // 8b pad, fah, sac
            u32 kcOff;
            u32 kcSz;
            u8 _pad2[0x8];
        } aciHeader{};

        static_assert(sizeof(AciHeader) == 0x40);

      public:
        // TODO: Store other information
        u8 *threadPrio = &npdmHeader.threadPrio; //!< Pointer to MainThreadPriority in META
        u8 *threadCore = &npdmHeader.threadCore; //!< Pointer to MainThreadCoreNumber in META
        u32 *threadStackSz = &npdmHeader.threadStackSz; //!< Pointer to MainThreadStackSize in META
        u64 *titleId = &aciHeader.titleId; //!< Pointer to ProgramId in ACI0
        u32 *systemResourceSize = &npdmHeader.systemResourceSize; //!< Pointer to SystemResourceSize in META
        std::vector<u32> capabilities; //!< KernelCapability in ACI0

        /**
         * @brief Parses main.npdm to usable MetaData
         * @param backing main.npdm data
         */
        void Load(std::shared_ptr<Backing> backing);

        /**
         * @brief Dummy MetaData for homebrew ROMs(ex. NRO)
         * @return Dummy MetaData
         */
        static MetaData Homebrew();
    };
}
