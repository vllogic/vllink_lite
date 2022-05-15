
#define __VSF_USBD_CLASS_INHERIT__

/*============================ INCLUDES ======================================*/

#include "vsf.h"
#include "dap/dap.h"
#include "vsf_usbd_cmsis_dap_v2.h"
#include "vsf_usbd_webusb_usr.h"

#ifdef APP_CFG_CDCSHELL_SUPPORT
#include "shell/extrauart.h"
#include "shell/help.h"
#endif

/*============================ PROTOTYPES ====================================*/

static uint16_t usrapp_get_serial(uint8_t *serial);
static void usrapp_config_usart(enum usart_idx_t idx, uint32_t *mode, uint32_t *baudrate, vsf_stream_t *tx, vsf_stream_t *rx, bool return_actual_baud);
static uint32_t usrapp_get_usart_baud(enum usart_idx_t idx, uint32_t baudrate);
static vsf_err_t usrapp_cdcext_set_line_coding(usb_cdcacm_line_coding_t *line_coding);
static vsf_err_t usrapp_cdcext_set_control_line(uint8_t control_line);
static vsf_err_t usrapp_cdcshell_set_line_coding(usb_cdcacm_line_coding_t *line_coding);

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

typedef struct usrapp_t {
    dap_t dap;

    uint32_t usart_ext_mode;
    uint32_t usart_ext_baud;
    #if SWO_UART
    uint32_t usart_swo_mode;
    uint32_t usart_swo_baud;
    #endif

    #ifdef APP_CFG_CDCSHELL_SUPPORT
    uint32_t cdc_shell_usart_mode;
    uint32_t cdc_shell_usart_baud;
    #endif
} usrapp_t;

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/

usrapp_t usrapp                 = {
    .dap.dap_param              = {
        .get_serial             = usrapp_get_serial,
        #if VENDOR_UART || SWO_UART
        .config_usart           = usrapp_config_usart,
        .get_usart_baud         = usrapp_get_usart_baud,
        #endif
        #if VENDOR_UART
        .ext_tx = {
            .op                 = &vsf_fifo_stream_op,
            .buffer             = usrapp.dap.dap_param.ext_tx_buf,
            .size               = sizeof(usrapp.dap.dap_param.ext_tx_buf),
        },
        .ext_rx = {
            .op                 = &vsf_fifo_stream_op,
            .buffer             = usrapp.dap.dap_param.ext_rx_buf,
            .size               = sizeof(usrapp.dap.dap_param.ext_rx_buf),
        },
        .swo_tx = {
            .op                 = &vsf_fifo_stream_op,
            .buffer             = usrapp.dap.dap_param.swo_tx_buf,
            .size               = sizeof(usrapp.dap.dap_param.swo_tx_buf),
        },
        #endif
        #if SWO_UART
        .swo_rx = {
            .op                 = &vsf_fifo_stream_op,
            .buffer             = usrapp.dap.dap_param.swo_rx_buf,
            .size               = sizeof(usrapp.dap.dap_param.swo_rx_buf),
        },
        #endif
        .do_abort               = false,
    },
};

/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

#include "usrapp_usbd_common.c"
#include "usrapp_usbd_vllinklite.c"

#ifdef APP_CFG_SERIAL_ATTACH_UUID
static void usrapp_init_serial(uint8_t *serial, uint16_t size)
{
    ASSERT(size > (2 + APP_CFG_SERIAL_HEADER_STR_LENGTH));

	uint8_t uid[16], uid_size, i;

    uid_size = vsfhal_uid_read(uid, sizeof(uid));
    memset(serial, 0, size);
    serial[0] = size;
    serial[1] = USB_DT_STRING;
    memcpy(serial + 2, APP_CFG_SERIAL_HEADER_STR, APP_CFG_SERIAL_HEADER_STR_LENGTH);
    serial += 2 + APP_CFG_SERIAL_HEADER_STR_LENGTH;
    size -= 2 + APP_CFG_SERIAL_HEADER_STR_LENGTH;

    size = min(size / 2, uid_size * 2);
    for (i = 0; i < size; i++) {
        uint8_t v = uid[i / 2];
		if (i & 0x1)
			v = v & 0xf;
		else
			v >>= 4;
		if (v < 10)
			*serial = '0' + v;
		else
			*serial = 'A' - 10 + v;
        serial += 2;
    }
}
#endif

