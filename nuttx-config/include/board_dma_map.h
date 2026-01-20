#pragma once

/*
 * DMA Mapping for Pixeagle board
 * Updated to match current defconfig (January 2026)
 *
 * DMA enabled only for:
 *   - SPI1 (BMI088 Accel+Gyro)
 *   - SPI4 (ICM-42688-P)
 *
 * DMA explicitly DISABLED for:
 *   - SPI2 (FRAM + SD Card) → fixes previous crashes
 *   - SPI3 (External connector)
 *   - All UARTs (no UART DMA in defconfig)
 *   - SPI5 / SPI6 (not used)
 */

#ifndef __ASSEMBLY__

/* ============================================================================
 * SPI1 - BMI088 (Accel + Gyro) → DMA ENABLED
 * ============================================================================ */
#define DMAMAP_SPI1_RX    DMAMAP_DMA12_SPI1RX_0     /* DMA1 Stream 3, Channel 37 */
#define DMAMAP_SPI1_TX    DMAMAP_DMA12_SPI1TX_0     /* DMA1 Stream 4, Channel 38 */


/* ============================================================================
 * SPI2 - Shared bus (FRAM + SD Card) → DMA DISABLED in defconfig
 * ============================================================================ */
//#define DMAMAP_SPI2_RX    DMAMAP_DMA12_SPI2RX_0   /* DMA1:39 */
//#define DMAMAP_SPI2_TX    DMAMAP_DMA12_SPI2TX_0   /* DMA1:40 */


/* ============================================================================
 * SPI3 - External connector → DMA DISABLED
 * ============================================================================ */
// No DMA mappings defined → matches # CONFIG_STM32H7_SPI3_DMA is not set


/* ============================================================================
 * SPI4 - ICM-42688-P → DMA ENABLED
 * ============================================================================ */
#define DMAMAP_SPI4_RX    DMAMAP_DMA12_SPI4RX_0     /* DMA1 Stream 4, Channel 83 */
#define DMAMAP_SPI4_TX    DMAMAP_DMA12_SPI4TX_0     /* DMA1 Stream 5, Channel 84 */


/* ============================================================================
 * UART DMA mappings → NOT enabled in current defconfig
 * Keep commented unless you later enable UARTx_DMA in menuconfig
 * ============================================================================ */
// #define DMAMAP_USART2_RX  DMAMAP_DMA12_USART2RX_0   /* DMA1:43 */
// #define DMAMAP_USART2_TX  DMAMAP_DMA12_USART2TX_0   /* DMA1:44 */

// #define DMAMAP_USART3_RX  DMAMAP_DMA12_USART3RX_0   /* DMA1:45 */
// #define DMAMAP_USART3_TX  DMAMAP_DMA12_USART3TX_0   /* DMA1:46 */

// #define DMAMAP_UART4_RX   DMAMAP_DMA12_UART4RX_0
// #define DMAMAP_UART4_TX   DMAMAP_DMA12_UART4TX_0

// #define DMAMAP_UART5_RX   DMAMAP_DMA12_UART5RX_0    /* DMA1:53 */
// #define DMAMAP_UART5_TX   DMAMAP_DMA12_UART5TX_0    /* DMA1:54 */

// #define DMAMAP_UART7_RX   DMAMAP_DMA12_UART7RX_0    /* DMA1:79 */
// #define DMAMAP_UART7_TX   DMAMAP_DMA12_UART7TX_0    /* DMA1:80 */

// #define DMAMAP_UART8_RX   DMAMAP_DMA12_UART8RX_0    /* DMA1:81 - Bootloader serial */
// #define DMAMAP_UART8_TX   DMAMAP_DMA12_UART8TX_0    /* DMA1:82 */


/* ============================================================================
 * Unused / Disabled in defconfig
 * ============================================================================ */
// No definitions needed for SPI5, SPI6 (disabled)

#endif /* __ASSEMBLY__ */
