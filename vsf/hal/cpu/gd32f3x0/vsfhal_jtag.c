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
	"IEEE_1149_JTAG_and_Boundary_Scan_Tutorial"
	https://github.com/ARMmbed/DAPLink/blob/master/source/daplink/cmsis-dap/JTAG_DP.c
*/

#ifdef DAP_BLOCK_TRANSFER

#define DAP_TRANSFER_RnW                (1U<<1)
#define DAP_TRANSFER_TIMESTAMP			(1U<<7)

#define DAP_TRANSFER_OK                 (1U<<0)
#define DAP_TRANSFER_WAIT               (1U<<1)

#if TIMESTAMP_CLOCK
extern uint32_t dap_timestamp;
#endif

struct vsfhal_jtag_param_t
{
	void (*jtag_delay)(uint16_t delay_tick);
	void (*jtag_rw)(uint16_t bits, uint8_t *tdi, uint8_t *tms, uint8_t *tdo);

	uint16_t cpu_clk_div_half_spi_clk;

	bool is_spi;
	uint8_t idle;
	uint16_t retry;
} static jtag_param;

extern void delay_jtag_125ns(uint16_t dummy);
extern void delay_jtag_250ns(uint16_t dummy);

static inline void jtag_set_spi_mode(void)
{
	IO_CFG_AF(PERIPHERAL_GPIO_TDI_PORT, PERIPHERAL_GPIO_TDI_PIN);
	IO_CFG_AF(PERIPHERAL_GPIO_TCK0_PORT, PERIPHERAL_GPIO_TCK0_PIN);
	IO_CFG_AF(PERIPHERAL_GPIO_TDO_MI_PORT, PERIPHERAL_GPIO_TDO_MI_PIN);
	jtag_param.is_spi = true;
}

static inline void jtag_set_io_mode(void)
{
	if (IO_GET(PERIPHERAL_GPIO_TDI_PORT, PERIPHERAL_GPIO_TDI_PIN))
		IO_SET(PERIPHERAL_GPIO_TDI_PORT, PERIPHERAL_GPIO_TDI_PIN);
	else
		IO_CLEAR(PERIPHERAL_GPIO_TDI_PORT, PERIPHERAL_GPIO_TDI_PIN);
	IO_CFG_OUTPUT(PERIPHERAL_GPIO_TDI_PORT, PERIPHERAL_GPIO_TDI_PIN);
	IO_CFG_OUTPUT(PERIPHERAL_GPIO_TCK0_PORT, PERIPHERAL_GPIO_TCK0_PIN);
	IO_CFG_INPUT(PERIPHERAL_GPIO_TDO_MI_PORT, PERIPHERAL_GPIO_TDO_MI_PIN);
	jtag_param.is_spi = false;
}

