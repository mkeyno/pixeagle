/**
 * @file bootloader_main.c
 *
 * Pixeagle-specific early startup code for bootloader
 */
#include <nuttx/serial/serial.h>
#include "board_config.h"
#include "bl.h"

#include <nuttx/config.h>
#include <nuttx/board.h>
#include <chip.h>
#include <stm32_uart.h>
#include <arch/board/board.h>
#include <px4_platform_common/init.h>


// Add this function to fix the linker error
#include <stdbool.h>

/**
 * @brief Check if flash cache is dirty
 * Required by flash_cache.c but may not be compiled in some configurations
 */
bool fc_is_dirty(void)
{
    // Simple implementation for bootloader
    // In full implementation, this would check if cache needs flushing
    return false;
}


/**
 * @brief Late initialization for the board
 */
void board_late_initialize(void)
{
    /* Enable sensor power rail (PA15, active high) */
    px4_arch_gpiowrite(GPIO_VDD_5V_PERIPH_EN, 1);

    modifyreg32(STM32_RCC_CR, 0, RCC_CR_HSEON);  // Enable HSE
    while (!(getreg32(STM32_RCC_CR) & RCC_CR_HSERDY));  // Wait ready
}

/**
 * @brief Timer hook for system tick handling
 */
extern void sys_tick_handler(void);

void board_timerhook(void)
{
    sys_tick_handler();
}

__EXPORT int main(int argc, char *argv[]);

extern void led_on(unsigned led);
extern void led_off(unsigned led);

int main(int argc, char *argv[])
{
    // Call the actual bootloader with timeout
    // 0 = infinite timeout (stay in bootloader forever)
    bootloader(0);
    
    return 0;
}
