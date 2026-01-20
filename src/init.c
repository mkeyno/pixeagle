#include "board_config.h"
#include <nuttx/config.h>
#include <nuttx/board.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <debug.h>
#include <errno.h>
#include <syslog.h>
#include <sys/stat.h>
#include <chip.h>
#include <stm32_rcc.h>
#include <arch/board/board.h>
#include <nuttx/spi/spi.h>
#include <sys/mount.h>
#include <unistd.h>
#include <drivers/drv_hrt.h>
#include <systemlib/px4_macros.h>
#include <px4_platform_common/init.h>
#include <px4_platform/gpio.h>
#include <px4_platform/board_determine_hw_info.h>
#include <px4_platform/board_dma_alloc.h>
#include <px4_arch/io_timer.h>
#include <nuttx/mtd/mtd.h>
#include <nuttx/mmcsd.h>
#include <stdint.h>
#include <nuttx/irq.h>
#include <nuttx/drivers/drivers.h>
#include <nuttx/fs/nxffs.h>
#include <arch/armv7-m/nvicpri.h>

#include <fcntl.h>  // For O_RDWR
#include <inttypes.h>


/* CRS Defines for HSI48 Sync */
#ifndef STM32_CRS_BASE
#  define STM32_CRS_BASE     0x58024c00
#endif
#define STM32_CRS_CR         (STM32_CRS_BASE + 0x00)
#define CRS_CR_CEN           (1 << 0)
#define CRS_CR_AUTOTRIMEN    (1 << 14)
#ifndef RCC_APB1HENR_CRSEN
#  define RCC_APB1HENR_CRSEN (1 << 1)
#endif

/* Constants */
#define FRAM_RDID_CMD      0x9F
#define FRAM_ID_LEN        9
#define FRAM_TEST_ADDR     0x003FFF
#define FRAM_TEST_PATTERN  0xA5

/* Ensure these registers are defined */
#ifndef STM32_RCC_APB1LENR
#  define STM32_RCC_APB1LENR  (STM32_RCC_BASE + 0x0E8)
#endif
#ifndef RCC_APB1LENR_SPI2EN
#  define RCC_APB1LENR_SPI2EN (1 << 14)
#endif

/* GPIO Base Addresses */
#ifndef STM32_GPIOB_BASE
#  define STM32_GPIOB_BASE 0x58020400
#endif
#ifndef STM32_GPIOD_BASE
#  define STM32_GPIOD_BASE 0x58020C00
#endif
#ifndef STM32_GPIO_MODER_OFFSET
#  define STM32_GPIO_MODER_OFFSET 0x00
#endif
#ifndef STM32_GPIO_ODR_OFFSET
#  define STM32_GPIO_ODR_OFFSET 0x14
#endif


/* GPIO Clock Enable bits */
#ifndef RCC_AHB4ENR_GPIOBEN
#  define RCC_AHB4ENR_GPIOBEN (1 << 1)
#endif
#ifndef RCC_AHB4ENR_GPIODEN
#  define RCC_AHB4ENR_GPIODEN (1 << 3)
#endif
#ifndef STM32_RCC_AHB4ENR
#  define STM32_RCC_AHB4ENR (STM32_RCC_BASE + 0x0E0)
#endif


extern void oled_debug_init(void);
extern void oled_debug_step(int step, const char *msg);
extern FAR struct mtd_dev_s *ramtron_initialize(FAR struct spi_dev_s *dev);
extern int mkfatfs(FAR const char *pathname, uint8_t nfats);

void sched_note_add(FAR const void *note, size_t notelen) { }

