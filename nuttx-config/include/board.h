/*
 * board.h
 *
 * Pixeagle board definitions (STM32H743VIT6, 16MHz HSE, 480MHz core).
 * Defines GPIOs: PA15 (GPIO_VDD_5V_PERIPH_EN, active high), PE14 (WS2812B),
 * PB1 (PPM), PC8 (PWM OUT 7, TIM8_CH3), etc.
 * Hardware: SPI1 (BMI088: PA4/PB2 CS), SPI4 (ICM-42688-P: PE4 CS),
 * I2C3 (IST8310: 0x0E, BMP388: 0x77), I2C4 (BMP390: 0x77), CAN1 (PD0/PD1), CAN2 (PB12/PB13),
 * USART2 (PD3-PD6). No PX4IO. External SPI3/I2C1 use TXS0108ERGYR for 5V.
 * Note: BMP388 and BMP390 share I2C address 0x77 but are on separate buses (I2C3, I2C4).
 */

#ifndef __BOARDS_ARM_PIXEAGLE_INCLUDE_BOARD_H
#define __BOARDS_ARM_PIXEAGLE_INCLUDE_BOARD_H

/************************************************************************************
 * Included Files
 ************************************************************************************/

#include "board_dma_map.h"

#include <nuttx/config.h>

#ifndef __ASSEMBLY__
# include <stdint.h>
#endif

#include "stm32_rcc.h"
#include "stm32_sdmmc.h"

/************************************************************************************
 * Pre-processor Definitions
 ************************************************************************************/

/* Clocking *************************************************************************/
/* The Pixeagle board provides the following clock sources:
 *
 *   X1: 16 MHz crystal for HSE
 *
 * So we have these clock sources available within the STM32
 *
 *   HSI: 16 MHz RC factory-trimmed
 *   HSE: 16 MHz crystal for HSE
 */

#define STM32_BOARD_XTAL        16000000ul

#define STM32_HSI_FREQUENCY     16000000ul
#define STM32_LSI_FREQUENCY     32000
#define STM32_HSE_FREQUENCY     STM32_BOARD_XTAL
#define STM32_LSE_FREQUENCY     32768

/* Main PLL Configuration.
 *
 * PLL source is HSE = 16,000,000
 *
 * PLL_VCOx = (STM32_HSE_FREQUENCY / PLLM) * PLLN
 * Subject to:
 *
 *     1 <= PLLM <= 63
 *     4 <= PLLN <= 512
 *   150 MHz <= PLL_VCOL <= 420MHz
 *   192 MHz <= PLL_VCOH <= 836MHz
 *
 * SYSCLK  = PLL_VCO / PLLP
 * CPUCLK  = SYSCLK / D1CPRE
 * Subject to
 *
 *   PLLP1   = {2, 4, 6, 8, ..., 128}
 *   PLLP2,3 = {2, 3, 4, ..., 128}
 *   CPUCLK <= 480 MHz
 */

#define STM32_BOARD_USEHSE

#define STM32_PLLCFG_PLLSRC      RCC_PLLCKSELR_PLLSRC_HSE

/* PLL1, wide 4 - 8 MHz input, enable DIVP, DIVQ, DIVR
 *
 *   PLL1_VCO = (16,000,000 / 1) * 60 = 960 MHz
 *   PLL1P = PLL1_VCO/2  = 960 MHz / 2   = 480 MHz
 *   PLL1Q = PLL1_VCO/4  = 960 MHz / 4   = 240 MHz
 *   PLL1R = PLL1_VCO/8  = 960 MHz / 8   = 120 MHz
 */

#define STM32_PLLCFG_PLL1CFG    (RCC_PLLCFGR_PLL1VCOSEL_WIDE | \
				 RCC_PLLCFGR_PLL1RGE_4_8_MHZ | \
				 RCC_PLLCFGR_DIVP1EN | \
				 RCC_PLLCFGR_DIVQ1EN | \
				 RCC_PLLCFGR_DIVR1EN)
#define STM32_PLLCFG_PLL1M       RCC_PLLCKSELR_DIVM1(1)
#define STM32_PLLCFG_PLL1N       RCC_PLL1DIVR_N1(60)
#define STM32_PLLCFG_PLL1P       RCC_PLL1DIVR_P1(2)
#define STM32_PLLCFG_PLL1Q       RCC_PLL1DIVR_Q1(4)
#define STM32_PLLCFG_PLL1R       RCC_PLL1DIVR_R1(8)

#define STM32_VCO1_FREQUENCY     ((STM32_HSE_FREQUENCY / 1) * 60)
#define STM32_PLL1P_FREQUENCY    (STM32_VCO1_FREQUENCY / 2)
#define STM32_PLL1Q_FREQUENCY    (STM32_VCO1_FREQUENCY / 4)
#define STM32_PLL1R_FREQUENCY    (STM32_VCO1_FREQUENCY / 8)

/* PLL2 */

