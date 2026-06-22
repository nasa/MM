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
 *   Unit tests for mm_app.c
 */

/************************************************************************
** Includes
*************************************************************************/
#include "mm_app.h"
#include "mm_eventids.h"
#include "mm_fcncodes.h"
#include "mm_msgids.h"

#include "mm_test_utils.h"

/************************************************************************
** UT Includes
*************************************************************************/
#include "utassert.h"
#include "utstubs.h"
#include "uttest.h"

#include "cfe.h"
#include "cfe_msgids.h"
#include <stdlib.h>
#include <unistd.h>

/* ==================== */
/* Function Definitions */
/* ==================== */

int32 MM_APP_TEST_CFE_ES_ExitAppHook(void                   *UserObj,
                                     int32                   StubRetcode,
                                     uint32                  CallCount,
                                     const UT_StubContext_t *Context)
{
    MM_AppData.HkTlm.Payload.CommandCounter++;

    return 0;
}

void MM_AppMain_Test_Nominal(void)
{
    CFE_SB_Buffer_t  Buf;
    CFE_SB_Buffer_t *BufPtr = &Buf;

    /* Set to exit loop after first run */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RunLoop), 1, true);

    /* Causes check for non-null buffer pointer to succeed */
    UT_SetDataBuffer(UT_KEY(CFE_SB_ReceiveBuffer), &BufPtr, sizeof(BufPtr), false);

    /* Execute the function being tested */
    MM_AppMain();

    /* Verify results */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_STUB_COUNT(CFE_ES_ExitApp, 1);
}

void MM_AppMain_Test_AppInitError(void)
{
    /* Set to exit loop after first run */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RunLoop), 1, true);

    /* Set to prevent call to CFE_SB_RcvMsg from returning an error */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_ReceiveBuffer), 1, CFE_SUCCESS);

    /* Set to satisfy condition "Status != CFE_SUCCESS" */
    UT_SetDeferredRetcode(UT_KEY(CFE_EVS_Register), 1, CFE_ES_BAD_ARGUMENT);

    /* Execute the function being tested */
    MM_AppMain();

    /* Verify results */
    UtAssert_INT32_EQ(MM_AppData.RunStatus, CFE_ES_RunStatus_APP_ERROR);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void MM_AppMain_Test_SBError(void)
{
    /* Set to exit loop after first run */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RunLoop), 1, true);

    /* Set to generate error message MM_PIPE_ERR_EID */
    UT_SetDefaultReturnValue(UT_KEY(CFE_SB_ReceiveBuffer), CFE_SB_BAD_ARGUMENT);

    /* Execute the function being tested */
    MM_AppMain();

    /* Verify results */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
    MM_Test_Verify_Event(1, MM_PIPE_ERR_EID, CFE_EVS_EventType_ERROR, "SB Pipe Read Error, App will exit. RC = 0x%08X");
}

void MM_AppMain_Test_SBTimeout(void)
{
    /* Set to exit loop after first run */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RunLoop), 1, true);

    /* Set to generate error message MM_PIPE_ERR_EID */
    UT_SetDefaultReturnValue(UT_KEY(CFE_SB_ReceiveBuffer), CFE_SB_TIME_OUT);

    /* Execute the function being tested */
    MM_AppMain();

    /* Verify results */
    /* Generates 1 event message we don't care about in this test */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void MM_AppMain_Test_SBNoMessage(void)
{
    size_t         forced_Size  = 1;
    CFE_SB_MsgId_t forced_MsgID = MM_UT_MID_1;

    /* Set to exit loop after first run */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RunLoop), 1, true);

    /* Set to generate error message CFE_SB_NO_MESSAGE */
    UT_SetDefaultReturnValue(UT_KEY(CFE_SB_ReceiveBuffer), CFE_SB_NO_MESSAGE);

    /* Set to prevent segmentation fault */
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);

    /* Used to verify completion of MM_AppMain by incrementing MM_AppData.HkPacket.CmdCounter. */
    UT_SetHookFunction(UT_KEY(CFE_ES_ExitApp), MM_APP_TEST_CFE_ES_ExitAppHook, NULL);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(MM_AppMain());

    /* Verify results */
    /* Generates 1 event message we don't care about in this test */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void MM_AppInit_Test_Nominal(void)
{
    CFE_Status_t Result;

    /* Initialize command counters to 1, in order verify that MM_AppInit actually
     * initializes them */
    MM_AppData.HkTlm.Payload.CommandCounter      = 1;
    MM_AppData.HkTlm.Payload.CommandErrorCounter = 1;

    /* Execute the function being tested */
    Result = MM_AppInit();

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);

    UtAssert_INT32_EQ(MM_AppData.RunStatus, CFE_ES_RunStatus_APP_RUN);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0, MM_INIT_INF_EID, CFE_EVS_EventType_INFORMATION, "MM Initialized. Version %d.%d.%d.%d");
}

