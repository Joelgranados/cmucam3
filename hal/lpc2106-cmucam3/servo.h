#ifndef SERVO_H
#define SERVO_H

#include "cc3_pin_defines.h"
#include "cc3.h"
#include "inttypes.h"
#include "stdbool.h"

// SERVO_RESOLUTION sets resolution within 1ms
// Failure may occure above 512
#define SERVO_RESOLUTION 255 
#define MAX_SERVOS 4
#define SERVO_PERIOD 18

// Sets up the servo timers and begins servicing the servos
void cc3_servo_init ();

// Interrupt called from interrupt.c when the servo needs to be serviced
void _cc3_servo_int ();

// Set a particular servo pin low
void _cc3_servo_lo (uint8_t n);

// Set all pins high at the start of the servo cycle
void _cc3_servo_hi_all ();

// User function to set a servo
uint8_t cc3_servo_set (uint8_t servo, uint32_t pos);

// User function to disable servos to conserve power
int cc3_servo_disable ();
#endif