#define STM32_PLLCFG_PLL2CFG     (RCC_PLLCFGR_PLL2VCOSEL_WIDE | \
				  RCC_PLLCFGR_PLL2RGE_4_8_MHZ | \
				  RCC_PLLCFGR_DIVP2EN | \
				  RCC_PLLCFGR_DIVQ2EN | \
				  RCC_PLLCFGR_DIVR2EN)
#define STM32_PLLCFG_PLL2M       RCC_PLLCKSELR_DIVM2(4)
#define STM32_PLLCFG_PLL2N       RCC_PLL2DIVR_N2(48)
#define STM32_PLLCFG_PLL2P       RCC_PLL2DIVR_P2(2)
#define STM32_PLLCFG_PLL2Q       RCC_PLL2DIVR_Q2(2)
#define STM32_PLLCFG_PLL2R       RCC_PLL2DIVR_R2(2)

#define STM32_VCO2_FREQUENCY     ((STM32_HSE_FREQUENCY / 4) * 48)
#define STM32_PLL2P_FREQUENCY    (STM32_VCO2_FREQUENCY / 2)
#define STM32_PLL2Q_FREQUENCY    (STM32_VCO2_FREQUENCY / 2)
#define STM32_PLL2R_FREQUENCY    (STM32_VCO2_FREQUENCY / 2)

/* PLL3 */

#define STM32_PLLCFG_PLL3CFG    (RCC_PLLCFGR_PLL3VCOSEL_WIDE | \
				 RCC_PLLCFGR_PLL3RGE_4_8_MHZ | \
				 RCC_PLLCFGR_DIVQ3EN)
#define STM32_PLLCFG_PLL3M      RCC_PLLCKSELR_DIVM3(4)
#define STM32_PLLCFG_PLL3N      RCC_PLL3DIVR_N3(48)
#define STM32_PLLCFG_PLL3P      RCC_PLL3DIVR_P3(2)
#define STM32_PLLCFG_PLL3Q      RCC_PLL3DIVR_Q3(4)
#define STM32_PLLCFG_PLL3R      RCC_PLL3DIVR_R3(2)

#define STM32_VCO3_FREQUENCY    ((STM32_HSE_FREQUENCY / 4) * 48)
#define STM32_PLL3P_FREQUENCY   (STM32_VCO3_FREQUENCY / 2)
#define STM32_PLL3Q_FREQUENCY   (STM32_VCO3_FREQUENCY / 4)
#define STM32_PLL3R_FREQUENCY   (STM32_VCO3_FREQUENCY / 2)

/* SYSCLK = PLL1P = 480MHz
 * CPUCLK = SYSCLK / 1 = 480 MHz
 */

#define STM32_RCC_D1CFGR_D1CPRE  (RCC_D1CFGR_D1CPRE_SYSCLK)
#define STM32_SYSCLK_FREQUENCY   (STM32_PLL1P_FREQUENCY)
#define STM32_CPUCLK_FREQUENCY   (STM32_SYSCLK_FREQUENCY / 1)

/* Configure Clock Assignments */

/* AHB clock (HCLK) is SYSCLK/2 (240 MHz max)
 * HCLK1 = HCLK2 = HCLK3 = HCLK4 = 240
 */

#define STM32_RCC_D1CFGR_HPRE   RCC_D1CFGR_HPRE_SYSCLKd2        /* HCLK  = SYSCLK / 2 */
#define STM32_ACLK_FREQUENCY    (STM32_CPUCLK_FREQUENCY / 2)    /* ACLK in D1, HCLK3 in D1 */
#define STM32_HCLK_FREQUENCY    (STM32_CPUCLK_FREQUENCY / 2)    /* HCLK in D2, HCLK4 in D3 */
#define STM32_BOARD_HCLK        STM32_HCLK_FREQUENCY            /* same as above, to satisfy compiler */

/* APB1 clock (PCLK1) is HCLK/2 (120 MHz) */

#define STM32_RCC_D2CFGR_D2PPRE1  RCC_D2CFGR_D2PPRE1_HCLKd2       /* PCLK1 = HCLK / 2 */
#define STM32_PCLK1_FREQUENCY     (STM32_HCLK_FREQUENCY/2)

/* APB2 clock (PCLK2) is HCLK/2 (120 MHz) */

#define STM32_RCC_D2CFGR_D2PPRE2  RCC_D2CFGR_D2PPRE2_HCLKd2       /* PCLK2 = HCLK / 2 */
#define STM32_PCLK2_FREQUENCY     (STM32_HCLK_FREQUENCY/2)

/* APB3 clock (PCLK3) is HCLK/2 (120 MHz) */

#define STM32_RCC_D1CFGR_D1PPRE   RCC_D1CFGR_D1PPRE_HCLKd2        /* PCLK3 = HCLK / 2 */
#define STM32_PCLK3_FREQUENCY     (STM32_HCLK_FREQUENCY/2)

/* APB4 clock (PCLK4) is HCLK/2 (120 MHz) */

