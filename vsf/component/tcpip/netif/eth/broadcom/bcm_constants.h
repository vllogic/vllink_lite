#ifndef __BCM_CONSTANTS_H_INCLUDED__
#define __BCM_CONSTANTS_H_INCLUDED__

#define BCM_CHIP_BCM43362A2						1

#define BCM_RAM_SIZE							0x3C000

// SPI registers
#define BCM_SPIREG_CONTROL						(uint32_t)0x0000
#define BCM_SPIREG_INTF							(uint32_t)0x0004
#define BCM_SPIREG_INTEN						(uint32_t)0x0006
#define BCM_SPIREG_STATUS						(uint32_t)0x0008
#define BCM_SPIREG_F1INFO						(uint32_t)0x000C
#define BCM_SPIREG_F2INFO						(uint32_t)0x000E
#define BCM_SPIREG_F3INFO						(uint32_t)0x0010
#define BCM_SPIREG_TEST							(uint32_t)0x0014

// BCM_SPIREG_STATUS bits
#define BCM_STATUS_DATA_UNAVIL					(uint32_t)0x00000001
#define BCM_STATUS_FIFO_RD_UNDERFLOW			(uint32_t)0x00000002
#define BCM_STATUS_FIFO_WR_OVERFLOW				(uint32_t)0x00000004
#define BCM_STATUS_F2_INTR						(uint32_t)0x00000008
#define BCM_STATUS_F2_RXRDY						(uint32_t)0x00000020
#define BCM_STATUS_F2_PKGAVAIL					(uint32_t)0x00000100
#define BCM_STATUS_F2_PKGLEN_OFFSET				9
#define BCM_STATUS_F2_PKGLEN_LENGTH				11
#define BCM_STATUS_F2_PKGLEN					(uint32_t)0x00000200

// BCM_SPIREG_INTF bits
#define BCM_INTF_DATA_UNAVIL					(uint32_t)0x0001
#define BCM_INTF_F23_FIFO_RD_UNDERFLOW			(uint32_t)0x0002
#define BCM_INTF_F23_FIFO_WR_OVERFLOW			(uint32_t)0x0004
#define BCM_INTF_COMMAND_ERROR					(uint32_t)0x0008
#define BCM_INTF_DATA_ERROR						(uint32_t)0x0010
#define BCM_INTF_F2_PACKET_AVAIL				(uint32_t)0x0020
#define BCM_INTF_F3_PACKET_AVAIL				(uint32_t)0x0040
#define BCM_INTF_F1_OVERFLOW					(uint32_t)0x0080
#define BCM_INTF_F1_INTR						(uint32_t)0x2000
#define BCM_INTF_F2_INTR						(uint32_t)0x4000
#define BCM_INTF_F3_INTR						(uint32_t)0x8000

// cores
#define BCM_CORE_CHIPCOMMON_BASE				0x18000000
#define BCM_CORE_DOT11MAC_BASE					0x18001000
#define BCM_CORE_SDIOD_BASE						0x18002000
#define BCM_CORE_ARM_BASE						0x18103000
#define BCM_CORE_SOCRAM_BASE					0x18104000

// SDIOD core registers
#define BCM_COREREG_SDIOD_CORE					BCM_CORE_SDIOD_BASE
#define BCM_COREREG_SDIOD_INT_STATUS			(BCM_CORE_SDIOD_BASE + 0x20)
#define BCM_COREREG_SDIOD_INT_HOST_MASK			(BCM_CORE_SDIOD_BASE + 0x24)
#	define BCM_HMB_SW_MASK						0x000000F0
#define BCM_COREREG_SDIOD_FINT_MASK				(BCM_CORE_SDIOD_BASE + 0x34)
#define BCM_COREREG_SDIOD_TO_SB_MAILBOX			(BCM_CORE_SDIOD_BASE + 0x40)
#define BCM_COREREG_SDIOD_TO_SB_MAILBOX_DATA	(BCM_CORE_SDIOD_BASE + 0x48)
#define BCM_COREREG_SDIOD_TO_HOST_MAILBOX_DATA	(BCM_CORE_SDIOD_BASE + 0x4C)

