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
 *   Unit tests for mm_dump.c
 */

/************************************************************************
** Includes
*************************************************************************/
#include "mm_dump.h"
#include "mm_eventids.h"
#include "mm_filedefs.h"
#include "mm_mem16.h"
#include "mm_mem32.h"
#include "mm_mem8.h"
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

void Test_MM_PeekMem_Byte(void)
{
    MM_PeekCmd_t CmdPacket;
    uint32       SrcAddress = 1;
    int32        Result;

    CmdPacket.Payload.DataSize = MM_INTERNAL_BYTE_BIT_WIDTH;
    CmdPacket.Payload.MemType  = MM_MemType_RAM;

    UT_SetDefaultReturnValue(UT_KEY(CFE_PSP_MemRead8), CFE_PSP_SUCCESS);

    /* Execute the function being tested */
    Result = MM_PeekMem(&CmdPacket, SrcAddress);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_SUCCESS, "Result == CFE_PSP_SUCCESS");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_True(MM_AppData.HkTlm.Payload.LastAction == MM_LastAction_PEEK,
                  "MM_AppData.HkTlm.Payload.LastAction == MM_LastAction_PEEK");
    UtAssert_True(MM_AppData.HkTlm.Payload.MemType == MM_MemType_RAM,
                  "MM_AppData.HkTlm.Payload.MemType == MM_MemType_RAM");
    UtAssert_ADDRESS_EQ(CFE_ES_MEMADDRESS_TO_PTR(MM_AppData.HkTlm.Payload.Address), 1);
    UtAssert_True(MM_AppData.HkTlm.Payload.BytesProcessed == 1, "MM_AppData.HkTlm.Payload.BytesProcessed == 1");
    UtAssert_True(MM_AppData.HkTlm.Payload.DataValue == 1, "MM_AppData.HkTlm.Payload.DataValue == 1");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_PEEK_BYTE_INF_EID,
                         CFE_EVS_EventType_INFORMATION,
                         "Peek Command: Addr = %p Size = %u bits Data = 0x%08X");
}

void Test_MM_PeekMem_ByteError(void)
{
    MM_PeekCmd_t CmdPacket;
    uint32       SrcAddress = 1;
    int32        Result;

    CmdPacket.Payload.DataSize = MM_INTERNAL_BYTE_BIT_WIDTH;

    /* Set to generate error message MM_PSP_READ_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemRead8), 1, CFE_PSP_ERROR_NOT_IMPLEMENTED);

    /* Execute the function being tested */
    Result = MM_PeekMem(&CmdPacket, SrcAddress);

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
                         "PSP read memory error: RC=%d, Address=%p, MemType=MEM%u");
}

void Test_MM_PeekMem_Word(void)
{
    MM_PeekCmd_t CmdPacket;
    uint32       SrcAddress = 1;
    int32        Result;

    CmdPacket.Payload.DataSize = MM_INTERNAL_WORD_BIT_WIDTH;
    CmdPacket.Payload.MemType  = MM_MemType_RAM;

    UT_SetDefaultReturnValue(UT_KEY(CFE_PSP_MemRead16), CFE_PSP_SUCCESS);

    /* Execute the function being tested */
    Result = MM_PeekMem(&CmdPacket, SrcAddress);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_SUCCESS, "Result == CFE_PSP_SUCCESS");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_True(MM_AppData.HkTlm.Payload.LastAction == MM_LastAction_PEEK,
                  "MM_AppData.HkTlm.Payload.LastAction == MM_LastAction_PEEK");
    UtAssert_True(MM_AppData.HkTlm.Payload.MemType == MM_MemType_RAM,
                  "MM_AppData.HkTlm.Payload.MemType == MM_MemType_RAM");
    UtAssert_ADDRESS_EQ(CFE_ES_MEMADDRESS_TO_PTR(MM_AppData.HkTlm.Payload.Address), 1);
    UtAssert_True(MM_AppData.HkTlm.Payload.BytesProcessed == 2, "MM_AppData.HkTlm.Payload.BytesProcessed == 2");
    UtAssert_True(MM_AppData.HkTlm.Payload.DataValue == 0, "MM_AppData.HkTlm.Payload.DataValue == 0");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_PEEK_WORD_INF_EID,
                         CFE_EVS_EventType_INFORMATION,
                         "Peek Command: Addr = %p Size = %u bits Data = 0x%08X");
}

