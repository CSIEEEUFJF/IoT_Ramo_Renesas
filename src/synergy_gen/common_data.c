/* generated common source file - do not edit */
#include "common_data.h"
#if (BSP_IRQ_DISABLED) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_g_transfer3) && !defined(SSP_SUPPRESS_ISR_DTCELC_EVENT_IIC0_RXI)
#define DTC_ACTIVATION_SRC_ELC_EVENT_IIC0_RXI
#if defined(DTC_ACTIVATION_SRC_ELC_EVENT_ELC_SOFTWARE_EVENT_0) && !defined(DTC_VECTOR_DEFINED_SOFTWARE_EVENT_0)
SSP_VECTOR_DEFINE(elc_software_event_isr, ELC, SOFTWARE_EVENT_0);
#define DTC_VECTOR_DEFINED_SOFTWARE_EVENT_0
#endif
#if defined(DTC_ACTIVATION_SRC_ELC_EVENT_ELC_SOFTWARE_EVENT_1) && !defined(DTC_VECTOR_DEFINED_SOFTWARE_EVENT_1)
SSP_VECTOR_DEFINE(elc_software_event_isr, ELC, SOFTWARE_EVENT_1);
#define DTC_VECTOR_DEFINED_SOFTWARE_EVENT_1
#endif
#endif
#endif

dtc_instance_ctrl_t g_transfer3_ctrl;
transfer_info_t g_transfer3_info =
{ .dest_addr_mode = TRANSFER_ADDR_MODE_INCREMENTED,
  .repeat_area = TRANSFER_REPEAT_AREA_DESTINATION,
  .irq = TRANSFER_IRQ_END,
  .chain_mode = TRANSFER_CHAIN_MODE_DISABLED,
  .src_addr_mode = TRANSFER_ADDR_MODE_FIXED,
  .size = TRANSFER_SIZE_1_BYTE,
  .mode = TRANSFER_MODE_NORMAL,
  .p_dest = (void*) NULL,
  .p_src = (void const*) NULL,
  .num_blocks = 0,
  .length = 0, };
const transfer_cfg_t g_transfer3_cfg =
{ .p_info = &g_transfer3_info,
  .activation_source = ELC_EVENT_IIC0_RXI,
  .auto_enable = false,
  .p_callback = NULL,
  .p_context = &g_transfer3,
  .irq_ipl = (BSP_IRQ_DISABLED) };
/* Instance structure to use this module. */
const transfer_instance_t g_transfer3 =
{ .p_ctrl = &g_transfer3_ctrl, .p_cfg = &g_transfer3_cfg, .p_api = &g_transfer_on_dtc };
#if (BSP_IRQ_DISABLED) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_g_transfer2) && !defined(SSP_SUPPRESS_ISR_DTCELC_EVENT_IIC0_TXI)
#define DTC_ACTIVATION_SRC_ELC_EVENT_IIC0_TXI
#if defined(DTC_ACTIVATION_SRC_ELC_EVENT_ELC_SOFTWARE_EVENT_0) && !defined(DTC_VECTOR_DEFINED_SOFTWARE_EVENT_0)
SSP_VECTOR_DEFINE(elc_software_event_isr, ELC, SOFTWARE_EVENT_0);
#define DTC_VECTOR_DEFINED_SOFTWARE_EVENT_0
#endif
#if defined(DTC_ACTIVATION_SRC_ELC_EVENT_ELC_SOFTWARE_EVENT_1) && !defined(DTC_VECTOR_DEFINED_SOFTWARE_EVENT_1)
SSP_VECTOR_DEFINE(elc_software_event_isr, ELC, SOFTWARE_EVENT_1);
#define DTC_VECTOR_DEFINED_SOFTWARE_EVENT_1
#endif
#endif
#endif

