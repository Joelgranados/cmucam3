#include "cc3.h"
#include "LPC2100.h"
#include "lpc_config.h"

void
cc3_system_setup (void)
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
	
	// Setup timer0 to count by milliseconds starting from 0
	REG(TIMER0_TCR)=0;   // turn off timer
	REG(TIMER0_MCR)=0;    // disable interrupt 
	REG(TIMER0_TC)=0;    // clear counter
	REG(TIMER0_PC)=0;    // clear prescale count 
	REG(TIMER0_PR) = (int)((FOSC*PLL_M)/1000);  // every 1 ms
	REG(TIMER0_TCR)=1;   // start the timer 

	
}
