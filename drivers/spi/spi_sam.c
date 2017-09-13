/**
 * \file
 *
 * \brief Serial Peripheral Interface (SPI) driver for SAM.
 *
 * Copyright (c) 2011-2016 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */

 #define SYS_LOG_LEVEL CONFIG_SYS_LOG_SPI_LEVEL
#include <logging/sys_log.h>

#include <misc/util.h>
#include <kernel.h>
#include <board.h>
#include <errno.h>
#include <spi.h>
 
/** Time-out value (number of attempts). */
#define SPI_TIMEOUT       15000

/** Status codes used by the SPI driver. */
typedef enum
{
	SPI_ERROR = -1,
	SPI_OK = 0,
	SPI_ERROR_TIMEOUT = 1,
	SPI_ERROR_ARGUMENT,
	SPI_ERROR_OVERRUN,
	SPI_ERROR_MODE_FAULT,
	SPI_ERROR_OVERRUN_AND_MODE_FAULT
} spi_status_t;

/** SPI Chip Select behavior modes while transferring. */
typedef enum spi_cs_behavior {
	/** CS does not rise until a new transfer is requested on different chip select. */
	SPI_CS_KEEP_LOW = SPI_CSR_CSAAT,
	/** CS rises if there is no more data to transfer. */
	SPI_CS_RISE_NO_TX = 0,
	/** CS is de-asserted systematically during a time DLYBCS. */
	SPI_CS_RISE_FORCED = SPI_CSR_CSNAAT
} spi_cs_behavior_t;

/**
 * \brief Generate Peripheral Chip Select Value from Chip Select ID
 * \note When chip select n is working, PCS bit n is set to low level.
 *
 * \param chip_sel_id The chip select number used
 */
#define spi_get_pcs(chip_sel_id) ((~(1u<<(chip_sel_id)))&0xF)

/* Reset SPI Device and set it to Slave mode. */
#define SPI_SAM_RESET(p)		p_spi->SPI_CR = SPI_CR_SWRST

/* Enable SPI Device */
#define SPI_SAM_ENABLE(p)		p->SPI_CR = SPI_CR_SPIEN

/* Disable SPI Device. CS is de-asserted, which indicates that the last data is done, and user
should check TX_EMPTY before disabling SPI. */
#define SPI_SAM_DISABLE(p)		p->SPI_CR = SPI_CR_SPIDIS

/* Issue a LASTXFER command. The next transfer is the last transfer and after that CS is de-asserted. */
#define SPI_SAM_LAST_XFER(p)	p->SPI_CR = SPI_CR_LASTXFER

/* Set SPI Device to Master */
#define SPI_SAM_MODE_MASTER(p)	p->SPI_MR |= SPI_MR_MSTR

/* Set SPI Device to Slave mode */
#define SPI_SAM_MODE_SLAVE(p)	p->SPI_MR &= (~SPI_MR_MSTR)

/* Get SPI mode */
#define SPI_SAM_MODE(p)			(p->SPI_MR & SPI_MR_MSTR)

/* Read status register */
#define SPI_SAM_STATUS(p)		p->SPI_SR

/* Write Data */
#define SPI_SAM_TX(p, d)		p->SPI_TDR = SPI_TDR_TD(d)

/* Read data */
#define SPI_SAM_RX(p)			(p_spi->SPI_RDR & SPI_RDR_RD_Msk)

/* Check if TX is done */
#define SPI_SAM_TX_EMPTY(p)		(p->SPI_SR & SPI_SR_TXEMPTY)

/* Check if TX is ready for more */ 
#define SPI_SAM_TX_READY(p)		(p->SPI_SR & SPI_SR_TDRE)

/* Check if RX is done */
#define SPI_SAM_RX_FULL(p)		(p_spi->SPI_SR & SPI_SR_RDRF)

/* Check if RX is ready */
#define SPI_SAM_RX_READY(p)		((p->SPI_SR & (SPI_SR_RDRF | SPI_SR_TXEMPTY)) == (SPI_SR_RDRF | SPI_SR_TXEMPTY))

/**
 * \brief Read the received data and it's peripheral chip select value.
 * While SPI works in fixed peripheral select mode, the peripheral chip select
 * value is meaningless.
 *
 * \param p_spi Pointer to an SPI instance.
 * \param data Pointer to the location where to store the received data word.
 * \param p_pcs Pointer to fill Peripheral Chip Select Value.
 *
 * \retval SPI_OK on Success.
 * \retval SPI_ERROR_TIMEOUT on Time-out.
 */
