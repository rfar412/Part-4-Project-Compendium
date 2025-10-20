/*** USE HEADER ***/
#include "main.h"

/*** GLOBAL VARIABLES ***/
// volatile uint8_t count = 0;
// volatile uint8_t frequency = 1;

int main(void) {
    // Initialize MCU and peripherals
    init();

    /*** Enable interrupts ***/
    __bis_SR_register(GIE);
    // uart_send_string("Hello\n\r");
    send_message("A");
 
    

    while(1) {
        unsigned int value = adc_read();
        char buffer[6];
        uint_to_str(value, buffer);
        uart_send_string(buffer);
        uart_send_string("\r\n");

        // unsigned int val = ADCMEM0;
        // uart_send_string("Raw: ");
        // char buffer[6];
        // uint_to_str(val, buffer);
        // uart_send_string(buffer);
        // uart_send_string("\r\n");

        

        __delay_cycles(100);  // Adjust sampling rate
    }
}

static inline void init(){
    WDTCTL = WDTPW | WDTHOLD; // Stop WDT
    PM5CTL0 &= ~LOCKLPM5;

    lcd_init();
    adc_init();
    timer_a_init();
    uart_init();
}
