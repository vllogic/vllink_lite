/*!
    \file  gd32f3x0_usart.h
    \brief definitions for the USART
    
    \version 2017-06-06, V1.0.0, firmware for GD32F3x0
    \version 2019-06-01, V2.0.0, firmware for GD32F3x0
*/

/*
    Copyright (c) 2019, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this 
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice, 
       this list of conditions and the following disclaimer in the documentation 
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors 
       may be used to endorse or promote products derived from this software without 
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
OF SUCH DAMAGE.
*/

#ifndef GD32F3X0_USART_H
#define GD32F3X0_USART_H

#include "gd32f3x0.h"

/* USARTx(x=0,1) definitions */
#define USART0_BASE                        (USART_BASE + 0x0000F400U)
#define USART1_BASE                        USART_BASE

/* registers definitions */
#define USART_CTL0(usartx)            REG32((usartx) + 0x00000000U)  /*!< USART control register 0 */
#define USART_CTL1(usartx)            REG32((usartx) + 0x00000004U)  /*!< USART control register 1 */
#define USART_CTL2(usartx)            REG32((usartx) + 0x00000008U)  /*!< USART control register 2 */
#define USART_BAUD(usartx)            REG32((usartx) + 0x0000000CU)  /*!< USART baud rate register */
#define USART_GP(usartx)              REG32((usartx) + 0x00000010U)  /*!< USART guard time and prescaler register */
#define USART_RT(usartx)              REG32((usartx) + 0x00000014U)  /*!< USART receiver timeout register */
#define USART_CMD(usartx)             REG32((usartx) + 0x00000018U)  /*!< USART command register */
#define USART_STAT(usartx)            REG32((usartx) + 0x0000001CU)  /*!< USART status register */
#define USART_INTC(usartx)            REG32((usartx) + 0x00000020U)  /*!< USART status clear register */
#define USART_RDATA(usartx)           REG32((usartx) + 0x00000024U)  /*!< USART receive data register */
#define USART_TDATA(usartx)           REG32((usartx) + 0x00000028U)  /*!< USART transmit data register */
#define USART_RFCS(usartx)            REG32((usartx) + 0x000000D0U)  /*!< USART receive FIFO control and status register */

/* bits definitions */
/* USARTx_CTL0 */
#define USART_CTL0_UEN                BIT(0)                         /*!< USART enable */
#define USART_CTL0_UESM               BIT(1)                         /*!< USART enable in deep-sleep mode */
#define USART_CTL0_REN                BIT(2)                         /*!< receiver enable */
#define USART_CTL0_TEN                BIT(3)                         /*!< transmitter enable */
#define USART_CTL0_IDLEIE             BIT(4)                         /*!< idle line detected interrupt enable */
#define USART_CTL0_RBNEIE             BIT(5)                         /*!< read data buffer not empty interrupt and overrun error interrupt enable */
#define USART_CTL0_TCIE               BIT(6)                         /*!< transmission complete interrupt enable */
#define USART_CTL0_TBEIE              BIT(7)                         /*!< transmitter register empty interrupt enable */
#define USART_CTL0_PERRIE             BIT(8)                         /*!< parity error interrupt enable */
#define USART_CTL0_PM                 BIT(9)                         /*!< parity mode */
#define USART_CTL0_PCEN               BIT(10)                        /*!< parity control enable */
#define USART_CTL0_WM                 BIT(11)                        /*!< wakeup method in mute mode */
#define USART_CTL0_WL                 BIT(12)                        /*!< word length */
#define USART_CTL0_MEN                BIT(13)                        /*!< mute mode enable */
#define USART_CTL0_AMIE               BIT(14)                        /*!< address match interrupt enable */
#define USART_CTL0_OVSMOD             BIT(15)                        /*!< oversample mode */
#define USART_CTL0_DED                BITS(16,20)                    /*!< driver enable deassertion time */
#define USART_CTL0_DEA                BITS(21,25)                    /*!< driver enable assertion time */
#define USART_CTL0_RTIE               BIT(26)                        /*!< receiver timeout interrupt enable */
#define USART_CTL0_EBIE               BIT(27)                        /*!< end of block interrupt enable */

/* USARTx_CTL1 */
#define USART_CTL1_ADDM               BIT(4)                         /*!< address detection mode */
#define USART_CTL1_LBLEN              BIT(5)                         /*!< LIN break frame length */
#define USART_CTL1_LBDIE              BIT(6)                         /*!< LIN break detection interrupt enable */
#define USART_CTL1_CLEN               BIT(8)                         /*!< last bit clock pulse */
#define USART_CTL1_CPH                BIT(9)                         /*!< clock phase */
#define USART_CTL1_CPL                BIT(10)                        /*!< clock polarity */
#define USART_CTL1_CKEN               BIT(11)                        /*!< ck pin enable */
#define USART_CTL1_STB                BITS(12,13)                    /*!< stop bits length */
#define USART_CTL1_LMEN               BIT(14)                        /*!< LIN mode enable */
#define USART_CTL1_STRP               BIT(15)                        /*!< swap TX/RX pins */
#define USART_CTL1_RINV               BIT(16)                        /*!< RX pin level inversion */
#define USART_CTL1_TINV               BIT(17)                        /*!< TX pin level inversion */
#define USART_CTL1_DINV               BIT(18)                        /*!< data bit level inversion */
#define USART_CTL1_MSBF               BIT(19)                        /*!< most significant bit first */
#define USART_CTL1_ABDEN              BIT(20)                        /*!< auto baud rate enable */
#define USART_CTL1_ABDM               BITS(21,22)                    /*!< auto baud rate mode */
#define USART_CTL1_RTEN               BIT(23)                        /*!< receiver timeout enable */
#define USART_CTL1_ADDR               BITS(24,31)                    /*!< address of the USART terminal */

/* USARTx_CTL2 */
#define USART_CTL2_ERRIE              BIT(0)                         /*!< error interrupt enable in multibuffer communication */
#define USART_CTL2_IREN               BIT(1)                         /*!< IrDA mode enable */
#define USART_CTL2_IRLP               BIT(2)                         /*!< IrDA low-power */
#define USART_CTL2_HDEN               BIT(3)                         /*!< half-duplex enable */
#define USART_CTL2_NKEN               BIT(4)                         /*!< NACK enable in smartcard mode */
#define USART_CTL2_SCEN               BIT(5)                         /*!< smartcard mode enable */
#define USART_CTL2_DENR               BIT(6)                         /*!< DMA enable for reception */
#define USART_CTL2_DENT               BIT(7)                         /*!< DMA enable for transmission */
#define USART_CTL2_RTSEN              BIT(8)                         /*!< RTS enable */
#define USART_CTL2_CTSEN              BIT(9)                         /*!< CTS enable */
#define USART_CTL2_CTSIE              BIT(10)                        /*!< CTS interrupt enable */
#define USART_CTL2_OSB                BIT(11)                        /*!< one sample bit mode */
#define USART_CTL2_OVRD               BIT(12)                        /*!< overrun disable */
#define USART_CTL2_DDRE               BIT(13)                        /*!< disable DMA on reception error */
#define USART_CTL2_DEM                BIT(14)                        /*!< driver enable mode */
#define USART_CTL2_DEP                BIT(15)                        /*!< driver enable polarity mode */
#define USART_CTL2_SCRTNUM            BITS(17,19)                    /*!< smartcard auto-retry number */
#define USART_CTL2_WUM                BITS(20,21)                    /*!< wakeup mode from deep-sleep mode */
#define USART_CTL2_WUIE               BIT(22)                        /*!< wakeup from deep-sleep mode interrupt enable */

/* USARTx_BAUD */
#define USART_BAUD_FRADIV             BITS(0,3)                      /*!< fraction of baud-rate divider */
#define USART_BAUD_INTDIV             BITS(4,15)                     /*!< integer of baud-rate divider */

/* USARTx_GP */
#define USART_GP_PSC                  BITS(0,7)                      /*!< prescaler value for dividing the system clock */
#define USART_GP_GUAT                 BITS(8,15)                     /*!< guard time value in smartcard mode */

/* USARTx_RT */
#define USART_RT_RT                   BITS(0,23)                     /*!< receiver timeout threshold */
#define USART_RT_BL                   BITS(24,31)                    /*!< block length */

/* USARTx_CMD */
#define USART_CMD_ABDCMD              BIT(0)                         /*!< auto baudrate detection command */
#define USART_CMD_SBKCMD              BIT(1)                         /*!< send break command */
#define USART_CMD_MMCMD               BIT(2)                         /*!< mute mode command */
#define USART_CMD_RXFCMD              BIT(3)                         /*!< receive data flush command */
#define USART_CMD_TXFCMD              BIT(4)                         /*!< transmit data flush request */

/* USARTx_STAT */
#define USART_STAT_PERR               BIT(0)                         /*!< parity error flag */
#define USART_STAT_FERR               BIT(1)                         /*!< frame error flag */
#define USART_STAT_NERR               BIT(2)                         /*!< noise error flag */
#define USART_STAT_ORERR              BIT(3)                         /*!< overrun error */
#define USART_STAT_IDLEF              BIT(4)                         /*!< idle line detected flag */
#define USART_STAT_RBNE               BIT(5)                         /*!< read data buffer not empty */
#define USART_STAT_TC                 BIT(6)                         /*!< transmission completed */
#define USART_STAT_TBE                BIT(7)                         /*!< transmit data register empty */
#define USART_STAT_LBDF               BIT(8)                         /*!< LIN break detected flag */
#define USART_STAT_CTSF               BIT(9)                         /*!< CTS change flag */
#define USART_STAT_CTS                BIT(10)                        /*!< CTS level */
#define USART_STAT_RTF                BIT(11)                        /*!< receiver timeout flag */
#define USART_STAT_EBF                BIT(12)                        /*!< end of block flag */
#define USART_STAT_ABDE               BIT(14)                        /*!< auto baudrate detection error */
#define USART_STAT_ABDF               BIT(15)                        /*!< auto baudrate detection flag */
#define USART_STAT_BSY                BIT(16)                        /*!< busy flag */
#define USART_STAT_AMF                BIT(17)                        /*!< address match flag */
#define USART_STAT_SBF                BIT(18)                        /*!< send break flag */
#define USART_STAT_RWU                BIT(19)                        /*!< receiver wakeup from mute mode */
#define USART_STAT_WUF                BIT(20)                        /*!< wakeup from deep-sleep mode flag */
#define USART_STAT_TEA                BIT(21)                        /*!< transmit enable acknowledge flag */
#define USART_STAT_REA                BIT(22)                        /*!< receive enable acknowledge flag */

