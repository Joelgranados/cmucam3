#include <stdlib.h>
#include <sys/stat.h>
#include <sys/reent.h>
#include "serial.h"

#include <errno.h>
#undef errno
extern int errno;


//int _write_r (struct _reent *r, int file, char *ptr, int len)
int _write (int file, char *ptr, int len)
{
  int i = 0;

  for (i = 0; i < len; i++) {
    uart0_putc(*ptr++);
  }

  /*  if (file == 1) {
  } else if (file == 3) {
    for (i = 0; i < len; i++) {
      uart1_putc(*ptr++);
    }
    }*/

  return i;
}

int _read (int file, char *ptr, int len)
{
  int i = 0;

  for (i = 0; i < len; i++) {
    *ptr++ = uart0_getc();
  }

  /*
  if (file == 0) {
  } else if (file == 3) {
    for (i = 0; i < len; i++) {
      *ptr++ = uart1_getc();
    }
  }
  */

  return i;
}


int _close(int file)
{
  return 0;
}

_off_t _lseek(int file, _off_t ptr, int dir) {
  return 0;
}

int _fstat(int file, struct stat *st)
{
  st->st_mode = S_IFCHR;	
  return 0;
}


extern void *end;               /*  end is set in the linker command 	*/
				/* file and is the end of statically 	*/
				/* allocated data (thus start of heap).	*/

static void *heap_ptr;		/* Points to current end of the heap.	*/


void *_sbrk(int nbytes) {
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



int isatty (int fd)
{
  return 1;
  fd = fd;
}
