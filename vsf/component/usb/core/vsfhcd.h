
#ifndef __VSFHCD_H_INCLUDED__
#define __VSFHCD_H_INCLUDED__

struct vsfhcd_device_t
{
	uint8_t devnum;
	uint8_t speed;		// full/low/high
	uint16_t toggle[2];	// one bit per endpoint
	void *priv;
};

#define URB_OK					VSFERR_NONE
#define URB_FAIL				VSFERR_FAIL
#define URB_PENDING				VSFERR_NOT_READY


/* ----------------------------------------------------------------------- */

/*
 * For various legacy reasons, Linux has a small cookie that's paired with
 * a struct usb_device to identify an endpoint queue.  Queue characteristics
 * are defined by the endpoint's descriptor.  This cookie is called a "pipe",
 * an unsigned int encoded as:
 *
 *  - direction:	bit 7		(0 = Host-to-Device [Out],
 *					 1 = Device-to-Host [In] ...
 *					like endpoint bEndpointAddress)
 *  - device address:	bits 8-14       ... bit positions known to uhci-hcd
 *  - endpoint:		bits 15-18      ... bit positions known to uhci-hcd
 *  - pipe type:	bits 30-31	(00 = isochronous, 01 = interrupt,
 *					 10 = control, 11 = bulk)
 *
 * Given the device address and endpoint descriptor, pipes are redundant.
 */

/* NOTE:  these are not the standard USB_ENDPOINT_XFER_* values!! */
/* (yet ... they're the values used by usbfs) */
#define PIPE_ISOCHRONOUS		0ul
#define PIPE_INTERRUPT			1ul
#define PIPE_CONTROL			2ul
#define PIPE_BULK				3ul

#define usb_pipein(pipe)		(((pipe) & USB_DIR_IN) >> 7)
#define usb_pipeout(pipe)		(!usb_pipein(pipe))

#define usb_pipedevice(pipe)	(((pipe) >> 8) & 0x7f)
#define usb_pipeendpoint(pipe)	(((pipe) >> 15) & 0xf)

#define usb_pipetype(pipe)		(((pipe) >> 30) & 3)
#define usb_pipeisoc(pipe)		(usb_pipetype((pipe)) == PIPE_ISOCHRONOUS)
#define usb_pipeint(pipe)		(usb_pipetype((pipe)) == PIPE_INTERRUPT)
#define usb_pipecontrol(pipe)	(usb_pipetype((pipe)) == PIPE_CONTROL)
#define usb_pipebulk(pipe)		(usb_pipetype((pipe)) == PIPE_BULK)

#define usb_pipespeed(pipe)		(((pipe) >> 26) & 3)
#define usb_pipeslow(pipe)		(usb_pipespeed(pipe) == USB_SPEED_LOW)

#define __create_pipe(dev, endpoint)	\
		(((dev)->devnum << 8) | ((endpoint) << 15) | ((dev->speed == USB_SPEED_LOW) ? (1UL << 26) : 0))

/* Create various pipes... */
#define usb_sndctrlpipe(dev, endpoint)	\
		((PIPE_CONTROL << 30) | __create_pipe((dev), (endpoint)))
#define usb_rcvctrlpipe(dev, endpoint)	\
		((PIPE_CONTROL << 30) | __create_pipe((dev), (endpoint)) | USB_DIR_IN)
#define usb_sndisocpipe(dev, endpoint)	\
		((PIPE_ISOCHRONOUS << 30) | __create_pipe((dev), (endpoint)))
#define usb_rcvisocpipe(dev, endpoint)	\
		((PIPE_ISOCHRONOUS << 30) | __create_pipe((dev), (endpoint)) | USB_DIR_IN)
#define usb_sndbulkpipe(dev, endpoint)	\
		((PIPE_BULK << 30) | __create_pipe((dev), (endpoint)))
#define usb_rcvbulkpipe(dev, endpoint)	\
		((PIPE_BULK << 30) | __create_pipe((dev), (endpoint)) | USB_DIR_IN)
#define usb_sndintpipe(dev, endpoint)	\
		((PIPE_INTERRUPT << 30) | __create_pipe((dev), (endpoint)))
#define usb_rcvintpipe(dev, endpoint)	\
		((PIPE_INTERRUPT << 30) | __create_pipe((dev), (endpoint)) | USB_DIR_IN)

#define default_pipe(dev)				((dev)->speed << 26)
#define usb_snddefctrl(dev)				\
		((PIPE_CONTROL << 30) | default_pipe(dev) | USB_DIR_OUT)
#define usb_rcvdefctrl(dev)				\
		((PIPE_CONTROL << 30) | default_pipe(dev) | USB_DIR_IN)
