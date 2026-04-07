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
 *   Unit tests for mm_dispatch.c
 */

/* ======== */
/* Includes */
/* ======== */

#include "mm_cmds.h"
#include "mm_dispatch.h"
#include "mm_eventids.h"
#include "mm_fcncodes.h"
#include "mm_msgdefs.h"
#include "mm_msgids.h"
#include "mm_test_utils.h"
#include "mm_topicids.h"

#include "cfe.h"
#include "utassert.h"
#include "utstubs.h"
#include "uttest.h"

/* ==================== */
/* Function Definitions */
/* ==================== */

void Test_MM_VerifyCmdLength_Nominal(void)
{
    bool              Result;
    size_t            ExpectedLen;
    CFE_SB_MsgId_t    MsgId;
    CFE_MSG_FcnCode_t FcnCode;

    /* Set up values for test */
    ExpectedLen = sizeof(MM_NoopCmd_t);
    MsgId       = CFE_SB_MSGID_C(MM_MISSION_CMD_TOPICID);
    FcnCode     = MM_NOOP_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &ExpectedLen, sizeof(size_t), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &MsgId, sizeof(CFE_SB_MsgId_t), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(CFE_MSG_FcnCode_t), false);

    /* Run function under test */
    Result = MM_VerifyCmdLength(NULL, sizeof(MM_NoopCmd_t));

    /* Evaluate run */
    UtAssert_True(Result, "MM_VerifyCmdLength returned true");

    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_VerifyCmdLength_InvalidSize(void)
{
    bool              Result;
    size_t            ExpectedLen;
    CFE_SB_MsgId_t    MsgId;
    CFE_MSG_FcnCode_t FcnCode;

    /* Set up values for test */
    ExpectedLen = sizeof(MM_NoopCmd_t) + 1;
    MsgId       = CFE_SB_MSGID_C(MM_MISSION_CMD_TOPICID);
    FcnCode     = MM_NOOP_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &ExpectedLen, sizeof(size_t), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &MsgId, sizeof(CFE_SB_MsgId_t), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(CFE_MSG_FcnCode_t), false);

    /* Run function under test */
    Result = MM_VerifyCmdLength(NULL, sizeof(MM_NoopCmd_t));

    /* Evaluate run */
    UtAssert_BOOL_FALSE(Result);

    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_CMD_LEN_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Invalid Msg length: ID = 0x%X,  CC = %u, Len = %u, Expected = %u");
}

void Test_MM_ProcessGroundCommand_NoopCmd(void)
{
    CFE_MSG_FcnCode_t CommandCode;
    size_t            ExpectedLen;

    /* Set up to run MM_NoopCmd() */
    CommandCode = MM_NOOP_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &CommandCode, sizeof(CFE_MSG_FcnCode_t), false);

    /* Set up to pass command length verification */
    ExpectedLen = sizeof(MM_NoopCmd_t);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &ExpectedLen, sizeof(size_t), false);

    /* Run function under test */
    MM_ProcessGroundCommand(NULL);

    /* Evaluate run */
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(MM_NoopCmd, 1);
}

void Test_MM_ProcessGroundCommand_NoopCmdErr(void)
{
    CFE_MSG_FcnCode_t CommandCode;
    size_t            ExpectedLen;
    CFE_SB_MsgId_t    MsgId;

    /* Provide a message ID for test */
    MsgId = CFE_SB_MSGID_C(MM_MISSION_CMD_TOPICID);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &MsgId, sizeof(CFE_SB_MsgId_t), false);

    /* Set up to run MM_NoopCmd() */
    CommandCode = MM_NOOP_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &CommandCode, sizeof(CFE_MSG_FcnCode_t), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &CommandCode, sizeof(CFE_MSG_FcnCode_t), false);

    /* Set up to fail command length verification */
    ExpectedLen = 0;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &ExpectedLen, sizeof(size_t), false);

    /* Run function under test */
    MM_ProcessGroundCommand(NULL);

    /* Evaluate run */
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(MM_NoopCmd, 0);
}

