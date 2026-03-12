#pragma once
#include <cstdint>
#include <vector>

// Deinterleaves LoRa symbols back into Hamming codewords.
// First block (header): takes 8 symbols, produces sf-2 codewords.
// Payload blocks:       takes cr+4 symbols, produces sf (or sf-2 if ldro) codewords.
// Each input symbol is a uint32_t with sf_app data bits (MSB-indexed).
std::vector<uint8_t> deinterleave(const std::vector<uint32_t>& symbols,
                                   uint8_t cr, uint8_t sf, bool ldro = false);
