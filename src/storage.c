#include "storage.h"
#include "main.h"
#include "hal_data.h"
#include "ui.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
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

#define USERS_JSON_CHUNK_SIZE 512U
#define USERS_JSON_OBJECT_SIZE 1024U
#define ACCESS_LOG_BUFFER_SIZE 4096
#define ACCESS_LOG_PENDING_SIZE 128U
#define METRICS_PENDING_SIZE 128U
#define STORAGE_MEETING_MODE_MAX_ALLOWED_CARDS (STORAGE_MAX_USERS * STORAGE_MAX_CARDS_PER_USER)
#define USERS_FILE_NAME "users.json"
#define USERS_TEMP_FILE_NAME "users.tmp"
#define USERS_BACKUP_FILE_NAME "users.bak"
#define ACCESS_LOG_FILE_NAME "access.log"
#define ACCESS_LOG_ARCHIVE_FILE_NAME "access.bak"
#define ACCESS_LOG_TEMP_FILE_NAME "access.tmp"
#define METRICS_LOG_FILE_NAME "metrics.log"
#define METRICS_TEMP_FILE_NAME "metrics.tmp"
#define MEETING_SCHEDULE_FILE_NAME "meeting.json"
#define MEETING_SCHEDULE_TEMP_FILE_NAME "meeting.tmp"
#define MEETING_SCHEDULE_BACKUP_FILE_NAME "meeting.bak"
#define MEETING_ACTIVE_FILE_NAME "meeting_active.json"
#define MEETING_ACTIVE_TEMP_FILE_NAME "meeting_active.tmp"
#define MEETING_ACTIVE_BACKUP_FILE_NAME "meeting_active.bak"
#define MEETING_SCHEDULE_BUFFER_SIZE 12288U
#define METRICS_LOG_BUFFER_SIZE 6144
#define STORAGE_RETENTION_READ_BUFFER_SIZE 512U
#define STORAGE_RETENTION_LINE_BUFFER_SIZE 192U
#define STORAGE_MEDIA_SAFE_DELAY_TICKS (2U * TX_TIMER_TICKS_PER_SECOND)
#define STORAGE_THREAD_STACK_SIZE 4096U
#define STORAGE_PHOTO_COPY_ROWS 8U
#define STORAGE_PHOTO_FILE_MAGIC 0x50485247UL
#define STORAGE_QSPI_BASE_ADDRESS ((uint8_t *) 0x60000000)
#define STORAGE_QSPI_ERASE_SIZE_BYTES (8U * 1024U * 1024U)
#define STORAGE_QSPI_ERASE_TIMEOUT_TICKS (120U * TX_TIMER_TICKS_PER_SECOND)

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
static bool g_storage_io_mutex_ready = false;
static bool g_storage_media_ready = false;
static bool g_storage_loaded = false;
static bool g_storage_users_load_failed = false;
static bool g_storage_access_log_loaded = false;
static bool g_storage_metrics_loaded = false;
static bool g_storage_thread_ready = false;
static bool g_storage_persist_requested = false;
static bool g_storage_format_requested = false;
static bool g_storage_access_log_persist_requested = false;
static bool g_storage_metrics_persist_requested = false;
static bool g_storage_photo_persist_requested = false;
static bool g_storage_load_requested = false;
static bool g_recent_user_valid = false;
static bool g_storage_meeting_mode_active = false;
static TX_MUTEX g_storage_mutex;
static TX_MUTEX g_storage_media_mutex;
static TX_MUTEX g_storage_io_mutex;
static TX_SEMAPHORE g_storage_persist_semaphore;
static TX_THREAD g_storage_thread;
static ULONG g_storage_thread_stack[STORAGE_THREAD_STACK_SIZE / sizeof(ULONG)];
static FX_FILE g_users_file;
static FX_FILE g_access_log_file;
static FX_FILE g_metrics_file;
static FX_FILE g_photo_file;
static FX_FILE g_meeting_schedule_file;
static FX_FILE g_meeting_active_file;
static FX_FILE g_retention_source_file;
static FX_FILE g_retention_temp_file;
static user_t g_storage_recent_io_snapshot;
static char g_storage_users_json_chunk[USERS_JSON_CHUNK_SIZE];
static char g_storage_users_json_object[USERS_JSON_OBJECT_SIZE];
static app_access_log_entry_t g_storage_access_log_pending[ACCESS_LOG_PENDING_SIZE];
static app_access_log_entry_t g_storage_access_log_work_entries[ACCESS_LOG_PENDING_SIZE];
static app_metric_entry_t g_storage_metric_pending[METRICS_PENDING_SIZE];
static app_metric_entry_t g_storage_metric_work_entries[METRICS_PENDING_SIZE];
static char g_storage_meeting_mode_allowed_cards[STORAGE_MEETING_MODE_MAX_ALLOWED_CARDS][UID_MAX_LEN];
static uint8_t g_storage_meeting_mode_profile_indices[STORAGE_MAX_USERS];
static char g_storage_meeting_mode_chapter[STORAGE_CHAPTER_MAX_LEN];
static char g_storage_access_log_work_text[ACCESS_LOG_BUFFER_SIZE];
static char g_storage_metrics_text[METRICS_LOG_BUFFER_SIZE];
static char g_storage_meeting_schedule_text[MEETING_SCHEDULE_BUFFER_SIZE];
static char g_storage_retention_read_buffer[STORAGE_RETENTION_READ_BUFFER_SIZE];
static char g_storage_retention_line_buffer[STORAGE_RETENTION_LINE_BUFFER_SIZE];
static char g_storage_pending_photo_id[STORAGE_PHOTO_ID_MAX_LEN];
static char g_storage_photo_work_id[STORAGE_PHOTO_ID_MAX_LEN];
static size_t g_storage_persist_json_size = 0U;
static size_t g_storage_access_log_work_text_size = 0U;
static size_t g_storage_metrics_text_size = 0U;
static unsigned int g_storage_access_log_pending_count = 0U;
static unsigned int g_storage_metric_pending_count = 0U;
static unsigned int g_storage_meeting_mode_selected_profiles = 0U;
static unsigned int g_storage_meeting_mode_allowed_count = 0U;
static ULONG g_storage_meeting_mode_start_unix = 0U;
static ULONG g_storage_meeting_mode_end_unix = 0U;
static int g_storage_users_io_count = 0;
static bool g_storage_recent_io_valid = false;
static volatile ULONG g_storage_persist_status = STORAGE_PERSIST_STATUS_IDLE;
volatile ULONG g_storage_debug_worker_runs = 0U;
volatile ULONG g_storage_debug_last_stage = 0U;
volatile ULONG g_storage_debug_last_media_status = 0U;
volatile ULONG g_storage_debug_last_save_status = 0U;
volatile ULONG g_storage_debug_last_load_status = 0U;
volatile ULONG g_storage_debug_last_saved_bytes = 0U;
volatile ULONG g_storage_debug_last_read_bytes = 0U;
volatile ULONG g_storage_debug_last_user_count = 0U;
volatile ULONG g_storage_debug_direct_persist_requests = 0U;
volatile ULONG g_storage_debug_direct_persist_successes = 0U;
volatile ULONG g_storage_debug_last_format_status = 0U;
volatile ULONG g_storage_debug_last_erase_status = 0U;
volatile ULONG g_storage_debug_last_erase_step = 0U;

typedef struct st_storage_photo_file_header
{
    ULONG magic;
    USHORT width;
    USHORT height;
    ULONG payload_size;
} storage_photo_file_header_t;

static void storage_thread_entry(ULONG initial_input);
static void storage_ensure_loaded_locked(void);
static void storage_release_qspi_stack_locked(void);
static bool storage_erase_qspi_now_locked(void);
static bool storage_format_media_now(void);
static bool storage_save_users_json_snapshot(size_t *out_json_size, ULONG *out_user_count);
static bool storage_load_users_file(const char *filename, bool commit_profiles);
static bool storage_save_access_log_buffer(const char *log_buffer, size_t log_size);
static bool storage_save_metrics_buffer(const char *log_buffer, size_t log_size);
static bool storage_history_entry_is_retained(ULONG entry_unix_utc, ULONG now_unix_utc);
static bool storage_log_line_is_retained(const char *line, size_t line_len, ULONG now_unix_utc);
static bool storage_scan_log_file_for_expired_lines(const char *filename, ULONG now_unix_utc, bool *out_has_expired);
static bool storage_rewrite_log_file_retention(const char *filename, const char *temp_filename, ULONG now_unix_utc);
static bool storage_prune_log_file_retention(const char *filename, const char *temp_filename, ULONG now_unix_utc);
static bool storage_prune_access_logs_open_media(void);
static bool storage_prune_metrics_log_open_media(void);
static bool storage_save_meeting_schedules_json(const storage_meeting_schedule_t *schedules, int schedule_count);
static bool storage_format_meeting_active_json(const storage_meeting_active_t *state, char *out, size_t out_size, size_t *out_len);
static bool storage_save_meeting_active_json(const storage_meeting_active_t *state);
static int storage_parse_meeting_active_json(const char *json_text, storage_meeting_active_t *out_state);
static bool storage_clear_meeting_active_file(void);
static int storage_parse_meeting_schedules_json(const char *json_text,
                                                storage_meeting_schedule_t *out_schedules,
                                                int max_schedules);
static bool storage_load_users_now(void);
static bool storage_load_users_stream(void);
static bool storage_load_access_log_now(void);
static bool storage_load_metrics_now(void);
static unsigned int storage_parse_access_log_text_into(const char *log_buffer,
                                                       app_access_log_entry_t *entries,
                                                       unsigned int max_entries,
                                                       unsigned int *entry_count);
static unsigned int storage_format_metric_entries(const app_metric_entry_t *entries,
                                                  unsigned int entry_count,
                                                  char *out,
                                                  size_t out_size,
                                                  size_t *out_len);
static void storage_requeue_metric_entries_locked(const app_metric_entry_t *entries, unsigned int entry_count);
static void storage_media_lock(void);
static void storage_media_unlock(void);
static void storage_io_lock(void);
static void storage_io_unlock(void);
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

static void storage_release_qspi_stack_locked(void)
{
    sf_block_media_lx_nor_instance_ctrl_t *block_ctrl;
    sf_memory_qspi_nor_instance_ctrl_t *memory_ctrl;

    g_storage_debug_last_erase_step = 1U;
    if (FX_MEDIA_ID == g_fx_media0.fx_media_id)
    {
        (void) fx_media_close(&g_fx_media0);
    }
    g_storage_media_ready = false;

    block_ctrl = (sf_block_media_lx_nor_instance_ctrl_t *) g_sf_block_media_lx_nor0.p_ctrl;
    if ((NULL != block_ctrl) && (0U != block_ctrl->open) && (NULL != block_ctrl->p_nor_flash))
    {
        g_storage_debug_last_erase_step = 2U;
        (void) g_sf_block_media_lx_nor0.p_api->close(g_sf_block_media_lx_nor0.p_ctrl);
    }
    else if ((NULL != block_ctrl) && (NULL != block_ctrl->close))
    {
        g_storage_debug_last_erase_step = 3U;
        (void) block_ctrl->close();
        block_ctrl->open = 0U;
    }

    memory_ctrl = (sf_memory_qspi_nor_instance_ctrl_t *) g_sf_memory_qspi_nor0.p_ctrl;
    if ((NULL != memory_ctrl) && (0U != memory_ctrl->open) && (NULL != memory_ctrl->p_qspi))
    {
        g_storage_debug_last_erase_step = 4U;
        (void) g_sf_memory_qspi_nor0.p_api->close(g_sf_memory_qspi_nor0.p_ctrl);
    }

    g_storage_debug_last_erase_step = 5U;
    (void) g_qspi0.p_api->close(g_qspi0.p_ctrl);
    tx_thread_sleep(2U);
}

static bool storage_erase_qspi_now_locked(void)
{
    ssp_err_t status;
    bool in_progress = true;
    ULONG start_tick;

    g_storage_debug_last_stage = 26U;
    g_storage_debug_last_erase_status = SSP_SUCCESS;
    g_storage_debug_last_erase_step = 0U;

    storage_release_qspi_stack_locked();

    g_storage_debug_last_erase_step = 6U;
    status = g_qspi0.p_api->open(g_qspi0.p_ctrl, g_qspi0.p_cfg);
    if ((SSP_SUCCESS != status) && (SSP_ERR_ALREADY_OPEN != status))
    {
        g_storage_debug_last_erase_status = (ULONG) status;
        return false;
    }

    g_storage_debug_last_erase_step = 7U;
    status = g_qspi0.p_api->erase(g_qspi0.p_ctrl,
                                  STORAGE_QSPI_BASE_ADDRESS,
                                  STORAGE_QSPI_ERASE_SIZE_BYTES);
    if (SSP_SUCCESS != status)
    {
        g_storage_debug_last_erase_status = (ULONG) status;
        (void) g_qspi0.p_api->close(g_qspi0.p_ctrl);
        return false;
    }

    start_tick = tx_time_get();
    while (in_progress)
    {
        g_storage_debug_last_erase_step = 8U;
        status = g_qspi0.p_api->statusGet(g_qspi0.p_ctrl, &in_progress);
        if (SSP_SUCCESS != status)
        {
            g_storage_debug_last_erase_status = (ULONG) status;
            (void) g_qspi0.p_api->close(g_qspi0.p_ctrl);
            return false;
        }
        if (!in_progress)
        {
            break;
        }
        if ((tx_time_get() - start_tick) > STORAGE_QSPI_ERASE_TIMEOUT_TICKS)
        {
            g_storage_debug_last_erase_status = SSP_ERR_TIMEOUT;
            (void) g_qspi0.p_api->close(g_qspi0.p_ctrl);
            return false;
        }
        tx_thread_sleep(1U);
    }

    g_storage_debug_last_erase_step = 9U;
    status = g_qspi0.p_api->close(g_qspi0.p_ctrl);
    if ((SSP_SUCCESS != status) && (SSP_ERR_NOT_OPEN != status))
    {
        g_storage_debug_last_erase_status = (ULONG) status;
        return false;
    }

    g_storage_debug_last_erase_status = SSP_SUCCESS;
    g_storage_debug_last_erase_step = 10U;
    return true;
}