void Test_MM_ProcessGroundCommand_ResetCountersCmd(void)
{
    CFE_MSG_FcnCode_t CommandCode;
    size_t            ExpectedLen;

    /* Set up to run MM_ResetCountersCmd() */
    CommandCode = MM_RESET_COUNTERS_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &CommandCode, sizeof(CFE_MSG_FcnCode_t), false);

    /* Set up to pass command length verification */
    ExpectedLen = sizeof(MM_ResetCountersCmd_t);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &ExpectedLen, sizeof(size_t), false);

    /* Run function under test */
    MM_ProcessGroundCommand(NULL);

    /* Evaluate run */
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(MM_ResetCountersCmd, 1);
}

void Test_MM_ProcessGroundCommand_ResetCountersCmdErr(void)
{
    CFE_MSG_FcnCode_t CommandCode;
    size_t            ExpectedLen;
    CFE_SB_MsgId_t    MsgId;

    /* Provide a message ID for test */
    MsgId = CFE_SB_MSGID_C(MM_MISSION_CMD_TOPICID);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &MsgId, sizeof(CFE_SB_MsgId_t), false);

    /* Set up to run MM_ResetCountersCmd() */
    CommandCode = MM_RESET_COUNTERS_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &CommandCode, sizeof(CFE_MSG_FcnCode_t), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &CommandCode, sizeof(CFE_MSG_FcnCode_t), false);

    /* Set up to fail command length verification */
    ExpectedLen = 0;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &ExpectedLen, sizeof(size_t), false);

    /* Run function under test */
    MM_ProcessGroundCommand(NULL);

    /* Evaluate run */
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(MM_ResetCountersCmd, 0);
}

void Test_MM_ProcessGroundCommand_PeekCmd(void)
{
    CFE_MSG_FcnCode_t CommandCode;
    size_t            ExpectedLen;

    /* Set up to run MM_PeekCmd() */
    CommandCode = MM_PEEK_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &CommandCode, sizeof(CFE_MSG_FcnCode_t), false);

    /* Set up to pass command length verification */
    ExpectedLen = sizeof(MM_PeekCmd_t);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &ExpectedLen, sizeof(size_t), false);

    /* Run function under test */
    MM_ProcessGroundCommand(NULL);

    /* Evaluate run */
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(MM_PeekCmd, 1);
}

void Test_MM_ProcessGroundCommand_PeekCmdErr(void)
{
    CFE_MSG_FcnCode_t CommandCode;
    size_t            ExpectedLen;
    CFE_SB_MsgId_t    MsgId;

    /* Provide a message ID for test */
    MsgId = CFE_SB_MSGID_C(MM_MISSION_CMD_TOPICID);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &MsgId, sizeof(CFE_SB_MsgId_t), false);

    /* Set up to run MM_PeekCmd() */
    CommandCode = MM_PEEK_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &CommandCode, sizeof(CFE_MSG_FcnCode_t), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &CommandCode, sizeof(CFE_MSG_FcnCode_t), false);

    /* Set up to fail command length verification */
    ExpectedLen = 0;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &ExpectedLen, sizeof(size_t), false);

    /* Run function under test */
    MM_ProcessGroundCommand(NULL);

    /* Evaluate run */
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(MM_PeekCmd, 0);
}

void Test_MM_ProcessGroundCommand_PokeCmd(void)
{
    CFE_MSG_FcnCode_t CommandCode;
    size_t            ExpectedLen;

    /* Set up to run MM_PokeCmd() */
    CommandCode = MM_POKE_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &CommandCode, sizeof(CFE_MSG_FcnCode_t), false);

    /* Set up to pass command length verification */
    ExpectedLen = sizeof(MM_PokeCmd_t);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &ExpectedLen, sizeof(size_t), false);

    /* Run function under test */
    MM_ProcessGroundCommand(NULL);

    /* Evaluate run */
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(MM_PokeCmd, 1);
}

void Test_MM_ProcessGroundCommand_PokeCmdErr(void)
{
    CFE_MSG_FcnCode_t CommandCode;
    size_t            ExpectedLen;
    CFE_SB_MsgId_t    MsgId;

    /* Provide a message ID for test */
    MsgId = CFE_SB_MSGID_C(MM_MISSION_CMD_TOPICID);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &MsgId, sizeof(CFE_SB_MsgId_t), false);

    /* Set up to run MM_PokeCmd() */
    CommandCode = MM_POKE_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &CommandCode, sizeof(CFE_MSG_FcnCode_t), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &CommandCode, sizeof(CFE_MSG_FcnCode_t), false);

    /* Set up to fail command length verification */
    ExpectedLen = 0;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &ExpectedLen, sizeof(size_t), false);

    /* Run function under test */
    MM_ProcessGroundCommand(NULL);

    /* Evaluate run */
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(MM_PokeCmd, 0);
}

