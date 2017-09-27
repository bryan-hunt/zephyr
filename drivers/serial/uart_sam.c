/*
 * Copyright (c) 2017 Piotr Mienkowski
 * SPDX-License-Identifier: Apache-2.0
 */

/** @file
 * @brief UART driver for Atmel SAM MCU family.
 *
 * Note:
 * - Error handling is not implemented.
 * - The driver works only in polling mode, interrupt mode is not implemented.
 */

#include <errno.h>
//#include <misc/__assert.h>
#include <device.h>
#include <init.h>
#include <soc.h>
#include <uart.h>


/* Device constant configuration parameters */
struct uart_sam_dev_cfg {
	Uart *regs;
	u32_t periph_id;
	struct soc_gpio_pin pin_rx;
	struct soc_gpio_pin pin_tx;
};

/* Device run time data */
struct uart_sam_dev_data {
	u32_t baud_rate;
};

#define DEV_CFG(dev) \
	((const struct uart_sam_dev_cfg *const)(dev)->config->config_info)
#define DEV_DATA(dev) \
	((struct uart_sam_dev_data *const)(dev)->driver_data)


static int baudrate_set(Uart *const uart, u32_t baudrate,
			u32_t mck_freq_hz);


static int uart_sam_init(struct device *dev)
{
	int retval;
	const struct uart_sam_dev_cfg *const cfg = DEV_CFG(dev);
	struct uart_sam_dev_data *const dev_data = DEV_DATA(dev);
	Uart *const uart = cfg->regs;

	/* Enable UART clock in PMC */
	soc_pmc_peripheral_enable(cfg->periph_id);

	/* Connect pins to the peripheral */
	soc_gpio_configure(&cfg->pin_rx);
	soc_gpio_configure(&cfg->pin_tx);

	/* Reset and disable UART */
	uart->UART_CR =   UART_CR_RSTRX | UART_CR_RSTTX
			| UART_CR_RXDIS | UART_CR_TXDIS | UART_CR_RSTSTA;

	/* Disable Interrupts */
	uart->UART_IDR = 0xFFFFFFFF;

	/* 8 bits of data, no parity, 1 stop bit in normal mode,  baud rate
	 * driven by the peripheral clock, UART does not filter the receive line
	 */
	uart->UART_MR =   UART_MR_PAR_NO
			| UART_MR_CHMODE_NORMAL;

	/* Set baud rate */
	retval = baudrate_set(uart, dev_data->baud_rate,
			      SOC_ATMEL_SAM_MCK_FREQ_HZ);
	if (retval != 0) {
		return retval;
	};

	/* Enable receiver and transmitter */
	uart->UART_CR = UART_CR_RXEN | UART_CR_TXEN;

	return 0;
}

static int uart_sam_poll_in(struct device *dev, unsigned char *c)
{
	Uart *const uart = DEV_CFG(dev)->regs;

	if (!(uart->UART_SR & UART_SR_RXRDY)) {
		return -EBUSY;
	}

	/* got a character */
	*c = (unsigned char)uart->UART_RHR;

	return 0;
}

static unsigned char uart_sam_poll_out(struct device *dev, unsigned char c)
{
	Uart *const uart = DEV_CFG(dev)->regs;

	/* Wait for transmitter to be ready */
	while (!(uart->UART_SR & UART_SR_TXRDY))
		;

	/* send a character */
	uart->UART_THR = (u32_t)c;
	return c;
}

static int baudrate_set(Uart *const uart, u32_t baudrate,
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

	uart->UART_BRGR = UART_BRGR_CD(divisor);

	return 0;
}

static const struct uart_driver_api uart_sam_driver_api = {
	.poll_in = uart_sam_poll_in,
	.poll_out = uart_sam_poll_out,
};

/* Get the DTS defined configuration based on the DTS id */
#define DTS_CONFIG(_dtsid_, _field_)	ATMEL_SAM_UART_ ## _dtsid_ ## _ ## _field_

/* Macro Expansion to get the base address of a module given it's DTS id */
#define DTS_BASE(x) 	DTS_CONFIG(x, BASE_ADDRESS_0)

#define UART_SAM_DRIVER_INIT(_id_, _did_)								\
DEVICE_DECLARE(uart_sam_##_id_);										\
static const struct uart_sam_dev_cfg uart_sam_config_##_id_ = {			\
	.regs = (void*)DTS_BASE(_did_),										\
	.periph_id = DTS_CONFIG(_did_, IRQ_0),								\
	.pin_rx = PIN_UART##_id_##_RXD,										\
	.pin_tx = PIN_UART##_id_##_TXD										\
};																		\
static struct uart_sam_dev_data uart_sam_data_##_id_ = {				\
	.baud_rate = DTS_CONFIG(_did_, CURRENT_SPEED)						\
};																		\
DEVICE_AND_API_INIT(uart_sam_##_id_, 									\
	DTS_CONFIG(_did_, LABEL),											\
	&uart_sam_init, 													\
	&uart_sam_data_##_id_, 												\
	&uart_sam_config_##_id_,											\
	PRE_KERNEL_1, CONFIG_KERNEL_INIT_PRIORITY_DEVICE,					\
	&uart_sam_driver_api);

#if DTS_BASE(CONFIG_UART_SAM_0_DTS_ID)
UART_SAM_DRIVER_INIT(0, CONFIG_UART_SAM_0_DTS_ID))
#endif

#if DTS_BASE(CONFIG_UART_SAM_1_DTS_ID)
UART_SAM_DRIVER_INIT(1, CONFIG_UART_SAM_1_DTS_ID)
#endif

#if DTS_BASE(CONFIG_UART_SAM_2_DTS_ID)
UART_SAM_DRIVER_INIT(2, CONFIG_UART_SAM_2_DTS_ID)
#endif

#if DTS_BASE(CONFIG_UART_SAM_3_DTS_ID)
UART_SAM_DRIVER_INIT(3, CONFIG_UART_SAM_3_DTS_ID)
#endif

#if DTS_BASE(CONFIG_UART_SAM_4_DTS_ID)
UART_SAM_DRIVER_INIT(4, CONFIG_UART_SAM_4_DTS_ID)
#endif
