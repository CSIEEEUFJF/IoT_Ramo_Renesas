#include "main.h"
#include "net.h"
#include "rfid.h"
#include "ui.h"
#include "gpio.h"
#include "storage.h"
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>

/* Configurações de rede */
const char * RELAY_HOST    = "192.168.1.100";
const ULONG    RELAY_IP_ADDR = IP_ADDRESS(192, 168, 1, 100);
const uint16_t RELAY_PORT    = 3000;
const char * DEVICE_ID     = "s7g2_master";
const char * API_KEY       = "SuperStrongKey123!";

#define NETWORK_ISOLATION_MODE 0
#define NETWORK_HTTP_ONLY_MODE 1

#define STACK_SIZE_RFID      2048
#define STACK_SIZE_UI        4096
#define STACK_SIZE_NET       8192
#define STACK_SIZE_TUNNEL    2048
#define STACK_SIZE_GPIO      1024
#define STACK_SIZE_STATUS    1024

#define PRIO_RFID            3
#define PRIO_UI              2
#define PRIO_NET             6
#define PRIO_TUNNEL          5
#define PRIO_GPIO            3
#define PRIO_STATUS          6

static TX_THREAD thread_rfid;
static TX_THREAD thread_ui;
static TX_THREAD thread_net;
#if !NETWORK_HTTP_ONLY_MODE
static TX_THREAD thread_tunnel;
static TX_THREAD thread_status;
#endif
static TX_THREAD thread_gpio;

static uint8_t stack_rfid   [STACK_SIZE_RFID]   BSP_ALIGN_VARIABLE(8);
static uint8_t stack_ui     [STACK_SIZE_UI]     BSP_ALIGN_VARIABLE(8);
static uint8_t stack_net    [STACK_SIZE_NET]    BSP_ALIGN_VARIABLE(8);
#if !NETWORK_HTTP_ONLY_MODE
static uint8_t stack_tunnel [STACK_SIZE_TUNNEL] BSP_ALIGN_VARIABLE(8);
static uint8_t stack_status [STACK_SIZE_STATUS] BSP_ALIGN_VARIABLE(8);
#endif
static uint8_t stack_gpio   [STACK_SIZE_GPIO]   BSP_ALIGN_VARIABLE(8);

static TX_MUTEX  g_state_mutex;
static app_access_log_entry_t g_access_log[ACCESS_LOG_SIZE];
static app_metric_entry_t g_metric_log[APP_METRIC_LOG_SIZE];
static int g_access_log_next = 0;
static int g_access_log_count = 0;
static int g_metric_log_next = 0;
static int g_metric_log_count = 0;
static ULONG g_app_time_base_utc = 0U;
static ULONG g_app_time_base_tick = 0U;
static bool g_app_time_synced = false;
static bool g_app_metric_door_pending = false;
static bool g_app_metric_ui_pending = false;
static ULONG g_app_metric_door_start_tick = 0U;
static ULONG g_app_metric_ui_start_tick = 0U;
static app_metric_case_t g_app_metric_door_case = APP_METRIC_CASE_UNKNOWN;
static app_metric_case_t g_app_metric_ui_case = APP_METRIC_CASE_UNKNOWN;
volatile app_state_t g_app_state = {
    .door_open     = false,
    .light_on      = false,
    .last_uid      = {0},
    .last_user     = {0},
    .uid_pending   = false,
    .net_ready     = false,
    .enroll_mode   = false,
    .ui_mode       = APP_UI_MODE_IDLE,
};

TX_QUEUE g_event_queue;
#define EVENT_QUEUE_MESSAGE_WORDS ((sizeof(app_event_t) + sizeof(ULONG) - 1U) / sizeof(ULONG))
static ULONG event_queue_buf[EVENT_QUEUE_SIZE * EVENT_QUEUE_MESSAGE_WORDS] BSP_ALIGN_VARIABLE(8);
volatile UINT g_app_debug_event_queue_status = TX_SUCCESS;
volatile ULONG g_app_debug_event_queue_fail_count = 0U;

static void app_format_unix_timestamp_local(ULONG unix_utc, char *out, size_t out_size);
static bool app_history_entry_is_retained(ULONG entry_unix_utc, ULONG now_unix_utc);
static void app_access_log_prune_locked(void);
static void app_metric_log_prune_locked(void);

static void app_store_last_uid(const char *uid)
{
    if ((NULL == uid) || ('\0' == uid[0]))
    {
        return;
    }

    app_state_lock();
    strncpy((char *) g_app_state.last_uid, uid, UID_MAX_LEN - 1U);
    g_app_state.last_uid[UID_MAX_LEN - 1U] = '\0';
    app_state_unlock();
}

static bool app_time_get_utc_locked(ULONG *out_unix_utc)
{
    ULONG now_tick;

    if ((!g_app_time_synced) || (NULL == out_unix_utc))
    {
        return false;
    }

    now_tick = tx_time_get();
    *out_unix_utc = g_app_time_base_utc + ((now_tick - g_app_time_base_tick) / TX_TIMER_TICKS_PER_SECOND);
    return true;
}