static spi_status_t spi_sam_read(Spi *p_spi, uint16_t *us_data, uint8_t *p_pcs)
{
	uint32_t timeout = SPI_TIMEOUT;
	static uint32_t reg_value;

	while (!(p_spi->SPI_SR & SPI_SR_RDRF)) {
		if (!timeout--) {
			return SPI_ERROR_TIMEOUT;
		}
	}

	reg_value = p_spi->SPI_RDR;
	if (spi_get_peripheral_select_mode(p_spi)) {
		*p_pcs = (uint8_t) ((reg_value & SPI_RDR_PCS_Msk) >> SPI_RDR_PCS_Pos);
	}
	*us_data = (uint16_t) (reg_value & SPI_RDR_RD_Msk);

	return SPI_OK;
}

/**
 * \brief Write the transmitted data with specified peripheral chip select value.
 *
 * \param p_spi Pointer to an SPI instance.
 * \param us_data The data to transmit.
 * \param uc_pcs Peripheral Chip Select Value while SPI works in peripheral select
 * mode, otherwise it's meaningless.
 * \param uc_last Indicate whether this data is the last one while SPI is working
 * in variable peripheral select mode.
 *
 * \retval SPI_OK on Success.
 * \retval SPI_ERROR_TIMEOUT on Time-out.
 */
static spi_status_t spi_sam_write(Spi *p_spi, uint16_t us_data,
		uint8_t uc_pcs, uint8_t uc_last)
{
	uint32_t timeout = SPI_TIMEOUT;
	uint32_t value;

	while (!(p_spi->SPI_SR & SPI_SR_TDRE)) {
		if (!timeout--) {
			return SPI_ERROR_TIMEOUT;
		}
	}

	if (spi_get_peripheral_select_mode(p_spi)) {
		value = SPI_TDR_TD(us_data) | SPI_TDR_PCS(uc_pcs);
		if (uc_last) {
			value |= SPI_TDR_LASTXFER;
		}
	} else {
		value = SPI_TDR_TD(us_data);
	}

	p_spi->SPI_TDR = value;

	return SPI_OK;
}

/**
 * \brief Set clock default state.
 *
 * \param p_spi Pointer to an SPI instance.
 * \param ul_pcs_ch Peripheral Chip Select channel (0~3).
 * \param ul_polarity Default clock state is logical one(high)/zero(low).
 */
static void spi_set_clock_polarity(Spi *p_spi, uint32_t ul_pcs_ch,
		uint32_t ul_polarity)
{
	if (ul_polarity) {
		p_spi->SPI_CSR[ul_pcs_ch] |= SPI_CSR_CPOL;
	} else {
		p_spi->SPI_CSR[ul_pcs_ch] &= (~SPI_CSR_CPOL);
	}
}

/**
 * \brief Set Data Capture Phase.
 *
 * \param p_spi Pointer to an SPI instance.
 *  \param ul_pcs_ch Peripheral Chip Select channel (0~3).
 *  \param ul_phase Data capture on the rising/falling edge of clock.
 */
void spi_set_clock_phase(Spi *p_spi, uint32_t ul_pcs_ch, uint32_t ul_phase)
{
	if (ul_phase) {
		p_spi->SPI_CSR[ul_pcs_ch] |= SPI_CSR_NCPHA;
	} else {
		p_spi->SPI_CSR[ul_pcs_ch] &= (~SPI_CSR_NCPHA);
	}
}

/**
 * \brief Configure CS behavior for SPI transfer (\ref spi_cs_behavior_t).
 *
 * \param p_spi Pointer to an SPI instance.
 * \param ul_pcs_ch Peripheral Chip Select channel (0~3).
 * \param ul_cs_behavior Behavior of the Chip Select after transfer.
 */
static void spi_configure_cs_behavior(Spi *p_spi, uint32_t ul_pcs_ch,
		uint32_t ul_cs_behavior)
{
	if (ul_cs_behavior == SPI_CS_RISE_FORCED) {
		p_spi->SPI_CSR[ul_pcs_ch] &= (~SPI_CSR_CSAAT);
		p_spi->SPI_CSR[ul_pcs_ch] |= SPI_CSR_CSNAAT;
	} else if (ul_cs_behavior == SPI_CS_RISE_NO_TX) {
		p_spi->SPI_CSR[ul_pcs_ch] &= (~SPI_CSR_CSAAT);
		p_spi->SPI_CSR[ul_pcs_ch] &= (~SPI_CSR_CSNAAT);
	} else if (ul_cs_behavior == SPI_CS_KEEP_LOW) {
		p_spi->SPI_CSR[ul_pcs_ch] |= SPI_CSR_CSAAT;
	}
}

