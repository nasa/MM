/************************************************************************
** File: mm_dump_tests.c 
**
**   Copyright © 2007-2014 United States Government as represented by the
**   Administrator of the National Aeronautics and Space Administration.
**   All Other Rights Reserved.
**
**   This software was created at NASA's Goddard Space Flight Center.
**   This software is governed by the NASA Open Source Agreement and may be
**   used, distributed and modified only pursuant to the terms of that
**   agreement.
**
** Purpose:
**   Unit tests for mm_dump.c
**
*************************************************************************/

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

#include <sys/fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <cfe.h>
#include "cfe_msgids.h"
#include "cfs_utils.h"

/* mm_dump_tests globals */
uint8 call_count_CFE_EVS_SendEvent;

uint8 DummyBuffer[MM_MAX_FILL_DATA_SEG * 2];

/*
 * Function Definitions
 */

int32 UT_MM_LOAD_TEST_CFE_SymbolLookupHook1(void *UserObj, int32 StubRetcode, uint32 CallCount,
                                            const UT_StubContext_t *Context)
{
    cpuaddr *ResolvedAddress = (cpuaddr *)Context->ArgPtr[1];

    /* provide a valid address for CFE_PSP_MemCpy */
    *ResolvedAddress = (cpuaddr)DummyBuffer;

    return true;
} /* end UT_MM_LOAD_TEST_CFE_SymbolLookupHook1 */

int32 UT_MM_LOAD_TEST_OS_WriteHook1(void *UserObj, int32 StubRetcode, uint32 CallCount, const UT_StubContext_t *Context)
{
    MM_LoadDumpFileHeader_t *header = (MM_LoadDumpFileHeader_t *)Context->ArgPtr[1];

    cpuaddr *ResolvedAddress = (cpuaddr *)&header->SymAddress.Offset;

    *ResolvedAddress = (cpuaddr)DummyBuffer;

    return sizeof(MM_LoadDumpFileHeader_t);
} /* end UT_MM_LOAD_TEST_OS_WriteHook1 */

void MM_PeekCmd_Test_Nominal(void)
{
    MM_PeekCmd_t      CmdPacket;
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    int32             strCmpResult;
    char              ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool              Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Peek Command: Addr = %%p Size = %%d bits Data = 0x%%08X");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent;
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    CmdPacket.MemType  = MM_RAM;
    CmdPacket.DataSize = 32;

    TestMsgId = MM_CMD_MID;
    FcnCode   = MM_PEEK_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    /* ignore dummy message length check */
    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, true);

    UT_SetDeferredRetcode(UT_KEY(CFS_ResolveSymAddr), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyPeekPokeParams), 1, true);

    /* Execute the function being tested */
    Result = MM_PeekCmd((CFE_SB_Buffer_t *)(&CmdPacket));

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventID, MM_PEEK_DWORD_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent.Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_PeekCmd_Test_Nominal */

void MM_PeekCmd_Test_SymNameError(void)
{
    MM_PeekCmd_t      CmdPacket;
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    int32             strCmpResult;
    char              ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool              Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Symbolic address can't be resolved: Name = '%%s'");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent;
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    TestMsgId = MM_CMD_MID;
    FcnCode   = MM_PEEK_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    CmdPacket.MemType  = MM_RAM;
    CmdPacket.DataSize = 32;

    strncpy(CmdPacket.SrcSymAddress.SymName, "name", OS_MAX_PATH_LEN);

    /* Set to generate error message MM_SYMNAME_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(OS_SymbolLookup), 1, -1);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    /* Execute the function being tested */
    Result = MM_PeekCmd((CFE_SB_Buffer_t *)(&CmdPacket));

    /* Verify results */

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventID, MM_SYMNAME_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent.Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

} /* end MM_PeekCmd_Test_SymNameError */

void MM_PeekCmd_Test_NoVerifyPeekPokeParams(void)
{
    MM_PeekCmd_t      CmdPacket;
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    bool              Result;

    CmdPacket.MemType  = MM_RAM;
    CmdPacket.DataSize = 32;

    TestMsgId = MM_CMD_MID;
    FcnCode   = MM_PEEK_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    /* ignore dummy message length check */
    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, true);

    UT_SetDeferredRetcode(UT_KEY(CFS_ResolveSymAddr), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyPeekPokeParams), 1, false);

    /* Execute the function being tested */
    Result = MM_PeekCmd((CFE_SB_Buffer_t *)(&CmdPacket));

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

} /* end MM_PeekCmd_Test_NoVerifyPeekPokeParams */

void MM_PeekMem_Test_Byte(void)
{
    MM_PeekCmd_t CmdPacket;
    uint32       SrcAddress = 1;
    int32        strCmpResult;
    char         ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool         Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Peek Command: Addr = %%p Size = %%d bits Data = 0x%%08X");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent;
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    CmdPacket.DataSize = MM_BYTE_BIT_WIDTH;
    CmdPacket.MemType  = MM_RAM;

    /* Execute the function being tested */
    Result = MM_PeekMem(&CmdPacket, SrcAddress);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventID, MM_PEEK_BYTE_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent.Spec);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

    UtAssert_True(MM_AppData.HkPacket.LastAction == MM_PEEK, "MM_AppData.HkPacket.LastAction == MM_PEEK");
    UtAssert_True(MM_AppData.HkPacket.MemType == MM_RAM, "MM_AppData.HkPacket.MemType == MM_RAM");
    UtAssert_True(MM_AppData.HkPacket.Address == 1, "MM_AppData.HkPacket.Address == 1");
    UtAssert_True(MM_AppData.HkPacket.BytesProcessed == 1, "MM_AppData.HkPacket.BytesProcessed == 1");
    UtAssert_True(MM_AppData.HkPacket.DataValue == 1, "MM_AppData.HkPacket.DataValue == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

} /* end MM_PeekMem_Test_Byte */

void MM_PeekMem_Test_ByteError(void)
{
    MM_PeekCmd_t CmdPacket;
    uint32       SrcAddress = 1;
    int32        strCmpResult;
    char         ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool         Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "PSP read memory error: RC=%%d, Address=%%p, MemType=MEM%%d");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent;
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    CmdPacket.DataSize = MM_BYTE_BIT_WIDTH;

    /* Set to generate error message MM_PSP_READ_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemRead8), 1, -1);

    /* Execute the function being tested */
    Result = MM_PeekMem(&CmdPacket, SrcAddress);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent.Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

} /* end MM_PeekMem_Test_ByteError */

void MM_PeekMem_Test_Word(void)
{
    MM_PeekCmd_t CmdPacket;
    uint32       SrcAddress = 1;
    int32        strCmpResult;
    char         ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool         Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Peek Command: Addr = %%p Size = %%d bits Data = 0x%%08X");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent;
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    CmdPacket.DataSize = MM_WORD_BIT_WIDTH;
    CmdPacket.MemType  = MM_RAM;

    /* Execute the function being tested */
    Result = MM_PeekMem(&CmdPacket, SrcAddress);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventID, MM_PEEK_WORD_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent.Spec);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

    UtAssert_True(MM_AppData.HkPacket.LastAction == MM_PEEK, "MM_AppData.HkPacket.LastAction == MM_PEEK");
    UtAssert_True(MM_AppData.HkPacket.MemType == MM_RAM, "MM_AppData.HkPacket.MemType == MM_RAM");
    UtAssert_True(MM_AppData.HkPacket.Address == 1, "MM_AppData.HkPacket.Address == 1");
    UtAssert_True(MM_AppData.HkPacket.BytesProcessed == 2, "MM_AppData.HkPacket.BytesProcessed == 2");
    UtAssert_True(MM_AppData.HkPacket.DataValue == 0, "MM_AppData.HkPacket.DataValue == 0");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

} /* end MM_PeekMem_Test_Word */

void MM_PeekMem_Test_WordError(void)
{
    MM_PeekCmd_t CmdPacket;
    uint32       SrcAddress = 0;
    int32        strCmpResult;
    char         ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool         Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "PSP read memory error: RC=%%d, Address=%%p, MemType=MEM%%d");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent;
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    CmdPacket.DataSize = MM_WORD_BIT_WIDTH;

    /* Set to generate error message MM_PSP_READ_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemRead16), 1, -1);

    /* Execute the function being tested */
    Result = MM_PeekMem(&CmdPacket, SrcAddress);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventID, MM_PSP_READ_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent.Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

} /* end MM_PeekMem_Test_WordError */