static void app_format_timestamp_from_tick_utc(ULONG tick, ULONG unix_utc, char *out, size_t out_size)
{
    ULONG total_seconds;
    ULONG hours;
    ULONG minutes;
    ULONG seconds;

    if ((NULL == out) || (0U == out_size))
    {
        return;
    }

    if (unix_utc > 0U)
    {
        app_format_unix_timestamp_local(unix_utc, out, out_size);
        return;
    }

    if (tick > 0U)
    {
        total_seconds = tick / TX_TIMER_TICKS_PER_SECOND;
        hours = (total_seconds / 3600U) % 24U;
        minutes = (total_seconds % 3600U) / 60U;
        seconds = total_seconds % 60U;
        snprintf(out,
                 out_size,
                 "T+%02lu:%02lu:%02lu",
                 (unsigned long) hours,
                 (unsigned long) minutes,
                 (unsigned long) seconds);
        return;
    }

    snprintf(out, out_size, "Sem horario");
}

static void app_access_log_push_locked(const app_access_log_entry_t *entry)
{
    if (NULL == entry)
    {
        return;
    }

    g_access_log[g_access_log_next] = *entry;
    g_access_log_next = (g_access_log_next + 1) % ACCESS_LOG_SIZE;
    if (g_access_log_count < ACCESS_LOG_SIZE)
    {
        g_access_log_count++;
    }
}

static void app_metric_log_push_locked(const app_metric_entry_t *entry)
{
    if (NULL == entry)
    {
        return;
    }

    g_metric_log[g_metric_log_next] = *entry;
    g_metric_log_next = (g_metric_log_next + 1) % APP_METRIC_LOG_SIZE;
    if (g_metric_log_count < APP_METRIC_LOG_SIZE)
    {
        g_metric_log_count++;
    }
}

static bool app_history_entry_is_retained(ULONG entry_unix_utc, ULONG now_unix_utc)
{
    if ((0U == now_unix_utc) || (entry_unix_utc >= now_unix_utc))
    {
        return true;
    }

    return ((now_unix_utc - entry_unix_utc) <= APP_HISTORY_RETENTION_SECONDS);
}

static void app_access_log_prune_locked(void)
{
    static app_access_log_entry_t retained_entries[ACCESS_LOG_SIZE];
    ULONG now_unix_utc = 0U;
    int retained_count = 0;

    if (!app_time_get_utc_locked(&now_unix_utc))
    {
        return;
    }

    for (int i = 0; i < g_access_log_count; i++)
    {
        int source_index = (g_access_log_next - g_access_log_count + i);
        while (source_index < 0)
        {
            source_index += ACCESS_LOG_SIZE;
        }
        source_index %= ACCESS_LOG_SIZE;

        if (app_history_entry_is_retained(g_access_log[source_index].unix_utc, now_unix_utc))
        {
            retained_entries[retained_count++] = g_access_log[source_index];
        }
    }

    if (retained_count == g_access_log_count)
    {
        return;
    }

    memset(g_access_log, 0, sizeof(g_access_log));
    g_access_log_next = 0;
    g_access_log_count = 0;

    for (int i = 0; i < retained_count; i++)
    {
        app_access_log_push_locked(&retained_entries[i]);
    }
}

static void app_metric_log_prune_locked(void)
{
    static app_metric_entry_t retained_entries[APP_METRIC_LOG_SIZE];
    ULONG now_unix_utc = 0U;
    int retained_count = 0;

    if (!app_time_get_utc_locked(&now_unix_utc))
    {
        return;
    }

    for (int i = 0; i < g_metric_log_count; i++)
    {
        int source_index = (g_metric_log_next - g_metric_log_count + i);
        while (source_index < 0)
        {
            source_index += APP_METRIC_LOG_SIZE;
        }
        source_index %= APP_METRIC_LOG_SIZE;

        if (app_history_entry_is_retained(g_metric_log[source_index].unix_utc, now_unix_utc))
        {
            retained_entries[retained_count++] = g_metric_log[source_index];
        }
    }

    if (retained_count == g_metric_log_count)
    {
        return;
    }

    memset(g_metric_log, 0, sizeof(g_metric_log));
    g_metric_log_next = 0;
    g_metric_log_count = 0;

    for (int i = 0; i < retained_count; i++)
    {
        app_metric_log_push_locked(&retained_entries[i]);
    }
}

