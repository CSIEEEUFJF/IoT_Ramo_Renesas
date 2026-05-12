#include "net.h"
#include "main.h"
#include "storage.h"
#include "ui.h"
#include "assets/profile_photo_ids.h"
#include "nxd_dhcp_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

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
#define STORAGE_EXPORT_PAGE_SIZE    5
#define ACCESS_LOG_PAGE_SIZE       10
#define MEETING_MODE_PAGE_SIZE      4
#define METRICS_PAGE_SIZE           4
#define PHOTO_OPTIONS_BUFFER_SIZE 2048
#define PROFILE_OPTIONS_BUFFER_SIZE 4096
#define METRIC_BUFFER_SIZE 8192
#define METRIC_GROUP_MAX 24
#define NET_API_JSON_BUFFER_SIZE 1536
#define NET_MEETING_SCHEDULE_STATUS_LEN 32
#define NET_MEETING_SCHEDULE_MAX_DELAY_SECONDS (24UL * 60UL * 60UL)
#define UPLOAD_IMAGE_DIM        160U
#define UPLOAD_TILE_DIM         40U
#define UPLOAD_TILE_COUNT_X     (UPLOAD_IMAGE_DIM / UPLOAD_TILE_DIM)
#define UPLOAD_TILE_COUNT_Y     (UPLOAD_IMAGE_DIM / UPLOAD_TILE_DIM)
#define UPLOAD_TILE_COUNT       (UPLOAD_TILE_COUNT_X * UPLOAD_TILE_COUNT_Y)
#define UPLOAD_TILE_PIXEL_BYTES (UPLOAD_TILE_DIM * UPLOAD_TILE_DIM * 2U)
#define UPLOAD_TILE_B64_BUFFER_SIZE ((UPLOAD_TILE_PIXEL_BYTES * 4U / 3U) + 128U)
#define HTTP_SEND_CHUNK_SIZE      512U
#define NET_DOWNLOAD_CHUNK_SIZE  1024U
#define NET_HTTP_PACKET_WAIT_TICKS (TX_TIMER_TICKS_PER_SECOND / 4U)
#define NET_DHCP_WAIT_SLICE_TICKS (TX_TIMER_TICKS_PER_SECOND / 2U)
#define NET_DHCP_MAX_WAIT_TICKS   (TX_TIMER_TICKS_PER_SECOND * 30U)
#define NET_NTP_PORT               123U
#define NET_NTP_PACKET_SIZE         48U
#define NET_NTP_UNIX_EPOCH_DELTA 2208988800UL
#define NET_NTP_QUERY_TIMEOUT_TICKS (3U * TX_TIMER_TICKS_PER_SECOND)
#define NET_NTP_RETRY_INTERVAL_TICKS (60U * TX_TIMER_TICKS_PER_SECOND)
#define NET_NTP_RESYNC_INTERVAL_TICKS (6U * 60U * 60U * TX_TIMER_TICKS_PER_SECOND)
#define NET_API_DOOR_COOLDOWN_TICKS (2U * TX_TIMER_TICKS_PER_SECOND)
#define WEB_ADMIN_PIN           "1234"
#define WEB_ADMIN_SESSION_TICKS (10U * 60U * TX_TIMER_TICKS_PER_SECOND)
#define WEB_ADMIN_LOGIN_MAX_FAILURES 5U
#define WEB_ADMIN_LOGIN_BLOCK_TICKS  (60U * TX_TIMER_TICKS_PER_SECOND)

typedef struct st_net_metric_group
{
    app_metric_kind_t kind;
    app_metric_case_t case_id;
    int count;
    int failures;
    uint64_t sum_ticks;
    ULONG min_ticks;
    ULONG max_ticks;
} net_metric_group_t;

static char g_net_flash_message[512] = {0};
static ULONG g_net_admin_session_deadline = 0U;
static ULONG g_net_admin_session_ip = 0U;
static ULONG g_net_current_request_ip = 0U;
static ULONG g_net_admin_login_block_until = 0U;
static ULONG g_net_api_door_next_allowed_tick = 0U;
static unsigned int g_net_admin_login_failures = 0U;
static bool g_net_action_mutex_ready = false;
static TX_MUTEX g_net_action_mutex;
static bool g_net_http_mutex_ready = false;
static TX_MUTEX g_net_http_mutex;
static bool g_net_dhcp_created = false;
static NX_DHCP g_net_dhcp_client;
static ULONG g_net_ntp_last_attempt_tick = 0U;
static ULONG g_net_ntp_last_success_tick = 0U;
static bool g_net_import_pending = false;
static bool g_net_import_processing = false;
static char g_net_pending_import_json[IMPORT_BUFFER_SIZE];
static bool g_net_upload_session_active = false;
static int g_net_pending_upload_profile_index = -1;
static int g_net_pending_upload_width = (int) UPLOAD_IMAGE_DIM;
static int g_net_pending_upload_height = (int) UPLOAD_IMAGE_DIM;
static uint32_t g_net_pending_upload_tile_mask = 0U;
static char g_net_pending_upload_photo_id[STORAGE_PHOTO_ID_MAX_LEN];
static bool g_net_meeting_draft_initialized = false;
static bool g_net_meeting_draft_selected[STORAGE_MAX_USERS];
static bool g_net_meeting_schedules_loaded = false;
static storage_meeting_schedule_t g_net_meeting_schedules[STORAGE_MEETING_SCHEDULE_MAX_ITEMS];
static int g_net_meeting_schedule_count = 0;
static ULONG g_net_meeting_schedule_next_id = 1U;
static ULONG g_net_meeting_schedule_last_id = 0U;
static ULONG g_net_meeting_schedule_last_start_utc = 0U;
static unsigned int g_net_meeting_schedule_last_selected = 0U;
static unsigned int g_net_meeting_schedule_last_allowed = 0U;
static char g_net_meeting_schedule_last_status[NET_MEETING_SCHEDULE_STATUS_LEN] = "idle";
static app_metric_entry_t g_net_metric_snapshot[APP_METRIC_LOG_SIZE];
static net_metric_group_t g_net_metric_groups[METRIC_GROUP_MAX];
static ULONG g_net_metric_durations[APP_METRIC_LOG_SIZE];
static char g_net_metric_buffer[METRIC_BUFFER_SIZE];
static uint8_t g_net_download_chunk[NET_DOWNLOAD_CHUNK_SIZE];
static char g_net_api_json[NET_API_JSON_BUFFER_SIZE];

static UINT send_html_response(NX_HTTP_SERVER *server, NX_PACKET *packet_ptr, const char *html);
static UINT send_plain_response(NX_HTTP_SERVER *server, NX_PACKET *packet_ptr, const char *text);
static UINT send_javascript_response(NX_HTTP_SERVER *server, NX_PACKET *packet_ptr, const char *script);
static UINT send_buffer_response(NX_HTTP_SERVER *server, NX_PACKET *packet_ptr, const void *data, size_t data_len, const char *content_type);
static UINT send_buffer_status_response(NX_HTTP_SERVER *server, NX_PACKET *packet_ptr, const char *status_code, const void *data, size_t data_len, const char *content_type);
static UINT send_stream_response_header(NX_HTTP_SERVER *server, NX_PACKET *packet_ptr, ULONG data_len, const char *content_type);
static UINT send_plain_response_fresh(NX_HTTP_SERVER *server, const char *text);
static UINT send_plain_status_response(NX_HTTP_SERVER *server, NX_PACKET *packet_ptr, const char *status_code, const char *text);
static UINT send_callback_response(NX_HTTP_SERVER *server, const char *status_code, const char *information);
static UINT send_redirect(NX_HTTP_SERVER *server, const char *location);
static UINT light_redirect_with_flash(NX_HTTP_SERVER *server_ptr, const char *location, const char *message);
static int net_format_metric_csv_line(char *out, size_t out_size, const app_metric_entry_t *entry);
static bool net_request_source_ip(NX_PACKET *packet_ptr, ULONG *out_ip);
static bool extract_header_from_packet(NX_PACKET *packet_ptr, const char *header_name, char *out, size_t out_size);
static bool net_api_request_is_authorized(NX_PACKET *packet_ptr);
static UINT handle_api_door_open(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr);
static UINT handle_api_meeting_schedule(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, const char *body);
static UINT handle_api_meeting_cancel(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, const char *body, const char *query);
static UINT handle_api_meeting_status(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr);
static void net_process_meeting_schedule(void);
static void net_html_escape(const char *src, char *out, size_t out_size);

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

static bool net_request_source_ip(NX_PACKET *packet_ptr, ULONG *out_ip)
{
    NX_IPV4_HEADER *ip_header;

    if (NULL != out_ip)
    {
        *out_ip = 0U;
    }

    if ((NULL == packet_ptr) ||
        (NULL == out_ip) ||
        (NX_IP_VERSION_V4 != packet_ptr->nx_packet_ip_version) ||
        (NULL == packet_ptr->nx_packet_ip_header))
    {
        return false;
    }

    ip_header = (NX_IPV4_HEADER *) packet_ptr->nx_packet_ip_header;
    *out_ip = ip_header->nx_ip_header_source_ip;
    return (0U != *out_ip);
}

static bool net_admin_is_authenticated(void)
{
    if (tx_time_get() >= g_net_admin_session_deadline)
    {
        return false;
    }

    return true;
}

static void net_admin_begin_session(void)
{
    g_net_admin_session_deadline = tx_time_get() + WEB_ADMIN_SESSION_TICKS;
    g_net_admin_session_ip = 0U;
}

static void net_admin_end_session(void)
{
    g_net_admin_session_deadline = 0U;
    g_net_admin_session_ip = 0U;
}

static const char *net_metric_http_case_for_path(const char *path)
{
    if (NULL == path)
    {
        return NULL;
    }

    if (0 == strcmp(path, "/login"))
    {
        return "/login";
    }
    if ((0 == strcmp(path, "/admin_profiles")) ||
        (0 == strncmp(path, "/admin_profiles/", strlen("/admin_profiles/"))))
    {
        return "/admin_profiles";
    }
    if (0 == strcmp(path, "/import_profiles"))
    {
        return "/import_profiles";
    }
    if ((0 == strcmp(path, "/upload_photo")) ||
        (0 == strcmp(path, "/upload_photo_begin")) ||
        (0 == strcmp(path, "/upload_photo_chunk")) ||
        (0 == strcmp(path, "/upload_photo_commit")))
    {
        return "/upload_photo";
    }
    if ((0 == strcmp(path, "/metrics")) ||
        (0 == strcmp(path, "/metrics_download")) ||
        (0 == strncmp(path, "/metrics/", strlen("/metrics/"))))
    {
        return "/metrics";
    }

    return NULL;
}

static const char *net_metric_import_case_for_count(int imported_count)
{
    if (imported_count <= 10)
    {
        return "10 perfis";
    }
    if (imported_count <= 50)
    {
        return "50 perfis";
    }
    return "50+ perfis";
}

static uint64_t net_isqrt64(uint64_t value)
{
    uint64_t result = 0U;
    uint64_t bit = (uint64_t) 1U << 62;

    while (bit > value)
    {
        bit >>= 2;
    }

    while (bit != 0U)
    {
        if (value >= (result + bit))
        {
            value -= (result + bit);
            result = (result >> 1U) + bit;
        }
        else
        {
            result >>= 1U;
        }
        bit >>= 2U;
    }

    return result;
}

static uint64_t net_ticks_to_ms_x10(ULONG ticks)
{
    return ((uint64_t) ticks * 10000ULL) / (uint64_t) TX_TIMER_TICKS_PER_SECOND;
}

