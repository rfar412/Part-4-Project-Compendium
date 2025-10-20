#include "error_check.h"

uint8_t calc_parity(uint8_t byte) {
    uint8_t parity = 0;
    int i;
    for (i = 0; i < 8; i++){
        parity ^= (byte >> i) & 1;
    }
    return parity;
}

uint8_t calc_checksum(const char *msg, int len) {
    uint8_t sum = 0;
    int i;
    for (i = 0; i < len; i++){
        sum += msg[i];
    }
    return sum;
}
