/*** MODULE INCLUDES ***/
#include <driverlib.h>
#include <stdio.h>
#include <msp430.h>
#include <msp430fr2433.h>
#include <stdint.h>
// #include <string.h>

/*** LIBRARY INCLUDES ***/
#include "comm/comm.h"
// #include "comm/encoder.h"
#include "drivers/adc_driver.h"
#include "drivers/lcd_driver.h"
#include "drivers/uart.h"
#include "timers/timer_a.h"

static inline void init(void);
 