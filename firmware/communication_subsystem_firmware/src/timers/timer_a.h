#ifndef TIMER_A_H
#define TIMER_A_H

#include <msp430.h>
#include <msp430fr2433.h>
#include <stdint.h>

#include "drivers/lcd_driver.h"
#include "comm/comm.h"

/*** MACROS ***/
#define F_CPU                   16000000UL
#define F_SMCLK                 1000000UL
#define F_SMCLK_DIV_8           125000UL

#define EIGHT_BIT               256     // number of values (so max useable value is 256-1)
#define SIXTEEN_BIT             65536   // number of values (so max useable value is 65536-1)

#define SYMBOL_REPS             5       // No. symbol repetitions
#define MAX_MESSAGE_BITS        512

extern volatile uint8_t transmitting;

void timer_a_init(void);
void timer_a_set_frequency(uint16_t frequency);
void timer_a_start_message(uint8_t *bits, uint16_t count);

#endif
