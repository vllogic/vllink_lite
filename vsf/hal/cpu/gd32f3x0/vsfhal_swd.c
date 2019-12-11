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
#include "vsfhal_core.h"

/*
Reference Document:
	"Serial Wire Debug and the CoreSightTM Debug and Trace Architecture"
	https://github.com/ARMmbed/DAPLink/blob/master/source/daplink/cmsis-dap/SW_DP.c
*/

#ifdef DAP_BLOCK_TRANSFER

#define SWD_SUCCESS				0x00
#define SWD_FAULT				0x80
#define SWD_RETRY_OUT			0x40
#define SWD_ACK_ERROR			0x20
#define SWD_PARITY_ERROR		0x10

#define SWD_ACK_OK				0x01
#define SWD_ACK_WAIT			0x02
#define SWD_ACK_FAULT			0x04

#define SWD_TRANS_RnW			(1 << 2)

struct vsfhal_swd_param_t
{
	void (*swd_delay)(uint16_t delay_tick);
	void (*swd_read)(uint8_t *data, uint32_t bits);
	void (*swd_write)(uint8_t *data, uint32_t bits);

	uint16_t cpu_clk_div_half_spi_clk;

	bool data_force;
	uint8_t idle;
	uint8_t trn;
	uint16_t retry;
} static swd_param;

extern void delay_swd_125ns(uint16_t dummy);
extern void delay_swd_250ns(uint16_t dummy);

static uint32_t get_parity_32bit(uint32_t data)
{
	uint32_t temp;
	temp = data >> 16;
	data = data ^ temp;
	temp = data >> 8;
	data = data ^ temp;
	temp = data >> 4;
	data = data ^ temp;
	temp = data >> 2;
	data = data ^ temp;
	temp = data >> 1;
	data = data ^ temp;
	return data & 0x1;
}

static uint32_t get_parity_4bit(uint8_t data)
{
	uint8_t temp;
	temp = data >> 2;
	data = data ^ temp;
	temp = data >> 1;
	data = data ^ temp;
	return data & 0x1;
}

static void swd_read_quick(uint8_t *data, uint32_t bits)
{
	uint8_t end, m;

	if (bits >= 8)
	{
		IO_CFG_AF(PERIPHERAL_GPIO_TMS_MI_PORT, PERIPHERAL_GPIO_TMS_MI_PIN);
		IO_CFG_AF(PERIPHERAL_GPIO_TCK0_PORT, PERIPHERAL_GPIO_TCK0_PIN);

		if (bits >= 16)
		{
			SPI_CTL0(PERIPHERAL_SWD_SPI) &= ~SPI_CTL0_SPIEN;
			SPI_CTL0(PERIPHERAL_SWD_SPI) |= SPI_CTL0_FF16;
			SPI_CTL0(PERIPHERAL_SWD_SPI) |= SPI_CTL0_SPIEN;
			do
			{
				SPI_DATA(PERIPHERAL_SWD_SPI) = 0;
				while (SPI_STAT(PERIPHERAL_SWD_SPI) & SPI_STAT_TRANS);
				*(uint16_t *)data =	SPI_DATA(PERIPHERAL_SWD_SPI);
				data += 2;
				bits -= 16;
			} while (bits >= 16);
			SPI_CTL0(PERIPHERAL_SWD_SPI) &= ~SPI_CTL0_FF16;		
		}
		while (bits >= 8)
		{
			SPI_DATA(PERIPHERAL_SWD_SPI) = 0;
			while (SPI_STAT(PERIPHERAL_SWD_SPI) & SPI_STAT_TRANS);
			*data =	SPI_DATA(PERIPHERAL_SWD_SPI);
			data += 1;
			bits -= 8;
		}

		IO_CFG_INPUT(PERIPHERAL_GPIO_TMS_MI_PORT, PERIPHERAL_GPIO_TMS_MI_PIN);
		IO_CFG_OUTPUT(PERIPHERAL_GPIO_TCK0_PORT, PERIPHERAL_GPIO_TCK0_PIN);
	}

	if (bits)
	{
		m = 8 - bits;
		end = 0;
		do
		{
			end >>= 1;
			IO_CLEAR(PERIPHERAL_GPIO_TCK0_PORT, PERIPHERAL_GPIO_TCK0_PIN);
			end |= IO_GET(PERIPHERAL_GPIO_TMS_MI_PORT, PERIPHERAL_GPIO_TMS_MI_PIN) ? 0x80 : 0;
			IO_SET(PERIPHERAL_GPIO_TCK0_PORT, PERIPHERAL_GPIO_TCK0_PIN);
			__ASM("NOP");
			bits--;
		} while (bits);
		*data = end >> m;
	}
}

