#include "05-interleaver.h"

static inline int imod(int a, int b) { return ((a % b) + b) % b; }

// Process one block: sf_app codewords → cw_len symbols (sf_app bits each, MSB-first).
// Permutation: inter_bin[i][j] = cw_bin[mod(i-j-1, sf_app)][i]
// where cw_bin[row][col] = (codewords[row] >> (cw_len-1-col)) & 1  (MSB-first)
// and symbol i = bool2int(inter_bin[i])
//             = sum_j( inter_bin[i][j] * 2^(sf_app-1-j) )
static void emit_block(const std::vector<uint8_t>& codewords, size_t pos,
                       int sf_app, int cw_len,
                       std::vector<uint32_t>& symbols) {
    for (int i = 0; i < cw_len; i++) {
        uint32_t sym = 0;
        for (int j = 0; j < sf_app; j++) {
            int row = imod(i - j - 1, sf_app);
            int bit = (codewords[pos + row] >> (cw_len - 1 - i)) & 1;
            sym |= (uint32_t)bit << (sf_app - 1 - j);
        }
        symbols.push_back(sym);
    }
}

std::vector<uint32_t> interleave(const std::vector<uint8_t>& codewords,
                                  uint8_t cr, uint8_t sf, bool ldro) {
    std::vector<uint32_t> symbols;
    size_t n   = codewords.size();
    size_t pos = 0;

    // Header block: sf_app = sf-2, cw_len = 8 (cr_app=4 → 4+4)
    if (pos < n) {
        emit_block(codewords, pos, sf - 2, 8, symbols);
        pos += (sf - 2);  // consumed sf_app codewords
    }

    // Payload blocks: sf_app = ldro ? sf-2 : sf, cw_len = cr+4
    int pay_sf  = ldro ? sf - 2 : sf;
    int pay_len = cr + 4;
    while (pos < n) {
        emit_block(codewords, pos, pay_sf, pay_len, symbols);
        pos += pay_sf;
    }

    return symbols;
}
