#include "vsf.h"
#include "vsfhal_core.h"
#include "vsfhal_usb.h"

#define GD32F3X0_USBD_EP_NUM					(6 + 2)
#define EP_NUM_MAX								4
#define RX_FIFO_FS_SIZE							(64 * 4)
#define EP0_TX_FIFO_FS_SIZE						(64 * 2)
#define EP1_TX_FIFO_FS_SIZE						(64 * 2)
#define EP2_TX_FIFO_FS_SIZE						(64 * 2)
#define EP3_TX_FIFO_FS_SIZE						(64 * 2)

#define USB_CORE_SPEED_HIGH                     0U    /* USB core speed is high-speed */
#define USB_CORE_SPEED_FULL                     1U    /* USB core speed is full-speed */

#define USBFS_MAX_PACKET_SIZE                   64U   /* USBFS max packet size */
#define USBFS_MAX_HOST_CHANNELCOUNT             8U    /* USBFS host channel count */
#define USBFS_MAX_DEV_EPCOUNT                   4U    /* USBFS device endpoint count */
#define USBFS_MAX_FIFO_WORDLEN                  320U  /* USBFS max fifo size in words */

#define USBHS_MAX_PACKET_SIZE                   512U  /* USBHS max packet size */
#define USBHS_MAX_HOST_CHANNELCOUNT             12U   /* USBHS host channel count */
#define USBHS_MAX_DEV_EPCOUNT                   6U    /* USBHS device endpoint count */
#define USBHS_MAX_FIFO_WORDLEN                  1280U /* USBHS max fifo size in words */

#define USB_CORE_ULPI_PHY                       1U    /* USB core use external ULPI PHY */
#define USB_CORE_EMBEDDED_PHY                   2U    /* USB core use embedded PHY */

#define DSTAT_ENUMSPD_HS_PHY_30MHZ_OR_60MHZ     0U    /* USB enumerate speed use high-speed PHY clock in 30MHz or 60MHz */
#define DSTAT_ENUMSPD_FS_PHY_30MHZ_OR_60MHZ     1U    /* USB enumerate speed use full-speed PHY clock in 30MHz or 60MHz */
#define DSTAT_ENUMSPD_LS_PHY_6MHZ               2U    /* USB enumerate speed use low-speed PHY clock in 6MHz */
#define DSTAT_ENUMSPD_FS_PHY_48MHZ              3U    /* USB enumerate speed use full-speed PHY clock in 48MHz */

#define GRSTATR_RPCKST_IN                       2U    /* IN data packet received */
#define GRSTATR_RPCKST_IN_XFER_COMP             3U    /* IN transfer completed (generates an interrupt if poped) */
#define GRSTATR_RPCKST_DATA_TOGGLE_ERR          5U    /* data toggle error (generates an interrupt if poped) */
#define GRSTATR_RPCKST_CH_HALTED                7U    /* channel halted (generates an interrupt if poped) */

#define DEVICE_MODE                             0U    /* USB core in device mode */
#define HOST_MODE                               1U    /* USB core in host mode */
#define OTG_MODE                                2U    /* USB core in OTG mode */

#define USB_EPTYPE_CTRL                         0U    /* USB control endpoint type */
#define USB_EPTYPE_ISOC                         1U    /* USB synchronous endpoint type */
#define USB_EPTYPE_BULK                         2U    /* USB bulk endpoint type */
#define USB_EPTYPE_INTR                         3U    /* USB interrupt endpoint type */
#define USB_EPTYPE_MASK                         3U    /* USB endpoint type mask */

#define RXSTAT_GOUT_NAK                         1U    /* global OUT NAK (triggers an interrupt) */
#define RXSTAT_DATA_UPDT                        2U    /* OUT data packet received */
#define RXSTAT_XFER_COMP                        3U    /* OUT transfer completed (triggers an interrupt) */
#define RXSTAT_SETUP_COMP                       4U    /* SETUP transaction completed (triggers an interrupt) */
#define RXSTAT_SETUP_UPDT                       6U    /* SETUP data packet received */

#define DPID_DATA0                              0U    /* device endpoint data PID is DATA0 */
#define DPID_DATA1                              2U    /* device endpoint data PID is DATA1 */
#define DPID_DATA2                              1U    /* device endpoint data PID is DATA2 */
#define DPID_MDATA                              3U    /* device endpoint data PID is MDATA */

#define HC_PID_DATA0                            0U    /* host channel data PID is DATA0 */
#define HC_PID_DATA2                            1U    /* host channel data PID is DATA2 */
#define HC_PID_DATA1                            2U    /* host channel data PID is DATA1 */
#define HC_PID_SETUP                            3U    /* host channel data PID is SETUP */

