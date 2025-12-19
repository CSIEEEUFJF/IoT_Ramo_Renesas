/* generated thread source file - do not edit */
#include "new_thread0.h"

TX_THREAD new_thread0;
void new_thread0_create(void);
static void new_thread0_func(ULONG thread_input);
static uint8_t new_thread0_stack[1024] BSP_PLACE_IN_SECTION_V2(".stack.new_thread0") BSP_ALIGN_VARIABLE_V2(BSP_STACK_ALIGNMENT);
void tx_startup_err_callback(void *p_instance, void *p_data);
void tx_startup_common_init(void);
#if (12) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_g_external_irq0) && !defined(SSP_SUPPRESS_ISR_ICU0)
SSP_VECTOR_DEFINE( icu_irq_isr, ICU, IRQ0);
#endif
#endif
static icu_instance_ctrl_t g_external_irq0_ctrl;
static const external_irq_cfg_t g_external_irq0_cfg =
{ .channel = 0,
  .trigger = EXTERNAL_IRQ_TRIG_RISING,
  .filter_enable = false,
  .pclk_div = EXTERNAL_IRQ_PCLK_DIV_BY_64,
  .autostart = true,
  .p_callback = NULL,
  .p_context = &g_external_irq0,
  .p_extend = NULL,
  .irq_ipl = (12), };
/* Instance structure to use this module. */
const external_irq_instance_t g_external_irq0 =
{ .p_ctrl = &g_external_irq0_ctrl, .p_cfg = &g_external_irq0_cfg, .p_api = &g_external_irq_on_icu };
sf_external_irq_instance_ctrl_t g_sf_external_irq0_ctrl;
const sf_external_irq_cfg_t g_sf_external_irq0_cfg =
{ .event = SF_EXTERNAL_IRQ_EVENT_SEMAPHORE_PUT, .p_lower_lvl_irq = &g_external_irq0, };
/* Instance structure to use this module. */
const sf_external_irq_instance_t g_sf_external_irq0 =
{ .p_ctrl = &g_sf_external_irq0_ctrl, .p_cfg = &g_sf_external_irq0_cfg, .p_api = &g_sf_external_irq_on_sf_external_irq };
/** Get driver cfg from bus and use all same settings except slave address and addressing mode. */
const i2c_cfg_t g_sf_i2c_device0_i2c_cfg =
{ .channel = g_sf_i2c_bus0_CHANNEL,
  .rate = g_sf_i2c_bus0_RATE,
  .slave = 0x00,
  .addr_mode = I2C_ADDR_MODE_7BIT,
  .sda_delay = g_sf_i2c_bus0_SDA_DELAY,
  .p_transfer_tx = g_sf_i2c_bus0_P_TRANSFER_TX,
  .p_transfer_rx = g_sf_i2c_bus0_P_TRANSFER_RX,
  .p_callback = g_sf_i2c_bus0_P_CALLBACK,
  .p_context = g_sf_i2c_bus0_P_CONTEXT,
  .rxi_ipl = g_sf_i2c_bus0_RXI_IPL,
  .txi_ipl = g_sf_i2c_bus0_TXI_IPL,
  .tei_ipl = g_sf_i2c_bus0_TEI_IPL,
  .eri_ipl = g_sf_i2c_bus0_ERI_IPL,
  .p_extend = g_sf_i2c_bus0_P_EXTEND, };

sf_i2c_instance_ctrl_t g_sf_i2c_device0_ctrl =
{ .p_lower_lvl_ctrl = &g_i2c0_ctrl, };
const sf_i2c_cfg_t g_sf_i2c_device0_cfg =
{ .p_bus = (sf_i2c_bus_t*) &g_sf_i2c_bus0, .p_lower_lvl_cfg = &g_sf_i2c_device0_i2c_cfg, };
/* Instance structure to use this module. */
const sf_i2c_instance_t g_sf_i2c_device0 =
{ .p_ctrl = &g_sf_i2c_device0_ctrl, .p_cfg = &g_sf_i2c_device0_cfg, .p_api = &g_sf_i2c_on_sf_i2c };
sf_touch_panel_chip_sx8654_instance_ctrl_t g_touch_panel_chip_sx8654_0_ctrl;
const sf_touch_panel_chip_on_sx8654_cfg_t g_touch_panel_chip_sx8654_0_cfg_extend =
{ .pin = IOPORT_PORT_06_PIN_09,
  .p_lower_lvl_framewrk = &g_sf_i2c_device0,
  .p_lower_lvl_irq = &g_sf_external_irq0,
  .hsize_pixels = 480,
  .vsize_pixels = 272 };
