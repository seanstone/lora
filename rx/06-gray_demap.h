#pragma once
#include <cstdint>
#include <vector>

// Applies Gray decoding + cyclic shift +1 to each symbol.
// Inverse Gray decode: result = XOR-fold of y (y ^ y>>1 ^ y>>2 ^ ... ^ y>>(sf-1)).
// Then result = (result + 1) mod 2^sf.
// This is the exact inverse of gray_map when combined with the modulator's -1 shift.
std::vector<uint32_t> gray_demap(const std::vector<uint32_t>& symbols, uint8_t sf);
