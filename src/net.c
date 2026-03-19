#include "net.h"
#include "main.h"
#include "storage.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern NX_IP g_ip0;
extern NX_HTTP_SERVER g_http_server0;
extern NX_REC nx_record1;

volatile ULONG g_net_debug_state = 0U;
volatile ULONG g_net_debug_link_status = 0U;
volatile ULONG g_net_debug_ip_status = 0U;
volatile ULONG g_net_debug_ip_address = 0U;
volatile ULONG g_net_debug_network_mask = 0U;
volatile ULONG g_net_debug_http_status = 0U;
volatile ULONG g_net_debug_media_status = 0U;
volatile ULONG g_net_debug_request_count = 0U;
volatile ULONG g_net_debug_last_request_type = 0U;
volatile ULONG g_net_debug_last_response_status = 0U;
volatile char  g_net_debug_last_resource[64] = {0};
volatile ULONG g_net_debug_ip_packets_sent = 0U;
volatile ULONG g_net_debug_ip_packets_received = 0U;
volatile ULONG g_net_debug_arp_requests_sent = 0U;
volatile ULONG g_net_debug_arp_requests_received = 0U;
volatile ULONG g_net_debug_arp_responses_sent = 0U;
volatile ULONG g_net_debug_arp_responses_received = 0U;
volatile ULONG g_net_debug_ip_invalid_packets = 0U;
volatile ULONG g_net_debug_ip_receive_packets_dropped = 0U;
volatile ULONG g_net_debug_ip_receive_checksum_errors = 0U;
volatile ULONG g_net_debug_arp_invalid_messages = 0U;
volatile ULONG g_net_debug_edmac_eesr = 0U;
volatile ULONG g_net_debug_etherc_ecsr = 0U;
volatile ULONG g_net_debug_etherc_psr = 0U;
volatile ULONG g_net_debug_edmac0_eesr = 0U;
volatile ULONG g_net_debug_edmac1_eesr = 0U;
volatile ULONG g_net_debug_etherc0_ecsr = 0U;
volatile ULONG g_net_debug_etherc1_ecsr = 0U;
volatile ULONG g_net_debug_etherc0_psr = 0U;
volatile ULONG g_net_debug_etherc1_psr = 0U;
volatile ULONG g_net_debug_driver_irq = 0U;
volatile ULONG g_net_debug_driver_irq_enabled = 0U;
volatile ULONG g_net_debug_driver_state = 0U;
volatile ULONG g_net_debug_driver_link_established = 0U;
volatile ULONG g_net_debug_driver_irq_count = 0U;
volatile ULONG g_net_debug_driver_tx_irq_count = 0U;
volatile ULONG g_net_debug_driver_rx_irq_count = 0U;
volatile ULONG g_net_debug_driver_eci_irq_count = 0U;
volatile ULONG g_net_debug_driver_error_irq_count = 0U;
volatile ULONG g_net_debug_driver_error_bits = 0U;
volatile ULONG g_net_debug_driver_last_eesr = 0U;
volatile ULONG g_net_debug_driver_last_ecsr = 0U;
volatile ULONG g_net_debug_driver_rx_bd_index = 0U;
volatile ULONG g_net_debug_driver_rx_bd0_status = 0U;
volatile ULONG g_net_debug_driver_edrrr = 0U;
volatile ULONG g_net_debug_driver_edmr = 0U;
volatile ULONG g_net_debug_driver_ecmr = 0U;
volatile ULONG g_net_debug_driver_tx_bd0_status = 0U;
volatile ULONG g_net_debug_driver_edtrr = 0U;
volatile ULONG g_net_debug_gratuitous_arp_status = 0U;
volatile ULONG g_net_debug_pmisc_pfenet = 0U;
volatile ULONG g_net_debug_phy_control = 0U;
volatile ULONG g_net_debug_phy_status = 0U;
volatile ULONG g_net_debug_phy_id1 = 0U;
volatile ULONG g_net_debug_phy_id2 = 0U;
volatile ULONG g_net_debug_phy_advertise = 0U;
volatile ULONG g_net_debug_phy_partner = 0U;
volatile ULONG g_net_debug_phy_rxer_counter = 0U;
volatile ULONG g_net_debug_phy_opmode = 0U;
volatile ULONG g_net_debug_phy_opmode_status = 0U;
volatile ULONG g_net_debug_phy_interrupt_status = 0U;
volatile ULONG g_net_debug_phy_control1 = 0U;
volatile ULONG g_net_debug_phy_control2 = 0U;
volatile ULONG g_net_debug_loopback_stage = 0U;
volatile ULONG g_net_debug_loopback_saved_control = 0U;
volatile ULONG g_net_debug_loopback_send_status = 0xFFFFFFFFU;
volatile ULONG g_net_debug_loopback_rx_irq_before = 0U;
volatile ULONG g_net_debug_loopback_rx_irq_after = 0U;
volatile ULONG g_net_debug_loopback_tick = 0U;
volatile ULONG g_net_debug_linkmd_stage = 0U;
volatile ULONG g_net_debug_linkmd_status = 0U;
volatile ULONG g_net_debug_linkmd_result = 0U;
volatile ULONG g_net_debug_linkmd_distance = 0U;

