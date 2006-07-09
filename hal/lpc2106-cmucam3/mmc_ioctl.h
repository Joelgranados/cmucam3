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


#ifndef MMC_IOCTL_INC
#define MMC_IOCTL_INC

#include <stdio.h>

#define	IOCTL_MMC_SEEK			0x101
#define	IOCTL_MMC_UNLINK		0x102
#define	IOCTL_MMC_RENAME		0x103
#define	IOCTL_MMC_FLUSH_DIR	0x104

int fflushdir (FILE * file);

struct ioctl_seek {
  _off_t *pos;
  int *whence;
};

struct ioctl_rename {
  const char *oldname;
  const char *newname;
};


#endif
