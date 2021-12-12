#include <cstdint>

namespace crc {

// Small implementation of std::array, needed until constexpr
// is added to the function 'reference operator[](size_type)'
template <typename T, std::size_t N>
struct array {
    T m_data[N];

    using value_type = T;
    using reference = value_type &;
    using const_reference = const value_type &;
    using size_type = std::size_t;

    // This is NOT constexpr in std::array until C++17
    constexpr reference operator[](size_type i) noexcept {
        return m_data[i];
    }

    constexpr const_reference operator[](size_type i) const noexcept {
        return m_data[i];
    }

    constexpr size_type size() const noexcept {
        return N;
    }
};


// Generates CRC-32 table, algorithm based from this link:
// http://www.hackersdelight.org/hdcodetxt/crc.c.txt
constexpr auto gen_crc32_table() {
    constexpr auto num_bytes = 256;
    constexpr auto num_iterations = 8;
    constexpr auto polynomial = 0xEDB88320;

    auto crc32_table = array<uint32_t, num_bytes>{};

    for (auto byte = 0u; byte < num_bytes; ++byte) {
        auto crc = byte;

        for (auto i = 0; i < num_iterations; ++i) {
            auto mask = -(crc & 1);
            crc = (crc >> 1) ^ (polynomial & mask);
        }

        crc32_table[byte] = crc;
    }

    return crc32_table;
}

// Stores CRC-32 table and softly validates it.
static constexpr auto crc32_table = gen_crc32_table();
static_assert(
    crc32_table.size() == 256 &&
        crc32_table[1] == 0x77073096 &&
        crc32_table[255] == 0x2D02EF8D,
    "gen_crc32_table generated unexpected result."
);

constexpr auto crc32(const void *in, size_t sz) {
    const auto *data = static_cast<const uint8_t*>(in);
    auto crc = 0xFFFFFFFFu;
    for (size_t i = 0; i < sz; ++i) {
        crc = crc32_table[(crc ^ data[i]) & 0xFFu] ^ (crc >> 8);
    }

    return ~crc;
}

}
