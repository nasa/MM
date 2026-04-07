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

/************************************************************************
** Includes
*************************************************************************/
#include "mm_test_utils.h"
#include "mm_app.h"
#include "mm_filedefs.h"
#include "mm_interface_cfg.h"
#include "mm_utils.h"

/************************************************************************
** UT Includes
*************************************************************************/
#include "utassert.h"
#include "utstubs.h"
#include "uttest.h"

/* =========== */
/* Global Data */
/* =========== */

CFE_EVS_SendEvent_context_t    context_CFE_EVS_SendEvent[3];
CFE_ES_WriteToSysLog_context_t context_CFE_ES_WriteToSysLog;

/* ======== */
/* Handlers */
/* ======== */

void UT_Handler_CFE_EVS_SendEvent(void *UserObj, UT_EntryKey_t FuncKey, const UT_StubContext_t *Context, va_list va)
{
    uint16 CallCount;
    uint16 idx;

    CallCount = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    if (CallCount > (sizeof(context_CFE_EVS_SendEvent) / sizeof(context_CFE_EVS_SendEvent[0])))
    {
        UtAssert_Failed("CFE_EVS_SendEvent UT depth %u exceeded: %u, increase "
                        "UT_MAX_SENDEVENT_DEPTH",
                        UT_MAX_SENDEVENT_DEPTH,
                        CallCount);
    }
    else
    {
        idx                                      = CallCount - 1;
        context_CFE_EVS_SendEvent[idx].EventID   = UT_Hook_GetArgValueByName(Context, "EventID", uint16);
        context_CFE_EVS_SendEvent[idx].EventType = UT_Hook_GetArgValueByName(Context, "EventType", uint16);

        strncpy(context_CFE_EVS_SendEvent[idx].Spec,
                UT_Hook_GetArgValueByName(Context, "Spec", const char *),
                CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);
        context_CFE_EVS_SendEvent[idx].Spec[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH - 1] = '\0';
    }
}

void UT_Handler_CFE_ES_WriteToSysLog(void *UserObj, UT_EntryKey_t FuncKey, const UT_StubContext_t *Context, va_list va)
{
    strncpy(context_CFE_ES_WriteToSysLog.Spec,
            UT_Hook_GetArgValueByName(Context, "SpecStringPtr", const char *),
            CFE_MISSION_EVS_MAX_MESSAGE_LENGTH - 1);
    context_CFE_ES_WriteToSysLog.Spec[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH - 1] = '\0';
}

void UT_Handler_MM_ResolveSymAddr(void *UserObj, UT_EntryKey_t FuncKey, const UT_StubContext_t *Context)
{
    cpuaddr *ResolvedAddress;

    ResolvedAddress  = UT_Hook_GetArgValueByName(Context, "ResolvedAddr", cpuaddr *);
    *ResolvedAddress = *((cpuaddr *)UserObj);
    ;
}

void UT_Handler_MM_WriteFileHeaders(void *UserObj, UT_EntryKey_t FuncKey, const UT_StubContext_t *Context)
{
    MM_LoadDumpFileHeader_t *MMFileHeaderPtr;

    MMFileHeaderPtr  = UT_Hook_GetArgValueByName(Context, "MMHeader", MM_LoadDumpFileHeader_t *);
    *MMFileHeaderPtr = *((MM_LoadDumpFileHeader_t *)UserObj);
}

void UT_Handler_MM_ReadFileHeaders(void *UserObj, UT_EntryKey_t FuncKey, const UT_StubContext_t *Context)
{
    MM_LoadDumpFileHeader_t *MMFileHeaderPtr;

    MMFileHeaderPtr  = UT_Hook_GetArgValueByName(Context, "MMHeader", MM_LoadDumpFileHeader_t *);
    *MMFileHeaderPtr = *((MM_LoadDumpFileHeader_t *)UserObj);
}

void UT_Handler_MM_ComputeCRCFromFile(void *UserObj, UT_EntryKey_t FuncKey, const UT_StubContext_t *Context)
{
    uint32 *CrcPtr;

    CrcPtr  = UT_Hook_GetArgValueByName(Context, "CrcPtr", uint32 *);
    *CrcPtr = *((uint32 *)UserObj);
}

void UT_Handler_CFE_SB_MessageStringGet(void *UserObj, UT_EntryKey_t FuncKey, const UT_StubContext_t *Context)
{
    char *FileName;

    FileName = UT_Hook_GetArgValueByName(Context, "DestStringPtr", char *);

    if (sizeof(FileName) > OS_MAX_FILE_NAME)
    {
        printf("Argument 1 (DestStringPtr) of CFE_SB_MessageStringGet exceeded the "
               "limit of %d\n",
               OS_MAX_FILE_NAME);
    }
    else
    {
        strncpy(FileName, UserObj, OS_MAX_FILE_NAME);
    }
}

/* ==================== */
/* Function Definitions */
/* ==================== */

void MM_Test_Verify_Event(uint8 IssuedOrder, uint16 EventId, uint16 EventType, const char *EventText)
{
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[IssuedOrder].EventID, EventId);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[IssuedOrder].EventType, EventType);

    UtAssert_StrnCmp(EventText,
                     context_CFE_EVS_SendEvent[IssuedOrder].Spec,
                     CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
                     "Event string expected: '%s'\nEvent string received: '%s'",
                     EventText,
                     context_CFE_EVS_SendEvent[IssuedOrder].Spec);
}

void MM_Test_Setup(void)
{
    /* initialize test environment to default state for every test */
    UT_ResetState(0);

    memset(&MM_AppData, 0, sizeof(MM_AppData));
    memset(context_CFE_EVS_SendEvent, 0, sizeof(context_CFE_EVS_SendEvent));
    memset(&context_CFE_ES_WriteToSysLog, 0, sizeof(context_CFE_ES_WriteToSysLog));

    /* Register custom handlers */
    UT_SetVaHandlerFunction(UT_KEY(CFE_EVS_SendEvent), UT_Handler_CFE_EVS_SendEvent, NULL);
    UT_SetVaHandlerFunction(UT_KEY(CFE_ES_WriteToSysLog), UT_Handler_CFE_ES_WriteToSysLog, NULL);
}

void MM_Test_TearDown(void)
{ /* cleanup test environment */
}
