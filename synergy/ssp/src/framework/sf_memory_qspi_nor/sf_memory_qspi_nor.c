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
 * File Name    : sf_memory_qspi_nor.c
 * Description  : QSPI NOR Memory framework.
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "sf_memory_qspi_nor.h"
#include "sf_memory_qspi_nor_cfg.h"
#include "sf_memory_qspi_nor_private_api.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#ifndef SF_MEMORY_QSPI_NOR_ERROR_RETURN
/*LDRA_INSPECTED 77 S This macro does not work when surrounded by parentheses. */
#define SF_MEMORY_QSPI_NOR_ERROR_RETURN(a, err) \
    SSP_ERROR_RETURN((a), (err), &g_module_name[0], &s_qspi_nor_flash_version)
#endif

#define SF_MEMORY_QSPI_NOR_MIN_WRITE (4U)
#define QSPI_FLASH_BASE_ADDRESS ( 0x60000000 )
#define QSPI_BANK_SIZE (64UL * 1024UL * 1024UL)
#ifndef SF_MEMORY_QSPI_NOR_SLEEP_TICKS
#define SF_MEMORY_QSPI_NOR_SLEEP_TICKS (1U)
#endif

/** "MQNO" in ASCII, used to identify framework Memory QSPI NOR handle*/
#define SF_MEMORY_QSPI_NOR_OPEN (0x4D514E4FU)

/* QSPI NOR limitation corrupts write when source and destination address is in same NOR device.
 * Circumvent the limitation by copying source data from QSPI to RAM buffer.*/
#define SF_MEMORY_QSPI_NOR_BUFFER_SIZE   (256UL)  /* Page size for QSPI W25Q64FV is 256 Bytes */
/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/
#if SF_MEMORY_QSPI_NOR_CFG_PARAM_CHECKING_ENABLE
static ssp_err_t sf_memory_qspi_nor_write_param_check (sf_memory_qspi_nor_instance_ctrl_t * const p_ctrl,
                                                       uint8_t                            * const p_src_address,
                                                       uint32_t                             const memory_address,
                                                       uint32_t                             const num_bytes);
#endif

static ssp_err_t sf_memory_qspi_nor_erase_handler (sf_memory_qspi_nor_instance_ctrl_t * const p_ctrl,
                                                   uint32_t                             const memory_address,
                                                   uint32_t                             const num_bytes,
                                                   uint32_t                             const erase_size);

static ssp_err_t sf_memory_qspi_nor_update_qspi (sf_memory_qspi_nor_instance_ctrl_t * const p_ctrl);

static ssp_err_t sf_memory_qspi_nor_wait_write_erase_complete (sf_memory_qspi_nor_instance_ctrl_t * const p_ctrl);

static uint32_t sf_memory_qspi_nor_max_erase_size (sf_memory_qspi_nor_instance_ctrl_t * const p_ctrl,
                                                   uint32_t                             const memory_address,
                                                   uint32_t                             const num_bytes);

static ssp_err_t sf_memory_qspi_nor_validate_address_range (sf_memory_qspi_nor_instance_ctrl_t * const p_ctrl,
                                                            uint32_t                             const memory_address,
                                                            uint32_t                             const num_bytes);

static ssp_err_t sf_memory_qspi_nor_validate_src_buffer (sf_memory_qspi_nor_instance_ctrl_t * const p_ctrl,
                                                         uint32_t                             const src_address,
                                                         uint32_t                             const num_bytes);

static ssp_err_t sf_memory_qspi_nor_rom_read(sf_memory_qspi_nor_instance_ctrl_t * const p_ctrl,
                                                uint8_t          * const p_dest_address,
                                                uint32_t           const memory_address,
                                                uint32_t           const num_bytes);

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/
#if defined(__GNUC__)
/* This structure is affected by warnings from the GCC compiler bug gcc.gnu.org/bugzilla/show_bug.cgi?id=60784
 * This pragma suppresses the warnings in this structure only, and will be removed when the SSP compiler is updated to
 * v5.3.*/
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
/** Version data structure used by error logger macro. */
static const ssp_version_t s_qspi_nor_flash_version =
{
    .api_version_minor  = SF_MEMORY_API_VERSION_MINOR,
    .api_version_major  = SF_MEMORY_API_VERSION_MAJOR,
    .code_version_major = SF_MEMORY_QSPI_NOR_CODE_VERSION_MAJOR,
    .code_version_minor = SF_MEMORY_QSPI_NOR_CODE_VERSION_MINOR
};
#if defined(__GNUC__)
/* Restore warning settings for 'missing-field-initializers' to as specified on command line. */
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic pop
#endif

/** Name of module used by error logger macro */
#if BSP_CFG_ERROR_LOG != 0
static const char g_module_name[] = "sf_memory_qspi_nor";
#endif

