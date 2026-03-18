/* generated common source file - do not edit */
#include "common_data.h"

#ifndef NX_DISABLE_IPV6
#ifndef FILL_NXD_IPV6_ADDRESS
#define FILL_NXD_IPV6_ADDRESS(ipv6,f0,f1,f2,f3,f4,f5,f6,f7) do { \
                                                                       ipv6.nxd_ip_address.v6[0] = (((uint32_t)f0 << 16) & 0xFFFF0000) | ((uint32_t)f1 & 0x0000FFFF);\
                                                                       ipv6.nxd_ip_address.v6[1] = (((uint32_t)f2 << 16) & 0xFFFF0000) | ((uint32_t)f3 & 0x0000FFFF);\
                                                                       ipv6.nxd_ip_address.v6[2] = (((uint32_t)f4 << 16) & 0xFFFF0000) | ((uint32_t)f5 & 0x0000FFFF);\
                                                                       ipv6.nxd_ip_address.v6[3] = (((uint32_t)f6 << 16) & 0xFFFF0000) | ((uint32_t)f7 & 0x0000FFFF);\
                                                                       ipv6.nxd_ip_version       = NX_IP_VERSION_V6;\
                                                                   } while(0);
#endif /* FILL_NXD_IPV6_ADDRESS */
#endif
#if SYNERGY_NOT_DEFINED != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_DRW)
SSP_VECTOR_DEFINE(drw_int_isr, DRW, INT);
#endif
#endif
SSP_VECTOR_DEFINE( jpeg_jdti_isr, JPEG, JDTI);
SSP_VECTOR_DEFINE( jpeg_jedi_isr, JPEG, JEDI);
static jpeg_decode_instance_ctrl_t g_jpeg_decode0_ctrl;
const jpeg_decode_cfg_t g_jpeg_decode0_cfg = { .input_data_format =
		JPEG_DECODE_DATA_FORMAT_NORMAL, .output_data_format =
		JPEG_DECODE_DATA_FORMAT_NORMAL, .pixel_format =
		JPEG_DECODE_PIXEL_FORMAT_RGB565, .alpha_value = 255, .p_callback = NULL,
		.jedi_ipl = (12), .jdti_ipl = (12), };
const jpeg_decode_instance_t g_jpeg_decode0 = { .p_api =
		(jpeg_decode_api_t const*) &g_jpeg_decode_on_jpeg_decode, .p_ctrl =
		&g_jpeg_decode0_ctrl, .p_cfg = &g_jpeg_decode0_cfg };
static sf_jpeg_decode_instance_ctrl_t g_sf_jpeg_decode0_ctrl;

static const sf_jpeg_decode_cfg_t g_sf_jpeg_decode0_cfg = {
		.p_lower_lvl_jpeg_decode =
				(jpeg_decode_instance_t const*) &g_jpeg_decode0 };
const sf_jpeg_decode_instance_t g_sf_jpeg_decode0 = { .p_api =
		&g_sf_jpeg_decode_on_sf_jpeg_decode, .p_ctrl = &g_sf_jpeg_decode0_ctrl,
		.p_cfg = (sf_jpeg_decode_cfg_t const*) &g_sf_jpeg_decode0_cfg };
#if (12) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_g_display0) && !defined(SSP_SUPPRESS_ISR_GLCD)
SSP_VECTOR_DEFINE( glcdc_line_detect_isr, GLCDC, LINE_DETECT);
#endif
#endif
#if (12) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_g_display0) && !defined(SSP_SUPPRESS_ISR_GLCD)
SSP_VECTOR_DEFINE( glcdc_underflow_1_isr, GLCDC, UNDERFLOW_1);
#endif
#endif
#if (12) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_g_display0) && !defined(SSP_SUPPRESS_ISR_GLCD)
SSP_VECTOR_DEFINE( glcdc_underflow_2_isr, GLCDC, UNDERFLOW_2);
#endif
#endif

/** Display frame buffer */
#if (true)
        uint8_t g_display0_fb_background[1][((256 * 320) * DISPLAY_BITS_PER_PIXEL_INPUT0) >> 3] BSP_ALIGN_VARIABLE_V2(64) BSP_PLACE_IN_SECTION_V2(".bss");
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
static const glcd_cfg_t g_display0_extend_cfg = { .tcon_hsync = GLCD_TCON_PIN_2,
		.tcon_vsync = GLCD_TCON_PIN_1, .tcon_de = GLCD_TCON_PIN_0,
		.correction_proc_order =
				GLCD_CORRECTION_PROC_ORDER_BRIGHTNESS_CONTRAST2GAMMA, .clksrc =
				GLCD_CLK_SRC_INTERNAL, .clock_div_ratio =
				GLCD_PANEL_CLK_DIVISOR_32, .dithering_mode =
				GLCD_DITHERING_MODE_TRUNCATE, .dithering_pattern_A =
				GLCD_DITHERING_PATTERN_11, .dithering_pattern_B =
				GLCD_DITHERING_PATTERN_11, .dithering_pattern_C =
				GLCD_DITHERING_PATTERN_11, .dithering_pattern_D =
				GLCD_DITHERING_PATTERN_11 };

/** Display control block instance */
glcd_instance_ctrl_t g_display0_ctrl;

