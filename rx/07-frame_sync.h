#pragma once
#include <complex>
#include <cstdint>
#include <vector>

// Detects a LoRa frame in a baseband IQ sample stream and extracts the
// payload symbol windows (2^sf samples each) starting from the first header symbol.
//
// Detection: scans N-sample windows, dechirps with reference downchirp, takes FFT.
//   Consecutive peaks at bin ~0 → preamble upchirps.
// After preamble_len - 3 consecutive upchirps are found, the payload start is
// calculated as: preamble_start + (preamble_len + 4)*N + N/4
//   (preamble_len upchirps + 2 sync-word chirps + 2.25 downchirps).
//
// Assumes os_factor = 1 (samp_rate == bw), no CFO/STO correction.
struct FrameSyncResult {
    bool                              found;
    std::vector<std::complex<float>>  symbols;  // flat: N samples per symbol
};

FrameSyncResult frame_sync(const std::vector<std::complex<float>>& samples,
                            uint8_t sf,
                            uint8_t sync_word  = 0x12,
                            uint16_t preamble_len = 8);
