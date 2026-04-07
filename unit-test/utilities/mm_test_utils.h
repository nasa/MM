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
 *   Test utility functions for MM unit tests
 */
#ifndef MM_TEST_UTILS_H
#define MM_TEST_UTILS_H

/************************************************************************
** Includes
*************************************************************************/
#include "mm_app.h"
#include "mm_filedefs.h"
#include "utstubs.h"

/* ====== */
/* Macros */
/* ====== */

#define ADD_TEST(test)         UtTest_Add(test, MM_Test_Setup, MM_Test_TearDown, #test)
#define MM_UT_OBJID_1          OS_ObjectIdFromInteger(1)
#define MM_UT_MID_1            CFE_SB_ValueToMsgId(1)
#define UT_MAX_SENDEVENT_DEPTH 4

/* ======== */
/* Typedefs */
/* ======== */

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

/* =========== */
/* Global Data */
/* =========== */

extern CFE_EVS_SendEvent_context_t    context_CFE_EVS_SendEvent[3];
extern CFE_ES_WriteToSysLog_context_t context_CFE_ES_WriteToSysLog;

/* ======== */
/* Handlers */
/* ======== */

void UT_Handler_CFE_EVS_SendEvent(void *UserObj, UT_EntryKey_t FuncKey, const UT_StubContext_t *Context, va_list va);
void UT_Handler_CFE_ES_WriteToSysLog(void *UserObj, UT_EntryKey_t FuncKey, const UT_StubContext_t *Context, va_list va);
void UT_Handler_MM_ResolveSymAddr(void *UserObj, UT_EntryKey_t FuncKey, const UT_StubContext_t *Context);
void UT_Handler_MM_WriteFileHeaders(void *UserObj, UT_EntryKey_t FuncKey, const UT_StubContext_t *Context);
void UT_Handler_MM_ReadFileHeaders(void *UserObj, UT_EntryKey_t FuncKey, const UT_StubContext_t *Context);
void UT_Handler_MM_ComputeCRCFromFile(void *UserObj, UT_EntryKey_t FuncKey, const UT_StubContext_t *Context);
void UT_Handler_CFE_SB_MessageStringGet(void *UserObj, UT_EntryKey_t FuncKey, const UT_StubContext_t *Context);

/* =================== */
/* Function Prototypes */
/* =================== */

void MM_Test_Verify_Event(uint8 IssuedOrder, uint16 EventId, uint16 EventType, const char *EventText);
void MM_Test_Setup(void);
void MM_Test_TearDown(void);

#endif
