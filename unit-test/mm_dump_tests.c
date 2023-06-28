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
 *   Unit tests for mm_dump.c
 */

/************************************************************************
** Includes
*************************************************************************/
#include "mm_dump.h"
#include "mm_msg.h"
#include "mm_msgdefs.h"
#include "mm_msgids.h"
#include "mm_events.h"
#include "mm_filedefs.h"
#include "mm_version.h"
#include "mm_utils.h"
#include "mm_test_utils.h"
#include "mm_mem32.h"
#include "mm_mem16.h"
#include "mm_mem8.h"

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

/* mm_dump_tests globals */
uint8 call_count_CFE_EVS_SendEvent;

uint8 Buffer[MM_MAX_FILL_DATA_SEG * 2];

/*
 * Function Definitions
 */

int32 UT_MM_LOAD_TEST_CFE_SymbolLookupHook1(void *UserObj, int32 StubRetcode, uint32 CallCount,
                                            const UT_StubContext_t *Context)
{
    cpuaddr *ResolvedAddress = (cpuaddr *)Context->ArgPtr[1];

    *ResolvedAddress = (cpuaddr)Buffer;

    return true;
}

int32 UT_MM_LOAD_TEST_OS_WriteHook1(void *UserObj, int32 StubRetcode, uint32 CallCount, const UT_StubContext_t *Context)
{
    MM_LoadDumpFileHeader_t *header = (MM_LoadDumpFileHeader_t *)Context->ArgPtr[1];

    cpuaddr *ResolvedAddress = (cpuaddr *)&header->SymAddress.Offset;

    *ResolvedAddress = (cpuaddr)Buffer;

    return sizeof(MM_LoadDumpFileHeader_t);
}

void MM_PeekCmd_Test_Nominal(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    int32             strCmpResult;
    char              ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool              Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Peek Command: Addr = %%p Size = %%u bits Data = 0x%%08X");

    UT_CmdBuf.PeekCmd.Payload.MemType  = MM_RAM;
    UT_CmdBuf.PeekCmd.Payload.DataSize = 32;

    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    FcnCode   = MM_PEEK_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    /* ignore dummy message length check */
    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_ResolveSymAddr), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyPeekPokeParams), 1, true);

    /* Execute the function being tested */
    Result = MM_PeekCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_PEEK_DWORD_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);
}

void MM_PeekCmd_Test_SymNameError(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    int32             strCmpResult;
    char              ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool              Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Symbolic address can't be resolved: Name = '%%s'");

    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    FcnCode   = MM_PEEK_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_CmdBuf.PeekCmd.Payload.MemType  = MM_RAM;
    UT_CmdBuf.PeekCmd.Payload.DataSize = 32;

    strncpy(UT_CmdBuf.PeekCmd.Payload.SrcSymAddress.SymName, "name", sizeof(UT_CmdBuf.PeekCmd.Payload.SrcSymAddress.SymName) - 1);

    /* Set to generate error message MM_SYMNAME_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(OS_SymbolLookup), 1, -1);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    /* Execute the function being tested */
    Result = MM_PeekCmd(&UT_CmdBuf.Buf);

    /* Verify results */

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_True(Result == false, "Result == false");

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

void MM_PeekCmd_Test_NoVerifyPeekPokeParams(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    bool              Result;

    UT_CmdBuf.PeekCmd.Payload.MemType  = MM_RAM;
    UT_CmdBuf.PeekCmd.Payload.DataSize = 32;

    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    FcnCode   = MM_PEEK_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    /* ignore dummy message length check */
    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_ResolveSymAddr), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyPeekPokeParams), 1, false);

    /* Execute the function being tested */
    Result = MM_PeekCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void MM_PeekMem_Test_Byte(void)
{
    MM_PeekCmd_t CmdPacket;
    uint32       SrcAddress = 1;
    int32        strCmpResult;
    char         ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool         Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Peek Command: Addr = %%p Size = %%u bits Data = 0x%%08X");

    CmdPacket.Payload.DataSize = MM_BYTE_BIT_WIDTH;
    CmdPacket.Payload.MemType  = MM_RAM;

    /* Execute the function being tested */
    Result = MM_PeekMem(&CmdPacket, SrcAddress);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_PEEK_BYTE_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);

    UtAssert_True(MM_AppData.HkPacket.Payload.LastAction == MM_PEEK, "MM_AppData.HkPacket.Payload.LastAction == MM_PEEK");
    UtAssert_True(MM_AppData.HkPacket.Payload.MemType == MM_RAM, "MM_AppData.HkPacket.Payload.MemType == MM_RAM");
    UtAssert_True(MM_AppData.HkPacket.Payload.Address == 1, "MM_AppData.HkPacket.Payload.Address == 1");
    UtAssert_True(MM_AppData.HkPacket.Payload.BytesProcessed == 1, "MM_AppData.HkPacket.Payload.BytesProcessed == 1");
    UtAssert_True(MM_AppData.HkPacket.Payload.DataValue == 1, "MM_AppData.HkPacket.Payload.DataValue == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void MM_PeekMem_Test_ByteError(void)
{
    MM_PeekCmd_t CmdPacket;
    uint32       SrcAddress = 1;
    int32        strCmpResult;
    char         ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool         Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "PSP read memory error: RC=%%d, Address=%%p, MemType=MEM%%u");

    CmdPacket.Payload.DataSize = MM_BYTE_BIT_WIDTH;

    /* Set to generate error message MM_PSP_READ_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemRead8), 1, -1);

    /* Execute the function being tested */
    Result = MM_PeekMem(&CmdPacket, SrcAddress);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void MM_PeekMem_Test_Word(void)
{
    MM_PeekCmd_t CmdPacket;
    uint32       SrcAddress = 1;
    int32        strCmpResult;
    char         ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool         Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Peek Command: Addr = %%p Size = %%u bits Data = 0x%%08X");

    CmdPacket.Payload.DataSize = MM_WORD_BIT_WIDTH;
    CmdPacket.Payload.MemType  = MM_RAM;

    /* Execute the function being tested */
    Result = MM_PeekMem(&CmdPacket, SrcAddress);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_PEEK_WORD_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);

    UtAssert_True(MM_AppData.HkPacket.Payload.LastAction == MM_PEEK, "MM_AppData.HkPacket.Payload.LastAction == MM_PEEK");
    UtAssert_True(MM_AppData.HkPacket.Payload.MemType == MM_RAM, "MM_AppData.HkPacket.Payload.MemType == MM_RAM");
    UtAssert_True(MM_AppData.HkPacket.Payload.Address == 1, "MM_AppData.HkPacket.Payload.Address == 1");
    UtAssert_True(MM_AppData.HkPacket.Payload.BytesProcessed == 2, "MM_AppData.HkPacket.Payload.BytesProcessed == 2");
    UtAssert_True(MM_AppData.HkPacket.Payload.DataValue == 0, "MM_AppData.HkPacket.Payload.DataValue == 0");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void MM_PeekMem_Test_WordError(void)
{
    MM_PeekCmd_t CmdPacket;
    uint32       SrcAddress = 0;
    int32        strCmpResult;
    char         ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool         Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "PSP read memory error: RC=%%d, Address=%%p, MemType=MEM%%u");

    CmdPacket.Payload.DataSize = MM_WORD_BIT_WIDTH;

    /* Set to generate error message MM_PSP_READ_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemRead16), 1, -1);

    /* Execute the function being tested */
    Result = MM_PeekMem(&CmdPacket, SrcAddress);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_PSP_READ_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void MM_PeekMem_Test_DWord(void)
{
    MM_PeekCmd_t CmdPacket;
    uint32       SrcAddress = 1;
    int32        strCmpResult;
    char         ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool         Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Peek Command: Addr = %%p Size = %%u bits Data = 0x%%08X");

    CmdPacket.Payload.DataSize = MM_DWORD_BIT_WIDTH;
    CmdPacket.Payload.MemType  = MM_RAM;

    /* Execute the function being tested */
    Result = MM_PeekMem(&CmdPacket, SrcAddress);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_PEEK_DWORD_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);

    UtAssert_True(MM_AppData.HkPacket.Payload.LastAction == MM_PEEK, "MM_AppData.HkPacket.Payload.LastAction == MM_PEEK");
    UtAssert_True(MM_AppData.HkPacket.Payload.MemType == MM_RAM, "MM_AppData.HkPacket.Payload.MemType == MM_RAM");
    UtAssert_True(MM_AppData.HkPacket.Payload.Address == 1, "MM_AppData.HkPacket.Payload.Address == 1");
    UtAssert_True(MM_AppData.HkPacket.Payload.BytesProcessed == 4, "MM_AppData.HkPacket.Payload.BytesProcessed == 4");
    UtAssert_True(MM_AppData.HkPacket.Payload.DataValue == 0, "MM_AppData.HkPacket.Payload.DataValue == 0");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void MM_PeekMem_Test_DWordError(void)
{
    MM_PeekCmd_t CmdPacket;
    uint32       SrcAddress = 0;
    int32        strCmpResult;
    char         ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool         Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "PSP read memory error: RC=%%d, Address=%%p, MemType=MEM%%u");

    CmdPacket.Payload.DataSize = MM_DWORD_BIT_WIDTH;

    /* Set to generate error message MM_PSP_READ_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemRead32), 1, -1);

    /* Execute the function being tested */
    Result = MM_PeekMem(&CmdPacket, SrcAddress);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_PSP_READ_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);
}

