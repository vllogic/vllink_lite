#ifndef __PROJ_CFG_H__
#define __PROJ_CFG_H__

#if defined(BOARD_TYPE_VLLINK_LITE)
#	include "proj_cfg_vllink_lite.h"
#elif defined(BOARD_TYPE_VLLINK_LITE_BOOTLOADER)
#	include "proj_cfg_vllink_lite_bootloader.h"
#else
#	error "UNKNOWN PRODUCT"
#endif

#ifdef PROJ_CFG_CMSIS_DAP_V2_SUPPORT
#	define CMSIS_DAP_V2_DESC_LENGTH			23
#	define CMSIS_DAP_V2_INTERFACE_COUNT		1
#else
#	define CMSIS_DAP_V2_DESC_LENGTH			0
#	define CMSIS_DAP_V2_INTERFACE_COUNT		0
#endif
#ifdef PROJ_CFG_WEBUSB_SUPPORT
#	define WEBUSB_DESC_LENGTH				9
#	define WEBUSB_INTERFACE_COUNT			1
#	define WEBUSB_BOS_DESC_LENGTH			24
#	define WEBUSB_BOS_COUNT					1
#else
#	define WEBUSB_DESC_LENGTH				0
#	define WEBUSB_INTERFACE_COUNT			0
#	define WEBUSB_BOS_DESC_LENGTH			0
#	define WEBUSB_BOS_COUNT					0
#endif
#if defined(PROJ_CFG_CMSIS_DAP_V2_SUPPORT)
#	define WINUSB_BOS_DESC_LENGTH			28
#	define WINUSB_BOS_COUNT					1
#else
#	define WINUSB_BOS_DESC_LENGTH			0
#	define WINUSB_BOS_COUNT					0
#endif

#ifdef PROJ_CFG_CDCEXT_SUPPORT
#	define CDCEXT_DESC_LENGTH				66
#	define CDCEXT_INTERFACE_COUNT			2
#else
#	define CDCEXT_DESC_LENGTH				0
#	define CDCEXT_INTERFACE_COUNT			0
#endif
#ifdef PROJ_CFG_CDCSHELL_SUPPORT
#	define CDCSHELL_DESC_LENGTH				66
#	define CDCSHELL_INTERFACE_COUNT			2
#else
#	define CDCSHELL_DESC_LENGTH				0
#	define CDCSHELL_INTERFACE_COUNT			0
#endif

#define USBD_CONFIGDESC_LENGTH		(9 + CMSIS_DAP_V2_DESC_LENGTH + WEBUSB_DESC_LENGTH + CDCEXT_DESC_LENGTH + CDCSHELL_DESC_LENGTH)
#define USBD_INTERFACE_COUNT		(CMSIS_DAP_V2_INTERFACE_COUNT + WEBUSB_INTERFACE_COUNT + CDCEXT_INTERFACE_COUNT + CDCSHELL_INTERFACE_COUNT)


#endif // __PROJ_CFG_H__

