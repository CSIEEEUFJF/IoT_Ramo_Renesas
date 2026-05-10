#include "gpio.h"
#include "main.h"

/* Header Arduino: D6 = P613 (porta), D5 = P608 (luz), D4 = P112 (botao externo da porta). */
#define PIN_RELAY_DOOR   IOPORT_PORT_06_PIN_13
#define PIN_RELAY_LIGHT  IOPORT_PORT_06_PIN_08
#define PIN_BTN_DOOR     IOPORT_PORT_00_PIN_08
#define PIN_BTN_LIGHT    IOPORT_PORT_00_PIN_09
#define PIN_BTN_DOOR_EXT IOPORT_PORT_01_PIN_12

#define DOOR_OPEN_TIME_MS  2000
#define BTN_POLL_MS         50
#define BTN_ADMIN_HOLD_TICKS ((TX_TIMER_TICKS_PER_SECOND * 3U) / 2U)

static volatile ULONG g_door_open_command_seq = 0U;

void gpio_init(void)
{
    /* Estes pinos nao sao inicializados pelo pin_data gerado, entao forçamos o modo GPIO. */
    (void) g_ioport.p_api->pinCfg(PIN_RELAY_DOOR,
                                  IOPORT_CFG_PORT_DIRECTION_OUTPUT |
                                  IOPORT_CFG_PORT_OUTPUT_HIGH |
                                  IOPORT_CFG_DRIVE_MID);
    (void) g_ioport.p_api->pinCfg(PIN_RELAY_LIGHT,
                                  IOPORT_CFG_PORT_DIRECTION_OUTPUT |
                                  IOPORT_CFG_PORT_OUTPUT_LOW |
                                  IOPORT_CFG_DRIVE_MID);
    (void) g_ioport.p_api->pinCfg(PIN_BTN_DOOR_EXT,
                                  IOPORT_CFG_PORT_DIRECTION_INPUT |
                                  IOPORT_CFG_PULLUP_ENABLE);
    g_ioport.p_api->pinWrite(PIN_RELAY_DOOR, IOPORT_LEVEL_HIGH);
    g_ioport.p_api->pinWrite(PIN_RELAY_LIGHT, IOPORT_LEVEL_LOW);
}

void gpio_set_door(bool open)
{
    g_ioport.p_api->pinWrite(PIN_RELAY_DOOR, open ? IOPORT_LEVEL_LOW : IOPORT_LEVEL_HIGH);
    if (open)
    {
        g_door_open_command_seq++;
    }
}

void gpio_set_light(bool on)
{
    g_ioport.p_api->pinWrite(PIN_RELAY_LIGHT, on ? IOPORT_LEVEL_HIGH : IOPORT_LEVEL_LOW);
}

