/*!
    \file  gd32f3x0_crc.h
    \brief definitions for the CRC
*/

/*
    Copyright (C) 2017 GigaDevice

    2017-06-06, V1.0.0, firmware for GD32F3x0
*/

#ifndef GD32F3X0_CRC_H
#define GD32F3X0_CRC_H

#include "gd32f3x0.h"

/* CRC definitions */
#define CRC                            CRC_BASE

/* registers definitions */
#define CRC_DATA                       REG32((CRC) + 0x00U)            /*!< CRC data register */
#define CRC_FDATA                      REG32((CRC) + 0x04U)            /*!< CRC free data register */
#define CRC_CTL                        REG32((CRC) + 0x08U)            /*!< CRC control register */
#define CRC_IDATA                      REG32((CRC) + 0x10U)            /*!< CRC initialization data register */
#define CRC_POLY                       REG32((CRC) + 0x14U)            /*!< CRC polynomial register */

/* bits definitions */
/* CRC_DATA */
#define CRC_DATA_DATA                  BITS(0,31)                      /*!< CRC data bits */

/* CRC_FDATA */
#define CRC_FDATA_FDATA                BITS(0,7)                       /*!< CRC free data bits */

/* CRC_CTL */
#define CRC_CTL_RST                    BIT(0)                          /*!< CRC reset bit */
#define CRC_CTL_PS                     BITS(3,4)                       /*!< size of polynomial function bits */
#define CRC_CTL_REV_I                  BITS(5,6)                       /*!< input data reverse function bits */
#define CRC_CTL_REV_O                  BIT(7)                          /*!< output data reverse function bit */

/* CRC_INIT */
#define CRC_IDATA_IDATA                BITS(0,31)                      /*!< CRC initialization data bits */

/* CRC_POLY */
#define CRC_POLY_POLY                  BITS(0,31)                      /*!< CRC polynomial value bits */

/* constants definitions */
/* size of polynomial function */
#define CTL_PS(regval)                 (BITS(3, 4) & ((regval) << 3))
#define CRC_CTL_PS_32                  CTL_PS(0)                       /*!< 32-bit polynomial for CRC calculation */
#define CRC_CTL_PS_16                  CTL_PS(1)                       /*!< 16-bit polynomial for CRC calculation */
#define CRC_CTL_PS_8                   CTL_PS(2)                       /*!< 8-bit polynomial for CRC calculation */
#define CRC_CTL_PS_7                   CTL_PS(3)                       /*!< 7-bit polynomial for CRC calculation */

/* input data reverse function */
#define CTL_REV_I(regval)              (BITS(5, 6) & ((regval) << 5))
#define CRC_INPUT_DATA_NOT             CTL_REV_I(0)                    /*!< input data not reverse */
#define CRC_INPUT_DATA_BYTE            CTL_REV_I(1)                    /*!< input data reversed by byte type */
#define CRC_INPUT_DATA_HALFWORD        CTL_REV_I(2)                    /*!< input data reversed by half-word type */
#define CRC_INPUT_DATA_WORD            CTL_REV_I(3)                    /*!< input data reversed by word type */

#endif /* GD32F3X0_CRC_H */
