/*============================ INCLUDES ======================================*/
#include "flash.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

#if FLASH_COUNT > 0

#define SPC_U16_NO_SECURITY         0x5aa5
#define SPC_U16_HIGH_SECURITY       0x33cc

vsf_err_t vsfhal_flash_security_config(enum flash_idx_t idx, uint32_t config)
{
    VSF_HAL_ASSERT(idx < GPIO_IDX_NUM);

    uint16_t spc_u16 = OB_SPC & 0xffff, new_spc = 0xffff;
    bool do_erase = false;

    switch (config) {
    case FLASH_SECURITY_NULL:
        if (spc_u16 == SPC_U16_HIGH_SECURITY) {
            return VSF_ERR_NOT_SUPPORT;
        } else if (spc_u16 != SPC_U16_NO_SECURITY) {
            if (spc_u16 != 0xffff)
                do_erase = true;
            new_spc = SPC_U16_NO_SECURITY;
        }
        break;
    case FLASH_SECURITY_LOW:
        if (spc_u16 == SPC_U16_HIGH_SECURITY) {
            return VSF_ERR_NOT_SUPPORT;
        } else if (spc_u16 == SPC_U16_NO_SECURITY) {
            do_erase = true;
        }
        break;
    case FLASH_SECURITY_HIGH:
        if (spc_u16 != SPC_U16_HIGH_SECURITY) {
            if (spc_u16 != 0xffff)
                do_erase = true;
            new_spc = SPC_U16_HIGH_SECURITY;
        }
        break;    
    }

    if (do_erase || (new_spc != 0xffff)) {
        FMC_KEY = UNLOCK_KEY0;
        FMC_KEY = UNLOCK_KEY1;
        FMC_OBKEY = UNLOCK_KEY0;
        FMC_OBKEY = UNLOCK_KEY1;

        if (do_erase) {
            FMC_CTL |= FMC_CTL_OBER;
            FMC_CTL |= FMC_CTL_START;
            while (FMC_STAT & FMC_STAT_BUSY);
            FMC_CTL &= ~FMC_CTL_OBER;
        }
        
        if (new_spc != 0xffff) {
            FMC_CTL |= FMC_CTL_OBPG;
            OB_SPC = new_spc;
            while (FMC_STAT & FMC_STAT_BUSY);
            FMC_CTL &= ~FMC_CTL_OBPG;
        }

        FMC_CTL = FMC_CTL_LK;
    }

    return VSF_ERR_NONE;
}

uint32_t vsfhal_flash_opsize(enum flash_idx_t idx, uint32_t addr, enum flash_op_t op)
{
    VSF_HAL_ASSERT(idx < GPIO_IDX_NUM);

    if (idx == FLASH0_IDX) {
        switch (op) {
        case FLASH_READ:
            return 1;
        case FLASH_WRITE:
            return 4;
        case FLASH_ERASE:
            return 1024;
        }
    }
    return 0;
}

vsf_err_t vsfhal_flash_read(enum flash_idx_t idx, uint32_t addr, uint32_t size, uint8_t *buff)
{
    VSF_HAL_ASSERT(idx < GPIO_IDX_NUM);

    if (idx == FLASH0_IDX) {
        memcpy(buff, (void *)addr, size);
        return size;
    }
    return 0;
}

vsf_err_t vsfhal_flash_write(enum flash_idx_t idx, uint32_t addr, uint32_t size, uint8_t *buff)
{
    VSF_HAL_ASSERT(idx < GPIO_IDX_NUM);

    if (idx == FLASH0_IDX) {
        uint32_t i;
        uint32_t *w = (uint32_t *)addr;
        uint32_t *r = (uint32_t *)buff;

        FMC_KEY = UNLOCK_KEY0;
        FMC_KEY = UNLOCK_KEY1;

        FMC_CTL |= FMC_CTL_PG;
        for (i = 0; i < size; i += 4) {
            
            *w++ = *r++;
            while (FMC_STAT & FMC_STAT_BUSY);
            
        }
        FMC_CTL &= ~FMC_CTL_PG;

        FMC_CTL = FMC_CTL_LK;

        return size;
    }
    return 0;
}

vsf_err_t vsfhal_flash_erase(enum flash_idx_t idx, uint8_t index, uint32_t addr, uint32_t size)
{
    VSF_HAL_ASSERT(idx < GPIO_IDX_NUM);

    if (idx == FLASH0_IDX) {
        FMC_KEY = UNLOCK_KEY0;
        FMC_KEY = UNLOCK_KEY1;

        FMC_CTL |= FMC_CTL_PER;
        for (uint32_t i = 0; i < size; i += 1024) {
            FMC_ADDR = addr + i;
            FMC_CTL |= FMC_CTL_START;
            while (FMC_STAT & FMC_STAT_BUSY);
        }
        FMC_CTL &= ~FMC_CTL_PER;

        FMC_CTL = FMC_CTL_LK;

        return size;
    }
    return 0;
}

#endif
