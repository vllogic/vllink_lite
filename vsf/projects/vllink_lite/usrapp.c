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

#include "vsf.h"
#include "usrapp.h"

#ifndef PROJ_CFG_BOOTLOADER_DFU_MODE
#	include "usbd_config.c"
#else
#	include "usbd_config_bootloader.c"
#endif

uint16_t usrapp_get_serial(uint8_t *serial);
vsf_err_t usrapp_update_swo_usart_param(uint8_t *mode, uint32_t *baudrate);
vsf_err_t usrapp_update_ext_usart_param(struct usb_CDCACM_line_coding_t *line_coding);

static vsf_err_t usrapp_usbd_vendor_request_prepare(struct vsfusbd_device_t *device);

struct usrapp_t usrapp =
{
	.usbd.device.num_of_configuration		= dimof(usrapp.usbd.config),
	.usbd.device.config						= usrapp.usbd.config,
	.usbd.device.desc_filter				= (struct vsfusbd_desc_filter_t *)USB_descriptors,
	.usbd.device.device_class_iface			= 0,
	.usbd.config[0].num_of_ifaces			= dimof(usrapp.usbd.ifaces),
	.usbd.config[0].iface					= usrapp.usbd.ifaces,
	.usbd.config[0].vendor_prepare			= usrapp_usbd_vendor_request_prepare,

#ifdef PROJ_CFG_BOOTLOADER_DFU_MODE
	.usbd.ifaces[0].class_protocol			= (struct vsfusbd_class_protocol_t *)&vsfusbd_dfu_class,
	.usbd.ifaces[0].protocol_param			= &usrapp.usbd.dfu,
#else
#ifdef PROJ_CFG_CMSIS_DAP_V2_SUPPORT
	.usbd.ifaces[0].class_protocol			= (struct vsfusbd_class_protocol_t *)&vsfusbd_cmsis_dap_v2_class,
	.usbd.ifaces[0].protocol_param			= &usrapp.usbd.cmsis_dap_v2,
#endif
#ifdef PROJ_CFG_WEBUSB_SUPPORT
	.usbd.ifaces[1].class_protocol			= (struct vsfusbd_class_protocol_t *)&vsfusbd_webusb_class,
	.usbd.ifaces[1].protocol_param			= &usrapp.usbd.webusb,
#endif
#ifdef PROJ_CFG_CDCEXT_SUPPORT
	.usbd.ifaces[2].class_protocol			= (struct vsfusbd_class_protocol_t *)&vsfusbd_CDCACMData_class,
	.usbd.ifaces[2].protocol_param			= &usrapp.usbd.cdcacm_ext,
	.usbd.ifaces[3].class_protocol			= (struct vsfusbd_class_protocol_t *)&vsfusbd_CDCACMControl_class,
	.usbd.ifaces[3].protocol_param			= &usrapp.usbd.cdcacm_ext,
#endif
#ifdef PROJ_CFG_CDCSHELL_SUPPORT
	.usbd.ifaces[4].class_protocol			= (struct vsfusbd_class_protocol_t *)&vsfusbd_CDCACMData_class,
	.usbd.ifaces[4].protocol_param			= &usrapp.usbd.cdcacm_shell,
	.usbd.ifaces[5].class_protocol			= (struct vsfusbd_class_protocol_t *)&vsfusbd_CDCACMControl_class,
	.usbd.ifaces[5].protocol_param			= &usrapp.usbd.cdcacm_shell,
#endif

#ifdef PROJ_CFG_CMSIS_DAP_V2_SUPPORT
	.usbd.cmsis_dap_v2.ep_in						= 1,
	.usbd.cmsis_dap_v2.ep_out						= 1,
	.usbd.cmsis_dap_v2.dap_param					= &usrapp.dap_param,
	.usbd.cmsis_dap_v2.dap_connected				= false,
#endif

#ifdef PROJ_CFG_CDCEXT_SUPPORT
	.usbd.cdcacm_ext.CDC.ep_in						= 2,
	.usbd.cdcacm_ext.CDC.ep_out						= 2,
	.usbd.cdcacm_ext.CDC.ep_notify					= 4,
	.usbd.cdcacm_ext.CDC.stream_tx					= (struct vsf_stream_t *)&usrapp.usart_ext.stream_rx,
	.usbd.cdcacm_ext.CDC.stream_rx					= (struct vsf_stream_t *)&usrapp.usart_ext.stream_tx,
	.usbd.cdcacm_ext.callback.set_line_coding		= usrapp_update_ext_usart_param,
	.usbd.cdcacm_ext.line_coding.bitrate			= 115200,
	.usbd.cdcacm_ext.line_coding.stopbittype		= 0,
	.usbd.cdcacm_ext.line_coding.paritytype			= 0,
	.usbd.cdcacm_ext.line_coding.datatype			= 8,
#endif
	
#ifdef PROJ_CFG_CDCSHELL_SUPPORT
	.usbd.cdcacm_shell.CDC.ep_in					= 3,
	.usbd.cdcacm_shell.CDC.ep_out					= 3,
	.usbd.cdcacm_shell.CDC.ep_notify				= 5,
	.usbd.cdcacm_shell.CDC.stream_tx				= (struct vsf_stream_t *)&usrapp.usart_trst_swo.stream_rx,
	.usbd.cdcacm_shell.CDC.stream_rx				= (struct vsf_stream_t *)&usrapp.usart_trst_swo.stream_tx,
	.usbd.cdcacm_shell.callback.set_line_coding		= usrapp_update_ext_usart_param,
	.usbd.cdcacm_shell.line_coding.bitrate			= 115200,
	.usbd.cdcacm_shell.line_coding.stopbittype		= 0,
	.usbd.cdcacm_shell.line_coding.paritytype		= 0,
	.usbd.cdcacm_shell.line_coding.datatype			= 8,
#endif

#if PROJ_CFG_USART_EXT_ENABLE
	.usart_ext.stream_tx.stream.op					= &fifostream_op,
	.usart_ext.stream_tx.mem.buffer.buffer			= (uint8_t *)&usrapp.usart_ext.txbuff,
	.usart_ext.stream_tx.mem.buffer.size			= sizeof(usrapp.usart_ext.txbuff),
	.usart_ext.stream_rx.stream.op					= &fifostream_op, 
	.usart_ext.stream_rx.mem.buffer.buffer			= (uint8_t *)&usrapp.usart_ext.rxbuff,
	.usart_ext.stream_rx.mem.buffer.size			= sizeof(usrapp.usart_ext.rxbuff),
#endif

#if PROJ_CFG_USART_TRST_SWO_ENABLE
	.usart_trst_swo.stream_tx.stream.op				= &fifostream_op,
	.usart_trst_swo.stream_tx.mem.buffer.buffer		= (uint8_t *)&usrapp.usart_trst_swo.txbuff,
	.usart_trst_swo.stream_tx.mem.buffer.size		= sizeof(usrapp.usart_trst_swo.txbuff),
	.usart_trst_swo.stream_rx.stream.op				= &fifostream_op, 
	.usart_trst_swo.stream_rx.mem.buffer.buffer		= (uint8_t *)&usrapp.usart_trst_swo.rxbuff,
	.usart_trst_swo.stream_rx.mem.buffer.size		= sizeof(usrapp.usart_trst_swo.rxbuff),
#endif
	
