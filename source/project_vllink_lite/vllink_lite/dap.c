#include "vsf.h"
#include "dap.h"

#ifdef DAP_VENDOR
const char DAP_Vendor[] = DAP_VENDOR;
#endif
#ifdef DAP_PRODUCT
const char DAP_Product[] = DAP_PRODUCT;
#endif
#ifdef DAP_FW_VER
const char DAP_FW_Ver[] = DAP_FW_VER;
#endif
#ifdef DAP_DEVICE_VENDOR
const char DAP_DEVICE_Vendor[] = DAP_DEVICE_VENDOR;
#endif
#ifdef DAP_DEVICE_NAME
const char DAP_DEVICE_Name[] = DAP_DEVICE_NAME;
#endif

#if TIMESTAMP_CLOCK
uint32_t dap_timestamp;
#endif

#ifdef JTAG_ASYNC
static uint64_t buf_tms[2], buf_tdi[2], buf_tdo[2];
#else
static uint64_t buf_tms, buf_tdi, buf_tdo;
#endif

static vsf_err_t port_init(dap_param_t* param, uint8_t port)
{
    if (port == DAP_PORT_SWD) {
        vsfhal_swd_init(PERIPHERAL_SWD_PRIORITY);
        vsfhal_swd_config(param->speed_khz, param->transfer.retry_count,
            param->transfer.idle_cycles, param->swd_conf.turnaround,
            param->swd_conf.data_phase);
    } else if (port == DAP_PORT_JTAG) {
        vsfhal_jtag_init(PERIPHERAL_JTAG_PRIORITY);
        vsfhal_jtag_config(param->speed_khz, param->transfer.retry_count,
            param->transfer.idle_cycles);
    }
    param->port_io_need_reconfig = false;
    return VSF_ERR_NONE;
}

static vsf_err_t port_fini(uint8_t port)
{
    if (port == DAP_PORT_SWD) {
        vsfhal_swd_fini();
    } else if (port == DAP_PORT_JTAG) {
        vsfhal_jtag_fini();
    }
    return VSF_ERR_NONE;
}

static vsf_err_t port_io_reconfig(dap_param_t* param, uint8_t port)
{
    if (port == DAP_PORT_SWD) {
        vsfhal_swd_io_reconfig();
    } else if (port == DAP_PORT_JTAG) {
        vsfhal_jtag_io_reconfig();
    }
    param->port_io_need_reconfig = false;
    return VSF_ERR_NONE;
}

static uint8_t get_dap_info(dap_param_t* param, uint8_t id, uint8_t* info, uint16_t pkt_size)
{
    uint8_t length = 0U;

    switch (id) {
    case DAP_ID_VENDOR:
#ifdef DAP_VENDOR
        if (info)
            memcpy(info, DAP_Vendor, sizeof(DAP_Vendor));
        length = sizeof(DAP_Vendor);
#endif
        break;
    case DAP_ID_PRODUCT:
#ifdef DAP_PRODUCT
        if (info)
            memcpy(info, DAP_Product, sizeof(DAP_Product));
        length = (uint8_t)sizeof(DAP_Product);
#endif
        break;
    case DAP_ID_SER_NUM:
        if (param->get_serial)
            length = param->get_serial(info);
        else
            length = 0;
        break;
    case DAP_ID_FW_VER:
#ifdef DAP_FW_VER
        if (info)
            memcpy(info, DAP_FW_VER, sizeof(DAP_FW_VER));
        length = (uint8_t)sizeof(DAP_FW_VER);
#endif
        break;
    case DAP_ID_DEVICE_VENDOR:
#ifdef DAP_DEVICE_VENDOR
        if (info)
            memcpy(info, DAP_DEVICE_Vendor, sizeof(DAP_DEVICE_Vendor));
        length = (uint8_t)sizeof(DAP_DEVICE_Vendor);
#endif
        break;
    case DAP_ID_DEVICE_NAME:
#ifdef DAP_DEVICE_NAME
        if (info)
            memcpy(info, DAP_DEVICE_Name, sizeof(DAP_DEVICE_Name));
        length = (uint8_t)sizeof(DAP_DEVICE_Name);
#endif
        break;
    case DAP_ID_CAPABILITIES:
        if (info)
            info[0] = (DAP_SWD ? (1U << 0) : 0U) | (DAP_JTAG ? (1U << 1) : 0U) | (SWO_UART ? (1U << 2) : 0U) | (SWO_MANCHESTER ? (1U << 3) : 0U) |
                /* Atomic Commands  */ (1U << 4) | (TIMESTAMP_CLOCK ? (1U << 5) : 0U) | (SWO_STREAM ? (1U << 6) : 0U);
        length = 1U;
        break;
    case DAP_ID_TIMESTAMP_CLOCK:
#if TIMESTAMP_CLOCK
        if (info)
            put_unaligned_le32(TIMESTAMP_CLOCK, info);
        length = 4U;
#endif
        break;
    case DAP_ID_SWO_BUFFER_SIZE:
#if SWO_UART || SWO_MANCHESTER
        if (info)
            put_unaligned_le32(SWO_BUFFER_SIZE, info);
        length = 4U;
#endif
        break;
    case DAP_ID_PACKET_SIZE:
        if (info)
            put_unaligned_le16(pkt_size, info);
        length = 2U;
        break;
    case DAP_ID_PACKET_COUNT:
        if (info)
            info[0] = DAP_PACKET_COUNT;
        length = 1U;
        break;
    default:
        break;
    }

    return length;
}

#ifdef JTAG_ASYNC
static uint16_t jtag_ir_to_raw(uint64_t *buf_tms, uint64_t *buf_tdi, uint32_t ir, uint8_t lr_length, uint16_t ir_before, uint16_t ir_after)
{
    uint16_t bitlen;

    *buf_tdi = 0;
    lr_length--;

    // Select-DR-Scan, Select-IR-Scan, Capture-IR, Shift-IR
    *buf_tms = (uint64_t)0x3 << 0;
    bitlen = 4;
    // Bypass before data
    if (ir_before) {
        *buf_tdi |= (((uint64_t)0x1 << ir_before) - 1) << bitlen;
        bitlen += ir_before;
    }
    // Set IR bitlen
    if (lr_length) {
        *buf_tdi |= (ir & (((uint64_t)0x1 << lr_length) - 1)) << bitlen;
        bitlen += lr_length;
    }
    // Bypass after data
    if (ir_after) {
        *buf_tdi |= ((ir >> lr_length) & 0x1) << bitlen;
        bitlen++;
        ir_after--;
        if (ir_after) {
            *buf_tdi |= (((uint64_t)0x1 << ir_after) - 1) << bitlen;
            bitlen += ir_after;
        }
        *buf_tms |= (uint64_t)0x1 << bitlen;
        *buf_tdi |= (uint64_t)0x1 << bitlen;
        bitlen++;
    } else {
        *buf_tms |= (uint64_t)0x1 << bitlen;
        *buf_tdi |= ((ir >> lr_length) & 0x1) << bitlen;
        bitlen++;
    }

    // Exit1-IR, Update-IR
    *buf_tms |= (uint64_t)0x1 << bitlen;
    bitlen += 1;
    // idle
    *buf_tdi |= (uint64_t)0x1 << bitlen;	// keep tdi high
    return bitlen + 1;
}

static uint16_t jtag_dr_to_raw(uint64_t *buf_tms, uint64_t *buf_tdi, uint8_t request, uint32_t dr, uint16_t dr_before, uint16_t dr_after, uint8_t idle)
{
    uint16_t bitlen;

    // Select-DR-Scan, Capture-DR, Shift-DR
    *buf_tms = (uint64_t)0x1 << 0;
    // Bypass before data
    bitlen = 3 + dr_before;
    // RnW, A2, A3
    *buf_tdi = (uint64_t)((request >> 1) & 0x7) << bitlen;
    bitlen += 3;
    // Data Transfer
    if (!(request & DAP_TRANSFER_RnW))
        *buf_tdi |= (uint64_t)dr << bitlen;
    bitlen += 31 + dr_after;		
    *buf_tms |= (uint64_t)0x1 << bitlen;
    bitlen++;
    // Update-DR, Idle
    *buf_tms |= (uint64_t)0x1 << bitlen;
    bitlen += 1 + idle;
    *buf_tdi |= (uint64_t)0x1 << bitlen;	// keep tdi high
    return bitlen + 1;
}
#endif

