/**
 * \file
 *
 * \brief Peripheral I/O description for SAMG54G19
 *
 * Copyright (c) 2017 Atmel Corporation, a wholly owned subsidiary of Microchip Technology Inc.
 *
 * \license_start
 *
 * \page License
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * \license_stop
 *
 */

#ifndef _SAMG54G19_PIO_H_
#define _SAMG54G19_PIO_H_

/* ========== Peripheral I/O pin numbers ========== */
#define PIN_PA0                     (  0)  /**< Pin Number for PA0 */
#define PIN_PA1                     (  1)  /**< Pin Number for PA1 */
#define PIN_PA2                     (  2)  /**< Pin Number for PA2 */
#define PIN_PA3                     (  3)  /**< Pin Number for PA3 */
#define PIN_PA4                     (  4)  /**< Pin Number for PA4 */
#define PIN_PA5                     (  5)  /**< Pin Number for PA5 */
#define PIN_PA6                     (  6)  /**< Pin Number for PA6 */
#define PIN_PA7                     (  7)  /**< Pin Number for PA7 */
#define PIN_PA8                     (  8)  /**< Pin Number for PA8 */
#define PIN_PA9                     (  9)  /**< Pin Number for PA9 */
#define PIN_PA10                    ( 10)  /**< Pin Number for PA10 */
#define PIN_PA11                    ( 11)  /**< Pin Number for PA11 */
#define PIN_PA12                    ( 12)  /**< Pin Number for PA12 */
#define PIN_PA13                    ( 13)  /**< Pin Number for PA13 */
#define PIN_PA14                    ( 14)  /**< Pin Number for PA14 */
#define PIN_PA15                    ( 15)  /**< Pin Number for PA15 */
#define PIN_PA16                    ( 16)  /**< Pin Number for PA16 */
#define PIN_PA17                    ( 17)  /**< Pin Number for PA17 */
#define PIN_PA18                    ( 18)  /**< Pin Number for PA18 */
#define PIN_PA19                    ( 19)  /**< Pin Number for PA19 */
#define PIN_PA20                    ( 20)  /**< Pin Number for PA20 */
#define PIN_PA21                    ( 21)  /**< Pin Number for PA21 */
#define PIN_PA22                    ( 22)  /**< Pin Number for PA22 */
#define PIN_PA23                    ( 23)  /**< Pin Number for PA23 */
#define PIN_PA24                    ( 24)  /**< Pin Number for PA24 */
#define PIN_PB0                     ( 32)  /**< Pin Number for PB0 */
#define PIN_PB1                     ( 33)  /**< Pin Number for PB1 */
#define PIN_PB2                     ( 34)  /**< Pin Number for PB2 */
#define PIN_PB3                     ( 35)  /**< Pin Number for PB3 */
#define PIN_PB4                     ( 36)  /**< Pin Number for PB4 */
#define PIN_PB5                     ( 37)  /**< Pin Number for PB5 */
#define PIN_PB6                     ( 38)  /**< Pin Number for PB6 */
#define PIN_PB7                     ( 39)  /**< Pin Number for PB7 */
#define PIN_PB8                     ( 40)  /**< Pin Number for PB8 */
#define PIN_PB9                     ( 41)  /**< Pin Number for PB9 */
#define PIN_PB10                    ( 42)  /**< Pin Number for PB10 */
#define PIN_PB11                    ( 43)  /**< Pin Number for PB11 */
#define PIN_PB12                    ( 44)  /**< Pin Number for PB12 */