	.dap_param.get_serial							= usrapp_get_serial,
#if PROJ_CFG_DAP_VERDOR_UART_ENABLE
	.dap_param.update_swo_usart_param				= usrapp_update_swo_usart_param,
#endif

#if PROJ_CFG_USART_EXT_ENABLE
	.usart_ext.usart_stream.index					= PERIPHERAL_UART_EXT_INDEX,
	.usart_ext.usart_stream.mode					= PERIPHERAL_UART_MODE_DEFAULT,
	.usart_ext.usart_stream.int_priority			= PERIPHERAL_UART_PRIORITY,
	.usart_ext.usart_stream.baudrate				= PERIPHERAL_UART_BAUD_DEFAULT,
	.usart_ext.usart_stream.stream_tx				= &usrapp.usart_ext.stream_tx.stream,
	.usart_ext.usart_stream.stream_rx				= &usrapp.usart_ext.stream_rx.stream,
#endif

#if PROJ_CFG_USART_TRST_SWO_ENABLE
	.usart_trst_swo.usart_stream.index				= PERIPHERAL_UART_SWO_INDEX,
	.usart_trst_swo.usart_stream.mode				= PERIPHERAL_UART_MODE_DEFAULT,
	.usart_trst_swo.usart_stream.int_priority		= PERIPHERAL_UART_PRIORITY,
	.usart_trst_swo.usart_stream.baudrate			= PERIPHERAL_UART_BAUD_DEFAULT,
	.usart_trst_swo.usart_stream.stream_tx			= &usrapp.usart_trst_swo.stream_tx.stream,
	.usart_trst_swo.usart_stream.stream_rx			= &usrapp.usart_trst_swo.stream_rx.stream,
#endif
#endif
};


