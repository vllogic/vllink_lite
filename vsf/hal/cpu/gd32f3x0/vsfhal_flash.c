#include "vsf.h"

#define GD32F3X0_FLASH_BASEADDR				0x08000000
#define GD32F3X0_FLASH_ADDR(addr)			(GD32F3X0_FLASH_BASEADDR + (addr))
#define GD32F3X0_FLASH_SIZE_KB				(*(uint16_t *)0x1FFFF7E0)

#define VSFHAL_FLASH_NUM					1

struct vsfhal_flash_t
{
	struct
	{
		void *param;
		void (*onfinish)(void*, vsf_err_t);
	} cb;
} static vsfhal_flash[VSFHAL_FLASH_NUM];

static vsf_err_t vsfhal_flash_checkidx(uint8_t index)
{
	return (index < VSFHAL_FLASH_NUM) ? VSFERR_NONE : VSFERR_NOT_SUPPORT;
}

vsf_err_t vsfhal_flash_capacity(uint8_t index, uint32_t *pagesize, 
		uint32_t *pagenum)
{
	uint16_t flash_size;

	switch (index)
	{
	case 0:
		flash_size = GD32F3X0_FLASH_SIZE_KB;
		if (NULL != pagesize)
		{
			*pagesize = 1024;
		}
		if (NULL != pagenum)
		{
			*pagenum = flash_size;
		}
		return VSFERR_NONE;
	default:
		return VSFERR_NOT_SUPPORT;
	}
}

uint32_t vsfhal_flash_baseaddr(uint8_t index)
{
	return GD32F3X0_FLASH_BASEADDR;
}

// op -- operation: 0(ERASE), 1(READ), 2(WRITE)
uint32_t vsfhal_flash_blocksize(uint8_t index, uint32_t addr, uint32_t size,
		int op)
{
	uint32_t pagesize;
	if (vsfhal_flash_capacity(index, &pagesize, NULL))
		return 0;
	return !op ? pagesize : 4;
}

vsf_err_t vsfhal_flash_config_cb(uint8_t index, int32_t int_priority,
		void *param, void (*onfinish)(void*, vsf_err_t))
{
	if (vsfhal_flash_checkidx(index))
		return VSFERR_NOT_SUPPORT;

	vsfhal_flash[index].cb.param = param;
	vsfhal_flash[index].cb.onfinish = onfinish;
	if (onfinish != NULL)
	{
		NVIC_SetPriority(FMC_IRQn, int_priority);
		NVIC_EnableIRQ(FMC_IRQn);
	}
	else
	{
		NVIC_DisableIRQ(FMC_IRQn);
	}
	return VSFERR_NONE;
}

void vsfhal_flash_security_set(uint8_t index, uint8_t l0_h1)
{
	uint16_t rdp = OB_SPC;
	
	if (rdp == 0x33cc)
		return;

	if (l0_h1 == 0)
	{
		if (rdp != 0x5aa5)
			return;
	}

	if (FMC_CTL & FMC_CTL_LK)
	{
		FMC_KEY = UNLOCK_KEY0;
		FMC_KEY = UNLOCK_KEY1;
	}
	
	if (!(FMC_CTL & FMC_CTL_OBWEN))
	{
		FMC_OBKEY = UNLOCK_KEY0;
		FMC_OBKEY = UNLOCK_KEY1;
	}
	
	FMC_CTL |= FMC_CTL_OBER;
	FMC_CTL |= FMC_CTL_START;
	while (FMC_STAT & FMC_STAT_BUSY);
	
	if (l0_h1)
	{
		FMC_CTL &= ~FMC_CTL_OBER;
		FMC_CTL |= FMC_CTL_OBPG;
		OB_SPC = 0x33cc;
		while (FMC_STAT & FMC_STAT_BUSY);
	}

	FMC_CTL = FMC_CTL_LK;
}

vsf_err_t vsfhal_flash_init(uint8_t index)
{
	uint32_t size, pagenum;
	if (vsfhal_flash_capacity(index, &size, &pagenum))
		return VSFERR_FAIL;

	memset(vsfhal_flash, 0, sizeof(struct vsfhal_flash_t));
	
	FMC_KEY = UNLOCK_KEY0;
	FMC_KEY = UNLOCK_KEY1;
	FMC_STAT |= FMC_STAT_BUSY | FMC_STAT_PGERR | FMC_STAT_WPERR | FMC_STAT_ENDF;
	return VSFERR_NONE;
}

vsf_err_t vsfhal_flash_fini(uint8_t index)
{
	if (vsfhal_flash_checkidx(index))
		return VSFERR_NOT_SUPPORT;
	
	FMC_CTL |= FMC_CTL_LK;
	return VSFERR_NONE;
}

vsf_err_t vsfhal_flash_erase(uint8_t index, uint32_t addr)
{
	switch (index)
	{
	case 0:
		if (FMC_STAT & FMC_STAT_BUSY)
			return VSFERR_NOT_READY;
		
		FMC_ADDR = GD32F3X0_FLASH_ADDR(addr);
		FMC_CTL |= FMC_CTL_PER | FMC_CTL_ENDIE;
		FMC_CTL |= FMC_CTL_START;
		break;
	default:
		return VSFERR_NOT_SUPPORT;
	}
	
	if (!vsfhal_flash[index].cb.onfinish)
	{
		while (FMC_STAT & FMC_STAT_BUSY);
		FMC_CTL &= ~(FMC_CTL_PER | FMC_CTL_ENDIE);
	}
	return VSFERR_NONE;
}

vsf_err_t vsfhal_flash_read(uint8_t index, uint32_t addr, uint8_t *buff)
{
	return VSFERR_NOT_SUPPORT;
}

vsf_err_t vsfhal_flash_write(uint8_t index, uint32_t addr, uint8_t *buff)
{
	switch (index)
	{
	case 0:
		if (FMC_STAT & FMC_STAT_BUSY)
			return VSFERR_NOT_READY;
		
		FMC_CTL |= FMC_CTL_PG | FMC_CTL_ENDIE;
		*(uint32_t *)GD32F3X0_FLASH_ADDR(addr) =  *(uint32_t *)buff;
		break;
	default:
		return VSFERR_NOT_SUPPORT;
	}
	
	if (!vsfhal_flash[index].cb.onfinish)
	{
		while (FMC_STAT & FMC_STAT_BUSY);
		FMC_CTL &= ~(FMC_CTL_PG | FMC_CTL_ENDIE);
	}
	return VSFERR_NONE;
}

void FMC_IRQHandler(void)
{
	FMC_CTL &= ~(FMC_CTL_PER | FMC_CTL_PG | FMC_CTL_ENDIE);
	if (vsfhal_flash[0].cb.onfinish != NULL)
	{
		vsfhal_flash[0].cb.onfinish(vsfhal_flash[0].cb.param,
				(FMC_STAT & FMC_STAT_WPERR) ? VSFERR_FAIL : VSFERR_NONE);
	}
}