static void app_access_log_add_with_user_locked(app_event_type_t type,
                                                const char *data,
                                                const char *user_override,
                                                app_access_log_entry_t *out_entry)
{
    app_access_log_entry_t entry;
    const char *user_text = (const char *) g_app_state.last_user;

    memset(&entry, 0, sizeof(entry));
    entry.tick = tx_time_get();
    entry.type = type;
    (void) app_time_get_utc_locked(&entry.unix_utc);

    if (NULL != data)
    {
        strncpy(entry.data, data, sizeof(entry.data) - 1U);
        entry.data[sizeof(entry.data) - 1U] = '\0';
    }

    if ((NULL != user_override) && ('\0' != user_override[0]))
    {
        user_text = user_override;
    }

    strncpy(entry.user, user_text, sizeof(entry.user) - 1U);
    entry.user[sizeof(entry.user) - 1U] = '\0';

    app_access_log_push_locked(&entry);

    if (NULL != out_entry)
    {
        *out_entry = entry;
    }
}

static void app_access_log_add_locked(app_event_type_t type, const char *data, app_access_log_entry_t *out_entry)
{
    app_access_log_add_with_user_locked(type, data, NULL, out_entry);
}

static app_metric_kind_t app_metric_kind_from_text(const char *metric_name)
{
    if (NULL == metric_name)
    {
        return APP_METRIC_KIND_UNKNOWN;
    }

    if (0 == strcmp(metric_name, "RFID read latency")) { return APP_METRIC_KIND_RFID_READ; }
    if (0 == strcmp(metric_name, "Auth latency")) { return APP_METRIC_KIND_AUTH; }
    if (0 == strcmp(metric_name, "Door actuation")) { return APP_METRIC_KIND_DOOR_ACTUATION; }
    if (0 == strcmp(metric_name, "HTTP latency")) { return APP_METRIC_KIND_HTTP; }
    if (0 == strcmp(metric_name, "JSON import")) { return APP_METRIC_KIND_JSON_IMPORT; }
    if (0 == strcmp(metric_name, "Storage persist latency")) { return APP_METRIC_KIND_STORAGE_PERSIST; }
    if (0 == strcmp(metric_name, "Storage load latency")) { return APP_METRIC_KIND_STORAGE_LOAD; }
    if (0 == strcmp(metric_name, "UI result latency")) { return APP_METRIC_KIND_UI_RESULT; }
    if (0 == strcmp(metric_name, "Reboot persistence")) { return APP_METRIC_KIND_REBOOT_PERSISTENCE; }
    return APP_METRIC_KIND_UNKNOWN;
}

static app_metric_case_t app_metric_case_from_text(const char *case_name)
{
    if (NULL == case_name)
    {
        return APP_METRIC_CASE_UNKNOWN;
    }

    if (0 == strcmp(case_name, "UID 4 bytes")) { return APP_METRIC_CASE_UID_4_BYTES; }
    if (0 == strcmp(case_name, "UID 7 bytes")) { return APP_METRIC_CASE_UID_7_BYTES; }
    if (0 == strcmp(case_name, "UID 10 bytes")) { return APP_METRIC_CASE_UID_10_BYTES; }
    if (0 == strcmp(case_name, "autorizado")) { return APP_METRIC_CASE_AUTH_OK; }
    if (0 == strcmp(case_name, "negado")) { return APP_METRIC_CASE_AUTH_DENIED; }
    if (0 == strcmp(case_name, "/login")) { return APP_METRIC_CASE_ROUTE_LOGIN; }
    if (0 == strcmp(case_name, "/admin_profiles")) { return APP_METRIC_CASE_ROUTE_ADMIN_PROFILES; }
    if (0 == strcmp(case_name, "/import_profiles")) { return APP_METRIC_CASE_ROUTE_IMPORT_PROFILES; }
    if (0 == strcmp(case_name, "/upload_photo")) { return APP_METRIC_CASE_ROUTE_UPLOAD_PHOTO; }
    if (0 == strcmp(case_name, "/metrics")) { return APP_METRIC_CASE_ROUTE_METRICS; }
    if (0 == strcmp(case_name, "10 perfis")) { return APP_METRIC_CASE_IMPORT_10_PROFILES; }
    if (0 == strcmp(case_name, "50 perfis")) { return APP_METRIC_CASE_IMPORT_50_PROFILES; }
    if (0 == strcmp(case_name, "50+ perfis")) { return APP_METRIC_CASE_IMPORT_50_PLUS_PROFILES; }
    if (0 == strcmp(case_name, "users.json")) { return APP_METRIC_CASE_USERS_JSON; }
    if (0 == strcmp(case_name, "access.log")) { return APP_METRIC_CASE_ACCESS_LOG; }
    if (0 == strcmp(case_name, "metrics.log")) { return APP_METRIC_CASE_METRICS_LOG; }
    if (0 == strcmp(case_name, "photo")) { return APP_METRIC_CASE_PHOTO; }
    if (0 == strcmp(case_name, "apos reboot")) { return APP_METRIC_CASE_AFTER_REBOOT; }
    return APP_METRIC_CASE_UNKNOWN;
}

