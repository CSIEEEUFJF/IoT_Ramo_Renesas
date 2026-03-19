/* generated thread source file - do not edit */
#include "main_thread.h"

TX_THREAD main_thread;
void main_thread_create(void);
static void main_thread_func(ULONG thread_input);
static uint8_t main_thread_stack[4096] BSP_PLACE_IN_SECTION_V2(".stack.main_thread") BSP_ALIGN_VARIABLE_V2(BSP_STACK_ALIGNMENT);
void tx_startup_err_callback(void *p_instance, void *p_data);
void tx_startup_common_init(void);
#if (BSP_IRQ_DISABLED) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_g_transfer3) && !defined(SSP_SUPPRESS_ISR_DTCELC_EVENT_SCI8_RXI)
#define DTC_ACTIVATION_SRC_ELC_EVENT_SCI8_RXI
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
transfer_info_t g_transfer3_info = { .dest_addr_mode =
		TRANSFER_ADDR_MODE_INCREMENTED, .repeat_area =
		TRANSFER_REPEAT_AREA_DESTINATION, .irq = TRANSFER_IRQ_END, .chain_mode =
		TRANSFER_CHAIN_MODE_DISABLED, .src_addr_mode = TRANSFER_ADDR_MODE_FIXED,
		.size = TRANSFER_SIZE_1_BYTE, .mode = TRANSFER_MODE_NORMAL, .p_dest =
				(void*) NULL, .p_src = (void const*) NULL, .num_blocks = 0,
		.length = 0, };
const transfer_cfg_t g_transfer3_cfg = { .p_info = &g_transfer3_info,
		.activation_source = ELC_EVENT_SCI8_RXI, .auto_enable = false,
		.p_callback = NULL, .p_context = &g_transfer3, .irq_ipl =
				(BSP_IRQ_DISABLED) };
/* Instance structure to use this module. */
const transfer_instance_t g_transfer3 = { .p_ctrl = &g_transfer3_ctrl, .p_cfg =
		&g_transfer3_cfg, .p_api = &g_transfer_on_dtc };
#if (BSP_IRQ_DISABLED) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_g_transfer2) && !defined(SSP_SUPPRESS_ISR_DTCELC_EVENT_SCI8_TXI)
#define DTC_ACTIVATION_SRC_ELC_EVENT_SCI8_TXI
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
transfer_info_t g_transfer2_info = { .dest_addr_mode = TRANSFER_ADDR_MODE_FIXED,
		.repeat_area = TRANSFER_REPEAT_AREA_SOURCE, .irq = TRANSFER_IRQ_END,
		.chain_mode = TRANSFER_CHAIN_MODE_DISABLED, .src_addr_mode =
				TRANSFER_ADDR_MODE_INCREMENTED, .size = TRANSFER_SIZE_1_BYTE,
		.mode = TRANSFER_MODE_NORMAL, .p_dest = (void*) NULL, .p_src =
				(void const*) NULL, .num_blocks = 0, .length = 0, };
const transfer_cfg_t g_transfer2_cfg = { .p_info = &g_transfer2_info,
		.activation_source = ELC_EVENT_SCI8_TXI, .auto_enable = false,
		.p_callback = NULL, .p_context = &g_transfer2, .irq_ipl =
				(BSP_IRQ_DISABLED) };
/* Instance structure to use this module. */
const transfer_instance_t g_transfer2 = { .p_ctrl = &g_transfer2_ctrl, .p_cfg =
		&g_transfer2_cfg, .p_api = &g_transfer_on_dtc };
#if !defined(SSP_SUPPRESS_ISR_g_spi_lcdc) && !defined(SSP_SUPPRESS_ISR_SCI8)
SSP_VECTOR_DEFINE_CHAN(sci_spi_rxi_isr, SCI, RXI, 8);
#endif
#if !defined(SSP_SUPPRESS_ISR_g_spi_lcdc) && !defined(SSP_SUPPRESS_ISR_SCI8)
SSP_VECTOR_DEFINE_CHAN(sci_spi_txi_isr, SCI, TXI, 8);
#endif
#if !defined(SSP_SUPPRESS_ISR_g_spi_lcdc) && !defined(SSP_SUPPRESS_ISR_SCI8)
SSP_VECTOR_DEFINE_CHAN(sci_spi_tei_isr, SCI, TEI, 8);
#endif
#if !defined(SSP_SUPPRESS_ISR_g_spi_lcdc) && !defined(SSP_SUPPRESS_ISR_SCI8)
SSP_VECTOR_DEFINE_CHAN(sci_spi_eri_isr, SCI, ERI, 8);
#endif
sci_spi_instance_ctrl_t g_spi_lcdc_ctrl;

