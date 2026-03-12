#include "07-frame_sync.h"
#include <cmath>
#include <algorithm>

extern "C" {
#include "../common/kiss_fft.h"
}

static const double TWO_PI = 2.0 * M_PI;

static std::vector<std::complex<float>> make_downchirp(uint8_t sf) {
    uint32_t N = 1u << sf;
    std::vector<std::complex<float>> dc(N);
    for (uint32_t n = 0; n < N; n++) {
        double t = n;
        double phase = t * t / (2.0 * N) - 0.5 * t;
        dc[n] = {(float)std::cos(TWO_PI * phase), -(float)std::sin(TWO_PI * phase)};
    }
    return dc;
}

static uint32_t symbol_bin(const std::complex<float>* samples,
                            const std::vector<std::complex<float>>& downchirp,
                            kiss_fft_cfg cfg,
                            kiss_fft_cpx* cx_in, kiss_fft_cpx* cx_out,
                            uint32_t N) {
    for (uint32_t i = 0; i < N; i++) {
        std::complex<float> d = samples[i] * downchirp[i];
        cx_in[i].r = d.real();
        cx_in[i].i = d.imag();
    }
    kiss_fft(cfg, cx_in, cx_out);
    float best = 0;
    uint32_t idx = 0;
    for (uint32_t i = 0; i < N; i++) {
        float mag = cx_out[i].r * cx_out[i].r + cx_out[i].i * cx_out[i].i;
        if (mag > best) { best = mag; idx = i; }
    }
    return idx;
}

FrameSyncResult frame_sync(const std::vector<std::complex<float>>& samples,
                            uint8_t sf,
                            uint8_t sync_word,
                            uint16_t preamble_len) {
    (void)sync_word;  // not verified in ideal-signal detection

    uint32_t N        = 1u << sf;
    uint32_t n_up_req = preamble_len - 3;

    auto downchirp = make_downchirp(sf);

    kiss_fft_cfg  cfg    = kiss_fft_alloc(N, 0, 0, 0);
    kiss_fft_cpx* cx_in  = new kiss_fft_cpx[N];
    kiss_fft_cpx* cx_out = new kiss_fft_cpx[N];

    uint32_t consec        = 0;
    uint32_t preamble_start = 0;
    FrameSyncResult res{false, {}};

    for (size_t w = 0; w + N <= samples.size(); w += N) {
        uint32_t bin   = symbol_bin(samples.data() + w, downchirp, cfg, cx_in, cx_out, N);
        bool     is_up = (bin <= 1 || bin >= N - 1);

        if (is_up) {
            if (consec == 0) preamble_start = (uint32_t)w;
            consec++;
            if (consec >= n_up_req) {
                // preamble_len upchirps + 2 sync + 2.25 downchirps
                size_t payload_start = preamble_start
                                     + (size_t)(preamble_len + 4) * N
                                     + N / 4;
                if (payload_start + N <= samples.size()) {
                    res.found = true;
                    size_t len = ((samples.size() - payload_start) / N) * N;
                    res.symbols.assign(samples.begin() + payload_start,
                                       samples.begin() + payload_start + len);
                }
                break;
            }
        } else {
            consec = 0;
        }
    }

    kiss_fft_free(cfg);
    delete[] cx_in;
    delete[] cx_out;
    return res;
}
