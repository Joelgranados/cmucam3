/*-----------------------------------------------------------------------------
          RDCF: A Reentrant DOS-Compatible File System, Version 2.0
   Public Domain - No Restrictions on Use by Philip J. Erdelsky pje@acm.org
                               January 15, 1993

 Nov 11, 2005 -- Tom Walsh <tom@openharware.net>
    Adapted for use under gcc + ARM + NewLib.
-----------------------------------------------------------------------------*/

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


/* #define _BIG_ENDIAN */

#include "rdcf2.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <sys/errno.h>
#include <sys/fcntl.h>


#define SECTOR_SIZE RDCF_SECTOR_SIZE

#define ENTRIES_PER_SECTOR  (SECTOR_SIZE/sizeof(struct directory))
#define NO_DIRECTORY_INDEX  0xFFFF

/* Special values for first byte of name. */

#define END_DIRECTORY     0x00
#define DELETED_FILE      0xE5

/* Special values for FAT entries. */

#define EMPTY_CLUSTER            0
#define RESERVED_CLUSTER_12_BIT  0xFF0
#define LAST_CLUSTER_12_BIT      0xFF8
#define LAST_CLUSTER_16_BIT      0xFFF8

/* buffer status */

enum buffer_status { EMPTY, CLEAN, DIRTY };

/* additional mode bits */

#define APPEND     (1<<6)
#define WRITTEN    (1<<7)

/*-----------------------------------------------------------------------------
Layout of first sector when read into buffer[].  A simple structure would not
be portable because some words are not aligned on even addresses.
-----------------------------------------------------------------------------*/

#define BYTES_PER_SECTOR     (buffer[11]|buffer[12]<<8)
#define SECTORS_PER_CLUSTER   buffer[13]
#define RESERVED_SECTORS     (buffer[14]|buffer[15]<<8)
#define NUMBER_OF_FATS        buffer[16]
#define ROOT_ENTRIES         (buffer[17]|buffer[18]<<8)
#define TOTAL_SECTORS        (buffer[19]|buffer[20]<<8)
#define MEDIA_DESCRIPTOR      buffer[21]
#define SECTORS_PER_FAT      (buffer[22]|buffer[23]<<8)
#define SECTORS_PER_TRACK    (buffer[24]|buffer[25]<<8)
#define HEADS                (buffer[26]|buffer[27]<<8)
#define HIDDEN_SECTORS       (buffer[28]|buffer[29]<<8)

/*-----------------------------------------------------------------------------
The following functions and macros convert words and double words from "big
endian" to "little endian" form or vice-versa.
-----------------------------------------------------------------------------*/

#ifdef _BIG_ENDIAN

static void swap_two (uint8_t * p)
{
  uint8_t x = p[0];
  p[0] = p[1];
  p[1] = x;
}

static void swap_four (uint8_t * p)
{
  uint8_t x = p[0];
  p[0] = p[3];
  p[3] = x;
  swap_two (p + 1);
}

#define convert_short(x) swap_two((uint8_t *)(&(x)))
#define convert_long(x)  swap_four((uint8_t *)(&(x)))

#endif

static void rdcf_get_date_and_time (struct rdcf_date_and_time *p)
{
  struct tm dateTime;
  struct timeval temp;
  time_t now;

  if (gettimeofday (&temp, 0) == 0) {
    // time is correct
    now = temp.tv_sec;
    localtime_r (&now, &dateTime);

    p->hour = dateTime.tm_hour;
    p->minute = dateTime.tm_min;
    p->second = dateTime.tm_sec;
    p->month = dateTime.tm_mon + 1;
    p->day = dateTime.tm_mday;
    p->year = dateTime.tm_year + 1900;
  } else {
    // time is incorrect, apply "the notorious 1-1-80 00:00:00"
    p->hour = 0;
    p->minute = 0;
    p->second = 0;
    p->month = 1;
    p->day = 1;
    p->year = 1980;
  }
}

/*-----------------------------------------------------------------------------
This function calls longjmp() to specify an error code and exit from the RDCF
function originally called.
-----------------------------------------------------------------------------*/

static void error_exit (struct rdcf *f, int error)
{
  longjmp (f->error, error);
}

/*-----------------------------------------------------------------------------
These macros make the calls on access_sector() more readable.
-----------------------------------------------------------------------------*/

static void read_sector (struct rdcf *f, unsigned sector, void *buffer)
{
  f->drive_error = f->ReadSector (sector, buffer);
  if (f->drive_error != 0)
    error_exit (f, ~EIO);
}

static void write_sector (struct rdcf *f, unsigned sector,
                          const uint8_t * buffer)
{
  f->drive_error = f->WriteSector (sector, buffer);
  if (f->drive_error != 0)
    error_exit (f, ~EIO);
}

/*-----------------------------------------------------------------------------
This function writes the buffer in the FCB if it is marked as "dirty".
-----------------------------------------------------------------------------*/

static void flush_buffer (struct rdcf *f)
{
  if (f->buffer_status == DIRTY) {
    // here is where we keep from thrashing while doing FAT operations.
    // if the sector to be written is in the first FAT table, then
    // mirror the write to corresponding sector in second FAT.
    write_sector (f, f->sector_in_buffer, f->buffer.buf);
    if ((f->sector_in_buffer >= f->first_FAT_sector) &&
        (f->sector_in_buffer <= f->first_FAT_sector + f->sectors_per_FAT)) {
      // mirror.
      write_sector (f, f->sector_in_buffer + f->sectors_per_FAT,
                    f->buffer.buf);
    }
    f->buffer_status = CLEAN;
  }
}

