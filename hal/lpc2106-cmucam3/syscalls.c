#include <stdlib.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include "serial.h"

#include <errno.h>
#undef errno
extern int errno;


int _write (int file, char *ptr, int len)
{
  int i = 0;

  char c;

  //uart0_write("in _write\r\n");

  if (file == UART0OUT_FILENO) {
    for (i = 0; i < len; i++) {
      //uart0_write(" uart0\r\n");
      c = *ptr++;
     /* if (c == '\n') {
	uart0_putc('\r');
      }*/
      uart0_putc(c);
    }
  } else if (file == UART1OUT_FILENO) {
    for (i = 0; i < len; i++) {
      //uart0_write(" uart1\r\n");
      c = *ptr++;
      if (c == '\n') {
	uart1_putc('\r');
      }
      uart1_putc(c);
    }
  }

  return i;
}

int _read (int file, char *ptr, int len)
{
  int i = 0;
 
  //uart0_write("in _read\r\n");

  if (file == UART0IN_FILENO) {
    //uart0_write(" uart0\r\n");
    for (i = 0; i < len; i++) {
      *ptr++ = uart0_getc();
    }
  } else if (file == UART1IN_FILENO) {
    //uart0_write(" uart1\r\n");
    for (i = 0; i < len; i++) {
      *ptr++ = uart1_getc();
    }
  }

  return i;
}


int _close(int file)
{
  //uart_0write("in _close\r\n");
  //uart_0write(" returning\r\n");
  return 0;
}

_off_t _lseek(int file, _off_t ptr, int dir)
{
  //uart_0write("in _lseek\r\n");
  //uart_0write(" returning\r\n");
  return 0;
}

int _fstat(int file, struct stat *st)
{
  //uart_0write("in _fstat\r\n");
  st->st_mode = S_IFCHR;	
  //uart_0write(" returning\r\n");
  return 0;
}


int isatty (int fd) 
{
  //uart_0write("in isatty\r\n");
  //uart_0write(" returning\r\n");
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

  //uart_0write(" heap_ptr = ");
  //uart_0write_hex((unsigned int) heap_ptr);

  base = heap_ptr;	/*  Point to end of heap.			*/

  //uart_0write(" base = ");
  //uart_0write_hex((unsigned int) base);

  if (base + nbytes >= (char *) 0x40010000) {
    errno = ENOMEM;
    return (void *) -1;
  }

  heap_ptr += nbytes;	/*  Increase heap.				*/
  
  //uart_0write(" heap_ptr = ");
  //uart_0write_hex((unsigned int) heap_ptr);

  //uart0_write(" returning\r\n");
  return base;		/*  Return pointer to start of new heap area.	*/
}

