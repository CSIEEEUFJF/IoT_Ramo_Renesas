#include "ui.h"
#include "main.h"
#include "storage.h"
#include "hardware/lcd.h"
#include "assets/arial_font_asset.h"
#include "assets/ramo_logo_asset.h"
#include <stdio.h>

#define UI_SCREEN_WIDTH        240
#define UI_SCREEN_HEIGHT       320
#define UI_FRAMEBUFFER_STRIDE  256
#define UI_FONT_SPACING        0
#define UI_QUEUE_POLL_TICKS    1U
#define UI_STATUS_HOLD_TICKS   (TX_TIMER_TICKS_PER_SECOND * 15U)
#define UI_BACKLIGHT_TIMEOUT_TICKS (TX_TIMER_TICKS_PER_SECOND * 60U)
#define UI_TOUCH_DEBOUNCE_TICKS (TX_TIMER_TICKS_PER_SECOND / 20U)
#define UI_PIN_LENGTH          4U
#define UI_ADMIN_PIN           "1234"
#define UI_LOGO_IDLE_WIDTH     200
#define UI_LOGO_IDLE_HEIGHT    200
#define UI_LOGO_WAIT_WIDTH     216
#define UI_LOGO_WAIT_HEIGHT    51
#define UI_GEAR_BUTTON_SIZE    28
#define UI_GEAR_TOOTH_SIZE     4
#define UI_GEAR_CENTER_SIZE    8
#define UI_PIN_BOX_WIDTH       38
#define UI_PIN_BOX_HEIGHT      48
#define UI_PIN_BOX_GAP         10
#define UI_PIN_KEY_WIDTH       64
#define UI_PIN_KEY_HEIGHT      42
#define UI_PIN_KEY_GAP         8
#define UI_PIN_KEY_START_Y     116
#define UI_PIN_HIT_PADDING     6
#define UI_PIN_CARD_X          10
#define UI_PIN_CARD_Y          18
#define UI_PIN_CARD_WIDTH      220
#define UI_PIN_CARD_HEIGHT     284
#define UI_PIN_DISPLAY_X       28
#define UI_PIN_DISPLAY_Y       44
#define UI_PIN_DISPLAY_WIDTH   184
#define UI_PIN_DISPLAY_HEIGHT  52
#define UI_ENROLL_BACK_X       20
#define UI_ENROLL_BACK_Y       264
#define UI_ENROLL_BACK_WIDTH   92
#define UI_ENROLL_BACK_HEIGHT  40
#define UI_ENROLL_BACK_HIT_PAD 12
#define UI_ENROLL_SAVE_X       128
#define UI_ENROLL_SAVE_Y       264
#define UI_ENROLL_SAVE_WIDTH   92
#define UI_ENROLL_SAVE_HEIGHT  40
#define UI_TOUCH_SDA           IOPORT_PORT_05_PIN_11
#define UI_TOUCH_SCL           IOPORT_PORT_05_PIN_12
#define UI_TOUCH_IRQ           IOPORT_PORT_00_PIN_04
#define UI_TOUCH_RESET         IOPORT_PORT_06_PIN_09
#define UI_TOUCH_I2C_DELAY_US  5U
#define UI_LOGO_CROP_X         0
#define UI_LOGO_CROP_Y         0
#define UI_LOGO_CROP_WIDTH     RAMO_LOGO_WIDTH
#define UI_LOGO_CROP_HEIGHT    RAMO_LOGO_HEIGHT

#define SX8654_I2C_ADDR        0x48U
#define SX8654_REG_TOUCH0      0x00U
#define SX8654_REG_TOUCH1      0x01U
#define SX8654_REG_TOUCH2      0x02U
#define SX8654_REG_CHANMSK     0x04U
#define SX8654_REG_PROX0       0x0BU
#define SX8654_REG_IRQMSK      0x22U
#define SX8654_REG_IRQSRC      0x23U
#define SX8654_CMD_PENTRG      0xE0U
#define SX8654_CMD_READ_REG    0x40U
#define SX8654_IRQ_PENTOUCH    (0x01U << 3)
#define SX8654_IRQ_PENRELEASE  (0x01U << 2)
#define SX8654_CHAN_XY         ((0x01U << 7) | (0x01U << 6))
#define SX8654_IRQ_ENABLE_MASK ((0x01U << 3) | (0x01U << 2))
#define SX8654_MAX_ADC         4095L

typedef enum e_ui_view
{
    UI_VIEW_IDLE = 0,
    UI_VIEW_RESULT,
    UI_VIEW_PIN,
    UI_VIEW_ENROLL_WAIT,
} ui_view_t;

typedef struct st_ui_snapshot
{
    bool door_open;
    bool light_on;
    bool net_ready;
    char last_uid[UID_MAX_LEN];
    char last_user[NAME_MAX_LEN];
} ui_snapshot_t;

typedef struct st_ui_status
{
    char line1[NAME_MAX_LEN];
    char line2[NAME_MAX_LEN];
    char context[NAME_MAX_LEN];
    uint16_t accent;
    ULONG deadline;
    bool sticky;
    ui_view_t view;
    char pin_buffer[UI_PIN_LENGTH + 1U];
    uint8_t pin_index;
    uint8_t pin_count;
} ui_status_t;

typedef struct st_ui_touch_event
{
    bool valid;
    bool pressed;
    bool just_pressed;
    bool just_released;
    int16_t x;
    int16_t y;
} ui_touch_event_t;

static const uint16_t UI_COLOR_BG         = (uint16_t)0x10A3U;
static const uint16_t UI_COLOR_PANEL_ALT  = (uint16_t)0x1947U;
static const uint16_t UI_COLOR_TEXT       = (uint16_t)0xFFFFU;
static const uint16_t UI_COLOR_OK         = (uint16_t)0x56EAU;
static const uint16_t UI_COLOR_WARN       = (uint16_t)0xFD20U;
static const uint16_t UI_COLOR_ERROR      = (uint16_t)0xF8C3U;
static const uint16_t UI_COLOR_IDLE       = (uint16_t)0x7E19U;
static const uint16_t UI_COLOR_BRAND      = (uint16_t)0x6E4AU;
static const uint16_t UI_COLOR_PORTRAIT   = (uint16_t)0x31C9U;
static const uint16_t UI_COLOR_PIN_BG     = (uint16_t)0x64D0U;
static const uint16_t UI_COLOR_PIN_CARD   = (uint16_t)0xFFFFU;
static const uint16_t UI_COLOR_PIN_BORDER = (uint16_t)0xD69AU;
static const uint16_t UI_COLOR_PIN_TEXT   = (uint16_t)0x738EU;
static const uint16_t UI_COLOR_WAIT_BG    = (uint16_t)0x0000U;
static const uint16_t UI_COLOR_WAIT_BAR   = (uint16_t)0x0865U;
static const uint16_t UI_COLOR_WAIT_LINE  = (uint16_t)0x39CFU;
static const uint16_t UI_COLOR_WAIT_TEXT2 = (uint16_t)0xA514U;

static bool g_ui_display_ready = false;
static bool g_ui_backlight_on = false;
static bool g_ui_touch_ready = false;
static bool g_ui_touch_pressed = false;
static int16_t g_ui_touch_last_x = 0;
static int16_t g_ui_touch_last_y = 0;
static ULONG g_ui_touch_last_action_tick = 0U;
volatile uint32_t g_ui_touch_debug_ready = 0U;
volatile uint32_t g_ui_touch_debug_fail_count = 0U;
volatile uint32_t g_ui_touch_debug_event_count = 0U;
volatile uint32_t g_ui_touch_debug_last_irqsrc = 0U;
volatile uint32_t g_ui_touch_debug_last_raw_x = 0U;
volatile uint32_t g_ui_touch_debug_last_raw_y = 0U;
volatile uint32_t g_ui_touch_debug_irq_level = 0U;
volatile uint32_t g_ui_touch_debug_reg_touch0 = 0U;
volatile uint32_t g_ui_touch_debug_reg_chanmsk = 0U;
volatile uint32_t g_ui_touch_debug_reg_irqmsk = 0U;
volatile uint32_t g_ui_touch_debug_reg_irqsrc = 0U;
volatile uint32_t g_ui_touch_debug_rearm_count = 0U;
volatile uint32_t g_ui_debug_loop_count = 0U;
volatile uint32_t g_ui_debug_last_view = 0U;
volatile uint32_t g_ui_debug_last_tick = 0U;
volatile uint32_t g_ui_debug_status_deadline = 0U;
volatile uint32_t g_ui_debug_expire_count = 0U;
static ULONG g_ui_touch_next_poll_tick = 0U;

static void ui_set_status(ui_status_t *status,
                          const char *line1,
                          const char *line2,
                          const char *context,
                          uint16_t accent,
                          ULONG hold_ticks,
                          bool sticky);
static bool ui_touch_poll(ui_touch_event_t *event);
static void ui_enter_idle(ui_status_t *status);
static bool ui_point_in_rect(int32_t x, int32_t y, int32_t rx, int32_t ry, int32_t rw, int32_t rh);
static bool ui_pin_hit_test(int32_t touch_x, int32_t touch_y, int32_t row, int32_t col);
static bool ui_pin_enter_hit_test(int32_t touch_x, int32_t touch_y);
static bool ui_enroll_back_hit_test(int32_t touch_x, int32_t touch_y);
static bool ui_enroll_save_hit_test(int32_t touch_x, int32_t touch_y);
static bool ui_touch_accept_action(void);
static void ui_draw_wait_screen(const char *line1, const char *line2, bool show_gear);
static void ui_draw_pixel_text_centered(int32_t y, const char *text, uint16_t color, uint32_t scale);
static void ui_draw_text_crisp(int32_t x, int32_t y, const char *text, uint16_t color, uint32_t scale);
static void ui_draw_text_crisp_centered(int32_t y, const char *text, uint16_t color, uint32_t scale);
static void ui_draw_rgb565_image_scaled_cropped(int32_t x,
                                                int32_t y,
                                                int32_t dst_width,
                                                int32_t dst_height,
                                                const uint16_t *pixels,
                                                int32_t src_width,
                                                int32_t src_height,
                                                int32_t crop_x,
                                                int32_t crop_y,
                                                int32_t crop_width,
                                                int32_t crop_height);