/* USARTx_INTC */
#define USART_INTC_PEC                BIT(0)                         /*!< parity error clear */
#define USART_INTC_FEC                BIT(1)                         /*!< frame error flag clear */
#define USART_INTC_NEC                BIT(2)                         /*!< noise detected clear */
#define USART_INTC_OREC               BIT(3)                         /*!< overrun error clear */
#define USART_INTC_IDLEC              BIT(4)                         /*!< idle line detected clear */
#define USART_INTC_TCC                BIT(6)                         /*!< transmission complete clear */
#define USART_INTC_LBDC               BIT(8)                         /*!< LIN break detected clear */
#define USART_INTC_CTSC               BIT(9)                         /*!< CTS change clear */
#define USART_INTC_RTC                BIT(11)                        /*!< receiver timeout clear */
#define USART_INTC_EBC                BIT(12)                        /*!< end of timeout clear */
#define USART_INTC_AMC                BIT(17)                        /*!< address match clear */
#define USART_INTC_WUC                BIT(20)                        /*!< wakeup from deep-sleep mode clear */

/* USARTx_RDATA */
#define USART_RDATA_RDATA             BITS(0,8)                      /*!< receive data value */

/* USARTx_TDATA */
#define USART_TDATA_TDATA             BITS(0,8)                      /*!< transmit data value */

/* USARTx_RFCS */
#define USART_RFCS_ELNACK             BIT(0)                         /*!< early NACK */
#define USART_RFCS_RFEN               BIT(8)                         /*!< receive FIFO enable */
#define USART_RFCS_RFFIE              BIT(9)                         /*!< receive FIFO full interrupt enable */
#define USART_RFCS_RFE                BIT(10)                        /*!< receive FIFO empty flag */
#define USART_RFCS_RFF                BIT(11)                        /*!< receive FIFO full flag */
#define USART_RFCS_RFCNT              BITS(12,14)                    /*!< receive FIFO counter number */
#define USART_RFCS_RFFINT             BIT(15)                        /*!< receive FIFO full interrupt flag */

/* constants definitions */

/* define the USART bit position and its register index offset */
#define USART_REGIDX_BIT(regidx, bitpos)    (((uint32_t)(regidx) << 6) | (uint32_t)(bitpos))
#define USART_REG_VAL(usartx, offset)       (REG32((usartx) + (((uint32_t)(offset) & 0x0000FFFFU) >> 6)))
#define USART_BIT_POS(val)                  ((uint32_t)(val) & 0x0000001FU)
#define USART_REGIDX_BIT2(regidx, bitpos, regidx2, bitpos2)   (((uint32_t)(regidx2) << 22) | (uint32_t)((bitpos2) << 16)\
                                                              | (((uint32_t)(regidx) << 6) | (uint32_t)(bitpos)))
#define USART_REG_VAL2(usartx, offset)       (REG32((usartx) + ((uint32_t)(offset) >> 22)))
#define USART_BIT_POS2(val)                  (((uint32_t)(val) & 0x001F0000U) >> 16)

/* register offset */
#define USART_CTL0_REG_OFFSET              (0x00000000U)                   /*!< CTL0 register offset */
#define USART_CTL1_REG_OFFSET              (0x00000004U)                   /*!< CTL1 register offset */
#define USART_CTL2_REG_OFFSET              (0x00000008U)                   /*!< CTL2 register offset */
#define USART_STAT_REG_OFFSET              (0x0000001CU)                   /*!< STAT register offset */
#define USART_RFCS_REG_OFFSET              (0x000000D0U)                   /*!< RFCS register offset */

/* USART flags */
typedef enum{
    /* flags in STAT register */
    USART_FLAG_REA = USART_REGIDX_BIT(USART_STAT_REG_OFFSET, 22U),         /*!< receive enable acknowledge flag */
    USART_FLAG_TEA = USART_REGIDX_BIT(USART_STAT_REG_OFFSET, 21U),         /*!< transmit enable acknowledge flag */
    USART_FLAG_WU = USART_REGIDX_BIT(USART_STAT_REG_OFFSET, 20U),          /*!< wakeup from deep-sleep mode flag */
    USART_FLAG_RWU = USART_REGIDX_BIT(USART_STAT_REG_OFFSET, 19U),         /*!< receiver wakeup from mute mode */
    USART_FLAG_SB = USART_REGIDX_BIT(USART_STAT_REG_OFFSET, 18U),          /*!< send break flag */
    USART_FLAG_AM = USART_REGIDX_BIT(USART_STAT_REG_OFFSET, 17U),          /*!< ADDR match flag */
    USART_FLAG_BSY = USART_REGIDX_BIT(USART_STAT_REG_OFFSET, 16U),         /*!< busy flag */
    USART_FLAG_ABD = USART_REGIDX_BIT(USART_STAT_REG_OFFSET, 15U),         /*!< auto baudrate detection flag */
    USART_FLAG_ABDE = USART_REGIDX_BIT(USART_STAT_REG_OFFSET, 14U),        /*!< auto baudrate detection error */
    USART_FLAG_EB = USART_REGIDX_BIT(USART_STAT_REG_OFFSET, 12U),          /*!< end of block flag */
    USART_FLAG_RT = USART_REGIDX_BIT(USART_STAT_REG_OFFSET, 11U),          /*!< receiver timeout flag */
    USART_FLAG_CTS = USART_REGIDX_BIT(USART_STAT_REG_OFFSET, 10U),         /*!< CTS level */
    USART_FLAG_CTSF = USART_REGIDX_BIT(USART_STAT_REG_OFFSET, 9U),         /*!< CTS change flag */
    USART_FLAG_LBD = USART_REGIDX_BIT(USART_STAT_REG_OFFSET, 8U),          /*!< LIN break detected flag */
    USART_FLAG_TBE = USART_REGIDX_BIT(USART_STAT_REG_OFFSET, 7U),          /*!< transmit data buffer empty */
    USART_FLAG_TC = USART_REGIDX_BIT(USART_STAT_REG_OFFSET, 6U),           /*!< transmission complete */
    USART_FLAG_RBNE = USART_REGIDX_BIT(USART_STAT_REG_OFFSET, 5U),         /*!< read data buffer not empty */
    USART_FLAG_IDLE = USART_REGIDX_BIT(USART_STAT_REG_OFFSET, 4U),         /*!< IDLE line detected flag */
    USART_FLAG_ORERR = USART_REGIDX_BIT(USART_STAT_REG_OFFSET, 3U),        /*!< overrun error */
    USART_FLAG_NERR = USART_REGIDX_BIT(USART_STAT_REG_OFFSET, 2U),         /*!< noise error flag */
    USART_FLAG_FERR = USART_REGIDX_BIT(USART_STAT_REG_OFFSET, 1U),         /*!< frame error flag */
    USART_FLAG_PERR = USART_REGIDX_BIT(USART_STAT_REG_OFFSET, 0U),         /*!< parity error flag */
    /* flags in RFCS register */
    USART_FLAG_RFF = USART_REGIDX_BIT(USART_RFCS_REG_OFFSET, 11U),         /*!< receive FIFO full flag */
    USART_FLAG_RFE = USART_REGIDX_BIT(USART_RFCS_REG_OFFSET, 10U),         /*!< receive FIFO empty flag */
}usart_flag_enum;

/* USART interrupt flags */
typedef enum
{
    /* interrupt flags in CTL0 register */
    USART_INT_FLAG_EB = USART_REGIDX_BIT2(USART_CTL0_REG_OFFSET, 27U, USART_STAT_REG_OFFSET, 12U),       /*!< end of block interrupt flag */
    USART_INT_FLAG_RT = USART_REGIDX_BIT2(USART_CTL0_REG_OFFSET, 26U, USART_STAT_REG_OFFSET, 11U),       /*!< receiver timeout interrupt flag */
    USART_INT_FLAG_AM = USART_REGIDX_BIT2(USART_CTL0_REG_OFFSET, 14U, USART_STAT_REG_OFFSET, 17U),       /*!< address match interrupt flag */
    USART_INT_FLAG_PERR = USART_REGIDX_BIT2(USART_CTL0_REG_OFFSET, 8U, USART_STAT_REG_OFFSET, 0U),       /*!< parity error interrupt flag */
    USART_INT_FLAG_TBE = USART_REGIDX_BIT2(USART_CTL0_REG_OFFSET, 7U, USART_STAT_REG_OFFSET, 7U),        /*!< transmitter buffer empty interrupt flag */
    USART_INT_FLAG_TC = USART_REGIDX_BIT2(USART_CTL0_REG_OFFSET, 6U, USART_STAT_REG_OFFSET, 6U),         /*!< transmission complete interrupt flag */
    USART_INT_FLAG_RBNE = USART_REGIDX_BIT2(USART_CTL0_REG_OFFSET, 5U, USART_STAT_REG_OFFSET, 5U),       /*!< read data buffer not empty interrupt flag */
    USART_INT_FLAG_RBNE_ORERR = USART_REGIDX_BIT2(USART_CTL0_REG_OFFSET, 5U, USART_STAT_REG_OFFSET, 3U), /*!< overrun error interrupt flag */
    USART_INT_FLAG_IDLE = USART_REGIDX_BIT2(USART_CTL0_REG_OFFSET, 4U, USART_STAT_REG_OFFSET, 4U),       /*!< IDLE line detected interrupt flag */
    /* interrupt flags in CTL1 register */
    USART_INT_FLAG_LBD = USART_REGIDX_BIT2(USART_CTL1_REG_OFFSET, 6U, USART_STAT_REG_OFFSET, 8U),        /*!< LIN break detected interrupt flag */
    /* interrupt flags in CTL2 register */
    USART_INT_FLAG_WU = USART_REGIDX_BIT2(USART_CTL2_REG_OFFSET, 22U, USART_STAT_REG_OFFSET, 20U),       /*!< wakeup from deep-sleep mode interrupt flag */
    USART_INT_FLAG_CTS = USART_REGIDX_BIT2(USART_CTL2_REG_OFFSET, 10U, USART_STAT_REG_OFFSET, 9U),       /*!< CTS interrupt flag */
    USART_INT_FLAG_ERR_NERR = USART_REGIDX_BIT2(USART_CTL2_REG_OFFSET, 0U, USART_STAT_REG_OFFSET, 2U),   /*!< noise error interrupt flag */
    USART_INT_FLAG_ERR_ORERR = USART_REGIDX_BIT2(USART_CTL2_REG_OFFSET, 0U, USART_STAT_REG_OFFSET, 3U),  /*!< overrun error interrupt flag */
    USART_INT_FLAG_ERR_FERR = USART_REGIDX_BIT2(USART_CTL2_REG_OFFSET, 0U, USART_STAT_REG_OFFSET, 1U),   /*!< frame error interrupt flag */
    /* interrupt flags in RFCS register */
    USART_INT_FLAG_RFFINT = USART_REGIDX_BIT2(USART_RFCS_REG_OFFSET, 9U, USART_RFCS_REG_OFFSET, 15U),    /*!< receive FIFO full interrupt flag */
}usart_interrupt_flag_enum;

