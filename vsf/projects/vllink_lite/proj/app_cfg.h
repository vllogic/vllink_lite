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

#define APPCFG_VSFTIMER_NUM				16
//#define APPCFG_BUFMGR_SIZE				1024

//#define APPCFG_HRT_QUEUE_LEN			0
#define APPCFG_SRT_QUEUE_LEN			16
#define APPCFG_NRT_QUEUE_LEN			16

#if (defined(APPCFG_HRT_QUEUE_LEN) && (APPCFG_HRT_QUEUE_LEN > 0)) ||\
	(defined(APPCFG_SRT_QUEUE_LEN) && (APPCFG_SRT_QUEUE_LEN > 0)) ||\
	(defined(APPCFG_NRT_QUEUE_LEN) && (APPCFG_NRT_QUEUE_LEN > 0))
#define VSFSM_CFG_PREMPT_EN				1
#else
#define VSFSM_CFG_PREMPT_EN				0
#endif

// define APPCFG_USR_POLL for round robin scheduling
//#define APPCFG_USR_POLL

#ifdef APPCFG_USR_POLL
#	define APPCFG_TICKCLK_PRIORITY		-1
#else
#	define APPCFG_TICKCLK_PRIORITY		VSFHAL_TICKCLK_PRIORITY
#endif

#define APPCFG_PENDSV_PRIORITY			VSFHAL_PENDSV_PRIORITY

/*******************************************************************************
	Bootloader Paramter Config
*******************************************************************************/
#if defined(BOARD_TYPE_VLLINK_LITE)
#	define APPCFG_USBD_VID					0x0D28
#	define APPCFG_USBD_PID					0x0204
#	define APPCFG_USBD_BCD					0x0100
#elif defined(BOARD_TYPE_VLLINK_LITE_BOOTLOADER)
#	define APPCFG_USBD_VID					0x1209
#	define APPCFG_USBD_PID					0x6666
#	define APPCFG_USBD_BCD					0x0001
#else
#	error "UNKNOWN PRODUCT"
#endif