static void ui_flush_pending_events(void)
{
    (void) tx_queue_flush(&g_event_queue);
}

static void ui_copy_text(char *dest, size_t dest_size, const char *src)
{
    size_t copy_len;

    if ((NULL == dest) || (0U == dest_size))
    {
        return;
    }

    if (NULL == src)
    {
        dest[0] = '\0';
        return;
    }

    copy_len = strlen(src);
    if (copy_len >= dest_size)
    {
        copy_len = dest_size - 1U;
    }

    memcpy(dest, src, copy_len);
    dest[copy_len] = '\0';
}

static uint16_t * ui_framebuffer(void)
{
    return (uint16_t *) &g_display0_fb_background[0][0];
}

static void ui_fill_rect(int32_t x, int32_t y, int32_t width, int32_t height, uint16_t color)
{
    int32_t x0;
    int32_t y0;
    int32_t x1;
    int32_t y1;
    uint16_t *framebuffer = ui_framebuffer();

    if ((width <= 0) || (height <= 0))
    {
        return;
    }

    x0 = (x < 0) ? 0 : x;
    y0 = (y < 0) ? 0 : y;
    x1 = ((x + width) > UI_SCREEN_WIDTH) ? UI_SCREEN_WIDTH : (x + width);
    y1 = ((y + height) > UI_SCREEN_HEIGHT) ? UI_SCREEN_HEIGHT : (y + height);

    if ((x0 >= x1) || (y0 >= y1))
    {
        return;
    }

    for (int32_t row = y0; row < y1; row++)
    {
        uint16_t *line = &framebuffer[(row * UI_FRAMEBUFFER_STRIDE) + x0];
        for (int32_t col = x0; col < x1; col++)
        {
            line[col - x0] = color;
        }
    }
}

static void ui_clear_screen(uint16_t color)
{
    ui_fill_rect(0, 0, UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, color);
}

static void ui_draw_gear_button(int32_t x, int32_t y)
{
    const uint16_t frame = (uint16_t) 0xD69AU;
    const uint16_t gear = (uint16_t) 0x8410U;
    const uint16_t hole = UI_COLOR_TEXT;
    const int32_t size = UI_GEAR_BUTTON_SIZE;
    const int32_t ring_x = x + 8;
    const int32_t ring_y = y + 8;

    ui_fill_rect(x, y, size, size, frame);
    ui_fill_rect(ring_x, y, 12, UI_GEAR_TOOTH_SIZE, gear);
    ui_fill_rect(ring_x, y + size - UI_GEAR_TOOTH_SIZE, 12, UI_GEAR_TOOTH_SIZE, gear);
    ui_fill_rect(x, ring_y, UI_GEAR_TOOTH_SIZE, 12, gear);
    ui_fill_rect(x + size - UI_GEAR_TOOTH_SIZE, ring_y, UI_GEAR_TOOTH_SIZE, 12, gear);
    ui_fill_rect(x + 3, y + 3, UI_GEAR_TOOTH_SIZE, UI_GEAR_TOOTH_SIZE, gear);
    ui_fill_rect(x + size - 7, y + 3, UI_GEAR_TOOTH_SIZE, UI_GEAR_TOOTH_SIZE, gear);
    ui_fill_rect(x + 3, y + size - 7, UI_GEAR_TOOTH_SIZE, UI_GEAR_TOOTH_SIZE, gear);
    ui_fill_rect(x + size - 7, y + size - 7, UI_GEAR_TOOTH_SIZE, UI_GEAR_TOOTH_SIZE, gear);
    ui_fill_rect(x + 7, y + 7, 14, 14, gear);
    ui_fill_rect(x + 10, y + 10, UI_GEAR_CENTER_SIZE, UI_GEAR_CENTER_SIZE, hole);
}

static void ui_draw_rgb565_image_scaled(int32_t x,
                                        int32_t y,
                                        int32_t dst_width,
                                        int32_t dst_height,
                                        const uint16_t *pixels,
                                        int32_t src_width,
                                        int32_t src_height)
{
    uint16_t *framebuffer = ui_framebuffer();

    if ((NULL == pixels) || (dst_width <= 0) || (dst_height <= 0) || (src_width <= 0) || (src_height <= 0))
    {
        return;
    }

    for (int32_t row = 0; row < dst_height; row++)
    {
        int32_t dst_y = y + row;
        int32_t src_y = (row * src_height) / dst_height;

        if ((dst_y < 0) || (dst_y >= UI_SCREEN_HEIGHT))
        {
            continue;
        }

        for (int32_t col = 0; col < dst_width; col++)
        {
            int32_t dst_x = x + col;
            int32_t src_x = (col * src_width) / dst_width;

            if ((dst_x < 0) || (dst_x >= UI_SCREEN_WIDTH))
            {
                continue;
            }

            framebuffer[(dst_y * UI_FRAMEBUFFER_STRIDE) + dst_x] = pixels[(src_y * src_width) + src_x];
        }
    }
}

static void ui_draw_rgb565_image_scaled_cropped(int32_t x,
                                                int32_t y,
                                                int32_t dst_width,
                                                int32_t dst_height,
                                                const uint16_t *pixels,
                                                int32_t src_width,
                                                int32_t src_height,
                                                int32_t crop_x,
                                                int32_t crop_y,
                                                int32_t crop_width,
                                                int32_t crop_height)
{
    uint16_t *framebuffer = ui_framebuffer();

    if ((NULL == pixels) || (dst_width <= 0) || (dst_height <= 0) ||
        (src_width <= 0) || (src_height <= 0) ||
        (crop_width <= 0) || (crop_height <= 0))
    {
        return;
    }

    if (crop_x < 0)
    {
        crop_x = 0;
    }
    if (crop_y < 0)
    {
        crop_y = 0;
    }
    if ((crop_x + crop_width) > src_width)
    {
        crop_width = src_width - crop_x;
    }
    if ((crop_y + crop_height) > src_height)
    {
        crop_height = src_height - crop_y;
    }

    for (int32_t row = 0; row < dst_height; row++)
    {
        int32_t dst_y = y + row;
        int32_t src_y = crop_y + ((row * crop_height) / dst_height);

        if ((dst_y < 0) || (dst_y >= UI_SCREEN_HEIGHT))
        {
            continue;
        }

        for (int32_t col = 0; col < dst_width; col++)
        {
            int32_t dst_x = x + col;
            int32_t src_x = crop_x + ((col * crop_width) / dst_width);

            if ((dst_x < 0) || (dst_x >= UI_SCREEN_WIDTH))
            {
                continue;
            }

            framebuffer[(dst_y * UI_FRAMEBUFFER_STRIDE) + dst_x] = pixels[(src_y * src_width) + src_x];
        }
    }
}

static void ui_set_backlight(bool enabled)
{
    g_ioport.p_api->pinWrite(LCD_BACKLIGHT, enabled ? IOPORT_LEVEL_HIGH : IOPORT_LEVEL_LOW);
    g_ui_backlight_on = enabled;
}

static void ui_touch_delay(void)
{
    R_BSP_SoftwareDelay(UI_TOUCH_I2C_DELAY_US, BSP_DELAY_UNITS_MICROSECONDS);
}

static void ui_touch_set_sda(bool high)
{
    g_ioport.p_api->pinWrite(UI_TOUCH_SDA, high ? IOPORT_LEVEL_HIGH : IOPORT_LEVEL_LOW);
}

static void ui_touch_set_scl(bool high)
{
    g_ioport.p_api->pinWrite(UI_TOUCH_SCL, high ? IOPORT_LEVEL_HIGH : IOPORT_LEVEL_LOW);
}

static bool ui_touch_read_sda(void)
{
    ioport_level_t level = IOPORT_LEVEL_LOW;
    (void) g_ioport.p_api->pinRead(UI_TOUCH_SDA, &level);
    return (IOPORT_LEVEL_HIGH == level);
}

static void ui_touch_i2c_start(void)
{
    ui_touch_set_sda(true);
    ui_touch_set_scl(true);
    ui_touch_delay();
    ui_touch_set_sda(false);
    ui_touch_delay();
    ui_touch_set_scl(false);
    ui_touch_delay();
}

static void ui_touch_i2c_stop(void)
{
    ui_touch_set_sda(false);
    ui_touch_delay();
    ui_touch_set_scl(true);
    ui_touch_delay();
    ui_touch_set_sda(true);
    ui_touch_delay();
}

static bool ui_touch_i2c_write_byte(uint8_t value)
{
    for (uint32_t bit = 0U; bit < 8U; bit++)
    {
        ui_touch_set_sda(0U != (value & 0x80U));
        ui_touch_delay();
        ui_touch_set_scl(true);
        ui_touch_delay();
        ui_touch_set_scl(false);
        ui_touch_delay();
        value <<= 1;
    }

    ui_touch_set_sda(true);
    ui_touch_delay();
    ui_touch_set_scl(true);
    ui_touch_delay();

    {
        bool ack = !ui_touch_read_sda();
        ui_touch_set_scl(false);
        ui_touch_delay();
        return ack;
    }
}