static void jtag_rw_quick(uint16_t bits, uint8_t *tdi, uint8_t *tms, uint8_t *tdo)
{
	uint8_t bits_io, temp, tdi_last, tms_last, tdo_last;

	while (bits >= 8)
	{
		if (*tms == 0)
		{
			if (IO_GET(PERIPHERAL_GPIO_TMS_MO_PORT, PERIPHERAL_GPIO_TMS_MO_PIN))
				IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_PORT, PERIPHERAL_GPIO_TMS_MO_PIN);

			if (!jtag_param.is_spi)
				jtag_set_spi_mode();

			SPI_DATA(PERIPHERAL_JTAG_SPI) = *tdi;
			tdi += 1;
			tms += 1;
			bits -= 8;
			while (SPI_STAT(PERIPHERAL_JTAG_SPI) & SPI_STAT_TRANS);
			*tdo = SPI_DATA(PERIPHERAL_JTAG_SPI);
			tdo += 1;
		}
		else if (*tms == 0xff)
		{
			if (IO_GET(PERIPHERAL_GPIO_TMS_MO_PORT, PERIPHERAL_GPIO_TMS_MO_PIN) == 0)
				IO_SET(PERIPHERAL_GPIO_TMS_MO_PORT, PERIPHERAL_GPIO_TMS_MO_PIN);

			if (!jtag_param.is_spi)
				jtag_set_spi_mode();

			SPI_DATA(PERIPHERAL_JTAG_SPI) = *tdi;
			tdi += 1;
			tms += 1;
			bits -= 8;
			while (SPI_STAT(PERIPHERAL_JTAG_SPI) & SPI_STAT_TRANS);
			*tdo = SPI_DATA(PERIPHERAL_JTAG_SPI);
			tdo += 1;
		}
		else
		{
			if (jtag_param.is_spi)
				jtag_set_io_mode();

			tdi_last = *tdi++;
			tms_last = *tms++;
			tdo_last = 0;
			bits -= 8;
			bits_io = 8;
			do
			{
				tdo_last >>= 1;
				if (tdi_last & 0x1)
					IO_SET(PERIPHERAL_GPIO_TDI_PORT, PERIPHERAL_GPIO_TDI_PIN);
				else
					IO_CLEAR(PERIPHERAL_GPIO_TDI_PORT, PERIPHERAL_GPIO_TDI_PIN);
				if (tms_last & 0x1)
					IO_SET(PERIPHERAL_GPIO_TMS_MO_PORT, PERIPHERAL_GPIO_TMS_MO_PIN);
				else
					IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_PORT, PERIPHERAL_GPIO_TMS_MO_PIN);
				IO_CLEAR(PERIPHERAL_GPIO_TCK0_PORT, PERIPHERAL_GPIO_TCK0_PIN);
				tms_last >>= 1;
				tdi_last >>= 1;
				bits_io--;
				__ASM("NOP");
				__ASM("NOP");
				__ASM("NOP");
				__ASM("NOP");
				IO_SET(PERIPHERAL_GPIO_TCK0_PORT, PERIPHERAL_GPIO_TCK0_PIN);
				tdo_last |= IO_GET(PERIPHERAL_GPIO_TDO_MI_PORT, PERIPHERAL_GPIO_TDO_MI_PIN) ? 0x80 : 0;
			} while (bits_io);
			*tdo++ = tdo_last;
		}
	}

	if (bits)
	{
		if (jtag_param.is_spi)
			jtag_set_io_mode();

		tdi_last = *tdi;
		tms_last = *tms;
		tdo_last = 0;
		temp = 8 - bits;
		do
		{
			tdo_last >>= 1;
			if (tdi_last & 0x1)
				IO_SET(PERIPHERAL_GPIO_TDI_PORT, PERIPHERAL_GPIO_TDI_PIN);
			else
				IO_CLEAR(PERIPHERAL_GPIO_TDI_PORT, PERIPHERAL_GPIO_TDI_PIN);
			if (tms_last & 0x1)
				IO_SET(PERIPHERAL_GPIO_TMS_MO_PORT, PERIPHERAL_GPIO_TMS_MO_PIN);
			else
				IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_PORT, PERIPHERAL_GPIO_TMS_MO_PIN);
			IO_CLEAR(PERIPHERAL_GPIO_TCK0_PORT, PERIPHERAL_GPIO_TCK0_PIN);
			tdi_last >>= 1;
			tms_last >>= 1;
			bits--;
			IO_SET(PERIPHERAL_GPIO_TCK0_PORT, PERIPHERAL_GPIO_TCK0_PIN);
			tdo_last |= IO_GET(PERIPHERAL_GPIO_TDO_MI_PORT, PERIPHERAL_GPIO_TDO_MI_PIN) ? 0x80 : 0;
		} while (bits);
		*tdo = tdo_last >> temp;
	}
}