#define HTML_BUFFER_SIZE         8192
#define RESOURCE_BUFFER_SIZE      96
#define QUERY_BUFFER_SIZE        256
#define DEVICE_IP_ADDR           IP_ADDRESS(192, 168, 15, 180)
#define DEVICE_NETMASK           IP_ADDRESS(255, 255, 255, 0)
#define DEVICE_GATEWAY_ADDR      IP_ADDRESS(192, 168, 15, 1)

static void network_stack_init_once(void)
{
    static bool initialized = false;

    if (initialized)
    {
        return;
    }

    g_net_debug_state = 1U;
    packet_pool_init0();
    ip_init0();
    http_server_init0();
    g_net_debug_state = 2U;
    initialized = true;
}

static void ip_to_string(ULONG ip_address, char *out, size_t out_size)
{
    snprintf(out, out_size, "%lu.%lu.%lu.%lu",
             (unsigned long) ((ip_address >> 24) & 0xFFUL),
             (unsigned long) ((ip_address >> 16) & 0xFFUL),
             (unsigned long) ((ip_address >> 8) & 0xFFUL),
             (unsigned long) (ip_address & 0xFFUL));
}

static void url_decode(char *text)
{
    char *src = text;
    char *dst = text;

    while ((NULL != src) && ('\0' != *src))
    {
        if ('+' == *src)
        {
            *dst++ = ' ';
            src++;
        }
        else if (('%' == *src) &&
                 (('\0' != src[1]) && ('\0' != src[2])))
        {
            char hex[3];
            char *end_ptr = NULL;
            long value;

            hex[0] = src[1];
            hex[1] = src[2];
            hex[2] = '\0';
            value = strtol(hex, &end_ptr, 16);

            if ((NULL != end_ptr) && ('\0' == *end_ptr))
            {
                *dst++ = (char) value;
                src += 3;
            }
            else
            {
                *dst++ = *src++;
            }
        }
        else
        {
            *dst++ = *src++;
        }
    }

    *dst = '\0';
}

static void sanitize_text(char *text)
{
    size_t write_index = 0U;

    for (size_t read_index = 0U; text[read_index] != '\0'; read_index++)
    {
        char ch = text[read_index];

        if ((ch == '"') || (ch == '\\') || (ch == '\r') || (ch == '\n') ||
            (ch == '<') || (ch == '>'))
        {
            continue;
        }

        text[write_index++] = ch;
    }

    text[write_index] = '\0';
}

static bool query_get_value(const char *query, const char *key, char *out, size_t out_size)
{
    size_t key_len = strlen(key);
    const char *cursor = query;

    if ((NULL == query) || (NULL == key) || (NULL == out) || (0U == out_size))
    {
        return false;
    }

    while ((NULL != cursor) && ('\0' != *cursor))
    {
        const char *next = strchr(cursor, '&');
        size_t token_len = (NULL == next) ? strlen(cursor) : (size_t) (next - cursor);

        if ((token_len > key_len) &&
            (0 == strncmp(cursor, key, key_len)) &&
            ('=' == cursor[key_len]))
        {
            size_t value_len = token_len - key_len - 1U;

            if (value_len >= out_size)
            {
                value_len = out_size - 1U;
            }

            memcpy(out, cursor + key_len + 1U, value_len);
            out[value_len] = '\0';
            url_decode(out);
            sanitize_text(out);
            return true;
        }

        cursor = (NULL == next) ? NULL : (next + 1);
    }

    return false;
}