static void swd_read_slow(uint8_t *data, uint32_t bits)
{
	uint16_t end, m;

	if (bits >= 8)
	{
		IO_CFG_AF(PERIPHERAL_GPIO_TMS_MI_PORT, PERIPHERAL_GPIO_TMS_MI_PIN);
		IO_CFG_AF(PERIPHERAL_GPIO_TCK0_PORT, PERIPHERAL_GPIO_TCK0_PIN);

		if (bits >= 16)
		{
			SPI_CTL0(PERIPHERAL_SWD_SPI) &= ~SPI_CTL0_SPIEN;
			SPI_CTL0(PERIPHERAL_SWD_SPI) |= SPI_CTL0_FF16;
			SPI_CTL0(PERIPHERAL_SWD_SPI) |= SPI_CTL0_SPIEN;
			do
			{
				SPI_DATA(PERIPHERAL_SWD_SPI) = 0;
				while (SPI_STAT(PERIPHERAL_SWD_SPI) & SPI_STAT_TRANS);
				*(uint16_t *)data =	SPI_DATA(PERIPHERAL_SWD_SPI);
				data += 2;
				bits -= 16;
			} while (bits >= 16);
			SPI_CTL0(PERIPHERAL_SWD_SPI) &= ~SPI_CTL0_FF16;		
		}
		while (bits >= 8)
		{
			SPI_DATA(PERIPHERAL_SWD_SPI) = 0;
			while (SPI_STAT(PERIPHERAL_SWD_SPI) & SPI_STAT_TRANS);
			*data =	SPI_DATA(PERIPHERAL_SWD_SPI);
			data += 1;
			bits -= 8;
		}

		IO_CFG_INPUT(PERIPHERAL_GPIO_TMS_MI_PORT, PERIPHERAL_GPIO_TMS_MI_PIN);
		IO_CFG_OUTPUT(PERIPHERAL_GPIO_TCK0_PORT, PERIPHERAL_GPIO_TCK0_PIN);
	}

	if (bits)
	{
		m = 8 - bits;
		end = 0;
		do
		{
			end >>= 1;
			IO_CLEAR(PERIPHERAL_GPIO_TCK0_PORT, PERIPHERAL_GPIO_TCK0_PIN);
			if (swd_param.swd_delay)
				swd_param.swd_delay(swd_param.cpu_clk_div_half_spi_clk);
			end |= IO_GET(PERIPHERAL_GPIO_TMS_MI_PORT, PERIPHERAL_GPIO_TMS_MI_PIN) ? 0x80 : 0;
			IO_SET(PERIPHERAL_GPIO_TCK0_PORT, PERIPHERAL_GPIO_TCK0_PIN);
			if (swd_param.swd_delay)
				swd_param.swd_delay(swd_param.cpu_clk_div_half_spi_clk);
			bits--;
		} while (bits);
		*data = end >> m;
	}
}

