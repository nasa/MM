/************************************************************************
 * NASA Docket No. GSC-18,923-1, and identified as “Core Flight
 * System (cFS) Memory Manager Application version 2.5.1”
 *
 * Copyright (c) 2021 United States Government as represented by the
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

/************************************************************************
 * Includes
 ************************************************************************/
#include "mm_msg.h"
#include "cfe.h"

/************************************************************************
 * Macro Definitions
 ************************************************************************/

/** \brief MM command pipe depth */
#define MM_CMD_PIPE_DEPTH (3 * CFE_PLATFORM_SB_DEFAULT_MSG_LIMIT)

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

/**
 * \brief Wakeup for MM
 *
 * \par Description
 *      Wakes up MM every 1 second for routine maintenance whether a
 *      message was received or not.
 */
#define MM_SB_TIMEOUT 1000

/************************************************************************
 * Type Definitions
 ************************************************************************/

/**
 *  \brief MM global data structure
 */
typedef struct
{
    MM_HkPacket_t HkPacket; /**< \brief Housekeeping telemetry packet */

    CFE_SB_PipeId_t CmdPipe; /**< \brief Command pipe ID */

    uint32 RunStatus; /**< \brief Application run status */

    uint32 LoadBuffer[MM_MAX_LOAD_DATA_SEG / 4]; /**< \brief Load file i/o buffer */
    uint32 DumpBuffer[MM_MAX_DUMP_DATA_SEG / 4]; /**< \brief Dump file i/o buffer */
    uint32 FillBuffer[MM_MAX_FILL_DATA_SEG / 4]; /**< \brief Fill memory buffer   */
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
int32 MM_AppInit(void);

/**
 * \brief Process a command pipe message
 *
 *  \par Description
 *       Processes a single software bus command pipe message. Checks
 *       the message and command IDs and calls the appropriate routine
 *       to handle the command.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param[in] msg Pointer to Software Bus buffer
 */
void MM_AppPipe(const CFE_SB_Buffer_t *msg);

/**
 * \brief Process housekeeping request
 *
 *  \par Description
 *       Processes an on-board housekeeping request message.
 *
 *  \par Assumptions, External Events, and Notes:
 *       This command does not affect the command execution counter
 *
 *  \param[in] msg Pointer to Software Bus buffer
 */
void MM_HousekeepingCmd(const CFE_SB_Buffer_t *msg);

/**
 * \brief Process noop command
 *
 *  \par Description
 *       Processes a noop ground command.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param[in] msg Pointer to Software Bus buffer
 *
 *  \sa #MM_NOOP_CC
 */
bool MM_NoopCmd(const CFE_SB_Buffer_t *msg);

/**
 * \brief Process reset counters command
 *
 *  \par Description
 *       Processes a reset counters ground command which will reset
 *       the memory manager commmand error and command execution counters
 *       to zero.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param[in] BufPtr Pointer to Software Bus buffer
 *
 *  \sa #MM_RESET_CC
 */
bool MM_ResetCmd(const CFE_SB_Buffer_t *BufPtr);

/**
 * \brief Process lookup symbol command
 *
 *  \par Description
 *       Processes a lookup symbol ground command which takes a
 *       symbol name and tries to resolve it to an address using the
 *       #OS_SymbolLookup OSAL function.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param[in] msg Pointer to Software Bus buffer
 *
 *  \sa #MM_LOOKUP_SYM_CC
 */
bool MM_LookupSymbolCmd(const CFE_SB_Buffer_t *msg);

/**
 * \brief Dump symbol table to file command
 *
 *  \par Description
 *       Processes a dump symbol table to file ground command which calls
 *       the #OS_SymbolTableDump OSAL function using the specified dump
 *       file name.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param[in] msg Pointer to Software Bus buffer
 *
 *  \sa #MM_SYMTBL_TO_FILE_CC
 */
bool MM_SymTblToFileCmd(const CFE_SB_Buffer_t *msg);

/**
 * \brief Write-enable EEPROM command
 *
 *  \par Description
 *       Processes an EEPROM write enable ground command which calls
 *       the #CFE_PSP_EepromWriteEnable cFE function using the specified
 *       bank number.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param[in] msg Pointer to Software Bus buffer
 *
 *  \sa #MM_ENABLE_EEPROM_WRITE_CC
 */
bool MM_EepromWriteEnaCmd(const CFE_SB_Buffer_t *msg);

/**
 * \brief Write-disable EEPROM command
 *
 *  \par Description
 *       Processes an EEPROM write disable ground command which calls
 *       the #CFE_PSP_EepromWriteDisable cFE function using the specified
 *       bank number.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param[in] msg Pointer to Software Bus buffer
 *
 *  \sa #MM_DISABLE_EEPROM_WRITE_CC
 */
bool MM_EepromWriteDisCmd(const CFE_SB_Buffer_t *msg);

#endif
