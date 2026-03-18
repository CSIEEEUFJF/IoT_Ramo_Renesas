#include "rfid.h"
#include "main.h"
#include "storage.h"

/* Arduino header on SK-S7G2: D10=SS, D9=RST, D11=MOSI, D12=MISO, D13=SCK. */
#define PIN_RFID_SS              IOPORT_PORT_05_PIN_07
#define PIN_RFID_RST             IOPORT_PORT_05_PIN_06
#define PIN_RFID_MOSI            IOPORT_PORT_01_PIN_05
#define PIN_RFID_MISO            IOPORT_PORT_01_PIN_04
#define PIN_RFID_SCK             IOPORT_PORT_01_PIN_06

#define RFID_POLL_TICKS          (TX_TIMER_TICKS_PER_SECOND / 10U)
#define RFID_CARD_LOST_TICKS     (TX_TIMER_TICKS_PER_SECOND / 5U)
#define RFID_STARTUP_DELAY_TICKS (TX_TIMER_TICKS_PER_SECOND * 2U)
#define RFID_SPI_TIMEOUT_LOOPS   2000U
#define RFID_SPI_DELAY_US        1U
#define RFID_MAX_FRAME_BYTES     18U
#define RFID_ENROLL_CONFIRM_READS 2U

#define RC522_CMD_IDLE           0x00U
#define RC522_CMD_CALC_CRC       0x03U
#define RC522_CMD_TRANSCEIVE     0x0CU
#define RC522_CMD_SOFT_RESET     0x0FU

#define RC522_REG_COMMAND        0x01U
#define RC522_REG_COMMIEN        0x02U
#define RC522_REG_DIVIRQ         0x05U
#define RC522_REG_COMMIRQ        0x04U
#define RC522_REG_ERROR          0x06U
#define RC522_REG_FIFO_DATA      0x09U
#define RC522_REG_FIFO_LEVEL     0x0AU
#define RC522_REG_CONTROL        0x0CU
#define RC522_REG_BIT_FRAMING    0x0DU
#define RC522_REG_COLL           0x0EU
#define RC522_REG_MODE           0x11U
#define RC522_REG_VERSION        0x37U
#define RC522_REG_TX_CONTROL     0x14U
#define RC522_REG_TX_ASK         0x15U
#define RC522_REG_TMODE          0x2AU
#define RC522_REG_TPRESCALER     0x2BU
#define RC522_REG_TRELOAD_H      0x2CU
#define RC522_REG_TRELOAD_L      0x2DU
#define RC522_REG_CRC_RESULT_H   0x21U
#define RC522_REG_CRC_RESULT_L   0x22U

#define PICC_CMD_REQA            0x26U
#define PICC_CMD_SEL_CL1         0x93U
#define PICC_CMD_SEL_CL2         0x95U
#define PICC_CMD_SEL_CL3         0x97U

#define RC522_STATUS_OK          0U
#define RC522_STATUS_NO_CARD     1U
#define RC522_STATUS_ERROR       2U

volatile uint32_t g_rfid_debug_state = 0U;
volatile uint32_t g_rfid_debug_reg_version = 0U;
volatile uint32_t g_rfid_debug_last_status = 0U;
volatile uint32_t g_rfid_debug_last_uid_size = 0U;
volatile char g_rfid_debug_last_uid[UID_MAX_LEN] = { 0 };
volatile uint32_t g_rfid_debug_request_status = 0U;
volatile uint32_t g_rfid_debug_request_bits = 0U;
volatile uint32_t g_rfid_debug_anticoll_status = 0U;
volatile uint32_t g_rfid_debug_anticoll_bits = 0U;
volatile uint32_t g_rfid_debug_select_status = 0U;
volatile uint32_t g_rfid_debug_select_bits = 0U;
volatile uint32_t g_rfid_debug_last_level = 0U;
volatile uint32_t g_rfid_debug_last_sak = 0U;
volatile uint8_t g_rfid_debug_level_serial[5] = { 0 };
volatile uint32_t g_rfid_debug_loop_count = 0U;
volatile uint32_t g_rfid_debug_last_tick = 0U;
volatile uint32_t g_rfid_debug_enroll_mode = 0U;
volatile uint32_t g_rfid_debug_enroll_session_armed = 0U;
volatile uint32_t g_rfid_debug_enroll_attempt_count = 0U;
volatile uint32_t g_rfid_debug_enroll_result = 0U;
volatile uint32_t g_rfid_debug_enroll_stage = 0U;