/** SPI extended configuration */
const sci_spi_extended_cfg g_spi_lcdc_cfg_extend =
		{ .bitrate_modulation = true };

const spi_cfg_t g_spi_lcdc_cfg = { .channel = 8, .operating_mode =
		SPI_MODE_MASTER, .clk_phase = SPI_CLK_PHASE_EDGE_EVEN, .clk_polarity =
		SPI_CLK_POLARITY_HIGH, .mode_fault = SPI_MODE_FAULT_ERROR_DISABLE,
		.bit_order = SPI_BIT_ORDER_MSB_FIRST, .bitrate = 100000,
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
		.p_callback = g_lcd_spi_callback, .p_context = (void*) &g_spi_lcdc,
		.rxi_ipl = (12), .txi_ipl = (12), .tei_ipl = (12), .eri_ipl = (12),
		.p_extend = &g_spi_lcdc_cfg_extend, };
/* Instance structure to use this module. */
const spi_instance_t g_spi_lcdc = { .p_ctrl = &g_spi_lcdc_ctrl, .p_cfg =
		&g_spi_lcdc_cfg, .p_api = &g_spi_on_sci };
NX_HTTP_SERVER g_http_server0;
uint8_t g_http_server0_stack_memory[4096] BSP_PLACE_IN_SECTION_V2(".stack.g_http_server0") BSP_ALIGN_VARIABLE_V2(BSP_STACK_ALIGNMENT);
#if defined(__ICCARM__)
#define g_http_server0_err_callback_WEAK_ATTRIBUTE
#pragma weak g_http_server0_err_callback  = g_http_server0_err_callback_internal
#elif defined(__GNUC__)
#define g_http_server0_err_callback_WEAK_ATTRIBUTE   __attribute__ ((weak, alias("g_http_server0_err_callback_internal")))
#endif
void g_http_server0_err_callback(void *p_instance, void *p_data)
g_http_server0_err_callback_WEAK_ATTRIBUTE;
/*******************************************************************************************************************//**
 * @brief      This is a weak example initialization error function.  It should be overridden by defining a user  function
 *             with the prototype below.
 *             - void g_http_server0_err_callback(void * p_instance, void * p_data)
 *
 * @param[in]  p_instance arguments used to identify which instance caused the error and p_data Callback arguments used to identify what error caused the callback.
 **********************************************************************************************************************/
void g_http_server0_err_callback_internal(void *p_instance, void *p_data);
void g_http_server0_err_callback_internal(void *p_instance, void *p_data) {
	/** Suppress compiler warning for not using parameters. */
	SSP_PARAMETER_NOT_USED(p_instance);
	SSP_PARAMETER_NOT_USED(p_data);

	/** An error has occurred. Please check function arguments for more information. */
	BSP_CFG_HANDLE_UNRECOVERABLE_ERROR(0);
}
/*******************************************************************************************************************//**
 * @brief     Initialization function that the user can choose to have called automatically during thread entry.
 *            The user can call this function at a later time if desired using the prototype below.
 *            - void http_server_init0(void)
 **********************************************************************************************************************/
void http_server_init0(void) {
	UINT g_http_server0_err;
	/* Create HTTP Server. */
	g_http_server0_err = nx_http_server_create(&g_http_server0,
			"g_http_server0 HTTP Server", &g_ip0, &g_fx_media0,
			&g_http_server0_stack_memory[0], 4096, &g_packet_pool0,
			authentication_check, request_notify);
	if (NX_SUCCESS != g_http_server0_err) {
		g_http_server0_err_callback((void*) &g_http_server0,
				&g_http_server0_err);
	}
}
#if (BSP_IRQ_DISABLED) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_g_timer0) && !defined(SSP_SUPPRESS_ISR_GPT0)
SSP_VECTOR_DEFINE_CHAN(gpt_counter_overflow_isr, GPT, COUNTER_OVERFLOW, 0);
#endif
#endif
static gpt_instance_ctrl_t g_timer0_ctrl;
static const timer_on_gpt_cfg_t g_timer0_extend = { .gtioca = {
		.output_enabled = false, .stop_level = GPT_PIN_LEVEL_LOW }, .gtiocb = {
		.output_enabled = false, .stop_level = GPT_PIN_LEVEL_LOW },
		.shortest_pwm_signal = GPT_SHORTEST_LEVEL_OFF, };
