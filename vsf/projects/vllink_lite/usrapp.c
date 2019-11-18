#include "vsf.h"
#include "usrapp.h"

#include "usbd_config.c"

uint16_t usrapp_get_serial(uint8_t *serial);
vsf_err_t usrapp_update_swo_usart_param(uint8_t *mode, uint32_t *baudrate);
vsf_err_t usrapp_update_ext_usart_param(struct usb_CDCACM_line_coding_t *line_coding);

struct usrapp_t usrapp =
{
	.usbd.device.num_of_configuration		= dimof(usrapp.usbd.config),
	.usbd.device.config						= usrapp.usbd.config,
	.usbd.device.desc_filter				= (struct vsfusbd_desc_filter_t *)USB_descriptors,
	.usbd.device.device_class_iface			= 0,
	.usbd.config[0].num_of_ifaces			= dimof(usrapp.usbd.ifaces),
	.usbd.config[0].iface					= usrapp.usbd.ifaces,
#ifdef PROJC_CFG_CMSIS_DAP_V2_SUPPORT
	.usbd.ifaces[0].class_protocol			= (struct vsfusbd_class_protocol_t *)&vsfusbd_cmsis_dap_v2_class,
	.usbd.ifaces[0].protocol_param			= &usrapp.usbd.cmsis_dap_v2,
#endif
#ifdef PROJC_CFG_CMSIS_DAP_V1_SUPPORT
	.usbd.ifaces[1].class_protocol			= (struct vsfusbd_class_protocol_t *)&vsfusbd_HID_class,
	.usbd.ifaces[1].protocol_param			= &usrapp.usbd.cmsis_dap,
#endif
#ifdef PROJC_CFG_CDCEXT_SUPPORT
	.usbd.ifaces[2].class_protocol			= (struct vsfusbd_class_protocol_t *)&vsfusbd_CDCACMData_class,
	.usbd.ifaces[2].protocol_param			= &usrapp.usbd.cdcacm_ext,
	.usbd.ifaces[3].class_protocol			= (struct vsfusbd_class_protocol_t *)&vsfusbd_CDCACMControl_class,
	.usbd.ifaces[3].protocol_param			= &usrapp.usbd.cdcacm_ext,
#endif
#ifdef PROJC_CFG_CDCSHELL_SUPPORT
	.usbd.ifaces[4].class_protocol			= (struct vsfusbd_class_protocol_t *)&vsfusbd_CDCACMData_class,
	.usbd.ifaces[4].protocol_param			= &usrapp.usbd.cdcacm_shell,
	.usbd.ifaces[5].class_protocol			= (struct vsfusbd_class_protocol_t *)&vsfusbd_CDCACMControl_class,
	.usbd.ifaces[5].protocol_param			= &usrapp.usbd.cdcacm_shell,
#endif

	.usbd.cmsis_dap_v2.ep_in						= 1,
	.usbd.cmsis_dap_v2.ep_out						= 1,
	.usbd.cmsis_dap_v2.dap_param					= &usrapp.dap_param,
	.usbd.cmsis_dap_v2.dap_connected				= false,

	.usbd.cmsis_dap.HID_param.ep_in					= 2,
	.usbd.cmsis_dap.HID_param.ep_out				= 2,
	.usbd.cmsis_dap.dap_param						= &usrapp.dap_param,
	.usbd.cmsis_dap.dap_connected					= false,

#ifdef PROJC_CFG_CDCEXT_SUPPORT
	.usbd.cdcacm_ext.CDC.ep_in						= 3,
	.usbd.cdcacm_ext.CDC.ep_out						= 3,
	.usbd.cdcacm_ext.CDC.ep_notify					= 5,
	.usbd.cdcacm_ext.CDC.stream_tx					= (struct vsf_stream_t *)&usrapp.usart_ext.stream_rx,
	.usbd.cdcacm_ext.CDC.stream_rx					= (struct vsf_stream_t *)&usrapp.usart_ext.stream_tx,
	.usbd.cdcacm_ext.callback.set_line_coding		= usrapp_update_ext_usart_param,
	.usbd.cdcacm_ext.line_coding.bitrate			= 115200,
	.usbd.cdcacm_ext.line_coding.stopbittype		= 0,
	.usbd.cdcacm_ext.line_coding.paritytype			= 0,
	.usbd.cdcacm_ext.line_coding.datatype			= 8,
#endif
	
#ifdef PROJC_CFG_CDCSHELL_SUPPORT
	.usbd.cdcacm_shell.CDC.ep_in					= 4,
	.usbd.cdcacm_shell.CDC.ep_out					= 4,
	.usbd.cdcacm_shell.CDC.ep_notify				= 6,
	.usbd.cdcacm_shell.CDC.stream_tx				= (struct vsf_stream_t *)&usrapp.usart_trst_swo.stream_rx,
	.usbd.cdcacm_shell.CDC.stream_rx				= (struct vsf_stream_t *)&usrapp.usart_trst_swo.stream_tx,
	.usbd.cdcacm_shell.callback.set_line_coding		= usrapp_update_ext_usart_param,
	.usbd.cdcacm_shell.line_coding.bitrate			= 115200,
	.usbd.cdcacm_shell.line_coding.stopbittype		= 0,
	.usbd.cdcacm_shell.line_coding.paritytype		= 0,
	.usbd.cdcacm_shell.line_coding.datatype			= 8,
#endif

