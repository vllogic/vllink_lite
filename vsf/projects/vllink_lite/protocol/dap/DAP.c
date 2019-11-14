#include "vsf.h"
#include "DAP.h"
#include "peripheral.h"

#ifndef DAP_BLOCK_TRANSFER

#define DAP_FW_VER      "1.10"  // Firmware Version

#ifdef DAP_VENDOR
const char DAP_Vendor[] = DAP_VENDOR;
#endif
#ifdef DAP_PRODUCT
const char DAP_Product[] = DAP_PRODUCT;
#endif
#ifdef DAP_SER_NUM
const char DAP_SerNum[] = DAP_SER_NUM;
#endif
const char DAP_FW_Ver[] = DAP_FW_VER;

#define VSFSM_EVT_IO_DONE				(VSFSM_EVT_USER_LOCAL + 1)
#define VSFSM_EVT_DAP_REQUEST			(VSFSM_EVT_USER_LOCAL + 2)
#define VSFSM_EVT_DAP_RESPONSE			(VSFSM_EVT_USER_LOCAL + 3)
#define VSFSM_EVT_ON_REVICE				(VSFSM_EVT_USER_LOCAL + 4)
#define VSFSM_EVT_ON_SEND_DONE			(VSFSM_EVT_USER_LOCAL + 5)

static uint8_t get_dap_info(uint8_t id, uint8_t *info)
{
	uint8_t length = 0U;

	switch (id)
	{
	case DAP_ID_VENDOR:
#ifdef DAP_VENDOR
		if (info)
			memcpy(info, DAP_Vendor, sizeof(DAP_Vendor));
		length = sizeof(DAP_Vendor);
#endif
		break;
	case DAP_ID_PRODUCT:
#ifdef DAP_PRODUCT
		if (info)
		memcpy(info, DAP_Product, sizeof(DAP_Product));
		length = (uint8_t)sizeof(DAP_Product);
#endif
		break;
	case DAP_ID_SER_NUM:
#ifdef DAP_SER_NUM
		if (info)
			memcpy(info, DAP_SerNum, sizeof(DAP_SerNum));
		length = (uint8_t)sizeof(DAP_SerNum);
#endif
		break;
	case DAP_ID_FW_VER:
		if (info)
			memcpy(info, DAP_FW_Ver, sizeof(DAP_FW_Ver));
		length = (uint8_t)sizeof(DAP_FW_Ver);
		break;
	case DAP_ID_DEVICE_VENDOR:
#if TARGET_DEVICE_FIXED
		if (info)
			memcpy(info, TargetDeviceVendor, sizeof(TargetDeviceVendor));
		length = (uint8_t)sizeof(TargetDeviceVendor);
#endif
		break;
	case DAP_ID_DEVICE_NAME:
#if TARGET_DEVICE_FIXED
		if (info)
			memcpy(info, TargetDeviceName, sizeof(TargetDeviceName));
		length = (uint8_t)sizeof(TargetDeviceName);
#endif
		break;
	case DAP_ID_CAPABILITIES:
		if (info)
			info[0] = (DAP_SWD ? (1U << 0) : 0U) |
					(DAP_JTAG ? (1U << 1) : 0U) |
					(SWO_UART ? (1U << 2) : 0U) |
					(SWO_MANCHESTER ? (1U << 3) : 0U) |
					/* Atomic Commands  */ (1U << 4);
		length = 1U;
		break;
	case DAP_ID_SWO_BUFFER_SIZE:
#if ((SWO_UART != 0) || (SWO_MANCHESTER != 0))
		if (info)
			*(uint32_t *)info = SWO_BUFFER_SIZE;
		length = 4U;
#endif
		break;
	case DAP_ID_PACKET_SIZE:
		if (info)
			*(uint16_t *)info = DAP_PACKET_SIZE;
		length = 2U;
		break;
	case DAP_ID_PACKET_COUNT:
		if (info)
			info[0] = DAP_PACKET_COUNT;
		length = 1U;
		break;
	default:
		break;
	}

	return length;
}

static void dap_callback(void *p, uint32_t ack, uint32_t data)
{
	struct dap_param_t *param = p;
	param->transfer_ack = ack & 0xff;
	param->transfer_data = data;
	vsfsm_post_evt_pending(&param->sm, VSFSM_EVT_IO_DONE);
}

static vsf_err_t port_init(struct dap_param_t *param, uint8_t port)
{
	vsf_err_t err = VSFERR_NONE;

	if (port == DAP_PORT_SWD)
	{
		err = peripheral_register(PERIPHERAL_SWD);
		if (err == VSFERR_NONE)
		{
			vsfhal_swd_init(PERIPHERAL_SWD_PRIORITY);
			vsfhal_swd_config(param->khz, param->transfer.idle_cycles, 
					param->swd_conf.turnaround, param->swd_conf.data_phase,
					param->transfer.retry_count, param,
					dap_callback);
		}
	}
	else if (port == DAP_PORT_JTAG)
	{
		err = peripheral_register(PERIPHERAL_JTAG);
		if (err == VSFERR_NONE)
		{
			vsfhal_jtag_init(PERIPHERAL_JTAG_PRIORITY);
			vsfhal_jtag_config(param->khz, param->transfer.retry_count,
					param->transfer.idle_cycles, param, dap_callback);
		}
	}
	return err;
}

static vsf_err_t port_fini(uint8_t port)
{
	if (port == DAP_PORT_SWD)
	{
		vsfhal_swd_fini();
		peripheral_unregister(PERIPHERAL_SWD);
	}
	else if (port == DAP_PORT_JTAG)
	{
		vsfhal_jtag_fini();
		peripheral_unregister(PERIPHERAL_JTAG);
	}
	return VSFERR_NONE;
}

static void bit64_set_by_array(uint64_t *bit64, uint8_t pos, uint8_t *data, uint8_t bitlen)
{
	uint64_t v = GET_LE_U64(data), mask = 0xffffffffffffffff;
	
	if (bitlen < 64)
		mask >>= (64 - bitlen);
	v <<= pos;
	mask <<= pos;
	
	*bit64 &= ~mask;
	*bit64 |= v & mask;
}

static void bit64_set_by_bit(uint64_t *bit64, uint8_t pos, uint8_t bit, uint8_t bitlen)
{
	uint64_t mask = 0xffffffffffffffff;
	
	if (bitlen < 64)
		mask >>= (64 - bitlen);
	mask <<= pos;
	if (bit)
		*bit64 |= mask;
	else
		*bit64 &= ~mask;
}

