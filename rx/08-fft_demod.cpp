#include "08-fft_demod.h"
#include <cmath>

extern "C" {
#include "../common/kiss_fft.h"
}

static const double TWO_PI = 2.0 * M_PI;

std::vector<uint32_t> fft_demod(const std::vector<std::complex<float>>& samples,
                                 uint8_t sf) {
    uint32_t N = 1u << sf;

    // Reference downchirp: conjugate of upchirp(id=0)
    std::vector<std::complex<float>> downchirp(N);
    for (uint32_t n = 0; n < N; n++) {
        double t     = n;
        double phase = t * t / (2.0 * N) - 0.5 * t;
        downchirp[n] = {(float)std::cos(TWO_PI * phase),
                        -(float)std::sin(TWO_PI * phase)};
    }

    kiss_fft_cfg  cfg    = kiss_fft_alloc(N, 0, 0, 0);
    kiss_fft_cpx* cx_in  = new kiss_fft_cpx[N];
    kiss_fft_cpx* cx_out = new kiss_fft_cpx[N];

    std::vector<uint32_t> out;
    out.reserve(samples.size() / N);

    for (size_t w = 0; w + N <= samples.size(); w += N) {
        for (uint32_t i = 0; i < N; i++) {
            std::complex<float> d = samples[w + i] * downchirp[i];
            cx_in[i].r = d.real();
            cx_in[i].i = d.imag();
        }
        kiss_fft(cfg, cx_in, cx_out);

        float    best = 0;
        uint32_t idx  = 0;
        for (uint32_t i = 0; i < N; i++) {
            float mag = cx_out[i].r * cx_out[i].r + cx_out[i].i * cx_out[i].i;
            if (mag > best) { best = mag; idx = i; }
        }
        out.push_back(idx);
    }

    kiss_fft_free(cfg);
    delete[] cx_in;
    delete[] cx_out;
    return out;
}