const char *app_metric_kind_text(app_metric_kind_t kind)
{
    switch (kind)
    {
        case APP_METRIC_KIND_RFID_READ: return "RFID read latency";
        case APP_METRIC_KIND_AUTH: return "Auth latency";
        case APP_METRIC_KIND_DOOR_ACTUATION: return "Door actuation";
        case APP_METRIC_KIND_HTTP: return "HTTP latency";
        case APP_METRIC_KIND_JSON_IMPORT: return "JSON import";
        case APP_METRIC_KIND_STORAGE_PERSIST: return "Storage persist latency";
        case APP_METRIC_KIND_STORAGE_LOAD: return "Storage load latency";
        case APP_METRIC_KIND_UI_RESULT: return "UI result latency";
        case APP_METRIC_KIND_REBOOT_PERSISTENCE: return "Reboot persistence";
        default: return "Unknown";
    }
}

const char *app_metric_case_text(app_metric_case_t case_id)
{
    switch (case_id)
    {
        case APP_METRIC_CASE_UID_4_BYTES: return "UID 4 bytes";
        case APP_METRIC_CASE_UID_7_BYTES: return "UID 7 bytes";
        case APP_METRIC_CASE_UID_10_BYTES: return "UID 10 bytes";
        case APP_METRIC_CASE_AUTH_OK: return "autorizado";
        case APP_METRIC_CASE_AUTH_DENIED: return "negado";
        case APP_METRIC_CASE_ROUTE_LOGIN: return "/login";
        case APP_METRIC_CASE_ROUTE_ADMIN_PROFILES: return "/admin_profiles";
        case APP_METRIC_CASE_ROUTE_IMPORT_PROFILES: return "/import_profiles";
        case APP_METRIC_CASE_ROUTE_UPLOAD_PHOTO: return "/upload_photo";
        case APP_METRIC_CASE_ROUTE_METRICS: return "/metrics";
        case APP_METRIC_CASE_IMPORT_10_PROFILES: return "10 perfis";
        case APP_METRIC_CASE_IMPORT_50_PROFILES: return "50 perfis";
        case APP_METRIC_CASE_IMPORT_50_PLUS_PROFILES: return "50+ perfis";
        case APP_METRIC_CASE_USERS_JSON: return "users.json";
        case APP_METRIC_CASE_ACCESS_LOG: return "access.log";
        case APP_METRIC_CASE_METRICS_LOG: return "metrics.log";
        case APP_METRIC_CASE_PHOTO: return "photo";
        case APP_METRIC_CASE_AFTER_REBOOT: return "apos reboot";
        default: return "-";
    }
}

static bool app_metric_add_locked_internal(app_metric_kind_t kind,
                                           app_metric_case_t case_id,
                                           ULONG duration_ticks,
                                           bool success,
                                           app_metric_entry_t *out_entry)
{
    app_metric_entry_t entry;

    if (APP_METRIC_KIND_UNKNOWN == kind)
    {
        return false;
    }

    memset(&entry, 0, sizeof(entry));
    entry.tick = tx_time_get();
    entry.duration_ticks = duration_ticks;
    entry.success = success;
    entry.kind = kind;
    entry.case_id = case_id;
    (void) app_time_get_utc_locked(&entry.unix_utc);

    app_metric_log_push_locked(&entry);
    if (NULL != out_entry)
    {
        *out_entry = entry;
    }
    return true;
}

static void app_civil_from_days(int64_t days_since_unix_epoch, int *year, unsigned int *month, unsigned int *day)
{
    int64_t era;
    unsigned int day_of_era;
    unsigned int year_of_era;
    unsigned int day_of_year;
    unsigned int month_param;
    int year_local;
    unsigned int month_local;

    days_since_unix_epoch += 719468LL;
    era = (days_since_unix_epoch >= 0LL) ? (days_since_unix_epoch / 146097LL)
                                         : ((days_since_unix_epoch - 146096LL) / 146097LL);
    day_of_era = (unsigned int) (days_since_unix_epoch - (era * 146097LL));
    year_of_era = (day_of_era - (day_of_era / 1460U) + (day_of_era / 36524U) - (day_of_era / 146096U)) / 365U;
    year_local = (int) year_of_era + (int) (era * 400LL);
    day_of_year = day_of_era - (365U * year_of_era + year_of_era / 4U - year_of_era / 100U);
    month_param = (5U * day_of_year + 2U) / 153U;
    *day = day_of_year - (153U * month_param + 2U) / 5U + 1U;
    month_local = (month_param < 10U) ? (month_param + 3U) : (month_param - 9U);
    year_local += (month_local <= 2U) ? 1 : 0;

    *year = year_local;
    *month = month_local;
}