static void swd_write_quick(uint8_t *data, uint32_t bits)
{
	uint16_t end;

	if (bits >= 8)
	{
		IO_CFG_AF(PERIPHERAL_GPIO_TMS_MO_PORT, PERIPHERAL_GPIO_TMS_MO_PIN);
		IO_CFG_AF(PERIPHERAL_GPIO_TCK0_PORT, PERIPHERAL_GPIO_TCK0_PIN);

		if (bits >= 16)
		{
			SPI_CTL0(PERIPHERAL_SWD_SPI) &= ~SPI_CTL0_SPIEN;
			SPI_CTL0(PERIPHERAL_SWD_SPI) |= SPI_CTL0_FF16;
			SPI_CTL0(PERIPHERAL_SWD_SPI) |= SPI_CTL0_SPIEN;
			do
			{
				SPI_DATA(PERIPHERAL_SWD_SPI) = *(uint16_t *)data;
				data += 2;
				bits -= 16;
				while (SPI_STAT(PERIPHERAL_SWD_SPI) & SPI_STAT_TRANS);
				end = SPI_DATA(PERIPHERAL_SWD_SPI);
			} while (bits >= 16);
			SPI_CTL0(PERIPHERAL_SWD_SPI) &= ~SPI_CTL0_FF16;		
		}
		while (bits >= 8)
		{
			SPI_DATA(PERIPHERAL_SWD_SPI) = *data;
			data += 1;
			bits -= 8;
			while (SPI_STAT(PERIPHERAL_SWD_SPI) & SPI_STAT_TRANS);
			end = SPI_DATA(PERIPHERAL_SWD_SPI);
		}

		if (IO_GET(PERIPHERAL_GPIO_TMS_MI_PORT, PERIPHERAL_GPIO_TMS_MI_PIN))
			IO_SET(PERIPHERAL_GPIO_TMS_MO_PORT, PERIPHERAL_GPIO_TMS_MO_PIN);
		else
			IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_PORT, PERIPHERAL_GPIO_TMS_MO_PIN);
		IO_CFG_OUTPUT(PERIPHERAL_GPIO_TMS_MO_PORT, PERIPHERAL_GPIO_TMS_MO_PIN);
		IO_CFG_OUTPUT(PERIPHERAL_GPIO_TCK0_PORT, PERIPHERAL_GPIO_TCK0_PIN);
	}

	if (bits)
	{
		end = *data;
		do
		{
			if (end & 0x1)
				IO_SET(PERIPHERAL_GPIO_TMS_MO_PORT, PERIPHERAL_GPIO_TMS_MO_PIN);
			else
				IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_PORT, PERIPHERAL_GPIO_TMS_MO_PIN);
			IO_CLEAR(PERIPHERAL_GPIO_TCK0_PORT, PERIPHERAL_GPIO_TCK0_PIN);
			end >>= 1;
			bits--;
			IO_SET(PERIPHERAL_GPIO_TCK0_PORT, PERIPHERAL_GPIO_TCK0_PIN);
		} while (bits);
	}

	IO_CFG_INPUT(PERIPHERAL_GPIO_TMS_MO_PORT, PERIPHERAL_GPIO_TMS_MO_PIN);
}

static void swd_write_slow(uint8_t *data, uint32_t bits)
{
	uint16_t end;

	if (bits >= 8)
	{
		IO_CFG_AF(PERIPHERAL_GPIO_TMS_MO_PORT, PERIPHERAL_GPIO_TMS_MO_PIN);
		IO_CFG_AF(PERIPHERAL_GPIO_TCK0_PORT, PERIPHERAL_GPIO_TCK0_PIN);

		if (bits >= 16)
		{
			SPI_CTL0(PERIPHERAL_SWD_SPI) &= ~SPI_CTL0_SPIEN;
			SPI_CTL0(PERIPHERAL_SWD_SPI) |= SPI_CTL0_FF16;
			SPI_CTL0(PERIPHERAL_SWD_SPI) |= SPI_CTL0_SPIEN;
			do
			{
				SPI_DATA(PERIPHERAL_SWD_SPI) = *(uint16_t *)data;
				data += 2;
				bits -= 16;
				while (SPI_STAT(PERIPHERAL_SWD_SPI) & SPI_STAT_TRANS);
				end = SPI_DATA(PERIPHERAL_SWD_SPI);
			} while (bits >= 16);
			SPI_CTL0(PERIPHERAL_SWD_SPI) &= ~SPI_CTL0_FF16;		
		}
		while (bits >= 8)
		{
			SPI_DATA(PERIPHERAL_SWD_SPI) = *data;
			data += 1;
			bits -= 8;
			while (SPI_STAT(PERIPHERAL_SWD_SPI) & SPI_STAT_TRANS);
			end = SPI_DATA(PERIPHERAL_SWD_SPI);
		}

		if (IO_GET(PERIPHERAL_GPIO_TMS_MI_PORT, PERIPHERAL_GPIO_TMS_MI_PIN))
			IO_SET(PERIPHERAL_GPIO_TMS_MO_PORT, PERIPHERAL_GPIO_TMS_MO_PIN);
		else
			IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_PORT, PERIPHERAL_GPIO_TMS_MO_PIN);
		IO_CFG_OUTPUT(PERIPHERAL_GPIO_TMS_MO_PORT, PERIPHERAL_GPIO_TMS_MO_PIN);
		IO_CFG_OUTPUT(PERIPHERAL_GPIO_TCK0_PORT, PERIPHERAL_GPIO_TCK0_PIN);
	}

	if (bits)
	{
		end = *data;
		do
		{
			if (end & 0x1)
				IO_SET(PERIPHERAL_GPIO_TMS_MO_PORT, PERIPHERAL_GPIO_TMS_MO_PIN);
			else
				IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_PORT, PERIPHERAL_GPIO_TMS_MO_PIN);
			IO_CLEAR(PERIPHERAL_GPIO_TCK0_PORT, PERIPHERAL_GPIO_TCK0_PIN);
			end >>= 1;
			bits--;
			if (swd_param.swd_delay)
				swd_param.swd_delay(swd_param.cpu_clk_div_half_spi_clk);
			IO_SET(PERIPHERAL_GPIO_TCK0_PORT, PERIPHERAL_GPIO_TCK0_PIN);
			if (swd_param.swd_delay)
				swd_param.swd_delay(swd_param.cpu_clk_div_half_spi_clk);
		} while (bits);
	}

	IO_CFG_INPUT(PERIPHERAL_GPIO_TMS_MO_PORT, PERIPHERAL_GPIO_TMS_MO_PIN);
}