#define BCM_COREREG_SDIOD_FRAME_AVAIL			0x000000F0

#define BCM_COREREG_AI_IOCTRL					0x408
#define BCM_COREREG_AI_RESETCTRL				0x800

// BCM_COREREG_AI_RESETCTRL bits
#define BCM_RESETCTRL_RESET						1

// BCM_COREREG_AI_IOCTRL bits
#define BCM_IOCTRL_SICF_FGC						0x0002
#define BCM_IOCTRL_SICF_CLOCK_EN				0x0001

// backplane registers
#define BCM_BPREG_SPROM_CS						(uint32_t)0x10000
#define BCM_BPREG_SPROM_INFO					(uint32_t)0x10001
#define BCM_BPREG_SPROM_DATALOW					(uint32_t)0x10002
#define BCM_BPREG_SPROM_DATAHIGH				(uint32_t)0x10003
#define BCM_BPREG_SPROM_ADDRLOW					(uint32_t)0x10004
#define BCM_BPREG_GPIO_SELECT					(uint32_t)0x10005
#define BCM_BPREG_GPIO_OUT						(uint32_t)0x10006
#define BCM_BPREG_GPIO_EN						(uint32_t)0x10007
#define BCM_BPREG_WATERMARK						(uint32_t)0x10008
#	define BCM_SPI_F2_WATERMARK					32
#define BCM_BPREG_DEVICE_CTL					(uint32_t)0x10009
#define BCM_BPREG_SBADDR						(uint32_t)0x1000A
#define BCM_BPREG_FRAMECTRL						(uint32_t)0x1000D
#	define BCM_BPREG_FRAMECTRL_RF_TERM			(1 << 0)
#define BCM_BPREG_CHIPCLKCSR					(uint32_t)0x1000E
#define BCM_BPREG_SDIOPU						(uint32_t)0x1000F
#define BCM_BPREG_WFRAME_BYTECOUNT				(uint32_t)0x10019
#define BCM_BPREG_RFRAME_BYTECOUNT				(uint32_t)0x1001B
#define BCM_BPREG_MESBUSYCTRL					(uint32_t)0x1001D
#define BCM_BPREG_WAKEUPCTRL					(uint32_t)0x1001E
#define BCM_BPREG_SLEEPCSR						(uint32_t)0x1001F

// BCM_BPREG_CHIPCLKCSR bits
#define BCM_CHIPCLKCSR_FORCE_ALP				(uint32_t)0x01
#define BCM_CHIPCLKCSR_FORCE_HT					(uint32_t)0x02
#define BCM_CHIPCLKCSR_FORCE_ILP				(uint32_t)0x04
#define BCM_CHIPCLKCSR_ALP_REQ					(uint32_t)0x08
#define BCM_CHIPCLKCSR_HT_REQ					(uint32_t)0x10
#define BCM_CHIPCLKCSR_FORCE_HW_CLK_OFF			(uint32_t)0x20
#define BCM_CHIPCLKCSR_ALP_AVAIL				(uint32_t)0x40
#define BCM_CHIPCLKCSR_HT_AVAIL					(uint32_t)0x80