/* ========== Peripheral I/O masks ========== */
#define PIO_PA0                     (_U(1) << 0) /**< PIO Mask for PA0 */
#define PIO_PA1                     (_U(1) << 1) /**< PIO Mask for PA1 */
#define PIO_PA2                     (_U(1) << 2) /**< PIO Mask for PA2 */
#define PIO_PA3                     (_U(1) << 3) /**< PIO Mask for PA3 */
#define PIO_PA4                     (_U(1) << 4) /**< PIO Mask for PA4 */
#define PIO_PA5                     (_U(1) << 5) /**< PIO Mask for PA5 */
#define PIO_PA6                     (_U(1) << 6) /**< PIO Mask for PA6 */
#define PIO_PA7                     (_U(1) << 7) /**< PIO Mask for PA7 */
#define PIO_PA8                     (_U(1) << 8) /**< PIO Mask for PA8 */
#define PIO_PA9                     (_U(1) << 9) /**< PIO Mask for PA9 */
#define PIO_PA10                    (_U(1) << 10) /**< PIO Mask for PA10 */
#define PIO_PA11                    (_U(1) << 11) /**< PIO Mask for PA11 */
#define PIO_PA12                    (_U(1) << 12) /**< PIO Mask for PA12 */
#define PIO_PA13                    (_U(1) << 13) /**< PIO Mask for PA13 */
#define PIO_PA14                    (_U(1) << 14) /**< PIO Mask for PA14 */
#define PIO_PA15                    (_U(1) << 15) /**< PIO Mask for PA15 */
#define PIO_PA16                    (_U(1) << 16) /**< PIO Mask for PA16 */
#define PIO_PA17                    (_U(1) << 17) /**< PIO Mask for PA17 */
#define PIO_PA18                    (_U(1) << 18) /**< PIO Mask for PA18 */
#define PIO_PA19                    (_U(1) << 19) /**< PIO Mask for PA19 */
#define PIO_PA20                    (_U(1) << 20) /**< PIO Mask for PA20 */
#define PIO_PA21                    (_U(1) << 21) /**< PIO Mask for PA21 */
#define PIO_PA22                    (_U(1) << 22) /**< PIO Mask for PA22 */
#define PIO_PA23                    (_U(1) << 23) /**< PIO Mask for PA23 */
#define PIO_PA24                    (_U(1) << 24) /**< PIO Mask for PA24 */
#define PIO_PB0                     (_U(1) << 0) /**< PIO Mask for PB0 */
#define PIO_PB1                     (_U(1) << 1) /**< PIO Mask for PB1 */
#define PIO_PB2                     (_U(1) << 2) /**< PIO Mask for PB2 */
#define PIO_PB3                     (_U(1) << 3) /**< PIO Mask for PB3 */
#define PIO_PB4                     (_U(1) << 4) /**< PIO Mask for PB4 */
#define PIO_PB5                     (_U(1) << 5) /**< PIO Mask for PB5 */
#define PIO_PB6                     (_U(1) << 6) /**< PIO Mask for PB6 */
#define PIO_PB7                     (_U(1) << 7) /**< PIO Mask for PB7 */
#define PIO_PB8                     (_U(1) << 8) /**< PIO Mask for PB8 */
#define PIO_PB9                     (_U(1) << 9) /**< PIO Mask for PB9 */
#define PIO_PB10                    (_U(1) << 10) /**< PIO Mask for PB10 */
#define PIO_PB11                    (_U(1) << 11) /**< PIO Mask for PB11 */
#define PIO_PB12                    (_U(1) << 12) /**< PIO Mask for PB12 */


/* ========== Peripheral I/O indexes ========== */
#define PIO_PA0_IDX                 (  0)  /**< PIO Index Number for PA0 */
#define PIO_PA1_IDX                 (  1)  /**< PIO Index Number for PA1 */
#define PIO_PA2_IDX                 (  2)  /**< PIO Index Number for PA2 */
#define PIO_PA3_IDX                 (  3)  /**< PIO Index Number for PA3 */
#define PIO_PA4_IDX                 (  4)  /**< PIO Index Number for PA4 */
#define PIO_PA5_IDX                 (  5)  /**< PIO Index Number for PA5 */
#define PIO_PA6_IDX                 (  6)  /**< PIO Index Number for PA6 */
#define PIO_PA7_IDX                 (  7)  /**< PIO Index Number for PA7 */
#define PIO_PA8_IDX                 (  8)  /**< PIO Index Number for PA8 */
#define PIO_PA9_IDX                 (  9)  /**< PIO Index Number for PA9 */
#define PIO_PA10_IDX                ( 10)  /**< PIO Index Number for PA10 */
#define PIO_PA11_IDX                ( 11)  /**< PIO Index Number for PA11 */
#define PIO_PA12_IDX                ( 12)  /**< PIO Index Number for PA12 */
#define PIO_PA13_IDX                ( 13)  /**< PIO Index Number for PA13 */
#define PIO_PA14_IDX                ( 14)  /**< PIO Index Number for PA14 */
#define PIO_PA15_IDX                ( 15)  /**< PIO Index Number for PA15 */
#define PIO_PA16_IDX                ( 16)  /**< PIO Index Number for PA16 */
#define PIO_PA17_IDX                ( 17)  /**< PIO Index Number for PA17 */
#define PIO_PA18_IDX                ( 18)  /**< PIO Index Number for PA18 */
#define PIO_PA19_IDX                ( 19)  /**< PIO Index Number for PA19 */
#define PIO_PA20_IDX                ( 20)  /**< PIO Index Number for PA20 */
#define PIO_PA21_IDX                ( 21)  /**< PIO Index Number for PA21 */
#define PIO_PA22_IDX                ( 22)  /**< PIO Index Number for PA22 */
#define PIO_PA23_IDX                ( 23)  /**< PIO Index Number for PA23 */
#define PIO_PA24_IDX                ( 24)  /**< PIO Index Number for PA24 */
#define PIO_PB0_IDX                 ( 32)  /**< PIO Index Number for PB0 */
#define PIO_PB1_IDX                 ( 33)  /**< PIO Index Number for PB1 */
#define PIO_PB2_IDX                 ( 34)  /**< PIO Index Number for PB2 */
#define PIO_PB3_IDX                 ( 35)  /**< PIO Index Number for PB3 */
#define PIO_PB4_IDX                 ( 36)  /**< PIO Index Number for PB4 */
#define PIO_PB5_IDX                 ( 37)  /**< PIO Index Number for PB5 */
#define PIO_PB6_IDX                 ( 38)  /**< PIO Index Number for PB6 */
#define PIO_PB7_IDX                 ( 39)  /**< PIO Index Number for PB7 */
#define PIO_PB8_IDX                 ( 40)  /**< PIO Index Number for PB8 */
#define PIO_PB9_IDX                 ( 41)  /**< PIO Index Number for PB9 */
#define PIO_PB10_IDX                ( 42)  /**< PIO Index Number for PB10 */
#define PIO_PB11_IDX                ( 43)  /**< PIO Index Number for PB11 */
#define PIO_PB12_IDX                ( 44)  /**< PIO Index Number for PB12 */