#define HPRT_PRTSPD_HIGH_SPEED                  0U    /* host port speed use high speed */
#define HPRT_PRTSPD_FULL_SPEED                  1U    /* host port speed use full speed */
#define HPRT_PRTSPD_LOW_SPEED                   2U    /* host port speed use low speed */

#define HCTLR_30_60_MHZ                         0U    /* USB PHY(ULPI) clock is 60MHz */
#define HCTLR_48_MHZ                            1U    /* USB PHY(embedded full-speed) clock is 48MHz */
#define HCTLR_6_MHZ                             2U    /* USB PHY(embedded low-speed) clock is 6MHz */

#define HCCHAR_CTRL                             0U    /* control channel type */
#define HCCHAR_ISOC                             1U    /* synchronous channel type */
#define HCCHAR_BULK                             2U    /* bulk channel type */
#define HCCHAR_INTR                             3U    /* interrupt channel type */

const uint8_t vsfhal_usbd_ep_num = GD32F3X0_USBD_EP_NUM;
struct vsfhal_usbd_callback_t vsfhal_usbd_callback;
static uint32_t out_buff[RX_FIFO_FS_SIZE / 4];

struct out_ep_t
{
	uint32_t *buf;
	uint16_t packet_size;
	uint16_t max_packet_size;
} static out_ep[EP_NUM_MAX];

vsf_err_t vsfhal_usbd_init(int32_t int_priority)
{
	struct vsfhal_info_t *vsfhal_info;
	
	if ((RCU_AHBEN & RCU_AHBEN_USBFS) && (USB_GUSBCS & GUSBCS_FDM))
		return VSFERR_NONE;
		
	memset(out_ep, 0, sizeof(out_ep));

	if (vsfhal_core_get_info(&vsfhal_info))
		return VSFERR_FAIL;
	if (vsfhal_info->clk_enable & GD32F3X0_CLK_HSI48M)
	{
		RCU_ADDAPB1EN |= RCU_ADDAPB1EN_CTCEN;
		CTC_CTL1 = (0x2ul << 28) | (0x1cul << 16) | (48000 - 1);
		CTC_CTL0 |= CTC_CTL0_AUTOTRIM | CTC_CTL0_CNTEN;

		RCU_ADDCTL |= RCU_ADDCTL_CK48MSEL;
	}
	else if (vsfhal_info->clk_enable & GD32F3X0_CLK_PLL)
	{
		switch (vsfhal_info->pll_freq_hz)
		{
		case 48000000:
			RCU_CFG0 &= ~RCU_CFG0_USBFSPSC;
			RCU_CFG2 &= ~RCU_CFG2_USBFSPSC2;
			RCU_CFG0 |= 0x1ul << 22;
			break;
		case 72000000:
			RCU_CFG0 &= ~RCU_CFG0_USBFSPSC;
			RCU_CFG2 &= ~RCU_CFG2_USBFSPSC2;
			break;
		case 96000000:
			RCU_CFG0 &= ~RCU_CFG0_USBFSPSC;
			RCU_CFG2 &= ~RCU_CFG2_USBFSPSC2;
			RCU_CFG0 |= 0x3ul << 22;
			break;
		case 120000000:
			RCU_CFG0 &= ~RCU_CFG0_USBFSPSC;
			RCU_CFG2 &= ~RCU_CFG2_USBFSPSC2;
			RCU_CFG0 |= 0x2ul << 22;
			break;
		case 144000000:
			RCU_CFG0 &= ~RCU_CFG0_USBFSPSC;
			RCU_CFG2 |= RCU_CFG2_USBFSPSC2;
			break;
		case 168000000:
			RCU_CFG0 |= RCU_CFG0_USBFSPSC;
			RCU_CFG2 |= RCU_CFG2_USBFSPSC2;
			break;
		default:
			return VSFERR_FAIL;
		}
		RCU_ADDCTL &= ~RCU_ADDCTL_CK48MSEL;
	}
	else
		return VSFERR_FAIL;
	RCU_AHBEN |= RCU_AHBEN_USBFS;

	if (int_priority >= 0)
	{
		NVIC_SetPriority(USBFS_IRQn, int_priority);
		NVIC_EnableIRQ(USBFS_IRQn);
	}
	
	/* disable USB global interrupt */
	USB_GAHBCS &= ~GAHBCS_GINTEN;

	/* soft reset the core */
	USB_GRSTCTL |= GRSTCTL_CSRST;
	while (USB_GRSTCTL & GRSTCTL_CSRST);	
	__ASM("NOP");
	__ASM("NOP");
	__ASM("NOP");
	__ASM("NOP");
	__ASM("NOP");
	__ASM("NOP");
	__ASM("NOP");
	__ASM("NOP");
	
	/* active the transceiver and enable vbus sensing */
	USB_GCCFG |= GCCFG_PWRON | GCCFG_VBUSACEN | GCCFG_VBUSBCEN;
	
	if (vsfhal_info->clk_enable & GD32F3X0_CLK_HSI48M)
		USB_GCCFG |= GCCFG_SOFOEN;
	
	/* set Tx FIFO empty level to empty mode */
	USB_GAHBCS = (USB_GAHBCS & ~GAHBCS_TXFTH) | TXFIFO_EMPTY;

	// set device mode
	USB_GUSBCS = (USB_GUSBCS & ~GUSBCS_FHM) | GUSBCS_FDM;

	// set full speed PHY, config periodic frmae interval to default
	USB_DCFG = (USB_DCFG & ~(DCFG_DS | DCFG_EOPFT)) | USB_SPEED_INP_FULL | FRAME_INTERVAL_80;

	// disconnect
	USB_DCTL |= DCTL_SD;
	
	USB_GCCFG |= GCCFG_VBUSIG;
	
	USB_PWRCLKCTL = 0;

	USB_DOEPINTEN = DOEPINTEN_STPFEN | DOEPINTEN_TFEN | DOEPINTEN_EPDISEN;
	USB_DIEPINTEN = DIEPINTEN_TFEN | DIEPINTEN_CITOEN | DIEPINTEN_EPDISEN;
	USB_DAEPINT = 0xffffffff;
	USB_DAEPINTEN = 0;

	USB_GINTF = 0xbfffffff;
	USB_GOTGINTF = 0xffffffff;

	USB_GINTEN = GINTEN_RSTIE | GINTEN_ENUMFIE | GINTEN_IEPIE | GINTEN_OEPIE |
			GINTEN_ISOONCIE | GINTEN_ISOINCIE | GINTEN_RXFNEIE;
	// USB_GINTEN |= GINTEN_SOFIE;
	// USB_GINTEN |= GINTEN_WKUPIE | GINTEN_SPIE;
	// USB_GINTEN |= GINTEN_OTGIE | GINTEN_SESIE | GINTEN_CIDPSCIE;

	USB_GAHBCS |= GAHBCS_GINTEN;
	
	return VSFERR_NONE;
}