// SDIO bus registers
#define SDIOD_CCCR_REV             ( (uint32_t)  0x00 )    /* CCCR/SDIO Revision */
#define SDIOD_CCCR_SDREV           ( (uint32_t)  0x01 )    /* SD Revision */
#define SDIOD_CCCR_IOEN            ( (uint32_t)  0x02 )    /* I/O Enable */
#define SDIOD_CCCR_IORDY           ( (uint32_t)  0x03 )    /* I/O Ready */
#define SDIOD_CCCR_INTEN           ( (uint32_t)  0x04 )    /* Interrupt Enable */
#define SDIOD_CCCR_INTPEND         ( (uint32_t)  0x05 )    /* Interrupt Pending */
#define SDIOD_CCCR_IOABORT         ( (uint32_t)  0x06 )    /* I/O Abort */
#define SDIOD_CCCR_BICTRL          ( (uint32_t)  0x07 )    /* Bus Interface control */
#define SDIOD_CCCR_CAPABLITIES     ( (uint32_t)  0x08 )    /* Card Capabilities */
#define SDIOD_CCCR_CISPTR_0        ( (uint32_t)  0x09 )    /* Common CIS Base Address Pointer Register 0 (LSB) */
#define SDIOD_CCCR_CISPTR_1        ( (uint32_t)  0x0A )    /* Common CIS Base Address Pointer Register 1 */
#define SDIOD_CCCR_CISPTR_2        ( (uint32_t)  0x0B )    /* Common CIS Base Address Pointer Register 2 (MSB - only bit 1 valid)*/
#define SDIOD_CCCR_BUSSUSP         ( (uint32_t)  0x0C )    /*  */
#define SDIOD_CCCR_FUNCSEL         ( (uint32_t)  0x0D )    /*  */
#define SDIOD_CCCR_EXECFLAGS       ( (uint32_t)  0x0E )    /*  */
#define SDIOD_CCCR_RDYFLAGS        ( (uint32_t)  0x0F )    /*  */
#define SDIOD_CCCR_BLKSIZE_0       ( (uint32_t)  0x10 )    /* Function 0 (Bus) SDIO Block Size Register 0 (LSB) */
#define SDIOD_CCCR_BLKSIZE_1       ( (uint32_t)  0x11 )    /* Function 0 (Bus) SDIO Block Size Register 1 (MSB) */
#define SDIOD_CCCR_POWER_CONTROL   ( (uint32_t)  0x12 )    /* Power Control */
#define SDIOD_CCCR_SPEED_CONTROL   ( (uint32_t)  0x13 )    /* Bus Speed Select  (control device entry into high-speed clocking mode)  */
#define SDIOD_CCCR_UHS_I           ( (uint32_t)  0x14 )    /* UHS-I Support */
#define SDIOD_CCCR_DRIVE           ( (uint32_t)  0x15 )    /* Drive Strength */
#define SDIOD_CCCR_INTEXT          ( (uint32_t)  0x16 )    /* Interrupt Extension */
#define SDIOD_SEP_INT_CTL          ( (uint32_t)  0xF2 )    /* Separate Interrupt Control*/
#define SDIOD_CCCR_F1INFO          ( (uint32_t) 0x100 )    /* Function 1 (Backplane) Info */
#define SDIOD_CCCR_F1HP            ( (uint32_t) 0x102 )    /* Function 1 (Backplane) High Power */
#define SDIOD_CCCR_F1CISPTR_0      ( (uint32_t) 0x109 )    /* Function 1 (Backplane) CIS Base Address Pointer Register 0 (LSB) */
#define SDIOD_CCCR_F1CISPTR_1      ( (uint32_t) 0x10A )    /* Function 1 (Backplane) CIS Base Address Pointer Register 1       */
#define SDIOD_CCCR_F1CISPTR_2      ( (uint32_t) 0x10B )    /* Function 1 (Backplane) CIS Base Address Pointer Register 2 (MSB - only bit 1 valid) */
#define SDIOD_CCCR_F1BLKSIZE_0     ( (uint32_t) 0x110 )    /* Function 1 (Backplane) SDIO Block Size Register 0 (LSB) */
#define SDIOD_CCCR_F1BLKSIZE_1     ( (uint32_t) 0x111 )    /* Function 1 (Backplane) SDIO Block Size Register 1 (MSB) */
#define SDIOD_CCCR_F2INFO          ( (uint32_t) 0x200 )    /* Function 2 (WLAN Data FIFO) Info */
#define SDIOD_CCCR_F2HP            ( (uint32_t) 0x202 )    /* Function 2 (WLAN Data FIFO) High Power */
#define SDIOD_CCCR_F2CISPTR_0      ( (uint32_t) 0x209 )    /* Function 2 (WLAN Data FIFO) CIS Base Address Pointer Register 0 (LSB) */
#define SDIOD_CCCR_F2CISPTR_1      ( (uint32_t) 0x20A )    /* Function 2 (WLAN Data FIFO) CIS Base Address Pointer Register 1       */
#define SDIOD_CCCR_F2CISPTR_2      ( (uint32_t) 0x20B )    /* Function 2 (WLAN Data FIFO) CIS Base Address Pointer Register 2 (MSB - only bit 1 valid) */
#define SDIOD_CCCR_F2BLKSIZE_0     ( (uint32_t) 0x210 )    /* Function 2 (WLAN Data FIFO) SDIO Block Size Register 0 (LSB) */
#define SDIOD_CCCR_F2BLKSIZE_1     ( (uint32_t) 0x211 )    /* Function 2 (WLAN Data FIFO) SDIO Block Size Register 1 (MSB) */
#define SDIOD_CCCR_F3INFO          ( (uint32_t) 0x300 )    /* Function 3 (Bluetooth Data FIFO) Info */
#define SDIOD_CCCR_F3HP            ( (uint32_t) 0x302 )    /* Function 3 (Bluetooth Data FIFO) High Power */
#define SDIOD_CCCR_F3CISPTR_0      ( (uint32_t) 0x309 )    /* Function 3 (Bluetooth Data FIFO) CIS Base Address Pointer Register 0 (LSB) */
#define SDIOD_CCCR_F3CISPTR_1      ( (uint32_t) 0x30A )    /* Function 3 (Bluetooth Data FIFO) CIS Base Address Pointer Register 1       */
#define SDIOD_CCCR_F3CISPTR_2      ( (uint32_t) 0x30B )    /* Function 3 (Bluetooth Data FIFO) CIS Base Address Pointer Register 2 (MSB - only bit 1 valid) */
#define SDIOD_CCCR_F3BLKSIZE_0     ( (uint32_t) 0x310 )    /* Function 3 (Bluetooth Data FIFO) SDIO Block Size Register 0 (LSB) */
#define SDIOD_CCCR_F3BLKSIZE_1     ( (uint32_t) 0x311 )    /* Function 3 (Bluetooth Data FIFO) SDIO Block Size Register 1 (MSB) */

