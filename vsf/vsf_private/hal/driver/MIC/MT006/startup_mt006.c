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

WEAK(SVC_Handler)
void SVC_Handler            (void){} 

WEAK(PendSV_Handler)
void PendSV_Handler         (void){} 

WEAK(SysTick_Handler)
void SysTick_Handler        (void){} 

WEAK(FLASH_IRQHandler)
void FLASH_IRQHandler        (void){} 

WEAK(UART0_IRQHandler)
void UART0_IRQHandler        (void){} 

WEAK(I2C0_IRQHandler)
void I2C0_IRQHandler        (void){} 

WEAK(SPI0_IRQHandler)
void SPI0_IRQHandler        (void){} 

WEAK(TIMER0_IRQHandler)
void TIMER0_IRQHandler        (void){} 

WEAK(MCPWM_IRQHandler)
void MCPWM_IRQHandler        (void){} 

WEAK(GPIO_IRQHandler)
void GPIO_IRQHandler        (void){} 

WEAK(ADC_IRQHandler)
void ADC_IRQHandler        (void){} 

WEAK(BOD_IRQHandler)
void BOD_IRQHandler        (void){} 

WEAK(BOR_IRQHandler)
void BOR_IRQHandler        (void){} 

WEAK(TSC_IRQHandler)
void TSC_IRQHandler        (void){} 

WEAK(CRS_IRQHandler)
void CRS_IRQHandler        (void){} 

WEAK(ACMP_IRQHandler)
void ACMP_IRQHandler        (void){} 

WEAK(USB_IRQHandler)
void USB_IRQHandler        (void){} 

WEAK(STIMER_IRQHandler)
void STIMER_IRQHandler        (void){} 

WEAK(CAN_IRQHandler)
void CAN_IRQHandler        (void){} 

/*----------------------------------------------------------------------------
  Exception / Interrupt Vector table
 *----------------------------------------------------------------------------*/

#if defined ( __GNUC__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

ROOT const pFunc __VECTOR_TABLE[] __VECTOR_TABLE_ATTRIBUTE = {
    (pFunc)(&__INITIAL_SP),                   /*     Initial Stack Pointer */
    Reset_Handler,                            /*     Reset Handler */
    NMI_Handler,                              /* -14 NMI Handler */
    HardFault_Handler,                        /* -13 Hard Fault Handler */
    0,                                        /*     Reserved */
    0,                                        /*     Reserved */
    0,                                        /*     Reserved */
    0,                                        /*     Reserved */
    0,                                        /*     Reserved */
    0,                                        /*     Reserved */
    0,                                        /*     Reserved */
    SVC_Handler,                              /*  -5 SVCall Handler */
    0,                                        /*     Reserved */
    0,                                        /*     Reserved */
    PendSV_Handler,                           /*  -2 PendSV Handler */
    SysTick_Handler,                          /*  -1 SysTick Handler */

    /* Interrupts */
    FLASH_IRQHandler,               /*  0  -- FLASH Handler */
    UART0_IRQHandler,               /*  1  -- UART0 Handler */
    I2C0_IRQHandler,                /*  2  -- I2C0 Handler */
    SPI0_IRQHandler,                /*  3  -- SPI0 Handler */
    TIMER0_IRQHandler,              /*  4  -- TIMER0 Handler */
    MCPWM_IRQHandler,               /*  5  -- MCPWM Handler */
    GPIO_IRQHandler,                /*  6  -- GPIO Handler */
    ADC_IRQHandler,                 /*  7  -- ADC Handler */
    BOD_IRQHandler,                 /*  8  -- BOD Handler */
    BOR_IRQHandler,                 /*  9  -- BOR Handler */
    TSC_IRQHandler,                 /*  10 -- TSC Handler */ 
    CRS_IRQHandler,                 /*  11 -- CRS Handler */
    ACMP_IRQHandler,                /*  12 -- CMP Handler */
    USB_IRQHandler,                 /*  13 -- USB Handler */
    STIMER_IRQHandler,              /*  14 -- Simple TIMER Handler */
    CAN_IRQHandler,                 /*  15 -- CAN Handler */
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

    __PROGRAM_START();                        /* Enter PreMain (C library entry point) */
}