static void setup_usb(void)
{
    uint32_t rcc_cr = getreg32(STM32_RCC_CR);
    rcc_cr |= RCC_CR_HSI48ON;
    putreg32(rcc_cr, STM32_RCC_CR);

    int timeout = 10000;
    while (!(getreg32(STM32_RCC_CR) & RCC_CR_HSI48RDY) && --timeout > 0);
    if (timeout == 0) {
        syslog(LOG_ERR, "[boot] HSI48 failed!\n");
        return;
    }
    syslog(LOG_INFO, "[boot] HSI48 OK\n");

    uint32_t d2ccip2r = getreg32(STM32_RCC_D2CCIP2R);
    d2ccip2r &= ~RCC_D2CCIP2R_USBSEL_MASK;
    d2ccip2r |= RCC_D2CCIP2R_USBSEL_HSI48;
    putreg32(d2ccip2r, STM32_RCC_D2CCIP2R);

    modifyreg32(STM32_RCC_APB1HENR, 0, RCC_APB1HENR_CRSEN);
    modifyreg32(STM32_CRS_CR, 0, CRS_CR_AUTOTRIMEN | CRS_CR_CEN);

    syslog(LOG_INFO, "[boot] USB clock (HSI48 + CRS) OK\n");
    syslog(LOG_INFO, "setup_usb: RCC_CR=0x%08lX D2CCIP2R=0x%08lX\n", 
           getreg32(STM32_RCC_CR), getreg32(STM32_RCC_D2CCIP2R));
}

static void clock_debug(void)
{
    uint32_t cfgr = getreg32(STM32_RCC_CFGR);
    uint32_t cr = getreg32(STM32_RCC_CR);
    uint32_t sws = (cfgr & RCC_CFGR_SWS_MASK) >> RCC_CFGR_SWS_SHIFT;
    uint32_t pllcfgr = getreg32(STM32_RCC_PLLCFGR);
    
    syslog(LOG_INFO, "PLLCFG:0x%08lx\n", (unsigned long)pllcfgr);
    
    bool hse_ready = (cr & RCC_CR_HSERDY);
    bool pll1_ready = (cr & RCC_CR_PLL1RDY);
    
    if (sws == 3) {
        syslog(LOG_INFO, "Clk: PLL1 OK\n");
        
        uint32_t pll1divr = getreg32(STM32_RCC_PLL1DIVR);
        uint32_t pllckselr = getreg32(STM32_RCC_PLLCKSELR);
        
        uint32_t pll1n = ((pll1divr >> RCC_PLL1DIVR_N1_SHIFT) & 0x1FF) + 1;
        uint32_t pll1p = ((pll1divr >> RCC_PLL1DIVR_P1_SHIFT) & 0x7F) + 1;
        uint32_t pll1m = ((pllckselr >> RCC_PLLCKSELR_DIVM1_SHIFT) & 0x3F);
        
        if (pll1m == 0) pll1m = 1;
        
        uint32_t vco_freq = (16000000 / pll1m) * pll1n;
        uint32_t sys_freq = vco_freq / pll1p;
        
        syslog(LOG_INFO, "SYSCLK:%luMHz\n", sys_freq / 1000000);
        up_mdelay(1000);
        
    } else if (sws == 0) {
        syslog(LOG_INFO, "HSE:%d PLL:%d\n", hse_ready, pll1_ready);
        
        if (!hse_ready) {
            syslog(LOG_INFO, "HSE FAIL-retry");
            modifyreg32(STM32_RCC_CR, 0, RCC_CR_HSEON);
            
            for (int i = 0; i < 100; i++) {
                if (getreg32(STM32_RCC_CR) & RCC_CR_HSERDY) {
                    syslog(LOG_INFO, "HSE recovered!");
                    break;
                }
                up_mdelay(1);
            }
        }
        
        while(1) {
            syslog(LOG_INFO, "HALTED-CLK");
            up_mdelay(2000);
        }
        
    } else {
        syslog(LOG_INFO, "Clk: UNKNOWN");
        up_mdelay(2000);
    }
    
    up_mdelay(500);
}