dtc_instance_ctrl_t g_transfer2_ctrl;
transfer_info_t g_transfer2_info =
{ .dest_addr_mode = TRANSFER_ADDR_MODE_FIXED,
  .repeat_area = TRANSFER_REPEAT_AREA_SOURCE,
  .irq = TRANSFER_IRQ_END,
  .chain_mode = TRANSFER_CHAIN_MODE_DISABLED,
  .src_addr_mode = TRANSFER_ADDR_MODE_INCREMENTED,
  .size = TRANSFER_SIZE_1_BYTE,
  .mode = TRANSFER_MODE_NORMAL,
  .p_dest = (void*) NULL,
  .p_src = (void const*) NULL,
  .num_blocks = 0,
  .length = 0, };
const transfer_cfg_t g_transfer2_cfg =
{ .p_info = &g_transfer2_info,
  .activation_source = ELC_EVENT_IIC0_TXI,
  .auto_enable = false,
  .p_callback = NULL,
  .p_context = &g_transfer2,
  .irq_ipl = (BSP_IRQ_DISABLED) };
/* Instance structure to use this module. */
const transfer_instance_t g_transfer2 =
{ .p_ctrl = &g_transfer2_ctrl, .p_cfg = &g_transfer2_cfg, .p_api = &g_transfer_on_dtc };
#if !defined(SSP_SUPPRESS_ISR_g_i2c0) && !defined(SSP_SUPPRESS_ISR_IIC0)
SSP_VECTOR_DEFINE_CHAN(iic_rxi_isr, IIC, RXI, 0);
#endif
#if !defined(SSP_SUPPRESS_ISR_g_i2c0) && !defined(SSP_SUPPRESS_ISR_IIC0)
SSP_VECTOR_DEFINE_CHAN(iic_txi_isr, IIC, TXI, 0);
#endif
#if !defined(SSP_SUPPRESS_ISR_g_i2c0) && !defined(SSP_SUPPRESS_ISR_IIC0)
SSP_VECTOR_DEFINE_CHAN(iic_tei_isr, IIC, TEI, 0);
#endif
#if !defined(SSP_SUPPRESS_ISR_g_i2c0) && !defined(SSP_SUPPRESS_ISR_IIC0)
SSP_VECTOR_DEFINE_CHAN(iic_eri_isr, IIC, ERI, 0);
#endif
riic_instance_ctrl_t g_i2c0_ctrl;
const riic_extended_cfg g_i2c0_extend =
{ .timeout_mode = RIIC_TIMEOUT_MODE_SHORT, };
const i2c_cfg_t g_i2c0_cfg =
{ .channel = 0, .rate = I2C_RATE_STANDARD, .slave = 0, .addr_mode = I2C_ADDR_MODE_7BIT, .sda_delay = 300,
#define SYNERGY_NOT_DEFINED (1)            
#if (SYNERGY_NOT_DEFINED == g_transfer2)
                .p_transfer_tx       = NULL,
#else
  .p_transfer_tx = &g_transfer2,
#endif
#if (SYNERGY_NOT_DEFINED == g_transfer3)
                .p_transfer_rx       = NULL,
#else
  .p_transfer_rx = &g_transfer3,
#endif
#undef SYNERGY_NOT_DEFINED	
  .p_callback = NULL,
  .p_context = (void*) &g_i2c0, .rxi_ipl = (12), .txi_ipl = (12), .tei_ipl = (12), .eri_ipl = (12), .p_extend =
          &g_i2c0_extend, };
