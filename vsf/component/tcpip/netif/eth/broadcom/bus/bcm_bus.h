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

#include "app_type.h"

#include "vsfhal.h"

#include "framework/vsfsm/vsfsm.h"
#include "framework/vsftimer/vsftimer.h"

#include "../../../vsfip_netif.h"
#include "component/fundation/buffer/buffer.h"

#ifndef __BCM_BUS_H_INCLUDED__
#define __BCM_BUS_H_INCLUDED__

#define BCM_CMD_READ			(uint8_t)0
#define BCM_CMD_WRITE			(uint8_t)1

#define BCM_ACCESS_FIX			(uint8_t)0
#define BCM_ACCESS_INCREMENTAL	(uint8_t)1

#define BCM_FUNC_BUS			(uint8_t)0
#define BCM_FUNC_BACKPLANE		(uint8_t)1
#define BCM_FUNC_F2				(uint8_t)2

struct bcm_bus_t;
struct bcm_bus_op_t
{
	vsf_err_t (*init)(struct vsfsm_pt_t *pt, vsfsm_evt_t evt);
	vsf_err_t (*enable)(struct vsfsm_pt_t *pt, vsfsm_evt_t evt);
	vsf_err_t (*waitf2)(struct vsfsm_pt_t *pt, vsfsm_evt_t evt);

	void (*init_int)(struct bcm_bus_t *bus, void (*callback)(void *param),
						void *param);
	void (*fini_int)(struct bcm_bus_t *bus);
	void (*enable_int)(struct bcm_bus_t *bus);
	bool (*is_int)(struct bcm_bus_t *bus);

	vsf_err_t (*f2_avail)(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
								uint16_t *size);
	vsf_err_t (*f2_read)(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
								uint16_t size, struct vsf_buffer_t *buffer);
	vsf_err_t (*f2_abort)(struct vsfsm_pt_t *pt, vsfsm_evt_t evt);

	uint32_t (*fix_u32)(struct bcm_bus_t *bcm_bus, uint32_t value);
	vsf_err_t (*transact)(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
						uint8_t rw, uint8_t func, uint32_t addr, uint16_t size,
						struct vsf_buffer_t *buffer);
};

#if VSFHAL_SPI_EN
struct bcm_bus_spi_t
{
	uint32_t retry;
	uint32_t bushead;
	uint32_t cur_backplane_base;
};
#endif

#if VSFHAL_SDIO_EN
struct bcm_bus_sdio_t
{
	struct sdio_info_t info;

	uint8_t index;
	uint8_t temp_byte;
	uint8_t logic_retry;
	uint8_t transfer_retry;

	struct vsf_buffer_t *transact_buf;
	uint32_t transact_size;

	uint8_t buff[8];
	struct vsf_buffer_t read_buf;
	uint16_t header[4];
	struct vsf_buffer_t header_buf;
};
#endif

struct bcm_bus_t
{
	enum
	{
		BCM_BUS_TYPE_SPI,
		BCM_BUS_TYPE_SDIO,
	} type;
	vsf_err_t (*firmware_read)(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
								uint8_t *buf, uint32_t offset, uint32_t len);
	struct bcm_bus_op_t const *op;
	struct
	{
		uint16_t freq_khz;
		uint8_t index;

		uint8_t rst_port;
		uint8_t rst_pin;
		uint8_t wakeup_port;
		uint8_t wakeup_pin;
		uint8_t mode_port;
		uint8_t mode_pin;

		struct
		{
			uint8_t cs_port;
			uint8_t cs_pin;
			uint8_t eint_port;
			uint8_t eint_pin;
			uint8_t eint;
		} spi;
	} port;

	union
	{
#if VSFHAL_SPI_EN
		struct bcm_bus_spi_t spi;
#endif
#if VSFHAL_SDIO_EN
		struct bcm_bus_sdio_t sdio;
#endif
	} priv;

	struct vsfip_netif_t *sta_netif;
	struct vsfip_netif_t *ap_netif;

	// init_pt is used in bcm_bus_init
	struct vsfsm_crit_t crit_init;
	struct vsfsm_pt_t init_pt;

	// port_init_pt is used in init procedure in specificed port driver
	struct vsfsm_pt_t port_init_pt;

	// regacc is used in bcm_bus_read_reg and bcm_bus_write_reg
	struct vsfsm_crit_t crit_reg;
	struct vsfsm_pt_t regacc_pt;
	struct vsf_buffer_t regacc_buffer;
	uint8_t regacc_mem[8];

	// download_firmware_pt is used in bcm_bus_download_firmware
	struct vsfsm_pt_t download_firmware_pt;
	struct vsfsm_pt_t download_image_pt;
	struct vsf_buffer_t download_image_buffer;
	uint32_t download_progress;
	uint8_t download_image_mem[64];
#if VSFHAL_SDIO_EN
	uint8_t verify_image_mem[64];
#endif

	// core_ctrl_pt is used in bcm_bus_disable_device_core,
	// 	bcm_bus_reset_device_core and bcm_bus_device_core_isup
	struct vsfsm_pt_t core_ctrl_pt;

	// bufacc is used in bcm_bus_transact
	struct vsfsm_crit_t crit_buffer;
	struct vsfsm_pt_t bufacc_pt;
	struct vsf_buffer_t bufacc_buffer;
	uint32_t f2rdy_retrycnt;
	uint8_t bufacc_mem[8];

	// f2_avail_pt is used in bcm_bus_xxx_f2_avail and bcm_bus_xxx_f2_read
	struct vsfsm_pt_t f2_pt;

	uint8_t is_up;
	uint8_t receiving;
	uint8_t retry;

	uint32_t f1sig;
	uint32_t intf;
	uint32_t status;
	enum
	{
		BCM_BUS_ENDIAN_LE = 0,
		BCM_BUS_ENDIAN_BE = 1,
	} endian;
	enum
	{
		BCM_BUS_WORDLEN_16 = 0,
		BCM_BUS_WORDLEN_32 = 1,
	} word_length;
	uint32_t cur_backplane_base;

	// for bus drivers
	struct vsfsm_t *bus_ops_sm;
	uint32_t buslen;
	uint32_t tmpreg;
};

extern const struct bcm_bus_op_t bcm_bus_spi_op;
extern const struct bcm_bus_op_t bcm_bus_sdio_op;

vsf_err_t bcm_bus_construct(struct bcm_bus_t *bus);
vsf_err_t bcm_bus_init(struct vsfsm_pt_t *pt, vsfsm_evt_t evt);
vsf_err_t bcm_bus_fini(struct vsfsm_pt_t *pt, vsfsm_evt_t evt);
vsf_err_t bcm_bus_transact(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
			uint8_t rw, uint8_t func, uint32_t addr, uint16_t size,
			struct vsf_buffer_t *buffer);
vsf_err_t bcm_bus_read_reg(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
							uint32_t addr, uint8_t len, uint8_t *value);
vsf_err_t bcm_bus_write_reg(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
							uint32_t addr, uint8_t len, uint32_t value);

#endif		// __BCM_BUS_H_INCLUDED__
