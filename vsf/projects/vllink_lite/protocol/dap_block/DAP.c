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
#include "DAP.h"

#define DAP_FW_VER      "0254"  // Firmware Version

#ifdef DAP_VENDOR
const char DAP_Vendor[] = DAP_VENDOR;
#endif
#ifdef DAP_PRODUCT
const char DAP_Product[] = DAP_PRODUCT;
#endif
const char DAP_FW_Ver[] = DAP_FW_VER;

//#define VSFSM_EVT_IO_DONE				(VSFSM_EVT_USER_LOCAL + 1)
#define VSFSM_EVT_DAP_REQUEST			(VSFSM_EVT_USER_LOCAL + 2)
#define VSFSM_EVT_DAP_RESPONSE			(VSFSM_EVT_USER_LOCAL + 3)
#define VSFSM_EVT_ON_REVICE				(VSFSM_EVT_USER_LOCAL + 4)
#define VSFSM_EVT_ON_SEND_DONE			(VSFSM_EVT_USER_LOCAL + 5)

#if PROJ_CFG_DAP_STANDARD_ENABLE
static uint8_t get_dap_info(struct dap_param_t *param, uint8_t id, uint8_t *info)
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
		if (param->get_serial)
			length = param->get_serial(info);
		else
			length = 0;
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
			SET_LE_U16(info, param->pkt_size);
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

static vsf_err_t port_init(struct dap_param_t *param, uint8_t port)
{
	vsf_err_t err = VSFERR_NONE;

	if (port == DAP_PORT_SWD)
	{
		vsfhal_swd_init(PERIPHERAL_SWD_PRIORITY);
		vsfhal_swd_config(param->khz, param->transfer.idle_cycles, 
				param->swd_conf.turnaround, param->swd_conf.data_phase,
				param->transfer.retry_count);
	}
	else if (port == DAP_PORT_JTAG)
	{
		vsfhal_jtag_init(PERIPHERAL_JTAG_PRIORITY);
		vsfhal_jtag_config(param->khz, param->transfer.retry_count,
				param->transfer.idle_cycles);
	}
	return err;
}

static vsf_err_t port_fini(uint8_t port)
{
	if (port == DAP_PORT_SWD)
	{
		vsfhal_swd_fini();
	}
	else if (port == DAP_PORT_JTAG)
	{
		vsfhal_jtag_fini();
	}
	return VSFERR_NONE;
}
#endif	// PROJ_CFG_DAP_STANDARD_ENABLE

