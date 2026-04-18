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
 * File Name    : sf_el_lx_nor.c
 * Description  : LevelX NOR Framework driver.
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "sf_el_lx_nor.h"
#include "sf_el_lx_nor_cfg.h"
/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
/** "LXNO" in ASCII, used to identify framework LevelX NOR handle*/
#define SF_EL_LX_NOR_OPEN (0x4C584E4FU)

#define SF_EL_LX_NOR_BYTES_PER_WORD (4U)
#define SF_EL_LX_NOR_ERASE_VERIFY_BUFFER_WORDS (8U)

#ifndef SF_EL_LX_NOR_ERROR_RETURN
/*LDRA_INSPECTED 77 S This macro does not work when surrounded by parentheses. */
#define SF_EL_LX_NOR_ERROR_RETURN(a, err) SSP_ERROR_RETURN((a), (err), &g_module_name[0], &g_sf_el_lx_nor_version)
#endif

/*******************************************************************************************************************//**
 * @addtogroup SF_EL_LX_NOR
 *
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Exported global variables (to be accessed by other files)
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/
static ssp_err_t sf_el_lx_nor_read_write_param_check(sf_el_lx_nor_instance_ctrl_t * p_ctrl,
                                                     ULONG                        * p_flash,
                                                     ULONG                        * p_dest);

static ssp_err_t sf_el_lx_nor_validate_memory_settings(sf_el_lx_nor_instance_ctrl_t * p_ctrl);

static ssp_err_t sf_el_lx_nor_validate_read_write_address(sf_el_lx_nor_instance_ctrl_t * p_ctrl,
                                                     ULONG                        * p_flash,
                                                     ULONG                        num_bytes);

static ssp_err_t sf_el_lx_nor_open_param_check(sf_el_lx_nor_instance_ctrl_t          * const p_ctrl,
                                               sf_el_lx_nor_instance_cfg_t     const * const p_cfg);

static ssp_err_t sf_el_lx_nor_block_erased_verify_param_check(sf_el_lx_nor_instance_ctrl_t * const p_ctrl);

/***********************************************************************************************************************
 * Global Variables
 **********************************************************************************************************************/
/** Name of module used by error logger macro */
#if BSP_CFG_ERROR_LOG != 0
static const char g_module_name[] = "sf_el_lx_nor";

#if defined(__GNUC__)
/* This structure is affected by warnings from a GCC compiler bug. This pragma suppresses the warnings in this
 * structure only.*/
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
/** Version data structure used by error logger macro. */
static const ssp_version_t g_sf_el_lx_nor_version =
{
    .api_version_minor  = SF_EL_LX_NOR_API_VERSION_MINOR,
    .api_version_major  = SF_EL_LX_NOR_API_VERSION_MAJOR,
    .code_version_major = SF_EL_LX_NOR_CODE_VERSION_MAJOR,
    .code_version_minor = SF_EL_LX_NOR_CODE_VERSION_MINOR
};
#if defined(__GNUC__)
/* Restore warning settings for 'missing-field-initializers' to as specified on command line. */
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic pop
#endif
#endif

/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief  Initializes LevelX NOR frame work read/write and control.
 *
 * Calls lower level driver initialization function.
 *
 * @param[in,out]  p_ctrl               Control block for the LevelX NOR framework instance.
 * @param[in,out]  p_cfg                LevelX NOR driver instance.
 *
 * @retval SSP_SUCCESS                  LevelX NOR driver is successfully opened.
 * @retval SSP_ERR_ASSERTION            p_ctrl or p_cfg is NULL.
 * @retval SSP_ERR_ALREADY_OPEN         Driver is already in OPEN state.
 * @retval SSP_ERR_INVALID_ARGUMENT     p_memory_settings structure configured to invalid values.
 * @return                              See @ref Common_Error_Codes or lower level drivers for other possible return
 *                                      codes. This function calls
 *                                        * sf_memory_api_t:open
 *                                        * sf_memory_api_t:infoGet
 *                                        * sf_memory_api_t:close
 **********************************************************************************************************************/