void Test_MM_ProcessGroundCommand_LoadMemWIDCmd(void)
{
    CFE_MSG_FcnCode_t CommandCode;
    size_t            ExpectedLen;

    /* Set up to run MM_LoadMemWIDCmd() */
    CommandCode = MM_LOAD_MEM_WID_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &CommandCode, sizeof(CFE_MSG_FcnCode_t), false);

    /* Set up to pass command length verification */
    ExpectedLen = sizeof(MM_LoadMemWIDCmd_t);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &ExpectedLen, sizeof(size_t), false);

    /* Run function under test */
    MM_ProcessGroundCommand(NULL);

    /* Evaluate run */
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(MM_LoadMemWIDCmd, 1);
}

void Test_MM_ProcessGroundCommand_LoadMemWIDCmdErr(void)
{
    CFE_MSG_FcnCode_t CommandCode;
    size_t            ExpectedLen;
    CFE_SB_MsgId_t    MsgId;

    /* Provide a message ID for test */
    MsgId = CFE_SB_MSGID_C(MM_MISSION_CMD_TOPICID);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &MsgId, sizeof(CFE_SB_MsgId_t), false);

    /* Set up to run MM_LoadMemWIDCmd() */
    CommandCode = MM_LOAD_MEM_WID_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &CommandCode, sizeof(CFE_MSG_FcnCode_t), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &CommandCode, sizeof(CFE_MSG_FcnCode_t), false);

    /* Set up to fail command length verification */
    ExpectedLen = 0;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &ExpectedLen, sizeof(size_t), false);

    /* Run function under test */
    MM_ProcessGroundCommand(NULL);

    /* Evaluate run */
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(MM_LoadMemWIDCmd, 0);
}

void Test_MM_ProcessGroundCommand_LoadMemFromFileCmd(void)
{
    CFE_MSG_FcnCode_t CommandCode;
    size_t            ExpectedLen;

    /* Set up to run MM_LoadMemFromFileCmd() */
    CommandCode = MM_LOAD_MEM_FROM_FILE_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &CommandCode, sizeof(CFE_MSG_FcnCode_t), false);

    /* Set up to pass command length verification */
    ExpectedLen = sizeof(MM_LoadMemFromFileCmd_t);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &ExpectedLen, sizeof(size_t), false);

    /* Run function under test */
    MM_ProcessGroundCommand(NULL);

    /* Evaluate run */
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(MM_LoadMemFromFileCmd, 1);
}

void Test_MM_ProcessGroundCommand_LoadMemFromFileCmdErr(void)
{
    CFE_MSG_FcnCode_t CommandCode;
    size_t            ExpectedLen;
    CFE_SB_MsgId_t    MsgId;

    /* Provide a message ID for test */
    MsgId = CFE_SB_MSGID_C(MM_MISSION_CMD_TOPICID);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &MsgId, sizeof(CFE_SB_MsgId_t), false);

    /* Set up to run MM_LoadMemFromFileCmd() */
    CommandCode = MM_LOAD_MEM_FROM_FILE_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &CommandCode, sizeof(CFE_MSG_FcnCode_t), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &CommandCode, sizeof(CFE_MSG_FcnCode_t), false);

    /* Set up to fail command length verification */
    ExpectedLen = 0;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &ExpectedLen, sizeof(size_t), false);

    /* Run function under test */
    MM_ProcessGroundCommand(NULL);

    /* Evaluate run */
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(MM_LoadMemFromFileCmd, 0);
}

void Test_MM_ProcessGroundCommand_DumpMemToFileCmd(void)
{
    CFE_MSG_FcnCode_t CommandCode;
    size_t            ExpectedLen;

    /* Set up to run MM_DumpMemToFileCmd() */
    CommandCode = MM_DUMP_MEM_TO_FILE_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &CommandCode, sizeof(CFE_MSG_FcnCode_t), false);

    /* Set up to pass command length verification */
    ExpectedLen = sizeof(MM_DumpMemToFileCmd_t);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &ExpectedLen, sizeof(size_t), false);

    /* Run function under test */
    MM_ProcessGroundCommand(NULL);

    /* Evaluate run */
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(MM_DumpMemToFileCmd, 1);
}

