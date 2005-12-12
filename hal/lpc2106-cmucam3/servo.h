#ifndef SERVO_H
#define SERVO_H

#include "cc3_pin_defines.h"
#include "cc3.h"
#include "inttypes.h"
#include "stdbool.h"

#define SERVO_RESOLUTION 255 
#define MAX_SERVOS 4
#define SERVO_PERIOD 18

// SERVO_RESOLUTION sets resolution within 1ms
void cc3_servo_init ();
void _cc3_servo_int ();
void _cc3_servo_lo (int n);
void _cc3_servo_hi_all ();
int cc3_servo_set (int servo, int pos);
int cc3_servo_disable ();
#endif
