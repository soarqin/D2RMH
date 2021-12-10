#include "pipehost.h"

#include <windows.h>

int main(int argc, char *argv[]) {
    d2mapapi::PipedChildProcess pcp;
    if (!pcp.start(L"d2mapapi_piped.exe", nullptr)) {
        MessageBoxA(nullptr, pcp.errMsg().c_str(), nullptr, 0);
        return -1;
    }
    for (int i = 1; i < argc; i += 3) {
        auto *map = pcp.queryMap(strtoul(argv[i], nullptr, 0),
                     strtoul(argv[i + 1], nullptr, 0),
                     strtoul(argv[i + 2], nullptr, 0));
        if (!map->built) {
            MessageBoxA(nullptr, map->errorString.c_str(), nullptr, 0);
        }
        delete map;
    }
    return 0;
}