static uint8_t ui_touch_i2c_read_byte(bool ack)
{
    uint8_t value = 0U;

    ui_touch_set_sda(true);

    for (uint32_t bit = 0U; bit < 8U; bit++)
    {
        value <<= 1;
        ui_touch_set_scl(true);
        ui_touch_delay();
        if (ui_touch_read_sda())
        {
            value |= 0x01U;
        }
        ui_touch_set_scl(false);
        ui_touch_delay();
    }

    ui_touch_set_sda(!ack);
    ui_touch_delay();
    ui_touch_set_scl(true);
    ui_touch_delay();
    ui_touch_set_scl(false);
    ui_touch_set_sda(true);
    ui_touch_delay();

    return value;
}

static bool ui_touch_i2c_write(uint8_t addr, const uint8_t *data, size_t length)
{
    if (NULL == data)
    {
        return false;
    }

    ui_touch_i2c_start();
    if (!ui_touch_i2c_write_byte((uint8_t) (addr << 1)))
    {
        ui_touch_i2c_stop();
        return false;
    }

    for (size_t i = 0U; i < length; i++)
    {
        if (!ui_touch_i2c_write_byte(data[i]))
        {
            ui_touch_i2c_stop();
            return false;
        }
    }

    ui_touch_i2c_stop();
    return true;
}

static bool ui_touch_i2c_write_then_read(uint8_t addr, uint8_t *write_data, size_t write_length, uint8_t *read_data, size_t read_length)
{
    if ((NULL == write_data) || (NULL == read_data))
    {
        return false;
    }

    ui_touch_i2c_start();
    if (!ui_touch_i2c_write_byte((uint8_t) (addr << 1)))
    {
        ui_touch_i2c_stop();
        return false;
    }

    for (size_t i = 0U; i < write_length; i++)
    {
        if (!ui_touch_i2c_write_byte(write_data[i]))
        {
            ui_touch_i2c_stop();
            return false;
        }
    }

    ui_touch_i2c_start();
    if (!ui_touch_i2c_write_byte((uint8_t) ((addr << 1) | 0x01U)))
    {
        ui_touch_i2c_stop();
        return false;
    }

    for (size_t i = 0U; i < read_length; i++)
    {
        read_data[i] = ui_touch_i2c_read_byte((i + 1U) < read_length);
    }

    ui_touch_i2c_stop();
    return true;
}

static bool ui_touch_i2c_read_stream(uint8_t addr, uint8_t *data, size_t length)
{
    if (NULL == data)
    {
        return false;
    }

    ui_touch_i2c_start();
    if (!ui_touch_i2c_write_byte((uint8_t) ((addr << 1) | 0x01U)))
    {
        ui_touch_i2c_stop();
        return false;
    }

    for (size_t i = 0U; i < length; i++)
    {
        data[i] = ui_touch_i2c_read_byte((i + 1U) < length);
    }

    ui_touch_i2c_stop();
    return true;
}

static bool ui_touch_read_register(uint8_t reg, uint8_t *value)
{
    uint8_t command = (uint8_t) (SX8654_CMD_READ_REG | reg);

    if (NULL == value)
    {
        return false;
    }

    return ui_touch_i2c_write_then_read(SX8654_I2C_ADDR, &command, 1U, value, 1U);
}

static void ui_touch_rearm_pen_trigger(void)
{
    uint8_t command = SX8654_CMD_PENTRG;

    if (ui_touch_i2c_write(SX8654_I2C_ADDR, &command, 1U))
    {
        g_ui_touch_debug_rearm_count++;
    }
}

static bool ui_touch_configure_controller(void)
{
    uint8_t command[4];

    command[0] = SX8654_REG_TOUCH0;
    command[1] = (uint8_t) ((0x07U << 4) | 0x04U);
    command[2] = (uint8_t) ((0x01U << 5) | (0x01U << 2) | 0x03U);
    command[3] = 0x04U;
    if (!ui_touch_i2c_write(SX8654_I2C_ADDR, command, 4U))
    {
        return false;
    }

    command[0] = SX8654_REG_CHANMSK;
    command[1] = SX8654_CHAN_XY;
    if (!ui_touch_i2c_write(SX8654_I2C_ADDR, command, 2U))
    {
        return false;
    }

    command[0] = SX8654_REG_IRQMSK;
    command[1] = SX8654_IRQ_ENABLE_MASK;
    if (!ui_touch_i2c_write(SX8654_I2C_ADDR, command, 2U))
    {
        return false;
    }

    command[0] = SX8654_REG_PROX0;
    command[1] = 0x00U;
    if (!ui_touch_i2c_write(SX8654_I2C_ADDR, command, 2U))
    {
        return false;
    }

    command[0] = SX8654_CMD_PENTRG;
    return ui_touch_i2c_write(SX8654_I2C_ADDR, command, 1U);
}

static void ui_touch_init(void)
{
    uint8_t reg_value = 0U;

    if (g_ui_touch_ready)
    {
        return;
    }

    (void) g_ioport.p_api->pinCfg(UI_TOUCH_IRQ, IOPORT_CFG_IRQ_ENABLE | IOPORT_CFG_PORT_DIRECTION_INPUT);
    (void) g_ioport.p_api->pinCfg(UI_TOUCH_RESET,
                                  IOPORT_CFG_PORT_DIRECTION_OUTPUT |
                                  IOPORT_CFG_PORT_OUTPUT_LOW);
    (void) g_ioport.p_api->pinCfg(UI_TOUCH_SDA,
                                  IOPORT_CFG_PORT_DIRECTION_OUTPUT |
                                  IOPORT_CFG_PORT_OUTPUT_HIGH |
                                  IOPORT_CFG_NMOS_ENABLE |
                                  IOPORT_CFG_DRIVE_MID_IIC);
    (void) g_ioport.p_api->pinCfg(UI_TOUCH_SCL,
                                  IOPORT_CFG_PORT_DIRECTION_OUTPUT |
                                  IOPORT_CFG_PORT_OUTPUT_HIGH |
                                  IOPORT_CFG_NMOS_ENABLE |
                                  IOPORT_CFG_DRIVE_MID_IIC);

    ui_touch_set_sda(true);
    ui_touch_set_scl(true);

    g_ioport.p_api->pinWrite(UI_TOUCH_RESET, IOPORT_LEVEL_LOW);
    tx_thread_sleep(2);
    g_ioport.p_api->pinWrite(UI_TOUCH_RESET, IOPORT_LEVEL_HIGH);
    tx_thread_sleep(2);

    g_ui_touch_ready = ui_touch_configure_controller();
    g_ui_touch_debug_ready = g_ui_touch_ready ? 1U : 0U;
    g_ui_touch_debug_fail_count = 0U;
    g_ui_touch_next_poll_tick = 0U;

    if (g_ui_touch_ready)
    {
        if (ui_touch_read_register(SX8654_REG_TOUCH0, &reg_value))
        {
            g_ui_touch_debug_reg_touch0 = reg_value;
        }

        if (ui_touch_read_register(SX8654_REG_CHANMSK, &reg_value))
        {
            g_ui_touch_debug_reg_chanmsk = reg_value;
        }

        if (ui_touch_read_register(SX8654_REG_IRQMSK, &reg_value))
        {
            g_ui_touch_debug_reg_irqmsk = reg_value;
        }

        if (ui_touch_read_register(SX8654_REG_IRQSRC, &reg_value))
        {
            g_ui_touch_debug_reg_irqsrc = reg_value;
        }
    }
}

static bool ui_touch_poll(ui_touch_event_t *event)
{
    uint8_t irqsrc_command = (uint8_t) (SX8654_CMD_READ_REG | SX8654_REG_IRQSRC);
    uint8_t irqsrc = 0U;
    uint8_t raw[4];
    int32_t x_raw;
    int32_t y_raw;
    int32_t scaled_x;
    int32_t scaled_y;
    bool released;
    bool touch_detected;
    ioport_level_t irq_level = IOPORT_LEVEL_HIGH;

    if (NULL == event)
    {
        return false;
    }

    memset(event, 0, sizeof(*event));

    if (!g_ui_touch_ready)
    {
        return false;
    }

    (void) g_ioport.p_api->pinRead(UI_TOUCH_IRQ, &irq_level);
    g_ui_touch_debug_irq_level = (IOPORT_LEVEL_LOW == irq_level) ? 0U : 1U;

    if ((0U != g_ui_touch_next_poll_tick) && ((LONG) (tx_time_get() - g_ui_touch_next_poll_tick) < 0))
    {
        return false;
    }
    g_ui_touch_next_poll_tick = tx_time_get() + 1U;

    if (!ui_touch_i2c_write_then_read(SX8654_I2C_ADDR, &irqsrc_command, 1U, &irqsrc, 1U))
    {
        g_ui_touch_debug_fail_count++;
        return false;
    }
    g_ui_touch_debug_last_irqsrc = irqsrc;
    g_ui_touch_debug_reg_irqsrc = irqsrc;
    g_ui_touch_debug_fail_count = 0U;

    if (!ui_touch_i2c_read_stream(SX8654_I2C_ADDR, raw, sizeof(raw)))
    {
        g_ui_touch_debug_fail_count++;
        return false;
    }

    x_raw = (int32_t) (((raw[0] & 0x0FU) << 8) | raw[1]);
    y_raw = (int32_t) (((raw[2] & 0x0FU) << 8) | raw[3]);
    g_ui_touch_debug_last_raw_x = (uint32_t) x_raw;
    g_ui_touch_debug_last_raw_y = (uint32_t) y_raw;

    released = (0U != (irqsrc & SX8654_IRQ_PENRELEASE));
    touch_detected = (0U != (irqsrc & SX8654_IRQ_PENTOUCH))
                     || ((((uint32_t) x_raw | (uint32_t) y_raw) != 0U)
                         && ((uint32_t) x_raw < (uint32_t) SX8654_MAX_ADC)
                         && ((uint32_t) y_raw < (uint32_t) SX8654_MAX_ADC));

    if (!touch_detected && !released)
    {
        ui_touch_rearm_pen_trigger();
        return false;
    }

    if (released || !touch_detected)
    {
        scaled_x = g_ui_touch_last_x;
        scaled_y = g_ui_touch_last_y;
    }
    else
    {
        scaled_x = (x_raw * UI_SCREEN_WIDTH) / SX8654_MAX_ADC;
        scaled_y = (y_raw * UI_SCREEN_HEIGHT) / SX8654_MAX_ADC;

        if (scaled_x < 0)
        {
            scaled_x = 0;
        }
        else if (scaled_x >= UI_SCREEN_WIDTH)
        {
            scaled_x = UI_SCREEN_WIDTH - 1;
        }

        scaled_y = UI_SCREEN_HEIGHT - scaled_y;
        if (scaled_y < 0)
        {
            scaled_y = 0;
        }
        else if (scaled_y >= UI_SCREEN_HEIGHT)
        {
            scaled_y = UI_SCREEN_HEIGHT - 1;
        }
    }

    event->valid = true;
    event->pressed = touch_detected && !released;
    g_ui_touch_debug_event_count++;
    ui_touch_rearm_pen_trigger();

    if (!event->pressed)
    {
        event->x = g_ui_touch_last_x;
        event->y = g_ui_touch_last_y;
        event->just_released = g_ui_touch_pressed;
        g_ui_touch_pressed = false;
    }
    else
    {
        event->x = (int16_t) scaled_x;
        event->y = (int16_t) scaled_y;
        event->just_pressed = !g_ui_touch_pressed;
        g_ui_touch_pressed = true;
        g_ui_touch_last_x = event->x;
        g_ui_touch_last_y = event->y;
    }

    return true;
}

