#ifndef STORAGE_H
#define STORAGE_H
#include <stdbool.h>
#include "main.h"

#define STORAGE_MAX_USERS            50
#define STORAGE_MAX_CARDS_PER_USER    4
#define STORAGE_ROLE_MAX_LEN         48
#define STORAGE_CHAPTER_MAX_LEN      48
#define STORAGE_PHOTO_ID_MAX_LEN     64
#define STORAGE_ADMIN_PIN_LEN         4
#define STORAGE_ADMIN_PIN_MAX_LEN    (STORAGE_ADMIN_PIN_LEN + 1)
#define STORAGE_RUNTIME_PHOTO_MAX_DIM 160

typedef struct
{
    char name[NAME_MAX_LEN];
    char role[STORAGE_ROLE_MAX_LEN];
    char chapter[STORAGE_CHAPTER_MAX_LEN];
    char photo_id[STORAGE_PHOTO_ID_MAX_LEN];
    bool is_admin;
    char admin_pin[STORAGE_ADMIN_PIN_MAX_LEN];
    unsigned int card_count;
    char cards[STORAGE_MAX_CARDS_PER_USER][UID_MAX_LEN];
} storage_user_profile_t;

typedef struct
{
    uint16_t width;
    uint16_t height;
    ULONG payload_size;
    ULONG total_size;
} storage_photo_export_info_t;

void storage_init(void);
bool storage_check_uid(const char *uid_str, char *out_name);
int  storage_user_count(void);
bool storage_user_get(int index, char *out_name, char *out_uid);
bool storage_add_user(const char *uid_str, const char *name);
bool storage_remove_uid(const char *uid_str);
int storage_profile_snapshot(storage_user_profile_t *out_profiles, int max_profiles);
int storage_profile_snapshot_nowait(storage_user_profile_t *out_profiles, int max_profiles);
bool storage_profile_get(int index, storage_user_profile_t *out_profile);
bool storage_profile_find_by_uid(const char *uid_str, storage_user_profile_t *out_profile);
bool storage_profile_upsert(const storage_user_profile_t *profile, int edit_index);
bool storage_profile_remove(int index);
bool storage_admin_pin_valid(const char *pin);
bool storage_persist_now(void);
bool storage_persist_wait(ULONG timeout_ticks);
unsigned int storage_persist_status(void);
bool storage_access_log_persist_now(void);
bool storage_access_log_ensure_loaded(void);
bool storage_metrics_persist_now(void);
bool storage_metrics_ensure_loaded(void);
bool storage_export_users_json(char *out, size_t out_size, size_t *out_len);
bool storage_photo_export_info(const char *photo_id, storage_photo_export_info_t *out_info);
bool storage_photo_export_read(const char *photo_id, ULONG offset, void *out, size_t out_size, size_t *out_read);
bool storage_photo_persist_now(const char *photo_id);
bool storage_photo_ensure_loaded(const char *photo_id);

#endif /* STORAGE_H */