ssp_err_t SF_EL_LX_NOR_Open(sf_el_lx_nor_instance_ctrl_t          * const p_ctrl,
                            sf_el_lx_nor_instance_cfg_t     const * const p_cfg)
{
    ssp_err_t        ret_val     = SSP_SUCCESS;
    sf_memory_info_t memory_info = {0};

    /** Validate the parameters */
    ret_val = sf_el_lx_nor_open_param_check(p_ctrl, p_cfg);
    SF_EL_LX_NOR_ERROR_RETURN(SSP_SUCCESS == ret_val, ret_val);

    /** Check whether instance is already open */
    SF_EL_LX_NOR_ERROR_RETURN(SF_EL_LX_NOR_OPEN != p_ctrl->open, SSP_ERR_ALREADY_OPEN);

    /** Update the instance control block */
    p_ctrl->p_lower_lvl       = p_cfg->p_lower_lvl;
    p_ctrl->p_lx_nor_flash    = p_cfg->p_lx_nor_flash;
    p_ctrl->p_callback        = p_cfg->p_callback;
    p_ctrl->p_context         = p_cfg->p_context;
    p_ctrl->p_memory_settings = p_cfg->p_memory_settings;

    /** Open the underlying memory instance */
    ret_val = p_ctrl->p_lower_lvl->p_api->open(p_ctrl->p_lower_lvl->p_ctrl, p_ctrl->p_lower_lvl->p_cfg);
    SF_EL_LX_NOR_ERROR_RETURN(SSP_SUCCESS == ret_val, ret_val);

    /** Get the underlying NOR flash info */
    ret_val = p_ctrl->p_lower_lvl->p_api->infoGet(p_ctrl->p_lower_lvl->p_ctrl, &memory_info);

    /** If unable to get NOR flash info, close lower layer driver */
    if (SSP_SUCCESS != ret_val)
    {
         p_ctrl->p_lower_lvl->p_api->close(p_ctrl->p_lower_lvl->p_ctrl);
         return ret_val;
    }

    /** Update memory region info */
    p_ctrl->p_region_info = memory_info.p_regions_info;

    if ((0U < (uint32_t)p_ctrl->p_memory_settings->size) || (p_ctrl->p_memory_settings->absolute_start_addr > p_ctrl->p_region_info->memory_start_address))
    {
        ret_val = sf_el_lx_nor_validate_memory_settings(p_ctrl);

        if (SSP_SUCCESS != ret_val)
        {
             p_ctrl->p_lower_lvl->p_api->close(p_ctrl->p_lower_lvl->p_ctrl);
             return ret_val;
        }

        /** Setup the base address of the flash memory.  */
        p_ctrl->p_lx_nor_flash->lx_nor_flash_base_address = (ULONG *) p_ctrl->p_memory_settings->absolute_start_addr;

        /** Setup geometry of the flash.  */
        p_ctrl->p_lx_nor_flash->lx_nor_flash_total_blocks = ((p_ctrl->p_memory_settings->size) / (p_ctrl->p_region_info->minimum_erase_size));
    }

    else
    {
        /** Setup the base address of the flash memory.  */
        p_ctrl->p_lx_nor_flash->lx_nor_flash_base_address = (ULONG *) p_ctrl->p_region_info->memory_start_address;

        /** Setup geometry of the flash.  */
        p_ctrl->p_lx_nor_flash->lx_nor_flash_total_blocks = (((p_ctrl->p_region_info->memory_end_address + 1U) -
                                                              p_ctrl->p_region_info->memory_start_address) /
                                                              p_ctrl->p_region_info->minimum_erase_size);
    }

    p_ctrl->p_lx_nor_flash->lx_nor_flash_words_per_block = p_ctrl->p_region_info->minimum_erase_size /
                                                           SF_EL_LX_NOR_BYTES_PER_WORD;

    /** Mark control block open so subsequent calls know the device is open. */
    p_ctrl->open = SF_EL_LX_NOR_OPEN;

    return ret_val;
}