static uint32_t ui_decode_codepoint(const char **text)
{
    const uint8_t *ptr;

    if ((NULL == text) || (NULL == *text))
    {
        return 0U;
    }

    ptr = (const uint8_t *) *text;

    if (0U == ptr[0])
    {
        return 0U;
    }

    if ((ptr[0] & 0x80U) == 0U)
    {
        (*text)++;
        return ptr[0];
    }

    if (((ptr[0] & 0xE0U) == 0xC0U) && ((ptr[1] & 0xC0U) == 0x80U))
    {
        uint32_t codepoint = (uint32_t) (((ptr[0] & 0x1FU) << 6) | (ptr[1] & 0x3FU));
        *text += 2;
        return codepoint;
    }

    (*text)++;
    return (uint32_t) '?';
}

static uint32_t ui_font_index_from_codepoint(uint32_t codepoint)
{
    for (uint32_t i = 0U; i < UI_FONT_GLYPH_COUNT; i++)
    {
        if (g_ui_font_codepoints[i] == codepoint)
        {
            return i;
        }
    }

    for (uint32_t i = 0U; i < UI_FONT_GLYPH_COUNT; i++)
    {
        if (g_ui_font_codepoints[i] == (uint32_t) '?')
        {
            return i;
        }
    }

    return 0U;
}

static uint16_t ui_blend_rgb565(uint16_t background, uint16_t foreground, uint8_t alpha)
{
    uint32_t bg_r = (background >> 11) & 0x1FU;
    uint32_t bg_g = (background >> 5) & 0x3FU;
    uint32_t bg_b = background & 0x1FU;
    uint32_t fg_r = (foreground >> 11) & 0x1FU;
    uint32_t fg_g = (foreground >> 5) & 0x3FU;
    uint32_t fg_b = foreground & 0x1FU;

    uint32_t out_r = ((fg_r * alpha) + (bg_r * (255U - alpha))) / 255U;
    uint32_t out_g = ((fg_g * alpha) + (bg_g * (255U - alpha))) / 255U;
    uint32_t out_b = ((fg_b * alpha) + (bg_b * (255U - alpha))) / 255U;

    return (uint16_t) ((out_r << 11) | (out_g << 5) | out_b);
}

static int32_t ui_text_width(const char *text, uint32_t scale)
{
    int32_t width = 0;
    const char *cursor = text;

    if ((NULL == text) || ('\0' == text[0]))
    {
        return 0;
    }

    while ('\0' != *cursor)
    {
        uint32_t glyph_index = ui_font_index_from_codepoint(ui_decode_codepoint(&cursor));

        width += ((int32_t) g_ui_font_advance[glyph_index] * (int32_t) scale) + ((int32_t) UI_FONT_SPACING * (int32_t) scale);
    }

    return width - ((int32_t) UI_FONT_SPACING * (int32_t) scale);
}

static void ui_draw_char(int32_t x, int32_t y, char c, uint16_t color, uint32_t scale)
{
    uint16_t *framebuffer = ui_framebuffer();
    uint32_t glyph_index = ui_font_index_from_codepoint((uint8_t) c);

    for (uint32_t row = 0U; row < UI_FONT_CELL_HEIGHT; row++)
    {
        for (uint32_t col = 0U; col < UI_FONT_CELL_WIDTH; col++)
        {
            uint8_t alpha = g_ui_font_alpha[glyph_index][row][col];

            if (0U == alpha)
            {
                continue;
            }

            for (uint32_t sy = 0U; sy < scale; sy++)
            {
                int32_t dst_y = y + ((int32_t) row * (int32_t) scale) + (int32_t) sy;

                if ((dst_y < 0) || (dst_y >= UI_SCREEN_HEIGHT))
                {
                    continue;
                }

                for (uint32_t sx = 0U; sx < scale; sx++)
                {
                    int32_t dst_x = x + ((int32_t) col * (int32_t) scale) + (int32_t) sx;

                    if ((dst_x < 0) || (dst_x >= UI_SCREEN_WIDTH))
                    {
                        continue;
                    }

                    framebuffer[(dst_y * UI_FRAMEBUFFER_STRIDE) + dst_x] =
                        ui_blend_rgb565(framebuffer[(dst_y * UI_FRAMEBUFFER_STRIDE) + dst_x], color, alpha);
                }
            }
        }
    }
}

static void ui_draw_text(int32_t x, int32_t y, const char *text, uint16_t color, uint32_t scale)
{
    int32_t cursor_x = x;
    const char *cursor = text;

    if (NULL == text)
    {
        return;
    }

    while ('\0' != *cursor)
    {
        uint32_t codepoint = ui_decode_codepoint(&cursor);
        uint32_t glyph_index = ui_font_index_from_codepoint(codepoint);
        ui_draw_char(cursor_x, y, (char) codepoint, color, scale);
        cursor_x += ((int32_t) g_ui_font_advance[glyph_index] * (int32_t) scale) + ((int32_t) UI_FONT_SPACING * (int32_t) scale);
    }
}

static void ui_draw_text_centered(int32_t y, const char *text, uint16_t color, uint32_t scale)
{
    int32_t width = ui_text_width(text, scale);
    int32_t x = (UI_SCREEN_WIDTH - width) / 2;
    ui_draw_text(x, y, text, color, scale);
}

static void ui_draw_text_centered_in_rect(int32_t x, int32_t width, int32_t y, const char *text, uint16_t color, uint32_t scale)
{
    int32_t text_width = ui_text_width(text, scale);
    int32_t text_x = x + ((width - text_width) / 2);
    ui_draw_text(text_x, y, text, color, scale);
}

static const uint8_t * ui_pixel_glyph(char c)
{
    static const uint8_t space[7] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    static const uint8_t a[7] = { 0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11 };
    static const uint8_t c_glyph[7] = { 0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E };
    static const uint8_t d[7] = { 0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E };
    static const uint8_t e_glyph[7] = { 0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F };
    static const uint8_t g[7] = { 0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0F };
    static const uint8_t i[7] = { 0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x1F };
    static const uint8_t l[7] = { 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F };
    static const uint8_t m[7] = { 0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11 };
    static const uint8_t n[7] = { 0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11 };
    static const uint8_t o[7] = { 0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E };
    static const uint8_t p[7] = { 0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10 };
    static const uint8_t r[7] = { 0x1E, 0x11, 0x11, 0x1E, 0x12, 0x11, 0x11 };
    static const uint8_t s[7] = { 0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E };
    static const uint8_t t[7] = { 0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04 };
    static const uint8_t u[7] = { 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E };
    static const uint8_t x_glyph[7] = { 0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11 };

    switch (c)
    {
        case 'A': return a;
        case 'C': return c_glyph;
        case 'D': return d;
        case 'E': return e_glyph;
        case 'G': return g;
        case 'I': return i;
        case 'L': return l;
        case 'M': return m;
        case 'N': return n;
        case 'O': return o;
        case 'P': return p;
        case 'R': return r;
        case 'S': return s;
        case 'T': return t;
        case 'U': return u;
        case 'X': return x_glyph;
        case ' ': return space;
        default: return space;
    }
}

