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
 *   Unit tests for mm_mem16.c
 */

/************************************************************************
** Includes
*************************************************************************/
#include "mm_eventids.h"
#include "mm_filedefs.h"
#include "mm_mem16.h"
#include "mm_msg.h"
#include "mm_msgdefs.h"
#include "mm_msgids.h"
#include "mm_test_utils.h"
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

void Test_MM_LoadMem16FromFile_Nominal(void)
{
    CFE_Status_t            Result;
    cpuaddr                 DestAddress = 1;
    osal_id_t               FileHandle  = MM_UT_OBJID_1;
    MM_LoadDumpFileHeader_t FileHeader;

    FileHeader.NumOfBytes = 2;

    /* Set to fail condition "(ReadLength = OS_read(FileHandle, ioBuffer8,
     * SegmentSize)) != SegmentSize" */
    UT_SetDeferredRetcode(UT_KEY(OS_write), 1, FileHeader.NumOfBytes);

    /* Execute the function being tested */
    Result = MM_LoadMem16FromFile(FileHandle, (char *)"filename", &FileHeader, DestAddress);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_SUCCESS, "Result == CFE_PSP_SUCCESS");
    UtAssert_True(MM_AppData.HkTlm.Payload.LastAction == MM_LastAction_LOAD_FROM_FILE,
                  "MM_AppData.HkTlm.Payload.LastAction == MM_LastAction_LOAD_FROM_FILE");
    UtAssert_True(MM_AppData.HkTlm.Payload.MemType == MM_MemType_MEM16,
                  "MM_AppData.HkTlm.Payload.MemType == MM_MemType_MEM16");
    UtAssert_ADDRESS_EQ(CFE_ES_MEMADDRESS_TO_PTR(MM_AppData.HkTlm.Payload.Address), DestAddress);
    UtAssert_True(MM_AppData.HkTlm.Payload.BytesProcessed == FileHeader.NumOfBytes,
                  "MM_AppData.HkTlm.Payload.BytesProcessed == FileHeader.NumOfBytes");
    UtAssert_True(strncmp(MM_AppData.HkTlm.Payload.FileName, "filename", CFE_MISSION_MAX_PATH_LEN) == 0,
                  "MM_AppData.HkTlm.Payload.FileName, 'filename', "
                  "CFE_MISSION_MAX_PATH_LEN) == 0");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_LoadMem16FromFile_CPUHogging(void)
{
    CFE_Status_t            Result;
    cpuaddr                 DestAddress = 1;
    osal_id_t               FileHandle  = MM_UT_OBJID_1;
    MM_LoadDumpFileHeader_t FileHeader;

    FileHeader.NumOfBytes = 2 * MM_INTERNAL_MAX_LOAD_DATA_SEG;

    /* Set to always fail condition "(ReadLength = OS_read(FileHandle, ioBuffer8,
     * SegmentSize)) != SegmentSize" */
    UT_SetDefaultReturnValue(UT_KEY(OS_write), FileHeader.NumOfBytes);

    /* Execute the function being tested */
    Result = MM_LoadMem16FromFile(FileHandle, (char *)"filename", &FileHeader, DestAddress);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_SUCCESS, "Result == CFE_PSP_SUCCESS");
    UtAssert_True(MM_AppData.HkTlm.Payload.LastAction == MM_LastAction_LOAD_FROM_FILE,
                  "MM_AppData.HkTlm.Payload.LastAction == MM_LastAction_LOAD_FROM_FILE");
    UtAssert_True(MM_AppData.HkTlm.Payload.MemType == MM_MemType_MEM16,
                  "MM_AppData.HkTlm.Payload.MemType == MM_MemType_MEM16");
    UtAssert_ADDRESS_EQ(CFE_ES_MEMADDRESS_TO_PTR(MM_AppData.HkTlm.Payload.Address), DestAddress);
    UtAssert_True(MM_AppData.HkTlm.Payload.BytesProcessed == FileHeader.NumOfBytes,
                  "MM_AppData.HkTlm.Payload.BytesProcessed == FileHeader.NumOfBytes");
    UtAssert_True(strncmp(MM_AppData.HkTlm.Payload.FileName, "filename", CFE_MISSION_MAX_PATH_LEN) == 0,
                  "MM_AppData.HkTlm.Payload.FileName, 'filename', "
                  "CFE_MISSION_MAX_PATH_LEN) == 0");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_LoadMem16FromFile_ReadError(void)
{
    CFE_Status_t            Result;
    cpuaddr                 DestAddress = 1;
    osal_id_t               FileHandle  = MM_UT_OBJID_1;
    MM_LoadDumpFileHeader_t FileHeader;

    FileHeader.NumOfBytes = 2;

    /* Set to generate error message MM_OS_READ_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(OS_read), 1, OS_ERROR);

    /* Execute the function being tested */
    Result = MM_LoadMem16FromFile(FileHandle, (char *)"filename", &FileHeader, DestAddress);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR, "Result == CFE_PSP_ERROR");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_OS_READ_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "OS_read error received: RC = 0x%08X Expected = %u File = '%s'");
}

