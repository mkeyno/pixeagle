/* script.ld
 *Linker script for the main firmware, defining memory layout for STM32H743 (2MB flash, 1MB RAM).
 * Linker script for Pixeagle firmware (STM32H743VIT6, 16MHz HSE, 2MB flash, 1MB RAM).
 * Flash: 0x08000000 (2048KB), RAM: 0x20000000 (1MB).
 * Hardware: SPI1-4, I2C1/3/4, UART3-5/7-8, CAN1-2, TIM1/3-5/8, ADC (PC0-3).
 * Sensor power: PA15 (GPIO_VDD_5V_PERIPH_EN, active high).
 * WS2812B: PE14 (FastLED). No PX4IO.
 */

MEMORY
{
    FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 2048K
    SRAM (rwx) : ORIGIN = 0x20000000, LENGTH = 1024K
}

OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-little")
OUTPUT_ARCH(arm)
ENTRY(__start)

SECTIONS
{
    .text : {
        _stext = .;
        *(.vector_table)
        *(.text .text.*)
        *(.fixup)
        *(.gnu.warning)
        *(.rodata .rodata.*)
        *(.glue_7t) *(.glue_7)
        . = ALIGN(4);
        _etext = .;
    } > FLASH

    .data : {
        _sdata = .;
        *(.data .data.*)
        . = ALIGN(4);
        _edata = .;
    } > SRAM AT > FLASH

    .bss : {
        _sbss = .;
        *(.bss .bss.*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > SRAM

    _end = .;
}