#define STM32_RCC_D3CFGR_D3PPRE   RCC_D3CFGR_D3PPRE_HCLKd2       /* PCLK4 = HCLK / 2 */
#define STM32_PCLK4_FREQUENCY     (STM32_HCLK_FREQUENCY/2)

/* Timer clock frequencies */

/* Timers driven from APB1 will be twice PCLK1 */

#define STM32_APB1_TIM2_CLKIN   (2*STM32_PCLK1_FREQUENCY)
#define STM32_APB1_TIM3_CLKIN   (2*STM32_PCLK1_FREQUENCY)
#define STM32_APB1_TIM4_CLKIN   (2*STM32_PCLK1_FREQUENCY)
#define STM32_APB1_TIM5_CLKIN   (2*STM32_PCLK1_FREQUENCY)
#define STM32_APB1_TIM6_CLKIN   (2*STM32_PCLK1_FREQUENCY)
#define STM32_APB1_TIM7_CLKIN   (2*STM32_PCLK1_FREQUENCY)
#define STM32_APB1_TIM12_CLKIN  (2*STM32_PCLK1_FREQUENCY)
#define STM32_APB1_TIM13_CLKIN  (2*STM32_PCLK1_FREQUENCY)
#define STM32_APB1_TIM14_CLKIN  (2*STM32_PCLK1_FREQUENCY)

/* Timers driven from APB2 will be twice PCLK2 */

#define STM32_APB2_TIM1_CLKIN   (2*STM32_PCLK2_FREQUENCY)
#define STM32_APB2_TIM8_CLKIN   (2*STM32_PCLK2_FREQUENCY)
#define STM32_APB2_TIM15_CLKIN  (2*STM32_PCLK2_FREQUENCY)
#define STM32_APB2_TIM16_CLKIN  (2*STM32_PCLK2_FREQUENCY)
#define STM32_APB2_TIM17_CLKIN  (2*STM32_PCLK2_FREQUENCY)

/* Kernel Clock Configuration
 *
 * Note: look at Table 54 in ST Manual
 */

/* I2C123 clock source */

#define STM32_RCC_D2CCIP2R_I2C123SRC RCC_D2CCIP2R_I2C123SEL_HSI

/* I2C4 clock source */

#define STM32_RCC_D3CCIPR_I2C4SRC    RCC_D3CCIPR_I2C4SEL_HSI

/* SPI123 clock source */

#define STM32_RCC_D2CCIP1R_SPI123SRC RCC_D2CCIP1R_SPI123SEL_PLL2

/* SPI45 clock source */

#define STM32_RCC_D2CCIP1R_SPI45SRC  RCC_D2CCIP1R_SPI45SEL_PLL2

/* SPI6 clock source */

#define STM32_RCC_D3CCIPR_SPI6SRC    RCC_D3CCIPR_SPI6SEL_PLL2

/* USB 1 and 2 clock source */

#define STM32_RCC_D2CCIP2R_USBSRC    RCC_D2CCIP2R_USBSEL_PLL3

/* UART clock selection */
/* reset to default to overwrite any changes done by any bootloader */

#define STM32_RCC_D2CCIP2R_USART234578_SEL RCC_D2CCIP2R_USART234578SEL_RCC
#define STM32_RCC_D2CCIP2R_USART16_SEL     RCC_D2CCIP2R_USART16SEL_RCC

/* ADC 1 2 3 clock source */

#define STM32_RCC_D3CCIPR_ADCSRC     RCC_D3CCIPR_ADCSEL_PLL2

/* FDCAN 1 2 clock source */

#define STM32_RCC_D2CCIP1R_FDCANSEL  RCC_D2CCIP1R_FDCANSEL_HSE   /* FDCAN 1 2 clock source */

#define STM32_FDCANCLK               STM32_HSE_FREQUENCY

/* UAVCAN timer */
#define UAVCAN_TIMER                 2  /* Use TIM2 for UAVCAN */

/* FLASH wait states
 *
 *  ------------ ---------- -----------
 *  Vcore        MAX ACLK   WAIT STATES
 *  ------------ ---------- -----------
 *  1.15-1.26 V     70 MHz    0
 *  (VOS1 level)   140 MHz    1
 *                 210 MHz    2
 *  1.05-1.15 V     55 MHz    0
 *  (VOS2 level)   110 MHz    1
 *                 165 MHz    2
 *                 220 MHz    3
 *  0.95-1.05 V     45 MHz    0
 *  (VOS3 level)    90 MHz    1
 *                 135 MHz    2
 *                 180 MHz    3
 *                 225 MHz    4
 *  ------------ ---------- -----------
 */

#define BOARD_FLASH_WAITSTATES 2

/* SDMMC definitions ********************************************************/
/* We don't use SDMMC, using SPI instead for MicroSD */

/* LED definitions ******************************************************************/
/* The Pixeagle board has WS2812B LED on PE14, Safety LED on PE15
 *
 * If CONFIG_ARCH_LEDS is not defined, then the user can control the LEDs in any way.
 * The following definitions are used to access individual LEDs.
 */

