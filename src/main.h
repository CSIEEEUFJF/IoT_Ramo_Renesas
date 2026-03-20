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
} app_event_type_t;

typedef struct {
    app_event_type_t type;
    char             data[UID_MAX_LEN];
} app_event_t;

typedef struct {
    ULONG tick;
    app_event_type_t type;
    char data[UID_MAX_LEN];
    char user[NAME_MAX_LEN];
} app_access_log_entry_t;

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

/* Protótipos das Threads */
void thread_rfid_entry   (ULONG arg);
void thread_ui_entry     (ULONG arg);
void thread_net_entry    (ULONG arg);
void thread_tunnel_entry (ULONG arg);
void thread_gpio_entry   (ULONG arg);
void thread_status_entry (ULONG arg);

#endif /* MAIN_H */
