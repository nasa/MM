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
 *   Unit tests for mm_utils.c
 */

/************************************************************************
** Includes
*************************************************************************/
#include "mm_dump.h"
#include "mm_eventids.h"
#include "mm_fcncodes.h"
#include "mm_filedefs.h"
#include "mm_msg.h"
#include "mm_msgdefs.h"
#include "mm_msgids.h"
#include "mm_test_utils.h"
#include "mm_utils.h"
#include "mm_version.h"

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

/*
 * Function Definitions
 */

void Test_MM_ResetHk(void)
{
    MM_AppData.HkTlm.Payload.LastAction     = 1;
    MM_AppData.HkTlm.Payload.MemType        = 2;
    MM_AppData.HkTlm.Payload.Address        = CFE_ES_MEMADDRESS_C(3);
    MM_AppData.HkTlm.Payload.DataValue      = 4;
    MM_AppData.HkTlm.Payload.BytesProcessed = 5;
    MM_AppData.HkTlm.Payload.FileName[0]    = 6;

    /* Execute the function being tested */
    MM_ResetHk();

    /* Verify results */
    UtAssert_True(MM_AppData.HkTlm.Payload.LastAction == MM_LastAction_NOACTION,
                  "MM_AppData.HkTlm.Payload.LastAction == MM_LastAction_NOACTION");
    UtAssert_True(MM_AppData.HkTlm.Payload.MemType == MM_MemType_NOMEMTYPE,
                  "MM_AppData.HkTlm.Payload.MemType == MM_MemType_NOMEMTYPE");
    UtAssert_ADDRESS_EQ(CFE_ES_MEMADDRESS_TO_PTR(MM_AppData.HkTlm.Payload.Address), MM_INTERNAL_CLEAR_ADDR);
    UtAssert_True(MM_AppData.HkTlm.Payload.DataValue == MM_INTERNAL_CLEAR_PATTERN,
                  "MM_AppData.HkTlm.Payload.DataValue == MM_INTERNAL_CLEAR_PATTERN");
    UtAssert_True(MM_AppData.HkTlm.Payload.BytesProcessed == 0, "MM_AppData.BytesProcessed == 0");
    UtAssert_True(MM_AppData.HkTlm.Payload.FileName[0] == MM_INTERNAL_CLEAR_FNAME,
                  "MM_AppData.HkTlm.Payload.FileName[0] == MM_INTERNAL_CLEAR_FNAME");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_SegmentBreak_Nominal(void)
{
    /* Execute the function being tested */
    MM_SegmentBreak();

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_VerifyPeekPokeParams_ByteWidthRAM(void)
{
    int32             Result;
    uint32            Address    = 0;
    MM_MemType_Enum_t MemType    = MM_MemType_RAM;
    size_t            SizeInBits = MM_INTERNAL_BYTE_BIT_WIDTH;

    /* Execute the function being tested */
    Result = MM_VerifyPeekPokeParams(Address, MemType, SizeInBits);

    /* Verify results */
    UtAssert_True(Result == OS_SUCCESS, "Result == OS_SUCCESS");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_VerifyPeekPokeParams_WordWidthMEM16(void)
{
    int32             Result;
    uint32            Address    = 0;
    MM_MemType_Enum_t MemType    = MM_MemType_MEM16;
    size_t            SizeInBits = MM_INTERNAL_WORD_BIT_WIDTH;

    /* Execute the function being tested */
    Result = MM_VerifyPeekPokeParams(Address, MemType, SizeInBits);

    /* Verify results */
    UtAssert_True(Result == OS_SUCCESS, "Result == OS_SUCCESS");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_VerifyPeekPokeParams_DWordWidthMEM32(void)
{
    int32             Result;
    uint32            Address    = 0;
    MM_MemType_Enum_t MemType    = MM_MemType_MEM32;
    size_t            SizeInBits = MM_INTERNAL_DWORD_BIT_WIDTH;

    /* Execute the function being tested */
    Result = MM_VerifyPeekPokeParams(Address, MemType, SizeInBits);

    /* Verify results */
    UtAssert_True(Result == OS_SUCCESS, "Result == OS_SUCCESS");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_VerifyPeekPokeParams_WordWidthAlignmentError(void)
{
    int32             Result;
    uint32            Address    = 1;
    MM_MemType_Enum_t MemType    = MM_MemType_MEM16;
    size_t            SizeInBits = MM_INTERNAL_WORD_BIT_WIDTH;

    /* Execute the function being tested */
    Result = MM_VerifyPeekPokeParams(Address, MemType, SizeInBits);

    /* Verify results */
    UtAssert_True(Result == OS_ERROR_ADDRESS_MISALIGNED, "Result == OS_ERROR_ADDRESS_MISALIGNED");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_ALIGN16_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Data and address not 16 bit aligned: Addr = %p Size = %u");
}

void Test_MM_VerifyPeekPokeParams_DWordWidthAlignmentError(void)
{
    int32             Result;
    uint32            Address    = 1;
    MM_MemType_Enum_t MemType    = MM_MemType_MEM32;
    size_t            SizeInBits = MM_INTERNAL_DWORD_BIT_WIDTH;

    /* Execute the function being tested */
    Result = MM_VerifyPeekPokeParams(Address, MemType, SizeInBits);

    /* Verify results */
    UtAssert_True(Result == OS_ERROR_ADDRESS_MISALIGNED, "Result == OS_ERROR_ADDRESS_MISALIGNED");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_ALIGN32_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Data and address not 32 bit aligned: Addr = %p Size = %u");
}

void Test_MM_VerifyPeekPokeParams_InvalidDataSize(void)
{
    int32             Result;
    uint32            Address    = 0;
    MM_MemType_Enum_t MemType    = MM_MemType_MEM8;
    /* To reach size error: Peeks and Pokes must be 8 bits wide for this memory
     * type */
    size_t            SizeInBits = MM_INTERNAL_WORD_BIT_WIDTH;

    /* Execute the function being tested */
    Result = MM_VerifyPeekPokeParams(Address, MemType, SizeInBits);

    /* Verify results */
    UtAssert_True(Result == OS_ERR_INVALID_SIZE, "Result == OS_ERR_INVALID_SIZE");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_DATA_SIZE_BITS_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Data size in bits invalid: Data Size = %u");
}

void Test_MM_VerifyPeekPokeParams_EEPROM(void)
{
    int32             Result;
    uint32            Address    = 0;
    MM_MemType_Enum_t MemType    = MM_MemType_EEPROM;
    size_t            SizeInBits = MM_INTERNAL_BYTE_BIT_WIDTH;

    /* Execute the function being tested */
    Result = MM_VerifyPeekPokeParams(Address, MemType, SizeInBits);

    /* Verify results */
    UtAssert_True(Result == OS_SUCCESS, "Result == OS_SUCCESS");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_VerifyPeekPokeParams_MEM8(void)
{
    int32             Result;
    uint32            Address    = 0;
    MM_MemType_Enum_t MemType    = MM_MemType_MEM8;
    size_t            SizeInBits = MM_INTERNAL_BYTE_BIT_WIDTH;

    /* Execute the function being tested */
    Result = MM_VerifyPeekPokeParams(Address, MemType, SizeInBits);

    /* Verify results */
    UtAssert_True(Result == OS_SUCCESS, "Result == OS_SUCCESS");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_VerifyPeekPokeParams_RAMValidateRangeError(void)
{
    int32             Result;
    uint32            Address    = 0;
    MM_MemType_Enum_t MemType    = MM_MemType_RAM;
    size_t            SizeInBits = MM_INTERNAL_BYTE_BIT_WIDTH;

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, CFE_PSP_INVALID_MEM_TYPE);

    /* Execute the function being tested */
    Result = MM_VerifyPeekPokeParams(Address, MemType, SizeInBits);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_INVALID_MEM_TYPE, "Result == CFE_PSP_INVALID_MEM_TYPE");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_OS_MEMVALIDATE_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "CFE_PSP_MemValidateRange error received: RC = 0x%08X "
                         "Addr = %p Size = %u MemType = MEM_RAM");
}

void Test_MM_VerifyPeekPokeParams_EEPROMValidateRangeError(void)
{
    int32             Result;
    uint32            Address    = 0;
    MM_MemType_Enum_t MemType    = MM_MemType_EEPROM;
    size_t            SizeInBits = MM_INTERNAL_BYTE_BIT_WIDTH;

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, CFE_PSP_INVALID_MEM_TYPE);

    /* Execute the function being tested */
    Result = MM_VerifyPeekPokeParams(Address, MemType, SizeInBits);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_INVALID_MEM_TYPE, "Result == CFE_PSP_INVALID_MEM_TYPE");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_OS_MEMVALIDATE_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "CFE_PSP_MemValidateRange error received: RC = 0x%08X "
                         "Addr = %p Size = %u MemType = MEM_EEPROM");
}

void Test_MM_VerifyPeekPokeParams_MEM32ValidateRangeError(void)
{
    int32             Result;
    uint32            Address    = 0;
    MM_MemType_Enum_t MemType    = MM_MemType_MEM32;
    size_t            SizeInBits = MM_INTERNAL_BYTE_BIT_WIDTH;

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, CFE_PSP_INVALID_MEM_TYPE);

    /* Execute the function being tested */
    Result = MM_VerifyPeekPokeParams(Address, MemType, SizeInBits);

    /* Verify results */
    UtAssert_INT32_EQ(Result, CFE_PSP_INVALID_MEM_TYPE);

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_OS_MEMVALIDATE_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "CFE_PSP_MemValidateRange error received: RC = 0x%08X "
                         "Addr = %p Size = %u MemType = MEM32");
}