/* LED index values for use with board_userled() */

#define BOARD_LED1        0
#define BOARD_LED2        1
#define BOARD_NLEDS       2

#define BOARD_LED_SAFETY  BOARD_LED1  /* PE15 */
#define BOARD_LED_WS2812B BOARD_LED2  /* PE14 - handled by FastLED */

/* LED bits for use with board_userled_all() */

#define BOARD_LED1_BIT    (1 << BOARD_LED1)
#define BOARD_LED2_BIT    (1 << BOARD_LED2)

/* If CONFIG_ARCH_LEDS is defined, the usage by the board port is defined in
 * include/board.h and src/stm32_leds.c. The LEDs are used to encode OS-related
 * events as follows:
 */

#define LED_STARTED        0 /* NuttX has been started   OFF  */
#define LED_HEAPALLOCATE   1 /* Heap has been allocated  ON   */
#define LED_IRQSENABLED    2 /* Interrupts enabled       ON   */
#define LED_STACKCREATED   3 /* Idle stack created       ON   */
#define LED_INIRQ          4 /* In an interrupt          GLOW */
#define LED_SIGNAL         5 /* In a signal handler      GLOW */
#define LED_ASSERTION      6 /* An assertion failed      GLOW */
#define LED_PANIC          7 /* The system has crashed   Blink*/
#define LED_IDLE           8 /* MCU is is sleep mode     OFF  */

/* Alternate function pin selections ************************************************/

/* USART pins based on your pin table */
#define GPIO_USART2_RX   GPIO_USART2_RX_2   /* PD6  */
#define GPIO_USART2_TX   GPIO_USART2_TX_2   /* PD5  */
#define GPIO_USART2_RTS  GPIO_USART2_RTS_1  /* PD4  */
#define GPIO_USART2_CTS  (GPIO_USART2_CTS_1 | GPIO_PULLDOWN) /* PD3  */

#define GPIO_USART3_RX   GPIO_USART3_RX_3   /* PD9  */
#define GPIO_USART3_TX   GPIO_USART3_TX_3   /* PD8  */

#define GPIO_UART4_RX    GPIO_UART4_RX_1    /* PC11 */
#define GPIO_UART4_TX    GPIO_UART4_TX_1    /* PC10 */

#define GPIO_UART5_RX    GPIO_UART5_RX_1    /* PD2  */
#define GPIO_UART5_TX    GPIO_UART5_TX_1    /* PC12 */

#define GPIO_UART7_RX    GPIO_UART7_RX_3    /* PE7  */
#define GPIO_UART7_TX    GPIO_UART7_TX_3    /* PE8  */
#define GPIO_UART7_RTS   GPIO_UART7_RTS_1   /* PE9  */
#define GPIO_UART7_CTS   (GPIO_UART7_CTS_1 | GPIO_PULLDOWN) /* PE10 */

#define GPIO_UART8_RX    GPIO_UART8_RX_1    /* PE0  */
#define GPIO_UART8_TX    GPIO_UART8_TX_1    /* PE1  */

/* CAN - Updated for your pin assignments */
#define GPIO_CAN1_RX     GPIO_CAN1_RX_3     /* PD0  */
#define GPIO_CAN1_TX     GPIO_CAN1_TX_3     /* PD1  */
#define GPIO_CAN2_RX     GPIO_CAN2_RX_1     /* PB12 */
#define GPIO_CAN2_TX     GPIO_CAN2_TX_1     /* PB13 */

/* SPI - Updated for your hardware configuration */

#define ADJ_SLEW_RATE(p) (((p) & ~GPIO_SPEED_MASK) | (GPIO_SPEED_2MHz))

/* SPI1 - BMI088 */
#define GPIO_SPI1_MISO   GPIO_SPI1_MISO_1               /* PA6  */
#define GPIO_SPI1_MOSI   GPIO_SPI1_MOSI_1               /* PA7  */
#define GPIO_SPI1_SCK    ADJ_SLEW_RATE(GPIO_SPI1_SCK_1) /* PA5  */

/* SPI2 - FRAM and MicroSD */
#define GPIO_SPI2_MISO   GPIO_SPI2_MISO_3               /* PB14 */
#define GPIO_SPI2_MOSI   GPIO_SPI2_MOSI_3               /* PB15 */
#define GPIO_SPI2_SCK    ADJ_SLEW_RATE(GPIO_SPI2_SCK_3) /* PB10 */

/* SPI3 - External */
#define GPIO_SPI3_MISO   GPIO_SPI3_MISO_1               /* PB4  */
#define GPIO_SPI3_MOSI   GPIO_SPI3_MOSI_1               /* PB5  */
#define GPIO_SPI3_SCK    ADJ_SLEW_RATE(GPIO_SPI3_SCK_1) /* PB3  */

/* SPI4 - ICM-42688-P */
#define GPIO_SPI4_MISO   GPIO_SPI4_MISO_1               /* PE5  */
#define GPIO_SPI4_MOSI   GPIO_SPI4_MOSI_1               /* PE6  */
#define GPIO_SPI4_SCK    ADJ_SLEW_RATE(GPIO_SPI4_SCK_1) /* PE2  */

