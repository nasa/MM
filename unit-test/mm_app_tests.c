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
 *   Unit tests for mm_app.c
 */

/************************************************************************
** Includes
*************************************************************************/
#include "mm_app.h"
#include "mm_msg.h"
#include "mm_load.h"
#include "mm_dump.h"
#include "mm_msgdefs.h"
#include "mm_msgids.h"
#include "mm_events.h"
#include "mm_version.h"
#include "mm_utils.h"
#include "mm_test_utils.h"

/************************************************************************
** UT Includes
*************************************************************************/
#include "uttest.h"
#include "utassert.h"
#include "utstubs.h"

#include <unistd.h>
#include <stdlib.h>
#include "cfe.h"
#include "cfe_msgids.h"

/* mm_app_tests globals */
uint8 call_count_CFE_EVS_SendEvent;

/*
 * Function Definitions
 */

CFE_Status_t MM_APP_TEST_CFE_SB_RcvMsgHook(void *UserObj, int32 StubRetcode, uint32 CallCount,
                                           const UT_StubContext_t *Context)
{
    return CFE_SUCCESS;
}

int32 MM_APP_TEST_CFE_ES_ExitAppHook(void *UserObj, int32 StubRetcode, uint32 CallCount,
                                     const UT_StubContext_t *Context)
{
    MM_AppData.HkPacket.Payload.CmdCounter++;

    return 0;
}

void MM_AppMain_Test_Nominal(void)
{
    CFE_SB_MsgId_t    TestMsgId   = CFE_SB_ValueToMsgId(MM_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode     = MM_NOOP_CC;
    size_t            forced_Size = sizeof(UT_CmdBuf.NoArgsCmd);
    CFE_SB_Buffer_t   Buf;
    CFE_SB_Buffer_t * BufPtr = &Buf;

    /* Set to exit loop after first run */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RunLoop), 1, true);

    /* Set to prevent call to CFE_SB_RcvMsg from returning an error */
    UT_SetHookFunction(UT_KEY(CFE_SB_ReceiveBuffer), MM_APP_TEST_CFE_SB_RcvMsgHook, NULL);
    /* Causes check for non-null buffer pointer to succeed */
    UT_SetDataBuffer(UT_KEY(CFE_SB_ReceiveBuffer), &BufPtr, sizeof(BufPtr), false);

    /* Set to prevent segmentation fault */
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    /* Execute the function being tested */
    MM_AppMain();

    /* Verify results */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 1);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);

    /* Generates 1 event message we don't care about in this test */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 2, "CFE_EVS_SendEvent was called %u time(s), expected 2",
                  call_count_CFE_EVS_SendEvent);

    uint8 call_count_CFE_ES_ExitApp = UT_GetStubCount(UT_KEY(CFE_ES_ExitApp));
    UtAssert_INT32_EQ(call_count_CFE_ES_ExitApp, 1);
}

void MM_AppMain_Test_AppInitError(void)
{
    CFE_SB_MsgId_t    TestMsgId   = CFE_SB_ValueToMsgId(MM_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode     = MM_NOOP_CC;
    size_t            forced_Size = 1;

    /* Set to exit loop after first run */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RunLoop), 1, true);

    /* Set to prevent call to CFE_SB_RcvMsg from returning an error */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_ReceiveBuffer), 1, CFE_SUCCESS);

    /* Set to prevent segmentation fault */
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);

    /* Set to satisfy condition "Status != CFE_SUCCESS" */
    UT_SetDeferredRetcode(UT_KEY(CFE_EVS_Register), 1, -1);

    /* Execute the function being tested */
    MM_AppMain();

    /* Verify results */
    UtAssert_True(MM_AppData.RunStatus == CFE_ES_RunStatus_APP_ERROR,
                  "MM_AppData.RunStatus == CFE_ES_RunStatus_APP_ERROR");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void MM_AppMain_Test_SBError(void)
{
    int32          strCmpResult;
    char           ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    size_t         forced_Size  = 1;
    CFE_SB_MsgId_t forced_MsgID = MM_UT_MID_1;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "SB Pipe Read Error, App will exit. RC = 0x%%08X");

    /* Set to exit loop after first run */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RunLoop), 1, true);

    /* Set to generate error message MM_PIPE_ERR_EID */
    UT_SetDefaultReturnValue(UT_KEY(CFE_SB_ReceiveBuffer), -1);

    /* Set to prevent segmentation fault */
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);

    /* Used to verify completion of MM_AppMain by incrementing MM_AppData.HkPacket.Payload.CmdCounter. */
    UT_SetHookFunction(UT_KEY(CFE_ES_ExitApp), MM_APP_TEST_CFE_ES_ExitAppHook, NULL);

    /* Execute the function being tested */
    MM_AppMain();

    /* Verify results */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 1);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, MM_PIPE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[1].Spec);

    /* Generates 1 event message we don't care about in this test */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 2, "CFE_EVS_SendEvent was called %u time(s), expected 2",
                  call_count_CFE_EVS_SendEvent);
}

