#ifndef __RISCV64_PLIC_H
#define __RISCV64_PLIC_H 

/**
 * @brief       PLIC External Interrupt Numbers
 *
 * @note        PLIC interrupt sources
 *
 * | Source | Name                     | Description                        |
 * |--------|--------------------------|------------------------------------|
 * | 0      | IRQN_NO_INTERRUPT        | The non-existent interrupt         |
 * | 1      | IRQN_SPI0_INTERRUPT      | SPI0 interrupt                     |
 * | 2      | IRQN_SPI1_INTERRUPT      | SPI1 interrupt                     |
 * | 3      | IRQN_SPI_SLAVE_INTERRUPT | SPI_SLAVE interrupt                |
 * | 4      | IRQN_SPI3_INTERRUPT      | SPI3 interrupt                     |
 * | 5      | IRQN_I2S0_INTERRUPT      | I2S0 interrupt                     |
 * | 6      | IRQN_I2S1_INTERRUPT      | I2S1 interrupt                     |
 * | 7      | IRQN_I2S2_INTERRUPT      | I2S2 interrupt                     |
 * | 8      | IRQN_I2C0_INTERRUPT      | I2C0 interrupt                     |
 * | 9      | IRQN_I2C1_INTERRUPT      | I2C1 interrupt                     |
 * | 10     | IRQN_I2C2_INTERRUPT      | I2C2 interrupt                     |
 * | 11     | IRQN_UART1_INTERRUPT     | UART1 interrupt                    |
 * | 12     | IRQN_UART2_INTERRUPT     | UART2 interrupt                    |
 * | 13     | IRQN_UART3_INTERRUPT     | UART3 interrupt                    |
 * | 14     | IRQN_TIMER0A_INTERRUPT   | TIMER0 channel 0 or 1 interrupt    |
 * | 15     | IRQN_TIMER0B_INTERRUPT   | TIMER0 channel 2 or 3 interrupt    |
 * | 16     | IRQN_TIMER1A_INTERRUPT   | TIMER1 channel 0 or 1 interrupt    |
 * | 17     | IRQN_TIMER1B_INTERRUPT   | TIMER1 channel 2 or 3 interrupt    |
 * | 18     | IRQN_TIMER2A_INTERRUPT   | TIMER2 channel 0 or 1 interrupt    |
 * | 19     | IRQN_TIMER2B_INTERRUPT   | TIMER2 channel 2 or 3 interrupt    |
 * | 20     | IRQN_RTC_INTERRUPT       | RTC tick and alarm interrupt       |
 * | 21     | IRQN_WDT0_INTERRUPT      | Watching dog timer0 interrupt      |
 * | 22     | IRQN_WDT1_INTERRUPT      | Watching dog timer1 interrupt      |
 * | 23     | IRQN_APB_GPIO_INTERRUPT  | APB GPIO interrupt                 |
 * | 24     | IRQN_DVP_INTERRUPT       | Digital video port interrupt       |
 * | 25     | IRQN_AI_INTERRUPT        | AI accelerator interrupt           |
 * | 26     | IRQN_FFT_INTERRUPT       | FFT accelerator interrupt          |
 * | 27     | IRQN_DMA0_INTERRUPT      | DMA channel0 interrupt             |
 * | 28     | IRQN_DMA1_INTERRUPT      | DMA channel1 interrupt             |
 * | 29     | IRQN_DMA2_INTERRUPT      | DMA channel2 interrupt             |
 * | 30     | IRQN_DMA3_INTERRUPT      | DMA channel3 interrupt             |
 * | 31     | IRQN_DMA4_INTERRUPT      | DMA channel4 interrupt             |
 * | 32     | IRQN_DMA5_INTERRUPT      | DMA channel5 interrupt             |
 * | 33     | IRQN_UARTHS_INTERRUPT    | Hi-speed UART0 interrupt           |
 * | 34     | IRQN_GPIOHS0_INTERRUPT   | Hi-speed GPIO0 interrupt           |
 * | 35     | IRQN_GPIOHS1_INTERRUPT   | Hi-speed GPIO1 interrupt           |
 * | 36     | IRQN_GPIOHS2_INTERRUPT   | Hi-speed GPIO2 interrupt           |
 * | 37     | IRQN_GPIOHS3_INTERRUPT   | Hi-speed GPIO3 interrupt           |
 * | 38     | IRQN_GPIOHS4_INTERRUPT   | Hi-speed GPIO4 interrupt           |
 * | 39     | IRQN_GPIOHS5_INTERRUPT   | Hi-speed GPIO5 interrupt           |
 * | 40     | IRQN_GPIOHS6_INTERRUPT   | Hi-speed GPIO6 interrupt           |
 * | 41     | IRQN_GPIOHS7_INTERRUPT   | Hi-speed GPIO7 interrupt           |
 * | 42     | IRQN_GPIOHS8_INTERRUPT   | Hi-speed GPIO8 interrupt           |
 * | 43     | IRQN_GPIOHS9_INTERRUPT   | Hi-speed GPIO9 interrupt           |
 * | 44     | IRQN_GPIOHS10_INTERRUPT  | Hi-speed GPIO10 interrupt          |
 * | 45     | IRQN_GPIOHS11_INTERRUPT  | Hi-speed GPIO11 interrupt          |
 * | 46     | IRQN_GPIOHS12_INTERRUPT  | Hi-speed GPIO12 interrupt          |
 * | 47     | IRQN_GPIOHS13_INTERRUPT  | Hi-speed GPIO13 interrupt          |
 * | 48     | IRQN_GPIOHS14_INTERRUPT  | Hi-speed GPIO14 interrupt          |
 * | 49     | IRQN_GPIOHS15_INTERRUPT  | Hi-speed GPIO15 interrupt          |
 * | 50     | IRQN_GPIOHS16_INTERRUPT  | Hi-speed GPIO16 interrupt          |
 * | 51     | IRQN_GPIOHS17_INTERRUPT  | Hi-speed GPIO17 interrupt          |
 * | 52     | IRQN_GPIOHS18_INTERRUPT  | Hi-speed GPIO18 interrupt          |
 * | 53     | IRQN_GPIOHS19_INTERRUPT  | Hi-speed GPIO19 interrupt          |
 * | 54     | IRQN_GPIOHS20_INTERRUPT  | Hi-speed GPIO20 interrupt          |
 * | 55     | IRQN_GPIOHS21_INTERRUPT  | Hi-speed GPIO21 interrupt          |
 * | 56     | IRQN_GPIOHS22_INTERRUPT  | Hi-speed GPIO22 interrupt          |
 * | 57     | IRQN_GPIOHS23_INTERRUPT  | Hi-speed GPIO23 interrupt          |
 * | 58     | IRQN_GPIOHS24_INTERRUPT  | Hi-speed GPIO24 interrupt          |
 * | 59     | IRQN_GPIOHS25_INTERRUPT  | Hi-speed GPIO25 interrupt          |
 * | 60     | IRQN_GPIOHS26_INTERRUPT  | Hi-speed GPIO26 interrupt          |
 * | 61     | IRQN_GPIOHS27_INTERRUPT  | Hi-speed GPIO27 interrupt          |
 * | 62     | IRQN_GPIOHS28_INTERRUPT  | Hi-speed GPIO28 interrupt          |
 * | 63     | IRQN_GPIOHS29_INTERRUPT  | Hi-speed GPIO29 interrupt          |
 * | 64     | IRQN_GPIOHS30_INTERRUPT  | Hi-speed GPIO30 interrupt          |
 * | 65     | IRQN_GPIOHS31_INTERRUPT  | Hi-speed GPIO31 interrupt          |
 *
 */

#include <xbook/hardirq.h>

void plic_init(void);

void plic_enable_irq(irqno_t irqno);

void plic_disable_irq(irqno_t irqno);

// ask PLIC what interrupt we should serve 
int plic_claim(void);

// tell PLIC that we've served this IRQ 
void plic_complete(int irq);

#endif 
