#include "vsf.h"
#include "../dap/dap.h"
#include "daphost_priv.h"
#include "daphost_jtag.h"
#include "daphost_swd.h"

static daphost_op_t const *daphost_op = NULL;

vsf_err_t daphost_init(void)
{
    // TODO
    //daphost_op = &daphost_jtag_op;
    daphost_op = &daphost_swd_op;
    return VSF_ERR_NONE;
}

vsf_err_t daphost_fini(void)
{
    if (daphost_op)
        daphost_op = NULL;
    return VSF_ERR_NONE;
}

vsf_err_t daphost_read_dp(uint8_t adr, uint32_t *val)
{
    if (daphost_op)
        return daphost_op->read_dp(adr, val);
    else
        return VSF_ERR_NOT_SUPPORT;
}

vsf_err_t daphost_write_dp(uint8_t adr, uint32_t val)
{
    if (daphost_op)
        return daphost_op->write_dp(adr, val);
    else
        return VSF_ERR_NOT_SUPPORT;
}

vsf_err_t daphost_read_ap(uint32_t adr, uint32_t *val)
{
    if (daphost_op)
        return daphost_op->read_ap(adr, val);
    else
        return VSF_ERR_NOT_SUPPORT;
}

vsf_err_t daphost_write_ap(uint32_t adr, uint32_t val)
{
    if (daphost_op)
        return daphost_op->write_ap(adr, val);
    else
        return VSF_ERR_NOT_SUPPORT;
}

vsf_err_t daphost_read_memory(uint32_t addr, uint32_t size, uint8_t *buf)
{
    if (daphost_op)
        return daphost_op->read_memory(addr, size, buf);
    else
        return VSF_ERR_NOT_SUPPORT;
}

vsf_err_t daphost_write_memory(uint32_t addr, uint32_t size, uint8_t *buf)
{
    if (daphost_op)
        return daphost_op->write_memory(addr, size, buf);
    else
        return VSF_ERR_NOT_SUPPORT;
}

vsf_err_t daphost_flash_syscall_exec(const program_syscall_t *sysCallParam,
        uint32_t entry, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4,
        flash_algo_return_t return_type)
{
    if (daphost_op)
        return daphost_op->flash_syscall_exec(sysCallParam, entry, arg1, arg2, arg3, arg4, return_type);
    else
        return VSF_ERR_NOT_SUPPORT;
}

vsf_err_t daphost_set_target_state_hw(target_state_t state)
{
    if (daphost_op)
        return daphost_op->set_target_state_hw(state);
    else
        return VSF_ERR_NOT_SUPPORT;
}

vsf_err_t daphost_set_target_state_sw(target_state_t state)
{
    if (daphost_op)
        return daphost_op->set_target_state_sw(state);
    else
        return VSF_ERR_NOT_SUPPORT;
}

void daphost_set_reset_connect(SWD_CONNECT_TYPE type)
{
    if (daphost_op)
        daphost_op->set_reset_connect(type);
}

void daphost_set_soft_reset(uint32_t soft_reset_type)
{
    if (daphost_op)
        daphost_op->set_soft_reset(soft_reset_type);
}
