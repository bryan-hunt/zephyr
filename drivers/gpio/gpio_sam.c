/*
 * Copyright (c) 2017 Microchip Technology Inc
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file Driver for the Atmel SAM PIO Controller.
 */

#include <errno.h>

#include <kernel.h>

#include <device.h>
#include <init.h>

#include <soc.h>

#include <gpio.h>
#include "gpio_utils.h"

/* Configuration data */
struct gpio_sam_config {
	Pio *				regs;
	u32_t				periph_id;
	void (*irq_config)(struct device *dev);
};

struct gpio_sam_runtime {
	/* callbacks */
	sys_slist_t		cb;
};

static void _config(struct device *dev, u32_t mask, int flags)
{
	const struct gpio_sam_config *cfg = dev->config->config_info;

	/* Disable the pin and return as setup is meaningless now */
	if (flags & GPIO_PIN_DISABLE) {
		cfg->regs->PIO_PDR = mask;
		return;
	}

	/* Setup the pin direction */
	if ((flags & GPIO_DIR_MASK) == GPIO_DIR_OUT) {
		cfg->regs->PIO_OER = mask;
	} else {
		cfg->regs->PIO_ODR = mask;
	}

	/* Setup interrupt config */
	if (flags & GPIO_INT) {
		if (flags & GPIO_INT_DOUBLE_EDGE) {
			cfg->regs->PIO_AIMDR = mask;
		} else {
			cfg->regs->PIO_AIMER = mask;

			if (flags & GPIO_INT_EDGE) {
				cfg->regs->PIO_ESR = mask;
			} else {
				cfg->regs->PIO_LSR = mask;
			}

			if (flags & GPIO_INT_ACTIVE_HIGH) {
				/* Trigger in high level or rising edge */
				cfg->regs->PIO_REHLSR = mask;
			} else {
				/* Trigger in low level or falling edge */
				cfg->regs->PIO_FELLSR = mask;
			}
		}
	}

	/* Pull-up? */
	if ((flags & GPIO_PUD_MASK) == GPIO_PUD_PULL_UP) {
		/* Enable pull-up */
		cfg->regs->PIO_PUER = mask;
	} else {
		/* Disable pull-up */
		cfg->regs->PIO_PUDR = mask;
	}

#if 0
	/* Debounce */
	if (flags & GPIO_INT_DEBOUNCE) {
		cfg->regs->PIO_DIFSR = mask;
	} else {
		cfg->regs->PIO_SCIFSR = mask;
	}
#endif

	/* Enable the pin last after pin setup */
	if (flags & GPIO_PIN_ENABLE) {
		cfg->regs->PIO_PER = mask;
	}
}

/**
 * @brief Configure pin or port
 *
 * @param dev Device struct
 * @param access_op Access operation (pin or port)
 * @param pin The pin number
 * @param flags Flags of pin or port
 *
 * @return 0 if successful, failed otherwise
 */
static int gpio_sam_config(struct device *dev, int access_op,
			    u32_t pin, int flags)
{
	switch (access_op) {
	case GPIO_ACCESS_BY_PIN:
		_config(dev, BIT(pin), flags);
		break;
	case GPIO_ACCESS_BY_PORT:
		_config(dev, (0xFFFFFFFF), flags);
		break;
	default:
		return -ENOTSUP;
	}

	return 0;
}

/**
 * @brief Set the pin or port output
 *
 * @param dev Device struct
 * @param access_op Access operation (pin or port)
 * @param pin The pin number
 * @param value Value to set (0 or 1)
 *
 * @return 0 if successful, failed otherwise
 */
static int gpio_sam_write(struct device *dev, int access_op,
			   u32_t pin, u32_t value)
{
	const struct gpio_sam_config *cfg = dev->config->config_info;

	switch (access_op) {
	case GPIO_ACCESS_BY_PIN:
		if (value) {
			/* set the pin */
			cfg->regs->PIO_SODR = BIT(pin);
		} else {
			/* clear the pin */
			cfg->regs->PIO_CODR = BIT(pin);
		}
		break;
	case GPIO_ACCESS_BY_PORT:
		if (value) {
			/* set all pins */
			cfg->regs->PIO_SODR = 0xFFFFFFFF;
		} else {
			/* clear all pins */
			cfg->regs->PIO_CODR = 0xFFFFFFFF;
		}
		break;
	default:
		return -ENOTSUP;
	}

	return 0;
}

/**
 * @brief Read the pin or port status
 *
 * @param dev Device struct
 * @param access_op Access operation (pin or port)
 * @param pin The pin number
 * @param value Value of input pin(s)
 *
 * @return 0 if successful, failed otherwise
 */
static int gpio_sam_read(struct device *dev, int access_op,
				       u32_t pin, u32_t *value)
{
	const struct gpio_sam_config *cfg = dev->config->config_info;

	*value = cfg->regs->PIO_PDSR;

	switch (access_op) {
	case GPIO_ACCESS_BY_PIN:
		*value = (*value >> pin) & 0x01;
		break;
	case GPIO_ACCESS_BY_PORT:
		break;
	default:
		return -ENOTSUP;
	}

	return 0;
}