/* Instance structure to use this module. */
const i2c_master_instance_t g_i2c0 =
{ .p_ctrl = &g_i2c0_ctrl, .p_cfg = &g_i2c0_cfg, .p_api = &g_i2c_master_on_riic };
static TX_MUTEX sf_bus_mutex_g_sf_i2c_bus0;
static TX_EVENT_FLAGS_GROUP sf_bus_eventflag_g_sf_i2c_bus0;
static sf_i2c_instance_ctrl_t *sf_curr_ctrl_g_sf_i2c_bus0;
static sf_i2c_instance_ctrl_t *sf_curr_bus_ctrl_g_sf_i2c_bus0;
sf_i2c_bus_t g_sf_i2c_bus0 =
{ .p_bus_name = (uint8_t*) "g_sf_i2c_bus0",
  .channel = 0,
  .p_lock_mutex = &sf_bus_mutex_g_sf_i2c_bus0,
  .p_sync_eventflag = &sf_bus_eventflag_g_sf_i2c_bus0,
  .pp_curr_ctrl = (sf_i2c_ctrl_t**) &sf_curr_ctrl_g_sf_i2c_bus0,
  .p_lower_lvl_api = (i2c_api_master_t*) &g_i2c_master_on_riic,
  .device_count = 0,
  .pp_curr_bus_ctrl = (sf_i2c_ctrl_t**) &sf_curr_bus_ctrl_g_sf_i2c_bus0, };
#if SYNERGY_NOT_DEFINED != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_DRW)
SSP_VECTOR_DEFINE(drw_int_isr, DRW, INT);
#endif
#endif
#if (BSP_IRQ_DISABLED) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_g_transfer1) && !defined(SSP_SUPPRESS_ISR_DTCELC_EVENT_SPI0_RXI)
#define DTC_ACTIVATION_SRC_ELC_EVENT_SPI0_RXI
#if defined(DTC_ACTIVATION_SRC_ELC_EVENT_ELC_SOFTWARE_EVENT_0) && !defined(DTC_VECTOR_DEFINED_SOFTWARE_EVENT_0)
SSP_VECTOR_DEFINE(elc_software_event_isr, ELC, SOFTWARE_EVENT_0);
#define DTC_VECTOR_DEFINED_SOFTWARE_EVENT_0
#endif
#if defined(DTC_ACTIVATION_SRC_ELC_EVENT_ELC_SOFTWARE_EVENT_1) && !defined(DTC_VECTOR_DEFINED_SOFTWARE_EVENT_1)
SSP_VECTOR_DEFINE(elc_software_event_isr, ELC, SOFTWARE_EVENT_1);
#define DTC_VECTOR_DEFINED_SOFTWARE_EVENT_1
#endif
#endif
#endif

dtc_instance_ctrl_t g_transfer1_ctrl;
transfer_info_t g_transfer1_info =
{ .dest_addr_mode = TRANSFER_ADDR_MODE_INCREMENTED,
  .repeat_area = TRANSFER_REPEAT_AREA_DESTINATION,
  .irq = TRANSFER_IRQ_END,
  .chain_mode = TRANSFER_CHAIN_MODE_DISABLED,
  .src_addr_mode = TRANSFER_ADDR_MODE_FIXED,
  .size = TRANSFER_SIZE_2_BYTE,
  .mode = TRANSFER_MODE_NORMAL,
  .p_dest = (void*) NULL,
  .p_src = (void const*) NULL,
  .num_blocks = 0,
  .length = 0, };
const transfer_cfg_t g_transfer1_cfg =
{ .p_info = &g_transfer1_info,
  .activation_source = ELC_EVENT_SPI0_RXI,
  .auto_enable = false,
  .p_callback = NULL,
  .p_context = &g_transfer1,
  .irq_ipl = (BSP_IRQ_DISABLED) };
