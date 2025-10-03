/**
 * @file board_dma_alloc.c
 *
 * Pixeagle DMA allocation helpers
 */

#include <px4_platform_common/px4_config.h>
#include <stdint.h>

/**
 * Get DMA usage statistics
 *
 * @param dma DMA usage array (8 elements for DMA1/DMA2 streams)
 */
__EXPORT void board_get_dma_usage(uint16_t *dma)
{
	// STM32H7 has DMA1 and DMA2, each with 8 streams
	// Initialize all to 0 (unused)
	for (int i = 0; i < 8; i++) {
		dma[i] = 0;
	}
	
	// Mark used DMA streams based on board_dma_map.h
	// DMA1_Stream0: SPI1 RX (BMI088)
	dma[0] = 1;
	// DMA1_Stream1: SPI1 TX (BMI088)
	dma[1] = 1;
	// DMA1_Stream2: UART3 RX (SBUS/PPM)
	dma[2] = 1;
	// DMA1_Stream3: SPI2 RX (MicroSD/FRAM)
	dma[3] = 1;
	// DMA1_Stream4: SPI2 TX (MicroSD/FRAM)
	dma[4] = 1;
	// DMA1_Stream5: SPI4 RX (ICM42688P)
	dma[5] = 1;
	// DMA1_Stream6: SPI4 TX (ICM42688P)
	dma[6] = 1;
}