/* SDIOD_CCCR_REV Bits */
#define SDIO_REV_SDIOID_MASK       ( (uint32_t)  0xF0 )    /* SDIO spec revision number */
#define SDIO_REV_CCCRID_MASK       ( (uint32_t)  0x0F )    /* CCCR format version number */

/* SDIOD_CCCR_SDREV Bits */
#define SD_REV_PHY_MASK            ( (uint32_t)  0x0F )    /* SD format version number */

/* SDIOD_CCCR_IOEN Bits */
#define SDIO_FUNC_ENABLE_1         ( (uint32_t)  0x02 )    /* function 1 I/O enable */
#define SDIO_FUNC_ENABLE_2         ( (uint32_t)  0x04 )    /* function 2 I/O enable */
#define SDIO_FUNC_ENABLE_3         ( (uint32_t)  0x08 )    /* function 3 I/O enable */

/* SDIOD_CCCR_IORDY Bits */
#define SDIO_FUNC_READY_1          ( (uint32_t)  0x02 )    /* function 1 I/O ready */
#define SDIO_FUNC_READY_2          ( (uint32_t)  0x04 )    /* function 2 I/O ready */
#define SDIO_FUNC_READY_3          ( (uint32_t)  0x08 )    /* function 3 I/O ready */

/* SDIOD_CCCR_INTEN Bits */
#define INTR_CTL_MASTER_EN         ( (uint32_t)  0x01 )    /* interrupt enable master */
#define INTR_CTL_FUNC1_EN          ( (uint32_t)  0x02 )    /* interrupt enable for function 1 */
#define INTR_CTL_FUNC2_EN          ( (uint32_t)  0x04 )    /* interrupt enable for function 2 */

/* SDIOD_SEP_INT_CTL Bits */
#define SEP_INTR_CTL_MASK          ( (uint32_t)  0x01 )    /* out-of-band interrupt mask */
#define SEP_INTR_CTL_EN            ( (uint32_t)  0x02 )    /* out-of-band interrupt output enable */
#define SEP_INTR_CTL_POL           ( (uint32_t)  0x04 )    /* out-of-band interrupt polarity */

