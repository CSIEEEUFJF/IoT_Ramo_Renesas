#include "main.h"
#include "net.h"
#include "rfid.h"
#include "ui.h"
#include "gpio.h"

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

void hal_entry(void)
{
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

    g_app_debug_event_queue_status = tx_queue_send(&g_event_queue, &ev, TX_NO_WAIT);
    if (TX_SUCCESS != g_app_debug_event_queue_status)
    {
        g_app_debug_event_queue_fail_count++;
    }
}