	.usart_ext.stream_tx.stream.op					= &fifostream_op,
	.usart_ext.stream_tx.mem.buffer.buffer			= (uint8_t *)&usrapp.usart_ext.txbuff,
	.usart_ext.stream_tx.mem.buffer.size			= sizeof(usrapp.usart_ext.txbuff),
	.usart_ext.stream_rx.stream.op					= &fifostream_op, 
	.usart_ext.stream_rx.mem.buffer.buffer			= (uint8_t *)&usrapp.usart_ext.rxbuff,
	.usart_ext.stream_rx.mem.buffer.size			= sizeof(usrapp.usart_ext.rxbuff),

	.usart_trst_swo.stream_tx.stream.op				= &fifostream_op,
	.usart_trst_swo.stream_tx.mem.buffer.buffer		= (uint8_t *)&usrapp.usart_trst_swo.txbuff,
	.usart_trst_swo.stream_tx.mem.buffer.size		= sizeof(usrapp.usart_trst_swo.txbuff),
	.usart_trst_swo.stream_rx.stream.op				= &fifostream_op, 
	.usart_trst_swo.stream_rx.mem.buffer.buffer		= (uint8_t *)&usrapp.usart_trst_swo.rxbuff,
	.usart_trst_swo.stream_rx.mem.buffer.size		= sizeof(usrapp.usart_trst_swo.rxbuff),
	
	.dap_param.get_serial							= usrapp_get_serial,
	.dap_param.update_swo_usart_param				= usrapp_update_swo_usart_param,

	.usart_ext.usart_stream.index					= PERIPHERAL_UART_EXT_INDEX,
	.usart_ext.usart_stream.mode					= PERIPHERAL_UART_MODE_DEFAULT,
	.usart_ext.usart_stream.int_priority			= PERIPHERAL_UART_PRIORITY,
	.usart_ext.usart_stream.baudrate				= PERIPHERAL_UART_BAUD_DEFAULT,
	.usart_ext.usart_stream.stream_tx				= &usrapp.usart_ext.stream_tx.stream,
	.usart_ext.usart_stream.stream_rx				= &usrapp.usart_ext.stream_rx.stream,

	.usart_trst_swo.usart_stream.index				= PERIPHERAL_UART_SWO_INDEX,
	.usart_trst_swo.usart_stream.mode				= PERIPHERAL_UART_MODE_DEFAULT,
	.usart_trst_swo.usart_stream.int_priority		= PERIPHERAL_UART_PRIORITY,
	.usart_trst_swo.usart_stream.baudrate			= PERIPHERAL_UART_BAUD_DEFAULT,
	.usart_trst_swo.usart_stream.stream_tx			= &usrapp.usart_trst_swo.stream_tx.stream,
	.usart_trst_swo.usart_stream.stream_rx			= &usrapp.usart_trst_swo.stream_rx.stream,
};


uint16_t usrapp_get_serial(uint8_t *serial)
{
	uint16_t i;
	for (i = 0; i < (sizeof(USB_StringSerial) - 2 / 2); i++)
		serial[i] = USB_StringSerial[i * 2 + 2];
	return i;
}

vsf_err_t usrapp_update_swo_usart_param(uint8_t *mode, uint32_t *baudrate)
{
	if (mode)
		usrapp.usart_trst_swo.usart_stream.mode = *mode;
	if (baudrate)
		usrapp.usart_trst_swo.usart_stream.baudrate = *baudrate;
	return usart_stream_init(&usrapp.usart_trst_swo.usart_stream);
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
	vsfusbd_device_fini(&usrapp.usbd.device);
	vsftimer_create_cb(delay_ms, 1, usrapp_reset_do, NULL);
}

void usrapp_initial_init(struct usrapp_t *app)
{
	
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
	
	
	vsfusbd_CMSIS_DAP_init(&app->usbd.cmsis_dap);
	usart_stream_init(&usrapp.usart_ext.usart_stream);	
	usart_stream_init(&usrapp.usart_trst_swo.usart_stream);

	vsfusbd_device_init(&app->usbd.device);
	vsfhal_usbd_disconnect();
	vsftimer_create_cb(200, 1, usrapp_usbd_conn, app);
}

void usrapp_nrt_init(struct usrapp_t *app)
{
	PERIPHERAL_LED_RED_INIT();
	PERIPHERAL_LED_GREEN_INIT();
	PERIPHERAL_LED_RED_OFF();
	PERIPHERAL_LED_GREEN_OFF();

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