/* USART interrupt enable or disable */
typedef enum
{
    /* interrupt in CTL0 register */
    USART_INT_EB = USART_REGIDX_BIT(USART_CTL0_REG_OFFSET, 27U),     /*!< end of block interrupt */
    USART_INT_RT = USART_REGIDX_BIT(USART_CTL0_REG_OFFSET, 26U),     /*!< receiver timeout interrupt */
    USART_INT_AM = USART_REGIDX_BIT(USART_CTL0_REG_OFFSET, 14U),     /*!< address match interrupt */
    USART_INT_PERR = USART_REGIDX_BIT(USART_CTL0_REG_OFFSET, 8U),    /*!< parity error interrupt */
    USART_INT_TBE = USART_REGIDX_BIT(USART_CTL0_REG_OFFSET, 7U),     /*!< transmitter buffer empty interrupt */
    USART_INT_TC = USART_REGIDX_BIT(USART_CTL0_REG_OFFSET, 6U),      /*!< transmission complete interrupt */
    USART_INT_RBNE = USART_REGIDX_BIT(USART_CTL0_REG_OFFSET, 5U),    /*!< read data buffer not empty interrupt and overrun error interrupt */
    USART_INT_IDLE = USART_REGIDX_BIT(USART_CTL0_REG_OFFSET, 4U),    /*!< IDLE line detected interrupt */
    /* interrupt in CTL1 register */
    USART_INT_LBD = USART_REGIDX_BIT(USART_CTL1_REG_OFFSET, 6U),     /*!< LIN break detected interrupt */
    /* interrupt in CTL2 register */
    USART_INT_WU = USART_REGIDX_BIT(USART_CTL2_REG_OFFSET, 22U),     /*!< wakeup from deep-sleep mode interrupt */
    USART_INT_CTS = USART_REGIDX_BIT(USART_CTL2_REG_OFFSET, 10U),    /*!< CTS interrupt */
    USART_INT_ERR = USART_REGIDX_BIT(USART_CTL2_REG_OFFSET, 0U),     /*!< error interrupt */
    /* interrupt in RFCS register */
    USART_INT_RFF = USART_REGIDX_BIT(USART_RFCS_REG_OFFSET, 9U),     /*!< receive FIFO full interrupt */
}usart_interrupt_enum;

/* USART invert configure */
typedef enum {
    /* data bit level inversion */
    USART_DINV_ENABLE,                                               /*!< data bit level inversion */
    USART_DINV_DISABLE,                                              /*!< data bit level not inversion */
    /* TX pin level inversion */
    USART_TXPIN_ENABLE,                                              /*!< TX pin level inversion */               
    USART_TXPIN_DISABLE,                                             /*!< TX pin level not inversion */
    /* RX pin level inversion */
    USART_RXPIN_ENABLE,                                              /*!< RX pin level inversion */
    USART_RXPIN_DISABLE,                                             /*!< RX pin level not inversion */
    /* swap TX/RX pins */
    USART_SWAP_ENABLE,                                               /*!< swap TX/RX pins */                
    USART_SWAP_DISABLE,                                              /*!< not swap TX/RX pins */
}usart_invert_enum;

/* USART receiver configure */
#define CTL0_REN(regval)              (BIT(2) & ((uint32_t)(regval) << 2))
#define USART_RECEIVE_ENABLE          CTL0_REN(1)                    /*!< enable receiver */
#define USART_RECEIVE_DISABLE         CTL0_REN(0)                    /*!< disable receiver */

/* USART transmitter configure */
#define CTL0_TEN(regval)              (BIT(3) & ((uint32_t)(regval) << 3))
#define USART_TRANSMIT_ENABLE         CTL0_TEN(1)                    /*!< enable transmitter */
#define USART_TRANSMIT_DISABLE        CTL0_TEN(0)                    /*!< disable transmitter */

/* USART parity bits definitions */
#define CTL0_PM(regval)               (BITS(9,10) & ((uint32_t)(regval) << 9))
#define USART_PM_NONE                 CTL0_PM(0)                     /*!< no parity */
#define USART_PM_EVEN                 CTL0_PM(2)                     /*!< even parity */
#define USART_PM_ODD                  CTL0_PM(3)                     /*!< odd parity */

/* USART wakeup method in mute mode */
#define CTL0_WM(regval)               (BIT(11) & ((uint32_t)(regval) << 11))
#define USART_WM_IDLE                 CTL0_WM(0)                     /*!< idle line */
#define USART_WM_ADDR                 CTL0_WM(1)                     /*!< address match */

/* USART word length definitions */
#define CTL0_WL(regval)               (BIT(12) & ((uint32_t)(regval) << 12))
#define USART_WL_8BIT                 CTL0_WL(0)                     /*!< 8 bits */
#define USART_WL_9BIT                 CTL0_WL(1)                     /*!< 9 bits */

/* USART oversample mode */
#define CTL0_OVSMOD(regval)           (BIT(15) & ((uint32_t)(regval) << 15))
#define USART_OVSMOD_8                CTL0_OVSMOD(1)                 /*!< oversampling by 8 */
#define USART_OVSMOD_16               CTL0_OVSMOD(0)                 /*!< oversampling by 16 */

/* USART address detection mode */
#define CTL1_ADDM(regval)             (BIT(4) & ((uint32_t)(regval) << 4))
#define USART_ADDM_4BIT               CTL1_ADDM(0)                   /*!< 4-bit address detection */
#define USART_ADDM_FULLBIT            CTL1_ADDM(1)                   /*!< full-bit address detection */

/* USART LIN break frame length */
#define CTL1_LBLEN(regval)            (BIT(5) & ((uint32_t)(regval) << 5))
#define USART_LBLEN_10B               CTL1_LBLEN(0)                  /*!< 10 bits break detection */
#define USART_LBLEN_11B               CTL1_LBLEN(1)                  /*!< 11 bits break detection */

/* USART last bit clock pulse */
#define CTL1_CLEN(regval)             (BIT(8) & ((uint32_t)(regval) << 8))
#define USART_CLEN_NONE               CTL1_CLEN(0)                   /*!< clock pulse of the last data bit (MSB) is not output to the CK pin */
#define USART_CLEN_EN                 CTL1_CLEN(1)                   /*!< clock pulse of the last data bit (MSB) is output to the CK pin */

/* USART clock phase */
#define CTL1_CPH(regval)              (BIT(9) & ((uint32_t)(regval) << 9))
#define USART_CPH_1CK                 CTL1_CPH(0)                    /*!< first clock transition is the first data capture edge */
#define USART_CPH_2CK                 CTL1_CPH(1)                    /*!< second clock transition is the first data capture edge */

/* USART clock polarity */
#define CTL1_CPL(regval)              (BIT(10) & ((uint32_t)(regval) << 10))
#define USART_CPL_LOW                 CTL1_CPL(0)                    /*!< steady low value on CK pin */
#define USART_CPL_HIGH                CTL1_CPL(1)                    /*!< steady high value on CK pin */

/* USART stop bits definitions */
#define CTL1_STB(regval)              (BITS(12,13) & ((uint32_t)(regval) << 12))
#define USART_STB_1BIT                CTL1_STB(0)                    /*!< 1 bit */
#define USART_STB_0_5BIT              CTL1_STB(1)                    /*!< 0.5 bit */
#define USART_STB_2BIT                CTL1_STB(2)                    /*!< 2 bits */
#define USART_STB_1_5BIT              CTL1_STB(3)                    /*!< 1.5 bits */

/* USART data is transmitted/received with the LSB/MSB first */
#define CTL1_MSBF(regval)             (BIT(19) & ((uint32_t)(regval) << 19))
#define USART_MSBF_LSB                CTL1_MSBF(0)                   /*!< LSB first */
#define USART_MSBF_MSB                CTL1_MSBF(1)                   /*!< MSB first */

/* USART auto baud rate detection mode bits definitions */
#define CTL1_ABDM(regval)             (BITS(21,22) & ((uint32_t)(regval) << 21))
#define USART_ABDM_FTOR               CTL1_ABDM(0)                   /*!< falling edge to rising edge measurement */
#define USART_ABDM_FTOF               CTL1_ABDM(1)                   /*!< falling edge to falling edge measurement */

