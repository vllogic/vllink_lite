#define APPCFG_VSFTIMER_NUM				16
//#define APPCFG_BUFMGR_SIZE				1024

//#define APPCFG_HRT_QUEUE_LEN			0
#define APPCFG_SRT_QUEUE_LEN			16
#define APPCFG_NRT_QUEUE_LEN			16

#if (defined(APPCFG_HRT_QUEUE_LEN) && (APPCFG_HRT_QUEUE_LEN > 0)) ||\
	(defined(APPCFG_SRT_QUEUE_LEN) && (APPCFG_SRT_QUEUE_LEN > 0)) ||\
	(defined(APPCFG_NRT_QUEUE_LEN) && (APPCFG_NRT_QUEUE_LEN > 0))
#define VSFSM_CFG_PREMPT_EN				1
#else
#define VSFSM_CFG_PREMPT_EN				0
#endif

// define APPCFG_USR_POLL for round robin scheduling
//#define APPCFG_USR_POLL

#ifdef APPCFG_USR_POLL
#	define APPCFG_TICKCLK_PRIORITY		-1
#else
#	define APPCFG_TICKCLK_PRIORITY		VSFHAL_TICKCLK_PRIORITY
#endif

#define APPCFG_PENDSV_PRIORITY			VSFHAL_PENDSV_PRIORITY

/*******************************************************************************
	Bootloader Paramter Config
*******************************************************************************/
#define APPCFG_USBD_VID					0x0D28
#define APPCFG_USBD_PID					0x0204
#define APPCFG_USBD_BCD					0x0100