static uint16_t usrapp_get_serial(uint8_t *serial)
{
    uint16_t i, size = (sizeof(__usrapp_usbd_vllinklite_nonconst.usbd.str_serial) - 2) / 2;
    if (serial) {
        for (i = 0; i < size; i++)
            serial[i] = __usrapp_usbd_vllinklite_nonconst.usbd.str_serial[i * 2 + 2];
    }
    return size;
}

static void usrapp_config_usart(enum usart_idx_t idx, uint32_t *mode, uint32_t *baudrate, vsf_stream_t *tx, vsf_stream_t *rx, bool return_actual_baud)
{
    switch (idx) {
    case PERIPHERAL_UART_EXT_IDX:
        if ((mode && *mode != usrapp.usart_ext_mode) || (usrapp.usart_ext_baud == 0)) {
            if (mode)
                usrapp.usart_ext_mode = *mode;
            if (baudrate)
                usrapp.usart_ext_baud = *baudrate;
            vsfhal_usart_init(PERIPHERAL_UART_EXT_IDX);
            if (return_actual_baud)
                usrapp.usart_ext_baud = vsfhal_usart_config(PERIPHERAL_UART_EXT_IDX, usrapp.usart_ext_baud, usrapp.usart_ext_mode);
            else
                vsfhal_usart_config(PERIPHERAL_UART_EXT_IDX, usrapp.usart_ext_baud, usrapp.usart_ext_mode);
            if (baudrate)
                *baudrate = usrapp.usart_ext_baud;
            vsfhal_usart_stream_config(PERIPHERAL_UART_EXT_IDX, USART_STREAM_EDA_PRIORITY, PERIPHERAL_UART_EXT_PRIORITY, tx, rx);
        } else if (baudrate && *baudrate != usrapp.usart_ext_baud) {
            usrapp.usart_ext_baud = *baudrate;
            if (return_actual_baud)
                usrapp.usart_ext_baud = vsfhal_usart_config(PERIPHERAL_UART_EXT_IDX, usrapp.usart_ext_baud, USART_RESET_BAUD_ONLY);
            else
                vsfhal_usart_config(PERIPHERAL_UART_EXT_IDX, usrapp.usart_ext_baud, USART_RESET_BAUD_ONLY);
        }
        if (!mode && !baudrate) {
            usrapp.usart_ext_mode = 0;
            usrapp.usart_ext_baud = 0;
            vsfhal_usart_fini(PERIPHERAL_UART_EXT_IDX);
        }
        break;
    #if SWO_UART
    case PERIPHERAL_UART_SWO_IDX:
        if ((mode && *mode != usrapp.usart_swo_mode) || (usrapp.usart_swo_baud == 0)) {
            if (mode)
                usrapp.usart_swo_mode = *mode;
            if (baudrate)
                usrapp.usart_swo_baud = *baudrate;
            vsfhal_usart_init(PERIPHERAL_UART_SWO_IDX);
            if (return_actual_baud)
                usrapp.usart_swo_baud = vsfhal_usart_config(PERIPHERAL_UART_SWO_IDX, usrapp.usart_swo_baud, usrapp.usart_swo_mode);
            else
                vsfhal_usart_config(PERIPHERAL_UART_SWO_IDX, usrapp.usart_swo_baud, usrapp.usart_swo_mode);
            if (baudrate)
                *baudrate = usrapp.usart_swo_baud;
            vsfhal_usart_stream_config(PERIPHERAL_UART_SWO_IDX, USART_STREAM_EDA_PRIORITY, PERIPHERAL_UART_EXT_PRIORITY, tx, rx);
        } else if (baudrate && *baudrate != usrapp.usart_swo_baud) {
            usrapp.usart_swo_baud = *baudrate;
            if (return_actual_baud)
                usrapp.usart_swo_baud = vsfhal_usart_config(PERIPHERAL_UART_SWO_IDX, usrapp.usart_swo_baud, USART_RESET_BAUD_ONLY);
            else
                vsfhal_usart_config(PERIPHERAL_UART_SWO_IDX, usrapp.usart_swo_baud, USART_RESET_BAUD_ONLY);
        }
        if (!mode && !baudrate) {
            usrapp.usart_swo_mode = 0;
            usrapp.usart_swo_baud = 0;
            vsfhal_usart_fini(PERIPHERAL_UART_SWO_IDX);
        }
        break;
    #endif
    }
}