/*-----------------------------------------------------------------------------
This function reads a sector into the buffer in the FCB, if it is not already
there.  If another sector is there, the buffer is first flushed.
-----------------------------------------------------------------------------*/

static void read_buffer (struct rdcf *f, unsigned sector)
{
  if (f->buffer_status == EMPTY || sector != f->sector_in_buffer) {
    flush_buffer (f);
    read_sector (f, sector, f->buffer.buf);
    f->sector_in_buffer = sector;
    f->buffer_status = CLEAN;
  }
}

/*-----------------------------------------------------------------------------
This function checks to see if a cluster number is valid and declares an error
if it is not.
-----------------------------------------------------------------------------*/

static void check_cluster (struct rdcf *f, unsigned cluster)
{
  if (cluster < 2 || cluster > f->maximum_cluster_number)
    error_exit (f, ~ESPIPE);
}

/*-----------------------------------------------------------------------------
This function returns the FAT entry for the specified cluster.
Reads from first copy of FAT.
-----------------------------------------------------------------------------*/

static unsigned FAT_entry (struct rdcf *f, unsigned cluster)
{
  check_cluster (f, cluster);
  if (f->maximum_cluster_number < RESERVED_CLUSTER_12_BIT) {
    unsigned byte_index = cluster + (cluster >> 1);
    uint8_t p[2];
    read_buffer (f, f->first_FAT_sector + byte_index / SECTOR_SIZE);
    p[0] = f->buffer.buf[byte_index % SECTOR_SIZE];
    byte_index++;
    read_buffer (f, f->first_FAT_sector + byte_index / SECTOR_SIZE);
    p[1] = f->buffer.buf[byte_index % SECTOR_SIZE];
    return (cluster & 1) ? (p[1] << 4 | p[0] >> 4) : (p[0] | p[1]) << 8 &
      0xF00;
  }
  else {
    ushort x;
    read_buffer (f, f->first_FAT_sector + cluster / (SECTOR_SIZE / 2));
    x = f->buffer.fat[cluster % (SECTOR_SIZE / 2)];
#ifdef _BIG_ENDIAN
    convert_short (x);
#endif
    return x;
  }
}

/*-----------------------------------------------------------------------------
This function sets the FAT entry for the specified cluster to the specified
value.  The 12-bit FAT entry always occupies two consecutive bytes, filling one
byte completely and filling only one nibble of the other byte.  Since these
bytes may be in different sectors, two separate calls on read_buffer() are
used.  The one-sector caching implemented by read_buffer() prevents multiple
disk accesses when both bytes are in the same sector.  Every copy of the FAT is
updated in the same way.
-----------------------------------------------------------------------------*/

static void set_FAT_entry (struct rdcf *f, unsigned cluster, unsigned x)
{
  unsigned sector;
  check_cluster (f, cluster);
#ifdef _BIG_ENDIAN
  if (f->maximum_cluster_number >= RESERVED_CLUSTER_12_BIT)
    convert_short (x);
#endif
  sector = f->first_FAT_sector;
  if (f->maximum_cluster_number < RESERVED_CLUSTER_12_BIT) {
    unsigned byte_index = cluster + (cluster >> 1);
    uint8_t *p;
    read_buffer (f, sector + byte_index / SECTOR_SIZE);
    p = f->buffer.buf + byte_index % SECTOR_SIZE;
    *p = (cluster & 1) ? (*p & 0x0F) | (x << 4) : x;
    f->buffer_status = DIRTY;
    read_buffer (f, sector + (byte_index + 1) / SECTOR_SIZE);
    p = f->buffer.buf + (byte_index + 1) % SECTOR_SIZE;
    *p = (cluster & 1) ? x >> 4 : (*p & 0xF0) | (x >> 8);
  }
  else {
    read_buffer (f, sector + cluster / (SECTOR_SIZE / 2));
    f->buffer.fat[cluster % (SECTOR_SIZE / 2)] = x;
  }
  f->buffer_status = DIRTY;
}

/*-----------------------------------------------------------------------------
This function checks the value of c (which is always in the range from 0 to
255, inclusive). If it represents a character that cannot appear in a valid
file name or extension, it bails out.
-----------------------------------------------------------------------------*/

static void check_file_character (struct rdcf *f, unsigned c)
{
  static uint8_t table[32] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0x05, 0xDC, 0x00, 0xFC,
    0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x90,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
  };
  if (table[c >> 3] & 1 << (c & 7))
    error_exit (f, ~EINVAL);
}

/*-----------------------------------------------------------------------------
This function edits a file or directory spec into the name-extension form used
in the file directory entry.  It returns a pointer to the character
following the last one of the spec.
-----------------------------------------------------------------------------*/

static const char *spec_to_name_extension (struct rdcf *f,
                                           uint8_t * name_extension,
                                           const uint8_t * spec)
{
  unsigned i = 0;
  unsigned c;
  while ((c = (*spec++)) != 0 && c != RDCF_SLASH_CHAR && c != '.') {
    check_file_character (f, c);
    if (i < NAME_SIZE)
      name_extension[i++] = toupper (c);
  }
  if (i == 0)
    error_exit (f, ~EINVAL);
  while (i < NAME_SIZE)
    name_extension[i++] = ' ';
  if (c == '.')
    while ((c = (*spec++)) != 0 && c != RDCF_SLASH_CHAR) {
      check_file_character (f, c);
      if (i < NAME_SIZE + EXTENSION_SIZE)
        name_extension[i++] = toupper (c);
    }
  while (i < NAME_SIZE + EXTENSION_SIZE)
    name_extension[i++] = ' ';
  return spec - 1;
}

