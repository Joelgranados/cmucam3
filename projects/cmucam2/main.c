#include "LPC2100.h"
#include "cc3.h"
#include "interrupt.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include "serial.h"

#define IMG_X	88
#define IMG_Y	144



int
main ()
{
  unsigned int i = 0;
  int val;

  cc3_system_setup();
  
  
  //disable_ext_interrupt ();
  //uart0_write("CMUcam3 v2 Starting up...\r");
  //system_setup ();
  cc3_io_init(115200);
  cc3_camera_init();
  
  
  printf ("CMUcam3 Starting up\r\n");


  cc3_set_led(true);
  cc3_set_raw_register(0x14, 0x20);	// set low resolutiun
  //camera_set_reg (0x14, 0x00);        // sets high resolutiun
  cc3_set_raw_register(0x13, 0x21);	 
  cc3_set_raw_register(0x12, 0x2C);	// color mode RGB White balance
  cc3_set_raw_register(0x11, 0x00);

  printf ("Registers Loaded\r\n");
  for(i=0; i<50; i++ )
  {
  	cc3_pixbuf_load();
  	printf (".\r\n");
  }
  printf ("Sending Image\r\n");
  while(1)
      {
	  image_send_direct(IMG_X, IMG_Y);
      }
  
  
  return 0;
}