/* Instance structure to use this module. */
const transfer_instance_t g_transfer1 =
{ .p_ctrl = &g_transfer1_ctrl, .p_cfg = &g_transfer1_cfg, .p_api = &g_transfer_on_dtc };
#if (BSP_IRQ_DISABLED) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_g_transfer0) && !defined(SSP_SUPPRESS_ISR_DTCELC_EVENT_SPI0_TXI)
#define DTC_ACTIVATION_SRC_ELC_EVENT_SPI0_TXI
#if defined(DTC_ACTIVATION_SRC_ELC_EVENT_ELC_SOFTWARE_EVENT_0) && !defined(DTC_VECTOR_DEFINED_SOFTWARE_EVENT_0)
SSP_VECTOR_DEFINE(elc_software_event_isr, ELC, SOFTWARE_EVENT_0);
#define DTC_VECTOR_DEFINED_SOFTWARE_EVENT_0
#endif
#if defined(DTC_ACTIVATION_SRC_ELC_EVENT_ELC_SOFTWARE_EVENT_1) && !defined(DTC_VECTOR_DEFINED_SOFTWARE_EVENT_1)
SSP_VECTOR_DEFINE(elc_software_event_isr, ELC, SOFTWARE_EVENT_1);
#define DTC_VECTOR_DEFINED_SOFTWARE_EVENT_1
#endif
#endif
#endif

dtc_instance_ctrl_t g_transfer0_ctrl;
transfer_info_t g_transfer0_info =
{ .dest_addr_mode = TRANSFER_ADDR_MODE_FIXED,
  .repeat_area = TRANSFER_REPEAT_AREA_SOURCE,
  .irq = TRANSFER_IRQ_END,
  .chain_mode = TRANSFER_CHAIN_MODE_DISABLED,
  .src_addr_mode = TRANSFER_ADDR_MODE_INCREMENTED,
  .size = TRANSFER_SIZE_2_BYTE,
  .mode = TRANSFER_MODE_NORMAL,
  .p_dest = (void*) NULL,
  .p_src = (void const*) NULL,
  .num_blocks = 0,
  .length = 0, };
const transfer_cfg_t g_transfer0_cfg =
{ .p_info = &g_transfer0_info,
  .activation_source = ELC_EVENT_SPI0_TXI,
  .auto_enable = false,
  .p_callback = NULL,
  .p_context = &g_transfer0,
  .irq_ipl = (BSP_IRQ_DISABLED) };
/* Instance structure to use this module. */
const transfer_instance_t g_transfer0 =
{ .p_ctrl = &g_transfer0_ctrl, .p_cfg = &g_transfer0_cfg, .p_api = &g_transfer_on_dtc };
#define RSPI_TRANSFER_SIZE_1_BYTE (0x52535049)
#define RSPI_SYNERGY_NOT_DEFINED 1
#if (RSPI_SYNERGY_NOT_DEFINED != RSPI_TRANSFER_SIZE_2_BYTE)
dtc_instance_ctrl_t g_spi0_transfer_tx_ctrl;
uint32_t g_spi0_tx_inter = 0;
transfer_info_t g_spi0_transfer_tx_info[2] =
{
#if (RSPI_TRANSFER_SIZE_1_BYTE == RSPI_TRANSFER_SIZE_2_BYTE)
    {
    .dest_addr_mode = TRANSFER_ADDR_MODE_FIXED, 
    .repeat_area = TRANSFER_REPEAT_AREA_SOURCE, 
    .irq = TRANSFER_IRQ_END,
    .chain_mode = TRANSFER_CHAIN_MODE_EACH, 
    .src_addr_mode = TRANSFER_ADDR_MODE_INCREMENTED, 
    .size = TRANSFER_SIZE_1_BYTE,
    .mode = TRANSFER_MODE_NORMAL, 
    .p_dest = (void *) &g_spi0_tx_inter, 
    .p_src = (void const *) NULL, 
    .num_blocks = 0, 
    .length = 0,
    }, 
    {
    .dest_addr_mode = TRANSFER_ADDR_MODE_FIXED, 
    .repeat_area = TRANSFER_REPEAT_AREA_SOURCE, 
    .irq = TRANSFER_IRQ_END,
    .chain_mode = TRANSFER_CHAIN_MODE_DISABLED, 
    .src_addr_mode = TRANSFER_ADDR_MODE_FIXED, 
    .size = TRANSFER_SIZE_4_BYTE,
    .mode = TRANSFER_MODE_NORMAL, 
    .p_dest = (void *) NULL, 
    .p_src = (void const *) &g_spi0_tx_inter, 
    .num_blocks = 0, 
    .length = 0,
    }, 
#else
  { .dest_addr_mode = TRANSFER_ADDR_MODE_FIXED,
    .repeat_area = TRANSFER_REPEAT_AREA_SOURCE,
    .irq = TRANSFER_IRQ_END,
    .chain_mode = TRANSFER_CHAIN_MODE_DISABLED,
    .src_addr_mode = TRANSFER_ADDR_MODE_INCREMENTED,
    .size = TRANSFER_SIZE_2_BYTE,
    .mode = TRANSFER_MODE_NORMAL,
    .p_dest = (void*) NULL,
    .p_src = (void const*) NULL,
    .num_blocks = 0,
    .length = 0, },
  { .dest_addr_mode = TRANSFER_ADDR_MODE_FIXED,
    .repeat_area = TRANSFER_REPEAT_AREA_SOURCE,
    .irq = TRANSFER_IRQ_END,
    .chain_mode = TRANSFER_CHAIN_MODE_DISABLED,
    .src_addr_mode = TRANSFER_ADDR_MODE_FIXED,
    .size = TRANSFER_SIZE_4_BYTE,
    .mode = TRANSFER_MODE_NORMAL,
    .p_dest = (void*) NULL,
    .p_src = (void const*) &g_spi0_tx_inter,
    .num_blocks = 0,
    .length = 0, },
#endif
        };

