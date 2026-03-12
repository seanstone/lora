#include <cstdint>
#include <vector>

struct FrameInfo {
    uint8_t payload_len;
    uint8_t cr;
    bool    has_crc;
    bool    valid;          // false if checksum mismatch or payload_len == 0
    std::vector<uint8_t> payload_nibbles;
};

// Strips the 5-nibble explicit header from a framed nibble stream and validates
// its checksum. If impl_head is true, the provided cr/pay_len/has_crc are used
// and the nibbles are returned unchanged.
FrameInfo decode_header(const std::vector<uint8_t>& frame,
                        bool impl_head,
                        uint8_t cr = 0, uint32_t pay_len = 0, bool has_crc = false)
{
    if (impl_head) {
        return { (uint8_t)pay_len, cr, has_crc, true,
                 std::vector<uint8_t>(frame.begin(), frame.end()) };
    }

    if (frame.size() < 5)
        return { 0, 0, false, false, {} };

    uint8_t h0 = frame[0], h1 = frame[1], h2 = frame[2];

    uint8_t decoded_len = (h0 << 4) | h1;
    bool    decoded_crc = h2 & 1;
    uint8_t decoded_cr  = h2 >> 1;

    // Stored checksum: c4 is bit 0 of nibble 3, c3..c0 are nibble 4
    uint8_t stored_chk = ((frame[3] & 1) << 4) | frame[4];

    // Recompute checksum
    bool c4 = ((h0 >> 3) & 1) ^ ((h0 >> 2) & 1) ^ ((h0 >> 1) & 1) ^ (h0 & 1);
    bool c3 = ((h0 >> 3) & 1) ^ ((h1 >> 3) & 1) ^ ((h1 >> 2) & 1) ^ ((h1 >> 1) & 1) ^ (h2 & 1);
    bool c2 = ((h0 >> 2) & 1) ^ ((h1 >> 3) & 1) ^ (h1 & 1) ^ ((h2 >> 3) & 1) ^ ((h2 >> 1) & 1);
    bool c1 = ((h0 >> 1) & 1) ^ ((h1 >> 2) & 1) ^ (h1 & 1) ^ ((h2 >> 2) & 1) ^ ((h2 >> 1) & 1) ^ (h2 & 1);
    bool c0 = (h0 & 1) ^ ((h1 >> 1) & 1) ^ ((h2 >> 3) & 1) ^ ((h2 >> 2) & 1) ^ ((h2 >> 1) & 1) ^ (h2 & 1);

    uint8_t computed_chk = (c4 << 4) | (c3 << 3) | (c2 << 2) | (c1 << 1) | c0;
    bool valid = (stored_chk == computed_chk) && (decoded_len > 0);

    return { decoded_len, decoded_cr, decoded_crc, valid,
             std::vector<uint8_t>(frame.begin() + 5, frame.end()) };
}
