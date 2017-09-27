/*
 * Copyright (c) 2017 Piotr Mienkowski
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/** @file
 * @brief I2C bus (TWI) driver for Atmel SAM MCU family.
 *
 * Limitations:
 * - Only I2C Master Mode with 7 bit addressing is currently supported.
 * - No reentrancy support.
 */

#include <errno.h>
#include <misc/__assert.h>
#include <stdbool.h>
#include <kernel.h>
#include <device.h>
#include <init.h>
#include <soc.h>
#include <i2c.h>
#include "i2c-priv.h"

#define SYS_LOG_DOMAIN "dev/i2c_sam_twi"
#define SYS_LOG_LEVEL CONFIG_SYS_LOG_I2C_LEVEL
#include <logging/sys_log.h>

/** I2C bus speed [Hz] in Standard Mode */
#define BUS_SPEED_STANDARD_HZ         100000U
/** I2C bus speed [Hz] in Fast Mode */
#define BUS_SPEED_FAST_HZ             400000U
/* Maximum value of Clock Divider (CKDIV) */
#define CKDIV_MAX                          7


#include <misc/printk.h>
//#define I2C_SAM_TWI_TIMEOUT			CONFIG_I2C_TRANSFER_TIMEOUT
#define I2C_SAM_TWI_TIMEOUT				K_FOREVER

//struct i2c_sam_twi_pin_cfg {
//	u32_t port;
//	u32_t pin;
//	u32_t mux;
//};

/* Device constant configuration parameters */
struct i2c_sam_twi_dev_cfg {
	Twi *regs;
	void (*irq_config)(void);
	u32_t bitrate;
//	const struct i2c_sam_twi_pin_cfg *pin_list;
	const struct soc_gpio_pin *pin_list;
	u8_t pin_list_size;
	u8_t periph_id;
};

struct twi_msg {
	/* Buffer containing data to read or write */
	u8_t *buf;
	/* Length of the buffer */
	u32_t len;
	/* Index of the next byte to be read/written from/to the buffer */
	u32_t idx;
	/* Value of TWI_SR at the end of the message */
	u32_t twi_sr;
	/* Transfer flags as defined in the i2c.h file */
	u8_t flags;
};

/* Device run time data */
struct i2c_sam_twi_dev_data {
	struct k_sem sem;
	struct twi_msg msg;
};

#define DEV_NAME(dev) ((dev)->config->name)
#define DEV_CFG(dev) \
	((const struct i2c_sam_twi_dev_cfg *const)(dev)->config->config_info)
#define DEV_DATA(dev) \
	((struct i2c_sam_twi_dev_data *)(dev)->driver_data)

static int i2c_clk_set(Twi *const twi, u32_t speed)
{
	u32_t ck_div = 0;
	u32_t cl_div;
	bool div_completed = false;

	/*  From the datasheet "TWI Clock Waveform Generator Register"
	 *  T_low = ( ( CLDIV × 2^CKDIV ) + 4 ) × T_MCK
	 */
	while (!div_completed) {
		cl_div =   ((SOC_ATMEL_SAM_MCK_FREQ_HZ / (2 * speed)) - 4)
			 / (1 << ck_div);

		if (cl_div <= 255) {
			div_completed = true;
		} else {
			ck_div++;
		}
	}

	if (ck_div > CKDIV_MAX) {
		SYS_LOG_ERR("Failed to configure I2C clock");
		return -EIO;
	}

	/* Set TWI clock duty cycle to 50% */
	twi->TWI_CWGR = TWI_CWGR_CLDIV(cl_div) | TWI_CWGR_CHDIV(cl_div)
			| TWI_CWGR_CKDIV(ck_div);

	return 0;
}

static int i2c_sam_twi_configure(struct device *dev, u32_t config)
{
	const struct i2c_sam_twi_dev_cfg *const dev_cfg = DEV_CFG(dev);
	Twi *const twi = dev_cfg->regs;
	u32_t cfg_speed = (config & I2C_SPEED_MASK) >> I2C_SPEED_SHIFT;
	u32_t bitrate;
	int ret;

	if (!(config & I2C_MODE_MASTER)) {
		SYS_LOG_ERR("Master Mode is not enabled");
		return -EIO;
	}

	if (config & I2C_ADDR_10_BITS) {
		SYS_LOG_ERR("I2C 10-bit addressing is currently not supported");
		SYS_LOG_ERR("Please submit a patch");
		return -EIO;
	}

	/* Configure clock */
	switch (cfg_speed) {
	case I2C_SPEED_STANDARD:
		bitrate = BUS_SPEED_STANDARD_HZ;
		break;
	case I2C_SPEED_FAST:
		bitrate = BUS_SPEED_FAST_HZ;
		break;
	default:
		SYS_LOG_ERR("Unsupported I2C speed value");
		return -EIO;
	}

	/* Disable Interrupts */
	twi->TWI_IDR = ~0UL;

	/* Setup clock waveform */
	ret = i2c_clk_set(twi, bitrate);
	if (ret < 0) {
		return ret;
	}

	/* Dummy Read to status register */
	ret = twi->TWI_SR;

	/* Set Slave Disable bit */
	twi->TWI_CR = TWI_CR_MSEN | TWI_CR_SVDIS;

	return 0;
}

