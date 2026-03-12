#pragma once
#include <cstdint>
#include <vector>

struct FrameInfo {
    uint8_t payload_len;
    uint8_t cr;
    bool    has_crc;
    bool    valid;
    std::vector<uint8_t> payload_nibbles;
};

FrameInfo decode_header(const std::vector<uint8_t>& frame,
                        bool impl_head,
                        uint8_t cr = 0, uint32_t pay_len = 0, bool has_crc = false);
