#include "LPC2100.h"
#include "serial.h"
#include "libc.h"

void
printf (const char *format, ...)
{
  void *args = (void *) &format + sizeof (format);
  char text[200];
  sprintf_args (text, format, args);
  WriteStringUART0 (text);
}

/*
 * get_buffer()
 *
 * This is a blocking function that uses the ARM's hardware UART0 to read serial characters
 * the characters are written to buffer, at most maxlen characters are read
 * the number of characters is returned
 *
 */
int
get_buffer (char *buffer, int maxlen)
{
  while ((REG (UART0_LSR) & LSR_RBR_EMPTY) == 0);
  return REG (UART0_RBR);
}


/*
 * getc()
 *
 * This is a blocking function that uses the ARM's hardware UART0 to read serial characters
 *
 */
char
getc ()
{
  while ((REG (UART0_LSR) & LSR_RBR_EMPTY) == 0);
  return REG (UART0_RBR);
}

/*
 * getc_nb()
 *
 * This is a non-blocking function that check the ARM's hardware UART0 for data.
 * If there is data, the data is returned and getc_status is set to 1.
 * On no data, 0 is returned and getc_status is set to 0.
 * Use getc_status as a global flag to detect new data, not a 0 value returned.
 *
 */
char
getc_nb ()
{
  if ((REG (UART0_LSR) & LSR_RBR_EMPTY) == 0)
    {
      getc_status = 0;
      return 0;
    }
  else
    {
      getc_status = 1;
      return REG (UART0_RBR);
    }
}

/*
 * getc1()
 *
 * Same as getc, only for UART1 instead of 0
 *
 */
char
getc1 ()
{
  while ((REG (UART1_LSR) & LSR_RBR_EMPTY) == 0);
  return REG (UART1_RBR);
}

/*
 * getc_nb1()
 *
 * Same as getc_nb, except for UART1 instead of 0
 *
 */
char
getc_nb1 ()
{
  if ((REG (UART1_LSR) & LSR_RBR_EMPTY) == 0)
    {
      getc_status1 = 0;
      return 0;
    }
  else
    {
      getc_status1 = 1;
      return REG (UART1_RBR);
    }
}

void
uart0_setup (void)
{
  int i, xflag;
  _4BYTE xdata;
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
void
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
  getc_status = 0;
  getc_status1 = 0;
}


/*
 * WriteStringUART0
 *
 * This is a simple function that outputs a constant string over UART0
 *
 */
void
WriteStringUART0 (char *str)
{
  int i;
  i = 0;
  while (str[i] != 0)
    putc (str[i++]);
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
void
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


void
uart_send (unsigned char *data, int length)
{


}
