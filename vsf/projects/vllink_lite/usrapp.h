#ifndef __USRAPP_H__
#define __USRAPP_H__

#include "protocol/cmsis_dap/vsfusbd_CMSIS_DAP.h"

struct usrapp_t
{
	struct usrapp_usbd_t
	{
		struct vsfusbd_device_t device;
		struct vsfusbd_config_t config[1];
		struct vsfusbd_iface_t ifaces[USBD_INTERFACE_COUNT];

#ifdef PROJC_CFG_CMSIS_DAP_V1_SUPPORT
		struct vsfusbd_CMSIS_DAP_param_t cmsis_dap;
#endif

#ifdef PROJC_CFG_CMSIS_DAP_V2_SUPPORT
		// TODO
		//struct vsfusbd_CMSIS_DAP_param_t cmsis_dap;
#endif

#ifdef PROJC_CFG_CDCEXT_SUPPORT
		struct vsfusbd_CDCACM_param_t cdcacm_ext;		// -> usart ext
#endif

#ifdef PROJC_CFG_CDCSHELL_SUPPORT
		struct vsfusbd_CDCACM_param_t cdcacm_shell;		// -> shell
#endif
	} usbd;

	struct dap_param_t dap_param;

	struct
	{
		struct usart_stream_info_t usart_stream;
		struct vsf_fifostream_t stream_tx;
		struct vsf_fifostream_t stream_rx;
		uint8_t txbuff[128 + 4];
		uint8_t rxbuff[128 + 4];
	} usart_ext;

	struct
	{
		struct usart_stream_info_t usart_stream;
		struct vsf_fifostream_t stream_tx;
		struct vsf_fifostream_t stream_rx;
		uint8_t txbuff[128 + 4];
		uint8_t rxbuff[128 + 4];
	} usart_trst_swo;
};

extern struct usrapp_t usrapp;

void usrapp_reset(uint32_t delay_ms);

void usrapp_initial_init(struct usrapp_t *app);
bool usrapp_cansleep(struct usrapp_t *app);
void usrapp_srt_init(struct usrapp_t *app);
void usrapp_srt_poll(struct usrapp_t *app);
void usrapp_nrt_init(struct usrapp_t *app);
void usrapp_nrt_poll(struct usrapp_t *app);

#endif // __USRAPP_H__