static void split_resource_and_query(const char *resource,
                                     char *path_out,
                                     size_t path_size,
                                     char *query_out,
                                     size_t query_size)
{
    const char *query = strchr(resource, '?');
    size_t path_len = (NULL == query) ? strlen(resource) : (size_t) (query - resource);

    if (path_len >= path_size)
    {
        path_len = path_size - 1U;
    }

    memcpy(path_out, resource, path_len);
    path_out[path_len] = '\0';

    if ((NULL != query) && (query_size > 0U))
    {
        strncpy(query_out, query + 1, query_size - 1U);
        query_out[query_size - 1U] = '\0';
    }
    else if (query_size > 0U)
    {
        query_out[0] = '\0';
    }
}

static UINT send_html_response(NX_HTTP_SERVER *server, NX_PACKET *packet_ptr, const char *html)
{
    UINT status;

    status = nx_http_server_callback_generate_response_header(server,
                                                              &packet_ptr,
                                                              NX_HTTP_STATUS_OK,
                                                              strlen(html),
                                                              "text/html",
                                                              NULL);
    if (NX_SUCCESS != status)
    {
        g_net_debug_last_response_status = status;
        return status;
    }

    status = nx_packet_data_append(packet_ptr,
                          (VOID *) html,
                          strlen(html),
                          server->nx_http_server_packet_pool_ptr,
                          NX_WAIT_FOREVER);
    if (NX_SUCCESS != status)
    {
        g_net_debug_last_response_status = status;
        return status;
    }

    status = nx_http_server_callback_packet_send(server, packet_ptr);
    if (NX_SUCCESS != status)
    {
        g_net_debug_last_response_status = status;
        return status;
    }

    g_net_debug_last_response_status = NX_HTTP_CALLBACK_COMPLETED;
    return NX_HTTP_CALLBACK_COMPLETED;
}

static UINT send_plain_response(NX_HTTP_SERVER *server, NX_PACKET *packet_ptr, const char *text)
{
    UINT status;

    status = nx_http_server_callback_generate_response_header(server,
                                                              &packet_ptr,
                                                              NX_HTTP_STATUS_OK,
                                                              strlen(text),
                                                              "text/plain",
                                                              NULL);
    if (NX_SUCCESS != status)
    {
        g_net_debug_last_response_status = status;
        return status;
    }

    status = nx_packet_data_append(packet_ptr,
                          (VOID *) text,
                          strlen(text),
                          server->nx_http_server_packet_pool_ptr,
                          NX_WAIT_FOREVER);
    if (NX_SUCCESS != status)
    {
        g_net_debug_last_response_status = status;
        return status;
    }

    status = nx_http_server_callback_packet_send(server, packet_ptr);
    if (NX_SUCCESS != status)
    {
        g_net_debug_last_response_status = status;
        return status;
    }

    g_net_debug_last_response_status = NX_HTTP_CALLBACK_COMPLETED;
    return NX_HTTP_CALLBACK_COMPLETED;
}

static UINT send_redirect(NX_HTTP_SERVER *server, const char *location)
{
    char header[128];
    NX_PACKET *resp_packet;

    snprintf(header,
             sizeof(header),
             "HTTP/1.1 303 See Other\r\nLocation: %s\r\nContent-Length: 0\r\n\r\n",
             location);

    if (NX_SUCCESS != nx_packet_allocate(server->nx_http_server_packet_pool_ptr,
                                         &resp_packet,
                                         NX_TCP_PACKET,
                                         NX_WAIT_FOREVER))
    {
        g_net_debug_last_response_status = NX_NOT_SUCCESSFUL;
        return NX_NOT_SUCCESSFUL;
    }

    if (NX_SUCCESS != nx_packet_data_append(resp_packet,
                                            (VOID *) header,
                                            strlen(header),
                                            server->nx_http_server_packet_pool_ptr,
                                            NX_WAIT_FOREVER))
    {
        nx_packet_release(resp_packet);
        g_net_debug_last_response_status = NX_NOT_SUCCESSFUL;
        return NX_NOT_SUCCESSFUL;
    }

    if (NX_SUCCESS != nx_http_server_callback_packet_send(server, resp_packet))
    {
        g_net_debug_last_response_status = NX_NOT_SUCCESSFUL;
        return NX_NOT_SUCCESSFUL;
    }

    g_net_debug_last_response_status = NX_HTTP_CALLBACK_COMPLETED;
    return NX_HTTP_CALLBACK_COMPLETED;
}