static const timer_cfg_t g_timer0_cfg = { .mode = TIMER_MODE_PERIODIC, .period =
		1, .unit = TIMER_UNIT_PERIOD_MSEC, .duty_cycle = 50, .duty_cycle_unit =
		TIMER_PWM_UNIT_RAW_COUNTS, .channel = 0, .autostart = true,
		.p_callback = NULL, .p_context = &g_timer0,
		.p_extend = &g_timer0_extend, .irq_ipl = (BSP_IRQ_DISABLED), };
/* Instance structure to use this module. */
const timer_instance_t g_timer0 = { .p_ctrl = &g_timer0_ctrl, .p_cfg =
		&g_timer0_cfg, .p_api = &g_timer_on_gpt };
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
transfer_info_t g_transfer1_info = { .dest_addr_mode =
		TRANSFER_ADDR_MODE_INCREMENTED, .repeat_area =
		TRANSFER_REPEAT_AREA_DESTINATION, .irq = TRANSFER_IRQ_END, .chain_mode =
		TRANSFER_CHAIN_MODE_DISABLED, .src_addr_mode = TRANSFER_ADDR_MODE_FIXED,
		.size = TRANSFER_SIZE_2_BYTE, .mode = TRANSFER_MODE_NORMAL, .p_dest =
				(void*) NULL, .p_src = (void const*) NULL, .num_blocks = 0,
		.length = 0, };
const transfer_cfg_t g_transfer1_cfg = { .p_info = &g_transfer1_info,
		.activation_source = ELC_EVENT_SPI0_RXI, .auto_enable = false,
		.p_callback = NULL, .p_context = &g_transfer1, .irq_ipl =
				(BSP_IRQ_DISABLED) };
/* Instance structure to use this module. */
const transfer_instance_t g_transfer1 = { .p_ctrl = &g_transfer1_ctrl, .p_cfg =
		&g_transfer1_cfg, .p_api = &g_transfer_on_dtc };
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
transfer_info_t g_transfer0_info = { .dest_addr_mode = TRANSFER_ADDR_MODE_FIXED,
		.repeat_area = TRANSFER_REPEAT_AREA_SOURCE, .irq = TRANSFER_IRQ_END,
		.chain_mode = TRANSFER_CHAIN_MODE_DISABLED, .src_addr_mode =
				TRANSFER_ADDR_MODE_INCREMENTED, .size = TRANSFER_SIZE_2_BYTE,
		.mode = TRANSFER_MODE_NORMAL, .p_dest = (void*) NULL, .p_src =
				(void const*) NULL, .num_blocks = 0, .length = 0, };
const transfer_cfg_t g_transfer0_cfg = { .p_info = &g_transfer0_info,
		.activation_source = ELC_EVENT_SPI0_TXI, .auto_enable = false,
		.p_callback = NULL, .p_context = &g_transfer0, .irq_ipl =
				(BSP_IRQ_DISABLED) };
/* Instance structure to use this module. */
const transfer_instance_t g_transfer0 = { .p_ctrl = &g_transfer0_ctrl, .p_cfg =
		&g_transfer0_cfg, .p_api = &g_transfer_on_dtc };
