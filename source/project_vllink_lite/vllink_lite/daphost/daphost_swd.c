#include "vsf.h"
#include "../dap/dap.h"
#include "daphost_priv.h"

// AP CSW register, base value
#define CSW_VALUE                           (CSW_RESERVED | CSW_MSTRDBG | CSW_HPROT | CSW_DBGSTAT | CSW_SADDRINC)

// This can vary from target to target and should be in the structure or flash blob
#define TARGET_AUTO_INCREMENT_PAGE_SIZE     (1024)

// Default NVIC and Core debug base addresses
// TODO: Read these addresses from ROM.
#define NVIC_Addr    (0xe000e000)
#define DBG_Addr     (0xe000edf0)

static SWD_CONNECT_TYPE reset_connect = CONNECT_NORMAL;
static DAP_STATE dap_state;
static uint32_t  soft_reset = SYSRESETREQ;

static vsf_err_t swd_read_dp(uint8_t adr, uint32_t *val)
{
    uint8_t ack;
    #ifdef SWD_ASYNC
    vsfhal_swd_read(SWD_REG_DP | SWD_REG_R | SWD_REG_ADR(adr), (uint8_t *)val);
    ack = vsfhal_swd_wait();
    #else
    ack = vsfhal_swd_read(SWD_REG_DP | SWD_REG_R | SWD_REG_ADR(adr), (uint8_t *)val);
    #endif
    return (ack == 1) ? VSF_ERR_NONE : VSF_ERR_FAIL;
}

static vsf_err_t swd_write_dp(uint8_t adr, uint32_t val)
{
    uint8_t ack;
    if ((dap_state.select == val) && (adr == DP_SELECT))
        return VSF_ERR_NONE;
    #ifdef SWD_ASYNC
    vsfhal_swd_write(SWD_REG_DP | SWD_REG_W | SWD_REG_ADR(adr), (uint8_t *)&val);
    ack = vsfhal_swd_wait();
    #else
    ack = vsfhal_swd_write(SWD_REG_DP | SWD_REG_W | SWD_REG_ADR(adr), (uint8_t *)&val);
    #endif
    if ((ack == DAP_TRANSFER_OK) && (adr == DP_SELECT))
        dap_state.select = val;
    return (ack == DAP_TRANSFER_OK) ? VSF_ERR_NONE : VSF_ERR_FAIL;
}

static vsf_err_t swd_read_ap(uint32_t adr, uint32_t *val)
{
    uint8_t ack, req;
    if (swd_write_dp(DP_SELECT, swd_get_apsel(adr) | (adr & APBANKSEL)))
        return VSF_ERR_FAIL;
    req = SWD_REG_AP | SWD_REG_R | SWD_REG_ADR(adr);
    vsfhal_swd_read(req, (uint8_t *)val);  // dummy read
    #ifdef SWD_ASYNC
    vsfhal_swd_read(req, (uint8_t *)val);
    ack = vsfhal_swd_wait();
    #else
    ack = vsfhal_swd_read(req, (uint8_t *)val);
    #endif
    return (ack == DAP_TRANSFER_OK) ? VSF_ERR_NONE : VSF_ERR_FAIL;
}

static vsf_err_t swd_write_ap(uint32_t adr, uint32_t val)
{
    uint8_t ack, req;
    if (swd_write_dp(DP_SELECT, swd_get_apsel(adr) | (adr & APBANKSEL)))
        return VSF_ERR_FAIL;
    if (adr == AP_CSW) {
        if (dap_state.csw == val)
            return VSF_ERR_NONE;
        dap_state.csw = val;
    }
    req = SWD_REG_AP | SWD_REG_W | SWD_REG_ADR(adr);
    #ifdef SWD_ASYNC
    vsfhal_swd_write(req, val);
    #else
    ack = vsfhal_swd_write(req, (uint8_t *)&val);
    if (ack != DAP_TRANSFER_OK)
        return VSF_ERR_FAIL;
    #endif
    req = SWD_REG_DP | SWD_REG_R | SWD_REG_ADR(DP_RDBUFF);
    #ifdef SWD_ASYNC
    ack = vsfhal_swd_read(req, NULL);
    if (ack != DAP_TRANSFER_OK)
        return VSF_ERR_FAIL;
    ack = vsfhal_swd_wait();
    #else
    ack = vsfhal_swd_write(req, NULL);
    #endif
    return (ack == DAP_TRANSFER_OK) ? VSF_ERR_NONE : VSF_ERR_FAIL;
}

static uint8_t swd_read_data(uint32_t addr, uint32_t *val)
{

}

static uint8_t swd_write_data(uint32_t address, uint32_t val)
{
    
}

static uint8_t swd_read_block(uint32_t addr, uint32_t size, uint8_t *buf)
{

}

static uint8_t swd_write_block(uint32_t addr, uint32_t size, uint8_t *buf)
{
    
}