void Test_MM_VerifyPeekPokeParams_MEM16ValidateRangeError(void)
{
    int32             Result;
    uint32            Address    = 0;
    MM_MemType_Enum_t MemType    = MM_MemType_MEM16;
    size_t            SizeInBits = MM_INTERNAL_BYTE_BIT_WIDTH;

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, CFE_PSP_INVALID_MEM_TYPE);

    /* Execute the function being tested */
    Result = MM_VerifyPeekPokeParams(Address, MemType, SizeInBits);

    /* Verify results */
    UtAssert_INT32_EQ(Result, CFE_PSP_INVALID_MEM_TYPE);

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_OS_MEMVALIDATE_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "CFE_PSP_MemValidateRange error received: RC = 0x%08X "
                         "Addr = %p Size = %u MemType = MEM16");
}

void Test_MM_VerifyPeekPokeParams_MEM8ValidateRangeError(void)
{
    int32             Result;
    uint32            Address    = 0;
    MM_MemType_Enum_t MemType    = MM_MemType_MEM8;
    size_t            SizeInBits = MM_INTERNAL_BYTE_BIT_WIDTH;

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, CFE_PSP_INVALID_MEM_TYPE);

    /* Execute the function being tested */
    Result = MM_VerifyPeekPokeParams(Address, MemType, SizeInBits);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_INVALID_MEM_TYPE, "Result == CFE_PSP_INVALID_MEM_TYPE");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_OS_MEMVALIDATE_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "CFE_PSP_MemValidateRange error received: RC = 0x%08X "
                         "Addr = %p Size = %u MemType = MEM8");
}

void Test_MM_VerifyPeekPokeParams_MEM32InvalidDataSize(void)
{
    int32             Result;
    uint32            Address    = 0;
    MM_MemType_Enum_t MemType    = MM_MemType_MEM32;
    size_t            SizeInBits = MM_INTERNAL_BYTE_BIT_WIDTH;

    /* Execute the function being tested */
    Result = MM_VerifyPeekPokeParams(Address, MemType, SizeInBits);

    /* Verify results */
    UtAssert_True(Result == OS_ERR_INVALID_SIZE, "Result == faOS_ERR_INVALID_SIZElse");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_DATA_SIZE_BITS_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Data size in bits invalid: Data Size = %u");
}

void Test_MM_VerifyPeekPokeParams_MEM16InvalidDataSize(void)
{
    int32             Result;
    uint32            Address    = 0;
    MM_MemType_Enum_t MemType    = MM_MemType_MEM16;
    size_t            SizeInBits = MM_INTERNAL_BYTE_BIT_WIDTH;

    /* Execute the function being tested */
    Result = MM_VerifyPeekPokeParams(Address, MemType, SizeInBits);

    /* Verify results */
    UtAssert_True(Result == OS_ERR_INVALID_SIZE, "Result == OS_ERR_INVALID_SIZE");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_DATA_SIZE_BITS_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Data size in bits invalid: Data Size = %u");
}

void Test_MM_VerifyPeekPokeParams_MEM8InvalidDataSize(void)
{
    int32             Result;
    uint32            Address    = 0;
    MM_MemType_Enum_t MemType    = MM_MemType_MEM8;
    size_t            SizeInBits = 99;

    /* Execute the function being tested */
    Result = MM_VerifyPeekPokeParams(Address, MemType, SizeInBits);

    /* Verify results */
    UtAssert_True(Result == OS_ERR_INVALID_SIZE, "Result == OS_ERR_INVALID_SIZE");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_DATA_SIZE_BITS_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Data size in bits invalid: Data Size = %u");
}

void Test_MM_VerifyPeekPokeParams_InvalidMemType(void)
{
    int32             Result;
    uint32            Address    = 0;
    MM_MemType_Enum_t MemType    = 99;
    size_t            SizeInBits = MM_INTERNAL_BYTE_BIT_WIDTH;

    /* Execute the function being tested */
    Result = MM_VerifyPeekPokeParams(Address, MemType, SizeInBits);

    /* Verify results */
    UtAssert_True(Result == OS_ERR_INVALID_ARGUMENT, "Result == OS_ERR_INVALID_ARGUMENT");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0, MM_MEMTYPE_ERR_EID, CFE_EVS_EventType_ERROR, "Invalid memory type specified: MemType = %d");
}