void Test_MM_PeekMem_WordError(void)
{
    MM_PeekCmd_t CmdPacket;
    uint32       SrcAddress = 0;
    int32        Result;

    CmdPacket.Payload.DataSize = MM_INTERNAL_WORD_BIT_WIDTH;

    /* Set to generate error message MM_PSP_READ_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemRead16), 1, CFE_PSP_ERROR_NOT_IMPLEMENTED);

    /* Execute the function being tested */
    Result = MM_PeekMem(&CmdPacket, SrcAddress);

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
                         "PSP read memory error: RC=%d, Address=%p, MemType=MEM%u");
}

void Test_MM_PeekMem_DWord(void)
{
    MM_PeekCmd_t CmdPacket;
    uint32       SrcAddress = 1;
    int32        Result;

    CmdPacket.Payload.DataSize = MM_INTERNAL_DWORD_BIT_WIDTH;
    CmdPacket.Payload.MemType  = MM_MemType_RAM;

    UT_SetDefaultReturnValue(UT_KEY(CFE_PSP_MemRead32), CFE_PSP_SUCCESS);

    /* Execute the function being tested */
    Result = MM_PeekMem(&CmdPacket, SrcAddress);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_SUCCESS, "Result == CFE_PSP_SUCCESS");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_True(MM_AppData.HkTlm.Payload.LastAction == MM_LastAction_PEEK,
                  "MM_AppData.HkTlm.Payload.LastAction == MM_LastAction_PEEK");
    UtAssert_True(MM_AppData.HkTlm.Payload.MemType == MM_MemType_RAM,
                  "MM_AppData.HkTlm.Payload.MemType == MM_MemType_RAM");
    UtAssert_ADDRESS_EQ(CFE_ES_MEMADDRESS_TO_PTR(MM_AppData.HkTlm.Payload.Address), 1);
    UtAssert_True(MM_AppData.HkTlm.Payload.BytesProcessed == 4, "MM_AppData.HkTlm.Payload.BytesProcessed == 4");
    UtAssert_True(MM_AppData.HkTlm.Payload.DataValue == 0, "MM_AppData.HkTlm.Payload.DataValue == 0");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_PEEK_DWORD_INF_EID,
                         CFE_EVS_EventType_INFORMATION,
                         "Peek Command: Addr = %p Size = %u bits Data = 0x%08X");
}

void Test_MM_PeekMem_DWordError(void)
{
    MM_PeekCmd_t CmdPacket;
    uint32       SrcAddress = 0;
    int32        Result;

    CmdPacket.Payload.DataSize = MM_INTERNAL_DWORD_BIT_WIDTH;

    /* Set to generate error message MM_PSP_READ_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemRead32), 1, CFE_PSP_ERROR_NOT_IMPLEMENTED);

    /* Execute the function being tested */
    Result = MM_PeekMem(&CmdPacket, SrcAddress);

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
                         "PSP read memory error: RC=%d, Address=%p, MemType=MEM%u");
}

void Test_MM_PeekMem_DefaultSwitch(void)
{
    MM_PeekCmd_t CmdPacket;
    uint32       SrcAddress = 1;
    int32        Result;

    CmdPacket.Payload.DataSize = 99;

    /* Execute the function being tested */
    Result = MM_PeekMem(&CmdPacket, SrcAddress);

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
                         "PSP read memory error: RC=%d, Address=%p, MemType=MEM%u");
}

