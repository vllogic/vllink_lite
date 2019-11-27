/***************************************************************************
 *   Copyright (C) 2018 - 2019 by Chen Le <talpachen@gmail.com>            *
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#ifndef __PROJ_CFG_LVL2_H__
#define __PROJ_CFG_LVL2_H__

#define PROJ_CFG_COMPOSITE_DEVICE
#define PROJ_CFG_CMSIS_DAP_V2_SUPPORT
#define PROJ_CFG_WEBUSB_SUPPORT
#define PROJ_CFG_CDCEXT_SUPPORT
//#define PROJ_CFG_CDCSHELL_SUPPORT


#define PROJ_CFG_DAP_STANDARD_ENABLE				1
#define PROJ_CFG_DAP_VERDOR_UART_ENABLE				1
#define PROJ_CFG_DAP_VERDOR_BOOTLOADER_ENABLE		0
#define PROJ_CFG_USART_EXT_ENABLE					1
#define PROJ_CFG_USART_TRST_SWO_ENABLE				1

/*
	DAP Config
*/
#define DAP_SWD						1
#define DAP_JTAG					1
#define DAP_JTAG_DEV_CNT			8
#define DAP_DEFAULT_PORT			1
#define DAP_DEFAULT_SWJ_CLOCK		4000000
#define DAP_CTRL_PACKET_SIZE		64
#define DAP_BULK_PACKET_SIZE		512
#ifdef VSFUSBD_CFG_HIGHSPEED
#	define DAP_HID_PACKET_SIZE		512
#else
#	define DAP_HID_PACKET_SIZE		64
#endif
#if DAP_BULK_PACKET_SIZE > DAP_HID_PACKET_SIZE
#	define DAP_PACKET_SIZE			DAP_BULK_PACKET_SIZE
#else
#	define DAP_PACKET_SIZE			DAP_HID_PACKET_SIZE
#endif
#define DAP_PACKET_COUNT			4
#define SWO_UART					1
#define SWO_UART_MAX_BAUDRATE		3200000
#define SWO_UART_MIN_BAUDRATE		16000
#define SWO_MANCHESTER				0
#define SWO_BUFFER_SIZE				128

//#define DAP_VENDOR					"Vllogic.com"
//#define DAP_PRODUCT					"Vllogic Lite"

#endif // __PROJ_CFG_LVL2_H__
