#ifndef MMC_IOCTL_INC
#define MMC_IOCTL_INC

#include <stdio.h>

#define	IOCTL_MMC_SEEK			0x101
#define	IOCTL_MMC_UNLINK		0x102
#define	IOCTL_MMC_RENAME		0x103
#define	IOCTL_MMC_FLUSH_DIR	0x104

int fflushdir (FILE * file);

struct ioctl_seek {
  _off_t * pos;
  int    * whence;
};

struct ioctl_rename {
  const char * oldname;
  const char * newname;
};
    

#endif