static void setup_i2c_clocks(void)
{
    syslog(LOG_INFO, "[I2C-CLK] Configuring I2C clocks...\n");
    
    uint32_t d2ccip2r = getreg32(STM32_RCC_D2CCIP2R);
    uint32_t d3ccipr = getreg32(STM32_RCC_D3CCIPR);
    
    syslog(LOG_INFO, "[I2C-CLK] Before: D2CCIP2R=0x%08lx D3CCIPR=0x%08lx\n", d2ccip2r, d3ccipr);
    
    /* I2C1/2/3 clock source (bits [21:20] of D2CCIP2R)
     * 00 = rcc_pclk1 (default)
     * 01 = PLL3R
     * 10 = HSI kernel clock (this is what we want)
     * 11 = CSI kernel clock
     * Current value: 0x302000 = bits[21:20] = 11b (CSI) - BAD!
     */
    d2ccip2r &= ~(0x3 << 20);  // Clear bits [21:20]
    d2ccip2r |= (0x2 << 20);   // Set bits [21:20] = 10b (HSI)
    
    /* I2C4 clock source (bits [9:8] of D3CCIPR)
     * 00 = rcc_pclk4 (default)
     * 01 = PLL3R
     * 10 = HSI kernel clock (this is what we want)
     * 11 = CSI kernel clock
     * Current value: 0x200 = bits[9:8] = 10b (HSI) - ALREADY CORRECT!
     */
    d3ccipr &= ~(0x3 << 8);   // Clear bits [9:8]
    d3ccipr |= (0x2 << 8);    // Set bits [9:8] = 10b (HSI)
    
    putreg32(d2ccip2r, STM32_RCC_D2CCIP2R);
    putreg32(d3ccipr, STM32_RCC_D3CCIPR);
    
    // Read back to verify
    d2ccip2r = getreg32(STM32_RCC_D2CCIP2R);
    d3ccipr = getreg32(STM32_RCC_D3CCIPR);
    
    syslog(LOG_INFO, "[I2C-CLK] After: D2CCIP2R=0x%08lx D3CCIPR=0x%08lx\n", d2ccip2r, d3ccipr);
    
    // Verify the change
    uint32_t i2c123_sel = (d2ccip2r >> 20) & 0x3;
    uint32_t i2c4_sel = (d3ccipr >> 8) & 0x3;
    
    syslog(LOG_INFO, "[I2C-CLK] I2C1/2/3 source: %s (%lu)\n", 
           i2c123_sel == 2 ? "HSI" : "OTHER", i2c123_sel);
    syslog(LOG_INFO, "[I2C-CLK] I2C4 source: %s (%lu)\n", 
           i2c4_sel == 2 ? "HSI" : "OTHER", i2c4_sel);
}

/////////////////////////////////////////////////////////////////////////////////
static void dump_spi2_registers(const char *label)
{
    syslog(LOG_INFO, "[%s] === SPI2 DUMP ===\n", label);
    syslog(LOG_INFO, "  CR1    = 0x%08lx\n", getreg32(STM32_SPI2_BASE + STM32_SPI_CR1_OFFSET));
    syslog(LOG_INFO, "  CR2    = 0x%08lx\n", getreg32(STM32_SPI2_BASE + 0x04));
    syslog(LOG_INFO, "  CFG1   = 0x%08lx\n", getreg32(STM32_SPI2_BASE + STM32_SPI_CFG1_OFFSET));
    syslog(LOG_INFO, "  CFG2   = 0x%08lx\n", getreg32(STM32_SPI2_BASE + STM32_SPI_CFG2_OFFSET));
    syslog(LOG_INFO, "  SR     = 0x%08lx\n", getreg32(STM32_SPI2_BASE + STM32_SPI_SR_OFFSET));
    syslog(LOG_INFO, "  IER    = 0x%08lx\n", getreg32(STM32_SPI2_BASE + 0x18));
    
    uint32_t gpiob_moder = getreg32(STM32_GPIOB_BASE + STM32_GPIO_MODER_OFFSET);
    uint32_t gpiob_odr = getreg32(STM32_GPIOB_BASE + STM32_GPIO_ODR_OFFSET);
    uint32_t gpiod_odr = getreg32(STM32_GPIOD_BASE + STM32_GPIO_ODR_OFFSET);
    
    syslog(LOG_INFO, "  GPIOB_MODER = 0x%08lx\n", gpiob_moder);
    syslog(LOG_INFO, "  GPIOB_ODR   = 0x%08lx (PB11_SD=%ld)\n", gpiob_odr, (gpiob_odr >> 11) & 1);
    syslog(LOG_INFO, "  GPIOD_ODR   = 0x%08lx (PD10_FRAM=%ld)\n", gpiod_odr, (gpiod_odr >> 10) & 1);
    syslog(LOG_INFO, "  RCC_APB1LENR = 0x%08lx (SPI2EN=%ld)\n", getreg32(STM32_RCC_APB1LENR), (getreg32(STM32_RCC_APB1LENR) >> 14) & 1);
    
    up_mdelay(50);
}





