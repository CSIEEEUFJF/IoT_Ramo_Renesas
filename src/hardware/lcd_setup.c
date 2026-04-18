#include "main_thread.h"
#include "hardware/lcd.h"

static const spi_cfg_t g_lcd_spi0_cfg =
{
    .channel        = 0,
    .rxi_ipl        = 12,
    .txi_ipl        = 12,
    .tei_ipl        = 12,
    .eri_ipl        = 12,
    .operating_mode = SPI_MODE_MASTER,
    .clk_phase      = SPI_CLK_PHASE_EDGE_EVEN,
    .clk_polarity   = SPI_CLK_POLARITY_HIGH,
    .mode_fault     = SPI_MODE_FAULT_ERROR_DISABLE,
    .bit_order      = SPI_BIT_ORDER_MSB_FIRST,
    .bitrate        = 100000,
    .p_transfer_tx  = g_spi0_P_TRANSFER_TX,
    .p_transfer_rx  = g_spi0_P_TRANSFER_RX,
    .p_callback     = g_lcd_spi_callback,
    .p_context      = (void *) &g_spi0,
    .p_extend       = (void *) &g_spi0_ext_cfg,
};

void g_lcd_spi_callback(spi_callback_args_t *p_args)
{
    if ((NULL != p_args) && (SPI_EVENT_TRANSFER_COMPLETE == p_args->event))
    {
        tx_semaphore_ceiling_put(&g_main_semaphore_lcdc, 1);
    }
}

static void lcd_write(uint8_t cmd, const uint8_t *data, uint32_t len)
{
    ssp_err_t err;

    g_ioport.p_api->pinWrite(LCD_CMD, IOPORT_LEVEL_LOW);
    g_ioport.p_api->pinWrite(LCD_CS, IOPORT_LEVEL_LOW);

    err = g_spi0.p_api->write(g_spi0.p_ctrl, &cmd, 1, SPI_BIT_WIDTH_8_BITS);
    if (SSP_SUCCESS != err)
    {
        while (1)
        {
        }
    }

    tx_semaphore_get(&g_main_semaphore_lcdc, TX_WAIT_FOREVER);

    if (len > 0U)
    {
        g_ioport.p_api->pinWrite(LCD_CMD, IOPORT_LEVEL_HIGH);

        err = g_spi0.p_api->write(g_spi0.p_ctrl, data, len, SPI_BIT_WIDTH_8_BITS);
        if (SSP_SUCCESS != err)
        {
            while (1)
            {
            }
        }

        tx_semaphore_get(&g_main_semaphore_lcdc, TX_WAIT_FOREVER);
    }

    g_ioport.p_api->pinWrite(LCD_CS, IOPORT_LEVEL_HIGH);
}

void lcd_setup(void)
{
    ssp_err_t err;

    err = g_spi0.p_api->open(g_spi0.p_ctrl, &g_lcd_spi0_cfg);
    if (SSP_ERR_ALREADY_OPEN == err)
    {
        (void) g_spi0.p_api->close(g_spi0.p_ctrl);
        err = g_spi0.p_api->open(g_spi0.p_ctrl, &g_lcd_spi0_cfg);
    }

    if (SSP_SUCCESS != err)
    {
        while (1)
        {
        }
    }

    g_ioport.p_api->pinWrite(LCD_CS, IOPORT_LEVEL_HIGH);
    g_ioport.p_api->pinWrite(LCD_RESET, IOPORT_LEVEL_HIGH);
    g_ioport.p_api->pinWrite(LCD_RESET, IOPORT_LEVEL_LOW);
    tx_thread_sleep(1);
    g_ioport.p_api->pinWrite(LCD_RESET, IOPORT_LEVEL_HIGH);

    /* 120 ms no sample da Renesas. */
    tx_thread_sleep(12);

    lcd_write(ILI9341_SW_RESET, NULL, 0U);
    tx_thread_sleep(5);

    lcd_write(ILI9341_POWERB,       (const uint8_t *) "\x00\xC1\x30", 3U);
    lcd_write(ILI9341_DTCA,         (const uint8_t *) "\x85\x00\x78", 3U);
    lcd_write(ILI9341_DTCB,         (const uint8_t *) "\x00\x00", 2U);
    lcd_write(ILI9341_POWERA,       (const uint8_t *) "\x39\x2C\x00\x34\x02", 5U);
    lcd_write(ILI9341_POWER_SEQ,    (const uint8_t *) "\x64\x03\x12\x81", 4U);
    lcd_write(ILI9341_PRC,          (const uint8_t *) "\x20", 1U);
    lcd_write(ILI9341_POWER1,       (const uint8_t *) "\x23", 1U);
    lcd_write(ILI9341_POWER2,       (const uint8_t *) "\x10", 1U);
    lcd_write(ILI9341_VCOM1,        (const uint8_t *) "\x3E\x28", 2U);
    lcd_write(ILI9341_VCOM2,        (const uint8_t *) "\x86", 1U);
    /* Rotaciona a imagem em 180 graus para coincidir com a montagem fisica atual. */
    lcd_write(ILI9341_MAC,          (const uint8_t *) "\x88", 1U);
    lcd_write(ILI9341_PIXEL_FORMAT, (const uint8_t *) "\x55", 1U);
    lcd_write(ILI9341_FRM_CTRL1,    (const uint8_t *) "\x00\x18", 2U);
    lcd_write(ILI9341_DFC,          (const uint8_t *) "\x08\x82\x27", 3U);
    lcd_write(ILI9341_3GAMMA_EN,    (const uint8_t *) "\x00", 1U);
    lcd_write(ILI9341_RGB_INTERFACE,(const uint8_t *) "\xC2", 1U);
    lcd_write(ILI9341_INTERFACE,    (const uint8_t *) "\x01\x00\x06", 3U);
    lcd_write(ILI9341_COLUMN_ADDR,  (const uint8_t *) "\x00\x00\x00\xEF", 4U);
    lcd_write(ILI9341_PAGE_ADDR,    (const uint8_t *) "\x00\x00\x01\x3F", 4U);
    lcd_write(ILI9341_GAMMA,        (const uint8_t *) "\x01", 1U);
    lcd_write(ILI9341_PGAMMA,       (const uint8_t *) "\x0F\x31\x2B\x0C\x0E\x08\x4E\xF1\x37\x07\x10\x03\x0E\x09\x00", 15U);
    lcd_write(ILI9341_NGAMMA,       (const uint8_t *) "\x00\x0E\x14\x03\x11\x07\x31\xC1\x48\x08\x0F\x0C\x31\x36\x0F", 15U);
    lcd_write(ILI9341_SLEEP_OUT,    (const uint8_t *) "\x00", 1U);
    tx_thread_sleep(2);
    lcd_write(ILI9341_DISP_ON,      (const uint8_t *) "\x00", 1U);

    g_ioport.p_api->pinWrite(LCD_BACKLIGHT, IOPORT_LEVEL_HIGH);
    (void) g_spi0.p_api->close(g_spi0.p_ctrl);
}