static void write_msg_start(Twi *const twi, struct twi_msg *msg, u8_t daddr)
{
	/* Set slave address and number of internal address bytes. */
	twi->TWI_MMR = TWI_MMR_DADR(daddr);

	/* Write first data byte on I2C bus */
	twi->TWI_THR = msg->buf[msg->idx++];

	/* Enable Transmit Ready and Transmission Completed interrupts */
	twi->TWI_IER = TWI_IER_TXRDY | TWI_IER_TXCOMP | TWI_IER_NACK;
}

static void read_msg_start(Twi *const twi, struct twi_msg *msg, u8_t daddr)
{
	/* Set slave address and number of internal address bytes */
	twi->TWI_MMR = TWI_MMR_MREAD | TWI_MMR_DADR(daddr);

	/* In single data byte read the START and STOP must both be set */
	twi->TWI_CR = TWI_CR_START;

	/* Enable Receive Ready and Transmission Completed interrupts */
	twi->TWI_IER = TWI_IER_RXRDY | TWI_IER_TXCOMP | TWI_IER_NACK;
}

static int i2c_sam_twi_transfer(struct device *dev, struct i2c_msg *msgs,
				u8_t num_msgs, u16_t addr)
{
	const struct i2c_sam_twi_dev_cfg *const dev_cfg = DEV_CFG(dev);
	struct i2c_sam_twi_dev_data * dev_data = DEV_DATA(dev);
	Twi *const twi = dev_cfg->regs;

	__ASSERT_NO_MSG(msgs);
	if (!num_msgs) {
		return 0;
	}

	/* Clear pending interrupts, such as NACK. */
	(void)twi->TWI_SR;

	/* Set number of internal address bytes to 0, not used. */
	twi->TWI_IADR = 0;

	for (; num_msgs > 0; num_msgs--, msgs++) {
		dev_data->msg.buf = msgs->buf;
		dev_data->msg.len = msgs->len;
		dev_data->msg.idx = 0;
		dev_data->msg.twi_sr = 0;
		dev_data->msg.flags = msgs->flags;

		/*
		 * REMARK: Dirty workaround:
		 *
		 * The controller does not have a documented, generic way to
		 * issue RESTART when changing transfer direction as master.
		 * Send a stop condition in such a case.
		 */
		if (num_msgs > 1) {
			if ((msgs[0].flags & I2C_MSG_RW_MASK) !=
			    (msgs[1].flags & I2C_MSG_RW_MASK)) {
				dev_data->msg.flags |= I2C_MSG_STOP;
			}
		}

		if ((msgs->flags & I2C_MSG_RW_MASK) == I2C_MSG_READ) {
			read_msg_start(twi, &dev_data->msg, addr);
		} else {
			write_msg_start(twi, &dev_data->msg, addr);
		}

		/* Wait for the transfer to complete */
		k_sem_take(&dev_data->sem, I2C_SAM_TWI_TIMEOUT);

		if (dev_data->msg.twi_sr > 0) {
			/* Something went wrong */
			return -EIO;
		}
	}

	return 0;
}

static void i2c_sam_twi_isr(void *arg)
{
	struct device *dev = (struct device *)arg;
	const struct i2c_sam_twi_dev_cfg *const dev_cfg = DEV_CFG(dev);
	struct i2c_sam_twi_dev_data * dev_data = DEV_DATA(dev);
	Twi *const twi = dev_cfg->regs;
	struct twi_msg *msg = &dev_data->msg;
	u32_t isr_status;

	/* Retrieve interrupt status */
	isr_status = twi->TWI_SR & twi->TWI_IMR;

	/* Not Acknowledged */
	if (isr_status & TWI_SR_NACK) {
		msg->twi_sr = isr_status;
	}

	/* Byte received */
	if (isr_status & TWI_SR_RXRDY) {
		msg->buf[msg->idx++] = twi->TWI_RHR;

		if (msg->idx == msg->len - 1) {
			/* Send a STOP condition on the TWI */
			twi->TWI_CR = TWI_CR_STOP;
		}
	}

	/* Byte sent */
	if (isr_status & TWI_SR_TXRDY) {
		if (msg->idx == msg->len) {
			if (msg->flags & I2C_MSG_STOP) {
				/* Send a STOP condition on the TWI */
				twi->TWI_CR = TWI_CR_STOP;
				/* Disable Transmit Ready interrupt */
				twi->TWI_IDR = TWI_IDR_TXRDY;
			} else {
				/* Transfer completed */
				isr_status |= TWI_SR_TXCOMP;
			}
		} else {
			twi->TWI_THR = msg->buf[msg->idx++];
		}
	}

	/* Transfer completed */
	if (isr_status & TWI_SR_TXCOMP) {
		/* Disable all enabled interrupts */
		twi->TWI_IDR = twi->TWI_IMR;
		/* All data transferred, nothing else to do */
		k_sem_give(&dev_data->sem);
	}
}