static void configure_spi2_gpio_clocks(void)
{
    syslog(LOG_INFO, "[GPIO-1] Enabling GPIOB and GPIOD clocks...\n");
    up_mdelay(10);
    
    /* Enable GPIOB clock (for PB10/PB11/PB14/PB15 - SPI2 pins + SD CS) */
    modifyreg32(STM32_RCC_AHB4ENR, 0, RCC_AHB4ENR_GPIOBEN);
    
    /* Enable GPIOD clock (for PD10 - FRAM CS) */
    modifyreg32(STM32_RCC_AHB4ENR, 0, RCC_AHB4ENR_GPIODEN);
    
    up_udelay(10);
    
    syslog(LOG_INFO, "[GPIO-2] Configuring CS pins as outputs...\n");
    up_mdelay(10);
    
    /* Manually configure PD10 (FRAM CS) as GPIO output, initially HIGH */
    px4_arch_configgpio(GPIO_SPI2_CS_FRAM);
    px4_arch_gpiowrite(GPIO_SPI2_CS_FRAM, 1);  // Deselect FRAM
    
    /* Manually configure PB11 (SD CS) as GPIO output, initially HIGH */
    px4_arch_configgpio(GPIO_SPI2_CS_SDCARD);
    px4_arch_gpiowrite(GPIO_SPI2_CS_SDCARD, 1);  // Deselect SD
    
    up_mdelay(10);
    
    /* Verify GPIO configuration */
    uint32_t gpiod_moder = getreg32(STM32_GPIOD_BASE + STM32_GPIO_MODER_OFFSET);
    uint32_t gpiob_moder = getreg32(STM32_GPIOB_BASE + STM32_GPIO_MODER_OFFSET);
    uint32_t gpiod_odr = getreg32(STM32_GPIOD_BASE + STM32_GPIO_ODR_OFFSET);
    uint32_t gpiob_odr = getreg32(STM32_GPIOB_BASE + STM32_GPIO_ODR_OFFSET);
    
    syslog(LOG_INFO, "[GPIO-3] GPIOD_MODER=0x%08lx GPIOD_ODR=0x%08lx (PD10=%ld)\n", gpiod_moder, gpiod_odr, (gpiod_odr >> 10) & 1);
    syslog(LOG_INFO, "[GPIO-4] GPIOB_MODER=0x%08lx GPIOB_ODR=0x%08lx (PB11=%ld)\n", gpiob_moder, gpiob_odr, (gpiob_odr >> 11) & 1);
    
    up_mdelay(10);
}


static struct spi_dev_s *configure_fram_spi2(void)
{
    syslog(LOG_INFO, "[SPI2-INIT-1] Configuring GPIO clocks and CS pins...\n");
    up_mdelay(10);
    
    /* CRITICAL: Configure GPIO BEFORE enabling SPI peripheral */
    configure_spi2_gpio_clocks();
    
    syslog(LOG_INFO, "[SPI2-INIT-2] Enabling SPI2 clock...\n");
    up_mdelay(10);
    
    /* 1. ENABLE CLOCK FIRST */
    modifyreg32(STM32_RCC_APB1LENR, 0, RCC_APB1LENR_SPI2EN);
    up_udelay(20);
    
    syslog(LOG_INFO, "[SPI2-INIT-3] Pre-reset SR: 0x%08lx\n", 
           getreg32(STM32_SPI2_BASE + STM32_SPI_SR_OFFSET));
    up_mdelay(10);
    
    /* 2. HARDWARE RESET */
    syslog(LOG_INFO, "[SPI2-INIT-4] Forcing hardware reset...\n");
    up_mdelay(10);
    
    modifyreg32(STM32_RCC_APB1LRSTR, 0, RCC_APB1LRSTR_SPI2RST);
    up_udelay(100);
    modifyreg32(STM32_RCC_APB1LRSTR, RCC_APB1LRSTR_SPI2RST, 0);
    up_udelay(100);
    
