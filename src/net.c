#include "net.h"
#include "main.h"
#include "storage.h"
#include "ui.h"
#include "assets/profile_photo_ids.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern NX_IP g_ip0;
extern NX_HTTP_SERVER g_http_server0;
extern NX_REC nx_record1;

volatile ULONG g_net_debug_state = 0U;
volatile ULONG g_net_debug_link_status = 0U;
volatile ULONG g_net_debug_ip_status = 0U;
volatile ULONG g_net_debug_ip_address = 0U;
volatile ULONG g_net_debug_network_mask = 0U;
volatile ULONG g_net_debug_http_status = 0U;
volatile ULONG g_net_debug_media_status = 0U;
volatile ULONG g_net_debug_request_count = 0U;
volatile ULONG g_net_debug_last_request_type = 0U;
volatile ULONG g_net_debug_last_response_status = 0U;
volatile char  g_net_debug_last_resource[64] = {0};
volatile ULONG g_net_debug_ip_packets_sent = 0U;
volatile ULONG g_net_debug_ip_packets_received = 0U;
volatile ULONG g_net_debug_arp_requests_sent = 0U;
volatile ULONG g_net_debug_arp_requests_received = 0U;
volatile ULONG g_net_debug_arp_responses_sent = 0U;
volatile ULONG g_net_debug_arp_responses_received = 0U;
volatile ULONG g_net_debug_ip_invalid_packets = 0U;
volatile ULONG g_net_debug_ip_receive_packets_dropped = 0U;
volatile ULONG g_net_debug_ip_receive_checksum_errors = 0U;
volatile ULONG g_net_debug_arp_invalid_messages = 0U;
volatile ULONG g_net_debug_edmac_eesr = 0U;
volatile ULONG g_net_debug_etherc_ecsr = 0U;
volatile ULONG g_net_debug_etherc_psr = 0U;
volatile ULONG g_net_debug_edmac0_eesr = 0U;
volatile ULONG g_net_debug_edmac1_eesr = 0U;
volatile ULONG g_net_debug_etherc0_ecsr = 0U;
volatile ULONG g_net_debug_etherc1_ecsr = 0U;
volatile ULONG g_net_debug_etherc0_psr = 0U;
volatile ULONG g_net_debug_etherc1_psr = 0U;
volatile ULONG g_net_debug_driver_irq = 0U;
volatile ULONG g_net_debug_driver_irq_enabled = 0U;
volatile ULONG g_net_debug_driver_state = 0U;
volatile ULONG g_net_debug_driver_link_established = 0U;
volatile ULONG g_net_debug_driver_irq_count = 0U;
volatile ULONG g_net_debug_driver_tx_irq_count = 0U;
volatile ULONG g_net_debug_driver_rx_irq_count = 0U;
volatile ULONG g_net_debug_driver_eci_irq_count = 0U;
volatile ULONG g_net_debug_driver_error_irq_count = 0U;
volatile ULONG g_net_debug_driver_error_bits = 0U;
volatile ULONG g_net_debug_driver_last_eesr = 0U;
volatile ULONG g_net_debug_driver_last_ecsr = 0U;
volatile ULONG g_net_debug_driver_rx_bd_index = 0U;
volatile ULONG g_net_debug_driver_rx_bd0_status = 0U;
volatile ULONG g_net_debug_driver_edrrr = 0U;
volatile ULONG g_net_debug_driver_edmr = 0U;
volatile ULONG g_net_debug_driver_ecmr = 0U;
volatile ULONG g_net_debug_driver_tx_bd0_status = 0U;
volatile ULONG g_net_debug_driver_edtrr = 0U;
volatile ULONG g_net_debug_gratuitous_arp_status = 0U;
volatile ULONG g_net_debug_pmisc_pfenet = 0U;
volatile ULONG g_net_debug_phy_control = 0U;
volatile ULONG g_net_debug_phy_status = 0U;
volatile ULONG g_net_debug_phy_id1 = 0U;
volatile ULONG g_net_debug_phy_id2 = 0U;
volatile ULONG g_net_debug_phy_advertise = 0U;
volatile ULONG g_net_debug_phy_partner = 0U;
volatile ULONG g_net_debug_phy_rxer_counter = 0U;
volatile ULONG g_net_debug_phy_opmode = 0U;
volatile ULONG g_net_debug_phy_opmode_status = 0U;
volatile ULONG g_net_debug_phy_interrupt_status = 0U;
volatile ULONG g_net_debug_phy_control1 = 0U;
volatile ULONG g_net_debug_phy_control2 = 0U;
volatile ULONG g_net_debug_loopback_stage = 0U;
volatile ULONG g_net_debug_loopback_saved_control = 0U;
volatile ULONG g_net_debug_loopback_send_status = 0xFFFFFFFFU;
volatile ULONG g_net_debug_loopback_rx_irq_before = 0U;
volatile ULONG g_net_debug_loopback_rx_irq_after = 0U;
volatile ULONG g_net_debug_loopback_tick = 0U;
volatile ULONG g_net_debug_linkmd_stage = 0U;
volatile ULONG g_net_debug_linkmd_status = 0U;
volatile ULONG g_net_debug_linkmd_result = 0U;
volatile ULONG g_net_debug_linkmd_distance = 0U;
volatile ULONG g_net_debug_http_stage = 0U;
volatile ULONG g_net_debug_http_entry_count = 0U;
volatile ULONG g_net_debug_http_exit_count = 0U;
volatile ULONG g_net_debug_http_inflight = 0U;
volatile ULONG g_net_debug_packet_pool_available = 0U;
volatile ULONG g_net_debug_packet_pool_empty_requests = 0U;
volatile ULONG g_net_debug_packet_pool_empty_suspensions = 0U;
volatile char  g_net_debug_last_path[64] = {0};

#define HTML_BUFFER_SIZE        16384
#define RESOURCE_BUFFER_SIZE     128
#define QUERY_BUFFER_SIZE       2048
#define IMPORT_BUFFER_SIZE     24576
#define FORM_CARDS_BUFFER_SIZE   256
#define FORM_PHOTO_BUFFER_SIZE    64
#define ROWS_BUFFER_SIZE         8192
#define PROFILES_PAGE_SIZE          8
#define PHOTO_OPTIONS_BUFFER_SIZE 2048
#define PROFILE_OPTIONS_BUFFER_SIZE 2048
#define UPLOAD_IMAGE_DIM        160U
#define UPLOAD_TILE_DIM         40U
#define UPLOAD_TILE_COUNT_X     (UPLOAD_IMAGE_DIM / UPLOAD_TILE_DIM)
#define UPLOAD_TILE_COUNT_Y     (UPLOAD_IMAGE_DIM / UPLOAD_TILE_DIM)
#define UPLOAD_TILE_COUNT       (UPLOAD_TILE_COUNT_X * UPLOAD_TILE_COUNT_Y)
#define UPLOAD_TILE_PIXEL_BYTES (UPLOAD_TILE_DIM * UPLOAD_TILE_DIM * 2U)
#define UPLOAD_TILE_B64_BUFFER_SIZE ((UPLOAD_TILE_PIXEL_BYTES * 4U / 3U) + 128U)
#define HTTP_SEND_CHUNK_SIZE      512U
#define NET_HTTP_PACKET_WAIT_TICKS (TX_TIMER_TICKS_PER_SECOND / 4U)
#define DEVICE_IP_ADDR           IP_ADDRESS(192, 168, 15, 180)
#define DEVICE_NETMASK           IP_ADDRESS(255, 255, 255, 0)
#define DEVICE_GATEWAY_ADDR      IP_ADDRESS(192, 168, 15, 1)
#define WEB_ADMIN_PIN           "1234"
#define WEB_ADMIN_SESSION_TICKS (10U * 60U * TX_TIMER_TICKS_PER_SECOND)

static char g_net_flash_message[512] = {0};
static ULONG g_net_admin_session_deadline = 0U;
static bool g_net_action_mutex_ready = false;
static TX_MUTEX g_net_action_mutex;
static bool g_net_http_mutex_ready = false;
static TX_MUTEX g_net_http_mutex;
static bool g_net_import_pending = false;
static char g_net_pending_import_json[IMPORT_BUFFER_SIZE];
static char g_net_pending_import_work_json[IMPORT_BUFFER_SIZE];
static bool g_net_upload_session_active = false;
static int g_net_pending_upload_profile_index = -1;
static int g_net_pending_upload_width = (int) UPLOAD_IMAGE_DIM;
static int g_net_pending_upload_height = (int) UPLOAD_IMAGE_DIM;
static uint32_t g_net_pending_upload_tile_mask = 0U;
static char g_net_pending_upload_photo_id[STORAGE_PHOTO_ID_MAX_LEN];

static UINT send_html_response(NX_HTTP_SERVER *server, NX_PACKET *packet_ptr, const char *html);
static UINT send_plain_response(NX_HTTP_SERVER *server, NX_PACKET *packet_ptr, const char *text);
static UINT send_javascript_response(NX_HTTP_SERVER *server, NX_PACKET *packet_ptr, const char *script);
static UINT send_plain_response_fresh(NX_HTTP_SERVER *server, const char *text);
static UINT send_plain_status_response(NX_HTTP_SERVER *server, NX_PACKET *packet_ptr, const char *status_code, const char *text);
static UINT send_callback_response(NX_HTTP_SERVER *server, const char *status_code, const char *information);
static UINT send_redirect(NX_HTTP_SERVER *server, const char *location);

static void net_set_flash_message(const char *message)
{
    if (NULL == message)
    {
        g_net_flash_message[0] = '\0';
        return;
    }

    strncpy(g_net_flash_message, message, sizeof(g_net_flash_message) - 1U);
    g_net_flash_message[sizeof(g_net_flash_message) - 1U] = '\0';
}

static void net_action_init_once(void)
{
    if (!g_net_action_mutex_ready)
    {
        if (TX_SUCCESS == tx_mutex_create(&g_net_action_mutex, "net_action_mutex", TX_NO_INHERIT))
        {
            g_net_action_mutex_ready = true;
        }
    }
}

static void net_http_init_once(void)
{
    if (!g_net_http_mutex_ready)
    {
        if (TX_SUCCESS == tx_mutex_create(&g_net_http_mutex, "net_http_mutex", TX_NO_INHERIT))
        {
            g_net_http_mutex_ready = true;
        }
    }
}

static void net_action_lock(void)
{
    net_action_init_once();
    if (g_net_action_mutex_ready)
    {
        tx_mutex_get(&g_net_action_mutex, TX_WAIT_FOREVER);
    }
}

static void net_action_unlock(void)
{
    if (g_net_action_mutex_ready)
    {
        tx_mutex_put(&g_net_action_mutex);
    }
}

static void net_http_lock(void)
{
    net_http_init_once();
    if (g_net_http_mutex_ready)
    {
        tx_mutex_get(&g_net_http_mutex, TX_WAIT_FOREVER);
    }
}

static void net_http_unlock(void)
{
    if (g_net_http_mutex_ready)
    {
        tx_mutex_put(&g_net_http_mutex);
    }
}

static bool net_admin_is_authenticated(void)
{
    return (tx_time_get() < g_net_admin_session_deadline);
}

static void net_admin_begin_session(void)
{
    g_net_admin_session_deadline = tx_time_get() + WEB_ADMIN_SESSION_TICKS;
}

static void net_admin_end_session(void)
{
    g_net_admin_session_deadline = 0U;
}

static bool net_photo_asset_exists(const char *photo_id)
{
    if ((NULL == photo_id) || ('\0' == photo_id[0]))
    {
        return false;
    }

    if (ui_has_uploaded_photo(photo_id))
    {
        return true;
    }

    for (size_t i = 0U; i < PROFILE_PHOTO_ID_COUNT; i++)
    {
        if (0 == strcmp(photo_id, g_profile_photo_ids[i]))
        {
            return true;
        }
    }

    return false;
}

static void net_build_photo_options(char *out, size_t out_size)
{
    size_t offset = 0U;

    if ((NULL == out) || (0U == out_size))
    {
        return;
    }

    out[0] = '\0';

    for (size_t i = 0U; i < PROFILE_PHOTO_ID_COUNT; i++)
    {
        int written = snprintf(&out[offset],
                               out_size - offset,
                               "<option value='%s'>%s</option>",
                               g_profile_photo_ids[i],
                               g_profile_photo_ids[i]);

        if ((written < 0) || ((size_t) written >= (out_size - offset)))
        {
            break;
        }

        offset += (size_t) written;
    }

}

static void net_build_profile_options(const storage_user_profile_t *profiles, int count, char *out, size_t out_size)
{
    size_t offset = 0U;

    if ((NULL == profiles) || (NULL == out) || (0U == out_size) || (count <= 0))
    {
        if ((NULL != out) && (out_size > 0U))
        {
            out[0] = '\0';
        }
        return;
    }

    out[0] = '\0';

    for (int i = 0; i < count; i++)
    {
        int written;

        written = snprintf(&out[offset],
                           out_size - offset,
                           "<option value='%d'>%s</option>",
                           i,
                           profiles[i].name);

        if ((written < 0) || ((size_t) written >= (out_size - offset)))
        {
            break;
        }

        offset += (size_t) written;
    }
}

static void network_stack_init_once(void)
{
    static bool initialized = false;

    if (initialized)
    {
        return;
    }

    g_net_debug_state = 1U;
    net_action_init_once();
    net_http_init_once();
    packet_pool_init0();
    ip_init0();
    http_server_init0();
    g_net_debug_state = 2U;
    initialized = true;
}

static void ip_to_string(ULONG ip_address, char *out, size_t out_size)
{
    snprintf(out, out_size, "%lu.%lu.%lu.%lu",
             (unsigned long) ((ip_address >> 24) & 0xFFUL),
             (unsigned long) ((ip_address >> 16) & 0xFFUL),
             (unsigned long) ((ip_address >> 8) & 0xFFUL),
             (unsigned long) (ip_address & 0xFFUL));
}

static void url_decode(char *text)
{
    char *src = text;
    char *dst = text;

    while ((NULL != src) && ('\0' != *src))
    {
        if ('+' == *src)
        {
            *dst++ = ' ';
            src++;
        }
        else if (('%' == *src) &&
                 (('\0' != src[1]) && ('\0' != src[2])))
        {
            char hex[3];
            char *end_ptr = NULL;
            long value;

            hex[0] = src[1];
            hex[1] = src[2];
            hex[2] = '\0';
            value = strtol(hex, &end_ptr, 16);

            if ((NULL != end_ptr) && ('\0' == *end_ptr))
            {
                *dst++ = (char) value;
                src += 3;
            }
            else
            {
                *dst++ = *src++;
            }
        }
        else
        {
            *dst++ = *src++;
        }
    }

    *dst = '\0';
}

static void sanitize_text(char *text)
{
    size_t write_index = 0U;

    for (size_t read_index = 0U; text[read_index] != '\0'; read_index++)
    {
        char ch = text[read_index];

        if ((ch == '"') || (ch == '\\') || (ch == '\r') || (ch == '\n') ||
            (ch == '<') || (ch == '>'))
        {
            continue;
        }

        text[write_index++] = ch;
    }

    text[write_index] = '\0';
}