static void gpio_sam_isr(void *arg)
{
	struct device *dev = (struct device *)arg;
	const struct gpio_sam_config *cfg = dev->config->config_info;
	struct gpio_sam_runtime *context = dev->driver_data;
	u32_t int_stat;

	int_stat = cfg->regs->PIO_ISR;

	_gpio_fire_callbacks(&context->cb, dev, int_stat);
}

static int gpio_sam_manage_callback(struct device *dev,
				     struct gpio_callback *callback,
				     bool set)
{
	struct gpio_sam_runtime *context = dev->driver_data;

	_gpio_manage_callback(&context->cb, callback, set);

	return 0;
}

static int gpio_sam_enable_callback(struct device *dev,
				     int access_op, u32_t pin)
{
	const struct gpio_sam_config *cfg = dev->config->config_info;
	u32_t mask;

	switch (access_op) {
	case GPIO_ACCESS_BY_PIN:
		mask = BIT(pin);
		break;
	case GPIO_ACCESS_BY_PORT:
		mask = 0xFFFFFFFF;
		break;
	default:
		return -ENOTSUP;
	}

	cfg->regs->PIO_IER |= mask;

	return 0;
}

static int gpio_sam_disable_callback(struct device *dev,
				      int access_op, u32_t pin)
{
	const struct gpio_sam_config *cfg = dev->config->config_info;
	u32_t mask;

	switch (access_op) {
	case GPIO_ACCESS_BY_PIN:
		mask = BIT(pin);
		break;
	case GPIO_ACCESS_BY_PORT:
		mask = 0xFFFFFFFF;
		break;
	default:
		return -ENOTSUP;
	}

	cfg->regs->PIO_IDR |= mask;

	return 0;
}

static const struct gpio_driver_api gpio_sam_drv_api = {
	.config = gpio_sam_config,
	.write = gpio_sam_write,
	.read = gpio_sam_read,
	.manage_callback = gpio_sam_manage_callback,
	.enable_callback = gpio_sam_enable_callback,
	.disable_callback = gpio_sam_disable_callback,
};


static int gpio_sam_init(struct device *dev)
{
	const struct gpio_sam_config *cfg;
	__ASSERT_NO_MSG(dev);
	__ASSERT_NO_MSG(dev->config);
	cfg = dev->config->config_info;
	__ASSERT_NO_MSG(cfg);

	soc_pmc_peripheral_enable(cfg->periph_id);

	cfg->irq_config(dev);
	irq_enable(cfg->periph_id);

	return 0;
}

/* Get the DTS defined configuration based on the DTS id (which is the base address of the module) */
#define DTS_CONFIG(_dtsid_, _field_)	ATMEL_SAM_GPIO_ ## _dtsid_ ## _ ## _field_

/* Macro Expansion to get the base address of a module given it's DTS id */
#define DTS_BASE(x) 	DTS_CONFIG(x, BASE_ADDRESS_0)

/* Driver Definition per instance of the device */
#define GPIO_SAM_DEVICE_INIT(_id_, _did_)							\
DEVICE_DECLARE(gpio_sam_##_id_);									\
static void gpio_sam_irq_config_##_id_(struct device *dev) {		\
	IRQ_CONNECT(DTS_CONFIG(_did_, IRQ_0), 							\
			DTS_CONFIG(_did_, IRQ_0_PRIORITY),						\
			gpio_sam_isr, DEVICE_GET(gpio_sam_##_id_), 0); }		\
static const struct gpio_sam_config gpio_sam_cfg_##_id_ = {			\
	.regs = (void*)DTS_BASE(_did_),									\
	.periph_id = DTS_CONFIG(_did_, IRQ_0),							\
	.irq_config = gpio_sam_irq_config_##_id_ };						\
static struct gpio_sam_runtime gpio_sam_data_##_id_;				\
DEVICE_AND_API_INIT(gpio_sam_##_id_,								\
	DTS_CONFIG(_did_, LABEL),										\
	gpio_sam_init,													\
	&gpio_sam_data_##_id_, 											\
	&gpio_sam_cfg_##_id_,											\
	POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEVICE,				\
	&gpio_sam_drv_api);


/* Port A */
#if DTS_BASE(CONFIG_GPIO_SAM_A_DTS_ID)
GPIO_SAM_DEVICE_INIT(a, CONFIG_GPIO_SAM_A_DTS_ID)
#endif

/* Port B */
#if DTS_BASE(CONFIG_GPIO_SAM_B_DTS_ID)
GPIO_SAM_DEVICE_INIT(b, CONFIG_GPIO_SAM_B_DTS_ID)
#endif

/* Port C */
#if DTS_BASE(CONFIG_GPIO_SAM_C_DTS_ID)
GPIO_SAM_DEVICE_INIT(c, CONFIG_GPIO_SAM_C_DTS_ID)
#endif

/* Port D */
#if DTS_BASE(CONFIG_GPIO_SAM_D_DTS_ID)
GPIO_SAM_DEVICE_INIT(d, CONFIG_GPIO_SAM_D_DTS_ID)
#endif

/* Port E */
#if DTS_BASE(CONFIG_GPIO_SAM_E_DTS_ID)
GPIO_SAM_DEVICE_INIT(e, CONFIG_GPIO_SAM_E_DTS_ID)
#endif