static vsf_err_t cmd_thread(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	vsf_err_t err;
	uint32_t data;
	struct dap_param_t *param = pt->user_data;
	uint8_t *request = param->request[param->request_head];
	uint8_t *response = param->response;

	vsfsm_pt_begin(pt);

	param->cmd_num--;
	param->cmd_id = request[param->req_ptr++];
	response[param->resp_ptr++] = param->cmd_id;
	
	if ((param->cmd_id == ID_DAP_ExecuteCommands) && (param->resp_ptr <= (DAP_PACKET_SIZE - 2)))
	{
		param->cmd_num = request[param->req_ptr++];
		response[param->resp_ptr++] = param->cmd_num;
	}
	else if (param->cmd_id == ID_DAP_Info)
	{
		data = request[param->req_ptr++];
		if (param->resp_ptr <= (DAP_PACKET_SIZE - 1 - get_dap_info(data, NULL)))
		{
			uint8_t num = get_dap_info(data, &response[param->resp_ptr + 1]);
			response[param->resp_ptr] = num;
			param->resp_ptr += num + 1;
		}
		else
			goto fault;
	}
	else if (param->cmd_id == ID_DAP_HostStatus)
	{
		data = request[param->req_ptr++];
		if (data == DAP_DEBUGGER_CONNECTED)
		{
			if (request[param->req_ptr++] & 0x1)
				PERIPHERAL_LED_RED_ON();
			else
				PERIPHERAL_LED_RED_OFF();
			response[param->resp_ptr++] = DAP_OK;
		}
		else if (data == DAP_TARGET_RUNNING)
		{
			if (request[param->req_ptr++] & 0x1)
				PERIPHERAL_LED_GREEN_ON();
			else
				PERIPHERAL_LED_GREEN_OFF();
			response[param->resp_ptr++] = DAP_OK;
		}
		else
			response[param->resp_ptr++] = DAP_ERROR;
	}
	else if (param->cmd_id == ID_DAP_Connect)
	{
		data = request[param->req_ptr++];
		if (data == DAP_PORT_AUTODETECT)
			data = DAP_DEFAULT_PORT;

		if (data == DAP_PORT_SWD)
		{
			port_fini(param->port);
			if (port_init(param, DAP_PORT_SWD) == VSFERR_NONE)
				data = DAP_PORT_SWD;
			else
				data = DAP_PORT_DISABLED;
		}
		else if (data == DAP_PORT_JTAG)
		{
			port_fini(param->port);
			if (port_init(param, DAP_PORT_JTAG) == VSFERR_NONE)
				data = DAP_PORT_JTAG;
			else
				data = DAP_PORT_DISABLED;
		}
		else
			data = DAP_PORT_DISABLED;
		param->port = data;
		response[param->resp_ptr++] = data;
	}
	else if (param->cmd_id == ID_DAP_Disconnect)
	{
		port_fini(param->port);
		param->port = DAP_PORT_DISABLED;
		response[param->resp_ptr++] = DAP_OK;
	}
	else if (param->cmd_id == ID_DAP_Delay)
	{
		data = GET_LE_U16(request + param->req_ptr) + 999 / 1000;
		param->resp_ptr += 2;
		vsfsm_pt_delay(pt, data);
		response[param->resp_ptr++] = DAP_OK;
	}
	else if (param->cmd_id == ID_DAP_ResetTarget)
	{
		response[param->resp_ptr++] = DAP_OK;
		response[param->resp_ptr++] = 0;	// reset target return
	}
	else if (param->cmd_id == ID_DAP_SWJ_Pins)
	{
		uint8_t value, select;
		uint32_t delay;
		value = request[param->req_ptr];
		select = request[param->req_ptr + 1];
		delay = GET_LE_U32(request + param->req_ptr + 2);

		if (param->port == DAP_PORT_SWD)
		{
			// output pin support: srst
			if (select & (0x1 << DAP_SWJ_nRESET))
			{
				if (value & (0x1 << DAP_SWJ_nRESET))
				{
					PERIPHERAL_GPIO_SRST_SET();
					PERIPHERAL_GPIO_SRST_SET_OUTPUT();
				}
				else
				{
					PERIPHERAL_GPIO_SRST_CLEAR();
					PERIPHERAL_GPIO_SRST_SET_OUTPUT();
				}
			}
			if (delay)
			{
				if (delay > 3000000)
					delay = 3000000;
				vsfsm_pt_delay(pt, (delay + 999) / 1000);
			}
			
			// input pin support: tck srst
			PERIPHERAL_GPIO_SRST_SET_INPUT();
			response[param->resp_ptr++] = (PERIPHERAL_GPIO_TCK_GET() ? (0x1 << DAP_SWJ_SWCLK_TCK) : 0) |
					(PERIPHERAL_GPIO_SRST_GET() ? (0x1 << DAP_SWJ_nRESET) : 0);
		}
		else if (param->port == DAP_PORT_JTAG)
		{
			// output pin support: trst srst
			if (select & (0x1 << DAP_SWJ_nRESET))
			{
				if (value & (0x1 << DAP_SWJ_nTRST))
					PERIPHERAL_GPIO_TRST_SET();
				else
					PERIPHERAL_GPIO_TRST_CLEAR();
			}
			if (select & (0x1 << DAP_SWJ_nRESET))
			{
				if (value & (0x1 << DAP_SWJ_nRESET))
				{
					PERIPHERAL_GPIO_SRST_SET();
					PERIPHERAL_GPIO_SRST_SET_OUTPUT();
				}
				else
				{
					PERIPHERAL_GPIO_SRST_CLEAR();
					PERIPHERAL_GPIO_SRST_SET_OUTPUT();
				}
			}
			if (delay)
			{
				if (delay > 3000000)
					delay = 3000000;
				vsfsm_pt_delay(pt, (delay + 999) / 1000);
			}
			
			// input pin support: srst
			PERIPHERAL_GPIO_SRST_SET_INPUT();
			response[param->resp_ptr++] = PERIPHERAL_GPIO_SRST_GET() ? (0x1 << DAP_SWJ_nRESET) : 0;
		}
		else
			response[param->resp_ptr++] = 0;
	}
	else if (param->cmd_id == ID_DAP_SWJ_Clock)
	{
		data = GET_LE_U32(request + param->req_ptr);
		param->req_ptr += 4;
		if (data)
		{
			param->khz = data / 1000;
			if (param->port == DAP_PORT_SWD)
				vsfhal_swd_config(param->khz, param->transfer.idle_cycles, 
						param->swd_conf.turnaround, param->swd_conf.data_phase,
						param->transfer.retry_count, param,
						dap_callback);
			else if (param->port == DAP_PORT_JTAG)			
				vsfhal_jtag_config(param->khz, param->transfer.retry_count,
						param->transfer.idle_cycles, param, dap_callback);
			response[param->resp_ptr++] = DAP_OK;
		}
		else
			response[param->resp_ptr++] = DAP_ERROR;
	}
	else if (param->cmd_id == ID_DAP_SWJ_Sequence)
	{
		data = request[param->req_ptr++];
		if (!data)
			data = 256;

		if (param->port == DAP_PORT_SWD)
		{
			vsfhal_swd_seqout(request + param->req_ptr, data);
			param->req_ptr += (data + 7) >> 3;
			vsfsm_pt_wfe(pt, VSFSM_EVT_IO_DONE);
			response[param->resp_ptr++] = DAP_OK;
		}
		else if (param->port == DAP_PORT_JTAG)
		{
			uint8_t bytes;			
			data = min(data, 64);
			bytes = (data + 7) >> 3;
			memcpy(&param->jtag_dev.buf_tms, request + param->req_ptr, bytes);
			param->req_ptr += bytes;
			param->jtag_dev.buf_tdi = 0xffffffffffffffff;
			err = vsfhal_jtag_raw(data, (uint8_t *)&param->jtag_dev.buf_tdi,
					(uint8_t *)&param->jtag_dev.buf_tms, (uint8_t *)&param->jtag_dev.buf_tdo);
			if (err == VSFERR_NONE)
				vsfsm_pt_wfe(pt, VSFSM_EVT_IO_DONE);
			response[param->resp_ptr++] = DAP_OK;
		}
		else
		{
			param->req_ptr += (data + 7) >> 3;
			response[param->resp_ptr++] = DAP_ERROR;
		}
	}
	else if (param->cmd_id == ID_DAP_SWD_Configure)
	{
		uint8_t v = request[param->req_ptr++];
		param->swd_conf.turnaround = (v & 0x03) + 1;
		param->swd_conf.data_phase = (v & 0x04) ? 1 : 0;
		vsfhal_swd_config(param->khz, param->transfer.idle_cycles, 
				param->swd_conf.turnaround, param->swd_conf.data_phase,
				param->transfer.retry_count, param,
				dap_callback);
		response[param->resp_ptr++] = DAP_OK;
	}
	else if (param->cmd_id == ID_DAP_JTAG_Sequence)
	{
		if (param->port == DAP_PORT_JTAG)
		{
			response[param->resp_ptr++] = DAP_OK;
			
			param->transfer_cnt = 0;
			param->transfer_num = request[param->req_ptr++];
			
			while (param->transfer_cnt < param->transfer_num)
			{
				uint8_t info, bits, bitlen;
				uint16_t ptr;
						
				// prepare tms tdi
				ptr = param->req_ptr;
				param->jtag_seq.prepare_transfer_cnt = param->transfer_cnt;
				bitlen = 0;
				do
				{
					info = request[ptr++];
					bits = info & JTAG_SEQUENCE_TCK;
					if (!bits) bits = 64;
					if ((bits + bitlen) > 64)
						break;
					bit64_set_by_array(&param->jtag_dev.buf_tdi, bitlen,
							request + ptr, bits);
					bit64_set_by_bit(&param->jtag_dev.buf_tms, bitlen,
							(info & JTAG_SEQUENCE_TMS) ? 1 : 0, bits);
					bitlen += bits;
					ptr += (bits + 7) >> 3;
					param->jtag_seq.prepare_transfer_cnt++;
				} while (param->jtag_seq.prepare_transfer_cnt < param->transfer_num);
				
				// transfer
				err = vsfhal_jtag_raw(bitlen, (uint8_t *)&param->jtag_dev.buf_tdi,
						(uint8_t *)&param->jtag_dev.buf_tms,
						(uint8_t *)&param->jtag_dev.buf_tdo);
				if (err == VSFERR_NONE)
					vsfsm_pt_wfe(pt, VSFSM_EVT_IO_DONE);

				// process tdo
				bitlen = 0;
				do
				{
					info = request[param->req_ptr++];
					bits = info & JTAG_SEQUENCE_TCK;
					if (!bits) bits = 64;
					if (info & JTAG_SEQUENCE_TDO)
					{
						uint8_t i;
						uint64_t temp = param->jtag_dev.buf_tdo >> bitlen;
						for (i = 0; i < bits; i += 8)
						{
							uint16_t mask = (0x1ul << min(bits - i, 8)) - 1;
							response[param->resp_ptr++] = temp & mask;
							temp >>= 8;
						}
					}
					bitlen += bits;
					param->req_ptr += (bits + 7) >> 3;
					param->transfer_cnt++;
				} while (param->transfer_cnt < param->jtag_seq.prepare_transfer_cnt);
			}
		}
		else
			response[param->resp_ptr++] = DAP_ERROR;
	}
	else if (param->cmd_id == ID_DAP_JTAG_Configure)
	{
		uint32_t n, count, length, bits = 0;
		
		count = request[param->req_ptr++];
		param->jtag_dev.count = min(count, DAP_JTAG_DEV_CNT);
		for (n = 0; n < count; n++)
		{
			length = request[param->req_ptr++];
			if (n < DAP_JTAG_DEV_CNT)
			{
				param->jtag_dev.ir_length[n] = length;
				param->jtag_dev.ir_before[n] = bits;
			}
			bits += length;
		}
		for (n = 0; n < count; n++)
		{
			bits -= param->jtag_dev.ir_length[n];
			if (n < DAP_JTAG_DEV_CNT)
				param->jtag_dev.ir_after[n] = bits;
		}
		response[param->resp_ptr++] = DAP_OK;
	}
	else if (param->cmd_id == ID_DAP_JTAG_IDCODE)
	{
		param->jtag_dev.index = request[param->req_ptr++];
		
		if ((param->port == DAP_PORT_JTAG) && (param->jtag_dev.index < DAP_JTAG_DEV_CNT))
		{
			uint16_t bitlen;
			
			// Select JTAG chain
			err = vsfhal_jtag_ir(JTAG_IDCODE, param->jtag_dev.ir_length[param->jtag_dev.index],
					param->jtag_dev.ir_before[param->jtag_dev.index],
					param->jtag_dev.ir_after[param->jtag_dev.index]);
			if (err != VSFERR_NONE) goto JTAG_IDCODE_EXIT;
			vsfsm_pt_wfe(pt, VSFSM_EVT_IO_DONE);
			if (param->transfer_ack != DAP_TRANSFER_OK)
				goto JTAG_IDCODE_EXIT;
			
			// Read IDCODE register
			param->jtag_dev.buf_tdi = 0;
			param->jtag_dev.buf_tms = 0;
			param->jtag_dev.buf_tdo = 0;
			bitlen = 0;
			param->jtag_dev.buf_tms |= 0x1 << bitlen;
			bitlen += 3 + param->jtag_dev.index + 32;
			param->jtag_dev.buf_tms |= 0x3 << (bitlen - 1);
			bitlen += 2;
			err = vsfhal_jtag_raw(bitlen, (uint8_t *)&param->jtag_dev.buf_tdi,
					(uint8_t *)&param->jtag_dev.buf_tms,
					(uint8_t *)&param->jtag_dev.buf_tdo);
			if (err != VSFERR_NONE) goto JTAG_IDCODE_EXIT;
			vsfsm_pt_wfe(pt, VSFSM_EVT_IO_DONE);
			if (param->transfer_ack != DAP_TRANSFER_OK)
				goto JTAG_IDCODE_EXIT;

			response[param->resp_ptr++] = DAP_OK;
			SET_LE_U32(response + param->resp_ptr, (uint32_t)(param->jtag_dev.buf_tdo >> (3 + param->jtag_dev.index)));
			param->resp_ptr += 4;
		}
		else
		{
JTAG_IDCODE_EXIT:
			response[param->resp_ptr++] = DAP_ERROR;
		}
	}
	else if (param->cmd_id == ID_DAP_TransferConfigure)
	{
		param->transfer.idle_cycles = request[param->req_ptr];
		param->transfer.retry_count = GET_LE_U16(request + param->req_ptr + 1);
		param->transfer.match_retry = GET_LE_U16(request + param->req_ptr + 3);
		
		if (param->port == DAP_PORT_SWD)
			vsfhal_swd_config(param->khz, param->transfer.idle_cycles, 
					param->swd_conf.turnaround, param->swd_conf.data_phase,
					param->transfer.retry_count, param,
					dap_callback);
		else if (param->port == DAP_PORT_JTAG)
			vsfhal_jtag_config(param->khz, param->transfer.retry_count,
					param->transfer.idle_cycles, param, dap_callback);
		param->req_ptr += 5;
		response[param->resp_ptr++] = DAP_OK;
	}
	else if (param->cmd_id == ID_DAP_Transfer)
	{
		if (param->port == DAP_PORT_SWD)
		{
			param->resp_start = param->resp_ptr;
			param->transfer_cnt = 0;
			param->transfer_num = request[param->req_ptr + 1];
			param->abort = false;
			param->post_read = false;
			param->check_write = false;
			param->transfer_ack = 0;
			param->req_ptr += 2;
			param->resp_ptr += 2;

			while (param->transfer_cnt < param->transfer_num)
			{
				param->transfer_cnt++;
				param->transfer_req = request[param->req_ptr++];
				
				if (param->transfer_req & DAP_TRANSFER_RnW)	// read
				{
					if (param->post_read)
					{
						if ((param->transfer_req & (DAP_TRANSFER_APnDP | DAP_TRANSFER_MATCH_VALUE)) == DAP_TRANSFER_APnDP)
						{
							// Read previous AP data and post next AP read
							err = vsfhal_swd_transact(param->transfer_req, 0);
						}
						else
						{
							param->post_read = false;
							// Read previous AP data
							err = vsfhal_swd_transact(DP_RDBUFF | DAP_TRANSFER_RnW, 0);
						}
						if (err != VSFERR_NONE) break;
						vsfsm_pt_wfe(pt, VSFSM_EVT_IO_DONE);
						if (param->transfer_ack != DAP_TRANSFER_OK)
							break;
						SET_LE_U32(response + param->resp_ptr, param->transfer_data);
						param->resp_ptr += 4;
					}
					
					if (param->transfer_req & DAP_TRANSFER_MATCH_VALUE)
					{
						if (param->transfer_req & DAP_TRANSFER_APnDP)
						{
							// Post AP read
							err = vsfhal_swd_transact(param->transfer_req, 0);
							if (err != VSFERR_NONE) break;
							vsfsm_pt_wfe(pt, VSFSM_EVT_IO_DONE);
							if (param->transfer_ack != DAP_TRANSFER_OK)
								break;
						}
						
						param->transfer_retry = param->transfer.match_retry;
						do
						{
							// Read register until its value matches or retry counter expires
							err = vsfhal_swd_transact(param->transfer_req, 0);
							if (err != VSFERR_NONE) break;
							vsfsm_pt_wfe(pt, VSFSM_EVT_IO_DONE);
							if (param->transfer_ack != DAP_TRANSFER_OK)
								break;
						} while (((param->transfer_data & param->transfer.match_mask) != GET_LE_U32(request + param->req_ptr)) &&
								param->transfer_retry-- && !param->abort);
						if ((param->transfer_data & param->transfer.match_mask) != GET_LE_U32(request + param->req_ptr))
							param->transfer_ack |= DAP_TRANSFER_MISMATCH;
						param->req_ptr += 4;
						if (param->transfer_ack != DAP_TRANSFER_OK)
							break;
					}
					else	// Normal read
					{
						if (param->transfer_req & DAP_TRANSFER_APnDP)	// Read AP register
						{
							if (!param->post_read)	// Post AP read
							{
								err = vsfhal_swd_transact(param->transfer_req, 0);
								if (err != VSFERR_NONE) break;
								vsfsm_pt_wfe(pt, VSFSM_EVT_IO_DONE);
								if (param->transfer_ack != DAP_TRANSFER_OK)
									break;
								param->post_read = true;
							}
						}
						else	// Read DP register
						{
							err = vsfhal_swd_transact(param->transfer_req, 0);
							if (err != VSFERR_NONE) break;
							vsfsm_pt_wfe(pt, VSFSM_EVT_IO_DONE);
							if (param->transfer_ack != DAP_TRANSFER_OK)
								break;
							SET_LE_U32(response + param->resp_ptr, param->transfer_data);
							param->resp_ptr += 4;
						}
					}
					param->check_write = false;
				}
				else	// Write register
				{
					if (param->post_read)
					{
						err = vsfhal_swd_transact(DP_RDBUFF | DAP_TRANSFER_RnW, 0);
						if (err != VSFERR_NONE) break;
						vsfsm_pt_wfe(pt, VSFSM_EVT_IO_DONE);
						if (param->transfer_ack != DAP_TRANSFER_OK)
							break;
						SET_LE_U32(response + param->resp_ptr, param->transfer_data);
						param->resp_ptr += 4;
						param->post_read = false;
					}
					
					data = GET_LE_U32(request + param->req_ptr);
					param->req_ptr += 4;
					
					if (param->transfer_req & DAP_TRANSFER_MATCH_MASK)
					{
						// Write match mask
						param->transfer.match_mask = data;
						param->transfer_ack = DAP_TRANSFER_OK;
					}
					else
					{
						// Write DP/AP register
						err = vsfhal_swd_transact(param->transfer_req, data);
						if (err != VSFERR_NONE) break;
						vsfsm_pt_wfe(pt, VSFSM_EVT_IO_DONE);
						if (param->transfer_ack != DAP_TRANSFER_OK)
							break;
						param->check_write = true;
					}
				}

				if (param->abort)
					break;
			}
			
			if (param->transfer_cnt < param->transfer_num)
			{
				uint16_t i = 0;				
				while ((param->transfer_cnt + i) < param->transfer_num)
				{
					data = request[param->req_ptr++];
					if (data & DAP_TRANSFER_RnW)
					{
						if (data & DAP_TRANSFER_MATCH_VALUE)	// Read with value match
							param->req_ptr += 4;
					}
					else	// Write register
						param->req_ptr += 4;
					i++;
				}
			}
			
			if (param->transfer_ack == DAP_TRANSFER_OK)
			{
				if (param->post_read)
				{
					err = vsfhal_swd_transact(DP_RDBUFF | DAP_TRANSFER_RnW, 0);
					if (err != VSFERR_NONE) break;
					vsfsm_pt_wfe(pt, VSFSM_EVT_IO_DONE);
					if (param->transfer_ack == DAP_TRANSFER_OK)
					{
						SET_LE_U32(response + param->resp_ptr, param->transfer_data);
						param->resp_ptr += 4;
					}
				}
				else if (param->check_write)
				{
					err = vsfhal_swd_transact(DP_RDBUFF | DAP_TRANSFER_RnW, 0);
					if (err != VSFERR_NONE) break;
					vsfsm_pt_wfe(pt, VSFSM_EVT_IO_DONE);
				}
			}
			
			response[param->resp_start++] = param->transfer_cnt;
			response[param->resp_start++] = param->transfer_ack;
		}
		else if (param->port == DAP_PORT_JTAG)
		{
			if (request[param->req_ptr] >= DAP_JTAG_DEV_CNT)
				goto DAP_Transfer_EXIT;
			
			param->jtag_dev.index = request[param->req_ptr];			
			param->resp_start = param->resp_ptr;
			param->transfer_cnt = 0;
			param->transfer_num = request[param->req_ptr];
			param->abort = false;
			param->post_read = false;
			param->jtag_ir = 0;
			param->req_ptr += 2;
			param->resp_ptr += 2;
			
			while (param->transfer_cnt < param->transfer_num)
			{
				param->transfer_req = request[param->req_ptr++];
				param->request_ir = (param->transfer_req & DAP_TRANSFER_APnDP) ? JTAG_APACC : JTAG_DPACC;
				
				if (param->transfer_req & DAP_TRANSFER_RnW)	// Read register
				{
					if (param->post_read)
					{
						if ((param->jtag_ir == param->request_ir) &&
								((param->transfer_req & DAP_TRANSFER_MATCH_VALUE) == 0))
						{
							// Read previous data and post next read
							err = vsfhal_jtag_dr(param->transfer_req,
									param->transfer_data, param->jtag_dev.index,
									param->jtag_dev.count - param->jtag_dev.index - 1);
							if (err != VSFERR_NONE) break;
							vsfsm_pt_wfe(pt, VSFSM_EVT_IO_DONE);
							if (param->transfer_ack != DAP_TRANSFER_OK)
								break;
						}
						else
						{
							// Select JTAG chain
							if (param->jtag_ir != JTAG_DPACC)
							{
								param->jtag_ir = JTAG_DPACC;
								err = vsfhal_jtag_ir(JTAG_DPACC, param->jtag_dev.ir_length[param->jtag_dev.index],
										param->jtag_dev.ir_before[param->jtag_dev.index],
										param->jtag_dev.ir_after[param->jtag_dev.index]);
								if (err != VSFERR_NONE) break;
								vsfsm_pt_wfe(pt, VSFSM_EVT_IO_DONE);
								if (param->transfer_ack != DAP_TRANSFER_OK)
									break;
							}
							// Read previous data
							err = vsfhal_jtag_dr(DP_RDBUFF | DAP_TRANSFER_RnW,
									param->transfer_data, param->jtag_dev.index,
									param->jtag_dev.count - param->jtag_dev.index - 1);
							if (err != VSFERR_NONE) break;
							vsfsm_pt_wfe(pt, VSFSM_EVT_IO_DONE);
							if (param->transfer_ack != DAP_TRANSFER_OK)
								break;
							param->post_read = 0;
						}
						SET_LE_U32(response + param->resp_ptr, param->transfer_data);
						param->resp_ptr += 4;
					}
					if (param->transfer_req & DAP_TRANSFER_MATCH_VALUE)
					{
						// Select JTAG chain
						if (param->jtag_ir != param->request_ir)
						{
							param->jtag_ir = param->request_ir;
							err = vsfhal_jtag_ir(param->jtag_ir, param->jtag_dev.ir_length[param->jtag_dev.index],
									param->jtag_dev.ir_before[param->jtag_dev.index],
									param->jtag_dev.ir_after[param->jtag_dev.index]);
							if (err != VSFERR_NONE) break;
							vsfsm_pt_wfe(pt, VSFSM_EVT_IO_DONE);
							if (param->transfer_ack != DAP_TRANSFER_OK)
								break;
						}
						
						// Post DP/AP read
						err = vsfhal_jtag_dr(param->transfer_req,
								param->transfer_data, param->jtag_dev.index,
								param->jtag_dev.count - param->jtag_dev.index - 1);
						if (err != VSFERR_NONE) break;
						vsfsm_pt_wfe(pt, VSFSM_EVT_IO_DONE);
						if (param->transfer_ack != DAP_TRANSFER_OK)
							break;
						
						param->transfer_retry = param->transfer.match_retry;
						do
						{
							err = vsfhal_jtag_dr(param->transfer_req,
									param->transfer_data, param->jtag_dev.index,
									param->jtag_dev.count - param->jtag_dev.index - 1);
							if (err != VSFERR_NONE) break;
							vsfsm_pt_wfe(pt, VSFSM_EVT_IO_DONE);
							if (param->transfer_ack != DAP_TRANSFER_OK)
								break;
						} while (((param->transfer_data & param->transfer.match_mask) != GET_LE_U32(request + param->req_ptr)) &&
								param->transfer_retry-- && !param->abort);
						if ((param->transfer_data & param->transfer.match_mask) != GET_LE_U32(request + param->req_ptr))
							param->transfer_ack |= DAP_TRANSFER_MISMATCH;
						param->req_ptr += 4;
						if (param->transfer_ack != DAP_TRANSFER_OK)
							break;
					}
					else	// Normal read
					{
						if (!param->post_read)
						{
							// Select JTAG chain
							if (param->jtag_ir != param->request_ir)
							{
								param->jtag_ir = param->request_ir;
								err = vsfhal_jtag_ir(param->jtag_ir, param->jtag_dev.ir_length[param->jtag_dev.index],
										param->jtag_dev.ir_before[param->jtag_dev.index],
										param->jtag_dev.ir_after[param->jtag_dev.index]);
								if (err != VSFERR_NONE) break;
								vsfsm_pt_wfe(pt, VSFSM_EVT_IO_DONE);
								if (param->transfer_ack != DAP_TRANSFER_OK)
									break;
							}
							
							// Post DP/AP read
							err = vsfhal_jtag_dr(param->transfer_req,
									param->transfer_data, param->jtag_dev.index,
									param->jtag_dev.count - param->jtag_dev.index - 1);
							if (err != VSFERR_NONE) break;
							vsfsm_pt_wfe(pt, VSFSM_EVT_IO_DONE);
							if (param->transfer_ack != DAP_TRANSFER_OK)
								break;
							param->post_read = 1;
						}
					}
				}
				else	// Write register
				{
					if (param->post_read)
					{
						// Select JTAG chain
						if (param->jtag_ir != JTAG_DPACC)
						{
							param->jtag_ir = JTAG_DPACC;
							err = vsfhal_jtag_ir(param->jtag_ir, param->jtag_dev.ir_length[param->jtag_dev.index],
									param->jtag_dev.ir_before[param->jtag_dev.index],
									param->jtag_dev.ir_after[param->jtag_dev.index]);
							if (err != VSFERR_NONE) break;
							vsfsm_pt_wfe(pt, VSFSM_EVT_IO_DONE);
							if (param->transfer_ack != DAP_TRANSFER_OK)
								break;
						}
						// Read previous data
						err = vsfhal_jtag_dr(DP_RDBUFF | DAP_TRANSFER_RnW,
								param->transfer_data, param->jtag_dev.index,
								param->jtag_dev.count - param->jtag_dev.index - 1);
						if (err != VSFERR_NONE) break;
						vsfsm_pt_wfe(pt, VSFSM_EVT_IO_DONE);
						if (param->transfer_ack != DAP_TRANSFER_OK)
							break;
						SET_LE_U32(response + param->resp_ptr, param->transfer_data);
						param->resp_ptr += 4;
						param->post_read = 0;
					}

					if (param->transfer_req & DAP_TRANSFER_MATCH_MASK)	// Write match mask
					{
						param->transfer.match_mask = GET_LE_U32(request + param->req_ptr);
						param->req_ptr += 4;
						param->transfer_ack = DAP_TRANSFER_OK;
					}
					else
					{
						// Select JTAG chain
						if (param->jtag_ir != param->request_ir)
						{
							param->jtag_ir = param->request_ir;
							err = vsfhal_jtag_ir(param->jtag_ir, param->jtag_dev.ir_length[param->jtag_dev.index],
									param->jtag_dev.ir_before[param->jtag_dev.index],
									param->jtag_dev.ir_after[param->jtag_dev.index]);
							if (err != VSFERR_NONE) break;
							vsfsm_pt_wfe(pt, VSFSM_EVT_IO_DONE);
							if (param->transfer_ack != DAP_TRANSFER_OK)
								break;
						}
						// Write DP/AP register
						err = vsfhal_jtag_dr(param->transfer_req,
								GET_LE_U32(request + param->req_ptr), param->jtag_dev.index,
								param->jtag_dev.count - param->jtag_dev.index - 1);
						param->req_ptr += 4;
						if (err != VSFERR_NONE) break;
						vsfsm_pt_wfe(pt, VSFSM_EVT_IO_DONE);
						if (param->transfer_ack != DAP_TRANSFER_OK)
							break;
					}
				}

				param->transfer_cnt++;				
				if (param->abort)
					break;
			}

			if (param->transfer_cnt < param->transfer_num)
			{
				uint16_t i = 0;	
				while ((param->transfer_cnt + i) < param->transfer_num)
				{
					data = request[param->req_ptr++];
					if (data & DAP_TRANSFER_RnW)
					{
						if (data & DAP_TRANSFER_MATCH_VALUE)	// Read with value match
							param->req_ptr += 4;
					}
					else	// Write register
						param->req_ptr += 4;
					i++;
				}
			}
			
			if (param->transfer_ack == DAP_TRANSFER_OK)
			{
				if (param->jtag_ir != JTAG_DPACC)
				{
					param->jtag_ir = JTAG_DPACC;
					err = vsfhal_jtag_ir(param->jtag_ir, param->jtag_dev.ir_length[param->jtag_dev.index],
							param->jtag_dev.ir_before[param->jtag_dev.index],
							param->jtag_dev.ir_after[param->jtag_dev.index]);
					if (err == VSFERR_NONE)
						vsfsm_pt_wfe(pt, VSFSM_EVT_IO_DONE);
				}

				err = vsfhal_jtag_dr(DP_RDBUFF | DAP_TRANSFER_RnW,
						param->transfer_data, param->jtag_dev.index,
						param->jtag_dev.count - param->jtag_dev.index - 1);
				if (err == VSFERR_NONE)
					vsfsm_pt_wfe(pt, VSFSM_EVT_IO_DONE);

				if (param->post_read && (param->transfer_ack == DAP_TRANSFER_OK))
				{
					SET_LE_U32(response + param->resp_ptr, param->transfer_data);
					param->resp_ptr += 4;
				}
			}
			
			response[param->resp_start++] = param->transfer_cnt;
			response[param->resp_start++] = param->transfer_ack;
		}
		else
		{
			uint32_t request_value, request_count;

DAP_Transfer_EXIT:
			request_count = request[param->req_ptr + 1];
			param->req_ptr += 2;
			while (request_count)
			{
				request_count--;
				request_value = request[param->req_ptr++];
				if (request_value & DAP_TRANSFER_RnW)
				{
					if (request_value & DAP_TRANSFER_MATCH_VALUE)
						param->req_ptr += 4;
				}
				else
					param->req_ptr += 4;
			}
			response[param->resp_ptr++] = 0;	// Response count
			response[param->resp_ptr++] = 0;	// Response value
		}
	}
	else if (param->cmd_id == ID_DAP_TransferBlock)
	{
		if (param->port == DAP_PORT_SWD)
		{
			param->resp_start = param->resp_ptr;
			param->transfer_cnt = 0;
			param->transfer_num = GET_LE_U16(request + param->req_ptr + 1);
			param->abort = false;
			param->transfer_ack = 0;
			param->req_ptr += 3;
			param->resp_ptr += 3;
			
			if (!param->transfer_num)
				goto DAP_TransferBlock_SWD_END;

			param->transfer_req = request[param->req_ptr++];
			if (param->transfer_req & DAP_TRANSFER_RnW)	// Read register block
			{
				if (param->transfer_req & DAP_TRANSFER_APnDP)	// Post AP read
				{
					err = vsfhal_swd_transact(param->transfer_req, 0);
					if (err != VSFERR_NONE) goto DAP_TransferBlock_SWD_END;
					vsfsm_pt_wfe(pt, VSFSM_EVT_IO_DONE);
					if (param->transfer_ack != DAP_TRANSFER_OK)
						goto DAP_TransferBlock_SWD_END;
				}
				
				while (param->transfer_cnt < param->transfer_num)	// Read DP/AP register
				{
					if ((param->transfer_cnt == (param->transfer_num - 1)) &&
							(param->transfer_req & DAP_TRANSFER_APnDP))
						param->transfer_req = DP_RDBUFF | DAP_TRANSFER_RnW;
					
					err = vsfhal_swd_transact(param->transfer_req, 0);
					if (err != VSFERR_NONE) goto DAP_TransferBlock_SWD_END;
					vsfsm_pt_wfe(pt, VSFSM_EVT_IO_DONE);
					if (param->transfer_ack != DAP_TRANSFER_OK)
						goto DAP_TransferBlock_SWD_END;
					SET_LE_U32(response + param->resp_ptr, param->transfer_data);
					param->resp_ptr += 4;
					param->transfer_cnt++;
				}
			}
			else	// Write register block
			{
				while (param->transfer_cnt < param->transfer_num)	// Write register block
				{
					err = vsfhal_swd_transact(param->transfer_req, GET_LE_U32(request + param->req_ptr));
					param->req_ptr += 4;
					if (err != VSFERR_NONE) goto DAP_TransferBlock_SWD_END;
					vsfsm_pt_wfe(pt, VSFSM_EVT_IO_DONE);
					if (param->transfer_ack != DAP_TRANSFER_OK)
						goto DAP_TransferBlock_SWD_END;
					param->transfer_cnt++;
				}

				// Check last write
				err = vsfhal_swd_transact(DP_RDBUFF | DAP_TRANSFER_RnW, 0);
				if (err != VSFERR_NONE) goto DAP_TransferBlock_SWD_END;
				vsfsm_pt_wfe(pt, VSFSM_EVT_IO_DONE);
			}
			
		DAP_TransferBlock_SWD_END:
			SET_LE_U16(response + param->resp_start, param->transfer_cnt);
			response[param->resp_start + 2] = param->transfer_ack;
		}
		else if (param->port == DAP_PORT_JTAG)
		{
			if (request[param->req_ptr] >= DAP_JTAG_DEV_CNT)
				goto DAP_TransferBlock_EXIT;
			
			param->jtag_dev.index = request[param->req_ptr];			
			param->resp_start = param->resp_ptr;
			param->transfer_cnt = 0;
			param->transfer_num = GET_LE_U16(request + param->req_ptr + 1);
			param->abort = false;
			param->transfer_ack = 0;
			param->req_ptr += 3;
			param->resp_ptr += 3;
			
			param->transfer_req = request[param->req_ptr++];
			
			// Select JTAG chain
			param->jtag_ir = (param->transfer_req & DAP_TRANSFER_APnDP) ? JTAG_APACC : JTAG_DPACC;
			err = vsfhal_jtag_ir(param->jtag_ir, param->jtag_dev.ir_length[param->jtag_dev.index],
					param->jtag_dev.ir_before[param->jtag_dev.index],
					param->jtag_dev.ir_after[param->jtag_dev.index]);
			if (err != VSFERR_NONE) goto DAP_TransferBlock_ERROR;
			vsfsm_pt_wfe(pt, VSFSM_EVT_IO_DONE);
			if (param->transfer_ack != DAP_TRANSFER_OK)
				goto DAP_TransferBlock_ERROR;
			
			if (param->transfer_req & DAP_TRANSFER_RnW)
			{
				// Post read
				err = vsfhal_jtag_dr(param->transfer_req,
						param->transfer_data, param->jtag_dev.index,
						param->jtag_dev.count - param->jtag_dev.index - 1);
				if (err != VSFERR_NONE) goto DAP_TransferBlock_ERROR;
				vsfsm_pt_wfe(pt, VSFSM_EVT_IO_DONE);
				if (param->transfer_ack != DAP_TRANSFER_OK)
					goto DAP_TransferBlock_ERROR;
				
				// Read register block
				while (param->transfer_cnt < param->transfer_num)
				{
					if (param->transfer_cnt == (param->transfer_num - 1))	// Last read
					{
						if (param->jtag_ir != JTAG_DPACC)
						{
							param->jtag_ir = JTAG_DPACC;
							err = vsfhal_jtag_ir(param->jtag_ir, param->jtag_dev.ir_length[param->jtag_dev.index],
									param->jtag_dev.ir_before[param->jtag_dev.index],
									param->jtag_dev.ir_after[param->jtag_dev.index]);
							if (err != VSFERR_NONE) goto DAP_TransferBlock_ERROR;
							vsfsm_pt_wfe(pt, VSFSM_EVT_IO_DONE);
							if (param->transfer_ack != DAP_TRANSFER_OK)
								goto DAP_TransferBlock_ERROR;
						}
						param->transfer_req = DP_RDBUFF | DAP_TRANSFER_RnW;
					}
					
					err = vsfhal_jtag_dr(param->transfer_req,
							param->transfer_data, param->jtag_dev.index,
							param->jtag_dev.count - param->jtag_dev.index - 1);
					if (err != VSFERR_NONE) goto DAP_TransferBlock_ERROR;
					vsfsm_pt_wfe(pt, VSFSM_EVT_IO_DONE);
					if (param->transfer_ack != DAP_TRANSFER_OK)
						goto DAP_TransferBlock_ERROR;
					SET_LE_U32(response + param->resp_ptr, param->transfer_data);
					param->resp_ptr += 4;
					param->transfer_cnt++;
				}
			}
			else	// Write register block
			{
				while (param->transfer_cnt < param->transfer_num)
				{
					// Write DP/AP register
					err = vsfhal_jtag_dr(param->transfer_req,
							GET_LE_U32(request + param->req_ptr), param->jtag_dev.index,
							param->jtag_dev.count - param->jtag_dev.index - 1);
					param->req_ptr += 4;
					if (err != VSFERR_NONE) goto DAP_TransferBlock_ERROR;
					vsfsm_pt_wfe(pt, VSFSM_EVT_IO_DONE);
					if (param->transfer_ack != DAP_TRANSFER_OK)
						goto DAP_TransferBlock_ERROR;
					param->transfer_cnt++;
				}
				
				// Check last write
				if (param->jtag_ir != JTAG_DPACC)
				{
					param->jtag_ir = JTAG_DPACC;
					err = vsfhal_jtag_ir(param->jtag_ir, param->jtag_dev.ir_length[param->jtag_dev.index],
							param->jtag_dev.ir_before[param->jtag_dev.index],
							param->jtag_dev.ir_after[param->jtag_dev.index]);
					if (err != VSFERR_NONE) goto DAP_TransferBlock_ERROR;
					vsfsm_pt_wfe(pt, VSFSM_EVT_IO_DONE);
					if (param->transfer_ack != DAP_TRANSFER_OK)
						goto DAP_TransferBlock_ERROR;
				}
				
				err = vsfhal_jtag_dr(DP_RDBUFF | DAP_TRANSFER_RnW,
						param->transfer_data, param->jtag_dev.index,
						param->jtag_dev.count - param->jtag_dev.index - 1);
				if (err != VSFERR_NONE) goto DAP_TransferBlock_ERROR;
				vsfsm_pt_wfe(pt, VSFSM_EVT_IO_DONE);
				if (param->transfer_ack != DAP_TRANSFER_OK)
					goto DAP_TransferBlock_ERROR;
			}

			SET_LE_U16(response + param->resp_start, param->transfer_cnt);
			response[param->resp_start + 2] = param->transfer_ack;
		}
		else
		{
DAP_TransferBlock_EXIT:
			if (request[param->req_ptr + 3] & DAP_TRANSFER_RnW)
				param->req_ptr += 4;
			else
				param->req_ptr += 4 + 4 * GET_LE_U16(request + param->req_ptr + 1);
DAP_TransferBlock_ERROR:
			response[param->resp_ptr++] = 0;	// Response count [7:0]
			response[param->resp_ptr++] = 0;	// Response count [15:8]
			response[param->resp_ptr++] = 0;	// Response value
		}
	}
	else if (param->cmd_id == ID_DAP_WriteABORT)
	{
		if (param->port == DAP_PORT_SWD)
		{
			// Ignore DAP index
			data = GET_LE_U32(request + param->req_ptr + 1);
			param->req_ptr += 5;
			err = vsfhal_swd_transact(DP_ABORT, data);
			if (err == VSFERR_NONE)
			{
				vsfsm_pt_wfe(pt, VSFSM_EVT_IO_DONE);
				response[param->resp_ptr++] = DAP_OK;
			}
			else
				response[param->resp_ptr++] = DAP_ERROR;
		}
		else if (param->port == DAP_PORT_JTAG)
		{
			if (request[param->req_ptr] >= DAP_JTAG_DEV_CNT)
				goto DAP_WriteABORT_EXIT;
			
			param->jtag_dev.index = request[param->req_ptr++];

			err = vsfhal_jtag_ir(JTAG_ABORT, param->jtag_dev.ir_length[param->jtag_dev.index],
					param->jtag_dev.ir_before[param->jtag_dev.index],
					param->jtag_dev.ir_after[param->jtag_dev.index]);
			if (err != VSFERR_NONE) goto DAP_WriteABORT_ERROR;
			vsfsm_pt_wfe(pt, VSFSM_EVT_IO_DONE);
			if (param->transfer_ack != DAP_TRANSFER_OK)
				goto DAP_WriteABORT_ERROR;
			
			// Write Abort register
			err = vsfhal_jtag_dr(0,
					GET_LE_U32(request + param->req_ptr), param->jtag_dev.index,
					param->jtag_dev.count - param->jtag_dev.index - 1);
			param->req_ptr += 4;
			if (err == VSFERR_NONE) goto DAP_WriteABORT_ERROR;
			vsfsm_pt_wfe(pt, VSFSM_EVT_IO_DONE);
			if (param->transfer_ack != DAP_TRANSFER_OK)
				goto DAP_WriteABORT_ERROR;
			
			response[param->resp_ptr++] = DAP_OK;
		}
		else
		{
DAP_WriteABORT_EXIT:
			param->req_ptr += 5;
DAP_WriteABORT_ERROR:
			response[param->resp_ptr++] = DAP_ERROR;
		}
	}
#if ((SWO_UART != 0) || (SWO_MANCHESTER != 0))
	else if (param->cmd_id == ID_DAP_SWO_Transport)
	{
		uint8_t ret = DAP_ERROR;
		uint8_t transport = request[param->req_ptr++];
		if (!(param->trace_status & DAP_SWO_CAPTURE_ACTIVE))
		{
			if (transport <= 1)
			{
				param->transport = transport;
				ret = DAP_OK;
			}
		}
		response[param->resp_ptr++] = ret;
	}
	else if (param->cmd_id == ID_DAP_SWO_Mode)
	{
		uint8_t mode = request[param->req_ptr++];
		if ((mode == DAP_SWO_OFF) || (mode == DAP_SWO_UART))
		{
			param->trace_mode = mode;
			response[param->resp_ptr++] = DAP_OK;
		}
		else
		{
			param->trace_mode = DAP_SWO_OFF;
			response[param->resp_ptr++] = DAP_ERROR;
		}
	}
	else if (param->cmd_id == ID_DAP_SWO_Baudrate)
	{
		uint32_t baudrate = GET_LE_U32(request + param->req_ptr);
		param->req_ptr += 4;
		if (baudrate > SWO_UART_MAX_BAUDRATE)
			baudrate = SWO_UART_MAX_BAUDRATE;
		else if (baudrate < SWO_UART_MIN_BAUDRATE)
			baudrate = SWO_UART_MIN_BAUDRATE;
#if (SWO_UART != 0)
		usrapp_update_swo_usart_param(NULL, &baudrate);
#endif
		SET_LE_U32(response + param->resp_ptr, baudrate);
		param->resp_ptr += 4;
	}
	else if (param->cmd_id == ID_DAP_SWO_Control)
	{
		uint8_t active = request[param->req_ptr++] & DAP_SWO_CAPTURE_ACTIVE;
		if (active != (param->trace_status & DAP_SWO_CAPTURE_ACTIVE))
		{
			if (active)
				VSFSTREAM_INIT((struct vsf_stream_t *)&param->swo_rx);
			param->trace_status = active;
		}
		response[param->resp_ptr++] = DAP_OK;
	}
	else if (param->cmd_id == ID_DAP_SWO_Status)
	{
		uint32_t count = VSFSTREAM_GET_DATA_SIZE((struct vsf_stream_t *)&param->swo_rx);		
		response[param->resp_ptr++] = param->trace_status;
		SET_LE_U32(response + param->resp_ptr, count);
		param->resp_ptr += 4;
	}
	else if (param->cmd_id == ID_DAP_SWO_Data)
	{
		uint16_t count = min(GET_LE_U16(request + param->req_ptr),
				VSFSTREAM_GET_DATA_SIZE((struct vsf_stream_t *)&param->swo_rx));
		param->req_ptr += 2;
		response[param->resp_ptr++] = param->trace_status;
		if (param->transport != 1)
			count = 0;
		if (count > (DAP_PACKET_SIZE - 2 - param->resp_ptr))
			count = DAP_PACKET_SIZE - 2 - param->resp_ptr;
		if (count)
		{
			struct vsf_buffer_t buffer;			
			buffer.buffer = response + param->resp_ptr + 2;
			buffer.size = count;
			count = VSFSTREAM_READ((struct vsf_stream_t *)&param->swo_rx, &buffer);
		}
		SET_LE_U16(response + param->resp_ptr, count);
		param->resp_ptr += 2 + count;
		
		if (param->trace_status == (DAP_SWO_CAPTURE_ACTIVE | DAP_SWO_CAPTURE_PAUSED))
		{
			if (VSFSTREAM_GET_FREE_SIZE((struct vsf_stream_t *)&param->swo_rx))
				param->trace_status = DAP_SWO_CAPTURE_ACTIVE;
		}
	}
#endif
	else
	{
		goto fault;
	}

	vsfsm_pt_end(pt);
	return VSFERR_NONE;

fault:
	param->response[param->resp_ptr - 1] = ID_DAP_Invalid;
	return VSFERR_NONE;
}

