/********************************************************************
 *
 * This is the device interface for the MMC drive.
 *
********************************************************************/
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "ioctl.h"
#include "devices.h"
#include "spi.h"
#include "rdcf2.h"
#include "mmc_hardware.h"

#include <errno.h>
#undef errno
extern int errno;

#define DEVICE_MMC 1

extern struct DRIVE_DESCRIPTION Drive;

// note, last file buffer is reserved
// for special ops such as delete, rename
// dir listings etc.
struct rdcf	fcbs [MaxFileBuffers+1];

static int8_t allocate_fcb (void)
{// find free fcb and return it or -1 if none available.
  int	i;
  for (i=0; i<MaxFileBuffers; i++) {
    if (fcbs[i].BufferInUse) continue;
    return i;
  }
  return -1;
}

static int openFileOnDrive (const char *name, int flags, 
			    int always666 __attribute((unused)))
{
  int	result;
  int	handle;
  
  // is a drive still there?
  if (!DriveDesc.IsValid) { errno = ENODEV; return -1; }
  // find a buffer to use.
  if ((handle = allocate_fcb()) == -1) { errno = ENOBUFS; return -1; }
  result = rdcf_open(&fcbs[handle], name, flags);
  if (result != 0) { errno = ~result; return -1; }
  return DEVICE(DEVICE_MMC) | handle;
}

static int closeFileOnDrive (int file)
{
  int	result;
  // is a drive still there?
  if (!DriveDesc.IsValid) { errno = ENODEV; return -1; }
  result = rdcf_close (&fcbs[file & 0xff]);
  if (result) { errno = ~result; return -1; }
  return 0;
}

static _ssize_t readFromDrive (int file, void *ptr, size_t len)
{
  int	result;
  // is a drive still there?
  if (!DriveDesc.IsValid) { errno = ENODEV; return -1; }
  result = rdcf_read (&fcbs[file & 0xff], ptr, len);
  if (result < 0) { errno = ~result; return -1; }
  return result;
}

static _ssize_t writeToDrive (int file, const void *ptr, size_t len)
{
  int	result;
  // is a drive still there?
  if (!DriveDesc.IsValid) { errno = ENODEV; return -1; }
  result = rdcf_write (&fcbs[file & 0xff], ptr, len);
  if (result < 0) { errno = ~result; return -1; }
  return result;
}

static int ioctl_dos_seek(int file, _off_t pos, int whence)
{
  int	result;
  switch (whence) {
  case SEEK_SET:
    result = rdcf_seek(&fcbs[file & 0xff], pos);
    if (result < 0) { errno = ~result; return -1; }
    return fcbs[file & 0xff].position;
  case SEEK_CUR:		// not implemented.
  case SEEK_END:		// not implemented.
    break;
  }
  errno = EINVAL;
  return -1;
}

static int ioctl_dos_unlink (char * name)
{
  int	result;
  result = rdcf_delete (&fcbs[MaxFileBuffers], name);
  if (result < 0) { errno = ~result; return -1; }
  return 0;
}

static int ioctl_dos_rename (const char * old, const char * new)
{
  int	result;
  result = rdcf_rename(&fcbs[MaxFileBuffers], old, new);
  if (result < 0) { errno = ~result; return -1; }
  return 0;
}

static int ioctl_dos_flush_dir (int file)
{// flush dir entry associated with file.
  int	result;
  if ((file & 0xff) < 0 || (file & 0xff) >= MaxFileBuffers) {
    return -1;
  }
  result = rdcf_flush_directory(&fcbs[file & 0xff]);
  if (result < 0) { errno = ~result; return -1; }
  return 0;
}


static int ioctlDrive (int file, int cmd, void *ptr)
{
  // is a drive still there?
  if (!DriveDesc.IsValid) { errno = ENODEV; return -1; }
  switch (cmd) {
  case IOCTL_MMC_SEEK:
    // ptr is to a structure containing two pointers to pointers.
    return ioctl_dos_seek (file, *(_off_t *)((long *)ptr)[0], *(int *)((long *)ptr)[1]);
  case IOCTL_MMC_UNLINK:
    // ptr is the address of a pointer to filename string.
    return ioctl_dos_unlink ((char *)((long *)ptr)[0]);
  case IOCTL_MMC_RENAME:
    return ioctl_dos_rename ((const char *)((long *)ptr)[0], (const char *)((long *)ptr)[1]);
  case IOCTL_MMC_FLUSH_DIR:
    return ioctl_dos_flush_dir(file);
  }
  // anything not implemented is "INVALID".
  errno = EINVAL;
  return -1;
}

static int initTheDrive (void)
{
  // initialize the SPI0 controller.
  cc3_spi0_init();
  // init the drive system & software.
  return initMMCdrive ();
}


DEVICE_TABLE_ENTRY mmc_driver = {
  "mmc",	// do not change, this is a reserved device name.
  DEVICE_MMC,
  openFileOnDrive,
  closeFileOnDrive,
  readFromDrive,
  writeToDrive,
  initTheDrive,
  ioctlDrive	
};

// vi:nowrap