static bool query_get_value(const char *query, const char *key, char *out, size_t out_size)
{
    size_t key_len = strlen(key);
    const char *cursor = query;

    if ((NULL == query) || (NULL == key) || (NULL == out) || (0U == out_size))
    {
        return false;
    }

    while ((NULL != cursor) && ('\0' != *cursor))
    {
        const char *next = strchr(cursor, '&');
        size_t token_len = (NULL == next) ? strlen(cursor) : (size_t) (next - cursor);

        if ((token_len > key_len) &&
            (0 == strncmp(cursor, key, key_len)) &&
            ('=' == cursor[key_len]))
        {
            size_t value_len = token_len - key_len - 1U;

            if (value_len >= out_size)
            {
                value_len = out_size - 1U;
            }

            memcpy(out, cursor + key_len + 1U, value_len);
            out[value_len] = '\0';
            url_decode(out);
            sanitize_text(out);
            return true;
        }

        cursor = (NULL == next) ? NULL : (next + 1);
    }

    return false;
}

static bool query_get_value_raw(const char *query, const char *key, char *out, size_t out_size)
{
    size_t key_len = strlen(key);
    const char *cursor = query;

    if ((NULL == query) || (NULL == key) || (NULL == out) || (0U == out_size))
    {
        return false;
    }

    while ((NULL != cursor) && ('\0' != *cursor))
    {
        const char *next = strchr(cursor, '&');
        size_t token_len = (NULL == next) ? strlen(cursor) : (size_t) (next - cursor);

        if ((token_len > key_len) &&
            (0 == strncmp(cursor, key, key_len)) &&
            ('=' == cursor[key_len]))
        {
            size_t value_len = token_len - key_len - 1U;

            if (value_len >= out_size)
            {
                value_len = out_size - 1U;
            }

            memcpy(out, cursor + key_len + 1U, value_len);
            out[value_len] = '\0';
            url_decode(out);
            return true;
        }

        cursor = (NULL == next) ? NULL : (next + 1);
    }

    return false;
}

static int query_get_int(const char *query, const char *key, int default_value)
{
    char value_text[16];
    char *end_ptr = NULL;
    long value;

    if (!query_get_value(query, key, value_text, sizeof(value_text)))
    {
        return default_value;
    }

    value = strtol(value_text, &end_ptr, 10);
    if ((NULL == end_ptr) || ('\0' != *end_ptr))
    {
        return default_value;
    }

    return (int) value;
}

static int net_base64_value(char ch)
{
    if ((ch >= 'A') && (ch <= 'Z')) return (int) (ch - 'A');
    if ((ch >= 'a') && (ch <= 'z')) return (int) (ch - 'a' + 26);
    if ((ch >= '0') && (ch <= '9')) return (int) (ch - '0' + 52);
    if ('+' == ch) return 62;
    if ('-' == ch) return 62;
    if ('/' == ch) return 63;
    if ('_' == ch) return 63;
    if ('=' == ch) return -2;
    return -1;
}

static size_t net_base64_decode(const char *input, uint8_t *out, size_t out_size)
{
    int quartet[4];
    int quartet_index = 0;
    size_t out_index = 0U;

    if ((NULL == input) || (NULL == out) || (0U == out_size))
    {
        return 0U;
    }

    while ('\0' != *input)
    {
        int value = net_base64_value(*input++);

        if (value < 0)
        {
            if (-2 == value)
            {
                quartet[quartet_index++] = -2;
            }
            else
            {
                continue;
            }
        }
        else
        {
            quartet[quartet_index++] = value;
        }

        if (quartet_index == 4)
        {
            if ((out_index + 1U) > out_size)
            {
                break;
            }
            out[out_index++] = (uint8_t) ((quartet[0] << 2) | (quartet[1] >> 4));

            if ((quartet[2] != -2) && ((out_index + 1U) <= out_size))
            {
                out[out_index++] = (uint8_t) (((quartet[1] & 0x0F) << 4) | (quartet[2] >> 2));
            }

            if ((quartet[2] != -2) && (quartet[3] != -2) && ((out_index + 1U) <= out_size))
            {
                out[out_index++] = (uint8_t) (((quartet[2] & 0x03) << 6) | quartet[3]);
            }

            quartet_index = 0;
        }
    }

    return out_index;
}

static const char *net_event_type_text(app_event_type_t type)
{
    switch (type)
    {
        case EVENT_RFID_AUTH_OK: return "Acesso liberado";
        case EVENT_RFID_AUTH_FAIL: return "Acesso negado";
        case EVENT_DOOR_OPEN: return "Porta aberta";
        case EVENT_DOOR_CLOSE: return "Porta fechada";
        case EVENT_CARD_REGISTERED: return "Cartao cadastrado";
        case EVENT_CARD_ALREADY_REGISTERED: return "Cartao ja cadastrado";
        case EVENT_CARD_REGISTRATION_FAILED: return "Falha no cadastro";
        default: return "Evento";
    }
}

static void net_format_tick_timestamp(ULONG tick, char *out, size_t out_size)
{
    ULONG total_seconds;
    ULONG hours;
    ULONG minutes;
    ULONG seconds;

    if ((NULL == out) || (0U == out_size))
    {
        return;
    }

    total_seconds = tick / TX_TIMER_TICKS_PER_SECOND;
    hours = (total_seconds / 3600U) % 24U;
    minutes = (total_seconds % 3600U) / 60U;
    seconds = total_seconds % 60U;

    snprintf(out,
             out_size,
             "2026-03-19:%02luh%02lumin%02lus",
             (unsigned long) hours,
             (unsigned long) minutes,
             (unsigned long) seconds);
}

static const char *json_skip_whitespace(const char *cursor, const char *limit)
{
    while ((NULL != cursor) && (cursor < limit) &&
           ((' ' == *cursor) || ('\t' == *cursor) || ('\r' == *cursor) || ('\n' == *cursor)))
    {
        cursor++;
    }

    return cursor;
}

static const char *json_find_key(const char *object_start, const char *object_end, const char *key)
{
    char pattern[48];
    const char *cursor;

    if ((NULL == object_start) || (NULL == object_end) || (NULL == key))
    {
        return NULL;
    }

    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    cursor = object_start;

    while ((NULL != cursor) && (cursor < object_end))
    {
        cursor = strstr(cursor, pattern);
        if ((NULL == cursor) || (cursor >= object_end))
        {
            return NULL;
        }

        return cursor + strlen(pattern);
    }

    return NULL;
}

static void profile_cards_to_csv(const storage_user_profile_t *profile, char *out, size_t out_size)
{
    size_t offset = 0U;

    if ((NULL == out) || (0U == out_size))
    {
        return;
    }

    out[0] = '\0';
    if (NULL == profile)
    {
        return;
    }

    for (unsigned int i = 0U; i < profile->card_count; i++)
    {
        int written = snprintf(&out[offset],
                               out_size - offset,
                               "%s%s",
                               (i > 0U) ? "," : "",
                               profile->cards[i]);
        if ((written < 0) || ((size_t) written >= (out_size - offset)))
        {
            break;
        }
        offset += (size_t) written;
    }
}

static bool profile_from_query(const char *query, storage_user_profile_t *profile, int *edit_index)
{
    char cards_text[FORM_CARDS_BUFFER_SIZE];
    const char *cursor;

    if ((NULL == query) || (NULL == profile))
    {
        return false;
    }

    memset(profile, 0, sizeof(*profile));

    if (NULL != edit_index)
    {
        *edit_index = query_get_int(query, "edit", -1);
    }

    if (!query_get_value(query, "name", profile->name, sizeof(profile->name)))
    {
        return false;
    }

    (void) query_get_value(query, "role", profile->role, sizeof(profile->role));
    (void) query_get_value(query, "chapter", profile->chapter, sizeof(profile->chapter));
    (void) query_get_value(query, "photo_id", profile->photo_id, sizeof(profile->photo_id));

    if (!query_get_value(query, "cards", cards_text, sizeof(cards_text)))
    {
        cards_text[0] = '\0';
    }

    cursor = cards_text;
    while (('\0' != *cursor) && (profile->card_count < STORAGE_MAX_CARDS_PER_USER))
    {
        const char *next = strchr(cursor, ',');
        size_t token_len = (NULL == next) ? strlen(cursor) : (size_t) (next - cursor);
        size_t copy_len = 0U;
        size_t write_len = 0U;

        while ((copy_len < token_len) &&
               (write_len + 1U < sizeof(profile->cards[profile->card_count])) &&
               ('\0' != cursor[copy_len]))
        {
            char ch = cursor[copy_len];

            if ((ch != ' ') && (ch != '\t') && (ch != '\r') && (ch != '\n') &&
                (ch != ';'))
            {
                profile->cards[profile->card_count][write_len++] = ch;
            }
            copy_len++;
        }

        profile->cards[profile->card_count][write_len] = '\0';

        if ('\0' != profile->cards[profile->card_count][0])
        {
            profile->card_count++;
        }

        cursor = (NULL == next) ? (cursor + strlen(cursor)) : (next + 1);
    }

    return ('\0' != profile->name[0]);
}

static bool import_extract_json_string(const char *object_start,
                                       const char *object_end,
                                       const char *key,
                                       char *out,
                                       size_t out_size)
{
    const char *found;
    const char *colon;
    const char *start;
    const char *end;

    if ((NULL == object_start) || (NULL == object_end) || (NULL == key) || (NULL == out) || (0U == out_size))
    {
        return false;
    }

    found = json_find_key(object_start, object_end, key);
    if (NULL == found)
    {
        out[0] = '\0';
        return false;
    }

    colon = json_skip_whitespace(found, object_end);
    if ((NULL == colon) || (colon >= object_end) || (':' != *colon))
    {
        out[0] = '\0';
        return false;
    }

    start = json_skip_whitespace(colon + 1, object_end);
    if ((NULL == start) || (start >= object_end) || ('"' != *start))
    {
        out[0] = '\0';
        return false;
    }

    start++;
    end = strchr(start, '"');
    if ((NULL == end) || (end > object_end))
    {
        out[0] = '\0';
        return false;
    }

    {
        size_t copy_len = (size_t) (end - start);
        if (copy_len >= out_size)
        {
            copy_len = out_size - 1U;
        }
        memcpy(out, start, copy_len);
        out[copy_len] = '\0';
    }

    return true;
}

static void import_extract_json_cards(const char *object_start,
                                      const char *object_end,
                                      storage_user_profile_t *profile)
{
    const char *found;
    const char *colon;
    const char *cursor;

    if ((NULL == object_start) || (NULL == object_end) || (NULL == profile))
    {
        return;
    }

    found = json_find_key(object_start, object_end, "cards");
    if (NULL == found)
    {
        char legacy_uid[UID_MAX_LEN];

        if (import_extract_json_string(object_start, object_end, "uid", legacy_uid, sizeof(legacy_uid)))
        {
            strncpy(profile->cards[0], legacy_uid, sizeof(profile->cards[0]) - 1U);
            profile->cards[0][sizeof(profile->cards[0]) - 1U] = '\0';
            if ('\0' != profile->cards[0][0])
            {
                profile->card_count = 1U;
            }
        }
        return;
    }

    colon = json_skip_whitespace(found, object_end);
    if ((NULL == colon) || (colon >= object_end) || (':' != *colon))
    {
        return;
    }

    cursor = json_skip_whitespace(colon + 1, object_end);
    if ((NULL == cursor) || (cursor >= object_end) || ('[' != *cursor))
    {
        return;
    }

    cursor++;
    while (('\0' != *cursor) && (']' != *cursor) && (profile->card_count < STORAGE_MAX_CARDS_PER_USER))
    {
        const char *start = strchr(cursor, '"');
        const char *end;
        size_t copy_len;

        if ((NULL == start) || (start > object_end) || (']' == *start))
        {
            break;
        }

        end = strchr(start + 1, '"');
        if ((NULL == end) || (end > object_end))
        {
            break;
        }

        copy_len = (size_t) (end - (start + 1));
        if (copy_len >= sizeof(profile->cards[profile->card_count]))
        {
            copy_len = sizeof(profile->cards[profile->card_count]) - 1U;
        }

        memcpy(profile->cards[profile->card_count], start + 1, copy_len);
        profile->cards[profile->card_count][copy_len] = '\0';

        if ('\0' != profile->cards[profile->card_count][0])
        {
            profile->card_count++;
        }

        cursor = end + 1;
    }
}

static int import_find_existing_profile_index(const storage_user_profile_t *profile)
{
    int count;

    if ((NULL == profile) || ('\0' == profile->name[0]))
    {
        return -1;
    }

    count = storage_user_count();
    for (int index = 0; index < count; index++)
    {
        storage_user_profile_t current;

        if (!storage_profile_get(index, &current))
        {
            continue;
        }

        if (('\0' != profile->photo_id[0]) &&
            ('\0' != current.photo_id[0]) &&
            (0 == strcmp(profile->photo_id, current.photo_id)))
        {
            return index;
        }

        if ((0 == strcmp(profile->name, current.name)) &&
            (0 == strcmp(profile->chapter, current.chapter)))
        {
            return index;
        }
    }

    return -1;
}

static int import_profiles_from_json(const char *json_text)
{
    const char *cursor = json_text;
    int imported = 0;

    if ((NULL == json_text) || ('\0' == json_text[0]))
    {
        return 0;
    }

    while ((NULL != cursor) && ('\0' != *cursor))
    {
        const char *name_key = strstr(cursor, "\"name\"");
        const char *object_start;
        const char *object_end;
        storage_user_profile_t profile;
        int edit_index;

        if (NULL == name_key)
        {
            break;
        }

        object_start = name_key;
        while ((object_start > json_text) && ('{' != *object_start))
        {
            object_start--;
        }
        if ('{' != *object_start)
        {
            break;
        }

        object_end = strchr(name_key, '}');
        if (NULL == object_end)
        {
            break;
        }

        memset(&profile, 0, sizeof(profile));
        if (!import_extract_json_string(object_start, object_end, "name", profile.name, sizeof(profile.name)))
        {
            cursor = object_end + 1;
            continue;
        }

        (void) import_extract_json_string(object_start, object_end, "role", profile.role, sizeof(profile.role));
        (void) import_extract_json_string(object_start, object_end, "chapter", profile.chapter, sizeof(profile.chapter));
        (void) import_extract_json_string(object_start, object_end, "photo_id", profile.photo_id, sizeof(profile.photo_id));
        import_extract_json_cards(object_start, object_end, &profile);

        edit_index = import_find_existing_profile_index(&profile);
        if (storage_profile_upsert(&profile, edit_index))
        {
            imported++;
        }

        cursor = object_end + 1;
    }

    return imported;
}

static bool net_queue_import_profiles(const char *form_data)
{
    bool queued = false;

    if (NULL == form_data)
    {
        return false;
    }

    net_action_lock();
    if (!g_net_import_pending &&
        query_get_value_raw(form_data, "import_json", g_net_pending_import_json, sizeof(g_net_pending_import_json)) &&
        ('\0' != g_net_pending_import_json[0]))
    {
        g_net_import_pending = true;
        queued = true;
    }
    net_action_unlock();

    return queued;
}

static void net_reset_upload_session(void)
{
    g_net_upload_session_active = false;
    g_net_pending_upload_profile_index = -1;
    g_net_pending_upload_width = (int) UPLOAD_IMAGE_DIM;
    g_net_pending_upload_height = (int) UPLOAD_IMAGE_DIM;
    g_net_pending_upload_tile_mask = 0U;
    g_net_pending_upload_photo_id[0] = '\0';
}