const sf_touch_panel_chip_cfg_t g_touch_panel_chip_sx8654_0_cfg =
{ .p_extend = &g_touch_panel_chip_sx8654_0_cfg_extend };
const sf_touch_panel_chip_instance_t g_touch_panel_chip_sx8654_0 =
{ .p_ctrl = &g_touch_panel_chip_sx8654_0_ctrl, .p_cfg = &g_touch_panel_chip_sx8654_0_cfg, .p_api =
          &g_sf_touch_panel_chip_sx8654 };
#if defined(__ICCARM__)
            #define g_sf_touch_panel_v2_0_err_callback_WEAK_ATTRIBUTE
            #pragma weak g_sf_touch_panel_v2_0_err_callback  = g_sf_touch_panel_v2_0_err_callback_internal
            #elif defined(__GNUC__)
            #define g_sf_touch_panel_v2_0_err_callback_WEAK_ATTRIBUTE   __attribute__ ((weak, alias("g_sf_touch_panel_v2_0_err_callback_internal")))
            #endif
void g_sf_touch_panel_v2_0_err_callback(void *p_instance, void *p_data)
g_sf_touch_panel_v2_0_err_callback_WEAK_ATTRIBUTE;
sf_touch_panel_v2_instance_ctrl_t g_sf_touch_panel_v2_0_ctrl;
const sf_touch_panel_v2_extend_cfg_t g_sf_touch_panel_v2_0_cfg_extend =
{ .p_chip = &g_touch_panel_chip_sx8654_0 };
const sf_touch_panel_v2_cfg_t g_sf_touch_panel_v2_0_cfg =
{ .hsize_pixels = 480, .vsize_pixels = 272, .priority = 3, .update_hz = 10, .p_extend =
          &g_sf_touch_panel_v2_0_cfg_extend,
  .rotation_angle = 0, .p_callback = NULL, .p_context = &g_sf_touch_panel_v2_0 };

/* Instance structure to use this module. */
const sf_touch_panel_v2_instance_t g_sf_touch_panel_v2_0 =
{ .p_ctrl = &g_sf_touch_panel_v2_0_ctrl, .p_cfg = &g_sf_touch_panel_v2_0_cfg, .p_api =
          &g_sf_touch_panel_v2_on_sf_touch_panel_v2 };
/*******************************************************************************************************************//**
 * @brief      Initialization function that the user can choose to have called automatically during thread entry.
 *             The user can call this function at a later time if desired using the prototype below.

 *             - void g_sf_touch_panel_v2_0_err_callback(void * p_instance, void * p_data)
 *
 * @param[in]  p_instance arguments used to identify which instance caused the error and p_data Callback arguments
 used to identify what error caused the callback.
 **********************************************************************************************************************/
void g_sf_touch_panel_v2_0_err_callback_internal(void *p_instance, void *p_data);
void g_sf_touch_panel_v2_0_err_callback_internal(void *p_instance, void *p_data)
{
    /** Suppress compiler warning for not using parameters. */
    SSP_PARAMETER_NOT_USED (p_instance);
    SSP_PARAMETER_NOT_USED (p_data);
    /** An error has occurred. Please check function arguments for more information. */
    BSP_CFG_HANDLE_UNRECOVERABLE_ERROR (0);
}
/*******************************************************************************************************************//**
 * @brief     This is sf touch panel initialization function. User Can call this function in the application 
 if required with the below mentioned prototype.
 *            - void sf_touch_panel_v2_init0(void)
 **********************************************************************************************************************/
