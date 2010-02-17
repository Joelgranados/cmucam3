#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define X_MAX		1024
#define Y_MAX		1024	
#define CHAN_MAX	3

uint8_t img[X_MAX][Y_MAX][CHAN_MAX];
int x, y, chan;
int x_max, y_max;
int
main (int argc, char *argv[])
{
  int k,j;
  uint8_t c,started;

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

    for(x=0; x<X_MAX; x++ )
    for(y=0; y<Y_MAX; y++ )
    for(chan=0; chan<CHAN_MAX; chan++ ) img[x][y][chan]=0;


   // Parse data from frame dump
  chan = 0;
  x_max=0;
  y_max=0;
  x=0;
  y=0;
  started=0;
  while (1)
    {
      c = fgetc (fp);

      if (c == 1)
	{
	  started=1;
	  // New Frame
	  printf ("Got new frame\n");
	  chan = 0;
	  x = -1;
	  y = 0;
	}
      else if (c == 2)
	{
	  // New Col 
	  chan = 0;
	  y = 0;
	  if(x<X_MAX) x++;
	  if(x>x_max)x_max=x;
	  printf ("New col %d\n", x);
	}
      else if (c == 3)
	break;
      else
	{
	  if(started && x>=0)
	 { 
	  // Image data (r,g,b)
	  img[x][y][chan] = c;
	  chan++;
	  if (chan >= CHAN_MAX)
	    {
	      chan = 0;
	     if(y<Y_MAX) y++;
	     if(y>y_max) y_max=y;
	    }
	 }
	}

    }
  fclose (fp);


  // Write the ppm output
  printf ("Writing out.ppm\n");
  fp = fopen ("out.ppm", "w");
  if (fp == NULL)
    {
      printf ("Could not write ppm output\n");
    }

  printf( "x_max=%d y_max=%d\n",x_max*2, y_max/(2) );
  fprintf (fp, "P6\n%d %d\n255\n", x_max*2, y_max/(2));
  for (y = 0; y < y_max; y+=2)
    for (x = 0; x < x_max; x++)
    {
      for(j=0; j<2; j++ )
      {
      	for (k = 0; k < 3; k++) fprintf (fp, "%c", img[x][y+j][k]);
      }
    }
  fclose (fp);
return 1;
}
