// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)

#pragma once

#include "KSyncObject.h"
#include "KSharedMemory.h"

namespace skyline::kernel::type {
    /**
     * @brief KThread class is responsible for holding the state of a thread
     */
    class KThread : public KSyncObject {
      private:
        KProcess *parent; //!< The parent process of this thread
        u64 entryPoint; //!< The address to start execution at
        u64 entryArg; //!< An argument to pass to the process on entry

        /**
         * @brief This holds a range of priorities for a corresponding system
         */
        struct Priority {
            i8 low; //!< The low range of priority
            i8 high; //!< The high range of priority

            /**
             * @param priority The priority range of the value
             * @param value The priority value to rescale
             * @return The rescaled priority value according to this range
             */
            constexpr inline i8 Rescale(const Priority &priority, i8 value) {
                return static_cast<i8>(priority.low + ((static_cast<float>(priority.high - priority.low) / static_cast<float>(priority.low - priority.high)) * (static_cast<float>(value) - priority.low)));
            }

            /**
             * @param value The priority value to check for validity
             * @return If the supplied priority value is valid
             */
            constexpr inline bool Valid(i8 value) {
                return (value >= low) && (value <= high);
            }
        };

      public:
        enum class Status {
            Created, //!< The thread has been created but has not been started yet
            Running, //!< The thread is running currently
            Dead //!< The thread is dead and not running
        } status = Status::Created; //!< The state of the thread
        std::atomic<bool> cancelSync{false}; //!< This is to flag to a thread to cancel a synchronization call it currently is in
        std::shared_ptr<type::KSharedMemory> ctxMemory; //!< The KSharedMemory of the shared memory allocated by the guest process TLS
        KHandle handle; // The handle of the object in the handle table
        pid_t tid; //!< The TID of the current thread
        u64 stackTop; //!< The top of the stack (Where it starts growing downwards from)
        u64 tls; //!< The address of TLS (Thread Local Storage) slot assigned to the current thread
        i8 priority; //!< The priority of a thread in Nintendo format
        u32 currentCore;
        i32 prefCoreOverride = -1; //!< Preferred core override
        u64 affMaskOverride = 0x1; //!< Affinity mask override
        u32 affOverrideCnt = 0;
        u32 prefCore{0}; //!< Preferred core
        u64 affMask{0x1}; //!< Affinity mask

        Priority androidPriority{19, -8}; //!< The range of priorities for Android
        Priority switchPriority{0, 63}; //!< The range of priorities for the Nintendo Switch

        /**
         * @param state The state of the device
         * @param handle The handle of the current thread
         * @param selfTid The TID of this thread
         * @param entryPoint The address to start execution at
         * @param entryArg An argument to pass to the process on entry
         * @param stackTop The top of the stack
         * @param tls The address of the TLS slot assigned
         * @param priority The priority of the thread in Nintendo format
         * @param cpuCore The preferred Cpu Core
         * @param parent The parent process of this thread
         * @param tlsMemory The KSharedMemory object for TLS memory allocated by the guest process
         */
        KThread(const DeviceState &state, KHandle handle, pid_t selfTid, u64 entryPoint, u64 entryArg, u64 stackTop, u64 tls, i8 priority, u32 cpuCore, KProcess *parent, const std::shared_ptr<type::KSharedMemory> &tlsMemory);

        /**
         * @brief Kills the thread and deallocates the memory allocated for stack.
         */
        ~KThread();

        /**
         * @brief This starts this thread process
         */
        void Start();

        /**
         * @brief This kills the thread
         */
        void Kill();

        /**
         * @brief Update the priority level for the process.
         * @details Set the priority of the current thread to `priority` using setpriority [https://linux.die.net/man/3/setpriority]. We rescale the priority from Nintendo scale to that of Android.
         * @param priority The priority of the thread in Nintendo format
         */
        void UpdatePriority(i8 priority);

        u32 UpdatePreferredCoreAndAffinityMask(u32 newPrefCore, u64 newAffMask);
    };
}