/*-----------------------------------------------------------------------------
This function edits the name-extension form used in a file entry to file spec.
-----------------------------------------------------------------------------*/

static void name_extension_to_spec (uint8_t * spec,
                                    const uint8_t * name_extension)
{
  unsigned i;
  uint8_t *s = spec;
  for (i = 0; i < NAME_SIZE && name_extension[i] != ' '; i++)
    *s++ = name_extension[i];
  if (name_extension[NAME_SIZE] != ' ') {
    *s++ = '.';
    for (i = NAME_SIZE;
         i < NAME_SIZE + EXTENSION_SIZE && name_extension[i] != ' '; i++) {
      *s++ = name_extension[i];
    }
  }
  *s = 0;
}

/*-----------------------------------------------------------------------------
This function calculates the number of the first sector in the specified
cluster.
-----------------------------------------------------------------------------*/

static unsigned first_sector_in_cluster (struct rdcf *f, unsigned cluster)
{
  check_cluster (f, cluster);
  return f->first_data_sector + (cluster - 2) * f->sectors_per_cluster;
}

/*-----------------------------------------------------------------------------
This function finds the directory entry referred to by f->directory_cluster and
f->directory_index, reads it into f->buffer, and returns a pointer to it.
-----------------------------------------------------------------------------*/

static struct directory *find_directory (struct rdcf *f)
{
  read_buffer (f, (f->directory_cluster == 0 ? f->first_directory_sector :
                   first_sector_in_cluster (f, f->directory_cluster)) +
               f->directory_index / ENTRIES_PER_SECTOR);
  return &f->buffer.dir[f->directory_index % ENTRIES_PER_SECTOR];
}

/*-----------------------------------------------------------------------------
This function updates a directory entry.  If the "delete_entry" parameter is
true (nonzero), it also marks the entry as deleted.
-----------------------------------------------------------------------------*/

static void update_directory_entry (struct rdcf *f, int delete_entry)
{
  struct directory *d = find_directory (f);
  if (f->file.attribute & RDCF_VOLUME || f->file.spec[0] == '.')
    memcpy (d->name_extension, f->file.spec, NAME_SIZE + EXTENSION_SIZE);
  else
    spec_to_name_extension (f, d->name_extension, f->file.spec);
  if (delete_entry)
    d->name_extension[0] = DELETED_FILE;
  d->attribute = f->file.attribute;
  d->date = (f->file.date_and_time.year - 1980) << 9 |
    f->file.date_and_time.month << 5 | f->file.date_and_time.day;
  d->time = f->file.date_and_time.hour << 11 |
    f->file.date_and_time.minute << 5 | f->file.date_and_time.second >> 1;
  d->first_cluster = f->file.first_cluster;
  d->size = f->file.size;
  memset (d->reserved, 0, sizeof (d->reserved));
#ifdef _BIG_ENDIAN
  convert_short (d->date);
  convert_short (d->time);
  convert_short (d->first_cluster);
  convert_long (d->size);
#endif
  f->buffer_status = DIRTY;
}

/*-----------------------------------------------------------------------------
This function reads directory information into an FCB.
-----------------------------------------------------------------------------*/

static void read_directory_entry (struct rdcf *f)
{
  struct directory *d = find_directory (f);
  if (d->attribute & RDCF_VOLUME) {
    memcpy (f->file.spec, d->name_extension, NAME_SIZE + EXTENSION_SIZE);
    f->file.spec[NAME_SIZE + EXTENSION_SIZE] = 0;
  }
  else
    name_extension_to_spec (f->file.spec, d->name_extension);
  f->file.attribute = d->attribute;
  {
    ushort date = d->date;
#ifdef _BIG_ENDIAN
    convert_short (date);
#endif
    f->file.date_and_time.year = (date >> 9) + 1980;
    f->file.date_and_time.month = date >> 5 & 0xF;
    f->file.date_and_time.day = date & 0x1F;
  }
  {
    ushort aTime = d->time;
#ifdef _BIG_ENDIAN
    convert_short (time);
#endif
    f->file.date_and_time.hour = aTime >> 11;
    f->file.date_and_time.minute = aTime >> 5 & 0x3F;
    f->file.date_and_time.second = aTime << 1 & 0x1F << 1;
  }
  f->file.first_cluster = d->first_cluster;
  f->file.size = d->size;
#ifdef _BIG_ENDIAN
  convert_short (f->file.first_cluster);
  convert_long (f->file.size);
#endif
}

/*-----------------------------------------------------------------------------
If the parameter "name_extension" is not NULL, this function looks up the name
and extension in the directory specified by f->directory_cluster (the entire
root directory if f->directory_cluster == 0). If found, it puts the appropriate
information into the file structure and returns a true (nonzero) result. If not
found, it returns a false (zero) value and also puts the cluster and index of
the first empty entry, if any, into f->directory_cluster and
f->directory_index.  In any case, the name and extension are left in
f->file.spec, and the first directory cluster is left in
f->directory_first_cluster.

If the parameter name_extension is NULL, this function looks up the volume
label in the same way (although in this case the calling function looks only in
the root directory, of course).

If the file or volume label is not found and there is no empty entry, the
special value NO_DIRECTORY_INDEX is left in f->directory_index.
-----------------------------------------------------------------------------*/

