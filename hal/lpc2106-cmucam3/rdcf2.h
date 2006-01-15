
/*-----------------------------------------------------------------------------
          RDCF: A Reentrant DOS-Compatible File System, Version 2.0

                          by Philip J. Erdelsky
                               pje@acm.org

                           September 15, 1992

                  Copyright (c) 1992 Philip J. Erdelsky
-----------------------------------------------------------------------------*/

#ifndef INC_RDCF_H
#define INC_RDCF_H

#include <setjmp.h>
#include <types.h>
#include <sysdefs.h>

/******************************************************
 * Important defines for the configuration of RDCF.
 * uncomment to activate them.
******************************************************/

#define RDCF_SECTOR_SIZE	512
#define MaxFileBuffers		5
// RDCF_FLUSH_DIR_AFTER_WRITE makes a big performance hit!
// try using the IOCTL_MMC_FLUSH_DIRS on a timer instead.
//#define RDCF_FLUSH_DIR_AFTER_WRITE
//#define _BIG_ENDIAN
//#define RDCF_SLASH_CHAR			'\\'
#define RDCF_SLASH_CHAR			'/'

/******************************************************
 * end of user defines, change nothing below.
******************************************************/

struct rdcf_date_and_time
{
  uchar second;
  uchar minute;
  uchar hour;
  uchar day;
  uchar month;
  ushort year;
};

struct rdcf_file_information
{
  uchar spec[13];
  uchar attribute;
    #define RDCF_READ_ONLY  0x01
    #define RDCF_HIDDEN     0x02
    #define RDCF_SYSTEM     0x04
    #define RDCF_VOLUME     0x08
    #define RDCF_DIRECTORY  0x10
    #define RDCF_ARCHIVE    0x20
  struct rdcf_date_and_time date_and_time;
  ushort first_cluster;
  ulong size;
};

/* Directory structure as it appears on the diskette. */

#define NAME_SIZE       8
#define EXTENSION_SIZE  3
struct directory
{
  uchar name_extension[NAME_SIZE+EXTENSION_SIZE];
  uchar attribute;
  uchar reserved[10];
  ushort time;
  ushort date;
  ushort first_cluster;
  ulong size;
};

union IO_BUFFER {
	struct directory dir[RDCF_SECTOR_SIZE / sizeof(struct directory)];
	ushort fat[RDCF_SECTOR_SIZE/2];
	uchar  buf[RDCF_SECTOR_SIZE];
};

/* FCB (File Control Block) */

struct rdcf
{
		/* values that must be initialized by the calling program */
	union 	IO_BUFFER buffer;
	bool		BufferInUse;
		// hardware access.
	bool	(*ReadSector)(long sector, uchar * buf);
	bool	(*WriteSector)(long sector, const uchar * buf);
		/* file information */
	struct 	rdcf_file_information file;
		/* result codes */
	ulong		position;
	ushort	drive_error;
	short		result;
		/* file system information */
	uchar		drive;
	ulong		first_FAT_sector;
	ushort	sectors_per_FAT;
	ulong		first_directory_sector;
	ulong		first_data_sector;
	uint		sectors_per_cluster;
	ushort	maximum_cluster_number;
	ushort	last_cluster_mark;
		/* internal use only */
	uchar		mode;
	ushort	directory_first_cluster;
	ushort	directory_cluster;
	ushort	directory_index;
	uchar		buffer_status;
	ushort	cluster;
	ushort	last_cluster;
	ulong		sector_in_buffer;
	jmp_buf error;
};

	// description of partion file system is on.
typedef struct {
		// is drive present?
	uchar		IsValid;
		// How large are the FAT tables?
	ushort	SectorsPerFAT;
		// Important info to decode FAT entries into sectors.
	uchar		SectorsPerCluster;
		// Quick reference to BootBlock, if need be.
	unsigned		SectorZero;
		// First File Allocation Table.
	unsigned		FirstFatSector;
		// "backup" copy of the First FAT. usually to undelete files.
	unsigned		SecondFatSector;
		// Where does the actual drive data area start?
	unsigned		RootDirSector;
		// How many entries can be in the root directory?
	ushort	NumberRootDirEntries;
		// where does data (cluster 2) actually reside?
	unsigned		DataStartSector;
		// What is the last data sector?
	unsigned		MaxDataSector;
} DRIVE_DESCRIPTION;

extern DRIVE_DESCRIPTION DriveDesc;


/* modes for rdcf_open() */

#define RDCF_READ			1
#define RDCF_WRITE		2

/* modes for rdcf_sort_directory() */

#define RDCF_EXTENSION_NAME  0
#define RDCF_NAME_EXTENSION  1
#define RDCF_DATE_TIME       2
#define RDCF_SIZE            3
#define RDCF_REVERSE         8

/* prototypes for functions defined in RDCF */

int rdcf_open(struct rdcf *, const char *, unsigned);
int rdcf_close(struct rdcf *);
int rdcf_write(struct rdcf *, const void *, int);
int rdcf_read(struct rdcf *, void *, int);
int rdcf_seek(struct rdcf *, ulong);
int rdcf_flush_directory(struct rdcf *);
int rdcf_delete(struct rdcf *, const char *);
int rdcf_rename(struct rdcf *, const char *, const char *);

#if 0
int rdcf_directory(struct rdcf *, const char *);
int rdcf_get_file_information(struct rdcf *, const char *, unsigned);
int rdcf_next_file_information(struct rdcf *);
int rdcf_date_and_time(struct rdcf *, const char *, struct rdcf_date_and_time *);
int rdcf_attribute(struct rdcf *, const char *, unsigned);
int rdcf_get_volume(struct rdcf *);
long rdcf_free_space(struct rdcf *);
#endif


#endif	//INC_RDCF_H