static vsf_err_t swd_read_memory(uint32_t addr, uint32_t size, uint8_t *buf)
{
    uint8_t ack;
    uint32_t block, temp;
    
    if (size && (addr & 0x3)) {
        block = min(size, 4 - (addr & 0x3));
        size -= block;
        if (swd_write_ap(AP_CSW, CSW_VALUE | CSW_SIZE8))
            return VSF_ERR_FAIL;
        for (int i = 0; i < block; i++) {
            if (swd_read_data(addr, &temp))
                return VSF_ERR_FAIL;
            *buf++ = (temp >> ((addr & 0x03) << 3)) & 0xff;
            addr++;
        }
    }

    while (size > 3) {
        block = min(size & 0xfffffffc, TARGET_AUTO_INCREMENT_PAGE_SIZE - (addr & (TARGET_AUTO_INCREMENT_PAGE_SIZE - 1)));
        if (swd_read_block(addr, block, buf))
            return VSF_ERR_FAIL;
        addr += block;
        buf += block;
        size -= block;
    }

    if (size) {
        if (swd_write_ap(AP_CSW, CSW_VALUE | CSW_SIZE8))
            return VSF_ERR_FAIL;
        for (int i = 0; i < size; i++) {
            if (swd_read_data(addr + i, &temp))
                return VSF_ERR_FAIL;
            *buf++ = (temp >> ((addr & 0x03) << 3)) & 0xff;
            addr++;
        }
    }
    
    return VSF_ERR_NONE;
}

static vsf_err_t swd_write_memory(uint32_t addr, uint32_t size, uint8_t *buf)
{
    uint8_t ack;
    uint32_t block, temp;

    if (size && (addr & 0x3)) {
        block = min(size, 4 - (addr & 0x3));
        size -= block;
        if (swd_write_ap(AP_CSW, CSW_VALUE | CSW_SIZE8))
            return VSF_ERR_FAIL;
        for (int i = 0; i < block; i++) {
            temp = (uint32_t)buf[0] << ((addr & 0x03) << 3);
            if (swd_write_data(addr, temp))
                return VSF_ERR_FAIL;
            buf++;
            addr++;
        }
    }

    while (size > 3) {
        block = min(size & 0xfffffffc, TARGET_AUTO_INCREMENT_PAGE_SIZE - (addr & (TARGET_AUTO_INCREMENT_PAGE_SIZE - 1)));
        if (swd_write_block(addr, block, buf))
            return VSF_ERR_FAIL;
        addr += block;
        buf += block;
        size -= block;
    }

    if (size) {
        if (swd_write_ap(AP_CSW, CSW_VALUE | CSW_SIZE8))
            return VSF_ERR_FAIL;
        for (int i = 0; i < size; i++) {
            temp = (uint32_t)buf[0] << ((addr & 0x03) << 3);
            if (swd_write_data(addr + i, temp))
                return VSF_ERR_FAIL;
            buf++;
            addr++;
        }
    }
    
    return VSF_ERR_NONE;
}

static vsf_err_t swd_flash_syscall_exec(const program_syscall_t *sysCallParam,
        uint32_t entry, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4,
        flash_algo_return_t return_type)
{
    DEBUG_STATE state = {{0}, 0};
    // Call flash algorithm function on target and wait for result.
    state.r[0]     = arg1;                   // R0: Argument 1
    state.r[1]     = arg2;                   // R1: Argument 2
    state.r[2]     = arg3;                   // R2: Argument 3
    state.r[3]     = arg4;                   // R3: Argument 4
    state.r[9]     = sysCallParam->static_base;    // SB: Static Base
    state.r[13]    = sysCallParam->stack_pointer;  // SP: Stack Pointer
    state.r[14]    = sysCallParam->breakpoint;     // LR: Exit Point
    state.r[15]    = entry;                        // PC: Entry Point
    state.xpsr     = 0x01000000;          // xPSR: T = 1, ISR = 0
    
    if (swd_write_debug_state(&state))
        return VSF_ERR_FAIL;
    
    if (swd_wait_until_halted())
        return VSF_ERR_FAIL;
    
    if (swd_read_core_register(0, &state.r[0]))
        return VSF_ERR_FAIL;
    
    // remove the C_MASKINTS
    if (swd_write_word(DBG_HCSR, DBGKEY | C_DEBUGEN | C_HALT))
        return VSF_ERR_FAIL;
    
    if (return_type == FLASHALGO_RETURN_POINTER) {
        // Flash verify functions return pointer to byte following the buffer if successful.
        if (state.r[0] != (arg1 + arg2))
            return VSF_ERR_FAIL;
    } else {
        // Flash functions return 0 if successful.
        if (state.r[0])
            return VSF_ERR_FAIL;
    }
    
    return VSF_ERR_NONE;
}

static vsf_err_t swd_set_target_state_hw(target_state_t state)
{
    
}

static vsf_err_t swd_set_target_state_sw(target_state_t state)
{
}

static void swd_set_reset_connect(SWD_CONNECT_TYPE type)
{
    reset_connect = type;
}

static void swd_set_soft_reset(uint32_t soft_reset_type)
{
    soft_reset = soft_reset_type;
}
