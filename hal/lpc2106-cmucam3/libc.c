// CMUcam LIBC functions
// 
// Uwe Maurer 2004

#include "sine_table.h"
// returns the value of sine(x)
// for x:  0..2pi -> 0..1023
// return value is between -1024 and 1024
/*
int sin(int x) {
	x&=1023;
	if (x<256) return sine_table[x];
	if (x<512) return sine_table[511-x];
	if (x<768) return -sine_table[x-512];
	return -sine_table[1023-x];
}
*/

// defined as macro in libc.h
/*
int cos(int x) {
	return sin(x+255);
}
*/

unsigned int random;
unsigned int
rand ()
{
  return (random = 1664525L * random + 1013904223L);
}

char *
itoa (char *str, int num)
{
  char *s = str;
  int cnt;
  char buffer[10];
  if (num < 0)
    {
      num = -num;
      *s++ = '-';
    }
  for (cnt = 9; cnt >= 0; cnt--)
    {
      buffer[cnt] = (num % 10) + '0';
      num /= 10;
      if (num == 0)
	break;
    }
  for (; cnt < 10; cnt++)
    *s++ = buffer[cnt];
  *s = 0;
  return str;
}

char *
itoa_hex (char *str, unsigned int num, int lcase)
{
  int i, n;
  str[0] = '0';
  str[1] = 'x';
  char a = lcase ? 'a' : 'A';
  for (i = 0; i < 8; i++)
    {
      n = num & 0xf;
      num >>= 4;
      str[9 - i] = n < 10 ? n + '0' : n - 10 + a;
    }
  str[10] = 0;
  return str;
}

char *
itoa_bin (char *str, unsigned int num, int bits)
{
  int i;
  char *s = str;
  *s++ = 'b';
  for (i = 0; i < bits; i++)
    {
      *s++ = '0' + ((num >> (bits - 1 - i)) & 1);
      if ((i & 3) == 3 && i < bits - 1)
	*s++ = ' ';
    }
  *s = 0;
  return str;
}

int
atoi (const char *str)
{
  int num = 0;
  int sign = 1;
  if (*str == '-')
    {
      sign = -1;
      str++;
    }
  while (*str && *str >= '0' && *str <= '9')
    {
      num = num * 10 + (*str - '0');
      str++;
    }
  return num * sign;
}

char *sprintf_args (char *str, const char *format, void *args);

// similar to sprintf from libc
// returns the first parameter 
// 
// example: sprintf(buf,"Important Email! %% %i, %d, %x, %X, %c, %s, %i.",512,511,511,0xdeadbeef,'A',"uwe",99);
// result:  "Important Email! % 512, 511, 0x000001ff, 0xDEADBEEF, A, uwe, 99." 
//
char *
sprintf (char *str, const char *format, ...)
{
  void *args = (void *) &format + sizeof (format);
  return sprintf_args (str, format, args);
}

char *
sprintf_args (char *str, const char *format, void *args)
{
  char *s = str;
  while (*format != 0)
    {
      if (*format == '%')
	{
	  format++;
	  if (*format == '%')
	    *s++ = '%';
	  else if (*format == 'i' || *format == 'd')
	    {			// decimal
	      itoa (s, *(int *) args);
	      args += sizeof (int);
	      while (*s != 0)
		s++;
	    }
	  else if (*format == 'X' || *format == 'x')
	    {			// hex, both uppercase
	      itoa_hex (s, *(unsigned int *) args, *format == 'x');
	      args += sizeof (int);
	      while (*s != 0)
		s++;
	    }
	  else if (*format == 's')
	    {			// string
	      char *inp = *(char **) args;
	      args += sizeof (char *);
	      while (*inp != 0)
		*s++ = *inp++;
	    }
	  else if (*format == 'c')
	    {			// character
	      *s++ = *(char *) args;
	      args += sizeof (int);	// 4 bytes for character
	    }
	  else if (*format == 'b' || *format == 'B')
	    {			// binary byte/int
	      itoa_bin (s, *(unsigned int *) args, (*format == 'b') ? 8 : 32);
	      args += sizeof (int);	// 4 bytes
	      while (*s != 0)
		s++;
	    }
	  else
	    *s++ = '?';		// error, unknown format character
	  format++;
	}
      else
	{
	  *s++ = *format++;	// just copy
	}
    }
  *s = 0;
  return str;
}



int
strlen (const char *s)
{
  const char *a = s;
  while (*a != 0)
    a++;
  return a - s;
}


int
strcmp (const char *s0, const char *s1)
{
  while (*s0 && *s1 && *s0 == *s1)
    {
      s0++;
      s1++;
    }
  if (*s0 == *s1)
    return 0;
  return (*s0 < *s1) ? -1 : 1;
}

char *
strcpy (char *dest, const char *str)
{
  char *s = dest;
  while (*str)
    *dest++ = *str++;
  *dest = 0;
  return s;
}
