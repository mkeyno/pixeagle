#include <nuttx/config.h>

#ifdef CONFIG_STM32H7_OTGFS

#include <stdint.h>
#include <arch/board/board.h>
#include "stm32_gpio.h"

/* Raw register access – the only thing available in bootloader */
#define RCC_BASE        0x58024400UL
#define PWR_BASE        0x58024800UL
#define UART4_BASE      0x40004C00UL
#define CRS_BASE        0x40006000UL

#define RCC_CR          (*(volatile uint32_t *)(RCC_BASE + 0x000))
#define RCC_APB4ENR     (*(volatile uint32_t *)(RCC_BASE + 0x0E8))
#define RCC_APB1LENR    (*(volatile uint32_t *)(RCC_BASE + 0x0E4))
#define RCC_APB1HENR    (*(volatile uint32_t *)(RCC_BASE + 0x0EC))
#define RCC_AHB1ENR     (*(volatile uint32_t *)(RCC_BASE + 0x0D8))
#define RCC_AHB1RSTR    (*(volatile uint32_t *)(RCC_BASE + 0x0C8))
#define RCC_D2CCIP2R    (*(volatile uint32_t *)(RCC_BASE + 0x0B0))

#define PWR_CR3         (*(volatile uint32_t *)(PWR_BASE + 0x0C))
#define CRS_CR          (*(volatile uint32_t *)(CRS_BASE + 0x00))

#define UART4_CR1       (*(volatile uint32_t *)(UART4_BASE + 0x00))
#define UART4_BRR       (*(volatile uint32_t *)(UART4_BASE + 0x0C))
#define UART4_ISR       (*(volatile uint32_t *)(UART4_BASE + 0x1C))
#define UART4_TDR       (*(volatile uint32_t *)(UART4_BASE + 0x28))

static void debug_putc(char c)
{
    while (!(UART4_ISR & (1U << 7)));
    UART4_TDR = (uint32_t)c;
}

static void debug_puts(const char *s)
{
    while (*s) {
        if (*s == '\n') debug_putc('\r');
        debug_putc(*s++);
    }
}

static void uart4_init(void)
{
    RCC_APB1LENR |= (1U << 19);           /* UART4EN */
    stm32_configgpio(GPIO_UART4_TX);
    stm32_configgpio(GPIO_UART4_RX);
    UART4_BRR = 1042;
    UART4_CR1 = (1U << 3) | (1U << 0);     /* TE + UE */
    for (volatile int i = 0; i < 10000; i++) ;
}

void stm32_usbinitialize(void)
{
    uart4_init();

    debug_puts("\r\n\r\n========================================\r\n");
    debug_puts("Pixeagle Bootloader USB Init\r\n");
    debug_puts("========================================\r\n");

    RCC_APB4ENR |= (1U << 28);            /* PWR clock */
    debug_puts("[1] PWR clock OK\r\n");

    PWR_CR3 |= (1U << 24);                /* USB33DEN */
    debug_puts("[2] USB33DEN OK\r\n");

    RCC_CR |= (1U << 12);                 /* HSI48ON */
    int t = 200000;
    while (!(RCC_CR & (1U << 13)) && --t);
    debug_puts("[3] HSI48 "); debug_puts(t ? "ready\r\n" : "TIMEOUT\r\n");

    RCC_APB1HENR |= (1U << 1);            /* CRSEN */
    CRS_CR = (1U << 5) | (1U << 3);
    debug_puts("[4] CRS OK\r\n");

    RCC_D2CCIP2R = (RCC_D2CCIP2R & ~(3U << 20)) | (3U << 20);
    debug_puts("[5] USB clock = HSI48 OK\r\n");

    RCC_AHB1ENR |= (1U << 30);            /* OTGFS clock */
    debug_puts("[6] OTGFS clock OK\r\n");

    RCC_AHB1RSTR |= (1U << 30);
    for (volatile int i = 0; i < 1000; i++) ;
    RCC_AHB1RSTR &= ~(1U << 30);
    debug_puts("[7] OTGFS reset OK\r\n");

    stm32_configgpio(GPIO_OTGFS_VBUS);
    stm32_configgpio(GPIO_OTGFS_DM);
    stm32_configgpio(GPIO_OTGFS_DP);
    debug_puts("[8] USB pins OK\r\n");

    debug_puts("\r\nUSB INIT COMPLETE – plug USB now\r\n");
    debug_puts("========================================\r\n\r\n");
}

/* usbdev_s is not visible in bootloader → use void* */
void stm32_usbsuspend(void *dev, bool resume)
{
    (void)dev;
    debug_puts(resume ? "[USB] RESUME\r\n" : "[USB] SUSPEND\r\n");
}

#endif /* CONFIG_STM32H7_OTGFS */