void Test_MM_LoadMem16FromFile_WriteError(void)
{
    /* CFE_PSP_MemWrite16 stub returns success with non-zero address */
    CFE_Status_t            Result;
    cpuaddr                 DestAddress = 0;
    osal_id_t               FileHandle  = MM_UT_OBJID_1;
    MM_LoadDumpFileHeader_t FileHeader;

    memset(&FileHeader, 0, sizeof(FileHeader));

    FileHeader.NumOfBytes = 2;

    /* Set to fail condition "(ReadLength = OS_read(FileHandle, ioBuffer8,
     * SegmentSize)) != SegmentSize" */
    UT_SetDeferredRetcode(UT_KEY(OS_read), 1, FileHeader.NumOfBytes);

    /* Set to generate error message MM_PSP_WRITE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemWrite16), 1, CFE_PSP_ERROR_NOT_IMPLEMENTED);

    /* Execute the function being tested */
    Result = MM_LoadMem16FromFile(FileHandle, (char *)"filename", &FileHeader, DestAddress);

    /* Verify results */
    UtAssert_INT32_EQ(Result, CFE_PSP_ERROR);

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_PSP_WRITE_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "PSP write memory error: RC=0x%08X, Address=%p, MemType=MEM16");
}

void Test_MM_DumpMem16ToFile_Nominal(void)
{
    int32                   Result;
    osal_id_t               FileHandle = MM_UT_OBJID_1;
    MM_LoadDumpFileHeader_t FileHeader;

    FileHeader.NumOfBytes        = MM_INTERNAL_MAX_DUMP_DATA_SEG;
    /* CFE_PSP_MemRead16 stub returns success with non-zero address */
    FileHeader.SymAddress.Offset = CFE_ES_MEMADDRESS_C(1);

    /* Execute the function being tested */
    Result = MM_DumpMem16ToFile(FileHandle, (char *)"filename", &FileHeader);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_SUCCESS, "Result == CFE_PSP_SUCCESS");
    UtAssert_True(MM_AppData.HkTlm.Payload.LastAction == MM_LastAction_DUMP_TO_FILE,
                  "MM_AppData.HkTlm.Payload.LastAction == MM_LastAction_DUMP_TO_FILE");
    UtAssert_True(MM_AppData.HkTlm.Payload.MemType == MM_MemType_MEM16,
                  "MM_AppData.HkTlm.Payload.MemType == MM_MemType_MEM16");
    UtAssert_ADDRESS_EQ(CFE_ES_MEMADDRESS_TO_PTR(MM_AppData.HkTlm.Payload.Address),
                        CFE_ES_MEMADDRESS_TO_PTR(FileHeader.SymAddress.Offset));
    UtAssert_True(MM_AppData.HkTlm.Payload.BytesProcessed == FileHeader.NumOfBytes,
                  "MM_AppData.HkTlm.Payload.BytesProcessed == FileHeader.NumOfBytes");
    UtAssert_True(strncmp(MM_AppData.HkTlm.Payload.FileName, "filename", CFE_MISSION_MAX_PATH_LEN) == 0,
                  "MM_AppData.HkTlm.Payload.FileName, 'filename', "
                  "CFE_MISSION_MAX_PATH_LEN) == 0");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_DumpMem16ToFile_CPUHogging(void)
{
    int32                   Result;
    osal_id_t               FileHandle = MM_UT_OBJID_1;
    MM_LoadDumpFileHeader_t FileHeader;

    FileHeader.NumOfBytes        = 2 * MM_INTERNAL_MAX_LOAD_DATA_SEG;
    /* CFE_PSP_MemRead16 stub returns success with non-zero address */
    FileHeader.SymAddress.Offset = CFE_ES_MEMADDRESS_C(1);

    /* Execute the function being tested */
    Result = MM_DumpMem16ToFile(FileHandle, (char *)"filename", &FileHeader);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_SUCCESS, "Result == CFE_PSP_SUCCESS");
    UtAssert_True(MM_AppData.HkTlm.Payload.LastAction == MM_LastAction_DUMP_TO_FILE,
                  "MM_AppData.HkTlm.Payload.LastAction == MM_LastAction_DUMP_TO_FILE");
    UtAssert_True(MM_AppData.HkTlm.Payload.MemType == MM_MemType_MEM16,
                  "MM_AppData.HkTlm.Payload.MemType == MM_MemType_MEM16");
    UtAssert_ADDRESS_EQ(CFE_ES_MEMADDRESS_TO_PTR(MM_AppData.HkTlm.Payload.Address),
                        CFE_ES_MEMADDRESS_TO_PTR(FileHeader.SymAddress.Offset));
    UtAssert_True(MM_AppData.HkTlm.Payload.BytesProcessed == FileHeader.NumOfBytes,
                  "MM_AppData.HkTlm.Payload.BytesProcessed == FileHeader.NumOfBytes");
    UtAssert_True(strncmp(MM_AppData.HkTlm.Payload.FileName, "filename", CFE_MISSION_MAX_PATH_LEN) == 0,
                  "MM_AppData.HkTlm.Payload.FileName, 'filename', "
                  "CFE_MISSION_MAX_PATH_LEN) == 0");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_DumpMem16ToFile_ReadError(void)
{
    int32                   Result;
    osal_id_t               FileHandle = MM_UT_OBJID_1;
    MM_LoadDumpFileHeader_t FileHeader;

    FileHeader.NumOfBytes = 2;

    /* CFE_PSP_MemRead16 stub returns success with non-zero address */
    FileHeader.SymAddress.Offset = CFE_ES_MEMADDRESS_C(0);

    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemRead16), 1, CFE_PSP_ERROR_NOT_IMPLEMENTED);

    /* Execute the function being tested */
    Result = MM_DumpMem16ToFile(FileHandle, (char *)"filename", &FileHeader);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR_NOT_IMPLEMENTED, "Result == CFE_PSP_ERROR_NOT_IMPLEMENTED");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_PSP_READ_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "PSP read memory error: RC=0x%08X, Src=%p, Tgt=%p, Type=MEM16");
}

