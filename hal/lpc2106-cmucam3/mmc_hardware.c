/******************************************************
 *
 * MMC interface routines.
 *
 * (C) 2005 - Tom Walsh tom@openhardware.net
 *
 *
******************************************************/
#include "LPC2100.h"
#include <stdbool.h>
#include <stdint.h>
#include <mmc_hardware.h>
#include <spi.h>
#include <time.h>
#include <rdcf2.h>

#define MMC_CMD_SIZE 8
uint8_t MMCCmd[MMC_CMD_SIZE];

uint8_t cmdReset [] = { 0x40, 0x00, 0x00, 0x00, 0x00, 0x95 };
uint8_t cmdInitCard [] = { 0x41, 0x00, 0x00, 0x00, 0x00, 0xFF };
uint8_t cmdSetBlock [] = { CMD16, 0, 0, 2, 0, 0xff };

/******************************************************
 *
 * EXT2 routines to detect card insertion / removal
 *
******************************************************/


/******************************************************
 *
 * spi1Init - transport media to communicate with
 * the Flash drive.
 *
******************************************************/
void spi1Init (void)
{// setup basic operation of the SPI1 controller.
		// set pins for SPI1 operation.
	PINSEL1 = (PINSEL1 & ~SSP_PINMASK) | SSP_PINSEL;
		// set clock rate to approx 3.93MHz (58.98MHz / 15);
	SSPCPSR = 2;
		// just turn on master mode for now.
		// clock is rising edge, pre-drive data bit before clock.
		// Most significant bit shifted out first.
	SSPCR0 = (SSP_DATA_SIZE(8) | SSP_FRAME_FORMAT_SPI);
	SSPCR1 = SSP_SSE;
}

/******************************************************
 *
 * selectMMC and unselectMMC
 *
 * board specific routines to communicate with MMC.
 *
******************************************************/

static unsigned char dummyReader;

static void selectMMC (void)
{// select SPI target and light the LED.
	IO0CLR = MMC_CS_BIT;		// set SS = 1 (off)
	IO1CLR = LED_MMC_BIT;
}

static void unselectMMC (void)
{// unselect SPI target and extinguish the LED.
	IO0SET = MMC_CS_BIT;		// set SS = 1 (off)
	IO1SET = LED_MMC_BIT;
}

static void spiPutByte(uint8_t inBuf)
{// spit a byte of data at the MMC.
	SSPDR = (REG16) inBuf; while (SSPSR & SSP_BSY);
		// dummy read clears SPI BSY flag on LPC2xxx processors.
	dummyReader = (uint8_t) SSPDR;
}

static uint8_t spiGetByte(void)
{// read one byte from the MMC card.
	SSPDR = (REG16) 0xff; while (SSPSR & SSP_BSY);
	return (uint8_t) SSPDR;
}

/***************************************************************
 *
 * SPI_Send
 *
 *   Send N bytes from buf into SPI
 * 
***************************************************************/
static void SPI_Send(const uint8_t *buf, long Length )
{
	uint8_t Dummy;
	if ( Length == 0 ) return;
	while ( Length != 0 ) {
			/* as long as TNF bit's set, TxFIFO isn¿t full, write */
		while ( !(SSPSR & 0x02) );
		SSPDR = *buf;
			/* Wait until the Busy bit is cleared */
		while ( !(SSPSR & 0x04) );
		Dummy = SSPDR; /* Flush the RxFIFO */
		Length--;
		buf++;
	}
	return;
}

/***************************************************************
 *
 * SPI_Read
 *
 * Reads N bytes into buffer
 *
***************************************************************/
static void SPI_Read (uint8_t *buf, long Length)
{
int	i;
	for (i=0; i<Length; i++) {
		SSPDR = (REG16) 0xff; while (SSPSR & SSP_BSY);
		buf[i] = SSPDR;
	}
}


/******************************************************
 *
 * mmcStatus
 *
 * get response status byte and see if it matches
 * what we are looking for.
 * return True if we DID get what we wanted.
 *
******************************************************/
static bool mmcStatus(uint8_t response)
{
int count = 4000;
uint8_t	resultStatus;
	resultStatus = ~response;
	while (resultStatus != response && --count) resultStatus = spiGetByte();
	return (count != 0);			// loop was exited before timeout
}

