/******************************************************************************
 * @file     startup_ARMCM4.c
 * @brief    CMSIS-Core(M) Device Startup File for a Cortex-M7 Device
 * @version  V2.0.0
 * @date     04. June 2019
 ******************************************************************************/
/*
 * Copyright (c) 2009-2019 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "./device.h"

/*----------------------------------------------------------------------------
  Exception / Interrupt Handler Function Prototype
 *----------------------------------------------------------------------------*/
typedef void( *pFunc )( void );

/*----------------------------------------------------------------------------
  External References
 *----------------------------------------------------------------------------*/
extern uint32_t __INITIAL_SP;

extern __NO_RETURN void __PROGRAM_START(void);

/*----------------------------------------------------------------------------
  Internal References
 *----------------------------------------------------------------------------*/

void __NO_RETURN Reset_Handler  (void);

/*----------------------------------------------------------------------------
  Exception / Interrupt Handler
 *----------------------------------------------------------------------------*/
/* Exceptions */
WEAK(NMI_Handler)
void NMI_Handler            (void){}

WEAK(HardFault_Handler)
void HardFault_Handler      (void){while(1);} 

WEAK(MemManage_Handler)
void MemManage_Handler      (void){while(1);} 

WEAK(BusFault_Handler)
void BusFault_Handler       (void){while(1);} 

WEAK(UsageFault_Handler)
void UsageFault_Handler     (void){while(1);}

WEAK(SVC_Handler)
void SVC_Handler            (void){} 

WEAK(DebugMon_Handler)
void DebugMon_Handler       (void){} 

WEAK(PendSV_Handler)
void PendSV_Handler         (void){} 

WEAK(SysTick_Handler)
void SysTick_Handler        (void){} 

WEAK(WWDGT_IRQHandler)
void WWDGT_IRQHandler       (void){} 

WEAK(LVD_IRQHandler)
void LVD_IRQHandler         (void){} 

WEAK(RTC_IRQHandler)
void RTC_IRQHandler         (void){} 

WEAK(FMC_IRQHandler)
void FMC_IRQHandler         (void){} 

WEAK(RCU_CTC_IRQHandler)
void RCU_CTC_IRQHandler     (void){} 

WEAK(EXTI0_1_IRQHandler)
void EXTI0_1_IRQHandler     (void){} 

WEAK(EXTI2_3_IRQHandler)
void EXTI2_3_IRQHandler     (void){} 

WEAK(EXTI4_15_IRQHandler)
void EXTI4_15_IRQHandler    (void){}

WEAK(TSI_IRQHandler)
void TSI_IRQHandler         (void){}

WEAK(DMA_Channel0_IRQHandler)
void DMA_Channel0_IRQHandler            (void){}

WEAK(DMA_Channel1_2_IRQHandler)
void DMA_Channel1_2_IRQHandler          (void){}

WEAK(DMA_Channel3_4_IRQHandler)
void DMA_Channel3_4_IRQHandler          (void){}

WEAK(ADC_CMP_IRQHandler)
void ADC_CMP_IRQHandler                 (void){}

WEAK(TIMER0_BRK_UP_TRG_COM_IRQHandler)
void TIMER0_BRK_UP_TRG_COM_IRQHandler   (void){}

WEAK(TIMER0_Channel_IRQHandler)
void TIMER0_Channel_IRQHandler          (void){}

WEAK(TIMER1_IRQHandler)
void TIMER1_IRQHandler      (void){}

WEAK(TIMER2_IRQHandler)
void TIMER2_IRQHandler      (void){}

WEAK(TIMER5_DAC_IRQHandler)
void TIMER5_DAC_IRQHandler  (void){}

WEAK(TIMER13_IRQHandler)
void TIMER13_IRQHandler     (void){}

WEAK(TIMER14_IRQHandler)
void TIMER14_IRQHandler     (void){}

WEAK(TIMER15_IRQHandler)
void TIMER15_IRQHandler     (void){}

WEAK(TIMER16_IRQHandler)
void TIMER16_IRQHandler     (void){}

WEAK(I2C0_EV_IRQHandler)
void I2C0_EV_IRQHandler     (void){}

WEAK(I2C1_EV_IRQHandler)
void I2C1_EV_IRQHandler     (void){}