/* USART IrDA low-power enable */
#define CTL2_IRLP(regval)             (BIT(2) & ((uint32_t)(regval) << 2))
#define USART_IRLP_LOW                CTL2_IRLP(1)                   /*!< low-power */
#define USART_IRLP_NORMAL             CTL2_IRLP(0)                   /*!< normal */

/* DMA enable for reception */
#define CTL2_DENR(regval)             (BIT(6) & ((uint32_t)(regval) << 6))
#define USART_DENR_ENABLE             CTL2_DENR(1)                   /*!< enable for reception */
#define USART_DENR_DISABLE            CTL2_DENR(0)                   /*!< disable for reception */

/* DMA enable for transmission */
#define CTL2_DENT(regval)             (BIT(7) & ((uint32_t)(regval) << 7))
#define USART_DENT_ENABLE             CTL2_DENT(1)                   /*!< enable for transmission */
#define USART_DENT_DISABLE            CTL2_DENT(0)                   /*!< disable for transmission */

/* USART RTS hardware flow control configure */
#define CTL2_RTSEN(regval)            (BIT(8) & ((uint32_t)(regval) << 8))
#define USART_RTS_ENABLE              CTL2_RTSEN(1)                  /*!< RTS hardware flow control enabled */
#define USART_RTS_DISABLE             CTL2_RTSEN(0)                  /*!< RTS hardware flow control disabled */

/* USART CTS hardware flow control configure */
#define CTL2_CTSEN(regval)            (BIT(9) & ((uint32_t)(regval) << 9))
#define USART_CTS_ENABLE              CTL2_CTSEN(1)                  /*!< CTS hardware flow control enabled */
#define USART_CTS_DISABLE             CTL2_CTSEN(0)                  /*!< CTS hardware flow control disabled */

/* USART one sample bit method configure */
#define CTL2_OSB(regval)              (BIT(11) & ((uint32_t)(regval) << 11))
#define USART_OSB_1BIT                CTL2_OSB(1)                    /*!< 1 sample bit */
#define USART_OSB_3BIT                CTL2_OSB(0)                    /*!< 3 sample bits */

/* USART driver enable polarity mode */
#define CTL2_DEP(regval)              (BIT(15) & ((uint32_t)(regval) << 15))
#define USART_DEP_HIGH                CTL2_DEP(0)                    /*!< DE signal is active high */
#define USART_DEP_LOW                 CTL2_DEP(1)                    /*!< DE signal is active low */

/* USART wakeup mode from deep-sleep mode */
#define CTL2_WUM(regval)              (BITS(20,21) & ((uint32_t)(regval) << 20))
#define USART_WUM_ADDR                CTL2_WUM(0)                    /*!< WUF active on address match */
#define USART_WUM_STARTB              CTL2_WUM(2)                    /*!< WUF active on start bit */
#define USART_WUM_RBNE                CTL2_WUM(3)                    /*!< WUF active on RBNE */

/* function declarations */
/* initialization functions */
/* reset USART */
void usart_deinit(uint32_t usart_periph);
/* configure USART baud rate value */
void usart_baudrate_set(uint32_t usart_periph, uint32_t baudval);
/* configure USART parity function */
void usart_parity_config(uint32_t usart_periph, uint32_t paritycfg);
/* configure USART word length */
void usart_word_length_set(uint32_t usart_periph, uint32_t wlen);
/* configure USART stop bit length */
void usart_stop_bit_set(uint32_t usart_periph, uint32_t stblen);
/* enable USART */
void usart_enable(uint32_t usart_periph);
/* disable USART */
void usart_disable(uint32_t usart_periph);
/* configure USART transmitter */
void usart_transmit_config(uint32_t usart_periph, uint32_t txconfig);
/* configure USART receiver */
void usart_receive_config(uint32_t usart_periph, uint32_t rxconfig);

/* USART normal mode communication */
/* data is transmitted/received with the LSB/MSB first */
void usart_data_first_config(uint32_t usart_periph, uint32_t msbf);
/* configure USART inverted */
void usart_invert_config(uint32_t usart_periph, usart_invert_enum invertpara);
/* enable the USART overrun function */
void usart_overrun_enable(uint32_t usart_periph);
/* disable the USART overrun function */
void usart_overrun_disable(uint32_t usart_periph);
/* configure the USART oversample mode */
void usart_oversample_config(uint32_t usart_periph, uint32_t oversamp);
/* configure sample bit method */
void usart_sample_bit_config(uint32_t usart_periph, uint32_t osb);
/* enable receiver timeout */
void usart_receiver_timeout_enable(uint32_t usart_periph);
/* disable receiver timeout */
void usart_receiver_timeout_disable(uint32_t usart_periph);
/* configure receiver timeout threshold */
void usart_receiver_timeout_threshold_config(uint32_t usart_periph, uint32_t rtimeout);
/* USART transmit data function */
void usart_data_transmit(uint32_t usart_periph, uint32_t data);
/* USART receive data function */
uint16_t usart_data_receive(uint32_t usart_periph);

/* auto baud rate detection */
/* enable auto baud rate detection */
void usart_autobaud_detection_enable(uint32_t usart_periph);
/* disable auto baud rate detection */
void usart_autobaud_detection_disable(uint32_t usart_periph);
/* configure auto baud rate detection mode */
void usart_autobaud_detection_mode_config(uint32_t usart_periph, uint32_t abdmod);

/* multi-processor communication */
/* configure the address of the USART in wake up by address match mode */
void usart_address_config(uint32_t usart_periph, uint8_t addr);
/* configure address detection mode */
void usart_address_detection_mode_config(uint32_t usart_periph, uint32_t addmod);
/* enable mute mode */
void usart_mute_mode_enable(uint32_t usart_periph);
/* disable mute mode */
void usart_mute_mode_disable(uint32_t usart_periph);
/* configure wakeup method in mute mode */
void usart_mute_mode_wakeup_config(uint32_t usart_periph, uint32_t wmethod);

/* LIN mode communication */
/* enable LIN mode */
void usart_lin_mode_enable(uint32_t usart_periph);
/* disable LIN mode */
void usart_lin_mode_disable(uint32_t usart_periph);
/* configure LIN break frame length */
void usart_lin_break_detection_length_config(uint32_t usart_periph, uint32_t lblen);

/* half-duplex communication */
/* enable half-duplex mode */
void usart_halfduplex_enable(uint32_t usart_periph);
/* disable half-duplex mode */
void usart_halfduplex_disable(uint32_t usart_periph);

/* synchronous communication */
/* enable USART clock */
void usart_clock_enable(uint32_t usart_periph);
/* disable USART clock */
void usart_clock_disable(uint32_t usart_periph);
/* configure USART synchronous mode parameters */
void usart_synchronous_clock_config(uint32_t usart_periph, uint32_t clen, uint32_t cph, uint32_t cpl);

/* smartcard communication */
/* configure guard time value in smartcard mode */
void usart_guard_time_config(uint32_t usart_periph, uint32_t guat);
/* enable smartcard mode */
void usart_smartcard_mode_enable(uint32_t usart_periph);
/* disable smartcard mode */
void usart_smartcard_mode_disable(uint32_t usart_periph);
/* enable NACK in smartcard mode */
void usart_smartcard_mode_nack_enable(uint32_t usart_periph);
/* disable NACK in smartcard mode */
void usart_smartcard_mode_nack_disable(uint32_t usart_periph);
/* enable early NACK in smartcard mode */
void usart_smartcard_mode_early_nack_enable(uint32_t usart_periph);
/* disable early NACK in smartcard mode */
void usart_smartcard_mode_early_nack_disable(uint32_t usart_periph);
/* configure smartcard auto-retry number */
void usart_smartcard_autoretry_config(uint32_t usart_periph, uint32_t scrtnum);
/* configure block length */
void usart_block_length_config(uint32_t usart_periph, uint32_t bl);

/* IrDA communication */
/* enable IrDA mode */
void usart_irda_mode_enable(uint32_t usart_periph);
/* disable IrDA mode */
void usart_irda_mode_disable(uint32_t usart_periph);
/* configure the peripheral clock prescaler in USART IrDA low-power mode or SmartCard mode */
void usart_prescaler_config(uint32_t usart_periph, uint32_t psc);
/* configure IrDA low-power */
void usart_irda_lowpower_config(uint32_t usart_periph, uint32_t irlp);

/* hardware flow communication */
/* configure hardware flow control RTS */
void usart_hardware_flow_rts_config(uint32_t usart_periph, uint32_t rtsconfig);
/* configure hardware flow control CTS */
void usart_hardware_flow_cts_config(uint32_t usart_periph, uint32_t ctsconfig);

/* enable RS485 driver */
void usart_rs485_driver_enable(uint32_t usart_periph);
/* disable RS485 driver */
void usart_rs485_driver_disable(uint32_t usart_periph);
/* configure driver enable assertion time */
void usart_driver_assertime_config(uint32_t usart_periph, uint32_t deatime);
/* configure driver enable de-assertion time */
void usart_driver_deassertime_config(uint32_t usart_periph, uint32_t dedtime);
/* configure driver enable polarity mode */
void usart_depolarity_config(uint32_t usart_periph, uint32_t dep);

/* USART DMA */
/* configure USART DMA for reception */
void usart_dma_receive_config(uint32_t usart_periph, uint32_t dmacmd);
/* configure USART DMA for transmission */
void usart_dma_transmit_config(uint32_t usart_periph, uint32_t dmacmd);
/* disable DMA on reception error */
void usart_reception_error_dma_disable(uint32_t usart_periph);
/* enable DMA on reception error */
void usart_reception_error_dma_enable(uint32_t usart_periph);

