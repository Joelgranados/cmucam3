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


#ifndef LPC2100_H
#define LPC2100_H


#define FASTIO
// write address to register macro
#define REG(addr) (*(volatile unsigned long *)(addr))

/* Watchdog Timer (32 bit data bus) */
#define WDOG_BASE                        (0xE0000000)	/*  */
#define WDOG_WDMOD                       (0xE0000000)	/*  */
#define WDOG_WDTC                        (0xE0000004)	/*  */
#define WDOG_WDFEED                      (0xE0000008)	/*  */
#define WDOG_WDTV                        (0xE000000C)	/*  */

/* Timer 0 (32 bit data bus) */
#define TIMER0_BASE  (0xE0004000)
#define TIMER0_IR    (0xE0004000)	/* intr reg */
#define TIMER0_TCR   (0xE0004004)	/* ctrl */
#define TIMER0_TC    (0xE0004008)	/* counter */
#define TIMER0_PR    (0xE000400C)	/* prescale reg */
#define TIMER0_PC    (0xE0004010)	/* prescale cnt */
#define TIMER0_MCR   (0xE0004014)	/* match ctrl  */
#define TIMER0_MR0   (0xE0004018)	/* match reg0  */
#define TIMER0_MR1   (0xE000401C)	/* match reg1  */
#define TIMER0_MR2   (0xE0004020)	/* match reg2  */
#define TIMER0_MR3   (0xE0004024)	/* match reg3  */
#define TIMER0_CCR   (0xE0004028)	/* capt ctrl  */
#define TIMER0_CR0   (0xE000402C)	/* capt reg0  */
#define TIMER0_CR1   (0xE0004030)	/* capt reg1  */
#define TIMER0_CR2   (0xE0004034)	/* capt reg2  */
#define TIMER0_CR3   (0xE0004038)	/* capt reg3  */
#define TIMER0_EMR   (0xE000403C)	/* ext match reg  */

/* Timer 1 (32 bit data bus) */
#define TIMER1_BASE   (0xE0008000)
#define TIMER1_IR     (0xE0008000)	/* Interrupt register */
#define TIMER1_TCR    (0xE0008004)	/* Timer Control register */
#define TIMER1_TC     (0xE0008008)	/* Timer Counter */
#define TIMER1_PR     (0xE000800C)	/* Prescale register */
#define TIMER1_PC     (0xE0008010)	/* Prescale Counter */
#define TIMER1_MCR    (0xE0008014)	/* Match Control register  */
#define TIMER1_MR0    (0xE0008018)	/* Match reg0  */
#define TIMER1_MR1    (0xE000801C)	/* Match reg1  */
#define TIMER1_MR2    (0xE0008020)	/* Match reg2  */
#define TIMER1_MR3    (0xE0008024)	/* Match reg3  */
#define TIMER1_CCR    (0xE0008028)	/* Capture Control register */
#define TIMER1_CR0    (0xE000802C)	/* Capt reg0  */
#define TIMER1_CR1    (0xE0008030)	/* Capt reg1  */
#define TIMER1_CR2    (0xE0008034)	/* Capt reg2  */
#define TIMER1_CR3    (0xE0008038)	/* Capt reg3  */
#define TIMER1_EMR    (0xE000803C)	/* External Match register */

/* UART0 (8 bit data bus) */
#define UART0_BASE        (0xE000C000)
#define UART0_RBR         (0xE000C000)	/* receive buffer-RO */
#define UART0_THR         (0xE000C000)	/* transmit hold buffer-WO */
#define UART0_IER         (0xE000C004)	/* interrupt enable */
#define UART0_IIR         (0xE000C008)	/* interrupt id-RO */
#define UART0_FCR         (0xE000C008)	/* fifo control-WO */
#define UART0_LCR         (0xE000C00C)	/* line control */
#define UART0_LSR         (0xE000C014)	/* line status-RO */
#define UART0_SCR         (0xE000C01C)	/* scratchpad  */
#define UART0_DLL         (0xE000C000)	/* divisor latch LSB */
#define UART0_DLM         (0xE000C004)	/* divisor latch MSB */

