#include "storage.h"
#include "main.h"
#include <stdio.h>
#include <string.h>

#define MAX_USERS 50
#define JSON_BUFFER_SIZE 4096
#define STORAGE_MEDIA_SAFE_DELAY_TICKS (5U * TX_TIMER_TICKS_PER_SECOND)
#define STORAGE_THREAD_STACK_SIZE 2048U

#define STORAGE_PERSIST_STATUS_IDLE     (0U)
#define STORAGE_PERSIST_STATUS_PENDING  (1U)
#define STORAGE_PERSIST_STATUS_SUCCESS  (2U)
#define STORAGE_PERSIST_STATUS_FAILED   (3U)

/* =========================================================================
   MANTENHA A 1 NA PRIMEIRA VEZ QUE GRAVAR NA PLACA.
   Depois do ecra acender, pode mudar para 0 nas proximas vezes que compilar.
   ========================================================================= */
#define WIPE_QSPI_FIRST_TIME 0

typedef struct
{
    char name[NAME_MAX_LEN];
    char uid[UID_MAX_LEN];
} user_t;

static user_t g_users[MAX_USERS];
static user_t g_recent_user;
static int g_user_count = 0;
static bool g_storage_initialized = false;
static bool g_storage_mutex_ready = false;
static bool g_storage_media_ready = false;
static bool g_storage_loaded = false;
static bool g_storage_thread_ready = false;
static bool g_storage_persist_requested = false;
static bool g_storage_load_requested = false;
static bool g_recent_user_valid = false;
static TX_MUTEX g_storage_mutex;
static TX_SEMAPHORE g_storage_persist_semaphore;
static TX_THREAD g_storage_thread;
static ULONG g_storage_thread_stack[STORAGE_THREAD_STACK_SIZE / sizeof(ULONG)];
static FX_FILE g_users_file;
static char g_storage_persist_json[JSON_BUFFER_SIZE];
static size_t g_storage_persist_json_size = 0U;
static volatile ULONG g_storage_persist_status = STORAGE_PERSIST_STATUS_IDLE;
volatile ULONG g_storage_debug_worker_runs = 0U;
volatile ULONG g_storage_debug_last_stage = 0U;
volatile ULONG g_storage_debug_last_media_status = 0U;
volatile ULONG g_storage_debug_last_save_status = 0U;
volatile ULONG g_storage_debug_last_load_status = 0U;
volatile ULONG g_storage_debug_last_user_count = 0U;
volatile ULONG g_storage_debug_last_read_bytes = 0U;
volatile ULONG g_storage_debug_last_saved_bytes = 0U;
volatile ULONG g_storage_debug_snapshot_user_count = 0U;
volatile ULONG g_storage_debug_snapshot_recent_valid = 0U;
volatile char g_storage_debug_snapshot_preview[96] = {0};
extern volatile ULONG g_net_debug_media_status;

static void storage_thread_entry(ULONG initial_input);
static void storage_refresh_persist_snapshot_locked(void);

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

    status = fx_media_init0_open();
    g_storage_debug_last_media_status = status;
    g_net_debug_media_status = status;
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