static int find_file_in_directory_or_find_volume (struct rdcf *f,
                                                  const uint8_t *
                                                  name_extension)
{
  unsigned empty_cluster = 2;
  unsigned empty_index = NO_DIRECTORY_INDEX;
  unsigned number_of_directory_entries = f->directory_cluster == 0 ?
    (f->first_data_sector - f->first_directory_sector) * ENTRIES_PER_SECTOR :
    ENTRIES_PER_SECTOR * f->sectors_per_cluster;
  f->directory_first_cluster = f->directory_cluster;
  while (1) {
    for (f->directory_index = 0;
         f->directory_index < number_of_directory_entries;
         f->directory_index++) {
      struct directory *d = find_directory (f);
      if ((d->name_extension[0] == DELETED_FILE ||
           d->name_extension[0] == END_DIRECTORY) &&
          empty_index == NO_DIRECTORY_INDEX) {
        empty_cluster = f->directory_cluster;
        empty_index = f->directory_index;
      }
      if (d->name_extension[0] == END_DIRECTORY)
        break;
      if ((name_extension == NULL &&
           (d->name_extension[0] != DELETED_FILE
            && d->attribute & RDCF_VOLUME)) || (name_extension != NULL
                                                &&
                                                (memcmp
                                                 (d->name_extension,
                                                  name_extension,
                                                  NAME_SIZE + EXTENSION_SIZE)
                                                 == 0
                                                 && (d->
                                                     attribute & RDCF_VOLUME)
                                                 == 0))) {
        read_directory_entry (f);
        return 1;
      }
    }
    if (f->directory_index < number_of_directory_entries ||
        f->directory_cluster == 0) {
      break;
    }
    {
      unsigned x = FAT_entry (f, f->directory_cluster);
      if (x >= f->last_cluster_mark)
        break;
      f->directory_cluster = x;
    }
  }
  f->directory_index = empty_index;
  if (f->directory_index != NO_DIRECTORY_INDEX)
    f->directory_cluster = empty_cluster;
  if (name_extension != NULL)
    name_extension_to_spec (f->file.spec, name_extension);
  return 0;
}

/*-----------------------------------------------------------------------------
This function follows the directory path and finds the directory entry for the
file (or directory) with the spec provided.  If found, it reads the directory
information into the FCB and returns a true (nonzero) value.  Otherwise, it
returns a false (zero) value and also puts the cluster and index of the first
available directory entry, if any, into f->directory_cluster and
f->directory_index.  If there is no available directory entry, it puts the
special value NO_DIRECTORY_INDEX into f->directory_index. In any case, the
number of the first cluster of the directory is left in
f->directory_first_cluster and the name and extension are left in f->file.spec.
-----------------------------------------------------------------------------*/

static int find_file (struct rdcf *f, const char *spec)
{
  /* start with root directory */
  f->directory_cluster = 0;
  while (1) {
    int found;
    uint8_t name_extension[NAME_SIZE + EXTENSION_SIZE];
    /* scan name and extension */
    spec = spec_to_name_extension (f, name_extension, spec);
    /* look it up in directory */
    found = find_file_in_directory_or_find_volume (f, name_extension);
    /* if this is the end of the file specification, return */
    if (*spec == 0)
      return found;
    /* otherwise, the name and extension were a subdirectory in the path */
    if (!found || (f->file.attribute & RDCF_DIRECTORY) == 0)
      error_exit (f, ~EISDIR);
    f->directory_cluster = f->file.first_cluster;
    /* skip over the \ after the subdirectory */
    spec++;
  }
}

/*-----------------------------------------------------------------------------
This function refers to data alread read from the file system partition info.
-----------------------------------------------------------------------------*/

static void read_file_system_information (struct rdcf *f)
{
  f->first_FAT_sector = DriveDesc.FirstFatSector;
  f->sectors_per_FAT = DriveDesc.SectorsPerFAT;
  f->sectors_per_cluster = DriveDesc.SectorsPerCluster;
  f->first_directory_sector = DriveDesc.RootDirSector;
  f->first_data_sector = DriveDesc.DataStartSector;
  f->maximum_cluster_number =
    ((DriveDesc.MaxDataSector - DriveDesc.DataStartSector) /
     DriveDesc.SectorsPerCluster) + 1;
  f->last_cluster_mark =
    f->maximum_cluster_number < RESERVED_CLUSTER_12_BIT ?
    LAST_CLUSTER_12_BIT : LAST_CLUSTER_16_BIT;
}

/*-----------------------------------------------------------------------------
This function gets the drive specifications and returns a pointer to the
character following the drive specifications.
-----------------------------------------------------------------------------*/

#ifdef RDCF_MULTIPLE_DRIVE
#error "multiple drives has not been implemented!"
static char *get_drive (struct rdcf *f, char *spec)
{
  if (spec[0] != 0 && spec[1] == ':') {
    if (isalpha (spec[0]))
      f->drive = toupper (spec[0]) - 'A';
    else
      error_exit (f, ~EINVAL);
    return spec + 2;
  }
  error_exit (f, ~EINVAL);
}
#endif

/*-----------------------------------------------------------------------------
This function scans the file spec and sets up the file control block for
further file operations.  It returns a pointer to the character following the
drive specifications.
-----------------------------------------------------------------------------*/

static const char *initialize_fcb (struct rdcf *f, const char *spec)
{
  f->buffer_status = EMPTY;
  read_file_system_information (f);
  return spec;
}

/*-----------------------------------------------------------------------------
This function checks write access and generates an error if write access is
denied.
-----------------------------------------------------------------------------*/

static void check_write_access (struct rdcf *f)
{
  if (f->file.attribute & (RDCF_READ_ONLY + RDCF_HIDDEN + RDCF_SYSTEM))
    error_exit (f, ~EACCES);
}