void Test_MM_ProcessGroundCommand_DumpMemToFileCmdErr(void)
{
    CFE_MSG_FcnCode_t CommandCode;
    size_t            ExpectedLen;
    CFE_SB_MsgId_t    MsgId;

    /* Provide a message ID for test */
    MsgId = CFE_SB_MSGID_C(MM_MISSION_CMD_TOPICID);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &MsgId, sizeof(CFE_SB_MsgId_t), false);

    /* Set up to run MM_DumpMemToFileCmd() */
    CommandCode = MM_DUMP_MEM_TO_FILE_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &CommandCode, sizeof(CFE_MSG_FcnCode_t), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &CommandCode, sizeof(CFE_MSG_FcnCode_t), false);

    /* Set up to fail command length verification */
    ExpectedLen = 0;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &ExpectedLen, sizeof(size_t), false);

    /* Run function under test */
    MM_ProcessGroundCommand(NULL);

    /* Evaluate run */
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(MM_DumpMemToFileCmd, 0);
}

void Test_MM_ProcessGroundCommand_DumpInEventCmd(void)
{
    CFE_MSG_FcnCode_t CommandCode;
    size_t            ExpectedLen;

    /* Set up to run MM_DumpInEventCmd() */
    CommandCode = MM_DUMP_IN_EVENT_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &CommandCode, sizeof(CFE_MSG_FcnCode_t), false);

    /* Set up to pass command length verification */
    ExpectedLen = sizeof(MM_DumpInEventCmd_t);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &ExpectedLen, sizeof(size_t), false);

    /* Run function under test */
    MM_ProcessGroundCommand(NULL);

    /* Evaluate run */
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(MM_DumpInEventCmd, 1);
}

void Test_MM_ProcessGroundCommand_DumpInEventCmdErr(void)
{
    CFE_MSG_FcnCode_t CommandCode;
    size_t            ExpectedLen;
    CFE_SB_MsgId_t    MsgId;

    /* Provide a message ID for test */
    MsgId = CFE_SB_MSGID_C(MM_MISSION_CMD_TOPICID);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &MsgId, sizeof(CFE_SB_MsgId_t), false);

    /* Set up to run MM_DumpInEventCmd() */
    CommandCode = MM_DUMP_IN_EVENT_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &CommandCode, sizeof(CFE_MSG_FcnCode_t), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &CommandCode, sizeof(CFE_MSG_FcnCode_t), false);

    /* Set up to fail command length verification */
    ExpectedLen = 0;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &ExpectedLen, sizeof(size_t), false);

    /* Run function under test */
    MM_ProcessGroundCommand(NULL);

    /* Evaluate run */
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(MM_DumpInEventCmd, 0);
}

void Test_MM_ProcessGroundCommand_FillMemCmd(void)
{
    CFE_MSG_FcnCode_t CommandCode;
    size_t            ExpectedLen;

    /* Set up to run MM_FillMemCmd() */
    CommandCode = MM_FILL_MEM_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &CommandCode, sizeof(CFE_MSG_FcnCode_t), false);

    /* Set up to pass command length verification */
    ExpectedLen = sizeof(MM_FillMemCmd_t);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &ExpectedLen, sizeof(size_t), false);

    /* Run function under test */
    MM_ProcessGroundCommand(NULL);

    /* Evaluate run */
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(MM_FillMemCmd, 1);
}

void Test_MM_ProcessGroundCommand_FillMemCmdErr(void)
{
    CFE_MSG_FcnCode_t CommandCode;
    size_t            ExpectedLen;
    CFE_SB_MsgId_t    MsgId;

    /* Provide a message ID for test */
    MsgId = CFE_SB_MSGID_C(MM_MISSION_CMD_TOPICID);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &MsgId, sizeof(CFE_SB_MsgId_t), false);

    /* Set up to run MM_FillMemCmd() */
    CommandCode = MM_FILL_MEM_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &CommandCode, sizeof(CFE_MSG_FcnCode_t), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &CommandCode, sizeof(CFE_MSG_FcnCode_t), false);

    /* Set up to fail command length verification */
    ExpectedLen = 0;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &ExpectedLen, sizeof(size_t), false);

    /* Run function under test */
    MM_ProcessGroundCommand(NULL);

    /* Evaluate run */
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(MM_FillMemCmd, 0);
}