/* Loading */
void Test_MM_VerifyLoadDumpParams_LoadRAMValidateRangeError(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_RAM;
    size_t            SizeInBytes = 1;

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, CFE_PSP_INVALID_MEM_TYPE);

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_LOAD);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_INVALID_MEM_TYPE, "Result == CFE_PSP_INVALID_MEM_TYPE");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_OS_MEMVALIDATE_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "CFE_PSP_MemValidateRange error received: RC = 0x%08X "
                         "Addr = %p Size = %u MemType = %s");
}

void Test_MM_VerifyLoadDumpParams_LoadRAMDataSizeErrorTooSmall(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_RAM;
    size_t            SizeInBytes = 0;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_LOAD);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR, "Result == CFE_PSP_ERROR");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_DATA_SIZE_BYTES_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Data size in bytes invalid or exceeds limits: Data Size = %u");
}

void Test_MM_VerifyLoadDumpParams_LoadRAMDataSizeErrorTooLarge(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_RAM;
    size_t            SizeInBytes = MM_INTERNAL_MAX_LOAD_FILE_DATA_RAM + 1;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_LOAD);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR, "Result == CFE_PSP_ERROR");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_DATA_SIZE_BYTES_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Data size in bytes invalid or exceeds limits: Data Size = %u");
}

void Test_MM_VerifyLoadDumpParams_LoadEEPROMValidateRangeError(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_EEPROM;
    size_t            SizeInBytes = 1;

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, CFE_PSP_INVALID_MEM_TYPE);

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_LOAD);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_INVALID_MEM_TYPE, "Result == CFE_PSP_INVALID_MEM_TYPE");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_OS_MEMVALIDATE_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "CFE_PSP_MemValidateRange error received: RC = 0x%08X "
                         "Addr = %p Size = %u MemType = %s");
}

void Test_MM_VerifyLoadDumpParams_LoadEEPROMDataSizeErrorTooSmall(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_EEPROM;
    size_t            SizeInBytes = 0;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_LOAD);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR, "Result == CFE_PSP_ERROR");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_DATA_SIZE_BYTES_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Data size in bytes invalid or exceeds limits: Data Size = %u");
}

void Test_MM_VerifyLoadDumpParams_LoadEEPROMDataSizeErrorTooLarge(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_EEPROM;
    size_t            SizeInBytes = MM_INTERNAL_MAX_LOAD_FILE_DATA_EEPROM + 1;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_LOAD);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR, "Result == CFE_PSP_ERROR");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_DATA_SIZE_BYTES_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Data size in bytes invalid or exceeds limits: Data Size = %u");
}

void Test_MM_VerifyLoadDumpParams_LoadMEM32ValidateRangeError(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_MEM32;
    size_t            SizeInBytes = 4;

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, CFE_PSP_INVALID_MEM_TYPE);

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_LOAD);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_INVALID_MEM_TYPE, "Result == CFE_PSP_INVALID_MEM_TYPE");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_OS_MEMVALIDATE_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "CFE_PSP_MemValidateRange error received: RC = 0x%08X "
                         "Addr = %p Size = %u MemType = %s");
}

void Test_MM_VerifyLoadDumpParams_LoadMEM32DataSizeErrorTooSmall(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_MEM32;
    size_t            SizeInBytes = 0;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_LOAD);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR, "Result == CFE_PSP_ERROR");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_DATA_SIZE_BYTES_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Data size in bytes invalid or exceeds limits: Data Size = %u");
}

void Test_MM_VerifyLoadDumpParams_LoadMEM32DataSizeErrorTooLarge(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_MEM32;
    size_t            SizeInBytes = MM_INTERNAL_MAX_LOAD_FILE_DATA_MEM32 + 4;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_LOAD);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR, "Result == CFE_PSP_ERROR");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_DATA_SIZE_BYTES_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Data size in bytes invalid or exceeds limits: Data Size = %u");
}

void Test_MM_VerifyLoadDumpParams_LoadMEM32AlignmentError(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_MEM32;
    size_t            SizeInBytes = 1;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_LOAD);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR_ADDRESS_MISALIGNED, "Result == CFE_PSP_ERROR_ADDRESS_MISALIGNED");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_ALIGN32_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Data and address not 32 bit aligned: Addr = %p Size = %u");
}

void Test_MM_VerifyLoadDumpParams_LoadMEM16ValidateRangeError(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_MEM16;
    size_t            SizeInBytes = 2;

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, CFE_PSP_INVALID_MEM_TYPE);

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_LOAD);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_INVALID_MEM_TYPE, "Result == CFE_PSP_INVALID_MEM_TYPE");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_OS_MEMVALIDATE_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "CFE_PSP_MemValidateRange error received: RC = 0x%08X "
                         "Addr = %p Size = %u MemType = %s");
}

void Test_MM_VerifyLoadDumpParams_LoadMEM16DataSizeErrorTooSmall(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_MEM16;
    size_t            SizeInBytes = 0;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_LOAD);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR, "Result == CFE_PSP_ERROR");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_DATA_SIZE_BYTES_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Data size in bytes invalid or exceeds limits: Data Size = %u");
}

void Test_MM_VerifyLoadDumpParams_LoadMEM16DataSizeErrorTooLarge(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_MEM16;
    size_t            SizeInBytes = MM_INTERNAL_MAX_LOAD_FILE_DATA_MEM16 + 2;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_LOAD);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR, "Result == CFE_PSP_ERROR");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_DATA_SIZE_BYTES_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Data size in bytes invalid or exceeds limits: Data Size = %u");
}

void Test_MM_VerifyLoadDumpParams_LoadMEM16AlignmentError(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_MEM16;
    size_t            SizeInBytes = 1;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_LOAD);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR_ADDRESS_MISALIGNED, "Result == CFE_PSP_ERROR_ADDRESS_MISALIGNED");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_ALIGN16_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Data and address not 16 bit aligned: Addr = %p Size = %u");
}

void Test_MM_VerifyLoadDumpParams_LoadMEM8ValidateRangeError(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_MEM8;
    size_t            SizeInBytes = 1;

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, CFE_PSP_INVALID_MEM_TYPE);

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_LOAD);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_INVALID_MEM_TYPE, "Result == CFE_PSP_INVALID_MEM_TYPE");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_OS_MEMVALIDATE_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "CFE_PSP_MemValidateRange error received: RC = 0x%08X "
                         "Addr = %p Size = %u MemType = %s");
}

void Test_MM_VerifyLoadDumpParams_LoadMEM8DataSizeErrorTooSmall(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_MEM8;
    size_t            SizeInBytes = 0;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_LOAD);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR, "Result == CFE_PSP_ERROR");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_DATA_SIZE_BYTES_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Data size in bytes invalid or exceeds limits: Data Size = %u");
}

void Test_MM_VerifyLoadDumpParams_LoadMEM8DataSizeErrorTooLarge(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_MEM8;
    size_t            SizeInBytes = MM_INTERNAL_MAX_LOAD_FILE_DATA_MEM8 + 1;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_LOAD);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR, "Result == CFE_PSP_ERROR");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_DATA_SIZE_BYTES_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Data size in bytes invalid or exceeds limits: Data Size = %u");
}