vsf_err_t vsfhal_usbd_fini(void)
{
	RCU_AHBEN &= ~RCU_AHBEN_USBFS;
	return VSFERR_NONE;
}

void USB_Istr(void);
vsf_err_t vsfhal_usbd_poll(void)
{
	USB_Istr();
	return VSFERR_NONE;
}

vsf_err_t vsfhal_usbd_reset(void)
{
	return VSFERR_NONE;
}

vsf_err_t vsfhal_usbd_connect(void)
{
	USB_DCTL &= ~DCTL_SD;
	return VSFERR_NONE;
}

vsf_err_t vsfhal_usbd_disconnect(void)
{
	USB_DCTL |= DCTL_SD;
	return VSFERR_NONE;
}

vsf_err_t vsfhal_usbd_set_address(uint8_t address)
{
	uint32_t v = address & 0x7f;
	v <<= 4;
	USB_DCFG = (USB_DCFG & ~DCFG_DAR) | v;
	return VSFERR_NONE;
}

uint8_t vsfhal_usbd_get_address(void)
{
	uint32_t v = USB_DCFG >> 4;
	return v & 0x7f;
}

vsf_err_t vsfhal_usbd_wakeup(void)
{
	return VSFERR_NONE;
}

uint32_t vsfhal_usbd_get_frame_number(void)
{
	return (USB_DSTAT >> 8) & 0x3fff;
}

vsf_err_t vsfhal_usbd_get_setup(uint8_t *buffer)
{
	if (out_ep[0].packet_size == 8)
	{
		memcpy(buffer, out_ep[0].buf, 8);
		out_ep[0].packet_size = 0;
		return VSFERR_NONE;
	}
	else
		return VSFERR_FAIL;
}

