#ifndef __PROJ_CFG_LVL2_H__
#define __PROJ_CFG_LVL2_H__

//#define PROJC_CFG_CDCSHELL_SUPPORT

/*
	DAP Config
*/
#define DAP_SWD						1
#define DAP_JTAG					1
#define DAP_JTAG_DEV_CNT			8
#define DAP_DEFAULT_PORT			1
#define DAP_DEFAULT_SWJ_CLOCK		4000000
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
#define DAP_PACKET_COUNT			2
#define SWO_UART					1
#define SWO_UART_MAX_BAUDRATE		3200000
#define SWO_UART_MIN_BAUDRATE		16000
#define SWO_MANCHESTER				0
#define SWO_BUFFER_SIZE				128

#define DAP_VENDOR					"Vllogic.com"
#define DAP_PRODUCT					"Vllogic Lite"

#endif // __PROJ_CFG_LVL2_H__