static uint32_t usrapp_get_usart_baud(enum usart_idx_t idx, uint32_t baudrate)
{
    return vsfhal_usart_config(idx, baudrate, USART_GET_BAUD_ONLY);
}

static vsf_callback_timer_t cb_timer;

#define DAP_TEST_PORT                   0   // 0, DAP_PORT_SWD, DAP_PORT_JTAG
#define DAP_TEST_SPEED_KHZ              4000
#define UART_SWO_TEST_ENABLE            0
#define UART_EXT_TEST_ENABLE            0
#if DAP_TEST_PORT
static void do_dap_test(vsf_callback_timer_t *timer)
{
    dap_test(&usrapp.dap, DAP_TEST_PORT, DAP_TEST_SPEED_KHZ);
}
#endif

static void connect_usbd(vsf_callback_timer_t *timer)
{
    vk_usbd_connect(&__usrapp_usbd_vllinklite.usbd.dev);
    
#if DAP_TEST_PORT
    timer->on_timer = do_dap_test;
    vsf_callback_timer_add_ms(timer, 1000);
#endif

#if UART_SWO_TEST_ENABLE
    static uint8_t test_swo[16] = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    vsfhal_usart_init(PERIPHERAL_UART_SWO_IDX);
    vsfhal_usart_config(PERIPHERAL_UART_SWO_IDX, 2000000, PERIPHERAL_UART_MODE_DEFAULT);
    vsfhal_usart_tx_bytes(PERIPHERAL_UART_SWO_IDX, test_swo, sizeof(test_swo));
    vsfhal_usart_config_cb(PERIPHERAL_UART_SWO_IDX, PERIPHERAL_UART_SWO_PRIORITY, NULL, NULL, NULL);
#endif
#if UART_EXT_TEST_ENABLE
    static uint8_t test_ext[16] = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    vsfhal_usart_init(PERIPHERAL_UART_EXT_IDX);
    vsfhal_usart_config(PERIPHERAL_UART_EXT_IDX, PERIPHERAL_UART_BAUD_DEFAULT, PERIPHERAL_UART_MODE_DEFAULT);
    vsfhal_usart_tx_bytes(PERIPHERAL_UART_EXT_IDX, test_ext, sizeof(test_ext));
    vsfhal_usart_config_cb(PERIPHERAL_UART_EXT_IDX, PERIPHERAL_UART_EXT_PRIORITY, NULL, NULL, NULL);
#endif
}

#ifdef APP_CFG_CDCEXT_SUPPORT
// USB_CDCACM_REQ_SET_LINE_CODING
static vsf_err_t usrapp_cdcext_set_line_coding(usb_cdcacm_line_coding_t *line_coding)
{
/*
struct usb_cdcacm_line_coding_t {
    uint32_t bitrate;
    uint8_t stop;
    uint8_t parity;
    uint8_t datalen;
};
	bitrate:
		eg. 115200
	stop:
		0 - 1bit
		1 - 1.5bit
		2 - 2bit
	parity:
		0 - None
		1 - Odd
		2 - Even
		3 - Mark
		4 - Space
	datalen:
		5, 6, 7, 8, 16
*/
    uint32_t baudrate = line_coding->bitrate;
    uint32_t mode = PERIPHERAL_UART_BITLEN_8;

    if (baudrate > PERIPHERAL_UART_EXT_BAUD_MAX)
        baudrate = PERIPHERAL_UART_EXT_BAUD_MAX;
    else if (baudrate < PERIPHERAL_UART_EXT_BAUD_MIN)
        baudrate = PERIPHERAL_UART_EXT_BAUD_MIN;
    
    if (line_coding->stop == 1)
        mode |= PERIPHERAL_UART_STOPBITS_1P5;
    else if (line_coding->stop == 2)
        mode |= PERIPHERAL_UART_STOPBITS_2;
    else
        mode |= PERIPHERAL_UART_STOPBITS_1;

    if (line_coding->parity == 1)
        mode |= PERIPHERAL_UART_PARITY_ODD;
    else if (line_coding->parity == 2)
        mode |= PERIPHERAL_UART_PARITY_EVEN;
    else
        mode |= PERIPHERAL_UART_PARITY_NONE;

    usrapp_config_usart(PERIPHERAL_UART_EXT_IDX, &mode, &baudrate,
            (vsf_stream_t *)&cdcext_usb2ext_stream,
            (vsf_stream_t *)&cdcext_ext2usb_stream, false);

    return VSF_ERR_NONE;
}

