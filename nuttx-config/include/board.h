#pragma once

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

//////////////////////////////////////////////////
/* Network socket definitions */
#ifdef CONFIG_NET
#  include <netinet/in.h>
#  include <sys/socket.h>
#  ifndef NET_SOCK_FAMILY
#    define NET_SOCK_FAMILY   PF_INET
#  endif
#  ifndef NET_SOCK_TYPE
#    define NET_SOCK_TYPE     SOCK_DGRAM
#  endif
#  ifndef NET_SOCK_PROTOCOL
#    define NET_SOCK_PROTOCOL 0
#  endif
#endif
// Add these CAN includes
// Conditional CAN includes - only for main firmware, not bootloader
#ifndef CONFIG_BUILD_BOOTLOADER
#  include <nuttx/can/can.h>
#  include <nuttx/net/netdev.h>

  // Fix the arm_netinitialize macro/function conflict
#  ifdef arm_netinitialize
#    undef arm_netinitialize
#  endif
  void arm_netinitialize(void);
#endif


/* NuttX SPI compatibility fix */
#ifndef spi_mediachange_t
typedef CODE void (*spi_mediachange_t)(FAR void *arg);
#endif

//////////////////////////////////////////////////////////////////////////

/* Clocking *************************************************************************/
/* The board provides the following clock sources:
 *
 *   X1: 16 MHz crystal for HSE
 *
 * So we have these clock source available within the STM32
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
 *
 *   PLL1P = PLL1_VCO/2  = 960 MHz / 2   = 480 MHz
 *   PLL1Q = PLL1_VCO/4  = 960 MHz / 4   = 240 MHz
 *   PLL1R = PLL1_VCO/8  = 960 MHz / 8   = 120 MHz
 */

#define STM32_PLLCFG_PLL1CFG    (RCC_PLLCFGR_PLL1VCOSEL_WIDE|RCC_PLLCFGR_PLL1RGE_4_8_MHZ|RCC_PLLCFGR_DIVP1EN|RCC_PLLCFGR_DIVQ1EN|RCC_PLLCFGR_DIVR1EN)
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

#define STM32_PLLCFG_PLL2CFG     (RCC_PLLCFGR_PLL2VCOSEL_WIDE|RCC_PLLCFGR_PLL2RGE_4_8_MHZ|RCC_PLLCFGR_DIVP2EN|RCC_PLLCFGR_DIVQ2EN|RCC_PLLCFGR_DIVR2EN)
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

#define STM32_PLLCFG_PLL3CFG    (RCC_PLLCFGR_PLL3VCOSEL_WIDE|RCC_PLLCFGR_PLL3RGE_4_8_MHZ|RCC_PLLCFGR_DIVQ3EN)
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

/* APB4 clock (PCLK4) is HCLK/4 (120 MHz) */

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

#define STM32_RCC_D2CCIP2R_I2C123SRC RCC_D2CCIP2R_I2C123SEL_HSI		/* I2C123 clock source */

#define STM32_RCC_D3CCIPR_I2C4SRC    RCC_D3CCIPR_I2C4SEL_HSI		/* I2C4 clock source */

#define STM32_RCC_D2CCIP1R_SPI123SRC RCC_D2CCIP1R_SPI123SEL_PLL2	/* SPI123 clock source */

#define STM32_RCC_D2CCIP1R_SPI45SRC  RCC_D2CCIP1R_SPI45SEL_PLL2		/* SPI45 clock source */

#define STM32_RCC_D3CCIPR_SPI6SRC    RCC_D3CCIPR_SPI6SEL_PLL2		/* SPI6 clock source */

#define STM32_RCC_D2CCIP2R_USBSRC    RCC_D2CCIP2R_USBSEL_PLL3		/* USB 1 and 2 clock source */

#define STM32_RCC_D3CCIPR_ADCSRC     RCC_D3CCIPR_ADCSEL_PLL2		/* ADC 1 2 3 clock source */

#define STM32_RCC_D2CCIP1R_FDCANSEL  RCC_D2CCIP1R_FDCANSEL_HSE   /* FDCAN 1 2 clock source */

#define STM32_FDCANCLK               STM32_HSE_FREQUENCY

/* FLASH wait states */