static uint16_t cmd_handler(struct dap_param_t *param, uint8_t *request, uint8_t *response)
{
	uint8_t cmd_id, cmd_num;
	uint16_t req_ptr, resp_ptr;
#if PROJ_CFG_DAP_STANDARD_ENABLE
	uint16_t transfer_cnt, transfer_num;
	uint32_t data;
	uint64_t buf_tms, buf_tdi, buf_tdo;
#endif

	req_ptr = 0;
	resp_ptr = 0;
	cmd_num = 1;
	
	do
	{
		cmd_num--;
		cmd_id = request[req_ptr++];
		response[resp_ptr++] = cmd_id;

		if ((cmd_id == ID_DAP_ExecuteCommands) && (resp_ptr <= (param->pkt_size - 2)))
		{
			cmd_num = request[req_ptr++];
			response[resp_ptr++] = cmd_num;
		}
		else if ((cmd_id >= ID_DAP_Vendor0) && (cmd_id <= ID_DAP_Vendor31))
		{
			switch (cmd_id)
			{
			case ID_DAP_Vendor0:
				if (param->get_serial && (resp_ptr <= (param->pkt_size - 1 - param->get_serial(NULL))))
				{
					uint8_t num = param->get_serial(&response[resp_ptr + 1]);
					response[resp_ptr] = num;
					resp_ptr += num + 1;
				}
				else
					response[resp_ptr++] = 0;
				break;
#if PROJ_CFG_DAP_VERDOR_UART_ENABLE
			case ID_DAP_Vendor1:
				/*
					Get uart line coding CMD:
					Request:
					CMD [1 byte]
					ID_DAP_Vendor1
					Response:
					CMD [1 byte]		uart (default 0) line coding [7 byte]
					ID_DAP_Vendor1		struct usb_CDCACM_line_coding_t
				*/
				break;
			case ID_DAP_Vendor2:
				/*
					Set uart line coding CMD:
					Request:
					CMD [1 byte]		uart (default 0) line coding [7 byte]
					ID_DAP_Vendor2		struct usb_CDCACM_line_coding_t
					Response:
					CMD [1 byte]		const [1 byte]
					ID_DAP_Vendor2		1
				*/
				break;
			case ID_DAP_Vendor3:
				/*
					uart read:
					Request:
					CMD [1 byte]
					ID_DAP_Vendor3
					Response:
					CMD [1 byte]		length [1 byte]			read data
					ID_DAP_Vendor3		[0, 0xff]				.. .. ..
				*/
				break;
			case ID_DAP_Vendor4:
				/*
					uart write:
					Request:
					CMD [1 byte]		length [1 byte]			write data
					ID_DAP_Vendor4		[0, 0xff]				.. .. ..
					Response:
					CMD [1 byte]		const
					ID_DAP_Vendor4		1
				*/
				break;
			case ID_DAP_Vendor5:
				/*
					Select uart
					Request:
					CMD [1 byte]		uart num [1 byte]
					ID_DAP_Vendor5		[0, 0xff]
					Response:
					CMD [1 byte]		uart num [1 byte]
					ID_DAP_Vendor5		[0, 0xff]
				*/
				break;
#endif	// PROJ_CFG_DAP_VERDOR_UART_ENABLE
#if PROJ_CFG_DAP_VERDOR_BOOTLOADER_ENABLE
			case ID_DAP_Vendor30:
			case ID_DAP_Vendor31:
				if (param->vendor_handler)
				{
					uint16_t req_size = 5;
					if (cmd_id == ID_DAP_Vendor30)
						req_size += GET_LE_U16(request + req_ptr);
					uint16_t resp_size = param->vendor_handler(cmd_id, request + req_ptr, response + resp_ptr, req_size, param->pkt_size - resp_ptr);
					
					if (resp_size)
					{
						req_ptr += req_size;
						resp_ptr += resp_size;
						break;
					}
				}
				goto fault;
				break;
#endif	// PROJ_CFG_DAP_VERDOR_BOOTLOADER_ENABLE
			default:
				goto fault;
			}
		}
#if PROJ_CFG_DAP_STANDARD_ENABLE
		else if (cmd_id == ID_DAP_Info)
		{
			data = request[req_ptr++];
			if (resp_ptr <= (param->pkt_size - 1 - get_dap_info(param, data, NULL)))
			{
				uint8_t num = get_dap_info(param, data, &response[resp_ptr + 1]);
				response[resp_ptr] = num;
				resp_ptr += num + 1;
			}
			else
				goto fault;
		}
		else if (cmd_id == ID_DAP_HostStatus)
		{
			data = request[req_ptr++];
			if (data == DAP_DEBUGGER_CONNECTED)
			{
				if (request[req_ptr++] & 0x1)
					PERIPHERAL_LED_RED_ON();
				else
					PERIPHERAL_LED_RED_OFF();
				response[resp_ptr++] = DAP_OK;
			}
			else if (data == DAP_TARGET_RUNNING)
			{
				if (request[req_ptr++] & 0x1)
					PERIPHERAL_LED_GREEN_ON();
				else
					PERIPHERAL_LED_GREEN_OFF();
				response[resp_ptr++] = DAP_OK;
			}
			else
				response[resp_ptr++] = DAP_ERROR;
		}
		else if (cmd_id == ID_DAP_Connect)
		{
			data = request[req_ptr++];
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
			response[resp_ptr++] = data;
		}
		else if (cmd_id == ID_DAP_Disconnect)
		{
			port_fini(param->port);
			param->port = DAP_PORT_DISABLED;
			response[resp_ptr++] = DAP_OK;
		}
		else if (cmd_id == ID_DAP_Delay)
		{
			data = (GET_LE_U16(request + req_ptr) + 999) / 1000;
			data += vsfhal_tickclk_get_ms();
			while (data > vsfhal_tickclk_get_ms());
			resp_ptr += 2;
			response[resp_ptr++] = DAP_OK;
		}
		else if (cmd_id == ID_DAP_ResetTarget)
		{
			response[resp_ptr++] = DAP_OK;
			response[resp_ptr++] = 0;	// reset target return
		}
		else if (cmd_id == ID_DAP_SWJ_Pins)
		{
			uint8_t value, select;
			uint32_t delay;
			value = request[req_ptr];
			select = request[req_ptr + 1];
			delay = GET_LE_U32(request + req_ptr + 2);

			if (select & (0x1 << DAP_SWJ_nRESET))
			{
				if (value & (0x1 << DAP_SWJ_nRESET))
					PERIPHERAL_GPIO_SRST_SET();
				else
					PERIPHERAL_GPIO_SRST_CLEAR();
				PERIPHERAL_GPIO_SRST_SET_OUTPUT();
			}
			if (param->port == DAP_PORT_JTAG)
			{
				if (select & (0x1 << DAP_SWJ_nTRST))
				{
					if (value & (0x1 << DAP_SWJ_nTRST))
						PERIPHERAL_GPIO_TRST_SET();
					else
						PERIPHERAL_GPIO_TRST_CLEAR();
					PERIPHERAL_GPIO_TRST_SET_OUTPUT();
				}
			}

			if (delay)
			{
				if (delay > 3000000)
					delay = 3000000;
				delay = (delay + 999) / 1000 + vsfhal_tickclk_get_ms();
				while (delay > vsfhal_tickclk_get_ms());
			}

			response[resp_ptr++] = 
					(PERIPHERAL_GPIO_TCK_GET() ? (0x1 << DAP_SWJ_SWCLK_TCK) : 0) |
					(PERIPHERAL_GPIO_TMS_GET() ? (0x1 << DAP_SWJ_SWDIO_TMS) : 0) |
					(PERIPHERAL_GPIO_TDI_GET() ? (0x1 << DAP_SWJ_TDI) : 0) |
					(PERIPHERAL_GPIO_TDO_GET() ? (0x1 << DAP_SWJ_TDO) : 0) |
					(PERIPHERAL_GPIO_TRST_GET() ? (0x1 << DAP_SWJ_nTRST) : 0) |
					(PERIPHERAL_GPIO_SRST_GET() ? (0x1 << DAP_SWJ_nRESET) : 0);
		}
		else if (cmd_id == ID_DAP_SWJ_Clock)
		{
			data = GET_LE_U32(request + req_ptr);
			req_ptr += 4;
			if (data)
			{
				param->khz = data / 1000;
				if (param->port == DAP_PORT_SWD)
					vsfhal_swd_config(param->khz, param->transfer.idle_cycles, 
							param->swd_conf.turnaround, param->swd_conf.data_phase,
							param->transfer.retry_count);
				else if (param->port == DAP_PORT_JTAG)			
					vsfhal_jtag_config(param->khz, param->transfer.retry_count,
							param->transfer.idle_cycles);
				response[resp_ptr++] = DAP_OK;
			}
			else
				response[resp_ptr++] = DAP_ERROR;
		}
		else if (cmd_id == ID_DAP_SWJ_Sequence)
		{
			data = request[req_ptr++];
			if (!data)
				data = 256;

			if (param->port == DAP_PORT_SWD)
			{
				vsfhal_swd_seqout(request + req_ptr, data);
				req_ptr += (data + 7) >> 3;
				response[resp_ptr++] = DAP_OK;
			}
			else if (param->port == DAP_PORT_JTAG)
			{
				uint8_t bytes;

				data = min(data, 64);
				bytes = (data + 7) >> 3;
				memcpy(&buf_tms, request + req_ptr, bytes);
				req_ptr += bytes;
				buf_tdi = 0xffffffffffffffff;
				vsfhal_jtag_raw(data, (uint8_t *)&buf_tdi, (uint8_t *)&buf_tms, (uint8_t *)&buf_tdo);
				response[resp_ptr++] = DAP_OK;
			}
			else
			{
				req_ptr += (data + 7) >> 3;
				response[resp_ptr++] = DAP_ERROR;
			}
		}
		else if (cmd_id == ID_DAP_SWD_Configure)
		{
			uint8_t v = request[req_ptr++];
			param->swd_conf.turnaround = (v & 0x03) + 1;
			param->swd_conf.data_phase = (v & 0x04) ? 1 : 0;
			vsfhal_swd_config(param->khz, param->transfer.idle_cycles, 
					param->swd_conf.turnaround, param->swd_conf.data_phase,
					param->transfer.retry_count);
			response[resp_ptr++] = DAP_OK;
		}
		else if (cmd_id == ID_DAP_JTAG_Sequence)
		{
			if (param->port == DAP_PORT_JTAG)
			{
				response[resp_ptr++] = DAP_OK;
				
				transfer_cnt = 0;
				transfer_num = request[req_ptr++];
				
				while (transfer_cnt < transfer_num)
				{
					uint8_t info, bitlen, bytes;
					
					// prepare tms tdi
					info = request[req_ptr++];
					bitlen = info & JTAG_SEQUENCE_TCK;
					if (!bitlen) bitlen = 64;
					bytes = (bitlen + 7) >> 3;
					buf_tms = (info & JTAG_SEQUENCE_TMS) ? 0xffffffffffffffff : 0;
					memcpy(&buf_tdi, request + req_ptr, bytes);
					req_ptr += bytes;

					// transfer
					vsfhal_jtag_raw(bitlen, (uint8_t *)&buf_tdi, (uint8_t *)&buf_tms, (uint8_t *)&buf_tdo);
					
					// process tdo
					if (info & JTAG_SEQUENCE_TDO)
					{
						memcpy(response + resp_ptr, &buf_tdo, bytes);
						resp_ptr += bytes;
					}
					transfer_cnt++;
				}
			}
			else
				response[resp_ptr++] = DAP_ERROR;
		}
		else if (cmd_id == ID_DAP_JTAG_Configure)
		{
			uint32_t n, count, length, bits = 0;
			
			count = request[req_ptr++];
			param->jtag_dev.count = min(count, DAP_JTAG_DEV_CNT);
			for (n = 0; n < count; n++)
			{
				length = request[req_ptr++];
				if (n < count)
				{
					param->jtag_dev.ir_length[n] = length;
					param->jtag_dev.ir_before[n] = bits;
				}
				bits += length;
			}
			for (n = 0; n < count; n++)
			{
				bits -= param->jtag_dev.ir_length[n];
				param->jtag_dev.ir_after[n] = bits;
			}
			response[resp_ptr++] = DAP_OK;
		}
		else if (cmd_id == ID_DAP_JTAG_IDCODE)
		{
			param->jtag_dev.index = request[req_ptr++];
			
			if ((param->port == DAP_PORT_JTAG) && (param->jtag_dev.index < DAP_JTAG_DEV_CNT))
			{
				uint16_t bitlen;
				
				// Select JTAG chain
				vsfhal_jtag_ir(JTAG_IDCODE, param->jtag_dev.ir_length[param->jtag_dev.index],
						param->jtag_dev.ir_before[param->jtag_dev.index],
						param->jtag_dev.ir_after[param->jtag_dev.index]);

				// Read IDCODE register
				buf_tdi = 0;
				buf_tms = 0;
				buf_tdo = 0;
				bitlen = 0;
				buf_tms |= 0x1 << bitlen;
				bitlen += 3 + param->jtag_dev.index + 32;
				buf_tms |= 0x3 << (bitlen - 1);
				bitlen += 2;
				vsfhal_jtag_raw(bitlen, (uint8_t *)&buf_tdi, (uint8_t *)&buf_tms, (uint8_t *)&buf_tdo);
				response[resp_ptr++] = DAP_OK;
				SET_LE_U32(response + resp_ptr, (uint32_t)(buf_tdo >> (3 + param->jtag_dev.index)));
				resp_ptr += 4;
			}
			else
			{
				response[resp_ptr++] = DAP_ERROR;
			}
		}
		else if (cmd_id == ID_DAP_TransferConfigure)
		{
			param->transfer.idle_cycles = request[req_ptr];
			param->transfer.retry_count = max(GET_LE_U16(request + req_ptr + 1), 255);
			param->transfer.match_retry = GET_LE_U16(request + req_ptr + 3);
			
			if (param->port == DAP_PORT_DISABLED)
			{
				if (port_init(param, DAP_PORT_JTAG) == VSFERR_NONE)
					param->port = DAP_PORT_JTAG;
			}

			if (param->port == DAP_PORT_SWD)
				vsfhal_swd_config(param->khz, param->transfer.idle_cycles, 
						param->swd_conf.turnaround, param->swd_conf.data_phase,
						param->transfer.retry_count);
			else if (param->port == DAP_PORT_JTAG)
				vsfhal_jtag_config(param->khz, param->transfer.retry_count,
						param->transfer.idle_cycles);
			req_ptr += 5;
			response[resp_ptr++] = DAP_OK;
		}
		else if (cmd_id == ID_DAP_Transfer)
		{
			uint8_t transfer_req;
			bool post_read = false;
			uint16_t transfer_retry, transfer_ack = 0;
			param->do_abort = false;

			if (param->port == DAP_PORT_SWD)
			{
				bool check_write = false;
				uint16_t resp_start = resp_ptr;

				transfer_cnt = 0;
				transfer_num = request[req_ptr + 1];
				req_ptr += 2;
				resp_ptr += 2;

				while (transfer_cnt < transfer_num)
				{
					transfer_cnt++;
					transfer_req = request[req_ptr++];
					
					if (transfer_req & DAP_TRANSFER_RnW)	// read
					{
						if (post_read)
						{
							if ((transfer_req & (DAP_TRANSFER_APnDP | DAP_TRANSFER_MATCH_VALUE)) == DAP_TRANSFER_APnDP)
							{
								// Read previous AP data and post next AP read
								transfer_ack = vsfhal_swd_read(transfer_req, &data);
							}
							else
							{
								post_read = false;
								// Read previous AP data
								transfer_ack = vsfhal_swd_read(DP_RDBUFF | DAP_TRANSFER_RnW, &data);
							}
							if (transfer_ack != DAP_TRANSFER_OK)
								break;
							SET_LE_U32(response + resp_ptr, data);
							resp_ptr += 4;
						}
						
						if (transfer_req & DAP_TRANSFER_MATCH_VALUE)
						{
							if (transfer_req & DAP_TRANSFER_APnDP)
							{
								// Post AP read
								transfer_ack = vsfhal_swd_read(transfer_req, &data);
								if (transfer_ack != DAP_TRANSFER_OK)
									break;
							}
							
							transfer_retry = param->transfer.match_retry;
							do
							{
								// Read register until its value matches or retry counter expires
								transfer_ack = vsfhal_swd_read(transfer_req, &data);
								if (transfer_ack != DAP_TRANSFER_OK)
									break;
							} while (((data & param->transfer.match_mask) != GET_LE_U32(request + req_ptr)) &&
									transfer_retry-- && !param->do_abort);
							if ((data & param->transfer.match_mask) != GET_LE_U32(request + req_ptr))
								transfer_ack |= DAP_TRANSFER_MISMATCH;
							req_ptr += 4;
							if (transfer_ack != DAP_TRANSFER_OK)
								break;
						}
						else	// Normal read
						{
							if (transfer_req & DAP_TRANSFER_APnDP)	// Read AP register
							{
								if (!post_read)	// Post AP read
								{
									transfer_ack = vsfhal_swd_read(transfer_req, &data);
									if (transfer_ack != DAP_TRANSFER_OK)
										break;
									post_read = true;
								}
							}
							else	// Read DP register
							{
								transfer_ack = vsfhal_swd_read(transfer_req, &data);
								if (transfer_ack != DAP_TRANSFER_OK)
									break;
								SET_LE_U32(response + resp_ptr, data);
								resp_ptr += 4;
							}
						}
						check_write = false;
					}
					else	// Write register
					{
						if (post_read)
						{
							transfer_ack = vsfhal_swd_read(DP_RDBUFF | DAP_TRANSFER_RnW, &data);
							if (transfer_ack != DAP_TRANSFER_OK)
								break;
							SET_LE_U32(response + resp_ptr, data);
							resp_ptr += 4;
							post_read = false;
						}
						
						data = GET_LE_U32(request + req_ptr);
						req_ptr += 4;
						
						if (transfer_req & DAP_TRANSFER_MATCH_MASK)
						{
							// Write match mask
							param->transfer.match_mask = data;
							transfer_ack = DAP_TRANSFER_OK;
						}
						else
						{
							// Write DP/AP register
							transfer_ack = vsfhal_swd_write(transfer_req, data);
							if (transfer_ack != DAP_TRANSFER_OK)
								break;
							check_write = true;
						}
					}

					if (param->do_abort)
						break;
				}
				
				if (transfer_cnt < transfer_num)
				{
					uint16_t i = 0;
					while ((transfer_cnt + i) < transfer_num)
					{
						data = request[req_ptr++];
						if (data & DAP_TRANSFER_RnW)
						{
							if (data & DAP_TRANSFER_MATCH_VALUE)	// Read with value match
								req_ptr += 4;
						}
						else	// Write register
							req_ptr += 4;
						i++;
					}
				}
				
				if (transfer_ack == DAP_TRANSFER_OK)
				{
					if (post_read)
					{
						transfer_ack = vsfhal_swd_read(DP_RDBUFF | DAP_TRANSFER_RnW, &data);
						if (transfer_ack == DAP_TRANSFER_OK)
						{
							SET_LE_U32(response + resp_ptr, data);
							resp_ptr += 4;
						}
					}
					else if (check_write)
					{
						transfer_ack = vsfhal_swd_read(DP_RDBUFF | DAP_TRANSFER_RnW, &data);
					}
				}
				
				response[resp_start++] = transfer_cnt;
				response[resp_start++] = transfer_ack;
			}
			else if (param->port == DAP_PORT_JTAG)
			{
				uint8_t jtag_ir = 0;
				uint16_t resp_start;

				resp_start = resp_ptr;
				resp_ptr += 2;
				transfer_cnt = 0;
				param->jtag_dev.index = request[req_ptr++];
				transfer_num = request[req_ptr++];
				if (param->jtag_dev.index >= param->jtag_dev.count)
					goto DAP_Transfer_EXIT;
				
				while (transfer_cnt < transfer_num)
				{
					uint8_t request_ir;

					transfer_req = request[req_ptr++];
					request_ir = (transfer_req & DAP_TRANSFER_APnDP) ? JTAG_APACC : JTAG_DPACC;
					
					if (transfer_req & DAP_TRANSFER_RnW)	// Read register
					{
						if (post_read)
						{
							if ((jtag_ir == request_ir) &&
									((transfer_req & DAP_TRANSFER_MATCH_VALUE) == 0))
							{
								// Read previous data and post next read
								transfer_ack = vsfhal_jtag_dr(transfer_req, data, param->jtag_dev.index,
										param->jtag_dev.count - param->jtag_dev.index - 1, &data);
								if (transfer_ack != DAP_TRANSFER_OK)
									break;
							}
							else
							{
								// Select JTAG chain
								if (jtag_ir != JTAG_DPACC)
								{
									jtag_ir = JTAG_DPACC;
									vsfhal_jtag_ir(JTAG_DPACC, param->jtag_dev.ir_length[param->jtag_dev.index],
											param->jtag_dev.ir_before[param->jtag_dev.index],
											param->jtag_dev.ir_after[param->jtag_dev.index]);
								}
								// Read previous data
								transfer_ack = vsfhal_jtag_dr(DP_RDBUFF | DAP_TRANSFER_RnW, data, param->jtag_dev.index,
										param->jtag_dev.count - param->jtag_dev.index - 1, &data);
								if (transfer_ack != DAP_TRANSFER_OK)
									break;
								post_read = 0;
							}
							SET_LE_U32(response + resp_ptr, data);
							resp_ptr += 4;
						}
						if (transfer_req & DAP_TRANSFER_MATCH_VALUE)
						{
							// Select JTAG chain
							if (jtag_ir != request_ir)
							{
								jtag_ir = request_ir;
								vsfhal_jtag_ir(jtag_ir, param->jtag_dev.ir_length[param->jtag_dev.index],
										param->jtag_dev.ir_before[param->jtag_dev.index],
										param->jtag_dev.ir_after[param->jtag_dev.index]);
							}
							
							// Post DP/AP read
							transfer_ack = vsfhal_jtag_dr(transfer_req, data, param->jtag_dev.index,
									param->jtag_dev.count - param->jtag_dev.index - 1, &data);
							if (transfer_ack != DAP_TRANSFER_OK)
								break;
							
							transfer_retry = param->transfer.match_retry;
							do
							{
								transfer_ack = vsfhal_jtag_dr(transfer_req, data, param->jtag_dev.index,
										param->jtag_dev.count - param->jtag_dev.index - 1, &data);
								if (transfer_ack != DAP_TRANSFER_OK)
									break;
							} while (((data & param->transfer.match_mask) != GET_LE_U32(request + req_ptr)) &&
									transfer_retry-- && !param->do_abort);
							if ((data & param->transfer.match_mask) != GET_LE_U32(request + req_ptr))
								transfer_ack |= DAP_TRANSFER_MISMATCH;
							req_ptr += 4;
							if (transfer_ack != DAP_TRANSFER_OK)
								break;
						}
						else	// Normal read
						{
							if (!post_read)
							{
								// Select JTAG chain
								if (jtag_ir != request_ir)
								{
									jtag_ir = request_ir;
									vsfhal_jtag_ir(jtag_ir, param->jtag_dev.ir_length[param->jtag_dev.index],
											param->jtag_dev.ir_before[param->jtag_dev.index],
											param->jtag_dev.ir_after[param->jtag_dev.index]);
								}
								
								// Post DP/AP read
								transfer_ack = vsfhal_jtag_dr(transfer_req, data, param->jtag_dev.index,
										param->jtag_dev.count - param->jtag_dev.index - 1, &data);
								if (transfer_ack != DAP_TRANSFER_OK)
									break;
								post_read = 1;
							}
						}
					}
					else	// Write register
					{
						if (post_read)
						{
							// Select JTAG chain
							if (jtag_ir != JTAG_DPACC)
							{
								jtag_ir = JTAG_DPACC;
								vsfhal_jtag_ir(jtag_ir, param->jtag_dev.ir_length[param->jtag_dev.index],
										param->jtag_dev.ir_before[param->jtag_dev.index],
										param->jtag_dev.ir_after[param->jtag_dev.index]);
								if (transfer_ack != DAP_TRANSFER_OK)
									break;
							}
							// Read previous data
							transfer_ack = vsfhal_jtag_dr(DP_RDBUFF | DAP_TRANSFER_RnW, data, param->jtag_dev.index,
									param->jtag_dev.count - param->jtag_dev.index - 1, &data);
							if (transfer_ack != DAP_TRANSFER_OK)
								break;
							SET_LE_U32(response + resp_ptr, data);
							resp_ptr += 4;
							post_read = 0;
						}

						if (transfer_req & DAP_TRANSFER_MATCH_MASK)	// Write match mask
						{
							param->transfer.match_mask = GET_LE_U32(request + req_ptr);
							req_ptr += 4;
							transfer_ack = DAP_TRANSFER_OK;
						}
						else
						{
							// Select JTAG chain
							if (jtag_ir != request_ir)
							{
								jtag_ir = request_ir;
								vsfhal_jtag_ir(jtag_ir, param->jtag_dev.ir_length[param->jtag_dev.index],
										param->jtag_dev.ir_before[param->jtag_dev.index],
										param->jtag_dev.ir_after[param->jtag_dev.index]);
							}
							// Write DP/AP register
							transfer_ack = vsfhal_jtag_dr(transfer_req,
									GET_LE_U32(request + req_ptr), param->jtag_dev.index,
									param->jtag_dev.count - param->jtag_dev.index - 1, &data);
							req_ptr += 4;
							if (transfer_ack != DAP_TRANSFER_OK)
								break;
						}
					}

					transfer_cnt++;				
					if (param->do_abort)
						break;
				}

				if (transfer_cnt < transfer_num)
				{
					uint16_t i = 0;	
					while ((transfer_cnt + i) < transfer_num)
					{
						data = request[req_ptr++];
						if (data & DAP_TRANSFER_RnW)
						{
							if (data & DAP_TRANSFER_MATCH_VALUE)	// Read with value match
								req_ptr += 4;
						}
						else	// Write register
							req_ptr += 4;
						i++;
					}
				}
				
				if (transfer_ack == DAP_TRANSFER_OK)
				{
					if (jtag_ir != JTAG_DPACC)
					{
						jtag_ir = JTAG_DPACC;
						vsfhal_jtag_ir(jtag_ir, param->jtag_dev.ir_length[param->jtag_dev.index],
								param->jtag_dev.ir_before[param->jtag_dev.index],
								param->jtag_dev.ir_after[param->jtag_dev.index]);
					}

					transfer_ack = vsfhal_jtag_dr(DP_RDBUFF | DAP_TRANSFER_RnW,
							data, param->jtag_dev.index,
							param->jtag_dev.count - param->jtag_dev.index - 1, &data);
					if (post_read && (transfer_ack == DAP_TRANSFER_OK))
					{
						SET_LE_U32(response + resp_ptr, data);
						resp_ptr += 4;
					}
				}
				
				response[resp_start++] = transfer_cnt;
				response[resp_start++] = transfer_ack;
			}
			else
			{
				uint32_t request_value, request_count;

	DAP_Transfer_EXIT:
				request_count = request[req_ptr + 1];
				req_ptr += 2;
				while (request_count)
				{
					request_count--;
					request_value = request[req_ptr++];
					if (request_value & DAP_TRANSFER_RnW)
					{
						if (request_value & DAP_TRANSFER_MATCH_VALUE)
							req_ptr += 4;
					}
					else
						req_ptr += 4;
				}
				response[resp_ptr++] = 0;	// Response count
				response[resp_ptr++] = 0;	// Response value
			}
		}
		else if (cmd_id == ID_DAP_TransferBlock)
		{
			uint8_t transfer_req;
			uint16_t transfer_ack = 0;

			if (param->port == DAP_PORT_SWD)
			{
				uint16_t resp_start = resp_ptr;
				transfer_cnt = 0;
				transfer_num = GET_LE_U16(request + req_ptr + 1);
				req_ptr += 3;
				resp_ptr += 3;
				
				if (!transfer_num)
					goto DAP_TransferBlock_SWD_END;

				transfer_req = request[req_ptr++];
				if (transfer_req & DAP_TRANSFER_RnW)	// Read register block
				{
					if (transfer_req & DAP_TRANSFER_APnDP)	// Post AP read
					{
						transfer_ack = vsfhal_swd_read(transfer_req, &data);
						if (transfer_ack != DAP_TRANSFER_OK)
							goto DAP_TransferBlock_SWD_END;
					}
					
					while (transfer_cnt < transfer_num)	// Read DP/AP register
					{
						if ((transfer_cnt == (transfer_num - 1)) &&
								(transfer_req & DAP_TRANSFER_APnDP))
							transfer_req = DP_RDBUFF | DAP_TRANSFER_RnW;
						
						transfer_ack = vsfhal_swd_read(transfer_req, &data);
						if (transfer_ack != DAP_TRANSFER_OK)
							goto DAP_TransferBlock_SWD_END;
						SET_LE_U32(response + resp_ptr, data);
						resp_ptr += 4;
						transfer_cnt++;
					}
				}
				else	// Write register block
				{
					while (transfer_cnt < transfer_num)	// Write register block
					{
						transfer_ack = vsfhal_swd_write(transfer_req, GET_LE_U32(request + req_ptr));
						req_ptr += 4;
						if (transfer_ack != DAP_TRANSFER_OK)
							goto DAP_TransferBlock_SWD_END;
						transfer_cnt++;
					}

					// Check last write
					transfer_ack = vsfhal_swd_read(DP_RDBUFF | DAP_TRANSFER_RnW, &data);
				}
				
			DAP_TransferBlock_SWD_END:
				SET_LE_U16(response + resp_start, transfer_cnt);
				response[resp_start + 2] = transfer_ack;
			}
			else if (param->port == DAP_PORT_JTAG)
			{
				uint8_t jtag_ir;
				uint16_t resp_start;

				if (request[req_ptr] >= DAP_JTAG_DEV_CNT)
					goto DAP_TransferBlock_EXIT;
				
				param->jtag_dev.index = request[req_ptr];			
				resp_start = resp_ptr;
				transfer_cnt = 0;
				transfer_num = GET_LE_U16(request + req_ptr + 1);
				transfer_ack = 0;
				req_ptr += 3;
				resp_ptr += 3;
				
				transfer_req = request[req_ptr++];
				
				// Select JTAG chain
				jtag_ir = (transfer_req & DAP_TRANSFER_APnDP) ? JTAG_APACC : JTAG_DPACC;
				vsfhal_jtag_ir(jtag_ir, param->jtag_dev.ir_length[param->jtag_dev.index],
						param->jtag_dev.ir_before[param->jtag_dev.index],
						param->jtag_dev.ir_after[param->jtag_dev.index]);
				
				if (transfer_req & DAP_TRANSFER_RnW)
				{
					// Post read
					transfer_ack = vsfhal_jtag_dr(transfer_req,
							data, param->jtag_dev.index,
							param->jtag_dev.count - param->jtag_dev.index - 1, &data);
					if (transfer_ack != DAP_TRANSFER_OK)
						goto DAP_TransferBlock_ERROR;
					
					// Read register block
					while (transfer_cnt < transfer_num)
					{
						if (transfer_cnt == (transfer_num - 1))	// Last read
						{
							if (jtag_ir != JTAG_DPACC)
							{
								jtag_ir = JTAG_DPACC;
								vsfhal_jtag_ir(jtag_ir, param->jtag_dev.ir_length[param->jtag_dev.index],
										param->jtag_dev.ir_before[param->jtag_dev.index],
										param->jtag_dev.ir_after[param->jtag_dev.index]);
							}
							transfer_req = DP_RDBUFF | DAP_TRANSFER_RnW;
						}
						
						transfer_ack = vsfhal_jtag_dr(transfer_req,	data, param->jtag_dev.index,
								param->jtag_dev.count - param->jtag_dev.index - 1, &data);
						if (transfer_ack != DAP_TRANSFER_OK)
							goto DAP_TransferBlock_ERROR;
						SET_LE_U32(response + resp_ptr, data);
						resp_ptr += 4;
						transfer_cnt++;
					}
				}
				else	// Write register block
				{
					while (transfer_cnt < transfer_num)
					{
						// Write DP/AP register
						transfer_ack = vsfhal_jtag_dr(transfer_req, GET_LE_U32(request + req_ptr), param->jtag_dev.index,
								param->jtag_dev.count - param->jtag_dev.index - 1, &data);
						req_ptr += 4;
						if (transfer_ack != DAP_TRANSFER_OK)
							goto DAP_TransferBlock_ERROR;
						transfer_cnt++;
					}
					
					// Check last write
					if (jtag_ir != JTAG_DPACC)
					{
						jtag_ir = JTAG_DPACC;
						vsfhal_jtag_ir(jtag_ir, param->jtag_dev.ir_length[param->jtag_dev.index],
								param->jtag_dev.ir_before[param->jtag_dev.index],
								param->jtag_dev.ir_after[param->jtag_dev.index]);
					}
					
					transfer_ack = vsfhal_jtag_dr(DP_RDBUFF | DAP_TRANSFER_RnW, data, param->jtag_dev.index,
							param->jtag_dev.count - param->jtag_dev.index - 1, &data);
					if (transfer_ack != DAP_TRANSFER_OK)
						goto DAP_TransferBlock_ERROR;
				}

				SET_LE_U16(response + resp_start, transfer_cnt);
				response[resp_start + 2] = transfer_ack;
			}
			else
			{
	DAP_TransferBlock_EXIT:
				if (request[req_ptr + 3] & DAP_TRANSFER_RnW)
					req_ptr += 4;
				else
					req_ptr += 4 + 4 * GET_LE_U16(request + req_ptr + 1);
	DAP_TransferBlock_ERROR:
				response[resp_ptr++] = 0;	// Response count [7:0]
				response[resp_ptr++] = 0;	// Response count [15:8]
				response[resp_ptr++] = 0;	// Response value
			}
		}
		else if (cmd_id == ID_DAP_WriteABORT)
		{
			uint16_t transfer_ack;

			if (param->port == DAP_PORT_SWD)
			{
				// Ignore DAP index
				data = GET_LE_U32(request + req_ptr + 1);
				req_ptr += 5;
				transfer_ack = vsfhal_swd_write(DP_ABORT, data);
				response[resp_ptr++] = DAP_OK;
			}
			else if (param->port == DAP_PORT_JTAG)
			{
				if (request[req_ptr] >= DAP_JTAG_DEV_CNT)
					goto DAP_WriteABORT_EXIT;
				
				param->jtag_dev.index = request[req_ptr++];

				vsfhal_jtag_ir(JTAG_ABORT, param->jtag_dev.ir_length[param->jtag_dev.index],
						param->jtag_dev.ir_before[param->jtag_dev.index],
						param->jtag_dev.ir_after[param->jtag_dev.index]);
				
				// Write Abort register
				transfer_ack = vsfhal_jtag_dr(0, GET_LE_U32(request + req_ptr), param->jtag_dev.index,
						param->jtag_dev.count - param->jtag_dev.index - 1, &data);
				req_ptr += 4;
				if (transfer_ack != DAP_TRANSFER_OK)
					goto DAP_WriteABORT_ERROR;
				
				response[resp_ptr++] = DAP_OK;
			}
			else
			{
	DAP_WriteABORT_EXIT:
				req_ptr += 5;
	DAP_WriteABORT_ERROR:
				response[resp_ptr++] = DAP_ERROR;
			}
		}
	#if ((SWO_UART != 0) || (SWO_MANCHESTER != 0))
		else if (cmd_id == ID_DAP_SWO_Transport)
		{
			uint8_t ret = DAP_ERROR;
			uint8_t transport = request[req_ptr++];
			if (!(param->trace_status & DAP_SWO_CAPTURE_ACTIVE))
			{
				if (transport <= 1)
				{
					param->transport = transport;
					ret = DAP_OK;
				}
			}
			response[resp_ptr++] = ret;
		}
		else if (cmd_id == ID_DAP_SWO_Mode)
		{
			uint8_t mode = request[req_ptr++];
			if ((mode == DAP_SWO_OFF) || (mode == DAP_SWO_UART))
			{
				param->trace_mode = mode;
				response[resp_ptr++] = DAP_OK;
			}
			else
			{
				param->trace_mode = DAP_SWO_OFF;
				response[resp_ptr++] = DAP_ERROR;
			}
		}
		else if (cmd_id == ID_DAP_SWO_Baudrate)
		{
			uint32_t baudrate = GET_LE_U32(request + req_ptr);
			req_ptr += 4;
			if (baudrate > SWO_UART_MAX_BAUDRATE)
				baudrate = SWO_UART_MAX_BAUDRATE;
			else if (baudrate < SWO_UART_MIN_BAUDRATE)
				baudrate = SWO_UART_MIN_BAUDRATE;
	#if (SWO_UART != 0)
			if (param->update_swo_usart_param)
				param->update_swo_usart_param(NULL, &baudrate);
	#endif
			SET_LE_U32(response + resp_ptr, baudrate);
			resp_ptr += 4;
		}
		else if (cmd_id == ID_DAP_SWO_Control)
		{
			uint8_t active = request[req_ptr++] & DAP_SWO_CAPTURE_ACTIVE;
			if (active != (param->trace_status & DAP_SWO_CAPTURE_ACTIVE))
			{
				if (active)
					STREAM_INIT((struct vsf_stream_t *)&param->swo_rx);
				param->trace_status = active;
			}
			response[resp_ptr++] = DAP_OK;
		}
		else if (cmd_id == ID_DAP_SWO_Status)
		{
			uint32_t count = STREAM_GET_DATA_SIZE((struct vsf_stream_t *)&param->swo_rx);		
			response[resp_ptr++] = param->trace_status;
			SET_LE_U32(response + resp_ptr, count);
			resp_ptr += 4;
		}
		else if (cmd_id == ID_DAP_SWO_Data)
		{
			uint16_t count = min(GET_LE_U16(request + req_ptr),
					STREAM_GET_DATA_SIZE((struct vsf_stream_t *)&param->swo_rx));
			req_ptr += 2;
			response[resp_ptr++] = param->trace_status;
			if (param->transport != 1)
				count = 0;
			if (count > (param->pkt_size - 2 - resp_ptr))
				count = param->pkt_size - 2 - resp_ptr;
			if (count)
			{
				struct vsf_buffer_t buffer;			
				buffer.buffer = response + resp_ptr + 2;
				buffer.size = count;
				count = STREAM_READ((struct vsf_stream_t *)&param->swo_rx, &buffer);
			}
			SET_LE_U16(response + resp_ptr, count);
			resp_ptr += 2 + count;
			
			if (param->trace_status == (DAP_SWO_CAPTURE_ACTIVE | DAP_SWO_CAPTURE_PAUSED))
			{
				if (STREAM_GET_FREE_SIZE((struct vsf_stream_t *)&param->swo_rx))
					param->trace_status = DAP_SWO_CAPTURE_ACTIVE;
			}
		}
	#endif
#endif // PROJ_CFG_DAP_STANDARD_ENABLE
		else
		{
			goto fault;
		}

	} while (cmd_num && (resp_ptr < param->pkt_size));
	goto exit;

fault:
	param->response[resp_ptr - 1] = ID_DAP_Invalid;
exit:
	return resp_ptr;
}