void MM_PeekMem_Test_DWord(void)
{
    MM_PeekCmd_t CmdPacket;
    uint32       SrcAddress = 1;
    int32        strCmpResult;
    char         ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool         Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Peek Command: Addr = %%p Size = %%d bits Data = 0x%%08X");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent;
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    CmdPacket.DataSize = MM_DWORD_BIT_WIDTH;
    CmdPacket.MemType  = MM_RAM;

    /* Execute the function being tested */
    Result = MM_PeekMem(&CmdPacket, SrcAddress);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventID, MM_PEEK_DWORD_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent.Spec);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

    UtAssert_True(MM_AppData.HkPacket.LastAction == MM_PEEK, "MM_AppData.HkPacket.LastAction == MM_PEEK");
    UtAssert_True(MM_AppData.HkPacket.MemType == MM_RAM, "MM_AppData.HkPacket.MemType == MM_RAM");
    UtAssert_True(MM_AppData.HkPacket.Address == 1, "MM_AppData.HkPacket.Address == 1");
    UtAssert_True(MM_AppData.HkPacket.BytesProcessed == 4, "MM_AppData.HkPacket.BytesProcessed == 4");
    UtAssert_True(MM_AppData.HkPacket.DataValue == 0, "MM_AppData.HkPacket.DataValue == 0");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

} /* end MM_PeekMem_Test_DWord */

void MM_PeekMem_Test_DWordError(void)
{
    MM_PeekCmd_t CmdPacket;
    uint32       SrcAddress = 0;
    int32        strCmpResult;
    char         ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool         Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "PSP read memory error: RC=%%d, Address=%%p, MemType=MEM%%d");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent;
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    CmdPacket.DataSize = MM_DWORD_BIT_WIDTH;

    /* Set to generate error message MM_PSP_READ_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemRead32), 1, -1);

    /* Execute the function being tested */
    Result = MM_PeekMem(&CmdPacket, SrcAddress);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventID, MM_PSP_READ_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent.Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_PeekMem_Test_DWordError */

void MM_PeekMem_Test_DefaultSwitch(void)
{
    MM_PeekCmd_t CmdPacket;
    uint32       SrcAddress = 1;
    bool         Result;

    CmdPacket.DataSize = 99;

    /* Execute the function being tested */
    Result = MM_PeekMem(&CmdPacket, SrcAddress);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_PeekMem_Test_DefaultSwitch */

void MM_PeekMem_Test_NoLengthVerify(void)
{
    MM_PeekCmd_t CmdPacket;
    bool         Result;

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, false);

    /* Execute the function being tested */
    Result = MM_PeekCmd((CFE_SB_Buffer_t *)(&CmdPacket));

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_PeekMem_Test_NoLengthVerify */

void MM_DumpMemToFileCmd_Test_RAM(void)
{
    MM_DumpMemToFileCmd_t CmdPacket;
    CFE_SB_MsgId_t        TestMsgId;
    CFE_MSG_FcnCode_t     FcnCode;
    int32                 strCmpResult;
    char                  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool                  Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Dump Memory To File Command: Dumped %%d bytes from address 0x%%08X to file '%%s'");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent;
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    TestMsgId = MM_CMD_MID;
    FcnCode   = MM_DUMP_MEM_TO_FILE_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    strncpy(CmdPacket.SrcSymAddress.SymName, "SymName", OS_MAX_PATH_LEN);
    CmdPacket.SrcSymAddress.Offset = (cpuaddr)&DummyBuffer[0];

    CmdPacket.MemType    = MM_RAM;
    CmdPacket.NumOfBytes = 1;

    strncpy(CmdPacket.FileName, "filename", OS_MAX_PATH_LEN);

    /* Causes call to CFS_ResolveSymAddr to return a known value for DestAddress */
    UT_SetHookFunction(UT_KEY(CFS_ResolveSymAddr), UT_MM_LOAD_TEST_CFE_SymbolLookupHook1, 0);

    /* Set to satisfy 2 instances of condition "Valid == true": after comment "Write the file headers" and comment "end
     * Valid == true if" */
    UT_SetDefaultReturnValue(UT_KEY(CFE_FS_WriteHeader), sizeof(CFE_FS_Header_t));

    /* Set to satisfy condition "Valid == true" before comment "Compute CRC of dumped data" */
    UT_SetDeferredRetcode(UT_KEY(OS_write), 1, sizeof(MM_LoadDumpFileHeader_t));

    /* Set to satisfy condition "Valid == true" before comment "Compute CRC of dumped data" */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemCpy), 1, CFE_PSP_SUCCESS);

    UT_SetDefaultReturnValue(UT_KEY(CFS_IsValidFilename), true);

    UT_SetDefaultReturnValue(UT_KEY(CFS_ResolveSymAddr), true);

    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), true);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    /* Execute the function being tested */
    Result = MM_DumpMemToFileCmd((CFE_SB_Buffer_t *)(&CmdPacket));

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventID, MM_DMP_MEM_FILE_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent.Spec);

    UtAssert_True(MM_AppData.HkPacket.LastAction == MM_DUMP_TO_FILE,
                  "MM_AppData.HkPacket.LastAction == MM_DUMP_TO_FILE");
    UtAssert_True(MM_AppData.HkPacket.Address == (cpuaddr)CmdPacket.SrcSymAddress.Offset,
                  "MM_AppData.HkPacket.Address == FileHeader.SymAddress.Offset");
    UtAssert_True(strncmp(MM_AppData.HkPacket.FileName, CmdPacket.FileName, OS_MAX_PATH_LEN) == 0,
                  "strncmp(MM_AppData.HkPacket.FileName, CmdPacket.FileName, OS_MAX_PATH_LEN) == 0");
    UtAssert_True(MM_AppData.HkPacket.MemType == CmdPacket.MemType, "MM_AppData.HkPacket.MemType == CmdPacket.MemType");
    UtAssert_True(MM_AppData.HkPacket.BytesProcessed == CmdPacket.NumOfBytes,
                  "MM_AppData.HkPacket.BytesProcessed == CmdPacket.NumOfBytes");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_DumpMemToFileCmd_Test_RAM */

void MM_DumpMemToFileCmd_Test_BadType(void)
{
    MM_DumpMemToFileCmd_t CmdPacket;
    CFE_SB_MsgId_t        TestMsgId;
    CFE_MSG_FcnCode_t     FcnCode;
    bool                  Result;

    TestMsgId = MM_CMD_MID;
    FcnCode   = MM_DUMP_MEM_TO_FILE_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    strncpy(CmdPacket.SrcSymAddress.SymName, "SymName", OS_MAX_PATH_LEN);
    CmdPacket.SrcSymAddress.Offset = (cpuaddr)&DummyBuffer[0];

    CmdPacket.MemType    = 99;
    CmdPacket.NumOfBytes = 1;

    strncpy(CmdPacket.FileName, "filename", OS_MAX_PATH_LEN);

    /* Causes call to CFS_ResolveSymAddr to return a known value for DestAddress */
    UT_SetHookFunction(UT_KEY(CFS_ResolveSymAddr), UT_MM_LOAD_TEST_CFE_SymbolLookupHook1, 0);

    /* Set to satisfy 2 instances of condition "Valid == true": after comment "Write the file headers" and comment "end
     * Valid == true if" */
    UT_SetDefaultReturnValue(UT_KEY(CFE_FS_WriteHeader), sizeof(CFE_FS_Header_t));

    /* Set to satisfy condition "Valid == true" before comment "Compute CRC of dumped data" */
    UT_SetDeferredRetcode(UT_KEY(OS_write), 1, sizeof(MM_LoadDumpFileHeader_t));

    /* Set to satisfy condition "Valid == true" before comment "Compute CRC of dumped data" */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemCpy), 1, CFE_PSP_SUCCESS);

    UT_SetDefaultReturnValue(UT_KEY(CFS_IsValidFilename), true);

    UT_SetDefaultReturnValue(UT_KEY(CFS_ResolveSymAddr), true);

    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), true);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    /* Execute the function being tested */
    Result = MM_DumpMemToFileCmd((CFE_SB_Buffer_t *)(&CmdPacket));

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_DumpMemToFileCmd_Test_BadType */

