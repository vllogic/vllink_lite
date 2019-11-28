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
#include "vsfusbd_dfu.h"
#include "usrapp.h"

#define STATE_APP_IDLE                  0x00
#define STATE_APP_DETACH                0x01
#define STATE_DFU_IDLE                  0x02
#define STATE_DFU_DOWNLOAD_SYNC         0x03
#define STATE_DFU_DOWNLOAD_BUSY         0x04
#define STATE_DFU_DOWNLOAD_IDLE         0x05
#define STATE_DFU_MANIFEST_SYNC         0x06
#define STATE_DFU_MANIFEST              0x07
#define STATE_DFU_MANIFEST_WAIT_RESET   0x08
#define STATE_DFU_UPLOAD_IDLE           0x09
#define STATE_DFU_ERROR                 0x0a

#define DFU_STATUS_OK                   0x00
#define DFU_STATUS_ERROR_TARGET         0x01
#define DFU_STATUS_ERROR_FILE           0x02
#define DFU_STATUS_ERROR_WRITE          0x03
#define DFU_STATUS_ERROR_ERASE          0x04
#define DFU_STATUS_ERROR_CHECK_ERASED   0x05
#define DFU_STATUS_ERROR_PROG           0x06
#define DFU_STATUS_ERROR_VERIFY         0x07
#define DFU_STATUS_ERROR_ADDRESS        0x08
#define DFU_STATUS_ERROR_NOTDONE        0x09
#define DFU_STATUS_ERROR_FIRMWARE       0x0a
#define DFU_STATUS_ERROR_VENDOR         0x0b
#define DFU_STATUS_ERROR_USBR           0x0c
#define DFU_STATUS_ERROR_POR            0x0d
#define DFU_STATUS_ERROR_UNKNOWN        0x0e
#define DFU_STATUS_ERROR_STALLEDPKT     0x0f

static void flash_erase_write(uint8_t *buf, uint32_t addr, uint32_t size)
{
	// write op == 4
	uint32_t start, end = addr + size, erase_op = vsfhal_flash_blocksize(0, addr, 0, 0);
	
	for (start = addr; start < end; start += 4)
	{
		if (!(start % erase_op))
			vsfhal_flash_erase(0, start);
		vsfhal_flash_write(0, start, buf);
		buf += 4;
	}
}

static void flash_read(uint8_t *buf, uint32_t addr, uint32_t size)
{
	memcpy(buf, (void *)addr, size);
}

static vsf_err_t vsfusbd_dfu_request_prepare(struct vsfusbd_device_t *device)
{
	struct vsfusbd_ctrl_handler_t *ctrl_handler = &device->ctrl_handler;
	struct vsf_buffer_t *buffer = &ctrl_handler->bufstream.mem.buffer;
	struct usb_ctrlrequest_t *request = &ctrl_handler->request;
	struct vsfusbd_config_t *config = &device->config[device->configuration];
	struct vsfusbd_dfu_param_t *param = config->iface[request->wIndex].protocol_param;

	memset(param->reply, 0, sizeof(param->reply));
	
	switch (request->bRequest)
	{
	case 1:		// DFU_DNLOAD
		param->state = STATE_DFU_DOWNLOAD_BUSY;
		param->op_addr = request->wValue * DFU_BLOCK_SIZE;
		param->op_size = min(request->wLength, DFU_BLOCK_SIZE);
		buffer->size = DFU_BLOCK_SIZE;
		buffer->buffer = param->block;
		break;
	case 2:		// DFU_UPLOAD
		param->op_addr = request->wValue * DFU_BLOCK_SIZE;
		param->op_size = min(request->wLength, DFU_BLOCK_SIZE);
		if (param->op_addr >= FIRMWARE_AREA_SIZE_MAX)
		{
#if 0	// TODO if set size 0, usb will lost status transaction.
			buffer->size = 0;
#else
			param->block[0] = 0xff;
			buffer->size = 1;
#endif
		}
		else
		{
			uint32_t size = min(FIRMWARE_AREA_SIZE_MAX - param->op_addr, param->op_size);
			flash_read(param->block, FIRMWARE_AREA_ADDR + param->op_addr, size);
			buffer->size = size;
		}
		buffer->buffer = param->block;
		break;
	case 3:		// DFU_GETSTATUS
		param->reply[0] = DFU_STATUS_OK;
		param->reply[4] = param->state;
		if (param->state == STATE_DFU_DOWNLOAD_BUSY)
		{
			SET_LE_U24(param->reply + 1, 50);
		}
		buffer->size = 6;
		buffer->buffer = param->reply;
		break;
	case 5:		// DFU_GETSTATE
		param->reply[0] = param->state;
		buffer->size = 1;
		buffer->buffer = param->reply;
		break;
	case 4:		// DFU_CLRSTATUS
		param->state = STATE_DFU_IDLE;
		break;
	case 0:		// DFU_DETACH
	case 6:		// DFU_ABORT
		break;
	}
	
	ctrl_handler->data_size = buffer->size;
	return VSFERR_NONE;
}

static vsf_err_t vsfusbd_dfu_request_process(struct vsfusbd_device_t *device)
{
	struct vsfusbd_ctrl_handler_t *ctrl_handler = &device->ctrl_handler;
	struct usb_ctrlrequest_t *request = &ctrl_handler->request;
	struct vsfusbd_config_t *config = &device->config[device->configuration];
	struct vsfusbd_dfu_param_t *param = config->iface[request->wIndex].protocol_param;

	switch (request->bRequest)
	{
	case 1:		// DFU_DNLOAD
		if (param->op_size && (param->op_addr < FIRMWARE_AREA_SIZE_MAX))
			param->state = STATE_DFU_DOWNLOAD_BUSY;
		break;
	case 3:		// DFU_GETSTATUS
		if (param->state == STATE_DFU_DOWNLOAD_BUSY)
		{
			if (!param->op_size)
				usrapp_reset(100);
			else
			{
				uint32_t size = min(FIRMWARE_AREA_SIZE_MAX - param->op_addr, param->op_size);
				flash_erase_write(param->block, FIRMWARE_AREA_ADDR + param->op_addr, size);
				param->state = STATE_DFU_DOWNLOAD_IDLE;
			}
		}
		break;
	}
	return VSFERR_NONE;
}

static vsf_err_t vsfusbd_dfu_init(uint8_t iface, struct vsfusbd_device_t *device)
{
	struct vsfusbd_ctrl_handler_t *ctrl_handler = &device->ctrl_handler;
	struct usb_ctrlrequest_t *request = &ctrl_handler->request;
	struct vsfusbd_config_t *config = &device->config[device->configuration];
	struct vsfusbd_dfu_param_t *param = config->iface[request->wIndex].protocol_param;
	
	param->state = STATE_DFU_IDLE;
	vsfhal_flash_init(0);
}

const struct vsfusbd_class_protocol_t vsfusbd_dfu_class =
{
	.request_prepare =	vsfusbd_dfu_request_prepare,
	.request_process =	vsfusbd_dfu_request_process,
	.init = vsfusbd_dfu_init,
};