// USB_CDCACM_REQ_SET_CONTROL_LINE_STATE
static vsf_err_t usrapp_cdcext_set_control_line(uint8_t control_line)
{
/*
	control_line:
		bit0: dtr
		bit1: rts
*/
    #ifdef PERIPHERAL_UART_EXT_DTR_IDX
    if (control_line & (0x1 << 0))
        vsfhal_gpio_set(PERIPHERAL_UART_EXT_DTR_IDX, 1 << PERIPHERAL_UART_EXT_DTR_PIN);
    else
        vsfhal_gpio_clear(PERIPHERAL_UART_EXT_DTR_IDX, 1 << PERIPHERAL_UART_EXT_DTR_PIN);
    #endif
    #ifdef PERIPHERAL_UART_EXT_RTS_IDX
    if (control_line & (0x1 << 1))
        vsfhal_gpio_set(PERIPHERAL_UART_EXT_RTS_IDX, 1 << PERIPHERAL_UART_EXT_RTS_PIN);
    else
        vsfhal_gpio_clear(PERIPHERAL_UART_EXT_RTS_IDX, 1 << PERIPHERAL_UART_EXT_RTS_PIN);
    #endif
    return VSF_ERR_NONE;
}
#endif

#ifdef APP_CFG_CDCSHELL_SUPPORT
static vsf_err_t usrapp_cdcshell_set_line_coding(usb_cdcacm_line_coding_t *line_coding)
{
    uint32_t baudrate = line_coding->bitrate;
    uint32_t mode = PERIPHERAL_UART_BITLEN_8;

    if (baudrate > PERIPHERAL_UART_EXT_BAUD_MAX)
        baudrate = PERIPHERAL_UART_EXT_BAUD_MAX;
    else if (baudrate < PERIPHERAL_UART_EXT_BAUD_MIN)
        baudrate = PERIPHERAL_UART_EXT_BAUD_MIN;
    
    if (line_coding->stop == 1)
        mode |= PERIPHERAL_UART_STOPBITS_1P5;
    else if (line_coding->stop == 2)
        mode |= PERIPHERAL_UART_STOPBITS_2;
    else
        mode |= PERIPHERAL_UART_STOPBITS_1;

    if (line_coding->parity == 1)
        mode |= PERIPHERAL_UART_PARITY_ODD;
    else if (line_coding->parity == 2)
        mode |= PERIPHERAL_UART_PARITY_EVEN;
    else
        mode |= PERIPHERAL_UART_PARITY_NONE;

    usrapp.cdc_shell_usart_mode = mode;
    usrapp.cdc_shell_usart_baud = baudrate;
    return VSF_ERR_NONE;
}
#endif

#ifdef APP_CFG_CDCSHELL_SUPPORT

#endif

#if 0

static uint32_t do_test_cnt = 0;
static void do_test(void)
{
    if (do_test_cnt < 0xffffffff)
        do_test_cnt++;
}

declare_vsf_pt(user_pt_sub_t)
def_vsf_pt(user_pt_sub_t,
    def_params(
        uint32_t cnt;
    ));
private implement_vsf_pt(user_pt_sub_t) 
{
    vsf_pt_begin();

    this.cnt++;
    do_test();

    vsf_pt_end();
}


dcl_simple_class(vsf_ptshell_t)
def_simple_class(vsf_ptshell_t) {
    private_member(
        vsf_eda_t eda;
        vsf_pt(user_pt_sub_t) input_task;
    )
};