/* SDIOD_CCCR_INTPEND Bits */
#define INTR_STATUS_FUNC1          ( (uint32_t)  0x02 )    /* interrupt pending for function 1 */
#define INTR_STATUS_FUNC2          ( (uint32_t)  0x04 )    /* interrupt pending for function 2 */
#define INTR_STATUS_FUNC3          ( (uint32_t)  0x08 )    /* interrupt pending for function 3 */

/* SDIOD_CCCR_IOABORT Bits */
#define IO_ABORT_RESET_ALL         ( (uint32_t)  0x08 )    /* I/O card reset */
#define IO_ABORT_FUNC_MASK         ( (uint32_t)  0x07 )    /* abort selction: function x */

/* SDIOD_CCCR_BICTRL Bits */
#define BUS_CARD_DETECT_DIS        ( (uint32_t)  0x80 )    /* Card Detect disable */
#define BUS_SPI_CONT_INTR_CAP      ( (uint32_t)  0x40 )    /* support continuous SPI interrupt */
#define BUS_SPI_CONT_INTR_EN       ( (uint32_t)  0x20 )    /* continuous SPI interrupt enable */
#define BUS_SD_DATA_WIDTH_MASK     ( (uint32_t)  0x03 )    /* bus width mask */
#define BUS_SD_DATA_WIDTH_4BIT     ( (uint32_t)  0x02 )    /* bus width 4-bit mode */
#define BUS_SD_DATA_WIDTH_1BIT     ( (uint32_t)  0x00 )    /* bus width 1-bit mode */

/* SDIOD_CCCR_CAPABLITIES Bits */
#define SDIO_CAP_4BLS              ( (uint32_t)  0x80 )    /* 4-bit support for low speed card */
#define SDIO_CAP_LSC               ( (uint32_t)  0x40 )    /* low speed card */
#define SDIO_CAP_E4MI              ( (uint32_t)  0x20 )    /* enable interrupt between block of data in 4-bit mode */
#define SDIO_CAP_S4MI              ( (uint32_t)  0x10 )    /* support interrupt between block of data in 4-bit mode */
#define SDIO_CAP_SBS               ( (uint32_t)  0x08 )    /* support suspend/resume */
#define SDIO_CAP_SRW               ( (uint32_t)  0x04 )    /* support read wait */
#define SDIO_CAP_SMB               ( (uint32_t)  0x02 )    /* support multi-block transfer */
#define SDIO_CAP_SDC               ( (uint32_t)  0x01 )    /* Support Direct commands during multi-byte transfer */

/* SDIOD_CCCR_POWER_CONTROL Bits */
#define SDIO_POWER_SMPC            ( (uint32_t)  0x01 )    /* supports master power control (RO) */
#define SDIO_POWER_EMPC            ( (uint32_t)  0x02 )    /* enable master power control (allow > 200mA) (RW) */

/* SDIOD_CCCR_SPEED_CONTROL Bits */
#define SDIO_SPEED_SHS             ( (uint32_t)  0x01 )    /* supports high-speed [clocking] mode (RO) */
#define SDIO_SPEED_EHS             ( (uint32_t)  0x02 )    /* enable high-speed [clocking] mode (RW) */


// SB registers
#define BCM_SBREG_F1SIG							(uint32_t)0x18000000

#define BCM_INTERFACE_STA						0
#define BCM_INTERFACE_AP						1
#define BCM_INTERFACE_P2P						2

// IO CONTROL
#define BCMIOVAR_TX_GLOM						"bus:txglom"
#define BCMIOVAR_APSTA							"apsta"
#define BCMIOVAR_ETHADDR						"cur_etheraddr"
#define BCMIOVAR_VER							"ver"
#define BCMIOVAR_AMPDU_BA_WSIZE					"ampdu_ba_wsize"
#define BCMIOVAR_AMPDU_MPDU						"ampdu_mpdu"
#define BCMIOVAR_AMPDU_RX_FACTOR				"ampdu_rx_factor"
#define BCMIOVAR_EVT_MSGS						"event_msgs"
#define BCMIOVAR_COUNTRY						"country"
#define BCMIOVAR_BSSCFG_EVT_MSGS				"bsscfg:event_msgs"
#define BCMIOVAR_BSSCFG_SUP_WPA					"bsscfg:sup_wpa"
#define BCMIOVAR_BSSCFG_SUP_WPA2_EAPVER			"bsscfg:sup_wpa2_eapver"
#define BCMIOVAR_ESCAN							"escan"