static uint16_t request_handler(dap_param_t* param, uint8_t* request,
        uint8_t* response, uint16_t pkt_size)
{
    uint8_t cmd_id, cmd_num;
    uint16_t req_ptr, resp_ptr;

    req_ptr = 0;
    resp_ptr = 0;
    cmd_num = 1;

    do {
        cmd_num--;
        cmd_id = request[req_ptr++];
        response[resp_ptr++] = cmd_id;

        if ((cmd_id >= ID_DAP_Vendor0) && (cmd_id <= ID_DAP_Vendor31)) {
            // TODO
        } else {
            switch (cmd_id) {
            case ID_DAP_ExecuteCommands:
                cmd_num = request[req_ptr++];
                response[resp_ptr++] = cmd_num;
                break;
            case ID_DAP_Info: {
                uint8_t id = request[req_ptr++];
                if (resp_ptr <= (pkt_size - 1 - get_dap_info(param, id, NULL, pkt_size))) {
                    uint8_t num = get_dap_info(param, id, &response[resp_ptr + 1], pkt_size);
                    response[resp_ptr] = num;
                    resp_ptr += num + 1;
                } else {
                    goto fault;
                }
            } break;
            case ID_DAP_HostStatus: {
                uint8_t status = request[req_ptr++];
                if (status == DAP_DEBUGGER_CONNECTED) {
                    if (request[req_ptr++] & 0x1)
                        PERIPHERAL_LED_RED_ON();
                    else
                        PERIPHERAL_LED_RED_OFF();
                    response[resp_ptr++] = DAP_OK;
                } else if (status == DAP_TARGET_RUNNING) {
                    if (request[req_ptr++] & 0x1)
                        PERIPHERAL_LED_GREEN_ON();
                    else
                        PERIPHERAL_LED_GREEN_OFF();
                    response[resp_ptr++] = DAP_OK;
                } else {
                    response[resp_ptr++] = DAP_ERROR;
                }
            } break;
            case ID_DAP_Connect: {
                uint8_t port = request[req_ptr++];
                if (port == DAP_PORT_AUTODETECT)
                    port = DAP_DEFAULT_PORT;

                if (port == DAP_PORT_SWD) {
                    port_fini(param->port);
                    if (port_init(param, DAP_PORT_SWD) == VSF_ERR_NONE)
                        port = DAP_PORT_SWD;
                    else
                        port = DAP_PORT_DISABLED;
                } else if (port == DAP_PORT_JTAG) {
                    port_fini(param->port);
                    if (port_init(param, DAP_PORT_JTAG) == VSF_ERR_NONE)
                        port = DAP_PORT_JTAG;
                    else
                        port = DAP_PORT_DISABLED;
                } else {
                    port = DAP_PORT_DISABLED;
                }
                param->port = port;
                response[resp_ptr++] = port;
            } break;
            case ID_DAP_Disconnect:
                port_fini(param->port);
                param->port = DAP_PORT_DISABLED;
                response[resp_ptr++] = DAP_OK;
                break;
            case ID_DAP_Delay: {
                vsf_systimer_cnt_t tick = vsf_systimer_get_tick() + vsf_systimer_us_to_tick(get_unaligned_le16(request + req_ptr));
                req_ptr += 2;
                while (tick > vsf_systimer_get_tick());
                response[resp_ptr++] = DAP_OK;
            } break;
            case ID_DAP_ResetTarget:
                response[resp_ptr++] = DAP_OK;
                response[resp_ptr++] = 0; // TODO: reset target return
                break;
            case ID_DAP_SWJ_Pins: {
                uint8_t value, select;
                uint64_t delay_us = get_unaligned_le32(request + req_ptr + 2);
                value = request[req_ptr];
                select = request[req_ptr + 1];
                req_ptr += 6;

                if (select & (1U << DAP_SWJ_SWCLK_TCK)) {
                    if (value & (0x1 << DAP_SWJ_SWCLK_TCK))
                        PERIPHERAL_GPIO_TCK_SET();
                    else
                        PERIPHERAL_GPIO_TCK_CLEAR();
                    PERIPHERAL_GPIO_TCK_SET_OUTPUT();
                }
                if (select & (1U << DAP_SWJ_SWDIO_TMS)) {
                    if (value & (0x1 << DAP_SWJ_SWDIO_TMS))
                        PERIPHERAL_GPIO_TMS_SET();
                    else
                        PERIPHERAL_GPIO_TMS_CLEAR();
                    PERIPHERAL_GPIO_TMS_SET_OUTPUT();
                }
                if (select & (0x1 << DAP_SWJ_nRESET)) {
                    if (value & (0x1 << DAP_SWJ_nRESET))
                        PERIPHERAL_GPIO_SRST_SET();
                    else
                        PERIPHERAL_GPIO_SRST_CLEAR();
                    PERIPHERAL_GPIO_SRST_SET_OUTPUT();
                }
                if (param->port == DAP_PORT_JTAG) {
                    if (select & (0x1 << DAP_SWJ_TDI)) {
                        if (value & (0x1 << DAP_SWJ_TDI))
                            PERIPHERAL_GPIO_TDI_SET();
                        else
                            PERIPHERAL_GPIO_TDI_CLEAR();
                        PERIPHERAL_GPIO_TDI_SET_OUTPUT();
                    }
                    if (select & (0x1 << DAP_SWJ_nTRST)) {
                        if (value & (0x1 << DAP_SWJ_nTRST))
                            PERIPHERAL_GPIO_TRST_SET();
                        else
                            PERIPHERAL_GPIO_TRST_CLEAR();
                        PERIPHERAL_GPIO_TRST_SET_OUTPUT();
                    }
                }

                if (delay_us) // us
                {
                    if (delay_us > 3000000)
                        delay_us = 3000000;
                    delay_us = vsf_systimer_get_tick() + vsf_systimer_us_to_tick(delay_us);;
                    do {
                        if (select & (1U << DAP_SWJ_SWCLK_TCK)) {
                            if (((value >> DAP_SWJ_SWCLK_TCK) & 0x1) ^ PERIPHERAL_GPIO_TDI_READ())
                                continue;
                        }
                        if (select & (1U << DAP_SWJ_SWDIO_TMS)) {
                            if (((value >> DAP_SWJ_SWDIO_TMS) & 0x1) ^ PERIPHERAL_GPIO_TMS_READ())
                                continue;
                        }
                        if (select & (1U << DAP_SWJ_TDI)) {
                            if (((value >> DAP_SWJ_TDI) & 0x1) ^ PERIPHERAL_GPIO_TCK_READ())
                                continue;
                        }
                        if (select & (1U << DAP_SWJ_nTRST)) {
                            if (((value >> DAP_SWJ_nTRST) & 0x1) ^ PERIPHERAL_GPIO_TRST_READ())
                                continue;
                        }
                        if (select & (1U << DAP_SWJ_nRESET)) {
                            if (((value >> DAP_SWJ_nRESET) & 0x1) ^ PERIPHERAL_GPIO_SRST_READ())
                                continue;
                        }
                        break;
                    } while (delay_us > vsf_systimer_get_tick());
                }
                param->port_io_need_reconfig = true;
                response[resp_ptr++] = (PERIPHERAL_GPIO_TCK_READ() ?  (0x1 << DAP_SWJ_SWCLK_TCK) : 0) |
                                       (PERIPHERAL_GPIO_TMS_READ() ? (0x1 << DAP_SWJ_SWDIO_TMS) : 0) |
                                       (PERIPHERAL_GPIO_TDI_READ() ? (0x1 << DAP_SWJ_TDI) : 0) |
                                       (PERIPHERAL_GPIO_TDO_READ() ? (0x1 << DAP_SWJ_TDO) : 0) |
                                       (PERIPHERAL_GPIO_TRST_READ() ? (0x1 << DAP_SWJ_nTRST) : 0) |
                                       (PERIPHERAL_GPIO_SRST_READ() ? (0x1 << DAP_SWJ_nRESET) : 0);
            } break;
            case ID_DAP_SWJ_Clock: {
                uint32_t speed_khz = get_unaligned_le32(request + req_ptr) / 1000;
                req_ptr += 4;
                if (!speed_khz)
                    speed_khz = 1;
                param->speed_khz = speed_khz;
                if (param->port == DAP_PORT_SWD)
                    vsfhal_swd_config(param->speed_khz, param->transfer.retry_count,
                            param->transfer.idle_cycles,
                            param->swd_conf.turnaround,
                            param->swd_conf.data_phase);
                else if (param->port == DAP_PORT_JTAG)
                    vsfhal_jtag_config(param->speed_khz, param->transfer.retry_count,
                            param->transfer.idle_cycles);
                response[resp_ptr++] = DAP_OK;
            } break;
            case ID_DAP_SWJ_Sequence: {
                uint16_t bitlen = request[req_ptr++];

                if (param->port_io_need_reconfig)
                    port_io_reconfig(param, param->port);

                if (!bitlen)
                    bitlen = 256;

                if (param->port == DAP_PORT_SWD) {
                    #ifdef SWD_ASYNC
                    vsfhal_swd_clear();
                    #endif
                    vsfhal_swd_seqout(request + req_ptr, bitlen);
                    req_ptr += (bitlen + 7) >> 3;
                    #ifdef SWD_ASYNC
                    vsfhal_swd_wait();
                    #endif
                    response[resp_ptr++] = DAP_OK;
                } else if (param->port == DAP_PORT_JTAG) {
                    uint8_t bytes;
                    
                    #ifdef JTAG_ASYNC
                    vsfhal_jtag_clear();
                    bitlen = min(bitlen, 64);
                    if (bitlen < 8) {
                        vsfhal_jtag_raw_less_8bit(bitlen, request[req_ptr++], 0xff);
                        response[resp_ptr++] = DAP_OK;
                    } else {
                        bytes = (bitlen + 7) >> 3;
                        buf_tdi[0] = 0xffffffffffffffff;
                        vsfhal_jtag_raw(0, bitlen, request + req_ptr, (uint8_t*)&buf_tdi[0], (uint8_t*)&buf_tdo[0]);
                        req_ptr += bytes;
                        vsfhal_jtag_wait();   // finish previous
                        response[resp_ptr++] = DAP_OK;
                    }
                    #else
                    bitlen = min(bitlen, 64);
                    bytes = (bitlen + 7) >> 3;
                    memcpy(&buf_tms, request + req_ptr, bytes);
                    req_ptr += bytes;
                    buf_tdi = 0xffffffffffffffff;
                    vsfhal_jtag_raw(bitlen, (uint8_t*)&buf_tms, (uint8_t*)&buf_tdi, (uint8_t*)&buf_tdo);
                    response[resp_ptr++] = DAP_OK;
                    #endif
                } else {
                    req_ptr += (bitlen + 7) >> 3;
                    response[resp_ptr++] = DAP_ERROR;
                }
            } break;
            case ID_DAP_SWD_Configure: {
                uint8_t config = request[req_ptr++];
                param->swd_conf.turnaround = (config & 0x03) + 1;
                param->swd_conf.data_phase = (config & 0x04) ? 1 : 0;
                vsfhal_swd_config(param->speed_khz, param->transfer.retry_count,
                        param->transfer.idle_cycles,
                        param->swd_conf.turnaround,
                        param->swd_conf.data_phase);
                response[resp_ptr++] = DAP_OK;
            } break;
            case ID_DAP_SWD_Sequence: {
                if (param->port == DAP_PORT_SWD) {
                    response[resp_ptr++] = DAP_OK;

                    #ifdef SWD_ASYNC
                    vsfhal_swd_clear();
                    #endif
                    
                    uint16_t transfer_cnt = 0;
                    uint16_t transfer_num = request[req_ptr++];

                    while (transfer_cnt < transfer_num) {
                        uint8_t info, bitlen, bytes;
                        info = request[req_ptr++];
                        bitlen = info & SWD_SEQUENCE_CLK;
                        if (!bitlen)
                            bitlen = 64;
                        bytes = (bitlen + 7) >> 3;
                        if (info & SWD_SEQUENCE_DIN) {
                            vsfhal_swd_seqin(response + resp_ptr, bitlen);
                            resp_ptr += bytes;
                        } else {
                            vsfhal_swd_seqout(request + req_ptr, bitlen);
                            req_ptr += bytes;
                        }
                        transfer_cnt++;
                    }
                    #ifdef SWD_ASYNC
                    vsfhal_swd_wait();
                    #endif
                } else {
                    response[resp_ptr++] = DAP_ERROR;
                }
            } break;
            case ID_DAP_JTAG_Sequence: {
                if (param->port == DAP_PORT_JTAG) {
                    #ifdef JTAG_ASYNC
                    vsfhal_jtag_clear();
                    #endif
                    
                    response[resp_ptr++] = DAP_OK;
                    uint16_t transfer_cnt = 0;
                    uint16_t transfer_num = request[req_ptr++];
                    while (transfer_cnt < transfer_num) {
                        uint8_t info, bitlen, bytes;
                        #ifdef JTAG_ASYNC
                        uint8_t select = transfer_cnt & 0x1;
                        uint8_t *pbuf_tdo;

                        // prepare tms tdi
                        info = request[req_ptr++];
                        bitlen = info & JTAG_SEQUENCE_TCK;
                        if (bitlen == 0)
                            bitlen = 64;
                        else if (bitlen == 1) {
                            uint32_t tdo;
                            tdo = vsfhal_jtag_raw_1bit(info & JTAG_SEQUENCE_TMS, request[req_ptr++] & 0x1);
                            if (info & JTAG_SEQUENCE_TDO)
                                response[resp_ptr++] = tdo;
                            transfer_cnt++;
                            continue;
                        } else if (bitlen < 8) {
                            uint32_t tdo;
                            tdo = vsfhal_jtag_raw_less_8bit(bitlen, (info & JTAG_SEQUENCE_TMS) ? 0xff : 0, request[req_ptr++]);
                            if (info & JTAG_SEQUENCE_TDO)
                                response[resp_ptr++] = tdo;
                            transfer_cnt++;
                            continue;
                        }
                        bytes = (bitlen + 7) >> 3;
                        buf_tms[select] = (info & JTAG_SEQUENCE_TMS) ? 0xffffffffffffffff : 0;
                        if (info & JTAG_SEQUENCE_TDO) {
                            pbuf_tdo = response + resp_ptr;
                            resp_ptr += bytes;
                        } else {
                            pbuf_tdo = (uint8_t*)&buf_tdo[select];
                        }
                        // transfer
                        vsfhal_jtag_raw(0, bitlen, (uint8_t*)&buf_tms[select], request + req_ptr, pbuf_tdo);
                        req_ptr += bytes;
                        #else
                        // prepare tms tdi
                        info = request[req_ptr++];
                        bitlen = info & JTAG_SEQUENCE_TCK;
                        if (bitlen == 0)
                            bitlen = 64;
                        bytes = (bitlen + 7) >> 3;
                        buf_tms = (info & JTAG_SEQUENCE_TMS) ? 0xffffffffffffffff : 0;
                        memcpy(&buf_tdi, request + req_ptr, bytes);
                        req_ptr += bytes;
                        // transfer
                        vsfhal_jtag_raw(bitlen, (uint8_t*)&buf_tms, (uint8_t*)&buf_tdi, (uint8_t*)&buf_tdo);
                        // process tdo
                        if (info & JTAG_SEQUENCE_TDO) {
                            memcpy(response + resp_ptr, &buf_tdo, bytes);
                            resp_ptr += bytes;
                        }
                        #endif
                        transfer_cnt++;
                    }
                    #ifdef JTAG_ASYNC
                    vsfhal_jtag_wait();   // finish previous
                    #endif
                } else {
                    response[resp_ptr++] = DAP_ERROR;
                }
            } break;
            case ID_DAP_JTAG_Configure: {
                uint32_t n, count, length, bits = 0;

                count = request[req_ptr++];
                param->jtag_dev.count = min(count, DAP_JTAG_DEV_CNT);
                for (n = 0; n < count; n++) {
                    length = request[req_ptr++];
                    if (n < count) {
                        param->jtag_dev.ir_length[n] = length;
                        param->jtag_dev.ir_before[n] = bits;
                    }
                    bits += length;
                }
                for (n = 0; n < count; n++) {
                    bits -= param->jtag_dev.ir_length[n];
                    param->jtag_dev.ir_after[n] = bits;
                }
                response[resp_ptr++] = DAP_OK;
            } break;
            case ID_DAP_JTAG_IDCODE: {
                param->jtag_dev.index = request[req_ptr++];

                if ((param->port == DAP_PORT_JTAG) && (param->jtag_dev.index < DAP_JTAG_DEV_CNT)) {
                    uint16_t bitlen;

                    #ifdef JTAG_ASYNC
                    vsfhal_jtag_clear();
                    
                    // Select JTAG chain
                    bitlen = jtag_ir_to_raw(&buf_tms[0], &buf_tdi[0], JTAG_IDCODE,
                            param->jtag_dev.ir_length[param->jtag_dev.index],
                            param->jtag_dev.ir_before[param->jtag_dev.index],
                            param->jtag_dev.ir_after[param->jtag_dev.index]);
                    vsfhal_jtag_raw(0, bitlen, (uint8_t*)&buf_tms[0], (uint8_t*)&buf_tdi[0], (uint8_t*)&buf_tdo[0]);

                    // Read IDCODE register
                    buf_tms[1] = 0x1 << 0;
                    buf_tdi[1] = 0;
                    buf_tdo[1] = 0;
                    bitlen = 3 + param->jtag_dev.index + 32;
                    buf_tms[1] |= 0x3 << (bitlen - 1);
                    bitlen += 2;

                    vsfhal_jtag_wait();   // finish previous
                    response[resp_ptr++] = DAP_OK;
                    put_unaligned_le32((uint32_t)(buf_tdo[1] >> (3 + param->jtag_dev.index)), response + resp_ptr);
                    #else
                    // Select JTAG chain
                    vsfhal_jtag_ir(JTAG_IDCODE,
                            param->jtag_dev.ir_length[param->jtag_dev.index],
                            param->jtag_dev.ir_before[param->jtag_dev.index],
                            param->jtag_dev.ir_after[param->jtag_dev.index]);

                    // Read IDCODE register
                    buf_tdi = 0;
                    buf_tdo = 0;
                    buf_tms = 0x1;
                    bitlen = 3 + param->jtag_dev.index + 32;
                    buf_tms |= 0x3 << (bitlen - 1);
                    bitlen += 2;
                    vsfhal_jtag_raw(bitlen, (uint8_t*)&buf_tms, (uint8_t*)&buf_tdi, (uint8_t*)&buf_tdo);
                    response[resp_ptr++] = DAP_OK;
                    put_unaligned_le32((uint32_t)(buf_tdo >> (3 + param->jtag_dev.index)), response + resp_ptr);
                    #endif
                    resp_ptr += 4;
                } else {
                    response[resp_ptr++] = DAP_ERROR;
                }
            } break;
            case ID_DAP_TransferConfigure: {
                param->transfer.idle_cycles = request[req_ptr];
                param->transfer.retry_count = max(get_unaligned_le16(request + req_ptr + 1), 255);
                param->transfer.match_retry = get_unaligned_le16(request + req_ptr + 3);
                req_ptr += 5;

                if (param->port == DAP_PORT_DISABLED) {
                    if (port_init(param, DAP_PORT_JTAG) == VSF_ERR_NONE)
                        param->port = DAP_PORT_JTAG;
                }

                if (param->port == DAP_PORT_SWD)
                    vsfhal_swd_config(param->speed_khz,
                            param->transfer.retry_count, param->transfer.idle_cycles,
                            param->swd_conf.turnaround, param->swd_conf.data_phase);
                else if (param->port == DAP_PORT_JTAG)
                    vsfhal_jtag_config(param->speed_khz, param->transfer.retry_count,
                            param->transfer.idle_cycles);
                response[resp_ptr++] = DAP_OK;
            } break;
            case ID_DAP_Transfer: {
                bool post_read = false;
                #if defined(SWD_ASYNC) || defined(JTAG_ASYNC)
                uint8_t select = 0;
                #endif
                uint8_t transfer_req = 0;
                uint16_t transfer_cnt = 0, transfer_num, transfer_retry, transfer_ack = 0, req_start = req_ptr, resp_start = resp_ptr;
                uint32_t data;

                param->do_abort = false;
                if (param->port == DAP_PORT_SWD)
                #ifdef SWD_ASYNC
                {
                    bool check_write = false;

                    vsfhal_swd_clear();

                    transfer_num = request[req_ptr + 1];
                    req_ptr += 2;
                    resp_ptr += 2;

                    while (transfer_cnt < transfer_num) {
                        transfer_cnt++;
                        transfer_req = request[req_ptr++];
                        if (transfer_req & DAP_TRANSFER_RnW) {  // read
                            if (post_read) {
                                if ((transfer_req & (DAP_TRANSFER_APnDP | DAP_TRANSFER_MATCH_VALUE)) == DAP_TRANSFER_APnDP) {
                                    // Read previous AP data and post next AP read
                                    transfer_ack = vsfhal_swd_read(transfer_req, response + resp_ptr);
                                } else {
                                    post_read = false;
                                    // Read previous AP data
                                    transfer_ack = vsfhal_swd_read(DP_RDBUFF | DAP_TRANSFER_RnW, response + resp_ptr);
                                }
                                if (transfer_ack != DAP_TRANSFER_OK)
                                    break;
                                resp_ptr += 4;

                                #if TIMESTAMP_CLOCK
                                if (post_read) {    // Store Timestamp of next AP read
                                    if (transfer_req & DAP_TRANSFER_TIMESTAMP) {
                                        put_unaligned_le32(vsfhal_swd_get_timestamp(), response + resp_ptr);
                                        resp_ptr += 4;
                                    }
                                }
                                #endif
                            }

                            if (transfer_req & DAP_TRANSFER_MATCH_VALUE) {
                                uint32_t match_value = get_unaligned_le32(request + req_ptr);
                                req_ptr += 4;

                                if (transfer_req & DAP_TRANSFER_APnDP) {    // Post AP read
                                    transfer_ack = vsfhal_swd_read(transfer_req, NULL);
                                    if (transfer_ack != DAP_TRANSFER_OK)
                                        break;
                                }

                                transfer_retry = param->transfer.match_retry;
                                do {
                                    // Read register until its value matches or retry counter expires
                                    transfer_ack = vsfhal_swd_read(transfer_req, (uint8_t *)&data);
                                    if (transfer_ack != DAP_TRANSFER_OK)
                                        break;
                                    transfer_ack = vsfhal_swd_wait();
                                    if (transfer_ack != DAP_TRANSFER_OK)
                                        break;
                                } while (((data & param->transfer.match_mask) != match_value) && transfer_retry-- && !param->do_abort);
                                if ((data & param->transfer.match_mask) != match_value)
                                    transfer_ack |= DAP_TRANSFER_MISMATCH;
                                if (transfer_ack != DAP_TRANSFER_OK)
                                    break;
                            } else {    // Normal read
                                if (transfer_req & DAP_TRANSFER_APnDP) {    // Read AP register
                                    if (!post_read) {   // Post AP read
                                        transfer_ack = vsfhal_swd_read(transfer_req, NULL);
                                        if (transfer_ack != DAP_TRANSFER_OK)
                                            break;
                                        post_read = true;
                                        #if TIMESTAMP_CLOCK
                                        if (transfer_req & DAP_TRANSFER_TIMESTAMP) {    // Store Timestamp
                                            put_unaligned_le32(vsfhal_swd_get_timestamp(), response + resp_ptr);
                                            resp_ptr += 4;
                                        }
                                        #endif
                                    }
                                } else {    // Read DP register
                                    transfer_ack = vsfhal_swd_read(transfer_req, response + resp_ptr);
                                    if (transfer_ack != DAP_TRANSFER_OK)
                                        break;
                                    #if TIMESTAMP_CLOCK
                                    if (transfer_req & DAP_TRANSFER_TIMESTAMP) {    // Store Timestamp
                                        put_unaligned_le32(vsfhal_swd_get_timestamp(), response + resp_ptr);
                                        resp_ptr += 4;
                                    }
                                    #endif
                                    resp_ptr += 4;
                                }
                            }
                            check_write = false;
                        } else {    // Write register
                            if (post_read) {
                                transfer_ack = vsfhal_swd_read(DP_RDBUFF | DAP_TRANSFER_RnW, response + resp_ptr);
                                if (transfer_ack != DAP_TRANSFER_OK)
                                    break;
                                resp_ptr += 4;
                                post_read = false;
                            }

                            if (transfer_req & DAP_TRANSFER_MATCH_MASK) {
                                // Write match mask
                                param->transfer.match_mask = get_unaligned_le32(request + req_ptr);
                                req_ptr += 4;
                                transfer_ack = DAP_TRANSFER_OK;
                            } else {
                                // Write DP/AP register
                                transfer_ack = vsfhal_swd_write(transfer_req, request + req_ptr);
                                req_ptr += 4;
                                if (transfer_ack != DAP_TRANSFER_OK)
                                    break;
                                #if TIMESTAMP_CLOCK
                                if (transfer_req & DAP_TRANSFER_TIMESTAMP) {    // Store Timestamp
                                    put_unaligned_le32(vsfhal_swd_get_timestamp(), response + resp_ptr);
                                    resp_ptr += 4;
                                }
                                #endif
                                check_write = true;
                            }
                        }
                        if (param->do_abort)
                            break;
                    }

                    while (transfer_cnt < transfer_num) {
                        transfer_req = request[req_ptr++];
                        if (transfer_req & DAP_TRANSFER_RnW) {
                            if (transfer_req & DAP_TRANSFER_MATCH_VALUE)    // Read with value match
                                req_ptr += 4;
                        }
                        else    // Write register
                            req_ptr += 4;
                        transfer_cnt++;
                    }

                    if (transfer_ack == DAP_TRANSFER_OK) {
                        if (post_read) {
                            transfer_ack = vsfhal_swd_read(DP_RDBUFF | DAP_TRANSFER_RnW, response + resp_ptr);
                            if (transfer_ack == DAP_TRANSFER_OK) {
                                resp_ptr += 4;
                            }
                        } else if (check_write) {
                            transfer_ack = vsfhal_swd_read(DP_RDBUFF | DAP_TRANSFER_RnW, NULL);
                        }
                    }
                    transfer_ack = vsfhal_swd_wait();
                }
                #else
                {
                    bool check_write = false;

                    transfer_num = request[req_ptr + 1];
                    req_ptr += 2;
                    resp_ptr += 2;

                    while (transfer_cnt < transfer_num) {
                        transfer_cnt++;
                        transfer_req = request[req_ptr++];
                        if (transfer_req & DAP_TRANSFER_RnW) {  // read
                            if (post_read) {
                                if ((transfer_req & (DAP_TRANSFER_APnDP | DAP_TRANSFER_MATCH_VALUE)) == DAP_TRANSFER_APnDP) {
                                    // Read previous AP data and post next AP read
                                    transfer_ack = vsfhal_swd_read(transfer_req, response + resp_ptr);
                                } else {
                                    post_read = false;
                                    // Read previous AP data
                                    transfer_ack = vsfhal_swd_read(DP_RDBUFF | DAP_TRANSFER_RnW, response + resp_ptr);
                                }
                                if (transfer_ack != DAP_TRANSFER_OK)
                                    break;
                                resp_ptr += 4;

                                #if TIMESTAMP_CLOCK
                                if (post_read) {    // Store Timestamp of next AP read
                                    if (transfer_req & DAP_TRANSFER_TIMESTAMP) {
                                        put_unaligned_le32(vsfhal_swd_get_timestamp(), response + resp_ptr);
                                        resp_ptr += 4;
                                    }
                                }
                                #endif
                            }

                            if (transfer_req & DAP_TRANSFER_MATCH_VALUE) {
                                uint32_t match_value = get_unaligned_le32(request + req_ptr);
                                req_ptr += 4;

                                if (transfer_req & DAP_TRANSFER_APnDP) {    // Post AP read
                                    transfer_ack = vsfhal_swd_read(transfer_req, NULL);
                                    if (transfer_ack != DAP_TRANSFER_OK)
                                        break;
                                }

                                transfer_retry = param->transfer.match_retry;
                                do {
                                    // Read register until its value matches or retry counter expires
                                    transfer_ack = vsfhal_swd_read(transfer_req, (uint8_t *)&data);
                                    if (transfer_ack != DAP_TRANSFER_OK)
                                        break;
                                } while (((data & param->transfer.match_mask) != match_value) && transfer_retry-- && !param->do_abort);
                                if ((data & param->transfer.match_mask) != match_value)
                                    transfer_ack |= DAP_TRANSFER_MISMATCH;
                                if (transfer_ack != DAP_TRANSFER_OK)
                                    break;
                            } else {    // Normal read
                                if (transfer_req & DAP_TRANSFER_APnDP) {    // Read AP register
                                    if (!post_read) {   // Post AP read
                                        transfer_ack = vsfhal_swd_read(transfer_req, NULL);
                                        if (transfer_ack != DAP_TRANSFER_OK)
                                            break;
                                        post_read = true;
                                        #if TIMESTAMP_CLOCK
                                        if (transfer_req & DAP_TRANSFER_TIMESTAMP) {    // Store Timestamp
                                            put_unaligned_le32(vsfhal_swd_get_timestamp(), response + resp_ptr);
                                            resp_ptr += 4;
                                        }
                                        #endif
                                    }
                                } else {    // Read DP register
                                    transfer_ack = vsfhal_swd_read(transfer_req, response + resp_ptr);
                                    if (transfer_ack != DAP_TRANSFER_OK)
                                        break;
                                    #if TIMESTAMP_CLOCK
                                    if (transfer_req & DAP_TRANSFER_TIMESTAMP) {    // Store Timestamp
                                        put_unaligned_le32(vsfhal_swd_get_timestamp(), response + resp_ptr);
                                        resp_ptr += 4;
                                    }
                                    #endif
                                    resp_ptr += 4;
                                }
                            }
                            check_write = false;
                        } else {    // Write register
                            if (post_read) {
                                transfer_ack = vsfhal_swd_read(DP_RDBUFF | DAP_TRANSFER_RnW, response + resp_ptr);
                                if (transfer_ack != DAP_TRANSFER_OK)
                                    break;
                                resp_ptr += 4;
                                post_read = false;
                            }

                            if (transfer_req & DAP_TRANSFER_MATCH_MASK) {
                                // Write match mask
                                param->transfer.match_mask = get_unaligned_le32(request + req_ptr);
                                req_ptr += 4;
                                transfer_ack = DAP_TRANSFER_OK;
                            } else {
                                // Write DP/AP register
                                transfer_ack = vsfhal_swd_write(transfer_req, request + req_ptr);
                                req_ptr += 4;
                                if (transfer_ack != DAP_TRANSFER_OK)
                                    break;
                                #if TIMESTAMP_CLOCK
                                if (transfer_req & DAP_TRANSFER_TIMESTAMP) {    // Store Timestamp
                                    put_unaligned_le32(vsfhal_swd_get_timestamp(), response + resp_ptr);
                                    resp_ptr += 4;
                                }
                                #endif
                                check_write = true;
                            }
                        }
                        if (param->do_abort)
                            break;
                    }

                    while (transfer_cnt < transfer_num) {
                        transfer_req = request[req_ptr++];
                        if (transfer_req & DAP_TRANSFER_RnW) {
                            if (transfer_req & DAP_TRANSFER_MATCH_VALUE)    // Read with value match
                                req_ptr += 4;
                        }
                        else    // Write register
                            req_ptr += 4;
                        transfer_cnt++;
                    }

                    if (transfer_ack == DAP_TRANSFER_OK) {
                        if (post_read) {
                            transfer_ack = vsfhal_swd_read(DP_RDBUFF | DAP_TRANSFER_RnW, response + resp_ptr);
                            if (transfer_ack == DAP_TRANSFER_OK) {
                                resp_ptr += 4;
                            }
                        } else if (check_write) {
                            transfer_ack = vsfhal_swd_read(DP_RDBUFF | DAP_TRANSFER_RnW, NULL);
                        }
                    }
                }
                #endif
                else if (param->port == DAP_PORT_JTAG)
                #ifdef JTAG_ASYNC
                {
                    uint8_t idle, jtag_ir = 0;
                    uint16_t bitlen, dr_before, dr_after;

                    resp_ptr += 2;
                    param->jtag_dev.index = request[req_ptr++];
                    if (param->jtag_dev.index >= param->jtag_dev.count)
                        goto DAP_Transfer_EXIT;

                    dr_before = param->jtag_dev.index;
                    dr_after = param->jtag_dev.count - param->jtag_dev.index - 1;
                    idle = param->transfer.idle_cycles;

                    vsfhal_jtag_clear();

                    transfer_num = request[req_ptr++];
                    while (transfer_cnt < transfer_num) {
                        uint8_t request_ir;

                        transfer_req = request[req_ptr++];
                        request_ir = (transfer_req & DAP_TRANSFER_APnDP) ? JTAG_APACC : JTAG_DPACC;

                        if (transfer_req & DAP_TRANSFER_RnW) {  // Read register
                            if (post_read) {    // Read was posted before
                                if ((jtag_ir == request_ir) && !(transfer_req & DAP_TRANSFER_MATCH_VALUE)) {
                                    // Read previous data and post next read
                                    bitlen = jtag_dr_to_raw(&buf_tms[select], &buf_tdi[select], transfer_req,
                                            0, dr_before, dr_after, idle);
                                    transfer_ack = vsfhal_jtag_raw(dr_before + 3, bitlen, (uint8_t*)&buf_tms[select], (uint8_t*)&buf_tdi[select], (uint8_t*)&buf_tdo[select]);
                                    if (transfer_ack != DAP_TRANSFER_OK)
                                        break;
                                    select = select ? 0 : 1;
                                } else {
                                    // Select JTAG chain
                                    if (jtag_ir != JTAG_DPACC) {
                                        jtag_ir = JTAG_DPACC;
                                        bitlen = jtag_ir_to_raw(&buf_tms[select], &buf_tdi[select], jtag_ir,
                                                param->jtag_dev.ir_length[dr_before],
                                                param->jtag_dev.ir_before[dr_before],
                                                param->jtag_dev.ir_after[dr_before]);
                                        transfer_ack = vsfhal_jtag_raw(0, bitlen, (uint8_t*)&buf_tms[select], (uint8_t*)&buf_tdi[select], (uint8_t*)&buf_tdo[select]);
                                        if (transfer_ack != DAP_TRANSFER_OK)
                                            break;
                                        select = select ? 0 : 1;
                                    }
                                    // Read previous data
                                    bitlen = jtag_dr_to_raw(&buf_tms[select], &buf_tdi[select], DP_RDBUFF | DAP_TRANSFER_RnW,
                                            0, dr_before, dr_after, idle);
                                    transfer_ack = vsfhal_jtag_raw(dr_before + 3, bitlen, (uint8_t*)&buf_tms[select], (uint8_t*)&buf_tdi[select], (uint8_t*)&buf_tdo[select]);
                                    if (transfer_ack != DAP_TRANSFER_OK)
                                        break;
                                    select = select ? 0 : 1;
                                    post_read = 0;
                                }
                                put_unaligned_le32(data, response + resp_ptr);
                                resp_ptr += 4;
                                #if TIMESTAMP_CLOCK
                                if (post_read) {    // Store Timestamp of next AP read
                                    if (transfer_req & DAP_TRANSFER_TIMESTAMP) {
                                        put_unaligned_le32(vsfhal_jtag_get_timestamp(), response + resp_ptr);
                                        resp_ptr += 4;
                                    }
                                }
                                #endif
                            }

                            if (transfer_req & DAP_TRANSFER_MATCH_VALUE) {
                                uint32_t match_value = get_unaligned_le32(request + req_ptr);
                                req_ptr += 4;

                                // Select JTAG chain
                                if (jtag_ir != request_ir) {
                                    jtag_ir = request_ir;
                                    bitlen = jtag_ir_to_raw(&buf_tms[select], &buf_tdi[select], jtag_ir,
                                            param->jtag_dev.ir_length[dr_before],
                                            param->jtag_dev.ir_before[dr_before],
                                            param->jtag_dev.ir_after[dr_before]);
                                    transfer_ack = vsfhal_jtag_raw(0, bitlen, (uint8_t*)&buf_tms[select], (uint8_t*)&buf_tdi[select], (uint8_t*)&buf_tdo[select]);
                                    if (transfer_ack != DAP_TRANSFER_OK)
                                        break;
                                    select = select ? 0 : 1;
                                }
                                
                                // Post DP/AP read, igore tdo
                                bitlen = jtag_dr_to_raw(&buf_tms[select], &buf_tdi[select], transfer_req,
                                        0, dr_before, dr_after, idle);
                                transfer_ack = vsfhal_jtag_raw(dr_before + 3, bitlen, (uint8_t*)&buf_tms[select], (uint8_t*)&buf_tdi[select], (uint8_t*)&buf_tdo[select]);
                                if (transfer_ack != DAP_TRANSFER_OK)
                                    break;
                                select = select ? 0 : 1;

                                transfer_retry = param->transfer.match_retry;
                                do
                                {
                                    bitlen = jtag_dr_to_raw(&buf_tms[select], &buf_tdi[select], transfer_req,
                                            0, dr_before, dr_after, idle);
                                    transfer_ack = vsfhal_jtag_raw(dr_before + 3, bitlen, (uint8_t*)&buf_tms[select], (uint8_t*)&buf_tdi[select], (uint8_t*)&buf_tdo[select]);
                                    if (transfer_ack != DAP_TRANSFER_OK)
                                        break;
                                    transfer_ack = vsfhal_jtag_wait();
                                    if (transfer_ack != DAP_TRANSFER_OK)
                                        break;
                                    data = buf_tdo[select] >> (dr_before + 6);
                                } while (((data & param->transfer.match_mask) != match_value) && transfer_retry-- && !param->do_abort);
                                if ((data & param->transfer.match_mask) != match_value)
                                    transfer_ack |= DAP_TRANSFER_MISMATCH;
                                if (transfer_ack != DAP_TRANSFER_OK)
                                    break;
                            } else if (!post_read) { // Normal read
                                // Select JTAG chain
                                if (jtag_ir != request_ir) {
                                    jtag_ir = request_ir;
                                    bitlen = jtag_ir_to_raw(&buf_tms[select], &buf_tdi[select], jtag_ir,
                                            param->jtag_dev.ir_length[dr_before],
                                            param->jtag_dev.ir_before[dr_before],
                                            param->jtag_dev.ir_after[dr_before]);
                                    transfer_ack = vsfhal_jtag_raw(0, bitlen, (uint8_t*)&buf_tms[select], (uint8_t*)&buf_tdi[select], (uint8_t*)&buf_tdo[select]);
                                    if (transfer_ack != DAP_TRANSFER_OK)
                                        break;
                                    select = select ? 0 : 1;
                                }

                                // Post DP/AP read, igore tdo
                                bitlen = jtag_dr_to_raw(&buf_tms[select], &buf_tdi[select], transfer_req,
                                        0, dr_before, dr_after, idle);
                                transfer_ack = vsfhal_jtag_raw(dr_before + 3, bitlen, (uint8_t*)&buf_tms[select], (uint8_t*)&buf_tdi[select], (uint8_t*)&buf_tdo[select]);
                                if (transfer_ack != DAP_TRANSFER_OK)
                                    break;
                                select = select ? 0 : 1;
                                post_read = 1;
                                #if TIMESTAMP_CLOCK
                                if (transfer_req & DAP_TRANSFER_TIMESTAMP) {
                                    put_unaligned_le32(vsfhal_jtag_get_timestamp(), response + resp_ptr);
                                    resp_ptr += 4;
                                }
                                #endif
                            }
                        } else {    // Write register
                            if (post_read) {
                                // Select JTAG chain
                                if (jtag_ir != JTAG_DPACC) {
                                    jtag_ir = JTAG_DPACC;
                                    bitlen = jtag_ir_to_raw(&buf_tms[select], &buf_tdi[select], jtag_ir,
                                            param->jtag_dev.ir_length[dr_before],
                                            param->jtag_dev.ir_before[dr_before],
                                            param->jtag_dev.ir_after[dr_before]);
                                    transfer_ack = vsfhal_jtag_raw(0, bitlen, (uint8_t*)&buf_tms[select], (uint8_t*)&buf_tdi[select], (uint8_t*)&buf_tdo[select]);
                                    if (transfer_ack != DAP_TRANSFER_OK)
                                        break;
                                    select = select ? 0 : 1;
                                }
                                // Read previous data
                                bitlen = jtag_dr_to_raw(&buf_tms[select], &buf_tdi[select], DP_RDBUFF | DAP_TRANSFER_RnW,
                                        0, dr_before, dr_after, idle);
                                transfer_ack = vsfhal_jtag_raw(dr_before + 3, bitlen, (uint8_t*)&buf_tms[select], (uint8_t*)&buf_tdi[select], (uint8_t*)&buf_tdo[select]);
                                if (transfer_ack != DAP_TRANSFER_OK)
                                    break;
                                transfer_ack = vsfhal_jtag_wait();
                                if (transfer_ack != DAP_TRANSFER_OK)
                                    break;
                                put_unaligned_le32(data, response + resp_ptr);
                                resp_ptr += 4;
                                post_read = 0;
                            }

                            if (transfer_req & DAP_TRANSFER_MATCH_MASK)	{   // Write match mask
                                param->transfer.match_mask = get_unaligned_le32(request + req_ptr);
                                req_ptr += 4;
                                transfer_ack = DAP_TRANSFER_OK;
                            } else {
                                // Select JTAG chain
                                if (jtag_ir != request_ir) {
                                    jtag_ir = request_ir;
                                    bitlen = jtag_ir_to_raw(&buf_tms[select], &buf_tdi[select], jtag_ir,
                                            param->jtag_dev.ir_length[dr_before],
                                            param->jtag_dev.ir_before[dr_before],
                                            param->jtag_dev.ir_after[dr_before]);
                                    transfer_ack = vsfhal_jtag_raw(0, bitlen, (uint8_t*)&buf_tms[select], (uint8_t*)&buf_tdi[select], (uint8_t*)&buf_tdo[select]);
                                    if (transfer_ack != DAP_TRANSFER_OK)
                                        break;
                                    select = select ? 0 : 1;
                                }
                                // Write DP/AP register
                                bitlen = jtag_dr_to_raw(&buf_tms[select], &buf_tdi[select], transfer_req,
                                        get_unaligned_le32(request + req_ptr), dr_before, dr_after, idle);
                                req_ptr += 4;
                                transfer_ack = vsfhal_jtag_raw(dr_before + 3, bitlen, (uint8_t*)&buf_tms[select], (uint8_t*)&buf_tdi[select], (uint8_t*)&buf_tdo[select]);
                                if (transfer_ack != DAP_TRANSFER_OK)
                                    break;
                                select = select ? 0 : 1;
                                #if TIMESTAMP_CLOCK
                                if (post_read) {    // Store Timestamp
                                    if (transfer_req & DAP_TRANSFER_TIMESTAMP) {
                                        put_unaligned_le32(vsfhal_jtag_get_timestamp(), response + resp_ptr);
                                        resp_ptr += 4;
                                    }
                                }
                                #endif
                            }
                        }
                        transfer_cnt++;
                        if (param->do_abort)
                            break;
                    }

                    while (transfer_cnt < transfer_num) {
                        transfer_req = request[req_ptr++];
                        if (transfer_req & DAP_TRANSFER_RnW) {
                            if (transfer_req & DAP_TRANSFER_MATCH_VALUE)    // Read with value match
                                req_ptr += 4;
                        }
                        else    // Write register
                            req_ptr += 4;
                        transfer_cnt++;
                    }

                    if (transfer_ack == DAP_TRANSFER_OK) {
                        if (jtag_ir != JTAG_DPACC) {
                            jtag_ir = JTAG_DPACC;
                            bitlen = jtag_ir_to_raw(&buf_tms[select], &buf_tdi[select], jtag_ir,
                                    param->jtag_dev.ir_length[dr_before],
                                    param->jtag_dev.ir_before[dr_before],
                                    param->jtag_dev.ir_after[dr_before]);
                            vsfhal_jtag_raw(0, bitlen, (uint8_t*)&buf_tms[select], (uint8_t*)&buf_tdi[select], (uint8_t*)&buf_tdo[select]);
                            select = select ? 0 : 1;
                        }

                        bitlen = jtag_dr_to_raw(&buf_tms[select], &buf_tdi[select], DP_RDBUFF | DAP_TRANSFER_RnW,
                                0, dr_before, dr_after, idle);
                        vsfhal_jtag_raw(dr_before + 3, bitlen, (uint8_t*)&buf_tms[select], (uint8_t*)&buf_tdi[select], (uint8_t*)&buf_tdo[select]);
                        transfer_ack = vsfhal_jtag_wait(); // Not check transfer_ack
                        if (post_read && (transfer_ack == DAP_TRANSFER_OK)) {
                            put_unaligned_le32(buf_tdo[select] >> (dr_before + 6), response + resp_ptr);
                            resp_ptr += 4;
                        }
                    }
                }
                #else
                {
                    uint8_t jtag_ir = 0;
                    uint16_t dr_before, dr_after;

                    resp_ptr += 2;
                    param->jtag_dev.index = request[req_ptr++];
                    if (param->jtag_dev.index >= param->jtag_dev.count)
                        goto DAP_Transfer_EXIT;

                    dr_before = param->jtag_dev.index;
                    dr_after = param->jtag_dev.count - param->jtag_dev.index - 1;

                    transfer_num = request[req_ptr++];
                    while (transfer_cnt < transfer_num) {
                        uint8_t request_ir;

                        transfer_req = request[req_ptr++];
                        request_ir = (transfer_req & DAP_TRANSFER_APnDP) ? JTAG_APACC : JTAG_DPACC;

                        if (transfer_req & DAP_TRANSFER_RnW) {  // Read register
                            if (post_read) {    // Read was posted before
                                if ((jtag_ir == request_ir) && !(transfer_req & DAP_TRANSFER_MATCH_VALUE)) {
                                    // Read previous data and post next read
                                    transfer_ack = vsfhal_jtag_dr(transfer_req, 0, dr_before, dr_after, response + resp_ptr);
                                    if (transfer_ack != DAP_TRANSFER_OK)
                                        break;
                                } else {
                                    // Select JTAG chain
                                    if (jtag_ir != JTAG_DPACC) {
                                        jtag_ir = JTAG_DPACC;
                                        vsfhal_jtag_ir(jtag_ir,
                                                param->jtag_dev.ir_length[param->jtag_dev.index],
                                                param->jtag_dev.ir_before[param->jtag_dev.index],
                                                param->jtag_dev.ir_after[param->jtag_dev.index]);
                                    }
                                    // Read previous data
                                    transfer_ack = vsfhal_jtag_dr(DP_RDBUFF | DAP_TRANSFER_RnW, 0, dr_before, dr_after, response + resp_ptr);
                                    if (transfer_ack != DAP_TRANSFER_OK)
                                        break;
                                    post_read = 0;
                                }
                                resp_ptr += 4;

                                #if TIMESTAMP_CLOCK
                                if (post_read) {    // Store Timestamp of next AP read
                                    if (transfer_req & DAP_TRANSFER_TIMESTAMP) {
                                        put_unaligned_le32(vsfhal_jtag_get_timestamp(), response + resp_ptr);
                                        resp_ptr += 4;
                                    }
                                }
                                #endif
                            }

                            if (transfer_req & DAP_TRANSFER_MATCH_VALUE) {
                                uint32_t match_value = get_unaligned_le32(request + req_ptr);
                                req_ptr += 4;

                                // Select JTAG chain
                                if (jtag_ir != request_ir) {
                                    jtag_ir = request_ir;
                                    vsfhal_jtag_ir(jtag_ir,
                                            param->jtag_dev.ir_length[param->jtag_dev.index],
                                            param->jtag_dev.ir_before[param->jtag_dev.index],
                                            param->jtag_dev.ir_after[param->jtag_dev.index]);
                                }
                                
                                // Post DP/AP read, igore tdo
                                transfer_ack = vsfhal_jtag_dr(transfer_req, 0, dr_before, dr_after, (uint8_t *)&data);
                                if (transfer_ack != DAP_TRANSFER_OK)
                                    break;

                                transfer_retry = param->transfer.match_retry;
                                do
                                {
                                    transfer_ack = vsfhal_jtag_dr(transfer_req, 0, dr_before, dr_after, (uint8_t *)&data);
                                    if (transfer_ack != DAP_TRANSFER_OK)
                                        break;
                                } while (((data & param->transfer.match_mask) != match_value) && transfer_retry-- && !param->do_abort);
                                if ((data & param->transfer.match_mask) != match_value)
                                    transfer_ack |= DAP_TRANSFER_MISMATCH;
                                if (transfer_ack != DAP_TRANSFER_OK)
                                    break;
                            } else if (!post_read) { // Normal read
                                // Select JTAG chain
                                if (jtag_ir != request_ir) {
                                    jtag_ir = request_ir;
                                    vsfhal_jtag_ir(jtag_ir,
                                            param->jtag_dev.ir_length[param->jtag_dev.index],
                                            param->jtag_dev.ir_before[param->jtag_dev.index],
                                            param->jtag_dev.ir_after[param->jtag_dev.index]);
                                }

                                // Post DP/AP read, igore tdo
                                transfer_ack = vsfhal_jtag_dr(transfer_req, 0, dr_before, dr_after, (uint8_t *)&data);
                                if (transfer_ack != DAP_TRANSFER_OK)
                                    break;
                                post_read = 1;
                                #if TIMESTAMP_CLOCK
                                if (transfer_req & DAP_TRANSFER_TIMESTAMP) {
                                    put_unaligned_le32(vsfhal_jtag_get_timestamp(), response + resp_ptr);
                                    resp_ptr += 4;
                                }
                                #endif
                            }
                        } else {    // Write register
                            if (post_read) {
                                // Select JTAG chain
                                if (jtag_ir != JTAG_DPACC) {
                                    jtag_ir = JTAG_DPACC;
                                    vsfhal_jtag_ir(jtag_ir,
                                            param->jtag_dev.ir_length[param->jtag_dev.index],
                                            param->jtag_dev.ir_before[param->jtag_dev.index],
                                            param->jtag_dev.ir_after[param->jtag_dev.index]);
                                }
                                // Read previous data
                                transfer_ack = vsfhal_jtag_dr(DP_RDBUFF | DAP_TRANSFER_RnW, 0, dr_before, dr_after, response + resp_ptr);
                                if (transfer_ack != DAP_TRANSFER_OK)
                                    break;
                                resp_ptr += 4;
                                post_read = 0;
                            }

                            if (transfer_req & DAP_TRANSFER_MATCH_MASK)	{   // Write match mask
                                param->transfer.match_mask = get_unaligned_le32(request + req_ptr);
                                req_ptr += 4;
                                transfer_ack = DAP_TRANSFER_OK;
                            } else {
                                // Select JTAG chain
                                if (jtag_ir != request_ir) {
                                    jtag_ir = request_ir;
                                    vsfhal_jtag_ir(jtag_ir,
                                            param->jtag_dev.ir_length[param->jtag_dev.index],
                                            param->jtag_dev.ir_before[param->jtag_dev.index],
                                            param->jtag_dev.ir_after[param->jtag_dev.index]);
                                }
                                // Write DP/AP register
                                transfer_ack = vsfhal_jtag_dr(transfer_req, get_unaligned_le32(request + req_ptr), dr_before, dr_after, (uint8_t *)&data);
                                req_ptr += 4;
                                if (transfer_ack != DAP_TRANSFER_OK)
                                    break;
                                #if TIMESTAMP_CLOCK
                                if (transfer_req & DAP_TRANSFER_TIMESTAMP) {
                                    put_unaligned_le32(vsfhal_jtag_get_timestamp(), response + resp_ptr);
                                    resp_ptr += 4;
                                }
                                #endif
                            }
                        }
                        transfer_cnt++;
                        if (param->do_abort)
                            break;
                    }

                    while (transfer_cnt < transfer_num) {
                        transfer_req = request[req_ptr++];
                        if (transfer_req & DAP_TRANSFER_RnW) {
                            if (transfer_req & DAP_TRANSFER_MATCH_VALUE)    // Read with value match
                                req_ptr += 4;
                        }
                        else    // Write register
                            req_ptr += 4;
                        transfer_cnt++;
                    }

                    if (transfer_ack == DAP_TRANSFER_OK) {
                        if (jtag_ir != JTAG_DPACC) {
                            jtag_ir = JTAG_DPACC;
                            vsfhal_jtag_ir(jtag_ir,
                                    param->jtag_dev.ir_length[param->jtag_dev.index],
                                    param->jtag_dev.ir_before[param->jtag_dev.index],
                                    param->jtag_dev.ir_after[param->jtag_dev.index]);
                        }

                        transfer_ack = vsfhal_jtag_dr(DP_RDBUFF | DAP_TRANSFER_RnW, 0, dr_before, dr_after, (uint8_t *)&data);
                        if (post_read && (transfer_ack == DAP_TRANSFER_OK)) {
                            put_unaligned_le32(data, response + resp_ptr);
                            resp_ptr += 4;
                        }
                    }
                }
                #endif
                else
                {
                DAP_Transfer_EXIT:
                    req_ptr = req_start;
                    resp_ptr = resp_start;
                    transfer_num = request[req_ptr + 1];
                    req_ptr += 2;
                    while (transfer_num--) {
                        transfer_req = request[req_ptr++];
                        if (transfer_req & DAP_TRANSFER_RnW) {
                            if (transfer_req & DAP_TRANSFER_MATCH_VALUE)
                                req_ptr += 4;
                        } else {
                            req_ptr += 4;
                        }
                    }
                    response[resp_ptr++] = 0;
                    response[resp_ptr++] = 0;
                    break;
                }
                response[resp_start++] = transfer_cnt;
                response[resp_start++] = transfer_ack;
            } break;
            case ID_DAP_TransferBlock:{
                #if defined(SWD_ASYNC) || defined(JTAG_ASYNC)
                uint8_t select = 0;
                #endif
                uint8_t transfer_req = 0;
                uint16_t transfer_cnt = 0, transfer_num, transfer_ack = 0, resp_start = resp_ptr;

                if (param->port == DAP_PORT_SWD)
                #ifdef SWD_ASYNC
                {
                    vsfhal_swd_clear();

                    transfer_num = get_unaligned_le16(request + req_ptr + 1);
                    req_ptr += 3;
                    resp_ptr += 3;
                    if (!transfer_num)
                        goto DAP_TransferBlock_END;
                    
                    transfer_req = request[req_ptr++];
                    if (transfer_req & DAP_TRANSFER_RnW) {  // Read register block
                        if (transfer_req & DAP_TRANSFER_APnDP) {    // Post AP read
                            transfer_ack = vsfhal_swd_read(transfer_req, NULL);
                            if (transfer_ack != DAP_TRANSFER_OK)
                                goto DAP_TransferBlock_END;
                        }

                        while (transfer_cnt < transfer_num) {   // Read DP/AP register
                            if ((transfer_cnt == (transfer_num - 1)) && (transfer_req & DAP_TRANSFER_APnDP))
                                transfer_req = DP_RDBUFF | DAP_TRANSFER_RnW;
                            transfer_ack = vsfhal_swd_read(transfer_req, response + resp_ptr);
                            if (transfer_ack != DAP_TRANSFER_OK)
                                goto DAP_TransferBlock_END;
                            resp_ptr += 4;
                            transfer_cnt++;
                        }
                    } else {    // Write register block
                        while (transfer_cnt < transfer_num) {   // Write register block
                            transfer_ack = vsfhal_swd_write(transfer_req, request + req_ptr);
                            req_ptr += 4;
                            if (transfer_ack != DAP_TRANSFER_OK)
                                goto DAP_TransferBlock_END;
                            transfer_cnt++;
                        }
                        // Check last write
                        transfer_ack = vsfhal_swd_read(DP_RDBUFF | DAP_TRANSFER_RnW, NULL);
                    }
                    transfer_ack = vsfhal_swd_wait();
                }
                #else
                {
                    transfer_num = get_unaligned_le16(request + req_ptr + 1);
                    req_ptr += 3;
                    resp_ptr += 3;
                    if (!transfer_num)
                        goto DAP_TransferBlock_END;
                    
                    transfer_req = request[req_ptr++];
                    if (transfer_req & DAP_TRANSFER_RnW) {  // Read register block
                        if (transfer_req & DAP_TRANSFER_APnDP) {    // Post AP read
                            transfer_ack = vsfhal_swd_read(transfer_req, NULL);
                            if (transfer_ack != DAP_TRANSFER_OK)
                                goto DAP_TransferBlock_END;
                        }

                        while (transfer_cnt < transfer_num) {   // Read DP/AP register
                            if ((transfer_cnt == (transfer_num - 1)) && (transfer_req & DAP_TRANSFER_APnDP))
                                transfer_req = DP_RDBUFF | DAP_TRANSFER_RnW;
                            transfer_ack = vsfhal_swd_read(transfer_req, response + resp_ptr);
                            if (transfer_ack != DAP_TRANSFER_OK)
                                goto DAP_TransferBlock_END;
                            resp_ptr += 4;
                            transfer_cnt++;
                        }
                    } else {    // Write register block
                        while (transfer_cnt < transfer_num) {   // Write register block
                            transfer_ack = vsfhal_swd_write(transfer_req, request + req_ptr);
                            req_ptr += 4;
                            if (transfer_ack != DAP_TRANSFER_OK)
                                goto DAP_TransferBlock_END;
                            transfer_cnt++;
                        }
                        // Check last write
                        transfer_ack = vsfhal_swd_read(DP_RDBUFF | DAP_TRANSFER_RnW, NULL);
                    }
                }
                #endif
                else if (param->port == DAP_PORT_JTAG)
                #ifdef JTAG_ASYNC
                {
                    uint8_t idle, jtag_ir = 0;
                    uint16_t bitlen, dr_before, dr_after;

                    param->jtag_dev.index = request[req_ptr];
                    if (param->jtag_dev.index >= param->jtag_dev.count)
                        goto DAP_TransferBlock_ERROR;

                    dr_before = param->jtag_dev.index;
                    dr_after = param->jtag_dev.count - param->jtag_dev.index - 1;
                    idle = param->transfer.idle_cycles;
                    
                    vsfhal_jtag_clear();

                    transfer_num = get_unaligned_le16(request + req_ptr + 1);
                    req_ptr += 3;
                    resp_ptr += 3;
                    if (!transfer_num)
                        goto DAP_TransferBlock_END;

                    transfer_req = request[req_ptr++];

                    // Select JTAG chain
                    jtag_ir = (transfer_req & DAP_TRANSFER_APnDP) ? JTAG_APACC : JTAG_DPACC;
                    bitlen = jtag_ir_to_raw(&buf_tms[select], &buf_tdi[select], jtag_ir,
                            param->jtag_dev.ir_length[dr_before],
                            param->jtag_dev.ir_before[dr_before],
                            param->jtag_dev.ir_after[dr_before]);
                    vsfhal_jtag_raw(0, bitlen, (uint8_t*)&buf_tms[select], (uint8_t*)&buf_tdi[select], (uint8_t*)&buf_tdo[select]);
                    select = select ? 0 : 1;

                    if (transfer_req & DAP_TRANSFER_RnW) {  // Read
                        // Post read
                        bitlen = jtag_dr_to_raw(&buf_tms[select], &buf_tdi[select], transfer_req,
                                0, dr_before, dr_after, idle);
                        transfer_ack = vsfhal_jtag_raw(dr_before + 3, bitlen, (uint8_t*)&buf_tms[select], (uint8_t*)&buf_tdi[select], (uint8_t*)&buf_tdo[select]);
                        if (transfer_ack != DAP_TRANSFER_OK)
                            goto DAP_TransferBlock_END;
                        select = select ? 0 : 1;

                        // TODO: need async here
                        // Read register block
                        while (transfer_cnt < transfer_num) {
                            if (transfer_cnt == (transfer_num - 1)) {   // Last read
                                if (jtag_ir != JTAG_DPACC) {
                                    jtag_ir = JTAG_DPACC;
                                    bitlen = jtag_ir_to_raw(&buf_tms[select], &buf_tdi[select], jtag_ir,
                                            param->jtag_dev.ir_length[dr_before],
                                            param->jtag_dev.ir_before[dr_before],
                                            param->jtag_dev.ir_after[dr_before]);
                                    transfer_ack = vsfhal_jtag_raw(0, bitlen, (uint8_t*)&buf_tms[select], (uint8_t*)&buf_tdi[select], (uint8_t*)&buf_tdo[select]);
                                    if (transfer_ack != DAP_TRANSFER_OK)
                                        goto DAP_TransferBlock_END;
                                    select = select ? 0 : 1;
                                }
                                transfer_req = DP_RDBUFF | DAP_TRANSFER_RnW;
                            }
                            
                            bitlen = jtag_dr_to_raw(&buf_tms[select], &buf_tdi[select], transfer_req,
                                    0, dr_before, dr_after, idle);
                            transfer_ack = vsfhal_jtag_raw(dr_before + 3, bitlen, (uint8_t*)&buf_tms[select], (uint8_t*)&buf_tdi[select], (uint8_t*)&buf_tdo[select]);
                            if (transfer_ack != DAP_TRANSFER_OK)
                                goto DAP_TransferBlock_END;
                            transfer_ack = vsfhal_jtag_wait(); // Not check transfer_ack
                            if (transfer_ack == DAP_TRANSFER_OK) {
                                put_unaligned_le32(buf_tdo[select] >> (dr_before + 6), response + resp_ptr);
                                resp_ptr += 4;
                            } else {
                                goto DAP_TransferBlock_END;
                            }
                            select = select ? 0 : 1;
                            transfer_cnt++;
                        }
                    } else {    // Write register block
                        while (transfer_cnt < transfer_num) {
                            // Write DP/AP register
                            bitlen = jtag_dr_to_raw(&buf_tms[select], &buf_tdi[select], transfer_req,
                                    get_unaligned_le32(request + req_ptr), dr_before, dr_after, idle);
                            req_ptr += 4;
                            transfer_ack = vsfhal_jtag_raw(dr_before + 3, bitlen, (uint8_t*)&buf_tms[select], (uint8_t*)&buf_tdi[select], (uint8_t*)&buf_tdo[select]);
                            if (transfer_ack != DAP_TRANSFER_OK)
                                goto DAP_TransferBlock_END;
                            select = select ? 0 : 1;
                            transfer_cnt++;
                        }

                        // Check last write
                        if (jtag_ir != JTAG_DPACC) {
                            jtag_ir = JTAG_DPACC;
                            bitlen = jtag_ir_to_raw(&buf_tms[select], &buf_tdi[select], jtag_ir,
                                    param->jtag_dev.ir_length[dr_before],
                                    param->jtag_dev.ir_before[dr_before],
                                    param->jtag_dev.ir_after[dr_before]);
                            transfer_ack = vsfhal_jtag_raw(0, bitlen, (uint8_t*)&buf_tms[select], (uint8_t*)&buf_tdi[select], (uint8_t*)&buf_tdo[select]);
                            if (transfer_ack != DAP_TRANSFER_OK)
                                goto DAP_TransferBlock_END;
                            select = select ? 0 : 1;
                        }
                        bitlen = jtag_dr_to_raw(&buf_tms[select], &buf_tdi[select], DP_RDBUFF | DAP_TRANSFER_RnW,
                                    0, dr_before, dr_after, idle);
                        transfer_ack = vsfhal_jtag_raw(dr_before + 3, bitlen, (uint8_t*)&buf_tms[select], (uint8_t*)&buf_tdi[select], (uint8_t*)&buf_tdo[select]);
                        if (transfer_ack != DAP_TRANSFER_OK)
                            goto DAP_TransferBlock_END;
                        transfer_ack = vsfhal_jtag_wait();
                        if (transfer_ack != DAP_TRANSFER_OK)
                            goto DAP_TransferBlock_END;
                    }
                }
                #else
                {
                    uint8_t jtag_ir = 0;
                    uint16_t dr_before, dr_after;

                    param->jtag_dev.index = request[req_ptr];
                    if (param->jtag_dev.index >= param->jtag_dev.count)
                        goto DAP_TransferBlock_ERROR;

                    dr_before = param->jtag_dev.index;
                    dr_after = param->jtag_dev.count - param->jtag_dev.index - 1;

                    transfer_num = get_unaligned_le16(request + req_ptr + 1);
                    req_ptr += 3;
                    resp_ptr += 3;
                    if (!transfer_num)
                        goto DAP_TransferBlock_END;

                    transfer_req = request[req_ptr++];

                    // Select JTAG chain
                    jtag_ir = (transfer_req & DAP_TRANSFER_APnDP) ? JTAG_APACC : JTAG_DPACC;
                    vsfhal_jtag_ir(jtag_ir,
                            param->jtag_dev.ir_length[param->jtag_dev.index],
                            param->jtag_dev.ir_before[param->jtag_dev.index],
                            param->jtag_dev.ir_after[param->jtag_dev.index]);

                    if (transfer_req & DAP_TRANSFER_RnW) {  // Read
                        // Post read
                        transfer_ack = vsfhal_jtag_dr(transfer_req, 0, dr_before, dr_after, NULL);
                        if (transfer_ack != DAP_TRANSFER_OK)
                            goto DAP_TransferBlock_END;

                        // Read register block
                        while (transfer_cnt < transfer_num) {
                            if (transfer_cnt == (transfer_num - 1)) {   // Last read
                                if (jtag_ir != JTAG_DPACC) {
                                    jtag_ir = JTAG_DPACC;
                                    vsfhal_jtag_ir(jtag_ir,
                                            param->jtag_dev.ir_length[param->jtag_dev.index],
                                            param->jtag_dev.ir_before[param->jtag_dev.index],
                                            param->jtag_dev.ir_after[param->jtag_dev.index]);
                                }
                                transfer_req = DP_RDBUFF | DAP_TRANSFER_RnW;
                            }
                            
                            transfer_ack = vsfhal_jtag_dr(transfer_req, 0, dr_before, dr_after, response + resp_ptr);
                            if (transfer_ack != DAP_TRANSFER_OK)
                                goto DAP_TransferBlock_END;
                            resp_ptr += 4;
                            transfer_cnt++;
                        }
                    } else {    // Write register block
                        while (transfer_cnt < transfer_num) {
                            // Write DP/AP register
                            transfer_ack = vsfhal_jtag_dr(transfer_req, get_unaligned_le32(request + req_ptr), dr_before, dr_after, NULL);
                            req_ptr += 4;
                            if (transfer_ack != DAP_TRANSFER_OK)
                                goto DAP_TransferBlock_END;
                            transfer_cnt++;
                        }

                        // Check last write
                        if (jtag_ir != JTAG_DPACC) {
                            jtag_ir = JTAG_DPACC;
                            vsfhal_jtag_ir(jtag_ir,
                                    param->jtag_dev.ir_length[param->jtag_dev.index],
                                    param->jtag_dev.ir_before[param->jtag_dev.index],
                                    param->jtag_dev.ir_after[param->jtag_dev.index]);
                        }

                        transfer_ack = vsfhal_jtag_dr(DP_RDBUFF | DAP_TRANSFER_RnW, get_unaligned_le32(request + req_ptr), dr_before, dr_after, NULL);
                        if (transfer_ack != DAP_TRANSFER_OK)
                            goto DAP_TransferBlock_END;
                    }
                }
                #endif
                else 
                {
                DAP_TransferBlock_ERROR:
                    if (request[req_ptr + 3] & DAP_TRANSFER_RnW) {
                        req_ptr += 4;
                    } else {
                        req_ptr += 4 + 4 * get_unaligned_le16(request + req_ptr + 1);
                    }
                    put_unaligned_le16(0, response + resp_start);
                    response[resp_start + 2] = 0;
                    break;
                }
            DAP_TransferBlock_END:
                put_unaligned_le16(transfer_cnt, response + resp_start);
				response[resp_start + 2] = transfer_ack;
            } break;
            case ID_DAP_WriteABORT: {
                uint16_t transfer_ack;

                if (param->port == DAP_PORT_SWD) {
                    #ifdef SWD_ASYNC
                    vsfhal_swd_clear();
                    #endif
                    vsfhal_swd_write(DP_ABORT, request + req_ptr + 1);
                    req_ptr += 5;
                    #ifdef SWD_ASYNC
                    vsfhal_swd_wait();
                    #endif
                    response[resp_ptr++] = DAP_OK;
                } else if (param->port == DAP_PORT_JTAG) {
                    #ifdef JTAG_ASYNC
                    uint16_t bitlen;
                    #endif
                    
                    if (request[req_ptr] >= DAP_JTAG_DEV_CNT)
                        goto DAP_WriteABORT_EXIT;
                    
                    param->jtag_dev.index = request[req_ptr++];

                    #ifdef JTAG_ASYNC
                    vsfhal_jtag_clear();

                    bitlen = jtag_ir_to_raw(&buf_tms[0], &buf_tdi[0], JTAG_ABORT,
                            param->jtag_dev.ir_length[param->jtag_dev.index],
                            param->jtag_dev.ir_before[param->jtag_dev.index],
                            param->jtag_dev.ir_after[param->jtag_dev.index]);
                    transfer_ack = vsfhal_jtag_raw(0, bitlen, (uint8_t*)&buf_tms[0], (uint8_t*)&buf_tdi[0], (uint8_t*)&buf_tdo[0]);

                    // Write Abort register
                    bitlen = jtag_dr_to_raw(&buf_tms[1], &buf_tdi[1], 0,
                        get_unaligned_le32(request + req_ptr), param->jtag_dev.index,
                        param->jtag_dev.count - param->jtag_dev.index - 1, param->transfer.idle_cycles);
                    req_ptr += 4;
                    transfer_ack = vsfhal_jtag_raw(param->jtag_dev.index + 3, bitlen, (uint8_t*)&buf_tms[1], (uint8_t*)&buf_tdi[1], (uint8_t*)&buf_tdo[1]);
                    transfer_ack = vsfhal_jtag_wait();   // finish previouss
                    #else
                    vsfhal_jtag_ir(JTAG_ABORT,
                            param->jtag_dev.ir_length[param->jtag_dev.index],
                            param->jtag_dev.ir_before[param->jtag_dev.index],
                            param->jtag_dev.ir_after[param->jtag_dev.index]);

                    // Write Abort register
                    transfer_ack = vsfhal_jtag_dr(
                        0, get_unaligned_le32(request + req_ptr), param->jtag_dev.index,
                        param->jtag_dev.count - param->jtag_dev.index - 1, NULL);
                    req_ptr += 4;
                    #endif
                    if (transfer_ack != DAP_TRANSFER_OK)
                        goto DAP_WriteABORT_ERROR;
                    response[resp_ptr++] = DAP_OK;
                } else {
                DAP_WriteABORT_EXIT:
                    req_ptr += 5;
                DAP_WriteABORT_ERROR:
                    response[resp_ptr++] = DAP_ERROR;
                }
            } break;
            #if SWO_UART || SWO_MANCHESTER
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
                uint16_t count = min(get_unaligned_le16(request + req_ptr),
                        VSF_STREAM_GET_DATA_SIZE(&param->swo_rx));
                req_ptr += 2;
                response[resp_ptr++] = param->trace_status;
                if (param->transport != 1)	// Read trace data via DAP_SWO_Data command
                    count = 0;
                if (count > (pkt_size - 2 - resp_ptr))
                    count = pkt_size - 2 - resp_ptr;
                if (count) {
                    count = VSF_STREAM_READ(&param->swo_rx, response + resp_ptr + 2, count);
                }
                put_unaligned_le32(count, response + resp_ptr);
                resp_ptr += 2 + count;
                param->trace_o += count;
            } break;
            #endif
            default:
                goto fault;
            }
        }
    } while (cmd_num && (resp_ptr < pkt_size));
    goto exit;