static void jtag_rw_slow(uint16_t bits, uint8_t *tdi, uint8_t *tms, uint8_t *tdo)
{
	uint8_t bits_io, temp, tdi_last, tms_last, tdo_last;

	while (bits >= 8)
	{
		if (*tms == 0)
		{
			if (IO_GET(PERIPHERAL_GPIO_TMS_MO_PORT, PERIPHERAL_GPIO_TMS_MO_PIN))
				IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_PORT, PERIPHERAL_GPIO_TMS_MO_PIN);

			if (!jtag_param.is_spi)
				jtag_set_spi_mode();

			SPI_DATA(PERIPHERAL_JTAG_SPI) = *tdi;
			tdi += 1;
			tms += 1;
			bits -= 8;
			while (SPI_STAT(PERIPHERAL_JTAG_SPI) & SPI_STAT_TRANS);
			*tdo = SPI_DATA(PERIPHERAL_JTAG_SPI);
			tdo += 1;
		}
		else if (*tms == 0xff)
		{
			if (IO_GET(PERIPHERAL_GPIO_TMS_MO_PORT, PERIPHERAL_GPIO_TMS_MO_PIN) == 0)
				IO_SET(PERIPHERAL_GPIO_TMS_MO_PORT, PERIPHERAL_GPIO_TMS_MO_PIN);

			if (!jtag_param.is_spi)
				jtag_set_spi_mode();

			SPI_DATA(PERIPHERAL_JTAG_SPI) = *tdi;
			tdi += 1;
			tms += 1;
			bits -= 8;
			while (SPI_STAT(PERIPHERAL_JTAG_SPI) & SPI_STAT_TRANS);
			*tdo = SPI_DATA(PERIPHERAL_JTAG_SPI);
			tdo += 1;
		}
		else
		{
			if (jtag_param.is_spi)
				jtag_set_io_mode();

			tdi_last = *tdi++;
			tms_last = *tms++;
			tdo_last = 0;
			bits -= 8;
			bits_io = 8;
			do
			{
				tdo_last >>= 1;
				if (tdi_last & 0x1)
					IO_SET(PERIPHERAL_GPIO_TDI_PORT, PERIPHERAL_GPIO_TDI_PIN);
				else
					IO_CLEAR(PERIPHERAL_GPIO_TDI_PORT, PERIPHERAL_GPIO_TDI_PIN);
				if (tms_last & 0x1)
					IO_SET(PERIPHERAL_GPIO_TMS_MO_PORT, PERIPHERAL_GPIO_TMS_MO_PIN);
				else
					IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_PORT, PERIPHERAL_GPIO_TMS_MO_PIN);
				IO_CLEAR(PERIPHERAL_GPIO_TCK0_PORT, PERIPHERAL_GPIO_TCK0_PIN);
				tdi_last >>= 1;
				tms_last >>= 1;
				bits_io--;
				if (jtag_param.jtag_delay)
					jtag_param.jtag_delay(jtag_param.cpu_clk_div_half_spi_clk);
				IO_SET(PERIPHERAL_GPIO_TCK0_PORT, PERIPHERAL_GPIO_TCK0_PIN);
				tdo_last |= IO_GET(PERIPHERAL_GPIO_TDO_MI_PORT, PERIPHERAL_GPIO_TDO_MI_PIN) ? 0x80 : 0;
				if (jtag_param.jtag_delay)
					jtag_param.jtag_delay(jtag_param.cpu_clk_div_half_spi_clk);
			} while (bits_io);
			*tdo++ = tdo_last;
		}
	}

	if (bits)
	{
		if (jtag_param.is_spi)
			jtag_set_io_mode();

		tdi_last = *tdi;
		tms_last = *tms;
		tdo_last = 0;
		temp = 8 - bits;
		do
		{
			tdo_last >>= 1;
			if (tdi_last & 0x1)
				IO_SET(PERIPHERAL_GPIO_TDI_PORT, PERIPHERAL_GPIO_TDI_PIN);
			else
				IO_CLEAR(PERIPHERAL_GPIO_TDI_PORT, PERIPHERAL_GPIO_TDI_PIN);
			if (tms_last & 0x1)
				IO_SET(PERIPHERAL_GPIO_TMS_MO_PORT, PERIPHERAL_GPIO_TMS_MO_PIN);
			else
				IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_PORT, PERIPHERAL_GPIO_TMS_MO_PIN);
			IO_CLEAR(PERIPHERAL_GPIO_TCK0_PORT, PERIPHERAL_GPIO_TCK0_PIN);
			if (jtag_param.jtag_delay)
				jtag_param.jtag_delay(jtag_param.cpu_clk_div_half_spi_clk);
			tdi_last >>= 1;
			tms_last >>= 1;
			bits--;
			IO_SET(PERIPHERAL_GPIO_TCK0_PORT, PERIPHERAL_GPIO_TCK0_PIN);
			tdo_last |= IO_GET(PERIPHERAL_GPIO_TDO_MI_PORT, PERIPHERAL_GPIO_TDO_MI_PIN) ? 0x80 : 0;
			if (jtag_param.jtag_delay)
				jtag_param.jtag_delay(jtag_param.cpu_clk_div_half_spi_clk);
		} while (bits);
		*tdo = tdo_last >> temp;
	}
}

