#pragma once
#include <cstdint>
#include <vector>

// Verifies the 4 raw CRC nibbles (appended by add_crc) against the dewhitened
// payload bytes. Returns true if the CRC matches.
bool verify_crc(const std::vector<uint8_t>& payload,
                const std::vector<uint8_t>& crc_nibbles);