void MM_AppMain_Test_SBTimeout(void)
{
    size_t         forced_Size  = 1;
    CFE_SB_MsgId_t forced_MsgID = MM_UT_MID_1;

    /* Set to exit loop after first run */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RunLoop), 1, true);

    /* Set to generate error message MM_PIPE_ERR_EID */
    UT_SetDefaultReturnValue(UT_KEY(CFE_SB_ReceiveBuffer), CFE_SB_TIME_OUT);

    /* Set to prevent segmentation fault */
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);

    /* Used to verify completion of MM_AppMain by incrementing MM_AppData.HkPacket.Payload.CmdCounter. */
    UT_SetHookFunction(UT_KEY(CFE_ES_ExitApp), MM_APP_TEST_CFE_ES_ExitAppHook, NULL);

    /* Execute the function being tested */
    MM_AppMain();

    /* Verify results */
    /* Generates 1 event message we don't care about in this test */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void MM_AppInit_Test_Nominal(void)
{
    CFE_Status_t Result;
    int32        strCmpResult;
    char         ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "MM Initialized. Version %%d.%%d.%%d.%%d");

    /* Initialize all elements to 1, in order verify that elements initialized to 0 by MM_AppInit are actually
     * initialized */
    memset(&MM_AppData, 1, sizeof(MM_AppData));

    /* Execute the function being tested */
    Result = MM_AppInit();

    /* Verify results */
    UtAssert_True(Result == CFE_SUCCESS, "Result == CFE_SUCCESS");

    UtAssert_True(MM_AppData.RunStatus == CFE_ES_RunStatus_APP_RUN, "MM_AppData.RunStatus == CFE_ES_RunStatus_APP_RUN");
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_INIT_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void MM_AppInit_Test_EVSRegisterError(void)
{
    CFE_Status_t Result;
    int32        strCmpResult;
    char         ExpectedSysLogString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedSysLogString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "MM App: Error Registering For Event Services, RC = 0x%%08X\n");

    /* Initialize all elements to 1, in order verify that elements initialized to 0 by MM_AppInit are actually
     * initialized */
    memset(&MM_AppData, 1, sizeof(MM_AppData));

    /* Set to generate system log message "MM App: Error Registering For Event Services " */
    UT_SetDeferredRetcode(UT_KEY(CFE_EVS_Register), 1, -1);

    /* Execute the function being tested */
    Result = MM_AppInit();

    /* Verify results */
    UtAssert_True(Result == -1, "Result == -1");

    UtAssert_True(MM_AppData.RunStatus == CFE_ES_RunStatus_APP_RUN, "MM_AppData.RunStatus == CFE_ES_RunStatus_APP_RUN");
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);

    strCmpResult = strncmp(ExpectedSysLogString, context_CFE_ES_WriteToSysLog.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Sys Log string matched expected result, '%s'", context_CFE_ES_WriteToSysLog.Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void MM_AppInit_Test_SBCreatePipeError(void)
{
    CFE_Status_t Result;
    int32        strCmpResult;
    char         ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Error Creating SB Pipe, RC = 0x%%08X");

    /* Initialize all elements to 1, in order verify that elements initialized to 0 by MM_AppInit are actually
     * initialized */
    memset(&MM_AppData, 1, sizeof(MM_AppData));

    /* Set to generate system log message "MM App: Error Creating SB Pipe" */
    UT_SetDefaultReturnValue(UT_KEY(CFE_SB_CreatePipe), -1);

    /* Execute the function being tested */
    Result = MM_AppInit();

    /* Verify results */
    UtAssert_True(Result == -1, "Result == -1");

    UtAssert_True(MM_AppData.RunStatus == CFE_ES_RunStatus_APP_RUN, "MM_AppData.RunStatus == CFE_ES_RunStatus_APP_RUN");
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Sys Log string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void MM_AppInit_Test_SBSubscribeHKError(void)
{
    CFE_Status_t Result;
    int32        strCmpResult;
    char         ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Error Subscribing to HK Request, RC = 0x%%08X");

    /* Initialize all elements to 1, in order verify that elements initialized to 0 by MM_AppInit are actually
     * initialized */
    memset(&MM_AppData, 1, sizeof(MM_AppData));

    /* Set to generate system log message "MM App: Error Subscribing to HK Request" */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_Subscribe), 1, -1);

    /* Execute the function being tested */
    Result = MM_AppInit();

    /* Verify results */
    UtAssert_True(Result == -1, "Result == -1");

    UtAssert_True(MM_AppData.RunStatus == CFE_ES_RunStatus_APP_RUN, "MM_AppData.RunStatus == CFE_ES_RunStatus_APP_RUN");
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Sys Log string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void MM_AppInit_Test_SBSubscribeMMError(void)
{
    CFE_Status_t Result;
    int32        strCmpResult;
    char         ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Error Subscribing to MM Command, RC = 0x%%08X");

    /* Initialize all elements to 1, in order verify that elements initialized to 0 by MM_AppInit are actually
     * initialized */
    memset(&MM_AppData, 1, sizeof(MM_AppData));

    /* Set to generate system log message "MM App: Error Subscribing to MM Command" */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_Subscribe), 2, -1);

    /* Execute the function being tested */
    Result = MM_AppInit();

    /* Verify results */
    UtAssert_True(Result == -1, "Result == -1");

    UtAssert_True(MM_AppData.RunStatus == CFE_ES_RunStatus_APP_RUN, "MM_AppData.RunStatus == CFE_ES_RunStatus_APP_RUN");
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Sys Log string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void MM_AppPipe_Test_SendHKSuccess(void)
{
    CFE_SB_MsgId_t TestMsgId = CFE_SB_ValueToMsgId(MM_SEND_HK_MID);

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    /* Execute the function being tested */
    MM_AppPipe(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void MM_AppPipe_Test_SendHKFail(void)
{
    CFE_SB_MsgId_t TestMsgId = CFE_SB_ValueToMsgId(MM_SEND_HK_MID);

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), false);

    /* Execute the function being tested */
    MM_AppPipe(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void MM_AppPipe_Test_NoopSuccess(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = MM_NOOP_CC;
    size_t            MsgSize   = sizeof(UT_CmdBuf.NoArgsCmd);
    int32             strCmpResult;
    char              ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "No-op command. Version %%d.%%d.%%d.%%d");

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    /* Execute the function being tested */
    MM_AppPipe(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(MM_AppData.HkPacket.Payload.LastAction == MM_NOOP, "MM_AppData.HkPacket.Payload.LastAction == MM_NOOP");
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 1);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);

    /* Note: this event message is generated in subfunction MM_NoopCmd.Payload.  It is checked here to verify that the
     * subfunction has been reached. */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_NOOP_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void MM_AppPipe_Test_NoopFail(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = MM_NOOP_CC;
    size_t            MsgSize   = sizeof(UT_CmdBuf.NoArgsCmd);
    char              ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "No-op command. Version %%d.%%d.%%d.%%d");

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), false);

    /* Execute the function being tested */
    MM_AppPipe(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 1);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void MM_AppPipe_Test_ResetSuccess(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = MM_RESET_CC;
    size_t            MsgSize   = sizeof(UT_CmdBuf.NoArgsCmd);
    int32             strCmpResult;
    char              ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Reset counters command received");

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    /* Execute the function being tested */
    MM_AppPipe(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(MM_AppData.HkPacket.Payload.LastAction == MM_RESET, "MM_AppData.HkPacket.Payload.LastAction == MM_RESET");
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);

    /* Note: this event message is generated in subfunction MM_ResetCmd.Payload.  It is checked here to verify that the
     * subfunction has been reached. */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_RESET_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void MM_AppPipe_Test_ResetFail(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = MM_RESET_CC;
    size_t            MsgSize   = sizeof(UT_CmdBuf.NoArgsCmd);
    char              ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Reset counters command received");

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), false);

    /* Execute the function being tested */
    MM_AppPipe(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void MM_AppPipe_Test_PeekSuccess(void)
{
    CFE_SB_MsgId_t    TestMsgId             = CFE_SB_ValueToMsgId(MM_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode               = MM_PEEK_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, true);
    UT_SetDefaultReturnValue(UT_KEY(MM_PeekCmd), true);

    /* Execute the function being tested */
    MM_AppPipe(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 1);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(MM_PeekCmd, 1);
}

void MM_AppPipe_Test_PeekFail(void)
{
    CFE_SB_MsgId_t    TestMsgId             = CFE_SB_ValueToMsgId(MM_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode               = MM_PEEK_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, false);

    /* Execute the function being tested */
    MM_AppPipe(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 1);

    UtAssert_STUB_COUNT(MM_PeekCmd, 0);
}

void MM_AppPipe_Test_PokeSuccess(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = MM_POKE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, true);
    UT_SetDefaultReturnValue(UT_KEY(MM_PokeCmd), true);

    /* Execute the function being tested */
    MM_AppPipe(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 1);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(MM_PokeCmd, 1);
}