/* UART1 (8 bit data bus) */
#define UART1_BASE       (0xE0010000)
#define UART1_RBR        (0xE0010000)	/* receive buffer-RO */
#define UART1_THR        (0xE0010000)	/* transmit hold buffer-WO */
#define UART1_IER        (0xE0010004)	/* interrupt enable */
#define UART1_IIR        (0xE0010008)	/* interrupt id-RO */
#define UART1_FCR        (0xE0010008)	/* fifo control-WO */
#define UART1_LCR        (0xE001000C)	/* line control */
#define UART1_MCR        (0xE0010010)	/* modem control */
#define UART1_LSR        (0xE0010014)	/* line status-RO */
#define UART1_MSR        (0xE0010018)	/* modem status-RO */
#define UART1_SCR        (0xE001001C)	/* scratchpad  */
#define UART1_DLL        (0xE0010000)	/* divisor latch LSB */
#define UART1_DLM        (0xE0010004)	/* divisor latch MSB */

/* PWM0 (32 bit data bus) */
#define PWM0_BASE        (0xE0014000)
#define PWM0_IR          (0xE0014000)	/* intr reg */
#define PWM0_TCR         (0xE0014004)	/* timer ctrl */
#define PWM0_TC          (0xE0014008)	/* timer counter */
#define PWM0_PR          (0xE001400C)	/* prescale reg */
#define PWM0_PC          (0xE0014010)	/* prescale count */
#define PWM0_MCR         (0xE0014014)	/* match ctrl reg  */
#define PWM0_MR0         (0xE0014018)	/* match reg0  */
#define PWM0_MR1         (0xE001401C)	/* match reg1  */
#define PWM0_MR2         (0xE0014020)	/* match reg2  */
#define PWM0_MR3         (0xE0014024)	/* match reg3  */
#define PWM0_CCR         (0xE0014028)	/* capt ctrl  */
#define PWM0_CR0         (0xE001402C)	/* capt reg0  */
#define PWM0_CR1         (0xE0014030)	/* capt reg1  */
#define PWM0_CR2         (0xE0014034)	/* capt reg2  */
#define PWM0_CR3         (0xE0014038)	/* capt reg3  */
#define PWM0_EMR         (0xE001403C)	/* ext match reg  */
#define PWM0_MR4         (0xE0014040)	/* match reg4  */
#define PWM0_MR5         (0xE0014044)	/* match reg5  */
#define PWM0_MR6         (0xE0014048)	/* match reg6  */
#define PWM0_PCR         (0xE001404C)	/* pwm ctrl reg  */
#define PWM0_LER         (0xE0014050)	/* latch enable reg  */

/* PWM1 (32 bit data bus) --addresses reserved from base + 0x18000 to 0xE00018050 */

/* I2C (8/16 bit data bus) */
#define I2C_BASE           (0xE001C000)
#define I2C_I2CONSET       (0xE001C000)	/* ctrl set reg */
#define I2C_I2STAT         (0xE001C004)	/* status reg-RO */
#define I2C_I2DAT          (0xE001C008)	/* data reg  */
#define I2C_I2ADR          (0xE001C00C)	/* addr reg  */
#define I2C_I2SCLH         (0xE001C010)	/* scl dutycycle hi  */
#define I2C_I2SCLL         (0xE001C014)	/* scl dutycycle lo  */
#define I2C_I2CONCLR       (0xE001C018)	/* ctrl clr reg */

/* SPI (8 bit data bus) (spec shows 0xE0020000 - 0xE0023FFF) */
#define SPI_BASE		(0xE0020000)
#define SPI_SPCR		(0xE0020000)	/* Control Register */
#define SPI_SPCR_POR		(uint32_t)0x00000000
#define SPI_SPCR_MASK		(uint32_t)0x000000F8
#define SPI_SPSR		(0xE0020004)	/* Status Register */
#define SPI_SPSR_POR		(uint32_t)0x00000000
#define SPI_SPSR_MASK		(uint32_t)0x000000F8
#define SPI_SPDR		(0xE0020008)	/* Data Register */
#define SPI_SPDR_POR		(uint32_t)0x00000000
#define SPI_SPDR_MASK		(uint32_t)0x000000FF
#define SPI_SPCCR		(0xE002000C)	/* Clock Counter Register */
#define SPI_SPCCR_POR		(uint32_t)0x00000000
#define SPI_SPCCR_MASK		(uint32_t)0x000000FE
#define SPI_SPTCR		(0xE0020010)	/* Test Control Register */
#define SPI_SPTCR_POR		(uint32_t)0x00000000
#define SPI_SPTCR_MASK		(uint32_t)0x00000001
#define SPI_SPTSR		(0xE0020014)	/* Test Status Register */
#define SPI_SPTSR_POR		(uint32_t)0x00000000
#define SPI_SPTSR_MASK		(uint32_t)0x000000F8
#define SPI_SPTOR		(0xE0020018)	/* Test Observe Register */