void sf_touch_panel_v2_init0(void)
{
    ssp_err_t ssp_err_g_sf_touch_panel_v2_0;
    ssp_err_g_sf_touch_panel_v2_0 = g_sf_touch_panel_v2_0.p_api->open (g_sf_touch_panel_v2_0.p_ctrl,
                                                                       g_sf_touch_panel_v2_0.p_cfg);
    if (SSP_SUCCESS != ssp_err_g_sf_touch_panel_v2_0)
    {
        g_sf_touch_panel_v2_0_err_callback ((void*) &g_sf_touch_panel_v2_0, &ssp_err_g_sf_touch_panel_v2_0);
    }
    if (1)
    {
        ssp_err_g_sf_touch_panel_v2_0 = g_sf_touch_panel_v2_0.p_api->start (g_sf_touch_panel_v2_0.p_ctrl);
        if (SSP_SUCCESS != ssp_err_g_sf_touch_panel_v2_0)
        {
            g_sf_touch_panel_v2_0_err_callback ((void*) &g_sf_touch_panel_v2_0, &ssp_err_g_sf_touch_panel_v2_0);
        }
    }
}
#if (BSP_IRQ_DISABLED) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_g_display0) && !defined(SSP_SUPPRESS_ISR_GLCD)
SSP_VECTOR_DEFINE(glcdc_line_detect_isr, GLCDC, LINE_DETECT);
#endif
#endif
#if (BSP_IRQ_DISABLED) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_g_display0) && !defined(SSP_SUPPRESS_ISR_GLCD)
SSP_VECTOR_DEFINE(glcdc_underflow_1_isr, GLCDC, UNDERFLOW_1);
#endif
#endif
#if (BSP_IRQ_DISABLED) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_g_display0) && !defined(SSP_SUPPRESS_ISR_GLCD)
SSP_VECTOR_DEFINE(glcdc_underflow_2_isr, GLCDC, UNDERFLOW_2);
#endif
#endif

/** Display frame buffer */
#if (true)
        uint8_t g_display0_fb_background[2][((800 * 480) * DISPLAY_BITS_PER_PIXEL_INPUT0) >> 3] BSP_ALIGN_VARIABLE_V2(64) BSP_PLACE_IN_SECTION_V2(".sdram");
        #else
/** Graphics screen1 is specified not to be used when starting */
#endif
#if (false)
        uint8_t g_display0_fb_foreground[2][((800 * 480) * DISPLAY_BITS_PER_PIXEL_INPUT1) >> 3] BSP_ALIGN_VARIABLE_V2(64) BSP_PLACE_IN_SECTION_V2(".sdram");
        #else
/** Graphics screen2 is specified not to be used when starting */
#endif

#if (false)
        /** Display CLUT buffer to be used for updating CLUT */
        static uint32_t CLUT_buffer[256];

        /** Display CLUT configuration(only used if using CLUT format) */
        display_clut_cfg_t g_display0_clut_cfg_glcd =
        {
            .p_base              = (uint32_t *)CLUT_buffer,
            .start               = 0,   /* User have to update this setting when using */
            .size                = 256  /* User have to update this setting when using */
        };
        #else
/** CLUT is specified not to be used */
#endif

#if (false | false | false)
        /** Display interface configuration */
        static const display_gamma_correction_t g_display0_gamma_cfg =
        {
            .r =
            {
                .enable      = false,
                .gain        = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                .threshold   = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
            },
            .g =
            {
                .enable      = false,
                .gain        = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                .threshold   = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
            },
            .b =
            {
                .enable      = false,
                .gain        = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                .threshold   = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
            }
        };
        #endif

