#include "vsf.h"

void delay_jtag_125ns(uint16_t dummy)
{
	__ASM("NOP");
	__ASM("NOP");
	__ASM("NOP");
	__ASM("NOP");
}

void delay_jtag_250ns(uint16_t dummy)
{
	dummy = 7;
	while (--dummy);
}

void delay_swd_125ns(uint16_t dummy)
{
	__ASM("NOP");
	__ASM("NOP");
	__ASM("NOP");
	__ASM("NOP");
	__ASM("NOP");
	__ASM("NOP");
	__ASM("NOP");
	__ASM("NOP");
	__ASM("NOP");
	__ASM("NOP");
	__ASM("NOP");
}

void delay_swd_250ns(uint16_t dummy)
{
	dummy = 9;
	while (--dummy);
}


