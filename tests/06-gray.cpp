#include <cstdio>
#include <cstdlib>
#include <vector>
#include "../tx/06-gray_mapping.h"
#include "../rx/06-gray_demap.h"

static int passed = 0, failed = 0;

static void check(const char* name, bool ok) {
    printf("%s: %s\n", name, ok ? "PASS" : "FAIL");
    ok ? passed++ : failed++;
}

// gray_map(x) = x ^ (x>>1)
static void test_gray_map_values() {
    uint8_t sf = 8;
    // Known pairs: x -> x ^ (x>>1)
    struct { uint32_t in, out; } cases[] = {
        {0, 0}, {1, 1}, {2, 3}, {3, 2}, {4, 6}, {7, 4}, {255, 128}
    };
    bool ok = true;
    for (auto& c : cases) {
        auto r = gray_map({c.in}, sf);
        if (r[0] != c.out) { ok = false; break; }
    }
    check("gray_map: known values", ok);
}

// gray_demap(gray_map(x)) = (x + 1) mod 2^sf
// (the +1 shift matches the LoRa modulator's -1 offset)
static void test_composition_sf(uint8_t sf) {
    uint32_t n = 1u << sf;
    bool ok = true;
    for (uint32_t x = 0; x < n; x++) {
        auto g = gray_map({x}, sf);
        auto r = gray_demap(g, sf);
        if (r[0] != ((x + 1) & (n - 1))) { ok = false; break; }
    }
    char name[64];
    snprintf(name, sizeof(name), "gray_demap(gray_map(x)) == (x+1) mod 2^sf  (sf=%d)", sf);
    check(name, ok);
}

// gray_map(gray_demap(x) - 1) = x  (inverse direction)
static void test_inverse_sf(uint8_t sf) {
    uint32_t n = 1u << sf;
    bool ok = true;
    for (uint32_t x = 0; x < n; x++) {
        auto d   = gray_demap({x}, sf);
        uint32_t shifted = (d[0] - 1 + n) & (n - 1);
        auto r   = gray_map({shifted}, sf);
        if (r[0] != x) { ok = false; break; }
    }
    char name[64];
    snprintf(name, sizeof(name), "gray_map(gray_demap(x)-1) == x  (sf=%d)", sf);
    check(name, ok);
}

// All output symbols stay within [0, 2^sf)
static void test_range(uint8_t sf) {
    uint32_t n = 1u << sf;
    std::vector<uint32_t> syms(n);
    for (uint32_t i = 0; i < n; i++) syms[i] = i;
    auto gm = gray_map(syms, sf);
    auto gd = gray_demap(syms, sf);
    bool ok = true;
    for (uint32_t i = 0; i < n; i++)
        if (gm[i] >= n || gd[i] >= n) { ok = false; break; }
    char name[64];
    snprintf(name, sizeof(name), "outputs in [0, 2^sf)  (sf=%d)", sf);
    check(name, ok);
}

// gray_map is a bijection (all outputs distinct)
static void test_bijection(uint8_t sf) {
    uint32_t n = 1u << sf;
    std::vector<uint32_t> syms(n);
    for (uint32_t i = 0; i < n; i++) syms[i] = i;
    auto gm = gray_map(syms, sf);
    std::vector<bool> seen(n, false);
    bool ok = true;
    for (uint32_t v : gm) {
        if (seen[v]) { ok = false; break; }
        seen[v] = true;
    }
    char name[64];
    snprintf(name, sizeof(name), "gray_map is bijection  (sf=%d)", sf);
    check(name, ok);
}

int main() {
    test_gray_map_values();

    for (uint8_t sf : {5, 6, 7, 8, 9, 10, 11, 12}) {
        test_composition_sf(sf);
        test_inverse_sf(sf);
        test_range(sf);
        test_bijection(sf);
    }

    printf("\n%d passed, %d failed\n", passed, failed);
    return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