void MM_DumpMemToFileCmd_Test_EEPROM(void)
{
    MM_DumpMemToFileCmd_t CmdPacket;
    CFE_SB_MsgId_t        TestMsgId;
    CFE_MSG_FcnCode_t     FcnCode;
    int32                 strCmpResult;
    char                  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool                  Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Dump Memory To File Command: Dumped %%d bytes from address 0x%%08X to file '%%s'");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent;
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    TestMsgId = MM_CMD_MID;
    FcnCode   = MM_DUMP_MEM_TO_FILE_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    strncpy(CmdPacket.SrcSymAddress.SymName, "SymName", OS_MAX_PATH_LEN);
    CmdPacket.SrcSymAddress.Offset = (cpuaddr)&DummyBuffer[0];

    CmdPacket.MemType    = MM_EEPROM;
    CmdPacket.NumOfBytes = 1;

    strncpy(CmdPacket.FileName, "filename", OS_MAX_PATH_LEN);

    /* Causes call to CFS_ResolveSymAddr to return a known value for DestAddress */
    UT_SetHookFunction(UT_KEY(CFS_ResolveSymAddr), UT_MM_LOAD_TEST_CFE_SymbolLookupHook1, 0);

    /* Set to satisfy 2 instances of condition "Valid == true": after comment "Write the file headers" and comment "end
     * Valid == true if" */
    UT_SetDefaultReturnValue(UT_KEY(CFE_FS_WriteHeader), sizeof(CFE_FS_Header_t));

    /* Set to satisfy condition "Valid == true" before comment "Compute CRC of dumped data" */
    UT_SetDeferredRetcode(UT_KEY(OS_write), 1, sizeof(MM_LoadDumpFileHeader_t));

    /* Set to satisfy condition "Valid == true" before comment "Compute CRC of dumped data" */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemCpy), 1, CFE_PSP_SUCCESS);

    UT_SetDefaultReturnValue(UT_KEY(CFS_IsValidFilename), true);

    UT_SetDefaultReturnValue(UT_KEY(CFS_ResolveSymAddr), true);

    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), true);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    /* Execute the function being tested */
    Result = MM_DumpMemToFileCmd((CFE_SB_Buffer_t *)(&CmdPacket));

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventID, MM_DMP_MEM_FILE_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent.Spec);

    UtAssert_True(MM_AppData.HkPacket.LastAction == MM_DUMP_TO_FILE,
                  "MM_AppData.HkPacket.LastAction == MM_DUMP_TO_FILE");
    UtAssert_True(strncmp(MM_AppData.HkPacket.FileName, CmdPacket.FileName, OS_MAX_PATH_LEN) == 0,
                  "strncmp(MM_AppData.HkPacket.FileName, CmdPacket.FileName, OS_MAX_PATH_LEN) == 0");
    UtAssert_True(MM_AppData.HkPacket.MemType == CmdPacket.MemType, "MM_AppData.HkPacket.MemType == CmdPacket.MemType");
    UtAssert_True(MM_AppData.HkPacket.BytesProcessed == CmdPacket.NumOfBytes,
                  "MM_AppData.HkPacket.BytesProcessed == CmdPacket.NumOfBytes");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_DumpMemToFileCmd_Test_EEPROM */

void MM_DumpMemToFileCmd_Test_MEM32(void)
{
    MM_DumpMemToFileCmd_t CmdPacket;
    CFE_SB_MsgId_t        TestMsgId;
    CFE_MSG_FcnCode_t     FcnCode;
    int32                 strCmpResult;
    char                  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool                  Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Dump Memory To File Command: Dumped %%d bytes from address 0x%%08X to file '%%s'");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent;
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    TestMsgId = MM_CMD_MID;
    FcnCode   = MM_DUMP_MEM_TO_FILE_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    strncpy(CmdPacket.SrcSymAddress.SymName, "SymName", OS_MAX_PATH_LEN);
    CmdPacket.SrcSymAddress.Offset = 0;

    CmdPacket.MemType    = MM_MEM32;
    CmdPacket.NumOfBytes = 4;

    strncpy(CmdPacket.FileName, "filename", OS_MAX_PATH_LEN);

    /* Set to satisfy 2 instances of condition "Valid == true": after comment "Write the file headers" and comment "end
     * Valid == true if" */
    UT_SetDefaultReturnValue(UT_KEY(CFE_FS_WriteHeader), sizeof(CFE_FS_Header_t));

    /* Set to satisfy condition "Valid == true" before comment "Compute CRC of dumped data" */
    UT_SetDeferredRetcode(UT_KEY(OS_write), 1, sizeof(MM_LoadDumpFileHeader_t));

    /* Set to satisfy condition "Valid == true" before comment "Compute CRC of dumped data" */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemCpy), 1, CFE_PSP_SUCCESS);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    UT_SetDefaultReturnValue(UT_KEY(CFS_IsValidFilename), true);
    UT_SetDefaultReturnValue(UT_KEY(CFS_ResolveSymAddr), true);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), true);
    UT_SetDefaultReturnValue(UT_KEY(CFE_PSP_MemValidateRange), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_DumpMem32ToFile), true);
    UT_SetDefaultReturnValue(UT_KEY(CFS_Verify32Aligned), true);

    /* Execute the function being tested */
    Result = MM_DumpMemToFileCmd((CFE_SB_Buffer_t *)(&CmdPacket));

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventID, MM_DMP_MEM_FILE_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent.Spec);

    UtAssert_True(MM_AppData.HkPacket.LastAction == MM_DUMP_TO_FILE,
                  "MM_AppData.HkPacket.LastAction == MM_DUMP_TO_FILE");
    UtAssert_True(strncmp(MM_AppData.HkPacket.FileName, CmdPacket.FileName, OS_MAX_PATH_LEN) == 0,
                  "strncmp(MM_AppData.HkPacket.FileName, CmdPacket.FileName, OS_MAX_PATH_LEN) == 0");
    UtAssert_True(MM_AppData.HkPacket.MemType == CmdPacket.MemType, "MM_AppData.HkPacket.MemType == CmdPacket.MemType");
    UtAssert_True(MM_AppData.HkPacket.BytesProcessed == CmdPacket.NumOfBytes,
                  "MM_AppData.HkPacket.BytesProcessed == CmdPacket.NumOfBytes");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_DumpMemToFileCmd_Test_MEM32 */

void MM_DumpMemToFileCmd_Test_MEM16(void)
{
    MM_DumpMemToFileCmd_t CmdPacket;
    CFE_SB_MsgId_t        TestMsgId;
    CFE_MSG_FcnCode_t     FcnCode;
    int32                 strCmpResult;
    char                  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool                  Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Dump Memory To File Command: Dumped %%d bytes from address 0x%%08X to file '%%s'");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent;
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    TestMsgId = MM_CMD_MID;
    FcnCode   = MM_DUMP_MEM_TO_FILE_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    strncpy(CmdPacket.SrcSymAddress.SymName, "SymName", OS_MAX_PATH_LEN);
    CmdPacket.SrcSymAddress.Offset = 0;

    CmdPacket.MemType    = MM_MEM16;
    CmdPacket.NumOfBytes = 2;

    strncpy(CmdPacket.FileName, "filename", OS_MAX_PATH_LEN);

    /* Set to satisfy 2 instances of condition "Valid == true": after comment "Write the file headers" and comment "end
     * Valid == true if" */
    UT_SetDefaultReturnValue(UT_KEY(CFE_FS_WriteHeader), sizeof(CFE_FS_Header_t));

    /* Set to satisfy condition "Valid == true" before comment "Compute CRC of dumped data" */
    UT_SetDeferredRetcode(UT_KEY(OS_write), 1, sizeof(MM_LoadDumpFileHeader_t));

    /* Set to satisfy condition "Valid == true" before comment "Compute CRC of dumped data" */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemCpy), 1, CFE_PSP_SUCCESS);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    UT_SetDefaultReturnValue(UT_KEY(CFS_IsValidFilename), true);
    UT_SetDefaultReturnValue(UT_KEY(CFS_ResolveSymAddr), true);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), true);
    UT_SetDefaultReturnValue(UT_KEY(CFE_PSP_MemValidateRange), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_DumpMem16ToFile), true);
    UT_SetDefaultReturnValue(UT_KEY(CFS_Verify16Aligned), true);

    /* Execute the function being tested */
    Result = MM_DumpMemToFileCmd((CFE_SB_Buffer_t *)(&CmdPacket));

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventID, MM_DMP_MEM_FILE_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent.Spec);

    UtAssert_True(MM_AppData.HkPacket.LastAction == MM_DUMP_TO_FILE,
                  "MM_AppData.HkPacket.LastAction == MM_DUMP_TO_FILE");
    UtAssert_True(strncmp(MM_AppData.HkPacket.FileName, CmdPacket.FileName, OS_MAX_PATH_LEN) == 0,
                  "strncmp(MM_AppData.HkPacket.FileName, CmdPacket.FileName, OS_MAX_PATH_LEN) == 0");
    UtAssert_True(MM_AppData.HkPacket.MemType == CmdPacket.MemType, "MM_AppData.HkPacket.MemType == CmdPacket.MemType");
    UtAssert_True(MM_AppData.HkPacket.BytesProcessed == CmdPacket.NumOfBytes,
                  "MM_AppData.HkPacket.BytesProcessed == CmdPacket.NumOfBytes");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_DumpMemToFileCmd_Test_MEM16 */

