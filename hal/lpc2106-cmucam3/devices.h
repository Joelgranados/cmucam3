/*
 * Copyright 2006  Anthony Rowe and Adam Goode
 */

/******************************************************
 *
 * MMC interface routines.
 *
 * (C) 2005 - Tom Walsh tom@openhardware.net
 *
 *
******************************************************/

/*
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
  _ssize_t (*read)(int file, void *ptr, size_t len);
  _ssize_t (*write)(int file, const void *ptr, size_t len);
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

