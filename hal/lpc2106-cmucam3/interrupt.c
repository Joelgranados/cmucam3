#include <stdlib.h>
#include "LPC2100.h"
#include "cc3.h"
#include "cc3_pin_defines.h"
#include "serial.h"

void
disable_ext_interrupt (void)
{
  // Enable bit 14, which is the external interrupt 0...
  REG (VICIntEnClr) = 0x4000;
  REG (PCB_PINSEL1) = 0x0;   // Switch from EINT0 mode to GPIO

}


void
enable_ext_interrupt (void)
{
  REG (SYSCON_EXTINT) = 0x1; 
   // Enable bit 14, which is the external interrupt 0...
  REG (VICIntEnable) = 0x4000;
  // Clear the interrupt status flag...
  REG (PCB_PINSEL1) = 0x1;   // Switch from GPIO to EIN0 mode
}


void
interrupt (void)
{
  
  REG (GPIO_IOCLR) = _CC3_BUF_WEE;	//BUF_WEE=0
  // Before returning, wait for the external interrupt line to go high...
  //while (REG(SYSCON_EXTINT) & 1);
  disable_ext_interrupt ();
}


void undefined(void)
{
  uart0_write("undefined instruction!\r\n");
  // XXX: tell us which instruction!
  exit(-1);
}

void swi(void)
{
  uart0_write("swi!\r\n");
  // XXX: tell us something
  exit(-1);
}

void prefetch_abort(void)
{
  register volatile char *r_14 asm("r14");
  long prev_pc = (long)r_14 - 4;
  uart0_write("prefetch abort!\r\n");
  uart0_write_hex(prev_pc);
  // XXX: register dump
  exit(-1);
}

void data_abort(void)
{
  uart0_write("data abort!\r\n");
  // XXX: register dump
  exit(-1);
}
