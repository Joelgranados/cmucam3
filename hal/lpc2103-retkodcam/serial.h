/******************************************************************************
 * based on software from:
 * Copyright 2004, R O SoftWare
 * No guarantees, warrantees, or promises, implied or otherwise.
 * May be used for hobby or commercial purposes provided copyright
 * notice remains intact.
 *
 * reduced to learn what has to be done to enable and use UART0
 *****************************************************************************/

/*
 * Copyright 2006  Anthony Rowe and Adam Goode
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */


#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>
#include "LPC2100.h"
#include "lpc_config.h"
#include "cc3.h"


///////////////////////////////////////////////////////////////////////////////
// UART defines

// Interrupt Enable Register bit definitions
#define UIER_ERBFI          (1 << 0)    // Enable Receive Data Available Interrupt
#define UIER_ETBEI          (1 << 1)    // Enable Transmit Holding Register Empty Interrupt
#define UIER_ELSI           (1 << 2)    // Enable Receive Line Status Interrupt
#define UIER_EDSSI          (1 << 3)    // Enable MODEM Status Interrupt

// Interrupt ID Register bit definitions
#define UIIR_NO_INT         (1 << 0)    // NO INTERRUPTS PENDING
#define UIIR_MS_INT         (0 << 1)    // MODEM Status
#define UIIR_THRE_INT       (1 << 1)    // Transmit Holding Register Empty
#define UIIR_RDA_INT        (2 << 1)    // Receive Data Available
#define UIIR_RLS_INT        (3 << 1)    // Receive Line Status
#define UIIR_CTI_INT        (6 << 1)    // Character Timeout Indicator
#define UIIR_ID_MASK        0x0E

// FIFO Control Register bit definitions
#define UFCR_FIFO_ENABLE    (1 << 0)    // FIFO Enable
#define UFCR_RX_FIFO_RESET  (1 << 1)    // Reset Receive FIFO
#define UFCR_TX_FIFO_RESET  (1 << 2)    // Reset Transmit FIFO
#define UFCR_FIFO_TRIG1     (0 << 6)    // Trigger @ 1 character in FIFO
#define UFCR_FIFO_TRIG4     (1 << 6)    // Trigger @ 4 characters in FIFO
#define UFCR_FIFO_TRIG8     (2 << 6)    // Trigger @ 8 characters in FIFO
#define UFCR_FIFO_TRIG14    (3 << 6)    // Trigger @ 14 characters in FIFO

// Line Control Register bit definitions
#define ULCR_CHAR_5         (0 << 0)    // 5-bit character length
#define ULCR_CHAR_6         (1 << 0)    // 6-bit character length
#define ULCR_CHAR_7         (2 << 0)    // 7-bit character length
#define ULCR_CHAR_8         (3 << 0)    // 8-bit character length
#define ULCR_STOP_1         (0 << 2)    // 1 stop bit
#define ULCR_STOP_2         (1 << 2)    // 2 stop bits
#define ULCR_PAR_NO         (0 << 3)    // No Parity
#define ULCR_PAR_ODD        (1 << 3)    // Odd Parity
#define ULCR_PAR_EVEN       (3 << 3)    // Even Parity
#define ULCR_PAR_MARK       (5 << 3)    // MARK "1" Parity
#define ULCR_PAR_SPACE      (7 << 3)    // SPACE "0" Paruty
#define ULCR_BREAK_ENABLE   (1 << 6)    // Output BREAK line condition
#define ULCR_DLAB_ENABLE    (1 << 7)    // Enable Divisor Latch Access

// Modem Control Register bit definitions
#define UMCR_DTR            (1 << 0)    // Data Terminal Ready
#define UMCR_RTS            (1 << 1)    // Request To Send
#define UMCR_LB             (1 << 4)    // Loopback

// Line Status Register bit definitions
#define ULSR_RDR            (1 << 0)    // Receive Data Ready
#define ULSR_OE             (1 << 1)    // Overrun Error
#define ULSR_PE             (1 << 2)    // Parity Error
#define ULSR_FE             (1 << 3)    // Framing Error
#define ULSR_BI             (1 << 4)    // Break Interrupt
#define ULSR_THRE           (1 << 5)    // Transmit Holding Register Empty
#define ULSR_TEMT           (1 << 6)    // Transmitter Empty
#define ULSR_RXFE           (1 << 7)    // Error in Receive FIFO
#define ULSR_ERR_MASK       0x1E

// Modem Status Register bit definitions
#define UMSR_DCTS           (1 << 0)    // Delta Clear To Send
#define UMSR_DDSR           (1 << 1)    // Delta Data Set Ready
#define UMSR_TERI           (1 << 2)    // Trailing Edge Ring Indicator
#define UMSR_DDCD           (1 << 3)    // Delta Data Carrier Detect
#define UMSR_CTS            (1 << 4)    // Clear To Send
#define UMSR_DSR            (1 << 5)    // Data Set Ready
#define UMSR_RI             (1 << 6)    // Ring Indicator
#define UMSR_DCD            (1 << 7)    // Data Carrier Detect