/** Display device extended configuration */
static const glcd_cfg_t g_display0_extend_cfg =
{ .tcon_hsync = GLCD_TCON_PIN_0,
  .tcon_vsync = GLCD_TCON_PIN_1,
  .tcon_de = GLCD_TCON_PIN_2,
  .correction_proc_order = GLCD_CORRECTION_PROC_ORDER_BRIGHTNESS_CONTRAST2GAMMA,
  .clksrc = GLCD_CLK_SRC_INTERNAL,
  .clock_div_ratio = GLCD_PANEL_CLK_DIVISOR_8,
  .dithering_mode = GLCD_DITHERING_MODE_TRUNCATE,
  .dithering_pattern_A = GLCD_DITHERING_PATTERN_11,
  .dithering_pattern_B = GLCD_DITHERING_PATTERN_11,
  .dithering_pattern_C = GLCD_DITHERING_PATTERN_11,
  .dithering_pattern_D = GLCD_DITHERING_PATTERN_11 };

/** Display control block instance */
glcd_instance_ctrl_t g_display0_ctrl;

/** Display interface configuration */
const display_cfg_t g_display0_cfg =
        {
        /** Input1(Graphics1 screen) configuration */
        .input[0
                  ] =
                  {
#if (true)
                .p_base              = (uint32_t *)&g_display0_fb_background[0],
                #else
                    .p_base = NULL,
#endif
                    .hsize = 800,
                    .vsize = 480, .hstride = 800, .format = DISPLAY_IN_FORMAT_16BITS_RGB565, .line_descending_enable =
                            false,
                    .lines_repeat_enable = false, .lines_repeat_times = 0 },

          /** Input2(Graphics2 screen) configuration */
          .input[1
                  ] =
                  {
#if (false)
                .p_base              = (uint32_t *)&g_display0_fb_foreground[0],
                #else
                    .p_base = NULL,
#endif
                    .hsize = 800,
                    .vsize = 480, .hstride = 800, .format = DISPLAY_IN_FORMAT_16BITS_RGB565, .line_descending_enable =
                            false,
                    .lines_repeat_enable = false, .lines_repeat_times = 0 },

          /** Input1(Graphics1 screen) layer configuration */
          .layer[0] =
          { .coordinate =
          { .x = 0, .y = 0 },
            .bg_color =
            { .byte =
            { .a = 255, .r = 255, .g = 255, .b = 255 } },
            .fade_control = DISPLAY_FADE_CONTROL_NONE, .fade_speed = 0 },

          /** Input2(Graphics2 screen) layer configuration */
          .layer[1] =
          { .coordinate =
          { .x = 0, .y = 0 },
            .bg_color =
            { .byte =
            { .a = 255, .r = 255, .g = 255, .b = 255 } },
            .fade_control = DISPLAY_FADE_CONTROL_NONE, .fade_speed = 0 },

          /** Output configuration */
          .output =
                  { .htiming =
                  { .total_cyc = 1024, .display_cyc = 800, .back_porch = 46, .sync_width = 20, .sync_polarity =
                            DISPLAY_SIGNAL_POLARITY_LOACTIVE },
                    .vtiming =
                    { .total_cyc = 525, .display_cyc = 480, .back_porch = 23, .sync_width = 10, .sync_polarity =
                              DISPLAY_SIGNAL_POLARITY_LOACTIVE },
                    .format = DISPLAY_OUT_FORMAT_24BITS_RGB888, .endian = DISPLAY_ENDIAN_LITTLE, .color_order =
                            DISPLAY_COLOR_ORDER_RGB,
                    .data_enable_polarity = DISPLAY_SIGNAL_POLARITY_HIACTIVE, .sync_edge =
                            DISPLAY_SIGNAL_SYNC_EDGE_RISING,
                    .bg_color =
                    { .byte =
                    { .a = 255, .r = 0, .g = 0, .b = 0 } },
                    .brightness =
                    { .enable = false, .r = 512, .g = 512, .b = 512 },
                    .contrast =
                    { .enable = false, .r = 128, .g = 128, .b = 128 },
#if (false | false | false)
                .p_gamma_correction  = (display_gamma_correction_t *)(&g_display0_gamma_cfg),
#else
                    .p_gamma_correction = NULL,
#endif
                    .dithering_on = false },

          /** Display device callback function pointer */
          .p_callback = NULL,
          .p_context = (void*) &g_display0,

          /** Display device extended configuration */
          .p_extend = (void*) (&g_display0_extend_cfg),

          .line_detect_ipl = (BSP_IRQ_DISABLED),
          .underflow_1_ipl = (BSP_IRQ_DISABLED), .underflow_2_ipl = (BSP_IRQ_DISABLED), };

