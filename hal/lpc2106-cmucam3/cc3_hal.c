#include "cc3_hal.h"
#include "cc3_pin_defines.h"
#include "serial.h"
#include <stdio.h>


_cc3_camera_state_t _cc3_g_current_camera_state;


/*****************************************
*                                        *
* CAMERA REGISTER CONTROL FUNCTIONS      *
*                                        *
******************************************/
/**
 * This is a private function that throbs the camera
 * reset pin.
 */
void _cc3_camera_reset ()
{
    // Reset the Camera 
    REG (GPIO_IOCLR) = _CC3_CAM_RESET;
    _cc3_delay_us_4 (1);        
    REG (GPIO_IOSET) = _CC3_CAM_RESET;
    _cc3_delay_us_4 (1);
    REG (GPIO_IOCLR) = _CC3_CAM_RESET;
    _cc3_delay_us_4 (1);
}


/**
 * This function resets the fifo chip.
 */
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



/*
 * This function goes through the register state structures and sets the corresponding camera registers.
 * It also updates any internal camera structures such as resolution that may change.
 */
void _cc3_set_register_state ()
{
    switch (_cc3_g_current_camera_state.camera_type) {
    case _CC3_OV6620:
        // Set the right data bus mode
        cc3_set_raw_register (0x14, 0x20);

        // Set the resolution and update the cc3_g_current_frame size flags
        if (_cc3_g_current_camera_state.resolution == CC3_LOW_RES) {
            cc3_g_current_frame.raw_width = 88;
            cc3_g_current_frame.raw_height = 144;
            cc3_set_raw_register (0x14, 0x20);
        }
        else {
            cc3_g_current_frame.raw_width = 176;
            cc3_g_current_frame.raw_height = 288;
            cc3_set_raw_register (0x14, 0x00);
        }

        if (_cc3_g_current_camera_state.auto_exposure) {
            cc3_set_raw_register (0x13, 0x21);
        }
        else {
            // No auto gain, so lets set brightness and contrast if need be
            cc3_set_raw_register (0x13, 0x20);
            if (_cc3_g_current_camera_state.brightness != -1)
                cc3_set_raw_register (0x06,
                                      (_cc3_g_current_camera_state.
                                       brightness & 0xFF));

            if (_cc3_g_current_camera_state.contrast != -1)
                cc3_set_raw_register (0x05,
                                      (_cc3_g_current_camera_state.
                                       contrast & 0xFF));
        }
        // Set Colorspace and Auto White Balance
        cc3_set_raw_register (0x12,
                              0x20 | (_cc3_g_current_camera_state.
                                      auto_white_balance << 2)
                              | (_cc3_g_current_camera_state.
                                 colorspace << 3));
        // Set Frame Clock rate divider
        cc3_set_raw_register (0x11,
                              _cc3_g_current_camera_state.clock_divider);


        break;

    case _CC3_OV7620:
        // XXX I need code.  The CMUcam2 is kind of wrong, so lets fix it...
        break;
    }


}