/* ========== PIO definition for ADC peripheral ========== */
#define PIN_PA8B_ADC_ADTRG                         _L(8)        /**< ADC signal: ADTRG on PA8 mux B*/
#define MUX_PA8B_ADC_ADTRG                         _L(1)       
#define PINMUX_PA8B_ADC_ADTRG                      ((PIN_PA8B_ADC_ADTRG << 16) | MUX_PA8B_ADC_ADTRG)
#define PIO_PA8B_ADC_ADTRG                         (_UL(1) << 8)

/* ========== PIO definition for I2SC0 peripheral ========== */
#define PIN_PA0A_I2SC0_I2SC0_CK                    _L(0)        /**< I2SC0 signal: I2SC0_CK on PA0 mux A*/
#define MUX_PA0A_I2SC0_I2SC0_CK                    _L(0)       
#define PINMUX_PA0A_I2SC0_I2SC0_CK                 ((PIN_PA0A_I2SC0_I2SC0_CK << 16) | MUX_PA0A_I2SC0_I2SC0_CK)
#define PIO_PA0A_I2SC0_I2SC0_CK                    (_UL(1) << 0)

#define PIN_PA2B_I2SC0_I2SC0_DI0                   _L(2)        /**< I2SC0 signal: I2SC0_DI0 on PA2 mux B*/
#define MUX_PA2B_I2SC0_I2SC0_DI0                   _L(1)       
#define PINMUX_PA2B_I2SC0_I2SC0_DI0                ((PIN_PA2B_I2SC0_I2SC0_DI0 << 16) | MUX_PA2B_I2SC0_I2SC0_DI0)
#define PIO_PA2B_I2SC0_I2SC0_DI0                   (_UL(1) << 2)

#define PIN_PA3B_I2SC0_I2SC0_DO0                   _L(3)        /**< I2SC0 signal: I2SC0_DO0 on PA3 mux B*/
#define MUX_PA3B_I2SC0_I2SC0_DO0                   _L(1)       
#define PINMUX_PA3B_I2SC0_I2SC0_DO0                ((PIN_PA3B_I2SC0_I2SC0_DO0 << 16) | MUX_PA3B_I2SC0_I2SC0_DO0)
#define PIO_PA3B_I2SC0_I2SC0_DO0                   (_UL(1) << 3)

#define PIN_PA17A_I2SC0_I2SC0_DO0                  _L(17)       /**< I2SC0 signal: I2SC0_DO0 on PA17 mux A*/
#define MUX_PA17A_I2SC0_I2SC0_DO0                  _L(0)       
#define PINMUX_PA17A_I2SC0_I2SC0_DO0               ((PIN_PA17A_I2SC0_I2SC0_DO0 << 16) | MUX_PA17A_I2SC0_I2SC0_DO0)
#define PIO_PA17A_I2SC0_I2SC0_DO0                  (_UL(1) << 17)

#define PIN_PA4B_I2SC0_I2SC0_MCK                   _L(4)        /**< I2SC0 signal: I2SC0_MCK on PA4 mux B*/
#define MUX_PA4B_I2SC0_I2SC0_MCK                   _L(1)       
#define PINMUX_PA4B_I2SC0_I2SC0_MCK                ((PIN_PA4B_I2SC0_I2SC0_MCK << 16) | MUX_PA4B_I2SC0_I2SC0_MCK)
#define PIO_PA4B_I2SC0_I2SC0_MCK                   (_UL(1) << 4)

