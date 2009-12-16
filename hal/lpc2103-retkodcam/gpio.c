/*
 * Copyright 2006-2007  Anthony Rowe and Adam Goode
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


#include "gpio.h"
#include "servo.h"
#include "LPC2100.h"
#include <stdio.h>
#include "cc3_pin_defines.h"
#include "cc3.h"
#include <stdint.h>
#include <stdbool.h>


void cc3_gpio_set_mode(uint8_t pin, cc3_gpio_mode_t mode)
{
  if (pin >= MAX_SERVOS) {
    return;
  }

  uint32_t pin_val = _cc3_servo_map[pin];

  switch (mode) {
  case CC3_GPIO_MODE_INPUT:
    // not a servo
    _cc3_servo_enable(pin, false);

    // set to 0
    REG (GPIO_IODIR) &= ~pin_val;
    break;

  case CC3_GPIO_MODE_OUTPUT:
    // not a servo
    _cc3_servo_enable(pin, false);

    // set to 1
    REG (GPIO_IODIR) |= pin_val;
    break;

  case CC3_GPIO_MODE_SERVO:
    // a servo
    _cc3_servo_enable(pin, true);

    // set to 1
    REG (GPIO_IODIR) |= pin_val;
    break;
  }
}

void cc3_gpio_set_value(uint8_t pin, bool value)
{
  if (pin >= MAX_SERVOS) {
    return;
  }

  if (value) {
    REG (GPIO_IOSET) = _cc3_servo_map[pin];
  } else {
    REG (GPIO_IOCLR) = _cc3_servo_map[pin];
  }
}


bool cc3_gpio_get_value(uint8_t pin)
{
  if (pin >= MAX_SERVOS) {
    return false;
  }

  return REG (GPIO_IOPIN) & _cc3_servo_map[pin];
}


uint8_t cc3_gpio_get_count()
{
  return MAX_SERVOS;
}
