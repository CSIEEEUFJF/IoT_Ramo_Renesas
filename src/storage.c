#include "storage.h"
#include "main.h"
#include "hal_data.h"
#include "ui.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

/* =========================================================================
   ESCUDOS CONTRA SIGTRAP
   Isto impede a placa de travar se tentar ler a flash antes de formatar.
   ========================================================================= */
UINT g_lx_nor0_system_error(UINT error_code);
UINT g_lx_nor0_system_error(UINT error_code)
{
    SSP_PARAMETER_NOT_USED(error_code);
    return 1U;
}

extern ssp_err_t fx_media_init0_format(void);
/* ========================================================================= */

#define JSON_BUFFER_SIZE 8192
#define ACCESS_LOG_BUFFER_SIZE 4096
#define ACCESS_LOG_ROTATE_SIZE (64U * 1024U)
#define ACCESS_LOG_PENDING_SIZE 48U
#define STORAGE_MEETING_MODE_MAX_ALLOWED_CARDS (STORAGE_MAX_USERS * STORAGE_MAX_CARDS_PER_USER)
#define ACCESS_LOG_FILE_NAME "access.log"
#define ACCESS_LOG_ARCHIVE_FILE_NAME "access.bak"
#define METRICS_LOG_BUFFER_SIZE 6144
#define STORAGE_MEDIA_SAFE_DELAY_TICKS (2U * TX_TIMER_TICKS_PER_SECOND)
#define STORAGE_THREAD_STACK_SIZE 2048U
#define STORAGE_PHOTO_COPY_ROWS 8U
#define STORAGE_PHOTO_FILE_MAGIC 0x50485247UL

#define STORAGE_PERSIST_STATUS_IDLE     (0U)
#define STORAGE_PERSIST_STATUS_PENDING  (1U)
#define STORAGE_PERSIST_STATUS_SUCCESS  (2U)
#define STORAGE_PERSIST_STATUS_FAILED   (3U)

typedef storage_user_profile_t user_t;

static user_t g_users[STORAGE_MAX_USERS];
static user_t g_recent_user;
static int g_user_count = 0;
static bool g_storage_initialized = false;
static bool g_storage_mutex_ready = false;
static bool g_storage_media_mutex_ready = false;
static bool g_storage_media_ready = false;
static bool g_storage_loaded = false;
static bool g_storage_access_log_loaded = false;
static bool g_storage_metrics_loaded = false;
static bool g_storage_thread_ready = false;
static bool g_storage_persist_requested = false;
static bool g_storage_access_log_persist_requested = false;
static bool g_storage_metrics_persist_requested = false;
static bool g_storage_photo_persist_requested = false;
static bool g_storage_load_requested = false;
static bool g_recent_user_valid = false;
static bool g_storage_meeting_mode_active = false;
static TX_MUTEX g_storage_mutex;
static TX_MUTEX g_storage_media_mutex;
static TX_SEMAPHORE g_storage_persist_semaphore;
static TX_THREAD g_storage_thread;
static ULONG g_storage_thread_stack[STORAGE_THREAD_STACK_SIZE / sizeof(ULONG)];
static FX_FILE g_users_file;
static FX_FILE g_access_log_file;
static FX_FILE g_metrics_file;
static FX_FILE g_photo_file;
static char g_storage_load_json[JSON_BUFFER_SIZE];
static char g_storage_persist_json[JSON_BUFFER_SIZE];
static char g_storage_persist_work_json[JSON_BUFFER_SIZE];
static app_access_log_entry_t g_storage_access_log_pending[ACCESS_LOG_PENDING_SIZE];
static char g_storage_meeting_mode_allowed_cards[STORAGE_MEETING_MODE_MAX_ALLOWED_CARDS][UID_MAX_LEN];
static char g_storage_access_log_work_text[ACCESS_LOG_BUFFER_SIZE];
static char g_storage_metrics_text[METRICS_LOG_BUFFER_SIZE];
static char g_storage_pending_photo_id[STORAGE_PHOTO_ID_MAX_LEN];
static char g_storage_photo_work_id[STORAGE_PHOTO_ID_MAX_LEN];
static size_t g_storage_persist_json_size = 0U;
static size_t g_storage_persist_work_json_size = 0U;
static size_t g_storage_access_log_work_text_size = 0U;
static size_t g_storage_metrics_text_size = 0U;
static unsigned int g_storage_access_log_pending_count = 0U;
static unsigned int g_storage_meeting_mode_selected_profiles = 0U;
static unsigned int g_storage_meeting_mode_allowed_count = 0U;
static volatile ULONG g_storage_persist_status = STORAGE_PERSIST_STATUS_IDLE;
volatile ULONG g_storage_debug_worker_runs = 0U;
volatile ULONG g_storage_debug_last_stage = 0U;
volatile ULONG g_storage_debug_last_media_status = 0U;
volatile ULONG g_storage_debug_last_save_status = 0U;
volatile ULONG g_storage_debug_last_load_status = 0U;
volatile ULONG g_storage_debug_last_saved_bytes = 0U;
volatile ULONG g_storage_debug_last_read_bytes = 0U;
volatile ULONG g_storage_debug_last_user_count = 0U;

typedef struct st_storage_photo_file_header
{
    ULONG magic;
    USHORT width;
    USHORT height;
    ULONG payload_size;
} storage_photo_file_header_t;

static void storage_thread_entry(ULONG initial_input);
static void storage_refresh_persist_snapshot_locked(void);
static void storage_refresh_metrics_snapshot(void);
static void storage_ensure_loaded_locked(void);
static bool storage_save_json_buffer(const char *json_buffer, size_t json_size);
static bool storage_save_access_log_buffer(const char *log_buffer, size_t log_size);
static bool storage_save_metrics_buffer(const char *log_buffer, size_t log_size);
static bool storage_load_users_now(void);
static bool storage_load_access_log_now(void);
static bool storage_load_metrics_now(void);
static bool storage_parse_access_log_text(const char *log_buffer);
static void storage_media_lock(void);
static void storage_media_unlock(void);
static void storage_photo_filename(const char *photo_id, char *out, size_t out_size);
static bool storage_save_photo_file(const char *photo_id);
static bool storage_load_photo_file(const char *photo_id);

static bool storage_media_access_allowed(void)
{
    return (tx_time_get() >= STORAGE_MEDIA_SAFE_DELAY_TICKS);
}

static UINT storage_media_open_internal(void)
{
    UINT status;

    if (g_storage_media_ready)
    {
        return FX_SUCCESS;
    }

    #define FORMAT_DRIVE_NOW 0
    #if FORMAT_DRIVE_NOW
        g_qspi0.p_api->open(g_qspi0.p_ctrl, g_qspi0.p_cfg);
        g_qspi0.p_api->erase(g_qspi0.p_ctrl, (uint8_t *) 0x60000000, 8 * 1024 * 1024);

        bool in_progress = true;
        while (in_progress)
        {
            g_qspi0.p_api->statusGet(g_qspi0.p_ctrl, &in_progress);
            tx_thread_sleep(1);
        }
        g_qspi0.p_api->close(g_qspi0.p_ctrl);
        fx_media_init0_format();
    #endif

    status = fx_media_init0_open();
    g_storage_debug_last_media_status = status;
    if (FX_SUCCESS == status)
    {
        g_storage_media_ready = true;
    }

    return status;
}

static bool storage_media_open(void)
{
    return (FX_SUCCESS == storage_media_open_internal());
}

static bool storage_read_file_chunk(const char *filename,
                                    ULONG offset,
                                    void *out,
                                    size_t out_size,
                                    ULONG *out_read)
{
    FX_FILE *file_ptr = NULL;
    UINT status;
    ULONG actual_read = 0U;
    bool file_opened = false;

    if ((NULL == filename) || ('\0' == filename[0]) || (NULL == out) || (0U == out_size))
    {
        return false;
    }

    storage_media_lock();
    if (!storage_media_open())
    {
        storage_media_unlock();
        return false;
    }

    if (0 == strcmp(filename, "users.json"))
    {
        file_ptr = &g_users_file;
    }
    else if (0 == strcmp(filename, "access.log"))
    {
        file_ptr = &g_access_log_file;
    }
    else if (0 == strcmp(filename, "metrics.log"))
    {
        file_ptr = &g_metrics_file;
    }
    else
    {
        file_ptr = &g_photo_file;
    }

    status = fx_file_open(&g_fx_media0, file_ptr, (CHAR *) filename, FX_OPEN_FOR_READ);
    if (FX_SUCCESS == status)
    {
        file_opened = true;
        status = fx_file_seek(file_ptr, offset);
    }
    if (FX_SUCCESS == status)
    {
        status = fx_file_read(file_ptr, out, (ULONG) out_size, &actual_read);
    }

    if (file_opened)
    {
        (void) fx_file_close(file_ptr);
    }
    (void) fx_media_close(&g_fx_media0);
    g_storage_media_ready = false;
    storage_media_unlock();

    if (NULL != out_read)
    {
        *out_read = actual_read;
    }

    return (FX_SUCCESS == status);
}

static bool storage_get_file_size(const char *filename, ULONG *out_size)
{
    FX_FILE *file_ptr = NULL;
    UINT status;
    ULONG actual_size = 0U;
    bool file_opened = false;

    if ((NULL == filename) || ('\0' == filename[0]))
    {
        return false;
    }

    storage_media_lock();
    if (!storage_media_open())
    {
        storage_media_unlock();
        return false;
    }

    if (0 == strcmp(filename, "users.json"))
    {
        file_ptr = &g_users_file;
    }
    else if (0 == strcmp(filename, "access.log"))
    {
        file_ptr = &g_access_log_file;
    }
    else if (0 == strcmp(filename, "metrics.log"))
    {
        file_ptr = &g_metrics_file;
    }
    else
    {
        file_ptr = &g_photo_file;
    }

    status = fx_file_open(&g_fx_media0, file_ptr, (CHAR *) filename, FX_OPEN_FOR_READ);
    if (FX_SUCCESS == status)
    {
        file_opened = true;
        actual_size = (ULONG) file_ptr->fx_file_current_file_size;
    }

    if (file_opened)
    {
        (void) fx_file_close(file_ptr);
    }
    (void) fx_media_close(&g_fx_media0);
    g_storage_media_ready = false;
    storage_media_unlock();

    if (NULL != out_size)
    {
        *out_size = actual_size;
    }

    return (FX_SUCCESS == status);
}

static void storage_lock(void)
{
    if (g_storage_mutex_ready)
    {
        tx_mutex_get(&g_storage_mutex, TX_WAIT_FOREVER);
    }
}

static void storage_unlock(void)
{
    if (g_storage_mutex_ready)
    {
        tx_mutex_put(&g_storage_mutex);
    }
}

static void storage_media_lock(void)
{
    if (g_storage_media_mutex_ready)
    {
        tx_mutex_get(&g_storage_media_mutex, TX_WAIT_FOREVER);
    }
}

static void storage_media_unlock(void)
{
    if (g_storage_media_mutex_ready)
    {
        tx_mutex_put(&g_storage_media_mutex);
    }
}

static void storage_copy_text(char *dest, size_t dest_size, const char *src, size_t src_len)
{
    if ((NULL == dest) || (0U == dest_size))
    {
        return;
    }
    if (NULL == src)
    {
        dest[0] = '\0';
        return;
    }
    if (src_len >= dest_size)
    {
        src_len = dest_size - 1U;
    }
    memcpy(dest, src, src_len);
    dest[src_len] = '\0';
}

