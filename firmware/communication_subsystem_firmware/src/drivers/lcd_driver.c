#include "lcd_driver.h"

void lcd_init(void) {
    // Configure GPIO connected to LCD
    P1DIR |= BIT0 | BIT1;  // both pins as output
    P1OUT |= BIT0;         // LCD starts ON (blocks light)
    P1OUT &= ~BIT1;        // second LED starts OFF
}

void lcd_toggle(void) {
    P1OUT ^= BIT0;  // Toggle LCD
}