void Test_MM_DumpMem16ToFile_WriteError(void)
{
    int32                   Result;
    osal_id_t               FileHandle = MM_UT_OBJID_1;
    MM_LoadDumpFileHeader_t FileHeader;

    memset(&FileHeader, 0, sizeof(FileHeader));

    FileHeader.NumOfBytes = 2;

    /* Set to generate error message MM_OS_WRITE_EXP_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(OS_write), 1, OS_ERROR);

    /* Execute the function being tested */
    Result = MM_DumpMem16ToFile(FileHandle, (char *)"filename", &FileHeader);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR, "Result == CFE_PSP_ERROR");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_OS_WRITE_EXP_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "OS_write error received: RC = 0x%08X Expected = %u File = '%s'");
}

void Test_MM_FillMem16_Nominal(void)
{
    MM_FillMemCmd_t CmdPacket;
    cpuaddr         DestAddress = 1;
    int32           Result;

    CmdPacket.Payload.NumOfBytes  = 2;
    CmdPacket.Payload.FillPattern = 3;

    /* Execute the function being tested */
    Result = MM_FillMem16(DestAddress, &CmdPacket);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_SUCCESS, "Result == CFE_PSP_SUCCESS");
    UtAssert_True(MM_AppData.HkTlm.Payload.LastAction == MM_LastAction_FILL,
                  "MM_AppData.HkTlm.Payload.LastAction == MM_LastAction_FILL");
    UtAssert_True(MM_AppData.HkTlm.Payload.MemType == MM_MemType_MEM16,
                  "MM_AppData.HkTlm.Payload.MemType == MM_MemType_MEM16");
    UtAssert_ADDRESS_EQ(CFE_ES_MEMADDRESS_TO_PTR(MM_AppData.HkTlm.Payload.Address), DestAddress);
    UtAssert_True(MM_AppData.HkTlm.Payload.DataValue == CmdPacket.Payload.FillPattern,
                  "MM_AppData.HkTlm.Payload.DataValue == CmdPacket.Payload.FillPattern");
    UtAssert_True(MM_AppData.HkTlm.Payload.BytesProcessed == CmdPacket.Payload.NumOfBytes,
                  "MM_AppData.HkTlm.Payload.BytesProcessed == "
                  "CmdPacket.Payload.NumOfBytes");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_FillMem16_CPUHogging(void)
{
    MM_FillMemCmd_t CmdPacket;
    cpuaddr         DestAddress = 1;
    int32           Result;

    CmdPacket.Payload.NumOfBytes  = 2 * MM_INTERNAL_MAX_LOAD_DATA_SEG;
    CmdPacket.Payload.FillPattern = 3;

    /* Execute the function being tested */
    Result = MM_FillMem16(DestAddress, &CmdPacket);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_SUCCESS, "Result == CFE_PSP_SUCCESS");
    UtAssert_True(MM_AppData.HkTlm.Payload.LastAction == MM_LastAction_FILL,
                  "MM_AppData.HkTlm.Payload.LastAction == MM_LastAction_FILL");
    UtAssert_True(MM_AppData.HkTlm.Payload.MemType == MM_MemType_MEM16,
                  "MM_AppData.HkTlm.Payload.MemType == MM_MemType_MEM16");
    UtAssert_ADDRESS_EQ(CFE_ES_MEMADDRESS_TO_PTR(MM_AppData.HkTlm.Payload.Address), DestAddress);
    UtAssert_True(MM_AppData.HkTlm.Payload.DataValue == CmdPacket.Payload.FillPattern,
                  "MM_AppData.HkTlm.Payload.DataValue == CmdPacket.Payload.FillPattern");
    UtAssert_True(MM_AppData.HkTlm.Payload.BytesProcessed == CmdPacket.Payload.NumOfBytes,
                  "MM_AppData.HkTlm.Payload.BytesProcessed == "
                  "CmdPacket.Payload.NumOfBytes");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_FillMem16_WriteError(void)
{
    MM_FillMemCmd_t CmdPacket;
    /* CFE_PSP_MemWrite16 stub returns success with non-zero address */
    cpuaddr         DestAddress = 0;
    int32           Result;

    CmdPacket.Payload.NumOfBytes  = 2;
    CmdPacket.Payload.FillPattern = 3;

    /* Set to generate error message MM_PSP_WRITE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemWrite16), 1, CFE_PSP_ERROR_NOT_IMPLEMENTED);

    /* Execute the function being tested */
    Result = MM_FillMem16(DestAddress, &CmdPacket);

    /* Verify results */
    UtAssert_INT32_EQ(Result, CFE_PSP_ERROR);

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_PSP_WRITE_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "PSP write memory error: RC=0x%08X, Address=%p, MemType=MEM16");
}

