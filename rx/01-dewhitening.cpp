#include <vector>
#include "../common/tables.h"

// Reverses whiten(): recombines nibble pairs and XORs with the whitening sequence.
// Input length must be even (pairs of nibbles as produced by whiten()).
std::vector<uint8_t> dewhiten(const std::vector<uint8_t>& nibbles) {
    size_t n = nibbles.size() / 2;
    std::vector<uint8_t> out(n);
    for (size_t i = 0; i < n; i++) {
        uint8_t low_nib  = nibbles[2 * i]     ^ (whitening_seq[i] & 0x0F);
        uint8_t high_nib = nibbles[2 * i + 1] ^ (whitening_seq[i] >> 4);
        out[i] = (high_nib << 4) | low_nib;
    }
    return out;
}