void MM_PeekMem_Test_DefaultSwitch(void)
{
    MM_PeekCmd_t CmdPacket;
    uint32       SrcAddress = 1;
    bool         Result;

    CmdPacket.Payload.DataSize = 99;

    /* Execute the function being tested */
    Result = MM_PeekMem(&CmdPacket, SrcAddress);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);
}

void MM_DumpMemToFileCmd_Test_RAM(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    int32             strCmpResult;
    char              ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool              Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Dump Memory To File Command: Dumped %%d bytes from address %%p to file '%%s'");

    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    FcnCode   = MM_DUMP_MEM_TO_FILE_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    strncpy(UT_CmdBuf.DumpMemToFileCmd.Payload.SrcSymAddress.SymName, "SymName",
            sizeof(UT_CmdBuf.DumpMemToFileCmd.Payload.SrcSymAddress.SymName) - 1);
    UT_CmdBuf.DumpMemToFileCmd.Payload.SrcSymAddress.Offset = (cpuaddr)&Buffer[0];

    UT_CmdBuf.DumpMemToFileCmd.Payload.MemType    = MM_RAM;
    UT_CmdBuf.DumpMemToFileCmd.Payload.NumOfBytes = 1;

    strncpy(UT_CmdBuf.DumpMemToFileCmd.Payload.FileName, "filename", sizeof(UT_CmdBuf.DumpMemToFileCmd.Payload.FileName) - 1);

    /* Causes call to MM_ResolveSymAddr to return a known value for DestAddress */
    UT_SetHookFunction(UT_KEY(MM_ResolveSymAddr), UT_MM_LOAD_TEST_CFE_SymbolLookupHook1, 0);

    /* Set to satisfy 2 instances of condition "Valid == true": after comment "Write the file headers" and comment "end
     * Valid == true if" */
    UT_SetDefaultReturnValue(UT_KEY(CFE_FS_WriteHeader), sizeof(CFE_FS_Header_t));

    /* Set to satisfy condition "Valid == true" before comment "Compute CRC of dumped data" */
    UT_SetDeferredRetcode(UT_KEY(OS_write), 1, sizeof(MM_LoadDumpFileHeader_t));

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), true);

    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), true);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    /* Execute the function being tested */
    Result = MM_DumpMemToFileCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_DMP_MEM_FILE_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_True(MM_AppData.HkPacket.Payload.LastAction == MM_DUMP_TO_FILE,
                  "MM_AppData.HkPacket.Payload.LastAction == MM_DUMP_TO_FILE");
    UtAssert_True(MM_AppData.HkPacket.Payload.Address == (cpuaddr)UT_CmdBuf.DumpMemToFileCmd.Payload.SrcSymAddress.Offset,
                  "MM_AppData.HkPacket.Payload.Address == FileHeader.SymAddress.Offset");
    UtAssert_STRINGBUF_EQ(MM_AppData.HkPacket.Payload.FileName, sizeof(MM_AppData.HkPacket.Payload.FileName),
                          UT_CmdBuf.DumpMemToFileCmd.Payload.FileName, sizeof(UT_CmdBuf.DumpMemToFileCmd.Payload.FileName));
    UtAssert_True(MM_AppData.HkPacket.Payload.MemType == UT_CmdBuf.DumpMemToFileCmd.Payload.MemType,
                  "MM_AppData.HkPacket.Payload.MemType == UT_CmdBuf.DumpMemToFileCmd.Payload.MemType");
    UtAssert_True(MM_AppData.HkPacket.Payload.BytesProcessed == UT_CmdBuf.DumpMemToFileCmd.Payload.NumOfBytes,
                  "MM_AppData.HkPacket.Payload.BytesProcessed == UT_CmdBuf.DumpMemToFileCmd.Payload.NumOfBytes");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);
}

void MM_DumpMemToFileCmd_Test_BadType(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    bool              Result;

    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    FcnCode   = MM_DUMP_MEM_TO_FILE_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    strncpy(UT_CmdBuf.DumpMemToFileCmd.Payload.SrcSymAddress.SymName, "SymName",
            sizeof(UT_CmdBuf.DumpMemToFileCmd.Payload.SrcSymAddress.SymName) - 1);
    UT_CmdBuf.DumpMemToFileCmd.Payload.SrcSymAddress.Offset = (cpuaddr)&Buffer[0];

    UT_CmdBuf.DumpMemToFileCmd.Payload.MemType    = 99;
    UT_CmdBuf.DumpMemToFileCmd.Payload.NumOfBytes = 1;

    strncpy(UT_CmdBuf.DumpMemToFileCmd.Payload.FileName, "filename", sizeof(UT_CmdBuf.DumpMemToFileCmd.Payload.FileName) - 1);

    /* Causes call to MM_ResolveSymAddr to return a known value for DestAddress */
    UT_SetHookFunction(UT_KEY(MM_ResolveSymAddr), UT_MM_LOAD_TEST_CFE_SymbolLookupHook1, 0);

    /* Set to satisfy 2 instances of condition "Valid == true": after comment "Write the file headers" and comment "end
     * Valid == true if" */
    UT_SetDefaultReturnValue(UT_KEY(CFE_FS_WriteHeader), sizeof(CFE_FS_Header_t));

    /* Set to satisfy condition "Valid == true" before comment "Compute CRC of dumped data" */
    UT_SetDeferredRetcode(UT_KEY(OS_write), 1, sizeof(MM_LoadDumpFileHeader_t));

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), true);

    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), true);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    /* Execute the function being tested */
    Result = MM_DumpMemToFileCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);
}

void MM_DumpMemToFileCmd_Test_EEPROM(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    int32             strCmpResult;
    char              ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool              Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Dump Memory To File Command: Dumped %%d bytes from address %%p to file '%%s'");

    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    FcnCode   = MM_DUMP_MEM_TO_FILE_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    strncpy(UT_CmdBuf.DumpMemToFileCmd.Payload.SrcSymAddress.SymName, "SymName",
            sizeof(UT_CmdBuf.DumpMemToFileCmd.Payload.SrcSymAddress.SymName) - 1);
    UT_CmdBuf.DumpMemToFileCmd.Payload.SrcSymAddress.Offset = (cpuaddr)&Buffer[0];

    UT_CmdBuf.DumpMemToFileCmd.Payload.MemType    = MM_EEPROM;
    UT_CmdBuf.DumpMemToFileCmd.Payload.NumOfBytes = 1;

    strncpy(UT_CmdBuf.DumpMemToFileCmd.Payload.FileName, "filename", sizeof(UT_CmdBuf.DumpMemToFileCmd.Payload.FileName) - 1);

    /* Causes call to MM_ResolveSymAddr to return a known value for DestAddress */
    UT_SetHookFunction(UT_KEY(MM_ResolveSymAddr), UT_MM_LOAD_TEST_CFE_SymbolLookupHook1, 0);

    /* Set to satisfy 2 instances of condition "Valid == true": after comment "Write the file headers" and comment "end
     * Valid == true if" */
    UT_SetDefaultReturnValue(UT_KEY(CFE_FS_WriteHeader), sizeof(CFE_FS_Header_t));

    /* Set to satisfy condition "Valid == true" before comment "Compute CRC of dumped data" */
    UT_SetDeferredRetcode(UT_KEY(OS_write), 1, sizeof(MM_LoadDumpFileHeader_t));

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), true);

    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), true);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    /* Execute the function being tested */
    Result = MM_DumpMemToFileCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_DMP_MEM_FILE_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_True(MM_AppData.HkPacket.Payload.LastAction == MM_DUMP_TO_FILE,
                  "MM_AppData.HkPacket.Payload.LastAction == MM_DUMP_TO_FILE");
    UtAssert_STRINGBUF_EQ(MM_AppData.HkPacket.Payload.FileName, sizeof(MM_AppData.HkPacket.Payload.FileName),
                          UT_CmdBuf.DumpMemToFileCmd.Payload.FileName, sizeof(UT_CmdBuf.DumpMemToFileCmd.Payload.FileName));
    UtAssert_True(MM_AppData.HkPacket.Payload.MemType == UT_CmdBuf.DumpMemToFileCmd.Payload.MemType,
                  "MM_AppData.HkPacket.Payload.MemType == UT_CmdBuf.DumpMemToFileCmd.Payload.MemType");
    UtAssert_True(MM_AppData.HkPacket.Payload.BytesProcessed == UT_CmdBuf.DumpMemToFileCmd.Payload.NumOfBytes,
                  "MM_AppData.HkPacket.Payload.BytesProcessed == UT_CmdBuf.DumpMemToFileCmd.Payload.NumOfBytes");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);
}