void rfid_spi_callback(spi_callback_args_t *p_args)
{
    SSP_PARAMETER_NOT_USED(p_args);
}

static bool rfid_spi_transfer(uint8_t *tx_buf, uint8_t *rx_buf, uint32_t length)
{
    g_ioport.p_api->pinWrite(PIN_RFID_SS, IOPORT_LEVEL_LOW);

    for (uint32_t byte_index = 0U; byte_index < length; byte_index++)
    {
        uint8_t tx_byte = tx_buf[byte_index];
        uint8_t rx_byte = 0U;

        for (uint8_t bit = 0U; bit < 8U; bit++)
        {
            ioport_level_t miso_level = IOPORT_LEVEL_LOW;

            g_ioport.p_api->pinWrite(PIN_RFID_MOSI, (0U != (tx_byte & 0x80U)) ? IOPORT_LEVEL_HIGH : IOPORT_LEVEL_LOW);
            tx_byte <<= 1;

            g_ioport.p_api->pinWrite(PIN_RFID_SCK, IOPORT_LEVEL_HIGH);
            R_BSP_SoftwareDelay(RFID_SPI_DELAY_US, BSP_DELAY_UNITS_MICROSECONDS);
            (void) g_ioport.p_api->pinRead(PIN_RFID_MISO, &miso_level);

            rx_byte <<= 1;
            if (IOPORT_LEVEL_HIGH == miso_level)
            {
                rx_byte |= 0x01U;
            }

            g_ioport.p_api->pinWrite(PIN_RFID_SCK, IOPORT_LEVEL_LOW);
            R_BSP_SoftwareDelay(RFID_SPI_DELAY_US, BSP_DELAY_UNITS_MICROSECONDS);
        }

        if (NULL != rx_buf)
        {
            rx_buf[byte_index] = rx_byte;
        }
    }

    g_ioport.p_api->pinWrite(PIN_RFID_SS, IOPORT_LEVEL_HIGH);
    return true;
}

static void rc522_write_reg(uint8_t reg, uint8_t val)
{
    uint8_t tx_buf[2] = { (uint8_t) ((reg << 1) & 0x7EU), val };
    uint8_t rx_buf[2] = { 0 };

    if (!rfid_spi_transfer(tx_buf, rx_buf, 2U))
    {
        while (1)
        {
        }
    }
}

static uint8_t rc522_read_reg(uint8_t reg)
{
    uint8_t tx_buf[2] = { (uint8_t) ((((uint8_t) (reg << 1)) & 0x7EU) | 0x80U), 0x00U };
    uint8_t rx_buf[2] = { 0 };

    if (!rfid_spi_transfer(tx_buf, rx_buf, 2U))
    {
        return 0U;
    }

    return rx_buf[1];
}

static void rc522_set_bit_mask(uint8_t reg, uint8_t mask)
{
    rc522_write_reg(reg, (uint8_t) (rc522_read_reg(reg) | mask));
}

static void rc522_clear_bit_mask(uint8_t reg, uint8_t mask)
{
    rc522_write_reg(reg, (uint8_t) (rc522_read_reg(reg) & ((uint8_t) (~mask))));
}

static void rc522_antenna_on(void)
{
    uint8_t value = rc522_read_reg(RC522_REG_TX_CONTROL);

    if ((value & 0x03U) != 0x03U)
    {
        rc522_set_bit_mask(RC522_REG_TX_CONTROL, 0x03U);
    }
}

static void rc522_reset(void)
{
    rc522_write_reg(RC522_REG_COMMAND, RC522_CMD_SOFT_RESET);
    tx_thread_sleep(1);
}

static void rc522_init_chip(void)
{
    rc522_reset();
    rc522_write_reg(RC522_REG_TMODE, 0x8DU);
    rc522_write_reg(RC522_REG_TPRESCALER, 0x3EU);
    rc522_write_reg(RC522_REG_TRELOAD_L, 30U);
    rc522_write_reg(RC522_REG_TRELOAD_H, 0U);
    rc522_write_reg(RC522_REG_TX_ASK, 0x40U);
    rc522_write_reg(RC522_REG_MODE, 0x3DU);
    rc522_antenna_on();
}

