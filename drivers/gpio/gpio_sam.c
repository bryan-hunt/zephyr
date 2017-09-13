/*
 * Copyright (c) 2016 Intel Corporation.
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

#if 0


/**
 * \name Interrupt Support
 *
 * The GPIO can be configured to generate an interrupt when it detects a
 * change on a GPIO pin.
 *
 * @{
 */

bool gpio_set_pin_callback(ioport_pin_t pin, gpio_pin_callback_t callback,
		uint8_t irq_level);

/**
 * \brief Enable the interrupt of a pin.
 *
 * \param [in] pin The pin number
 */
static inline void gpio_enable_pin_interrupt(ioport_pin_t pin)
{
	GpioPort *gpio_port = &(GPIO->GPIO_PORT[ioport_pin_to_port_id(pin)]);
	gpio_port->GPIO_IERS = ioport_pin_to_mask(pin);
}

/**
 * \brief Disable the interrupt of a pin.
 *
 * \param [in] pin The pin number
 */
static inline void gpio_disable_pin_interrupt(ioport_pin_t pin)
{
	GpioPort *gpio_port = &(GPIO->GPIO_PORT[ioport_pin_to_port_id(pin)]);
	gpio_port->GPIO_IERC = ioport_pin_to_mask(pin);
}

/**
 * \brief Get the interrupt flag of a pin.
 *
 * \param [in] pin The pin number
 *
 * \retval uint32_t The pin interrupt flag (0/1)
 */
static inline uint32_t gpio_get_pin_interrupt_flag(ioport_pin_t pin)
{
	GpioPort *gpio_port = &(GPIO->GPIO_PORT[ioport_pin_to_port_id(pin)]);
	return (((gpio_port->GPIO_IFR && ioport_pin_to_mask(pin)) == 0) ? 0 : 1);
}

/**
 * \brief Clear the interrupt flag of a pin.
 *
 * \param [in] pin The pin number
 */
static inline void gpio_clear_pin_interrupt_flag(ioport_pin_t pin)
{
	GpioPort *gpio_port = &(GPIO->GPIO_PORT[ioport_pin_to_port_id(pin)]);
	gpio_port->GPIO_IFRC = ioport_pin_to_mask(pin);
}



/**
 * \name Peripheral Event System Support
 *
 * The GPIO can be programmed to output peripheral events whenever an interrupt
 * condition is detected, such as pin value change, or only when a rising or
 * falling edge is detected.
 *
 * @{
 */

/**
 * \brief Enable the peripheral event generation of a pin.
 *
 * \param [in] pin The pin number
 */
static inline void gpio_enable_pin_periph_event(ioport_pin_t pin)
{
	GpioPort *gpio_port = &(GPIO->GPIO_PORT[ioport_pin_to_port_id(pin)]);
	gpio_port->GPIO_EVERS = ioport_pin_to_mask(pin);
}

/**
 * \brief Disable the peripheral event generation of a pin.
 *
 * \param [in] pin The pin number
 *
 */
static inline void gpio_disable_pin_periph_event(ioport_pin_t pin)
{
	GpioPort *gpio_port = &(GPIO->GPIO_PORT[ioport_pin_to_port_id(pin)]);
	gpio_port->GPIO_EVERC = ioport_pin_to_mask(pin);
}







/**
 * Maximum number of interrupt sources that can be defined. This
 * constant can be increased, but the current value is the smallest possible
 * one that will be compatible with all existing projects.
 */
#ifndef GPIO_MAX_INTERRUPT_SOURCES
#define GPIO_MAX_INTERRUPT_SOURCES       6
#endif

/** GPIO interrupt generators are grouped into groups of eight  */
#define GPIO_INT_GROUP_MASK0	0x000000FFu
#define GPIO_INT_GROUP_MASK1	0x0000FF00u
#define GPIO_INT_GROUP_MASK2	0x00FF0000u
#define GPIO_INT_GROUP_MASK3	0xFF000000u


/**
 * Structure for a GPIO pin interrupt source.
 */
struct gpio_interrupt_source {
	uint32_t pin;                 /**< The pin number */
	gpio_pin_callback_t callback; /**< Interrupt callback function. */
};

/* List of interrupt sources. */
static struct gpio_interrupt_source gpio_int_sources[GPIO_MAX_INTERRUPT_SOURCES];

/* Number of currently defined interrupt sources. */
static uint32_t gpio_nb_sources = 0;

/**
 * \brief Set callback for given GPIO pin
 *
 * \param [in] pin The pin number
 * \param [in] callback callback function pointer
 * \param [in] irq_level interrupt level
 *
 * \retval true Set successfully
 * \retval false Wrong parameters or maximum number of interrupt
 *                  sources has been exceeding
 */
