#include "05-deinterleaver.h"

static inline int imod(int a, int b) { return ((a % b) + b) % b; }

// Process one block: cw_len symbols (sf_app bits each) → sf_app codewords (cw_len bits each).
// Inverse permutation: cw[row] bit at col i = sym[i] bit at pos (sf_app-1-j)
//   where j = mod(i - row - 1, sf_app)
static void consume_block(const std::vector<uint32_t>& symbols, size_t pos,
                          int sf_app, int cw_len,
                          std::vector<uint8_t>& codewords) {
    for (int row = 0; row < sf_app; row++) {
        uint8_t cw = 0;
        for (int i = 0; i < cw_len; i++) {
            int j   = imod(i - row - 1, sf_app);
            int bit = (symbols[pos + i] >> (sf_app - 1 - j)) & 1;
            cw |= (uint8_t)bit << (cw_len - 1 - i);
        }
        codewords.push_back(cw);
    }
}

std::vector<uint8_t> deinterleave(const std::vector<uint32_t>& symbols,
                                   uint8_t cr, uint8_t sf, bool ldro) {
    std::vector<uint8_t> codewords;
    size_t n   = symbols.size();
    size_t pos = 0;

    // Header block: 8 symbols → sf-2 codewords
    if (pos + 8 <= n) {
        consume_block(symbols, pos, sf - 2, 8, codewords);
        pos += 8;
    }

    // Payload blocks: cr+4 symbols → sf_app codewords
    int pay_sf  = ldro ? sf - 2 : sf;
    int pay_len = cr + 4;
    while (pos + pay_len <= n) {
        consume_block(symbols, pos, pay_sf, pay_len, codewords);
        pos += pay_len;
    }

    return codewords;
}
