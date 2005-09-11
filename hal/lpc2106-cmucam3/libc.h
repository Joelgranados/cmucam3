#ifndef __LIBC__
#define __LIBC__



#define MIN(a,b) ( ((a)<(b)) ? (a) : (b))
#define MAX(a,b) ( ((a)>(b)) ? (a) : (b))
#define ABS(a)   ( (a<0) ? -(a):(a))
#define SWAP(a,b) tmp = a; a = b; b = tmp;

#define NULL ((void*)0)

// returns the value of sine(x)
// for x:  0..2pi -> 0..1023
// return value is between -1024 and 1024

//int sin(int x);

//#define cos(x) (sin((x)+255))

unsigned int rand ();
extern unsigned int random;

char *itoa (char *str, int num);
char *itoa_hex (char *str, unsigned int num, int lcase);
char *sprintf_args (char *str, const char *format, void *args);
char *sprintf (char *str, const char *format, ...);
void printf (const char *format, ...);

int strlen (const char *str);
char *strcpy (char *dest, const char *str);
int strcmp (const char *str0, const char *str1);
int atoi (const char *str);

#endif
