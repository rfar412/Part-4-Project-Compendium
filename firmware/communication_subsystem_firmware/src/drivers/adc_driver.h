#ifndef ADC_DRIVER_H
#define ADC_DRIVER_H

#include <stdint.h>
#include <msp430.h>
#include "driverlib.h"

void adc_init(void);
uint16_t adc_read(void);

#endif