void MM_DumpMemToFileCmd_Test_MEM32(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    int32             strCmpResult;
    char              ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool              Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Dump Memory To File Command: Dumped %%d bytes from address %%p to file '%%s'");

    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    FcnCode   = MM_DUMP_MEM_TO_FILE_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    strncpy(UT_CmdBuf.DumpMemToFileCmd.Payload.SrcSymAddress.SymName, "SymName",
            sizeof(UT_CmdBuf.DumpMemToFileCmd.Payload.SrcSymAddress.SymName) - 1);
    UT_CmdBuf.DumpMemToFileCmd.Payload.SrcSymAddress.Offset = 0;

    UT_CmdBuf.DumpMemToFileCmd.Payload.MemType    = MM_MEM32;
    UT_CmdBuf.DumpMemToFileCmd.Payload.NumOfBytes = 4;

    strncpy(UT_CmdBuf.DumpMemToFileCmd.Payload.FileName, "filename", sizeof(UT_CmdBuf.DumpMemToFileCmd.Payload.FileName) - 1);

    /* Set to satisfy 2 instances of condition "Valid == true": after comment "Write the file headers" and comment "end
     * Valid == true if" */
    UT_SetDefaultReturnValue(UT_KEY(CFE_FS_WriteHeader), sizeof(CFE_FS_Header_t));

    /* Set to satisfy condition "Valid == true" before comment "Compute CRC of dumped data" */
    UT_SetDeferredRetcode(UT_KEY(OS_write), 1, sizeof(MM_LoadDumpFileHeader_t));

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), true);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), true);
    UT_SetDefaultReturnValue(UT_KEY(CFE_PSP_MemValidateRange), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_DumpMem32ToFile), true);
    UT_SetDefaultReturnValue(UT_KEY(MM_Verify32Aligned), true);

    /* Execute the function being tested */
    Result = MM_DumpMemToFileCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_DMP_MEM_FILE_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_True(MM_AppData.HkPacket.Payload.LastAction == MM_DUMP_TO_FILE,
                  "MM_AppData.HkPacket.Payload.LastAction == MM_DUMP_TO_FILE");
    UtAssert_STRINGBUF_EQ(MM_AppData.HkPacket.Payload.FileName, sizeof(MM_AppData.HkPacket.Payload.FileName),
                          UT_CmdBuf.DumpMemToFileCmd.Payload.FileName, sizeof(UT_CmdBuf.DumpMemToFileCmd.Payload.FileName));
    UtAssert_True(MM_AppData.HkPacket.Payload.MemType == UT_CmdBuf.DumpMemToFileCmd.Payload.MemType,
                  "MM_AppData.HkPacket.Payload.MemType == UT_CmdBuf.DumpMemToFileCmd.Payload.MemType");
    UtAssert_True(MM_AppData.HkPacket.Payload.BytesProcessed == UT_CmdBuf.DumpMemToFileCmd.Payload.NumOfBytes,
                  "MM_AppData.HkPacket.Payload.BytesProcessed == UT_CmdBuf.DumpMemToFileCmd.Payload.NumOfBytes");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);
}

void MM_DumpMemToFileCmd_Test_MEM16(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    int32             strCmpResult;
    char              ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool              Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Dump Memory To File Command: Dumped %%d bytes from address %%p to file '%%s'");

    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    FcnCode   = MM_DUMP_MEM_TO_FILE_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    strncpy(UT_CmdBuf.DumpMemToFileCmd.Payload.SrcSymAddress.SymName, "SymName",
            sizeof(UT_CmdBuf.DumpMemToFileCmd.Payload.SrcSymAddress.SymName) - 1);
    UT_CmdBuf.DumpMemToFileCmd.Payload.SrcSymAddress.Offset = 0;

    UT_CmdBuf.DumpMemToFileCmd.Payload.MemType    = MM_MEM16;
    UT_CmdBuf.DumpMemToFileCmd.Payload.NumOfBytes = 2;

    strncpy(UT_CmdBuf.DumpMemToFileCmd.Payload.FileName, "filename", sizeof(UT_CmdBuf.DumpMemToFileCmd.Payload.FileName) - 1);

    /* Set to satisfy 2 instances of condition "Valid == true": after comment "Write the file headers" and comment "end
     * Valid == true if" */
    UT_SetDefaultReturnValue(UT_KEY(CFE_FS_WriteHeader), sizeof(CFE_FS_Header_t));

    /* Set to satisfy condition "Valid == true" before comment "Compute CRC of dumped data" */
    UT_SetDeferredRetcode(UT_KEY(OS_write), 1, sizeof(MM_LoadDumpFileHeader_t));

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), true);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), true);
    UT_SetDefaultReturnValue(UT_KEY(CFE_PSP_MemValidateRange), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_DumpMem16ToFile), true);
    UT_SetDefaultReturnValue(UT_KEY(MM_Verify16Aligned), true);

    /* Execute the function being tested */
    Result = MM_DumpMemToFileCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_DMP_MEM_FILE_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_True(MM_AppData.HkPacket.Payload.LastAction == MM_DUMP_TO_FILE,
                  "MM_AppData.HkPacket.Payload.LastAction == MM_DUMP_TO_FILE");
    UtAssert_STRINGBUF_EQ(MM_AppData.HkPacket.Payload.FileName, sizeof(MM_AppData.HkPacket.Payload.FileName),
                          UT_CmdBuf.DumpMemToFileCmd.Payload.FileName, sizeof(UT_CmdBuf.DumpMemToFileCmd.Payload.FileName));
    UtAssert_True(MM_AppData.HkPacket.Payload.MemType == UT_CmdBuf.DumpMemToFileCmd.Payload.MemType,
                  "MM_AppData.HkPacket.Payload.MemType == UT_CmdBuf.DumpMemToFileCmd.Payload.MemType");
    UtAssert_True(MM_AppData.HkPacket.Payload.BytesProcessed == UT_CmdBuf.DumpMemToFileCmd.Payload.NumOfBytes,
                  "MM_AppData.HkPacket.Payload.BytesProcessed == UT_CmdBuf.DumpMemToFileCmd.Payload.NumOfBytes");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);
}

void MM_DumpMemToFileCmd_Test_MEM8(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    int32             strCmpResult;
    char              ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool              Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Dump Memory To File Command: Dumped %%d bytes from address %%p to file '%%s'");

    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    FcnCode   = MM_DUMP_MEM_TO_FILE_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    strncpy(UT_CmdBuf.DumpMemToFileCmd.Payload.SrcSymAddress.SymName, "SymName",
            sizeof(UT_CmdBuf.DumpMemToFileCmd.Payload.SrcSymAddress.SymName) - 1);
    UT_CmdBuf.DumpMemToFileCmd.Payload.SrcSymAddress.Offset = 0;

    UT_CmdBuf.DumpMemToFileCmd.Payload.MemType    = MM_MEM8;
    UT_CmdBuf.DumpMemToFileCmd.Payload.NumOfBytes = 1;

    strncpy(UT_CmdBuf.DumpMemToFileCmd.Payload.FileName, "filename", sizeof(UT_CmdBuf.DumpMemToFileCmd.Payload.FileName) - 1);

    /* Set to satisfy 2 instances of condition "Valid == true": after comment "Write the file headers" and comment "end
     * Valid == true if" */
    UT_SetDefaultReturnValue(UT_KEY(CFE_FS_WriteHeader), sizeof(CFE_FS_Header_t));

    /* Set to satisfy condition "Valid == true" before comment "Compute CRC of dumped data" */
    UT_SetDeferredRetcode(UT_KEY(OS_write), 1, sizeof(MM_LoadDumpFileHeader_t));

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), true);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), true);
    UT_SetDefaultReturnValue(UT_KEY(CFE_PSP_MemValidateRange), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_DumpMem8ToFile), true);

    /* Execute the function being tested */
    Result = MM_DumpMemToFileCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_DMP_MEM_FILE_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_True(MM_AppData.HkPacket.Payload.LastAction == MM_DUMP_TO_FILE,
                  "MM_AppData.HkPacket.Payload.LastAction == MM_DUMP_TO_FILE");
    UtAssert_STRINGBUF_EQ(MM_AppData.HkPacket.Payload.FileName, sizeof(MM_AppData.HkPacket.Payload.FileName),
                          UT_CmdBuf.DumpMemToFileCmd.Payload.FileName, sizeof(UT_CmdBuf.DumpMemToFileCmd.Payload.FileName));
    UtAssert_True(MM_AppData.HkPacket.Payload.MemType == UT_CmdBuf.DumpMemToFileCmd.Payload.MemType,
                  "MM_AppData.HkPacket.Payload.MemType == UT_CmdBuf.DumpMemToFileCmd.Payload.MemType");
    UtAssert_True(MM_AppData.HkPacket.Payload.BytesProcessed == UT_CmdBuf.DumpMemToFileCmd.Payload.NumOfBytes,
                  "MM_AppData.HkPacket.Payload.BytesProcessed == UT_CmdBuf.DumpMemToFileCmd.Payload.NumOfBytes");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);
}

