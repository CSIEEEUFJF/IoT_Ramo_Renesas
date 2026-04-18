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
* File Name    : sf_memory_api.h
* Description  : This file describes the structures and the function calls supported by the memory interface
***********************************************************************************************************************/


#ifndef SF_MEMORY_API_H
#define SF_MEMORY_API_H
/*******************************************************************************************************************//**
 * @ingroup SF_Interface_Library
 * @defgroup SF_MEMORY Memory interface
 * @brief Interface for Memory API
 *
 * @section MEMORY_API_SUMMARY Summary
 * The Memory interface supports the following features.
 * - Read from any memory
 * - Erase memory
 * - Write to Memory
 * - Retrieve device specific information
 *
 * Implemented by:
 * @ref SF_MEMORY_QSPI_NOR
 *
 * Related SSP architecture topics:
 *  - @ref ssp-interfaces
 *  - @ref ssp-predefined-layers
 *  - @ref using-ssp-modules
 *
 * Memory Interface description: @ref FrameworkMemoryInterface
 * @{
 **********************************************************************************************************************/
/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
/** Register definitions, common services and error codes. */
#include "bsp_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/**********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
#define SF_MEMORY_API_VERSION_MAJOR   (2U)
#define SF_MEMORY_API_VERSION_MINOR   (0U)

/**********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/

/** Representation of a memory layout */
typedef struct st_memory_region_info
{
    uint32_t   memory_start_address; ///< Start address of this memory region
    uint32_t   memory_end_address;   ///< End address of this memory region
    uint32_t   minimum_erase_size;   ///< Specify the minimum erase size of the memory type
    uint32_t   minimum_write_size;   ///< Specify the minimum write size of the memory type
} sf_memory_region_info_t;

/** Memory information supported by the instance */
typedef struct st_memory_info
{
    uint32_t  number_of_regions;               ///< Number of memory regions supported by this memory type
    sf_memory_region_info_t * p_regions_info;  ///< Memory region specific information
}sf_memory_info_t;

/** User configuration structure, used in open function */
typedef struct st_sf_memory_cfg
{
    void const     * p_extend;       ///< Extension parameter for hardware specific settings.
} sf_memory_cfg_t;

/** SF Memory API framework control block.  Allocate an instance specific control block to pass into the
 * framework API calls.
 * @par Implemented as
 * - sf_memory_qspi_nor_instance_ctrl_t
 */
typedef void sf_memory_ctrl_t;

/** SF_MEMORY functions implemented at the Framework layer will follow this API. */
typedef struct st_sf_memory_api
{
    /** Initialize Memory framework.
     * @par Implemented as
     * - SF_MEMORY_QSPI_NOR_Open
     * @param[in]   p_api_ctrl  Pointer to control block. Must be declared by user. Elements set here.
     * @param[in]   p_config    Pointer to configuration structure. All elements of this structure must be set by user.
     */
    ssp_err_t (* open)(sf_memory_ctrl_t       * const p_api_ctrl,
                       sf_memory_cfg_t  const * const p_cfg);

    /** Reads data from the specified memory device address to the location specified by the caller.
     * @par Implemented as
     * - SF_MEMORY_QSPI_NOR_Read
     * @param[in]   p_api_ctrl         Control block set in sf_memory_api_t::open call.
     * @param[in]   p_dest_address     Destination to read the data into.
     * @param[in]   memory_address     Address to read the data from.
     * @param[in]   num_bytes          Number of bytes of data to read.
     */
    ssp_err_t (* read)(sf_memory_ctrl_t * const p_api_ctrl,
                       uint8_t          * const p_dest_address,
                       uint32_t           const memory_address,
                       uint32_t           const num_bytes);

    /** Writes the specified number of data bytes to the specified device memory address.
     * @par Implemented as
     * - SF_MEMORY_QSPI_NOR_Write
     * @param[in]   p_api_ctrl        Control block set in sf_memory_api_t::open call.
     * @param[in]   p_src_address     Address to read the data to be written.
     * @param[in]   memory_address    Address to write the data to.
     * @param[in]   num_bytes         Number of bytes of data to write.
     */
    ssp_err_t (* write)(sf_memory_ctrl_t * const p_api_ctrl,
                        uint8_t          * const p_src_address,
                        uint32_t           const memory_address,
                        uint32_t           const num_bytes);

    /** Performs any buffered pending writes. This function MUST be called by the application after
     * the final write is called to complete any pending writes.
     * @par Implemented as
     * - SF_MEMORY_QSPI_NOR_Flush
     * @param[in]   p_api_ctrl     Control block set in sf_memory_api_t::open call.
     */
    ssp_err_t (* flush)(sf_memory_ctrl_t * const p_api_ctrl);

    /** Erases the specified number of bytes from the memory device.
     * @par Implemented as
     * - SF_MEMORY_QSPI_NOR_Erase
     * @param[in]   p_api_ctrl         Control block set in sf_memory_api_t::open call.
     * @param[in]   memory_address     Address to start the erase process at.
     * @param[in]   num_bytes          Number of bytes of data to erase.
     */
    ssp_err_t (* erase)(sf_memory_ctrl_t * const p_api_ctrl,
                        uint32_t           const memory_address,
                        uint32_t           const num_bytes);

    /** Returns information about the memory implementation.
     * @par Implemented as
     * - SF_MEMORY_QSPI_NOR_InfoGet
     * @param[in]   p_api_ctrl  Control block set in sf_memory_api_t::open call.
     * @param[out]  p_info      Pointer to information structure. All elements of this structure will be set by the
     *                          function.
     */
    ssp_err_t (* infoGet)(sf_memory_ctrl_t     * const p_api_ctrl,
                          sf_memory_info_t     * const p_info);

    /** Closes the module.
     * @par Implemented as
     * - SF_MEMORY_QSPI_NOR_Close
     * @param[in]   p_api_ctrl  Control block set in sf_memory_api_t::open call.
     */
    ssp_err_t (* close)(sf_memory_ctrl_t     * const p_api_ctrl);

    /** Gets version and stores it in provided pointer p_version.
     * @par Implemented as
     * - SF_MEMORY_QSPI_NOR_VersionGet
     * @param[out]  p_version  Code and API version used.
     */
    ssp_err_t (* versionGet)(ssp_version_t     * const p_version);

} sf_memory_api_t;

/** This structure encompasses everything that is needed to use an instance of this interface. */
typedef struct st_sf_memory_instance
{
    sf_memory_ctrl_t      * p_ctrl;    ///< Pointer to the control structure for this instance
    sf_memory_cfg_t const * p_cfg;     ///< Pointer to the configuration structure for this instance
    sf_memory_api_t const * p_api;     ///< Pointer to the API structure for this instance
} sf_memory_instance_t;

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

/*******************************************************************************************************************//**
 * @} (end addtogroup SF_MEMORY)
***********************************************************************************************************************/

#endif /* SF_MEMORY_API_H */