/* enable USART to wakeup the mcu from deep-sleep mode */
void usart_wakeup_enable(uint32_t usart_periph);
/* disable USART to wakeup the mcu from deep-sleep mode */
void usart_wakeup_disable(uint32_t usart_periph);
/* configure the USART wakeup mode from deep-sleep mode */
void usart_wakeup_mode_config(uint32_t usart_periph, uint32_t wum);
/* enable USART command */
void usart_command_enable(uint32_t usart_periph, uint32_t cmdtype);

/* USART receive FIFO */
/* enable receive FIFO */
void usart_receive_fifo_enable(uint32_t usart_periph);
/* disable receive FIFO */
void usart_receive_fifo_disable(uint32_t usart_periph);
/* read receive FIFO counter number */
uint8_t usart_receive_fifo_counter_number(uint32_t usart_periph);

/* flag & interrupt functions */
/* get flag in STAT/RFCS register */
FlagStatus usart_flag_get(uint32_t usart_periph, usart_flag_enum flag);
/* clear flag in STAT register */
void usart_flag_clear(uint32_t usart_periph, usart_flag_enum flag);
/* enable USART interrupt */
void usart_interrupt_enable(uint32_t usart_periph, usart_interrupt_enum interrupt);
/* disable USART interrupt */
void usart_interrupt_disable(uint32_t usart_periph, usart_interrupt_enum interrupt);
/* get USART interrupt and flag status */
FlagStatus usart_interrupt_flag_get(uint32_t usart_periph, usart_interrupt_flag_enum int_flag);
/* clear USART interrupt flag */
void usart_interrupt_flag_clear(uint32_t usart_periph, usart_interrupt_flag_enum int_flag);


/******************************************************************************/
/*                                                                            */
/*      Universal Synchronous Asynchronous Receiver Transmitter (USART)       */
/*                                                                            */
/******************************************************************************/
/*
 * @brief Specific device feature definitions (not present on all devices in the STM32F7 serie)
 */
/* Support of TCBGT feature : Supported from USART IP version c7amba_sci3 v1.3 */

/******************  Bit definition for USART_CR1 register  *******************/
#define USART_CR1_UE_Pos              (0U)                                     
#define USART_CR1_UE_Msk              (0x1UL << USART_CR1_UE_Pos)               /*!< 0x00000001 */
#define USART_CR1_UE                  USART_CR1_UE_Msk                         /*!< USART Enable                                    */
#define USART_CR1_RE_Pos              (2U)                                     
#define USART_CR1_RE_Msk              (0x1UL << USART_CR1_RE_Pos)               /*!< 0x00000004 */
#define USART_CR1_RE                  USART_CR1_RE_Msk                         /*!< Receiver Enable                                 */
#define USART_CR1_TE_Pos              (3U)                                     
#define USART_CR1_TE_Msk              (0x1UL << USART_CR1_TE_Pos)               /*!< 0x00000008 */
#define USART_CR1_TE                  USART_CR1_TE_Msk                         /*!< Transmitter Enable                              */
#define USART_CR1_IDLEIE_Pos          (4U)                                     
#define USART_CR1_IDLEIE_Msk          (0x1UL << USART_CR1_IDLEIE_Pos)           /*!< 0x00000010 */
#define USART_CR1_IDLEIE              USART_CR1_IDLEIE_Msk                     /*!< IDLE Interrupt Enable                           */
#define USART_CR1_RXNEIE_Pos          (5U)                                     
#define USART_CR1_RXNEIE_Msk          (0x1UL << USART_CR1_RXNEIE_Pos)           /*!< 0x00000020 */
#define USART_CR1_RXNEIE              USART_CR1_RXNEIE_Msk                     /*!< RXNE Interrupt Enable                           */
#define USART_CR1_TCIE_Pos            (6U)                                     
#define USART_CR1_TCIE_Msk            (0x1UL << USART_CR1_TCIE_Pos)             /*!< 0x00000040 */
#define USART_CR1_TCIE                USART_CR1_TCIE_Msk                       /*!< Transmission Complete Interrupt Enable          */
#define USART_CR1_TXEIE_Pos           (7U)                                     
#define USART_CR1_TXEIE_Msk           (0x1UL << USART_CR1_TXEIE_Pos)            /*!< 0x00000080 */
#define USART_CR1_TXEIE               USART_CR1_TXEIE_Msk                      /*!< TXE Interrupt Enable                            */
#define USART_CR1_PEIE_Pos            (8U)                                     
#define USART_CR1_PEIE_Msk            (0x1UL << USART_CR1_PEIE_Pos)             /*!< 0x00000100 */
#define USART_CR1_PEIE                USART_CR1_PEIE_Msk                       /*!< PE Interrupt Enable                             */
#define USART_CR1_PS_Pos              (9U)                                     
#define USART_CR1_PS_Msk              (0x1UL << USART_CR1_PS_Pos)               /*!< 0x00000200 */
#define USART_CR1_PS                  USART_CR1_PS_Msk                         /*!< Parity Selection                                */
#define USART_CR1_PCE_Pos             (10U)                                    
#define USART_CR1_PCE_Msk             (0x1UL << USART_CR1_PCE_Pos)              /*!< 0x00000400 */
#define USART_CR1_PCE                 USART_CR1_PCE_Msk                        /*!< Parity Control Enable                           */
#define USART_CR1_WAKE_Pos            (11U)                                    
#define USART_CR1_WAKE_Msk            (0x1UL << USART_CR1_WAKE_Pos)             /*!< 0x00000800 */
#define USART_CR1_WAKE                USART_CR1_WAKE_Msk                       /*!< Receiver Wakeup method                          */
#define USART_CR1_M_Pos               (12U)                                    
#define USART_CR1_M_Msk               (0x10001UL << USART_CR1_M_Pos)            /*!< 0x10001000 */
#define USART_CR1_M                   USART_CR1_M_Msk                          /*!< Word length                                     */
#define USART_CR1_M0                  (0x00001UL << USART_CR1_M_Pos)            /*!< 0x00001000 */
#define USART_CR1_MME_Pos             (13U)                                    
#define USART_CR1_MME_Msk             (0x1UL << USART_CR1_MME_Pos)              /*!< 0x00002000 */
#define USART_CR1_MME                 USART_CR1_MME_Msk                        /*!< Mute Mode Enable                                */
#define USART_CR1_CMIE_Pos            (14U)                                    
#define USART_CR1_CMIE_Msk            (0x1UL << USART_CR1_CMIE_Pos)             /*!< 0x00004000 */
#define USART_CR1_CMIE                USART_CR1_CMIE_Msk                       /*!< Character match interrupt enable                */
#define USART_CR1_OVER8_Pos           (15U)                                    
#define USART_CR1_OVER8_Msk           (0x1UL << USART_CR1_OVER8_Pos)            /*!< 0x00008000 */
#define USART_CR1_OVER8               USART_CR1_OVER8_Msk                      /*!< Oversampling by 8-bit or 16-bit mode            */
#define USART_CR1_DEDT_Pos            (16U)                                    
#define USART_CR1_DEDT_Msk            (0x1FUL << USART_CR1_DEDT_Pos)            /*!< 0x001F0000 */
#define USART_CR1_DEDT                USART_CR1_DEDT_Msk                       /*!< DEDT[4:0] bits (Driver Enable Deassertion Time) */
#define USART_CR1_DEDT_0              (0x01UL << USART_CR1_DEDT_Pos)            /*!< 0x00010000 */
#define USART_CR1_DEDT_1              (0x02UL << USART_CR1_DEDT_Pos)            /*!< 0x00020000 */
#define USART_CR1_DEDT_2              (0x04UL << USART_CR1_DEDT_Pos)            /*!< 0x00040000 */
#define USART_CR1_DEDT_3              (0x08UL << USART_CR1_DEDT_Pos)            /*!< 0x00080000 */
#define USART_CR1_DEDT_4              (0x10UL << USART_CR1_DEDT_Pos)            /*!< 0x00100000 */
#define USART_CR1_DEAT_Pos            (21U)                                    
#define USART_CR1_DEAT_Msk            (0x1FUL << USART_CR1_DEAT_Pos)            /*!< 0x03E00000 */
#define USART_CR1_DEAT                USART_CR1_DEAT_Msk                       /*!< DEAT[4:0] bits (Driver Enable Assertion Time)   */
#define USART_CR1_DEAT_0              (0x01UL << USART_CR1_DEAT_Pos)            /*!< 0x00200000 */
#define USART_CR1_DEAT_1              (0x02UL << USART_CR1_DEAT_Pos)            /*!< 0x00400000 */
#define USART_CR1_DEAT_2              (0x04UL << USART_CR1_DEAT_Pos)            /*!< 0x00800000 */
#define USART_CR1_DEAT_3              (0x08UL << USART_CR1_DEAT_Pos)            /*!< 0x01000000 */
#define USART_CR1_DEAT_4              (0x10UL << USART_CR1_DEAT_Pos)            /*!< 0x02000000 */
#define USART_CR1_RTOIE_Pos           (26U)                                    
#define USART_CR1_RTOIE_Msk           (0x1UL << USART_CR1_RTOIE_Pos)            /*!< 0x04000000 */
#define USART_CR1_RTOIE               USART_CR1_RTOIE_Msk                      /*!< Receive Time Out interrupt enable */
#define USART_CR1_EOBIE_Pos           (27U)                                    
#define USART_CR1_EOBIE_Msk           (0x1UL << USART_CR1_EOBIE_Pos)            /*!< 0x08000000 */
#define USART_CR1_EOBIE               USART_CR1_EOBIE_Msk                      /*!< End of Block interrupt enable     */
#define USART_CR1_M1                  0x10000000U                              /*!< Word length - Bit 1               */

/* Legacy defines */
#define  USART_CR1_M_0                       USART_CR1_M0          /*!< Word length - Bit 0               */
#define  USART_CR1_M_1                       USART_CR1_M1          /*!< Word length - Bit 1               */