static bool storage_format_media_now(void)
{
    ssp_err_t format_status;
    UINT open_status;

    storage_media_lock();
    g_storage_debug_last_stage = 19U;
    g_storage_debug_last_format_status = SSP_SUCCESS;

    if (g_storage_media_ready)
    {
        (void) fx_media_close(&g_fx_media0);
        g_storage_media_ready = false;
        tx_thread_sleep(2U);
    }

    if (!storage_erase_qspi_now_locked())
    {
        g_storage_debug_last_media_status = g_storage_debug_last_erase_status;
        storage_media_unlock();
        return false;
    }

    tx_thread_sleep(2U);
    g_storage_debug_last_stage = 19U;
    format_status = fx_media_init0_format();
    g_storage_debug_last_format_status = (ULONG) format_status;
    if (SSP_SUCCESS != format_status)
    {
        g_storage_debug_last_media_status = (ULONG) format_status;
        storage_media_unlock();
        return false;
    }

    open_status = fx_media_init0_open();
    g_storage_debug_last_media_status = open_status;
    if (FX_SUCCESS == open_status)
    {
        g_storage_media_ready = true;
    }

    storage_media_unlock();
    return (FX_SUCCESS == open_status);
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

    if (0 == strcmp(filename, USERS_FILE_NAME))
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

    if (0 == strcmp(filename, USERS_FILE_NAME))
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

static void storage_io_lock(void)
{
    if (g_storage_io_mutex_ready)
    {
        tx_mutex_get(&g_storage_io_mutex, TX_WAIT_FOREVER);
    }
}

static void storage_io_unlock(void)
{
    if (g_storage_io_mutex_ready)
    {
        tx_mutex_put(&g_storage_io_mutex);
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

static bool storage_appendf(char *buffer, size_t buffer_size, size_t *offset, const char *format, ...)
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

static bool storage_history_entry_is_retained(ULONG entry_unix_utc, ULONG now_unix_utc)
{
    if ((0U == now_unix_utc) || (entry_unix_utc >= now_unix_utc))
    {
        return true;
    }

    return ((now_unix_utc - entry_unix_utc) <= APP_HISTORY_RETENTION_SECONDS);
}

static bool storage_log_line_is_retained(const char *line, size_t line_len, ULONG now_unix_utc)
{
    char timestamp_text[16];
    size_t timestamp_len = 0U;
    char *end_ptr = NULL;
    unsigned long entry_unix;

    if ((NULL == line) || (0U == line_len) || (0U == now_unix_utc))
    {
        return true;
    }

    while ((timestamp_len < line_len) && ('|' != line[timestamp_len]) &&
           ('\r' != line[timestamp_len]) && ('\n' != line[timestamp_len]))
    {
        if (!isdigit((unsigned char) line[timestamp_len]) ||
            ((timestamp_len + 1U) >= sizeof(timestamp_text)))
        {
            return true;
        }
        timestamp_text[timestamp_len] = line[timestamp_len];
        timestamp_len++;
    }

    if ((0U == timestamp_len) || (timestamp_len >= line_len) || ('|' != line[timestamp_len]))
    {
        return true;
    }

    timestamp_text[timestamp_len] = '\0';
    entry_unix = strtoul(timestamp_text, &end_ptr, 10);
    if ((NULL == end_ptr) || ('\0' != *end_ptr))
    {
        return true;
    }

    return storage_history_entry_is_retained((ULONG) entry_unix, now_unix_utc);
}

static bool storage_scan_retention_line(const char *line,
                                        size_t line_len,
                                        ULONG now_unix_utc,
                                        bool *has_expired)
{
    if ((NULL == has_expired) || (NULL == line))
    {
        return false;
    }

    if (!storage_log_line_is_retained(line, line_len, now_unix_utc))
    {
        *has_expired = true;
    }

    return true;
}

static bool storage_scan_log_file_for_expired_lines(const char *filename, ULONG now_unix_utc, bool *out_has_expired)
{
    UINT status;
    ULONG remaining;
    size_t line_len = 0U;
    bool line_overflow = false;

    if (NULL != out_has_expired)
    {
        *out_has_expired = false;
    }

    if ((NULL == filename) || (NULL == out_has_expired) || (0U == now_unix_utc))
    {
        return true;
    }

    status = fx_file_open(&g_fx_media0, &g_retention_source_file, (CHAR *) filename, FX_OPEN_FOR_READ);
    if (FX_SUCCESS != status)
    {
        return true;
    }

    remaining = (ULONG) g_retention_source_file.fx_file_current_file_size;
    while (remaining > 0U)
    {
        ULONG to_read = remaining;
        ULONG actual_bytes = 0U;

        if (to_read > sizeof(g_storage_retention_read_buffer))
        {
            to_read = sizeof(g_storage_retention_read_buffer);
        }

        status = fx_file_read(&g_retention_source_file,
                              g_storage_retention_read_buffer,
                              to_read,
                              &actual_bytes);
        if ((FX_SUCCESS != status) || (0U == actual_bytes))
        {
            (void) fx_file_close(&g_retention_source_file);
            return false;
        }

        remaining -= actual_bytes;
        for (ULONG i = 0U; i < actual_bytes; i++)
        {
            char ch = g_storage_retention_read_buffer[i];

            if (!line_overflow)
            {
                if ((line_len + 1U) < sizeof(g_storage_retention_line_buffer))
                {
                    g_storage_retention_line_buffer[line_len++] = ch;
                }
                else
                {
                    line_overflow = true;
                    line_len = 0U;
                }
            }

            if ('\n' == ch)
            {
                if (!line_overflow)
                {
                    (void) storage_scan_retention_line(g_storage_retention_line_buffer,
                                                       line_len,
                                                       now_unix_utc,
                                                       out_has_expired);
                }
                line_len = 0U;
                line_overflow = false;
            }
        }
    }

    if ((line_len > 0U) && !line_overflow)
    {
        (void) storage_scan_retention_line(g_storage_retention_line_buffer,
                                           line_len,
                                           now_unix_utc,
                                           out_has_expired);
    }

    (void) fx_file_close(&g_retention_source_file);
    return true;
}

static bool storage_write_retained_line(FX_FILE *output_file,
                                        const char *line,
                                        size_t line_len,
                                        ULONG now_unix_utc,
                                        bool *changed,
                                        ULONG *kept_lines)
{
    UINT status;

    if ((NULL == output_file) || (NULL == line) || (NULL == changed) || (NULL == kept_lines))
    {
        return false;
    }

    if (!storage_log_line_is_retained(line, line_len, now_unix_utc))
    {
        *changed = true;
        return true;
    }

    status = fx_file_write(output_file, (VOID *) line, (ULONG) line_len);
    if (FX_SUCCESS != status)
    {
        return false;
    }

    (*kept_lines)++;
    return true;
}

static bool storage_rewrite_log_file_retention(const char *filename, const char *temp_filename, ULONG now_unix_utc)
{
    UINT status;
    ULONG remaining;
    size_t line_len = 0U;
    bool line_overflow = false;
    bool changed = false;
    ULONG kept_lines = 0U;
    bool ok = true;

    if ((NULL == filename) || (NULL == temp_filename) || (0U == now_unix_utc))
    {
        return true;
    }

    status = fx_file_open(&g_fx_media0, &g_retention_source_file, (CHAR *) filename, FX_OPEN_FOR_READ);
    if (FX_SUCCESS != status)
    {
        return true;
    }

    (void) fx_file_delete(&g_fx_media0, (CHAR *) temp_filename);
    status = fx_file_create(&g_fx_media0, (CHAR *) temp_filename);
    if ((FX_SUCCESS != status) && (FX_ALREADY_CREATED != status))
    {
        (void) fx_file_close(&g_retention_source_file);
        return false;
    }

    status = fx_file_open(&g_fx_media0, &g_retention_temp_file, (CHAR *) temp_filename, FX_OPEN_FOR_WRITE);
    if (FX_SUCCESS != status)
    {
        (void) fx_file_close(&g_retention_source_file);
        return false;
    }

    status = fx_file_truncate(&g_retention_temp_file, 0U);
    if (FX_SUCCESS != status)
    {
        (void) fx_file_close(&g_retention_temp_file);
        (void) fx_file_close(&g_retention_source_file);
        (void) fx_file_delete(&g_fx_media0, (CHAR *) temp_filename);
        return false;
    }

    remaining = (ULONG) g_retention_source_file.fx_file_current_file_size;
    while ((remaining > 0U) && ok)
    {
        ULONG to_read = remaining;
        ULONG actual_bytes = 0U;

        if (to_read > sizeof(g_storage_retention_read_buffer))
        {
            to_read = sizeof(g_storage_retention_read_buffer);
        }

        status = fx_file_read(&g_retention_source_file,
                              g_storage_retention_read_buffer,
                              to_read,
                              &actual_bytes);
        if ((FX_SUCCESS != status) || (0U == actual_bytes))
        {
            ok = false;
            break;
        }

        remaining -= actual_bytes;
        for (ULONG i = 0U; i < actual_bytes; i++)
        {
            char ch = g_storage_retention_read_buffer[i];

            if (!line_overflow)
            {
                if ((line_len + 1U) < sizeof(g_storage_retention_line_buffer))
                {
                    g_storage_retention_line_buffer[line_len++] = ch;
                }
                else
                {
                    changed = true;
                    line_overflow = true;
                    line_len = 0U;
                }
            }

            if ('\n' == ch)
            {
                if (!line_overflow)
                {
                    ok = storage_write_retained_line(&g_retention_temp_file,
                                                     g_storage_retention_line_buffer,
                                                     line_len,
                                                     now_unix_utc,
                                                     &changed,
                                                     &kept_lines);
                    if (!ok)
                    {
                        break;
                    }
                }
                line_len = 0U;
                line_overflow = false;
            }
        }
    }

    if (ok && (line_len > 0U) && !line_overflow)
    {
        ok = storage_write_retained_line(&g_retention_temp_file,
                                         g_storage_retention_line_buffer,
                                         line_len,
                                         now_unix_utc,
                                         &changed,
                                         &kept_lines);
    }

    (void) fx_file_close(&g_retention_temp_file);
    (void) fx_file_close(&g_retention_source_file);

    if (!ok)
    {
        (void) fx_file_delete(&g_fx_media0, (CHAR *) temp_filename);
        return false;
    }

    if (!changed)
    {
        (void) fx_file_delete(&g_fx_media0, (CHAR *) temp_filename);
        return true;
    }

    (void) fx_file_delete(&g_fx_media0, (CHAR *) filename);
    if (0U == kept_lines)
    {
        (void) fx_file_delete(&g_fx_media0, (CHAR *) temp_filename);
        return true;
    }

    status = fx_file_rename(&g_fx_media0, (CHAR *) temp_filename, (CHAR *) filename);
    if (FX_SUCCESS != status)
    {
        (void) fx_file_delete(&g_fx_media0, (CHAR *) temp_filename);
        return false;
    }

    return true;
}

static bool storage_prune_log_file_retention(const char *filename, const char *temp_filename, ULONG now_unix_utc)
{
    bool has_expired = false;

    if ((NULL == filename) || (NULL == temp_filename) || (0U == now_unix_utc))
    {
        return true;
    }

    if (!storage_scan_log_file_for_expired_lines(filename, now_unix_utc, &has_expired))
    {
        return false;
    }

    if (!has_expired)
    {
        return true;
    }

    return storage_rewrite_log_file_retention(filename, temp_filename, now_unix_utc);
}

static bool storage_prune_access_logs_open_media(void)
{
    ULONG now_unix_utc = 0U;
    bool ok = true;

    if (!app_time_get_utc(&now_unix_utc))
    {
        return true;
    }

    ok = storage_prune_log_file_retention(ACCESS_LOG_ARCHIVE_FILE_NAME,
                                          ACCESS_LOG_TEMP_FILE_NAME,
                                          now_unix_utc) && ok;
    ok = storage_prune_log_file_retention(ACCESS_LOG_FILE_NAME,
                                          ACCESS_LOG_TEMP_FILE_NAME,
                                          now_unix_utc) && ok;
    if (ok)
    {
        (void) fx_media_flush(&g_fx_media0);
    }

    return ok;
}

static bool storage_prune_metrics_log_open_media(void)
{
    ULONG now_unix_utc = 0U;
    bool ok;

    if (!app_time_get_utc(&now_unix_utc))
    {
        return true;
    }

    ok = storage_prune_log_file_retention(METRICS_LOG_FILE_NAME,
                                          METRICS_TEMP_FILE_NAME,
                                          now_unix_utc);
    if (ok)
    {
        (void) fx_media_flush(&g_fx_media0);
    }

    return ok;
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

static void storage_requeue_access_log_entries_locked(const app_access_log_entry_t *entries, unsigned int entry_count)
{
    unsigned int count_to_restore = entry_count;
    unsigned int pending_to_keep = g_storage_access_log_pending_count;

    if ((NULL == entries) || (0U == entry_count))
    {
        return;
    }

    if (count_to_restore > ACCESS_LOG_PENDING_SIZE)
    {
        count_to_restore = ACCESS_LOG_PENDING_SIZE;
    }

    if (pending_to_keep > (ACCESS_LOG_PENDING_SIZE - count_to_restore))
    {
        unsigned int drop_count = pending_to_keep - (ACCESS_LOG_PENDING_SIZE - count_to_restore);

        memmove(g_storage_access_log_pending,
                &g_storage_access_log_pending[drop_count],
                (pending_to_keep - drop_count) * sizeof(g_storage_access_log_pending[0]));
        pending_to_keep -= drop_count;
    }

    if (pending_to_keep > 0U)
    {
        memmove(&g_storage_access_log_pending[count_to_restore],
                g_storage_access_log_pending,
                pending_to_keep * sizeof(g_storage_access_log_pending[0]));
    }

    memcpy(g_storage_access_log_pending, entries, count_to_restore * sizeof(g_storage_access_log_pending[0]));
    g_storage_access_log_pending_count = count_to_restore + pending_to_keep;
    g_storage_access_log_persist_requested = true;
}

static unsigned int storage_format_metric_entries(const app_metric_entry_t *entries,
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
        int written = snprintf(&out[offset],
                               out_size - offset,
                               "%lu|%lu|%u|%u|%u\n",
                               (unsigned long) entries[i].unix_utc,
                               (unsigned long) entries[i].duration_ticks,
                               entries[i].success ? 1U : 0U,
                               (unsigned int) entries[i].kind,
                               (unsigned int) entries[i].case_id);
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

static void storage_requeue_metric_entries_locked(const app_metric_entry_t *entries, unsigned int entry_count)
{
    unsigned int count_to_restore = entry_count;
    unsigned int pending_to_keep = g_storage_metric_pending_count;

    if ((NULL == entries) || (0U == entry_count))
    {
        return;
    }

    if (count_to_restore > METRICS_PENDING_SIZE)
    {
        count_to_restore = METRICS_PENDING_SIZE;
    }

    if (pending_to_keep > (METRICS_PENDING_SIZE - count_to_restore))
    {
        unsigned int drop_count = pending_to_keep - (METRICS_PENDING_SIZE - count_to_restore);

        memmove(g_storage_metric_pending,
                &g_storage_metric_pending[drop_count],
                (pending_to_keep - drop_count) * sizeof(g_storage_metric_pending[0]));
        pending_to_keep -= drop_count;
    }

    if (pending_to_keep > 0U)
    {
        memmove(&g_storage_metric_pending[count_to_restore],
                g_storage_metric_pending,
                pending_to_keep * sizeof(g_storage_metric_pending[0]));
    }

    memcpy(g_storage_metric_pending, entries, count_to_restore * sizeof(g_storage_metric_pending[0]));
    g_storage_metric_pending_count = count_to_restore + pending_to_keep;
    g_storage_metrics_persist_requested = true;
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

static bool storage_append_json_char(char *json_buffer, size_t json_buffer_size, size_t *offset, char ch)
{
    if ((NULL == json_buffer) || (NULL == offset) || ((*offset + 1U) >= json_buffer_size))
    {
        return false;
    }

    json_buffer[*offset] = ch;
    (*offset)++;
    json_buffer[*offset] = '\0';
    return true;
}

static bool storage_append_json_escaped_string(char *json_buffer,
                                               size_t json_buffer_size,
                                               size_t *offset,
                                               const char *value)
{
    const char *cursor = (NULL != value) ? value : "";

    if (!storage_append_json_char(json_buffer, json_buffer_size, offset, '"'))
    {
        return false;
    }

    while ('\0' != *cursor)
    {
        unsigned char ch = (unsigned char) *cursor++;

        if (('"' == ch) || ('\\' == ch))
        {
            if (!storage_append_json_char(json_buffer, json_buffer_size, offset, '\\') ||
                !storage_append_json_char(json_buffer, json_buffer_size, offset, (char) ch))
            {
                return false;
            }
        }
        else if ('\n' == ch)
        {
            if (!storage_append_jsonf(json_buffer, json_buffer_size, offset, "\\n"))
            {
                return false;
            }
        }
        else if ('\r' == ch)
        {
            if (!storage_append_jsonf(json_buffer, json_buffer_size, offset, "\\r"))
            {
                return false;
            }
        }
        else if ('\t' == ch)
        {
            if (!storage_append_jsonf(json_buffer, json_buffer_size, offset, "\\t"))
            {
                return false;
            }
        }
        else if (ch < 0x20U)
        {
            if (!storage_append_json_char(json_buffer, json_buffer_size, offset, ' '))
            {
                return false;
            }
        }
        else if (!storage_append_json_char(json_buffer, json_buffer_size, offset, (char) ch))
        {
            return false;
        }
    }

    return storage_append_json_char(json_buffer, json_buffer_size, offset, '"');
}

static bool storage_append_json_string_field(char *json_buffer,
                                             size_t json_buffer_size,
                                             size_t *offset,
                                             const char *key,
                                             const char *value,
                                             bool prepend_comma)
{
    if (!storage_append_jsonf(json_buffer,
                              json_buffer_size,
                              offset,
                              "%s\"%s\":",
                              prepend_comma ? "," : "",
                              key))
    {
        return false;
    }

    return storage_append_json_escaped_string(json_buffer, json_buffer_size, offset, value);
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

    if (!storage_append_jsonf(json_buffer, json_buffer_size, offset, "%s{", prepend_comma ? "," : ""))
    {
        return false;
    }

    if (!storage_append_json_string_field(json_buffer,
                                          json_buffer_size,
                                          offset,
                                          "name",
                                          profile->name,
                                          false))
    {
        return false;
    }

    if (('\0' != profile->role[0]) &&
        !storage_append_json_string_field(json_buffer, json_buffer_size, offset, "role", profile->role, true))
    {
        return false;
    }

    if (('\0' != profile->chapter[0]) &&
        !storage_append_json_string_field(json_buffer, json_buffer_size, offset, "chapter", profile->chapter, true))
    {
        return false;
    }

    if (('\0' != profile->photo_id[0]) &&
        !storage_append_json_string_field(json_buffer, json_buffer_size, offset, "photo_id", profile->photo_id, true))
    {
        return false;
    }

    if (profile->is_admin &&
        !storage_append_jsonf(json_buffer, json_buffer_size, offset, ",\"is_admin\":true"))
    {
        return false;
    }

    if (('\0' != profile->admin_pin[0]) &&
        !storage_append_json_string_field(json_buffer, json_buffer_size, offset, "admin_pin", profile->admin_pin, true))
    {
        return false;
    }

    if (profile->card_count > 0U)
    {
        storage_profile_cards_to_csv(profile, cards_csv, sizeof(cards_csv));
        if (!storage_append_jsonf(json_buffer, json_buffer_size, offset, ",\"uid\":") ||
            !storage_append_json_escaped_string(json_buffer, json_buffer_size, offset, profile->cards[0]) ||
            !storage_append_jsonf(json_buffer, json_buffer_size, offset, ",\"cards_csv\":") ||
            !storage_append_json_escaped_string(json_buffer, json_buffer_size, offset, cards_csv) ||
            !storage_append_jsonf(json_buffer, json_buffer_size, offset, ",\"cards\":["))
        {
            return false;
        }

        for (unsigned int card_index = 0U; card_index < profile->card_count; card_index++)
        {
            if (((card_index > 0U) &&
                 !storage_append_json_char(json_buffer, json_buffer_size, offset, ',')) ||
                !storage_append_json_escaped_string(json_buffer,
                                                    json_buffer_size,
                                                    offset,
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

static bool storage_profile_key_from_profile_locked(const user_t *profile, char *out_key, size_t out_size)
{
    if ((NULL == out_key) || (0U == out_size))
    {
        return false;
    }

    out_key[0] = '\0';
    if (NULL == profile)
    {
        return false;
    }

    for (unsigned int card_index = 0U; card_index < profile->card_count; card_index++)
    {
        storage_copy_clean_uid(out_key, out_size, profile->cards[card_index]);
        if ('\0' != out_key[0])
        {
            return true;
        }
    }

    return false;
}

static uint32_t storage_meeting_profile_key_hash_from_text(const char *key)
{
    uint32_t hash = 2166136261UL;

    if ((NULL == key) || ('\0' == key[0]))
    {
        return 0U;
    }

    while ('\0' != *key)
    {
        hash ^= (uint8_t) *key;
        hash *= 16777619UL;
        key++;
    }

    return (0U != hash) ? hash : 1U;
}

static bool storage_profile_key_hash_from_profile_locked(const user_t *profile, uint32_t *out_hash)
{
    char key[UID_MAX_LEN];

    if (NULL == out_hash)
    {
        return false;
    }

    *out_hash = 0U;
    if (!storage_profile_key_from_profile_locked(profile, key, sizeof(key)))
    {
        return false;
    }

    *out_hash = storage_meeting_profile_key_hash_from_text(key);
    return (0U != *out_hash);
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

static void storage_refresh_recent_user_locked(void)
{
    if (!g_recent_user_valid)
    {
        return;
    }

    for (unsigned int recent_card_index = 0U; recent_card_index < g_recent_user.card_count; recent_card_index++)
    {
        const char *recent_card = g_recent_user.cards[recent_card_index];

        if ('\0' == recent_card[0])
        {
            continue;
        }

        for (int user_index = 0; user_index < g_user_count; user_index++)
        {
            for (unsigned int card_index = 0U; card_index < g_users[user_index].card_count; card_index++)
            {
                if (0 == strcmp(g_users[user_index].cards[card_index], recent_card))
                {
                    storage_set_recent_user_locked(&g_users[user_index]);
                    return;
                }
            }
        }
    }

    storage_set_recent_user_locked(NULL);
}

static void storage_reset_users_io_snapshot(void)
{
    g_storage_users_io_count = 0;
    g_storage_recent_io_valid = false;
    memset(&g_storage_recent_io_snapshot, 0, sizeof(g_storage_recent_io_snapshot));
}

static void storage_capture_users_io_snapshot_locked(void)
{
    int count_to_copy = g_user_count;

    if (count_to_copy < 0)
    {
        count_to_copy = 0;
    }
    if (count_to_copy > STORAGE_MAX_USERS)
    {
        count_to_copy = STORAGE_MAX_USERS;
    }

    storage_reset_users_io_snapshot();
    g_storage_users_io_count = count_to_copy;
    if (g_recent_user_valid)
    {
        g_storage_recent_io_snapshot = g_recent_user;
        g_storage_recent_io_valid = true;
    }
}

static bool storage_copy_user_for_io(int index, user_t *out_profile)
{
    bool copied = false;

    if (NULL == out_profile)
    {
        return false;
    }

    storage_lock();
    if ((index >= 0) && (index < g_user_count) && (index < STORAGE_MAX_USERS))
    {
        *out_profile = g_users[index];
        copied = true;
    }
    storage_unlock();

    return copied;
}

static void storage_append_user_to_io_snapshot(const user_t *profile)
{
    if (NULL == profile)
    {
        return;
    }

    storage_lock();
    if (g_storage_users_io_count < STORAGE_MAX_USERS)
    {
        g_users[g_storage_users_io_count++] = *profile;
        g_user_count = g_storage_users_io_count;
        if (storage_profile_has_cards(profile))
        {
            g_storage_recent_io_snapshot = *profile;
            g_storage_recent_io_valid = true;
        }
    }
    storage_unlock();
}

static void storage_commit_users_io_snapshot_locked(void)
{
    int count_to_commit = g_storage_users_io_count;

    if (count_to_commit < 0)
    {
        count_to_commit = 0;
    }
    if (count_to_commit > STORAGE_MAX_USERS)
    {
        count_to_commit = STORAGE_MAX_USERS;
    }

    g_user_count = count_to_commit;

    if (g_storage_recent_io_valid)
    {
        g_recent_user = g_storage_recent_io_snapshot;
        g_recent_user_valid = true;
    }
    else
    {
        storage_set_recent_user_locked(NULL);
    }

    g_storage_loaded = true;
    g_storage_users_load_failed = false;
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
    memset(g_storage_meeting_mode_profile_indices, 0, sizeof(g_storage_meeting_mode_profile_indices));
    g_storage_meeting_mode_chapter[0] = '\0';
    g_storage_meeting_mode_active = false;
    g_storage_meeting_mode_selected_profiles = 0U;
    g_storage_meeting_mode_allowed_count = 0U;
    g_storage_meeting_mode_start_unix = 0U;
    g_storage_meeting_mode_end_unix = 0U;
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

static bool storage_copy_json_string_value(char *out,
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
    if (!storage_copy_json_string_value(out, out_size, start, object_end, &end))
    {
        out[0] = '\0';
        return false;
    }

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
    while ((cursor < object_end) && (count < STORAGE_MAX_CARDS_PER_USER))
    {
        char raw_card[UID_MAX_LEN];
        const char *end;

        cursor = storage_skip_whitespace(cursor, object_end);
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

        if (!storage_copy_json_string_value(raw_card, sizeof(raw_card), cursor + 1, object_end, &end))
        {
            break;
        }

        storage_copy_clean_uid(profile->cards[count], sizeof(profile->cards[count]), raw_card);
        if ('\0' != profile->cards[count][0])
        {
            count++;
        }

        cursor = end + 1;
    }

    profile->card_count = count;
}

static bool storage_parse_user_object_locked(const char *object_start, const char *object_end, user_t *out_profile)
{
    user_t profile;
    char legacy_uid[UID_MAX_LEN];
    char cards_csv[UID_MAX_LEN * STORAGE_MAX_CARDS_PER_USER];

    if ((NULL == object_start) || (NULL == object_end) || (NULL == out_profile) || (object_end <= object_start))
    {
        return false;
    }

    storage_clear_profile(&profile);
    memset(legacy_uid, 0, sizeof(legacy_uid));
    memset(cards_csv, 0, sizeof(cards_csv));

    if (!storage_extract_json_string(object_start, object_end, "name", profile.name, sizeof(profile.name)))
    {
        return false;
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

    if ('\0' == profile.name[0])
    {
        return false;
    }

    *out_profile = profile;
    return true;
}

static UINT storage_flush_users_json_buffer(size_t *buffer_len, size_t *json_size)
{
    UINT status;

    if ((NULL == buffer_len) || (NULL == json_size))
    {
        return FX_PTR_ERROR;
    }
    if (0U == *buffer_len)
    {
        return FX_SUCCESS;
    }

    status = fx_file_write(&g_users_file, (VOID *) g_storage_users_json_chunk, (ULONG) *buffer_len);
    if (FX_SUCCESS == status)
    {
        *json_size += *buffer_len;
        *buffer_len = 0U;
    }

    return status;
}

static UINT storage_write_users_json_fragment(const char *fragment,
                                              size_t fragment_size,
                                              size_t *buffer_len,
                                              size_t *json_size)
{
    if ((NULL == fragment) || (NULL == buffer_len) || (NULL == json_size))
    {
        return FX_PTR_ERROR;
    }

    while (fragment_size > 0U)
    {
        size_t buffer_space = sizeof(g_storage_users_json_chunk) - *buffer_len;
        size_t copy_size;

        if (0U == buffer_space)
        {
            UINT flush_status = storage_flush_users_json_buffer(buffer_len, json_size);
            if (FX_SUCCESS != flush_status)
            {
                return flush_status;
            }
            buffer_space = sizeof(g_storage_users_json_chunk);
        }

        copy_size = (fragment_size < buffer_space) ? fragment_size : buffer_space;
        memcpy(&g_storage_users_json_chunk[*buffer_len], fragment, copy_size);
        *buffer_len += copy_size;
        fragment += copy_size;
        fragment_size -= copy_size;
    }

    return FX_SUCCESS;
}

static bool storage_save_users_json_snapshot(size_t *out_json_size, ULONG *out_user_count)
{
    UINT status;
    bool file_opened = false;
    bool prepend_comma = false;
    size_t write_buffer_len = 0U;
    size_t json_size = 0U;
    int count_to_write = g_storage_users_io_count;
    ULONG start_tick = tx_time_get();

    if (count_to_write < 0)
    {
        count_to_write = 0;
    }
    if (count_to_write > STORAGE_MAX_USERS)
    {
        count_to_write = STORAGE_MAX_USERS;
    }

    if (NULL != out_json_size)
    {
        *out_json_size = 0U;
    }
    if (NULL != out_user_count)
    {
        *out_user_count = (ULONG) count_to_write;
    }

    g_storage_debug_last_stage = 10U;
    g_storage_debug_last_saved_bytes = 0U;

    storage_media_lock();
    if (!storage_media_open())
    {
        status = g_storage_debug_last_media_status;
        g_storage_debug_last_save_status = status;
        g_storage_debug_last_stage = 11U;
        storage_media_unlock();
        return false;
    }

    status = fx_file_open(&g_fx_media0, &g_users_file, USERS_FILE_NAME, FX_OPEN_FOR_WRITE);
    if (FX_SUCCESS != status)
    {
        status = fx_file_create(&g_fx_media0, USERS_FILE_NAME);
        if ((FX_SUCCESS != status) && (FX_ALREADY_CREATED != status))
        {
            (void) fx_media_close(&g_fx_media0);
            g_storage_media_ready = false;
            g_storage_debug_last_save_status = status;
            g_storage_debug_last_stage = 12U;
            storage_media_unlock();
            return false;
        }

        status = fx_file_open(&g_fx_media0, &g_users_file, USERS_FILE_NAME, FX_OPEN_FOR_WRITE);
        if (FX_SUCCESS != status)
        {
            (void) fx_media_close(&g_fx_media0);
            g_storage_media_ready = false;
            g_storage_debug_last_save_status = status;
            g_storage_debug_last_stage = 13U;
            storage_media_unlock();
            return false;
        }
    }

    if (FX_SUCCESS == status)
    {
        file_opened = true;
        status = fx_file_truncate(&g_users_file, 0U);
    }
    if (FX_SUCCESS == status)
    {
        status = fx_file_seek(&g_users_file, 0U);
    }
    if (FX_SUCCESS == status)
    {
        status = storage_write_users_json_fragment("[", 1U, &write_buffer_len, &json_size);
    }

    if ((FX_SUCCESS == status) && (0 == count_to_write) && g_storage_recent_io_valid)
    {
        size_t object_size = 0U;

        if (storage_append_user_json_locked(&g_storage_recent_io_snapshot,
                                            prepend_comma,
                                            g_storage_users_json_object,
                                            sizeof(g_storage_users_json_object),
                                            &object_size))
        {
            status = storage_write_users_json_fragment(g_storage_users_json_object,
                                                       object_size,
                                                       &write_buffer_len,
                                                       &json_size);
            if (FX_SUCCESS == status)
            {
                prepend_comma = true;
            }
        }
        else
        {
            status = FX_INVALID_NAME;
        }
    }

    for (int i = 0; (FX_SUCCESS == status) && (i < count_to_write); i++)
    {
        user_t profile;
        size_t object_size = 0U;

        if (!storage_copy_user_for_io(i, &profile))
        {
            status = FX_INVALID_NAME;
            break;
        }

        if (!storage_append_user_json_locked(&profile,
                                             prepend_comma,
                                             g_storage_users_json_object,
                                             sizeof(g_storage_users_json_object),
                                             &object_size))
        {
            status = FX_INVALID_NAME;
            break;
        }

        status = storage_write_users_json_fragment(g_storage_users_json_object,
                                                   object_size,
                                                   &write_buffer_len,
                                                   &json_size);
        if (FX_SUCCESS == status)
        {
            prepend_comma = true;
        }
    }

    if (FX_SUCCESS == status)
    {
        status = storage_write_users_json_fragment("]", 1U, &write_buffer_len, &json_size);
    }
    if (FX_SUCCESS == status)
    {
        status = storage_flush_users_json_buffer(&write_buffer_len, &json_size);
    }

    if (file_opened)
    {
        UINT close_status = fx_file_close(&g_users_file);
        if (FX_SUCCESS == status)
        {
            status = close_status;
        }
    }

    if (FX_SUCCESS == status)
    {
        status = fx_media_flush(&g_fx_media0);
    }

    if (FX_SUCCESS == status)
    {
        status = fx_media_close(&g_fx_media0);
        g_storage_media_ready = false;
    }
    else
    {
        (void) fx_media_close(&g_fx_media0);
        g_storage_media_ready = false;
    }

    storage_media_unlock();

    g_storage_debug_last_save_status = status;
    g_storage_debug_last_stage = 15U;
    g_storage_debug_last_saved_bytes = (ULONG) json_size;
    if (NULL != out_json_size)
    {
        *out_json_size = json_size;
    }
    app_metric_add_no_persist("Storage persist latency", "users.json", tx_time_get() - start_tick, (FX_SUCCESS == status));
    return (FX_SUCCESS == status);
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

    (void) storage_prune_access_logs_open_media();

    status = fx_file_open(&g_fx_media0, &g_access_log_file, ACCESS_LOG_FILE_NAME, FX_OPEN_FOR_WRITE);
    if (FX_SUCCESS == status)
    {
        file_opened = true;
        current_size = (ULONG) g_access_log_file.fx_file_current_file_size;
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
    ULONG current_size = 0U;
    ULONG start_tick = tx_time_get();

    if ((NULL == log_buffer) && (log_size > 0U))
    {
        return false;
    }

    storage_media_lock();
    if (!storage_media_open())
    {
        storage_media_unlock();
        return false;
    }

    (void) storage_prune_metrics_log_open_media();

    status = fx_file_open(&g_fx_media0, &g_metrics_file, METRICS_LOG_FILE_NAME, FX_OPEN_FOR_WRITE);
    if (FX_SUCCESS != status)
    {
        status = fx_file_create(&g_fx_media0, METRICS_LOG_FILE_NAME);
        if ((FX_SUCCESS != status) && (FX_ALREADY_CREATED != status))
        {
            (void) fx_media_close(&g_fx_media0);
            g_storage_media_ready = false;
            storage_media_unlock();
            app_metric_add_no_persist("Storage persist latency", "metrics.log", tx_time_get() - start_tick, false);
            return false;
        }

        status = fx_file_open(&g_fx_media0, &g_metrics_file, METRICS_LOG_FILE_NAME, FX_OPEN_FOR_WRITE);
        if (FX_SUCCESS != status)
        {
            (void) fx_media_close(&g_fx_media0);
            g_storage_media_ready = false;
            storage_media_unlock();
            app_metric_add_no_persist("Storage persist latency", "metrics.log", tx_time_get() - start_tick, false);
            return false;
        }
    }

    current_size = (ULONG) g_metrics_file.fx_file_current_file_size;
    status = fx_file_seek(&g_metrics_file, current_size);
    if ((FX_SUCCESS == status) && (log_size > 0U))
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

static bool storage_format_meeting_schedules_json(const storage_meeting_schedule_t *schedules,
                                                  int schedule_count,
                                                  char *out,
                                                  size_t out_size,
                                                  size_t *out_len)
{
    size_t offset = 0U;

    if ((NULL == out) || (0U == out_size) || (NULL == out_len) ||
        ((schedule_count > 0) && (NULL == schedules)))
    {
        return false;
    }

    out[0] = '\0';
    *out_len = 0U;

    if (!storage_appendf(out, out_size, &offset, "["))
    {
        return false;
    }

    for (int i = 0; i < schedule_count; i++)
    {
        unsigned int profile_count = schedules[i].profile_count;
        unsigned int profile_key_count = schedules[i].profile_key_count;

        if (profile_count > STORAGE_MAX_USERS)
        {
            profile_count = STORAGE_MAX_USERS;
        }
        if (profile_key_count > STORAGE_MAX_USERS)
        {
            profile_key_count = STORAGE_MAX_USERS;
        }

        if (!storage_appendf(out,
                             out_size,
                             &offset,
                             "%s{\"id\":%lu,\"start_unix\":%lu,\"end_unix\":%lu,\"recurrence\":%u,\"weekdays_mask\":%u",
                             (i > 0) ? "," : "",
                             (unsigned long) schedules[i].id,
                             (unsigned long) schedules[i].start_unix,
                             (unsigned long) schedules[i].end_unix,
                             (unsigned int) schedules[i].recurrence,
                             (unsigned int) schedules[i].weekdays_mask))
        {
            return false;
        }

        if (!storage_append_json_string_field(out,
                                              out_size,
                                              &offset,
                                              "meeting_chapter",
                                              schedules[i].meeting_chapter,
                                              true))
        {
            return false;
        }

        if (profile_key_count > 0U)
        {
            if (!storage_appendf(out, out_size, &offset, ",\"profile_key_hashes\":["))
            {
                return false;
            }

            for (unsigned int profile_index = 0U; profile_index < profile_key_count; profile_index++)
            {
                if (!storage_appendf(out,
                                     out_size,
                                     &offset,
                                     "%lu",
                                     (unsigned long) schedules[i].profile_key_hashes[profile_index]))
                {
                    return false;
                }
                if ((profile_index + 1U) < profile_key_count)
                {
                    if (!storage_appendf(out, out_size, &offset, ","))
                    {
                        return false;
                    }
                }
            }
        }
        else
        {
            if (!storage_appendf(out, out_size, &offset, ",\"profiles\":["))
            {
                return false;
            }

            for (unsigned int profile_index = 0U; profile_index < profile_count; profile_index++)
            {
                if (!storage_appendf(out,
                                     out_size,
                                     &offset,
                                     "%s%u",
                                     (profile_index > 0U) ? "," : "",
                                     (unsigned int) schedules[i].profile_indices[profile_index]))
                {
                    return false;
                }
            }
        }

        if (!storage_appendf(out, out_size, &offset, "]}"))
        {
            return false;
        }
    }

    if (!storage_appendf(out, out_size, &offset, "]"))
    {
        return false;
    }

    *out_len = offset;
    return true;
}

static bool storage_save_meeting_schedules_json(const storage_meeting_schedule_t *schedules, int schedule_count)
{
    UINT status;
    bool file_opened = false;
    size_t json_size = 0U;
    ULONG start_tick = tx_time_get();

    if ((schedule_count < 0) || (schedule_count > STORAGE_MEETING_SCHEDULE_MAX_ITEMS) ||
        !storage_format_meeting_schedules_json(schedules,
                                               schedule_count,
                                               g_storage_meeting_schedule_text,
                                               sizeof(g_storage_meeting_schedule_text),
                                               &json_size))
    {
        return false;
    }

    storage_media_lock();
    if (!storage_media_open())
    {
        storage_media_unlock();
        return false;
    }

    (void) fx_file_delete(&g_fx_media0, MEETING_SCHEDULE_TEMP_FILE_NAME);
    status = fx_file_create(&g_fx_media0, MEETING_SCHEDULE_TEMP_FILE_NAME);
    if ((FX_SUCCESS != status) && (FX_ALREADY_CREATED != status))
    {
        (void) fx_media_close(&g_fx_media0);
        g_storage_media_ready = false;
        storage_media_unlock();
        app_metric_add_no_persist("Storage persist latency", "meeting.json", tx_time_get() - start_tick, false);
        return false;
    }

    status = fx_file_open(&g_fx_media0, &g_meeting_schedule_file, MEETING_SCHEDULE_TEMP_FILE_NAME, FX_OPEN_FOR_WRITE);
    if (FX_SUCCESS == status)
    {
        file_opened = true;
        status = fx_file_truncate(&g_meeting_schedule_file, 0U);
    }
    if (FX_SUCCESS == status)
    {
        status = fx_file_seek(&g_meeting_schedule_file, 0U);
    }
    if (FX_SUCCESS == status)
    {
        status = fx_file_write(&g_meeting_schedule_file, (VOID *) g_storage_meeting_schedule_text, (ULONG) json_size);
    }

    if (file_opened)
    {
        UINT close_status = fx_file_close(&g_meeting_schedule_file);
        if (FX_SUCCESS == status)
        {
            status = close_status;
        }
    }

    if (FX_SUCCESS == status)
    {
        status = fx_media_flush(&g_fx_media0);
    }

    if (FX_SUCCESS == status)
    {
        bool old_file_renamed = false;
        UINT rename_status;

        (void) fx_file_delete(&g_fx_media0, MEETING_SCHEDULE_BACKUP_FILE_NAME);
        rename_status = fx_file_rename(&g_fx_media0, MEETING_SCHEDULE_FILE_NAME, MEETING_SCHEDULE_BACKUP_FILE_NAME);
        if (FX_SUCCESS == rename_status)
        {
            old_file_renamed = true;
        }
        else
        {
            FX_FILE probe_file;
            if (FX_SUCCESS == fx_file_open(&g_fx_media0, &probe_file, MEETING_SCHEDULE_FILE_NAME, FX_OPEN_FOR_READ))
            {
                (void) fx_file_close(&probe_file);
                status = rename_status;
            }
        }

        if (FX_SUCCESS == status)
        {
            status = fx_file_rename(&g_fx_media0, MEETING_SCHEDULE_TEMP_FILE_NAME, MEETING_SCHEDULE_FILE_NAME);
            if ((FX_SUCCESS != status) && old_file_renamed)
            {
                (void) fx_file_rename(&g_fx_media0, MEETING_SCHEDULE_BACKUP_FILE_NAME, MEETING_SCHEDULE_FILE_NAME);
            }
        }

        if (FX_SUCCESS == status)
        {
            (void) fx_file_delete(&g_fx_media0, MEETING_SCHEDULE_BACKUP_FILE_NAME);
            status = fx_media_flush(&g_fx_media0);
        }
    }

    if (FX_SUCCESS == status)
    {
        status = fx_media_close(&g_fx_media0);
        g_storage_media_ready = false;
    }
    else
    {
        (void) fx_file_delete(&g_fx_media0, MEETING_SCHEDULE_TEMP_FILE_NAME);
        (void) fx_media_close(&g_fx_media0);
        g_storage_media_ready = false;
    }

    storage_media_unlock();
    app_metric_add_no_persist("Storage persist latency", "meeting.json", tx_time_get() - start_tick, (FX_SUCCESS == status));
    return (FX_SUCCESS == status);
}

static bool storage_format_meeting_active_json(const storage_meeting_active_t *state,
                                               char *out,
                                               size_t out_size,
                                               size_t *out_len)
{
    size_t offset = 0U;
    unsigned int profile_count;
    unsigned int profile_key_count;

    if ((NULL == state) || (NULL == out) || (0U == out_size) || (NULL == out_len) ||
        (0U == state->start_unix) || (state->end_unix <= state->start_unix) ||
        (((0U == state->profile_count) || (state->profile_count > STORAGE_MAX_USERS)) &&
         ((0U == state->profile_key_count) || (state->profile_key_count > STORAGE_MAX_USERS))))
    {
        return false;
    }

    out[0] = '\0';
    *out_len = 0U;
    profile_count = state->profile_count;
    profile_key_count = state->profile_key_count;
    if (profile_count > STORAGE_MAX_USERS)
    {
        profile_count = STORAGE_MAX_USERS;
    }
    if (profile_key_count > STORAGE_MAX_USERS)
    {
        profile_key_count = STORAGE_MAX_USERS;
    }

    if (!storage_appendf(out,
                         out_size,
                         &offset,
                         "{\"start_unix\":%lu,\"end_unix\":%lu",
                         (unsigned long) state->start_unix,
                         (unsigned long) state->end_unix))
    {
        return false;
    }

    if (!storage_append_json_string_field(out,
                                          out_size,
                                          &offset,
                                          "meeting_chapter",
                                          state->meeting_chapter,
                                          true))
    {
        return false;
    }

    if (profile_key_count > 0U)
    {
        if (!storage_appendf(out, out_size, &offset, ",\"profile_key_hashes\":["))
        {
            return false;
        }

        for (unsigned int profile_index = 0U; profile_index < profile_key_count; profile_index++)
        {
            if (!storage_appendf(out,
                                 out_size,
                                 &offset,
                                 "%lu",
                                 (unsigned long) state->profile_key_hashes[profile_index]))
            {
                return false;
            }
            if ((profile_index + 1U) < profile_key_count)
            {
                if (!storage_appendf(out, out_size, &offset, ","))
                {
                    return false;
                }
            }
        }
    }
    else
    {
        if (!storage_appendf(out, out_size, &offset, ",\"profiles\":["))
        {
            return false;
        }

        for (unsigned int profile_index = 0U; profile_index < profile_count; profile_index++)
        {
            if (!storage_appendf(out,
                                 out_size,
                                 &offset,
                                 "%s%u",
                                 (profile_index > 0U) ? "," : "",
                                 (unsigned int) state->profile_indices[profile_index]))
            {
                return false;
            }
        }
    }

    if (!storage_appendf(out, out_size, &offset, "]}"))
    {
        return false;
    }

    *out_len = offset;
    return true;
}

static bool storage_save_meeting_active_json(const storage_meeting_active_t *state)
{
    UINT status;
    bool file_opened = false;
    size_t json_size = 0U;
    ULONG start_tick = tx_time_get();

    if (!storage_format_meeting_active_json(state,
                                            g_storage_meeting_schedule_text,
                                            sizeof(g_storage_meeting_schedule_text),
                                            &json_size))
    {
        return false;
    }

    storage_media_lock();
    if (!storage_media_open())
    {
        storage_media_unlock();
        return false;
    }

    (void) fx_file_delete(&g_fx_media0, MEETING_ACTIVE_TEMP_FILE_NAME);
    status = fx_file_create(&g_fx_media0, MEETING_ACTIVE_TEMP_FILE_NAME);
    if ((FX_SUCCESS != status) && (FX_ALREADY_CREATED != status))
    {
        (void) fx_media_close(&g_fx_media0);
        g_storage_media_ready = false;
        storage_media_unlock();
        app_metric_add_no_persist("Storage persist latency", "meeting_active.json", tx_time_get() - start_tick, false);
        return false;
    }

    status = fx_file_open(&g_fx_media0, &g_meeting_active_file, MEETING_ACTIVE_TEMP_FILE_NAME, FX_OPEN_FOR_WRITE);
    if (FX_SUCCESS == status)
    {
        file_opened = true;
        status = fx_file_truncate(&g_meeting_active_file, 0U);
    }
    if (FX_SUCCESS == status)
    {
        status = fx_file_seek(&g_meeting_active_file, 0U);
    }
    if (FX_SUCCESS == status)
    {
        status = fx_file_write(&g_meeting_active_file, (VOID *) g_storage_meeting_schedule_text, (ULONG) json_size);
    }

    if (file_opened)
    {
        UINT close_status = fx_file_close(&g_meeting_active_file);
        if (FX_SUCCESS == status)
        {
            status = close_status;
        }
    }

    if (FX_SUCCESS == status)
    {
        status = fx_media_flush(&g_fx_media0);
    }

    if (FX_SUCCESS == status)
    {
        bool old_file_renamed = false;
        UINT rename_status;

        (void) fx_file_delete(&g_fx_media0, MEETING_ACTIVE_BACKUP_FILE_NAME);
        rename_status = fx_file_rename(&g_fx_media0, MEETING_ACTIVE_FILE_NAME, MEETING_ACTIVE_BACKUP_FILE_NAME);
        if (FX_SUCCESS == rename_status)
        {
            old_file_renamed = true;
        }
        else
        {
            FX_FILE probe_file;
            if (FX_SUCCESS == fx_file_open(&g_fx_media0, &probe_file, MEETING_ACTIVE_FILE_NAME, FX_OPEN_FOR_READ))
            {
                (void) fx_file_close(&probe_file);
                status = rename_status;
            }
        }

        if (FX_SUCCESS == status)
        {
            status = fx_file_rename(&g_fx_media0, MEETING_ACTIVE_TEMP_FILE_NAME, MEETING_ACTIVE_FILE_NAME);
            if ((FX_SUCCESS != status) && old_file_renamed)
            {
                (void) fx_file_rename(&g_fx_media0, MEETING_ACTIVE_BACKUP_FILE_NAME, MEETING_ACTIVE_FILE_NAME);
            }
        }

        if (FX_SUCCESS == status)
        {
            (void) fx_file_delete(&g_fx_media0, MEETING_ACTIVE_BACKUP_FILE_NAME);
            status = fx_media_flush(&g_fx_media0);
        }
    }

    if (FX_SUCCESS == status)
    {
        status = fx_media_close(&g_fx_media0);
        g_storage_media_ready = false;
    }
    else
    {
        (void) fx_file_delete(&g_fx_media0, MEETING_ACTIVE_TEMP_FILE_NAME);
        (void) fx_media_close(&g_fx_media0);
        g_storage_media_ready = false;
    }

    storage_media_unlock();
    app_metric_add_no_persist("Storage persist latency", "meeting_active.json", tx_time_get() - start_tick, (FX_SUCCESS == status));
    return (FX_SUCCESS == status);
}

static bool storage_clear_meeting_active_file(void)
{
    UINT status = FX_SUCCESS;
    ULONG start_tick = tx_time_get();

    storage_media_lock();
    if (!storage_media_open())
    {
        storage_media_unlock();
        return false;
    }

    (void) fx_file_delete(&g_fx_media0, MEETING_ACTIVE_TEMP_FILE_NAME);
    (void) fx_file_delete(&g_fx_media0, MEETING_ACTIVE_BACKUP_FILE_NAME);
    (void) fx_file_delete(&g_fx_media0, MEETING_ACTIVE_FILE_NAME);
    status = fx_media_flush(&g_fx_media0);
    if (FX_SUCCESS == status)
    {
        status = fx_media_close(&g_fx_media0);
        g_storage_media_ready = false;
    }
    else
    {
        (void) fx_media_close(&g_fx_media0);
        g_storage_media_ready = false;
    }

    storage_media_unlock();
    app_metric_add_no_persist("Storage persist latency", "meeting_active.json", tx_time_get() - start_tick, (FX_SUCCESS == status));
    return (FX_SUCCESS == status);
}

static const char *storage_meeting_json_find_key(const char *object_start, const char *object_end, const char *key)
{
    char pattern[40];
    const char *found;

    if ((NULL == object_start) || (NULL == object_end) || (NULL == key))
    {
        return NULL;
    }

    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    found = strstr(object_start, pattern);
    if ((NULL == found) || (found >= object_end))
    {
        return NULL;
    }

    return found + strlen(pattern);
}

static bool storage_meeting_json_get_ulong(const char *object_start,
                                           const char *object_end,
                                           const char *key,
                                           ULONG *out_value)
{
    const char *found;
    const char *colon;
    const char *cursor;
    char *end_ptr;
    unsigned long value;

    if ((NULL == object_start) || (NULL == object_end) || (NULL == key) || (NULL == out_value))
    {
        return false;
    }

    found = storage_meeting_json_find_key(object_start, object_end, key);
    if (NULL == found)
    {
        return false;
    }

    colon = found;
    while ((colon < object_end) && isspace((unsigned char) *colon))
    {
        colon++;
    }
    if ((colon >= object_end) || (':' != *colon))
    {
        return false;
    }

    cursor = colon + 1;
    while ((cursor < object_end) && isspace((unsigned char) *cursor))
    {
        cursor++;
    }
    if ((cursor >= object_end) || ('-' == *cursor))
    {
        return false;
    }

    value = strtoul(cursor, &end_ptr, 10);
    if ((end_ptr == cursor) || (end_ptr > object_end))
    {
        return false;
    }

    *out_value = (ULONG) value;
    return true;
}

static bool storage_meeting_json_get_profiles(const char *object_start,
                                              const char *object_end,
                                              storage_meeting_schedule_t *schedule)
{
    const char *found;
    const char *colon;
    const char *cursor;
    unsigned int profile_count = 0U;

    if ((NULL == object_start) || (NULL == object_end) || (NULL == schedule))
    {
        return false;
    }

    found = storage_meeting_json_find_key(object_start, object_end, "profiles");
    if (NULL == found)
    {
        return false;
    }

    colon = found;
    while ((colon < object_end) && isspace((unsigned char) *colon))
    {
        colon++;
    }
    if ((colon >= object_end) || (':' != *colon))
    {
        return false;
    }

    cursor = colon + 1;
    while ((cursor < object_end) && isspace((unsigned char) *cursor))
    {
        cursor++;
    }
    if ((cursor >= object_end) || ('[' != *cursor))
    {
        return false;
    }

    cursor++;
    while (cursor < object_end)
    {
        char *end_ptr;
        long value;

        while ((cursor < object_end) && (isspace((unsigned char) *cursor) || (',' == *cursor)))
        {
            cursor++;
        }

        if ((cursor >= object_end) || (']' == *cursor))
        {
            break;
        }
        if ('-' == *cursor)
        {
            return false;
        }

        value = strtol(cursor, &end_ptr, 10);
        if ((end_ptr == cursor) || (end_ptr > object_end) ||
            (value < 0L) || (value >= (long) STORAGE_MAX_USERS) ||
            (profile_count >= STORAGE_MAX_USERS))
        {
            return false;
        }

        schedule->profile_indices[profile_count++] = (uint8_t) value;
        cursor = end_ptr;
    }

    schedule->profile_count = profile_count;
    return (profile_count > 0U);
}

static bool storage_meeting_json_get_profile_key_hashes(const char *object_start,
                                                        const char *object_end,
                                                        uint32_t *hashes,
                                                        unsigned int *key_count)
{
    const char *found;
    const char *colon;
    const char *cursor;
    unsigned int count = 0U;
    bool legacy_string_keys = false;

    if ((NULL == object_start) || (NULL == object_end) || (NULL == hashes) || (NULL == key_count))
    {
        return false;
    }

    *key_count = 0U;
    found = storage_meeting_json_find_key(object_start, object_end, "profile_key_hashes");
    if (NULL == found)
    {
        found = storage_meeting_json_find_key(object_start, object_end, "profile_keys");
        legacy_string_keys = (NULL != found);
    }
    if (NULL == found)
    {
        return false;
    }

    colon = found;
    while ((colon < object_end) && isspace((unsigned char) *colon))
    {
        colon++;
    }
    if ((colon >= object_end) || (':' != *colon))
    {
        return false;
    }

    cursor = colon + 1;
    while ((cursor < object_end) && isspace((unsigned char) *cursor))
    {
        cursor++;
    }
    if ((cursor >= object_end) || ('[' != *cursor))
    {
        return false;
    }

    cursor++;
    while (cursor < object_end)
    {
        while ((cursor < object_end) && (isspace((unsigned char) *cursor) || (',' == *cursor)))
        {
            cursor++;
        }

        if ((cursor >= object_end) || (']' == *cursor))
        {
            break;
        }
        if (count >= STORAGE_MAX_USERS)
        {
            return false;
        }

        if (legacy_string_keys)
        {
            char raw_key[UID_MAX_LEN];
            char cleaned_key[UID_MAX_LEN];
            size_t raw_len = 0U;

            if ('"' != *cursor)
            {
                return false;
            }

            cursor++;
            while ((cursor < object_end) && ('"' != *cursor))
            {
                if ('\\' == *cursor)
                {
                    cursor++;
                    if (cursor >= object_end)
                    {
                        return false;
                    }
                }
                if ((raw_len + 1U) < sizeof(raw_key))
                {
                    raw_key[raw_len++] = *cursor;
                }
                cursor++;
            }
            if ((cursor >= object_end) || ('"' != *cursor))
            {
                return false;
            }
            raw_key[raw_len] = '\0';
            storage_copy_clean_uid(cleaned_key, sizeof(cleaned_key), raw_key);
            if ('\0' != cleaned_key[0])
            {
                hashes[count] = storage_meeting_profile_key_hash_from_text(cleaned_key);
                if (0U != hashes[count])
                {
                    count++;
                }
            }
            cursor++;
        }
        else
        {
            char *end_ptr;
            unsigned long value;

            if ('"' == *cursor)
            {
                char raw_hash[16];
                size_t raw_len = 0U;

                cursor++;
                while ((cursor < object_end) && ('"' != *cursor))
                {
                    if ((raw_len + 1U) < sizeof(raw_hash))
                    {
                        raw_hash[raw_len++] = *cursor;
                    }
                    cursor++;
                }
                if ((cursor >= object_end) || ('"' != *cursor))
                {
                    return false;
                }
                raw_hash[raw_len] = '\0';
                value = strtoul(raw_hash, &end_ptr, 10);
                cursor++;
            }
            else
            {
                value = strtoul(cursor, &end_ptr, 10);
                if (end_ptr == cursor)
                {
                    return false;
                }
                cursor = end_ptr;
            }

            if ((0UL == value) || (value > 0xFFFFFFFFUL))
            {
                return false;
            }
            hashes[count++] = (uint32_t) value;
        }
    }

    *key_count = count;
    return (count > 0U);
}

static const char *storage_meeting_json_object_end(const char *object_start, const char *limit)
{
    const char *cursor;

    if ((NULL == object_start) || (NULL == limit) || (object_start >= limit) || ('{' != *object_start))
    {
        return NULL;
    }

    cursor = object_start + 1;
    while (cursor < limit)
    {
        if ('}' == *cursor)
        {
            return cursor;
        }
        cursor++;
    }

    return NULL;
}

static int storage_parse_meeting_active_json(const char *json_text, storage_meeting_active_t *out_state)
{
    const char *object_start;
    const char *object_end;
    storage_meeting_schedule_t schedule;

    if ((NULL == json_text) || (NULL == out_state))
    {
        return -1;
    }

    object_start = strchr(json_text, '{');
    if (NULL == object_start)
    {
        return 0;
    }

    object_end = storage_meeting_json_object_end(object_start, json_text + strlen(json_text));
    if (NULL == object_end)
    {
        return -1;
    }

    memset(&schedule, 0, sizeof(schedule));
    if (!storage_meeting_json_get_ulong(object_start, object_end, "start_unix", &schedule.start_unix) ||
        !storage_meeting_json_get_ulong(object_start, object_end, "end_unix", &schedule.end_unix) ||
        (schedule.end_unix <= schedule.start_unix))
    {
        return -1;
    }
    if (!storage_meeting_json_get_profile_key_hashes(object_start,
                                                     object_end,
                                                     schedule.profile_key_hashes,
                                                     &schedule.profile_key_count) &&
        !storage_meeting_json_get_profiles(object_start, object_end, &schedule))
    {
        return -1;
    }

    memset(out_state, 0, sizeof(*out_state));
    out_state->start_unix = schedule.start_unix;
    out_state->end_unix = schedule.end_unix;
    out_state->profile_count = schedule.profile_count;
    out_state->profile_key_count = schedule.profile_key_count;
    for (unsigned int i = 0U; i < schedule.profile_count; i++)
    {
        out_state->profile_indices[i] = schedule.profile_indices[i];
    }
    for (unsigned int i = 0U; i < schedule.profile_key_count; i++)
    {
        out_state->profile_key_hashes[i] = schedule.profile_key_hashes[i];
    }

    if (!storage_extract_json_string(object_start,
                                     object_end,
                                     "meeting_chapter",
                                     out_state->meeting_chapter,
                                     sizeof(out_state->meeting_chapter)))
    {
        (void) storage_extract_json_string(object_start,
                                           object_end,
                                           "chapter",
                                           out_state->meeting_chapter,
                                           sizeof(out_state->meeting_chapter));
    }

    return 1;
}

static int storage_parse_meeting_schedules_json(const char *json_text,
                                                storage_meeting_schedule_t *out_schedules,
                                                int max_schedules)
{
    const char *cursor;
    const char *limit;
    int count = 0;

    if ((NULL == json_text) || (NULL == out_schedules) || (max_schedules <= 0))
    {
        return -1;
    }

    cursor = json_text;
    limit = json_text + strlen(json_text);
    while ((cursor < limit) && (count < max_schedules))
    {
        const char *object_start = strchr(cursor, '{');
        const char *object_end;
        storage_meeting_schedule_t schedule;

        if ((NULL == object_start) || (object_start >= limit))
        {
            break;
        }

        object_end = storage_meeting_json_object_end(object_start, limit);
        if (NULL == object_end)
        {
            return -1;
        }

        memset(&schedule, 0, sizeof(schedule));
        if (storage_meeting_json_get_ulong(object_start, object_end, "id", &schedule.id) &&
            storage_meeting_json_get_ulong(object_start, object_end, "start_unix", &schedule.start_unix) &&
            (storage_meeting_json_get_profile_key_hashes(object_start,
                                                         object_end,
                                                         schedule.profile_key_hashes,
                                                         &schedule.profile_key_count) ||
             storage_meeting_json_get_profiles(object_start, object_end, &schedule)))
        {
            ULONG value = 0U;
            ULONG duration = STORAGE_MEETING_DEFAULT_DURATION_SECONDS;

            if (!storage_meeting_json_get_ulong(object_start, object_end, "end_unix", &schedule.end_unix) ||
                (schedule.end_unix <= schedule.start_unix))
            {
                schedule.end_unix = schedule.start_unix + duration;
            }

            if (!storage_extract_json_string(object_start,
                                             object_end,
                                             "meeting_chapter",
                                             schedule.meeting_chapter,
                                             sizeof(schedule.meeting_chapter)))
            {
                (void) storage_extract_json_string(object_start,
                                                   object_end,
                                                   "chapter",
                                                   schedule.meeting_chapter,
                                                   sizeof(schedule.meeting_chapter));
            }

            if (storage_meeting_json_get_ulong(object_start, object_end, "recurrence", &value) &&
                (value <= STORAGE_MEETING_RECURRENCE_WEEKLY))
            {
                schedule.recurrence = (uint8_t) value;
            }

            value = 0U;
            if (storage_meeting_json_get_ulong(object_start, object_end, "weekdays_mask", &value))
            {
                schedule.weekdays_mask = (uint8_t) (value & STORAGE_MEETING_WEEKDAY_MASK_ALL);
            }

            if ((STORAGE_MEETING_RECURRENCE_WEEKLY == schedule.recurrence) &&
                (0U == (schedule.weekdays_mask & STORAGE_MEETING_WEEKDAY_MASK_ALL)))
            {
                schedule.weekdays_mask = (uint8_t) (1U << (uint8_t) (((schedule.start_unix / 86400UL) + 4UL) % 7UL));
            }

            out_schedules[count++] = schedule;
        }

        cursor = object_end + 1;
    }

    return count;
}

static unsigned int storage_parse_access_log_text_into(const char *log_buffer,
                                                       app_access_log_entry_t *entries,
                                                       unsigned int max_entries,
                                                       unsigned int *entry_count)
{
    const char *cursor = log_buffer;
    unsigned int parsed_count = 0U;
    ULONG now_unix_utc = 0U;
    bool time_synced = false;

    if ((NULL == log_buffer) || (NULL == entries) || (NULL == entry_count) || (0U == max_entries))
    {
        return 0U;
    }

    time_synced = app_time_get_utc(&now_unix_utc);

    while ('\0' != *cursor)
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
                    (type_value <= (long) EVENT_CARD_REGISTRATION_FAILED) &&
                    ((!time_synced) || storage_history_entry_is_retained((ULONG) unix_utc, now_unix_utc)))
                {
                    if (*entry_count >= max_entries)
                    {
                        memmove(entries,
                                &entries[1],
                                (max_entries - 1U) * sizeof(entries[0]));
                        *entry_count = max_entries - 1U;
                    }

                    entries[*entry_count].tick = 0U;
                    entries[*entry_count].unix_utc = (ULONG) unix_utc;
                    entries[*entry_count].type = (app_event_type_t) type_value;
                    storage_copy_text(entries[*entry_count].data,
                                      sizeof(entries[*entry_count].data),
                                      second_sep + 1,
                                      strlen(second_sep + 1));
                    storage_copy_text(entries[*entry_count].user,
                                      sizeof(entries[*entry_count].user),
                                      third_sep + 1,
                                      strlen(third_sep + 1));
                    (*entry_count)++;
                    parsed_count++;
                }
            }
        }

        cursor = ('\0' != *line_end) ? (line_end + 1) : line_end;
    }

    return parsed_count;
}

static bool storage_load_access_log_tail_file(const char *filename,
                                              char *log_buffer,
                                              size_t log_buffer_size,
                                              app_access_log_entry_t *entries,
                                              unsigned int max_entries,
                                              unsigned int *entry_count)
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

    return (storage_parse_access_log_text_into(parse_cursor, entries, max_entries, entry_count) > 0U);
}

static bool storage_parse_metrics_text(const char *log_buffer, bool *out_pruned)
{
    static app_metric_entry_t entries[APP_METRIC_LOG_SIZE];
    int entry_count = 0;
    const char *cursor = log_buffer;
    ULONG now_unix_utc = 0U;
    bool time_synced = false;

    if (NULL == log_buffer)
    {
        return false;
    }

    if (NULL != out_pruned)
    {
        *out_pruned = false;
    }

    time_synced = app_time_get_utc(&now_unix_utc);
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
                        if (time_synced && !storage_history_entry_is_retained((ULONG) unix_utc, now_unix_utc))
                        {
                            if (NULL != out_pruned)
                            {
                                *out_pruned = true;
                            }
                        }
                        else
                        {
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

static bool storage_load_users_file(const char *filename, bool commit_profiles)
{
    typedef enum
    {
        STORAGE_JSON_BEFORE_ARRAY = 0,
        STORAGE_JSON_EXPECT_FIRST_VALUE_OR_END,
        STORAGE_JSON_EXPECT_VALUE,
        STORAGE_JSON_EXPECT_COMMA_OR_END,
        STORAGE_JSON_AFTER_ARRAY
    } storage_users_json_state_t;

    UINT status;
    ULONG actual_bytes = 0U;
    ULONG bytes_left = 0U;
    ULONG total_read = 0U;
    bool object_active = false;
    bool in_string = false;
    bool escaped = false;
    bool file_opened = false;
    bool saw_array_open = false;
    bool saw_array_close = false;
    bool object_overflow = false;
    unsigned int brace_depth = 0U;
    unsigned int parsed_profiles = 0U;
    size_t object_offset = 0U;
    storage_users_json_state_t json_state = STORAGE_JSON_BEFORE_ARRAY;

    g_storage_debug_last_stage = 20U;
    g_storage_debug_last_read_bytes = 0U;

    if (!storage_media_access_allowed())
    {
        g_storage_debug_last_stage = 22U;
        g_storage_users_load_failed = true;
        return false;
    }

    storage_media_lock();
    if (!storage_media_open())
    {
        storage_media_unlock();
        g_storage_debug_last_stage = 23U;
        g_storage_users_load_failed = true;
        return false;
    }

    status = fx_file_open(&g_fx_media0, &g_users_file, (CHAR *) filename, FX_OPEN_FOR_READ);
    g_storage_debug_last_load_status = status;
    if (FX_SUCCESS == status)
    {
        file_opened = true;
        bytes_left = (ULONG) g_users_file.fx_file_current_file_size;
        if (commit_profiles)
        {
            storage_lock();
            storage_reset_users_io_snapshot();
            memset(g_users, 0, sizeof(g_users));
            g_user_count = 0;
            g_recent_user_valid = false;
            storage_unlock();
        }

        while ((FX_SUCCESS == status) && (bytes_left > 0U))
        {
            ULONG request_size = bytes_left;

            if (request_size > (ULONG) sizeof(g_storage_users_json_chunk))
            {
                request_size = (ULONG) sizeof(g_storage_users_json_chunk);
            }

            status = fx_file_read(&g_users_file, g_storage_users_json_chunk, request_size, &actual_bytes);
            if (FX_SUCCESS != status)
            {
                break;
            }
            if (0U == actual_bytes)
            {
                status = FX_INVALID_NAME;
                break;
            }

            for (ULONG byte_index = 0U; byte_index < actual_bytes; byte_index++)
            {
                char ch = g_storage_users_json_chunk[byte_index];
                bool object_started = false;

                if (!object_active)
                {
                    if (isspace((unsigned char) ch))
                    {
                        continue;
                    }

                    if (STORAGE_JSON_BEFORE_ARRAY == json_state)
                    {
                        if ('[' == ch)
                        {
                            saw_array_open = true;
                            json_state = STORAGE_JSON_EXPECT_FIRST_VALUE_OR_END;
                            continue;
                        }

                        status = FX_INVALID_NAME;
                        break;
                    }

                    if (STORAGE_JSON_AFTER_ARRAY == json_state)
                    {
                        status = FX_INVALID_NAME;
                        break;
                    }

                    if (STORAGE_JSON_EXPECT_COMMA_OR_END == json_state)
                    {
                        if (',' == ch)
                        {
                            json_state = STORAGE_JSON_EXPECT_VALUE;
                            continue;
                        }
                        if (']' == ch)
                        {
                            saw_array_close = true;
                            json_state = STORAGE_JSON_AFTER_ARRAY;
                            continue;
                        }

                        status = FX_INVALID_NAME;
                        break;
                    }

                    if ((STORAGE_JSON_EXPECT_FIRST_VALUE_OR_END == json_state) && (']' == ch))
                    {
                        saw_array_close = true;
                        json_state = STORAGE_JSON_AFTER_ARRAY;
                        continue;
                    }

                    if ((STORAGE_JSON_EXPECT_FIRST_VALUE_OR_END != json_state) &&
                        (STORAGE_JSON_EXPECT_VALUE != json_state))
                    {
                        status = FX_INVALID_NAME;
                        break;
                    }

                    if ('{' != ch)
                    {
                        status = FX_INVALID_NAME;
                        break;
                    }
                    object_active = true;
                    in_string = false;
                    escaped = false;
                    object_overflow = false;
                    brace_depth = 1U;
                    object_offset = 0U;
                    object_started = true;
                }

                if (object_offset + 1U < sizeof(g_storage_users_json_object))
                {
                    g_storage_users_json_object[object_offset++] = ch;
                }
                else
                {
                    object_overflow = true;
                }

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
                    continue;
                }

                if (('{' == ch) && !object_started)
                {
                    if (brace_depth < 0xFFFFU)
                    {
                        brace_depth++;
                    }
                }
                else if ('}' == ch)
                {
                    if (brace_depth > 0U)
                    {
                        brace_depth--;
                    }

                    if (0U == brace_depth)
                    {
                        user_t profile;

                        if (object_overflow)
                        {
                            status = FX_INVALID_NAME;
                            break;
                        }

                        g_storage_users_json_object[object_offset] = '\0';
                        if (storage_parse_user_object_locked(g_storage_users_json_object,
                                                             &g_storage_users_json_object[object_offset - 1U],
                                                             &profile))
                        {
                            parsed_profiles++;
                            if (commit_profiles)
                            {
                                storage_append_user_to_io_snapshot(&profile);
                            }
                        }
                        else
                        {
                            status = FX_INVALID_NAME;
                            break;
                        }

                        object_active = false;
                        object_offset = 0U;
                        json_state = STORAGE_JSON_EXPECT_COMMA_OR_END;
                    }
                }
            }

            total_read += actual_bytes;
            bytes_left -= actual_bytes;
        }

        if ((FX_SUCCESS == status) &&
            (object_active || !saw_array_open || !saw_array_close ||
             (STORAGE_JSON_AFTER_ARRAY != json_state)))
        {
            status = FX_INVALID_NAME;
        }

        fx_file_close(&g_users_file);
        g_storage_debug_last_load_status = status;
        {
            UINT close_status = fx_media_close(&g_fx_media0);
            if (FX_SUCCESS == status)
            {
                status = close_status;
                g_storage_debug_last_load_status = status;
            }
        }
        g_storage_media_ready = false;
    }
    else
    {
        (void) fx_media_close(&g_fx_media0);
        g_storage_media_ready = false;
    }
    storage_media_unlock();

    if ((FX_SUCCESS == status) && file_opened && commit_profiles)
    {
        storage_lock();
        storage_commit_users_io_snapshot_locked();
        storage_unlock();
    }

    g_storage_debug_last_user_count = commit_profiles ? (ULONG) g_storage_users_io_count : (ULONG) parsed_profiles;
    g_storage_debug_last_read_bytes = total_read;
    g_storage_debug_last_stage = 25U;
    if ((FX_SUCCESS == status) && file_opened)
    {
        g_storage_users_load_failed = false;
    }
    else if ((FX_SUCCESS != status) && file_opened && (commit_profiles || !g_storage_loaded))
    {
        g_storage_users_load_failed = true;
    }
    return (FX_SUCCESS == status);
}

static bool storage_load_users_stream(void)
{
    bool main_file_valid;
    bool committed = false;

    g_storage_users_load_failed = false;
    main_file_valid = storage_load_users_file(USERS_FILE_NAME, false);
    if (!main_file_valid &&
        (22U != g_storage_debug_last_stage) &&
        (23U != g_storage_debug_last_stage))
    {
        bool backup_file_valid = storage_load_users_file(USERS_BACKUP_FILE_NAME, false);

        if (backup_file_valid)
        {
            committed = storage_load_users_file(USERS_BACKUP_FILE_NAME, true);
        }
    }
    else if (main_file_valid)
    {
        committed = storage_load_users_file(USERS_FILE_NAME, true);
    }

    if (committed)
    {
        return true;
    }

    if ((22U == g_storage_debug_last_stage) || (23U == g_storage_debug_last_stage))
    {
        return false;
    }

    storage_lock();
    if (!g_storage_loaded && !main_file_valid)
    {
        g_user_count = 0;
        g_recent_user_valid = false;
        memset(g_users, 0, sizeof(g_users));
        g_storage_loaded = true;
        g_storage_users_load_failed = false;
        g_storage_debug_last_user_count = 0U;
    }
    committed = g_storage_loaded;
    storage_unlock();
    return committed;
}

static void storage_load_users_locked(void)
{
    if (g_storage_loaded)
    {
        g_storage_debug_last_stage = 21U;
        return;
    }

    storage_unlock();
    (void) storage_load_users_now();
    storage_lock();
}

static bool storage_load_users_now(void)
{
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

    storage_io_lock();
    storage_lock();
    if (g_storage_loaded)
    {
        g_storage_debug_last_stage = 21U;
        loaded = true;
        storage_unlock();
        storage_io_unlock();
        return true;
    }
    storage_unlock();

    (void) storage_load_users_stream();
    storage_lock();
    loaded = g_storage_loaded;
    storage_unlock();
    storage_io_unlock();

    app_metric_add_no_persist("Storage load latency", "users.json", tx_time_get() - start_tick, loaded);
    app_metric_add_no_persist("Reboot persistence", "apos reboot", tx_time_get() - start_tick, (g_storage_debug_last_read_bytes > 0U));
    return loaded;
}

static bool storage_load_access_log_now(void)
{
    bool loaded_any = false;
    static char log_buffer[ACCESS_LOG_BUFFER_SIZE];
    static app_access_log_entry_t entries[ACCESS_LOG_SIZE];
    unsigned int entry_count = 0U;
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

    (void) storage_prune_access_logs_open_media();

    memset(entries, 0, sizeof(entries));
    log_buffer[0] = '\0';
    loaded_any = storage_load_access_log_tail_file(ACCESS_LOG_ARCHIVE_FILE_NAME,
                                                   log_buffer,
                                                   sizeof(log_buffer),
                                                   entries,
                                                   ACCESS_LOG_SIZE,
                                                   &entry_count) || loaded_any;
    loaded_any = storage_load_access_log_tail_file(ACCESS_LOG_FILE_NAME,
                                                   log_buffer,
                                                   sizeof(log_buffer),
                                                   entries,
                                                   ACCESS_LOG_SIZE,
                                                   &entry_count) || loaded_any;

    if (FX_SUCCESS == fx_media_close(&g_fx_media0))
    {
        g_storage_media_ready = false;
    }
    storage_media_unlock();

    if (entry_count > 0U)
    {
        app_access_log_restore(entries, (int) entry_count);
    }

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
    ULONG read_offset = 0U;
    static char log_buffer[METRICS_LOG_BUFFER_SIZE];
    ULONG start_tick = tx_time_get();
    bool pruned_metrics = false;

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

    (void) storage_prune_metrics_log_open_media();

    status = fx_file_open(&g_fx_media0, &g_metrics_file, METRICS_LOG_FILE_NAME, FX_OPEN_FOR_READ);
    if (FX_SUCCESS == status)
    {
        if (g_metrics_file.fx_file_current_file_size > (ULONG) (METRICS_LOG_BUFFER_SIZE - 1U))
        {
            read_offset = (ULONG) (g_metrics_file.fx_file_current_file_size - (ULONG) (METRICS_LOG_BUFFER_SIZE - 1U));
        }
        status = fx_file_seek(&g_metrics_file, read_offset);
        if (FX_SUCCESS == status)
        {
            status = fx_file_read(&g_metrics_file, log_buffer, METRICS_LOG_BUFFER_SIZE - 1U, &actual_bytes);
        }
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
        const char *parse_cursor = log_buffer;

        if (read_offset > 0U)
        {
            const char *first_newline = strchr(log_buffer, '\n');
            parse_cursor = (NULL != first_newline) ? (first_newline + 1) : "";
        }
        (void) storage_parse_metrics_text(parse_cursor, &pruned_metrics);
    }

    storage_lock();
    g_storage_metrics_loaded = true;
    storage_unlock();

    app_metric_add_no_persist("Storage load latency", "metrics.log", tx_time_get() - start_tick, ((FX_SUCCESS == status) && (actual_bytes > 0U)));
    if (pruned_metrics)
    {
        (void) storage_metrics_persist_now();
    }
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
    g_storage_users_load_failed = false;
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

    if (!g_storage_io_mutex_ready)
    {
        if (TX_SUCCESS == tx_mutex_create(&g_storage_io_mutex, "storage_io_mutex", TX_NO_INHERIT))
        {
            g_storage_io_mutex_ready = true;
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
    g_storage_metric_pending_count = 0U;
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
            else if (g_storage_format_requested)
            {
                g_storage_debug_last_stage = 19U;
                g_storage_format_requested = false;
                storage_unlock();

                storage_io_lock();
                storage_lock();
                g_storage_users_load_failed = false;
                storage_capture_users_io_snapshot_locked();
                storage_unlock();

                ok = storage_format_media_now();
                if (ok)
                {
                    ok = storage_save_users_json_snapshot(&persist_size, &user_count_snapshot);
                }
                storage_io_unlock();

                storage_lock();
                g_storage_persist_json_size = persist_size;
                g_storage_persist_status = ok ? STORAGE_PERSIST_STATUS_SUCCESS : STORAGE_PERSIST_STATUS_FAILED;
                g_storage_debug_last_user_count = user_count_snapshot;
                storage_unlock();
                continue;
            }
            else if (g_storage_persist_requested)
            {
                bool needs_load = false;

                g_storage_debug_last_stage = 1U;
                g_storage_persist_requested = false;
                needs_load = !g_storage_loaded;
                storage_unlock();

                if (needs_load)
                {
                    ok = storage_load_users_now();
                    storage_lock();
                    if (!ok || !g_storage_loaded || g_storage_users_load_failed)
                    {
                        storage_unlock();
                        g_storage_persist_status = STORAGE_PERSIST_STATUS_FAILED;
                        g_storage_debug_last_save_status = FX_INVALID_NAME;
                        continue;
                    }
                    storage_unlock();
                }

                storage_io_lock();
                storage_lock();
                if (g_storage_users_load_failed)
                {
                    if (!g_storage_loaded || ((g_user_count <= 0) && !g_recent_user_valid))
                    {
                        storage_unlock();
                        storage_io_unlock();
                        g_storage_persist_status = STORAGE_PERSIST_STATUS_FAILED;
                        g_storage_debug_last_save_status = FX_INVALID_NAME;
                        continue;
                    }
                    g_storage_users_load_failed = false;
                }
                storage_capture_users_io_snapshot_locked();
                storage_unlock();

                ok = storage_save_users_json_snapshot(&persist_size, &user_count_snapshot);
                storage_io_unlock();

                storage_lock();
                g_storage_persist_json_size = persist_size;
                storage_unlock();
                g_storage_persist_status = ok ? STORAGE_PERSIST_STATUS_SUCCESS : STORAGE_PERSIST_STATUS_FAILED;
                g_storage_debug_last_user_count = user_count_snapshot;
                continue;
            }
            else if (g_storage_access_log_persist_requested)
            {
                g_storage_debug_last_stage = 16U;
                while (g_storage_access_log_persist_requested)
                {
                    g_storage_access_log_persist_requested = false;
                    access_log_size = 0U;
                    access_log_count = storage_format_access_log_entries(g_storage_access_log_pending,
                                                                        g_storage_access_log_pending_count,
                                                                        g_storage_access_log_work_text,
                                                                        sizeof(g_storage_access_log_work_text),
                                                                        &access_log_size);
                    g_storage_access_log_work_text_size = access_log_size;

                    if ((0U == access_log_count) || (0U == g_storage_access_log_work_text_size))
                    {
                        storage_unlock();
                        storage_media_lock();
                        if (storage_media_open())
                        {
                            (void) storage_prune_access_logs_open_media();
                            if (FX_SUCCESS == fx_media_close(&g_fx_media0))
                            {
                                g_storage_media_ready = false;
                            }
                            else
                            {
                                (void) fx_media_close(&g_fx_media0);
                                g_storage_media_ready = false;
                            }
                        }
                        storage_media_unlock();
                        break;
                    }

                    memcpy(g_storage_access_log_work_entries,
                           g_storage_access_log_pending,
                           access_log_count * sizeof(g_storage_access_log_work_entries[0]));

                    if (g_storage_access_log_pending_count > access_log_count)
                    {
                        memmove(g_storage_access_log_pending,
                                &g_storage_access_log_pending[access_log_count],
                                (g_storage_access_log_pending_count - access_log_count) * sizeof(g_storage_access_log_pending[0]));
                    }
                    g_storage_access_log_pending_count -= access_log_count;
                    if (g_storage_access_log_pending_count > 0U)
                    {
                        g_storage_access_log_persist_requested = true;
                    }
                    storage_unlock();

                    ok = storage_save_access_log_buffer(g_storage_access_log_work_text,
                                                        g_storage_access_log_work_text_size);

                    storage_lock();
                    if (!ok)
                    {
                        storage_requeue_access_log_entries_locked(g_storage_access_log_work_entries, access_log_count);
                        storage_unlock();
                        break;
                    }
                    if (!g_storage_access_log_persist_requested)
                    {
                        storage_unlock();
                        break;
                    }
                }
                continue;
            }
            else if (g_storage_metrics_persist_requested)
            {
                unsigned int metric_count = 0U;
                size_t metric_log_size = 0U;

                g_storage_debug_last_stage = 17U;
                while (g_storage_metrics_persist_requested)
                {
                    g_storage_metrics_persist_requested = false;
                    metric_log_size = 0U;
                    metric_count = storage_format_metric_entries(g_storage_metric_pending,
                                                                 g_storage_metric_pending_count,
                                                                 g_storage_metrics_text,
                                                                 sizeof(g_storage_metrics_text),
                                                                 &metric_log_size);
                    g_storage_metrics_text_size = metric_log_size;

                    if ((0U == metric_count) || (0U == g_storage_metrics_text_size))
                    {
                        storage_unlock();
                        storage_media_lock();
                        if (storage_media_open())
                        {
                            (void) storage_prune_metrics_log_open_media();
                            if (FX_SUCCESS == fx_media_close(&g_fx_media0))
                            {
                                g_storage_media_ready = false;
                            }
                            else
                            {
                                (void) fx_media_close(&g_fx_media0);
                                g_storage_media_ready = false;
                            }
                        }
                        storage_media_unlock();
                        break;
                    }

                    memcpy(g_storage_metric_work_entries,
                           g_storage_metric_pending,
                           metric_count * sizeof(g_storage_metric_work_entries[0]));

                    if (g_storage_metric_pending_count > metric_count)
                    {
                        memmove(g_storage_metric_pending,
                                &g_storage_metric_pending[metric_count],
                                (g_storage_metric_pending_count - metric_count) * sizeof(g_storage_metric_pending[0]));
                    }
                    g_storage_metric_pending_count -= metric_count;
                    if (g_storage_metric_pending_count > 0U)
                    {
                        g_storage_metrics_persist_requested = true;
                    }
                    storage_unlock();

                    ok = storage_save_metrics_buffer(g_storage_metrics_text, g_storage_metrics_text_size);

                    storage_lock();
                    if (!ok)
                    {
                        storage_requeue_metric_entries_locked(g_storage_metric_work_entries, metric_count);
                        storage_unlock();
                        break;
                    }
                    if (!g_storage_metrics_persist_requested)
                    {
                        storage_unlock();
                        break;
                    }
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
            !profile->is_admin &&
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

int storage_user_count_nowait(void)
{
    int count;

    storage_init();
    storage_lock();
    count = g_user_count;
    storage_unlock();

    return count;
}

bool storage_users_loaded_nowait(void)
{
    bool loaded;

    storage_init();
    storage_lock();
    loaded = g_storage_loaded;
    storage_unlock();

    return loaded;
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

bool storage_profile_get_nowait(int index, storage_user_profile_t *out_profile)
{
    bool ok = false;

    if (NULL == out_profile)
    {
        return false;
    }

    storage_init();
    storage_lock();

    if ((index >= 0) && (index < g_user_count))
    {
        *out_profile = g_users[index];
        ok = true;
    }

    storage_unlock();
    return ok;
}

bool storage_meeting_profile_key_hash_for_index(int index, uint32_t *out_hash)
{
    bool ok = false;

    if (NULL == out_hash)
    {
        return false;
    }

    *out_hash = 0U;
    storage_init();
    storage_lock();
    storage_ensure_loaded_locked();

    if ((index >= 0) && (index < g_user_count))
    {
        ok = storage_profile_key_hash_from_profile_locked(&g_users[index], out_hash);
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
    else
    {
        storage_refresh_recent_user_locked();
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

static bool storage_meeting_mode_start_internal(const int *profile_indices,
                                                int profile_count,
                                                const char *meeting_chapter,
                                                ULONG start_unix,
                                                ULONG end_unix,
                                                bool persist_active_state,
                                                unsigned int *out_selected_profiles,
                                                unsigned int *out_allowed_cards)
{
    unsigned int selected_profiles = 0U;
    unsigned int allowed_cards = 0U;
    unsigned int valid_cards = 0U;
    bool seen_profiles[STORAGE_MAX_USERS];
    storage_meeting_active_t active_state;
    bool active_saved = true;

    memset(seen_profiles, 0, sizeof(seen_profiles));
    memset(&active_state, 0, sizeof(active_state));

    if ((0U == start_unix) || (end_unix <= start_unix))
    {
        if (NULL != out_selected_profiles)
        {
            *out_selected_profiles = 0U;
        }
        if (NULL != out_allowed_cards)
        {
            *out_allowed_cards = 0U;
        }
        return false;
    }

    storage_init();
    storage_lock();
    storage_ensure_loaded_locked();

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

                storage_copy_clean_uid(cleaned_uid, sizeof(cleaned_uid), profile->cards[card_index]);
                if ('\0' != cleaned_uid[0])
                {
                    valid_cards++;
                }
            }
        }
    }

    if ((0U == selected_profiles) || (0U == valid_cards))
    {
        if (NULL != out_selected_profiles)
        {
            *out_selected_profiles = 0U;
        }
        if (NULL != out_allowed_cards)
        {
            *out_allowed_cards = 0U;
        }

        storage_unlock();
        return false;
    }

    storage_meeting_mode_clear_locked();
    memset(seen_profiles, 0, sizeof(seen_profiles));
    selected_profiles = 0U;
    allowed_cards = 0U;

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

        if (selected_profiles < STORAGE_MAX_USERS)
        {
            g_storage_meeting_mode_profile_indices[selected_profiles] = (uint8_t) index;
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

    g_storage_meeting_mode_selected_profiles = selected_profiles;
    g_storage_meeting_mode_allowed_count = allowed_cards;
    storage_copy_text(g_storage_meeting_mode_chapter,
                      sizeof(g_storage_meeting_mode_chapter),
                      (NULL != meeting_chapter) ? meeting_chapter : "",
                      (NULL != meeting_chapter) ? strlen(meeting_chapter) : 0U);
    g_storage_meeting_mode_start_unix = start_unix;
    g_storage_meeting_mode_end_unix = end_unix;
    g_storage_meeting_mode_active = ((selected_profiles > 0U) && (allowed_cards > 0U));
    if (!g_storage_meeting_mode_active)
    {
        storage_meeting_mode_clear_locked();
    }
    else
    {
        active_state.start_unix = g_storage_meeting_mode_start_unix;
        active_state.end_unix = g_storage_meeting_mode_end_unix;
        active_state.profile_count = g_storage_meeting_mode_selected_profiles;
        for (unsigned int i = 0U; i < active_state.profile_count; i++)
        {
            uint32_t profile_key_hash = 0U;

            active_state.profile_indices[i] = g_storage_meeting_mode_profile_indices[i];
            if ((i < STORAGE_MAX_USERS) &&
                storage_profile_key_hash_from_profile_locked(&g_users[g_storage_meeting_mode_profile_indices[i]],
                                                             &profile_key_hash))
            {
                active_state.profile_key_hashes[active_state.profile_key_count] = profile_key_hash;
                active_state.profile_key_count++;
            }
        }
        storage_copy_text(active_state.meeting_chapter,
                          sizeof(active_state.meeting_chapter),
                          g_storage_meeting_mode_chapter,
                          strlen(g_storage_meeting_mode_chapter));
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
    if (persist_active_state)
    {
        storage_io_lock();
        active_saved = storage_save_meeting_active_json(&active_state);
        storage_io_unlock();
        if (!active_saved)
        {
            storage_io_lock();
            (void) storage_clear_meeting_active_file();
            storage_io_unlock();
            storage_lock();
            storage_meeting_mode_clear_locked();
            storage_unlock();
            return false;
        }
    }

    return g_storage_meeting_mode_active;
}

bool storage_meeting_mode_start(const int *profile_indices,
                                int profile_count,
                                const char *meeting_chapter,
                                ULONG start_unix,
                                ULONG end_unix,
                                unsigned int *out_selected_profiles,
                                unsigned int *out_allowed_cards)
{
    return storage_meeting_mode_start_internal(profile_indices,
                                               profile_count,
                                               meeting_chapter,
                                               start_unix,
                                               end_unix,
                                               true,
                                               out_selected_profiles,
                                               out_allowed_cards);
}

bool storage_meeting_mode_restore(const int *profile_indices,
                                  int profile_count,
                                  const char *meeting_chapter,
                                  ULONG start_unix,
                                  ULONG end_unix,
                                  unsigned int *out_selected_profiles,
                                  unsigned int *out_allowed_cards)
{
    return storage_meeting_mode_start_internal(profile_indices,
                                               profile_count,
                                               meeting_chapter,
                                               start_unix,
                                               end_unix,
                                               false,
                                               out_selected_profiles,
                                               out_allowed_cards);
}

void storage_meeting_mode_stop(void)
{
    storage_init();
    storage_lock();
    storage_meeting_mode_clear_locked();
    storage_unlock();
    (void) storage_meeting_active_clear();
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

bool storage_meeting_mode_chapter(char *out_chapter, size_t out_size)
{
    bool has_chapter;

    if ((NULL == out_chapter) || (0U == out_size))
    {
        return false;
    }

    storage_init();
    storage_lock();
    storage_copy_text(out_chapter,
                      out_size,
                      g_storage_meeting_mode_chapter,
                      strlen(g_storage_meeting_mode_chapter));
    has_chapter = g_storage_meeting_mode_active && ('\0' != out_chapter[0]);
    storage_unlock();
    return has_chapter;
}

bool storage_meeting_mode_end_utc(ULONG *out_end_unix)
{
    bool active;

    if (NULL == out_end_unix)
    {
        return false;
    }

    storage_init();
    storage_lock();
    active = g_storage_meeting_mode_active;
    *out_end_unix = active ? g_storage_meeting_mode_end_unix : 0U;
    storage_unlock();
    return active && (0U != *out_end_unix);
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

int storage_meeting_active_load(storage_meeting_active_t *out_state)
{
    UINT status;
    ULONG actual_bytes = 0U;
    int parsed_count = 0;
    bool file_opened = false;
    ULONG start_tick = tx_time_get();

    if (NULL == out_state)
    {
        return -1;
    }

    memset(out_state, 0, sizeof(*out_state));
    storage_init();
    if (!storage_media_access_allowed())
    {
        return -1;
    }

    storage_io_lock();
    storage_media_lock();
    if (!storage_media_open())
    {
        storage_media_unlock();
        storage_io_unlock();
        return -1;
    }

    status = fx_file_open(&g_fx_media0, &g_meeting_active_file, MEETING_ACTIVE_FILE_NAME, FX_OPEN_FOR_READ);
    if (FX_SUCCESS == status)
    {
        file_opened = true;
        if (g_meeting_active_file.fx_file_current_file_size >= sizeof(g_storage_meeting_schedule_text))
        {
            status = FX_BUFFER_ERROR;
        }
    }

    if (FX_SUCCESS == status)
    {
        status = fx_file_read(&g_meeting_active_file,
                              g_storage_meeting_schedule_text,
                              sizeof(g_storage_meeting_schedule_text) - 1U,
                              &actual_bytes);
        if (actual_bytes < sizeof(g_storage_meeting_schedule_text))
        {
            g_storage_meeting_schedule_text[actual_bytes] = '\0';
        }
        else
        {
            g_storage_meeting_schedule_text[sizeof(g_storage_meeting_schedule_text) - 1U] = '\0';
        }
    }

    if (file_opened)
    {
        (void) fx_file_close(&g_meeting_active_file);
    }
    (void) fx_media_close(&g_fx_media0);
    g_storage_media_ready = false;
    storage_media_unlock();

    if (FX_SUCCESS == status)
    {
        parsed_count = storage_parse_meeting_active_json(g_storage_meeting_schedule_text, out_state);
    }
    else if (!file_opened)
    {
        parsed_count = 0;
    }
    else
    {
        parsed_count = -1;
    }

    storage_io_unlock();
    app_metric_add_no_persist("Storage load latency", "meeting_active.json", tx_time_get() - start_tick, (parsed_count >= 0));
    return parsed_count;
}

bool storage_meeting_active_clear(void)
{
    bool ok;

    storage_init();
    if (!storage_media_access_allowed())
    {
        return false;
    }

    storage_io_lock();
    ok = storage_clear_meeting_active_file();
    storage_io_unlock();
    return ok;
}

int storage_meeting_schedule_load(storage_meeting_schedule_t *out_schedules, int max_schedules)
{
    UINT status;
    ULONG actual_bytes = 0U;
    int parsed_count = 0;
    bool file_opened = false;
    ULONG start_tick = tx_time_get();

    if ((NULL == out_schedules) || (max_schedules <= 0))
    {
        return -1;
    }

    storage_init();
    if (!storage_media_access_allowed())
    {
        return -1;
    }

    storage_io_lock();
    storage_media_lock();
    if (!storage_media_open())
    {
        storage_media_unlock();
        storage_io_unlock();
        return -1;
    }

    status = fx_file_open(&g_fx_media0, &g_meeting_schedule_file, MEETING_SCHEDULE_FILE_NAME, FX_OPEN_FOR_READ);
    if (FX_SUCCESS == status)
    {
        file_opened = true;
        if (g_meeting_schedule_file.fx_file_current_file_size >= sizeof(g_storage_meeting_schedule_text))
        {
            status = FX_BUFFER_ERROR;
        }
    }

    if (FX_SUCCESS == status)
    {
        status = fx_file_read(&g_meeting_schedule_file,
                              g_storage_meeting_schedule_text,
                              sizeof(g_storage_meeting_schedule_text) - 1U,
                              &actual_bytes);
        if (actual_bytes < sizeof(g_storage_meeting_schedule_text))
        {
            g_storage_meeting_schedule_text[actual_bytes] = '\0';
        }
        else
        {
            g_storage_meeting_schedule_text[sizeof(g_storage_meeting_schedule_text) - 1U] = '\0';
        }
    }

    if (file_opened)
    {
        (void) fx_file_close(&g_meeting_schedule_file);
    }
    (void) fx_media_close(&g_fx_media0);
    g_storage_media_ready = false;
    storage_media_unlock();

    if (FX_SUCCESS == status)
    {
        parsed_count = storage_parse_meeting_schedules_json(g_storage_meeting_schedule_text,
                                                            out_schedules,
                                                            max_schedules);
    }
    else if (!file_opened)
    {
        parsed_count = 0;
    }
    else
    {
        parsed_count = -1;
    }

    storage_io_unlock();
    app_metric_add_no_persist("Storage load latency", "meeting.json", tx_time_get() - start_tick, (parsed_count >= 0));
    return parsed_count;
}

bool storage_meeting_schedule_save(const storage_meeting_schedule_t *schedules, int schedule_count)
{
    bool ok;

    if ((schedule_count < 0) || (schedule_count > STORAGE_MEETING_SCHEDULE_MAX_ITEMS) ||
        ((schedule_count > 0) && (NULL == schedules)))
    {
        return false;
    }

    storage_init();
    if (!storage_media_access_allowed())
    {
        return false;
    }

    storage_io_lock();
    ok = storage_save_meeting_schedules_json(schedules, schedule_count);
    storage_io_unlock();
    return ok;
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

bool storage_admin_pin_configured(void)
{
    bool configured = false;

    storage_init();
    storage_lock();
    storage_ensure_loaded_locked();

    if (g_storage_users_load_failed)
    {
        storage_unlock();
        return true;
    }

    for (int i = 0; i < g_user_count; i++)
    {
        if (g_users[i].is_admin && ('\0' != g_users[i].admin_pin[0]))
        {
            configured = true;
            break;
        }
    }

    storage_unlock();
    return configured;
}

bool storage_remove_uid(const char *uid_str)
{
    bool ok = false;
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

    storage_init();
    storage_lock();
    storage_load_users_locked();

    for (int i = 0; i < g_user_count; i++)
    {
        for (unsigned int card_index = 0U; card_index < g_users[i].card_count; card_index++)
        {
            if (0 == strcmp(g_users[i].cards[card_index], cleaned_uid))
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

                storage_refresh_recent_user_locked();

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
    UINT semaphore_status;

    storage_init();
    if (!g_storage_thread_ready)
    {
        return false;
    }

    storage_lock();
    g_storage_persist_requested = true;
    g_storage_persist_status = STORAGE_PERSIST_STATUS_PENDING;
    storage_unlock();

    semaphore_status = tx_semaphore_put(&g_storage_persist_semaphore);
    if (TX_SUCCESS != semaphore_status)
    {
        storage_lock();
        g_storage_persist_requested = false;
        g_storage_persist_status = STORAGE_PERSIST_STATUS_FAILED;
        storage_unlock();
        return false;
    }

    return true;
}

bool storage_persist_now_direct(void)
{
    bool requested;
    bool ok;

    storage_init();
    g_storage_debug_direct_persist_requests++;
    g_storage_debug_last_stage = 18U;

    /*
     * Keep FileX/QSPI writes in the dedicated storage worker.  The web thread
     * waits for completion, but does not touch the media directly.
     */
    requested = storage_persist_now();
    ok = requested && storage_persist_wait(5U * TX_TIMER_TICKS_PER_SECOND);
    if (ok)
    {
        g_storage_debug_direct_persist_successes++;
    }

    return ok;
}

bool storage_format_qspi_and_persist(void)
{
    UINT semaphore_status;

    storage_init();
    if (!g_storage_thread_ready)
    {
        return false;
    }

    storage_lock();
    g_storage_format_requested = true;
    g_storage_persist_status = STORAGE_PERSIST_STATUS_PENDING;
    g_storage_debug_last_stage = 19U;
    storage_unlock();

    semaphore_status = tx_semaphore_put(&g_storage_persist_semaphore);
    if (TX_SUCCESS != semaphore_status)
    {
        storage_lock();
        g_storage_format_requested = false;
        g_storage_persist_status = STORAGE_PERSIST_STATUS_FAILED;
        storage_unlock();
        return false;
    }

    return true;
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

void storage_debug_snapshot(storage_debug_info_t *out_info)
{
    if (NULL == out_info)
    {
        return;
    }

    memset(out_info, 0, sizeof(*out_info));
    storage_init();
    storage_lock();
    out_info->persist_status = (unsigned int) g_storage_persist_status;
    out_info->thread_ready = g_storage_thread_ready;
    out_info->loaded = g_storage_loaded;
    out_info->load_failed = g_storage_users_load_failed;
    out_info->last_json_size = (ULONG) g_storage_persist_json_size;
    storage_unlock();

    out_info->worker_runs = g_storage_debug_worker_runs;
    out_info->last_stage = g_storage_debug_last_stage;
    out_info->last_media_status = g_storage_debug_last_media_status;
    out_info->last_save_status = g_storage_debug_last_save_status;
    out_info->last_load_status = g_storage_debug_last_load_status;
    out_info->last_saved_bytes = g_storage_debug_last_saved_bytes;
    out_info->last_read_bytes = g_storage_debug_last_read_bytes;
    out_info->last_user_count = g_storage_debug_last_user_count;
    out_info->direct_persist_requests = g_storage_debug_direct_persist_requests;
    out_info->direct_persist_successes = g_storage_debug_direct_persist_successes;
    out_info->format_status = g_storage_debug_last_format_status;
    out_info->erase_status = g_storage_debug_last_erase_status;
    out_info->erase_step = g_storage_debug_last_erase_step;
}

bool storage_access_log_enqueue(const app_access_log_entry_t *entry)
{
    bool queued = false;

    storage_init();
    if ((!g_storage_thread_ready) || (NULL == entry))
    {
        return false;
    }

    for (unsigned int attempt = 0U; attempt < 8U; attempt++)
    {
        storage_lock();
        if (g_storage_access_log_pending_count < ACCESS_LOG_PENDING_SIZE)
        {
            g_storage_access_log_pending[g_storage_access_log_pending_count++] = *entry;
            queued = true;
        }
        g_storage_access_log_persist_requested = true;
        storage_unlock();

        (void) tx_semaphore_put(&g_storage_persist_semaphore);
        if (queued)
        {
            return true;
        }

        tx_thread_sleep(1U);
    }

    return false;
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

bool storage_metric_enqueue(const app_metric_entry_t *entry)
{
    bool queued = false;

    storage_init();
    if ((!g_storage_thread_ready) || (NULL == entry))
    {
        return false;
    }

    for (unsigned int attempt = 0U; attempt < 8U; attempt++)
    {
        storage_lock();
        if (g_storage_metric_pending_count < METRICS_PENDING_SIZE)
        {
            g_storage_metric_pending[g_storage_metric_pending_count++] = *entry;
            queued = true;
        }
        g_storage_metrics_persist_requested = true;
        storage_unlock();

        (void) tx_semaphore_put(&g_storage_persist_semaphore);
        if (queued)
        {
            return true;
        }

        tx_thread_sleep(1U);
    }

    return false;
}

bool storage_metrics_persist_now(void)
{
    storage_init();
    if (!g_storage_thread_ready)
    {
        return false;
    }

    storage_lock();
    if (g_storage_format_requested ||
        g_storage_persist_requested ||
        (STORAGE_PERSIST_STATUS_PENDING == g_storage_persist_status))
    {
        storage_unlock();
        return true;
    }
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

    return storage_get_file_size(USERS_FILE_NAME, out_size);
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

    if (!storage_read_file_chunk(USERS_FILE_NAME, 0U, out, out_size - 1U, &actual_read))
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

    if (!storage_read_file_chunk(USERS_FILE_NAME, offset, out, out_size, &actual_read))
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