void Test_MM_DumpMemToFile_Nominal(void)
{
    osal_id_t               FileHandle = MM_UT_OBJID_1;
    char                    FileName[CFE_MISSION_MAX_PATH_LEN];
    MM_LoadDumpFileHeader_t FileHeader;
    int32                   Result;

    strncpy(FileName, "filename", sizeof(FileName) - 1);
    FileName[sizeof(FileName) - 1] = '\0';

    FileHeader.NumOfBytes        = 1;
    /* a valid source address is required input to memcpy */
    FileHeader.SymAddress.Offset = CFE_ES_MEMADDRESS_C(&MM_AppData.LoadBuffer[0]);
    FileHeader.MemType           = MM_MemType_RAM;

    /* Execute the function being tested */
    Result = MM_DumpMemToFile(FileHandle, FileName, &FileHeader);

    /* Verify results */
    UtAssert_True(Result == OS_SUCCESS, "Result == OS_SUCCESS");
    UtAssert_True(MM_AppData.HkTlm.Payload.LastAction == MM_LastAction_DUMP_TO_FILE,
                  "MM_AppData.HkTlm.Payload.LastAction == MM_LastAction_DUMP_TO_FILE");
    UtAssert_True(MM_AppData.HkTlm.Payload.MemType == FileHeader.MemType,
                  "MM_AppData.HkTlm.Payload.MemType == FileHeader.MemType");
    UtAssert_ADDRESS_EQ(CFE_ES_MEMADDRESS_TO_PTR(MM_AppData.HkTlm.Payload.Address),
                        CFE_ES_MEMADDRESS_TO_PTR(FileHeader.SymAddress.Offset));
    UtAssert_True(MM_AppData.HkTlm.Payload.BytesProcessed == 1, "MM_AppData.HkTlm.Payload.BytesProcessed == 1");
    UtAssert_STRINGBUF_EQ(MM_AppData.HkTlm.Payload.FileName,
                          sizeof(MM_AppData.HkTlm.Payload.FileName),
                          FileName,
                          sizeof(FileName));

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_DumpMemToFile_CPUHogging(void)
{
    osal_id_t               FileHandle = MM_UT_OBJID_1;
    char                    FileName[CFE_MISSION_MAX_PATH_LEN];
    MM_LoadDumpFileHeader_t FileHeader;
    int32                   Result;
    char                    Data[2 * MM_INTERNAL_MAX_DUMP_DATA_SEG] = { 0 };

    strncpy(FileName, "filename", sizeof(FileName) - 1);
    FileName[sizeof(FileName) - 1] = '\0';

    FileHeader.NumOfBytes        = sizeof(Data);
    /* a valid source address is required input to memcpy */
    FileHeader.SymAddress.Offset = CFE_ES_MEMADDRESS_C(Data);
    FileHeader.MemType           = MM_MemType_RAM;

    /* Execute the function being tested */
    Result = MM_DumpMemToFile(FileHandle, FileName, &FileHeader);

    /* Verify results */
    UtAssert_True(Result == OS_SUCCESS, "Result == OS_SUCCESS");
    UtAssert_True(MM_AppData.HkTlm.Payload.LastAction == MM_LastAction_DUMP_TO_FILE,
                  "MM_AppData.HkTlm.Payload.LastAction == MM_LastAction_DUMP_TO_FILE");
    UtAssert_True(MM_AppData.HkTlm.Payload.MemType == FileHeader.MemType,
                  "MM_AppData.HkTlm.Payload.MemType == FileHeader.MemType");
    UtAssert_ADDRESS_EQ(CFE_ES_MEMADDRESS_TO_PTR(MM_AppData.HkTlm.Payload.Address),
                        CFE_ES_MEMADDRESS_TO_PTR(FileHeader.SymAddress.Offset));
    UtAssert_True(MM_AppData.HkTlm.Payload.BytesProcessed == 2 * MM_INTERNAL_MAX_DUMP_DATA_SEG,
                  "MM_AppData.HkTlm.Payload.BytesProcessed == 2 * "
                  "MM_INTERNAL_MAX_DUMP_DATA_SEG");
    UtAssert_STRINGBUF_EQ(MM_AppData.HkTlm.Payload.FileName,
                          sizeof(MM_AppData.HkTlm.Payload.FileName),
                          FileName,
                          sizeof(FileName));

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_DumpMemToFile_WriteError(void)
{
    osal_id_t               FileHandle = MM_UT_OBJID_1;
    char                    FileName[CFE_MISSION_MAX_PATH_LEN];
    MM_LoadDumpFileHeader_t FileHeader;
    int32                   Result;

    strncpy(FileName, "filename", sizeof(FileName) - 1);
    FileName[sizeof(FileName) - 1] = '\0';

    FileHeader.NumOfBytes        = 1;
    /* a valid source address is required input to memcpy */
    FileHeader.SymAddress.Offset = CFE_ES_MEMADDRESS_C(&MM_AppData.LoadBuffer[0]);
    FileHeader.MemType           = MM_MemType_RAM;

    /* Set to generate error message MM_OS_WRITE_EXP_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(OS_write), 1, OS_ERROR);

    /* Execute the function being tested */
    Result = MM_DumpMemToFile(FileHandle, FileName, &FileHeader);

    /* Verify results */
    UtAssert_True(Result == OS_ERROR, "Result == OS_ERROR");

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_OS_WRITE_EXP_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "OS_write error received: RC = %d, Expected = %u, File = '%s'");
}

void Test_MM_WriteFileHeaders_Nominal(void)
{
    char                    FileName[CFE_MISSION_MAX_PATH_LEN];
    osal_id_t               FileHandle = MM_UT_OBJID_1;
    CFE_FS_Header_t         CFEHeader;
    MM_LoadDumpFileHeader_t MMHeader;
    int32                   Result;

    strncpy(FileName, "filename", sizeof(FileName) - 1);
    FileName[sizeof(FileName) - 1] = '\0';

    MMHeader.NumOfBytes        = 1;
    MMHeader.SymAddress.Offset = CFE_ES_MEMADDRESS_C(0);
    MMHeader.MemType           = MM_MemType_RAM;

    UT_SetDefaultReturnValue(UT_KEY(CFE_FS_WriteHeader), sizeof(CFE_FS_Header_t));

    /* Execute the function being tested */
    Result = MM_WriteFileHeaders(FileName, FileHandle, &CFEHeader, &MMHeader);

    /* Verify results */
    UtAssert_INT32_EQ(Result, OS_SUCCESS);

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_WriteFileHeaders_WriteHeaderError(void)
{
    char                    FileName[CFE_MISSION_MAX_PATH_LEN];
    osal_id_t               FileHandle = MM_UT_OBJID_1;
    CFE_FS_Header_t         CFEHeader;
    MM_LoadDumpFileHeader_t MMHeader;
    int32                   Result;

    strncpy(FileName, "filename", sizeof(FileName) - 1);
    FileName[sizeof(FileName) - 1] = '\0';

    MMHeader.NumOfBytes        = 1;
    MMHeader.SymAddress.Offset = CFE_ES_MEMADDRESS_C(0);
    MMHeader.MemType           = MM_MemType_RAM;

    /* Set to generate error message MM_CFE_FS_WRITEHDR_ERR_EID */
    UT_SetDefaultReturnValue(UT_KEY(CFE_FS_WriteHeader), CFE_FS_BAD_ARGUMENT);

    /* Execute the function being tested */
    Result = MM_WriteFileHeaders(FileName, FileHandle, &CFEHeader, &MMHeader);

    /* Verify results */
    UtAssert_INT32_EQ(Result, OS_ERR_INVALID_SIZE);

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_CFE_FS_WRITEHDR_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "CFE_FS_WriteHeader error received: RC = %d Expected = %d File = '%s'");
}

void Test_MM_WriteFileHeaders_WriteError(void)
{
    char                    FileName[CFE_MISSION_MAX_PATH_LEN];
    osal_id_t               FileHandle = MM_UT_OBJID_1;
    CFE_FS_Header_t         CFEHeader;
    MM_LoadDumpFileHeader_t MMHeader;
    int32                   Result;

    strncpy(FileName, "filename", sizeof(FileName) - 1);
    FileName[sizeof(FileName) - 1] = '\0';

    MMHeader.NumOfBytes        = 1;
    MMHeader.SymAddress.Offset = CFE_ES_MEMADDRESS_C(0);
    MMHeader.MemType           = MM_MemType_RAM;

    /* Set to generate error message MM_CFE_FS_WRITEHDR_ERR_EID */
    UT_SetDefaultReturnValue(UT_KEY(OS_write), OS_ERROR);

    /* Execute the function being tested */
    Result = MM_WriteFileHeaders(FileName, FileHandle, &CFEHeader, &MMHeader);

    /* Verify results */
    UtAssert_INT32_EQ(Result, OS_ERR_INVALID_SIZE);

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_OS_WRITE_EXP_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "OS_write error received: RC = %d Expected = %u File = '%s'");
}