/******************  Bit definition for USART_CR2 register  *******************/
#define USART_CR2_ADDM7_Pos           (4U)                                     
#define USART_CR2_ADDM7_Msk           (0x1UL << USART_CR2_ADDM7_Pos)            /*!< 0x00000010 */
#define USART_CR2_ADDM7               USART_CR2_ADDM7_Msk                      /*!< 7-bit or 4-bit Address Detection       */
#define USART_CR2_LBDL_Pos            (5U)                                     
#define USART_CR2_LBDL_Msk            (0x1UL << USART_CR2_LBDL_Pos)             /*!< 0x00000020 */
#define USART_CR2_LBDL                USART_CR2_LBDL_Msk                       /*!< LIN Break Detection Length             */
#define USART_CR2_LBDIE_Pos           (6U)                                     
#define USART_CR2_LBDIE_Msk           (0x1UL << USART_CR2_LBDIE_Pos)            /*!< 0x00000040 */
#define USART_CR2_LBDIE               USART_CR2_LBDIE_Msk                      /*!< LIN Break Detection Interrupt Enable   */
#define USART_CR2_LBCL_Pos            (8U)                                     
#define USART_CR2_LBCL_Msk            (0x1UL << USART_CR2_LBCL_Pos)             /*!< 0x00000100 */
#define USART_CR2_LBCL                USART_CR2_LBCL_Msk                       /*!< Last Bit Clock pulse                   */
#define USART_CR2_CPHA_Pos            (9U)                                     
#define USART_CR2_CPHA_Msk            (0x1UL << USART_CR2_CPHA_Pos)             /*!< 0x00000200 */
#define USART_CR2_CPHA                USART_CR2_CPHA_Msk                       /*!< Clock Phase                            */
#define USART_CR2_CPOL_Pos            (10U)                                    
#define USART_CR2_CPOL_Msk            (0x1UL << USART_CR2_CPOL_Pos)             /*!< 0x00000400 */
#define USART_CR2_CPOL                USART_CR2_CPOL_Msk                       /*!< Clock Polarity                         */
#define USART_CR2_CLKEN_Pos           (11U)                                    
#define USART_CR2_CLKEN_Msk           (0x1UL << USART_CR2_CLKEN_Pos)            /*!< 0x00000800 */
#define USART_CR2_CLKEN               USART_CR2_CLKEN_Msk                      /*!< Clock Enable                           */
#define USART_CR2_STOP_Pos            (12U)                                    
#define USART_CR2_STOP_Msk            (0x3UL << USART_CR2_STOP_Pos)             /*!< 0x00003000 */
#define USART_CR2_STOP                USART_CR2_STOP_Msk                       /*!< STOP[1:0] bits (STOP bits)             */
#define USART_CR2_STOP_0              (0x1UL << USART_CR2_STOP_Pos)             /*!< 0x00001000 */
#define USART_CR2_STOP_1              (0x2UL << USART_CR2_STOP_Pos)             /*!< 0x00002000 */
#define USART_CR2_LINEN_Pos           (14U)                                    
#define USART_CR2_LINEN_Msk           (0x1UL << USART_CR2_LINEN_Pos)            /*!< 0x00004000 */
#define USART_CR2_LINEN               USART_CR2_LINEN_Msk                      /*!< LIN mode enable                        */
#define USART_CR2_SWAP_Pos            (15U)                                    
#define USART_CR2_SWAP_Msk            (0x1UL << USART_CR2_SWAP_Pos)             /*!< 0x00008000 */
#define USART_CR2_SWAP                USART_CR2_SWAP_Msk                       /*!< SWAP TX/RX pins                        */
#define USART_CR2_RXINV_Pos           (16U)                                    
#define USART_CR2_RXINV_Msk           (0x1UL << USART_CR2_RXINV_Pos)            /*!< 0x00010000 */
#define USART_CR2_RXINV               USART_CR2_RXINV_Msk                      /*!< RX pin active level inversion          */
#define USART_CR2_TXINV_Pos           (17U)                                    
#define USART_CR2_TXINV_Msk           (0x1UL << USART_CR2_TXINV_Pos)            /*!< 0x00020000 */
#define USART_CR2_TXINV               USART_CR2_TXINV_Msk                      /*!< TX pin active level inversion          */
#define USART_CR2_DATAINV_Pos         (18U)                                    
#define USART_CR2_DATAINV_Msk         (0x1UL << USART_CR2_DATAINV_Pos)          /*!< 0x00040000 */
#define USART_CR2_DATAINV             USART_CR2_DATAINV_Msk                    /*!< Binary data inversion                  */
#define USART_CR2_MSBFIRST_Pos        (19U)                                    
#define USART_CR2_MSBFIRST_Msk        (0x1UL << USART_CR2_MSBFIRST_Pos)         /*!< 0x00080000 */
#define USART_CR2_MSBFIRST            USART_CR2_MSBFIRST_Msk                   /*!< Most Significant Bit First             */
#define USART_CR2_ABREN_Pos           (20U)                                    
#define USART_CR2_ABREN_Msk           (0x1UL << USART_CR2_ABREN_Pos)            /*!< 0x00100000 */
#define USART_CR2_ABREN               USART_CR2_ABREN_Msk                      /*!< Auto Baud-Rate Enable                  */
#define USART_CR2_ABRMODE_Pos         (21U)                                    
#define USART_CR2_ABRMODE_Msk         (0x3UL << USART_CR2_ABRMODE_Pos)          /*!< 0x00600000 */
#define USART_CR2_ABRMODE             USART_CR2_ABRMODE_Msk                    /*!< ABRMOD[1:0] bits (Auto Baud-Rate Mode) */
#define USART_CR2_ABRMODE_0           (0x1UL << USART_CR2_ABRMODE_Pos)          /*!< 0x00200000 */
#define USART_CR2_ABRMODE_1           (0x2UL << USART_CR2_ABRMODE_Pos)          /*!< 0x00400000 */
#define USART_CR2_RTOEN_Pos           (23U)                                    
#define USART_CR2_RTOEN_Msk           (0x1UL << USART_CR2_RTOEN_Pos)            /*!< 0x00800000 */
#define USART_CR2_RTOEN               USART_CR2_RTOEN_Msk                      /*!< Receiver Time-Out enable  */
#define USART_CR2_ADD_Pos             (24U)                                    
#define USART_CR2_ADD_Msk             (0xFFUL << USART_CR2_ADD_Pos)             /*!< 0xFF000000 */
#define USART_CR2_ADD                 USART_CR2_ADD_Msk                        /*!< Address of the USART node */

/******************  Bit definition for USART_CR3 register  *******************/
#define USART_CR3_EIE_Pos             (0U)                                     
#define USART_CR3_EIE_Msk             (0x1UL << USART_CR3_EIE_Pos)              /*!< 0x00000001 */
#define USART_CR3_EIE                 USART_CR3_EIE_Msk                        /*!< Error Interrupt Enable                         */
#define USART_CR3_IREN_Pos            (1U)                                     
#define USART_CR3_IREN_Msk            (0x1UL << USART_CR3_IREN_Pos)             /*!< 0x00000002 */
#define USART_CR3_IREN                USART_CR3_IREN_Msk                       /*!< IrDA mode Enable                               */
#define USART_CR3_IRLP_Pos            (2U)                                     
#define USART_CR3_IRLP_Msk            (0x1UL << USART_CR3_IRLP_Pos)             /*!< 0x00000004 */
#define USART_CR3_IRLP                USART_CR3_IRLP_Msk                       /*!< IrDA Low-Power                                 */
#define USART_CR3_HDSEL_Pos           (3U)                                     
#define USART_CR3_HDSEL_Msk           (0x1UL << USART_CR3_HDSEL_Pos)            /*!< 0x00000008 */
#define USART_CR3_HDSEL               USART_CR3_HDSEL_Msk                      /*!< Half-Duplex Selection                          */
#define USART_CR3_NACK_Pos            (4U)                                     
#define USART_CR3_NACK_Msk            (0x1UL << USART_CR3_NACK_Pos)             /*!< 0x00000010 */
#define USART_CR3_NACK                USART_CR3_NACK_Msk                       /*!< SmartCard NACK enable                          */
#define USART_CR3_SCEN_Pos            (5U)                                     
#define USART_CR3_SCEN_Msk            (0x1UL << USART_CR3_SCEN_Pos)             /*!< 0x00000020 */
#define USART_CR3_SCEN                USART_CR3_SCEN_Msk                       /*!< SmartCard mode enable                          */
#define USART_CR3_DMAR_Pos            (6U)                                     
#define USART_CR3_DMAR_Msk            (0x1UL << USART_CR3_DMAR_Pos)             /*!< 0x00000040 */
#define USART_CR3_DMAR                USART_CR3_DMAR_Msk                       /*!< DMA Enable Receiver                            */
#define USART_CR3_DMAT_Pos            (7U)                                     
#define USART_CR3_DMAT_Msk            (0x1UL << USART_CR3_DMAT_Pos)             /*!< 0x00000080 */
#define USART_CR3_DMAT                USART_CR3_DMAT_Msk                       /*!< DMA Enable Transmitter                         */
#define USART_CR3_RTSE_Pos            (8U)                                     
#define USART_CR3_RTSE_Msk            (0x1UL << USART_CR3_RTSE_Pos)             /*!< 0x00000100 */
#define USART_CR3_RTSE                USART_CR3_RTSE_Msk                       /*!< RTS Enable                                     */
#define USART_CR3_CTSE_Pos            (9U)                                     
#define USART_CR3_CTSE_Msk            (0x1UL << USART_CR3_CTSE_Pos)             /*!< 0x00000200 */
#define USART_CR3_CTSE                USART_CR3_CTSE_Msk                       /*!< CTS Enable                                     */
#define USART_CR3_CTSIE_Pos           (10U)                                    
#define USART_CR3_CTSIE_Msk           (0x1UL << USART_CR3_CTSIE_Pos)            /*!< 0x00000400 */
#define USART_CR3_CTSIE               USART_CR3_CTSIE_Msk                      /*!< CTS Interrupt Enable                           */
#define USART_CR3_ONEBIT_Pos          (11U)                                    
#define USART_CR3_ONEBIT_Msk          (0x1UL << USART_CR3_ONEBIT_Pos)           /*!< 0x00000800 */
#define USART_CR3_ONEBIT              USART_CR3_ONEBIT_Msk                     /*!< One sample bit method enable                   */
#define USART_CR3_OVRDIS_Pos          (12U)                                    
#define USART_CR3_OVRDIS_Msk          (0x1UL << USART_CR3_OVRDIS_Pos)           /*!< 0x00001000 */
#define USART_CR3_OVRDIS              USART_CR3_OVRDIS_Msk                     /*!< Overrun Disable                                */
#define USART_CR3_DDRE_Pos            (13U)                                    
#define USART_CR3_DDRE_Msk            (0x1UL << USART_CR3_DDRE_Pos)             /*!< 0x00002000 */
#define USART_CR3_DDRE                USART_CR3_DDRE_Msk                       /*!< DMA Disable on Reception Error                 */
#define USART_CR3_DEM_Pos             (14U)                                    
#define USART_CR3_DEM_Msk             (0x1UL << USART_CR3_DEM_Pos)              /*!< 0x00004000 */
#define USART_CR3_DEM                 USART_CR3_DEM_Msk                        /*!< Driver Enable Mode                             */
#define USART_CR3_DEP_Pos             (15U)                                    
#define USART_CR3_DEP_Msk             (0x1UL << USART_CR3_DEP_Pos)              /*!< 0x00008000 */
#define USART_CR3_DEP                 USART_CR3_DEP_Msk                        /*!< Driver Enable Polarity Selection               */
#define USART_CR3_SCARCNT_Pos         (17U)                                    
#define USART_CR3_SCARCNT_Msk         (0x7UL << USART_CR3_SCARCNT_Pos)          /*!< 0x000E0000 */
#define USART_CR3_SCARCNT             USART_CR3_SCARCNT_Msk                    /*!< SCARCNT[2:0] bits (SmartCard Auto-Retry Count) */
#define USART_CR3_SCARCNT_0           (0x1UL << USART_CR3_SCARCNT_Pos)          /*!< 0x00020000 */
#define USART_CR3_SCARCNT_1           (0x2UL << USART_CR3_SCARCNT_Pos)          /*!< 0x00040000 */
#define USART_CR3_SCARCNT_2           (0x4UL << USART_CR3_SCARCNT_Pos)          /*!< 0x00080000 */
#define USART_CR3_TCBGTIE_Pos         (24U)                                    
#define USART_CR3_TCBGTIE_Msk         (0x1UL << USART_CR3_TCBGTIE_Pos)          /*!< 0x01000000 */
#define USART_CR3_TCBGTIE             USART_CR3_TCBGTIE_Msk                    /*!< Transmission complete before guard time interrupt enable */

