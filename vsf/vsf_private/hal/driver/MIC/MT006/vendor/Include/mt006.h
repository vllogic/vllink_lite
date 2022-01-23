/**
  ******************************************************************************
  * @file    mt006.h
  * @author  MIC Software Team
  * @version V1.0.0
  * @date    09/01/2020
  * @brief   CMSIS Cortex-M0 Device Peripheral Access Layer Header File. 
  *          This file contains all the peripheral register's definitions, bits 
  *          definitions and memory mapping for MT006 devices.  
  *          
  *          The file is the unique include file that the application programmer
  *          is using in the C source code, usually in main.c. This file contains:
  *           - Configuration section that allows to select:
  *              - The device used in the target application
  *              - To use or not the peripheral drivers in application code(i.e. 
  *                code will be based on direct access to peripheral registers 
  *                rather than drivers API), this option is controlled by 
  *                "#define USE_STDPERIPH_DRIVER"
  *              - To change few application-specific parameters such as the HSE 
  *                crystal frequency
  *           - Data structures and the address mapping for all peripherals
  *           - Peripheral's registers declarations and bits definition
  *           - Macros to access peripheral registers hardware
  *
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, MIC SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT MIC</center></h2>
  ******************************************************************************
  */

/** @addtogroup CMSIS
  * @{
  */

/** @addtogroup MIC
  * @{
  */

#ifndef __MT006_H
#define __MT006_H

#ifdef __cplusplus
 extern "C" {
#endif 

/** @addtogroup Library_configuration_section
  * @{
  */

#if !defined  USE_STDPERIPH_DRIVER
/**
 * @brief Comment the line below if you will not use the peripherals drivers.
   In this case, these drivers will not be included and the application code will 
   be based on direct access to peripherals registers 
   */
  /*#define USE_STDPERIPH_DRIVER*/
#endif /* USE_STDPERIPH_DRIVER */

/**
 * @brief MT006 Standard Peripheral Library version number V1.0.0
   */
#define __MT006_STDPERIPH_VERSION_MAIN   (0x01) /*!< [31:24] main version */
#define __MT006_STDPERIPH_VERSION_SUB1   (0x00) /*!< [23:16] sub1 version */
#define __MT006_STDPERIPH_VERSION_SUB2   (0x00) /*!< [15:8]  sub2 version */
#define __MT006_STDPERIPH_VERSION_RC     (0x00) /*!< [7:0]  release candidate */ 
#define __MT006_STDPERIPH_VERSION        ((__MT006_STDPERIPH_VERSION_MAIN << 24)\
                                         |(__MT006_STDPERIPH_VERSION_SUB1 << 16)\
                                         |(__MT006_STDPERIPH_VERSION_SUB2 << 8)\
                                         |(__MT006_STDPERIPH_VERSION_RC))
/**
  * @}
  */

/** @addtogroup Configuration_section_for_CMSIS
  * @{
  */

/**
 * @brief MT006 Interrupt Number Definition, according to the selected device 
 *        in @ref Library_configuration_section 
 */
#define __CM0_REV                 0 /*!< Core Revision r0p0                            */
#define __MPU_PRESENT             0 /*!< MIC do not provide MPU                  */
#define __NVIC_PRIO_BITS          2 /*!< MIC uses 2 Bits for the Priority Levels */
#define __Vendor_SysTickConfig    0 /*!< Set to 1 if different SysTick Config is used  */

/*!< Interrupt Number Definition */
typedef enum IRQn
{
	/******  Cortex-M0 Processor Exceptions Numbers *****************************************************/
	NonMaskableInt_IRQn         = -14,    /*!< 2 Non Maskable Interrupt                                 */
	HardFault_IRQn              = -13,    /*!< 3 Cortex-M0 Hard Fault Interrupt                         */
	SVC_IRQn                    = -5,     /*!< 11 Cortex-M0 SV Call Interrupt                           */
	PendSV_IRQn                 = -2,     /*!< 14 Cortex-M0 Pend SV Interrupt                           */
	SysTick_IRQn                = -1,     /*!< 15 Cortex-M0 System Tick Interrupt                       */

	/******  MT006 specific Interrupt Numbers **********************************************************/
	FLASH_IRQn                  = 0,      /*!< FLASH Interrupts                                         */
	UART0_IRQn                  = 1,      /*!< UART0 Interrupts                                         */
	I2C0_IRQn                   = 2,      /*!< I2C0 Interrupts                                          */
	SPI0_IRQn                   = 3,      /*!< SPI0 Interrupt                                           */
	TIMER0_IRQn                 = 4,      /*!< TIMER0 Interrupt                                         */
	MCPWM_IRQn                  = 5,      /*!< MCPWM Interrupts                                         */
	GPIO_IRQn                   = 6,      /*!< GPIO Interrupts                                          */
	ADC_IRQn                    = 7,      /*!< ADC Interrupts                                           */
	BOD_IRQn                    = 8,      /*!< BOD Interrupt                                            */
	BOR_IRQn                    = 9,      /*!< BOR Interrupt                                            */
	TSC_IRQn                    = 10,     /*!< TSC Interrupt                                            */
    CRS_IRQn                    = 11,     /*!< CRS Interrupt                                          */
	ACMP_IRQn                   = 12,     /*!< ACMP Interrupt                                           */
	USB_IRQn                    = 13,     /*!< USB Interrupt                                         */
	STIMER_IRQn                 = 14,     /*!< SIMPLE TIMER Interrupt                                   */
	CAN_IRQn                    = 15,     /*!< CAN Interrupt                                   */
} IRQn_Type;

/**
  * @}
  */

#include "core_cm0.h"
//#include "system_mt006.h"
#include <stdint.h>

/** @addtogroup Exported_types
  * @{
  */  

typedef enum {RESET = 0, SET = !RESET} FlagStatus, ITStatus;

typedef enum {DISABLE = 0, ENABLE = !DISABLE} FunctionalState;
#define IS_FUNCTIONAL_STATE(STATE) (((STATE) == DISABLE) || ((STATE) == ENABLE))

typedef enum {ERROR = 0, SUCCESS = !ERROR} ErrorStatus;

typedef unsigned char   u8;
typedef unsigned char   byte;
typedef unsigned short  u16;
typedef unsigned long   u32;
typedef signed char     s8;
typedef signed short    s16;
typedef signed long     s32;

/** @addtogroup Peripheral_registers_structures
  * @{
  */   

/** 
  * @brief Reset and Clock Control
  */
typedef struct
{
	__IO uint32_t PRESETCTRL0;      //0x00
	__IO uint32_t PRESETCTRL0_SET;
	__IO uint32_t PRESETCTRL0_CLR;
	__IO uint32_t PRESETCTRL0_TOG;
	__IO uint32_t AHBCLKCTRL0;      //0x10
	__IO uint32_t AHBCLKCTRL0_SET;
	__IO uint32_t AHBCLKCTRL0_CLR;
	__IO uint32_t AHBCLKCTRL0_TOG;
	__IO uint32_t BOOTREMAP;        //0x20
	__IO uint32_t REMAPFLASH;       //0x24
	__IO uint32_t OSC12_CTRL;       //0x28
	__IO uint32_t IRC10_CTRL;       //0x2C
	__IO uint32_t SYSPLLCTRL;       //0x30
	__IO uint32_t SYSPLLSTAT;       //0x34
	__IO uint32_t LDOCTRL;          //0x38
    #if 0
    uint32_t RESERVED0;
    #else
    __IO uint32_t USBCTRL;
    #endif
	__IO uint32_t SYSRSTSTAT;       //0x40
		 uint32_t RESERVED1[3];
	__IO uint32_t MAINCLKSEL;       //0x50
	__IO uint32_t MAINCLKUEN;       //0x54
	__IO uint32_t UARTCLKSEL;       //0x58
	__IO uint32_t UARTCLKUEN;       //0x5C
		 uint32_t RESERVED2[4];     //
    __IO uint32_t OUTCLKSEL;        //0x70
	__IO uint32_t OUTCLKUEN;        //0x74
         uint32_t RESERVED3[2];
	__IO uint32_t SYSAHBCLKDIV;     //0x80
		 uint32_t RESERVED4[3];
	__IO uint32_t UART0CLKDIV;      //0x90
		 uint32_t RESERVED5[3];
	__IO uint32_t SYSTICKCLKDIV;    //0xa0
	__IO uint32_t SPI0CLKDIV;       //0xa4
         uint32_t RESERVED6;
	__IO uint32_t OUTCLKDIV;        //0xac
	__IO uint32_t ADCCLKDIV;        //0xb0
         uint32_t RESERVED70[3];    
	__IO uint32_t USBCLKDIV;        //0xc0
         uint32_t RESERVED71[7];    
	__IO uint32_t WAKEUPEN;         //0xe0
         uint32_t RESERVED8[15];
	__IO uint32_t SYSTICKCAL;       //0x120
		 uint32_t RESERVED9[3];
	__IO uint32_t BORCTRL;          //0x130
		 uint32_t RESERVED10[51];
	__IO uint32_t PDSLEEPCFG;       //0x200
	__IO uint32_t PDAWAKECFG;       //0x204
	__IO uint32_t PDRUNCFG;         //0x208
         uint32_t RESERVED11[5];
	__IO uint32_t DEVICEID;         //0x220
         uint32_t RESERVED12[3];
	__IO uint32_t DEVICEID1;        //0x230
         uint32_t RESERVED13[19];
	__IO uint32_t PCON;             //0x280
         uint32_t RESERVED14[3];
	__IO uint32_t DBGCTRL;          //0x290
         uint32_t RESERVED15[27];
	__IO uint32_t ANACTRL1;         //0x300
         uint32_t RESERVED16[3];
	__IO uint32_t DEEPSLEEPCTRL;    //0x310
} RCC_TypeDef;

/** 
  * @brief ACMP and OP
  */
typedef struct
{
	__IO uint32_t CTRL0;            //0x00
         uint32_t RESERVED0[3];
	__IO uint32_t CTRL1;		    //0x10
         uint32_t RESERVED1[3];
         uint32_t RESERVED2[4];
	__IO uint32_t STATUS;		    //0x30
         uint32_t RESERVED3[3];
	__IO uint32_t OPA_CTRL;		    //0x40
         uint32_t RESERVED4[3];
	__IO uint32_t ADV_CTRL0;	    //0x50
         uint32_t RESERVED5[3];
	__IO uint32_t ADV_CTRL1;        //0x60
         uint32_t RESERVED6[3];
	__IO uint32_t ADV_CTRL2;        //0x70
         uint32_t RESERVED7[3];
	__IO uint32_t ADV_CTRL3;        //0x80
         uint32_t RESERVED8[3];
	__IO uint32_t ADV_CTRL4;        //0x90
         uint32_t RESERVED9[3];
	__IO uint32_t CMPCLKDIV;        //0xA0
         uint32_t RESERVED10[3];
} ACMP_TypeDef;

/**
  * @brief ADC Interface
  */
typedef struct
{
    __IO uint32_t ISR;              //0x00
    __IO uint32_t IER;              //0x04
    __IO uint32_t CR;               //0x08
    __IO uint32_t CFGR1;            //0x0C
    __IO uint32_t CFGR2;            //0x10
         uint32_t RESERVED0[3];
    __IO uint32_t TR;               //0x20
         uint32_t RESERVED1;
    __IO uint32_t CHSELR;           //0x28
         uint32_t RESERVED2;
    __IO uint32_t MISR;             //0x30
         uint32_t RESERVED3[3];
    __IO uint32_t DR;               //0x40
         uint32_t RESERVED4[177];
    __IO uint32_t CCR;              //0x308
         uint32_t RESERVED5[5];
    __IO uint32_t VREF;             //0x320
         uint32_t RESERVED6[55];
    __IO uint32_t ACR0;             //0x400
         uint32_t RESERVED7[3];
    __IO uint32_t DR0;              //0x410
    __IO uint32_t DR1;              //0x414
    __IO uint32_t DR2;              //0x418
    __IO uint32_t DR3;              //0x41C
    __IO uint32_t DR4;              //0x420
    __IO uint32_t DR5;              //0x424
    __IO uint32_t DR6;              //0x428
    __IO uint32_t DR7;              //0x42C
    __IO uint32_t DR8;              //0x430
    __IO uint32_t DR9;              //0x434
    __IO uint32_t DR10;             //0x438
    __IO uint32_t DR11;             //0x43C
    __IO uint32_t DR12;             //0x440
    __IO uint32_t DR13;             //0x444
    __IO uint32_t DR14;             //0x448
    __IO uint32_t DR15;             //0x44C
         uint32_t RESERVED8[4];
    __IO uint32_t ACR1;             //0x460
         uint32_t RESERVED9[3];
    __IO uint32_t ACR2;             //0x470
         uint32_t RESERVED10[3];
    __IO uint32_t DELAY_CTRL;       //0x480
         uint32_t RESERVED11[3];
    __IO uint32_t STATUS;           //0x490
}ADC_TypeDef;

/** 
  * @brief FLASH Memory Control
  */
typedef struct
{
	__IO uint32_t ACR;		        //0x00
	__IO uint32_t KEYR;		        //0x04
	__IO uint32_t ERASE;	        //0x08
	__IO uint32_t SR;		        //0x0C
	__IO uint32_t CR;		        //0x10
	__IO uint32_t RESERVE0[5];
	__IO uint32_t INFO0;            //0x28
	__IO uint32_t INFO1;            //0x3C
	__IO uint32_t INFO2;            //0x30
	__IO uint32_t INFO3;            //0x34
	__I  uint32_t ID0;	            //0x38
	__I  uint32_t ID1;              //0x3C
	__I  uint32_t ID2;              //0x40
	__I  uint32_t ID3;              //0x44
	__IO uint32_t RESERVE1[2];
	__IO uint32_t TRIM;		        //0x50
} FMC_TypeDef;

/** 
  * @brief General Purpose I/O
  */
typedef struct
{
    __IO uint32_t DT;               //0x00
    __IO uint32_t DT_SET;           //0x04
    __IO uint32_t DT_CLR;           //0x08
    __IO uint32_t DT_TOG;           //0x0C
} GPIO_DATA_TypeDef;

typedef struct
{
    __IO uint32_t FILTER;           //0x00
} GPIO_FILTER_TypeDef;

typedef struct
{
    __IO uint32_t BSRR_LOW;         //0x00
    __IO uint32_t BSRR_HIGH;        //0x04
} GPIO_BSRR_TypeDef;

typedef struct
{
    __IO uint32_t DIR;              //0x00
    __IO uint32_t DIR_SET;
    __IO uint32_t DIR_CLR;
    __IO uint32_t DIR_TOG;
    __IO uint32_t IS;               //0x10
    __IO uint32_t IS_SET;
    __IO uint32_t IS_CLR;
    __IO uint32_t IS_TOG;
    __IO uint32_t IBE;              //0x20
    __IO uint32_t IBE_SET;
    __IO uint32_t IBE_CLR;
    __IO uint32_t IBE_TOG;
    __IO uint32_t IEV;              //0x30
    __IO uint32_t IEV_SET;
    __IO uint32_t IEV_CLR;
    __IO uint32_t IEV_TOG;
    __IO uint32_t IE;               //0x40
    __IO uint32_t IE_SET;
    __IO uint32_t IE_CLR;
    __IO uint32_t IE_TOG;
    __IO uint32_t IRS;              //0x50
    __IO uint32_t IRS_SET;
    __IO uint32_t IRS_CLR;
    __IO uint32_t IRS_TOG;
    __IO uint32_t MIS;              //0x60
    __IO uint32_t MIS_SET;
    __IO uint32_t MIS_CLR;
    __IO uint32_t MIS_TOG;
    __IO uint32_t IC;               //0x70
    __IO uint32_t IC_SET;
    __IO uint32_t IC_CLR;
    __IO uint32_t IC_TOG;
    __IO uint32_t DATAMASK;         //0x80
    __IO uint32_t DATAMASK_SET;
    __IO uint32_t DATAMASK_CLR;
    __IO uint32_t DATAMASK_TOG;
} GPIO_BANK_TypeDef;

typedef struct
{				
    __IO uint32_t CON;              //0x00
}IOCON_TypeDef;

typedef struct
{				
    __IO uint32_t GPIO0_0;
    __IO uint32_t GPIO0_1;
    __IO uint32_t GPIO0_2;
    __IO uint32_t GPIO0_3;
    __IO uint32_t GPIO0_4;
    __IO uint32_t GPIO0_5;
    __IO uint32_t GPIO0_6;
    __IO uint32_t GPIO0_7;
    __IO uint32_t GPIO1_0;
    __IO uint32_t GPIO1_1;
    __IO uint32_t GPIO1_2;
    __IO uint32_t GPIO1_3;
    __IO uint32_t GPIO1_4;
    __IO uint32_t GPIO1_5;
    __IO uint32_t GPIO1_6;
    __IO uint32_t GPIO1_7;
    __IO uint32_t GPIO2_0;
    __IO uint32_t GPIO2_1;
    __IO uint32_t GPIO2_2;
    __IO uint32_t GPIO2_3;
    __IO uint32_t GPIO2_4;
    __IO uint32_t GPIO2_5;
    __IO uint32_t GPIO2_6;
    __IO uint32_t GPIO2_7;
}GPIO_IOCON_TypeDef;

/**
  * @brief MI2C
  */
typedef struct
{
    __IO uint32_t ADDR;         //0x00
	__IO uint32_t DATA;         //0x04
	__IO uint32_t CNTR;         //0x08
    union {
        __IO uint32_t STAT;     //0xC
        __IO uint32_t CCR;      //0xC
    }REG03;
    __IO uint32_t XADDR;        //0x10
    __IO uint32_t RESERVED0;    //0x14
    __IO uint32_t RESERVED1;    //0x18
	__IO uint32_t SRST;       	//0x1C
}I2C_TypeDef;

/** 
  * @brief Independent WATCHDOG
  */
typedef struct
{
    __IO uint32_t KR;               //0x00
    __IO uint32_t PR;               //0x04
    __IO uint32_t RLR;              //0x08
    __IO uint32_t SR;               //0x0C
    __IO uint32_t WINR;             //0x10
} IWDG_TypeDef;

/** 
  * @brief Window WATCHDOG
  */
typedef struct
{
    __IO uint32_t CR;               //0x00
    __IO uint32_t CFR;              //0x04
    __IO uint32_t SR;               //0x08
} WWDG_TypeDef;

/** 
  * @brief MCPWM
  */
typedef struct
{
    __IO uint32_t CON;              //0x00
    __IO uint32_t CON_SET;          //0x04
    __IO uint32_t CON_CLR;          //0x08
    __IO uint32_t CAPCON;           //0x0C
    __IO uint32_t CAPCON_SET;       //0x10
    __IO uint32_t CAPCON_CLR;       //0x14
    __IO uint32_t TC0;              //0x18
    __IO uint32_t TC1;              //0x1C
    __IO uint32_t TC2;              //0x20
    __IO uint32_t LIM0;             //0x24
    __IO uint32_t LIM1;             //0x28
    __IO uint32_t LIM2;             //0x2C
    __IO uint32_t MAT0;             //0x30
    __IO uint32_t MAT1;             //0x34
    __IO uint32_t MAT2;             //0x38
    __IO uint32_t DT;               //0x3C
    __IO uint32_t CCP;              //0x40
    __IO uint32_t CAP0;             //0x44
    __IO uint32_t CAP1;             //0x48
    __IO uint32_t CAP2;             //0x4C
    __IO uint32_t INTEN;            //0x50
    __IO uint32_t INTEN_SET;        //0x54
    __IO uint32_t INTEN_CLR;        //0x58
    __IO uint32_t CNTCON;           //0x5C
    __IO uint32_t CNTCON_SET;       //0x60
    __IO uint32_t CNTCON_CLR;       //0x64
    __IO uint32_t INTF;             //0x68
    __IO uint32_t INTF_SET;         //0x6C
    __IO uint32_t INTF_CLR;         //0x70
    __IO uint32_t CAP_CLR;          //0x74
    __IO uint32_t HALL;             //0x78
    __IO uint32_t HALLS;            //0x7C
    __IO uint32_t HALL_VEL_CMP;     //0x80
    __IO uint32_t HALL_VEL_VAL;     //0x84
    __IO uint32_t HALL_VEL_TH;      //0x88
    __IO uint32_t HALL_VEL_MCIST;   //0x8C
    __IO uint32_t PR;               //0x90
    __IO uint32_t CHGPH;            //0x94
    __IO uint32_t HALLAEN;          //0x98
    __IO uint32_t HALLA;            //0x9C
    __IO uint32_t ADC_TRIG0;        //0xA0
    __IO uint32_t ADC_TRIG1;        //0xA4
    __IO uint32_t ADC_TRIG2;        //0xA8
} MCPWM_TypeDef;

/**
  * @brief SPI
  */
typedef struct
{
    __IO uint32_t CR0;	            //0x00
    __IO uint32_t CR1;	            //0x04
    __IO uint32_t DR;	            //0x08
    __IO uint32_t SR;	            //0x0c
    __IO uint32_t CPSR;	            //0x10
    __IO uint32_t IMSC;	            //0x14
    __IO uint32_t RIS;	            //0x18
    __IO uint32_t MIS;	            //0x1c
    __IO uint32_t ICR;	            //0x20
    __IO uint32_t DMACR;	        //0x24
} SPI_TypeDef;

/** 
  * @brief TIMER
  */
typedef struct
{
    __IO uint32_t IR;               //0x000
         uint32_t RESERVED0[3];
    __IO uint32_t TCR;              //0x010
         uint32_t RESERVED1[3];
    __IO uint32_t DIR;              //0x020
         uint32_t RESERVED2[3];
    __IO uint32_t TC0;              //0x030
         uint32_t RESERVED3[3];
    __IO uint32_t TC1;              //0x040
         uint32_t RESERVED4[3];
    __IO uint32_t TC2;              //0x050
         uint32_t RESERVED5[3];
    __IO uint32_t TC3;              //0x060
         uint32_t RESERVED6[3];
    __IO uint32_t PR;               //0x070
         uint32_t RESERVED7[3];
    __IO uint32_t PC;               //0x080
         uint32_t RESERVED8[3];
    __IO uint32_t MCR;              //0x090
         uint32_t RESERVED9[3];
    __IO uint32_t MR0;              //0x0A0
         uint32_t RESERVED10[3];
    __IO uint32_t MR1;              //0x0B0
         uint32_t RESERVED11[3];
    __IO uint32_t MR2;              //0x0C0
         uint32_t RESERVED12[3];
    __IO uint32_t MR3;              //0x0D0
         uint32_t RESERVED13[3];
    __IO uint32_t CCR0;              //0x0E0
         uint32_t RESERVED14[3];
    __IO uint32_t CR0;              //0x0F0
         uint32_t RESERVED15[3];
    __IO uint32_t CR1;              //0x100
         uint32_t RESERVED16[3];
    __IO uint32_t CR2;              //0x110
         uint32_t RESERVED17[3];
    __IO uint32_t CR3;              //0x120
         uint32_t RESERVED18[3];
    __IO uint32_t EMR;              //0x130
         uint32_t RESERVED19[3];
    __IO uint32_t PWMTH0;           //0x140
         uint32_t RESERVED20[3];
    __IO uint32_t PWMTH1;           //0x150
         uint32_t RESERVED21[3];
    __IO uint32_t PWMTH2;           //0x160
         uint32_t RESERVED22[3];
    __IO uint32_t PWMTH3;           //0x170
         uint32_t RESERVED23[3];
    __IO uint32_t CTCR;             //0x180
         uint32_t RESERVED24[3];
    __IO uint32_t PWMC;             //0x190
         uint32_t RESERVED25[3];
    __IO uint32_t PWMV0;            //0x1A0
         uint32_t RESERVED26[3];
    __IO uint32_t PWMV1;            //0x1B0
         uint32_t RESERVED27[3];
    __IO uint32_t PWMV2;            //0x1C0
         uint32_t RESERVED28[3];
    __IO uint32_t PWMV3;            //0x1D0
         uint32_t RESERVED29[11];
    __IO uint32_t CCR1;             //0x200
         uint32_t RESERVED30[7];
    __IO uint32_t PERIOD0H;         //0x220
         uint32_t RESERVED31[3];
    __IO uint32_t PERIOD0L;         //0x230
         uint32_t RESERVED32[3];
    __IO uint32_t PERIOD1H;         //0x240
         uint32_t RESERVED33[3];
    __IO uint32_t PERIOD1L;         //0x250
         uint32_t RESERVED34[3];
    __IO uint32_t PERIOD2H;         //0x260
         uint32_t RESERVED35[3];
    __IO uint32_t PERIOD2L;         //0x270
         uint32_t RESERVED36[3];
    __IO uint32_t PERIOD3H;         //0x280
         uint32_t RESERVED37[3];
    __IO uint32_t PERIOD3L;         //0x290
} TIMER_TypeDef;

/** 
  * @brief TSC
  */
typedef struct
{
    __IO uint32_t CR;               //0x00
    __IO uint32_t ENABLE;           //0x04
    __IO uint32_t IER;              //0x08
    __IO uint32_t ICR;              //0x0C
    __IO uint32_t ISR;              //0x10
    __IO uint32_t STAT;             //0x14
    __IO uint32_t OVERFLOW;         //0x18
    __IO uint32_t RANGE;            //0x1C
         uint32_t RESERVED0[4];     //
    __IO uint32_t IOCCR0;           //0x30
         uint32_t RESERVED1[3];     //
    __IO uint32_t IOCCR1;           //0x40
         uint32_t RESERVED2[3];     //
    __IO uint32_t IOCCR2;           //0x50
         uint32_t RESERVED3[3];     //
    __IO uint32_t IOCCR3;           //0x60
         uint32_t RESERVED4[3];     //
    __IO uint32_t IOCCR4;           //0x70
         uint32_t RESERVED5[3];     //
    __IO uint32_t IOCCR5;           //0x80
         uint32_t RESERVED6[3];     //
    __IO uint32_t IOCCR6;           //0x90
         uint32_t RESERVED7[3];     //
    __IO uint32_t IOCCR7;           //0xA0
         uint32_t RESERVED8[3];     //
    __IO uint32_t IOCCR8;           //0xB0
         uint32_t RESERVED9[3];     //
    __IO uint32_t IOCCR9;           //0xC0
         uint32_t RESERVED10[3];    //
    __IO uint32_t IOCCR10;          //0xD0
         uint32_t RESERVED11[3];    //
} TSC_TypeDef;

/** 
  * @brief UART
  */
typedef struct
{
    __IO uint32_t DR;       //0x00
    union {
        __IO uint32_t RSR;  //0x04
        __IO uint32_t ECR;  //0x04
    }UART04;
         uint32_t RESERVED0[4];//0x08-0x14
    __IO uint32_t TFR;      //0x18
         uint32_t RESERVED1;//0x1C
    __IO uint32_t ILPR;     //0x20
    __IO uint32_t IBRD;     //0x24
    __IO uint32_t FBRD;     //0x28
    __IO uint32_t LCRH;     //0x2C
    __IO uint32_t CR;       //0x30
    __IO uint32_t IFLS;     //0x34
    __IO uint32_t IMSC;     //0x38
    __IO uint32_t RIS;      //0x3C
    __IO uint32_t MIS;      //0x40
    __IO uint32_t ICR;      //0x44
         uint32_t RESERVED2[2];//
    __IO uint32_t IDCR;     //0x50
} UART_TypeDef;

/**
  * @}
  */

/** 
  * @brief Controller Area Network 
  */
typedef struct
{
  __IO uint8_t uused[13];
} CAN_RTxArea_TypeDef;

//TX
typedef struct
{
  __IO uint8_t Control;
  __IO uint8_t ID[2];
  __IO uint8_t DATA[8];
  __IO uint8_t uused[2];
} CAN_TxArea_SFF_TypeDef;

typedef struct
{
  __IO uint8_t Control;
  __IO uint8_t ID[4];
  __IO uint8_t DATA[8];
} CAN_TxArea_EFF_TypeDef;

//RX
typedef struct
{
  __IO uint8_t Control;
  __IO uint8_t ID[2];
  __IO uint8_t DATA[8];
  __IO uint8_t uused[2];
} CAN_RxArea_SFF_TypeDef;

typedef struct
{
  __IO uint8_t Control;
  __IO uint8_t ID[4];
  __IO uint8_t DATA[8];
} CAN_RxArea_EFF_TypeDef;

typedef struct
{
  __IO uint8_t MOD;
  __IO uint8_t CMR;
  __IO uint8_t SR;
  __IO uint8_t IR;
  __IO uint8_t IER;
  __IO uint8_t BTR0;
  __IO uint8_t BTR1;
  __IO uint8_t BTR2;
  __IO uint8_t OCR;
  __IO uint8_t unsed1;
  __IO uint8_t unsed2;
  __IO uint8_t ALC;
  __IO uint8_t ECC;
  __IO uint8_t EWLR;
  __IO uint8_t RXERR;
  __IO uint8_t TXERR;
  CAN_RTxArea_TypeDef	sRTx;
  __IO uint8_t RMC;
  __IO uint8_t RBSA;	
  __IO uint8_t CDR;
  __IO uint8_t RXFIFO[64];
  __IO uint8_t TXBUFR[13];
} CAN_TypeDef;
/**
  * @}
  */

/** 
  * @brief Simple-Timer
  */
typedef struct
{
    __IO uint32_t LOADCOUNT0;       //0x00
    __IO uint32_t CURRENTVALUE0;    //0x04
    __IO uint32_t CONTROLREG0;      //0x08
    __IO uint32_t EOI0;             //0x0C
    __IO uint32_t INTSTATUS0;       //0x10
    __IO uint32_t LOADCOUNT1;       //0x14
    __IO uint32_t CURRENTVALUE1;    //0x18
    __IO uint32_t CONTROLREG1;      //0x1C
    __IO uint32_t EOI1;             //0x20
    __IO uint32_t INTSTATUS1;       //0x24
    uint32_t RESERVED[30];          //0x28-0x9C
    __IO uint32_t INTSTATUS;        //0xA0
    __IO uint32_t EOI;              //0xA4
    __IO uint32_t RAWINTSTATUS;     //0xA8
} Simple_Timer_TypeDef;

