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
 *   Unit tests for mm_load.c
 */

/************************************************************************
** Includes
*************************************************************************/
#include "mm_load.h"
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

/* mm_load_tests globals */
uint8 call_count_CFE_EVS_SendEvent;

MM_LoadDumpFileHeader_t MMHeaderSave;

MM_LoadDumpFileHeader_t *MMHeaderRestore;

uint8 Buffer[MM_MAX_FILL_DATA_SEG * 2];

/*
 * Function Definitions
 */

int32 UT_MM_LOAD_TEST_MM_FillMemHook1(void *UserObj, int32 StubRetcode, uint32 CallCount,
                                      const UT_StubContext_t *Context)
{
    MM_AppData.HkPacket.LastAction = MM_FILL;

    return true;
}

int32 UT_MM_LOAD_TEST_MM_ComputeCrcHook1(void *UserObj, int32 StubRetcode, uint32 CallCount,
                                         const UT_StubContext_t *Context)
{
    uint32 *ComputedCRC = (uint32 *)Context->ArgPtr[1];

    if (UT_Stub_CopyToLocal(UT_KEY(MM_ComputeCRCFromFile), ComputedCRC, sizeof(*ComputedCRC)) == 0)
    {
        *ComputedCRC = 0;
    }

    return OS_SUCCESS;
}

int32 UT_MM_LOAD_TEST_CFE_SymbolLookupHook1(void *UserObj, int32 StubRetcode, uint32 CallCount,
                                            const UT_StubContext_t *Context)
{
    uint32 * SymbolAddress   = (uint32 *)Context->ArgPtr[0];
    cpuaddr *ResolvedAddress = (cpuaddr *)Context->ArgPtr[1];

    *ResolvedAddress = (cpuaddr)Buffer;
    *SymbolAddress   = 0;
    *MMHeaderRestore = MMHeaderSave;

    return true;
}

int32 UT_MM_LOAD_TEST_CFE_SymbolLookupHook2(void *UserObj, int32 StubRetcode, uint32 CallCount,
                                            const UT_StubContext_t *Context)
{
    uint32 * SymbolAddress   = (uint32 *)Context->ArgPtr[0];
    cpuaddr *ResolvedAddress = (cpuaddr *)Context->ArgPtr[1];

    *ResolvedAddress = (cpuaddr)Buffer;
    *SymbolAddress   = 0;
    *MMHeaderRestore = MMHeaderSave;

    return false;
}

int32 UT_MM_LOAD_TEST_CFE_SymbolLookupHook3(void *UserObj, int32 StubRetcode, uint32 CallCount,
                                            const UT_StubContext_t *Context)
{
    cpuaddr *ResolvedAddress = (cpuaddr *)Context->ArgPtr[1];

    *ResolvedAddress = (cpuaddr)Buffer;

    return true;
}

MM_MemType_t UT_MM_CFE_OS_ReadHook1_MemType;
int32        UT_MM_CFE_OS_ReadHook_RunCount;
int32        UT_MM_CFE_OS_ReadHook1(void *UserObj, int32 StubRetcode, uint32 CallCount, const UT_StubContext_t *Context)
{
    void *buffer = *(void **)Context->ArgPtr[1];

    MM_LoadDumpFileHeader_t *header = (MM_LoadDumpFileHeader_t *)buffer;

    ((MM_LoadDumpFileHeader_t *)(buffer))->NumOfBytes        = 4;
    ((MM_LoadDumpFileHeader_t *)(buffer))->Crc               = 0;
    ((MM_LoadDumpFileHeader_t *)(buffer))->MemType           = UT_MM_CFE_OS_ReadHook1_MemType;
    ((MM_LoadDumpFileHeader_t *)(buffer))->SymAddress.Offset = 0;

    strncpy(((MM_LoadDumpFileHeader_t *)(buffer))->SymAddress.SymName, "name", OS_MAX_SYM_LEN);
    MMHeaderRestore = header;
    MMHeaderSave    = *header;

    UT_MM_CFE_OS_ReadHook_RunCount++;
    if (UT_MM_CFE_OS_ReadHook_RunCount == 1)
    {
        return sizeof(MM_LoadDumpFileHeader_t);
    }
    else if (UT_MM_CFE_OS_ReadHook_RunCount == 2)
    {
        return 4;
    }
    else
    {
        return 0;
    }
}

int32 UT_MM_CFE_OS_ReadHook2(void *UserObj, int32 StubRetcode, uint32 CallCount, const UT_StubContext_t *Context)
{
    void *buffer = *(void **)Context->ArgPtr[1];

    MM_LoadDumpFileHeader_t *header = (MM_LoadDumpFileHeader_t *)buffer;

    ((MM_LoadDumpFileHeader_t *)(buffer))->NumOfBytes        = 4;
    ((MM_LoadDumpFileHeader_t *)(buffer))->Crc               = 0;
    ((MM_LoadDumpFileHeader_t *)(buffer))->MemType           = 99;
    ((MM_LoadDumpFileHeader_t *)(buffer))->SymAddress.Offset = 0;

    strncpy(((MM_LoadDumpFileHeader_t *)(buffer))->SymAddress.SymName, "name", OS_MAX_SYM_LEN);
    MMHeaderRestore = header;
    MMHeaderSave    = *header;

    return sizeof(MM_LoadDumpFileHeader_t);
}

int32 UT_MM_CFE_OS_ReadHook3(void *UserObj, int32 StubRetcode, uint32 CallCount, const UT_StubContext_t *Context)
{
    void *buffer = *(void **)Context->ArgPtr[1];

    MM_LoadDumpFileHeader_t *header = (MM_LoadDumpFileHeader_t *)buffer;

    ((MM_LoadDumpFileHeader_t *)(buffer))->NumOfBytes        = 4;
    ((MM_LoadDumpFileHeader_t *)(buffer))->Crc               = 100;
    ((MM_LoadDumpFileHeader_t *)(buffer))->MemType           = 99;
    ((MM_LoadDumpFileHeader_t *)(buffer))->SymAddress.Offset = 0;

    strncpy(((MM_LoadDumpFileHeader_t *)(buffer))->SymAddress.SymName, "name", OS_MAX_SYM_LEN);
    MMHeaderRestore = header;
    MMHeaderSave    = *header;

    return sizeof(MM_LoadDumpFileHeader_t);
}

int32 UT_MM_LOAD_TEST_CFE_OS_StatHook1(void *UserObj, int32 StubRetcode, uint32 CallCount,
                                       const UT_StubContext_t *Context)
{
    os_fstat_t filestats = {.FileSize = sizeof(CFE_FS_Header_t) + sizeof(MM_LoadDumpFileHeader_t)};

    /* Load buffer */
    UT_SetDataBuffer(UT_KEY(OS_stat), &filestats, sizeof(filestats), true);

    return OS_SUCCESS;
}

