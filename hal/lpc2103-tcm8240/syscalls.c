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


#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include <sys/time.h>
#include <sys/times.h>

#include "LPC2100.h"
#include "devices.h"
#include "serial.h"

#include <errno.h>
#undef errno
extern int errno;

// register char *stack_ptr asm ("sp");
// The above line can be used to check the stack pointer
// uart0_write_hex(stack_ptr);


/* prototypes */
int _write (int file, const char *ptr, int len);
int _read (int file, char *ptr, int len);
int kill(int pid, int sig);
void _exit(int status);
void abort(void);
int _close(int file);
_off_t _lseek(int file, _off_t offset, int dir);
int _fstat(int file, struct stat *st);
int _isatty (int file);
int _system(const char *s);
int _link(char *old, char *new);
int _open(const char *name, int flags, int mode);
int _rename(char *oldpath, char *newpath);
int _gettimeofday (struct timeval *tp, struct timezone *tzp);
int _kill(int pid, int sig);
int _getpid(void);
int _times(struct tms *buf);
int _unlink(char *name);
int _raise(int sig);
void *_sbrk(int nbytes);


/* implementation */
int _write (int file, const char *ptr, int len)
{
  _cc3_device_driver_t *dev = _cc3_get_driver_for_file_number(file);
  if (dev == NULL) {
    errno = EBADF;
    return -1;
  }

  return dev->write(_cc3_get_internal_file_number(file),
		    ptr, len);
}

int _read (int file, char *ptr, int len)
{
  _cc3_device_driver_t *dev = _cc3_get_driver_for_file_number(file);
  if (dev == NULL) {
    errno = EBADF;
    return -1;
  }

  return dev->read(_cc3_get_internal_file_number(file),
		   ptr, len);
}

int kill(int pid __attribute__((unused)),
	 int sig __attribute__((unused)))
{
  errno = EINVAL;
  return(-1);
}

void _exit(int status __attribute__((unused)))
{
  // XXX: should call cc3_power_down
  while(1);
}

void abort(void)
{
  _exit(1);
}

int _close(int file)
{
  _cc3_device_driver_t *dev = _cc3_get_driver_for_file_number(file);
  if (dev == NULL) {
    errno = EBADF;
    return -1;
  }

  return dev->close(_cc3_get_internal_file_number(file));
}

_off_t _lseek(int file, _off_t offset, int dir)
{
  _cc3_device_driver_t * dev = _cc3_get_driver_for_file_number(file);
  if (dev == NULL) {
    errno = EBADF;
    return -1;
  }

  return dev->lseek(_cc3_get_internal_file_number(file),
		    offset,
		    dir);
}

int _fstat(int file, struct stat *st)
{
  _cc3_device_driver_t * dev = _cc3_get_driver_for_file_number(file);
  if (dev == NULL) {
    errno = EBADF;
    return -1;
  }

  return dev->fstat(_cc3_get_internal_file_number(file),
		    st);
}


int _isatty (int file)
{
  _cc3_device_driver_t * dev = _cc3_get_driver_for_file_number(file);
  if (dev == NULL) {
    errno = EBADF;
    return -1;
  }

  return dev->is_tty ? 1 : 0;
}

int _system(const char *s)
{
  if (s == NULL) {
    return 0; /* no shell */
  } else {
    errno = EINVAL;
    return -1;
  }
}

int _link(char *old __attribute__((unused)),
	  char *new __attribute__((unused))) {
  // we do not support hard links
  errno = EPERM;
  return -1;
}




static void normalize_filename(char *name) {
  // make all caps, and change '\' to '/'
  int i = 0;
  char c;
  while ((c = name[i]) != '\0') {
    if (c == '\\' ) {
      name[i] = '/';
    } else {
      name[i] = toupper(c);
    }
    i++;
  }
}

int _open(const char *name, int flags, int mode)
{
  int result = -1;
  char *norm = strdup(name);
  _cc3_device_driver_t *dev;

  if (norm == NULL) {
    return result;
  }

  normalize_filename(norm);

  dev = _cc3_get_driver_for_name(norm);
  if (dev == NULL) {
    errno = ENOENT;
    result = -1;
  } else {
    result = _cc3_make_file_number(dev,
				   dev->open(name, flags, mode));
  }

  free(norm);
  return result;
}

