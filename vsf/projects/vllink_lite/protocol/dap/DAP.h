#ifndef __DAP_H__
#define __DAP_H__

#ifdef __cplusplus
extern "C" {
#endif

// DAP Command IDs
#define ID_DAP_Info                     0x00U
#define ID_DAP_HostStatus               0x01U
#define ID_DAP_Connect                  0x02U
#define ID_DAP_Disconnect               0x03U
#define ID_DAP_TransferConfigure        0x04U
#define ID_DAP_Transfer                 0x05U
#define ID_DAP_TransferBlock            0x06U
#define ID_DAP_TransferAbort            0x07U
#define ID_DAP_WriteABORT               0x08U
#define ID_DAP_Delay                    0x09U
#define ID_DAP_ResetTarget              0x0AU
#define ID_DAP_SWJ_Pins                 0x10U
#define ID_DAP_SWJ_Clock                0x11U
#define ID_DAP_SWJ_Sequence             0x12U
#define ID_DAP_SWD_Configure            0x13U
#define ID_DAP_JTAG_Sequence            0x14U
#define ID_DAP_JTAG_Configure           0x15U
#define ID_DAP_JTAG_IDCODE              0x16U
#define ID_DAP_SWO_Transport            0x17U
#define ID_DAP_SWO_Mode                 0x18U
#define ID_DAP_SWO_Baudrate             0x19U
#define ID_DAP_SWO_Control              0x1AU
#define ID_DAP_SWO_Status               0x1BU
#define ID_DAP_SWO_Data                 0x1CU

#define ID_DAP_QueueCommands            0x7EU
#define ID_DAP_ExecuteCommands          0x7FU

// DAP Vendor Command IDs
#define ID_DAP_Vendor0                  0x80U
#define ID_DAP_Vendor1                  0x81U
#define ID_DAP_Vendor2                  0x82U
#define ID_DAP_Vendor3                  0x83U
#define ID_DAP_Vendor4                  0x84U
#define ID_DAP_Vendor5                  0x85U
#define ID_DAP_Vendor6                  0x86U
#define ID_DAP_Vendor7                  0x87U
#define ID_DAP_Vendor8                  0x88U
#define ID_DAP_Vendor9                  0x89U
#define ID_DAP_Vendor10                 0x8AU
#define ID_DAP_Vendor11                 0x8BU
#define ID_DAP_Vendor12                 0x8CU
#define ID_DAP_Vendor13                 0x8DU
#define ID_DAP_Vendor14                 0x8EU
#define ID_DAP_Vendor15                 0x8FU
#define ID_DAP_Vendor16                 0x90U
#define ID_DAP_Vendor17                 0x91U
#define ID_DAP_Vendor18                 0x92U
#define ID_DAP_Vendor19                 0x93U
#define ID_DAP_Vendor20                 0x94U
#define ID_DAP_Vendor21                 0x95U
#define ID_DAP_Vendor22                 0x96U
#define ID_DAP_Vendor23                 0x97U
#define ID_DAP_Vendor24                 0x98U
#define ID_DAP_Vendor25                 0x99U
#define ID_DAP_Vendor26                 0x9AU
#define ID_DAP_Vendor27                 0x9BU
#define ID_DAP_Vendor28                 0x9CU
#define ID_DAP_Vendor29                 0x9DU
#define ID_DAP_Vendor30                 0x9EU
#define ID_DAP_Vendor31                 0x9FU

#define ID_DAP_Invalid                  0xFFU

// DAP Status Code
#define DAP_OK                          0U
#define DAP_ERROR                       0xFFU

// DAP ID
#define DAP_ID_VENDOR                   1U
#define DAP_ID_PRODUCT                  2U
#define DAP_ID_SER_NUM                  3U
#define DAP_ID_FW_VER                   4U
#define DAP_ID_DEVICE_VENDOR            5U
#define DAP_ID_DEVICE_NAME              6U
#define DAP_ID_CAPABILITIES             0xF0U
#define DAP_ID_SWO_BUFFER_SIZE          0xFDU
#define DAP_ID_PACKET_COUNT             0xFEU
#define DAP_ID_PACKET_SIZE              0xFFU

// DAP Host Status
#define DAP_DEBUGGER_CONNECTED          0U
#define DAP_TARGET_RUNNING              1U

// DAP Port
#define DAP_PORT_AUTODETECT             0U      // Autodetect Port
#define DAP_PORT_DISABLED               0U      // Port Disabled (I/O pins in High-Z)
#define DAP_PORT_SWD                    1U      // SWD Port (SWCLK, SWDIO) + nRESET
#define DAP_PORT_JTAG                   2U      // JTAG Port (TCK, TMS, TDI, TDO, nTRST) + nRESET

// DAP SWJ Pins
#define DAP_SWJ_SWCLK_TCK               0       // SWCLK/TCK
#define DAP_SWJ_SWDIO_TMS               1       // SWDIO/TMS
#define DAP_SWJ_TDI                     2       // TDI
#define DAP_SWJ_TDO                     3       // TDO
#define DAP_SWJ_nTRST                   5       // nTRST
#define DAP_SWJ_nRESET                  7       // nRESET

// DAP Transfer Request
#define DAP_TRANSFER_APnDP              (1U<<0)
#define DAP_TRANSFER_RnW                (1U<<1)
#define DAP_TRANSFER_A2                 (1U<<2)
#define DAP_TRANSFER_A3                 (1U<<3)
#define DAP_TRANSFER_MATCH_VALUE        (1U<<4)
#define DAP_TRANSFER_MATCH_MASK         (1U<<5)

// DAP Transfer Response
#define DAP_TRANSFER_OK                 (1U<<0)
#define DAP_TRANSFER_WAIT               (1U<<1)
#define DAP_TRANSFER_FAULT              (1U<<2)
#define DAP_TRANSFER_ERROR              (1U<<3)
#define DAP_TRANSFER_MISMATCH           (1U<<4)

// DAP SWO Trace Mode
#define DAP_SWO_OFF                     0U
#define DAP_SWO_UART                    1U
#define DAP_SWO_MANCHESTER              2U

// DAP SWO Trace Status
#define DAP_SWO_CAPTURE_ACTIVE          (1U<<0)
#define DAP_SWO_CAPTURE_PAUSED          (1U<<1)
#define DAP_SWO_STREAM_ERROR            (1U<<6)
#define DAP_SWO_BUFFER_OVERRUN          (1U<<7)


// Debug Port Register Addresses
#define DP_IDCODE                       0x00U   // IDCODE Register (SW Read only)
#define DP_ABORT                        0x00U   // Abort Register (SW Write only)
#define DP_CTRL_STAT                    0x04U   // Control & Status
#define DP_WCR                          0x04U   // Wire Control Register (SW Only)
#define DP_SELECT                       0x08U   // Select Register (JTAG R/W & SW W)
#define DP_RESEND                       0x08U   // Resend (SW Read Only)
#define DP_RDBUFF                       0x0CU   // Read Buffer (Read Only)

// JTAG IR Codes
#define JTAG_ABORT                      0x08U
#define JTAG_DPACC                      0x0AU
#define JTAG_APACC                      0x0BU
#define JTAG_IDCODE                     0x0EU
#define JTAG_BYPASS                     0x0FU

// JTAG Sequence Info
#define JTAG_SEQUENCE_TCK               0x3FU   // TCK count
#define JTAG_SEQUENCE_TMS               0x40U   // TMS value
#define JTAG_SEQUENCE_TDO               0x80U   // TDO capture

struct dap_param_t
{
	void *cb_param;
	void (*cb_response)(void *p, uint8_t *buf, uint16_t size);

	struct vsfsm_t sm;
	struct vsfsm_pt_t pt;
	struct vsfsm_pt_t cmd_pt;
	
	struct vsfsm_t sem_sm;
	struct vsfsm_sem_t request_sem;
	struct vsfsm_sem_t response_sem;

#if SWO_UART || SWO_MANCHESTER
#if SWO_UART
	struct vsf_fifostream_t swo_rx;
	uint8_t swo_buff[SWO_BUFFER_SIZE];
#endif
	uint8_t transport;
	
	uint8_t trace_status;
	uint8_t trace_mode;
#endif
	
	uint8_t request[DAP_PACKET_COUNT][DAP_PACKET_SIZE];
	uint8_t response[DAP_PACKET_SIZE];
	uint8_t request_head;
	uint8_t request_cnt;
	
	uint16_t req_ptr;
	uint16_t resp_ptr;
	uint16_t resp_start;
	
	uint16_t transfer_cnt;
	uint16_t transfer_num;
	
	union
	{
		struct
		{
			uint16_t prepare_req_ptr;
			uint16_t prepare_transfer_cnt;
			uint16_t dummy;
		} jtag_seq;
	};
	
	bool abort;
	bool post_read;
	bool check_write;
	uint8_t transfer_req;
	uint8_t transfer_ack;
	uint32_t transfer_data;
	uint16_t transfer_retry;
	
	uint8_t jtag_ir;
	uint8_t request_ir;
	
	uint8_t cmd_id;
	uint8_t cmd_num;
	
	uint8_t port;
	uint16_t khz;
	struct
	{
		uint8_t idle_cycles;						// Idle cycles after transfer
		uint16_t retry_count;						// Number of retries after WAIT response
		uint16_t match_retry;						// Number of retries if read value does not match
		uint32_t match_mask;						// Match Mask
	} transfer;
#if DAP_SWD
	struct 
	{
		uint8_t turnaround;						// Turnaround period
		uint8_t data_phase;						// Always generate Data Phase
	} swd_conf;
#endif
#if DAP_JTAG
	struct 
	{
		uint8_t count;								// Number of devices
		uint8_t index;								// Device index (device at TDO has index 0)
		uint8_t ir_length[DAP_JTAG_DEV_CNT];		// IR Length in bits
		uint16_t ir_before[DAP_JTAG_DEV_CNT];		// Bits before IR
		uint16_t ir_after[DAP_JTAG_DEV_CNT];		// Bits after IR

		uint64_t buf_tdi;
		uint64_t buf_tms;
		uint64_t buf_tdo;
	} jtag_dev;
#endif
};

vsf_err_t DAP_init(struct dap_param_t *param);
vsf_err_t DAP_register(struct dap_param_t *param, void *cb_param,
		void (*cb_response)(void *, uint8_t *, uint16_t));
void DAP_unregister(struct dap_param_t *param, void *cb_param,
		void (*cb_response)(void *, uint8_t *, uint16_t));
vsf_err_t DAP_recvive_request(struct dap_param_t *param, uint8_t *buf, uint16_t size);
void DAP_send_response_done(struct dap_param_t *param);

#ifdef __cplusplus
}
#endif

#endif  // __DAP_H__
