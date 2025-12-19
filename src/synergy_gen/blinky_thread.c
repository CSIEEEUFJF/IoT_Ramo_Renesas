/* generated thread source file - do not edit */
#include "blinky_thread.h"

TX_THREAD blinky_thread;
void blinky_thread_create(void);
static void blinky_thread_func(ULONG thread_input);
static uint8_t blinky_thread_stack[1024] BSP_PLACE_IN_SECTION_V2(".stack.blinky_thread") BSP_ALIGN_VARIABLE_V2(BSP_STACK_ALIGNMENT);
void tx_startup_err_callback(void *p_instance, void *p_data);
void tx_startup_common_init(void);
/** Get driver cfg from bus and use all same settings except slave address and addressing mode. */
const spi_cfg_t g_sf_spi0_spi_cfg =
{ .channel = g_sf_spi_bus0_CHANNEL,
  .operating_mode = g_sf_spi_bus0_OPERATING_MODE,
  .clk_phase = SPI_CLK_PHASE_EDGE_ODD,
  .clk_polarity = SPI_CLK_POLARITY_LOW,
  .mode_fault = g_sf_spi_bus0_MODE_FAULT,
  .bit_order = g_sf_spi_bus0_BIT_ORDER,
  .bitrate = g_sf_spi_bus0_BIT_RATE,
  .p_transfer_tx = g_sf_spi_bus0_P_TRANSFER_TX,
  .p_transfer_rx = g_sf_spi_bus0_P_TRANSFER_RX,
  .p_callback = g_sf_spi_bus0_P_CALLBACK,
  .p_context = g_sf_spi_bus0_P_CONTEXT,
  .rxi_ipl = g_sf_spi_bus0_RXI_IPL,
  .txi_ipl = g_sf_spi_bus0_TXI_IPL,
  .tei_ipl = g_sf_spi_bus0_TEI_IPL,
  .eri_ipl = g_sf_spi_bus0_ERI_IPL,
  .p_extend = g_sf_spi_bus0_P_EXTEND, };

sf_spi_instance_ctrl_t g_sf_spi0_ctrl =
{ .p_lower_lvl_ctrl = &g_spi0_ctrl, };

const sf_spi_cfg_t g_sf_spi0_cfg =
{ .p_bus = (sf_spi_bus_t*) &g_sf_spi_bus0, .chip_select = IOPORT_PORT_01_PIN_03, .chip_select_level_active =
          IOPORT_LEVEL_LOW,
  .p_lower_lvl_cfg = &g_sf_spi0_spi_cfg, };

/* Instance structure to use this module. */
const sf_spi_instance_t g_sf_spi0 =
{ .p_ctrl = &g_sf_spi0_ctrl, .p_cfg = &g_sf_spi0_cfg, .p_api = &g_sf_spi_on_sf_spi };
TX_QUEUE g_command_queue;
static uint8_t queue_memory_g_command_queue[20];
extern bool g_ssp_common_initialized;
extern uint32_t g_ssp_common_thread_count;
extern TX_SEMAPHORE g_ssp_common_initialized_semaphore;

void blinky_thread_create(void)
{
    /* Increment count so we will know the number of ISDE created threads. */
    g_ssp_common_thread_count++;

    /* Initialize each kernel object. */
    UINT err_g_command_queue;
    err_g_command_queue = tx_queue_create (&g_command_queue, (CHAR*) "New Queue", 1, &queue_memory_g_command_queue,
                                           sizeof(queue_memory_g_command_queue));
    if (TX_SUCCESS != err_g_command_queue)
    {
        tx_startup_err_callback (&g_command_queue, 0);
    }

    UINT err;
    err = tx_thread_create (&blinky_thread, (CHAR*) "Blinky Thread", blinky_thread_func, (ULONG) NULL,
                            &blinky_thread_stack, 1024, 1, 1, 1, TX_AUTO_START);
    if (TX_SUCCESS != err)
    {
        tx_startup_err_callback (&blinky_thread, 0);
    }
}

static void blinky_thread_func(ULONG thread_input)
{
    /* Not currently using thread_input. */
    SSP_PARAMETER_NOT_USED (thread_input);

    /* Initialize common components */
    tx_startup_common_init ();

    /* Initialize each module instance. */

    /* Enter user code for this thread. */
    blinky_thread_entry ();
}