#define RSPI_TRANSFER_SIZE_1_BYTE (0x52535049)
#define RSPI_SYNERGY_NOT_DEFINED 1
#if (RSPI_SYNERGY_NOT_DEFINED != RSPI_TRANSFER_SIZE_2_BYTE)
dtc_instance_ctrl_t g_spi0_transfer_tx_ctrl;
uint32_t g_spi0_tx_inter = 0;
transfer_info_t g_spi0_transfer_tx_info[2] = {
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
		{ .dest_addr_mode = TRANSFER_ADDR_MODE_FIXED, .repeat_area =
				TRANSFER_REPEAT_AREA_SOURCE, .irq = TRANSFER_IRQ_END,
				.chain_mode = TRANSFER_CHAIN_MODE_DISABLED, .src_addr_mode =
						TRANSFER_ADDR_MODE_INCREMENTED, .size =
						TRANSFER_SIZE_2_BYTE, .mode = TRANSFER_MODE_NORMAL,
				.p_dest = (void*) NULL, .p_src = (void const*) NULL,
				.num_blocks = 0, .length = 0, }, { .dest_addr_mode =
				TRANSFER_ADDR_MODE_FIXED, .repeat_area =
				TRANSFER_REPEAT_AREA_SOURCE, .irq = TRANSFER_IRQ_END,
				.chain_mode = TRANSFER_CHAIN_MODE_DISABLED, .src_addr_mode =
						TRANSFER_ADDR_MODE_FIXED, .size = TRANSFER_SIZE_4_BYTE,
				.mode = TRANSFER_MODE_NORMAL, .p_dest = (void*) NULL, .p_src =
						(void const*) &g_spi0_tx_inter, .num_blocks = 0,
				.length = 0, },
#endif
		};

const transfer_cfg_t g_spi0_transfer_tx_cfg = { .p_info =
		g_spi0_transfer_tx_info, .activation_source = ELC_EVENT_SPI0_TXI,
		.auto_enable = false, .p_callback = NULL, .p_context =
				&g_spi0_transfer_tx, .irq_ipl = (BSP_IRQ_DISABLED) };
/* Instance structure to use this module. */
const transfer_instance_t g_spi0_transfer_tx = { .p_ctrl =
		&g_spi0_transfer_tx_ctrl, .p_cfg = &g_spi0_transfer_tx_cfg, .p_api =
		&g_transfer_on_dtc };

dtc_instance_ctrl_t g_spi0_transfer_rx_ctrl;
uint32_t g_spi0_rx_inter = 0;
transfer_info_t g_spi0_transfer_rx_info[2] = {
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
		{ .dest_addr_mode = TRANSFER_ADDR_MODE_INCREMENTED, .repeat_area =
				TRANSFER_REPEAT_AREA_DESTINATION, .irq = TRANSFER_IRQ_END,
				.chain_mode = TRANSFER_CHAIN_MODE_DISABLED, .src_addr_mode =
						TRANSFER_ADDR_MODE_FIXED, .size = TRANSFER_SIZE_2_BYTE,
				.mode = TRANSFER_MODE_NORMAL, .p_dest = (void*) NULL, .p_src =
						(void const*) NULL, .num_blocks = 0, }, {
				.dest_addr_mode = TRANSFER_ADDR_MODE_INCREMENTED, .repeat_area =
						TRANSFER_REPEAT_AREA_SOURCE, .irq = TRANSFER_IRQ_END,
				.chain_mode = TRANSFER_CHAIN_MODE_DISABLED, .src_addr_mode =
						TRANSFER_ADDR_MODE_FIXED, .size = TRANSFER_SIZE_1_BYTE,
				.mode = TRANSFER_MODE_NORMAL, .p_dest = (void*) NULL, .p_src =
						(void const*) &g_spi0_rx_inter, .num_blocks = 0,
				.length = 0, },
#endif
		};

const transfer_cfg_t g_spi0_transfer_rx_cfg = { .p_info =
		g_spi0_transfer_rx_info, .activation_source = ELC_EVENT_SPI0_RXI,
		.auto_enable = false, .p_callback = NULL, .p_context =
				&g_spi0_transfer_rx, .irq_ipl = (BSP_IRQ_DISABLED) };