static void app_format_unix_timestamp_local(ULONG unix_utc, char *out, size_t out_size)
{
    int64_t local_seconds;
    int64_t days_since_epoch;
    ULONG seconds_of_day;
    ULONG hours;
    ULONG minutes;
    ULONG seconds;
    int year = 1970;
    unsigned int month = 1U;
    unsigned int day = 1U;

    if ((NULL == out) || (0U == out_size))
    {
        return;
    }

    local_seconds = (int64_t) unix_utc - (3LL * 3600LL);
    if (local_seconds < 0LL)
    {
        local_seconds = 0LL;
    }

    days_since_epoch = local_seconds / 86400LL;
    seconds_of_day = (ULONG) (local_seconds % 86400LL);
    hours = seconds_of_day / 3600U;
    minutes = (seconds_of_day % 3600U) / 60U;
    seconds = seconds_of_day % 60U;

    app_civil_from_days(days_since_epoch, &year, &month, &day);

    snprintf(out,
             out_size,
             "%04d-%02u-%02u:%02luh%02lumin%02lus",
             year,
             month,
             day,
             (unsigned long) hours,
             (unsigned long) minutes,
             (unsigned long) seconds);
}

void app_set_last_identity(const char *uid, const char *user)
{
    app_state_lock();

    if (NULL != uid)
    {
        strncpy((char *) g_app_state.last_uid, uid, UID_MAX_LEN - 1U);
        g_app_state.last_uid[UID_MAX_LEN - 1U] = '\0';
    }

    if (NULL != user)
    {
        strncpy((char *) g_app_state.last_user, user, NAME_MAX_LEN - 1U);
        g_app_state.last_user[NAME_MAX_LEN - 1U] = '\0';
    }

    app_state_unlock();
}

static void app_trimmed_bounds(const char *text, const char **out_start, size_t *out_len)
{
    const char *start = (NULL != text) ? text : "";
    const char *end;

    while (('\0' != *start) && isspace((int) (unsigned char) *start))
    {
        start++;
    }

    end = start + strlen(start);
    while ((end > start) && isspace((int) (unsigned char) *(end - 1)))
    {
        end--;
    }

    if (NULL != out_start)
    {
        *out_start = start;
    }
    if (NULL != out_len)
    {
        *out_len = (size_t) (end - start);
    }
}

static bool app_names_match(const char *lhs, const char *rhs)
{
    const char *lhs_start;
    const char *rhs_start;
    size_t lhs_len;
    size_t rhs_len;

    app_trimmed_bounds(lhs, &lhs_start, &lhs_len);
    app_trimmed_bounds(rhs, &rhs_start, &rhs_len);

    if ((0U == lhs_len) || (lhs_len != rhs_len))
    {
        return false;
    }

    for (size_t i = 0U; i < lhs_len; i++)
    {
        unsigned char lhs_char = (unsigned char) lhs_start[i];
        unsigned char rhs_char = (unsigned char) rhs_start[i];

        if (tolower((int) lhs_char) != tolower((int) rhs_char))
        {
            return false;
        }
    }

    return true;
}

static bool app_lookup_primary_uid_for_user_name_nowait(const char *user_name, char *out_uid, size_t out_uid_size)
{
    int user_count;

    if ((NULL != out_uid) && (out_uid_size > 0U))
    {
        out_uid[0] = '\0';
    }

    if ((NULL == user_name) || ('\0' == user_name[0]) || (NULL == out_uid) || (out_uid_size == 0U))
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

        if (!storage_profile_get_nowait(i, &profile) ||
            !app_names_match(profile.name, user_name) ||
            (0U == profile.card_count) ||
            ('\0' == profile.cards[0][0]))
        {
            continue;
        }

        for (size_t j = 0U; ((j + 1U) < out_uid_size) && ('\0' != profile.cards[0][j]); j++)
        {
            out_uid[j] = profile.cards[0][j];
            out_uid[j + 1U] = '\0';
        }
        return ('\0' != out_uid[0]);
    }

    return false;
}

void app_set_ui_mode(app_ui_mode_t mode)
{
    app_state_lock();
    g_app_state.ui_mode = mode;
    app_state_unlock();
}

app_ui_mode_t app_get_ui_mode(void)
{
    app_ui_mode_t mode;

    app_state_lock();
    mode = g_app_state.ui_mode;
    app_state_unlock();

    return mode;
}

void app_set_enrollment_mode(bool enabled)
{
    app_state_lock();
    g_app_state.enroll_mode = enabled;
    app_state_unlock();
}

bool app_is_enrollment_mode(void)
{
    bool enabled;

    app_state_lock();
    enabled = g_app_state.enroll_mode;
    app_state_unlock();

    return enabled;
}

void app_time_set_utc(ULONG unix_utc)
{
    app_state_lock();
    g_app_time_base_utc = unix_utc;
    g_app_time_base_tick = tx_time_get();
    g_app_time_synced = (unix_utc > 0U);
    app_state_unlock();
}

bool app_time_get_utc(ULONG *out_unix_utc)
{
    bool ok;

    app_state_lock();
    ok = app_time_get_utc_locked(out_unix_utc);
    app_state_unlock();

    return ok;
}

