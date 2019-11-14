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

#ifndef __VSFSCSI_H_INCLUDED__
#define __VSFSCSI_H_INCLUDED__

// SCSI
enum SCSI_PDT_t
{
	SCSI_PDT_DIRECT_ACCESS_BLOCK				= 0x00,
	SCSI_PDT_CD_DVD								= 0x05,
};

enum SCSI_sensekey_t
{
	SCSI_SENSEKEY_NO_SENSE						= 0,
	SCSI_SENSEKEY_RECOVERED_ERROR				= 1,
	SCSI_SENSEKEY_NOT_READY						= 2,
	SCSI_SENSEKEY_MEDIUM_ERROR					= 3,
	SCSI_SENSEKEY_HARDWARE_ERROR				= 4,
	SCSI_SENSEKEY_ILLEGAL_REQUEST				= 5,
	SCSI_SENSEKEY_UNIT_ATTENTION				= 6,
	SCSI_SENSEKEY_DATA_PROTECT					= 7,
	SCSI_SENSEKEY_BLANK_CHECK					= 8,
	SCSI_SENSEKEY_VENDOR_SPECIFIC				= 9,
	SCSI_SENSEKEY_COPY_ABORTED					= 10,
	SCSI_SENSEKEY_ABORTED_COMMAND				= 11,
	SCSI_SENSEKEY_VOLUME_OVERFLOW				= 13,
	SCSI_SENSEKEY_MISCOMPARE					= 14,
};

enum SCSI_asc_t
{
	SCSI_ASC_NONE								= 0x00,
	SCSI_ASC_PARAMETER_LIST_LENGTH_ERROR		= 0x1A,
	SCSI_ASC_INVALID_COMMAND					= 0x20,
	SCSI_ASC_INVALID_FIELD_IN_COMMAND			= 0x24,
	SCSI_ASC_INVALID_FIELD_IN_PARAMETER_LIST	= 0x26,
	SCSI_ASC_MEDIUM_HAVE_CHANGED				= 0x28,
	SCSI_ASC_ADDRESS_OUT_OF_RANGE				= 0x21,
	SCSI_ASC_MEDIUM_NOT_PRESENT					= 0x3A,
};

#define SCSI_GROUPCODE6							0x00
#define SCSI_GROUPCODE10_1						0x20
#define SCSI_GROUPCODE10_2						0x40
#define SCSI_GROUPCODE16						0x80
#define SCSI_GROUPCODE12						0xA0

#define SCSI_CMDCODE_TEST_UNIT_READY			0x00
#define SCSI_CMDCODE_REQUEST_SENSE				0x03	// SCSI_GROUPCODE6
#define SCSI_CMDCODE_READ_FORMAT_CAPACITIES		0x03	// SCSI_GROUPCODE10_1
#define SCSI_CMDCODE_READ_TOC					0x03	// SCSI_GROUPCODE10_2
#define SCSI_CMDCODE_FORMAT_UNIT				0x04
#define SCSI_CMDCODE_READ_CAPACITY				0x05
#define SCSI_CMDCODE_READ						0x08
#define SCSI_CMDCODE_WRITE						0x0A
#define SCSI_CMDCODE_GET_EVENT_STATUS_NOTIFY	0x0A	// SCSI_GROUPCODE10_2
#define SCSI_CMDCODE_VERIFY						0x0F
#define SCSI_CMDCODE_INQUIRY					0x12
#define SCSI_CMDCODE_MODE_SELECT				0x15
#define SCSI_CMDCODE_MODE_SENSE					0x1A
#define SCSI_CMDCODE_START_STOP_UNIT			0x1B
#define SCSI_CMDCODE_SEND_DIAGNOSTIC			0x1D
#define SCSI_CMDCODE_ALLOW_MEDIUM_REMOVAL		0x1E

// vsfscsi
struct vsfscsi_device_t;
struct vsfscsi_lun_t;
struct vsfscsi_transact_t
{
	uint64_t LBA;
	uint32_t data_size;
	vsf_err_t err;
	struct vsfscsi_lun_t *lun;
};

struct vsfscsi_lun_op_t
{
	vsf_err_t (*init)(struct vsfscsi_lun_t *lun);
	vsf_err_t (*execute)(struct vsfscsi_lun_t *lun, uint8_t *CDB,
							uint8_t CDB_size, uint32_t size);
	void (*cancel)(struct vsfscsi_lun_t *lun);
};

struct vsfscsi_device_t;
struct vsfscsi_lun_t
{
	struct vsfscsi_lun_op_t *op;
	struct vsf_stream_t *stream;
	void *param;

	// private
	struct vsfscsi_device_t *dev;
	enum SCSI_sensekey_t sensekey;
	enum SCSI_asc_t asc;
};

#define SCSI_HANDLER_NAME(title, cmd)			title##_##cmd
#define SCSI_HANDLER_NULL						{0, NULL}
struct vsfscsi_handler_t
{
	uint8_t opcode;
	vsf_err_t (*handler)(struct vsfscsi_lun_t *lun, uint8_t *CDB);
};

struct vsfscsi_device_t
{
	uint8_t max_lun;
	struct vsfscsi_lun_t *lun;

	struct vsfscsi_transact_t transact;
};

// mal2scsi
// lun->param is pointer to vsf_mal2scsi_t
struct vsf_mal2scsi_cparam_t
{
	uint32_t block_size;
	bool removable;
	char vendor[8];
	char product[16];
	char revision[4];
	enum SCSI_PDT_t type;
};

// scsistream can be bufstream or mbufstream
// lun->stream MUST be scsistream for mal2scsi
struct vsf_scsistream_t
{
	struct vsf_stream_t stream;
	union
	{
		struct vsf_mbufstream_mem_t mbufstream_mem;
		struct vsf_bufstream_mem_t bufstream_mem;
	};
	struct vsf_multibuf_t mbuf;
};

struct vsf_mal2scsi_t
{
	struct vsfscsi_handler_t *vendor_handlers;
	struct vsf_mal2scsi_cparam_t cparam;
	void *param;

	struct vsf_malstream_t malstream;
};

// scsi2mal
struct vsf_scsi2mal_t
{
	struct vsfscsi_lun_t *lun;
	struct vsf_mbufstream_t bufstream;
	uint8_t *cur_pos;
	uint32_t size;
};

#ifndef VSFCFG_EXCLUDE_SCSI
vsf_err_t vsfscsi_init(struct vsfscsi_device_t *dev);
vsf_err_t vsfscsi_execute(struct vsfscsi_lun_t *lun, uint8_t *CDB,
							uint8_t CDB_size, uint32_t size);
void vsfscsi_cancel_transact(struct vsfscsi_transact_t *transact);
void vsfscsi_release_transact(struct vsfscsi_transact_t *transact);
extern const struct vsfscsi_lun_op_t vsf_mal2scsi_op;
extern const struct vsfmal_drv_t vsf_scsi2mal_op;
#endif

#endif // __VSFSCSI_H_INCLUDED__