/**
  * @}
  */

/** 
  * @brief CRS
  */
typedef struct
{
    __IO uint32_t CR;       //0x00
    __IO uint32_t CFGR;     //0x04
    __IO uint32_t ISR;      //0x08
    __IO uint32_t ICR;      //0x0C
} CRS_TypeDef;
/**
  * @}
  */

/** @addtogroup Peripheral_memory_map
* @{
*/
/*!< Peripheral memory map */
#define FLASH_BASE              ((uint32_t)0x08000000)      /*!< FLASH base address in the alias region */
#define FLASH_SIZE				((uint32_t)0x00008000)      /*!< FLASH size */
#define FLASH_INFO_BASE         ((uint32_t)0x1FFFF400)      /*!< FLASH base address in the alias region */
#define FLASH_INFO_SIZE         ((uint32_t)0x00000400)      /*!< FLASH size */
#define SRAM_BASE				((uint32_t)0x20000000)      /*!< SRAM base address in the alias region */
#define SRAM_SIZE				((uint32_t)0x00000800)      /*!< SRAM size */
#define PERIPH_BASE				((uint32_t)0x40000000)      /*!< Peripheral base address in the alias region */
#define SRAM_BB_BASE            ((uint32_t)0x22000000)      /*!< SRAM base address in the bit-band region */
#define PERIPH_BB_BASE          ((uint32_t)0x42000000)      /*!< Peripheral base address in the bit-band region */

#define APBPERIPH_BASE			(PERIPH_BASE)

#define RCC_BASE                (APBPERIPH_BASE)
#define IOCON_BASE              (APBPERIPH_BASE + 0x1000)
#define TSC_BASE                (APBPERIPH_BASE + 0x1800)
#define UART0_BASE              (APBPERIPH_BASE + 0x2000)
#define UART1_BASE              (APBPERIPH_BASE + 0x2800)
#define ACMP_BASE               (APBPERIPH_BASE + 0x3000)
#define I2C0_BASE               (APBPERIPH_BASE + 0x4000)
#define ADC_BASE                (APBPERIPH_BASE + 0x5000)
#define CAN_BASE				(APBPERIPH_BASE + 0x6000)
#define MCPWM_BASE              (APBPERIPH_BASE + 0x7000)
#define SPI0_BASE               (APBPERIPH_BASE + 0x8000)
#define TIMER0_BASE             (APBPERIPH_BASE + 0x9000)
#define TIMER1_BASE             (APBPERIPH_BASE + 0xA000)
#define FMC_BASE                (APBPERIPH_BASE + 0xB000)
#define WWDG_BASE               (APBPERIPH_BASE + 0xC000)
#define IWDG_BASE               (APBPERIPH_BASE + 0xC800)
#define SIMPLE_TIMER_BASE       (APBPERIPH_BASE + 0xD000)
#define CRS_BASE                (APBPERIPH_BASE + 0xF000)

#define GPIO_BASE               (APBPERIPH_BASE + 0x20000)
#define GPIO_DATA0_BASE         (GPIO_BASE)
#define GPIO_BSRR0_BASE         (GPIO_DATA0_BASE + 0x4000)
#define GPIO_BANK0_BASE         (GPIO_DATA0_BASE + 0x8000)
#define GPIO_FILTER_BASE        (GPIO_BASE + 0x0100)

/**
  * @}
  */

/** @addtogroup Peripheral_declaration
  * @{
  */  
#define RCC                     ((RCC_TypeDef *) RCC_BASE)
#define TSC                     ((TSC_TypeDef *) TSC_BASE)
#define UART0                   ((UART_TypeDef *) UART0_BASE)
#define I2C0                    ((I2C_TypeDef *) I2C0_BASE)
#define ADC                     ((ADC_TypeDef *) ADC_BASE)
#define CAN                		((CAN_TypeDef *) CAN_BASE)
#define ACMP                    ((ACMP_TypeDef *) ACMP_BASE)
#define MCPWM                   ((MCPWM_TypeDef *) MCPWM_BASE)
#define SPI0                    ((SPI_TypeDef *) SPI0_BASE)
#define TIMER0                  ((TIMER_TypeDef *) TIMER0_BASE)
#define TIMER1                  ((TIMER_TypeDef *) TIMER1_BASE)
#define FLASH                   ((FMC_TypeDef *) FMC_BASE)
#define WWDG                    ((WWDG_TypeDef *) WWDG_BASE)
#define IWDG                    ((IWDG_TypeDef *) IWDG_BASE)
#define ST                      ((Simple_Timer_TypeDef *) SIMPLE_TIMER_BASE)
#define CRS                     ((CRS_TypeDef *) CRS_BASE)

#define GPIODATA0               ((GPIO_DATA_TypeDef *) GPIO_DATA0_BASE)
#define GPIOBSRR0               ((GPIO_BSRR_TypeDef *) GPIO_BSRR0_BASE)
#define GPIOBANK0               ((GPIO_BANK_TypeDef *) GPIO_BANK0_BASE)
#define GPIOFILTER              ((GPIO_FILTER_TypeDef *) GPIO_FILTER_BASE)
#define GPIOIOCON               ((GPIO_IOCON_TypeDef *) IOCON_BASE)

/**
* @}
*/

#ifdef USE_STDPERIPH_DRIVER
  #include "mt006_conf.h"
#endif

/** @addtogroup Exported_constants
  * @{
  */

/** @addtogroup Peripheral_Registers_Bits_Definition
* @{
*/

/******************************************************************************/
/*                         Peripheral Registers Bits Definition               */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/*                                     RCC                                    */
/*                                                                            */
/******************************************************************************/
/*******************  Bit definition for RCC_PRESETCTRL register  *************/
#define RCC_PRESETCTRL_FLASH                ((uint32_t)0x00000001)          /*!< Reset FLASH */
#define RCC_PRESETCTRL_RAM                  ((uint32_t)0x00000002)          /*!< Reset RAM */
#define RCC_PRESETCTRL_GPIO                 ((uint32_t)0x00000004)          /*!< Reset GPIO */
#define RCC_PRESETCTRL_UART0                ((uint32_t)0x00000008)          /*!< Reset UART0 */
#define RCC_PRESETCTRL_I2C0                 ((uint32_t)0x00000010)          /*!< Reset I2C0 */
#define RCC_PRESETCTRL_ADC                  ((uint32_t)0x00000020)          /*!< Reset ADC */
#define RCC_PRESETCTRL_IOCON                ((uint32_t)0x00000040)          /*!< Reset IOCON */
#define RCC_PRESETCTRL_WWDG                 ((uint32_t)0x00000080)          /*!< Reset WWDG */
#define RCC_PRESETCTRL_MCPWM                ((uint32_t)0x00000100)          /*!< Reset MCPWM */
#define RCC_PRESETCTRL_SPI0                 ((uint32_t)0x00000200)          /*!< Reset SPI0 */
#define RCC_PRESETCTRL_TIMER0               ((uint32_t)0x00000400)          /*!< Reset TIMER0 */
#define RCC_PRESETCTRL_IWDG                 ((uint32_t)0x00001000)          /*!< Reset IWDG */
#define RCC_PRESETCTRL_TSC                  ((uint32_t)0x00002000)          /*!< Reset TSC */
#define RCC_PRESETCTRL_ACMP                 ((uint32_t)0x00008000)          /*!< Reset ACMP */
#define RCC_PRESETCTRL_STIMER               ((uint32_t)0x00020000)          /*!< Reset STIMER */
#define RCC_PRESETCTRL_CAN                  ((uint32_t)0x00040000)          /*!< Reset CAN */
#define RCC_PRESETCTRL_USB                  ((uint32_t)0x00080000)          /*!< Reset USB */
#define RCC_PRESETCTRL_CRS                  ((uint32_t)0x00100000)          /*!< Reset CRS */

/*******************  Bit definition for RCC_AHBCLKCTRL register  *************/
#define RCC_AHBCLKCTRL_FLASH                ((uint32_t)0x00000001)          /*!< FLASH clock bit */
#define RCC_AHBCLKCTRL_RAM                  ((uint32_t)0x00000002)          /*!< RAM clock bit */
#define RCC_AHBCLKCTRL_GPIO                 ((uint32_t)0x00000004)          /*!< GPIO clock bit */
#define RCC_AHBCLKCTRL_UART0                ((uint32_t)0x00000008)          /*!< UART0 clock bit */
#define RCC_AHBCLKCTRL_I2C0                 ((uint32_t)0x00000010)          /*!< I2C0 clock bit */
#define RCC_AHBCLKCTRL_ADC                  ((uint32_t)0x00000020)          /*!< ADC clock bit */
#define RCC_AHBCLKCTRL_IOCON                ((uint32_t)0x00000040)          /*!< IOCON clock bit */
#define RCC_AHBCLKCTRL_WWDG                 ((uint32_t)0x00000080)          /*!< WWDG clock bit */
#define RCC_AHBCLKCTRL_MCPWM                ((uint32_t)0x00000100)          /*!< MCPWM clock bit */
#define RCC_AHBCLKCTRL_SPI0                 ((uint32_t)0x00000200)          /*!< SPI0 clock bit */
#define RCC_AHBCLKCTRL_TIMER0               ((uint32_t)0x00000400)          /*!< TIMER0 clock bit */
#define RCC_AHBCLKCTRL_IWDG                 ((uint32_t)0x00001000)          /*!< IWDG clock bit */
#define RCC_AHBCLKCTRL_TSC                  ((uint32_t)0x00002000)          /*!< TSC clock bit */
#define RCC_AHBCLKCTRL_ACMP                 ((uint32_t)0x00008000)          /*!< ACMP clock bit */
#define RCC_AHBCLKCTRL_STIMER               ((uint32_t)0x00020000)          /*!< Simple Timer clock bit */
#define RCC_AHBCLKCTRL_CAN                  ((uint32_t)0x00040000)          /*!< STIMER clock bit */
#define RCC_AHBCLKCTRL_USB                  ((uint32_t)0x00080000)          /*!< USB clock bit */
#define RCC_AHBCLKCTRL_CRS                  ((uint32_t)0x00100000)          /*!< CRS clock bit */

/*******************  Bit definition for RCC_BOOTREMAP register  **************/
#define RCC_BOOTREMAP_REMAP                 ((uint32_t)0x00000002)          /*!< Remap flag bit */
#define RCC_BOOTREMAP_PROTECT0              ((uint32_t)0x00000004)          /*!< Protect0 flag bit */
#define RCC_BOOTREMAP_PROTECT1              ((uint32_t)0x00000008)          /*!< Protect1 flag bit */

/*******************  Bit definition for RCC_BOOTREMAP register  **************/
#define RCC_REMAP_FLASH_ADDR_MASK           ((uint32_t)0x0000FFFF)          /*!< Mask of flash Remap addr */
#define RCC_REMAP_FLASH_ENABLE              ((uint32_t)0x55550000)          /*!< Remap write enable */
#define RCC_REMAP_FLASH_DISABLE             ((uint32_t)0xAAAA0000)          /*!< Remap disable */

/*******************  Bit definition for RCC_12MOSCCTRL register  *************/
#define RCC_12OSCCTRL_OSC_EI                ((uint32_t)0x00010000)          /*!< OSC enable */
#define RCC_12OSCCTRL_OSC_EO                ((uint32_t)0x00020000)          /*!< OSC enable */
#define RCC_12OSCCTRL_OSC_OK                ((uint32_t)0x80000000)          /*!< OSC flag bit */

/*******************  Bit definition for RCC_SYSPLLCTRL register  *************/
#define RCC_SYSPLLCTRL_FREQ                 ((uint32_t)0x0000001F)          /*!< System pll frequency */
#define RCC_REF_SEL                         ((uint32_t)0x40000000)          /*!< System pll clock source */
#define RCC_SYSPLLCTRL_FORCELOCK            ((uint32_t)0x80000000)          /*!< System pll lock */

/*******************  Bit definition for RCC_SYSPLLSTAT register  *************/
#define RCC_SYSPLLSTAT_LOCK                 ((uint32_t)0x00000001)         /*!< System pll lock bit */

/*******************  Bit definition for RCC_SYSRSTSTAT register  *************/
#define RCC_SYSRSTSTAT_PORRST               ((uint32_t)0x00000001)         /*!< POR reset */
#define RCC_SYSRSTSTAT_EXTRST               ((uint32_t)0x00000002)         /*!< EXT reset */
#define RCC_SYSRSTSTAT_WWDGRST              ((uint32_t)0x00000004)         /*!< WWDG reset */
#define RCC_SYSRSTSTAT_BORRST               ((uint32_t)0x00000008)         /*!< BOR reset */
#define RCC_SYSRSTSTAT_SYSRST               ((uint32_t)0x00000010)         /*!< SYS reset */
#define RCC_SYSRSTSTAT_IWDGRST              ((uint32_t)0x00000020)         /*!< IWDG reset */

/*******************  Bit definition for RCC_MAINCLKSEL register  *************/
#define RCC_MAINCLKSEL_SEL_MASK             ((uint32_t)0x00000003)         /*!< Main clock source */
#define RCC_MAINCLKSEL_SEL_12M_IRC          ((uint32_t)0x00000000)         /*!< Main clock source -- 12M IRC */
#define RCC_MAINCLKSEL_SEL_SYSTEM_PLL       ((uint32_t)0x00000001)         /*!< Main clock source -- SYSTEM PLL */
#define RCC_MAINCLKSEL_SEL_12M_OSC          ((uint32_t)0x00000002)         /*!< Main clock source -- 12M OSC */
#define RCC_MAINCLKSEL_SEL_25K_IRC          ((uint32_t)0x00000003)         /*!< Main clock source -- 25K IRC */

/*******************  Bit definition for RCC_UARTCLKSEL register  ************/
#define RCC_UARTCLKSEL_SEL_MASK            ((uint32_t)0x00000003)         /*!< UART clock source */
#define RCC_UARTCLKSEL_SEL_12M_IRC         ((uint32_t)0x00000000)         /*!< UART clock source -- 12M IRC */
#define RCC_UARTCLKSEL_SEL_SYSTEM_PLL      ((uint32_t)0x00000001)         /*!< UART clock source -- SYSTEM PLL */
#define RCC_UARTCLKSEL_SEL_12M_OSC         ((uint32_t)0x00000002)         /*!< UART clock source -- 12M OSC */
#define RCC_UARTCLKSEL_SEL_25K_IRC         ((uint32_t)0x00000003)         /*!< UART clock source -- 25K IRC */

/*******************  Bit definition for RCC_OUTCLKSEL register  **************/
#define RCC_OUTCLKSEL_SEL                   ((uint32_t)0x00000003)         /*!< OUTCLK clock source */
#define RCC_OUTCLKSEL_SEL_12M_IRC           ((uint32_t)0x00000000)         /*!< OUTCLK clock source -- 12M IRC */
#define RCC_OUTCLKSEL_SEL_SYSTEM_PLL        ((uint32_t)0x00000001)         /*!< OUTCLK clock source -- SYSTEM PLL */
#define RCC_OUTCLKSEL_SEL_25K_IRC           ((uint32_t)0x00000002)         /*!< OUTCLK clock source -- 25K OSC */
#define RCC_OUTCLKSEL_SEL_12M_OSC           ((uint32_t)0x00000003)         /*!< OUTCLK clock source -- 12M IRC */

/*******************  Bit definition for RCC_WAKEUPEN register  ***************/
#define RCC_WAKEUPEN_MASK                   ((uint32_t)0x000000FF)         /*!< Wakeup pin mask */
#define RCC_WAKEUPEN_0                      ((uint32_t)0x00000001)         /*!< wakeup pin 0 */
#define RCC_WAKEUPEN_1                      ((uint32_t)0x00000002)         /*!< wakeup pin 1 */
#define RCC_WAKEUPEN_2                      ((uint32_t)0x00000004)         /*!< wakeup pin 2 */
#define RCC_WAKEUPEN_3                      ((uint32_t)0x00000008)         /*!< wakeup pin 3 */
#define RCC_WAKEUPEN_4                      ((uint32_t)0x00000010)         /*!< wakeup pin 4 */
#define RCC_WAKEUPEN_5                      ((uint32_t)0x00000020)         /*!< wakeup pin 5 */
#define RCC_WAKEUPEN_6                      ((uint32_t)0x00000040)         /*!< wakeup pin 6 */
#define RCC_WAKEUPEN_7                      ((uint32_t)0x00000080)         /*!< wakeup pin 7 */

/*******************  Bit definition for RCC_BORCTRL register  ****************/
#define RCC_BORCTRL_BORLEV_MASK             ((uint32_t)0x00000003)         /*!< Mask of BOR level */
#define RCC_BORCTRL_BORLEV_3_7V             ((uint32_t)0x00000000)         /*!< BOR level 3.7V */
#define RCC_BORCTRL_BORLEV_2_7V             ((uint32_t)0x00000001)         /*!< BOR level 2.7V */
#define RCC_BORCTRL_BORLEV_2_2V             ((uint32_t)0x00000002)         /*!< BOR level 2.2V */
#define RCC_BORCTRL_BORLEV_1_8V             ((uint32_t)0x00000003)         /*!< BOR level 1.8V */
#define RCC_BORCTRL_BORENA                  ((uint32_t)0x00000004)         /*!< Enable BOR */
#define RCC_BORCTRL_BODLEV_MASK             ((uint32_t)0x00180000)         /*!< Mask of BOD level */
#define RCC_BORCTRL_BODLEV_2_2V             ((uint32_t)0x00000000)         /*!< BOD level 2.2V */
#define RCC_BORCTRL_BODLEV_2_7V             ((uint32_t)0x00080000)         /*!< BOD level 2.7V */
#define RCC_BORCTRL_BODLEV_3_7V             ((uint32_t)0x00100000)         /*!< BOD level 3.7V */
#define RCC_BORCTRL_BODLEV_4_4V             ((uint32_t)0x00180000)         /*!< BOD level 4.4V */

/*******************  Bit definition for RCC_PDSLEEPCFG register  ****************/
#define RCC_PDSLEEPCFG_ADC                  ((uint32_t)0x00000001)         /*!< Power down ADC */
#define RCC_PDSLEEPCFG_BOD                  ((uint32_t)0x00000002)         /*!< Power down BOD */
#define RCC_PDSLEEPCFG_BOR                  ((uint32_t)0x00000004)         /*!< Power down BOR */
#define RCC_PDSLEEPCFG_IRC12M               ((uint32_t)0x00000008)         /*!< Power down 12MIRC */
#define RCC_PDSLEEPCFG_FLASH                ((uint32_t)0x00000010)         /*!< Power down FLASH */
#define RCC_PDSLEEPCFG_SYSPLL               ((uint32_t)0x00000020)         /*!< Power down SYSPLL */

/*******************  Bit definition for RCC_PDAWAKECFG register  ****************/
#define RCC_PDAWAKECFG_ADC                  ((uint32_t)0x00000001)         /*!< Power on ADC */
#define RCC_PDAWAKECFG_BOD                  ((uint32_t)0x00000002)         /*!< Power on BOD */
#define RCC_PDAWAKECFG_BOR                  ((uint32_t)0x00000004)         /*!< Power on BOR */
#define RCC_PDAWAKECFG_IRC12M               ((uint32_t)0x00000008)         /*!< Power on 12MIRC */
#define RCC_PDAWAKECFG_FLASH                ((uint32_t)0x00000010)         /*!< Power on FLASH */
#define RCC_PDAWAKECFG_SYSPLL               ((uint32_t)0x00000020)         /*!< Power on SYSPLL */

/*******************  Bit definition for RCC_PDRUNCFG register  ****************/
#define RCC_PDRUNCFG_ADC                    ((uint32_t)0x00000001)         /*!< Power on/down ADC */
#define RCC_PDRUNCFG_BOD                    ((uint32_t)0x00000002)         /*!< Power on/down BOD */
#define RCC_PDRUNCFG_BOR                    ((uint32_t)0x00000004)         /*!< Power on/down BOR */
#define RCC_PDRUNCFG_IRC12M                 ((uint32_t)0x00000008)         /*!< Power on/down 12MIRC */
#define RCC_PDRUNCFG_FLASH                  ((uint32_t)0x00000010)         /*!< Power on/down FLASH */
#define RCC_PDRUNCFG_SYSPLL                 ((uint32_t)0x00000020)         /*!< Power on/down SYSPLL */
#define RCC_PDRUNCFG_USB                    ((uint32_t)0x00000080)         /*!< Power on/down USB */
 
/*******************  Bit definition for RCC_PCON register  *******************/
#define RCC_PCON_DPDEN                      ((uint32_t)0x00000002)         /*!< Deep power down */
#define RCC_PCON_SLEEP                      ((uint32_t)0x00000100)         /*!< Deep sleep flag bit */
#define RCC_PCON_DPFLAG                     ((uint32_t)0x00000800)         /*!< Deep power down flag bit */
#define RCC_PCON_LOWPOWER                   ((uint32_t)0x00010000)         /*!< Change LDO source */
#define RCC_PCON_LDO_STATUS                 ((uint32_t)0x00100000)         /*!< Main LDO flag bit */

/*******************  Bit definition for RCC_DBGCTRL register  ****************/
#define RCC_DBGCTRL_IWDG                    ((uint32_t)0x00000010)         /*!< Stop IWDG count when SW debug */
#define RCC_DBGCTRL_WWDG                    ((uint32_t)0x00000020)         /*!< Stop WWDG count when SW debug */
#define RCC_DBGCTRL_DIS_NRST                ((uint32_t)0x00100000)         /*!< Disable NRST */

/*******************  Bit definition for RCC_ANACTRL1 register  ****************/
#define RCC_ANACTRL1_CMP2PWM_MASK           ((uint32_t)0x00000003)         /*!< Mask of MCPWM Abort */
#define RCC_ANACTRL1_CMP2PWM_2IO            ((uint32_t)0x00000000)         /*!< MCPWM Abort connect to IO */
#define RCC_ANACTRL1_CMP2PWM_2CMP0          ((uint32_t)0x00000001)         /*!< MCPWM Abort connect to CMP0 */
#define RCC_ANACTRL1_CMP2PWM_2CMP1          ((uint32_t)0x00000002)         /*!< MCPWM Abort connect to CMP1 */
#define RCC_ANACTRL1_CMP2PWM_2CMP01         ((uint32_t)0x00000003)         /*!< MCPWM Abort connect to CMP0 & CMP1 */
#define RCC_ANACTRL1_ADCBUF                 ((uint32_t)0x00000004)         /*!< ADC buffer mode */
#define RCC_ANACTRL1_ADCCHOP                ((uint32_t)0x00000008)         /*!< ADC chop mode */
#define RCC_ANACTRL1_OPA0CHOP               ((uint32_t)0x00000010)         /*!< OPA0 chop mode */
#define RCC_ANACTRL1_OPA1CHOP               ((uint32_t)0x00000020)         /*!< OPA1 chop mode */
#define RCC_ANACTRL1_512B_MODE              ((uint32_t)0x00000100)         /*!< Flash 512bytes mode */
#define RCC_ANACTRL1_ADC_REG                ((uint32_t)0x00000200)         /*!< ADC reg mode */
#define RCC_ANACTRL1_UART_INVR              ((uint32_t)0x00001000)         /*!< Inverse UART RX */
#define RCC_ANACTRL1_UART_INVT              ((uint32_t)0x00002000)         /*!< Inverse UART TX */

/******************************************************************************/
/*                                                                            */
/*         Universal Asynchronous Receiver Transmitter (UART)                 */
/*                                                                            */
/******************************************************************************/
/*******************  Bit definition for HW_UART_DR register  *****************/
#define UART_DR_DATA                        ((uint32_t)0x000000FF)          /*!< DATA. */
#define UART_DR_FE                          ((uint32_t)0x00000100)          /*!< Framing error bit. */
#define UART_DR_PE                          ((uint32_t)0x00000200)          /*!< Parity error bit. */
#define UART_DR_BE                          ((uint32_t)0x00000400)          /*!< Break error bit. */
#define UART_DR_OE                          ((uint32_t)0x00000800)          /*!< Overrun error bit. */

/*******************  Bit definition for HW_UART_RSR/ECR register  ************/
#define UART_RSR_FE                         ((uint32_t)0x00000001)          /*!< Framing error bit. */
#define UART_RSR_PE                         ((uint32_t)0x00000002)          /*!< Parity error bit. */
#define UART_RSR_BE                         ((uint32_t)0x00000004)          /*!< Break error bit. */
#define UART_RSR_OE                         ((uint32_t)0x00000008)          /*!< Overrun error bit. */

#define UART_ECR_FE                         ((uint32_t)0x00000001)          /*!< Framing error bit. */
#define UART_ECR_PE                         ((uint32_t)0x00000002)          /*!< Parity error bit. */
#define UART_ECR_BE                         ((uint32_t)0x00000004)          /*!< Break error bit. */
#define UART_ECR_OE                         ((uint32_t)0x00000008)          /*!< Overrun error bit. */

/*******************  Bit definition for HW_UART_FR register  *****************/
#define UART_FR_BUSY                        ((uint32_t)0x00000008)          /*!< Uart busy. */
#define UART_FR_RXFE                        ((uint32_t)0x00000010)          /*!< Receive fifo empty. */
#define UART_FR_TXFF                        ((uint32_t)0x00000020)          /*!< Transmit fifo full. */
#define UART_FR_RXFF                        ((uint32_t)0x00000040)          /*!< Receive fifo full. */
#define UART_FR_TXFE                        ((uint32_t)0x00000080)          /*!< Transmit fifo empty. */
#define UART_FR_RI                          ((uint32_t)0x00000100)          /*!< Ring indicator. */

/*******************  Bit definition for HW_UART_ILPR register  *****************/
#define UART_ILPR_DVSR                      ((uint32_t)0x000000FF)          /*!< 8-bit low-power divisor value. */

/*******************  Bit definition for HW_UART_IBRD register  *****************/
#define UART_IBRD_DIVINT                    ((uint32_t)0x0000FFFF)          /*!< The integer baud rate divisor. */

/*******************  Bit definition for HW_UART_FBRD register  *****************/
#define UART_FBRD_DIVFRAC                   ((uint32_t)0x0000003F)          /*!< The fractional baud rate divisor. */

/*******************  Bit definition for HW_UART_LCRH register  *****************/
#define UART_LCRH_BRK                       ((uint32_t)0x00000001)          /*!< Send break. */
#define UART_LCRH_PEN                       ((uint32_t)0x00000002)          /*!< Parity enable. */
#define UART_LCRH_EPS                       ((uint32_t)0x00000004)          /*!< Even parity select. */
#define UART_LCRH_STP2                      ((uint32_t)0x00000008)          /*!< Two stop bits select. */
#define UART_LCRH_FEN                       ((uint32_t)0x00000010)          /*!< Enable FIFOs. */
#define UART_LCRH_WLEN                      ((uint32_t)0x00000060)          /*!< Word length. */
#define UART_LCRH_WLEN_5BITS                ((uint32_t)0x00000000)          /*!< Word length 5 bits. */
#define UART_LCRH_WLEN_6BITS                ((uint32_t)0x00000020)          /*!< Word length 6 bits. */
#define UART_LCRH_WLEN_7BITS                ((uint32_t)0x00000040)          /*!< Word length 7 bits. */
#define UART_LCRH_WLEN_8BITS                ((uint32_t)0x00000060)          /*!< Word length 8 bits. */
#define UART_LCRH_SPS                       ((uint32_t)0x00000080)          /*!< Stick parity select. */

/*******************  Bit definition for HW_UART_CR register  *******************/
#define UART_CR_UARTEN                      ((uint32_t)0x00000001)          /*!< UART enable. */
#define UART_CR_TXE                         ((uint32_t)0x00000100)          /*!< Transmit enable. */
#define UART_CR_RXE                         ((uint32_t)0x00000200)          /*!< Receive enable. */

/*******************  Bit definition for HW_UART_IFLS register  *****************/
#define UART_IFLS_TXIFLSEL                  ((uint32_t)0x00000007)          /*!< Transmit interrupt fifo level select. */
#define UART_IFLS_RXIFLSEL                  ((uint32_t)0x00000038)          /*!< Receive interrupt fifo level select. */

/*******************  Bit definition for HW_UART_IMSC register  *****************/
#define UART_IMSC_RXIM                      ((uint32_t)0x00000010)          /*!< Receive interrupt mask. */
#define UART_IMSC_TXIM                      ((uint32_t)0x00000020)          /*!< Transmit interrupt mask. */
#define UART_IMSC_RTIM                      ((uint32_t)0x00000040)          /*!< Receive timeout interrupt mask. */
#define UART_IMSC_FEIM                      ((uint32_t)0x00000080)          /*!< Framing error interrupt mask. */
#define UART_IMSC_PEIM                      ((uint32_t)0x00000100)          /*!< Parity error interrupt mask. */
#define UART_IMSC_BEIM                      ((uint32_t)0x00000200)          /*!< Break error interrupt mask. */
#define UART_IMSC_OEIM                      ((uint32_t)0x00000400)          /*!< Overrun error interrupt mask. */
#define UART_IMSC_IDIM                      ((uint32_t)0x00000800)          /*!< Idle interrupt mask. */

/*******************  Bit definition for HW_UART_RIS register  ******************/
#define UART_RIS_RXRIS                      ((uint32_t)0x00000010)          /*!< Receive interrupt raw status. */
#define UART_RIS_TXRIS                      ((uint32_t)0x00000020)          /*!< Transmit interrupt raw status. */
#define UART_RIS_RTRIS                      ((uint32_t)0x00000040)          /*!< Receive timeout interrupt raw status. */
#define UART_RIS_FERIS                      ((uint32_t)0x00000080)          /*!< Framing error interrupt raw status. */
#define UART_RIS_PERIS                      ((uint32_t)0x00000100)          /*!< Parity error interrupt raw status. */
#define UART_RIS_BERIS                      ((uint32_t)0x00000200)          /*!< Break error interrupt raw status. */
#define UART_RIS_OERIS                      ((uint32_t)0x00000400)          /*!< Overrun error interrupt raw status. */
#define UART_RIS_IDRIS                      ((uint32_t)0x00000800)          /*!< Idle interrupt raw status. */