/* RTC (32 bit data bus) */
#define RTC_BASE           (0xE0024000)
#define RTC_ILR            (0xE0024000)	/* Interrupt Location Register */
#define RTC_CTC            (0xE0024004)	/* Clock Tick Counter */
#define RTC_CCR            (0xE0024008)	/* Clock Register */
#define RTC_CIIR           (0xE002400C)	/* Clock Increment Interrupt Register */
#define RTC_AMR            (0xE0024010)	/* Alarm Mask Register */
#define RTC_CTIME0         (0xE0024014)	/* Consolidated Timer Register 0 */
#define RTC_CTIME1         (0xE0024018)	/* Consolidated Timer Register 1 */
#define RTC_CTIME2         (0xE002401C)	/* Consolidated Timer Register 2 */
#define RTC_SECOND         (0xE0024020)	/* Seconds value */
#define RTC_SEC            (0xE0024020)	/* Seconds value */
#define RTC_MINUTE         (0xE0024024)	/* Minutes value */
#define RTC_MIN            (0xE0024024)	/* Minutes value */
#define RTC_HOUR           (0xE0024028)	/* Hours value */
#define RTC_DAY_OF_MONTH   (0xE002402C)	/* Day of month value */
#define RTC_DAY_OF_WEEK    (0xE0024030)	/* Day of week value */
#define RTC_DAY_OF_YEAR    (0xE0024034)	/* Day of year value */
#define RTC_MONTH          (0xE0024038)	/* Month value */
#define RTC_YEAR           (0xE002403C)	/* Year value */
#define RTC_ALSEC          (0xE0024060)	/* Alarm value for seconds */
#define RTC_ALMIN          (0xE0024064)	/* Alarm value for minutes */
#define RTC_ALHOUR         (0xE0024068)	/* Alarm value for hours */
#define RTC_ALDOM          (0xE002406C)	/* Alarm value for day of month */
#define RTC_ALDOW          (0xE0024070)	/* Alarm value for day of week */
#define RTC_ALDOY          (0xE0024074)	/* Alarm value for day of year */
#define RTC_ALMON          (0xE0024078)	/* Alarm value for months */
#define RTC_ALYEAR         (0xE002407C)	/* Alarm value for years */
#define RTC_PREINT         (0xE0024080)	/* Prescale value, integer portion */
#define RTC_PREFRAC        (0xE0024084)	/* Prescale value, fractional portion */


/* General Pupupose IO (GPIO) (32 bit data bus) */
#ifndef FASTIO
  #define GPIO_BASE          (0xE0028000)
  #define GPIO_IOPIN         (0xE0028000)	/* GPIO Pin value reg  */
  #define GPIO_IOSET         (0xE0028004)	/* GPIO Output set reg */
  #define GPIO_IODIR         (0xE0028008)	/* GPIO Direction cntrl reg */
  #define GPIO_IOCLR         (0xE002800C)	/* GPIO Output clear reg */
#else
  #define GPIO_IOPIN         (0x3FFFC014)	/* fast GPIO Pin value reg  */
  #define GPIO_IOSET         (0x3FFFC018)	/* fast GPIO Output set reg */
  #define GPIO_IODIR         (0x3FFFC000)	/* fast GPIO Direction reg */
  #define GPIO_IOCLR         (0x3FFFC01C)	/* fast GPIO Output clear reg */
#endif

/* LPC2106-01 FAST IO extension ports */
// Currently we just rename the original gpio functions, but in the
// future we plan to use these instead.
#define FIO_IODIR	(0x3FFFC000)
#define FIO_IOMASK	(0x3FFFC010)
#define FIO_IOPIN	(0x3FFFC014)
#define FIO_IOSET	(0x3FFFC018)
#define FIO_IOCLR	(0x3FFFC01C)

/* Pin Connect Block (PCB) (32 bit data bus) */
#define PCB_BASE           (0xE002C000)
#define PCB_PINSEL0        (0xE002C000)	/* pin function sel reg 0 */
#define PCB_PINSEL1        (0xE002C004)	/* pin function sel reg 1 */

