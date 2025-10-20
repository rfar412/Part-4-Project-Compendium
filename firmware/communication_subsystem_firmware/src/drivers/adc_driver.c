#include "adc_driver.h"

void adc_init(void) {

    P1DIR &= ~BIT2;           // input
    P1OUT &= ~BIT2;           // clear the output latch
    P1REN &= ~BIT2;           // disable internal pull resistor

    P1SEL1 |= BIT2;         // choose analogue function for P1.2
    P1SEL0 |= BIT2;         // 11 = Analogue

    ADCCTL0 |= ADCSHT_2;    // set conv clock cycles = 16(10)
    // ADCCTL0 |= (1 << 5);      // REFON = 1 (turn on reference)
    // ADCCTL0 |= (0 << 6);      // REF2_5V = 0 (select 1.5V ref)
    ADCCTL0 |= ADCON;       // turn on ADC

    ADCCTL1 |= ADCSSEL_2;   // choose SMCLK
    ADCCTL1 |= ADCSHP;      // sample signal source  = sampling timer

    // ADCCTL2 &= ~ADCDF;       // straight binary, right-aligned
    ADCCTL2 &= ~ADCRES;     // clear resolution
    ADCCTL2 |= ADCRES_1;    // 10 bit resolution

    

    ADCMCTL0 |= ADCINCH_2;  // ADC input = A4 (P1.2)
}

uint16_t adc_read(void) {
    ADCCTL0 |= ADCENC | ADCSC;          // Start conversion
    while ((ADCIFG & ADCIFG0) == 0);          // Wait for conversion to complete
    return ADCMEM0;                     // Return result
}