/*******************  Bit definition for HW_UART_MIS register  ******************/
#define UART_MIS_RXMIS                      ((uint32_t)0x00000010)          /*!< Masked Receive interrupt raw status. */
#define UART_MIS_TXMIS                      ((uint32_t)0x00000020)          /*!< Masked Transmit interrupt raw status. */
#define UART_MIS_RTMIS                      ((uint32_t)0x00000040)          /*!< Masked Receive timeout interrupt raw status. */
#define UART_MIS_FEMIS                      ((uint32_t)0x00000080)          /*!< Masked Framing error interrupt raw status. */
#define UART_MIS_PEMIS                      ((uint32_t)0x00000100)          /*!< Masked Parity error interrupt raw status. */
#define UART_MIS_BEMIS                      ((uint32_t)0x00000200)          /*!< Masked Break error interrupt raw status. */
#define UART_MIS_OEMIS                      ((uint32_t)0x00000400)          /*!< Masked Overrun error interrupt raw status. */
#define UART_MIS_IDMIS                      ((uint32_t)0x00000800)          /*!< Masked Idle interrupt raw status. */

/*******************  Bit definition for HW_UART_ICR register  ******************/
#define UART_ICR_RXIC                       ((uint32_t)0x00000010)          /*!< Clear Receive interrupt. */
#define UART_ICR_TXIC                       ((uint32_t)0x00000020)          /*!< Clear Transmit interrupt. */
#define UART_ICR_RTIC                       ((uint32_t)0x00000040)          /*!< Clear Receive timeout interrupt. */
#define UART_ICR_FEIC                       ((uint32_t)0x00000080)          /*!< Clear Framing error interrupt. */
#define UART_ICR_PEIC                       ((uint32_t)0x00000100)          /*!< Clear Parity error interrupt. */
#define UART_ICR_BEIC                       ((uint32_t)0x00000200)          /*!< Clear Break error interrupt. */
#define UART_ICR_OEIC                       ((uint32_t)0x00000400)          /*!< Clear Overrun error interrupt. */
#define UART_ICR_IDIC                       ((uint32_t)0x00000800)          /*!< Clear Idle interrupt. */

/*******************  Bit definition for HW_UART_IDCR register  *****************/
#define UART_IDCR_COUNT_MASK                ((uint32_t)0x0000000F)          /*!< Idle count. */
#define UART_IDCR_ENABLE                    ((uint32_t)0x00000100)          /*!< Idle enable. */

/******************************************************************************/
/*                                                                            */
/*                         Controller Area Network                            */
/*                                                                            */
/******************************************************************************/
/*******************  Bit definition for CAN_MOD register  ********************/
#define  CAN_MOD_RM                         ((uint8_t)0x01)            /*!<Reset MODE */
#define  CAN_MOD_LOM                        ((uint8_t)0x02)            /*!<Listen only MODE */
#define  CAN_MOD_STM                        ((uint8_t)0x04)            /*!<Self Test MODE */
#define  CAN_MOD_AFM                        ((uint8_t)0x08)            /*!<Acceptance Filter Mode */
#define  CAN_MOD_SM                         ((uint8_t)0x10)            /*!<Sleep Mode */

/*******************  Bit definition for CAN_CMR register  ********************/
#define  CAN_CMR_TR                         ((uint8_t)0x01)            /*!<Transmission Request */
#define  CAN_CMR_AT                        	((uint8_t)0x02)            /*!<Abort Transmission */
#define  CAN_CMR_RRB                        ((uint8_t)0x04)            /*!<Release Receive Buffer*/
#define  CAN_CMR_CDO                        ((uint8_t)0x08)            /*!<Clear Data Overrun*/
#define  CAN_CMR_SRR                        ((uint8_t)0x10)            /*!<Self Reception Request */

/*******************  Bit definition for CAN_IR register  ********************/
#define  CAN_IR_RI                         ((uint8_t)0x01)            /*!<Receive Interrupt*/
#define  CAN_IR_TI                         ((uint8_t)0x02)            /*!<Transmit Interrupt*/
#define  CAN_IR_EI                         ((uint8_t)0x04)            /*!<Error Warning Interrupt*/
#define  CAN_IR_DOI                        ((uint8_t)0x08)            /*!<Data Overrupt Interrupt*/
#define  CAN_IR_WUI                        ((uint8_t)0x10)            /*!<Wake-up Interrupt*/
#define  CAN_IR_EPI                        ((uint8_t)0x20)            /*!<Error Passive Interrupt*/
#define  CAN_IR_ALI                        ((uint8_t)0x40)            /*!<Arbitration Lost Interrupt*/
#define  CAN_IR_BEI                        ((uint8_t)0x80)            /*!<Bus Error Interrupt*/

/******************************************************************************/
/*                                                                            */
/*                           Flash Memory Control                			  */
/*                                                                            */
/******************************************************************************/
/*******************  Bit definition for HW_FLASH_ACR register  ***************/
#define FLASH_ACR_LATENCY                   ((uint32_t)0x00000007)          /*!< LATENCY bit (Latency) */
#define FLASH_ACR_LATENCY_0                 ((uint32_t)0x00000000)          /*!< LATENCY bit (Latency) 0 */
#define FLASH_ACR_LATENCY_1                 ((uint32_t)0x00000001)          /*!< LATENCY bit (Latency) 1 */
#define FLASH_ACR_LATENCY_2                 ((uint32_t)0x00000002)          /*!< LATENCY bit (Latency) 2 */
#define FLASH_ACR_LATENCY_3                 ((uint32_t)0x00000003)          /*!< LATENCY bit (Latency) 3 */
#define FLASH_ACR_LATENCY_4                 ((uint32_t)0x00000004)          /*!< LATENCY bit (Latency) 4 */
#define FLASH_ACR_LATENCY_5                 ((uint32_t)0x00000005)          /*!< LATENCY bit (Latency) 5 */
#define FLASH_ACR_LATENCY_6                 ((uint32_t)0x00000006)          /*!< LATENCY bit (Latency) 6 */
#define FLASH_ACR_CACHENA                   ((uint32_t)0x00000100)          /*!< Cache Enable */
#define FLASH_ACR_FLUSH                     ((uint32_t)0x00010000)          /*!< Flush Cache */
#define FLASH_ACR_CS                        ((uint32_t)0x80000000)          /*!< FLASH CS */

/*******************  Bit definition for HW_FLASH_KEYR register  **************/
#define FLASH_KEYR                          ((uint32_t)0xFFFFFFFF)          /*!< CR Key */

/******************  FLASH Keys  **********************************************/
#define FLASH_FKEY1                         ((uint32_t)0x76543210)          /*!< Flash program erase key1 */
#define FLASH_FKEY2                         ((uint32_t)0xFEDCBA98)          /*!< Flash program erase key2: used with FLASH_PEKEY1
                                                                                to unlock the write access to the CR. */

/*******************  Bit definition for HW_FLASH_KEYR register  **************/
#define FLASH_ERASER_NUM                    ((uint32_t)0x0000003F)          /*!< Flash sector number */
#define FLASH_ERASER_MAP_FLAG               ((uint32_t)0x10000000)          /*!< Flash address 0x10000000 */

/*******************  Bit definition for HW_FLASH_SR register  ****************/
#define FLASH_SR_EOP                        ((uint32_t)0x00000001)          /*!< End of operation */
#define FLASH_SR_EOE                        ((uint32_t)0x00000002)          /*!< End of erase */
#define FLASH_SR_BUSY                       ((uint32_t)0x00010000)          /*!< Busy */

/*******************  Bit definition for HW_FLASH_CR register  ****************/
#define FLASH_CR_PG                         ((uint32_t)0x00000001)          /*!< Programming */
#define FLASH_CR_SER                        ((uint32_t)0x00000002)          /*!< Sector erase */
#define FLASH_CR_MER                        ((uint32_t)0x00000004)          /*!< Mass erase */
#define FLASH_CR_NVR                        ((uint32_t)0x00000008)          /*!< Main or Info */
#define FLASH_CR_LDROMEN                    ((uint32_t)0x00000100)          /*!< LDROM */
#define FLASH_CR_START                      ((uint32_t)0x00010000)          /*!< Start */
#define FLASH_CR_EOPIE                      ((uint32_t)0x00100000)          /*!< End of operation interrupt enable */
#define FLASH_CR_EOEIE                      ((uint32_t)0x00200000)          /*!< End of erase interrupt enable */
#define FLASH_CR_OPENNVR                    ((uint32_t)0x20000000)          /*!< Enable NVR read */
#define FLASH_CR_LOCK                       ((uint32_t)0x80000000)          /*!< CR lock */

/******************************************************************************/
/*                                                                            */
/*                                 IOCON                                      */
/*                                                                            */
/******************************************************************************/
/*******************  Bit definition for IOCON register  **********************/
#define	IOCON_FUNC_MASK                     ((uint32_t)0x00000007)          /*!< IO function mask. */
#define	IOCON_FUNC_0                        ((uint32_t)0x00000000)          /*!< IO function 0. */
#define	IOCON_FUNC_1                        ((uint32_t)0x00000001)          /*!< IO function 1. */
#define	IOCON_FUNC_2                        ((uint32_t)0x00000002)          /*!< IO function 2. */
#define	IOCON_FUNC_3                        ((uint32_t)0x00000003)          /*!< IO function 3. */
#define	IOCON_FUNC_4                        ((uint32_t)0x00000004)          /*!< IO function 4. */
#define	IOCON_FUNC_5                        ((uint32_t)0x00000005)          /*!< IO function 5. */
#define	IOCON_FUNC_6                        ((uint32_t)0x00000006)          /*!< IO function 6. */
#define	IOCON_FUNC_7                        ((uint32_t)0x00000007)          /*!< IO function 7. */
#define	IOCON_MODE_MASK                     ((uint32_t)0x00000018)          /*!< IO mode mask. */
#define	IOCON_MODE_0                        ((uint32_t)0x00000000)          /*!< IO floating. */
#define	IOCON_MODE_1                        ((uint32_t)0x00000008)          /*!< IO pull down. */
#define	IOCON_MODE_2                        ((uint32_t)0x00000010)          /*!< IO pull up. */
#define	IOCON_MODE_3                        ((uint32_t)0x00000018)          /*!< reserved. */
#define	IOCON_ANA                           ((uint32_t)0x00000020)          /*!< IO Analog mode. */
#define	IOCON_IE                            ((uint32_t)0x00000080)          /*!< IO in enable. */
#define	IOCON_SR                            ((uint32_t)0x00000200)          /*!< IO speed. */
#define	IOCON_OD                            ((uint32_t)0x00001000)          /*!< IO open-drain. */
#define	IOCON_OS                            ((uint32_t)0x00002000)          /*!< IO OS. */

/******************************************************************************/
/*                                                                            */
/*                General Purpose and Alternate Function I/O                  */
/*                                                                            */
/******************************************************************************/
/*******************  Bit definition for GPIO_DATA register  ******************/
#define GPIO_DATA_BIT0                      ((uint32_t)0x00000001)          /*!< Pin data, bit 0 */
#define GPIO_DATA_BIT1                      ((uint32_t)0x00000002)          /*!< Pin data, bit 1 */
#define GPIO_DATA_BIT2                      ((uint32_t)0x00000004)          /*!< Pin data, bit 2 */
#define GPIO_DATA_BIT3                      ((uint32_t)0x00000008)          /*!< Pin data, bit 3 */
#define GPIO_DATA_BIT4                      ((uint32_t)0x00000010)          /*!< Pin data, bit 4 */
#define GPIO_DATA_BIT5                      ((uint32_t)0x00000020)          /*!< Pin data, bit 5 */
#define GPIO_DATA_BIT6                      ((uint32_t)0x00000040)          /*!< Pin data, bit 6 */
#define GPIO_DATA_BIT7                      ((uint32_t)0x00000080)          /*!< Pin data, bit 7 */
#define GPIO_DATA_BIT8                      ((uint32_t)0x00000100)          /*!< Pin data, bit 8 */
#define GPIO_DATA_BIT9                      ((uint32_t)0x00000200)          /*!< Pin data, bit 9 */
#define GPIO_DATA_BIT10                     ((uint32_t)0x00000400)          /*!< Pin data, bit 10 */
#define GPIO_DATA_BIT11                     ((uint32_t)0x00000800)          /*!< Pin data, bit 11 */
#define GPIO_DATA_BIT12                     ((uint32_t)0x00001000)          /*!< Pin data, bit 12 */
#define GPIO_DATA_BIT13                     ((uint32_t)0x00002000)          /*!< Pin data, bit 13 */
#define GPIO_DATA_BIT14                     ((uint32_t)0x00004000)          /*!< Pin data, bit 14 */
#define GPIO_DATA_BIT15                     ((uint32_t)0x00008000)          /*!< Pin data, bit 15 */
#define GPIO_DATA_BIT16                     ((uint32_t)0x00010000)          /*!< Pin data, bit 16 */
#define GPIO_DATA_BIT17                     ((uint32_t)0x00020000)          /*!< Pin data, bit 17 */
#define GPIO_DATA_BIT18                     ((uint32_t)0x00040000)          /*!< Pin data, bit 18 */
#define GPIO_DATA_BIT19                     ((uint32_t)0x00080000)          /*!< Pin data, bit 19 */
#define GPIO_DATA_BIT20                     ((uint32_t)0x00100000)          /*!< Pin data, bit 20 */
#define GPIO_DATA_BIT21                     ((uint32_t)0x00200000)          /*!< Pin data, bit 21 */
#define GPIO_DATA_BIT22                     ((uint32_t)0x00400000)          /*!< Pin data, bit 22 */
#define GPIO_DATA_BIT23                     ((uint32_t)0x00800000)          /*!< Pin data, bit 23 */
#define GPIO_DATA_BIT24                     ((uint32_t)0x01000000)          /*!< Pin data, bit 24 */
#define GPIO_DATA_BIT25                     ((uint32_t)0x02000000)          /*!< Pin data, bit 25 */
#define GPIO_DATA_BIT26                     ((uint32_t)0x04000000)          /*!< Pin data, bit 26 */
#define GPIO_DATA_BIT27                     ((uint32_t)0x08000000)          /*!< Pin data, bit 27 */
#define GPIO_DATA_BIT28                     ((uint32_t)0x10000000)          /*!< Pin data, bit 28 */
#define GPIO_DATA_BIT29                     ((uint32_t)0x20000000)          /*!< Pin data, bit 29 */
#define GPIO_DATA_BIT30                     ((uint32_t)0x40000000)          /*!< Pin data, bit 30 */
#define GPIO_DATA_BIT31                     ((uint32_t)0x80000000)          /*!< Pin data, bit 31 */

/*******************  Bit definition for GPIO_DIR register  *******************/
#define GPIO_DIR_BIT0                       ((uint32_t)0x00000001)          /*!< Pin dir, bit 0 */
#define GPIO_DIR_BIT1                       ((uint32_t)0x00000002)          /*!< Pin dir, bit 1 */
#define GPIO_DIR_BIT2                       ((uint32_t)0x00000004)          /*!< Pin dir, bit 2 */
#define GPIO_DIR_BIT3                       ((uint32_t)0x00000008)          /*!< Pin dir, bit 3 */
#define GPIO_DIR_BIT4                       ((uint32_t)0x00000010)          /*!< Pin dir, bit 4 */
#define GPIO_DIR_BIT5                       ((uint32_t)0x00000020)          /*!< Pin dir, bit 5 */
#define GPIO_DIR_BIT6                       ((uint32_t)0x00000040)          /*!< Pin dir, bit 6 */
#define GPIO_DIR_BIT7                       ((uint32_t)0x00000080)          /*!< Pin dir, bit 7 */
#define GPIO_DIR_BIT8                       ((uint32_t)0x00000100)          /*!< Pin dir, bit 8 */
#define GPIO_DIR_BIT9                       ((uint32_t)0x00000200)          /*!< Pin dir, bit 9 */
#define GPIO_DIR_BIT10                      ((uint32_t)0x00000400)          /*!< Pin dir, bit 10 */
#define GPIO_DIR_BIT11                      ((uint32_t)0x00000800)          /*!< Pin dir, bit 11 */
#define GPIO_DIR_BIT12                      ((uint32_t)0x00001000)          /*!< Pin dir, bit 12 */
#define GPIO_DIR_BIT13                      ((uint32_t)0x00002000)          /*!< Pin dir, bit 13 */
#define GPIO_DIR_BIT14                      ((uint32_t)0x00004000)          /*!< Pin dir, bit 14 */
#define GPIO_DIR_BIT15                      ((uint32_t)0x00008000)          /*!< Pin dir, bit 15 */
#define GPIO_DIR_BIT16                      ((uint32_t)0x00010000)          /*!< Pin dir, bit 16 */
#define GPIO_DIR_BIT17                      ((uint32_t)0x00020000)          /*!< Pin dir, bit 17 */
#define GPIO_DIR_BIT18                      ((uint32_t)0x00040000)          /*!< Pin dir, bit 18 */
#define GPIO_DIR_BIT19                      ((uint32_t)0x00080000)          /*!< Pin dir, bit 19 */
#define GPIO_DIR_BIT20                      ((uint32_t)0x00100000)          /*!< Pin dir, bit 20 */
#define GPIO_DIR_BIT21                      ((uint32_t)0x00200000)          /*!< Pin dir, bit 21 */
#define GPIO_DIR_BIT22                      ((uint32_t)0x00400000)          /*!< Pin dir, bit 22 */
#define GPIO_DIR_BIT23                      ((uint32_t)0x00800000)          /*!< Pin dir, bit 23 */
#define GPIO_DIR_BIT24                      ((uint32_t)0x01000000)          /*!< Pin dir, bit 24 */
#define GPIO_DIR_BIT25                      ((uint32_t)0x02000000)          /*!< Pin dir, bit 25 */
#define GPIO_DIR_BIT26                      ((uint32_t)0x04000000)          /*!< Pin dir, bit 26 */
#define GPIO_DIR_BIT27                      ((uint32_t)0x08000000)          /*!< Pin dir, bit 27 */
#define GPIO_DIR_BIT28                      ((uint32_t)0x10000000)          /*!< Pin dir, bit 28 */
#define GPIO_DIR_BIT29                      ((uint32_t)0x20000000)          /*!< Pin dir, bit 29 */
#define GPIO_DIR_BIT30                      ((uint32_t)0x40000000)          /*!< Pin dir, bit 30 */
#define GPIO_DIR_BIT31                      ((uint32_t)0x80000000)          /*!< Pin dir, bit 31 */

/*******************  Bit definition for GPIO_IS register  *******************/
#define GPIO_IS_BIT0                        ((uint32_t)0x00000001)          /*!< Pin is, bit 0 */
#define GPIO_IS_BIT1                        ((uint32_t)0x00000002)          /*!< Pin is, bit 1 */
#define GPIO_IS_BIT2                        ((uint32_t)0x00000004)          /*!< Pin is, bit 2 */
#define GPIO_IS_BIT3                        ((uint32_t)0x00000008)          /*!< Pin is, bit 3 */
#define GPIO_IS_BIT4                        ((uint32_t)0x00000010)          /*!< Pin is, bit 4 */
#define GPIO_IS_BIT5                        ((uint32_t)0x00000020)          /*!< Pin is, bit 5 */
#define GPIO_IS_BIT6                        ((uint32_t)0x00000040)          /*!< Pin is, bit 6 */
#define GPIO_IS_BIT7                        ((uint32_t)0x00000080)          /*!< Pin is, bit 7 */
#define GPIO_IS_BIT8                        ((uint32_t)0x00000100)          /*!< Pin is, bit 8 */
#define GPIO_IS_BIT9                        ((uint32_t)0x00000200)          /*!< Pin is, bit 9 */
#define GPIO_IS_BIT10                       ((uint32_t)0x00000400)          /*!< Pin is, bit 10 */
#define GPIO_IS_BIT11                       ((uint32_t)0x00000800)          /*!< Pin is, bit 11 */
#define GPIO_IS_BIT12                       ((uint32_t)0x00001000)          /*!< Pin is, bit 12 */
#define GPIO_IS_BIT13                       ((uint32_t)0x00002000)          /*!< Pin is, bit 13 */
#define GPIO_IS_BIT14                       ((uint32_t)0x00004000)          /*!< Pin is, bit 14 */
#define GPIO_IS_BIT15                       ((uint32_t)0x00008000)          /*!< Pin is, bit 15 */
#define GPIO_IS_BIT16                       ((uint32_t)0x00010000)          /*!< Pin is, bit 16 */
#define GPIO_IS_BIT17                       ((uint32_t)0x00020000)          /*!< Pin is, bit 17 */
#define GPIO_IS_BIT18                       ((uint32_t)0x00040000)          /*!< Pin is, bit 18 */
#define GPIO_IS_BIT19                       ((uint32_t)0x00080000)          /*!< Pin is, bit 19 */
#define GPIO_IS_BIT20                       ((uint32_t)0x00100000)          /*!< Pin is, bit 20 */
#define GPIO_IS_BIT21                       ((uint32_t)0x00200000)          /*!< Pin is, bit 21 */
#define GPIO_IS_BIT22                       ((uint32_t)0x00400000)          /*!< Pin is, bit 22 */
#define GPIO_IS_BIT23                       ((uint32_t)0x00800000)          /*!< Pin is, bit 23 */
#define GPIO_IS_BIT24                       ((uint32_t)0x01000000)          /*!< Pin is, bit 24 */
#define GPIO_IS_BIT25                       ((uint32_t)0x02000000)          /*!< Pin is, bit 25 */
#define GPIO_IS_BIT26                       ((uint32_t)0x04000000)          /*!< Pin is, bit 26 */
#define GPIO_IS_BIT27                       ((uint32_t)0x08000000)          /*!< Pin is, bit 27 */
#define GPIO_IS_BIT28                       ((uint32_t)0x10000000)          /*!< Pin is, bit 28 */
#define GPIO_IS_BIT29                       ((uint32_t)0x20000000)          /*!< Pin is, bit 29 */
#define GPIO_IS_BIT30                       ((uint32_t)0x40000000)          /*!< Pin is, bit 30 */
#define GPIO_IS_BIT31                       ((uint32_t)0x80000000)          /*!< Pin is, bit 31 */

/*******************  Bit definition for GPIO_IBE register  *******************/
#define GPIO_IBE_BIT0                       ((uint32_t)0x00000001)          /*!< Pin ibe, bit 0 */
#define GPIO_IBE_BIT1                       ((uint32_t)0x00000002)          /*!< Pin ibe, bit 1 */
#define GPIO_IBE_BIT2                       ((uint32_t)0x00000004)          /*!< Pin ibe, bit 2 */
#define GPIO_IBE_BIT3                       ((uint32_t)0x00000008)          /*!< Pin ibe, bit 3 */
#define GPIO_IBE_BIT4                       ((uint32_t)0x00000010)          /*!< Pin ibe, bit 4 */
#define GPIO_IBE_BIT5                       ((uint32_t)0x00000020)          /*!< Pin ibe, bit 5 */
#define GPIO_IBE_BIT6                       ((uint32_t)0x00000040)          /*!< Pin ibe, bit 6 */
#define GPIO_IBE_BIT7                       ((uint32_t)0x00000080)          /*!< Pin ibe, bit 7 */
#define GPIO_IBE_BIT8                       ((uint32_t)0x00000100)          /*!< Pin ibe, bit 8 */
#define GPIO_IBE_BIT9                       ((uint32_t)0x00000200)          /*!< Pin ibe, bit 9 */
#define GPIO_IBE_BIT10                      ((uint32_t)0x00000400)          /*!< Pin ibe, bit 10 */
#define GPIO_IBE_BIT11                      ((uint32_t)0x00000800)          /*!< Pin ibe, bit 11 */
#define GPIO_IBE_BIT12                      ((uint32_t)0x00001000)          /*!< Pin ibe, bit 12 */
#define GPIO_IBE_BIT13                      ((uint32_t)0x00002000)          /*!< Pin ibe, bit 13 */
#define GPIO_IBE_BIT14                      ((uint32_t)0x00004000)          /*!< Pin ibe, bit 14 */
#define GPIO_IBE_BIT15                      ((uint32_t)0x00008000)          /*!< Pin ibe, bit 15 */
#define GPIO_IBE_BIT16                      ((uint32_t)0x00010000)          /*!< Pin ibe, bit 16 */
#define GPIO_IBE_BIT17                      ((uint32_t)0x00020000)          /*!< Pin ibe, bit 17 */
#define GPIO_IBE_BIT18                      ((uint32_t)0x00040000)          /*!< Pin ibe, bit 18 */
#define GPIO_IBE_BIT19                      ((uint32_t)0x00080000)          /*!< Pin ibe, bit 19 */
#define GPIO_IBE_BIT20                      ((uint32_t)0x00100000)          /*!< Pin ibe, bit 20 */
#define GPIO_IBE_BIT21                      ((uint32_t)0x00200000)          /*!< Pin ibe, bit 21 */
#define GPIO_IBE_BIT22                      ((uint32_t)0x00400000)          /*!< Pin ibe, bit 22 */
#define GPIO_IBE_BIT23                      ((uint32_t)0x00800000)          /*!< Pin ibe, bit 23 */
#define GPIO_IBE_BIT24                      ((uint32_t)0x01000000)          /*!< Pin ibe, bit 24 */
#define GPIO_IBE_BIT25                      ((uint32_t)0x02000000)          /*!< Pin ibe, bit 25 */
#define GPIO_IBE_BIT26                      ((uint32_t)0x04000000)          /*!< Pin ibe, bit 26 */
#define GPIO_IBE_BIT27                      ((uint32_t)0x08000000)          /*!< Pin ibe, bit 27 */
#define GPIO_IBE_BIT28                      ((uint32_t)0x10000000)          /*!< Pin ibe, bit 28 */
#define GPIO_IBE_BIT29                      ((uint32_t)0x20000000)          /*!< Pin ibe, bit 29 */
#define GPIO_IBE_BIT30                      ((uint32_t)0x40000000)          /*!< Pin ibe, bit 30 */
#define GPIO_IBE_BIT31                      ((uint32_t)0x80000000)          /*!< Pin ibe, bit 31 */

/*******************  Bit definition for GPIO_IEV register  *******************/
#define GPIO_IEV_BIT0                       ((uint32_t)0x00000001)          /*!< Pin iev, bit 0 */
#define GPIO_IEV_BIT1                       ((uint32_t)0x00000002)          /*!< Pin iev, bit 1 */
#define GPIO_IEV_BIT2                       ((uint32_t)0x00000004)          /*!< Pin iev, bit 2 */
#define GPIO_IEV_BIT3                       ((uint32_t)0x00000008)          /*!< Pin iev, bit 3 */
#define GPIO_IEV_BIT4                       ((uint32_t)0x00000010)          /*!< Pin iev, bit 4 */
#define GPIO_IEV_BIT5                       ((uint32_t)0x00000020)          /*!< Pin iev, bit 5 */
#define GPIO_IEV_BIT6                       ((uint32_t)0x00000040)          /*!< Pin iev, bit 6 */
#define GPIO_IEV_BIT7                       ((uint32_t)0x00000080)          /*!< Pin iev, bit 7 */
#define GPIO_IEV_BIT8                       ((uint32_t)0x00000100)          /*!< Pin iev, bit 8 */
#define GPIO_IEV_BIT9                       ((uint32_t)0x00000200)          /*!< Pin iev, bit 9 */
#define GPIO_IEV_BIT10                      ((uint32_t)0x00000400)          /*!< Pin iev, bit 10 */
#define GPIO_IEV_BIT11                      ((uint32_t)0x00000800)          /*!< Pin iev, bit 11 */
#define GPIO_IEV_BIT12                      ((uint32_t)0x00001000)          /*!< Pin iev, bit 12 */
#define GPIO_IEV_BIT13                      ((uint32_t)0x00002000)          /*!< Pin iev, bit 13 */
#define GPIO_IEV_BIT14                      ((uint32_t)0x00004000)          /*!< Pin iev, bit 14 */
#define GPIO_IEV_BIT15                      ((uint32_t)0x00008000)          /*!< Pin iev, bit 15 */
#define GPIO_IEV_BIT16                      ((uint32_t)0x00010000)          /*!< Pin iev, bit 16 */
#define GPIO_IEV_BIT17                      ((uint32_t)0x00020000)          /*!< Pin iev, bit 17 */
#define GPIO_IEV_BIT18                      ((uint32_t)0x00040000)          /*!< Pin iev, bit 18 */
#define GPIO_IEV_BIT19                      ((uint32_t)0x00080000)          /*!< Pin iev, bit 19 */
#define GPIO_IEV_BIT20                      ((uint32_t)0x00100000)          /*!< Pin iev, bit 20 */
#define GPIO_IEV_BIT21                      ((uint32_t)0x00200000)          /*!< Pin iev, bit 21 */
#define GPIO_IEV_BIT22                      ((uint32_t)0x00400000)          /*!< Pin iev, bit 22 */
#define GPIO_IEV_BIT23                      ((uint32_t)0x00800000)          /*!< Pin iev, bit 23 */
#define GPIO_IEV_BIT24                      ((uint32_t)0x01000000)          /*!< Pin iev, bit 24 */
#define GPIO_IEV_BIT25                      ((uint32_t)0x02000000)          /*!< Pin iev, bit 25 */
#define GPIO_IEV_BIT26                      ((uint32_t)0x04000000)          /*!< Pin iev, bit 26 */
#define GPIO_IEV_BIT27                      ((uint32_t)0x08000000)          /*!< Pin iev, bit 27 */
#define GPIO_IEV_BIT28                      ((uint32_t)0x10000000)          /*!< Pin iev, bit 28 */
#define GPIO_IEV_BIT29                      ((uint32_t)0x20000000)          /*!< Pin iev, bit 29 */
#define GPIO_IEV_BIT30                      ((uint32_t)0x40000000)          /*!< Pin iev, bit 30 */
#define GPIO_IEV_BIT31                      ((uint32_t)0x80000000)          /*!< Pin iev, bit 31 */

