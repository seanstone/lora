#pragma once
#include <cstdint>
#include <vector>

// Computes CRC16 on payload_bytes and appends 4 raw (unwhitened) CRC nibbles
// to the end of framed_nibbles. No-op if has_crc is false.
// Call order: whiten → add_header → add_crc
std::vector<uint8_t> add_crc(const std::vector<uint8_t>& framed_nibbles,
                              const std::vector<uint8_t>& payload,
                              bool has_crc);
