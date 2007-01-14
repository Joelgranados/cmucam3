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

#include "serial.h"
#include "devices.h"
#include "mmc_ioctl.h"

#include <errno.h>
#undef errno
extern int errno;

// register char *stack_ptr asm ("sp");
// The above line can be used to check the stack pointer
// uart0_write_hex(stack_ptr);


extern DEVICE_TABLE_ENTRY mmc_driver;

static int init_mmc(void) {
  return mmc_driver.init();
}

/* prototypes */
int _write (int file, char *ptr, int len);
int _read (int file, char *ptr, int len);
int kill(int pid, int sig);
void _exit(int status);
int _close(int file);
_off_t _lseek(int file, _off_t ptr, int dir);
int _fstat(int file, struct stat *st);
int isatty (int file);
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
int _write (int file, char *ptr, int len)
{
  int i = 0;

  //uart0_write("in _write\r\n");

  switch (file) {
  case 1:   // non-redirected stdout
  case 2:   // non-redirected stderr
  case DEVICE(UART_DEVICE) | 0:
    for (i = 0; i < len; i++) {
      if (_cc3_uart0_binmode == CC3_UART_BINMODE_TEXT
	  && *ptr == '\n') {
	uart0_putc('\r');
      }
      uart0_putc(*ptr++);
    }
    return i;

  case DEVICE(UART_DEVICE) | 1:
    for (i = 0; i < len; i++) {
      if (_cc3_uart1_binmode == CC3_UART_BINMODE_TEXT
	  && *ptr == '\n') {
	uart1_putc('\r');
      }
      uart1_putc(*ptr++);
    }
    return i;

  default:
    // MMC?
    if (DEVICE_TYPE(file) == mmc_driver.device_type) {
      init_mmc();
      return mmc_driver.write(file, ptr, len);
    } else {
      errno = EBADF;
      return -1;
    }
  }
}

