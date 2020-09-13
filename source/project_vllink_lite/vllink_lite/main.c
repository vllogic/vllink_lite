
#define __VSF_USBD_CLASS_INHERIT__

/*============================ INCLUDES ======================================*/

#include "vsf.h"
#include "dap.h"
#include "ramuart.h"
#include "vsf_usbd_cmsis_dap_v2.h"
#include "vsf_usbd_webusb_usr.h"


/*============================ PROTOTYPES ====================================*/

static uint16_t usrapp_get_serial(uint8_t *serial);
static void usrapp_config_usart(enum usart_idx_t idx, uint32_t *mode, uint32_t *baudrate, vsf_stream_t *tx, vsf_stream_t *rx);
static vsf_err_t usrapp_cdcext_set_line_coding(usb_cdcacm_line_coding_t *line_coding);
static vsf_err_t usrapp_cdcshell_set_line_coding(usb_cdcacm_line_coding_t *line_coding);

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

typedef struct usrapp_t {
    dap_t dap;

    uint32_t usart_ext_mode;
    uint32_t usart_ext_baud;
    uint32_t usart_swo_mode;
    uint32_t usart_swo_baud;
    uint32_t cdc_shell_usart_mode;
    uint32_t cdc_shell_usart_baud;
} usrapp_t;

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/

usrapp_t usrapp                 = {
    .dap.dap_param              = {
        .get_serial             = usrapp_get_serial,
        .config_usart           = usrapp_config_usart,
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
    },
};

/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

#include "usrapp_usbd_common.c"
#include "usrapp_usbd_vllinklite.c"

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

static uint16_t usrapp_get_serial(uint8_t *serial)
{
    uint16_t i, size = (sizeof(__usrapp_usbd_vllinklite_nonconst.usbd.str_serial) - 2) / 2;
    if (serial) {
        for (i = 0; i < size; i++)
            serial[i] = __usrapp_usbd_vllinklite_nonconst.usbd.str_serial[i * 2 + 2];
    }
    return size;
}

static void usrapp_config_usart(enum usart_idx_t idx, uint32_t *mode, uint32_t *baudrate, vsf_stream_t *tx, vsf_stream_t *rx)
{
    switch (idx) {
    case PERIPHERAL_UART_EXT_IDX:
        if ((mode && *mode != usrapp.usart_ext_mode) || (baudrate && *baudrate != usrapp.usart_ext_baud)) {
            if (mode)
                usrapp.usart_ext_mode = *mode;
            if (baudrate)
                usrapp.usart_ext_baud = *baudrate;
            vsfhal_usart_init(PERIPHERAL_UART_EXT_IDX);
            vsfhal_usart_config(PERIPHERAL_UART_EXT_IDX, usrapp.usart_ext_baud, usrapp.usart_ext_mode);
        }
        vsfhal_usart_stream_init(PERIPHERAL_UART_EXT_IDX, PERIPHERAL_UART_EXT_PRIORITY, tx, rx);
        if (!mode && !baudrate)
            vsfhal_usart_fini(PERIPHERAL_UART_EXT_IDX);
        break;
    case PERIPHERAL_UART_SWO_IDX:
        if ((mode && *mode != usrapp.usart_swo_mode) || (baudrate && *baudrate != usrapp.usart_swo_baud)) {
            if (mode)
                usrapp.usart_swo_mode = *mode;
            if (baudrate)
                usrapp.usart_swo_baud = *baudrate;
            vsfhal_usart_init(PERIPHERAL_UART_SWO_IDX);
            vsfhal_usart_config(PERIPHERAL_UART_SWO_IDX, usrapp.usart_swo_baud, usrapp.usart_swo_mode);
        }
        vsfhal_usart_stream_init(PERIPHERAL_UART_SWO_IDX, PERIPHERAL_UART_SWO_PRIORITY, tx, rx);
        if (!mode && !baudrate)
            vsfhal_usart_fini(PERIPHERAL_UART_SWO_IDX);
        break;
    }
}

static vsf_callback_timer_t cb_timer;

#define DAP_TEST_PORT                   0   // 0, DAP_PORT_SWD, DAP_PORT_JTAG
#define DAP_TEST_SPEED_KHZ              8000
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
}

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
            (vsf_stream_t *)&__usrapp_usbd_vllinklite.usbd.cdcext.usb2ext,
            (vsf_stream_t *)&__usrapp_usbd_vllinklite.usbd.cdcext.ext2usb);

    return VSF_ERR_NONE;
}

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

int main(void)
{
    usrapp_init_serial(__usrapp_usbd_vllinklite_nonconst.usbd.str_serial, sizeof(__usrapp_usbd_vllinklite_nonconst.usbd.str_serial));

    PERIPHERAL_LED_RED_INIT();
    PERIPHERAL_LED_GREEN_INIT();
    PERIPHERAL_LED_RED_OFF();
    PERIPHERAL_LED_GREEN_OFF();

    #if VENDOR_UART
    VSF_STREAM_INIT(&usrapp.dap.dap_param.ext_tx);
    VSF_STREAM_INIT(&usrapp.dap.dap_param.ext_rx);
    VSF_STREAM_INIT(&usrapp.dap.dap_param.swo_tx);
    #endif
    #if SWO_UART
    VSF_STREAM_INIT(&usrapp.dap.dap_param.swo_rx);
    #endif

    VSF_STREAM_INIT(&__usrapp_usbd_vllinklite.usbd.cdcext.usb2ext);
    VSF_STREAM_INIT(&__usrapp_usbd_vllinklite.usbd.cdcext.ext2usb);
    VSF_STREAM_INIT(&__usrapp_usbd_vllinklite.usbd.cdcshell.usb2shell);
    VSF_STREAM_INIT(&__usrapp_usbd_vllinklite.usbd.cdcshell.shell2usb);

    dap_init(&usrapp.dap, vsf_prio_0);

    vk_usbd_init(&__usrapp_usbd_vllinklite.usbd.dev);
    vk_usbd_disconnect(&__usrapp_usbd_vllinklite.usbd.dev);
    
    cb_timer.on_timer = connect_usbd;
    vsf_callback_timer_add_ms(&cb_timer, 100);
    return 0;
}

/* EOF */