void Test_MM_VerifyLoadDumpParams_LoadInvalidMemTypeError(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = 99;
    size_t            SizeInBytes = MM_INTERNAL_MAX_LOAD_FILE_DATA_MEM8 + 1;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_LOAD);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR, "Result == CFE_PSP_ERROR");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0, MM_MEMTYPE_ERR_EID, CFE_EVS_EventType_ERROR, "Invalid memory type specified: MemType = %d");
}

void Test_MM_VerifyLoadDumpParams_LoadInvalidVerifyTypeError(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_RAM;
    size_t            SizeInBytes = MM_INTERNAL_MAX_LOAD_FILE_DATA_MEM8;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, -1);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR, "Result == CFE_PSP_ERROR");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

/* Dumping */
void Test_MM_VerifyLoadDumpParams_DumpRAM(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_RAM;
    size_t            SizeInBytes = 1;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_DUMP);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_SUCCESS, "Result == CFE_PSP_SUCCESS");

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_VerifyLoadDumpParams_DumpEEPROM(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_EEPROM;
    size_t            SizeInBytes = 1;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_DUMP);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_SUCCESS, "Result == CFE_PSP_SUCCESS");

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_VerifyLoadDumpParams_DumpMEM32(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_MEM32;
    size_t            SizeInBytes = 4;

    UT_SetDefaultReturnValue(UT_KEY(CFE_PSP_MemValidateRange), CFE_PSP_SUCCESS);

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_DUMP);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_SUCCESS, "Result == CFE_PSP_SUCCESS");

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_VerifyLoadDumpParams_DumpMEM16(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_MEM16;
    size_t            SizeInBytes = 2;

    UT_SetDefaultReturnValue(UT_KEY(CFE_PSP_MemValidateRange), CFE_PSP_SUCCESS);

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_DUMP);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_SUCCESS, "Result == CFE_PSP_SUCCESS");

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_VerifyLoadDumpParams_DumpMEM8(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_MEM8;
    size_t            SizeInBytes = 1;

    UT_SetDefaultReturnValue(UT_KEY(CFE_PSP_MemValidateRange), CFE_PSP_SUCCESS);

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_DUMP);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_SUCCESS, "Result == CFE_PSP_SUCCESS");

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_VerifyLoadDumpParams_DumpRAMRangeError(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_RAM;
    size_t            SizeInBytes = 1;

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, CFE_PSP_INVALID_MEM_TYPE);

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_DUMP);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_INVALID_MEM_TYPE, "Result == CFE_PSP_INVALID_MEM_TYPE");

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_OS_MEMVALIDATE_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "CFE_PSP_MemValidateRange error received: RC = 0x%08X "
                         "Addr = %p Size = %u MemType = %s");
}

void Test_MM_VerifyLoadDumpParams_DumpRAMInvalidSizeTooSmall(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_RAM;
    size_t            SizeInBytes = 0;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_DUMP);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR, "Result == CFE_PSP_ERROR");

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_DATA_SIZE_BYTES_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Data size in bytes invalid or exceeds limits: Data Size = %u");
}

void Test_MM_VerifyLoadDumpParams_DumpRAMInvalidSizeTooLarge(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_RAM;
    size_t            SizeInBytes = MM_INTERNAL_MAX_DUMP_FILE_DATA_RAM + 1;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_DUMP);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR, "Result == CFE_PSP_ERROR");

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_DATA_SIZE_BYTES_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Data size in bytes invalid or exceeds limits: Data Size = %u");
}

void Test_MM_VerifyLoadDumpParams_DumpEEPROMRangeError(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_EEPROM;
    size_t            SizeInBytes = 1;

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, CFE_PSP_INVALID_MEM_TYPE);

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_DUMP);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_INVALID_MEM_TYPE, "Result == CFE_PSP_INVALID_MEM_TYPE");

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_OS_MEMVALIDATE_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "CFE_PSP_MemValidateRange error received: RC = 0x%08X "
                         "Addr = %p Size = %u MemType = %s");
}

void Test_MM_VerifyLoadDumpParams_DumpEEPROMInvalidSizeTooSmall(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_EEPROM;
    size_t            SizeInBytes = 0;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_DUMP);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR, "Result == CFE_PSP_ERROR");

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_DATA_SIZE_BYTES_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Data size in bytes invalid or exceeds limits: Data Size = %u");
}

void Test_MM_VerifyLoadDumpParams_DumpEEPROMInvalidSizeTooLarge(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_EEPROM;
    size_t            SizeInBytes = MM_INTERNAL_MAX_DUMP_FILE_DATA_EEPROM + 1;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_DUMP);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR, "Result == CFE_PSP_ERROR");

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_DATA_SIZE_BYTES_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Data size in bytes invalid or exceeds limits: Data Size = %u");
}

void Test_MM_VerifyLoadDumpParams_DumpMEM32RangeError(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_MEM32;
    size_t            SizeInBytes = 4;

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, CFE_PSP_INVALID_MEM_TYPE);

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_DUMP);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_INVALID_MEM_TYPE, "Result == CFE_PSP_INVALID_MEM_TYPE");

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_OS_MEMVALIDATE_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "CFE_PSP_MemValidateRange error received: RC = 0x%08X "
                         "Addr = %p Size = %u MemType = %s");
}

void Test_MM_VerifyLoadDumpParams_DumpMEM32InvalidSizeTooSmall(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_MEM32;
    size_t            SizeInBytes = 0;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_DUMP);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR, "Result == CFE_PSP_ERROR");

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_DATA_SIZE_BYTES_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Data size in bytes invalid or exceeds limits: Data Size = %u");
}

void Test_MM_VerifyLoadDumpParams_DumpMEM32InvalidSizeTooLarge(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_MEM32;
    size_t            SizeInBytes = MM_INTERNAL_MAX_DUMP_FILE_DATA_MEM32 + 4;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_DUMP);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR, "Result == CFE_PSP_ERROR");

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_DATA_SIZE_BYTES_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Data size in bytes invalid or exceeds limits: Data Size = %u");
}

void Test_MM_VerifyLoadDumpParams_DumpMEM32AlignmentError(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_MEM32;
    size_t            SizeInBytes = 3;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_DUMP);

    /* Verify results */
    UtAssert_INT32_EQ(Result, CFE_PSP_ERROR_ADDRESS_MISALIGNED);

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_ALIGN32_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Data and address not 32 bit aligned: Addr = %p Size = %u");
}

void Test_MM_VerifyLoadDumpParams_DumpMEM16RangeError(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_MEM16;
    size_t            SizeInBytes = 2;

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, CFE_PSP_INVALID_MEM_TYPE);

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_DUMP);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_INVALID_MEM_TYPE, "Result == CFE_PSP_INVALID_MEM_TYPE");

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_OS_MEMVALIDATE_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "CFE_PSP_MemValidateRange error received: RC = 0x%08X "
                         "Addr = %p Size = %u MemType = %s");
}

void Test_MM_VerifyLoadDumpParams_DumpMEM16InvalidSizeTooSmall(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_MEM16;
    size_t            SizeInBytes = 0;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_DUMP);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR, "Result == CFE_PSP_ERROR");

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_DATA_SIZE_BYTES_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Data size in bytes invalid or exceeds limits: Data Size = %u");
}