bool gpio_set_pin_callback(ioport_pin_t pin, gpio_pin_callback_t callback,
		uint8_t irq_level)
{
	int8_t i;
	int8_t irq_line;

	if (gpio_nb_sources >= GPIO_MAX_INTERRUPT_SOURCES) {
		return false;
	}

	/*
	 * Get IRQ line for the given pin.
	 *
	 * \note Grouping interrupt generators into groups of eight, four
	 * different interrupt handlers can be installed for each GPIO port.
	 */
	for (i = 0; i < 4; i++) {
		if (ioport_pin_to_mask(pin) & (GPIO_INT_GROUP_MASK << (i * 8))) {
			break;
		}
	}
	irq_line = GPIO_0_IRQn + ioport_pin_to_port_id(pin) * 4 + i;

	gpio_int_sources[gpio_nb_sources].pin = pin;
	gpio_int_sources[gpio_nb_sources].callback = callback;
	NVIC_ClearPendingIRQ((IRQn_Type)irq_line);
	NVIC_SetPriority((IRQn_Type)irq_line, irq_level);
	NVIC_EnableIRQ((IRQn_Type)irq_line);

	gpio_nb_sources++;

	return true;
}

/**
 * Common GPIO handler.
 */
static void gpio_common_handler(uint32_t port_id, uint32_t port_mask)
{
	GpioPort *gpio_port = &(GPIO->GPIO_PORT[port_id]);
	uint32_t i;
	uint32_t int_flags;
	ioport_pin_t pin;

	int_flags = gpio_port->GPIO_IFR;

	for (i = 0; i < gpio_nb_sources; i++) {
		pin = gpio_int_sources[i].pin;
		if ((ioport_pin_to_port_id(pin) == port_id) &&
				(ioport_pin_to_mask(pin) & int_flags)) {
			if (gpio_int_sources[i].callback != NULL) {
				gpio_int_sources[i].callback();
			} else {
				Assert(false); /* Catch unexpected interrupt */
			}
		}
	}

	gpio_port->GPIO_IFRC = (int_flags & port_mask);
}

/**
 * GPIO handler 0 (PA 0..7)
 */
void GPIO_0_Handler(void)
{
	gpio_common_handler(IOPORT_GPIOA, GPIO_INT_GROUP_MASK0);
}

/**
 * GPIO handler 1 (PA 8..15)
 */
void GPIO_1_Handler(void)
{
	gpio_common_handler(IOPORT_GPIOA, GPIO_INT_GROUP_MASK1);
}

/**
 * GPIO handler 2 (PA 16..23)
 */
void GPIO_2_Handler(void)
{
	gpio_common_handler(IOPORT_GPIOA, GPIO_INT_GROUP_MASK2);
}

/**
 * GPIO handler 3 (PA 24..31)
 */
void GPIO_3_Handler(void)
{
	gpio_common_handler(IOPORT_GPIOA, GPIO_INT_GROUP_MASK3);
}

/**
 * GPIO handler 4 (PB 0..7)
 */
void GPIO_4_Handler(void)
{
	gpio_common_handler(IOPORT_GPIOB, GPIO_INT_GROUP_MASK0);
}

/**
 * GPIO handler 5 (PB 8..15)
 */
void GPIO_5_Handler(void)
{
	gpio_common_handler(IOPORT_GPIOB, GPIO_INT_GROUP_MASK1);
}

/**
 * GPIO handler 6 (PB 16..23)
 */
void GPIO_6_Handler(void)
{
	gpio_common_handler(IOPORT_GPIOB, GPIO_INT_GROUP_MASK2);
}

/**
 * GPIO handler 7 (PB 24..31)
 */
void GPIO_7_Handler(void)
{
	gpio_common_handler(IOPORT_GPIOB, GPIO_INT_GROUP_MASK3);
}

/**
 * GPIO handler 8 (PC 0..7)
 */
void GPIO_8_Handler(void)
{
	gpio_common_handler(IOPORT_GPIOC, GPIO_INT_GROUP_MASK0);
}

/**
 * GPIO handler 9 (PC 8..15)
 */
void GPIO_9_Handler(void)
{
	gpio_common_handler(IOPORT_GPIOC, GPIO_INT_GROUP_MASK1);
}

/**
 * GPIO handler 10 (PC 16..23)
 */
void GPIO_10_Handler(void)
{
	gpio_common_handler(IOPORT_GPIOC, GPIO_INT_GROUP_MASK2);
}

/**
 * GPIO handler 11 (PC 24..31)
 */
void GPIO_11_Handler(void)
{
	gpio_common_handler(IOPORT_GPIOC, GPIO_INT_GROUP_MASK3);
}



#endif






































typedef void (*config_func_t)(struct device *dev);

