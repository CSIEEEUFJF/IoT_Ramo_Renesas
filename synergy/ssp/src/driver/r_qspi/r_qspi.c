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
 * File Name    : r_qspi.c
 * Description  : QSPI HAL API
 **********************************************************************************************************************/

#include "bsp_api.h"

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "r_qspi.h"
#include "r_qspi_private.h"
#include "r_qspi_private_api.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
/** "RQSP" in ASCII, used to determine if channel is open. */
#define RQSPI_OPEN               (0x52515350ULL)

#ifndef QSPI_ERROR_RETURN
/*LDRA_INSPECTED 77 S This macro does not work when surrounded by parentheses. */
#define QSPI_ERROR_RETURN(a, err) SSP_ERROR_RETURN((a), (err), &g_module_name[0], &s_qspi_version)
#endif

#ifndef QSPI_COMMAND_PAGE_PROGRAM_DUAL
/* Default to extended SPI page program command. */
#define QSPI_COMMAND_PAGE_PROGRAM_DUAL  (QSPI_COMMAND_PAGE_PROGRAM)
#endif

#ifndef QSPI_COMMAND_PAGE_PROGRAM_QUAD
/* Default to extended SPI page program command. */
#define QSPI_COMMAND_PAGE_PROGRAM_QUAD  (QSPI_COMMAND_PAGE_PROGRAM)
#endif

#ifndef QSPI_COMMAND_4BYTE_PAGE_PROGRAM_DUAL
/* Default to extended SPI 4-byte page program command. */
#define QSPI_COMMAND_4BYTE_PAGE_PROGRAM_DUAL  (QSPI_COMMAND_4BYTE_PAGE_PROGRAM)
#endif

#ifndef QSPI_COMMAND_4BYTE_PAGE_PROGRAM_QUAD
/* Default to extended SPI 4-byte page program command. */
#define QSPI_COMMAND_4BYTE_PAGE_PROGRAM_QUAD  (QSPI_COMMAND_4BYTE_PAGE_PROGRAM)
#endif

/* Access address check beyond 16MB. */
#define QSPI_ADDR_RANGE_3_BYTES       (0x61000000)

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
/* Number of address bytes in 4 byte address mode. */
#define QSPI_4_BYTE_ADDRESS     (4U)

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/
static void qspi_d0_byte_write_quad_mode (R_QSPI_Type * p_qspi_reg, uint8_t byte);

static void qspi_d0_byte_write_dual_mode (R_QSPI_Type * p_qspi_reg, uint8_t byte);

static void qspi_d0_byte_write_standard (R_QSPI_Type * p_qspi_reg, uint8_t byte);

static ssp_err_t qspi_program_param_check (qspi_instance_ctrl_t * p_ctrl,
                                           uint8_t              * p_device_address,
                                           uint8_t              * p_memory_address,
                                           uint32_t               byte_count);

static ssp_err_t qspi_validate_address_range (qspi_instance_ctrl_t * p_ctrl,
                                              uint8_t              * p_device_address,
                                              uint32_t               byte_count);

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
static const ssp_version_t s_qspi_version =
{
    .api_version_minor  = QSPI_API_VERSION_MINOR,
    .api_version_major  = QSPI_API_VERSION_MAJOR,
    .code_version_major = QSPI_CODE_VERSION_MAJOR,
    .code_version_minor = QSPI_CODE_VERSION_MINOR
};
#if defined(__GNUC__)
/* Restore warning settings for 'missing-field-initializers' to as specified on command line. */
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic pop
#endif

/** Name of module used by error logger macro */
#if BSP_CFG_ERROR_LOG != 0
static const char          g_module_name[] = "qspi";
#endif

static R_QSPI_Type * gp_qspi_reg = NULL;

/* Page program command.  Index by [data_lines][num_address_bytes >> 2].  data_lines is 0 for 1 data line,
 * 1 for 2 data lines, or 2 for 4 data lines.  Address mode is always 3 or 4, so num_address_bytes >> 2 maps 3 byte
 * address to index 0 and 4 byte address to index 1. */
static const uint8_t g_qspi_prv_program_command[3][2] =
{
    {QSPI_COMMAND_PAGE_PROGRAM,        QSPI_COMMAND_4BYTE_PAGE_PROGRAM},
    {QSPI_COMMAND_PAGE_PROGRAM_DUAL,   QSPI_COMMAND_4BYTE_PAGE_PROGRAM_DUAL},
    {QSPI_COMMAND_PAGE_PROGRAM_QUAD,   QSPI_COMMAND_4BYTE_PAGE_PROGRAM_QUAD}
};

