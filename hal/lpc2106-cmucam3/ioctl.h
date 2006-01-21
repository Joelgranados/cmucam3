#ifndef IOCTL_INC
#define IOCTL_INC

#include <stdio.h>

#define	IOCTL_MMC_SEEK			0x101
#define	IOCTL_MMC_UNLINK		0x102
#define	IOCTL_MMC_RENAME		0x103
#define	IOCTL_MMC_FLUSH_DIR	0x104

int fflushdir (FILE * file);

#endif

