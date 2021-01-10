#include "vsf.h"
#include "dap.h"

enum vender_id_rename_t {
    VENDOR_ID_GET_USART_INFO        = ID_DAP_Vendor0,
    VENDOR_ID_GET_USART_CFG         = ID_DAP_Vendor1,
    VENDOR_ID_SET_USART_CFG         = ID_DAP_Vendor2,
    VENDOR_ID_GET_USART_STATUS      = ID_DAP_Vendor3,
    VENDOR_ID_READ_USART_DATA       = ID_DAP_Vendor4,
    VENDOR_ID_WRITE_USART_DATA      = ID_DAP_Vendor5,
};

/*
VENDOR_ID_GET_USART_INFO:
    Request:                            Response:
    CMD             [1 byte]            CMD                 [1 byte]
                                        DAP_OK              [1 byte]
                                        USART NUM           [1 byte]
                                        USART0_BAUD_MIN     [4 byte]
                                        USART0_BAUD_MAX     [4 byte]
                                        USART0_VALID_CFG    [4 byte]
                                        USART1_BAUD_MIN     [4 byte]
                                        USART1_BAUD_MAX     [4 byte]
                                        USART1_VALID_CFG    [4 byte]
                                        ...

VENDOR_ID_GET_USART_CFG:
    Request:                            Response:
    CMD             [1 byte]            CMD                 [1 byte]
    USART IDX       [1 byte]            DAP_OK              [1 byte]
                                        USARTx_BAUD         [4 byte]
                                        USARTx_CUR_CFG      [4 byte]

VENDOR_ID_SET_USART_CFG:
    Request:                            Response:
    CMD             [1 byte]            CMD                 [1 byte]
    USART IDX       [1 byte]            DAP_OK              [1 byte]
    USARTx_BAUD     [4 byte]            USARTx_BAUD         [4 byte]
    USARTx_CUR_CFG  [4 byte]            USARTx_CUR_CFG      [4 byte]

VENDOR_ID_GET_USART_STATUS:
    Request:                            Response:
    CMD             [1 byte]            CMD                 [1 byte]
    STATUS NUM      [1 byte]            DAP_OK              [1 byte]
    STATUS0_IDX     [1 byte]            STATUS0             [4 byte]
    STATUS1_IDX     [1 byte]            STATUS1             [4 byte]
    ...                                 ...

VENDOR_ID_READ_USART_DATA:
    Request:                            Response:
    CMD             [1 byte]            CMD                 [1 byte]
    READ NUM        [1 byte]            DAP_OK              [1 byte]
    READ0_IDX       [1 byte]            READ0_LENGTH        [2 byte]
    READ0_LENGTH    [2 byte]            READ0_DATA          [{LENGTH} byte]
    READ1_IDX       [1 byte]            READ0_LENGTH        [2 byte]
    READ1_LENGTH    [2 byte]            READ1_DATA          [{LENGTH} byte]
    ...                                 ...
                                        READ0_STATUS        [4 byte]
                                        READ1_STATUS        [4 byte]
                                        ...

VENDOR_ID_WRITE_USART_DATA:
    Request:                            Response:
    CMD             [1 byte]            CMD                 [1 byte]
    WRITE NUM       [1 byte]            DAP_OK              [1 byte]
    WRITE0_IDX      [1 byte]            WRITE0_LENGTH       [2 byte]
    WRITE0_LENGTH   [2 byte]            WRITE1_LENGTH       [2 byte]
    WRITE0_DATA     [{LENGTH} byte]     ...
    WRITE1_IDX      [1 byte]            WRITE0_STATUS       [4 byte]
    WRITE1_LENGTH   [2 byte]            WRITE0_STATUS       [4 byte]
    WRITE1_DATA     [{LENGTH} byte]     ...
    ...
*/

// TODO
uint32_t dap_vendor_request_handler(dap_param_t* param, uint8_t* request,
        uint8_t* response, uint8_t cmd_id, uint16_t remaining_size)
{
    uint16_t req_ptr = 0, resp_ptr = 0;

    switch (cmd_id) {
    case VENDOR_ID_GET_USART_INFO: {
        
    } break;
    case VENDOR_ID_GET_USART_CFG: {
        
    } break;
    case VENDOR_ID_SET_USART_CFG: {
        
    } break;
    case VENDOR_ID_GET_USART_STATUS: {
        
    } break;
    case VENDOR_ID_READ_USART_DATA: {
        
    } break;
    case VENDOR_ID_WRITE_USART_DATA: {
        
    } break;
    default:
        break;
    }
    return ((uint32_t)resp_ptr << 16) | req_ptr;
}
