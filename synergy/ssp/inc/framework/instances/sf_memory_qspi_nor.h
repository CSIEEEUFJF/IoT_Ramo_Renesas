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
 * File Name    : sf_memory_qspi_nor.h
 * Description  : QSPI NOR Memory framework header file.
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @ingroup SF_Library
 * @defgroup SF_MEMORY_QSPI_NOR Memory framework
 * @brief RTOS-integrated Memory framework for QSPI NOR driver.
 *
 * @{
 **********************************************************************************************************************/

#ifndef SF_MEMORY_QSPI_NOR_H
#define SF_MEMORY_QSPI_NOR_H

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "r_qspi_api.h"
#include "sf_memory_api.h"
#include "tx_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
/** Version of code that implements the API defined in this file */
#define SF_MEMORY_QSPI_NOR_CODE_VERSION_MAJOR (2U)
#define SF_MEMORY_QSPI_NOR_CODE_VERSION_MINOR (0U)

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

typedef enum e_sf_memory_qspi_nor_pending_operation
{
    SF_MEMORY_QSPI_NOR_PENDING_WRITE,
    SF_MEMORY_QSPI_NOR_PENDING_ERASE
} sf_memory_qspi_nor_pending_operation_t;

typedef struct st_sf_memory_qspi_nor_delay_callback_args
{
    sf_memory_qspi_nor_pending_operation_t pending_operation;   ///< The last operation sent to the QSPI chip.
    uint32_t   pending_operation_size;                          ///< The size of the last operation.
    uint32_t   timeout_remaining;                               ///< Remaining timeout.
    uint32_t   times_called;                                    ///< Number of times the callback has been called since 
                                                                ///< the operation started.
    void     * p_context;                                       ///< Context provided by user.
} sf_memory_qspi_nor_delay_callback_args_t;

/** Control block. DO NOT INITIALIZE.  Initialization occurs when sf_memory_api_t::open is called */
typedef struct st_sf_memory_qpsi_nor_instance_ctrl
{
    qspi_instance_t *                      p_qspi;             ///< QSPI instance
    qspi_info_t                            qspi_info;          ///< QSPI info
    uint32_t                               open;               ///< Track opened/closed status
    sf_memory_region_info_t                region_info;        ///< Memory region info
    uint32_t                               timeout_ticks;      ///< Number of ticks to timeout on erase or write waiting
    sf_memory_qspi_nor_pending_operation_t pending_operation;  ///< The last operation sent to the QSPI chip.
    uint32_t                               pending_operation_size; ///< The size of the last operation.
    void                           *       p_delay_callback_context;  ///< Context passed to the delay callback.
    void   (* p_delay_callback)(sf_memory_qspi_nor_delay_callback_args_t * p_args); ///< Pointer to user callback function
} sf_memory_qspi_nor_instance_ctrl_t;

/** User configuration structure, used in open function */
typedef struct st_sf_memory_qspi_nor_cfg
{
    qspi_instance_t const * const p_qspi;                   ///< Lower level driver
    uint32_t                      timeout_ticks;            ///< Number of ticks to timeout on erase or write waiting.
    void                  *       p_delay_callback_context; ///< Context passed to the delay callback.
    /** A custom delay callback can be used to fine tune the amount of time to wait before starting to poll the QSPI
        chip after a write or erase operation. If a custom delay callback is used and a timeout occurs the 
        p_args->timeout_remaining variable must be set to 0 to indicate to the calling function that a timeout
        has occurred. */
    void (* p_delay_callback)(sf_memory_qspi_nor_delay_callback_args_t * p_args);


} sf_memory_qspi_nor_cfg_t;

/**********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/
/** @cond INC_HEADER_DEFS_SEC */
/** Filled in Interface API structure for this Instance. */
extern const sf_memory_api_t g_sf_memory_on_sf_memory_qspi_nor;
/** @endcond */

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

/*******************************************************************************************************************//**
 * @} (end defgroup SF_MEMORY_QSPI_NOR)
 **********************************************************************************************************************/
#endif /* SF_MEMORY_QSPI_NOR_H */
