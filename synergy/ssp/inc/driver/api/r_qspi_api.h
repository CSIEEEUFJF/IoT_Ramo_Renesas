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
 * File Name    : r_qspi_api.h
 * Description  : QSPI Interface
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @ingroup Interface_Library
 * @defgroup QSPI_API Quad SPI Flash Interface
 * @brief Interface for accessing external SPI flash devices.
 *
 * @section QSPI_API_SUMMARY Summary
 * The QSPI module provides an interface that writes and erases sectors in quad SPI flash devices connected to the QSPI
 * interface.
 *
 * Related SSP architecture topics:
 *  - @ref ssp-interfaces
 *  - @ref ssp-predefined-layers
 *  - @ref using-ssp-modules
 *
 * QSPI Interface description: @ref HALQSPIInterface
 * @{
 **********************************************************************************************************************/

#ifndef DRV_QSPI_API_H
#define DRV_QSPI_API_H

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
/* Register definitions, common services and error codes. */
#include "bsp_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define QSPI_API_VERSION_MAJOR (2U)
#define QSPI_API_VERSION_MINOR (0U)

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/** User configuration structure used for selecting addressing mode */
typedef enum e_qspi_address_mode
{
    QSPI_3BYTE_ADDR_MODE = 3U,
    QSPI_4BYTE_ADDR_MODE = 4U
} qspi_address_mode_t;

/** User configuration structure used by the open function */
typedef struct st_qspi_cfg
{
    void  * p_extend;           ///< place holder for future development
    qspi_address_mode_t addr_mode;
} qspi_cfg_t;

/** QSPI control block.  Allocate an instance specific control block to pass into the QSPI API calls.
 * @par Implemented as
 * - qspi_instance_ctrl_t
 */
typedef void qspi_ctrl_t;

/** QSPI information structure to store information specific to the currently used QSPI. */
typedef struct st_qspi_info
{
    uint32_t      total_size_bytes;             ///< Size of this QSPI in bytes
    uint32_t      min_program_size_bytes;       ///< Minimum program size in bytes.
    uint32_t    * p_erase_sizes_bytes;          ///< Array of available erase sizes. Each entry is in bytes.
    uint8_t       num_erase_sizes;              ///< Number of available erase sizes.
} qspi_info_t;

/** QSPI functions implemented at the HAL layer follow this API. */
typedef struct st_qspi_api
{
    /** Open the QSPI driver module.
     * @par Implemented as
     * - R_QSPI_Open()
     *
     * @param[in] p_ctrl               Pointer to a driver handle
     * @param[in] p_cfg                Pointer to a configuration structure
     **/
    ssp_err_t (* open)(qspi_ctrl_t * p_ctrl, qspi_cfg_t const * const p_cfg);

    /** Close the QSPI driver module.
     * @par Implemented as
     * - R_QSPI_Close()
     * @param[in] p_ctrl               Pointer to a driver handle
     **/
    ssp_err_t (* close)(qspi_ctrl_t * p_ctrl);

    /** Read a block of data from the flash.
     * @par Implemented as
     * - R_QSPI_Read()
     *
     * @param[in] p_ctrl               Pointer to a driver handle
     * @param[in] p_device_address     The location in the flash device address space to read
     * @param[in] p_memory_address     The memory address of a buffer to place the read data in
     * @param[in] byte_count           The number of bytes to read
     **/
    ssp_err_t (* read)(qspi_ctrl_t * p_ctrl, uint8_t * p_device_address, uint8_t * p_memory_address,
                       uint32_t byte_count);

    /** Program a page of data to the flash.
     * @par Implemented as
     * - R_QSPI_PageProgram()
     *
     * @param[in] p_ctrl               Pointer to a driver handle
     * @param[in] p_device_address     The location in the flash device address space to write the data to
     * @param[in] p_memory_address     The memory address of the data to write to the flash device
     * @param[in] byte_count           The number of bytes to write
     **/
    ssp_err_t (* pageProgram)(qspi_ctrl_t * p_ctrl, uint8_t * p_device_address, uint8_t * p_memory_address,
                              uint32_t byte_count);

    /** Erase a certain number of bytes of the flash.
     * @par Implemented as
     * - R_QSPI_Erase()
     *
     * @param[in] p_ctrl               Pointer to a driver handle
     * @param[in] p_device_address     The location in the flash device address space to start the erase from
     * @param[in] byte_count           The number of bytes to erase
     **/
    ssp_err_t (* erase)(qspi_ctrl_t * p_ctrl, uint8_t * p_device_address, uint32_t byte_count);

    /** Get information about this specific QSPI flash.
     * @par Implemented as
     * - R_QSPI_InfoGet()
     *
     * @param[in]   p_ctrl     Control block set in qspi_api_t::open call for this timer.
     * @param[out]  p_info     Collection of information for this QSPI.
     */
    ssp_err_t (* infoGet)(qspi_ctrl_t    * const p_ctrl, qspi_info_t    * const p_info);

    /** Erase a sector of the flash.
     * @par Implemented as
     * - R_QSPI_SectorErase()
     *
     * @param[in] p_ctrl               Pointer to a driver handle
     * @param[in] p_device_address     The location in the flash device address space to start the erase from
     **/
    ssp_err_t (* sectorErase)(qspi_ctrl_t * p_ctrl, uint8_t * p_device_address);

    /** Get the write or erase status of the flash.
     * @par Implemented as
     * - R_QSPI_StatusGet()
     * @param[in] p_ctrl               Pointer to a driver handle
     * @param[in] p_write_in_progress  The write or erase status  - TRUE = write/erase in progress
     **/
    ssp_err_t (* statusGet)(qspi_ctrl_t * p_ctrl, bool * p_write_in_progress);

    /** Select the bank to access.
     * @par Implemented as
     * - R_QSPI_BankSelect()
     *
     * @param[in] bank                 The bank number
     **/
    ssp_err_t (* bankSelect)(uint32_t bank);

    /** Get the driver version based on compile time macros.
     * @par Implemented as
     * - R_QSPI_VersionGet()
     * @param[out]  p_version  Code and API version used.
     **/
    ssp_err_t (* versionGet)(ssp_version_t * const p_version);
} qspi_api_t;

/** This structure encompasses everything that is needed to use an instance of this interface. */
typedef struct st_qspi_instance
{
    qspi_ctrl_t      * p_ctrl;    ///< Pointer to the control structure for this instance
    qspi_cfg_t const * p_cfg;     ///< Pointer to the configuration structure for this instance
    qspi_api_t const * p_api;     ///< Pointer to the API structure for this instance
} qspi_instance_t;

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* DRV_QSPI_API_H */

/*******************************************************************************************************************//**
 * @} (end addtogroup QSPI_API)
 **********************************************************************************************************************/