void Test_MM_FillDumpInEventBuffer_RAM(void)
{
    MM_DumpInEventCmd_t CmdPacket;
    /* a valid source address is required input to memcpy */
    cpuaddr             SrcAddress = (cpuaddr)&MM_AppData.LoadBuffer[0];
    int32               Result;

    CmdPacket.Payload.MemType              = MM_MemType_RAM;
    CmdPacket.Payload.NumOfBytes           = 1;
    CmdPacket.Payload.SrcSymAddress.Offset = CFE_ES_MEMADDRESS_C(0);

    /* Execute the function being tested */
    Result = MM_FillDumpInEventBuffer(SrcAddress, &CmdPacket, (uint8 *)(&MM_AppData.DumpBuffer[0]));

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_SUCCESS, "Result == CFE_PSP_SUCCESS");

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_FillDumpInEventBuffer_BadType(void)
{
    MM_DumpInEventCmd_t CmdPacket;
    /* a valid source address is required input to memcpy */
    cpuaddr             SrcAddress = (cpuaddr)&MM_AppData.LoadBuffer[0];
    int32               Result;

    CmdPacket.Payload.MemType              = 99;
    CmdPacket.Payload.NumOfBytes           = 1;
    CmdPacket.Payload.SrcSymAddress.Offset = CFE_ES_MEMADDRESS_C(0);

    /* Execute the function being tested */
    Result = MM_FillDumpInEventBuffer(SrcAddress, &CmdPacket, (uint8 *)(&MM_AppData.DumpBuffer[0]));

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR, "Result == CFE_PSP_ERROR");

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_FillDumpInEventBuffer_EEPROM(void)
{
    MM_DumpInEventCmd_t CmdPacket;
    /* a valid source address is required input to memcpy */
    cpuaddr             SrcAddress = (cpuaddr)&MM_AppData.LoadBuffer[0];
    int32               Result;

    CmdPacket.Payload.MemType              = MM_MemType_EEPROM;
    CmdPacket.Payload.NumOfBytes           = 1;
    CmdPacket.Payload.SrcSymAddress.Offset = CFE_ES_MEMADDRESS_C(0);

    /* Execute the function being tested */
    Result = MM_FillDumpInEventBuffer(SrcAddress, &CmdPacket, (uint8 *)(&MM_AppData.DumpBuffer[0]));

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_SUCCESS, "Result == CFE_PSP_SUCCESS");

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_FillDumpInEventBuffer_MEM32(void)
{
    MM_DumpInEventCmd_t CmdPacket;
    /* CFE_PSP_MemRead32 stub returns success with non-zero address */
    cpuaddr             SrcAddress = 1;
    int32               Result;

    CmdPacket.Payload.MemType              = MM_MemType_MEM32;
    CmdPacket.Payload.NumOfBytes           = 4;
    CmdPacket.Payload.SrcSymAddress.Offset = CFE_ES_MEMADDRESS_C(0);

    /* Execute the function being tested */
    Result = MM_FillDumpInEventBuffer(SrcAddress, &CmdPacket, (uint8 *)(&MM_AppData.DumpBuffer[0]));

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_SUCCESS, "Result == CFE_PSP_SUCCESS");

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_FillDumpInEventBuffer_MEM16(void)
{
    MM_DumpInEventCmd_t CmdPacket;
    /* CFE_PSP_MemRead16 stub returns success with non-zero address */
    cpuaddr             SrcAddress = 1;
    int32               Result;

    CmdPacket.Payload.MemType              = MM_MemType_MEM16;
    CmdPacket.Payload.NumOfBytes           = 2;
    CmdPacket.Payload.SrcSymAddress.Offset = CFE_ES_MEMADDRESS_C(0);

    /* Execute the function being tested */
    Result = MM_FillDumpInEventBuffer(SrcAddress, &CmdPacket, (uint8 *)(&MM_AppData.DumpBuffer[0]));

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_SUCCESS, "Result == CFE_PSP_SUCCESS");

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_FillDumpInEventBuffer_MEM8(void)
{
    MM_DumpInEventCmd_t CmdPacket;
    cpuaddr             SrcAddress = 0;
    int32               Result;

    CmdPacket.Payload.MemType              = MM_MemType_MEM8;
    CmdPacket.Payload.NumOfBytes           = 1;
    CmdPacket.Payload.SrcSymAddress.Offset = CFE_ES_MEMADDRESS_C(0);

    /* Execute the function being tested */
    Result = MM_FillDumpInEventBuffer(SrcAddress, &CmdPacket, (uint8 *)(&MM_AppData.DumpBuffer[0]));

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_SUCCESS, "Result == CFE_PSP_SUCCESS");

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_FillDumpInEventBuffer_MEM32ReadError(void)
{
    MM_DumpInEventCmd_t CmdPacket;
    cpuaddr             SrcAddress = 0;
    int32               Result;

    CmdPacket.Payload.MemType              = MM_MemType_MEM32;
    CmdPacket.Payload.NumOfBytes           = 4;
    CmdPacket.Payload.SrcSymAddress.Offset = CFE_ES_MEMADDRESS_C(0);

    /* Set to generate error message MM_PSP_READ_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemRead32), 1, CFE_PSP_ERROR);

    /* Execute the function being tested */
    Result = MM_FillDumpInEventBuffer(SrcAddress, &CmdPacket, (uint8 *)(&MM_AppData.DumpBuffer[0]));

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR, "Result == CFE_PSP_ERROR");

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_PSP_READ_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "PSP read memory error: RC=%d, Src=%p, Tgt=%p, Type=MEM32");
}

