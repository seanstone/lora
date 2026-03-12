#include "03-crc_verif.h"

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

static uint16_t compute_crc(const std::vector<uint8_t>& data) {
    size_t n = data.size();
    uint16_t crc = 0;
    for (size_t i = 0; i < n - 2; i++)
        crc = crc16_byte(crc, data[i]);
    crc ^= data[n - 1] ^ ((uint16_t)data[n - 2] << 8);
    return crc;
}

bool verify_crc(const std::vector<uint8_t>& payload,
                const std::vector<uint8_t>& crc_nibbles) {
    uint16_t received = (uint16_t)crc_nibbles[0]
                      | ((uint16_t)crc_nibbles[1] <<  4)
                      | ((uint16_t)crc_nibbles[2] <<  8)
                      | ((uint16_t)crc_nibbles[3] << 12);
    return compute_crc(payload) == received;
}