/* I2C - Updated for your configuration
 *
 *   The optional _GPIO configurations allow the I2C driver to manually
 *   reset the bus to clear stuck slaves.  They match the pin configuration,
 *   but are normally-high GPIOs.
 */

/* I2C1 - External */
#define GPIO_I2C1_SCL GPIO_I2C1_SCL_2       /* PB6  */
#define GPIO_I2C1_SDA GPIO_I2C1_SDA_2       /* PB7  */

#define GPIO_I2C1_SCL_GPIO                  (GPIO_OUTPUT | GPIO_OPENDRAIN |GPIO_SPEED_50MHz | GPIO_OUTPUT_SET | GPIO_PORTB | GPIO_PIN6)
#define GPIO_I2C1_SDA_GPIO                  (GPIO_OUTPUT | GPIO_OPENDRAIN |GPIO_SPEED_50MHz | GPIO_OUTPUT_SET | GPIO_PORTB | GPIO_PIN7)

/* I2C3 - IST8310 (0x0E), BMP388 (0x77) */
#define GPIO_I2C3_SCL GPIO_I2C3_SCL_1       /* PA8  */
#define GPIO_I2C3_SDA GPIO_I2C3_SDA_1       /* PC9  */

#define GPIO_I2C3_SCL_GPIO                  (GPIO_OUTPUT | GPIO_OPENDRAIN |GPIO_SPEED_50MHz | GPIO_OUTPUT_SET | GPIO_PORTA | GPIO_PIN8)
#define GPIO_I2C3_SDA_GPIO                  (GPIO_OUTPUT | GPIO_OPENDRAIN |GPIO_SPEED_50MHz | GPIO_OUTPUT_SET | GPIO_PORTC | GPIO_PIN9)

/* I2C4 - BMP390 (0x77) */
#define GPIO_I2C4_SCL GPIO_I2C4_SCL_1       /* PB8  */
#define GPIO_I2C4_SDA GPIO_I2C4_SDA_1       /* PB9  */

#define GPIO_I2C4_SCL_GPIO                  (GPIO_OUTPUT | GPIO_OPENDRAIN | GPIO_SPEED_50MHz | GPIO_OUTPUT_SET | GPIO_PORTB | GPIO_PIN8)
#define GPIO_I2C4_SDA_GPIO                  (GPIO_OUTPUT | GPIO_OPENDRAIN | GPIO_SPEED_50MHz | GPIO_OUTPUT_SET | GPIO_PORTB | GPIO_PIN9)

/* USB */
#define GPIO_OTG_FS_DM   GPIO_OTG_FS_DM_0   /* PA11 */
#define GPIO_OTG_FS_DP   GPIO_OTG_FS_DP_0   /* PA12 */
#define GPIO_OTG_FS_VBUS (GPIO_INPUT|GPIO_FLOAT|GPIO_SPEED_100MHz|GPIO_OPENDRAIN|GPIO_PORTA|GPIO_PIN9) /* PA9 */

/* PWM Output definitions based on your pin mapping */

/* Timer 1 */
#define GPIO_TIM1_CH2OUT    GPIO_TIM1_CH2OUT_2  /* PE11 - AUX GPIO 6 */
#define GPIO_TIM1_CH3OUT    GPIO_TIM1_CH3OUT_2  /* PA10 - PWM OUT 8 */
#define GPIO_TIM1_CH4OUT    GPIO_TIM1_CH4OUT_2  /* PE14 - WS2812B, not used for PWM (FastLED) */

/* Timer 3 */
#define GPIO_TIM3_CH1OUT    GPIO_TIM3_CH1OUT_2  /* PC6  - PWM OUT 5 */
#define GPIO_TIM3_CH3OUT    GPIO_TIM3_CH3OUT_1  /* PB0  - AUX GPIO 5 */
#define GPIO_TIM3_CH4IN     GPIO_TIM3_CH4IN_1   /* PB1  - PPM input */

/* Timer 4 */
#define GPIO_TIM4_CH1OUT    GPIO_TIM4_CH1OUT_2  /* PD12 - PWM OUT 1 */
#define GPIO_TIM4_CH2OUT    GPIO_TIM4_CH2OUT_2  /* PD13 - PWM OUT 2 */
#define GPIO_TIM4_CH3OUT    GPIO_TIM4_CH3OUT_2  /* PD14 - PWM OUT 3 */
#define GPIO_TIM4_CH4OUT    GPIO_TIM4_CH4OUT_2  /* PD15 - PWM OUT 4 */

/* Timer 5 */
#define GPIO_TIM5_CH1OUT    GPIO_TIM5_CH1OUT_1  /* PA0  - AUX GPIO 1 */
#define GPIO_TIM5_CH2OUT    GPIO_TIM5_CH2OUT_1  /* PA1  - AUX GPIO 2 */
#define GPIO_TIM5_CH3OUT    GPIO_TIM5_CH3OUT_1  /* PA2  - AUX GPIO 3 */
#define GPIO_TIM5_CH4OUT    GPIO_TIM5_CH4OUT_1  /* PA3  - AUX GPIO 4 */