static UINT handle_light_upload_begin(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, const char *query)
{
    int profile_index;
    int width;
    int height;
    storage_user_profile_t profile;

    if (!net_admin_is_authenticated())
    {
        return send_plain_status_response(server_ptr, packet_ptr, NX_HTTP_STATUS_UNAUTHORIZED, "ERR_AUTH");
    }

    profile_index = query_get_int(query, "profile_index", -1);
    width = query_get_int(query, "width", (int) UPLOAD_IMAGE_DIM);
    height = query_get_int(query, "height", (int) UPLOAD_IMAGE_DIM);

    if ((profile_index < 0) || !storage_profile_get(profile_index, &profile))
    {
        return send_plain_status_response(server_ptr, packet_ptr, NX_HTTP_STATUS_BAD_REQUEST, "ERR_PROFILE");
    }

    if ((width <= 0) || (height <= 0) ||
        (width > (int) UPLOAD_IMAGE_DIM) || (height > (int) UPLOAD_IMAGE_DIM))
    {
        return send_plain_status_response(server_ptr, packet_ptr, NX_HTTP_STATUS_BAD_REQUEST, "ERR_DIM");
    }

    net_action_lock();
    if (g_net_upload_session_active)
    {
        net_action_unlock();
        return send_plain_status_response(server_ptr, packet_ptr, NX_HTTP_STATUS_CONFLICT, "ERR_BUSY");
    }

    g_net_upload_session_active = true;
    g_net_pending_upload_profile_index = profile_index;
    g_net_pending_upload_width = width;
    g_net_pending_upload_height = height;
    g_net_pending_upload_tile_mask = 0U;
    snprintf(g_net_pending_upload_photo_id, sizeof(g_net_pending_upload_photo_id), "upload_profile_%d", profile_index);
    if (!ui_prepare_uploaded_photo_rgb565(g_net_pending_upload_photo_id, (uint16_t) width, (uint16_t) height))
    {
        net_reset_upload_session();
        net_action_unlock();
        return send_plain_status_response(server_ptr, packet_ptr, NX_HTTP_STATUS_BAD_REQUEST, "ERR_STORE");
    }
    net_action_unlock();

    return send_plain_status_response(server_ptr, packet_ptr, NX_HTTP_STATUS_OK, "OK");
}

static UINT handle_light_upload_chunk(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, const char *query)
{
    static char tile_b64[UPLOAD_TILE_B64_BUFFER_SIZE];
    static uint8_t raw_tile[UPLOAD_TILE_PIXEL_BYTES];
    size_t raw_size;
    int tile_index;
    int tile_x;
    int tile_y;

    if (!net_admin_is_authenticated())
    {
        return send_plain_status_response(server_ptr, packet_ptr, NX_HTTP_STATUS_UNAUTHORIZED, "ERR_AUTH");
    }

    tile_index = query_get_int(query, "tile", -1);
    if ((tile_index < 0) || (tile_index >= (int) UPLOAD_TILE_COUNT))
    {
        return send_plain_status_response(server_ptr, packet_ptr, NX_HTTP_STATUS_BAD_REQUEST, "ERR_TILE");
    }

    if (!query_get_value_raw(query, "data", tile_b64, sizeof(tile_b64)))
    {
        return send_plain_status_response(server_ptr, packet_ptr, NX_HTTP_STATUS_BAD_REQUEST, "ERR_DATA");
    }

    raw_size = net_base64_decode(tile_b64, raw_tile, sizeof(raw_tile));
    if (raw_size != (size_t) UPLOAD_TILE_PIXEL_BYTES)
    {
        return send_plain_status_response(server_ptr, packet_ptr, NX_HTTP_STATUS_BAD_REQUEST, "ERR_DECODE");
    }

    tile_x = (tile_index % (int) UPLOAD_TILE_COUNT_X) * (int) UPLOAD_TILE_DIM;
    tile_y = (tile_index / (int) UPLOAD_TILE_COUNT_X) * (int) UPLOAD_TILE_DIM;

    net_action_lock();
    if (!g_net_upload_session_active)
    {
        net_action_unlock();
        return send_plain_status_response(server_ptr, packet_ptr, NX_HTTP_STATUS_CONFLICT, "ERR_SESSION");
    }

    {
        static uint16_t tile_pixels[UPLOAD_TILE_DIM * UPLOAD_TILE_DIM];

        for (int y = 0; y < (int) UPLOAD_TILE_DIM; y++)
        {
            for (int x = 0; x < (int) UPLOAD_TILE_DIM; x++)
            {
                size_t src = (size_t) (y * (int) UPLOAD_TILE_DIM + x) * 2U;
                tile_pixels[(y * (int) UPLOAD_TILE_DIM) + x] =
                    (uint16_t) (((uint16_t) raw_tile[src] << 8) | (uint16_t) raw_tile[src + 1U]);
            }
        }

        if (!ui_write_uploaded_photo_tile_rgb565(g_net_pending_upload_photo_id,
                                                 tile_pixels,
                                                 (uint16_t) tile_x,
                                                 (uint16_t) tile_y,
                                                 (uint16_t) UPLOAD_TILE_DIM,
                                                 (uint16_t) UPLOAD_TILE_DIM))
        {
            net_reset_upload_session();
            net_action_unlock();
            return send_plain_status_response(server_ptr, packet_ptr, NX_HTTP_STATUS_BAD_REQUEST, "ERR_STORE");
        }
    }

    g_net_pending_upload_tile_mask |= (1UL << tile_index);
    net_action_unlock();

    return send_plain_status_response(server_ptr, packet_ptr, NX_HTTP_STATUS_OK, "OK");
}

static UINT handle_light_upload_commit(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr)
{
    storage_user_profile_t profile;
    int profile_index = -1;
    bool photo_persist_requested = false;
    bool profile_persist_requested = false;

    if (!net_admin_is_authenticated())
    {
        return send_plain_status_response(server_ptr, packet_ptr, NX_HTTP_STATUS_UNAUTHORIZED, "ERR_AUTH");
    }

    net_action_lock();
    if (!g_net_upload_session_active ||
        (g_net_pending_upload_tile_mask != ((1UL << UPLOAD_TILE_COUNT) - 1UL)))
    {
        net_reset_upload_session();
        net_action_unlock();
        return send_plain_status_response(server_ptr, packet_ptr, NX_HTTP_STATUS_BAD_REQUEST, "ERR_INCOMPLETE");
    }

    profile_index = g_net_pending_upload_profile_index;
    g_net_upload_session_active = false;
    net_action_unlock();

    if ((profile_index < 0) || !storage_profile_get(profile_index, &profile))
    {
        net_action_lock();
        net_reset_upload_session();
        net_action_unlock();
        net_set_flash_message("<div class='card warn'>Upload de foto falhou: perfil invalido.</div>");
        return send_plain_status_response(server_ptr, packet_ptr, NX_HTTP_STATUS_BAD_REQUEST, "ERR_PROFILE");
    }

    strncpy(profile.photo_id, g_net_pending_upload_photo_id, sizeof(profile.photo_id) - 1U);
    profile.photo_id[sizeof(profile.photo_id) - 1U] = '\0';
    if (!storage_profile_upsert(&profile, profile_index))
    {
        net_action_lock();
        net_reset_upload_session();
        net_action_unlock();
        net_set_flash_message("<div class='card warn'>A foto foi recebida, mas o perfil nao pode ser atualizado.</div>");
        return send_plain_status_response(server_ptr, packet_ptr, NX_HTTP_STATUS_BAD_REQUEST, "ERR_UPSERT");
    }

    ui_invalidate_profile_cache();
    photo_persist_requested = storage_photo_persist_now(profile.photo_id);
    profile_persist_requested = storage_persist_now();
    net_set_flash_message((photo_persist_requested && profile_persist_requested)
                              ? "<div class='card ok'>Foto HD enviada, vinculada ao perfil e persistencia automatica solicitada.</div>"
                              : "<div class='card warn'>Foto HD enviada em runtime, mas a persistencia automatica nao confirmou a solicitacao completa.</div>");

    net_action_lock();
    net_reset_upload_session();
    net_action_unlock();

    return send_plain_status_response(server_ptr, packet_ptr, NX_HTTP_STATUS_OK, "OK");
}

static void net_process_pending_actions(void)
{
    bool import_pending = false;
    int imported_count = 0;
    bool persist_requested = false;

    net_action_lock();
    if (g_net_import_pending)
    {
        strncpy(g_net_pending_import_work_json,
                g_net_pending_import_json,
                sizeof(g_net_pending_import_work_json) - 1U);
        g_net_pending_import_work_json[sizeof(g_net_pending_import_work_json) - 1U] = '\0';
        g_net_pending_import_json[0] = '\0';
        g_net_import_pending = false;
        import_pending = true;
    }
    net_action_unlock();

    if (import_pending)
    {
        imported_count = import_profiles_from_json(g_net_pending_import_work_json);
        if (imported_count > 0)
        {
            persist_requested = storage_persist_now();
            net_set_flash_message(persist_requested
                                      ? "<div class='card ok'>Perfis importados e persistencia automatica solicitada.</div>"
                                      : "<div class='card ok'>Perfis importados em runtime. Se quiser, use \"Persistir cadastros\" como fallback.</div>");
        }
        else
        {
            net_set_flash_message("<div class='card warn'>Nenhum perfil foi importado. Use o JSON simples gerado em script/firebase_bundle/profiles_import.json.</div>");
        }
        g_net_pending_import_work_json[0] = '\0';
    }
}

static void extract_query_from_packet(NX_PACKET *packet_ptr, char *query_out, size_t query_size)
{
    query_out[0] = '\0';
    if ((NULL == packet_ptr) || (NULL == packet_ptr->nx_packet_prepend_ptr))
    {
        return;
    }

    /* Acede ao buffer bruto do pacote HTTP para não depender da framework do NetX */
    char *req = (char *)packet_ptr->nx_packet_prepend_ptr;
    char *req_end = (char *)packet_ptr->nx_packet_append_ptr;

    char *http_ver = strstr(req, " HTTP/");
    if ((NULL == http_ver) || (http_ver > req_end))
    {
        return;
    }

    char *q_mark = strchr(req, '?');
    if ((NULL != q_mark) && (q_mark < http_ver))
    {
        size_t len = (size_t)(http_ver - (q_mark + 1));
        if (len >= query_size)
        {
            len = query_size - 1U;
        }
        memcpy(query_out, q_mark + 1, len);
        query_out[len] = '\0';
    }
}

static UINT send_html_response(NX_HTTP_SERVER *server, NX_PACKET *packet_ptr, const char *html)
{
    UINT status;

    if (NULL == html)
    {
        g_net_debug_last_response_status = NX_PTR_ERROR;
        return NX_PTR_ERROR;
    }

    status = nx_http_server_callback_generate_response_header(server,
                                                              &packet_ptr,
                                                              NX_HTTP_STATUS_OK,
                                                              strlen(html),
                                                              "text/html",
                                                              NULL);
    if (NX_SUCCESS != status)
    {
        g_net_debug_last_response_status = status;
        return status;
    }

    status = nx_packet_data_append(packet_ptr,
                                   (VOID *) html,
                                   strlen(html),
                                   server->nx_http_server_packet_pool_ptr,
                                   NET_HTTP_PACKET_WAIT_TICKS);
    if (NX_SUCCESS != status)
    {
        nx_packet_release(packet_ptr);
        g_net_debug_last_response_status = status;
        return status;
    }

    status = nx_http_server_callback_packet_send(server, packet_ptr);
    if (NX_SUCCESS != status)
    {
        nx_packet_release(packet_ptr);
        g_net_debug_last_response_status = status;
        return status;
    }

    g_net_debug_last_response_status = NX_HTTP_CALLBACK_COMPLETED;
    return NX_HTTP_CALLBACK_COMPLETED;
}

static UINT send_plain_response(NX_HTTP_SERVER *server, NX_PACKET *packet_ptr, const char *text)
{
    UINT status;

    if (NULL == text)
    {
        g_net_debug_last_response_status = NX_PTR_ERROR;
        return NX_PTR_ERROR;
    }

    status = nx_http_server_callback_generate_response_header(server,
                                                              &packet_ptr,
                                                              NX_HTTP_STATUS_OK,
                                                              strlen(text),
                                                              "text/plain",
                                                              NULL);
    if (NX_SUCCESS != status)
    {
        g_net_debug_last_response_status = status;
        return status;
    }

    status = nx_packet_data_append(packet_ptr,
                                   (VOID *) text,
                                   strlen(text),
                                   server->nx_http_server_packet_pool_ptr,
                                   NET_HTTP_PACKET_WAIT_TICKS);
    if (NX_SUCCESS != status)
    {
        nx_packet_release(packet_ptr);
        g_net_debug_last_response_status = status;
        return status;
    }

    status = nx_http_server_callback_packet_send(server, packet_ptr);
    if (NX_SUCCESS != status)
    {
        nx_packet_release(packet_ptr);
        g_net_debug_last_response_status = status;
        return status;
    }

    g_net_debug_last_response_status = NX_HTTP_CALLBACK_COMPLETED;
    return NX_HTTP_CALLBACK_COMPLETED;
}

static UINT send_javascript_response(NX_HTTP_SERVER *server, NX_PACKET *packet_ptr, const char *script)
{
    UINT status;

    if (NULL == script)
    {
        g_net_debug_last_response_status = NX_PTR_ERROR;
        return NX_PTR_ERROR;
    }

    status = nx_http_server_callback_generate_response_header(server,
                                                              &packet_ptr,
                                                              NX_HTTP_STATUS_OK,
                                                              strlen(script),
                                                              "application/javascript",
                                                              NULL);
    if (NX_SUCCESS != status)
    {
        g_net_debug_last_response_status = status;
        return status;
    }

    status = nx_packet_data_append(packet_ptr,
                                   (VOID *) script,
                                   strlen(script),
                                   server->nx_http_server_packet_pool_ptr,
                                   NET_HTTP_PACKET_WAIT_TICKS);
    if (NX_SUCCESS != status)
    {
        nx_packet_release(packet_ptr);
        g_net_debug_last_response_status = status;
        return status;
    }

    status = nx_http_server_callback_packet_send(server, packet_ptr);
    if (NX_SUCCESS != status)
    {
        nx_packet_release(packet_ptr);
        g_net_debug_last_response_status = status;
        return status;
    }

    g_net_debug_last_response_status = NX_HTTP_CALLBACK_COMPLETED;
    return NX_HTTP_CALLBACK_COMPLETED;
}

static UINT send_plain_response_fresh(NX_HTTP_SERVER *server, const char *text)
{
    NX_PACKET *resp_packet;
    char response[128];
    int header_len;
    size_t text_len;

    if ((NULL == server) || (NULL == text))
    {
        g_net_debug_last_response_status = NX_PTR_ERROR;
        return NX_PTR_ERROR;
    }

    text_len = strlen(text);
    header_len = snprintf(response,
                          sizeof(response),
                          "HTTP/1.1 200 OK\r\n"
                          "Content-Type: text/plain\r\n"
                          "Connection: close\r\n"
                          "Content-Length: %lu\r\n"
                          "\r\n"
                          "%s",
                          (unsigned long) text_len,
                          text);
    if ((header_len <= 0) || ((size_t) header_len >= sizeof(response)))
    {
        g_net_debug_last_response_status = NX_SIZE_ERROR;
        return NX_SIZE_ERROR;
    }

    if (NX_SUCCESS != nx_packet_allocate(server->nx_http_server_packet_pool_ptr,
                                         &resp_packet,
                                         NX_TCP_PACKET,
                                         NET_HTTP_PACKET_WAIT_TICKS))
    {
        g_net_debug_last_response_status = NX_NOT_SUCCESSFUL;
        return NX_NOT_SUCCESSFUL;
    }

    if (NX_SUCCESS != nx_packet_data_append(resp_packet,
                                            (VOID *) response,
                                            (ULONG) header_len,
                                            server->nx_http_server_packet_pool_ptr,
                                            NET_HTTP_PACKET_WAIT_TICKS))
    {
        nx_packet_release(resp_packet);
        g_net_debug_last_response_status = NX_NOT_SUCCESSFUL;
        return NX_NOT_SUCCESSFUL;
    }

    if (NX_SUCCESS != nx_http_server_callback_packet_send(server, resp_packet))
    {
        nx_packet_release(resp_packet);
        g_net_debug_last_response_status = NX_NOT_SUCCESSFUL;
        return NX_NOT_SUCCESSFUL;
    }

    g_net_debug_last_response_status = NX_HTTP_CALLBACK_COMPLETED;
    return NX_HTTP_CALLBACK_COMPLETED;
}

