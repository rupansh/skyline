// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)

#include <sys/resource.h>
#include <nce.h>
#include "KThread.h"
#include "KProcess.h"

namespace skyline::kernel::type {
    KThread::KThread(const DeviceState &state, KHandle handle, pid_t selfTid, u64 entryPoint, u64 entryArg, u64 stackTop, u64 tls, i8 priority, u32 cpuCore, KProcess *parent, const std::shared_ptr<type::KSharedMemory> &tlsMemory) : handle(handle), tid(selfTid), entryPoint(entryPoint), entryArg(entryArg), stackTop(stackTop), tls(tls), priority(priority), parent(parent), ctxMemory(tlsMemory), KSyncObject(state,
        KType::KThread) {
        currentCore = cpuCore;
        affMask = 1 << currentCore;
        UpdatePriority(priority);
    }

    KThread::~KThread() {
        Kill();
    }

    void KThread::Start() {
        if (status == Status::Created) {
            if (tid == parent->pid)
                parent->status = KProcess::Status::Started;
            status = Status::Running;

            state.nce->StartThread(entryArg, handle, parent->threads.at(tid));
        }
    }

    void KThread::Kill() {
        if (status != Status::Dead) {
            status = Status::Dead;
            Signal();

            tgkill(parent->pid, tid, SIGTERM);
        }
    }

    void KThread::UpdatePriority(i8 priority) {
        this->priority = priority;
        auto priorityValue = androidPriority.Rescale(switchPriority, priority);

        if (setpriority(PRIO_PROCESS, static_cast<id_t>(tid), priorityValue) == -1)
            throw exception("Couldn't set process priority to {} for PID: {}", priorityValue, tid);
    }

    u32 KThread::UpdatePreferredCoreAndAffinityMask(u32 newPrefCore, u64 newAffMask) {
        bool override = affOverrideCnt != 0;

        if (static_cast<i32>(newPrefCore) == -3) {
            newPrefCore = override ? prefCoreOverride : prefCore;
            if (newAffMask & (1 << newPrefCore))
                return constant::status::InvCombination;
        }

        if (override) {
            prefCoreOverride = newPrefCore;
            affMaskOverride = newAffMask;
        } else {
            u64 oldAffMask = affMask;
            prefCore = newPrefCore;
            affMask = newAffMask;

            if (oldAffMask != newAffMask) {
                //u32 oldCore = currentCore;

                if (currentCore >= 0 && !((affMask >> currentCore) & 1)) {
                    if (prefCore < 0) {
                        for (u32 core = 3; core >= 0; core--) {
                            if ((affMask >> core) & 1) {
                                currentCore = core;
                                break;
                            }
                        }
                    } else {
                        currentCore = prefCore;
                    }
                }

                // TODO: Adjust Scheduler
            }
        }

        return constant::status::Success;
    }
}
