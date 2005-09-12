#ifndef SERIAL_H
#define SERIAL_H

#include "LPC2100.h"

/************************************************ UART ****************************/
typedef unsigned int _4BYTE;
typedef unsigned short _2BYTE;
typedef unsigned char _BYTE;
unsigned char ucGlobalTemp;
void uart0_setup (void);

// LPC21000 misc uart0 definitions
#define UART0_PCB_PINSEL_CFG     (_4BYTE)0x00000005
#define UART1_PCB_PINSEL_CFG     (_4BYTE)0x00050005
#define UART0_INT_BIT            (_4BYTE)0x0040
#define LCR_DISABLE_LATCH_ACCESS (_4BYTE)0x00000000
#define LCR_ENABLE_LATCH_ACCESS  (_4BYTE)0x00000080
#define LCR_DISABLE_BREAK_TRANS  (_4BYTE)0x00000000
#define LCR_ODD_PARITY           (_4BYTE)0x00000000
#define LCR_ENABLE_PARITY        (_4BYTE)0x00000008
#define LCR_1_STOP_BIT           (_4BYTE)0x00000000
#define LCR_CHAR_LENGTH_8        (_4BYTE)0x00000003
#define BAUD_9600	    	         144
#define BAUD_19200	    	         72
#define BAUD_115200	    	         12	// 8 for 10MHz, 12 for 14.745MHz
#define LSR_THR_EMPTY           (_4BYTE)0x00000020
#define LSR_RBR_EMPTY           (_4BYTE)0x00000001


char uart0_putc(const char c);
char uart1_putc(const char c);

int uart0_getc();
int uart0_getc_nb();     // return -1 if no char avail

int uart1_getc();
int uart1_getc_nb();     // return -1 if no char avail



#endif
