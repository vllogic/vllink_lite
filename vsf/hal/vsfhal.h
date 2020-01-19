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

#ifndef __VSFHAL_H_INCLUDED__
#define __VSFHAL_H_INCLUDED__

#include "vsf_type.h"
#include "vsfhal_cfg.h"
#include "vsfhal_const.h"

/*******************************************************************************
CORE
*******************************************************************************/
vsf_err_t vsfhal_core_init(void *p);
vsf_err_t vsfhal_core_fini(void *p);
void vsfhal_core_reset(void *p);
uint32_t vsfhal_core_get_stack(void);
vsf_err_t vsfhal_core_set_stack(uint32_t sp);
uint8_t vsfhal_core_set_intlevel(uint8_t level);
void vsfhal_core_sleep(uint32_t mode);
vsf_err_t vsfhal_core_pendsv_config(int32_t int_priority, void (*on_pendsv)(void *), void *param);
vsf_err_t vsfhal_core_pendsv_trigger(void);

/*******************************************************************************
UID
*******************************************************************************/
uint32_t vsfhal_uid_get(uint8_t *buffer, uint32_t size);

/*******************************************************************************
tickclk
*******************************************************************************/
vsf_err_t vsfhal_tickclk_init(int32_t int_priority);
vsf_err_t vsfhal_tickclk_fini(void);
vsf_err_t vsfhal_tickclk_start(void);
vsf_err_t vsfhal_tickclk_stop(void);
uint32_t vsfhal_tickclk_get_ms(void);
uint32_t vsfhal_tickclk_get_us(void);
void vsfhal_tickclk_delay(uint16_t delay_tick);
void vsfhal_tickclk_poll(void);
vsf_err_t vsfhal_tickclk_config_cb(void (*)(void*), void*);

/*******************************************************************************
WDT
*******************************************************************************/
vsf_err_t vsfhal_wdt_init(int32_t feed_ms, int32_t rst_delay_ms, void (*cb)(void *), void *param);
vsf_err_t vsfhal_wdt_fini(void);
void vsfhal_wdt_feed(void);

/*******************************************************************************
TIMER
*******************************************************************************/
vsf_err_t vsfhal_timer_init(uint8_t index, int32_t int_priority);
vsf_err_t vsfhal_timer_fini(uint8_t index);
vsf_err_t vsfhal_timer_callback_config(uint8_t index, void (*cb)(void *),
		void *param);
vsf_err_t vsfhal_timer_callback_start(uint8_t index, uint32_t interval_us,
		int32_t trigger_cnt);
vsf_err_t vsfhal_timer_stop(uint8_t index);

/*******************************************************************************
FLASH
*******************************************************************************/
vsf_err_t vsfhal_flash_init(uint8_t index);
vsf_err_t vsfhal_flash_fini(uint8_t index);
vsf_err_t vsfhal_flash_capacity(uint8_t index, uint32_t *pagesize, uint32_t *pagenum);
uint32_t vsfhal_flash_baseaddr(uint8_t index);
uint32_t vsfhal_flash_blocksize(uint8_t index, uint32_t addr, uint32_t size, int op);
vsf_err_t vsfhal_flash_config_cb(uint8_t index, int32_t int_priority, void *param, void (*onfinish)(void*, vsf_err_t));
vsf_err_t vsfhal_flash_erase(uint8_t index, uint32_t addr);
vsf_err_t vsfhal_flash_read(uint8_t index, uint32_t addr, uint8_t *buff);
vsf_err_t vsfhal_flash_write(uint8_t index, uint32_t addr, uint8_t *buff);

/*******************************************************************************
USART
*******************************************************************************/
vsf_err_t vsfhal_usart_init(uint8_t index);
vsf_err_t vsfhal_usart_fini(uint8_t index);
vsf_err_t vsfhal_usart_config(uint8_t index, uint32_t baudrate, uint32_t mode);
vsf_err_t vsfhal_usart_config_cb(uint8_t index, int32_t int_priority, void *p, void (*ontx)(void *), void (*onrx)(void *, uint16_t));
uint16_t vsfhal_usart_tx_bytes(uint8_t index, uint8_t *data, uint16_t size);
uint16_t vsfhal_usart_tx_get_free_size(uint8_t index);
uint16_t vsfhal_usart_rx_bytes(uint8_t index, uint8_t *data, uint16_t size);
uint16_t vsfhal_usart_rx_get_data_size(uint8_t index);

/*******************************************************************************
GPIO
*******************************************************************************/
struct vsfhal_gpio_pin_t
{
	uint8_t port;
	uint8_t pin;
};

