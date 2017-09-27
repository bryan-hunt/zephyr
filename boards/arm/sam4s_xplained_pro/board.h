/*
 * Copyright (c) 2017 Microchip Technology Inc
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __INC_BOARD_H
#define __INC_BOARD_H

#include <soc.h>

#ifndef __ASSEMBLER__

#define LED0_GPIO_PORT	"GPIO_C"
#define LED0_GPIO_PIN	23


#if defined(CONFIG_WIFI_WINC1500)

/* GPIO numbers where the WINC1500 module is connected to */
#define CONFIG_WINC1500_GPIO_CHIP_EN 6 /* AP_GPIO6_ADC6 EXTERNAL_PAD_6 Out    */
#define CONFIG_WINC1500_GPIO_WAKE    5 /* AP_GPIO5_ADC5 EXTERNAL_PAD_5 Out    */
#define CONFIG_WINC1500_GPIO_IRQN    4 /* AP_GPIO4_ADC4 EXTERNAL_PAD_4 In Irq */
#define CONFIG_WINC1500_GPIO_RESET_N 0 /* AP_GPIO_AON0  AON_GPIO_PAD_0 Out    */

typedef enum _winc1500_gpio_index {
	/* If all the GPIOs can be served by same driver, then you
	 * can set the values to be the same. The first enum should
	 * always have a value of 0.
	 */
	WINC1500_GPIO_IDX_CHIP_EN = 0,
	WINC1500_GPIO_IDX_WAKE    = 0,
	WINC1500_GPIO_IDX_IRQN    = 0,

	WINC1500_GPIO_IDX_RESET_N = 1,

	WINC1500_GPIO_IDX_LAST_ENTRY
} winc1500_gpio_index;

struct device **winc1500_configure_gpios(void);
void winc1500_configure_intgpios(void);

#endif /* CONFIG_WIFI_WINC1500 */

#endif /* __ASSEMBLER__ */

#endif /* __INC_BOARD_H */
