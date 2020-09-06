// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Rupansh Sekar <rupanshsekar@hotmail.com>

#include "KProcessCapabilities.h"

namespace skyline::kernel::type {
    void KProcessCapabilities::InitializeUserProcess(const DeviceState &state, const std::vector<u32> &caps, std::shared_ptr<KSharedMemory> &stack) {
        u32 flagMask = 0;
        u32 svcBMask = 0;

        for (std::vector<u32>::iterator cap; cap < caps.end(); cap++) {
            u32 code = capType(*cap);
            if (code == 0x40) {
                if (cap + 1 >= caps.end())
                    throw exception("KProcessCapabilities: Invalid Combo");

                u32 prevCap = *cap;
                cap = cap + 1;

                if (capType(*cap) != 0x40 || *cap & 0x78000000 || !(*cap & 0x7ffff80))
                    throw exception("KProcessCapabilities: Invalid Capability");

                u64 addr = ((u64) (u32) prevCap << 5) & 0xffffff000;
                u64 sz = ((u64) (u32) *cap << 5) & 0xfffff000;

                if ((addr + sz - 1) >> 36)
                    throw exception("KProcessCapabilities: Invalid Address");

                // TODO: Map Memory
            } else {
                if (code == 1)
                    throw exception("KProcessCapabilities: Invalid Capability");
                else if (code == 0) // Ignorable
                    return;

                u32 codeMask = 1u << (32 - (u32)__builtin_clz(code+1));
                if ((flagMask & codeMask) & 0x1e008)
                    throw exception("KProcessCapabilities: Invalid Combo");

                flagMask |= codeMask;

                switch (code) {
                    case 0x8: {
                        if (coreMask || threadMask)
                            throw exception("KProcessCapabilities: Invalid Capability");

                        u32 lCore = (*cap >> 16) & 0xff;
                        u32 hCore = (*cap >> 24) & 0xff;
                        if (lCore > hCore)
                            throw exception("KProcessCapabilities: Invalid Combo");

                        u32 lThread = (*cap >> 10) & 0x3f;
                        u32 hThread = (*cap >> 4) & 0x3f;
                        if (lThread > hThread || hCore >= 4)
                            throw exception("KProcessCapabilities: Invalid Cpu Core Count");

                        coreMask = makeMask(lCore, hCore);
                        threadMask = makeMask(lThread, hThread);

                        break;
                    }
                    case 0x10: {
                        u32 slot = (*cap >> 29) & 7;
                        u32 svcSlotMsk = 1 << slot;
                        if (svcBMask & svcSlotMsk)
                            throw exception("KProcessCapabilities: Invalid Combo");
                        svcBMask |= svcSlotMsk;

                        u32 svcMsk = (*cap >> 5) & 0xffffff;
                        u32 baseSvc = slot*24;

                        for (u32 i = 0; i < 24; i++) {
                            if (!((svcMsk >> i) & 1))
                                continue;

                            u32 svcId = baseSvc + i;
                            if (svcId > 0x7f)
                                throw exception("KProcessCapabilities: SvcId Limit Exceeded");

                            svcMask[svcId/8] |= (u8)(1 << (svcId & 7));
                        }

                        break;
                    }
                    case 0x80:
                        // u64 addr = ((u64)(u32)*cap << 4) & 0xffffff000;
                        // TODO: Map IO Memory
                        break;
                    case 0x800: {
                        u32 int0 = (*cap >> 12) & 0x3ff;
                        u32 int1 = (*cap >> 22) & 0x3ff;

                        if (int0 != 0x3ff)
                            irqMask[int0/8] |= (u8)(1 << (int0 & 0x7));
                        if (int1 != 0x3ff)
                            irqMask[int1/8] |= (u8)(1 << (int1 & 0x7));

                        break;
                    }
                    case 0x2000: {
                        u32 appT = *cap >> 17;
                        if (appT)
                            throw exception("KProcessCapabilities: Invalid App Type");

                        applicationT = (*cap >> 14) & 0b111;

                        break;
                    }
                    case 0x4000: {
                        if (kernelVersion >> 19 || *cap < 0x80000)
                            throw exception("KProcessCapabilities: Invalid Kernel Version");

                        kernelVersion = *cap;

                        break;
                    }
                    case 0x8000: {
                        u32 reserved = *cap >> 26;
                        if (reserved)
                            throw exception("KProcessCapabilities: Invalid Handle Table Sz");

                        handleTableSize = static_cast<int32_t >((*cap >> 16) & 0x3FF);

                        break;
                    }
                    case 0x10000: {
                        u32 debugF = *cap >> 19;
                        if (debugF)
                            throw exception("KProcessCapabilities: Invalid Debugging Flags");

                        debuggingFlags &= ~3;
                        debuggingFlags |= debugF;

                        break;
                    }
                    default: throw exception("KProcessCapabilities: Invalid Capability");
                }
            }
        }
    }

    u64 constexpr KProcessCapabilities::makeMask(u64 low, u64 high) {
        const u64 r = high - low + 1;
        const u64 msk = (1 << r) - 1;

        return msk << low;
    }
}