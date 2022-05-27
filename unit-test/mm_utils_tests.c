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
 *   Unit tests for mm_utils.c
 */

/************************************************************************
** Includes
*************************************************************************/
#include "mm_utils.h"
#include "mm_msg.h"
#include "mm_msgdefs.h"
#include "mm_msgids.h"
#include "mm_events.h"
#include "mm_filedefs.h"
#include "mm_version.h"
#include "mm_test_utils.h"
#include "mm_dump.h"

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

/* mm_utils_tests globals */
uint8 call_count_CFE_EVS_SendEvent;

/*
 * Function Definitions
 */

void MM_ResetHk_Test(void)
{
    MM_AppData.HkPacket.LastAction     = 1;
    MM_AppData.HkPacket.MemType        = 2;
    MM_AppData.HkPacket.Address        = 3;
    MM_AppData.HkPacket.DataValue      = 4;
    MM_AppData.HkPacket.BytesProcessed = 5;
    MM_AppData.HkPacket.FileName[0]    = 6;

    /* Execute the function being tested */
    MM_ResetHk();

    /* Verify results */
    UtAssert_True(MM_AppData.HkPacket.LastAction == MM_NOACTION, "MM_AppData.HkPacket.LastAction == MM_NOACTION");
    UtAssert_True(MM_AppData.HkPacket.MemType == MM_NOMEMTYPE, "MM_AppData.HkPacket.MemType == MM_NOMEMTYPE");
    UtAssert_True(MM_AppData.HkPacket.Address == MM_CLEAR_ADDR, "MM_AppData.HkPacket.Address == MM_CLEAR_ADDR");
    UtAssert_True(MM_AppData.HkPacket.DataValue == MM_CLEAR_PATTERN,
                  "MM_AppData.HkPacket.DataValue == MM_CLEAR_PATTERN");
    UtAssert_True(MM_AppData.HkPacket.BytesProcessed == 0, "MM_AppData.BytesProcessed == 0");
    UtAssert_True(MM_AppData.HkPacket.FileName[0] == MM_CLEAR_FNAME,
                  "MM_AppData.HkPacket.FileName[0] == MM_CLEAR_FNAME");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_ResetHk_Test */

void MM_VerifyCmdLength_Test_Nominal(void)
{
    bool              Result;
    uint16            ExpectedLength = sizeof(MM_PeekCmd_t);
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;

    TestMsgId = CFE_SB_ValueToMsgId(MM_CMD_MID);
    FcnCode   = MM_PEEK_CC;
    MsgSize   = sizeof(MM_PeekCmd_t);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Execute the function being tested */
    Result = MM_VerifyCmdLength(&UT_CmdBuf.PeekCmd.CmdHeader.Msg, ExpectedLength);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyCmdLength_Test_Nominal */

void MM_VerifyCmdLength_Test_HKRequestLengthError(void)
{
    bool              Result;
    uint16            ExpectedLength = 99;
    CFE_SB_MsgId_t    TestMsgId      = CFE_SB_ValueToMsgId(MM_SEND_HK_MID);
    CFE_MSG_FcnCode_t FcnCode        = 0;
    size_t            MsgSize        = 1;
    int32             strCmpResult;
    char              ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid HK request msg length: ID = 0x%%08lX, CC = %%d, Len = %%d, Expected = %%d");

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Execute the function being tested */
    Result = MM_VerifyCmdLength(&UT_CmdBuf.PeekCmd.CmdHeader.Msg, ExpectedLength);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_HKREQ_LEN_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyCmdLength_Test_HKRequestLengthError */

void MM_VerifyCmdLength_Test_LengthError(void)
{
    bool              Result;
    uint16            ExpectedLength = 99;
    CFE_SB_MsgId_t    TestMsgId      = MM_UT_MID_1;
    CFE_MSG_FcnCode_t FcnCode        = 0;
    size_t            MsgSize        = 1;
    int32             strCmpResult;
    char              ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid msg length: ID = 0x%%08lX, CC = %%d, Len = %%d, Expected = %%d");

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Execute the function being tested */
    Result = MM_VerifyCmdLength(&UT_CmdBuf.PeekCmd.CmdHeader.Msg, ExpectedLength);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_LEN_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyCmdLength_Test_LengthError */

void MM_SegmentBreak_Test_Nominal(void)
{
    /* Execute the function being tested */
    MM_SegmentBreak();

    /* Verify results */

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected o",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_SegmentBreak_Test_Nominal */

void MM_VerifyPeekPokeParams_Test_ByteWidthRAM(void)
{
    bool   Result;
    uint32 Address    = 0;
    uint8  MemType    = MM_RAM;
    uint8  SizeInBits = 8;

    /* Execute the function being tested */
    Result = MM_VerifyPeekPokeParams(Address, MemType, SizeInBits);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyPeekPokeParams_Test_ByteWidthRAM */

void MM_VerifyPeekPokeParams_Test_WordWidthMEM16(void)
{
    bool   Result;
    uint32 Address    = 0;
    uint8  MemType    = MM_MEM16;
    uint8  SizeInBits = 16;

    /* Execute the function being tested */
    Result = MM_VerifyPeekPokeParams(Address, MemType, SizeInBits);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyPeekPokeParams_Test_WordWidthMEM16 */

void MM_VerifyPeekPokeParams_Test_DWordWidthMEM32(void)
{
    bool   Result;
    uint32 Address    = 0;
    uint8  MemType    = MM_MEM32;
    uint8  SizeInBits = 32;

    /* Execute the function being tested */
    Result = MM_VerifyPeekPokeParams(Address, MemType, SizeInBits);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyPeekPokeParams_Test_DWordWidthMEM32 */

void MM_VerifyPeekPokeParams_Test_WordWidthAlignmentError(void)
{
    bool   Result;
    uint32 Address    = 1;
    uint8  MemType    = MM_MEM16;
    uint8  SizeInBits = 16;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Data and address not 16 bit aligned: Addr = %%p Size = %%d");

    /* Execute the function being tested */
    Result = MM_VerifyPeekPokeParams(Address, MemType, SizeInBits);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_ALIGN16_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyPeekPokeParams_Test_WordWidthAlignmentError */

void MM_VerifyPeekPokeParams_Test_DWordWidthAlignmentError(void)
{
    bool   Result;
    uint32 Address    = 1;
    uint8  MemType    = MM_MEM32;
    uint8  SizeInBits = 32;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Data and address not 32 bit aligned: Addr = %%p Size = %%d");

    /* Execute the function being tested */
    Result = MM_VerifyPeekPokeParams(Address, MemType, SizeInBits);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_ALIGN32_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyPeekPokeParams_Test_DWordWidthAlignmentError */

void MM_VerifyPeekPokeParams_Test_InvalidDataSize(void)
{
    bool   Result;
    uint32 Address = 0;
    uint8  MemType = MM_MEM8;
    /* To reach size error: Peeks and Pokes must be 8 bits wide for this memory type */
    uint8 SizeInBits = 16;
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Data size in bits invalid: Data Size = %%d");

    /* Execute the function being tested */
    Result = MM_VerifyPeekPokeParams(Address, MemType, SizeInBits);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_DATA_SIZE_BITS_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyPeekPokeParams_Test_InvalidDataSize */

void MM_VerifyPeekPokeParams_Test_EEPROM(void)
{
    bool   Result;
    uint32 Address    = 0;
    uint8  MemType    = MM_EEPROM;
    uint8  SizeInBits = 8;

    /* Execute the function being tested */
    Result = MM_VerifyPeekPokeParams(Address, MemType, SizeInBits);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyPeekPokeParams_Test_EEPROM */

void MM_VerifyPeekPokeParams_Test_MEM8(void)
{
    bool   Result;
    uint32 Address    = 0;
    uint8  MemType    = MM_MEM8;
    uint8  SizeInBits = 8;

    /* Execute the function being tested */
    Result = MM_VerifyPeekPokeParams(Address, MemType, SizeInBits);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyPeekPokeParams_Test_MEM8 */

void MM_VerifyPeekPokeParams_Test_RAMValidateRangeError(void)
{
    bool   Result;
    uint32 Address    = 0;
    uint8  MemType    = MM_RAM;
    uint8  SizeInBits = 8;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "CFE_PSP_MemValidateRange error received: RC = 0x%%08X Addr = %%p Size = %%d MemType = MEM_RAM");

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, -1);

    /* Execute the function being tested */
    Result = MM_VerifyPeekPokeParams(Address, MemType, SizeInBits);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_OS_MEMVALIDATE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyPeekPokeParams_Test_RAMValidateRangeError */

void MM_VerifyPeekPokeParams_Test_EEPROMValidateRangeError(void)
{
    bool   Result;
    uint32 Address    = 0;
    uint8  MemType    = MM_EEPROM;
    uint8  SizeInBits = 8;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "CFE_PSP_MemValidateRange error received: RC = 0x%%08X Addr = %%p Size = %%d MemType = MEM_EEPROM");

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, -1);

    /* Execute the function being tested */
    Result = MM_VerifyPeekPokeParams(Address, MemType, SizeInBits);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_OS_MEMVALIDATE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyPeekPokeParams_Test_EEPROMValidateRangeError */

void MM_VerifyPeekPokeParams_Test_MEM32ValidateRangeError(void)
{
    bool   Result;
    uint32 Address    = 0;
    uint8  MemType    = MM_MEM32;
    uint8  SizeInBits = 8;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "CFE_PSP_MemValidateRange error received: RC = 0x%%08X Addr = %%p Size = %%d MemType = MEM32");

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, -1);

    /* Execute the function being tested */
    Result = MM_VerifyPeekPokeParams(Address, MemType, SizeInBits);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_OS_MEMVALIDATE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyPeekPokeParams_Test_MEM32ValidateRangeError */

void MM_VerifyPeekPokeParams_Test_MEM16ValidateRangeError(void)
{
    bool   Result;
    uint32 Address    = 0;
    uint8  MemType    = MM_MEM16;
    uint8  SizeInBits = 8;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "CFE_PSP_MemValidateRange error received: RC = 0x%%08X Addr = %%p Size = %%d MemType = MEM16");

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, -1);

    /* Execute the function being tested */
    Result = MM_VerifyPeekPokeParams(Address, MemType, SizeInBits);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_OS_MEMVALIDATE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyPeekPokeParams_Test_MEM16ValidateRangeError */

void MM_VerifyPeekPokeParams_Test_MEM8ValidateRangeError(void)
{
    bool   Result;
    uint32 Address    = 0;
    uint8  MemType    = MM_MEM8;
    uint8  SizeInBits = 8;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "CFE_PSP_MemValidateRange error received: RC = 0x%%08X Addr = %%p Size = %%d MemType = MEM8");

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, -1);

    /* Execute the function being tested */
    Result = MM_VerifyPeekPokeParams(Address, MemType, SizeInBits);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_OS_MEMVALIDATE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyPeekPokeParams_Test_MEM8ValidateRangeError */

void MM_VerifyPeekPokeParams_Test_MEM32InvalidDataSize(void)
{
    bool   Result;
    uint32 Address    = 0;
    uint8  MemType    = MM_MEM32;
    uint8  SizeInBits = 8;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Data size in bits invalid: Data Size = %%d");

    /* Execute the function being tested */
    Result = MM_VerifyPeekPokeParams(Address, MemType, SizeInBits);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_DATA_SIZE_BITS_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyPeekPokeParams_Test_MEM32InvalidDataSize */

void MM_VerifyPeekPokeParams_Test_MEM16InvalidDataSize(void)
{
    bool   Result;
    uint32 Address    = 0;
    uint8  MemType    = MM_MEM16;
    uint8  SizeInBits = 8;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Data size in bits invalid: Data Size = %%d");

    /* Execute the function being tested */
    Result = MM_VerifyPeekPokeParams(Address, MemType, SizeInBits);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_DATA_SIZE_BITS_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyPeekPokeParams_Test_MEM16InvalidDataSize */

void MM_VerifyPeekPokeParams_Test_MEM8InvalidDataSize(void)
{
    bool   Result;
    uint32 Address    = 0;
    uint8  MemType    = MM_MEM8;
    uint8  SizeInBits = 99;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Data size in bits invalid: Data Size = %%d");

    /* Execute the function being tested */
    Result = MM_VerifyPeekPokeParams(Address, MemType, SizeInBits);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_DATA_SIZE_BITS_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyPeekPokeParams_Test_MEM8InvalidDataSize */

void MM_VerifyPeekPokeParams_Test_InvalidMemType(void)
{
    bool   Result;
    uint32 Address    = 0;
    uint8  MemType    = 99;
    uint8  SizeInBits = 8;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Invalid memory type specified: MemType = %%d");

    /* Execute the function being tested */
    Result = MM_VerifyPeekPokeParams(Address, MemType, SizeInBits);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_MEMTYPE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyPeekPokeParams_Test_InvalidMemType */

/*************************************/
/* MM_VerifyLoadDumpParams Tests */
/*************************************/

/* Loading */
void MM_VerifyLoadDumpParams_Test_LoadRAMValidateRangeError(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_RAM;
    uint32 SizeInBytes = 1;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "CFE_PSP_MemValidateRange error received: RC = 0x%%08X Addr = %%p Size = %%d MemType = %%s");

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, -1);

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_LOAD);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_OS_MEMVALIDATE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_LoadRAMValidateRangeError */

void MM_VerifyLoadDumpParams_Test_LoadRAMDataSizeErrorTooSmall(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_RAM;
    uint32 SizeInBytes = 0;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Data size in bytes invalid or exceeds limits: Data Size = %%d");

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_LOAD);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_DATA_SIZE_BYTES_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_LoadRAMDataSizeErrorTooSmall */

void MM_VerifyLoadDumpParams_Test_LoadRAMDataSizeErrorTooLarge(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_RAM;
    uint32 SizeInBytes = MM_MAX_LOAD_FILE_DATA_RAM + 1;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Data size in bytes invalid or exceeds limits: Data Size = %%d");

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_LOAD);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_DATA_SIZE_BYTES_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_LoadRAMDataSizeErrorTooLarge */

void MM_VerifyLoadDumpParams_Test_LoadEEPROMValidateRangeError(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_EEPROM;
    uint32 SizeInBytes = 1;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "CFE_PSP_MemValidateRange error received: RC = 0x%%08X Addr = %%p Size = %%d MemType = %%s");

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, -1);

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_LOAD);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_OS_MEMVALIDATE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_LoadEEPROMValidateRangeError */

void MM_VerifyLoadDumpParams_Test_LoadEEPROMDataSizeErrorTooSmall(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_EEPROM;
    uint32 SizeInBytes = 0;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Data size in bytes invalid or exceeds limits: Data Size = %%d");

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_LOAD);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_DATA_SIZE_BYTES_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_LoadEEPROMDataSizeErrorTooSmall */

void MM_VerifyLoadDumpParams_Test_LoadEEPROMDataSizeErrorTooLarge(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_EEPROM;
    uint32 SizeInBytes = MM_MAX_LOAD_FILE_DATA_EEPROM + 1;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Data size in bytes invalid or exceeds limits: Data Size = %%d");

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_LOAD);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_DATA_SIZE_BYTES_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_LoadEEPROMDataSizeErrorTooLarge */

void MM_VerifyLoadDumpParams_Test_LoadMEM32ValidateRangeError(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_MEM32;
    uint32 SizeInBytes = 4;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "CFE_PSP_MemValidateRange error received: RC = 0x%%08X Addr = %%p Size = %%d MemType = %%s");

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, -1);

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_LOAD);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_OS_MEMVALIDATE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_LoadMEM32ValidateRangeError */

void MM_VerifyLoadDumpParams_Test_LoadMEM32DataSizeErrorTooSmall(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_MEM32;
    uint32 SizeInBytes = 0;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Data size in bytes invalid or exceeds limits: Data Size = %%d");

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_LOAD);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_DATA_SIZE_BYTES_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_LoadMEM32DataSizeErrorTooSmall */

void MM_VerifyLoadDumpParams_Test_LoadMEM32DataSizeErrorTooLarge(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_MEM32;
    uint32 SizeInBytes = MM_MAX_LOAD_FILE_DATA_MEM32 + 4;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Data size in bytes invalid or exceeds limits: Data Size = %%d");

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_LOAD);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_DATA_SIZE_BYTES_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_LoadMEM32DataSizeErrorTooLarge */

void MM_VerifyLoadDumpParams_Test_LoadMEM32AlignmentError(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_MEM32;
    uint32 SizeInBytes = 1;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Data and address not 32 bit aligned: Addr = %%p Size = %%d");

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_LOAD);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_ALIGN32_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_LoadMEM32AlignmentError */

void MM_VerifyLoadDumpParams_Test_LoadMEM16ValidateRangeError(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_MEM16;
    uint32 SizeInBytes = 2;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "CFE_PSP_MemValidateRange error received: RC = 0x%%08X Addr = %%p Size = %%d MemType = %%s");

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, -1);

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_LOAD);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_OS_MEMVALIDATE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_MEM16ValidateRangeError */

void MM_VerifyLoadDumpParams_Test_LoadMEM16DataSizeErrorTooSmall(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_MEM16;
    uint32 SizeInBytes = 0;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Data size in bytes invalid or exceeds limits: Data Size = %%d");

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_LOAD);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_DATA_SIZE_BYTES_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_LoadMEM16DataSizeErrorTooSmall */

void MM_VerifyLoadDumpParams_Test_LoadMEM16DataSizeErrorTooLarge(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_MEM16;
    uint32 SizeInBytes = MM_MAX_LOAD_FILE_DATA_MEM16 + 2;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Data size in bytes invalid or exceeds limits: Data Size = %%d");

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_LOAD);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_DATA_SIZE_BYTES_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_LoadMEM16DataSizeErrorTooLarge */

void MM_VerifyLoadDumpParams_Test_LoadMEM16AlignmentError(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_MEM16;
    uint32 SizeInBytes = 1;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Data and address not 16 bit aligned: Addr = %%p Size = %%d");

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_LOAD);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_ALIGN16_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_LoadMEM16AlignmentError */

void MM_VerifyLoadDumpParams_Test_LoadMEM8ValidateRangeError(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_MEM8;
    uint32 SizeInBytes = 1;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "CFE_PSP_MemValidateRange error received: RC = 0x%%08X Addr = %%p Size = %%d MemType = %%s");

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, -1);

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_LOAD);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_OS_MEMVALIDATE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_LoadMEM8ValidateRangeError */

void MM_VerifyLoadDumpParams_Test_LoadMEM8DataSizeErrorTooSmall(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_MEM8;
    uint32 SizeInBytes = 0;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Data size in bytes invalid or exceeds limits: Data Size = %%d");

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_LOAD);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_DATA_SIZE_BYTES_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_LoadMEM8DataSizeErrorTooSmall */

void MM_VerifyLoadDumpParams_Test_LoadMEM8DataSizeErrorTooLarge(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_MEM8;
    uint32 SizeInBytes = MM_MAX_LOAD_FILE_DATA_MEM8 + 1;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Data size in bytes invalid or exceeds limits: Data Size = %%d");

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_LOAD);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_DATA_SIZE_BYTES_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_LoadMEM8DataSizeErrorTooLarge */

void MM_VerifyLoadDumpParams_Test_LoadInvalidMemTypeError(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = 99;
    uint32 SizeInBytes = MM_MAX_LOAD_FILE_DATA_MEM8 + 1;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Invalid memory type specified: MemType = %%d");

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_LOAD);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_MEMTYPE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_LoadInvalidMemTypeError */

void MM_VerifyLoadDumpParams_Test_LoadInvalidVerifyTypeError(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_RAM;
    uint32 SizeInBytes = MM_MAX_LOAD_FILE_DATA_MEM8;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, -1);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_LoadInvalidVerifyTypeError */

/* Dumping */
void MM_VerifyLoadDumpParams_Test_DumpRAM(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_RAM;
    uint32 SizeInBytes = 1;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_DUMP);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_DumpRAM */

void MM_VerifyLoadDumpParams_Test_DumpEEPROM(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_EEPROM;
    uint32 SizeInBytes = 1;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_DUMP);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_DumpEEPROM */

void MM_VerifyLoadDumpParams_Test_DumpMEM32(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_MEM32;
    uint32 SizeInBytes = 4;

    UT_SetDefaultReturnValue(UT_KEY(CFE_PSP_MemValidateRange), CFE_PSP_SUCCESS);

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_DUMP);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_DumpMEM32 */

void MM_VerifyLoadDumpParams_Test_DumpMEM16(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_MEM16;
    uint32 SizeInBytes = 2;

    UT_SetDefaultReturnValue(UT_KEY(CFE_PSP_MemValidateRange), CFE_PSP_SUCCESS);

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_DUMP);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_DumpMEM16 */

void MM_VerifyLoadDumpParams_Test_DumpMEM8(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_MEM8;
    uint32 SizeInBytes = 1;

    UT_SetDefaultReturnValue(UT_KEY(CFE_PSP_MemValidateRange), CFE_PSP_SUCCESS);

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_DUMP);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_DumpMEM8 */

void MM_VerifyLoadDumpParams_Test_DumpRAMRangeError(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_RAM;
    uint32 SizeInBytes = 1;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "CFE_PSP_MemValidateRange error received: RC = 0x%%08X Addr = %%p Size = %%d MemType = %%s");

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, -1);

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_DUMP);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_OS_MEMVALIDATE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_DumpRAMRangeError */

