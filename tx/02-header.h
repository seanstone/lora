#pragma once
#include <cstdint>
#include <vector>

std::vector<uint8_t> add_header(const std::vector<uint8_t>& nibbles,
                                bool impl_head, bool has_crc, uint8_t cr);