static uint8_t rc522_calculate_crc(const uint8_t *data, uint8_t length, uint8_t *out_crc)
{
    uint32_t timeout = RFID_SPI_TIMEOUT_LOOPS;

    rc522_write_reg(RC522_REG_COMMAND, RC522_CMD_IDLE);
    rc522_write_reg(RC522_REG_DIVIRQ, 0x04U);
    rc522_write_reg(RC522_REG_FIFO_LEVEL, 0x80U);

    for (uint8_t i = 0U; i < length; i++)
    {
        rc522_write_reg(RC522_REG_FIFO_DATA, data[i]);
    }

    rc522_write_reg(RC522_REG_COMMAND, RC522_CMD_CALC_CRC);

    while (timeout-- > 0U)
    {
        if (0U != (rc522_read_reg(RC522_REG_DIVIRQ) & 0x04U))
        {
            out_crc[0] = rc522_read_reg(RC522_REG_CRC_RESULT_L);
            out_crc[1] = rc522_read_reg(RC522_REG_CRC_RESULT_H);
            return RC522_STATUS_OK;
        }
    }

    return RC522_STATUS_ERROR;
}

static uint8_t rc522_transceive(const uint8_t *send_data,
                                uint8_t send_length,
                                uint8_t *back_data,
                                uint32_t *back_bits,
                                uint8_t tx_last_bits)
{
    uint32_t timeout = RFID_SPI_TIMEOUT_LOOPS;
    uint8_t irq;
    uint8_t fifo_level;
    uint8_t last_bits;
    uint8_t error;

    rc522_write_reg(RC522_REG_COMMAND, RC522_CMD_IDLE);
    rc522_write_reg(RC522_REG_COMMIEN, 0xF7U);
    rc522_write_reg(RC522_REG_COMMIRQ, 0x7FU);
    rc522_write_reg(RC522_REG_FIFO_LEVEL, 0x80U);
    rc522_write_reg(RC522_REG_BIT_FRAMING, tx_last_bits);

    for (uint8_t i = 0U; i < send_length; i++)
    {
        rc522_write_reg(RC522_REG_FIFO_DATA, send_data[i]);
    }

    rc522_write_reg(RC522_REG_COMMAND, RC522_CMD_TRANSCEIVE);
    rc522_set_bit_mask(RC522_REG_BIT_FRAMING, 0x80U);

    do
    {
        irq = rc522_read_reg(RC522_REG_COMMIRQ);
        timeout--;
    } while ((timeout > 0U) && (0U == (irq & 0x31U)));

    rc522_clear_bit_mask(RC522_REG_BIT_FRAMING, 0x80U);

    if (0U == timeout)
    {
        return RC522_STATUS_NO_CARD;
    }

    error = rc522_read_reg(RC522_REG_ERROR);
    if (0U != (error & 0x1BU))
    {
        return RC522_STATUS_ERROR;
    }

    if (0U != (irq & 0x01U))
    {
        return RC522_STATUS_NO_CARD;
    }

    fifo_level = rc522_read_reg(RC522_REG_FIFO_LEVEL);
    last_bits = (uint8_t) (rc522_read_reg(RC522_REG_CONTROL) & 0x07U);

    if (NULL != back_bits)
    {
        *back_bits = (0U != last_bits) ? (((uint32_t) (fifo_level - 1U) * 8U) + last_bits)
                                       : ((uint32_t) fifo_level * 8U);
    }

    if (NULL != back_data)
    {
        if (fifo_level > RFID_MAX_FRAME_BYTES)
        {
            fifo_level = RFID_MAX_FRAME_BYTES;
        }

        for (uint8_t i = 0U; i < fifo_level; i++)
        {
            back_data[i] = rc522_read_reg(RC522_REG_FIFO_DATA);
        }
    }

    return RC522_STATUS_OK;
}

static uint8_t rc522_request(void)
{
    uint8_t req = PICC_CMD_REQA;
    uint32_t back_bits = 0U;

    rc522_write_reg(RC522_REG_BIT_FRAMING, 0x07U);

    g_rfid_debug_request_status = rc522_transceive(&req, 1U, NULL, &back_bits, 0x07U);
    g_rfid_debug_request_bits = back_bits;

    if (RC522_STATUS_OK != g_rfid_debug_request_status)
    {
        return RC522_STATUS_NO_CARD;
    }

    return (16U == back_bits) ? RC522_STATUS_OK : RC522_STATUS_ERROR;
}