/* Timer 8 */
#define GPIO_TIM8_CH2OUT    GPIO_TIM8_CH2OUT_1  /* PC7  - PWM OUT 6 */
#define GPIO_TIM8_CH3OUT    GPIO_TIM8_CH3OUT_1  /* PC8  - PWM OUT 7 */

/* Board-specific GPIO definitions */

/* Power control */
#define GPIO_VDD_5V_PERIPH_EN (GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_2MHz|GPIO_OUTPUT_CLEAR|GPIO_PORTA|GPIO_PIN15)

/* Safety and indicators */
#define GPIO_SAFETY_SWITCH_IN (GPIO_INPUT|GPIO_PULLUP|GPIO_PORTE|GPIO_PIN12)
#define GPIO_SAFETY_LED       (GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_2MHz|GPIO_OUTPUT_CLEAR|GPIO_PORTE|GPIO_PIN15)
#define GPIO_BUZZER           (GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_2MHz|GPIO_OUTPUT_CLEAR|GPIO_PORTE|GPIO_PIN13)

/* WS2812B LED - controlled by FastLED library */
#define GPIO_WS2812B_LED      (GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_50MHz|GPIO_OUTPUT_CLEAR|GPIO_PORTE|GPIO_PIN14)

/* PPM Input */
#define GPIO_PPM_IN           (GPIO_ALT|GPIO_AF2|GPIO_PULLUP|GPIO_PORTB|GPIO_PIN1) /* TIM3_CH4 */

/* SPI Chip Selects */
#define GPIO_SPI1_CS_BMI088_ACC (GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_50MHz|GPIO_OUTPUT_SET|GPIO_PORTA|GPIO_PIN4)
#define GPIO_SPI1_CS_BMI088_GYR (GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_50MHz|GPIO_OUTPUT_SET|GPIO_PORTB|GPIO_PIN2)
#define GPIO_SPI2_CS_FRAM       (GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_50MHz|GPIO_OUTPUT_SET|GPIO_PORTD|GPIO_PIN10)
#define GPIO_SPI2_CS_MICROSD    (GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_50MHz|GPIO_OUTPUT_SET|GPIO_PORTB|GPIO_PIN11)
#define GPIO_SPI3_CS_EXT        (GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_50MHz|GPIO_OUTPUT_SET|GPIO_PORTD|GPIO_PIN7)
#define GPIO_SPI4_CS_ICM42688P  (GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_50MHz|GPIO_OUTPUT_SET|GPIO_PORTE|GPIO_PIN4)

/* Sensor interrupt pins */
#define GPIO_BMI088_ACC_INT1    (GPIO_INPUT|GPIO_PULLUP|GPIO_EXTI|GPIO_PORTC|GPIO_PIN4)
#define GPIO_BMI088_GYR_INT1    (GPIO_INPUT|GPIO_PULLUP|GPIO_EXTI|GPIO_PORTC|GPIO_PIN5)
#define GPIO_ICM42688P_DRDY     (GPIO_INPUT|GPIO_PULLUP|GPIO_EXTI|GPIO_PORTE|GPIO_PIN3)

/* Analog pins */
#define GPIO_ADC1_IN10          (GPIO_ANALOG|GPIO_PORTC|GPIO_PIN0)  /* Analog sensor 1 */
#define GPIO_ADC1_IN11          (GPIO_ANALOG|GPIO_PORTC|GPIO_PIN1)  /* Analog sensor 2 */
#define GPIO_ADC1_IN12          (GPIO_ANALOG|GPIO_PORTC|GPIO_PIN2)  /* Battery voltage */
#define GPIO_ADC1_IN13          (GPIO_ANALOG|GPIO_PORTC|GPIO_PIN3)  /* Battery current */

/* CM4 Interface */
#define GPIO_CM4_STATUS         (GPIO_INPUT|GPIO_PULLUP|GPIO_PORTD|GPIO_PIN11)

/* Board provides GPIO or other Hardware for signaling to timing analyzer */

