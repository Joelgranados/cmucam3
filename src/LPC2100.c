#include "LPC2100.h"


void
system_setup ()
{
  /*  Crank up the juice on the PLL  */
  REG (SYSCON_PLLCON) = 0x03;
  REG (SYSCON_PLLCFG) = 0x05;

  REG (SYSCON_PLLFEED) = 0xAA;
  REG (SYSCON_PLLFEED) = 0x55;

  /* Turn on the MAM  */
  REG (MAMCR) = 0x00;
  REG (MAMTIM) = 0x03;		// 3 cycles per prefetch
  REG (MAMCR) = 0x02;
  REG (SYSCON_MEMMAP) = 0x1;
}
