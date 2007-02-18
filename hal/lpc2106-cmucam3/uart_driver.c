#include "cc3.h"
#include "devices.h"
#include "serial.h"

#include <stdlib.h>

#include <errno.h>
#undef errno
extern int errno;

// we are using sparse file descriptor space,
// so don't increase this willy-nilly
static const int max_uarts = 16;

static int process_uart_filename(const char *name) {
  // parse "COMx:"
  if (name[0] == 'C' && name[1] == 'O' && name[2] == 'M') {
    char *endptr;
    int uart = strtol(name + 3,
		      &endptr,
		      10);
    if (endptr[0] == ':' && endptr[1] == '\0') {
      return uart;
    }
  }

  return -1;
}

static int uart_open (const char *name,
		      int flags __attribute__((unused)),
		      int mode __attribute__((unused)))
{
  int uart_num = process_uart_filename(name);
  if (uart_num >= 0 && uart_num < cc3_uart_get_count()) {
    return uart_num + max_uarts; // new uart
  } else {
    errno = ENODEV;
    return -1;
  }
}

static int uart_close (int file __attribute__((unused)))
{
  return 0;
}

static ssize_t uart_read (int file, void *ptr, size_t len)
{
  cc3_uart_binmode_t binmode;
  int (*uart_getc) (void);

  unsigned int i;
  char c;
  char *c_ptr = (char *) ptr;

  switch (file) {
  case 0:  // non-redirected stdin
  case max_uarts + 0:
    binmode = _cc3_uart0_binmode;
    uart_getc = uart0_getc;
    break;
  case max_uarts + 1:
    binmode = _cc3_uart1_binmode;
    uart_getc = uart1_getc;
    break;
  default:
    errno = EBADF;
    return -1;
  }

  for (i = 0; i < len; i++) {
    c = uart_getc();
    if (binmode == CC3_UART_BINMODE_TEXT && c == '\r') {
      c = '\n';
    }

    *c_ptr++ = c;
    if (c == '\n' || c == '\r') {
      i++;
      break;
    }
  }
  return i;
}

static ssize_t uart_write (int file, const void *ptr, size_t len)
{
  cc3_uart_binmode_t binmode;
  char (*uart_putc) (char);

  unsigned int i;
  const char *c_ptr = ptr;

  switch (file) {
  case 1:  // non-redirected stdout
  case 2:  // non-redirected stderr
  case max_uarts + 0:
    binmode = _cc3_uart0_binmode;
    uart_putc = uart0_putc;
    break;
  case max_uarts + 1:
    binmode = _cc3_uart1_binmode;
    uart_putc = uart1_putc;
    break;
  default:
    errno = EBADF;
    return -1;
  }

  for (i = 0; i < len; i++) {
    if (binmode == CC3_UART_BINMODE_TEXT && *c_ptr == '\n') {
      uart_putc('\r');
    }
    uart_putc(*c_ptr++);
  }
  return i;
}

static bool uart_recognize (const char *name)
{
  return process_uart_filename(name) != -1;
}

static off_t uart_lseek (int file __attribute__((unused)),
			 off_t offset __attribute__((unused)),
			 int dir  __attribute__((unused)))
{
  errno = ESPIPE;
  return -1;
}

static int uart_fstat (int file __attribute__((unused)),
		       struct stat *st)
{
  st->st_mode = S_IFCHR;
  return 0;
}

static int uart_unlink (const char *name __attribute__((unused)))
{
  errno = EINVAL;
  return -1;
}

static int uart_rename (const char *oldname __attribute__((unused)),
			const char *newname __attribute__((unused)))
{
  errno = EINVAL;
  return -1;
}

_cc3_device_driver_t _cc3_uart_driver = {
  .id        = _CC3_DEVICE_UART,
  .is_tty    = true,
  .open      = uart_open,
  .close     = uart_close,
  .read      = uart_read,
  .write     = uart_write,
  .recognize = uart_recognize,
  .lseek     = uart_lseek,
  .unlink    = uart_unlink,
  .rename    = uart_rename,
  .fstat     = uart_fstat
};