static void ui_draw_pixel_text(int32_t x, int32_t y, const char *text, uint16_t color, uint32_t scale)
{
    uint16_t *framebuffer = ui_framebuffer();
    int32_t cursor_x = x;

    if (NULL == text)
    {
        return;
    }

    while ('\0' != *text)
    {
        const uint8_t *glyph = ui_pixel_glyph(*text);

        for (int32_t row = 0; row < 7; row++)
        {
            for (int32_t col = 0; col < 5; col++)
            {
                if (0U == (glyph[row] & (uint8_t) (1U << (4 - col))))
                {
                    continue;
                }

                for (uint32_t sy = 0U; sy < scale; sy++)
                {
                    int32_t dst_y = y + (row * (int32_t) scale) + (int32_t) sy;

                    if ((dst_y < 0) || (dst_y >= UI_SCREEN_HEIGHT))
                    {
                        continue;
                    }

                    for (uint32_t sx = 0U; sx < scale; sx++)
                    {
                        int32_t dst_x = cursor_x + (col * (int32_t) scale) + (int32_t) sx;

                        if ((dst_x < 0) || (dst_x >= UI_SCREEN_WIDTH))
                        {
                            continue;
                        }

                        framebuffer[(dst_y * UI_FRAMEBUFFER_STRIDE) + dst_x] = color;
                    }
                }
            }
        }

        cursor_x += (int32_t) ((5U * scale) + scale);
        text++;
    }
}

static void ui_draw_pixel_text_centered(int32_t y, const char *text, uint16_t color, uint32_t scale)
{
    size_t len;
    int32_t width;
    int32_t x;

    if (NULL == text)
    {
        return;
    }

    len = strlen(text);
    width = (int32_t) (len * ((5U * scale) + scale));
    if (width > 0)
    {
        width -= (int32_t) scale;
    }

    x = (UI_SCREEN_WIDTH - width) / 2;
    ui_draw_pixel_text(x, y, text, color, scale);
}

static void ui_draw_text_crisp(int32_t x, int32_t y, const char *text, uint16_t color, uint32_t scale)
{
    int32_t cursor_x = x;
    const char *cursor = text;
    uint16_t *framebuffer = ui_framebuffer();

    if (NULL == text)
    {
        return;
    }

    while ('\0' != *cursor)
    {
        uint32_t codepoint = ui_decode_codepoint(&cursor);
        uint32_t glyph_index = ui_font_index_from_codepoint(codepoint);

        for (uint32_t row = 0U; row < UI_FONT_CELL_HEIGHT; row++)
        {
            for (uint32_t col = 0U; col < UI_FONT_CELL_WIDTH; col++)
            {
                if (g_ui_font_alpha[glyph_index][row][col] < 96U)
                {
                    continue;
                }

                for (uint32_t sy = 0U; sy < scale; sy++)
                {
                    int32_t dst_y = y + ((int32_t) row * (int32_t) scale) + (int32_t) sy;

                    if ((dst_y < 0) || (dst_y >= UI_SCREEN_HEIGHT))
                    {
                        continue;
                    }

                    for (uint32_t sx = 0U; sx < scale; sx++)
                    {
                        int32_t dst_x = cursor_x + ((int32_t) col * (int32_t) scale) + (int32_t) sx;

                        if ((dst_x < 0) || (dst_x >= UI_SCREEN_WIDTH))
                        {
                            continue;
                        }

                        framebuffer[(dst_y * UI_FRAMEBUFFER_STRIDE) + dst_x] = color;
                    }
                }
            }
        }

        cursor_x += ((int32_t) g_ui_font_advance[glyph_index] * (int32_t) scale) + ((int32_t) UI_FONT_SPACING * (int32_t) scale);
    }
}

static void ui_draw_text_crisp_centered(int32_t y, const char *text, uint16_t color, uint32_t scale)
{
    int32_t width = ui_text_width(text, scale);
    int32_t x = (UI_SCREEN_WIDTH - width) / 2;
    ui_draw_text_crisp(x, y, text, color, scale);
}

static void ui_draw_button(int32_t x, int32_t y, int32_t width, int32_t height, const char *label, uint16_t fill, uint16_t text_color)
{
    ui_fill_rect(x, y, width, height, fill);
    ui_fill_rect(x + 2, y + 2, width - 4, height - 4, UI_COLOR_TEXT);
    ui_draw_text_centered_in_rect(x, width, y + ((height - 14) / 2), label, text_color, 1U);
}

static void ui_reset_pin(ui_status_t *status)
{
    if (NULL == status)
    {
        return;
    }

    memset(status->pin_buffer, '0', UI_PIN_LENGTH);
    status->pin_buffer[UI_PIN_LENGTH] = '\0';
    status->pin_index = 0U;
    status->pin_count = 0U;
}

static void ui_pin_get_key_rect(int32_t row, int32_t col, int32_t *x, int32_t *y, int32_t *width, int32_t *height)
{
    const int32_t total_width = (UI_PIN_KEY_WIDTH * 3) + (UI_PIN_KEY_GAP * 2);
    const int32_t start_x = (UI_SCREEN_WIDTH - total_width) / 2;

    if (NULL != x)
    {
        *x = start_x + (col * (UI_PIN_KEY_WIDTH + UI_PIN_KEY_GAP));
    }

    if (NULL != y)
    {
        *y = UI_PIN_KEY_START_Y + (row * (UI_PIN_KEY_HEIGHT + UI_PIN_KEY_GAP));
    }

    if (NULL != width)
    {
        *width = UI_PIN_KEY_WIDTH;
    }

    if (NULL != height)
    {
        *height = UI_PIN_KEY_HEIGHT;
    }
}

static bool ui_pin_hit_test(int32_t touch_x, int32_t touch_y, int32_t row, int32_t col)
{
    int32_t x;
    int32_t y;

    ui_pin_get_key_rect(row, col, &x, &y, NULL, NULL);
    return ui_point_in_rect(touch_x,
                            touch_y,
                            x - UI_PIN_HIT_PADDING,
                            y - UI_PIN_HIT_PADDING,
                            UI_PIN_KEY_WIDTH + (UI_PIN_HIT_PADDING * 2),
                            UI_PIN_KEY_HEIGHT + (UI_PIN_HIT_PADDING * 2));
}

static bool ui_pin_enter_hit_test(int32_t touch_x, int32_t touch_y)
{
    int32_t x;
    int32_t y;

    ui_pin_get_key_rect(3, 2, &x, &y, NULL, NULL);
    return ui_point_in_rect(touch_x,
                            touch_y,
                            x - 16,
                            y - 16,
                            UI_PIN_KEY_WIDTH + 32,
                            UI_PIN_KEY_HEIGHT + 32);
}

static bool ui_enroll_back_hit_test(int32_t touch_x, int32_t touch_y)
{
    return ui_point_in_rect(touch_x,
                            touch_y,
                            UI_ENROLL_BACK_X - UI_ENROLL_BACK_HIT_PAD,
                            UI_ENROLL_BACK_Y - 12,
                            UI_ENROLL_BACK_WIDTH + (UI_ENROLL_BACK_HIT_PAD * 2),
                            UI_SCREEN_HEIGHT - UI_ENROLL_BACK_Y + 12);
}

static bool ui_enroll_save_hit_test(int32_t touch_x, int32_t touch_y)
{
    return ui_point_in_rect(touch_x,
                            touch_y,
                            UI_ENROLL_SAVE_X - UI_ENROLL_BACK_HIT_PAD,
                            UI_ENROLL_SAVE_Y - 12,
                            UI_ENROLL_SAVE_WIDTH + (UI_ENROLL_BACK_HIT_PAD * 2),
                            UI_SCREEN_HEIGHT - UI_ENROLL_BACK_Y + 12);
}

static bool ui_touch_accept_action(void)
{
    ULONG now = tx_time_get();

    if ((0U != g_ui_touch_last_action_tick)
        && ((now - g_ui_touch_last_action_tick) < UI_TOUCH_DEBOUNCE_TICKS))
    {
        return false;
    }

    g_ui_touch_last_action_tick = now;
    return true;
}

static void ui_enter_idle(ui_status_t *status)
{
    ui_set_status(status,
                  "",
                  "Aproxime o cart" "\303\243" "o",
                  "",
                  UI_COLOR_IDLE,
                  0U,
                  true);
    status->view = UI_VIEW_IDLE;
    g_ui_touch_last_action_tick = 0U;
    g_ui_touch_pressed = false;
    app_set_enrollment_mode(false);
    app_set_ui_mode(APP_UI_MODE_IDLE);
}

static void ui_enter_pin_entry(ui_status_t *status)
{
    if (NULL == status)
    {
        return;
    }

    ui_set_status(status,
                  "Digite o PIN",
                  "Toque nos n" "\303\272" "meros",
                  "Limpar e OK abaixo",
                  UI_COLOR_BRAND,
                  0U,
                  true);
    status->view = UI_VIEW_PIN;
    ui_reset_pin(status);
    g_ui_touch_last_action_tick = 0U;
    g_ui_touch_pressed = false;
    ui_flush_pending_events();
    app_set_enrollment_mode(false);
    app_set_ui_mode(APP_UI_MODE_PIN_ENTRY);
}

static void ui_enter_enroll_wait(ui_status_t *status)
{
    if (NULL == status)
    {
        return;
    }

    ui_set_status(status,
                  "Modo cadastro",
                  "Aproxime o cart" "\303\243" "o",
                  "Toque em voltar para sair",
                  UI_COLOR_BRAND,
                  0U,
                  true);
    status->view = UI_VIEW_ENROLL_WAIT;
    g_ui_touch_last_action_tick = 0U;
    g_ui_touch_pressed = false;
    ui_flush_pending_events();
    app_set_enrollment_mode(true);
    app_set_ui_mode(APP_UI_MODE_ENROLL_WAIT);
}

static void ui_pin_increment(ui_status_t *status)
{
    char digit;

    if ((NULL == status) || (UI_VIEW_PIN != status->view) || (status->pin_index >= UI_PIN_LENGTH))
    {
        return;
    }

    digit = status->pin_buffer[status->pin_index];
    digit = (digit >= '9') ? '0' : (char) (digit + 1);
    status->pin_buffer[status->pin_index] = digit;
}