void vsfhal_jtag_raw(uint16_t bitlen, uint8_t *tdi, uint8_t *tms, uint8_t *tdo)
{
	jtag_param.jtag_rw(bitlen, tdi, tms, tdo);
}

void vsfhal_jtag_ir(uint32_t ir, uint8_t lr_length, uint16_t ir_before, uint16_t ir_after)
{
	uint16_t bitlen;
	uint64_t buf_tdi, buf_tms, buf_tdo;

	bitlen = 0;
	buf_tdi = 0;
	buf_tms = 0;
	lr_length--;

	// Select-DR-Scan, Select-IR-Scan, Capture-IR, Shift-IR
	buf_tms |= (uint64_t)0x3 << bitlen;
	bitlen += 4;
	// Bypass before data
	if (ir_before)
	{
		buf_tdi |= (((uint64_t)0x1 << ir_before) - 1) << bitlen;
		bitlen += ir_before;
	}
	// Set IR bitlen
	if (lr_length)
	{
		buf_tdi |= (ir & (((uint64_t)0x1 << lr_length) - 1)) << bitlen;
		bitlen += lr_length;
	}
	// Bypass after data
	if (ir_after)
	{
		buf_tdi |= ((ir >> lr_length) & 0x1) << bitlen;
		bitlen++;
		ir_after--;
		if (ir_after)
		{
			buf_tdi |= (((uint64_t)0x1 << ir_after) - 1) << bitlen;
			bitlen += ir_after;
		}
		buf_tms |= (uint64_t)0x1 << bitlen;
		buf_tdi |= (uint64_t)0x1 << bitlen;
		bitlen++;
	}
	else
	{
		buf_tms |= (uint64_t)0x1 << bitlen;
		buf_tdi |= ((ir >> lr_length) & 0x1) << bitlen;
		bitlen++;
	}

	// Exit1-IR, Update-IR
	buf_tms |= (uint64_t)0x1 << bitlen;
	bitlen += 1;
	// idle
	buf_tdi |= (uint64_t)0x1 << bitlen;	// keep tdi high
	bitlen += 1;

	jtag_param.jtag_rw(bitlen, (uint8_t *)&buf_tdi, (uint8_t *)&buf_tms, (uint8_t *)&buf_tdo);
}