void MM_VerifyLoadDumpParams_Test_DumpRAMInvalidSizeTooSmall(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_RAM;
    uint32 SizeInBytes = 0;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Data size in bytes invalid or exceeds limits: Data Size = %%d");

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_DUMP);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_DATA_SIZE_BYTES_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_DumpRAMInvalidSizeTooSmall */

void MM_VerifyLoadDumpParams_Test_DumpRAMInvalidSizeTooLarge(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_RAM;
    uint32 SizeInBytes = MM_MAX_DUMP_FILE_DATA_RAM + 1;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Data size in bytes invalid or exceeds limits: Data Size = %%d");

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_DUMP);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_DATA_SIZE_BYTES_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_DumpRAMInvalidSizeTooLarge */

void MM_VerifyLoadDumpParams_Test_DumpEEPROMRangeError(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_EEPROM;
    uint32 SizeInBytes = 1;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "CFE_PSP_MemValidateRange error received: RC = 0x%%08X Addr = %%p Size = %%d MemType = %%s");

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, -1);

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_DUMP);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_OS_MEMVALIDATE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_DumpEEPROMRangeError */

void MM_VerifyLoadDumpParams_Test_DumpEEPROMInvalidSizeTooSmall(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_EEPROM;
    uint32 SizeInBytes = 0;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Data size in bytes invalid or exceeds limits: Data Size = %%d");

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_DUMP);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_DATA_SIZE_BYTES_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_DumpEEPROMInvalidSizeTooSmall */