vsf_err_t vsfhal_usbd_prepare_buffer(void)
{
	USB_GRFLEN = (USB_GRFLEN & ~GRFLEN_RXFD) | (RX_FIFO_FS_SIZE / 4);
	USB_DIEP0TFLEN = ((uint32_t)(EP0_TX_FIFO_FS_SIZE / 4) << 16) + (RX_FIFO_FS_SIZE / 4);
	USB_DIEPxTFLEN(1) = ((uint32_t)(EP1_TX_FIFO_FS_SIZE / 4) << 16) + ((RX_FIFO_FS_SIZE + EP0_TX_FIFO_FS_SIZE) / 4);
	USB_DIEPxTFLEN(2) = ((uint32_t)(EP2_TX_FIFO_FS_SIZE / 4) << 16) + ((RX_FIFO_FS_SIZE + EP0_TX_FIFO_FS_SIZE + EP1_TX_FIFO_FS_SIZE) / 4);
	USB_DIEPxTFLEN(3) = ((uint32_t)(EP3_TX_FIFO_FS_SIZE / 4) << 16) + ((RX_FIFO_FS_SIZE + EP0_TX_FIFO_FS_SIZE + EP1_TX_FIFO_FS_SIZE + EP2_TX_FIFO_FS_SIZE) / 4);

	USB_GRSTCTL = GRSTCTL_TXFNUM | GRSTCTL_TXFF | GRSTCTL_RXFF;
	while (USB_GRSTCTL & (GRSTCTL_TXFF | GRSTCTL_RXFF));
	//fifo_addr = RX_FIFO_FS_SIZE + EP0_TX_FIFO_FS_SIZE;
	return VSFERR_NONE;
}

vsf_err_t vsfhal_usbd_ep_reset(uint8_t idx)
{
	return VSFERR_NONE;
}

vsf_err_t vsfhal_usbd_ep_set_type(uint8_t idx, enum vsfhal_usbd_eptype_t type)
{
	if (idx >= EP_NUM_MAX)
		return VSFERR_NOT_ENOUGH_RESOURCES;

	if (idx == 0)
	{
		USB_DIEP0CTL |= DEP0CTL_EPACT;
		USB_DOEP0CTL |= DEP0CTL_EPACT;
	}
	else
	{
		switch (type)
		{
		case USB_EP_TYPE_CONTROL:
			if (USB_DIEPxCTL(idx) & DEPCTL_MPL)
				USB_DIEPxCTL(idx) = (USB_DIEPxCTL(idx) & ~DEPCTL_EPTYPE) | DEPCTL_EPACT;
			if (USB_DOEPxCTL(idx) & DEPCTL_MPL)
				USB_DOEPxCTL(idx) = (USB_DOEPxCTL(idx) & ~DEPCTL_EPTYPE) | DEPCTL_EPACT;
			break;
		case USB_EP_TYPE_INTERRUPT:
			if (USB_DIEPxCTL(idx) & DEPCTL_MPL)
				USB_DIEPxCTL(idx) = (USB_DIEPxCTL(idx) & ~DEPCTL_EPTYPE) | (0x3ul << 18) | DEPCTL_EPACT;
			if (USB_DOEPxCTL(idx) & DEPCTL_MPL)
				USB_DOEPxCTL(idx) = (USB_DOEPxCTL(idx) & ~DEPCTL_EPTYPE) | (0x3ul << 18) | DEPCTL_EPACT;
			break;
		case USB_EP_TYPE_BULK:
			if (USB_DIEPxCTL(idx) & DEPCTL_MPL)
				USB_DIEPxCTL(idx) = (USB_DIEPxCTL(idx) & ~DEPCTL_EPTYPE) | (0x2ul << 18) | DEPCTL_EPACT;
			if (USB_DOEPxCTL(idx) & DEPCTL_MPL)
				USB_DOEPxCTL(idx) = (USB_DOEPxCTL(idx) & ~DEPCTL_EPTYPE) | (0x2ul << 18) | DEPCTL_EPACT;
			break;
		case USB_EP_TYPE_ISO:
			if (USB_DIEPxCTL(idx) & DEPCTL_MPL)
				USB_DIEPxCTL(idx) = (USB_DIEPxCTL(idx) & ~DEPCTL_EPTYPE) | (0x1ul << 18) | DEPCTL_EPACT;
			if (USB_DOEPxCTL(idx) & DEPCTL_MPL)
				USB_DOEPxCTL(idx) = (USB_DOEPxCTL(idx) & ~DEPCTL_EPTYPE) | (0x1ul << 18) | DEPCTL_EPACT;
			break;
		}
	}
	return VSFERR_NONE;
}

vsf_err_t vsfhal_usbd_ep_set_IN_dbuffer(uint8_t idx)
{
	return VSFERR_NONE;
}

bool vsfhal_usbd_ep_is_IN_dbuffer(uint8_t idx)
{
	return false;
}

