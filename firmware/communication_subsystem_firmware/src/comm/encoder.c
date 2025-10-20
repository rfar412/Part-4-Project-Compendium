#include "encoder.h"
#include "../utils/error_check.h"

// // Simple example: 8-bit ASCII + 1 parity bit
// uint16_t encode_char(char c) {
//     uint8_t parity = calc_parity((uint8_t)c);
//     return ((uint16_t)c << 1) | parity;
// }

// char decode_char(uint16_t code) {
//     uint8_t data = code >> 1;
//     uint8_t parity = code & 1;
//     // TODO: check parity
//     return data;
// }