void MM_VerifyLoadDumpParams_Test_DumpEEPROMInvalidSizeTooLarge(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_EEPROM;
    uint32 SizeInBytes = MM_MAX_DUMP_FILE_DATA_EEPROM + 1;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Data size in bytes invalid or exceeds limits: Data Size = %%d");

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_DUMP);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_DATA_SIZE_BYTES_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_DumpEEPROMInvalidSizeTooLarge */

void MM_VerifyLoadDumpParams_Test_DumpMEM32RangeError(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_MEM32;
    uint32 SizeInBytes = 4;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "CFE_PSP_MemValidateRange error received: RC = 0x%%08X Addr = %%p Size = %%d MemType = %%s");

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, -1);

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_DUMP);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_OS_MEMVALIDATE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_DumpMEM32RangeError */

void MM_VerifyLoadDumpParams_Test_DumpMEM32InvalidSizeTooSmall(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_MEM32;
    uint32 SizeInBytes = 0;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Data size in bytes invalid or exceeds limits: Data Size = %%d");

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_DUMP);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_DATA_SIZE_BYTES_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_DumpMEM32InvalidSizeTooSmall */

void MM_VerifyLoadDumpParams_Test_DumpMEM32InvalidSizeTooLarge(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_MEM32;
    uint32 SizeInBytes = MM_MAX_DUMP_FILE_DATA_MEM32 + 4;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Data size in bytes invalid or exceeds limits: Data Size = %%d");

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_DUMP);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_DATA_SIZE_BYTES_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_DumpMEM32InvalidSizeTooLarge */

