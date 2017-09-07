/*
 * Copyright (c) 2016 Piotr Mienkowski
 * SPDX-License-Identifier: Apache-2.0
 */

/** @file
 * @brief USART driver for Atmel SAM MCU family.
 *
 * Note:
 * - Only basic USART features sufficient to support printf functionality
 *   are currently implemented.
 * - The driver works only in polling mode, interrupt mode is not implemented.
 */

#include <errno.h>
#include <misc/__assert.h>
#include <device.h>
#include <init.h>
#include <soc.h>
#include <uart.h>

/*
 * Verify Kconfig configuration
 */

#if CONFIG_USART_SAM_PORT_0 == 1

#if CONFIG_USART_SAM_PORT_0_BAUD_RATE == 0
#error "CONFIG_USART_SAM_PORT_0_BAUD_RATE has to be bigger than 0"
#endif

#endif

#if CONFIG_USART_SAM_PORT_1 == 1

#if CONFIG_USART_SAM_PORT_1_BAUD_RATE == 0
#error "CONFIG_USART_SAM_PORT_1_BAUD_RATE has to be bigger than 0"
#endif

#endif

#if CONFIG_USART_SAM_PORT_2 == 1

#if CONFIG_USART_SAM_PORT_2_BAUD_RATE == 0
#error "CONFIG_USART_SAM_PORT_2_BAUD_RATE has to be bigger than 0"
#endif

#endif

/* Device constant configuration parameters */
struct usart_sam_dev_cfg {
	Usart *regs;
	u32_t periph_id;
	struct soc_gpio_pin pin_rx;
	struct soc_gpio_pin pin_tx;
};

/* Device run time data */
struct usart_sam_dev_data {
	u32_t baud_rate;
};

#define DEV_CFG(dev) \
	((const struct usart_sam_dev_cfg *const)(dev)->config->config_info)
#define DEV_DATA(dev) \
	((struct usart_sam_dev_data *const)(dev)->driver_data)


static int baudrate_set(Usart *const usart, u32_t baudrate,
			u32_t mck_freq_hz);


static int usart_sam_init(struct device *dev)
{
	int retval;
	const struct usart_sam_dev_cfg *const cfg = DEV_CFG(dev);
	struct usart_sam_dev_data *const dev_data = DEV_DATA(dev);
	Usart *const usart = cfg->regs;

	/* Enable USART clock in PMC */
	soc_pmc_peripheral_enable(cfg->periph_id);

	/* Connect pins to the peripheral */
	soc_gpio_configure(&cfg->pin_rx);
	soc_gpio_configure(&cfg->pin_tx);

	/* Reset and disable USART */
	usart->US_CR =   US_CR_RSTRX | US_CR_RSTTX
		       | US_CR_RXDIS | US_CR_TXDIS | US_CR_RSTSTA;

	/* Disable Interrupts */
	usart->US_IDR = 0xFFFFFFFF;

	/* 8 bits of data, no parity, 1 stop bit in normal mode */
	usart->US_MR =   US_MR_NBSTOP_1_BIT
		       | US_MR_PAR_NO
		       | US_MR_CHRL_8_BIT
		       | US_MR_USCLKS_MCK
		       | US_MR_CHMODE_NORMAL;

	/* Set baud rate */
	retval = baudrate_set(usart, dev_data->baud_rate,
			      SOC_ATMEL_SAM_MCK_FREQ_HZ);
	if (retval != 0) {
		return retval;
	};

	/* Enable receiver and transmitter */
	usart->US_CR = US_CR_RXEN | US_CR_TXEN;

	return 0;
}

static int usart_sam_poll_in(struct device *dev, unsigned char *c)
{
	Usart *const usart = DEV_CFG(dev)->regs;

	if (!(usart->US_CSR & US_CSR_RXRDY)) {
		return -EBUSY;
	}

	/* got a character */
	*c = (unsigned char)usart->US_RHR;

	return 0;
}

static unsigned char usart_sam_poll_out(struct device *dev, unsigned char c)
{
	Usart *const usart = DEV_CFG(dev)->regs;

	/* Wait for transmitter to be ready */
	while (!(usart->US_CSR & US_CSR_TXRDY))
		;

	/* send a character */
	usart->US_THR = (u32_t)c;
	return c;
}