static UINT render_home_page(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, const char *message)
{
    /* ADICIONADO 'static' PARA EVITAR STACK OVERFLOW! */
    static char html[HTML_BUFFER_SIZE];
    static char rows[5200];

    char ip_text[20];
    char netmask_text[20];
    char gateway_text[20];
    char last_uid[UID_MAX_LEN];
    char last_user[NAME_MAX_LEN];

    /* Variáveis restauradas para a leitura do storage */
    char user_name[NAME_MAX_LEN];
    char user_uid[UID_MAX_LEN];
    const char *persist_text = "ociosa";

    ULONG ip_address;
    ULONG network_mask;
    ULONG link_status = 0U;
    size_t rows_len = 0U;
    int user_count;
    bool has_users = false;

    nx_ip_address_get(&g_ip0, &ip_address, &network_mask);
    ip_to_string(ip_address, ip_text, sizeof(ip_text));
    ip_to_string(network_mask, netmask_text, sizeof(netmask_text));
    ip_to_string(DEVICE_GATEWAY_ADDR, gateway_text, sizeof(gateway_text));
    (void) nx_ip_status_check(&g_ip0, NX_IP_LINK_ENABLED, &link_status, NX_NO_WAIT);

    app_state_lock();
    strncpy(last_uid, (const char *) g_app_state.last_uid, sizeof(last_uid) - 1U);
    last_uid[sizeof(last_uid) - 1U] = '\0';
    strncpy(last_user, (const char *) g_app_state.last_user, sizeof(last_user) - 1U);
    last_user[sizeof(last_user) - 1U] = '\0';
    app_state_unlock();

    rows[0] = '\0';
    user_count = storage_user_count();

    switch (storage_persist_status())
    {
        case 1U:
            persist_text = "pendente";
            break;
        case 2U:
            persist_text = "ok";
            break;
        case 3U:
            persist_text = "falhou";
            break;
        default:
            persist_text = "ociosa";
            break;
    }

    for (int i = 0; i < user_count; i++)
    {
        if (storage_user_get(i, user_name, user_uid))
        {
            int written = snprintf(&rows[rows_len],
                                   sizeof(rows) - rows_len,
                                   "<tr><td>%s</td><td>%s</td><td><a class='small danger' href='/remove_user?uid=%s'>Remover</a></td></tr>",
                                   user_name,
                                   user_uid,
                                   user_uid);

            if ((written > 0) && ((size_t) written < (sizeof(rows) - rows_len)))
            {
                rows_len += (size_t) written;
                has_users = true;
            }
        }
    }

    if (!has_users)
    {
        strncpy(rows,
                "<tr><td colspan='3'>Nenhum usuario cadastrado</td></tr>",
                sizeof(rows) - 1U);
        rows[sizeof(rows) - 1U] = '\0';
    }

    snprintf(html,
             sizeof(html),
             "<html><head><meta charset='utf-8'>"
             "<style>"
             "body{font-family:Arial,sans-serif;background:#f5f7fb;color:#18212d;margin:0;padding:20px;}"
             ".wrap{max-width:900px;margin:0 auto;}"
             ".card{background:#fff;border-radius:16px;padding:20px;margin-bottom:18px;box-shadow:0 8px 26px rgba(0,0,0,.08);}"
             "h1,h2{margin:0 0 12px 0;}p{margin:6px 0;}table{width:100%%;border-collapse:collapse;}"
             "td,th{padding:10px;border-bottom:1px solid #e7ebf2;text-align:left;}"
             "input{width:100%%;padding:12px;border:1px solid #ccd4e0;border-radius:10px;margin:8px 0 12px 0;box-sizing:border-box;}"
             ".btn,.small{display:inline-block;text-decoration:none;border:none;border-radius:10px;padding:12px 16px;background:#0b6ef3;color:#fff;}"
             ".small{padding:8px 12px;font-size:13px;}.danger{background:#d64545;}.muted{color:#607086;font-size:14px;}.ok{color:#137333;}"
             ".warn{color:#b26a00;}.grid{display:grid;grid-template-columns:1fr 1fr;gap:16px;}"
             "@media(max-width:700px){.grid{grid-template-columns:1fr;}}"
             "</style></head><body><div class='wrap'>"
             "<div class='card'><h1>S7G2 - Controle de acesso</h1>"
             "<p class='muted'>IP atual: <strong>%s</strong> | Mascara: <strong>%s</strong> | Gateway: <strong>%s</strong></p>"
             "<p class='%s'>Link Ethernet: <strong>%s</strong></p>"
             "<p class='muted'>Esta versao usa IP estatico. Se a sua rede nao estiver na faixa 192.168.15.x, a placa nao vai responder sem ajuste em <code>net.c</code>.</p>"
             "%s"
             "</div>"
             "<div class='grid'>"
             "<div class='card'><h2>Cadastrar usuario</h2>"
             "<form action='/add_user' method='get'>"
             "<label>Nome</label><input type='text' name='name' maxlength='31' placeholder='Nome do usuario'>"
             "<label>UID</label><input type='text' name='uid' maxlength='20' value='%s' placeholder='UID lido do cartao'>"
             "<button class='btn' type='submit'>Salvar usuario</button></form>"
             "<p><a class='small' href='/save_users'>Persistir cadastros</a></p>"
             "<p class='muted'>Persistencia QSPI: <strong>%s</strong></p>"
             "<p class='muted'>Ultimo cartao lido: <strong>%s</strong></p>"
             "<p class='muted'>Ultimo usuario: <strong>%s</strong></p>"
             "<p><a class='small' href='/portaon'>Abrir porta</a></p></div>"
             "<div class='card'><h2>Usuarios cadastrados</h2>"
             "<table><tr><th>Nome</th><th>UID</th><th>Acao</th></tr>%s</table>"
             "</div></div></div></body></html>",
             ip_text,
             netmask_text,
             gateway_text,
             (0U != link_status) ? "ok" : "warn",
             (0U != link_status) ? "conectado" : "sem link",
             (NULL != message) ? message : "",
             last_uid,
             persist_text,
             (last_uid[0] != '\0') ? last_uid : "-",
             (last_user[0] != '\0') ? last_user : "-",
             rows);

    return send_html_response(server_ptr, packet_ptr, html);
}

