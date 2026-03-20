#include "main.h"
#include "net.h"
#include "rfid.h"
#include "ui.h"
#include "gpio.h"
#include "storage.h"
#include <stdio.h>
#include <stdint.h>

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
#define STACK_SIZE_NET       4096
#define STACK_SIZE_TUNNEL    2048
#define STACK_SIZE_GPIO      1024
#define STACK_SIZE_STATUS    1024

#define PRIO_RFID            3
#define PRIO_UI              2
#define PRIO_NET             4
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
static int g_access_log_next = 0;
static int g_access_log_count = 0;
static ULONG g_app_time_base_utc = 0U;
static ULONG g_app_time_base_tick = 0U;
static bool g_app_time_synced = false;
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
static uint8_t event_queue_buf[EVENT_QUEUE_SIZE * sizeof(app_event_t)];
volatile UINT g_app_debug_event_queue_status = TX_SUCCESS;
volatile ULONG g_app_debug_event_queue_fail_count = 0U;

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

static void app_access_log_add_locked(app_event_type_t type, const char *data)
{
    app_access_log_entry_t entry;

    memset(&entry, 0, sizeof(entry));
    entry.tick = tx_time_get();
    entry.type = type;
    (void) app_time_get_utc_locked(&entry.unix_utc);

    if (NULL != data)
    {
        strncpy(entry.data, data, sizeof(entry.data) - 1U);
        entry.data[sizeof(entry.data) - 1U] = '\0';
    }

    strncpy(entry.user, (const char *) g_app_state.last_user, sizeof(entry.user) - 1U);
    entry.user[sizeof(entry.user) - 1U] = '\0';

    app_access_log_push_locked(&entry);
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

    if ((NULL != uid) && ('\0' != uid[0]))
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
    ULONG total_seconds;
    ULONG hours;
    ULONG minutes;
    ULONG seconds;

    if ((NULL == entry) || (NULL == out) || (0U == out_size))
    {
        return;
    }

    if (entry->unix_utc > 0U)
    {
        app_format_unix_timestamp_local(entry->unix_utc, out, out_size);
        return;
    }

    if (entry->tick > 0U)
    {
        total_seconds = entry->tick / TX_TIMER_TICKS_PER_SECOND;
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
                    sizeof(app_event_t) / sizeof(uint32_t),
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

    switch (type)
    {
        case EVENT_DOOR_OPEN:
            app_set_door(true);
            break;

        case EVENT_DOOR_CLOSE:
            app_set_door(false);
            break;

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
            app_access_log_add_locked(type, data);
            app_state_unlock();
            (void) storage_access_log_persist_now();
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

int app_access_log_snapshot(app_access_log_entry_t *out_entries, int max_entries)
{
    int count;

    if ((NULL == out_entries) || (max_entries <= 0))
    {
        return 0;
    }

    app_state_lock();
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

    app_state_unlock();
}