void Test_MM_VerifyLoadDumpParams_DumpMEM16InvalidSizeTooLarge(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_MEM16;
    size_t            SizeInBytes = MM_INTERNAL_MAX_DUMP_FILE_DATA_MEM16 + 2;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_DUMP);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR, "Result == CFE_PSP_ERROR");

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_DATA_SIZE_BYTES_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Data size in bytes invalid or exceeds limits: Data Size = %u");
}

void Test_MM_VerifyLoadDumpParams_DumpMEM16AlignmentError(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_MEM16;
    size_t            SizeInBytes = 3;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_DUMP);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR_ADDRESS_MISALIGNED, "Result == CFE_PSP_ERROR_ADDRESS_MISALIGNED");

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_ALIGN16_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Data and address not 16 bit aligned: Addr = %p Size = %u");
}

void Test_MM_VerifyLoadDumpParams_DumpMEM8RangeError(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_MEM8;
    size_t            SizeInBytes = 1;

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, CFE_PSP_INVALID_MEM_TYPE);

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_DUMP);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_INVALID_MEM_TYPE, "Result == CFE_PSP_INVALID_MEM_TYPE");

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_OS_MEMVALIDATE_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "CFE_PSP_MemValidateRange error received: RC = 0x%08X "
                         "Addr = %p Size = %u MemType = %s");
}

void Test_MM_VerifyLoadDumpParams_DumpMEM8InvalidSizeTooSmall(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_MEM8;
    size_t            SizeInBytes = 0;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_DUMP);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR, "Result == CFE_PSP_ERROR");

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_DATA_SIZE_BYTES_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Data size in bytes invalid or exceeds limits: Data Size = %u");
}

void Test_MM_VerifyLoadDumpParams_DumpMEM8InvalidSizeTooLarge(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_MEM8;
    size_t            SizeInBytes = MM_INTERNAL_MAX_DUMP_FILE_DATA_MEM8 + 1;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_DUMP);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR, "Result == CFE_PSP_ERROR");

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_DATA_SIZE_BYTES_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Data size in bytes invalid or exceeds limits: Data Size = %u");
}

void Test_MM_VerifyLoadDumpParams_DumpInvalidMemoryType(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = 99;
    size_t            SizeInBytes = 1;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_DUMP);

    /* Verify results */
    UtAssert_INT32_EQ(Result, CFE_PSP_ERROR);

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0, MM_MEMTYPE_ERR_EID, CFE_EVS_EventType_ERROR, "Invalid memory type specified: MemType = %d");
}

/* Dump in event */

void Test_MM_VerifyLoadDumpParams_DumpEventRAM(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_RAM;
    size_t            SizeInBytes = 1;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_EVENT);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_SUCCESS, "Result == CFE_PSP_SUCCESS");

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_VerifyLoadDumpParams_DumpEventEEPROM(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_EEPROM;
    size_t            SizeInBytes = 1;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_EVENT);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_SUCCESS, "Result == CFE_PSP_SUCCESS");

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_VerifyLoadDumpParams_DumpEventMEM32(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_MEM32;
    size_t            SizeInBytes = 4;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_EVENT);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_SUCCESS, "Result == CFE_PSP_SUCCESS");

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_VerifyLoadDumpParams_DumpEventMEM16(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_MEM16;
    size_t            SizeInBytes = 2;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_EVENT);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_SUCCESS, "Result == CFE_PSP_SUCCESS");

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_VerifyLoadDumpParams_DumpEventMEM8(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_MEM8;
    size_t            SizeInBytes = 1;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_EVENT);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_SUCCESS, "Result == CFE_PSP_SUCCESS");

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_VerifyLoadDumpParams_DumpEventInvalidDataSizeTooSmall(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_MEM8;
    size_t            SizeInBytes = 0;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_EVENT);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR, "Result == CFE_PSP_ERROR");

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_DATA_SIZE_BYTES_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Data size in bytes invalid or exceeds limits: Data Size = %u");
}

void Test_MM_VerifyLoadDumpParams_DumpEventInvalidDataSizeTooLarge(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_MEM8;
    size_t            SizeInBytes = MM_INTERNAL_MAX_DUMP_INEVENT_BYTES + 1;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_EVENT);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR, "Result == CFE_PSP_ERROR");

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_DATA_SIZE_BYTES_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Data size in bytes invalid or exceeds limits: Data Size = %u");
}

void Test_MM_VerifyLoadDumpParams_DumpEventRAMRangeError(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_RAM;
    size_t            SizeInBytes = 1;

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, CFE_PSP_INVALID_MEM_TYPE);

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_EVENT);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_INVALID_MEM_TYPE, "Result == CFE_PSP_INVALID_MEM_TYPE");

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_OS_MEMVALIDATE_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "CFE_PSP_MemValidateRange error received: RC = 0x%08X "
                         "Addr = %p Size = %u MemType = %s");
}

void Test_MM_VerifyLoadDumpParams_DumpEventEEPROMRangeError(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_EEPROM;
    size_t            SizeInBytes = 1;

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, CFE_PSP_INVALID_MEM_TYPE);

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_EVENT);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_INVALID_MEM_TYPE, "Result == CFE_PSP_INVALID_MEM_TYPE");

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_OS_MEMVALIDATE_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "CFE_PSP_MemValidateRange error received: RC = 0x%08X "
                         "Addr = %p Size = %u MemType = %s");
}

void Test_MM_VerifyLoadDumpParams_DumpEventMEM32RangeError(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_MEM32;
    size_t            SizeInBytes = 4;

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, CFE_PSP_INVALID_MEM_TYPE);

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_EVENT);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_INVALID_MEM_TYPE, "Result == CFE_PSP_INVALID_MEM_TYPE");

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_OS_MEMVALIDATE_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "CFE_PSP_MemValidateRange error received: RC = 0x%08X "
                         "Addr = %p Size = %u MemType = %s");
}

void Test_MM_VerifyLoadDumpParams_DumpEventMEM32AlignmentError(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_MEM32;
    size_t            SizeInBytes = 3;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_EVENT);

    /* Verify results */
    UtAssert_INT32_EQ(Result, CFE_PSP_ERROR_ADDRESS_MISALIGNED);

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_ALIGN32_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Data and address not 32 bit aligned: Addr = %p Size = %u");
}

void Test_MM_VerifyLoadDumpParams_DumpEventMEM16RangeError(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_MEM16;
    size_t            SizeInBytes = 2;

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, CFE_PSP_INVALID_MEM_TYPE);

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_EVENT);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_INVALID_MEM_TYPE, "Result == CFE_PSP_INVALID_MEM_TYPE");

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_OS_MEMVALIDATE_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "CFE_PSP_MemValidateRange error received: RC = 0x%08X "
                         "Addr = %p Size = %u MemType = %s");
}

void Test_MM_VerifyLoadDumpParams_DumpEventMEM16AlignmentError(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_MEM16;
    size_t            SizeInBytes = 3;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_EVENT);

    /* Verify results */
    UtAssert_INT32_EQ(Result, CFE_PSP_ERROR_ADDRESS_MISALIGNED);

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_ALIGN16_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Data and address not 16 bit aligned: Addr = %p Size = %u");
}