void MM_DumpMemToFileCmd_Test_ComputeCRCError(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    int32             strCmpResult;
    char              ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool              Result;

    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "MM_ComputeCRCFromFile error received: RC = 0x%%08X File = '%%s'");

    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    FcnCode   = MM_DUMP_MEM_TO_FILE_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    strncpy(UT_CmdBuf.DumpMemToFileCmd.Payload.SrcSymAddress.SymName, "SymName",
            sizeof(UT_CmdBuf.DumpMemToFileCmd.Payload.SrcSymAddress.SymName) - 1);
    UT_CmdBuf.DumpMemToFileCmd.Payload.SrcSymAddress.Offset = 0;

    UT_CmdBuf.DumpMemToFileCmd.Payload.MemType    = MM_MEM8;
    UT_CmdBuf.DumpMemToFileCmd.Payload.NumOfBytes = 1;

    strncpy(UT_CmdBuf.DumpMemToFileCmd.Payload.FileName, "filename", sizeof(UT_CmdBuf.DumpMemToFileCmd.Payload.FileName) - 1);

    /* Set to satisfy 2 instances of condition "Valid == true": after comment "Write the file headers" and comment "end
     * Valid == true if" */
    UT_SetDefaultReturnValue(UT_KEY(CFE_FS_WriteHeader), sizeof(CFE_FS_Header_t));

    /* Set to satisfy condition "Valid == true" before comment "Compute CRC of dumped data" */
    UT_SetDeferredRetcode(UT_KEY(OS_write), 1, sizeof(MM_LoadDumpFileHeader_t));

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), true);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), true);
    UT_SetDefaultReturnValue(UT_KEY(CFE_PSP_MemValidateRange), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_DumpMem8ToFile), true);

    /* Set to generate error message MM_COMPUTECRCFROMFILE_ERR_EID */
    UT_SetDefaultReturnValue(UT_KEY(MM_ComputeCRCFromFile), -1);

    /* Execute the function being tested */
    Result = MM_DumpMemToFileCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_COMPUTECRCFROMFILE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);
}

void MM_DumpMemToFileCmd_Test_CloseError(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    int32             strCmpResult;
    char              ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool              Result;

    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Dump Memory To File Command: Dumped %%d bytes from address %%p to file '%%s'");

    snprintf(ExpectedEventString[1], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "OS_close error received: RC = 0x%%08X File = '%%s'");

    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    FcnCode   = MM_DUMP_MEM_TO_FILE_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    strncpy(UT_CmdBuf.DumpMemToFileCmd.Payload.SrcSymAddress.SymName, "SymName",
            sizeof(UT_CmdBuf.DumpMemToFileCmd.Payload.SrcSymAddress.SymName) - 1);
    UT_CmdBuf.DumpMemToFileCmd.Payload.SrcSymAddress.Offset = 0;

    UT_CmdBuf.DumpMemToFileCmd.Payload.MemType    = MM_MEM8;
    UT_CmdBuf.DumpMemToFileCmd.Payload.NumOfBytes = 1;

    strncpy(UT_CmdBuf.DumpMemToFileCmd.Payload.FileName, "filename", sizeof(UT_CmdBuf.DumpMemToFileCmd.Payload.FileName) - 1);

    /* Set to satisfy 2 instances of condition "Valid == true": after comment "Write the file headers" and comment "end
     * Valid == true if" */
    UT_SetDefaultReturnValue(UT_KEY(CFE_FS_WriteHeader), sizeof(CFE_FS_Header_t));

    /* Set to satisfy condition "Valid == true" before comment "Compute CRC of dumped data" */
    UT_SetDeferredRetcode(UT_KEY(OS_write), 1, sizeof(MM_LoadDumpFileHeader_t));

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), true);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), true);
    UT_SetDefaultReturnValue(UT_KEY(CFE_PSP_MemValidateRange), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_DumpMem8ToFile), true);

    /* Set to generate error message MM_OS_CLOSE_ERR_EID */
    UT_SetDefaultReturnValue(UT_KEY(OS_close), -1);

    /* Execute the function being tested */
    Result = MM_DumpMemToFileCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_DMP_MEM_FILE_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, \n'%s' \n'%s'",
                  context_CFE_EVS_SendEvent[0].Spec, ExpectedEventString[0]);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, MM_OS_CLOSE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[1], context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[1].Spec);

    UtAssert_True(MM_AppData.HkPacket.Payload.LastAction == MM_DUMP_TO_FILE,
                  "MM_AppData.HkPacket.Payload.LastAction == MM_DUMP_TO_FILE");
    UtAssert_STRINGBUF_EQ(MM_AppData.HkPacket.Payload.FileName, sizeof(MM_AppData.HkPacket.Payload.FileName),
                          UT_CmdBuf.DumpMemToFileCmd.Payload.FileName, sizeof(UT_CmdBuf.DumpMemToFileCmd.Payload.FileName));
    UtAssert_True(MM_AppData.HkPacket.Payload.MemType == UT_CmdBuf.DumpMemToFileCmd.Payload.MemType,
                  "MM_AppData.HkPacket.Payload.MemType == UT_CmdBuf.DumpMemToFileCmd.Payload.MemType");
    UtAssert_True(MM_AppData.HkPacket.Payload.BytesProcessed == UT_CmdBuf.DumpMemToFileCmd.Payload.NumOfBytes,
                  "MM_AppData.HkPacket.Payload.BytesProcessed == UT_CmdBuf.DumpMemToFileCmd.Payload.NumOfBytes");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 2, "CFE_EVS_SendEvent was called %u time(s), expected 2",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);
}

void MM_DumpMemToFileCmd_Test_CreatError(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    int32             strCmpResult;
    char              ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool              Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "OS_OpenCreate error received: RC = %%d File = '%%s'");

    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    FcnCode   = MM_DUMP_MEM_TO_FILE_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    strncpy(UT_CmdBuf.DumpMemToFileCmd.Payload.SrcSymAddress.SymName, "SymName",
            sizeof(UT_CmdBuf.DumpMemToFileCmd.Payload.SrcSymAddress.SymName) - 1);
    UT_CmdBuf.DumpMemToFileCmd.Payload.SrcSymAddress.Offset = 0;

    UT_CmdBuf.DumpMemToFileCmd.Payload.MemType    = MM_MEM8;
    UT_CmdBuf.DumpMemToFileCmd.Payload.NumOfBytes = 1;

    strncpy(UT_CmdBuf.DumpMemToFileCmd.Payload.FileName, "filename", sizeof(UT_CmdBuf.DumpMemToFileCmd.Payload.FileName) - 1);

    /* Set to satisfy 2 instances of condition "Valid == true": after comment "Write the file headers" and comment "end
     * Valid == true if" */
    UT_SetDefaultReturnValue(UT_KEY(CFE_FS_WriteHeader), sizeof(CFE_FS_Header_t));

    /* Set to satisfy condition "Valid == true" before comment "Compute CRC of dumped data" */
    UT_SetDeferredRetcode(UT_KEY(OS_write), 1, sizeof(MM_LoadDumpFileHeader_t));

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), true);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), true);
    UT_SetDefaultReturnValue(UT_KEY(CFE_PSP_MemValidateRange), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_DumpMem8ToFile), true);

    /* Set to generate error message MM_OS_CREAT_ERR_EID */
    UT_SetDefaultReturnValue(UT_KEY(OS_OpenCreate), -1);

    /* Execute the function being tested */
    Result = MM_DumpMemToFileCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_OS_CREAT_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);
}

void MM_DumpMemToFileCmd_Test_InvalidDumpResult(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    bool              Result;

    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    FcnCode   = MM_DUMP_MEM_TO_FILE_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    strncpy(UT_CmdBuf.DumpMemToFileCmd.Payload.SrcSymAddress.SymName, "SymName",
            sizeof(UT_CmdBuf.DumpMemToFileCmd.Payload.SrcSymAddress.SymName) - 1);
    UT_CmdBuf.DumpMemToFileCmd.Payload.SrcSymAddress.Offset = 0;

    UT_CmdBuf.DumpMemToFileCmd.Payload.MemType    = MM_MEM8;
    UT_CmdBuf.DumpMemToFileCmd.Payload.NumOfBytes = 1;

    strncpy(UT_CmdBuf.DumpMemToFileCmd.Payload.FileName, "filename", sizeof(UT_CmdBuf.DumpMemToFileCmd.Payload.FileName) - 1);

    /* Set to satisfy 2 instances of condition "Valid == true": after comment "Write the file headers" and comment "end
     * Valid == true if" */
    UT_SetDefaultReturnValue(UT_KEY(CFE_FS_WriteHeader), sizeof(CFE_FS_Header_t));

    /* Set to satisfy condition "Valid == false" before comment "Compute CRC of dumped data" */
    UT_SetDeferredRetcode(UT_KEY(MM_DumpMem8ToFile), 1, false);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), true);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), true);
    UT_SetDefaultReturnValue(UT_KEY(CFE_PSP_MemValidateRange), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_DumpMem8ToFile), true);

    /* Execute the function being tested */
    Result = MM_DumpMemToFileCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);
}