/**
 * \brief Set number of bits per transfer.
 *
 * \param p_spi Pointer to an SPI instance.
 * \param ul_pcs_ch Peripheral Chip Select channel (0~3).
 * \param ul_bits Number of bits (8~16), use the pattern defined
 *        in the device header file.
 */
static void spi_set_bits_per_transfer(Spi *p_spi, uint32_t ul_pcs_ch,
		uint32_t ul_bits)
{
	p_spi->SPI_CSR[ul_pcs_ch] &= (~SPI_CSR_BITS_Msk);
	p_spi->SPI_CSR[ul_pcs_ch] |= ul_bits;
}

/**
 * \brief Calculate the baudrate divider.
 *
 * \param baudrate Baudrate value.
 * \param mck      SPI module input clock frequency (MCK clock, Hz).
 *
 * \return Divider or error code.
 *   \retval > 0  Success.
 *   \retval < 0  Error.
 */
static int16_t spi_calc_baudrate_div(const uint32_t baudrate, uint32_t mck)
{
	int baud_div = div_ceil(mck, baudrate);

	/* The value of baud_div is from 1 to 255 in the SCBR field. */
	if (baud_div <= 0 || baud_div > 255) {
		return -1;
	}

	return baud_div;
}

/**
 * \brief Set Serial Clock Baud Rate divider value (SCBR).
 *
 * \param p_spi Pointer to an SPI instance.
 * \param ul_pcs_ch Peripheral Chip Select channel (0~3).
 * \param uc_baudrate_divider Baudrate divider from MCK.
 *
 * \return
 * \retval is 0 Success.
 * \retval is -1 Error.
 */
static int16_t spi_set_baudrate_div(Spi *p_spi, uint32_t ul_pcs_ch,
		uint8_t uc_baudrate_divider)
{
    /* Programming the SCBR field to 0 is forbidden */
    if (!uc_baudrate_divider)
        return -1;

	p_spi->SPI_CSR[ul_pcs_ch] &= (~SPI_CSR_SCBR_Msk);
	p_spi->SPI_CSR[ul_pcs_ch] |= SPI_CSR_SCBR(uc_baudrate_divider);
    return 0;
}

/**
 * \brief Configure timing for SPI transfer.
 *
 * \param p_spi Pointer to an SPI instance.
 * \param ul_pcs_ch Peripheral Chip Select channel (0~3).
 * \param uc_dlybs Delay before SPCK (in number of MCK clocks).
 * \param uc_dlybct Delay between consecutive transfers (in number of MCK clocks).
 */
static void spi_set_transfer_delay(Spi *p_spi, uint32_t ul_pcs_ch,
		uint8_t uc_dlybs, uint8_t uc_dlybct)
{
	p_spi->SPI_CSR[ul_pcs_ch] &= ~(SPI_CSR_DLYBS_Msk | SPI_CSR_DLYBCT_Msk);
	p_spi->SPI_CSR[ul_pcs_ch] |= SPI_CSR_DLYBS(uc_dlybs)
			| SPI_CSR_DLYBCT(uc_dlybct);
}


/**
 * \brief Enable or disable write protection of SPI registers.
 *
 * \param p_spi Pointer to an SPI instance.
 * \param ul_enable 1 to enable, 0 to disable.
 */
static void spi_set_writeprotect(Spi *p_spi, uint32_t ul_enable)
{
#if SAM4L
	if (ul_enable) {
		p_spi->SPI_WPCR = SPI_WPCR_SPIWPKEY_VALUE | SPI_WPCR_SPIWPEN;
	} else {
		p_spi->SPI_WPCR = SPI_WPCR_SPIWPKEY_VALUE;
	}
#else
	if (ul_enable) {
		p_spi->SPI_WPMR = SPI_WPMR_WPKEY_PASSWD | SPI_WPMR_WPEN;
	} else {
		p_spi->SPI_WPMR = SPI_WPMR_WPKEY_PASSWD;
	}
#endif
}

/**
 * \brief Indicate write protect status.
 *
 * \param p_spi Pointer to an SPI instance.
 *
 * \return SPI_WPSR value.
 */
static uint32_t spi_get_writeprotect_status(Spi *p_spi)
{
	return p_spi->SPI_WPSR;
}

/* Driver API Structure */
static const struct spi_driver_api spi_sam_driver_api = {
	.transceive = spi_sam_transceive,
#ifdef CONFIG_POLL
	.transceive_async = spi_sam_transceive_async,
#endif
	.release = spi_sam_release,
};