/*-------------------------------------------------------------------------*/

struct iso_packet_descriptor_t
{
	uint32_t offset;				/*!< Start offset in transfer buffer	*/
	uint32_t length;				/*!< Length in transfer buffer			*/
	uint32_t actual_length;			/*!< Actual transfer length				*/
	int32_t status;					/*!< Transfer status					*/
};

// urb is the interface between higher software and hcd
#define URB_DIR_IN				0x0200		// Transfer from device to host
#define URB_DIR_OUT				0
#define URB_DIR_MASK			URB_DIR_IN

// struct vsfhcd_urb_t.transfer_flags
#define USB_DISABLE_SPD			0x0001		// Short Packet Disable
#define USB_ISO_ASAP			0x0002
#define USB_ASYNC_UNLINK		0x0008
#define USB_QUEUE_BULK			0x0010
#define USB_NO_FSBR				0x0020
#define USB_ZERO_PACKET			0x0040		// Finish bulk OUTs always with zero length packet
#define USB_TIMEOUT_KILLED		0x1000		// only set by HCD!

#define URB_BUFFER_DYNALLOC		0x2000
#define URB_SHORT_NOT_OK		USB_DISABLE_SPD
#define URB_ZERO_PACKET			USB_ZERO_PACKET
#define URB_ISO_ASAP			USB_ISO_ASAP
#define URB_NO_INTERRUPT		0x0080		// HINT: no non-error interrupt needed

struct vsfhcd_urb_t
{
	struct vsfhcd_device_t *hcddev;
	uint32_t pipe;					/*!< pipe information						*/

	uint16_t packet_size;
	uint16_t transfer_flags;		/*!< USB_DISABLE_SPD | USB_ISO_ASAP | etc.	*/
	void *transfer_buffer;
	uint32_t transfer_length;
	uint32_t actual_length;

	struct usb_ctrlrequest_t setup_packet;

	int16_t status;					/*!< returned status				*/
	uint16_t interval;				/*!< polling interval (iso/irq only)*/
#if VSFHAL_HCD_ISO_SUPPORT
	uint32_t start_frame;			/*!< start frame (iso/irq only)		*/
	uint32_t number_of_packets;		/*!< number of packets (iso)		*/
	//uint32_t error_count;			/*!< number of errors (iso only)	*/
	struct iso_packet_descriptor_t iso_frame_desc[VSFHAL_HCD_ISO_PACKET_LIMIT];
#endif // VSFHAL_HCD_ISO_SUPPORT
	
	uint32_t timeout;
	struct vsfsm_t *notifier_sm;

	uint32_t priv[0];
};

struct vsfhcd_t;
struct vsfhcd_drv_t
{
	vsf_err_t (*init_thread)(struct vsfsm_pt_t *pt, vsfsm_evt_t evt);
	vsf_err_t (*fini)(struct vsfhcd_t *hcd);
	vsf_err_t (*suspend)(struct vsfhcd_t *hcd);
	vsf_err_t (*resume)(struct vsfhcd_t *hcd);
	vsf_err_t (*alloc_device)(struct vsfhcd_t *hcd, struct vsfhcd_device_t *hcddev);
	void (*free_device)(struct vsfhcd_t *hcd, struct vsfhcd_device_t *hcddev);
	struct vsfhcd_urb_t * (*alloc_urb)(struct vsfhcd_t *hcd);
	void (*free_urb)(struct vsfhcd_t *hcd, struct vsfhcd_urb_t *urb);
	vsf_err_t (*submit_urb)(struct vsfhcd_t *hcd, struct vsfhcd_urb_t *urb);
	vsf_err_t (*relink_urb)(struct vsfhcd_t *hcd, struct vsfhcd_urb_t *urb);
	int (*rh_control)(struct vsfhcd_t *hcd, struct vsfhcd_urb_t *urb);
};

struct vsfhcd_t
{
	const struct vsfhcd_drv_t *drv;
	void *param;
	void *priv;

	uint8_t rh_speed;
};

/* The D0/D1 toggle bits ... USE WITH CAUTION (they're almost hcd-internal) */
#define usb_gettoggle(dev, ep, out) (((dev)->toggle[out] >> (ep)) & 1)
#define	usb_dotoggle(dev, ep, out)  ((dev)->toggle[out] ^= (1 << (ep)))
#define usb_settoggle(dev, ep, out, bit) \
		((dev)->toggle[out] = ((dev)->toggle[out] & ~(1 << (ep))) | ((bit) << (ep)))

#endif	// __VSFHCD_H_INCLUDED__