static UINT send_plain_status_response(NX_HTTP_SERVER *server, NX_PACKET *packet_ptr, const char *status_code, const char *text)
{
    UINT status;
    size_t text_len;

    if ((NULL == server) || (NULL == packet_ptr) || (NULL == status_code) || (NULL == text))
    {
        g_net_debug_last_response_status = NX_PTR_ERROR;
        return NX_PTR_ERROR;
    }

    text_len = strlen(text);
    status = nx_http_server_callback_generate_response_header(server,
                                                              &packet_ptr,
                                                              (CHAR *) status_code,
                                                              text_len,
                                                              "text/plain",
                                                              NULL);
    if (NX_SUCCESS != status)
    {
        g_net_debug_last_response_status = status;
        return status;
    }

    status = nx_packet_data_append(packet_ptr,
                                   (VOID *) text,
                                   text_len,
                                   server->nx_http_server_packet_pool_ptr,
                                   NET_HTTP_PACKET_WAIT_TICKS);
    if (NX_SUCCESS != status)
    {
        nx_packet_release(packet_ptr);
        g_net_debug_last_response_status = status;
        return status;
    }

    status = nx_http_server_callback_packet_send(server, packet_ptr);
    if (NX_SUCCESS != status)
    {
        nx_packet_release(packet_ptr);
        g_net_debug_last_response_status = status;
        return status;
    }

    g_net_debug_last_response_status = NX_HTTP_CALLBACK_COMPLETED;
    return NX_HTTP_CALLBACK_COMPLETED;
}

static UINT send_callback_response(NX_HTTP_SERVER *server, const char *status_code, const char *information)
{
    UINT status;

    if ((NULL == server) || (NULL == status_code) || (NULL == information))
    {
        g_net_debug_last_response_status = NX_PTR_ERROR;
        return NX_PTR_ERROR;
    }

    status = nx_http_server_callback_response_send(server,
                                                   (CHAR *) status_code,
                                                   (CHAR *) information,
                                                   NX_NULL);
    if (NX_SUCCESS != status)
    {
        g_net_debug_last_response_status = status;
        return status;
    }

    g_net_debug_last_response_status = NX_HTTP_CALLBACK_COMPLETED;
    return NX_HTTP_CALLBACK_COMPLETED;
}

static UINT send_redirect(NX_HTTP_SERVER *server, const char *location)
{
    char header[128];
    NX_PACKET *resp_packet;

    snprintf(header,
             sizeof(header),
             "HTTP/1.1 303 See Other\r\nLocation: %s\r\nContent-Length: 0\r\n\r\n",
             location);

    if (NX_SUCCESS != nx_packet_allocate(server->nx_http_server_packet_pool_ptr,
                                         &resp_packet,
                                         NX_TCP_PACKET,
                                         NET_HTTP_PACKET_WAIT_TICKS))
    {
        g_net_debug_last_response_status = NX_NOT_SUCCESSFUL;
        return NX_NOT_SUCCESSFUL;
    }

    if (NX_SUCCESS != nx_packet_data_append(resp_packet,
                                            (VOID *) header,
                                            strlen(header),
                                            server->nx_http_server_packet_pool_ptr,
                                            NET_HTTP_PACKET_WAIT_TICKS))
    {
        nx_packet_release(resp_packet);
        g_net_debug_last_response_status = NX_NOT_SUCCESSFUL;
        return NX_NOT_SUCCESSFUL;
    }

    if (NX_SUCCESS != nx_http_server_callback_packet_send(server, resp_packet))
    {
        nx_packet_release(resp_packet);
        g_net_debug_last_response_status = NX_NOT_SUCCESSFUL;
        return NX_NOT_SUCCESSFUL;
    }

    g_net_debug_last_response_status = NX_HTTP_CALLBACK_COMPLETED;
    return NX_HTTP_CALLBACK_COMPLETED;
}

static UINT render_home_page(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, const char *message, int edit_index)
{
    /* ADICIONADO 'static' PARA EVITAR STACK OVERFLOW! */
    static char html[HTML_BUFFER_SIZE];
    static char rows[ROWS_BUFFER_SIZE];
    static char photo_options[PHOTO_OPTIONS_BUFFER_SIZE];
    static char profile_options[2048];
    static storage_user_profile_t profile_snapshot[STORAGE_MAX_USERS];
    const char *message_to_render = message;

    char ip_text[20];
    char netmask_text[20];
    char gateway_text[20];
    char last_uid[UID_MAX_LEN];
    char last_user[NAME_MAX_LEN];

    /* Variáveis restauradas para a leitura do storage */
    char form_cards[FORM_CARDS_BUFFER_SIZE];
    const char *persist_text = "ociosa";
    storage_user_profile_t form_profile;

    ULONG ip_address;
    ULONG network_mask;
    ULONG link_status = 0U;
    size_t rows_len = 0U;
    int user_count;
    bool has_users = false;
    bool editing = false;
    const char *current_photo_status = "-";

    nx_ip_address_get(&g_ip0, &ip_address, &network_mask);
    ip_to_string(ip_address, ip_text, sizeof(ip_text));
    ip_to_string(network_mask, netmask_text, sizeof(netmask_text));
    ip_to_string(DEVICE_GATEWAY_ADDR, gateway_text, sizeof(gateway_text));
    (void) nx_ip_status_check(&g_ip0, NX_IP_LINK_ENABLED, &link_status, NX_NO_WAIT);

    app_state_lock();
    strncpy(last_uid, (const char *) g_app_state.last_uid, sizeof(last_uid) - 1U);
    last_uid[sizeof(last_uid) - 1U] = '\0';
    strncpy(last_user, (const char *) g_app_state.last_user, sizeof(last_user) - 1U);
    last_user[sizeof(last_user) - 1U] = '\0';
    app_state_unlock();

    memset(&form_profile, 0, sizeof(form_profile));
    form_cards[0] = '\0';

    rows[0] = '\0';
    user_count = storage_profile_snapshot_nowait(profile_snapshot, STORAGE_MAX_USERS);

    if ((edit_index >= 0) && (edit_index < user_count))
    {
        form_profile = profile_snapshot[edit_index];
        editing = true;
        profile_cards_to_csv(&form_profile, form_cards, sizeof(form_cards));
    }
    else if ('\0' != last_uid[0])
    {
        strncpy(form_cards, last_uid, sizeof(form_cards) - 1U);
        form_cards[sizeof(form_cards) - 1U] = '\0';
    }

    net_build_photo_options(photo_options, sizeof(photo_options));
    net_build_profile_options(profile_snapshot, user_count, profile_options, sizeof(profile_options));
    if ('\0' != form_profile.photo_id[0])
    {
        current_photo_status = net_photo_asset_exists(form_profile.photo_id) ? "asset encontrado" : "asset nao encontrado";
    }

    switch (storage_persist_status())
    {
        case 1U:
            persist_text = "pendente";
            break;
        case 2U:
            persist_text = "ok";
            break;
        case 3U:
            persist_text = "falhou";
            break;
        default:
            persist_text = "ociosa";
            break;
    }

    if ((NULL == message_to_render) && ('\0' != g_net_flash_message[0]))
    {
        message_to_render = g_net_flash_message;
        g_net_flash_message[0] = '\0';
    }

    for (int i = 0; i < user_count; i++)
    {
        char row_cards[FORM_CARDS_BUFFER_SIZE];

        int written;

        profile_cards_to_csv(&profile_snapshot[i], row_cards, sizeof(row_cards));

        written = snprintf(&rows[rows_len],
                           sizeof(rows) - rows_len,
                           "<tr><td>%s</td><td>%s</td><td>%s</td><td>%s%s</td><td>%s</td>"
                           "<td><a class='small' href='/?edit=%d'>Editar</a> "
                           "<a class='small danger' href='/remove_user?index=%d'>Remover</a></td></tr>",
                           profile_snapshot[i].name,
                           ('\0' != profile_snapshot[i].role[0]) ? profile_snapshot[i].role : "-",
                           ('\0' != profile_snapshot[i].chapter[0]) ? profile_snapshot[i].chapter : "-",
                           ('\0' != profile_snapshot[i].photo_id[0]) ? profile_snapshot[i].photo_id : "-",
                           ('\0' != profile_snapshot[i].photo_id[0]) ? (net_photo_asset_exists(profile_snapshot[i].photo_id) ? " <span class='muted'>(ok)</span>" : " <span class='muted'>(sem asset)</span>") : "",
                           ('\0' != row_cards[0]) ? row_cards : "-",
                           i,
                           i);

        if ((written > 0) && ((size_t) written < (sizeof(rows) - rows_len)))
        {
            rows_len += (size_t) written;
            has_users = true;
        }
    }

    if (!has_users)
    {
        strncpy(rows,
                "<tr><td colspan='6'>Nenhum usuario cadastrado</td></tr>",
                sizeof(rows) - 1U);
        rows[sizeof(rows) - 1U] = '\0';
    }

    snprintf(html,
             sizeof(html),
             "<html><head><meta charset='utf-8'>"
             "<style>"
             "body{font-family:Arial,sans-serif;background:#f5f7fb;color:#18212d;margin:0;padding:20px;}"
             ".wrap{max-width:1100px;margin:0 auto;}"
             ".card{background:#fff;border-radius:16px;padding:20px;margin-bottom:18px;box-shadow:0 8px 26px rgba(0,0,0,.08);}"
             "h1,h2{margin:0 0 12px 0;}p{margin:6px 0;}table{width:100%%;border-collapse:collapse;}"
             "td,th{padding:10px;border-bottom:1px solid #e7ebf2;text-align:left;vertical-align:top;}"
             "input,textarea{width:100%%;padding:12px;border:1px solid #ccd4e0;border-radius:10px;margin:8px 0 12px 0;box-sizing:border-box;font-family:Consolas,monospace;}"
             "textarea{min-height:180px;resize:vertical;}"
             ".btn,.small{display:inline-block;text-decoration:none;border:none;border-radius:10px;padding:12px 16px;background:#0b6ef3;color:#fff;}"
             ".small{padding:8px 12px;font-size:13px;}.danger{background:#d64545;}.muted{color:#607086;font-size:14px;}.ok{color:#137333;}"
             ".warn{color:#b26a00;}.grid{display:grid;grid-template-columns:1.1fr 1.3fr;gap:16px;}"
             "@media(max-width:900px){.grid{grid-template-columns:1fr;}}"
             "</style></head><body><div class='wrap'>"
             "<div class='card'><h1>S7G2 - Controle de acesso</h1>"
             "<p class='muted'>IP atual: <strong>%s</strong> | Mascara: <strong>%s</strong> | Gateway: <strong>%s</strong></p>"
             "<p class='%s'>Link Ethernet: <strong>%s</strong></p>"
             "<p class='muted'>Esta versao usa IP estatico. Se a sua rede nao estiver na faixa 192.168.15.x, a placa nao vai responder sem ajuste em <code>net.c</code>.</p>"
             "%s"
             "</div>"
             "<div class='grid'>"
             "<div class='card'><h2>%s</h2>"
             "<form action='/add_user' method='get'>"
             "<input type='hidden' name='edit' value='%d'>"
             "<label>Nome</label><input type='text' name='name' maxlength='31' value='%s' placeholder='Nome do usuario'>"
             "<label>Cargo</label><input type='text' name='role' maxlength='47' value='%s' placeholder='Ex.: Presidente'>"
             "<label>Capitulo IEEE</label><input type='text' name='chapter' maxlength='47' value='%s' placeholder='Ex.: Computer Society'>"
             "<label>Cartoes (separados por virgula)</label><input type='text' name='cards' maxlength='127' value='%s' placeholder='E35C051C,1234ABCD ou deixe vazio para vincular depois'>"
             "<label>Foto (identificador)</label><input type='text' name='photo_id' list='photo-id-list' maxlength='63' value='%s' placeholder='Escolha um photo_id importado'>"
             "<datalist id='photo-id-list'>%s</datalist>"
             "<p class='muted'>Fotos no firmware: <strong>%u</strong>. Status do photo_id atual: <strong>%s</strong>.</p>"
             "<button class='btn' type='submit'>Salvar perfil</button></form>"
             "%s"
             "<p><a class='small' href='/save_users'>Persistir cadastros</a></p>"
             "<p class='muted'>Persistencia QSPI: <strong>%s</strong></p>"
             "<p class='muted'>Ultimo cartao lido: <strong>%s</strong></p>"
             "<p class='muted'>Ultimo usuario: <strong>%s</strong></p>"
             "<p class='muted'>Campo foto preparado para upload/downscale e importacao offline futura.</p>"
             "<p><a class='small' href='/portaon'>Abrir porta</a></p></div>"
             "<div class='card'><h2>Perfis cadastrados</h2>"
             "<table><tr><th>Nome</th><th>Cargo</th><th>Capitulo</th><th>Foto</th><th>Cartoes</th><th>Acao</th></tr>%s</table>"
             "<div class='card' style='margin-top:18px;'><h2>Anexar foto a perfil</h2>"
             "<form action='/attach_photo' method='get'>"
             "<label>Perfil</label><select name='profile_index' style='width:100%%;padding:12px;border:1px solid #ccd4e0;border-radius:10px;margin:8px 0 12px 0;box-sizing:border-box;'>%s</select>"
             "<label>Photo ID</label><input type='text' name='photo_id' list='photo-id-list' maxlength='63' placeholder='Escolha um photo_id existente'>"
             "<button class='btn' type='submit'>Anexar foto</button></form>"
             "<p class='muted'>Use esta funcao para vincular uma foto ja embutida no firmware a um perfil existente.</p>"
             "</div>"
             "<div class='card' style='margin-top:18px;'><h2>Importar perfis offline</h2>"
             "<form action='/import_profiles' method='post'>"
             "<label>JSON de perfis</label>"
             "<textarea name='import_json' placeholder='Cole aqui o JSON gerado em script/firebase_bundle/profiles_import.json'></textarea>"
             "<button class='btn' type='submit'>Importar perfis</button></form>"
             "<p class='muted'>Use o JSON simples de perfis, sem foto binaria. Os cartoes podem ficar vazios e ser vinculados depois pela interface web.</p>"
             "</div>"
             "</div></div></div></body></html>",
             ip_text,
             netmask_text,
             gateway_text,
             (0U != link_status) ? "ok" : "warn",
             (0U != link_status) ? "conectado" : "sem link",
             (NULL != message_to_render) ? message_to_render : "",
             editing ? "Editar perfil" : "Cadastrar perfil",
             editing ? edit_index : -1,
             form_profile.name,
             form_profile.role,
             form_profile.chapter,
             form_cards,
             form_profile.photo_id,
             photo_options,
             (unsigned int) PROFILE_PHOTO_ID_COUNT,
             current_photo_status,
             editing ? "<p><a class='small danger' href='/'>Cancelar edicao</a></p>" : "",
             persist_text,
             (last_uid[0] != '\0') ? last_uid : "-",
             (last_user[0] != '\0') ? last_user : "-",
             rows,
             profile_options);

    return send_html_response(server_ptr, packet_ptr, html);
}

