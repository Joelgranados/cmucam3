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


#ifndef CC3_HAL_H
#define CC3_HAL_H

#include "cc3_pin_defines.h"
#include "cc3.h"
#include <stdint.h>
#include <stdbool.h>

/*****************************************
*                                        *
* CAMERA REGISTER CONTROL FUNCTIONS      *
*                                        *
******************************************/

typedef enum {
    _CC3_OV6620 = 0xC0,
    _CC3_OV7620 = 0x42
} _cc3_camera_type_t;

/**
 * This structure manages the internal record keeping that can get mapped into camera registers.
 */
typedef struct {
    uint8_t colorspace;
    int16_t brightness;
    int16_t contrast;
    uint8_t clock_divider;
    bool auto_exposure;
    bool auto_white_balance;
    uint8_t resolution;
    _cc3_camera_type_t camera_type;
} _cc3_camera_state_t;

/**
 * This is the global structure that the different cc3_set_xxxx() functions use in order to maintain consistency
 * if an attribute is changed that shares the same register as others.
 */
extern _cc3_camera_state_t _cc3_g_current_camera_state;

void _cc3_set_register_state (void);
void _cc3_camera_reset (void);
void _cc3_fifo_reset (void);
void _cc3_delay_us_4 (int cnt);
void _cc3_delay_i2c (void);

void _cc3_pixbuf_write_rewind (void);


#endif