fault:
    response[resp_ptr - 1] = ID_DAP_Invalid;
exit:
    return resp_ptr;
}

implement_vsf_task(dap_task_t)
{
    dap_request_t* request = &this.request[this.request_head];

    vsf_task_begin();
    enum {
        WAIT_FOR_REQ_SEM = 0,
        WAIT_FOR_RESP_SEM,
    };

    on_vsf_task_init() { vsf_sem_init(&this.request_sem, 0); }

    switch (vsf_task_state) {
    case WAIT_FOR_REQ_SEM:
        vsf_task_wait_until(vsf_sem_pend(&this.request_sem));

        request = &this.request[this.request_head];

        this.response.response_sem = request->response_sem;
        this.response.response = request->response;
        this.response.response_param = request->response_param;
        this.response.response_size = request_handler(this.dap_param, request->request_buf,
            this.response.response_buf, request->pkt_size);

        vsf_gint_state_t orig = vsf_disable_interrupt();
        if (++this.request_head == DAP_PACKET_COUNT)
            this.request_head = 0;
        this.request_cnt--;
        vsf_set_interrupt(orig);
        
        vsf_task_state = WAIT_FOR_RESP_SEM;

    case WAIT_FOR_RESP_SEM:
        vsf_task_wait_until(vsf_sem_pend(this.response.response_sem));

        this.response.response(this.response.response_param,
            this.response.response_buf,
            this.response.response_size);
        
        vsf_task_state = WAIT_FOR_REQ_SEM;
        break;
    }
    vsf_task_end();
}

