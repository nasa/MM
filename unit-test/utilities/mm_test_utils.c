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

/************************************************************************
** Includes
*************************************************************************/
#include "mm_test_utils.h"
#include "mm_app.h"

/************************************************************************
** UT Includes
*************************************************************************/
#include "uttest.h"
#include "utassert.h"
#include "utstubs.h"

#define UT_MAX_SENDEVENT_DEPTH 4
CFE_EVS_SendEvent_context_t    context_CFE_EVS_SendEvent[UT_MAX_SENDEVENT_DEPTH];
CFE_ES_WriteToSysLog_context_t context_CFE_ES_WriteToSysLog;

UT_CmdBuf_t UT_CmdBuf;

/*
 * Function Definitions
 */
void UT_Handler_CFE_EVS_SendEvent(void *UserObj, UT_EntryKey_t FuncKey, const UT_StubContext_t *Context, va_list va)
{
    uint16 CallCount;
    uint16 idx;

    CallCount = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    if (CallCount > (sizeof(context_CFE_EVS_SendEvent) / sizeof(context_CFE_EVS_SendEvent[0])))
    {
        UtAssert_Failed("CFE_EVS_SendEvent UT depth %u exceeded: %u, increase UT_MAX_SENDEVENT_DEPTH",
                        UT_MAX_SENDEVENT_DEPTH, CallCount);
    }
    else
    {
        idx                                      = CallCount - 1;
        context_CFE_EVS_SendEvent[idx].EventID   = UT_Hook_GetArgValueByName(Context, "EventID", uint16);
        context_CFE_EVS_SendEvent[idx].EventType = UT_Hook_GetArgValueByName(Context, "EventType", uint16);

        strncpy(context_CFE_EVS_SendEvent[idx].Spec, UT_Hook_GetArgValueByName(Context, "Spec", const char *),
                CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);
        context_CFE_EVS_SendEvent[idx].Spec[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH - 1] = '\0';
    }
}

void UT_Handler_CFE_ES_WriteToSysLog(void *UserObj, UT_EntryKey_t FuncKey, const UT_StubContext_t *Context, va_list va)
{
    strncpy(context_CFE_ES_WriteToSysLog.Spec, UT_Hook_GetArgValueByName(Context, "SpecStringPtr", const char *),
            CFE_MISSION_EVS_MAX_MESSAGE_LENGTH - 1);
    context_CFE_ES_WriteToSysLog.Spec[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH - 1] = '\0';
}

void MM_Test_Setup(void)
{
    /* initialize test environment to default state for every test */
    UT_ResetState(0);

    memset(&MM_AppData, 0, sizeof(MM_AppData_t));
    memset(context_CFE_EVS_SendEvent, 0, sizeof(context_CFE_EVS_SendEvent));
    memset(&context_CFE_ES_WriteToSysLog, 0, sizeof(context_CFE_ES_WriteToSysLog));
    memset(&UT_CmdBuf, 0, sizeof(UT_CmdBuf));

    /* Register custom handlers */
    UT_SetVaHandlerFunction(UT_KEY(CFE_EVS_SendEvent), UT_Handler_CFE_EVS_SendEvent, NULL);
    UT_SetVaHandlerFunction(UT_KEY(CFE_ES_WriteToSysLog), UT_Handler_CFE_ES_WriteToSysLog, NULL);
}

void MM_Test_TearDown(void)
{
    /* cleanup test environment */
}

/*
 * Placeholder stubs until PSP UT implementation is available
 */

int32 CFE_PSP_EepromWriteEnable(uint32 Bank)
{
    int32 status;
    status = UT_DEFAULT_IMPL(CFE_PSP_EepromWriteEnable);
    return status;
}

int32 CFE_PSP_EepromWriteDisable(uint32 Bank)
{
    int32 status;
    status = UT_DEFAULT_IMPL(CFE_PSP_EepromWriteDisable);
    return status;
}

int32 CFE_PSP_EepromWrite32(cpuaddr MemoryAddress, uint32 uint32Value)
{
    int32 status;
    status = UT_DEFAULT_IMPL(CFE_PSP_EepromWrite32);
    return status;
}

int32 CFE_PSP_EepromWrite16(cpuaddr MemoryAddress, uint16 uint16Value)
{
    int32 status;
    status = UT_DEFAULT_IMPL(CFE_PSP_EepromWrite16);
    return status;
}

int32 CFE_PSP_EepromWrite8(cpuaddr MemoryAddress, uint8 ByteValue)
{
    int32 status;
    status = UT_DEFAULT_IMPL(CFE_PSP_EepromWrite8);
    return status;
}

int32 CFE_PSP_MemWrite8(cpuaddr MemoryAddress, uint8 ByteValue)
{
    int32 status;
    status = UT_DEFAULT_IMPL(CFE_PSP_MemWrite8);
    return status;
}

int32 CFE_PSP_MemRead16(cpuaddr MemoryAddress, uint16 *uint16Value)
{
    int32 status;
    status = UT_DEFAULT_IMPL(CFE_PSP_MemRead16);
    return status;
}

int32 CFE_PSP_MemWrite16(cpuaddr MemoryAddress, uint16 uint16Value)
{
    int32 status;
    status = UT_DEFAULT_IMPL(CFE_PSP_MemWrite16);
    return status;
}

int32 CFE_PSP_MemRead32(cpuaddr MemoryAddress, uint32 *uint32Value)
{
    int32 status;
    status = UT_DEFAULT_IMPL(CFE_PSP_MemRead32);
    return status;
}

int32 CFE_PSP_MemWrite32(cpuaddr MemoryAddress, uint32 uint32Value)
{
    int32 status;
    status = UT_DEFAULT_IMPL(CFE_PSP_MemWrite32);
    return status;
}