void MM_DumpMemToFileCmd_Test_MEM8(void)
{
    MM_DumpMemToFileCmd_t CmdPacket;
    CFE_SB_MsgId_t        TestMsgId;
    CFE_MSG_FcnCode_t     FcnCode;
    int32                 strCmpResult;
    char                  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool                  Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Dump Memory To File Command: Dumped %%d bytes from address 0x%%08X to file '%%s'");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent;
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    TestMsgId = MM_CMD_MID;
    FcnCode   = MM_DUMP_MEM_TO_FILE_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    strncpy(CmdPacket.SrcSymAddress.SymName, "SymName", OS_MAX_PATH_LEN);
    CmdPacket.SrcSymAddress.Offset = 0;

    CmdPacket.MemType    = MM_MEM8;
    CmdPacket.NumOfBytes = 1;

    strncpy(CmdPacket.FileName, "filename", OS_MAX_PATH_LEN);

    /* Set to satisfy 2 instances of condition "Valid == true": after comment "Write the file headers" and comment "end
     * Valid == true if" */
    UT_SetDefaultReturnValue(UT_KEY(CFE_FS_WriteHeader), sizeof(CFE_FS_Header_t));

    /* Set to satisfy condition "Valid == true" before comment "Compute CRC of dumped data" */
    UT_SetDeferredRetcode(UT_KEY(OS_write), 1, sizeof(MM_LoadDumpFileHeader_t));

    /* Set to satisfy condition "Valid == true" before comment "Compute CRC of dumped data" */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemCpy), 1, CFE_PSP_SUCCESS);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    UT_SetDefaultReturnValue(UT_KEY(CFS_IsValidFilename), true);
    UT_SetDefaultReturnValue(UT_KEY(CFS_ResolveSymAddr), true);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), true);
    UT_SetDefaultReturnValue(UT_KEY(CFE_PSP_MemValidateRange), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_DumpMem8ToFile), true);

    /* Execute the function being tested */
    Result = MM_DumpMemToFileCmd((CFE_SB_Buffer_t *)(&CmdPacket));

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventID, MM_DMP_MEM_FILE_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent.Spec);

    UtAssert_True(MM_AppData.HkPacket.LastAction == MM_DUMP_TO_FILE,
                  "MM_AppData.HkPacket.LastAction == MM_DUMP_TO_FILE");
    UtAssert_True(strncmp(MM_AppData.HkPacket.FileName, CmdPacket.FileName, OS_MAX_PATH_LEN) == 0,
                  "strncmp(MM_AppData.HkPacket.FileName, CmdPacket.FileName, OS_MAX_PATH_LEN) == 0");
    UtAssert_True(MM_AppData.HkPacket.MemType == CmdPacket.MemType, "MM_AppData.HkPacket.MemType == CmdPacket.MemType");
    UtAssert_True(MM_AppData.HkPacket.BytesProcessed == CmdPacket.NumOfBytes,
                  "MM_AppData.HkPacket.BytesProcessed == CmdPacket.NumOfBytes");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_DumpMemToFileCmd_Test_MEM8 */

void MM_DumpMemToFileCmd_Test_ComputeCRCError(void)
{
    MM_DumpMemToFileCmd_t CmdPacket;
    CFE_SB_MsgId_t        TestMsgId;
    CFE_MSG_FcnCode_t     FcnCode;
    int32                 strCmpResult;
    char                  ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool                  Result;

    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "CFS_ComputeCRCFromFile error received: RC = 0x%%08X File = '%%s'");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent[2];
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    TestMsgId = MM_CMD_MID;
    FcnCode   = MM_DUMP_MEM_TO_FILE_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    strncpy(CmdPacket.SrcSymAddress.SymName, "SymName", OS_MAX_PATH_LEN);
    CmdPacket.SrcSymAddress.Offset = 0;

    CmdPacket.MemType    = MM_MEM8;
    CmdPacket.NumOfBytes = 1;

    strncpy(CmdPacket.FileName, "filename", OS_MAX_PATH_LEN);

    /* Set to satisfy 2 instances of condition "Valid == true": after comment "Write the file headers" and comment "end
     * Valid == true if" */
    UT_SetDefaultReturnValue(UT_KEY(CFE_FS_WriteHeader), sizeof(CFE_FS_Header_t));

    /* Set to satisfy condition "Valid == true" before comment "Compute CRC of dumped data" */
    UT_SetDeferredRetcode(UT_KEY(OS_write), 1, sizeof(MM_LoadDumpFileHeader_t));

    /* Set to satisfy condition "Valid == true" before comment "Compute CRC of dumped data" */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemCpy), 1, CFE_PSP_SUCCESS);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    UT_SetDefaultReturnValue(UT_KEY(CFS_IsValidFilename), true);
    UT_SetDefaultReturnValue(UT_KEY(CFS_ResolveSymAddr), true);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), true);
    UT_SetDefaultReturnValue(UT_KEY(CFE_PSP_MemValidateRange), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_DumpMem8ToFile), true);

    /* Set to generate error message MM_CFS_COMPUTECRCFROMFILE_ERR_EID */
    UT_SetDefaultReturnValue(UT_KEY(CFS_ComputeCRCFromFile), -1);

    /* Execute the function being tested */
    Result = MM_DumpMemToFileCmd((CFE_SB_Buffer_t *)(&CmdPacket));

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_CFS_COMPUTECRCFROMFILE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_DumpMemToFileCmd_Test_ComputeCRCError */

void MM_DumpMemToFileCmd_Test_CloseError(void)
{
    MM_DumpMemToFileCmd_t CmdPacket;
    CFE_SB_MsgId_t        TestMsgId;
    CFE_MSG_FcnCode_t     FcnCode;
    int32                 strCmpResult;
    char                  ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool                  Result;

    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Dump Memory To File Command: Dumped %%d bytes from address 0x%%08X to file '%%s'");

    snprintf(ExpectedEventString[1], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "OS_close error received: RC = 0x%%08X File = '%%s'");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent[2];
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    TestMsgId = MM_CMD_MID;
    FcnCode   = MM_DUMP_MEM_TO_FILE_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    strncpy(CmdPacket.SrcSymAddress.SymName, "SymName", OS_MAX_PATH_LEN);
    CmdPacket.SrcSymAddress.Offset = 0;

    CmdPacket.MemType    = MM_MEM8;
    CmdPacket.NumOfBytes = 1;

    strncpy(CmdPacket.FileName, "filename", OS_MAX_PATH_LEN);

    /* Set to satisfy 2 instances of condition "Valid == true": after comment "Write the file headers" and comment "end
     * Valid == true if" */
    UT_SetDefaultReturnValue(UT_KEY(CFE_FS_WriteHeader), sizeof(CFE_FS_Header_t));

    /* Set to satisfy condition "Valid == true" before comment "Compute CRC of dumped data" */
    UT_SetDeferredRetcode(UT_KEY(OS_write), 1, sizeof(MM_LoadDumpFileHeader_t));

    /* Set to satisfy condition "Valid == true" before comment "Compute CRC of dumped data" */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemCpy), 1, CFE_PSP_SUCCESS);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    UT_SetDefaultReturnValue(UT_KEY(CFS_IsValidFilename), true);
    UT_SetDefaultReturnValue(UT_KEY(CFS_ResolveSymAddr), true);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), true);
    UT_SetDefaultReturnValue(UT_KEY(CFE_PSP_MemValidateRange), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_DumpMem8ToFile), true);

    /* Set to generate error message MM_OS_CLOSE_ERR_EID */
    UT_SetDefaultReturnValue(UT_KEY(OS_close), -1);

    /* Execute the function being tested */
    Result = MM_DumpMemToFileCmd((CFE_SB_Buffer_t *)(&CmdPacket));

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

    UtAssert_True(MM_AppData.HkPacket.LastAction == MM_DUMP_TO_FILE,
                  "MM_AppData.HkPacket.LastAction == MM_DUMP_TO_FILE");
    UtAssert_True(strncmp(MM_AppData.HkPacket.FileName, CmdPacket.FileName, OS_MAX_PATH_LEN) == 0,
                  "strncmp(MM_AppData.HkPacket.FileName, CmdPacket.FileName, OS_MAX_PATH_LEN) == 0");
    UtAssert_True(MM_AppData.HkPacket.MemType == CmdPacket.MemType, "MM_AppData.HkPacket.MemType == CmdPacket.MemType");
    UtAssert_True(MM_AppData.HkPacket.BytesProcessed == CmdPacket.NumOfBytes,
                  "MM_AppData.HkPacket.BytesProcessed == CmdPacket.NumOfBytes");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 2, "CFE_EVS_SendEvent was called %u time(s), expected 2",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_DumpMemToFileCmd_Test_CloseError */

void MM_DumpMemToFileCmd_Test_CreatError(void)
{
    MM_DumpMemToFileCmd_t CmdPacket;
    CFE_SB_MsgId_t        TestMsgId;
    CFE_MSG_FcnCode_t     FcnCode;
    int32                 strCmpResult;
    char                  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool                  Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "OS_OpenCreate error received: RC = %%d File = '%%s'");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent;
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    TestMsgId = MM_CMD_MID;
    FcnCode   = MM_DUMP_MEM_TO_FILE_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    strncpy(CmdPacket.SrcSymAddress.SymName, "SymName", OS_MAX_PATH_LEN);
    CmdPacket.SrcSymAddress.Offset = 0;

    CmdPacket.MemType    = MM_MEM8;
    CmdPacket.NumOfBytes = 1;

    strncpy(CmdPacket.FileName, "filename", OS_MAX_PATH_LEN);

    /* Set to satisfy 2 instances of condition "Valid == true": after comment "Write the file headers" and comment "end
     * Valid == true if" */
    UT_SetDefaultReturnValue(UT_KEY(CFE_FS_WriteHeader), sizeof(CFE_FS_Header_t));

    /* Set to satisfy condition "Valid == true" before comment "Compute CRC of dumped data" */
    UT_SetDeferredRetcode(UT_KEY(OS_write), 1, sizeof(MM_LoadDumpFileHeader_t));

    /* Set to satisfy condition "Valid == true" before comment "Compute CRC of dumped data" */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemCpy), 1, CFE_PSP_SUCCESS);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    UT_SetDefaultReturnValue(UT_KEY(CFS_IsValidFilename), true);
    UT_SetDefaultReturnValue(UT_KEY(CFS_ResolveSymAddr), true);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), true);
    UT_SetDefaultReturnValue(UT_KEY(CFE_PSP_MemValidateRange), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_DumpMem8ToFile), true);

    /* Set to generate error message MM_OS_CREAT_ERR_EID */
    UT_SetDefaultReturnValue(UT_KEY(OS_OpenCreate), -1);

    /* Execute the function being tested */
    Result = MM_DumpMemToFileCmd((CFE_SB_Buffer_t *)(&CmdPacket));

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventID, MM_OS_CREAT_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent.Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_DumpMemToFileCmd_Test_CreatError */