/* Page program command.  Index by [data_lines].  data_lines is 0 for 1 data line, 1 for 2 data lines, or
 * 2 for 4 data lines. */
static void (* const gp_qspi_prv_byte_write[3]) (R_QSPI_Type * p_qspi_reg, uint8_t byte)  =
{
    qspi_d0_byte_write_standard,
    qspi_d0_byte_write_dual_mode,
    qspi_d0_byte_write_quad_mode
};

/*******************************************************************************************************************//**
 * @addtogroup QSPI
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Global Variables
 **********************************************************************************************************************/
/*LDRA_INSPECTED 27 D This structure must be accessible in user code. It cannot be static. */
const qspi_api_t g_qspi_on_qspi =
{
    .open           = R_QSPI_Open,
    .close          = R_QSPI_Close,
    .read           = R_QSPI_Read,
    .pageProgram    = R_QSPI_PageProgram,
    .erase          = R_QSPI_Erase,
    .infoGet        = R_QSPI_InfoGet,
    .sectorErase    = R_QSPI_SectorErase,
    .statusGet      = R_QSPI_StatusGet,
    .bankSelect     = R_QSPI_BankSelect,
    .versionGet     = R_QSPI_VersionGet
};

/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief  Open the QSPI driver module.
 *
 * Open the QSPI module driver in direct communication mode for the purposes of reading and writing flash memory via SPI
 * protocols.
 *
 * @retval SSP_SUCCESS             Configuration was successful.
 * @retval SSP_ERR_ASSERTION       The parameter p_ctrl or p_cfg is NULL.
 * @retval SSP_ERR_UNSUPPORTED     Driver not able to query the following information from the flash
 *                                 manufacturer id,memory capacity and memory type.
 * @retval SSP_ERR_IN_USE          QSPI resource is locked.
 * @return                         See @ref Common_Error_Codes or functions called by this function for other possible
 *                                 return codes. This function calls:
 *                                 * fmi_api_t::productFeatureGet
 **********************************************************************************************************************/
ssp_err_t R_QSPI_Open (qspi_ctrl_t * p_api_ctrl, qspi_cfg_t const * const p_cfg)
{
    qspi_instance_ctrl_t * p_ctrl = (qspi_instance_ctrl_t *) p_api_ctrl;

#if QSPI_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(p_ctrl);
    SSP_ASSERT(p_cfg);
#else
    /* Configuration structure not used in this function. */
    SSP_PARAMETER_NOT_USED (p_cfg);
#endif

    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};
    ssp_feature.channel = 0U;
    ssp_feature.unit = 0U;
    ssp_feature.id = SSP_IP_QSPI;
    fmi_feature_info_t info = {0U};
    ssp_err_t err = g_fmi_on_fmi.productFeatureGet(&ssp_feature, &info);
    QSPI_ERROR_RETURN(SSP_SUCCESS == err, err);
    p_ctrl->p_reg = (R_QSPI_Type *) info.ptr;
    gp_qspi_reg = p_ctrl->p_reg;

    /* Attempt to reserve the lock if available */
    err = R_BSP_HardwareLock(&ssp_feature);
    QSPI_ERROR_RETURN(SSP_SUCCESS == err, err);

    /** Set the default bank to the first bank */
    HW_QSPI_BANK_SELECT(p_ctrl->p_reg, 0);

    /* Get the Addressing mode from configuration structure */
    p_ctrl->num_address_bytes = p_cfg->addr_mode;

    /** Get the configuration of the quad SPI flash device and remember it for subsequent operations */
    bsp_qspi_config_get(&(p_ctrl->manufacturer_id),
                        &(p_ctrl->memory_type),
                        &(p_ctrl->memory_capacity),
                        &(p_ctrl->max_eraseable_size),
                        &(p_ctrl->num_address_bytes),
                        &(p_ctrl->spi_mode),
                        &(p_ctrl->page_size),
                        &(p_ctrl->xip_mode));

    /** If populated flash is 16MB & address mode configured as 4-byte,returns an unsupported error */
    if (0U == p_ctrl->num_address_bytes)
    {
        /* Release hardware lock. */
        R_BSP_HardwareUnlock(&ssp_feature);
        return SSP_ERR_UNSUPPORTED;
    }

#ifdef QSPI_PAGE_PROGRAM_DATA_LINES
    /* This is the index into g_qspi_prv_program_command.  The index should be 0 for data on one line, 1 for data on
     * 2 lines, and 2 for data on 4 lines. */
    uint8_t index = QSPI_PAGE_PROGRAM_DATA_LINES - 1U;
    if (3U == index)
    {
        index = 2U;
    }
    p_ctrl->data_lines = index;
