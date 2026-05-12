#ifndef MAIN_H
#define MAIN_H

#include "bsp_api.h"
#include "tx_api.h"
#include "nx_api.h"
#include "hal_data.h"
#include "main_thread.h" /* Essencial: Traz as instâncias geradas pelo configurador */
#include <stdbool.h>
#include <string.h>

/* Endereços do relay */
extern const char * RELAY_HOST;
extern const ULONG     RELAY_IP_ADDR; /* IP Numérico para os Sockets NetX */
extern const uint16_t  RELAY_PORT;
extern const char * DEVICE_ID;
extern const char * API_KEY;

#define UID_MAX_LEN     21
#define NAME_MAX_LEN    32
#define EVENT_QUEUE_SIZE 16
#define ACCESS_LOG_SIZE  32
#define APP_METRIC_LOG_SIZE 128

typedef enum
{
    APP_UI_MODE_IDLE = 0,
    APP_UI_MODE_PIN_ENTRY,
    APP_UI_MODE_ENROLL_WAIT,
} app_ui_mode_t;

/* Tipos de evento */
typedef enum {
    EVENT_RFID_AUTH_OK,
    EVENT_RFID_AUTH_FAIL,
    EVENT_DOOR_OPEN,
    EVENT_DOOR_CLOSE,
    EVENT_LIGHT_ON,
    EVENT_LIGHT_OFF,
    EVENT_USER_ADDED,
    EVENT_USER_REMOVED,
    EVENT_UI_ADMIN_REQUEST,
    EVENT_UI_NAV_INC,
    EVENT_UI_NAV_CONFIRM,
    EVENT_UI_NAV_CANCEL,
    EVENT_CARD_REGISTERED,
    EVENT_CARD_ALREADY_REGISTERED,
    EVENT_CARD_REGISTRATION_FAILED,
    EVENT_UI_SHOW_IP,
} app_event_type_t;

typedef struct {
    app_event_type_t type;
    char             data[UID_MAX_LEN];
} app_event_t;

typedef struct {
    ULONG tick;
    ULONG unix_utc;
    app_event_type_t type;
    char data[UID_MAX_LEN];
    char user[NAME_MAX_LEN];
} app_access_log_entry_t;

typedef enum
{
    APP_METRIC_KIND_UNKNOWN = 0,
    APP_METRIC_KIND_RFID_READ,
    APP_METRIC_KIND_AUTH,
    APP_METRIC_KIND_DOOR_ACTUATION,
    APP_METRIC_KIND_HTTP,
    APP_METRIC_KIND_JSON_IMPORT,
    APP_METRIC_KIND_STORAGE_PERSIST,
    APP_METRIC_KIND_STORAGE_LOAD,
    APP_METRIC_KIND_UI_RESULT,
    APP_METRIC_KIND_REBOOT_PERSISTENCE,
} app_metric_kind_t;

typedef enum
{
    APP_METRIC_CASE_UNKNOWN = 0,
    APP_METRIC_CASE_UID_4_BYTES,
    APP_METRIC_CASE_UID_7_BYTES,
    APP_METRIC_CASE_UID_10_BYTES,
    APP_METRIC_CASE_AUTH_OK,
    APP_METRIC_CASE_AUTH_DENIED,
    APP_METRIC_CASE_ROUTE_LOGIN,
    APP_METRIC_CASE_ROUTE_ADMIN_PROFILES,
    APP_METRIC_CASE_ROUTE_IMPORT_PROFILES,
    APP_METRIC_CASE_ROUTE_UPLOAD_PHOTO,
    APP_METRIC_CASE_ROUTE_METRICS,
    APP_METRIC_CASE_IMPORT_10_PROFILES,
    APP_METRIC_CASE_IMPORT_50_PROFILES,
    APP_METRIC_CASE_IMPORT_50_PLUS_PROFILES,
    APP_METRIC_CASE_USERS_JSON,
    APP_METRIC_CASE_ACCESS_LOG,
    APP_METRIC_CASE_METRICS_LOG,
    APP_METRIC_CASE_PHOTO,
    APP_METRIC_CASE_AFTER_REBOOT,
} app_metric_case_t;

typedef struct {
    ULONG tick;
    ULONG unix_utc;
    ULONG duration_ticks;
    app_metric_kind_t kind;
    app_metric_case_t case_id;
    bool success;
} app_metric_entry_t;

/* Estado global da aplicação */
typedef struct {
    bool    door_open;
    bool    light_on;
    char    last_uid[UID_MAX_LEN];
    char    last_user[NAME_MAX_LEN];
    bool    uid_pending;
    bool    net_ready;
    bool    enroll_mode;
    app_ui_mode_t ui_mode;
} app_state_t;

extern volatile app_state_t g_app_state;
extern TX_QUEUE             g_event_queue;

/* API de controlo */
void app_state_lock   (void);
void app_state_unlock (void);
void app_set_door     (bool open);
void app_set_light    (bool on);
void app_set_last_identity(const char *uid, const char *user);
void app_set_ui_mode(app_ui_mode_t mode);
app_ui_mode_t app_get_ui_mode(void);
void app_set_enrollment_mode(bool enabled);
bool app_is_enrollment_mode(void);
void app_post_event   (app_event_type_t type, const char * data);
int app_access_log_snapshot(app_access_log_entry_t *out_entries, int max_entries);
void app_access_log_restore(const app_access_log_entry_t *entries, int entry_count);
void app_time_set_utc(ULONG unix_utc);
bool app_time_get_utc(ULONG *out_unix_utc);
void app_format_access_log_timestamp(const app_access_log_entry_t *entry, char *out, size_t out_size);
void app_metric_add(const char *metric_name, const char *case_name, ULONG duration_ticks, bool success);
void app_metric_add_no_persist(const char *metric_name, const char *case_name, ULONG duration_ticks, bool success);
int app_metric_snapshot(app_metric_entry_t *out_entries, int max_entries);
void app_metric_restore(const app_metric_entry_t *entries, int entry_count);
void app_format_metric_timestamp(const app_metric_entry_t *entry, char *out, size_t out_size);
const char *app_metric_kind_text(app_metric_kind_t kind);
const char *app_metric_case_text(app_metric_case_t case_id);
void app_metric_begin_door(const char *case_name);
void app_metric_finish_door(bool success);
void app_metric_begin_ui_result(const char *case_name);
void app_metric_finish_ui_result(bool success);

/* Protótipos das Threads */
void thread_rfid_entry   (ULONG arg);
void thread_ui_entry     (ULONG arg);
void thread_net_entry    (ULONG arg);
void thread_tunnel_entry (ULONG arg);
void thread_gpio_entry   (ULONG arg);
void thread_status_entry (ULONG arg);

#endif /* MAIN_H */