void Test_MM_VerifyLoadDumpParams_DumpEventMEM8RangeError(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_MEM8;
    size_t            SizeInBytes = 1;

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, CFE_PSP_INVALID_MEM_TYPE);

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_EVENT);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_INVALID_MEM_TYPE, "Result == CFE_PSP_INVALID_MEM_TYPE");

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_OS_MEMVALIDATE_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "CFE_PSP_MemValidateRange error received: RC = 0x%08X "
                         "Addr = %p Size = %u MemType = %s");
}

void Test_MM_VerifyLoadDumpParams_DumpEventInvalidMemType(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = 99;
    size_t            SizeInBytes = 1;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_EVENT);

    /* Verify results */
    UtAssert_INT32_EQ(Result, CFE_PSP_ERROR);

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0, MM_MEMTYPE_ERR_EID, CFE_EVS_EventType_ERROR, "Invalid memory type specified: MemType = %d");
}

/* Fill */

void Test_MM_VerifyLoadDumpParams_FillRAMValidateRangeError(void)
{
    int32             Result;
    uint32            Address     = 1;
    MM_MemType_Enum_t MemType     = MM_MemType_RAM;
    size_t            SizeInBytes = 1;

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, CFE_PSP_INVALID_MEM_TYPE);

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_FILL);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_INVALID_MEM_TYPE, "Result == CFE_PSP_INVALID_MEM_TYPE");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_OS_MEMVALIDATE_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "CFE_PSP_MemValidateRange error received: RC = 0x%08X "
                         "Addr = %p Size = %u MemType = %s");
}

void Test_MM_VerifyLoadDumpParams_FillRAMDataSizeErrorTooSmall(void)
{
    int32             Result;
    uint32            Address     = 1;
    MM_MemType_Enum_t MemType     = MM_MemType_RAM;
    size_t            SizeInBytes = 0;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_FILL);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR, "Result == CFE_PSP_ERROR");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_DATA_SIZE_BYTES_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Data size in bytes invalid or exceeds limits: Data Size = %u");
}

void Test_MM_VerifyLoadDumpParams_FillRAMDataSizeErrorTooLarge(void)
{
    int32             Result;
    uint32            Address     = 1;
    MM_MemType_Enum_t MemType     = MM_MemType_RAM;
    size_t            SizeInBytes = MM_INTERNAL_MAX_FILL_DATA_RAM + 1;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_FILL);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR, "Result == CFE_PSP_ERROR");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_DATA_SIZE_BYTES_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Data size in bytes invalid or exceeds limits: Data Size = %u");
}

void Test_MM_VerifyLoadDumpParams_FillEEPROMValidateRangeError(void)
{
    int32             Result;
    uint32            Address     = 1;
    MM_MemType_Enum_t MemType     = MM_MemType_EEPROM;
    size_t            SizeInBytes = 1;

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, CFE_PSP_INVALID_MEM_TYPE);

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_FILL);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_INVALID_MEM_TYPE, "Result == CFE_PSP_INVALID_MEM_TYPE");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_OS_MEMVALIDATE_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "CFE_PSP_MemValidateRange error received: RC = 0x%08X "
                         "Addr = %p Size = %u MemType = %s");
}

void Test_MM_VerifyLoadDumpParams_FillEEPROMDataSizeErrorTooSmall(void)
{
    int32             Result;
    uint32            Address     = 1;
    MM_MemType_Enum_t MemType     = MM_MemType_EEPROM;
    size_t            SizeInBytes = 0;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_FILL);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR, "Result == CFE_PSP_ERROR");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_DATA_SIZE_BYTES_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Data size in bytes invalid or exceeds limits: Data Size = %u");
}

void Test_MM_VerifyLoadDumpParams_FillEEPROMDataSizeErrorTooLarge(void)
{
    int32             Result;
    uint32            Address     = 1;
    MM_MemType_Enum_t MemType     = MM_MemType_EEPROM;
    size_t            SizeInBytes = MM_INTERNAL_MAX_FILL_DATA_EEPROM + 1;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_FILL);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR, "Result == CFE_PSP_ERROR");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_DATA_SIZE_BYTES_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Data size in bytes invalid or exceeds limits: Data Size = %u");
}

void Test_MM_VerifyLoadDumpParams_FillMEM32ValidateRangeError(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_MEM32;
    size_t            SizeInBytes = 4;

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, CFE_PSP_INVALID_MEM_TYPE);

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_FILL);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_INVALID_MEM_TYPE, "Result == CFE_PSP_INVALID_MEM_TYPE");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_OS_MEMVALIDATE_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "CFE_PSP_MemValidateRange error received: RC = 0x%08X "
                         "Addr = %p Size = %u MemType = %s");
}

void Test_MM_VerifyLoadDumpParams_FillMEM32DataSizeErrorTooSmall(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_MEM32;
    size_t            SizeInBytes = 0;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_FILL);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR, "Result == CFE_PSP_ERROR");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_DATA_SIZE_BYTES_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Data size in bytes invalid or exceeds limits: Data Size = %u");
}

void Test_MM_VerifyLoadDumpParams_FillMEM32DataSizeErrorTooLarge(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_MEM32;
    size_t            SizeInBytes = MM_INTERNAL_MAX_FILL_DATA_MEM32 + 4;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_FILL);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR, "Result == CFE_PSP_ERROR");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_DATA_SIZE_BYTES_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Data size in bytes invalid or exceeds limits: Data Size = %u");
}

void Test_MM_VerifyLoadDumpParams_FillMEM32AlignmentError(void)
{
    int32             Result;
    uint32            Address     = 1;
    MM_MemType_Enum_t MemType     = MM_MemType_MEM32;
    size_t            SizeInBytes = 1;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_FILL);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR_ADDRESS_MISALIGNED, "Result == CFE_PSP_ERROR_ADDRESS_MISALIGNED");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_ALIGN32_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Data and address not 32 bit aligned: Addr = %p Size = %u");
}

void Test_MM_VerifyLoadDumpParams_FillMEM16ValidateRangeError(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_MEM16;
    size_t            SizeInBytes = 2;

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, CFE_PSP_INVALID_MEM_TYPE);

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_FILL);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_INVALID_MEM_TYPE, "Result == CFE_PSP_INVALID_MEM_TYPE");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_OS_MEMVALIDATE_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "CFE_PSP_MemValidateRange error received: RC = 0x%08X "
                         "Addr = %p Size = %u MemType = %s");
}

void Test_MM_VerifyLoadDumpParams_FillMEM16DataSizeErrorTooSmall(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_MEM16;
    size_t            SizeInBytes = 0;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_FILL);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR, "Result == CFE_PSP_ERROR");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_DATA_SIZE_BYTES_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Data size in bytes invalid or exceeds limits: Data Size = %u");
}