static void ui_pin_set_digit(ui_status_t *status, char digit)
{
    uint8_t current_index;

    if ((NULL == status) || (UI_VIEW_PIN != status->view) || (status->pin_index >= UI_PIN_LENGTH))
    {
        return;
    }

    current_index = status->pin_index;
    status->pin_buffer[current_index] = digit;
    if (status->pin_count < (uint8_t) (current_index + 1U))
    {
        status->pin_count = (uint8_t) (current_index + 1U);
    }

    if ((status->pin_index + 1U) < UI_PIN_LENGTH)
    {
        status->pin_index++;
    }

    if ((status->pin_count == UI_PIN_LENGTH)
        && (0 == strncmp(status->pin_buffer, UI_ADMIN_PIN, UI_PIN_LENGTH)))
    {
        ui_enter_enroll_wait(status);
    }
}

static void ui_pin_backspace(ui_status_t *status)
{
    if ((NULL == status) || (UI_VIEW_PIN != status->view))
    {
        return;
    }

    if (status->pin_count > 0U)
    {
        status->pin_count--;
        status->pin_index = status->pin_count;
        status->pin_buffer[status->pin_index] = '0';
        return;
    }

    status->pin_index = 0U;
    status->pin_buffer[0] = '0';
}

static void ui_pin_submit(ui_status_t *status)
{
    if ((NULL == status) || (UI_VIEW_PIN != status->view))
    {
        return;
    }

    if ((status->pin_count == UI_PIN_LENGTH)
        && (0 == strncmp(status->pin_buffer, UI_ADMIN_PIN, UI_PIN_LENGTH)))
    {
        ui_enter_enroll_wait(status);
        return;
    }

    ui_set_status(status,
                  "PIN incorreto",
                  "Toque na tela para digitar",
                  "Use limpar para corrigir",
                  UI_COLOR_ERROR,
                  0U,
                  true);
    status->view = UI_VIEW_PIN;
    ui_reset_pin(status);
    app_set_enrollment_mode(false);
    app_set_ui_mode(APP_UI_MODE_PIN_ENTRY);
}

static void ui_pin_commit(ui_status_t *status)
{
    if ((NULL == status) || (UI_VIEW_PIN != status->view))
    {
        return;
    }

    if ((status->pin_index + 1U) < UI_PIN_LENGTH)
    {
        status->pin_index++;
        return;
    }

    ui_pin_submit(status);
}

static void ui_draw_pin_screen(const ui_status_t *status)
{
    char pin_mask[UI_PIN_LENGTH + 1U];
    int32_t key_x;
    int32_t key_y;

    for (uint32_t i = 0U; i < UI_PIN_LENGTH; i++)
    {
        pin_mask[i] = (i < status->pin_count) ? '*' : ' ';
    }
    pin_mask[UI_PIN_LENGTH] = '\0';

    ui_fill_rect(0, 0, UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, UI_COLOR_PIN_BG);
    ui_fill_rect(UI_PIN_CARD_X + 2, UI_PIN_CARD_Y + 2, UI_PIN_CARD_WIDTH, UI_PIN_CARD_HEIGHT, UI_COLOR_PANEL_ALT);
    ui_fill_rect(UI_PIN_CARD_X, UI_PIN_CARD_Y, UI_PIN_CARD_WIDTH, UI_PIN_CARD_HEIGHT, UI_COLOR_PIN_CARD);
    ui_fill_rect(UI_PIN_DISPLAY_X, UI_PIN_DISPLAY_Y, UI_PIN_DISPLAY_WIDTH, UI_PIN_DISPLAY_HEIGHT, UI_COLOR_PIN_BORDER);
    ui_fill_rect(UI_PIN_DISPLAY_X + 2, UI_PIN_DISPLAY_Y + 2, UI_PIN_DISPLAY_WIDTH - 4, UI_PIN_DISPLAY_HEIGHT - 4, UI_COLOR_PIN_CARD);
    ui_draw_text_centered_in_rect(UI_PIN_DISPLAY_X, UI_PIN_DISPLAY_WIDTH, UI_PIN_DISPLAY_Y + 12, pin_mask, UI_COLOR_BG, 2U);

    ui_pin_get_key_rect(0, 0, &key_x, &key_y, NULL, NULL);
    ui_draw_text_centered_in_rect(key_x, UI_PIN_KEY_WIDTH, key_y + 11, "1", UI_COLOR_PIN_TEXT, 2U);
    ui_pin_get_key_rect(0, 1, &key_x, &key_y, NULL, NULL);
    ui_draw_text_centered_in_rect(key_x, UI_PIN_KEY_WIDTH, key_y + 11, "2", UI_COLOR_PIN_TEXT, 2U);
    ui_pin_get_key_rect(0, 2, &key_x, &key_y, NULL, NULL);
    ui_draw_text_centered_in_rect(key_x, UI_PIN_KEY_WIDTH, key_y + 11, "3", UI_COLOR_PIN_TEXT, 2U);
    ui_pin_get_key_rect(1, 0, &key_x, &key_y, NULL, NULL);
    ui_draw_text_centered_in_rect(key_x, UI_PIN_KEY_WIDTH, key_y + 11, "4", UI_COLOR_PIN_TEXT, 2U);
    ui_pin_get_key_rect(1, 1, &key_x, &key_y, NULL, NULL);
    ui_draw_text_centered_in_rect(key_x, UI_PIN_KEY_WIDTH, key_y + 11, "5", UI_COLOR_PIN_TEXT, 2U);
    ui_pin_get_key_rect(1, 2, &key_x, &key_y, NULL, NULL);
    ui_draw_text_centered_in_rect(key_x, UI_PIN_KEY_WIDTH, key_y + 11, "6", UI_COLOR_PIN_TEXT, 2U);
    ui_pin_get_key_rect(2, 0, &key_x, &key_y, NULL, NULL);
    ui_draw_text_centered_in_rect(key_x, UI_PIN_KEY_WIDTH, key_y + 11, "7", UI_COLOR_PIN_TEXT, 2U);
    ui_pin_get_key_rect(2, 1, &key_x, &key_y, NULL, NULL);
    ui_draw_text_centered_in_rect(key_x, UI_PIN_KEY_WIDTH, key_y + 11, "8", UI_COLOR_PIN_TEXT, 2U);
    ui_pin_get_key_rect(2, 2, &key_x, &key_y, NULL, NULL);
    ui_draw_text_centered_in_rect(key_x, UI_PIN_KEY_WIDTH, key_y + 11, "9", UI_COLOR_PIN_TEXT, 2U);
    ui_pin_get_key_rect(3, 0, &key_x, &key_y, NULL, NULL);
    ui_draw_text_centered_in_rect(key_x, UI_PIN_KEY_WIDTH, key_y + 13, "limpa", UI_COLOR_PIN_TEXT, 1U);
    ui_pin_get_key_rect(3, 1, &key_x, &key_y, NULL, NULL);
    ui_draw_text_centered_in_rect(key_x, UI_PIN_KEY_WIDTH, key_y + 11, "0", UI_COLOR_PIN_TEXT, 2U);
    ui_pin_get_key_rect(3, 2, &key_x, &key_y, NULL, NULL);
    ui_draw_text_centered_in_rect(key_x, UI_PIN_KEY_WIDTH, key_y + 13, "entrar", UI_COLOR_PIN_TEXT, 1U);
}

static void ui_draw_enroll_wait_screen(const ui_status_t *status)
{
    ui_fill_rect(0, 0, UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, UI_COLOR_TEXT);
    ui_draw_rgb565_image_scaled((UI_SCREEN_WIDTH - 136) / 2,
                                22,
                                136,
                                136,
                                g_ramo_logo_rgb565,
                                (int32_t) RAMO_LOGO_WIDTH,
                                (int32_t) RAMO_LOGO_HEIGHT);
    ui_fill_rect(14, 166, 212, 76, UI_COLOR_BRAND);
    ui_fill_rect(18, 170, 204, 68, UI_COLOR_TEXT);
    ui_draw_text_centered(184, status->line1, UI_COLOR_BG, 1U);
    ui_draw_text_centered(208, status->line2, UI_COLOR_BRAND, 1U);
    ui_draw_text_centered(232, "Encoste o cartao no leitor", UI_COLOR_BG, 1U);
    ui_draw_button(UI_ENROLL_BACK_X,
                   UI_ENROLL_BACK_Y,
                   UI_ENROLL_BACK_WIDTH,
                   UI_ENROLL_BACK_HEIGHT,
                   "Voltar",
                   UI_COLOR_WARN,
                   UI_COLOR_BG);
    ui_draw_button(UI_ENROLL_SAVE_X,
                   UI_ENROLL_SAVE_Y,
                   UI_ENROLL_SAVE_WIDTH,
                   UI_ENROLL_SAVE_HEIGHT,
                   "Salvar",
                   UI_COLOR_OK,
                   UI_COLOR_BG);
}

static void ui_extract_initials(const char *name, char *initials, size_t initials_size)
{
    size_t out = 0U;
    bool take_next = true;

    if ((NULL == initials) || (0U == initials_size))
    {
        return;
    }

    initials[0] = '\0';

    if ((NULL == name) || ('\0' == name[0]))
    {
        ui_copy_text(initials, initials_size, "RF");
        return;
    }

    while (('\0' != *name) && ((out + 1U) < initials_size))
    {
        char c = *name;

        if (' ' == c)
        {
            take_next = true;
        }
        else if (take_next)
        {
            if ((c >= 'a') && (c <= 'z'))
            {
                c = (char) (c - ('a' - 'A'));
            }

            initials[out++] = c;
            take_next = false;
        }

        name++;
    }

    initials[out] = '\0';

    if ('\0' == initials[0])
    {
        ui_copy_text(initials, initials_size, "RF");
    }
}