void Test_MM_FillDumpInEventBuffer_MEM16ReadError(void)
{
    MM_DumpInEventCmd_t CmdPacket;
    cpuaddr             SrcAddress = 0;
    int32               Result;

    CmdPacket.Payload.MemType              = MM_MemType_MEM16;
    CmdPacket.Payload.NumOfBytes           = 4;
    CmdPacket.Payload.SrcSymAddress.Offset = CFE_ES_MEMADDRESS_C(0);

    /* Set to generate error message MM_PSP_READ_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemRead16), 1, CFE_PSP_ERROR_NOT_IMPLEMENTED);

    /* Execute the function being tested */
    Result = MM_FillDumpInEventBuffer(SrcAddress, &CmdPacket, (uint8 *)(&MM_AppData.DumpBuffer[0]));

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR_NOT_IMPLEMENTED, "Result == CFE_PSP_ERROR_NOT_IMPLEMENTED");

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_PSP_READ_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "PSP read memory error: RC=%d, Src=%p, Tgt=%p, Type=MEM16");
}

void Test_MM_FillDumpInEventBuffer_MEM8ReadError(void)
{
    MM_DumpInEventCmd_t CmdPacket;
    cpuaddr             SrcAddress = 0;
    int32               Result;

    CmdPacket.Payload.MemType              = MM_MemType_MEM8;
    CmdPacket.Payload.NumOfBytes           = 4;
    CmdPacket.Payload.SrcSymAddress.Offset = CFE_ES_MEMADDRESS_C(0);

    /* Set to generate error message MM_PSP_READ_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemRead8), 1, CFE_PSP_ERROR_NOT_IMPLEMENTED);

    /* Execute the function being tested */
    Result = MM_FillDumpInEventBuffer(SrcAddress, &CmdPacket, (uint8 *)(&MM_AppData.DumpBuffer[0]));

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR_NOT_IMPLEMENTED, "Result == CFE_PSP_ERROR_NOT_IMPLEMENTED");

    /* no command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_PSP_READ_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "PSP read memory error: RC=%d, Src=%p, Tgt=%p, Type=MEM8");
}

/*
 * Register the test cases to execute with the unit test tool
 */