/*******************  Bit definition for GPIO_IE register  *******************/
#define GPIO_IE_BIT0                        ((uint32_t)0x00000001)          /*!< Pin ie, bit 0 */
#define GPIO_IE_BIT1                        ((uint32_t)0x00000002)          /*!< Pin ie, bit 1 */
#define GPIO_IE_BIT2                        ((uint32_t)0x00000004)          /*!< Pin ie, bit 2 */
#define GPIO_IE_BIT3                        ((uint32_t)0x00000008)          /*!< Pin ie, bit 3 */
#define GPIO_IE_BIT4                        ((uint32_t)0x00000010)          /*!< Pin ie, bit 4 */
#define GPIO_IE_BIT5                        ((uint32_t)0x00000020)          /*!< Pin ie, bit 5 */
#define GPIO_IE_BIT6                        ((uint32_t)0x00000040)          /*!< Pin ie, bit 6 */
#define GPIO_IE_BIT7                        ((uint32_t)0x00000080)          /*!< Pin ie, bit 7 */
#define GPIO_IE_BIT8                        ((uint32_t)0x00000100)          /*!< Pin ie, bit 8 */
#define GPIO_IE_BIT9                        ((uint32_t)0x00000200)          /*!< Pin ie, bit 9 */
#define GPIO_IE_BIT10                       ((uint32_t)0x00000400)          /*!< Pin ie, bit 10 */
#define GPIO_IE_BIT11                       ((uint32_t)0x00000800)          /*!< Pin ie, bit 11 */
#define GPIO_IE_BIT12                       ((uint32_t)0x00001000)          /*!< Pin ie, bit 12 */
#define GPIO_IE_BIT13                       ((uint32_t)0x00002000)          /*!< Pin ie, bit 13 */
#define GPIO_IE_BIT14                       ((uint32_t)0x00004000)          /*!< Pin ie, bit 14 */
#define GPIO_IE_BIT15                       ((uint32_t)0x00008000)          /*!< Pin ie, bit 15 */
#define GPIO_IE_BIT16                       ((uint32_t)0x00010000)          /*!< Pin ie, bit 16 */
#define GPIO_IE_BIT17                       ((uint32_t)0x00020000)          /*!< Pin ie, bit 17 */
#define GPIO_IE_BIT18                       ((uint32_t)0x00040000)          /*!< Pin ie, bit 18 */
#define GPIO_IE_BIT19                       ((uint32_t)0x00080000)          /*!< Pin ie, bit 19 */
#define GPIO_IE_BIT20                       ((uint32_t)0x00100000)          /*!< Pin ie, bit 20 */
#define GPIO_IE_BIT21                       ((uint32_t)0x00200000)          /*!< Pin ie, bit 21 */
#define GPIO_IE_BIT22                       ((uint32_t)0x00400000)          /*!< Pin ie, bit 22 */
#define GPIO_IE_BIT23                       ((uint32_t)0x00800000)          /*!< Pin ie, bit 23 */
#define GPIO_IE_BIT24                       ((uint32_t)0x01000000)          /*!< Pin ie, bit 24 */
#define GPIO_IE_BIT25                       ((uint32_t)0x02000000)          /*!< Pin ie, bit 25 */
#define GPIO_IE_BIT26                       ((uint32_t)0x04000000)          /*!< Pin ie, bit 26 */
#define GPIO_IE_BIT27                       ((uint32_t)0x08000000)          /*!< Pin ie, bit 27 */
#define GPIO_IE_BIT28                       ((uint32_t)0x10000000)          /*!< Pin ie, bit 28 */
#define GPIO_IE_BIT29                       ((uint32_t)0x20000000)          /*!< Pin ie, bit 29 */
#define GPIO_IE_BIT30                       ((uint32_t)0x40000000)          /*!< Pin ie, bit 30 */
#define GPIO_IE_BIT31                       ((uint32_t)0x80000000)          /*!< Pin ie, bit 31 */

/*******************  Bit definition for GPIO_IRS register  *******************/
#define GPIO_IRS_BIT0                       ((uint32_t)0x00000001)          /*!< Pin irs, bit 0 */
#define GPIO_IRS_BIT1                       ((uint32_t)0x00000002)          /*!< Pin irs, bit 1 */
#define GPIO_IRS_BIT2                       ((uint32_t)0x00000004)          /*!< Pin irs, bit 2 */
#define GPIO_IRS_BIT3                       ((uint32_t)0x00000008)          /*!< Pin irs, bit 3 */
#define GPIO_IRS_BIT4                       ((uint32_t)0x00000010)          /*!< Pin irs, bit 4 */
#define GPIO_IRS_BIT5                       ((uint32_t)0x00000020)          /*!< Pin irs, bit 5 */
#define GPIO_IRS_BIT6                       ((uint32_t)0x00000040)          /*!< Pin irs, bit 6 */
#define GPIO_IRS_BIT7                       ((uint32_t)0x00000080)          /*!< Pin irs, bit 7 */
#define GPIO_IRS_BIT8                       ((uint32_t)0x00000100)          /*!< Pin irs, bit 8 */
#define GPIO_IRS_BIT9                       ((uint32_t)0x00000200)          /*!< Pin irs, bit 9 */
#define GPIO_IRS_BIT10                      ((uint32_t)0x00000400)          /*!< Pin irs, bit 10 */
#define GPIO_IRS_BIT11                      ((uint32_t)0x00000800)          /*!< Pin irs, bit 11 */
#define GPIO_IRS_BIT12                      ((uint32_t)0x00001000)          /*!< Pin irs, bit 12 */
#define GPIO_IRS_BIT13                      ((uint32_t)0x00002000)          /*!< Pin irs, bit 13 */
#define GPIO_IRS_BIT14                      ((uint32_t)0x00004000)          /*!< Pin irs, bit 14 */
#define GPIO_IRS_BIT15                      ((uint32_t)0x00008000)          /*!< Pin irs, bit 15 */
#define GPIO_IRS_BIT16                      ((uint32_t)0x00010000)          /*!< Pin irs, bit 16 */
#define GPIO_IRS_BIT17                      ((uint32_t)0x00020000)          /*!< Pin irs, bit 17 */
#define GPIO_IRS_BIT18                      ((uint32_t)0x00040000)          /*!< Pin irs, bit 18 */
#define GPIO_IRS_BIT19                      ((uint32_t)0x00080000)          /*!< Pin irs, bit 19 */
#define GPIO_IRS_BIT20                      ((uint32_t)0x00100000)          /*!< Pin irs, bit 20 */
#define GPIO_IRS_BIT21                      ((uint32_t)0x00200000)          /*!< Pin irs, bit 21 */
#define GPIO_IRS_BIT22                      ((uint32_t)0x00400000)          /*!< Pin irs, bit 22 */
#define GPIO_IRS_BIT23                      ((uint32_t)0x00800000)          /*!< Pin irs, bit 23 */
#define GPIO_IRS_BIT24                      ((uint32_t)0x01000000)          /*!< Pin irs, bit 24 */
#define GPIO_IRS_BIT25                      ((uint32_t)0x02000000)          /*!< Pin irs, bit 25 */
#define GPIO_IRS_BIT26                      ((uint32_t)0x04000000)          /*!< Pin irs, bit 26 */
#define GPIO_IRS_BIT27                      ((uint32_t)0x08000000)          /*!< Pin irs, bit 27 */
#define GPIO_IRS_BIT28                      ((uint32_t)0x10000000)          /*!< Pin irs, bit 28 */
#define GPIO_IRS_BIT29                      ((uint32_t)0x20000000)          /*!< Pin irs, bit 29 */
#define GPIO_IRS_BIT30                      ((uint32_t)0x40000000)          /*!< Pin irs, bit 30 */
#define GPIO_IRS_BIT31                      ((uint32_t)0x80000000)          /*!< Pin irs, bit 31 */

/*******************  Bit definition for GPIO_MIS register  *******************/
#define GPIO_MIS_BIT0                       ((uint32_t)0x00000001)          /*!< Pin mis, bit 0 */
#define GPIO_MIS_BIT1                       ((uint32_t)0x00000002)          /*!< Pin mis, bit 1 */
#define GPIO_MIS_BIT2                       ((uint32_t)0x00000004)          /*!< Pin mis, bit 2 */
#define GPIO_MIS_BIT3                       ((uint32_t)0x00000008)          /*!< Pin mis, bit 3 */
#define GPIO_MIS_BIT4                       ((uint32_t)0x00000010)          /*!< Pin mis, bit 4 */
#define GPIO_MIS_BIT5                       ((uint32_t)0x00000020)          /*!< Pin mis, bit 5 */
#define GPIO_MIS_BIT6                       ((uint32_t)0x00000040)          /*!< Pin mis, bit 6 */
#define GPIO_MIS_BIT7                       ((uint32_t)0x00000080)          /*!< Pin mis, bit 7 */
#define GPIO_MIS_BIT8                       ((uint32_t)0x00000100)          /*!< Pin mis, bit 8 */
#define GPIO_MIS_BIT9                       ((uint32_t)0x00000200)          /*!< Pin mis, bit 9 */
#define GPIO_MIS_BIT10                      ((uint32_t)0x00000400)          /*!< Pin mis, bit 10 */
#define GPIO_MIS_BIT11                      ((uint32_t)0x00000800)          /*!< Pin mis, bit 11 */
#define GPIO_MIS_BIT12                      ((uint32_t)0x00001000)          /*!< Pin mis, bit 12 */
#define GPIO_MIS_BIT13                      ((uint32_t)0x00002000)          /*!< Pin mis, bit 13 */
#define GPIO_MIS_BIT14                      ((uint32_t)0x00004000)          /*!< Pin mis, bit 14 */
#define GPIO_MIS_BIT15                      ((uint32_t)0x00008000)          /*!< Pin mis, bit 15 */
#define GPIO_MIS_BIT16                      ((uint32_t)0x00010000)          /*!< Pin mis, bit 16 */
#define GPIO_MIS_BIT17                      ((uint32_t)0x00020000)          /*!< Pin mis, bit 17 */
#define GPIO_MIS_BIT18                      ((uint32_t)0x00040000)          /*!< Pin mis, bit 18 */
#define GPIO_MIS_BIT19                      ((uint32_t)0x00080000)          /*!< Pin mis, bit 19 */
#define GPIO_MIS_BIT20                      ((uint32_t)0x00100000)          /*!< Pin mis, bit 20 */
#define GPIO_MIS_BIT21                      ((uint32_t)0x00200000)          /*!< Pin mis, bit 21 */
#define GPIO_MIS_BIT22                      ((uint32_t)0x00400000)          /*!< Pin mis, bit 22 */
#define GPIO_MIS_BIT23                      ((uint32_t)0x00800000)          /*!< Pin mis, bit 23 */
#define GPIO_MIS_BIT24                      ((uint32_t)0x01000000)          /*!< Pin mis, bit 24 */
#define GPIO_MIS_BIT25                      ((uint32_t)0x02000000)          /*!< Pin mis, bit 25 */
#define GPIO_MIS_BIT26                      ((uint32_t)0x04000000)          /*!< Pin mis, bit 26 */
#define GPIO_MIS_BIT27                      ((uint32_t)0x08000000)          /*!< Pin mis, bit 27 */
#define GPIO_MIS_BIT28                      ((uint32_t)0x10000000)          /*!< Pin mis, bit 28 */
#define GPIO_MIS_BIT29                      ((uint32_t)0x20000000)          /*!< Pin mis, bit 29 */
#define GPIO_MIS_BIT30                      ((uint32_t)0x40000000)          /*!< Pin mis, bit 30 */
#define GPIO_MIS_BIT31                      ((uint32_t)0x80000000)          /*!< Pin mis, bit 31 */

/*******************  Bit definition for GPIO_IC register  *******************/
#define GPIO_IC_BIT0                        ((uint32_t)0x00000001)          /*!< Pin ic, bit 0 */
#define GPIO_IC_BIT1                        ((uint32_t)0x00000002)          /*!< Pin ic, bit 1 */
#define GPIO_IC_BIT2                        ((uint32_t)0x00000004)          /*!< Pin ic, bit 2 */
#define GPIO_IC_BIT3                        ((uint32_t)0x00000008)          /*!< Pin ic, bit 3 */
#define GPIO_IC_BIT4                        ((uint32_t)0x00000010)          /*!< Pin ic, bit 4 */
#define GPIO_IC_BIT5                        ((uint32_t)0x00000020)          /*!< Pin ic, bit 5 */
#define GPIO_IC_BIT6                        ((uint32_t)0x00000040)          /*!< Pin ic, bit 6 */
#define GPIO_IC_BIT7                        ((uint32_t)0x00000080)          /*!< Pin ic, bit 7 */
#define GPIO_IC_BIT8                        ((uint32_t)0x00000100)          /*!< Pin ic, bit 8 */
#define GPIO_IC_BIT9                        ((uint32_t)0x00000200)          /*!< Pin ic, bit 9 */
#define GPIO_IC_BIT10                       ((uint32_t)0x00000400)          /*!< Pin ic, bit 10 */
#define GPIO_IC_BIT11                       ((uint32_t)0x00000800)          /*!< Pin ic, bit 11 */
#define GPIO_IC_BIT12                       ((uint32_t)0x00001000)          /*!< Pin ic, bit 12 */
#define GPIO_IC_BIT13                       ((uint32_t)0x00002000)          /*!< Pin ic, bit 13 */
#define GPIO_IC_BIT14                       ((uint32_t)0x00004000)          /*!< Pin ic, bit 14 */
#define GPIO_IC_BIT15                       ((uint32_t)0x00008000)          /*!< Pin ic, bit 15 */
#define GPIO_IC_BIT16                       ((uint32_t)0x00010000)          /*!< Pin ic, bit 16 */
#define GPIO_IC_BIT17                       ((uint32_t)0x00020000)          /*!< Pin ic, bit 17 */
#define GPIO_IC_BIT18                       ((uint32_t)0x00040000)          /*!< Pin ic, bit 18 */
#define GPIO_IC_BIT19                       ((uint32_t)0x00080000)          /*!< Pin ic, bit 19 */
#define GPIO_IC_BIT20                       ((uint32_t)0x00100000)          /*!< Pin ic, bit 20 */
#define GPIO_IC_BIT21                       ((uint32_t)0x00200000)          /*!< Pin ic, bit 21 */
#define GPIO_IC_BIT22                       ((uint32_t)0x00400000)          /*!< Pin ic, bit 22 */
#define GPIO_IC_BIT23                       ((uint32_t)0x00800000)          /*!< Pin ic, bit 23 */
#define GPIO_IC_BIT24                       ((uint32_t)0x01000000)          /*!< Pin ic, bit 24 */
#define GPIO_IC_BIT25                       ((uint32_t)0x02000000)          /*!< Pin ic, bit 25 */
#define GPIO_IC_BIT26                       ((uint32_t)0x04000000)          /*!< Pin ic, bit 26 */
#define GPIO_IC_BIT27                       ((uint32_t)0x08000000)          /*!< Pin ic, bit 27 */
#define GPIO_IC_BIT28                       ((uint32_t)0x10000000)          /*!< Pin ic, bit 28 */
#define GPIO_IC_BIT29                       ((uint32_t)0x20000000)          /*!< Pin ic, bit 29 */
#define GPIO_IC_BIT30                       ((uint32_t)0x40000000)          /*!< Pin ic, bit 30 */
#define GPIO_IC_BIT31                       ((uint32_t)0x80000000)          /*!< Pin ic, bit 31 */

/*******************  Bit definition for GPIO_DATAMASK register  *******************/
#define GPIO_DATAMASK_BIT0                  ((uint32_t)0x00000001)          /*!< Data mask, bit 0 */
#define GPIO_DATAMASK_BIT1                  ((uint32_t)0x00000002)          /*!< Data mask, bit 1 */
#define GPIO_DATAMASK_BIT2                  ((uint32_t)0x00000004)          /*!< Data mask, bit 2 */
#define GPIO_DATAMASK_BIT3                  ((uint32_t)0x00000008)          /*!< Data mask, bit 3 */
#define GPIO_DATAMASK_BIT4                  ((uint32_t)0x00000010)          /*!< Data mask, bit 4 */
#define GPIO_DATAMASK_BIT5                  ((uint32_t)0x00000020)          /*!< Data mask, bit 5 */
#define GPIO_DATAMASK_BIT6                  ((uint32_t)0x00000040)          /*!< Data mask, bit 6 */
#define GPIO_DATAMASK_BIT7                  ((uint32_t)0x00000080)          /*!< Data mask, bit 7 */
#define GPIO_DATAMASK_BIT8                  ((uint32_t)0x00000100)          /*!< Data mask, bit 8 */
#define GPIO_DATAMASK_BIT9                  ((uint32_t)0x00000200)          /*!< Data mask, bit 9 */
#define GPIO_DATAMASK_BIT10                 ((uint32_t)0x00000400)          /*!< Data mask, bit 10 */
#define GPIO_DATAMASK_BIT11                 ((uint32_t)0x00000800)          /*!< Data mask, bit 11 */
#define GPIO_DATAMASK_BIT12                 ((uint32_t)0x00001000)          /*!< Data mask, bit 12 */
#define GPIO_DATAMASK_BIT13                 ((uint32_t)0x00002000)          /*!< Data mask, bit 13 */
#define GPIO_DATAMASK_BIT14                 ((uint32_t)0x00004000)          /*!< Data mask, bit 14 */
#define GPIO_DATAMASK_BIT15                 ((uint32_t)0x00008000)          /*!< Data mask, bit 15 */
#define GPIO_DATAMASK_BIT16                 ((uint32_t)0x00010000)          /*!< Data mask, bit 16 */
#define GPIO_DATAMASK_BIT17                 ((uint32_t)0x00020000)          /*!< Data mask, bit 17 */
#define GPIO_DATAMASK_BIT18                 ((uint32_t)0x00040000)          /*!< Data mask, bit 18 */
#define GPIO_DATAMASK_BIT19                 ((uint32_t)0x00080000)          /*!< Data mask, bit 19 */
#define GPIO_DATAMASK_BIT20                 ((uint32_t)0x00100000)          /*!< Data mask, bit 20 */
#define GPIO_DATAMASK_BIT21                 ((uint32_t)0x00200000)          /*!< Data mask, bit 21 */
#define GPIO_DATAMASK_BIT22                 ((uint32_t)0x00400000)          /*!< Data mask, bit 22 */
#define GPIO_DATAMASK_BIT23                 ((uint32_t)0x00800000)          /*!< Data mask, bit 23 */
#define GPIO_DATAMASK_BIT24                 ((uint32_t)0x01000000)          /*!< Data mask, bit 24 */
#define GPIO_DATAMASK_BIT25                 ((uint32_t)0x02000000)          /*!< Data mask, bit 25 */
#define GPIO_DATAMASK_BIT26                 ((uint32_t)0x04000000)          /*!< Data mask, bit 26 */
#define GPIO_DATAMASK_BIT27                 ((uint32_t)0x08000000)          /*!< Data mask, bit 27 */
#define GPIO_DATAMASK_BIT28                 ((uint32_t)0x10000000)          /*!< Data mask, bit 28 */
#define GPIO_DATAMASK_BIT29                 ((uint32_t)0x20000000)          /*!< Data mask, bit 29 */
#define GPIO_DATAMASK_BIT30                 ((uint32_t)0x40000000)          /*!< Data mask, bit 30 */
#define GPIO_DATAMASK_BIT31                 ((uint32_t)0x80000000)          /*!< Data mask, bit 31 */

/*******************  Bit definition for GPIO_FILTER register  ****************/
#define GPIO_FILTER_ENABLE                  ((uint32_t)0x80000000)          /*!< Enable GPIO filter */
#define GPIO_FILTER_CYCLES_MASK             ((uint32_t)0x000000FF)          /*!< GPIO filter mask */

/********************************GPIO MACRO************************************/
#define P00_IN()                            (GPIOBANK0->DIR_CLR = GPIO_DIR_BIT0)
#define P01_IN()                            (GPIOBANK0->DIR_CLR = GPIO_DIR_BIT1)
#define P02_IN()                            (GPIOBANK0->DIR_CLR = GPIO_DIR_BIT2)
#define P03_IN()                            (GPIOBANK0->DIR_CLR = GPIO_DIR_BIT3)
#define P04_IN()                            (GPIOBANK0->DIR_CLR = GPIO_DIR_BIT4)
#define P05_IN()                            (GPIOBANK0->DIR_CLR = GPIO_DIR_BIT5)
#define P06_IN()                            (GPIOBANK0->DIR_CLR = GPIO_DIR_BIT6)
#define P07_IN()                            (GPIOBANK0->DIR_CLR = GPIO_DIR_BIT7)
#define P10_IN()                            (GPIOBANK0->DIR_CLR = GPIO_DIR_BIT8)
#define P11_IN()                            (GPIOBANK0->DIR_CLR = GPIO_DIR_BIT9)
#define P12_IN()                            (GPIOBANK0->DIR_CLR = GPIO_DIR_BIT10)
#define P13_IN()                            (GPIOBANK0->DIR_CLR = GPIO_DIR_BIT11)
#define P14_IN()                            (GPIOBANK0->DIR_CLR = GPIO_DIR_BIT12)
#define P15_IN()                            (GPIOBANK0->DIR_CLR = GPIO_DIR_BIT13)
#define P16_IN()                            (GPIOBANK0->DIR_CLR = GPIO_DIR_BIT14)
#define P17_IN()                            (GPIOBANK0->DIR_CLR = GPIO_DIR_BIT15)
#define P20_IN()                            (GPIOBANK0->DIR_CLR = GPIO_DIR_BIT16)
#define P21_IN()                            (GPIOBANK0->DIR_CLR = GPIO_DIR_BIT17)
#define P22_IN()                            (GPIOBANK0->DIR_CLR = GPIO_DIR_BIT18)
#define P23_IN()                            (GPIOBANK0->DIR_CLR = GPIO_DIR_BIT19)
#define P24_IN()                            (GPIOBANK0->DIR_CLR = GPIO_DIR_BIT20)
#define P25_IN()                            (GPIOBANK0->DIR_CLR = GPIO_DIR_BIT21)
#define P26_IN()                            (GPIOBANK0->DIR_CLR = GPIO_DIR_BIT22)
#define P27_IN()                            (GPIOBANK0->DIR_CLR = GPIO_DIR_BIT23)

#define P00_OUT()                           (GPIOBANK0->DIR_SET = GPIO_DIR_BIT0)
#define P01_OUT()                           (GPIOBANK0->DIR_SET = GPIO_DIR_BIT1)
#define P02_OUT()                           (GPIOBANK0->DIR_SET = GPIO_DIR_BIT2)
#define P03_OUT()                           (GPIOBANK0->DIR_SET = GPIO_DIR_BIT3)
#define P04_OUT()                           (GPIOBANK0->DIR_SET = GPIO_DIR_BIT4)
#define P05_OUT()                           (GPIOBANK0->DIR_SET = GPIO_DIR_BIT5)
#define P06_OUT()                           (GPIOBANK0->DIR_SET = GPIO_DIR_BIT6)
#define P07_OUT()                           (GPIOBANK0->DIR_SET = GPIO_DIR_BIT7)
#define P10_OUT()                           (GPIOBANK0->DIR_SET = GPIO_DIR_BIT8)
#define P11_OUT()                           (GPIOBANK0->DIR_SET = GPIO_DIR_BIT9)
#define P12_OUT()                           (GPIOBANK0->DIR_SET = GPIO_DIR_BIT10)
#define P13_OUT()                           (GPIOBANK0->DIR_SET = GPIO_DIR_BIT11)
#define P14_OUT()                           (GPIOBANK0->DIR_SET = GPIO_DIR_BIT12)
#define P15_OUT()                           (GPIOBANK0->DIR_SET = GPIO_DIR_BIT13)
#define P16_OUT()                           (GPIOBANK0->DIR_SET = GPIO_DIR_BIT14)
#define P17_OUT()                           (GPIOBANK0->DIR_SET = GPIO_DIR_BIT15)
#define P20_OUT()                           (GPIOBANK0->DIR_SET = GPIO_DIR_BIT16)
#define P21_OUT()                           (GPIOBANK0->DIR_SET = GPIO_DIR_BIT17)
#define P22_OUT()                           (GPIOBANK0->DIR_SET = GPIO_DIR_BIT18)
#define P23_OUT()                           (GPIOBANK0->DIR_SET = GPIO_DIR_BIT19)
#define P24_OUT()                           (GPIOBANK0->DIR_SET = GPIO_DIR_BIT20)
#define P25_OUT()                           (GPIOBANK0->DIR_SET = GPIO_DIR_BIT21)
#define P26_OUT()                           (GPIOBANK0->DIR_SET = GPIO_DIR_BIT22)
#define P27_OUT()                           (GPIOBANK0->DIR_SET = GPIO_DIR_BIT23)

#define P00_OUT_H()                         (GPIODATA0->DT_SET = GPIO_DATA_BIT0)
#define P01_OUT_H()                         (GPIODATA0->DT_SET = GPIO_DATA_BIT1)
#define P02_OUT_H()                         (GPIODATA0->DT_SET = GPIO_DATA_BIT2)
#define P03_OUT_H()                         (GPIODATA0->DT_SET = GPIO_DATA_BIT3)
#define P04_OUT_H()                         (GPIODATA0->DT_SET = GPIO_DATA_BIT4)
#define P05_OUT_H()                         (GPIODATA0->DT_SET = GPIO_DATA_BIT5)
#define P06_OUT_H()                         (GPIODATA0->DT_SET = GPIO_DATA_BIT6)
#define P07_OUT_H()                         (GPIODATA0->DT_SET = GPIO_DATA_BIT7)
#define P10_OUT_H()                         (GPIODATA0->DT_SET = GPIO_DATA_BIT8)
#define P11_OUT_H()                         (GPIODATA0->DT_SET = GPIO_DATA_BIT9)
#define P12_OUT_H()                         (GPIODATA0->DT_SET = GPIO_DATA_BIT10)
#define P13_OUT_H()                         (GPIODATA0->DT_SET = GPIO_DATA_BIT11)
#define P14_OUT_H()                         (GPIODATA0->DT_SET = GPIO_DATA_BIT12)
#define P15_OUT_H()                         (GPIODATA0->DT_SET = GPIO_DATA_BIT13)
#define P16_OUT_H()                         (GPIODATA0->DT_SET = GPIO_DATA_BIT14)
#define P17_OUT_H()                         (GPIODATA0->DT_SET = GPIO_DATA_BIT15)
#define P20_OUT_H()                         (GPIODATA0->DT_SET = GPIO_DATA_BIT16)
#define P21_OUT_H()                         (GPIODATA0->DT_SET = GPIO_DATA_BIT17)
#define P22_OUT_H()                         (GPIODATA0->DT_SET = GPIO_DATA_BIT18)
#define P23_OUT_H()                         (GPIODATA0->DT_SET = GPIO_DATA_BIT19)
#define P24_OUT_H()                         (GPIODATA0->DT_SET = GPIO_DATA_BIT20)
#define P25_OUT_H()                         (GPIODATA0->DT_SET = GPIO_DATA_BIT21)
#define P26_OUT_H()                         (GPIODATA0->DT_SET = GPIO_DATA_BIT22)
#define P27_OUT_H()                         (GPIODATA0->DT_SET = GPIO_DATA_BIT23)

#define P00_OUT_L()                         (GPIODATA0->DT_CLR = GPIO_DATA_BIT0)
#define P01_OUT_L()                         (GPIODATA0->DT_CLR = GPIO_DATA_BIT1)
#define P02_OUT_L()                         (GPIODATA0->DT_CLR = GPIO_DATA_BIT2)
#define P03_OUT_L()                         (GPIODATA0->DT_CLR = GPIO_DATA_BIT3)
#define P04_OUT_L()                         (GPIODATA0->DT_CLR = GPIO_DATA_BIT4)
#define P05_OUT_L()                         (GPIODATA0->DT_CLR = GPIO_DATA_BIT5)
#define P06_OUT_L()                         (GPIODATA0->DT_CLR = GPIO_DATA_BIT6)
#define P07_OUT_L()                         (GPIODATA0->DT_CLR = GPIO_DATA_BIT7)
#define P10_OUT_L()                         (GPIODATA0->DT_CLR = GPIO_DATA_BIT8)
#define P11_OUT_L()                         (GPIODATA0->DT_CLR = GPIO_DATA_BIT9)
#define P12_OUT_L()                         (GPIODATA0->DT_CLR = GPIO_DATA_BIT10)
#define P13_OUT_L()                         (GPIODATA0->DT_CLR = GPIO_DATA_BIT11)
#define P14_OUT_L()                         (GPIODATA0->DT_CLR = GPIO_DATA_BIT12)
#define P15_OUT_L()                         (GPIODATA0->DT_CLR = GPIO_DATA_BIT13)
#define P16_OUT_L()                         (GPIODATA0->DT_CLR = GPIO_DATA_BIT14)
#define P17_OUT_L()                         (GPIODATA0->DT_CLR = GPIO_DATA_BIT15)
#define P20_OUT_L()                         (GPIODATA0->DT_CLR = GPIO_DATA_BIT16)
#define P21_OUT_L()                         (GPIODATA0->DT_CLR = GPIO_DATA_BIT17)
#define P22_OUT_L()                         (GPIODATA0->DT_CLR = GPIO_DATA_BIT18)
#define P23_OUT_L()                         (GPIODATA0->DT_CLR = GPIO_DATA_BIT19)
#define P24_OUT_L()                         (GPIODATA0->DT_CLR = GPIO_DATA_BIT20)
#define P25_OUT_L()                         (GPIODATA0->DT_CLR = GPIO_DATA_BIT21)
#define P26_OUT_L()                         (GPIODATA0->DT_CLR = GPIO_DATA_BIT22)
#define P27_OUT_L()                         (GPIODATA0->DT_CLR = GPIO_DATA_BIT23)

