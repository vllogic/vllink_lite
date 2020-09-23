#include "vsf.h"
#include "dap.h"

#if SWO_UART || SWO_MANCHESTER
uint32_t dap_swo_request_handler(dap_param_t* param, uint8_t* request,
        uint8_t* response, uint8_t cmd_id, uint16_t remaining_size)
{
    uint16_t req_ptr = 0, resp_ptr = 0;

    switch (cmd_id) {
    case ID_DAP_SWO_Transport: {
        uint8_t ret = DAP_ERROR;
        uint8_t transport = request[req_ptr++];
        if (!(param->trace_status & DAP_SWO_CAPTURE_ACTIVE)) {
            #if SWO_STREAM
            if (transport <= 2) {
            #else
            if (transport <= 1) {
            #endif
                param->transport = transport;
                ret = DAP_OK;
            }
        }
        response[resp_ptr++] = ret;
    } break;
    case ID_DAP_SWO_Mode: {
        uint8_t mode = request[req_ptr++];
        // Note: not support DAP_SWO_MANCHESTER
        if ((mode == DAP_SWO_OFF) || (mode == DAP_SWO_UART)) {
            param->trace_mode = mode;
            response[resp_ptr++] = DAP_OK;
        } else {
            param->trace_mode = DAP_SWO_OFF;
            response[resp_ptr++] = DAP_ERROR;
        }
    } break;
    case ID_DAP_SWO_Baudrate: {
        uint32_t baudrate = get_unaligned_le32(request + req_ptr);
        req_ptr += 4;
        if (baudrate > SWO_UART_MAX_BAUDRATE)
            baudrate = SWO_UART_MAX_BAUDRATE;
        else if (baudrate < SWO_UART_MIN_BAUDRATE)
            baudrate = SWO_UART_MIN_BAUDRATE;
        if (param->get_usart_baud)
            baudrate = param->get_usart_baud(PERIPHERAL_UART_SWO_IDX, baudrate);
        #if (SWO_UART != 0)
        param->swo_baudrate = baudrate;
        #endif
        put_unaligned_le32(baudrate, response + resp_ptr);
        resp_ptr += 4;
    } break;
    case ID_DAP_SWO_Control: {
        uint8_t active = request[req_ptr++] & DAP_SWO_CAPTURE_ACTIVE;
        if (active != (param->trace_status & DAP_SWO_CAPTURE_ACTIVE)) {
            uint32_t mode = PERIPHERAL_UART_MODE_DEFAULT;
            if (active) {
                param->trace_o = 0;
                #if TIMESTAMP_CLOCK
                param->trace_timestamp = 0;
                #endif
                if (param->config_usart) {
                    param->config_usart(PERIPHERAL_UART_SWO_IDX, &mode, &param->swo_baudrate, NULL, (vsf_stream_t *)&param->swo_rx);
                }
                VSF_STREAM_CONNECT_RX(&param->swo_rx);
            } else {
                VSF_STREAM_DISCONNECT_RX(&param->swo_rx);
                param->config_usart(PERIPHERAL_UART_SWO_IDX, NULL, NULL, NULL, NULL);
            }
            param->trace_status = active;
        }
        response[resp_ptr++] = DAP_OK;
    } break;
    case ID_DAP_SWO_Status: {
        uint32_t count = VSF_STREAM_GET_DATA_SIZE(&param->swo_rx);
        response[resp_ptr++] = param->trace_status;
        put_unaligned_le32(count, response + resp_ptr);
        resp_ptr += 4;
    } break;
    case ID_DAP_SWO_ExtendedStatus: {
        uint8_t sb_cmd = request[req_ptr++];

        if (sb_cmd & 0x1)
            response[resp_ptr++] = param->trace_status;
        if (sb_cmd & 0x2) {
            uint32_t count = VSF_STREAM_GET_DATA_SIZE(&param->swo_rx);
            put_unaligned_le32(count, response + resp_ptr);
            resp_ptr += 4;
        }
        #if TIMESTAMP_CLOCK
        if (sb_cmd & 0x4) {
            put_unaligned_le32(param->trace_o + VSF_STREAM_GET_DATA_SIZE(&param->swo_rx), response + resp_ptr);
            put_unaligned_le32(param->trace_timestamp, response + resp_ptr + 4);
            resp_ptr += 8;
        }
        #endif
    } break;
    case ID_DAP_SWO_Data: {
        int32_t count = min(get_unaligned_le16(request + req_ptr),
                VSF_STREAM_GET_DATA_SIZE(&param->swo_rx));
        req_ptr += 2;
        if (count >= 16)
            __ASM("NOP");
        response[resp_ptr++] = param->trace_status;
        if (param->transport != 1)	// Read trace data via DAP_SWO_Data command
            count = 0;
        if (count > (remaining_size - 3))
            count = remaining_size - 3;
        if (count) {
            count = VSF_STREAM_READ(&param->swo_rx, response + resp_ptr + 2, count);
        }
        put_unaligned_le16(count, response + resp_ptr);
        resp_ptr += 2 + count;
        param->trace_o += count;
    } break;
    default:
        break;
    }
    return ((uint32_t)resp_ptr << 16) | req_ptr;
}
#endif