#include "07-modulate.h"
#include <cmath>

static const double TWO_PI = 2.0 * M_PI;

using sample_t = std::complex<float>;

// Build one upchirp for symbol id into out (2^sf samples, os_factor=1).
// Phase formula from gr-lora_sdr build_upchirp:
//   n < N-id: phase = n²/(2N) + (id/N - 0.5)*n
//   n ≥ N-id: phase = n²/(2N) + (id/N - 1.5)*n
static void append_upchirp(std::vector<sample_t>& out, uint32_t id, uint8_t sf) {
    uint32_t N      = 1u << sf;
    uint32_t n_fold = N - id;
    for (uint32_t n = 0; n < N; n++) {
        double t = (double)n;
        double phase = t * t / (2.0 * N) + ((n < n_fold)
            ? (double(id) / N - 0.5) * t
            : (double(id) / N - 1.5) * t);
        out.emplace_back((float)std::cos(TWO_PI * phase),
                         (float)std::sin(TWO_PI * phase));
    }
}

// Downchirp = conjugate of upchirp(id=0).
static void append_downchirp(std::vector<sample_t>& out, uint8_t sf, uint32_t samples) {
    uint32_t N = 1u << sf;
    for (uint32_t n = 0; n < samples; n++) {
        double t     = (double)n;
        double phase = t * t / (2.0 * N) - 0.5 * t;
        out.emplace_back((float)std::cos(TWO_PI * phase),
                        -(float)std::sin(TWO_PI * phase));  // conjugate
    }
}

std::vector<sample_t> modulate(const std::vector<uint32_t>& symbols,
                                uint8_t sf,
                                uint8_t sync_word,
                                uint16_t preamble_len) {
    uint32_t N = 1u << sf;

    // Sync word → two chirp ids (same mapping as GNURadio modulate block)
    uint32_t sw0 = uint32_t((sync_word & 0xF0u) >> 4) << 3;
    uint32_t sw1 = uint32_t( sync_word & 0x0Fu)       << 3;

    std::vector<sample_t> out;
    out.reserve((preamble_len + 4 + (uint32_t)symbols.size()) * N + N / 4);

    // Preamble upchirps
    for (int i = 0; i < preamble_len; i++)
        append_upchirp(out, 0, sf);

    // Two sync-word chirps
    append_upchirp(out, sw0, sf);
    append_upchirp(out, sw1, sf);

    // 2.25 downchirps
    append_downchirp(out, sf, 2 * N + N / 4);

    // Payload
    for (uint32_t sym : symbols)
        append_upchirp(out, sym, sf);

    return out;
}