/* Configuration data */
struct gpio_sam_config {
	Pio *port;
	config_func_t		config_func;
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
		cfg->port->PIO_PDR = mask;
		return;
	}

	/* Setup the pin direction */
	if ((flags & GPIO_DIR_MASK) == GPIO_DIR_OUT) {
		cfg->port->PIO_OER = mask;
	} else {
		cfg->port->PIO_ODR = mask;
	}

	/* Setup interrupt config */
	if (flags & GPIO_INT) {
		if (flags & GPIO_INT_DOUBLE_EDGE) {
			cfg->port->PIO_AIMDR = mask;
		} else {
			cfg->port->PIO_AIMER = mask;

			if (flags & GPIO_INT_EDGE) {
				cfg->port->PIO_ESR = mask;
			} else {
				cfg->port->PIO_LSR = mask;
			}

			if (flags & GPIO_INT_ACTIVE_HIGH) {
				/* Trigger in high level or rising edge */
				cfg->port->PIO_REHLSR = mask;
			} else {
				/* Trigger in low level or falling edge */
				cfg->port->PIO_FELLSR = mask;
			}
		}
	}

	/* Pull-up? */
	if ((flags & GPIO_PUD_MASK) == GPIO_PUD_PULL_UP) {
		/* Enable pull-up */
		cfg->port->PIO_PUER = mask;
	} else {
		/* Disable pull-up */
		cfg->port->PIO_PUDR = mask;
	}

#if 0
	/* Debounce */
	if (flags & GPIO_INT_DEBOUNCE) {
		cfg->port->PIO_DIFSR = mask;
	} else {
		cfg->port->PIO_SCIFSR = mask;
	}
#endif

	/* Enable the pin last after pin setup */
	if (flags & GPIO_PIN_ENABLE) {
		cfg->port->PIO_PER = mask;
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
			cfg->port->PIO_SODR = BIT(pin);
		} else {
			/* clear the pin */
			cfg->port->PIO_CODR = BIT(pin);
		}
		break;
	case GPIO_ACCESS_BY_PORT:
		if (value) {
			/* set all pins */
			cfg->port->PIO_SODR = 0xFFFFFFFF;
		} else {
			/* clear all pins */
			cfg->port->PIO_CODR = 0xFFFFFFFF;
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

	*value = cfg->port->PIO_PDSR;

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

	int_stat = cfg->port->PIO_ISR;

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

	cfg->port->PIO_IER |= mask;

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

	cfg->port->PIO_IDR |= mask;

	return 0;
}

static const struct gpio_driver_api gpio_sam_drv_api_funcs = {
	.config = gpio_sam_config,
	.write = gpio_sam_write,
	.read = gpio_sam_read,
	.manage_callback = gpio_sam_manage_callback,
	.enable_callback = gpio_sam_enable_callback,
	.disable_callback = gpio_sam_disable_callback,
};

/**
 * @brief Initialization function of MMIO
 *
 * @param dev Device struct
 * @return 0 if successful, failed otherwise.
 */
static int gpio_sam_init(struct device *dev)
{
	const struct gpio_sam_config *cfg = dev->config->config_info;

	cfg->config_func(dev);

	return 0;
}

#define GPIO_DEVICE_INIT(_name_, _NAME_)										\
DEVICE_DECLARE(gpio_sam_##_name_);												\
\
static void gpio_sam_config_irq_##_name_(struct device *dev) {					\
	PMC->PMC_PCER0 = BIT(ID_PIO##_NAME_);										\
	IRQ_CONNECT(PIOA_IRQn, CONFIG_GPIO_SAM_PORT##_NAME_##_IRQ_PRI,					\
		    gpio_sam_isr, DEVICE_GET(gpio_sam_##_name_), 0);					\
	irq_enable(PIO##_NAME_##_IRQn); }											\
	\
static const struct gpio_sam_config gpio_sam_cfg_##_name_ = {					\
	.port = PIO##_NAME_,														\
	.config_func = gpio_sam_config_irq_##_name_ };								\
	\
static struct gpio_sam_runtime gpio_sam_data_##_name_;						\
\
DEVICE_AND_API_INIT(gpio_sam_##_name_, "GPIO" #_NAME_,						\
	gpio_sam_init, &gpio_sam_data_##_name_, 								\
	&gpio_sam_cfg_##_name_,													\
	POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEVICE, &gpio_sam_drv_api_funcs);

/* Port A */
#if CONFIG_GPIO_SAM_PORTA
GPIO_DEVICE_INIT(a, A);
#endif /* CONFIG_GPIO_SAM_PORTA */

/* Port B */
#if CONFIG_GPIO_SAM_PORTA
GPIO_DEVICE_INIT(b, B);
#endif /* CONFIG_GPIO_SAM_PORTB */

/* Port C */
#if CONFIG_GPIO_SAM_PORTC
GPIO_DEVICE_INIT(c, C);
#endif /* CONFIG_GPIO_SAM_PORTC */