/*-----------------------------------------------------------------------------
This function releases all the FAT entries of a file.
-----------------------------------------------------------------------------*/

static void release_FAT_entries (struct rdcf *f)
{
  unsigned j;
  j = f->file.first_cluster;
  if (j != EMPTY_CLUSTER) {
    while (j < f->last_cluster_mark && j) {
      unsigned k = FAT_entry (f, j);
      if (j < DriveDesc.FirstPossiblyEmptyCluster) {
        DriveDesc.FirstPossiblyEmptyCluster = j;
      }
      set_FAT_entry (f, j, EMPTY_CLUSTER);
      j = k;
    }
  }
  f->mode |= WRITTEN;
}

/*-----------------------------------------------------------------------------
This function finds a new cluster, if possible, and adds it to a chain ending
with the specified cluster.  It returns the new cluster number, or
EMPTY_CLUSTER if there are no free clusters.
-----------------------------------------------------------------------------*/

static unsigned add_new_cluster (struct rdcf *f, unsigned cluster)
{
  unsigned new_cluster;
  for (new_cluster = DriveDesc.FirstPossiblyEmptyCluster;
       new_cluster <= f->maximum_cluster_number; new_cluster++) {
    if (FAT_entry (f, new_cluster) == EMPTY_CLUSTER)
      break;
  }
  if (new_cluster > f->maximum_cluster_number)
    return EMPTY_CLUSTER;
  if (cluster != EMPTY_CLUSTER)
    set_FAT_entry (f, cluster, new_cluster);
  set_FAT_entry (f, new_cluster, f->last_cluster_mark);
  return new_cluster;
}

/*-----------------------------------------------------------------------------
This function writes zeros into a cluster.
-----------------------------------------------------------------------------*/

static void clear_cluster (struct rdcf *f, unsigned cluster)
{
  unsigned count = f->sectors_per_cluster;
  unsigned sector = first_sector_in_cluster (f, cluster);
  flush_buffer (f);
  f->buffer_status = EMPTY;
  memset (f->buffer.buf, 0, SECTOR_SIZE);
  do
    write_sector (f, sector++, f->buffer.buf);
  while (--count != 0);
}

/*-----------------------------------------------------------------------------
This function adds another cluster to the directory if necessary to accommodate
a new file or subdirectory.
-----------------------------------------------------------------------------*/

static void lengthen_directory_if_necessary (struct rdcf *f)
{
  if (f->directory_index == NO_DIRECTORY_INDEX) {
    if (f->directory_cluster == 0)
      error_exit (f, ~ENOSPC);
    f->directory_cluster = add_new_cluster (f, f->directory_cluster);
    if (f->directory_cluster == 0)
      error_exit (f, ~ENOSPC);
    f->directory_index = 0;
    clear_cluster (f, f->directory_cluster);
  }
}

/*-----------------------------------------------------------------------------
The following functions are publicly defined.  They do not call each other and
any of them may be removed without making changes to those that remain.  The
functions are defined in alphabetical order.
-----------------------------------------------------------------------------*/

#define CHANGEABLE_ATTRIBUTES \
    (RDCF_ARCHIVE+RDCF_HIDDEN+RDCF_READ_ONLY+RDCF_SYSTEM)

int rdcf_close (struct rdcf *f)
{
  if ((f->result = setjmp (f->error)) != 0)
    return f->result;
  if (f->mode & WRITTEN) {
    f->buffer_status = EMPTY;
    f->file.attribute |= RDCF_ARCHIVE;
    rdcf_get_date_and_time (&f->file.date_and_time);
    // do not allow empty files.
    update_directory_entry (f, (f->file.size) ? 0 : 1);
    flush_buffer (f);
  }
  return 0;
}

int rdcf_delete (struct rdcf *f, const char *spec)
{
  if ((f->result = setjmp (f->error)) != 0)
    return f->result;
  spec = initialize_fcb (f, spec);
  if (*spec == 0) {             /* delete volume label */
    if (!find_file_in_directory_or_find_volume (f, NULL))
      error_exit (f, ~ENOENT);
  }
  else if (!find_file (f, spec))
    error_exit (f, ~ENOENT);
  /* check to see that a directory is empty before deleting it */
  else if (f->file.attribute & RDCF_DIRECTORY) {
    unsigned cluster = f->file.first_cluster;
    while (cluster != EMPTY_CLUSTER && cluster < f->last_cluster_mark) {
      unsigned sector = first_sector_in_cluster (f, cluster);
      unsigned sector_count = f->sectors_per_cluster;
      unsigned next_cluster = FAT_entry (f, cluster);
      do {
        unsigned entry_count = ENTRIES_PER_SECTOR;
        uint8_t *p = f->buffer.buf;
        read_buffer (f, sector);
        do {
          unsigned c = *p;
          if (c == END_DIRECTORY)
            break;
          if (c != DELETED_FILE && c != '.')
            error_exit (f, ~EACCES);
          p += sizeof (struct directory);
        } while (--entry_count != 0);
        if (entry_count != 0)
          break;
      } while (--sector_count != 0);
      cluster = next_cluster;
    }
  }
  else
    check_write_access (f);
  release_FAT_entries (f);
  update_directory_entry (f, 1);
  flush_buffer (f);
  return 0;
}

