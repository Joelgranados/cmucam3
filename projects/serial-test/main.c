#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <cc3.h>

#include "fnv.h"

int main (void) {
  // configure uarts
  cc3_uart_init (0,
		 CC3_UART_RATE_115200,
		 CC3_UART_MODE_8N1,
		 CC3_UART_BINMODE_BINARY);
  // Make it so that stdout and stdin are not buffered
  setbuf(stdout, NULL);
  setbuf(stdin, NULL);

  // wait
  getchar();

  // go
  uint32_t val = FNV1_32_INIT;
  while (true) {
    // print
    printf("%#.8x\n", val);

    // most significant byte first
    uint8_t buf[4];
    buf[0] = (val >> 24) & 0xFF;
    buf[1] = (val >> 16) & 0xFF;
    buf[2] = (val >> 8) & 0xFF;
    buf[3] = (val >> 0) & 0xFF;

    val = fnv_32_buf(buf, 4, val);
  }

  return 0;
}