void MM_DumpMemToFileCmd_Test_InvalidDumpResult(void)
{
    MM_DumpMemToFileCmd_t CmdPacket;
    CFE_SB_MsgId_t        TestMsgId;
    CFE_MSG_FcnCode_t     FcnCode;
    bool                  Result;

    TestMsgId = MM_CMD_MID;
    FcnCode   = MM_DUMP_MEM_TO_FILE_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    strncpy(CmdPacket.SrcSymAddress.SymName, "SymName", OS_MAX_PATH_LEN);
    CmdPacket.SrcSymAddress.Offset = 0;

    CmdPacket.MemType    = MM_MEM8;
    CmdPacket.NumOfBytes = 1;

    strncpy(CmdPacket.FileName, "filename", OS_MAX_PATH_LEN);

    /* Set to satisfy 2 instances of condition "Valid == true": after comment "Write the file headers" and comment "end
     * Valid == true if" */
    UT_SetDefaultReturnValue(UT_KEY(CFE_FS_WriteHeader), sizeof(CFE_FS_Header_t));

    /* Set to satisfy condition "Valid == false" before comment "Compute CRC of dumped data" */
    UT_SetDeferredRetcode(UT_KEY(MM_DumpMem8ToFile), 1, false);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    UT_SetDefaultReturnValue(UT_KEY(CFS_IsValidFilename), true);
    UT_SetDefaultReturnValue(UT_KEY(CFS_ResolveSymAddr), true);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), true);
    UT_SetDefaultReturnValue(UT_KEY(CFE_PSP_MemValidateRange), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_DumpMem8ToFile), true);

    /* Execute the function being tested */
    Result = MM_DumpMemToFileCmd((CFE_SB_Buffer_t *)(&CmdPacket));

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_DumpMemToFileCmd_Test_CreatError */

void MM_DumpMemToFileCmd_Test_lseekError(void)
{
    MM_DumpMemToFileCmd_t CmdPacket;
    CFE_SB_MsgId_t        TestMsgId;
    CFE_MSG_FcnCode_t     FcnCode;
    bool                  Result;

    TestMsgId = MM_CMD_MID;
    FcnCode   = MM_DUMP_MEM_TO_FILE_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    strncpy(CmdPacket.SrcSymAddress.SymName, "SymName", OS_MAX_PATH_LEN);
    CmdPacket.SrcSymAddress.Offset = 0;

    CmdPacket.MemType    = MM_MEM8;
    CmdPacket.NumOfBytes = 1;

    strncpy(CmdPacket.FileName, "filename", OS_MAX_PATH_LEN);

    /* Set to satisfy 2 instances of condition "Valid == true": after comment "Write the file headers" and comment "end
     * Valid == true if" */
    UT_SetDefaultReturnValue(UT_KEY(CFE_FS_WriteHeader), sizeof(CFE_FS_Header_t));

    /* Set to satisfy condition "Valid == false" before comment "Compute CRC of dumped data" */
    UT_SetDeferredRetcode(UT_KEY(OS_lseek), 1, -1);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    UT_SetDefaultReturnValue(UT_KEY(CFS_IsValidFilename), true);
    UT_SetDefaultReturnValue(UT_KEY(CFS_ResolveSymAddr), true);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), true);
    UT_SetDefaultReturnValue(UT_KEY(CFE_PSP_MemValidateRange), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_DumpMem8ToFile), true);

    /* Execute the function being tested */
    Result = MM_DumpMemToFileCmd((CFE_SB_Buffer_t *)(&CmdPacket));

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_DumpMemToFileCmd_Test_CreatError */

void MM_DumpMemToFileCmd_Test_SymNameError(void)
{
    MM_DumpMemToFileCmd_t CmdPacket;
    CFE_SB_MsgId_t        TestMsgId;
    CFE_MSG_FcnCode_t     FcnCode;
    int32                 strCmpResult;
    char                  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool                  Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Symbolic address can't be resolved: Name = '%%s'");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent;
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    TestMsgId = MM_CMD_MID;
    FcnCode   = MM_DUMP_MEM_TO_FILE_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    strncpy(CmdPacket.SrcSymAddress.SymName, "SymName", OS_MAX_PATH_LEN);
    CmdPacket.SrcSymAddress.Offset = 0;

    CmdPacket.MemType    = MM_MEM8;
    CmdPacket.NumOfBytes = 1;

    strncpy(CmdPacket.FileName, "filename", OS_MAX_PATH_LEN);

    /* Set to satisfy 2 instances of condition "Valid == true": after comment "Write the file headers" and comment "end
     * Valid == true if" */
    UT_SetDefaultReturnValue(UT_KEY(CFE_FS_WriteHeader), sizeof(CFE_FS_Header_t));

    /* Set to satisfy condition "Valid == true" before comment "Compute CRC of dumped data" */
    UT_SetDeferredRetcode(UT_KEY(OS_write), 1, sizeof(MM_LoadDumpFileHeader_t));

    /* Set to satisfy condition "Valid == true" before comment "Compute CRC of dumped data" */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemCpy), 1, CFE_PSP_SUCCESS);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    UT_SetDefaultReturnValue(UT_KEY(CFS_IsValidFilename), true);

    /* Set to generate error message MM_SYMNAME_ERR_EID */
    UT_SetDefaultReturnValue(UT_KEY(CFS_ResolveSymAddr), false);

    /* Execute the function being tested */
    Result = MM_DumpMemToFileCmd((CFE_SB_Buffer_t *)(&CmdPacket));

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventID, MM_SYMNAME_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent.Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_DumpMemToFileCmd_Test_SymNameError */

void MM_DumpMemToFileCmd_Test_FilenameError(void)
{
    MM_DumpMemToFileCmd_t CmdPacket;
    CFE_SB_MsgId_t        TestMsgId;
    CFE_MSG_FcnCode_t     FcnCode;
    int32                 strCmpResult;
    char                  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool                  Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Command specified filename invalid: Name = '%%s'");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent;
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    TestMsgId = MM_CMD_MID;
    FcnCode   = MM_DUMP_MEM_TO_FILE_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    strncpy(CmdPacket.SrcSymAddress.SymName, "SymName", OS_MAX_PATH_LEN);
    CmdPacket.SrcSymAddress.Offset = 0;

    CmdPacket.MemType    = MM_MEM8;
    CmdPacket.NumOfBytes = 1;

    strncpy(CmdPacket.FileName, "filename", OS_MAX_PATH_LEN);

    /* Set to satisfy 2 instances of condition "Valid == true": after comment "Write the file headers" and comment "end
     * Valid == true if" */
    UT_SetDefaultReturnValue(UT_KEY(CFE_FS_WriteHeader), sizeof(CFE_FS_Header_t));

    /* Set to satisfy condition "Valid == true" before comment "Compute CRC of dumped data" */
    UT_SetDeferredRetcode(UT_KEY(OS_write), 1, sizeof(MM_LoadDumpFileHeader_t));

    /* Set to satisfy condition "Valid == true" before comment "Compute CRC of dumped data" */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemCpy), 1, CFE_PSP_SUCCESS);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    /* Set to generate error message MM_CMD_FNAME_ERR_EID */
    UT_SetDefaultReturnValue(UT_KEY(CFS_IsValidFilename), false);

    /* Execute the function being tested */
    Result = MM_DumpMemToFileCmd((CFE_SB_Buffer_t *)(&CmdPacket));

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventID, MM_CMD_FNAME_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent.Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_DumpMemToFileCmd_Test_FilenameError */