void Test_MM_ProcessGroundCommand_LookupSymCmd(void)
{
    CFE_MSG_FcnCode_t CommandCode;
    size_t            ExpectedLen;

    /* Set up to run MM_LookupSymCmd() */
    CommandCode = MM_LOOKUP_SYM_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &CommandCode, sizeof(CFE_MSG_FcnCode_t), false);

    /* Set up to pass command length verification */
    ExpectedLen = sizeof(MM_LookupSymCmd_t);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &ExpectedLen, sizeof(size_t), false);

    /* Run function under test */
    MM_ProcessGroundCommand(NULL);

    /* Evaluate run */
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(MM_LookupSymCmd, 1);
}

void Test_MM_ProcessGroundCommand_LookupSymCmdErr(void)
{
    CFE_MSG_FcnCode_t CommandCode;
    size_t            ExpectedLen;
    CFE_SB_MsgId_t    MsgId;

    /* Provide a message ID for test */
    MsgId = CFE_SB_MSGID_C(MM_MISSION_CMD_TOPICID);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &MsgId, sizeof(CFE_SB_MsgId_t), false);

    /* Set up to run MM_LookupSymCmd() */
    CommandCode = MM_LOOKUP_SYM_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &CommandCode, sizeof(CFE_MSG_FcnCode_t), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &CommandCode, sizeof(CFE_MSG_FcnCode_t), false);

    /* Set up to fail command length verification */
    ExpectedLen = 0;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &ExpectedLen, sizeof(size_t), false);

    /* Run function under test */
    MM_ProcessGroundCommand(NULL);

    /* Evaluate run */
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(MM_LookupSymCmd, 0);
}

void Test_MM_ProcessGroundCommand_SymTblToFileCmd(void)
{
    CFE_MSG_FcnCode_t CommandCode;
    size_t            ExpectedLen;

    /* Set up to run MM_SymTblToFileCmd() */
    CommandCode = MM_SYM_TBL_TO_FILE_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &CommandCode, sizeof(CFE_MSG_FcnCode_t), false);

    /* Set up to pass command length verification */
    ExpectedLen = sizeof(MM_SymTblToFileCmd_t);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &ExpectedLen, sizeof(size_t), false);

    /* Run function under test */
    MM_ProcessGroundCommand(NULL);

    /* Evaluate run */
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(MM_SymTblToFileCmd, 1);
}

void Test_MM_ProcessGroundCommand_SymTblToFileCmdErr(void)
{
    CFE_MSG_FcnCode_t CommandCode;
    size_t            ExpectedLen;
    CFE_SB_MsgId_t    MsgId;

    /* Provide a message ID for test */
    MsgId = CFE_SB_MSGID_C(MM_MISSION_CMD_TOPICID);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &MsgId, sizeof(CFE_SB_MsgId_t), false);

    /* Set up to run MM_SymTblToFileCmd() */
    CommandCode = MM_SYM_TBL_TO_FILE_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &CommandCode, sizeof(CFE_MSG_FcnCode_t), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &CommandCode, sizeof(CFE_MSG_FcnCode_t), false);

    /* Set up to fail command length verification */
    ExpectedLen = 0;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &ExpectedLen, sizeof(size_t), false);

    /* Run function under test */
    MM_ProcessGroundCommand(NULL);

    /* Evaluate run */
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(MM_SymTblToFileCmd, 0);
}

void Test_MM_ProcessGroundCommand_EepromWriteEnaCmd(void)
{
    CFE_MSG_FcnCode_t CommandCode;
    size_t            ExpectedLen;

    /* Set up to run MM_EepromWriteEnaCmd() */
    CommandCode = MM_EEPROM_WRITE_ENA_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &CommandCode, sizeof(CFE_MSG_FcnCode_t), false);

    /* Set up to pass command length verification */
    ExpectedLen = sizeof(MM_EepromWriteEnaCmd_t);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &ExpectedLen, sizeof(size_t), false);

    /* Run function under test */
    MM_ProcessGroundCommand(NULL);

    /* Evaluate run */
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(MM_EepromWriteEnaCmd, 1);
}

