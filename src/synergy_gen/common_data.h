/* generated common header file - do not edit */
#ifndef COMMON_DATA_H_
#define COMMON_DATA_H_
#include <stdint.h>
#include "bsp_api.h"
#include "r_dtc.h"
#include "r_transfer_api.h"
#include "r_riic.h"
#include "r_i2c_api.h"
#include "r_i2c_api.h"
#include "sf_i2c.h"
#include "sf_i2c_api.h"
#include "r_rspi.h"
#include "r_spi_api.h"
#include "r_cgc_api.h"
#include "r_spi_api.h"
#include "sf_spi.h"
#include "sf_spi_api.h"
#include "r_fmi.h"
#include "r_fmi_api.h"
#include "r_ioport.h"
#include "r_ioport_api.h"
#include "r_elc.h"
#include "r_elc_api.h"
#include "r_cgc.h"
#include "r_cgc_api.h"
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
extern const i2c_cfg_t g_i2c0_cfg;
/** I2C on RIIC Instance. */
extern const i2c_master_instance_t g_i2c0;
#ifndef NULL
void NULL(i2c_callback_args_t *p_args);
#endif

extern riic_instance_ctrl_t g_i2c0_ctrl;
extern const riic_extended_cfg g_i2c0_extend;
#define SYNERGY_NOT_DEFINED (1)            
#if (SYNERGY_NOT_DEFINED == g_transfer2)
    #define g_i2c0_P_TRANSFER_TX (NULL)
#else
#define g_i2c0_P_TRANSFER_TX (&g_transfer2)
#endif
#if (SYNERGY_NOT_DEFINED == g_transfer3)
    #define g_i2c0_P_TRANSFER_RX (NULL)
#else
#define g_i2c0_P_TRANSFER_RX (&g_transfer3)
#endif
#undef SYNERGY_NOT_DEFINED
#define g_i2c0_P_EXTEND (&g_i2c0_extend)
extern sf_i2c_bus_t g_sf_i2c_bus0;
extern i2c_api_master_t const g_i2c_master_on_riic;

#define g_sf_i2c_bus0_CHANNEL        (0)
#define g_sf_i2c_bus0_RATE           (I2C_RATE_STANDARD)
#define g_sf_i2c_bus0_SLAVE          (0)
#define g_sf_i2c_bus0_ADDR_MODE      (I2C_ADDR_MODE_7BIT)          
#define g_sf_i2c_bus0_SDA_DELAY      (300)  
#define g_sf_i2c_bus0_P_CALLBACK     (NULL)
#define g_sf_i2c_bus0_P_CONTEXT      (&g_i2c0)
#define g_sf_i2c_bus0_RXI_IPL        ((12))
#define g_sf_i2c_bus0_TXI_IPL        ((12))
#define g_sf_i2c_bus0_TEI_IPL        ((12))            
#define g_sf_i2c_bus0_ERI_IPL        ((12))

/** These are obtained by macros in the I2C driver XMLs. */
#define g_sf_i2c_bus0_P_TRANSFER_TX  (g_i2c0_P_TRANSFER_TX)
#define g_sf_i2c_bus0_P_TRANSFER_RX  (g_i2c0_P_TRANSFER_RX)            
#define g_sf_i2c_bus0_P_EXTEND       (g_i2c0_P_EXTEND)
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
#ifndef NULL
void NULL(spi_callback_args_t *p_args);
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
extern sf_spi_bus_t g_sf_spi_bus0;
extern spi_api_t const g_spi_on_rspi;

#define g_sf_spi_bus0_CHANNEL        (0)
#define g_sf_spi_bus0_OPERATING_MODE (SPI_MODE_MASTER)
#define g_sf_spi_bus0_CLK_PHASE      (SPI_CLK_PHASE_EDGE_ODD)
#define g_sf_spi_bus0_CLK_POLARITY   (SPI_CLK_POLARITY_LOW)          
#define g_sf_spi_bus0_MODE_FAULT     (SPI_MODE_FAULT_ERROR_DISABLE)
#define g_sf_spi_bus0_BIT_ORDER      (SPI_BIT_ORDER_MSB_FIRST)          
#define g_sf_spi_bus0_BIT_RATE       (500000)  
#define g_sf_spi_bus0_P_CALLBACK     (NULL)
#define g_sf_spi_bus0_P_CONTEXT      (&g_spi0)
#define g_sf_spi_bus0_RXI_IPL        ((12))
#define g_sf_spi_bus0_TXI_IPL        ((12))
#define g_sf_spi_bus0_TEI_IPL        ((12))            
#define g_sf_spi_bus0_ERI_IPL        ((12))

/** These are obtained by macros in the SPI driver XMLs. */
#define g_sf_spi_bus0_P_TRANSFER_TX  (g_spi0_P_TRANSFER_TX)
#define g_sf_spi_bus0_P_TRANSFER_RX  (g_spi0_P_TRANSFER_RX)            
#define g_sf_spi_bus0_P_EXTEND       (g_spi0_P_EXTEND)
/** FMI on FMI Instance. */
extern const fmi_instance_t g_fmi;
/** IOPORT Instance */
extern const ioport_instance_t g_ioport;
/** ELC Instance */
extern const elc_instance_t g_elc;
/** CGC Instance */
extern const cgc_instance_t g_cgc;
void g_common_init(void);
#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* COMMON_DATA_H_ */
