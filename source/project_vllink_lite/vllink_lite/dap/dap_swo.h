#ifndef __DAP_SWO_H__
#define __DAP_SWO_H__

#ifdef __cplusplus
extern "C" {
#endif

uint32_t dap_swo_request_handler(dap_param_t* param, uint8_t* request,
        uint8_t* response, uint8_t cmd_id, uint16_t remaining_size);

#ifdef __cplusplus
}
#endif

#endif // __DAP_SWO_H__
