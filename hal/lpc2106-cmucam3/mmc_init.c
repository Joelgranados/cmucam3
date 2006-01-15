#include <stdio.h>
#include <rdcf2.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <mmc_hardware.h>

#define SIZEOF_DIR_ENTRY	32

extern struct rdcf	fcbs [MaxFileBuffers];

/*******************************************************************
 * structure defs
*******************************************************************/

struct PARTITION_ENTRY	{
	// single partition entry for MMC.
	uint8_t	BootActive;
	uint32_t	FirstPartitionSector:24;
	uint8_t	FileSystemDescriptor;
	uint32_t	LastPartitionSector:24;
	uint32_t	FirstSectorPosition;
	uint32_t	NumberSectorsInPartition;
} __attribute__ ((packed));


struct PARTITION_TABLE {
	uint8_t	BootCode[446];
	struct PARTITION_ENTRY	PartitionEntry0;
	struct PARTITION_ENTRY	PartitionEntry1;
	struct PARTITION_ENTRY	PartitionEntry2;
	struct PARTITION_ENTRY	PartitionEntry3;
	uint16_t	Signature;
} __attribute__ ((packed));


struct BOOTSECTOR_ENTRY {
	// this description is for an MMC boot block not for MSDOS per se.
	uint32_t		JumpCommand:24;
	char		OEM_NAME[8];
	uint16_t	BytesPerSector;
	uint8_t		SectorsPerCluster;
	uint16_t	ReservedSectors;
	uint8_t		NumberOfFATs;
	uint16_t	NumberRootDirEntries;
	uint16_t	NumberOfSectorsOnMedia;
	uint8_t		MediaDescriptor;
	uint16_t	SectorsPerFAT;
	uint16_t	SectorsPerTrack;
	uint16_t	NumberOfHeads;
	uint32_t		NumberOfHiddenSectors;
	uint32_t		NumberOfTotalSectors;
	uint8_t		DriveNumber;
	uint8_t		__RESERVED__;
	uint8_t		ExtendedBootSignature;
	uint32_t		VolumeID;
	char		VolumeLabel[11];
	char		FileSystemType[8];
	uint8_t		LoadProgramCode[448];
	uint16_t	Signature;
} __attribute__ ((packed));

struct DATABUFFER {
	uint8_t	data [RDCF_SECTOR_SIZE];
};

	// re-use buffer area, don't waste RAM.
union MMC_IO_BUFFER {
	struct BOOTSECTOR_ENTRY		BootBlock;
	struct PARTITION_TABLE		PartitionTable;
};

/*******************************************************************
 * structure vars
*******************************************************************/
union MMC_IO_BUFFER IoBuffer;		// scratch area for transitory file i/o.
DRIVE_DESCRIPTION DriveDesc;


/*******************************************************************
 * routines specific to rdcf2 operation.
*******************************************************************/

static void init_rdcf2_struct(void)
{
int	i;
		// remember to do the reserved fcb as well.
	for (i=0; i<MaxFileBuffers+1; i++) {
		fcbs[i].ReadSector = mmcReadBlock;
		fcbs[i].WriteSector = mmcWriteBlock;
		fcbs[i].BufferInUse = false;
	}
}

/********************************************************************
 * initMMCdrive
 * return true if error, false if everything went well.
********************************************************************/

static void CollectDataAboutDrive (void)
{
		// How large are the FAT tables?
	DriveDesc.SectorsPerFAT = IoBuffer.BootBlock.SectorsPerFAT;
		// Important info to decode FAT entries into sectors.
	DriveDesc.SectorsPerCluster = IoBuffer.BootBlock.SectorsPerCluster;
		// First File Allocation Table.
	DriveDesc.FirstFatSector = DriveDesc.SectorZero +
			IoBuffer.BootBlock.ReservedSectors;
		// "backup" copy of the First FAT. usually to undelete files.
	if (IoBuffer.BootBlock.NumberOfFATs > 1) {
		DriveDesc.SecondFatSector = DriveDesc.FirstFatSector + DriveDesc.SectorsPerFAT;
	} else {
			// nope, only one FAT...
		DriveDesc.SecondFatSector = -1;
	}
		// Where does the actual drive data area start?
	if (DriveDesc.SecondFatSector == -1) {
			// only one FAT, so data follows first FAT.
		DriveDesc.RootDirSector = DriveDesc.FirstFatSector +
				IoBuffer.BootBlock.SectorsPerFAT;
	} else {
			// data follows both FAT tables.
		DriveDesc.RootDirSector = DriveDesc.FirstFatSector +
				(2 * IoBuffer.BootBlock.SectorsPerFAT);
	}
		// How many entries can be in the root directory?
	DriveDesc.NumberRootDirEntries = IoBuffer.BootBlock.NumberRootDirEntries;
		// where does cluster 2 begin?
	DriveDesc.DataStartSector = DriveDesc.RootDirSector +
		(DriveDesc.NumberRootDirEntries * SIZEOF_DIR_ENTRY) / RDCF_SECTOR_SIZE;
		// where does the partition end?
	DriveDesc.MaxDataSector = DriveDesc.SectorZero +
		((IoBuffer.BootBlock.NumberOfSectorsOnMedia) ?
		IoBuffer.BootBlock.NumberOfSectorsOnMedia :
		IoBuffer.BootBlock.NumberOfTotalSectors);
}

bool initMMCdrive (void)
{ // access drive and collect structure info.
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
	if (mmcInit() == false) return true;
		// init fcbs.
	init_rdcf2_struct();
		// get the partition table to find the boot block.
	if (mmcReadBlock(0, (uint8_t *) &IoBuffer)) return true;
		// validate.
	if (IoBuffer.PartitionTable.Signature != 0xaa55) return true;
		// get the boot block now.
	DriveDesc.SectorZero =
			IoBuffer.PartitionTable.PartitionEntry0.FirstSectorPosition;
	if (mmcReadBlock(DriveDesc.SectorZero, (uint8_t *) &IoBuffer))
		return true;
		// validate.
	if (IoBuffer.BootBlock.Signature != 0xaa55) return true;
		// looks good, make a note of where stuff starts at.
	CollectDataAboutDrive();
		// pass all tests before validating drive.
	DriveDesc.IsValid = true;
	return false;
}