#define PIN_PA18A_I2SC0_I2SC0_MCK                  _L(18)       /**< I2SC0 signal: I2SC0_MCK on PA18 mux A*/
#define MUX_PA18A_I2SC0_I2SC0_MCK                  _L(0)       
#define PINMUX_PA18A_I2SC0_I2SC0_MCK               ((PIN_PA18A_I2SC0_I2SC0_MCK << 16) | MUX_PA18A_I2SC0_I2SC0_MCK)
#define PIO_PA18A_I2SC0_I2SC0_MCK                  (_UL(1) << 18)

#define PIN_PA1A_I2SC0_I2SC0_WS                    _L(1)        /**< I2SC0 signal: I2SC0_WS on PA1 mux A*/
#define MUX_PA1A_I2SC0_I2SC0_WS                    _L(0)       
#define PINMUX_PA1A_I2SC0_I2SC0_WS                 ((PIN_PA1A_I2SC0_I2SC0_WS << 16) | MUX_PA1A_I2SC0_I2SC0_WS)
#define PIO_PA1A_I2SC0_I2SC0_WS                    (_UL(1) << 1)

/* ========== PIO definition for I2SC1 peripheral ========== */
#define PIN_PA19B_I2SC1_I2SC1_CK                   _L(19)       /**< I2SC1 signal: I2SC1_CK on PA19 mux B*/
#define MUX_PA19B_I2SC1_I2SC1_CK                   _L(1)       
#define PINMUX_PA19B_I2SC1_I2SC1_CK                ((PIN_PA19B_I2SC1_I2SC1_CK << 16) | MUX_PA19B_I2SC1_I2SC1_CK)
#define PIO_PA19B_I2SC1_I2SC1_CK                   (_UL(1) << 19)

#define PIN_PA22B_I2SC1_I2SC1_DI0                  _L(22)       /**< I2SC1 signal: I2SC1_DI0 on PA22 mux B*/
#define MUX_PA22B_I2SC1_I2SC1_DI0                  _L(1)       
#define PINMUX_PA22B_I2SC1_I2SC1_DI0               ((PIN_PA22B_I2SC1_I2SC1_DI0 << 16) | MUX_PA22B_I2SC1_I2SC1_DI0)
#define PIO_PA22B_I2SC1_I2SC1_DI0                  (_UL(1) << 22)

#define PIN_PA23A_I2SC1_I2SC1_DO0                  _L(23)       /**< I2SC1 signal: I2SC1_DO0 on PA23 mux A*/
#define MUX_PA23A_I2SC1_I2SC1_DO0                  _L(0)       
#define PINMUX_PA23A_I2SC1_I2SC1_DO0               ((PIN_PA23A_I2SC1_I2SC1_DO0 << 16) | MUX_PA23A_I2SC1_I2SC1_DO0)
#define PIO_PA23A_I2SC1_I2SC1_DO0                  (_UL(1) << 23)

#define PIN_PA24A_I2SC1_I2SC1_MCK                  _L(24)       /**< I2SC1 signal: I2SC1_MCK on PA24 mux A*/
#define MUX_PA24A_I2SC1_I2SC1_MCK                  _L(0)       
#define PINMUX_PA24A_I2SC1_I2SC1_MCK               ((PIN_PA24A_I2SC1_I2SC1_MCK << 16) | MUX_PA24A_I2SC1_I2SC1_MCK)
#define PIO_PA24A_I2SC1_I2SC1_MCK                  (_UL(1) << 24)

#define PIN_PA20B_I2SC1_I2SC1_WS                   _L(20)       /**< I2SC1 signal: I2SC1_WS on PA20 mux B*/
#define MUX_PA20B_I2SC1_I2SC1_WS                   _L(1)       
#define PINMUX_PA20B_I2SC1_I2SC1_WS                ((PIN_PA20B_I2SC1_I2SC1_WS << 16) | MUX_PA20B_I2SC1_I2SC1_WS)
#define PIO_PA20B_I2SC1_I2SC1_WS                   (_UL(1) << 20)

/* ========== PIO definition for PDMIC0 peripheral ========== */
#define PIN_PA10B_PDMIC0_PDMIC0_CLK                _L(10)       /**< PDMIC0 signal: PDMIC0_CLK on PA10 mux B*/
#define MUX_PA10B_PDMIC0_PDMIC0_CLK                _L(1)       
#define PINMUX_PA10B_PDMIC0_PDMIC0_CLK             ((PIN_PA10B_PDMIC0_PDMIC0_CLK << 16) | MUX_PA10B_PDMIC0_PDMIC0_CLK)
#define PIO_PA10B_PDMIC0_PDMIC0_CLK                (_UL(1) << 10)

