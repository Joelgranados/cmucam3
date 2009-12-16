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


#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <stdbool.h>

extern volatile bool _cc3_button_trigger;

void enable_ext_interrupt (void);
void disable_ext_interrupt (void);
void enable_servo_interrupt (void);
void disable_servo_interrupt (void);
void enable_button_interrupt (void);
void disable_button_interrupt (void);

void interrupt (void);

void undefined (void);
void swi (void);
void prefetch_abort (void);
void data_abort (void);
void segfault (void);

#endif