/** Display interface configuration */
const display_cfg_t g_display0_cfg =
		{
		/** Input1(Graphics1 screen) configuration */
		.input[0] =
		{
#if (true)
                .p_base              = (uint32_t *)&g_display0_fb_background[0],
                #else
				.p_base = NULL,
#endif
				.hsize = 256, .vsize = 320, .hstride = 256, .format =
						DISPLAY_IN_FORMAT_16BITS_RGB565,
				.line_descending_enable = false, .lines_repeat_enable = false,
				.lines_repeat_times = 0 },

		/** Input2(Graphics2 screen) configuration */
		.input[1] =
		{
#if (false)
                .p_base              = (uint32_t *)&g_display0_fb_foreground[0],
                #else
				.p_base = NULL,
#endif
				.hsize = 800, .vsize = 480, .hstride = 800, .format =
						DISPLAY_IN_FORMAT_16BITS_RGB565,
				.line_descending_enable = false, .lines_repeat_enable = false,
				.lines_repeat_times = 0 },

		/** Input1(Graphics1 screen) layer configuration */
		.layer[0] =
		{ .coordinate = { .x = 0, .y = 0 }, .bg_color = { .byte = { .a = 255,
				.r = 255, .g = 255, .b = 255 } }, .fade_control =
				DISPLAY_FADE_CONTROL_NONE, .fade_speed = 0 },

		/** Input2(Graphics2 screen) layer configuration */
		.layer[1] =
		{ .coordinate = { .x = 0, .y = 0 }, .bg_color = { .byte = { .a = 255,
				.r = 255, .g = 255, .b = 255 } }, .fade_control =
				DISPLAY_FADE_CONTROL_NONE, .fade_speed = 0 },

				/** Output configuration */
				.output =
						{ .htiming = { .total_cyc = 320, .display_cyc = 240,
								.back_porch = 6, .sync_width = 4,
								.sync_polarity =
										DISPLAY_SIGNAL_POLARITY_LOACTIVE },
								.vtiming =
										{ .total_cyc = 328, .display_cyc = 320,
												.back_porch = 4,
												.sync_width = 4,
												.sync_polarity =
														DISPLAY_SIGNAL_POLARITY_LOACTIVE },
								.format = DISPLAY_OUT_FORMAT_16BITS_RGB565,
								.endian = DISPLAY_ENDIAN_LITTLE, .color_order =
										DISPLAY_COLOR_ORDER_RGB,
								.data_enable_polarity =
										DISPLAY_SIGNAL_POLARITY_HIACTIVE,
								.sync_edge = DISPLAY_SIGNAL_SYNC_EDGE_RISING,
								.bg_color = { .byte = { .a = 255, .r = 0,
										.g = 0, .b = 0 } }, .brightness = {
										.enable = false, .r = 512, .g = 512,
										.b = 512 }, .contrast = { .enable =
										false, .r = 128, .g = 128, .b = 128 },
#if (false | false | false)
                .p_gamma_correction  = (display_gamma_correction_t *)(&g_display0_gamma_cfg),
#else
								.p_gamma_correction = NULL,
#endif
								.dithering_on = false },

				/** Display device callback function pointer */
				.p_callback = NULL, .p_context = (void*) &g_display0,

				/** Display device extended configuration */
				.p_extend = (void*) (&g_display0_extend_cfg),

				.line_detect_ipl = (12), .underflow_1_ipl = (12),
				.underflow_2_ipl = (12), };

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
                .hsize               = 256,
                .vsize               = 320,
                .hstride             = 256,
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
const display_instance_t g_display0 = { .p_ctrl = &g_display0_ctrl, .p_cfg =
		(display_cfg_t*) &g_display0_cfg, .p_api =
		(display_api_t*) &g_display_on_glcd };
/** GUIX Canvas Buffer */
#if false
            #if (1 == 1) /* Inherit Frame Buffer Name from Graphics Screen 1 */
            uint8_t g_sf_el_gx0_canvas[sizeof(g_display0_fb_background[0])] BSP_ALIGN_VARIABLE_V2(4) BSP_PLACE_IN_SECTION_V2(".bss");
            #else /* Inherit Frame Buffer Name from Graphics Screen 2 */
            uint8_t g_sf_el_gx0_canvas[sizeof(g_display0_fb_foreground[0])] BSP_ALIGN_VARIABLE_V2(4) BSP_PLACE_IN_SECTION_V2(".bss");
            #endif
            #endif

/** JPEG Work Buffer */
#if GX_USE_SYNERGY_JPEG
            #if (8192)
            uint8_t g_sf_el_gx0_jpegbuffer[8192] BSP_ALIGN_VARIABLE_V2(64) BSP_PLACE_IN_SECTION_V2(".bss");
            #endif
            #endif

/** GUIX Port module control block instance */
static sf_el_gx_instance_ctrl_t g_sf_el_gx0_ctrl;

/** GUIX Port module configuration */
static const sf_el_gx_cfg_t g_sf_el_gx0_cfg = {
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
#if (1 > 1) /* Multiple frame buffers are used for Graphics Screen 1 */
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
                .jpegbuffer_size = 8192,
                .p_sf_jpeg_decode_instance = (void *)&g_sf_jpeg_decode0,
            #else
		.p_jpegbuffer = NULL, .jpegbuffer_size = 0, .p_sf_jpeg_decode_instance =
				NULL,
#endif

		/* D/AVE 2D Buffer Cache */
		.dave2d_buffer_cache_enabled = true };

/** GUIX Port module instance */
sf_el_gx_instance_t g_sf_el_gx0 = { .p_api = &sf_el_gx_on_guix, .p_ctrl =
		&g_sf_el_gx0_ctrl, .p_cfg = &g_sf_el_gx0_cfg };
