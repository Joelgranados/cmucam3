#include <stdlib.h>
#include "LPC2100.h"
#include "cc3.h"
#include "cc3_pin_defines.h"
#include "serial.h"

void
disable_ext_interrupt ()
{
  // Enable bit 14, which is the external interrupt 0...
  REG (VICIntEnClr) = 0x4000;
  REG (PCB_PINSEL1) = 0x0;

}


void
enable_ext_interrupt ()
{
  REG (SYSCON_EXTINT) = 0x1;
// Enable bit 14, which is the external interrupt 0...
  REG (VICIntEnable) = 0x4000;
  // Clear the interrupt status flag...
  REG (PCB_PINSEL1) = 0x1;
}


void
interrupt ()
{
  
  uart0_write("EXT Interrupt!\r\n");
  REG (GPIO_IOCLR) = _CC3_BUF_WEE;	//BUF_WEE=0
  // Before returning, wait for the external interrupt line to got high...
  //while (REG(SYSCON_EXTINT) & 1);
  disable_ext_interrupt ();
}


void segfault()
{
  uart0_write("segfault\r\n");
  exit(-1);
}
