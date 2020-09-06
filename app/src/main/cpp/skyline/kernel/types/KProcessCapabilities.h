// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Rupansh Sekar <rupanshsekar@hotmail.com>

#pragma once

#include "KSyncObject.h"
#include "KSharedMemory.h"

namespace skyline::kernel::type {
    /**
    * @brief The KProcessCapabilities class is responsible for holding capabilities of a process
    */
    class KProcessCapabilities {
      private:
        static constexpr u32 capType(u32 val) { return ((val + 1) & ~val); };
        static constexpr u64 makeMask(u64 low, u64 high);
      public:
        u8 svcMask[0x10] = {0}; //!< SVC access permission mask
        u8 irqMask[0x80] = {0}; //!< Interrupt validity mask

        u64 coreMask = 0; //!< Allowed Cores Mask
        u64 threadMask = 0; //!< Allowed Thread Mask

        u32 debuggingFlags = 0; //!< Debugging related capabilities
        int32_t handleTableSize = 0; //!< Max allowed handles
        u32 kernelVersion = 0; //!< kernel version
        u32 applicationT = 0; //!< Application type of the process

        /**
        * @brief Initializes a KProcessCapabilities object for KProcess
        * @param state The state of the device
        * @param caps Capabilities from metadata
        * @param stack The KSharedMemory object for Stack memory allocated by the guest process
        */
        void InitializeUserProcess(const DeviceState &state, const std::vector<u32> &caps, std::shared_ptr<KSharedMemory> &stack);
    };
}