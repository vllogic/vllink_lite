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

#ifndef __VSFHAL_USB_H_INCLUDED__
#define __VSFHAL_USB_H_INCLUDED__

extern struct vsfdwc2_device_t vsfhal_usb_dwc2_device;
extern const uint8_t vsfhal_usbd_ep_num;
extern struct vsfhal_usbd_callback_t vsfhal_usbd_callback;

vsf_err_t vsfhal_usbd_init(int32_t int_priority);
vsf_err_t vsfhal_usbd_fini(void);
vsf_err_t vsfhal_usbd_poll(void);
vsf_err_t vsfhal_usbd_reset(void);
vsf_err_t vsfhal_usbd_connect(void);
vsf_err_t vsfhal_usbd_disconnect(void);
vsf_err_t vsfhal_usbd_set_address(uint8_t addr);
uint8_t vsfhal_usbd_get_address(void);
vsf_err_t vsfhal_usbd_wakeup(void);
uint32_t vsfhal_usbd_get_frame_number(void);
vsf_err_t vsfhal_usbd_get_setup(uint8_t *buffer);
vsf_err_t vsfhal_usbd_prepare_buffer(void);
vsf_err_t vsfhal_usbd_ep_reset(uint8_t idx);
vsf_err_t vsfhal_usbd_ep_set_type(uint8_t idx, enum vsfhal_usbd_eptype_t type);
vsf_err_t vsfhal_usbd_ep_set_IN_dbuffer(uint8_t idx);
bool vsfhal_usbd_ep_is_IN_dbuffer(uint8_t idx);
vsf_err_t vsfhal_usbd_ep_switch_IN_buffer(uint8_t idx);
vsf_err_t vsfhal_usbd_ep_set_IN_epsize(uint8_t idx, uint16_t size);
uint16_t vsfhal_usbd_ep_get_IN_epsize(uint8_t idx);
vsf_err_t vsfhal_usbd_ep_set_IN_stall(uint8_t idx);
vsf_err_t vsfhal_usbd_ep_clear_IN_stall(uint8_t idx);
bool vsfhal_usbd_ep_is_IN_stall(uint8_t idx);
vsf_err_t vsfhal_usbd_ep_reset_IN_toggle(uint8_t idx);
vsf_err_t vsfhal_usbd_ep_set_IN_count(uint8_t idx, uint16_t size);
vsf_err_t vsfhal_usbd_ep_write_IN_buffer(uint8_t idx, uint8_t *buffer, uint16_t size);
vsf_err_t vsfhal_usbd_ep_set_OUT_dbuffer(uint8_t idx);
bool vsfhal_usbd_ep_is_OUT_dbuffer(uint8_t idx);
vsf_err_t vsfhal_usbd_ep_switch_OUT_buffer(uint8_t idx);
vsf_err_t vsfhal_usbd_ep_set_OUT_epsize(uint8_t idx, uint16_t size);
uint16_t vsfhal_usbd_ep_get_OUT_epsize(uint8_t idx);
vsf_err_t vsfhal_usbd_ep_set_OUT_stall(uint8_t idx);
vsf_err_t vsfhal_usbd_ep_clear_OUT_stall(uint8_t idx);
bool vsfhal_usbd_ep_is_OUT_stall(uint8_t idx);
vsf_err_t vsfhal_usbd_ep_reset_OUT_toggle(uint8_t idx);
uint16_t vsfhal_usbd_ep_get_OUT_count(uint8_t idx);
vsf_err_t vsfhal_usbd_ep_read_OUT_buffer(uint8_t idx, uint8_t *buffer, uint16_t size);
vsf_err_t vsfhal_usbd_ep_enable_OUT(uint8_t idx);

#ifdef VSFDWC2_DEVICE_ENABLE
vsf_err_t vsfhal_usb_dwc2_device_init(void *p, uint32_t *rx_buff, uint32_t rx_buffer_size);
vsf_err_t vsfhal_usb_dwc2_device_fini(void *p);
#endif

#ifdef VSFDWC2_HOST_ENABLE
#endif

#endif	// __VSFHAL_USB_H_INCLUDED__