void Test_MM_ProcessGroundCommand_EepromWriteEnaCmdErr(void)
{
    CFE_MSG_FcnCode_t CommandCode;
    size_t            ExpectedLen;
    CFE_SB_MsgId_t    MsgId;

    /* Provide a message ID for test */
    MsgId = CFE_SB_MSGID_C(MM_MISSION_CMD_TOPICID);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &MsgId, sizeof(CFE_SB_MsgId_t), false);

    /* Set up to run MM_EepromWriteEnaCmd() */
    CommandCode = MM_EEPROM_WRITE_ENA_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &CommandCode, sizeof(CFE_MSG_FcnCode_t), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &CommandCode, sizeof(CFE_MSG_FcnCode_t), false);

    /* Set up to fail command length verification */
    ExpectedLen = 0;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &ExpectedLen, sizeof(size_t), false);

    /* Run function under test */
    MM_ProcessGroundCommand(NULL);

    /* Evaluate run */
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(MM_EepromWriteEnaCmd, 0);
}

void Test_MM_ProcessGroundCommand_EepromWriteDisCmd(void)
{
    CFE_MSG_FcnCode_t CommandCode;
    size_t            ExpectedLen;

    /* Set up to run MM_EepromWriteDisCmd() */
    CommandCode = MM_EEPROM_WRITE_DIS_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &CommandCode, sizeof(CFE_MSG_FcnCode_t), false);

    /* Set up to pass command length verification */
    ExpectedLen = sizeof(MM_EepromWriteDisCmd_t);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &ExpectedLen, sizeof(size_t), false);

    /* Run function under test */
    MM_ProcessGroundCommand(NULL);

    /* Evaluate run */
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(MM_EepromWriteDisCmd, 1);
}

void Test_MM_ProcessGroundCommand_EepromWriteDisCmdErr(void)
{
    CFE_MSG_FcnCode_t CommandCode;
    size_t            ExpectedLen;
    CFE_SB_MsgId_t    MsgId;

    /* Provide a message ID for test */
    MsgId = CFE_SB_MSGID_C(MM_MISSION_CMD_TOPICID);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &MsgId, sizeof(CFE_SB_MsgId_t), false);

    /* Set up to run MM_EepromWriteDisCmd() */
    CommandCode = MM_EEPROM_WRITE_DIS_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &CommandCode, sizeof(CFE_MSG_FcnCode_t), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &CommandCode, sizeof(CFE_MSG_FcnCode_t), false);

    /* Set up to fail command length verification */
    ExpectedLen = 0;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &ExpectedLen, sizeof(size_t), false);

    /* Run function under test */
    MM_ProcessGroundCommand(NULL);

    /* Evaluate run */
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(MM_EepromWriteDisCmd, 0);
}

void Test_MM_ProcessGroundCommand_UnknownCC(void)
{
    CFE_MSG_FcnCode_t CommandCode;

    /* Set up for a failed command code check */
    CommandCode = 200;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &CommandCode, sizeof(CFE_MSG_FcnCode_t), false);

    /* Run function under test */
    MM_ProcessGroundCommand(NULL);

    /* Evaluate run */
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_CC_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_StrnCmp("Invalid ground command code %d",
                     context_CFE_EVS_SendEvent[0].Spec,
                     CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
                     "Event string matched expected result, '%s'",
                     context_CFE_EVS_SendEvent[0].Spec);
}

void Test_MM_TaskPipe_SendHk(void)
{
    CFE_SB_MsgId_t MsgId;

    /* Set up to send an HK packet */
    MsgId = CFE_SB_ValueToMsgId(MM_SEND_HK_MID);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &MsgId, sizeof(CFE_SB_MsgId_t), false);

    /* Run function under test */
    MM_TaskPipe(NULL);

    /* Evaluate run */
    UtAssert_STUB_COUNT(MM_SendHkCmd, 1);

    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);
}

