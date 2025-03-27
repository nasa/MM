/************************************************************************
 * NASA Docket No. GSC-19,200-1, and identified as "cFS Draco"
 *
 * Copyright (c) 2023 United States Government as represented by the
 * Administrator of the National Aeronautics and Space Administration.
 * All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ************************************************************************/

/**
 * @file
 *   Unit specification for the Core Flight System (CFS)
 *   Memory Manger (MM) Application.
 */
#ifndef MM_APP_H
#define MM_APP_H

/* ======== */
/* Includes */
/* ======== */

#include "cfe.h"
#include "mm_mission_cfg.h"
#include "mm_msg.h"
#include "mm_platform_cfg.h"

/************************************************************************
 * Macro Definitions
 ************************************************************************/

/**
 * \name MM Command verification selection
 * \{
 */
#define MM_VERIFY_DUMP  0 /**< \brief Verify dump parameters */
#define MM_VERIFY_LOAD  1 /**< \brief Verify load parameters */
#define MM_VERIFY_EVENT 2 /**< \brief Verify dump in event parameters */
#define MM_VERIFY_FILL  3 /**< \brief Verify fill parameters */
#define MM_VERIFY_WID   4 /**< \brief Verify write interrupts disabled parameters */
/**\}*/

#define MM_MAX_MEM_TYPE_STR_LEN 11 /**< \brief Maximum memory type string length */

/************************************************************************
 * Type Definitions
 ************************************************************************/

/**
 *  \brief MM global data structure
 */
typedef struct
{
    MM_HkTlm_t           HkTlm;           /**< \brief Housekeeping telemetry packet */
    MM_SymLookupPacket_t SymLookupPacket; /**< \brief Symbol lookup telemetry packet */
    MM_PeekPacket_t      PeekPacket;      /**< \brief Memory peek telemetry packet */

    CFE_SB_PipeId_t CmdPipe; /**< \brief Command pipe ID */

    uint32 RunStatus; /**< \brief Application run status */

    size_t LoadBuffer[MM_INTERNAL_MAX_LOAD_DATA_SEG / 4]; /**< \brief Load file i/o buffer */
    size_t DumpBuffer[MM_INTERNAL_MAX_DUMP_DATA_SEG / 4]; /**< \brief Dump file i/o buffer */
    size_t FillBuffer[MM_INTERNAL_MAX_FILL_DATA_SEG / 4]; /**< \brief Fill memory buffer   */
} MM_AppData_t;

/** \brief Memory Manager application global */
extern MM_AppData_t MM_AppData;

/************************************************************************
 * Function prototypes
 ************************************************************************/

/**
 * \brief CFS Memory Manager (MM) application entry point
 *
 *  \par Description
 *       Memory Manager application entry point and main process loop.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 */
void MM_AppMain(void);

/**
 * \brief Initialize the memory manager CFS application
 *
 *  \par Description
 *       Memory manager application initialization routine. This
 *       function performs all the required startup steps to
 *       get the application registered with the cFE services so
 *       it can begin to receive command messages.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \return Execution status, see \ref CFEReturnCodes
 *  \retval #CFE_SUCCESS \copybrief CFE_SUCCESS
 */
CFE_Status_t MM_AppInit(void);

#endif