/******************************************************
 *
 * mmcInit
 *
 * condition MMC for operation
 *
 * Returns True if all went well or False if not good.
 *
******************************************************/
bool mmcInit(void)
{
unsigned int count;
bool	result;
	unselectMMC();
	for (count = 0; count < 10; count++) spiPutByte(0xFF);
	selectMMC();
	SPI_Send (cmdReset, 6);
	if (mmcStatus(StatusIdle) == False) { unselectMMC(); spiGetByte(); return False; }
	count = 255;
	do {
		SPI_Send (cmdInitCard, 6);
		count--;
	} while ((!mmcStatus(0x00)) && (count > 0));
	unselectMMC();
	spiGetByte();
	if (count == 0) return False;
	selectMMC();
	SPI_Send (cmdSetBlock, 6);
	result = mmcStatus(0x00);
	unselectMMC(); spiGetByte();
	return result;
}

/******************************************************
 *
 * mmcReadBlock
 *
 * called with:
 *  sector number to read, target buffer addr
 *  offset into sector and number of bytes read.
 *
 * return True on error, False if all went well.
 *
******************************************************/
bool mmcReadBlock(long sector, uint8_t * buf)
{
	sector <<= 1;
	selectMMC();
	MMCCmd [0] = CMD17;
	MMCCmd [1] = (sector>>16) & 0xff; MMCCmd [2] = (sector>>8) & 0xff;
	MMCCmd [3] = sector & 0xff; MMCCmd [4] = 0;
	MMCCmd [5] = 0xff;
	SPI_Send (MMCCmd, 6);
	if (mmcStatus(0x00)) {
			// get the data token for single block read.
		if (mmcStatus(0xFE)) {
			SPI_Read (buf, RDCF_SECTOR_SIZE);
				// read off, and discard, CRC bytes.
			spiGetByte(); spiGetByte();
		} else { unselectMMC(); spiGetByte(); return True; }
	} else { unselectMMC(); spiGetByte(); return True; }
	unselectMMC(); spiGetByte(); return False;
}

/******************************************************
 *
 * getWriteResultCode
 *
 * MMC will be busy (0xff), wait until status code
 * comes back and return that
 *
******************************************************/
static uint8_t getWriteResultCode (void)
{
int	count = 60000l;
uint8_t	result = 0;
	while (result == 0 && --count) result = spiGetByte();
	return result;
}


/******************************************************
 *
 * mmcWriteBlock
 *
 * called with:
 *   sector to write and source buffer.
 * this always writes RDCF_SECTOR_SIZE chunks of one sector
 *
 * returns True on error, False if all went well.
 *
******************************************************/

bool mmcWriteBlock(long sector, const uint8_t * buf)
{
int	result = 0;
	sector <<= 1;
	selectMMC();
	MMCCmd [0] = CMD24;
	MMCCmd [1] = (sector>>16) & 0xff; MMCCmd [2] = (sector>>8) & 0xff;
	MMCCmd [3] = sector & 0xff; MMCCmd [4] = 0;
	MMCCmd [5] = 0xff;
	SPI_Send (MMCCmd, 6);
	if (mmcStatus(0x00)) {
			// send data token for single block write.
		spiPutByte(0xFE);
		SPI_Send (buf, RDCF_SECTOR_SIZE);
			// dummy the CRC.
		spiPutByte(0xff); spiPutByte(0xff);
			// next we see if all went well.
		result = spiGetByte();
		if ((result & 0xf) != StatusDataAccepted) {
				// something went wrong with block write.
			unselectMMC();	
			spiGetByte();
			return True;
		}
	} else {
			// failed to get "ok" for CMD24.
		unselectMMC();	spiGetByte(); return True;
	}
		// wait for operation to complete itself.
		// we wait until the card is no longer busy.
	result = getWriteResultCode();
		// no longer busy, proceed.
	unselectMMC(); spiGetByte();
	return False;
}

// vi:nowrap:
