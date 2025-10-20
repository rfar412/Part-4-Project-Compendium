#ifndef COMM_H
#define COMM_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "encoder.h"
#include "drivers/lcd_driver.h"
#include "drivers/adc_driver.h"
#include "timers/timer_a.h"

#define MAX_MESSAGE_BITS 512

static uint8_t bit_array[MAX_MESSAGE_BITS];
static uint16_t bit_array_len;

static void encode_char(uint8_t c);
static void add_preamble(void);
void send_message(const char *msg);


#endif

