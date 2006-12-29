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


#ifndef SERVO_H
#define SERVO_H

#include "cc3_pin_defines.h"
#include "cc3.h"
#include <stdint.h>
#include <stdbool.h>

// SERVO_RESOLUTION sets resolution within 1ms
// Failure may occure above 512
#define SERVO_RESOLUTION 255 
#define MAX_SERVOS 4
#define SERVO_PERIOD 18



// Interrupt called from interrupt.c when the servo needs to be serviced
void _cc3_servo_int (void);

#endif
