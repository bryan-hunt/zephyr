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
#include <misc/__assert.h>
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

#define UART_SAM_DRIVER_INIT(_id_)														\
static const struct uart_sam_dev_cfg uart##_id_##_sam_config = {				\
	.regs = (void*)ATMEL_SAM_UART_##_id_##_BASE_ADDRESS_0,						\
	.periph_id = ID_UART##_id_,													\
	.pin_rx = PIN_UART##_id_##_RXD,												\
	.pin_tx = PIN_UART##_id_##_TXD												\
};																				\
static struct uart_sam_dev_data uart##_id_##_sam_data = {						\
	.baud_rate = ATMEL_SAM_UART_##_id_##_CURRENT_SPEED							\
};																				\
DEVICE_AND_API_INIT(uart##_id_##_sam, ATMEL_SAM_UART_##_id_##_LABEL, 			\
	&uart_sam_init, &uart##_id_##_sam_data, &uart##_id_##_sam_config, 			\
	PRE_KERNEL_1, CONFIG_KERNEL_INIT_PRIORITY_DEVICE, &uart_sam_driver_api);

#ifdef CONFIG_UART_SAM_PORT_0
UART_SAM_DRIVER_INIT(0)
#endif

#ifdef CONFIG_UART_SAM_PORT_1
UART_SAM_DRIVER_INIT(1)
#endif

#ifdef CONFIG_UART_SAM_PORT_2
UART_SAM_DRIVER_INIT(2)
#endif

#ifdef CONFIG_UART_SAM_PORT_3
UART_SAM_DRIVER_INIT(3)
#endif

#ifdef CONFIG_UART_SAM_PORT_4
UART_SAM_DRIVER_INIT(4)
#endif