static qspi_instance_ctrl_t g_qspi0_ctrl;
const qspi_cfg_t g_qspi0_cfg = { .p_extend = NULL, .addr_mode =
		QSPI_3BYTE_ADDR_MODE, };
/** This structure encompasses everything that is needed to use an instance of this interface. */
const qspi_instance_t g_qspi0 = { .p_ctrl = &g_qspi0_ctrl,
		.p_cfg = &g_qspi0_cfg, .p_api = &g_qspi_on_qspi, };
/* SF_MEMORY QSPI NOR g_sf_memory_qspi_nor0 */
const sf_memory_qspi_nor_cfg_t g_sf_memory_qspi_nor0_ext_cfg = { .p_qspi =
		&g_qspi0, .timeout_ticks = 30000, .p_delay_callback = NULL,
		.p_delay_callback_context = (void*) &g_sf_memory_qspi_nor0 };

/* SF_MEMORY QSPI NOR instance control */
sf_memory_qspi_nor_instance_ctrl_t g_sf_memory_qspi_nor0_ctrl;

/* SF_MEMORY QSPI config */
const sf_memory_cfg_t g_sf_memory_qspi_nor0_cfg = { .p_extend =
		&g_sf_memory_qspi_nor0_ext_cfg };

/* SF_MEMORY instance */
const sf_memory_instance_t g_sf_memory_qspi_nor0 = { .p_ctrl =
		&g_sf_memory_qspi_nor0_ctrl, .p_cfg = &g_sf_memory_qspi_nor0_cfg,
		.p_api = &g_sf_memory_on_sf_memory_qspi_nor };
#define FLASH_DEVICE_PHYSICAL_ADDRESS_OFFSET 0x60000000

/* sf_el_lx_nor driver instance control block */
static sf_el_lx_nor_instance_ctrl_t g_sf_el_lx_nor0_ctrl;

const sf_el_lx_nor_memory_settings_t g_sf_el_lx_nor0_memory_settings =
		{ .absolute_start_addr = 0x00000000
				+ FLASH_DEVICE_PHYSICAL_ADDRESS_OFFSET, .size = 0 * 1024 };

/* sf_el_lx_nor driver config block */
static sf_el_lx_nor_instance_cfg_t g_sf_el_lx_nor0_cfg = { .p_lower_lvl =
		&g_sf_memory_qspi_nor0, .p_callback = NULL, .p_memory_settings =
		&g_sf_el_lx_nor0_memory_settings };

/** Define LevelX NOR common initialization */
#define LX_NOR_COMMON_INITIALIZE (1)

/***********************************************************************************************************************
 * @brief     Initialization function that the user can choose to have called automatically during thread entry.
 *            The user can call this function at a later time if desired using the prototype below.
 *            - void lx_nor_common_init0(void)
 **********************************************************************************************************************/
void lx_nor_common_init0(void) {
	/** Initialize the LevelX NOR Flash system. */
	lx_nor_flash_initialize();
}
/** LevelX NOR Instance */
LX_NOR_FLASH g_lx_nor0;
#ifndef LX_DIRECT_READ
ULONG g_lx_nor0_ReadBuffer[SSP_LX_READ_BUFFER_SIZE_WORDS] = { 0 };
#endif

/** WEAK system error call back */
#if defined(__ICCARM__)
      #define g_lx_nor0_system_error_WEAK_ATTRIBUTE
      #pragma weak g_lx_nor0_system_error  = g_lx_nor0_system_error_internal
      #elif defined(__GNUC__)
      #define g_lx_nor0_system_error_WEAK_ATTRIBUTE   \
              __attribute__ ((weak, alias("g_lx_nor0_system_error_internal")))
      #endif

UINT g_lx_nor0_system_error(UINT error_code)
g_lx_nor0_system_error_WEAK_ATTRIBUTE;
/*****************************************************************************************************************//**
 * @brief      This is a weak example initialization error function.  It should be overridden by defining a user  function
 *             with the prototype below.
 *             - void g_lx_nor0_system_error(UINT error_code)
 *
 * @param[in]  error_code represents the error that occurred.
 **********************************************************************************************************************/
UINT g_lx_nor0_system_error_internal(UINT error_code);
UINT g_lx_nor0_system_error_internal(UINT error_code) {
	/** Suppress compiler warning for not using parameters. */
	SSP_PARAMETER_NOT_USED(error_code);

	/** An error has occurred. Please check function arguments for more information. */
	BSP_CFG_HANDLE_UNRECOVERABLE_ERROR(0);

	return LX_ERROR;
}

/** LevelX NOR instance "Read Sector" service */
static UINT g_lx_nor0_read(ULONG *flash_address, ULONG *destination,
		ULONG words) {
	ssp_err_t err;

	err = SF_EL_LX_NOR_Read(&g_sf_el_lx_nor0_ctrl, flash_address, destination,
			words);
	if (SSP_SUCCESS != err) {
		return (LX_ERROR);
	}
	return LX_SUCCESS;
}

/** LevelX NOR instance "Write Sector" service */
static UINT g_lx_nor0_write(ULONG *flash_address, ULONG *source, ULONG words) {
	ssp_err_t err;

	err = SF_EL_LX_NOR_Write(&g_sf_el_lx_nor0_ctrl, flash_address, source,
			words);
	if (SSP_SUCCESS != err) {
		return (LX_ERROR);
	}
	return LX_SUCCESS;
}

/** LevelX NOR instance "Block Erase" service */
static UINT g_lx_nor0_block_erase(ULONG block, ULONG block_erase_count) {
	ssp_err_t err;

	err = SF_EL_LX_NOR_BlockErase(&g_sf_el_lx_nor0_ctrl, block,
			block_erase_count);
	if (SSP_SUCCESS != err) {
		return (LX_ERROR);
	}
	return LX_SUCCESS;
}

