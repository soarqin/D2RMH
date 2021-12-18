#pragma once

#include <cstdint>

namespace d2mapapi {

enum D2Version {
    D2_111a,
    D2_111b,
    D2_112a,
    D2_113c,
    D2_113d,
};
D2Version getD2Version();
bool defineOffsets();

}
