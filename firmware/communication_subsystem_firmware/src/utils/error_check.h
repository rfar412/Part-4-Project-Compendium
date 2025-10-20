#ifndef ERROR_CHECK_H
#define ERROR_CHECK_H

#include <stdint.h>
#include <stdio.h>

uint8_t calc_parity(uint8_t byte);
uint8_t calc_checksum(const char *msg, int len);

#endif 