void app_format_access_log_timestamp(const app_access_log_entry_t *entry, char *out, size_t out_size)
{
    if ((NULL == entry) || (NULL == out) || (0U == out_size))
    {
        return;
    }

    app_format_timestamp_from_tick_utc(entry->tick, entry->unix_utc, out, out_size);
}

void hal_entry(void)
{
#define WIPE_QSPI_NOW 0
    #if WIPE_QSPI_NOW
        g_qspi0.p_api->open(g_qspi0.p_ctrl, g_qspi0.p_cfg);
        g_qspi0.p_api->erase(g_qspi0.p_ctrl, (uint8_t *) 0x60000000, 8 * 1024 * 1024);

        bool in_progress = true;
        while (in_progress)
        {
            g_qspi0.p_api->statusGet(g_qspi0.p_ctrl, &in_progress);
        }
        g_qspi0.p_api->close(g_qspi0.p_ctrl);
    #endif
    gpio_init();

    tx_mutex_create(&g_state_mutex, "state_mutex", TX_NO_INHERIT);

    tx_queue_create(&g_event_queue, "event_queue",
                    EVENT_QUEUE_MESSAGE_WORDS,
                    event_queue_buf, sizeof(event_queue_buf));

    tx_thread_create(&thread_ui, "ui", thread_ui_entry, 0,
                     stack_ui, STACK_SIZE_UI, PRIO_UI, PRIO_UI,
                     TX_NO_TIME_SLICE, TX_AUTO_START);

    tx_thread_create(&thread_rfid, "rfid", thread_rfid_entry, 0,
                     stack_rfid, STACK_SIZE_RFID, PRIO_RFID, PRIO_RFID,
                     TX_NO_TIME_SLICE, TX_AUTO_START);

    tx_thread_create(&thread_gpio, "gpio", thread_gpio_entry, 0,
                     stack_gpio, STACK_SIZE_GPIO, PRIO_GPIO, PRIO_GPIO,
                     TX_NO_TIME_SLICE, TX_AUTO_START);

#if !NETWORK_ISOLATION_MODE
    tx_thread_create(&thread_net, "net", thread_net_entry, 0,
                     stack_net, STACK_SIZE_NET, PRIO_NET, PRIO_NET,
                     TX_NO_TIME_SLICE, TX_AUTO_START);

#if !NETWORK_HTTP_ONLY_MODE
    tx_thread_create(&thread_tunnel, "tunnel", thread_tunnel_entry, 0,
                     stack_tunnel, STACK_SIZE_TUNNEL, PRIO_TUNNEL, PRIO_TUNNEL,
                     TX_NO_TIME_SLICE, TX_AUTO_START);

    tx_thread_create(&thread_status, "status", thread_status_entry, 0,
                     stack_status, STACK_SIZE_STATUS, PRIO_STATUS, PRIO_STATUS,
                     TX_NO_TIME_SLICE, TX_AUTO_START);
#endif
#endif
}

void app_state_lock(void)   { tx_mutex_get(&g_state_mutex, TX_WAIT_FOREVER); }
void app_state_unlock(void) { tx_mutex_put(&g_state_mutex); }

void app_set_door(bool open)
{
    app_state_lock();
    g_app_state.door_open = open;
    gpio_set_door(open);
    app_state_unlock();
}

void app_set_light(bool on)
{
    app_state_lock();
    g_app_state.light_on = on;
    gpio_set_light(on);
    app_state_unlock();
}

void app_post_event(app_event_type_t type, const char * data)
{
    app_event_t ev = { .type = type };
    app_access_log_entry_t access_log_entry;

    memset(&access_log_entry, 0, sizeof(access_log_entry));

    switch (type)
    {
        case EVENT_DOOR_OPEN:
        {
            const char *door_user = ((NULL != data) && ('\0' != data[0])) ? data : "Abertura manual";

            app_set_last_identity("", door_user);
            app_metric_begin_ui_result("autorizado");
            app_metric_begin_door(door_user);
            app_set_door(true);
            break;
        }

        case EVENT_LIGHT_ON:
            app_set_light(true);
            break;

        case EVENT_LIGHT_OFF:
            app_set_light(false);
            break;

        case EVENT_RFID_AUTH_OK:
        case EVENT_RFID_AUTH_FAIL:
        case EVENT_CARD_REGISTERED:
        case EVENT_CARD_ALREADY_REGISTERED:
        case EVENT_CARD_REGISTRATION_FAILED:
            app_store_last_uid(data);
            break;

        default:
            break;
    }

    if (data)
    {
        strncpy(ev.data, data, sizeof(ev.data) - 1U);
        ev.data[sizeof(ev.data) - 1U] = '\0';
    }

    switch (type)
    {
        case EVENT_RFID_AUTH_OK:
        case EVENT_RFID_AUTH_FAIL:
        case EVENT_DOOR_OPEN:
        case EVENT_CARD_REGISTERED:
        case EVENT_CARD_ALREADY_REGISTERED:
        case EVENT_CARD_REGISTRATION_FAILED:
            app_state_lock();
            app_access_log_add_locked(type, data, &access_log_entry);
            app_state_unlock();
            (void) storage_access_log_enqueue(&access_log_entry);
            break;

        default:
            break;
    }

    g_app_debug_event_queue_status = tx_queue_send(&g_event_queue, &ev, TX_NO_WAIT);
    if (TX_SUCCESS != g_app_debug_event_queue_status)
    {
        g_app_debug_event_queue_fail_count++;
    }
}