void Test_MM_FillMem16_Align(void)
{
    MM_FillMemCmd_t CmdPacket;
    cpuaddr         DestAddress = 1;
    int32           Result;

    CmdPacket.Payload.NumOfBytes  = 3;
    CmdPacket.Payload.FillPattern = 4;

    /* Execute the function being tested */
    Result = MM_FillMem16(DestAddress, &CmdPacket);

    /* Verify results */
    UtAssert_INT32_EQ(Result, CFE_PSP_ERROR);

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_FILL_MEM16_ALIGN_WARN_INF_EID,
                         CFE_EVS_EventType_INFORMATION,
                         "MM_FillMem16 NumOfBytes not multiple of 2. Reducing from %u to %u.");
}

/*
 * Register the test cases to execute with the unit test tool
 */
void UtTest_Setup(void)
{
    ADD_TEST(Test_MM_LoadMem16FromFile_Nominal);
    ADD_TEST(Test_MM_LoadMem16FromFile_CPUHogging);
    ADD_TEST(Test_MM_LoadMem16FromFile_ReadError);
    ADD_TEST(Test_MM_LoadMem16FromFile_WriteError);
    ADD_TEST(Test_MM_DumpMem16ToFile_Nominal);
    ADD_TEST(Test_MM_DumpMem16ToFile_CPUHogging);
    ADD_TEST(Test_MM_DumpMem16ToFile_ReadError);
    ADD_TEST(Test_MM_DumpMem16ToFile_WriteError);
    ADD_TEST(Test_MM_FillMem16_Nominal);
    ADD_TEST(Test_MM_FillMem16_CPUHogging);
    ADD_TEST(Test_MM_FillMem16_WriteError);
    ADD_TEST(Test_MM_FillMem16_Align);
}