static UINT handle_add_user(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, const char *query)
{
    storage_user_profile_t profile;
    char primary_uid[UID_MAX_LEN];
    int edit_index = -1;

    if (!profile_from_query(query, &profile, &edit_index))
    {
        return render_home_page(server_ptr,
                                packet_ptr,
                                "<div class='card warn'>Preencha ao menos o nome do perfil. Os cartoes podem ser vinculados depois.</div>",
                                edit_index);
    }

    if (storage_profile_upsert(&profile, edit_index))
    {
        if (profile.card_count > 0U)
        {
            strncpy(primary_uid, profile.cards[0], sizeof(primary_uid) - 1U);
            primary_uid[sizeof(primary_uid) - 1U] = '\0';
            app_post_event(EVENT_USER_ADDED, primary_uid);
        }
        return render_home_page(server_ptr,
                                packet_ptr,
                                "<div class='card ok'>Perfil salvo em runtime. Clique em \"Persistir cadastros\" para salvar na QSPI.</div>",
                                -1);
    }

    return render_home_page(server_ptr,
                            packet_ptr,
                            "<div class='card warn'>Nao foi possivel salvar o perfil. Verifique cartoes duplicados ou campos vazios.</div>",
                            edit_index);
}

static UINT handle_attach_photo(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, const char *query)
{
    storage_user_profile_t profile;
    int profile_index = query_get_int(query, "profile_index", -1);
    bool persist_requested = false;

    if ((profile_index < 0) || !storage_profile_get(profile_index, &profile))
    {
        return render_home_page(server_ptr,
                                packet_ptr,
                                "<div class='card warn'>Selecione um perfil valido para anexar a foto.</div>",
                                -1);
    }

    if (!query_get_value(query, "photo_id", profile.photo_id, sizeof(profile.photo_id)))
    {
        return render_home_page(server_ptr,
                                packet_ptr,
                                "<div class='card warn'>Selecione um photo_id antes de anexar a foto.</div>",
                                -1);
    }

    if (!net_photo_asset_exists(profile.photo_id))
    {
        return render_home_page(server_ptr,
                                packet_ptr,
                                "<div class='card warn'>Esse photo_id nao existe no firmware atual.</div>",
                                -1);
    }

    if (storage_profile_upsert(&profile, profile_index))
    {
        persist_requested = storage_persist_now();
        return render_home_page(server_ptr,
                                packet_ptr,
                                persist_requested
                                    ? "<div class='card ok'>Foto anexada e persistencia automatica solicitada.</div>"
                                    : "<div class='card ok'>Foto anexada em runtime. Se quiser, use \"Persistir cadastros\" como fallback.</div>",
                                -1);
    }

    return render_home_page(server_ptr,
                            packet_ptr,
                            "<div class='card warn'>Nao foi possivel anexar a foto ao perfil.</div>",
                            -1);
}

static UINT handle_import_profiles(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, const char *form_data)
{
    static char import_json[IMPORT_BUFFER_SIZE];
    int imported_count;
    SSP_PARAMETER_NOT_USED(packet_ptr);

    if (!query_get_value_raw(form_data, "import_json", import_json, sizeof(import_json)))
    {
        net_set_flash_message("<div class='card warn'>Cole o JSON de perfis antes de importar.</div>");
        return send_redirect(server_ptr, "/");
    }

    imported_count = import_profiles_from_json(import_json);
    if (imported_count > 0)
    {
        net_set_flash_message("<div class='card ok'>Perfis importados em runtime. Clique em \"Persistir cadastros\" para salvar na QSPI.</div>");
        return send_redirect(server_ptr, "/");
    }

    net_set_flash_message("<div class='card warn'>Nenhum perfil foi importado. Use o JSON simples gerado em script/firebase_bundle/profiles_import.json.</div>");
    return send_redirect(server_ptr, "/");
}

static void extract_body_from_packet_fallback(NX_PACKET *packet_ptr, char *body_out, size_t body_size)
{
    char *req;
    char *req_end;
    char *body_start;
    size_t len;

    body_out[0] = '\0';
    if ((NULL == packet_ptr) || (NULL == packet_ptr->nx_packet_prepend_ptr) || (0U == body_size))
    {
        return;
    }

    req = (char *) packet_ptr->nx_packet_prepend_ptr;
    req_end = (char *) packet_ptr->nx_packet_append_ptr;
    body_start = strstr(req, "\r\n\r\n");
    if ((NULL == body_start) || (body_start > req_end))
    {
        return;
    }

    body_start += 4;
    if (body_start > req_end)
    {
        return;
    }

    len = (size_t) (req_end - body_start);
    if (len >= body_size)
    {
        len = body_size - 1U;
    }

    memcpy(body_out, body_start, len);
    body_out[len] = '\0';
}

static void extract_body_from_packet(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, char *body_out, size_t body_size)
{
    ULONG content_length = 0U;
    ULONG byte_offset = 0U;
    size_t total_read = 0U;
    UINT status;

    if ((NULL == body_out) || (0U == body_size))
    {
        return;
    }

    body_out[0] = '\0';

    if ((NULL == server_ptr) || (NULL == packet_ptr))
    {
        return;
    }

    status = nx_http_server_content_length_get_extended(packet_ptr, &content_length);
    if ((NX_SUCCESS == status) && (content_length > 0U))
    {
        while ((byte_offset < content_length) && ((total_read + 1U) < body_size))
        {
            UINT actual_size = 0U;
            UINT chunk_size = (UINT) (body_size - total_read - 1U);
            ULONG remaining = content_length - byte_offset;

            if (remaining < (ULONG) chunk_size)
            {
                chunk_size = (UINT) remaining;
            }

            status = nx_http_server_content_get_extended(server_ptr,
                                                         packet_ptr,
                                                         byte_offset,
                                                         &body_out[total_read],
                                                         chunk_size,
                                                         &actual_size);
            if ((NX_SUCCESS != status) || (0U == actual_size))
            {
                break;
            }

            total_read += (size_t) actual_size;
            byte_offset += (ULONG) actual_size;
        }

        body_out[total_read] = '\0';
        if (total_read > 0U)
        {
            return;
        }
    }

    extract_body_from_packet_fallback(packet_ptr, body_out, body_size);
}

static UINT handle_remove_user(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, const char *query)
{
    int index = query_get_int(query, "index", -1);

    if (index < 0)
    {
        return render_home_page(server_ptr, packet_ptr, "<div class='card warn'>Indice invalido para remocao.</div>", -1);
    }

    if (storage_profile_remove(index))
    {
        app_post_event(EVENT_USER_REMOVED, NULL);
        return render_home_page(server_ptr, packet_ptr, "<div class='card ok'>Perfil removido em runtime. Clique em \"Persistir cadastros\" para salvar na QSPI.</div>", -1);
    }

    return render_home_page(server_ptr, packet_ptr, "<div class='card warn'>Perfil nao encontrado.</div>", -1);
}

static UINT handle_save_users(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr)
{
    if (storage_persist_now())
    {
        return render_home_page(server_ptr, packet_ptr, "<div class='card ok'>Persistencia solicitada. Aguarde alguns segundos e recarregue a pagina.</div>", -1);
    }

    return render_home_page(server_ptr, packet_ptr, "<div class='card warn'>Nao foi possivel persistir os cadastros agora.</div>", -1);
}

static UINT light_redirect_with_flash(NX_HTTP_SERVER *server_ptr, const char *location, const char *message)
{
    net_set_flash_message(message);
    return send_redirect(server_ptr, location);
}

static const char *light_persist_status_text(void)
{
    switch (storage_persist_status())
    {
        case 1U:
            return "pendente";
        case 2U:
            return "ok";
        case 3U:
            return "falhou";
        default:
            return "ociosa";
    }
}

static void extract_query_from_resource(const char *resource, char *query_out, size_t query_size)
{
    const char *q_mark;
    size_t len;

    if ((NULL == query_out) || (0U == query_size))
    {
        return;
    }

    query_out[0] = '\0';
    if (NULL == resource)
    {
        return;
    }

    q_mark = strchr(resource, '?');
    if (NULL == q_mark)
    {
        return;
    }

    len = strlen(q_mark + 1);
    if (len >= query_size)
    {
        len = query_size - 1U;
    }

    memcpy(query_out, q_mark + 1, len);
    query_out[len] = '\0';
}

static UINT render_light_shell(NX_HTTP_SERVER *server_ptr,
                               NX_PACKET *packet_ptr,
                               const char *title,
                               const char *body_html,
                               const char *message)
{
    static char html[HTML_BUFFER_SIZE];
    static char content_card[HTML_BUFFER_SIZE / 2];
    const char *message_to_render = message;
    const char *content_html = "";
    const bool render_content_card = (((NULL != title) && ('\0' != title[0])) ||
                                      ((NULL != body_html) && ('\0' != body_html[0])));
    char ip_text[20];
    char netmask_text[20];
    char gateway_text[20];
    ULONG ip_address = 0U;
    ULONG network_mask = 0U;
    ULONG link_status = 0U;
    bool is_admin = net_admin_is_authenticated();

    nx_ip_address_get(&g_ip0, &ip_address, &network_mask);
    ip_to_string(ip_address, ip_text, sizeof(ip_text));
    ip_to_string(network_mask, netmask_text, sizeof(netmask_text));
    ip_to_string(DEVICE_GATEWAY_ADDR, gateway_text, sizeof(gateway_text));
    (void) nx_ip_status_check(&g_ip0, NX_IP_LINK_ENABLED, &link_status, NX_NO_WAIT);

    if ((NULL == message_to_render) && ('\0' != g_net_flash_message[0]))
    {
        message_to_render = g_net_flash_message;
        g_net_flash_message[0] = '\0';
    }

    if (render_content_card)
    {
        snprintf(content_card,
                 sizeof(content_card),
                 "<div class='card'><h2>%s</h2>%s</div>",
                 (NULL != title) ? title : "",
                 (NULL != body_html) ? body_html : "");
        content_html = content_card;
    }

    snprintf(html,
             sizeof(html),
             "<html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'>"
             "<style>"
             "body{font-family:Arial,sans-serif;background:#f5f7fb;color:#18212d;margin:0;padding:20px;}"
             ".wrap{max-width:1100px;margin:0 auto;}"
             ".card{background:#fff;border-radius:16px;padding:20px;margin-bottom:18px;box-shadow:0 8px 26px rgba(0,0,0,.08);}"
             "h1,h2{margin:0 0 12px 0;}h2{font-size:24px;}p{margin:6px 0;}table{width:100%%;border-collapse:collapse;}"
             "td,th{padding:10px;border-bottom:1px solid #e7ebf2;text-align:left;vertical-align:top;}"
             "input,textarea,select{width:100%%;padding:12px;border:1px solid #ccd4e0;border-radius:10px;margin:8px 0 12px 0;box-sizing:border-box;font-family:Consolas,monospace;font-size:16px;}"
             "textarea{min-height:220px;resize:vertical;}"
             ".btn,.small{display:inline-block;text-decoration:none;border:none;border-radius:10px;padding:12px 16px;background:#0b6ef3;color:#fff;cursor:pointer;}"
             ".small{padding:8px 12px;font-size:13px;margin-right:8px;margin-bottom:8px;}.secondary{background:#6b7a90;}.danger{background:#d64545;}"
             ".muted{color:#607086;font-size:14px;}.ok{color:#137333;}.warn{color:#b26a00;}.nav{display:flex;flex-wrap:wrap;gap:8px;margin-top:12px;}.actions{display:flex;flex-wrap:wrap;gap:10px;margin-top:10px;}.table-wrap{overflow-x:auto;-webkit-overflow-scrolling:touch;}"
             "@media(max-width:720px){body{padding:12px;}.wrap{max-width:100%%;}.card{padding:14px;border-radius:14px;}.nav,.actions{flex-direction:column;align-items:stretch;gap:8px;}.btn,.small{display:block;width:100%%;box-sizing:border-box;margin-right:0;margin-bottom:0;text-align:center;}.small{padding:12px 14px;font-size:14px;}h1{font-size:24px;}h2{font-size:20px;}table{display:block;overflow-x:auto;-webkit-overflow-scrolling:touch;}td,th{padding:8px;font-size:13px;}}"
             "</style></head><body><div class='wrap'>"
             "<div class='card'><h1>S7G2 - Controle de acesso</h1>"
             "<p class='muted'>IP atual: <strong>%s</strong> | Mascara: <strong>%s</strong> | Gateway: <strong>%s</strong></p>"
             "<p class='%s'>Link Ethernet: <strong>%s</strong></p>"
             "<p class='muted'>Sessao admin web: <strong>%s</strong> | Persistencia QSPI: <strong>%s</strong></p>"
             "<div class='nav'><a class='small' href='/'>Inicio</a>%s%s</div></div>"
             "%s"
             "%s"
             "</div></body></html>",
             ip_text,
             netmask_text,
             gateway_text,
             (0U != link_status) ? "ok" : "warn",
             (0U != link_status) ? "conectado" : "sem link",
             is_admin ? "autenticada" : "bloqueada",
             light_persist_status_text(),
             is_admin ? "<a class='small' href='/admin_profiles'>Perfis</a><a class='small' href='/profile_form'>Novo perfil</a><a class='small' href='/upload_photo'>Upload foto</a><a class='small' href='/import'>Importar</a><a class='small' href='/access_log'>Log</a><a class='small' href='/door'>Porta</a>" : "",
             is_admin ? "<a class='small secondary' href='/logout'>Sair</a>" : "<a class='small' href='/login'>Entrar</a>",
             (NULL != message_to_render) ? message_to_render : "",
             content_html);

    return send_html_response(server_ptr, packet_ptr, html);
}

static UINT render_light_login_page(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, const char *message)
{
    static char body[2048];

    if (net_admin_is_authenticated())
    {
        snprintf(body,
                 sizeof(body),
                 "<p class='ok'>A autenticacao admin ja esta ativa.</p>"
                 "<div class='actions'><a class='small' href='/admin_profiles'>Ir para perfis</a><a class='small secondary' href='/logout'>Encerrar sessao</a></div>");
    }
    else
    {
        snprintf(body,
                 sizeof(body),
                 "<p class='muted'>Use o PIN admin para habilitar cadastro, edicao, importacao e controle da porta.</p>"
                 "<form action='/login' method='post'>"
                 "<label>PIN admin</label><input type='password' name='pin' maxlength='8' placeholder='Digite o PIN'>"
                 "<button class='btn' type='submit'>Entrar</button></form>");
    }

    return render_light_shell(server_ptr, packet_ptr, "Autenticacao admin", body, message);
}

static UINT render_light_dashboard_page(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, const char *message)
{
    return render_light_shell(server_ptr, packet_ptr, "", NULL, message);
}