static void storage_copy_clean_uid(char *dest, size_t dest_size, const char *src)
{
    size_t write_index = 0U;

    if ((NULL == dest) || (0U == dest_size))
    {
        return;
    }

    dest[0] = '\0';
    if (NULL == src)
    {
        return;
    }

    while (('\0' != *src) && (write_index + 1U < dest_size))
    {
        char ch = *src++;

        if (isspace((unsigned char) ch) || (',' == ch) || (';' == ch))
        {
            continue;
        }

        if ((ch >= 'a') && (ch <= 'z'))
        {
            ch = (char) (ch - ('a' - 'A'));
        }

        dest[write_index++] = ch;
    }

    dest[write_index] = '\0';
}

static void storage_copy_log_field(char *dest, size_t dest_size, const char *src)
{
    size_t write_index = 0U;

    if ((NULL == dest) || (0U == dest_size))
    {
        return;
    }

    dest[0] = '\0';
    if (NULL == src)
    {
        return;
    }

    while (('\0' != *src) && (write_index + 1U < dest_size))
    {
        char ch = *src++;

        if (('|' == ch) || ('\r' == ch) || ('\n' == ch))
        {
            ch = ' ';
        }

        dest[write_index++] = ch;
    }

    dest[write_index] = '\0';
}

static unsigned int storage_format_access_log_entries(const app_access_log_entry_t *entries,
                                                      unsigned int entry_count,
                                                      char *out,
                                                      size_t out_size,
                                                      size_t *out_len)
{
    size_t offset = 0U;
    unsigned int used_count = 0U;

    if ((NULL == out) || (0U == out_size))
    {
        return 0U;
    }

    out[0] = '\0';
    if (NULL != out_len)
    {
        *out_len = 0U;
    }

    if ((NULL == entries) || (0U == entry_count))
    {
        return 0U;
    }

    for (unsigned int i = 0U; i < entry_count; i++)
    {
        char data[UID_MAX_LEN];
        char user[NAME_MAX_LEN];
        int written;

        storage_copy_log_field(data, sizeof(data), entries[i].data);
        storage_copy_log_field(user, sizeof(user), entries[i].user);

        written = snprintf(&out[offset],
                           out_size - offset,
                           "%lu|%u|%s|%s\n",
                           (unsigned long) entries[i].unix_utc,
                           (unsigned int) entries[i].type,
                           data,
                           user);
        if ((written <= 0) || ((size_t) written >= (out_size - offset)))
        {
            break;
        }

        offset += (size_t) written;
        used_count++;
    }

    if (NULL != out_len)
    {
        *out_len = offset;
    }

    return used_count;
}

static void storage_refresh_metrics_snapshot(void)
{
    static app_metric_entry_t entries[APP_METRIC_LOG_SIZE];
    size_t offset = 0U;
    int count;

    memset(g_storage_metrics_text, 0, sizeof(g_storage_metrics_text));
    g_storage_metrics_text_size = 0U;

    count = app_metric_snapshot(entries, APP_METRIC_LOG_SIZE);
    if (count <= 0)
    {
        return;
    }

    for (int i = 0; i < count; i++)
    {
        int written;

        written = snprintf(&g_storage_metrics_text[offset],
                           sizeof(g_storage_metrics_text) - offset,
                           "%lu|%lu|%u|%u|%u\n",
                           (unsigned long) entries[i].unix_utc,
                           (unsigned long) entries[i].duration_ticks,
                           entries[i].success ? 1U : 0U,
                           (unsigned int) entries[i].kind,
                           (unsigned int) entries[i].case_id);
        if ((written <= 0) || ((size_t) written >= (sizeof(g_storage_metrics_text) - offset)))
        {
            break;
        }

        offset += (size_t) written;
    }

    g_storage_metrics_text_size = offset;
}

static void storage_photo_filename(const char *photo_id, char *out, size_t out_size)
{
    ULONG hash = 2166136261UL;
    const unsigned char *cursor = (const unsigned char *) photo_id;

    if ((NULL == out) || (0U == out_size))
    {
        return;
    }

    out[0] = '\0';
    if ((NULL == photo_id) || ('\0' == photo_id[0]))
    {
        return;
    }

    while ('\0' != *cursor)
    {
        hash ^= (ULONG) *cursor++;
        hash *= 16777619UL;
    }

    (void) snprintf(out, out_size, "photo_%08lX.bin", (unsigned long) hash);
}

static void storage_clear_profile(user_t *profile)
{
    if (NULL != profile)
    {
        memset(profile, 0, sizeof(*profile));
    }
}

static void storage_profile_cards_to_csv(const user_t *profile, char *out, size_t out_size)
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

static void storage_copy_admin_pin(char *out, size_t out_size, const char *src)
{
    char cleaned[STORAGE_ADMIN_PIN_MAX_LEN];
    size_t write_len = 0U;

    if ((NULL == out) || (0U == out_size))
    {
        return;
    }

    cleaned[0] = '\0';
    if (NULL == src)
    {
        out[0] = '\0';
        return;
    }

    while (('\0' != *src) && (write_len + 1U < sizeof(cleaned)))
    {
        if (isdigit((int) (unsigned char) *src))
        {
            cleaned[write_len++] = *src;
        }
        src++;
    }

    cleaned[write_len] = '\0';
    storage_copy_text(out, out_size, cleaned, write_len);
}

static void storage_extract_cards_csv(const char *cards_csv, user_t *profile)
{
    const char *cursor = cards_csv;

    if ((NULL == cards_csv) || (NULL == profile))
    {
        return;
    }

    profile->card_count = 0U;

    while (('\0' != *cursor) && (profile->card_count < STORAGE_MAX_CARDS_PER_USER))
    {
        const char *next = strchr(cursor, ',');
        size_t token_len = (NULL == next) ? strlen(cursor) : (size_t) (next - cursor);
        char token[UID_MAX_LEN];

        storage_copy_text(token, sizeof(token), cursor, token_len);
        storage_copy_clean_uid(profile->cards[profile->card_count],
                               sizeof(profile->cards[profile->card_count]),
                               token);

        if ('\0' != profile->cards[profile->card_count][0])
        {
            profile->card_count++;
        }

        cursor = (NULL == next) ? (cursor + strlen(cursor)) : (next + 1);
    }
}

static void storage_extract_legacy_uid_cards(const char *legacy_uid, user_t *profile)
{
    if ((NULL == legacy_uid) || (NULL == profile) || ('\0' == legacy_uid[0]))
    {
        return;
    }

    storage_extract_cards_csv(legacy_uid, profile);
}

static bool storage_append_jsonf(char *json_buffer, size_t json_buffer_size, size_t *offset, const char *format, ...)
{
    va_list args;
    int written;

    if ((NULL == json_buffer) || (NULL == offset) || (NULL == format) || (*offset >= json_buffer_size))
    {
        return false;
    }

    va_start(args, format);
    written = vsnprintf(&json_buffer[*offset], json_buffer_size - *offset, format, args);
    va_end(args);

    if ((written < 0) || ((size_t) written >= (json_buffer_size - *offset)))
    {
        return false;
    }

    *offset += (size_t) written;
    return true;
}

static bool storage_append_user_json_locked(const user_t *profile,
                                            bool prepend_comma,
                                            char *json_buffer,
                                            size_t json_buffer_size,
                                            size_t *offset)
{
    char cards_csv[UID_MAX_LEN * STORAGE_MAX_CARDS_PER_USER];

    if ((NULL == profile) || (NULL == json_buffer) || (NULL == offset))
    {
        return false;
    }

    if (!storage_append_jsonf(json_buffer,
                              json_buffer_size,
                              offset,
                              "%s{\"name\":\"%s\"",
                              prepend_comma ? "," : "",
                              profile->name))
    {
        return false;
    }

    if (('\0' != profile->role[0]) &&
        !storage_append_jsonf(json_buffer, json_buffer_size, offset, ",\"role\":\"%s\"", profile->role))
    {
        return false;
    }

    if (('\0' != profile->chapter[0]) &&
        !storage_append_jsonf(json_buffer, json_buffer_size, offset, ",\"chapter\":\"%s\"", profile->chapter))
    {
        return false;
    }

    if (('\0' != profile->photo_id[0]) &&
        !storage_append_jsonf(json_buffer, json_buffer_size, offset, ",\"photo_id\":\"%s\"", profile->photo_id))
    {
        return false;
    }

    if (profile->is_admin &&
        !storage_append_jsonf(json_buffer, json_buffer_size, offset, ",\"is_admin\":true"))
    {
        return false;
    }

    if (('\0' != profile->admin_pin[0]) &&
        !storage_append_jsonf(json_buffer, json_buffer_size, offset, ",\"admin_pin\":\"%s\"", profile->admin_pin))
    {
        return false;
    }

    if (profile->card_count > 0U)
    {
        storage_profile_cards_to_csv(profile, cards_csv, sizeof(cards_csv));
        if (!storage_append_jsonf(json_buffer,
                                  json_buffer_size,
                                  offset,
                                  ",\"uid\":\"%s\",\"cards_csv\":\"%s\",\"cards\":[",
                                  profile->cards[0],
                                  cards_csv))
        {
            return false;
        }

        for (unsigned int card_index = 0U; card_index < profile->card_count; card_index++)
        {
            if (!storage_append_jsonf(json_buffer,
                                      json_buffer_size,
                                      offset,
                                      "%s\"%s\"",
                                      (card_index > 0U) ? "," : "",
                                      profile->cards[card_index]))
            {
                return false;
            }
        }

        if (!storage_append_jsonf(json_buffer, json_buffer_size, offset, "]"))
        {
            return false;
        }
    }

    return storage_append_jsonf(json_buffer, json_buffer_size, offset, "}");
}

static bool storage_profile_has_cards(const user_t *profile)
{
    return (NULL != profile) && (profile->card_count > 0U) && ('\0' != profile->cards[0][0]);
}

static void storage_set_recent_user_locked(const user_t *profile)
{
    if ((NULL == profile) || !storage_profile_has_cards(profile))
    {
        g_recent_user_valid = false;
        memset(&g_recent_user, 0, sizeof(g_recent_user));
        return;
    }

    g_recent_user = *profile;
    g_recent_user_valid = true;
}

static int storage_find_user_by_card_locked(const char *uid_str)
{
    char cleaned_uid[UID_MAX_LEN];

    if ((NULL == uid_str) || ('\0' == uid_str[0]))
    {
        return -1;
    }

    storage_copy_clean_uid(cleaned_uid, sizeof(cleaned_uid), uid_str);
    if ('\0' == cleaned_uid[0])
    {
        return -1;
    }

    for (int user_index = 0; user_index < g_user_count; user_index++)
    {
        for (unsigned int card_index = 0U; card_index < g_users[user_index].card_count; card_index++)
        {
            if (0 == strcmp(g_users[user_index].cards[card_index], cleaned_uid))
            {
                return user_index;
            }
        }
    }

    return -1;
}

static const user_t *storage_find_profile_by_uid_locked(const char *uid_str)
{
    int index;

    index = storage_find_user_by_card_locked(uid_str);
    if (index >= 0)
    {
        return &g_users[index];
    }

    if (g_recent_user_valid)
    {
        char cleaned_uid[UID_MAX_LEN];

        storage_copy_clean_uid(cleaned_uid, sizeof(cleaned_uid), uid_str);
        if ('\0' != cleaned_uid[0])
        {
            for (unsigned int card_index = 0U; card_index < g_recent_user.card_count; card_index++)
            {
                if (0 == strcmp(g_recent_user.cards[card_index], cleaned_uid))
                {
                    return &g_recent_user;
                }
            }
        }
    }

    return NULL;
}

static void storage_meeting_mode_clear_locked(void)
{
    memset(g_storage_meeting_mode_allowed_cards, 0, sizeof(g_storage_meeting_mode_allowed_cards));
    g_storage_meeting_mode_active = false;
    g_storage_meeting_mode_selected_profiles = 0U;
    g_storage_meeting_mode_allowed_count = 0U;
}