int rdcf_rename (struct rdcf *f, const char *old_spec, const char *new_spec)
{
  uint8_t name_extension[NAME_SIZE + EXTENSION_SIZE];
  if ((f->result = setjmp (f->error)) != 0)
    return f->result;
  old_spec = initialize_fcb (f, old_spec);
  if (!find_file (f, old_spec))
    error_exit (f, ~ENOENT);
  if (f->file.attribute & (RDCF_HIDDEN + RDCF_SYSTEM))
    error_exit (f, ~EACCES);
  spec_to_name_extension (f, name_extension, new_spec);
  f->directory_cluster = f->directory_first_cluster;
  if (find_file_in_directory_or_find_volume (f, name_extension))
    error_exit (f, ~ENOENT);
  find_file (f, old_spec);
  name_extension_to_spec (f->file.spec, name_extension);
  update_directory_entry (f, 0);
  flush_buffer (f);
  return 0;
}

int rdcf_open (struct rdcf *f, const char *spec, unsigned mode)
{
  int found;
  if ((f->result = setjmp (f->error)) != 0)
    return f->result;

  //  printf("mode: 0x%xd\n", mode);

  // does it exist already ?
  found = find_file (f, initialize_fcb (f, spec));

  // is it a directory ?
  if (found && f->file.attribute & RDCF_DIRECTORY)
    error_exit (f, ~EISDIR);

  // see how to open the file
  if (mode & O_CREAT) {
    //    printf("O_CREAT\n");
    bool new_file = false;
    if (mode & O_TRUNC && found) {
      //      printf(" O_TRUNC and found\n");
      // TRUNC and CREATE and found -> empty the file of clusters
      check_write_access (f);
      release_FAT_entries (f);

      new_file = true;
    } else if (!found) {
      //      printf(" not O_TRUNC, not found\n");
      // CREATE and not found -> create a new file
      lengthen_directory_if_necessary (f);

      new_file = true;
    }

    // if found and APPEND, don't do anything to it (new_file = false)

    // write back any changes we made here
    if (new_file) {
      //      printf(" new_file = true\n");
      f->file.attribute = RDCF_ARCHIVE;
      rdcf_get_date_and_time (&f->file.date_and_time);
      f->mode |= WRITTEN;
      f->file.first_cluster = EMPTY_CLUSTER;
      f->file.size = 0L;

      update_directory_entry (f, 0);
      flush_buffer (f);
    }
  } else if (!found) {
    //    printf("!found, !O_CREAT\n");
    // not found, not CREATE -> error
    error_exit (f, ~ENOENT);
  }

  // mark APPEND
  if (mode & O_APPEND) {
    f->mode |= APPEND;
  }


  // now file is created or we have signalled an error
  // decide the file mode
  switch (mode & 0x3) {
  case O_RDONLY:
    //    printf("open for reading\n");
    f->mode |= RDCF_READ;
    break;

  case O_RDWR:
    //    printf("open for reading\n");
    f->mode |= RDCF_READ;
    // fallthrough to write

  case O_WRONLY:
    //    printf("open for writing\n");
    check_write_access (f);
    f->mode |= RDCF_WRITE;
    break;
  }

  // do final setup
  f->last_cluster = EMPTY_CLUSTER;
  f->position = 0;
  f->cluster = f->file.first_cluster;

  // seek to end of file if O_WRONLY and O_APPEND
  if ((mode & O_APPEND) && ((mode & 0x3) == O_WRONLY)) {
    //    printf("O_APPEND seeking now\n");
    int result;
    if ((result = rdcf_seek (f, f->file.size)) != 0)
      return result;
  }
  return 0;
}

/***************************************************************
 * rdcf_read
 *
 *   fill buf with data from opened file.
 *   returns positive number on succes of bytes read.
 *
***************************************************************/
int rdcf_read (struct rdcf *f, void *buf, int count)
{
  uint32_t size = f->file.size;
  unsigned unread_bytes = count;
  uint32_t position = f->position;
  char *buffer = buf;
  if ((f->result = setjmp (f->error)) != 0)
    return f->result;
  if ((f->mode & RDCF_READ) == 0)
    error_exit (f, ~EBADFD);
  f->buffer_status = EMPTY;
  while (unread_bytes > 0) {
    unsigned n = unread_bytes;
    unsigned rest_of_sector = SECTOR_SIZE - position % SECTOR_SIZE;
    unsigned sector;
    if (size < position + n)
      n = size - position;
    if (n == 0)
      break;
    sector = first_sector_in_cluster (f, f->cluster) +
      (position / SECTOR_SIZE) % f->sectors_per_cluster;
    if (n > rest_of_sector)
      n = rest_of_sector;
    if (position % SECTOR_SIZE == 0 && n == SECTOR_SIZE)
      read_sector (f, sector, buffer);
    else {                      /* read a partial sector */

      read_buffer (f, sector);
      memcpy (buffer, &f->buffer.buf[position % SECTOR_SIZE], n);
    }
    buffer += n;
    position += n;
    unread_bytes -= n;
    if (position % (f->sectors_per_cluster * SECTOR_SIZE) == 0
        && position < size) {
      unsigned next_cluster = FAT_entry (f, f->cluster);
      if (next_cluster >= f->last_cluster_mark)
        f->last_cluster = f->cluster;
      f->cluster = next_cluster;
    }
  }
  f->position = position;
  return f->result = count - unread_bytes;
}