/*******************************************************************************************************************//**
 * @brief  LevelX NOR driver "read sector" service.
 *
 * This is responsible for reading a specific sector in a specific block of the NOR flash. All error checking and
 * correcting logic is the responsibility of the this service.
 *
 * @param[in]      p_ctrl               Control block for the LevelX NOR framework instance.
 * @param[in]      p_flash              Specifies the address of a logical sector within a NOR flash block of memory.
 * @param[in,out]  p_dest               Specifies where to place the sector contents.
 * @param[in]      word_count           Specifies how many 32-bit words to read.
 *
 * @retval SSP_SUCCESS                  LevelX NOR flash sector read successful.
 * @retval SSP_ERR_ASSERTION            p_ctrl, p_flash or p_dest is NULL.
 * @retval SSP_ERR_NOT_OPEN             Driver not in OPEN state for reading.
 * @retval SSP_ERR_INVALID_ARGUMENT     Requested range can't fit in the flash address range.
 * @return                              See @ref Common_Error_Codes or lower level drivers for other possible return
 *                                      codes. This function calls
 *                                        * sf_memory_api_t:read
 **********************************************************************************************************************/
ssp_err_t SF_EL_LX_NOR_Read(sf_el_lx_nor_instance_ctrl_t * const p_ctrl,
                            ULONG                        * const p_flash,
                            ULONG                        * const p_dest,
                            ULONG                                word_count)
{
    ssp_err_t              ret_val;
    sf_memory_instance_t * p_inst_ctrl = (sf_memory_instance_t *) p_ctrl->p_lower_lvl;

    /** Validate the parameters */
    ret_val = sf_el_lx_nor_read_write_param_check(p_ctrl, p_flash, p_dest);
    SF_EL_LX_NOR_ERROR_RETURN(SSP_SUCCESS == ret_val, ret_val);

    if ((0U < (uint32_t)p_ctrl->p_memory_settings->size) || (p_ctrl->p_memory_settings->absolute_start_addr > p_ctrl->p_region_info->memory_start_address))
    {
        ret_val = sf_el_lx_nor_validate_read_write_address(p_ctrl, p_flash, word_count);
        SF_EL_LX_NOR_ERROR_RETURN(SSP_SUCCESS == ret_val, ret_val);
    }

    /** Check whether the driver is in OPEN state */
    SF_EL_LX_NOR_ERROR_RETURN(SF_EL_LX_NOR_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);

    /** Read from underlying API */
    ret_val = p_inst_ctrl->p_api->read(p_inst_ctrl->p_ctrl,
                                       (uint8_t *) p_dest,
                                       (uint32_t) p_flash,
                                       word_count * SF_EL_LX_NOR_BYTES_PER_WORD);
    SF_EL_LX_NOR_ERROR_RETURN(SSP_SUCCESS == ret_val, ret_val);

    return ret_val;
}

/*******************************************************************************************************************//**
 * @brief  LevelX NOR driver "write sector" service.
 *
 * This is responsible for writing a specific sector into a block of the NOR flash. All error checking is the
 * responsibility of the this service.
 *
 * @param[in]      p_ctrl               Control block for the LevelX NOR framework instance.
 * @param[in,out]  p_flash              Specifies the address of a logical sector within a NOR flash block of memory.
 * @param[in]      p_src                Specifies the source of the write.
 * @param[in]      word_count           Specifies how many 32-bit words to write.
 *
 * @retval SSP_SUCCESS                  LevelX NOR flash sector write successful.
 * @retval SSP_ERR_ASSERTION            p_ctrl, p_flash or p_src is NULL.
 * @retval SSP_ERR_NOT_OPEN             Driver not in OPEN state for writing.
 * @retval SSP_ERR_INVALID_ARGUMENT     Requested range can't fit in the flash address range.
 * @return                              See @ref Common_Error_Codes or lower level drivers for other possible return
 *                                      codes. This function calls
 *                                        * sf_memory_api_t:write
 **********************************************************************************************************************/
