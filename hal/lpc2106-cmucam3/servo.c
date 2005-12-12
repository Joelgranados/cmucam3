#include "servo.h"
#include "LPC2100.h"
#include "lpc_config.h"
#include "interrupt.h"
#include <stdio.h>

uint32_t servo_val[MAX_SERVOS];
uint32_t servo_tmp[MAX_SERVOS];


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
    if (servo < 0 || servo > MAX_SERVOS)
        return -1;
    if (pos < 0 || pos > SERVO_RESOLUTION)
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
    for (i = 0; i < MAX_SERVOS; i++)
        servo_val[i] = SERVO_RESOLUTION / 2;
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
void _cc3_servo_hi_all ()
{
    REG (GPIO_IOSET) =
        _CC3_SERVO_0 | _CC3_SERVO_1 | _CC3_SERVO_2 | _CC3_SERVO_3;
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
        REG (GPIO_IOCLR) = _CC3_SERVO_0;
        break;
    case 1:
        REG (GPIO_IOCLR) = _CC3_SERVO_1;
        break;
    case 2:
        REG (GPIO_IOCLR) = _CC3_SERVO_2;
        break;
    case 3:
        REG (GPIO_IOCLR) = _CC3_SERVO_3;
        break;
    }

}

/**
 * cc3_disable()
 *
 * This function disables the servo interrupt and 
 * sets the servo lines low.
 */
void cc3_disable ()
{
uint8_t i;
    disable_servo_interrupt ();
    for (i = 0; i < MAX_SERVOS; i++)
         _cc3_servo_lo (i);
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
    uint32_t ct, i;
    uint32_t min;
    uint8_t safety;
    safety=1;
    ct = REG (TIMER1_TC);
//printf( "timer1 = %d\n",REG(TIMER1_TC) );
    if (ct == (SERVO_RESOLUTION * SERVO_PERIOD)) {
        // First time interrupt called, rising edge for all
        REG (TIMER1_TC) = 0;
        REG (TIMER1_MR0) = SERVO_RESOLUTION;    // schedule next wakeup for 1ms  
        _cc3_servo_hi_all ();
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
            if (servo_tmp[i] <= (ct+safety))
                _cc3_servo_lo (i);
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
