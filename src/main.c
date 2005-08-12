#include "LPC2100.h"
#include "cmucam.h"
#include "libc.h"
#include "serial.h"
#include "interrupt.h"
#include <math.h>

//#define IMG_X 160     
//#define IMG_Y 240     
#define IMG_X	88
#define IMG_Y	144
//#define IMG_X   320   
//#define IMG_Y 480     
#define STR_MAX 32
#define ACK 1
#define SEND_RAW_IMAGE 2
#define RESET 3
char frame_done = 1;
//unsigned char img[IMG_Y][IMG_X][3];   // Store like this for best pointer stride
int read_command ();


int
main ()
{
  unsigned int i = 0;
  int val;
  system_setup ();
  camera_setup ();
  uart0_setup ();

  REG (GPIO_IOSET) = LED;
  disable_ext_interrupt ();
  printf ("CMUcam3 v2 Starting up...\r");

  camera_set_reg (0x14, 0x20);	// set low resolutiun
  //camera_set_reg (0x14, 0x00);        // sets high resolutiun
  camera_set_reg (0x13, 0x21);	// 
  camera_set_reg (0x12, 0x2C);	// color mode RGB White balance
  camera_set_reg (0x11, 0x00);

  while (1)
    {
      printf (":");
      val = read_command ();
      if (val == -1)
	printf ("NCK\r");
      else
	{
	  printf ("ACK\r");
	  switch (val)
	    {
	    case ACK:
	      break;
	    case RESET:
	      printf ("CMUcam2 v0.0 c6\r");
	      break;
	    case SEND_RAW_IMAGE:
	      image_send_direct (IMG_X, IMG_Y);
	      printf ("done\r");
	      break;
	    default:
	      printf ("PC load letter?\r");
	    }
	}
      // fifo_load_frame ();
      // image_fifo_to_mem (img, IMG_X, IMG_Y);
      // image_send_uart (img, IMG_X, IMG_Y);

    }


  return 0;
}

int
read_command ()
{
  char str[STR_MAX];
  char c;
  int i;

  i = 0;
  c = 0;
  while (c != '\r' && c != ' ')
    {
      c = getc ();
      str[i] = c;
      if (i < STR_MAX - 1)
	i++;
    }
  if (str[0] == '\r')
    return ACK;
  str[i - 1] = '\0';
  if (strcmp (str, "sf") == 0)
    return SEND_RAW_IMAGE;
  if (strcmp (str, "rs") == 0)
    return RESET;
  return -1;

}
