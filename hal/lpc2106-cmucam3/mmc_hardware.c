/*
 * Copyright 2006  Anthony Rowe and Adam Goode
 */

/******************************************************
 *
 * MMC interface routines.
 *
 * (C) 2005 - Tom Walsh tom@openhardware.net
 *
 *
******************************************************/

/*
 * This file is part of cc3.
 *
 * cc3 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * cc3 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with cc3; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "LPC2100.h"
#include "spi.h"
#include "cc3_pin_defines.h"
#include "cc3.h"
#include "mmc_hardware.h"

#include <time.h>
#include "rdcf2.h"

#define MMC_CMD_SIZE 8
uint8_t MMCCmd[MMC_CMD_SIZE];

uint8_t cmdReset[] = { 0x40, 0x00, 0x00, 0x00, 0x00, 0x95 };
uint8_t cmdInitCard[] = { 0x41, 0x00, 0x00, 0x00, 0x00, 0xFF };
uint8_t cmdSetBlock[] = { CMD16, 0, 0, 2, 0, 0xff };

/******************************************************
 *
 * EXT2 routines to detect card insertion / removal
 *
******************************************************/


/******************************************************
 *
 * spi0Init - transport media to communicate with
 * the Flash drive.
 *
******************************************************/
void cc3_spi0_init (void)
{                               // setup basic operation of the SPI0 controller.
  // set pins for SPI0 operation.
  // REG(PCB_PINSEL0) = REG(PCB_PINSEL0) | _CC3_SPI_PINSEL;
  REG (PCB_PINSEL0) =
    ((~_CC3_SPI_MASK) & REG (PCB_PINSEL0)) | _CC3_SPI_PINSEL;
  //REG(PCB_PINSEL0) = 0x5500;
  // set clock rate to approx 7.4975 MHz?
  REG (SPI_SPCCR) = 8;
  // just turn on master mode for now.
  // clock is rising edge, pre-drive data bit before clock.
  // Most significant bit shifted out first.
  REG (SPI_SPCR) = 0x20;        // bit 5 (master mode), all others 0
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
{                               // select SPI target and light the LED.
  //printf("selectMMC\r\n");

  REG (GPIO_IODIR) |= _CC3_MMC_CS;      // switch chip select to output
  REG (GPIO_IOCLR) = _CC3_MMC_CS;       // chip select (neg true)
  cc3_set_led (true);
}

static void unselectMMC (void)
{                               // unselect SPI target and extinguish the LED.
  //printf("unselectMMC\r\n");

  REG (GPIO_IOSET) = _CC3_MMC_CS;       // chip select (neg true)
  REG (GPIO_IODIR) &= ~_CC3_MMC_CS;     // switch chip select to input
  cc3_set_led (false);
}

static void spiPutByte (uint8_t inBuf)
{                               // spit a byte of data at the MMC.
  //printf("spiPutByte 0x%x ", inBuf);
  uint32_t to;
  to = 0;
  REG (SPI_SPDR) = SPI_SPDR_MASK & inBuf;
  while (!(REG (SPI_SPSR) & _CC3_SPI_SPIF));    // wait for bit

  //printf("(SPI_SPSR 0x%x)\r\n", (uint8_t) REG(SPI_SPSR));

  // clear bit
  dummyReader = (uint8_t) REG (SPI_SPDR) & SPI_SPDR_MASK;
}

static uint8_t spiGetByte (void)
{                               // read one byte from the MMC card.
  uint8_t result;
  uint32_t to;
  to = 0;
  //printf("sgb ");
  REG (SPI_SPDR) = SPI_SPDR_MASK & 0xFF;        // fake value, maybe XXX
  while (!(REG (SPI_SPSR) & _CC3_SPI_SPIF));    // wait for bit
  //printf("(SPI_SPSR 0x%x) ", (uint8_t) REG(SPI_SPSR));
  result = (uint8_t) REG (SPI_SPDR) & SPI_SPDR_MASK;

  //printf("0x%x\r\n", result);

  return result;
}

/***************************************************************
 *
 * SPI_Send
 *
 *   Send N bytes from buf into SPI
 * 
***************************************************************/
static void SPI_Send (const uint8_t * buf, long Length)
{
  while (Length != 0) {
    spiPutByte (*buf);
    Length--;
    buf++;
  }
}

/***************************************************************
 *
 * SPI_Read
 *
 * Reads N bytes into buffer
 *
***************************************************************/
static void SPI_Read (uint8_t * buf, long Length)
{
  int i;
  for (i = 0; i < Length; i++) {
    buf[i] = spiGetByte ();
  }
}


/******************************************************
 *
 * mmcStatus
 *
 * get response status byte and see if it matches
 * what we are looking for.
 * return true if we DID get what we wanted.
 *
******************************************************/
static bool mmcStatus (uint8_t response)
{
  int i;
  int count = 4000;

  uint8_t resultStatus;

  for (i = 0; i < count; i++) {
    resultStatus = spiGetByte ();
    if (resultStatus == response) {
      return true;                    // happy end
    }
  }

  DriveDesc.IsValid = false;          // reset MMC subsystem
  return false;                       // sad end
}

/******************************************************
 *
 * mmcInit
 *
 * condition MMC for operation
 *
 * Returns true if all went well or false if not good.
 *
******************************************************/
bool mmcInit (void)
{
  unsigned int count;
  bool result;
  unselectMMC ();
  for (count = 0; count < 10; count++)
    spiPutByte (0xFF);
  selectMMC ();
  SPI_Send (cmdReset, 6);
  if (mmcStatus (StatusIdle) == false) {
    unselectMMC ();
    spiGetByte ();
    return false;
  }
  count = 255;
  do {
    SPI_Send (cmdInitCard, 6);
    count--;
  } while ((!mmcStatus (0x00)) && (count > 0));
  unselectMMC ();
  spiGetByte ();
  if (count == 0)
    return false;
  selectMMC ();
  SPI_Send (cmdSetBlock, 6);
  result = mmcStatus (0x00);
  unselectMMC ();
  spiGetByte ();
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
 * return true on error, false if all went well.
 *
******************************************************/
bool mmcReadBlock (long sector, uint8_t * buf)
{
  sector <<= 1;
  selectMMC ();
  MMCCmd[0] = CMD17;
  MMCCmd[1] = (sector >> 16) & 0xff;
  MMCCmd[2] = (sector >> 8) & 0xff;
  MMCCmd[3] = sector & 0xff;
  MMCCmd[4] = 0;
  MMCCmd[5] = 0xff;
  SPI_Send (MMCCmd, 6);
  if (mmcStatus (0x00)) {
    // get the data token for single block read.
    if (mmcStatus (0xFE)) {
      SPI_Read (buf, RDCF_SECTOR_SIZE);
      // read off, and discard, CRC bytes.
      spiGetByte ();
      spiGetByte ();
    }
    else {
      unselectMMC ();
      spiGetByte ();
      return true;
    }
  }
  else {
    unselectMMC ();
    spiGetByte ();
    return true;
  }
  unselectMMC ();
  spiGetByte ();
  return false;
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
  int count = 60000l;
  uint8_t result = 0;
  while (result == 0 && --count)
    result = spiGetByte ();
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
 * returns true on error, false if all went well.
 *
******************************************************/

bool mmcWriteBlock (long sector, const uint8_t * buf)
{
  int result = 0;
  sector <<= 1;
  selectMMC ();
  MMCCmd[0] = CMD24;
  MMCCmd[1] = (sector >> 16) & 0xff;
  MMCCmd[2] = (sector >> 8) & 0xff;
  MMCCmd[3] = sector & 0xff;
  MMCCmd[4] = 0;
  MMCCmd[5] = 0xff;
  SPI_Send (MMCCmd, 6);
  if (mmcStatus (0x00)) {
    // send data token for single block write.
    spiPutByte (0xFE);
    SPI_Send (buf, RDCF_SECTOR_SIZE);
    // dummy the CRC.
    spiPutByte (0xff);
    spiPutByte (0xff);
    // next we see if all went well.
    result = spiGetByte ();
    if ((result & 0xf) != StatusDataAccepted) {
      // something went wrong with block write.
      unselectMMC ();
      spiGetByte ();
      return true;
    }
  }
  else {
    // failed to get "ok" for CMD24.
    unselectMMC ();
    spiGetByte ();
    return true;
  }
  // wait for operation to complete itself.
  // we wait until the card is no longer busy.
  result = getWriteResultCode ();
  // no longer busy, proceed.
  unselectMMC ();
  spiGetByte ();
  return false;
}

// vi:nowrap:
