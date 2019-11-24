#include "vsf.h"
#include "usrapp.h"

#include "usbd_config.c"

uint16_t usrapp_get_serial(uint8_t *serial);
vsf_err_t usrapp_update_swo_usart_param(uint8_t *mode, uint32_t *baudrate);
vsf_err_t usrapp_update_ext_usart_param(struct usb_CDCACM_line_coding_t *line_coding);

static vsf_err_t usrapp_usbd_vendor_request_prepare(struct vsfusbd_device_t *device);
static uint16_t usrapp_vendor_handler(uint8_t cmd, uint8_t *req, uint8_t *resp, uint16_t req_data_size, uint16_t resp_free_size);

struct usrapp_t usrapp =
{
	.usbd.device.num_of_configuration		= dimof(usrapp.usbd.config),
	.usbd.device.config						= usrapp.usbd.config,
	.usbd.device.desc_filter				= (struct vsfusbd_desc_filter_t *)USB_descriptors,
	.usbd.device.device_class_iface			= 0,
	.usbd.config[0].num_of_ifaces			= dimof(usrapp.usbd.ifaces),
	.usbd.config[0].iface					= usrapp.usbd.ifaces,
	.usbd.config[0].vendor_prepare			= usrapp_usbd_vendor_request_prepare,

#ifdef PROJ_CFG_WEBUSB_SUPPORT
	.usbd.ifaces[0].class_protocol			= (struct vsfusbd_class_protocol_t *)&vsfusbd_webusb_class,
	.usbd.ifaces[0].protocol_param			= &usrapp.usbd.webusb,
#endif
#ifdef PROJ_CFG_CMSIS_DAP_V2_SUPPORT
	.usbd.ifaces[1].class_protocol			= (struct vsfusbd_class_protocol_t *)&vsfusbd_cmsis_dap_v2_class,
	.usbd.ifaces[1].protocol_param			= &usrapp.usbd.cmsis_dap_v2,
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
#if PROJ_CFG_DAP_VERDOR_BOOTLOADER_ENABLE
	.dap_param.vendor_handler						= usrapp_vendor_handler,
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
};


static vsf_err_t usrapp_usbd_vendor_request_prepare(struct vsfusbd_device_t *device)
{
	struct vsfusbd_ctrl_handler_t *ctrl_handler = &device->ctrl_handler;
	struct usb_ctrlrequest_t *request = &ctrl_handler->request;
	struct vsf_buffer_t *buffer = &ctrl_handler->bufstream.mem.buffer;
	
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
	else if (request->bRequest == 0x21)	// USBD_WEBUSB_VENDOR_CODE
	{
		// TODO
		__ASM("NOP");
	}
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
	
#if 0	// TODO baud table
	if (bitrate >= 8000000)
		bitrate = 8000000;		// 64M div 8
	else if (bitrate > 4000000)
		bitrate = 4000000;		// 64M div 16
	else if (bitrate >= 3200000)
		bitrate = 3200000;		// 64M div 20/20+ 
	else if (bitrate < 2000)
		bitrate = 2000;			// 64M div 32000/32000-
#endif

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
	vsftimer_create_cb(delay_ms, 1, usrapp_reset_do, NULL);
}

void usrapp_initial_init(struct usrapp_t *app)
{
#if PROJ_CFG_DAP_VERDOR_BOOTLOADER_ENABLE
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

#if PROJ_CFG_DAP_VERDOR_BOOTLOADER_ENABLE
static void flash_erase_write(uint8_t *buf, uint32_t addr, uint32_t size)
{
	// write op == 4
	uint32_t i;
	uint32_t erase_op = vsfhal_flash_blocksize(0, addr, 0, 0);
	
	for (i = 0; i < size; i += 4)
	{
		if (!(addr % erase_op))
			vsfhal_flash_erase(0, addr);
		vsfhal_flash_write(0, addr, buf + i);
	}
}

static void flash_read(uint8_t *buf, uint32_t addr, uint32_t size)
{
	memcpy(buf, (void *)addr, size);
}

static uint16_t usrapp_vendor_handler(uint8_t cmd, uint8_t *req, uint8_t *resp, uint16_t req_data_size, uint16_t resp_free_size)
{
	uint16_t ret = 0;
	switch (cmd)
	{
	case ID_DAP_Vendor30:
		/*
			APP Firmware Write CMD:

			Request chip reset:
			CMD [1 byte]		Size [2 byte]		cmd [3 byte]		
			ID_DAP_Vendor30		0x0					0	
			Response:
			CMD [1 byte]		Size [2 byte]		cmd [3 byte]		
			ID_DAP_Vendor30		0x0					0

			Request firmware rite:
			CMD [1 byte]		Size [2 byte]		Addr [3 byte]		DATA [{Size} byte]
			ID_DAP_Vendor30		[0x1, 0xffff]		[0x0, 0xffffff]			.. .. ..
			Response:
			CMD [1 byte]		Size [2 byte]		Addr [3 byte]		
			ID_DAP_Vendor30		[0x1, 0xffff]		[0x0, 0xffffff]		
		*/
		if (resp_free_size >= 5)
		{
			uint16_t size = GET_LE_U16(req);
			if (size == 0)	// subcmd mode
			{
				uint32_t subcmd = GET_LE_U24(req + 2);
				if (subcmd == 0)	// reset
				{
					uint32_t tick = GET_LE_U32(req + 5);
					if (!tick)
						tick = 100;
					usrapp_reset(tick);
				}
				else
					goto error;
			}
			else	// write mode
			{
				uint32_t addr = GET_LE_U24(req + 2);
				uint32_t write_op = vsfhal_flash_blocksize(0, FIRMWARE_AREA_ADDR, 0, 2);
				if (((addr + size) < FIRMWARE_AREA_SIZE_MAX) && !(addr % write_op) && !(size % write_op))
					flash_erase_write(req + 5, FIRMWARE_AREA_ADDR + addr, size);
				else
					goto error;
			}
			memcpy(resp, req, 5);
			ret += 5;
		}
		break;
	case ID_DAP_Vendor31:
		/*
			APP Firmware Read CMD:

			Request:
			CMD [1 byte]		Size [2 byte]		cmd [3 byte]		
			ID_DAP_Vendor31		0x0					0x0
			Response:
			CMD [1 byte]		Size [2 byte]		cmd [3 byte]		app_max_size[4 byte]		erase_block_size[4 byte]		write_block_size[4 byte]
			ID_DAP_Vendor31		0x0					0x0					[0x0, 0xffff]				[0x0, 0xffff]					[0x0, 0xffff]

			Request:
			CMD [1 byte]		Size [2 byte]		Addr [3 byte]		
			ID_DAP_Vendor31		[0x1, 0xffffff]		[0x0, 0xffff]
			Response:
			CMD [1 byte]		Size [2 byte]		Addr [3 byte]		DATA [{Size} byte]
			ID_DAP_Vendor31		[0x1, 0xffffff]		[0x0, 0xffff]			.. .. ..
		*/
		if (resp_free_size >= 5)
		{
			uint16_t size = GET_LE_U16(req);
			if (size == 0)	// subcmd mode
			{
				uint32_t subcmd = GET_LE_U24(req + 2);
				if (subcmd == 0)	// get flash op info
				{
					uint32_t app_size = FIRMWARE_AREA_SIZE_MAX;
					uint32_t erase_op = vsfhal_flash_blocksize(0, FIRMWARE_AREA_ADDR, 0, 0);
					uint32_t write_op = vsfhal_flash_blocksize(0, FIRMWARE_AREA_ADDR, 0, 2);
					SET_LE_U32(resp + 5, app_size);
					SET_LE_U32(resp + 9, erase_op);
					SET_LE_U32(resp + 13, write_op);
					ret += 12;
				}
				else
					goto error;
			}
			else	// read mode
			{
				uint32_t addr = GET_LE_U24(req + 2);
				if (((addr + size) < FIRMWARE_AREA_SIZE_MAX) && ((size + 5) <= resp_free_size))
				{
					flash_read(resp + 5, FIRMWARE_AREA_ADDR + addr, size);
					ret += size;
				}
				else
					goto error;
			}
			memcpy(resp, req, 5);
			ret += 5;
		}
		break;
	}
	return ret;
error:
	return 0;
}
#endif

void usrapp_nrt_init(struct usrapp_t *app)
{
	PERIPHERAL_LED_RED_INIT();
	PERIPHERAL_LED_GREEN_INIT();
#if PROJ_CFG_DAP_VERDOR_BOOTLOADER_ENABLE
	PERIPHERAL_LED_RED_ON();
	PERIPHERAL_LED_GREEN_ON();
	
	vsfhal_flash_init(0);
#else
	PERIPHERAL_LED_RED_OFF();
	PERIPHERAL_LED_GREEN_OFF();
#endif

	DAP_init(&app->dap_param);

#if 0
	extern void DAP_test(struct dap_param_t *param);
	DAP_test(&app->dap_param);
#endif
}

int main()
{
	return vsfmain();
}