/*******************************************************************************************************************//**
 * @addtogroup SF_MEMORY_QSPI_NOR
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Global Variables
 **********************************************************************************************************************/
/*LDRA_INSPECTED 27 D This structure must be accessible in user code. It cannot be static. */
const sf_memory_api_t g_sf_memory_on_sf_memory_qspi_nor =
{
    .open       = SF_MEMORY_QSPI_NOR_Open,
    .read       = SF_MEMORY_QSPI_NOR_Read,
    .write      = SF_MEMORY_QSPI_NOR_Write,
    .flush      = SF_MEMORY_QSPI_NOR_Flush,
    .erase      = SF_MEMORY_QSPI_NOR_Erase,
    .infoGet    = SF_MEMORY_QSPI_NOR_InfoGet,
    .close      = SF_MEMORY_QSPI_NOR_Close,
    .versionGet = SF_MEMORY_QSPI_NOR_VersionGet
};

/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief  Open the SF Memory QSPI Nor driver module.
 *
 * Open the SF Memory QSPI Nor driver module for the purposes of reading and writing flash memory.
 *
 * @retval SSP_SUCCESS             Configuration was successful.
 * @retval SSP_ERR_ASSERTION       The parameter p_ctrl or p_cfg is NULL.
 * @retval SSP_ERR_ALREADY_OPEN    Driver is already open.
 * @return                         See @ref Common_Error_Codes or functions called by this function for other possible
 *                                 return codes. This function calls:
 *                                 * qspi_api_t::open
 *                                 * qspi_api_t::infoGet
 *                                 * qspi_api_t::close
 **********************************************************************************************************************/
