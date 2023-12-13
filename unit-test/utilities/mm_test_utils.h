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
 *   Test utility functions for MM unit tests
 */
#ifndef MM_TEST_UTILS_H
#define MM_TEST_UTILS_H

/************************************************************************
** Includes
*************************************************************************/
#include "mm_app.h"
#include "utstubs.h"

extern MM_AppData_t MM_AppData;

/*
 * Global context structures
 */
typedef struct
{
    uint16 EventID;
    uint16 EventType;
    char   Spec[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
} CFE_EVS_SendEvent_context_t;

typedef struct
{
    char Spec[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
} CFE_ES_WriteToSysLog_context_t;

extern CFE_EVS_SendEvent_context_t    context_CFE_EVS_SendEvent[];
extern CFE_ES_WriteToSysLog_context_t context_CFE_ES_WriteToSysLog;

/* Command buffer typedef for any handler */
typedef union
{
    CFE_SB_Buffer_t         Buf;
    MM_NoArgsCmd_t          NoArgsCmd;
    MM_PeekCmd_t            PeekCmd;
    MM_PokeCmd_t            PokeCmd;
    MM_LoadMemWIDCmd_t      LoadMemWIDCmd;
    MM_DumpInEventCmd_t     DumpInEventCmd;
    MM_LoadMemFromFileCmd_t LoadMemFromFileCmd;
    MM_DumpMemToFileCmd_t   DumpMemToFileCmd;
    MM_FillMemCmd_t         FillMemCmd;
    MM_LookupSymCmd_t       LookupSymCmd;
    MM_SymTblToFileCmd_t    SymTblToFileCmd;
    MM_EepromWriteEnaCmd_t  EepromWriteEnaCmd;
    MM_EepromWriteDisCmd_t  EepromWriteDisCmd;
} UT_CmdBuf_t;

extern UT_CmdBuf_t UT_CmdBuf;

/* Unit test ids */
#define MM_UT_OBJID_1 OS_ObjectIdFromInteger(1)
#define MM_UT_MID_1   CFE_SB_ValueToMsgId(1)

/*
 * Function Definitions
 */

void MM_Test_Setup(void);
void MM_Test_TearDown(void);

#endif