#if (true)
        /** Display on GLCD run-time configuration(for the graphics1 screen) */
        display_runtime_cfg_t g_display0_runtime_cfg_bg =
        {
            .input =
            {
                #if (true)
                .p_base              = (uint32_t *)&g_display0_fb_background[0],
                #else
                .p_base              = NULL,
                #endif
                .hsize               = 800,
                .vsize               = 480,
                .hstride             = 800,
                .format              = DISPLAY_IN_FORMAT_16BITS_RGB565,
                .line_descending_enable = false,
                .lines_repeat_enable = false,
                .lines_repeat_times  = 0
            },
            .layer =
            {
                .coordinate = {
                        .x           = 0,
                        .y           = 0
                },
                .bg_color            =
                {
                    .byte            =
                    {
                        .a           = 255,
                        .r           = 255,
                        .g           = 255,
                        .b           = 255
                    }
                },
                .fade_control        = DISPLAY_FADE_CONTROL_NONE,
                .fade_speed          = 0
            }
        };
#endif
#if (false)
        /** Display on GLCD run-time configuration(for the graphics2 screen) */
        display_runtime_cfg_t g_display0_runtime_cfg_fg =
        {
            .input =
            {
                #if (false)
                .p_base              = (uint32_t *)&g_display0_fb_foreground[0],
                #else
                .p_base              = NULL,
                #endif
                .hsize               = 800,
                .vsize               = 480,
                .hstride             = 800,
                .format              = DISPLAY_IN_FORMAT_16BITS_RGB565,
                .line_descending_enable = false,
                .lines_repeat_enable = false,
                .lines_repeat_times  = 0
             },
            .layer =
            {
                .coordinate = {
                        .x           = 0,
                        .y           = 0
                },
                .bg_color            =
                {
                    .byte            =
                    {
                        .a           = 255,
                        .r           = 255,
                        .g           = 255,
                        .b           = 255
                    }
                },
                .fade_control        = DISPLAY_FADE_CONTROL_NONE,
                .fade_speed          = 0
            }
        };
#endif

/* Instance structure to use this module. */
const display_instance_t g_display0 =
{ .p_ctrl = &g_display0_ctrl, .p_cfg = (display_cfg_t*) &g_display0_cfg, .p_api = (display_api_t*) &g_display_on_glcd };
/** GUIX Canvas Buffer */
#if false
            #if (1 == 1) /* Inherit Frame Buffer Name from Graphics Screen 1 */
            uint8_t g_sf_el_gx0_canvas[sizeof(g_display0_fb_background[0])] BSP_ALIGN_VARIABLE_V2(4) BSP_PLACE_IN_SECTION_V2(".sdram");
            #else /* Inherit Frame Buffer Name from Graphics Screen 2 */
            uint8_t g_sf_el_gx0_canvas[sizeof(g_display0_fb_foreground[0])] BSP_ALIGN_VARIABLE_V2(4) BSP_PLACE_IN_SECTION_V2(".sdram");
            #endif
            #endif

/** JPEG Work Buffer */
#if GX_USE_SYNERGY_JPEG
            #if (768000)
            uint8_t g_sf_el_gx0_jpegbuffer[768000] BSP_ALIGN_VARIABLE_V2(64) BSP_PLACE_IN_SECTION_V2(".sdram");
            #endif
            #endif

/** GUIX Port module control block instance */
static sf_el_gx_instance_ctrl_t g_sf_el_gx0_ctrl;

