#include "vsf.h"
#include "vsfusbd_webusb.h"

static vsf_err_t vsfusbd_webusb_request_prepare(struct vsfusbd_device_t *device)
{
	// TODO
	return VSFERR_NONE;
}

static vsf_err_t vsfusbd_webusb_request_process(struct vsfusbd_device_t *device)
{
	// TODO
	return VSFERR_NONE;
}

const struct vsfusbd_class_protocol_t vsfusbd_webusb_class =
{
	.request_prepare =	vsfusbd_webusb_request_prepare,
	.request_process =	vsfusbd_webusb_request_process,
};

