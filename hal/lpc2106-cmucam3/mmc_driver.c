/******************************************************
 *
 * MMC interface routines.
 *
 * (C) 2005 - Tom Walsh tom@openhardware.net
 *
 *
******************************************************/

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


/********************************************************************
 *
 * This is the device interface for the MMC drive.
 *
********************************************************************/
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>

#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "devices.h"
#include "spi.h"
#include "rdcf2.h"
#include "mmc_hardware.h"

#include <errno.h>
#undef errno
extern int errno;

extern struct DRIVE_DESCRIPTION Drive;

static int mmc_init (void)
{
  // init the drive system & software.
  return initMMCdrive();
}

// demand allocated file control blocks
static struct rdcf *fcbs[MaxFileBuffers];

static struct rdcf *allocate_1_fcb (void)
{
  struct rdcf *fcb = calloc(1, sizeof(struct rdcf));
  if (fcb != NULL) {
    fcb->ReadSector = mmcReadBlock;
    fcb->WriteSector = mmcWriteBlock;
  }

  return fcb;
}

static int8_t allocate_fcb (void)
{
  // find free fcb and return it or -1 if none available
  int i;
  for (i = 0; i < MaxFileBuffers; i++) {
    if (fcbs[i] != NULL) {
      continue;
    }

    // do allocation
    fcbs[i] = allocate_1_fcb();
    if (fcbs[i] == NULL) {
      return -1;
    }
    return i;
  }
  return -1;
}

static bool mmc_recognize (const char *name)
{
  // filename starts with "C:/"
  return name[0] == 'C' && name[1] == ':' && name[2] == '/';
}

static const char *remove_prefix (const char *name)
{
  // take away the "C:/"
  return name + 3;
}

static int mmc_open (const char *name, int flags,
		     int mode __attribute__ ((unused)))
{
  int result;
  int handle;

  mmc_init();

  // is a drive still there?
  if (!DriveDesc.IsValid) {
    errno = ENODEV;
    return -1;
  }
  // find a buffer to use.
  if ((handle = allocate_fcb ()) == -1) {
    errno = ENOBUFS;
    return -1;
  }

  result = rdcf_open (fcbs[handle], remove_prefix(name), flags);
  if (result != 0) {
    free(fcbs[handle]);
    fcbs[handle] = NULL;
    errno = ~result;
    return -1;
  }
  return handle;
}

static int mmc_close (int file)
{
  int result;
  struct rdcf *fcb;

  mmc_init();

  // is a drive still there?
  if (!DriveDesc.IsValid) {
    errno = ENODEV;
    return -1;
  }

  fcb = fcbs[file];
  if (file > MaxFileBuffers || fcb == NULL) {
    errno = EBADF;
    return -1;
  }

  result = rdcf_close (fcb);

  if (result) {
    free(fcb);
    fcbs[file] = NULL;
    errno = ~result;
    return -1;
  }

  free(fcb);
  fcbs[file] = NULL;
  return 0;
}

static ssize_t mmc_read (int file, void *ptr, size_t len)
{
  int result;

  mmc_init();

  // is a drive still there?
  if (!DriveDesc.IsValid) {
    errno = ENODEV;
    return -1;
  }
  result = rdcf_read (fcbs[file], ptr, len);
  if (result < 0) {
    errno = ~result;
    return -1;
  }
  return result;
}

static ssize_t mmc_write (int file, const void *ptr, size_t len)
{
  int result;

  mmc_init();

  // is a drive still there?
  if (!DriveDesc.IsValid) {
    errno = ENODEV;
    return -1;
  }
  result = rdcf_write (fcbs[file], ptr, len);
  if (result < 0) {
    errno = ~result;
    return -1;
  }
  return result;
}

static off_t mmc_lseek (int file, off_t pos, int whence)
{
  int result;

  mmc_init();

  // is a drive still there?
  if (!DriveDesc.IsValid) {
    errno = ENODEV;
    return -1;
  }

  switch (whence) {
  case SEEK_SET:
    result = rdcf_seek (fcbs[file], pos);
    if (result < 0) {
      errno = ~result;
      return -1;
    }
    return fcbs[file]->position;
  case SEEK_CUR:               // not implemented.
  case SEEK_END:               // not implemented.
    break;
  }
  errno = EINVAL;
  return -1;
}

static int mmc_unlink (const char *name)
{
  mmc_init();

  // is a drive still there?
  if (!DriveDesc.IsValid) {
    errno = ENODEV;
    return -1;
  }

  int result;
  struct rdcf *fcb = allocate_1_fcb();
  if (fcb == NULL) {
    return -1;
  }

  result = rdcf_delete (fcb, remove_prefix(name));
  free(fcb);

  if (result < 0) {
    errno = ~result;
    return -1;
  }
  return 0;
}

static int mmc_rename (const char *old, const char *new)
{
  mmc_init();

  // is a drive still there?
  if (!DriveDesc.IsValid) {
    errno = ENODEV;
    return -1;
  }

  int result;
  struct rdcf *fcb = allocate_1_fcb();
  if (fcb == NULL) {
    return -1;
  }

  result = rdcf_rename (fcb, remove_prefix(old), remove_prefix(new));
  free(fcb);

  if (result < 0) {
    errno = ~result;
    return -1;
  }
  return 0;
}

// TODO: activate
static int mmc_flush_dir (int file)
{
  // flush dir entry associated with file.

  mmc_init();

  // is a drive still there?
  if (!DriveDesc.IsValid) {
    errno = ENODEV;
    return -1;
  }

  int result;
  if (file < 0 || file > MaxFileBuffers) {
    return -1;
  }
  result = rdcf_flush_directory (fcbs[file]);
  if (result < 0) {
    errno = ~result;
    return -1;
  }
  return 0;
}

static int mmc_fstat (int file __attribute__((unused)),
		      struct stat *st __attribute__((unused)))
{
  errno = EIO;
  return -1;
}

_cc3_device_driver_t _cc3_mmc_driver = {
  .id        = _CC3_DEVICE_MMC,
  .is_tty    = false,
  .open      = mmc_open,
  .close     = mmc_close,
  .read      = mmc_read,
  .write     = mmc_write,
  .recognize = mmc_recognize,
  .lseek     = mmc_lseek,
  .unlink    = mmc_unlink,
  .rename    = mmc_rename,
  .fstat     = mmc_fstat
};