static void vsf_ptshell_evthandler(vsf_eda_t *eda, vsf_evt_t evt)
{
    vsf_ptshell_t *shell = container_of(eda, vsf_ptshell_t, eda);
    
    switch (evt) {
    case VSF_EVT_INIT:
        do_test();
        vsf_eda_post_evt(eda, VSF_EVT_USER);
        break;
    case VSF_EVT_USER:
        vsf_eda_call_pt(user_pt_sub_t, &shell->input_task);
        break;
    }
}

static vsf_ptshell_t vsf_ptshell;

void vsf_ptshell_init(vsf_ptshell_t *ptshell)
{
    const vsf_eda_cfg_t cfg = {
        .fn.evthandler  = vsf_ptshell_evthandler,
        .priority       = vsf_prio_0,
    };
    
    vsf_eda_start(&ptshell->eda, (vsf_eda_cfg_t *)&cfg);
}

#endif

int main(void)
{
#if 1
    #if VSF_USE_PTSHELL == ENABLED
    vsf_ptshell_init(&vsf_ptshell);
    #endif
    
    #ifdef APP_CFG_SERIAL_ATTACH_UUID
    usrapp_init_serial(__usrapp_usbd_vllinklite_nonconst.usbd.str_serial, sizeof(__usrapp_usbd_vllinklite_nonconst.usbd.str_serial));
    #endif

    PERIPHERAL_LED_RED_INIT();
    PERIPHERAL_LED_GREEN_INIT();
    PERIPHERAL_LED_RED_OFF();
    PERIPHERAL_LED_GREEN_OFF();

    #ifdef PERIPHERAL_UART_EXT_DTR_IDX
    vsfhal_gpio_init(PERIPHERAL_UART_EXT_DTR_IDX);
    vsfhal_gpio_config(PERIPHERAL_UART_EXT_DTR_IDX, 0x1 << PERIPHERAL_UART_EXT_DTR_PIN, PERIPHERAL_UART_EXT_DTR_CONFIG);
    vsfhal_gpio_set(PERIPHERAL_UART_EXT_DTR_IDX, 1 << PERIPHERAL_UART_EXT_DTR_PIN);
    #endif
    #ifdef PERIPHERAL_UART_EXT_RTS_IDX
    vsfhal_gpio_init(PERIPHERAL_UART_EXT_RTS_IDX);
    vsfhal_gpio_config(PERIPHERAL_UART_EXT_RTS_IDX, 0x1 << PERIPHERAL_UART_EXT_RTS_PIN, PERIPHERAL_UART_EXT_RTS_CONFIG);
    vsfhal_gpio_set(PERIPHERAL_UART_EXT_RTS_IDX, 1 << PERIPHERAL_UART_EXT_RTS_PIN);
    #endif

    #if VENDOR_UART
    VSF_STREAM_INIT(&usrapp.dap.dap_param.ext_tx);
    VSF_STREAM_INIT(&usrapp.dap.dap_param.ext_rx);
    VSF_STREAM_INIT(&usrapp.dap.dap_param.swo_tx);
    #endif
    #if SWO_UART
    VSF_STREAM_INIT(&usrapp.dap.dap_param.swo_rx);
    #endif


    #ifdef APP_CFG_CDCEXT_SUPPORT
    VSF_STREAM_INIT(&cdcext_ext2usb_stream);
    VSF_STREAM_INIT(&cdcext_usb2ext_stream);
    #endif
    #ifdef APP_CFG_CDCSHELL_SUPPORT
    VSF_STREAM_INIT(&cdcshell_shell2usb_stream);
    VSF_STREAM_INIT(&cdcshell_usb2shell_stream);
    #endif

    dap_init(&usrapp.dap, vsf_prio_0);

    vk_usbd_init(&__usrapp_usbd_vllinklite.usbd.dev);
    vk_usbd_disconnect(&__usrapp_usbd_vllinklite.usbd.dev);
    
    cb_timer.on_timer = connect_usbd;
    vsf_callback_timer_add_ms(&cb_timer, 100);

    #ifdef APP_CFG_CDCSHELL_SUPPORT
    #endif
#endif
    return 0;
}

#if DAP_SRST_CLEAR_ATTACH_SYSRESETREQ
#define APBANKSEL      0x000000F0  // APBANKSEL Mask