WEAK(SPI0_IRQHandler)
void SPI0_IRQHandler        (void){}

WEAK(SPI1_IRQHandler)
void SPI1_IRQHandler        (void){}

WEAK(USART0_IRQHandler)
void USART0_IRQHandler      (void){}

WEAK(USART1_IRQHandler)
void USART1_IRQHandler      (void){}

WEAK(CEC_IRQHandler)
void CEC_IRQHandler         (void){}

WEAK(I2C0_ER_IRQHandler)
void I2C0_ER_IRQHandler     (void){}

WEAK(I2C1_ER_IRQHandler)
void I2C1_ER_IRQHandler     (void){}

WEAK(USBFS_WKUP_IRQHandler)
void USBFS_WKUP_IRQHandler  (void){}

WEAK(DMA_Channel5_6_IRQHandler)
void DMA_Channel5_6_IRQHandler          (void){}

WEAK(USBFS_IRQHandler)
void USBFS_IRQHandler       (void){}

WEAK(SWI0_IRQHandler) 
void SWI0_IRQHandler        (void){}

WEAK(SWI1_IRQHandler) 
void SWI1_IRQHandler        (void){}

WEAK(SWI2_IRQHandler) 
void SWI2_IRQHandler        (void){}

WEAK(SWI3_IRQHandler) 
void SWI3_IRQHandler        (void){}

WEAK(SWI4_IRQHandler) 
void SWI4_IRQHandler        (void){}

WEAK(SWI5_IRQHandler) 
void SWI5_IRQHandler        (void){}

WEAK(SWI6_IRQHandler) 
void SWI6_IRQHandler        (void){}

WEAK(SWI7_IRQHandler) 
void SWI7_IRQHandler        (void){}

WEAK(SWI8_IRQHandler) 
void SWI8_IRQHandler        (void){}


/*----------------------------------------------------------------------------
  Exception / Interrupt Vector table
 *----------------------------------------------------------------------------*/

#if defined ( __GNUC__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