void MM_DumpMemToFileCmd_Test_lseekError(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    bool              Result;

    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    FcnCode   = MM_DUMP_MEM_TO_FILE_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    strncpy(UT_CmdBuf.DumpMemToFileCmd.Payload.SrcSymAddress.SymName, "SymName",
            sizeof(UT_CmdBuf.DumpMemToFileCmd.Payload.SrcSymAddress.SymName) - 1);
    UT_CmdBuf.DumpMemToFileCmd.Payload.SrcSymAddress.Offset = 0;

    UT_CmdBuf.DumpMemToFileCmd.Payload.MemType    = MM_MEM8;
    UT_CmdBuf.DumpMemToFileCmd.Payload.NumOfBytes = 1;

    strncpy(UT_CmdBuf.DumpMemToFileCmd.Payload.FileName, "filename", sizeof(UT_CmdBuf.DumpMemToFileCmd.Payload.FileName) - 1);

    /* Set to satisfy 2 instances of condition "Valid == true": after comment "Write the file headers" and comment "end
     * Valid == true if" */
    UT_SetDefaultReturnValue(UT_KEY(CFE_FS_WriteHeader), sizeof(CFE_FS_Header_t));

    /* Set to satisfy condition "Valid == false" before comment "Compute CRC of dumped data" */
    UT_SetDeferredRetcode(UT_KEY(OS_lseek), 1, -1);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), true);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), true);
    UT_SetDefaultReturnValue(UT_KEY(CFE_PSP_MemValidateRange), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_DumpMem8ToFile), true);

    /* Execute the function being tested */
    Result = MM_DumpMemToFileCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);
}

void MM_DumpMemToFileCmd_Test_SymNameError(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    int32             strCmpResult;
    char              ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool              Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Symbolic address can't be resolved: Name = '%%s'");

    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    FcnCode   = MM_DUMP_MEM_TO_FILE_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    strncpy(UT_CmdBuf.DumpMemToFileCmd.Payload.SrcSymAddress.SymName, "SymName",
            sizeof(UT_CmdBuf.DumpMemToFileCmd.Payload.SrcSymAddress.SymName) - 1);
    UT_CmdBuf.DumpMemToFileCmd.Payload.SrcSymAddress.Offset = 0;

    UT_CmdBuf.DumpMemToFileCmd.Payload.MemType    = MM_MEM8;
    UT_CmdBuf.DumpMemToFileCmd.Payload.NumOfBytes = 1;

    strncpy(UT_CmdBuf.DumpMemToFileCmd.Payload.FileName, "filename", sizeof(UT_CmdBuf.DumpMemToFileCmd.Payload.FileName) - 1);

    /* Set to satisfy 2 instances of condition "Valid == true": after comment "Write the file headers" and comment "end
     * Valid == true if" */
    UT_SetDefaultReturnValue(UT_KEY(CFE_FS_WriteHeader), sizeof(CFE_FS_Header_t));

    /* Set to satisfy condition "Valid == true" before comment "Compute CRC of dumped data" */
    UT_SetDeferredRetcode(UT_KEY(OS_write), 1, sizeof(MM_LoadDumpFileHeader_t));

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    /* Set to generate error message MM_SYMNAME_ERR_EID */
    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), false);

    /* Execute the function being tested */
    Result = MM_DumpMemToFileCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_SYMNAME_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);
}

void MM_DumpMemToFileCmd_Test_NoVerifyDumpParams(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    bool              Result;

    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    FcnCode   = MM_DUMP_MEM_TO_FILE_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    strncpy(UT_CmdBuf.DumpMemToFileCmd.Payload.SrcSymAddress.SymName, "SymName",
            sizeof(UT_CmdBuf.DumpMemToFileCmd.Payload.SrcSymAddress.SymName) - 1);
    UT_CmdBuf.DumpMemToFileCmd.Payload.SrcSymAddress.Offset = 0;

    UT_CmdBuf.DumpMemToFileCmd.Payload.MemType = MM_RAM;
    /* Set to fail MM_VerifyFileDumpParams */

    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), false);
    UT_CmdBuf.DumpMemToFileCmd.Payload.NumOfBytes = 0;

    strncpy(UT_CmdBuf.DumpMemToFileCmd.Payload.FileName, "filename", sizeof(UT_CmdBuf.DumpMemToFileCmd.Payload.FileName) - 1);

    /* Set to satisfy 2 instances of condition "Valid == true": after comment "Write the file headers" and comment "end
     * Valid == true if" */
    UT_SetDefaultReturnValue(UT_KEY(CFE_FS_WriteHeader), sizeof(CFE_FS_Header_t));

    /* Set to satisfy condition "Valid == true" before comment "Compute CRC of dumped data" */
    UT_SetDeferredRetcode(UT_KEY(OS_write), 1, sizeof(MM_LoadDumpFileHeader_t));

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), true);

    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), false);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    /* Execute the function being tested */
    Result = MM_DumpMemToFileCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_True(MM_AppData.HkPacket.Payload.BytesProcessed == UT_CmdBuf.DumpMemToFileCmd.Payload.NumOfBytes,
                  "MM_AppData.HkPacket.Payload.BytesProcessed == UT_CmdBuf.DumpMemToFileCmd.Payload.NumOfBytes");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);
}

void MM_DumpMemToFileCmd_Test_NoWriteHeaders(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    int32             strCmpResult;
    char              ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool              Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "CFE_FS_WriteHeader error received: RC = %%d Expected = %%d File = '%%s'");

    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    FcnCode   = MM_DUMP_MEM_TO_FILE_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    strncpy(UT_CmdBuf.DumpMemToFileCmd.Payload.SrcSymAddress.SymName, "SymName",
            sizeof(UT_CmdBuf.DumpMemToFileCmd.Payload.SrcSymAddress.SymName) - 1);
    UT_CmdBuf.DumpMemToFileCmd.Payload.SrcSymAddress.Offset = 0;

    UT_CmdBuf.DumpMemToFileCmd.Payload.MemType    = MM_RAM;
    UT_CmdBuf.DumpMemToFileCmd.Payload.NumOfBytes = 1;

    strncpy(UT_CmdBuf.DumpMemToFileCmd.Payload.FileName, "filename", sizeof(UT_CmdBuf.DumpMemToFileCmd.Payload.FileName) - 1);

    /* Set to satisfy 2 instances of condition "Valid == true": after comment "Write the file headers" and comment "end
     * Valid == true if" */
    UT_SetDefaultReturnValue(UT_KEY(CFE_FS_WriteHeader), sizeof(CFE_FS_Header_t));

    /* Set to satisfy condition "Valid == true" before comment "Compute CRC of dumped data" */
    UT_SetDeferredRetcode(UT_KEY(OS_write), 1, sizeof(MM_LoadDumpFileHeader_t));

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), true);

    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), true);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    UT_SetDefaultReturnValue(UT_KEY(CFE_FS_WriteHeader), -1);

    /* Execute the function being tested */
    Result = MM_DumpMemToFileCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_CFE_FS_WRITEHDR_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);
}

void MM_DumpMemToFile_Test_Nominal(void)
{
    osal_id_t               FileHandle = MM_UT_OBJID_1;
    char                    FileName[OS_MAX_PATH_LEN];
    MM_LoadDumpFileHeader_t FileHeader;
    bool                    Result;

    strncpy(FileName, "filename", sizeof(FileName) - 1);
    FileName[sizeof(FileName) - 1] = '\0';

    FileHeader.NumOfBytes = 1;
    /* a valid source address is required input to memcpy */
    FileHeader.SymAddress.Offset = (cpuaddr)&MM_AppData.LoadBuffer[0];
    FileHeader.MemType           = MM_RAM;

    /* Execute the function being tested */
    Result = MM_DumpMemToFile(FileHandle, FileName, &FileHeader);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");
    UtAssert_True(MM_AppData.HkPacket.Payload.LastAction == MM_DUMP_TO_FILE,
                  "MM_AppData.HkPacket.Payload.LastAction == MM_DUMP_TO_FILE");
    UtAssert_True(MM_AppData.HkPacket.Payload.MemType == FileHeader.MemType,
                  "MM_AppData.HkPacket.Payload.MemType == FileHeader.MemType");
    UtAssert_True(MM_AppData.HkPacket.Payload.Address == (cpuaddr)FileHeader.SymAddress.Offset,
                  "MM_AppData.HkPacket.Payload.Address == FileHeader.SymAddress.Offset");
    UtAssert_True(MM_AppData.HkPacket.Payload.BytesProcessed == 1, "MM_AppData.HkPacket.Payload.BytesProcessed == 1");
    UtAssert_STRINGBUF_EQ(MM_AppData.HkPacket.Payload.FileName, sizeof(MM_AppData.HkPacket.Payload.FileName), FileName,
                          sizeof(FileName));

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);
}

