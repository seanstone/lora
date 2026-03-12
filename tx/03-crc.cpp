#include "03-crc.h"

static uint16_t crc16_byte(uint16_t crc, uint8_t byte) {
    for (int i = 0; i < 8; i++) {
        if (((crc & 0x8000) >> 8) ^ (byte & 0x80))
            crc = (crc << 1) ^ 0x1021;
        else
            crc = (crc << 1);
        byte <<= 1;
    }
    return crc;
}

// CRC-CCITT (Poly=0x1021, Init=0x0000) on bytes[0..N-3],
// then XOR with bytes[N-1] ^ (bytes[N-2] << 8).
static uint16_t compute_crc(const std::vector<uint8_t>& data) {
    size_t n = data.size();
    uint16_t crc = 0;
    for (size_t i = 0; i < n - 2; i++)
        crc = crc16_byte(crc, data[i]);
    crc ^= data[n - 1] ^ ((uint16_t)data[n - 2] << 8);
    return crc;
}

std::vector<uint8_t> add_crc(const std::vector<uint8_t>& framed_nibbles,
                              const std::vector<uint8_t>& payload,
                              bool has_crc) {
    if (!has_crc)
        return framed_nibbles;

    uint16_t crc = compute_crc(payload);
    auto out = framed_nibbles;
    // Append as 4 raw nibbles, little-endian
    out.push_back( crc        & 0x0F);
    out.push_back((crc >>  4) & 0x0F);
    out.push_back((crc >>  8) & 0x0F);
    out.push_back((crc >> 12) & 0x0F);
    return out;
}