#else
    /* This is for compatibility with a prior implementation of the BSP QSPI code that set spi_mode based on the
     * number of data lines available instead of the current SPI mode.  QSPI_PAGE_PROGRAM_DATA_LINES should always be
     * defined for new implementations that use multiple data lines for page program. */
    p_ctrl->data_lines = (uint8_t) p_ctrl->spi_mode;
#endif

    p_ctrl->total_size_bytes = (uint32_t)((uint32_t)1 << p_ctrl->memory_capacity);

    /** A zero in the manufacturer_id mean the flash device is broken, misconfigured, or not populated */
    if (p_ctrl->manufacturer_id == 0U)
    {
        /* Release hardware lock. */
        R_BSP_HardwareUnlock(&ssp_feature);
        return SSP_ERR_UNSUPPORTED;
    }

    /** Mark driver as opened by initializing it to "RQSP" in its ASCII equivalent for this unit. */
    p_ctrl->open = RQSPI_OPEN;

    /** Exit XIP mode while the driver is open */
    if (p_ctrl->xip_mode)
    {
        bsp_qspi_xip_exit();
    }

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Close the QSPI driver module.
 *
 * Return the QSPI module back to ROM access mode.
 *
 * @retval SSP_SUCCESS             Configuration was successful.
 * @retval SSP_ERR_ASSERTION       p_ctrl is NULL.
 * @retval SSP_ERR_NOT_OPEN        Driver is not opened.
 **********************************************************************************************************************/
ssp_err_t R_QSPI_Close (qspi_ctrl_t * p_api_ctrl)
{
    qspi_instance_ctrl_t * p_ctrl = (qspi_instance_ctrl_t *) p_api_ctrl;

#if QSPI_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(p_ctrl);
#endif
    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};
    ssp_feature.channel = 0U;
    ssp_feature.unit = 0U;
    ssp_feature.id = SSP_IP_QSPI;

    /* Check if the device is even open, return an error if not */
    QSPI_ERROR_RETURN(RQSPI_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);

    /** Check if the device is open */
    QSPI_ERROR_RETURN(p_ctrl->manufacturer_id != 0U, SSP_ERR_NOT_OPEN);

    /** Re-enter XIP mode if it was running in this mode before entering opening the driver */
    if (p_ctrl->xip_mode)
    {
        bsp_qspi_xip_enter();
    }

    /* Release hardware lock. */
    R_BSP_HardwareUnlock(&ssp_feature);

    /* The device is now considered closed */
    p_ctrl->open = 0U;

    /** Clearing the manufacturing_id, memory_type and memory_capacity */
    p_ctrl->manufacturer_id = (uint8_t)0;
    p_ctrl->memory_type = (uint8_t)0;
    p_ctrl->memory_capacity =(uint8_t)0;
    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Read data from the flash.
 *
 * Read a block of data from a particular address on the SPI flash device.
 *
 * @retval SSP_SUCCESS             The flash was programmed successfully.
 * @retval SSP_ERR_UNSUPPORTED     The device address is invalid.
 * @retval SSP_ERR_ASSERTION       p_ctrl,p_device_address or p_memory_address is NULL.
 * @retval SSP_ERR_NOT_OPEN        Driver is not opened.
 * @retval SSP_ERR_TRANSFER_BUSY   Another serial communications transfer is in progress.
 **********************************************************************************************************************/