vsf_err_t vsfhal_usbd_ep_switch_IN_buffer(uint8_t idx)
{
	return VSFERR_NONE;
}

vsf_err_t vsfhal_usbd_ep_set_IN_epsize(uint8_t idx, uint16_t epsize)
{
	if (idx >= EP_NUM_MAX)
		return VSFERR_NOT_ENOUGH_RESOURCES;

	if (idx == 0)
	{
		switch (epsize)
		{
		case 64:
			USB_DIEP0CTL = USB_DIEP0CTL & ~DEPCTL_MPL;
			break;
		case 32:
			USB_DIEP0CTL = (USB_DIEP0CTL & ~DEPCTL_MPL) | 1;
			break;
		case 16:
			USB_DIEP0CTL = (USB_DIEP0CTL & ~DEPCTL_MPL) | 2;
			break;
		case 8:
			USB_DIEP0CTL = (USB_DIEP0CTL & ~DEPCTL_MPL) | 3;
			break;
		}
	}
	else
	{
#if 0
		uint32_t fifo_depth = max((epsize + 3) & 0xfffffffc, 16);
		USB_DIEPxTFLEN(idx) = (fifo_depth << 16) | fifo_addr;
		fifo_addr += fifo_depth;
		USB_DIEPxCTL(idx) = epsize | ((uint32_t)idx << 22);	
#else
		//USB_DIEPxTFLEN(idx) = ((uint32_t)epsize << 16) | fifo_addr;
		//fifo_addr += epsize;
		USB_DIEPxCTL(idx) = epsize | ((uint32_t)idx << 22);
#endif
	}
	USB_DAEPINTEN |= 0x1ul << idx;
	
	return VSFERR_NONE;
}

uint16_t vsfhal_usbd_ep_get_IN_epsize(uint8_t idx)
{
	if (idx == 0)
	{
		switch (USB_DIEP0CTL & DEP0CTL_MPL)
		{
		case 0:
			return 64;
		case 1:
			return 32;
		case 2:
			return 16;
		case 3:
			return 8;
		}
	}
	else if (idx < EP_NUM_MAX)
		return USB_DIEPxCTL(idx) & DEPCTL_MPL;

	return 0;
}

vsf_err_t vsfhal_usbd_ep_set_IN_stall(uint8_t idx)
{
	if (idx >= EP_NUM_MAX)
		return VSFERR_NOT_ENOUGH_RESOURCES;
	
	if (idx == 0)
		USB_DIEP0CTL |= DEP0CTL_STALL;
	else
		USB_DIEPxCTL(idx) |= DEPCTL_STALL;	
	
	return VSFERR_NONE;
}

vsf_err_t vsfhal_usbd_ep_clear_IN_stall(uint8_t idx)
{
	if (idx >= EP_NUM_MAX)
		return VSFERR_NOT_ENOUGH_RESOURCES;
	
	if (idx == 0)
		USB_DIEP0CTL &= ~DEP0CTL_STALL;
	else
		USB_DIEPxCTL(idx) &= ~DEPCTL_STALL;	
	
	return VSFERR_NONE;
}

bool vsfhal_usbd_ep_is_IN_stall(uint8_t idx)
{
	if (idx >= EP_NUM_MAX)
		return false;

	if (idx == 0)
		return USB_DIEP0CTL & DEP0CTL_STALL ? true : false;
	else
		return USB_DIEPxCTL(idx) & DEPCTL_STALL ? true : false;	
}

vsf_err_t vsfhal_usbd_ep_reset_IN_toggle(uint8_t idx)
{
	if (!idx || (idx >= EP_NUM_MAX))
		return VSFERR_NOT_ENOUGH_RESOURCES;
	
	USB_DIEPxCTL(idx) &= ~DEPCTL_DPID;	
	
	return VSFERR_NONE;
}

vsf_err_t vsfhal_usbd_ep_toggle_IN_toggle(uint8_t idx)
{
	if (!idx || (idx >= EP_NUM_MAX))
		return VSFERR_NOT_ENOUGH_RESOURCES;
	
	if (USB_DIEPxCTL(idx) & DEPCTL_DPID)
		USB_DIEPxCTL(idx) &= ~DEPCTL_DPID;
	else
		USB_DIEPxCTL(idx) |= DEPCTL_DPID;
	
	return VSFERR_NONE;
}

vsf_err_t vsfhal_usbd_ep_set_IN_count(uint8_t idx, uint16_t size)
{
	if (idx >= EP_NUM_MAX)
		return VSFERR_NOT_ENOUGH_RESOURCES;
	
	if (!size)
	{
		USB_DIEPxLEN(idx) = 0x1ul << 19;
		USB_DIEPxCTL(idx) |= DEPCTL_EPEN | DEPCTL_CNAK;
	}
	return VSFERR_NONE;
}

