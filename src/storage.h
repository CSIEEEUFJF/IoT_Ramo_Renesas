#ifndef STORAGE_H
#define STORAGE_H
#include <stdbool.h>
#include "main.h"

#define STORAGE_MAX_USERS           100
#define STORAGE_MAX_CARDS_PER_USER    4
#define STORAGE_ROLE_MAX_LEN         48
#define STORAGE_CHAPTER_MAX_LEN      48
#define STORAGE_PHOTO_ID_MAX_LEN     64
#define STORAGE_ADMIN_PIN_LEN         4
#define STORAGE_ADMIN_PIN_MAX_LEN    (STORAGE_ADMIN_PIN_LEN + 1)
#define STORAGE_RUNTIME_PHOTO_MAX_DIM 160
#define STORAGE_MEETING_SCHEDULE_MAX_ITEMS 8
#define STORAGE_MEETING_RECURRENCE_NONE   0U
#define STORAGE_MEETING_RECURRENCE_DAILY  1U
#define STORAGE_MEETING_RECURRENCE_WEEKLY 2U
#define STORAGE_MEETING_WEEKDAY_MASK_ALL  0x7FU
#define STORAGE_MEETING_MIN_DURATION_SECONDS (1UL * 60UL)
#define STORAGE_MEETING_MAX_DURATION_SECONDS (24UL * 60UL * 60UL)
#define STORAGE_MEETING_DEFAULT_DURATION_SECONDS (60UL * 60UL)

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

typedef struct
{
    ULONG id;
    ULONG start_unix;
    ULONG end_unix;
    unsigned int profile_count;
    uint8_t profile_indices[STORAGE_MAX_USERS];
    unsigned int profile_key_count;
    uint32_t profile_key_hashes[STORAGE_MAX_USERS];
    uint8_t recurrence;
    uint8_t weekdays_mask;
    char meeting_chapter[STORAGE_CHAPTER_MAX_LEN];
} storage_meeting_schedule_t;

typedef struct
{
    ULONG start_unix;
    ULONG end_unix;
    unsigned int profile_count;
    uint8_t profile_indices[STORAGE_MAX_USERS];
    unsigned int profile_key_count;
    uint32_t profile_key_hashes[STORAGE_MAX_USERS];
    char meeting_chapter[STORAGE_CHAPTER_MAX_LEN];
} storage_meeting_active_t;

typedef enum
{
    STORAGE_ACCESS_RESULT_NOT_FOUND = 0,
    STORAGE_ACCESS_RESULT_GRANTED,
    STORAGE_ACCESS_RESULT_DENIED_MEETING_MODE,
} storage_access_result_t;

typedef struct
{
    unsigned int persist_status;
    bool thread_ready;
    bool loaded;
    bool load_failed;
    ULONG worker_runs;
    ULONG last_stage;
    ULONG last_media_status;
    ULONG last_save_status;
    ULONG last_load_status;
    ULONG last_saved_bytes;
    ULONG last_read_bytes;
    ULONG last_user_count;
    ULONG last_json_size;
    ULONG direct_persist_requests;
    ULONG direct_persist_successes;
    ULONG format_status;
    ULONG erase_status;
    ULONG erase_step;
} storage_debug_info_t;

void storage_init(void);
bool storage_check_uid(const char *uid_str, char *out_name);
storage_access_result_t storage_authorize_uid(const char *uid_str, char *out_name);
int  storage_user_count(void);
int  storage_user_count_nowait(void);
bool storage_users_loaded_nowait(void);
bool storage_user_get(int index, char *out_name, char *out_uid);
bool storage_add_user(const char *uid_str, const char *name);
bool storage_remove_uid(const char *uid_str);
int storage_profile_snapshot(storage_user_profile_t *out_profiles, int max_profiles);
int storage_profile_snapshot_nowait(storage_user_profile_t *out_profiles, int max_profiles);
bool storage_profile_get(int index, storage_user_profile_t *out_profile);
bool storage_profile_get_nowait(int index, storage_user_profile_t *out_profile);
bool storage_profile_find_by_uid(const char *uid_str, storage_user_profile_t *out_profile);
bool storage_profile_upsert(const storage_user_profile_t *profile, int edit_index);
bool storage_profile_remove(int index);
bool storage_meeting_profile_key_hash_for_index(int index, uint32_t *out_hash);
bool storage_meeting_mode_start(const int *profile_indices,
                                int profile_count,
                                const char *meeting_chapter,
                                ULONG start_unix,
                                ULONG end_unix,
                                unsigned int *out_selected_profiles,
                                unsigned int *out_allowed_cards);
bool storage_meeting_mode_restore(const int *profile_indices,
                                  int profile_count,
                                  const char *meeting_chapter,
                                  ULONG start_unix,
                                  ULONG end_unix,
                                  unsigned int *out_selected_profiles,
                                  unsigned int *out_allowed_cards);
void storage_meeting_mode_stop(void);
bool storage_meeting_mode_is_active(void);
bool storage_meeting_mode_chapter(char *out_chapter, size_t out_size);
bool storage_meeting_mode_end_utc(ULONG *out_end_unix);
unsigned int storage_meeting_mode_selected_profile_count(void);
unsigned int storage_meeting_mode_allowed_card_count(void);
bool storage_meeting_mode_profile_selected(const storage_user_profile_t *profile);
int storage_meeting_active_load(storage_meeting_active_t *out_state);
bool storage_meeting_active_clear(void);
int storage_meeting_schedule_load(storage_meeting_schedule_t *out_schedules, int max_schedules);
bool storage_meeting_schedule_save(const storage_meeting_schedule_t *schedules, int schedule_count);
bool storage_admin_pin_valid(const char *pin);
bool storage_admin_pin_configured(void);
bool storage_persist_now(void);
bool storage_persist_now_direct(void);
bool storage_format_qspi_and_persist(void);
bool storage_persist_wait(ULONG timeout_ticks);
unsigned int storage_persist_status(void);
void storage_debug_snapshot(storage_debug_info_t *out_info);
bool storage_access_log_enqueue(const app_access_log_entry_t *entry);
bool storage_access_log_persist_now(void);
bool storage_access_log_ensure_loaded(void);
bool storage_metric_enqueue(const app_metric_entry_t *entry);
bool storage_metrics_persist_now(void);
bool storage_metrics_ensure_loaded(void);
bool storage_export_users_json_info(ULONG *out_size);
bool storage_export_users_json(char *out, size_t out_size, size_t *out_len);
bool storage_export_users_json_read(ULONG offset, void *out, size_t out_size, size_t *out_read);
bool storage_photo_export_info(const char *photo_id, storage_photo_export_info_t *out_info);
bool storage_photo_export_read(const char *photo_id, ULONG offset, void *out, size_t out_size, size_t *out_read);
bool storage_photo_persist_now(const char *photo_id);
bool storage_photo_ensure_loaded(const char *photo_id);

#endif /* STORAGE_H */