#define BCMIOCTRL_UP							(uint32_t)2
#define BCMIOCTRL_DOWN							(uint32_t)3
#define BCMIOCTRL_SET_INFRA						(uint32_t)20
#define BCMIOCTRL_SET_AUTH						(uint32_t)22
#define BCMIOCTRL_SET_SSID						(uint32_t)26
#define BCMIOCTRL_GET_CHANNEL					(uint32_t)29
#define BCMIOCTRL_SET_CHANNEL					(uint32_t)30
#define BCMIOCTRL_DISASSOC						(uint32_t)52
#define BCMIOCTRL_SET_GMODE						(uint32_t)110
#define BCMIOCTRL_SET_WSEC						(uint32_t)134
#define BCMIOCTRL_SET_WPA_AUTH					(uint32_t)165
#define BCMIOCTRL_GET_VAR						(uint32_t)262
#define BCMIOCTRL_SET_VAR						(uint32_t)263
#define BCMIOCTRL_SET_SWEC_PMK					(uint32_t)268

// async events
#define BCM_ETHTYPE								0x886C
#define BCM_OUI									"\x00\x10\x18"

#define BCMASYNC_EVT_NONE						-1
#define BCMASYNC_EVT_SET_SSID					0
#define BCMASYNC_EVT_JOIN						1
#define BCMASYNC_EVT_START						2
#define BCMASYNC_EVT_AUTH						3
#define BCMASYNC_EVT_AUTH_IND					4
#define BCMASYNC_EVT_DEAUTH						5
#define BCMASYNC_EVT_DEAUTH_IND					6
#define BCMASYNC_EVT_ASSOC						7
#define BCMASYNC_EVT_ASSOC_IND					8
#define BCMASYNC_EVT_REASSOC					9
#define BCMASYNC_EVT_REASSOC_IND				10
#define BCMASYNC_EVT_DISASSOC					11
#define BCMASYNC_EVT_DISASSOC_IND				12
#define BCMASYNC_EVT_LINK						16
#define BCMASYNC_EVT_ROAM						19
#define BCMASYNC_EVT_PRUNE						23
#define BCMASYNC_EVT_PSK_SUP					46
#define BCMASYNC_EVT_SCAN_RESULT				69
#if BCM_CHIP_BCM43362A2
#	define BCMASYNC_EVT_LAST					87
#endif

#define BCMASYNC_EVT_MASKLEN					((BCMASYNC_EVT_LAST + 7) >> 3)

#define BCMASYNC_SUPSTATUS_OFFSEET				256

enum bcm_async_status_t
{
	BCMASYNC_EVTSTATUS_SUCCESS				= 0,
	BCMASYNC_EVTSTATUS_FAIL					= 1,
	BCMASYNC_EVTSTATUS_TIMEOUT				= 2,
	BCMASYNC_EVTSTATUS_NO_NETWORKS			= 3,
	BCMASYNC_EVTSTATUS_ABORT				= 4,
	BCMASYNC_EVTSTATUS_NO_ACK				= 5,
	BCMASYNC_EVTSTATUS_UNSOLICITED			= 6,
	BCMASYNC_EVTSTATUS_ATTEMPT				= 7,
	BCMASYNC_EVTSTATUS_PARTIAL				= 8,
	BCMASYNC_EVTSTATUS_NEWSCAN				= 9,
	BCMASYNC_EVTSTATUS_NEWASSOC				= 10,
	BCMASYNC_EVTSTATUS_11HQUIET				= 11,
	BCMASYNC_EVTSTATUS_SUPPRESS				= 12,
	BCMASYNC_EVTSTATUS_NOCHANS				= 13,
	BCMASYNC_EVTSTATUS_CCXFASTRM			= 14,
	BCMASYNC_EVTSTATUS_CS_ABORT				= 15,