#define PIN_PA9B_PDMIC0_PDMIC0_DAT                 _L(9)        /**< PDMIC0 signal: PDMIC0_DAT on PA9 mux B*/
#define MUX_PA9B_PDMIC0_PDMIC0_DAT                 _L(1)       
#define PINMUX_PA9B_PDMIC0_PDMIC0_DAT              ((PIN_PA9B_PDMIC0_PDMIC0_DAT << 16) | MUX_PA9B_PDMIC0_PDMIC0_DAT)
#define PIO_PA9B_PDMIC0_PDMIC0_DAT                 (_UL(1) << 9)

/* ========== PIO definition for PMC peripheral ========== */
#define PIN_PA6B_PMC_PCK0                          _L(6)        /**< PMC signal: PCK0 on PA6 mux B*/
#define MUX_PA6B_PMC_PCK0                          _L(1)       
#define PINMUX_PA6B_PMC_PCK0                       ((PIN_PA6B_PMC_PCK0 << 16) | MUX_PA6B_PMC_PCK0)
#define PIO_PA6B_PMC_PCK0                          (_UL(1) << 6)

#define PIN_PA17B_PMC_PCK1                         _L(17)       /**< PMC signal: PCK1 on PA17 mux B*/
#define MUX_PA17B_PMC_PCK1                         _L(1)       
#define PINMUX_PA17B_PMC_PCK1                      ((PIN_PA17B_PMC_PCK1 << 16) | MUX_PA17B_PMC_PCK1)
#define PIO_PA17B_PMC_PCK1                         (_UL(1) << 17)

#define PIN_PA21B_PMC_PCK1                         _L(21)       /**< PMC signal: PCK1 on PA21 mux B*/
#define MUX_PA21B_PMC_PCK1                         _L(1)       
#define PINMUX_PA21B_PMC_PCK1                      ((PIN_PA21B_PMC_PCK1 << 16) | MUX_PA21B_PMC_PCK1)
#define PIO_PA21B_PMC_PCK1                         (_UL(1) << 21)

#define PIN_PA18B_PMC_PCK2                         _L(18)       /**< PMC signal: PCK2 on PA18 mux B*/
#define MUX_PA18B_PMC_PCK2                         _L(1)       
#define PINMUX_PA18B_PMC_PCK2                      ((PIN_PA18B_PMC_PCK2 << 16) | MUX_PA18B_PMC_PCK2)
#define PIO_PA18B_PMC_PCK2                         (_UL(1) << 18)

#define PIN_PB3B_PMC_PCK2                          _L(35)       /**< PMC signal: PCK2 on PB3 mux B*/
#define MUX_PB3B_PMC_PCK2                          _L(1)       
#define PINMUX_PB3B_PMC_PCK2                       ((PIN_PB3B_PMC_PCK2 << 16) | MUX_PB3B_PMC_PCK2)
#define PIO_PB3B_PMC_PCK2                          (_UL(1) << 3)

/* ========== PIO definition for SPI peripheral ========== */
#define PIN_PA12A_SPI_MISO                         _L(12)       /**< SPI signal: MISO on PA12 mux A*/
#define MUX_PA12A_SPI_MISO                         _L(0)       
#define PINMUX_PA12A_SPI_MISO                      ((PIN_PA12A_SPI_MISO << 16) | MUX_PA12A_SPI_MISO)
#define PIO_PA12A_SPI_MISO                         (_UL(1) << 12)

#define PIN_PA13A_SPI_MOSI                         _L(13)       /**< SPI signal: MOSI on PA13 mux A*/
#define MUX_PA13A_SPI_MOSI                         _L(0)       
#define PINMUX_PA13A_SPI_MOSI                      ((PIN_PA13A_SPI_MOSI << 16) | MUX_PA13A_SPI_MOSI)
#define PIO_PA13A_SPI_MOSI                         (_UL(1) << 13)

#define PIN_PA11A_SPI_NPCS0                        _L(11)       /**< SPI signal: NPCS0 on PA11 mux A*/
#define MUX_PA11A_SPI_NPCS0                        _L(0)       
#define PINMUX_PA11A_SPI_NPCS0                     ((PIN_PA11A_SPI_NPCS0 << 16) | MUX_PA11A_SPI_NPCS0)
#define PIO_PA11A_SPI_NPCS0                        (_UL(1) << 11)

#define PIN_PA5B_SPI_NPCS1                         _L(5)        /**< SPI signal: NPCS1 on PA5 mux B*/
#define MUX_PA5B_SPI_NPCS1                         _L(1)       
#define PINMUX_PA5B_SPI_NPCS1                      ((PIN_PA5B_SPI_NPCS1 << 16) | MUX_PA5B_SPI_NPCS1)
#define PIO_PA5B_SPI_NPCS1                         (_UL(1) << 5)