void MM_DumpMemToFileCmd_Test_NoVerifyDumpParams(void)
{
    MM_DumpMemToFileCmd_t CmdPacket;
    CFE_SB_MsgId_t        TestMsgId;
    CFE_MSG_FcnCode_t     FcnCode;
    bool                  Result;

    TestMsgId = MM_CMD_MID;
    FcnCode   = MM_DUMP_MEM_TO_FILE_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    strncpy(CmdPacket.SrcSymAddress.SymName, "SymName", OS_MAX_PATH_LEN);
    CmdPacket.SrcSymAddress.Offset = 0;

    CmdPacket.MemType = MM_RAM;
    /* Set to fail MM_VerifyFileDumpParams */

    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), false);
    CmdPacket.NumOfBytes = 0;

    strncpy(CmdPacket.FileName, "filename", OS_MAX_PATH_LEN);

    /* Set to satisfy 2 instances of condition "Valid == true": after comment "Write the file headers" and comment "end
     * Valid == true if" */
    UT_SetDefaultReturnValue(UT_KEY(CFE_FS_WriteHeader), sizeof(CFE_FS_Header_t));

    /* Set to satisfy condition "Valid == true" before comment "Compute CRC of dumped data" */
    UT_SetDeferredRetcode(UT_KEY(OS_write), 1, sizeof(MM_LoadDumpFileHeader_t));

    /* Set to satisfy condition "Valid == true" before comment "Compute CRC of dumped data" */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemCpy), 1, CFE_PSP_SUCCESS);

    UT_SetDefaultReturnValue(UT_KEY(CFS_IsValidFilename), true);

    UT_SetDefaultReturnValue(UT_KEY(CFS_ResolveSymAddr), true);

    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), false);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    /* Execute the function being tested */
    Result = MM_DumpMemToFileCmd((CFE_SB_Buffer_t *)(&CmdPacket));

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_True(MM_AppData.HkPacket.BytesProcessed == CmdPacket.NumOfBytes,
                  "MM_AppData.HkPacket.BytesProcessed == CmdPacket.NumOfBytes");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_DumpMemToFileCmd_Test_NoVerifyDumpParams */

void MM_DumpMemToFileCmd_Test_NoLengthVerify(void)
{
    MM_DumpMemToFileCmd_t CmdPacket;
    bool                  Result;

    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), false);

    /* Execute the function being tested */
    Result = MM_DumpMemToFileCmd((CFE_SB_Buffer_t *)(&CmdPacket));

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_DumpMemToFileCmd_Test_NoLengthVerify */

void MM_DumpMemToFileCmd_Test_NoWriteHeaders(void)
{
    MM_DumpMemToFileCmd_t CmdPacket;
    CFE_SB_MsgId_t        TestMsgId;
    CFE_MSG_FcnCode_t     FcnCode;
    int32                 strCmpResult;
    char                  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool                  Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "CFE_FS_WriteHeader error received: RC = %%d Expected = %%d File = '%%s'");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent;
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    TestMsgId = MM_CMD_MID;
    FcnCode   = MM_DUMP_MEM_TO_FILE_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    strncpy(CmdPacket.SrcSymAddress.SymName, "SymName", OS_MAX_PATH_LEN);
    CmdPacket.SrcSymAddress.Offset = 0;

    CmdPacket.MemType    = MM_RAM;
    CmdPacket.NumOfBytes = 1;

    strncpy(CmdPacket.FileName, "filename", OS_MAX_PATH_LEN);

    /* Set to satisfy 2 instances of condition "Valid == true": after comment "Write the file headers" and comment "end
     * Valid == true if" */
    UT_SetDefaultReturnValue(UT_KEY(CFE_FS_WriteHeader), sizeof(CFE_FS_Header_t));

    /* Set to satisfy condition "Valid == true" before comment "Compute CRC of dumped data" */
    UT_SetDeferredRetcode(UT_KEY(OS_write), 1, sizeof(MM_LoadDumpFileHeader_t));

    /* Set to satisfy condition "Valid == true" before comment "Compute CRC of dumped data" */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemCpy), 1, CFE_PSP_SUCCESS);

    UT_SetDefaultReturnValue(UT_KEY(CFS_IsValidFilename), true);

    UT_SetDefaultReturnValue(UT_KEY(CFS_ResolveSymAddr), true);

    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), true);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    UT_SetDefaultReturnValue(UT_KEY(CFE_FS_WriteHeader), -1);

    /* Execute the function being tested */
    Result = MM_DumpMemToFileCmd((CFE_SB_Buffer_t *)(&CmdPacket));

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventID, MM_CFE_FS_WRITEHDR_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent.Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_DumpMemToFileCmd_Test_NoWriteHeaders */