    /* 3. VERIFY RESET */
    uint32_t sr_after = getreg32(STM32_SPI2_BASE + STM32_SPI_SR_OFFSET);
    syslog(LOG_INFO, "[SPI2-INIT-5] Post-reset SR: 0x%08lx %s\n", sr_after,
           (sr_after & (1 << 11)) ? "SUSP FAIL" : "SUSP OK");
    up_mdelay(10);
    
    if (sr_after & (1 << 11)) {
        syslog(LOG_ERR, "[SPI2-INIT-FATAL] Reset failed\n");
        return NULL;
    }
    
    /* 4. MEMORY BARRIERS */
    __asm__ __volatile__ ("dsb" : : : "memory");
    __asm__ __volatile__ ("isb" : : : "memory");
    up_mdelay(10);
    
    syslog(LOG_INFO, "[SPI2-INIT-6] Calling stm32_spibus_initialize(2)...\n");
    up_mdelay(10);
	
	
	
/* Clear persistent SUSP flag - critical for H7 SPI after reset */
modifyreg32(STM32_SPI2_BASE + STM32_SPI_CR1_OFFSET, SPI_CR1_SPE, 0);  // Ensure disabled
putreg32((1 << 11), STM32_SPI2_BASE + STM32_SPI_IFCR_OFFSET);  // Clear SUSPC bit directly
up_udelay(20);

/* Verify */
uint32_t sr = getreg32(STM32_SPI2_BASE + STM32_SPI_SR_OFFSET);
syslog(LOG_INFO, "[SPI2] After IFCR SUSPC clear SR=0x%08lx %s\n", sr,
       (sr & (1 << 11)) ? "(SUSP STILL SET!)" : "(SUSP cleared - good)");
	
	
	
	
	
	
    
    /* 5. INITIALIZE BUS */
    struct spi_dev_s *spi2 = stm32_spibus_initialize(2);
    if (!spi2) {
        syslog(LOG_ERR, "[SPI2-INIT-FATAL] spibus_initialize returned NULL\n");
        return NULL;
    }
    
    syslog(LOG_INFO, "[SPI2-INIT-7] Bus init OK, spi2=%p\n", spi2);
    up_mdelay(10);
    
    /* 6. CONFIGURE */
    syslog(LOG_INFO, "[SPI2-INIT-8] Configuring SPI2...\n");
    up_mdelay(10);
    
    SPI_LOCK(spi2, true);
    syslog(LOG_INFO, "[SPI2-INIT-9] Lock acquired\n");
    up_mdelay(10);
    
    SPI_SETMODE(spi2, SPIDEV_MODE0);
    syslog(LOG_INFO, "[SPI2-INIT-10] Mode set\n");
    up_mdelay(10);
    
    SPI_SETBITS(spi2, 8);
    syslog(LOG_INFO, "[SPI2-INIT-11] Bits set\n");
    up_mdelay(10);
    
    SPI_SETFREQUENCY(spi2, 400000);
    syslog(LOG_INFO, "[SPI2-INIT-12] Freq set\n");
    up_mdelay(10);
    
    SPI_LOCK(spi2, false);
    syslog(LOG_INFO, "[SPI2-INIT-13] Lock released\n");
    up_mdelay(10);
    
    dump_spi2_registers("POST-INIT");
    
    syslog(LOG_INFO, "[SPI2-INIT-COMPLETE] Success\n");
    return spi2;
}
////////////////////////////////////////////////////////////////////////////////
/* --- Debug: inspect spi ops/select pointer --- */



/* Replace these ranges with your platform's flash/RAM ranges if different */
#ifndef FLASH_CODE_START
#  define FLASH_CODE_START 0x08000000UL
#  define FLASH_CODE_END   0x08200000UL   /* 2MB flash window (adjust if needed) */
#endif

#ifndef SRAM_CODE_START
#  define SRAM_CODE_START  0x24000000UL
#  define SRAM_CODE_END    0x24040000UL   /* small SRAM region used for code on some builds */
#endif

static inline bool is_likely_code_address(uintptr_t addr)
{
    if (addr == 0) {
        return false;
    }

    /* Check common flash and SRAM code ranges; extend if your map differs */
    if ((addr >= FLASH_CODE_START && addr < FLASH_CODE_END) ||
        (addr >= SRAM_CODE_START  && addr < SRAM_CODE_END)) {
        return true;
    }

    return false;
}