/******************  Bit definition for USART_BRR register  *******************/
#define USART_BRR_DIV_FRACTION_Pos    (0U)                                     
#define USART_BRR_DIV_FRACTION_Msk    (0xFUL << USART_BRR_DIV_FRACTION_Pos)     /*!< 0x0000000F */
#define USART_BRR_DIV_FRACTION        USART_BRR_DIV_FRACTION_Msk               /*!< Fraction of USARTDIV */
#define USART_BRR_DIV_MANTISSA_Pos    (4U)                                     
#define USART_BRR_DIV_MANTISSA_Msk    (0xFFFUL << USART_BRR_DIV_MANTISSA_Pos)   /*!< 0x0000FFF0 */
#define USART_BRR_DIV_MANTISSA        USART_BRR_DIV_MANTISSA_Msk               /*!< Mantissa of USARTDIV */

/******************  Bit definition for USART_GTPR register  ******************/
#define USART_GTPR_PSC_Pos            (0U)                                     
#define USART_GTPR_PSC_Msk            (0xFFUL << USART_GTPR_PSC_Pos)            /*!< 0x000000FF */
#define USART_GTPR_PSC                USART_GTPR_PSC_Msk                       /*!< PSC[7:0] bits (Prescaler value) */
#define USART_GTPR_GT_Pos             (8U)                                     
#define USART_GTPR_GT_Msk             (0xFFUL << USART_GTPR_GT_Pos)             /*!< 0x0000FF00 */
#define USART_GTPR_GT                 USART_GTPR_GT_Msk                        /*!< GT[7:0] bits (Guard time value) */


/*******************  Bit definition for USART_RTOR register  *****************/
#define USART_RTOR_RTO_Pos            (0U)                                     
#define USART_RTOR_RTO_Msk            (0xFFFFFFUL << USART_RTOR_RTO_Pos)        /*!< 0x00FFFFFF */
#define USART_RTOR_RTO                USART_RTOR_RTO_Msk                       /*!< Receiver Time Out Value */
#define USART_RTOR_BLEN_Pos           (24U)                                    
#define USART_RTOR_BLEN_Msk           (0xFFUL << USART_RTOR_BLEN_Pos)           /*!< 0xFF000000 */
#define USART_RTOR_BLEN               USART_RTOR_BLEN_Msk                      /*!< Block Length */

/*******************  Bit definition for USART_RQR register  ******************/
#define USART_RQR_ABRRQ_Pos           (0U)                                     
#define USART_RQR_ABRRQ_Msk           (0x1UL << USART_RQR_ABRRQ_Pos)            /*!< 0x00000001 */
#define USART_RQR_ABRRQ               USART_RQR_ABRRQ_Msk                      /*!< Auto-Baud Rate Request      */
#define USART_RQR_SBKRQ_Pos           (1U)                                     
#define USART_RQR_SBKRQ_Msk           (0x1UL << USART_RQR_SBKRQ_Pos)            /*!< 0x00000002 */
#define USART_RQR_SBKRQ               USART_RQR_SBKRQ_Msk                      /*!< Send Break Request          */
#define USART_RQR_MMRQ_Pos            (2U)                                     
#define USART_RQR_MMRQ_Msk            (0x1UL << USART_RQR_MMRQ_Pos)             /*!< 0x00000004 */
#define USART_RQR_MMRQ                USART_RQR_MMRQ_Msk                       /*!< Mute Mode Request           */
#define USART_RQR_RXFRQ_Pos           (3U)                                     
#define USART_RQR_RXFRQ_Msk           (0x1UL << USART_RQR_RXFRQ_Pos)            /*!< 0x00000008 */
#define USART_RQR_RXFRQ               USART_RQR_RXFRQ_Msk                      /*!< Receive Data flush Request  */
#define USART_RQR_TXFRQ_Pos           (4U)                                     
#define USART_RQR_TXFRQ_Msk           (0x1UL << USART_RQR_TXFRQ_Pos)            /*!< 0x00000010 */
#define USART_RQR_TXFRQ               USART_RQR_TXFRQ_Msk                      /*!< Transmit data flush Request */