static UINT handle_add_user(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, const char *query)
{
    char uid[UID_MAX_LEN];
    char name[NAME_MAX_LEN];

    if (!query_get_value(query, "uid", uid, sizeof(uid)) ||
        !query_get_value(query, "name", name, sizeof(name)))
    {
        return render_home_page(server_ptr, packet_ptr, "<div class='card warn'>Preencha nome e UID para cadastrar.</div>");
    }

    if (storage_add_user(uid, name))
    {
        app_post_event(EVENT_USER_ADDED, uid);
        return render_home_page(server_ptr, packet_ptr, "<div class='card ok'>Usuario salvo em runtime. Clique em \"Persistir cadastros\" para salvar na QSPI.</div>");
    }

    return render_home_page(server_ptr, packet_ptr, "<div class='card warn'>Nao foi possivel salvar o usuario.</div>");
}

static UINT handle_remove_user(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, const char *query)
{
    char uid[UID_MAX_LEN];

    if (!query_get_value(query, "uid", uid, sizeof(uid)))
    {
        return render_home_page(server_ptr, packet_ptr, "<div class='card warn'>UID invalido para remocao.</div>");
    }

    if (storage_remove_uid(uid))
    {
        app_post_event(EVENT_USER_REMOVED, uid);
        return render_home_page(server_ptr, packet_ptr, "<div class='card ok'>Usuario removido em runtime. Clique em \"Persistir cadastros\" para salvar na QSPI.</div>");
    }

    return render_home_page(server_ptr, packet_ptr, "<div class='card warn'>Usuario nao encontrado.</div>");
}

static UINT handle_save_users(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr)
{
    if (storage_persist_now())
    {
        return render_home_page(server_ptr, packet_ptr, "<div class='card ok'>Persistencia solicitada. Aguarde alguns segundos e recarregue a pagina.</div>");
    }

    return render_home_page(server_ptr, packet_ptr, "<div class='card warn'>Nao foi possivel persistir os cadastros agora.</div>");
}