void MM_DumpMemToFile_Test_CPUHogging(void)
{
    osal_id_t               FileHandle = MM_UT_OBJID_1;
    char                    FileName[OS_MAX_PATH_LEN];
    MM_LoadDumpFileHeader_t FileHeader;
    bool                    Result;
    char                    Data[2 * MM_MAX_DUMP_DATA_SEG] = {0};

    strncpy(FileName, "filename", sizeof(FileName) - 1);
    FileName[sizeof(FileName) - 1] = '\0';

    FileHeader.NumOfBytes = sizeof(Data);
    /* a valid source address is required input to memcpy */
    FileHeader.SymAddress.Offset = (cpuaddr)Data;
    FileHeader.MemType           = MM_RAM;

    /* Execute the function being tested */
    Result = MM_DumpMemToFile(FileHandle, FileName, &FileHeader);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");
    UtAssert_True(MM_AppData.HkPacket.Payload.LastAction == MM_DUMP_TO_FILE,
                  "MM_AppData.HkPacket.Payload.LastAction == MM_DUMP_TO_FILE");
    UtAssert_True(MM_AppData.HkPacket.Payload.MemType == FileHeader.MemType,
                  "MM_AppData.HkPacket.Payload.MemType == FileHeader.MemType");
    UtAssert_True(MM_AppData.HkPacket.Payload.Address == (cpuaddr)FileHeader.SymAddress.Offset,
                  "MM_AppData.HkPacket.Payload.Address == FileHeader.SymAddress.Offset");
    UtAssert_True(MM_AppData.HkPacket.Payload.BytesProcessed == 2 * MM_MAX_DUMP_DATA_SEG,
                  "MM_AppData.HkPacket.Payload.BytesProcessed == 2 * MM_MAX_DUMP_DATA_SEG");
    UtAssert_STRINGBUF_EQ(MM_AppData.HkPacket.Payload.FileName, sizeof(MM_AppData.HkPacket.Payload.FileName), FileName,
                          sizeof(FileName));

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);
}

void MM_DumpMemToFile_Test_WriteError(void)
{
    osal_id_t               FileHandle = MM_UT_OBJID_1;
    char                    FileName[OS_MAX_PATH_LEN];
    MM_LoadDumpFileHeader_t FileHeader;
    int32                   strCmpResult;
    char                    ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool                    Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "OS_write error received: RC = %%d, Expected = %%u, File = '%%s'");

    strncpy(FileName, "filename", sizeof(FileName) - 1);
    FileName[sizeof(FileName) - 1] = '\0';

    FileHeader.NumOfBytes = 1;
    /* a valid source address is required input to memcpy */
    FileHeader.SymAddress.Offset = (cpuaddr)&MM_AppData.LoadBuffer[0];
    FileHeader.MemType           = MM_RAM;

    /* Set to generate error message MM_OS_WRITE_EXP_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(OS_write), 1, -1);

    /* Execute the function being tested */
    Result = MM_DumpMemToFile(FileHandle, FileName, &FileHeader);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_OS_WRITE_EXP_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);
}

void MM_WriteFileHeaders_Test_Nominal(void)
{
    char                    FileName[OS_MAX_PATH_LEN];
    osal_id_t               FileHandle = MM_UT_OBJID_1;
    CFE_FS_Header_t         CFEHeader;
    MM_LoadDumpFileHeader_t MMHeader;
    bool                    Result;

    strncpy(FileName, "filename", sizeof(FileName) - 1);
    FileName[sizeof(FileName) - 1] = '\0';

    MMHeader.NumOfBytes        = 1;
    MMHeader.SymAddress.Offset = 0;
    MMHeader.MemType           = MM_RAM;

    UT_SetDefaultReturnValue(UT_KEY(CFE_FS_WriteHeader), sizeof(CFE_FS_Header_t));

    /* Execute the function being tested */
    Result = MM_WriteFileHeaders(FileName, FileHandle, &CFEHeader, &MMHeader);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);
}

void MM_WriteFileHeaders_Test_WriteHeaderError(void)
{
    char                    FileName[OS_MAX_PATH_LEN];
    osal_id_t               FileHandle = MM_UT_OBJID_1;
    CFE_FS_Header_t         CFEHeader;
    MM_LoadDumpFileHeader_t MMHeader;
    int32                   strCmpResult;
    char                    ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool                    Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "CFE_FS_WriteHeader error received: RC = %%d Expected = %%d File = '%%s'");

    strncpy(FileName, "filename", sizeof(FileName) - 1);
    FileName[sizeof(FileName) - 1] = '\0';

    MMHeader.NumOfBytes        = 1;
    MMHeader.SymAddress.Offset = 0;
    MMHeader.MemType           = MM_RAM;

    /* Set to generate error message MM_CFE_FS_WRITEHDR_ERR_EID */
    UT_SetDefaultReturnValue(UT_KEY(CFE_FS_WriteHeader), -1);

    /* Execute the function being tested */
    Result = MM_WriteFileHeaders(FileName, FileHandle, &CFEHeader, &MMHeader);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_CFE_FS_WRITEHDR_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);
}

void MM_WriteFileHeaders_Test_WriteError(void)
{
    char                    FileName[OS_MAX_PATH_LEN];
    osal_id_t               FileHandle = MM_UT_OBJID_1;
    CFE_FS_Header_t         CFEHeader;
    MM_LoadDumpFileHeader_t MMHeader;
    int32                   strCmpResult;
    char                    ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool                    Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "OS_write error received: RC = %%d Expected = %%u File = '%%s'");

    strncpy(FileName, "filename", sizeof(FileName) - 1);
    FileName[sizeof(FileName) - 1] = '\0';

    MMHeader.NumOfBytes        = 1;
    MMHeader.SymAddress.Offset = 0;
    MMHeader.MemType           = MM_RAM;

    /* Set to generate error message MM_CFE_FS_WRITEHDR_ERR_EID */
    UT_SetDefaultReturnValue(UT_KEY(OS_write), -1);

    /* Execute the function being tested */
    Result = MM_WriteFileHeaders(FileName, FileHandle, &CFEHeader, &MMHeader);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_OS_WRITE_EXP_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);
}

void MM_DumpInEventCmd_Test_Nominal(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    int32             strCmpResult;
    char              ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool              Result;

    /* The event string is dynamically generated with string contatentation
       and is reported in context_CFE_EVS_SendEvent as a single string.
       The event string cannot be matched to a preformatted value. */
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "%%s");

    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    FcnCode   = MM_DUMP_IN_EVENT_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_CmdBuf.DumpInEventCmd.Payload.MemType              = MM_RAM;
    UT_CmdBuf.DumpInEventCmd.Payload.NumOfBytes           = 1;
    UT_CmdBuf.DumpInEventCmd.Payload.SrcSymAddress.Offset = (cpuaddr)&Buffer[0];

    UT_CmdBuf.DumpInEventCmd.Payload.SrcSymAddress.SymName[0] = '\0';

    /* Causes call to MM_ResolveSymAddr to return a known value for DestAddress */
    UT_SetHookFunction(UT_KEY(MM_ResolveSymAddr), UT_MM_LOAD_TEST_CFE_SymbolLookupHook1, 0);

    /* Set to satisfy 2 instances of condition "Valid == true": after comment "Write the file headers" and comment "end
     * Valid == true if" */
    UT_SetDefaultReturnValue(UT_KEY(CFE_FS_WriteHeader), sizeof(CFE_FS_Header_t));

    /* Set to satisfy condition "Valid == true" before comment "Compute CRC of dumped data" */
    UT_SetDeferredRetcode(UT_KEY(OS_write), 1, sizeof(MM_LoadDumpFileHeader_t));

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);
    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), true);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), true);

    /* Execute the function being tested */
    Result = MM_DumpInEventCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    UtAssert_True(MM_AppData.HkPacket.Payload.LastAction == MM_DUMP_INEVENT,
                  "MM_AppData.HkPacket.Payload.LastAction == MM_DUMP_INEVENT");
    UtAssert_True(MM_AppData.HkPacket.Payload.MemType == MM_RAM, "MM_AppData.HkPacket.Payload.MemType == MM_RAM");
    UtAssert_True(MM_AppData.HkPacket.Payload.Address == (cpuaddr)UT_CmdBuf.DumpInEventCmd.Payload.SrcSymAddress.Offset,
                  "MM_AppData.HkPacketAddress == FileHeader.SymAddress.Offset");
    UtAssert_True(MM_AppData.HkPacket.Payload.BytesProcessed == 1, "MM_AppData.HkPacket.Payload.BytesProcessed == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_DUMP_INEVENT_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s' '%s'",
                  context_CFE_EVS_SendEvent[0].Spec, ExpectedEventString);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);
}