	// SUP status
	BCMASYNC_SUPSTATUS_KEYXCHANGE			= 5 + BCMASYNC_SUPSTATUS_OFFSEET,
	BCMASYNC_SUPSTATUS_KEYED				= 6 + BCMASYNC_SUPSTATUS_OFFSEET,
	BCMASYNC_SUPSTATUS_LAST_BASIC_STATE		= 8 + BCMASYNC_SUPSTATUS_OFFSEET,
};

// WIFI
// ESCAN
#define BCM_WIFI_ESCAN_VER						1
#define BCM_WIFI_SCANACT_START					1
#define BCM_WIFI_SCANACT_CONTINUE				2
#define BCM_WIFI_SCANACT_ABORT					3
#define BCM_WIFI_SCANTYPE_ACTIVE				0x00
#define BCM_WIFI_SCANTYPE_PASSIVE				0x01
#define BCM_WIFI_BSSTYPE_INFRASTRUCTURE			0
#define BCM_WIFI_BSSTYPE_ADHOC					1
#define BCM_WIFI_BSSTYPE_ANY					2

// JOIN
#define BCM_WIFI_AUTH_OPEN						0x00000000
#define BCM_WIFI_AUTH_WEP_ENABLED				0x00000001
#define BCM_WIFI_AUTH_TKIP_ENABLED				0x00000002
#define BCM_WIFI_AUTH_AES_ENABLED				0x00000004
#define BCM_WIFI_AUTH_WSEC_SWFLAG				0x00000008
#define BCM_WIFI_AUTH_SHARED_ENABLED			0x00008000
#define BCM_WIFI_AUTH_WPA_ENABLED				0x00200000
#define BCM_WIFI_AUTH_WPA2_ENABLED				0x00400000
#define BCM_WIFI_AUTH_WPS_ENABLED				0x10000000

#define BCM_WIFI_WPAAUTH_DISABLED				0x00000000
#define BCM_WIFI_WPAAUTH_NONE					0x00000001
#define BCM_WIFI_WPAAUTH_UNSPECIFIED			0x00000002
#define BCM_WIFI_WPAAUTH_PSK					0x00000004

#define BCM_WIFI_WPA2AUTH_UNSPECIFIED			0x00000040
#define BCM_WIFI_WPA2AUTH_PSK					0x00000080

enum bcm_authtype_t
{
	BCM_AUTHTYPE_OPEN			= 0,
	BCM_AUTHTYPE_WEP_PSK		= BCM_WIFI_AUTH_WEP_ENABLED,
	BCM_AUTHTYPE_WEP_SHARED		= BCM_WIFI_AUTH_WEP_ENABLED | BCM_WIFI_AUTH_SHARED_ENABLED,
	BCM_AUTHTYPE_WPA_TKIP_PSK	= BCM_WIFI_AUTH_WPA_ENABLED | BCM_WIFI_AUTH_TKIP_ENABLED,
	BCM_AUTHTYPE_WPA_AES_PSK	= BCM_WIFI_AUTH_WPA_ENABLED | BCM_WIFI_AUTH_AES_ENABLED,
	BCM_AUTHTYPE_WPA2_AES_PSK	= BCM_WIFI_AUTH_WPA2_ENABLED | BCM_WIFI_AUTH_AES_ENABLED,
	BCM_AUTHTYPE_WPA2_TKIP_PSK	= BCM_WIFI_AUTH_WPA2_ENABLED | BCM_WIFI_AUTH_TKIP_ENABLED,
	BCM_AUTHTYPE_WPA2_MIXED_PSK	= BCM_WIFI_AUTH_WPA2_ENABLED | BCM_WIFI_AUTH_TKIP_ENABLED | BCM_WIFI_AUTH_AES_ENABLED,
	BCM_AUTHTYPE_WPS_OPEN		= BCM_WIFI_AUTH_WPS_ENABLED,
	BCM_AUTHTYPE_WPS_SECURE		= BCM_WIFI_AUTH_WPS_ENABLED | BCM_WIFI_AUTH_AES_ENABLED,
	BCM_AUTHTYPE_UNKNOWN		= -1,
};

#endif		// __BCM_CONSTANTS_H_INCLUDED__
