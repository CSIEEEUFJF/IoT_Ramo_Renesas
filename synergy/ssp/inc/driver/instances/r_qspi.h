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
 * File Name    : r_qspi.h
 * Description  : QSPI HAL module header file
 **********************************************************************************************************************/


/*******************************************************************************************************************//**
 * @ingroup HAL_Library
 * @defgroup QSPI QSPI
 * @brief Driver for the Quad Serial Peripheral Interface (QSPI).
 *
 * This is a driver for the Quad-SPI module (QSPI) which is a memory controller for connecting a serial ROM (non-volatile
 * memory such as a serial flash memory, serial EEPROM, or serial FeRAM) that has an SPI-compatible interface.
 *
 * @section QSPI_SUMMARY Summary
 * Extends @ref QSPI_API.
 * @{
 **********************************************************************************************************************/

#ifndef R_QSPI_H
#define R_QSPI_H


/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "bsp_api.h"
#include <string.h>
#include "r_qspi_cfg.h"
#include "r_qspi_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define QSPI_CODE_VERSION_MAJOR (2U)
#define QSPI_CODE_VERSION_MINOR (0U)

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
/** Instance control block. DO NOT INITIALIZE.  Initialization occurs when qspi_api_t::open is called */
typedef struct st_qspi_instance_ctrl
{
    R_QSPI_Type * p_reg;          ///< Pointer to QSPI base register
    uint32_t  max_eraseable_size; ///< Largest eraseable sector size in kbytes. Used to determine buffer size for
                                  /// partial sector erases.
    uint32_t  num_address_bytes;  ///< Number of bytes used to represent the address
    uint32_t  spi_mode;           ///< SPI mode - 0 = Extended, 1 = Dual, 2 = Quad
    uint32_t  page_size;          ///< Number of bytes in a programmable page
    uint8_t   data_lines;         ///< data lines - 0 = 1 line, 1 = 2 lines, 2 = 4 lines
    uint8_t   manufacturer_id;    ///< Manufacturer ID
    uint8_t   memory_type;        ///< Memory type
    uint8_t   memory_capacity;    ///< Memory capacity (in MByte)
    bool      xip_mode;           ///< 0 = run in read mode, 1 = run in XIP mode
    uint32_t  total_size_bytes;   ///< Total size of the flash in bytes
    uint32_t  open;               ///< Flag to determine if the device is open
} qspi_instance_ctrl_t;

/**********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/
/** @cond INC_HEADER_DEFS_SEC */
/** Filled in Interface API structure for this Instance. */
extern const qspi_api_t g_qspi_on_qspi;
/** @endcond */

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* R_QSPI_H */

/*******************************************************************************************************************//**
 * @} (end defgroup QSPI)
 **********************************************************************************************************************/