UINT authentication_check(NX_HTTP_SERVER *server_ptr,
                          UINT request_type,
                          CHAR *resource,
                          CHAR **name,
                          CHAR **password,
                          CHAR **realm)
{
    SSP_PARAMETER_NOT_USED(server_ptr);
    SSP_PARAMETER_NOT_USED(request_type);
    SSP_PARAMETER_NOT_USED(resource);
    SSP_PARAMETER_NOT_USED(name);
    SSP_PARAMETER_NOT_USED(password);
    SSP_PARAMETER_NOT_USED(realm);

    return NX_SUCCESS;
}

UINT request_notify(NX_HTTP_SERVER *server_ptr, UINT request_type, CHAR *resource, NX_PACKET *packet_ptr)
{
    char path[RESOURCE_BUFFER_SIZE];
    char query[QUERY_BUFFER_SIZE];
    size_t resource_len;

    g_net_debug_request_count++;
    g_net_debug_last_request_type = request_type;
    resource_len = strlen(resource);
    if (resource_len >= sizeof(g_net_debug_last_resource))
    {
        resource_len = sizeof(g_net_debug_last_resource) - 1U;
    }
    memcpy((void *) g_net_debug_last_resource, resource, resource_len);
    g_net_debug_last_resource[resource_len] = '\0';

    split_resource_and_query(resource, path, sizeof(path), query, sizeof(query));

    if (NX_HTTP_SERVER_GET_REQUEST == request_type)
    {
        if (0 == strcmp(path, "/health"))
        {
            return send_plain_response(server_ptr, packet_ptr, "OK");
        }
        if (0 == strcmp(path, "/"))
        {
            return render_home_page(server_ptr, packet_ptr, NULL);
        }
        if (0 == strcmp(path, "/add_user"))
        {
            return handle_add_user(server_ptr, packet_ptr, query);
        }
        if (0 == strcmp(path, "/remove_user"))
        {
            return handle_remove_user(server_ptr, packet_ptr, query);
        }
        if (0 == strcmp(path, "/save_users"))
        {
            return handle_save_users(server_ptr, packet_ptr);
        }
        if (0 == strcmp(path, "/portaon"))
        {
            app_post_event(EVENT_DOOR_OPEN, "WEB");
            return send_redirect(server_ptr, "/");
        }
        if (0 == strcmp(path, "/lampadatoggle"))
        {
            app_state_lock();
            bool current_light = g_app_state.light_on;
            app_state_unlock();

            app_post_event(current_light ? EVENT_LIGHT_OFF : EVENT_LIGHT_ON, NULL);
            return send_redirect(server_ptr, "/");
        }
        if (0 == strcmp(path, "/get_last_uid"))
        {
            char uid_text[UID_MAX_LEN] = "";

            app_state_lock();
            strncpy(uid_text, (const char *) g_app_state.last_uid, sizeof(uid_text) - 1U);
            uid_text[sizeof(uid_text) - 1U] = '\0';
            app_state_unlock();

            return send_plain_response(server_ptr, packet_ptr, uid_text);
        }
    }

    return send_plain_response(server_ptr, packet_ptr, "Not Found");
}

