#pragma once
#include <complex>
#include <cstdint>
#include <vector>

// Demodulate LoRa symbols from a flat IQ sample vector.
// samples: consecutive N = 2^sf sample windows (output of frame_sync).
// Returns one bin index per symbol window (no -1 shift; matches standalone
// gray_demap which has no +1 offset).
std::vector<uint32_t> fft_demod(const std::vector<std::complex<float>>& samples,
                                 uint8_t sf);