static uint8_t rc522_anticoll(uint8_t cascade_cmd, uint8_t *serial)
{
    uint8_t buffer[RFID_MAX_FRAME_BYTES] = { cascade_cmd, 0x20U };
    uint32_t back_bits = 0U;
    uint8_t bcc = 0U;

    rc522_write_reg(RC522_REG_BIT_FRAMING, 0x00U);
    rc522_write_reg(RC522_REG_COLL, 0x80U);

    g_rfid_debug_anticoll_status = rc522_transceive(buffer, 2U, buffer, &back_bits, 0x00U);
    g_rfid_debug_anticoll_bits = back_bits;

    if (RC522_STATUS_OK != g_rfid_debug_anticoll_status)
    {
        return RC522_STATUS_ERROR;
    }

    if (40U != back_bits)
    {
        return RC522_STATUS_ERROR;
    }

    for (uint8_t i = 0U; i < 4U; i++)
    {
        serial[i] = buffer[i];
        bcc ^= buffer[i];
    }

    serial[4] = buffer[4];
    memcpy((void *) g_rfid_debug_level_serial, serial, sizeof(g_rfid_debug_level_serial));
    return (bcc == buffer[4]) ? RC522_STATUS_OK : RC522_STATUS_ERROR;
}

static uint8_t rc522_select(uint8_t cascade_cmd, const uint8_t *serial, uint8_t *sak)
{
    uint8_t buffer[9];
    uint8_t response[3] = { 0 };
    uint32_t back_bits = 0U;

    buffer[0] = cascade_cmd;
    buffer[1] = 0x70U;
    memcpy(&buffer[2], serial, 5U);

    if (RC522_STATUS_OK != rc522_calculate_crc(buffer, 7U, &buffer[7]))
    {
        return RC522_STATUS_ERROR;
    }

    g_rfid_debug_select_status = rc522_transceive(buffer, 9U, response, &back_bits, 0x00U);
    g_rfid_debug_select_bits = back_bits;

    if (RC522_STATUS_OK != g_rfid_debug_select_status)
    {
        return RC522_STATUS_ERROR;
    }

    if (24U != back_bits)
    {
        return RC522_STATUS_ERROR;
    }

    *sak = response[0];
    g_rfid_debug_last_sak = *sak;
    return RC522_STATUS_OK;
}

static bool rc522_read_uid(uint8_t *uid, uint8_t *uid_size)
{
    static const uint8_t cascade_levels[] = { PICC_CMD_SEL_CL1, PICC_CMD_SEL_CL2, PICC_CMD_SEL_CL3 };
    uint8_t level_serial[5];
    uint8_t sak = 0U;
    uint8_t length = 0U;

    if (RC522_STATUS_OK != rc522_request())
    {
        return false;
    }

    /* Read each cascade level so cards with 4, 7, or 10-byte UIDs can be handled. */
    for (uint8_t level = 0U; level < (sizeof(cascade_levels) / sizeof(cascade_levels[0])); level++)
    {
        g_rfid_debug_last_level = (uint32_t) (level + 1U);

        if (RC522_STATUS_OK != rc522_anticoll(cascade_levels[level], level_serial))
        {
            return false;
        }

        if (RC522_STATUS_OK != rc522_select(cascade_levels[level], level_serial, &sak))
        {
            return false;
        }

        if ((0U == level) && (0x88U == level_serial[0]))
        {
            memcpy(&uid[length], &level_serial[1], 3U);
            length = (uint8_t) (length + 3U);
        }
        else if ((level > 0U) && (0x88U == level_serial[0]))
        {
            memcpy(&uid[length], &level_serial[1], 3U);
            length = (uint8_t) (length + 3U);
        }
        else
        {
            memcpy(&uid[length], &level_serial[0], 4U);
            length = (uint8_t) (length + 4U);
        }

        if (0U == (sak & 0x04U))
        {
            *uid_size = length;
            return true;
        }
    }

    return false;
}

static void rfid_uid_to_string(const uint8_t *uid, uint8_t uid_size, char *buffer, size_t buffer_size)
{
    static const char hex[] = "0123456789ABCDEF";
    size_t out = 0U;

    if ((NULL == buffer) || (0U == buffer_size))
    {
        return;
    }

    for (uint8_t i = 0U; (i < uid_size) && ((out + 2U) < buffer_size); i++)
    {
        buffer[out++] = hex[(uid[i] >> 4) & 0x0FU];
        buffer[out++] = hex[uid[i] & 0x0FU];
    }

    buffer[out] = '\0';
}