/** LevelX NOR instance "Block Erased Verify" service */
static UINT g_lx_nor0_block_erased_verify(ULONG block) {
	ssp_err_t err;

	err = SF_EL_LX_NOR_BlockErasedVerify(&g_sf_el_lx_nor0_ctrl, block);
	if (SSP_SUCCESS != err) {
		return (LX_ERROR);
	}
	return LX_SUCCESS;
}

/** LevelX NOR instance "Driver Initialization" service */
static UINT g_lx_nor0_initialize(LX_NOR_FLASH *p_nor_flash) {
	ssp_err_t err;

	/** Make a copy of config structure as we can't change the constant one and update LevelX NOR Flash pointer */
	sf_el_lx_nor_instance_cfg_t g_sf_el_lx_nor0_cfg_copy = g_sf_el_lx_nor0_cfg;
	g_sf_el_lx_nor0_cfg_copy.p_lx_nor_flash = p_nor_flash;

	/** Open the sf_el_lx_nor driver */
	err = SF_EL_LX_NOR_Open(&g_sf_el_lx_nor0_ctrl, &g_sf_el_lx_nor0_cfg_copy);

	if (SSP_SUCCESS != err) {
		return (LX_ERROR);
	}

#ifndef LX_DIRECT_READ
	/** lx_nor_flash_sector_buffer is used only when LX_DIRECT_READ disabled */
	p_nor_flash->lx_nor_flash_sector_buffer = &g_lx_nor0_ReadBuffer[0];
#endif
	p_nor_flash->lx_nor_flash_driver_read = g_lx_nor0_read;
	p_nor_flash->lx_nor_flash_driver_write = g_lx_nor0_write;
	p_nor_flash->lx_nor_flash_driver_block_erase = g_lx_nor0_block_erase;
	p_nor_flash->lx_nor_flash_driver_block_erased_verify =
			g_lx_nor0_block_erased_verify;
	p_nor_flash->lx_nor_flash_driver_system_error = g_lx_nor0_system_error;
	return LX_SUCCESS;
}

/** LevelX NOR instance "Driver Close" service */
static ssp_err_t g_lx_nor0_close() {
	return SF_EL_LX_NOR_Close(&g_sf_el_lx_nor0_ctrl);
}
/* Block Media LevelX */
/* Block Media LevelX Config */
static const sf_block_media_on_lx_nor_cfg_t g_sf_block_media_lx_nor0_block_media_cfg =
		{ .nor_driver_initialize = g_lx_nor0_initialize, .p_nor_flash =
				&g_lx_nor0, .p_nor_flash_name = "g_lx_nor0", .close =
				&g_lx_nor0_close };

/* Block Media LevelX Instance Control */
static sf_block_media_lx_nor_instance_ctrl_t g_sf_block_media_lx_nor0_ctrl;

/* Block Media Config */
static sf_block_media_cfg_t g_sf_block_media_lx_nor0_cfg = { .block_size =
		SF_BLOCK_MEDIA_LX_NOR_BLOCK_SIZE_BYTES, .p_extend =
		&g_sf_block_media_lx_nor0_block_media_cfg };

/* Block Media Instance  */
sf_block_media_instance_t g_sf_block_media_lx_nor0 = { .p_ctrl =
		&g_sf_block_media_lx_nor0_ctrl, .p_cfg = &g_sf_block_media_lx_nor0_cfg,
		.p_api = &g_sf_block_media_on_sf_block_media_lx_nor };
#define g_sf_el_fx0_total_partition       0U

#if (g_sf_el_fx0_total_partition > 1U)
            sf_el_fx_media_partition_data_info_t g_sf_el_fx0_partition_data_info[g_sf_el_fx0_total_partition];
            #endif

sf_el_fx_instance_ctrl_t g_sf_el_fx0_ctrl;

/** SF_EL_FX interface configuration */
const sf_el_fx_config_t g_sf_el_fx0_config =
		{
#if (g_sf_el_fx0_total_partition > 1U) 
                .p_partition_data        = (sf_el_fx_media_partition_data_info_t *)g_sf_el_fx0_partition_data_info, 
            #else 
				.p_partition_data = NULL,
#endif 
				.p_lower_lvl_block_media = &g_sf_block_media_lx_nor0,
				.p_context = &g_sf_el_fx0_cfg, .p_extend = NULL,
				.total_partitions = g_sf_el_fx0_total_partition,
#if (g_sf_el_fx0_total_partition > 1U) 
                .p_callback              = NULL, 
            #else 
				.p_callback = NULL,
#endif 
		};

/* Instance structure to use this module. */
sf_el_fx_t g_sf_el_fx0_cfg = { .p_ctrl = &g_sf_el_fx0_ctrl, .p_config =
		&g_sf_el_fx0_config, };
NX_REC nx_record1;
static NX_CALLBACK_REC g_sf_el_nx_callbacks = {
		.nx_ether_unknown_packet_receive_callback = NULL,
		.nx_ether_mac_address_change_callback = NULL, };
static sf_el_nx_cfg_t sf_el_nx1_cfg = { .channel = 1, .nx_mac_address = {
		.nx_mac_address_h = SF_EL_NX_CFG_ENET1_MAC_H, .nx_mac_address_l =
				SF_EL_NX_CFG_ENET1_MAC_L }, .p_callback_rec =
		&g_sf_el_nx_callbacks, .etherc_ptr = R_ETHERC1, .edmac_ptr = R_EDMAC1, };