static UINT render_light_profiles_page(NX_HTTP_SERVER *server_ptr,
                                       NX_PACKET *packet_ptr,
                                       const char *message,
                                       int page)
{
    static char html[8192];
    static char rows[4096];
    static storage_user_profile_t profile_snapshot[STORAGE_MAX_USERS];
    const char *message_to_render = message;
    size_t rows_len = 0U;
    int user_count;
    int start_index;
    int end_index;
    bool has_prev;
    bool has_next;
    char prev_href[32];
    char next_href[32];
    char prev_button[96];
    char next_button[96];

    if (!net_admin_is_authenticated())
    {
        return render_light_login_page(server_ptr, packet_ptr, "<div class='card warn'>Autentique-se para visualizar e editar os perfis.</div>");
    }

    if ((NULL == message_to_render) && ('\0' != g_net_flash_message[0]))
    {
        message_to_render = g_net_flash_message;
        g_net_flash_message[0] = '\0';
    }

    if (page < 0)
    {
        page = 0;
    }

    user_count = storage_profile_snapshot_nowait(profile_snapshot, STORAGE_MAX_USERS);
    start_index = page * PROFILES_PAGE_SIZE;
    if (start_index > user_count)
    {
        start_index = 0;
        page = 0;
    }
    end_index = start_index + PROFILES_PAGE_SIZE;
    if (end_index > user_count)
    {
        end_index = user_count;
    }
    has_prev = (page > 0);
    has_next = (end_index < user_count);
    snprintf(prev_href, sizeof(prev_href), "/admin_profiles?page=%d", has_prev ? (page - 1) : 0);
    snprintf(next_href, sizeof(next_href), "/admin_profiles?page=%d", page + 1);
    if (has_prev)
    {
        snprintf(prev_button, sizeof(prev_button), "<a class='small secondary' href='%s'>Anterior</a>", prev_href);
    }
    else
    {
        prev_button[0] = '\0';
    }
    if (has_next)
    {
        snprintf(next_button, sizeof(next_button), "<a class='small' href='%s'>Proxima</a>", next_href);
    }
    else
    {
        next_button[0] = '\0';
    }

    rows[0] = '\0';
    if (user_count <= 0)
    {
        strncpy(rows,
                "<tr><td colspan='5'>Nenhum perfil carregado em RAM no momento.</td></tr>",
                sizeof(rows) - 1U);
        rows[sizeof(rows) - 1U] = '\0';
    }
    else
    {
        for (int i = start_index; i < end_index; i++)
        {
            char cards_csv[FORM_CARDS_BUFFER_SIZE];
            int written;

            profile_cards_to_csv(&profile_snapshot[i], cards_csv, sizeof(cards_csv));
            written = snprintf(&rows[rows_len],
                               sizeof(rows) - rows_len,
                               "<tr><td>%d</td><td>%s</td><td>%s</td><td>%s</td><td>%s</td><td>"
                               "<a class='small' href='/profile_form?edit=%d'>Editar</a> "
                               "<a class='small danger' href='/remove_user?index=%d'>Remover</a>"
                               "</td></tr>",
                               i,
                               profile_snapshot[i].name,
                               ('\0' != profile_snapshot[i].role[0]) ? profile_snapshot[i].role : "-",
                               ('\0' != profile_snapshot[i].chapter[0]) ? profile_snapshot[i].chapter : "-",
                               ('\0' != cards_csv[0]) ? cards_csv : "-",
                               i,
                               i);
            if ((written <= 0) || ((size_t) written >= (sizeof(rows) - rows_len)))
            {
                break;
            }
            rows_len += (size_t) written;
        }
    }

    snprintf(html,
             sizeof(html),
             "<html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'>"
             "<style>"
             "body{font-family:Arial,sans-serif;background:#f5f7fb;color:#18212d;margin:0;padding:18px;}"
             ".wrap{max-width:980px;margin:0 auto;}"
             ".card{background:#fff;border-radius:16px;padding:18px;margin-bottom:16px;box-shadow:0 8px 26px rgba(0,0,0,.08);}"
             ".actions{display:flex;flex-wrap:wrap;gap:10px;margin-top:14px;}"
             ".small{display:inline-block;text-decoration:none;border:none;border-radius:10px;padding:12px 16px;background:#0b6ef3;color:#fff;}"
             ".secondary{background:#6b7a90;}.danger{background:#d64545;}.muted{color:#607086;font-size:14px;}.warn{color:#b26a00;}.ok{color:#137333;}"
             ".table-wrap{overflow-x:auto;-webkit-overflow-scrolling:touch;}table{width:100%%;border-collapse:collapse;}th,td{padding:10px;border-bottom:1px solid #e7ebf2;text-align:left;vertical-align:top;}"
             "@media(max-width:720px){body{padding:12px;}.wrap{max-width:100%%;}.card{padding:14px;border-radius:14px;}.actions{flex-direction:column;align-items:stretch;gap:8px;}.small{display:block;width:100%%;box-sizing:border-box;text-align:center;}.table-wrap{margin:0 -4px;}table{display:block;overflow-x:auto;-webkit-overflow-scrolling:touch;}th,td{padding:8px;font-size:13px;white-space:nowrap;}}"
             "</style></head><body><div class='wrap'><div class='card'>"
             "<h1>Perfis existentes</h1>"
             "<div class='actions'><a class='small' href='/'>Inicio</a><a class='small' href='/profile_form'>Novo perfil</a><a class='small' href='/import'>Importar perfis</a><a class='small' href='/save_users'>Persistir cadastros</a><a class='small secondary' href='/'>Voltar</a></div>"
             "%s"
             "%s"
             "<p class='muted'>Mostrando %d a %d de %d perfis carregados em RAM.</p>"
             "<div class='table-wrap'><table><tr><th>Indice</th><th>Nome</th><th>Cargo</th><th>Capitulo</th><th>Cartoes</th><th>Acao</th></tr>%s</table></div>"
             "<div class='actions'>"
             "%s"
             "%s"
             "</div>"
             "</div></div></body></html>",
             (NULL != message_to_render) ? message_to_render : "",
             (user_count > 0)
                 ? ""
                 : "<p class='warn'>Se voce acabou de ligar a placa, aguarde alguns segundos e recarregue. Esta pagina usa o snapshot em RAM para ficar leve.</p>",
             (user_count > 0) ? (start_index + 1) : 0,
             end_index,
             user_count,
             rows,
             prev_button,
             next_button);

    return send_html_response(server_ptr, packet_ptr, html);
}

static UINT render_light_profile_form_page(NX_HTTP_SERVER *server_ptr,
                                           NX_PACKET *packet_ptr,
                                           const char *message,
                                           int edit_index)
{
    static char html[8192];
    static char photo_options[PHOTO_OPTIONS_BUFFER_SIZE];
    static storage_user_profile_t profile_snapshot[STORAGE_MAX_USERS];
    storage_user_profile_t form_profile;
    char form_cards[FORM_CARDS_BUFFER_SIZE];
    const char *current_photo_status = "-";
    const char *message_to_render = message;
    char last_uid[UID_MAX_LEN];
    int user_count;
    bool editing = false;

    if (!net_admin_is_authenticated())
    {
        return render_light_login_page(server_ptr, packet_ptr, "<div class='card warn'>Autentique-se para criar ou editar perfis.</div>");
    }

    if ((NULL == message_to_render) && ('\0' != g_net_flash_message[0]))
    {
        message_to_render = g_net_flash_message;
        g_net_flash_message[0] = '\0';
    }

    memset(&form_profile, 0, sizeof(form_profile));
    form_cards[0] = '\0';
    last_uid[0] = '\0';
    user_count = storage_profile_snapshot_nowait(profile_snapshot, STORAGE_MAX_USERS);

    if ((edit_index >= 0) && (edit_index < user_count))
    {
        form_profile = profile_snapshot[edit_index];
        editing = true;
        profile_cards_to_csv(&form_profile, form_cards, sizeof(form_cards));
    }
    else
    {
        app_state_lock();
        strncpy(last_uid, (const char *) g_app_state.last_uid, sizeof(last_uid) - 1U);
        last_uid[sizeof(last_uid) - 1U] = '\0';
        app_state_unlock();

        if ('\0' != last_uid[0])
        {
            strncpy(form_cards, last_uid, sizeof(form_cards) - 1U);
            form_cards[sizeof(form_cards) - 1U] = '\0';
        }
    }

    net_build_photo_options(photo_options, sizeof(photo_options));
    if ('\0' != form_profile.photo_id[0])
    {
        current_photo_status = net_photo_asset_exists(form_profile.photo_id) ? "asset encontrado" : "asset nao encontrado";
    }

    snprintf(html,
             sizeof(html),
             "<html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'>"
             "<style>"
             "body{font-family:Arial,sans-serif;background:#f5f7fb;color:#18212d;margin:0;padding:18px;}"
             ".wrap{max-width:760px;margin:0 auto;}"
             ".card{background:#fff;border-radius:16px;padding:18px;margin-bottom:16px;box-shadow:0 8px 26px rgba(0,0,0,.08);}"
             "input,select{width:100%%;padding:12px;border:1px solid #ccd4e0;border-radius:10px;margin:8px 0 12px 0;box-sizing:border-box;font-family:Consolas,monospace;font-size:16px;}"
             ".btn,.small{display:inline-block;text-decoration:none;border:none;border-radius:10px;padding:12px 16px;background:#0b6ef3;color:#fff;cursor:pointer;}"
             ".small{padding:8px 12px;font-size:13px;margin-right:8px;}.secondary{background:#6b7a90;}"
             ".muted{color:#607086;font-size:14px;}.warn{color:#b26a00;}.actions{display:flex;flex-wrap:wrap;gap:10px;margin-top:10px;}"
             "@media(max-width:720px){body{padding:12px;}.wrap{max-width:100%%;}.card{padding:14px;border-radius:14px;}.actions{flex-direction:column;align-items:stretch;gap:8px;}.btn,.small{display:block;width:100%%;box-sizing:border-box;text-align:center;margin-right:0;}}"
             "</style></head><body><div class='wrap'>"
             "<div class='card'><h2>%s</h2>"
             "<div class='actions'><a class='small' href='/'>Inicio</a><a class='small' href='/admin_profiles'>Perfis</a><a class='small secondary' href='/admin_profiles'>Voltar</a></div>"
             "%s"
             "%s"
             "<form action='/add_user' method='get'>"
             "<input type='hidden' name='edit' value='%d'>"
             "<label>Nome</label><input type='text' name='name' maxlength='31' value='%s' placeholder='Nome do usuario'>"
             "<label>Cargo</label><input type='text' name='role' maxlength='47' value='%s' placeholder='Ex.: Presidente'>"
             "<label>Capitulo IEEE</label><input type='text' name='chapter' maxlength='47' value='%s' placeholder='Ex.: Computer Society'>"
             "<label>Cartoes (separados por virgula)</label><input type='text' name='cards' maxlength='255' value='%s' placeholder='E35C051C,1234ABCD'>"
             "<label>Foto (identificador)</label><input type='text' name='photo_id' list='photo-id-list' maxlength='63' value='%s' placeholder='Escolha um photo_id importado'>"
             "<datalist id='photo-id-list'>%s</datalist>"
             "<p class='muted'>Fotos no firmware: <strong>%u</strong>. Status do photo_id atual: <strong>%s</strong>.</p>"
             "<div class='actions'><button class='btn' type='submit'>Salvar perfil</button></div></form>"
             "<p class='muted'>Esta pagina aceita mais de um cartao por perfil, separados por virgula.</p>"
             "</div></div></body></html>",
             editing ? "Editar perfil" : "Criar perfil",
             (NULL != message_to_render) ? message_to_render : "",
             (user_count > 0)
                 ? ""
                 : "<p class='warn'>Os perfis ainda nao foram carregados na RAM. Se necessario, volte para a home e tente novamente em alguns segundos.</p>",
             editing ? edit_index : -1,
             form_profile.name,
             form_profile.role,
             form_profile.chapter,
             form_cards,
             form_profile.photo_id,
             photo_options,
             (unsigned int) PROFILE_PHOTO_ID_COUNT,
             current_photo_status);

    return send_html_response(server_ptr, packet_ptr, html);
}

static UINT render_light_import_page(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, const char *message)
{
    static char body[4096];

    if (!net_admin_is_authenticated())
    {
        return render_light_login_page(server_ptr, packet_ptr, "<div class='card warn'>Autentique-se para importar perfis offline.</div>");
    }

    snprintf(body,
             sizeof(body),
              "<form action='/import_profiles' method='post'>"
              "<label>JSON de perfis</label>"
              "<textarea name='import_json' placeholder='Cole aqui o JSON gerado em script/firebase_bundle/profiles_import.json'></textarea>"
              "<div class='actions'><button class='btn' type='submit'>Importar perfis</button><a class='small secondary' href='/admin_profiles'>Voltar</a></div></form>"
              "<p class='muted'>Use o JSON simples de perfis, sem foto binaria. Os cartoes podem ficar vazios e ser vinculados depois.</p>"
              "<p class='muted'>A importacao roda em background para nao travar a interface web.</p>");

    return render_light_shell(server_ptr, packet_ptr, "Importar perfis offline", body, message);
}

static UINT render_light_upload_page(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, const char *message)
{
    static char html[4096];
    const char *message_to_render = message;

    if (!net_admin_is_authenticated())
    {
        return render_light_login_page(server_ptr, packet_ptr, "<div class='card warn'>Autentique-se para enviar fotos.</div>");
    }

    if ((NULL == message_to_render) && ('\0' != g_net_flash_message[0]))
    {
        message_to_render = g_net_flash_message;
        g_net_flash_message[0] = '\0';
    }

    snprintf(html,
             sizeof(html),
             "<html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'>"
             "<style>"
             "body{font-family:Arial,sans-serif;margin:16px;background:#f5f7fb;color:#18212d;}"
             ".card{max-width:560px;background:#fff;padding:16px;border-radius:14px;box-shadow:0 6px 20px rgba(0,0,0,.08);}"
             "input,button{width:100%%;box-sizing:border-box;font-size:16px;padding:10px;margin:8px 0;}"
             "a{display:inline-block;margin-right:8px;margin-bottom:8px;}"
             ".m{color:#5e6c84;font-size:14px;}"
             "</style></head><body><div class='card'>"
             "<h2>Upload de foto</h2>"
             "<p><a href='/'>Inicio</a><a href='/admin_profiles'>Perfis</a><a href='/admin_profiles'>Voltar</a></p>"
             "%s"
             "<p class='m'>Foto HD em runtime via 16 blocos reais de 40x40. Use o indice exibido em Perfis.</p>"
             "<label>Indice do perfil</label><input id='photo-profile' type='number' min='0' step='1' placeholder='Ex.: 0'>"
             "<label>Arquivo de imagem</label><input id='photo-file' type='file' accept='image/*'>"
             "<p id='upload-status' class='m'>Selecione uma imagem para preparar o envio.</p>"
             "<button id='upload-submit' type='button' disabled>Enviar foto HD</button>"
             "<iframe id='upload-target' name='upload-target' style='display:none;'></iframe>"
             "<form id='upload-stage-form' method='post' target='upload-target' style='display:none;'>"
             "<input type='hidden' id='upload-stage-profile' name='profile_index' value=''>"
             "<input type='hidden' id='upload-stage-width' name='width' value='%u'>"
             "<input type='hidden' id='upload-stage-height' name='height' value='%u'>"
             "<input type='hidden' id='upload-stage-tile' name='tile' value='0'>"
             "<textarea id='upload-stage-data' name='data' style='display:none;'></textarea>"
             "</form>"
             "<p class='m'>A imagem eh reduzida para %ux%u e remontada na placa em 16 blocos. Nesta versao ela nao persiste apos reboot.</p>"
             "<script src='/upload_photo_script'></script>"
             "</div></body></html>",
             (NULL != message_to_render) ? message_to_render : "",
             (unsigned int) UPLOAD_IMAGE_DIM,
             (unsigned int) UPLOAD_IMAGE_DIM,
             (unsigned int) UPLOAD_IMAGE_DIM,
             (unsigned int) UPLOAD_IMAGE_DIM);

    return send_html_response(server_ptr, packet_ptr, html);
}

