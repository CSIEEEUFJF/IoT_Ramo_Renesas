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
 * File Name    : sf_block_media_lx_nor.c
 * Description  : Block Media driver interface for LevelX NOR Framework.
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "sf_block_media_lx_nor.h"
#include "sf_block_media_lx_nor_cfg.h"
#include "sf_block_media_lx_nor_private_api.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @addtogroup SF_BLOCK_MEDIA_LX_NOR
 *
 * @{
 **********************************************************************************************************************/

/** Macro for error logger. */
#ifndef SF_BLOCK_MEDIA_LX_NOR_ERROR_RETURN
/*LDRA_INSPECTED 77 S This macro does not work when surrounded by parentheses. */
#define SF_BLOCK_MEDIA_LX_NOR_ERROR_RETURN(a, err) \
    SSP_ERROR_RETURN((a), (err), &g_module_name[0], &g_block_media_lx_nor_version)
#endif

/** "BMLO" in ASCII, used to identify block media LevelX handle*/
#define SF_BLOCK_MEDIA_LX_NOR_OPEN (0x424D4C4FU)

#define SF_BLOCK_MEDIA_LX_NOR_BLOCK_SIZE_BYTES (512U)
/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Exported global variables (to be accessed by other files)
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/
static ssp_err_t sf_block_media_lx_nor_ctrl_param_check (sf_block_media_lx_nor_instance_ctrl_t * p_inst_ctrl,
                                                         void                                  * p_data);

/***********************************************************************************************************************
 * Global Variables
 **********************************************************************************************************************/

/** Block Media LevelX function pointers   */
/*LDRA_INSPECTED 27 D This structure must be accessible in user code. It cannot be static. */
const sf_block_media_api_t g_sf_block_media_on_sf_block_media_lx_nor =
{
    .open       = SF_BLOCK_MEDIA_LX_NOR_Open,
    .read       = SF_BLOCK_MEDIA_LX_NOR_Read,
    .write      = SF_BLOCK_MEDIA_LX_NOR_Write,
    .ioctl      = SF_BLOCK_MEDIA_LX_NOR_Control,
    .close      = SF_BLOCK_MEDIA_LX_NOR_Close,
    .versionGet = SF_BLOCK_MEDIA_LX_NOR_VersionGet
};

/** Name of module used by error logger macro */
#if BSP_CFG_ERROR_LOG != 0
static const char g_module_name[] = "sf_block_media_lx_nor";
#endif

#if defined(__GNUC__)
/* This structure is affected by warnings from a GCC compiler bug. This pragma suppresses the warnings in this
 * structure only.*/
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
/** Version data structure used by error logger macro. */
static const ssp_version_t g_block_media_lx_nor_version =
{
    .api_version_minor  = BLOCK_MEDIA_API_VERSION_MINOR,
    .api_version_major  = BLOCK_MEDIA_API_VERSION_MAJOR,
    .code_version_major = SF_BLOCK_MEDIA_LX_NOR_CODE_VERSION_MAJOR,
    .code_version_minor = SF_BLOCK_MEDIA_LX_NOR_CODE_VERSION_MINOR
};
#if defined(__GNUC__)
/* Restore warning settings for 'missing-field-initializers' to as specified on command line. */
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic pop
#endif



/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief  Open device for read/write and control.
 *
 * Open LevelX flash device for read/write and control. This function initializes
 * the LevelX driver and hardware the first time it is called out of reset.
 * The underlying flash needs to either be erased or already initialized with LevelX.
 *
 * @retval SSP_SUCCESS                  LevelX flash is available and is now open for read, write, and control access.
 * @retval SSP_ERR_ASSERTION            p_ctrl, p_cfg or an input pointer is NULL.
 * @retval SSP_ERR_MEDIA_OPEN_FAILED    LevelX NOR or the underlying flash failed to open. The underlying flash needs
 *                                      to either be erased or already initialized with LevelX.
 * @retval SSP_ERR_ALREADY_OPEN         The block media LevelX NOR instance has already been opened. No configurations
 *                                      were changed. Call the associated Close function or use associated Control
 *                                      commands to reconfigure the instance.
 * @return                              See @ref Common_Error_Codes or lower LevelX drivers for other possible return
 *                                      codes.
 *                                      This function calls:
 *                                      * lx_nor_flash_open
 **********************************************************************************************************************/
ssp_err_t SF_BLOCK_MEDIA_LX_NOR_Open(sf_block_media_ctrl_t      * const p_ctrl,
                                     sf_block_media_cfg_t const * const p_cfg)
{
    sf_block_media_lx_nor_instance_ctrl_t * p_inst_ctrl = (sf_block_media_lx_nor_instance_ctrl_t *) p_ctrl;
    sf_block_media_on_lx_nor_cfg_t  const * p_block_media_cfg;

    /** Validate the parameters */
#if (SF_BLOCK_MEDIA_LX_NOR_CFG_PARAM_CHECKING_ENABLE)
    SSP_ASSERT(NULL != p_inst_ctrl);
    SSP_ASSERT(NULL != p_cfg);
    p_block_media_cfg = (sf_block_media_on_lx_nor_cfg_t *) p_cfg->p_extend;
    SSP_ASSERT(NULL != p_block_media_cfg);
    SSP_ASSERT(NULL != p_block_media_cfg->p_nor_flash);
    SSP_ASSERT(NULL != p_block_media_cfg->p_nor_flash_name);
    SSP_ASSERT(NULL != p_block_media_cfg->close);
#else
    p_block_media_cfg = (sf_block_media_on_lx_nor_cfg_t *) p_cfg->p_extend;
#endif

    /** Check whether instance is already open */
    SF_BLOCK_MEDIA_LX_NOR_ERROR_RETURN(SF_BLOCK_MEDIA_LX_NOR_OPEN != p_inst_ctrl->open, SSP_ERR_ALREADY_OPEN);

    /** Update instance control block */
    p_inst_ctrl->p_nor_flash      = p_block_media_cfg->p_nor_flash;
    p_inst_ctrl->p_nor_flash_name = p_block_media_cfg->p_nor_flash_name;
    p_inst_ctrl->close            = p_block_media_cfg->close;

    /** Open underlying LevelX */
    UINT lx_err = lx_nor_flash_open(p_block_media_cfg->p_nor_flash,
                                    p_block_media_cfg->p_nor_flash_name,
                                    p_block_media_cfg->nor_driver_initialize);

    SF_BLOCK_MEDIA_LX_NOR_ERROR_RETURN(lx_err == (UINT) LX_SUCCESS, SSP_ERR_MEDIA_OPEN_FAILED);

    /** Mark control block open so subsequent calls know the device is open. */
    p_inst_ctrl->open = SF_BLOCK_MEDIA_LX_NOR_OPEN;
    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Read data from flash using LevelX.
 *
 * @retval  SSP_SUCCESS                  Data read successfully.
 * @retval  SSP_ERR_ASSERTION            p_ctrl or p_dest is NULL.
 * @retval  SSP_ERR_NOT_OPEN             The block media is not open.
 * @retval  SSP_ERR_READ_FAILED          Data read failed.
 * @return                               See @ref Common_Error_Codes or lower level drivers for other possible return
 *                                       codes. This function calls:
 *                                       * lx_nor_flash_sector_read
 *
 **********************************************************************************************************************/

ssp_err_t SF_BLOCK_MEDIA_LX_NOR_Read(sf_block_media_ctrl_t * const p_ctrl,
                                     uint8_t               * const p_dest,
                                     uint32_t                const start_sector,
                                     uint32_t                const sector_count)
{
    sf_block_media_lx_nor_instance_ctrl_t * p_inst_ctrl = (sf_block_media_lx_nor_instance_ctrl_t *) p_ctrl;
    UINT                                    ret_val     = LX_SUCCESS;

    /** Validate the parameters */
#if (SF_BLOCK_MEDIA_LX_NOR_CFG_PARAM_CHECKING_ENABLE)
    SSP_ASSERT(p_inst_ctrl);
    SSP_ASSERT(p_dest);
#endif

    /** Check whether the instance is in open state */
    SF_BLOCK_MEDIA_LX_NOR_ERROR_RETURN(SF_BLOCK_MEDIA_LX_NOR_OPEN == p_inst_ctrl->open, SSP_ERR_NOT_OPEN);

    /** Loop to read sectors from flash.  */
    for (uint32_t i = start_sector; i < (start_sector + sector_count); i++)
    {
        uint8_t * p_cur_dest = p_dest + ((i - start_sector) * SF_BLOCK_MEDIA_LX_NOR_BLOCK_SIZE_BYTES);
        /** Read a sector from NOR flash.  */
        ret_val = lx_nor_flash_sector_read(p_inst_ctrl->p_nor_flash,// NOR FLASH
                                           i,                       // Sector number
                                           p_cur_dest               // Destination
        );

        SF_BLOCK_MEDIA_LX_NOR_ERROR_RETURN(LX_SUCCESS == ret_val, SSP_ERR_READ_FAILED);
    }

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Write data to flash using LevelX.
 *
 * @retval  SSP_SUCCESS                  Write finished successfully.
 * @retval  SSP_ERR_ASSERTION            p_ctrl or p_src is NULL.
 * @retval  SSP_ERR_NOT_OPEN             The block media is not open.
 * @retval  SSP_ERR_WRITE_FAILED         Data write failed.
 * @return                               See @ref Common_Error_Codes or lower level drivers for other possible return
 *                                       codes. This function calls:
 *                                       * lx_nor_flash_sector_write
 **********************************************************************************************************************/

ssp_err_t SF_BLOCK_MEDIA_LX_NOR_Write(sf_block_media_ctrl_t * const p_ctrl,
                                          uint8_t     const * const p_src,
                                          uint32_t            const start_sector,
                                          uint32_t            const sector_count)
{
    sf_block_media_lx_nor_instance_ctrl_t * p_inst_ctrl = (sf_block_media_lx_nor_instance_ctrl_t *) p_ctrl;
    UINT                                    ret_val     = LX_SUCCESS;
    uint8_t                               * p_temp_buffer;

    /** Validate the parameters */
#if (SF_BLOCK_MEDIA_LX_NOR_CFG_PARAM_CHECKING_ENABLE)
    SSP_ASSERT(p_inst_ctrl);
    SSP_ASSERT(p_src);
#endif

    /** Check whether the instance is in open state */
    SF_BLOCK_MEDIA_LX_NOR_ERROR_RETURN(SF_BLOCK_MEDIA_LX_NOR_OPEN == p_inst_ctrl->open, SSP_ERR_NOT_OPEN);

    /** Loop to write sectors into flash.  */
    for (uint32_t i = start_sector; i < (start_sector + sector_count); i++)
    {
        p_temp_buffer = (uint8_t *) p_src + ((i - start_sector) * SF_BLOCK_MEDIA_LX_NOR_BLOCK_SIZE_BYTES);

        /** Write a sector into NOR flash.  */
        ret_val = lx_nor_flash_sector_write(p_inst_ctrl->p_nor_flash,// NOR FLASH
                                            i,                       // Sector number
                                            p_temp_buffer            // Source
        );

        SF_BLOCK_MEDIA_LX_NOR_ERROR_RETURN(LX_SUCCESS == ret_val, SSP_ERR_WRITE_FAILED);
    }

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Send control commands to Block Media LevelX NOR driver.
 *
 * @retval SSP_SUCCESS                      Command executed successfully.
 * @retval SSP_ERR_ASSERTION                p_ctrl or p_data is Null.
 * @retval SSP_ERR_NOT_OPEN                 The block media is not open.
 * @retval SSP_ERR_UNSUPPORTED              This module doesn't support requested command.
 * @retval SSP_ERR_SECTOR_RELEASE_FAILED    Sector release command failed.
 * @return                          See @ref Common_Error_Codes or lower level drivers for other possible return codes.
 *                                  This function calls:
 *                                  * lx_nor_flash_sector_release
 *
 **********************************************************************************************************************/
ssp_err_t SF_BLOCK_MEDIA_LX_NOR_Control(sf_block_media_ctrl_t * const p_ctrl,
                                        ssp_command_t           const command,
                                        void                  * p_data)
{
    sf_block_media_lx_nor_instance_ctrl_t * p_inst_ctrl = (sf_block_media_lx_nor_instance_ctrl_t *) p_ctrl;
    ssp_err_t                               ret_val     = SSP_SUCCESS;

    /** Validate the parameters */
    ret_val = sf_block_media_lx_nor_ctrl_param_check(p_inst_ctrl, p_data);
    SF_BLOCK_MEDIA_LX_NOR_ERROR_RETURN(SSP_SUCCESS == ret_val, ret_val);

    LX_NOR_FLASH * p_nor_flash = p_inst_ctrl->p_nor_flash;

    /** Check whether the instance is in open state */
    SF_BLOCK_MEDIA_LX_NOR_ERROR_RETURN(SF_BLOCK_MEDIA_LX_NOR_OPEN == p_inst_ctrl->open, SSP_ERR_NOT_OPEN);

    switch (command)
    {
        case SSP_COMMAND_GET_SECTOR_COUNT:
        {
            /** Get the sector count */
            *((uint32_t *) p_data) = p_nor_flash->lx_nor_flash_total_physical_sectors;
            break;
        }
        case SSP_COMMAND_GET_SECTOR_SIZE:
        {
            /** LevelX divides each NOR flash block into 512-byte logical sectors */
            *((uint32_t *) p_data) = SF_BLOCK_MEDIA_LX_NOR_BLOCK_SIZE_BYTES;
            break;
        }
        case SSP_COMMAND_GET_WRITE_PROTECTED:
        {
            /** It's not write protected */
            *((uint32_t *) p_data) = false;
            break;
        }
        case SSP_COMMAND_GET_SECTOR_RELEASE:
        {
            /** LevelX supports sector release */
            *((uint8_t *) p_data) = LX_TRUE;
            break;
        }
        case SSP_COMMAND_CTRL_SECTOR_RELEASE:
        {
            UINT release_status = LX_ERROR;
            /** Release NOR flash sector.  */
            release_status = lx_nor_flash_sector_release(p_nor_flash, *((ULONG *) p_data));

            ret_val = (LX_SUCCESS == release_status) ? SSP_SUCCESS : SSP_ERR_SECTOR_RELEASE_FAILED;
            break;
        }
        default:
            ret_val = SSP_ERR_UNSUPPORTED;
    }

    return ret_val;
}

/*******************************************************************************************************************//**
 * @brief      Close open Block Media LevelX NOR driver.
 *
 * Close an open Block Media LevelX NOR driver.
 *
 * @retval     SSP_SUCCESS              Successfully closed.
 * @retval     SSP_ERR_ASSERTION        p_ctrl or p_nor_flash is NULL.
 * @retval     SSP_ERR_NOT_OPEN         The block media is not open.
 * @return                              See @ref Common_Error_Codes or lower level drivers for other possible return
 *                                      codes. This function calls:
 *                                      * lx_nor_flash_close
 *                                      * SF_EL_LX_NOR_Close
 *
 **********************************************************************************************************************/
ssp_err_t SF_BLOCK_MEDIA_LX_NOR_Close(sf_block_media_ctrl_t * const p_ctrl)
{
    sf_block_media_lx_nor_instance_ctrl_t * p_inst_ctrl = (sf_block_media_lx_nor_instance_ctrl_t *) p_ctrl;

    /** Validate the parameters */
#if (SF_BLOCK_MEDIA_LX_NOR_CFG_PARAM_CHECKING_ENABLE)
    SSP_ASSERT(NULL != p_inst_ctrl);
    SSP_ASSERT(NULL != p_inst_ctrl->p_nor_flash);
#endif

    /** Check whether the instance is in open state */
    SF_BLOCK_MEDIA_LX_NOR_ERROR_RETURN(SF_BLOCK_MEDIA_LX_NOR_OPEN == p_inst_ctrl->open, SSP_ERR_NOT_OPEN);

    /** Close the LevelX NOR flash driver.  */
    lx_nor_flash_close(p_inst_ctrl->p_nor_flash);

    /** Close underlying NOR driver, if available */
    if (p_inst_ctrl->close)
    {
        p_inst_ctrl->close();
    }

    /** Mark control block close so subsequent calls know the device is close. */
    p_inst_ctrl->open = 0U;

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Get version of Block Media LevelX driver.
 *
 * Return the version of the firmware and API.
 *
 * @retval SSP_ERR_ASSERTION        p_version is Pointer.
 * @retval SSP_SUCCESS              version read successfully.
 * @return                          See @ref Common_Error_Codes or lower level drivers for other possible return codes.
 *
 * @note This function is reentrant.
 *
 **********************************************************************************************************************/

ssp_err_t SF_BLOCK_MEDIA_LX_NOR_VersionGet(ssp_version_t * const p_version)
{
    /** Validate the parameters */
#if (SF_BLOCK_MEDIA_LX_NOR_CFG_PARAM_CHECKING_ENABLE)
    SSP_ASSERT(NULL != p_version);
#endif

    p_version->version_id = g_block_media_lx_nor_version.version_id;
    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @} (end addtogroup SF_BLOCK_MEDIA_LX_NOR)
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief      Validate the input parameters.
 *
 * @param      p_inst_ctrl     The instance control
 * @param      p_data          The data
 *
 * @retval     SSP_ERR_ASSERTION  One of the input parameter are NULL
 * @return                        Parameter validation status.
 **********************************************************************************************************************/
static ssp_err_t sf_block_media_lx_nor_ctrl_param_check (sf_block_media_lx_nor_instance_ctrl_t * p_inst_ctrl,
                                                         void                                  * p_data)
{
    /** Validate the parameters */
#if (SF_BLOCK_MEDIA_LX_NOR_CFG_PARAM_CHECKING_ENABLE)
    SSP_ASSERT(NULL != p_inst_ctrl);
    SSP_ASSERT(NULL != p_data);
    SSP_ASSERT(NULL != p_inst_ctrl->p_nor_flash);
#endif
    return SSP_SUCCESS;
}