ssp_err_t R_QSPI_Read (qspi_ctrl_t * p_api_ctrl,
                       uint8_t     * p_device_address,
                       uint8_t     * p_memory_address,
                       uint32_t    byte_count)
{
    qspi_instance_ctrl_t * p_ctrl = (qspi_instance_ctrl_t *) p_api_ctrl;

#if QSPI_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(p_ctrl);
    SSP_ASSERT(p_device_address);
    SSP_ASSERT(p_memory_address);
#endif

    /** Check if the device is open */
    QSPI_ERROR_RETURN(p_ctrl->manufacturer_id != 0U, SSP_ERR_NOT_OPEN);

    /** Check whether the device address is valid */
    ssp_err_t err = qspi_validate_address_range(p_ctrl, p_device_address, byte_count);
    QSPI_ERROR_RETURN(SSP_SUCCESS == err, err);

    /** Make sure no other communication is in progress. */
    QSPI_ERROR_RETURN(!HW_QSPI_COM_STATUS_READ(p_ctrl->p_reg), SSP_ERR_TRANSFER_BUSY);

    memcpy(p_memory_address, p_device_address, byte_count);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Program a page of data to the flash.
 *
 * @retval SSP_SUCCESS                  The flash was programmed successfully.
 * @retval SSP_ERR_UNSUPPORTED          The device address is invalid.
 * @retval SSP_ERR_ASSERTION            p_ctrl, p_device_address or p_memory_address is NULL.
 * @retval SSP_ERR_INVALID_ARGUMENT     Invalid parameter is passed.
 * @retval SSP_ERR_NOT_OPEN             Driver is not opened.
 **********************************************************************************************************************/
ssp_err_t R_QSPI_PageProgram (qspi_ctrl_t * p_api_ctrl,
                              uint8_t     * p_device_address,
                              uint8_t     * p_memory_address,
                              uint32_t      byte_count)
{
    qspi_instance_ctrl_t * p_ctrl = (qspi_instance_ctrl_t *) p_api_ctrl;

    ssp_err_t err = qspi_program_param_check(p_ctrl, p_device_address, p_memory_address, byte_count);
    QSPI_ERROR_RETURN(SSP_SUCCESS == err, err);

    /** Check whether the device address is valid */
    err = qspi_validate_address_range(p_ctrl, p_device_address, byte_count);
    QSPI_ERROR_RETURN(SSP_SUCCESS == err, err);

    uint32_t chip_address      = (uint32_t) p_device_address - BSP_PRV_QSPI_DEVICE_PHYSICAL_ADDRESS;

    /* Place the QSPI block into Direct Communication mode */
    HW_QSPI_DIRECT_COMMUNICATION_ENTER(p_ctrl->p_reg);

    /** Send command to enable writing */
    HW_QSPI_BYTE_WRITE(p_ctrl->p_reg, QSPI_COMMAND_WRITE_ENABLE);

    /* Close the SPI bus cycle */
    HW_QSPI_DIRECT_COMMUNICATION_ENTER(p_ctrl->p_reg);

    bool restore_spi_mode = false;
    void (*write_command) (R_QSPI_Type * p_qspi_reg, uint8_t byte) = qspi_d0_byte_write_standard;
    void (*write_address) (R_QSPI_Type * p_qspi_reg, uint8_t byte) = qspi_d0_byte_write_standard;

    /** If the peripheral is in extended SPI mode, and the configuration provided in the BSP allows for programming on
     * multiple data lines, and a unique command is provided for the required mode, update the SPI protocol to send
     * data on multiple lines. */
    uint8_t command = g_qspi_prv_program_command[p_ctrl->data_lines][p_ctrl->num_address_bytes >> 2U];
    uint8_t one_line_command = g_qspi_prv_program_command[QSPI_EXTENDED_SPI_PROTOCOL][p_ctrl->num_address_bytes >> 2U];
    if ((QSPI_EXTENDED_SPI_PROTOCOL == HW_QSPI_SPI_MODE_GET(p_ctrl->p_reg)) && (command != one_line_command))
    {
        /* Exit direct communication mode to update the SPI protocol. */
        HW_QSPI_DIRECT_COMMUNICATION_EXIT(p_ctrl->p_reg);

        HW_QSPI_SPI_MODE_SET(p_ctrl->p_reg, p_ctrl->data_lines);

        /* Place the QSPI block back into direct communication mode. */
        HW_QSPI_DIRECT_COMMUNICATION_ENTER(p_ctrl->p_reg);

        restore_spi_mode = true;

        /* Write command in extended SPI mode on one line. */
        write_command = gp_qspi_prv_byte_write[p_ctrl->data_lines];

#ifdef QSPI_PAGE_PROGRAM_ADDRESS_ONE_LINE
#if QSPI_PAGE_PROGRAM_ADDRESS_ONE_LINE == 1U
        /* Write address in extended SPI mode on one line. */
        write_address = gp_qspi_prv_byte_write[p_ctrl->data_lines];
#endif
#endif
    }

    /** Send command to write data */
    write_command(p_ctrl->p_reg, command);

    /** Write the address. */
    if (p_ctrl->num_address_bytes == QSPI_4_BYTE_ADDRESS)
    {
        /* Send the MSByte of the address */
        write_address(p_ctrl->p_reg, (uint8_t) (chip_address >> 24));
    }

    /* Send the remaining bytes of the address */
    write_address(p_ctrl->p_reg, (uint8_t) (chip_address >> 16));
    write_address(p_ctrl->p_reg, (uint8_t) (chip_address >> 8));
    write_address(p_ctrl->p_reg, (uint8_t) (chip_address));

    /** Write the data. */
    while (byte_count)
    {
        /* Read the device memory into the passed in buffer */
        HW_QSPI_BYTE_WRITE(p_ctrl->p_reg, *p_memory_address);
        p_memory_address = p_memory_address + 1;
        byte_count = byte_count - 1;
    }

    /* Close the SPI bus cycle */
    HW_QSPI_DIRECT_COMMUNICATION_ENTER(p_ctrl->p_reg);

    /** If the SPI protocol was modified in this function, restore it. */
    if (restore_spi_mode)
    {
        /* Exit direct communication mode to restore the SPI protocol setting. */
        HW_QSPI_DIRECT_COMMUNICATION_EXIT(p_ctrl->p_reg);

        /* Restore SPI mode to extended SPI mode. */
        HW_QSPI_SPI_MODE_SET(p_ctrl->p_reg, 0U);

        /* Place the QSPI block back into direct communication mode,. */
        HW_QSPI_DIRECT_COMMUNICATION_ENTER(p_ctrl->p_reg);
    }

    /** Send command to disable writing */
    HW_QSPI_BYTE_WRITE(p_ctrl->p_reg, QSPI_COMMAND_WRITE_DISABLE);

    /* Close the SPI bus cycle */
    HW_QSPI_DIRECT_COMMUNICATION_ENTER(p_ctrl->p_reg);

    /* Return to ROM access mode */
    HW_QSPI_DIRECT_COMMUNICATION_EXIT(p_ctrl->p_reg);

    return SSP_SUCCESS;
}
/*******************************************************************************************************************//**
 * @brief  Erase a number of byte from the flash.
 *
 *
 * @retval SSP_SUCCESS              The command to erase the flash was executed successfully.
 * @retval SSP_ERR_UNSUPPORTED      The device address is invalid.
 * @retval SSP_ERR_ASSERTION        p_ctrl or p_device_address is NULL.
 * @retval SSP_ERR_INVALID_ARGUMENT Invalid byte_count entered.
 * @retval SSP_ERR_NOT_OPEN         Driver is not opened.
 **********************************************************************************************************************/
ssp_err_t R_QSPI_Erase(qspi_ctrl_t * p_api_ctrl, uint8_t * p_device_address, uint32_t byte_count)
{
    qspi_instance_ctrl_t * p_ctrl = (qspi_instance_ctrl_t *) p_api_ctrl;

#if QSPI_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(p_ctrl);
    SSP_ASSERT(p_device_address);
#endif

    /** Check if the device is open */
    QSPI_ERROR_RETURN(p_ctrl->manufacturer_id != 0U, SSP_ERR_NOT_OPEN);

    ssp_err_t      ret_val    = SSP_ERR_INVALID_ARGUMENT;
    qspi_info_t    qspi_info = {0};
    uint8_t        size_index = 0;
    uint8_t        cmd_index = 0;
    uint8_t        erase_command = 0;

    /** Check whether the device address is valid */
    ssp_err_t err = qspi_validate_address_range(p_ctrl, p_device_address, byte_count);
    QSPI_ERROR_RETURN(SSP_SUCCESS == err, err);

    uint32_t chip_address = (uint32_t) p_device_address - BSP_PRV_QSPI_DEVICE_PHYSICAL_ADDRESS;

    /** Get the information of underlying flash */
    bsp_qspi_erase_sizes_get(&qspi_info.p_erase_sizes_bytes, &qspi_info.num_erase_sizes);

    for(size_index = (uint8_t)0; size_index < qspi_info.num_erase_sizes; size_index++)
    {
        /** If requested byte_count is supported by underlying flash, assign the value of
         *  size_index to cmd_index for searching the command */
        if(byte_count == qspi_info.p_erase_sizes_bytes[size_index])
        {
            cmd_index = size_index;
            ret_val   = SSP_SUCCESS;
            break;
        }
    }

    if(SSP_SUCCESS ==  ret_val)
    {
        /* Place the QSPI block into Direct Communication mode */
        HW_QSPI_DIRECT_COMMUNICATION_ENTER(p_ctrl->p_reg);

        /** Send command to enable writing */
        HW_QSPI_BYTE_WRITE(p_ctrl->p_reg, QSPI_COMMAND_WRITE_ENABLE);

        /* Close the SPI bus cycle */
        HW_QSPI_DIRECT_COMMUNICATION_ENTER(p_ctrl->p_reg);

        /** Get the erase command */
        bsp_qspi_erase_command_get(&erase_command, cmd_index);

        /** Send command to erase */
        HW_QSPI_BYTE_WRITE(p_ctrl->p_reg, erase_command);

        /** If the command is not a chip erase command then send the start address */
        if (byte_count != p_ctrl->total_size_bytes)
        {
            if (p_ctrl->num_address_bytes == (uint32_t)QSPI_4_BYTE_ADDRESS)
            {
                /** Send command to write data */
                HW_QSPI_BYTE_WRITE(p_ctrl->p_reg, (uint8_t)(chip_address >> 24));
            }
            HW_QSPI_BYTE_WRITE(p_ctrl->p_reg, (uint8_t)(chip_address >> 16));
            HW_QSPI_BYTE_WRITE(p_ctrl->p_reg, (uint8_t)(chip_address >> 8));
            HW_QSPI_BYTE_WRITE(p_ctrl->p_reg, (uint8_t)(chip_address));
        }

        /* Close the SPI bus cycle */
        HW_QSPI_DIRECT_COMMUNICATION_ENTER(p_ctrl->p_reg);

        /** Send command to disable writing */
        HW_QSPI_BYTE_WRITE(p_ctrl->p_reg, QSPI_COMMAND_WRITE_DISABLE);

        /* Close the SPI bus cycle */
        HW_QSPI_DIRECT_COMMUNICATION_ENTER(p_ctrl->p_reg);

        /* Return to ROM access mode */
        HW_QSPI_DIRECT_COMMUNICATION_EXIT(p_ctrl->p_reg);
    }
    return ret_val;
}

/*******************************************************************************************************************//**
 * @brief  Returns the information about the flash.
 * @retval SSP_SUCCESS             Operation was successful.
 * @retval SSP_ERR_ASSERTION       p_ctrl or p_info is NULL.
 * @retval SSP_ERR_NOT_OPEN        Driver is not opened.
 **********************************************************************************************************************/
ssp_err_t R_QSPI_InfoGet(qspi_ctrl_t  * p_api_ctrl, qspi_info_t  * const p_info)
{
    qspi_instance_ctrl_t * p_ctrl = (qspi_instance_ctrl_t *) p_api_ctrl;

#if QSPI_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(p_ctrl);
    SSP_ASSERT(p_info);
#endif

    /** Check if the device is open */
    QSPI_ERROR_RETURN(p_ctrl->manufacturer_id != 0U, SSP_ERR_NOT_OPEN);

    /** Get the information of underlying flash */
    bsp_qspi_erase_sizes_get(&p_info->p_erase_sizes_bytes, &p_info->num_erase_sizes);

    p_info->min_program_size_bytes = p_ctrl->page_size;
    p_info->total_size_bytes = p_ctrl->total_size_bytes;

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Erase a sector on the flash.
 *
 * Erase one sector on the SPI flash device. Any passed in address within the sector to be erased is acceptable.
 *
 * @retval SSP_SUCCESS             The command to erase the sector of flash was executed successfully.
 * @retval SSP_ERR_UNSUPPORTED     The device address is invalid.
 * @retval SSP_ERR_ASSERTION       p_ctrl or p_device_address is NULL.
 * @retval SSP_ERR_NOT_OPEN        Driver is not opened.
 **********************************************************************************************************************/
ssp_err_t R_QSPI_SectorErase (qspi_ctrl_t * p_api_ctrl, uint8_t * p_device_address)
{
    qspi_instance_ctrl_t * p_ctrl = (qspi_instance_ctrl_t *) p_api_ctrl;

#if QSPI_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(p_ctrl);
    SSP_ASSERT(p_device_address);
#endif

    /** Check whether the device address is valid */
    ssp_err_t err = qspi_validate_address_range(p_ctrl, p_device_address, 0U);
    QSPI_ERROR_RETURN(SSP_SUCCESS == err, err);

    uint32_t chip_address = (uint32_t) p_device_address - BSP_PRV_QSPI_DEVICE_PHYSICAL_ADDRESS;

    /** Check if the device is open */
    QSPI_ERROR_RETURN(p_ctrl->manufacturer_id != 0U, SSP_ERR_NOT_OPEN);

    /** Place the QSPI block into Direct Communication mode */
    HW_QSPI_DIRECT_COMMUNICATION_ENTER(p_ctrl->p_reg);

    /** Send command to enable writing */
    HW_QSPI_BYTE_WRITE(p_ctrl->p_reg, QSPI_COMMAND_WRITE_ENABLE);

    /** Close the SPI bus cycle */
    HW_QSPI_DIRECT_COMMUNICATION_ENTER(p_ctrl->p_reg);

    if (p_ctrl->num_address_bytes == (uint32_t)QSPI_4_BYTE_ADDRESS)
    {
        /** Send command to erase */
        HW_QSPI_BYTE_WRITE(p_ctrl->p_reg, QSPI_COMMAND_4BYTE_SECTOR_ERASE);

        /** Send command to write data */
        HW_QSPI_BYTE_WRITE(p_ctrl->p_reg, (uint8_t)(chip_address >> 24));
    }
    else
    {
        /** Send command to erase */
        HW_QSPI_BYTE_WRITE(p_ctrl->p_reg, QSPI_COMMAND_SECTOR_ERASE);
    }

    HW_QSPI_BYTE_WRITE(p_ctrl->p_reg, (uint8_t)(chip_address >> 16));
    HW_QSPI_BYTE_WRITE(p_ctrl->p_reg, (uint8_t)(chip_address >> 8));
    HW_QSPI_BYTE_WRITE(p_ctrl->p_reg, (uint8_t)(chip_address));

    /** Close the SPI bus cycle */
    HW_QSPI_DIRECT_COMMUNICATION_ENTER(p_ctrl->p_reg);

    /** Send command to disable writing */
    HW_QSPI_BYTE_WRITE(p_ctrl->p_reg, QSPI_COMMAND_WRITE_DISABLE);

    /** Close the SPI bus cycle */
    HW_QSPI_DIRECT_COMMUNICATION_ENTER(p_ctrl->p_reg);

    /** Return to ROM access mode */
    HW_QSPI_DIRECT_COMMUNICATION_EXIT(p_ctrl->p_reg);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Get the write or erase status of the flash.
 *
 * Return the write status of the flash. This is most useful for determining if erases are complete.
 *
 * @retval SSP_SUCCESS             The write status is correct.
 * @retval SSP_ERR_ASSERTION        p_ctrl or p_write_in_progress is NULL.
 * @retval SSP_ERR_NOT_OPEN        Driver is not opened.
 **********************************************************************************************************************/
ssp_err_t R_QSPI_StatusGet (qspi_ctrl_t * p_api_ctrl, bool * p_write_in_progress)
{
    qspi_instance_ctrl_t * p_ctrl = (qspi_instance_ctrl_t *) p_api_ctrl;

#if QSPI_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(p_ctrl);
    SSP_ASSERT(p_write_in_progress);
#endif

    /** Check if the device is open */
    QSPI_ERROR_RETURN(p_ctrl->manufacturer_id != 0U, SSP_ERR_NOT_OPEN);

    /** Place the QSPI block into Direct Communication mode */
    HW_QSPI_DIRECT_COMMUNICATION_ENTER(p_ctrl->p_reg);

    /** Get the write status from the device */
    bsp_qspi_status_get(p_write_in_progress);

    /** Return to ROM access mode */
    HW_QSPI_DIRECT_COMMUNICATION_EXIT(p_ctrl->p_reg);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Select the bank to access.
 *
 * A bank is a 64MB sliding access window into the flash memory space. This function sets the current bank.
 *
 * @retval SSP_SUCCESS             Bank successfully selected.
 *
 **********************************************************************************************************************/
ssp_err_t R_QSPI_BankSelect (uint32_t bank)
{
    /** Return back to ROM access mode */
    HW_QSPI_BANK_SELECT(gp_qspi_reg, bank);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief      Get the driver version based on compile time macros.
 *
 * @retval     SSP_SUCCESS          Successful close.
 * @retval     SSP_ERR_ASSERTION    p_version is NULL.
 *
 **********************************************************************************************************************/
ssp_err_t R_QSPI_VersionGet (ssp_version_t * const p_version)
{
#if QSPI_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(p_version);
#endif

    p_version->version_id = s_qspi_version.version_id;

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @} (end addtogroup QSPI)
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * Writes a byte in extended SPI mode when the SPI peripheral is configured for quad mode.
 *
 * @param[in] p_qspi_reg   Pointer to QSPI registers
 * @param[in] byte         Byte to write
 **********************************************************************************************************************/
static void qspi_d0_byte_write_quad_mode (R_QSPI_Type * p_qspi_reg, uint8_t byte)
{
    /* The LSB of each nibble ends up on D0. */
    uint32_t value = 0xEEEEEEEE;
    for (uint32_t i = 0U; i < 8U; i++)
    {
        uint32_t bit = ((uint32_t) (byte >> i) & 1U);

        /* Place bits in every 4th bit (bit 0, 4, 8, ... 28). */
        uint32_t bit_mask = bit << (i * 4U);

        value |= bit_mask;
    }

    HW_QSPI_BYTE_WRITE(p_qspi_reg, (uint8_t) (value >> 24U));
    HW_QSPI_BYTE_WRITE(p_qspi_reg, (uint8_t) (value >> 16U));
    HW_QSPI_BYTE_WRITE(p_qspi_reg, (uint8_t) (value >> 8U));
    HW_QSPI_BYTE_WRITE(p_qspi_reg, (uint8_t) value);
}

/*******************************************************************************************************************//**
 * Writes a byte in extended SPI mode when the SPI peripheral is configured for dual mode.
 *
 * @param[in] p_qspi_reg   Pointer to QSPI registers
 * @param[in] byte         Byte to write
 **********************************************************************************************************************/
static void qspi_d0_byte_write_dual_mode (R_QSPI_Type * p_qspi_reg, uint8_t byte)
{
    /* Every other bit ends up on D0. Unused bits are set. */
    uint16_t value = 0xAAAA;
    for (uint32_t i = 0U; i < 8U; i++)
    {
        uint16_t bit = ((uint16_t) (byte >> i) & 1U);

        /* Place bits in every other bit (bit 0, 2, 4 ... 14). */
        uint16_t bit_mask = (uint16_t) (bit << (i * 2U));

        value |= bit_mask;
    }

    HW_QSPI_BYTE_WRITE(p_qspi_reg, (uint8_t) (value >> 8U));
    HW_QSPI_BYTE_WRITE(p_qspi_reg, (uint8_t) value);
}

/*******************************************************************************************************************//**
 * Writes a byte in the SPI mode configured in the QSPI peripheral.
 *
 * @param[in] p_qspi_reg   Pointer to QSPI registers
 * @param[in] byte         Byte to write
 **********************************************************************************************************************/
static void qspi_d0_byte_write_standard (R_QSPI_Type * p_qspi_reg, uint8_t byte)
{
    HW_QSPI_BYTE_WRITE(p_qspi_reg, byte);
}

/*******************************************************************************************************************//**
 * Parameter checking for R_QSPI_PageProgram.
 *
 * @param[in] p_ctrl               Pointer to a driver handle
 * @param[in] p_device_address     The location in the flash device address space to write the data to
 * @param[in] p_memory_address     The memory address of the data to write to the flash device
 * @param[in] byte_count           The number of bytes to write
 *
 * @retval SSP_SUCCESS                  Parameters are valid.
 * @retval SSP_ERR_ASSERTION            p_ctrl,p_device_address or p_memory_address is NULL.
 * @retval SSP_ERR_INVALID_ARGUMENT     Invalid parameter is passed.
 * @retval SSP_ERR_NOT_OPEN             Driver is not opened.
 **********************************************************************************************************************/
static ssp_err_t qspi_program_param_check (qspi_instance_ctrl_t * p_ctrl,
                                           uint8_t              * p_device_address,
                                           uint8_t              * p_memory_address,
                                           uint32_t               byte_count)
{

#if QSPI_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(p_ctrl);
    SSP_ASSERT(p_device_address);
    SSP_ASSERT(p_memory_address);
#else
    /* Device address and memory address not used in this function. */
    SSP_PARAMETER_NOT_USED (p_device_address);
    SSP_PARAMETER_NOT_USED (p_memory_address);
#endif

    /* Check if the device is open */
    QSPI_ERROR_RETURN(p_ctrl->manufacturer_id != 0U, SSP_ERR_NOT_OPEN);

    /* Check if byte_count is valid */
    QSPI_ERROR_RETURN(byte_count <= p_ctrl->page_size, SSP_ERR_INVALID_ARGUMENT);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief      Validate the address range based on device address.
 *
 * @param[in]  p_ctrl            The instance control
 * @param[in]  p_device_address  The qspi device address
 * @param[in]  byte_count        The number of bytes
 *
 * @retval     SSP_SUCCESS          The parameters are successfully validated.
 * @retval     SSP_ERR_UNSUPPORTED  The parameters are invalid.
 * @return     Parameter validation status.
 **********************************************************************************************************************/
static ssp_err_t qspi_validate_address_range (qspi_instance_ctrl_t * p_ctrl,
                                              uint8_t              * p_device_address,
                                              uint32_t               byte_count)
{
    /* If device address is more than 16MB and addressing mode is configured as 3 bytes,
     * then returns an error */
    if (((uint8_t *)QSPI_ADDR_RANGE_3_BYTES <= p_device_address) &&
        (QSPI_3BYTE_ADDR_MODE == p_ctrl->num_address_bytes))
    {
        return SSP_ERR_UNSUPPORTED;
    }
    /* If device address is less than mapped device physical start address, then returns an error */
    else if ((uint8_t*)BSP_PRV_QSPI_DEVICE_PHYSICAL_ADDRESS > p_device_address)
    {
        return SSP_ERR_UNSUPPORTED;
    }
    /* If calculated memory size i.e. addition of device start address and byte count exceeds the
     * total size of the qspi flash memory available, then returns an error */
    else if ((BSP_PRV_QSPI_DEVICE_PHYSICAL_ADDRESS + p_ctrl->total_size_bytes) <
             (byte_count + (uint32_t)p_device_address))
    {
        return SSP_ERR_UNSUPPORTED;
    }

    return SSP_SUCCESS;
}
