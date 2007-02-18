/*
 * Copyright 2006-2007  Anthony Rowe and Adam Goode
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
#include "interrupt.h"
#include "serial.h"
#include <stdio.h>

static uint8_t servo_val[MAX_SERVOS] = {
  SERVO_RESOLUTION / 2,
  SERVO_RESOLUTION / 2,
  SERVO_RESOLUTION / 2,
  SERVO_RESOLUTION / 2,
};

static uint8_t servo_tmp[MAX_SERVOS];
static uint32_t servo_mask;

const uint32_t _cc3_servo_map[] =
  {
    _CC3_SERVO_0,
    _CC3_SERVO_1,
    _CC3_SERVO_2,
    _CC3_SERVO_3,
  };


bool cc3_gpio_set_servo_position (uint8_t servo, uint8_t pos)
{
  if (servo >= MAX_SERVOS)
    return false;
  servo_val[servo] = pos;
  return true;
}

uint8_t cc3_gpio_get_servo_position (uint8_t servo)
{
  if (servo >= MAX_SERVOS) {
    return 0;
  }
  return servo_val[servo];
}

static void servo_init (void)
{
    // Setup timer1 to handle servos
    REG (TIMER1_TCR) = 0;       // turn off timer
    REG (TIMER1_TC) = 0;        // clear counter
    REG (TIMER1_PC) = 0;        // clear prescale count
    REG (TIMER1_PR) = (int) (((FOSC * PLL_M) / 1000) / SERVO_RESOLUTION);       // 1 tick is

    REG (TIMER1_IR) = 0x01;     // Clear pending interrupt on MR0
    REG (TIMER1_MCR) = 0x01;    // Interrupt on MR0 and reset TC
    REG (TIMER1_MR0) = SERVO_RESOLUTION * SERVO_PERIOD; // interrupt every 20 ms
    REG (TIMER1_TCR) = 1;       // start the timer
    enable_servo_interrupt ();
}

/**
 * _cc3_servo_hi_all()
 *
 * This function pulls all servo pins high at the start of
 * the 20ms servo period.
 */
static void servo_hi_all (void)
{
  uint32_t tmp = 0;
  if(servo_mask&0x1) tmp |= _CC3_SERVO_0;
  if(servo_mask&0x2) tmp |= _CC3_SERVO_1;
  if(servo_mask&0x4) tmp |= _CC3_SERVO_2;
  if(servo_mask&0x8) tmp |= _CC3_SERVO_3;
  REG( GPIO_IOSET) = tmp;
}

/**
 *  _cc3_servo_lo()
 *
 *  This pulls a particular servo line low.
 */
static void servo_lo (uint8_t n)
{
  REG (GPIO_IOCLR) = _cc3_servo_map[n];
}

void _cc3_servo_enable(uint8_t servo, bool enable)
{
  bool some_servos_already_enabled = !!servo_mask;

  uint32_t mask = 1 << servo;

  if (enable) {
    servo_mask |= mask;
    if (!some_servos_already_enabled) {
      servo_init();
    }
  } else {
    servo_mask &= ~mask;

    // any servos left?
    if (!servo_mask) {
      disable_servo_interrupt();
    }

    // set to low
    servo_lo(servo);
  }
}

/**
 * _cc3_servo_int()
 *
 * This is where the servo magic happens.  This function is called from
 * the timer1 interrupt in interrupt.c.
 *
 * It will schedule the next timer interrupt to fire when the next pin
 * state transition is needed.
 */
void _cc3_servo_int ()
{
    uint32_t ct, i;
    uint32_t min;
    uint8_t safety;
    safety=1;
    ct = REG (TIMER1_TC);
    //printf( "timer1 = %d\n",REG(TIMER1_TC) );
    // This used to be == instead of >= but one day the servos stopped working so we changed it
    // Now it works.
    if (ct >= (SERVO_RESOLUTION * SERVO_PERIOD)) {
        // First time interrupt called, rising edge for all
	//uart0_write( "FIRST call\r\n" );	
        REG (TIMER1_TC) = 0;
        REG (TIMER1_MR0) = SERVO_RESOLUTION;    // schedule next wakeup for 1ms
        servo_hi_all ();
    	// Copy current values into working values to avoid changes while
	// in the scheduling loop
    	for (i = 0; i < MAX_SERVOS; i++)
            servo_tmp[i] = servo_val[i];
    }
    else {
	// After the first edge, this section schedules the next interrupt
	// and pulls pins low as needed.
	if(SERVO_RESOLUTION>255) safety=2;
        ct -= SERVO_RESOLUTION;
        min = SERVO_RESOLUTION + 1;
        for (i = 0; i < MAX_SERVOS; i++) {
            if (servo_tmp[i] < min && servo_tmp[i] > (ct + safety))
                min = servo_tmp[i];
            if ((servo_tmp[i] <= (ct+safety)) && (servo_mask & (1 << i)))
                servo_lo (i);
        }
	// If all pins have been serviced, set a new interrupt for the
	// next servo period 20ms later
        if (min == SERVO_RESOLUTION + 1) {
            REG (TIMER1_MR0) = SERVO_RESOLUTION * SERVO_PERIOD;
        }                       // interrupt every 20 ms
        else
            REG (TIMER1_MR0) = SERVO_RESOLUTION + min;
    }
}