static vsf_err_t dap_thread(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct dap_param_t *param = pt->user_data;

	vsfsm_pt_begin(pt);
	
	while (1)
	{
		if (vsfsm_sem_pend(&param->request_sem, pt->sm))
			vsfsm_pt_wfe(pt, param->request_sem.evt);
		
		param->req_ptr = 0;
		param->resp_ptr = 0;
		param->cmd_num = 1;
		//memset(param->response, 0, DAP_PACKET_SIZE);
		
		while ((param->cmd_num) && (param->resp_ptr < DAP_PACKET_SIZE))
			vsfsm_pt_wfpt(pt, &param->cmd_pt);
		
		param->request_head++;
		if (param->request_head == DAP_PACKET_COUNT)
			param->request_head = 0;
		param->request_cnt--;		
		
		if (vsfsm_sem_pend(&param->response_sem, pt->sm))
			vsfsm_pt_wfe(pt, param->response_sem.evt);
		
		if (param->cb_response)
			param->cb_response(param->cb_param, param->response, param->resp_ptr);
	}

	vsfsm_pt_end(pt);

	return VSFERR_NONE;
}

static struct vsfsm_state_t *sem_evt_handler(struct vsfsm_t *sm, vsfsm_evt_t evt)
{
	struct dap_param_t *param = sm->user_data;