#define PIN_PB2B_SPI_NPCS1                         _L(34)       /**< SPI signal: NPCS1 on PB2 mux B*/
#define MUX_PB2B_SPI_NPCS1                         _L(1)       
#define PINMUX_PB2B_SPI_NPCS1                      ((PIN_PB2B_SPI_NPCS1 << 16) | MUX_PB2B_SPI_NPCS1)
#define PIO_PB2B_SPI_NPCS1                         (_UL(1) << 2)

#define PIN_PA14A_SPI_SPCK                         _L(14)       /**< SPI signal: SPCK on PA14 mux A*/
#define MUX_PA14A_SPI_SPCK                         _L(0)       
#define PINMUX_PA14A_SPI_SPCK                      ((PIN_PA14A_SPI_SPCK << 16) | MUX_PA14A_SPI_SPCK)
#define PIO_PA14A_SPI_SPCK                         (_UL(1) << 14)

/* ========== PIO definition for TC0 peripheral ========== */
#define PIN_PA2A_TC0_TCLK0                         _L(2)        /**< TC0 signal: TCLK0 on PA2 mux A*/
#define MUX_PA2A_TC0_TCLK0                         _L(0)       
#define PINMUX_PA2A_TC0_TCLK0                      ((PIN_PA2A_TC0_TCLK0 << 16) | MUX_PA2A_TC0_TCLK0)
#define PIO_PA2A_TC0_TCLK0                         (_UL(1) << 2)

#define PIN_PA19A_TC0_TCLK1                        _L(19)       /**< TC0 signal: TCLK1 on PA19 mux A*/
#define MUX_PA19A_TC0_TCLK1                        _L(0)       
#define PINMUX_PA19A_TC0_TCLK1                     ((PIN_PA19A_TC0_TCLK1 << 16) | MUX_PA19A_TC0_TCLK1)
#define PIO_PA19A_TC0_TCLK1                        (_UL(1) << 19)

#define PIN_PA20A_TC0_TCLK2                        _L(20)       /**< TC0 signal: TCLK2 on PA20 mux A*/
#define MUX_PA20A_TC0_TCLK2                        _L(0)       
#define PINMUX_PA20A_TC0_TCLK2                     ((PIN_PA20A_TC0_TCLK2 << 16) | MUX_PA20A_TC0_TCLK2)
#define PIO_PA20A_TC0_TCLK2                        (_UL(1) << 20)

#define PIN_PA0B_TC0_TIOA0                         _L(0)        /**< TC0 signal: TIOA0 on PA0 mux B*/
#define MUX_PA0B_TC0_TIOA0                         _L(1)       
#define PINMUX_PA0B_TC0_TIOA0                      ((PIN_PA0B_TC0_TIOA0 << 16) | MUX_PA0B_TC0_TIOA0)
#define PIO_PA0B_TC0_TIOA0                         (_UL(1) << 0)

#define PIN_PA23B_TC0_TIOA1                        _L(23)       /**< TC0 signal: TIOA1 on PA23 mux B*/
#define MUX_PA23B_TC0_TIOA1                        _L(1)       
#define PINMUX_PA23B_TC0_TIOA1                     ((PIN_PA23B_TC0_TIOA1 << 16) | MUX_PA23B_TC0_TIOA1)
#define PIO_PA23B_TC0_TIOA1                        (_UL(1) << 23)

#define PIN_PA21A_TC0_TIOA2                        _L(21)       /**< TC0 signal: TIOA2 on PA21 mux A*/
#define MUX_PA21A_TC0_TIOA2                        _L(0)       
#define PINMUX_PA21A_TC0_TIOA2                     ((PIN_PA21A_TC0_TIOA2 << 16) | MUX_PA21A_TC0_TIOA2)
#define PIO_PA21A_TC0_TIOA2                        (_UL(1) << 21)

#define PIN_PA1B_TC0_TIOB0                         _L(1)        /**< TC0 signal: TIOB0 on PA1 mux B*/
#define MUX_PA1B_TC0_TIOB0                         _L(1)       
#define PINMUX_PA1B_TC0_TIOB0                      ((PIN_PA1B_TC0_TIOB0 << 16) | MUX_PA1B_TC0_TIOB0)
#define PIO_PA1B_TC0_TIOB0                         (_UL(1) << 1)

#define PIN_PA16B_TC0_TIOB1                        _L(16)       /**< TC0 signal: TIOB1 on PA16 mux B*/
#define MUX_PA16B_TC0_TIOB1                        _L(1)       
#define PINMUX_PA16B_TC0_TIOB1                     ((PIN_PA16B_TC0_TIOB1 << 16) | MUX_PA16B_TC0_TIOB1)
#define PIO_PA16B_TC0_TIOB1                        (_UL(1) << 16)