const transfer_cfg_t g_spi0_transfer_tx_cfg =
{ .p_info = g_spi0_transfer_tx_info,
  .activation_source = ELC_EVENT_SPI0_TXI,
  .auto_enable = false,
  .p_callback = NULL,
  .p_context = &g_spi0_transfer_tx,
  .irq_ipl = (BSP_IRQ_DISABLED) };
/* Instance structure to use this module. */
const transfer_instance_t g_spi0_transfer_tx =
{ .p_ctrl = &g_spi0_transfer_tx_ctrl, .p_cfg = &g_spi0_transfer_tx_cfg, .p_api = &g_transfer_on_dtc };

dtc_instance_ctrl_t g_spi0_transfer_rx_ctrl;
uint32_t g_spi0_rx_inter = 0;
transfer_info_t g_spi0_transfer_rx_info[2] =
{
#if (RSPI_TRANSFER_SIZE_1_BYTE == RSPI_TRANSFER_SIZE_2_BYTE)
    {
    .dest_addr_mode = TRANSFER_ADDR_MODE_FIXED, 
    .repeat_area = TRANSFER_REPEAT_AREA_SOURCE, 
    .irq = TRANSFER_IRQ_END,
    .chain_mode = TRANSFER_CHAIN_MODE_EACH, 
    .src_addr_mode = TRANSFER_ADDR_MODE_FIXED, 
    .size = TRANSFER_SIZE_4_BYTE,
    .mode = TRANSFER_MODE_NORMAL, 
    .p_dest = (void *) &g_spi0_rx_inter, 
    .p_src = (void const *) NULL, 
    .num_blocks = 0, 
    .length = 0,
    }, 
    {
    .dest_addr_mode = TRANSFER_ADDR_MODE_INCREMENTED, 
    .repeat_area = TRANSFER_REPEAT_AREA_SOURCE, 
    .irq = TRANSFER_IRQ_END,
    .chain_mode = TRANSFER_CHAIN_MODE_DISABLED, 
    .src_addr_mode = TRANSFER_ADDR_MODE_FIXED, 
    .size = TRANSFER_SIZE_1_BYTE,
    .mode = TRANSFER_MODE_NORMAL, 
    .p_dest = (void *) NULL, 
    .p_src = (void const *) &g_spi0_rx_inter, 
    .num_blocks = 0, 
    .length = 0,
    }, 
#else
  { .dest_addr_mode = TRANSFER_ADDR_MODE_INCREMENTED,
    .repeat_area = TRANSFER_REPEAT_AREA_DESTINATION,
    .irq = TRANSFER_IRQ_END,
    .chain_mode = TRANSFER_CHAIN_MODE_DISABLED,
    .src_addr_mode = TRANSFER_ADDR_MODE_FIXED,
    .size = TRANSFER_SIZE_2_BYTE,
    .mode = TRANSFER_MODE_NORMAL,
    .p_dest = (void*) NULL,
    .p_src = (void const*) NULL,
    .num_blocks = 0, },
  { .dest_addr_mode = TRANSFER_ADDR_MODE_INCREMENTED,
    .repeat_area = TRANSFER_REPEAT_AREA_SOURCE,
    .irq = TRANSFER_IRQ_END,
    .chain_mode = TRANSFER_CHAIN_MODE_DISABLED,
    .src_addr_mode = TRANSFER_ADDR_MODE_FIXED,
    .size = TRANSFER_SIZE_1_BYTE,
    .mode = TRANSFER_MODE_NORMAL,
    .p_dest = (void*) NULL,
    .p_src = (void const*) &g_spi0_rx_inter,
    .num_blocks = 0,
    .length = 0, },
#endif
        };

