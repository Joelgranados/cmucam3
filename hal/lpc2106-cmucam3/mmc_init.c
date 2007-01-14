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


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "rdcf2.h"
#include "mmc_hardware.h"

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


/********************************************************************
 * initMMCdrive
 * return true if error, false if everything went well.
 ********************************************************************/

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

bool initMMCdrive (void)
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
  if (mmcReadBlock (0, (uint8_t *) IoBuffer)) {
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
  if (mmcReadBlock (DriveDesc.SectorZero, (uint8_t *) IoBuffer)) {
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
  // pass all tests before validating drive.
  DriveDesc.IsValid = true;

  free(IoBuffer);
  return false;
}
