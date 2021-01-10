/*============================ INCLUDES ======================================*/

#include "hal/vsf_hal_cfg.h"
#include "../__device.h"

/*============================ MACROS ========================================*/

#if !defined(__VSF_HAL_SWI_NUM)
//! when there is no defined __VSF_HAL_SWI_NUM, use the maximum available value
#   define __VSF_DEV_SWI_NUM                VSF_DEV_SWI_NUM
#elif __VSF_HAL_SWI_NUM > VSF_ARCH_SWI_NUM
#   if (__VSF_HAL_SWI_NUM - VSF_ARCH_SWI_NUM) > VSF_DEV_SWI_NUM
#       define MFUNC_IN_U8_DEC_VALUE       (VSF_DEV_SWI_NUM)
#   else
#       define MFUNC_IN_U8_DEC_VALUE       (__VSF_HAL_SWI_NUM - VSF_ARCH_SWI_NUM)
#   endif
#   include "utilities/preprocessor/mf_u8_dec2str.h"
#   define __VSF_DEV_SWI_NUM           MFUNC_OUT_DEC_STR
#else
#   define __VSF_DEV_SWI_NUM           0
#endif

#define __GD32E10X_SWI(__N, __VALUE)                                            \
    ROOT ISR(SWI##__N##_IRQHandler)                                             \
    {                                                                           \
        if (__gd32e10x_common.swi[__N].handler != NULL) {                       \
            __gd32e10x_common.swi[__N].handler(__gd32e10x_common.swi[__N].param);\
        }                                                                       \
    }

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

#if __VSF_DEV_SWI_NUM > 0
static const IRQn_Type gd32e10x_soft_irq[VSF_DEV_SWI_NUM] = {
    VSF_DEV_SWI_LIST
};

typedef struct gd32e10x_common_t {
    struct {
        vsf_swi_handler_t *handler;
        void *param;
    } swi[__VSF_DEV_SWI_NUM];
} gd32e10x_common_t;
#endif

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/

#if __VSF_DEV_SWI_NUM > 0
static gd32e10x_common_t __gd32e10x_common;
#endif

/*============================ PROTOTYPES ====================================*/

extern vsf_err_t vsf_usr_swi_init(uint_fast8_t idx, 
                                vsf_arch_prio_t priority,
                                vsf_swi_handler_t *handler, 
                                void *param);
extern void vsf_usr_swi_trigger(uint_fast8_t idx);

/*============================ IMPLEMENTATION ================================*/

// SWI
#if __VSF_DEV_SWI_NUM > 0
REPEAT_MACRO(__VSF_DEV_SWI_NUM, __GD32E10X_SWI, NULL)

static ALWAYS_INLINE vsf_err_t vsf_drv_swi_init(uint_fast8_t idx, 
                                                vsf_arch_prio_t priority,
                                                vsf_swi_handler_t *handler, 
                                                void *param)
{
    if (idx < __VSF_DEV_SWI_NUM) {
        if (handler != NULL) {
            __gd32e10x_common.swi[idx].handler = handler;
            __gd32e10x_common.swi[idx].param = param;

            NVIC_SetPriority(gd32e10x_soft_irq[idx], priority);
            NVIC_EnableIRQ(gd32e10x_soft_irq[idx]);
        } else {
            NVIC_DisableIRQ(gd32e10x_soft_irq[idx]);
        }
        return VSF_ERR_NONE;
    }
    VSF_HAL_ASSERT(false);
    return VSF_ERR_FAIL;
}

static ALWAYS_INLINE void vsf_drv_swi_trigger(uint_fast8_t idx)
{
    if (idx < __VSF_DEV_SWI_NUM) {
        NVIC_SetPendingIRQ(gd32e10x_soft_irq[idx]);
        return;
    }
    VSF_HAL_ASSERT(false);
}
#endif

#if __VSF_HAL_SWI_NUM > 0 || !defined(__VSF_HAL_SWI_NUM)
// SWI

#ifndef WEAK_VSF_USR_SWI_TRIGGER
WEAK(vsf_usr_swi_trigger)
void vsf_usr_swi_trigger(uint_fast8_t idx)
{
    VSF_HAL_ASSERT(false);
}
#endif

void vsf_drv_usr_swi_trigger(uint_fast8_t idx)
{
#if __VSF_HAL_SWI_NUM > VSF_ARCH_SWI_NUM || !defined(__VSF_HAL_SWI_NUM)
#   if __VSF_DEV_SWI_NUM > 0
    if (idx < __VSF_DEV_SWI_NUM) {
        vsf_drv_swi_trigger(idx);
        return;
    }
    idx -= __VSF_DEV_SWI_NUM;
#   endif

#   if      (__VSF_HAL_SWI_NUM > VSF_ARCH_SWI_NUM + __VSF_DEV_SWI_NUM)          \
        ||  !defined(__VSF_HAL_SWI_NUM)
    vsf_usr_swi_trigger(idx);
#   else
    VSF_HAL_ASSERT(false);
#   endif
#else
    VSF_HAL_ASSERT(false);
#endif
}

#ifndef WEAK_VSF_USR_SWI_INIT
WEAK(vsf_usr_swi_init)
vsf_err_t vsf_usr_swi_init(uint_fast8_t idx, 
                                vsf_arch_prio_t priority,
                                vsf_swi_handler_t *handler, 
                                void *param)
{
    VSF_HAL_ASSERT(false);
    return VSF_ERR_FAIL;
}
#endif

vsf_err_t vsf_drv_usr_swi_init( uint_fast8_t idx, 
                                vsf_arch_prio_t priority,
                                vsf_swi_handler_t *handler, 
                                void *param)
{
#if __VSF_HAL_SWI_NUM > VSF_ARCH_SWI_NUM || !defined(__VSF_HAL_SWI_NUM)
#   if __VSF_DEV_SWI_NUM > 0
    if (idx < __VSF_DEV_SWI_NUM) {
        return vsf_drv_swi_init(idx, priority, handler, param);
    }
    idx -= __VSF_DEV_SWI_NUM;
#   endif

#   if      (__VSF_HAL_SWI_NUM > VSF_ARCH_SWI_NUM + __VSF_DEV_SWI_NUM)          \
        ||  !defined(__VSF_HAL_SWI_NUM)
    return vsf_usr_swi_init(idx, priority, handler, param);
#   else
    VSF_HAL_ASSERT(false);
    return VSF_ERR_FAIL;
#   endif
#else
    VSF_HAL_ASSERT(false);
    return VSF_ERR_FAIL;
#endif
}
#endif