/*
Read:	vsfhal_jtag_dr(request, 0, dr_before, dr_after, *read_buf)
Write:	vsfhal_jtag_dr(request, write_value, dr_before, dr_after, NULL)
*/
uint8_t vsfhal_jtag_dr(uint8_t request, uint32_t dr, uint16_t dr_before, uint16_t dr_after,
		uint32_t *data)
{
	uint8_t ack;
	uint16_t retry, bitlen;
	uint64_t buf_tdi, buf_tms, buf_tdo;

	retry = 0;
	bitlen = 0;
	buf_tdi = 0;
	buf_tms = 0;
	
	// Select-DR-Scan, Capture-DR, Shift-DR
	buf_tms |= (uint64_t)0x1 << bitlen;
	bitlen += 3;
	// Bypass before data
	bitlen += dr_before;
	// RnW, A2, A3
	buf_tdi |= (uint64_t)((request >> 1) & 0x7) << bitlen;
	bitlen += 3;
	// Data Transfer
	if (!(request & DAP_TRANSFER_RnW))
		buf_tdi |= (uint64_t)dr << bitlen;
	bitlen += 31 + dr_after;		
	buf_tms |= (uint64_t)0x1 << bitlen;
	bitlen++;
	// Update-DR, Idle
	buf_tms |= (uint64_t)0x1 << bitlen;
	bitlen += 1 + jtag_param.idle;
	buf_tdi |= (uint64_t)0x1 << bitlen;	// keep tdi high
	bitlen++;

	#if TIMESTAMP_CLOCK
	if (request & DAP_TRANSFER_TIMESTAMP)
		dap_timestamp = vsfhal_tickclk_get_us();
	#endif

	do
	{
		jtag_param.jtag_rw(bitlen, (uint8_t *)&buf_tdi, (uint8_t *)&buf_tms, (uint8_t *)&buf_tdo);
		ack = (buf_tdo >> (dr_before + 3)) & 0x7;
		ack = (ack & 0x4) | ((ack & 0x2) >> 1) | ((ack & 0x1) << 1);
		if (ack != DAP_TRANSFER_WAIT)
			break;
	} while (retry++ < jtag_param.retry);

	*data = buf_tdo >> (dr_before + 6);	
	return ack;
}

vsf_err_t vsfhal_jtag_init(int32_t int_priority)
{
	if (int_priority < 0)
		return VSFERR_INVALID_PARAMETER;
	
	PERIPHERAL_GPIO_TDI_INIT();
	PERIPHERAL_GPIO_TMS_INIT();
	PERIPHERAL_GPIO_TCK_INIT();
	PERIPHERAL_GPIO_TDO_INIT();
	PERIPHERAL_GPIO_SRST_INIT();
	PERIPHERAL_GPIO_TRST_INIT();
	PERIPHERAL_JTAG_IO_AF_CONFIG();

	IO_SET(PERIPHERAL_GPIO_TDI_PORT, PERIPHERAL_GPIO_TDI_PIN);
	IO_CFG_OUTPUT(PERIPHERAL_GPIO_TDI_PORT, PERIPHERAL_GPIO_TDI_PIN);
	IO_CFG_HIGHSPEED(PERIPHERAL_GPIO_TDI_PORT, PERIPHERAL_GPIO_TDI_PIN);

	IO_SET(PERIPHERAL_GPIO_TMS_MO_PORT, PERIPHERAL_GPIO_TMS_MO_PIN);
	IO_CFG_OUTPUT(PERIPHERAL_GPIO_TMS_MO_PORT, PERIPHERAL_GPIO_TMS_MO_PIN);
	IO_CFG_HIGHSPEED(PERIPHERAL_GPIO_TMS_MO_PORT, PERIPHERAL_GPIO_TMS_MO_PIN);

	IO_SET(PERIPHERAL_GPIO_TCK0_PORT, PERIPHERAL_GPIO_TCK0_PIN);
	IO_CFG_OUTPUT(PERIPHERAL_GPIO_TCK0_PORT, PERIPHERAL_GPIO_TCK0_PIN);	
	IO_CFG_HIGHSPEED(PERIPHERAL_GPIO_TCK0_PORT, PERIPHERAL_GPIO_TCK0_PIN);

	IO_CFG_INPUT(PERIPHERAL_GPIO_TDO_MI_PORT, PERIPHERAL_GPIO_TDO_MI_PIN);

	IO_SET(PERIPHERAL_GPIO_SRST_PORT, PERIPHERAL_GPIO_SRST_PIN);
	IO_CFG_OUTPUT(PERIPHERAL_GPIO_SRST_PORT, PERIPHERAL_GPIO_SRST_PIN);	

	IO_SET(PERIPHERAL_GPIO_TRST_MO_PORT, PERIPHERAL_GPIO_TRST_MO_PIN);
	IO_CFG_OUTPUT(PERIPHERAL_GPIO_TRST_MO_PORT, PERIPHERAL_GPIO_TRST_MO_PIN);
	IO_CFG_HIGHSPEED(PERIPHERAL_GPIO_TRST_MO_PORT, PERIPHERAL_GPIO_TRST_MO_PIN);

	return VSFERR_NONE;
}

