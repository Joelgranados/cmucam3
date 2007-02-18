/*
 * Copyright 2006  Anthony Rowe and Adam Goode
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */


#include "servo.h"
#include "LPC2100.h"
#include "lpc_config.h"
#include <stdio.h>

// Set a particular servo pin low
static void _cc3_servo_lo (uint8_t n);

// Set all pins high at the start of the servo cycle
static void _cc3_servo_hi_all (void);


static uint32_t servo_val[MAX_SERVOS];
static uint32_t servo_tmp[MAX_SERVOS];
static uint32_t servo_mask;

/**
 * cc3_servo_set()
 * This function sets a servo to be at given position.
 *
 * Returns 1 upon success.
 * Returns -1 if the servo or position is out of bounds.
 * 
 *  The servo will physically move on the next servo cycle.
 *  The servo operates at 50hz.
 */
bool cc3_gpio_set_servo_position (uint8_t servo, uint8_t pos)
{
    if (servo > MAX_SERVOS)
        return false;
    if (pos > SERVO_RESOLUTION)
        return false;
    servo_val[servo] = pos;
    return true;
}

uint8_t cc3_gpio_get_servo_position (uint8_t servo)
{
    if (servo >= MAX_SERVOS)
        return 0;
    return servo_val[servo];
}

/**
 * _cc3_servo_hi_all()
 *
 * This function pulls all servo pins high at the start of
 * the 20ms servo period.
 */
void _cc3_servo_hi_all ()
{
uint32_t tmp;
   tmp=0;
   if(servo_mask&0x1) tmp |= _CC3_SERVO_0;
   if(servo_mask&0x2) tmp |= _CC3_SERVO_1;
   if(servo_mask&0x4) tmp |= _CC3_SERVO_2;
   if(servo_mask&0x8) tmp |= _CC3_SERVO_3;
   printf( "cc3_servo_hi_all()\n" );
}

/**
 *  _cc3_servo_lo()
 *
 *  This pulls a particular servo line low.
 */
void _cc3_servo_lo (uint8_t n)
{
    switch (n) {
    case 0:
        if(servo_mask&0x1) 
	   printf( "cc3 servo 0 low\n" );
        break;
    case 1:
        if(servo_mask&0x2) 
	   printf( "cc3 servo 1 low\n" );
        break;
    case 2:
        if(servo_mask&0x4) 
	   printf( "cc3 servo 2 low\n" );
        break;
    case 3:
        if(servo_mask&0x8) 
	   printf( "cc3 servo 3 low\n" );
        break;
    }

}

/**
 * _cc3_servo_int()
 *
 * This is where the servo magic happens.  This function is called from
 * the timer1 interrupt in interrupts.c.
 *
 * It will schedule the next timer interrupt to fire when the next pin
 * state transition is needed.
 */
void _cc3_servo_int ()
{

}
