/*
 * Copyright 2006  Anthony Rowe and Adam Goode
 *
 * This file is part of cc3.
 *
 * cc3 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * cc3 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with cc3; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
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
uint8_t cc3_servo_set (uint8_t servo, uint32_t pos)
{
    if (servo > MAX_SERVOS)
        return -1;
    if (pos > SERVO_RESOLUTION)
        return -1;
    servo_val[servo] = pos;
    return 1;
}

/**
 * cc3_servo_init()
 *
 * This function sets up timer1 to control the servos.
 * It is periodically called by an interrupt in interrupt.c
 *
 * This function can be called again after servos are disabled.
 */
void cc3_servo_init ()
{
    int i;
    servo_mask=0xFFFFF;
    for (i = 0; i < MAX_SERVOS; i++)
        servo_val[i] = SERVO_RESOLUTION / 2;
    // Setup timer1 to handle servos
}

void cc3_servo_mask(uint32_t mask)
{
servo_mask=mask;
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
 * cc3_disable()
 *
 * This function disables the servo interrupt and 
 * sets the servo lines low.
 */
void cc3_servo_disable ()
{
uint8_t i;
printf( "cc3 servo disabled\n" );
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