#if SF_EL_NX_CFG_IRQ_IPL != BSP_IRQ_DISABLED
            #if !defined(SSP_SUPPRESS_ISR_g_sf_el_nx) && !defined(SSP_SUPPRESS_ISR_EDMAC1)
            SSP_VECTOR_DEFINE_CHAN(edmac_eint_isr, EDMAC, EINT, 1);
            #endif
            #endif

void nx_ether_driver_eth1(NX_IP_DRIVER *driver_req_ptr) {
	nx_ether_driver(driver_req_ptr, &nx_record1, &sf_el_nx1_cfg);
}

/** Make user given name point to correct driver entry point. */
VOID (*g_sf_el_nx)(NX_IP_DRIVER *driver_req_ptr) = nx_ether_driver_eth1;
#define FX_COMMON_INITIALIZE (1)
/*******************************************************************************************************************//**
 * @brief     Initialization function that the user can choose to have called automatically during thread entry.
 *            The user can call this function at a later time if desired using the prototype below.
 *            - void fx_common_init0(void)
 **********************************************************************************************************************/
void fx_common_init0(void) {
	/** Initialize the FileX system. */
	fx_system_initialize();
}
#if defined(__ICCARM__)
            #define g_fx_media0_err_callback_WEAK_ATTRIBUTE
            #pragma weak g_fx_media0_err_callback  = g_fx_media0_err_callback_internal
            #elif defined(__GNUC__)
            #define g_fx_media0_err_callback_WEAK_ATTRIBUTE   __attribute__ ((weak, alias("g_fx_media0_err_callback_internal")))
            #endif
void g_fx_media0_err_callback(void *p_instance, void *p_data)
g_fx_media0_err_callback_WEAK_ATTRIBUTE;
#define SF_EL_FX_FORMAT_MEDIA_ENABLE_g_fx_media0 (0)
#define SF_EL_FX_FORMAT_FULL_MEDIA_g_fx_media0 (1)
#define SF_EL_FX_AUTO_INIT_g_fx_media0 (0)
ssp_err_t SF_EL_FX_Get_MEDIA_Info(sf_el_fx_ctrl_t *const p_api_ctrl,
		sf_el_fx_config_t const *const p_config, uint32_t *sector_size,
		uint32_t *sector_count);
FX_MEDIA g_fx_media0;
uint8_t g_media_memory_g_fx_media0[512];
/*******************************************************************************************************************//**
 * @brief      This is a weak example initialization error function.  It should be overridden by defining a user  function
 *             with the prototype below.
 *             - void g_fx_media0_err_callback(void * p_instance, void * p_data)
 *
 * @param[in]  p_instance arguments used to identify which instance caused the error and p_data Callback arguments used to identify what error caused the callback.
 **********************************************************************************************************************/
void g_fx_media0_err_callback_internal(void *p_instance, void *p_data);
void g_fx_media0_err_callback_internal(void *p_instance, void *p_data) {
	/** Suppress compiler warning for not using parameters. */
	SSP_PARAMETER_NOT_USED(p_instance);
	SSP_PARAMETER_NOT_USED(p_data);

	/** An error has occurred. Please check function arguments for more information. */
	BSP_CFG_HANDLE_UNRECOVERABLE_ERROR(0);
}

ssp_err_t fx_media_init0_format(void) {
	uint32_t fx_ret_val = FX_SUCCESS;

	uint32_t sector_size = 512;
	uint32_t sector_count = 3751936;

#if SF_EL_FX_FORMAT_FULL_MEDIA_g_fx_media0
	ssp_err_t error = SF_EL_FX_Get_MEDIA_Info(g_sf_el_fx0_cfg.p_ctrl,
			g_sf_el_fx0_cfg.p_config, &sector_size, &sector_count);

	if ((error != SSP_SUCCESS) || (sector_count <= 0)) {
		return SSP_ERR_MEDIA_FORMAT_FAILED;
	}

	sector_count -= 0;
#endif

	if (sizeof(g_media_memory_g_fx_media0) < 512) {
		return SSP_ERR_MEDIA_FORMAT_FAILED;
	}

	/* Format media.  */
#ifdef FX_ENABLE_EXFAT
                fx_ret_val = fx_media_exFAT_format(&g_fx_media0, // Pointer to FileX media control block.
                                             SF_EL_FX_BlockDriver, // Driver entry
                                             &g_sf_el_fx0_cfg, // Pointer to Block Media Driver
                                             g_media_memory_g_fx_media0, // Media buffer pointer
                                             sizeof(g_media_memory_g_fx_media0), // Media buffer size
                                             (CHAR *)"Volume 1", // Volume Name
                                             1, // Number of FATs
                                             0, // Hidden sectors
                                             sector_count, // Total sectors - Hidden Sectors
                                             sector_size, // Sector size
                                             1, // Sectors per cluster
                                             12345, // Volume Serial Number
                                             128); // Boundary unit
#else
	fx_ret_val = fx_media_format(&g_fx_media0, // Pointer to FileX media control block.
			SF_EL_FX_BlockDriver, // Driver entry
			&g_sf_el_fx0_cfg, // Pointer to Block Media Driver
			g_media_memory_g_fx_media0, // Media buffer pointer
			sizeof(g_media_memory_g_fx_media0), // Media buffer size
			(CHAR*) "Volume 1", // Volume Name
			1, // Number of FATs
			256, // Directory Entries
			0, // Hidden sectors
			sector_count, // Total sectors - Hidden Sectors
			sector_size, // Sector size
			1, // Sectors per cluster
			1, // Heads
			1); // Sectors per track
#endif
	if (FX_SUCCESS != fx_ret_val) {
		return SSP_ERR_MEDIA_FORMAT_FAILED;
	}

	return SSP_SUCCESS;
}

