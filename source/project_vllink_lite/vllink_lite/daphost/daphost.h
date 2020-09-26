#ifndef __DAPHOST_H__
#define __DAPHOST_H__

#ifdef __cplusplus
extern "C" {
#endif

vsf_err_t daphost_read_dp(uint8_t adr, uint32_t *val);
vsf_err_t daphost_write_dp(uint8_t adr, uint32_t val);
vsf_err_t daphost_read_ap(uint32_t adr, uint32_t *val);
vsf_err_t daphost_write_ap(uint32_t adr, uint32_t val);
vsf_err_t daphost_read_memory(uint32_t addr, uint32_t size, uint8_t *buf);
vsf_err_t daphost_write_memory(uint32_t addr, uint32_t size, uint8_t *buf);
vsf_err_t daphost_flash_syscall_exec(const program_syscall_t *sysCallParam, uint32_t entry, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, flash_algo_return_t return_type);
vsf_err_t daphost_set_target_state_hw(target_state_t state);
vsf_err_t daphost_set_target_state_sw(target_state_t state);
void daphost_set_reset_connect(SWD_CONNECT_TYPE type);
void daphost_set_soft_reset(uint32_t soft_reset_type);

#ifdef __cplusplus
}
#endif

#endif // __DAPHOST_H__