#define P00_OUT_TOG()                       (GPIODATA0->DT_TOG = GPIO_DATA_BIT0)
#define P01_OUT_TOG()                       (GPIODATA0->DT_TOG = GPIO_DATA_BIT1)
#define P02_OUT_TOG()                       (GPIODATA0->DT_TOG = GPIO_DATA_BIT2)
#define P03_OUT_TOG()                       (GPIODATA0->DT_TOG = GPIO_DATA_BIT3)
#define P04_OUT_TOG()                       (GPIODATA0->DT_TOG = GPIO_DATA_BIT4)
#define P05_OUT_TOG()                       (GPIODATA0->DT_TOG = GPIO_DATA_BIT5)
#define P06_OUT_TOG()                       (GPIODATA0->DT_TOG = GPIO_DATA_BIT6)
#define P07_OUT_TOG()                       (GPIODATA0->DT_TOG = GPIO_DATA_BIT7)
#define P10_OUT_TOG()                       (GPIODATA0->DT_TOG = GPIO_DATA_BIT8)
#define P11_OUT_TOG()                       (GPIODATA0->DT_TOG = GPIO_DATA_BIT9)
#define P12_OUT_TOG()                       (GPIODATA0->DT_TOG = GPIO_DATA_BIT10)
#define P13_OUT_TOG()                       (GPIODATA0->DT_TOG = GPIO_DATA_BIT11)
#define P14_OUT_TOG()                       (GPIODATA0->DT_TOG = GPIO_DATA_BIT12)
#define P15_OUT_TOG()                       (GPIODATA0->DT_TOG = GPIO_DATA_BIT13)
#define P16_OUT_TOG()                       (GPIODATA0->DT_TOG = GPIO_DATA_BIT14)
#define P17_OUT_TOG()                       (GPIODATA0->DT_TOG = GPIO_DATA_BIT15)
#define P20_OUT_TOG()                       (GPIODATA0->DT_TOG = GPIO_DATA_BIT16)
#define P21_OUT_TOG()                       (GPIODATA0->DT_TOG = GPIO_DATA_BIT17)
#define P22_OUT_TOG()                       (GPIODATA0->DT_TOG = GPIO_DATA_BIT18)
#define P23_OUT_TOG()                       (GPIODATA0->DT_TOG = GPIO_DATA_BIT19)
#define P24_OUT_TOG()                       (GPIODATA0->DT_TOG = GPIO_DATA_BIT20)
#define P25_OUT_TOG()                       (GPIODATA0->DT_TOG = GPIO_DATA_BIT21)
#define P26_OUT_TOG()                       (GPIODATA0->DT_TOG = GPIO_DATA_BIT22)
#define P27_OUT_TOG()                       (GPIODATA0->DT_TOG = GPIO_DATA_BIT23)

#define P00_PU()                            (GPIOIOCON->GPIO0_0 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO0_0 |= IOCON_MODE_2)
#define P01_PU()                            (GPIOIOCON->GPIO0_1 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO0_1 |= IOCON_MODE_2)
#define P02_PU()                            (GPIOIOCON->GPIO0_2 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO0_2 |= IOCON_MODE_2)
#define P03_PU()                            (GPIOIOCON->GPIO0_3 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO0_3 |= IOCON_MODE_2)
#define P04_PU()                            (GPIOIOCON->GPIO0_4 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO0_4 |= IOCON_MODE_2)
#define P05_PU()                            (GPIOIOCON->GPIO0_5 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO0_5 |= IOCON_MODE_2)
#define P06_PU()                            (GPIOIOCON->GPIO0_6 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO0_6 |= IOCON_MODE_2)
#define P07_PU()                            (GPIOIOCON->GPIO0_7 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO0_7 |= IOCON_MODE_2)
#define P10_PU()                            (GPIOIOCON->GPIO1_0 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO1_0 |= IOCON_MODE_2)
#define P11_PU()                            (GPIOIOCON->GPIO1_1 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO1_1 |= IOCON_MODE_2)
#define P12_PU()                            (GPIOIOCON->GPIO1_2 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO1_2 |= IOCON_MODE_2)
#define P13_PU()                            (GPIOIOCON->GPIO1_3 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO1_3 |= IOCON_MODE_2)
#define P14_PU()                            (GPIOIOCON->GPIO1_4 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO1_4 |= IOCON_MODE_2)
#define P15_PU()                            (GPIOIOCON->GPIO1_5 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO1_5 |= IOCON_MODE_2)
#define P16_PU()                            (GPIOIOCON->GPIO1_6 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO1_6 |= IOCON_MODE_2)
#define P17_PU()                            (GPIOIOCON->GPIO1_7 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO1_7 |= IOCON_MODE_2)
#define P20_PU()                            (GPIOIOCON->GPIO2_0 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO2_0 |= IOCON_MODE_2)
#define P21_PU()                            (GPIOIOCON->GPIO2_1 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO2_1 |= IOCON_MODE_2)
#define P22_PU()                            (GPIOIOCON->GPIO2_2 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO2_2 |= IOCON_MODE_2)
#define P23_PU()                            (GPIOIOCON->GPIO2_3 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO2_3 |= IOCON_MODE_2)
#define P24_PU()                            (GPIOIOCON->GPIO2_4 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO2_4 |= IOCON_MODE_2)
#define P25_PU()                            (GPIOIOCON->GPIO2_5 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO2_5 |= IOCON_MODE_2)
#define P26_PU()                            (GPIOIOCON->GPIO2_6 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO2_6 |= IOCON_MODE_2)
#define P27_PU()                            (GPIOIOCON->GPIO2_7 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO2_7 |= IOCON_MODE_2)

#define P00_PD()                            (GPIOIOCON->GPIO0_0 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO0_0 |= IOCON_MODE_1)
#define P01_PD()                            (GPIOIOCON->GPIO0_1 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO0_1 |= IOCON_MODE_1)
#define P02_PD()                            (GPIOIOCON->GPIO0_2 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO0_2 |= IOCON_MODE_1)
#define P03_PD()                            (GPIOIOCON->GPIO0_3 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO0_3 |= IOCON_MODE_1)
#define P04_PD()                            (GPIOIOCON->GPIO0_4 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO0_4 |= IOCON_MODE_1)
#define P05_PD()                            (GPIOIOCON->GPIO0_5 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO0_5 |= IOCON_MODE_1)
#define P06_PD()                            (GPIOIOCON->GPIO0_6 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO0_6 |= IOCON_MODE_1)
#define P07_PD()                            (GPIOIOCON->GPIO0_7 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO0_7 |= IOCON_MODE_1)
#define P10_PD()                            (GPIOIOCON->GPIO1_0 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO1_0 |= IOCON_MODE_1)
#define P11_PD()                            (GPIOIOCON->GPIO1_1 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO1_1 |= IOCON_MODE_1)
#define P12_PD()                            (GPIOIOCON->GPIO1_2 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO1_2 |= IOCON_MODE_1)
#define P13_PD()                            (GPIOIOCON->GPIO1_3 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO1_3 |= IOCON_MODE_1)
#define P14_PD()                            (GPIOIOCON->GPIO1_4 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO1_4 |= IOCON_MODE_1)
#define P15_PD()                            (GPIOIOCON->GPIO1_5 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO1_5 |= IOCON_MODE_1)
#define P16_PD()                            (GPIOIOCON->GPIO1_6 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO1_6 |= IOCON_MODE_1)
#define P17_PD()                            (GPIOIOCON->GPIO1_7 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO1_7 |= IOCON_MODE_1)
#define P20_PD()                            (GPIOIOCON->GPIO2_0 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO2_0 |= IOCON_MODE_1)
#define P21_PD()                            (GPIOIOCON->GPIO2_1 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO2_1 |= IOCON_MODE_1)
#define P22_PD()                            (GPIOIOCON->GPIO2_2 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO2_2 |= IOCON_MODE_1)
#define P23_PD()                            (GPIOIOCON->GPIO2_3 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO2_3 |= IOCON_MODE_1)
#define P24_PD()                            (GPIOIOCON->GPIO2_4 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO2_4 |= IOCON_MODE_1)
#define P25_PD()                            (GPIOIOCON->GPIO2_5 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO2_5 |= IOCON_MODE_1)
#define P26_PD()                            (GPIOIOCON->GPIO2_6 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO2_6 |= IOCON_MODE_1)
#define P27_PD()                            (GPIOIOCON->GPIO2_7 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO2_7 |= IOCON_MODE_1)

#define P00_FLOAT()                         (GPIOIOCON->GPIO0_0 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO0_0 |= IOCON_MODE_0)
#define P01_FLOAT()                         (GPIOIOCON->GPIO0_1 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO0_1 |= IOCON_MODE_0)
#define P02_FLOAT()                         (GPIOIOCON->GPIO0_2 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO0_2 |= IOCON_MODE_0)
#define P03_FLOAT()                         (GPIOIOCON->GPIO0_3 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO0_3 |= IOCON_MODE_0)
#define P04_FLOAT()                         (GPIOIOCON->GPIO0_4 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO0_4 |= IOCON_MODE_0)
#define P05_FLOAT()                         (GPIOIOCON->GPIO0_5 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO0_5 |= IOCON_MODE_0)
#define P06_FLOAT()                         (GPIOIOCON->GPIO0_6 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO0_6 |= IOCON_MODE_0)
#define P07_FLOAT()                         (GPIOIOCON->GPIO0_7 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO0_7 |= IOCON_MODE_0)
#define P10_FLOAT()                         (GPIOIOCON->GPIO1_0 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO1_0 |= IOCON_MODE_0)
#define P11_FLOAT()                         (GPIOIOCON->GPIO1_1 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO1_1 |= IOCON_MODE_0)
#define P12_FLOAT()                         (GPIOIOCON->GPIO1_2 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO1_2 |= IOCON_MODE_0)
#define P13_FLOAT()                         (GPIOIOCON->GPIO1_3 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO1_3 |= IOCON_MODE_0)
#define P14_FLOAT()                         (GPIOIOCON->GPIO1_4 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO1_4 |= IOCON_MODE_0)
#define P15_FLOAT()                         (GPIOIOCON->GPIO1_5 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO1_5 |= IOCON_MODE_0)
#define P16_FLOAT()                         (GPIOIOCON->GPIO1_6 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO1_6 |= IOCON_MODE_0)
#define P17_FLOAT()                         (GPIOIOCON->GPIO1_7 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO1_7 |= IOCON_MODE_0)
#define P20_FLOAT()                         (GPIOIOCON->GPIO2_0 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO2_0 |= IOCON_MODE_0)
#define P21_FLOAT()                         (GPIOIOCON->GPIO2_1 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO2_1 |= IOCON_MODE_0)
#define P22_FLOAT()                         (GPIOIOCON->GPIO2_2 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO2_2 |= IOCON_MODE_0)
#define P23_FLOAT()                         (GPIOIOCON->GPIO2_3 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO2_3 |= IOCON_MODE_0)
#define P24_FLOAT()                         (GPIOIOCON->GPIO2_4 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO2_4 |= IOCON_MODE_0)
#define P25_FLOAT()                         (GPIOIOCON->GPIO2_5 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO2_5 |= IOCON_MODE_0)
#define P26_FLOAT()                         (GPIOIOCON->GPIO2_6 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO2_6 |= IOCON_MODE_0)
#define P27_FLOAT()                         (GPIOIOCON->GPIO2_7 &= ~IOCON_MODE_MASK); (GPIOIOCON->GPIO2_7 |= IOCON_MODE_0)


/******************************************************************************/
/*                                                                            */
/*                      Inter-integrated Circuit Interface                    */
/*                                                                            */
/******************************************************************************/
/*******************  Bit definition for I2C_ADDR register  ********************/
#define I2C_ADDR_SLA                        ((uint8_t)0xFE)                
#define I2C_ADDR_GCE                        ((uint8_t)0x01)                

/*******************  Bit definition for IC_XADDR register  *********************/
#define  I2C_XADDR                          ((uint8_t)0xFF)

/*******************  Bit definition for IC_CNTR register  *********************/
#define  I2C_CNTR_AAK                       ((uint8_t)0x04)
#define  I2C_CNTR_IFLG                      ((uint8_t)0x08)
#define  I2C_CNTR_STP                       ((uint8_t)0x10)
#define  I2C_CNTR_STA                       ((uint8_t)0x20)
#define  I2C_CNTR_ENAB                      ((uint8_t)0x40)
#define  I2C_CNTR_IEN                       ((uint8_t)0x80)

/*******************  Bit definition for IC_CCR register  ****************/
#define  I2C_CCR_N                          ((uint8_t)0x07)
#define  I2C_CCR_M                          ((uint8_t)0x78)


/******************************************************************************/
/*                                                                            */
/*                 Serial Peripheral Interface                                */
/*                                                                            */
/******************************************************************************/
/*******************  Bit definition for HW_SPI_CR0 register  *****************/
#define SPI_CR0_DSS_MASK                    ((uint32_t)0x0000000F)          /*!< Data size  */
#define SPI_CR0_DSS_4BIT                    ((uint32_t)0x00000003)          /*!< Data size 4BIT */
#define SPI_CR0_DSS_5BIT                    ((uint32_t)0x00000004)          /*!< Data size 5BIT */
#define SPI_CR0_DSS_6BIT                    ((uint32_t)0x00000005)          /*!< Data size 6BIT */
#define SPI_CR0_DSS_7BIT                    ((uint32_t)0x00000006)          /*!< Data size 7BIT */
#define SPI_CR0_DSS_8BIT                    ((uint32_t)0x00000007)          /*!< Data size 8BIT */
#define SPI_CR0_DSS_9BIT                    ((uint32_t)0x00000008)          /*!< Data size 9BIT */
#define SPI_CR0_DSS_10BIT                   ((uint32_t)0x00000009)          /*!< Data size 10BIT */
#define SPI_CR0_DSS_11BIT                   ((uint32_t)0x0000000A)          /*!< Data size 11BIT */
#define SPI_CR0_DSS_12BIT                   ((uint32_t)0x0000000B)          /*!< Data size 12BIT */
#define SPI_CR0_DSS_13BIT                   ((uint32_t)0x0000000C)          /*!< Data size 13BIT */
#define SPI_CR0_DSS_14BIT                   ((uint32_t)0x0000000D)          /*!< Data size 14BIT */
#define SPI_CR0_DSS_15BIT                   ((uint32_t)0x0000000E)          /*!< Data size 15BIT */
#define SPI_CR0_DSS_16BIT                   ((uint32_t)0x0000000F)          /*!< Data size 16BIT */
#define SPI_CR0_FRF_MASK                    ((uint32_t)0x00000030)          /*!< Frame format mask */
#define SPI_CR0_FRF_MOTOROLA_SPI            ((uint32_t)0x00000000)          /*!< Motorola spi frame format */
#define SPI_CR0_FRF_TI_SYNCHRONOUS          ((uint32_t)0x00000010)          /*!< Ti synchronous frame format */
#define SPI_CR0_FRF_NATIONAL MICROWIRE      ((uint32_t)0x00000020)          /*!< National microwire frame format */
#define SPI_CR0_SPO                         ((uint32_t)0x00000040)          /*!< Clkout polarity(applicable to Motorola SPI frame format only) */
#define SPI_CR0_SPH                         ((uint32_t)0x00000080)          /*!< Clkout phase(applicable to Motorola SPI frame format only) */
#define SPI_CR0_SCR_MASK                    ((uint32_t)0x0000FF00)          /*!< Serial clock rate */

/*******************  Bit definition for HW_SPI_CR1 register  *****************/
#define	SPI_CR1_LBM                         ((uint32_t)0x00000001)          /*!< Loop back mode */
#define	SPI_CR1_SSE                         ((uint32_t)0x00000002)          /*!< Synchronous serial port enable */
#define	SPI_CR1_MS                          ((uint32_t)0x00000004)          /*!< Master or slave mode select */
#define	SPI_CR1_SOD                         ((uint32_t)0x00000008)          /*!< Enable slave receive mode */

/*******************  Bit definition for HW_SPI_DR register  ******************/
#define	SPI_DR_MASK                         ((uint32_t)0x0000FFFF)          /*!< Transmit/Receive FIFO */

/*******************  Bit definition for HW_SPI_SR register  ******************/
#define	SPI_SR_TFE                          ((uint32_t)0x00000001)          /*!< Transmit FIFO empty */
#define	SPI_SR_TNF                          ((uint32_t)0x00000002)          /*!< Transmit FIFO not full */
#define	SPI_SR_RNE                          ((uint32_t)0x00000004)          /*!< Receive FIFO not empty */
#define	SPI_SR_RFF                          ((uint32_t)0x00000008)          /*!< Receive FIFO full */
#define	SPI_SR_BSY                          ((uint32_t)0x00000010)          /*!< Busy flag */

/*******************  Bit definition for HW_SPI_CPSR register  ****************/
#define	SPI_CPSR_CPSDV_MASK                 ((uint32_t)0x000000FF)          /*!< Clock prescale divisor mask */

/*******************  Bit definition for HW_SPI_IMSC register  ****************/
#define	SPI_IMSC_RORIM                      ((uint32_t)0x00000001)          /*!< Receive overrun interrupt mask */
#define	SPI_IMSC_RTIM                       ((uint32_t)0x00000002)          /*!< Receive timeout interrupt mask */
#define	SPI_IMSC_RXIM                       ((uint32_t)0x00000004)          /*!< Receive FIFO interrupt mask */
#define	SPI_IMSC_TXIM                       ((uint32_t)0x00000008)          /*!< Transmit FIFO interrupt mask */

/*******************  Bit definition for HW_SPI_RIS register  ****************/
#define	SPI_RIS_RORRIS                      ((uint32_t)0x00000001)          /*!< RORINTR interrupt */
#define	SPI_RIS_RTRIS                       ((uint32_t)0x00000002)          /*!< RTINTR interrupt */
#define	SPI_RIS_RXRIS                       ((uint32_t)0x00000004)          /*!< RXINTR interrupt */
#define	SPI_RIS_TXRIS                       ((uint32_t)0x00000008)          /*!< TXINTR interrupt */

/*******************  Bit definition for HW_SPI_MIS register  ****************/
#define	SPI_MIS_RORMIS                      ((uint32_t)0x00000001)          /*!< RORINTR interrupt */
#define	SPI_MIS_RTMIS                       ((uint32_t)0x00000002)          /*!< RTINTR interrupt */
#define	SPI_MIS_RXMIS                       ((uint32_t)0x00000004)          /*!< RXINTR interrupt */
#define	SPI_MIS_TXMIS                       ((uint32_t)0x00000008)          /*!< TXINTR interrupt */

/*******************  Bit definition for HW_SPI_ICR register  ****************/
#define	SPI_ICR_RORIC                       ((uint32_t)0x00000001)          /*!< Clear the RORINTR interrupt */
#define	SPI_ICR_RTIC                        ((uint32_t)0x00000002)          /*!< Clear the RTINTR interrupt */

/*******************  Bit definition for HW_SPI_DMACR register  ****************/
#define	SPI_DMACR_RXDMAE                    ((uint32_t)0x00000001)          /*!< Enable dma for the receive FIFO */
#define	SPI_DMACR_TXDMAE                    ((uint32_t)0x00000002)          /*!< Enable dma for the transmit FIFO */

/******************************************************************************/
/*                                                                            */
/*                 Analog to Digital Converter                                */
/*                                                                            */
/******************************************************************************/
/*******************  Bit definition for HW_ADC_ISR register  *****************/
#define	ADC_ISR_EOC                         ((uint32_t)0x00000004)              /*!< End of conversion flag */
#define	ADC_ISR_EOSEQ                       ((uint32_t)0x00000008)              /*!< End of sequence flag */
#define	ADC_ISR_OVR                         ((uint32_t)0x00000010)              /*!< ADC overrun */
#define	ADC_ISR_AWD                         ((uint32_t)0x00000080)              /*!< Analog watchdog flag */

/*******************  Bit definition for HW_ADC_IER register  *****************/
#define	ADC_IER_EOCIE                       ((uint32_t)0x00000004)              /*!< End of conversion interrupt enable */
#define	ADC_IER_EOSEQIE                     ((uint32_t)0x00000008)              /*!< End of conversion sequence interrupt enable */
#define	ADC_IER_OVRIE                       ((uint32_t)0x00000010)              /*!< Overrun interrupt enable */
#define	ADC_IER_AWDIE                       ((uint32_t)0x00000080)              /*!< Analog watchdog interrupt enable */

/*******************  Bit definition for HW_ADC_CR register  ******************/
#define	ADC_CR_ADSTART                      ((uint32_t)0x00000004)              /*!< ADC start conversion command */

/*******************  Bit definition for HW_ADC_CFGR1 register  ***************/
#define	ADC_CFGR1_SCANDIR                   ((uint32_t)0x00000004)              /*!< Scan sequence direction */
#define	ADC_CFGR1_RES_MASK                  ((uint32_t)0x00000018)              /*!< Data resolution mask */
#define	ADC_CFGR1_ALIGN                     ((uint32_t)0x00000020)              /*!< Data alignment */
#define	ADC_CFGR1_EXTSEL_MASK               ((uint32_t)0x000003C0)              /*!< External trigger selection */
#define	ADC_CFGR1_EXTEN_MASK                ((uint32_t)0x00000C00)              /*!< External trigger enable and polarity selection */
#define	ADC_CFGR1_OVRMOD                    ((uint32_t)0x00001000)              /*!< Overrun management mode */
#define	ADC_CFGR1_CONT                      ((uint32_t)0x00002000)              /*!< Single / continuous conversion mode */
#define	ADC_CFGR1_WAIT                      ((uint32_t)0x00004000)              /*!< Wait conversion mode */
#define	ADC_CFGR1_DISCEN                    ((uint32_t)0x00010000)              /*!< Discontinuous mode */
#define	ADC_CFGR1_AWDSGL                    ((uint32_t)0x00400000)              /*!< Enable the watchdog on a single channel or on all channels */
#define	ADC_CFGR1_AWDEN                     ((uint32_t)0x00800000)              /*!< Analog watchdog enable */
#define	ADC_CFGR1_AWDCH_MASK                ((uint32_t)0x7F000000)              /*!< Analog watchdog channel selection mask */
#define	ADC_CFGR1_ASYNC_TRG                 ((uint32_t)0x80000000)              /*!< External trigger selection */

/*******************  Bit definition for HW_ADC_CFGR2 register  ***************/
#define	ADC_CFGR2_CLKMODE_MASK              ((uint32_t)0xC0000000)              /*!< ADC clock mode */

/*******************  Bit definition for HW_ADC_TR register  ******************/
#define	ADC_TR_LT_MASK                      ((uint32_t)0x00001FFF)              /*!< Analog watchdog lower threshold */
#define	ADC_TR_HT_MASK                      ((uint32_t)0x1FFF0000)              /*!< Analog watchdog higher threshold */

/*******************  Bit definition for HW_ADC_CHSELR register  **************/
#define	ADC_CHSELR_CH0                      ((uint32_t)0x00000001)              /*!< Channel0 selection */
#define	ADC_CHSELR_CH1                      ((uint32_t)0x00000002)              /*!< Channel1 selection */
#define	ADC_CHSELR_CH2                      ((uint32_t)0x00000004)              /*!< Channel2 selection */
#define	ADC_CHSELR_CH3                      ((uint32_t)0x00000008)              /*!< Channel3 selection */
#define	ADC_CHSELR_CH4                      ((uint32_t)0x00000010)              /*!< Channel4 selection */
#define	ADC_CHSELR_CH5                      ((uint32_t)0x00000020)              /*!< Channel5 selection */
#define	ADC_CHSELR_CH6                      ((uint32_t)0x00000040)              /*!< Channel6 selection */
#define	ADC_CHSELR_CH7                      ((uint32_t)0x00000080)              /*!< Channel7 selection */
#define	ADC_CHSELR_CH8                      ((uint32_t)0x00000100)              /*!< Channel8 selection */
#define	ADC_CHSELR_CH9                      ((uint32_t)0x00000200)              /*!< Channel9 selection */
#define	ADC_CHSELR_CH10                     ((uint32_t)0x00000400)              /*!< Channel10 selection */
#define	ADC_CHSELR_CH11                     ((uint32_t)0x00000800)              /*!< Channel11 selection */
#define	ADC_CHSELR_CH12                     ((uint32_t)0x00001000)              /*!< Channel12 selection */
#define	ADC_CHSELR_CH13                     ((uint32_t)0x00002000)              /*!< Channel13 selection */
#define	ADC_CHSELR_CH14                     ((uint32_t)0x00004000)              /*!< Channel14 selection */
#define	ADC_CHSELR_CH15                     ((uint32_t)0x00008000)              /*!< Channel15 selection*/

/*******************  Bit definition for HW_ADC_DR register  ******************/
#define	ADC_DR_DATA_MASK                    ((uint32_t)0x0000FFFF)              /*!< Converted data mask */

/*******************  Bit definition for HW_ADC_CCR register  *****************/
#define	ADC_CCR_TSEN                        ((uint32_t)0x00800000)              /*!< Temperature sensor enable */

/*******************  Bit definition for HW_ADC_ACR0 register  ****************/
#define	ADC_ACR0_DISCARD0_MASK              ((uint32_t)0x00000003)              /*!< Discard0 */
#define	ADC_ACR0_DISCARD1_MASK              ((uint32_t)0x00000030)              /*!< Discard1 */

/*******************  Bit definition for HW_ADC_ACR1 register  ****************/
#define	ADC_ACR1_BVOSI                      ((uint32_t)0x0000007F)              /*!< BVOSI */
#define	ADC_ACR1_RESET_CAL                  ((uint32_t)0x00000080)              /*!< Reset CAL */
#define	ADC_ACR1_RESETADC_ANA               ((uint32_t)0x00000100)              /*!< Reset analog */
#define	ADC_ACR1_LOAD_CAL                   ((uint32_t)0x00000200)              /*!< Load CAL */
#define	ADC_ACR1_BYPASS_BVOS_CAL            ((uint32_t)0x00000400)              /*!< Bypass BVOS */
#define	ADC_ACR1_BYPASS_MULTI_CAL           ((uint32_t)0x00000800)              /*!< Bypass MULTI */
#define	ADC_ACR1_START_CAL                  ((uint32_t)0x00001000)              /*!< Start CAL */

/*******************  Bit definition for HW_ADC_ACR2 register  ****************/
#define	ADC_ACR2_MULTI_SINGLE               ((uint32_t)0x00000FFF)              /*!< Multiplication */

/*******************  Bit definition for HW_ADC_STATUS register  **************/
#define	ADC_STATUS_BVOS                     ((uint32_t)0x0000007F)              /*!< BVOS */
#define	ADC_STATUS_CAL_ON                   ((uint32_t)0x80000000)              /*!< CAL enabled */

/*******************  Bit definition for HW_ADC_DR_CH register  ***************/
#define	ADC_DR_MASK                         ((uint32_t)0x0000FFFF)              /*!< Data mask */

/******************************************************************************/
/*                                                                            */
/*                                    ACMP                                    */
/*                                                                            */
/******************************************************************************/
/*******************  Bit definition for ACMP_CTRL register  ******************/
#define	ACMP_CTRL_CMP_EN                    ((uint32_t)0x00000001)              /*!< ACMP enable */
#define	ACMP_CTRL_CMP_IE                    ((uint32_t)0x00000002)              /*!< ACMP interrupt enable */
#define	ACMP_CTRL_CMP_HYSEN_MASK            ((uint32_t)0x0000000C)              /*!< Mask of ACMP hysteresis */
#define	ACMP_CTRL_CMP_HYSEN_0               ((uint32_t)0x00000000)              /*!< ACMP hysteresis 0mv */
#define	ACMP_CTRL_CMP_HYSEN_15MV            ((uint32_t)0x00000008)              /*!< ACMP hysteresis 15mv */
#define	ACMP_CTRL_CMP_HYSEN_40MV            ((uint32_t)0x0000000C)              /*!< ACMP hysteresis 40mv */
#define	ACMP_CTRL_CMP_NEG_SEL_MASK          ((uint32_t)0x000000F0)              /*!< Mask of ACMP Negative source */
#define	ACMP_CTRL_CMP_NEG_SEL_PIN0_1        ((uint32_t)0x00000000)              /*!< ACMP Negative source is PIO0_1 */
#define	ACMP_CTRL_CMP_NEG_SEL_PIN0_2        ((uint32_t)0x00000010)              /*!< ACMP Negative source is PIO0_2 */
#define	ACMP_CTRL_CMP_NEG_SEL_PIN0_3        ((uint32_t)0x00000020)              /*!< ACMP Negative source is PIO0_3 */
#define	ACMP_CTRL_CMP_NEG_SEL_PIN0_4        ((uint32_t)0x00000030)              /*!< ACMP Negative source is PIO0_4 */
#define	ACMP_CTRL_CMP_NEG_SEL_PIN0_5        ((uint32_t)0x00000040)              /*!< ACMP Negative source is PIO0_5 */
#define	ACMP_CTRL_CMP_NEG_SEL_PIN0_6        ((uint32_t)0x00000050)              /*!< ACMP Negative source is PIO0_6 */
#define	ACMP_CTRL_CMP_NEG_SEL_PIN0_7        ((uint32_t)0x00000060)              /*!< ACMP Negative source is PIO0_7 */
#define	ACMP_CTRL_CMP_NEG_SEL_PIN1_0        ((uint32_t)0x00000070)              /*!< ACMP Negative source is PIO1_0 */
#define	ACMP_CTRL_CMP_NEG_SEL_R             ((uint32_t)0x00000080)              /*!< ACMP Negative source is resistance */
#define	ACMP_CTRL_CMP_NEG_SEL_TEMP          ((uint32_t)0x00000090)              /*!< ACMP Negative source is temperature */
#define	ACMP_CTRL_CMP_NEG_SEL_VREF_1P25V    ((uint32_t)0x000000A0)              /*!< ACMP Negative source is vref 1.25V */
#define	ACMP_CTRL_CMP_NEG_SEL_ADC_VREF      ((uint32_t)0x000000B0)              /*!< ACMP Negative source is ADC VREF */
#define	ACMP_CTRL_CMP_NEG_SEL_CORE_LDO      ((uint32_t)0x000000C0)              /*!< ACMP Negative source is core ldo */
#define	ACMP_CTRL_CMP_POS_SEL_MASK          ((uint32_t)0x00000700)              /*!< Mask of ACMP positive source */
#define	ACMP_CTRL_CMP_POS_SEL_PIN0_1        ((uint32_t)0x00000000)              /*!< ACMP positive source is PIO0_1 */
#define	ACMP_CTRL_CMP_POS_SEL_PIN0_2        ((uint32_t)0x00000100)              /*!< ACMP positive source is PIO0_2 */
#define	ACMP_CTRL_CMP_POS_SEL_PIN0_3        ((uint32_t)0x00000200)              /*!< ACMP positive source is PIO0_3 */
#define	ACMP_CTRL_CMP_POS_SEL_PIN0_4        ((uint32_t)0x00000300)              /*!< ACMP positive source is PIO0_4 */
#define	ACMP_CTRL_CMP_POS_SEL_PIN0_5        ((uint32_t)0x00000400)              /*!< ACMP positive source is PIO0_5 */
#define	ACMP_CTRL_CMP_POS_SEL_PIN0_6        ((uint32_t)0x00000500)              /*!< ACMP positive source is PIO0_6 */
#define	ACMP_CTRL_CMP_POS_SEL_PIN0_7        ((uint32_t)0x00000600)              /*!< ACMP positive source is PIO0_7 */
#define	ACMP_CTRL_CMP_POS_SEL_PIN1_0        ((uint32_t)0x00000700)              /*!< ACMP positive source is PIO1_0 */
#define ACMP_CTRL_CMP_EDGE_SEL_MASK         ((uint32_t)0x00030000)              /*!< Mask of ACMP edge select */
#define ACMP_CTRL_CMP_EDGE_SEL_FALL         ((uint32_t)0x00010000)              /*!< Select ACMP falling edge */
#define ACMP_CTRL_CMP_EDGE_SEL_RISE         ((uint32_t)0x00020000)              /*!< Select ACMP rising edge */
#define ACMP_CTRL_CMP_EDGE_SEL_DOUBLE       ((uint32_t)0x00000000)              /*!< Select ACMP double edge */
#define ACMP_CTRL_CMP_INV                   ((uint32_t)0x00100000)              /*!< ACMP inverse output state */

