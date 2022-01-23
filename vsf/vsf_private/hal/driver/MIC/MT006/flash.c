/*============================ INCLUDES ======================================*/

#include "flash.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

#if FLASH_COUNT > 0

/*
Name    Size    Pyh Addr    Logic Addr

APROM   32K     0x08000000  0x08000000
                0x08007FFF  0x08007FFF

Info    1K      0x08008000  0x1FFFF400
                0x080083FF  0x1FFFF7FF
*/

vsf_err_t vsfhal_flash_security_config(enum flash_idx_t idx, uint32_t config)
{
    return VSF_ERR_NOT_SUPPORT;
}

uint32_t vsfhal_flash_opsize(enum flash_idx_t idx, uint32_t addr, enum flash_op_t op)
{
    if (idx == FLASH_INVALID_IDX)
        return 4;
    VSF_HAL_ASSERT(idx < FLASH_IDX_NUM);

    if (idx == FLASH0_IDX) {
        switch (op) {
        case FLASH_READ:
            return 1;
        case FLASH_WRITE:
            return 4;
        case FLASH_ERASE:
            return 512;
        }
    }
    return 0;
}

static uint32_t get_logic_addr(uint32_t phy_addr)
{
    if (phy_addr <= 0x08007FFF)
      return phy_addr;
    else
      return phy_addr + 0x1FFFF400 - 0x08008000;
}

uint32_t vsfhal_flash_read(enum flash_idx_t idx, uint32_t addr, uint32_t size, uint8_t *buff)
{
    if (idx == FLASH_INVALID_IDX)
        return size;
    VSF_HAL_ASSERT(idx < FLASH_IDX_NUM);

    if (idx == FLASH0_IDX) {
        addr = get_logic_addr(addr);
        memcpy(buff, (void *)addr, size);
        return size;
    }
    return 0;
}

static void flash_erase_write_start(void)
{
    FLASH->ACR &= ~FLASH_ACR_CACHENA;
    FLASH->ACR |= FLASH_ACR_FLUSH;
    if (FLASH->CR & FLASH_CR_LOCK) {
        FLASH->KEYR = FLASH_FKEY1;
        FLASH->KEYR = FLASH_FKEY2;
    }
}

static void flash_erase_write_end(void)
{
    FLASH->CR |= FLASH_CR_LOCK;
    FLASH->ACR &= ~FLASH_ACR_FLUSH;
    FLASH->ACR |= FLASH_ACR_CACHENA;
}

uint32_t vsfhal_flash_write(enum flash_idx_t idx, uint32_t addr, uint32_t size, uint8_t *buff)
{
    if (idx == FLASH_INVALID_IDX)
        return size;
    VSF_HAL_ASSERT(idx < FLASH_IDX_NUM);

    vsf_gint_state_t vsf_gint_state = vsf_get_interrupt();
    vsf_disable_interrupt();
    
    uint32_t i, cr = (addr >= 0x08008000) ? (FLASH_CR_NVR | FLASH_CR_PG) : FLASH_CR_PG;

    addr = get_logic_addr(addr);

    volatile uint32_t *w = (uint32_t *)addr;

    flash_erase_write_start();

    //FLASH->SR |= FLASH_SR_EOP;
    FLASH->CR = cr;

    for (i = 0; i < size; i += 4) {
        if (cr & FLASH_CR_NVR)
            FLASH->CR = cr;
        while (FLASH->SR & FLASH_SR_BUSY);
        *w++ = get_unaligned_le32(buff + i);
    }

    //while (!(FLASH->SR & FLASH_SR_EOP));
    //FLASH->SR |= FLASH_SR_EOP;
    while(FLASH->SR & FLASH_SR_BUSY);

    FLASH->CR &= ~FLASH_CR_PG;

    flash_erase_write_end();
    
    vsf_set_interrupt(vsf_gint_state);

    return VSF_ERR_NONE;
}

uint32_t vsfhal_flash_erase(enum flash_idx_t idx, uint32_t addr, uint32_t size)
{
    if (idx == FLASH_INVALID_IDX)
        return size;
    VSF_HAL_ASSERT(idx < FLASH_IDX_NUM);

    vsf_gint_state_t vsf_gint_state = vsf_get_interrupt();
    vsf_disable_interrupt();

    RCC->ANACTRL1 |= RCC_ANACTRL1_512B_MODE;
    
    uint32_t i = 0;
    uint32_t erase = (addr & 0x7fff) >> 9;
    uint32_t cr = (addr >= 0x08008000) ? (FLASH_CR_NVR | FLASH_CR_SER | FLASH_CR_START) : (FLASH_CR_SER | FLASH_CR_START);
    size = size / 512;
    
    flash_erase_write_start();

    do {
        //FLASH->SR |= FLASH_SR_EOE;
        while(FLASH->SR & FLASH_SR_BUSY);

        FLASH->ERASE = erase + i;
        FLASH->CR = cr;

        //while (!(FLASH->SR & FLASH_SR_EOE));
        //FLASH->SR |= FLASH_SR_EOE;
        while(FLASH->SR & FLASH_SR_BUSY);
    } while (++i < size);

    FLASH->CR &= ~FLASH_CR_SER;

    flash_erase_write_end();

    RCC->ANACTRL1 &= ~RCC_ANACTRL1_512B_MODE;

    vsf_set_interrupt(vsf_gint_state);

    return size;
}

#endif
