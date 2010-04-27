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


#ifndef DEVICES_H
#define DEVICES_H

#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

typedef enum {
  _CC3_DEVICE_UART = 0,  // must be zero for normal stdin/out/err to work
  _CC3_DEVICE_MMC,

  _CC3_NUM_DEVICES
} _cc3_device_t;

typedef const struct {
  _cc3_device_t id;
  bool    (*recognize) (const char *name);
  bool    is_tty;

  int     (*open)      (const char *name, int flags, int mode);
  int     (*close)     (int file);
  ssize_t (*read)      (int file, void *ptr, size_t len);
  ssize_t (*write)     (int file, const void *ptr, size_t len);
  off_t   (*lseek)     (int file, off_t offset, int dir);
  int     (*unlink)    (const char *name);
  int     (*rename)    (const char *oldname, const char *newname);
  int     (*fstat)     (int file, struct stat *st);
} _cc3_device_driver_t;


// the device drivers
extern _cc3_device_driver_t _cc3_uart_driver;
extern _cc3_device_driver_t _cc3_mmc_driver;


_cc3_device_driver_t *_cc3_get_driver_for_name (const char *name);
uint8_t _cc3_get_internal_file_number (const int file);
_cc3_device_driver_t *_cc3_get_driver_for_file_number (const int file);
int _cc3_make_file_number (const _cc3_device_driver_t *dev,
			   const int16_t file);

#endif

