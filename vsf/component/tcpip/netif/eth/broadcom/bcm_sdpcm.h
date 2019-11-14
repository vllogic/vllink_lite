#ifndef __SDPCM_H_INCLUDED__
#define __SDPCM_H_INCLUDED__

struct bcm_wifi_t;
struct bcm_sdpcm_t
{
	struct vsfsm_t outthread_sm;
	struct vsfsm_pt_t outthread_pt;
	struct vsfsm_pt_t outtrans_pt;
	struct vsfip_buffer_t *curtxbuf;
	bool out_pending_for_credit;
	
	struct vsfsm_t inthread_sm;
	struct vsfsm_pt_t inthread_pt;
	struct vsfsm_pt_t intrans_pt;
	struct vsfsm_sem_t in_sem;
	struct vsfip_buffer_t *currxbuf;
	
	struct vsfsm_crit_t ioctrl_crit;
	struct vsfsm_sem_t ioctrl_sem;
	struct vsftimer_t ioctrl_to;
	uint16_t ioctrl_id;
	struct vsfip_buffer_t *ioctrl_reply;
	
	bool interrupted;
	bool quit;
	uint8_t sequence;
	uint8_t credit;
	uint8_t flowcontrol;
	
	bool async_pending;
	struct vsfsm_sem_t async_sem;
	struct vsfip_buffer_t *async_evt;
	
	uint16_t f2_size;
	struct vsf_buffer_t headbuffer;
	uint8_t head_buff_mem[16];
};

vsf_err_t bcm_sdpcm_init(struct bcm_wifi_t *wifi);
vsf_err_t bcm_sdpcm_fini(struct bcm_wifi_t *wifi);

vsf_err_t bcm_sdpcm_header_sta(struct vsfip_buffer_t *buf,
			enum vsfip_netif_proto_t proto, const struct vsfip_macaddr_t *dest);
vsf_err_t bcm_sdpcm_header_ap(struct vsfip_buffer_t *buf,
			enum vsfip_netif_proto_t proto, const struct vsfip_macaddr_t *dest);

// io control
#define BCM_SDPCM_CMDTYPE_GET			0x00
#define BCM_SDPCM_CMDTYPE_SET			0x02

uint8_t* bcm_sdpcm_get_ioctrl_buffer(struct vsfip_buffer_t **buf, uint16_t len);
uint8_t* bcm_sdpcm_get_iovar_buffer(struct vsfip_buffer_t **buf, uint16_t len,
									const char *name);

vsf_err_t bcm_sdpcm_ioctrl(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
				uint32_t interface, uint32_t type, uint32_t cmd,
				struct vsfip_buffer_t *buf, struct vsfip_buffer_t **reply);

// async event struct
PACKED_HEAD struct PACKED_MID bcm_ethhead_t
{
    uint8_t destaddr[6];
	uint8_t srcaddr[6];
	uint16_t ethertype;
}; PACKED_TAIL
PACKED_HEAD struct PACKED_MID bcm_sdpcm_ethhead_t
{
	uint16_t subtype;
	uint16_t len;
	uint8_t version;
	uint8_t oui[3];
	uint16_t usr_subtype;
}; PACKED_TAIL
PACKED_HEAD struct PACKED_MID bcm_sdpcm_evthead_t
{
	uint16_t version;
	uint16_t flags;
	uint32_t evttype;
	uint32_t status;
	uint32_t reason;
	uint32_t authtype;
	uint32_t datalen;
	uint8_t addr[6];
	char ifname[16];
	uint8_t ifidx;
	uint8_t bss_cfg_idx;
}; PACKED_TAIL
PACKED_HEAD struct PACKED_MID bcm_sdpcm_evtpkg_t
{
	struct bcm_ethhead_t ethhead;
	struct bcm_sdpcm_ethhead_t sdpcm_ethhead;
	struct bcm_sdpcm_evthead_t evthead;
}; PACKED_TAIL

#endif		// __SDPCM_H_INCLUDED__
