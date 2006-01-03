#include <stdlib.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include <sys/time.h>
#include <sys/times.h>
#include "serial.h"

#include <errno.h>
#undef errno

// register char *stack_ptr asm ("sp");
// The above line can be used to check the stack pointer
// uart0_write_hex(stack_ptr);


extern int errno;


int _write (int file, char *ptr, int len)
{
  int i = 0;

  //uart0_write("in _write\r\n");

  if (file == UART0OUT_FILENO) {
    for (i = 0; i < len; i++) {
      //uart0_write(" uart0\r\n");
      uart0_putc(*ptr++);
    }
  } else if (file == UART1OUT_FILENO) {
    for (i = 0; i < len; i++) {
      //uart0_write(" uart1\r\n");
      uart1_putc(*ptr++);
    }
  } else {
    errno = EBADF;
    return -1;
  }

  return i;
}

int _read (int file, char *ptr, int len)
{
  int i = 0;
 
  //uart0_write("in _read\r\n");
  if (file == UART0IN_FILENO) {
    for (i = 0; i < len; i++) {
	if((*ptr++ = uart0_getc())=='\n') { i++; break; }
    }
  } else if (file == UART1IN_FILENO) {
    for (i = 0; i < len; i++) {
      if((*ptr++ = uart1_getc())=='\n') { i++; break; }
    }
  } else {
    errno = EBADF;
    return -1;
  }

  return i;
}

int kill(int pid, int sig)
{
  errno=EINVAL;
  return(-1);
}

void _exit(int status __attribute((unused)))
{
  while(1);
}

int _close(int file __attribute((unused)))
{
  //uart_0write("in _close\r\n");
  //uart_0write(" returning\r\n");
  return 0;
}

_off_t _lseek(int file __attribute((unused)), 
	      _off_t ptr __attribute((unused)), 
	      int dir __attribute((unused)))
{
  //uart_0write("in _lseek\r\n");
  //uart_0write(" returning\r\n");
  return 0;
}

int _fstat(int file __attribute((unused)), 
	   struct stat *st __attribute((unused)))
{
  //uart_0write("in _fstat\r\n");
  st->st_mode = S_IFCHR;	
  //uart_0write(" returning\r\n");
  return 0;
}


int isatty (int fd __attribute((unused))) 
{
  //uart_0write("in isatty\r\n");
  //uart_0write(" returning\r\n");
  return 1;
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

int _link(char *old __attribute((unused)), 
	  char *new __attribute((unused))){
  errno = EMLINK;
  return -1;
}

int _open(const char *name __attribute((unused)), 
	  int flags __attribute((unused)), 
	  int mode __attribute((unused))){
  errno = ENOENT;
  return -1;
}

int _stat(char *file __attribute((unused)), 
	  struct stat *st) {
  st->st_mode = S_IFCHR;
  return 0;
}

int _rename(char *oldpath __attribute((unused)), 
	    char *newpath __attribute((unused))) {
  errno = EINVAL;
  return -1;
}

int _gettimeofday (struct timeval *tp __attribute((unused)), 
		   struct timezone *tzp __attribute((unused))) {
  return -1;
}

int _kill(int pid, int sig)
{
  errno = EINVAL;
  return -1;
}

int _getpid() 
{
  return 1;
}

int _times(struct tms *buf) {
  clock_t ticks 
    = REG(TIMER0_TC) / (1000 / CLOCKS_PER_SEC); // REG in milliseconds
  buf->tms_utime = ticks;
  buf->tms_stime = 0;
  buf->tms_cutime = 0;
  buf->tms_cstime = 0;
  return ticks;
}

int _unlink(char *name __attribute((unused))) {
  errno = ENOENT;
  return -1; 
}

int _raise(int sig) 
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

