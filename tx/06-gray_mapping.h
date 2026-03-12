#pragma once
#include <cstdint>
#include <vector>

// Applies Gray encoding to each LoRa symbol: out = x ^ (x >> 1).
// sf is the spreading factor; symbols must be in [0, 2^sf).
std::vector<uint32_t> gray_map(const std::vector<uint32_t>& symbols, uint8_t sf);