static bool storage_meeting_mode_uid_allowed_locked(const char *uid_str)
{
    char cleaned_uid[UID_MAX_LEN];

    if ((NULL == uid_str) || ('\0' == uid_str[0]))
    {
        return false;
    }

    storage_copy_clean_uid(cleaned_uid, sizeof(cleaned_uid), uid_str);
    if ('\0' == cleaned_uid[0])
    {
        return false;
    }

    for (unsigned int i = 0U; i < g_storage_meeting_mode_allowed_count; i++)
    {
        if (0 == strcmp(g_storage_meeting_mode_allowed_cards[i], cleaned_uid))
        {
            return true;
        }
    }

    return false;
}

static bool storage_meeting_mode_profile_selected_locked(const user_t *profile)
{
    if (!g_storage_meeting_mode_active || (NULL == profile))
    {
        return false;
    }

    for (unsigned int card_index = 0U; card_index < profile->card_count; card_index++)
    {
        if (storage_meeting_mode_uid_allowed_locked(profile->cards[card_index]))
        {
            return true;
        }
    }

    return false;
}

static const char *storage_find_within_object(const char *start, const char *end, const char *pattern)
{
    size_t pattern_len;
    const char *cursor;

    if ((NULL == start) || (NULL == end) || (NULL == pattern) || (end < start))
    {
        return NULL;
    }

    pattern_len = strlen(pattern);
    if (0U == pattern_len)
    {
        return NULL;
    }

    for (cursor = start; (cursor + pattern_len) <= end; cursor++)
    {
        if (0 == memcmp(cursor, pattern, pattern_len))
        {
            return cursor;
        }
    }

    return NULL;
}

static const char *storage_skip_whitespace(const char *cursor, const char *limit)
{
    while ((NULL != cursor) && (cursor < limit) &&
           ((' ' == *cursor) || ('\t' == *cursor) || ('\r' == *cursor) || ('\n' == *cursor)))
    {
        cursor++;
    }

    return cursor;
}

static const char *storage_find_json_key(const char *object_start, const char *object_end, const char *key)
{
    char pattern[48];
    const char *found;

    if ((NULL == object_start) || (NULL == object_end) || (NULL == key))
    {
        return NULL;
    }

    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    found = storage_find_within_object(object_start, object_end, pattern);
    if (NULL == found)
    {
        return NULL;
    }

    return found + strlen(pattern);
}

static bool storage_extract_json_string(const char *object_start,
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

    found = storage_find_json_key(object_start, object_end, key);
    if (NULL == found)
    {
        out[0] = '\0';
        return false;
    }

    colon = storage_skip_whitespace(found, object_end);
    if ((NULL == colon) || (colon >= object_end) || (':' != *colon))
    {
        out[0] = '\0';
        return false;
    }

    start = storage_skip_whitespace(colon + 1, object_end);
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

    storage_copy_text(out, out_size, start, (size_t) (end - start));
    return true;
}

