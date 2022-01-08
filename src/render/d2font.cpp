#include "d2font.h"

#include "cfg.h"
#include "d2r/storage.h"

#include <iostream>
#include <cstring>

namespace render {

bool D2Font::add(const std::string &filename, int param) {
    originFontSize_ = param;
    if (filename.empty()) {
        std::vector<uint8_t> dc6, tbl, pal;
        char path[256], dc6file[256], tblfile[256];
        const char *lngDir = "latin2";
        if (cfg->language == "enUS") {
            lngDir = "latin";
        } else if (cfg->language == "jaJP" || cfg->language == "zhTW" || cfg->language == "zhCN") {
            lngDir = "chi";
        } else if (cfg->language == "koKR") {
            lngDir = "kor";
        }
        snprintf(path, 256, "data:data/local/font/%s/font%2d", lngDir, param);
        snprintf(dc6file, 256, "%s.dc6", path);
        snprintf(tblfile, 256, "%s.tbl", path);
        d2r::storage.readFile(dc6file, dc6);
        d2r::storage.readFile(tblfile, tbl);
        d2r::storage.readFile("data:data/global/palette/fechar/pal.dat", pal);
        return loadMem(dc6.data(), dc6.size(), tbl.data(), tbl.size(), pal.data(), pal.size());
    }
    return load(filename + ".dc6", filename + ".tbl", filename + ".pal");
}

bool D2Font::makeCache(Font::FontData *fd, uint32_t ch, int fontSize) {
    if (ch > 65535) {
        return false;
    }
    auto &tbl = tblCharacters_[ch];
    DC6FrameHeader hdr;
    std::vector<uint32_t> data;
    read(tbl.dc6Index, hdr, data);
    fd->w = hdr.width;
    fd->h = hdr.height;
    fd->ix0 = 0;
    fd->iy0 = tblHeader_.capHeight;
    fd->advW = tbl.width;
    fd->origW = uint8_t(uint32_t(originFontSize_));

    if (!updateTexture(fd->rpidx, fd->rpx, fd->rpy, fd->w, fd->h, (const uint8_t*)data.data())) {
        memset(fd, 0, sizeof(FontData));
        return false;
    }
    return true;
}

bool D2Font::load(const std::string &dc6file, const std::string &tblfile, const std::string &palfile) {
    std::vector<uint8_t> dc6, tbl;
    uint8_t pal[768];

    if (std::ifstream ifs(dc6file, std::ios::in | std::ios::binary); ifs.is_open()) {
        ifs.seekg(0, std::ios::end);
        auto size = ifs.tellg();
        ifs.seekg(0, std::ios::beg);
        dc6.resize(size);
        ifs.read((char *)dc6.data(), size);
        ifs.close();
    } else {
        return false;
    }
    if (std::ifstream ifs(tblfile, std::ios::in | std::ios::binary); ifs.is_open()) {
        ifs.seekg(0, std::ios::end);
        auto size = ifs.tellg();
        ifs.seekg(0, std::ios::beg);
        tbl.resize(size);
        ifs.read((char *)tbl.data(), size);
        ifs.close();
    } else {
        return false;
    }
    if (std::ifstream ifs(palfile, std::ios::in | std::ios::binary); ifs.is_open()) {
        ifs.seekg(0, std::ios::end);
        auto size = ifs.tellg();
        if (size < 768) {
            ifs.close();
            return false;
        }
        ifs.seekg(0, std::ios::beg);
        ifs.read((char *)pal, 768);
        ifs.close();
    } else {
        return false;
    }

    return loadMem(dc6.data(), dc6.size(), tbl.data(), tbl.size(), pal, 768);
}

bool D2Font::loadMem(const void *dc6, size_t dc6Size, const void *tbl, size_t tblSize, const void *pal, size_t palSize) {
    if (palSize < 768) {
        return false;
    }
    if (tblSize < sizeof(TblHeader)) {
        return false;
    }
    if (dc6Size < sizeof(DC6Header)) {
        return false;
    }

    const auto *palData = (const uint8_t *)pal;
    for (int i = 0; i < 256; i++) {
        auto idx = i * 3;
        palette_[i] = 0xFF000000u | (uint32_t(palData[idx]) << 16) | (uint32_t(palData[idx + 1]) << 8) | uint32_t(palData[idx + 2]);
    }

    memcpy(&tblHeader_, tbl, sizeof(TblHeader));
    const auto *tblData = (const uint8_t*)tbl + sizeof(TblHeader);
    tblSize -= sizeof(TblHeader);
    auto tblCharSize = sizeof(TblCharacter) * tblHeader_.numChars;
    if (tblCharSize > tblSize) {
        return false;
    }
    const auto *chars = (const TblCharacter*)tblData;
    for (uint16_t i = 0; i < tblHeader_.numChars; ++i) {
        tblCharacters_[chars[i].code] = chars[i];
    }

    dc6Data_.assign((const uint8_t*)dc6, (const uint8_t*)dc6 + dc6Size);
    dc6Header_ = (const DC6Header*)dc6Data_.data();
    auto count = dc6Header_->numDir * dc6Header_->numFrame;
    const auto *dc6Data = (const uint8_t*)dc6Data_.data() + sizeof(DC6Header);
    dc6Size -= sizeof(DC6Header);
    auto dc6OffsetTableSize = sizeof(uint32_t) * count;
    if (dc6Size < dc6OffsetTableSize) {
        return false;
    }
    dc6OffsetTable_ = (const uint32_t*)dc6Data;
    return true;
}

bool D2Font::read(uint32_t index, DC6FrameHeader &hdr, std::vector<uint32_t> &data) {
    const auto *ptr = dc6Data_.data() + dc6OffsetTable_[index];
    memcpy(&hdr, ptr, sizeof(DC6FrameHeader));
    ptr += sizeof(DC6FrameHeader);
    size_t y = hdr.height - 1;
    size_t x = 0;
    data.resize(hdr.height * hdr.width);
    auto *end = ptr + hdr.length;
    while (ptr < end) {
        auto b = *ptr++;
        if (b & 0x80) {
            if (b == 0x80) {
                x = 0;
                if (--y < 0) {
                    break;
                }
            } else {
                x += b & 0x7f;
            }
        } else {
            for (; b && ptr < end; b--) {
                if (x >= hdr.width) {
                    ptr += b;
                    break;
                }
                auto c = *ptr++;
                data[y * hdr.width + x] = palette_[c];
                x++;
            }
        }
    }
    return true;
}

}