#define UART_BAUD(baud) (uint16_t)(((2*(((FOSC*PLL_M/APBDIV_VAL) / ((baud) * 16))))+1)/2)

///////////////////////////////////////////////////////////////////////////////
// Definitions for typical UART 'baud' settings
//#define B1200         UART_BAUD(1200)
//#define B9600         UART_BAUD(9600)
//#define B19200        UART_BAUD(19200)
//#define B38400        UART_BAUD(38400)
//#define B57600        UART_BAUD(57600)
//#define B115200       UART_BAUD(115200)
//#define B230400	      UART_BAUD(230400)

///////////////////////////////////////////////////////////////////////////////
// Definitions for typical UART 'mode' settings
#define UART_8N1      (uint8_t)(ULCR_CHAR_8 + ULCR_PAR_NO   + ULCR_STOP_1)
#define UART_7N1      (uint8_t)(ULCR_CHAR_7 + ULCR_PAR_NO   + ULCR_STOP_1)
#define UART_8N2      (uint8_t)(ULCR_CHAR_8 + ULCR_PAR_NO   + ULCR_STOP_2)
#define UART_7N2      (uint8_t)(ULCR_CHAR_7 + ULCR_PAR_NO   + ULCR_STOP_2)
#define UART_8E1      (uint8_t)(ULCR_CHAR_8 + ULCR_PAR_EVEN + ULCR_STOP_1)
#define UART_7E1      (uint8_t)(ULCR_CHAR_7 + ULCR_PAR_EVEN + ULCR_STOP_1)
#define UART_8E2      (uint8_t)(ULCR_CHAR_8 + ULCR_PAR_EVEN + ULCR_STOP_2)
#define UART_7E2      (uint8_t)(ULCR_CHAR_7 + ULCR_PAR_EVEN + ULCR_STOP_2)
#define UART_8O1      (uint8_t)(ULCR_CHAR_8 + ULCR_PAR_ODD  + ULCR_STOP_1)
#define UART_7O1      (uint8_t)(ULCR_CHAR_7 + ULCR_PAR_ODD  + ULCR_STOP_1)
#define UART_8O2      (uint8_t)(ULCR_CHAR_8 + ULCR_PAR_ODD  + ULCR_STOP_2)
#define UART_7O2      (uint8_t)(ULCR_CHAR_7 + ULCR_PAR_ODD  + ULCR_STOP_2)

///////////////////////////////////////////////////////////////////////////////
// Definitions for typical UART 'fmode' settings
#define UART_FIFO_OFF (0x00)
#define UART_FIFO_1   (uint8_t)(UFCR_FIFO_ENABLE + UFCR_FIFO_TRIG1)
#define UART_FIFO_4   (uint8_t)(UFCR_FIFO_ENABLE + UFCR_FIFO_TRIG4)
#define UART_FIFO_8   (uint8_t)(UFCR_FIFO_ENABLE + UFCR_FIFO_TRIG8)
#define UART_FIFO_14  (uint8_t)(UFCR_FIFO_ENABLE + UFCR_FIFO_TRIG14)

void _cc3_uart0_setup(uint16_t baud, uint8_t mode, uint8_t fmode);
void _cc3_uart1_setup(uint16_t baud, uint8_t mode, uint8_t fmode);
/************************************************ UART ****************************/


// LPC2100 misc uart0 definitions
#define UART0_PCB_PINSEL_CFG     (uint32_t)0x00000005
#define UART1_PCB_PINSEL_CFG     (uint32_t)0x00050005
#define UART0_INT_BIT            (uint32_t)0x0040
#define LCR_DISABLE_LATCH_ACCESS (uint32_t)0x00000000
#define LCR_ENABLE_LATCH_ACCESS  (uint32_t)0x00000080
#define LCR_DISABLE_BREAK_TRANS  (uint32_t)0x00000000
#define LCR_ODD_PARITY           (uint32_t)0x00000000
#define LCR_ENABLE_PARITY        (uint32_t)0x00000008
#define LCR_1_STOP_BIT           (uint32_t)0x00000000
#define LCR_CHAR_LENGTH_8        (uint32_t)0x00000003
//#define BAUD_9600	    	         144
//#define BAUD_19200	    	         72
//#define BAUD_115200	    	         12	// 8 for 10MHz, 12 for 14.745MHz
//#define BAUD_230400	    	         6	// 8 for 10MHz, 12 for 14.745MHz
#define LSR_THR_EMPTY           (uint32_t)0x00000020
#define LSR_RBR_EMPTY           (uint32_t)0x00000001


extern cc3_uart_binmode_t _cc3_uart0_binmode;
extern cc3_uart_binmode_t _cc3_uart1_binmode;

char uart0_putc(const char c);
char uart1_putc(const char c);

int uart0_getc(void);
int uart0_getc_nb(void);     // return -1 if no char avail

int uart1_getc(void);
int uart1_getc_nb(void);     // return -1 if no char avail


void uart0_write (const char *str);
void uart0_write_hex (unsigned int i);

#endif