static UINT render_light_upload_script(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr)
{
    static char script[4096];

    snprintf(script,
             sizeof(script),
             "(function(){"
             "const input=document.getElementById('photo-file');"
             "const profile=document.getElementById('photo-profile');"
             "const submit=document.getElementById('upload-submit');"
             "const status=document.getElementById('upload-status');"
             "const stageForm=document.getElementById('upload-stage-form');"
             "const stageProfile=document.getElementById('upload-stage-profile');"
             "const stageWidth=document.getElementById('upload-stage-width');"
             "const stageHeight=document.getElementById('upload-stage-height');"
             "const stageTile=document.getElementById('upload-stage-tile');"
             "const stageData=document.getElementById('upload-stage-data');"
             "const uploadTarget=document.getElementById('upload-target');"
             "const dim=%u,tileDim=%u;"
              "let state=null;"
              "function b64(bytes){let s='';for(let i=0;i<bytes.length;i++){s+=String.fromCharCode(bytes[i]);}return btoa(s).replace(/\\+/g,'-').replace(/\\//g,'_');}"
              "function post(action){stageForm.action=action;stageForm.submit();}"
              "function tile(ctx,tx,ty){const data=ctx.getImageData(tx,ty,tileDim,tileDim).data;const out=new Uint8Array(tileDim*tileDim*2);for(let i=0,j=0;i<data.length;i+=4,j+=2){const rgb=((data[i]&248)<<8)|((data[i+1]&252)<<3)|(data[i+2]>>3);out[j]=(rgb>>8)&255;out[j+1]=rgb&255;}return b64(out);}"
             "function next(){if(!state)return;if(state.tile>=state.tiles.length){state.phase='commit';stageTile.value='0';stageData.value='';post('/upload_photo_commit');return;}stageTile.value=String(state.tile);stageData.value=state.tiles[state.tile];state.phase='tile';status.textContent='Enviando bloco '+(state.tile+1)+'/'+state.tiles.length+'...';post('/upload_photo_chunk');}"
             "uploadTarget.addEventListener('load',function(){if(!state)return;let txt='';try{txt=(uploadTarget.contentDocument&&uploadTarget.contentDocument.body&&uploadTarget.contentDocument.body.textContent||'').trim();}catch(e){txt='';}if(txt!=='OK'){status.textContent='Falha no upload: '+(txt||'resposta invalida');submit.disabled=false;state=null;return;}if(state.phase==='begin'){next();return;}if(state.phase==='tile'){state.tile++;next();return;}status.textContent='Foto enviada. Redirecionando...';state=null;window.location='/admin_profiles';});"
             "input.addEventListener('change',function(){const file=input.files&&input.files[0];if(!file){submit.disabled=true;delete submit.dataset.tiles;status.textContent='Selecione uma imagem para preparar o envio.';return;}const img=new Image();img.onload=function(){const c=document.createElement('canvas');c.width=dim;c.height=dim;const ctx=c.getContext('2d');ctx.imageSmoothingEnabled=true;ctx.imageSmoothingQuality='high';const crop=Math.min(img.width,img.height);const sx=(img.width-crop)/2;const sy=(img.height-crop)/2;ctx.drawImage(img,sx,sy,crop,crop,0,0,dim,dim);const tiles=[];for(let ty=0;ty<dim;ty+=tileDim){for(let tx=0;tx<dim;tx+=tileDim){tiles.push(tile(ctx,tx,ty));}}submit.dataset.tiles=JSON.stringify(tiles);submit.disabled=false;status.textContent='Imagem pronta para envio em 16 blocos de 40x40.';URL.revokeObjectURL(img.src);};img.src=URL.createObjectURL(file);});"
             "submit.addEventListener('click',function(){const idx=(profile.value||'').trim();if(!idx){status.textContent='Informe o indice do perfil.';return;}if(!submit.dataset.tiles){status.textContent='Selecione uma imagem primeiro.';return;}if(state){status.textContent='Ja existe um upload em andamento.';return;}submit.disabled=true;stageProfile.value=idx;stageWidth.value=String(dim);stageHeight.value=String(dim);stageTile.value='0';stageData.value='';state={tiles:JSON.parse(submit.dataset.tiles),tile:0,phase:'begin'};status.textContent='Iniciando upload em 16 blocos...';post('/upload_photo_begin');});"
             "})();",
             (unsigned int) UPLOAD_IMAGE_DIM,
             (unsigned int) UPLOAD_TILE_DIM);

    return send_javascript_response(server_ptr, packet_ptr, script);
}

static UINT render_light_access_log_page(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, const char *message)
{
    static char body[HTML_BUFFER_SIZE];
    static char rows[8192];
    static app_access_log_entry_t entries[ACCESS_LOG_SIZE];
    char timestamp[32];
    size_t rows_len = 0U;
    int count;

    if (!net_admin_is_authenticated())
    {
        return render_light_login_page(server_ptr, packet_ptr, "<div class='card warn'>Autentique-se para ver o log de acesso.</div>");
    }

    count = app_access_log_snapshot(entries, ACCESS_LOG_SIZE);
    rows[0] = '\0';

    if (count <= 0)
    {
        strncpy(rows, "<tr><td colspan='4'>Nenhum evento registrado ainda.</td></tr>", sizeof(rows) - 1U);
        rows[sizeof(rows) - 1U] = '\0';
    }
    else
    {
        for (int i = count - 1; i >= 0; i--)
        {
            net_format_tick_timestamp(entries[i].tick, timestamp, sizeof(timestamp));
            int written = snprintf(&rows[rows_len],
                                   sizeof(rows) - rows_len,
                                   "<tr><td>%s</td><td>%s</td><td>%s</td><td>%s</td></tr>",
                                   timestamp,
                                   net_event_type_text(entries[i].type),
                                   ('\0' != entries[i].data[0]) ? entries[i].data : "-",
                                   ('\0' != entries[i].user[0]) ? entries[i].user : "-");
            if ((written <= 0) || ((size_t) written >= (sizeof(rows) - rows_len)))
            {
                break;
            }
            rows_len += (size_t) written;
        }
    }

    snprintf(body,
             sizeof(body),
             "<p class='muted'>Log em RAM dos acessos e eventos principais.</p>"
             "<div class='table-wrap'><table><tr><th>Timestamp</th><th>Evento</th><th>Dado</th><th>Usuario</th></tr>%s</table></div>",
             rows);

    return render_light_shell(server_ptr, packet_ptr, "Log de acesso", body, message);
}

static UINT render_light_door_page(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, const char *message)
{
    static char body[3072];
    bool light_on = false;
    bool door_open = false;

    if (!net_admin_is_authenticated())
    {
        return render_light_login_page(server_ptr, packet_ptr, "<div class='card warn'>Autentique-se para controlar a porta.</div>");
    }

    app_state_lock();
    light_on = g_app_state.light_on;
    door_open = g_app_state.door_open;
    app_state_unlock();

    snprintf(body,
             sizeof(body),
             "<p class='muted'>Estado da porta: <strong>%s</strong></p>"
             "<p class='muted'>Estado da luz: <strong>%s</strong></p>"
             "<div class='actions'><a class='small' href='/portaon'>Abrir porta</a><a class='small' href='/lampadatoggle'>Alternar luz</a><a class='small secondary' href='/admin_profiles'>Voltar</a></div>",
             door_open ? "aberta" : "fechada",
             light_on ? "ligada" : "desligada");

    return render_light_shell(server_ptr, packet_ptr, "Controle da porta", body, message);
}

static UINT handle_light_login(NX_HTTP_SERVER *server_ptr, const char *form_data)
{
    char pin[16];

    if (!query_get_value(form_data, "pin", pin, sizeof(pin)))
    {
        return light_redirect_with_flash(server_ptr, "/login", "<div class='card warn'>Digite o PIN antes de entrar.</div>");
    }

    if (0 == strcmp(pin, WEB_ADMIN_PIN))
    {
        net_admin_begin_session();
        return light_redirect_with_flash(server_ptr, "/", "<div class='card ok'>Sessao admin iniciada.</div>");
    }

    return light_redirect_with_flash(server_ptr, "/login", "<div class='card warn'>PIN invalido.</div>");
}

static UINT handle_light_add_user(NX_HTTP_SERVER *server_ptr, const char *query)
{
    storage_user_profile_t profile;
    char primary_uid[UID_MAX_LEN];
    char location[64];
    int edit_index = -1;
    bool persist_requested = false;
    bool persist_ok = false;

    if (!net_admin_is_authenticated())
    {
        return light_redirect_with_flash(server_ptr, "/login", "<div class='card warn'>Autentique-se para salvar perfis.</div>");
    }

    if (!profile_from_query(query, &profile, &edit_index))
    {
        if (edit_index >= 0)
        {
            snprintf(location, sizeof(location), "/profile_form?edit=%d", edit_index);
        }
        else
        {
            strncpy(location, "/profile_form", sizeof(location) - 1U);
            location[sizeof(location) - 1U] = '\0';
        }

        return light_redirect_with_flash(server_ptr, location, "<div class='card warn'>Preencha ao menos o nome do perfil.</div>");
    }

    if (storage_profile_upsert(&profile, edit_index))
    {
        if (profile.card_count > 0U)
        {
            strncpy(primary_uid, profile.cards[0], sizeof(primary_uid) - 1U);
            primary_uid[sizeof(primary_uid) - 1U] = '\0';
            app_post_event(EVENT_USER_ADDED, primary_uid);
        }

        persist_requested = storage_persist_now();
        if (persist_requested)
        {
            persist_ok = storage_persist_wait(5U * TX_TIMER_TICKS_PER_SECOND);
        }
        return light_redirect_with_flash(server_ptr,
                                         "/admin_profiles",
                                         persist_ok
                                             ? "<div class='card ok'>Perfil salvo e gravado automaticamente na QSPI.</div>"
                                             : (persist_requested
                                                    ? "<div class='card warn'>Perfil salvo em runtime, mas a gravacao automatica na QSPI ainda nao foi confirmada.</div>"
                                                    : "<div class='card warn'>Perfil salvo em runtime, mas nao foi possivel iniciar a gravacao automatica na QSPI.</div>"));
    }

    if (edit_index >= 0)
    {
        snprintf(location, sizeof(location), "/profile_form?edit=%d", edit_index);
    }
    else
    {
        strncpy(location, "/profile_form", sizeof(location) - 1U);
        location[sizeof(location) - 1U] = '\0';
    }

    return light_redirect_with_flash(server_ptr, location, "<div class='card warn'>Nao foi possivel salvar o perfil. Verifique cartoes duplicados ou campos obrigatorios.</div>");
}

static UINT handle_light_remove_user(NX_HTTP_SERVER *server_ptr, const char *query)
{
    int index = query_get_int(query, "index", -1);
    bool persist_requested = false;

    if (!net_admin_is_authenticated())
    {
        return light_redirect_with_flash(server_ptr, "/login", "<div class='card warn'>Autentique-se para remover perfis.</div>");
    }

    if (index < 0)
    {
        return light_redirect_with_flash(server_ptr, "/admin_profiles", "<div class='card warn'>Indice invalido para remocao.</div>");
    }

    if (storage_profile_remove(index))
    {
        app_post_event(EVENT_USER_REMOVED, NULL);
        persist_requested = storage_persist_now();
        return light_redirect_with_flash(server_ptr,
                                         "/admin_profiles",
                                         persist_requested
                                             ? "<div class='card ok'>Perfil removido e persistencia automatica solicitada.</div>"
                                             : "<div class='card ok'>Perfil removido em runtime. Se quiser, use \"Persistir cadastros\" como fallback.</div>");
    }

    return light_redirect_with_flash(server_ptr, "/admin_profiles", "<div class='card warn'>Perfil nao encontrado.</div>");
}

static UINT handle_light_save_users(NX_HTTP_SERVER *server_ptr)
{
    if (!net_admin_is_authenticated())
    {
        return light_redirect_with_flash(server_ptr, "/login", "<div class='card warn'>Autentique-se para persistir os perfis.</div>");
    }

    if (storage_persist_now())
    {
        return light_redirect_with_flash(server_ptr, "/admin_profiles", "<div class='card ok'>Persistencia solicitada. Aguarde alguns segundos e recarregue a pagina.</div>");
    }

    return light_redirect_with_flash(server_ptr, "/admin_profiles", "<div class='card warn'>Nao foi possivel persistir os cadastros agora.</div>");
}

static UINT handle_light_import_profiles(NX_HTTP_SERVER *server_ptr, const char *form_data)
{
    if (!net_admin_is_authenticated())
    {
        return light_redirect_with_flash(server_ptr, "/login", "<div class='card warn'>Autentique-se para importar perfis.</div>");
    }

    if (!net_queue_import_profiles(form_data))
    {
        return light_redirect_with_flash(server_ptr, "/import", "<div class='card warn'>Cole o JSON de perfis antes de importar.</div>");
    }

    return light_redirect_with_flash(server_ptr, "/admin_profiles", "<div class='card ok'>Importacao agendada. Recarregue a pagina em alguns instantes para ver o resultado.</div>");
}

UINT authentication_check(NX_HTTP_SERVER *server_ptr,
                          UINT request_type,
                          CHAR *resource,
                          CHAR **name,
                          CHAR **password,
                          CHAR **realm)
{
    SSP_PARAMETER_NOT_USED(server_ptr);
    SSP_PARAMETER_NOT_USED(request_type);
    SSP_PARAMETER_NOT_USED(resource);
    SSP_PARAMETER_NOT_USED(name);
    SSP_PARAMETER_NOT_USED(password);
    SSP_PARAMETER_NOT_USED(realm);

    return NX_SUCCESS;
}

