#ifndef __VSFUSBD_WEBUSB_H_INCLUDED__
#define __VSFUSBD_WEBUSB_H_INCLUDED__

#ifdef DAP_BLOCK_TRANSFER
#include "../dap_block/DAP.h"
#else
#include "../dap/DAP.h"
#endif

struct vsfusbd_webusb_param_t
{
	bool dap_connected;
	struct dap_param_t *dap_param;

	uint8_t txbuff[DAP_CTRL_PACKET_SIZE];
	uint8_t rxbuff[DAP_CTRL_PACKET_SIZE];
};

extern const struct vsfusbd_class_protocol_t vsfusbd_webusb_class;

#endif	// __VSFUSBD_WEBUSB_H_INCLUDED__
