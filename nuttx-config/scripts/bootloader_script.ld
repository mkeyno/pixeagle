/* bootloader_script.ld
 * Linker script for the bootloader, reserving 256KB flash for bootloader code
 * Linker script for Pixeagle bootloader (STM32H743VIT6, 16MHz HSE).
 * Flash: 0x08000000 (256KB reserved), RAM: 0x20000000 (1MB).
 * Bootloader uses UART4 (PC10/PC11, debug, /dev/ttyS3).
 * No sensor power (PA15) or peripherals (SPI/I2C/PWM) in bootloader.
 */

MEMORY
{
    FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 256K
    SRAM (rwx) : ORIGIN = 0x20000000, LENGTH = 1024K
}

OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-little")
OUTPUT_ARCH(arm)
ENTRY(__start)

SECTIONS
{
    .text : {
        _stext = ABSOLUTE(.);
        *(.vectors)
        *(.text .text.*)
        *(.fixup)
        *(.gnu.warning)
        *(.rodata .rodata.*)
        *(.gnu.linkonce.t.*)
        *(.glue_7)
        *(.glue_7t)
        *(.got)
        *(.gcc_except_table)
        *(.gnu.linkonce.r.*)
        _etext = ABSOLUTE(.);
        _eronly = ABSOLUTE(.);  /* ADD THIS LINE */
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