vsf_err_t vsfhal_jtag_fini(void)
{
	IO_CFG_INPUT(PERIPHERAL_GPIO_TDI_PORT, PERIPHERAL_GPIO_TDI_PIN);
	IO_CFG_INPUT(PERIPHERAL_GPIO_TMS_MO_PORT, PERIPHERAL_GPIO_TMS_MO_PIN);
	IO_CFG_INPUT(PERIPHERAL_GPIO_TCK0_PORT, PERIPHERAL_GPIO_TCK0_PIN);
	IO_CFG_INPUT(PERIPHERAL_GPIO_TDO_MI_PORT, PERIPHERAL_GPIO_TDO_MI_PIN);
	IO_CFG_INPUT(PERIPHERAL_GPIO_SRST_PORT, PERIPHERAL_GPIO_SRST_PIN);
	IO_CFG_INPUT(PERIPHERAL_GPIO_TRST_MO_PORT, PERIPHERAL_GPIO_TRST_MO_PIN);
	
	PERIPHERAL_GPIO_TDI_FINI();
	PERIPHERAL_GPIO_TMS_FINI();
	PERIPHERAL_GPIO_TCK_FINI();
	PERIPHERAL_GPIO_TDO_FINI();
	PERIPHERAL_GPIO_SRST_FINI();
	PERIPHERAL_GPIO_TRST_FINI();

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

vsf_err_t vsfhal_jtag_config(uint16_t kHz, uint16_t retry, uint8_t idle)
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
	jtag_param.cpu_clk_div_half_spi_clk = info->cpu_freq_hz / (kHz * 2000);

	if (kHz >= 4000)
	{
		jtag_param.jtag_rw = jtag_rw_quick;
		jtag_param.jtag_delay = NULL;
	}
	else if (kHz >= 2000)
	{
		jtag_param.jtag_rw = jtag_rw_slow;
		jtag_param.jtag_delay = delay_jtag_125ns;
	}
	else if (kHz >= 1000)
	{
		jtag_param.jtag_rw = jtag_rw_slow;
		jtag_param.jtag_delay = delay_jtag_250ns;
	}
	else
	{
		jtag_param.jtag_rw = jtag_rw_slow;
		jtag_param.jtag_delay = vsfhal_tickclk_delay;
	}

	RCU_APB2EN |= RCU_APB2EN_SPI0EN;
	RCU_APB2RST |= RCU_APB2RST_SPI0RST;
	RCU_APB2RST &= ~RCU_APB2RST_SPI0RST;

	if (temp < dimof(spi_clk_list))
	{
		SPI_CTL0(PERIPHERAL_JTAG_SPI) &= ~SPI_CTL0_PSC;
		SPI_CTL0(PERIPHERAL_JTAG_SPI) |= temp << 3;
	}
	SPI_CTL0(PERIPHERAL_JTAG_SPI) &= ~(SPI_CTL0_BDEN | SPI_CTL0_CRCEN | SPI_CTL0_FF16);
	SPI_CTL0(PERIPHERAL_JTAG_SPI) |= SPI_CTL0_MSTMOD | SPI_CTL0_SWNSSEN | SPI_CTL0_SWNSS |
			SPI_CTL0_LF | SPI_CTL0_CKPL | SPI_CTL0_CKPH;
	SPI_CTL0(PERIPHERAL_JTAG_SPI) |= SPI_CTL0_SPIEN;

	jtag_set_io_mode();
	
	jtag_param.retry = retry;
	jtag_param.idle = idle;

	return VSFERR_NONE;
}

#endif
