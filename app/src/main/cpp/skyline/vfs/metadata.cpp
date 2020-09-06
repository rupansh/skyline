// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Rupansh Sekar <rupanshsekar@hotmail.com>


#include "metadata.h"
#include "backing.h"

namespace skyline::vfs {
    void MetaData::Load(std::shared_ptr<Backing> backing) {
        if (backing->size < sizeof(NpdmHeader))
            throw exception("Bad NPDM File");

        backing->Read(&npdmHeader);
        backing->Read(&acidHeader, npdmHeader.acidOff);
        backing->Read(&aciHeader, npdmHeader.aciOff);

        capabilities.resize(aciHeader.kcSz/sizeof(u32));
        backing->Read(capabilities.data(), npdmHeader.aciOff + aciHeader.kcOff, aciHeader.kcSz);
    }

    MetaData MetaData::Homebrew() {
        MetaData res = MetaData();

        *res.threadPrio = 0x2c;
        *res.threadCore = 0;
        *res.threadStackSz = 0x00100000;
        *res.systemResourceSize = 0xF8000000; // 4 gigs
        res.capabilities = {};

        return res;
    }
}