static bool storage_parse_users(const char *json_buffer)
{
    const char *ptr = json_buffer;

    g_user_count = 0;
    g_recent_user_valid = false;

    while ((ptr = strstr(ptr, "\"name\"")) != NULL && g_user_count < MAX_USERS)
    {
        const char *name_start = strchr(ptr + 6, '"');
        const char *name_end;
        const char *uid_key;
        const char *uid_start;
        const char *uid_end;

        if (NULL == name_start)
        {
            break;
        }

        name_start++;
        name_end = strchr(name_start, '"');
        if (NULL == name_end)
        {
            break;
        }

        uid_key = strstr(name_end, "\"uid\"");
        if (NULL == uid_key)
        {
            break;
        }

        uid_start = strchr(uid_key + 5, '"');
        if (NULL == uid_start)
        {
            break;
        }

        uid_start++;
        uid_end = strchr(uid_start, '"');
        if (NULL == uid_end)
        {
            break;
        }

        storage_copy_text(g_users[g_user_count].name,
                          sizeof(g_users[g_user_count].name),
                          name_start,
                          (size_t) (name_end - name_start));
        storage_copy_text(g_users[g_user_count].uid,
                          sizeof(g_users[g_user_count].uid),
                          uid_start,
                          (size_t) (uid_end - uid_start));

        g_user_count++;
        ptr = uid_end;
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
        int written;

        written = snprintf(&json_buffer[offset],
                           json_buffer_size - offset,
                           "{\"name\":\"%s\",\"uid\":\"%s\"}",
                           g_recent_user.name,
                           g_recent_user.uid);
        if ((written < 0) || ((size_t) written >= (json_buffer_size - offset)))
        {
            return false;
        }

        offset += (size_t) written;
    }

    for (int i = 0; i < count_to_write; i++)
    {
        int written;

        written = snprintf(&json_buffer[offset],
                           json_buffer_size - offset,
                           "%s{\"name\":\"%s\",\"uid\":\"%s\"}",
                           (i == 0) ? "" : ",",
                           g_users[i].name,
                           g_users[i].uid);
        if ((written < 0) || ((size_t) written >= (json_buffer_size - offset)))
        {
            return false;
        }

        offset += (size_t) written;
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

    if (storage_prepare_users_json_locked(g_storage_persist_json,
                                          sizeof(g_storage_persist_json),
                                          &g_storage_persist_json_size))
    {
        g_storage_debug_last_user_count = (ULONG) g_user_count;
        g_storage_debug_snapshot_user_count = (ULONG) g_user_count;
        g_storage_debug_snapshot_recent_valid = g_recent_user_valid ? 1U : 0U;
        storage_copy_text((char *) g_storage_debug_snapshot_preview,
                          sizeof(g_storage_debug_snapshot_preview),
                          g_storage_persist_json,
                          strlen(g_storage_persist_json));
    }
}

static bool storage_save_users_locked(void)
{
    UINT status;

    g_storage_debug_last_saved_bytes = (ULONG) g_storage_persist_json_size;

    status = fx_file_open(&g_fx_media0, &g_users_file, "users.json", FX_OPEN_FOR_WRITE);
    g_storage_debug_last_stage = 10U;
    if (FX_SUCCESS != status)
    {
        status = fx_file_create(&g_fx_media0, "users.json");
        if ((FX_SUCCESS != status) && (FX_ALREADY_CREATED != status))
        {
            return false;
        }

        status = fx_file_open(&g_fx_media0, &g_users_file, "users.json", FX_OPEN_FOR_WRITE);
        if (FX_SUCCESS != status)
        {
            return false;
        }
    }

    status = fx_file_truncate(&g_users_file, 0U);
    g_storage_debug_last_stage = 11U;
    if (FX_SUCCESS == status)
    {
        status = fx_file_seek(&g_users_file, 0U);
    }

    if (FX_SUCCESS == status)
    {
        g_storage_debug_last_stage = 12U;
        status = fx_file_write(&g_users_file, g_storage_persist_json, g_storage_persist_json_size);
    }

    if (FX_SUCCESS == status)
    {
        g_storage_debug_last_stage = 13U;
        status = fx_file_close(&g_users_file);
    }
    else
    {
        (void) fx_file_close(&g_users_file);
        return false;
    }

    if (FX_SUCCESS == status)
    {
        g_storage_debug_last_stage = 14U;
        status = fx_media_flush(&g_fx_media0);
    }

    if (FX_SUCCESS == status)
    {
        g_storage_debug_last_stage = 15U;
        status = fx_media_close(&g_fx_media0);
        if (FX_SUCCESS == status)
        {
            g_storage_media_ready = false;
        }
    }

    g_storage_debug_last_save_status = status;

    return (FX_SUCCESS == status);
}

static void storage_load_users_locked(void)
{
    UINT status;
    ULONG actual_bytes;
    static char json_buffer[JSON_BUFFER_SIZE];

    if (g_storage_loaded)
    {
        return;
    }

    if (!storage_media_access_allowed())
    {
        return;
    }

    if (!storage_media_open())
    {
        return;
    }

    status = fx_file_open(&g_fx_media0, &g_users_file, "users.json", FX_OPEN_FOR_READ);
    g_storage_debug_last_stage = 20U;
    if (FX_SUCCESS == status)
    {
        status = fx_file_read(&g_users_file, json_buffer, JSON_BUFFER_SIZE - 1U, &actual_bytes);
        g_storage_debug_last_read_bytes = actual_bytes;
        if (FX_SUCCESS == status)
        {
            json_buffer[actual_bytes] = '\0';
            (void) storage_parse_users(json_buffer);
            g_storage_debug_last_user_count = (ULONG) g_user_count;
        }
        (void) fx_file_close(&g_users_file);
    }
    else
    {
        g_user_count = 0;
        g_recent_user_valid = false;
    }

    g_storage_debug_last_load_status = status;
    g_storage_loaded = true;
}

static void storage_mark_runtime_state_loaded(void)
{
    g_storage_loaded = true;
    g_storage_debug_last_user_count = (ULONG) g_user_count;
    storage_refresh_persist_snapshot_locked();
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
    g_storage_load_requested = true;

#if WIPE_QSPI_FIRST_TIME
    g_qspi0.p_api->open(g_qspi0.p_ctrl, g_qspi0.p_cfg);
    g_qspi0.p_api->erase(g_qspi0.p_ctrl, (uint8_t *) 0x60000000, 8 * 1024 * 1024);

    bool in_progress = true;
    while (in_progress)
    {
        g_qspi0.p_api->statusGet(g_qspi0.p_ctrl, &in_progress);
    }
    g_qspi0.p_api->close(g_qspi0.p_ctrl);
#endif

    g_storage_initialized = true;

    if (g_storage_thread_ready)
    {
        (void) tx_semaphore_put(&g_storage_persist_semaphore);
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
            g_storage_debug_worker_runs++;
            g_storage_debug_last_stage = 1U;

            while (!storage_media_access_allowed())
            {
                tx_thread_sleep(10U);
            }

            storage_lock();
            g_storage_debug_last_stage = 2U;

            if (g_storage_persist_requested)
            {
                g_storage_persist_requested = false;
                g_storage_debug_last_stage = 3U;
                if ((0 == g_user_count) && !g_recent_user_valid)
                {
                    storage_load_users_locked();
                }

                if (storage_media_open())
                {
                    g_storage_debug_last_stage = 4U;
                    ok = storage_save_users_locked();
                }

                g_storage_debug_last_stage = ok ? 5U : 6U;
                g_storage_persist_status = ok ? STORAGE_PERSIST_STATUS_SUCCESS : STORAGE_PERSIST_STATUS_FAILED;
            }
            else if (g_storage_load_requested)
            {
                g_storage_load_requested = false;
                g_storage_debug_last_stage = 21U;
                storage_load_users_locked();
            }
            storage_unlock();
        }
    }
}