/*******************  Bit definition for ACMP_STATUS register  ****************/
#define	ACMP_STATUS_INT_FLAG0               ((uint32_t)0x00000001)              /*!< ACMP0 interrupt flag */
#define	ACMP_STATUS_INT_FLAG1               ((uint32_t)0x00000002)              /*!< ACMP1 interrupt flag */
#define	ACMP_STATUS_RESULT0                 ((uint32_t)0x00000010)              /*!< ACMP0 result */
#define	ACMP_STATUS_RESULT1                 ((uint32_t)0x00000020)              /*!< ACMP1 result */

/*******************  Bit definition for ACMP_ADV_CTRL0 register  *************/
#define	ACMP_ADV_CTRL0_CMP0_TRGS_MASK       ((uint32_t)0x00000003)              /*!< Mask of CMP0 TRGS */
#define	ACMP_ADV_CTRL0_CMP0_TRGS_CON        ((uint32_t)0x00000000)              /*!< CMP0 TRGS continuous */
#define	ACMP_ADV_CTRL0_CMP0_TRGS_SYNC_PWM0  ((uint32_t)0x00000001)              /*!< CMP0 TRGS pwm0 */
#define	ACMP_ADV_CTRL0_CMP0_TRGS_SYNC_PWM1  ((uint32_t)0x00000002)              /*!< CMP0 TRGS pwm1 */
#define	ACMP_ADV_CTRL0_CMP0_TRGS_SYNC_PWM2  ((uint32_t)0x00000003)              /*!< CMP0 TRGS pwm2 */
#define	ACMP_ADV_CTRL0_CMP0_TRGPOL          ((uint32_t)0x00000004)              /*!< CMP0 TRGPOL */
#define	ACMP_ADV_CTRL0_CMP1_TRGS_MASK       ((uint32_t)0x00000030)              /*!< Mask of CMP1 TRGS */
#define	ACMP_ADV_CTRL0_CMP1_TRGS_CON        ((uint32_t)0x00000000)              /*!< CMP1 TRGS continuous */
#define	ACMP_ADV_CTRL0_CMP1_TRGS_SYNC_PWM0  ((uint32_t)0x00000010)              /*!< CMP1 TRGS pwm0 */
#define	ACMP_ADV_CTRL0_CMP1_TRGS_SYNC_PWM1  ((uint32_t)0x00000020)              /*!< CMP1 TRGS pwm1 */
#define	ACMP_ADV_CTRL0_CMP1_TRGS_SYNC_PWM2  ((uint32_t)0x00000030)              /*!< CMP1 TRGS pwm2 */
#define	ACMP_ADV_CTRL0_CMP1_TRGPOL          ((uint32_t)0x00000040)              /*!< CMP1 TRGPOL */

/*******************  Bit definition for ACMP_ADV_CTRL1 register  *************/
#define	ACMP_ADV_CTRL1_CMP0_FT2S_MASK       ((uint32_t)0x00000007)              /*!< Mask of CMP0 filter2 parameter */
#define	ACMP_ADV_CTRL1_CMP0_FT2S_8          ((uint32_t)0x00000000)              /*!< CMP0 filter2 parameter 8 */
#define	ACMP_ADV_CTRL1_CMP0_FT2S_16         ((uint32_t)0x00000001)              /*!< CMP0 filter2 parameter 16 */
#define	ACMP_ADV_CTRL1_CMP0_FT2S_32         ((uint32_t)0x00000002)              /*!< CMP0 filter2 parameter 32 */
#define	ACMP_ADV_CTRL1_CMP0_FT2S_64         ((uint32_t)0x00000003)              /*!< CMP0 filter2 parameter 64 */
#define	ACMP_ADV_CTRL1_CMP0_FT2S_128        ((uint32_t)0x00000004)              /*!< CMP0 filter2 parameter 128 */
#define	ACMP_ADV_CTRL1_CMP0_FT2S_256        ((uint32_t)0x00000005)              /*!< CMP0 filter2 parameter 256 */
#define	ACMP_ADV_CTRL1_CMP0_FT2S_512        ((uint32_t)0x00000006)              /*!< CMP0 filter2 parameter 512 */
#define	ACMP_ADV_CTRL1_CMP0_FT2S_1024       ((uint32_t)0x00000007)              /*!< CMP0 filter2 parameter 1024 */
#define	ACMP_ADV_CTRL1_CMP0_FT2EN           ((uint32_t)0x00000008)              /*!< Enable CMP0 filter2 */
#define	ACMP_ADV_CTRL1_CMP0_FT1S_MASK       ((uint32_t)0x00000070)              /*!< Mask of CMP0 filter1 parameter */
#define	ACMP_ADV_CTRL1_CMP0_FT1S_32         ((uint32_t)0x00000000)              /*!< CMP0 filter1 parameter 32 */
#define	ACMP_ADV_CTRL1_CMP0_FT1S_64         ((uint32_t)0x00000010)              /*!< CMP0 filter1 parameter 64 */
#define	ACMP_ADV_CTRL1_CMP0_FT1S_128        ((uint32_t)0x00000020)              /*!< CMP0 filter1 parameter 128 */
#define	ACMP_ADV_CTRL1_CMP0_FT1S_256        ((uint32_t)0x00000030)              /*!< CMP0 filter1 parameter 256 */
#define	ACMP_ADV_CTRL1_CMP0_FT1S_512        ((uint32_t)0x00000040)              /*!< CMP0 filter1 parameter 512 */
#define	ACMP_ADV_CTRL1_CMP0_FT1S_1024       ((uint32_t)0x00000050)              /*!< CMP0 filter1 parameter 1024 */
#define	ACMP_ADV_CTRL1_CMP0_FT1S_2048       ((uint32_t)0x00000060)              /*!< CMP0 filter1 parameter 2048 */
#define	ACMP_ADV_CTRL1_CMP0_FT1S_4096       ((uint32_t)0x00000070)              /*!< CMP0 filter1 parameter 4096 */
#define	ACMP_ADV_CTRL1_CMP0_FT1EN           ((uint32_t)0x00000080)              /*!< Enable CMP0 filter1 */
#define	ACMP_ADV_CTRL1_CMP1_FT2S_MASK       ((uint32_t)0x00000700)              /*!< Mask of CMP1 filter2 parameter */
#define	ACMP_ADV_CTRL1_CMP1_FT2S_8          ((uint32_t)0x00000000)              /*!< CMP1 filter2 parameter 8 */
#define	ACMP_ADV_CTRL1_CMP1_FT2S_16         ((uint32_t)0x00000100)              /*!< CMP1 filter2 parameter 16 */
#define	ACMP_ADV_CTRL1_CMP1_FT2S_32         ((uint32_t)0x00000200)              /*!< CMP1 filter2 parameter 32 */
#define	ACMP_ADV_CTRL1_CMP1_FT2S_64         ((uint32_t)0x00000300)              /*!< CMP1 filter2 parameter 64 */
#define	ACMP_ADV_CTRL1_CMP1_FT2S_128        ((uint32_t)0x00000400)              /*!< CMP1 filter2 parameter 128 */
#define	ACMP_ADV_CTRL1_CMP1_FT2S_256        ((uint32_t)0x00000500)              /*!< CMP1 filter2 parameter 256 */
#define	ACMP_ADV_CTRL1_CMP1_FT2S_512        ((uint32_t)0x00000600)              /*!< CMP1 filter2 parameter 512 */
#define	ACMP_ADV_CTRL1_CMP1_FT2S_1024       ((uint32_t)0x00000700)              /*!< CMP1 filter2 parameter 1024 */
#define	ACMP_ADV_CTRL1_CMP1_FT2EN           ((uint32_t)0x00000800)              /*!< Enable CMP1 filter2 */
#define	ACMP_ADV_CTRL1_CMP1_FT1S_MASK       ((uint32_t)0x00007000)              /*!< Mask of CMP1 filter1 parameter */
#define	ACMP_ADV_CTRL1_CMP1_FT1S_32         ((uint32_t)0x00000000)              /*!< CMP1 filter1 parameter 32 */
#define	ACMP_ADV_CTRL1_CMP1_FT1S_64         ((uint32_t)0x00001000)              /*!< CMP1 filter1 parameter 64 */
#define	ACMP_ADV_CTRL1_CMP1_FT1S_128        ((uint32_t)0x00002000)              /*!< CMP1 filter1 parameter 128 */
#define	ACMP_ADV_CTRL1_CMP1_FT1S_256        ((uint32_t)0x00003000)              /*!< CMP1 filter1 parameter 256 */
#define	ACMP_ADV_CTRL1_CMP1_FT1S_512        ((uint32_t)0x00004000)              /*!< CMP1 filter1 parameter 512 */
#define	ACMP_ADV_CTRL1_CMP1_FT1S_1024       ((uint32_t)0x00005000)              /*!< CMP1 filter1 parameter 1024 */
#define	ACMP_ADV_CTRL1_CMP1_FT1S_2048       ((uint32_t)0x00006000)              /*!< CMP1 filter1 parameter 2048 */
#define	ACMP_ADV_CTRL1_CMP1_FT1S_4096       ((uint32_t)0x00007000)              /*!< CMP1 filter1 parameter 4096 */
#define	ACMP_ADV_CTRL1_CMP1_FT1EN           ((uint32_t)0x00008000)              /*!< Enable CMP1 filter1 */

/*******************  Bit definition for ACMP_ADV_CTRL2 register  *************/
#define	ACMP_ADV_CTRL2_CMP0_DEB_EN          ((uint32_t)0x00000001)              /*!< Enable CMP0 DEB */
#define	ACMP_ADV_CTRL2_CMP1_DEB_EN          ((uint32_t)0x00000002)              /*!< Enable CMP1 DEB */
#define	ACMP_ADV_CTRL2_CMP0_DEB_NUM         ((uint32_t)0x0000FF00)              /*!< CMP0 DEB number */
#define	ACMP_ADV_CTRL2_CMP1_DEB_NUM         ((uint32_t)0x00FF0000)              /*!< CMP1 DEB number */

/*******************  Bit definition for ACMP_ADV_CTRL3 register  *************/
#define	ACMP_ADV_CTRL3_CMP0_FLT_SEL         ((uint32_t)0x00000001)              /*!< CMP0 fault select */
#define	ACMP_ADV_CTRL3_CMP1_FLT_SEL         ((uint32_t)0x00000002)              /*!< CMP1 fault select */
#define	ACMP_ADV_CTRL3_PWM0_FLT_SEL_MASK    ((uint32_t)0x00000030)              /*!< Mask of PWM0 FLT select */
#define	ACMP_ADV_CTRL3_PWM0_FLT_SEL_ALWAYS  ((uint32_t)0x00000000)              /*!< PWM0 FLT select */
#define	ACMP_ADV_CTRL3_PWM0_FLT_SEL_CMP0    ((uint32_t)0x00000010)              /*!< PWM0 FLT select */
#define	ACMP_ADV_CTRL3_PWM0_FLT_SEL_CMP1    ((uint32_t)0x00000020)              /*!< PWM0 FLT select */
#define	ACMP_ADV_CTRL3_PWM1_FLT_SEL_MASK    ((uint32_t)0x000000C0)              /*!< Mask of PWM1 FLT select */
#define	ACMP_ADV_CTRL3_PWM1_FLT_SEL_ALWAYS  ((uint32_t)0x00000000)              /*!< PWM1 FLT select */
#define	ACMP_ADV_CTRL3_PWM1_FLT_SEL_CMP0    ((uint32_t)0x00000040)              /*!< PWM1 FLT select */
#define	ACMP_ADV_CTRL3_PWM1_FLT_SEL_CMP1    ((uint32_t)0x00000080)              /*!< PWM1 FLT select */
#define	ACMP_ADV_CTRL3_PWM2_FLT_SEL_MASK    ((uint32_t)0x00000300)              /*!< Mask of PWM2 FLT select */
#define	ACMP_ADV_CTRL3_PWM2_FLT_SEL_ALWAYS  ((uint32_t)0x00000000)              /*!< PWM2 FLT select */
#define	ACMP_ADV_CTRL3_PWM2_FLT_SEL_CMP0    ((uint32_t)0x00000100)              /*!< PWM2 FLT select */
#define	ACMP_ADV_CTRL3_PWM2_FLT_SEL_CMP1    ((uint32_t)0x00000200)              /*!< PWM2 FLT select */
#define	ACMP_ADV_CTRL3_PWM3_FLT_SEL_MASK    ((uint32_t)0x00000C00)              /*!< Mask of PWM3 FLT select */
#define	ACMP_ADV_CTRL3_PWM3_FLT_SEL_ALWAYS  ((uint32_t)0x00000000)              /*!< PWM3 FLT select */
#define	ACMP_ADV_CTRL3_PWM3_FLT_SEL_CMP0    ((uint32_t)0x00000400)              /*!< PWM3 FLT select */
#define	ACMP_ADV_CTRL3_PWM3_FLT_SEL_CMP1    ((uint32_t)0x00000800)              /*!< PWM3 FLT select */

/*******************  Bit definition for ACMP_CMPCLKDIV register  *************/
#define	ACMP_CMPCLKDIV_DIV_MASK             ((uint32_t)0x000000FF)              /*!< CMP clock div */

/******************************************************************************/
/*                                                                            */
/*                                    TIMER                                   */
/*                                                                            */
/******************************************************************************/
/*******************  Bit definition for TIMER_IR register  *******************/
#define TIMER_IR_MR0                        ((uint32_t)0x00000001)              /*!< Matching channel 0 interrupt flag */
#define TIMER_IR_MR1                        ((uint32_t)0x00000002)              /*!< Matching channel 1 interrupt flag */
#define TIMER_IR_MR2                        ((uint32_t)0x00000004)              /*!< Matching channel 2 interrupt flag */
#define TIMER_IR_MR3                        ((uint32_t)0x00000008)              /*!< Matching channel 3 interrupt flag */
#define TIMER_IR_CR0                        ((uint32_t)0x00000010)              /*!< Capture 0 interrupt flag */
#define TIMER_IR_SING0                      ((uint32_t)0x00000020)              /*!< Channel 0 single interrupt flag */
#define TIMER_IR_SING1                      ((uint32_t)0x00000040)              /*!< Channel 1 single interrupt flag */
#define TIMER_IR_SING2                      ((uint32_t)0x00000080)              /*!< Channel 2 single interrupt flag */
#define TIMER_IR_SING3                      ((uint32_t)0x00000100)              /*!< Channel 3 single interrupt flag */
#define TIMER_IR_CR1                        ((uint32_t)0x00000200)              /*!< Capture 1 interrupt flag */
#define TIMER_IR_CR2                        ((uint32_t)0x00000400)              /*!< Capture 2 interrupt flag */
#define TIMER_IR_CR3                        ((uint32_t)0x00000800)              /*!< Capture 3 interrupt flag */

/*******************  Bit definition for TIMER_TCR register  ******************/
#define TIMER_TCR0_ENABLE                   ((uint32_t)0x00000001)              /*!< Enable timer channel 0 */
#define TIMER_TCR0_RESET                    ((uint32_t)0x00000010)              /*!< Reset timer channel 0 */
#define TIMER_TCR1_ENABLE                   ((uint32_t)0x00000002)              /*!< Enable timer channel 1 */
#define TIMER_TCR1_RESET                    ((uint32_t)0x00000020)              /*!< Reset timer channel 1 */
#define TIMER_TCR2_ENABLE                   ((uint32_t)0x00000004)              /*!< Enable timer channel 2 */
#define TIMER_TCR2_RESET                    ((uint32_t)0x00000040)              /*!< Reset timer channel 2 */
#define TIMER_TCR3_ENABLE                   ((uint32_t)0x00000008)              /*!< Enable timer channel 3 */
#define TIMER_TCR3_RESET                    ((uint32_t)0x00000080)              /*!< Reset timer channel 3 */
#define TIMER_TCR0_OUT                      ((uint32_t)0x00000300)              /*!< Channel0 idle level mask */
#define TIMER_TCR1_OUT                      ((uint32_t)0x00000C00)              /*!< Channel0 idle level mask */
#define TIMER_TCR2_OUT                      ((uint32_t)0x00003000)              /*!< Channel0 idle level mask */
#define TIMER_TCR3_OUT                      ((uint32_t)0x0000C000)              /*!< Channel0 idle level mask */
#define TIMER_TCR0_PWMEN                    ((uint32_t)0x00010000)              /*!< Enable Channel 0 pwm */
#define TIMER_TCR1_PWMEN                    ((uint32_t)0x00020000)              /*!< Enable Channel 1 pwm */
#define TIMER_TCR2_PWMEN                    ((uint32_t)0x00040000)              /*!< Enable Channel 2 pwm */
#define TIMER_TCR3_PWMEN                    ((uint32_t)0x00080000)              /*!< Enable Channel 3 pwm */
#define TIMER_TCR0_PWMIO                    ((uint32_t)0x00100000)              /*!< Set IO to PWM 0 */
#define TIMER_TCR1_PWMIO                    ((uint32_t)0x00200000)              /*!< Set IO to PWM 1 */
#define TIMER_TCR2_PWMIO                    ((uint32_t)0x00400000)              /*!< Set IO to PWM 2 */
#define TIMER_TCR3_PWMIO                    ((uint32_t)0x00800000)              /*!< Set IO to PWM 3 */

/*******************  Bit definition for TIMER_DIR register  ******************/
#define TIMER_DIR0                          ((uint32_t)0x00000003)              /*!< Timer channel 0 Counting direction */
#define TIMER_DIR1                          ((uint32_t)0x00000030)              /*!< Timer channel 1 Counting direction */
#define TIMER_DIR2                          ((uint32_t)0x00000300)              /*!< Timer channel 2 Counting direction */
#define TIMER_DIR3                          ((uint32_t)0x00003000)              /*!< Timer channel 3 Counting direction */
#define TIMER_DIR_CARR0_MASK                ((uint32_t)0x03000000)              /*!< Timer channel 0 carrier */
#define TIMER_DIR_CARR0_AND_CH0             ((uint32_t)0x00000000)              /*!< Timer channel 0 carrier */
#define TIMER_DIR_CARR0_AND_CH1             ((uint32_t)0x01000000)              /*!< Timer channel 0 carrier */
#define TIMER_DIR_CARR0_AND_CH2             ((uint32_t)0x02000000)              /*!< Timer channel 0 carrier */
#define TIMER_DIR_CARR0_AND_CH3             ((uint32_t)0x03000000)              /*!< Timer channel 0 carrier */
#define TIMER_DIR_CARR1_MASK                ((uint32_t)0x0C000000)              /*!< Timer channel 1 carrier */
#define TIMER_DIR_CARR1_AND_CH0             ((uint32_t)0x0C000000)              /*!< Timer channel 1 carrier */
#define TIMER_DIR_CARR1_AND_CH1             ((uint32_t)0x00000000)              /*!< Timer channel 1 carrier */
#define TIMER_DIR_CARR1_AND_CH2             ((uint32_t)0x04000000)              /*!< Timer channel 1 carrier */
#define TIMER_DIR_CARR1_AND_CH3             ((uint32_t)0x08000000)              /*!< Timer channel 1 carrier */
#define TIMER_DIR_CARR2_MASK                ((uint32_t)0x30000000)              /*!< Timer channel 2 carrier */
#define TIMER_DIR_CARR2_AND_CH0             ((uint32_t)0x20000000)              /*!< Timer channel 2 carrier */
#define TIMER_DIR_CARR2_AND_CH1             ((uint32_t)0x30000000)              /*!< Timer channel 2 carrier */
#define TIMER_DIR_CARR2_AND_CH2             ((uint32_t)0x00000000)              /*!< Timer channel 2 carrier */
#define TIMER_DIR_CARR2_AND_CH3             ((uint32_t)0x10000000)              /*!< Timer channel 2 carrier */
#define TIMER_DIR_CARR3_MASK                ((uint32_t)0xC0000000)              /*!< Timer channel 3 carrier */
#define TIMER_DIR_CARR3_AND_CH0             ((uint32_t)0x40000000)              /*!< Timer channel 3 carrier */
#define TIMER_DIR_CARR3_AND_CH1             ((uint32_t)0x80000000)              /*!< Timer channel 3 carrier */
#define TIMER_DIR_CARR3_AND_CH2             ((uint32_t)0xC0000000)              /*!< Timer channel 3 carrier */
#define TIMER_DIR_CARR3_AND_CH3             ((uint32_t)0x00000000)              /*!< Timer channel 3 carrier */

/*******************  Bit definition for TIMER_TC0 register  ******************/
#define TIMER_TC0                           ((uint32_t)0xFFFFFFFF)              /*!< Timer channel 0 count */

/*******************  Bit definition for TIMER_TC1 register  ******************/
#define TIMER_TC1                           ((uint32_t)0xFFFFFFFF)              /*!< Timer channel 0 count */

/*******************  Bit definition for TIMER_TC2 register  ******************/
#define TIMER_TC2                           ((uint32_t)0xFFFFFFFF)              /*!< Timer channel 0 count */

/*******************  Bit definition for TIMER_TC3 register  ******************/
#define TIMER_TC3                           ((uint32_t)0xFFFFFFFF)              /*!< Timer channel 0 count */

/*******************  Bit definition for TIMER_PR register  *******************/
#define TIMER_PR_VAL                        ((uint32_t)0xFFFFFFFF)              /*!< Timer Prescale register */

/*******************  Bit definition for TIMER_PC register  *******************/
#define TIMER_PC_VAL                        ((uint32_t)0xFFFFFFFF)              /*!< Timer Prescale counter */

/*******************  Bit definition for TIMER_MCR register  ******************/
#define TIMER_MCR_MR0I                      ((uint32_t)0x00000001)              /*!< Enable interrupt of matching register 0 */
#define TIMER_MCR_MR0R                      ((uint32_t)0x00000002)              /*!< Reset TC0 when matching */
#define TIMER_MCR_MR0S                      ((uint32_t)0x00000004)              /*!< Disable TC0 when matching */
#define TIMER_MCR_MR1I                      ((uint32_t)0x00000008)              /*!< Enable interrupt of matching register 1 */
#define TIMER_MCR_MR1R                      ((uint32_t)0x00000010)              /*!< Reset TC1 when matching */
#define TIMER_MCR_MR1S                      ((uint32_t)0x00000020)              /*!< Disable TC1 when matching */
#define TIMER_MCR_MR2I                      ((uint32_t)0x00000040)              /*!< Enable interrupt of matching register 2 */
#define TIMER_MCR_MR2R                      ((uint32_t)0x00000080)              /*!< Reset TC2 when matching */
#define TIMER_MCR_MR2S                      ((uint32_t)0x00000100)              /*!< Disable TC2 when matching */
#define TIMER_MCR_MR3I                      ((uint32_t)0x00000200)              /*!< Enable interrupt of matching register 3 */
#define TIMER_MCR_MR3R                      ((uint32_t)0x00000400)              /*!< Reset TC3 when matching */
#define TIMER_MCR_MR3S                      ((uint32_t)0x00000800)              /*!< Disable TC3 when matching */

/*******************  Bit definition for TIMER_MR0 register  ******************/
#define TIMER_MRO_VAL                       ((uint32_t)0xFFFFFFFF)              /*!< Timer channel 0 match value */

/*******************  Bit definition for TIMER_MR1 register  ******************/
#define TIMER_MR1_VAL                       ((uint32_t)0xFFFFFFFF)              /*!< Timer channel 1 match value */

/*******************  Bit definition for TIMER_MR2 register  ******************/
#define TIMER_MR2_VAL                       ((uint32_t)0xFFFFFFFF)              /*!< Timer channel 2 match value */

/*******************  Bit definition for TIMER_MR3 register  ******************/
#define TIMER_MR3_VAL                       ((uint32_t)0xFFFFFFFF)              /*!< Timer channel 3 match value */

/*******************  Bit definition for TIMER_CCR0 register  *****************/
#define TIMER_CCR_CAP0RE                    ((uint32_t)0x00000001)              /*!< Capture 0 when rising edge */
#define TIMER_CCR_CAP0FE                    ((uint32_t)0x00000002)              /*!< Capture 0 when falling edge */    
#define TIMER_CCR_CAP0I                     ((uint32_t)0x00000004)              /*!< Enable Capture 0 interrupt */    
#define TIMER_CCR_CAP0FILTER                ((uint32_t)0x000000F0)              /*!< Caputre 0 filter */    
#define TIMER_CCR_CAP1RE                    ((uint32_t)0x00000100)              /*!< Capture 1 when rising edge */
#define TIMER_CCR_CAP1FE                    ((uint32_t)0x00000200)              /*!< Capture 1 when falling edge */    
#define TIMER_CCR_CAP1I                     ((uint32_t)0x00000400)              /*!< Enable Capture 1 interrupt */    
#define TIMER_CCR_CAP1FILTER                ((uint32_t)0x0000F000)              /*!< Caputre 1 filter */    
#define TIMER_CCR_CNT0CAP_MASK              ((uint32_t)0x00030000)              /*!< Count 0 capture PIN */    
#define TIMER_CCR_CNT0CAP_PIN0              ((uint32_t)0x00000000)              /*!< Count 0 capture PIN 0 */    
#define TIMER_CCR_CNT0CAP_PIN1              ((uint32_t)0x00010000)              /*!< Count 0 capture PIN 1 */    
#define TIMER_CCR_CNT0CAP_PIN2              ((uint32_t)0x00020000)              /*!< Count 0 capture PIN 2 */    
#define TIMER_CCR_CNT0CAP_PIN3              ((uint32_t)0x00030000)              /*!< Count 0 capture PIN 3 */    
#define TIMER_CCR_CNT1CAP_MASK              ((uint32_t)0x000C0000)              /*!< Count 1 capture PIN */    
#define TIMER_CCR_CNT1CAP_PIN0              ((uint32_t)0x00000000)              /*!< Count 1 capture PIN 0 */    
#define TIMER_CCR_CNT1CAP_PIN1              ((uint32_t)0x00040000)              /*!< Count 1 capture PIN 1 */
#define TIMER_CCR_CNT1CAP_PIN2              ((uint32_t)0x00080000)              /*!< Count 1 capture PIN 2 */
#define TIMER_CCR_CNT1CAP_PIN3              ((uint32_t)0x000C0000)              /*!< Count 1 capture PIN 3 */
#define TIMER_CCR_CNT2CAP_MASK              ((uint32_t)0x00300000)              /*!< Count 2 capture PIN */
#define TIMER_CCR_CNT2CAP_PIN0              ((uint32_t)0x00000000)              /*!< Count 2 capture PIN 0 */
#define TIMER_CCR_CNT2CAP_PIN1              ((uint32_t)0x00100000)              /*!< Count 2 capture PIN 1 */
#define TIMER_CCR_CNT2CAP_PIN2              ((uint32_t)0x00200000)              /*!< Count 2 capture PIN 2 */
#define TIMER_CCR_CNT2CAP_PIN3              ((uint32_t)0x00300000)              /*!< Count 2 capture PIN 3 */
#define TIMER_CCR_CNT3CAP_MASK              ((uint32_t)0x00C00000)              /*!< Count 3 capture PIN */
#define TIMER_CCR_CNT3CAP_PIN0              ((uint32_t)0x00000000)              /*!< Count 3 capture PIN 0 */
#define TIMER_CCR_CNT3CAP_PIN1              ((uint32_t)0x00400000)              /*!< Count 3 capture PIN 1 */
#define TIMER_CCR_CNT3CAP_PIN2              ((uint32_t)0x00800000)              /*!< Count 3 capture PIN 2 */
#define TIMER_CCR_CNT3CAP_PIN3              ((uint32_t)0x00C00000)              /*!< Count 3 capture PIN 3 */

/*******************  Bit definition for TIMER_CCR1 register  *****************/
#define TIMER_CCR1_CAP2RE                   ((uint32_t)0x00000001)              /*!< Capture 2 when rising edge */
#define TIMER_CCR1_CAP2FE                   ((uint32_t)0x00000002)              /*!< Capture 2 when falling edge */    
#define TIMER_CCR1_CAP2I                    ((uint32_t)0x00000004)              /*!< Enable Capture 2 interrupt */    
#define TIMER_CCR1_CAP2FILTER               ((uint32_t)0x000000F0)              /*!< Caputre 2 filter */    
#define TIMER_CCR1_CAP3RE                   ((uint32_t)0x00000100)              /*!< Capture 3 when rising edge */
#define TIMER_CCR1_CAP3FE                   ((uint32_t)0x00000200)              /*!< Capture 3 when falling edge */    
#define TIMER_CCR1_CAP3I                    ((uint32_t)0x00000400)              /*!< Enable Capture 3 interrupt */    
#define TIMER_CCR1_CAP3FILTER               ((uint32_t)0x0000F000)              /*!< Caputre 3 filter */    

