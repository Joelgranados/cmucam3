/******************************************************************************
 * based on software from:
 * Copyright 2004, R O SoftWare
 * No guarantees, warrantees, or promises, implied or otherwise.
 * May be used for hobby or commercial purposes provided copyright
 * notice remains intact.
 *****************************************************************************/

/*
 * Copyright 2006  Anthony Rowe and Adam Goode
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */


#include <stdio.h>
#include <stdbool.h>

#include "LPC2100.h"
#include "cc3.h"
#include "serial.h"

#define PINSEL_BITPIN0  0
#define PINSEL_BITPIN1  2
// #define PINSEL_BITPIN2  4
#define PINSEL_FIRST_ALT_FUNC   1
// #define PINSEL_SECOND_ALT_FUNC   2

// Values of Bits 0-3 in PINSEL to activate UART0
#define UART0_PINSEL    ((PINSEL_FIRST_ALT_FUNC<<PINSEL_BITPIN0)|(PINSEL_FIRST_ALT_FUNC<<PINSEL_BITPIN1))
// Mask of Bits 0-4
#define UART0_PINMASK      (0x0000000F)    /* PINSEL0 Mask for UART0 */

// REG(UART0__LCR devisor latch bit 
#define UART0_LCR_DLAB  7


cc3_uart_binmode_t _cc3_uart0_binmode;
cc3_uart_binmode_t _cc3_uart1_binmode;


/*    baudrate divisor - use UART_BAUD macro
 *    mode - see typical modes (uart.h)
 *    fmode - see typical fmodes (uart.h)
 */
void _cc3_uart0_setup(uint16_t baud, uint8_t mode, uint8_t fmode)
{
  // setup Pin Function Select Register (Pin Connect Block) 
  // make sure old values of Bits 0-4 are masked out and
  // set them according to UART0-Pin-Selection
  printf( "cc3_uart0_setup()\n" );
}

void _cc3_uart1_setup(uint16_t baud, uint8_t mode, uint8_t fmode)
{
  // setup Pin Function Select Register (Pin Connect Block) 
  // make sure old values of Bits 0-4 are masked out and
  // set them according to UART0-Pin-Selection
  //REG(PCB_PINSEL0) = (REG(PCB_PINSEL0) & ~UART1_PINMASK) | UART1_PINSEL;

  printf( "cc3_uart1_setup()\n" );

}

uint8_t cc3_uart_get_count(void) 
{
  return 2;
}



bool cc3_uart_init (uint8_t uart, 
		    cc3_uart_rate_t rate, 
		    cc3_uart_mode_t mode,
		    cc3_uart_binmode_t binmode)
{
  static bool started;
/*
  uint16_t rate_bits = UART_BAUD(rate);
  uint8_t mode_bits;

  if (!started) {
    // C stuff
    setbuf(stdout, NULL);
    setbuf(stdin, NULL);
    started = true;
  }

  if (uart >= cc3_get_uart_count()) {
    return false;
  }

  switch (mode) {
  default:
  case CC3_UART_MODE_8N1:
    mode_bits = UART_8N1;
    break;
  case CC3_UART_MODE_7N1:
    mode_bits = UART_7N1;
    break;
  case CC3_UART_MODE_8N2:
    mode_bits = UART_8N2;
    break;
  case CC3_UART_MODE_7N2:
    mode_bits = UART_7N2;
    break;
  case CC3_UART_MODE_8E1:
    mode_bits = UART_8E1;
    break;
  case CC3_UART_MODE_7E1:
    mode_bits = UART_7E1;
    break;
  case CC3_UART_MODE_8E2:
    mode_bits = UART_8E2;
    break;
  case CC3_UART_MODE_7E2:
    mode_bits = UART_7E2;
    break;
  case CC3_UART_MODE_8O1:
    mode_bits = UART_8O1;
    break;
  case CC3_UART_MODE_7O1:
    mode_bits = UART_7O1;
    break;
  case CC3_UART_MODE_8O2:
    mode_bits = UART_8O2;
    break;
  case CC3_UART_MODE_7O2:
    mode_bits = UART_7O2;
    break;
  }

  switch (uart) {
  case 0:
    _cc3_uart0_setup (rate_bits, mode_bits, UART_FIFO_8);
    _cc3_uart0_binmode = binmode;
    break;
  case 1:
    _cc3_uart1_setup (rate_bits, mode_bits, UART_FIFO_8);
    _cc3_uart1_binmode = binmode;
    break;
  }
*/
  return true;
}

FILE *cc3_uart_fopen(uint8_t uart, const char *mode)
{
 // char buf[8];  // safe, because uint8_t

 // snprintf(buf, 8, "COM%d:", uart);
 // return fopen(buf, mode);
 return stdout;
}

bool cc3_uart_has_data(uint8_t uart)
{
  /*switch (uart) {
  case 0:
    return ((REG (UART0_LSR) & LSR_RBR_EMPTY)==0 );
  case 1:
    return ((REG (UART1_LSR) & LSR_RBR_EMPTY)==0 );
  default:
    return false;
  }*/
return 0;
  
}


char uart0_putc(const char c)
{
  //while ((REG(UART0_LSR) & LSR_THR_EMPTY) == 0);
 // REG(UART0_THR) = c;
  putchar(c); 
  return c;
}

char uart1_putc(const char c)
{
  //while((REG(UART1_LSR) & LSR_THR_EMPTY) == 0);
  //REG(UART1_THR) = c;
  putchar(c);
  return c;
}


/*
 * uart0_getc()
 *
 * This is a blocking function that uses the ARM's hardware UART0 to read serial characters
 *
 */
int uart0_getc()
{
  //while ((REG (UART0_LSR) & LSR_RBR_EMPTY) == 0);
  //return REG (UART0_RBR);
  char c;
  c=getchar();
  return c;
}

/*
 * uart0_getc_nb()
 *
 * This is a non-blocking function that check the ARM's hardware UART0 for data.
 * If there is data, the data is returned.
 * On no data, -1 is returned.
 *
 */
int uart0_getc_nb()
{
/*
  if ((REG (UART0_LSR) & LSR_RBR_EMPTY) == 0) {
    return -1;
  }
  else {
    return REG (UART0_RBR);
  }*/
return 0;
}


/*
 * uart1_getc()
 *
 * This is a blocking function that uses the ARM's hardware UART1 to read serial characters
 *
 */
int uart1_getc()
{
 // while ((REG (UART1_LSR) & LSR_RBR_EMPTY) == 0);
 // return REG (UART1_RBR);
  return 0;
 
}

/*
 * uart1_getc_nb()
 *
 * This is a non-blocking function that check the ARM's hardware UART1 for data.
 * If there is data, the data is returned.
 * On no data, -1 is returned.
 *
 */
int uart1_getc_nb()
{
 /* if ((REG (UART1_LSR) & LSR_RBR_EMPTY) == 0) {
    return -1;
  }
  else {
    return REG (UART1_RBR);
  }*/
  return 0;
}


void uart0_write (char *str)
{
  while (*str != '\0') {
    uart0_putc(*str++);
  }
}

void uart0_write_hex (unsigned int i)
{
  char buffer[8];
  int b = 7;
  int tmp;

  while (b >= 0) {
    tmp = i % 16;
    if (tmp < 10) {
      buffer[b] = tmp + 0x30;
    } else {
      buffer[b] = tmp + 0x37;
    }
    i /= 16;
    b--;
  }

  uart0_write("0x");
  for (b = 0; b < 8; b++) {
    uart0_putc(buffer[b]);
  }
  uart0_write("\r\n");
}
