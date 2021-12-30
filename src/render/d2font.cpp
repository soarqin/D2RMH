#include "d2font.h"

#include <iostream>
#include <cstring>

namespace render {

bool D2Font::add(const std::string &filename, int param) {
    originFontSize_ = param;
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
    if (std::ifstream ifs0(palfile, std::ios::in | std::ios::binary); ifs0.is_open()) {
        uint8_t n[756];
        ifs0.read((char *)n, 756);
        ifs0.close();
        for (int i = 0; i < 256; i++) {
            auto idx = i * 3;
            palette_[i] = 0xFF000000u | (uint32_t(n[idx]) << 16) | (uint32_t(n[idx + 1]) << 8) | uint32_t(n[idx + 2]);
        }
    } else {
        for (uint32_t i = 0; i < 256; i++) {
            palette_[i] = i << 24 | 0xFFFFFFu;
        }
    }

    if (std::ifstream ifs1(tblfile, std::ios::in | std::ios::binary); ifs1.is_open()) {
        ifs1.read((char *)&tblHeader_, sizeof(TblHeader));
        std::vector<TblCharacter> tblCharacters;
        tblCharacters.resize(tblHeader_.numChars);
        ifs1.read((char *)tblCharacters.data(), sizeof(TblCharacter) * tblHeader_.numChars);
        ifs1.close();
        for (auto &ch: tblCharacters) {
            tblCharacters_[ch.code] = ch;
        }
    } else {
        return false;
    }

    dc6fs.open(dc6file, std::ios::in | std::ios::binary);
    if (dc6fs.is_open()) {
        dc6fs.read((char *)&dc6Header_, sizeof(DC6Header));
        auto count = dc6Header_.numDir * dc6Header_.numFrame;
        dc6OffsetTable.resize(count);
        dc6fs.read((char *)dc6OffsetTable.data(), sizeof(uint32_t) * count);
    } else {
        return false;
    }
    return true;
}

bool D2Font::read(uint32_t index, DC6FrameHeader &hdr, std::vector<uint32_t> &data) {
    dc6fs.seekg(dc6OffsetTable[index], std::ios::beg);
    dc6fs.read((char *)&hdr, sizeof(DC6FrameHeader));
    if (!dc6fs.good()) {
        return false;
    }
    size_t y = hdr.height - 1;
    size_t x = 0;
    std::vector<uint8_t> frame(hdr.length);
    dc6fs.read((char *)frame.data(), hdr.length);
    if (!dc6fs.good()) {
        return false;
    }
    data.resize(hdr.height * hdr.width);
    auto *ptr = frame.data();
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