static void net_format_ms_x10(uint64_t value_x10, char *out, size_t out_size)
{
    if ((NULL == out) || (0U == out_size))
    {
        return;
    }

    snprintf(out,
             out_size,
             "%llu.%llums",
             (unsigned long long) (value_x10 / 10ULL),
             (unsigned long long) (value_x10 % 10ULL));
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

static void net_build_profile_options(char *out, size_t out_size)
{
    size_t offset = 0U;
    int count;

    if ((NULL == out) || (0U == out_size))
    {
        if ((NULL != out) && (out_size > 0U))
        {
            out[0] = '\0';
        }
        return;
    }

    out[0] = '\0';
    count = storage_user_count_nowait();
    if (count > STORAGE_MAX_USERS)
    {
        count = STORAGE_MAX_USERS;
    }

    for (int i = 0; i < count; i++)
    {
        storage_user_profile_t profile;
        char name_html[NAME_MAX_LEN * 6];
        int written;

        if (!storage_profile_get_nowait(i, &profile))
        {
            continue;
        }

        net_html_escape(profile.name, name_html, sizeof(name_html));
        written = snprintf(&out[offset],
                           out_size - offset,
                           "<option value='%d'>%s%s</option>",
                           i,
                           profile.is_admin ? "[ADM] " : "",
                           name_html);

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
    UINT status = NX_SUCCESS;

    if (initialized)
    {
        return;
    }

    g_net_debug_state = 1U;
    net_action_init_once();
    net_http_init_once();
    packet_pool_init0();
    ip_init0();
    status = nx_dhcp_create(&g_net_dhcp_client, &g_ip0, "g_net_dhcp_client");
    if (NX_SUCCESS == status)
    {
        g_net_dhcp_created = true;
        (void) nx_dhcp_user_option_request(&g_net_dhcp_client, NX_DHCP_OPTION_NTP_SVR);
    }
    g_net_debug_ip_status = status;
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

static bool net_wait_for_dhcp_address(ULONG *out_ip_address, ULONG *out_network_mask)
{
    ULONG waited = 0U;
    ULONG ip_address = 0U;
    ULONG network_mask = 0U;

    while (waited < NET_DHCP_MAX_WAIT_TICKS)
    {
        (void) nx_ip_address_get(&g_ip0, &ip_address, &network_mask);
        g_net_debug_ip_address = ip_address;
        g_net_debug_network_mask = network_mask;

        if (0U != ip_address)
        {
            if (NULL != out_ip_address)
            {
                *out_ip_address = ip_address;
            }

            if (NULL != out_network_mask)
            {
                *out_network_mask = network_mask;
            }

            return true;
        }

        tx_thread_sleep(NET_DHCP_WAIT_SLICE_TICKS);
        waited += NET_DHCP_WAIT_SLICE_TICKS;
    }

    return false;
}

static bool net_get_dhcp_ntp_server(ULONG *out_ip_address)
{
    UCHAR option_data[16];
    UINT option_size = sizeof(option_data);
    UINT status;

    if ((NULL == out_ip_address) || !g_net_dhcp_created)
    {
        return false;
    }

    status = nx_dhcp_user_option_retrieve(&g_net_dhcp_client,
                                          NX_DHCP_OPTION_NTP_SVR,
                                          option_data,
                                          &option_size);
    if ((NX_SUCCESS != status) || (option_size < 4U))
    {
        return false;
    }

    *out_ip_address = ((ULONG) option_data[0] << 24) |
                      ((ULONG) option_data[1] << 16) |
                      ((ULONG) option_data[2] << 8) |
                      (ULONG) option_data[3];
    return (0U != *out_ip_address);
}

static bool net_query_ntp_server(ULONG server_ip_address, ULONG *out_unix_utc)
{
    NX_UDP_SOCKET socket;
    NX_PACKET *send_packet = NX_NULL;
    NX_PACKET *receive_packet = NX_NULL;
    UCHAR request[NET_NTP_PACKET_SIZE] = {0};
    UCHAR response[NET_NTP_PACKET_SIZE] = {0};
    ULONG bytes_copied = 0U;
    UINT local_port = 0U;
    UINT status;
    bool ok = false;

    if ((0U == server_ip_address) || (NULL == out_unix_utc))
    {
        return false;
    }

    status = nx_udp_socket_create(&g_ip0,
                                  &socket,
                                  "ntp_socket",
                                  NX_IP_NORMAL,
                                  NX_FRAGMENT_OKAY,
                                  NX_IP_TIME_TO_LIVE,
                                  4U);
    if (NX_SUCCESS != status)
    {
        return false;
    }

    status = nx_udp_free_port_find(&g_ip0, 49152U, &local_port);
    if (NX_SUCCESS != status)
    {
        (void) nx_udp_socket_delete(&socket);
        return false;
    }

    status = nx_udp_socket_bind(&socket, local_port, NET_HTTP_PACKET_WAIT_TICKS);
    if (NX_SUCCESS != status)
    {
        (void) nx_udp_socket_delete(&socket);
        return false;
    }

    request[0] = 0x1BU;

    status = nx_packet_allocate(&g_packet_pool0, &send_packet, NX_UDP_PACKET, NET_HTTP_PACKET_WAIT_TICKS);
    if (NX_SUCCESS == status)
    {
        status = nx_packet_data_append(send_packet,
                                       request,
                                       sizeof(request),
                                       &g_packet_pool0,
                                       NET_HTTP_PACKET_WAIT_TICKS);
    }

    if (NX_SUCCESS == status)
    {
        status = nx_udp_socket_send(&socket, send_packet, server_ip_address, NET_NTP_PORT);
        if (NX_SUCCESS == status)
        {
            send_packet = NX_NULL;
        }
    }

    if (NX_SUCCESS == status)
    {
        status = nx_udp_socket_receive(&socket, &receive_packet, NET_NTP_QUERY_TIMEOUT_TICKS);
    }

    if ((NX_SUCCESS == status) && (NX_NULL != receive_packet))
    {
        status = nx_packet_data_extract_offset(receive_packet,
                                               0U,
                                               response,
                                               sizeof(response),
                                               &bytes_copied);
        if ((NX_SUCCESS == status) && (bytes_copied >= NET_NTP_PACKET_SIZE))
        {
            ULONG ntp_seconds = ((ULONG) response[40] << 24) |
                                ((ULONG) response[41] << 16) |
                                ((ULONG) response[42] << 8) |
                                (ULONG) response[43];

            if (ntp_seconds > NET_NTP_UNIX_EPOCH_DELTA)
            {
                *out_unix_utc = ntp_seconds - NET_NTP_UNIX_EPOCH_DELTA;
                ok = true;
            }
        }
    }

    if (NX_NULL != receive_packet)
    {
        nx_packet_release(receive_packet);
    }
    if (NX_NULL != send_packet)
    {
        nx_packet_release(send_packet);
    }

    (void) nx_udp_socket_unbind(&socket);
    (void) nx_udp_socket_delete(&socket);
    return ok;
}

static bool net_sync_time_with_ntp(void)
{
    ULONG ntp_server_ip = 0U;
    const ULONG fallback_servers[] = {
        IP_ADDRESS(129, 6, 15, 28),
        IP_ADDRESS(129, 6, 15, 29),
    };
    ULONG unix_utc = 0U;

    g_net_ntp_last_attempt_tick = tx_time_get();

    if (net_get_dhcp_ntp_server(&ntp_server_ip) && net_query_ntp_server(ntp_server_ip, &unix_utc))
    {
        app_time_set_utc(unix_utc);
        g_net_ntp_last_success_tick = tx_time_get();
        return true;
    }

    for (size_t i = 0U; i < (sizeof(fallback_servers) / sizeof(fallback_servers[0])); i++)
    {
        if (net_query_ntp_server(fallback_servers[i], &unix_utc))
        {
            app_time_set_utc(unix_utc);
            g_net_ntp_last_success_tick = tx_time_get();
            return true;
        }
    }

    return false;
}

static void net_service_ntp(void)
{
    ULONG now = tx_time_get();
    ULONG wait_ticks = (0U == g_net_ntp_last_success_tick) ? NET_NTP_RETRY_INTERVAL_TICKS
                                                           : NET_NTP_RESYNC_INTERVAL_TICKS;

    if ((0U != g_net_ntp_last_attempt_tick) && ((now - g_net_ntp_last_attempt_tick) < wait_ticks))
    {
        return;
    }

    (void) net_sync_time_with_ntp();
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

static void net_html_escape(const char *src, char *out, size_t out_size)
{
    size_t offset = 0U;

    if ((NULL == out) || (0U == out_size))
    {
        return;
    }

    out[0] = '\0';
    if (NULL == src)
    {
        return;
    }

    while (('\0' != *src) && (offset + 1U < out_size))
    {
        const char *replacement = NULL;

        switch (*src)
        {
            case '&': replacement = "&amp;"; break;
            case '<': replacement = "&lt;"; break;
            case '>': replacement = "&gt;"; break;
            case '"': replacement = "&quot;"; break;
            case '\'': replacement = "&#39;"; break;
            default: break;
        }

        if (NULL != replacement)
        {
            size_t replacement_len = strlen(replacement);
            if ((offset + replacement_len) >= out_size)
            {
                break;
            }

            memcpy(&out[offset], replacement, replacement_len);
            offset += replacement_len;
        }
        else
        {
            out[offset++] = *src;
        }

        src++;
    }

    out[offset] = '\0';
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

static int net_path_get_index_after_prefix(const char *path, const char *prefix, int default_value)
{
    const char *suffix;
    char *end_ptr = NULL;
    long value;

    if ((NULL == path) || (NULL == prefix))
    {
        return default_value;
    }

    if (0 != strncmp(path, prefix, strlen(prefix)))
    {
        return default_value;
    }

    suffix = path + strlen(prefix);
    if ('\0' == *suffix)
    {
        return default_value;
    }

    value = strtol(suffix, &end_ptr, 10);
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

static void net_format_access_log_timestamp(const app_access_log_entry_t *entry, char *out, size_t out_size)
{
    if ((NULL == out) || (0U == out_size))
    {
        return;
    }

    if (NULL == entry)
    {
        out[0] = '\0';
        return;
    }

    app_format_access_log_timestamp(entry, out, out_size);
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

static bool json_copy_string_value(char *out,
                                   size_t out_size,
                                   const char *start,
                                   const char *limit,
                                   const char **out_end)
{
    size_t write_index = 0U;
    const char *cursor = start;

    if ((NULL == out) || (0U == out_size) || (NULL == start) || (NULL == limit) || (start > limit))
    {
        return false;
    }

    out[0] = '\0';
    while (cursor < limit)
    {
        char ch = *cursor++;

        if ('"' == ch)
        {
            out[write_index] = '\0';
            if (NULL != out_end)
            {
                *out_end = cursor - 1;
            }
            return true;
        }

        if (('\\' == ch) && (cursor < limit))
        {
            char escaped = *cursor++;

            switch (escaped)
            {
                case '"':
                case '\\':
                case '/':
                    ch = escaped;
                    break;

                case 'n':
                    ch = '\n';
                    break;

                case 'r':
                    ch = '\r';
                    break;

                case 't':
                    ch = '\t';
                    break;

                default:
                    ch = escaped;
                    break;
            }
        }

        if (write_index + 1U < out_size)
        {
            out[write_index++] = ch;
        }
    }

    out[0] = '\0';
    return false;
}

static const char *json_find_object_end(const char *object_start, const char *limit)
{
    const char *cursor = object_start;
    unsigned int brace_depth = 0U;
    bool in_string = false;
    bool escaped = false;

    if ((NULL == object_start) || (NULL == limit) || (object_start >= limit) || ('{' != *object_start))
    {
        return NULL;
    }

    while (cursor < limit)
    {
        char ch = *cursor++;

        if (in_string)
        {
            if (escaped)
            {
                escaped = false;
            }
            else if ('\\' == ch)
            {
                escaped = true;
            }
            else if ('"' == ch)
            {
                in_string = false;
            }
            continue;
        }

        if ('"' == ch)
        {
            in_string = true;
        }
        else if ('{' == ch)
        {
            brace_depth++;
        }
        else if ('}' == ch)
        {
            if (0U == brace_depth)
            {
                return NULL;
            }
            brace_depth--;
            if (0U == brace_depth)
            {
                return cursor - 1;
            }
        }
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
    profile->is_admin = (query_get_int(query, "is_admin", 0) != 0);
    (void) query_get_value(query, "admin_pin", profile->admin_pin, sizeof(profile->admin_pin));

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
    if (!json_copy_string_value(out, out_size, start, object_end, &end))
    {
        out[0] = '\0';
        return false;
    }

    return true;
}

static bool import_extract_json_bool(const char *object_start,
                                     const char *object_end,
                                     const char *key,
                                     bool default_value)
{
    const char *found;
    const char *colon;
    const char *cursor;

    if ((NULL == object_start) || (NULL == object_end) || (NULL == key))
    {
        return default_value;
    }

    found = json_find_key(object_start, object_end, key);
    if (NULL == found)
    {
        return default_value;
    }

    colon = json_skip_whitespace(found, object_end);
    if ((NULL == colon) || (colon >= object_end) || (':' != *colon))
    {
        return default_value;
    }

    cursor = json_skip_whitespace(colon + 1, object_end);
    if ((NULL == cursor) || (cursor >= object_end))
    {
        return default_value;
    }

    if (0 == strncmp(cursor, "true", 4))
    {
        return true;
    }
    if (0 == strncmp(cursor, "false", 5))
    {
        return false;
    }
    if (0 == strncmp(cursor, "\"1\"", 3))
    {
        return true;
    }
    if (0 == strncmp(cursor, "\"0\"", 3))
    {
        return false;
    }
    if ('1' == *cursor)
    {
        return true;
    }
    if ('0' == *cursor)
    {
        return false;
    }

    return default_value;
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
    while ((cursor < object_end) && (profile->card_count < STORAGE_MAX_CARDS_PER_USER))
    {
        char raw_card[UID_MAX_LEN];
        const char *end;

        cursor = json_skip_whitespace(cursor, object_end);
        if ((NULL == cursor) || (cursor >= object_end) || (']' == *cursor))
        {
            break;
        }

        if (',' == *cursor)
        {
            cursor++;
            continue;
        }

        if ('"' != *cursor)
        {
            break;
        }

        if (!json_copy_string_value(raw_card, sizeof(raw_card), cursor + 1, object_end, &end))
        {
            break;
        }

        {
            size_t copy_len = strlen(raw_card);

            if (copy_len >= sizeof(profile->cards[profile->card_count]))
            {
                copy_len = sizeof(profile->cards[profile->card_count]) - 1U;
            }
            memcpy(profile->cards[profile->card_count], raw_card, copy_len);
            profile->cards[profile->card_count][copy_len] = '\0';
        }

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
    const char *json_end;
    int imported = 0;

    if ((NULL == json_text) || ('\0' == json_text[0]))
    {
        return 0;
    }

    json_end = json_text + strlen(json_text);
    while ((NULL != cursor) && (cursor < json_end) && ('\0' != *cursor))
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

        object_end = json_find_object_end(object_start, json_end);
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
        profile.is_admin = import_extract_json_bool(object_start, object_end, "is_admin", false);
        (void) import_extract_json_string(object_start, object_end, "admin_pin", profile.admin_pin, sizeof(profile.admin_pin));
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
        !g_net_import_processing &&
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
        net_action_lock();
        net_reset_upload_session();
        net_action_unlock();
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
        net_action_lock();
        net_reset_upload_session();
        net_action_unlock();
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
        net_action_lock();
        net_reset_upload_session();
        net_action_unlock();
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
    ULONG import_start_tick = 0U;

    net_action_lock();
    if (g_net_import_pending)
    {
        g_net_import_pending = false;
        g_net_import_processing = true;
        import_pending = true;
    }
    net_action_unlock();

    if (import_pending)
    {
        import_start_tick = tx_time_get();
        imported_count = import_profiles_from_json(g_net_pending_import_json);
        app_metric_add("JSON import",
                       net_metric_import_case_for_count(imported_count),
                       tx_time_get() - import_start_tick,
                       (imported_count > 0));
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

        net_action_lock();
        g_net_pending_import_json[0] = '\0';
        g_net_import_processing = false;
        net_action_unlock();
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
    ULONG html_len;
    size_t offset = 0U;

    if (NULL == html)
    {
        g_net_debug_last_response_status = NX_PTR_ERROR;
        return NX_PTR_ERROR;
    }

    html_len = (ULONG) strlen(html);
    if (NX_HTTP_CALLBACK_COMPLETED != send_stream_response_header(server, packet_ptr, html_len, "text/html"))
    {
        return g_net_debug_last_response_status;
    }

    while (offset < (size_t) html_len)
    {
        ULONG chunk_len = (ULONG) sizeof(g_net_download_chunk);

        if (((size_t) html_len - offset) < (size_t) chunk_len)
        {
            chunk_len = (ULONG) ((size_t) html_len - offset);
        }

        g_net_debug_last_response_status = nx_http_server_callback_data_send(server,
                                                                             (VOID *) &html[offset],
                                                                             chunk_len);
        if (NX_SUCCESS != g_net_debug_last_response_status)
        {
            return g_net_debug_last_response_status;
        }

        offset += (size_t) chunk_len;
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

static UINT send_buffer_response(NX_HTTP_SERVER *server, NX_PACKET *packet_ptr, const void *data, size_t data_len, const char *content_type)
{
    UINT status;

    if ((NULL == data) || (NULL == content_type))
    {
        g_net_debug_last_response_status = NX_PTR_ERROR;
        return NX_PTR_ERROR;
    }

    status = nx_http_server_callback_generate_response_header(server,
                                                              &packet_ptr,
                                                              NX_HTTP_STATUS_OK,
                                                              data_len,
                                                              (CHAR *) content_type,
                                                              NULL);
    if (NX_SUCCESS != status)
    {
        g_net_debug_last_response_status = status;
        return status;
    }

    status = nx_packet_data_append(packet_ptr,
                                   (VOID *) data,
                                   data_len,
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

static UINT send_buffer_status_response(NX_HTTP_SERVER *server,
                                        NX_PACKET *packet_ptr,
                                        const char *status_code,
                                        const void *data,
                                        size_t data_len,
                                        const char *content_type)
{
    UINT status;

    if ((NULL == server) || (NULL == packet_ptr) || (NULL == status_code) || (NULL == data) || (NULL == content_type))
    {
        g_net_debug_last_response_status = NX_PTR_ERROR;
        return NX_PTR_ERROR;
    }

    status = nx_http_server_callback_generate_response_header(server,
                                                              &packet_ptr,
                                                              (CHAR *) status_code,
                                                              data_len,
                                                              (CHAR *) content_type,
                                                              NULL);
    if (NX_SUCCESS != status)
    {
        g_net_debug_last_response_status = status;
        return status;
    }

    status = nx_packet_data_append(packet_ptr,
                                   (VOID *) data,
                                   data_len,
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

static UINT send_stream_response_header(NX_HTTP_SERVER *server, NX_PACKET *packet_ptr, ULONG data_len, const char *content_type)
{
    UINT status;

    if (NULL == content_type)
    {
        g_net_debug_last_response_status = NX_PTR_ERROR;
        return NX_PTR_ERROR;
    }

    status = nx_http_server_callback_generate_response_header(server,
                                                              &packet_ptr,
                                                              NX_HTTP_STATUS_OK,
                                                              data_len,
                                                              (CHAR *) content_type,
                                                              NULL);
    if (NX_SUCCESS != status)
    {
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

static int net_format_metric_csv_line(char *out, size_t out_size, const app_metric_entry_t *entry)
{
    char timestamp[32];
    char duration_text[24];

    if ((NULL == out) || (0U == out_size) || (NULL == entry))
    {
        return -1;
    }

    app_format_metric_timestamp(entry, timestamp, sizeof(timestamp));
    net_format_ms_x10(net_ticks_to_ms_x10(entry->duration_ticks), duration_text, sizeof(duration_text));

    return snprintf(out,
                    out_size,
                    "%s,%s,%s,%lu,%s,%s\r\n",
                    timestamp,
                    app_metric_kind_text(entry->kind),
                    app_metric_case_text(entry->case_id),
                    (unsigned long) entry->duration_ticks,
                    duration_text,
                    entry->success ? "1" : "0");
}

static UINT render_home_page(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, const char *message, int edit_index)
{
    /* ADICIONADO 'static' PARA EVITAR STACK OVERFLOW! */
    static char html[HTML_BUFFER_SIZE];
    static char rows[ROWS_BUFFER_SIZE];
    static char photo_options[PHOTO_OPTIONS_BUFFER_SIZE];
    static char profile_options[2048];
    const char *message_to_render = message;

    char ip_text[20];
    char netmask_text[20];
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
    user_count = storage_user_count_nowait();
    if (user_count > STORAGE_MAX_USERS)
    {
        user_count = STORAGE_MAX_USERS;
    }

    if ((edit_index >= 0) && (edit_index < user_count) && storage_profile_get_nowait(edit_index, &form_profile))
    {
        editing = true;
        profile_cards_to_csv(&form_profile, form_cards, sizeof(form_cards));
    }
    else if ('\0' != last_uid[0])
    {
        strncpy(form_cards, last_uid, sizeof(form_cards) - 1U);
        form_cards[sizeof(form_cards) - 1U] = '\0';
    }

    net_build_photo_options(photo_options, sizeof(photo_options));
    net_build_profile_options(profile_options, sizeof(profile_options));
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
        storage_user_profile_t row_profile;
        char row_cards[FORM_CARDS_BUFFER_SIZE];
        int written;

        if (!storage_profile_get_nowait(i, &row_profile))
        {
            continue;
        }

        profile_cards_to_csv(&row_profile, row_cards, sizeof(row_cards));

        written = snprintf(&rows[rows_len],
                           sizeof(rows) - rows_len,
                           "<tr><td>%s</td><td>%s</td><td>%s</td><td>%s%s</td><td>%s</td>"
                           "<td><a class='small' href='/?edit=%d'>Editar</a> "
                           "<a class='small danger' href='/remove_user?index=%d'>Remover</a></td></tr>",
                           row_profile.name,
                           ('\0' != row_profile.role[0]) ? row_profile.role : "-",
                           ('\0' != row_profile.chapter[0]) ? row_profile.chapter : "-",
                           ('\0' != row_profile.photo_id[0]) ? row_profile.photo_id : "-",
                           ('\0' != row_profile.photo_id[0]) ? (net_photo_asset_exists(row_profile.photo_id) ? " <span class='muted'>(ok)</span>" : " <span class='muted'>(sem asset)</span>") : "",
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
             "<div class='card'><h1>Ramo Estudantil IEEE UFJF - Controle de acesso</h1>"
             "<p class='muted'>IP atual: <strong>%s</strong> | Mascara: <strong>%s</strong> | Rede: <strong>DHCP</strong></p>"
             "<p class='%s'>Link Ethernet: <strong>%s</strong></p>"
             "<p class='muted'>Endereco obtido automaticamente por DHCP.</p>"
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
             "<button class='btn' type='submit'>Salvar perfil</button></form>"
             "%s"
             "<p><a class='small' href='/save_users'>Persistir cadastros</a></p>"
             "<p class='muted'>Persistencia QSPI: <strong>%s</strong></p>"
             "<p class='muted'>Ultimo cartao lido: <strong>%s</strong></p>"
             "<p class='muted'>Ultimo usuario: <strong>%s</strong></p>"
             "<p><a class='small' href='/portaon'>Abrir porta</a></p></div>"
             "<div class='card'><h2>Perfis cadastrados</h2>"
             "<table><tr><th>Nome</th><th>Cargo</th><th>Capitulo</th><th>Foto</th><th>Cartoes</th><th>Acao</th></tr>%s</table>"
             "<div class='card' style='margin-top:18px;'><h2>Anexar foto a perfil</h2>"
             "<form action='/attach_photo' method='get'>"
             "<label>Perfil</label><select name='profile_index' style='width:100%%;padding:12px;border:1px solid #ccd4e0;border-radius:10px;margin:8px 0 12px 0;box-sizing:border-box;'>%s</select>"
             "<label>Photo ID</label><input type='text' name='photo_id' list='photo-id-list' maxlength='63' placeholder='Escolha um photo_id existente'>"
             "<button class='btn' type='submit'>Anexar foto</button></form>"
             "</div>"
             "<div class='card' style='margin-top:18px;'><h2>Importar perfis offline</h2>"
             "<form action='/import_profiles' method='post'>"
             "<label>JSON de perfis</label>"
             "<textarea name='import_json' placeholder='Cole aqui o JSON'></textarea>"
             "<button class='btn' type='submit'>Importar perfis</button></form>"
             "<p class='muted'>Use o JSON simples de perfis.</p>"
             "</div>"
             "</div></div></div></body></html>",
             ip_text,
             netmask_text,
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
    SSP_PARAMETER_NOT_USED(packet_ptr);

    if (!net_admin_is_authenticated())
    {
        return light_redirect_with_flash(server_ptr,
                                         "/login",
                                         "<div class='card warn'>Autentique-se para anexar fotos aos perfis.</div>");
    }

    if ((profile_index < 0) || !storage_profile_get(profile_index, &profile))
    {
        return light_redirect_with_flash(server_ptr,
                                         "/admin_profiles",
                                         "<div class='card warn'>Selecione um perfil valido para anexar a foto.</div>");
    }

    if (!query_get_value(query, "photo_id", profile.photo_id, sizeof(profile.photo_id)))
    {
        return light_redirect_with_flash(server_ptr,
                                         "/admin_profiles",
                                         "<div class='card warn'>Selecione um photo_id antes de anexar a foto.</div>");
    }

    if (!net_photo_asset_exists(profile.photo_id))
    {
        return light_redirect_with_flash(server_ptr,
                                         "/admin_profiles",
                                         "<div class='card warn'>Esse photo_id nao existe no firmware atual.</div>");
    }

    if (storage_profile_upsert(&profile, profile_index))
    {
        persist_requested = storage_persist_now();
        return light_redirect_with_flash(server_ptr,
                                         "/admin_profiles",
                                         persist_requested
                                             ? "<div class='card ok'>Foto anexada e persistencia automatica solicitada.</div>"
                                             : "<div class='card ok'>Foto anexada. Se quiser, use \"Persistir cadastros\" para confirmar.</div>");
    }

    return light_redirect_with_flash(server_ptr,
                                     "/admin_profiles",
                                     "<div class='card warn'>Nao foi possivel anexar a foto ao perfil.</div>");
}

static bool extract_body_from_packet_fallback(NX_PACKET *packet_ptr, char *body_out, size_t body_size)
{
    char *req;
    char *req_end;
    char *body_start;
    size_t len;

    body_out[0] = '\0';
    if ((NULL == packet_ptr) || (NULL == packet_ptr->nx_packet_prepend_ptr) || (0U == body_size))
    {
        return false;
    }

    req = (char *) packet_ptr->nx_packet_prepend_ptr;
    req_end = (char *) packet_ptr->nx_packet_append_ptr;
    body_start = strstr(req, "\r\n\r\n");
    if ((NULL == body_start) || (body_start > req_end))
    {
        return true;
    }

    body_start += 4;
    if (body_start > req_end)
    {
        return true;
    }

    len = (size_t) (req_end - body_start);
    if (len >= body_size)
    {
        return false;
    }

    memcpy(body_out, body_start, len);
    body_out[len] = '\0';
    return true;
}

static bool extract_body_from_packet(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, char *body_out, size_t body_size)
{
    ULONG content_length = 0U;
    ULONG byte_offset = 0U;
    size_t total_read = 0U;
    UINT status;

    if ((NULL == body_out) || (0U == body_size))
    {
        return false;
    }

    body_out[0] = '\0';

    if ((NULL == server_ptr) || (NULL == packet_ptr))
    {
        return false;
    }

    status = nx_http_server_content_length_get_extended(packet_ptr, &content_length);
    if ((NX_SUCCESS == status) && (content_length > 0U))
    {
        if (content_length >= (ULONG) body_size)
        {
            return false;
        }

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
                return false;
            }

            total_read += (size_t) actual_size;
            byte_offset += (ULONG) actual_size;
        }

        body_out[total_read] = '\0';
        return (byte_offset == content_length);
    }

    return extract_body_from_packet_fallback(packet_ptr, body_out, body_size);
}

static bool net_ascii_span_equals_ignore_case(const char *left, const char *right, size_t len)
{
    size_t i;

    if ((NULL == left) || (NULL == right))
    {
        return false;
    }

    for (i = 0U; i < len; i++)
    {
        if (tolower((unsigned char) left[i]) != tolower((unsigned char) right[i]))
        {
            return false;
        }
    }

    return true;
}

static const char *net_find_crlf(const char *cursor, const char *end)
{
    const char *p;

    if ((NULL == cursor) || (NULL == end) || (cursor >= end))
    {
        return NULL;
    }

    for (p = cursor; (p + 1) < end; p++)
    {
        if ((p[0] == '\r') && (p[1] == '\n'))
        {
            return p;
        }
    }

    return NULL;
}

static bool extract_header_from_packet(NX_PACKET *packet_ptr, const char *header_name, char *out, size_t out_size)
{
    const char *req;
    const char *req_end;
    const char *cursor;
    const char *line_end;
    size_t header_name_len;

    if ((NULL == out) || (0U == out_size))
    {
        return false;
    }

    out[0] = '\0';

    if ((NULL == packet_ptr) || (NULL == packet_ptr->nx_packet_prepend_ptr) || (NULL == header_name))
    {
        return false;
    }

    req = (const char *) packet_ptr->nx_packet_prepend_ptr;
    req_end = (const char *) packet_ptr->nx_packet_append_ptr;
    if ((NULL == req_end) || (req >= req_end))
    {
        return false;
    }

    line_end = net_find_crlf(req, req_end);
    if (NULL == line_end)
    {
        return false;
    }

    header_name_len = strlen(header_name);
    cursor = line_end + 2;

    while (cursor < req_end)
    {
        const char *colon = NULL;
        const char *value_start;
        const char *value_end;
        const char *p;
        size_t value_len;

        line_end = net_find_crlf(cursor, req_end);
        if (NULL == line_end)
        {
            break;
        }

        if (line_end == cursor)
        {
            break;
        }

        for (p = cursor; p < line_end; p++)
        {
            if (':' == *p)
            {
                colon = p;
                break;
            }
        }

        if ((NULL != colon) &&
            (((size_t) (colon - cursor)) == header_name_len) &&
            net_ascii_span_equals_ignore_case(cursor, header_name, header_name_len))
        {
            value_start = colon + 1;
            while ((value_start < line_end) && isspace((unsigned char) *value_start))
            {
                value_start++;
            }

            value_end = line_end;
            while ((value_end > value_start) && isspace((unsigned char) value_end[-1]))
            {
                value_end--;
            }

            value_len = (size_t) (value_end - value_start);
            if (value_len >= out_size)
            {
                value_len = out_size - 1U;
            }

            memcpy(out, value_start, value_len);
            out[value_len] = '\0';
            return true;
        }

        cursor = line_end + 2;
    }

    return false;
}

static bool net_authorization_matches_api_key(const char *authorization_value)
{
    static const char bearer_prefix[] = "Bearer ";
    const char *token;
    size_t prefix_len = sizeof(bearer_prefix) - 1U;

    if (NULL == authorization_value)
    {
        return false;
    }

    if (!net_ascii_span_equals_ignore_case(authorization_value, bearer_prefix, prefix_len))
    {
        return false;
    }

    token = authorization_value + prefix_len;
    while (' ' == *token)
    {
        token++;
    }

    return (0 == strcmp(token, API_KEY));
}

static bool net_api_request_is_authorized(NX_PACKET *packet_ptr)
{
    char api_key_value[96];
    char authorization_value[128];

    if (extract_header_from_packet(packet_ptr, "X-API-KEY", api_key_value, sizeof(api_key_value)))
    {
        if (0 == strcmp(api_key_value, API_KEY))
        {
            return true;
        }
    }

    if (extract_header_from_packet(packet_ptr, "Authorization", authorization_value, sizeof(authorization_value)))
    {
        if (net_authorization_matches_api_key(authorization_value))
        {
            return true;
        }
    }

    return false;
}

static UINT handle_remove_user(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, const char *query)
{
    int index = query_get_int(query, "index", -1);

    if (index < 0)
    {
        return render_home_page(server_ptr, packet_ptr, "<div class='card warn'>Índice invalido para remoção.</div>", -1);
    }

    if (storage_profile_remove(index))
    {
        app_post_event(EVENT_USER_REMOVED, NULL);
        return render_home_page(server_ptr, packet_ptr, "<div class='card ok'>Perfil removido. Clique em \"Persistir cadastros\" para salvar na memória interna.</div>", -1);
    }

    return render_home_page(server_ptr, packet_ptr, "<div class='card warn'>Perfil não encontrado.</div>", -1);
}

static UINT handle_save_users(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr)
{
    if (storage_persist_now())
    {
        return render_home_page(server_ptr, packet_ptr, "<div class='card ok'>Persistencia solicitada. Aguarde alguns segundos e recarregue a pagina.</div>", -1);
    }

    return render_home_page(server_ptr, packet_ptr, "<div class='card warn'>Nao foi possivel salvar os cadastros agora.</div>", -1);
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

static bool net_appendf(char *buffer, size_t buffer_size, size_t *offset, const char *format, ...)
{
    va_list args;
    int written;

    if ((NULL == buffer) || (NULL == offset) || (NULL == format) || (*offset >= buffer_size))
    {
        return false;
    }

    va_start(args, format);
    written = vsnprintf(&buffer[*offset], buffer_size - *offset, format, args);
    va_end(args);

    if ((written < 0) || ((size_t) written >= (buffer_size - *offset)))
    {
        if (buffer_size > 0U)
        {
            buffer[buffer_size - 1U] = '\0';
        }
        return false;
    }

    *offset += (size_t) written;
    return true;
}

static void net_meeting_mode_draft_reset(void)
{
    memset(g_net_meeting_draft_selected, 0, sizeof(g_net_meeting_draft_selected));
    g_net_meeting_draft_initialized = true;
}

static void net_meeting_mode_draft_sync_from_active(void)
{
    int user_count;

    if (g_net_meeting_draft_initialized)
    {
        return;
    }

    net_meeting_mode_draft_reset();
    user_count = storage_user_count_nowait();
    if (user_count > STORAGE_MAX_USERS)
    {
        user_count = STORAGE_MAX_USERS;
    }

    for (int i = 0; i < user_count; i++)
    {
        storage_user_profile_t profile;

        if (storage_profile_get_nowait(i, &profile) && storage_meeting_mode_profile_selected(&profile))
        {
            g_net_meeting_draft_selected[i] = true;
        }
    }
}

static unsigned int net_meeting_mode_draft_profile_count(void)
{
    unsigned int count = 0U;
    int user_count = storage_user_count_nowait();

    if (user_count > STORAGE_MAX_USERS)
    {
        user_count = STORAGE_MAX_USERS;
    }

    for (int i = 0; i < user_count; i++)
    {
        storage_user_profile_t profile;

        if (g_net_meeting_draft_selected[i] &&
            storage_profile_get_nowait(i, &profile) &&
            (profile.card_count > 0U))
        {
            count++;
        }
    }

    return count;
}

static void net_meeting_schedule_set_status_locked(const char *status)
{
    if (NULL == status)
    {
        status = "unknown";
    }

    strncpy(g_net_meeting_schedule_last_status,
            status,
            sizeof(g_net_meeting_schedule_last_status) - 1U);
    g_net_meeting_schedule_last_status[sizeof(g_net_meeting_schedule_last_status) - 1U] = '\0';
}

static bool net_parse_ulong_strict(const char *text, ULONG *out_value)
{
    const char *cursor;
    char *end_ptr;
    unsigned long value;

    if ((NULL == text) || (NULL == out_value))
    {
        return false;
    }

    cursor = text;
    while (('\0' != *cursor) && isspace((unsigned char) *cursor))
    {
        cursor++;
    }

    if (('\0' == *cursor) || ('-' == *cursor))
    {
        return false;
    }

    value = strtoul(cursor, &end_ptr, 10);
    if (end_ptr == cursor)
    {
        return false;
    }

    while (('\0' != *end_ptr) && isspace((unsigned char) *end_ptr))
    {
        end_ptr++;
    }

    if ('\0' != *end_ptr)
    {
        return false;
    }

    *out_value = (ULONG) value;
    return true;
}

static bool net_json_get_ulong_value(const char *json, const char *key, ULONG *out_value)
{
    const char *limit;
    const char *found;
    const char *colon;
    const char *cursor;

    if ((NULL == json) || (NULL == key) || (NULL == out_value))
    {
        return false;
    }

    limit = json + strlen(json);
    found = json_find_key(json, limit, key);
    if (NULL == found)
    {
        return false;
    }

    colon = json_skip_whitespace(found, limit);
    if ((NULL == colon) || (colon >= limit) || (':' != *colon))
    {
        return false;
    }

    cursor = json_skip_whitespace(colon + 1, limit);
    if ((NULL == cursor) || (cursor >= limit))
    {
        return false;
    }

    if ('"' == *cursor)
    {
        char text[24];
        const char *end_quote;

        if (!json_copy_string_value(text, sizeof(text), cursor + 1, limit, &end_quote))
        {
            return false;
        }

        return net_parse_ulong_strict(text, out_value);
    }

    {
        char *end_ptr;
        unsigned long value;

        if ('-' == *cursor)
        {
            return false;
        }

        value = strtoul(cursor, &end_ptr, 10);
        if (end_ptr == cursor)
        {
            return false;
        }

        cursor = json_skip_whitespace(end_ptr, limit);
        if ((cursor < limit) && (',' != *cursor) && ('}' != *cursor) && (']' != *cursor))
        {
            return false;
        }

        *out_value = (ULONG) value;
        return true;
    }
}

static bool net_form_get_ulong_value(const char *form, const char *key, ULONG *out_value)
{
    char text[24];

    if ((NULL == form) || (NULL == key) || (NULL == out_value))
    {
        return false;
    }

    if (!query_get_value(form, key, text, sizeof(text)))
    {
        return false;
    }

    return net_parse_ulong_strict(text, out_value);
}

static bool net_profile_index_add_unique(int *profiles, int max_profiles, int *profile_count, long value)
{
    if ((NULL == profiles) || (NULL == profile_count) ||
        (value < 0L) || (value >= (long) STORAGE_MAX_USERS))
    {
        return false;
    }

    for (int i = 0; i < *profile_count; i++)
    {
        if (profiles[i] == (int) value)
        {
            return true;
        }
    }

    if (*profile_count >= max_profiles)
    {
        return false;
    }

    profiles[*profile_count] = (int) value;
    (*profile_count)++;
    return true;
}

static bool net_parse_profile_indices_csv(const char *text, int *profiles, int max_profiles, int *profile_count)
{
    const char *cursor;

    if ((NULL == text) || (NULL == profiles) || (NULL == profile_count))
    {
        return false;
    }

    *profile_count = 0;
    cursor = text;
    while ('\0' != *cursor)
    {
        char *end_ptr;
        long value;

        while (('\0' != *cursor) &&
               (isspace((unsigned char) *cursor) || (',' == *cursor) || (';' == *cursor)))
        {
            cursor++;
        }

        if ('\0' == *cursor)
        {
            break;
        }

        if ('-' == *cursor)
        {
            return false;
        }

        value = strtol(cursor, &end_ptr, 10);
        if (end_ptr == cursor)
        {
            return false;
        }

        if (!net_profile_index_add_unique(profiles, max_profiles, profile_count, value))
        {
            return false;
        }

        cursor = end_ptr;
        while (('\0' != *cursor) && isspace((unsigned char) *cursor))
        {
            cursor++;
        }

        if (('\0' != *cursor) && (',' != *cursor) && (';' != *cursor))
        {
            return false;
        }
    }

    return (*profile_count > 0);
}

static bool net_json_get_profile_indices(const char *json, int *profiles, int max_profiles, int *profile_count)
{
    const char *limit;
    const char *found;
    const char *colon;
    const char *cursor;

    if ((NULL == json) || (NULL == profiles) || (NULL == profile_count))
    {
        return false;
    }

    limit = json + strlen(json);
    found = json_find_key(json, limit, "profile_indices");
    if (NULL == found)
    {
        found = json_find_key(json, limit, "profiles");
    }
    if (NULL == found)
    {
        return false;
    }

    colon = json_skip_whitespace(found, limit);
    if ((NULL == colon) || (colon >= limit) || (':' != *colon))
    {
        return false;
    }

    cursor = json_skip_whitespace(colon + 1, limit);
    if ((NULL == cursor) || (cursor >= limit))
    {
        return false;
    }

    *profile_count = 0;
    if ('"' == *cursor)
    {
        char csv[192];
        const char *end_quote;

        if (!json_copy_string_value(csv, sizeof(csv), cursor + 1, limit, &end_quote))
        {
            return false;
        }

        return net_parse_profile_indices_csv(csv, profiles, max_profiles, profile_count);
    }

    if ('[' != *cursor)
    {
        char *end_ptr;
        long value;

        if ('-' == *cursor)
        {
            return false;
        }

        value = strtol(cursor, &end_ptr, 10);
        if (end_ptr == cursor)
        {
            return false;
        }

        return net_profile_index_add_unique(profiles, max_profiles, profile_count, value);
    }

    cursor++;
    while (cursor < limit)
    {
        cursor = json_skip_whitespace(cursor, limit);
        if ((NULL == cursor) || (cursor >= limit))
        {
            return false;
        }

        if (']' == *cursor)
        {
            return (*profile_count > 0);
        }

        if ('"' == *cursor)
        {
            char text[16];
            const char *end_quote;
            ULONG parsed_value;

            if (!json_copy_string_value(text, sizeof(text), cursor + 1, limit, &end_quote) ||
                !net_parse_ulong_strict(text, &parsed_value) ||
                !net_profile_index_add_unique(profiles, max_profiles, profile_count, (long) parsed_value))
            {
                return false;
            }
            cursor = end_quote + 1;
        }
        else
        {
            char *end_ptr;
            long value;

            if ('-' == *cursor)
            {
                return false;
            }

            value = strtol(cursor, &end_ptr, 10);
            if (end_ptr == cursor)
            {
                return false;
            }

            if (!net_profile_index_add_unique(profiles, max_profiles, profile_count, value))
            {
                return false;
            }
            cursor = end_ptr;
        }

        cursor = json_skip_whitespace(cursor, limit);
        if ((NULL == cursor) || (cursor >= limit))
        {
            return false;
        }

        if (',' == *cursor)
        {
            cursor++;
            continue;
        }

        if (']' == *cursor)
        {
            return (*profile_count > 0);
        }

        return false;
    }

    return false;
}

static bool net_form_get_profile_indices(const char *form, int *profiles, int max_profiles, int *profile_count)
{
    char csv[192];

    if ((NULL == form) || (NULL == profiles) || (NULL == profile_count))
    {
        return false;
    }

    if (!query_get_value(form, "profile_indices", csv, sizeof(csv)) &&
        !query_get_value(form, "profiles", csv, sizeof(csv)))
    {
        return false;
    }

    return net_parse_profile_indices_csv(csv, profiles, max_profiles, profile_count);
}

static uint8_t net_meeting_weekday_from_unix(ULONG unix_utc)
{
    return (uint8_t) (((unix_utc / 86400UL) + 4UL) % 7UL);
}

static const char *net_meeting_recurrence_text(uint8_t recurrence)
{
    switch (recurrence)
    {
        case STORAGE_MEETING_RECURRENCE_DAILY:
            return "daily";
        case STORAGE_MEETING_RECURRENCE_WEEKLY:
            return "weekly";
        default:
            return "none";
    }
}

static bool net_meeting_parse_recurrence_text(const char *text, uint8_t *out_recurrence)
{
    char normalized[16];
    size_t write_index = 0U;

    if ((NULL == text) || (NULL == out_recurrence))
    {
        return false;
    }

    while (('\0' != *text) && isspace((unsigned char) *text))
    {
        text++;
    }

    while (('\0' != *text) && (write_index + 1U < sizeof(normalized)))
    {
        if (!isspace((unsigned char) *text))
        {
            normalized[write_index++] = (char) tolower((unsigned char) *text);
        }
        text++;
    }
    normalized[write_index] = '\0';

    if ((0 == strcmp(normalized, "none")) || (0 == strcmp(normalized, "once")) || (0 == strcmp(normalized, "0")))
    {
        *out_recurrence = STORAGE_MEETING_RECURRENCE_NONE;
        return true;
    }
    if ((0 == strcmp(normalized, "daily")) || (0 == strcmp(normalized, "diario")) ||
        (0 == strcmp(normalized, "diaria")) || (0 == strcmp(normalized, "1")))
    {
        *out_recurrence = STORAGE_MEETING_RECURRENCE_DAILY;
        return true;
    }
    if ((0 == strcmp(normalized, "weekly")) || (0 == strcmp(normalized, "semanal")) || (0 == strcmp(normalized, "2")))
    {
        *out_recurrence = STORAGE_MEETING_RECURRENCE_WEEKLY;
        return true;
    }

    return false;
}

static bool net_json_get_recurrence_value(const char *json, uint8_t *out_recurrence, bool *out_found)
{
    const char *limit;
    const char *found;
    const char *colon;
    const char *cursor;
    ULONG value = 0U;

    if ((NULL == json) || (NULL == out_recurrence) || (NULL == out_found))
    {
        return false;
    }

    *out_found = false;
    limit = json + strlen(json);
    found = json_find_key(json, limit, "recurrence");
    if (NULL == found)
    {
        return true;
    }

    colon = json_skip_whitespace(found, limit);
    if ((NULL == colon) || (colon >= limit) || (':' != *colon))
    {
        return false;
    }

    cursor = json_skip_whitespace(colon + 1, limit);
    if ((NULL == cursor) || (cursor >= limit))
    {
        return false;
    }

    *out_found = true;
    if ('"' == *cursor)
    {
        char text[16];
        const char *end_quote;

        if (!json_copy_string_value(text, sizeof(text), cursor + 1, limit, &end_quote))
        {
            return false;
        }
        return net_meeting_parse_recurrence_text(text, out_recurrence);
    }

    if (!net_json_get_ulong_value(json, "recurrence", &value) ||
        (value > STORAGE_MEETING_RECURRENCE_WEEKLY))
    {
        return false;
    }

    *out_recurrence = (uint8_t) value;
    return true;
}

static bool net_form_get_recurrence_value(const char *form, uint8_t *out_recurrence, bool *out_found)
{
    char text[16];

    if ((NULL == form) || (NULL == out_recurrence) || (NULL == out_found))
    {
        return false;
    }

    *out_found = false;
    if (!query_get_value(form, "recurrence", text, sizeof(text)))
    {
        return true;
    }

    *out_found = true;
    return net_meeting_parse_recurrence_text(text, out_recurrence);
}

static bool net_weekday_add_unique(uint8_t *mask, long value)
{
    if ((NULL == mask) || (value < 0L) || (value > 6L))
    {
        return false;
    }

    *mask = (uint8_t) (*mask | (uint8_t) (1U << (uint8_t) value));
    return true;
}

static bool net_parse_weekdays_csv(const char *text, uint8_t *out_mask)
{
    const char *cursor;
    uint8_t mask = 0U;

    if ((NULL == text) || (NULL == out_mask))
    {
        return false;
    }

    cursor = text;
    while ('\0' != *cursor)
    {
        char *end_ptr;
        long value;

        while (('\0' != *cursor) &&
               (isspace((unsigned char) *cursor) || (',' == *cursor) || (';' == *cursor)))
        {
            cursor++;
        }

        if ('\0' == *cursor)
        {
            break;
        }

        if ('-' == *cursor)
        {
            return false;
        }

        value = strtol(cursor, &end_ptr, 10);
        if ((end_ptr == cursor) || !net_weekday_add_unique(&mask, value))
        {
            return false;
        }

        cursor = end_ptr;
        while (('\0' != *cursor) && isspace((unsigned char) *cursor))
        {
            cursor++;
        }

        if (('\0' != *cursor) && (',' != *cursor) && (';' != *cursor))
        {
            return false;
        }
    }

    *out_mask = mask;
    return true;
}

static bool net_json_get_weekdays_value(const char *json, uint8_t *out_mask, bool *out_found)
{
    const char *limit;
    const char *found;
    const char *colon;
    const char *cursor;
    ULONG mask_value = 0U;
    uint8_t mask = 0U;

    if ((NULL == json) || (NULL == out_mask) || (NULL == out_found))
    {
        return false;
    }

    *out_found = false;
    if (net_json_get_ulong_value(json, "weekdays_mask", &mask_value))
    {
        *out_mask = (uint8_t) (mask_value & STORAGE_MEETING_WEEKDAY_MASK_ALL);
        *out_found = true;
        return true;
    }

    limit = json + strlen(json);
    found = json_find_key(json, limit, "weekdays");
    if (NULL == found)
    {
        return true;
    }

    colon = json_skip_whitespace(found, limit);
    if ((NULL == colon) || (colon >= limit) || (':' != *colon))
    {
        return false;
    }

    cursor = json_skip_whitespace(colon + 1, limit);
    if ((NULL == cursor) || (cursor >= limit))
    {
        return false;
    }

    *out_found = true;
    if ('"' == *cursor)
    {
        char csv[32];
        const char *end_quote;

        if (!json_copy_string_value(csv, sizeof(csv), cursor + 1, limit, &end_quote) ||
            !net_parse_weekdays_csv(csv, &mask))
        {
            return false;
        }
        *out_mask = mask;
        return true;
    }

    if ('[' != *cursor)
    {
        char *end_ptr;
        long value;

        value = strtol(cursor, &end_ptr, 10);
        if ((end_ptr == cursor) || !net_weekday_add_unique(&mask, value))
        {
            return false;
        }
        *out_mask = mask;
        return true;
    }

    cursor++;
    while (cursor < limit)
    {
        char *end_ptr;
        long value;

        cursor = json_skip_whitespace(cursor, limit);
        if ((NULL == cursor) || (cursor >= limit))
        {
            return false;
        }

        if (']' == *cursor)
        {
            *out_mask = mask;
            return true;
        }

        value = strtol(cursor, &end_ptr, 10);
        if ((end_ptr == cursor) || !net_weekday_add_unique(&mask, value))
        {
            return false;
        }

        cursor = json_skip_whitespace(end_ptr, limit);
        if ((NULL == cursor) || (cursor >= limit))
        {
            return false;
        }

        if (',' == *cursor)
        {
            cursor++;
            continue;
        }

        if (']' == *cursor)
        {
            *out_mask = mask;
            return true;
        }

        return false;
    }

    return false;
}

static bool net_form_get_weekdays_value(const char *form, uint8_t *out_mask, bool *out_found)
{
    ULONG mask_value = 0U;
    char csv[32];

    if ((NULL == form) || (NULL == out_mask) || (NULL == out_found))
    {
        return false;
    }

    *out_found = false;
    if (net_form_get_ulong_value(form, "weekdays_mask", &mask_value))
    {
        *out_mask = (uint8_t) (mask_value & STORAGE_MEETING_WEEKDAY_MASK_ALL);
        *out_found = true;
        return true;
    }

    if (!query_get_value(form, "weekdays", csv, sizeof(csv)))
    {
        return true;
    }

    *out_found = true;
    return net_parse_weekdays_csv(csv, out_mask);
}

static void net_meeting_schedule_rebuild_next_id_locked(void)
{
    ULONG next_id = 1U;

    for (int i = 0; i < g_net_meeting_schedule_count; i++)
    {
        if (g_net_meeting_schedules[i].id >= next_id)
        {
            next_id = g_net_meeting_schedules[i].id + 1U;
        }
    }

    if (0U == next_id)
    {
        next_id = 1U;
    }
    g_net_meeting_schedule_next_id = next_id;
}

static bool net_meeting_schedule_ensure_loaded(void)
{
    storage_meeting_schedule_t loaded_schedules[STORAGE_MEETING_SCHEDULE_MAX_ITEMS];
    int loaded_count;
    bool already_loaded;

    net_action_lock();
    already_loaded = g_net_meeting_schedules_loaded;
    net_action_unlock();
    if (already_loaded)
    {
        return true;
    }

    loaded_count = storage_meeting_schedule_load(loaded_schedules, STORAGE_MEETING_SCHEDULE_MAX_ITEMS);
    if (loaded_count < 0)
    {
        return false;
    }

    net_action_lock();
    if (!g_net_meeting_schedules_loaded)
    {
        g_net_meeting_schedule_count = loaded_count;
        for (int i = 0; i < loaded_count; i++)
        {
            g_net_meeting_schedules[i] = loaded_schedules[i];
        }
        g_net_meeting_schedules_loaded = true;
        net_meeting_schedule_rebuild_next_id_locked();
        net_meeting_schedule_set_status_locked((loaded_count > 0) ? "loaded" : "idle");
    }
    net_action_unlock();
    return true;
}

static bool net_meeting_schedule_save_locked(void)
{
    return storage_meeting_schedule_save(g_net_meeting_schedules, g_net_meeting_schedule_count);
}

static bool net_meeting_schedule_validate_profiles(const int *profiles, int profile_count, const char **out_error)
{
    int user_count = storage_user_count();

    if ((NULL == profiles) || (profile_count <= 0))
    {
        if (NULL != out_error)
        {
            *out_error = "no_profiles";
        }
        return false;
    }

    for (int i = 0; i < profile_count; i++)
    {
        storage_user_profile_t profile;

        if ((profiles[i] < 0) || (profiles[i] >= user_count) || !storage_profile_get(profiles[i], &profile))
        {
            if (NULL != out_error)
            {
                *out_error = "invalid_profile";
            }
            return false;
        }

        if (0U == profile.card_count)
        {
            if (NULL != out_error)
            {
                *out_error = "profile_without_cards";
            }
            return false;
        }
    }

    return true;
}

static bool net_meeting_schedule_parse_request(const char *body,
                                               ULONG *out_start_utc,
                                               ULONG *out_delay_seconds,
                                               uint8_t *out_recurrence,
                                               uint8_t *out_weekdays_mask,
                                               int *out_profiles,
                                               int *out_profile_count,
                                               const char **out_error)
{
    const char *cursor;
    bool is_json;
    bool has_delay;
    bool has_start;
    ULONG delay_seconds = 0U;
    ULONG start_utc = 0U;
    uint8_t recurrence = STORAGE_MEETING_RECURRENCE_NONE;
    uint8_t weekdays_mask = 0U;
    bool recurrence_found = false;
    bool weekdays_found = false;
    int profile_count = 0;

    if (NULL != out_error)
    {
        *out_error = "bad_request";
    }

    if ((NULL == body) || (NULL == out_start_utc) || (NULL == out_delay_seconds) ||
        (NULL == out_recurrence) || (NULL == out_weekdays_mask) ||
        (NULL == out_profiles) || (NULL == out_profile_count))
    {
        return false;
    }

    cursor = body;
    while (('\0' != *cursor) && isspace((unsigned char) *cursor))
    {
        cursor++;
    }
    if ('\0' == *cursor)
    {
        if (NULL != out_error)
        {
            *out_error = "empty_body";
        }
        return false;
    }

    is_json = ('{' == *cursor);
    has_delay = is_json
                    ? net_json_get_ulong_value(cursor, "delay_seconds", &delay_seconds)
                    : net_form_get_ulong_value(cursor, "delay_seconds", &delay_seconds);
    has_start = is_json
                    ? net_json_get_ulong_value(cursor, "start_unix", &start_utc)
                    : net_form_get_ulong_value(cursor, "start_unix", &start_utc);

    if (!(is_json
              ? net_json_get_recurrence_value(cursor, &recurrence, &recurrence_found)
              : net_form_get_recurrence_value(cursor, &recurrence, &recurrence_found)))
    {
        if (NULL != out_error)
        {
            *out_error = "invalid_recurrence";
        }
        return false;
    }

    if (!(is_json
              ? net_json_get_weekdays_value(cursor, &weekdays_mask, &weekdays_found)
              : net_form_get_weekdays_value(cursor, &weekdays_mask, &weekdays_found)))
    {
        if (NULL != out_error)
        {
            *out_error = "invalid_weekdays";
        }
        return false;
    }

    SSP_PARAMETER_NOT_USED(recurrence_found);

    if (has_delay)
    {
        ULONG now_utc = 0U;

        if ((0U == delay_seconds) || (delay_seconds > NET_MEETING_SCHEDULE_MAX_DELAY_SECONDS))
        {
            if (NULL != out_error)
            {
                *out_error = "invalid_delay";
            }
            return false;
        }

        if (!app_time_get_utc(&now_utc))
        {
            if (NULL != out_error)
            {
                *out_error = "time_not_synced";
            }
            return false;
        }

        if ((0xFFFFFFFFUL - now_utc) < delay_seconds)
        {
            if (NULL != out_error)
            {
                *out_error = "invalid_delay";
            }
            return false;
        }

        *out_start_utc = now_utc + delay_seconds;
        *out_delay_seconds = delay_seconds;
    }
    else if (has_start)
    {
        ULONG now_utc = 0U;

        if ((0U == start_utc) || (app_time_get_utc(&now_utc) && (start_utc <= now_utc)))
        {
            if (NULL != out_error)
            {
                *out_error = "start_in_past";
            }
            return false;
        }

        *out_start_utc = start_utc;
        *out_delay_seconds = 0U;
    }
    else
    {
        if (NULL != out_error)
        {
            *out_error = "missing_start";
        }
        return false;
    }

    if (!(is_json
              ? net_json_get_profile_indices(cursor, out_profiles, STORAGE_MAX_USERS, &profile_count)
              : net_form_get_profile_indices(cursor, out_profiles, STORAGE_MAX_USERS, &profile_count)))
    {
        if (NULL != out_error)
        {
            *out_error = "invalid_profiles";
        }
        return false;
    }

    if (!net_meeting_schedule_validate_profiles(out_profiles, profile_count, out_error))
    {
        return false;
    }

    if (STORAGE_MEETING_RECURRENCE_WEEKLY == recurrence)
    {
        if (!weekdays_found || (0U == (weekdays_mask & STORAGE_MEETING_WEEKDAY_MASK_ALL)))
        {
            weekdays_mask = (uint8_t) (1U << net_meeting_weekday_from_unix(start_utc));
        }
        else
        {
            weekdays_mask = (uint8_t) (weekdays_mask & STORAGE_MEETING_WEEKDAY_MASK_ALL);
        }
    }
    else if (STORAGE_MEETING_RECURRENCE_DAILY == recurrence)
    {
        weekdays_mask = 0U;
    }
    else
    {
        recurrence = STORAGE_MEETING_RECURRENCE_NONE;
        weekdays_mask = 0U;
    }

    *out_recurrence = recurrence;
    *out_weekdays_mask = weekdays_mask;
    *out_profile_count = profile_count;
    return true;
}

static bool net_meeting_schedule_advance_recurring(storage_meeting_schedule_t *schedule, ULONG now_utc)
{
    uint64_t next_unix = 0ULL;

    if (NULL == schedule)
    {
        return false;
    }

    if (STORAGE_MEETING_RECURRENCE_DAILY == schedule->recurrence)
    {
        uint64_t start_unix = (uint64_t) schedule->start_unix;

        if (start_unix <= (uint64_t) now_utc)
        {
            uint64_t missed_days = (((uint64_t) now_utc - start_unix) / 86400ULL) + 1ULL;
            next_unix = start_unix + (missed_days * 86400ULL);
        }
        else
        {
            next_unix = start_unix;
        }
    }
    else if (STORAGE_MEETING_RECURRENCE_WEEKLY == schedule->recurrence)
    {
        uint8_t mask = (uint8_t) (schedule->weekdays_mask & STORAGE_MEETING_WEEKDAY_MASK_ALL);
        uint64_t seconds_of_day = ((uint64_t) schedule->start_unix) % 86400ULL;
        uint64_t base_day = ((uint64_t) now_utc) / 86400ULL;

        if (0U == mask)
        {
            mask = (uint8_t) (1U << net_meeting_weekday_from_unix(schedule->start_unix));
        }

        for (uint64_t delta = 0ULL; delta <= 7ULL; delta++)
        {
            uint64_t candidate_day = base_day + delta;
            uint8_t weekday = (uint8_t) ((candidate_day + 4ULL) % 7ULL);

            if (0U == (mask & (uint8_t) (1U << weekday)))
            {
                continue;
            }

            next_unix = (candidate_day * 86400ULL) + seconds_of_day;
            if (next_unix > (uint64_t) now_utc)
            {
                break;
            }
            next_unix = 0ULL;
        }
    }
    else
    {
        return false;
    }

    if ((0ULL == next_unix) || (next_unix > 0xFFFFFFFFULL))
    {
        return false;
    }

    schedule->start_unix = (ULONG) next_unix;
    return true;
}

static void net_process_meeting_schedule(void)
{
    storage_meeting_schedule_t due_schedule;
    int due_profile_indices[STORAGE_MAX_USERS];
    bool should_start = false;
    ULONG now_utc = 0U;
    unsigned int selected_profiles = 0U;
    unsigned int allowed_cards = 0U;
    bool started;

    if (!net_meeting_schedule_ensure_loaded() || !app_time_get_utc(&now_utc))
    {
        return;
    }

    memset(&due_schedule, 0, sizeof(due_schedule));
    net_action_lock();
    for (int i = 0; i < g_net_meeting_schedule_count; i++)
    {
        if (now_utc >= g_net_meeting_schedules[i].start_unix)
        {
            due_schedule = g_net_meeting_schedules[i];
            if (net_meeting_schedule_advance_recurring(&g_net_meeting_schedules[i], now_utc))
            {
                /* Mantem o mesmo id e apenas move a proxima ocorrencia para o futuro. */
            }
            else
            {
                for (int move_index = i; move_index < (g_net_meeting_schedule_count - 1); move_index++)
                {
                    g_net_meeting_schedules[move_index] = g_net_meeting_schedules[move_index + 1];
                }
                g_net_meeting_schedule_count--;
                if (g_net_meeting_schedule_count < 0)
                {
                    g_net_meeting_schedule_count = 0;
                }
            }

            g_net_meeting_schedule_last_id = due_schedule.id;
            g_net_meeting_schedule_last_start_utc = due_schedule.start_unix;
            net_meeting_schedule_set_status_locked("starting");
            (void) net_meeting_schedule_save_locked();
            should_start = true;
            break;
        }
    }
    net_action_unlock();

    if (!should_start)
    {
        return;
    }

    for (unsigned int i = 0U; i < due_schedule.profile_count; i++)
    {
        due_profile_indices[i] = (int) due_schedule.profile_indices[i];
    }

    started = storage_meeting_mode_start(due_profile_indices,
                                         (int) due_schedule.profile_count,
                                         &selected_profiles,
                                         &allowed_cards);

    net_action_lock();
    g_net_meeting_schedule_last_selected = selected_profiles;
    g_net_meeting_schedule_last_allowed = allowed_cards;
    net_meeting_schedule_set_status_locked(started ? "started" : "failed");
    net_action_unlock();
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
    const char *message_to_render = message;
    const bool render_content_card = (((NULL != title) && ('\0' != title[0])) ||
                                      ((NULL != body_html) && ('\0' != body_html[0])));
    const char *content_card_open = render_content_card ? "<div class='card'>" : "";
    const char *content_card_close = render_content_card ? "</div>" : "";
    const char *content_title_open = ((NULL != title) && ('\0' != title[0])) ? "<h2>" : "";
    const char *content_title_text = ((NULL != title) && ('\0' != title[0])) ? title : "";
    const char *content_title_close = ((NULL != title) && ('\0' != title[0])) ? "</h2>" : "";
    char ip_text[20];
    char netmask_text[20];
    ULONG ip_address = 0U;
    ULONG network_mask = 0U;
    ULONG link_status = 0U;
    bool is_admin = net_admin_is_authenticated();
    storage_debug_info_t storage_debug;
    char storage_debug_line[256];

    nx_ip_address_get(&g_ip0, &ip_address, &network_mask);
    ip_to_string(ip_address, ip_text, sizeof(ip_text));
    ip_to_string(network_mask, netmask_text, sizeof(netmask_text));
    (void) nx_ip_status_check(&g_ip0, NX_IP_LINK_ENABLED, &link_status, NX_NO_WAIT);
    storage_debug_snapshot(&storage_debug);
    if (is_admin)
    {
        snprintf(storage_debug_line,
                 sizeof(storage_debug_line),
                 "<p class='muted'>QSPI diag: <strong>stage=%lu media=%lu save=%lu load=%lu bytes=%lu users=%lu runs=%lu loaded=%u failed=%u</strong></p>",
                 (unsigned long) storage_debug.last_stage,
                 (unsigned long) storage_debug.last_media_status,
                 (unsigned long) storage_debug.last_save_status,
                 (unsigned long) storage_debug.last_load_status,
                 (unsigned long) storage_debug.last_saved_bytes,
                 (unsigned long) storage_debug.last_user_count,
                 (unsigned long) storage_debug.worker_runs,
                 storage_debug.loaded ? 1U : 0U,
                 storage_debug.load_failed ? 1U : 0U);
    }
    else
    {
        storage_debug_line[0] = '\0';
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
             "<div class='card'><h1>Ramo Estudantil IEEE UFJF - Controle de acesso</h1>"
             "<p class='muted'>IP atual: <strong>%s</strong> | Mascara: <strong>%s</strong> | Rede: <strong>DHCP</strong></p>"
             "<p class='%s'>Link Ethernet: <strong>%s</strong></p>"
             "<p class='muted'>Sessao admin web: <strong>%s</strong></p>"
             "<p class='muted'>Persistencia QSPI: <strong>%s</strong></p>"
             "%s"
             "<div class='nav'><a class='small' href='/'>Inicio</a>%s%s</div></div>"
             "%s"
             "%s%s%s%s%s%s"
             "</div></body></html>",
             ip_text,
             netmask_text,
             (0U != link_status) ? "ok" : "warn",
             (0U != link_status) ? "conectado" : "sem link",
             is_admin ? "autenticada" : "bloqueada",
             light_persist_status_text(),
             storage_debug_line,
             is_admin ? "<a class='small' href='/admin_profiles'>Perfis</a><a class='small' href='/profile_form'>Novo perfil</a><a class='small' href='/upload_photo'>Upload foto</a><a class='small' href='/import'>Importar</a><a class='small' href='/storage_export'>Downloads</a><a class='small' href='/access_log'>Log</a><a class='small' href='/meeting_mode'>Reuniao</a><a class='small' href='/metrics'>Metricas</a><a class='small' href='/door'>Porta</a>" : "",
             is_admin ? "<a class='small secondary' href='/logout'>Sair</a>" : "<a class='small' href='/login'>Entrar</a>",
             (NULL != message_to_render) ? message_to_render : "",
             content_card_open,
             content_title_open,
             content_title_text,
             content_title_close,
             (NULL != body_html) ? body_html : "",
             content_card_close);

    return send_html_response(server_ptr, packet_ptr, html);
}

static UINT render_light_login_page(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, const char *message)
{
    static char body[2048];

    if (net_admin_is_authenticated())
    {
        snprintf(body,
                 sizeof(body),
                 "<p class='ok'>Você já está conectado como administrador.</p>"
                 "<div class='actions'><a class='small' href='/admin_profiles'>Ir para perfis</a><a class='small secondary' href='/logout'>Encerrar sessão</a></div>");
    }
    else
    {
        snprintf(body,
                 sizeof(body),
                 "<p class='muted'>Digite o PIN de um perfil administrador para utilizar o sistema.</p>"
                 "<form action='/login' method='post'>"
                 "<label>PIN admin</label><input type='password' name='pin' inputmode='numeric' maxlength='4' placeholder='Digite 4 digitos'>"
                 "<button class='btn' type='submit'>Entrar</button></form>");
    }

    return render_light_shell(server_ptr, packet_ptr, "Autenticacao admin", body, message);
}

static UINT render_light_dashboard_page(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, const char *message)
{
    static char html[3072];
    const char *message_to_render = message;
    bool is_admin = net_admin_is_authenticated();

    if ((NULL == message_to_render) && ('\0' != g_net_flash_message[0]))
    {
        message_to_render = g_net_flash_message;
        g_net_flash_message[0] = '\0';
    }

    snprintf(html,
             sizeof(html),
             "<html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'>"
             "<style>"
             "body{font-family:Arial,sans-serif;background:#f5f7fb;color:#18212d;margin:0;padding:18px;}"
             ".wrap{max-width:920px;margin:0 auto;}.card{background:#fff;border-radius:16px;padding:18px;margin-bottom:14px;box-shadow:0 8px 26px rgba(0,0,0,.08);}"
             ".nav{display:flex;flex-wrap:wrap;gap:8px;margin-top:12px;}.small{display:inline-block;text-decoration:none;border-radius:10px;padding:10px 14px;background:#0b6ef3;color:#fff;}.secondary{background:#6b7a90;}"
             ".muted{color:#607086;font-size:14px;}.ok{color:#137333;}.warn{color:#b26a00;}"
             "@media(max-width:720px){body{padding:12px;}.nav{flex-direction:column;}.small{display:block;text-align:center;}}"
             "</style></head><body><div class='wrap'><div class='card'>"
             "<h1>Ramo Estudantil IEEE UFJF</h1>"
             "<p class='muted'>Controle de acesso - painel web leve.</p>"
             "<p class='muted'>Sessao admin web: <strong>%s</strong></p>"
             "<p class='muted'>Persistencia QSPI: <strong>%s</strong></p>"
             "<div class='nav'><a class='small' href='/health'>Health</a>%s%s</div>"
             "</div>%s</div></body></html>",
             is_admin ? "autenticada" : "bloqueada",
             light_persist_status_text(),
             is_admin ? "<a class='small' href='/admin_profiles'>Perfis</a><a class='small' href='/profile_form'>Novo perfil</a><a class='small' href='/storage_export'>Downloads</a><a class='small' href='/access_log'>Log</a><a class='small' href='/meeting_mode'>Reuniao</a><a class='small' href='/metrics'>Metricas</a><a class='small' href='/door'>Porta</a>" : "",
             is_admin ? "<a class='small secondary' href='/logout'>Sair</a>" : "<a class='small' href='/login'>Entrar</a>",
             (NULL != message_to_render) ? message_to_render : "");

    return send_html_response(server_ptr, packet_ptr, html);
}

static UINT render_light_profiles_page(NX_HTTP_SERVER *server_ptr,
                                       NX_PACKET *packet_ptr,
                                       const char *message,
                                       int page)
{
    static char html[8192];
    static char rows[4096];
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

    user_count = storage_user_count_nowait();
    if (user_count > STORAGE_MAX_USERS)
    {
        user_count = STORAGE_MAX_USERS;
    }
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
    snprintf(prev_href, sizeof(prev_href), "/admin_profiles/%d", has_prev ? (page - 1) : 0);
    snprintf(next_href, sizeof(next_href), "/admin_profiles/%d", page + 1);
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
                "<tr><td colspan='7'>Nenhum perfil carregado no momento.</td></tr>",
                sizeof(rows) - 1U);
        rows[sizeof(rows) - 1U] = '\0';
    }
    else
    {
        for (int i = start_index; i < end_index; i++)
        {
            storage_user_profile_t profile;
            char cards_csv[FORM_CARDS_BUFFER_SIZE];
            char cards_html[(UID_MAX_LEN * STORAGE_MAX_CARDS_PER_USER * 6) + 16];
            char name_html[NAME_MAX_LEN * 6];
            char role_html[STORAGE_ROLE_MAX_LEN * 6];
            char chapter_html[STORAGE_CHAPTER_MAX_LEN * 6];
            int written;

            if (!storage_profile_get_nowait(i, &profile))
            {
                continue;
            }

            profile_cards_to_csv(&profile, cards_csv, sizeof(cards_csv));
            net_html_escape(profile.name, name_html, sizeof(name_html));
            net_html_escape(('\0' != profile.role[0]) ? profile.role : "-", role_html, sizeof(role_html));
            net_html_escape(('\0' != profile.chapter[0]) ? profile.chapter : "-", chapter_html, sizeof(chapter_html));
            net_html_escape(('\0' != cards_csv[0]) ? cards_csv : "-", cards_html, sizeof(cards_html));
            written = snprintf(&rows[rows_len],
                               sizeof(rows) - rows_len,
                               "<tr><td>%d</td><td>%s</td><td>%s</td><td>%s</td><td>%s</td><td>%s</td><td>"
                               "<a class='small' href='/profile_form/%d'>Editar</a> "
                               "<a class='small danger' href='/remove_user?index=%d'>Remover</a>"
                               "</td></tr>",
                               i,
                               name_html,
                               role_html,
                               chapter_html,
                               profile.is_admin ? "Sim" : "Nao",
                               cards_html,
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
             "<div class='actions'><a class='small' href='/'>Inicio</a><a class='small' href='/profile_form'>Novo perfil</a><a class='small' href='/import'>Importar perfis</a><a class='small' href='/storage_export'>Exportar storage</a><a class='small' href='/save_users'>Persistir cadastros</a><a class='small secondary' href='/'>Voltar</a></div>"
             "%s"
             "%s"
             "<p class='muted'>Mostrando %d a %d de %d perfis carregados.</p>"
             "<div class='table-wrap'><table><tr><th>Indice</th><th>Nome</th><th>Cargo</th><th>Capitulo</th><th>Admin</th><th>Cartoes</th><th>Acao</th></tr>%s</table></div>"
             "<div class='actions'>"
             "%s"
             "%s"
             "</div>"
             "</div></div></body></html>",
             (NULL != message_to_render) ? message_to_render : "",
             (user_count > 0)
                 ? ""
                 : "<p class='warn'>Se você acabou de ligar a placa, aguarde alguns segundos e recarregue. Esta página usa a memória RAM para não travar o sistema.</p>",
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
    static char form_name_html[NAME_MAX_LEN * 6];
    static char form_role_html[STORAGE_ROLE_MAX_LEN * 6];
    static char form_chapter_html[STORAGE_CHAPTER_MAX_LEN * 6];
    static char form_cards_html[FORM_CARDS_BUFFER_SIZE * 6];
    static char form_photo_html[STORAGE_PHOTO_ID_MAX_LEN * 6];
    static char form_admin_pin_html[(STORAGE_ADMIN_PIN_MAX_LEN + 8) * 6];
    storage_user_profile_t form_profile;
    char form_cards[FORM_CARDS_BUFFER_SIZE];
    const char *admin_checked = "";
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
    user_count = storage_user_count_nowait();
    if (user_count > STORAGE_MAX_USERS)
    {
        user_count = STORAGE_MAX_USERS;
    }

    if ((edit_index >= 0) && storage_profile_get_nowait(edit_index, &form_profile))
    {
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
    admin_checked = form_profile.is_admin ? " checked" : "";
    if ('\0' != form_profile.photo_id[0])
    {
        current_photo_status = net_photo_asset_exists(form_profile.photo_id) ? "asset encontrado" : "asset não encontrado";
    }

    net_html_escape(form_profile.name, form_name_html, sizeof(form_name_html));
    net_html_escape(form_profile.role, form_role_html, sizeof(form_role_html));
    net_html_escape(form_profile.chapter, form_chapter_html, sizeof(form_chapter_html));
    net_html_escape(form_cards, form_cards_html, sizeof(form_cards_html));
    net_html_escape(form_profile.photo_id, form_photo_html, sizeof(form_photo_html));
    form_admin_pin_html[0] = '\0';

    snprintf(html,
             sizeof(html),
             "<html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'>"
             "<style>"
             "body{font-family:Arial,sans-serif;background:#f5f7fb;color:#18212d;margin:0;padding:18px;}"
             ".wrap{max-width:760px;margin:0 auto;}"
             ".card{background:#fff;border-radius:16px;padding:18px;margin-bottom:16px;box-shadow:0 8px 26px rgba(0,0,0,.08);}"
             "input,select{width:100%%;padding:12px;border:1px solid #ccd4e0;border-radius:10px;margin:8px 0 12px 0;box-sizing:border-box;font-family:Consolas,monospace;font-size:16px;}"
             ".checkline{display:flex;align-items:center;gap:10px;margin:8px 0 12px 0;font-size:15px;}"
             ".checkline input{width:auto;margin:0;}"
             ".btn,.small{display:inline-block;text-decoration:none;border:none;border-radius:10px;padding:12px 16px;background:#0b6ef3;color:#fff;cursor:pointer;}"
             ".small{padding:8px 12px;font-size:13px;margin-right:8px;}.secondary{background:#6b7a90;}"
             ".muted{color:#607086;font-size:14px;}.warn{color:#b26a00;}.actions{display:flex;flex-wrap:wrap;gap:10px;margin-top:10px;}"
             "@media(max-width:720px){body{padding:12px;}.wrap{max-width:100%%;}.card{padding:14px;border-radius:14px;}.actions{flex-direction:column;align-items:stretch;gap:8px;}.btn,.small{display:block;width:100%%;box-sizing:border-box;text-align:center;margin-right:0;}}"
             "</style></head><body><div class='wrap'>"
             "<div class='card'><h2>%s</h2>"
             "<div class='actions'><a class='small' href='/'>Inicio</a><a class='small' href='/admin_profiles'>Perfis</a><a class='small secondary' href='/admin_profiles'>Voltar</a></div>"
             "%s"
             "%s"
             "<form action='/add_user' method='post'>"
             "<input type='hidden' name='edit' value='%d'>"
             "<label>Nome</label><input type='text' name='name' maxlength='31' value='%s' placeholder='Nome do usuario'>"
             "<label>Cargo</label><input type='text' name='role' maxlength='47' value='%s' placeholder='Ex.: Presidente'>"
             "<label>Capitulo IEEE</label><input type='text' name='chapter' maxlength='47' value='%s' placeholder='Ex.: Computer Society'>"
             "<label class='checkline'><input type='checkbox' name='is_admin' value='1'%s> Perfil administrador</label>"
             "<label>PIN administrador</label><input type='password' name='admin_pin' inputmode='numeric' maxlength='4' value='%s' placeholder='4 digitos. Em edicao, deixe vazio para manter o atual'>"
             "<label>Cartoes (separados por virgula)</label><input type='text' name='cards' maxlength='255' value='%s' placeholder='E35C051C,1234ABCD'>"
             "<label>Foto (identificador)</label><input type='text' name='photo_id' list='photo-id-list' maxlength='63' value='%s' placeholder='Escolha um photo_id importado'>"
             "<div class='actions'><button class='btn' type='submit'>Salvar perfil</button></div></form>"
             "<p class='muted'>Esta página aceita mais de um cartão por perfil, com os IDs separados por vírgula.</p>"
             "</div></div></body></html>",
             editing ? "Editar perfil" : "Criar perfil",
             (NULL != message_to_render) ? message_to_render : "",
             (user_count > 0)
                 ? ""
                 : "<p class='warn'>Os perfis ainda nao foram carregados. Se necessario, volte para a página inicial e tente novamente em alguns segundos.</p>",
             editing ? edit_index : -1,
             form_name_html,
             form_role_html,
             form_chapter_html,
             admin_checked,
             form_admin_pin_html,
             form_cards_html,
             form_photo_html);

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
              "<label>Importar perfis via JSON</label>"
              "<textarea name='import_json' placeholder='Cole aqui o JSON gerado em script/firebase_bundle/profiles_import.json'></textarea>"
              "<div class='actions'><button class='btn' type='submit'>Importar perfis</button><a class='small secondary' href='/admin_profiles'>Voltar</a></div></form>"
              "<p class='muted'>Use o JSON simples. Os cartoes podem ficar vazios e ser vinculados depois.</p>"
              "<p class='muted'>A importação roda em segundo plano para nao travar a interface web.</p>");

    return render_light_shell(server_ptr, packet_ptr, "Importar perfis offline", body, message);
}

static UINT render_light_upload_page(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, const char *message)
{
    static char html[4096];
    static char profile_options[PROFILE_OPTIONS_BUFFER_SIZE];
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

    net_build_profile_options(profile_options, sizeof(profile_options));

    snprintf(html,
             sizeof(html),
             "<html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'>"
             "<style>"
             "body{font-family:Arial,sans-serif;margin:16px;background:#f5f7fb;color:#18212d;}"
             ".card{max-width:560px;background:#fff;padding:16px;border-radius:14px;box-shadow:0 6px 20px rgba(0,0,0,.08);}"
             "input,select,button{width:100%%;box-sizing:border-box;font-size:16px;padding:10px;margin:8px 0;}"
             "a{display:inline-block;margin-right:8px;margin-bottom:8px;}"
             ".m{color:#5e6c84;font-size:14px;}"
             "</style></head><body><div class='card'>"
             "<h2>Upload de foto</h2>"
             "<p><a href='/'>Inicio</a><a href='/admin_profiles'>Perfis</a><a href='/admin_profiles'>Voltar</a></p>"
             "%s"
             "<p class='m'>Trocar foto de usuário. Selecione o perfil:</p>"
             "<label>Perfil</label><select id='photo-profile'>%s</select>"
             "<label>Arquivo de imagem</label><input id='photo-file' type='file' accept='image/*'>"
             "<p id='upload-status' class='m'>Selecione uma imagem para preparar o envio.</p>"
             "<button id='upload-submit' type='button' disabled>Salvar nova foto</button>"
             "<iframe id='upload-target' name='upload-target' style='display:none;'></iframe>"
             "<form id='upload-stage-form' method='post' target='upload-target' style='display:none;'>"
             "<input type='hidden' id='upload-stage-profile' name='profile_index' value=''>"
             "<input type='hidden' id='upload-stage-width' name='width' value='%u'>"
             "<input type='hidden' id='upload-stage-height' name='height' value='%u'>"
             "<input type='hidden' id='upload-stage-tile' name='tile' value='0'>"
             "<textarea id='upload-stage-data' name='data' style='display:none;'></textarea>"
             "</form>"
             "<script src='/upload_photo_script'></script>"
             "</div></body></html>",
             (NULL != message_to_render) ? message_to_render : "",
             ('\0' != profile_options[0]) ? profile_options : "<option value=''>Nenhum perfil disponivel</option>",
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
             "submit.addEventListener('click',function(){const idx=(profile.value||'').trim();if(!idx){status.textContent='Selecione um perfil antes de enviar.';return;}if(!submit.dataset.tiles){status.textContent='Selecione uma imagem primeiro.';return;}if(state){status.textContent='Ja existe um upload em andamento.';return;}submit.disabled=true;stageProfile.value=idx;stageWidth.value=String(dim);stageHeight.value=String(dim);stageTile.value='0';stageData.value='';state={tiles:JSON.parse(submit.dataset.tiles),tile:0,phase:'begin'};status.textContent='Iniciando upload em 16 blocos...';post('/upload_photo_begin');});"
             "})();",
             (unsigned int) UPLOAD_IMAGE_DIM,
             (unsigned int) UPLOAD_TILE_DIM);

    return send_javascript_response(server_ptr, packet_ptr, script);
}

static UINT render_light_metrics_page(NX_HTTP_SERVER *server_ptr,
                                      NX_PACKET *packet_ptr,
                                      const char *message,
                                      int page)
{
    const char *message_to_render = message;
    int metric_count;
    int group_count = 0;
    size_t body_len = 0U;
    int start_index;
    int end_index;
    bool has_prev;
    bool has_next;
    char prev_button[96];
    char next_button[96];

    if (!net_admin_is_authenticated())
    {
        return render_light_login_page(server_ptr, packet_ptr, "<div class='card warn'>Autentique-se para visualizar as metricas.</div>");
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

    metric_count = app_metric_snapshot(g_net_metric_snapshot, APP_METRIC_LOG_SIZE);
    memset(g_net_metric_groups, 0, sizeof(g_net_metric_groups));
    g_net_metric_buffer[0] = '\0';

    for (int i = 0; i < metric_count; i++)
    {
        int group_index = -1;

        if (APP_METRIC_KIND_UNKNOWN == g_net_metric_snapshot[i].kind)
        {
            continue;
        }

        for (int g = 0; g < group_count; g++)
        {
            if ((g_net_metric_groups[g].kind == g_net_metric_snapshot[i].kind) &&
                (g_net_metric_groups[g].case_id == g_net_metric_snapshot[i].case_id))
            {
                group_index = g;
                break;
            }
        }

        if ((group_index < 0) && (group_count < METRIC_GROUP_MAX))
        {
            group_index = group_count++;
            g_net_metric_groups[group_index].kind = g_net_metric_snapshot[i].kind;
            g_net_metric_groups[group_index].case_id = g_net_metric_snapshot[i].case_id;
            g_net_metric_groups[group_index].min_ticks = g_net_metric_snapshot[i].duration_ticks;
            g_net_metric_groups[group_index].max_ticks = g_net_metric_snapshot[i].duration_ticks;
        }

        if (group_index >= 0)
        {
            g_net_metric_groups[group_index].count++;
            g_net_metric_groups[group_index].sum_ticks += g_net_metric_snapshot[i].duration_ticks;
            if (g_net_metric_snapshot[i].duration_ticks < g_net_metric_groups[group_index].min_ticks)
            {
                g_net_metric_groups[group_index].min_ticks = g_net_metric_snapshot[i].duration_ticks;
            }
            if (g_net_metric_snapshot[i].duration_ticks > g_net_metric_groups[group_index].max_ticks)
            {
                g_net_metric_groups[group_index].max_ticks = g_net_metric_snapshot[i].duration_ticks;
            }
            if (!g_net_metric_snapshot[i].success)
            {
                g_net_metric_groups[group_index].failures++;
            }
        }
    }

    start_index = page * METRICS_PAGE_SIZE;
    if ((start_index >= group_count) && (group_count > 0))
    {
        start_index = 0;
        page = 0;
    }
    end_index = start_index + METRICS_PAGE_SIZE;
    if (end_index > group_count)
    {
        end_index = group_count;
    }
    has_prev = (page > 0);
    has_next = (end_index < group_count);
    if (has_prev)
    {
        snprintf(prev_button, sizeof(prev_button), "<a class='small secondary' href='/metrics/%d'>Anterior</a>", page - 1);
    }
    else
    {
        prev_button[0] = '\0';
    }
    if (has_next)
    {
        snprintf(next_button, sizeof(next_button), "<a class='small' href='/metrics/%d'>Proxima</a>", page + 1);
    }
    else
    {
        next_button[0] = '\0';
    }

    if (group_count <= 0)
    {
        body_len = (size_t) snprintf(g_net_metric_buffer,
                                     sizeof(g_net_metric_buffer),
                                     "<p class='muted'>As metricas sao coletadas automaticamente com <code>tx_time_get()</code>, persistidas na QSPI e exibidas em paginas menores para nao travar a interface.</p>"
                                     "<p class='muted'>Cada pagina mostra ate %d grupos agregados.</p>"
                                     "<div class='actions'><a class='small' href='/metrics_download' download='metrics_log.csv'>Baixar log bruto (CSV)</a><a class='small secondary' href='/admin_profiles'>Voltar</a></div>"
                                     "<p class='muted'>Mostrando 0 a 0 de 0 grupos agregados.</p>"
                                     "<div class='table-wrap'><table><tr><th>Metrica</th><th>Caso</th><th>N</th><th>Media</th><th>Mediana</th><th>Min</th><th>Max</th><th>Desvio padrao</th><th>Falhas</th></tr>"
                                     "<tr><td colspan='9'>Nenhuma metrica coletada ainda.</td></tr></table></div>",
                                     METRICS_PAGE_SIZE);
    }
    else
    {
        body_len = (size_t) snprintf(g_net_metric_buffer,
                                     sizeof(g_net_metric_buffer),
                                     "<p class='muted'>As metricas sao coletadas automaticamente com <code>tx_time_get()</code>, persistidas na QSPI e exibidas em paginas menores para nao travar a interface.</p>"
                                     "<p class='muted'>Cada pagina mostra ate %d grupos agregados.</p>"
                                     "<div class='actions'><a class='small' href='/metrics_download' download='metrics_log.csv'>Baixar log bruto (CSV)</a><a class='small secondary' href='/admin_profiles'>Voltar</a></div>"
                                     "<p class='muted'>Mostrando %d a %d de %d grupos agregados.</p>"
                                     "<div class='table-wrap'><table><tr><th>Metrica</th><th>Caso</th><th>N</th><th>Media</th><th>Mediana</th><th>Min</th><th>Max</th><th>Desvio padrao</th><th>Falhas</th></tr>",
                                     METRICS_PAGE_SIZE,
                                     start_index + 1,
                                     end_index,
                                     group_count);

        for (int g = start_index; g < end_index; g++)
        {
            int duration_count = 0;
            uint64_t mean_ms_x10;
            uint64_t median_ms_x10;
            uint64_t stddev_ms_x10 = 0U;
            char mean_text[24];
            char median_text[24];
            char min_text[24];
            char max_text[24];
            char stddev_text[24];

            for (int i = 0; i < metric_count; i++)
            {
                if ((g_net_metric_groups[g].kind == g_net_metric_snapshot[i].kind) &&
                    (g_net_metric_groups[g].case_id == g_net_metric_snapshot[i].case_id) &&
                    (duration_count < APP_METRIC_LOG_SIZE))
                {
                    g_net_metric_durations[duration_count++] = g_net_metric_snapshot[i].duration_ticks;
                }
            }

            for (int i = 1; i < duration_count; i++)
            {
                ULONG value = g_net_metric_durations[i];
                int j = i - 1;
                while ((j >= 0) && (g_net_metric_durations[j] > value))
                {
                    g_net_metric_durations[j + 1] = g_net_metric_durations[j];
                    j--;
                }
                g_net_metric_durations[j + 1] = value;
            }

            mean_ms_x10 = (g_net_metric_groups[g].sum_ticks * 10000ULL) /
                          ((uint64_t) TX_TIMER_TICKS_PER_SECOND * (uint64_t) g_net_metric_groups[g].count);
            if (duration_count > 0)
            {
                if (0 != (duration_count % 2))
                {
                    median_ms_x10 = net_ticks_to_ms_x10(g_net_metric_durations[duration_count / 2]);
                }
                else
                {
                    uint64_t median_ticks_x10 = ((uint64_t) g_net_metric_durations[(duration_count / 2) - 1] +
                                                 (uint64_t) g_net_metric_durations[duration_count / 2]) * 5ULL;
                    median_ms_x10 = (median_ticks_x10 * 1000ULL) / (uint64_t) TX_TIMER_TICKS_PER_SECOND;
                }

                if (g_net_metric_groups[g].count > 0)
                {
                    uint64_t mean_ticks_x100 = (g_net_metric_groups[g].sum_ticks * 100ULL) / (uint64_t) g_net_metric_groups[g].count;
                    uint64_t variance_acc = 0U;

                    for (int i = 0; i < duration_count; i++)
                    {
                        int64_t diff = ((int64_t) g_net_metric_durations[i] * 100LL) - (int64_t) mean_ticks_x100;
                        variance_acc += (uint64_t) (diff * diff);
                    }

                    variance_acc /= (uint64_t) g_net_metric_groups[g].count;
                    stddev_ms_x10 = (net_isqrt64(variance_acc) * 100ULL) / (uint64_t) TX_TIMER_TICKS_PER_SECOND;
                }
            }
            else
            {
                median_ms_x10 = 0U;
            }

            net_format_ms_x10(mean_ms_x10, mean_text, sizeof(mean_text));
            net_format_ms_x10(median_ms_x10, median_text, sizeof(median_text));
            net_format_ms_x10(net_ticks_to_ms_x10(g_net_metric_groups[g].min_ticks), min_text, sizeof(min_text));
            net_format_ms_x10(net_ticks_to_ms_x10(g_net_metric_groups[g].max_ticks), max_text, sizeof(max_text));
            net_format_ms_x10(stddev_ms_x10, stddev_text, sizeof(stddev_text));

            {
                int written = snprintf(&g_net_metric_buffer[body_len],
                                       sizeof(g_net_metric_buffer) - body_len,
                                       "<tr><td>%s</td><td>%s</td><td>%d</td><td>%s</td><td>%s</td><td>%s</td><td>%s</td><td>%s</td><td>%d</td></tr>",
                                       app_metric_kind_text(g_net_metric_groups[g].kind),
                                       app_metric_case_text(g_net_metric_groups[g].case_id),
                                       g_net_metric_groups[g].count,
                                       mean_text,
                                       median_text,
                                       min_text,
                                       max_text,
                                       stddev_text,
                                       g_net_metric_groups[g].failures);
                if ((written <= 0) || ((size_t) written >= (sizeof(g_net_metric_buffer) - body_len)))
                {
                    break;
                }
                body_len += (size_t) written;
            }
        }
        if (body_len < sizeof(g_net_metric_buffer))
        {
            int written = snprintf(&g_net_metric_buffer[body_len],
                                   sizeof(g_net_metric_buffer) - body_len,
                                   "</table></div>");
            if ((written > 0) && ((size_t) written < (sizeof(g_net_metric_buffer) - body_len)))
            {
                body_len += (size_t) written;
            }
        }
    }

    if ((body_len < sizeof(g_net_metric_buffer)) &&
        ((strlen(prev_button) + strlen(next_button) + 32U) < (sizeof(g_net_metric_buffer) - body_len)))
    {
        (void) snprintf(&g_net_metric_buffer[body_len],
                        sizeof(g_net_metric_buffer) - body_len,
                        "<div class='actions'>%s%s</div>",
                        prev_button,
                        next_button);
    }

    return render_light_shell(server_ptr, packet_ptr, "Metricas de desempenho", g_net_metric_buffer, message_to_render);
}

static UINT handle_light_metrics_download(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr)
{
    static const char csv_header[] = "timestamp,metric,case,duration_ticks,duration_ms,success\r\n";
    char line_buffer[160];
    int metric_count;
    ULONG total_size = (ULONG) (sizeof(csv_header) - 1U);
    size_t chunk_len = 0U;

    if (!net_admin_is_authenticated())
    {
        return render_light_login_page(server_ptr, packet_ptr, "<div class='card warn'>Autentique-se para baixar o log de metricas.</div>");
    }

    metric_count = app_metric_snapshot(g_net_metric_snapshot, APP_METRIC_LOG_SIZE);
    for (int i = 0; i < metric_count; i++)
    {
        int written = net_format_metric_csv_line(line_buffer, sizeof(line_buffer), &g_net_metric_snapshot[i]);

        if ((written <= 0) || ((size_t) written >= sizeof(line_buffer)))
        {
            return send_plain_status_response(server_ptr, packet_ptr, NX_HTTP_STATUS_INTERNAL_ERROR, "ERR_EXPORT");
        }

        total_size += (ULONG) written;
    }

    if (NX_HTTP_CALLBACK_COMPLETED != send_stream_response_header(server_ptr, packet_ptr, total_size, "text/csv"))
    {
        return g_net_debug_last_response_status;
    }

    memcpy(g_net_download_chunk, csv_header, sizeof(csv_header) - 1U);
    chunk_len = sizeof(csv_header) - 1U;

    for (int i = 0; i < metric_count; i++)
    {
        int written = net_format_metric_csv_line(line_buffer, sizeof(line_buffer), &g_net_metric_snapshot[i]);

        if ((written <= 0) || ((size_t) written >= sizeof(line_buffer)))
        {
            return NX_NOT_SUCCESSFUL;
        }

        if ((chunk_len + (size_t) written) > sizeof(g_net_download_chunk))
        {
            if (NX_SUCCESS != nx_http_server_callback_data_send(server_ptr, g_net_download_chunk, (ULONG) chunk_len))
            {
                return NX_NOT_SUCCESSFUL;
            }
            chunk_len = 0U;
        }

        memcpy(&g_net_download_chunk[chunk_len], line_buffer, (size_t) written);
        chunk_len += (size_t) written;
    }

    if ((chunk_len > 0U) &&
        (NX_SUCCESS != nx_http_server_callback_data_send(server_ptr, g_net_download_chunk, (ULONG) chunk_len)))
    {
        return NX_NOT_SUCCESSFUL;
    }

    return NX_HTTP_CALLBACK_COMPLETED;
}

static UINT render_light_storage_export_page(NX_HTTP_SERVER *server_ptr,
                                             NX_PACKET *packet_ptr,
                                             const char *message,
                                             int page)
{
    const char *message_to_render = message;
    storage_user_profile_t profile;
    int profile_count;
    int start_index;
    int end_index;
    bool has_prev;
    bool has_next;
    char prev_button[96];
    char next_button[96];
    size_t body_len = 0U;
    int written;

    if (!net_admin_is_authenticated())
    {
        return render_light_login_page(server_ptr, packet_ptr, "<div class='card warn'>Autentique-se para exportar dados do storage.</div>");
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

    profile_count = storage_user_count_nowait();
    start_index = page * STORAGE_EXPORT_PAGE_SIZE;
    if ((start_index >= profile_count) && (profile_count > 0))
    {
        start_index = 0;
        page = 0;
    }
    end_index = start_index + STORAGE_EXPORT_PAGE_SIZE;
    if (end_index > profile_count)
    {
        end_index = profile_count;
    }
    has_prev = (page > 0);
    has_next = (end_index < profile_count);
    if (has_prev)
    {
        snprintf(prev_button, sizeof(prev_button), "<a class='small secondary' href='/storage_export/%d'>Anterior</a>", page - 1);
    }
    else
    {
        prev_button[0] = '\0';
    }
    if (has_next)
    {
        snprintf(next_button, sizeof(next_button), "<a class='small' href='/storage_export/%d'>Proxima</a>", page + 1);
    }
    else
    {
        next_button[0] = '\0';
    }

    written = snprintf(g_net_metric_buffer,
                       sizeof(g_net_metric_buffer),
                       "<p class='muted'>A listagem foi paginada para manter a interface leve. Cada pagina mostra ate %d perfis persistidos.</p>"
                       "<div class='actions'><a class='small' href='/storage_users_download' download='users.json'>Baixar users.json</a><a class='small secondary' href='/admin_profiles'>Voltar</a></div>"
                       "<p class='muted'>Mostrando %d a %d de %d perfis persistidos.</p>"
                       "<div class='table-wrap'><table><tr><th>Nome</th><th>Cargo</th><th>Photo ID</th><th>Arquivo</th></tr>",
                       STORAGE_EXPORT_PAGE_SIZE,
                       (profile_count > 0) ? (start_index + 1) : 0,
                       end_index,
                       profile_count);
    if ((written <= 0) || ((size_t) written >= sizeof(g_net_metric_buffer)))
    {
        return render_light_shell(server_ptr, packet_ptr, "Exportar dados do storage", "<p class='warn'>Nao foi possivel montar a pagina de exportacao.</p>", message_to_render);
    }
    body_len = (size_t) written;

    if (profile_count <= 0)
    {
        written = snprintf(&g_net_metric_buffer[body_len],
                           sizeof(g_net_metric_buffer) - body_len,
                           "<tr><td colspan='4'>Nenhum perfil carregado a partir do storage.</td></tr>");
        if ((written > 0) && ((size_t) written < (sizeof(g_net_metric_buffer) - body_len)))
        {
            body_len += (size_t) written;
        }
    }
    else
    {
        for (int i = start_index; i < end_index; i++)
        {
            char name_html[NAME_MAX_LEN * 6];
            char role_html[STORAGE_ROLE_MAX_LEN * 6];
            char photo_id_html[STORAGE_PHOTO_ID_MAX_LEN * 6];

            if (!storage_profile_get_nowait(i, &profile))
            {
                continue;
            }

            net_html_escape(profile.name, name_html, sizeof(name_html));
            net_html_escape(('\0' != profile.role[0]) ? profile.role : "-", role_html, sizeof(role_html));
            net_html_escape(profile.photo_id, photo_id_html, sizeof(photo_id_html));

            if ('\0' != profile.photo_id[0])
            {
                written = snprintf(&g_net_metric_buffer[body_len],
                                   sizeof(g_net_metric_buffer) - body_len,
                                   "<tr><td>%s</td><td>%s</td><td>%s</td><td><a class='small' href='/storage_photo_download?photo_id=%s' download>Baixar foto</a></td></tr>",
                                   name_html,
                                   role_html,
                                   photo_id_html,
                                   photo_id_html);
            }
            else
            {
                written = snprintf(&g_net_metric_buffer[body_len],
                                   sizeof(g_net_metric_buffer) - body_len,
                                   "<tr><td>%s</td><td>%s</td><td>-</td><td>-</td></tr>",
                                   name_html,
                                   role_html);
            }

            if ((written <= 0) || ((size_t) written >= (sizeof(g_net_metric_buffer) - body_len)))
            {
                break;
            }
            body_len += (size_t) written;
        }
    }

    if (body_len < sizeof(g_net_metric_buffer))
    {
        written = snprintf(&g_net_metric_buffer[body_len],
                           sizeof(g_net_metric_buffer) - body_len,
                           "</table></div>");
        if ((written > 0) && ((size_t) written < (sizeof(g_net_metric_buffer) - body_len)))
        {
            body_len += (size_t) written;
        }
    }

    if ((body_len < sizeof(g_net_metric_buffer)) &&
        ((strlen(prev_button) + strlen(next_button) + 32U) < (sizeof(g_net_metric_buffer) - body_len)))
    {
        (void) snprintf(&g_net_metric_buffer[body_len],
                        sizeof(g_net_metric_buffer) - body_len,
                        "<div class='actions'>%s%s</div>",
                        prev_button,
                        next_button);
    }

    return render_light_shell(server_ptr, packet_ptr, "Exportar dados do storage", g_net_metric_buffer, message_to_render);
}

static UINT handle_light_storage_users_download(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr)
{
    ULONG total_size = 0U;
    ULONG offset = 0U;

    if (!net_admin_is_authenticated())
    {
        return render_light_login_page(server_ptr, packet_ptr, "<div class='card warn'>Autentique-se para baixar os perfis persistidos.</div>");
    }

    if (!storage_export_users_json_info(&total_size) || (0U == total_size))
    {
        return light_redirect_with_flash(server_ptr, "/storage_export", "<div class='card warn'>Nao foi possivel ler o users.json da QSPI.</div>");
    }

    if (NX_HTTP_CALLBACK_COMPLETED != send_stream_response_header(server_ptr, packet_ptr, total_size, "application/json"))
    {
        return g_net_debug_last_response_status;
    }

    while (offset < total_size)
    {
        size_t read_now = 0U;
        size_t request_size = sizeof(g_net_download_chunk);

        if ((total_size - offset) < request_size)
        {
            request_size = (size_t) (total_size - offset);
        }

        if (!storage_export_users_json_read(offset, g_net_download_chunk, request_size, &read_now) || (0U == read_now))
        {
            return NX_NOT_SUCCESSFUL;
        }

        if (NX_SUCCESS != nx_http_server_callback_data_send(server_ptr, g_net_download_chunk, (ULONG) read_now))
        {
            return NX_NOT_SUCCESSFUL;
        }

        offset += (ULONG) read_now;
    }

    return NX_HTTP_CALLBACK_COMPLETED;
}

static UINT handle_light_storage_photo_download(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, const char *query)
{
    char photo_id[STORAGE_PHOTO_ID_MAX_LEN];
    storage_photo_export_info_t info;
    ULONG offset = 0U;

    if (!net_admin_is_authenticated())
    {
        return render_light_login_page(server_ptr, packet_ptr, "<div class='card warn'>Autentique-se para baixar as fotos persistidas.</div>");
    }

    if (!query_get_value(query, "photo_id", photo_id, sizeof(photo_id)))
    {
        return light_redirect_with_flash(server_ptr, "/storage_export", "<div class='card warn'>Selecione um photo_id valido.</div>");
    }

    if (!storage_photo_export_info(photo_id, &info))
    {
        return light_redirect_with_flash(server_ptr, "/storage_export", "<div class='card warn'>Nao foi possivel abrir a foto persistida na QSPI.</div>");
    }

    if (NX_HTTP_CALLBACK_COMPLETED != send_stream_response_header(server_ptr, packet_ptr, info.total_size, "application/octet-stream"))
    {
        return g_net_debug_last_response_status;
    }

    while (offset < info.total_size)
    {
        size_t read_now = 0U;
        size_t request_size = sizeof(g_net_download_chunk);

        if ((info.total_size - offset) < request_size)
        {
            request_size = (size_t) (info.total_size - offset);
        }

        if (!storage_photo_export_read(photo_id, offset, g_net_download_chunk, request_size, &read_now) || (0U == read_now))
        {
            return NX_NOT_SUCCESSFUL;
        }

        if (NX_SUCCESS != nx_http_server_callback_data_send(server_ptr, g_net_download_chunk, (ULONG) read_now))
        {
            return NX_NOT_SUCCESSFUL;
        }

        offset += (ULONG) read_now;
    }

    return NX_HTTP_CALLBACK_COMPLETED;
}

static UINT render_light_access_log_page(NX_HTTP_SERVER *server_ptr,
                                         NX_PACKET *packet_ptr,
                                         const char *message,
                                         int page)
{
    static char html[8192];
    static char rows[4096];
    static app_access_log_entry_t entries[ACCESS_LOG_SIZE];
    const char *message_to_render = message;
    char timestamp[32];
    size_t rows_len = 0U;
    int count;
    int start_offset;
    int page_items;
    bool has_prev;
    bool has_next;
    char prev_button[96];
    char next_button[96];
    char prev_href[32];
    char next_href[32];

    if (!net_admin_is_authenticated())
    {
        return render_light_login_page(server_ptr, packet_ptr, "<div class='card warn'>Autentique-se para ver o log de acesso.</div>");
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

    count = app_access_log_snapshot(entries, ACCESS_LOG_SIZE);
    rows[0] = '\0';
    start_offset = page * ACCESS_LOG_PAGE_SIZE;
    if ((count > 0) && (start_offset >= count))
    {
        page = 0;
        start_offset = 0;
    }
    page_items = (count > start_offset) ? (count - start_offset) : 0;
    if (page_items > ACCESS_LOG_PAGE_SIZE)
    {
        page_items = ACCESS_LOG_PAGE_SIZE;
    }
    has_prev = (page > 0);
    has_next = ((start_offset + page_items) < count);
    snprintf(prev_href, sizeof(prev_href), "/access_log/%d", has_prev ? (page - 1) : 0);
    snprintf(next_href, sizeof(next_href), "/access_log/%d", page + 1);
    if (has_prev)
    {
        snprintf(prev_button, sizeof(prev_button), "<a class='small secondary' href='%s'>Mais novos</a>", prev_href);
    }
    else
    {
        prev_button[0] = '\0';
    }
    if (has_next)
    {
        snprintf(next_button, sizeof(next_button), "<a class='small' href='%s'>Mais antigos</a>", next_href);
    }
    else
    {
        next_button[0] = '\0';
    }

    if (count <= 0)
    {
        strncpy(rows, "<tr><td colspan='4'>Nenhum evento registrado ainda.</td></tr>", sizeof(rows) - 1U);
        rows[sizeof(rows) - 1U] = '\0';
    }
    else
    {
        for (int item = 0; item < page_items; item++)
        {
            int entry_index = count - 1 - (start_offset + item);
            char data_html[UID_MAX_LEN * 6];
            char user_html[NAME_MAX_LEN * 6];

            net_format_access_log_timestamp(&entries[entry_index], timestamp, sizeof(timestamp));
            net_html_escape(('\0' != entries[entry_index].data[0]) ? entries[entry_index].data : "-",
                            data_html,
                            sizeof(data_html));
            net_html_escape(('\0' != entries[entry_index].user[0]) ? entries[entry_index].user : "-",
                            user_html,
                            sizeof(user_html));
            int written = snprintf(&rows[rows_len],
                                   sizeof(rows) - rows_len,
                                   "<tr><td>%s</td><td>%s</td><td>%s</td><td>%s</td></tr>",
                                   timestamp,
                                   net_event_type_text(entries[entry_index].type),
                                   data_html,
                                   user_html);
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
             ".secondary{background:#6b7a90;}.muted{color:#607086;font-size:14px;}.warn{color:#b26a00;}.ok{color:#137333;}"
             ".table-wrap{overflow-x:auto;-webkit-overflow-scrolling:touch;}table{width:100%%;border-collapse:collapse;}th,td{padding:10px;border-bottom:1px solid #e7ebf2;text-align:left;vertical-align:top;}"
             "@media(max-width:720px){body{padding:12px;}.wrap{max-width:100%%;}.card{padding:14px;border-radius:14px;}.actions{flex-direction:column;align-items:stretch;gap:8px;}.small{display:block;width:100%%;box-sizing:border-box;text-align:center;}.table-wrap{margin:0 -4px;}table{display:block;overflow-x:auto;-webkit-overflow-scrolling:touch;}th,td{padding:8px;font-size:13px;white-space:nowrap;}}"
             "</style></head><body><div class='wrap'><div class='card'>"
             "<h1>Log de acesso</h1>"
             "<div class='actions'><a class='small' href='/'>Inicio</a><a class='small' href='/admin_profiles'>Perfis</a><a class='small' href='/save_access_log'>Salvar log</a><a class='small secondary' href='/'>Voltar</a></div>"
             "%s"
             "<p class='muted'>A tela mostra os ultimos eventos em RAM. O arquivo persistido cresce em append na QSPI e gira por tamanho.</p>"
             "<p class='muted'>Mostrando %d evento(s) nesta pagina, de um total de %d em memoria.</p>"
             "<div class='table-wrap'><table><tr><th>Timestamp</th><th>Evento</th><th>Dado</th><th>Usuario</th></tr>%s</table></div>"
             "<div class='actions'>%s%s</div>"
             "</div></div></body></html>",
             (NULL != message_to_render) ? message_to_render : "",
             page_items,
             count,
             rows,
             prev_button,
             next_button);

    return send_html_response(server_ptr, packet_ptr, html);
}

static UINT render_light_meeting_mode_page(NX_HTTP_SERVER *server_ptr,
                                           NX_PACKET *packet_ptr,
                                           const char *message,
                                           int page)
{
    const char *message_to_render = message;
    size_t offset = 0U;
    int user_count;
    int selectable_count = 0;
    int selectable_total = 0;
    int page_start;
    int page_end;
    int rendered_count = 0;
    bool active;
    unsigned int selected_profiles;
    unsigned int allowed_cards;
    unsigned int draft_selected_profiles;
    char prev_button[96];
    char next_button[96];

    if (!net_admin_is_authenticated())
    {
        return render_light_login_page(server_ptr, packet_ptr, "<div class='card warn'>Autentique-se para configurar o modo reuniao.</div>");
    }

    if ((NULL == message_to_render) && ('\0' != g_net_flash_message[0]))
    {
        message_to_render = g_net_flash_message;
        g_net_flash_message[0] = '\0';
    }

    active = storage_meeting_mode_is_active();
    selected_profiles = storage_meeting_mode_selected_profile_count();
    allowed_cards = storage_meeting_mode_allowed_card_count();
    net_meeting_mode_draft_sync_from_active();
    draft_selected_profiles = net_meeting_mode_draft_profile_count();
    user_count = storage_user_count_nowait();
    if (user_count > STORAGE_MAX_USERS)
    {
        user_count = STORAGE_MAX_USERS;
    }

    if (page < 0)
    {
        page = 0;
    }

    for (int i = 0; i < user_count; i++)
    {
        storage_user_profile_t profile;

        if (storage_profile_get_nowait(i, &profile) && (profile.card_count > 0U))
        {
            selectable_total++;
        }
    }

    page_start = page * MEETING_MODE_PAGE_SIZE;
    if (page_start >= selectable_total)
    {
        page = 0;
        page_start = 0;
    }
    page_end = page_start + MEETING_MODE_PAGE_SIZE;
    if (page_end > selectable_total)
    {
        page_end = selectable_total;
    }

    if (page > 0)
    {
        snprintf(prev_button,
                 sizeof(prev_button),
                 "<a class='small secondary' href='/meeting_mode/%d'>Anterior</a>",
                 page - 1);
    }
    else
    {
        prev_button[0] = '\0';
    }

    if (page_end < selectable_total)
    {
        snprintf(next_button,
                 sizeof(next_button),
                 "<a class='small' href='/meeting_mode/%d'>Proxima</a>",
                 page + 1);
    }
    else
    {
        next_button[0] = '\0';
    }

    g_net_metric_buffer[0] = '\0';
    if (!net_appendf(g_net_metric_buffer,
                     sizeof(g_net_metric_buffer),
                     &offset,
                     "<html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'>"
                     "<style>"
                     "body{font-family:Arial,sans-serif;background:#f5f7fb;color:#18212d;margin:0;padding:18px;}"
                     ".wrap{max-width:980px;margin:0 auto;}"
                     ".card{background:#fff;border-radius:16px;padding:18px;margin-bottom:16px;box-shadow:0 8px 26px rgba(0,0,0,.08);}"
                     ".actions{display:flex;flex-wrap:wrap;gap:10px;margin-top:14px;}"
                     ".small,.btn{display:inline-block;text-decoration:none;border:none;border-radius:10px;padding:12px 16px;background:#0b6ef3;color:#fff;cursor:pointer;}"
                     ".secondary{background:#6b7a90;}.danger{background:#d64545;}.muted{color:#607086;font-size:14px;}.warn{color:#b26a00;}.ok{color:#137333;}"
                     ".table-wrap{overflow-x:auto;-webkit-overflow-scrolling:touch;}table{width:100%%;border-collapse:collapse;}th,td{padding:10px;border-bottom:1px solid #e7ebf2;text-align:left;vertical-align:top;}"
                     "@media(max-width:720px){body{padding:12px;}.wrap{max-width:100%%;}.card{padding:14px;border-radius:14px;}.actions{flex-direction:column;align-items:stretch;gap:8px;}.small,.btn{display:block;width:100%%;box-sizing:border-box;text-align:center;}.table-wrap{margin:0 -4px;}table{display:block;overflow-x:auto;-webkit-overflow-scrolling:touch;}th,td{padding:8px;font-size:13px;white-space:nowrap;}}"
                     "</style></head><body><div class='wrap'><div class='card'>"
                     "<h1>Modo reuniao</h1>"
                     "<div class='actions'><a class='small' href='/'>Inicio</a><a class='small' href='/admin_profiles'>Perfis</a><a class='small secondary' href='/logout'>Sair</a></div>"
                     "%s"
                     "<p class='%s'><strong>Modo reuniao %s.</strong></p>"
                     "<p class='muted'>Perfis liberados agora: <strong>%u</strong> | Cartoes liberados: <strong>%u</strong></p>"
                     "<p class='muted'>Selecao preparada para iniciar: <strong>%u</strong> perfil(is).</p>"
                     "<p class='muted'>Mostrando %d a %d de %d perfis com cartao RFID.</p>"
                     "%s"
                     "<div class='table-wrap'><table><tr><th>Nome</th><th>Cargo</th><th>Capitulo</th><th>Cartoes</th><th>Acao</th></tr>",
                     (NULL != message_to_render) ? message_to_render : "",
                     active ? "ok" : "warn",
                     active ? "ativo" : "desativado",
                     selected_profiles,
                     allowed_cards,
                     draft_selected_profiles,
                     (selectable_total > 0) ? (page_start + 1) : 0,
                     page_end,
                     selectable_total,
                     active ? "<div class='actions'><a class='small danger' href='/meeting_mode_stop'>Encerrar modo reuniao</a></div>" : ""))
    {
        return render_light_login_page(server_ptr, packet_ptr, "<div class='card warn'>Nao foi possivel montar a pagina do modo reuniao.</div>");
    }

    for (int i = 0; i < user_count; i++)
    {
        storage_user_profile_t profile;
        char name_html[NAME_MAX_LEN * 6];
        char role_html[STORAGE_ROLE_MAX_LEN * 6];
        char chapter_html[STORAGE_CHAPTER_MAX_LEN * 6];
        const char *toggle_class;
        const char *toggle_label;

        if (!storage_profile_get_nowait(i, &profile) || (0U == profile.card_count))
        {
            continue;
        }

        if ((selectable_count < page_start) || (selectable_count >= page_end))
        {
            selectable_count++;
            continue;
        }

        selectable_count++;
        rendered_count++;
        net_html_escape(profile.name, name_html, sizeof(name_html));
        net_html_escape(('\0' != profile.role[0]) ? profile.role : "-", role_html, sizeof(role_html));
        net_html_escape(('\0' != profile.chapter[0]) ? profile.chapter : "-", chapter_html, sizeof(chapter_html));
        toggle_class = g_net_meeting_draft_selected[i] ? "small secondary" : "small";
        toggle_label = g_net_meeting_draft_selected[i] ? "Remover" : "Selecionar";

        if (!net_appendf(g_net_metric_buffer,
                         sizeof(g_net_metric_buffer),
                         &offset,
                         "<tr><td>%s</td><td>%s</td><td>%s</td><td>%u</td><td><a class='%s' href='/meeting_mode_toggle?index=%d&page=%d'>%s</a></td></tr>",
                         name_html,
                         role_html,
                         chapter_html,
                         profile.card_count,
                         toggle_class,
                         i,
                         page,
                         toggle_label))
        {
            return render_light_login_page(server_ptr, packet_ptr, "<div class='card warn'>Nao foi possivel montar a pagina do modo reuniao.</div>");
        }
    }

    if (0 == selectable_total)
    {
        if (!net_appendf(g_net_metric_buffer,
                         sizeof(g_net_metric_buffer),
                         &offset,
                         "<tr><td colspan='5'>Nenhum perfil com cartao RFID vinculado esta disponivel.</td></tr>"))
        {
            return render_light_login_page(server_ptr, packet_ptr, "<div class='card warn'>Nao foi possivel montar a pagina do modo reuniao.</div>");
        }
    }
    else if (0 == rendered_count)
    {
        if (!net_appendf(g_net_metric_buffer,
                         sizeof(g_net_metric_buffer),
                         &offset,
                         "<tr><td colspan='5'>Nao ha perfis nesta pagina. Volte para a primeira pagina.</td></tr>"))
        {
            return render_light_login_page(server_ptr, packet_ptr, "<div class='card warn'>Nao foi possivel montar a pagina do modo reuniao.</div>");
        }
    }

    if (!net_appendf(g_net_metric_buffer,
                     sizeof(g_net_metric_buffer),
                     &offset,
                     "</table></div>"
                     "<form action='/meeting_mode_start' method='post'>"
                     "<div class='actions'>%s%s<a class='small secondary' href='/meeting_mode_clear?page=%d'>Limpar selecao</a><button class='btn' type='submit'>Iniciar modo reuniao</button><a class='small secondary' href='/admin_profiles'>Voltar</a></div>"
                     "</form>"
                     "</div></div></body></html>",
                     prev_button,
                     next_button,
                     page))
    {
        return render_light_login_page(server_ptr, packet_ptr, "<div class='card warn'>Nao foi possivel montar a pagina do modo reuniao.</div>");
    }

    return send_html_response(server_ptr, packet_ptr, g_net_metric_buffer);
}

static UINT handle_light_meeting_mode_start(NX_HTTP_SERVER *server_ptr, const char *form_data)
{
    int selected_indices[STORAGE_MAX_USERS];
    int selected_count;
    unsigned int selected_profiles = 0U;
    unsigned int allowed_cards = 0U;
    int user_count;

    SSP_PARAMETER_NOT_USED(form_data);

    if (!net_admin_is_authenticated())
    {
        return light_redirect_with_flash(server_ptr, "/login", "<div class='card warn'>Autentique-se para iniciar o modo reuniao.</div>");
    }

    net_meeting_mode_draft_sync_from_active();
    selected_count = 0;
    user_count = storage_user_count();
    if (user_count > STORAGE_MAX_USERS)
    {
        user_count = STORAGE_MAX_USERS;
    }

    for (int i = 0; i < user_count; i++)
    {
        storage_user_profile_t profile;

        if (!g_net_meeting_draft_selected[i])
        {
            continue;
        }

        if (!storage_profile_get(i, &profile) || (0U == profile.card_count))
        {
            continue;
        }

        if (selected_count < STORAGE_MAX_USERS)
        {
            selected_indices[selected_count++] = i;
        }
    }

    if (selected_count <= 0)
    {
        return light_redirect_with_flash(server_ptr, "/meeting_mode", "<div class='card warn'>Selecione ao menos um perfil com cartao para iniciar a reuniao.</div>");
    }

    if (!storage_meeting_mode_start(selected_indices, selected_count, &selected_profiles, &allowed_cards))
    {
        return light_redirect_with_flash(server_ptr, "/meeting_mode", "<div class='card warn'>Nao foi possivel iniciar o modo reuniao. Verifique se os perfis escolhidos possuem cartoes validos.</div>");
    }

    {
        char message[192];

        g_net_meeting_draft_initialized = false;
        snprintf(message,
                 sizeof(message),
                 "<div class='card ok'>Modo reuniao ativado com %u perfil(is) e %u cartao(oes) liberado(s). Apenas os membros selecionados entram por RFID.</div>",
                 selected_profiles,
                 allowed_cards);
        return light_redirect_with_flash(server_ptr, "/meeting_mode", message);
    }
}

static UINT handle_light_meeting_mode_toggle(NX_HTTP_SERVER *server_ptr, const char *query)
{
    storage_user_profile_t profile;
    char location[32];
    int index = query_get_int(query, "index", -1);
    int page = query_get_int(query, "page", 0);

    if (!net_admin_is_authenticated())
    {
        return light_redirect_with_flash(server_ptr, "/login", "<div class='card warn'>Autentique-se para editar a selecao do modo reuniao.</div>");
    }

    if (page < 0)
    {
        page = 0;
    }

    snprintf(location, sizeof(location), "/meeting_mode/%d", page);
    if ((index < 0) || (index >= STORAGE_MAX_USERS) ||
        !storage_profile_get(index, &profile) ||
        (0U == profile.card_count))
    {
        return light_redirect_with_flash(server_ptr, location, "<div class='card warn'>Selecione um perfil valido com cartao RFID.</div>");
    }

    net_meeting_mode_draft_sync_from_active();
    g_net_meeting_draft_selected[index] = !g_net_meeting_draft_selected[index];
    return light_redirect_with_flash(server_ptr, location, "<div class='card ok'>Selecao do modo reuniao atualizada.</div>");
}

static UINT handle_light_meeting_mode_clear(NX_HTTP_SERVER *server_ptr, const char *query)
{
    char location[32];
    int page = query_get_int(query, "page", 0);

    if (!net_admin_is_authenticated())
    {
        return light_redirect_with_flash(server_ptr, "/login", "<div class='card warn'>Autentique-se para limpar a selecao do modo reuniao.</div>");
    }

    if (page < 0)
    {
        page = 0;
    }

    net_meeting_mode_draft_reset();
    snprintf(location, sizeof(location), "/meeting_mode/%d", page);
    return light_redirect_with_flash(server_ptr, location, "<div class='card ok'>Selecao do modo reuniao limpa.</div>");
}

static UINT handle_light_meeting_mode_stop(NX_HTTP_SERVER *server_ptr)
{
    if (!net_admin_is_authenticated())
    {
        return light_redirect_with_flash(server_ptr, "/login", "<div class='card warn'>Autentique-se para encerrar o modo reuniao.</div>");
    }

    storage_meeting_mode_stop();
    g_net_meeting_draft_initialized = false;
    return light_redirect_with_flash(server_ptr, "/meeting_mode", "<div class='card ok'>Modo reuniao encerrado. O acesso voltou ao comportamento normal.</div>");
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

static bool net_pin_is_valid_4_digits(const char *pin)
{
    size_t len = 0U;

    if ((NULL == pin) || ('\0' == pin[0]))
    {
        return false;
    }

    while ('\0' != pin[len])
    {
        if (!isdigit((int) (unsigned char) pin[len]))
        {
            return false;
        }
        len++;
    }

    return (len == STORAGE_ADMIN_PIN_LEN);
}

static bool net_admin_pin_valid_nowait(const char *pin, bool *out_pin_configured)
{
    int user_count;
    bool pin_configured = false;
    bool pin_valid = false;

    if (NULL != out_pin_configured)
    {
        *out_pin_configured = false;
    }
    if ((NULL == pin) || ('\0' == pin[0]))
    {
        return false;
    }

    user_count = storage_user_count_nowait();
    if (user_count > STORAGE_MAX_USERS)
    {
        user_count = STORAGE_MAX_USERS;
    }

    for (int i = 0; i < user_count; i++)
    {
        storage_user_profile_t profile;

        if (!storage_profile_get_nowait(i, &profile) || !profile.is_admin || ('\0' == profile.admin_pin[0]))
        {
            continue;
        }

        pin_configured = true;
        if (0 == strcmp(profile.admin_pin, pin))
        {
            pin_valid = true;
            break;
        }
    }

    if (NULL != out_pin_configured)
    {
        *out_pin_configured = pin_configured;
    }
    return pin_valid;
}

static UINT handle_light_login(NX_HTTP_SERVER *server_ptr, const char *form_data)
{
    char pin[16];
    ULONG now = tx_time_get();
    bool pin_configured = false;

    if (!query_get_value(form_data, "pin", pin, sizeof(pin)))
    {
        return light_redirect_with_flash(server_ptr, "/login", "<div class='card warn'>Digite o PIN antes de entrar.</div>");
    }

    if (0 == strcmp(pin, WEB_ADMIN_PIN))
    {
        net_admin_begin_session();
        g_net_admin_login_failures = 0U;
        g_net_admin_login_block_until = 0U;
        return light_redirect_with_flash(server_ptr, "/", "<div class='card ok'>Sessao admin iniciada.</div>");
    }

    if ((0U != g_net_admin_login_block_until) &&
        ((LONG) (now - g_net_admin_login_block_until) < 0))
    {
        return light_redirect_with_flash(server_ptr,
                                         "/login",
                                         "<div class='card warn'>Muitas tentativas. Aguarde um minuto antes de tentar novamente.</div>");
    }

    if (net_admin_pin_valid_nowait(pin, &pin_configured))
    {
        net_admin_begin_session();
        g_net_admin_login_failures = 0U;
        g_net_admin_login_block_until = 0U;
        return light_redirect_with_flash(server_ptr, "/", "<div class='card ok'>Sessao admin iniciada.</div>");
    }

    if (!storage_users_loaded_nowait())
    {
        return light_redirect_with_flash(server_ptr,
                                         "/login",
                                         "<div class='card warn'>Perfis ainda carregando da QSPI. Aguarde alguns segundos e tente novamente.</div>");
    }

    g_net_admin_login_failures++;
    if (g_net_admin_login_failures >= WEB_ADMIN_LOGIN_MAX_FAILURES)
    {
        g_net_admin_login_failures = 0U;
        g_net_admin_login_block_until = now + WEB_ADMIN_LOGIN_BLOCK_TICKS;
    }

    return light_redirect_with_flash(server_ptr, "/login", "<div class='card warn'>PIN invalido.</div>");
}

static UINT handle_light_add_user(NX_HTTP_SERVER *server_ptr, const char *query)
{
    storage_user_profile_t profile;
    storage_user_profile_t existing_profile;
    char primary_uid[UID_MAX_LEN];
    char location[64];
    int edit_index = -1;
    bool persist_requested = false;
    bool persist_ok = false;
    bool has_existing_profile = false;

    if (!net_admin_is_authenticated())
    {
        return light_redirect_with_flash(server_ptr, "/login", "<div class='card warn'>Autentique-se para salvar perfis.</div>");
    }

    if (!profile_from_query(query, &profile, &edit_index))
    {
        if (edit_index >= 0)
        {
            snprintf(location, sizeof(location), "/profile_form/%d", edit_index);
        }
        else
        {
            strncpy(location, "/profile_form", sizeof(location) - 1U);
            location[sizeof(location) - 1U] = '\0';
        }

        return light_redirect_with_flash(server_ptr, location, "<div class='card warn'>Preencha ao menos o nome do perfil.</div>");
    }

    has_existing_profile = (edit_index >= 0) && storage_profile_get(edit_index, &existing_profile);

    if (profile.is_admin)
    {
        if ('\0' == profile.admin_pin[0])
        {
            if (has_existing_profile && existing_profile.is_admin && ('\0' != existing_profile.admin_pin[0]))
            {
                strncpy(profile.admin_pin, existing_profile.admin_pin, sizeof(profile.admin_pin) - 1U);
                profile.admin_pin[sizeof(profile.admin_pin) - 1U] = '\0';
            }
            else
            {
                if (edit_index >= 0)
                {
                    snprintf(location, sizeof(location), "/profile_form/%d", edit_index);
                }
                else
                {
                    strncpy(location, "/profile_form", sizeof(location) - 1U);
                    location[sizeof(location) - 1U] = '\0';
                }
                return light_redirect_with_flash(server_ptr, location, "<div class='card warn'>Perfis administradores precisam de um PIN de 4 digitos.</div>");
            }
        }

        if (!net_pin_is_valid_4_digits(profile.admin_pin))
        {
            if (edit_index >= 0)
            {
                snprintf(location, sizeof(location), "/profile_form/%d", edit_index);
            }
            else
            {
                strncpy(location, "/profile_form", sizeof(location) - 1U);
                location[sizeof(location) - 1U] = '\0';
            }
            return light_redirect_with_flash(server_ptr, location, "<div class='card warn'>O PIN do administrador deve ter exatamente 4 digitos numericos.</div>");
        }
    }
    else
    {
        profile.admin_pin[0] = '\0';
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
                                                    ? "<div class='card warn'>Perfil salvo, mas a gravacao automatica na memória interna ainda não foi confirmada.</div>"
                                                    : "<div class='card warn'>Perfil salvo, mas não foi possivel iniciar a gravacao automatica na memória interna.</div>"));
    }

    if (edit_index >= 0)
    {
        snprintf(location, sizeof(location), "/profile_form/%d", edit_index);
    }
    else
    {
        strncpy(location, "/profile_form", sizeof(location) - 1U);
        location[sizeof(location) - 1U] = '\0';
    }

    return light_redirect_with_flash(server_ptr, location, "<div class='card warn'>Nao foi possivel salvar o perfil. Verifique cartões duplicados ou campos obrigatórios.</div>");
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
        return light_redirect_with_flash(server_ptr, "/admin_profiles", "<div class='card warn'>Indice invalido para remoção.</div>");
    }

    if (storage_profile_remove(index))
    {
        bool persist_ok = false;

        app_post_event(EVENT_USER_REMOVED, NULL);
        persist_requested = storage_persist_now();
        if (persist_requested)
        {
            persist_ok = storage_persist_wait(5U * TX_TIMER_TICKS_PER_SECOND);
        }
        return light_redirect_with_flash(server_ptr,
                                         "/admin_profiles",
                                         persist_ok
                                             ? "<div class='card ok'>Perfil removido e gravado automaticamente na QSPI.</div>"
                                             : (persist_requested
                                                    ? "<div class='card warn'>Perfil removido, mas a gravacao automatica na QSPI ainda nao foi confirmada.</div>"
                                                    : "<div class='card warn'>Perfil removido, mas nao foi possivel iniciar a gravacao automatica na QSPI.</div>"));
    }

    return light_redirect_with_flash(server_ptr, "/admin_profiles", "<div class='card warn'>Perfil não encontrado.</div>");
}

static UINT handle_light_save_users(NX_HTTP_SERVER *server_ptr)
{
    bool persist_requested;
    bool persist_ok = false;

    if (!net_admin_is_authenticated())
    {
        return light_redirect_with_flash(server_ptr, "/login", "<div class='card warn'>Autentique-se para persistir os perfis.</div>");
    }

    persist_requested = storage_persist_now();
    if (persist_requested)
    {
        persist_ok = storage_persist_wait(8U * TX_TIMER_TICKS_PER_SECOND);
    }

    if (persist_ok)
    {
        return light_redirect_with_flash(server_ptr, "/admin_profiles", "<div class='card ok'>Cadastros gravados na QSPI.</div>");
    }

    return light_redirect_with_flash(server_ptr,
                                     "/admin_profiles",
                                     persist_requested
                                         ? "<div class='card warn'>A gravacao na QSPI foi solicitada, mas nao foi confirmada no tempo esperado.</div>"
                                         : "<div class='card warn'>Nao foi possivel iniciar a gravacao dos cadastros na QSPI.</div>");
}

static UINT handle_light_save_access_log(NX_HTTP_SERVER *server_ptr)
{
    if (!net_admin_is_authenticated())
    {
        return light_redirect_with_flash(server_ptr, "/login", "<div class='card warn'>Autentique-se para persistir o log de acesso.</div>");
    }

    if (storage_access_log_persist_now())
    {
        return light_redirect_with_flash(server_ptr, "/access_log", "<div class='card ok'>Persistencia do log de acesso solicitada. Aguarde alguns segundos e recarregue a página.</div>");
    }

    return light_redirect_with_flash(server_ptr, "/access_log", "<div class='card warn'>Não foi possivel salvar o log de acesso agora.</div>");
}

static UINT handle_light_import_profiles(NX_HTTP_SERVER *server_ptr, const char *form_data)
{
    if (!net_admin_is_authenticated())
    {
        return light_redirect_with_flash(server_ptr, "/login", "<div class='card warn'>Autentique-se para importar perfis.</div>");
    }

    if (!net_queue_import_profiles(form_data))
    {
        return light_redirect_with_flash(server_ptr, "/import", "<div class='card warn'>Cole o JSON antes de importar.</div>");
    }

    return light_redirect_with_flash(server_ptr, "/admin_profiles", "<div class='card ok'>Importação agendada. Recarregue a página em alguns instantes para ver o resultado.</div>");
}

static UINT handle_api_door_open(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr)
{
    static const char ok_json[] = "{\"ok\":true,\"message\":\"Door open command sent.\"}";
    static const char unauthorized_json[] = "{\"ok\":false,\"error\":\"unauthorized\"}";
    static const char busy_json[] = "{\"ok\":false,\"error\":\"cooldown\"}";
    ULONG now = tx_time_get();

    if (!net_api_request_is_authorized(packet_ptr))
    {
        return send_buffer_status_response(server_ptr,
                                           packet_ptr,
                                           NX_HTTP_STATUS_UNAUTHORIZED,
                                           unauthorized_json,
                                           sizeof(unauthorized_json) - 1U,
                                            "application/json");
    }

    if ((0U != g_net_api_door_next_allowed_tick) &&
        ((LONG) (now - g_net_api_door_next_allowed_tick) < 0))
    {
        return send_buffer_status_response(server_ptr,
                                           packet_ptr,
                                           NX_HTTP_STATUS_CONFLICT,
                                           busy_json,
                                           sizeof(busy_json) - 1U,
                                           "application/json");
    }

    g_net_api_door_next_allowed_tick = now + NET_API_DOOR_COOLDOWN_TICKS;
    app_post_event(EVENT_DOOR_OPEN, "API");
    return send_buffer_status_response(server_ptr,
                                       packet_ptr,
                                       NX_HTTP_STATUS_OK,
                                       ok_json,
                                       sizeof(ok_json) - 1U,
                                       "application/json");
}

static UINT net_send_json_error(NX_HTTP_SERVER *server_ptr,
                                NX_PACKET *packet_ptr,
                                const char *status_code,
                                const char *error)
{
    if (NULL == error)
    {
        error = "error";
    }

    snprintf(g_net_api_json,
             sizeof(g_net_api_json),
             "{\"ok\":false,\"error\":\"%s\"}",
             error);

    return send_buffer_status_response(server_ptr,
                                       packet_ptr,
                                       status_code,
                                       g_net_api_json,
                                       strlen(g_net_api_json),
                                       "application/json");
}

static bool net_meeting_cancel_parse_id(const char *body, const char *query, ULONG *out_id, bool *out_has_id)
{
    const char *cursor = body;
    ULONG id = 0U;

    if ((NULL == out_id) || (NULL == out_has_id))
    {
        return false;
    }

    *out_id = 0U;
    *out_has_id = false;

    if ((NULL != query) && net_form_get_ulong_value(query, "id", &id))
    {
        *out_id = id;
        *out_has_id = true;
        return (0U != id);
    }

    if (NULL == cursor)
    {
        return true;
    }

    while (('\0' != *cursor) && isspace((unsigned char) *cursor))
    {
        cursor++;
    }

    if ('\0' == *cursor)
    {
        return true;
    }

    if ('{' == *cursor)
    {
        if (!net_json_get_ulong_value(cursor, "id", &id))
        {
            return false;
        }
    }
    else if (!net_form_get_ulong_value(cursor, "id", &id))
    {
        return false;
    }

    *out_id = id;
    *out_has_id = true;
    return (0U != id);
}

static UINT handle_api_meeting_schedule(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, const char *body)
{
    int profiles[STORAGE_MAX_USERS];
    int profile_count = 0;
    ULONG start_utc = 0U;
    ULONG delay_seconds = 0U;
    ULONG schedule_id = 0U;
    uint8_t recurrence = STORAGE_MEETING_RECURRENCE_NONE;
    uint8_t weekdays_mask = 0U;
    int pending_count = 0;
    const char *error = "bad_request";
    bool time_synced;
    ULONG now_utc = 0U;
    bool saved = false;

    if (!net_api_request_is_authorized(packet_ptr))
    {
        return net_send_json_error(server_ptr, packet_ptr, NX_HTTP_STATUS_UNAUTHORIZED, "unauthorized");
    }

    if (!net_meeting_schedule_parse_request(body,
                                            &start_utc,
                                            &delay_seconds,
                                            &recurrence,
                                            &weekdays_mask,
                                            profiles,
                                            &profile_count,
                                            &error))
    {
        return net_send_json_error(server_ptr, packet_ptr, NX_HTTP_STATUS_BAD_REQUEST, error);
    }

    if (!net_meeting_schedule_ensure_loaded())
    {
        return net_send_json_error(server_ptr, packet_ptr, NX_HTTP_STATUS_CONFLICT, "schedule_storage_unavailable");
    }

    net_action_lock();
    if (g_net_meeting_schedule_count >= STORAGE_MEETING_SCHEDULE_MAX_ITEMS)
    {
        net_action_unlock();
        return net_send_json_error(server_ptr, packet_ptr, NX_HTTP_STATUS_CONFLICT, "schedule_full");
    }

    schedule_id = g_net_meeting_schedule_next_id++;
    if (0U == g_net_meeting_schedule_next_id)
    {
        g_net_meeting_schedule_next_id = 1U;
    }

    memset(&g_net_meeting_schedules[g_net_meeting_schedule_count], 0, sizeof(g_net_meeting_schedules[g_net_meeting_schedule_count]));
    g_net_meeting_schedules[g_net_meeting_schedule_count].id = schedule_id;
    g_net_meeting_schedules[g_net_meeting_schedule_count].start_unix = start_utc;
    g_net_meeting_schedules[g_net_meeting_schedule_count].profile_count = (unsigned int) profile_count;
    g_net_meeting_schedules[g_net_meeting_schedule_count].recurrence = recurrence;
    g_net_meeting_schedules[g_net_meeting_schedule_count].weekdays_mask = weekdays_mask;
    for (int i = 0; i < profile_count; i++)
    {
        g_net_meeting_schedules[g_net_meeting_schedule_count].profile_indices[i] = (uint8_t) profiles[i];
    }
    g_net_meeting_schedule_count++;
    g_net_meeting_schedule_last_id = schedule_id;
    g_net_meeting_schedule_last_start_utc = start_utc;
    g_net_meeting_schedule_last_selected = 0U;
    g_net_meeting_schedule_last_allowed = 0U;
    net_meeting_schedule_set_status_locked("scheduled");
    saved = net_meeting_schedule_save_locked();
    pending_count = g_net_meeting_schedule_count;
    if (!saved)
    {
        if (g_net_meeting_schedule_count > 0)
        {
            g_net_meeting_schedule_count--;
            memset(&g_net_meeting_schedules[g_net_meeting_schedule_count], 0, sizeof(g_net_meeting_schedules[g_net_meeting_schedule_count]));
        }
        net_meeting_schedule_set_status_locked("save_failed");
    }
    net_action_unlock();

    if (!saved)
    {
        return net_send_json_error(server_ptr, packet_ptr, NX_HTTP_STATUS_INTERNAL_ERROR, "schedule_save_failed");
    }

    time_synced = app_time_get_utc(&now_utc);
    snprintf(g_net_api_json,
             sizeof(g_net_api_json),
             "{\"ok\":true,\"id\":%lu,\"pending_count\":%d,\"active\":%s,"
             "\"time_synced\":%s,\"now_unix\":%lu,\"start_unix\":%lu,"
             "\"delay_seconds\":%lu,\"profile_count\":%d,"
             "\"recurrence\":\"%s\",\"weekdays_mask\":%u}",
             (unsigned long) schedule_id,
             pending_count,
             storage_meeting_mode_is_active() ? "true" : "false",
             time_synced ? "true" : "false",
             (unsigned long) (time_synced ? now_utc : 0U),
             (unsigned long) start_utc,
             (unsigned long) delay_seconds,
             profile_count,
             net_meeting_recurrence_text(recurrence),
             (unsigned int) weekdays_mask);

    return send_buffer_status_response(server_ptr,
                                       packet_ptr,
                                       NX_HTTP_STATUS_OK,
                                       g_net_api_json,
                                       strlen(g_net_api_json),
                                       "application/json");
}

static UINT handle_api_meeting_cancel(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, const char *body, const char *query)
{
    storage_meeting_schedule_t backup_schedules[STORAGE_MEETING_SCHEDULE_MAX_ITEMS];
    int backup_count = 0;
    ULONG id = 0U;
    bool has_id = false;
    int canceled_count = 0;
    int pending_count = 0;
    bool saved = true;

    if (!net_api_request_is_authorized(packet_ptr))
    {
        return net_send_json_error(server_ptr, packet_ptr, NX_HTTP_STATUS_UNAUTHORIZED, "unauthorized");
    }

    if (!net_meeting_cancel_parse_id(body, query, &id, &has_id))
    {
        return net_send_json_error(server_ptr, packet_ptr, NX_HTTP_STATUS_BAD_REQUEST, "invalid_id");
    }

    if (!net_meeting_schedule_ensure_loaded())
    {
        return net_send_json_error(server_ptr, packet_ptr, NX_HTTP_STATUS_CONFLICT, "schedule_storage_unavailable");
    }

    net_action_lock();
    backup_count = g_net_meeting_schedule_count;
    for (int i = 0; i < backup_count; i++)
    {
        backup_schedules[i] = g_net_meeting_schedules[i];
    }

    if (has_id)
    {
        for (int i = 0; i < g_net_meeting_schedule_count; i++)
        {
            if (g_net_meeting_schedules[i].id == id)
            {
                for (int move_index = i; move_index < (g_net_meeting_schedule_count - 1); move_index++)
                {
                    g_net_meeting_schedules[move_index] = g_net_meeting_schedules[move_index + 1];
                }
                g_net_meeting_schedule_count--;
                canceled_count = 1;
                break;
            }
        }
    }
    else
    {
        canceled_count = g_net_meeting_schedule_count;
        g_net_meeting_schedule_count = 0;
    }

    if (canceled_count > 0)
    {
        saved = net_meeting_schedule_save_locked();
    }
    if (!saved)
    {
        g_net_meeting_schedule_count = backup_count;
        for (int i = 0; i < backup_count; i++)
        {
            g_net_meeting_schedules[i] = backup_schedules[i];
        }
        net_meeting_schedule_set_status_locked("save_failed");
    }
    else
    {
        net_meeting_schedule_set_status_locked((canceled_count > 0) ? "canceled" : "idle");
    }
    pending_count = g_net_meeting_schedule_count;
    net_action_unlock();

    if (!saved)
    {
        return net_send_json_error(server_ptr, packet_ptr, NX_HTTP_STATUS_INTERNAL_ERROR, "schedule_save_failed");
    }

    snprintf(g_net_api_json,
             sizeof(g_net_api_json),
             "{\"ok\":true,\"canceled_count\":%d,\"pending_count\":%d,\"active\":%s}",
             canceled_count,
             pending_count,
             storage_meeting_mode_is_active() ? "true" : "false");

    return send_buffer_status_response(server_ptr,
                                       packet_ptr,
                                       NX_HTTP_STATUS_OK,
                                       g_net_api_json,
                                       strlen(g_net_api_json),
                                       "application/json");
}

static UINT handle_api_meeting_status(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr)
{
    int pending_count;
    ULONG last_id;
    ULONG last_start_utc;
    unsigned int last_selected;
    unsigned int last_allowed;
    char last_status[NET_MEETING_SCHEDULE_STATUS_LEN];
    bool time_synced;
    ULONG now_utc = 0U;
    bool active;
    unsigned int active_selected;
    unsigned int active_allowed;

    if (!net_api_request_is_authorized(packet_ptr))
    {
        return net_send_json_error(server_ptr, packet_ptr, NX_HTTP_STATUS_UNAUTHORIZED, "unauthorized");
    }

    if (!net_meeting_schedule_ensure_loaded())
    {
        return net_send_json_error(server_ptr, packet_ptr, NX_HTTP_STATUS_CONFLICT, "schedule_storage_unavailable");
    }

    active = storage_meeting_mode_is_active();
    active_selected = storage_meeting_mode_selected_profile_count();
    active_allowed = storage_meeting_mode_allowed_card_count();
    time_synced = app_time_get_utc(&now_utc);

    net_action_lock();
    pending_count = g_net_meeting_schedule_count;
    last_id = g_net_meeting_schedule_last_id;
    last_start_utc = g_net_meeting_schedule_last_start_utc;
    last_selected = g_net_meeting_schedule_last_selected;
    last_allowed = g_net_meeting_schedule_last_allowed;
    strncpy(last_status, g_net_meeting_schedule_last_status, sizeof(last_status) - 1U);
    last_status[sizeof(last_status) - 1U] = '\0';

    {
        size_t offset = 0U;

        (void) net_appendf(g_net_api_json,
                           sizeof(g_net_api_json),
                           &offset,
                           "{\"ok\":true,\"active\":%s,\"pending_count\":%d,"
                           "\"time_synced\":%s,\"now_unix\":%lu,"
                           "\"active_selected_profiles\":%u,\"active_allowed_cards\":%u,"
                           "\"last_id\":%lu,\"last_start_unix\":%lu,"
                           "\"last_selected_profiles\":%u,\"last_allowed_cards\":%u,"
                           "\"last_status\":\"%s\",\"schedules\":[",
                           active ? "true" : "false",
                           pending_count,
                           time_synced ? "true" : "false",
                           (unsigned long) (time_synced ? now_utc : 0U),
                           active_selected,
                           active_allowed,
                           (unsigned long) last_id,
                           (unsigned long) last_start_utc,
                           last_selected,
                           last_allowed,
                           last_status);

        for (int i = 0; i < g_net_meeting_schedule_count; i++)
        {
            (void) net_appendf(g_net_api_json,
                               sizeof(g_net_api_json),
                               &offset,
                               "%s{\"id\":%lu,\"start_unix\":%lu,\"profile_count\":%u,\"recurrence\":\"%s\",\"weekdays_mask\":%u}",
                               (i > 0) ? "," : "",
                               (unsigned long) g_net_meeting_schedules[i].id,
                               (unsigned long) g_net_meeting_schedules[i].start_unix,
                               g_net_meeting_schedules[i].profile_count,
                               net_meeting_recurrence_text(g_net_meeting_schedules[i].recurrence),
                               (unsigned int) g_net_meeting_schedules[i].weekdays_mask);
        }

        (void) net_appendf(g_net_api_json, sizeof(g_net_api_json), &offset, "]}");
    }
    net_action_unlock();

    return send_buffer_status_response(server_ptr,
                                       packet_ptr,
                                       NX_HTTP_STATUS_OK,
                                       g_net_api_json,
                                       strlen(g_net_api_json),
                                       "application/json");
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
    if (query[0] == '\0')
    {
        extract_query_from_packet(packet_ptr, query, sizeof(query));
    }
    body[0] = '\0';
    if (NX_HTTP_SERVER_POST_REQUEST == request_type)
    {
        if (!extract_body_from_packet(server_ptr, packet_ptr, body, sizeof(body)))
        {
            return send_plain_status_response(server_ptr, packet_ptr, NX_HTTP_STATUS_ENTITY_TOO_LARGE, "ERR_BODY_TOO_LARGE");
        }
    }

    if (NX_HTTP_SERVER_GET_REQUEST == request_type)
    {
        g_net_debug_http_stage = 3U;
        if (0 == strcmp(path, "/health"))
        {
            return send_plain_response(server_ptr, packet_ptr, "OK");
        }
        if (0 == strcmp(path, "/api/meeting/status"))
        {
            return handle_api_meeting_status(server_ptr, packet_ptr);
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
            if (!net_admin_is_authenticated())
            {
                return light_redirect_with_flash(server_ptr, "/", "<div class='card warn'>Sessao admin nao pertence a este cliente.</div>");
            }
            net_admin_end_session();
            return light_redirect_with_flash(server_ptr, "/", "<div class='card ok'>Sessão de administração encerrada.</div>");
        }
        if (0 == strcmp(path, "/profiles"))
        {
            return light_redirect_with_flash(server_ptr, "/admin_profiles", "<div class='card ok'>Rota de perfis atualizada.</div>");
        }
        if (0 == strcmp(path, "/admin_profiles"))
        {
            return render_light_profiles_page(server_ptr, packet_ptr, NULL, query_get_int(query, "page", 0));
        }
        if (0 == strncmp(path, "/admin_profiles/", strlen("/admin_profiles/")))
        {
            return render_light_profiles_page(server_ptr,
                                              packet_ptr,
                                              NULL,
                                              net_path_get_index_after_prefix(path, "/admin_profiles/", 0));
        }
        if (0 == strcmp(path, "/profile_form"))
        {
            return render_light_profile_form_page(server_ptr, packet_ptr, NULL, query_get_int(query, "edit", -1));
        }
        if (0 == strncmp(path, "/profile_form/", strlen("/profile_form/")))
        {
            return render_light_profile_form_page(server_ptr,
                                                  packet_ptr,
                                                  NULL,
                                                  net_path_get_index_after_prefix(path, "/profile_form/", -1));
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
            return render_light_access_log_page(server_ptr, packet_ptr, NULL, query_get_int(query, "page", 0));
        }
        if (0 == strncmp(path, "/access_log/", strlen("/access_log/")))
        {
            return render_light_access_log_page(server_ptr,
                                                packet_ptr,
                                                NULL,
                                                net_path_get_index_after_prefix(path, "/access_log/", 0));
        }
        if (0 == strcmp(path, "/meeting_mode"))
        {
            return render_light_meeting_mode_page(server_ptr, packet_ptr, NULL, query_get_int(query, "page", 0));
        }
        if (0 == strncmp(path, "/meeting_mode/", strlen("/meeting_mode/")))
        {
            return render_light_meeting_mode_page(server_ptr,
                                                  packet_ptr,
                                                  NULL,
                                                  net_path_get_index_after_prefix(path, "/meeting_mode/", 0));
        }
        if (0 == strcmp(path, "/meeting_mode_toggle"))
        {
            return handle_light_meeting_mode_toggle(server_ptr, query);
        }
        if (0 == strcmp(path, "/meeting_mode_clear"))
        {
            return handle_light_meeting_mode_clear(server_ptr, query);
        }
        if (0 == strcmp(path, "/meeting_mode_stop"))
        {
            return handle_light_meeting_mode_stop(server_ptr);
        }
        if (0 == strcmp(path, "/door"))
        {
            return render_light_door_page(server_ptr, packet_ptr, NULL);
        }
        if (0 == strcmp(path, "/metrics"))
        {
            return render_light_metrics_page(server_ptr, packet_ptr, NULL, query_get_int(query, "page", 0));
        }
        if (0 == strncmp(path, "/metrics/", strlen("/metrics/")))
        {
            return render_light_metrics_page(server_ptr,
                                             packet_ptr,
                                             NULL,
                                             net_path_get_index_after_prefix(path, "/metrics/", 0));
        }
        if (0 == strcmp(path, "/metrics_download"))
        {
            return handle_light_metrics_download(server_ptr, packet_ptr);
        }
        if (0 == strcmp(path, "/storage_export"))
        {
            return render_light_storage_export_page(server_ptr, packet_ptr, NULL, query_get_int(query, "page", 0));
        }
        if (0 == strncmp(path, "/storage_export/", strlen("/storage_export/")))
        {
            return render_light_storage_export_page(server_ptr,
                                                    packet_ptr,
                                                    NULL,
                                                    net_path_get_index_after_prefix(path, "/storage_export/", 0));
        }
        if (0 == strcmp(path, "/storage_users_download"))
        {
            return handle_light_storage_users_download(server_ptr, packet_ptr);
        }
        if (0 == strcmp(path, "/storage_photo_download"))
        {
            return handle_light_storage_photo_download(server_ptr, packet_ptr, query);
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
        if (0 == strcmp(path, "/save_access_log"))
        {
            return handle_light_save_access_log(server_ptr);
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

            if (!net_admin_is_authenticated())
            {
                return send_plain_status_response(server_ptr, packet_ptr, NX_HTTP_STATUS_UNAUTHORIZED, "ERR_AUTH");
            }

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
        if (0 == strcmp(path, "/api/door/open"))
        {
            return handle_api_door_open(server_ptr, packet_ptr);
        }
        if (0 == strcmp(path, "/api/meeting/schedule"))
        {
            return handle_api_meeting_schedule(server_ptr, packet_ptr, body);
        }
        if (0 == strcmp(path, "/api/meeting/cancel"))
        {
            return handle_api_meeting_cancel(server_ptr, packet_ptr, body, query);
        }
        if (0 == strcmp(path, "/login"))
        {
            return handle_light_login(server_ptr, ('\0' != body[0]) ? body : query);
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
        if (0 == strcmp(path, "/meeting_mode_start"))
        {
            return handle_light_meeting_mode_start(server_ptr, body);
        }
    }

    g_net_debug_http_stage = 5U;
    return send_plain_response(server_ptr, packet_ptr, "Not Found");
}

UINT request_notify(NX_HTTP_SERVER *server_ptr, UINT request_type, CHAR *resource, NX_PACKET *packet_ptr)
{
    UINT status;
    ULONG start_tick = tx_time_get();
    char path_copy[sizeof(g_net_debug_last_path)];
    const char *metric_case;
    g_net_debug_http_inflight = 1U;
    net_http_lock();
    (void) net_request_source_ip(packet_ptr, &g_net_current_request_ip);
    status = request_notify_impl(server_ptr, request_type, resource, packet_ptr);
    g_net_current_request_ip = 0U;
    net_http_unlock();
    strncpy(path_copy, (const char *) g_net_debug_last_path, sizeof(path_copy) - 1U);
    path_copy[sizeof(path_copy) - 1U] = '\0';
    metric_case = net_metric_http_case_for_path(path_copy);
    if (NULL != metric_case)
    {
        app_metric_add("HTTP latency",
                       metric_case,
                       tx_time_get() - start_tick,
                       ((NX_SUCCESS == status) || (NX_HTTP_CALLBACK_COMPLETED == status)));
    }
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
    ULONG ip_address = 0U;
    ULONG network_mask = 0U;

    network_stack_init_once();
    storage_init();
    g_net_debug_state = 3U;

    if (NX_SUCCESS == nx_ip_status_check(&g_ip0, NX_IP_LINK_ENABLED, &actual_status, TX_WAIT_FOREVER))
    {
        g_net_debug_link_status = actual_status;
        g_net_debug_state = 4U;
        status = g_net_dhcp_created ? nx_dhcp_start(&g_net_dhcp_client) : NX_NOT_ENABLED;
        g_net_debug_ip_status = status;
        if ((NX_SUCCESS == status) || (NX_DHCP_ALREADY_STARTED == status))
        {
            g_net_debug_state = 5U;
            if (net_wait_for_dhcp_address(&ip_address, &network_mask))
            {
                g_net_debug_ip_address = ip_address;
                g_net_debug_network_mask = network_mask;

                app_state_lock();
                g_app_state.net_ready = true;
                app_state_unlock();

                g_net_debug_http_status = nx_http_server_start(&g_http_server0);
                g_net_debug_state = (NX_SUCCESS == g_net_debug_http_status) ? 6U : 0xEEU;
                if (NX_SUCCESS == g_net_debug_http_status)
                {
                    (void) net_sync_time_with_ntp();
                }
            }
            else
            {
                g_net_debug_state = 0xEDU;
            }
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
        net_service_ntp();
        net_process_meeting_schedule();

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
