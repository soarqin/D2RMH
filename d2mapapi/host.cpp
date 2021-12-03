#include "pipehost.h"

int main(int argc, char *argv[]) {
    d2mapapi::PipedChildProcess pcp;
    if (!pcp.start(L"d2mapapi_piped.exe", nullptr)) {
        return -1;
    }
    for (int i = 1; i < argc; i += 3) {
        auto *map = pcp.queryMap(strtoul(argv[i], nullptr, 0),
                     strtoul(argv[i + 1], nullptr, 0),
                     strtoul(argv[i + 2], nullptr, 0));
        delete map;
    }
    return 0;
}