uint32_t fx_media_init0_open(void) {
	return fx_media_open(&g_fx_media0, (CHAR*) "g_fx_media0",
			SF_EL_FX_BlockDriver, &g_sf_el_fx0_cfg, g_media_memory_g_fx_media0,
			sizeof(g_media_memory_g_fx_media0));
}

/*******************************************************************************************************************//**
 * @brief     Initialization function that the user can choose to have called automatically during thread entry.
 *            The user can call this function at a later time if desired using the prototype below.
 *            - void fx_media_init0(void)
 **********************************************************************************************************************/
void fx_media_init0(void) {
#if SF_EL_FX_FORMAT_MEDIA_ENABLE_g_fx_media0

	ssp_err_t err_format = fx_media_init0_format();

	if (err_format != SSP_SUCCESS) {
		g_fx_media0_err_callback((void*) &g_fx_media0, &err_format);
	}

#endif

	uint32_t err_open = fx_media_init0_open();

	if (err_open != FX_SUCCESS) {
		g_fx_media0_err_callback((void*) &g_fx_media0, &err_open);
	}
}
/*******************************************************************************************************************//**
 * @brief     Initialization function that the user can choose to have called automatically during thread entry.
 *            The user can call this function at a later time if desired using the prototype below.
 *            - void nx_common_init0(void)
 **********************************************************************************************************************/
void nx_common_init0(void) {
	/** Initialize the NetX Duo system. */
	nx_system_initialize();
}
NX_PACKET_POOL g_packet_pool0;
uint8_t g_packet_pool0_pool_memory[(16 * (1568 + sizeof(NX_PACKET)))];
#if defined(__ICCARM__)
            #define g_packet_pool0_err_callback_WEAK_ATTRIBUTE
            #pragma weak g_packet_pool0_err_callback  = g_packet_pool0_err_callback_internal
            #elif defined(__GNUC__)
            #define g_packet_pool0_err_callback_WEAK_ATTRIBUTE   __attribute__ ((weak, alias("g_packet_pool0_err_callback_internal")))
            #endif
void g_packet_pool0_err_callback(void *p_instance, void *p_data)
g_packet_pool0_err_callback_WEAK_ATTRIBUTE;
/*******************************************************************************************************************//**
 * @brief      This is a weak example initialization error function.  It should be overridden by defining a user  function
 *             with the prototype below.
 *             - void g_packet_pool0_err_callback(void * p_instance, void * p_data)
 *
 * @param[in]  p_instance arguments used to identify which instance caused the error and p_data Callback arguments used to identify what error caused the callback.
 **********************************************************************************************************************/
void g_packet_pool0_err_callback_internal(void *p_instance, void *p_data);
void g_packet_pool0_err_callback_internal(void *p_instance, void *p_data) {
	/** Suppress compiler warning for not using parameters. */
	SSP_PARAMETER_NOT_USED(p_instance);
	SSP_PARAMETER_NOT_USED(p_data);

	/** An error has occurred. Please check function arguments for more information. */
	BSP_CFG_HANDLE_UNRECOVERABLE_ERROR(0);
}
/*******************************************************************************************************************//**
 * @brief     Initialization function that the user can choose to have called automatically during thread entry.
 *            The user can call this function at a later time if desired using the prototype below.
 *            - void packet_pool_init0(void)
 **********************************************************************************************************************/
void packet_pool_init0(void) {
	UINT g_packet_pool0_err;
	/* Create Client packet pool. */
	g_packet_pool0_err = nx_packet_pool_create(&g_packet_pool0,
			"g_packet_pool0 Packet Pool", 1568, &g_packet_pool0_pool_memory[0],
			(16 * (1568 + sizeof(NX_PACKET))));
	if (NX_SUCCESS != g_packet_pool0_err) {
		g_packet_pool0_err_callback((void*) &g_packet_pool0,
				&g_packet_pool0_err);
	}
}
NX_IP g_ip0;
#ifndef NX_DISABLE_IPV6
UINT g_ip0_interface_index = 0;
UINT g_ip0_address_index;
NXD_ADDRESS g_ip0_global_ipv6_address;
NXD_ADDRESS g_ip0_local_ipv6_address;
#endif            
uint8_t g_ip0_stack_memory[2048] BSP_PLACE_IN_SECTION_V2(".stack.g_ip0") BSP_ALIGN_VARIABLE_V2(BSP_STACK_ALIGNMENT);
#if 1 == 1                       // Check for ARP is enabled
#if (0 == 0)    // Check for ARP cache storage units is in bytes
#define    NX_ARP_CACHE_SIZE    (520)
#else
#define    NX_ARP_CACHE_SIZE    (520 * sizeof(NX_ARP))
#endif
uint8_t g_ip0_arp_cache_memory[NX_ARP_CACHE_SIZE] BSP_ALIGN_VARIABLE(4);
#endif
ULONG g_ip0_actual_status;

#ifndef NULL
#define NULL_DEFINE
void NULL(struct NX_IP_STRUCT *ip_ptr, UINT interface_index, UINT link_up);
#endif            
#if defined(__ICCARM__)
            #define g_ip0_err_callback_WEAK_ATTRIBUTE
            #pragma weak g_ip0_err_callback  = g_ip0_err_callback_internal
            #elif defined(__GNUC__)
            #define g_ip0_err_callback_WEAK_ATTRIBUTE   __attribute__ ((weak, alias("g_ip0_err_callback_internal")))
            #endif
