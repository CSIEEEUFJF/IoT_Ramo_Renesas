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
 * File Name    : sf_el_lx_nor.h
 * Description  : SF_EL_LX_NOR
 ***********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @ingroup SF_Library
 * @defgroup SF_EL_LX_NOR EL_LX_NOR
 * @brief LevelX NOR driver implementation.
 *
 * @{
 ***********************************************************************************************************************/

#ifndef SF_EL_LX_NOR_API_H
#define SF_EL_LX_NOR_API_H

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/

/* SSP Related */
#include "bsp_api.h"
#include "sf_memory_api.h"
#include "lx_api.h"

/** Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/**********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
/** Version of the API defined in this file */
#define SF_EL_LX_NOR_API_VERSION_MAJOR (2U)
#define SF_EL_LX_NOR_API_VERSION_MINOR (0U)

/** Version of code that implements the API defined in this file */
#define SF_EL_LX_NOR_CODE_VERSION_MAJOR (2U)
#define SF_EL_LX_NOR_CODE_VERSION_MINOR (0U)

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/** Options for the callback events. */
typedef enum e_sf_el_lx_nor_event
{
    SF_EL_LX_NOR_EVENT_BLOCK_ERASE,   ///< Block erase event triggered
} sf_el_lx_nor_event_t;

/** SF_EL_LX_NOR memory settings for partition functionality. */
typedef struct st_sf_el_lx_nor_memory_settings
{
    uint32_t                absolute_start_addr;  ///< Starting address of memory partition.
    uint32_t                size;                 ///< Size of the partitioned region.
}sf_el_lx_nor_memory_settings_t;

/** SF_EL_LX_NOR callback arguments definitions  */
typedef struct st_sf_el_lx_nor_callback_args
{
    sf_el_lx_nor_event_t event;             ///< LevelX NOR driver callback event
    void const *         p_context;         ///< Placeholder for user data
    uint32_t             erase_block_number;///< Erase block number
    uint32_t             erase_block_count; ///< Erase count of specified block number
} sf_el_lx_nor_callback_args_t;


/** SF_EL_LX_NOR Control Block Type */
typedef struct st_sf_el_lx_nor_instance_ctrl
{
    sf_memory_instance_t              const * p_lower_lvl;        ///< Lower level memory pointer
    LX_NOR_FLASH                            * p_lx_nor_flash;     ///< Pointer to the LevelX nor flash instance.
    sf_memory_region_info_t                 * p_region_info;      ///< Memory region information
    void                              const * p_context;          ///< Placeholder for user data. Passed to the user callback.
    sf_el_lx_nor_memory_settings_t    const * p_memory_settings;  ///< Pointer to memory settings structure for partitioning functionality.
    void (*p_callback)(sf_el_lx_nor_callback_args_t * p_args);    ///< Callback function
    uint32_t open;                                                ///< Used to determine if framework is initialized.
} sf_el_lx_nor_instance_ctrl_t;

/** SF_EL_LX_NOR Config Block Type */
typedef struct st_sf_el_lx_nor_instance_cfg
{
    sf_memory_instance_t             const * p_lower_lvl;        ///< Lower level memory pointer
    LX_NOR_FLASH                           * p_lx_nor_flash;     ///< Pointer to the LevelX nor flash instance.
    void                             const * p_context;          ///< Placeholder for user data. Passed to the user callback.
    sf_el_lx_nor_memory_settings_t   const * p_memory_settings;  ///< Pointer to memory settings structure for partitioning functionality.
    void (*p_callback)(sf_el_lx_nor_callback_args_t * p_args);   ///< Callback function
} sf_el_lx_nor_instance_cfg_t;

/**********************************************************************************************************************
 * Public Functions
 **********************************************************************************************************************/
ssp_err_t SF_EL_LX_NOR_Open (sf_el_lx_nor_instance_ctrl_t       * const p_ctrl,
                             sf_el_lx_nor_instance_cfg_t  const * const p_cfg);

ssp_err_t SF_EL_LX_NOR_Read (sf_el_lx_nor_instance_ctrl_t       * const ctrl,
                             ULONG                              * const flash_address,
                             ULONG                              * const destination,
                             ULONG                                      words);

ssp_err_t SF_EL_LX_NOR_Write (sf_el_lx_nor_instance_ctrl_t       * const ctrl,
                              ULONG                              * const flash_address,
                              ULONG                              * const source,
                              ULONG                                      words);

ssp_err_t SF_EL_LX_NOR_BlockErase (sf_el_lx_nor_instance_ctrl_t       * const ctrl,
                                   ULONG                                      block,
                                   ULONG                                      erase_count);

ssp_err_t SF_EL_LX_NOR_BlockErasedVerify (sf_el_lx_nor_instance_ctrl_t       * const ctrl,
                                          ULONG                                      block);

ssp_err_t SF_EL_LX_NOR_Close (sf_el_lx_nor_instance_ctrl_t       * const p_ctrl);

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif // SF_EL_LX_NOR_API_H

/*******************************************************************************************************************//**
 * @} (end defgroup EL_LX_NOR)
 ***********************************************************************************************************************/
