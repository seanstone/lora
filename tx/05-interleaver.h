#pragma once
#include <cstdint>
#include <vector>

// Interleaves Hamming codewords into LoRa symbols.
// First block (header): sf_app = sf-2, cw_len = 8  (cr_app always 4).
// Payload blocks:       sf_app = ldro ? sf-2 : sf,  cw_len = cr+4.
// Each output symbol is a uint32_t with sf_app data bits (bit j = LSB-indexed).
std::vector<uint32_t> interleave(const std::vector<uint8_t>& codewords,
                                  uint8_t cr, uint8_t sf, bool ldro = false);
