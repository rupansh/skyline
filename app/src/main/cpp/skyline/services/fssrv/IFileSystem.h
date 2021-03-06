// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)

#pragma once

#include <vfs/filesystem.h>
#include <services/base_service.h>
#include <services/serviceman.h>

namespace skyline::service::fssrv {
    /**
     * @brief IFileSystem is used to interact with a filesystem (https://switchbrew.org/wiki/Filesystem_services#IFileSystem)
     */
    class IFileSystem : public BaseService {
      private:
        std::shared_ptr<vfs::FileSystem> backing;

      public:
        IFileSystem(std::shared_ptr<vfs::FileSystem> backing, const DeviceState &state, ServiceManager &manager);

        /**
         * @brief This creates a file at the specified path in the filesystem
         */
        void CreateFile(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

        /**
         * @brief This queries the DirectoryEntryType of the given path (https://switchbrew.org/wiki/Filesystem_services#GetEntryType)
         */
        void GetEntryType(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

        /**
         * @brief This returns an IFile handle for the requested path (https://switchbrew.org/wiki/Filesystem_services#OpenFile)
         */
        void OpenFile(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

        /**
         * @brief This commits all changes to the filesystem (https://switchbrew.org/wiki/Filesystem_services#Commit)
         */
        void Commit(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);
    };
}
