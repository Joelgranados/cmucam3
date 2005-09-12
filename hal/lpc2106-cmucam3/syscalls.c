#include <stdlib.h>
#include <sys/stat.h>
#include "serial.h"

#include <errno.h>
#undef errno
extern int errno;


int write (int file, char *ptr, int len)
{
  int i = 0;

  if (file == 0) {
    for (i = 0; i < len; i++) {
      uart0_putc(*ptr++);
    }
  } else if (file == 1) {
    for (i = 0; i < len; i++) {
      uart1_putc(*ptr++);
    }
  }

  return i;
}

int read (int file, char *ptr, int len)
{
  int i = 0;

  if (file == 0) {
    for (i = 0; i < len; i++) {
      *ptr++ = uart0_getc();
    }
  } else if (file == 1) {
    for (i = 0; i < len; i++) {
      *ptr++ = uart1_getc();
    }
  }

  return i;
}



extern char end[];              /*  end is set in the linker command 	*/
				/* file and is the end of statically 	*/
				/* allocated data (thus start of heap).	*/

static char *heap_ptr;		/* Points to current end of the heap.	*/


caddr_t sbrk(int nbytes) {
  char *base;		/*  errno should be set to  ENOMEM on error	*/
  
  if (!heap_ptr) {	/*  Initialize if first time through.		*/
    heap_ptr = end;
  }

  base = heap_ptr;	/*  Point to end of heap.			*/

  if (base + nbytes >= (char *) 0x40010000) {
    errno = ENOMEM;
    return (void *) -1;
  }

  heap_ptr += nbytes;	/*  Increase heap.				*/
  
  return base;		/*  Return pointer to start of new heap area.	*/
}