void MM_VerifyLoadDumpParams_Test_DumpMEM32AlignmentError(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_MEM32;
    uint32 SizeInBytes = 3;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Data and address not 32 bit aligned: Addr = %%p Size = %%d");

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_DUMP);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_ALIGN32_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_DumpMEM32AlignmentError */

void MM_VerifyLoadDumpParams_Test_DumpMEM16RangeError(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_MEM16;
    uint32 SizeInBytes = 2;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "CFE_PSP_MemValidateRange error received: RC = 0x%%08X Addr = %%p Size = %%d MemType = %%s");

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, -1);

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_DUMP);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_OS_MEMVALIDATE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_DumpMEM16RangeError */

void MM_VerifyLoadDumpParams_Test_DumpMEM16InvalidSizeTooSmall(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_MEM16;
    uint32 SizeInBytes = 0;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Data size in bytes invalid or exceeds limits: Data Size = %%d");

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_DUMP);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_DATA_SIZE_BYTES_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_DumpMEM16InvalidSizeTooSmall */

void MM_VerifyLoadDumpParams_Test_DumpMEM16InvalidSizeTooLarge(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_MEM16;
    uint32 SizeInBytes = MM_MAX_DUMP_FILE_DATA_MEM16 + 2;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Data size in bytes invalid or exceeds limits: Data Size = %%d");

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_DUMP);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_DATA_SIZE_BYTES_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_DumpMEM16InvalidSizeTooLarge */

void MM_VerifyLoadDumpParams_Test_DumpMEM16AlignmentError(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_MEM16;
    uint32 SizeInBytes = 3;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Data and address not 16 bit aligned: Addr = %%p Size = %%d");

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_DUMP);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_ALIGN16_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_DumpMEM16AlignmentError */

void MM_VerifyLoadDumpParams_Test_DumpMEM8RangeError(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_MEM8;
    uint32 SizeInBytes = 1;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "CFE_PSP_MemValidateRange error received: RC = 0x%%08X Addr = %%p Size = %%d MemType = %%s");

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, -1);

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_DUMP);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_OS_MEMVALIDATE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_DumpMEM8RangeError */

void MM_VerifyLoadDumpParams_Test_DumpMEM8InvalidSizeTooSmall(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_MEM8;
    uint32 SizeInBytes = 0;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Data size in bytes invalid or exceeds limits: Data Size = %%d");

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_DUMP);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_DATA_SIZE_BYTES_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_DumpMEM8InvalidSizeTooSmall */

void MM_VerifyLoadDumpParams_Test_DumpMEM8InvalidSizeTooLarge(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_MEM8;
    uint32 SizeInBytes = MM_MAX_DUMP_FILE_DATA_MEM8 + 1;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Data size in bytes invalid or exceeds limits: Data Size = %%d");

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_DUMP);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_DATA_SIZE_BYTES_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_DumpMEM8InvalidSizeTooLarge */

void MM_VerifyLoadDumpParams_Test_DumpInvalidMemoryType(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = 99;
    uint32 SizeInBytes = 1;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Invalid memory type specified: MemType = %%d");

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, -1);

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_DUMP);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_MEMTYPE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_DumpInvalidMemoryType */

/* Dump in event */

void MM_VerifyLoadDumpParams_Test_DumpEventRAM(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_RAM;
    uint32 SizeInBytes = 1;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_EVENT);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_DumpEventRAM */

void MM_VerifyLoadDumpParams_Test_DumpEventEEPROM(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_EEPROM;
    uint32 SizeInBytes = 1;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_EVENT);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_DumpEventEEPROM */

void MM_VerifyLoadDumpParams_Test_DumpEventMEM32(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_MEM32;
    uint32 SizeInBytes = 4;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_EVENT);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_DumpEventMEM32 */

void MM_VerifyLoadDumpParams_Test_DumpEventMEM16(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_MEM16;
    uint32 SizeInBytes = 2;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_EVENT);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_DumpEventMEM16 */

void MM_VerifyLoadDumpParams_Test_DumpEventMEM8(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_MEM8;
    uint32 SizeInBytes = 1;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_EVENT);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_DumpEventMEM8 */

void MM_VerifyLoadDumpParams_Test_DumpEventInvalidDataSizeTooSmall(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_MEM8;
    uint32 SizeInBytes = 0;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Data size in bytes invalid or exceeds limits: Data Size = %%d");

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_EVENT);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_DATA_SIZE_BYTES_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_DumpEventInvalidDataSizeTooSmall */

