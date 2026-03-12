#pragma once
#include <complex>
#include <cstdint>
#include <vector>

// Modulates gray-demapped LoRa symbols into a baseband IQ sample stream.
// Produces: preamble_len upchirps, 2 sync-word chirps, 2.25 downchirps,
//           then one upchirp per payload symbol.
// Each chirp is 2^sf samples (os_factor = 1, samp_rate = bw).
// sync_word: single byte, e.g. 0x12 (LoRaWAN public network).
std::vector<std::complex<float>> modulate(const std::vector<uint32_t>& symbols,
                                          uint8_t sf,
                                          uint8_t sync_word = 0x12,
                                          uint16_t preamble_len = 8);
