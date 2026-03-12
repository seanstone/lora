#include "01-whitening.h"
#include "../common/tables.h"

// XORs each input byte with the whitening sequence and outputs two nibbles per byte:
// out[2*i]   = low nibble of (in[i] ^ whitening_seq[i])
// out[2*i+1] = high nibble of (in[i] ^ whitening_seq[i])
std::vector<uint8_t> whiten(const std::vector<uint8_t>& payload) {
    std::vector<uint8_t> out(2 * payload.size());
    for (size_t i = 0; i < payload.size(); i++) {
        uint8_t w = payload[i] ^ whitening_seq[i];
        out[2 * i]     = w & 0x0F;
        out[2 * i + 1] = w >> 4;
    }
    return out;
}