void Test_MM_VerifyLoadDumpParams_FillMEM16DataSizeErrorTooLarge(void)
{
    int32             Result;
    uint32            Address     = 0;
    MM_MemType_Enum_t MemType     = MM_MemType_MEM16;
    size_t            SizeInBytes = MM_INTERNAL_MAX_FILL_DATA_MEM16 + 2;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_FILL);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR, "Result == CFE_PSP_ERROR");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_DATA_SIZE_BYTES_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Data size in bytes invalid or exceeds limits: Data Size = %u");
}

void Test_MM_VerifyLoadDumpParams_FillMEM16AlignmentError(void)
{
    int32             Result;
    uint32            Address     = 1;
    MM_MemType_Enum_t MemType     = MM_MemType_MEM16;
    size_t            SizeInBytes = 1;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_FILL);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR_ADDRESS_MISALIGNED, "Result == CFE_PSP_ERROR_ADDRESS_MISALIGNED");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_ALIGN16_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Data and address not 16 bit aligned: Addr = %p Size = %u");
}

void Test_MM_VerifyLoadDumpParams_FillMEM8ValidateRangeError(void)
{
    int32             Result;
    uint32            Address     = 1;
    MM_MemType_Enum_t MemType     = MM_MemType_MEM8;
    size_t            SizeInBytes = 1;

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, CFE_PSP_INVALID_MEM_TYPE);

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_FILL);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_INVALID_MEM_TYPE, "Result == CFE_PSP_INVALID_MEM_TYPE");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_OS_MEMVALIDATE_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "CFE_PSP_MemValidateRange error received: RC = 0x%08X "
                         "Addr = %p Size = %u MemType = %s");
}

void Test_MM_VerifyLoadDumpParams_FillMEM8DataSizeErrorTooSmall(void)
{
    int32             Result;
    uint32            Address     = 1;
    MM_MemType_Enum_t MemType     = MM_MemType_MEM8;
    size_t            SizeInBytes = 0;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_FILL);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR, "Result == CFE_PSP_ERROR");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_DATA_SIZE_BYTES_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Data size in bytes invalid or exceeds limits: Data Size = %u");
}

void Test_MM_VerifyLoadDumpParams_FillMEM8DataSizeErrorTooLarge(void)
{
    int32             Result;
    uint32            Address     = 1;
    MM_MemType_Enum_t MemType     = MM_MemType_MEM8;
    size_t            SizeInBytes = MM_INTERNAL_MAX_FILL_DATA_MEM8 + 1;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_FILL);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR, "Result == CFE_PSP_ERROR");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_DATA_SIZE_BYTES_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Data size in bytes invalid or exceeds limits: Data Size = %u");
}

void Test_MM_VerifyLoadDumpParams_FillInvalidMemTypeError(void)
{
    int32             Result;
    uint32            Address     = 1;
    MM_MemType_Enum_t MemType     = 99;
    size_t            SizeInBytes = 1;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MemType, SizeInBytes, MM_VERIFY_FILL);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR, "Result == CFE_PSP_ERROR");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0, MM_MEMTYPE_ERR_EID, CFE_EVS_EventType_ERROR, "Invalid memory type specified: MemType = %d");
}

/* WID */
void Test_MM_VerifyLoadDumpParams_WIDNominal(void)
{
    uint32 Address     = 0;
    size_t SizeInBytes = 1;
    int32  Result;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MM_MemType_RAM, SizeInBytes, MM_VERIFY_WID);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_SUCCESS, "Result == CFE_PSP_SUCCESS");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_VerifyLoadDumpParams_WIDMemValidateError(void)
{
    uint32 Address     = 0;
    size_t SizeInBytes = 1;
    int32  Result;

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemValidateRange), 1, CFE_PSP_INVALID_MEM_TYPE);

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MM_MemType_RAM, SizeInBytes, MM_VERIFY_WID);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_INVALID_MEM_TYPE, "Result == CFE_PSP_INVALID_MEM_TYPE");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_OS_MEMVALIDATE_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "CFE_PSP_MemValidateRange error received: RC = 0x%08X "
                         "Addr = %p Size = %u MemType = %s");
}

void Test_MM_VerifyLoadDumpParams_WIDDataSizeErrorTooSmall(void)
{
    uint32 Address     = 0;
    size_t SizeInBytes = 0;
    int32  Result;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MM_MemType_RAM, SizeInBytes, MM_VERIFY_WID);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR, "Result == CFE_PSP_ERROR");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_DATA_SIZE_BYTES_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Data size in bytes invalid or exceeds limits: Data Size = %u");
}

void Test_MM_VerifyLoadDumpParams_WIDDataSizeErrorTooLarge(void)
{
    uint32 Address     = 0;
    size_t SizeInBytes = MM_INTERFACE_MAX_UNINTERRUPTIBLE_DATA + 1;
    int32  Result;

    /* Execute the function being tested */
    Result = MM_VerifyLoadDumpParams(Address, MM_MemType_RAM, SizeInBytes, MM_VERIFY_WID);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR, "Result == CFE_PSP_ERROR");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_DATA_SIZE_BYTES_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Data size in bytes invalid or exceeds limits: Data Size = %u");
}

void Test_MM_Verify32Aligned(void)
{
    bool    Result;
    cpuaddr Addr;
    size_t  Size;

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
}

void Test_MM_Verify16Aligned(void)
{
    bool    Result;
    cpuaddr Addr;
    size_t  Size;

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
}

void Test_MM_ResolveSymAddr_Nominal(void)
{
    MM_SymAddr_t SymAddr;
    cpuaddr      OldResolvedAddr;
    cpuaddr      ResolvedAddr;
    int32        Result;

    memset(&SymAddr, 0, sizeof(MM_SymAddr_t));
    strncpy(SymAddr.SymName, "symname", sizeof(SymAddr.SymName));
    SymAddr.Offset  = CFE_ES_MEMADDRESS_C(22);
    OldResolvedAddr = 20;
    ResolvedAddr    = OldResolvedAddr;

    UT_SetDataBuffer(UT_KEY(OS_SymbolLookup), &ResolvedAddr, sizeof(ResolvedAddr), false);
    UT_SetDefaultReturnValue(UT_KEY(OS_SymbolLookup), OS_SUCCESS);

    /* Execute the function being tested */
    Result = MM_ResolveSymAddr(&SymAddr, &ResolvedAddr);

    /* Verify results */
    UtAssert_INT32_EQ(Result, OS_SUCCESS);
    OldResolvedAddr += (cpuaddr)CFE_ES_MEMADDRESS_TO_PTR(SymAddr.Offset);
    UtAssert_ADDRESS_EQ(ResolvedAddr, OldResolvedAddr);
}

void Test_MM_ResolveSymAddr_NullString(void)
{
    MM_SymAddr_t SymAddr;
    cpuaddr      ResolvedAddr;
    int32        Result;

    memset(&SymAddr, 0, sizeof(MM_SymAddr_t));
    SymAddr.Offset = CFE_ES_MEMADDRESS_C(99);
    ResolvedAddr   = 0;

    /* Execute the function being tested */
    Result = MM_ResolveSymAddr(&SymAddr, &ResolvedAddr);

    /* Verify results */
    UtAssert_INT32_EQ(Result, OS_ERROR_NAME_LENGTH);
    UtAssert_ADDRESS_EQ(ResolvedAddr, CFE_ES_MEMADDRESS_TO_PTR(SymAddr.Offset));
}

