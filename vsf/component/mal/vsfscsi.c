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

void vsfscsi_release_transact(struct vsfscsi_transact_t *transact)
{
	transact->lun = NULL;
}

vsf_err_t vsfscsi_init(struct vsfscsi_device_t *dev)
{
	uint32_t i;
	struct vsfscsi_lun_t *lun = dev->lun;

	for (i = 0; i <= dev->max_lun; i++, lun++)
	{
		lun->dev = dev;
		lun->sensekey = SCSI_SENSEKEY_NO_SENSE;
		lun->asc = SCSI_ASC_NONE;
		if (lun->op->init != NULL)
		{
			lun->op->init(lun);
		}
	}
	return VSFERR_NONE;
}

static struct vsfscsi_handler_t*
vsfscsi_get_handler(struct vsfscsi_handler_t *handlers, uint8_t opcode)
{
	while (handlers->handler != NULL)
	{
		if (handlers->opcode == opcode)
		{
			return handlers;
		}
		handlers++;
	}
	return NULL;
}

vsf_err_t vsfscsi_execute(struct vsfscsi_lun_t *lun, uint8_t *CDB,
							uint8_t CDB_size, uint32_t size)
{
	return lun->op->execute ?
			lun->op->execute(lun, CDB, CDB_size, size) : VSFERR_FAIL;
}

void vsfscsi_cancel_transact(struct vsfscsi_transact_t *transact)
{
	// TODO: release other resources & callbacks
	vsfscsi_release_transact(transact);
}

// mal2scsi
static uint8_t* vsf_mal2scsi_prepare_transact(struct vsfscsi_lun_t *lun,
									uint32_t size, bool write, bool bufstream)
{
	struct vsfscsi_transact_t *transact = &lun->dev->transact;
	struct vsf_scsistream_t *stream = (struct vsf_scsistream_t *)lun->stream;
	uint8_t *buffer = stream->mbuf.buffer_list[0];

	if (NULL == buffer)
	{
		return NULL;
	}

	if (bufstream)
	{
		struct vsf_bufstream_t *bufstream = (struct vsf_bufstream_t *)stream;

		memset(buffer, 0, size);
		// setup bufstream
		bufstream->mem.buffer.buffer = buffer;
		bufstream->mem.buffer.size = size;
		bufstream->mem.read = true;
		stream->stream.op = &bufstream_op;
		stream->stream.callback_tx.param = transact;
		stream->stream.callback_tx.on_inout = NULL;
		stream->stream.callback_tx.on_connect = NULL;
		stream->stream.callback_tx.on_disconnect = NULL;
		STREAM_INIT(stream);
	}
	else
	{
		struct vsf_mbufstream_t *mbufstream = (struct vsf_mbufstream_t *)stream;

		// setup malstream
		mbufstream->mem.multibuf = stream->mbuf;
		stream->stream.op = &mbufstream_op;
		STREAM_INIT(stream);
	}
	transact->data_size = size;
	transact->err = VSFERR_NONE;
	transact->lun = lun;
	return buffer;
}

static void vsf_mal2scsi_mal_finish(void *param)
{
	struct vsfscsi_lun_t *lun = (struct vsfscsi_lun_t *)param;
	struct vsf_mal2scsi_t *mal2scsi = (struct vsf_mal2scsi_t *)lun->param;
	struct vsf_malstream_t *malstream = &mal2scsi->malstream;
	struct vsfscsi_transact_t *transact = &lun->dev->transact;

	if (malstream->offset < malstream->size)
	{
		transact->err = VSFERR_FAIL;
	}
}