	switch (evt)
	{
	case VSFSM_EVT_ON_REVICE:
		vsfsm_sem_post(&param->request_sem);
		break;
	case VSFSM_EVT_ON_SEND_DONE:
		vsfsm_sem_post(&param->response_sem);
		break;
	}

	return NULL;
}

vsf_err_t DAP_init(struct dap_param_t *param)
{
	param->port = DAP_PORT_DISABLED;
	param->khz = DAP_DEFAULT_SWJ_CLOCK / 1000;
	param->transfer.idle_cycles = 0;
	param->transfer.retry_count = 100;
	param->transfer.match_retry = 0;
	param->transfer.match_mask = 0;
#if DAP_SWD
	param->swd_conf.turnaround = 1;
	param->swd_conf.data_phase = 0;
#endif
#if SWO_UART
	param->swo_rx.stream.op = &vsf_fifostream_op;
	param->swo_rx.mem.buffer.buffer = (uint8_t *)param->swo_buff;
	param->swo_rx.mem.buffer.size = sizeof(param->swo_buff);
#endif
	
	param->sem_sm.init_state.evt_handler = sem_evt_handler;
	param->sem_sm.user_data = param;
	vsfsm_init(&param->sem_sm);
	
	vsfsm_sem_init(&param->request_sem, 0, VSFSM_EVT_DAP_REQUEST);
	vsfsm_sem_init(&param->response_sem, 1, VSFSM_EVT_DAP_RESPONSE);

	param->cmd_pt.thread = cmd_thread;
	param->cmd_pt.user_data = param;
	param->cmd_pt.state = 0;
	param->cmd_pt.sm = &param->sm;
	
	param->pt.thread = dap_thread;
	param->pt.user_data = param;
	return vsfsm_pt_init(&param->sm, &param->pt);	
}