const transfer_cfg_t g_spi0_transfer_rx_cfg =
{ .p_info = g_spi0_transfer_rx_info,
  .activation_source = ELC_EVENT_SPI0_RXI,
  .auto_enable = false,
  .p_callback = NULL,
  .p_context = &g_spi0_transfer_rx,
  .irq_ipl = (BSP_IRQ_DISABLED) };
/* Instance structure to use this module. */
const transfer_instance_t g_spi0_transfer_rx =
{ .p_ctrl = &g_spi0_transfer_rx_ctrl, .p_cfg = &g_spi0_transfer_rx_cfg, .p_api = &g_transfer_on_dtc };
#endif
#undef RSPI_TRANSFER_SIZE_1_BYTE	
#undef RSPI_SYNERGY_NOT_DEFINED

#if !defined(SSP_SUPPRESS_ISR_g_spi0) && !defined(SSP_SUPPRESS_ISR_SPI0)
SSP_VECTOR_DEFINE_CHAN(spi_rxi_isr, SPI, RXI, 0);
#endif
#if !defined(SSP_SUPPRESS_ISR_g_spi0) && !defined(SSP_SUPPRESS_ISR_SPI0)
SSP_VECTOR_DEFINE_CHAN(spi_txi_isr, SPI, TXI, 0);
#endif
#if !defined(SSP_SUPPRESS_ISR_g_spi0) && !defined(SSP_SUPPRESS_ISR_SPI0)
SSP_VECTOR_DEFINE_CHAN(spi_eri_isr, SPI, ERI, 0);
#endif
#if !defined(SSP_SUPPRESS_ISR_g_spi0) && !defined(SSP_SUPPRESS_ISR_SPI0)
SSP_VECTOR_DEFINE_CHAN(spi_tei_isr, SPI, TEI, 0);
#endif
rspi_instance_ctrl_t g_spi0_ctrl;

