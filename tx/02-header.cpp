#include "02-header.h"

// Prepends a 5-nibble explicit header to whitened nibbles:
//   [0]   high nibble of payload length
//   [1]   low nibble of payload length
//   [2]   (cr << 1) | has_crc
//   [3]   checksum bit c4
//   [4]   checksum nibble c3..c0
// If impl_head is true the header is omitted and nibbles are returned as-is.
std::vector<uint8_t> add_header(const std::vector<uint8_t>& nibbles,
                                bool impl_head, bool has_crc, uint8_t cr)
{
    if (impl_head)
        return nibbles;

    uint8_t payload_len = nibbles.size() / 2;

    uint8_t h0 = payload_len >> 4;
    uint8_t h1 = payload_len & 0x0F;
    uint8_t h2 = (cr << 1) | has_crc;

    bool c4 = ((h0 >> 3) & 1) ^ ((h0 >> 2) & 1) ^ ((h0 >> 1) & 1) ^ (h0 & 1);
    bool c3 = ((h0 >> 3) & 1) ^ ((h1 >> 3) & 1) ^ ((h1 >> 2) & 1) ^ ((h1 >> 1) & 1) ^ (h2 & 1);
    bool c2 = ((h0 >> 2) & 1) ^ ((h1 >> 3) & 1) ^ (h1 & 1) ^ ((h2 >> 3) & 1) ^ ((h2 >> 1) & 1);
    bool c1 = ((h0 >> 1) & 1) ^ ((h1 >> 2) & 1) ^ (h1 & 1) ^ ((h2 >> 2) & 1) ^ ((h2 >> 1) & 1) ^ (h2 & 1);
    bool c0 = (h0 & 1) ^ ((h1 >> 1) & 1) ^ ((h2 >> 3) & 1) ^ ((h2 >> 2) & 1) ^ ((h2 >> 1) & 1) ^ (h2 & 1);

    std::vector<uint8_t> out;
    out.reserve(5 + nibbles.size());
    out.push_back(h0);
    out.push_back(h1);
    out.push_back(h2);
    out.push_back(c4);
    out.push_back((c3 << 3) | (c2 << 2) | (c1 << 1) | c0);
    out.insert(out.end(), nibbles.begin(), nibbles.end());
    return out;
}
