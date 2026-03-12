#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <complex>
#include <vector>
#include "../tx/07-modulate.h"

static int passed = 0, failed = 0;

static void check(const char* name, bool ok) {
    printf("%s: %s\n", name, ok ? "PASS" : "FAIL");
    ok ? passed++ : failed++;
}

// Output length: (preamble_len + 2 sync + 2.25 downchirps + n_payload) * N
static void test_output_length(uint8_t sf, uint16_t preamble_len, int n_syms) {
    uint32_t N   = 1u << sf;
    std::vector<uint32_t> syms(n_syms, 0);
    auto out     = modulate(syms, sf, 0x12, preamble_len);
    size_t expect = (size_t)(preamble_len + 2 + n_syms) * N + N / 4 + 2 * N;
    // 2.25 downchirps = 2*N + N/4 samples
    // total = (preamble_len + 2_sync + n_syms) * N + 2*N + N/4
    expect = (size_t)(preamble_len + 2 + n_syms) * N + 2 * N + N / 4;
    char name[80];
    snprintf(name, sizeof(name), "output length sf=%d preamble=%d syms=%d", sf, preamble_len, n_syms);
    check(name, out.size() == expect);
}

// All samples have unit magnitude (chirps are complex exponentials)
static void test_unit_magnitude(uint8_t sf) {
    std::vector<uint32_t> syms = {0, 1, (1u<<sf)-1};
    auto out = modulate(syms, sf);
    bool ok  = true;
    for (auto& s : out) {
        float mag = std::abs(s);
        if (std::abs(mag - 1.0f) > 1e-5f) { ok = false; break; }
    }
    char name[64];
    snprintf(name, sizeof(name), "unit magnitude sf=%d", sf);
    check(name, ok);
}

// All preamble chirps (id=0) are identical
static void test_preamble_identical(uint8_t sf, uint16_t preamble_len) {
    uint32_t N   = 1u << sf;
    auto out     = modulate({}, sf, 0x12, preamble_len);
    bool ok = true;
    for (uint16_t p = 1; p < preamble_len; p++) {
        for (uint32_t n = 0; n < N; n++) {
            auto a = out[n];
            auto b = out[p * N + n];
            if (std::abs(a - b) > 1e-5f) { ok = false; break; }
        }
        if (!ok) break;
    }
    char name[64];
    snprintf(name, sizeof(name), "preamble chirps identical sf=%d", sf);
    check(name, ok);
}

// Two different symbols produce different chirps
static void test_symbols_differ(uint8_t sf) {
    uint32_t N    = 1u << sf;
    auto out0     = modulate({0}, sf, 0x12, 1);
    auto out1     = modulate({1}, sf, 0x12, 1);
    // Compare the last N samples (the payload chirp)
    size_t offset = out0.size() - N;
    bool differ   = false;
    for (uint32_t n = 0; n < N; n++) {
        if (std::abs(out0[offset + n] - out1[offset + n]) > 1e-5f) {
            differ = true; break;
        }
    }
    char name[64];
    snprintf(name, sizeof(name), "distinct symbols differ sf=%d", sf);
    check(name, differ);
}

// Sync word 0x12: sw0 = (1<<3)=8, sw1 = (2<<3)=16
// Verify the sync-word chirps differ from the preamble (id=0)
static void test_sync_word_differs_from_preamble(uint8_t sf) {
    uint32_t N    = 1u << sf;
    uint16_t pl   = 4;
    auto out      = modulate({}, sf, 0x12, pl);
    // sync_word chirps start at pl*N
    bool ok = false;
    for (uint32_t n = 0; n < N; n++) {
        if (std::abs(out[n] - out[pl * N + n]) > 1e-5f) { ok = true; break; }
    }
    char name[64];
    snprintf(name, sizeof(name), "sync word != preamble chirp sf=%d", sf);
    check(name, ok);
}

int main() {
    // Length tests
    test_output_length(7, 8, 0);
    test_output_length(7, 8, 5);
    test_output_length(8, 8, 10);
    test_output_length(9, 6, 3);
    test_output_length(12, 8, 1);

    // Unit magnitude
    for (uint8_t sf : {7, 8, 9, 12})
        test_unit_magnitude(sf);

    // Preamble
    for (uint8_t sf : {7, 8, 9})
        test_preamble_identical(sf, 8);

    // Symbol distinctness
    for (uint8_t sf : {7, 8, 9, 12})
        test_symbols_differ(sf);

    // Sync word
    for (uint8_t sf : {7, 8, 9})
        test_sync_word_differs_from_preamble(sf);

    printf("\n%d passed, %d failed\n", passed, failed);
    return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