#define PIN_PA22A_TC0_TIOB2                        _L(22)       /**< TC0 signal: TIOB2 on PA22 mux A*/
#define MUX_PA22A_TC0_TIOB2                        _L(0)       
#define PINMUX_PA22A_TC0_TIOB2                     ((PIN_PA22A_TC0_TIOB2 << 16) | MUX_PA22A_TC0_TIOB2)
#define PIO_PA22A_TC0_TIOB2                        (_UL(1) << 22)

/* ========== PIO definition for TWI1 peripheral ========== */
#define PIN_PB9A_TWI1_TWCK1                        _L(41)       /**< TWI1 signal: TWCK1 on PB9 mux A*/
#define MUX_PB9A_TWI1_TWCK1                        _L(0)       
#define PINMUX_PB9A_TWI1_TWCK1                     ((PIN_PB9A_TWI1_TWCK1 << 16) | MUX_PB9A_TWI1_TWCK1)
#define PIO_PB9A_TWI1_TWCK1                        (_UL(1) << 9)

#define PIN_PB11A_TWI1_TWCK1                       _L(43)       /**< TWI1 signal: TWCK1 on PB11 mux A*/
#define MUX_PB11A_TWI1_TWCK1                       _L(0)       
#define PINMUX_PB11A_TWI1_TWCK1                    ((PIN_PB11A_TWI1_TWCK1 << 16) | MUX_PB11A_TWI1_TWCK1)
#define PIO_PB11A_TWI1_TWCK1                       (_UL(1) << 11)

#define PIN_PB8A_TWI1_TWD1                         _L(40)       /**< TWI1 signal: TWD1 on PB8 mux A*/
#define MUX_PB8A_TWI1_TWD1                         _L(0)       
#define PINMUX_PB8A_TWI1_TWD1                      ((PIN_PB8A_TWI1_TWD1 << 16) | MUX_PB8A_TWI1_TWD1)
#define PIO_PB8A_TWI1_TWD1                         (_UL(1) << 8)

#define PIN_PB10A_TWI1_TWD1                        _L(42)       /**< TWI1 signal: TWD1 on PB10 mux A*/
#define MUX_PB10A_TWI1_TWD1                        _L(0)       
#define PINMUX_PB10A_TWI1_TWD1                     ((PIN_PB10A_TWI1_TWD1 << 16) | MUX_PB10A_TWI1_TWD1)
#define PIO_PB10A_TWI1_TWD1                        (_UL(1) << 10)

/* ========== PIO definition for TWI2 peripheral ========== */
#define PIN_PB1B_TWI2_TWCK2                        _L(33)       /**< TWI2 signal: TWCK2 on PB1 mux B*/
#define MUX_PB1B_TWI2_TWCK2                        _L(1)       
#define PINMUX_PB1B_TWI2_TWCK2                     ((PIN_PB1B_TWI2_TWCK2 << 16) | MUX_PB1B_TWI2_TWCK2)
#define PIO_PB1B_TWI2_TWCK2                        (_UL(1) << 1)

#define PIN_PB11B_TWI2_TWCK2                       _L(43)       /**< TWI2 signal: TWCK2 on PB11 mux B*/
#define MUX_PB11B_TWI2_TWCK2                       _L(1)       
#define PINMUX_PB11B_TWI2_TWCK2                    ((PIN_PB11B_TWI2_TWCK2 << 16) | MUX_PB11B_TWI2_TWCK2)
#define PIO_PB11B_TWI2_TWCK2                       (_UL(1) << 11)

#define PIN_PB0B_TWI2_TWD2                         _L(32)       /**< TWI2 signal: TWD2 on PB0 mux B*/
#define MUX_PB0B_TWI2_TWD2                         _L(1)       
#define PINMUX_PB0B_TWI2_TWD2                      ((PIN_PB0B_TWI2_TWD2 << 16) | MUX_PB0B_TWI2_TWD2)
#define PIO_PB0B_TWI2_TWD2                         (_UL(1) << 0)

#define PIN_PB10B_TWI2_TWD2                        _L(42)       /**< TWI2 signal: TWD2 on PB10 mux B*/
#define MUX_PB10B_TWI2_TWD2                        _L(1)       
#define PINMUX_PB10B_TWI2_TWD2                     ((PIN_PB10B_TWI2_TWD2 << 16) | MUX_PB10B_TWI2_TWD2)
#define PIO_PB10B_TWI2_TWD2                        (_UL(1) << 10)

/* ========== PIO definition for TWI0 peripheral ========== */
#define PIN_PA4A_TWI0_TWCK0                        _L(4)        /**< TWI0 signal: TWCK0 on PA4 mux A*/
#define MUX_PA4A_TWI0_TWCK0                        _L(0)       
#define PINMUX_PA4A_TWI0_TWCK0                     ((PIN_PA4A_TWI0_TWCK0 << 16) | MUX_PA4A_TWI0_TWCK0)
#define PIO_PA4A_TWI0_TWCK0                        (_UL(1) << 4)

