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