void MM_VerifyLoadDumpParams_Test_DumpEventInvalidDataSizeTooLarge(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_MEM8;
    uint32 SizeInBytes = MM_MAX_DUMP_INEVENT_BYTES + 1;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Data size in bytes invalid or exceeds limits: Data Size = %%d");

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_EVENT);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_DATA_SIZE_BYTES_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_DumpEventInvalidDataSizeTooLarge */

void MM_VerifyLoadDumpParams_Test_DumpEventRAMRangeError(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_RAM;
    uint32 SizeInBytes = 1;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "CFE_PSP_MemValidateRange error received: RC = 0x%%08X Addr = %%p Size = %%d MemType = %%s");

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, -1);

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_EVENT);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_OS_MEMVALIDATE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_DumpEventRAMRangeError */

void MM_VerifyLoadDumpParams_Test_DumpEventEEPROMRangeError(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_EEPROM;
    uint32 SizeInBytes = 1;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "CFE_PSP_MemValidateRange error received: RC = 0x%%08X Addr = %%p Size = %%d MemType = %%s");

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, -1);

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_EVENT);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_OS_MEMVALIDATE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_DumpEventEEPROMRangeError */

void MM_VerifyLoadDumpParams_Test_DumpEventMEM32RangeError(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_MEM32;
    uint32 SizeInBytes = 4;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "CFE_PSP_MemValidateRange error received: RC = 0x%%08X Addr = %%p Size = %%d MemType = %%s");

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, -1);

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_EVENT);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_OS_MEMVALIDATE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_DumpEventMEM32RangeError */

void MM_VerifyLoadDumpParams_Test_DumpEventMEM32AlignmentError(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_MEM32;
    uint32 SizeInBytes = 3;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Data and address not 32 bit aligned: Addr = %%p Size = %%d");

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_EVENT);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_ALIGN32_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_DumpEventMEM32AlignmentError */

void MM_VerifyLoadDumpParams_Test_DumpEventMEM16RangeError(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_MEM16;
    uint32 SizeInBytes = 2;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "CFE_PSP_MemValidateRange error received: RC = 0x%%08X Addr = %%p Size = %%d MemType = %%s");

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, -1);

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_EVENT);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_OS_MEMVALIDATE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_DumpEventMEM16RangeError */

void MM_VerifyLoadDumpParams_Test_DumpEventMEM16AlignmentError(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_MEM16;
    uint32 SizeInBytes = 3;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Data and address not 16 bit aligned: Addr = %%p Size = %%d");

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_EVENT);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_ALIGN16_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_DumpEventMEM16AlignmentError */

void MM_VerifyLoadDumpParams_Test_DumpEventMEM8RangeError(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_MEM8;
    uint32 SizeInBytes = 1;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "CFE_PSP_MemValidateRange error received: RC = 0x%%08X Addr = %%p Size = %%d MemType = %%s");

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, -1);

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_EVENT);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_OS_MEMVALIDATE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_DumpEventMEM32RangeError */

void MM_VerifyLoadDumpParams_Test_DumpEventInvalidMemType(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = 99;
    uint32 SizeInBytes = 1;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Invalid memory type specified: MemType = %%d");

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, -1);

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_EVENT);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_MEMTYPE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* no command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_DumpEventInvalidMemType */

/* Fill */

void MM_VerifyLoadDumpParams_Test_FillRAMValidateRangeError(void)
{
    bool   Result;
    uint32 Address     = 1;
    uint8  MemType     = MM_RAM;
    uint32 SizeInBytes = 1;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "CFE_PSP_MemValidateRange error received: RC = 0x%%08X Addr = %%p Size = %%d MemType = %%s");

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, -1);

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_FILL);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_OS_MEMVALIDATE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_FillRAMValidateRangeError */

void MM_VerifyLoadDumpParams_Test_FillRAMDataSizeErrorTooSmall(void)
{
    bool   Result;
    uint32 Address     = 1;
    uint8  MemType     = MM_RAM;
    uint32 SizeInBytes = 0;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Data size in bytes invalid or exceeds limits: Data Size = %%d");

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_FILL);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_DATA_SIZE_BYTES_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_FillRAMDataSizeErrorTooSmall */

void MM_VerifyLoadDumpParams_Test_FillRAMDataSizeErrorTooLarge(void)
{
    bool   Result;
    uint32 Address     = 1;
    uint8  MemType     = MM_RAM;
    uint32 SizeInBytes = MM_MAX_FILL_DATA_RAM + 1;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Data size in bytes invalid or exceeds limits: Data Size = %%d");

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_FILL);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_DATA_SIZE_BYTES_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_FillRAMDataSizeErrorTooLarge */

void MM_VerifyLoadDumpParams_Test_FillEEPROMValidateRangeError(void)
{
    bool   Result;
    uint32 Address     = 1;
    uint8  MemType     = MM_EEPROM;
    uint32 SizeInBytes = 1;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "CFE_PSP_MemValidateRange error received: RC = 0x%%08X Addr = %%p Size = %%d MemType = %%s");

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, -1);

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_FILL);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_OS_MEMVALIDATE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_FillEEPROMValidateRangeError */

void MM_VerifyLoadDumpParams_Test_FillEEPROMDataSizeErrorTooSmall(void)
{
    bool   Result;
    uint32 Address     = 1;
    uint8  MemType     = MM_EEPROM;
    uint32 SizeInBytes = 0;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Data size in bytes invalid or exceeds limits: Data Size = %%d");

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_FILL);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_DATA_SIZE_BYTES_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_FillEEPROMDataSizeErrorTooSmall */

void MM_VerifyLoadDumpParams_Test_FillEEPROMDataSizeErrorTooLarge(void)
{
    bool   Result;
    uint32 Address     = 1;
    uint8  MemType     = MM_EEPROM;
    uint32 SizeInBytes = MM_MAX_FILL_DATA_EEPROM + 1;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Data size in bytes invalid or exceeds limits: Data Size = %%d");

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_FILL);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_DATA_SIZE_BYTES_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_FillEEPROMDataSizeErrorTooLarge */

void MM_VerifyLoadDumpParams_Test_FillMEM32ValidateRangeError(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_MEM32;
    uint32 SizeInBytes = 4;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "CFE_PSP_MemValidateRange error received: RC = 0x%%08X Addr = %%p Size = %%d MemType = %%s");

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, -1);

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_FILL);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_OS_MEMVALIDATE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_FillMEM32ValidateRangeError */

void MM_VerifyLoadDumpParams_Test_FillMEM32DataSizeErrorTooSmall(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_MEM32;
    uint32 SizeInBytes = 0;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Data size in bytes invalid or exceeds limits: Data Size = %%d");

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_FILL);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_DATA_SIZE_BYTES_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_FillMEM32DataSizeErrorTooSmall */

void MM_VerifyLoadDumpParams_Test_FillMEM32DataSizeErrorTooLarge(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_MEM32;
    uint32 SizeInBytes = MM_MAX_FILL_DATA_MEM32 + 4;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Data size in bytes invalid or exceeds limits: Data Size = %%d");

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_FILL);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_DATA_SIZE_BYTES_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_FillMEM32DataSizeErrorTooLarge */

