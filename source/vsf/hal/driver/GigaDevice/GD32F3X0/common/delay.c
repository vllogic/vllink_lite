#include "delay.h"

#if __IS_COMPILER_IAR__

#pragma optimize = low
void vsfhal_delay_jtag_125ns(uint16_t dummy)
{
	__ASM("NOP");
	__ASM("NOP");
	__ASM("NOP");
	__ASM("NOP");
}

#pragma optimize = low
void vsfhal_delay_jtag_250ns(uint16_t dummy)
{
	dummy = 7;
	while (--dummy);
}

#pragma optimize = low
void vsfhal_delay_swd_125ns(uint16_t dummy)
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

#pragma optimize = low
void vsfhal_delay_swd_250ns(uint16_t dummy)
{
	dummy = 9;
	while (--dummy);
}

#elif __IS_COMPILER_GCC_

void vsfhal_delay_jtag_125ns(uint16_t dummy) __attribute__((optimize("O0")))
{
	__ASM("NOP");
	__ASM("NOP");
	__ASM("NOP");
	__ASM("NOP");
}

void vsfhal_delay_jtag_250ns(uint16_t dummy) __attribute__((optimize("O0")))
{
	dummy = 7;
	while (--dummy);
}

void vsfhal_delay_swd_125ns(uint16_t dummy) __attribute__((optimize("O0")))
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

void vsfhal_delay_swd_250ns(uint16_t dummy) __attribute__((optimize("O0")))
{
	dummy = 9;
	while (--dummy);
}

#else
#	error "Not Support"
#endif