static void rfid_make_enroll_name(const char *uid_text, char *name_buffer, size_t name_buffer_size)
{
    const char *suffix = uid_text;
    size_t uid_length;

    if ((NULL == name_buffer) || (0U == name_buffer_size))
    {
        return;
    }

    if ((NULL == uid_text) || ('\0' == uid_text[0]))
    {
        name_buffer[0] = '\0';
        return;
    }

    uid_length = strlen(uid_text);
    if (uid_length > 4U)
    {
        suffix = &uid_text[uid_length - 4U];
    }

    (void) snprintf(name_buffer, name_buffer_size, "Cartao %s", suffix);
}

void rfid_init(void)
{
    g_rfid_debug_state = 1U;
    (void) g_ioport.p_api->pinCfg(PIN_RFID_SS,
                                  IOPORT_CFG_PORT_DIRECTION_OUTPUT |
                                  IOPORT_CFG_PORT_OUTPUT_HIGH |
                                  IOPORT_CFG_DRIVE_MID);
    (void) g_ioport.p_api->pinCfg(PIN_RFID_RST,
                                  IOPORT_CFG_PORT_DIRECTION_OUTPUT |
                                  IOPORT_CFG_PORT_OUTPUT_LOW |
                                  IOPORT_CFG_DRIVE_MID);
    (void) g_ioport.p_api->pinCfg(PIN_RFID_MOSI,
                                  IOPORT_CFG_PORT_DIRECTION_OUTPUT |
                                  IOPORT_CFG_PORT_OUTPUT_LOW |
                                  IOPORT_CFG_DRIVE_MID);
    (void) g_ioport.p_api->pinCfg(PIN_RFID_MISO,
                                  IOPORT_CFG_PORT_DIRECTION_INPUT);
    (void) g_ioport.p_api->pinCfg(PIN_RFID_SCK,
                                  IOPORT_CFG_PORT_DIRECTION_OUTPUT |
                                  IOPORT_CFG_PORT_OUTPUT_LOW |
                                  IOPORT_CFG_DRIVE_MID);
    g_ioport.p_api->pinWrite(PIN_RFID_SS, IOPORT_LEVEL_HIGH);
    g_ioport.p_api->pinWrite(PIN_RFID_RST, IOPORT_LEVEL_LOW);
    g_ioport.p_api->pinWrite(PIN_RFID_MOSI, IOPORT_LEVEL_LOW);
    g_ioport.p_api->pinWrite(PIN_RFID_SCK, IOPORT_LEVEL_LOW);
    tx_thread_sleep(2);
    g_rfid_debug_state = 2U;
    g_ioport.p_api->pinWrite(PIN_RFID_RST, IOPORT_LEVEL_HIGH);
    tx_thread_sleep(5);

    g_rfid_debug_state = 3U;
    g_rfid_debug_state = 4U;
    rc522_init_chip();
    g_rfid_debug_reg_version = rc522_read_reg(RC522_REG_VERSION);
    g_rfid_debug_state = 5U;
}