static bool verify_fram_spi2(struct spi_dev_s *spi2)
{
    uint8_t id[FRAM_ID_LEN] = {0};

    if (!spi2) {
        syslog(LOG_ERR, "[FRAM-DBG] spi2 is NULL\n");
        return false;
    }

    /* ops is const in the spi_dev_s definition */
    const struct spi_ops_s *ops = spi2->ops;

    syslog(LOG_INFO, "[FRAM-DBG] verify_fram_spi2 entry spi2=%p\n", spi2);
    syslog(LOG_INFO, "[FRAM-DBG] ops=%p\n", ops);

    if (!ops) {
        syslog(LOG_ERR, "[FRAM-DBG] spi2->ops is NULL\n");
        return false;
    }

    /* Print the key callbacks and their addresses for addr2line / nm */
    syslog(LOG_INFO, "[FRAM-DBG] ops->lock      = %p\n", (const void *)ops->lock);
    syslog(LOG_INFO, "[FRAM-DBG] ops->select    = %p\n", (const void *)ops->select);
    syslog(LOG_INFO, "[FRAM-DBG] ops->status    = %p\n", (const void *)ops->status);
    syslog(LOG_INFO, "[FRAM-DBG] ops->setfrequency = %p\n", (const void *)ops->setfrequency);
    syslog(LOG_INFO, "[FRAM-DBG] ops->setmode   = %p\n", (const void *)ops->setmode);
    syslog(LOG_INFO, "[FRAM-DBG] ops->setbits   = %p\n", (const void *)ops->setbits);

    /* Sanity checks */
    if (!ops->status) {
        syslog(LOG_ERR, "[FRAM-DBG] ops->status is NULL\n");
        return false;
    }

    /* Query device status (some drivers implement this) */
    int st = ops->status(spi2, SPIDEV_FLASH(0));
    syslog(LOG_INFO, "[FRAM-DBG] ops->status(spi2, SPIDEV_FLASH(0)) = 0x%02x\n", st);

    /* Lock the bus */
    if (ops->lock) {
        int lock_ret = ops->lock(spi2, true);
        syslog(LOG_INFO, "[FRAM-DBG] calling ops->lock(spi2, true) -> %d\n", lock_ret);
    } else {
        syslog(LOG_INFO, "[FRAM-DBG] ops->lock is NULL (continuing)\n");
    }

    /* Dump SPI registers and GPIO state before asserting CS */
    dump_spi2_registers("BEFORE-CS-ASSERT");

    /* Validate select callback pointer before calling it */
    if (!ops->select) {
        syslog(LOG_ERR, "[FRAM-DBG] ops->select is NULL - cannot assert CS\n");
        if (ops->lock) ops->lock(spi2, false);
        return false;
    }

    /* Print the select pointer as integer for addr2line convenience */
    syslog(LOG_INFO, "[FRAM-DBG] About to call ops->select(spi2, SPIDEV_FLASH(0), true) -> %p\n",
           (const void *)ops->select);

    /* Call the select callback to assert CS */
    /* NOTE: select prototype is: void (*select)(FAR struct spi_dev_s *dev, uint32_t devid, bool selected) */
    ops->select(spi2, SPIDEV_FLASH(0), true);

    /* Small delay to let hardware settle */
    up_udelay(10);

    /* Dump registers after CS asserted */
    dump_spi2_registers("AFTER-CS-ASSERT");

    /* Send RDID command and read ID */
    SPI_SEND(spi2, FRAM_RDID_CMD);
    SPI_RECVBLOCK(spi2, id, FRAM_ID_LEN);

    /* Deassert CS */
    ops->select(spi2, SPIDEV_FLASH(0), false);

    /* Unlock the bus */
    if (ops->lock) {
        ops->lock(spi2, false);
    }

    syslog(LOG_INFO, "[FRAM-DBG] FRAM ID: ");
    for (int i = 0; i < FRAM_ID_LEN; i++) {
        syslog(LOG_INFO, "%02X ", id[i]);
    }
    syslog(LOG_INFO, "\n");

    /* Check for Cypress/Ramtron ID (C2 21) */
    if (id[6] == 0xC2 && id[7] == 0x21) {
        syslog(LOG_INFO, "[FRAM-DBG] ID match - FRAM OK\n");
        return true;
    }

    syslog(LOG_ERR, "[FRAM-DBG] ID mismatch or read failed\n");
    return false;
}

