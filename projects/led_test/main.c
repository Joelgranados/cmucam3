#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <cc3.h>

// led test
int main (void) {
  // setup system
  cc3_system_setup ();

  // configure uarts
  cc3_uart_init (0,
		 CC3_UART_RATE_115200,
		 CC3_UART_MODE_8N1,
		 CC3_UART_BINMODE_BINARY);
  // Make it so that stdout and stdin are not buffered
  setvbuf(stdout, NULL, _IONBF, 0 );
  setvbuf(stdin, NULL, _IONBF, 0 );

  cc3_camera_init ();

  while (true) {
    int i;
    for (i = 0; i < 8; i++) {
      if (i & 1) {
	printf ("0");
	cc3_set_led(0);
      } else {
	printf ("_");
	cc3_clr_led(0);
      }

      if (i & 2) {
	printf ("1");
	cc3_set_led(1);
      } else {
	printf ("_");
	cc3_clr_led(1);
      }

      if (i & 4) {
	printf ("2");
	cc3_set_led(2);
      } else {
	printf ("_");
	cc3_clr_led(2);
      }

      printf("\r\n");
      cc3_wait_ms(200);
    }
  }


  return 0;
}
