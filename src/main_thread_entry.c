#include "main_thread.h"

/* Neutraliza callbacks gerados pelo SSP que derrubavam a aplicacao. */
void tx_startup_err_callback(void *p_instance, void *p_data);

void g_ip0_err_callback(void *p_instance, void *p_data)
{
    SSP_PARAMETER_NOT_USED(p_instance);
    SSP_PARAMETER_NOT_USED(p_data);
}

void g_packet_pool0_err_callback(void *p_instance, void *p_data)
{
    SSP_PARAMETER_NOT_USED(p_instance);
    SSP_PARAMETER_NOT_USED(p_data);
}

void g_http_server0_err_callback(void *p_instance, void *p_data)
{
    SSP_PARAMETER_NOT_USED(p_instance);
    SSP_PARAMETER_NOT_USED(p_data);
}

void g_fx_media0_err_callback(void *p_instance, void *p_data)
{
    SSP_PARAMETER_NOT_USED(p_instance);
    SSP_PARAMETER_NOT_USED(p_data);
}

void tx_startup_err_callback(void *p_instance, void *p_data)
{
    SSP_PARAMETER_NOT_USED(p_instance);
    SSP_PARAMETER_NOT_USED(p_data);
}

void main_thread_entry(void)
{
    static bool app_started = false;

    if (!app_started)
    {
        /* Num projeto ThreadX, a aplicacao precisa ser disparada daqui. */
        app_started = true;
        hal_entry();
    }

    while (1)
    {
        tx_thread_sleep(100);
    }
}