void g_ip0_err_callback(void *p_instance, void *p_data)
g_ip0_err_callback_WEAK_ATTRIBUTE;
/*******************************************************************************************************************//**
 * @brief      This is a weak example initialization error function.  It should be overridden by defining a user  function
 *             with the prototype below.
 *             - void g_ip0_err_callback(void * p_instance, void * p_data)
 *
 * @param[in]  p_instance arguments used to identify which instance caused the error and p_data Callback arguments used to identify what error caused the callback.
 **********************************************************************************************************************/
void g_ip0_err_callback_internal(void *p_instance, void *p_data);
void g_ip0_err_callback_internal(void *p_instance, void *p_data) {
	/** Suppress compiler warning for not using parameters. */
	SSP_PARAMETER_NOT_USED(p_instance);
	SSP_PARAMETER_NOT_USED(p_data);

	/** An error has occurred. Please check function arguments for more information. */
	BSP_CFG_HANDLE_UNRECOVERABLE_ERROR(0);
}

/*******************************************************************************************************************//**
 * @brief     Initialization function that the user can choose to have called automatically during thread entry.
 *            The user can call this function at a later time if desired using the prototype below.
 *            - void ip_init0(void)
 **********************************************************************************************************************/
void ip_init0(void) {
	UINT g_ip0_err;
#ifndef NX_DISABLE_IPV6
	FILL_NXD_IPV6_ADDRESS(g_ip0_global_ipv6_address, 0x2001, 0x0, 0x0, 0x0, 0x0,
			0x0, 0x0, 0x1);
	FILL_NXD_IPV6_ADDRESS(g_ip0_local_ipv6_address, 0x0, 0x0, 0x0, 0x0, 0x0,
			0x0, 0x0, 0x0);

#endif
	/* Create an IP instance. */
	g_ip0_err = nx_ip_create(&g_ip0, "g_ip0 IP Instance",
			IP_ADDRESS(0, 0, 0, 0), IP_ADDRESS(0, 0, 0, 0), &g_packet_pool0,
			g_sf_el_nx, &g_ip0_stack_memory[0], 2048, 3);
	if (NX_SUCCESS != g_ip0_err) {
		g_ip0_err_callback((void*) &g_ip0, &g_ip0_err);
	}
#define SYNERGY_NOT_DEFINED     (0xFFFFFFFF)
#if (SYNERGY_NOT_DEFINED != 1)
	g_ip0_err = nx_arp_enable(&g_ip0, &g_ip0_arp_cache_memory[0],
			NX_ARP_CACHE_SIZE);
	if (NX_SUCCESS != g_ip0_err) {
		g_ip0_err_callback((void*) &g_ip0, &g_ip0_err);
	}
#endif
#if (SYNERGY_NOT_DEFINED != SYNERGY_NOT_DEFINED)
                g_ip0_err = nx_rarp_enable(&g_ip0);
                if (NX_SUCCESS != g_ip0_err)
                {
                    g_ip0_err_callback((void *)&g_ip0,&g_ip0_err);
                }
                #endif
#if (SYNERGY_NOT_DEFINED != 1)
	g_ip0_err = nx_tcp_enable(&g_ip0);
	if (NX_SUCCESS != g_ip0_err) {
		g_ip0_err_callback((void*) &g_ip0, &g_ip0_err);
	}
#endif
#if (SYNERGY_NOT_DEFINED != 1)
	g_ip0_err = nx_udp_enable(&g_ip0);
	if (NX_SUCCESS != g_ip0_err) {
		g_ip0_err_callback((void*) &g_ip0, &g_ip0_err);
	}
#endif
#if (SYNERGY_NOT_DEFINED != 1)
	g_ip0_err = nx_icmp_enable(&g_ip0);
	if (NX_SUCCESS != g_ip0_err) {
		g_ip0_err_callback((void*) &g_ip0, &g_ip0_err);
	}
#endif
#if (SYNERGY_NOT_DEFINED != 1)
	g_ip0_err = nx_igmp_enable(&g_ip0);
	if (NX_SUCCESS != g_ip0_err) {
		g_ip0_err_callback((void*) &g_ip0, &g_ip0_err);
	}
#endif
#if (SYNERGY_NOT_DEFINED != SYNERGY_NOT_DEFINED)
                g_ip0_err = nx_ip_fragment_enable(&g_ip0);
                if (NX_SUCCESS != g_ip0_err)
                {
                    g_ip0_err_callback((void *)&g_ip0,&g_ip0_err);
                }                        
                #endif            
#undef SYNERGY_NOT_DEFINED

#ifndef NX_DISABLE_IPV6
	/** Here's where IPv6 is enabled. */
	g_ip0_err = nxd_ipv6_enable(&g_ip0);
	if (NX_SUCCESS != g_ip0_err) {
		g_ip0_err_callback((void*) &g_ip0, &g_ip0_err);
	}
	g_ip0_err = nxd_icmp_enable(&g_ip0);
	if (NX_SUCCESS != g_ip0_err) {
		g_ip0_err_callback((void*) &g_ip0, &g_ip0_err);
	}
	/* Wait for link to be initialized so MAC address is valid. */
	/** Wait for init to finish. */
	g_ip0_err = nx_ip_interface_status_check(&g_ip0, 0, NX_IP_INITIALIZE_DONE,
			&g_ip0_actual_status, NX_WAIT_FOREVER);
	if (NX_SUCCESS != g_ip0_err) {
		g_ip0_err_callback((void*) &g_ip0, &g_ip0_err);
	}
	/** Setting link local address */
	if (0x0
			== (g_ip0_local_ipv6_address.nxd_ip_address.v6[0]
					| g_ip0_local_ipv6_address.nxd_ip_address.v6[1]
					| g_ip0_local_ipv6_address.nxd_ip_address.v6[2]
					| g_ip0_local_ipv6_address.nxd_ip_address.v6[3])) {
		g_ip0_err = nxd_ipv6_address_set(&g_ip0, g_ip0_interface_index, NX_NULL,
				10, NX_NULL);
	} else {
		g_ip0_err = nxd_ipv6_address_set(&g_ip0, g_ip0_interface_index,
				&g_ip0_local_ipv6_address, 10, &g_ip0_address_index);
	}
	if (NX_SUCCESS != g_ip0_err) {
		g_ip0_err_callback((void*) &g_ip0, &g_ip0_err);
	}
	if (0x0
			!= (g_ip0_global_ipv6_address.nxd_ip_address.v6[0]
					| g_ip0_global_ipv6_address.nxd_ip_address.v6[1]
					| g_ip0_global_ipv6_address.nxd_ip_address.v6[2]
					| g_ip0_global_ipv6_address.nxd_ip_address.v6[3])) {
		g_ip0_err = nxd_ipv6_address_set(&g_ip0, g_ip0_interface_index,
				&g_ip0_global_ipv6_address, 64, &g_ip0_address_index);
	}
	if (NX_SUCCESS != g_ip0_err) {
		g_ip0_err_callback((void*) &g_ip0, &g_ip0_err);
	}
#endif

#ifdef NULL_DEFINE
	g_ip0_err = nx_ip_link_status_change_notify_set(&g_ip0, NULL);
	if (NX_SUCCESS != g_ip0_err) {
		g_ip0_err_callback((void*) &g_ip0, &g_ip0_err);
	}
#endif

	/* Gateway IP Address */
#define IP_VALID(a,b,c,d)     (a|b|c|d)
#if IP_VALID(0,0,0,0)
                g_ip0_err = nx_ip_gateway_address_set(&g_ip0,
											     IP_ADDRESS(0,0,0,0));       
														   
                if (NX_SUCCESS != g_ip0_err)
                {
                    g_ip0_err_callback((void *)&g_ip0,&g_ip0_err);
                }                       
                #endif         
#undef IP_VALID

}
const cgc_instance_t g_cgc = { .p_api = &g_cgc_on_cgc, .p_cfg = NULL };
/* Instance structure to use this module. */
const fmi_instance_t g_fmi = { .p_api = &g_fmi_on_fmi };
const ioport_instance_t g_ioport =
		{ .p_api = &g_ioport_on_ioport, .p_cfg = NULL };
