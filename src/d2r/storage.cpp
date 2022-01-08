/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#include "storage.h"

#include <CascLib.h>
#include <windows.h>

namespace d2r {

Storage storage;

struct StorageCtx {
    HANDLE storage = nullptr;
};

d2r::Storage::Storage() noexcept: ctx_(new(std::nothrow) StorageCtx) {
}

d2r::Storage::~Storage() {
    if (ctx_->storage) {
        CascCloseStorage(ctx_->storage);
    }
    delete ctx_;
}

bool d2r::Storage::init() {
    HKEY key;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE"
#if defined(_M_AMD64)
                                          "\\WOW6432Node"
#endif
                                          "\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Diablo II Resurrected", 0, KEY_READ, &key) != ERROR_SUCCESS) { return false; }
    wchar_t regpath[MAX_PATH];
    DWORD pathSize = sizeof(regpath);
    bool result = RegQueryValueExW(key, L"InstallSource", nullptr, nullptr, LPBYTE(regpath), &pathSize) == ERROR_SUCCESS;
    RegCloseKey(key);
    if (!result) { return false; }
    /* CascOpenStorage may fail in rare case, just retry for 3 times */
    int counter = 3;
    while (!CascOpenStorage(regpath, CASC_LOCALE_ALL, &ctx_->storage) && --counter) {
        if (GetCascError() == ERROR_NOT_SUPPORTED || GetCascError() == ERROR_INVALID_PARAMETER) {
            return false;
        }
        Sleep(500);
    }
    return result;
}

bool Storage::readFile(const char *filename, std::vector<uint8_t> &data) {
    if (!ctx_->storage) { return false; }
    HANDLE file;
    if (!CascOpenFile(ctx_->storage, filename, CASC_LOCALE_ALL, CASC_OPEN_BY_NAME, &file)) {
        return false;
    }
    auto size = CascGetFileSize(file, nullptr);
    data.resize(size);
    auto result = CascReadFile(file, data.data(), size, nullptr);
    CascCloseFile(file);
    return result;
}

}
