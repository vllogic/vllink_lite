#ifndef __VSFUSBD_CMSIS_DAP_H_INCLUDED__
#define __VSFUSBD_CMSIS_DAP_H_INCLUDED__

#ifdef DAP_BLOCK_TRANSFER
#include "../dap_block/DAP.h"
#else
#include "../dap/DAP.h"
#endif

struct vsfusbd_CMSIS_DAP_param_t
{
	struct vsfusbd_HID_param_t HID_param;
	struct vsfusbd_HID_report_t reports[3];
	
	struct dap_param_t *dap_param;
	bool dap_connected;
	
	uint8_t feature[1];
	uint8_t in[DAP_HID_PACKET_SIZE];
	uint8_t out[DAP_HID_PACKET_SIZE];
};

#if DAP_HID_PACKET_SIZE > 255
extern const uint8_t vsfusbd_CMSIS_DAP_HIDReportDesc[35];
#else
extern const uint8_t vsfusbd_CMSIS_DAP_HIDReportDesc[33];
#endif
vsf_err_t vsfusbd_CMSIS_DAP_init(struct vsfusbd_CMSIS_DAP_param_t *param);

#endif	// __VSFUSBD_CMSIS_DAP_H_INCLUDED__