void MM_AppPipe_Test_PokeFail(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = MM_POKE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, false);
    UT_SetDefaultReturnValue(UT_KEY(MM_PokeCmd), false);

    /* Execute the function being tested */
    MM_AppPipe(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 1);

    UtAssert_STUB_COUNT(MM_PokeCmd, 0);
}

void MM_AppPipe_Test_LoadMemWIDSuccess(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = MM_LOAD_MEM_WID_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, true);
    UT_SetDefaultReturnValue(UT_KEY(MM_LoadMemWIDCmd), true);

    /* Execute the function being tested */
    MM_AppPipe(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 1);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(MM_LoadMemWIDCmd, 1);
}

void MM_AppPipe_Test_LoadMemWIDFail(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = MM_LOAD_MEM_WID_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, false);
    UT_SetDefaultReturnValue(UT_KEY(MM_LoadMemWIDCmd), false);

    /* Execute the function being tested */
    MM_AppPipe(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 1);

    UtAssert_STUB_COUNT(MM_LoadMemWIDCmd, 0);
}

void MM_AppPipe_Test_LoadMemFromFileSuccess(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = MM_LOAD_MEM_FROM_FILE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, true);
    UT_SetDefaultReturnValue(UT_KEY(MM_LoadMemFromFileCmd), true);

    /* Execute the function being tested */
    MM_AppPipe(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 1);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(MM_LoadMemFromFileCmd, 1);
}

void MM_AppPipe_Test_LoadMemFromFileFail(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = MM_LOAD_MEM_FROM_FILE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, false);
    UT_SetDefaultReturnValue(UT_KEY(MM_LoadMemFromFileCmd), false);

    /* Execute the function being tested */
    MM_AppPipe(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 1);

    UtAssert_STUB_COUNT(MM_LoadMemFromFileCmd, 0);
}

void MM_AppPipe_Test_DumpMemToFileSuccess(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = MM_DUMP_MEM_TO_FILE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, true);
    UT_SetDefaultReturnValue(UT_KEY(MM_DumpMemToFileCmd), true);

    /* Execute the function being tested */
    MM_AppPipe(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 1);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(MM_DumpMemToFileCmd, 1);
}

void MM_AppPipe_Test_DumpMemToFileFail(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = MM_DUMP_MEM_TO_FILE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, false);
    UT_SetDefaultReturnValue(UT_KEY(MM_DumpMemToFileCmd), false);

    /* Execute the function being tested */
    MM_AppPipe(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 1);

    UtAssert_STUB_COUNT(MM_DumpMemToFileCmd, 0);
}

