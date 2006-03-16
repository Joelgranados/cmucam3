#ifndef SERVO_H
#define SERVO_H

#include "cc3_pin_defines.h"
#include "cc3.h"
#include <stdint.h>
#include <stdbool.h>

// SERVO_RESOLUTION sets resolution within 1ms
// Failure may occure above 512
#define SERVO_RESOLUTION 255 
#define MAX_SERVOS 4
#define SERVO_PERIOD 18



// Interrupt called from interrupt.c when the servo needs to be serviced
void _cc3_servo_int (void);

#endif