void vsfhal_swd_seqout(uint8_t *data, uint16_t bitlen)
{
	swd_param.swd_write(data, bitlen);
}

void vsfhal_swd_seqin(uint8_t *data, uint16_t bitlen)
{
	swd_param.swd_read(data, bitlen);
}

int vsfhal_swd_read(uint8_t request, uint32_t *r_data)
{
	uint16_t retry;
	uint32_t ret[2], ack;

	retry = 0;
	request <<= 1;
	request = (request & 0x1e) | 0x81 | (get_parity_4bit(request >> 1) << 5);
	do
	{
		swd_param.swd_write(&request, 8);
		swd_param.swd_read((uint8_t *)&ack, swd_param.trn + 3);
		ack = (ack >> swd_param.trn) & 0x7;
		if (ack == SWD_ACK_OK)
		{
			swd_param.swd_read((uint8_t *)&ret, 32 + 1 + swd_param.trn + swd_param.idle);
			if ((ret[1] & 0x1) == get_parity_32bit(ret[0]))
			{
				*r_data = ret[0];
				return SWD_ACK_OK | SWD_SUCCESS;
			}
			else
			{
				*r_data = ret[0];
				return SWD_ACK_OK | SWD_PARITY_ERROR;
			}
		}
		else if ((ack == SWD_ACK_WAIT) || (ack == SWD_ACK_FAULT))
		{
			if (swd_param.data_force)
				swd_param.swd_read(NULL, 32 + 1 + swd_param.trn + swd_param.idle);
			else
				swd_param.swd_read(NULL, swd_param.trn);

			if (ack != SWD_ACK_WAIT)
				break;
		}
		else
		{
			swd_param.swd_read(NULL, 32 + 1 + swd_param.trn);
			break;
		}
	} while (retry++ < swd_param.retry);

	*r_data = 0;
	return ack;
}

int vsfhal_swd_write(uint8_t request, uint32_t w_data)
{
	uint16_t retry;
	uint32_t ret[2], ack;

	retry = 0;
	request <<= 1;
	request = (request & 0x1e) | 0x81 | (get_parity_4bit(request >> 1) << 5);
	do
	{
		swd_param.swd_write(&request, 8);
		swd_param.swd_read((uint8_t *)&ack, swd_param.trn * 2 + 3);
		ack = (ack >> swd_param.trn) & 0x7;
		if (ack == SWD_ACK_OK)
		{
			ret[0] = w_data;
			ret[1] = 0xfffffffe | get_parity_32bit(w_data);
			swd_param.swd_write((uint8_t *)&ret, 32 + 1 + swd_param.idle);
			return SWD_ACK_OK | SWD_SUCCESS;
		}
		else if ((ack == SWD_ACK_WAIT) || (ack == SWD_ACK_FAULT))
		{
			if (swd_param.data_force)
			{
				ret[0] = 0;
				ret[1] = 0;
				swd_param.swd_write((uint8_t *)&ret, 32 + 1);
			}
			
			if (ack != SWD_ACK_WAIT)
				break;
		}
		else
		{
			// Back off data phase
			swd_param.swd_read(NULL, 32 + 1 + swd_param.trn);
			break;
		}
	} while (retry++ < swd_param.retry);

	return ack;
}