void MM_AppPipe_Test_DumpInEventSuccess(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = MM_DUMP_IN_EVENT_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, true);
    UT_SetDefaultReturnValue(UT_KEY(MM_DumpInEventCmd), true);

    /* Execute the function being tested */
    MM_AppPipe(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 1);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(MM_DumpInEventCmd, 1);
}

void MM_AppPipe_Test_DumpInEventFail(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = MM_DUMP_IN_EVENT_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, false);
    UT_SetDefaultReturnValue(UT_KEY(MM_DumpInEventCmd), false);

    /* Execute the function being tested */
    MM_AppPipe(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 1);

    UtAssert_STUB_COUNT(MM_DumpInEventCmd, 0);
}

void MM_AppPipe_Test_FillMemSuccess(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = MM_FILL_MEM_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, true);
    UT_SetDefaultReturnValue(UT_KEY(MM_FillMemCmd), true);

    /* Execute the function being tested */
    MM_AppPipe(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 1);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(MM_FillMemCmd, 1);
}

void MM_AppPipe_Test_FillMemFail(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = MM_FILL_MEM_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, false);
    UT_SetDefaultReturnValue(UT_KEY(MM_FillMemCmd), false);

    /* Execute the function being tested */
    MM_AppPipe(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 1);

    UtAssert_STUB_COUNT(MM_FillMemCmd, 0);
}