void MM_VerifyLoadDumpParams_Test_FillMEM32AlignmentError(void)
{
    bool   Result;
    uint32 Address     = 1;
    uint8  MemType     = MM_MEM32;
    uint32 SizeInBytes = 1;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Data and address not 32 bit aligned: Addr = %%p Size = %%d");

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_FILL);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_ALIGN32_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_FillMEM32AlignmentError */

void MM_VerifyLoadDumpParams_Test_FillMEM16ValidateRangeError(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_MEM16;
    uint32 SizeInBytes = 2;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "CFE_PSP_MemValidateRange error received: RC = 0x%%08X Addr = %%p Size = %%d MemType = %%s");

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, -1);

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_FILL);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_OS_MEMVALIDATE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_FillMEM16ValidateRangeError */

void MM_VerifyLoadDumpParams_Test_FillMEM16DataSizeErrorTooSmall(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_MEM16;
    uint32 SizeInBytes = 0;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Data size in bytes invalid or exceeds limits: Data Size = %%d");

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_FILL);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_DATA_SIZE_BYTES_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_FillMEM16DataSizeErrorTooSmall */

void MM_VerifyLoadDumpParams_Test_FillMEM16DataSizeErrorTooLarge(void)
{
    bool   Result;
    uint32 Address     = 0;
    uint8  MemType     = MM_MEM16;
    uint32 SizeInBytes = MM_MAX_FILL_DATA_MEM16 + 2;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Data size in bytes invalid or exceeds limits: Data Size = %%d");

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_FILL);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_DATA_SIZE_BYTES_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_FillMEM16DataSizeErrorTooLarge */

void MM_VerifyLoadDumpParams_Test_FillMEM16AlignmentError(void)
{
    bool   Result;
    uint32 Address     = 1;
    uint8  MemType     = MM_MEM16;
    uint32 SizeInBytes = 1;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Data and address not 16 bit aligned: Addr = %%p Size = %%d");

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_FILL);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_ALIGN16_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_FillMEM16AlignmentError */

void MM_VerifyLoadDumpParams_Test_FillMEM8ValidateRangeError(void)
{
    bool   Result;
    uint32 Address     = 1;
    uint8  MemType     = MM_MEM8;
    uint32 SizeInBytes = 1;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "CFE_PSP_MemValidateRange error received: RC = 0x%%08X Addr = %%p Size = %%d MemType = %%s");

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, -1);

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_FILL);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_OS_MEMVALIDATE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_FillMEM8ValidateRangeError */

void MM_VerifyLoadDumpParams_Test_FillMEM8DataSizeErrorTooSmall(void)
{
    bool   Result;
    uint32 Address     = 1;
    uint8  MemType     = MM_MEM8;
    uint32 SizeInBytes = 0;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Data size in bytes invalid or exceeds limits: Data Size = %%d");

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_FILL);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_DATA_SIZE_BYTES_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_FillMEM8DataSizeErrorTooSmall */

void MM_VerifyLoadDumpParams_Test_FillMEM8DataSizeErrorTooLarge(void)
{
    bool   Result;
    uint32 Address     = 1;
    uint8  MemType     = MM_MEM8;
    uint32 SizeInBytes = MM_MAX_FILL_DATA_MEM8 + 1;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Data size in bytes invalid or exceeds limits: Data Size = %%d");

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_FILL);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_DATA_SIZE_BYTES_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_FillMEM8DataSizeErrorTooLarge */

void MM_VerifyLoadDumpParams_Test_FillInvalidMemTypeError(void)
{
    bool   Result;
    uint32 Address     = 1;
    uint8  MemType     = 99;
    uint32 SizeInBytes = 1;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Invalid memory type specified: MemType = %%d");

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_FILL);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_MEMTYPE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_FillMEM8InvalidMemTypeError */

/* WID */
void MM_VerifyLoadDumpParams_Test_WIDNominal(void)
{
    uint32 Address     = 0;
    uint32 SizeInBytes = 1;
    bool   Result;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MM_RAM, SizeInBytes, MM_VERIFY_WID);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_WIDNominal */

void MM_VerifyLoadDumpParams_Test_WIDMemValidateError(void)
{
    uint32 Address     = 0;
    uint32 SizeInBytes = 1;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool   Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "CFE_PSP_MemValidateRange error received: RC = 0x%%08X Addr = %%p Size = %%d MemType = %%s");

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, -1);

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MM_RAM, SizeInBytes, MM_VERIFY_WID);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_OS_MEMVALIDATE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_WIDMemValidateError */

void MM_VerifyLoadDumpParams_Test_WIDDataSizeErrorTooSmall(void)
{
    uint32 Address     = 0;
    uint32 SizeInBytes = 0;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool   Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Data size in bytes invalid or exceeds limits: Data Size = %%d");

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MM_RAM, SizeInBytes, MM_VERIFY_WID);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_DATA_SIZE_BYTES_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_WIDDataSizeErrorTooSmall */

void MM_VerifyLoadDumpParams_Test_WIDDataSizeErrorTooLarge(void)
{
    uint32 Address     = 0;
    uint32 SizeInBytes = MM_MAX_UNINTERRUPTIBLE_DATA + 1;
    ;
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool  Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Data size in bytes invalid or exceeds limits: Data Size = %%d");

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MM_RAM, SizeInBytes, MM_VERIFY_WID);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_DATA_SIZE_BYTES_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);

} /* end MM_VerifyLoadDumpParams_Test_WIDDataSizeErrorTooLarge */

void MM_Verify32Aligned_Test(void)
{
    bool    Result;
    cpuaddr Addr;
    uint32  Size;

    Addr = 0; /* address is aligned */
    Size = 4; /* size is aligned */

    /* Execute the function being tested */
    Result = MM_Verify32Aligned(Addr, Size);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    Addr = 0; /* address is aligned */
    Size = 1; /* size is not aligned */

    /* Execute the function being tested */
    Result = MM_Verify32Aligned(Addr, Size);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    Addr = 1; /* address is not aligned */
    Size = 0; /* size is aligned */

    /* Execute the function being tested */
    Result = MM_Verify32Aligned(Addr, Size);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

} /* end MM_Verify32Aligned_Test */

void MM_Verify16Aligned_Test(void)
{
    bool    Result;
    cpuaddr Addr;
    uint32  Size;

    Addr = 0; /* address is aligned */
    Size = 4; /* size is aligned */

    /* Execute the function being tested */
    Result = MM_Verify16Aligned(Addr, Size);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    Addr = 0; /* address is aligned */
    Size = 1; /* size is not aligned */

    /* Execute the function being tested */
    Result = MM_Verify16Aligned(Addr, Size);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    Addr = 1; /* address is not aligned */
    Size = 0; /* size is aligned */

    /* Execute the function being tested */
    Result = MM_Verify16Aligned(Addr, Size);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

} /* end MM_Verify16Aligned_Test */