/*******************  Bit definition for TIMER_CR0 register  ******************/
#define TIMER_CR0_VAL                       ((uint32_t)0xFFFFFFFF)              /*!< Capture register 0 */    

/*******************  Bit definition for TIMER_CR1 register  ******************/
#define TIMER_CR1_VAL                       ((uint32_t)0xFFFFFFFF)              /*!< Capture register 1 */    

/*******************  Bit definition for TIMER_CR2 register  ******************/
#define TIMER_CR2_VAL                       ((uint32_t)0xFFFFFFFF)              /*!< Capture register 2 */    

/*******************  Bit definition for TIMER_CR3 register  ******************/
#define TIMER_CR3_VAL                       ((uint32_t)0xFFFFFFFF)              /*!< Capture register 3 */    

/*******************  Bit definition for TIMER_EMR register  ******************/
#define TIMER_EMR_EM0                       ((uint32_t)0x00000001)              /*!< External match 0 */
#define TIMER_EMR_EM1                       ((uint32_t)0x00000002)              /*!< External match 1 */
#define TIMER_EMR_EM2                       ((uint32_t)0x00000004)              /*!< External match 2 */
#define TIMER_EMR_EM3                       ((uint32_t)0x00000008)              /*!< External match 3 */
#define TIMER_EMR_EMC0                      ((uint32_t)0x00000030)              /*!< External match 0 configuration */
#define TIMER_EMR_EMC1                      ((uint32_t)0x000000C0)              /*!< External match 1 configuration */
#define TIMER_EMR_EMC2                      ((uint32_t)0x00000300)              /*!< External match 2 configuration */
#define TIMER_EMR_EMC3                      ((uint32_t)0x00000C00)              /*!< External match 3 configuration */
#define TIMER_EMR_EMO0                      ((uint32_t)0x00001000)              /*!< Channel 0 default level */
#define TIMER_EMR_EMO1                      ((uint32_t)0x00002000)              /*!< Channel 1 default level */
#define TIMER_EMR_EMO2                      ((uint32_t)0x00004000)              /*!< Channel 2 default level */
#define TIMER_EMR_EMO3                      ((uint32_t)0x00008000)              /*!< Channel 3 default level */

/*******************  Bit definition for TIMER_PWMTH0 register  ***************/
#define TIMER_PWMTH0                        ((uint32_t)0xFFFFFFFF)              /*!< PWM 0 threshold */

/*******************  Bit definition for TIMER_PWMTH1 register  ***************/
#define TIMER_PWMTH1                        ((uint32_t)0xFFFFFFFF)              /*!< PWM 1 threshold */

/*******************  Bit definition for TIMER_PWMTH2 register  ***************/
#define TIMER_PWMTH2                        ((uint32_t)0xFFFFFFFF)              /*!< PWM 2 threshold */

/*******************  Bit definition for TIMER_PWMTH3 register  ***************/
#define TIMER_PWMTH3                        ((uint32_t)0xFFFFFFFF)              /*!< PWM 3 threshold */

/*******************  Bit definition for TIMER_CTCR register  *****************/
#define TIMER_CTCR_MODE0_MASK               ((uint32_t)0x00000003)              /*!< TC0 mode */
#define TIMER_CTCR_MODE0_TIMER              ((uint32_t)0x00000000)              /*!< Timer  */
#define TIMER_CTCR_MODE0_COUNTR_RE          ((uint32_t)0x00000001)              /*!< Counter rising edge */
#define TIMER_CTCR_MODE0_COUNTR_FE          ((uint32_t)0x00000002)              /*!< Counter falling edge */
#define TIMER_CTCR_MODE0_COUNTR_DE          ((uint32_t)0x00000003)              /*!< Counter double edge */
#define TIMER_CTCR_MODE1_MASK               ((uint32_t)0x0000000C)              /*!< TC1 mode */
#define TIMER_CTCR_MODE1_TIMER              ((uint32_t)0x00000000)              /*!< Timer  */
#define TIMER_CTCR_MODE1_COUNTR_RE          ((uint32_t)0x00000004)              /*!< Counter rising edge */
#define TIMER_CTCR_MODE1_COUNTR_FE          ((uint32_t)0x00000008)              /*!< Counter falling edge */
#define TIMER_CTCR_MODE1_COUNTR_DE          ((uint32_t)0x0000000C)              /*!< Counter double edge */
#define TIMER_CTCR_MODE2_MASK               ((uint32_t)0x00000030)              /*!< TC2 mode */
#define TIMER_CTCR_MODE2_TIMER              ((uint32_t)0x00000000)              /*!< Timer  */
#define TIMER_CTCR_MODE2_COUNTR_RE          ((uint32_t)0x00000010)              /*!< Counter rising edge */
#define TIMER_CTCR_MODE2_COUNTR_FE          ((uint32_t)0x00000020)              /*!< Counter falling edge */
#define TIMER_CTCR_MODE2_COUNTR_DE          ((uint32_t)0x00000030)              /*!< Counter double edge */
#define TIMER_CTCR_MODE3_MASK               ((uint32_t)0x000000C0)              /*!< TC3 mode */
#define TIMER_CTCR_MODE3_TIMER              ((uint32_t)0x00000000)              /*!< Timer  */
#define TIMER_CTCR_MODE3_COUNTR_RE          ((uint32_t)0x00000040)              /*!< Counter rising edge */
#define TIMER_CTCR_MODE3_COUNTR_FE          ((uint32_t)0x00000080)              /*!< Counter falling edge */
#define TIMER_CTCR_MODE3_COUNTR_DE          ((uint32_t)0x000000C0)              /*!< Counter double edge */

/*******************  Bit definition for TIMER_PWMCR register  ****************/
#define TIMER_PWMCR_EN0                     ((uint32_t)0x00000001)              /*!< Enable PWM0 */
#define TIMER_PWMCR_EN1                     ((uint32_t)0x00000002)              /*!< Enable PWM1 */
#define TIMER_PWMCR_EN2                     ((uint32_t)0x00000004)              /*!< Enable PWM2 */
#define TIMER_PWMCR_EN3                     ((uint32_t)0x00000008)              /*!< Enable PWM3 */
#define TIMER_PWMCR_PULSE_COUNTER0          ((uint32_t)0x00000010)              /*!< Enable pulse counter 0 */
#define TIMER_PWMCR_PULSE_COUNTER1          ((uint32_t)0x00000020)              /*!< Enable pulse counter 1 */
#define TIMER_PWMCR_PULSE_COUNTER2          ((uint32_t)0x00000040)              /*!< Enable pulse counter 2 */
#define TIMER_PWMCR_PULSE_COUNTER3          ((uint32_t)0x00000080)              /*!< Enable pulse counter 3 */
#define TIMER_PWMCR_A0HIZ                   ((uint32_t)0x00000100)              /*!< CH0 Active level */
#define TIMER_PWMCR_INA0HIZ                 ((uint32_t)0x00000200)              /*!< CH0 Inactive level */
#define TIMER_PWMCR_A1HIZ                   ((uint32_t)0x00000400)              /*!< CH1 Active level */
#define TIMER_PWMCR_INA1HIZ                 ((uint32_t)0x00000800)              /*!< CH1 Inactive level */
#define TIMER_PWMCR_A2HIZ                   ((uint32_t)0x00001000)              /*!< CH2 Active level */
#define TIMER_PWMCR_INA2HIZ                 ((uint32_t)0x00002000)              /*!< CH2 Inactive level */
#define TIMER_PWMCR_A3HIZ                   ((uint32_t)0x00004000)              /*!< CH3 Active level */
#define TIMER_PWMCR_INA3HIZ                 ((uint32_t)0x00008000)              /*!< CH3 Inactive level */
#define TIMER_PWMCR_INV0                    ((uint32_t)0x00010000)              /*!< Invert CH0 */
#define TIMER_PWMCR_INV1                    ((uint32_t)0x00020000)              /*!< Invert CH1 */
#define TIMER_PWMCR_INV2                    ((uint32_t)0x00040000)              /*!< Invert CH2 */
#define TIMER_PWMCR_INV3                    ((uint32_t)0x00080000)              /*!< Invert CH3 */
#define TIMER_PWMCR_PULSE_COUNTER0_INT      ((uint32_t)0x00100000)              /*!< Enable pwm channel0 pulse counter interrupt */
#define TIMER_PWMCR_PULSE_COUNTER1_INT      ((uint32_t)0x00200000)              /*!< Enable pwm channel1 pulse counter interrupt */
#define TIMER_PWMCR_PULSE_COUNTER2_INT      ((uint32_t)0x00400000)              /*!< Enable pwm channel2 pulse counter interrupt */
#define TIMER_PWMCR_PULSE_COUNTER3_INT      ((uint32_t)0x00800000)              /*!< Enable pwm channel3 pulse counter interrupt */

/*******************  Bit definition for TIMER_PWMV0 register  ****************/
#define TIMER_PWMV0_VALUE_MASK              ((uint32_t)0xFFFFFFFF)              /*!< PWM channel0 count */

/*******************  Bit definition for TIMER_PWMV1 register  ****************/
#define TIMER_PWMV1_VALUE_MASK              ((uint32_t)0xFFFFFFFF)              /*!< PWM channel1 count */

/*******************  Bit definition for TIMER_PWMV2 register  ****************/
#define TIMER_PWMV2_VALUE_MASK              ((uint32_t)0xFFFFFFFF)              /*!< PWM channel2 count */

/*******************  Bit definition for TIMER_PWMV3 register  ****************/
#define TIMER_PWMV3_VALUE_MASK              ((uint32_t)0xFFFFFFFF)              /*!< PWM channel3 count */

/******************************************************************************/
/*                                                                            */
/*                                    MCPWM                                   */
/*                                                                            */
/******************************************************************************/
/*******************  Bit definition for MCPWM_CON register  ******************/
#define MCPWM_CON_RUN0                      ((uint32_t)0x00000001)          /*!< Start or stop mcpwm0 */
#define MCPWM_CON_CENTER0                   ((uint32_t)0x00000002)          /*!< Channel 0 Edge-align or center-align */
#define MCPWM_CON_POLA0                     ((uint32_t)0x00000004)          /*!< Channel 0 status level */
#define MCPWM_CON_DTE0                      ((uint32_t)0x00000008)          /*!< Enable or disable dead time of channel 0 */
#define MCPWM_CON_DISUP0                    ((uint32_t)0x00000010)          /*!< Enable or disable update function of channel 0 */
#define MCPWM_CON_CH0EN                     ((uint32_t)0x00000020)          /*!< Default voltage */
#define MCPWM_CON_A0LVL                     ((uint32_t)0x00000040)          /*!< A0 default voltage */
#define MCPWM_CON_B0LVL                     ((uint32_t)0x00000080)          /*!< B0 default voltage */
#define MCPWM_CON_RUN1                      ((uint32_t)0x00000100)          /*!< Start or stop mcpwm1 */
#define MCPWM_CON_CENTER1                   ((uint32_t)0x00000200)          /*!< Channel 1 Edge-align or center-align */
#define MCPWM_CON_POLA1                     ((uint32_t)0x00000400)          /*!< Channel 1 status level */
#define MCPWM_CON_DTE1                      ((uint32_t)0x00000800)          /*!< Enable or disable dead time of channel 1 */
#define MCPWM_CON_DISUP1                    ((uint32_t)0x00001000)          /*!< Enable or disable update function of channel 1 */
#define MCPWM_CON_CH1EN                     ((uint32_t)0x00002000)          /*!< Default voltage */
#define MCPWM_CON_A1LVL                     ((uint32_t)0x00004000)          /*!< A1 default voltage */
#define MCPWM_CON_B1LVL                     ((uint32_t)0x00008000)          /*!< B1 default voltage */
#define MCPWM_CON_RUN2                      ((uint32_t)0x00010000)          /*!< Start or stop mcpwm2 */
#define MCPWM_CON_CENTER2                   ((uint32_t)0x00020000)          /*!< Channel 2 Edge-align or center-align */
#define MCPWM_CON_POLA2                     ((uint32_t)0x00040000)          /*!< Channel 2 status level */
#define MCPWM_CON_DTE2                      ((uint32_t)0x00080000)          /*!< Enable or disable dead time of channel 2 */
#define MCPWM_CON_DISUP2                    ((uint32_t)0x00100000)          /*!< Enable or disable update function of channel 2 */
#define MCPWM_CON_CH2EN                     ((uint32_t)0x00200000)          /*!< Default voltage */
#define MCPWM_CON_A2LVL                     ((uint32_t)0x00400000)          /*!< A2 default voltage */
#define MCPWM_CON_B2LVL                     ((uint32_t)0x00800000)          /*!< B2 default voltage */
#define MCPWM_CON_HALL                      ((uint32_t)0x01000000)          /*!< Enable or disable HALL function */
#define MCPWM_CON_VEL                       ((uint32_t)0x02000000)          /*!< Enable or disable measuring speed function */
#define MCPWM_CON_HALLS_DIS                 ((uint32_t)0x10000000)          /*!< Enable or disable auto update halls */
#define MCPWM_CON_CHGPH                     ((uint32_t)0x10000000)          /*!< Phase ShiftPhase */
#define MCPWM_CON_INVBDC                    ((uint32_t)0x20000000)          /*!< Polarity of MCOB */
#define MCPWM_CON_ACMODE                    ((uint32_t)0x40000000)          /*!< AC mode */
#define MCPWM_CON_DCMODE                    ((uint32_t)0x80000000)          /*!< DC mode */

/*******************  Bit definition for MCPWM_CAP register  ******************/
#define MCPWM_CAPCON_C0_SOURCE              ((uint32_t)0x0000003F)          /*!< Channel 0 capture source */
#define MCPWM_CAPCON_C0T0_RE                ((uint32_t)0x00000001)          /*!< Capture channel 0 when MCI0 rising edge */
#define MCPWM_CAPCON_C0T0_FE                ((uint32_t)0x00000002)          /*!< Capture channel 0 when MCI0 falling edge */
#define MCPWM_CAPCON_C0T1_RE                ((uint32_t)0x00000004)          /*!< Capture channel 0 when MCI1 rising edge */
#define MCPWM_CAPCON_C0T1_FE                ((uint32_t)0x00000008)          /*!< Capture channel 0 when MCI1 falling edge */
#define MCPWM_CAPCON_C0T2_RE                ((uint32_t)0x00000010)          /*!< Capture channel 0 when MCI2 rising edge */
#define MCPWM_CAPCON_C0T2_FE                ((uint32_t)0x00000020)          /*!< Capture channel 0 when MCI2 falling edge */
#define MCPWM_CAPCON_C1_SOURCE              ((uint32_t)0x00000FC0)          /*!< Channel 1 capture source */
#define MCPWM_CAPCON_C1T0_RE                ((uint32_t)0x00000040)          /*!< Capture channel 1 when MCI0 rising edge */
#define MCPWM_CAPCON_C1T0_FE                ((uint32_t)0x00000080)          /*!< Capture channel 1 when MCI0 falling edge */
#define MCPWM_CAPCON_C1T1_RE                ((uint32_t)0x00000100)          /*!< Capture channel 1 when MCI1 rising edge */
#define MCPWM_CAPCON_C1T1_FE                ((uint32_t)0x00000200)          /*!< Capture channel 1 when MCI1 falling edge */
#define MCPWM_CAPCON_C1T2_RE                ((uint32_t)0x00000400)          /*!< Capture channel 1 when MCI2 rising edge */
#define MCPWM_CAPCON_C1T2_FE                ((uint32_t)0x00000800)          /*!< Capture channel 1 when MCI2 falling edge */
#define MCPWM_CAPCON_C2_SOURCE              ((uint32_t)0x0003F000)          /*!< Channel 2 capture source */
#define MCPWM_CAPCON_C2T0_RE                ((uint32_t)0x00001000)          /*!< Capture channel 2 when MCI0 rising edge */
#define MCPWM_CAPCON_C2T0_FE                ((uint32_t)0x00002000)          /*!< Capture channel 2 when MCI0 falling edge */
#define MCPWM_CAPCON_C2T1_RE                ((uint32_t)0x00004000)          /*!< Capture channel 2 when MCI1 rising edge */
#define MCPWM_CAPCON_C2T1_FE                ((uint32_t)0x00008000)          /*!< Capture channel 2 when MCI1 falling edge */
#define MCPWM_CAPCON_C2T2_RE                ((uint32_t)0x00010000)          /*!< Capture channel 2 when MCI2 rising edge */
#define MCPWM_CAPCON_C2T2_FE                ((uint32_t)0x00020000)          /*!< Capture channel 2 when MCI2 falling edge */
#define MCPWM_CAPCON_RT0                    ((uint32_t)0x00040000)          /*!< Reset TC when capturing channel 0 */
#define MCPWM_CAPCON_RT1                    ((uint32_t)0x00080000)          /*!< Reset TC when capturing channel 1 */
#define MCPWM_CAPCON_RT2                    ((uint32_t)0x00100000)          /*!< Reset TC when capturing channel 2 */
#define MCPWM_CAPCON_HNFCAP0                ((uint32_t)0x00200000)          /*!< Enable or disable noise filter of channel 0 */
#define MCPWM_CAPCON_HNFCAP1                ((uint32_t)0x00400000)          /*!< Enable or disable noise filter of channel 0 */
#define MCPWM_CAPCON_HNFCAP2                ((uint32_t)0x00800000)          /*!< Enable or disable noise filter of channel 0 */

/*******************  Bit definition for MCPWM_TC0 register  ******************/
#define MCPWM_TC0                           ((uint32_t)0xFFFFFFFF)          /*!< TC0 */

/*******************  Bit definition for MCPWM_TC1 register  ******************/
#define MCPWM_TC1                           ((uint32_t)0xFFFFFFFF)          /*!< TC1 */

/*******************  Bit definition for MCPWM_TC2 register  ******************/
#define MCPWM_TC2                           ((uint32_t)0xFFFFFFFF)          /*!< TC2 */

/*******************  Bit definition for MCPWM_LIM0 register  *****************/
#define MCPWM_LIM0                          ((uint32_t)0xFFFFFFFF)          /*!< LIM0 */

/*******************  Bit definition for MCPWM_LIM1 register  *****************/
#define MCPWM_LIM1                          ((uint32_t)0xFFFFFFFF)          /*!< LIM1 */

/*******************  Bit definition for MCPWM_LIM2 register  *****************/
#define MCPWM_LIM2                          ((uint32_t)0xFFFFFFFF)          /*!< LIM2 */

/*******************  Bit definition for MCPWM_MAT0 register  *****************/
#define MCPWM_MAT0                          ((uint32_t)0xFFFFFFFF)          /*!< MAT0 */

/*******************  Bit definition for MCPWM_MAT1 register  *****************/
#define MCPWM_MAT1                          ((uint32_t)0xFFFFFFFF)          /*!< MAT1 */

/*******************  Bit definition for MCPWM_MAT2 register  *****************/
#define MCPWM_MAT2                          ((uint32_t)0xFFFFFFFF)          /*!< MAT2 */

/*******************  Bit definition for MCPWM_DT register  *******************/
#define MCPWM_DT_DT0                        ((uint32_t)0x000003FF)          /*!< Dead time 0 */
#define MCPWM_DT_DT1                        ((uint32_t)0x000FFC00)          /*!< Dead time 1 */
#define MCPWM_DT_DT2                        ((uint32_t)0x3FF00000)          /*!< Dead time 2 */

/*******************  Bit definition for MCPWM_CP register  *******************/
#define MCPWM_CP_CPA0                       ((uint32_t)0x00000001)          /*!< MCOA0 signal */
#define MCPWM_CP_CPB0                       ((uint32_t)0x00000002)          /*!< MCOB0 signal */
#define MCPWM_CP_CPA1                       ((uint32_t)0x00000004)          /*!< MCOA1 signal */
#define MCPWM_CP_CPB1                       ((uint32_t)0x00000008)          /*!< MCOB1 signal */
#define MCPWM_CP_CPA2                       ((uint32_t)0x00000010)          /*!< MCOA2 signal */
#define MCPWM_CP_CPB2                       ((uint32_t)0x00000020)          /*!< MCOB2 signal */

/*******************  Bit definition for MCPWM_CAP0 register  *****************/
#define MCPWM_CAP0                          ((uint32_t)0xFFFFFFFF)          /*!< Capture register of channel 0 */

/*******************  Bit definition for MCPWM_CAP1 register  *****************/
#define MCPWM_CAP1                          ((uint32_t)0xFFFFFFFF)          /*!< Capture register of channel 1 */

/*******************  Bit definition for MCPWM_CAP2 register  *****************/
#define MCPWM_CAP2                          ((uint32_t)0xFFFFFFFF)          /*!< Capture register of channel 2 */

/*******************  Bit definition for MCPWM_INTEN register  ****************/
#define MCPWM_INTEN_ILIM0                   ((uint32_t)0x00000001)          /*!< Channel 0 limit interrupt */
#define MCPWM_INTEN_IMAT0                   ((uint32_t)0x00000002)          /*!< Channel 0 match interrupt */
#define MCPWM_INTEN_ICAP0                   ((uint32_t)0x00000004)          /*!< Channel 0 capture interrupt */
#define MCPWM_INTEN_ILIM1                   ((uint32_t)0x00000010)          /*!< Channel 1 limit interrupt */
#define MCPWM_INTEN_IMAT1                   ((uint32_t)0x00000020)          /*!< Channel 1 match interrupt */
#define MCPWM_INTEN_ICAP1                   ((uint32_t)0x00000040)          /*!< Channel 1 capture interrupt */
#define MCPWM_INTEN_ILIM2                   ((uint32_t)0x00000100)          /*!< Channel 2 limit interrupt */
#define MCPWM_INTEN_IMAT2                   ((uint32_t)0x00000200)          /*!< Channel 2 match interrupt */
#define MCPWM_INTEN_ICAP2                   ((uint32_t)0x00000400)          /*!< Channel 2 capture interrupt */
#define MCPWM_INTEN_ABORT                   ((uint32_t)0x00001000)          /*!< Abort interrupt */
#define MCPWM_INTEN_STCHG                   ((uint32_t)0x00010000)          /*!< Status change interrupt */
#define MCPWM_INTEN_STERR                   ((uint32_t)0x00020000)          /*!< Status error interrupt */
#define MCPWM_INTEN_VSLOW                   ((uint32_t)0x00040000)          /*!< Velocity low interrupt */
#define MCPWM_INTEN_VFAST                   ((uint32_t)0x00080000)          /*!< Velocity fast interrupt */
#define MCPWM_INTEN_MATR0                   ((uint32_t)0x00100000)          /*!< TC 0->MAT interrupt */
#define MCPWM_INTEN_MATF0                   ((uint32_t)0x00200000)          /*!< TC LIM->MAT interrupt */
#define MCPWM_INTEN_CTZERO0                 ((uint32_t)0x00400000)          /*!< TC MAT->0 interrupt */
#define MCPWM_INTEN_MATR1                   ((uint32_t)0x01000000)          /*!< TC 0->MAT interrupt */
#define MCPWM_INTEN_MATF1                   ((uint32_t)0x02000000)          /*!< TC LIM->MAT interrupt */
#define MCPWM_INTEN_CTZERO1                 ((uint32_t)0x04000000)          /*!< TC MAT->0 interrupt */
#define MCPWM_INTEN_MATR2                   ((uint32_t)0x10000000)          /*!< TC 0->MAT interrupt */
#define MCPWM_INTEN_MATF2                   ((uint32_t)0x20000000)          /*!< TC LIM->MAT interrupt */
#define MCPWM_INTEN_CTZERO2                 ((uint32_t)0x40000000)          /*!< TC MAT->0 interrupt */

/*******************  Bit definition for MCPWM CNTCON register  ***************/
#define MCPWM_CNTCON_TC0_SOURCE             ((uint32_t)0x0000003F)          /*!< TC0 Counter mode */
#define MCPWM_CNTCON_TC0I0_RE               ((uint32_t)0x00000001)          /*!< Counter mode */
#define MCPWM_CNTCON_TC0I0_FE               ((uint32_t)0x00000002)          /*!< Counter mode */
#define MCPWM_CNTCON_TC0I1_RE               ((uint32_t)0x00000004)          /*!< Counter mode */
#define MCPWM_CNTCON_TC0I1_FE               ((uint32_t)0x00000008)          /*!< Counter mode */
#define MCPWM_CNTCON_TC0I2_RE               ((uint32_t)0x00000010)          /*!< Counter mode */
#define MCPWM_CNTCON_TC0I2_FE               ((uint32_t)0x00000020)          /*!< Counter mode */
#define MCPWM_CNTCON_TC1_SOURCE             ((uint32_t)0x00000FC0)          /*!< TC1 Counter mode */
#define MCPWM_CNTCON_TC1I0_RE               ((uint32_t)0x00000040)          /*!< Counter mode */
#define MCPWM_CNTCON_TC1I0_FE               ((uint32_t)0x00000080)          /*!< Counter mode */
#define MCPWM_CNTCON_TC1I1_RE               ((uint32_t)0x00000100)          /*!< Counter mode */
#define MCPWM_CNTCON_TC1I1_FE               ((uint32_t)0x00000200)          /*!< Counter mode */
#define MCPWM_CNTCON_TC1I2_RE               ((uint32_t)0x00000400)          /*!< Counter mode */
#define MCPWM_CNTCON_TC1I2_FE               ((uint32_t)0x00000800)          /*!< Counter mode */
#define MCPWM_CNTCON_TC2_SOURCE             ((uint32_t)0x0003F000)          /*!< TC2 Counter mode */
#define MCPWM_CNTCON_TC2I0_RE               ((uint32_t)0x00001000)          /*!< Counter mode */
#define MCPWM_CNTCON_TC2I0_FE               ((uint32_t)0x00002000)          /*!< Counter mode */
#define MCPWM_CNTCON_TC2I1_RE               ((uint32_t)0x00004000)          /*!< Counter mode */
#define MCPWM_CNTCON_TC2I1_FE               ((uint32_t)0x00008000)          /*!< Counter mode */
#define MCPWM_CNTCON_TC2I2_RE               ((uint32_t)0x00010000)          /*!< Counter mode */
#define MCPWM_CNTCON_TC2I2_FE               ((uint32_t)0x00020000)          /*!< Counter mode */
#define MCPWM_CNTCON_CNTR0                  ((uint32_t)0x20000000)          /*!< Timer or Counter */
#define MCPWM_CNTCON_CNTR1                  ((uint32_t)0x40000000)          /*!< Timer or Counter */
#define MCPWM_CNTCON_CNTR2                  ((uint32_t)0x80000000)          /*!< Timer or Counter */

/*******************  Bit definition for MCPWM INTF register  *****************/
#define MCPWM_INTF_ILIM0                    ((uint32_t)0x00000001)          /*!< Channel 0 limit interrupt flag */
#define MCPWM_INTF_IMAT0                    ((uint32_t)0x00000002)          /*!< Channel 0 match interrupt flag */
#define MCPWM_INTF_ICAP0                    ((uint32_t)0x00000004)          /*!< Channel 0 capture interrupt flag */
#define MCPWM_INTF_ILIM1                    ((uint32_t)0x00000010)          /*!< Channel 1 limit interrupt flag */
#define MCPWM_INTF_IMAT1                    ((uint32_t)0x00000020)          /*!< Channel 1 match interrupt flag */
#define MCPWM_INTF_ICAP1                    ((uint32_t)0x00000040)          /*!< Channel 1 capture interrupt flag */
#define MCPWM_INTF_ILIM2                    ((uint32_t)0x00000100)          /*!< Channel 2 limit interrupt flag */
#define MCPWM_INTF_IMAT2                    ((uint32_t)0x00000200)          /*!< Channel 2 match interrupt flag */
#define MCPWM_INTF_ICAP2                    ((uint32_t)0x00000400)          /*!< Channel 2 capture interrupt flag */
#define MCPWM_INTF_ABORT                    ((uint32_t)0x00001000)          /*!< Abort interrupt flag */
#define MCPWM_INTF_STCHG                    ((uint32_t)0x00010000)          /*!< Status change interrupt flag */
#define MCPWM_INTF_STERR                    ((uint32_t)0x00020000)          /*!< Status error interrupt flag */
#define MCPWM_INTF_VSLOW                    ((uint32_t)0x00040000)          /*!< Velocity low interrupt flag */
#define MCPWM_INTF_VFAST                    ((uint32_t)0x00080000)          /*!< Velocity fast interrupt flag */
#define MCPWM_INTF_MATR0                    ((uint32_t)0x00100000)          /*!< TC 0->MAT interrupt flag */
#define MCPWM_INTF_MATF0                    ((uint32_t)0x00200000)          /*!< TC LIM->MAT interrupt flag */
#define MCPWM_INTF_CTZERO0                  ((uint32_t)0x00400000)          /*!< TC MAT->0 interrupt flag */
#define MCPWM_INTF_MATR1                    ((uint32_t)0x01000000)          /*!< TC 0->MAT interrupt flag */
#define MCPWM_INTF_MATF1                    ((uint32_t)0x02000000)          /*!< TC LIM->MAT interrupt flag */
#define MCPWM_INTF_CTZERO1                  ((uint32_t)0x04000000)          /*!< TC MAT->0 interrupt flag */
#define MCPWM_INTF_MATR2                    ((uint32_t)0x10000000)          /*!< TC 0->MAT interrupt flag */
#define MCPWM_INTF_MATF2                    ((uint32_t)0x20000000)          /*!< TC LIM->MAT interrupt flag */
#define MCPWM_INTF_CTZERO2                  ((uint32_t)0x40000000)          /*!< TC MAT->0 interrupt flag */

/*******************  Bit definition for MCPWM CAPCLR register  ***************/
#define MCPWM_CAPCLR_CLR0                   ((uint32_t)0x00000001)          /*!< Clear CR0 */
#define MCPWM_CAPCLR_CLR1                   ((uint32_t)0x00000002)          /*!< Clear CR1 */
#define MCPWM_CAPCLR_CLR2                   ((uint32_t)0x00000004)          /*!< Clear CR2 */