int rdcf_seek (struct rdcf *f, uint32_t offset)
{
  unsigned i, cluster;
  if ((f->result = setjmp (f->error)) != 0)
    return f->result;
  if (offset > f->file.size)
    error_exit (f, ~EINVAL);
  f->buffer_status = EMPTY;
  cluster = f->file.first_cluster;
  for (i = offset / (f->sectors_per_cluster * SECTOR_SIZE); i > 0; i--) {
    unsigned new_cluster = FAT_entry (f, cluster);
    if (new_cluster >= f->last_cluster_mark)
      f->last_cluster = cluster;
    cluster = new_cluster;
  }
  f->cluster = cluster;
  f->position = offset;
  return 0;
}

static int real_rdcf_write (struct rdcf *f, const uint8_t * buf, int count)
{
  uint32_t size = f->file.size;
  uint32_t position = f->position;
  unsigned unwritten_bytes = count;
  const uint8_t *buffer = buf;
  if ((f->result = setjmp (f->error)) != 0)
    return f->result;
  f->buffer_status = EMPTY;
  if ((f->mode & RDCF_WRITE) == 0)
    error_exit (f, ~EBADFD);

  if (f->mode & APPEND) {
    //    printf("O_APPEND seeking now\n");
    int result;
    if ((result = rdcf_seek (f, size)) != 0)
      return result;

    position = f->position;
  }

  while (unwritten_bytes > 0) {
    unsigned sector;
    unsigned n = unwritten_bytes;
    unsigned rest_of_sector = SECTOR_SIZE - position % SECTOR_SIZE;
    if (n > rest_of_sector)
      n = rest_of_sector;
    if (f->cluster == EMPTY_CLUSTER || f->cluster >= f->last_cluster_mark) {
      unsigned new_cluster =
        add_new_cluster (f, f->last_cluster);
      if (new_cluster == EMPTY_CLUSTER)
        break;
      DriveDesc.FirstPossiblyEmptyCluster = new_cluster + 1;
      f->cluster = f->last_cluster = new_cluster;
      if (f->file.first_cluster == EMPTY_CLUSTER)
        f->file.first_cluster = new_cluster;
    }
    sector = first_sector_in_cluster (f, f->cluster) +
      (position / SECTOR_SIZE) % f->sectors_per_cluster;
    if (position % SECTOR_SIZE == 0 &&
        (n == SECTOR_SIZE || position + n >= size)) {
      write_sector (f, sector, buffer);
    }
    else {                      /* write a partial sector */

      read_buffer (f, sector);
      memcpy (&f->buffer.buf[position % SECTOR_SIZE], buffer, n);
      f->buffer_status = DIRTY;
    }
    buffer += n;
    position += n;
    unwritten_bytes -= n;
    if (position > size)
      size = position;
    if (position % (f->sectors_per_cluster * SECTOR_SIZE) == 0) {
      unsigned next_cluster = FAT_entry (f, f->cluster);
      if (next_cluster >= f->last_cluster_mark)
        f->last_cluster = f->cluster;
      f->cluster = next_cluster;
    }
  }
  flush_buffer (f);
  f->position = position;
  f->file.size = size;
  f->mode |= WRITTEN;
#ifdef RDCF_FLUSH_DIR_AFTER_WRITE
  rdcf_get_date_and_time (&f->file.date_and_time);
  // do not allow empty files.
  update_directory_entry (f, (f->file.size) ? 0 : 1);
  flush_buffer (f);
#endif
  return f->result = count - unwritten_bytes;
}

int rdcf_write (struct rdcf *f, const void *buf, int count)
{                               // traps write() and ensures no dangling FAT chains.
  int result;
  result = real_rdcf_write (f, buf, count);
  // uhoh, something really bad happened!
  // flush buffer and close the handle.
  if (result <= 0) {
    rdcf_close (f);
  }
  return result;
}

int rdcf_flush_directory (struct rdcf *f)
{
  if ((f->result = setjmp (f->error)) != 0)
    return f->result;
  rdcf_get_date_and_time (&f->file.date_and_time);
  // do not allow empty files.
  update_directory_entry (f, (f->file.size) ? 0 : 1);
  flush_buffer (f);
  return 0;
}

#if 0
int rdcf_attribute (struct rdcf *f, const char *spec, unsigned attribute)
{
  if ((f->result = setjmp (f->error)) != 0)
    return f->result;
  if (!find_file (f, initialize_fcb (f, spec)) ||
      f->file.attribute & RDCF_DIRECTORY) {
    error_exit (f, ~ENOENT);
  }
  f->file.attribute = (f->file.attribute & ~CHANGEABLE_ATTRIBUTES) |
    (attribute & CHANGEABLE_ATTRIBUTES);
  update_directory_entry (f, 0);
  flush_buffer (f);
  return 0;
}

int rdcf_date_and_time (struct rdcf *f, const char *spec,
                        struct rdcf_date_and_time *p)
{
  if ((f->result = setjmp (f->error)) != 0)
    return f->result;
  spec = initialize_fcb (f, spec);
  if (*spec == 0) {
    f->directory_cluster = 0;
    if (!find_file_in_directory_or_find_volume (f, NULL))
      error_exit (f, ~ENOENT);
  }
  else {
    if (!find_file (f, spec))
      error_exit (f, ~ENOENT);
  }
  f->file.date_and_time = *p;
  if ((f->file.attribute & (RDCF_DIRECTORY + RDCF_VOLUME)) == 0) {
    check_write_access (f);
    f->file.attribute |= RDCF_ARCHIVE;
  }
  update_directory_entry (f, 0);
  if (f->file.attribute & RDCF_DIRECTORY)
    update_dot_and_dot_dot (f);
  flush_buffer (f);
  return 0;
}