void Test_MM_TaskPipe_Cmd(void)
{
    CFE_SB_MsgId_t    MsgId;
    CFE_MSG_FcnCode_t CommandCode;
    size_t            ExpectedLen;

    /* Set up to run MM_NoopCmd() */
    CommandCode = MM_NOOP_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &CommandCode, sizeof(CFE_MSG_FcnCode_t), false);

    /* Set up to pass command length verification */
    ExpectedLen = sizeof(MM_NoopCmd_t);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &ExpectedLen, sizeof(size_t), false);

    /* Set up to send an HK packet */
    MsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &MsgId, sizeof(CFE_SB_MsgId_t), false);

    /* Run function under test */
    MM_TaskPipe(NULL);

    /* Evaluate run */
    UtAssert_STUB_COUNT(MM_NoopCmd, 1);

    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);
}

void Test_MM_TaskPipe_UnknownMID(void)
{
    CFE_SB_MsgId_t MsgId;

    /* Set up to send an HK packet */
    MsgId = CFE_SB_ValueToMsgId(400);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &MsgId, sizeof(CFE_SB_MsgId_t), false);

    /* Run function under test */
    MM_TaskPipe(NULL);

    /* Evaluate run */
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8_t, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0, MM_MID_ERR_EID, CFE_EVS_EventType_ERROR, "Invalid command pipe message ID: 0x%08lX");
}

/*
 * Register the test cases to execute with the unit test tool
 */
void UtTest_Setup(void)
{
    ADD_TEST(Test_MM_VerifyCmdLength_Nominal);
    ADD_TEST(Test_MM_VerifyCmdLength_InvalidSize);
    ADD_TEST(Test_MM_ProcessGroundCommand_NoopCmd);
    ADD_TEST(Test_MM_ProcessGroundCommand_NoopCmdErr);
    ADD_TEST(Test_MM_ProcessGroundCommand_ResetCountersCmd);
    ADD_TEST(Test_MM_ProcessGroundCommand_ResetCountersCmdErr);
    ADD_TEST(Test_MM_ProcessGroundCommand_PeekCmd);
    ADD_TEST(Test_MM_ProcessGroundCommand_PeekCmdErr);
    ADD_TEST(Test_MM_ProcessGroundCommand_PokeCmd);
    ADD_TEST(Test_MM_ProcessGroundCommand_PokeCmdErr);
    ADD_TEST(Test_MM_ProcessGroundCommand_LoadMemWIDCmd);
    ADD_TEST(Test_MM_ProcessGroundCommand_LoadMemWIDCmdErr);
    ADD_TEST(Test_MM_ProcessGroundCommand_LoadMemFromFileCmd);
    ADD_TEST(Test_MM_ProcessGroundCommand_LoadMemFromFileCmdErr);
    ADD_TEST(Test_MM_ProcessGroundCommand_DumpMemToFileCmd);
    ADD_TEST(Test_MM_ProcessGroundCommand_DumpMemToFileCmdErr);
    ADD_TEST(Test_MM_ProcessGroundCommand_DumpInEventCmd);
    ADD_TEST(Test_MM_ProcessGroundCommand_DumpInEventCmdErr);
    ADD_TEST(Test_MM_ProcessGroundCommand_FillMemCmd);
    ADD_TEST(Test_MM_ProcessGroundCommand_FillMemCmdErr);
    ADD_TEST(Test_MM_ProcessGroundCommand_LookupSymCmd);
    ADD_TEST(Test_MM_ProcessGroundCommand_LookupSymCmdErr);
    ADD_TEST(Test_MM_ProcessGroundCommand_SymTblToFileCmd);
    ADD_TEST(Test_MM_ProcessGroundCommand_SymTblToFileCmdErr);
    ADD_TEST(Test_MM_ProcessGroundCommand_EepromWriteEnaCmd);
    ADD_TEST(Test_MM_ProcessGroundCommand_EepromWriteEnaCmdErr);
    ADD_TEST(Test_MM_ProcessGroundCommand_EepromWriteDisCmd);
    ADD_TEST(Test_MM_ProcessGroundCommand_EepromWriteDisCmdErr);
    ADD_TEST(Test_MM_ProcessGroundCommand_UnknownCC);
    ADD_TEST(Test_MM_TaskPipe_SendHk);
    ADD_TEST(Test_MM_TaskPipe_Cmd);
    ADD_TEST(Test_MM_TaskPipe_UnknownMID);
}
