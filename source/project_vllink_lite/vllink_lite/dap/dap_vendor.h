#ifndef __DAP_VENDOR_H__
#define __DAP_VENDOR_H__

#ifdef __cplusplus
extern "C" {
#endif

uint32_t dap_vendor_request_handler(dap_param_t* param, uint8_t* request,
        uint8_t* response, uint8_t cmd_id, uint16_t remaining_size);

#ifdef __cplusplus
}
#endif

#endif // __DAP_VENDOR_H__
