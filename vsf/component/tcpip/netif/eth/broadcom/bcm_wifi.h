#ifndef __BCM_WIFI_H_INCLUDED__
#define __BCM_WIFI_H_INCLUDED__

#include "bcm_constants.h"
#include "bus/bcm_bus.h"
#include "bcm_sdpcm.h"

#define BCM_WIFI_JOINSTATUS_ASSOCED					(1UL << 0)
#define BCM_WIFI_JOINSTATUS_AUTHED					(1UL << 1)
#define BCM_WIFI_JOINSTATUS_LINKED					(1UL << 2)
#define BCM_WIFI_JOINSTATUS_SECURED					(1UL << 3)
#define BCM_WIFI_JOINSTATUS_COMPLETED				(1UL << 4)
#define BCM_WIFI_JOINSTATUS_NO_NETWORKS				(1UL << 5)

#define BCM_WIFI_JOINSTATUS_ASSOC_FAILED			(1UL << 8)
#define BCM_WIFI_JOINSTATUS_AUTH_FAILED				(1UL << 9)
#define BCM_WIFI_JOINSTATUS_LINK_FAILED				(1UL << 10)
#define BCM_WIFI_JOINSTATUS_SECURE_FAILED			(1UL << 11)

struct bcm_wifi_t
{
	struct bcm_bus_t bus;
	char *country;

	// private
	struct bcm_sdpcm_t sdpcm;

	// resources for bcm_wifi_init/scan/join ...
	struct vsfsm_pt_t init_pt;
	struct vsfip_buffer_t *buffer;
	// resources for bcm_wifi_prepare_join
	struct vsfsm_pt_t prepare_join_pt;
	// resources for bcm_wifi_ioctrl
	struct vsfsm_pt_t ioctrl_pt;

	bool ap_isup;
	bool p2p_isup;
	uint32_t retry;
	uint32_t join_status[3];
	struct vsfip_netif_t *netif;
};

extern struct vsfip_netdrv_op_t bcm_wifi_netdrv_op;

vsf_err_t bcm_wifi_scan(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
						struct vsfip_buffer_t **result, uint8_t interface);

vsf_err_t bcm_wifi_join(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
			uint32_t *status, const char *ssid, enum bcm_authtype_t authtype,
			const uint8_t *key, uint8_t keylen);
vsf_err_t bcm_wifi_leave(struct vsfsm_pt_t *pt, vsfsm_evt_t evt);

// TODO:
// bcm_wifi_enable_powersave
// bcm_wifi_disable_powersave
// bcm_wifi_set_listen_interval
// bcm_wifi_get_listen_interval
// ......

// WIFI structures
PACKED_HEAD struct PACKED_MID bcm_wifi_bss_info_t
{
	uint32_t version;
	uint32_t length;
	uint8_t bssid[6];
	uint16_t beacon_period;
	uint16_t capability;
	uint8_t ssid_len;
	uint8_t ssid[32];
	struct
	{
		uint32_t count;
		uint8_t rates[16];
	} rateset;
	uint16_t chanspec;
	uint16_t atim_window;
	uint16_t dtim_period;
	uint16_t rssi;
	int8_t phy_noise;

	uint8_t n_ap;
	uint32_t nbss_cap;
	uint8_t ctl_ch;
	uint32_t reserved32[1];
	uint8_t flags;
	uint8_t reserved[3];
	uint8_t basic_mcs[16];

	uint16_t ie_offset;
	uint32_t ie_length;
	int16_t snr;
}; PACKED_TAIL
PACKED_HEAD struct PACKED_MID bcm_wifi_escan_result_t
{
	uint32_t buflen;
	uint32_t version;
	uint16_t syncid;
	uint16_t bss_count;
	struct bcm_wifi_bss_info_t bss_info[1];
}; PACKED_TAIL

#endif		// __BCM_WIFI_H_INCLUDED__