void thread_rfid_entry(ULONG arg)
{
    uint8_t uid[10];
    uint8_t uid_size = 0U;
    char uid_text[UID_MAX_LEN];
    char last_seen_uid[UID_MAX_LEN] = { 0 };
    char enroll_candidate_uid[UID_MAX_LEN] = { 0 };
    char user_name[NAME_MAX_LEN];
    bool previous_enroll_mode = false;
    bool enroll_session_armed = false;
    bool card_present = false;
    uint8_t enroll_candidate_reads = 0U;
    ULONG last_seen_tick = 0U;

    SSP_PARAMETER_NOT_USED(arg);
    tx_thread_sleep(RFID_STARTUP_DELAY_TICKS);
    storage_init();
    rfid_init();

    while (1)
    {
        bool enroll_mode = app_is_enrollment_mode();
        g_rfid_debug_loop_count++;
        g_rfid_debug_last_tick = tx_time_get();
        g_rfid_debug_enroll_mode = enroll_mode ? 1U : 0U;

        if (enroll_mode != previous_enroll_mode)
        {
            previous_enroll_mode = enroll_mode;
            enroll_session_armed = enroll_mode;
            enroll_candidate_reads = 0U;
            enroll_candidate_uid[0] = '\0';
            card_present = false;
            last_seen_uid[0] = '\0';
            g_rfid_debug_enroll_attempt_count = 0U;
            g_rfid_debug_enroll_result = 0U;
            g_rfid_debug_enroll_stage = 0U;
        }
        g_rfid_debug_enroll_session_armed = enroll_session_armed ? 1U : 0U;

        if (rc522_read_uid(uid, &uid_size))
        {
            rfid_uid_to_string(uid, uid_size, uid_text, sizeof(uid_text));
            g_rfid_debug_last_status = RC522_STATUS_OK;
            g_rfid_debug_last_uid_size = uid_size;
            memcpy((char *) g_rfid_debug_last_uid, uid_text, sizeof(g_rfid_debug_last_uid) - 1U);
            g_rfid_debug_last_uid[sizeof(g_rfid_debug_last_uid) - 1U] = '\0';
            last_seen_tick = tx_time_get();

            if (enroll_mode)
            {
                if (enroll_session_armed)
                {
                    if (0 == strncmp(uid_text, enroll_candidate_uid, sizeof(enroll_candidate_uid)))
                    {
                        if (enroll_candidate_reads < 0xFFU)
                        {
                            enroll_candidate_reads++;
                        }
                    }
                    else
                    {
                        strncpy(enroll_candidate_uid, uid_text, sizeof(enroll_candidate_uid) - 1U);
                        enroll_candidate_uid[sizeof(enroll_candidate_uid) - 1U] = '\0';
                        enroll_candidate_reads = 1U;
                    }

                    if (enroll_candidate_reads < RFID_ENROLL_CONFIRM_READS)
                    {
                        tx_thread_sleep(RFID_POLL_TICKS);
                        continue;
                    }

                    g_rfid_debug_enroll_attempt_count++;
                    enroll_session_armed = false;
                    card_present = false;
                    last_seen_uid[0] = '\0';
                    enroll_candidate_reads = 0U;
                    enroll_candidate_uid[0] = '\0';
                    g_rfid_debug_enroll_session_armed = 0U;
                    g_rfid_debug_enroll_stage = 1U;

                    g_rfid_debug_enroll_stage = 2U;
                    if (storage_check_uid(uid_text, user_name))
                    {
                        g_rfid_debug_enroll_stage = 3U;
                        g_rfid_debug_enroll_result = 1U;
                        app_set_last_identity(uid_text, user_name);
                        app_set_enrollment_mode(false);
                        app_post_event(EVENT_CARD_ALREADY_REGISTERED, uid_text);
                    }
                    else
                    {
                        g_rfid_debug_enroll_stage = 4U;
                        rfid_make_enroll_name(uid_text, user_name, sizeof(user_name));
                        g_rfid_debug_enroll_stage = 5U;
                        if (storage_add_user(uid_text, user_name))
                        {
                            g_rfid_debug_enroll_stage = 6U;
                            g_rfid_debug_enroll_result = 2U;
                            app_set_last_identity(uid_text, user_name);
                            app_set_enrollment_mode(false);
                            app_post_event(EVENT_CARD_REGISTERED, uid_text);
                        }
                        else
                        {
                            g_rfid_debug_enroll_stage = 7U;
                            g_rfid_debug_enroll_result = 3U;
                            app_set_last_identity(uid_text, "Falha no cadastro");
                            app_set_enrollment_mode(false);
                            app_post_event(EVENT_CARD_REGISTRATION_FAILED, uid_text);
                        }
                    }
                }
            }
            else if ((!card_present) || (0 != strncmp(uid_text, last_seen_uid, sizeof(last_seen_uid))))
            {
                card_present = true;
                strncpy(last_seen_uid, uid_text, sizeof(last_seen_uid) - 1U);
                last_seen_uid[sizeof(last_seen_uid) - 1U] = '\0';

                if (storage_check_uid(uid_text, user_name))
                {
                    app_set_last_identity(uid_text, user_name);
                    app_set_door(true);
                    app_post_event(EVENT_RFID_AUTH_OK, uid_text);
                }
                else
                {
                    app_set_last_identity(uid_text, "N" "\303\243" "o cadastrado");
                    app_post_event(EVENT_RFID_AUTH_FAIL, uid_text);
                }
            }
        }
        else if (card_present && ((tx_time_get() - last_seen_tick) > RFID_CARD_LOST_TICKS))
        {
            card_present = false;
            last_seen_uid[0] = '\0';
        }
        else
        {
            g_rfid_debug_last_status = RC522_STATUS_NO_CARD;
        }

        tx_thread_sleep(RFID_POLL_TICKS);
    }
}