void MM_PokeCmd_Test_EEPROM(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    int32             strCmpResult;
    char              ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool              Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Poke Command: Addr = %%p, Size = 32 bits, Data = 0x%%08X");

    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    FcnCode   = MM_POKE_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_CmdBuf.PokeCmd.MemType  = MM_EEPROM;
    UT_CmdBuf.PokeCmd.DataSize = 32;

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_ResolveSymAddr), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyPeekPokeParams), 1, true);

    /* Execute the function being tested */
    Result = MM_PokeCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_POKE_DWORD_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_PokeCmd_Test_NonEEPROM(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    int32             strCmpResult;
    char              ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool              Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Poke Command: Addr = %%p, Size = %%d bits, Data = 0x%%08X");

    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    FcnCode   = MM_POKE_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_CmdBuf.PokeCmd.MemType  = MM_RAM;
    UT_CmdBuf.PokeCmd.DataSize = 32;

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_ResolveSymAddr), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyPeekPokeParams), 1, true);

    /* Execute the function being tested */
    Result = MM_PokeCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_POKE_DWORD_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_PokeCmd_Test_SymNameError(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    int32             strCmpResult;
    char              ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool              Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Symbolic address can't be resolved: Name = '%%s'");

    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    FcnCode   = MM_POKE_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_CmdBuf.PokeCmd.MemType  = MM_RAM;
    UT_CmdBuf.PokeCmd.DataSize = 32;

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_ResolveSymAddr), 1, false);

    /* Execute the function being tested */
    Result = MM_PokeCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_SYMNAME_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_PokeCmd_Test_NoVerifyCmdLength(void)
{
    bool Result;

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, false);

    /* Execute the function being tested */
    Result = MM_PokeCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_PokeCmd_Test_NoVerifyPeekPokeParams(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    bool              Result;

    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    FcnCode   = MM_POKE_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_CmdBuf.PokeCmd.MemType  = MM_RAM;
    UT_CmdBuf.PokeCmd.DataSize = 32;

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_ResolveSymAddr), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyPeekPokeParams), 1, false);

    /* Execute the function being tested */
    Result = MM_PokeCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_PokeMem_Test_NoDataSize(void)
{
    MM_PokeCmd_t CmdPacket;
    uint32       DestAddress;
    CmdPacket.MemType  = MM_RAM;
    CmdPacket.DataSize = 0;
    bool Result;

    DestAddress = 1;

    /* Execute the function being tested */
    Result = MM_PokeMem(&CmdPacket, DestAddress);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_PokeMem_Test_8bit(void)
{
    MM_PokeCmd_t CmdPacket;
    uint32       DestAddress;
    int32        strCmpResult;
    char         ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool         Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Poke Command: Addr = %%p, Size = %%d bits, Data = 0x%%08X");

    CmdPacket.MemType  = MM_RAM;
    CmdPacket.DataSize = MM_BYTE_BIT_WIDTH;
    CmdPacket.Data     = (uint8)(5);

    DestAddress = 1;

    /* Execute the function being tested */
    Result = MM_PokeMem(&CmdPacket, DestAddress);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    UtAssert_True(MM_AppData.HkPacket.LastAction == MM_POKE, "MM_AppData.HkPacket.LastAction == MM_POKE");
    UtAssert_True(MM_AppData.HkPacket.MemType == MM_RAM, "MM_AppData.HkPacket.MemType == MM_RAM");
    UtAssert_True(MM_AppData.HkPacket.Address == DestAddress, "MM_AppData.HkPacket.Address == DestAddress");
    UtAssert_True(MM_AppData.HkPacket.DataValue == 5, "MM_AppData.HkPacket.DataValue  == 5");
    UtAssert_True(MM_AppData.HkPacket.BytesProcessed == 1, "MM_AppData.HkPacket.BytesProcessed == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_POKE_BYTE_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_PokeMem_Test_8bitError(void)
{
    MM_PokeCmd_t CmdPacket;
    uint32       DestAddress;
    int32        strCmpResult;
    char         ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool         Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "PSP write memory error: RC=0x%%08X, Address=%%p, MemType=MEM%%d");

    CmdPacket.MemType  = MM_RAM;
    CmdPacket.DataSize = MM_BYTE_BIT_WIDTH;
    CmdPacket.Data     = (uint8)(5);

    /* CFE_PSP_MemWrite8 stub returns success with non-zero address */
    DestAddress = 0;

    /* Set to generate error message MM_PSP_WRITE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemWrite8), 1, -1);

    /* Execute the function being tested */
    Result = MM_PokeMem(&CmdPacket, DestAddress);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_PSP_WRITE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_PokeMem_Test_16bit(void)
{
    MM_PokeCmd_t CmdPacket;
    uint32       DestAddress;
    int32        strCmpResult;
    char         ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool         Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Poke Command: Addr = %%p, Size = %%d bits, Data = 0x%%08X");

    CmdPacket.MemType  = MM_RAM;
    CmdPacket.DataSize = MM_WORD_BIT_WIDTH;
    CmdPacket.Data     = (uint16)(5);

    DestAddress = 1;

    /* Execute the function being tested */
    Result = MM_PokeMem(&CmdPacket, DestAddress);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    UtAssert_True(MM_AppData.HkPacket.LastAction == MM_POKE, "MM_AppData.HkPacket.LastAction == MM_POKE");
    UtAssert_True(MM_AppData.HkPacket.MemType == MM_RAM, "MM_AppData.HkPacket.MemType == MM_RAM");
    UtAssert_True(MM_AppData.HkPacket.Address == DestAddress, "MM_AppData.HkPacket.Address == DestAddress");
    UtAssert_True(MM_AppData.HkPacket.DataValue == 5, "MM_AppData.HkPacket.DataValue  == 5");
    UtAssert_True(MM_AppData.HkPacket.BytesProcessed == 2, "MM_AppData.HkPacket.BytesProcessed == 2");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_POKE_WORD_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_PokeMem_Test_16bitError(void)
{
    MM_PokeCmd_t CmdPacket;
    uint32       DestAddress;
    int32        strCmpResult;
    char         ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool         Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "PSP write memory error: RC=0x%%08X, Address=%%p, MemType=MEM%%d");

    CmdPacket.MemType  = MM_RAM;
    CmdPacket.DataSize = MM_WORD_BIT_WIDTH;
    CmdPacket.Data     = (uint16)(5);

    /* CFE_PSP_MemWrite16 stub returns success with non-zero address */
    DestAddress = 0;

    /* Set to generate error message MM_PSP_WRITE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemWrite16), 1, -1);

    /* Execute the function being tested */
    Result = MM_PokeMem(&CmdPacket, DestAddress);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_PSP_WRITE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_PokeMem_Test_32bit(void)
{
    MM_PokeCmd_t CmdPacket;
    uint32       DestAddress;
    int32        strCmpResult;
    char         ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool         Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Poke Command: Addr = %%p, Size = %%d bits, Data = 0x%%08X");

    CmdPacket.MemType  = MM_RAM;
    CmdPacket.DataSize = MM_DWORD_BIT_WIDTH;
    CmdPacket.Data     = (uint32)(5);

    DestAddress = 1;

    /* Execute the function being tested */
    Result = MM_PokeMem(&CmdPacket, DestAddress);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    UtAssert_True(MM_AppData.HkPacket.LastAction == MM_POKE, "MM_AppData.HkPacket.LastAction == MM_POKE");
    UtAssert_True(MM_AppData.HkPacket.MemType == MM_RAM, "MM_AppData.HkPacket.MemType == MM_RAM");
    UtAssert_True(MM_AppData.HkPacket.Address == DestAddress, "MM_AppData.HkPacket.Address == DestAddress");
    UtAssert_True(MM_AppData.HkPacket.DataValue == 5, "MM_AppData.HkPacket.DataValue  == 5");
    UtAssert_True(MM_AppData.HkPacket.BytesProcessed == 4, "MM_AppData.HkPacket.BytesProcessed == 4");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_POKE_DWORD_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_PokeMem_Test_32bitError(void)
{
    MM_PokeCmd_t CmdPacket;
    uint32       DestAddress;
    int32        strCmpResult;
    char         ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool         Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "PSP write memory error: RC=0x%%08X, Address=%%p, MemType=MEM%%d");

    CmdPacket.MemType  = MM_RAM;
    CmdPacket.DataSize = MM_DWORD_BIT_WIDTH;
    CmdPacket.Data     = (uint32)(5);

    /* CFE_PSP_MemWrite32 stub returns success with non-zero address */
    DestAddress = 0;

    /* Set to generate error message MM_PSP_WRITE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemWrite32), 1, -1);

    /* Execute the function being tested */
    Result = MM_PokeMem(&CmdPacket, DestAddress);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_PSP_WRITE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_PokeEeprom_Test_NoDataSize(void)
{
    MM_PokeCmd_t CmdPacket;
    uint32       DestAddress;
    bool         Result;

    CmdPacket.MemType  = MM_EEPROM;
    CmdPacket.DataSize = 0;

    DestAddress = 1;

    /* Execute the function being tested */
    Result = MM_PokeEeprom(&CmdPacket, DestAddress);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_PokeEeprom_Test_8bit(void)
{
    MM_PokeCmd_t CmdPacket;
    uint32       DestAddress;
    int32        strCmpResult;
    char         ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool         Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Poke Command: Addr = %%p, Size = 8 bits, Data = 0x%%02X");

    CmdPacket.MemType  = MM_EEPROM;
    CmdPacket.DataSize = MM_BYTE_BIT_WIDTH;
    CmdPacket.Data     = (uint8)(5);

    DestAddress = 1;

    /* Execute the function being tested */
    Result = MM_PokeEeprom(&CmdPacket, DestAddress);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    UtAssert_True(MM_AppData.HkPacket.LastAction == MM_POKE, "MM_AppData.HkPacket.LastAction == MM_POKE");
    UtAssert_True(MM_AppData.HkPacket.MemType == MM_EEPROM, "MM_AppData.HkPacket.MemType == MM_EEPROM");
    UtAssert_True(MM_AppData.HkPacket.Address == DestAddress, "MM_AppData.HkPacket.Address == DestAddress");
    UtAssert_True(MM_AppData.HkPacket.DataValue == 5, "MM_AppData.HkPacket.DataValue  == 5");
    UtAssert_True(MM_AppData.HkPacket.BytesProcessed == 1, "MM_AppData.HkPacket.BytesProcessed == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_POKE_BYTE_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_PokeEeprom_Test_8bitError(void)
{
    MM_PokeCmd_t CmdPacket;
    uint32       DestAddress;
    int32        strCmpResult;
    char         ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool         Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "CFE_PSP_EepromWrite8 error received: RC = 0x%%08X, Addr = %%p");

    CmdPacket.MemType  = MM_EEPROM;
    CmdPacket.DataSize = MM_BYTE_BIT_WIDTH;
    CmdPacket.Data     = (uint8)(5);

    /* CFE_PSP_MemWrite8 stub returns success with non-zero address */
    DestAddress = 0;

    /* Set to generate error message MM_OS_EEPROMWRITE8_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_EepromWrite8), 1, -1);

    /* Execute the function being tested */
    Result = MM_PokeEeprom(&CmdPacket, DestAddress);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_OS_EEPROMWRITE8_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_PokeEeprom_Test_16bit(void)
{
    MM_PokeCmd_t CmdPacket;
    uint32       DestAddress;
    int32        strCmpResult;
    char         ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool         Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Poke Command: Addr = %%p, Size = 16 bits, Data = 0x%%04X");

    CmdPacket.MemType  = MM_EEPROM;
    CmdPacket.DataSize = MM_WORD_BIT_WIDTH;
    CmdPacket.Data     = (uint16)(5);

    DestAddress = 1;

    /* Execute the function being tested */
    Result = MM_PokeEeprom(&CmdPacket, DestAddress);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    UtAssert_True(MM_AppData.HkPacket.LastAction == MM_POKE, "MM_AppData.HkPacket.LastAction == MM_POKE");
    UtAssert_True(MM_AppData.HkPacket.MemType == MM_EEPROM, "MM_AppData.HkPacket.MemType == MM_EEPROM");
    UtAssert_True(MM_AppData.HkPacket.Address == DestAddress, "MM_AppData.HkPacket.Address == DestAddress");
    UtAssert_True(MM_AppData.HkPacket.DataValue == 5, "MM_AppData.HkPacket.DataValue  == 5");
    UtAssert_True(MM_AppData.HkPacket.BytesProcessed == 2, "MM_AppData.HkPacket.BytesProcessed == 2");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_POKE_WORD_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_PokeEeprom_Test_16bitError(void)
{
    MM_PokeCmd_t CmdPacket;
    uint32       DestAddress;
    int32        strCmpResult;
    char         ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool         Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "CFE_PSP_EepromWrite16 error received: RC = 0x%%08X, Addr = %%p");

    CmdPacket.MemType  = MM_EEPROM;
    CmdPacket.DataSize = MM_WORD_BIT_WIDTH;
    CmdPacket.Data     = (uint16)(5);

    /* CFE_PSP_MemWrite16 stub returns success with non-zero address */
    DestAddress = 0;

    /* Set to generate error message MM_OS_EEPROMWRITE16_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_EepromWrite16), 1, -1);

    /* Execute the function being tested */
    Result = MM_PokeEeprom(&CmdPacket, DestAddress);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_OS_EEPROMWRITE16_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_PokeEeprom_Test_32bit(void)
{
    MM_PokeCmd_t CmdPacket;
    uint32       DestAddress;
    int32        strCmpResult;
    char         ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool         Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Poke Command: Addr = %%p, Size = 32 bits, Data = 0x%%08X");

    CmdPacket.MemType  = MM_EEPROM;
    CmdPacket.DataSize = MM_DWORD_BIT_WIDTH;
    CmdPacket.Data     = (uint32)(5);

    DestAddress = 1;

    /* Execute the function being tested */
    Result = MM_PokeEeprom(&CmdPacket, DestAddress);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    UtAssert_True(MM_AppData.HkPacket.LastAction == MM_POKE, "MM_AppData.HkPacket.LastAction == MM_POKE");
    UtAssert_True(MM_AppData.HkPacket.MemType == MM_EEPROM, "MM_AppData.HkPacket.MemType == MM_EEPROM");
    UtAssert_True(MM_AppData.HkPacket.Address == DestAddress, "MM_AppData.HkPacket.Address == DestAddress");
    UtAssert_True(MM_AppData.HkPacket.DataValue == 5, "MM_AppData.HkPacket.DataValue  == 5");
    UtAssert_True(MM_AppData.HkPacket.BytesProcessed == 4, "MM_AppData.HkPacket.BytesProcessed == 4");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_POKE_DWORD_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_PokeEeprom_Test_32bitError(void)
{
    MM_PokeCmd_t CmdPacket;
    uint32       DestAddress;
    int32        strCmpResult;
    char         ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool         Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "CFE_PSP_EepromWrite32 error received: RC = 0x%%08X, Addr = %%p");

    CmdPacket.MemType  = MM_EEPROM;
    CmdPacket.DataSize = MM_DWORD_BIT_WIDTH;
    CmdPacket.Data     = (uint32)(5);

    /* CFE_PSP_MemWrite32 stub returns success with non-zero address */
    DestAddress = 0;

    /* Set to generate error message MM_OS_EEPROMWRITE32_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_EepromWrite32), 1, -1);

    /* Execute the function being tested */
    Result = MM_PokeEeprom(&CmdPacket, DestAddress);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_OS_EEPROMWRITE32_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_LoadMemWIDCmd_Test_Nominal(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    int32             strCmpResult;
    char              ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool              Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Load Memory WID Command: Wrote %%d bytes to address: %%p");

    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    FcnCode   = MM_LOAD_MEM_WID_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_CmdBuf.LoadMemWIDCmd.DataArray[0]          = 1;
    UT_CmdBuf.LoadMemWIDCmd.DataArray[1]          = 2;
    UT_CmdBuf.LoadMemWIDCmd.NumOfBytes            = 2;
    UT_CmdBuf.LoadMemWIDCmd.Crc                   = 0;
    UT_CmdBuf.LoadMemWIDCmd.DestSymAddress.Offset = 0;

    UT_CmdBuf.LoadMemWIDCmd.DestSymAddress.SymName[0] = '\0';

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_ResolveSymAddr), 1, true);

    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), true);

    /* Causes call to MM_ResolveSymAddr to return a known value for DestAddress */
    UT_SetHookFunction(UT_KEY(MM_ResolveSymAddr), UT_MM_LOAD_TEST_CFE_SymbolLookupHook3, 0);

    /* Execute the function being tested */
    Result = MM_LoadMemWIDCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_LOAD_WID_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_LoadMemWIDCmd_Test_NoVerifyCmdLength(void)
{
    bool Result;

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, false);

    /* Execute the function being tested */
    Result = MM_LoadMemWIDCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_LoadMemWIDCmd_Test_CRCError(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    int32             strCmpResult;
    char              ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool              Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Interrupts Disabled Load CRC failure: Expected = 0x%%X Calculated = 0x%%X");

    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    FcnCode   = MM_LOAD_MEM_WID_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_CmdBuf.LoadMemWIDCmd.DataArray[0]          = 1;
    UT_CmdBuf.LoadMemWIDCmd.DataArray[1]          = 2;
    UT_CmdBuf.LoadMemWIDCmd.NumOfBytes            = 2;
    UT_CmdBuf.LoadMemWIDCmd.Crc                   = 1;
    UT_CmdBuf.LoadMemWIDCmd.DestSymAddress.Offset = 1;

    UT_CmdBuf.LoadMemWIDCmd.DestSymAddress.SymName[0] = '\0';

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_ResolveSymAddr), 1, true);

    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), true);

    /* Execute the function being tested */
    Result = MM_LoadMemWIDCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_LOAD_WID_CRC_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_LoadMemWIDCmd_Test_SymNameErr(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    int32             strCmpResult;
    char              ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool              Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Symbolic address can't be resolved: Name = '%%s'");

    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    FcnCode   = MM_LOAD_MEM_WID_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_CmdBuf.LoadMemWIDCmd.DataArray[0]          = 1;
    UT_CmdBuf.LoadMemWIDCmd.DataArray[1]          = 2;
    UT_CmdBuf.LoadMemWIDCmd.NumOfBytes            = 2;
    UT_CmdBuf.LoadMemWIDCmd.Crc                   = 0;
    UT_CmdBuf.LoadMemWIDCmd.DestSymAddress.Offset = 1;

    UT_CmdBuf.LoadMemWIDCmd.DestSymAddress.SymName[0] = '\0';

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_ResolveSymAddr), 1, false);

    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), true);

    /* Execute the function being tested */
    Result = MM_LoadMemWIDCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_SYMNAME_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_LoadMemWIDCmd_Test_NoVerifyLoadWIDParams(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    bool              Result;

    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    FcnCode   = MM_LOAD_MEM_WID_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    UT_CmdBuf.LoadMemWIDCmd.DataArray[0]          = 1;
    UT_CmdBuf.LoadMemWIDCmd.DataArray[1]          = 2;
    UT_CmdBuf.LoadMemWIDCmd.NumOfBytes            = 0;
    UT_CmdBuf.LoadMemWIDCmd.Crc                   = 0;
    UT_CmdBuf.LoadMemWIDCmd.DestSymAddress.Offset = 1;

    UT_CmdBuf.LoadMemWIDCmd.DestSymAddress.SymName[0] = '\0';

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_ResolveSymAddr), 1, true);

    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), false);

    /* Execute the function being tested */
    Result = MM_LoadMemWIDCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_LoadMemFromFileCmd_Test_RAM(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool  Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Load Memory From File Command: Loaded %%d bytes to address %%p from file '%%s'");

    UT_MM_CFE_OS_ReadHook1_MemType = MM_RAM;

    strncpy(UT_CmdBuf.LoadMemFromFileCmd.FileName, "name", sizeof(UT_CmdBuf.LoadMemFromFileCmd.FileName) - 1);

    /* Causes call to MM_VerifyLoadFileSize to return true, in order to satisfy the immediately following condition
     * "Valid == true" */
    UT_SetHookFunction(UT_KEY(OS_stat), UT_MM_LOAD_TEST_CFE_OS_StatHook1, 0);

    /* Causes call to MM_ResolveSymAddr to return a known value for DestAddress */
    UT_SetHookFunction(UT_KEY(MM_ResolveSymAddr), UT_MM_LOAD_TEST_CFE_SymbolLookupHook1, 0);
    UT_SetDeferredRetcode(UT_KEY(MM_ResolveSymAddr), 1, true);

    /* Causes call to MM_LoadMemFromFile to return true, in order to generate event message MM_LD_MEM_FILE_INF_EID */
    UT_SetHookFunction(UT_KEY(OS_read), UT_MM_CFE_OS_ReadHook1, 0);
    UT_MM_CFE_OS_ReadHook_RunCount = 0;

    /* Causes call to MM_ComputeCRCFromFile to return 0 for ComputedCRC */
    UT_SetHookFunction(UT_KEY(MM_ComputeCRCFromFile), UT_MM_LOAD_TEST_MM_ComputeCrcHook1, 0);

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_LoadMemFromFile), 1, true);

    UT_SetDeferredRetcode(UT_KEY(CFE_FS_ReadHeader), 1, sizeof(CFE_FS_Header_t));

    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), true);

    /* Execute the function being tested */
    Result = MM_LoadMemFromFileCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_LD_MEM_FILE_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_LoadMemFromFileCmd_Test_BadType(void)
{
    bool Result;

    strncpy(UT_CmdBuf.LoadMemFromFileCmd.FileName, "name", sizeof(UT_CmdBuf.LoadMemFromFileCmd.FileName) - 1);

    /* Causes call to MM_VerifyLoadFileSize to return true, in order to satisfy the immediately following condition
     * "Valid == true" */
    UT_SetHookFunction(UT_KEY(OS_stat), UT_MM_LOAD_TEST_CFE_OS_StatHook1, 0);

    /* Causes call to MM_ResolveSymAddr to return a known value for DestAddress */
    UT_SetHookFunction(UT_KEY(MM_ResolveSymAddr), UT_MM_LOAD_TEST_CFE_SymbolLookupHook1, 0);
    UT_SetDeferredRetcode(UT_KEY(MM_ResolveSymAddr), 1, true);

    /* Causes call to MM_LoadMemFromFile to return true, in order to generate event message MM_LD_MEM_FILE_INF_EID */
    UT_SetHookFunction(UT_KEY(OS_read), UT_MM_CFE_OS_ReadHook2, 0);
    UT_MM_CFE_OS_ReadHook_RunCount = 0;

    /* Causes call to MM_ComputeCRCFromFile to return 0 for ComputedCRC */
    UT_SetHookFunction(UT_KEY(MM_ComputeCRCFromFile), UT_MM_LOAD_TEST_MM_ComputeCrcHook1, 0);

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_LoadMemFromFile), 1, true);

    UT_SetDeferredRetcode(UT_KEY(CFE_FS_ReadHeader), 1, sizeof(CFE_FS_Header_t));

    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), true);

    /* Execute the function being tested */
    Result = MM_LoadMemFromFileCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_LoadMemFromFileCmd_Test_EEPROM(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool  Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Load Memory From File Command: Loaded %%d bytes to address %%p from file '%%s'");

    UT_MM_CFE_OS_ReadHook1_MemType = MM_EEPROM;

    strncpy(UT_CmdBuf.LoadMemFromFileCmd.FileName, "name", sizeof(UT_CmdBuf.LoadMemFromFileCmd.FileName) - 1);

    /* Causes call to MM_VerifyLoadFileSize to return true, in order to satisfy the immediately following condition
     * "Valid == true" */
    UT_SetHookFunction(UT_KEY(OS_stat), UT_MM_LOAD_TEST_CFE_OS_StatHook1, 0);

    /* Causes call to MM_ResolveSymAddr to return a known value for DestAddress */
    UT_SetHookFunction(UT_KEY(MM_ResolveSymAddr), UT_MM_LOAD_TEST_CFE_SymbolLookupHook1, 0);
    UT_SetDeferredRetcode(UT_KEY(MM_ResolveSymAddr), 1, true);

    /* Causes call to MM_LoadMemFromFile to return true, in order to generate event message MM_LD_MEM_FILE_INF_EID */
    UT_SetHookFunction(UT_KEY(OS_read), UT_MM_CFE_OS_ReadHook1, 0);
    UT_MM_CFE_OS_ReadHook_RunCount = 0;

    /* Causes call to MM_ComputeCRCFromFile to return 0 for ComputedCRC */
    UT_SetHookFunction(UT_KEY(MM_ComputeCRCFromFile), UT_MM_LOAD_TEST_MM_ComputeCrcHook1, 0);

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, true);

    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), true);
    UT_SetDeferredRetcode(UT_KEY(MM_LoadMemFromFile), 1, true);

    /* Execute the function being tested */
    Result = MM_LoadMemFromFileCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_LD_MEM_FILE_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_LoadMemFromFileCmd_Test_MEM32(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool  Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Load Memory From File Command: Loaded %%d bytes to address %%p from file '%%s'");

    UT_MM_CFE_OS_ReadHook1_MemType = MM_MEM32;

    strncpy(UT_CmdBuf.LoadMemFromFileCmd.FileName, "name", sizeof(UT_CmdBuf.LoadMemFromFileCmd.FileName) - 1);

    /* Causes call to MM_VerifyLoadFileSize to return true, in order to satisfy the immediately following condition
     * "Valid == true" */
    UT_SetHookFunction(UT_KEY(OS_stat), UT_MM_LOAD_TEST_CFE_OS_StatHook1, 0);

    /* Causes call to MM_ResolveSymAddr to return a known value for DestAddress */
    UT_SetHookFunction(UT_KEY(MM_ResolveSymAddr), UT_MM_LOAD_TEST_CFE_SymbolLookupHook1, 0);
    UT_SetDeferredRetcode(UT_KEY(MM_ResolveSymAddr), 1, true);

    /* Causes call to MM_LoadMemFromFile to return true, in order to generate event message MM_LD_MEM_FILE_INF_EID */
    UT_SetHookFunction(UT_KEY(OS_read), UT_MM_CFE_OS_ReadHook1, 0);
    UT_MM_CFE_OS_ReadHook_RunCount = 0;

    /* Causes call to MM_ComputeCRCFromFile to return 0 for ComputedCRC */
    UT_SetHookFunction(UT_KEY(MM_ComputeCRCFromFile), UT_MM_LOAD_TEST_MM_ComputeCrcHook1, 0);

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_LoadMem32FromFile), 1, true);

    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), true);
    UT_SetDeferredRetcode(UT_KEY(MM_Verify32Aligned), 1, true);

    /* Execute the function being tested */
    Result = MM_LoadMemFromFileCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_LD_MEM_FILE_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_LoadMemFromFileCmd_Test_MEM32Invalid(void)
{
    bool Result;

    UT_MM_CFE_OS_ReadHook1_MemType = MM_MEM32;

    strncpy(UT_CmdBuf.LoadMemFromFileCmd.FileName, "name", sizeof(UT_CmdBuf.LoadMemFromFileCmd.FileName) - 1);

    /* Causes call to MM_VerifyLoadFileSize to return true, in order to satisfy the immediately following condition
     * "Valid == true" */
    UT_SetHookFunction(UT_KEY(OS_stat), UT_MM_LOAD_TEST_CFE_OS_StatHook1, 0);

    /* Causes call to MM_ResolveSymAddr to return a known value for DestAddress */
    UT_SetHookFunction(UT_KEY(MM_ResolveSymAddr), UT_MM_LOAD_TEST_CFE_SymbolLookupHook1, 0);
    UT_SetDeferredRetcode(UT_KEY(MM_ResolveSymAddr), 1, true);

    /* Causes call to MM_LoadMemFromFile to return true, in order to generate event message MM_LD_MEM_FILE_INF_EID */
    UT_SetHookFunction(UT_KEY(OS_read), UT_MM_CFE_OS_ReadHook1, 0);
    UT_MM_CFE_OS_ReadHook_RunCount = 0;

    /* Causes call to MM_ComputeCRCFromFile to return 0 for ComputedCRC */
    UT_SetHookFunction(UT_KEY(MM_ComputeCRCFromFile), UT_MM_LOAD_TEST_MM_ComputeCrcHook1, 0);

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_LoadMem32FromFile), 1, false);

    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), true);
    UT_SetDeferredRetcode(UT_KEY(MM_Verify32Aligned), 1, true);

    /* Execute the function being tested */
    Result = MM_LoadMemFromFileCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_LoadMemFromFileCmd_Test_MEM16(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool  Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Load Memory From File Command: Loaded %%d bytes to address %%p from file '%%s'");

    UT_MM_CFE_OS_ReadHook1_MemType = MM_MEM16;

    strncpy(UT_CmdBuf.LoadMemFromFileCmd.FileName, "name", sizeof(UT_CmdBuf.LoadMemFromFileCmd.FileName) - 1);

    /* Causes call to MM_VerifyLoadFileSize to return true, in order to satisfy the immediately following condition
     * "Valid == true" */
    UT_SetHookFunction(UT_KEY(OS_stat), UT_MM_LOAD_TEST_CFE_OS_StatHook1, 0);

    /* Causes call to MM_ResolveSymAddr to return a known value for DestAddress */
    UT_SetHookFunction(UT_KEY(MM_ResolveSymAddr), UT_MM_LOAD_TEST_CFE_SymbolLookupHook1, 0);
    UT_SetDeferredRetcode(UT_KEY(MM_ResolveSymAddr), 1, true);

    /* Causes call to MM_LoadMemFromFile to return true, in order to generate event message MM_LD_MEM_FILE_INF_EID */
    UT_SetHookFunction(UT_KEY(OS_read), UT_MM_CFE_OS_ReadHook1, 0);
    UT_MM_CFE_OS_ReadHook_RunCount = 0;

    /* Causes call to MM_ComputeCRCFromFile to return 0 for ComputedCRC */
    UT_SetHookFunction(UT_KEY(MM_ComputeCRCFromFile), UT_MM_LOAD_TEST_MM_ComputeCrcHook1, 0);

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_LoadMem16FromFile), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_Verify16Aligned), 1, true);

    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), true);

    /* Execute the function being tested */
    Result = MM_LoadMemFromFileCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_LD_MEM_FILE_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_LoadMemFromFileCmd_Test_MEM8(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool  Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Load Memory From File Command: Loaded %%d bytes to address %%p from file '%%s'");

    UT_MM_CFE_OS_ReadHook1_MemType = MM_MEM8;

    strncpy(UT_CmdBuf.LoadMemFromFileCmd.FileName, "name", sizeof(UT_CmdBuf.LoadMemFromFileCmd.FileName) - 1);

    /* Causes call to MM_VerifyLoadFileSize to return true, in order to satisfy the immediately following condition
     * "Valid == true" */
    UT_SetHookFunction(UT_KEY(OS_stat), UT_MM_LOAD_TEST_CFE_OS_StatHook1, 0);

    /* Causes call to MM_ResolveSymAddr to return a known value for DestAddress */
    UT_SetHookFunction(UT_KEY(MM_ResolveSymAddr), UT_MM_LOAD_TEST_CFE_SymbolLookupHook1, 0);
    UT_SetDeferredRetcode(UT_KEY(MM_ResolveSymAddr), 1, true);

    /* Causes call to MM_LoadMemFromFile to return true, in order to generate event message MM_LD_MEM_FILE_INF_EID */
    UT_SetHookFunction(UT_KEY(OS_read), UT_MM_CFE_OS_ReadHook1, 0);
    UT_MM_CFE_OS_ReadHook_RunCount = 0;

    /* Causes call to MM_ComputeCRCFromFile to return 0 for ComputedCRC */
    UT_SetHookFunction(UT_KEY(MM_ComputeCRCFromFile), UT_MM_LOAD_TEST_MM_ComputeCrcHook1, 0);

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_LoadMem8FromFile), 1, true);

    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), true);

    /* Execute the function being tested */
    Result = MM_LoadMemFromFileCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_LD_MEM_FILE_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_LoadMemFromFileCmd_Test_NoVerifyCmdLength(void)
{
    bool Result;

    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), true);
    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, false);

    /* Execute the function being tested */
    Result = MM_LoadMemFromFileCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_LoadMemFromFileCmd_Test_NoReadFileHeaders(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool  Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "CFE_FS_ReadHeader error received: RC = 0x%%08X Expected = %%u File = '%%s'");

    strncpy(UT_CmdBuf.LoadMemFromFileCmd.FileName, "name", sizeof(UT_CmdBuf.LoadMemFromFileCmd.FileName) - 1);

    /* Causes call to MM_VerifyLoadFileSize to return true, in order to satisfy the immediately following condition
     * "Valid == true" */
    UT_SetHookFunction(UT_KEY(OS_stat), UT_MM_LOAD_TEST_CFE_OS_StatHook1, 0);

    /* Causes call to MM_ResolveSymAddr to return a known value for DestAddress */
    UT_SetHookFunction(UT_KEY(MM_ResolveSymAddr), UT_MM_LOAD_TEST_CFE_SymbolLookupHook1, 0);
    UT_SetDeferredRetcode(UT_KEY(MM_ResolveSymAddr), 1, true);

    /* Causes call to MM_LoadMemFromFile to return true, in order to generate event message MM_LD_MEM_FILE_INF_EID */
    UT_SetHookFunction(UT_KEY(OS_read), UT_MM_CFE_OS_ReadHook1, 0);
    UT_MM_CFE_OS_ReadHook_RunCount = 0;

    /* Causes call to MM_ComputeCRCFromFile to return 0 for ComputedCRC */
    UT_SetHookFunction(UT_KEY(MM_ComputeCRCFromFile), UT_MM_LOAD_TEST_MM_ComputeCrcHook1, 0);

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_LoadMem8FromFile), 1, true);

    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), true);
    /* Set to satisfy condition "OS_Status != sizeof(CFE_FS_Header_t)" */
    UT_SetDeferredRetcode(UT_KEY(CFE_FS_ReadHeader), 1, 0);

    /* Execute the function being tested */
    Result = MM_LoadMemFromFileCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_CFE_FS_READHDR_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_LoadMemFromFileCmd_Test_NoVerifyLoadFileSize(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool  Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Load file size error: Reported by OS = %%d Expected = %%d File = '%%s'");

    strncpy(UT_CmdBuf.LoadMemFromFileCmd.FileName, "name", sizeof(UT_CmdBuf.LoadMemFromFileCmd.FileName) - 1);

    /* Causes call to MM_ResolveSymAddr to return a known value for DestAddress */
    UT_SetHookFunction(UT_KEY(MM_ResolveSymAddr), UT_MM_LOAD_TEST_CFE_SymbolLookupHook1, 0);
    UT_SetDeferredRetcode(UT_KEY(MM_ResolveSymAddr), 1, true);

    /* Causes call to MM_LoadMemFromFile to return true, in order to generate event message MM_LD_MEM_FILE_INF_EID */
    UT_SetHookFunction(UT_KEY(OS_read), UT_MM_CFE_OS_ReadHook1, 0);
    UT_MM_CFE_OS_ReadHook_RunCount = 0;

    /* Causes call to MM_ComputeCRCFromFile to return 0 for ComputedCRC */
    UT_SetHookFunction(UT_KEY(MM_ComputeCRCFromFile), UT_MM_LOAD_TEST_MM_ComputeCrcHook1, 0);

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_LoadMem8FromFile), 1, true);

    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), true);

    /* Execute the function being tested */
    Result = MM_LoadMemFromFileCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_LD_FILE_SIZE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_LoadMemFromFileCmd_Test_lseekError(void)
{
    UT_MM_CFE_OS_ReadHook1_MemType = MM_MEM8;
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool  Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Load file CRC failure: Expected = 0x%%X Calculated = 0x%%X File = '%%s'");

    strncpy(UT_CmdBuf.LoadMemFromFileCmd.FileName, "name", sizeof(UT_CmdBuf.LoadMemFromFileCmd.FileName) - 1);

    /* Causes call to MM_VerifyLoadFileSize to return true, in order to satisfy the immediately following condition
     * "Valid == true" */
    UT_SetHookFunction(UT_KEY(OS_stat), UT_MM_LOAD_TEST_CFE_OS_StatHook1, 0);

    /* Causes call to MM_ResolveSymAddr to return a known value for DestAddress */
    UT_SetHookFunction(UT_KEY(MM_ResolveSymAddr), UT_MM_LOAD_TEST_CFE_SymbolLookupHook1, 0);
    UT_SetDeferredRetcode(UT_KEY(MM_ResolveSymAddr), 1, true);

    /* Causes call to MM_LoadMemFromFile to return true, in order to generate event message MM_LD_MEM_FILE_INF_EID */
    UT_SetHookFunction(UT_KEY(OS_read), UT_MM_CFE_OS_ReadHook1, 0);
    UT_MM_CFE_OS_ReadHook_RunCount = 0;

    /* Causes call to MM_ComputeCRCFromFile to return 0 for ComputedCRC */
    UT_SetHookFunction(UT_KEY(MM_ComputeCRCFromFile), UT_MM_LOAD_TEST_MM_ComputeCrcHook1, 0);

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_LoadMem8FromFile), 1, false);

    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), true);

    UT_SetDeferredRetcode(UT_KEY(OS_lseek), 1, -1);

    /* Execute the function being tested */
    Result = MM_LoadMemFromFileCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_LOAD_FILE_CRC_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_LoadMemFromFileCmd_Test_LoadParamsError(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool  Result;

    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Load file failed parameters check: File = '%%s'");

    UT_MM_CFE_OS_ReadHook1_MemType = 99;

    strncpy(UT_CmdBuf.LoadMemFromFileCmd.FileName, "name", sizeof(UT_CmdBuf.LoadMemFromFileCmd.FileName) - 1);

    /* Causes call to MM_VerifyLoadFileSize to return true, in order to satisfy the immediately following condition
     * "Valid == true" */
    UT_SetHookFunction(UT_KEY(OS_stat), UT_MM_LOAD_TEST_CFE_OS_StatHook1, 0);

    /* Causes call to MM_ResolveSymAddr to return a known value for DestAddress */
    UT_SetHookFunction(UT_KEY(MM_ResolveSymAddr), UT_MM_LOAD_TEST_CFE_SymbolLookupHook1, 0);
    UT_SetDeferredRetcode(UT_KEY(MM_ResolveSymAddr), 1, true);

    /* Causes call to MM_LoadMemFromFile to return true, in order to generate event message MM_LD_MEM_FILE_INF_EID */
    UT_SetHookFunction(UT_KEY(OS_read), UT_MM_CFE_OS_ReadHook1, 0);
    UT_MM_CFE_OS_ReadHook_RunCount = 0;

    /* Causes call to MM_ComputeCRCFromFile to return 0 for ComputedCRC */
    UT_SetHookFunction(UT_KEY(MM_ComputeCRCFromFile), UT_MM_LOAD_TEST_MM_ComputeCrcHook1, 0);

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_LoadMem8FromFile), 1, true);

    /* Execute the function being tested */
    Result = MM_LoadMemFromFileCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_FILE_LOAD_PARAMS_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_LoadMemFromFileCmd_Test_SymNameError(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool  Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Symbolic address can't be resolved: Name = '%%s'");

    UT_MM_CFE_OS_ReadHook1_MemType = MM_MEM8;

    strncpy(UT_CmdBuf.LoadMemFromFileCmd.FileName, "name", sizeof(UT_CmdBuf.LoadMemFromFileCmd.FileName) - 1);

    /* Causes call to MM_VerifyLoadFileSize to return true, in order to satisfy the immediately following condition
     * "Valid == true" */
    UT_SetHookFunction(UT_KEY(OS_stat), UT_MM_LOAD_TEST_CFE_OS_StatHook1, 0);

    /* Set to generate error message MM_SYMNAME_ERR_EID */
    UT_SetHookFunction(UT_KEY(MM_ResolveSymAddr), UT_MM_LOAD_TEST_CFE_SymbolLookupHook2, 0);
    UT_SetDeferredRetcode(UT_KEY(MM_ResolveSymAddr), 1, false);

    /* Causes call to MM_LoadMemFromFile to return true, in order to generate event message MM_LD_MEM_FILE_INF_EID */
    UT_SetHookFunction(UT_KEY(OS_read), UT_MM_CFE_OS_ReadHook1, 0);
    UT_MM_CFE_OS_ReadHook_RunCount = 0;

    /* Causes call to MM_ComputeCRCFromFile to return 0 for ComputedCRC */
    UT_SetHookFunction(UT_KEY(MM_ComputeCRCFromFile), UT_MM_LOAD_TEST_MM_ComputeCrcHook1, 0);

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyLoadDumpParams), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_LoadMem8FromFile), 1, true);

    /* Execute the function being tested */
    Result = MM_LoadMemFromFileCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_SYMNAME_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_LoadMemFromFileCmd_Test_LoadFileCRCError(void)
{
    int32                   strCmpResult;
    char                    ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool                    Result;
    MM_LoadDumpFileHeader_t Hdr;
    uint32                  Crc = 99;
    Hdr.Crc                     = 99 + 1;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Load file CRC failure: Expected = 0x%%X Calculated = 0x%%X File = '%%s'");

    strncpy(UT_CmdBuf.LoadMemFromFileCmd.FileName, "name", sizeof(UT_CmdBuf.LoadMemFromFileCmd.FileName) - 1);

    /* Causes call to MM_VerifyLoadFileSize to return true, in order to satisfy the immediately following condition
     * "Valid == true" */

    UT_MM_CFE_OS_ReadHook1_MemType = MM_MEM8;
    UT_SetHookFunction(UT_KEY(OS_stat), UT_MM_LOAD_TEST_CFE_OS_StatHook1, 0);

    /* Causes call to MM_ResolveSymAddr to return a known value for DestAddress */
    UT_SetHookFunction(UT_KEY(MM_ResolveSymAddr), UT_MM_LOAD_TEST_CFE_SymbolLookupHook1, 0);
    UT_SetDeferredRetcode(UT_KEY(MM_ResolveSymAddr), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyLoadDumpParams), 1, true);

    /* Force non-zero crc */
    UT_SetDataBuffer(UT_KEY(MM_ComputeCRCFromFile), &Crc, sizeof(Crc), false);
    UT_SetHookFunction(UT_KEY(MM_ComputeCRCFromFile), UT_MM_LOAD_TEST_MM_ComputeCrcHook1, 0);

    UT_SetDataBuffer(UT_KEY(OS_read), &Hdr, sizeof(Hdr), false);
    UT_SetHookFunction(UT_KEY(OS_read), UT_MM_CFE_OS_ReadHook3, 0);
    UT_MM_CFE_OS_ReadHook_RunCount = 0;

    UT_SetDeferredRetcode(UT_KEY(MM_LoadMem8FromFile), 1, true);

    /* Execute the function being tested */
    Result = MM_LoadMemFromFileCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_LOAD_FILE_CRC_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_LoadMemFromFileCmd_Test_ComputeCRCError(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool  Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "MM_ComputeCRCFromFile error received: RC = 0x%%08X File = '%%s'");

    strncpy(UT_CmdBuf.LoadMemFromFileCmd.FileName, "name", sizeof(UT_CmdBuf.LoadMemFromFileCmd.FileName) - 1);

    /* Causes call to MM_VerifyLoadFileSize to return true, in order to satisfy the immediately following condition
     * "Valid == true" */
    UT_SetHookFunction(UT_KEY(OS_stat), UT_MM_LOAD_TEST_CFE_OS_StatHook1, 0);

    /* Causes call to MM_ResolveSymAddr to return a known value for DestAddress */
    UT_SetHookFunction(UT_KEY(MM_ResolveSymAddr), UT_MM_LOAD_TEST_CFE_SymbolLookupHook1, 0);
    UT_SetDeferredRetcode(UT_KEY(MM_ResolveSymAddr), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyLoadDumpParams), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_LoadMem8FromFile), 1, true);

    /* Causes call to MM_ComputeCRCFromFile to fail,
       in order to generate error message MM_LOAD_FILE_CRC_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(MM_ComputeCRCFromFile), 1, -1);

    /* Execute the function being tested */
    Result = MM_LoadMemFromFileCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_COMPUTECRCFROMFILE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_LoadMemFromFileCmd_Test_CloseError(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool  Result;

    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "OS_close error received: RC = 0x%%08X File = '%%s'");

    strncpy(UT_CmdBuf.LoadMemFromFileCmd.FileName, "name", sizeof(UT_CmdBuf.LoadMemFromFileCmd.FileName) - 1);

    /* Causes call to MM_VerifyLoadFileSize to return true, in order to satisfy the immediately following condition
     * "Valid == true" */
    UT_SetHookFunction(UT_KEY(OS_stat), UT_MM_LOAD_TEST_CFE_OS_StatHook1, 0);

    /* Causes call to MM_ResolveSymAddr to return a known value for DestAddress */
    UT_SetHookFunction(UT_KEY(MM_ResolveSymAddr), UT_MM_LOAD_TEST_CFE_SymbolLookupHook1, 0);
    UT_SetDeferredRetcode(UT_KEY(MM_ResolveSymAddr), 1, true);

    /* Causes call to MM_LoadMemFromFile to return true, in order to generate event message MM_LD_MEM_FILE_INF_EID */
    UT_SetHookFunction(UT_KEY(OS_read), UT_MM_CFE_OS_ReadHook1, 0);
    UT_MM_CFE_OS_ReadHook_RunCount = 0;

    /* Causes call to MM_ComputeCRCFromFile to return 0 for ComputedCRC */
    UT_SetHookFunction(UT_KEY(MM_ComputeCRCFromFile), UT_MM_LOAD_TEST_MM_ComputeCrcHook1, 0);

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_LoadMem8FromFile), 1, true);

    /* Set to generate error message MM_OS_CLOSE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(OS_close), 1, -1);

    /* Execute the function being tested */
    Result = MM_LoadMemFromFileCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, MM_OS_CLOSE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    /* Generates 1 event message we don't care about in this test */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 2, "CFE_EVS_SendEvent was called %u time(s), expected 2",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_LoadMemFromFileCmd_Test_OpenError(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool  Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "OS_OpenCreate error received: RC = %%d File = '%%s'");

    strncpy(UT_CmdBuf.LoadMemFromFileCmd.FileName, "name", sizeof(UT_CmdBuf.LoadMemFromFileCmd.FileName) - 1);

    /* Causes call to MM_VerifyLoadFileSize to return true, in order to satisfy the immediately following condition
     * "Valid == true" */
    UT_SetHookFunction(UT_KEY(OS_stat), UT_MM_LOAD_TEST_CFE_OS_StatHook1, 0);

    /* Causes call to MM_ResolveSymAddr to return a known value for DestAddress */
    UT_SetHookFunction(UT_KEY(MM_ResolveSymAddr), UT_MM_LOAD_TEST_CFE_SymbolLookupHook1, 0);
    UT_SetDeferredRetcode(UT_KEY(MM_ResolveSymAddr), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_LoadMem8FromFile), 1, true);

    /* Set to generate error message MM_OS_OPEN_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(OS_OpenCreate), 1, -1);

    /* Execute the function being tested */
    Result = MM_LoadMemFromFileCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_OS_OPEN_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_LoadMemFromFile_Test_PreventCPUHogging(void)
{
    bool                    Result;
    MM_LoadDumpFileHeader_t FileHeader;
    char                    FileName[] = "filename";

    FileHeader.MemType    = MM_EEPROM;
    FileHeader.NumOfBytes = 2 * MM_MAX_LOAD_DATA_SEG;

    /* Set to satisfy condition "(ReadLength = OS_read(FileHandle, ioBuffer, SegmentSize)) == SegmentSize" */
    UT_SetDefaultReturnValue(UT_KEY(OS_read), MM_MAX_LOAD_DATA_SEG);

    /* Execute the function being tested */
    Result = MM_LoadMemFromFile(MM_UT_OBJID_1, FileName, &FileHeader, (cpuaddr)&Buffer[0]);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");
    UtAssert_True(MM_AppData.HkPacket.LastAction == MM_LOAD_FROM_FILE,
                  "MM_AppData.HkPacket.LastAction == MM_LOAD_FROM_FILE");
    UtAssert_True(MM_AppData.HkPacket.MemType == MM_EEPROM, "MM_AppData.HkPacket.MemType == MM_EEPROM");
    UtAssert_True(MM_AppData.HkPacket.Address == (cpuaddr)(&Buffer[0]), "MM_AppData.HkPacket.Address == 0");
    UtAssert_True(MM_AppData.HkPacket.BytesProcessed == 2 * MM_MAX_LOAD_DATA_SEG,
                  "MM_AppData.HkPacket.BytesProcessed == 2*MM_MAX_LOAD_DATA_SEG");
    UtAssert_STRINGBUF_EQ(MM_AppData.HkPacket.FileName, sizeof(MM_AppData.HkPacket.FileName), FileName,
                          sizeof(FileName));

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_LoadMemFromFile_Test_ReadError(void)
{
    bool                    Result;
    MM_LoadDumpFileHeader_t FileHeader;
    FileHeader.MemType    = MM_EEPROM;
    FileHeader.NumOfBytes = 2;
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "OS_read error received: RC = 0x%%08X Expected = %%d File = '%%s'");

    /* Set to generate error message MM_OS_READ_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(OS_read), 1, 0);

    /* Execute the function being tested */
    Result = MM_LoadMemFromFile(MM_UT_OBJID_1, (char *)"filename", &FileHeader, (cpuaddr)&Buffer[0]);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_OS_READ_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_LoadMemFromFile_Test_NotEepromMemType(void)
{
    bool                    Result;
    MM_LoadDumpFileHeader_t FileHeader;
    char                    FileName[] = "filename";

    FileHeader.MemType    = MM_MEM8;
    FileHeader.NumOfBytes = 2 * MM_MAX_LOAD_DATA_SEG;

    /* Set to satisfy condition "(ReadLength = OS_read(FileHandle, ioBuffer, SegmentSize)) == SegmentSize" */
    UT_SetDefaultReturnValue(UT_KEY(OS_read), MM_MAX_LOAD_DATA_SEG);

    /* Execute the function being tested */
    Result = MM_LoadMemFromFile(MM_UT_OBJID_1, FileName, &FileHeader, (cpuaddr)&Buffer[0]);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");
    UtAssert_True(MM_AppData.HkPacket.LastAction == MM_LOAD_FROM_FILE,
                  "MM_AppData.HkPacket.LastAction == MM_LOAD_FROM_FILE");
    UtAssert_True(MM_AppData.HkPacket.MemType == MM_MEM8, "MM_AppData.HkPacket.MemType == MM_MEM8");
    UtAssert_True(MM_AppData.HkPacket.Address == (cpuaddr)(&Buffer[0]), "MM_AppData.HkPacket.Address == 0");
    UtAssert_True(MM_AppData.HkPacket.BytesProcessed == 2 * MM_MAX_LOAD_DATA_SEG,
                  "MM_AppData.HkPacket.BytesProcessed == 2*MM_MAX_LOAD_DATA_SEG");
    UtAssert_STRINGBUF_EQ(MM_AppData.HkPacket.FileName, sizeof(MM_AppData.HkPacket.FileName), FileName,
                          sizeof(FileName));

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_ReadFileHeaders_Test_ReadHeaderError(void)
{
    bool                    Result;
    osal_id_t               FileHandle = MM_UT_OBJID_1;
    CFE_FS_Header_t         CFEHeader;
    MM_LoadDumpFileHeader_t MMHeader;
    int32                   strCmpResult;
    char                    ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "CFE_FS_ReadHeader error received: RC = 0x%%08X Expected = %%u File = '%%s'");

    /* Set to satisfy condition "OS_Status != sizeof(CFE_FS_Header_t)" */
    UT_SetDeferredRetcode(UT_KEY(CFE_FS_ReadHeader), 1, 0);

    /* Execute the function being tested */
    Result = MM_ReadFileHeaders((char *)"filename", FileHandle, &CFEHeader, &MMHeader);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_CFE_FS_READHDR_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_ReadFileHeaders_Test_ReadError(void)
{
    bool                    Result;
    osal_id_t               FileHandle = MM_UT_OBJID_1;
    CFE_FS_Header_t         CFEHeader;
    MM_LoadDumpFileHeader_t MMHeader;
    int32                   strCmpResult;
    char                    ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "OS_read error received: RC = 0x%%08X Expected = %%u File = '%%s'");

    /* Set to generate error message MM_OS_READ_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(OS_read), 1, 0);

    /* Execute the function being tested */
    Result = MM_ReadFileHeaders((char *)"filename", FileHandle, &CFEHeader, &MMHeader);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_OS_READ_EXP_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_VerifyLoadFileSize_Test_StatError(void)
{
    bool                    Result;
    MM_LoadDumpFileHeader_t FileHeader;
    int32                   strCmpResult;
    char                    ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "OS_stat error received: RC = 0x%%08X File = '%%s'");

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(OS_stat), 1, -1);

    /* Execute the function being tested */
    Result = MM_VerifyLoadFileSize((char *)"filename", &FileHeader);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_OS_STAT_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_VerifyLoadFileSize_Test_SizeError(void)
{
    bool                    Result;
    MM_LoadDumpFileHeader_t FileHeader;
    int32                   strCmpResult;
    char                    ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Load file size error: Reported by OS = %%d Expected = %%d File = '%%s'");

    FileHeader.NumOfBytes = 99;

    /* Generates error message MM_LD_FILE_SIZE_ERR_EID */
    UT_SetHookFunction(UT_KEY(OS_stat), UT_MM_LOAD_TEST_CFE_OS_StatHook1, 0);

    /* Execute the function being tested */
    Result = MM_VerifyLoadFileSize((char *)"filename", &FileHeader);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_LD_FILE_SIZE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_FillMemCmd_Test_RAM(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool  Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Fill Memory Command: Filled %%d bytes at address: %%p with pattern: 0x%%08X");

    /* Causes call to MM_ResolveSymAddr to return a known value for DestAddress */
    UT_SetHookFunction(UT_KEY(MM_ResolveSymAddr), UT_MM_LOAD_TEST_CFE_SymbolLookupHook3, 0);

    UT_CmdBuf.FillMemCmd.MemType    = MM_RAM;
    UT_CmdBuf.FillMemCmd.NumOfBytes = 1;

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyLoadDumpParams), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_ResolveSymAddr), 1, true);

    /* Execute the function being tested */
    Result = MM_FillMemCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_FILL_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_FillMemCmd_Test_EEPROM(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool  Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Fill Memory Command: Filled %%d bytes at address: %%p with pattern: 0x%%08X");

    /* Causes call to MM_ResolveSymAddr to return a known value for DestAddress */
    UT_SetHookFunction(UT_KEY(MM_ResolveSymAddr), UT_MM_LOAD_TEST_CFE_SymbolLookupHook3, 0);

    UT_CmdBuf.FillMemCmd.MemType    = MM_EEPROM;
    UT_CmdBuf.FillMemCmd.NumOfBytes = 1;

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyLoadDumpParams), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_ResolveSymAddr), 1, true);

    /* Execute the function being tested */
    Result = MM_FillMemCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_FILL_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_FillMemCmd_Test_MEM32(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool  Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Fill Memory Command: Filled %%d bytes at address: %%p with pattern: 0x%%08X");

    UT_CmdBuf.FillMemCmd.MemType    = MM_MEM32;
    UT_CmdBuf.FillMemCmd.NumOfBytes = 4;

    /* Causes MM_AppData.HkPacket.LastAction == MM_FILL */
    UT_SetHookFunction(UT_KEY(MM_FillMem32), UT_MM_LOAD_TEST_MM_FillMemHook1, 0);

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyLoadDumpParams), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_ResolveSymAddr), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_Verify32Aligned), 1, true);

    /* Execute the function being tested */
    Result = MM_FillMemCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_FILL_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_FillMemCmd_Test_MEM16(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool  Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Fill Memory Command: Filled %%d bytes at address: %%p with pattern: 0x%%08X");

    UT_CmdBuf.FillMemCmd.MemType    = MM_MEM16;
    UT_CmdBuf.FillMemCmd.NumOfBytes = 2;

    /* Causes MM_AppData.HkPacket.LastAction == MM_FILL */
    UT_SetHookFunction(UT_KEY(MM_FillMem16), UT_MM_LOAD_TEST_MM_FillMemHook1, 0);

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyLoadDumpParams), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_ResolveSymAddr), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_Verify16Aligned), 1, true);

    /* Execute the function being tested */
    Result = MM_FillMemCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_FILL_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_FillMemCmd_Test_MEM8(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool  Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Fill Memory Command: Filled %%d bytes at address: %%p with pattern: 0x%%08X");

    UT_CmdBuf.FillMemCmd.MemType    = MM_MEM8;
    UT_CmdBuf.FillMemCmd.NumOfBytes = 1;

    /* Causes MM_AppData.HkPacket.LastAction == MM_FILL */
    UT_SetHookFunction(UT_KEY(MM_FillMem8), UT_MM_LOAD_TEST_MM_FillMemHook1, 0);

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyLoadDumpParams), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_ResolveSymAddr), 1, true);

    /* Execute the function being tested */
    Result = MM_FillMemCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_FILL_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_FillMemCmd_Test_SymNameError(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool  Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Symbolic address can't be resolved: Name = '%%s'");

    UT_CmdBuf.FillMemCmd.MemType    = MM_MEM8;
    UT_CmdBuf.FillMemCmd.NumOfBytes = 1;

    /* Causes MM_AppData.HkPacket.LastAction == MM_FILL */
    UT_SetHookFunction(UT_KEY(MM_FillMem8), UT_MM_LOAD_TEST_MM_FillMemHook1, 0);

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, true);

    /* Execute the function being tested */
    Result = MM_FillMemCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_SYMNAME_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_FillMemCmd_Test_NoVerifyCmdLength(void)
{
    bool Result;

    /* Causes MM_AppData.HkPacket.LastAction == MM_FILL */
    UT_SetHookFunction(UT_KEY(MM_FillMem8), UT_MM_LOAD_TEST_MM_FillMemHook1, 0);

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, false);

    /* Execute the function being tested */
    Result = MM_FillMemCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_FillMemCmd_Test_NoLastActionFill(void)
{
    bool Result;

    UT_CmdBuf.FillMemCmd.MemType    = MM_MEM8;
    UT_CmdBuf.FillMemCmd.NumOfBytes = 1;

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyLoadDumpParams), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_ResolveSymAddr), 1, true);

    /* Execute the function being tested */
    Result = MM_FillMemCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_FillMemCmd_Test_NoVerifyLoadDump(void)
{
    bool Result;

    UT_CmdBuf.FillMemCmd.MemType    = MM_MEM8;
    UT_CmdBuf.FillMemCmd.NumOfBytes = 1;

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyLoadDumpParams), 1, false);

    UT_SetDeferredRetcode(UT_KEY(MM_ResolveSymAddr), 1, true);

    /* Execute the function being tested */
    Result = MM_FillMemCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_FillMemCmd_Test_BadType(void)
{
    bool Result;

    /* Causes call to MM_ResolveSymAddr to return a known value for DestAddress */
    UT_SetHookFunction(UT_KEY(MM_ResolveSymAddr), UT_MM_LOAD_TEST_CFE_SymbolLookupHook3, 0);

    UT_CmdBuf.FillMemCmd.MemType    = 99;
    UT_CmdBuf.FillMemCmd.NumOfBytes = 1;

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyCmdLength), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_VerifyLoadDumpParams), 1, true);

    UT_SetDeferredRetcode(UT_KEY(MM_ResolveSymAddr), 1, true);

    /* Execute the function being tested */
    Result = MM_FillMemCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_FillMem_Test_Nominal(void)
{
    MM_FillMemCmd_t CmdPacket;
    bool            Result;

    memset(&CmdPacket, 0, sizeof(CmdPacket));

    CmdPacket.MemType    = MM_EEPROM;
    CmdPacket.NumOfBytes = 2;
    memset(Buffer, 1, (MM_MAX_FILL_DATA_SEG * 2));

    /* Execute the function being tested */
    Result = MM_FillMem((cpuaddr)Buffer, &CmdPacket);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    UtAssert_True(MM_AppData.HkPacket.LastAction == MM_FILL, "MM_AppData.HkPacket.LastAction == MM_FILL");
    UtAssert_True(MM_AppData.HkPacket.MemType == CmdPacket.MemType, "MM_AppData.HkPacket.MemType == CmdPacket.MemType");
    UtAssert_True(MM_AppData.HkPacket.Address == (cpuaddr)Buffer, "MM_AppData.HkPacket.Address == (cpuaddr)Buffer");
    UtAssert_True(MM_AppData.HkPacket.DataValue == CmdPacket.FillPattern,
                  "MM_AppData.HkPacket.DataValue == CmdPacket.FillPattern");
    UtAssert_True(MM_AppData.HkPacket.BytesProcessed == 2, "MM_AppData.HkPacket.BytesProcessed == 2");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_FillMem_Test_MaxFillDataSegment(void)
{
    MM_FillMemCmd_t CmdPacket;
    bool            Result;

    memset(&CmdPacket, 0, sizeof(CmdPacket));

    CmdPacket.MemType    = MM_EEPROM;
    CmdPacket.NumOfBytes = MM_MAX_FILL_DATA_SEG + 1;

    memset(Buffer, 1, (MM_MAX_FILL_DATA_SEG * 2));
    /* Execute the function being tested */
    Result = MM_FillMem((cpuaddr)Buffer, &CmdPacket);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    UtAssert_True(MM_AppData.HkPacket.LastAction == MM_FILL, "MM_AppData.HkPacket.LastAction == MM_FILL");
    UtAssert_True(MM_AppData.HkPacket.MemType == CmdPacket.MemType, "MM_AppData.HkPacket.MemType == CmdPacket.MemType");
    UtAssert_True(MM_AppData.HkPacket.Address == (cpuaddr)Buffer, "MM_AppData.HkPacket.Address == (cpuaddr)Buffer");
    UtAssert_True(MM_AppData.HkPacket.DataValue == CmdPacket.FillPattern,
                  "MM_AppData.HkPacket.DataValue == CmdPacket.FillPattern");
    UtAssert_True(MM_AppData.HkPacket.BytesProcessed == MM_MAX_FILL_DATA_SEG + 1,
                  "MM_AppData.HkPacket.BytesProcessed == MM_MAX_FILL_DATA_SEG + 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

/*
 * Register the test cases to execute with the unit test tool
 */
void UtTest_Setup(void)
{
    UtTest_Add(MM_PokeCmd_Test_EEPROM, MM_Test_Setup, MM_Test_TearDown, "MM_PokeCmd_Test_EEPROM");
    UtTest_Add(MM_PokeCmd_Test_NonEEPROM, MM_Test_Setup, MM_Test_TearDown, "MM_PokeCmd_Test_NonEEPROM");
    UtTest_Add(MM_PokeCmd_Test_SymNameError, MM_Test_Setup, MM_Test_TearDown, "MM_PokeCmd_Test_SymNameError");
    UtTest_Add(MM_PokeCmd_Test_NoVerifyCmdLength, MM_Test_Setup, MM_Test_TearDown, "MM_PokeCmd_Test_NoVerifyCmdLength");
    UtTest_Add(MM_PokeCmd_Test_NoVerifyPeekPokeParams, MM_Test_Setup, MM_Test_TearDown,
               "MM_PokeCmd_Test_NoVerifyPeekPokeParams");
    UtTest_Add(MM_PokeMem_Test_NoDataSize, MM_Test_Setup, MM_Test_TearDown, "MM_PokeMem_Test_NoDataSize");
    UtTest_Add(MM_PokeMem_Test_8bit, MM_Test_Setup, MM_Test_TearDown, "MM_PokeMem_Test_8bit");
    UtTest_Add(MM_PokeMem_Test_8bitError, MM_Test_Setup, MM_Test_TearDown, "MM_PokeMem_Test_8bitError");
    UtTest_Add(MM_PokeMem_Test_16bit, MM_Test_Setup, MM_Test_TearDown, "MM_PokeMem_Test_16bit");
    UtTest_Add(MM_PokeMem_Test_16bitError, MM_Test_Setup, MM_Test_TearDown, "MM_PokeMem_Test_16bitError");
    UtTest_Add(MM_PokeMem_Test_32bit, MM_Test_Setup, MM_Test_TearDown, "MM_PokeMem_Test_32bit");
    UtTest_Add(MM_PokeMem_Test_32bitError, MM_Test_Setup, MM_Test_TearDown, "MM_PokeMem_Test_32bitError");
    UtTest_Add(MM_PokeEeprom_Test_NoDataSize, MM_Test_Setup, MM_Test_TearDown, "MM_PokeMem_Test_NoDataSize");
    UtTest_Add(MM_PokeEeprom_Test_8bit, MM_Test_Setup, MM_Test_TearDown, "MM_PokeEeprom_Test_8bit");
    UtTest_Add(MM_PokeEeprom_Test_8bitError, MM_Test_Setup, MM_Test_TearDown, "MM_PokeEeprom_Test_8bitError");
    UtTest_Add(MM_PokeEeprom_Test_16bit, MM_Test_Setup, MM_Test_TearDown, "MM_PokeEeprom_Test_16bit");
    UtTest_Add(MM_PokeEeprom_Test_16bitError, MM_Test_Setup, MM_Test_TearDown, "MM_PokeEeprom_Test_16bitError");
    UtTest_Add(MM_PokeEeprom_Test_32bit, MM_Test_Setup, MM_Test_TearDown, "MM_PokeEeprom_Test_32bit");
    UtTest_Add(MM_PokeEeprom_Test_32bitError, MM_Test_Setup, MM_Test_TearDown, "MM_PokeEeprom_Test_32bitError");

    UtTest_Add(MM_LoadMemWIDCmd_Test_Nominal, MM_Test_Setup, MM_Test_TearDown, "MM_LoadMemWIDCmd_Test_Nominal");
    UtTest_Add(MM_LoadMemWIDCmd_Test_NoVerifyCmdLength, MM_Test_Setup, MM_Test_TearDown,
               "MM_LoadMemWIDCmd_Test_NoVerifyCmdLength");
    UtTest_Add(MM_LoadMemWIDCmd_Test_CRCError, MM_Test_Setup, MM_Test_TearDown, "MM_LoadMemWIDCmd_Test_CRCError");
    UtTest_Add(MM_LoadMemWIDCmd_Test_SymNameErr, MM_Test_Setup, MM_Test_TearDown, "MM_LoadMemWIDCmd_Test_SymNameErr");
    UtTest_Add(MM_LoadMemWIDCmd_Test_NoVerifyLoadWIDParams, MM_Test_Setup, MM_Test_TearDown,
               "MM_LoadMemWIDCmd_Test_NoVerifyLoadWIDParams");

    UtTest_Add(MM_LoadMemFromFileCmd_Test_RAM, MM_Test_Setup, MM_Test_TearDown, "MM_LoadMemFromFileCmd_Test_RAM");
    UtTest_Add(MM_LoadMemFromFileCmd_Test_BadType, MM_Test_Setup, MM_Test_TearDown,
               "MM_LoadMemFromFileCmd_Test_BadType");
    UtTest_Add(MM_LoadMemFromFileCmd_Test_EEPROM, MM_Test_Setup, MM_Test_TearDown, "MM_LoadMemFromFileCmd_Test_EEPROM");
    UtTest_Add(MM_LoadMemFromFileCmd_Test_MEM32, MM_Test_Setup, MM_Test_TearDown, "MM_LoadMemFromFileCmd_Test_MEM32");
    UtTest_Add(MM_LoadMemFromFileCmd_Test_MEM32Invalid, MM_Test_Setup, MM_Test_TearDown,
               "MM_LoadMemFromFileCmd_Test_MEM32Invalid");
    UtTest_Add(MM_LoadMemFromFileCmd_Test_MEM16, MM_Test_Setup, MM_Test_TearDown, "MM_LoadMemFromFileCmd_Test_MEM16");
    UtTest_Add(MM_LoadMemFromFileCmd_Test_MEM8, MM_Test_Setup, MM_Test_TearDown, "MM_LoadMemFromFileCmd_Test_MEM8");
    UtTest_Add(MM_LoadMemFromFileCmd_Test_NoVerifyCmdLength, MM_Test_Setup, MM_Test_TearDown,
               "MM_LoadMemFromFileCmd_Test_NoVerifyCmdLength");
    UtTest_Add(MM_LoadMemFromFileCmd_Test_NoReadFileHeaders, MM_Test_Setup, MM_Test_TearDown,
               "MM_LoadMemFromFileCmd_Test_NoReadFileHeaders");
    UtTest_Add(MM_LoadMemFromFileCmd_Test_NoVerifyLoadFileSize, MM_Test_Setup, MM_Test_TearDown,
               "MM_LoadMemFromFileCmd_Test_NoVerifyLoadFileSize");
    UtTest_Add(MM_LoadMemFromFileCmd_Test_lseekError, MM_Test_Setup, MM_Test_TearDown,
               "MM_LoadMemFromFileCmd_Test_lseekError");

    UtTest_Add(MM_LoadMemFromFileCmd_Test_LoadParamsError, MM_Test_Setup, MM_Test_TearDown,
               "MM_LoadMemFromFileCmd_Test_LoadParamsError");
    UtTest_Add(MM_LoadMemFromFileCmd_Test_SymNameError, MM_Test_Setup, MM_Test_TearDown,
               "MM_LoadMemFromFileCmd_Test_SymNameError");
    UtTest_Add(MM_LoadMemFromFileCmd_Test_LoadFileCRCError, MM_Test_Setup, MM_Test_TearDown,
               "MM_LoadMemFromFileCmd_Test_LoadFileCRCError");
    UtTest_Add(MM_LoadMemFromFileCmd_Test_ComputeCRCError, MM_Test_Setup, MM_Test_TearDown,
               "MM_LoadMemFromFileCmd_Test_ComputeCRCError");
    UtTest_Add(MM_LoadMemFromFileCmd_Test_CloseError, MM_Test_Setup, MM_Test_TearDown,
               "MM_LoadMemFromFileCmd_Test_CloseError");
    UtTest_Add(MM_LoadMemFromFileCmd_Test_OpenError, MM_Test_Setup, MM_Test_TearDown,
               "MM_LoadMemFromFileCmd_Test_OpenError");

    UtTest_Add(MM_LoadMemFromFile_Test_PreventCPUHogging, MM_Test_Setup, MM_Test_TearDown,
               "MM_LoadMemFromFile_Test_PreventCPUHogging");
    UtTest_Add(MM_LoadMemFromFile_Test_ReadError, MM_Test_Setup, MM_Test_TearDown, "MM_LoadMemFromFile_Test_ReadError");
    UtTest_Add(MM_LoadMemFromFile_Test_NotEepromMemType, MM_Test_Setup, MM_Test_TearDown,
               "MM_LoadMemFromFile_Test_NotEepromMemType");
    UtTest_Add(MM_VerifyLoadFileSize_Test_StatError, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadFileSize_Test_StatError");
    UtTest_Add(MM_VerifyLoadFileSize_Test_SizeError, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadFileSize_Test_SizeError");

    UtTest_Add(MM_ReadFileHeaders_Test_ReadHeaderError, MM_Test_Setup, MM_Test_TearDown,
               "MM_ReadFileHeaders_Test_ReadHeaderError");
    UtTest_Add(MM_ReadFileHeaders_Test_ReadError, MM_Test_Setup, MM_Test_TearDown, "MM_ReadFileHeaders_Test_ReadError");

    UtTest_Add(MM_FillMemCmd_Test_RAM, MM_Test_Setup, MM_Test_TearDown, "MM_FillMemCmd_Test_RAM");
    UtTest_Add(MM_FillMemCmd_Test_EEPROM, MM_Test_Setup, MM_Test_TearDown, "MM_FillMemCmd_Test_EEPROM");
    UtTest_Add(MM_FillMemCmd_Test_MEM32, MM_Test_Setup, MM_Test_TearDown, "MM_FillMemCmd_Test_MEM32");
    UtTest_Add(MM_FillMemCmd_Test_MEM16, MM_Test_Setup, MM_Test_TearDown, "MM_FillMemCmd_Test_MEM16");
    UtTest_Add(MM_FillMemCmd_Test_MEM8, MM_Test_Setup, MM_Test_TearDown, "MM_FillMemCmd_Test_MEM8");
    UtTest_Add(MM_FillMemCmd_Test_SymNameError, MM_Test_Setup, MM_Test_TearDown, "MM_FillMemCmd_Test_SymNameError");
    UtTest_Add(MM_FillMemCmd_Test_NoVerifyCmdLength, MM_Test_Setup, MM_Test_TearDown,
               "MM_FillMemCmd_Test_NoVerifyCmdLength");
    UtTest_Add(MM_FillMemCmd_Test_NoLastActionFill, MM_Test_Setup, MM_Test_TearDown,
               "MM_FillMemCmd_Test_NoLastActionFill");
    UtTest_Add(MM_FillMemCmd_Test_NoVerifyLoadDump, MM_Test_Setup, MM_Test_TearDown,
               "MM_FillMemCmd_Test_NoVerifyLoadDump");
    UtTest_Add(MM_FillMemCmd_Test_BadType, MM_Test_Setup, MM_Test_TearDown, "MM_FillMemCmd_Test_BadType");
    UtTest_Add(MM_FillMem_Test_Nominal, MM_Test_Setup, MM_Test_TearDown, "MM_FillMem_Test_Nominal");
    UtTest_Add(MM_FillMem_Test_MaxFillDataSegment, MM_Test_Setup, MM_Test_TearDown,
               "MM_FillMem_Test_MaxFillDataSegment");
}
