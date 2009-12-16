/******************************************************
 *
 * MMC interface routines.
 *
 * (C) 2005 - Tom Walsh tom@openhardware.net
 *
 *
******************************************************/

/*
 * Copyright 2006-2007  Anthony Rowe and Adam Goode
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


#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "LPC2100.h"
#include "spi.h"
#include "cc3_pin_defines.h"
#include "cc3.h"
#include "mmc_hardware.h"

#include "interrupt.h"

#include <time.h>
#include "rdcf2.h"

#define MMC_CMD_SIZE 8

static const uint8_t cmdReset[] = { 0x40, 0x00, 0x00, 0x00, 0x00, 0x95 };
static const uint8_t cmdInitCard[] = { 0x41, 0x00, 0x00, 0x00, 0x00, 0xFF };
static const uint8_t cmdSetBlock[] = { CMD16, 0, 0, 2, 0, 0xff };


#define SIZEOF_DIR_ENTRY	32

/*******************************************************************
 * structure defs
 *******************************************************************/

struct PARTITION_ENTRY {
  // single partition entry for MMC.
  uint8_t BootActive;
  uint32_t FirstPartitionSector:24;
  uint8_t FileSystemDescriptor;
  uint32_t LastPartitionSector:24;
  uint32_t FirstSectorPosition;
  uint32_t NumberSectorsInPartition;
} __attribute__ ((packed));


struct PARTITION_TABLE {
  uint8_t BootCode[446];
  struct PARTITION_ENTRY PartitionEntry0;
  struct PARTITION_ENTRY PartitionEntry1;
  struct PARTITION_ENTRY PartitionEntry2;
  struct PARTITION_ENTRY PartitionEntry3;
  uint16_t Signature;
} __attribute__ ((packed));


struct BOOTSECTOR_ENTRY {
  // this description is for an MMC boot block not for MSDOS per se.
  uint32_t JumpCommand:24;
  char OEM_NAME[8];
  uint16_t BytesPerSector;
  uint8_t SectorsPerCluster;
  uint16_t ReservedSectors;
  uint8_t NumberOfFATs;
  uint16_t NumberRootDirEntries;
  uint16_t NumberOfSectorsOnMedia;
  uint8_t MediaDescriptor;
  uint16_t SectorsPerFAT;
  uint16_t SectorsPerTrack;
  uint16_t NumberOfHeads;
  uint32_t NumberOfHiddenSectors;
  uint32_t NumberOfTotalSectors;
  uint8_t DriveNumber;
  uint8_t __RESERVED__;
  uint8_t ExtendedBootSignature;
  uint32_t VolumeID;
  char VolumeLabel[11];
  char FileSystemType[8];
  uint8_t LoadProgramCode[448];
  uint16_t Signature;
} __attribute__ ((packed));

struct DATABUFFER {
  uint8_t data[RDCF_SECTOR_SIZE];
};

// re-use buffer area, don't waste RAM.
union MMC_IO_BUFFER {
  struct BOOTSECTOR_ENTRY BootBlock;
  struct PARTITION_TABLE PartitionTable;
};


/*******************************************************************
 * structure vars
 *******************************************************************/
DRIVE_DESCRIPTION DriveDesc;


static void CollectDataAboutDrive (union MMC_IO_BUFFER *IoBuffer)
{
  // How large are the FAT tables?
  DriveDesc.SectorsPerFAT = IoBuffer->BootBlock.SectorsPerFAT;
  // Important info to decode FAT entries into sectors.
  DriveDesc.SectorsPerCluster = IoBuffer->BootBlock.SectorsPerCluster;
  // First File Allocation Table.
  DriveDesc.FirstFatSector = DriveDesc.SectorZero +
    IoBuffer->BootBlock.ReservedSectors;
  // "backup" copy of the First FAT. usually to undelete files.
  if (IoBuffer->BootBlock.NumberOfFATs > 1) {
    DriveDesc.SecondFatSector =
      DriveDesc.FirstFatSector + DriveDesc.SectorsPerFAT;
  }
  else {
    // nope, only one FAT...
    DriveDesc.SecondFatSector = -1;
  }
  // Where does the actual drive data area start?
  if (DriveDesc.SecondFatSector == -1) {
    // only one FAT, so data follows first FAT.
    DriveDesc.RootDirSector = DriveDesc.FirstFatSector +
      IoBuffer->BootBlock.SectorsPerFAT;
  }
  else {
    // data follows both FAT tables.
    DriveDesc.RootDirSector = DriveDesc.FirstFatSector +
      (2 * IoBuffer->BootBlock.SectorsPerFAT);
  }
  // How many entries can be in the root directory?
  DriveDesc.NumberRootDirEntries = IoBuffer->BootBlock.NumberRootDirEntries;
  // where does cluster 2 begin?
  DriveDesc.DataStartSector = DriveDesc.RootDirSector +
    (DriveDesc.NumberRootDirEntries * SIZEOF_DIR_ENTRY) / RDCF_SECTOR_SIZE;
  // where does the partition end?
  DriveDesc.MaxDataSector = DriveDesc.SectorZero +
    ((IoBuffer->BootBlock.NumberOfSectorsOnMedia) ?
     IoBuffer->BootBlock.NumberOfSectorsOnMedia :
     IoBuffer->BootBlock.NumberOfTotalSectors);
}