const elc_instance_t g_elc = { .p_api = &g_elc_on_elc, .p_cfg = NULL };
ssp_err_t SF_EL_FX_Get_MEDIA_Info(sf_el_fx_ctrl_t *const p_api_ctrl,
		sf_el_fx_config_t const *const p_config, uint32_t *sector_size,
		uint32_t *sector_count) {
	sf_el_fx_instance_ctrl_t *p_ctrl = (sf_el_fx_instance_ctrl_t*) p_api_ctrl;
	sf_block_media_instance_t *p_block_media =
			(sf_block_media_instance_t*) p_config->p_lower_lvl_block_media;
	ssp_err_t ret_val = SSP_SUCCESS;

	if (SF_EL_FX_PARTITION_GLOBAL_OPEN
			!= p_ctrl->media_info.global_open.status) {
		ret_val = p_block_media->p_api->open(p_block_media->p_ctrl,
				p_block_media->p_cfg);
		if (ret_val != SSP_SUCCESS) {
			return ret_val;
		}
	}

	/* Get actual sector size from media. */
	ret_val = p_block_media->p_api->ioctl(p_block_media->p_ctrl,
			SSP_COMMAND_GET_SECTOR_SIZE, sector_size);
	if (ret_val != SSP_SUCCESS) {
		return ret_val;
	}

	/* Get actual sector count from media. */
	ret_val = p_block_media->p_api->ioctl(p_block_media->p_ctrl,
			SSP_COMMAND_GET_SECTOR_COUNT, sector_count);
	if (ret_val != SSP_SUCCESS) {
		return ret_val;
	}

	if (SF_EL_FX_PARTITION_GLOBAL_OPEN
			!= p_ctrl->media_info.global_open.status) {
		/* Close driver.  */
		ret_val = p_block_media->p_api->close(p_block_media->p_ctrl);
	}

	return ret_val;
}
void g_common_init(void) {

	/** Call initialization function if user has selected to do so. */
#if LX_NOR_COMMON_INITIALIZE
	lx_nor_common_init0();
#endif
	/** Call initialization function if user has selected to do so. */
#if FX_COMMON_INITIALIZE
	fx_common_init0();
#endif
	/** Call initialization function if user has selected to do so. */
#if SF_EL_FX_AUTO_INIT_g_fx_media0
                fx_media_init0();
            #endif
	/** Call initialization function if user has selected to do so. */
#if (1)
	nx_common_init0();
#endif
	/** Call initialization function if user has selected to do so. */
#if (0)
                packet_pool_init0();
            #endif
	/** Call initialization function if user has selected to do so. */
#if (0)
                 ip_init0();
            #endif
}
