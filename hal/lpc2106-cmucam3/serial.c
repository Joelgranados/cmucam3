#include <stdbool.h>

#include "LPC2100.h"
#include "serial.h"
#include "cc3.h"

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

/*    baudrate divisor - use UART_BAUD macro
 *    mode - see typical modes (uart.h)
 *    fmode - see typical fmodes (uart.h)
 *    NOTE: uart0Init(UART_BAUD(9600), UART_8N1, UART_FIFO_8); 
 */
void _cc3_uart0_setup(uint16_t baud, uint8_t mode, uint8_t fmode)
{
  // setup Pin Function Select Register (Pin Connect Block) 
  // make sure old values of Bits 0-4 are masked out and
  // set them according to UART0-Pin-Selection
  REG(PCB_PINSEL0) = (REG(PCB_PINSEL0) & ~UART0_PINMASK) | UART0_PINSEL;

  REG(UART0_IER) = 0x00;             // disable all interrupts
  REG(UART0_IIR) = 0x00;             // clear interrupt ID register
  REG(UART0_LSR )= 0x00;             // clear line status register

  // set the baudrate - DLAB must be set to access DLL/DLM
  REG(UART0_LCR )= (1<<UART0_LCR_DLAB); // set divisor latches (DLAB)
  REG(UART0_DLL )= (uint8_t)baud;         // set for baud low byte
  REG(UART0_DLM )= (uint8_t)(baud >> 8);  // set for baud high byte
  
  // set the number of characters and other
  // user specified operating parameters
  // Databits, Parity, Stopbits - Settings in Line Control Register
  REG(UART0_LCR )= (mode & ~(1<<UART0_LCR_DLAB)); // clear DLAB "on-the-fly"
  // setup FIFO Control Register (fifo-enabled + xx trig) 
  REG(UART0_FCR) = fmode;
}

void _cc3_uart1_setup(uint16_t baud, uint8_t mode, uint8_t fmode)
{
  // setup Pin Function Select Register (Pin Connect Block) 
  // make sure old values of Bits 0-4 are masked out and
  // set them according to UART0-Pin-Selection
  //REG(PCB_PINSEL0) = (REG(PCB_PINSEL0) & ~UART1_PINMASK) | UART1_PINSEL;
  REG(PCB_PINSEL0) = (REG(PCB_PINSEL0) & ~0x000F0000) | 0x00050000;

  REG(UART1_IER) = 0x00;             // disable all interrupts
  REG(UART1_IIR) = 0x00;             // clear interrupt ID register
  REG(UART1_LSR )= 0x00;             // clear line status register

  // set the baudrate - DLAB must be set to access DLL/DLM
  REG(UART1_LCR )= (1<<UART0_LCR_DLAB); // set divisor latches (DLAB) // same as uart0
  REG(UART1_DLL )= (uint8_t)baud;         // set for baud low byte
  REG(UART1_DLM )= (uint8_t)(baud >> 8);  // set for baud high byte
  
  // set the number of characters and other
  // user specified operating parameters
  // Databits, Parity, Stopbits - Settings in Line Control Register
  REG(UART1_LCR )= (mode & ~(1<<UART0_LCR_DLAB)); // clear DLAB "on-the-fly"
  // setup FIFO Control Register (fifo-enabled + xx trig) 
  REG(UART1_FCR) = fmode;
}



char uart0_putc(const char c)
{
  while ((REG(UART0_LSR) & LSR_THR_EMPTY) == 0);
  REG(UART0_THR) = c;
  
  return c;
}

char uart1_putc(const char c)
{
  while((REG(UART1_LSR) & LSR_THR_EMPTY) == 0);
  REG(UART1_THR) = c;

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
  while ((REG (UART0_LSR) & LSR_RBR_EMPTY) == 0);
  return REG (UART0_RBR);
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
  if ((REG (UART0_LSR) & LSR_RBR_EMPTY) == 0) {
    return -1;
  }
  else {
    return REG (UART0_RBR);
  }
}


/*
 * uart1_getc()
 *
 * This is a blocking function that uses the ARM's hardware UART1 to read serial characters
 *
 */
int uart1_getc()
{
  while ((REG (UART1_LSR) & LSR_RBR_EMPTY) == 0);
  return REG (UART1_RBR);
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
  if ((REG (UART1_LSR) & LSR_RBR_EMPTY) == 0) {
    return -1;
  }
  else {
    return REG (UART1_RBR);
  }
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
