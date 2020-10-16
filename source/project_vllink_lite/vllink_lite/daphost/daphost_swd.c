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

#define DCRDR 0xE000EDF8
#define DCRSR 0xE000EDF4
#define DHCSR 0xE000EDF0
#define REGWnR (1 << 16)

#define MAX_SWD_RETRY 100//10
#define MAX_TIMEOUT   1000000  // Timeout for syscalls on target

static SWD_CONNECT_TYPE reset_connect = CONNECT_NORMAL;
static DAP_STATE dap_state;
static uint32_t  soft_reset = SYSRESETREQ;

static uint32_t swd_get_apsel(uint32_t adr)
{
    // TODO: get target apsel
    return adr & 0xff000000;
}

static vsf_err_t swd_read_dp(uint8_t adr, uint32_t *val)
{
    uint8_t ack;

    #ifdef SWD_ASYNC
    vsfhal_swd_read(SWD_REG_DP | SWD_REG_R | SWD_REG_ADR(adr), (uint8_t *)val);
    ack = vsfhal_swd_wait();
    #else
    ack = vsfhal_swd_read(SWD_REG_DP | SWD_REG_R | SWD_REG_ADR(adr), (uint8_t *)val);
    #endif

    return (ack == DAP_TRANSFER_OK) ? VSF_ERR_NONE : VSF_ERR_FAIL;
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

static vsf_err_t swd_read_block(uint32_t addr, uint32_t size, uint8_t *buf)
{
    uint8_t ack, req;
    uint32_t i;

    if (swd_write_ap(AP_CSW, CSW_VALUE | CSW_SIZE32))
        return VSF_ERR_FAIL;
    
    // TAR write
    req = SWD_REG_AP | SWD_REG_W | AP_TAR;
    #ifdef SWD_ASYNC
    vsfhal_swd_write(req, (uint8_t *)&addr);
    #else
    ack = vsfhal_swd_write(req, (uint8_t *)&addr);
    if (ack != DAP_TRANSFER_OK)
        return VSF_ERR_FAIL;
    #endif
    
    // read data
    req = SWD_REG_AP | SWD_REG_R | AP_DRW;
    ack = vsfhal_swd_read(req, NULL);
    if (ack != DAP_TRANSFER_OK)
        return VSF_ERR_FAIL;
    
    for (i = 0; i < (size - 4); i += 4) {
        ack = vsfhal_swd_read(req, buf + i);
        if (ack != DAP_TRANSFER_OK)
            return VSF_ERR_FAIL;
    }
    
    // read last word
    #ifdef SWD_ASYNC
    ack = vsfhal_swd_read(req, buf + i);
    if (ack != DAP_TRANSFER_OK)
        return VSF_ERR_FAIL;
    ack = vsfhal_swd_wait();
    #else
    ack = vsfhal_swd_read(req, buf + i);
    #endif

    return (ack == DAP_TRANSFER_OK) ? VSF_ERR_NONE : VSF_ERR_FAIL;
}

static vsf_err_t swd_write_block(uint32_t addr, uint32_t size, uint8_t *buf)
{
    uint8_t ack, req;
    uint32_t i;

    if (swd_write_ap(AP_CSW, CSW_VALUE | CSW_SIZE32))
        return VSF_ERR_FAIL;

    // TAR write
    req = SWD_REG_AP | SWD_REG_W | AP_TAR;
    #ifdef SWD_ASYNC
    vsfhal_swd_write(req, (uint8_t *)&addr);
    #else
    ack = vsfhal_swd_write(req, (uint8_t *)&addr);
    if (ack != DAP_TRANSFER_OK)
        return VSF_ERR_FAIL;
    #endif
    
    // DRW write
    req = SWD_REG_AP | SWD_REG_W | AP_DRW;
    for (i = 0; i < size; i += 4) {
        ack = vsfhal_swd_write(req, buf + i);
        if (ack != DAP_TRANSFER_OK)
            return VSF_ERR_FAIL;
    }
    
    // dummy read
    req = SWD_REG_DP | SWD_REG_R | SWD_REG_ADR(DP_RDBUFF);
    #ifdef SWD_ASYNC
    ack = vsfhal_swd_read(req, NULL);
    if (ack != DAP_TRANSFER_OK)
        return VSF_ERR_FAIL;
    ack = vsfhal_swd_wait();
    #else
    ack = vsfhal_swd_read(req, NULL);
    #endif

    return (ack == DAP_TRANSFER_OK) ? VSF_ERR_NONE : VSF_ERR_FAIL;
}

static vsf_err_t swd_read_data(uint32_t addr, uint32_t *val)
{
    uint8_t ack, req;

    // TAR write
    req = SWD_REG_AP | SWD_REG_W | AP_TAR;
    #ifdef SWD_ASYNC
    vsfhal_swd_write(req, (uint8_t *)&addr);
    #else
    ack = vsfhal_swd_write(req, (uint8_t *)&addr);
    if (ack != DAP_TRANSFER_OK)
        return VSF_ERR_FAIL;
    #endif
    
    // read data
    req = SWD_REG_AP | SWD_REG_R | AP_DRW;
    ack = vsfhal_swd_read(req, NULL);
    if (ack != DAP_TRANSFER_OK)
        return VSF_ERR_FAIL;

    // read data
    #ifdef SWD_ASYNC
    ack = vsfhal_swd_read(req, (uint8_t *)val);
    if (ack != DAP_TRANSFER_OK)
        return VSF_ERR_FAIL;
    ack = vsfhal_swd_wait();
    #else
    ack = vsfhal_swd_read(req, (uint8_t *)val);
    #endif
    
    return (ack == DAP_TRANSFER_OK) ? VSF_ERR_NONE : VSF_ERR_FAIL;
}

static vsf_err_t swd_write_data(uint32_t addr, uint32_t val)
{
    uint8_t ack, req;

    // TAR write
    req = SWD_REG_AP | SWD_REG_W | AP_TAR;
    #ifdef SWD_ASYNC
    vsfhal_swd_write(req, (uint8_t *)&addr);
    #else
    ack = vsfhal_swd_write(req, (uint8_t *)&addr);
    if (ack != DAP_TRANSFER_OK)
        return VSF_ERR_FAIL;
    #endif
    
    // DRW write
    req = SWD_REG_AP | SWD_REG_W | AP_DRW;
    ack = vsfhal_swd_write(req, (uint8_t *)&val);
    if (ack != DAP_TRANSFER_OK)
        return VSF_ERR_FAIL;
    
    // dummy read
    req = SWD_REG_DP | SWD_REG_R | SWD_REG_ADR(DP_RDBUFF);
    #ifdef SWD_ASYNC
    ack = vsfhal_swd_read(req, NULL);
    if (ack != DAP_TRANSFER_OK)
        return VSF_ERR_FAIL;
    ack = vsfhal_swd_wait();
    #else
    ack = vsfhal_swd_read(req, NULL);
    #endif

    return (ack == DAP_TRANSFER_OK) ? VSF_ERR_NONE : VSF_ERR_FAIL;
}

static vsf_err_t swd_read_word(uint32_t addr, uint32_t *val)
{
    if (swd_write_ap(AP_CSW, CSW_VALUE | CSW_SIZE32))
        return VSF_ERR_FAIL;
    return swd_read_data(addr, val);
}

static vsf_err_t swd_write_word(uint32_t addr, uint32_t val)
{
    if (swd_write_ap(AP_CSW, CSW_VALUE | CSW_SIZE32))
        return VSF_ERR_FAIL;
    return swd_write_data(addr, val);
}

static vsf_err_t swd_read_byte(uint32_t addr, uint8_t *val)
{
    uint32_t tmp;

    if (swd_write_ap(AP_CSW, CSW_VALUE | CSW_SIZE8))
        return VSF_ERR_FAIL;

    if (swd_read_data(addr, &tmp))
        return VSF_ERR_FAIL;

    *val = (uint8_t)(tmp >> ((addr & 0x03) << 3));
    return VSF_ERR_NONE;
}

static vsf_err_t swd_write_byte(uint32_t addr, uint8_t val)
{
    uint32_t tmp;

    if (swd_write_ap(AP_CSW, CSW_VALUE | CSW_SIZE8))
        return VSF_ERR_FAIL;

    tmp = (uint32_t)val << ((addr & 0x03) << 3);

    return swd_write_data(addr, tmp);
}

static vsf_err_t swd_read_memory(uint32_t addr, uint32_t size, uint8_t *buf)
{
    uint32_t i, block;
    
    if (size && (addr & 0x3)) {
        block = min(size, 4 - (addr & 0x3));
        for (i = 0; i < block; i++)
            swd_read_byte(addr + i, buf + i);
        size -= block;
        addr += block;
        buf += block;
    }

    while (size > 3) {
        block = min(size & 0xfffffffc, TARGET_AUTO_INCREMENT_PAGE_SIZE - (addr & (TARGET_AUTO_INCREMENT_PAGE_SIZE - 1)));
        if (swd_read_block(addr, block, buf))
            return VSF_ERR_FAIL;
        size -= block;
        addr += block;
        buf += block;
    }

    if (size) {
        for (i = 0; i < size; i++)
            swd_read_byte(addr + i, buf + i);
    }
    
    return VSF_ERR_NONE;
}

static vsf_err_t swd_write_memory(uint32_t addr, uint32_t size, uint8_t *buf)
{
    uint32_t i, block;

    if (size && (addr & 0x3)) {
        block = min(size, 4 - (addr & 0x3));
        for (i = 0; i < block; i++)
            swd_write_byte(addr + i, buf[i]);
        size -= block;
        addr += block;
        buf += block;
    }

    while (size > 3) {
        block = min(size & 0xfffffffc, TARGET_AUTO_INCREMENT_PAGE_SIZE - (addr & (TARGET_AUTO_INCREMENT_PAGE_SIZE - 1)));
        if (swd_write_block(addr, block, buf))
            return VSF_ERR_FAIL;
        size -= block;
        addr += block;
        buf += block;
    }

    if (size) {
        for (i = 0; i < size; i++)
            swd_write_byte(addr + i, buf[i]);
    }
    
    return VSF_ERR_NONE;
}

vsf_err_t swd_read_core_register(uint32_t n, uint32_t *val)
{
    uint32_t i, timeout = 100;
    
    if (swd_write_word(DCRSR, n))
        return VSF_ERR_FAIL;
    
    // wait for S_REGRDY
    for (i = 0; i < timeout; i++) {
        if (swd_read_word(DHCSR, val))
            return VSF_ERR_FAIL;
        if (*val & S_REGRDY)
            break;
    }

    if (i == timeout)
        return VSF_ERR_FAIL;

    if (swd_read_word(DCRDR, val))
        return VSF_ERR_FAIL;
    
    return VSF_ERR_NONE;
}

vsf_err_t swd_write_core_register(uint32_t n, uint32_t val)
{
    uint32_t i, timeout = 100;
    
    if (swd_write_word(DCRDR, val))
        return VSF_ERR_FAIL;
    
    if (swd_write_word(DCRSR, n | REGWnR))
        return VSF_ERR_FAIL;
    
    // wait for S_REGRDY
    for (i = 0; i < timeout; i++) {
        if (swd_read_word(DHCSR, &val))
            return VSF_ERR_FAIL;
        if (val & S_REGRDY)
            return VSF_ERR_NONE;
    }

    return VSF_ERR_FAIL;
}

static vsf_err_t swd_write_debug_state(DEBUG_STATE *state)
{
    uint32_t i, status;

    if (swd_write_dp(DP_SELECT, 0))
        return VSF_ERR_FAIL;

    // R0, R1, R2, R3
    for (i = 0; i < 4; i++) {
        if (swd_write_core_register(i, state->r[i]))
            return VSF_ERR_FAIL;
    }

    // R9
    if (swd_write_core_register(9, state->r[9]))
        return VSF_ERR_FAIL;

    // R13, R14, R15
    for (i = 13; i < 16; i++) {
        if (swd_write_core_register(i, state->r[i]))
            return VSF_ERR_FAIL;
    }

    // xPSR
    if (swd_write_core_register(16, state->xpsr))
        return VSF_ERR_FAIL;

    if (swd_write_word(DBG_HCSR, DBGKEY | C_DEBUGEN | C_MASKINTS | C_HALT))
        return VSF_ERR_FAIL;

    if (swd_write_word(DBG_HCSR, DBGKEY | C_DEBUGEN | C_MASKINTS))
        return VSF_ERR_FAIL;

    // check status
    if (swd_read_dp(DP_CTRL_STAT, &status))
        return VSF_ERR_FAIL;

    if (status & (STICKYERR | WDATAERR))
        return VSF_ERR_FAIL;

    return VSF_ERR_NONE;
}

static vsf_err_t swd_wait_until_halted(void)
{
    // Wait for target to stop
    uint32_t val, i, timeout = MAX_TIMEOUT;

    for (i = 0; i < timeout; i++) {
        if (swd_read_word(DBG_HCSR, &val))
            return VSF_ERR_FAIL;
        if (val & S_HALT)
            return VSF_ERR_NONE;
    }

    return VSF_ERR_FAIL;
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
    // TODO
}

static vsf_err_t swd_set_target_state_sw(target_state_t state)
{
    // TODO
}

static void swd_set_reset_connect(SWD_CONNECT_TYPE type)
{
    reset_connect = type;
}

static void swd_set_soft_reset(uint32_t soft_reset_type)
{
    soft_reset = soft_reset_type;
}

const daphost_op_t daphost_swd_op = {
    swd_read_dp,
    swd_write_dp,
    swd_read_ap,
    swd_write_ap,
    swd_read_memory,
    swd_write_memory,
    swd_read_core_register,
    swd_write_core_register,
    swd_flash_syscall_exec,
    swd_set_target_state_hw,
    swd_set_target_state_sw,
    swd_set_reset_connect,
    swd_set_soft_reset,
};
