/*-----------------------------------------------------------------------------
  RDCF: A Reentrant DOS-Compatible File System, Version 2.0
  Public Domain - No Restrictions on Use by Philip J. Erdelsky pje@acm.org
  January 15, 1993

  Nov 11, 2005 -- Tom Walsh <tom@openharware.net>
  Adapted for use under gcc + ARM + NewLib.
  -----------------------------------------------------------------------------*/

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


#ifndef INC_RDCF_H
#define INC_RDCF_H

#include <setjmp.h>
#include <stdint.h>
#include <stdbool.h>

/******************************************************
 * Important defines for the configuration of RDCF.
 * uncomment to activate them.
 ******************************************************/
#define RDCF_SECTOR_SIZE	512

// make sure this is not over 256
#define MaxFileBuffers		5


// RDCF_FLUSH_DIR_AFTER_WRITE makes a big performance hit!
// try using the IOCTL_MMC_FLUSH_DIRS on a timer instead.
//#define RDCF_FLUSH_DIR_AFTER_WRITE
//#define _BIG_ENDIAN
//#define RDCF_SLASH_CHAR                       '\\'
#define RDCF_SLASH_CHAR			'/'

/******************************************************
 * end of user defines, change nothing below.
 ******************************************************/

struct rdcf_date_and_time {
  uint8_t second;
  uint8_t minute;
  uint8_t hour;
  uint8_t day;
  uint8_t month;
  uint16_t year;
};

struct rdcf_file_information {
  uint8_t spec[13];
  uint8_t attribute;
#define RDCF_READ_ONLY  0x01
#define RDCF_HIDDEN     0x02
#define RDCF_SYSTEM     0x04
#define RDCF_VOLUME     0x08
#define RDCF_DIRECTORY  0x10
#define RDCF_ARCHIVE    0x20
  struct rdcf_date_and_time date_and_time;
  uint16_t first_cluster;
  uint32_t size;
};

/* Directory structure as it appears on the diskette. */

#define NAME_SIZE       8
#define EXTENSION_SIZE  3
struct directory {
  uint8_t name_extension[NAME_SIZE + EXTENSION_SIZE];
  uint8_t attribute;
  uint8_t reserved[10];
  uint16_t time;
  uint16_t date;
  uint16_t first_cluster;
  uint32_t size;
};

union IO_BUFFER {
  struct directory dir[RDCF_SECTOR_SIZE / sizeof (struct directory)];
  uint16_t fat[RDCF_SECTOR_SIZE / 2];
  uint8_t buf[RDCF_SECTOR_SIZE];
};

/* FCB (File Control Block) */

struct rdcf {
  /* values that must be initialized by the calling program */
  union IO_BUFFER buffer;
  // hardware access.
    bool (*ReadSector) (uint32_t sector, uint8_t * buf);
    bool (*WriteSector) (uint32_t sector, const uint8_t * buf);
  /* file information */
  struct rdcf_file_information file;
  /* result codes */
  uint32_t position;
  uint16_t drive_error;
  int16_t result;
  /* file system information */
  uint8_t drive;
  uint32_t first_FAT_sector;
  uint16_t sectors_per_FAT;
  uint32_t first_directory_sector;
  uint32_t first_data_sector;
  uint32_t sectors_per_cluster;
  uint16_t maximum_cluster_number;
  uint16_t last_cluster_mark;
  /* internal use only */
  uint8_t mode;
  uint16_t directory_first_cluster;
  uint16_t directory_cluster;
  uint16_t directory_index;
  uint8_t buffer_status;
  uint16_t cluster;
  uint16_t last_cluster;
  uint32_t sector_in_buffer;
  jmp_buf error;
};

// description of partion file system is on.
typedef struct {
  // is drive present?
  uint8_t IsValid;
  // How large are the FAT tables?
  uint16_t SectorsPerFAT;
  // Important info to decode FAT entries into sectors.
  uint8_t SectorsPerCluster;
  // Quick reference to BootBlock, if need be.
  uint32_t SectorZero;
  // First File Allocation Table.
  uint32_t FirstFatSector;
  // "backup" copy of the First FAT. usually to undelete files.
  uint32_t SecondFatSector;
  // Where does the actual drive data area start?
  uint32_t RootDirSector;
  // How many entries can be in the root directory?
  uint16_t NumberRootDirEntries;
  // where does data (cluster 2) actually reside?
  uint32_t DataStartSector;
  // What is the last data sector?
  uint32_t MaxDataSector;
  // What is the first free cluster?
  uint32_t FirstPossiblyEmptyCluster;
} DRIVE_DESCRIPTION;

extern DRIVE_DESCRIPTION DriveDesc;


/* modes for rdcf_open() */

#define RDCF_READ   1
#define RDCF_WRITE  2

/* modes for rdcf_sort_directory() */

#define RDCF_EXTENSION_NAME  0
#define RDCF_NAME_EXTENSION  1
#define RDCF_DATE_TIME       2
#define RDCF_SIZE            3
#define RDCF_REVERSE         8

/* prototypes for functions defined in RDCF */

int rdcf_open (struct rdcf *, const char *, unsigned);
int rdcf_close (struct rdcf *);
int rdcf_write (struct rdcf *, const void *, int);
int rdcf_read (struct rdcf *, void *, int);
int rdcf_seek (struct rdcf *, uint32_t);
int rdcf_flush_directory (struct rdcf *);
int rdcf_delete (struct rdcf *, const char *);
int rdcf_rename (struct rdcf *, const char *, const char *);

#if 0
int rdcf_directory (struct rdcf *, const char *);
int rdcf_get_file_information (struct rdcf *, const char *, unsigned);
int rdcf_next_file_information (struct rdcf *);
int rdcf_date_and_time (struct rdcf *, const char *,
                        struct rdcf_date_and_time *);
int rdcf_attribute (struct rdcf *, const char *, unsigned);
int rdcf_get_volume (struct rdcf *);
long rdcf_free_space (struct rdcf *);
#endif


#endif //INC_RDCF_H
