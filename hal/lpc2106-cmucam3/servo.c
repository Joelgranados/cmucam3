#include "servo.h"
#include "LPC2100.h"
#include "lpc_config.h"
#include "interrupt.h"
#include <stdio.h>

int blah;
int servo_val[MAX_SERVOS];
int servo_tmp[MAX_SERVOS];
int cc3_servo_set (int servo, int pos)
{
    if (servo < 0 || servo > MAX_SERVOS)
        return -1;
    if (pos < 0 || pos > SERVO_RESOLUTION)
        return -1;
    servo_val[servo] = pos;
    return 1;
}

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

void _cc3_servo_hi_all ()
{
    REG (GPIO_IOSET) =
        _CC3_SERVO_0 | _CC3_SERVO_1 | _CC3_SERVO_2 | _CC3_SERVO_3;
}


void _cc3_servo_lo (int n)
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

void cc3_disable ()
{
    disable_servo_interrupt ();
}

void _cc3_servo_int ()
{
    int ct, i;
    ct = REG (TIMER1_TC);
//printf( "timer1 = %d\n",REG(TIMER1_TC) );
    if (ct == (SERVO_RESOLUTION * SERVO_PERIOD)) {
        // First time interrupt called, rising edge for all
        REG (TIMER1_TC) = 0;
        _cc3_servo_hi_all ();
        REG (TIMER1_MR0) = SERVO_RESOLUTION;    // schedule next wakeup for 1ms  
        for (i = 0; i < MAX_SERVOS; i++)
            servo_tmp[i] = servo_val[i];
    }
    else {
        int min;
        ct -= SERVO_RESOLUTION;
        min = SERVO_RESOLUTION + 1;
        for (i = 0; i < MAX_SERVOS; i++) {
            if (servo_tmp[i] < min && servo_tmp[i] > (ct + 1))
                min = servo_tmp[i];
            if (servo_tmp[i] <= (ct+1))
                _cc3_servo_lo (i);
        }

        if (min == SERVO_RESOLUTION + 1) {
            REG (TIMER1_MR0) = SERVO_RESOLUTION * SERVO_PERIOD;
        }                       // interrupt every 20 ms  
        else
            REG (TIMER1_MR0) = SERVO_RESOLUTION + min;
    }


}