vsf_err_t vsfhal_usbd_ep_write_IN_buffer(uint8_t idx, uint8_t *buffer,
		uint16_t size)
{
	uint16_t i;

	if (idx >= EP_NUM_MAX)
		return VSFERR_NOT_ENOUGH_RESOURCES;

	USB_DIEPxLEN(idx) = (0x1ul << 19) | size;
	USB_DIEPxCTL(idx) |= DEPCTL_EPEN | DEPCTL_CNAK;

	for (i = 0; i < size; i += 4)
	{
		*USB_FIFO(idx) = *(uint32_t *)buffer;
		buffer += 4;
	}
	USB_DIEPFEINTEN |= 0x1ul << idx;

	return VSFERR_NONE;
}

vsf_err_t vsfhal_usbd_ep_set_OUT_dbuffer(uint8_t idx)
{
	return VSFERR_NONE;
}

bool vsfhal_usbd_ep_is_OUT_dbuffer(uint8_t idx)
{
	return false;
}

vsf_err_t vsfhal_usbd_ep_switch_OUT_buffer(uint8_t idx)
{
	return VSFERR_NONE;
}

vsf_err_t vsfhal_usbd_ep_set_OUT_epsize(uint8_t idx, uint16_t epsize)
{
	if (idx >= EP_NUM_MAX)
		return VSFERR_NOT_ENOUGH_RESOURCES;

	if (idx == 0)
	{
		out_ep[0].buf = out_buff;
		out_ep[0].packet_size = 0;
		out_ep[0].max_packet_size = epsize;
		switch (epsize)
		{
		case 64:
			USB_DOEP0CTL = USB_DOEP0CTL & ~DEPCTL_MPL;
			break;
		case 32:
			USB_DOEP0CTL = (USB_DOEP0CTL & ~DEPCTL_MPL) | 1;
			break;
		case 16:
			USB_DOEP0CTL = (USB_DOEP0CTL & ~DEPCTL_MPL) | 2;
			break;
		case 8:
			USB_DOEP0CTL = (USB_DOEP0CTL & ~DEPCTL_MPL) | 3;
			break;
		}
	}
	else
	{
		uint16_t out_buff_free_size = (uint32_t)out_buff + sizeof(out_buff) -
				(uint32_t)(out_ep[idx - 1].buf);
		if (out_buff_free_size < epsize)
			return VSFERR_NOT_ENOUGH_RESOURCES;
		
		out_ep[idx].buf = out_ep[idx - 1].buf + out_ep[idx - 1].max_packet_size / 4;
		out_ep[idx].packet_size = 0;
		out_ep[idx].max_packet_size = epsize;
		
		USB_DOEPxCTL(idx) = (USB_DOEPxCTL(idx) & ~DEPCTL_MPL) | epsize;
	}
	USB_DAEPINTEN |= 0x10000ul << idx;

	return VSFERR_NONE;
}

uint16_t vsfhal_usbd_ep_get_OUT_epsize(uint8_t idx)
{
	if (idx < EP_NUM_MAX)
		return out_ep[idx].max_packet_size;
	return 0;
}

vsf_err_t vsfhal_usbd_ep_set_OUT_stall(uint8_t idx)
{

	return VSFERR_BUG;
}

vsf_err_t vsfhal_usbd_ep_clear_OUT_stall(uint8_t idx)
{

	return VSFERR_BUG;
}

bool vsfhal_usbd_ep_is_OUT_stall(uint8_t idx)
{

	return VSFERR_BUG;
}

vsf_err_t vsfhal_usbd_ep_reset_OUT_toggle(uint8_t idx)
{

	return VSFERR_BUG;
}

vsf_err_t vsfhal_usbd_ep_toggle_OUT_toggle(uint8_t idx)
{
	return VSFERR_NOT_SUPPORT;
}

uint16_t vsfhal_usbd_ep_get_OUT_count(uint8_t idx)
{
	if (idx < EP_NUM_MAX)
		return out_ep[idx].packet_size;
	return 0;
}

vsf_err_t vsfhal_usbd_ep_read_OUT_buffer(uint8_t idx, uint8_t *buffer, uint16_t size)
{
	if (idx < EP_NUM_MAX)
	{
		size = min(out_ep[idx].packet_size, size);
		if (size && buffer)
		{
			memcpy(buffer, out_ep[idx].buf, size);
			out_ep[idx].packet_size -= size;
			return VSFERR_NONE;
		}
	}
	return VSFERR_BUG;
}