void MM_DumpMemToFile_Test_Nominal(void)
{
    uint32                  FileHandle = 1;
    char                    FileName[OS_MAX_PATH_LEN];
    MM_LoadDumpFileHeader_t FileHeader;
    bool                    Result;

    strncpy(FileName, "filename", OS_MAX_PATH_LEN);

    FileHeader.NumOfBytes = 1;
    /* a valid source address is required input to memcpy */
    FileHeader.SymAddress.Offset = (cpuaddr)&MM_AppData.LoadBuffer[0];
    FileHeader.MemType           = MM_RAM;

    /* Execute the function being tested */
    Result = MM_DumpMemToFile(FileHandle, FileName, &FileHeader);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");
    UtAssert_True(MM_AppData.HkPacket.LastAction == MM_DUMP_TO_FILE,
                  "MM_AppData.HkPacket.LastAction == MM_DUMP_TO_FILE");
    UtAssert_True(MM_AppData.HkPacket.MemType == FileHeader.MemType,
                  "MM_AppData.HkPacket.MemType == FileHeader.MemType");
    UtAssert_True(MM_AppData.HkPacket.Address == (cpuaddr)FileHeader.SymAddress.Offset,
                  "MM_AppData.HkPacket.Address == FileHeader.SymAddress.Offset");
    UtAssert_True(MM_AppData.HkPacket.BytesProcessed == 1, "MM_AppData.HkPacket.BytesProcessed == 1");
    UtAssert_True(strncmp(MM_AppData.HkPacket.FileName, FileName, OS_MAX_PATH_LEN) == 0,
                  "strncmp(MM_AppData.HkPacket.FileName, FileName, OS_MAX_PATH_LEN) == 0");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_DumpMemToFile_Test_Nominal */

void MM_DumpMemToFile_Test_CPUHogging(void)
{
    uint32                  FileHandle = 1;
    char                    FileName[OS_MAX_PATH_LEN];
    MM_LoadDumpFileHeader_t FileHeader;
    bool                    Result;

    strncpy(FileName, "filename", OS_MAX_PATH_LEN);

    FileHeader.NumOfBytes = 2 * MM_MAX_DUMP_DATA_SEG;
    /* a valid source address is required input to memcpy */
    FileHeader.SymAddress.Offset = (cpuaddr)&MM_AppData.LoadBuffer[0];
    FileHeader.MemType           = MM_RAM;

    /* Execute the function being tested */
    Result = MM_DumpMemToFile(FileHandle, FileName, &FileHeader);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");
    UtAssert_True(MM_AppData.HkPacket.LastAction == MM_DUMP_TO_FILE,
                  "MM_AppData.HkPacket.LastAction == MM_DUMP_TO_FILE");
    UtAssert_True(MM_AppData.HkPacket.MemType == FileHeader.MemType,
                  "MM_AppData.HkPacket.MemType == FileHeader.MemType");
    UtAssert_True(MM_AppData.HkPacket.Address == (cpuaddr)FileHeader.SymAddress.Offset,
                  "MM_AppData.HkPacket.Address == FileHeader.SymAddress.Offset");
    UtAssert_True(MM_AppData.HkPacket.BytesProcessed == 2 * MM_MAX_DUMP_DATA_SEG,
                  "MM_AppData.HkPacket.BytesProcessed == 2 * MM_MAX_DUMP_DATA_SEG");
    UtAssert_True(strncmp(MM_AppData.HkPacket.FileName, FileName, OS_MAX_PATH_LEN) == 0,
                  "strncmp(MM_AppData.HkPacket.FileName, FileName, OS_MAX_PATH_LEN) == 0");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_DumpMemToFile_Test_CPUHogging */

void MM_DumpMemToFile_Test_WriteError(void)
{
    uint32                  FileHandle = 1;
    char                    FileName[OS_MAX_PATH_LEN];
    MM_LoadDumpFileHeader_t FileHeader;
    int32                   strCmpResult;
    char                    ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool                    Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "OS_write error received: RC = %%d, Expected = %%d, File = '%%s'");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent;
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    strncpy(FileName, "filename", OS_MAX_PATH_LEN);

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

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventID, MM_OS_WRITE_EXP_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent.Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_DumpMemToFile_Test_WriteError */

void MM_WriteFileHeaders_Test_Nominal(void)
{
    char                    FileName[OS_MAX_PATH_LEN];
    int32                   FileHandle = 1;
    CFE_FS_Header_t         CFEHeader;
    MM_LoadDumpFileHeader_t MMHeader;
    bool                    Result;

    strncpy(FileName, "filename", OS_MAX_PATH_LEN);

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
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_WriteFileHeaders_Test_Nominal */

void MM_WriteFileHeaders_Test_WriteHeaderError(void)
{
    char                    FileName[OS_MAX_PATH_LEN];
    int32                   FileHandle = 1;
    CFE_FS_Header_t         CFEHeader;
    MM_LoadDumpFileHeader_t MMHeader;
    int32                   strCmpResult;
    char                    ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool                    Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "CFE_FS_WriteHeader error received: RC = %%d Expected = %%d File = '%%s'");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent;
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    strncpy(FileName, "filename", OS_MAX_PATH_LEN);

    MMHeader.NumOfBytes        = 1;
    MMHeader.SymAddress.Offset = 0;
    MMHeader.MemType           = MM_RAM;

    /* Set to generate error message MM_CFE_FS_WRITEHDR_ERR_EID */
    UT_SetDefaultReturnValue(UT_KEY(CFE_FS_WriteHeader), -1);

    /* Execute the function being tested */
    Result = MM_WriteFileHeaders(FileName, FileHandle, &CFEHeader, &MMHeader);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventID, MM_CFE_FS_WRITEHDR_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent.Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_WriteFileHeaders_Test_WriteHeaderError */

void MM_WriteFileHeaders_Test_WriteError(void)
{
    char                    FileName[OS_MAX_PATH_LEN];
    int32                   FileHandle = 1;
    CFE_FS_Header_t         CFEHeader;
    MM_LoadDumpFileHeader_t MMHeader;
    int32                   strCmpResult;
    char                    ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool                    Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "OS_write error received: RC = %%d Expected = %%d File = '%%s'");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent;
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    strncpy(FileName, "filename", OS_MAX_PATH_LEN);

    MMHeader.NumOfBytes        = 1;
    MMHeader.SymAddress.Offset = 0;
    MMHeader.MemType           = MM_RAM;

    /* Set to generate error message MM_CFE_FS_WRITEHDR_ERR_EID */
    UT_SetDefaultReturnValue(UT_KEY(OS_write), -1);

    /* Execute the function being tested */
    Result = MM_WriteFileHeaders(FileName, FileHandle, &CFEHeader, &MMHeader);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventID, MM_OS_WRITE_EXP_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent.Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_WriteFileHeaders_Test_WriteError */

void MM_DumpInEventCmd_Test_Nominal(void)
{
    MM_DumpInEventCmd_t MessagePtr;
    CFE_SB_MsgId_t      TestMsgId;
    CFE_MSG_FcnCode_t   FcnCode;
    int32               strCmpResult;
    char                ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool                Result;

    /* The event string is dynamically generated with string contatentation
       and is reported in context_CFE_EVS_SendEvent as a single string.
       The event string cannot be matched to a preformatted value. */
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "%%s");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent;
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    TestMsgId = MM_CMD_MID;
    FcnCode   = MM_DUMP_IN_EVENT_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    MessagePtr.MemType              = MM_RAM;
    MessagePtr.NumOfBytes           = 1;
    MessagePtr.SrcSymAddress.Offset = (cpuaddr)&DummyBuffer[0];

    strncpy(MessagePtr.SrcSymAddress.SymName, "", OS_MAX_PATH_LEN);

    /* Causes call to CFS_ResolveSymAddr to return a known value for DestAddress */
    UT_SetHookFunction(UT_KEY(CFS_ResolveSymAddr), UT_MM_LOAD_TEST_CFE_SymbolLookupHook1, 0);

    /* Set to satisfy 2 instances of condition "Valid == true": after comment "Write the file headers" and comment "end
     * Valid == true if" */
    UT_SetDefaultReturnValue(UT_KEY(CFE_FS_WriteHeader), sizeof(CFE_FS_Header_t));

    /* Set to satisfy condition "Valid == true" before comment "Compute CRC of dumped data" */
    UT_SetDeferredRetcode(UT_KEY(OS_write), 1, sizeof(MM_LoadDumpFileHeader_t));

    /* Set to satisfy condition "Valid == true" before comment "Compute CRC of dumped data" */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemCpy), 1, CFE_PSP_SUCCESS);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);
    UT_SetDefaultReturnValue(UT_KEY(CFS_IsValidFilename), true);
    UT_SetDefaultReturnValue(UT_KEY(CFS_ResolveSymAddr), true);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), true);

    /* Execute the function being tested */
    Result = MM_DumpInEventCmd((CFE_SB_Buffer_t *)(&MessagePtr));

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    UtAssert_True(MM_AppData.HkPacket.LastAction == MM_DUMP_INEVENT,
                  "MM_AppData.HkPacket.LastAction == MM_DUMP_INEVENT");
    UtAssert_True(MM_AppData.HkPacket.MemType == MM_RAM, "MM_AppData.HkPacket.MemType == MM_RAM");
    UtAssert_True(MM_AppData.HkPacket.Address == (cpuaddr)MessagePtr.SrcSymAddress.Offset,
                  "MM_AppData.HkPacketAddress == FileHeader.SymAddress.Offset");
    UtAssert_True(MM_AppData.HkPacket.BytesProcessed == 1, "MM_AppData.HkPacket.BytesProcessed == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventID, MM_DUMP_INEVENT_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s' '%s'", context_CFE_EVS_SendEvent.Spec,
                  ExpectedEventString);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_DumpInEventCmd_Test_Nominal */

void MM_DumpInEventCmd_Test_SymNameError(void)
{
    MM_DumpInEventCmd_t MessagePtr;
    CFE_SB_MsgId_t      TestMsgId;
    CFE_MSG_FcnCode_t   FcnCode;
    int32               strCmpResult;
    char                ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool                Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Symbolic address can't be resolved: Name = '%%s'");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent;
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    TestMsgId = MM_CMD_MID;
    FcnCode   = MM_DUMP_IN_EVENT_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    MessagePtr.MemType              = MM_RAM;
    MessagePtr.NumOfBytes           = 1;
    MessagePtr.SrcSymAddress.Offset = 0;

    strncpy(MessagePtr.SrcSymAddress.SymName, "name", OS_MAX_PATH_LEN);

    /* Set to generate error message MM_SYMNAME_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(OS_SymbolLookup), 1, -1);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), true);

    /* Execute the function being tested */
    Result = MM_DumpInEventCmd((CFE_SB_Buffer_t *)(&MessagePtr));

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventID, MM_SYMNAME_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent.Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_DumpInEventCmd_Test_SymNameError */

void MM_DumpInEventCmd_Test_NoLengthVerify(void)
{
    MM_DumpInEventCmd_t MessagePtr;
    bool                Result;

    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), false);

    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), true);

    /* Execute the function being tested */
    Result = MM_DumpInEventCmd((CFE_SB_Buffer_t *)(&MessagePtr));

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_DumpInEventCmd_Test_NoVerifyDumpParams(void)
{
    MM_DumpInEventCmd_t MessagePtr;
    CFE_SB_MsgId_t      TestMsgId;
    CFE_MSG_FcnCode_t   FcnCode;
    bool                Result;

    TestMsgId = MM_CMD_MID;
    FcnCode   = MM_DUMP_IN_EVENT_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    MessagePtr.MemType              = MM_RAM;
    MessagePtr.NumOfBytes           = 0;
    MessagePtr.SrcSymAddress.Offset = 0;

    strncpy(MessagePtr.SrcSymAddress.SymName, "name", OS_MAX_PATH_LEN);

    UT_SetDeferredRetcode(UT_KEY(CFS_ResolveSymAddr), 1, true);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);

    /* Execute the function being tested */
    Result = MM_DumpInEventCmd((CFE_SB_Buffer_t *)(&MessagePtr));

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_DumpInEventCmd_Test_NoVerifyDumpParams */

void MM_DumpInEventCmd_Test_FillDumpInvalid(void)
{
    MM_DumpInEventCmd_t MessagePtr;
    CFE_SB_MsgId_t      TestMsgId;
    CFE_MSG_FcnCode_t   FcnCode;
    bool                Result;

    TestMsgId = MM_CMD_MID;
    FcnCode   = MM_DUMP_IN_EVENT_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    MessagePtr.MemType              = MM_MEM8;
    MessagePtr.NumOfBytes           = 1;
    MessagePtr.SrcSymAddress.Offset = (cpuaddr)&DummyBuffer[0];

    strncpy(MessagePtr.SrcSymAddress.SymName, "", OS_MAX_PATH_LEN);

    /* Causes call to CFS_ResolveSymAddr to return a known value for DestAddress */
    UT_SetHookFunction(UT_KEY(CFS_ResolveSymAddr), UT_MM_LOAD_TEST_CFE_SymbolLookupHook1, 0);

    /* Set to satisfy 2 instances of condition "Valid == true": after comment "Write the file headers" and comment "end
     * Valid == true if" */
    UT_SetDefaultReturnValue(UT_KEY(CFE_FS_WriteHeader), sizeof(CFE_FS_Header_t));

    /* Set to satisfy condition "Valid == true" before comment "Compute CRC of dumped data" */
    UT_SetDeferredRetcode(UT_KEY(OS_write), 1, sizeof(MM_LoadDumpFileHeader_t));

    /* Set to generate error message MM_PSP_READ_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemRead8), 1, -1);

    /* Set to satisfy condition "Valid == false" after comment "Fill a local data buffer with the dump words" */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemCpy), 1, -1);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyCmdLength), true);
    UT_SetDefaultReturnValue(UT_KEY(CFS_IsValidFilename), true);
    UT_SetDefaultReturnValue(UT_KEY(CFS_ResolveSymAddr), true);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), true);

    /* Execute the function being tested */
    Result = MM_DumpInEventCmd((CFE_SB_Buffer_t *)(&MessagePtr));

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_DumpInEventCmd_Test_FillDumpInvalid */