/* System Control Block (32 bit data bus) */


#define SYSCON_EXTINT      (0xE01FC140)
#define SYSCON_EXTWAKE     (0xE01FC144)
#define SYSCON_MEMMAP      (0xE01FC040)
#define SYSCON_PLLCON      (0xE01FC080)
#define SYSCON_PLLCFG      (0xE01FC084)
#define SYSCON_PLLSTAT     (0xE01FC088)
#define SYSCON_PLLFEED     (0xE01FC08C)
#define SYSCON_PCON        (0xE01FC0C0)
#define SYSCON_PCONP       (0xE01FC0C4)
#define SYSCON_APBDIV      (0xE01FC100)
#define SYSCON_SCS	   (0xE01FC1A0)

// Added for lpc2103
#define SYSCON_EXTMODE	   (0xE01FC148)
#define SYSCON_EXTPOLAR    (0xE01FC14C)


/* MAM control registers  */
#define MAMCR		   (0xE01FC000)
#define MAMTIM		   (0xE01FC004)

/* PERIPHERAL SLOTS #11 thru #125 are unimplemented 
*/

/* FLASHIF (32 bit data bus) 
*/

/***** --- VIC REGISTERS ---  *****/

#define VIC_BASE_ADDR         (0xFFFFF000)

#define VICIRQStatus     (VIC_BASE_ADDR + 0x000)
#define VICFIQStatus     (VIC_BASE_ADDR + 0x004)
#define VICRawIntr       (VIC_BASE_ADDR + 0x008)
#define VICIntSelect     (VIC_BASE_ADDR + 0x00C)
#define VICIntEnable     (VIC_BASE_ADDR + 0x010)
#define VICIntEnClear    (VIC_BASE_ADDR + 0x014)
#define VICIntEnClr      (VIC_BASE_ADDR + 0x014)
#define VICSoftInt       (VIC_BASE_ADDR + 0x018)
#define VICSoftIntClear  (VIC_BASE_ADDR + 0x01C)
#define VICProtection    (VIC_BASE_ADDR + 0x020)
#define VICVectAddr      (VIC_BASE_ADDR + 0x030)
#define VICDefVectAddr   (VIC_BASE_ADDR + 0x034)

#define VICVectAddr_BASE (VIC_BASE_ADDR + 0x100)
#define VICVectAddr_0    (VIC_BASE_ADDR + 0x100)
#define VICVectAddr0     (VIC_BASE_ADDR + 0x100)
#define VICVectAddr_1    (VIC_BASE_ADDR + 0x104)
#define VICVectAddr1     (VIC_BASE_ADDR + 0x104)
#define VICVectAddr_2    (VIC_BASE_ADDR + 0x108)
#define VICVectAddr2     (VIC_BASE_ADDR + 0x108)
#define VICVectAddr_3    (VIC_BASE_ADDR + 0x10C)
#define VICVectAddr3     (VIC_BASE_ADDR + 0x10C)
#define VICVectAddr_4    (VIC_BASE_ADDR + 0x110)
#define VICVectAddr4     (VIC_BASE_ADDR + 0x110)
#define VICVectAddr_5    (VIC_BASE_ADDR + 0x114)
#define VICVectAddr5     (VIC_BASE_ADDR + 0x114)
#define VICVectAddr_6    (VIC_BASE_ADDR + 0x118)
#define VICVectAddr6     (VIC_BASE_ADDR + 0x118)
#define VICVectAddr_7    (VIC_BASE_ADDR + 0x11C)
#define VICVectAddr7     (VIC_BASE_ADDR + 0x11C)
#define VICVectAddr_8    (VIC_BASE_ADDR + 0x120)
#define VICVectAddr8     (VIC_BASE_ADDR + 0x120)
#define VICVectAddr_9    (VIC_BASE_ADDR + 0x124)
#define VICVectAddr9     (VIC_BASE_ADDR + 0x124)
#define VICVectAddr_10   (VIC_BASE_ADDR + 0x128)
#define VICVectAddr10    (VIC_BASE_ADDR + 0x128)
#define VICVectAddr_11   (VIC_BASE_ADDR + 0x12C)
#define VICVectAddr11    (VIC_BASE_ADDR + 0x12C)
#define VICVectAddr_12   (VIC_BASE_ADDR + 0x130)
#define VICVectAddr12    (VIC_BASE_ADDR + 0x130)
#define VICVectAddr_13   (VIC_BASE_ADDR + 0x134)
#define VICVectAddr13    (VIC_BASE_ADDR + 0x134)
#define VICVectAddr_14   (VIC_BASE_ADDR + 0x138)
#define VICVectAddr14    (VIC_BASE_ADDR + 0x138)
#define VICVectAddr_15   (VIC_BASE_ADDR + 0x13C)
#define VICVectAddr15    (VIC_BASE_ADDR + 0x13C)

