#include <cstdio>
#include <cstdlib>
#include <vector>
#include "../tx/07-modulate.h"
#include "../rx/07-frame_sync.h"
#include "../rx/08-fft_demod.h"

static int passed = 0, failed = 0;

static void check(const char* name, bool ok) {
    printf("%s: %s\n", name, ok ? "PASS" : "FAIL");
    ok ? passed++ : failed++;
}

static void test_roundtrip(uint8_t sf, uint16_t preamble_len, int n_syms) {
    uint32_t N = 1u << sf;
    std::vector<uint32_t> syms(n_syms);
    for (int i = 0; i < n_syms; i++)
        syms[i] = (uint32_t)(i * 13 + 7) % N;

    auto iq   = modulate(syms, sf, 0x12, preamble_len);
    auto sync = frame_sync(iq, sf, 0x12, preamble_len);

    char name[80];
    snprintf(name, sizeof(name),
             "frame_sync found sf=%d pl=%d nsyms=%d", sf, preamble_len, n_syms);
    check(name, sync.found);
    if (!sync.found) return;

    auto recovered = fft_demod(sync.symbols, sf);

    snprintf(name, sizeof(name),
             "symbol count  sf=%d pl=%d nsyms=%d", sf, preamble_len, n_syms);
    check(name, recovered.size() == (size_t)n_syms);
    if (recovered.size() != (size_t)n_syms) return;

    bool match = true;
    for (int i = 0; i < n_syms; i++) {
        if (recovered[i] != syms[i]) { match = false; break; }
    }
    snprintf(name, sizeof(name),
             "symbols match  sf=%d pl=%d nsyms=%d", sf, preamble_len, n_syms);
    check(name, match);
}

int main() {
    test_roundtrip(7,  8, 1);
    test_roundtrip(7,  8, 5);
    test_roundtrip(7,  8, 10);
    test_roundtrip(8,  8, 4);
    test_roundtrip(9,  8, 3);
    test_roundtrip(7,  6, 2);
    test_roundtrip(12, 8, 1);

    printf("\n%d passed, %d failed\n", passed, failed);
    return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