vsf_err_t vsfhal_usbd_ep_enable_OUT(uint8_t idx)
{
	if (idx >= EP_NUM_MAX)
		return VSFERR_NOT_ENOUGH_RESOURCES;
	
	USB_DOEPxLEN(idx) = (USB_DOEPxLEN(idx) & ~(DEPLEN_TLEN | DEPLEN_PCNT)) |
			(0x1ul << 19) | out_ep[idx].max_packet_size;
	USB_DOEPxCTL(idx) |= DEPCTL_EPEN | DEPCTL_CNAK;	
	return VSFERR_NONE;
}

static void vsfhal_usbd_cb(enum vsfhal_usbd_evt_t evt, uint32_t value)
{
	if (vsfhal_usbd_callback.on_event != NULL)
		vsfhal_usbd_callback.on_event(vsfhal_usbd_callback.param, evt, value);
}

static void ep_read(uint8_t ep_num, uint16_t size)
{
	size = min(out_ep[ep_num].max_packet_size, size);

	if (size)
	{
		uint16_t i;
		uint32_t *dest = out_ep[ep_num].buf;
		out_ep[ep_num].packet_size = size;
		size = (size + 3) / 4;
		for (i = 0; i < size; i++)
			dest[i] = *USB_FIFO(0);
	}
}

