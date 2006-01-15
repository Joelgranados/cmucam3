/********************************************************************
 *
 * This is the device interface for the MMC drive.
 *
********************************************************************/
#include <stdio.h>
#include <fcntl.h>
#include <reent.h>
#include <errno.h>
#include <devices.h>
#include <spi.h>
#include <ioctl.h>
#include <rdcf2.h>
#include <sysdefs.h>
#include <string.h>
#include <mmc_hardware.h>
#include <sys/errno.h>


extern struct DRIVE_DESCRIPTION Drive;

// note, last file buffer is reserved
// for special ops such as delete, rename
// dir listings etc.
struct rdcf	fcbs [MaxFileBuffers+1];

static uchar allocate_fcb (void)
{// find free fcb and return it or 0 if none available.
int	i;
	for (i=0; i<MaxFileBuffers; i++) {
		if (fcbs[i].BufferInUse) continue;
		return i;
	}
	return -1;
}

static int openFileOnDrive (struct _reent *r, const char *name, int flags, int always666)
{
int	result;
unsigned	handle;
		// is a drive still there?
	if (!DriveDesc.IsValid) { r->_errno = ENODEV; return -1; }
		// find a buffer to use.
	if ((handle = allocate_fcb()) == -1) { r->_errno = ENOBUFS; return -1; }
	result = rdcf_open(&fcbs[handle], name, flags);
	if (result != 0) { r->_errno = ~result; return -1; }
	return DEVICE(DEVICE_MMC) | handle;
}

static int closeFileOnDrive (struct _reent *r, int file)
{
int	result;
		// is a drive still there?
	if (!DriveDesc.IsValid) { r->_errno = ENODEV; return -1; }
	result = rdcf_close (&fcbs[file & 0xff]);
	if (result) { r->_errno = ~result; return -1; }
	return 0;
}

static _ssize_t readFromDrive (struct _reent *r, int file, void *ptr, size_t len)
{
int	result;
		// is a drive still there?
	if (!DriveDesc.IsValid) { r->_errno = ENODEV; return -1; }
	result = rdcf_read (&fcbs[file & 0xff], ptr, len);
	if (result < 0) { r->_errno = ~result; return -1; }
	return result;
}

static _ssize_t writeToDrive (struct _reent *r, int file, const void *ptr, size_t len)
{
int	result;
		// is a drive still there?
	if (!DriveDesc.IsValid) { r->_errno = ENODEV; return -1; }
	result = rdcf_write (&fcbs[file & 0xff], ptr, len);
	if (result < 0) { r->_errno = ~result; return -1; }
	return result;
}

static int ioctl_dos_seek(struct _reent * r, int file, _off_t pos, int whence)
{
int	result;
	switch (whence) {
		case SEEK_SET:
			result = rdcf_seek(&fcbs[file & 0xff], pos);
			if (result < 0) { r->_errno = ~result; return -1; }
			return fcbs[file & 0xff].position;
		case SEEK_CUR:		// not implemented.
		case SEEK_END:		// not implemented.
			break;
	}
	r->_errno = EINVAL;
	return -1;
}

static int ioctl_dos_unlink (struct _reent *r, char * name)
{
int	result;
	result = rdcf_delete (&fcbs[MaxFileBuffers], name);
	if (result < 0) { r->_errno = ~result; return -1; }
	return 0;
}

static int ioctl_dos_rename (struct _reent *r, const char * old, const char * new)
{
int	result;
	result = rdcf_rename(&fcbs[MaxFileBuffers], old, new);
	if (result < 0) { r->_errno = ~result; return -1; }
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


static int ioctlDrive (struct _reent *r, int file, int cmd, void *ptr)
{
		// is a drive still there?
	if (!DriveDesc.IsValid) { r->_errno = ENODEV; return -1; }
	switch (cmd) {
		case IOCTL_MMC_SEEK:
				// ptr is to a structure containing two pointers to pointers.
			return ioctl_dos_seek (r, file, *(_off_t *)((long *)ptr)[0], *(int *)((long *)ptr)[1]);
		case IOCTL_MMC_UNLINK:
				// ptr is the address of a pointer to filename string.
			return ioctl_dos_unlink (r, (char *)((long *)ptr)[0]);
		case IOCTL_MMC_RENAME:
			return ioctl_dos_rename (r, (const char *)((long *)ptr)[0], (const char *)((long *)ptr)[1]);
		case IOCTL_MMC_FLUSH_DIR:
			return ioctl_dos_flush_dir(file);
	}
		// anything not implemented is "INVALID".
	r->_errno = EINVAL;
	return -1;
}

static int initTheDrive (void)
{
		// initialize the SPI0 controller.
	spi1Init();
		// init the drive system & software.
	return initMMCdrive ();
}

DEVICE_TABLE_ENTRY mmc_driver = {
	"mmc",	// do not change, this is a reserved device name.
	DEVICE_MMC,
#ifdef HAS_MMC
	openFileOnDrive,
	closeFileOnDrive,
	readFromDrive,
	writeToDrive,
	initTheDrive,
	ioctlDrive	
#else
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
#endif
};

// vi:nowrap

