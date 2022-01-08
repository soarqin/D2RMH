#pragma once

#include "font.h"

#include <vector>
#include <array>
#include <string>
#include <fstream>
#include <cstdint>

namespace render {

#pragma pack(push, 1)

struct TblHeader {
    uint32_t sign;      // +00 - 0x216f6f57 (Woo!)
    int16_t unk0;       // +04 - 0x0001
    int16_t unk1;       // +06 - mostly 0x0000
    uint16_t numChars;  // +08
    uint8_t lnSpacing; // +0A
    uint8_t capHeight; // +0B
};

struct TblCharacter {
    uint16_t code;           // +00
    int8_t unk0;            // +02 - mostly 0x00
    uint8_t width;          // +03
    uint8_t height;         // +04
    int8_t unk1;            // +05 - mostly 1, seldomly 0
    int16_t unk2;            // +06 - mostly 0x0000
    uint16_t dc6Index;       // +08
    int32_t unk3;            // +0A
};

struct DC6Header {
    uint32_t version;   // +00 - 0x00000006
    uint32_t unk0;      // +04 - 0x00000001
    uint32_t unk1;      // +08 - 0x00000000
    uint32_t term;      // +0c - 0xeeeeeeee or 0xcdcdcdcd
    uint32_t numDir;    // +10 - #Direction
    uint32_t numFrame;  // +14 - #Frame per Direction
};

struct DC6FrameHeader {
    uint32_t flip;      // +00 - 1 if Flipped, 0 else
    uint32_t width;     // +04
    uint32_t height;    // +08
    uint32_t offsetX;   // +0c
    uint32_t offsetY;   // +10
    uint32_t unk0;      // +14 - 0x00000000
    uint32_t nextBlock; // +18
    uint32_t length;    // +1c
};

#pragma pack(pop)

class D2Font: public Font {
public:
    using Font::Font;
    bool add(const std::string &filename, int param) override;

protected:
    bool makeCache(FontData *fd, uint32_t ch, int fontSize) override;
    uint64_t makeFontKey(uint32_t ch, int fontSize) override {
        return ch;
    }

private:
    bool load(const std::string &dc6file, const std::string &tblfile, const std::string &palfile);
    bool loadMem(const void *dc6, size_t dc6Size, const void *tbl, size_t tblSize, const void *pal, size_t palSize);
    bool read(uint32_t index, DC6FrameHeader &hdr, std::vector<uint32_t> &data);

private:
    std::array<uint32_t, 256> palette_ = {};

    TblHeader tblHeader_ = {};
    std::array<TblCharacter, 65536> tblCharacters_ = {};

    std::vector<uint8_t> dc6Data_;
    const DC6Header *dc6Header_ = {};
    const uint32_t *dc6OffsetTable_;

    int originFontSize_ = 0;
};

}