bool storage_check_uid(const char *uid_str, char *out_name)
{
    bool found = false;

    if ((NULL == uid_str) || (NULL == out_name))
    {
        return false;
    }

    storage_init();
    storage_lock();
    storage_load_users_locked();

    for (int i = 0; i < g_user_count; i++)
    {
        if (0 == strcmp(g_users[i].uid, uid_str))
        {
            storage_copy_text(out_name, NAME_MAX_LEN, g_users[i].name, strlen(g_users[i].name));
            found = true;
            break;
        }
    }

    if (!found && g_recent_user_valid && (0 == strcmp(g_recent_user.uid, uid_str)))
    {
        storage_copy_text(out_name, NAME_MAX_LEN, g_recent_user.name, strlen(g_recent_user.name));
        found = true;
    }

    storage_unlock();
    return found;
}

int storage_user_count(void)
{
    int count;

    storage_init();
    storage_lock();
    storage_load_users_locked();
    count = g_user_count;
    storage_unlock();

    return count;
}

bool storage_user_get(int index, char *out_name, char *out_uid)
{
    bool ok = false;

    storage_init();
    storage_lock();
    storage_load_users_locked();

    if ((index >= 0) && (index < g_user_count))
    {
        if (NULL != out_name)
        {
            storage_copy_text(out_name, NAME_MAX_LEN, g_users[index].name, strlen(g_users[index].name));
        }

        if (NULL != out_uid)
        {
            storage_copy_text(out_uid, UID_MAX_LEN, g_users[index].uid, strlen(g_users[index].uid));
        }

        ok = true;
    }

    storage_unlock();
    return ok;
}

bool storage_add_user(const char *uid_str, const char *name)
{
    bool ok = false;

    if ((NULL == uid_str) || (NULL == name) || ('\0' == uid_str[0]) || ('\0' == name[0]))
    {
        return false;
    }

    storage_init();
    storage_lock();
    storage_load_users_locked();

    for (int i = 0; i < g_user_count; i++)
    {
        if (0 == strcmp(g_users[i].uid, uid_str))
        {
            storage_copy_text(g_users[i].name, sizeof(g_users[i].name), name, strlen(name));
            storage_copy_text(g_recent_user.uid, sizeof(g_recent_user.uid), uid_str, strlen(uid_str));
            storage_copy_text(g_recent_user.name, sizeof(g_recent_user.name), name, strlen(name));
            g_recent_user_valid = true;
            storage_mark_runtime_state_loaded();
            ok = true;
            storage_unlock();
            return ok;
        }
    }

    if (g_user_count < MAX_USERS)
    {
        storage_copy_text(g_users[g_user_count].uid, sizeof(g_users[g_user_count].uid), uid_str, strlen(uid_str));
        storage_copy_text(g_users[g_user_count].name, sizeof(g_users[g_user_count].name), name, strlen(name));
        storage_copy_text(g_recent_user.uid, sizeof(g_recent_user.uid), uid_str, strlen(uid_str));
        storage_copy_text(g_recent_user.name, sizeof(g_recent_user.name), name, strlen(name));
        g_recent_user_valid = true;
        g_user_count++;
        storage_mark_runtime_state_loaded();
        ok = true;
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
        if (0 == strcmp(g_users[i].uid, uid_str))
        {
            for (int j = i; j < (g_user_count - 1); j++)
            {
                g_users[j] = g_users[j + 1];
            }

            if (g_user_count > 0)
            {
                g_user_count--;
            }

            if (g_recent_user_valid && (0 == strcmp(g_recent_user.uid, uid_str)))
            {
                g_recent_user_valid = false;
                g_recent_user.uid[0] = '\0';
                g_recent_user.name[0] = '\0';
            }

            storage_mark_runtime_state_loaded();
            ok = true;
            break;
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

unsigned int storage_persist_status(void)
{
    return (unsigned int) g_storage_persist_status;
}
