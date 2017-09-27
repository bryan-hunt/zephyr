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

#define SYS_LOG_LEVEL 	CONFIG_SYS_LOG_SPI_LEVEL
#include <logging/sys_log.h>

#include <misc/util.h>
#include <kernel.h>
#include <board.h>
#include <errno.h>
#include <spi.h>
#include <spi_context.h>

/* Dummy byte to send when performing reads */
#define SPI_SAM_DUMMY_BYTE		0xFF

/* Driver Configuration */
struct spi_sam_config {
	Spi* 				regs;
	u32_t				periph_id;
	void (*irq_config)(void);
	const struct soc_gpio_pin *pin_list;
	u8_t pin_list_size;
};

/* Macro to get config info from SPI API: expects struct spi_config */
#define DEV_CFG(x)	((const struct spi_sam_config * const)((x)->config->config_info))
#define SPI_CFG(x)	DEV_CFG(x->dev)

/* Driver Data */
struct spi_sam_data {
	struct spi_context ctx;
};

/* Macro to get driver context from SPI API: expects struct spi_config */
#define DEV_CTX(x)	&((struct spi_sam_data * const)((x)->driver_data))->ctx
#define SPI_CTX(x)  DEV_CTX(x->dev)

/* Reset SPI Device and set it to Slave mode. */
static inline void spi_sam_reset(Spi * p_spi)
{
	p_spi->SPI_CR = SPI_CR_SWRST;
}

/* Enable SPI Device */
static inline void spi_sam_enable(Spi * p_spi)
{
	p_spi->SPI_CR = SPI_CR_SPIEN;
}

/* Disable SPI Device. CS is de-asserted, which indicates that the last data is done, and user
should check TX_EMPTY before disabling SPI. */
static inline void spi_sam_disable(Spi * p_spi)
{
	p_spi->SPI_CR = SPI_CR_SPIDIS;
}

/* Issue a LASTXFER command. The next transfer is the last transfer and after that CS is de-asserted. */
static inline void spi_sam_last_xfer(Spi * p_spi)
{
	p_spi->SPI_CR = SPI_CR_LASTXFER;
}

/* Set SPI Device to Master */
static inline void spi_sam_mode_master(Spi * p_spi)
{
	p_spi->SPI_MR |= SPI_MR_MSTR;
}

/* Set SPI Device to Slave mode */
static inline void spi_sam_mode_slave(Spi * p_spi)
{
	p_spi->SPI_MR &= (~SPI_MR_MSTR);
}

/* Get SPI mode */
static inline bool spi_sam_mode(Spi* p_spi)
{
	return (SPI_MR_MSTR == (p_spi->SPI_MR & SPI_MR_MSTR));
}

/* Read status register */
static inline u8_t spi_sam_status(Spi * p_spi)
{
	return p_spi->SPI_SR;
}

/* Send Data */
static inline void spi_sam_tx(Spi * p_spi, u16_t data)
{
	p_spi->SPI_TDR = SPI_TDR_TD(data);
}

/* Read data */
static inline u16_t spi_sam_rx(Spi * p_spi)
{
	return (p_spi->SPI_RDR & SPI_RDR_RD_Msk);
}

/* Check if TX is done */
static inline bool spi_sam_tx_empty(Spi * p_spi)
{
	return (SPI_SR_TXEMPTY == (p_spi->SPI_SR & SPI_SR_TXEMPTY));
}

/* Check if TX is ready for more */
static inline bool spi_sam_tx_ready(Spi * p_spi)
{
	return (SPI_SR_TDRE == (p_spi->SPI_SR & SPI_SR_TDRE));
}

/* Check if RX is done */
static inline bool spi_sam_rx_full(Spi * p_spi)
{
	return (SPI_SR_RDRF == (p_spi->SPI_SR & SPI_SR_RDRF));
}