void MM_ResolveSymAddr_Test(void)
{
    MM_SymAddr_t SymAddr;
    cpuaddr      ResolvedAddr = 0;
    bool         Result;

    memset(&SymAddr, 0, sizeof(MM_SymAddr_t));

    SymAddr.Offset = 99;

    /* Execute the function being tested */
    Result = MM_ResolveSymAddr(&SymAddr, &ResolvedAddr);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");
    UtAssert_True(ResolvedAddr == SymAddr.Offset, "ResolvedAddr == SymAddr.Offset");

    ResolvedAddr = 0;
    strncpy(SymAddr.SymName, "symname", OS_MAX_PATH_LEN);

    UT_SetDataBuffer(UT_KEY(OS_SymbolLookup), &ResolvedAddr, sizeof(ResolvedAddr), false);
    UT_SetDefaultReturnValue(UT_KEY(OS_SymbolLookup), OS_SUCCESS);

    /* Execute the function being tested */
    Result = MM_ResolveSymAddr(&SymAddr, &ResolvedAddr);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");
    UtAssert_True(ResolvedAddr == SymAddr.Offset, "ResolvedAddr == SymAddr.Offset");

    UT_SetDefaultReturnValue(UT_KEY(OS_SymbolLookup), -1);

    /* Execute the function being tested */
    Result = MM_ResolveSymAddr(&SymAddr, &ResolvedAddr);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");
}

void MM_ComputeCRCFromFile_Test(void)
{
    osal_id_t FileHandle;
    uint32    CrcPtr;
    uint32    TypeCRC;
    int32     Result;

    FileHandle = MM_UT_OBJID_1;
    CrcPtr     = 0;
    TypeCRC    = 0;

    UT_SetDataBuffer(UT_KEY(OS_read), &FileHandle, sizeof(FileHandle), false);
    UT_SetDeferredRetcode(UT_KEY(OS_read), 1, 1);
    UT_SetDeferredRetcode(UT_KEY(OS_read), 1, 0);

    UT_SetDefaultReturnValue(UT_KEY(CFE_ES_CalculateCRC), 1);

    /* Execute the function being tested */
    Result = MM_ComputeCRCFromFile(FileHandle, &CrcPtr, TypeCRC);

    /* Verify results */
    UtAssert_True(Result == OS_SUCCESS, "Result == OS_SUCCESS");
    UtAssert_True(CrcPtr == 1, "CrcPtr == 1");

    CrcPtr = 0;

    UT_SetDataBuffer(UT_KEY(OS_read), &FileHandle, sizeof(FileHandle), false);
    UT_SetDeferredRetcode(UT_KEY(OS_read), 1, -1);

    UT_SetDefaultReturnValue(UT_KEY(CFE_ES_CalculateCRC), 1);

    /* Execute the function being tested */
    Result = MM_ComputeCRCFromFile(FileHandle, &CrcPtr, TypeCRC);

    /* Verify results */
    UtAssert_True(Result == -1, "Result == -1");
}

/*
 * Register the test cases to execute with the unit test tool
 */