static vsf_err_t vsf_mal2scsi_execute(struct vsfscsi_lun_t *lun, uint8_t *CDB,
							uint8_t CDB_size, uint32_t size)
{
	struct vsf_mal2scsi_t *mal2scsi = (struct vsf_mal2scsi_t *)lun->param;
	struct vsfscsi_transact_t *transact = &lun->dev->transact;
	struct vsfmal_t *mal = mal2scsi->malstream.mal;
	uint8_t group_code = CDB[0] & 0xE0, cmd_code = CDB[0] & 0x1F;
	uint8_t *pbuf;

	if (transact->lun != NULL)
	{
		return VSFERR_FAIL;
	}

	// check user_handler first
	if (mal2scsi->vendor_handlers != NULL)
	{
		struct vsfscsi_handler_t *handler =
				vsfscsi_get_handler(mal2scsi->vendor_handlers, CDB[0]);
		if (handler != NULL)
		{
			return handler->handler(lun, CDB);
		}
	}

	if (!mal2scsi->malstream.mal_ready || (transact->lun != NULL))
	{
		goto exit_not_ready;
	}

	switch (group_code)
	{
	case SCSI_GROUPCODE6:
		switch (cmd_code)
		{
		case SCSI_CMDCODE_MODE_SELECT:
		case SCSI_CMDCODE_TEST_UNIT_READY:
		case SCSI_CMDCODE_VERIFY:
		case SCSI_CMDCODE_FORMAT_UNIT:
		case SCSI_CMDCODE_START_STOP_UNIT:
		case SCSI_CMDCODE_ALLOW_MEDIUM_REMOVAL:
			break;
		case SCSI_CMDCODE_REQUEST_SENSE:
			pbuf = vsf_mal2scsi_prepare_transact(lun, 18, false, true);
			if (NULL == pbuf)
			{
				goto exit_not_ready;
			}

			pbuf[0] = 0x70;
			pbuf[2] = lun->sensekey;
			pbuf[7] = 0x0A;
			pbuf[12] = lun->asc;
			transact->data_size = min(CDB[4], 18);
			break;
		case SCSI_CMDCODE_READ:
			transact->LBA = GET_BE_U16(&CDB[2]);
			transact->data_size = CDB[4];
			goto do_mal_read;
			break;
		case SCSI_CMDCODE_WRITE:
			transact->LBA = GET_BE_U16(&CDB[2]);
			transact->data_size = CDB[4];
			goto do_mal_write;
			break;
		case SCSI_CMDCODE_INQUIRY:
			if (CDB[1] & 1)
			{
				// When the EVPD bit is set to one,
				// the PAGE CODE field specifies which page of
				// vital product data information the device server shall return
				if (CDB[2] != 0)
				{
					goto exit_invalid_field_in_command;
				}

				// 0x00: Supported VPD Pages
				pbuf = vsf_mal2scsi_prepare_transact(lun, 5, false, true);
				if (NULL == pbuf)
				{
					goto exit_not_ready;
				}
			}
			else
			{
				struct vsf_mal2scsi_cparam_t *cparam =
							(struct vsf_mal2scsi_cparam_t *)&mal2scsi->cparam;

				if (CDB[2] != 0)
				{
					// If the PAGE CODE field is not set to zero
					// when the EVPD bit is set to zero,
					// the command shall be terminated with CHECK CONDITION status,
					// with the sense key set to ILLEGAL REQUEST,
					// and the additional sense code set to INVALID FIELD IN CDB.
					goto exit_invalid_field_in_command;
				}

				// If the EVPD bit is set to zero,
				// the device server shall return the standard INQUIRY data.
				pbuf = vsf_mal2scsi_prepare_transact(lun, 36, false, true);
				if (NULL == pbuf)
				{
					goto exit_not_ready;
				}

				pbuf[0] = cparam->type;
				if (cparam->removable)
				{
					pbuf[1] = 0x80;
				}
				pbuf[3] = 2;
				pbuf[4] = 31;
				pbuf += 8;
				memcpy(pbuf, cparam->vendor, sizeof(cparam->vendor));
				pbuf += sizeof(cparam->vendor);
				memcpy(pbuf, cparam->product, sizeof(cparam->product));
				pbuf += sizeof(cparam->product);
				memcpy(pbuf, cparam->revision, sizeof(cparam->revision));
			}
			break;
		case SCSI_CMDCODE_MODE_SENSE:
			pbuf = vsf_mal2scsi_prepare_transact(lun, 4, false, true);
			if (NULL == pbuf)
			{
				goto exit_not_ready;
			}

			pbuf[0] = 3;
			break;
		default:
			goto exit_invalid_command;
		}
		break;
	case SCSI_GROUPCODE10_1:
		switch (cmd_code)
		{
		case SCSI_CMDCODE_READ_FORMAT_CAPACITIES:
			pbuf = vsf_mal2scsi_prepare_transact(lun, 12, false, true);
			if (NULL == pbuf)
			{
				goto exit_not_ready;
			}

			pbuf[3] = 8;
			SET_BE_U32(&pbuf[4], mal->cap.block_num);
			SET_BE_U32(&pbuf[8], mal->cap.block_size);
			pbuf[8] = 2;
			break;
		case SCSI_CMDCODE_READ_CAPACITY:
			pbuf = vsf_mal2scsi_prepare_transact(lun, 8, false, true);
			if (NULL == pbuf)
			{
				goto exit_not_ready;
			}

			SET_BE_U32(&pbuf[0], mal->cap.block_num - 1);
			SET_BE_U32(&pbuf[4], mal->cap.block_size);
			break;
		case SCSI_CMDCODE_READ:
			transact->LBA = GET_BE_U32(&CDB[2]);
			transact->data_size = GET_BE_U16(&CDB[7]);
			goto do_mal_read;
			break;
		case SCSI_CMDCODE_WRITE:
			transact->LBA = GET_BE_U32(&CDB[2]);
			transact->data_size = GET_BE_U16(&CDB[7]);
			goto do_mal_write;
			break;
		default:
			goto exit_invalid_command;
		}
		break;
	case SCSI_GROUPCODE10_2:
		switch (cmd_code)
		{
		case SCSI_CMDCODE_MODE_SELECT:
			break;
		case SCSI_CMDCODE_MODE_SENSE:
			break;
		default:
			goto exit_invalid_command;
		}
		break;
	case SCSI_GROUPCODE16:
		switch (cmd_code)
		{
		case SCSI_CMDCODE_READ:
			transact->LBA = GET_BE_U64(&CDB[2]);
			transact->data_size = GET_BE_U32(&CDB[10]);

		do_mal_read:
			transact->LBA *= mal->cap.block_size;
			transact->data_size *= mal->cap.block_size;
			pbuf = vsf_mal2scsi_prepare_transact(lun, transact->data_size,
													false, false);
			if (NULL == pbuf)
			{
				goto exit_not_ready;
			}

			vsf_malstream_read(&mal2scsi->malstream, transact->LBA,
									transact->data_size);
			break;
		case SCSI_CMDCODE_WRITE:
			transact->LBA = GET_BE_U64(&CDB[2]);
			transact->data_size = GET_BE_U32(&CDB[10]);

		do_mal_write:
			transact->LBA *= mal->cap.block_size;
			transact->data_size *= mal->cap.block_size;
			pbuf = vsf_mal2scsi_prepare_transact(lun, transact->data_size,
													true, false);
			if (NULL == pbuf)
			{
				goto exit_not_ready;
			}

			vsf_malstream_write(&mal2scsi->malstream, transact->LBA,
									transact->data_size);
			break;
		default:
			goto exit_invalid_command;
		}
		break;
	case SCSI_GROUPCODE12:
		switch (cmd_code)
		{
		case SCSI_CMDCODE_READ:
			transact->LBA = GET_BE_U32(&CDB[2]);
			transact->data_size = GET_BE_U32(&CDB[6]);
			goto do_mal_read;
			break;
		case SCSI_CMDCODE_WRITE:
			transact->LBA = GET_BE_U32(&CDB[2]);
			transact->data_size = GET_BE_U32(&CDB[6]);
			goto do_mal_write;
			break;
		default:
			goto exit_invalid_command;
		}
		break;
	default:
		goto exit_invalid_command;
	}

	lun->sensekey = SCSI_SENSEKEY_NO_SENSE;
	lun->asc = SCSI_ASC_NONE;
	if ((transact->lun != NULL) && (transact->lun->stream != NULL) &&
		(transact->lun->stream->op == &bufstream_op))
	{
		stream_connect_tx(transact->lun->stream);
	}
	return VSFERR_NONE;
exit_invalid_command:
	lun->sensekey = SCSI_SENSEKEY_ILLEGAL_REQUEST;
	lun->asc = SCSI_ASC_INVALID_COMMAND;
	return VSFERR_FAIL;
exit_invalid_field_in_command:
	lun->sensekey = SCSI_SENSEKEY_ILLEGAL_REQUEST;
	lun->asc = SCSI_ASC_INVALID_FIELD_IN_COMMAND;
	return VSFERR_FAIL;
exit_not_ready:
	lun->sensekey = SCSI_SENSEKEY_NOT_READY;
	lun->asc = SCSI_ASC_NONE;
	return VSFERR_FAIL;
}

