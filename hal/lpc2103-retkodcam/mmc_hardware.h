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

bool _cc3_mmc_block_write (uint32_t sector, const uint8_t * buf);
bool _cc3_mmc_block_read  (uint32_t sector, uint8_t * buf);
bool _cc3_mmc_init (void);
void _cc3_mmc_idle (void);

#endif