static bool mount_fram_storage(struct spi_dev_s *spi2)
{
    syslog(LOG_INFO, "[MTD-1] Calling ramtron_initialize...\n");
    
    FAR struct mtd_dev_s *mtd = ramtron_initialize(spi2);
    if (!mtd) {
        syslog(LOG_ERR, "[MTD-FATAL] ramtron_initialize failed\n");
        return false;
    }
    
    syslog(LOG_INFO, "[MTD-2] ramtron_initialize OK, mtd=%p\n", mtd);
    
    /* Register MTD device */
    int ret = register_mtddriver("/dev/mtd0", mtd, 0755, NULL);
    if (ret < 0) {
        syslog(LOG_ERR, "[MTD-ERR] register_mtddriver failed: %d\n", ret);
        /* Continue anyway - PX4 param system can work with just /dev/mtd0 */
    } else {
        syslog(LOG_INFO, "[MTD-3] MTD registered as /dev/mtd0\n");
    }
    
    /* Try to mount NXFFS directly on MTD (without BCH) */
    ret = nx_mount("/dev/mtd0", "/fs/mtd_params", "nxffs", 0, NULL);
    if (ret < 0) {
        syslog(LOG_INFO, "[MTD-4] Direct mount failed (%d), trying format...\n", ret);
        
        ret = nx_mount("/dev/mtd0", "/fs/mtd_params", "nxffs", 0, "forceformat");
        if (ret < 0) {
            syslog(LOG_KERN, "[MTD-5] NXFFS mount failed: %d\n", ret);
            syslog(LOG_INFO, "[MTD-6] PX4 can still use /dev/mtd0 directly for params\n");
            return true;  /* Not fatal - MTD device is available */
        }
    }
    
    syslog(LOG_INFO, "[MTD-SUCCESS] FRAM storage ready at /fs/mtd_params\n");
    return true;
}


void stm32_boardinitialize(void)
{
    board_on_reset(-1);
    const uint32_t gpio[] = PX4_GPIO_INIT_LIST;
    px4_gpio_init(gpio, arraySize(gpio));
    
    board_control_spi_sensors_power_configgpio();
    clock_debug();
    setup_i2c_clocks();
    // *** CRITICAL: DO NOT initialize USB here! ***
    // USB will be initialized AFTER CDC-ACM driver is ready
    
    syslog(LOG_INFO, "stm32_boardinitialize done (USB deferred to late init)\n");
}