#if defined(CONFIG_BOARD_USE_PROBES)
# include "stm32_gpio.h"
# define PROBE_N(n) (1<<((n)-1))
# define PROBE_1    (GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_2MHz|GPIO_OUTPUT_CLEAR|GPIO_PORTA|GPIO_PIN0)   /* PA0  AUX1 */
# define PROBE_2    (GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_2MHz|GPIO_OUTPUT_CLEAR|GPIO_PORTA|GPIO_PIN1)   /* PA1  AUX2 */
# define PROBE_3    (GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_2MHz|GPIO_OUTPUT_CLEAR|GPIO_PORTA|GPIO_PIN2)   /* PA2  AUX3 */
# define PROBE_4    (GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_2MHz|GPIO_OUTPUT_CLEAR|GPIO_PORTA|GPIO_PIN3)   /* PA3  AUX4 */
# define PROBE_5    (GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_2MHz|GPIO_OUTPUT_CLEAR|GPIO_PORTB|GPIO_PIN0)   /* PB0  AUX5 */
# define PROBE_6    (GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_2MHz|GPIO_OUTPUT_CLEAR|GPIO_PORTE|GPIO_PIN11)  /* PE11 AUX6 */
# define PROBE_7    (GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_2MHz|GPIO_OUTPUT_CLEAR|GPIO_PORTC|GPIO_PIN6)   /* PC6  PWM5 */
# define PROBE_8    (GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_2MHz|GPIO_OUTPUT_CLEAR|GPIO_PORTA|GPIO_PIN10)  /* PA10 PWM8 */

# define PROBE_INIT(mask) \
	do { \
		if ((mask)& PROBE_N(1)) { stm32_configgpio(PROBE_1); } \
		if ((mask)& PROBE_N(2)) { stm32_configgpio(PROBE_2); } \
		if ((mask)& PROBE_N(3)) { stm32_configgpio(PROBE_3); } \
		if ((mask)& PROBE_N(4)) { stm32_configgpio(PROBE_4); } \
		if ((mask)& PROBE_N(5)) { stm32_configgpio(PROBE_5); } \
		if ((mask)& PROBE_N(6)) { stm32_configgpio(PROBE_6); } \
		if ((mask)& PROBE_N(7)) { stm32_configgpio(PROBE_7); } \
		if ((mask)& PROBE_N(8)) { stm32_configgpio(PROBE_8); } \
	} while(0)

# define PROBE(n,s)  do {stm32_gpiowrite(PROBE_##n,(s));}while(0)
# define PROBE_MARK(n) PROBE(n,false);PROBE(n,true)
#else
# define PROBE_INIT(mask)
# define PROBE(n,s)
# define PROBE_MARK(n)
#endif

/* High-resolution timer */
#define HRT_TIMER               8  /* use timer8 for the HRT */
#define HRT_TIMER_CHANNEL       3  /* use capture/compare channel 3 */

/* PWM Configuration */
#define DIRECT_PWM_OUTPUT_CHANNELS  8
#define DIRECT_INPUT_TIMER_CHANNELS 8

/* This board provides a DMA pool and APIs */
#define BOARD_DMA_ALLOC_POOL_SIZE 5120

/* This board provides the board_on_reset interface */
#define BOARD_HAS_ON_RESET 1

/* The list of GPIO that will be initialized */
#define PX4_GPIO_PWM_INIT_LIST { \
		GPIO_TIM1_CH2OUT,    \
		GPIO_TIM1_CH3OUT,    \
		GPIO_TIM3_CH1OUT,    \
		GPIO_TIM3_CH3OUT,    \
		GPIO_TIM4_CH1OUT,    \
		GPIO_TIM4_CH2OUT,    \
		GPIO_TIM4_CH3OUT,    \
		GPIO_TIM4_CH4OUT,    \
		GPIO_TIM5_CH1OUT,    \
		GPIO_TIM5_CH2OUT,    \
		GPIO_TIM5_CH3OUT,    \
		GPIO_TIM5_CH4OUT,    \
		GPIO_TIM8_CH2OUT,    \
		GPIO_TIM8_CH3OUT,    \
	}

#define PX4_GPIO_INIT_LIST { \
		PX4_ADC_GPIO,                     \
		GPIO_CAN1_TX,                     \
		GPIO_CAN1_RX,                     \
		GPIO_CAN2_TX,                     \
		GPIO_CAN2_RX,                     \
		GPIO_VDD_5V_PERIPH_EN,            \
		GPIO_SAFETY_SWITCH_IN,            \
		GPIO_SAFETY_LED,                  \
		GPIO_BUZZER,                      \
		GPIO_WS2812B_LED,                 \
		GPIO_PPM_IN,                      \
		GPIO_CM4_STATUS,                  \
	}

/* Define True logic Power Control in arch agnostic form */
#define VDD_5V_PERIPH_EN(on_true)          px4_arch_gpiowrite(GPIO_VDD_5V_PERIPH_EN, (on_true))
#define VDD_5V_HIPOWER_EN(on_true)         VDD_5V_PERIPH_EN(on_true)

/* Define True logic Power Control in arch agnostic form */
#define PX4_ADC_GPIO  \
	/* PA0 */  GPIO_ADC1_IN10, \
	/* PC1 */  GPIO_ADC1_IN11, \
	/* PC2 */  GPIO_ADC1_IN12, \
	/* PC3 */  GPIO_ADC1_IN13

/* Define Battery 1 Voltage Divider and A per V */
#define BOARD_BATTERY1_V_DIV         (11.0f)     /* measured with the provided PM board */
#define BOARD_BATTERY1_A_PER_V       (17.0f)