void MM_FillDumpInEventBuffer_Test_RAM(void)
{
    MM_DumpInEventCmd_t MessagePtr;
    /* a valid source address is required input to memcpy */
    cpuaddr SrcAddress = (cpuaddr)&MM_AppData.LoadBuffer[0];
    bool    Result;

    MessagePtr.MemType              = MM_RAM;
    MessagePtr.NumOfBytes           = 1;
    MessagePtr.SrcSymAddress.Offset = 0;

    /* Execute the function being tested */
    Result = MM_FillDumpInEventBuffer(SrcAddress, &MessagePtr, (uint8 *)(&MM_AppData.DumpBuffer[0]));

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_FillDumpInEventBuffer_Test_RAM */

void MM_FillDumpInEventBuffer_Test_BadType(void)
{
    MM_DumpInEventCmd_t MessagePtr;
    /* a valid source address is required input to memcpy */
    cpuaddr SrcAddress = (cpuaddr)&MM_AppData.LoadBuffer[0];
    bool    Result;

    MessagePtr.MemType              = 99;
    MessagePtr.NumOfBytes           = 1;
    MessagePtr.SrcSymAddress.Offset = 0;

    /* Execute the function being tested */
    Result = MM_FillDumpInEventBuffer(SrcAddress, &MessagePtr, (uint8 *)(&MM_AppData.DumpBuffer[0]));

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_FillDumpInEventBuffer_Test_RAM */

void MM_FillDumpInEventBuffer_Test_EEPROM(void)
{
    MM_DumpInEventCmd_t MessagePtr;
    /* a valid source address is required input to memcpy */
    cpuaddr SrcAddress = (cpuaddr)&MM_AppData.LoadBuffer[0];
    bool    Result;

    MessagePtr.MemType              = MM_EEPROM;
    MessagePtr.NumOfBytes           = 1;
    MessagePtr.SrcSymAddress.Offset = 0;

    /* Execute the function being tested */
    Result = MM_FillDumpInEventBuffer(SrcAddress, &MessagePtr, (uint8 *)(&MM_AppData.DumpBuffer[0]));

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_FillDumpInEventBuffer_Test_EEPROM */

void MM_FillDumpInEventBuffer_Test_MEM32(void)
{
    MM_DumpInEventCmd_t MessagePtr;
    /* CFE_PSP_MemRead32 stub returns success with non-zero address */
    cpuaddr SrcAddress = 1;
    bool    Result;

    MessagePtr.MemType              = MM_MEM32;
    MessagePtr.NumOfBytes           = 4;
    MessagePtr.SrcSymAddress.Offset = 0;

    /* Execute the function being tested */
    Result = MM_FillDumpInEventBuffer(SrcAddress, &MessagePtr, (uint8 *)(&MM_AppData.DumpBuffer[0]));

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_FillDumpInEventBuffer_Test_MEM32 */

void MM_FillDumpInEventBuffer_Test_MEM16(void)
{
    MM_DumpInEventCmd_t MessagePtr;
    /* CFE_PSP_MemRead16 stub returns success with non-zero address */
    cpuaddr SrcAddress = 1;
    bool    Result;

    MessagePtr.MemType              = MM_MEM16;
    MessagePtr.NumOfBytes           = 2;
    MessagePtr.SrcSymAddress.Offset = 0;

    /* Execute the function being tested */
    Result = MM_FillDumpInEventBuffer(SrcAddress, &MessagePtr, (uint8 *)(&MM_AppData.DumpBuffer[0]));

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_FillDumpInEventBuffer_Test_MEM16 */

void MM_FillDumpInEventBuffer_Test_MEM8(void)
{
    MM_DumpInEventCmd_t MessagePtr;
    cpuaddr             SrcAddress = 0;
    bool                Result;

    MessagePtr.MemType              = MM_MEM8;
    MessagePtr.NumOfBytes           = 1;
    MessagePtr.SrcSymAddress.Offset = 0;

    /* Execute the function being tested */
    Result = MM_FillDumpInEventBuffer(SrcAddress, &MessagePtr, (uint8 *)(&MM_AppData.DumpBuffer[0]));

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_FillDumpInEventBuffer_Test_MEM8 */

void MM_FillDumpInEventBuffer_Test_MEM32ReadError(void)
{
    MM_DumpInEventCmd_t MessagePtr;
    cpuaddr             SrcAddress = 0;
    int32               strCmpResult;
    char                ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool                Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "PSP read memory error: RC=%%d, Src=%%p, Tgt=%%p, Type=MEM32");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent;
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    MessagePtr.MemType              = MM_MEM32;
    MessagePtr.NumOfBytes           = 4;
    MessagePtr.SrcSymAddress.Offset = 0;

    /* Set to generate error message MM_PSP_READ_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemRead32), 1, -1);

    /* Execute the function being tested */
    Result = MM_FillDumpInEventBuffer(SrcAddress, &MessagePtr, (uint8 *)(&MM_AppData.DumpBuffer[0]));

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventID, MM_PSP_READ_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent.Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_FillDumpInEventBuffer_Test_MEM32ReadError */

void MM_FillDumpInEventBuffer_Test_MEM16ReadError(void)
{
    MM_DumpInEventCmd_t MessagePtr;
    cpuaddr             SrcAddress = 0;
    int32               strCmpResult;
    char                ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool                Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "PSP read memory error: RC=%%d, Src=%%p, Tgt=%%p, Type=MEM16");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent;
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    MessagePtr.MemType              = MM_MEM16;
    MessagePtr.NumOfBytes           = 4;
    MessagePtr.SrcSymAddress.Offset = 0;

    /* Set to generate error message MM_PSP_READ_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemRead16), 1, -1);

    /* Execute the function being tested */
    Result = MM_FillDumpInEventBuffer(SrcAddress, &MessagePtr, (uint8 *)(&MM_AppData.DumpBuffer[0]));

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventID, MM_PSP_READ_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent.Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_FillDumpInEventBuffer_Test_MEM16ReadError */

void MM_FillDumpInEventBuffer_Test_MEM8ReadError(void)
{
    MM_DumpInEventCmd_t MessagePtr;
    cpuaddr             SrcAddress = 0;
    int32               strCmpResult;
    char                ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool                Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "PSP read memory error: RC=%%d, Src=%%p, Tgt=%%p, Type=MEM8");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent;
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    MessagePtr.MemType              = MM_MEM8;
    MessagePtr.NumOfBytes           = 4;
    MessagePtr.SrcSymAddress.Offset = 0;

    /* Set to generate error message MM_PSP_READ_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemRead8), 1, -1);

    /* Execute the function being tested */
    Result = MM_FillDumpInEventBuffer(SrcAddress, &MessagePtr, (uint8 *)(&MM_AppData.DumpBuffer[0]));

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventID, MM_PSP_READ_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent.Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_FillDumpInEventBuffer_Test_MEM8ReadError */

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
    UtTest_Add(MM_PeekMem_Test_NoLengthVerify, MM_Test_Setup, MM_Test_TearDown, "MM_PeekMem_Test_NoLengthVerify");

    /* CFE_PSP_MemCpy stub and CFS_SymAddr_t Offset type causes segmentation faults for these 2 tests */
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
    UtTest_Add(MM_DumpMemToFileCmd_Test_FilenameError, MM_Test_Setup, MM_Test_TearDown,
               "MM_DumpMemToFileCmd_Test_FilenameError");
    UtTest_Add(MM_DumpMemToFileCmd_Test_NoVerifyDumpParams, MM_Test_Setup, MM_Test_TearDown,
               "MM_DumpMemToFileCmd_Test_NoVerifyDumpParams");
    UtTest_Add(MM_DumpMemToFileCmd_Test_NoLengthVerify, MM_Test_Setup, MM_Test_TearDown,
               "MM_DumpMemToFileCmd_Test_NoLengthVerify");
    UtTest_Add(MM_DumpMemToFileCmd_Test_NoWriteHeaders, MM_Test_Setup, MM_Test_TearDown,
               "MM_DumpMemToFileCmd_Test_NoWriteHeaders");

    /* CFE_PSP_MemCpy stub and CFS_SymAddr_t Offset type causes segmentation faults for these 3 tests */
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
    UtTest_Add(MM_DumpInEventCmd_Test_NoLengthVerify, MM_Test_Setup, MM_Test_TearDown,
               "MM_DumpInEventCmd_Test_NoLengthVerify");
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
} /* end UtTest_Setup */

/************************/
/*  End of File Comment */
/************************/