int _rename(char *oldpath, char *newpath) {
  int result = -1;
  char *n_oldpath;
  char *n_newpath;

  _cc3_device_driver_t *dev1;
  _cc3_device_driver_t *dev2;

  // normalize paths
  n_oldpath = strdup(oldpath);
  if (n_oldpath == NULL) {
    return -1;
  }
  n_newpath = strdup(newpath);
  if (n_newpath == NULL) {
    free(n_oldpath);
    return -1;
  }
  normalize_filename(n_oldpath);
  normalize_filename(n_newpath);

  // get the device drivers for the paths
  dev1 = _cc3_get_driver_for_name(n_oldpath);
  dev2 = _cc3_get_driver_for_name(n_newpath);

  if (dev1 != dev2) {
    // make sure the devices are the same
    errno = EXDEV;
  } else if (dev1 == NULL || dev2 == NULL) {
    // make sure the drivers exist
    errno = ENOENT;
  } else {
    // pass it down
    result = dev1->rename(n_oldpath, n_newpath);
  }

  // done
  free(n_oldpath);
  free(n_newpath);
  return result;
}

int _gettimeofday (struct timeval *tp __attribute__((unused)),
		   struct timezone *tzp __attribute__((unused))) {
  return -1;
}

int _kill(int pid __attribute__((unused)),
	  int sig __attribute__((unused)))
{
  errno = EINVAL;
  return -1;
}

int _getpid()
{
  return 1;
}

int _times(struct tms *buf)
{
  clock_t ticks
    = REG(TIMER0_TC) / (1000 / CLOCKS_PER_SEC); // REG in milliseconds
  buf->tms_utime = ticks;
  buf->tms_stime = 0;
  buf->tms_cutime = 0;
  buf->tms_cstime = 0;
  return ticks;
}

int _unlink(char *name)
{
  int result = -1;
  char *norm = strdup(name);
  _cc3_device_driver_t *dev;

  if (norm == NULL) {
    return -1;
  }

  // normalize
  normalize_filename(norm);

  // get device
  dev = _cc3_get_driver_for_name(norm);
  if (dev != NULL) {
    result = dev->unlink(name);
  } else {
    errno = ENOENT;
  }

  free(norm);
  return result;
}

int _raise(int sig __attribute__((unused)))
{
  return 1;
}


/* exciting memory management! */


extern char _end[];             /* end is set in the linker command 	*/
				/* file and is the end of statically 	*/
				/* allocated data (thus start of heap).	*/

extern char _heap_end[];        /* heap_end is also set in the linker   */
                                /* and represents the physical end of   */
                                /* ram (and the ultimate limit of the   */
                                /* heap).                               */

static void *heap_ptr;		/* Points to current end of the heap.	*/


void *_sbrk(int nbytes)
{
  char *base;		/*  errno should be set to  ENOMEM on error	*/
  //  uart0_write("in _sbrk\r\n");

  //  uart0_write(" nbytes = ");
  //  uart0_write_hex((unsigned int) nbytes);

  //  uart0_write(" heap_ptr = ");
  //  uart0_write_hex((unsigned int) heap_ptr);

  if (!heap_ptr) {	/*  Initialize if first time through.		*/
    heap_ptr = _end;
  }

  //  uart0_write(" heap_ptr = ");
  //  uart0_write_hex((unsigned int) heap_ptr);

  base = heap_ptr;	/*  Point to end of heap.			*/

  //  uart0_write(" base = ");
  //  uart0_write_hex((unsigned int) base);

  if (base + nbytes >= (char *) _heap_end) {
    uart0_write(" ENOMEM!\r\n");
    errno = ENOMEM;
    return (void *) -1;
  }

  heap_ptr = (char *)heap_ptr + nbytes;	        /*  Increase heap */

  //  uart0_write(" heap_ptr = ");
  //  uart0_write_hex((unsigned int) heap_ptr);

  //  uart0_write(" returning\r\n");
  return base;		/*  Return pointer to start of new heap area.	*/
}