int _read (int file, char *ptr, int len)
{
  int i = 0;
  char c;

  //uart0_write("in _read\r\n");
  switch (file) {
  case 0:   // non-redirected stdin
  case DEVICE(UART_DEVICE) | 0:
    for (i = 0; i < len; i++) {
      c = uart0_getc();
      if (_cc3_uart0_binmode == CC3_UART_BINMODE_TEXT
	  && c == '\r') {
	c = '\n';
      }

      *ptr++ = c;
      if (c == '\n' || c == '\r') {
	i++;
	break;
      }
    }
    return i;

  case DEVICE(UART_DEVICE) | 1:
    for (i = 0; i < len; i++) {
      c = uart1_getc();
      if (_cc3_uart1_binmode == CC3_UART_BINMODE_TEXT
	  && c == '\r') {
	c = '\n';
      }

      *ptr++ = c;
      if (c == '\n' || c == '\r') {
	i++;
	break;
      }
    }
    return i;

  default:
    // MMC?
    if (DEVICE_TYPE(file) == mmc_driver.device_type) {
      init_mmc();
      return mmc_driver.read(file, ptr, len);
    } else {
      errno = EBADF;
      return -1;
    }
  }
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

int _close(int file)
{
  // MMC?
  if (DEVICE_TYPE(file) == mmc_driver.device_type) {
    init_mmc();
    return mmc_driver.close(file);
  } else {
    return 0;
  }
}

_off_t _lseek(int file, _off_t ptr, int dir)
{
  // MMC?
  if (DEVICE_TYPE(file) == mmc_driver.device_type) {
    struct ioctl_seek seeker;

    seeker.pos = &ptr;
    seeker.whence = &dir;

    init_mmc();

    return mmc_driver.ioctl(file, IOCTL_MMC_SEEK, &seeker);
  } else {
    return 0;
  }
}

int _fstat(int file, struct stat *st)
{
  if (file == DEVICE_TYPE(UART_DEVICE)) {
    st->st_mode = S_IFCHR;
    return 0;
  } else {
    errno = EIO;
    return -1;
  }
}


int isatty (int file)
{
  return file == DEVICE_TYPE(UART_DEVICE);
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




static bool is_mmc_filename(const char *name) {
  // filename starts with "c:/"
  return name[0] == 'C' && name[1] == ':' && name[2] == '/';
}

static int process_uart_filename(const char *name) {
  int uart;
  int result;
  char buf[2];

  result = sscanf(name, "COM%3d:%c", &uart, buf);
  if (result != 1) {
    return -1;
  }

  return uart;
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
  int uart_num;
  char *norm = strdup(name);

  if (norm == NULL) {
    errno = ENOMEM;
    return result;
  }

  normalize_filename(norm);

  if (is_mmc_filename(norm)) {
    // skip "c:/" part
    init_mmc();
    result = mmc_driver.open(&norm[3], flags, mode);
  } else if ((uart_num = process_uart_filename(norm)) != -1) {
    // extract from "COMxx:"
    if (uart_num < cc3_get_uart_count()) {
      result = DEVICE(UART_DEVICE) | uart_num;
    } else {
      errno = ENODEV;
    }
  } else {
    errno = ENOENT;
  }

  free(norm);
  return result;
}

int _rename(char *oldpath, char *newpath) {
  int result = -1;
  char *n_oldpath;
  char *n_newpath;

  n_oldpath = strdup(oldpath);
  if (n_oldpath == NULL) {
    errno = ENOMEM;
    return -1;
  }
  n_newpath = strdup(newpath);
  if (n_newpath == NULL) {
    free(n_oldpath);
    errno = ENOMEM;
    return -1;
  }

  normalize_filename(n_oldpath);
  normalize_filename(n_newpath);

  if (is_mmc_filename(n_oldpath) && is_mmc_filename(n_newpath)) {
    struct ioctl_rename renamer;
    renamer.oldname = &n_oldpath[3];  // skip "c:/"
    renamer.newname = n_newpath;

    init_mmc();
    result = mmc_driver.ioctl(0, IOCTL_MMC_RENAME, &renamer);
  } else {
    errno = EINVAL;
  }

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

  if (norm == NULL) {
    errno = ENOMEM;
    return -1;
  }

  normalize_filename(norm);

  if (is_mmc_filename(norm)) {
    long ptr = (long) &norm[3];  // skip "c:/"

    init_mmc();
    result = mmc_driver.ioctl(0, IOCTL_MMC_UNLINK, &ptr);
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


extern char end[];              /*  end is set in the linker command 	*/
				/* file and is the end of statically 	*/
				/* allocated data (thus start of heap).	*/

static void *heap_ptr;		/* Points to current end of the heap.	*/


void *_sbrk(int nbytes)
{
  char *base;		/*  errno should be set to  ENOMEM on error	*/
  //uart0_write("in _sbrk\r\n");

  //uart0_write(" nbytes = ");
  //uart0_write_hex((unsigned int) nbytes);

  //uart0_write(" heap_ptr = ");
  //uart0_write_hex((unsigned int) heap_ptr);

  if (!heap_ptr) {	/*  Initialize if first time through.		*/
    heap_ptr = end;
  }

  //uart0_write(" heap_ptr = ");
  //uart0_write_hex((unsigned int) heap_ptr);

  base = heap_ptr;	/*  Point to end of heap.			*/

  //uart0_write(" base = ");
  //uart0_write_hex((unsigned int) base);

  if (base + nbytes >= (char *) 0x40010000) {
    errno = ENOMEM;
    return (void *) -1;
  }

  heap_ptr = (char *)heap_ptr + nbytes;	        /*  Increase heap */

  //uart0_write(" heap_ptr = ");
  //uart0_write_hex((unsigned int) heap_ptr);

  //uart0_write(" returning\r\n");
  return base;		/*  Return pointer to start of new heap area.	*/
}
