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



static void
uart0_setup (void)
{
  // enable access to divisor latch regs
  REG (UART0_LCR) = LCR_ENABLE_LATCH_ACCESS;
  // set divisor for desired baud
  REG (UART0_DLM) = 0;
  REG (UART0_DLL) = BAUD_115200;
  //REG(UART0_DLL) = BAUD_19200;                                          
  // disable access to divisor latch regs (enable access to xmit/rcv fifos
  // and int enable regs)
  REG (UART0_LCR) = LCR_DISABLE_LATCH_ACCESS;
  // disable all UART0 interrupts
  REG (UART0_IER) = 0;
  // setup fifo control reg - trigger level 0 (1 byte fifos), no dma
  // disable fifos (450 mode)
  REG (UART0_FCR) = 0;
  // setup line control reg - disable break transmittion, even parity,
  // 1 stop bit, 8 bit chars
  //REG(UART0_LCR) = 0x1B;
  REG (UART0_LCR) = 0x03;	// Turn off even parity 
}

void uart1_setup()
{
  REG (UART1_LCR) = LCR_ENABLE_LATCH_ACCESS;
  REG (UART1_DLM) = 0;
  REG (UART1_DLL) = BAUD_9600;
  REG (UART1_LCR) = LCR_DISABLE_LATCH_ACCESS;
  REG (UART1_IER) = 0;
  REG (UART1_FCR) = 0;
  REG (UART1_LCR) = 0x03;	// Turn off even parity 
}



/*
 * InitializeUART0
 *
 * This function sets up the UARTS.
 * It configures baudrate, parity, stop bits etc.
 * Call this once at the start of the program.
 *
 */
static void
InitializeUARTs (void)
{
  /*
   * UART0 setup
   * 8bits, 1 stop bit, no parity, 115200 BAUD
   *
   */

#if 0
  // enable access to divisor latch regs
  REG (UART0_LCR) = LCR_ENABLE_LATCH_ACCESS;
  // set divisor for desired baud
  REG (UART0_DLM) = BAUD_115200_MSB;
  REG (UART0_DLL) = BAUD_115200_LSB;
  //REG(UART0_DLL) = BAUD_19200;                                          
  // disable access to divisor latch regs (enable access to xmit/rcv fifos
  // and int enable regs)
  REG (UART0_LCR) = LCR_DISABLE_LATCH_ACCESS;
#endif

  // disable all UART0 interrupts
  REG (UART0_IER) = 0;
  // setup fifo control reg - trigger level 0 (1 byte fifos), no dma
  // disable fifos (450 mode)
  REG (UART0_FCR) = 0;
  // setup line control reg - disable break transmittion, even parity,
  // 1 stop bit, 8 bit chars
  //REG(UART0_LCR) = 0x1B;
  REG (UART0_LCR) = 0x03;	// Turn off even parity 

  /*
   * UART1 setup
   * 8bits, 1 stop bit, no parity, 9600 BAUD
   *
   */
  //REG(UART1_LCR) = LCR_ENABLE_LATCH_ACCESS;
  //REG(UART1_DLM) = BAUD_4800_MSB;                                                   
  //REG(UART1_DLL) = BAUD_4800_LSB;                                           
  //REG(UART1_DLM) = BAUD_9600_MSB;                                                   
  //REG(UART1_DLL) = BAUD_9600_LSB;                                           
  //REG(UART1_LCR) = LCR_DISABLE_LATCH_ACCESS;
  REG (UART1_IER) = 0;
  REG (UART1_FCR) = 0;
  REG (UART1_LCR) = 0x03;	// Turn off even parity 
}





/*******************************************************
 * Interrupt based UART functions
 ******************************************************/

/*
 * InitializeUART0_Int
 *
 * This function sets up the UART0.
 * It configures baudrate, parity, stop bits etc.
 * Call this once at the start of the program.
 *
 */
static void
InitializeUART0_Int (void)
{
  /*
   * UART0 setup
   * 8bits, 1 stop bit, no parity, 115200 BAUD
   */

#if 0
  // enable access to divisor latch regs
  REG (UART0_LCR) = LCR_ENABLE_LATCH_ACCESS;
  // set divisor for desired baud
  REG (UART0_DLM) = BAUD_115200_MSB;
  REG (UART0_DLL) = BAUD_115200_LSB;
  // disable access to divisor latch regs 
  REG (UART0_LCR) = LCR_DISABLE_LATCH_ACCESS;
#endif

  // enable UART0 interrupts
  REG (UART0_IER) = 0;
  // setup fifo control reg 
  // enable FIFO (will reset Rx and Tx FIFO)
  // set Rx trigger level to 8 bytes
  REG (UART0_FCR) = (2 << 6) | 1;
  // setup line control reg 
  // 8 bit char length
  // no stop bit
  // no parity
  // diable break transmission
  // disable divisor latch access
  REG (UART0_LCR) = 0x03;
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
