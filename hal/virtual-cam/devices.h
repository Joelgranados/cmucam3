/******************************************************
 *
 * MMC interface routines.
 *
 * (C) 2005 - Tom Walsh tom@openhardware.net
 *
 *
******************************************************/

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


#ifndef INC_DEVICE_TABLE_H
#define INC_DEVICE_TABLE_H

#include <stdint.h>
#include <stdbool.h>

// describes what is found at driver level.
typedef const struct {
  const char	*name;
  // constant to define device type: mmc, etc.
  uint16_t	device_type;
  // device methods for newlib interface.
  int (*open)(const char *name, int flags, int mode);
  int (*close)(int file);
  uint8_t (*read)(int file, void *ptr, size_t len);
  uint8_t (*write)(int file, const void *ptr, size_t len);
  // init the device / software layers.
  int (*init)(void);
  // and the venerable catch-22...
  int (*ioctl)(int file, int cmd, void *ptr);
} DEVICE_TABLE_ENTRY;

// device number is high byte of FILE "pointer".
#define	DEVICE(D)	(D << 8)
#define	DEVICE_TYPE(D)	((D >> 8) & 0xff)

typedef const struct {
  const DEVICE_TABLE_ENTRY * item;
} DEVICE_TABLE_ARRAY;

typedef const DEVICE_TABLE_ARRAY * DEVICE_TABLE_LIST;


#endif