static void ui_draw_hero_panel(int32_t x,
                               int32_t y,
                               int32_t width,
                               int32_t height,
                               const char *name,
                               uint16_t accent,
                               bool idle_mode)
{
    char initials[8];

    ui_extract_initials(name, initials, sizeof(initials));

    if (idle_mode)
    {
        SSP_PARAMETER_NOT_USED(x);
        SSP_PARAMETER_NOT_USED(y);
        SSP_PARAMETER_NOT_USED(width);
        SSP_PARAMETER_NOT_USED(height);
        SSP_PARAMETER_NOT_USED(accent);
        ui_draw_wait_screen("AGUARDANDO USUARIO", "APROXIME O CARTAO", true);
        return;
    }

    ui_fill_rect(0, 0, UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, UI_COLOR_BG);
    ui_fill_rect(x + 12, y + 8, width - 24, height - 34, UI_COLOR_PORTRAIT);
    ui_fill_rect(x + 36, y + 32, width - 72, height - 86, accent);
    ui_draw_text_centered_in_rect(x + 36, width - 72, y + 86, initials, UI_COLOR_TEXT, 2U);
}

static void ui_draw_wait_screen(const char *line1, const char *line2, bool show_gear)
{
    const int32_t frame_x = 2;
    const int32_t frame_y = 10;
    const int32_t frame_w = 236;
    const int32_t frame_h = 178;
    const int32_t inner_x = frame_x + 4;
    const int32_t inner_y = frame_y + 4;
    const int32_t inner_w = frame_w - 8;
    const int32_t inner_h = frame_h - 8;
    const int32_t logo_x = inner_x + ((inner_w - UI_LOGO_WAIT_WIDTH) / 2);
    const int32_t logo_y = inner_y + ((inner_h - UI_LOGO_WAIT_HEIGHT) / 2) - 4;

    ui_fill_rect(0, 0, UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, UI_COLOR_WAIT_BG);
    ui_fill_rect(frame_x, frame_y, frame_w, frame_h, UI_COLOR_WAIT_LINE);
    ui_fill_rect(inner_x, inner_y, inner_w, inner_h, UI_COLOR_TEXT);
    ui_draw_rgb565_image_scaled_cropped(logo_x,
                                        logo_y,
                                        UI_LOGO_WAIT_WIDTH,
                                        UI_LOGO_WAIT_HEIGHT,
                                        g_ramo_logo_rgb565,
                                        (int32_t) RAMO_LOGO_WIDTH,
                                        (int32_t) RAMO_LOGO_HEIGHT,
                                        UI_LOGO_CROP_X,
                                        UI_LOGO_CROP_Y,
                                        UI_LOGO_CROP_WIDTH,
                                        UI_LOGO_CROP_HEIGHT);
    ui_fill_rect(0, frame_y + frame_h, UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT - (frame_y + frame_h), UI_COLOR_WAIT_BAR);
    ui_draw_pixel_text_centered(242, line1, UI_COLOR_TEXT, 2U);
    ui_draw_pixel_text_centered(278, line2, UI_COLOR_WAIT_TEXT2, 1U);

    if (show_gear)
    {
        ui_draw_gear_button(UI_SCREEN_WIDTH - UI_GEAR_BUTTON_SIZE - 12, 12);
    }
}

static void ui_show_splash(void)
{
    ui_draw_wait_screen("AGUARDANDO USUARIO", "APROXIME O CARTAO", false);
    tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND);
}

static void ui_capture_snapshot(ui_snapshot_t *snapshot)
{
    if (NULL == snapshot)
    {
        return;
    }

    app_state_lock();
    snapshot->door_open = g_app_state.door_open;
    snapshot->light_on = g_app_state.light_on;
    snapshot->net_ready = g_app_state.net_ready;
    strncpy(snapshot->last_uid, (const char *) g_app_state.last_uid, UID_MAX_LEN - 1U);
    snapshot->last_uid[UID_MAX_LEN - 1U] = '\0';
    strncpy(snapshot->last_user, (const char *) g_app_state.last_user, NAME_MAX_LEN - 1U);
    snapshot->last_user[NAME_MAX_LEN - 1U] = '\0';
    app_state_unlock();
}

static bool ui_snapshot_changed(const ui_snapshot_t *lhs, const ui_snapshot_t *rhs)
{
    if ((lhs->door_open != rhs->door_open)
        || (lhs->light_on != rhs->light_on)
        || (lhs->net_ready != rhs->net_ready))
    {
        return true;
    }

    if (0 != strncmp(lhs->last_uid, rhs->last_uid, UID_MAX_LEN))
    {
        return true;
    }

    return (0 != strncmp(lhs->last_user, rhs->last_user, NAME_MAX_LEN));
}

static void ui_set_status(ui_status_t *status,
                          const char *line1,
                          const char *line2,
                          const char *context,
                          uint16_t accent,
                          ULONG hold_ticks,
                          bool sticky)
{
    if (NULL == status)
    {
        return;
    }

    ui_copy_text(status->line1, sizeof(status->line1), line1);
    ui_copy_text(status->line2, sizeof(status->line2), line2);
    ui_copy_text(status->context, sizeof(status->context), context);
    status->accent = accent;
    status->sticky = sticky;
    status->deadline = sticky ? 0U : (tx_time_get() + hold_ticks);
    g_ui_debug_status_deadline = status->deadline;
}

static void ui_set_idle_status(ui_status_t *status)
{
    ui_enter_idle(status);
}

static bool ui_status_expired(const ui_status_t *status)
{
    if ((NULL == status) || status->sticky || (0U == status->deadline))
    {
        return false;
    }

    return (tx_time_get() >= status->deadline);
}

static void ui_update_status_from_event(ui_status_t *status, const app_event_t *event)
{
    if ((NULL == status) || (NULL == event))
    {
        return;
    }

    if (UI_VIEW_PIN == status->view)
    {
        if ((EVENT_RFID_AUTH_OK == event->type) ||
            (EVENT_RFID_AUTH_FAIL == event->type) ||
            (EVENT_CARD_REGISTERED == event->type) ||
            (EVENT_CARD_ALREADY_REGISTERED == event->type) ||
            (EVENT_CARD_REGISTRATION_FAILED == event->type))
        {
            return;
        }
    }

    if (UI_VIEW_ENROLL_WAIT == status->view)
    {
        if ((EVENT_RFID_AUTH_OK == event->type) ||
            (EVENT_RFID_AUTH_FAIL == event->type))
        {
            return;
        }
    }

    switch (event->type)
    {
        case EVENT_UI_ADMIN_REQUEST:
            ui_enter_pin_entry(status);
            break;

        case EVENT_UI_NAV_INC:
            if (UI_VIEW_PIN == status->view)
            {
                ui_pin_increment(status);
            }
            break;

        case EVENT_UI_NAV_CONFIRM:
            if (UI_VIEW_PIN == status->view)
            {
                ui_pin_commit(status);
            }
            break;

        case EVENT_UI_NAV_CANCEL:
            ui_enter_idle(status);
            break;

        case EVENT_RFID_AUTH_OK:
            ui_set_status(status,
                          "",
                          "Acesso autorizado",
                          "Toque para voltar",
                          UI_COLOR_OK,
                          UI_STATUS_HOLD_TICKS,
                          false);
            status->view = UI_VIEW_RESULT;
            app_set_ui_mode(APP_UI_MODE_IDLE);
            app_set_enrollment_mode(false);
            break;

        case EVENT_RFID_AUTH_FAIL:
            ui_set_status(status,
                          "",
                          "Acesso negado",
                          "Toque para voltar",
                          UI_COLOR_ERROR,
                          UI_STATUS_HOLD_TICKS,
                          false);
            status->view = UI_VIEW_RESULT;
            app_set_ui_mode(APP_UI_MODE_IDLE);
            app_set_enrollment_mode(false);
            break;

        case EVENT_DOOR_OPEN:
        case EVENT_DOOR_CLOSE:
        case EVENT_LIGHT_ON:
        case EVENT_LIGHT_OFF:
            break;

        case EVENT_USER_ADDED:
            ui_set_status(status,
                          "",
                          "Usu" "\303\241" "rio adicionado",
                          "Toque para voltar",
                          UI_COLOR_OK,
                          UI_STATUS_HOLD_TICKS,
                          false);
            status->view = UI_VIEW_RESULT;
            app_set_ui_mode(APP_UI_MODE_IDLE);
            break;

        case EVENT_USER_REMOVED:
            ui_set_status(status,
                          "",
                          "Usu" "\303\241" "rio removido",
                          "Toque para voltar",
                          UI_COLOR_WARN,
                          UI_STATUS_HOLD_TICKS,
                          false);
            status->view = UI_VIEW_RESULT;
            app_set_ui_mode(APP_UI_MODE_IDLE);
            break;

        case EVENT_CARD_REGISTERED:
            ui_set_status(status,
                          "",
                          "Cart" "\303\243" "o cadastrado",
                          "Toque para voltar",
                          UI_COLOR_OK,
                          UI_STATUS_HOLD_TICKS,
                          false);
            status->view = UI_VIEW_RESULT;
            app_set_ui_mode(APP_UI_MODE_IDLE);
            app_set_enrollment_mode(false);
            break;

        case EVENT_CARD_ALREADY_REGISTERED:
            ui_set_status(status,
                          "",
                          "Cart" "\303\243" "o j" "\303\241" " cadastrado",
                          "Toque para voltar",
                          UI_COLOR_WARN,
                          UI_STATUS_HOLD_TICKS,
                          false);
            status->view = UI_VIEW_RESULT;
            app_set_ui_mode(APP_UI_MODE_IDLE);
            app_set_enrollment_mode(false);
            break;

        case EVENT_CARD_REGISTRATION_FAILED:
            ui_set_status(status,
                          "",
                          "Falha no cadastro",
                          "Toque para voltar",
                          UI_COLOR_ERROR,
                          UI_STATUS_HOLD_TICKS,
                          false);
            status->view = UI_VIEW_RESULT;
            app_set_ui_mode(APP_UI_MODE_IDLE);
            app_set_enrollment_mode(false);
            break;

        default:
            ui_set_status(status,
                          "",
                          "Processando",
                          "Toque para voltar",
                          UI_COLOR_IDLE,
                          UI_STATUS_HOLD_TICKS,
                          false);
            status->view = UI_VIEW_RESULT;
            app_set_ui_mode(APP_UI_MODE_IDLE);
            break;
    }
}