vsf_err_t vsfhal_swd_init(int32_t int_priority)
{
	if (int_priority < 0)
		return VSFERR_INVALID_PARAMETER;

	PERIPHERAL_GPIO_TMS_INIT();
	PERIPHERAL_GPIO_TCK_INIT();
	PERIPHERAL_GPIO_SRST_INIT();
	PERIPHERAL_SWD_IO_AF_CONFIG();

	IO_CFG_HIGHSPEED(PERIPHERAL_GPIO_TMS_MO_PORT, PERIPHERAL_GPIO_TMS_MO_PIN);
	IO_CFG_HIGHSPEED(PERIPHERAL_GPIO_TCK0_PORT, PERIPHERAL_GPIO_TCK0_PIN);
	
	IO_SET(PERIPHERAL_GPIO_TCK0_PORT, PERIPHERAL_GPIO_TCK0_PIN);
	return VSFERR_NONE;
}

vsf_err_t vsfhal_swd_fini(void)
{
	PERIPHERAL_GPIO_TMS_FINI();
	PERIPHERAL_GPIO_TCK_FINI();
	PERIPHERAL_GPIO_SRST_FINI();

	return VSFERR_NONE;
}

const static uint16_t spi_clk_list[] = 
{
	// 64M	PSC
	32000,	// 2
	16000,	// 4
	8000,	// 8
	4000,	// 16
	2000,	// 32
	1000,	// 64
	500,	// 128
	250,	// 256
};

vsf_err_t vsfhal_swd_config(uint16_t kHz, uint8_t idle, uint8_t trn,
		bool data_force, uint16_t retry)
{
	uint32_t temp;
	struct vsfhal_info_t *info;
	
	if (vsfhal_core_get_info(&info) || (NULL == info))
		return VSFERR_BUG;

	if (kHz < spi_clk_list[dimof(spi_clk_list) - 1])
		kHz = spi_clk_list[dimof(spi_clk_list) - 1];
	for (temp = 0; temp < dimof(spi_clk_list); temp++)
	{
		if (kHz >= spi_clk_list[temp])
		{
			kHz = spi_clk_list[temp];
			break;
		}
	}
	swd_param.cpu_clk_div_half_spi_clk = info->cpu_freq_hz / (kHz * 2000);

	if (kHz >= 8000)
	{
		swd_param.swd_read = swd_read_quick;
		swd_param.swd_write = swd_write_quick;
		swd_param.swd_delay = NULL;
	}
	else if (kHz >= 4000)
	{
		swd_param.swd_read = swd_read_slow;
		swd_param.swd_write = swd_write_slow;
		swd_param.swd_delay = NULL;
	}
	else if (kHz >= 2000)
	{
		swd_param.swd_read = swd_read_slow;
		swd_param.swd_write = swd_write_slow;
		swd_param.swd_delay = delay_swd_125ns;
	}
	else if (kHz >= 1000)
	{
		swd_param.swd_read = swd_read_slow;
		swd_param.swd_write = swd_write_slow;
		swd_param.swd_delay = delay_swd_250ns;
	}
	else
	{
		swd_param.swd_read = swd_read_slow;
		swd_param.swd_write = swd_write_slow;
		swd_param.swd_delay = vsfhal_tickclk_delay;
	}
	
	RCU_APB2EN |= RCU_APB2EN_SPI0EN;
	RCU_APB2RST |= RCU_APB2RST_SPI0RST;
	RCU_APB2RST &= ~RCU_APB2RST_SPI0RST;

	if (temp < dimof(spi_clk_list))
	{
		SPI_CTL0(PERIPHERAL_SWD_SPI) &= ~SPI_CTL0_PSC;
		SPI_CTL0(PERIPHERAL_SWD_SPI) |= temp << 3;
	}	
	SPI_CTL0(PERIPHERAL_SWD_SPI) &= ~(SPI_CTL0_BDEN | SPI_CTL0_CRCEN | SPI_CTL0_FF16);	
	SPI_CTL0(PERIPHERAL_SWD_SPI) |= SPI_CTL0_MSTMOD | SPI_CTL0_SWNSSEN | SPI_CTL0_SWNSS |
			SPI_CTL0_LF | SPI_CTL0_CKPL | SPI_CTL0_CKPH;
	SPI_CTL0(PERIPHERAL_SWD_SPI) |= SPI_CTL0_SPIEN;

	if (idle <= (32 * 6))
		swd_param.idle = idle;
	else
		swd_param.idle = 32 * 6;
	if (trn <= 14)
		swd_param.trn = trn;
	else
		swd_param.trn = 14;
	swd_param.data_force = data_force;
	swd_param.retry = retry;
	
	return VSFERR_NONE;
}

#endif
