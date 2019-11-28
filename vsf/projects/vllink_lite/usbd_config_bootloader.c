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

static const uint8_t USB_DeviceDescriptor[] =
{
	0x12,        // bLength
	0x01,        // bDescriptorType (Device)
	0x10, 0x02,  // bcdUSB 2.10
	0x00,        // bDeviceClass 
	0x00,        // bDeviceSubClass 
	0x00,        // bDeviceProtocol 
	0x40,        // bMaxPacketSize0 64
	(APPCFG_USBD_VID >> 0) & 0xFF,
	(APPCFG_USBD_VID >> 8) & 0xFF,
				// vendor
	(APPCFG_USBD_PID >> 0) & 0xFF,
	(APPCFG_USBD_PID >> 8) & 0xFF,
				// product
	(APPCFG_USBD_BCD >> 0) & 0xFF,
	(APPCFG_USBD_BCD >> 8) & 0xFF,
				// product
				// bcdDevice
	1,			// manu facturer
	2,			// product
	3,			// serial number
	0x01		// number of configuration 
};

static const uint8_t USB_ConfigDescriptor[] =
{
	USB_DT_CONFIG_SIZE,
	USB_DT_CONFIG,
				// wTotalLength
	(USBD_CONFIGDESC_LENGTH >> 0) & 0xFF,
	(USBD_CONFIGDESC_LENGTH >> 8) & 0xFF,
	USBD_INTERFACE_COUNT,		// bNumInterfaces
	0x01,		// bConfigurationValue: Configuration value
	0x00,		// iConfiguration: Index of string descriptor describing the configuration
	0xc0,		// bmAttributes: bus powered
	0x32,	// MaxPower 500 mA

	/* DFU, Length: 9 */
	0x09,        //   bLength
	0x04,        //   bDescriptorType (Interface)
	0x00,        //   bInterfaceNumber 0
	0x00,        //   bInterfaceNumber 0
	0x00,        //   bInterfaceNumber 0
	0xFE,        //   bInterfaceClass
	0x01,        //   bInterfaceSubClass
	0x02,        //   bInterfaceProtocol
	0x04,        //   iInterface (String Index)
	
	0x09, 
	0x21, 
	0x0b, 
	0xff, 
	0x00,
	0x00, 0x04,		// block size
	0x10,
	0x01,
};

static const uint8_t WINUSB_Descriptor[] =
{
	(40 >> 0) & 0xFF,
	(40 >> 8) & 0xFF,
	0x00, 0x00,
	0x00, 0x01,
	0x04, 0x00,
	0x1,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
	
	0x0,
	0x1, 
	'W', 'I', 'N', 'U', 'S', 'B', 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
};

#define BOS_NUMBER			1
#define DFU_BOS_DESC_LENGTH	24
#define BOS_DESC_LENGTH		(5 + DFU_BOS_DESC_LENGTH)
static const uint8_t USB_BOSDescriptor[] =
{
	0x05,
	0x0F,
	(BOS_DESC_LENGTH >> 0) & 0xFF,
	(BOS_DESC_LENGTH >> 8) & 0xFF,
	BOS_NUMBER,

	DFU_BOS_DESC_LENGTH,
	0x10,
	0x05,
	0x00,
	0x38, 0xB6, 0x08, 0x34,
	0xA9, 0x09, 0xA0, 0x47,
	0x8B, 0xFD, 0xA0, 0x76,
	0x88, 0x15, 0xB6, 0x65,
	(0x0100 >> 0) & 0xFF,
	(0x0100 >> 8) & 0xFF,
	0x01,
	0x01,
};

#define WEB_URL		"vllogic.github.io/vllink_lite/"
static const uint8_t webusb_url_descriptor[] =
{
	3 + sizeof(WEB_URL) - 1,
	3,	// dt url
	1,	// https
	0x76, 0x6C, 0x6C, 0x6F, 0x67, 0x69, 0x63, 0x2E, 0x67, 0x69, 0x74, 0x68,
	0x75, 0x62, 0x2E, 0x69, 0x6F, 0x2F, 0x76, 0x6C, 0x6C, 0x69, 0x6E, 0x6B,
	0x5F, 0x6C, 0x69, 0x74, 0x65, 0x2F,
};

static const uint8_t USB_StringLangID[] =
{
	4,
	USB_DT_STRING,
	0x09,
	0x04
};

static const uint8_t USB_StringVendor[] =
{
	16,
	USB_DT_STRING,
	'V', 0, 'l', 0, 'l', 0, 'o', 0, 'g', 0, 'i', 0, 'c', 0,
};

static const uint8_t USB_StringProduct[] =
{
	32,
	USB_DT_STRING,
	'V', 0, 'l', 0, 'l', 0, 'i', 0, 'n', 0, 'k', 0, ' ', 0, 'L', 0,
	'i', 0, 't', 0, 'e', 0, ' ', 0,	'D', 0, 'F', 0, 'U', 0,
};

static uint8_t USB_StringSerial[50] =
{
	50,
	USB_DT_STRING,
	'0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0,
	'0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0,
	'0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0,
};

static const uint8_t DFU_StringFunc[] =
{
	8,
	USB_DT_STRING,
	'D', 0, 'F', 0, 'U', 0,
};

static const uint8_t BOS_StringFunc[] =
{
	18,
	USB_DT_STRING,
	'M', 0, 'S', 0, 'F', 0, 'T', 0, '1', 0, '0', 0, '0', 0, 0x21, 0,
};

static const struct vsfusbd_desc_filter_t USB_descriptors[] =
{
	VSFUSBD_DESC_DEVICE(0, USB_DeviceDescriptor, sizeof(USB_DeviceDescriptor)),
	VSFUSBD_DESC_CONFIG(0, 0, USB_ConfigDescriptor, sizeof(USB_ConfigDescriptor)),
	VSFUSBD_BOS_DESC(0, 0, USB_BOSDescriptor, sizeof(USB_BOSDescriptor)),
	VSFUSBD_DESC_STRING(0, 0, USB_StringLangID, sizeof(USB_StringLangID)),
	VSFUSBD_DESC_STRING(0x0409, 1, USB_StringVendor, sizeof(USB_StringVendor)),
	VSFUSBD_DESC_STRING(0x0409, 2, USB_StringProduct, sizeof(USB_StringProduct)),
	VSFUSBD_DESC_STRING(0x0409, 3, USB_StringSerial, sizeof(USB_StringSerial)),
	VSFUSBD_DESC_STRING(0x0409, 4, DFU_StringFunc, sizeof(DFU_StringFunc)),
	VSFUSBD_DESC_STRING(0x0000, 0xee, BOS_StringFunc, sizeof(BOS_StringFunc)),
	VSFUSBD_DESC_NULL
};