static bool storage_extract_json_bool(const char *object_start,
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

    found = storage_find_json_key(object_start, object_end, key);
    if (NULL == found)
    {
        return default_value;
    }

    colon = storage_skip_whitespace(found, object_end);
    if ((NULL == colon) || (colon >= object_end) || (':' != *colon))
    {
        return default_value;
    }

    cursor = storage_skip_whitespace(colon + 1, object_end);
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

static void storage_extract_json_cards(const char *object_start, const char *object_end, user_t *profile)
{
    const char *found;
    const char *colon;
    const char *cursor;
    unsigned int count = 0U;

    if ((NULL == object_start) || (NULL == object_end) || (NULL == profile))
    {
        return;
    }

    found = storage_find_json_key(object_start, object_end, "cards");
    if (NULL == found)
    {
        char legacy_uid[UID_MAX_LEN];

        if (storage_extract_json_string(object_start, object_end, "uid", legacy_uid, sizeof(legacy_uid)))
        {
            storage_extract_legacy_uid_cards(legacy_uid, profile);
        }
        return;
    }

    colon = storage_skip_whitespace(found, object_end);
    if ((NULL == colon) || (colon >= object_end) || (':' != *colon))
    {
        return;
    }

    cursor = storage_skip_whitespace(colon + 1, object_end);
    if ((NULL == cursor) || (cursor >= object_end) || ('[' != *cursor))
    {
        return;
    }

    cursor++;
    while (('\0' != *cursor) && (']' != *cursor) && (count < STORAGE_MAX_CARDS_PER_USER))
    {
        const char *start = strchr(cursor, '"');
        const char *end;

        if ((NULL == start) || (start > object_end) || (']' == *start))
        {
            break;
        }

        end = strchr(start + 1, '"');
        if ((NULL == end) || (end > object_end))
        {
            break;
        }

        storage_copy_text(profile->cards[count], sizeof(profile->cards[count]), start + 1, (size_t) (end - (start + 1)));
        storage_copy_clean_uid(profile->cards[count], sizeof(profile->cards[count]), profile->cards[count]);
        if ('\0' != profile->cards[count][0])
        {
            count++;
        }

        cursor = end + 1;
    }

    profile->card_count = count;
}

static bool storage_parse_users(const char *json_buffer)
{
    const char *cursor = json_buffer;

    g_user_count = 0;
    g_recent_user_valid = false;
    memset(g_users, 0, sizeof(g_users));

    if (NULL == json_buffer)
    {
        return false;
    }

    while ((NULL != cursor) && ('\0' != *cursor) && (g_user_count < STORAGE_MAX_USERS))
    {
        const char *name_key = strstr(cursor, "\"name\"");
        const char *object_start;
        const char *object_end;
        user_t profile;
        char legacy_uid[UID_MAX_LEN];
        char cards_csv[UID_MAX_LEN * STORAGE_MAX_CARDS_PER_USER];

        if (NULL == name_key)
        {
            break;
        }

        object_start = name_key;
        while ((object_start > json_buffer) && ('{' != *object_start))
        {
            object_start--;
        }
        if ('{' != *object_start)
        {
            cursor = name_key + 6;
            continue;
        }

        object_end = strchr(object_start, '}');
        if (NULL == object_end)
        {
            break;
        }

        storage_clear_profile(&profile);
        memset(legacy_uid, 0, sizeof(legacy_uid));
        memset(cards_csv, 0, sizeof(cards_csv));

        if (!storage_extract_json_string(object_start, object_end, "name", profile.name, sizeof(profile.name)))
        {
            cursor = object_end + 1;
            continue;
        }

        (void) storage_extract_json_string(object_start, object_end, "uid", legacy_uid, sizeof(legacy_uid));
        (void) storage_extract_json_string(object_start, object_end, "role", profile.role, sizeof(profile.role));
        (void) storage_extract_json_string(object_start, object_end, "chapter", profile.chapter, sizeof(profile.chapter));
        (void) storage_extract_json_string(object_start, object_end, "photo_id", profile.photo_id, sizeof(profile.photo_id));
        profile.is_admin = storage_extract_json_bool(object_start, object_end, "is_admin", false);
        (void) storage_extract_json_string(object_start, object_end, "admin_pin", profile.admin_pin, sizeof(profile.admin_pin));
        storage_copy_admin_pin(profile.admin_pin, sizeof(profile.admin_pin), profile.admin_pin);
        (void) storage_extract_json_string(object_start, object_end, "cards_csv", cards_csv, sizeof(cards_csv));
        if ('\0' != cards_csv[0])
        {
            storage_extract_cards_csv(cards_csv, &profile);
        }
        else
        {
            storage_extract_json_cards(object_start, object_end, &profile);
        }

        if (!storage_profile_has_cards(&profile) && ('\0' != legacy_uid[0]))
        {
            storage_extract_legacy_uid_cards(legacy_uid, &profile);
        }

        if ('\0' != profile.name[0])
        {
            g_users[g_user_count++] = profile;
            if (storage_profile_has_cards(&profile))
            {
                storage_set_recent_user_locked(&profile);
            }
        }

        cursor = object_end + 1;
    }

    return true;
}

static bool storage_prepare_users_json_locked(char *json_buffer, size_t json_buffer_size, size_t *json_size)
{
    size_t offset = 0U;
    int count_to_write = g_user_count;

    if ((NULL == json_buffer) || (NULL == json_size) || (0U == json_buffer_size))
    {
        return false;
    }

    json_buffer[offset++] = '[';

    if ((0 == count_to_write) && g_recent_user_valid)
    {
        if (!storage_append_user_json_locked(&g_recent_user, false, json_buffer, json_buffer_size, &offset))
        {
            return false;
        }
    }

    for (int i = 0; i < count_to_write; i++)
    {
        if (!storage_append_user_json_locked(&g_users[i],
                                             (offset > 1U),
                                             json_buffer,
                                             json_buffer_size,
                                             &offset))
        {
            return false;
        }
    }

    if (offset >= (json_buffer_size - 2U))
    {
        return false;
    }

    json_buffer[offset++] = ']';
    json_buffer[offset] = '\0';
    *json_size = offset;
    return true;
}

static void storage_refresh_persist_snapshot_locked(void)
{
    memset(g_storage_persist_json, 0, sizeof(g_storage_persist_json));
    g_storage_persist_json_size = 0U;
    storage_prepare_users_json_locked(g_storage_persist_json, sizeof(g_storage_persist_json), &g_storage_persist_json_size);
}

static bool storage_save_access_log_buffer(const char *log_buffer, size_t log_size)
{
    UINT status;
    ULONG current_size = 0U;
    bool file_opened = false;
    ULONG start_tick = tx_time_get();

    if ((NULL == log_buffer) || (0U == log_size))
    {
        return false;
    }

    storage_media_lock();
    if (!storage_media_open())
    {
        storage_media_unlock();
        return false;
    }

    status = fx_file_open(&g_fx_media0, &g_access_log_file, ACCESS_LOG_FILE_NAME, FX_OPEN_FOR_WRITE);
    if (FX_SUCCESS == status)
    {
        file_opened = true;
        current_size = (ULONG) g_access_log_file.fx_file_current_file_size;
    }

    if (file_opened && ((current_size + (ULONG) log_size) > ACCESS_LOG_ROTATE_SIZE))
    {
        (void) fx_file_close(&g_access_log_file);
        file_opened = false;
        current_size = 0U;

        (void) fx_file_delete(&g_fx_media0, ACCESS_LOG_ARCHIVE_FILE_NAME);
        status = fx_file_rename(&g_fx_media0, ACCESS_LOG_FILE_NAME, ACCESS_LOG_ARCHIVE_FILE_NAME);
        if (FX_SUCCESS != status)
        {
            (void) fx_media_close(&g_fx_media0);
            g_storage_media_ready = false;
            storage_media_unlock();
            return false;
        }
    }

    if (!file_opened)
    {
        status = fx_file_open(&g_fx_media0, &g_access_log_file, ACCESS_LOG_FILE_NAME, FX_OPEN_FOR_WRITE);
        if (FX_SUCCESS != status)
        {
            status = fx_file_create(&g_fx_media0, ACCESS_LOG_FILE_NAME);
            if ((FX_SUCCESS != status) && (FX_ALREADY_CREATED != status))
            {
                (void) fx_media_close(&g_fx_media0);
                g_storage_media_ready = false;
                storage_media_unlock();
                return false;
            }

            status = fx_file_open(&g_fx_media0, &g_access_log_file, ACCESS_LOG_FILE_NAME, FX_OPEN_FOR_WRITE);
        }
        if (FX_SUCCESS != status)
        {
            (void) fx_media_close(&g_fx_media0);
            g_storage_media_ready = false;
            storage_media_unlock();
            return false;
        }

        file_opened = true;
        current_size = (ULONG) g_access_log_file.fx_file_current_file_size;
    }

    status = fx_file_seek(&g_access_log_file, current_size);
    if (FX_SUCCESS == status)
    {
        status = fx_file_write(&g_access_log_file, (VOID *) log_buffer, log_size);
    }
    if (FX_SUCCESS == status)
    {
        status = fx_file_close(&g_access_log_file);
    }
    else
    {
        fx_file_close(&g_access_log_file);
        (void) fx_media_close(&g_fx_media0);
        g_storage_media_ready = false;
        storage_media_unlock();
        return false;
    }

    if (FX_SUCCESS == status)
    {
        status = fx_media_flush(&g_fx_media0);
    }

    if (FX_SUCCESS == status)
    {
        status = fx_media_close(&g_fx_media0);
        if (FX_SUCCESS == status)
        {
            g_storage_media_ready = false;
        }
    }
    else
    {
        (void) fx_media_close(&g_fx_media0);
        g_storage_media_ready = false;
    }

    storage_media_unlock();
    app_metric_add_no_persist("Storage persist latency", "access.log", tx_time_get() - start_tick, (FX_SUCCESS == status));
    return (FX_SUCCESS == status);
}

static bool storage_save_metrics_buffer(const char *log_buffer, size_t log_size)
{
    UINT status;
    ULONG start_tick = tx_time_get();

    if ((NULL == log_buffer) || (0U == log_size))
    {
        return false;
    }

    storage_media_lock();
    if (!storage_media_open())
    {
        storage_media_unlock();
        return false;
    }

    status = fx_file_open(&g_fx_media0, &g_metrics_file, "metrics.log", FX_OPEN_FOR_WRITE);
    if (FX_SUCCESS != status)
    {
        status = fx_file_create(&g_fx_media0, "metrics.log");
        if ((FX_SUCCESS != status) && (FX_ALREADY_CREATED != status))
        {
            (void) fx_media_close(&g_fx_media0);
            g_storage_media_ready = false;
            storage_media_unlock();
            app_metric_add_no_persist("Storage persist latency", "metrics.log", tx_time_get() - start_tick, false);
            return false;
        }

        status = fx_file_open(&g_fx_media0, &g_metrics_file, "metrics.log", FX_OPEN_FOR_WRITE);
        if (FX_SUCCESS != status)
        {
            (void) fx_media_close(&g_fx_media0);
            g_storage_media_ready = false;
            storage_media_unlock();
            app_metric_add_no_persist("Storage persist latency", "metrics.log", tx_time_get() - start_tick, false);
            return false;
        }
    }

    status = fx_file_truncate(&g_metrics_file, 0U);
    if (FX_SUCCESS == status)
    {
        status = fx_file_seek(&g_metrics_file, 0U);
    }
    if (FX_SUCCESS == status)
    {
        status = fx_file_write(&g_metrics_file, (VOID *) log_buffer, log_size);
    }
    if (FX_SUCCESS == status)
    {
        status = fx_file_close(&g_metrics_file);
    }
    else
    {
        fx_file_close(&g_metrics_file);
        (void) fx_media_close(&g_fx_media0);
        g_storage_media_ready = false;
        storage_media_unlock();
        app_metric_add_no_persist("Storage persist latency", "metrics.log", tx_time_get() - start_tick, false);
        return false;
    }

    if (FX_SUCCESS == status)
    {
        status = fx_media_flush(&g_fx_media0);
    }

    if (FX_SUCCESS == status)
    {
        status = fx_media_close(&g_fx_media0);
        if (FX_SUCCESS == status)
        {
            g_storage_media_ready = false;
        }
    }
    else
    {
        (void) fx_media_close(&g_fx_media0);
        g_storage_media_ready = false;
    }

    storage_media_unlock();
    app_metric_add_no_persist("Storage persist latency", "metrics.log", tx_time_get() - start_tick, (FX_SUCCESS == status));
    return (FX_SUCCESS == status);
}

static bool storage_parse_access_log_text(const char *log_buffer)
{
    static app_access_log_entry_t entries[ACCESS_LOG_SIZE];
    int entry_count = 0;
    const char *cursor = log_buffer;

    if (NULL == log_buffer)
    {
        return false;
    }

    memset(entries, 0, sizeof(entries));

    while (('\0' != *cursor) && (entry_count < ACCESS_LOG_SIZE))
    {
        const char *line_end = strchr(cursor, '\n');
        char line[128];
        char *first_sep;
        char *second_sep;
        char *third_sep;
        char *end_ptr = NULL;
        unsigned long unix_utc;
        long type_value;

        if (NULL == line_end)
        {
            line_end = cursor + strlen(cursor);
        }

        storage_copy_text(line, sizeof(line), cursor, (size_t) (line_end - cursor));
        first_sep = strchr(line, '|');
        second_sep = (NULL != first_sep) ? strchr(first_sep + 1, '|') : NULL;
        third_sep = (NULL != second_sep) ? strchr(second_sep + 1, '|') : NULL;

        if ((NULL != first_sep) && (NULL != second_sep) && (NULL != third_sep))
        {
            *first_sep = '\0';
            *second_sep = '\0';
            *third_sep = '\0';

            unix_utc = strtoul(line, &end_ptr, 10);
            if ((NULL != end_ptr) && ('\0' == *end_ptr))
            {
                type_value = strtol(first_sep + 1, &end_ptr, 10);
                if ((NULL != end_ptr) &&
                    ('\0' == *end_ptr) &&
                    (type_value >= (long) EVENT_RFID_AUTH_OK) &&
                    (type_value <= (long) EVENT_CARD_REGISTRATION_FAILED))
                {
                    entries[entry_count].tick = 0U;
                    entries[entry_count].unix_utc = (ULONG) unix_utc;
                    entries[entry_count].type = (app_event_type_t) type_value;
                    storage_copy_text(entries[entry_count].data,
                                      sizeof(entries[entry_count].data),
                                      second_sep + 1,
                                      strlen(second_sep + 1));
                    storage_copy_text(entries[entry_count].user,
                                      sizeof(entries[entry_count].user),
                                      third_sep + 1,
                                      strlen(third_sep + 1));
                    entry_count++;
                }
            }
        }

        cursor = ('\0' != *line_end) ? (line_end + 1) : line_end;
    }

    if (entry_count > 0)
    {
        app_access_log_restore(entries, entry_count);
    }

    return true;
}

static bool storage_load_access_log_tail_file(const char *filename, char *log_buffer, size_t log_buffer_size)
{
    UINT status;
    ULONG actual_bytes = 0U;
    ULONG read_offset = 0U;
    const char *parse_cursor;

    if ((NULL == filename) || (NULL == log_buffer) || (log_buffer_size < 2U))
    {
        return false;
    }

    status = fx_file_open(&g_fx_media0, &g_access_log_file, (CHAR *) filename, FX_OPEN_FOR_READ);
    if (FX_SUCCESS != status)
    {
        return false;
    }

    if (g_access_log_file.fx_file_current_file_size > (ULONG) (log_buffer_size - 1U))
    {
        read_offset = (ULONG) (g_access_log_file.fx_file_current_file_size - (ULONG) (log_buffer_size - 1U));
    }

    status = fx_file_seek(&g_access_log_file, read_offset);
    if (FX_SUCCESS == status)
    {
        status = fx_file_read(&g_access_log_file,
                              log_buffer,
                              (ULONG) (log_buffer_size - 1U),
                              &actual_bytes);
    }

    (void) fx_file_close(&g_access_log_file);
    if ((FX_SUCCESS != status) || (0U == actual_bytes))
    {
        log_buffer[0] = '\0';
        return false;
    }

    log_buffer[actual_bytes] = '\0';
    parse_cursor = log_buffer;

    if (read_offset > 0U)
    {
        const char *first_newline = strchr(log_buffer, '\n');
        if (NULL == first_newline)
        {
            return false;
        }
        parse_cursor = first_newline + 1;
    }

    if ('\0' == *parse_cursor)
    {
        return false;
    }

    return storage_parse_access_log_text(parse_cursor);
}

static bool storage_parse_metrics_text(const char *log_buffer)
{
    static app_metric_entry_t entries[APP_METRIC_LOG_SIZE];
    int entry_count = 0;
    const char *cursor = log_buffer;

    if (NULL == log_buffer)
    {
        return false;
    }

    memset(entries, 0, sizeof(entries));

    while (('\0' != *cursor) && (entry_count < APP_METRIC_LOG_SIZE))
    {
        const char *line_end = strchr(cursor, '\n');
        char line[160];
        char *first_sep;
        char *second_sep;
        char *third_sep;
        char *fourth_sep;
        char *end_ptr = NULL;
        unsigned long unix_utc;
        unsigned long duration_ticks;
        unsigned long success_value;
        unsigned long kind_value;
        unsigned long case_value;

        if (NULL == line_end)
        {
            line_end = cursor + strlen(cursor);
        }

        storage_copy_text(line, sizeof(line), cursor, (size_t) (line_end - cursor));
        first_sep = strchr(line, '|');
        second_sep = (NULL != first_sep) ? strchr(first_sep + 1, '|') : NULL;
        third_sep = (NULL != second_sep) ? strchr(second_sep + 1, '|') : NULL;
        fourth_sep = (NULL != third_sep) ? strchr(third_sep + 1, '|') : NULL;

        if ((NULL != first_sep) && (NULL != second_sep) && (NULL != third_sep) && (NULL != fourth_sep))
        {
            *first_sep = '\0';
            *second_sep = '\0';
            *third_sep = '\0';
            *fourth_sep = '\0';

            unix_utc = strtoul(line, &end_ptr, 10);
            if ((NULL != end_ptr) && ('\0' == *end_ptr))
            {
                duration_ticks = strtoul(first_sep + 1, &end_ptr, 10);
                if ((NULL != end_ptr) && ('\0' == *end_ptr))
                {
                    success_value = strtoul(second_sep + 1, &end_ptr, 10);
                    if ((NULL != end_ptr) && ('\0' == *end_ptr))
                    {
                        kind_value = strtoul(third_sep + 1, &end_ptr, 10);
                        if ((NULL == end_ptr) || ('\0' != *end_ptr))
                        {
                            cursor = ('\0' != *line_end) ? (line_end + 1) : line_end;
                            continue;
                        }
                        case_value = strtoul(fourth_sep + 1, &end_ptr, 10);
                        if ((NULL == end_ptr) || ('\0' != *end_ptr))
                        {
                            cursor = ('\0' != *line_end) ? (line_end + 1) : line_end;
                            continue;
                        }
                        entries[entry_count].tick = 0U;
                        entries[entry_count].unix_utc = (ULONG) unix_utc;
                        entries[entry_count].duration_ticks = (ULONG) duration_ticks;
                        entries[entry_count].success = (0U != success_value);
                        entries[entry_count].kind = (app_metric_kind_t) kind_value;
                        entries[entry_count].case_id = (app_metric_case_t) case_value;
                        entry_count++;
                    }
                }
            }
        }

        cursor = ('\0' != *line_end) ? (line_end + 1) : line_end;
    }

    if (entry_count > 0)
    {
        app_metric_restore(entries, entry_count);
    }

    return true;
}

/* =========================================================================
   GRAVACAO E LEITURA USANDO O FILEX
   ========================================================================= */
static bool storage_save_json_buffer(const char *json_buffer, size_t json_size)
{
    UINT status;
    ULONG start_tick = tx_time_get();

    if ((NULL == json_buffer) || (0U == json_size))
    {
        return false;
    }

    g_storage_debug_last_stage = 10U;
    g_storage_debug_last_saved_bytes = 0U;
    storage_media_lock();
    if (!storage_media_open())
    {
        storage_media_unlock();
        return false;
    }

    status = fx_file_open(&g_fx_media0, &g_users_file, "users.json", FX_OPEN_FOR_WRITE);
    if (FX_SUCCESS != status)
    {
        status = fx_file_create(&g_fx_media0, "users.json");
        if ((FX_SUCCESS != status) && (FX_ALREADY_CREATED != status))
        {
            (void) fx_media_close(&g_fx_media0);
            g_storage_media_ready = false;
            storage_media_unlock();
            return false;
        }

        status = fx_file_open(&g_fx_media0, &g_users_file, "users.json", FX_OPEN_FOR_WRITE);
        if (FX_SUCCESS != status)
        {
            (void) fx_media_close(&g_fx_media0);
            g_storage_media_ready = false;
            storage_media_unlock();
            return false;
        }
    }

    status = fx_file_truncate(&g_users_file, 0U);
    if (FX_SUCCESS == status)
    {
        status = fx_file_seek(&g_users_file, 0U);
    }
    if (FX_SUCCESS == status)
    {
        status = fx_file_write(&g_users_file, (VOID *) json_buffer, json_size);
        if (FX_SUCCESS == status)
        {
            g_storage_debug_last_saved_bytes = (ULONG) json_size;
        }
    }
    if (FX_SUCCESS == status)
    {
        status = fx_file_close(&g_users_file);
    }
    else
    {
        fx_file_close(&g_users_file);
        storage_media_unlock();
        return false;
    }

    if (FX_SUCCESS == status)
    {
        status = fx_media_flush(&g_fx_media0);
    }

    if (FX_SUCCESS == status)
    {
        status = fx_media_close(&g_fx_media0);
        if (FX_SUCCESS == status)
        {
            g_storage_media_ready = false;
        }
    }
    else
    {
        (void) fx_media_close(&g_fx_media0);
        g_storage_media_ready = false;
    }

    g_storage_debug_last_save_status = status;
    g_storage_debug_last_stage = 15U;
    storage_media_unlock();
    app_metric_add_no_persist("Storage persist latency", "users.json", tx_time_get() - start_tick, (FX_SUCCESS == status));
    return (FX_SUCCESS == status);
}

static bool storage_save_photo_file(const char *photo_id)
{
    static uint16_t row_buffer[STORAGE_RUNTIME_PHOTO_MAX_DIM * STORAGE_PHOTO_COPY_ROWS];
    storage_photo_file_header_t header;
    char filename[32];
    uint16_t width = 0U;
    uint16_t height = 0U;
    UINT status;
    ULONG start_tick = tx_time_get();

    if ((NULL == photo_id) || ('\0' == photo_id[0]))
    {
        return false;
    }

    if (!ui_get_uploaded_photo_info(photo_id, &width, &height))
    {
        return false;
    }

    if ((0U == width) || (0U == height) ||
        (width > STORAGE_RUNTIME_PHOTO_MAX_DIM) ||
        (height > STORAGE_RUNTIME_PHOTO_MAX_DIM))
    {
        return false;
    }

    storage_photo_filename(photo_id, filename, sizeof(filename));
    header.magic = STORAGE_PHOTO_FILE_MAGIC;
    header.width = width;
    header.height = height;
    header.payload_size = (ULONG) width * (ULONG) height * 2UL;

    g_storage_debug_last_stage = 30U;
    storage_media_lock();
    if (!storage_media_open())
    {
        storage_media_unlock();
        return false;
    }

    status = fx_file_open(&g_fx_media0, &g_photo_file, filename, FX_OPEN_FOR_WRITE);
    if (FX_SUCCESS != status)
    {
        status = fx_file_create(&g_fx_media0, filename);
        if ((FX_SUCCESS != status) && (FX_ALREADY_CREATED != status))
        {
            (void) fx_media_close(&g_fx_media0);
            g_storage_media_ready = false;
            storage_media_unlock();
            return false;
        }

        status = fx_file_open(&g_fx_media0, &g_photo_file, filename, FX_OPEN_FOR_WRITE);
        if (FX_SUCCESS != status)
        {
            (void) fx_media_close(&g_fx_media0);
            g_storage_media_ready = false;
            storage_media_unlock();
            return false;
        }
    }

    status = fx_file_truncate(&g_photo_file, 0U);
    if (FX_SUCCESS == status)
    {
        status = fx_file_seek(&g_photo_file, 0U);
    }
    if (FX_SUCCESS == status)
    {
        status = fx_file_write(&g_photo_file, &header, sizeof(header));
    }

    for (uint16_t row = 0U; (FX_SUCCESS == status) && (row < height); row = (uint16_t) (row + STORAGE_PHOTO_COPY_ROWS))
    {
        uint16_t rows_to_copy = (uint16_t) (((uint32_t) row + STORAGE_PHOTO_COPY_ROWS <= (uint32_t) height)
                                            ? STORAGE_PHOTO_COPY_ROWS
                                            : (uint32_t) (height - row));
        ULONG bytes_to_write = (ULONG) width * (ULONG) rows_to_copy * 2UL;

        if (!ui_copy_uploaded_photo_rows_rgb565(photo_id,
                                                row,
                                                rows_to_copy,
                                                row_buffer,
                                                sizeof(row_buffer) / sizeof(row_buffer[0])))
        {
            status = FX_INVALID_NAME;
            break;
        }

        status = fx_file_write(&g_photo_file, row_buffer, bytes_to_write);
    }

    if (FX_SUCCESS == status)
    {
        status = fx_file_close(&g_photo_file);
    }
    else
    {
        fx_file_close(&g_photo_file);
        (void) fx_media_close(&g_fx_media0);
        g_storage_media_ready = false;
        storage_media_unlock();
        return false;
    }

    if (FX_SUCCESS == status)
    {
        status = fx_media_flush(&g_fx_media0);
    }

    if (FX_SUCCESS == status)
    {
        status = fx_media_close(&g_fx_media0);
        if (FX_SUCCESS == status)
        {
            g_storage_media_ready = false;
        }
    }
    else
    {
        (void) fx_media_close(&g_fx_media0);
        g_storage_media_ready = false;
    }

    g_storage_debug_last_stage = (FX_SUCCESS == status) ? 35U : 34U;
    storage_media_unlock();
    app_metric_add_no_persist("Storage persist latency", "photo", tx_time_get() - start_tick, (FX_SUCCESS == status));
    return (FX_SUCCESS == status);
}

static bool storage_load_photo_file(const char *photo_id)
{
    static uint16_t row_buffer[STORAGE_RUNTIME_PHOTO_MAX_DIM * STORAGE_PHOTO_COPY_ROWS];
    storage_photo_file_header_t header;
    char filename[32];
    ULONG actual_bytes = 0U;
    UINT status;

    if ((NULL == photo_id) || ('\0' == photo_id[0]))
    {
        return false;
    }

    if (!storage_media_access_allowed())
    {
        return false;
    }

    storage_photo_filename(photo_id, filename, sizeof(filename));
    g_storage_debug_last_stage = 40U;

    storage_media_lock();
    if (!storage_media_open())
    {
        storage_media_unlock();
        return false;
    }

    status = fx_file_open(&g_fx_media0, &g_photo_file, filename, FX_OPEN_FOR_READ);
    if (FX_SUCCESS != status)
    {
        (void) fx_media_close(&g_fx_media0);
        g_storage_media_ready = false;
        storage_media_unlock();
        return false;
    }

    status = fx_file_read(&g_photo_file, &header, sizeof(header), &actual_bytes);
    if ((FX_SUCCESS != status) ||
        (actual_bytes != sizeof(header)) ||
        (header.magic != STORAGE_PHOTO_FILE_MAGIC) ||
        (0U == header.width) ||
        (0U == header.height) ||
        (header.width > STORAGE_RUNTIME_PHOTO_MAX_DIM) ||
        (header.height > STORAGE_RUNTIME_PHOTO_MAX_DIM) ||
        (header.payload_size != ((ULONG) header.width * (ULONG) header.height * 2UL)) ||
        !ui_prepare_uploaded_photo_rgb565(photo_id, header.width, header.height))
    {
        fx_file_close(&g_photo_file);
        (void) fx_media_close(&g_fx_media0);
        g_storage_media_ready = false;
        storage_media_unlock();
        return false;
    }

    for (uint16_t row = 0U; (FX_SUCCESS == status) && (row < header.height); row = (uint16_t) (row + STORAGE_PHOTO_COPY_ROWS))
    {
        uint16_t rows_to_read = (uint16_t) (((uint32_t) row + STORAGE_PHOTO_COPY_ROWS <= (uint32_t) header.height)
                                            ? STORAGE_PHOTO_COPY_ROWS
                                            : (uint32_t) (header.height - row));
        ULONG bytes_to_read = (ULONG) header.width * (ULONG) rows_to_read * 2UL;

        status = fx_file_read(&g_photo_file, row_buffer, bytes_to_read, &actual_bytes);
        if ((FX_SUCCESS != status) || (actual_bytes != bytes_to_read))
        {
            break;
        }

        if (!ui_write_uploaded_photo_tile_rgb565(photo_id,
                                                 row_buffer,
                                                 0U,
                                                 row,
                                                 header.width,
                                                 rows_to_read))
        {
            status = FX_INVALID_NAME;
            break;
        }
    }

    fx_file_close(&g_photo_file);
    if (FX_SUCCESS == fx_media_close(&g_fx_media0))
    {
        g_storage_media_ready = false;
    }
    storage_media_unlock();

    g_storage_debug_last_stage = (FX_SUCCESS == status) ? 45U : 44U;
    return (FX_SUCCESS == status);
}

static void storage_load_users_locked(void)
{
    UINT status;
    ULONG actual_bytes;

    g_storage_debug_last_stage = 20U;
    g_storage_debug_last_read_bytes = 0U;

    if (g_storage_loaded)
    {
        g_storage_debug_last_stage = 21U;
        return;
    }

    if (!storage_media_access_allowed())
    {
        g_storage_debug_last_stage = 22U;
        return;
    }

    storage_media_lock();
    if (!storage_media_open())
    {
        storage_media_unlock();
        g_storage_debug_last_stage = 23U;
        return;
    }

    status = fx_file_open(&g_fx_media0, &g_users_file, "users.json", FX_OPEN_FOR_READ);
    g_storage_debug_last_load_status = status;
    if (FX_SUCCESS == status)
    {
        status = fx_file_read(&g_users_file, g_storage_load_json, JSON_BUFFER_SIZE - 1U, &actual_bytes);
        if (FX_SUCCESS == status)
        {
            g_storage_load_json[actual_bytes] = '\0';
            g_storage_debug_last_read_bytes = actual_bytes;
            (void) storage_parse_users(g_storage_load_json);
        }
        fx_file_close(&g_users_file);
        g_storage_debug_last_load_status = status;
        status = fx_media_close(&g_fx_media0);
        if (FX_SUCCESS == status)
        {
            g_storage_media_ready = false;
        }
    }
    else
    {
        g_user_count = 0;
        g_recent_user_valid = false;
        memset(g_users, 0, sizeof(g_users));
        (void) fx_media_close(&g_fx_media0);
        g_storage_media_ready = false;
    }
    storage_media_unlock();

    g_storage_debug_last_user_count = (ULONG) g_user_count;
    g_storage_debug_last_stage = 25U;
    g_storage_loaded = true;
}

static bool storage_load_users_now(void)
{
    UINT status;
    ULONG actual_bytes = 0U;
    bool loaded = false;
    ULONG start_tick = tx_time_get();

    storage_lock();
    if (g_storage_loaded)
    {
        g_storage_debug_last_stage = 21U;
        storage_unlock();
        return true;
    }
    storage_unlock();

    g_storage_debug_last_stage = 20U;
    g_storage_debug_last_read_bytes = 0U;

    if (!storage_media_access_allowed())
    {
        g_storage_debug_last_stage = 22U;
        return false;
    }

    storage_media_lock();
    if (!storage_media_open())
    {
        storage_media_unlock();
        g_storage_debug_last_stage = 23U;
        return false;
    }

    status = fx_file_open(&g_fx_media0, &g_users_file, "users.json", FX_OPEN_FOR_READ);
    g_storage_debug_last_load_status = status;
    if (FX_SUCCESS == status)
    {
        status = fx_file_read(&g_users_file, g_storage_load_json, JSON_BUFFER_SIZE - 1U, &actual_bytes);
        if (FX_SUCCESS == status)
        {
            g_storage_load_json[actual_bytes] = '\0';
            g_storage_debug_last_read_bytes = actual_bytes;
        }
        else
        {
            g_storage_load_json[0] = '\0';
            actual_bytes = 0U;
        }

        fx_file_close(&g_users_file);
    }
    else
    {
        g_storage_load_json[0] = '\0';
        actual_bytes = 0U;
    }

    if (FX_SUCCESS == fx_media_close(&g_fx_media0))
    {
        g_storage_media_ready = false;
    }
    storage_media_unlock();

    storage_lock();
    if (!g_storage_loaded)
    {
        if ((FX_SUCCESS == status) && (actual_bytes > 0U))
        {
            (void) storage_parse_users(g_storage_load_json);
        }
        else
        {
            g_user_count = 0;
            g_recent_user_valid = false;
            memset(g_users, 0, sizeof(g_users));
        }

        g_storage_debug_last_load_status = status;
        g_storage_debug_last_user_count = (ULONG) g_user_count;
        g_storage_debug_last_stage = 25U;
        g_storage_loaded = true;
    }
    loaded = g_storage_loaded;
    storage_unlock();

    app_metric_add_no_persist("Storage load latency", "users.json", tx_time_get() - start_tick, loaded);
    app_metric_add_no_persist("Reboot persistence", "apos reboot", tx_time_get() - start_tick, ((FX_SUCCESS == status) && (actual_bytes > 0U)));
    return loaded;
}

static bool storage_load_access_log_now(void)
{
    bool loaded_any = false;
    static char log_buffer[ACCESS_LOG_BUFFER_SIZE];
    ULONG start_tick = tx_time_get();

    storage_lock();
    if (g_storage_access_log_loaded)
    {
        storage_unlock();
        return true;
    }
    storage_unlock();

    if (!storage_media_access_allowed())
    {
        return false;
    }

    storage_media_lock();
    if (!storage_media_open())
    {
        storage_media_unlock();
        return false;
    }

    log_buffer[0] = '\0';
    loaded_any = storage_load_access_log_tail_file(ACCESS_LOG_ARCHIVE_FILE_NAME, log_buffer, sizeof(log_buffer)) || loaded_any;
    loaded_any = storage_load_access_log_tail_file(ACCESS_LOG_FILE_NAME, log_buffer, sizeof(log_buffer)) || loaded_any;

    if (FX_SUCCESS == fx_media_close(&g_fx_media0))
    {
        g_storage_media_ready = false;
    }
    storage_media_unlock();

    storage_lock();
    g_storage_access_log_loaded = true;
    storage_unlock();

    app_metric_add_no_persist("Storage load latency", "access.log", tx_time_get() - start_tick, loaded_any);
    return true;
}

static bool storage_load_metrics_now(void)
{
    UINT status;
    ULONG actual_bytes = 0U;
    static char log_buffer[METRICS_LOG_BUFFER_SIZE];
    ULONG start_tick = tx_time_get();

    storage_lock();
    if (g_storage_metrics_loaded)
    {
        storage_unlock();
        return true;
    }
    storage_unlock();

    if (!storage_media_access_allowed())
    {
        return false;
    }

    storage_media_lock();
    if (!storage_media_open())
    {
        storage_media_unlock();
        return false;
    }

    status = fx_file_open(&g_fx_media0, &g_metrics_file, "metrics.log", FX_OPEN_FOR_READ);
    if (FX_SUCCESS == status)
    {
        status = fx_file_read(&g_metrics_file, log_buffer, METRICS_LOG_BUFFER_SIZE - 1U, &actual_bytes);
        if (FX_SUCCESS == status)
        {
            log_buffer[actual_bytes] = '\0';
        }
        else
        {
            log_buffer[0] = '\0';
            actual_bytes = 0U;
        }

        fx_file_close(&g_metrics_file);
    }
    else
    {
        log_buffer[0] = '\0';
        actual_bytes = 0U;
    }

    if (FX_SUCCESS == fx_media_close(&g_fx_media0))
    {
        g_storage_media_ready = false;
    }
    storage_media_unlock();

    if ((FX_SUCCESS == status) && (actual_bytes > 0U))
    {
        (void) storage_parse_metrics_text(log_buffer);
    }

    storage_lock();
    g_storage_metrics_loaded = true;
    storage_unlock();

    app_metric_add_no_persist("Storage load latency", "metrics.log", tx_time_get() - start_tick, ((FX_SUCCESS == status) && (actual_bytes > 0U)));
    return true;
}

static void storage_ensure_loaded_locked(void)
{
    unsigned int attempts = 0U;

    while (!g_storage_loaded)
    {
        ULONG now = tx_time_get();

        if (now < STORAGE_MEDIA_SAFE_DELAY_TICKS)
        {
            ULONG wait_ticks = (STORAGE_MEDIA_SAFE_DELAY_TICKS - now) + 2U;
            storage_unlock();
            tx_thread_sleep(wait_ticks);
            storage_lock();
        }

        storage_unlock();
        (void) storage_load_users_now();
        storage_lock();
        if (g_storage_loaded)
        {
            break;
        }

        if (++attempts >= 3U)
        {
            break;
        }

        storage_unlock();
        tx_thread_sleep(5U);
        storage_lock();
    }
}
/* ========================================================================= */

static void storage_mark_runtime_state_loaded(void)
{
    g_storage_loaded = true;
}

void storage_init(void)
{
    if (g_storage_initialized)
    {
        return;
    }

    if (!g_storage_mutex_ready)
    {
        if (TX_SUCCESS == tx_mutex_create(&g_storage_mutex, "storage_mutex", TX_NO_INHERIT))
        {
            g_storage_mutex_ready = true;
        }
    }

    if (!g_storage_media_mutex_ready)
    {
        if (TX_SUCCESS == tx_mutex_create(&g_storage_media_mutex, "storage_media_mutex", TX_NO_INHERIT))
        {
            g_storage_media_mutex_ready = true;
        }
    }

    if (!g_storage_thread_ready)
    {
        if (TX_SUCCESS == tx_semaphore_create(&g_storage_persist_semaphore, "storage_persist", 0U))
        {
            if (TX_SUCCESS == tx_thread_create(&g_storage_thread,
                                               "storage_thread",
                                               storage_thread_entry,
                                               0U,
                                               g_storage_thread_stack,
                                               sizeof(g_storage_thread_stack),
                                               14U,
                                               14U,
                                               TX_NO_TIME_SLICE,
                                               TX_AUTO_START))
            {
                g_storage_thread_ready = true;
            }
        }
    }

    g_user_count = 0;
    g_recent_user_valid = false;
    g_storage_loaded = false;
    g_storage_access_log_loaded = false;
    g_storage_metrics_loaded = false;
    g_storage_access_log_pending_count = 0U;
    g_storage_access_log_work_text[0] = '\0';
    g_storage_access_log_work_text_size = 0U;
    g_storage_load_requested = true;
    g_storage_initialized = true;

    if (g_storage_thread_ready)
    {
        tx_semaphore_put(&g_storage_persist_semaphore);
    }
}

static void storage_thread_entry(ULONG initial_input)
{
    SSP_PARAMETER_NOT_USED(initial_input);

    while (1)
    {
        if (TX_SUCCESS == tx_semaphore_get(&g_storage_persist_semaphore, TX_WAIT_FOREVER))
        {
            bool ok = false;
            size_t persist_size = 0U;
            size_t access_log_size = 0U;
            unsigned int access_log_count = 0U;
            ULONG user_count_snapshot = 0U;

            g_storage_debug_worker_runs++;

            while (!storage_media_access_allowed())
            {
                tx_thread_sleep(10U);
            }

            storage_lock();
            if (g_storage_load_requested)
            {
                g_storage_debug_last_stage = 2U;
                g_storage_load_requested = false;
                storage_unlock();
                (void) storage_load_users_now();
                (void) storage_load_access_log_now();
                (void) storage_load_metrics_now();
                continue;
            }
            else if (g_storage_persist_requested)
            {
                g_storage_debug_last_stage = 1U;
                g_storage_persist_requested = false;
                if ((0 == g_user_count) && !g_recent_user_valid)
                {
                    storage_load_users_locked();
                }
                persist_size = g_storage_persist_json_size;
                if (persist_size > sizeof(g_storage_persist_work_json))
                {
                    persist_size = sizeof(g_storage_persist_work_json);
                }
                if (persist_size > 0U)
                {
                    memcpy(g_storage_persist_work_json, g_storage_persist_json, persist_size);
                    g_storage_persist_work_json_size = persist_size;
                }
                else
                {
                    g_storage_persist_work_json[0] = '\0';
                    g_storage_persist_work_json_size = 0U;
                }
                user_count_snapshot = (ULONG) g_user_count;
                storage_unlock();

                if (g_storage_persist_work_json_size > 0U)
                {
                    ok = storage_save_json_buffer(g_storage_persist_work_json, g_storage_persist_work_json_size);
                }
                g_storage_persist_status = ok ? STORAGE_PERSIST_STATUS_SUCCESS : STORAGE_PERSIST_STATUS_FAILED;
                g_storage_debug_last_user_count = user_count_snapshot;
                continue;
            }
            else if (g_storage_access_log_persist_requested)
            {
                g_storage_debug_last_stage = 16U;
                g_storage_access_log_persist_requested = false;
                access_log_size = 0U;
                access_log_count = storage_format_access_log_entries(g_storage_access_log_pending,
                                                                    g_storage_access_log_pending_count,
                                                                    g_storage_access_log_work_text,
                                                                    sizeof(g_storage_access_log_work_text),
                                                                    &access_log_size);
                g_storage_access_log_work_text_size = access_log_size;
                storage_unlock();

                while ((access_log_count > 0U) && (g_storage_access_log_work_text_size > 0U))
                {
                    ok = storage_save_access_log_buffer(g_storage_access_log_work_text,
                                                        g_storage_access_log_work_text_size);

                    storage_lock();
                    if (!ok)
                    {
                        storage_unlock();
                        break;
                    }

                    if (g_storage_access_log_pending_count > access_log_count)
                    {
                        memmove(g_storage_access_log_pending,
                                &g_storage_access_log_pending[access_log_count],
                                (g_storage_access_log_pending_count - access_log_count) * sizeof(g_storage_access_log_pending[0]));
                    }
                    g_storage_access_log_pending_count -= access_log_count;

                    access_log_size = 0U;
                    access_log_count = storage_format_access_log_entries(g_storage_access_log_pending,
                                                                        g_storage_access_log_pending_count,
                                                                        g_storage_access_log_work_text,
                                                                        sizeof(g_storage_access_log_work_text),
                                                                        &access_log_size);
                    g_storage_access_log_work_text_size = access_log_size;
                    storage_unlock();
                }
                continue;
            }
            else if (g_storage_metrics_persist_requested)
            {
                g_storage_debug_last_stage = 17U;
                g_storage_metrics_persist_requested = false;
                storage_unlock();

                storage_refresh_metrics_snapshot();
                if (g_storage_metrics_text_size > 0U)
                {
                    (void) storage_save_metrics_buffer(g_storage_metrics_text, g_storage_metrics_text_size);
                }
                continue;
            }
            else if (g_storage_photo_persist_requested)
            {
                g_storage_debug_last_stage = 3U;
                g_storage_photo_persist_requested = false;
                storage_copy_text(g_storage_photo_work_id,
                                  sizeof(g_storage_photo_work_id),
                                  g_storage_pending_photo_id,
                                  strlen(g_storage_pending_photo_id));
                storage_unlock();
                (void) storage_save_photo_file(g_storage_photo_work_id);
                continue;
            }
            storage_unlock();
        }
    }
}

bool storage_check_uid(const char *uid_str, char *out_name)
{
    bool found = false;
    char cleaned_uid[UID_MAX_LEN];
    const user_t *profile = NULL;

    if ((NULL == uid_str) || (NULL == out_name))
    {
        return false;
    }

    storage_copy_clean_uid(cleaned_uid, sizeof(cleaned_uid), uid_str);
    if ('\0' == cleaned_uid[0])
    {
        return false;
    }

    storage_init();
    storage_lock();
    storage_ensure_loaded_locked();
    profile = storage_find_profile_by_uid_locked(cleaned_uid);
    if (NULL != profile)
    {
        storage_copy_text(out_name, NAME_MAX_LEN, profile->name, strlen(profile->name));
        found = true;
    }

    storage_unlock();
    return found;
}

storage_access_result_t storage_authorize_uid(const char *uid_str, char *out_name)
{
    storage_access_result_t result = STORAGE_ACCESS_RESULT_NOT_FOUND;
    char cleaned_uid[UID_MAX_LEN];
    const user_t *profile = NULL;

    if ((NULL == uid_str) || ('\0' == uid_str[0]))
    {
        return STORAGE_ACCESS_RESULT_NOT_FOUND;
    }

    storage_copy_clean_uid(cleaned_uid, sizeof(cleaned_uid), uid_str);
    if ('\0' == cleaned_uid[0])
    {
        return STORAGE_ACCESS_RESULT_NOT_FOUND;
    }

    if (NULL != out_name)
    {
        out_name[0] = '\0';
    }

    storage_init();
    storage_lock();
    storage_ensure_loaded_locked();

    profile = storage_find_profile_by_uid_locked(cleaned_uid);
    if (NULL != profile)
    {
        if (NULL != out_name)
        {
            storage_copy_text(out_name, NAME_MAX_LEN, profile->name, strlen(profile->name));
        }

        result = STORAGE_ACCESS_RESULT_GRANTED;
        if (g_storage_meeting_mode_active &&
            !storage_meeting_mode_uid_allowed_locked(cleaned_uid))
        {
            result = STORAGE_ACCESS_RESULT_DENIED_MEETING_MODE;
        }
    }

    storage_unlock();
    return result;
}

int storage_user_count(void)
{
    int count;

    storage_init();
    storage_lock();
    storage_ensure_loaded_locked();
    count = g_user_count;
    storage_unlock();

    return count;
}

bool storage_user_get(int index, char *out_name, char *out_uid)
{
    bool ok = false;

    storage_init();
    storage_lock();
    storage_ensure_loaded_locked();

    if ((index >= 0) && (index < g_user_count))
    {
        if (NULL != out_name)
        {
            storage_copy_text(out_name, NAME_MAX_LEN, g_users[index].name, strlen(g_users[index].name));
        }
        if ((NULL != out_uid) && storage_profile_has_cards(&g_users[index]))
        {
            storage_copy_text(out_uid, UID_MAX_LEN, g_users[index].cards[0], strlen(g_users[index].cards[0]));
        }
        ok = true;
    }

    storage_unlock();
    return ok;
}

int storage_profile_snapshot(storage_user_profile_t *out_profiles, int max_profiles)
{
    int count = 0;

    if ((NULL == out_profiles) || (max_profiles <= 0))
    {
        return 0;
    }

    storage_init();
    storage_lock();
    storage_ensure_loaded_locked();

    count = g_user_count;
    if (count > max_profiles)
    {
        count = max_profiles;
    }

    for (int i = 0; i < count; i++)
    {
        out_profiles[i] = g_users[i];
    }

    storage_unlock();
    return count;
}

int storage_profile_snapshot_nowait(storage_user_profile_t *out_profiles, int max_profiles)
{
    int count = 0;

    if ((NULL == out_profiles) || (max_profiles <= 0))
    {
        return 0;
    }

    storage_init();
    storage_lock();

    count = g_user_count;
    if (count > max_profiles)
    {
        count = max_profiles;
    }

    for (int i = 0; i < count; i++)
    {
        out_profiles[i] = g_users[i];
    }

    storage_unlock();
    return count;
}

bool storage_profile_get(int index, storage_user_profile_t *out_profile)
{
    bool ok = false;

    if (NULL == out_profile)
    {
        return false;
    }

    storage_init();
    storage_lock();
    storage_ensure_loaded_locked();

    if ((index >= 0) && (index < g_user_count))
    {
        *out_profile = g_users[index];
        ok = true;
    }

    storage_unlock();
    return ok;
}

bool storage_profile_find_by_uid(const char *uid_str, storage_user_profile_t *out_profile)
{
    bool ok = false;
    char cleaned_uid[UID_MAX_LEN];
    const user_t *profile = NULL;

    if ((NULL == uid_str) || (NULL == out_profile) || ('\0' == uid_str[0]))
    {
        return false;
    }

    storage_copy_clean_uid(cleaned_uid, sizeof(cleaned_uid), uid_str);
    if ('\0' == cleaned_uid[0])
    {
        return false;
    }

    storage_init();
    storage_lock();
    storage_ensure_loaded_locked();

    profile = storage_find_profile_by_uid_locked(cleaned_uid);
    if (NULL != profile)
    {
        *out_profile = *profile;
        ok = true;
    }

    storage_unlock();
    return ok;
}

bool storage_profile_upsert(const storage_user_profile_t *profile, int edit_index)
{
    user_t normalized;
    int target_index = edit_index;

    if ((NULL == profile) || ('\0' == profile->name[0]))
    {
        return false;
    }

    storage_clear_profile(&normalized);
    storage_copy_text(normalized.name, sizeof(normalized.name), profile->name, strlen(profile->name));
    storage_copy_text(normalized.role, sizeof(normalized.role), profile->role, strlen(profile->role));
    storage_copy_text(normalized.chapter, sizeof(normalized.chapter), profile->chapter, strlen(profile->chapter));
    storage_copy_text(normalized.photo_id, sizeof(normalized.photo_id), profile->photo_id, strlen(profile->photo_id));
    normalized.is_admin = profile->is_admin;
    storage_copy_admin_pin(normalized.admin_pin, sizeof(normalized.admin_pin), profile->admin_pin);
    if (!normalized.is_admin)
    {
        normalized.admin_pin[0] = '\0';
    }

    storage_init();
    storage_lock();
    storage_ensure_loaded_locked();

    for (unsigned int card_index = 0U; card_index < profile->card_count; card_index++)
    {
        char cleaned_uid[UID_MAX_LEN];
        bool duplicate_in_same_profile = false;
        int owner_index;

        storage_copy_clean_uid(cleaned_uid, sizeof(cleaned_uid), profile->cards[card_index]);
        if ('\0' == cleaned_uid[0])
        {
            continue;
        }

        for (unsigned int existing = 0U; existing < normalized.card_count; existing++)
        {
            if (0 == strcmp(normalized.cards[existing], cleaned_uid))
            {
                duplicate_in_same_profile = true;
                break;
            }
        }

        if (duplicate_in_same_profile)
        {
            continue;
        }

        if (normalized.card_count >= STORAGE_MAX_CARDS_PER_USER)
        {
            break;
        }

        owner_index = storage_find_user_by_card_locked(cleaned_uid);
        if ((owner_index >= 0) && ((target_index < 0) || (owner_index != target_index)))
        {
            storage_unlock();
            return false;
        }

        storage_copy_text(normalized.cards[normalized.card_count],
                          sizeof(normalized.cards[normalized.card_count]),
                          cleaned_uid,
                          strlen(cleaned_uid));
        normalized.card_count++;
    }

    if ((target_index < 0) || (target_index >= g_user_count))
    {
        if (g_user_count >= STORAGE_MAX_USERS)
        {
            storage_unlock();
            return false;
        }

        target_index = g_user_count++;
    }

    g_users[target_index] = normalized;
    if (storage_profile_has_cards(&g_users[target_index]))
    {
        storage_set_recent_user_locked(&g_users[target_index]);
    }
    storage_mark_runtime_state_loaded();
    storage_unlock();
    return true;
}

bool storage_add_user(const char *uid_str, const char *name)
{
    storage_user_profile_t profile;
    bool ok = false;

    if ((NULL == uid_str) || (NULL == name) || ('\0' == uid_str[0]) || ('\0' == name[0]))
    {
        return false;
    }

    memset(&profile, 0, sizeof(profile));
    storage_copy_text(profile.name, sizeof(profile.name), name, strlen(name));
    storage_copy_clean_uid(profile.cards[0], sizeof(profile.cards[0]), uid_str);
    profile.card_count = ('\0' != profile.cards[0][0]) ? 1U : 0U;

    storage_init();
    storage_lock();
    storage_ensure_loaded_locked();

    {
        int existing_index = storage_find_user_by_card_locked(profile.cards[0]);

        if (existing_index >= 0)
        {
            storage_copy_text(g_users[existing_index].name, sizeof(g_users[existing_index].name), name, strlen(name));
            storage_set_recent_user_locked(&g_users[existing_index]);
            storage_mark_runtime_state_loaded();
            storage_unlock();
            (void) storage_persist_now();
            return true;
        }
    }

    storage_unlock();
    ok = storage_profile_upsert(&profile, -1);
    if (ok)
    {
        (void) storage_persist_now();
    }

    return ok;
}

bool storage_profile_remove(int index)
{
    bool ok = false;

    storage_init();
    storage_lock();
    storage_ensure_loaded_locked();

    if ((index >= 0) && (index < g_user_count))
    {
        for (int move_index = index; move_index < (g_user_count - 1); move_index++)
        {
            g_users[move_index] = g_users[move_index + 1];
        }
        if (g_user_count > 0)
        {
            g_user_count--;
        }
        storage_clear_profile(&g_users[g_user_count]);

        if (g_user_count > 0)
        {
            storage_set_recent_user_locked(&g_users[g_user_count - 1]);
        }
        else
        {
            storage_set_recent_user_locked(NULL);
        }

        storage_mark_runtime_state_loaded();
        ok = true;
    }

    storage_unlock();
    return ok;
}

bool storage_meeting_mode_start(const int *profile_indices,
                                int profile_count,
                                unsigned int *out_selected_profiles,
                                unsigned int *out_allowed_cards)
{
    unsigned int selected_profiles = 0U;
    unsigned int allowed_cards = 0U;
    bool seen_profiles[STORAGE_MAX_USERS];

    memset(seen_profiles, 0, sizeof(seen_profiles));

    storage_init();
    storage_lock();
    storage_ensure_loaded_locked();
    storage_meeting_mode_clear_locked();

    if ((NULL != profile_indices) && (profile_count > 0))
    {
        for (int i = 0; i < profile_count; i++)
        {
            int index = profile_indices[i];
            const user_t *profile;

            if ((index < 0) || (index >= g_user_count) || seen_profiles[index])
            {
                continue;
            }

            profile = &g_users[index];
            seen_profiles[index] = true;
            if (!storage_profile_has_cards(profile))
            {
                continue;
            }

            selected_profiles++;
            for (unsigned int card_index = 0U; card_index < profile->card_count; card_index++)
            {
                char cleaned_uid[UID_MAX_LEN];
                bool duplicate_card = false;

                storage_copy_clean_uid(cleaned_uid, sizeof(cleaned_uid), profile->cards[card_index]);
                if ('\0' == cleaned_uid[0])
                {
                    continue;
                }

                for (unsigned int existing = 0U; existing < allowed_cards; existing++)
                {
                    if (0 == strcmp(g_storage_meeting_mode_allowed_cards[existing], cleaned_uid))
                    {
                        duplicate_card = true;
                        break;
                    }
                }

                if (duplicate_card || (allowed_cards >= STORAGE_MEETING_MODE_MAX_ALLOWED_CARDS))
                {
                    continue;
                }

                storage_copy_text(g_storage_meeting_mode_allowed_cards[allowed_cards],
                                  sizeof(g_storage_meeting_mode_allowed_cards[allowed_cards]),
                                  cleaned_uid,
                                  strlen(cleaned_uid));
                allowed_cards++;
            }
        }
    }

    g_storage_meeting_mode_selected_profiles = selected_profiles;
    g_storage_meeting_mode_allowed_count = allowed_cards;
    g_storage_meeting_mode_active = ((selected_profiles > 0U) && (allowed_cards > 0U));
    if (!g_storage_meeting_mode_active)
    {
        storage_meeting_mode_clear_locked();
    }

    if (NULL != out_selected_profiles)
    {
        *out_selected_profiles = g_storage_meeting_mode_selected_profiles;
    }
    if (NULL != out_allowed_cards)
    {
        *out_allowed_cards = g_storage_meeting_mode_allowed_count;
    }

    storage_unlock();
    return g_storage_meeting_mode_active;
}

void storage_meeting_mode_stop(void)
{
    storage_init();
    storage_lock();
    storage_meeting_mode_clear_locked();
    storage_unlock();
}

bool storage_meeting_mode_is_active(void)
{
    bool active;

    storage_init();
    storage_lock();
    active = g_storage_meeting_mode_active;
    storage_unlock();
    return active;
}

unsigned int storage_meeting_mode_selected_profile_count(void)
{
    unsigned int count;

    storage_init();
    storage_lock();
    count = g_storage_meeting_mode_selected_profiles;
    storage_unlock();
    return count;
}

unsigned int storage_meeting_mode_allowed_card_count(void)
{
    unsigned int count;

    storage_init();
    storage_lock();
    count = g_storage_meeting_mode_allowed_count;
    storage_unlock();
    return count;
}

bool storage_meeting_mode_profile_selected(const storage_user_profile_t *profile)
{
    bool selected;

    storage_init();
    storage_lock();
    selected = storage_meeting_mode_profile_selected_locked(profile);
    storage_unlock();
    return selected;
}

bool storage_admin_pin_valid(const char *pin)
{
    bool ok = false;

    if ((NULL == pin) || ('\0' == pin[0]))
    {
        return false;
    }

    storage_init();
    storage_lock();
    storage_ensure_loaded_locked();

    for (int i = 0; i < g_user_count; i++)
    {
        if (g_users[i].is_admin &&
            ('\0' != g_users[i].admin_pin[0]) &&
            (0 == strcmp(g_users[i].admin_pin, pin)))
        {
            ok = true;
            break;
        }
    }

    storage_unlock();
    return ok;
}

bool storage_remove_uid(const char *uid_str)
{
    bool ok = false;

    if ((NULL == uid_str) || ('\0' == uid_str[0]))
    {
        return false;
    }

    storage_init();
    storage_lock();
    storage_load_users_locked();

    for (int i = 0; i < g_user_count; i++)
    {
        for (unsigned int card_index = 0U; card_index < g_users[i].card_count; card_index++)
        {
            if (0 == strcmp(g_users[i].cards[card_index], uid_str))
            {
                for (unsigned int move_index = card_index; (move_index + 1U) < g_users[i].card_count; move_index++)
                {
                    storage_copy_text(g_users[i].cards[move_index],
                                      sizeof(g_users[i].cards[move_index]),
                                      g_users[i].cards[move_index + 1U],
                                      strlen(g_users[i].cards[move_index + 1U]));
                }

                if (g_users[i].card_count > 0U)
                {
                    g_users[i].card_count--;
                    g_users[i].cards[g_users[i].card_count][0] = '\0';
                }

                if (g_recent_user_valid)
                {
                    bool recent_still_valid = false;

                    for (unsigned int recent_card_index = 0U; recent_card_index < g_recent_user.card_count; recent_card_index++)
                    {
                        if ('\0' != g_recent_user.cards[recent_card_index][0])
                        {
                            recent_still_valid = true;
                            break;
                        }
                    }

                    if (!recent_still_valid)
                    {
                        storage_set_recent_user_locked(NULL);
                    }
                }

                storage_mark_runtime_state_loaded();
                ok = true;
                storage_unlock();
                return ok;
            }
        }
    }

    storage_unlock();
    return ok;
}

bool storage_persist_now(void)
{
    storage_init();
    if (!g_storage_thread_ready)
    {
        return false;
    }

    storage_lock();
    storage_refresh_persist_snapshot_locked();

    if (0U == g_storage_persist_json_size)
    {
        storage_unlock();
        return false;
    }

    g_storage_persist_requested = true;
    g_storage_persist_status = STORAGE_PERSIST_STATUS_PENDING;
    storage_unlock();

    return (TX_SUCCESS == tx_semaphore_put(&g_storage_persist_semaphore));
}

bool storage_persist_wait(ULONG timeout_ticks)
{
    ULONG start_tick;

    storage_init();
    start_tick = tx_time_get();

    while ((tx_time_get() - start_tick) < timeout_ticks)
    {
        unsigned int status = storage_persist_status();

        if (STORAGE_PERSIST_STATUS_SUCCESS == status)
        {
            return true;
        }

        if (STORAGE_PERSIST_STATUS_FAILED == status)
        {
            return false;
        }

        tx_thread_sleep(2U);
    }

    return (STORAGE_PERSIST_STATUS_SUCCESS == storage_persist_status());
}

unsigned int storage_persist_status(void)
{
    return (unsigned int) g_storage_persist_status;
}

bool storage_access_log_enqueue(const app_access_log_entry_t *entry)
{
    storage_init();
    if ((!g_storage_thread_ready) || (NULL == entry))
    {
        return false;
    }

    storage_lock();
    if (g_storage_access_log_pending_count < ACCESS_LOG_PENDING_SIZE)
    {
        g_storage_access_log_pending[g_storage_access_log_pending_count++] = *entry;
    }
    else
    {
        memmove(g_storage_access_log_pending,
                &g_storage_access_log_pending[1],
                (ACCESS_LOG_PENDING_SIZE - 1U) * sizeof(g_storage_access_log_pending[0]));
        g_storage_access_log_pending[ACCESS_LOG_PENDING_SIZE - 1U] = *entry;
    }
    g_storage_access_log_persist_requested = true;
    storage_unlock();

    return (TX_SUCCESS == tx_semaphore_put(&g_storage_persist_semaphore));
}

bool storage_access_log_persist_now(void)
{
    storage_init();
    if (!g_storage_thread_ready)
    {
        return false;
    }

    storage_lock();
    g_storage_access_log_persist_requested = true;
    storage_unlock();

    return (TX_SUCCESS == tx_semaphore_put(&g_storage_persist_semaphore));
}

bool storage_access_log_ensure_loaded(void)
{
    storage_init();

    storage_lock();
    if (g_storage_access_log_loaded)
    {
        storage_unlock();
        return true;
    }
    storage_unlock();

    return storage_load_access_log_now();
}

bool storage_metrics_persist_now(void)
{
    storage_init();
    if (!g_storage_thread_ready)
    {
        return false;
    }

    storage_lock();
    g_storage_metrics_persist_requested = true;
    storage_unlock();

    return (TX_SUCCESS == tx_semaphore_put(&g_storage_persist_semaphore));
}

bool storage_metrics_ensure_loaded(void)
{
    storage_init();

    storage_lock();
    if (g_storage_metrics_loaded)
    {
        storage_unlock();
        return true;
    }
    storage_unlock();

    return storage_load_metrics_now();
}

bool storage_export_users_json_info(ULONG *out_size)
{
    if (NULL != out_size)
    {
        *out_size = 0U;
    }

    storage_init();
    if (!storage_media_access_allowed())
    {
        return false;
    }

    return storage_get_file_size("users.json", out_size);
}

bool storage_export_users_json(char *out, size_t out_size, size_t *out_len)
{
    ULONG actual_read = 0U;

    if ((NULL == out) || (0U == out_size))
    {
        return false;
    }

    storage_init();
    if (!storage_media_access_allowed())
    {
        return false;
    }

    if (!storage_read_file_chunk("users.json", 0U, out, out_size - 1U, &actual_read))
    {
        out[0] = '\0';
        return false;
    }

    out[actual_read] = '\0';
    if (NULL != out_len)
    {
        *out_len = (size_t) actual_read;
    }

    return true;
}

bool storage_export_users_json_read(ULONG offset, void *out, size_t out_size, size_t *out_read)
{
    ULONG actual_read = 0U;

    if (NULL != out_read)
    {
        *out_read = 0U;
    }

    storage_init();
    if (!storage_media_access_allowed())
    {
        return false;
    }

    if (!storage_read_file_chunk("users.json", offset, out, out_size, &actual_read))
    {
        return false;
    }

    if (NULL != out_read)
    {
        *out_read = (size_t) actual_read;
    }

    return true;
}

bool storage_photo_export_info(const char *photo_id, storage_photo_export_info_t *out_info)
{
    storage_photo_file_header_t header;
    char filename[32];
    ULONG actual_read = 0U;

    if ((NULL == photo_id) || ('\0' == photo_id[0]) || (NULL == out_info))
    {
        return false;
    }

    storage_init();
    if (!storage_media_access_allowed())
    {
        return false;
    }

    storage_photo_filename(photo_id, filename, sizeof(filename));
    if (!storage_read_file_chunk(filename, 0U, &header, sizeof(header), &actual_read))
    {
        return false;
    }

    if ((actual_read != sizeof(header)) || (header.magic != STORAGE_PHOTO_FILE_MAGIC))
    {
        return false;
    }

    out_info->width = header.width;
    out_info->height = header.height;
    out_info->payload_size = header.payload_size;
    out_info->total_size = (ULONG) sizeof(header) + header.payload_size;
    return true;
}

bool storage_photo_export_read(const char *photo_id, ULONG offset, void *out, size_t out_size, size_t *out_read)
{
    char filename[32];
    ULONG actual_read = 0U;

    if ((NULL == photo_id) || ('\0' == photo_id[0]) || (NULL == out) || (0U == out_size))
    {
        return false;
    }

    storage_init();
    if (!storage_media_access_allowed())
    {
        return false;
    }

    storage_photo_filename(photo_id, filename, sizeof(filename));
    if (!storage_read_file_chunk(filename, offset, out, out_size, &actual_read))
    {
        return false;
    }

    if (NULL != out_read)
    {
        *out_read = (size_t) actual_read;
    }

    return true;
}

bool storage_photo_persist_now(const char *photo_id)
{
    if ((NULL == photo_id) || ('\0' == photo_id[0]))
    {
        return false;
    }

    storage_init();
    if (!g_storage_thread_ready)
    {
        return false;
    }

    storage_lock();
    storage_copy_text(g_storage_pending_photo_id,
                      sizeof(g_storage_pending_photo_id),
                      photo_id,
                      strlen(photo_id));
    g_storage_photo_persist_requested = true;
    storage_unlock();

    return (TX_SUCCESS == tx_semaphore_put(&g_storage_persist_semaphore));
}

bool storage_photo_ensure_loaded(const char *photo_id)
{
    if ((NULL == photo_id) || ('\0' == photo_id[0]))
    {
        return false;
    }

    if (ui_has_uploaded_photo(photo_id))
    {
        return true;
    }

    storage_init();
    return storage_load_photo_file(photo_id);
}
