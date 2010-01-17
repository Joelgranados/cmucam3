#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define X_MAX		1024
#define Y_MAX		768
#define CHAN_MAX	3

uint8_t img[X_MAX][Y_MAX][CHAN_MAX];
int x, y;

int
main (int argc, char *argv[])
{
  int i, j, k, chan;
  uint8_t c;

  FILE *fp;
  if (argc != 2)
    {
      printf ("Usage: %s <serial-port>\nExample: %s /dev/ttyUSB0\n", argv[0],
	      argv[0]);
      return -1;
    }
  printf ("Opening Port: %s\n", argv[1]);
  fp = fopen (argv[1], "r+");
  if (fp == NULL)
    {
      printf ("Could not open serial port: %s\n", argv[1]);
      return -1;
    }

  printf ("Sending Frame Dump Command\n");
  fprintf (fp, "\r\r\rdf\r");

  // Parse data from frame dump
  chan = 0;
  while (1)
    {
      c = fgetc (fp);

      if (c == 1)
	{
	  // New Frame
	  printf ("Got new frame\n");
	  chan = 0;
	  x = 0;
	  y = 0;
	}
      else if (c == 2)
	{
	  // New Col 
	  printf ("New col %d\n", x);
	  chan = 0;
	  if(x<X_MAX) x++;
	  y = 0;
	}
      else if (c == 3)
	break;
      else
	{
	  // Image data (r,g,b)
	  img[x][y][chan] = c;
	  chan++;
	  if (chan >= CHAN_MAX)
	    {
	      chan = 0;
	     if(y<Y_MAX) y++;
	    }
	}

    }
  fclose (fp);


  // Write the ppm output
  printf ("Writing out.ppm");
  fp = fopen ("out.ppm", "w");
  if (fp == NULL)
    {
      printf ("Could not write ppm output\n");
    }

  fprintf (fp, "P6\n%d %d\n255\n", x, y);
  for (i = 0; i < x; i++)
    for (j = 0; j < y; j++)
      for (k = 0; k < 3; k++)
	fprintf (fp, "%c", img[i][j][k]);
  fclose (fp);
return 1;
}