/*******************  Bit definition for USART_ISR register  ******************/
#define USART_ISR_PE_Pos              (0U)                                     
#define USART_ISR_PE_Msk              (0x1UL << USART_ISR_PE_Pos)               /*!< 0x00000001 */
#define USART_ISR_PE                  USART_ISR_PE_Msk                         /*!< Parity Error                        */
#define USART_ISR_FE_Pos              (1U)                                     
#define USART_ISR_FE_Msk              (0x1UL << USART_ISR_FE_Pos)               /*!< 0x00000002 */
#define USART_ISR_FE                  USART_ISR_FE_Msk                         /*!< Framing Error                       */
#define USART_ISR_NE_Pos              (2U)                                     
#define USART_ISR_NE_Msk              (0x1UL << USART_ISR_NE_Pos)               /*!< 0x00000004 */
#define USART_ISR_NE                  USART_ISR_NE_Msk                         /*!< Noise detected Flag                 */
#define USART_ISR_ORE_Pos             (3U)                                     
#define USART_ISR_ORE_Msk             (0x1UL << USART_ISR_ORE_Pos)              /*!< 0x00000008 */
#define USART_ISR_ORE                 USART_ISR_ORE_Msk                        /*!< OverRun Error                       */
#define USART_ISR_IDLE_Pos            (4U)                                     
#define USART_ISR_IDLE_Msk            (0x1UL << USART_ISR_IDLE_Pos)             /*!< 0x00000010 */
#define USART_ISR_IDLE                USART_ISR_IDLE_Msk                       /*!< IDLE line detected                  */
#define USART_ISR_RXNE_Pos            (5U)                                     
#define USART_ISR_RXNE_Msk            (0x1UL << USART_ISR_RXNE_Pos)             /*!< 0x00000020 */
#define USART_ISR_RXNE                USART_ISR_RXNE_Msk                       /*!< Read Data Register Not Empty        */
#define USART_ISR_TC_Pos              (6U)                                     
#define USART_ISR_TC_Msk              (0x1UL << USART_ISR_TC_Pos)               /*!< 0x00000040 */
#define USART_ISR_TC                  USART_ISR_TC_Msk                         /*!< Transmission Complete               */
#define USART_ISR_TXE_Pos             (7U)                                     
#define USART_ISR_TXE_Msk             (0x1UL << USART_ISR_TXE_Pos)              /*!< 0x00000080 */
#define USART_ISR_TXE                 USART_ISR_TXE_Msk                        /*!< Transmit Data Register Empty        */
#define USART_ISR_LBDF_Pos            (8U)                                     
#define USART_ISR_LBDF_Msk            (0x1UL << USART_ISR_LBDF_Pos)             /*!< 0x00000100 */
#define USART_ISR_LBDF                USART_ISR_LBDF_Msk                       /*!< LIN Break Detection Flag            */
#define USART_ISR_CTSIF_Pos           (9U)                                     
#define USART_ISR_CTSIF_Msk           (0x1UL << USART_ISR_CTSIF_Pos)            /*!< 0x00000200 */
#define USART_ISR_CTSIF               USART_ISR_CTSIF_Msk                      /*!< CTS interrupt flag                  */
#define USART_ISR_CTS_Pos             (10U)                                    
#define USART_ISR_CTS_Msk             (0x1UL << USART_ISR_CTS_Pos)              /*!< 0x00000400 */
#define USART_ISR_CTS                 USART_ISR_CTS_Msk                        /*!< CTS flag                            */
#define USART_ISR_RTOF_Pos            (11U)                                    
#define USART_ISR_RTOF_Msk            (0x1UL << USART_ISR_RTOF_Pos)             /*!< 0x00000800 */
#define USART_ISR_RTOF                USART_ISR_RTOF_Msk                       /*!< Receiver Time Out                   */
#define USART_ISR_EOBF_Pos            (12U)                                    
#define USART_ISR_EOBF_Msk            (0x1UL << USART_ISR_EOBF_Pos)             /*!< 0x00001000 */
#define USART_ISR_EOBF                USART_ISR_EOBF_Msk                       /*!< End Of Block Flag                   */
#define USART_ISR_ABRE_Pos            (14U)                                    
#define USART_ISR_ABRE_Msk            (0x1UL << USART_ISR_ABRE_Pos)             /*!< 0x00004000 */
#define USART_ISR_ABRE                USART_ISR_ABRE_Msk                       /*!< Auto-Baud Rate Error                */
#define USART_ISR_ABRF_Pos            (15U)                                    
#define USART_ISR_ABRF_Msk            (0x1UL << USART_ISR_ABRF_Pos)             /*!< 0x00008000 */
#define USART_ISR_ABRF                USART_ISR_ABRF_Msk                       /*!< Auto-Baud Rate Flag                 */
#define USART_ISR_BUSY_Pos            (16U)                                    
#define USART_ISR_BUSY_Msk            (0x1UL << USART_ISR_BUSY_Pos)             /*!< 0x00010000 */
#define USART_ISR_BUSY                USART_ISR_BUSY_Msk                       /*!< Busy Flag                           */
#define USART_ISR_CMF_Pos             (17U)                                    
#define USART_ISR_CMF_Msk             (0x1UL << USART_ISR_CMF_Pos)              /*!< 0x00020000 */
#define USART_ISR_CMF                 USART_ISR_CMF_Msk                        /*!< Character Match Flag                */
#define USART_ISR_SBKF_Pos            (18U)                                    
#define USART_ISR_SBKF_Msk            (0x1UL << USART_ISR_SBKF_Pos)             /*!< 0x00040000 */
#define USART_ISR_SBKF                USART_ISR_SBKF_Msk                       /*!< Send Break Flag                     */
#define USART_ISR_RWU_Pos             (19U)                                    
#define USART_ISR_RWU_Msk             (0x1UL << USART_ISR_RWU_Pos)              /*!< 0x00080000 */
#define USART_ISR_RWU                 USART_ISR_RWU_Msk                        /*!< Receive Wake Up from mute mode Flag */
#define USART_ISR_TEACK_Pos           (21U)                                    
#define USART_ISR_TEACK_Msk           (0x1UL << USART_ISR_TEACK_Pos)            /*!< 0x00200000 */
#define USART_ISR_TEACK               USART_ISR_TEACK_Msk                      /*!< Transmit Enable Acknowledge Flag    */
#define USART_ISR_TCBGT_Pos           (25U)                                    
#define USART_ISR_TCBGT_Msk           (0x1UL << USART_ISR_TCBGT_Pos)          /*!< 0x02000000 */
#define USART_ISR_TCBGT               USART_ISR_TCBGT_Msk                    /*!< Transmission complete before guard time flag */

/*******************  Bit definition for USART_ICR register  ******************/
#define USART_ICR_PECF_Pos            (0U)                                     
#define USART_ICR_PECF_Msk            (0x1UL << USART_ICR_PECF_Pos)             /*!< 0x00000001 */
#define USART_ICR_PECF                USART_ICR_PECF_Msk                       /*!< Parity Error Clear Flag             */
#define USART_ICR_FECF_Pos            (1U)                                     
#define USART_ICR_FECF_Msk            (0x1UL << USART_ICR_FECF_Pos)             /*!< 0x00000002 */
#define USART_ICR_FECF                USART_ICR_FECF_Msk                       /*!< Framing Error Clear Flag            */
#define USART_ICR_NCF_Pos             (2U)                                     
#define USART_ICR_NCF_Msk             (0x1UL << USART_ICR_NCF_Pos)              /*!< 0x00000004 */
#define USART_ICR_NCF                 USART_ICR_NCF_Msk                        /*!< Noise detected Clear Flag           */
#define USART_ICR_ORECF_Pos           (3U)                                     
#define USART_ICR_ORECF_Msk           (0x1UL << USART_ICR_ORECF_Pos)            /*!< 0x00000008 */
#define USART_ICR_ORECF               USART_ICR_ORECF_Msk                      /*!< OverRun Error Clear Flag            */
#define USART_ICR_IDLECF_Pos          (4U)                                     
#define USART_ICR_IDLECF_Msk          (0x1UL << USART_ICR_IDLECF_Pos)           /*!< 0x00000010 */
#define USART_ICR_IDLECF              USART_ICR_IDLECF_Msk                     /*!< IDLE line detected Clear Flag       */
#define USART_ICR_TCCF_Pos            (6U)                                     
#define USART_ICR_TCCF_Msk            (0x1UL << USART_ICR_TCCF_Pos)             /*!< 0x00000040 */
#define USART_ICR_TCCF                USART_ICR_TCCF_Msk                       /*!< Transmission Complete Clear Flag    */
#define USART_ICR_TCBGTCF_Pos         (7U)                                    
#define USART_ICR_TCBGTCF_Msk         (0x1UL << USART_ICR_TCBGTCF_Pos)          /*!< 0x00000080 */
#define USART_ICR_TCBGTCF             USART_ICR_TCBGTCF_Msk                    /*!< Transmission complete before guard time clear flag */
#define USART_ICR_LBDCF_Pos           (8U)                                     
#define USART_ICR_LBDCF_Msk           (0x1UL << USART_ICR_LBDCF_Pos)            /*!< 0x00000100 */
#define USART_ICR_LBDCF               USART_ICR_LBDCF_Msk                      /*!< LIN Break Detection Clear Flag      */
#define USART_ICR_CTSCF_Pos           (9U)                                     
#define USART_ICR_CTSCF_Msk           (0x1UL << USART_ICR_CTSCF_Pos)            /*!< 0x00000200 */
#define USART_ICR_CTSCF               USART_ICR_CTSCF_Msk                      /*!< CTS Interrupt Clear Flag            */
#define USART_ICR_RTOCF_Pos           (11U)                                    
#define USART_ICR_RTOCF_Msk           (0x1UL << USART_ICR_RTOCF_Pos)            /*!< 0x00000800 */
#define USART_ICR_RTOCF               USART_ICR_RTOCF_Msk                      /*!< Receiver Time Out Clear Flag        */
#define USART_ICR_EOBCF_Pos           (12U)                                    
#define USART_ICR_EOBCF_Msk           (0x1UL << USART_ICR_EOBCF_Pos)            /*!< 0x00001000 */
#define USART_ICR_EOBCF               USART_ICR_EOBCF_Msk                      /*!< End Of Block Clear Flag             */
#define USART_ICR_CMCF_Pos            (17U)                                    
#define USART_ICR_CMCF_Msk            (0x1UL << USART_ICR_CMCF_Pos)             /*!< 0x00020000 */
#define USART_ICR_CMCF                USART_ICR_CMCF_Msk                       /*!< Character Match Clear Flag          */

/*******************  Bit definition for USART_RDR register  ******************/
#define USART_RDR_RDR_Pos             (0U)                                     
#define USART_RDR_RDR_Msk             (0x1FFUL << USART_RDR_RDR_Pos)            /*!< 0x000001FF */
#define USART_RDR_RDR                 USART_RDR_RDR_Msk                        /*!< RDR[8:0] bits (Receive Data value) */

/*******************  Bit definition for USART_TDR register  ******************/
#define USART_TDR_TDR_Pos             (0U)                                     
#define USART_TDR_TDR_Msk             (0x1FFUL << USART_TDR_TDR_Pos)            /*!< 0x000001FF */
#define USART_TDR_TDR                 USART_TDR_TDR_Msk                        /*!< TDR[8:0] bits (Transmit Data value) */

typedef struct
{
  __IO uint32_t CR1;    /*!< USART Control register 1,                 Address offset: 0x00 */
  __IO uint32_t CR2;    /*!< USART Control register 2,                 Address offset: 0x04 */
  __IO uint32_t CR3;    /*!< USART Control register 3,                 Address offset: 0x08 */
  __IO uint32_t BRR;    /*!< USART Baud rate register,                 Address offset: 0x0C */
  __IO uint32_t GTPR;   /*!< USART Guard time and prescaler register,  Address offset: 0x10 */
  __IO uint32_t RTOR;   /*!< USART Receiver Time Out register,         Address offset: 0x14 */
  __IO uint32_t RQR;    /*!< USART Request register,                   Address offset: 0x18 */
  __IO uint32_t ISR;    /*!< USART Interrupt and status register,      Address offset: 0x1C */
  __IO uint32_t ICR;    /*!< USART Interrupt flag Clear register,      Address offset: 0x20 */
  __IO uint32_t RDR;    /*!< USART Receive Data register,              Address offset: 0x24 */
  __IO uint32_t TDR;    /*!< USART Transmit Data register,             Address offset: 0x28 */
} USART_TypeDef;

#define USART0              ((USART_TypeDef *) USART0_BASE)
#define USART1              ((USART_TypeDef *) USART1_BASE)

#endif /* GD32F3X0_USART_H */
