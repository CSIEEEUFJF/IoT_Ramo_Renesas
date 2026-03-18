#include "storage.h"
#include "main.h"
#include <stdio.h>
#include <string.h>

#define MAX_USERS 50
#define JSON_BUFFER_SIZE 4096

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
static bool g_recent_user_valid = false;
static TX_MUTEX g_storage_mutex;
static FX_FILE g_users_file;
extern volatile ULONG g_net_debug_media_status;

static bool storage_media_open(void)
{
    UINT status;

    if (g_storage_media_ready)
    {
        return true;
    }

    status = fx_media_init0_open();
    g_net_debug_media_status = status;
    if (FX_SUCCESS == status)
    {
        g_storage_media_ready = true;
        return true;
    }

    return false;
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

static bool storage_save_users_locked(void)
{
    UINT status;
    char json_buffer[JSON_BUFFER_SIZE];
    size_t offset = 0U;

    json_buffer[offset++] = '[';

    for (int i = 0; i < g_user_count; i++)
    {
        int written;

        written = snprintf(&json_buffer[offset],
                           sizeof(json_buffer) - offset,
                           "%s{\"name\":\"%s\",\"uid\":\"%s\"}",
                           (i == 0) ? "" : ",",
                           g_users[i].name,
                           g_users[i].uid);
        if ((written < 0) || ((size_t) written >= (sizeof(json_buffer) - offset)))
        {
            return false;
        }

        offset += (size_t) written;
    }

    if (offset >= (sizeof(json_buffer) - 2U))
    {
        return false;
    }

    json_buffer[offset++] = ']';
    json_buffer[offset] = '\0';

    status = fx_file_open(&g_fx_media0, &g_users_file, "users.json", FX_OPEN_FOR_WRITE);
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
    if (FX_SUCCESS == status)
    {
        status = fx_file_seek(&g_users_file, 0U);
    }

    if (FX_SUCCESS == status)
    {
        status = fx_file_write(&g_users_file, json_buffer, offset);
    }

    if (FX_SUCCESS == status)
    {
        status = fx_file_close(&g_users_file);
    }
    else
    {
        (void) fx_file_close(&g_users_file);
        return false;
    }

    return (FX_SUCCESS == status);
}

void storage_init(void)
{
    UINT status;
    ULONG actual_bytes;
    static char json_buffer[JSON_BUFFER_SIZE];

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

    g_user_count = 0;
    g_recent_user_valid = false;

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

    storage_lock();
    status = fx_file_open(&g_fx_media0, &g_users_file, "users.json", FX_OPEN_FOR_READ);
    if (FX_SUCCESS == status)
    {
        status = fx_file_read(&g_users_file, json_buffer, JSON_BUFFER_SIZE - 1U, &actual_bytes);
        if (FX_SUCCESS == status)
        {
            json_buffer[actual_bytes] = '\0';
            (void) storage_parse_users(json_buffer);
        }
        (void) fx_file_close(&g_users_file);
    }
    else
    {
        status = fx_file_create(&g_fx_media0, "users.json");
        if ((FX_SUCCESS == status) || (FX_ALREADY_CREATED == status))
        {
            g_user_count = 0;
            (void) storage_save_users_locked();
        }
    }
    g_storage_initialized = true;
    storage_unlock();
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
    count = g_user_count;
    storage_unlock();

    return count;
}

bool storage_user_get(int index, char *out_name, char *out_uid)
{
    bool ok = false;

    storage_init();
    storage_lock();

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

    for (int i = 0; i < g_user_count; i++)
    {
        if (0 == strcmp(g_users[i].uid, uid_str))
        {
            storage_copy_text(g_users[i].name, sizeof(g_users[i].name), name, strlen(name));
            storage_copy_text(g_recent_user.uid, sizeof(g_recent_user.uid), uid_str, strlen(uid_str));
            storage_copy_text(g_recent_user.name, sizeof(g_recent_user.name), name, strlen(name));
            g_recent_user_valid = true;
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

            ok = true;
            break;
        }
    }

    storage_unlock();
    return ok;
}