ssp_err_t SF_EL_LX_NOR_Write(sf_el_lx_nor_instance_ctrl_t * const p_ctrl,
                             ULONG                        * const p_flash,
                             ULONG                        * const p_src,
                             ULONG                                word_count)
{
    ssp_err_t ret_val;

    /** Validate the parameters */
    ret_val = sf_el_lx_nor_read_write_param_check(p_ctrl, p_flash, p_src);
    SF_EL_LX_NOR_ERROR_RETURN(SSP_SUCCESS == ret_val, ret_val);

    if ((0U < (uint32_t)p_ctrl->p_memory_settings->size) || (p_ctrl->p_memory_settings->absolute_start_addr > p_ctrl->p_region_info->memory_start_address))
    {
        ret_val = sf_el_lx_nor_validate_read_write_address(p_ctrl, p_flash, word_count);
        SF_EL_LX_NOR_ERROR_RETURN(SSP_SUCCESS == ret_val, ret_val);
    }

    /** Check whether the driver is in OPEN state */
    SF_EL_LX_NOR_ERROR_RETURN(SF_EL_LX_NOR_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);

    /** Write to underlying API */
    ret_val = p_ctrl->p_lower_lvl->p_api->write(p_ctrl->p_lower_lvl->p_ctrl,
                                                (uint8_t *) p_src,
                                                (uint32_t) p_flash,
                                                word_count * SF_EL_LX_NOR_BYTES_PER_WORD);
    SF_EL_LX_NOR_ERROR_RETURN(SSP_SUCCESS == ret_val, ret_val);

    return ret_val;
}

/*******************************************************************************************************************//**
 * @brief  LevelX NOR driver "block erase" service.
 *
 * This is responsible for erasing the specified block of the NOR flash.
 *
 * @param[in]      p_ctrl               Control block for the LevelX NOR framework instance.
 * @param[in]      block                Specifies which NOR block to erase.
 * @param[in]      erase_count          Provided for diagnostic purposes(currently unused).
 *
 * @retval SSP_SUCCESS                  LevelX NOR flash block erase successful.
 * @retval SSP_ERR_ASSERTION            p_ctrl is NULL.
 * @retval SSP_ERR_NOT_OPEN             Driver not in OPEN state for erasing.
 * @return                              See @ref Common_Error_Codes or lower level drivers for other possible return
 *                                      codes. This function calls
 *                                        * sf_memory_api_t:erase
 **********************************************************************************************************************/
ssp_err_t SF_EL_LX_NOR_BlockErase(sf_el_lx_nor_instance_ctrl_t * const p_ctrl, ULONG block, ULONG erase_count)
{
    ssp_err_t              ret_val;
    sf_memory_instance_t * p_inst_ctrl;
    uint32_t               block_address;

    /** Validate the parameters */
#if SF_EL_LX_NOR_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(p_ctrl);
    SSP_ASSERT(p_ctrl->p_lower_lvl);
#endif
    /** If the driver is not open return an error. */
    SF_EL_LX_NOR_ERROR_RETURN(SF_EL_LX_NOR_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);

    p_inst_ctrl = (sf_memory_instance_t *) p_ctrl->p_lower_lvl;

    /** Calculate the block address */
    block_address = p_ctrl->p_region_info->memory_start_address +
                    ((uint32_t) block * p_ctrl->p_region_info->minimum_erase_size);

    if ((0U < (uint32_t)p_ctrl->p_memory_settings->size) || (p_ctrl->p_memory_settings->absolute_start_addr > p_ctrl->p_region_info->memory_start_address))
    {
        ret_val = sf_el_lx_nor_validate_read_write_address(p_ctrl, (ULONG *) block_address, erase_count);
        SF_EL_LX_NOR_ERROR_RETURN(SSP_SUCCESS == ret_val, ret_val);
    }

    /** Erase using underlying API */
    ret_val = p_inst_ctrl->p_api->erase(p_inst_ctrl->p_ctrl, block_address, p_ctrl->p_region_info->minimum_erase_size);
    SF_EL_LX_NOR_ERROR_RETURN(SSP_SUCCESS == ret_val, ret_val);

    /** Call the user function if available */
    if (p_ctrl->p_callback)
    {
        /** Prepare the callback arguments */
        sf_el_lx_nor_callback_args_t calback_args;
        calback_args.p_context          = p_ctrl->p_context;
        calback_args.erase_block_number = (uint32_t) block;
        calback_args.erase_block_count  = (uint32_t) erase_count;
        calback_args.event              = SF_EL_LX_NOR_EVENT_BLOCK_ERASE;

        /** Invoke callback function */
        p_ctrl->p_callback(&calback_args);
    }

    return ret_val;
}