static vsf_err_t usrapp_usbd_vendor_request_prepare(struct vsfusbd_device_t *device)
{
	struct vsfusbd_ctrl_handler_t *ctrl_handler = &device->ctrl_handler;
	struct usb_ctrlrequest_t *request = &ctrl_handler->request;
	struct vsf_buffer_t *buffer = &ctrl_handler->bufstream.mem.buffer;
	
#ifndef PROJ_CFG_BOOTLOADER_DFU_MODE
	if (request->bRequest == 0x20)	// USBD_WINUSB_VENDOR_CODE
	{
		if (request->wIndex == 0x07)	// WINUSB_REQUEST_GET_DESCRIPTOR_SET
		{
			buffer->buffer = (uint8_t *)WINUSB_Descriptor;
			buffer->size = sizeof(WINUSB_Descriptor);
			ctrl_handler->data_size = sizeof(WINUSB_Descriptor);
			return VSFERR_NONE;
		}
	}
#else
	if (request->bRequest == 0x21)
	{
		if (request->wIndex == 0x02)
		{
			buffer->buffer = (uint8_t *)webusb_url_descriptor;
			buffer->size = sizeof(webusb_url_descriptor);
			ctrl_handler->data_size = sizeof(webusb_url_descriptor);
			return VSFERR_NONE;
		}
		else if (request->wIndex == 0x04)
		{
			buffer->buffer = (uint8_t *)WINUSB_Descriptor;
			buffer->size = sizeof(WINUSB_Descriptor);
			ctrl_handler->data_size = sizeof(WINUSB_Descriptor);
			return VSFERR_NONE;
		}
	}
#endif
	return VSFERR_FAIL;
}

uint16_t usrapp_get_serial(uint8_t *serial)
{
	if (serial)
	{
		uint16_t i;
		for (i = 0; i < ((sizeof(USB_StringSerial) - 2) / 2); i++)
			serial[i] = USB_StringSerial[i * 2 + 2];
		return i;
	}
	else
		return (sizeof(USB_StringSerial) - 2) / 2;
}

vsf_err_t usrapp_update_swo_usart_param(uint8_t *mode, uint32_t *baudrate)
{
#if PROJ_CFG_USART_TRST_SWO_ENABLE
	if (mode)
		usrapp.usart_trst_swo.usart_stream.mode = *mode;
	if (baudrate)
		usrapp.usart_trst_swo.usart_stream.baudrate = *baudrate;
	return usart_stream_init(&usrapp.usart_trst_swo.usart_stream);
#else
	return VSFERR_NOT_SUPPORT;
#endif
}

vsf_err_t usrapp_update_ext_usart_param(struct usb_CDCACM_line_coding_t *line_coding)
{
/*
struct usb_CDCACM_line_coding_t
{
	uint32_t bitrate;
	uint8_t stopbittype;
	uint8_t paritytype;
	uint8_t datatype;
};
	bitrate:
		eg. 115200
	stopbittype:
		0 - 1bit
		1 - 1.5bit
		2 - 2bit
	paritytype:
		0 - None
		1 - Odd
		2 - Even
		3 - Mark
		4 - Space
	datatype:
		5, 6, 7, 8, 16
*/

#if PROJ_CFG_USART_TRST_SWO_ENABLE
	uint32_t mode;
	uint32_t bitrate = line_coding->bitrate;

	if (bitrate >= 8000000)
		bitrate = 8000000;		// 64M div 8
	else if (bitrate > 4000000)
		bitrate = 4000000;		// 64M div 16
	else if (bitrate >= 3200000)
		bitrate = 3200000;		// 64M div 20/20+ 
	else if (bitrate < 2000)
		bitrate = 2000;			// 64M div 32000/32000-

	if (line_coding->stopbittype == 1)
		mode = PERIPHERAL_UART_STOPBITS_1P5;
	else if (line_coding->stopbittype == 2)
		mode = PERIPHERAL_UART_STOPBITS_2;
	else
		mode = PERIPHERAL_UART_STOPBITS_1;
	
	if (line_coding->paritytype == 1)
		mode |= PERIPHERAL_UART_PARITY_ODD;
	else if (line_coding->paritytype == 2)
		mode |= PERIPHERAL_UART_PARITY_EVEN;
	else
		mode |= PERIPHERAL_UART_PARITY_NONE;

	// datatype
	mode |= PERIPHERAL_UART_BITLEN_8;
	
	usrapp.usart_ext.usart_stream.baudrate = bitrate;
	usrapp.usart_ext.usart_stream.mode = mode;
	return usart_stream_init(&usrapp.usart_ext.usart_stream);
#else
	return VSFERR_NOT_SUPPORT;
#endif
}

