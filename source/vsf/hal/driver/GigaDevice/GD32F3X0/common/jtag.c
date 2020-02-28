#include "jtag.h"
#include "delay.h"

/*
Reference Document:
	"IEEE_1149_JTAG_and_Boundary_Scan_Tutorial"
	https://github.com/ARMmbed/DAPLink/blob/master/source/daplink/cmsis-dap/JTAG_DP.c
*/

void vsfhal_jtag_init(int32_t int_priority){}
void vsfhal_jtag_fini(void){}
void vsfhal_jtag_config(uint16_t kHz, uint16_t retry, uint8_t idle){}
void vsfhal_jtag_raw(uint16_t bitlen, uint8_t *tdi, uint8_t *tms, uint8_t *tdo){}
void vsfhal_jtag_ir(uint32_t ir, uint8_t lr_length, uint16_t ir_before, uint16_t ir_after){}
uint8_t vsfhal_jtag_dr(uint8_t request, uint32_t dr, uint16_t dr_before, uint16_t dr_after,
		uint32_t *data){}