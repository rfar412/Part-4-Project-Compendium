#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <msp430.h>

#include "comm/comm.h"

#define UART_BUFFER_SIZE 128

// Initialize UART hardware
void uart_init(void);

// Send a single character
void uart_send_char(char c);

// Send a null-terminated string
void uart_send_string(const char *str);

// Send a number (for ADC)
void uart_send_num(unsigned int value);

// Helper to send binary representation of single char
static void uart_send_binary_char(char c);

// Receive a single character (blocking for now)
char uart_receive_char(void);

// Non-blocking receive check (optional later)
int uart_available(void);

void uint_to_str(unsigned int num, char *buf);

#endif // UART_H