int rdcf_directory (struct rdcf *f, const char *spec)
{
  /* uint8_t name_extension[NAME_SIZE+EXTENSION_SIZE]; ??? */
  if ((f->result = setjmp (f->error)) != 0)
    return f->result;
  if (find_file (f, initialize_fcb (f, spec)))
    error_exit (f, ~EISDIR);
  /* spec_to_name_extension(f, name_extension, f->file.spec); ??? */
  /* name_extension_to_spec(f->file.spec, name_extension); ??? */
  /* determine whether there is enough free space for directory */
  {
    unsigned cluster = 2;
    unsigned required_clusters =
      f->directory_index == NO_DIRECTORY_INDEX ? 2 : 1;
    for (cluster = 2; required_clusters != 0; cluster++) {
      if (cluster > f->maximum_cluster_number)
        error_exit (f, ~ENOSPC);
      if (FAT_entry (f, cluster) == EMPTY_CLUSTER)
        required_clusters--;
    }
  }
  lengthen_directory_if_necessary (f);
  f->file.attribute = RDCF_DIRECTORY;
  f->file.first_cluster = add_new_cluster (f, EMPTY_CLUSTER, 2);
  clear_cluster (f, f->file.first_cluster);
  f->file.size = 0L;
  rdcf_get_date_and_time (&f->file.date_and_time);
  update_directory_entry (f, 0);
  update_dot_and_dot_dot (f);
  flush_buffer (f);
  return 0;
}

long int rdcf_free_space (struct rdcf *f
#ifdef RDCF_MULTIPLE_DRIVE
                          , char *spec
#endif
  )
{
  unsigned cluster;
  unsigned number_of_empty_clusters = 0;
  if ((f->result = setjmp (f->error)) != 0)
    return (long) (f->result);
#ifndef RDCF_MULTIPLE_DRIVE
  initialize_fcb (f, NULL);
#else
  if (*initialize_fcb (f, spec) != 0)
    error_exit (f, ~EINVAL);
#endif
  for (cluster = 2; cluster <= f->maximum_cluster_number; cluster++) {
    if (FAT_entry (f, cluster) == EMPTY_CLUSTER)
      number_of_empty_clusters++;
  }
  f->file.size = (uint32_t) number_of_empty_clusters *
    (f->sectors_per_cluster * SECTOR_SIZE);
  return (long) (f->file.size);
}

int rdcf_get_file_information (struct rdcf *f, const char *spec, unsigned idx)
{
  if ((f->result = setjmp (f->error)) != 0)
    return f->result;
  find_entry (f, spec, idx);
  read_directory_entry (f);
  return f->result = f->file.spec[0] == DELETED_FILE ? ENOENT :
    f->file.spec[0] == 0 ? ENOSPC : 0;
}

int rdcf_next_file_information (struct rdcf *f)
{
  if ((f->result = setjmp (f->error)) != 0)
    return f->result;
  f->directory_index++;
  if (f->directory_cluster == 0) {
    if (f->directory_index >=
        (f->first_data_sector -
         f->first_directory_sector) * ENTRIES_PER_SECTOR) {
      error_exit (f, ~ENOSPC);
    }
  }
  else {
    if (f->directory_index >= ENTRIES_PER_SECTOR * f->sectors_per_cluster) {
      f->directory_cluster = FAT_entry (f, f->directory_cluster);
      if (f->directory_cluster >= f->last_cluster_mark)
        error_exit (f, ~ENOSPC);
      f->directory_index = 0;
    }
  }
  read_directory_entry (f);
  return f->result = f->file.spec[0] == DELETED_FILE ? ENOENT :
    f->file.spec[0] == 0 ? ENOSPC : 0;
}

/*-----------------------------------------------------------------------------
When f represents a subdirectory, this function updates the . and .. entries at
the beginning of its first cluster.
-----------------------------------------------------------------------------*/

static void update_dot_and_dot_dot (struct rdcf *f)
{
  f->directory_cluster = f->file.first_cluster;
  f->directory_index = 0;
  memset (f->file.spec, ' ', NAME_SIZE + EXTENSION_SIZE);
  f->file.spec[0] = '.';
  update_directory_entry (f, 0);
  f->file.first_cluster = f->directory_first_cluster;
  f->directory_index = 1;
  f->file.spec[1] = '.';
  update_directory_entry (f, 0);
}

/*-----------------------------------------------------------------------------
This function contains the common code from rdcf_get_file_information() and
rdcf_set_file_information().  It finds the directory entry specified by the
spec and index, and puts its cluster and index into f->directory_cluster and
f->directory_index.
-----------------------------------------------------------------------------*/

static void find_entry (struct rdcf *f, const char *spec, unsigned idx)
{
  spec = initialize_fcb (f, spec);
  if (*spec == 0) {
    if (idx >=
        (f->first_data_sector -
         f->first_directory_sector) * ENTRIES_PER_SECTOR) {
      error_exit (f, ~ENOSPC);
    }
    f->directory_first_cluster = f->directory_cluster = 0;
  }
  else {
    if (!find_file (f, spec) || (f->file.attribute & RDCF_DIRECTORY) == 0)
      error_exit (f, ~ENOENT);
    f->directory_first_cluster = f->directory_cluster = f->file.first_cluster;
    while (idx >= ENTRIES_PER_SECTOR * f->sectors_per_cluster) {
      f->directory_cluster = FAT_entry (f, f->directory_cluster);
      if (f->directory_cluster >= f->last_cluster_mark)
        error_exit (f, ~ENOSPC);
      idx -= ENTRIES_PER_SECTOR * f->sectors_per_cluster;
    }
  }
  f->directory_index = idx;
}

#endif

// vi:nowrap:
