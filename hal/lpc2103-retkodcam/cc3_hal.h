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


#ifndef CC3_HAL_H
#define CC3_HAL_H

#include "cc3_pin_defines.h"
#include "cc3.h"
#include <stdint.h>
#include <stdbool.h>

#define CC3_HI_RES_HEIGHT		287
#define CC3_HI_RES_WIDTH		352	

#define CC3_LO_RES_HEIGHT 		143	
#define CC3_LO_RES_WIDTH		176




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
    int16_t brightness;
    int16_t contrast;
    uint8_t colorspace;
    uint8_t clock_divider;
    uint8_t resolution;
    _cc3_camera_type_t camera_type;
    bool auto_exposure;
    bool auto_white_balance;
} _cc3_camera_state_t;

// called only from startup.s
void _cc3_system_setup (void);

void _cc3_camera_reset (void);
void _cc3_fifo_reset (void);
void _cc3_delay_us_4 (int cnt);
void _cc3_delay_sccb (void);

void _cc3_pixbuf_write_rewind (void);


#endif