/** GUIX Port module configuration */
static const sf_el_gx_cfg_t g_sf_el_gx0_cfg =
{
/* Display Instance Configuration */
.p_display_instance = (display_instance_t*) &g_display0,

  /* Display Driver Runtime Configuration */
#if (1 == 1) /* Inherit Frame Buffer Name from Graphics Screen 1 */
  .p_display_runtime_cfg = &g_display0_runtime_cfg_bg,
#else /* Inherit Frame Buffer Name from Graphics Screen 2 */
                .p_display_runtime_cfg = &g_display0_runtime_cfg_fg,
            #endif

  /* GUIX Canvas Configuration */
#if (false)
                .p_canvas        = g_sf_el_gx0_canvas,
            #else
  .p_canvas = NULL,
#endif

  /* Display Driver Frame Buffer A Configuration */
#if (1 == 1) /* Inherit Frame Buffer Name from Graphics Screen 1 */
  .p_framebuffer_a = &g_display0_fb_background[0], /* Always array[0] is used */
  .inherit_frame_layer = DISPLAY_FRAME_LAYER_1,
#else /* Inherit Frame Buffer Name from Graphics Screen 2 */
                .p_framebuffer_a = &g_display0_fb_foreground[0], /* Always array[0] is used */
	         .inherit_frame_layer = DISPLAY_FRAME_LAYER_2,
            #endif

  /* Display Driver Frame Buffer B Configuration */
#if (1 == 1) /* Inherit Frame Buffer Name from Graphics Screen 1 */
#if (2 > 1) /* Multiple frame buffers are used for Graphics Screen 1 */
  .p_framebuffer_b = &g_display0_fb_background[1], /* Always array[1] is used */
#else /* Single Frame Buffer is used for Graphics Screen 1 */
                .p_framebuffer_b = NULL,
                #endif
#else /* Inherit Frame Buffer Name from Graphics Screen 2 */
                #if (2 > 1) /* Multiple frame buffers are used for Graphics Screen 2 */
                .p_framebuffer_b = &g_display0_fb_foreground[1], /* Always array[1] is used */
                #else /* Single Frame Buffer is used for Graphics Screen 2 */
                .p_framebuffer_b = NULL,
                #endif
            #endif

  /* User Callback Configuration */
  .p_callback = NULL,

  /* JPEG Work Buffer Configuration */
#if GX_USE_SYNERGY_JPEG
                .p_jpegbuffer    = g_sf_el_gx0_jpegbuffer,
                .jpegbuffer_size = 768000,
                .p_sf_jpeg_decode_instance = (void *)&SYNERGY_NOT_DEFINED,
            #else
  .p_jpegbuffer = NULL,
  .jpegbuffer_size = 0, .p_sf_jpeg_decode_instance = NULL,
#endif

  /* D/AVE 2D Buffer Cache */
  .dave2d_buffer_cache_enabled = true };

/** GUIX Port module instance */
sf_el_gx_instance_t g_sf_el_gx0 =
{ .p_api = &sf_el_gx_on_guix, .p_ctrl = &g_sf_el_gx0_ctrl, .p_cfg = &g_sf_el_gx0_cfg };
extern bool g_ssp_common_initialized;
extern uint32_t g_ssp_common_thread_count;
extern TX_SEMAPHORE g_ssp_common_initialized_semaphore;

void new_thread0_create(void)
{
    /* Increment count so we will know the number of ISDE created threads. */
    g_ssp_common_thread_count++;

    /* Initialize each kernel object. */

    UINT err;
    err = tx_thread_create (&new_thread0, (CHAR*) "New Thread", new_thread0_func, (ULONG) NULL, &new_thread0_stack,
                            1024, 1, 1, 1, TX_AUTO_START);
    if (TX_SUCCESS != err)
    {
        tx_startup_err_callback (&new_thread0, 0);
    }
}

static void new_thread0_func(ULONG thread_input)
{
    /* Not currently using thread_input. */
    SSP_PARAMETER_NOT_USED (thread_input);

    /* Initialize common components */
    tx_startup_common_init ();

    /* Initialize each module instance. */
    /** Call initialization function if user has selected to do so. */
#if (1)
    sf_touch_panel_v2_init0 ();
#endif

    /* Enter user code for this thread. */
    new_thread0_entry ();
}
