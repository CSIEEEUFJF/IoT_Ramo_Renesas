/* generated thread header file - do not edit */
#ifndef MAIN_THREAD_H_
#define MAIN_THREAD_H_
#include "bsp_api.h"
#include "tx_api.h"
#include "hal_data.h"
#ifdef __cplusplus
                extern "C" void main_thread_entry(void);
                #else
extern void main_thread_entry(void);
#endif
#include "r_dtc.h"
#include "r_transfer_api.h"
#include "r_sci_spi.h"
#include "r_spi_api.h"
#include "nxd_http_server.h"
#include "r_gpt.h"
#include "r_timer_api.h"
#include "r_rspi.h"
#include "r_spi_api.h"
#ifdef __cplusplus
extern "C" {
#endif
/* Transfer on DTC Instance. */
extern const transfer_instance_t g_transfer3;
#ifndef NULL
void NULL(transfer_callback_args_t *p_args);
#endif
/* Transfer on DTC Instance. */
extern const transfer_instance_t g_transfer2;
#ifndef NULL
void NULL(transfer_callback_args_t *p_args);
#endif
extern const spi_cfg_t g_spi_lcdc_cfg;
/** SPI on SCI Instance. */
extern const spi_instance_t g_spi_lcdc;
extern sci_spi_instance_ctrl_t g_spi_lcdc_ctrl;
extern const sci_spi_extended_cfg g_spi_lcdc_cfg_extend;

#ifndef g_lcd_spi_callback
void g_lcd_spi_callback(spi_callback_args_t *p_args);
#endif

#define SYNERGY_NOT_DEFINED (1)            
#if (SYNERGY_NOT_DEFINED == g_transfer2)
    #define g_spi_lcdc_P_TRANSFER_TX (NULL)
#else
#define g_spi_lcdc_P_TRANSFER_TX (&g_transfer2)
#endif
#if (SYNERGY_NOT_DEFINED == g_transfer3)
    #define g_spi_lcdc_P_TRANSFER_RX (NULL)
#else
#define g_spi_lcdc_P_TRANSFER_RX (&g_transfer3)
#endif
#undef SYNERGY_NOT_DEFINED

#define g_spi_lcdc_P_EXTEND (&g_spi_lcdc_cfg_extend)
extern NX_HTTP_SERVER g_http_server0;
#if !defined(authentication_check)
UINT authentication_check(NX_HTTP_SERVER *server_ptr, UINT request_type,
		CHAR *resource, CHAR **name, CHAR **password, CHAR **realm);
#endif
#if !defined(request_notify)
UINT request_notify(NX_HTTP_SERVER *server_ptr, UINT request_type,
		CHAR *resource, NX_PACKET *packet_ptr);
#endif
void g_http_server0_err_callback(void *p_instance, void *p_data);
void http_server_init0(void);
/** Timer on GPT Instance. */
extern const timer_instance_t g_timer0;
#ifndef NULL
void NULL(timer_callback_args_t *p_args);
#endif
/* Transfer on DTC Instance. */
extern const transfer_instance_t g_transfer1;
#ifndef NULL
void NULL(transfer_callback_args_t *p_args);
#endif
/* Transfer on DTC Instance. */
extern const transfer_instance_t g_transfer0;
#ifndef NULL
void NULL(transfer_callback_args_t *p_args);
#endif
/** SPI config */
extern const spi_cfg_t g_spi0_cfg;
/** RSPI extended config */
extern const spi_on_rspi_cfg_t g_spi0_ext_cfg;
/** SPI on RSPI Instance. */
extern const spi_instance_t g_spi0;
/** SPI instance control */
extern rspi_instance_ctrl_t g_spi0_ctrl;
#ifndef rfid_spi_callback
void rfid_spi_callback(spi_callback_args_t *p_args);
#endif

extern const transfer_instance_t g_spi0_transfer_rx;
extern const transfer_instance_t g_spi0_transfer_tx;

#define SYNERGY_NOT_DEFINED (1)
#define RSPI_TRANSFER_SIZE_1_BYTE (0x52535049)

#if (SYNERGY_NOT_DEFINED == g_transfer0)
    #define g_spi0_P_TRANSFER_TX (NULL)
#else
#define g_spi0_P_TRANSFER_TX (&g_spi0_transfer_tx)
#endif
#if (SYNERGY_NOT_DEFINED == g_transfer1)
    #define g_spi0_P_TRANSFER_RX (NULL)
#else
#define g_spi0_P_TRANSFER_RX (&g_spi0_transfer_rx)
#endif

#undef RSPI_TRANSFER_SIZE_1_BYTE
#undef SYNERGY_NOT_DEFINED
#define g_spi0_P_EXTEND (&g_spi0_ext_cfg)
extern TX_SEMAPHORE g_main_semaphore_lcdc;
#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* MAIN_THREAD_H_ */