void MM_AppPipe_Test_LookupSymbolSuccess(void)
{
    CFE_SB_MsgId_t    TestMsgId    = CFE_SB_ValueToMsgId(MM_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode      = MM_LOOKUP_SYM_CC;
    size_t            MsgSize      = sizeof(UT_CmdBuf.LookupSymCmd);
    cpuaddr           ResolvedAddr = 0;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    strncpy(UT_CmdBuf.LookupSymCmd.Payload.SymName, "name", sizeof(UT_CmdBuf.LookupSymCmd.Payload.SymName) - 1);
    UT_SetDataBuffer(UT_KEY(OS_SymbolLookup), &ResolvedAddr, sizeof(ResolvedAddr), false);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    /* Execute the function being tested */
    MM_AppPipe(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 1);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);

    /* Generates 1 event message in subfunction MM_LookupSymbolCmd.Payload.  Event message count = 1 verifies that the
     * subfunction has been reached. */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void MM_AppPipe_Test_LookupSymbolFail(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = MM_LOOKUP_SYM_CC;
    size_t            MsgSize   = sizeof(UT_CmdBuf.LookupSymCmd);

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), false);

    /* Execute the function being tested */
    MM_AppPipe(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 1);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void MM_AppPipe_Test_SymTblToFileSuccess(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = MM_SYM_TBL_TO_FILE_CC;
    size_t            MsgSize   = sizeof(UT_CmdBuf.SymTblToFileCmd);

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    strncpy(UT_CmdBuf.SymTblToFileCmd.Payload.FileName, "name", sizeof(UT_CmdBuf.SymTblToFileCmd.Payload.FileName) - 1);

    /* Set to satisfy condition "OS_Status == OS_SUCCESS" */
    UT_SetDeferredRetcode(UT_KEY(OS_SymbolTableDump), 1, CFE_SUCCESS);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    /* Execute the function being tested */
    MM_AppPipe(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 1);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);

    /* Generates 1 event message in subfunction MM_SymTblToFileCmd.Payload.  Event message count = 1 verifies that the
     * subfunction has been reached. */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void MM_AppPipe_Test_SymTblToFileFail(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = MM_SYM_TBL_TO_FILE_CC;
    size_t            MsgSize   = sizeof(UT_CmdBuf.SymTblToFileCmd);

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), false);

    /* Execute the function being tested */
    MM_AppPipe(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 1);

    /* Generates 1 event message in subfunction MM_SymTblToFileCmd.Payload.  Event message count = 1 verifies that the
     * subfunction has been reached. */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void MM_AppPipe_Test_EnableEepromWriteSuccess(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = MM_ENABLE_EEPROM_WRITE_CC;
    size_t            MsgSize   = sizeof(UT_CmdBuf.EepromWriteEnaCmd);

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    /* Execute the function being tested */
    MM_AppPipe(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 1);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);

    /* Generates 1 event message in subfunction MM_EepromWriteEnaCmd.Payload.  Event message count = 1 verifies that the
     * subfunction has been reached. */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void MM_AppPipe_Test_EnableEepromWriteFail(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = MM_ENABLE_EEPROM_WRITE_CC;
    size_t            MsgSize   = sizeof(UT_CmdBuf.EepromWriteEnaCmd);

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), false);

    /* Execute the function being tested */
    MM_AppPipe(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 1);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void MM_AppPipe_Test_DisableEepromWriteSuccess(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = MM_DISABLE_EEPROM_WRITE_CC;
    size_t            MsgSize   = sizeof(UT_CmdBuf.EepromWriteDisCmd);

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    /* Execute the function being tested */
    MM_AppPipe(&UT_CmdBuf.Buf);

    /* Verify results */

    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 1);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);

    /* Generates 1 event message in subfunction MM_EepromWriteDisCmd.Payload.  Event message count = 1 verifies that the
     * subfunction has been reached. */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void MM_AppPipe_Test_DisableEepromWriteFail(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = MM_DISABLE_EEPROM_WRITE_CC;
    size_t            MsgSize   = sizeof(UT_CmdBuf.EepromWriteDisCmd);

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), false);

    /* Execute the function being tested */
    MM_AppPipe(&UT_CmdBuf.Buf);

    /* Verify results */

    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 1);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void MM_AppPipe_Test_InvalidCommandCode(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = 99;
    size_t            MsgSize   = sizeof(UT_CmdBuf.NoArgsCmd);
    int32             strCmpResult;
    char              ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid ground command code: ID = 0x%%08lX, CC = %%d");

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    /* Execute the function being tested */
    MM_AppPipe(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_CC1_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    /* Generates 1 event message we don't care about in this test */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void MM_AppPipe_Test_InvalidCommandPipeMessageID(void)
{
    CFE_SB_MsgId_t    TestMsgId = MM_UT_MID_1;
    CFE_MSG_FcnCode_t FcnCode   = 0;
    size_t            MsgSize   = sizeof(UT_CmdBuf.NoArgsCmd);
    int32             strCmpResult;
    char              ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Invalid command pipe message ID: 0x%%08lX");

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    /* Execute the function being tested */
    MM_AppPipe(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_MID_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void MM_HousekeepingCmd_Test(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(MM_HK_TLM_MID);
    CFE_MSG_FcnCode_t FcnCode   = 0;
    size_t            MsgSize   = sizeof(UT_CmdBuf.NoArgsCmd);

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    MM_AppData.HkPacket.Payload.CmdCounter     = 1;
    MM_AppData.HkPacket.Payload.ErrCounter     = 2;
    MM_AppData.HkPacket.Payload.LastAction     = 3;
    MM_AppData.HkPacket.Payload.MemType        = 4;
    MM_AppData.HkPacket.Payload.Address        = 5;
    MM_AppData.HkPacket.Payload.DataValue      = 6;
    MM_AppData.HkPacket.Payload.BytesProcessed = 7;

    strncpy(MM_AppData.HkPacket.Payload.FileName, "name", sizeof(MM_AppData.HkPacket.Payload.FileName) - 1);

    /* Execute the function being tested */
    MM_HousekeepingCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(MM_AppData.HkPacket.Payload.CmdCounter == 1, "MM_AppData.HkPacket.Payload.CmdCounter == 1");
    UtAssert_True(MM_AppData.HkPacket.Payload.ErrCounter == 2, "MM_AppData.HkPacket.Payload.ErrCounter == 2");
    UtAssert_True(MM_AppData.HkPacket.Payload.LastAction == 3, "MM_AppData.HkPacket.Payload.LastAction == 3");
    UtAssert_True(MM_AppData.HkPacket.Payload.MemType == 4, "MM_AppData.HkPacket.Payload.MemType == 4");
    UtAssert_True(MM_AppData.HkPacket.Payload.Address == 5, "MM_AppData.HkPacket.Payload.Address == 5");
    UtAssert_True(MM_AppData.HkPacket.Payload.DataValue == 6, "MM_AppData.HkPacket.Payload.DataValue == 6");
    UtAssert_True(MM_AppData.HkPacket.Payload.BytesProcessed == 7, "MM_AppData.HkPacket.Payload.BytesProcessed == 7");

    UtAssert_True(strncmp(MM_AppData.HkPacket.Payload.FileName, MM_AppData.HkPacket.Payload.FileName, OS_MAX_PATH_LEN) == 0,
                  "strncmp(MM_AppData.HkPacket.Payload.FileName, MM_AppData.HkPacket.Payload.FileName, OS_MAX_PATH_LEN) == 0");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void MM_LookupSymbolCmd_Test_Nominal(void)
{
    CFE_SB_MsgId_t    TestMsgId    = CFE_SB_ValueToMsgId(MM_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode      = MM_LOOKUP_SYM_CC;
    size_t            MsgSize      = sizeof(UT_CmdBuf.LookupSymCmd);
    cpuaddr           ResolvedAddr = 0;
    int32             strCmpResult;
    char              ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Symbol Lookup Command: Name = '%%s' Addr = %%p");

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    strncpy(UT_CmdBuf.LookupSymCmd.Payload.SymName, "name", sizeof(UT_CmdBuf.LookupSymCmd.Payload.SymName) - 1);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    UT_SetDataBuffer(UT_KEY(OS_SymbolLookup), &ResolvedAddr, sizeof(ResolvedAddr), false);

    /* Execute the function being tested */
    MM_LookupSymbolCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(MM_AppData.HkPacket.Payload.LastAction == MM_SYM_LOOKUP, "MM_AppData.HkPacket.Payload.LastAction == MM_SYM_LOOKUP");
    UtAssert_True(MM_AppData.HkPacket.Payload.Address == 0, "MM_AppData.HkPacket.Payload.Address == 0");
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_SYM_LOOKUP_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void MM_LookupSymbolCmd_Test_SymbolNameNull(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = MM_LOOKUP_SYM_CC;
    size_t            MsgSize   = sizeof(UT_CmdBuf.LookupSymCmd);
    int32             strCmpResult;
    char              ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "NUL (empty) string specified as symbol name");

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    UT_CmdBuf.LookupSymCmd.Payload.SymName[0] = '\0';

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    /* Execute the function being tested */
    MM_LookupSymbolCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_SYMNAME_NUL_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void MM_LookupSymbolCmd_Test_SymbolLookupError(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = MM_LOOKUP_SYM_CC;
    size_t            MsgSize   = sizeof(UT_CmdBuf.LookupSymCmd);
    int32             strCmpResult;
    char              ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Symbolic address can't be resolved: Name = '%%s'");

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    strncpy(UT_CmdBuf.LookupSymCmd.Payload.SymName, "name", sizeof(UT_CmdBuf.LookupSymCmd.Payload.SymName) - 1);

    /* Set to generate error message MM_SYMNAME_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(OS_SymbolLookup), 1, -1);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    /* Execute the function being tested */
    MM_LookupSymbolCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_SYMNAME_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void MM_SymTblToFileCmd_Test_Nominal(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = MM_SYM_TBL_TO_FILE_CC;
    size_t            MsgSize   = sizeof(UT_CmdBuf.SymTblToFileCmd);
    int32             strCmpResult;
    char              ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Symbol Table Dump to File Started: Name = '%%s'");

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    strncpy(UT_CmdBuf.SymTblToFileCmd.Payload.FileName, "name", sizeof(UT_CmdBuf.SymTblToFileCmd.Payload.FileName) - 1);

    /* Set to satisfy condition "OS_Status == OS_SUCCESS" */
    UT_SetDeferredRetcode(UT_KEY(OS_SymbolTableDump), 1, CFE_SUCCESS);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    /* Execute the function being tested */
    MM_SymTblToFileCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(MM_AppData.HkPacket.Payload.LastAction == MM_SYMTBL_SAVE, "MM_AppData.HkPacket.Payload.LastAction == MM_SYMTBL_SAVE");
    UtAssert_True(strncmp(MM_AppData.HkPacket.Payload.FileName, UT_CmdBuf.SymTblToFileCmd.Payload.FileName, OS_MAX_PATH_LEN) == 0,
                  "strncmp(MM_AppData.HkPacket.Payload.FileName, UT_CmdBuf.SymTblToFileCmd.Payload.FileName, OS_MAX_PATH_LEN) == 0");
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_SYMTBL_TO_FILE_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void MM_SymTblToFileCmd_Test_SymbolFilenameNull(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = MM_SYM_TBL_TO_FILE_CC;
    size_t            MsgSize   = sizeof(UT_CmdBuf.SymTblToFileCmd);
    int32             strCmpResult;
    char              ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "NUL (empty) string specified as symbol dump file name");

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    UT_CmdBuf.SymTblToFileCmd.Payload.FileName[0] = '\0';

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    /* Execute the function being tested */
    MM_SymTblToFileCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_SYMFILENAME_NUL_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void MM_SymTblToFileCmd_Test_SymbolTableDumpError(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = MM_SYM_TBL_TO_FILE_CC;
    size_t            MsgSize   = sizeof(UT_CmdBuf.SymTblToFileCmd);
    int32             strCmpResult;
    char              ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Error dumping symbol table, OS_Status= 0x%%X, File='%%s'");

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    strncpy(UT_CmdBuf.SymTblToFileCmd.Payload.FileName, "name", sizeof(UT_CmdBuf.SymTblToFileCmd.Payload.FileName) - 1);

    /* Set to generate error message MM_SYMTBL_TO_FILE_FAIL_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(OS_SymbolTableDump), 1, -1);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    /* Execute the function being tested */
    MM_SymTblToFileCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_SYMTBL_TO_FILE_FAIL_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void MM_EepromWriteEnaCmd_Test_Nominal(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = MM_ENABLE_EEPROM_WRITE_CC;
    size_t            MsgSize   = sizeof(UT_CmdBuf.EepromWriteEnaCmd);
    int32             strCmpResult;
    char              ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "EEPROM bank %%d write enabled, cFE_Status= 0x%%X");

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    UT_CmdBuf.EepromWriteEnaCmd.Payload.Bank = 0;

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    UT_SetDefaultReturnValue(UT_KEY(CFE_PSP_EepromWriteEnable), CFE_PSP_SUCCESS);

    /* Execute the function being tested */
    MM_EepromWriteEnaCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(MM_AppData.HkPacket.Payload.LastAction == MM_EEPROMWRITE_ENA,
                  "MM_AppData.HkPacket.Payload.LastAction == MM_EEPROMWRITE_ENA");
    UtAssert_True(MM_AppData.HkPacket.Payload.MemType == MM_EEPROM, "MM_AppData.HkPacket.Payload.MemType == MM_EEPROM");
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_EEPROM_WRITE_ENA_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void MM_EepromWriteEnaCmd_Test_Error(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = MM_ENABLE_EEPROM_WRITE_CC;
    size_t            MsgSize   = sizeof(UT_CmdBuf.EepromWriteEnaCmd);
    int32             strCmpResult;
    char              ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Error requesting EEPROM bank %%d write enable, cFE_Status= 0x%%X");

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Stub will fail on bank 1 */
    UT_CmdBuf.EepromWriteEnaCmd.Payload.Bank = 1;

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    /* Set to generate error message MM_EEPROM_WRITE_ENA_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_EepromWriteEnable), 1, -1);

    /* Execute the function being tested */
    MM_EepromWriteEnaCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_EEPROM_WRITE_ENA_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void MM_EepromWriteDisCmd_Test_Nominal(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = MM_DISABLE_EEPROM_WRITE_CC;
    size_t            MsgSize   = sizeof(UT_CmdBuf.EepromWriteDisCmd);
    int32             strCmpResult;
    char              ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "EEPROM bank %%d write disabled, cFE_Status= 0x%%X");

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    UT_CmdBuf.EepromWriteDisCmd.Payload.Bank = 0;

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    UT_SetDefaultReturnValue(UT_KEY(CFE_PSP_EepromWriteDisable), CFE_PSP_SUCCESS);

    /* Execute the function being tested */
    MM_EepromWriteDisCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(MM_AppData.HkPacket.Payload.LastAction == MM_EEPROMWRITE_DIS,
                  "MM_AppData.HkPacket.Payload.LastAction == MM_EEPROMWRITE_DIS");
    UtAssert_True(MM_AppData.HkPacket.Payload.MemType == MM_EEPROM, "MM_AppData.HkPacket.Payload.MemType == MM_EEPROM");
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_EEPROM_WRITE_DIS_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void MM_EepromWriteDisCmd_Test_Error(void)
{
    CFE_SB_MsgId_t    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    CFE_MSG_FcnCode_t FcnCode   = MM_DISABLE_EEPROM_WRITE_CC;
    size_t            MsgSize   = sizeof(UT_CmdBuf.EepromWriteDisCmd);
    int32             strCmpResult;
    char              ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Error requesting EEPROM bank %%d write disable, cFE_Status= 0x%%X");

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Stub will fail on bank 1 */
    UT_CmdBuf.EepromWriteDisCmd.Payload.Bank = 1;

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    /* Set to generate error message MM_EEPROM_WRITE_DIS_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_EepromWriteDisable), 1, -1);

    /* Execute the function being tested */
    MM_EepromWriteDisCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_EEPROM_WRITE_DIS_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

/*
 * Register the test cases to execute with the unit test tool
 */
void UtTest_Setup(void)
{
    UtTest_Add(MM_AppMain_Test_Nominal, MM_Test_Setup, MM_Test_TearDown, "MM_AppMain_Test_Nominal");
    UtTest_Add(MM_AppMain_Test_AppInitError, MM_Test_Setup, MM_Test_TearDown, "MM_AppMain_Test_AppInitError");
    UtTest_Add(MM_AppMain_Test_SBError, MM_Test_Setup, MM_Test_TearDown, "MM_AppMain_Test_SBError");
    UtTest_Add(MM_AppMain_Test_SBTimeout, MM_Test_Setup, MM_Test_TearDown, "MM_AppMain_Test_SBTimeout");
    UtTest_Add(MM_AppInit_Test_Nominal, MM_Test_Setup, MM_Test_TearDown, "MM_AppInit_Test_Nominal");
    UtTest_Add(MM_AppInit_Test_EVSRegisterError, MM_Test_Setup, MM_Test_TearDown, "MM_AppInit_Test_EVSRegisterError");
    UtTest_Add(MM_AppInit_Test_SBCreatePipeError, MM_Test_Setup, MM_Test_TearDown, "MM_AppInit_Test_SBCreatePipeError");
    UtTest_Add(MM_AppInit_Test_SBSubscribeHKError, MM_Test_Setup, MM_Test_TearDown,
               "MM_AppInit_Test_SBSubscribeHKError");
    UtTest_Add(MM_AppInit_Test_SBSubscribeMMError, MM_Test_Setup, MM_Test_TearDown,
               "MM_AppInit_Test_SBSubscribeMMError");
    UtTest_Add(MM_AppPipe_Test_SendHKSuccess, MM_Test_Setup, MM_Test_TearDown, "MM_AppPipe_Test_SendHkSuccess");
    UtTest_Add(MM_AppPipe_Test_SendHKFail, MM_Test_Setup, MM_Test_TearDown, "MM_AppPipe_Test_SendHkFail");
    UtTest_Add(MM_AppPipe_Test_NoopSuccess, MM_Test_Setup, MM_Test_TearDown, "MM_AppPipe_Test_NoopSuccess");
    UtTest_Add(MM_AppPipe_Test_NoopFail, MM_Test_Setup, MM_Test_TearDown, "MM_AppPipe_Test_NoopFail");
    UtTest_Add(MM_AppPipe_Test_ResetSuccess, MM_Test_Setup, MM_Test_TearDown, "MM_AppPipe_Test_ResetSuccess");
    UtTest_Add(MM_AppPipe_Test_ResetFail, MM_Test_Setup, MM_Test_TearDown, "MM_AppPipe_Test_ResetFail");
    UtTest_Add(MM_AppPipe_Test_PeekSuccess, MM_Test_Setup, MM_Test_TearDown, "MM_AppPipe_Test_PeekSuccess");
    UtTest_Add(MM_AppPipe_Test_PeekFail, MM_Test_Setup, MM_Test_TearDown, "MM_AppPipe_Test_PeekFail");
    UtTest_Add(MM_AppPipe_Test_PokeSuccess, MM_Test_Setup, MM_Test_TearDown, "MM_AppPipe_Test_PokeSuccess");
    UtTest_Add(MM_AppPipe_Test_PokeFail, MM_Test_Setup, MM_Test_TearDown, "MM_AppPipe_Test_PokeFail");
    UtTest_Add(MM_AppPipe_Test_LoadMemWIDSuccess, MM_Test_Setup, MM_Test_TearDown, "MM_AppPipe_Test_LoadMemWIDSuccess");
    UtTest_Add(MM_AppPipe_Test_LoadMemWIDFail, MM_Test_Setup, MM_Test_TearDown, "MM_AppPipe_Test_LoadMemWIDFail");
    UtTest_Add(MM_AppPipe_Test_LoadMemFromFileSuccess, MM_Test_Setup, MM_Test_TearDown, "LoadMemFromFileSuccess");
    UtTest_Add(MM_AppPipe_Test_LoadMemFromFileFail, MM_Test_Setup, MM_Test_TearDown, "LoadMemFromFileFail");
    UtTest_Add(MM_AppPipe_Test_DumpMemToFileSuccess, MM_Test_Setup, MM_Test_TearDown,
               "MM_AppPipe_Test_DumpMemToFileSuccess");
    UtTest_Add(MM_AppPipe_Test_DumpMemToFileFail, MM_Test_Setup, MM_Test_TearDown, "MM_AppPipe_Test_DumpMemToFileFail");
    UtTest_Add(MM_AppPipe_Test_DumpInEventSuccess, MM_Test_Setup, MM_Test_TearDown,
               "MM_AppPipe_Test_DumpInEventSuccess");
    UtTest_Add(MM_AppPipe_Test_DumpInEventFail, MM_Test_Setup, MM_Test_TearDown, "MM_AppPipe_Test_DumpInEventFail");
    UtTest_Add(MM_AppPipe_Test_FillMemSuccess, MM_Test_Setup, MM_Test_TearDown, "MM_AppPipe_Test_FillMemSuccess");
    UtTest_Add(MM_AppPipe_Test_FillMemFail, MM_Test_Setup, MM_Test_TearDown, "MM_AppPipe_Test_FillMemFail");
    UtTest_Add(MM_AppPipe_Test_LookupSymbolSuccess, MM_Test_Setup, MM_Test_TearDown,
               "MM_AppPipe_Test_LookupSymbolSuccess");
    UtTest_Add(MM_AppPipe_Test_LookupSymbolFail, MM_Test_Setup, MM_Test_TearDown, "MM_AppPipe_Test_LookupSymbolFail");
    UtTest_Add(MM_AppPipe_Test_SymTblToFileSuccess, MM_Test_Setup, MM_Test_TearDown,
               "MM_AppPipe_Test_SymTblToFileSuccess");
    UtTest_Add(MM_AppPipe_Test_SymTblToFileFail, MM_Test_Setup, MM_Test_TearDown, "MM_AppPipe_Test_SymTblToFileFail");
    UtTest_Add(MM_AppPipe_Test_EnableEepromWriteSuccess, MM_Test_Setup, MM_Test_TearDown,
               "MM_AppPipe_Test_EnableEepromWriteSuccess");
    UtTest_Add(MM_AppPipe_Test_EnableEepromWriteFail, MM_Test_Setup, MM_Test_TearDown,
               "MM_AppPipe_Test_EnableEepromWriteFail");
    UtTest_Add(MM_AppPipe_Test_DisableEepromWriteSuccess, MM_Test_Setup, MM_Test_TearDown,
               "MM_AppPipe_Test_DisableEepromWriteSuccess");
    UtTest_Add(MM_AppPipe_Test_DisableEepromWriteFail, MM_Test_Setup, MM_Test_TearDown,
               "MM_AppPipe_Test_DisableEepromWriteFail");
    UtTest_Add(MM_AppPipe_Test_InvalidCommandCode, MM_Test_Setup, MM_Test_TearDown,
               "MM_AppPipe_Test_InvalidCommandCode");
    UtTest_Add(MM_AppPipe_Test_InvalidCommandPipeMessageID, MM_Test_Setup, MM_Test_TearDown,
               "MM_AppPipe_Test_InvalidCommandPipeMessageID");

    UtTest_Add(MM_HousekeepingCmd_Test, MM_Test_Setup, MM_Test_TearDown, "MM_HousekeepingCmd_Test");

    UtTest_Add(MM_LookupSymbolCmd_Test_Nominal, MM_Test_Setup, MM_Test_TearDown, "MM_LookupSymbolCmd_Test_Nominal");
    UtTest_Add(MM_LookupSymbolCmd_Test_SymbolNameNull, MM_Test_Setup, MM_Test_TearDown,
               "MM_LookupSymbolCmd_Test_SymbolNameNull");
    UtTest_Add(MM_LookupSymbolCmd_Test_SymbolLookupError, MM_Test_Setup, MM_Test_TearDown,
               "MM_LookupSymbolCmd_Test_SymbolLookupError");

    UtTest_Add(MM_SymTblToFileCmd_Test_Nominal, MM_Test_Setup, MM_Test_TearDown, "MM_SymTblToFileCmd_Test_Nominal");
    UtTest_Add(MM_SymTblToFileCmd_Test_SymbolFilenameNull, MM_Test_Setup, MM_Test_TearDown,
               "MM_SymTblToFileCmd_Test_SymbolFilenameNull");
    UtTest_Add(MM_SymTblToFileCmd_Test_SymbolTableDumpError, MM_Test_Setup, MM_Test_TearDown,
               "MM_SymTblToFileCmd_Test_SymbolTableDumpError");

    UtTest_Add(MM_EepromWriteEnaCmd_Test_Nominal, MM_Test_Setup, MM_Test_TearDown, "MM_EepromWriteEnaCmd_Test_Nominal");
    UtTest_Add(MM_EepromWriteEnaCmd_Test_Error, MM_Test_Setup, MM_Test_TearDown, "MM_EepromWriteEnaCmd_Test_Error");

    UtTest_Add(MM_EepromWriteDisCmd_Test_Nominal, MM_Test_Setup, MM_Test_TearDown, "MM_EepromWriteDisCmd_Test_Nominal");
    UtTest_Add(MM_EepromWriteDisCmd_Test_Error, MM_Test_Setup, MM_Test_TearDown, "MM_EepromWriteDisCmd_Test_Error");
}