void app_post_door_open_event(const char *source, const char *user)
{
    app_event_t ev = { .type = EVENT_DOOR_OPEN };
    app_access_log_entry_t access_log_entry;
    char door_uid[UID_MAX_LEN];
    const char *door_source = ((NULL != source) && ('\0' != source[0])) ? source : "API";
    const char *door_user = ((NULL != user) && ('\0' != user[0])) ? user : door_source;
    bool found_profile_uid;

    memset(&access_log_entry, 0, sizeof(access_log_entry));

    found_profile_uid = app_lookup_primary_uid_for_user_name_nowait(door_user, door_uid, sizeof(door_uid));
    app_set_last_identity(found_profile_uid ? door_uid : "", door_user);
    app_metric_begin_ui_result("autorizado");
    app_metric_begin_door(door_source);
    app_set_door(true);

    strncpy(ev.data, door_source, sizeof(ev.data) - 1U);
    ev.data[sizeof(ev.data) - 1U] = '\0';

    app_state_lock();
    app_access_log_add_with_user_locked(EVENT_DOOR_OPEN, door_source, door_user, &access_log_entry);
    app_state_unlock();
    (void) storage_access_log_enqueue(&access_log_entry);

    g_app_debug_event_queue_status = tx_queue_send(&g_event_queue, &ev, TX_NO_WAIT);
    if (TX_SUCCESS != g_app_debug_event_queue_status)
    {
        g_app_debug_event_queue_fail_count++;
    }
}

int app_access_log_snapshot(app_access_log_entry_t *out_entries, int max_entries)
{
    int count;

    if ((NULL == out_entries) || (max_entries <= 0))
    {
        return 0;
    }

    (void) storage_access_log_ensure_loaded();

    app_state_lock();
    app_access_log_prune_locked();
    count = g_access_log_count;
    if (count > max_entries)
    {
        count = max_entries;
    }

    for (int i = 0; i < count; i++)
    {
        int source_index = (g_access_log_next - g_access_log_count + i);
        while (source_index < 0)
        {
            source_index += ACCESS_LOG_SIZE;
        }
        source_index %= ACCESS_LOG_SIZE;
        out_entries[i] = g_access_log[source_index];
    }
    app_state_unlock();

    return count;
}

void app_metric_add(const char *metric_name, const char *case_name, ULONG duration_ticks, bool success)
{
    app_metric_kind_t kind = app_metric_kind_from_text(metric_name);
    app_metric_case_t case_id = app_metric_case_from_text(case_name);
    app_metric_entry_t entry;
    bool added;

    app_state_lock();
    added = app_metric_add_locked_internal(kind, case_id, duration_ticks, success, &entry);
    app_state_unlock();
    if (added)
    {
        (void) storage_metric_enqueue(&entry);
    }
}

void app_metric_add_no_persist(const char *metric_name, const char *case_name, ULONG duration_ticks, bool success)
{
    app_metric_kind_t kind = app_metric_kind_from_text(metric_name);
    app_metric_case_t case_id = app_metric_case_from_text(case_name);

    app_state_lock();
    (void) app_metric_add_locked_internal(kind, case_id, duration_ticks, success, NULL);
    app_state_unlock();
}

int app_metric_snapshot(app_metric_entry_t *out_entries, int max_entries)
{
    int count;

    if ((NULL == out_entries) || (max_entries <= 0))
    {
        return 0;
    }

    (void) storage_metrics_ensure_loaded();

    app_state_lock();
    app_metric_log_prune_locked();
    count = g_metric_log_count;
    if (count > max_entries)
    {
        count = max_entries;
    }

    for (int i = 0; i < count; i++)
    {
        int source_index = (g_metric_log_next - g_metric_log_count + i);
        while (source_index < 0)
        {
            source_index += APP_METRIC_LOG_SIZE;
        }
        source_index %= APP_METRIC_LOG_SIZE;
        out_entries[i] = g_metric_log[source_index];
    }
    app_state_unlock();

    return count;
}