static void spi0_init (void)
{
  // setup basic operation of the SPI0 controller.
  // set pins for SPI0 operation.
  REG (PCB_PINSEL0) =
    ((~_CC3_SPI_MASK) & REG (PCB_PINSEL0)) | _CC3_SPI_PINSEL;

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

static void selectMMC (void)
{                               // select SPI target and light the LED.
  //printf("selectMMC\r\n");
  disable_button_interrupt();      // button is multiplexed with CS
  REG (GPIO_IODIR) |= _CC3_MMC_CS;      // switch chip select to output
  REG (GPIO_IOCLR) = _CC3_MMC_CS;       // chip select (neg true)
}

static void unselectMMC (void)
{                               // unselect SPI target and extinguish the LED.
  //printf("unselectMMC\r\n");

  REG (GPIO_IOSET) = _CC3_MMC_CS;       // chip select (neg true)
  REG (GPIO_IODIR) &= ~_CC3_MMC_CS;     // switch chip select to input
  if (!_cc3_button_trigger) {
    enable_button_interrupt();    // button is multiplexed with CS
  }
}

static void spiPutByte (uint8_t inBuf)
{                               // spit a byte of data at the MMC.
  //printf("spiPutByte 0x%x ", inBuf);
  uint8_t dummyReader;
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
static bool mmcInit (void)
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
 * _cc3_mmc_block_read
 *
 * called with:
 *  sector number to read, target buffer addr
 *  offset into sector and number of bytes read.
 *
 * return true on error, false if all went well.
 *
******************************************************/
bool _cc3_mmc_block_read (uint32_t sector, uint8_t * buf)
{
  uint8_t MMCCmd[MMC_CMD_SIZE];

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
  unsigned int count = 100000;
  uint8_t result = 0;
  while (result == 0 && count>0)
  {
    result = spiGetByte ();
    count--;
  }
  return result;
}


/******************************************************
 *
 * _cc3_mmc_block_write
 *
 * called with:
 *   sector to write and source buffer.
 * this always writes RDCF_SECTOR_SIZE chunks of one sector
 *
 * returns true on error, false if all went well.
 *
******************************************************/

bool _cc3_mmc_block_write (uint32_t sector, const uint8_t * buf)
{
  int result = 0;
  uint8_t MMCCmd[MMC_CMD_SIZE];

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



/********************************************************************
 * _cc3_mmc_init
 * return true if error, false if everything went well.
 ********************************************************************/
bool _cc3_mmc_init (void)
{
  // access drive and collect structure info.

  union MMC_IO_BUFFER *IoBuffer;

  // see if we have a card inserted.
  /* no detect on cmucam3 */
  /*
     if (IO0PIN & MMC_DETECT_BIT) {
     // drive disappeared!
     DriveDesc.IsValid = false;
     return true;
     }
  */

  // initialize the SPI0 controller.
  spi0_init();

  if (DriveDesc.IsValid) {
    // we already know about this drive.
    return false;
  }

  // init temporary structure
  IoBuffer = calloc(1, sizeof(union MMC_IO_BUFFER));
  if (IoBuffer == NULL) {
    return true;
  }

  if (mmcInit () == false) {
    free(IoBuffer);
    return true;
  }

  // get the partition table to find the boot block.
  if (_cc3_mmc_block_read (0, (uint8_t *) IoBuffer)) {
    free(IoBuffer);
    return true;
  }

  // validate.
  if (IoBuffer->PartitionTable.Signature != 0xaa55) {
    free(IoBuffer);
    return true;
  }

  // get the boot block now.
  DriveDesc.SectorZero =
    IoBuffer->PartitionTable.PartitionEntry0.FirstSectorPosition;
  if (_cc3_mmc_block_read (DriveDesc.SectorZero, (uint8_t *) IoBuffer)) {
    free(IoBuffer);
    return true;
  }

  // validate.
  if (IoBuffer->BootBlock.Signature != 0xaa55) {
    free(IoBuffer);
    return true;
  }

  // looks good, make a note of where stuff starts at.
  CollectDataAboutDrive (IoBuffer);

  // initialize free cluster cache
  DriveDesc.FirstPossiblyEmptyCluster = 2;

  // pass all tests before validating drive.
  DriveDesc.IsValid = true;

  free(IoBuffer);
  return false;
}


void _cc3_mmc_idle (void)
{
  REG (PCB_PINSEL0) =
    ((~_CC3_SPI_MASK) & REG (PCB_PINSEL0)) | _CC3_SPI_PINSEL_DISABLE;
}