static int spi_sam_init(struct device *dev)
{
	struct spi_stm32_data *data __attribute__((unused)) = dev->driver_data;
	const struct spi_stm32_config *cfg = dev->config->config_info;

	__ASSERT_NO_MSG(device_get_binding(STM32_CLOCK_CONTROL_NAME));

	clock_control_on(device_get_binding(STM32_CLOCK_CONTROL_NAME),
			       (clock_control_subsys_t) &cfg->pclken);

#ifdef CONFIG_SPI_SAM_INTERRUPT
	cfg->irq_config(dev);
#endif

	spi_context_unlock_unconditionally(&data->ctx);

	return 0;
}


#define SPI_SAM_DRIVER_INIT(_id_)														\
DEVICE_DECLARE(spi_sam_##_id_);		\
\
static void spi_sam_irq_config_##_id_(struct device *dev) {				\
	IRQ_CONNECT(ATMEL_SAM_SPI_##_id_##_IRQ_0, ATMEL_SAM_SPI_##_id_##_IRQ_0_PRIORITY, 		\
		spi_sam_isr, DEVICE_GET(spi_sam_##_id_), 0);		\
	irq_enable(ATMEL_SAM_SPI_##_id_##_IRQ_0);		\
}				\
static const struct spi_sam_config spi_sam_cfg_##_id_ = {		\
	.regs = (void*)ATMEL_SAM_SPI_##_id_##_BASE_ADDRESS_0,		\
	.periph_id = ATMEL_SAM_SPI_##_id_##_IRQ_0,					\
	.irq_config = spi_stm32_irq_config_func_##_id_##,			\
};	\
	\
static struct spi_sam_data spi_sam_dev_data_##_id_ = {		\
	SPI_CONTEXT_INIT_LOCK(spi_stm32_dev_data_##_id_, ctx),	\
	SPI_CONTEXT_INIT_SYNC(spi_stm32_dev_data_##_id_, ctx),	\
};	\
\
DEVICE_AND_API_INIT(spi_sam_##_id_, ATMEL_SAM_SPI_##_id_##_LABEL, 				\
			&spi_sam_init, &spi_sam_dev_data_##_id_, &spi_stm32_cfg_##_id_,		\
		    POST_KERNEL, CONFIG_SPI_INIT_PRIORITY, &spi_sam_driver_api);

#if CONFIG_SPI_0
SPI_SAM_DRIVER_INIT(0);
#endif

#if CONFIG_SPI_1
SPI_SAM_DRIVER_INIT(1);
#endif

#if CONFIG_SPI_2
SPI_SAM_DRIVER_INIT(2);
#endif

#if CONFIG_SPI_3
SPI_SAM_DRIVER_INIT(3);
#endif





















#if 0




/*
 * Copyright (c) 2016 BayLibre, SAS
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define SYS_LOG_LEVEL CONFIG_SYS_LOG_SPI_LEVEL
#include <logging/sys_log.h>

#include <misc/util.h>
#include <kernel.h>
#include <board.h>
#include <errno.h>
#include <spi.h>

#include <clock_control/stm32_clock_control.h>
#include <clock_control.h>

#include <drivers/spi/spi_ll_stm32.h>
#include <spi_ll_stm32.h>

#define CONFIG_CFG(cfg)						\
((const struct spi_stm32_config * const)(cfg)->dev->config->config_info)

#define CONFIG_DATA(cfg)					\
((struct spi_stm32_data * const)(cfg)->dev->driver_data)

#ifdef LL_SPI_SR_UDR
#define SPI_STM32_ERR_MSK (LL_SPI_SR_UDR | LL_SPI_SR_CRCERR | LL_SPI_SR_MODF | \
			   LL_SPI_SR_OVR | LL_SPI_SR_FRE)
#else
#define SPI_STM32_ERR_MSK (LL_SPI_SR_CRCERR | LL_SPI_SR_MODF | \
			   LL_SPI_SR_OVR | LL_SPI_SR_FRE)
#endif

/* Value to shift out when no application data needs transmitting. */
#define SPI_STM32_TX_NOP 0x00

static bool spi_stm32_transfer_ongoing(struct spi_stm32_data *data)
{
	return spi_context_tx_on(&data->ctx) || spi_context_rx_on(&data->ctx);
}

static int spi_stm32_get_err(SPI_TypeDef *spi)
{
	u32_t sr = LL_SPI_ReadReg(spi, SR);

	return (int)(sr & SPI_STM32_ERR_MSK);
}

static inline u8_t spi_stm32_next_tx(struct spi_stm32_data *data)
{
	return spi_context_tx_on(&data->ctx) ?
		*data->ctx.tx_buf : SPI_STM32_TX_NOP;
}

/* Shift a SPI frame as master. */
static void spi_stm32_shift_m(SPI_TypeDef *spi, struct spi_stm32_data *data)
{
	u8_t tx_frame;
	u8_t rx_frame;

	tx_frame = spi_stm32_next_tx(data);
	while (!LL_SPI_IsActiveFlag_TXE(spi)) {
		/* NOP */
	}
	LL_SPI_TransmitData8(spi, tx_frame);
	/* The update is ignored if TX is off. */
	spi_context_update_tx(&data->ctx, 1);

	while (!LL_SPI_IsActiveFlag_RXNE(spi)) {
		/* NOP */
	}
	rx_frame = LL_SPI_ReceiveData8(spi);
	if (spi_context_rx_on(&data->ctx)) {
		*data->ctx.rx_buf = rx_frame;
		spi_context_update_rx(&data->ctx, 1);
	}
}

/* Shift a SPI frame as slave. */
static void spi_stm32_shift_s(SPI_TypeDef *spi, struct spi_stm32_data *data)
{
	u8_t tx_frame;
	u8_t rx_frame;

	tx_frame = spi_stm32_next_tx(data);
	if (LL_SPI_IsActiveFlag_TXE(spi)) {
		LL_SPI_TransmitData8(spi, tx_frame);
		/* The update is ignored if TX is off. */
		spi_context_update_tx(&data->ctx, 1);
	}

	if (LL_SPI_IsActiveFlag_RXNE(spi)) {
		rx_frame = LL_SPI_ReceiveData8(spi);
		if (spi_context_rx_on(&data->ctx)) {
			*data->ctx.rx_buf = rx_frame;
			spi_context_update_rx(&data->ctx, 1);
		}
	}
}

/*
 * Without a FIFO, we can only shift out one frame's worth of SPI
 * data, and read the response back.
 *
 * TODO: support 16-bit data frames.
 */
static int spi_stm32_shift_frames(SPI_TypeDef *spi, struct spi_stm32_data *data)
{
	u16_t operation = data->ctx.config->operation;

	if (SPI_OP_MODE_GET(operation) == SPI_OP_MODE_MASTER) {
		spi_stm32_shift_m(spi, data);
	} else {
		spi_stm32_shift_s(spi, data);
	}

	return spi_stm32_get_err(spi);
}

static void spi_stm32_complete(struct spi_stm32_data *data, SPI_TypeDef *spi,
			       int status)
{
#ifdef CONFIG_SPI_STM32_INTERRUPT
	LL_SPI_DisableIT_TXE(spi);
	LL_SPI_DisableIT_RXNE(spi);
	LL_SPI_DisableIT_ERR(spi);
#endif

	spi_context_cs_control(&data->ctx, false);

#if defined(CONFIG_SOC_SERIES_STM32L4X) || defined(CONFIG_SOC_SERIES_STM32F3X)
	/* Flush RX buffer */
	while (LL_SPI_IsActiveFlag_RXNE(spi)) {
		(void) LL_SPI_ReceiveData8(spi);
	}
#endif

	if (LL_SPI_GetMode(spi) == LL_SPI_MODE_MASTER) {
		while (LL_SPI_IsActiveFlag_BSY(spi)) {
			/* NOP */
		}
	}

	LL_SPI_Disable(spi);

#ifdef CONFIG_SPI_STM32_INTERRUPT
	spi_context_complete(&data->ctx, status);
#endif
}

#ifdef CONFIG_SPI_STM32_INTERRUPT
static void spi_stm32_isr(void *arg)
{
	struct device * const dev = (struct device *) arg;
	const struct spi_stm32_config *cfg = dev->config->config_info;
	struct spi_stm32_data *data = dev->driver_data;
	SPI_TypeDef *spi = cfg->spi;
	int err;

	err = spi_stm32_get_err(spi);
	if (err) {
		spi_stm32_complete(data, spi, err);
		return;
	}

	if (spi_stm32_transfer_ongoing(data)) {
		err = spi_stm32_shift_frames(spi, data);
	}

	if (err || !spi_stm32_transfer_ongoing(data)) {
		spi_stm32_complete(data, spi, err);
	}
}
#endif

static int spi_stm32_configure(struct spi_config *config)
{
	const struct spi_stm32_config *cfg = CONFIG_CFG(config);
	struct spi_stm32_data *data = CONFIG_DATA(config);
	const u32_t scaler[] = {
		LL_SPI_BAUDRATEPRESCALER_DIV2,
		LL_SPI_BAUDRATEPRESCALER_DIV4,
		LL_SPI_BAUDRATEPRESCALER_DIV8,
		LL_SPI_BAUDRATEPRESCALER_DIV16,
		LL_SPI_BAUDRATEPRESCALER_DIV32,
		LL_SPI_BAUDRATEPRESCALER_DIV64,
		LL_SPI_BAUDRATEPRESCALER_DIV128,
		LL_SPI_BAUDRATEPRESCALER_DIV256
	};
	SPI_TypeDef *spi = cfg->spi;
	u32_t clock;
	int br;

	if (spi_context_configured(&data->ctx, config)) {
		/* Nothing to do */
		return 0;
	}

	if (SPI_WORD_SIZE_GET(config->operation) != 8) {
		return -ENOTSUP;
	}

	clock_control_get_rate(device_get_binding(STM32_CLOCK_CONTROL_NAME),
			       (clock_control_subsys_t) &cfg->pclken, &clock);

	for (br = 1 ; br <= ARRAY_SIZE(scaler) ; ++br) {
		u32_t clk = clock >> br;

		if (clk < config->frequency) {
			break;
		}
	}

	if (br > ARRAY_SIZE(scaler)) {
		SYS_LOG_ERR("Unsupported frequency %uHz, max %uHz, min %uHz",
			    config->frequency,
			    clock >> 1,
			    clock >> ARRAY_SIZE(scaler));
		return -EINVAL;
	}

	LL_SPI_Disable(spi);
	LL_SPI_SetBaudRatePrescaler(spi, scaler[br - 1]);

	if (SPI_MODE_GET(config->operation) ==  SPI_MODE_CPOL) {
		LL_SPI_SetClockPolarity(spi, LL_SPI_POLARITY_HIGH);
	} else {
		LL_SPI_SetClockPolarity(spi, LL_SPI_POLARITY_LOW);
	}

	if (SPI_MODE_GET(config->operation) == SPI_MODE_CPHA) {
		LL_SPI_SetClockPhase(spi, LL_SPI_PHASE_2EDGE);
	} else {
		LL_SPI_SetClockPhase(spi, LL_SPI_PHASE_1EDGE);
	}

	LL_SPI_SetTransferDirection(spi, LL_SPI_FULL_DUPLEX);

	if (config->operation & SPI_TRANSFER_LSB) {
		LL_SPI_SetTransferBitOrder(spi, LL_SPI_LSB_FIRST);
	} else {
		LL_SPI_SetTransferBitOrder(spi, LL_SPI_MSB_FIRST);
	}

	LL_SPI_DisableCRC(spi);

	if (config->operation & SPI_OP_MODE_SLAVE) {
		LL_SPI_SetMode(spi, LL_SPI_MODE_SLAVE);
	} else {
		LL_SPI_SetMode(spi, LL_SPI_MODE_MASTER);
	}

	if (config->vendor & STM32_SPI_NSS_IGNORE) {
		LL_SPI_SetNSSMode(spi, LL_SPI_NSS_SOFT);
	} else {
		if (config->operation & SPI_OP_MODE_SLAVE) {
			LL_SPI_SetNSSMode(spi, LL_SPI_NSS_HARD_INPUT);
		} else {
			LL_SPI_SetNSSMode(spi, LL_SPI_NSS_HARD_OUTPUT);
		}
	}

	LL_SPI_SetDataWidth(spi, LL_SPI_DATAWIDTH_8BIT);

#if defined(CONFIG_SOC_SERIES_STM32L4X) || defined(CONFIG_SOC_SERIES_STM32F3X)
	LL_SPI_SetRxFIFOThreshold(spi, LL_SPI_RX_FIFO_TH_QUARTER);
#endif
	LL_SPI_SetStandard(spi, LL_SPI_PROTOCOL_MOTOROLA);

	/* At this point, it's mandatory to set this on the context! */
	data->ctx.config = config;

	spi_context_cs_configure(&data->ctx);

	SYS_LOG_DBG("Installed config %p: freq %uHz (div = %u),"
		    " mode %u/%u/%u, slave %u",
		    config, clock >> br, 1 << br,
		    (SPI_MODE_GET(config->operation) & SPI_MODE_CPOL) ? 1 : 0,
		    (SPI_MODE_GET(config->operation) & SPI_MODE_CPHA) ? 1 : 0,
		    (SPI_MODE_GET(config->operation) & SPI_MODE_LOOP) ? 1 : 0,
		    config->slave);

	return 0;
}

static int spi_stm32_release(struct spi_config *config)
{
	struct spi_stm32_data *data = CONFIG_DATA(config);

	spi_context_unlock_unconditionally(&data->ctx);

	return 0;
}

static int transceive(struct spi_config *config,
		      const struct spi_buf *tx_bufs, u32_t tx_count,
		      struct spi_buf *rx_bufs, u32_t rx_count,
		      bool asynchronous, struct k_poll_signal *signal)
{
	const struct spi_stm32_config *cfg = CONFIG_CFG(config);
	struct spi_stm32_data *data = CONFIG_DATA(config);
	SPI_TypeDef *spi = cfg->spi;
	int ret;

	if (!tx_count && !rx_count) {
		return 0;
	}

#ifndef CONFIG_SPI_STM32_INTERRUPT
	if (asynchronous) {
		return -ENOTSUP;
	}
#endif

	spi_context_lock(&data->ctx, asynchronous, signal);

	ret = spi_stm32_configure(config);
	if (ret) {
		return ret;
	}

	/* Set buffers info */
	spi_context_buffers_setup(&data->ctx, tx_bufs, tx_count,
				  rx_bufs, rx_count, 1);

#if defined(CONFIG_SOC_SERIES_STM32L4X) || defined(CONFIG_SOC_SERIES_STM32F3X)
	/* Flush RX buffer */
	while (LL_SPI_IsActiveFlag_RXNE(spi)) {
		(void) LL_SPI_ReceiveData8(spi);
	}
#endif

	LL_SPI_Enable(spi);

	/* This is turned off in spi_stm32_complete(). */
	spi_context_cs_control(&data->ctx, true);

#ifdef CONFIG_SPI_STM32_INTERRUPT
	LL_SPI_EnableIT_ERR(spi);

	if (rx_bufs) {
		LL_SPI_EnableIT_RXNE(spi);
	}

	LL_SPI_EnableIT_TXE(spi);

	ret = spi_context_wait_for_completion(&data->ctx);
#else
	do {
		ret = spi_stm32_shift_frames(spi, data);
	} while (!ret && spi_stm32_transfer_ongoing(data));

	spi_stm32_complete(data, spi, ret);
#endif

	spi_context_release(&data->ctx, ret);

	if (ret) {
		SYS_LOG_ERR("error mask 0x%x", ret);
	}

	return ret ? -EIO : 0;
}

static int spi_stm32_transceive(struct spi_config *config,
				const struct spi_buf *tx_bufs, u32_t tx_count,
				struct spi_buf *rx_bufs, u32_t rx_count)
{
	return transceive(config, tx_bufs, tx_count,
			  rx_bufs, rx_count, false, NULL);
}

#ifdef CONFIG_POLL
static int spi_stm32_transceive_async(struct spi_config *config,
				      const struct spi_buf *tx_bufs,
				      size_t tx_count,
				      struct spi_buf *rx_bufs,
				      size_t rx_count,
				      struct k_poll_signal *async)
{
	return transceive(config, tx_bufs, tx_count,
			  rx_bufs, rx_count, true, async);
}
#endif /* CONFIG_POLL */

static const struct spi_driver_api api_funcs = {
	.transceive = spi_stm32_transceive,
#ifdef CONFIG_POLL
	.transceive_async = spi_stm32_transceive_async,
#endif
	.release = spi_stm32_release,
};

static int spi_stm32_init(struct device *dev)
{
	struct spi_stm32_data *data __attribute__((unused)) = dev->driver_data;
	const struct spi_stm32_config *cfg = dev->config->config_info;

	__ASSERT_NO_MSG(device_get_binding(STM32_CLOCK_CONTROL_NAME));

	clock_control_on(device_get_binding(STM32_CLOCK_CONTROL_NAME),
			       (clock_control_subsys_t) &cfg->pclken);

#ifdef CONFIG_SPI_STM32_INTERRUPT
	cfg->irq_config(dev);
#endif

	spi_context_unlock_unconditionally(&data->ctx);

	return 0;
}

#ifdef CONFIG_SPI_1

#ifdef CONFIG_SPI_STM32_INTERRUPT
static void spi_stm32_irq_config_func_1(struct device *port);
#endif

static const struct spi_stm32_config spi_stm32_cfg_1 = {
	.spi = (SPI_TypeDef *) SPI1_BASE,
	.pclken = {
		.enr = LL_APB2_GRP1_PERIPH_SPI1,
		.bus = STM32_CLOCK_BUS_APB2
	},
#ifdef CONFIG_SPI_STM32_INTERRUPT
	.irq_config = spi_stm32_irq_config_func_1,
#endif
};

static struct spi_stm32_data spi_stm32_dev_data_1 = {
	SPI_CONTEXT_INIT_LOCK(spi_stm32_dev_data_1, ctx),
	SPI_CONTEXT_INIT_SYNC(spi_stm32_dev_data_1, ctx),
};

DEVICE_AND_API_INIT(spi_stm32_1, CONFIG_SPI_1_NAME, &spi_stm32_init,
		    &spi_stm32_dev_data_1, &spi_stm32_cfg_1,
		    POST_KERNEL, CONFIG_SPI_INIT_PRIORITY,
		    &api_funcs);

#ifdef CONFIG_SPI_STM32_INTERRUPT
static void spi_stm32_irq_config_func_1(struct device *dev)
{
	IRQ_CONNECT(SPI1_IRQn, CONFIG_SPI_1_IRQ_PRI,
		    spi_stm32_isr, DEVICE_GET(spi_stm32_1), 0);
	irq_enable(SPI1_IRQn);
}
#endif

#endif /* CONFIG_SPI_1 */

#ifdef CONFIG_SPI_2

#ifdef CONFIG_SPI_STM32_INTERRUPT
static void spi_stm32_irq_config_func_2(struct device *port);
#endif

static const struct spi_stm32_config spi_stm32_cfg_2 = {
	.spi = (SPI_TypeDef *) SPI2_BASE,
	.pclken = {
		.enr = LL_APB1_GRP1_PERIPH_SPI2,
		.bus = STM32_CLOCK_BUS_APB1
	},
#ifdef CONFIG_SPI_STM32_INTERRUPT
	.irq_config = spi_stm32_irq_config_func_2,
#endif
};

static struct spi_stm32_data spi_stm32_dev_data_2 = {
	SPI_CONTEXT_INIT_LOCK(spi_stm32_dev_data_2, ctx),
	SPI_CONTEXT_INIT_SYNC(spi_stm32_dev_data_2, ctx),
};

DEVICE_AND_API_INIT(spi_stm32_2, CONFIG_SPI_2_NAME, &spi_stm32_init,
		    &spi_stm32_dev_data_2, &spi_stm32_cfg_2,
		    POST_KERNEL, CONFIG_SPI_INIT_PRIORITY,
		    &api_funcs);

#ifdef CONFIG_SPI_STM32_INTERRUPT
static void spi_stm32_irq_config_func_2(struct device *dev)
{
	IRQ_CONNECT(SPI2_IRQn, CONFIG_SPI_2_IRQ_PRI,
		    spi_stm32_isr, DEVICE_GET(spi_stm32_2), 0);
	irq_enable(SPI2_IRQn);
}
#endif

#endif /* CONFIG_SPI_2 */

#ifdef CONFIG_SPI_3

#ifdef CONFIG_SPI_STM32_INTERRUPT
static void spi_stm32_irq_config_func_3(struct device *port);
#endif

static const  struct spi_stm32_config spi_stm32_cfg_3 = {
	.spi = (SPI_TypeDef *) SPI3_BASE,
	.pclken = {
		.enr = LL_APB1_GRP1_PERIPH_SPI3,
		.bus = STM32_CLOCK_BUS_APB1
	},
#ifdef CONFIG_SPI_STM32_INTERRUPT
	.irq_config = spi_stm32_irq_config_func_3,
#endif
};

static struct spi_stm32_data spi_stm32_dev_data_3 = {
	SPI_CONTEXT_INIT_LOCK(spi_stm32_dev_data_3, ctx),
	SPI_CONTEXT_INIT_SYNC(spi_stm32_dev_data_3, ctx),
};

DEVICE_AND_API_INIT(spi_stm32_3, CONFIG_SPI_3_NAME, &spi_stm32_init,
		    &spi_stm32_dev_data_3, &spi_stm32_cfg_3,
		    POST_KERNEL, CONFIG_SPI_INIT_PRIORITY,
		    &api_funcs);

#ifdef CONFIG_SPI_STM32_INTERRUPT
static void spi_stm32_irq_config_func_3(struct device *dev)
{
	IRQ_CONNECT(SPI3_IRQn, CONFIG_SPI_3_IRQ_PRI,
		    spi_stm32_isr, DEVICE_GET(spi_stm32_3), 0);
	irq_enable(SPI3_IRQn);
}
#endif

#endif /* CONFIG_SPI_3 */

#endif