/** RSPI extended configuration for RSPI HAL driver */
const spi_on_rspi_cfg_t g_spi0_ext_cfg =
{ .rspi_clksyn = RSPI_OPERATION_SPI,
  /* Communication mode is configured by the driver. write calls use TX_ONLY. read and writeRead use FULL_DUPLEX. */
  .rspi_comm = RSPI_COMMUNICATION_FULL_DUPLEX,
  .ssl_polarity.rspi_ssl0 = RSPI_SSLP_LOW,
  .loopback.rspi_loopback1 = RSPI_LOOPBACK1_NORMAL_DATA,
  .loopback.rspi_loopback2 = RSPI_LOOPBACK2_NORMAL_DATA,
  .mosi_idle.rspi_mosi_idle_fixed_val = RSPI_MOSI_IDLE_FIXED_VAL_LOW,
  .mosi_idle.rspi_mosi_idle_val_fixing = RSPI_MOSI_IDLE_VAL_FIXING_DISABLE,
  .parity.rspi_parity = RSPI_PARITY_STATE_DISABLE,
  .parity.rspi_parity_mode = RSPI_PARITY_MODE_ODD,
  .ssl_select = RSPI_SSL_SELECT_SSL0,
  .ssl_level_keep = RSPI_SSL_LEVEL_KEEP_NOT,
  .clock_delay.rspi_clock_delay_count = RSPI_CLOCK_DELAY_COUNT_1,
  .clock_delay.rspi_clock_delay_state = RSPI_CLOCK_DELAY_STATE_DISABLE,
  .ssl_neg_delay.rspi_ssl_neg_delay_count = RSPI_SSL_NEGATION_DELAY_1,
  .ssl_neg_delay.rspi_ssl_neg_delay_state = RSPI_SSL_NEGATION_DELAY_DISABLE,
  .access_delay.rspi_next_access_delay_count = RSPI_NEXT_ACCESS_DELAY_COUNT_1,
  .access_delay.rspi_next_access_delay_state = RSPI_NEXT_ACCESS_DELAY_STATE_DISABLE,
  .byte_swap = RSPI_BYTE_SWAP_DISABLE, };

const spi_cfg_t g_spi0_cfg =
{ .channel = 0, .operating_mode = SPI_MODE_MASTER, .clk_phase = SPI_CLK_PHASE_EDGE_ODD, .clk_polarity =
          SPI_CLK_POLARITY_LOW,
  .mode_fault = SPI_MODE_FAULT_ERROR_DISABLE, .bit_order = SPI_BIT_ORDER_MSB_FIRST, .bitrate = 500000, .p_transfer_tx =
          g_spi0_P_TRANSFER_TX,
  .p_transfer_rx = g_spi0_P_TRANSFER_RX, .p_callback = NULL, .p_context = (void*) &g_spi0, .p_extend =
          (void*) &g_spi0_ext_cfg,
  .rxi_ipl = (12), .txi_ipl = (12), .eri_ipl = (12), .tei_ipl = (12), };
/* Instance structure to use this module. */
const spi_instance_t g_spi0 =
{ .p_ctrl = &g_spi0_ctrl, .p_cfg = &g_spi0_cfg, .p_api = &g_spi_on_rspi };
static TX_MUTEX sf_bus_mutex_g_sf_spi_bus0;
static TX_EVENT_FLAGS_GROUP sf_bus_eventflag_g_sf_spi_bus0;
static sf_spi_ctrl_t *p_sf_curr_ctrl_g_sf_spi_bus0;

sf_spi_bus_t g_sf_spi_bus0 =
{ .p_bus_name = (uint8_t*) "g_sf_spi_bus0",
  .channel = 0,
  .freq_hz_min = 0,
  .p_lock_mutex = &sf_bus_mutex_g_sf_spi_bus0,
  .p_sync_eventflag = &sf_bus_eventflag_g_sf_spi_bus0,
  .pp_curr_ctrl = (sf_spi_ctrl_t**) &p_sf_curr_ctrl_g_sf_spi_bus0,
  .p_lower_lvl_api = (spi_api_t*) &g_spi_on_rspi,
  .device_count = 0, };
/* Instance structure to use this module. */
const fmi_instance_t g_fmi =
{ .p_api = &g_fmi_on_fmi };
const ioport_instance_t g_ioport =
{ .p_api = &g_ioport_on_ioport, .p_cfg = NULL };
const elc_instance_t g_elc =
{ .p_api = &g_elc_on_elc, .p_cfg = NULL };
const cgc_instance_t g_cgc =
{ .p_api = &g_cgc_on_cgc, .p_cfg = NULL };
void g_common_init(void)
{
}