static bool ui_point_in_rect(int32_t x, int32_t y, int32_t rx, int32_t ry, int32_t rw, int32_t rh)
{
    return (x >= rx) && (x < (rx + rw)) && (y >= ry) && (y < (ry + rh));
}

static void ui_handle_touch(ui_status_t *status, const ui_touch_event_t *touch_event, bool *force_redraw)
{
    int32_t x;
    int32_t y;

    if ((NULL == status)
        || (NULL == touch_event)
        || (NULL == force_redraw)
        || !touch_event->valid)
    {
        return;
    }

    if ((UI_VIEW_RESULT == status->view)
        && touch_event->just_pressed
        && ui_touch_accept_action())
    {
        ui_enter_idle(status);
        *force_redraw = true;
        return;
    }

    if ((UI_VIEW_ENROLL_WAIT == status->view)
        && touch_event->just_pressed)
    {
        x = touch_event->x;
        y = touch_event->y;

        if (ui_enroll_back_hit_test(x, y) && ui_touch_accept_action())
        {
            ui_enter_idle(status);
            *force_redraw = true;
            return;
        }

        if (ui_enroll_save_hit_test(x, y) && ui_touch_accept_action())
        {
            if (storage_persist_now())
            {
                ui_set_status(status,
                              "",
                              "Cadastros salvos",
                              "Toque para voltar",
                              UI_COLOR_OK,
                              UI_STATUS_HOLD_TICKS,
                              false);
            }
            else
            {
                ui_set_status(status,
                              "",
                              "Falha ao salvar",
                              "Tente novamente",
                              UI_COLOR_ERROR,
                              UI_STATUS_HOLD_TICKS,
                              false);
            }

            status->view = UI_VIEW_RESULT;
            app_set_ui_mode(APP_UI_MODE_IDLE);
            app_set_enrollment_mode(false);
            *force_redraw = true;
        }
        return;
    }

    if (!touch_event->just_pressed)
    {
        return;
    }

    if (!ui_touch_accept_action())
    {
        return;
    }

    x = touch_event->x;
    y = touch_event->y;

    if (UI_VIEW_IDLE == status->view)
    {
        if (ui_point_in_rect(x,
                             y,
                             UI_SCREEN_WIDTH - UI_GEAR_BUTTON_SIZE - 12,
                             12,
                             UI_GEAR_BUTTON_SIZE,
                             UI_GEAR_BUTTON_SIZE))
        {
            ui_enter_pin_entry(status);
            *force_redraw = true;
        }
        return;
    }

    if (UI_VIEW_PIN == status->view)
    {
        if (ui_pin_hit_test(x, y, 0, 0)) { ui_pin_set_digit(status, '1'); *force_redraw = true; return; }
        if (ui_pin_hit_test(x, y, 0, 1)) { ui_pin_set_digit(status, '2'); *force_redraw = true; return; }
        if (ui_pin_hit_test(x, y, 0, 2)) { ui_pin_set_digit(status, '3'); *force_redraw = true; return; }
        if (ui_pin_hit_test(x, y, 1, 0)) { ui_pin_set_digit(status, '4'); *force_redraw = true; return; }
        if (ui_pin_hit_test(x, y, 1, 1)) { ui_pin_set_digit(status, '5'); *force_redraw = true; return; }
        if (ui_pin_hit_test(x, y, 1, 2)) { ui_pin_set_digit(status, '6'); *force_redraw = true; return; }
        if (ui_pin_hit_test(x, y, 2, 0)) { ui_pin_set_digit(status, '7'); *force_redraw = true; return; }
        if (ui_pin_hit_test(x, y, 2, 1)) { ui_pin_set_digit(status, '8'); *force_redraw = true; return; }
        if (ui_pin_hit_test(x, y, 2, 2)) { ui_pin_set_digit(status, '9'); *force_redraw = true; return; }
        if (ui_pin_hit_test(x, y, 3, 0)) { ui_pin_backspace(status); *force_redraw = true; return; }
        if (ui_pin_hit_test(x, y, 3, 1)) { ui_pin_set_digit(status, '0'); *force_redraw = true; return; }
        if (ui_pin_enter_hit_test(x, y) || ui_pin_hit_test(x, y, 3, 2)) { ui_pin_submit(status); *force_redraw = true; return; }
        return;
    }

    SSP_PARAMETER_NOT_USED(x);
    SSP_PARAMETER_NOT_USED(y);
}

static void ui_init_display(void)
{
    if (g_ui_display_ready)
    {
        return;
    }

    memset(g_display0_fb_background[0], 0, sizeof(g_display0_fb_background[0]));

    (void) g_display0.p_api->open(g_display0.p_ctrl, g_display0.p_cfg);
    (void) g_display0.p_api->start(g_display0.p_ctrl);
    lcd_setup();
    ui_set_backlight(true);
    ui_touch_init();

    g_ui_display_ready = true;
}

static void ui_render(const ui_status_t *status, const ui_snapshot_t *snapshot)
{
    const char *user_text;
    bool idle_mode;
    uint16_t hero_color;
    if ((NULL == status) || (NULL == snapshot))
    {
        return;
    }

    idle_mode = status->sticky;
    user_text = idle_mode ? "Aguardando cartão" : snapshot->last_user;
    hero_color = idle_mode ? UI_COLOR_BRAND : status->accent;

    ui_draw_hero_panel(8, 14, 224, 214, user_text, hero_color, idle_mode);

    if (idle_mode)
    {
        return;
    }

    ui_fill_rect(0, 236, UI_SCREEN_WIDTH, 84, UI_COLOR_PANEL_ALT);
    ui_draw_text_centered(248, user_text, UI_COLOR_TEXT, 1U);
    ui_draw_text_centered(274, status->line2, status->accent, 1U);
    if ('\0' != status->context[0])
    {
        ui_draw_text_centered(298, status->context, UI_COLOR_TEXT, 1U);
    }
}

static void ui_render_screen(const ui_status_t *status, const ui_snapshot_t *snapshot)
{
    const char *user_text;

    if ((NULL == status) || (NULL == snapshot))
    {
        return;
    }

    if (UI_VIEW_IDLE == status->view)
    {
        ui_draw_wait_screen("AGUARDANDO USUARIO", "APROXIME O CARTAO", false);
        return;
    }

    if (UI_VIEW_PIN == status->view)
    {
        ui_draw_pin_screen(status);
        return;
    }

    if (UI_VIEW_ENROLL_WAIT == status->view)
    {
        ui_draw_enroll_wait_screen(status);
        return;
    }

    user_text = snapshot->last_user;
    SSP_PARAMETER_NOT_USED(user_text);
    ui_render(status, snapshot);
}

void thread_ui_entry(ULONG arg)
{
    app_event_t event;
    ui_status_t status;
    ui_snapshot_t current_snapshot = { 0 };
    ui_snapshot_t previous_snapshot = { 0 };
    bool force_redraw = true;
    ULONG last_activity_tick = 0U;

    SSP_PARAMETER_NOT_USED(arg);

    ui_init_display();
    ui_show_splash();
    ui_set_idle_status(&status);
    ui_capture_snapshot(&previous_snapshot);
    ui_capture_snapshot(&current_snapshot);
    last_activity_tick = tx_time_get();

    while (1)
    {
        ui_touch_event_t touch_event = { 0 };
        g_ui_debug_loop_count++;
        g_ui_debug_last_view = (uint32_t) status.view;
        g_ui_debug_last_tick = tx_time_get();

        if (TX_SUCCESS == tx_queue_receive(&g_event_queue, &event, UI_QUEUE_POLL_TICKS))
        {
            ui_update_status_from_event(&status, &event);
            force_redraw = true;
            last_activity_tick = tx_time_get();

            if (!g_ui_backlight_on)
            {
                ui_set_backlight(true);
            }
        }

        if (ui_touch_poll(&touch_event))
        {
            ui_handle_touch(&status, &touch_event, &force_redraw);
            last_activity_tick = tx_time_get();

            if (!g_ui_backlight_on)
            {
                ui_set_backlight(true);
            }
        }

        if (ui_status_expired(&status))
        {
            ui_set_idle_status(&status);
            force_redraw = true;
            last_activity_tick = tx_time_get();
            g_ui_debug_expire_count++;
        }

        ui_capture_snapshot(&current_snapshot);

        if (force_redraw || ui_snapshot_changed(&previous_snapshot, &current_snapshot))
        {
            ui_render_screen(&status, &current_snapshot);
            previous_snapshot = current_snapshot;
            force_redraw = false;

            if (!g_ui_backlight_on)
            {
                ui_set_backlight(true);
            }
        }

        if (status.sticky && g_ui_backlight_on && ((tx_time_get() - last_activity_tick) >= UI_BACKLIGHT_TIMEOUT_TICKS))
        {
            ui_set_backlight(false);
        }
    }
}