ssp_err_t SF_MEMORY_QSPI_NOR_Open (sf_memory_ctrl_t * const p_ctrl, sf_memory_cfg_t const * const p_cfg)
{
    sf_memory_qspi_nor_instance_ctrl_t * p_inst_ctrl = (sf_memory_qspi_nor_instance_ctrl_t *) p_ctrl;
    sf_memory_qspi_nor_cfg_t const *     p_extend;
    ssp_err_t                            err;

    /** Validate the parameters */
#if SF_MEMORY_QSPI_NOR_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(p_inst_ctrl);
    SSP_ASSERT(p_cfg);
    SSP_ASSERT(p_cfg->p_extend);
    p_extend = (sf_memory_qspi_nor_cfg_t *) p_cfg->p_extend;
    SSP_ASSERT(p_extend->p_qspi);
#else
    p_extend = (sf_memory_qspi_nor_cfg_t *) p_cfg->p_extend;
#endif

    /** Check whether the framework is in already open state */
    SF_MEMORY_QSPI_NOR_ERROR_RETURN(SF_MEMORY_QSPI_NOR_OPEN != p_inst_ctrl->open, SSP_ERR_ALREADY_OPEN);

    /** Update instance control with the lower level driver and configuration information. */
    p_inst_ctrl->p_qspi = (qspi_instance_t *) p_extend->p_qspi;
    p_inst_ctrl->timeout_ticks = p_extend->timeout_ticks;
    p_inst_ctrl->p_delay_callback = p_extend->p_delay_callback;
    p_inst_ctrl->p_delay_callback_context = p_extend->p_delay_callback_context;

    /** Open the QSPI driver */
    err = p_inst_ctrl->p_qspi->p_api->open(p_inst_ctrl->p_qspi->p_ctrl, p_inst_ctrl->p_qspi->p_cfg);
    SF_MEMORY_QSPI_NOR_ERROR_RETURN(SSP_SUCCESS == err, err);

    /** Update the memory info */
    err = sf_memory_qspi_nor_update_qspi(p_inst_ctrl);

    /** Close QSPI in case of failure */
    if (SSP_SUCCESS != err)
    {
        p_inst_ctrl->p_qspi->p_api->close(p_inst_ctrl->p_qspi->p_ctrl);
        return err;
    }

    /** Mark instance as open for future reference */
    p_inst_ctrl->open = SF_MEMORY_QSPI_NOR_OPEN;

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Close the Memory QSPI NOR driver module.
 *
 * @retval SSP_SUCCESS             Close was successful.
 * @retval SSP_ERR_ASSERTION       p_ctrl is NULL.
 * @retval SSP_ERR_NOT_OPEN        Driver is not opened.
 * @return                         See @ref Common_Error_Codes or functions called by this function for other possible
 *                                 return codes. This function calls:
 *                                 * qspi_api_t::close
 **********************************************************************************************************************/
ssp_err_t SF_MEMORY_QSPI_NOR_Close (sf_memory_ctrl_t * const p_ctrl)
{
    sf_memory_qspi_nor_instance_ctrl_t * p_inst_ctrl = (sf_memory_qspi_nor_instance_ctrl_t *) p_ctrl;

    /** Validate the parameters */
#if SF_MEMORY_QSPI_NOR_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(p_inst_ctrl);
    SF_MEMORY_QSPI_NOR_ERROR_RETURN(SF_MEMORY_QSPI_NOR_OPEN == p_inst_ctrl->open, SSP_ERR_NOT_OPEN);
#endif

    /** Close the underlying QSPI */
    p_inst_ctrl->p_qspi->p_api->close(p_inst_ctrl->p_qspi->p_ctrl);

    /** Mark instance as closed for future reference */
    p_inst_ctrl->open = 0U;
    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Read data from the flash.
 *
 * Read specified number of bytes of data from a particular address on the QSPI flash device.
 *
 * @retval SSP_SUCCESS              The data read was successful.
 * @retval SSP_ERR_ASSERTION        p_ctrl,p_dest_address or memory_address is NULL.
 * @retval SSP_ERR_NOT_OPEN         Driver is not opened.
 * @retval SSP_ERR_INVALID_ARGUMENT Number of bytes requested are invalid.
 * @retval SSP_ERR_TIMEOUT          Wait timed out.
 * @return                         See @ref Common_Error_Codes or functions called by this function for other possible
 *                                 return codes. This function calls:
 *                                 * qspi_api_t::read
 *                                 * qspi_api_t::statusGet
 **********************************************************************************************************************/
ssp_err_t SF_MEMORY_QSPI_NOR_Read (sf_memory_ctrl_t * const p_ctrl,
                                   uint8_t          * const p_dest_address,
                                   uint32_t           const memory_address,
                                   uint32_t           const num_bytes)
{
    ssp_err_t err = SSP_SUCCESS;
    sf_memory_qspi_nor_instance_ctrl_t * p_inst_ctrl = (sf_memory_qspi_nor_instance_ctrl_t *) p_ctrl;

    /** Validate the parameters */
#if SF_MEMORY_QSPI_NOR_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(p_inst_ctrl);
    SSP_ASSERT(p_dest_address);
    SSP_ASSERT(memory_address);
    SF_MEMORY_QSPI_NOR_ERROR_RETURN(num_bytes != 0U, SSP_ERR_INVALID_ARGUMENT);
#endif

    /** Check whether instance is open or not */
    SF_MEMORY_QSPI_NOR_ERROR_RETURN(SF_MEMORY_QSPI_NOR_OPEN == p_inst_ctrl->open, SSP_ERR_NOT_OPEN);

    /** Check whether the device address is valid */
    err = sf_memory_qspi_nor_validate_address_range(p_inst_ctrl, memory_address, num_bytes);
    SF_MEMORY_QSPI_NOR_ERROR_RETURN(SSP_SUCCESS == err, err);

    /* Iteration to read total number of bytes from ROM area(within 64mb) till read bytes is 0 */
    /** Read data from banks using ROM area. */
    err = sf_memory_qspi_nor_rom_read(p_inst_ctrl, p_dest_address, memory_address, num_bytes);

    return err;
}

/*******************************************************************************************************************//**
 * @brief  Program data to the flash.
 *
 * @retval SSP_SUCCESS                  The flash was programmed successfully.
 * @retval SSP_ERR_ASSERTION            p_ctrl,p_src_address or memory_address is NULL.
 * @retval SSP_ERR_INVALID_ARGUMENT     Invalid parameter is passed.
 * @retval SSP_ERR_NOT_OPEN             Driver is not opened.
 * @retval SSP_ERR_TIMEOUT              Wait timed out.
 * @return                              See @ref Common_Error_Codes or functions called by this function for other
 *                                      possible return codes. This function calls:
 *                                         * qspi_api_t::pageProgram
 *                                         * qspi_api_t::statusGet
 **********************************************************************************************************************/
ssp_err_t SF_MEMORY_QSPI_NOR_Write (sf_memory_ctrl_t * const p_ctrl,
                                    uint8_t          * const p_src_address,
                                    uint32_t           const memory_address,
                                    uint32_t           const num_bytes)
{
    ssp_err_t                            err                = SSP_SUCCESS;
    uint32_t                             current_write_size = 0;
    sf_memory_qspi_nor_instance_ctrl_t * p_inst_ctrl        = (sf_memory_qspi_nor_instance_ctrl_t *) p_ctrl;
    uint8_t  * p_local_src_address;
    ssp_err_t qspi_nor_addr_valid = SSP_SUCCESS;
    uint8_t g_mem_qspi_nor0_ReadBuffer[SF_MEMORY_QSPI_NOR_BUFFER_SIZE] = { 0 };

    /** Validate the parameters */
#if SF_MEMORY_QSPI_NOR_CFG_PARAM_CHECKING_ENABLE
    err = sf_memory_qspi_nor_write_param_check(p_inst_ctrl, p_src_address, memory_address, num_bytes);
    SF_MEMORY_QSPI_NOR_ERROR_RETURN(SSP_SUCCESS == err, err);

#endif

    /** Check whether instance is open or not */
    SF_MEMORY_QSPI_NOR_ERROR_RETURN(SF_MEMORY_QSPI_NOR_OPEN == p_inst_ctrl->open, SSP_ERR_NOT_OPEN);

    /** Check whether the device address is valid */
    err = sf_memory_qspi_nor_validate_address_range(p_inst_ctrl, memory_address, num_bytes);
    SF_MEMORY_QSPI_NOR_ERROR_RETURN(SSP_SUCCESS == err, err);

    /* The QSPI hardware is unable to write data correctly when the source address lies within the QSPI NOR itself
     * This limitation is overcome by copying the source data into local buffer and passing its address as
     * parameter to low level page program write API */
    /** Check whether page size of QSPI NOR is larger than the read buffer size */
    qspi_nor_addr_valid = sf_memory_qspi_nor_validate_src_buffer(p_inst_ctrl, (uint32_t)p_src_address, num_bytes);
    SF_MEMORY_QSPI_NOR_ERROR_RETURN(SSP_ERR_OUT_OF_MEMORY != qspi_nor_addr_valid, SSP_ERR_OUT_OF_MEMORY);

    for (uint32_t offset = 0U; offset < num_bytes; offset += current_write_size)
    {
        /** Calculate current write size */
        current_write_size = p_inst_ctrl->qspi_info.min_program_size_bytes -
                             (((uint32_t) memory_address + offset) % p_inst_ctrl->qspi_info.min_program_size_bytes);

        if (current_write_size > (num_bytes - offset))
        {
            current_write_size = num_bytes - offset;
        }

        /* Check whether the source address refer to address space of QSPI NOR device */
        if (SSP_SUCCESS == qspi_nor_addr_valid)
        {
            p_local_src_address = g_mem_qspi_nor0_ReadBuffer;
            /** Copy data from source address to a buffer for write if the source address is within address range of QSPI NOR */
            memcpy(g_mem_qspi_nor0_ReadBuffer, (void*)((uint32_t)p_src_address + offset), current_write_size);
        }
        else
        {
            p_local_src_address = (uint8_t *)((uint32_t)p_src_address + offset);
        }

        /** Program using underlying QSPI driver */
        err = p_inst_ctrl->p_qspi->p_api->pageProgram(p_inst_ctrl->p_qspi->p_ctrl,
                                                      (uint8_t *) (memory_address + offset),
                                                      p_local_src_address,
                                                      current_write_size);
        SF_MEMORY_QSPI_NOR_ERROR_RETURN(SSP_SUCCESS == err, err);

        p_inst_ctrl->pending_operation = SF_MEMORY_QSPI_NOR_PENDING_WRITE;
        p_inst_ctrl->pending_operation_size = current_write_size;

        /** Wait if there is any write operation in progress */
        err = sf_memory_qspi_nor_wait_write_erase_complete(p_inst_ctrl);
        SF_MEMORY_QSPI_NOR_ERROR_RETURN(SSP_SUCCESS == err, err);
    }

    return err;
}

/*******************************************************************************************************************//**
 * @brief  Flush any pending data to the disk. This is not required for QSPI NOR Flash.
 *
 * @retval SSP_SUCCESS              NO error detected.
 * @retval SSP_ERR_ASSERTION        p_ctrl is NULL.
 * @retval SSP_ERR_NOT_OPEN         Driver is not opened.
 **********************************************************************************************************************/
ssp_err_t SF_MEMORY_QSPI_NOR_Flush (sf_memory_ctrl_t * const p_ctrl)
{
    sf_memory_qspi_nor_instance_ctrl_t * p_inst_ctrl = (sf_memory_qspi_nor_instance_ctrl_t *) p_ctrl;

    /** Validate the parameters */
#if SF_MEMORY_QSPI_NOR_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(p_inst_ctrl);
#endif

    /** Check whether instance is open or not */
    SF_MEMORY_QSPI_NOR_ERROR_RETURN(SF_MEMORY_QSPI_NOR_OPEN == p_inst_ctrl->open, SSP_ERR_NOT_OPEN);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Erase a number of bytes from the flash.
 *
 * @retval SSP_SUCCESS              The command to erase the flash was executed successfully.
 * @retval SSP_ERR_ASSERTION        p_ctrl or memory_address is NULL.
 * @retval SSP_ERR_INVALID_ARGUMENT Invalid num_bytes entered.
 * @retval SSP_ERR_NOT_OPEN         Driver is not opened.
 * @retval SSP_ERR_TIMEOUT          Wait timed out.
 * @return                          See @ref Common_Error_Codes or functions called by this function for other possible
 *                                  return codes.
 *                                  This function calls
 *                                       * qspi_api_t::erase
 *                                       * qspi_api_t::statusGet
 **********************************************************************************************************************/
ssp_err_t SF_MEMORY_QSPI_NOR_Erase (sf_memory_ctrl_t * p_ctrl,
                                    uint32_t const     memory_address,
                                    uint32_t const     num_bytes)
{
    sf_memory_qspi_nor_instance_ctrl_t * p_inst_ctrl = (sf_memory_qspi_nor_instance_ctrl_t *) p_ctrl;
    uint32_t                             erase_size  = 0U;
    ssp_err_t                            err         = SSP_SUCCESS;

    /** Validate the parameters */
#if SF_MEMORY_QSPI_NOR_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(p_inst_ctrl);
    SSP_ASSERT(memory_address);
#endif

    /** Check whether instance is open or not */
    SF_MEMORY_QSPI_NOR_ERROR_RETURN(SF_MEMORY_QSPI_NOR_OPEN == p_inst_ctrl->open, SSP_ERR_NOT_OPEN);

    /** Check whether the device address is valid */
    err = sf_memory_qspi_nor_validate_address_range(p_inst_ctrl, memory_address, num_bytes);
    SF_MEMORY_QSPI_NOR_ERROR_RETURN(SSP_SUCCESS == err, err);

    /** Calculate maximum erase size */
    erase_size = sf_memory_qspi_nor_max_erase_size(p_inst_ctrl, memory_address, num_bytes);

    /** Check whether we have valid erase size */
    SF_MEMORY_QSPI_NOR_ERROR_RETURN(0U != erase_size, SSP_ERR_INVALID_ARGUMENT);

    return sf_memory_qspi_nor_erase_handler(p_inst_ctrl, memory_address, num_bytes, erase_size);
}

/*******************************************************************************************************************//**
 * @brief  Returns the information about the flash.
 * @retval SSP_SUCCESS             Memory info structure updated successfully.
 * @retval SSP_ERR_ASSERTION       p_ctrl or p_info is NULL.
 * @retval SSP_ERR_NOT_OPEN        Driver is not opened.
 * @return                         InfoGet status.
 **********************************************************************************************************************/
ssp_err_t SF_MEMORY_QSPI_NOR_InfoGet (sf_memory_ctrl_t * const p_ctrl, sf_memory_info_t * const p_info)
{
    sf_memory_qspi_nor_instance_ctrl_t * p_inst_ctrl = (sf_memory_qspi_nor_instance_ctrl_t *) p_ctrl;

    /** Validate the parameters */
#if SF_MEMORY_QSPI_NOR_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(p_inst_ctrl);
    SSP_ASSERT(p_info);
#endif

    SF_MEMORY_QSPI_NOR_ERROR_RETURN(SF_MEMORY_QSPI_NOR_OPEN == p_inst_ctrl->open, SSP_ERR_NOT_OPEN);

    /** Memory info is obtained while opening, use it */
    p_info->number_of_regions = 1U;
    p_info->p_regions_info    = &p_inst_ctrl->region_info;

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief      Get the driver version based on compile time macros.
 *
 * @retval     SSP_SUCCESS          Successful close.
 * @retval     SSP_ERR_ASSERTION    p_version is NULL.
 * @return                          API and Code version.
 **********************************************************************************************************************/
ssp_err_t SF_MEMORY_QSPI_NOR_VersionGet (ssp_version_t * const p_version)
{
    /** Validate the parameters */
#if SF_MEMORY_QSPI_NOR_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(p_version);
#endif

    p_version->version_id = s_qspi_nor_flash_version.version_id;

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @} (end addtogroup SF_MEMORY_QSPI_NOR)
 **********************************************************************************************************************/

#if SF_MEMORY_QSPI_NOR_CFG_PARAM_CHECKING_ENABLE
/*******************************************************************************************************************//**
 * @brief   This is the parameter checking subroutine for the SF_MEMORY_QSPI_NOR_Write API.
 *
 * @param      p_ctrl          The memory QSPI instance control block
 * @param      p_src_address   The QSPI device address
 * @param[in]  memory_address  The buffer memory address
 * @param[in]  num_bytes       The byte count
 *
 * @retval  SSP_SUCCESS               No parameter error found
 * @retval  SSP_ERR_ASSERTION         p_ctrl, p_src_address or memory_address is NULL.
 * @retval  SSP_ERR_INVALID_ARGUMENT  Invalid num_bytes.
 * @return                            Parameter validation status.
 **********************************************************************************************************************/
static ssp_err_t sf_memory_qspi_nor_write_param_check (sf_memory_qspi_nor_instance_ctrl_t * const p_ctrl,
                                                       uint8_t                            * const p_src_address,
                                                       uint32_t                             const memory_address,
                                                       uint32_t                             const num_bytes)
{
    SSP_ASSERT(p_ctrl);
    SSP_ASSERT(p_src_address);
    SSP_ASSERT(memory_address);
    SF_MEMORY_QSPI_NOR_ERROR_RETURN((num_bytes % SF_MEMORY_QSPI_NOR_MIN_WRITE) == 0, SSP_ERR_INVALID_ARGUMENT);
    SF_MEMORY_QSPI_NOR_ERROR_RETURN(num_bytes != 0U, SSP_ERR_INVALID_ARGUMENT);
    SF_MEMORY_QSPI_NOR_ERROR_RETURN(((uint32_t) memory_address % SF_MEMORY_QSPI_NOR_MIN_WRITE) == 0,
                                SSP_ERR_INVALID_ARGUMENT);
    return SSP_SUCCESS;
}
#endif

/*******************************************************************************************************************//**
 * @brief   Update the QSPI info.
 *
 * @param      p_ctrl                 The memory QSPI instance control block
 *
 * @retval  SSP_SUCCESS               No parameter error found
 * @retval  SSP_ERR_ASSERTION         p_ctrl, p_qspi is NULL or Invalid QSPI info.
 * @return                            See @ref Common_Error_Codes or HAL driver for other possible return codes or causes.
 *                                    This function calls
 *                                    * qspi_api_t::infoGet
 **********************************************************************************************************************/
static ssp_err_t sf_memory_qspi_nor_update_qspi(sf_memory_qspi_nor_instance_ctrl_t * const p_ctrl)
{
    ssp_err_t err;
    /* Get the information from QSPI */
    err = p_ctrl->p_qspi->p_api->infoGet(p_ctrl->p_qspi->p_ctrl, &p_ctrl->qspi_info);

    /* Check whether received information is valid, at least one erase size should be there */
    if ((uint8_t) 0U == p_ctrl->qspi_info.num_erase_sizes)
    {
        err = SSP_ERR_ASSERTION;
    }
    SF_MEMORY_QSPI_NOR_ERROR_RETURN(SSP_SUCCESS == err, err);

    /* Update the control structure with memory information */
    p_ctrl->region_info.memory_start_address = BSP_PRV_QSPI_DEVICE_PHYSICAL_ADDRESS;
    p_ctrl->region_info.memory_end_address   = (BSP_PRV_QSPI_DEVICE_PHYSICAL_ADDRESS +
                                               p_ctrl->qspi_info.total_size_bytes) - 1U;
    p_ctrl->region_info.minimum_erase_size = p_ctrl->qspi_info.p_erase_sizes_bytes[0U];
    p_ctrl->region_info.minimum_write_size = SF_MEMORY_QSPI_NOR_MIN_WRITE;

    /* It might be possible erase sizes may not be in ascending order. Find out the minimum erase size */
    for (uint32_t i = 1U; i < p_ctrl->qspi_info.num_erase_sizes; i++)
    {
        uint32_t temp = p_ctrl->qspi_info.p_erase_sizes_bytes[i];
        if (temp < p_ctrl->region_info.minimum_erase_size)
        {
            p_ctrl->region_info.minimum_erase_size = temp;
        }
    }
    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief   QSPI erase handler.
 *
 * @param      p_ctrl               The memory QSPI instance control block
 * @param[in]  memory_address       The QSPI device address
 * @param[in]  num_bytes            The byte count
 * @param[in]  erase_size           The erase size
 *
 * @retval  SSP_SUCCESS               Erased successfully.
 * @retval  SSP_ERR_ASSERTION         p_ctrl, p_qspi is NULL.
 * @retval  SSP_ERR_TIMEOUT           Wait timed out.
 * @return                            See @ref Common_Error_Codes or HAL driver for other possible return codes or causes.
 *                                    This function calls
 *                                       * qspi_api_t::erase
 *                                       * qspi_api_t::statusGet
 **********************************************************************************************************************/
static ssp_err_t sf_memory_qspi_nor_erase_handler(sf_memory_qspi_nor_instance_ctrl_t * const p_ctrl,
                                                  uint32_t                             const memory_address,
                                                  uint32_t                             const num_bytes,
                                                  uint32_t                             const erase_size)
{
    ssp_err_t err;

    /** Iterate over number of erase cycles */
    for (uint32_t offset = 0U; offset < num_bytes; offset += erase_size)
    {
        err = p_ctrl->p_qspi->p_api->erase(p_ctrl->p_qspi->p_ctrl,
                                                (uint8_t *) (memory_address + offset),
                                                erase_size);
        SF_MEMORY_QSPI_NOR_ERROR_RETURN(SSP_SUCCESS == err, err);

        p_ctrl->pending_operation = SF_MEMORY_QSPI_NOR_PENDING_ERASE;
        p_ctrl->pending_operation_size = erase_size;

        err = sf_memory_qspi_nor_wait_write_erase_complete(p_ctrl);
        SF_MEMORY_QSPI_NOR_ERROR_RETURN(SSP_SUCCESS == err, err);
    }

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief      Wait until the current program or erase operation completes.
 * @param      p_ctrl                   The memory QSPI instance control block
 * @retval     SSP_SUCCESS              Successfully waited.
 * @retval     SSP_ERR_TIMEOUT          Wait timed out.
 * @return                              See @ref Common_Error_Codes or HAL driver for other possible return codes or
 *                                      causes.
 *                                      This function calls
 *                                          * qspi_api_t::statusGet
 **********************************************************************************************************************/
static ssp_err_t sf_memory_qspi_nor_wait_write_erase_complete(sf_memory_qspi_nor_instance_ctrl_t * const p_ctrl)
{
    bool write_in_progress = true;
    ssp_err_t err;
    sf_memory_qspi_nor_delay_callback_args_t args;

    /* Configure callback arguments. */
    args.pending_operation = p_ctrl->pending_operation;
    args.pending_operation_size = p_ctrl->pending_operation_size;
    args.timeout_remaining = p_ctrl->timeout_ticks;
    args.p_context = p_ctrl->p_delay_callback_context;
    args.times_called = 0U;

    /** While the qspi driver is busy sleep or call the delay callback if defined. If timeout return error. */
    do
    {
        err = p_ctrl->p_qspi->p_api->statusGet(p_ctrl->p_qspi->p_ctrl, &write_in_progress);
        SF_MEMORY_QSPI_NOR_ERROR_RETURN(SSP_SUCCESS == err, err);

        /* If write in progress call the user defined callback funtion. */
        if (write_in_progress)
        {
            if(NULL != p_ctrl->p_delay_callback)
            {
                p_ctrl->p_delay_callback(&args);
                args.times_called++;
            }
            else
            {
                tx_thread_sleep(SF_MEMORY_QSPI_NOR_SLEEP_TICKS);
                if(args.timeout_remaining < SF_MEMORY_QSPI_NOR_SLEEP_TICKS)
                {
                    args.timeout_remaining = 0U;
                }
                else
                {
                    args.timeout_remaining -= SF_MEMORY_QSPI_NOR_SLEEP_TICKS;
                }
            }
        }
    } while (write_in_progress && (args.timeout_remaining > 0U));

    /* Check whether write completed */
    SF_MEMORY_QSPI_NOR_ERROR_RETURN(!write_in_progress, SSP_ERR_TIMEOUT);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief      Calculate the maximum valid erase size.
 *
 * Erase size "0" means invalid
 *
 * @param      p_ctrl            The memory QSPI instance control block
 * @param[in]  memory_address  The QSPI device address
 * @param[in]  num_bytes        The byte count
 *
 * @return     maximum erase size.
 **********************************************************************************************************************/
static uint32_t sf_memory_qspi_nor_max_erase_size(sf_memory_qspi_nor_instance_ctrl_t * const p_ctrl,
                                                  uint32_t                             const memory_address,
                                                  uint32_t                             const num_bytes)
{
    uint32_t max_erase_size = 0U;
    for (uint32_t i = 0U; i < p_ctrl->qspi_info.num_erase_sizes; i++)
    {
        uint32_t temp = p_ctrl->qspi_info.p_erase_sizes_bytes[i];
        if (((memory_address % temp) == 0U) && ((num_bytes % temp) == 0U) && (temp > max_erase_size))
        {
            max_erase_size = temp;
        }
    }
    return max_erase_size;
}

/*******************************************************************************************************************//**
 * @brief      Validate the address range based on memory address and number of bytes.
 *
 * @param      p_ctrl          The instance control
 * @param[in]  memory_address  The memory address
 * @param[in]  num_bytes       The number bytes
 *
 * @retval     SSP_SUCCESS              The parameters are successfully validated.
 * @retval     SSP_ERR_INVALID_ARGUMENT The parameters are invalid.
 * @return     Parameter validation status.
 **********************************************************************************************************************/
static ssp_err_t sf_memory_qspi_nor_validate_address_range (sf_memory_qspi_nor_instance_ctrl_t * const p_ctrl,
                                                            uint32_t                             const memory_address,
                                                            uint32_t                             const num_bytes)
{
    /** Check whether the device address is valid */
    if ((memory_address < p_ctrl->region_info.memory_start_address) ||
        ((p_ctrl->region_info.memory_end_address + 1U) < (num_bytes + memory_address)))
    {
        return SSP_ERR_INVALID_ARGUMENT;
    }
    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief      Validate the source buffer memory.
 *
 * @param      p_ctrl          The instance control
 * @param[in]  src_address     The source address
 * @param[in]  num_bytes       The number bytes
 *
 * @retval     SSP_SUCCESS              The parameters are successfully validated.
 * @retval     SSP_ERR_OUT_OF_MEMORY    The program size byte larger than allocated buffer.
 * @return     Parameter validation status.
 **********************************************************************************************************************/
static ssp_err_t sf_memory_qspi_nor_validate_src_buffer (sf_memory_qspi_nor_instance_ctrl_t * const p_ctrl,
                                                               uint32_t                       const src_address,
                                                               uint32_t                       const num_bytes)
{
    ssp_err_t status = SSP_SUCCESS;
    /** Check whether the source address fall within valid memory range */
    status = sf_memory_qspi_nor_validate_address_range(p_ctrl, src_address, num_bytes);
    /** Check whether page size of QSPI NOR is larger than the read buffer size */
    if (SSP_SUCCESS == status)
    {
        SF_MEMORY_QSPI_NOR_ERROR_RETURN(SF_MEMORY_QSPI_NOR_BUFFER_SIZE >= p_ctrl->qspi_info.min_program_size_bytes, SSP_ERR_OUT_OF_MEMORY);
    }
    return status;
}

/*******************************************************************************************************************//**
 * @brief   This is the subroutine for the SF_BLOCK_MEDIA_QSPI_READ API to read data from banks using ROM area.
 *
 * @retval  SSP_SUCCESS      No parameter error found
 * @return                   See @ref Common_Error_Codes or HAL driver for other possible return codes or causes.
 *                           This function calls
 *                           * qspi_api_t::read
 **********************************************************************************************************************/
static ssp_err_t sf_memory_qspi_nor_rom_read(sf_memory_qspi_nor_instance_ctrl_t * const p_ctrl,
        uint8_t          * const p_dest,
        uint32_t           const memory_address,
        uint32_t           const num_bytes)
{
    ssp_err_t err = SSP_SUCCESS;
    uint32_t flash_address;
    uint32_t total_readable_bytes;
    uint32_t bank_number;
    uint32_t rom_area_readable_bytes;
    uint32_t rom_offset_address;
    uint8_t * p_rom_address;
    uint8_t * p_dest_address_buffer = p_dest;
    uint32_t read_bytes;

    flash_address = memory_address;
    /* Total number of bytes to be read */
    total_readable_bytes = num_bytes;
    while(total_readable_bytes > 0U)
    {
        /** Select the bank base on device address */
        bank_number = (flash_address - QSPI_FLASH_BASE_ADDRESS)/((uint32_t) QSPI_BANK_SIZE);

        err = p_ctrl->p_qspi->p_api->bankSelect(bank_number);
        SF_MEMORY_QSPI_NOR_ERROR_RETURN(SSP_SUCCESS == err, err);

        /** Identify accesses that cross boundaries */
        /* Calculate the address offset to calculate rom address and number of bytes to be read */
        rom_offset_address = (flash_address - QSPI_FLASH_BASE_ADDRESS) % ((uint32_t) QSPI_BANK_SIZE);
        /* Calculate the max readable byte length in the selected bank(rom area) */
        rom_area_readable_bytes = (uint32_t) QSPI_BANK_SIZE - rom_offset_address;
        /* Calculate the starting address of ROM to copy data */
        p_rom_address = (uint8_t *)(rom_offset_address + QSPI_FLASH_BASE_ADDRESS);
        /* Calculate the actual no. of bytes to be copied from offset start address */
        read_bytes = (rom_area_readable_bytes < total_readable_bytes) ? (rom_area_readable_bytes) : (total_readable_bytes);

        /** Read data from block media QSPI flash */
        err = p_ctrl->p_qspi->p_api->read(p_ctrl->p_qspi->p_ctrl,
                p_rom_address, p_dest_address_buffer, read_bytes);
        SF_MEMORY_QSPI_NOR_ERROR_RETURN(SSP_SUCCESS == err, err);

        /* Subtract the number of bytes already read */
        total_readable_bytes -= read_bytes;
        /* Increment the destination pointer to store read data */
        p_dest_address_buffer += read_bytes;
        /* Increment the source pointer address to read from location*/
        flash_address += read_bytes;
    }
    return err;
}

