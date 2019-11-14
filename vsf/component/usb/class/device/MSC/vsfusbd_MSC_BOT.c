/***************************************************************************
 *   Copyright (C) 2009 - 2010 by Simon Qian <SimonQian@SimonQian.com>     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "vsf.h"

static void vsfusbd_MSCBOT_on_idle(void *p);
static void vsfusbd_MSCBOT_on_data_finish(void *p);

static vsf_err_t vsfusbd_MSCBOT_SendCSW(struct vsfusbd_device_t *device,
							struct vsfusbd_MSCBOT_param_t *param)
{
	struct vsfscsi_transact_t *scsi_transact = &param->scsi_dev->transact;
	struct USBMSC_CSW_t *CSW = &param->CSW;
	uint32_t remain_size;

	CSW->dCSWSignature = SYS_TO_LE_U32(USBMSC_CSW_SIGNATURE);
	CSW->dCSWTag = param->CBW.dCBWTag;
	remain_size = param->CBW.dCBWDataTransferLength;
	if (scsi_transact->lun != NULL)
	{
		remain_size -= scsi_transact->data_size;
		vsfscsi_release_transact(scsi_transact);
	}
	CSW->dCSWDataResidue = SYS_TO_LE_U32(remain_size);

	param->bufstream.mem.buffer.buffer = (uint8_t *)&param->CSW;
	param->bufstream.mem.buffer.size = sizeof(param->CSW);
	param->bufstream.mem.read = true;
	param->bufstream.stream.op = &bufstream_op;
	STREAM_INIT(&param->bufstream);

	param->transact.ep = param->ep_in;
	param->transact.data_size = sizeof(param->CSW);
	param->transact.stream = (struct vsf_stream_t *)&param->bufstream;
	param->transact.cb.on_finish = vsfusbd_MSCBOT_on_idle;
	param->transact.cb.param = param;
	return vsfusbd_ep_send(param->device, &param->transact);
}

static void vsfusbd_MSCBOT_to_SendCSW(void *p)
{
	struct vsfusbd_MSCBOT_param_t *param = (struct vsfusbd_MSCBOT_param_t *)p;
	vsfusbd_MSCBOT_SendCSW(param->device, param);
}

static vsf_err_t vsfusbd_MSCBOT_ErrHandler(struct vsfusbd_device_t *device,
			struct vsfusbd_MSCBOT_param_t *param,  uint8_t error)
{
	struct vsfscsi_transact_t *scsi_transact = &param->scsi_dev->transact;
	param->CSW.dCSWStatus = error;

	// OUT:	NACK(don't enable_OUT, if OUT is enabled, it will be disabled after data is sent)
	// IN:	if (dCBWDataTransferLength > 0)
	// 		  send_ZLP
	// 		send_CSW
	if (((param->CBW.bmCBWFlags & USBMSC_CBWFLAGS_DIR_MASK) ==
				USBMSC_CBWFLAGS_DIR_IN) &&
		(param->CBW.dCBWDataTransferLength > 0))
	{
		if (scsi_transact->lun != NULL)
		{
			scsi_transact->data_size = 0;
		}

		param->transact.ep = param->ep_in;
		param->transact.data_size = 0;
		param->transact.stream = NULL;
		param->transact.cb.on_finish = vsfusbd_MSCBOT_to_SendCSW;
		param->transact.cb.param = param;
		return vsfusbd_ep_send(param->device, &param->transact);
	}
	else
	{
		vsfusbd_MSCBOT_SendCSW(device, param);
	}

	return VSFERR_NONE;
}

static void vsfusbd_MSCBOT_dummy_inout(void *p)
{
	struct vsfusbd_MSCBOT_param_t *param = (struct vsfusbd_MSCBOT_param_t *)p;
	struct vsfusbd_transact_t *transact = &param->transact;
	struct vsf_buffer_t buffer =
	{
		.buffer = NULL,
		.size = stream_get_data_size(transact->stream),
	};

	stream_read(transact->stream, &buffer);
}

static void vsfusbd_MSCBOT_on_data_finish(void *p)
{
	struct vsfusbd_MSCBOT_param_t *param = (struct vsfusbd_MSCBOT_param_t *)p;
	struct USBMSC_CBW_t *CBW = &param->CBW;
	struct vsfscsi_transact_t *scsi_transact = &param->scsi_dev->transact;
	struct vsfusbd_transact_t *transact = &param->transact;

	if (scsi_transact->err)
	{
		param->CSW.dCSWStatus = USBMSC_CSW_FAIL;
	}
	if ((CBW->bmCBWFlags & USBMSC_CBWFLAGS_DIR_MASK) == USBMSC_CBWFLAGS_DIR_IN)
	{
		uint32_t remain = stream_get_data_size(transact->stream);
		if (remain)
		{
			transact->data_size = remain;
			transact->zlp =
				CBW->dCBWDataTransferLength > scsi_transact->data_size;
		send_remain:
			transact->stream = scsi_transact->lun->stream;
			transact->stream->callback_tx.on_connect = NULL;
			transact->stream->callback_tx.on_disconnect = NULL;
			transact->stream->callback_tx.on_inout = NULL;
			transact->cb.on_finish = vsfusbd_MSCBOT_to_SendCSW;
			vsfusbd_ep_send(param->device, transact);
			return;
		}
		else if ((CBW->dCBWDataTransferLength > scsi_transact->data_size) &&
			!((param->transact_size - transact->data_size) &
				(vsfhal_usbd_ep_get_IN_epsize(param->ep_in) - 1)))
		{
			transact->data_size = 0;
			goto send_remain;
		}
	}
	else
	{
		uint32_t remain = transact->data_size;
		if (remain)
		{
			transact->data_size = remain;
			transact->stream->callback_rx.param = param;
			transact->stream->callback_rx.on_connect = NULL;
			transact->stream->callback_rx.on_disconnect = NULL;
			transact->stream->callback_rx.on_inout = vsfusbd_MSCBOT_dummy_inout;
			transact->cb.on_finish = vsfusbd_MSCBOT_to_SendCSW;
			vsfusbd_ep_recv(param->device, transact);
			return;
		}
	}
	vsfusbd_MSCBOT_to_SendCSW(param);
}

static void vsfusbd_MSCBOT_on_cbw(void *p)
{
	struct vsfusbd_MSCBOT_param_t *param = (struct vsfusbd_MSCBOT_param_t *)p;
	struct USBMSC_CBW_t *CBW = &param->CBW;
	struct vsfscsi_transact_t *scsi_transact = &param->scsi_dev->transact;
	struct vsfusbd_device_t *device = param->device;
	struct vsfscsi_lun_t *lun;

	if (param->transact.data_size ||
		(CBW->dCBWSignature != USBMSC_CBW_SIGNATURE) ||
		(CBW->bCBWCBLength < 1) || (CBW->bCBWCBLength > 16))
	{
		vsfusbd_MSCBOT_on_idle(param);
		return;
	}

	if (CBW->bCBWLUN > param->scsi_dev->max_lun)
	{
	reply_failure:
		vsfusbd_MSCBOT_ErrHandler(device, param, USBMSC_CSW_FAIL);
		return;
	}

	param->CSW.dCSWStatus = USBMSC_CSW_OK;
	lun = &param->scsi_dev->lun[CBW->bCBWLUN];
	if (vsfscsi_execute(lun, CBW->CBWCB, CBW->bCBWCBLength,
			((CBW->bmCBWFlags & 0x80) << 24) | CBW->dCBWDataTransferLength))
	{
		goto reply_failure;
	}

	if (CBW->dCBWDataTransferLength)
	{
		struct vsfusbd_transact_t *transact = &param->transact;

		if (!scsi_transact->lun)
		{
			goto reply_failure;
		}
		param->transact_size = transact->data_size = scsi_transact->data_size ?
				scsi_transact->data_size : CBW->dCBWDataTransferLength;
		transact->stream = scsi_transact->lun->stream;
		transact->cb.on_finish = vsfusbd_MSCBOT_on_data_finish;
		transact->cb.param = param;

		if ((CBW->bmCBWFlags & USBMSC_CBWFLAGS_DIR_MASK) ==
					USBMSC_CBWFLAGS_DIR_IN)
		{
			transact->ep = param->ep_in;
			vsfusbd_ep_send(param->device, transact);
		}
		else
		{
			transact->ep = param->ep_out;
			vsfusbd_ep_recv(param->device, transact);
		}
	}
	else
	{
		if (scsi_transact->lun)
		{
			vsfscsi_cancel_transact(scsi_transact);
		}
		vsfusbd_MSCBOT_SendCSW(device, param);
	}
}

static void vsfusbd_MSCBOT_on_idle(void *p)
{
	struct vsfusbd_MSCBOT_param_t *param = (struct vsfusbd_MSCBOT_param_t *)p;

	param->bufstream.mem.buffer.buffer = (uint8_t *)&param->CBW;
	param->bufstream.mem.buffer.size = sizeof(param->CBW);
	param->bufstream.mem.read = false;
	param->bufstream.stream.op = &bufstream_op;
	STREAM_INIT(&param->bufstream);

	param->transact.ep = param->ep_out;
	param->transact.data_size = sizeof(param->CBW);
	param->transact.stream = (struct vsf_stream_t *)&param->bufstream;
	param->transact.cb.on_finish = vsfusbd_MSCBOT_on_cbw;
	param->transact.cb.param = param;
	vsfusbd_ep_recv(param->device, &param->transact);
}

static vsf_err_t vsfusbd_MSCBOT_class_init(uint8_t iface,
											struct vsfusbd_device_t *device)
{
	struct vsfusbd_config_t *config = &device->config[device->configuration];
	struct vsfusbd_MSCBOT_param_t *param =
		(struct vsfusbd_MSCBOT_param_t *)config->iface[iface].protocol_param;

	param->device = device;
	vsfusbd_MSCBOT_on_idle(param);
	return VSFERR_NONE;
}

static vsf_err_t vsfusbd_MSCBOT_request_prepare(struct vsfusbd_device_t *device)
{
	struct vsfusbd_ctrl_handler_t *ctrl_handler = &device->ctrl_handler;
	struct vsf_buffer_t *buffer = &ctrl_handler->bufstream.mem.buffer;
	struct usb_ctrlrequest_t *request = &ctrl_handler->request;
	uint8_t iface = request->wIndex;
	struct vsfusbd_config_t *config = &device->config[device->configuration];
	struct vsfusbd_MSCBOT_param_t *param =
		(struct vsfusbd_MSCBOT_param_t *)config->iface[iface].protocol_param;

	switch (request->bRequest)
	{
	case USB_MSCBOTREQ_GET_MAX_LUN:
		if ((request->wLength != 1) || (request->wValue != 0))
		{
			return VSFERR_FAIL;
		}
		buffer->buffer = &param->scsi_dev->max_lun;
		buffer->size = 1;
		break;
	case USB_MSCBOTREQ_RESET:
		if ((request->wLength != 0) || (request->wValue != 0) ||
			vsfhal_usbd_ep_reset_IN_toggle(param->ep_in) ||
			vsfhal_usbd_ep_reset_OUT_toggle(param->ep_out) ||
			vsfusbd_MSCBOT_class_init(iface, device))
		{
			return VSFERR_FAIL;
		}
		break;
	default:
		return VSFERR_FAIL;
	}
	ctrl_handler->data_size = buffer->size;
	return VSFERR_NONE;
}

const struct vsfusbd_class_protocol_t vsfusbd_MSCBOT_class =
{
	.request_prepare = vsfusbd_MSCBOT_request_prepare,
	.init = vsfusbd_MSCBOT_class_init,
};

