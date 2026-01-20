/****************************************************************************
 * boards/px4/pixeagle/src/spi_register.c
 *
 * Board-specific SPI callback registration for Pixeagle.
 *
 * NOTE: stm32_spiXselect and stm32_spiXstatus are NOT defined here because
 * they are provided by the common PX4/NuttX SPI driver (libarch_spi), which
 * uses the configuration from spi.cpp to toggle pins automatically.
 * * We only provide stm32_spiXregister because CONFIG_SPI_CALLBACK is enabled.
 ****************************************************************************/
#include <nuttx/config.h>
#include <nuttx/spi/spi.h>
#include <stm32_gpio.h>
#include <stm32_spi.h>
#include "board_config.h"

/****************************************************************************
 * Name: stm32_spi1register
 ****************************************************************************/
int stm32_spi1register(FAR struct spi_dev_s *dev, spi_mediachange_t callback, FAR void *arg){    return OK;}
/****************************************************************************
 * Name: stm32_spi2register
 ****************************************************************************/
int stm32_spi2register(FAR struct spi_dev_s *dev, spi_mediachange_t callback, FAR void *arg){    return OK;}

/****************************************************************************
 * Name: stm32_spi3register
 ****************************************************************************/
int stm32_spi3register(FAR struct spi_dev_s *dev, spi_mediachange_t callback, FAR void *arg){    return OK;}
/****************************************************************************
 * Name: stm32_spi4register
 ****************************************************************************/
int stm32_spi4register(FAR struct spi_dev_s *dev, spi_mediachange_t callback, FAR void *arg){    return OK;}