static void USB_Istr(void)
{
	uint32_t status;

	if (USB_CURRENT_MODE_GET() != DEVICE_MODE)
		return;
	
	status = USB_GINTF;
	status &= USB_GINTEN;

	// mode mismatch interrupt
	if (status & GINTF_MFIF)
		USB_GINTF = GINTF_MFIF;

	// USB reset interrupt
	if (status & GINTF_RST)
	{
		vsfhal_usbd_cb(VSFHAL_USBD_ON_RESET, 0);
		USB_GINTF = GINTF_RST;
	}

	// enumeration has been finished interrupt
	if (status & GINTF_ENUMF)	
	{
		uint8_t speed = (USB_DSTAT & DSTAT_ES) >> 1;
		USB_GUSBCS &= ~GUSBCS_UTT;
		switch (speed)
		{
		case DSTAT_ENUMSPD_HS_PHY_30MHZ_OR_60MHZ:
			USB_DOEP0CTL |= EP0MPL_64;
			USB_GUSBCS |= 0x09U << 10;
			break;
		case DSTAT_ENUMSPD_FS_PHY_30MHZ_OR_60MHZ:
			USB_DOEP0CTL |= EP0MPL_64;
			USB_GUSBCS |= 0x05U << 10;
			break;
		case DSTAT_ENUMSPD_FS_PHY_48MHZ:
			USB_DOEP0CTL |= EP0MPL_64;
			USB_GUSBCS |= 0x05U << 10;
			break;
		case DSTAT_ENUMSPD_LS_PHY_6MHZ:
			USB_DOEP0CTL |= EP0MPL_8;
			USB_GUSBCS |= 0x05U << 10;
			break;
		}
		USB_DCTL |= DCTL_CGINAK;
		USB_GINTF = GINTF_ENUMF;
	}

	// early suspend interrupt
	if (status & GINTF_ESP)
	{
		USB_GINTEN &= ~GINTEN_ESPIE;
		USB_GINTF = GINTF_ESP;
	}

	// suspend interrupt
	if (status & GINTF_SP)
	{
		vsfhal_usbd_cb(VSFHAL_USBD_ON_SUSPEND, 0);
		USB_GINTF = GINTF_SP;
	}

	// wakeup interrupt
	if (status & GINTF_WKUPIF)
	{
		vsfhal_usbd_cb(VSFHAL_USBD_ON_RESUME, 0);
		USB_GINTF = GINTF_WKUPIF;
	}

	// start of frame interrupt
	if (status & GINTF_SOF)
	{
		vsfhal_usbd_cb(VSFHAL_USBD_ON_SOF, 0);
		USB_GINTF = GINTF_SOF;
	}

	// OUT endpoints interrupts
	if (status & GINTF_OEPIF)	
	{
		uint8_t ep_num = 0;
		uint32_t ep_int = USB_DAEPINT;
		ep_int = (ep_int & USB_DAEPINTEN) >> 16;

		while (ep_int)
		{
			if (ep_int & 0x1)
			{
				uint32_t int_status = USB_DOEPxINTF(ep_num);
				int_status &= USB_DOEPINTEN;
				
				// transfer complete interrupt
				if (int_status & DOEPINTF_TF)
				{
					vsfhal_usbd_cb(VSFHAL_USBD_ON_OUT, ep_num);
					USB_DOEPxINTF(ep_num) = DOEPINTF_TF;
				}

				// endpoint disable interrupt
				if (int_status & DOEPINTF_EPDIS)
					USB_DOEPxINTF(ep_num) = DOEPINTF_EPDIS;

				// setup phase finished interrupt (just for control endpoints)
				if (int_status & DOEPINTF_STPF)
				{
					// need update address immediately
					if (((out_ep[0].buf[0] & 0xff00ffff) == 0x00000500) &&
							(out_ep[0].buf[1] == 0x0))
						vsfhal_usbd_set_address(out_ep[0].buf[0] >> 16);
					
					vsfhal_usbd_cb(VSFHAL_USBD_ON_SETUP, 0);
					USB_DOEP0LEN |= 0x3ul <<  29;
					USB_DOEPxINTF(ep_num) = DOEPINTF_STPF;
				}

				// back to back setup packets received
				if (int_status & DOEPINTF_BTBSTP)
					USB_DOEPxINTF(ep_num) = DOEPINTF_BTBSTP;
			}
			ep_int >>= 1;
			ep_num ++;
		}
	}

	// IN endpoints interrupts
	if (status & GINTF_IEPIF)	
	{
		uint8_t ep_num = 0;
		uint32_t ep_int = USB_DAEPINT;
		ep_int = (ep_int & USB_DAEPINTEN) & 0xffff;
		
		while (ep_int)
		{
			if (ep_int & 0x1)
			{
				uint32_t int_status = USB_DIEPxINTF(ep_num);
				uint32_t int_msak = USB_DIEPINTEN;
				int_msak |= ((USB_DIEPFEINTEN >> ep_num) & 0x1) << 7;
				int_status &= int_msak;
				
				if (int_status & DIEPINTF_TF)
				{
					vsfhal_usbd_cb(VSFHAL_USBD_ON_IN, ep_num);
					USB_DIEPxINTF(ep_num) = DIEPINTF_TF;
				}
				
				if (int_status & DIEPINTF_EPDIS)
					USB_DIEPxINTF(ep_num) = DIEPINTF_EPDIS;
				
				if (int_status & DIEPINTF_CITO)
					USB_DIEPxINTF(ep_num) = DIEPINTF_CITO;
				
				if (int_status & DIEPINTF_IEPNE)
					USB_DIEPxINTF(ep_num) = DIEPINTF_IEPNE;
				
				if (int_status & DIEPINTF_TXFE)
				{
					USB_DIEPFEINTEN &= ~(0x1ul << ep_num);
					USB_DIEPxINTF(ep_num) = DIEPINTF_TXFE;
				}
			}
			ep_int >>= 1;
			ep_num ++;
		}
	}

	// reveive fifo not empty interrupt
	if (status & GINTF_RXFNEIF)	
	{
		uint8_t ep_num, pid;
		uint16_t size;
		uint32_t rx_status;
		
		USB_GINTEN &= ~GINTEN_RXFNEIE;
		
		rx_status = USB_GRSTATP;
		ep_num = rx_status & GRSTATRP_EPNUM;
		pid = (rx_status & GRSTATRP_DPID) >> 15;
		size = (rx_status & GRSTATRP_BCOUNT) >> 4;
		
		if (ep_num == 1)
			__ASM("NOP");
		
		switch ((rx_status & GRSTATRP_RPCKST) >> 17)
		{
		case RXSTAT_SETUP_UPDT:
			*(uint32_t *)0x5000081CU |= 0x00020000U;
			if (ep_num || (size != 8) || (pid != DPID_DATA0))
				break;
		case RXSTAT_DATA_UPDT:
			ep_read(ep_num, size);
			break;
		//case RXSTAT_GOUT_NAK:
		//case RXSTAT_SETUP_COMP:
		default:
			break;
		}

		USB_GINTEN |= GINTEN_RXFNEIE;
	}

#if 0
	// incomplete synchronization in transfer interrupt
	if (status & GINTF_ISOINCIF)	
	{

	}

	// incomplete synchronization out transfer interrupt
	if (status & GINTF_ISOONCIF)	
	{

	}
#endif

#if 0
	// session request interrupt
	if (status & GINTF_SESIF)	
	{
		// TODO
		USB_GINTF = GINTF_SESIF;
	}

	// OTG mode interrupt
	if (status & GINTF_OTGIF)	
	{
		// TODO
	}
#endif
}

ROOT void USBFS_IRQHandler(void)
{
	USB_Istr();
}