void MM_AppInit_Test_EVSRegisterError(void)
{
    CFE_Status_t Result;

    /* Initialize command counters to 1, in order verify that MM_AppInit actually
     * initializes them */
    MM_AppData.HkTlm.Payload.CommandCounter      = 1;
    MM_AppData.HkTlm.Payload.CommandErrorCounter = 1;

    /* Set to generate system log message "MM App: Error Registering For Event
     * Services " */
    UT_SetDeferredRetcode(UT_KEY(CFE_EVS_Register), 1, CFE_ES_BAD_ARGUMENT);

    /* Execute the function being tested */
    Result = MM_AppInit();

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_ES_BAD_ARGUMENT);

    UtAssert_INT32_EQ(MM_AppData.RunStatus, CFE_ES_RunStatus_APP_RUN);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
    UtAssert_StrnCmp("MM App: Error Registering For Event Services, RC = 0x%08X\n",
                     context_CFE_ES_WriteToSysLog.Spec,
                     CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
                     "Sys Log string matched expected result, '%s'",
                     context_CFE_ES_WriteToSysLog.Spec);
}

void MM_AppInit_Test_SBCreatePipeError(void)
{
    CFE_Status_t Result;

    /* Initialize command counters to 1, in order verify that MM_AppInit actually
     * initializes them */
    MM_AppData.HkTlm.Payload.CommandCounter      = 1;
    MM_AppData.HkTlm.Payload.CommandErrorCounter = 1;

    /* Set to generate system log message "MM App: Error Creating SB Pipe" */
    UT_SetDefaultReturnValue(UT_KEY(CFE_SB_CreatePipe), CFE_SB_PIPE_CR_ERR);

    /* Execute the function being tested */
    Result = MM_AppInit();

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SB_PIPE_CR_ERR);

    UtAssert_INT32_EQ(MM_AppData.RunStatus, CFE_ES_RunStatus_APP_RUN);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_StrnCmp("Error Creating SB Pipe, RC = 0x%08X",
                     context_CFE_EVS_SendEvent[0].Spec,
                     CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
                     "Sys Log string matched expected result, '%s'",
                     context_CFE_EVS_SendEvent[0].Spec);
}

void MM_AppInit_Test_SBSubscribeHKError(void)
{
    CFE_Status_t Result;

    /* Initialize command counters to 1, in order verify that MM_AppInit actually
     * initializes them */
    MM_AppData.HkTlm.Payload.CommandCounter      = 1;
    MM_AppData.HkTlm.Payload.CommandErrorCounter = 1;

    /* Set to generate system log message "MM App: Error Subscribing to HK
     * Request" */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_Subscribe), 1, CFE_SB_BAD_ARGUMENT);

    /* Execute the function being tested */
    Result = MM_AppInit();

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SB_BAD_ARGUMENT);

    UtAssert_INT32_EQ(MM_AppData.RunStatus, CFE_ES_RunStatus_APP_RUN);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_StrnCmp("Error Subscribing to HK Request, RC = 0x%08X",
                     context_CFE_EVS_SendEvent[0].Spec,
                     CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
                     "Sys Log string matched expected result, '%s'",
                     context_CFE_EVS_SendEvent[0].Spec);
}

void MM_AppInit_Test_SBSubscribeMMError(void)
{
    CFE_Status_t Result;

    /* Initialize command counters to 1, in order verify that MM_AppInit actually
     * initializes them */
    MM_AppData.HkTlm.Payload.CommandCounter      = 1;
    MM_AppData.HkTlm.Payload.CommandErrorCounter = 1;

    /* Set to generate system log message "MM App: Error Subscribing to MM
     * Command" */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_Subscribe), 2, CFE_SB_BAD_ARGUMENT);

    /* Execute the function being tested */
    Result = MM_AppInit();

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SB_BAD_ARGUMENT);

    UtAssert_INT32_EQ(MM_AppData.RunStatus, CFE_ES_RunStatus_APP_RUN);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_StrnCmp("Error Subscribing to MM Command, RC = 0x%08X",
                     context_CFE_EVS_SendEvent[0].Spec,
                     CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
                     "Sys Log string matched expected result, '%s'",
                     context_CFE_EVS_SendEvent[0].Spec);
}

/*
 * Register the test cases to execute with the unit test tool
 */
void UtTest_Setup(void)
{
    ADD_TEST(MM_AppMain_Test_Nominal);
    ADD_TEST(MM_AppMain_Test_AppInitError);
    ADD_TEST(MM_AppMain_Test_SBError);
    ADD_TEST(MM_AppMain_Test_SBTimeout);
    ADD_TEST(MM_AppMain_Test_SBNoMessage);
    ADD_TEST(MM_AppInit_Test_Nominal);
    ADD_TEST(MM_AppInit_Test_EVSRegisterError);
    ADD_TEST(MM_AppInit_Test_SBCreatePipeError);
    ADD_TEST(MM_AppInit_Test_SBSubscribeHKError);
    ADD_TEST(MM_AppInit_Test_SBSubscribeMMError);
}