static vsf_err_t vsf_mal2scsi_init(struct vsfscsi_lun_t *lun)
{
	struct vsf_mal2scsi_t *mal2scsi = (struct vsf_mal2scsi_t *)lun->param;
	mal2scsi->malstream.mbufstream = (struct vsf_mbufstream_t *)lun->stream;
	mal2scsi->malstream.cb.param = lun;
	mal2scsi->malstream.cb.on_finish = vsf_mal2scsi_mal_finish;
	return vsf_malstream_init(&mal2scsi->malstream);
}

// scsi2mal
static uint32_t vsf_scsi2mal_blocksize(struct vsfmal_t *mal, uint64_t addr,
					uint32_t size, enum vsfmal_op_t op)
{
	return mal->cap.block_size;
}

static vsf_err_t vsf_scsi2mal_init(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct vsfmal_t *mal = (struct vsfmal_t *)pt->user_data;
	struct vsf_scsi2mal_t *scsi2mal = (struct vsf_scsi2mal_t *)mal->param;

	vsfsm_pt_begin(pt);

	

	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

static vsf_err_t vsf_scsi2mal_fini(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
}

static vsf_err_t vsf_scsi2mal_read(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
					uint64_t addr, uint8_t *buff, uint32_t size)
{
}

static vsf_err_t vsf_scsi2mal_write(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
					uint64_t addr, uint8_t *buff, uint32_t size)
{
}

const struct vsfscsi_lun_op_t vsf_mal2scsi_op =
{
	.init = vsf_mal2scsi_init,
	.execute = vsf_mal2scsi_execute,
};
const struct vsfmal_drv_t vsf_scsi2mal_op =
{
	.block_size = vsf_scsi2mal_blocksize,
	.init = vsf_scsi2mal_init,
	.fini = vsf_scsi2mal_fini,
	.read = vsf_scsi2mal_read,
	.write = vsf_scsi2mal_write,
};