void thread_net_entry(ULONG arg)
{
    SSP_PARAMETER_NOT_USED(arg);

    ULONG actual_status = 0U;
    UINT status;

    network_stack_init_once();
    storage_init();
    g_net_debug_state = 3U;

    if (NX_SUCCESS == nx_ip_status_check(&g_ip0, NX_IP_LINK_ENABLED, &actual_status, TX_WAIT_FOREVER))
    {
        g_net_debug_link_status = actual_status;
        g_net_debug_state = 4U;
        status = nx_ip_address_set(&g_ip0, DEVICE_IP_ADDR, DEVICE_NETMASK);
        g_net_debug_ip_status = status;
        if (NX_SUCCESS == status)
        {
            ULONG ip_address = 0U;
            ULONG network_mask = 0U;

            (void) nx_ip_gateway_address_set(&g_ip0, DEVICE_GATEWAY_ADDR);
            (void) nx_ip_address_get(&g_ip0, &ip_address, &network_mask);
            g_net_debug_ip_address = ip_address;
            g_net_debug_network_mask = network_mask;
            g_net_debug_state = 5U;

            app_state_lock();
            g_app_state.net_ready = true;
            app_state_unlock();

            g_net_debug_http_status = nx_http_server_start(&g_http_server0);
            g_net_debug_state = (NX_SUCCESS == g_net_debug_http_status) ? 6U : 0xEEU;
        }
    }

    while (1)
    {
        ULONG bytes_sent = 0U;
        ULONG bytes_received = 0U;
        ULONG invalid_packets = 0U;
        ULONG packets_dropped_rx = 0U;
        ULONG checksum_errors = 0U;
        ULONG packets_dropped_tx = 0U;
        ULONG fragments_sent = 0U;
        ULONG fragments_received = 0U;
        ULONG arp_dynamic_entries = 0U;
        ULONG arp_static_entries = 0U;
        ULONG arp_aged_entries = 0U;
        ULONG arp_invalid_messages = 0U;
        ULONG ip_packets_sent = 0U;
        ULONG ip_packets_received = 0U;

        (void) nx_ip_info_get(&g_ip0,
                              &ip_packets_sent,
                              &bytes_sent,
                              &ip_packets_received,
                              &bytes_received,
                              &invalid_packets,
                              &packets_dropped_rx,
                              &checksum_errors,
                              &packets_dropped_tx,
                              &fragments_sent,
                              &fragments_received);

        g_net_debug_ip_packets_sent = ip_packets_sent;
        g_net_debug_ip_packets_received = ip_packets_received;
        g_net_debug_ip_invalid_packets = invalid_packets;
        g_net_debug_ip_receive_packets_dropped = packets_dropped_rx;
        g_net_debug_ip_receive_checksum_errors = checksum_errors;

        (void) nx_arp_info_get(&g_ip0,
                               (ULONG *) &g_net_debug_arp_requests_sent,
                               (ULONG *) &g_net_debug_arp_requests_received,
                               (ULONG *) &g_net_debug_arp_responses_sent,
                               (ULONG *) &g_net_debug_arp_responses_received,
                               &arp_dynamic_entries,
                               &arp_static_entries,
                               &arp_aged_entries,
                               &arp_invalid_messages);

        g_net_debug_arp_invalid_messages = arp_invalid_messages;
        g_net_debug_driver_irq = (ULONG) nx_record1.irq;
        g_net_debug_driver_irq_enabled = (ULONG) __NVIC_GetEnableIRQ(nx_record1.irq);
        g_net_debug_driver_state = (ULONG) nx_record1.nx_state;
        g_net_debug_driver_link_established = (ULONG) nx_record1.link_established;
        g_net_debug_driver_rx_bd_index = nx_record1.driver_rx_bd_index;
        g_net_debug_driver_rx_bd0_status = (NULL != nx_record1.driver_rx_bd) ? nx_record1.driver_rx_bd[0].bd_status : 0U;
        g_net_debug_driver_tx_bd0_status = (NULL != nx_record1.driver_tx_bd) ? nx_record1.driver_tx_bd[0].bd_status : 0U;

        tx_thread_sleep(50);
    }
}

