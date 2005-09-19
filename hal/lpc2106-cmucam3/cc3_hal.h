#ifndef CC3_HAL_H
#define CC3_HAL_H

#include "cc3_pin_defines.h"
#include "cc3.h"
#include "inttypes.h"

/*****************************************
*                                        *
* CAMERA REGISTER CONTROL FUNCTIONS      *
*                                        *
******************************************/
void _cc3_camera_reset ();
void _cc3_fifo_reset ();
void _cc3_delay_us_4 (int cnt);
void _cc3_delay_i2c ();
// Move to the next byte in the FIFO 
#define _CC3_FIFO_READ_INC() REG(GPIO_IOSET)=_CC3_BUF_RCK; REG(GPIO_IOCLR)=_CC3_BUF_RCK
void _cc3_pixbuf_write_rewind ();


#endif