static vsf_err_t dap_thread(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct dap_param_t *param = pt->user_data;

	vsfsm_pt_begin(pt);
	
	while (1)
	{
		if (vsfsm_sem_pend(&param->request_sem, pt->sm))
		{
			param->busy = false;
			vsfsm_pt_wfe(pt, param->request_sem.evt);
		}

		param->busy = true;
		param->response_size = cmd_handler(param, param->request[param->request_head], param->response);

		param->request_head++;
		if (param->request_head == DAP_PACKET_COUNT)
			param->request_head = 0;
		DISABLE_GLOBAL_INTERRUPT();
		param->request_cnt--;
		ENABLE_GLOBAL_INTERRUPT();

		if (vsfsm_sem_pend(&param->response_sem, pt->sm))
			vsfsm_pt_wfe(pt, param->response_sem.evt);
		
		if (param->cb_response)
			param->cb_response(param->cb_param, param->response, param->response_size);
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
#if PROJ_CFG_DAP_STANDARD_ENABLE
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
	param->swo_rx.stream.op = &fifostream_op;
	param->swo_rx.mem.buffer.buffer = (uint8_t *)param->swo_buff;
	param->swo_rx.mem.buffer.size = sizeof(param->swo_buff);
#endif
#endif	// PROJ_CFG_DAP_STANDARD_ENABLE
	
	param->sem_sm.init_state.evt_handler = sem_evt_handler;
	param->sem_sm.user_data = param;
	vsfsm_init(&param->sem_sm);
	
	vsfsm_sem_init(&param->request_sem, 0, VSFSM_EVT_DAP_REQUEST);
	vsfsm_sem_init(&param->response_sem, 1, VSFSM_EVT_DAP_RESPONSE);

	param->pt.thread = dap_thread;
	param->pt.user_data = param;
	return vsfsm_pt_init(&param->sm, &param->pt);	
}

vsf_err_t DAP_recvive_request(struct dap_param_t *param, uint8_t *buf, uint16_t size)
{
	if (!size)
		return VSFERR_FAIL;
	
	if (buf[0] == ID_DAP_TransferAbort)
	{
		param->do_abort = true;
		return VSFERR_NONE;
	}
	else if (param->request_cnt < DAP_PACKET_COUNT)
	{
		size = min(size, param->pkt_size);
		if (size)
		{
			uint8_t store = (param->request_head + param->request_cnt) % DAP_PACKET_COUNT;
			memcpy(param->request[store], buf, param->pkt_size);
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
		void (*cb_response)(void *, uint8_t *, uint16_t), uint16_t pkt_size)
{
	if (param->busy)
		return VSFERR_FAIL;

	if (param->cb_response)
		param->cb_response(param->cb_param, NULL, 0);
	param->cb_param = cb_param;
	param->cb_response = cb_response;
	param->pkt_size = pkt_size;
	return VSFERR_NONE;
}

void DAP_send_response_done(struct dap_param_t *param)
{
	vsfsm_post_evt_pending(&param->sem_sm, VSFSM_EVT_ON_SEND_DONE);
}

void DAP_test(struct dap_param_t *param)
{
#if PROJ_CFG_DAP_STANDARD_ENABLE
	param->port = DAP_PORT_JTAG;
	param->khz = 1000;
	param->transfer.retry_count = 100;
	param->swd_conf.turnaround = 1;
	param->jtag_dev.count = 2;
	param->jtag_dev.ir_length[0] = 0x04;
	param->jtag_dev.ir_length[1] = 0x05;
	param->jtag_dev.ir_before[1] = 0x04;
	param->jtag_dev.ir_after[0] = 0x05;
	
	param->request[0][0] = 0x05;
	param->request[0][1] = 0x00;
	param->request[0][2] = 0x01;
	param->request[0][3] = 0x08;
	
	port_init(param, DAP_PORT_JTAG);
	while (1)
		cmd_handler(param, param->request[param->request_head], param->response);
#endif	// PROJ_CFG_DAP_STANDARD_ENABLE
}