/* HW has to large of R termination on cubepilot serial port, hw debug is not possible */
#define PX4_GPIO_INIT_SUBDRIVER_LIST { \
		LED_SAFETY_INIT, \
	}

__EXPORT void stm32_spiinitialize(void);

#define board_spi_reset(ms, bus_mask)

#define PX4_SPI_BUS_SENSORS     1
#define PX4_SPI_BUS_RAMTRON     2
#define PX4_SPI_BUS_EXT         3
#define PX4_SPI_BUS_ICM42688P   4

/* SPI chip selects */
#define PX4_SPIDEV_BMI088_ACC          PX4_MK_SPI_SEL(PX4_SPI_BUS_SENSORS,0)
#define PX4_SPIDEV_BMI088_GYR          PX4_MK_SPI_SEL(PX4_SPI_BUS_SENSORS,1)
#define PX4_SPIDEV_FRAM                PX4_MK_SPI_SEL(PX4_SPI_BUS_RAMTRON,0)
#define PX4_SPIDEV_MICROSD             PX4_MK_SPI_SEL(PX4_SPI_BUS_RAMTRON,1)
#define PX4_SPIDEV_EXT0                PX4_MK_SPI_SEL(PX4_SPI_BUS_EXT,0)
#define PX4_SPIDEV_ICM42688P           PX4_MK_SPI_SEL(PX4_SPI_BUS_ICM42688P,0)

/* I2C busses */
#define PX4_I2C_BUS_EXT                1
#define PX4_I2C_BUS_IST8310_BMP388     3
#define PX4_I2C_BUS_BMP390             4

/* Devices on the onboard buses.
 *
 * Note that these are unshifted addresses.
 */
#define PX4_I2C_OBDEV_IST8310          0x0e
#define PX4_I2C_OBDEV_BMP388           0x77
#define PX4_I2C_OBDEV_BMP390           0x77

/* Safety Switch is HW version dependent on having an PX4IO
 * So we init all the pins that could be used.
 */
#define GPIO_SAFETY_SWITCH_IN        GPIO_SAFETY_SWITCH_IN
#define GPIO_SAFETY_LED              GPIO_SAFETY_LED
/* Enable the FMU to control it if there is no px4io fixme:This should be BOARD_SAFETY_LED(__ontrue) */
#define GPIO_LED_SAFETY GPIO_SAFETY_LED
#define BOARD_SAFETY_LED(on_true)    px4_arch_gpiowrite(GPIO_SAFETY_LED, !(on_true))

/* Power switch controls */
#define SPEKTRUM_POWER(_on_true)                  VDD_5V_PERIPH_EN((_on_true))

/*
 * ADC channels
 *
 * These are the channel numbers of the ADCs of the microcontroller that
 * can be used by the Px4 Firmware in the adc driver
 */

/* ADC defines to be used in sensors.cpp to read from a particular channel */
#define ADC1_CH(n)                  (n)
#define ADC1_GPIO(n)                GPIO_ADC1_IN##n

/* Define GPIO pins used as ADC N.B. Channel numbers must match below */
#define PX4_ADC_GPIO  \
	/* PC0 */  GPIO_ADC1_IN10, \
	/* PC1 */  GPIO_ADC1_IN11, \
	/* PC2 */  GPIO_ADC1_IN12, \
	/* PC3 */  GPIO_ADC1_IN13

/* Define Channel numbers must match above GPIO pin IN(n)*/
#define ADC_ANALOG_SENSOR1_CHANNEL                 ADC1_CH(10)
#define ADC_ANALOG_SENSOR2_CHANNEL                 ADC1_CH(11)
#define ADC_BATTERY_VOLTAGE_CHANNEL                ADC1_CH(12)
#define ADC_BATTERY_CURRENT_CHANNEL                ADC1_CH(13)

#define ADC_CHANNELS \
	((1 << ADC_ANALOG_SENSOR1_CHANNEL)       | \
	 (1 << ADC_ANALOG_SENSOR2_CHANNEL)       | \
	 (1 << ADC_BATTERY_VOLTAGE_CHANNEL)      | \
	 (1 << ADC_BATTERY_CURRENT_CHANNEL))

/* HW Rev and Ver detection */
#define BOARD_HAS_HW_SPLIT_VERSIONING

#define HW_INFO_INIT_PREFIX           "PX4_"

#define HW_INFO_INIT \
		{HW_INFO_INIT_PREFIX"FMUV6C", "PIXEAGLE", "V1.0"},

enum board_bus_types {
	BOARD_INVALID_BUS = 0,
	BOARD_SPI_BUS     = 1,
	BOARD_I2C_BUS     = 2,
};

/* Board configuration */
#define BOARD_TYPE             100
#define BOARD_FLASH_SECTORS    16
#define BOARD_FLASH_SIZE       (2 * 1024 * 1024)
#define BOARD_HAS_NO_RESET     0
#define BOARD_HAS_NO_BOOTLOADER 0

#endif  /* __NUTTX_CONFIG_PX4_PIXEAGLE_INCLUDE_BOARD_H */