// SWD register access
#define SWD_REG_AP        (1)
#define SWD_REG_DP        (0)
#define SWD_REG_R         (1<<1)
#define SWD_REG_W         (0<<1)
#define SWD_REG_ADR(a)    (a & 0x0c)

#define AP_CSW         0x00        // Control and Status Word

// AP Control and Status Word definitions
#define CSW_SIZE       0x00000007  // Access Size: Selection Mask
#define CSW_SIZE8      0x00000000  // Access Size: 8-bit
#define CSW_SIZE16     0x00000001  // Access Size: 16-bit
#define CSW_SIZE32     0x00000002  // Access Size: 32-bit
#define CSW_ADDRINC    0x00000030  // Auto Address Increment Mask
#define CSW_NADDRINC   0x00000000  // No Address Increment
#define CSW_SADDRINC   0x00000010  // Single Address Increment
#define CSW_PADDRINC   0x00000020  // Packed Address Increment
#define CSW_DBGSTAT    0x00000040  // Debug Status
#define CSW_TINPROG    0x00000080  // Transfer in progress
#define CSW_HPROT      0x02000000  // User/Privilege Control
#define CSW_MSTRTYPE   0x20000000  // Master Type Mask
#define CSW_MSTRCORE   0x00000000  // Master Type: Core
#define CSW_MSTRDBG    0x20000000  // Master Type: Debug
#define CSW_RESERVED   0x01000000  // Reserved Value

#define CSW_VALUE (CSW_RESERVED | CSW_MSTRDBG | CSW_HPROT | CSW_DBGSTAT | CSW_SADDRINC)

static uint32_t SWD_Transfer(uint32_t request, uint32_t *data)
{
    if (request & DAP_TRANSFER_RnW) {
        return vsfhal_swd_read(request, (uint8_t *)data);
    } else {
        return vsfhal_swd_write(request, (uint8_t *)data);
    }
}

// Write debug port register
static uint8_t swd_write_dp(uint8_t adr, uint32_t val)
{
    uint32_t req;
    uint8_t ack;

    //check if the right bank is already selected
    if (adr == DP_SELECT) {
        return 1;
    }

    req = SWD_REG_DP | SWD_REG_W | SWD_REG_ADR(adr);
    ack = SWD_Transfer(req, (uint32_t *)&val);
    return (ack == 0x01);
}

// Write access port register
static uint8_t swd_write_ap(uint32_t adr, uint32_t val)
{
    uint8_t req, ack;
    uint32_t apsel = adr & 0xff000000;
    uint32_t bank_sel = adr & APBANKSEL;

    if (!swd_write_dp(DP_SELECT, apsel | bank_sel)) {
        return 0;
    }

    req = SWD_REG_AP | SWD_REG_W | SWD_REG_ADR(adr);
    if (SWD_Transfer(req, (uint32_t *)&val) != 0x01) {
        return 0;
    }

    req = SWD_REG_DP | SWD_REG_R | SWD_REG_ADR(DP_RDBUFF);
    ack = SWD_Transfer(req, NULL);
    return (ack == 0x01);
}

// Write target memory.
static uint8_t swd_write_data(uint32_t address, uint32_t data)
{
    uint8_t req, ack;
    
    req = SWD_REG_AP | SWD_REG_W | (1 << 2);
    if (SWD_Transfer(req, &address) != 0x01) {
        return 0;
    }

    // write data
    req = SWD_REG_AP | SWD_REG_W | (3 << 2);
    if (SWD_Transfer(req, &data) != 0x01) {
        return 0;
    }

    // dummy read
    req = SWD_REG_DP | SWD_REG_R | SWD_REG_ADR(DP_RDBUFF);
    ack = SWD_Transfer(req, NULL);
    return (ack == 0x01) ? 1 : 0;
}

void dap_srst_clear_attach_handler(void)
{
    uint32_t addr = (uint32_t)&SCB->AIRCR;
    uint32_t val = (0x5FAul << SCB_AIRCR_VECTKEY_Pos) | SCB_AIRCR_SYSRESETREQ_Msk;

    swd_write_ap(AP_CSW, CSW_VALUE | CSW_SIZE32);
    swd_write_data(addr, val);
}
#endif

/* EOF */
