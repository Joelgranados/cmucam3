#include "cc3_hal.h"
#include "cc3_pin_defines.h"
#include "serial.h"

/*****************************************
*                                        *
* CAMERA REGISTER CONTROL FUNCTIONS      *
*                                        *
******************************************/
/**
 * _cc3_camera_reset()
 *
 * This is a private function that throbs the camera
 * reset pin.
 */
void _cc3_camera_reset ()
{
    // Reset the Camera 
    REG (GPIO_IOCLR) = _CC3_CAM_RESET;
    _cc3_delay_us_4 (1);        // XXX maybe wrong
    REG (GPIO_IOSET) = _CC3_CAM_RESET;
    _cc3_delay_us_4 (1);
    REG (GPIO_IOCLR) = _CC3_CAM_RESET;
    _cc3_delay_us_4 (1);
}



void _cc3_fifo_reset ()
{
    REG (GPIO_IOCLR) = _CC3_BUF_RESET;
    _cc3_delay_us_4 (1);
    REG (GPIO_IOSET) = _CC3_BUF_RESET;

    REG (GPIO_IOCLR) = _CC3_BUF_WEE;
    REG (GPIO_IOCLR) = _CC3_BUF_WRST;
    REG (GPIO_IOCLR) = _CC3_BUF_RRST;
    REG (GPIO_IOCLR) = _CC3_BUF_RCK;
    REG (GPIO_IOSET) = _CC3_BUF_RCK;
    REG (GPIO_IOCLR) = _CC3_BUF_RCK;
    _cc3_delay_us_4 (1);
    REG (GPIO_IOSET) = _CC3_BUF_WRST;
    REG (GPIO_IOSET) = _CC3_BUF_RRST;

}

/**
 * _cc3_delay_us_4()
 *
 * This function delays in intervauls of 4us
 * without using the timer.
 */
void _cc3_delay_us_4 (int cnt)
{
    volatile int i, x;
    for (i = 0; i < cnt; i++)
        for (x = 0; x < 10; x++);
}

void _cc3_delay_i2c ()
{
    volatile int x;
    for (x = 0; x < 1000; x++);

}

void _cc3_pixbuf_write_rewind ()
{
    REG (GPIO_IOCLR) = _CC3_BUF_WEE;
    REG (GPIO_IOCLR) = _CC3_BUF_WRST;
    _cc3_delay_us_4 (1);
    REG (GPIO_IOSET) = _CC3_BUF_WRST;
}
