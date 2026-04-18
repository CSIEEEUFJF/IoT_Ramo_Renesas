/***********************************************************************************************************************
 * Copyright [2015-2025] Renesas Electronics Corporation and/or its licensors. All Rights Reserved.
 * 
 * This file is part of Renesas SynergyTM Software Package (SSP)
 *
 * The contents of this file (the "contents") are proprietary and confidential to Renesas Electronics Corporation
 * and/or its licensors ("Renesas") and subject to statutory and contractual protections.
 *
 * This file is subject to a Renesas SSP license agreement. Unless otherwise agreed in an SSP license agreement with
 * Renesas: 1) you may not use, copy, modify, distribute, display, or perform the contents; 2) you may not use any name
 * or mark of Renesas for advertising or publicity purposes or in connection with your use of the contents; 3) RENESAS
 * MAKES NO WARRANTY OR REPRESENTATIONS ABOUT THE SUITABILITY OF THE CONTENTS FOR ANY PURPOSE; THE CONTENTS ARE PROVIDED
 * "AS IS" WITHOUT ANY EXPRESS OR IMPLIED WARRANTY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, AND NON-INFRINGEMENT; AND 4) RENESAS SHALL NOT BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, OR
 * CONSEQUENTIAL DAMAGES, INCLUDING DAMAGES RESULTING FROM LOSS OF USE, DATA, OR PROJECTS, WHETHER IN AN ACTION OF
 * CONTRACT OR TORT, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THE CONTENTS. Third-party contents
 * included in this file may be subject to different terms.
 **********************************************************************************************************************/
/**********************************************************************************************************************
 * File Name    : sf_block_media_lx_nor.h
 * Description  : Block Media driver interface for LevelX NOR Framework header file.
 ***********************************************************************************************************************/

#ifndef SF_BLOCK_MEDIA_LX_NOR_H
#define SF_BLOCK_MEDIA_LX_NOR_H

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "bsp_api.h"
#include "sf_block_media_api.h"
#include "lx_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/*******************************************************************************************************************//**
 * @ingroup SF_Library
 * @defgroup SF_BLOCK_MEDIA_LX_NOR BLOCK_MEDIA_LEVELX_NOR
 * @brief RTOS-integrated Block Media framework for LEVELX driver.
 *
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
#define SF_BLOCK_MEDIA_LX_NOR_CODE_VERSION_MAJOR (2U)
#define SF_BLOCK_MEDIA_LX_NOR_CODE_VERSION_MINOR (0U)

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/

/** LevelX NOR block media config structure */
typedef struct st_block_media_on_lx_nor_cfg
{
    UINT          (* nor_driver_initialize)(LX_NOR_FLASH *);///< Pointer to the initialization function
    LX_NOR_FLASH   * p_nor_flash;                           ///< NOR Flash instance
    CHAR           * p_nor_flash_name;                      ///< NOR Flash instance name
    ssp_err_t     (* close)();                              ///< Pointer to underlying driver close
} sf_block_media_on_lx_nor_cfg_t;

/** LevelX NOR block media instance control block. */
typedef struct st_sf_block_media_lx_nor_instance_ctrl
{
    LX_NOR_FLASH * p_nor_flash;     ///< NOR Flash instance
    CHAR         * p_nor_flash_name;///< NOR Flash instance name
    uint32_t       block_size;      ///< Block size in bytes.
    uint32_t       open;            ///< Used to determine if framework is initialized.
    ssp_err_t   (* close)();           ///< Pointer to underlying driver close
} sf_block_media_lx_nor_instance_ctrl_t;

/**********************************************************************************************************************
Exported global variables
***********************************************************************************************************************/
/** @cond INC_HEADER_DEFS_SEC */
extern const sf_block_media_api_t g_sf_block_media_on_sf_block_media_lx_nor;
/** @endcond */

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif // SF_BLOCK_MEDIA_LX_NOR_H

/*******************************************************************************************************************//**
 * @} (end defgroup BLOCK_MEDIA_LEVELX_NOR)
 ***********************************************************************************************************************/
