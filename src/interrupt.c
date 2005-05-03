#include "LPC2100.h"
#include "cmucam.h"

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
  REG (GPIO_IOCLR) = BUF_WEE;	//BUF_WEE=0
  // Before returning, wait for the external interrupt line to got high...
  //while (REG(SYSCON_EXTINT) & 1);
  disable_ext_interrupt ();
}