static int baudrate_set(Usart *const usart, u32_t baudrate,
			u32_t mck_freq_hz)
{
	u32_t divisor;

	__ASSERT(baudrate,
		 "baud rate has to be bigger than 0");
	__ASSERT(mck_freq_hz/16 >= baudrate,
		 "MCK frequency is too small to set required baud rate");

	divisor = mck_freq_hz / 16 / baudrate;

	if (divisor > 0xFFFF) {
		return -EINVAL;
	};

	usart->US_BRGR = US_BRGR_CD(divisor);

	return 0;
}

static const struct uart_driver_api usart_sam_driver_api = {
	.poll_in = usart_sam_poll_in,
	.poll_out = usart_sam_poll_out,
};


#define USART_X_CONFIG_STRUCT(x)	usart ## x ## _sam_config
#define USART_X_DATA(x)				usart ## x ## _sam_data
#define USART_REGISTER(x)			USART ## x
#define USART_PERIPH_ID(x)			ID_USART ## x
#define USART_RX_PIN(x)				PIN_USART ## x ## _RXD
#define USART_TX_PIN(x)				PIN_USART ## x ## _TXD

#define USART_CONFIG(x)															\
static const struct usart_sam_dev_cfg USART_X_CONFIG_STRUCT(x) = {				\
	.regs = USART_REGISTER(x),													\
	.periph_id = USART_PERIPH_ID(x),											\
	.pin_rx = USART_RX_PIN(x),													\
	.pin_tx = USART_TX_PIN(x)													\
};																				\
static struct usart_sam_dev_data USART_X_DATA(x) = {							\
	.baud_rate = CONFIG_USART_SAM_PORT_ ## x ## _BAUD_RATE						\
};																				\
DEVICE_AND_API_INIT(usart ## x ## _sam, CONFIG_USART_SAM_PORT_ ## x ## _NAME, 	\
	&usart_sam_init, &USART_X_DATA(x), &USART_X_CONFIG_STRUCT(x), 				\
	PRE_KERNEL_1, CONFIG_KERNEL_INIT_PRIORITY_DEVICE, &usart_sam_driver_api);	\



/*
#define USART_CONFIG(x)															\
static const struct usart_sam_dev_cfg usart ## x ## _sam_config = {				\
	.regs = USART ## x,															\
	.periph_id = ID_USART ## x,													\
	.pin_rx = PIN_USART ## x ## _RXD,											\
	.pin_tx = PIN_USART ## x ## _TXD											\
};																				\
static struct usart_sam_dev_data usart ## x ## _sam_data = {					\
	.baud_rate = CONFIG_USART_SAM_PORT_ ## x ## _BAUD_RATE						\
};																				\
DEVICE_AND_API_INIT(usart ## x ## _sam, CONFIG_USART_SAM_PORT_ ## x ## _NAME, 	\
	&usart_sam_init, &usart ## x ## _sam_data, &usart ## x ## _sam_config, 		\
	PRE_KERNEL_1, CONFIG_KERNEL_INIT_PRIORITY_DEVICE, &usart_sam_driver_api);	\
*/

/* USART0 */
#if CONFIG_USART_SAM_PORT_0
#if CONFIG_USART_SAM_PORT_0_BAUD_RATE < 2400
#error "CONFIG_USART_SAM_PORT_0_BAUD_RATE has to be bigger than 0"
#endif
USART_CONFIG(0)
#endif

/* USART1 */
#if CONFIG_USART_SAM_PORT_1
#if CONFIG_USART_SAM_PORT_1_BAUD_RATE < 2400
#error "CONFIG_USART_SAM_PORT_1_BAUD_RATE has to be bigger than 0"
#endif
USART_CONFIG(1)
#endif

/* USART2 */
#if CONFIG_USART_SAM_PORT_2
#if CONFIG_USART_SAM_PORT_2_BAUD_RATE < 2400
#error "CONFIG_USART_SAM_PORT_2_BAUD_RATE has to be bigger than 0"
#endif
USART_CONFIG(2)
#endif

/* USART3 */
#if CONFIG_USART_SAM_PORT_3
#if CONFIG_USART_SAM_PORT_3_BAUD_RATE < 2400
#error "CONFIG_USART_SAM_PORT_3_BAUD_RATE has to be bigger than 0"
#endif
USART_CONFIG(3)
#endif