vsf_err_t vsfhal_gpio_init(uint8_t index);
vsf_err_t vsfhal_gpio_fini(uint8_t index);
vsf_err_t vsfhal_gpio_config(uint8_t index, uint8_t pin_idx, uint32_t mode);
vsf_err_t vsfhal_gpio_set(uint8_t index, uint32_t pin_mask);
vsf_err_t vsfhal_gpio_clear(uint8_t index, uint32_t pin_mask);
uint32_t vsfhal_gpio_get(uint8_t index, uint32_t pin_mask);

/*******************************************************************************
I2C
*******************************************************************************/
struct vsfhal_i2c_msg_t
{
	uint16_t flag;
	uint16_t len;
	uint8_t *buf;
};
vsf_err_t vsfhal_i2c_init(uint8_t index);
vsf_err_t vsfhal_i2c_fini(uint8_t index);
vsf_err_t vsfhal_i2c_config(uint8_t index, uint16_t kHz);
vsf_err_t vsfhal_i2c_config_cb(uint8_t index, int32_t int_priority,
		void *param, void (*cb)(void*, vsf_err_t));
vsf_err_t vsfhal_i2c_xfer(uint8_t index, uint16_t addr,
		struct vsfhal_i2c_msg_t *msg, uint8_t msg_len);

/*******************************************************************************
SPI
*******************************************************************************/
vsf_err_t vsfhal_spi_init(uint8_t index);
vsf_err_t vsfhal_spi_fini(uint8_t index);
vsf_err_t vsfhal_spi_config(uint8_t index, uint32_t kHz, uint32_t mode);
vsf_err_t vsfhal_spi_config_cb(uint8_t index, int32_t int_priority,
		void *p, void (*callback)(void *));
vsf_err_t vsfhal_spi_start(uint8_t index, uint8_t *out, uint8_t *in,
		uint32_t len);
uint32_t vsfhal_spi_stop(uint8_t index);

/*******************************************************************************
PWM
*******************************************************************************/
vsf_err_t vsfhal_pwm_init(uint32_t index);
vsf_err_t vsfhal_pwm_fini(uint32_t index);
vsf_err_t vsfhal_pwm_config_mode(uint32_t index, uint32_t mode);
vsf_err_t vsfhal_pwm_config_freq(uint32_t index, uint32_t Hz);
vsf_err_t vsfhal_pwm_out(uint32_t index, uint16_t rate);

/*******************************************************************************
ADC
*******************************************************************************/
vsf_err_t vsfhal_adc_init(uint8_t index);
vsf_err_t vsfhal_adc_fini(uint8_t index);
vsf_err_t vsfhal_adc_config(uint8_t index, uint32_t SPS);
vsf_err_t vsfhal_adc_config_cb(uint8_t index, int32_t int_priority,
		void *p, void (*callback)(void *, int32_t));
int32_t vsfhal_adc_start(uint8_t index, uint8_t channel);
vsf_err_t vsfhal_adc_stop(uint8_t index, uint8_t channel);

/*******************************************************************************
USBD
*******************************************************************************/
enum vsfhal_usbd_eptype_t
{
	USB_EP_TYPE_CONTROL,
	USB_EP_TYPE_INTERRUPT,
	USB_EP_TYPE_BULK,
	USB_EP_TYPE_ISO
};
enum vsfhal_usbd_evt_t
{
	VSFHAL_USBD_ON_ATTACH = 0,
	VSFHAL_USBD_ON_DETACH,
	VSFHAL_USBD_ON_RESET,
	VSFHAL_USBD_ON_SETUP,
	VSFHAL_USBD_ON_ERROR,
	VSFHAL_USBD_ON_SUSPEND,
	VSFHAL_USBD_ON_RESUME,
	VSFHAL_USBD_ON_SOF,
	VSFHAL_USBD_ON_IN,
	VSFHAL_USBD_ON_NAK,
	VSFHAL_USBD_ON_OUT,
	VSFHAL_USBD_ON_UNDERFLOW,
	VSFHAL_USBD_ON_OVERFLOW,
	VSFHAL_USBD_ON_USER,
};
enum vsfhal_usbd_error_t
{
	USBERR_ERROR,
	USBERR_INVALID_CRC,
	USBERR_SOF_TO,
};
struct vsfhal_usbd_callback_t
{
	void *param;
	void (*on_event)(void*, enum vsfhal_usbd_evt_t, uint32_t);
};

#include "vsfhal_usb.h"

#endif	// __VSFHAL_H_INCLUDED__