/* Check if RX is ready */
static inline bool spi_sam_rx_ready(Spi * p_spi)
{
	return ((SPI_SR_RDRF | SPI_SR_TXEMPTY) == (p_spi->SPI_SR & (SPI_SR_RDRF | SPI_SR_TXEMPTY)));
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
	int baud_div = (((mck) + (baudrate) - 1) / (baudrate));

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
    if (!uc_baudrate_divider) {
        return -1;
    }

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

static inline bool spi_sam_is_active(struct spi_context * ctx)
{
	return (spi_context_tx_on(ctx) || spi_context_rx_on(ctx));
}

static inline u8_t spi_sam_get_tx(struct spi_context * ctx)
{
	if(spi_context_tx_on(ctx)) {
		return *ctx->tx_buf;
	}
	else {
		return SPI_SAM_DUMMY_BYTE;
	}
}

static inline void spi_sam_put_rx(struct spi_context *ctx, u8_t rx_data)
{
	if (spi_context_rx_on(ctx)) {
		*ctx->rx_buf = rx_data;
		spi_context_update_rx(ctx, 1, 1);
	}
}

/* Configuration of the peripheral based on  */
static int spi_sam_configure(struct spi_config *config)
{
	Spi * p_spi = SPI_CFG(config)->regs;
	struct spi_context * ctx = SPI_CTX(config);

	u32_t clk_div;

	/* Check if the device has already been configured */
	if (spi_context_configured(ctx, config)) {
		/* Nothing to do */
		return 0;
	}

	if (SPI_WORD_SIZE_GET(config->operation) != 8) {
		return -ENOTSUP;
	}

	/* Check the request clock rate */
	clk_div = spi_calc_baudrate_div(config->frequency, SOC_ATMEL_SAM_MCK_FREQ_HZ);

	/* Make sure the peripheral clock is enabled first */
	soc_pmc_peripheral_enable(SPI_CFG(config)->periph_id);

	/* Disable the Module */
	spi_sam_disable(p_spi);

	/* Configure the new clock rate */
	spi_set_baudrate_div(p_spi, 0, clk_div);

	/* Configure clock polarity */
	spi_set_clock_polarity(p_spi, 0, (SPI_MODE_CPOL==SPI_MODE_GET(config->operation)));

	/* Configure edge sampling */
	spi_set_clock_phase(p_spi, 0, (SPI_MODE_CPHA == SPI_MODE_GET(config->operation)));

	/* Configure bit transfer direction */
	if (config->operation & SPI_TRANSFER_LSB) {
		return -ENOTSUP;
	}

	/* Set operational mode */
	if (config->operation & SPI_OP_MODE_SLAVE) {
		spi_sam_mode_slave(p_spi);
	} else {
		spi_sam_mode_master(p_spi);
	}

	/* Configure data transfer width */
	spi_set_bits_per_transfer(p_spi, 0, SPI_WORD_SIZE_GET(config->operation));

	/* At this point, it's mandatory to set this on the context! */
	ctx->config = config;

	/* System API chip select configuration */
	spi_context_cs_configure(ctx);

	SYS_LOG_DBG("Installed config %p: freq %uHz (div = %u),"
		    " mode %u/%u/%u, slave %u",
		    config, clock >> br, 1 << br,
		    (SPI_MODE_GET(config->operation) & SPI_MODE_CPOL) ? 1 : 0,
		    (SPI_MODE_GET(config->operation) & SPI_MODE_CPHA) ? 1 : 0,
		    (SPI_MODE_GET(config->operation) & SPI_MODE_LOOP) ? 1 : 0,
		    config->slave);

	return 0;
}

static void spi_sam_shift_frames(Spi *p_spi, struct spi_context *ctx)
{
	if(spi_sam_tx_ready(p_spi))
	{
		spi_sam_tx(p_spi, spi_sam_get_tx(ctx));

		spi_context_update_tx(ctx, 1, 1);
	}

	if(spi_sam_rx_full(p_spi))
	{
		spi_sam_put_rx(ctx, spi_sam_rx(p_spi));
	}
}

static void spi_sam_isr(void *arg)
{
	Spi *p_spi = DEV_CFG((struct device*)arg)->regs;
	struct spi_context *ctx = DEV_CTX((struct device*)arg);

	if (spi_sam_is_active(ctx)) {
		spi_sam_shift_frames(p_spi, ctx);
	}

	if (!spi_sam_is_active(ctx)) {
		spi_context_cs_control(ctx, false);
		spi_context_complete(ctx, true);
	}
}

static int spi_sam_release(struct spi_config *config)
{
	spi_context_unlock_unconditionally(SPI_CTX(config));
	return 0;
}

static int spi_sam_transceive_internal(struct spi_config *config,
		      const struct spi_buf *tx_bufs, u32_t tx_count,
		      struct spi_buf *rx_bufs, u32_t rx_count,
		      bool asynchronous, struct k_poll_signal *signal)
{
	Spi *p_spi = SPI_CFG(config)->regs;
	struct spi_context * ctx = SPI_CTX(config);
	int ret;

	if (!tx_count && !rx_count) {
		return 0;
	}

	spi_context_lock(ctx, asynchronous, signal);

	ret = spi_sam_configure(config);
	if (ret) {
		return ret;
	}

	/* Set buffers info */
	spi_context_buffers_setup(ctx, tx_bufs, tx_count,
				  rx_bufs, rx_count, 1);

	spi_sam_enable(p_spi);

	/* Assert the chip select */
	spi_context_cs_control(ctx, true);

#ifdef CONFIG_SPI_SAM_INTERRUPT
	if (rx_bufs) {
		LL_SPI_EnableIT_RXNE(spi);
	}

	LL_SPI_EnableIT_TXE(spi);

	ret = spi_context_wait_for_completion(&data->ctx);
#else
	do {
		spi_sam_shift_frames(p_spi, ctx);
	} while (spi_sam_is_active(ctx));

	spi_context_cs_control(ctx, false);
#endif

	spi_context_release(ctx, ret);

	if (ret) {
		SYS_LOG_ERR("error mask 0x%x", ret);
	}

	return ret ? -EIO : 0;
}

static int spi_sam_transceive(struct spi_config *config,
				const struct spi_buf *tx_bufs, u32_t tx_count,
				struct spi_buf *rx_bufs, u32_t rx_count)
{
	return spi_sam_transceive_internal(config, tx_bufs, tx_count,
			  rx_bufs, rx_count, false, NULL);
}

#if CONFIG_POLL

static int spi_sam_transceive_async(struct spi_config *config,
				const struct spi_buf *tx_bufs, u32_t tx_count,
				struct spi_buf *rx_bufs, u32_t rx_count,
				struct k_poll_signal *async)
{
	return spi_sam_transceive_internal(config, tx_bufs, tx_count,
			  rx_bufs, rx_count, true, async);
}
#endif

/* Driver API Structure */
static const struct spi_driver_api spi_sam_driver_api = {
	.transceive = spi_sam_transceive,
#if CONFIG_POLL
	.transceive_async = spi_sam_transceive_async,
#endif
	.release = spi_sam_release,
};

static int spi_sam_init(struct device *dev)
{
	struct spi_sam_data *data;
	const struct spi_sam_config *cfg;

	__ASSERT_NO_MSG(dev);
	__ASSERT_NO_MSG(dev->config);

	data = dev->driver_data;
	__ASSERT_NO_MSG(data);
	cfg = dev->config->config_info;
	__ASSERT_NO_MSG(cfg);

	soc_pmc_peripheral_enable(cfg->periph_id);

	/* Connect pins to the peripheral */
	soc_gpio_list_configure(cfg->pin_list, cfg->pin_list_size);

#if CONFIG_SPI_SAM_INTERRUPT
	cfg->irq_config(dev);
	irq_enable(cfg->periph_id);
#endif

	spi_context_unlock_unconditionally(&data->ctx);

	return 0;
}

/* Get the DTS defined configuration based on the DTS id (which is the base address of the module) */
#define DTS_CONFIG(_dtsid_, _field_)	ATMEL_SAM_SPI_ ## _dtsid_ ## _ ## _field_

/* Macro Expansion to get the base address of a module given it's DTS id */
#define DTS_BASE(x) 	DTS_CONFIG(x, BASE_ADDRESS_0)

/* Driver Definition per instance of the device */
#define SPI_SAM_DEVICE_INIT(_id_, _did_)						\
DEVICE_DECLARE(spi_sam_##_id_);									\
static void spi_sam_irq_config_##_id_(void) {					\
	IRQ_CONNECT(DTS_CONFIG(_did_, IRQ_0),						\
		DTS_CONFIG(_did_, IRQ_0_PRIORITY), 						\
		spi_sam_isr, DEVICE_GET(spi_sam_##_id_), 0);			\
}																\
static const struct soc_gpio_pin spi_sam_pins_##_id_[] = PINS_SPI##_id_;	\
static const struct spi_sam_config spi_sam_cfg_##_id_ = {		\
	.regs = (void*)DTS_BASE(_did_),								\
	.periph_id = DTS_CONFIG(_did_, IRQ_0),						\
	.irq_config = spi_sam_irq_config_##_id_,					\
	.pin_list = spi_sam_pins_##_id_,							\
	.pin_list_size = ARRAY_SIZE(spi_sam_pins_##_id_)			\
};																\
static struct spi_sam_data spi_sam_dev_data_##_id_ = {			\
	SPI_CONTEXT_INIT_LOCK(spi_sam_dev_data_##_id_, ctx),		\
	SPI_CONTEXT_INIT_SYNC(spi_sam_dev_data_##_id_, ctx),		\
};																\
DEVICE_AND_API_INIT(spi_sam_##_id_, 							\
	DTS_CONFIG(_did_, LABEL), 									\
	&spi_sam_init, 												\
	&spi_sam_dev_data_##_id_, 									\
	&spi_sam_cfg_##_id_,										\
	POST_KERNEL, CONFIG_SPI_INIT_PRIORITY,						\
	&spi_sam_driver_api);

#if DTS_BASE(CONFIG_SPI_SAM_0_DTS_ID)
SPI_SAM_DEVICE_INIT(0, CONFIG_SPI_SAM_0_DTS_ID);
#endif

#if DTS_BASE(CONFIG_SPI_SAM_1_DTS_ID)
SPI_SAM_DEVICE_INIT(1, CONFIG_SPI_SAM_1_DTS_ID);
#endif

#if DTS_BASE(CONFIG_SPI_SAM_2_DTS_ID)
SPI_SAM_DEVICE_INIT(2, CONFIG_SPI_SAM_2_DTS_ID);
#endif

#if DTS_BASE(CONFIG_SPI_SAM_3_DTS_ID)
SPI_SAM_DEVICE_INIT(3, CONFIG_SPI_SAM_3_DTS_ID);
#endif

#if DTS_BASE(CONFIG_SPI_SAM_4_DTS_ID)
SPI_SAM_DEVICE_INIT(4, CONFIG_SPI_SAM_4_DTS_ID);
#endif

#if DTS_BASE(CONFIG_SPI_SAM_5_DTS_ID)
SPI_SAM_DEVICE_INIT(5, CONFIG_SPI_SAM_5_DTS_ID);
#endif