vsf_err_t DAP_recvive_request(struct dap_param_t *param, uint8_t *buf, uint16_t size)
{
	if (param->request_cnt < DAP_PACKET_COUNT)
	{
		size = min(size, DAP_PACKET_SIZE);
		if (size)
		{
			uint8_t store = (param->request_head + param->request_cnt) % DAP_PACKET_COUNT;
			memcpy(param->request[store], buf, DAP_PACKET_SIZE);
			param->request_cnt++;
			vsfsm_post_evt_pending(&param->sem_sm, VSFSM_EVT_ON_REVICE);
			return VSFERR_NONE;
		}
		else
			return VSFERR_FAIL;
	}
	else
		return VSFERR_NOT_ENOUGH_RESOURCES;
}

vsf_err_t DAP_register(struct dap_param_t *param, void *cb_param,
		void (*cb_response)(void *, uint8_t *, uint16_t))
{
	if (!param->cb_response)
	{
		param->cb_param = cb_param;
		param->cb_response = cb_response;
		return VSFERR_NONE;
	}
	else
		return VSFERR_FAIL;
}

void DAP_unregister(struct dap_param_t *param, void *cb_param,
		void (*cb_response)(void *, uint8_t *, uint16_t))
{
	if (param->cb_response == cb_response)
	{
		param->cb_param = NULL;
		param->cb_response = NULL;
	}
}

void DAP_send_response_done(struct dap_param_t *param)
{
	vsfsm_post_evt_pending(&param->sem_sm, VSFSM_EVT_ON_SEND_DONE);
}

#endif
