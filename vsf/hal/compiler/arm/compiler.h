#ifndef __COMPILER_H_INCLUDED__
#define __COMPILER_H_INCLUDED__

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "cmsis_compiler.h"
#include "arm_math.h"

#if defined(__IS_COMPILER_IAR__) || defined(__IAR_SYSTEMS_ICC__)
#   define ROOT                 __root
#   define ALWAYS_INLINE        __attribute__((always_inline))
#   define WEAK                 __weak
#   define RAMFUNC              __ramfunc
#   define __asm__              __asm
#   define PACKED               __attribute__((packed))
#   define UNALIGNED            __attribute__((packed))
#elif defined(__GNUC__)
#   define ROOT                 __attribute__((used))
#   define ALWAYS_INLINE        __attribute__((always_inline))
#   define WEAK                 __attribute__((weak))
#   define RAMFUNC              __attribute__((section (".textrw")))
#   define __asm__              __asm
#   define PACKED               __attribute__((packed))
#   define UNALIGNED            __attribute__((packed))
#else
#   error "No Compiler Selected!"
#endif


#ifndef NOP
    #define NOP()                               __asm__ __volatile__ ("nop");
#endif

#if defined(__IS_COMPILER_IAR__) || defined(__IAR_SYSTEMS_ICC__)
#   define ENABLE_GLOBAL_INTERRUPT()            __iar_builtin_enable_interrupt()
#   define DISABLE_GLOBAL_INTERRUPT()           __iar_builtin_disable_interrupt()
typedef __istate_t                              istate_t;
#   define SET_GLOBAL_INTERRUPT_STATE(__STATE)  __iar_builtin_set_interrupt_state(__STATE)
#   define GET_GLOBAL_INTERRUPT_STATE()         __iar_builtin_get_interrupt_state()
#elif defined(__GNUC__)
#   define ENABLE_GLOBAL_INTERRUPT()            __enable_irq()
#   define DISABLE_GLOBAL_INTERRUPT()           __disable_irq()
typedef uint32_t                                istate_t;
#   define SET_GLOBAL_INTERRUPT_STATE(__STATE)  __set_PRIMASK(__STATE)
#   define GET_GLOBAL_INTERRUPT_STATE()         __get_PRIMASK()
#else
#   error("No Compiler Selected!")
#endif

static ALWAYS_INLINE inline void compiler_set_stack(unsigned int stack)
{
    __set_MSP(stack);
}

static ALWAYS_INLINE inline void compiler_set_pc(unsigned int pc)
{
    __asm__("MOV pc, %0" : :"r"(pc));
}

static ALWAYS_INLINE inline unsigned int compiler_get_lr(void)
{
    unsigned int reg;
    __asm__("MOV %0, lr" : "=r"(reg));
    return reg;
}

#endif	// __COMPILER_H_INCLUDED__