#define BOARD_FLASH_WAITSTATES 2

/* SDMMC definitions ********************************************************/

/* Init 400kHz, freq = PLL1Q/(2*div)  div =  PLL1Q/(2*freq) */

#define STM32_SDMMC_INIT_CLKDIV     (300 << STM32_SDMMC_CLKCR_CLKDIV_SHIFT)

/* 25 MHz Max for now, 25 mHZ = PLL1Q/(2*div), div =  PLL1Q/(2*freq)
 * div = 4.8 = 240 / 50, So round up to 5 for default speed 24 MB/s
 */

#if defined(CONFIG_STM32H7_SDMMC_XDMA) || defined(CONFIG_STM32H7_SDMMC_IDMA)
#  define STM32_SDMMC_MMCXFR_CLKDIV   (5 << STM32_SDMMC_CLKCR_CLKDIV_SHIFT)
#else
#  define STM32_SDMMC_MMCXFR_CLKDIV   (100 << STM32_SDMMC_CLKCR_CLKDIV_SHIFT)
#endif
#if defined(CONFIG_STM32H7_SDMMC_XDMA) || defined(CONFIG_STM32H7_SDMMC_IDMA)
#  define STM32_SDMMC_SDXFR_CLKDIV    (5 << STM32_SDMMC_CLKCR_CLKDIV_SHIFT)
#else
#  define STM32_SDMMC_SDXFR_CLKDIV    (100 << STM32_SDMMC_CLKCR_CLKDIV_SHIFT)
#endif

#define STM32_SDMMC_CLKCR_EDGE      STM32_SDMMC_CLKCR_NEGEDGE

/* LED definitions ******************************************************************/
/* The Pixeagle board has three LEDs that can be controlled by software.
 *
 * If CONFIG_ARCH_LEDS is not defined, then the user can control the LEDs in any way.
 * The following definitions are used to access individual LEDs.
 */

/* LED index values for use with board_userled() */

#define BOARD_LED1        0
#define BOARD_LED2        1
#define BOARD_LED3        2
#define BOARD_NLEDS       3

#define BOARD_LED_RED     BOARD_LED1
#define BOARD_LED_GREEN   BOARD_LED2
#define BOARD_LED_BLUE    BOARD_LED3

/* LED bits for use with board_userled_all() */

#define BOARD_LED1_BIT    (1 << BOARD_LED1)
#define BOARD_LED2_BIT    (1 << BOARD_LED2)
#define BOARD_LED3_BIT    (1 << BOARD_LED3)

/* If CONFIG_ARCH_LEDS is defined, the usage by the board port is defined in
 * include/board.h and src/stm32_leds.c. The LEDs are used to encode OS-related
 * events as follows:
 *
 *
 *   SYMBOL                     Meaning                      LED state
 *                                                        Red   Green Blue
 *   ----------------------  --------------------------  ------ ------ ----*/

#define LED_STARTED        0 /* NuttX has been started   OFF    OFF   OFF  */
#define LED_HEAPALLOCATE   1 /* Heap has been allocated  OFF    OFF   ON   */
#define LED_IRQSENABLED    2 /* Interrupts enabled       OFF    ON    OFF  */
#define LED_STACKCREATED   3 /* Idle stack created       OFF    ON    ON   */
#define LED_INIRQ          4 /* In an interrupt          N/C    N/C   GLOW */
#define LED_SIGNAL         5 /* In a signal handler      N/C    GLOW  N/C  */
#define LED_ASSERTION      6 /* An assertion failed      GLOW   N/C   GLOW */
#define LED_PANIC          7 /* The system has crashed   Blink  OFF   N/C  */
#define LED_IDLE           8 /* MCU is is sleep mode     ON     OFF   OFF  */

/* Thus if the Green LED is statically on, NuttX has successfully booted and
 * is, apparently, running normally.  If the Red LED is flashing at
 * approximately 2Hz, then a fatal error has been detected and the system
 * has halted.
 */

/* Alternate function pin selections ************************************************/

/* UART Definitions - Based on Pixeagle pinout */

#define GPIO_USART2_RX   GPIO_USART2_RX_2      /* PD6 */
#define GPIO_USART2_TX   GPIO_USART2_TX_2      /* PD5 */
#define GPIO_USART2_RTS  GPIO_USART2_RTS_2     /* PD4 */
#define GPIO_USART2_CTS  GPIO_USART2_CTS_NSS_2 /* PD3 */