/*******************  Bit definition for MCPWM HALL register  *****************/
#define MCPWM_HALL_MCP_MASK                 ((uint32_t)0x0000003F)          /*!< MCP MASK */
#define MCPWM_HALL_MCPA0                    ((uint32_t)0x00000001)          /*!< Enable or disable MCOA0 */
#define MCPWM_HALL_MCPB0                    ((uint32_t)0x00000002)          /*!< Enable or disable MCOB0 */
#define MCPWM_HALL_MCPA1                    ((uint32_t)0x00000004)          /*!< Enable or disable MCOA2 */
#define MCPWM_HALL_MCPB1                    ((uint32_t)0x00000008)          /*!< Enable or disable MCOB1 */
#define MCPWM_HALL_MCPA2                    ((uint32_t)0x00000010)          /*!< Enable or disable MCOA2 */
#define MCPWM_HALL_MCPB2                    ((uint32_t)0x00000020)          /*!< Enable or disable MCOB2 */
#define MCPWM_HALL_INS_MASK                 ((uint32_t)0x00003F00)          /*!< Inactive status MASK */
#define MCPWM_HALL_PSLA0                    ((uint32_t)0x00000100)          /*!< Inactive status of MCOA0 */
#define MCPWM_HALL_PSLB0                    ((uint32_t)0x00000200)          /*!< Inactive status of MCOB0 */
#define MCPWM_HALL_PSLA1                    ((uint32_t)0x00000400)          /*!< Inactive status of MCOA1 */
#define MCPWM_HALL_PSLB1                    ((uint32_t)0x00000800)          /*!< Inactive status of MCOB1 */
#define MCPWM_HALL_PSLA2                    ((uint32_t)0x00001000)          /*!< Inactive status of MCOA2 */
#define MCPWM_HALL_PSLB2                    ((uint32_t)0x00002000)          /*!< Inactive status of MCOB2 */
#define MCPWM_HALL_EXPH                     ((uint32_t)0x00070000)          /*!< Expected HALL status */
#define MCPWM_HALL_CURH                     ((uint32_t)0x00700000)          /*!< Current HALL status */

/*******************  Bit definition for MCPWM HALLS register  *****************/
#define MCPWM_HALLS_MCPA0                   ((uint32_t)0x00000001)          /*!< Enable or disable MCOA0 (SHADOW) */
#define MCPWM_HALLS_MCPB0                   ((uint32_t)0x00000002)          /*!< Enable or disable MCOB0 (SHADOW) */
#define MCPWM_HALLS_MCPA1                   ((uint32_t)0x00000004)          /*!< Enable or disable MCOA2 (SHADOW) */
#define MCPWM_HALLS_MCPB1                   ((uint32_t)0x00000008)          /*!< Enable or disable MCOB1 (SHADOW) */
#define MCPWM_HALLS_MCPA2                   ((uint32_t)0x00000010)          /*!< Enable or disable MCOA2 (SHADOW) */
#define MCPWM_HALLS_MCPB2                   ((uint32_t)0x00000020)          /*!< Enable or disable MCOB2 (SHADOW) */
#define MCPWM_HALLS_PSLA0                   ((uint32_t)0x00000100)          /*!< Inactive status of MCOA0 (SHADOW) */
#define MCPWM_HALLS_PSLB0                   ((uint32_t)0x00000200)          /*!< Inactive status of MCOB0 (SHADOW) */
#define MCPWM_HALLS_PSLA1                   ((uint32_t)0x00000400)          /*!< Inactive status of MCOA1 (SHADOW) */
#define MCPWM_HALLS_PSLB1                   ((uint32_t)0x00000800)          /*!< Inactive status of MCOB1 (SHADOW) */
#define MCPWM_HALLS_PSLA2                   ((uint32_t)0x00001000)          /*!< Inactive status of MCOA2 (SHADOW) */
#define MCPWM_HALLS_PSLB2                   ((uint32_t)0x00002000)          /*!< Inactive status of MCOB2 (SHADOW) */
#define MCPWM_HALLS_EXPH                    ((uint32_t)0x00010000)          /*!< Expected HALL status (SHADOW) */
#define MCPWM_HALLS_CURH                    ((uint32_t)0x00100000)          /*!< Current HALL status (SHADOW) */

/*******************  Bit definition for MCPWM VELCMP register  ***************/
#define MCPWM_VELCMP                        ((uint32_t)0xFFFFFFFF)          /*!< Velocity comparison value */

/*******************  Bit definition for MCPWM VELVAL register  ***************/
#define MCPWM_VELVAL                        ((uint32_t)0xFFFFFFFF)          /*!< Current velocity value */

/*******************  Bit definition for MCPWM TH register  *******************/
#define MCPWM_TH                            ((uint32_t)0xFFFFFFFF)          /*!< Velocity threshold value */

/*******************  Bit definition for MCPWM MCSIT register  ****************/
#define MCPWM_MCIST                         ((uint32_t)0x00000007)          /*!< MCI level */
#define MCPWM_MCIST_MCI2                    ((uint32_t)0x00000001)          /*!< MCI0 level */
#define MCPWM_MCIST_MCI1                    ((uint32_t)0x00000002)          /*!< MCI1 level */
#define MCPWM_MCIST_MCI0                    ((uint32_t)0x00000004)          /*!< MCI2 level */

/*******************  Bit definition for MCPWM_CHGPH register  ****************/
#define MCPWM_CHGPH_CH1                     ((uint32_t)0x0000FFFF)          /*!< CHGPH */
#define MCPWM_CHGPH_CH2                     ((uint32_t)0xFFFF0000)          /*!< CHGPH */

/*******************  Bit definition for MCPWM_HALLAEN register  **************/
#define MCPWM_HALLAEN_ENABLE                ((uint32_t)0x00000001)          /*!< HALLA Enable */
#define MCPWM_HALLAEN_DISABLE               ((uint32_t)0x00000000)          /*!< HALLA Disable */

/*******************  Bit definition for MCPWM_HALLA register  ****************/
#define MCPWM_HALLA_DLY_CNT                 ((uint32_t)0x000FFFFF)          /*!< HALLA delay count */
#define MCPWM_HALLA_MCPA0                   ((uint32_t)0x00100000)          /*!< HALLA MCPA0 */
#define MCPWM_HALLA_MCPB0                   ((uint32_t)0x00200000)          /*!< HALLA MCPB0 */
#define MCPWM_HALLA_MCPA1                   ((uint32_t)0x00400000)          /*!< HALLA MCPA1 */
#define MCPWM_HALLA_MCPB1                   ((uint32_t)0x00800000)          /*!< HALLA MCPB1 */
#define MCPWM_HALLA_MCPA2                   ((uint32_t)0x01000000)          /*!< HALLA MCPA2 */
#define MCPWM_HALLA_MCPB2                   ((uint32_t)0x02000000)          /*!< HALLA MCPB2 */
#define MCPWM_HALLA_PSLA0                   ((uint32_t)0x04000000)          /*!< HALLA PSLA0 */
#define MCPWM_HALLA_PSLB0                   ((uint32_t)0x08000000)          /*!< HALLA PSLB0 */
#define MCPWM_HALLA_PSLA1                   ((uint32_t)0x10000000)          /*!< HALLA PSLA1 */
#define MCPWM_HALLA_PSLB1                   ((uint32_t)0x20000000)          /*!< HALLA PSLB1 */
#define MCPWM_HALLA_PSLA2                   ((uint32_t)0x40000000)          /*!< HALLA PSLA2 */
#define MCPWM_HALLA_PSLB2                   ((uint32_t)0x80000000)          /*!< HALLA PSLB2 */

/*******************  Bit definition for MCPWM_ADC_TRIG0 register  ***********/
#define MCPWM_ADC_TRIG0_TRIG_CNT            ((uint32_t)0x0000FFFF)          /*!< ADC_TRIG0 count */
#define MCPWM_ADC_TRIG0_DIR                 ((uint32_t)0x40000000)          /*!< ADC_TRIG0 trigger dir */
#define MCPWM_ADC_TRIG0_EN                  ((uint32_t)0x80000000)          /*!< ADC_TRIG0 enable */

/*******************  Bit definition for MCPWM_ADC_TRIG1 register  ***********/
#define MCPWM_ADC_TRIG1_TRIG_CNT            ((uint32_t)0x0000FFFF)          /*!< ADC_TRIG1 count */
#define MCPWM_ADC_TRIG1_DIR                 ((uint32_t)0x40000000)          /*!< ADC_TRIG1 trigger dir */
#define MCPWM_ADC_TRIG1_EN                  ((uint32_t)0x80000000)          /*!< ADC_TRIG1 enable */

/*******************  Bit definition for MCPWM_ADC_TRIG2 register  ***********/
#define MCPWM_ADC_TRIG2_TRIG_CNT            ((uint32_t)0x0000FFFF)          /*!< ADC_TRIG2 count */
#define MCPWM_ADC_TRIG2_DIR                 ((uint32_t)0x40000000)          /*!< ADC_TRIG2 trigger dir */
#define MCPWM_ADC_TRIG2_EN                  ((uint32_t)0x80000000)          /*!< ADC_TRIG2 enable */


/******************************************************************************/
/*                                                                            */
/*                           Independent WATCHDOG                             */
/*                                                                            */
/******************************************************************************/

/*******************  Bit definition for IWDG_KR register  ********************/
#define IWDG_KR_KEY                         ((uint16_t)0xFFFF)              /*!< Key value (write only, read 0000h) */

/*******************  Bit definition for IWDG_PR register  ********************/
#define IWDG_PR_PR                          ((uint8_t)0x07)                 /*!< PR[2:0] (Prescaler divider) */
#define IWDG_PR_PR_0                        ((uint8_t)0x01)                 /*!< Bit 0 */
#define IWDG_PR_PR_1                        ((uint8_t)0x02)                 /*!< Bit 1 */
#define IWDG_PR_PR_2                        ((uint8_t)0x04)                 /*!< Bit 2 */

/*******************  Bit definition for IWDG_RLR register  *******************/
#define IWDG_RLR_RL                         ((uint16_t)0x0FFF)              /*!< Watchdog counter reload value */

/*******************  Bit definition for IWDG_SR register  ********************/
#define IWDG_SR_PVU                         ((uint8_t)0x01)                 /*!< Watchdog prescaler value update */
#define IWDG_SR_RVU                         ((uint8_t)0x02)                 /*!< Watchdog counter reload value update */

/******************************************************************************/
/*                                                                            */
/*                      Touch Senser Controller (TSC)                         */
/*                                                                            */
/******************************************************************************/

/*******************  Bit definition for TSC_CR register  *********************/
#define TSC_CR_DISCARD_MASK                 ((uint32_t)0x00000003)          /*!< Mask of Discard */
#define TSC_CR_DISCARD_0                    ((uint32_t)0x00000000)          /*!< Discard 0 */
#define TSC_CR_DISCARD_1                    ((uint32_t)0x00000001)          /*!< Discard 1 */
#define TSC_CR_DISCARD_2                    ((uint32_t)0x00000002)          /*!< Discard 2 */
#define TSC_CR_DISCARD_3                    ((uint32_t)0x00000003)          /*!< Discard 3 */
#define TSC_CR_SCAN                         ((uint32_t)0x00000010)          /*!< Scan mode */
#define TSC_CR_POLL                         ((uint32_t)0x00000100)          /*!< Poll mode */
#define TSC_CR_DIG_PREDIV                   ((uint32_t)0x0000F000)          /*!< Digital prediv */
#define TSC_CR_ANA_ENABLE                   ((uint32_t)0x00010000)          /*!< Analog enable bit */
#define TSC_CR_ANA_PREDIV                   ((uint32_t)0x0F000000)          /*!< Analog prediv */

/*******************  Bit definition for TSC_ENABLE register  *****************/
#define TSC_ENABLE_PIO0_0                   ((uint32_t)0x00000001)          /*!< Enable PIO */
#define TSC_ENABLE_PIO0_1                   ((uint32_t)0x00000002)          /*!< Enable PIO */
#define TSC_ENABLE_PIO0_2                   ((uint32_t)0x00000004)          /*!< Enable PIO */
#define TSC_ENABLE_PIO0_3                   ((uint32_t)0x00000008)          /*!< Enable PIO */
#define TSC_ENABLE_PIO0_4                   ((uint32_t)0x00000010)          /*!< Enable PIO */
#define TSC_ENABLE_PIO0_5                   ((uint32_t)0x00000020)          /*!< Enable PIO */
#define TSC_ENABLE_PIO0_6                   ((uint32_t)0x00000040)          /*!< Enable PIO */
#define TSC_ENABLE_PIO0_7                   ((uint32_t)0x00000080)          /*!< Enable PIO */
#define TSC_ENABLE_PIO1_0                   ((uint32_t)0x00000100)          /*!< Enable PIO */
#define TSC_ENABLE_PIO1_1                   ((uint32_t)0x00000200)          /*!< Enable PIO */
#define TSC_ENABLE_PIO1_2                   ((uint32_t)0x00000400)          /*!< Enable PIO */
#define TSC_ENABLE_PIO1_3                   ((uint32_t)0x00000800)          /*!< Enable PIO */
#define TSC_ENABLE_PIO1_4                   ((uint32_t)0x00001000)          /*!< Enable PIO */
#define TSC_ENABLE_PIO1_5                   ((uint32_t)0x00002000)          /*!< Enable PIO */
#define TSC_ENABLE_PIO1_6                   ((uint32_t)0x00004000)          /*!< Enable PIO */
#define TSC_ENABLE_PIO1_7                   ((uint32_t)0x00008000)          /*!< Enable PIO */
#define TSC_ENABLE_PIO2_0                   ((uint32_t)0x00010000)          /*!< Enable PIO */
#define TSC_ENABLE_PIO2_1                   ((uint32_t)0x00020000)          /*!< Enable PIO */
#define TSC_ENABLE_PIO2_2                   ((uint32_t)0x00040000)          /*!< Enable PIO */
#define TSC_ENABLE_PIO2_3                   ((uint32_t)0x00080000)          /*!< Enable PIO */

/*******************  Bit definition for TSC_IER register  ********************/
#define TSC_IER_ENABLE                      ((uint32_t)0x00000001)          /*!< Enable interrupt */

/*******************  Bit definition for TSC_ICR register  ********************/
#define TSC_ICR_CLEAR                       ((uint32_t)0x00000001)          /*!< Clear interrupt flag */

/*******************  Bit definition for TSC_ISR register  ********************/
#define TSC_ISR_STATUS                      ((uint32_t)0x00000001)          /*!< Interrupt status flag */

/*******************  Bit definition for TSC_STAT register  *******************/
#define TSC_STAT_PIO0_0                     ((uint32_t)0x00000001)          /*!< TSC PIO flag */
#define TSC_STAT_PIO0_1                     ((uint32_t)0x00000002)          /*!< TSC PIO flag */
#define TSC_STAT_PIO0_2                     ((uint32_t)0x00000004)          /*!< TSC PIO flag */
#define TSC_STAT_PIO0_3                     ((uint32_t)0x00000008)          /*!< TSC PIO flag */
#define TSC_STAT_PIO0_4                     ((uint32_t)0x00000010)          /*!< TSC PIO flag */
#define TSC_STAT_PIO0_5                     ((uint32_t)0x00000020)          /*!< TSC PIO flag */
#define TSC_STAT_PIO0_6                     ((uint32_t)0x00000040)          /*!< TSC PIO flag */
#define TSC_STAT_PIO0_7                     ((uint32_t)0x00000080)          /*!< TSC PIO flag */
#define TSC_STAT_PIO1_0                     ((uint32_t)0x00000100)          /*!< TSC PIO flag */
#define TSC_STAT_PIO1_1                     ((uint32_t)0x00000200)          /*!< TSC PIO flag */
#define TSC_STAT_PIO1_2                     ((uint32_t)0x00000400)          /*!< TSC PIO flag */
#define TSC_STAT_PIO1_3                     ((uint32_t)0x00000800)          /*!< TSC PIO flag */
#define TSC_STAT_PIO1_4                     ((uint32_t)0x00001000)          /*!< TSC PIO flag */
#define TSC_STAT_PIO1_5                     ((uint32_t)0x00002000)          /*!< TSC PIO flag */
#define TSC_STAT_PIO1_6                     ((uint32_t)0x00004000)          /*!< TSC PIO flag */
#define TSC_STAT_PIO1_7                     ((uint32_t)0x00008000)          /*!< TSC PIO flag */
#define TSC_STAT_PIO2_0                     ((uint32_t)0x00010000)          /*!< TSC PIO flag */
#define TSC_STAT_PIO2_1                     ((uint32_t)0x00020000)          /*!< TSC PIO flag */
#define TSC_STAT_PIO2_2                     ((uint32_t)0x00040000)          /*!< TSC PIO flag */

/*******************  Bit definition for TSC_IOCCR0 register  *****************/
#define TSC_IOCCR0_VAL_PIO0_0_MASK          ((uint32_t)0x00007FFF)          /*!< Mask of TSC PIO value */
#define TSC_IOCCR0_VAL_PIO0_1_MASK          ((uint32_t)0x7FFF0000)          /*!< Mask of TSC PIO value */

/*******************  Bit definition for TSC_IOCCR1 register  *****************/
#define TSC_IOCCR1_VAL_PIO0_2_MASK          ((uint32_t)0x00007FFF)          /*!< Mask of TSC PIO value */
#define TSC_IOCCR1_VAL_PIO0_3_MASK          ((uint32_t)0x7FFF0000)          /*!< Mask of TSC PIO value */

/*******************  Bit definition for TSC_IOCCR2 register  *****************/
#define TSC_IOCCR2_VAL_PIO0_4_MASK          ((uint32_t)0x00007FFF)          /*!< Mask of TSC PIO value */
#define TSC_IOCCR2_VAL_PIO0_5_MASK          ((uint32_t)0x7FFF0000)          /*!< Mask of TSC PIO value */

/*******************  Bit definition for TSC_IOCCR3 register  *****************/
#define TSC_IOCCR3_VAL_PIO0_6_MASK          ((uint32_t)0x00007FFF)          /*!< Mask of TSC PIO value */
#define TSC_IOCCR3_VAL_PIO0_7_MASK          ((uint32_t)0x7FFF0000)          /*!< Mask of TSC PIO value */

/*******************  Bit definition for TSC_IOCCR4 register  *****************/
#define TSC_IOCCR4_VAL_PIO1_0_MASK          ((uint32_t)0x00007FFF)          /*!< Mask of TSC PIO value */
#define TSC_IOCCR4_VAL_PIO1_1_MASK          ((uint32_t)0x7FFF0000)          /*!< Mask of TSC PIO value */

/*******************  Bit definition for TSC_IOCCR5 register  *****************/
#define TSC_IOCCR5_VAL_PIO1_2_MASK          ((uint32_t)0x00007FFF)          /*!< Mask of TSC PIO value */
#define TSC_IOCCR5_VAL_PIO1_3_MASK          ((uint32_t)0x7FFF0000)          /*!< Mask of TSC PIO value */

/*******************  Bit definition for TSC_IOCCR6 register  *****************/
#define TSC_IOCCR6_VAL_PIO1_4_MASK          ((uint32_t)0x00007FFF)          /*!< Mask of TSC PIO value */
#define TSC_IOCCR6_VAL_PIO1_5_MASK          ((uint32_t)0x7FFF0000)          /*!< Mask of TSC PIO value */

/*******************  Bit definition for TSC_IOCCR7 register  *****************/
#define TSC_IOCCR7_VAL_PIO1_6_MASK          ((uint32_t)0x00007FFF)          /*!< Mask of TSC PIO value */
#define TSC_IOCCR7_VAL_PIO1_7_MASK          ((uint32_t)0x7FFF0000)          /*!< Mask of TSC PIO value */

/*******************  Bit definition for TSC_IOCCR8 register  *****************/
#define TSC_IOCCR8_VAL_PIO2_0_MASK          ((uint32_t)0x00007FFF)          /*!< Mask of TSC PIO value */
#define TSC_IOCCR8_VAL_PIO2_1_MASK          ((uint32_t)0x7FFF0000)          /*!< Mask of TSC PIO value */

/*******************  Bit definition for TSC_IOCCR9 register  *****************/
#define TSC_IOCCR9_VAL_PIO2_2_MASK          ((uint32_t)0x00007FFF)          /*!< Mask of TSC PIO value */
#define TSC_IOCCR9_VAL_PIO2_3_MASK          ((uint32_t)0x7FFF0000)          /*!< Mask of TSC PIO value */


/******************************************************************************/
/*                                                                            */
/*                         Window WATCHDOG (WWDG)                             */
/*                                                                            */
/******************************************************************************/

/*******************  Bit definition for WWDG_CR register  ********************/
#define WWDG_CR_T                           ((uint8_t)0x7F)                 /*!< T[6:0] bits (7-Bit counter (MSB to LSB)) */
#define WWDG_CR_T0                          ((uint8_t)0x01)                 /*!< Bit 0 */
#define WWDG_CR_T1                          ((uint8_t)0x02)                 /*!< Bit 1 */
#define WWDG_CR_T2                          ((uint8_t)0x04)                 /*!< Bit 2 */
#define WWDG_CR_T3                          ((uint8_t)0x08)                 /*!< Bit 3 */
#define WWDG_CR_T4                          ((uint8_t)0x10)                 /*!< Bit 4 */
#define WWDG_CR_T5                          ((uint8_t)0x20)                 /*!< Bit 5 */
#define WWDG_CR_T6                          ((uint8_t)0x40)                 /*!< Bit 6 */

#define WWDG_CR_WDGA                        ((uint8_t)0x80)                 /*!< Activation bit */

/*******************  Bit definition for WWDG_CFR register  *******************/
#define WWDG_CFR_W                          ((uint16_t)0x007F)              /*!< W[6:0] bits (7-bit window value) */
#define WWDG_CFR_W0                         ((uint16_t)0x0001)              /*!< Bit 0 */
#define WWDG_CFR_W1                         ((uint16_t)0x0002)              /*!< Bit 1 */
#define WWDG_CFR_W2                         ((uint16_t)0x0004)              /*!< Bit 2 */
#define WWDG_CFR_W3                         ((uint16_t)0x0008)              /*!< Bit 3 */
#define WWDG_CFR_W4                         ((uint16_t)0x0010)              /*!< Bit 4 */
#define WWDG_CFR_W5                         ((uint16_t)0x0020)              /*!< Bit 5 */
#define WWDG_CFR_W6                         ((uint16_t)0x0040)              /*!< Bit 6 */

#define WWDG_CFR_WDGTB                      ((uint16_t)0x0180)              /*!< WDGTB[1:0] bits (Timer Base) */
#define WWDG_CFR_WDGTB0                     ((uint16_t)0x0080)              /*!< Bit 0 */
#define WWDG_CFR_WDGTB1                     ((uint16_t)0x0100)              /*!< Bit 1 */

#define WWDG_CFR_EWI                        ((uint16_t)0x0200)              /*!< Early Wakeup Interrupt */

/*******************  Bit definition for WWDG_SR register  ********************/
#define WWDG_SR_EWIF                        ((uint8_t)0x01)                 /*!< Early Wakeup Interrupt Flag */

/******************************************************************************/
/*                                                                            */
/*                         Simple Timer (ST)                                  */
/*                                                                            */
/******************************************************************************/

/*******************  Bit definition for ST_LOAD_COUNT register  **************/
#define ST_LOAD_COUNT                       ((uint16_t)0xFFFF)              /*!< Simple Timer load count */

/*******************  Bit definition for ST_CURRENT_VALUE register  ***********/
#define ST_CURRENT_VALUE                    ((uint16_t)0xFFFF)              /*!< Simple Timer current value */

/*******************  Bit definition for ST_CTRL register  ********************/
#define ST_CTRL_ENABLE                      ((uint16_t)0x0001)              /*!< Enable Simple Timer */
#define ST_CTRL_MODE                        ((uint16_t)0x0002)              /*!< Simple Timer mode */
#define ST_CTRL_INTR_MASK                   ((uint16_t)0x0004)              /*!< Mask of Simple Timer interrupt bit */

/*******************  Bit definition for ST_EOI register  *********************/
#define ST_EOI                              ((uint16_t)0x0001)              /*!< Read Clears all active interrupts */

/*******************  Bit definition for ST_INT register  *********************/
#define ST_INT_STATUS                       ((uint16_t)0x0001)              /*!< Contains the interrupt status */

/*******************  Bit definition for STS_INT register  ********************/
#define STS_INT_STATUS_0                    ((uint16_t)0x0001)              /*!< Contains the interrupt status for timer0 */
#define STS_INT_STATUS_1                    ((uint16_t)0x0002)              /*!< Contains the interrupt status for timer1 */

/*******************  Bit definition for STS_EOI register  ********************/
#define STS_EOI_0                           ((uint16_t)0x0001)              /*!< Read Clears all active interrupts. */
#define STS_EOI_1                           ((uint16_t)0x0002)              /*!< Read Clears all active interrupts. */

/*******************  Bit definition for ST_RAW_INT register  *****************/
#define STS_RAW_INT_STATUS_0                ((uint16_t)0x0001)              /*!< contains the unmasked interrupt status
                                                                                 of all timers in the component */
#define STS_RAW_INT_STATUS_1                ((uint16_t)0x0002)              /*!< contains the unmasked interrupt status
                                                                                 of all timers in the component */
/******************************************************************************/
/*                                                                            */
/*                                  CRS                                       */
/*                                                                            */
/******************************************************************************/

/**********************  Bit definition for CRS_CR register  ******************/
#define CRS_CR_SYNCOKIE                     ((uint16_t)0x0001)              /*!< SYNC event OK interrupt enable */
#define CRS_CR_SYNCWARNIE                   ((uint16_t)0x0002)              /*!< SYNC warning interrupt enable */
#define CRS_CR_ERRIE                        ((uint16_t)0x0004)              /*!< Synchronization or trimming error interrupt enable */
#define CRS_CR_ESYNCIE                      ((uint16_t)0x0008)              /*!< Expected SYNC interrupt enable */
#define CRS_CR_CEN                          ((uint16_t)0x0020)              /*!< Frequency error counter enable */
#define CRS_CR_AUTOTRIMEN                   ((uint16_t)0x0040)              /*!< Automatic trimming enable */
#define CRS_CR_SWSYNC                       ((uint16_t)0x0080)              /*!< Generate software SYNC event */
#define CRS_CR_TRIM                         ((uint16_t)0xFF00)              /*!< IRC oscillator smooth trimming */

/**********************  Bit definition for CRS_CFGR register  ****************/
#define CRS_CFGR_RELOAD                     ((uint32_t)0x0000FFFF)          /*!< Counter reload value */
#define CRS_CFGR_FELIM                      ((uint32_t)0x00FF0000)          /*!< Frequency error limit */
#define CRS_CFGR_SYNCDIV_MASK               ((uint32_t)0x07000000)          /*!< Mask of SYNC divider */
#define CRS_CFGR_SYNCDIV_1                  ((uint32_t)0x00000000)          /*!< SYNC divider 1 */
#define CRS_CFGR_SYNCDIV_2                  ((uint32_t)0x01000000)          /*!< SYNC divider 2 */
#define CRS_CFGR_SYNCDIV_4                  ((uint32_t)0x02000000)          /*!< SYNC divider 4 */
#define CRS_CFGR_SYNCDIV_8                  ((uint32_t)0x03000000)          /*!< SYNC divider 8 */
#define CRS_CFGR_SYNCDIV_16                 ((uint32_t)0x04000000)          /*!< SYNC divider 16 */
#define CRS_CFGR_SYNCDIV_32                 ((uint32_t)0x05000000)          /*!< SYNC divider 32 */
#define CRS_CFGR_SYNCDIV_64                 ((uint32_t)0x06000000)          /*!< SYNC divider 64 */
#define CRS_CFGR_SYNCDIV_128                ((uint32_t)0x07000000)          /*!< SYNC divider 128 */
#define CRS_CFGR_SYNCSRC_MASK               ((uint32_t)0x30000000)          /*!< Mask of SYNC signal source selection */
#define CRS_CFGR_SYNCSRC_GPIO               ((uint32_t)0x00000000)          /*!< SYNC signal source selection gpio */
#define CRS_CFGR_SYNCSRC_SOF                ((uint32_t)0x20000000)          /*!< SYNC signal source selection usb sof */
#define CRS_CFGR_SYNCPOL                    ((uint32_t)0x80000000)          /*!< SYNC polarity selection */

/**********************  Bit definition for CRS_ISR register  *****************/
#define CRS_ISR_SYNCOKF                     ((uint32_t)0x00000001)          /*!< SYNC event OK flag */
#define CRS_ISR_SYNCWARNF                   ((uint32_t)0x00000002)          /*!< SYNC warning flag */
#define CRS_ISR_ERRF                        ((uint32_t)0x00000004)          /*!< Error flag */
#define CRS_ISR_ESYNCF                      ((uint32_t)0x00000008)          /*!< Expected SYNC flag */
#define CRS_ISR_SYNCERR                     ((uint32_t)0x00000100)          /*!< SYNC error */
#define CRS_ISR_SYNCMISS                    ((uint32_t)0x00000200)          /*!< SYNC missed */
#define CRS_ISR_TRIMOVF                     ((uint32_t)0x00000400)          /*!< Trimming overflow or underflow */
#define CRS_ISR_FEDIR                       ((uint32_t)0x00008000)          /*!< Frequency error direction */
#define CRS_ISR_FECAP                       ((uint32_t)0xFFFF0000)          /*!< Frequency error capture */

/**********************  Bit definition for CRS_ICR register  *****************/
#define CRS_ICR_SYNCOKC                     ((uint32_t)0x00000001)          /*!< SYNC event OK clear flag */
#define CRS_ICR_SYNCWARNC                   ((uint32_t)0x00000002)          /*!< SYNC warning clear flag */
#define CRS_ICR_ERRC                        ((uint32_t)0x00000004)          /*!< Error clear flag */
#define CRS_ICR_ESYNCC                      ((uint32_t)0x00000008)          /*!< Expected SYNC clear flag */

/**
  * @}
  */

 /**
  * @}
  */ 

/** @addtogroup Exported_macro
  * @{
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __MT006_H */

/**
  * @}
  */

  /**
  * @}
  */

/************************ (C) COPYRIGHT MIC *****END OF FILE****/