#define PIN_PA3A_TWI0_TWD0                         _L(3)        /**< TWI0 signal: TWD0 on PA3 mux A*/
#define MUX_PA3A_TWI0_TWD0                         _L(0)       
#define PINMUX_PA3A_TWI0_TWD0                      ((PIN_PA3A_TWI0_TWD0 << 16) | MUX_PA3A_TWI0_TWD0)
#define PIO_PA3A_TWI0_TWD0                         (_UL(1) << 3)

/* ========== PIO definition for UART0 peripheral ========== */
#define PIN_PA9A_UART0_URXD0                       _L(9)        /**< UART0 signal: URXD0 on PA9 mux A*/
#define MUX_PA9A_UART0_URXD0                       _L(0)       
#define PINMUX_PA9A_UART0_URXD0                    ((PIN_PA9A_UART0_URXD0 << 16) | MUX_PA9A_UART0_URXD0)
#define PIO_PA9A_UART0_URXD0                       (_UL(1) << 9)

#define PIN_PA10A_UART0_UTXD0                      _L(10)       /**< UART0 signal: UTXD0 on PA10 mux A*/
#define MUX_PA10A_UART0_UTXD0                      _L(0)       
#define PINMUX_PA10A_UART0_UTXD0                   ((PIN_PA10A_UART0_UTXD0 << 16) | MUX_PA10A_UART0_UTXD0)
#define PIO_PA10A_UART0_UTXD0                      (_UL(1) << 10)

/* ========== PIO definition for UART1 peripheral ========== */
#define PIN_PB2A_UART1_URXD1                       _L(34)       /**< UART1 signal: URXD1 on PB2 mux A*/
#define MUX_PB2A_UART1_URXD1                       _L(0)       
#define PINMUX_PB2A_UART1_URXD1                    ((PIN_PB2A_UART1_URXD1 << 16) | MUX_PB2A_UART1_URXD1)
#define PIO_PB2A_UART1_URXD1                       (_UL(1) << 2)

#define PIN_PB3A_UART1_UTXD1                       _L(35)       /**< UART1 signal: UTXD1 on PB3 mux A*/
#define MUX_PB3A_UART1_UTXD1                       _L(0)       
#define PINMUX_PB3A_UART1_UTXD1                    ((PIN_PB3A_UART1_UTXD1 << 16) | MUX_PB3A_UART1_UTXD1)
#define PIO_PB3A_UART1_UTXD1                       (_UL(1) << 3)

/* ========== PIO definition for USART peripheral ========== */
#define PIN_PA16A_USART_CTS                        _L(16)       /**< USART signal: CTS on PA16 mux A*/
#define MUX_PA16A_USART_CTS                        _L(0)       
#define PINMUX_PA16A_USART_CTS                     ((PIN_PA16A_USART_CTS << 16) | MUX_PA16A_USART_CTS)
#define PIO_PA16A_USART_CTS                        (_UL(1) << 16)

#define PIN_PA15A_USART_RTS                        _L(15)       /**< USART signal: RTS on PA15 mux A*/
#define MUX_PA15A_USART_RTS                        _L(0)       
#define PINMUX_PA15A_USART_RTS                     ((PIN_PA15A_USART_RTS << 16) | MUX_PA15A_USART_RTS)
#define PIO_PA15A_USART_RTS                        (_UL(1) << 15)

#define PIN_PA5A_USART_RXD                         _L(5)        /**< USART signal: RXD on PA5 mux A*/
#define MUX_PA5A_USART_RXD                         _L(0)       
#define PINMUX_PA5A_USART_RXD                      ((PIN_PA5A_USART_RXD << 16) | MUX_PA5A_USART_RXD)
#define PIO_PA5A_USART_RXD                         (_UL(1) << 5)

#define PIN_PA15B_USART_SCK                        _L(15)       /**< USART signal: SCK on PA15 mux B*/
#define MUX_PA15B_USART_SCK                        _L(1)       
#define PINMUX_PA15B_USART_SCK                     ((PIN_PA15B_USART_SCK << 16) | MUX_PA15B_USART_SCK)
#define PIO_PA15B_USART_SCK                        (_UL(1) << 15)

#define PIN_PA6A_USART_TXD                         _L(6)        /**< USART signal: TXD on PA6 mux A*/
#define MUX_PA6A_USART_TXD                         _L(0)       
#define PINMUX_PA6A_USART_TXD                      ((PIN_PA6A_USART_TXD << 16) | MUX_PA6A_USART_TXD)
#define PIO_PA6A_USART_TXD                         (_UL(1) << 6)


#endif /* _SAMG54G19_PIO_H_ */