void thread_gpio_entry(ULONG arg)
{
    SSP_PARAMETER_NOT_USED(arg);

    ioport_level_t btn_door_level, btn_light_level, btn_door_ext_level;
    bool last_btn_door_pressed = false;
    bool last_btn_light_pressed = false;
    bool last_btn_door_ext_pressed = false;
    bool combo_tracking = false;
    bool combo_fired = false;
    ULONG combo_start_tick = 0U;

    ULONG door_open_tick = 0;
    ULONG last_door_open_command_seq = 0U;
    bool is_door_pulsing = false;
    const ULONG DOOR_PULSE_TICKS = (DOOR_OPEN_TIME_MS * TX_TIMER_TICKS_PER_SECOND) / 1000;

    while (1)
    {
        bool btn_door_pressed;
        bool btn_light_pressed;
        bool btn_door_ext_pressed;
        app_ui_mode_t ui_mode;

        g_ioport.p_api->pinRead(PIN_BTN_DOOR, &btn_door_level);
        g_ioport.p_api->pinRead(PIN_BTN_LIGHT, &btn_light_level);
        g_ioport.p_api->pinRead(PIN_BTN_DOOR_EXT, &btn_door_ext_level);
        btn_door_pressed = (IOPORT_LEVEL_LOW == btn_door_level);
        btn_light_pressed = (IOPORT_LEVEL_LOW == btn_light_level);
        btn_door_ext_pressed = (IOPORT_LEVEL_LOW == btn_door_ext_level);
        ui_mode = app_get_ui_mode();

        if (btn_door_pressed && btn_light_pressed)
        {
            if (!combo_tracking)
            {
                combo_tracking = true;
                combo_fired = false;
                combo_start_tick = tx_time_get();
            }
            else if ((!combo_fired) && ((tx_time_get() - combo_start_tick) >= BTN_ADMIN_HOLD_TICKS))
            {
                app_post_event((APP_UI_MODE_IDLE == ui_mode) ? EVENT_UI_ADMIN_REQUEST : EVENT_UI_NAV_CANCEL, NULL);
                combo_fired = true;
            }
        }
        else
        {
            if (combo_tracking)
            {
                combo_tracking = false;
                combo_start_tick = 0U;

                if (combo_fired)
                {
                    combo_fired = false;
                }
                else
                {
                    if (APP_UI_MODE_IDLE == ui_mode)
                    {
                        if (last_btn_door_pressed && !btn_door_pressed)
                        {
                            app_post_event(EVENT_DOOR_OPEN, "BOTAO");
                        }

                        if (last_btn_light_pressed && !btn_light_pressed)
                        {
                            app_state_lock();
                            bool current_light = g_app_state.light_on;
                            app_state_unlock();

                            if (current_light)
                            {
                                app_post_event(EVENT_LIGHT_OFF, NULL);
                            }
                            else
                            {
                                app_post_event(EVENT_LIGHT_ON, NULL);
                            }
                        }
                    }
                    else
                    {
                        if (last_btn_door_pressed && !btn_door_pressed)
                        {
                            app_post_event(EVENT_UI_NAV_INC, NULL);
                        }

                        if (last_btn_light_pressed && !btn_light_pressed)
                        {
                            app_post_event(EVENT_UI_NAV_CONFIRM, NULL);
                        }
                    }
                }
            }
            else
            {
                if (APP_UI_MODE_IDLE == ui_mode)
                {
                    if (last_btn_door_pressed && !btn_door_pressed)
                    {
                        app_post_event(EVENT_DOOR_OPEN, "BOTAO");
                    }

                    if (last_btn_light_pressed && !btn_light_pressed)
                    {
                        app_state_lock();
                        bool current_light = g_app_state.light_on;
                        app_state_unlock();

                        if (current_light)
                        {
                            app_post_event(EVENT_LIGHT_OFF, NULL);
                        }
                        else
                        {
                            app_post_event(EVENT_LIGHT_ON, NULL);
                        }
                    }
                }
                else
                {
                    if (last_btn_door_pressed && !btn_door_pressed)
                    {
                        app_post_event(EVENT_UI_NAV_INC, NULL);
                    }

                    if (last_btn_light_pressed && !btn_light_pressed)
                    {
                        app_post_event(EVENT_UI_NAV_CONFIRM, NULL);
                    }
                }
            }
        }

        if (!last_btn_door_ext_pressed && btn_door_ext_pressed)
        {
            app_post_event(EVENT_DOOR_OPEN, "BOTAO_D4");
        }

        last_btn_door_pressed = btn_door_pressed;
        last_btn_light_pressed = btn_light_pressed;
        last_btn_door_ext_pressed = btn_door_ext_pressed;

        app_state_lock();
        bool current_door = g_app_state.door_open;
        app_state_unlock();

        if (current_door && (last_door_open_command_seq != g_door_open_command_seq)) {
            is_door_pulsing = true;
            door_open_tick = tx_time_get();
            last_door_open_command_seq = g_door_open_command_seq;
        } else if (current_door && is_door_pulsing) {
            if ((tx_time_get() - door_open_tick) >= DOOR_PULSE_TICKS) {
                app_post_event(EVENT_DOOR_CLOSE, NULL);
                is_door_pulsing = false;
            }
        } else if (!current_door) {
            is_door_pulsing = false;
        }

        tx_thread_sleep((BTN_POLL_MS * TX_TIMER_TICKS_PER_SECOND) / 1000);
    }
}