/* Instance structure to use this module. */
const transfer_instance_t g_spi0_transfer_rx = { .p_ctrl =
		&g_spi0_transfer_rx_ctrl, .p_cfg = &g_spi0_transfer_rx_cfg, .p_api =
		&g_transfer_on_dtc };
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
		.rspi_comm = RSPI_COMMUNICATION_FULL_DUPLEX, .ssl_polarity.rspi_ssl0 =
				RSPI_SSLP_LOW, .loopback.rspi_loopback1 =
				RSPI_LOOPBACK1_NORMAL_DATA, .loopback.rspi_loopback2 =
				RSPI_LOOPBACK2_NORMAL_DATA,
				.mosi_idle.rspi_mosi_idle_fixed_val =
						RSPI_MOSI_IDLE_FIXED_VAL_LOW,
				.mosi_idle.rspi_mosi_idle_val_fixing =
						RSPI_MOSI_IDLE_VAL_FIXING_DISABLE, .parity.rspi_parity =
						RSPI_PARITY_STATE_DISABLE, .parity.rspi_parity_mode =
						RSPI_PARITY_MODE_ODD,
				.ssl_select = RSPI_SSL_SELECT_SSL0, .ssl_level_keep =
						RSPI_SSL_LEVEL_KEEP_NOT,
				.clock_delay.rspi_clock_delay_count = RSPI_CLOCK_DELAY_COUNT_1,
				.clock_delay.rspi_clock_delay_state =
						RSPI_CLOCK_DELAY_STATE_DISABLE,
				.ssl_neg_delay.rspi_ssl_neg_delay_count =
						RSPI_SSL_NEGATION_DELAY_1,
				.ssl_neg_delay.rspi_ssl_neg_delay_state =
						RSPI_SSL_NEGATION_DELAY_DISABLE,
				.access_delay.rspi_next_access_delay_count =
						RSPI_NEXT_ACCESS_DELAY_COUNT_1,
				.access_delay.rspi_next_access_delay_state =
						RSPI_NEXT_ACCESS_DELAY_STATE_DISABLE, .byte_swap =
						RSPI_BYTE_SWAP_DISABLE, };

const spi_cfg_t g_spi0_cfg = { .channel = 0, .operating_mode = SPI_MODE_MASTER,
		.clk_phase = SPI_CLK_PHASE_EDGE_ODD, .clk_polarity =
				SPI_CLK_POLARITY_LOW,
		.mode_fault = SPI_MODE_FAULT_ERROR_DISABLE, .bit_order =
				SPI_BIT_ORDER_MSB_FIRST, .bitrate = 1000000, .p_transfer_tx =
				g_spi0_P_TRANSFER_TX, .p_transfer_rx = g_spi0_P_TRANSFER_RX,
		.p_callback = rfid_spi_callback, .p_context = (void*) &g_spi0,
		.p_extend = (void*) &g_spi0_ext_cfg, .rxi_ipl = (12), .txi_ipl = (12),
		.eri_ipl = (12), .tei_ipl = (12), };
/* Instance structure to use this module. */
const spi_instance_t g_spi0 = { .p_ctrl = &g_spi0_ctrl, .p_cfg = &g_spi0_cfg,
		.p_api = &g_spi_on_rspi };
TX_SEMAPHORE g_main_semaphore_lcdc;
extern bool g_ssp_common_initialized;
extern uint32_t g_ssp_common_thread_count;
extern TX_SEMAPHORE g_ssp_common_initialized_semaphore;

void main_thread_create(void) {
	/* Increment count so we will know the number of ISDE created threads. */
	g_ssp_common_thread_count++;

	/* Initialize each kernel object. */
	UINT err_g_main_semaphore_lcdc;
	err_g_main_semaphore_lcdc = tx_semaphore_create(&g_main_semaphore_lcdc,
			(CHAR*) "New Semaphore", 0);
	if (TX_SUCCESS != err_g_main_semaphore_lcdc) {
		tx_startup_err_callback(&g_main_semaphore_lcdc, 0);
	}

	UINT err;
	err = tx_thread_create(&main_thread, (CHAR*) "Main Thread",
			main_thread_func, (ULONG) NULL, &main_thread_stack, 4096, 5, 5, 1,
			TX_AUTO_START);
	if (TX_SUCCESS != err) {
		tx_startup_err_callback(&main_thread, 0);
	}
}

static void main_thread_func(ULONG thread_input) {
	/* Not currently using thread_input. */
	SSP_PARAMETER_NOT_USED(thread_input);

	/* Initialize common components */
	tx_startup_common_init();

	/* Initialize each module instance. */
	/** Call initialization function if user has selected to do so. */
#if (1)
	http_server_init0();
#endif

	/* Enter user code for this thread. */
	main_thread_entry();
}
