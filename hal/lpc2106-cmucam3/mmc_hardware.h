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


#ifndef INC_MMC_HARDWARE_H
#define INC_MMC_HARDWARE_H


#define CMD0 0x40               // software reset
#define CMD1 0x41               // brings card out of idle state
#define CMD13 0x4d              // read status word of card.
#define CMD16 0x50              // sets the block length used by the memory card
#define CMD17 0x51              // read single block
#define CMD24 0x58              // writes a single block

#ifndef BIT
#define BIT(n)              (1 << (n))
#endif

#define StatusIdle				BIT(0)  // in IDLE state and initializing process.
#define StatusEraseReset		BIT(1)  // erase sequence aborted.
#define StatusIllegalCmd		BIT(2)  // illegal command was sent.
#define StatusErrorCRC			BIT(3)  // bad CRC in last command.
#define StatusEraseSeqErr		BIT(4)  // error in sequence of erase commands.
#define StatusAddrError			BIT(5)  // address did not match block length.
#define StatusParamError		BIT(6)  // illegal argument used in command.
#define StatusDataAccepted		0x5     // write command is ok to go.
#define StatusDataCrcError		0xb     // crc error on data block.
#define StatusDataWriteError	0xd     // write error on media (?).

bool mmcInit (void);
bool mmcWriteBlock (long sector, const uint8_t * buf);
bool mmcReadBlock (long sector, uint8_t * buf);
bool initMMCdrive (void);

#endif