void Test_MM_ResolveSymAddr_SymLookupErr(void)
{
    MM_SymAddr_t SymAddr;
    cpuaddr      ResolvedAddr;
    int32        Result;

    UT_SetDefaultReturnValue(UT_KEY(OS_SymbolLookup), OS_ERROR);

    /* Execute the function being tested */
    Result = MM_ResolveSymAddr(&SymAddr, &ResolvedAddr);

    /* Verify results */
    UtAssert_INT32_EQ(Result, OS_ERROR);
}

void Test_MM_ComputeCRCFromFile(void)
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
    UT_SetDeferredRetcode(UT_KEY(OS_read), 1, OS_ERROR);

    UT_SetDefaultReturnValue(UT_KEY(CFE_ES_CalculateCRC), 1);

    /* Execute the function being tested */
    Result = MM_ComputeCRCFromFile(FileHandle, &CrcPtr, TypeCRC);

    /* Verify results */
    UtAssert_True(Result == OS_ERROR, "Result == OS_ERROR");
}

/*
 * Register the test cases to execute with the unit test tool
 */
void UtTest_Setup(void)
{
    ADD_TEST(Test_MM_ResetHk);
    ADD_TEST(Test_MM_SegmentBreak_Nominal);
    ADD_TEST(Test_MM_VerifyPeekPokeParams_ByteWidthRAM);
    ADD_TEST(Test_MM_VerifyPeekPokeParams_WordWidthMEM16);
    ADD_TEST(Test_MM_VerifyPeekPokeParams_DWordWidthMEM32);
    ADD_TEST(Test_MM_VerifyPeekPokeParams_WordWidthAlignmentError);
    ADD_TEST(Test_MM_VerifyPeekPokeParams_DWordWidthAlignmentError);
    ADD_TEST(Test_MM_VerifyPeekPokeParams_InvalidDataSize);
    ADD_TEST(Test_MM_VerifyPeekPokeParams_EEPROM);
    ADD_TEST(Test_MM_VerifyPeekPokeParams_MEM8);
    ADD_TEST(Test_MM_VerifyPeekPokeParams_RAMValidateRangeError);
    ADD_TEST(Test_MM_VerifyPeekPokeParams_EEPROMValidateRangeError);
    ADD_TEST(Test_MM_VerifyPeekPokeParams_MEM32ValidateRangeError);
    ADD_TEST(Test_MM_VerifyPeekPokeParams_MEM16ValidateRangeError);
    ADD_TEST(Test_MM_VerifyPeekPokeParams_MEM8ValidateRangeError);
    ADD_TEST(Test_MM_VerifyPeekPokeParams_MEM32InvalidDataSize);
    ADD_TEST(Test_MM_VerifyPeekPokeParams_MEM16InvalidDataSize);
    ADD_TEST(Test_MM_VerifyPeekPokeParams_MEM8InvalidDataSize);
    ADD_TEST(Test_MM_VerifyPeekPokeParams_InvalidMemType);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_LoadRAMValidateRangeError);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_LoadRAMDataSizeErrorTooSmall);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_LoadRAMDataSizeErrorTooLarge);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_LoadEEPROMValidateRangeError);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_LoadEEPROMDataSizeErrorTooSmall);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_LoadEEPROMDataSizeErrorTooLarge);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_LoadMEM32ValidateRangeError);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_LoadMEM32DataSizeErrorTooSmall);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_LoadMEM32DataSizeErrorTooLarge);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_LoadMEM32AlignmentError);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_LoadMEM16ValidateRangeError);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_LoadMEM16DataSizeErrorTooSmall);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_LoadMEM16DataSizeErrorTooLarge);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_LoadMEM16AlignmentError);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_LoadMEM8ValidateRangeError);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_LoadMEM8DataSizeErrorTooSmall);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_LoadMEM8DataSizeErrorTooLarge);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_LoadInvalidMemTypeError);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_LoadInvalidVerifyTypeError);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_DumpRAM);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_DumpEEPROM);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_DumpMEM32);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_DumpMEM16);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_DumpMEM8);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_DumpRAMRangeError);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_DumpRAMInvalidSizeTooSmall);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_DumpRAMInvalidSizeTooLarge);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_DumpEEPROMRangeError);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_DumpEEPROMInvalidSizeTooSmall);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_DumpEEPROMInvalidSizeTooLarge);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_DumpMEM32RangeError);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_DumpMEM32InvalidSizeTooSmall);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_DumpMEM32InvalidSizeTooLarge);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_DumpMEM32AlignmentError);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_DumpMEM16RangeError);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_DumpMEM16InvalidSizeTooSmall);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_DumpMEM16InvalidSizeTooLarge);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_DumpMEM16AlignmentError);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_DumpMEM8RangeError);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_DumpMEM8InvalidSizeTooSmall);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_DumpMEM8InvalidSizeTooLarge);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_DumpInvalidMemoryType);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_DumpEventRAM);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_DumpEventEEPROM);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_DumpEventMEM32);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_DumpEventMEM16);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_DumpEventMEM8);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_DumpEventInvalidDataSizeTooSmall);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_DumpEventInvalidDataSizeTooLarge);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_DumpEventRAMRangeError);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_DumpEventEEPROMRangeError);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_DumpEventMEM32RangeError);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_DumpEventMEM32AlignmentError);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_DumpEventMEM16RangeError);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_DumpEventMEM16AlignmentError);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_DumpEventMEM8RangeError);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_DumpEventInvalidMemType);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_FillRAMValidateRangeError);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_FillRAMDataSizeErrorTooSmall);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_FillRAMDataSizeErrorTooLarge);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_FillEEPROMValidateRangeError);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_FillEEPROMDataSizeErrorTooSmall);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_FillEEPROMDataSizeErrorTooLarge);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_FillMEM32ValidateRangeError);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_FillMEM32DataSizeErrorTooSmall);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_FillMEM32DataSizeErrorTooLarge);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_FillMEM32AlignmentError);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_FillMEM16ValidateRangeError);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_FillMEM16DataSizeErrorTooSmall);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_FillMEM16DataSizeErrorTooLarge);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_FillMEM16AlignmentError);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_FillMEM8ValidateRangeError);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_FillMEM8DataSizeErrorTooSmall);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_FillMEM8DataSizeErrorTooLarge);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_FillInvalidMemTypeError);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_WIDNominal);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_WIDMemValidateError);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_WIDDataSizeErrorTooSmall);
    ADD_TEST(Test_MM_VerifyLoadDumpParams_WIDDataSizeErrorTooLarge);
    ADD_TEST(Test_MM_Verify32Aligned);
    ADD_TEST(Test_MM_Verify16Aligned);
    ADD_TEST(Test_MM_ResolveSymAddr_Nominal);
    ADD_TEST(Test_MM_ResolveSymAddr_NullString);
    ADD_TEST(Test_MM_ResolveSymAddr_SymLookupErr);
    ADD_TEST(Test_MM_ComputeCRCFromFile);
}
