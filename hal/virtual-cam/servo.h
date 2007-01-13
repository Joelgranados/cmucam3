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
