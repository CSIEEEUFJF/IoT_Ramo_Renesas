#ifndef NET_H
#define NET_H
#include "bsp_api.h"
#include "nx_api.h"
#include "nxd_http_server.h"

/* Callbacks com a assinatura exata gerada em main_thread.h */
UINT request_notify(NX_HTTP_SERVER *server_ptr, UINT request_type, CHAR *resource, NX_PACKET *packet_ptr);
UINT authentication_check(NX_HTTP_SERVER *server_ptr, UINT request_type, CHAR *resource, CHAR **name, CHAR **password, CHAR **realm);

#endif /* NET_H */