static int i2c_sam_twi_initialize(struct device *dev)
{
	const struct i2c_sam_twi_dev_cfg *const dev_cfg = DEV_CFG(dev);
	struct i2c_sam_twi_dev_data * dev_data = DEV_DATA(dev);
	Twi *const twi = dev_cfg->regs;
	u32_t bitrate_cfg;
	int ret;

	/* Configure interrupts */
	dev_cfg->irq_config();

	/* Initialize semaphore */
	k_sem_init(&dev_data->sem, 0, 1);

	/* Connect pins to the peripheral */
	soc_gpio_list_configure(dev_cfg->pin_list, dev_cfg->pin_list_size);

	/* Enable module's clock */
	soc_pmc_peripheral_enable(dev_cfg->periph_id);

	/* Reset TWI module */
	twi->TWI_CR = TWI_CR_SWRST;

	bitrate_cfg = _i2c_map_dt_bitrate(dev_cfg->bitrate);

	ret = i2c_sam_twi_configure(dev, I2C_MODE_MASTER | bitrate_cfg);
	if (ret < 0) {
		SYS_LOG_ERR("Failed to initialize %s device", DEV_NAME(dev));
		return ret;
	}

	/* Enable module's IRQ */
	irq_enable(dev_cfg->periph_id);

	SYS_LOG_INF("Device %s initialized", DEV_NAME(dev));

	return 0;
}

static const struct i2c_driver_api i2c_sam_twi_driver_api = {
	.configure = i2c_sam_twi_configure,
	.transfer = i2c_sam_twi_transfer,
};

/* Get the DTS defined configuration based on the DTS id (which is the base address of the module) */
#define DTS_CONFIG(_dtsid_, _field_)	ATMEL_SAM_I2C_TWI_ ## _dtsid_ ## _ ## _field_

/* Macro Expansion to get the base address of a module given it's DTS id */
#define DTS_BASE(x) 	DTS_CONFIG(x, BASE_ADDRESS_0)

/* Driver Definition per instance of the device */
#define I2C_SAM_DEVICE_INIT(_m, _i, _d)							\
DEVICE_DECLARE(_m##_i);											\
static void _m##_irq_cfg_##_i(void) {							\
	IRQ_CONNECT(DTS_CONFIG(_d, IRQ_0),							\
			DTS_CONFIG(_d, IRQ_0_PRIORITY),						\
			_m##_isr,											\
		    DEVICE_GET(_m##_i), 0);								\
}																\
static const struct soc_gpio_pin _m##_pins_##_i[] = PINS_TWI##_i;	\
static const struct _m##_dev_cfg _m##_cfg_##_i = {				\
	.regs = (void*)DTS_BASE(_d),								\
	.irq_config = _m##_irq_cfg_##_i,							\
	.periph_id = DTS_CONFIG(_d, IRQ_0),							\
	.pin_list = _m##_pins_##_i,									\
	.pin_list_size = ARRAY_SIZE(_m##_pins_##_i),				\
	.bitrate = DTS_CONFIG(_d, CLOCK_FREQUENCY),					\
};																\
static struct _m##_dev_data _m##_data_##_i;						\
DEVICE_AND_API_INIT(_m##_i, 									\
	DTS_CONFIG(_d, LABEL), 										\
	&_m ## _initialize,											\
	&_m ## _data_ ## _i, 										\
	&_m ## _cfg_ ## _i, 										\
	POST_KERNEL, CONFIG_I2C_INIT_PRIORITY, 						\
	&_m ## _driver_api);

/* I2C Channel 0 */
#if DTS_BASE(CONFIG_I2C_SAM_0_DTS_ID)
I2C_SAM_DEVICE_INIT(i2c_sam_twi, 0, CONFIG_I2C_SAM_0_DTS_ID);
#endif

/* I2C Channel 1 */
#if DTS_BASE(CONFIG_I2C_SAM_1_DTS_ID)
I2C_SAM_DEVICE_INIT(i2c_sam_twi, 1, CONFIG_I2C_SAM_1_DTS_ID);
#endif

/* I2C Channel 2 */
#if DTS_BASE(CONFIG_I2C_SAM_2_DTS_ID)
I2C_SAM_DEVICE_INIT(i2c_sam_twi, 2, CONFIG_I2C_SAM_2_DTS_ID);
#endif

/* I2C Channel 3 */
#if DTS_BASE(CONFIG_I2C_SAM_3_DTS_ID)
I2C_SAM_DEVICE_INIT(i2c_sam_twi, 3, CONFIG_I2C_SAM_3_DTS_ID);
#endif

/* I2C Channel 4 */
#if DTS_BASE(CONFIG_I2C_SAM_4_DTS_ID)
I2C_SAM_DEVICE_INIT(i2c_sam_twi, 4, CONFIG_I2C_SAM_4_DTS_ID);
#endif