static UINT request_notify_impl(NX_HTTP_SERVER *server_ptr, UINT request_type, CHAR *resource, NX_PACKET *packet_ptr)
{
    static char path[RESOURCE_BUFFER_SIZE];
    static char query[QUERY_BUFFER_SIZE];
    static char body[IMPORT_BUFFER_SIZE];
    size_t resource_len;

    g_net_debug_request_count++;
    g_net_debug_http_entry_count++;
    g_net_debug_http_stage = 1U;
    g_net_debug_last_request_type = request_type;
    resource_len = strlen(resource);
    if (resource_len >= sizeof(g_net_debug_last_resource))
    {
        resource_len = sizeof(g_net_debug_last_resource) - 1U;
    }
    memcpy((void *) g_net_debug_last_resource, resource, resource_len);
    g_net_debug_last_resource[resource_len] = '\0';

    /* O NetX Duo corta a string do resource. Pega no caminho limpo */
    strncpy(path, resource, sizeof(path) - 1U);
    path[sizeof(path) - 1U] = '\0';
    {
        char *query_start = strchr(path, '?');
        if (NULL != query_start)
        {
            *query_start = '\0';
        }
    }

    resource_len = strlen(path);
    if (resource_len >= sizeof(g_net_debug_last_path))
    {
        resource_len = sizeof(g_net_debug_last_path) - 1U;
    }
    memcpy((void *) g_net_debug_last_path, path, resource_len);
    g_net_debug_last_path[resource_len] = '\0';

    /* Prefira o resource do NetX para extrair a query. */
    g_net_debug_http_stage = 2U;
    extract_query_from_resource(resource, query, sizeof(query));
    if ((query[0] == '\0') && (NX_HTTP_SERVER_GET_REQUEST != request_type))
    {
        extract_query_from_packet(packet_ptr, query, sizeof(query));
    }
    body[0] = '\0';
    if (NX_HTTP_SERVER_POST_REQUEST == request_type)
    {
        extract_body_from_packet(server_ptr, packet_ptr, body, sizeof(body));
    }

    if (NX_HTTP_SERVER_GET_REQUEST == request_type)
    {
        g_net_debug_http_stage = 3U;
        if (0 == strcmp(path, "/health"))
        {
            return send_plain_response(server_ptr, packet_ptr, "OK");
        }
        if (0 == strcmp(path, "/"))
        {
            return render_light_dashboard_page(server_ptr, packet_ptr, NULL);
        }
        if (0 == strcmp(path, "/login"))
        {
            return render_light_login_page(server_ptr, packet_ptr, NULL);
        }
        if (0 == strcmp(path, "/logout"))
        {
            net_admin_end_session();
            return light_redirect_with_flash(server_ptr, "/", "<div class='card ok'>Sessao admin encerrada.</div>");
        }
        if (0 == strcmp(path, "/profiles"))
        {
            return light_redirect_with_flash(server_ptr, "/admin_profiles", "<div class='card ok'>Rota de perfis atualizada.</div>");
        }
        if (0 == strcmp(path, "/admin_profiles"))
        {
            return render_light_profiles_page(server_ptr, packet_ptr, NULL, query_get_int(query, "page", 0));
        }
        if (0 == strcmp(path, "/profile_form"))
        {
            return render_light_profile_form_page(server_ptr, packet_ptr, NULL, query_get_int(query, "edit", -1));
        }
        if (0 == strcmp(path, "/upload_photo"))
        {
            return render_light_upload_page(server_ptr, packet_ptr, NULL);
        }
        if (0 == strcmp(path, "/upload_photo_script"))
        {
            return render_light_upload_script(server_ptr, packet_ptr);
        }
        if (0 == strcmp(path, "/upload_photo_begin"))
        {
            return handle_light_upload_begin(server_ptr, packet_ptr, query);
        }
        if (0 == strcmp(path, "/upload_photo_chunk"))
        {
            return handle_light_upload_chunk(server_ptr, packet_ptr, query);
        }
        if (0 == strcmp(path, "/upload_photo_commit"))
        {
            return handle_light_upload_commit(server_ptr, packet_ptr);
        }
        if (0 == strcmp(path, "/import"))
        {
            return render_light_import_page(server_ptr, packet_ptr, NULL);
        }
        if (0 == strcmp(path, "/access_log"))
        {
            return render_light_access_log_page(server_ptr, packet_ptr, NULL);
        }
        if (0 == strcmp(path, "/door"))
        {
            return render_light_door_page(server_ptr, packet_ptr, NULL);
        }
        if (0 == strcmp(path, "/add_user"))
        {
            return handle_light_add_user(server_ptr, query);
        }
        if (0 == strcmp(path, "/remove_user"))
        {
            return handle_light_remove_user(server_ptr, query);
        }
        if (0 == strcmp(path, "/attach_photo"))
        {
            return handle_attach_photo(server_ptr, packet_ptr, query);
        }
        if (0 == strcmp(path, "/save_users"))
        {
            return handle_light_save_users(server_ptr);
        }
        if (0 == strcmp(path, "/portaon"))
        {
            if (!net_admin_is_authenticated())
            {
                return light_redirect_with_flash(server_ptr, "/login", "<div class='card warn'>Autentique-se para controlar a porta.</div>");
            }
            app_post_event(EVENT_DOOR_OPEN, "WEB");
            return light_redirect_with_flash(server_ptr, "/door", "<div class='card ok'>Comando de abertura enviado.</div>");
        }
        if (0 == strcmp(path, "/lampadatoggle"))
        {
            if (!net_admin_is_authenticated())
            {
                return light_redirect_with_flash(server_ptr, "/login", "<div class='card warn'>Autentique-se para controlar a luz.</div>");
            }
            app_state_lock();
            bool current_light = g_app_state.light_on;
            app_state_unlock();

            app_post_event(current_light ? EVENT_LIGHT_OFF : EVENT_LIGHT_ON, NULL);
            return light_redirect_with_flash(server_ptr, "/door", "<div class='card ok'>Comando de luz enviado.</div>");
        }
        if (0 == strcmp(path, "/get_last_uid"))
        {
            char uid_text[UID_MAX_LEN] = "";

            app_state_lock();
            strncpy(uid_text, (const char *) g_app_state.last_uid, sizeof(uid_text) - 1U);
            uid_text[sizeof(uid_text) - 1U] = '\0';
            app_state_unlock();

            return send_plain_response(server_ptr, packet_ptr, uid_text);
        }
    }

    if (NX_HTTP_SERVER_POST_REQUEST == request_type)
    {
        g_net_debug_http_stage = 4U;
        if (0 == strcmp(path, "/login"))
        {
            return handle_light_login(server_ptr, body);
        }
        if (0 == strcmp(path, "/upload_photo_begin"))
        {
            return handle_light_upload_begin(server_ptr, packet_ptr, body);
        }
        if (0 == strcmp(path, "/upload_photo_chunk"))
        {
            return handle_light_upload_chunk(server_ptr, packet_ptr, body);
        }
        if (0 == strcmp(path, "/upload_photo_commit"))
        {
            return handle_light_upload_commit(server_ptr, packet_ptr);
        }
        if (0 == strcmp(path, "/add_user"))
        {
            if ('\0' != body[0])
            {
                return handle_light_add_user(server_ptr, body);
            }

            return handle_light_add_user(server_ptr, query);
        }
        if (0 == strcmp(path, "/import_profiles"))
        {
            return handle_light_import_profiles(server_ptr, body);
        }
    }

    g_net_debug_http_stage = 5U;
    return send_plain_response(server_ptr, packet_ptr, "Not Found");
}

UINT request_notify(NX_HTTP_SERVER *server_ptr, UINT request_type, CHAR *resource, NX_PACKET *packet_ptr)
{
    UINT status;

    g_net_debug_http_inflight = 1U;
    net_http_lock();
    status = request_notify_impl(server_ptr, request_type, resource, packet_ptr);
    net_http_unlock();
    g_net_debug_http_inflight = 0U;
    g_net_debug_http_exit_count++;
    g_net_debug_http_stage = 6U;

    return status;
}

void thread_net_entry(ULONG arg)
{
    SSP_PARAMETER_NOT_USED(arg);

    ULONG actual_status = 0U;
    UINT status;

    network_stack_init_once();
    storage_init();
    g_net_debug_state = 3U;

    if (NX_SUCCESS == nx_ip_status_check(&g_ip0, NX_IP_LINK_ENABLED, &actual_status, TX_WAIT_FOREVER))
    {
        g_net_debug_link_status = actual_status;
        g_net_debug_state = 4U;
        status = nx_ip_address_set(&g_ip0, DEVICE_IP_ADDR, DEVICE_NETMASK);
        g_net_debug_ip_status = status;
        if (NX_SUCCESS == status)
        {
            ULONG ip_address = 0U;
            ULONG network_mask = 0U;

            (void) nx_ip_gateway_address_set(&g_ip0, DEVICE_GATEWAY_ADDR);
            (void) nx_ip_address_get(&g_ip0, &ip_address, &network_mask);
            g_net_debug_ip_address = ip_address;
            g_net_debug_network_mask = network_mask;
            g_net_debug_state = 5U;

            app_state_lock();
            g_app_state.net_ready = true;
            app_state_unlock();

            g_net_debug_http_status = nx_http_server_start(&g_http_server0);
            g_net_debug_state = (NX_SUCCESS == g_net_debug_http_status) ? 6U : 0xEEU;
        }
    }

    while (1)
    {
        ULONG bytes_sent = 0U;
        ULONG bytes_received = 0U;
        ULONG invalid_packets = 0U;
        ULONG packets_dropped_rx = 0U;
        ULONG checksum_errors = 0U;
        ULONG packets_dropped_tx = 0U;
        ULONG fragments_sent = 0U;
        ULONG fragments_received = 0U;
        ULONG arp_dynamic_entries = 0U;
        ULONG arp_static_entries = 0U;
        ULONG arp_aged_entries = 0U;
        ULONG arp_invalid_messages = 0U;
        ULONG ip_packets_sent = 0U;
        ULONG ip_packets_received = 0U;
        ULONG total_packets = 0U;
        ULONG free_packets = 0U;
        ULONG empty_pool_requests = 0U;
        ULONG empty_pool_suspensions = 0U;
        ULONG invalid_packet_releases = 0U;

        net_process_pending_actions();

        (void) nx_ip_info_get(&g_ip0,
                              &ip_packets_sent,
                              &bytes_sent,
                              &ip_packets_received,
                              &bytes_received,
                              &invalid_packets,
                              &packets_dropped_rx,
                              &checksum_errors,
                              &packets_dropped_tx,
                              &fragments_sent,
                              &fragments_received);

        g_net_debug_ip_packets_sent = ip_packets_sent;
        g_net_debug_ip_packets_received = ip_packets_received;
        g_net_debug_ip_invalid_packets = invalid_packets;
        g_net_debug_ip_receive_packets_dropped = packets_dropped_rx;
        g_net_debug_ip_receive_checksum_errors = checksum_errors;

        (void) nx_arp_info_get(&g_ip0,
                               (ULONG *) &g_net_debug_arp_requests_sent,
                               (ULONG *) &g_net_debug_arp_requests_received,
                               (ULONG *) &g_net_debug_arp_responses_sent,
                               (ULONG *) &g_net_debug_arp_responses_received,
                               &arp_dynamic_entries,
                               &arp_static_entries,
                               &arp_aged_entries,
                               &arp_invalid_messages);

        g_net_debug_arp_invalid_messages = arp_invalid_messages;
        g_net_debug_driver_irq = (ULONG) nx_record1.irq;
        g_net_debug_driver_irq_enabled = (ULONG) __NVIC_GetEnableIRQ(nx_record1.irq);
        g_net_debug_driver_state = (ULONG) nx_record1.nx_state;
        g_net_debug_driver_link_established = (ULONG) nx_record1.link_established;
        g_net_debug_driver_rx_bd_index = nx_record1.driver_rx_bd_index;
        g_net_debug_driver_rx_bd0_status = (NULL != nx_record1.driver_rx_bd) ? nx_record1.driver_rx_bd[0].bd_status : 0U;
        g_net_debug_driver_tx_bd0_status = (NULL != nx_record1.driver_tx_bd) ? nx_record1.driver_tx_bd[0].bd_status : 0U;

        if ((NULL != g_http_server0.nx_http_server_packet_pool_ptr) &&
            (NX_SUCCESS == nx_packet_pool_info_get(g_http_server0.nx_http_server_packet_pool_ptr,
                                                   &total_packets,
                                                   &free_packets,
                                                   &empty_pool_requests,
                                                   &empty_pool_suspensions,
                                                   &invalid_packet_releases)))
        {
            SSP_PARAMETER_NOT_USED(total_packets);
            SSP_PARAMETER_NOT_USED(invalid_packet_releases);
            g_net_debug_packet_pool_available = free_packets;
            g_net_debug_packet_pool_empty_requests = empty_pool_requests;
            g_net_debug_packet_pool_empty_suspensions = empty_pool_suspensions;
        }

        tx_thread_sleep(50);
    }
}

void thread_tunnel_entry(ULONG arg)
{
    SSP_PARAMETER_NOT_USED(arg);
    NX_TCP_SOCKET tunnel_socket;
    NX_PACKET *receive_packet;

    while (1)
    {
        app_state_lock();
        bool ready = g_app_state.net_ready;
        app_state_unlock();
        if (ready)
        {
            break;
        }
        tx_thread_sleep(100);
    }

    nx_tcp_socket_create(&g_ip0,
                         &tunnel_socket,
                         "Tunnel Socket",
                         NX_IP_NORMAL,
                         NX_FRAGMENT_OKAY,
                         NX_IP_TIME_TO_LIVE,
                         512,
                         NX_NULL,
                         NX_NULL);

    while (1)
    {
        nx_tcp_client_socket_bind(&tunnel_socket, NX_ANY_PORT, TX_WAIT_FOREVER);
        UINT status = nx_tcp_client_socket_connect(&tunnel_socket, RELAY_IP_ADDR, RELAY_PORT, 500);

        if (NX_SUCCESS == status)
        {
            char req[256];

            snprintf(req,
                     sizeof(req),
                     "GET /tunnel?device_id=%s HTTP/1.1\r\nHost: %s\r\nX-API-KEY: %s\r\nConnection: keep-alive\r\n\r\n",
                     DEVICE_ID,
                     RELAY_HOST,
                     API_KEY);

            NX_PACKET *send_packet;
            nx_packet_allocate(&g_packet_pool0, &send_packet, NX_TCP_PACKET, NX_WAIT_FOREVER);
            nx_packet_data_append(send_packet, (VOID *) req, strlen(req), &g_packet_pool0, NX_WAIT_FOREVER);
            nx_tcp_socket_send(&tunnel_socket, send_packet, 500);

            while (NX_SUCCESS == nx_tcp_socket_receive(&tunnel_socket, &receive_packet, 500))
            {
                ULONG bytes_read;
                char buffer[128];
                ULONG copy_len;

                nx_packet_length_get(receive_packet, &bytes_read);
                copy_len = (bytes_read < 127U) ? bytes_read : 127U;
                nx_packet_data_extract_offset(receive_packet, 0, buffer, copy_len, &bytes_read);
                buffer[copy_len] = '\0';

                if (NULL != strstr(buffer, "\"portaon\""))
                {
                    app_post_event(EVENT_DOOR_OPEN, "TUNNEL");
                }
                else if (NULL != strstr(buffer, "\"lampadaon\""))
                {
                    app_post_event(EVENT_LIGHT_ON, NULL);
                }
                else if (NULL != strstr(buffer, "\"lampadaoff\""))
                {
                    app_post_event(EVENT_LIGHT_OFF, NULL);
                }

                nx_packet_release(receive_packet);
            }

            nx_tcp_socket_disconnect(&tunnel_socket, 200);
        }

        nx_tcp_client_socket_unbind(&tunnel_socket);
        tx_thread_sleep(200);
    }
}

void thread_status_entry(ULONG arg)
{
    SSP_PARAMETER_NOT_USED(arg);
    NX_TCP_SOCKET status_socket;

    while (1)
    {
        app_state_lock();
        bool ready = g_app_state.net_ready;
        app_state_unlock();
        if (ready)
        {
            break;
        }
        tx_thread_sleep(100);
    }

    nx_tcp_socket_create(&g_ip0,
                         &status_socket,
                         "Status Socket",
                         NX_IP_NORMAL,
                         NX_FRAGMENT_OKAY,
                         NX_IP_TIME_TO_LIVE,
                         512,
                         NX_NULL,
                         NX_NULL);

    while (1)
    {
        nx_tcp_client_socket_bind(&status_socket, NX_ANY_PORT, TX_WAIT_FOREVER);
        if (NX_SUCCESS == nx_tcp_client_socket_connect(&status_socket, RELAY_IP_ADDR, RELAY_PORT, 200))
        {
            bool p_stat;
            bool l_stat;
            char payload[128];
            char req[256];
            NX_PACKET *send_packet;

            app_state_lock();
            p_stat = g_app_state.door_open;
            l_stat = g_app_state.light_on;
            app_state_unlock();

            snprintf(payload,
                     sizeof(payload),
                     "{\"device_id\":\"%s\",\"door\":%d,\"light\":%d}",
                     DEVICE_ID,
                     p_stat ? 1 : 0,
                     l_stat ? 1 : 0);

            snprintf(req,
                     sizeof(req),
                     "POST /status HTTP/1.1\r\nHost: %s\r\nX-API-KEY: %s\r\nContent-Type: application/json\r\nContent-Length: %d\r\n\r\n%s",
                     RELAY_HOST,
                     API_KEY,
                     (int) strlen(payload),
                     payload);

            if (NX_SUCCESS == nx_packet_allocate(&g_packet_pool0, &send_packet, NX_TCP_PACKET, NX_WAIT_FOREVER))
            {
                nx_packet_data_append(send_packet, (VOID *) req, strlen(req), &g_packet_pool0, NX_WAIT_FOREVER);
                nx_tcp_socket_send(&status_socket, send_packet, 200);
            }

            nx_tcp_socket_disconnect(&status_socket, 200);
        }

        nx_tcp_client_socket_unbind(&status_socket);
        tx_thread_sleep(1000);
    }
}
