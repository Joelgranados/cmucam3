#include "LPC2100.h"
#include "lpc_config.h"

void
cc3_system_setup ()
{

  // --- enable and connect the PLL (Phase Locked Loop) ---
        // a. set multiplier and divider
        REG(SYSCON_PLLCFG) = MSEL | (1<<PSEL1) | (0<<PSEL0);
        // b. enable PLL
        REG(SYSCON_PLLCON) = (1<<PLLE);
        // c. feed sequence
        REG(SYSCON_PLLFEED) = PLL_FEED1;
        REG(SYSCON_PLLFEED) = PLL_FEED2;
        // d. wait for PLL lock (PLOCK bit is set if locked)
        while (!(REG(SYSCON_PLLSTAT) & (1<<PLOCK)));
        // e. connect (and enable) PLL
        REG(SYSCON_PLLCON) = (1<<PLLE) | (1<<PLLC);
        // f. feed sequence
        REG(SYSCON_PLLFEED) = PLL_FEED1;
        REG(SYSCON_PLLFEED) = PLL_FEED2;

        // --- setup and enable the MAM (Memory Accelerator Module) ---
        // a. start change by turning of the MAM (redundant)
        REG(MAMCR) = 0;
        // b. set MAM-Fetch cycle to 3 cclk as recommended for >40MHz
        REG(MAMTIM) = MAM_FETCH;
        // c. enable MAM
        REG(MAMCR) = MAM_MODE;

        // --- set VPB speed ---
        REG(SYSCON_VPBDIV) = VPBDIV_VAL;

        // --- map INT-vector ---
	REG(SYSCON_MEMMAP) = MEMMAP_USER_FLASH_MODE;
	
	//REG(PCB_PINSEL1) = 0x1;  // External interrupt 0
 

}