void MM_DumpInEventCmd_Test_SymNameError(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    int32             strCmpResult;
    char              ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool              Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Symbolic address can't be resolved: Name = '%%s'");

    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    FcnCode   = MM_DUMP_IN_EVENT_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_CmdBuf.DumpInEventCmd.Payload.MemType              = MM_RAM;
    UT_CmdBuf.DumpInEventCmd.Payload.NumOfBytes           = 1;
    UT_CmdBuf.DumpInEventCmd.Payload.SrcSymAddress.Offset = 0;

    strncpy(UT_CmdBuf.DumpInEventCmd.Payload.SrcSymAddress.SymName, "name",
            sizeof(UT_CmdBuf.DumpInEventCmd.Payload.SrcSymAddress.SymName) - 1);

    /* Set to generate error message MM_SYMNAME_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(OS_SymbolLookup), 1, -1);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), true);

    /* Execute the function being tested */
    Result = MM_DumpInEventCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_SYMNAME_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);
}

void MM_DumpInEventCmd_Test_NoVerifyDumpParams(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    bool              Result;

    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    FcnCode   = MM_DUMP_IN_EVENT_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_CmdBuf.DumpInEventCmd.Payload.MemType              = MM_RAM;
    UT_CmdBuf.DumpInEventCmd.Payload.NumOfBytes           = 0;
    UT_CmdBuf.DumpInEventCmd.Payload.SrcSymAddress.Offset = 0;

    strncpy(UT_CmdBuf.DumpInEventCmd.Payload.SrcSymAddress.SymName, "name",
            sizeof(UT_CmdBuf.DumpInEventCmd.Payload.SrcSymAddress.SymName) - 1);

    UT_SetDeferredRetcode(UT_KEY(MM_ResolveSymAddr), 1, true);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    /* Execute the function being tested */
    Result = MM_DumpInEventCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);
}

void MM_DumpInEventCmd_Test_FillDumpInvalid(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    bool              Result;

    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    FcnCode   = MM_DUMP_IN_EVENT_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_CmdBuf.DumpInEventCmd.Payload.MemType              = MM_MEM8;
    UT_CmdBuf.DumpInEventCmd.Payload.NumOfBytes           = 1;
    UT_CmdBuf.DumpInEventCmd.Payload.SrcSymAddress.Offset = (cpuaddr)&Buffer[0];

    UT_CmdBuf.DumpInEventCmd.Payload.SrcSymAddress.SymName[0] = '\0';

    /* Causes call to MM_ResolveSymAddr to return a known value for DestAddress */
    UT_SetHookFunction(UT_KEY(MM_ResolveSymAddr), UT_MM_LOAD_TEST_CFE_SymbolLookupHook1, 0);

    /* Set to satisfy 2 instances of condition "Valid == true": after comment "Write the file headers" and comment "end
     * Valid == true if" */
    UT_SetDefaultReturnValue(UT_KEY(CFE_FS_WriteHeader), sizeof(CFE_FS_Header_t));

    /* Set to satisfy condition "Valid == true" before comment "Compute CRC of dumped data" */
    UT_SetDeferredRetcode(UT_KEY(OS_write), 1, sizeof(MM_LoadDumpFileHeader_t));

    /* Set to generate error message MM_PSP_READ_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemRead8), 1, -1);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);
    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), true);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), true);

    /* Execute the function being tested */
    Result = MM_DumpInEventCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);
}

void MM_FillDumpInEventBuffer_Test_RAM(void)
{
    MM_DumpInEventCmd_t CmdPacket;
    /* a valid source address is required input to memcpy */
    cpuaddr SrcAddress = (cpuaddr)&MM_AppData.LoadBuffer[0];
    bool    Result;

    CmdPacket.Payload.MemType              = MM_RAM;
    CmdPacket.Payload.NumOfBytes           = 1;
    CmdPacket.Payload.SrcSymAddress.Offset = 0;

    /* Execute the function being tested */
    Result = MM_FillDumpInEventBuffer(SrcAddress, &CmdPacket, (uint8 *)(&MM_AppData.DumpBuffer[0]));

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);
}

void MM_FillDumpInEventBuffer_Test_BadType(void)
{
    MM_DumpInEventCmd_t CmdPacket;
    /* a valid source address is required input to memcpy */
    cpuaddr SrcAddress = (cpuaddr)&MM_AppData.LoadBuffer[0];
    bool    Result;

    CmdPacket.Payload.MemType              = 99;
    CmdPacket.Payload.NumOfBytes           = 1;
    CmdPacket.Payload.SrcSymAddress.Offset = 0;

    /* Execute the function being tested */
    Result = MM_FillDumpInEventBuffer(SrcAddress, &CmdPacket, (uint8 *)(&MM_AppData.DumpBuffer[0]));

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);
}

void MM_FillDumpInEventBuffer_Test_EEPROM(void)
{
    MM_DumpInEventCmd_t CmdPacket;
    /* a valid source address is required input to memcpy */
    cpuaddr SrcAddress = (cpuaddr)&MM_AppData.LoadBuffer[0];
    bool    Result;

    CmdPacket.Payload.MemType              = MM_EEPROM;
    CmdPacket.Payload.NumOfBytes           = 1;
    CmdPacket.Payload.SrcSymAddress.Offset = 0;

    /* Execute the function being tested */
    Result = MM_FillDumpInEventBuffer(SrcAddress, &CmdPacket, (uint8 *)(&MM_AppData.DumpBuffer[0]));

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);
}

void MM_FillDumpInEventBuffer_Test_MEM32(void)
{
    MM_DumpInEventCmd_t CmdPacket;
    /* CFE_PSP_MemRead32 stub returns success with non-zero address */
    cpuaddr SrcAddress = 1;
    bool    Result;

    CmdPacket.Payload.MemType              = MM_MEM32;
    CmdPacket.Payload.NumOfBytes           = 4;
    CmdPacket.Payload.SrcSymAddress.Offset = 0;

    /* Execute the function being tested */
    Result = MM_FillDumpInEventBuffer(SrcAddress, &CmdPacket, (uint8 *)(&MM_AppData.DumpBuffer[0]));

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);
}

void MM_FillDumpInEventBuffer_Test_MEM16(void)
{
    MM_DumpInEventCmd_t CmdPacket;
    /* CFE_PSP_MemRead16 stub returns success with non-zero address */
    cpuaddr SrcAddress = 1;
    bool    Result;

    CmdPacket.Payload.MemType              = MM_MEM16;
    CmdPacket.Payload.NumOfBytes           = 2;
    CmdPacket.Payload.SrcSymAddress.Offset = 0;

    /* Execute the function being tested */
    Result = MM_FillDumpInEventBuffer(SrcAddress, &CmdPacket, (uint8 *)(&MM_AppData.DumpBuffer[0]));

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);
}

void MM_FillDumpInEventBuffer_Test_MEM8(void)
{
    MM_DumpInEventCmd_t CmdPacket;
    cpuaddr             SrcAddress = 0;
    bool                Result;

    CmdPacket.Payload.MemType              = MM_MEM8;
    CmdPacket.Payload.NumOfBytes           = 1;
    CmdPacket.Payload.SrcSymAddress.Offset = 0;

    /* Execute the function being tested */
    Result = MM_FillDumpInEventBuffer(SrcAddress, &CmdPacket, (uint8 *)(&MM_AppData.DumpBuffer[0]));

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);
}

void MM_FillDumpInEventBuffer_Test_MEM32ReadError(void)
{
    MM_DumpInEventCmd_t CmdPacket;
    cpuaddr             SrcAddress = 0;
    int32               strCmpResult;
    char                ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool                Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "PSP read memory error: RC=%%d, Src=%%p, Tgt=%%p, Type=MEM32");

    CmdPacket.Payload.MemType              = MM_MEM32;
    CmdPacket.Payload.NumOfBytes           = 4;
    CmdPacket.Payload.SrcSymAddress.Offset = 0;

    /* Set to generate error message MM_PSP_READ_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemRead32), 1, -1);

    /* Execute the function being tested */
    Result = MM_FillDumpInEventBuffer(SrcAddress, &CmdPacket, (uint8 *)(&MM_AppData.DumpBuffer[0]));

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_PSP_READ_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);
}

void MM_FillDumpInEventBuffer_Test_MEM16ReadError(void)
{
    MM_DumpInEventCmd_t CmdPacket;
    cpuaddr             SrcAddress = 0;
    int32               strCmpResult;
    char                ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool                Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "PSP read memory error: RC=%%d, Src=%%p, Tgt=%%p, Type=MEM16");

    CmdPacket.Payload.MemType              = MM_MEM16;
    CmdPacket.Payload.NumOfBytes           = 4;
    CmdPacket.Payload.SrcSymAddress.Offset = 0;

    /* Set to generate error message MM_PSP_READ_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemRead16), 1, -1);

    /* Execute the function being tested */
    Result = MM_FillDumpInEventBuffer(SrcAddress, &CmdPacket, (uint8 *)(&MM_AppData.DumpBuffer[0]));

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_PSP_READ_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);
}