void app_access_log_restore(const app_access_log_entry_t *entries, int entry_count)
{
    static app_access_log_entry_t current_entries[ACCESS_LOG_SIZE];
    int current_count = 0;

    if ((NULL == entries) || (entry_count <= 0))
    {
        return;
    }

    app_state_lock();

    current_count = g_access_log_count;
    if (current_count > ACCESS_LOG_SIZE)
    {
        current_count = ACCESS_LOG_SIZE;
    }

    for (int i = 0; i < current_count; i++)
    {
        int source_index = (g_access_log_next - g_access_log_count + i);
        while (source_index < 0)
        {
            source_index += ACCESS_LOG_SIZE;
        }
        source_index %= ACCESS_LOG_SIZE;
        current_entries[i] = g_access_log[source_index];
    }

    memset(g_access_log, 0, sizeof(g_access_log));
    g_access_log_next = 0;
    g_access_log_count = 0;

    if (entry_count > ACCESS_LOG_SIZE)
    {
        entries += (entry_count - ACCESS_LOG_SIZE);
        entry_count = ACCESS_LOG_SIZE;
    }

    for (int i = 0; i < entry_count; i++)
    {
        app_access_log_push_locked(&entries[i]);
    }

    for (int i = 0; i < current_count; i++)
    {
        app_access_log_push_locked(&current_entries[i]);
    }

    app_access_log_prune_locked();
    app_state_unlock();
}

void app_metric_restore(const app_metric_entry_t *entries, int entry_count)
{
    static app_metric_entry_t current_entries[APP_METRIC_LOG_SIZE];
    int current_count = 0;

    if ((NULL == entries) || (entry_count <= 0))
    {
        return;
    }

    app_state_lock();

    current_count = g_metric_log_count;
    if (current_count > APP_METRIC_LOG_SIZE)
    {
        current_count = APP_METRIC_LOG_SIZE;
    }

    for (int i = 0; i < current_count; i++)
    {
        int source_index = (g_metric_log_next - g_metric_log_count + i);
        while (source_index < 0)
        {
            source_index += APP_METRIC_LOG_SIZE;
        }
        source_index %= APP_METRIC_LOG_SIZE;
        current_entries[i] = g_metric_log[source_index];
    }

    memset(g_metric_log, 0, sizeof(g_metric_log));
    g_metric_log_next = 0;
    g_metric_log_count = 0;

    if (entry_count > APP_METRIC_LOG_SIZE)
    {
        entries += (entry_count - APP_METRIC_LOG_SIZE);
        entry_count = APP_METRIC_LOG_SIZE;
    }

    for (int i = 0; i < entry_count; i++)
    {
        app_metric_log_push_locked(&entries[i]);
    }

    for (int i = 0; i < current_count; i++)
    {
        app_metric_log_push_locked(&current_entries[i]);
    }

    app_metric_log_prune_locked();
    app_state_unlock();
}

void app_format_metric_timestamp(const app_metric_entry_t *entry, char *out, size_t out_size)
{
    if ((NULL == entry) || (NULL == out) || (0U == out_size))
    {
        return;
    }

    app_format_timestamp_from_tick_utc(entry->tick, entry->unix_utc, out, out_size);
}

void app_metric_begin_door(const char *case_name)
{
    app_state_lock();
    g_app_metric_door_pending = true;
    g_app_metric_door_start_tick = tx_time_get();
    g_app_metric_door_case = app_metric_case_from_text(case_name);
    app_state_unlock();
}

void app_metric_finish_door(bool success)
{
    ULONG start_tick = 0U;
    app_metric_case_t case_id = APP_METRIC_CASE_UNKNOWN;
    bool pending = false;

    app_state_lock();
    pending = g_app_metric_door_pending;
    if (pending)
    {
        start_tick = g_app_metric_door_start_tick;
        case_id = g_app_metric_door_case;
        g_app_metric_door_pending = false;
        g_app_metric_door_start_tick = 0U;
        g_app_metric_door_case = APP_METRIC_CASE_UNKNOWN;
    }
    app_state_unlock();

    if (pending)
    {
        app_metric_add(app_metric_kind_text(APP_METRIC_KIND_DOOR_ACTUATION),
                       app_metric_case_text(case_id),
                       tx_time_get() - start_tick,
                       success);
    }
}

void app_metric_begin_ui_result(const char *case_name)
{
    app_state_lock();
    g_app_metric_ui_pending = true;
    g_app_metric_ui_start_tick = tx_time_get();
    g_app_metric_ui_case = app_metric_case_from_text(case_name);
    app_state_unlock();
}

void app_metric_finish_ui_result(bool success)
{
    ULONG start_tick = 0U;
    app_metric_case_t case_id = APP_METRIC_CASE_UNKNOWN;
    bool pending = false;

    app_state_lock();
    pending = g_app_metric_ui_pending;
    if (pending)
    {
        start_tick = g_app_metric_ui_start_tick;
        case_id = g_app_metric_ui_case;
        g_app_metric_ui_pending = false;
        g_app_metric_ui_start_tick = 0U;
        g_app_metric_ui_case = APP_METRIC_CASE_UNKNOWN;
    }
    app_state_unlock();

    if (pending)
    {
        app_metric_add(app_metric_kind_text(APP_METRIC_KIND_UI_RESULT),
                       app_metric_case_text(case_id),
                       tx_time_get() - start_tick,
                       success);
    }
}