static void usrapp_usbd_conn(void *p)
{
	vsfhal_usbd_connect();
}

static void usrapp_reset_do(void *p)
{
	vsfhal_core_reset(NULL);
}

void usrapp_reset(uint32_t delay_ms)
{
	vsfhal_usbd_disconnect();
	vsftimer_create_cb(delay_ms, 1, usrapp_reset_do, NULL);
}

void usrapp_initial_init(struct usrapp_t *app)
{
#ifdef PROJ_CFG_BOOTLOADER_DFU_MODE
#	ifdef PERIPHERAL_KEY_PORT
	vsfhal_gpio_init(PERIPHERAL_KEY_PORT);
#	endif
#	if PERIPHERAL_KEY_VALID_LEVEL
	vsfhal_gpio_config(PERIPHERAL_KEY_PORT, PERIPHERAL_KEY_PIN, VSFHAL_GPIO_INPUT | VSFHAL_GPIO_PULLDOWN);
	if (vsfhal_gpio_get(PERIPHERAL_KEY_PORT, 1 << PERIPHERAL_KEY_PIN) == 0)
#	else
	vsfhal_gpio_config(PERIPHERAL_KEY_PORT, PERIPHERAL_KEY_PIN, VSFHAL_GPIO_INPUT | VSFHAL_GPIO_PULLUP);
	if (vsfhal_gpio_get(PERIPHERAL_KEY_PORT, 1 << PERIPHERAL_KEY_PIN))
#	endif
	{
		uint32_t app_main_addr = *(uint32_t *)(FIRMWARE_AREA_ADDR + 4);
		uint32_t sp_addr = *(uint32_t *)(FIRMWARE_AREA_ADDR);
		
		if ((app_main_addr >= FIRMWARE_AREA_ADDR) && (app_main_addr < (FIRMWARE_AREA_ADDR + FIRMWARE_AREA_SIZE_MAX)) && 
				(sp_addr >= FIRMWARE_SP_ADDR) && (sp_addr < (FIRMWARE_SP_ADDR + FIRMWARE_SP_SIZE_MAX)))
		{
			uint32_t (*app_main)(void) = (uint32_t(*)(void))app_main_addr;
			__set_MSP(sp_addr);
			app_main();
		}
	}
#endif
}

void usrapp_srt_init(struct usrapp_t *app)
{
	uint8_t i;
	uint8_t uid[12];
	uint8_t *serial = USB_StringSerial + 2;
	vsfhal_uid_get((uint8_t *)uid, 12);
	
	for (i = 23; i > 0; i--)
	{
		uint8_t v = uid[i / 2];
		if (i & 0x1)
			v >>= 4;
		else
			v = v & 0xf;
		if (v < 10)
			*serial = '0' + v;
		else
			*serial = 'A' - 10 + v;
		serial += 2;
	}
	
#if PROJ_CFG_USART_EXT_ENABLE
	usart_stream_init(&usrapp.usart_ext.usart_stream);
#endif
#if PROJ_CFG_USART_TRST_SWO_ENABLE
	usart_stream_init(&usrapp.usart_trst_swo.usart_stream);
#endif

	vsfusbd_device_init(&app->usbd.device);
	vsfhal_usbd_disconnect();
	vsftimer_create_cb(200, 1, usrapp_usbd_conn, app);
}

void usrapp_nrt_init(struct usrapp_t *app)
{
	PERIPHERAL_LED_RED_INIT();
	PERIPHERAL_LED_GREEN_INIT();
#ifdef PROJ_CFG_BOOTLOADER_DFU_MODE
	PERIPHERAL_LED_RED_ON();
	PERIPHERAL_LED_GREEN_ON();
#else
	PERIPHERAL_LED_RED_OFF();
	PERIPHERAL_LED_GREEN_OFF();

	DAP_init(&app->dap_param);

#if 0
	extern void DAP_test(struct dap_param_t *param);
	DAP_test(&app->dap_param);
#endif
#endif
}

int main()
{
	return vsfmain();
}