void MM_FillDumpInEventBuffer_Test_MEM8ReadError(void)
{
    MM_DumpInEventCmd_t CmdPacket;
    cpuaddr             SrcAddress = 0;
    int32               strCmpResult;
    char                ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool                Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "PSP read memory error: RC=%%d, Src=%%p, Tgt=%%p, Type=MEM8");

    CmdPacket.Payload.MemType              = MM_MEM8;
    CmdPacket.Payload.NumOfBytes           = 4;
    CmdPacket.Payload.SrcSymAddress.Offset = 0;

    /* Set to generate error message MM_PSP_READ_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemRead8), 1, -1);

    /* Execute the function being tested */
    Result = MM_FillDumpInEventBuffer(SrcAddress, &CmdPacket, (uint8 *)(&MM_AppData.DumpBuffer[0]));

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_PSP_READ_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.Payload.ErrCounter, 0);
}

/*
 * Register the test cases to execute with the unit test tool
 */
void UtTest_Setup(void)
{
    UtTest_Add(MM_PeekCmd_Test_Nominal, MM_Test_Setup, MM_Test_TearDown, "MM_PeekCmd_Test_Nominal");
    UtTest_Add(MM_PeekCmd_Test_SymNameError, MM_Test_Setup, MM_Test_TearDown, "MM_PeekCmd_Test_SymNameError");
    UtTest_Add(MM_PeekCmd_Test_NoVerifyPeekPokeParams, MM_Test_Setup, MM_Test_TearDown,
               "MM_PeekCmd_Test_NoVerifyPeekPokeParams");
    UtTest_Add(MM_PeekMem_Test_Byte, MM_Test_Setup, MM_Test_TearDown, "MM_PeekMem_Test_Byte");
    UtTest_Add(MM_PeekMem_Test_ByteError, MM_Test_Setup, MM_Test_TearDown, "MM_PeekMem_Test_ByteError");
    UtTest_Add(MM_PeekMem_Test_Word, MM_Test_Setup, MM_Test_TearDown, "MM_PeekMem_Test_Word");
    UtTest_Add(MM_PeekMem_Test_WordError, MM_Test_Setup, MM_Test_TearDown, "MM_PeekMem_Test_WordError");
    UtTest_Add(MM_PeekMem_Test_DWord, MM_Test_Setup, MM_Test_TearDown, "MM_PeekMem_Test_DWord");
    UtTest_Add(MM_PeekMem_Test_DWordError, MM_Test_Setup, MM_Test_TearDown, "MM_PeekMem_Test_DWordError");
    UtTest_Add(MM_PeekMem_Test_DefaultSwitch, MM_Test_Setup, MM_Test_TearDown, "MM_PeekMem_Test_DefaultSwitch");

    UtTest_Add(MM_DumpMemToFileCmd_Test_RAM, MM_Test_Setup, MM_Test_TearDown, "MM_DumpMemToFileCmd_Test_RAM");
    UtTest_Add(MM_DumpMemToFileCmd_Test_BadType, MM_Test_Setup, MM_Test_TearDown, "MM_DumpMemToFileCmd_Test_BadType");
    UtTest_Add(MM_DumpMemToFileCmd_Test_EEPROM, MM_Test_Setup, MM_Test_TearDown, "MM_DumpMemToFileCmd_Test_EEPROM");

    UtTest_Add(MM_DumpMemToFileCmd_Test_MEM32, MM_Test_Setup, MM_Test_TearDown, "MM_DumpMemToFileCmd_Test_MEM32");
    UtTest_Add(MM_DumpMemToFileCmd_Test_MEM16, MM_Test_Setup, MM_Test_TearDown, "MM_DumpMemToFileCmd_Test_MEM16");
    UtTest_Add(MM_DumpMemToFileCmd_Test_MEM8, MM_Test_Setup, MM_Test_TearDown, "MM_DumpMemToFileCmd_Test_MEM8");
    UtTest_Add(MM_DumpMemToFileCmd_Test_ComputeCRCError, MM_Test_Setup, MM_Test_TearDown,
               "MM_DumpMemToFileCmd_Test_ComputeCRCError");
    UtTest_Add(MM_DumpMemToFileCmd_Test_CloseError, MM_Test_Setup, MM_Test_TearDown,
               "MM_DumpMemToFileCmd_Test_CloseError");
    UtTest_Add(MM_DumpMemToFileCmd_Test_CreatError, MM_Test_Setup, MM_Test_TearDown,
               "MM_DumpMemToFileCmd_Test_CreatError");
    UtTest_Add(MM_DumpMemToFileCmd_Test_InvalidDumpResult, MM_Test_Setup, MM_Test_TearDown,
               "MM_DumpMemToFileCmd_Test_InvalidDumpResult");
    UtTest_Add(MM_DumpMemToFileCmd_Test_lseekError, MM_Test_Setup, MM_Test_TearDown,
               "MM_DumpMemToFileCmd_Test_lseekError");
    UtTest_Add(MM_DumpMemToFileCmd_Test_SymNameError, MM_Test_Setup, MM_Test_TearDown,
               "MM_DumpMemToFileCmd_Test_SymNameError");
    UtTest_Add(MM_DumpMemToFileCmd_Test_NoVerifyDumpParams, MM_Test_Setup, MM_Test_TearDown,
               "MM_DumpMemToFileCmd_Test_NoVerifyDumpParams");
    UtTest_Add(MM_DumpMemToFileCmd_Test_NoWriteHeaders, MM_Test_Setup, MM_Test_TearDown,
               "MM_DumpMemToFileCmd_Test_NoWriteHeaders");

    UtTest_Add(MM_DumpMemToFile_Test_Nominal, MM_Test_Setup, MM_Test_TearDown, "MM_DumpMemToFile_Test_Nominal");
    UtTest_Add(MM_DumpMemToFile_Test_CPUHogging, MM_Test_Setup, MM_Test_TearDown, "MM_DumpMemToFile_Test_CPUHogging");
    UtTest_Add(MM_DumpMemToFile_Test_WriteError, MM_Test_Setup, MM_Test_TearDown, "MM_DumpMemToFile_Test_WriteError");

    UtTest_Add(MM_WriteFileHeaders_Test_Nominal, MM_Test_Setup, MM_Test_TearDown, "MM_WriteFileHeaders_Test_Nominal");
    UtTest_Add(MM_WriteFileHeaders_Test_WriteHeaderError, MM_Test_Setup, MM_Test_TearDown,
               "MM_WriteFileHeaders_Test_WriteHeaderError");
    UtTest_Add(MM_WriteFileHeaders_Test_WriteError, MM_Test_Setup, MM_Test_TearDown,
               "MM_WriteFileHeaders_Test_WriteError");

    UtTest_Add(MM_DumpInEventCmd_Test_Nominal, MM_Test_Setup, MM_Test_TearDown, "MM_DumpInEventCmd_Test_Nominal");
    UtTest_Add(MM_DumpInEventCmd_Test_SymNameError, MM_Test_Setup, MM_Test_TearDown,
               "MM_DumpInEventCmd_Test_SymNameError");
    UtTest_Add(MM_DumpInEventCmd_Test_NoVerifyDumpParams, MM_Test_Setup, MM_Test_TearDown,
               "MM_DumpInEventCmd_Test_NoVerifyDumpParams");
    UtTest_Add(MM_DumpInEventCmd_Test_FillDumpInvalid, MM_Test_Setup, MM_Test_TearDown,
               "MM_DumpInEventCmd_Test_FillDumpInvalid");

    UtTest_Add(MM_FillDumpInEventBuffer_Test_RAM, MM_Test_Setup, MM_Test_TearDown, "MM_FillDumpInEventBuffer_Test_RAM");
    UtTest_Add(MM_FillDumpInEventBuffer_Test_BadType, MM_Test_Setup, MM_Test_TearDown,
               "MM_FillDumpInEventBuffer_Test_BadType");
    UtTest_Add(MM_FillDumpInEventBuffer_Test_EEPROM, MM_Test_Setup, MM_Test_TearDown,
               "MM_FillDumpInEventBuffer_Test_EEPROM");
    UtTest_Add(MM_FillDumpInEventBuffer_Test_MEM32, MM_Test_Setup, MM_Test_TearDown,
               "MM_FillDumpInEventBuffer_Test_MEM32");
    UtTest_Add(MM_FillDumpInEventBuffer_Test_MEM16, MM_Test_Setup, MM_Test_TearDown,
               "MM_FillDumpInEventBuffer_Test_MEM16");
    UtTest_Add(MM_FillDumpInEventBuffer_Test_MEM8, MM_Test_Setup, MM_Test_TearDown,
               "MM_FillDumpInEventBuffer_Test_MEM8");
    UtTest_Add(MM_FillDumpInEventBuffer_Test_MEM32ReadError, MM_Test_Setup, MM_Test_TearDown,
               "MM_FillDumpInEventBuffer_Test_MEM32ReadError");
    UtTest_Add(MM_FillDumpInEventBuffer_Test_MEM16ReadError, MM_Test_Setup, MM_Test_TearDown,
               "MM_FillDumpInEventBuffer_Test_MEM16ReadError");
    UtTest_Add(MM_FillDumpInEventBuffer_Test_MEM8ReadError, MM_Test_Setup, MM_Test_TearDown,
               "MM_FillDumpInEventBuffer_Test_MEM8ReadError");
}
