/****************************************************************************
 * boards/px4/pixeagle/src/spi_register.c
 *
 * SPI device registration callbacks
 *
 ****************************************************************************/

#include <nuttx/config.h>
#include <nuttx/spi/spi.h>

/****************************************************************************
 * Name: stm32_spi1/2/3/4register
 *
 * Description:
 *   Called to configure SPI chip select GPIO pins. These are dummy
 *   implementations since Pixeagle doesn't use media change detection.
 *
 ****************************************************************************/

void stm32_spi1register(struct spi_dev_s *dev, spi_mediachange_t callback, void *arg)
{
	/* No media change callback for SPI1 */
}

void stm32_spi2register(struct spi_dev_s *dev, spi_mediachange_t callback, void *arg)
{
	/* No media change callback for SPI2 */
}

void stm32_spi3register(struct spi_dev_s *dev, spi_mediachange_t callback, void *arg)
{
	/* No media change callback for SPI3 */
}

void stm32_spi4register(struct spi_dev_s *dev, spi_mediachange_t callback, void *arg)
{
	/* No media change callback for SPI4 */
}