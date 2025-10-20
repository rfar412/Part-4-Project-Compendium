/*** USE OWN HEADER ***/
#include "timer_a.h"

/*** MACROS ***/
#define LOW_FREQ  170       // Hz
#define HIGH_FREQ 240       // Hz

/*** GLOBAL VARIABLES ***/
volatile uint8_t bit_buffer[MAX_MESSAGE_BITS];
volatile uint16_t bit_count = 0;    // total bits in buffer
volatile uint16_t bit_index = 0;    // current bit being sent
volatile uint16_t cycles_remaining = 0;
volatile uint8_t transmitting = 0;

/*** ISRs ***/
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A0 (void){
    lcd_toggle();

    if (--cycles_remaining == 0) {
        // P1OUT ^= BIT1;  // toggle second LED for each symbol
        // _delay_cycles(50000);
        // P1OUT ^= BIT1;  // toggle second LED for each symbol

        load_next_bit();
    }
}

void timer_a_init(){
    /*** Set up Timer A ***/
    TA0CCTL0 = CCIE;                             // TACCR0 interrupt enabled
    TA0CTL = TASSEL__SMCLK + MC__STOP;             // SMCLK source, stopped initially
}

void timer_a_set_frequency(uint16_t frequency){
    TA0CTL &= ~(ID0 | ID1);

    uint16_t LCD_frequency = (frequency * 2) - (frequency * 0.09); // Accounts for ON + OFF = 2

    if(frequency <= 16){
        TA0CTL |= ID__8;
        TA0CCR0 = F_SMCLK_DIV_8 /LCD_frequency;
    }
    else {
        TA0CTL |= ID__1;
        TA0CCR0 = F_SMCLK / LCD_frequency;
    }
}

static void load_next_bit(void) {
    if (bit_index >= bit_count) {
        // done sending message
        TA0CTL &= ~MC__UP;   // stop timer
        transmitting = false;
        return;
    }

    uint8_t bit = bit_buffer[bit_index++];
    if (bit == 0) {
        timer_a_set_frequency(LOW_FREQ);
    }
    else {
        timer_a_set_frequency(HIGH_FREQ);
    }

    cycles_remaining = SYMBOL_REPS * 2;
}

void timer_a_start_message(uint8_t *bits, uint16_t count) {
    if (count > MAX_MESSAGE_BITS) {
        count = MAX_MESSAGE_BITS;
    }
    uint16_t i;
    for (i = 0; i < count; i++) {
        bit_buffer[i] = bits[i];
    }
    bit_count = count;
    bit_index = 0;

    load_next_bit();
    TA0CTL |= MC__UP; // start timer in up mode
}

