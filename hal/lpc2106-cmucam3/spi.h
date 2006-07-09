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


#ifndef INC_SPI_H
#define INC_SPI_H

#include "LPC2100.h"

#define MAXIM_SET_BAUD_MASK	0x000f
#define MAXIM_B600		0xf
#define MAXIM_B1200		0xe
#define MAXIM_B2400		0xd
#define MAXIM_B4800		0xc
#define MAXIM_B9600		0xb
#define MAXIM_B19200		0xa
#define MAXIM_B38400		0x9
#define MAXIM_B57600		0x2
#define MAXIM_B115200	0x1
#define MAXIM_B230400	0x0

#define MAXIM_SET_LENGTH_MASK	0x0010
#define MAXIM_8BIT		0
#define MAXIM_7BIT		1

#define MAXIM_PARITY_ENABLE_MASK	0x0020
#define MAXIM_PARITY_ENABLE		0x0020

#define MAXIM_STOPBIT_MASK			0x0040
#define MAXIM_ONE_STOP				0x0000
#define MAXIM_TWO_STOP				0x0040

#define MAXIM_CMD_MASK				0xc000
#define MAXIM_CMD_WRITE_CONFIG	0xc000
#define MAXIM_CMD_READ_CONFIG		0x4000
#define MAXIM_CMD_WRITE_DATA		0x8000
#define MAXIM_CMD_READ_DATA		0x0000

#define SPI_CPHA		BIT(3)
#define SPI_CPOL		BIT(4)
#define SPI_MASTER	BIT(5)
#define SPI_LSBF		BIT(6)
#define SPI_SPIE		BIT(7)



#define SSP_DATA_SIZE_MASK		(0x000f)
#define SSP_DATA_SIZE(A)	(A-1)

#define SSP_FRAME_FORMAT_MASK	(0x0030)
#define SSP_FRAME_FORMAT_SPI	(0x0000)
#define SSP_FRAME_FORMAT_SSI	(0x0010)
#define SSP_FRAME_FORMAT_MW	(0x0020)

#define SSP_CPOL		BIT(6)
#define SSP_CPHA		BIT(7)

#define SSP_SCR_MASK				(0xff00)
#define SSP_SCR(A)				(A << 8)

#define SSP_LBM		BIT(0)
#define SSP_SSE		BIT(1)
#define SSP_MS			BIT(2)
#define SSP_SOD		BIT(3)

#define SSP_TFE		BIT(0)
#define SSP_TNF		BIT(1)
#define SSP_RNE		BIT(2)
#define SSP_RFF		BIT(3)
#define SSP_BSY		BIT(4)

#define SSP_RORMIS	BIT(0)
#define SSP_RTMIS		BIT(1)
#define SSP_RXMIS		BIT(2)
#define SSP_TXMIS		BIT(3)

#define SSP_RORIC		BIT(0)
#define SSP_RTIC		BIT(1)

#define SSP_RORIM		BIT(0)
#define SSP_RTIM		BIT(1)
#define SSP_RXIM		BIT(2)
#define SSP_TXIM		BIT(3)

#define SSP_RORRIS	BIT(0)
#define SSP_RTRIS		BIT(1)
#define SSP_RXRIS		BIT(2)
#define SSP_TXRIS		BIT(3)

void cc3_spi0_init(void);

#endif  // INC_SPI_H