#define VICVectCntl_BASE (VIC_BASE_ADDR + 0x200)
#define VICVectCntl_0    (VIC_BASE_ADDR + 0x200)
#define VICVectCntl0     (VIC_BASE_ADDR + 0x200)
#define VICVectCntl_1    (VIC_BASE_ADDR + 0x204)
#define VICVectCntl1     (VIC_BASE_ADDR + 0x204)
#define VICVectCntl_2    (VIC_BASE_ADDR + 0x208)
#define VICVectCntl2     (VIC_BASE_ADDR + 0x208)
#define VICVectCntl_3    (VIC_BASE_ADDR + 0x20C)
#define VICVectCntl3     (VIC_BASE_ADDR + 0x20C)
#define VICVectCntl_4    (VIC_BASE_ADDR + 0x210)
#define VICVectCntl4     (VIC_BASE_ADDR + 0x210)
#define VICVectCntl_5    (VIC_BASE_ADDR + 0x214)
#define VICVectCntl5     (VIC_BASE_ADDR + 0x214)
#define VICVectCntl_6    (VIC_BASE_ADDR + 0x218)
#define VICVectCntl6     (VIC_BASE_ADDR + 0x218)
#define VICVectCntl_7    (VIC_BASE_ADDR + 0x21C)
#define VICVectCntl7     (VIC_BASE_ADDR + 0x21C)
#define VICVectCntl_8    (VIC_BASE_ADDR + 0x220)
#define VICVectCntl8     (VIC_BASE_ADDR + 0x220)
#define VICVectCntl_9    (VIC_BASE_ADDR + 0x224)
#define VICVectCntl9     (VIC_BASE_ADDR + 0x224)
#define VICVectCntl_10   (VIC_BASE_ADDR + 0x228)
#define VICVectCntl10    (VIC_BASE_ADDR + 0x228)
#define VICVectCntl_11   (VIC_BASE_ADDR + 0x22C)
#define VICVectCntl11    (VIC_BASE_ADDR + 0x22C)
#define VICVectCntl_12   (VIC_BASE_ADDR + 0x230)
#define VICVectCntl12    (VIC_BASE_ADDR + 0x230)
#define VICVectCntl_13   (VIC_BASE_ADDR + 0x234)
#define VICVectCntl13    (VIC_BASE_ADDR + 0x234)
#define VICVectCntl_14   (VIC_BASE_ADDR + 0x238)
#define VICVectCntl14    (VIC_BASE_ADDR + 0x238)
#define VICVectCntl_15   (VIC_BASE_ADDR + 0x23C)
#define VICVectCntl15    (VIC_BASE_ADDR + 0x23C)

#define VICITCR          (VIC_BASE_ADDR + 0x300)
#define VICITIP1         (VIC_BASE_ADDR + 0x304)
#define VICITIP2         (VIC_BASE_ADDR + 0x308)
#define VICITOP1         (VIC_BASE_ADDR + 0x30C)
#define VICITOP2         (VIC_BASE_ADDR + 0x310)

#define VICPeriphID0     (VIC_BASE_ADDR + 0xFE0)
#define VICPeriphID1     (VIC_BASE_ADDR + 0xFE4)
#define VICPeriphID2     (VIC_BASE_ADDR + 0xFE8)
#define VICPeriphID3     (VIC_BASE_ADDR + 0xFEC)

#define VICPCellID0      (VIC_BASE_ADDR + 0xFF0)
#define VICPCellID1      (VIC_BASE_ADDR + 0xFF4)
#define VICPCellID2      (VIC_BASE_ADDR + 0xFF8)
#define VICPCellID3      (VIC_BASE_ADDR + 0xFFC)



#endif