ROOT const pFunc __VECTOR_TABLE[240] __VECTOR_TABLE_ATTRIBUTE = {
    (pFunc)(&__INITIAL_SP),                   /*     Initial Stack Pointer */
    Reset_Handler,                            /*     Reset Handler */
    NMI_Handler,                              /* -14 NMI Handler */
    HardFault_Handler,                        /* -13 Hard Fault Handler */
    MemManage_Handler,                        /* -12 MPU Fault Handler */
    BusFault_Handler,                         /* -11 Bus Fault Handler */
    UsageFault_Handler,                       /* -10 Usage Fault Handler */
    0,                                        /*     Reserved */
    0,                                        /*     Reserved */
    0,                                        /*     Reserved */
    0,                                        /*     Reserved */
    SVC_Handler,                              /*  -5 SVCall Handler */
    DebugMon_Handler,                         /*  -4 Debug Monitor Handler */
    0,                                        /*     Reserved */
    PendSV_Handler,                           /*  -2 PendSV Handler */
    SysTick_Handler,                          /*  -1 SysTick Handler */

    /* Interrupts */
    WWDGT_IRQHandler,                         /* Vector Number 16,Window watchdog timer */
    LVD_IRQHandler,                           /* Vector Number 17,LVD through EXTI Line detect */
    RTC_IRQHandler,                           /* Vector Number 18,RTC through EXTI Line */
    FMC_IRQHandler,                           /* Vector Number 19,FMC */
    RCU_CTC_IRQHandler,                       /* Vector Number 20,RCU and CTC */
    EXTI0_1_IRQHandler,                       /* Vector Number 21,EXTI Line 0 and EXTI Line 1 */
    EXTI2_3_IRQHandler,                       /* Vector Number 22,EXTI Line 2 and EXTI Line 3 */
    EXTI4_15_IRQHandler,                      /* Vector Number 23,EXTI Line 4 to EXTI Line 15 */
    TSI_IRQHandler,                           /* Vector Number 24,TSI */
    DMA_Channel0_IRQHandler,                  /* Vector Number 25,DMA Channel 0 */
    DMA_Channel1_2_IRQHandler,                /* Vector Number 26,DMA Channel 1 and DMA Channel 2 */
    DMA_Channel3_4_IRQHandler,                /* Vector Number 27,DMA Channel 3 and DMA Channel 4 */
    ADC_CMP_IRQHandler,                       /* Vector Number 28,ADC and Comparator 1-2  */
    TIMER0_BRK_UP_TRG_COM_IRQHandler,         /* Vector Number 29,TIMER0 Break, Update, Trigger and Commutation */
    TIMER0_Channel_IRQHandler,                /* Vector Number 30,TIMER0 Channel Capture Compare */
    TIMER1_IRQHandler,                        /* Vector Number 31,TIMER1 */
    TIMER2_IRQHandler,                        /* Vector Number 32,TIMER2 */
    TIMER5_DAC_IRQHandler,                    /* Vector Number 33,TIMER5 and DAC */
    0,                                        /* Reserved */
    TIMER13_IRQHandler,                       /* Vector Number 35,TIMER13 */
    TIMER14_IRQHandler,                       /* Vector Number 36,TIMER14 */
    TIMER15_IRQHandler,                       /* Vector Number 37,TIMER15 */
    TIMER16_IRQHandler,                       /* Vector Number 38,TIMER16 */
    I2C0_EV_IRQHandler,                       /* Vector Number 39,I2C0 Event  */
    I2C1_EV_IRQHandler,                       /* Vector Number 40,I2C1 Event */
    SPI0_IRQHandler,                          /* Vector Number 41,SPI0 */
    SPI1_IRQHandler,                          /* Vector Number 42,SPI1 */
    USART0_IRQHandler,                        /* Vector Number 43,USART0 */
    USART1_IRQHandler,                        /* Vector Number 44,USART1 */
    0,                                        /* Reserved */
    CEC_IRQHandler,                           /* Vector Number 46,CEC */
    0,                                        /* Reserved         */
    I2C0_ER_IRQHandler,                       /* Vector Number 48,I2C0 Error */
    0,                                        /* Reserved     */
    I2C1_ER_IRQHandler,                       /* Vector Number 50,I2C1 Error */
    0,                                        /* Reserved */
    0,                                        /* Reserved */
    0,                                        /* Reserved */
    0,                                        /* Reserved */
    0,                                        /* Reserved  */
    0,                                        /* Reserved  */
    0,                                        /* Reserved  */
    USBFS_WKUP_IRQHandler,                    /* Vector Number 58,USBFS Wakeup */
    0,                                        /* Reserved */
    0,                                        /* Reserved */
    0,                                        /* Reserved */
    0,                                        /* Reserved */
    0,                                        /* Reserved */
    DMA_Channel5_6_IRQHandler,                /* Vector Number 64,DMA Channel5 and Channel6  */
    0,                                        /* Reserved */
    0,                                        /* Reserved */
    0,                                        /* Reserved */
    0,                                        /* Reserved */
    0,                                        /* Reserved */
    SWI0_IRQHandler,                          /*  70 SWI7 */
    SWI1_IRQHandler,                          /*  71 SWI7 */
    SWI2_IRQHandler,                          /*  72 SWI7 */
    SWI3_IRQHandler,                          /*  73 SWI7 */
    SWI4_IRQHandler,                          /*  74 SWI7 */
    SWI5_IRQHandler,                          /*  75 SWI7 */
    SWI6_IRQHandler,                          /*  76 SWI8 */
    SWI7_IRQHandler,                          /*  77 SWI8 */
    SWI8_IRQHandler,                          /*  78 SWI8 */
    0,                                        /* Reserved */
    0,                                        /* Reserved */
    0,                                        /* Reserved */
    0,                                        /* Reserved */
    USBFS_IRQHandler,                         /* Vector Number 83,USBFS */
};

#if defined ( __GNUC__ )
#pragma GCC diagnostic pop
#endif

WEAK(vsf_hal_pre_startup_init)
void vsf_hal_pre_startup_init(void)
{}

/*----------------------------------------------------------------------------
  Reset Handler called on controller reset
 *----------------------------------------------------------------------------*/
void Reset_Handler(void)
{
    vsf_hal_pre_startup_init();

    //! enable FPU
    SCB->CPACR |= ((3UL << 10*2) | (3UL << 11*2));  /* set CP10 and CP11 Full Access */

    __PROGRAM_START();                        /* Enter PreMain (C library entry point) */
}