void UtTest_Setup(void)
{
    ADD_TEST(Test_MM_PeekMem_Byte);
    ADD_TEST(Test_MM_PeekMem_ByteError);
    ADD_TEST(Test_MM_PeekMem_Word);
    ADD_TEST(Test_MM_PeekMem_WordError);
    ADD_TEST(Test_MM_PeekMem_DWord);
    ADD_TEST(Test_MM_PeekMem_DWordError);
    ADD_TEST(Test_MM_PeekMem_DefaultSwitch);
    ADD_TEST(Test_MM_DumpMemToFile_Nominal);
    ADD_TEST(Test_MM_DumpMemToFile_CPUHogging);
    ADD_TEST(Test_MM_DumpMemToFile_WriteError);
    ADD_TEST(Test_MM_WriteFileHeaders_Nominal);
    ADD_TEST(Test_MM_WriteFileHeaders_WriteHeaderError);
    ADD_TEST(Test_MM_WriteFileHeaders_WriteError);
    ADD_TEST(Test_MM_FillDumpInEventBuffer_RAM);
    ADD_TEST(Test_MM_FillDumpInEventBuffer_BadType);
    ADD_TEST(Test_MM_FillDumpInEventBuffer_EEPROM);
    ADD_TEST(Test_MM_FillDumpInEventBuffer_MEM32);
    ADD_TEST(Test_MM_FillDumpInEventBuffer_MEM16);
    ADD_TEST(Test_MM_FillDumpInEventBuffer_MEM8);
    ADD_TEST(Test_MM_FillDumpInEventBuffer_MEM32ReadError);
    ADD_TEST(Test_MM_FillDumpInEventBuffer_MEM16ReadError);
    ADD_TEST(Test_MM_FillDumpInEventBuffer_MEM8ReadError);
}