void thread_tunnel_entry(ULONG arg)
{
    SSP_PARAMETER_NOT_USED(arg);
    NX_TCP_SOCKET tunnel_socket;
    NX_PACKET *receive_packet;

    while (1)
    {
        app_state_lock();
        bool ready = g_app_state.net_ready;
        app_state_unlock();
        if (ready)
        {
            break;
        }
        tx_thread_sleep(100);
    }

    nx_tcp_socket_create(&g_ip0,
                         &tunnel_socket,
                         "Tunnel Socket",
                         NX_IP_NORMAL,
                         NX_FRAGMENT_OKAY,
                         NX_IP_TIME_TO_LIVE,
                         512,
                         NX_NULL,
                         NX_NULL);

    while (1)
    {
        nx_tcp_client_socket_bind(&tunnel_socket, NX_ANY_PORT, TX_WAIT_FOREVER);
        UINT status = nx_tcp_client_socket_connect(&tunnel_socket, RELAY_IP_ADDR, RELAY_PORT, 500);

        if (NX_SUCCESS == status)
        {
            char req[256];

            snprintf(req,
                     sizeof(req),
                     "GET /tunnel?device_id=%s HTTP/1.1\r\nHost: %s\r\nX-API-KEY: %s\r\nConnection: keep-alive\r\n\r\n",
                     DEVICE_ID,
                     RELAY_HOST,
                     API_KEY);

            NX_PACKET *send_packet;
            nx_packet_allocate(&g_packet_pool0, &send_packet, NX_TCP_PACKET, NX_WAIT_FOREVER);
            nx_packet_data_append(send_packet, (VOID *) req, strlen(req), &g_packet_pool0, NX_WAIT_FOREVER);
            nx_tcp_socket_send(&tunnel_socket, send_packet, 500);

            while (NX_SUCCESS == nx_tcp_socket_receive(&tunnel_socket, &receive_packet, 500))
            {
                ULONG bytes_read;
                char buffer[128];
                ULONG copy_len;

                nx_packet_length_get(receive_packet, &bytes_read);
                copy_len = (bytes_read < 127U) ? bytes_read : 127U;
                nx_packet_data_extract_offset(receive_packet, 0, buffer, copy_len, &bytes_read);
                buffer[copy_len] = '\0';

                if (NULL != strstr(buffer, "\"portaon\""))
                {
                    app_post_event(EVENT_DOOR_OPEN, "TUNNEL");
                }
                else if (NULL != strstr(buffer, "\"lampadaon\""))
                {
                    app_post_event(EVENT_LIGHT_ON, NULL);
                }
                else if (NULL != strstr(buffer, "\"lampadaoff\""))
                {
                    app_post_event(EVENT_LIGHT_OFF, NULL);
                }

                nx_packet_release(receive_packet);
            }

            nx_tcp_socket_disconnect(&tunnel_socket, 200);
        }

        nx_tcp_client_socket_unbind(&tunnel_socket);
        tx_thread_sleep(200);
    }
}

void thread_status_entry(ULONG arg)
{
    SSP_PARAMETER_NOT_USED(arg);
    NX_TCP_SOCKET status_socket;

    while (1)
    {
        app_state_lock();
        bool ready = g_app_state.net_ready;
        app_state_unlock();
        if (ready)
        {
            break;
        }
        tx_thread_sleep(100);
    }

    nx_tcp_socket_create(&g_ip0,
                         &status_socket,
                         "Status Socket",
                         NX_IP_NORMAL,
                         NX_FRAGMENT_OKAY,
                         NX_IP_TIME_TO_LIVE,
                         512,
                         NX_NULL,
                         NX_NULL);

    while (1)
    {
        nx_tcp_client_socket_bind(&status_socket, NX_ANY_PORT, TX_WAIT_FOREVER);
        if (NX_SUCCESS == nx_tcp_client_socket_connect(&status_socket, RELAY_IP_ADDR, RELAY_PORT, 200))
        {
            bool p_stat;
            bool l_stat;
            char payload[128];
            char req[256];
            NX_PACKET *send_packet;

            app_state_lock();
            p_stat = g_app_state.door_open;
            l_stat = g_app_state.light_on;
            app_state_unlock();

            snprintf(payload,
                     sizeof(payload),
                     "{\"device_id\":\"%s\",\"door\":%d,\"light\":%d}",
                     DEVICE_ID,
                     p_stat ? 1 : 0,
                     l_stat ? 1 : 0);

            snprintf(req,
                     sizeof(req),
                     "POST /status HTTP/1.1\r\nHost: %s\r\nX-API-KEY: %s\r\nContent-Type: application/json\r\nContent-Length: %d\r\n\r\n%s",
                     RELAY_HOST,
                     API_KEY,
                     (int) strlen(payload),
                     payload);

            if (NX_SUCCESS == nx_packet_allocate(&g_packet_pool0, &send_packet, NX_TCP_PACKET, NX_WAIT_FOREVER))
            {
                nx_packet_data_append(send_packet, (VOID *) req, strlen(req), &g_packet_pool0, NX_WAIT_FOREVER);
                nx_tcp_socket_send(&status_socket, send_packet, 200);
            }

            nx_tcp_socket_disconnect(&status_socket, 200);
        }

        nx_tcp_client_socket_unbind(&status_socket);
        tx_thread_sleep(1000);
    }
}