int board_app_initialize(uintptr_t arg)
{
    char stack_marker;
    syslog(LOG_INFO, "[BOOT-1] PixEagle Init, SP=%p\n", &stack_marker);
    
    // DMA init
    if (board_dma_alloc_init() < 0) {        syslog(LOG_ERR, "[BOOT-ERR] DMA init failed\n");    }
    
    // Sensor power
    syslog(LOG_INFO, "[BOOT-2] Enabling sensor power...\n");
    VDD_3V3_SENSORS_EN(true);
    board_control_spi_sensors_power(true, 0xffff);
    up_udelay(50000);
    
    // Initialize ALL SPI buses
    syslog(LOG_INFO, "[BOOT-3] Initializing SPI buses...\n");
    stm32_spiinitialize();
    
    // FRAM on SPI2
    syslog(LOG_INFO, "[BOOT-4] Configuring FRAM...\n");
    struct spi_dev_s *spi2 = configure_fram_spi2();
    
    if (spi2 && verify_fram_spi2(spi2)) {
        syslog(LOG_INFO, "[BOOT-5] FRAM OK, attempting mount...\n");
        mount_fram_storage(spi2);  // Non-fatal if it fails
    } 
	else         syslog(LOG_KERN, "[BOOT-5] FRAM not available (will use defaults)\n");
     
    
    // SD Card (also on SPI2)
    syslog(LOG_INFO, "[BOOT-6] Initializing SD card...\n");
    if (spi2) {        mmcsd_spislotinitialize(0, 0, spi2);    }
    
    // *** USB INITIALIZATION - MOVED HERE ***
    syslog(LOG_INFO, "[BOOT-7] Configuring USB (late init)...\n");
    setup_usb();           // Configure HSI48 + CRS
    
    syslog(LOG_INFO, "[BOOT-8] Starting USB device stack...\n");
    stm32_usbinitialize(); // Initialize USB hardware
    
    // Give USB time to stabilize before CDC-ACM registration
    up_mdelay(50);
    
    // Start PX4 platform (this registers CDC-ACM class driver)
    syslog(LOG_INFO, "[BOOT-9] Starting PX4 platform...\n");
	
	syslog(LOG_INFO, "[MEM-DBG] MPU_CTRL=0x%08lx\n", getreg32(0xE000ED94));
syslog(LOG_INFO, "[MEM-DBG] SCB_CACR=0x%08lx\n", getreg32(0xE000ED84));
syslog(LOG_INFO, "[MEM-DBG] AXI_SRAM @ 0x24000000 test...\n");

// Test write-read to AXI SRAM with cache flush
volatile uint32_t *test_ptr = (uint32_t *)0x24000000;
*test_ptr = 0xDEADBEEF;
__asm__ __volatile__ ("dsb" ::: "memory");
__asm__ __volatile__ ("isb" ::: "memory");

syslog(LOG_INFO, "[MEM-DBG] Wrote 0xDEADBEEF, read back 0x%08lx\n", *test_ptr);


syslog(LOG_INFO, "[WQ-DEBUG] About to start px4_platform_init()\n");







	
    px4_platform_init();
    
    syslog(LOG_INFO, "[NSH-DEBUG] Checking NSH configuration...\n");
    
    // Check if console is open
    int console_fd = open("/dev/console", O_RDWR);
    if (console_fd < 0) {        syslog(LOG_ERR, "[NSH-DEBUG] CRITICAL: Cannot open /dev/console! errno=%d\n", errno);    } 
	else {        syslog(LOG_INFO, "[NSH-DEBUG] Console open OK (fd=%d)\n", console_fd);        close(console_fd);    }
    
    // Check if UART4 is the console
    int uart4_fd = open("/dev/ttyS3", O_RDWR); // UART4 = ttyS3
    if (uart4_fd < 0) {        syslog(LOG_ERR, "[NSH-DEBUG] Cannot open /dev/ttyS3 (UART4)! errno=%d\n", errno);    } 
	else 			  {        syslog(LOG_INFO, "[NSH-DEBUG] UART4 (/dev/ttyS3) open OK (fd=%d)\n", uart4_fd); 
						// Try to write directly to UART4
						const char *test_msg = "\r\n[NSH-TEST] Direct UART4 write test\r\n";
						ssize_t written = write(uart4_fd, test_msg, strlen(test_msg));
						syslog(LOG_INFO, "[NSH-DEBUG] Wrote %d bytes to UART4\n", (int)written);
						
						close(uart4_fd);
					}
    
    // Manually trigger NSH (for testing)
    syslog(LOG_INFO, "[NSH-DEBUG] Attempting manual NSH start...\n");
    
    // Give system time to settle
    up_mdelay(500);
    
    syslog(LOG_INFO, "[NSH-DEBUG] board_app_initialize() complete, returning to kernel\n");
    
    return OK;
}





void board_on_reset(int status)
{
    for (int i = 0; i < DIRECT_PWM_OUTPUT_CHANNELS; ++i) {
        px4_arch_configgpio(PX4_MAKE_GPIO_INPUT(io_timer_channel_get_as_pwm_input(i)));
    }
    if (status >= 0) {
        up_mdelay(100);
    }
}

void board_peripheral_reset(int ms)
{
    VDD_3V3_SENSORS_EN(false);
    board_control_spi_sensors_power(false, 0xffff);
    usleep(ms * 1000);
    board_control_spi_sensors_power(true, 0xffff);
    VDD_3V3_SENSORS_EN(true);
}

#ifndef CONFIG_NET
void arm_netinitialize(void) {}
#endif