void UtTest_Setup(void)
{
    UtTest_Add(MM_ResetHk_Test, MM_Test_Setup, MM_Test_TearDown, "MM_ResetHk_Test");
    UtTest_Add(MM_VerifyCmdLength_Test_Nominal, MM_Test_Setup, MM_Test_TearDown, "MM_VerifyCmdLength_Test_Nominal");
    UtTest_Add(MM_VerifyCmdLength_Test_HKRequestLengthError, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyCmdLength_Test_HKRequestLengthError");
    UtTest_Add(MM_VerifyCmdLength_Test_LengthError, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyCmdLength_Test_LengthError");

    UtTest_Add(MM_SegmentBreak_Test_Nominal, MM_Test_Setup, MM_Test_TearDown, "MM_SegmentBreak_Test_Nominal");

    UtTest_Add(MM_VerifyPeekPokeParams_Test_ByteWidthRAM, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyPeekPokeParams_Test_ByteWidthRAM");
    UtTest_Add(MM_VerifyPeekPokeParams_Test_WordWidthMEM16, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyPeekPokeParams_Test_WordWidthMEM16");
    UtTest_Add(MM_VerifyPeekPokeParams_Test_DWordWidthMEM32, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyPeekPokeParams_Test_DWordWidthMEM32");
    UtTest_Add(MM_VerifyPeekPokeParams_Test_WordWidthAlignmentError, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyPeekPokeParams_Test_WordWidthAlignmentError");
    UtTest_Add(MM_VerifyPeekPokeParams_Test_DWordWidthAlignmentError, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyPeekPokeParams_Test_DWordWidthAlignmentError");
    UtTest_Add(MM_VerifyPeekPokeParams_Test_InvalidDataSize, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyPeekPokeParams_Test_InvalidDataSize");
    UtTest_Add(MM_VerifyPeekPokeParams_Test_EEPROM, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyPeekPokeParams_Test_EEPROM");
    UtTest_Add(MM_VerifyPeekPokeParams_Test_MEM8, MM_Test_Setup, MM_Test_TearDown, "MM_VerifyPeekPokeParams_Test_MEM8");
    UtTest_Add(MM_VerifyPeekPokeParams_Test_RAMValidateRangeError, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyPeekPokeParams_Test_RAMValidateRangeError");
    UtTest_Add(MM_VerifyPeekPokeParams_Test_EEPROMValidateRangeError, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyPeekPokeParams_Test_EEPROMValidateRangeError");
    UtTest_Add(MM_VerifyPeekPokeParams_Test_MEM32ValidateRangeError, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyPeekPokeParams_Test_MEM32ValidateRangeError");
    UtTest_Add(MM_VerifyPeekPokeParams_Test_MEM32InvalidDataSize, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyPeekPokeParams_Test_MEM32InvalidDataSize");
    UtTest_Add(MM_VerifyPeekPokeParams_Test_MEM16ValidateRangeError, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyPeekPokeParams_Test_MEM16ValidateRangeError");
    UtTest_Add(MM_VerifyPeekPokeParams_Test_MEM16InvalidDataSize, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyPeekPokeParams_Test_MEM16InvalidDataSize");
    UtTest_Add(MM_VerifyPeekPokeParams_Test_MEM8ValidateRangeError, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyPeekPokeParams_Test_MEM8ValidateRangeError");
    UtTest_Add(MM_VerifyPeekPokeParams_Test_MEM8InvalidDataSize, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyPeekPokeParams_Test_MEM8InvalidDataSize");
    UtTest_Add(MM_VerifyPeekPokeParams_Test_InvalidMemType, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyPeekPokeParams_Test_InvalidMemType");

    /* MM_VerifyLoadDumpParams Tests */

    /* Loading */
    UtTest_Add(MM_VerifyLoadDumpParams_Test_LoadRAMValidateRangeError, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_LoadRAMValidateRangeError");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_LoadRAMDataSizeErrorTooSmall, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_LoadRAMDataSizeErrorTooSmall");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_LoadRAMDataSizeErrorTooLarge, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_LoadRAMDataSizeErrorTooLarge");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_LoadEEPROMValidateRangeError, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_LoadEEPROMValidateRangeError");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_LoadEEPROMDataSizeErrorTooSmall, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_LoadEEPROMDataSizeErrorTooSmall");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_LoadEEPROMDataSizeErrorTooLarge, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_LoadEEPROMDataSizeErrorTooLarge");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_LoadMEM32ValidateRangeError, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_LoadMEM32ValidateRangeError");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_LoadMEM32DataSizeErrorTooSmall, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_LoadMEM32DataSizeErrorTooSmall");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_LoadMEM32DataSizeErrorTooLarge, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_LoadMEM32DataSizeErrorTooLarge");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_LoadMEM32AlignmentError, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_LoadMEM32AlignmentError");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_LoadMEM16ValidateRangeError, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_LoadMEM16ValidateRangeError");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_LoadMEM16DataSizeErrorTooSmall, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_LoadMEM16DataSizeErrorTooSmall");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_LoadMEM16DataSizeErrorTooLarge, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_LoadMEM16DataSizeErrorTooLarge");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_LoadMEM16AlignmentError, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_LoadMEM16AlignmentError");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_LoadMEM8ValidateRangeError, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_LoadMEM8ValidateRangeError");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_LoadMEM8DataSizeErrorTooSmall, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_LoadMEM8DataSizeErrorTooSmall");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_LoadMEM8DataSizeErrorTooLarge, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_LoadMEM8DataSizeErrorTooLarge");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_LoadInvalidMemTypeError, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_LoadInvalidMemTypeError");

    /* Dumping */
    UtTest_Add(MM_VerifyLoadDumpParams_Test_DumpRAM, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_DumpRAM");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_DumpEEPROM, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_DumpEEPROM");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_DumpMEM32, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_DumpMEM32");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_DumpMEM16, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_DumpMEM16");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_DumpMEM8, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_DumpMEM8");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_DumpRAMRangeError, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_DumpRAMRangeError");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_DumpRAMInvalidSizeTooSmall, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_DumpRAMInvalidSizeTooSmall");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_DumpRAMInvalidSizeTooLarge, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_DumpRAMInvalidSizeTooLarge");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_DumpEEPROMRangeError, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_DumpEEPROMRangeError");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_DumpEEPROMInvalidSizeTooSmall, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_DumpEEPROMInvalidSizeTooSmall");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_DumpEEPROMInvalidSizeTooLarge, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_DumpEEPROMInvalidSizeTooLarge");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_DumpMEM32RangeError, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_DumpMEM32RangeError");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_DumpMEM32InvalidSizeTooSmall, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_DumpMEM32InvalidSizeTooSmall");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_DumpMEM32InvalidSizeTooLarge, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_DumpMEM32InvalidSizeTooLarge");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_DumpMEM32AlignmentError, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_DumpMEM32AlignmentError");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_DumpMEM16RangeError, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_DumpMEM16RangeError");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_DumpMEM16InvalidSizeTooSmall, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_DumpMEM16InvalidSizeTooSmall");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_DumpMEM16InvalidSizeTooLarge, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_DumpMEM16InvalidSizeTooLarge");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_DumpMEM16AlignmentError, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_DumpMEM16AlignmentError");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_DumpMEM8RangeError, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_DumpMEM8RangeError");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_DumpMEM8InvalidSizeTooSmall, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_DumpMEM8InvalidSizeTooSmall");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_DumpMEM8InvalidSizeTooLarge, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_DumpMEM8InvalidSizeTooLarge");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_DumpInvalidMemoryType, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_DumpInvalidMemoryType");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_LoadInvalidVerifyTypeError, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_LoadInvalidVerifyTypeError");

    /* Dump in event */
    UtTest_Add(MM_VerifyLoadDumpParams_Test_DumpEventRAM, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_DumpEventRAM");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_DumpEventEEPROM, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_DumpEventEEPROM");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_DumpEventMEM32, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_DumpEventMEM32");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_DumpEventMEM16, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_DumpEventMEM16");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_DumpEventMEM8, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_DumpEventMEM8");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_DumpEventInvalidDataSizeTooSmall, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_DumpEventInvalidDataSizeTooSmall");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_DumpEventInvalidDataSizeTooLarge, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_DumpEventInvalidDataSizeTooLarge");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_DumpEventRAMRangeError, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_DumpEventRAMRangeError");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_DumpEventEEPROMRangeError, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_DumpEventEEPROMRangeError");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_DumpEventMEM32RangeError, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_DumpEventMEM32RangeError");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_DumpEventMEM32AlignmentError, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_DumpEventMEM32AlignmentError");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_DumpEventMEM16RangeError, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_DumpEventMEM16RangeError");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_DumpEventMEM16AlignmentError, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_DumpEventMEM16AlignmentError");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_DumpEventMEM8RangeError, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_DumpEventMEM8RangeError");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_DumpEventInvalidMemType, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_DumpEventInvalidMemType");

    /* Fill */
    UtTest_Add(MM_VerifyLoadDumpParams_Test_FillRAMValidateRangeError, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_FillRAMValidateRangeError");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_FillRAMDataSizeErrorTooSmall, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_FillRAMDataSizeErrorTooSmall");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_FillRAMDataSizeErrorTooLarge, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_FillRAMDataSizeErrorTooLarge");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_FillEEPROMValidateRangeError, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_FillEEPROMValidateRangeError");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_FillEEPROMDataSizeErrorTooSmall, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_FillEEPROMDataSizeErrorTooSmall");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_FillEEPROMDataSizeErrorTooLarge, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_FillEEPROMDataSizeErrorTooLarge");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_FillMEM32ValidateRangeError, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_FillMEM32ValidateRangeError");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_FillMEM32DataSizeErrorTooSmall, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_FillMEM32DataSizeErrorTooSmall");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_FillMEM32DataSizeErrorTooLarge, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_FillMEM32DataSizeErrorTooLarge");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_FillMEM32AlignmentError, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_FillMEM32AlignmentError");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_FillMEM16ValidateRangeError, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_FillMEM16ValidateRangeError");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_FillMEM16DataSizeErrorTooSmall, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_FillMEM16DataSizeErrorTooSmall");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_FillMEM16DataSizeErrorTooLarge, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_FillMEM16DataSizeErrorTooLarge");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_FillMEM16AlignmentError, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_FillMEM16AlignmentError");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_FillMEM8ValidateRangeError, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_FillMEM8ValidateRangeError");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_FillMEM8DataSizeErrorTooSmall, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_FillMEM8DataSizeErrorTooSmall");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_FillMEM8DataSizeErrorTooLarge, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_FillMEM8DataSizeErrorTooLarge");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_FillInvalidMemTypeError, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_FillInvalidMemTypeError");

    /* WID */
    UtTest_Add(MM_VerifyLoadDumpParams_Test_WIDNominal, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_WIDNominal");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_WIDMemValidateError, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_WIDMemValidateError");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_WIDDataSizeErrorTooSmall, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_WIDDataSizeErrorTooSmall");
    UtTest_Add(MM_VerifyLoadDumpParams_Test_WIDDataSizeErrorTooLarge, MM_Test_Setup, MM_Test_TearDown,
               "MM_VerifyLoadDumpParams_Test_WIDDataSizeErrorTooLarge");

    UtTest_Add(MM_Verify32Aligned_Test, MM_Test_Setup, MM_Test_TearDown, "MM_Verify32Aligned_Test");
    UtTest_Add(MM_Verify16Aligned_Test, MM_Test_Setup, MM_Test_TearDown, "MM_Verify16Aligned_Test");

    UtTest_Add(MM_ResolveSymAddr_Test, MM_Test_Setup, MM_Test_TearDown, "MM_ResolveSymAddr_Test");

    UtTest_Add(MM_ComputeCRCFromFile_Test, MM_Test_Setup, MM_Test_TearDown, "MM_ComputeCRCFromFile_Test");

} /* end UtTest_Setup */

/************************/
/*  End of File Comment */
/************************/