void dap_init(dap_t *dap, vsf_prio_t prio)
{
    dap->dap_task.dap_param = &dap->dap_param;
    init_vsf_task(dap_task_t, &dap->dap_task, prio);
}

vsf_err_t dap_requset(dap_t *dap, vsf_sem_t *response_sem,
        void (*response)(void* p, uint8_t* buf, uint16_t size),
        void* response_param, uint8_t* buf, uint16_t pkt_size)
{
    dap_param_t *param = &dap->dap_param;
    dap_task_t *task = &dap->dap_task;

    if (buf[0] == ID_DAP_TransferAbort) {
        param->do_abort = true;
        return VSF_ERR_NONE;
    } else if (task->request_cnt < DAP_PACKET_COUNT) {
        dap_request_t *request = &task->request[(task->request_head + task->request_cnt) % DAP_PACKET_COUNT];
        request->response_sem = response_sem;
        request->response = response;
        request->response_param = response_param;
        request->pkt_size = min(pkt_size, DAP_PACKET_SIZE);
        memcpy(request->request_buf, buf, request->pkt_size);
        task->request_cnt++;
        vsf_sem_post(&dap->dap_task.request_sem);
        return VSF_ERR_NONE;
    }
    return VSF_ERR_NOT_READY;
}

void dap_test(dap_t *dap, uint8_t port, uint16_t speed_khz)
{
    uint16_t ret;
    dap_param_t *param = &dap->dap_param;
    dap_task_t *dap_task = &dap->dap_task;
    dap_request_t *request = &dap_task->request[0];
    dap_response_t *response = &dap_task->response;

    param->port = port;
    param->speed_khz = speed_khz;
    param->transfer.idle_cycles = 0;
    param->transfer.retry_count = 100;
    param->transfer.match_retry = 0;
    param->transfer.match_mask = 0;
#if DAP_SWD
    param->swd_conf.turnaround = 1;
    param->swd_conf.data_phase = 0;
#endif
#if DAP_JTAG
    param->jtag_dev.count = 2;
    param->jtag_dev.index = 0;
    param->jtag_dev.ir_length[0] = 4;
    param->jtag_dev.ir_before[0] = 0;
    param->jtag_dev.ir_after[0] = 5;
    param->jtag_dev.ir_length[1] = 5;
    param->jtag_dev.ir_before[1] = 4;
    param->jtag_dev.ir_after[1] = 0;
#endif

    port_init(param, param->port);
    
    if (param->port == DAP_PORT_SWD) {
        // ID_DAP_SWJ_Sequence
        const uint8_t sreq_ID_DAP_SWJ_Sequence[] = {
            0x12, 0x88, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
            0xff, 0x9e, 0xe7, 0xff, 0xff, 0xff, 0xff, 0xff, 
            0xff, 0xff, 0x00
        };
#if 0   // OpenOCD
        // ID_DAP_Transfer
        const uint8_t sreq_ID_DAP_Transfer1[] = {
            0x05, 0x00, 0x02, 0x02, 0x00, 0x1e, 0x00, 0x00,
            0x00
        };
        const uint8_t sreq_ID_DAP_Transfer2[] = {
            0x05, 0x00, 0x02, 0x08, 0x00, 0x00, 0x00, 0x00,
            0x06
        };
        const uint8_t sreq_ID_DAP_Transfer3[] = {
            0x05, 0x00, 0x04, 0x04, 0x20, 0x00, 0x00, 0x50,
            0x06, 0x04, 0x00, 0x00, 0x00, 0x50, 0x06
        };
        const uint8_t sreq_ID_DAP_Transfer4[] = {
            0x05, 0x00, 0x01, 0x06
        };
        const uint8_t sreq_ID_DAP_Transfer5[] = {
            0x05, 0x00, 0x03, 0x06, 0x04, 0x00, 0x00, 0x00,
            0x50, 0x06
        };
        const uint8_t sreq_ID_DAP_Transfer6[] = {
            0x05, 0x00, 0x03, 0x08, 0xf0, 0x00, 0x00, 0x00,
            0x0f, 0x0e
        };
        const uint8_t sreq_ID_DAP_Transfer7[] = {
            0x05, 0x00, 0x08, 0x08, 0x00, 0x00, 0x00, 0x00,
            0x01, 0x20, 0x00, 0x00, 0xa2, 0x05, 0x00, 0x00,
            0x00, 0x00, 0x03, 0x0e, 0x08, 0xf0, 0x00, 0x00,
            0x00, 0x07, 0x0e
        };
        const uint8_t sreq_ID_DAP_Transfer8[] = {
            0x05, 0x00, 0x05, 0x08, 0x00, 0x00, 0x00, 0x00,
            0x01, 0x22, 0x00, 0x00, 0xa2, 0x05, 0x00, 0x00,
            0xe0, 0x0f, 0x0e
        };
        const uint8_t sreq_ID_DAP_Transfer9[] = {
            0x05, 0x00, 0x03, 0x05, 0x40, 0xef, 0x00, 0xe0,
            0x0f, 0x0e
        };
        const uint8_t sreq_ID_DAP_Transfer10[] = {
            0x05, 0x00, 0x02, 0x0f, 0x0e
        };
#else   // IAR
        // ID_DAP_Transfer
        const uint8_t sreq_ID_DAP_Transfer1[] = {
            0x05, 0x00, 0x01, 0x02,
        };
        const uint8_t sreq_ID_DAP_Transfer2[] = {
            0x05, 0x00, 0x01, 0x02,
        };
        const uint8_t sreq_ID_DAP_Transfer3[] = {
            0x05, 0x00, 0x01, 0x00, 0x1e, 0x00, 0x00, 0x00,
        };
        const uint8_t sreq_ID_DAP_Transfer4[] = {
            0x05, 0x00, 0x01, 0x06
        };
        const uint8_t sreq_ID_DAP_Transfer5[] = {
            0x05, 0x00, 0x03, 0x06, 0x04, 0x00, 0x00, 0x00,
            0x50, 0x06
        };
        const uint8_t sreq_ID_DAP_Transfer6[] = {
            0x05, 0x00, 0x03, 0x08, 0xf0, 0x00, 0x00, 0x00,
            0x0f, 0x0e
        };
        const uint8_t sreq_ID_DAP_Transfer7[] = {
            0x05, 0x00, 0x08, 0x08, 0x00, 0x00, 0x00, 0x00,
            0x01, 0x20, 0x00, 0x00, 0xa2, 0x05, 0x00, 0x00,
            0x00, 0x00, 0x03, 0x0e, 0x08, 0xf0, 0x00, 0x00,
            0x00, 0x07, 0x0e
        };
        const uint8_t sreq_ID_DAP_Transfer8[] = {
            0x05, 0x00, 0x05, 0x08, 0x00, 0x00, 0x00, 0x00,
            0x01, 0x22, 0x00, 0x00, 0xa2, 0x05, 0x00, 0x00,
            0xe0, 0x0f, 0x0e
        };
        const uint8_t sreq_ID_DAP_Transfer9[] = {
            0x05, 0x00, 0x03, 0x05, 0x40, 0xef, 0x00, 0xe0,
            0x0f, 0x0e
        };
        const uint8_t sreq_ID_DAP_Transfer10[] = {
            0x05, 0x00, 0x02, 0x0f, 0x0e
        };
#endif

        memcpy(request->request_buf, sreq_ID_DAP_SWJ_Sequence, sizeof(sreq_ID_DAP_SWJ_Sequence));
        ret = request_handler(param, request->request_buf, response->response_buf, DAP_PACKET_SIZE);
        
        memcpy(request->request_buf, sreq_ID_DAP_Transfer1, sizeof(sreq_ID_DAP_Transfer1));
        ret = request_handler(param, request->request_buf, response->response_buf, DAP_PACKET_SIZE);
        
        memcpy(request->request_buf, sreq_ID_DAP_Transfer2, sizeof(sreq_ID_DAP_Transfer2));
        ret = request_handler(param, request->request_buf, response->response_buf, DAP_PACKET_SIZE);
        
        memcpy(request->request_buf, sreq_ID_DAP_Transfer3, sizeof(sreq_ID_DAP_Transfer3));
        ret = request_handler(param, request->request_buf, response->response_buf, DAP_PACKET_SIZE);
        
        memcpy(request->request_buf, sreq_ID_DAP_Transfer4, sizeof(sreq_ID_DAP_Transfer4));
        ret = request_handler(param, request->request_buf, response->response_buf, DAP_PACKET_SIZE);
        
        memcpy(request->request_buf, sreq_ID_DAP_Transfer5, sizeof(sreq_ID_DAP_Transfer5));
        ret = request_handler(param, request->request_buf, response->response_buf, DAP_PACKET_SIZE);
        
        memcpy(request->request_buf, sreq_ID_DAP_Transfer6, sizeof(sreq_ID_DAP_Transfer6));
        ret = request_handler(param, request->request_buf, response->response_buf, DAP_PACKET_SIZE);
        
        memcpy(request->request_buf, sreq_ID_DAP_Transfer7, sizeof(sreq_ID_DAP_Transfer7));
        ret = request_handler(param, request->request_buf, response->response_buf, DAP_PACKET_SIZE);
        
        memcpy(request->request_buf, sreq_ID_DAP_Transfer8, sizeof(sreq_ID_DAP_Transfer8));
        ret = request_handler(param, request->request_buf, response->response_buf, DAP_PACKET_SIZE);
        
        memcpy(request->request_buf, sreq_ID_DAP_Transfer9, sizeof(sreq_ID_DAP_Transfer9));
        ret = request_handler(param, request->request_buf, response->response_buf, DAP_PACKET_SIZE);
        
        memcpy(request->request_buf, sreq_ID_DAP_Transfer10, sizeof(sreq_ID_DAP_Transfer10));
        ret = request_handler(param, request->request_buf, response->response_buf, DAP_PACKET_SIZE);
    } else if (param->port == DAP_PORT_JTAG) {
#if 0   // OpenOCD
        // ID_DAP_SWJ_Sequence
        const uint8_t jreq_ID_DAP_SWJ_Sequence[] = {0x12, 0x08, 0xff};
        // ID_DAP_JTAG_Sequence
        const uint8_t jreq_ID_DAP_JTAG_Sequence1[] = {
            0x14, 0x14, 
            0x41, 0x00, 
            0x41, 0x00, 
            0x41, 0x00, 
            0x01, 0x00, 
            0x41, 0x00, 
            0x01, 0x00, 
            0x01, 0x00, 
            0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
            0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
            0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
            0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
            0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
            0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
            0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
            0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
            0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
            0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0x9f, 0xff, 0xff, 0xff, 0x7f, 
            0xc1, 0x01, 
            0x01, 0x01
        };
        const uint8_t jreq_ID_DAP_JTAG_Sequence2[] = {
            0x14, 0x0d, 0x41, 0x00, 0x41, 0x00, 0x01, 0x00, 0x41, 0x00, 0x41, 0x00, 0x01, 0x00, 0x01, 0x00, 
            0x8a, 0xff, 0xff, 0xc1, 0xff, 0x01, 0xff, 0x41, 0x00, 0x41, 0x00, 0x01, 0x00
        };
        
        memcpy(request->request_buf, jreq_ID_DAP_SWJ_Sequence, sizeof(jreq_ID_DAP_SWJ_Sequence));
        ret = request_handler(param, request->request_buf, response->response_buf, DAP_PACKET_SIZE);
        
        memcpy(request->request_buf, jreq_ID_DAP_JTAG_Sequence1, sizeof(jreq_ID_DAP_JTAG_Sequence1));
        ret = request_handler(param, request->request_buf, response->response_buf, DAP_PACKET_SIZE);
        
        memcpy(request->request_buf, jreq_ID_DAP_SWJ_Sequence, sizeof(jreq_ID_DAP_SWJ_Sequence));
        ret = request_handler(param, request->request_buf, response->response_buf, DAP_PACKET_SIZE);
        
        memcpy(request->request_buf, jreq_ID_DAP_JTAG_Sequence2, sizeof(jreq_ID_DAP_JTAG_Sequence2));
        ret = request_handler(param, request->request_buf, response->response_buf, DAP_PACKET_SIZE);
#elif 1 // IAR
        // ID_DAP_TransferConfigure
        const uint8_t jreq_ID_DAP_TransferConfigure[] = {0x04, 0x00, 0x64, 0x00, 0x00, 0x00};
        // ID_DAP_JTAG_Configure
        const uint8_t jreq_ID_DAP_JTAG_Configure[] = {0x15, 0x02, 0x04, 0x05};
        // ID_DAP_SWJ_Sequence
        const uint8_t jreq_ID_DAP_SWJ_Sequence[] = {0x12, 0x10, 0xe7, 0x3c};
        // ID_DAP_Transfer
        const uint8_t jreq_ID_DAP_Transfer1[] = {
        0x05, 0x00, 0x01, 0x08, 0x00, 0x00, 0x00, 0x00,
        };
        const uint8_t jreq_ID_DAP_Transfer2[] = {
        0x05, 0x00, 0x01, 0x08, 0x00, 0x00, 0x00, 0x00,
        };
        const uint8_t jreq_ID_DAP_Transfer3[] = {
        0x05, 0x00, 0x01, 0x06,
        };
        const uint8_t jreq_ID_DAP_Transfer4[] = {
        0x05, 0x00, 0x01, 0x04, 0x32, 0x00, 0x00, 0x50
        };
        const uint8_t jreq_ID_DAP_Transfer5[] = {
        0x05, 0x00, 0x01, 0x06,
        };
        const uint8_t jreq_ID_DAP_Transfer6[] = {
        0x05, 0x00, 0x01, 0x08, 0xf0, 0x00, 0x00, 0x00
        };
        const uint8_t jreq_ID_DAP_Transfer7[] = {
        0x05, 0x00, 0x01, 0x0f
        };
        const uint8_t jreq_ID_DAP_Transfer8[] = {
        0x05, 0x00, 0x01, 0x0b
        };
        const uint8_t jreq_ID_DAP_Transfer9[] = {
        0x05, 0x00, 0x06, 0x04, 0x32, 0x00, 0x00, 0x50, 0x08, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x00, 0x00, 0xe3, 0x05, 0x00, 0xed, 0x00, 0xe0, 0x0f, 0x06
        };

        memcpy(request->request_buf, jreq_ID_DAP_TransferConfigure, sizeof(jreq_ID_DAP_TransferConfigure));
        ret = request_handler(param, request->request_buf, response->response_buf, DAP_PACKET_SIZE);
        
        memcpy(request->request_buf, jreq_ID_DAP_JTAG_Configure, sizeof(jreq_ID_DAP_JTAG_Configure));
        ret = request_handler(param, request->request_buf, response->response_buf, DAP_PACKET_SIZE);

        memcpy(request->request_buf, jreq_ID_DAP_SWJ_Sequence, sizeof(jreq_ID_DAP_SWJ_Sequence));
        ret = request_handler(param, request->request_buf, response->response_buf, DAP_PACKET_SIZE);
        
        memcpy(request->request_buf, jreq_ID_DAP_Transfer1, sizeof(jreq_ID_DAP_Transfer1));
        ret = request_handler(param, request->request_buf, response->response_buf, DAP_PACKET_SIZE);
        
        memcpy(request->request_buf, jreq_ID_DAP_Transfer2, sizeof(jreq_ID_DAP_Transfer2));
        ret = request_handler(param, request->request_buf, response->response_buf, DAP_PACKET_SIZE);
        
        memcpy(request->request_buf, jreq_ID_DAP_Transfer3, sizeof(jreq_ID_DAP_Transfer3));
        ret = request_handler(param, request->request_buf, response->response_buf, DAP_PACKET_SIZE);
        
        memcpy(request->request_buf, jreq_ID_DAP_Transfer4, sizeof(jreq_ID_DAP_Transfer4));
        ret = request_handler(param, request->request_buf, response->response_buf, DAP_PACKET_SIZE);
        
        memcpy(request->request_buf, jreq_ID_DAP_Transfer5, sizeof(jreq_ID_DAP_Transfer5));
        ret = request_handler(param, request->request_buf, response->response_buf, DAP_PACKET_SIZE);
        
        memcpy(request->request_buf, jreq_ID_DAP_Transfer6, sizeof(jreq_ID_DAP_Transfer6));
        ret = request_handler(param, request->request_buf, response->response_buf, DAP_PACKET_SIZE);
        
        memcpy(request->request_buf, jreq_ID_DAP_Transfer7, sizeof(jreq_ID_DAP_Transfer7));
        ret = request_handler(param, request->request_buf, response->response_buf, DAP_PACKET_SIZE);
        
        memcpy(request->request_buf, jreq_ID_DAP_Transfer8, sizeof(jreq_ID_DAP_Transfer8));
        ret = request_handler(param, request->request_buf, response->response_buf, DAP_PACKET_SIZE);
        
        memcpy(request->request_buf, jreq_ID_DAP_Transfer9, sizeof(jreq_ID_DAP_Transfer9));
        ret = request_handler(param, request->request_buf, response->response_buf, DAP_PACKET_SIZE);
#endif
        
    }
}