/*******************************************************************************************************************//**
 * @brief  LevelX NOR driver "block erased verify" service.
 *
 * This is responsible for verifying the specified block of the NOR flash is erased.
 *
 * @param[in]      p_ctrl               Control block for the LevelX NOR framework instance.
 * @param[in]      block                Specifies which block to verify that it is erased.
 *
 * @retval SSP_SUCCESS                  LevelX flash block erase verification successful.
 * @retval SSP_ERR_ASSERTION            p_ctrl or lower level driver is NULL.
 * @retval SSP_ERR_NOT_OPEN             Driver not in OPEN state for verifying.
 * @retval SSP_ERR_NOT_ERASED           The block is not erased properly.
 * @return                              See @ref Common_Error_Codes or lower level drivers for other possible return
 *                                      codes. This function calls
 *                                        * sf_memory_api_t:read
 **********************************************************************************************************************/
ssp_err_t SF_EL_LX_NOR_BlockErasedVerify(sf_el_lx_nor_instance_ctrl_t * const p_ctrl, ULONG block)
{
    uint32_t *             read_address;
    uint32_t               words_to_verify;
    uint32_t               current_read_size                                   = 0U;
    uint32_t               read_buffer[SF_EL_LX_NOR_ERASE_VERIFY_BUFFER_WORDS] = {0};
    ssp_err_t              err;
    sf_memory_instance_t * p_inst_ctrl;

    /** Validate the parameters */
    err = sf_el_lx_nor_block_erased_verify_param_check(p_ctrl);
    SF_EL_LX_NOR_ERROR_RETURN(SSP_SUCCESS == err, err);

    /** Check whether the driver is in OPEN state */
    SF_EL_LX_NOR_ERROR_RETURN(SF_EL_LX_NOR_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);

    if ((0U < (uint32_t)p_ctrl->p_memory_settings->size) || (p_ctrl->p_memory_settings->absolute_start_addr > p_ctrl->p_region_info->memory_start_address))
    {
        read_address = (uint32_t *) p_ctrl->p_memory_settings->absolute_start_addr;
    }

    else
    {
        read_address    = (uint32_t *) (p_ctrl->p_region_info->memory_start_address +
                                     (block * p_ctrl->p_region_info->minimum_erase_size));
    }

    words_to_verify = p_ctrl->p_region_info->minimum_erase_size / SF_EL_LX_NOR_BYTES_PER_WORD;
    p_inst_ctrl     = (sf_memory_instance_t *) p_ctrl->p_lower_lvl;

    /** Loop to check if the block is erased.  */
    for (uint32_t offset = 0U; offset < words_to_verify; offset += current_read_size)
    {
        current_read_size = ((words_to_verify - offset) / SF_EL_LX_NOR_ERASE_VERIFY_BUFFER_WORDS) > 0
                                ? SF_EL_LX_NOR_ERASE_VERIFY_BUFFER_WORDS
                                : (words_to_verify % SF_EL_LX_NOR_ERASE_VERIFY_BUFFER_WORDS);

        err = p_inst_ctrl->p_api->read(p_inst_ctrl->p_ctrl,
                                       (uint8_t *) read_buffer,
                                       (uint32_t) read_address,
                                       current_read_size * SF_EL_LX_NOR_BYTES_PER_WORD);

        /** Check whether the driver read is success or not */
        SF_EL_LX_NOR_ERROR_RETURN(SSP_SUCCESS == err, err);

        /** Iterate over buffer and validate */
        for (uint32_t i = 0U; i < current_read_size; ++i)
        {
            /** Is this word erased?  */
            if (read_buffer[i] != 0xFFFFFFFFU)
            {
                return (SSP_ERR_NOT_ERASED);
            }
        }
        read_address += current_read_size;
    }
    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  LevelX NOR driver close service.
 *
 * This is responsible for closing the driver properly.
 *
 * @param[in]      p_ctrl               Control block for the LevelX NOR framework instance.
 *
 * @retval SSP_SUCCESS                  LevelX flash is available and is now open for read, write, and control access.
 * @retval SSP_ERR_ASSERTION            p_ctrl is NULL.
 * @retval SSP_ERR_NOT_OPEN             Driver not in OPEN state for closing.
 * @return                              See @ref Common_Error_Codes or lower level drivers for other possible return
 *                                      codes. This function calls
 *                                        * sf_memory_api_t:close
 **********************************************************************************************************************/
ssp_err_t SF_EL_LX_NOR_Close(sf_el_lx_nor_instance_ctrl_t * const p_ctrl)
{
    ssp_err_t ret_val;

    /** Validate the parameters */
#if SF_EL_LX_NOR_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(p_ctrl);
    SSP_ASSERT(p_ctrl->p_lower_lvl);
#endif
    /** Check whether the driver is in OPEN state */
    SF_EL_LX_NOR_ERROR_RETURN(SF_EL_LX_NOR_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);

    /** Close underlying API */
    ret_val = p_ctrl->p_lower_lvl->p_api->close(p_ctrl->p_lower_lvl->p_ctrl);

    /** Reset OPEN state */
    p_ctrl->open = 0U;
    return ret_val;
}

/*******************************************************************************************************************//**
 * @} (end addtogroup SF_EL_LX_NOR)
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * Validate the input parameter for read and write function calls.
 *
 * @param[in]      p_ctrl               Control block for the LevelX NOR framework instance.
 * @param[in]      p_flash              Flash device address.
 * @param[in]      p_mem                Buffer address.
 *
 * @retval SSP_SUCCESS                  All parameters are valid.
 * @retval SSP_ERR_INVALID_ADDRESS      p_flash is not a valid address to write to or read from the data.
 * @retval SSP_ERR_ASSERTION            p_ctrl / p_flash / p_mem is NULL.
 * @return                              Parameter validation status.
 **********************************************************************************************************************/
static ssp_err_t sf_el_lx_nor_read_write_param_check(sf_el_lx_nor_instance_ctrl_t * p_ctrl,
                                                     ULONG                        * p_flash,
                                                     ULONG                        * p_mem)
{
    /** Validate parameters */
#if SF_EL_LX_NOR_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(p_ctrl);
    SSP_ASSERT(p_ctrl->p_lower_lvl);
    SSP_ASSERT(p_flash);
    SSP_ASSERT(p_mem);
#else
    SSP_PARAMETER_NOT_USED(p_ctrl);
    SSP_PARAMETER_NOT_USED(p_flash);
    SSP_PARAMETER_NOT_USED(p_mem);
#endif

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * Validate the memory settings.
 *
 * @param[in]      p_ctrl               Control block for the LevelX NOR framework instance.
 *
 * @retval SSP_SUCCESS                  All parameters are valid.
 * @retval SSP_ERR_INVALID_ARGUMENT     p_memory_setting structure members -
 *                                      absolute_address and size are configured to invalid values.
 * @return                              Parameter validation status.
 **********************************************************************************************************************/

static ssp_err_t sf_el_lx_nor_validate_memory_settings(sf_el_lx_nor_instance_ctrl_t * p_ctrl)
{
    /** Validating whether the p_memory_settings structure is configured with correct and valid values.\ */
    if ((p_ctrl->p_memory_settings->absolute_start_addr > p_ctrl->p_region_info->memory_end_address )||
        (p_ctrl->p_memory_settings->absolute_start_addr < p_ctrl->p_region_info->memory_start_address) ||
        (((p_ctrl->p_memory_settings->absolute_start_addr + p_ctrl->p_memory_settings->size) - (1U)) > p_ctrl->p_region_info->memory_end_address) ||
        ((p_ctrl->p_memory_settings->size + p_ctrl->p_memory_settings->absolute_start_addr) == p_ctrl->p_memory_settings->absolute_start_addr))
    {
        return SSP_ERR_INVALID_ARGUMENT;
    }

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * Validate the write to and read from address.
 *
 * @param[in]      p_ctrl               Control block for the LevelX NOR framework instance.
 * @param[in]      p_flash              Flash device address.
 * @param[in]      num_words            Total number of 32-bit words to read, write and erase.
 *
 * @retval SSP_SUCCESS                  All parameters are valid.
 * @retval SSP_ERR_INVALID_ARGUMENT     p_flash is not a valid address.
 * @return                              Parameter validation status.
 **********************************************************************************************************************/
static ssp_err_t sf_el_lx_nor_validate_read_write_address(sf_el_lx_nor_instance_ctrl_t * p_ctrl,
                                                     ULONG                        * p_flash,
                                                     ULONG                        num_words)
{
    /** Validating the user provided address. The user provided address + number of bytes should not go beyond the end address. */
    if (0U < (uint32_t)p_ctrl->p_memory_settings->size)
    {
        if((p_flash > (ULONG *)(((p_ctrl->p_memory_settings->absolute_start_addr) + (p_ctrl->p_memory_settings->size)) - (1U))) ||
           (p_flash < (ULONG *)   p_ctrl->p_memory_settings->absolute_start_addr) ||
           ((ULONG *)((ULONG)p_flash + num_words) > (ULONG *) ((p_ctrl->p_memory_settings->absolute_start_addr) + (p_ctrl->p_memory_settings->size))))
        {
            return SSP_ERR_INVALID_ARGUMENT;
        }
    }

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Parameter check for Open API.
 * @param[in]     p_ctrl                 Pointer to LevelX NOR framework control structure
 * @param[in]     p_cfg                  Pointer to LevelX NOR framework configuration structure
 *
 * @retval        SSP_SUCCESS            Provided parameters not NULL.
 * @retval        SSP_ERR_ASSERTION      The parameter p_ctrl or p_cfg is NULL or
 *                                       the pointer to lower level framework is NULL or
 *                                       the pointer to NOR Flash instance is NULL.
 **********************************************************************************************************************/
static ssp_err_t sf_el_lx_nor_open_param_check(sf_el_lx_nor_instance_ctrl_t          * const p_ctrl,
                                               sf_el_lx_nor_instance_cfg_t     const * const p_cfg)
{
#if SF_EL_LX_NOR_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(p_ctrl);
    SSP_ASSERT(p_cfg);
    SSP_ASSERT(p_cfg->p_lower_lvl);
    SSP_ASSERT(p_cfg->p_lx_nor_flash);
#endif

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Parameter check for BlockErasedVerify.
 * @param[in]     p_ctrl                 Pointer to LevelX NOR framework control structure
 *
 * @retval        SSP_SUCCESS            Provided parameters not NULL.
 * @retval        SSP_ERR_ASSERTION      The parameter p_ctrl or p_cfg is NULL or
 *                                       the pointer to lower level framework is NULL.
 **********************************************************************************************************************/
static ssp_err_t sf_el_lx_nor_block_erased_verify_param_check(sf_el_lx_nor_instance_ctrl_t * const p_ctrl)
{
#if SF_EL_LX_NOR_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(p_ctrl);
    SSP_ASSERT(p_ctrl->p_lower_lvl);
#endif

    return SSP_SUCCESS;
}