#define GPIO_USART3_RX   GPIO_USART3_RX_3      /* PD9 - RC SBUS */
#define GPIO_USART3_TX   GPIO_USART3_TX_3      /* PD8 */

#define GPIO_UART4_RX    GPIO_UART4_RX_3       /* PC11 - DEBUG */
#define GPIO_UART4_TX    GPIO_UART4_TX_3       /* PC10 */

#define GPIO_UART5_RX    GPIO_UART5_RX_1       /* PD2 - IO */
#define GPIO_UART5_TX    GPIO_UART5_TX_3       /* PC12 */

#define GPIO_UART7_RX    GPIO_UART7_RX_1       /* PE8 - RPi CM4 */
#define GPIO_UART7_TX    GPIO_UART7_TX_1       /* PE7 */
#define GPIO_UART7_RTS   GPIO_UART7_RTS_1      /* PE9 */
#define GPIO_UART7_CTS   GPIO_UART7_CTS_1      /* PE10 */

#define GPIO_UART8_RX    GPIO_UART8_RX_1       /* PE0 */
#define GPIO_UART8_TX    GPIO_UART8_TX_1       /* PE1 */

/* CAN - Based on Pixeagle pinout */

#define GPIO_CAN1_RX     GPIO_CAN1_RX_3        /* PD0 */
#define GPIO_CAN1_TX     GPIO_CAN1_TX_3        /* PD1 */

#define GPIO_CAN2_RX     GPIO_CAN2_RX_2        /* PB12 */
#define GPIO_CAN2_TX     GPIO_CAN2_TX_2        /* PB13 */

/* SPI - Based on Pixeagle pinout */

/* SPI1 - BMI088 */
#define GPIO_SPI1_MISO   GPIO_SPI1_MISO_1      /* PA6 */
#define GPIO_SPI1_MOSI   GPIO_SPI1_MOSI_1      /* PA7 */
#define GPIO_SPI1_SCK    GPIO_SPI1_SCK_1       /* PA5 */

/* SPI2 - SD Card & FRAM (shared bus) */
#define GPIO_SPI2_MISO   GPIO_SPI2_MISO_1      /* PB14 */
#define GPIO_SPI2_MOSI   GPIO_SPI2_MOSI_1      /* PB15 */
#define GPIO_SPI2_SCK    GPIO_SPI2_SCK_3       /* PB10 */

/* SPI3 - External */
#define GPIO_SPI3_MISO   GPIO_SPI3_MISO_1      /* PB4 */
#define GPIO_SPI3_MOSI   GPIO_SPI3_MOSI_2      /* PB5 */
#define GPIO_SPI3_SCK    GPIO_SPI3_SCK_1       /* PB3 */

/* SPI4 - ICM-42688-P */
#define GPIO_SPI4_MISO   GPIO_SPI4_MISO_1      /* PE5 */
#define GPIO_SPI4_MOSI   GPIO_SPI4_MOSI_1      /* PE6 */
#define GPIO_SPI4_SCK    GPIO_SPI4_SCK_2       /* PE2 */

/* I2C - Based on Pixeagle pinout */

/* I2C1 - External */
#define GPIO_I2C1_SCL    GPIO_I2C1_SCL_2       /* PB6 */
#define GPIO_I2C1_SDA    GPIO_I2C1_SDA_2       /* PB7 */

/* I2C3 - BMP388 & IST8310 (shared) */
#define GPIO_I2C3_SCL    GPIO_I2C3_SCL_1       /* PA8 */
#define GPIO_I2C3_SDA    GPIO_I2C3_SDA_2       /* PC9 */

/* I2C4 - BMP390 */
#define GPIO_I2C4_SCL    (GPIO_ALT|GPIO_AF4|GPIO_SPEED_50MHz|GPIO_OPENDRAIN|GPIO_PORTB|GPIO_PIN8)
#define GPIO_I2C4_SDA    (GPIO_ALT|GPIO_AF4|GPIO_SPEED_50MHz|GPIO_OPENDRAIN|GPIO_PORTB|GPIO_PIN9)
