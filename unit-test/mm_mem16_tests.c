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
 *   Unit tests for mm_mem16.c
 */

/************************************************************************
** Includes
*************************************************************************/
#include "mm_mem16.h"
#include "mm_msg.h"
#include "mm_msgdefs.h"
#include "mm_msgids.h"
#include "mm_events.h"
#include "mm_filedefs.h"
#include "mm_version.h"
#include "mm_test_utils.h"

/************************************************************************
** UT Includes
*************************************************************************/
#include "utassert.h"
#include "utstubs.h"
#include "uttest.h"

#include <unistd.h>
#include <stdlib.h>
#include "cfe.h"
#include "cfe_msgids.h"

/* mm_mem16_tests globals */
uint8 call_count_CFE_EVS_SendEvent;

/*
 * Function Definitions
 */

void MM_LoadMem16FromFile_Test_Nominal(void)
{
    bool                    Result;
    uint32                  DestAddress = 1;
    osal_id_t               FileHandle  = MM_UT_OBJID_1;
    MM_LoadDumpFileHeader_t FileHeader;

    FileHeader.NumOfBytes = 2;

    /* Set to fail condition "(ReadLength = OS_read(FileHandle, ioBuffer8, SegmentSize)) != SegmentSize" */
    UT_SetDeferredRetcode(UT_KEY(OS_write), 1, FileHeader.NumOfBytes);

    /* Execute the function being tested */
    Result = MM_LoadMem16FromFile(FileHandle, (char *)"filename", &FileHeader, DestAddress);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");
    UtAssert_True(MM_AppData.HkPacket.LastAction == MM_LOAD_FROM_FILE,
                  "MM_AppData.HkPacket.LastAction == MM_LOAD_FROM_FILE");
    UtAssert_True(MM_AppData.HkPacket.MemType == MM_MEM16, "MM_AppData.HkPacket.MemType == MM_MEM16");
    UtAssert_True(MM_AppData.HkPacket.Address == DestAddress, "MM_AppData.HkPacket.Address == DestAddress");
    UtAssert_True(MM_AppData.HkPacket.BytesProcessed == FileHeader.NumOfBytes,
                  "MM_AppData.HkPacket.BytesProcessed == FileHeader.NumOfBytes");
    UtAssert_True(strncmp(MM_AppData.HkPacket.FileName, "filename", OS_MAX_PATH_LEN) == 0,
                  "MM_AppData.HkPacket.FileName, 'filename', OS_MAX_PATH_LEN) == 0");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_LoadMem16FromFile_Test_CPUHogging(void)
{
    bool                    Result;
    uint32                  DestAddress = 1;
    osal_id_t               FileHandle  = MM_UT_OBJID_1;
    MM_LoadDumpFileHeader_t FileHeader;

    FileHeader.NumOfBytes = 2 * MM_MAX_LOAD_DATA_SEG;

    /* Set to always fail condition "(ReadLength = OS_read(FileHandle, ioBuffer8, SegmentSize)) != SegmentSize" */
    UT_SetDefaultReturnValue(UT_KEY(OS_write), FileHeader.NumOfBytes);

    /* Execute the function being tested */
    Result = MM_LoadMem16FromFile(FileHandle, (char *)"filename", &FileHeader, DestAddress);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");
    UtAssert_True(MM_AppData.HkPacket.LastAction == MM_LOAD_FROM_FILE,
                  "MM_AppData.HkPacket.LastAction == MM_LOAD_FROM_FILE");
    UtAssert_True(MM_AppData.HkPacket.MemType == MM_MEM16, "MM_AppData.HkPacket.MemType == MM_MEM16");
    UtAssert_True(MM_AppData.HkPacket.Address == DestAddress, "MM_AppData.HkPacket.Address == DestAddress");
    UtAssert_True(MM_AppData.HkPacket.BytesProcessed == FileHeader.NumOfBytes,
                  "MM_AppData.HkPacket.BytesProcessed == FileHeader.NumOfBytes");
    UtAssert_True(strncmp(MM_AppData.HkPacket.FileName, "filename", OS_MAX_PATH_LEN) == 0,
                  "MM_AppData.HkPacket.FileName, 'filename', OS_MAX_PATH_LEN) == 0");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_LoadMem16FromFile_Test_ReadError(void)
{
    bool                    Result;
    uint32                  DestAddress = 1;
    osal_id_t               FileHandle  = MM_UT_OBJID_1;
    MM_LoadDumpFileHeader_t FileHeader;
    int32                   strCmpResult;
    char                    ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "OS_read error received: RC = 0x%%08X Expected = %%u File = '%%s'");

    FileHeader.NumOfBytes = 2;

    /* Set to generate error message MM_OS_READ_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(OS_read), 1, -1);

    /* Execute the function being tested */
    Result = MM_LoadMem16FromFile(FileHandle, (char *)"filename", &FileHeader, DestAddress);

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

void MM_LoadMem16FromFile_Test_WriteError(void)
{
    bool Result;
    /* CFE_PSP_MemWrite16 stub returns success with non-zero address */
    uint32                  DestAddress = 0;
    osal_id_t               FileHandle  = MM_UT_OBJID_1;
    MM_LoadDumpFileHeader_t FileHeader;
    int32                   strCmpResult;
    char                    ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    memset(&FileHeader, 0, sizeof(FileHeader));

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "PSP write memory error: RC=0x%%08X, Address=%%p, MemType=MEM16");

    FileHeader.NumOfBytes = 2;

    /* Set to fail condition "(ReadLength = OS_read(FileHandle, ioBuffer8, SegmentSize)) != SegmentSize" */
    UT_SetDeferredRetcode(UT_KEY(OS_read), 1, FileHeader.NumOfBytes);

    /* Set to generate error message MM_PSP_WRITE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemWrite16), 1, -1);

    /* Execute the function being tested */
    Result = MM_LoadMem16FromFile(FileHandle, (char *)"filename", &FileHeader, DestAddress);

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

void MM_DumpMem16ToFile_Test_Nominal(void)
{
    bool                    Result;
    osal_id_t               FileHandle = MM_UT_OBJID_1;
    MM_LoadDumpFileHeader_t FileHeader;

    FileHeader.NumOfBytes = MM_MAX_DUMP_DATA_SEG;
    /* CFE_PSP_MemRead16 stub returns success with non-zero address */
    FileHeader.SymAddress.Offset = 1;

    /* Execute the function being tested */
    Result = MM_DumpMem16ToFile(FileHandle, (char *)"filename", &FileHeader);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");
    UtAssert_True(MM_AppData.HkPacket.LastAction == MM_DUMP_TO_FILE,
                  "MM_AppData.HkPacket.LastAction == MM_DUMP_TO_FILE");
    UtAssert_True(MM_AppData.HkPacket.MemType == MM_MEM16, "MM_AppData.HkPacket.MemType == MM_MEM16");
    UtAssert_True(MM_AppData.HkPacket.Address == FileHeader.SymAddress.Offset,
                  "MM_AppData.HkPacket.Address == FileHeader.SymAddress.Offset");
    UtAssert_True(MM_AppData.HkPacket.BytesProcessed == FileHeader.NumOfBytes,
                  "MM_AppData.HkPacket.BytesProcessed == FileHeader.NumOfBytes");
    UtAssert_True(strncmp(MM_AppData.HkPacket.FileName, "filename", OS_MAX_PATH_LEN) == 0,
                  "MM_AppData.HkPacket.FileName, 'filename', OS_MAX_PATH_LEN) == 0");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_DumpMem16ToFile_Test_CPUHogging(void)
{
    bool                    Result;
    osal_id_t               FileHandle = MM_UT_OBJID_1;
    MM_LoadDumpFileHeader_t FileHeader;

    FileHeader.NumOfBytes = 2 * MM_MAX_LOAD_DATA_SEG;
    /* CFE_PSP_MemRead16 stub returns success with non-zero address */
    FileHeader.SymAddress.Offset = 1;

    /* Execute the function being tested */
    Result = MM_DumpMem16ToFile(FileHandle, (char *)"filename", &FileHeader);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");
    UtAssert_True(MM_AppData.HkPacket.LastAction == MM_DUMP_TO_FILE,
                  "MM_AppData.HkPacket.LastAction == MM_DUMP_TO_FILE");
    UtAssert_True(MM_AppData.HkPacket.MemType == MM_MEM16, "MM_AppData.HkPacket.MemType == MM_MEM16");
    UtAssert_True(MM_AppData.HkPacket.Address == FileHeader.SymAddress.Offset,
                  "MM_AppData.HkPacket.Address == FileHeader.SymAddress.Offset");
    UtAssert_True(MM_AppData.HkPacket.BytesProcessed == FileHeader.NumOfBytes,
                  "MM_AppData.HkPacket.BytesProcessed == FileHeader.NumOfBytes");
    UtAssert_True(strncmp(MM_AppData.HkPacket.FileName, "filename", OS_MAX_PATH_LEN) == 0,
                  "MM_AppData.HkPacket.FileName, 'filename', OS_MAX_PATH_LEN) == 0");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_DumpMem16ToFile_Test_ReadError(void)
{
    bool                    Result;
    osal_id_t               FileHandle = MM_UT_OBJID_1;
    MM_LoadDumpFileHeader_t FileHeader;
    int32                   strCmpResult;
    char                    ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "PSP read memory error: RC=0x%%08X, Src=%%p, Tgt=%%p, Type=MEM16");

    FileHeader.NumOfBytes = 2;

    /* CFE_PSP_MemRead16 stub returns success with non-zero address */
    FileHeader.SymAddress.Offset = 0;

    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemRead16), 1, -1);

    /* Execute the function being tested */
    Result = MM_DumpMem16ToFile(FileHandle, (char *)"filename", &FileHeader);

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
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_DumpMem16ToFile_Test_WriteError(void)
{
    bool                    Result;
    osal_id_t               FileHandle = MM_UT_OBJID_1;
    MM_LoadDumpFileHeader_t FileHeader;
    int32                   strCmpResult;
    char                    ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    memset(&FileHeader, 0, sizeof(FileHeader));

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "OS_write error received: RC = 0x%%08X Expected = %%u File = '%%s'");

    FileHeader.NumOfBytes = 2;

    /* Set to generate error message MM_OS_WRITE_EXP_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(OS_write), 1, -1);

    /* Execute the function being tested */
    Result = MM_DumpMem16ToFile(FileHandle, (char *)"filename", &FileHeader);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_OS_WRITE_EXP_ERR_EID);
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

void MM_FillMem16_Test_Nominal(void)
{
    MM_FillMemCmd_t CmdPacket;
    uint32          DestAddress = 1;
    bool            Result;
    CmdPacket.NumOfBytes  = 2;
    CmdPacket.FillPattern = 3;

    /* Execute the function being tested */
    Result = MM_FillMem16(DestAddress, &CmdPacket);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");
    UtAssert_True(MM_AppData.HkPacket.LastAction == MM_FILL, "MM_AppData.HkPacket.LastAction == MM_FILL");
    UtAssert_True(MM_AppData.HkPacket.MemType == MM_MEM16, "MM_AppData.HkPacket.MemType == MM_MEM16");
    UtAssert_True(MM_AppData.HkPacket.Address == DestAddress, "MM_AppData.HkPacket.Address == DestAddress");
    UtAssert_True(MM_AppData.HkPacket.DataValue == CmdPacket.FillPattern,
                  "MM_AppData.HkPacket.DataValue == CmdPacket.FillPattern");
    UtAssert_True(MM_AppData.HkPacket.BytesProcessed == CmdPacket.NumOfBytes,
                  "MM_AppData.HkPacket.BytesProcessed == CmdPacket.NumOfBytes");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_FillMem16_Test_CPUHogging(void)
{
    MM_FillMemCmd_t CmdPacket;
    uint32          DestAddress = 1;
    bool            Result;

    CmdPacket.NumOfBytes  = 2 * MM_MAX_LOAD_DATA_SEG;
    CmdPacket.FillPattern = 3;

    /* Execute the function being tested */
    Result = MM_FillMem16(DestAddress, &CmdPacket);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");
    UtAssert_True(MM_AppData.HkPacket.LastAction == MM_FILL, "MM_AppData.HkPacket.LastAction == MM_FILL");
    UtAssert_True(MM_AppData.HkPacket.MemType == MM_MEM16, "MM_AppData.HkPacket.MemType == MM_MEM16");
    UtAssert_True(MM_AppData.HkPacket.Address == DestAddress, "MM_AppData.HkPacket.Address == DestAddress");
    UtAssert_True(MM_AppData.HkPacket.DataValue == CmdPacket.FillPattern,
                  "MM_AppData.HkPacket.DataValue == CmdPacket.FillPattern");
    UtAssert_True(MM_AppData.HkPacket.BytesProcessed == CmdPacket.NumOfBytes,
                  "MM_AppData.HkPacket.BytesProcessed == CmdPacket.NumOfBytes");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* No command-handling function should be updating the cmd or err counter itself */
    UtAssert_INT32_EQ(MM_AppData.HkPacket.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkPacket.ErrCounter, 0);
}

void MM_FillMem16_Test_WriteError(void)
{
    MM_FillMemCmd_t CmdPacket;
    /* CFE_PSP_MemWrite16 stub returns success with non-zero address */
    uint32 DestAddress = 0;
    int32  strCmpResult;
    char   ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool   Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "PSP write memory error: RC=0x%%08X, Address=%%p, MemType=MEM16");

    CmdPacket.NumOfBytes  = 2;
    CmdPacket.FillPattern = 3;

    /* Set to generate error message MM_PSP_WRITE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemWrite16), 1, -1);

    /* Execute the function being tested */
    Result = MM_FillMem16(DestAddress, &CmdPacket);

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

void MM_FillMem16_Test_Align(void)
{
    MM_FillMemCmd_t CmdPacket;
    uint32          DestAddress = 1;
    CmdPacket.NumOfBytes        = 3;
    CmdPacket.FillPattern       = 3;
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    bool  Result;

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "MM_FillMem16 NumOfBytes not multiple of 2. Reducing from %%d to %%d.");

    /* Execute the function being tested */
    Result = MM_FillMem16(DestAddress, &CmdPacket);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, MM_FILL_MEM16_ALIGN_WARN_INF_EID);
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

/*
 * Register the test cases to execute with the unit test tool
 */
void UtTest_Setup(void)
{
    UtTest_Add(MM_LoadMem16FromFile_Test_Nominal, MM_Test_Setup, MM_Test_TearDown, "MM_LoadMem16FromFile_Test_Nominal");
    UtTest_Add(MM_LoadMem16FromFile_Test_CPUHogging, MM_Test_Setup, MM_Test_TearDown,
               "MM_LoadMem16FromFile_Test_CPUHogging");
    UtTest_Add(MM_LoadMem16FromFile_Test_ReadError, MM_Test_Setup, MM_Test_TearDown,
               "MM_LoadMem16FromFile_Test_ReadError");
    UtTest_Add(MM_LoadMem16FromFile_Test_WriteError, MM_Test_Setup, MM_Test_TearDown,
               "MM_LoadMem16FromFile_Test_WriteError");
    UtTest_Add(MM_DumpMem16ToFile_Test_Nominal, MM_Test_Setup, MM_Test_TearDown, "MM_DumpMem16ToFile_Test_Nominal");
    UtTest_Add(MM_DumpMem16ToFile_Test_CPUHogging, MM_Test_Setup, MM_Test_TearDown,
               "MM_DumpMem16ToFile_Test_CPUHogging");
    UtTest_Add(MM_DumpMem16ToFile_Test_ReadError, MM_Test_Setup, MM_Test_TearDown, "MM_DumpMem16ToFile_Test_ReadError");
    UtTest_Add(MM_DumpMem16ToFile_Test_WriteError, MM_Test_Setup, MM_Test_TearDown,
               "MM_DumpMem16ToFile_Test_WriteError");
    UtTest_Add(MM_FillMem16_Test_Nominal, MM_Test_Setup, MM_Test_TearDown, "MM_FillMem16_Test_Nominal");
    UtTest_Add(MM_FillMem16_Test_CPUHogging, MM_Test_Setup, MM_Test_TearDown, "MM_FillMem16_Test_CPUHogging");
    UtTest_Add(MM_FillMem16_Test_WriteError, MM_Test_Setup, MM_Test_TearDown, "MM_FillMem16_Test_WriteError");
    UtTest_Add(MM_FillMem16_Test_Align, MM_Test_Setup, MM_Test_TearDown, "MM_FillMem16_Test_Align");
}
