#include "comm.h"

static uint8_t bit_array[MAX_MESSAGE_BITS];
static uint16_t bit_array_len;

// ================== Encode character to bits ==================
static void encode_char(uint8_t c) {
    int i;
    for (i = 6; i >= 0; i--) {
        bit_array[bit_array_len++] = (c >> i) & 1;
    }
}

// ================== Send preamble ==================
static void add_preamble(void) {
    int i;
    for (i = 7; i >= 0; i--) {
        bit_array[bit_array_len++] = 1; // preamble: 8 high bits
    }
}

// ================== Send message ==================
void send_message(const char *msg) {
    if (transmitting){
        return; // don’t restart mid-transmission
    }
    transmitting = true;
    bit_array_len = 0;

    // add_preamble();                                                                                                                                                                                                     
    size_t i;
    for (i = 0; i < strlen(msg); i++) {
        encode_char((uint8_t)msg[i]);
    }

    // start automatic transmission via timer ISR
    timer_a_start_message(bit_array, bit_array_len);
}
