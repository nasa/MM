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
 *   Unit tests for mm_cmds.c
 */

/* ======== */
/* Includes */
/* ======== */

#include "mm_cmds.h"
#include "mm_dispatch.h"
#include "mm_dump.h"
#include "mm_eventids.h"
#include "mm_filedefs.h"
#include "mm_interface_cfg.h"
#include "mm_load.h"
#include "mm_mem16.h"
#include "mm_mem32.h"
#include "mm_mem8.h"
#include "mm_msgids.h"
#include "mm_test_utils.h"
#include "mm_utils.h"

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

void Test_MM_SendHkCmd_Nominal(void)
{
    MM_SendHkCmd_t SendHkCmd;
    CFE_Status_t   Result;

    memset((void *)&SendHkCmd, 0, sizeof(MM_SendHkCmd_t));

    /* Execute the function being tested */
    Result = MM_SendHkCmd(&SendHkCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
    UtAssert_STUB_COUNT(CFE_SB_TimeStampMsg, 1);
    UtAssert_STUB_COUNT(CFE_SB_TransmitMsg, 1);
}

void Test_MM_NoopCmd_Nominal(void)
{
    MM_NoopCmd_t NoopCmd;
    CFE_Status_t Status;

    memset((void *)&NoopCmd, 0, sizeof(MM_NoopCmd_t));

    /* Set up values to evaluate against */
    MM_AppData.HkTlm.Payload.LastAction = MM_LastAction_NOACTION;

    /* Run function under test */
    Status = MM_NoopCmd(&NoopCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Status, CFE_SUCCESS);

    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.LastAction, MM_LastAction_NOOP);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 1);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0, MM_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "No-op command. Version %d.%d.%d.%d");
}

void Test_MM_ResetCountersCmd_Nominal(void)
{
    MM_ResetCountersCmd_t ResetCountersCmd;
    CFE_Status_t          Status;

    memset((void *)&ResetCountersCmd, 0, sizeof(MM_ResetCountersCmd_t));

    /* Set up values to evaluate against */
    MM_AppData.HkTlm.Payload.LastAction          = MM_LastAction_NOACTION;
    MM_AppData.HkTlm.Payload.CommandCounter      = 1;
    MM_AppData.HkTlm.Payload.CommandErrorCounter = 1;

    /* Run function under test */
    Status = MM_ResetCountersCmd(&ResetCountersCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Status, CFE_SUCCESS);

    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.LastAction, MM_LastAction_RESET);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0, MM_RESET_INF_EID, CFE_EVS_EventType_INFORMATION, "Reset counters command received");
}

void Test_MM_LookupSymCmd_Nominal(void)
{
    MM_LookupSymCmd_t LookupSymCmd;
    uint32            Addr;
    cpuaddr           ResolvedAddr;
    CFE_Status_t      Result;

    /* Set up value to evaluate against */
    Addr         = 0x42;
    ResolvedAddr = (cpuaddr)&Addr;

    /* Force functions to act as expected */
    UT_SetDefaultReturnValue(UT_KEY(OS_strnlen), 1);
    UT_SetDefaultReturnValue(UT_KEY(OS_SymbolLookup), OS_SUCCESS);

    UT_SetDataBuffer(UT_KEY(OS_SymbolLookup), &ResolvedAddr, sizeof(cpuaddr), false);

    /* Execute the function being tested */
    Result = MM_LookupSymCmd(&LookupSymCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 1);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.LastAction, MM_LastAction_SYM_LOOKUP);
    UtAssert_ADDRESS_EQ(CFE_ES_MEMADDRESS_TO_PTR(MM_AppData.HkTlm.Payload.Address), ResolvedAddr);

    MM_Test_Verify_Event(0,
                         MM_SYM_LOOKUP_INF_EID,
                         CFE_EVS_EventType_INFORMATION,
                         "Symbol Lookup Command: Name = '%s' Addr = %p");
}

void Test_MM_LookupSymCmd_SymbolNameNull(void)
{
    MM_LookupSymCmd_t LookupSymCmd;
    CFE_Status_t      Result;

    UT_SetDefaultReturnValue(UT_KEY(OS_strnlen), 0);

    /* Execute the function being tested */
    Result = MM_LookupSymCmd(&LookupSymCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_SYMNAME_NUL_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "NUL (empty) string specified as symbol name");
}

void Test_MM_LookupSymCmd_SymbolLookupError(void)
{
    MM_LookupSymCmd_t LookupSymCmd;
    CFE_Status_t      Result;

    UT_SetDefaultReturnValue(UT_KEY(OS_strnlen), 1);
    UT_SetDefaultReturnValue(UT_KEY(OS_SymbolLookup), OS_ERROR);

    /* Execute the function being tested */
    Result = MM_LookupSymCmd(&LookupSymCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_SYMNAME_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Symbolic address can't be resolved: Name = '%s'");
}

void Test_MM_SymTblToFileCmd_Nominal(void)
{
    MM_SymTblToFileCmd_t SymTblToFileCmd;
    CFE_Status_t         Result;
    char                 FileName[9] = { "filename" };

    UT_SetDefaultReturnValue(UT_KEY(OS_strnlen), 1);
    UT_SetDefaultReturnValue(UT_KEY(OS_SymbolTableDump), OS_SUCCESS);

    UT_SetHandlerFunction(UT_KEY(CFE_SB_MessageStringGet), UT_Handler_CFE_SB_MessageStringGet, FileName);

    /* Execute the function being tested */
    Result = MM_SymTblToFileCmd(&SymTblToFileCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 1);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.LastAction, MM_LastAction_SYMTBL_SAVE);
    UtAssert_StrnCmp(MM_AppData.HkTlm.Payload.FileName,
                     FileName,
                     sizeof(FileName),
                     "Expected '%s' == '%s",
                     MM_AppData.HkTlm.Payload.FileName,
                     FileName);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_SYMTBL_TO_FILE_INF_EID,
                         CFE_EVS_EventType_INFORMATION,
                         "Symbol Table Dump to File Started: Name = '%s'");
}

void Test_MM_SymTblToFileCmd_SymbolFilenameNull(void)
{
    MM_SymTblToFileCmd_t SymTblToFileCmd;
    CFE_Status_t         Result;

    UT_SetDefaultReturnValue(UT_KEY(OS_strnlen), 0);

    /* Execute the function being tested */
    Result = MM_SymTblToFileCmd(&SymTblToFileCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_SYMFILENAME_NUL_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "NUL (empty) string specified as symbol dump file name");
}

void Test_MM_SymTblToFileCmd_SymbolTableDumpError(void)
{
    MM_SymTblToFileCmd_t SymTblToFileCmd;
    CFE_Status_t         Result;

    UT_SetDefaultReturnValue(UT_KEY(OS_strnlen), 1);
    UT_SetDefaultReturnValue(UT_KEY(OS_SymbolTableDump), OS_ERROR);

    /* Execute the function being tested */
    Result = MM_SymTblToFileCmd(&SymTblToFileCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_SYMTBL_TO_FILE_FAIL_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Error dumping symbol table, OS_Status= 0x%X, File='%s'");
}

void Test_MM_EepromWriteEnaCmd_Nominal(void)
{
    MM_EepromWriteEnaCmd_t EepromWriteEnaCmd;
    CFE_Status_t           Result;

    UT_SetDefaultReturnValue(UT_KEY(CFE_PSP_EepromWriteEnable), CFE_PSP_SUCCESS);

    /* Execute the function being tested */
    Result = MM_EepromWriteEnaCmd(&EepromWriteEnaCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 1);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.LastAction, MM_LastAction_EEPROMWRITE_ENA);
    UtAssert_EQ(MM_MemType_Enum_t, MM_AppData.HkTlm.Payload.MemType, MM_MemType_EEPROM);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_EEPROM_WRITE_ENA_INF_EID,
                         CFE_EVS_EventType_INFORMATION,
                         "EEPROM bank %d write enabled, cFE_Status= 0x%X");
}

void Test_MM_EepromWriteEnaCmd_Error(void)
{
    MM_EepromWriteEnaCmd_t EepromWriteEnaCmd;
    CFE_Status_t           Result;

    /* Set to generate error message MM_EEPROM_WRITE_ENA_ERR_EID */
    UT_SetDefaultReturnValue(UT_KEY(CFE_PSP_EepromWriteEnable), CFE_PSP_ERROR_NOT_IMPLEMENTED);

    /* Execute the function being tested */
    Result = MM_EepromWriteEnaCmd(&EepromWriteEnaCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_EEPROM_WRITE_ENA_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Error requesting EEPROM bank %d write enable, cFE_Status= 0x%X");
}

void Test_MM_EepromWriteDisCmd_Nominal(void)
{
    MM_EepromWriteDisCmd_t EepromWriteDisCmd;
    CFE_Status_t           Result;

    UT_SetDefaultReturnValue(UT_KEY(CFE_PSP_EepromWriteDisable), CFE_PSP_SUCCESS);

    /* Execute the function being tested */
    Result = MM_EepromWriteDisCmd(&EepromWriteDisCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 1);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.LastAction, MM_LastAction_EEPROMWRITE_DIS);
    UtAssert_EQ(MM_MemType_Enum_t, MM_AppData.HkTlm.Payload.MemType, MM_MemType_EEPROM);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_EEPROM_WRITE_DIS_INF_EID,
                         CFE_EVS_EventType_INFORMATION,
                         "EEPROM bank %d write disabled, cFE_Status= 0x%X");
}

void Test_MM_EepromWriteDisCmd_Error(void)
{
    MM_EepromWriteDisCmd_t EepromWriteDisCmd;
    CFE_Status_t           Result;

    /* Set to generate error message MM_EEPROM_WRITE_DISA_ERR_EID */
    UT_SetDefaultReturnValue(UT_KEY(CFE_PSP_EepromWriteDisable), CFE_PSP_ERROR_NOT_IMPLEMENTED);

    /* Execute the function being tested */
    Result = MM_EepromWriteDisCmd(&EepromWriteDisCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_EEPROM_WRITE_DIS_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Error requesting EEPROM bank %d write disable, cFE_Status= 0x%X");
}

void Test_MM_PokeCmd_EEPROM(void)
{
    CFE_Status_t Result;
    MM_PokeCmd_t PokeCmd;

    memset(&(PokeCmd.Payload), 0, sizeof(MM_PokeCmd_Payload_t));

    PokeCmd.Payload.MemType = MM_MemType_EEPROM;

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyPeekPokeParams), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_PokeEeprom), CFE_PSP_SUCCESS);

    /* Execute the function being tested */
    Result = MM_PokeCmd(&PokeCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 1);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(MM_PokeEeprom, 1);

    /* Event issued in MM_PokeMem but wouldn't show up here */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_PokeCmd_NonEEPROM(void)
{
    CFE_Status_t Result;
    MM_PokeCmd_t PokeCmd;

    memset(&(PokeCmd.Payload), 0, sizeof(MM_PokeCmd_Payload_t));

    PokeCmd.Payload.MemType = MM_MemType_RAM;

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyPeekPokeParams), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_PokeMem), CFE_PSP_SUCCESS);

    /* Execute the function being tested */
    Result = MM_PokeCmd(&PokeCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 1);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(MM_PokeMem, 1);

    /* Event issued in MM_PokeMem but wouldn't show up here */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_PokeCmd_SymNameError(void)
{
    CFE_Status_t Result;
    MM_PokeCmd_t PokeCmd;

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), OS_ERROR);

    /* Execute the function being tested */
    Result = MM_PokeCmd(&PokeCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_SYMNAME_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Symbolic address can't be resolved: Name = '%s'");
}

void Test_MM_PokeCmd_NoVerifyPeekPokeParams(void)
{
    CFE_Status_t Result;
    MM_PokeCmd_t PokeCmd;

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyPeekPokeParams), OS_ERROR);

    /* Execute the function being tested */
    Result = MM_PokeCmd(&PokeCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_PokeCmd_PokeErr(void)
{
    CFE_Status_t Status;
    MM_PokeCmd_t PokeCmd;

    memset(&(PokeCmd.Payload), 0, sizeof(MM_PokeCmd_Payload_t));
    PokeCmd.Payload.MemType = MM_MemType_EEPROM;

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyPeekPokeParams), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_PokeEeprom), CFE_PSP_ERROR_NOT_IMPLEMENTED);

    /* Execute the function being tested */
    Status = MM_PokeCmd(&PokeCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Status, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(MM_PokeEeprom, 1);

    /* Event issued in MM_PokeMem but wouldn't show up here */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_LoadMemWIDCmd_Nominal(void)
{
    CFE_Status_t       Result;
    MM_LoadMemWIDCmd_t LoadMemWIDCmd;
    uint32             ComputedCRC;
    cpuaddr            SymAddr;

    memset(&(LoadMemWIDCmd.Payload), 0, sizeof(MM_LoadMemWIDCmd_Payload_t));
    memset(LoadMemWIDCmd.Payload.DataArray, 1, sizeof(uint8) * MM_INTERFACE_MAX_UNINTERRUPTIBLE_DATA);

    ComputedCRC                      = 0x42;
    SymAddr                          = (cpuaddr)&ComputedCRC;
    LoadMemWIDCmd.Payload.Crc        = ComputedCRC;
    LoadMemWIDCmd.Payload.NumOfBytes = 1;

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), CFE_PSP_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(CFE_ES_CalculateCRC), ComputedCRC);

    UT_SetHandlerFunction(UT_KEY(MM_ResolveSymAddr), UT_Handler_MM_ResolveSymAddr, &SymAddr);

    /* Execute the function being tested */
    Result = MM_LoadMemWIDCmd(&LoadMemWIDCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 1);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.LastAction, MM_LastAction_LOAD_WID);
    UtAssert_ADDRESS_EQ(CFE_ES_MEMADDRESS_TO_PTR(MM_AppData.HkTlm.Payload.Address), SymAddr);
    UtAssert_EQ(MM_MemType_Enum_t, MM_AppData.HkTlm.Payload.MemType, MM_MemType_RAM);
    UtAssert_EQ(size_t, MM_AppData.HkTlm.Payload.BytesProcessed, LoadMemWIDCmd.Payload.NumOfBytes);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_LOAD_WID_INF_EID,
                         CFE_EVS_EventType_INFORMATION,
                         "Load Memory WID Command: Wrote %d bytes to address: %p");
}

void Test_MM_LoadMemWIDCmd_CRCError(void)
{
    CFE_Status_t       Result;
    MM_LoadMemWIDCmd_t LoadMemWIDCmd;
    uint32             ComputedCRC;

    memset(&(LoadMemWIDCmd.Payload), 0, sizeof(MM_LoadMemWIDCmd_Payload_t));

    ComputedCRC               = 0x42;
    LoadMemWIDCmd.Payload.Crc = ComputedCRC + 1;

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), CFE_PSP_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(CFE_ES_CalculateCRC), ComputedCRC);

    /* Execute the function being tested */
    Result = MM_LoadMemWIDCmd(&LoadMemWIDCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_LOAD_WID_CRC_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Interrupts Disabled Load CRC failure: Expected = 0x%X "
                         "Calculated = 0x%X");
}

void Test_MM_LoadMemWIDCmd_SymNameErr(void)
{
    CFE_Status_t       Result;
    MM_LoadMemWIDCmd_t LoadMemWIDCmd;

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), OS_ERROR);

    /* Execute the function being tested */
    Result = MM_LoadMemWIDCmd(&LoadMemWIDCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_SYMNAME_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Symbolic address can't be resolved: Name = '%s'");
}

void Test_MM_LoadMemWIDCmd_NoVerifyLoadWIDParams(void)
{
    CFE_Status_t       Result;
    MM_LoadMemWIDCmd_t LoadMemWIDCmd;

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), CFE_PSP_ERROR);

    /* Execute the function being tested */
    Result = MM_LoadMemWIDCmd(&LoadMemWIDCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_LoadMemFromFileCmd_RAM(void)
{
    CFE_Status_t            Result;
    MM_LoadDumpFileHeader_t Hdr;
    uint32                  ComputedCrc;
    MM_LoadMemFromFileCmd_t LoadMemFromFileCmd;

    memset(&Hdr, 0, sizeof(MM_LoadDumpFileHeader_t));

    ComputedCrc = 99;
    Hdr.Crc     = ComputedCrc;
    Hdr.MemType = MM_MemType_RAM;

    UT_SetDefaultReturnValue(UT_KEY(OS_OpenCreate), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_ReadFileHeaders), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadFileSize), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_ComputeCRCFromFile), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(OS_lseek), sizeof(CFE_FS_Header_t) + sizeof(MM_LoadDumpFileHeader_t));
    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), CFE_PSP_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_LoadMemFromFile), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(OS_close), OS_SUCCESS);

    UT_SetHandlerFunction(UT_KEY(MM_ComputeCRCFromFile), UT_Handler_MM_ComputeCRCFromFile, &ComputedCrc);
    UT_SetHandlerFunction(UT_KEY(MM_ReadFileHeaders), UT_Handler_MM_ReadFileHeaders, &Hdr);

    /* Execute the function being tested */
    Result = MM_LoadMemFromFileCmd(&LoadMemFromFileCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 1);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_LD_MEM_FILE_INF_EID,
                         CFE_EVS_EventType_INFORMATION,
                         "Load Memory From File Command: Loaded %d bytes to "
                         "address %p from file '%s'");
}

void Test_MM_LoadMemFromFileCmd_BadType(void)
{
    CFE_Status_t            Result;
    MM_LoadDumpFileHeader_t Hdr;
    uint32                  ComputedCrc;
    MM_LoadMemFromFileCmd_t LoadMemFromFileCmd;

    memset(&Hdr, 0, sizeof(MM_LoadDumpFileHeader_t));

    ComputedCrc = 99;
    Hdr.Crc     = ComputedCrc;
    Hdr.MemType = 42;

    UT_SetDefaultReturnValue(UT_KEY(OS_OpenCreate), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_ReadFileHeaders), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadFileSize), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_ComputeCRCFromFile), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(OS_lseek), sizeof(CFE_FS_Header_t) + sizeof(MM_LoadDumpFileHeader_t));
    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), CFE_PSP_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(OS_close), OS_SUCCESS);

    UT_SetHandlerFunction(UT_KEY(MM_ComputeCRCFromFile), UT_Handler_MM_ComputeCRCFromFile, &ComputedCrc);
    UT_SetHandlerFunction(UT_KEY(MM_ReadFileHeaders), UT_Handler_MM_ReadFileHeaders, &Hdr);

    /* Execute the function being tested */
    Result = MM_LoadMemFromFileCmd(&LoadMemFromFileCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_LoadMemFromFileCmd_EEPROM(void)
{
    CFE_Status_t            Result;
    MM_LoadDumpFileHeader_t Hdr;
    uint32                  ComputedCrc;
    MM_LoadMemFromFileCmd_t LoadMemFromFileCmd;

    memset(&Hdr, 0, sizeof(MM_LoadDumpFileHeader_t));

    ComputedCrc = 99;
    Hdr.Crc     = ComputedCrc;
    Hdr.MemType = MM_MemType_EEPROM;

    UT_SetDefaultReturnValue(UT_KEY(OS_OpenCreate), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_ReadFileHeaders), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadFileSize), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_ComputeCRCFromFile), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(OS_lseek), sizeof(CFE_FS_Header_t) + sizeof(MM_LoadDumpFileHeader_t));
    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), CFE_PSP_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_LoadMemFromFile), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(OS_close), OS_SUCCESS);

    UT_SetHandlerFunction(UT_KEY(MM_ComputeCRCFromFile), UT_Handler_MM_ComputeCRCFromFile, &ComputedCrc);
    UT_SetHandlerFunction(UT_KEY(MM_ReadFileHeaders), UT_Handler_MM_ReadFileHeaders, &Hdr);

    /* Execute the function being tested */
    Result = MM_LoadMemFromFileCmd(&LoadMemFromFileCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 1);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_LD_MEM_FILE_INF_EID,
                         CFE_EVS_EventType_INFORMATION,
                         "Load Memory From File Command: Loaded %d bytes to "
                         "address %p from file '%s'");
}

void Test_MM_LoadMemFromFileCmd_MEM32(void)
{
    CFE_Status_t            Result;
    MM_LoadDumpFileHeader_t Hdr;
    uint32                  ComputedCrc;
    MM_LoadMemFromFileCmd_t LoadMemFromFileCmd;

    memset(&Hdr, 0, sizeof(MM_LoadDumpFileHeader_t));

    ComputedCrc = 99;
    Hdr.Crc     = ComputedCrc;
    Hdr.MemType = MM_MemType_MEM32;

    UT_SetDefaultReturnValue(UT_KEY(OS_OpenCreate), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_ReadFileHeaders), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadFileSize), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_ComputeCRCFromFile), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(OS_lseek), sizeof(CFE_FS_Header_t) + sizeof(MM_LoadDumpFileHeader_t));
    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), CFE_PSP_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_LoadMem32FromFile), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(OS_close), OS_SUCCESS);

    UT_SetHandlerFunction(UT_KEY(MM_ComputeCRCFromFile), UT_Handler_MM_ComputeCRCFromFile, &ComputedCrc);
    UT_SetHandlerFunction(UT_KEY(MM_ReadFileHeaders), UT_Handler_MM_ReadFileHeaders, &Hdr);

    /* Execute the function being tested */
    Result = MM_LoadMemFromFileCmd(&LoadMemFromFileCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 1);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(MM_LoadMem32FromFile, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_LD_MEM_FILE_INF_EID,
                         CFE_EVS_EventType_INFORMATION,
                         "Load Memory From File Command: Loaded %d bytes to "
                         "address %p from file '%s'");
}

void Test_MM_LoadMemFromFileCmd_MEM32Invalid(void)
{
    CFE_Status_t            Result;
    MM_LoadDumpFileHeader_t Hdr;
    uint32                  ComputedCrc;
    MM_LoadMemFromFileCmd_t LoadMemFromFileCmd;

    memset(&Hdr, 0, sizeof(MM_LoadDumpFileHeader_t));

    ComputedCrc = 99;
    Hdr.Crc     = ComputedCrc;
    Hdr.MemType = MM_MemType_MEM32;

    UT_SetDefaultReturnValue(UT_KEY(OS_OpenCreate), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_ReadFileHeaders), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadFileSize), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_ComputeCRCFromFile), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(OS_lseek), sizeof(CFE_FS_Header_t) + sizeof(MM_LoadDumpFileHeader_t));
    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), CFE_PSP_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_LoadMem32FromFile), OS_ERROR);
    UT_SetDefaultReturnValue(UT_KEY(OS_close), OS_SUCCESS);

    UT_SetHandlerFunction(UT_KEY(MM_ComputeCRCFromFile), UT_Handler_MM_ComputeCRCFromFile, &ComputedCrc);
    UT_SetHandlerFunction(UT_KEY(MM_ReadFileHeaders), UT_Handler_MM_ReadFileHeaders, &Hdr);

    /* Execute the function being tested */
    Result = MM_LoadMemFromFileCmd(&LoadMemFromFileCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(MM_LoadMem32FromFile, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_LoadMemFromFileCmd_MEM16(void)
{
    CFE_Status_t            Result;
    MM_LoadDumpFileHeader_t Hdr;
    uint32                  ComputedCrc;
    MM_LoadMemFromFileCmd_t LoadMemFromFileCmd;

    memset(&Hdr, 0, sizeof(MM_LoadDumpFileHeader_t));

    ComputedCrc = 99;
    Hdr.Crc     = ComputedCrc;
    Hdr.MemType = MM_MemType_MEM16;

    UT_SetDefaultReturnValue(UT_KEY(OS_OpenCreate), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_ReadFileHeaders), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadFileSize), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_ComputeCRCFromFile), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(OS_lseek), sizeof(CFE_FS_Header_t) + sizeof(MM_LoadDumpFileHeader_t));
    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), CFE_PSP_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_LoadMem16FromFile), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(OS_close), OS_SUCCESS);

    UT_SetHandlerFunction(UT_KEY(MM_ComputeCRCFromFile), UT_Handler_MM_ComputeCRCFromFile, &ComputedCrc);
    UT_SetHandlerFunction(UT_KEY(MM_ReadFileHeaders), UT_Handler_MM_ReadFileHeaders, &Hdr);

    /* Execute the function being tested */
    Result = MM_LoadMemFromFileCmd(&LoadMemFromFileCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 1);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(MM_LoadMem16FromFile, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_LD_MEM_FILE_INF_EID,
                         CFE_EVS_EventType_INFORMATION,
                         "Load Memory From File Command: Loaded %d bytes to "
                         "address %p from file '%s'");
}

void Test_MM_LoadMemFromFileCmd_MEM8(void)
{
    CFE_Status_t            Result;
    MM_LoadDumpFileHeader_t Hdr;
    uint32                  ComputedCrc;
    MM_LoadMemFromFileCmd_t LoadMemFromFileCmd;

    memset(&Hdr, 0, sizeof(MM_LoadDumpFileHeader_t));

    ComputedCrc = 99;
    Hdr.Crc     = ComputedCrc;
    Hdr.MemType = MM_MemType_MEM8;

    UT_SetDefaultReturnValue(UT_KEY(OS_OpenCreate), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_ReadFileHeaders), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadFileSize), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_ComputeCRCFromFile), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(OS_lseek), sizeof(CFE_FS_Header_t) + sizeof(MM_LoadDumpFileHeader_t));
    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), CFE_PSP_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_LoadMem8FromFile), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(OS_close), OS_SUCCESS);

    UT_SetHandlerFunction(UT_KEY(MM_ComputeCRCFromFile), UT_Handler_MM_ComputeCRCFromFile, &ComputedCrc);
    UT_SetHandlerFunction(UT_KEY(MM_ReadFileHeaders), UT_Handler_MM_ReadFileHeaders, &Hdr);

    /* Execute the function being tested */
    Result = MM_LoadMemFromFileCmd(&LoadMemFromFileCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 1);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(MM_LoadMem8FromFile, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_LD_MEM_FILE_INF_EID,
                         CFE_EVS_EventType_INFORMATION,
                         "Load Memory From File Command: Loaded %d bytes to "
                         "address %p from file '%s'");
}

void Test_MM_LoadMemFromFileCmd_NoReadFileHeaders(void)
{
    CFE_Status_t            Result;
    MM_LoadMemFromFileCmd_t LoadMemFromFileCmd;

    UT_SetDefaultReturnValue(UT_KEY(OS_OpenCreate), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_ReadFileHeaders), OS_ERROR);

    /* Execute the function being tested */
    Result = MM_LoadMemFromFileCmd(&LoadMemFromFileCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_LoadMemFromFileCmd_NoVerifyLoadFileSize(void)
{
    CFE_Status_t            Result;
    MM_LoadMemFromFileCmd_t LoadMemFromFileCmd;

    UT_SetDefaultReturnValue(UT_KEY(OS_OpenCreate), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_ReadFileHeaders), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadFileSize), OS_ERR_INVALID_SIZE);

    /* Execute the function being tested */
    Result = MM_LoadMemFromFileCmd(&LoadMemFromFileCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_LoadMemFromFileCmd_lseekError(void)
{
    CFE_Status_t            Result;
    MM_LoadDumpFileHeader_t Hdr;
    uint32                  ComputedCrc;
    MM_LoadMemFromFileCmd_t LoadMemFromFileCmd;

    ComputedCrc = 99;
    Hdr.Crc     = ComputedCrc;

    UT_SetDefaultReturnValue(UT_KEY(OS_OpenCreate), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_ReadFileHeaders), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadFileSize), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_ComputeCRCFromFile), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(OS_lseek), OS_ERROR);
    UT_SetDefaultReturnValue(UT_KEY(OS_close), OS_SUCCESS);

    UT_SetHandlerFunction(UT_KEY(MM_ComputeCRCFromFile), UT_Handler_MM_ComputeCRCFromFile, &ComputedCrc);
    UT_SetHandlerFunction(UT_KEY(MM_ReadFileHeaders), UT_Handler_MM_ReadFileHeaders, &Hdr);

    /* Execute the function being tested */
    Result = MM_LoadMemFromFileCmd(&LoadMemFromFileCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_LOAD_FILE_CRC_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Load file CRC failure: Expected = 0x%X Calculated = 0x%X File = '%s'");
}

void Test_MM_LoadMemFromFileCmd_LoadParamsError(void)
{
    CFE_Status_t            Result;
    MM_LoadDumpFileHeader_t Hdr;
    uint32                  ComputedCrc;
    MM_LoadMemFromFileCmd_t LoadMemFromFileCmd;

    ComputedCrc = 99;
    Hdr.Crc     = ComputedCrc;

    UT_SetDefaultReturnValue(UT_KEY(OS_OpenCreate), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_ReadFileHeaders), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadFileSize), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_ComputeCRCFromFile), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(OS_lseek), sizeof(CFE_FS_Header_t) + sizeof(MM_LoadDumpFileHeader_t));
    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), CFE_PSP_ERROR_NOT_IMPLEMENTED);
    UT_SetDefaultReturnValue(UT_KEY(OS_close), OS_SUCCESS);

    UT_SetHandlerFunction(UT_KEY(MM_ComputeCRCFromFile), UT_Handler_MM_ComputeCRCFromFile, &ComputedCrc);
    UT_SetHandlerFunction(UT_KEY(MM_ReadFileHeaders), UT_Handler_MM_ReadFileHeaders, &Hdr);

    /* Execute the function being tested */
    Result = MM_LoadMemFromFileCmd(&LoadMemFromFileCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_FILE_LOAD_PARAMS_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Load file failed parameters check: File = '%s'");
}

void Test_MM_LoadMemFromFileCmd_SymNameError(void)
{
    CFE_Status_t            Result;
    MM_LoadDumpFileHeader_t Hdr;
    uint32                  ComputedCrc;
    MM_LoadMemFromFileCmd_t LoadMemFromFileCmd;

    ComputedCrc = 99;
    Hdr.Crc     = ComputedCrc;

    UT_SetDefaultReturnValue(UT_KEY(OS_OpenCreate), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_ReadFileHeaders), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadFileSize), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_ComputeCRCFromFile), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(OS_lseek), sizeof(CFE_FS_Header_t) + sizeof(MM_LoadDumpFileHeader_t));
    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), OS_ERROR);
    UT_SetDefaultReturnValue(UT_KEY(OS_close), OS_SUCCESS);

    UT_SetHandlerFunction(UT_KEY(MM_ComputeCRCFromFile), UT_Handler_MM_ComputeCRCFromFile, &ComputedCrc);
    UT_SetHandlerFunction(UT_KEY(MM_ReadFileHeaders), UT_Handler_MM_ReadFileHeaders, &Hdr);

    /* Execute the function being tested */
    Result = MM_LoadMemFromFileCmd(&LoadMemFromFileCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_SYMNAME_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Symbolic address can't be resolved: Name = '%s'");
}

void Test_MM_LoadMemFromFileCmd_LoadFileCRCError(void)
{
    CFE_Status_t            Result;
    MM_LoadDumpFileHeader_t Hdr;
    uint32                  ComputedCrc;
    MM_LoadMemFromFileCmd_t LoadMemFromFileCmd;

    ComputedCrc = 99;
    Hdr.Crc     = ComputedCrc + 1;

    UT_SetDefaultReturnValue(UT_KEY(OS_OpenCreate), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_ReadFileHeaders), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadFileSize), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_ComputeCRCFromFile), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(OS_lseek), sizeof(CFE_FS_Header_t) + sizeof(MM_LoadDumpFileHeader_t));
    UT_SetDefaultReturnValue(UT_KEY(OS_close), OS_SUCCESS);

    UT_SetHandlerFunction(UT_KEY(MM_ComputeCRCFromFile), UT_Handler_MM_ComputeCRCFromFile, &ComputedCrc);
    UT_SetHandlerFunction(UT_KEY(MM_ReadFileHeaders), UT_Handler_MM_ReadFileHeaders, &Hdr);

    /* Execute the function being tested */
    Result = MM_LoadMemFromFileCmd(&LoadMemFromFileCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_LOAD_FILE_CRC_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Load file CRC failure: Expected = 0x%X Calculated = 0x%X File = '%s'");
}

void Test_MM_LoadMemFromFileCmd_ComputeCRCError(void)
{
    CFE_Status_t            Result;
    MM_LoadMemFromFileCmd_t LoadMemFromFileCmd;

    UT_SetDefaultReturnValue(UT_KEY(OS_OpenCreate), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_ReadFileHeaders), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadFileSize), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_ComputeCRCFromFile), OS_ERROR);
    UT_SetDefaultReturnValue(UT_KEY(OS_close), OS_SUCCESS);

    /* Execute the function being tested */
    Result = MM_LoadMemFromFileCmd(&LoadMemFromFileCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_COMPUTECRCFROMFILE_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "MM_ComputeCRCFromFile error received: RC = 0x%08X File = '%s'");
}

void Test_MM_LoadMemFromFileCmd_CloseError(void)
{
    CFE_Status_t            Result;
    MM_LoadMemFromFileCmd_t LoadMemFromFileCmd;

    UT_SetDefaultReturnValue(UT_KEY(OS_OpenCreate), OS_SUCCESS);

    /* Skip most of the function since it's tested elsewhere */
    UT_SetDefaultReturnValue(UT_KEY(MM_ReadFileHeaders), OS_ERROR);

    UT_SetDefaultReturnValue(UT_KEY(OS_close), OS_ERROR);

    /* Execute the function being tested */
    Result = MM_LoadMemFromFileCmd(&LoadMemFromFileCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 2);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_OS_CLOSE_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "OS_close error received: RC = 0x%08X File = '%s'");
}

void Test_MM_LoadMemFromFileCmd_OpenError(void)
{
    CFE_Status_t            Result;
    MM_LoadMemFromFileCmd_t LoadMemFromFileCmd;

    UT_SetDefaultReturnValue(UT_KEY(OS_OpenCreate), OS_ERROR);

    /* Execute the function being tested */
    Result = MM_LoadMemFromFileCmd(&LoadMemFromFileCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_OS_OPEN_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "OS_OpenCreate error received: RC = %d File = '%s'");
}

void Test_MM_FillMemCmd_RAM(void)
{
    CFE_Status_t    Result;
    MM_FillMemCmd_t FillMemCmd;

    memset(&(FillMemCmd.Payload), 0, sizeof(MM_FillMemCmd_Payload_t));

    FillMemCmd.Payload.MemType          = MM_MemType_RAM;
    MM_AppData.HkTlm.Payload.LastAction = MM_LastAction_FILL;

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), CFE_PSP_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_FillMem), CFE_PSP_SUCCESS);

    /* Execute the function being tested */
    Result = MM_FillMemCmd(&FillMemCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 1);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_FILL_INF_EID,
                         CFE_EVS_EventType_INFORMATION,
                         "Fill Memory Command: Filled %d bytes at address: %p "
                         "with pattern: 0x%08X");
}

void Test_MM_FillMemCmd_EEPROM(void)
{
    CFE_Status_t    Result;
    MM_FillMemCmd_t FillMemCmd;

    memset(&(FillMemCmd.Payload), 0, sizeof(MM_FillMemCmd_Payload_t));

    FillMemCmd.Payload.MemType          = MM_MemType_EEPROM;
    MM_AppData.HkTlm.Payload.LastAction = MM_LastAction_FILL;

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), CFE_PSP_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_FillMem), CFE_PSP_SUCCESS);

    /* Execute the function being tested */
    Result = MM_FillMemCmd(&FillMemCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 1);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_FILL_INF_EID,
                         CFE_EVS_EventType_INFORMATION,
                         "Fill Memory Command: Filled %d bytes at address: %p "
                         "with pattern: 0x%08X");
}

void Test_MM_FillMemCmd_MEM32(void)
{
    CFE_Status_t    Result;
    MM_FillMemCmd_t FillMemCmd;

    memset(&(FillMemCmd.Payload), 0, sizeof(MM_FillMemCmd_Payload_t));

    FillMemCmd.Payload.MemType          = MM_MemType_MEM32;
    MM_AppData.HkTlm.Payload.LastAction = MM_LastAction_FILL;

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), CFE_PSP_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_FillMem32), CFE_PSP_SUCCESS);

    /* Execute the function being tested */
    Result = MM_FillMemCmd(&FillMemCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 1);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(MM_FillMem32, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_FILL_INF_EID,
                         CFE_EVS_EventType_INFORMATION,
                         "Fill Memory Command: Filled %d bytes at address: %p "
                         "with pattern: 0x%08X");
}

void Test_MM_FillMemCmd_MEM16(void)
{
    CFE_Status_t    Result;
    MM_FillMemCmd_t FillMemCmd;

    memset(&(FillMemCmd.Payload), 0, sizeof(MM_FillMemCmd_Payload_t));

    FillMemCmd.Payload.MemType          = MM_MemType_MEM16;
    MM_AppData.HkTlm.Payload.LastAction = MM_LastAction_FILL;

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), CFE_PSP_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_FillMem16), CFE_PSP_SUCCESS);

    /* Execute the function being tested */
    Result = MM_FillMemCmd(&FillMemCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 1);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(MM_FillMem16, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_FILL_INF_EID,
                         CFE_EVS_EventType_INFORMATION,
                         "Fill Memory Command: Filled %d bytes at address: %p "
                         "with pattern: 0x%08X");
}

void Test_MM_FillMemCmd_MEM8(void)
{
    CFE_Status_t    Result;
    MM_FillMemCmd_t FillMemCmd;

    memset(&(FillMemCmd.Payload), 0, sizeof(MM_FillMemCmd_Payload_t));

    FillMemCmd.Payload.MemType          = MM_MemType_MEM8;
    MM_AppData.HkTlm.Payload.LastAction = MM_LastAction_FILL;

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), CFE_PSP_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_FillMem8), CFE_PSP_SUCCESS);

    /* Execute the function being tested */
    Result = MM_FillMemCmd(&FillMemCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 1);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(MM_FillMem8, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_FILL_INF_EID,
                         CFE_EVS_EventType_INFORMATION,
                         "Fill Memory Command: Filled %d bytes at address: %p "
                         "with pattern: 0x%08X");
}

void Test_MM_FillMemCmd_SymNameError(void)
{
    CFE_Status_t    Result;
    MM_FillMemCmd_t FillMemCmd;

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), OS_ERROR);

    /* Execute the function being tested */
    Result = MM_FillMemCmd(&FillMemCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_SYMNAME_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Symbolic address can't be resolved: Name = '%s'");
}

void Test_MM_FillMemCmd_NoLastActionFill(void)
{
    CFE_Status_t    Result;
    MM_FillMemCmd_t FillMemCmd;

    memset(&(FillMemCmd.Payload), 0, sizeof(MM_FillMemCmd_Payload_t));

    FillMemCmd.Payload.MemType          = MM_MemType_RAM;
    MM_AppData.HkTlm.Payload.LastAction = MM_LastAction_NOACTION;

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), CFE_PSP_SUCCESS);

    /* Execute the function being tested */
    Result = MM_FillMemCmd(&FillMemCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_FillMemCmd_NoVerifyLoadDump(void)
{
    CFE_Status_t    Result;
    MM_FillMemCmd_t FillMemCmd;

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), CFE_PSP_ERROR);

    /* Execute the function being tested */
    Result = MM_FillMemCmd(&FillMemCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_FillMemCmd_BadType(void)
{
    CFE_Status_t    Result;
    MM_FillMemCmd_t FillMemCmd;

    memset(&(FillMemCmd.Payload), 0, sizeof(MM_FillMemCmd_Payload_t));

    FillMemCmd.Payload.MemType    = 99;
    FillMemCmd.Payload.NumOfBytes = 1;

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), CFE_PSP_SUCCESS);

    /* Execute the function being tested */
    Result = MM_FillMemCmd(&FillMemCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 1);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_PeekCmd_Nominal(void)
{
    CFE_Status_t Result;
    MM_PeekCmd_t PeekCmd;

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyPeekPokeParams), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_PeekMem), CFE_PSP_SUCCESS);

    /* Execute the function being tested */
    Result = MM_PeekCmd(&PeekCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 1);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_PeekCmd_SymNameError(void)
{
    CFE_Status_t Result;
    MM_PeekCmd_t PeekCmd;

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), OS_ERROR_NAME_LENGTH);

    /* Execute the function being tested */
    Result = MM_PeekCmd(&PeekCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_SYMNAME_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Symbolic address can't be resolved: Name = '%s'");
}

void Test_MM_PeekCmd_NoVerifyPeekPokeParams(void)
{
    CFE_Status_t Result;
    MM_PeekCmd_t PeekCmd;

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyPeekPokeParams), OS_ERROR);

    /* Execute the function being tested */
    Result = MM_PeekCmd(&PeekCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_PeekCmd_PeekErr(void)
{
    CFE_Status_t Result;
    MM_PeekCmd_t PeekCmd;

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyPeekPokeParams), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_PeekMem), CFE_PSP_ERROR_NOT_IMPLEMENTED);

    /* Execute the function being tested */
    Result = MM_PeekCmd(&PeekCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_DumpMemToFileCmd_RAM(void)
{
    CFE_Status_t          Result;
    MM_DumpMemToFileCmd_t DumpMemToFileCmd;
    cpuaddr               SymAddr;

    memset(&(DumpMemToFileCmd.Payload), 0, sizeof(MM_DumpMemToFileCmd_Payload_t));

    SymAddr                             = 0x42;
    DumpMemToFileCmd.Payload.MemType    = MM_MemType_RAM;
    DumpMemToFileCmd.Payload.NumOfBytes = 50;
    strncpy(DumpMemToFileCmd.Payload.FileName, "filename_RAM", sizeof(DumpMemToFileCmd.Payload.FileName) - 1);

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), CFE_PSP_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(OS_OpenCreate), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_WriteFileHeaders), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_DumpMemToFile), CFE_PSP_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(OS_lseek), sizeof(CFE_FS_Header_t) + sizeof(MM_LoadDumpFileHeader_t));
    UT_SetDefaultReturnValue(UT_KEY(MM_ComputeCRCFromFile), OS_SUCCESS);
    /* Second call to MM_WriteFileHeaders will also return OS_SUCCESS as set above
     */

    UT_SetHandlerFunction(UT_KEY(MM_ResolveSymAddr), UT_Handler_MM_ResolveSymAddr, &SymAddr);

    /* Execute the function being tested */
    Result = MM_DumpMemToFileCmd(&(DumpMemToFileCmd));

    /* Verify the results */
    UtAssert_UINT8_EQ(MM_AppData.HkTlm.Payload.LastAction, MM_LastAction_DUMP_TO_FILE);
    UtAssert_EQ(MM_MemType_Enum_t, MM_AppData.HkTlm.Payload.MemType, DumpMemToFileCmd.Payload.MemType);
    UtAssert_ADDRESS_EQ(CFE_ES_MEMADDRESS_TO_PTR(MM_AppData.HkTlm.Payload.Address), SymAddr);
    UtAssert_EQ(size_t, MM_AppData.HkTlm.Payload.BytesProcessed, DumpMemToFileCmd.Payload.NumOfBytes);
    UtAssert_StrCmp(MM_AppData.HkTlm.Payload.FileName,
                    DumpMemToFileCmd.Payload.FileName,
                    "Expected '%s' == '%s",
                    MM_AppData.HkTlm.Payload.FileName,
                    DumpMemToFileCmd.Payload.FileName);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 1);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(MM_DumpMemToFile, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_DMP_MEM_FILE_INF_EID,
                         CFE_EVS_EventType_INFORMATION,
                         "Dump Memory To File Command: Dumped %d bytes from "
                         "address %p to file '%s'");
}

void Test_MM_DumpMemToFileCmd_EEPROM(void)
{
    CFE_Status_t          Result;
    MM_DumpMemToFileCmd_t DumpMemToFileCmd;
    cpuaddr               SymAddr;

    memset(&(DumpMemToFileCmd.Payload), 0, sizeof(MM_DumpMemToFileCmd_Payload_t));

    SymAddr                             = 0x42;
    DumpMemToFileCmd.Payload.MemType    = MM_MemType_EEPROM;
    DumpMemToFileCmd.Payload.NumOfBytes = 40;
    strncpy(DumpMemToFileCmd.Payload.FileName, "filename_EEPROM", sizeof(DumpMemToFileCmd.Payload.FileName) - 1);

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), CFE_PSP_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(OS_OpenCreate), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_WriteFileHeaders), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_DumpMemToFile), CFE_PSP_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(OS_lseek), sizeof(CFE_FS_Header_t) + sizeof(MM_LoadDumpFileHeader_t));
    UT_SetDefaultReturnValue(UT_KEY(MM_ComputeCRCFromFile), OS_SUCCESS);
    /* Second call to MM_WriteFileHeaders will also return OS_SUCCESS as set above
     */

    UT_SetHandlerFunction(UT_KEY(MM_ResolveSymAddr), UT_Handler_MM_ResolveSymAddr, &SymAddr);

    /* Execute the function being tested */
    Result = MM_DumpMemToFileCmd(&(DumpMemToFileCmd));

    /* Verify the results */
    UtAssert_UINT8_EQ(MM_AppData.HkTlm.Payload.LastAction, MM_LastAction_DUMP_TO_FILE);
    UtAssert_EQ(MM_MemType_Enum_t, MM_AppData.HkTlm.Payload.MemType, DumpMemToFileCmd.Payload.MemType);
    UtAssert_ADDRESS_EQ(CFE_ES_MEMADDRESS_TO_PTR(MM_AppData.HkTlm.Payload.Address), SymAddr);
    UtAssert_EQ(size_t, MM_AppData.HkTlm.Payload.BytesProcessed, DumpMemToFileCmd.Payload.NumOfBytes);
    UtAssert_StrCmp(MM_AppData.HkTlm.Payload.FileName,
                    DumpMemToFileCmd.Payload.FileName,
                    "Expected '%s' == '%s",
                    MM_AppData.HkTlm.Payload.FileName,
                    DumpMemToFileCmd.Payload.FileName);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 1);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(MM_DumpMemToFile, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_DMP_MEM_FILE_INF_EID,
                         CFE_EVS_EventType_INFORMATION,
                         "Dump Memory To File Command: Dumped %d bytes from "
                         "address %p to file '%s'");
}

void Test_MM_DumpMemToFileCmd_MEM32(void)
{
    CFE_Status_t          Result;
    MM_DumpMemToFileCmd_t DumpMemToFileCmd;
    cpuaddr               SymAddr;

    memset(&(DumpMemToFileCmd.Payload), 0, sizeof(MM_DumpMemToFileCmd_Payload_t));

    SymAddr                             = 0x42;
    DumpMemToFileCmd.Payload.MemType    = MM_MemType_MEM32;
    DumpMemToFileCmd.Payload.NumOfBytes = 32;
    strncpy(DumpMemToFileCmd.Payload.FileName, "filename_32", sizeof(DumpMemToFileCmd.Payload.FileName) - 1);

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), CFE_PSP_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(OS_OpenCreate), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_WriteFileHeaders), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_DumpMem32ToFile), CFE_PSP_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(OS_lseek), sizeof(CFE_FS_Header_t) + sizeof(MM_LoadDumpFileHeader_t));
    UT_SetDefaultReturnValue(UT_KEY(MM_ComputeCRCFromFile), OS_SUCCESS);
    /* Second call to MM_WriteFileHeaders will also return OS_SUCCESS as set above
     */

    UT_SetHandlerFunction(UT_KEY(MM_ResolveSymAddr), UT_Handler_MM_ResolveSymAddr, &SymAddr);

    /* Execute the function being tested */
    Result = MM_DumpMemToFileCmd(&(DumpMemToFileCmd));

    /* Verify the results */
    UtAssert_UINT8_EQ(MM_AppData.HkTlm.Payload.LastAction, MM_LastAction_DUMP_TO_FILE);
    UtAssert_EQ(MM_MemType_Enum_t, MM_AppData.HkTlm.Payload.MemType, DumpMemToFileCmd.Payload.MemType);
    UtAssert_ADDRESS_EQ(CFE_ES_MEMADDRESS_TO_PTR(MM_AppData.HkTlm.Payload.Address), SymAddr);
    UtAssert_EQ(size_t, MM_AppData.HkTlm.Payload.BytesProcessed, DumpMemToFileCmd.Payload.NumOfBytes);
    UtAssert_StrCmp(MM_AppData.HkTlm.Payload.FileName,
                    DumpMemToFileCmd.Payload.FileName,
                    "Expected '%s' == '%s",
                    MM_AppData.HkTlm.Payload.FileName,
                    DumpMemToFileCmd.Payload.FileName);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 1);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(MM_DumpMem32ToFile, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_DMP_MEM_FILE_INF_EID,
                         CFE_EVS_EventType_INFORMATION,
                         "Dump Memory To File Command: Dumped %d bytes from "
                         "address %p to file '%s'");
}

void Test_MM_DumpMemToFileCmd_MEM16(void)
{
    CFE_Status_t          Result;
    MM_DumpMemToFileCmd_t DumpMemToFileCmd;
    cpuaddr               SymAddr;

    memset(&(DumpMemToFileCmd.Payload), 0, sizeof(MM_DumpMemToFileCmd_Payload_t));

    SymAddr                             = 0x42;
    DumpMemToFileCmd.Payload.MemType    = MM_MemType_MEM16;
    DumpMemToFileCmd.Payload.NumOfBytes = 16;
    strncpy(DumpMemToFileCmd.Payload.FileName, "filename_16", sizeof(DumpMemToFileCmd.Payload.FileName) - 1);

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), CFE_PSP_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(OS_OpenCreate), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_WriteFileHeaders), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_DumpMem16ToFile), CFE_PSP_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(OS_lseek), sizeof(CFE_FS_Header_t) + sizeof(MM_LoadDumpFileHeader_t));
    UT_SetDefaultReturnValue(UT_KEY(MM_ComputeCRCFromFile), OS_SUCCESS);
    /* Second call to MM_WriteFileHeaders will also return OS_SUCCESS as set above
     */

    UT_SetHandlerFunction(UT_KEY(MM_ResolveSymAddr), UT_Handler_MM_ResolveSymAddr, &SymAddr);

    /* Execute the function being tested */
    Result = MM_DumpMemToFileCmd(&(DumpMemToFileCmd));

    /* Verify the results */
    UtAssert_UINT8_EQ(MM_AppData.HkTlm.Payload.LastAction, MM_LastAction_DUMP_TO_FILE);
    UtAssert_EQ(MM_MemType_Enum_t, MM_AppData.HkTlm.Payload.MemType, DumpMemToFileCmd.Payload.MemType);
    UtAssert_ADDRESS_EQ(CFE_ES_MEMADDRESS_TO_PTR(MM_AppData.HkTlm.Payload.Address), SymAddr);
    UtAssert_EQ(size_t, MM_AppData.HkTlm.Payload.BytesProcessed, DumpMemToFileCmd.Payload.NumOfBytes);
    UtAssert_StrCmp(MM_AppData.HkTlm.Payload.FileName,
                    DumpMemToFileCmd.Payload.FileName,
                    "Expected '%s' == '%s",
                    MM_AppData.HkTlm.Payload.FileName,
                    DumpMemToFileCmd.Payload.FileName);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 1);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(MM_DumpMem16ToFile, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_DMP_MEM_FILE_INF_EID,
                         CFE_EVS_EventType_INFORMATION,
                         "Dump Memory To File Command: Dumped %d bytes from "
                         "address %p to file '%s'");
}

void Test_MM_DumpMemToFileCmd_MEM8(void)
{
    CFE_Status_t          Result;
    MM_DumpMemToFileCmd_t DumpMemToFileCmd;
    cpuaddr               SymAddr;

    memset(&(DumpMemToFileCmd.Payload), 0, sizeof(MM_DumpMemToFileCmd_Payload_t));

    SymAddr                             = 0x42;
    DumpMemToFileCmd.Payload.MemType    = MM_MemType_MEM8;
    DumpMemToFileCmd.Payload.NumOfBytes = 8;
    strncpy(DumpMemToFileCmd.Payload.FileName, "filename_8", sizeof(DumpMemToFileCmd.Payload.FileName) - 1);

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), CFE_PSP_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(OS_OpenCreate), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_WriteFileHeaders), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_DumpMem8ToFile), CFE_PSP_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(OS_lseek), sizeof(CFE_FS_Header_t) + sizeof(MM_LoadDumpFileHeader_t));
    UT_SetDefaultReturnValue(UT_KEY(MM_ComputeCRCFromFile), OS_SUCCESS);
    /* Second call to MM_WriteFileHeaders will also return OS_SUCCESS as set above
     */

    UT_SetHandlerFunction(UT_KEY(MM_ResolveSymAddr), UT_Handler_MM_ResolveSymAddr, &SymAddr);

    /* Execute the function being tested */
    Result = MM_DumpMemToFileCmd(&(DumpMemToFileCmd));

    /* Verify the results */
    UtAssert_UINT8_EQ(MM_AppData.HkTlm.Payload.LastAction, MM_LastAction_DUMP_TO_FILE);
    UtAssert_EQ(MM_MemType_Enum_t, MM_AppData.HkTlm.Payload.MemType, DumpMemToFileCmd.Payload.MemType);
    UtAssert_ADDRESS_EQ(CFE_ES_MEMADDRESS_TO_PTR(MM_AppData.HkTlm.Payload.Address), SymAddr);
    UtAssert_EQ(size_t, MM_AppData.HkTlm.Payload.BytesProcessed, DumpMemToFileCmd.Payload.NumOfBytes);
    UtAssert_StrCmp(MM_AppData.HkTlm.Payload.FileName,
                    DumpMemToFileCmd.Payload.FileName,
                    "Expected '%s' == '%s",
                    MM_AppData.HkTlm.Payload.FileName,
                    DumpMemToFileCmd.Payload.FileName);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 1);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_STUB_COUNT(MM_DumpMem8ToFile, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_DMP_MEM_FILE_INF_EID,
                         CFE_EVS_EventType_INFORMATION,
                         "Dump Memory To File Command: Dumped %d bytes from "
                         "address %p to file '%s'");
}

void Test_MM_DumpMemToFileCmd_ComputeCRCError(void)
{
    CFE_Status_t            Result;
    MM_DumpMemToFileCmd_t   DumpMemToFileCmd;
    MM_LoadDumpFileHeader_t MMFileHeader;

    memset(&MMFileHeader, 0, sizeof(MM_LoadDumpFileHeader_t));

    MMFileHeader.MemType = MM_MemType_RAM;

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), CFE_PSP_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(OS_OpenCreate), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_WriteFileHeaders), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_DumpMemToFile), CFE_PSP_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(OS_lseek), sizeof(CFE_FS_Header_t) + sizeof(MM_LoadDumpFileHeader_t));

    UT_SetHandlerFunction(UT_KEY(MM_WriteFileHeaders), UT_Handler_MM_WriteFileHeaders, &MMFileHeader);

    /* Set to generate error message MM_COMPUTECRCFROMFILE_ERR_EID */
    UT_SetDefaultReturnValue(UT_KEY(MM_ComputeCRCFromFile), OS_ERROR);

    /* Execute the function being tested */
    Result = MM_DumpMemToFileCmd(&DumpMemToFileCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_COMPUTECRCFROMFILE_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "MM_ComputeCRCFromFile error received: RC = 0x%08X File = '%s'");
}

void Test_MM_DumpMemToFileCmd_CloseError(void)
{
    CFE_Status_t          Result;
    MM_DumpMemToFileCmd_t DumpMemToFileCmd;

    memset(&(DumpMemToFileCmd.Payload), 0, sizeof(MM_DumpMemToFileCmd_Payload_t));

    DumpMemToFileCmd.Payload.MemType = MM_MemType_MEM8;

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), CFE_PSP_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(CFE_PSP_MemValidateRange), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_DumpMem8ToFile), CFE_PSP_SUCCESS);

    /* Set to generate error message MM_OS_CLOSE_ERR_EID */
    UT_SetDefaultReturnValue(UT_KEY(OS_close), OS_ERROR);

    /* Execute the function being tested */
    Result = MM_DumpMemToFileCmd(&DumpMemToFileCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 1);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);

    MM_Test_Verify_Event(0,
                         MM_DMP_MEM_FILE_INF_EID,
                         CFE_EVS_EventType_INFORMATION,
                         "Dump Memory To File Command: Dumped %d bytes from "
                         "address %p to file '%s'");

    MM_Test_Verify_Event(1,
                         MM_OS_CLOSE_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "OS_close error received: RC = 0x%08X File = '%s'");
}

void Test_MM_DumpMemToFileCmd_CreatError(void)
{
    CFE_Status_t          Result;
    MM_DumpMemToFileCmd_t DumpMemToFileCmd;

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), CFE_PSP_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(OS_OpenCreate), OS_ERROR);

    /* Execute the function being tested */
    Result = MM_DumpMemToFileCmd(&DumpMemToFileCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_OS_CREAT_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "OS_OpenCreate error received: RC = %d File = '%s'");
}

void Test_MM_DumpMemToFileCmd_InvalidDumpResult(void)
{
    CFE_Status_t          Result;
    MM_DumpMemToFileCmd_t DumpMemToFileCmd;

    memset(&(DumpMemToFileCmd.Payload), 0, sizeof(MM_DumpMemToFileCmd_Payload_t));

    DumpMemToFileCmd.Payload.MemType = MM_MemType_MEM8;

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), CFE_PSP_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(OS_OpenCreate), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_WriteFileHeaders), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_DumpMem8ToFile), CFE_PSP_ERROR_NOT_IMPLEMENTED);

    /* Execute the function being tested */
    Result = MM_DumpMemToFileCmd(&DumpMemToFileCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_DumpMemToFileCmd_lseekError(void)
{
    CFE_Status_t          Result;
    MM_DumpMemToFileCmd_t DumpMemToFileCmd;

    memset(&(DumpMemToFileCmd.Payload), 0, sizeof(MM_DumpMemToFileCmd_Payload_t));

    DumpMemToFileCmd.Payload.MemType = MM_MemType_MEM8;

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), CFE_PSP_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(OS_OpenCreate), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_WriteFileHeaders), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_DumpMem8ToFile), CFE_PSP_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(OS_lseek), OS_ERROR);

    /* Execute the function being tested */
    Result = MM_DumpMemToFileCmd(&DumpMemToFileCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_DumpMemToFileCmd_SymNameError(void)
{
    CFE_Status_t          Result;
    MM_DumpMemToFileCmd_t DumpMemToFileCmd;

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), OS_ERROR);

    /* Execute the function being tested */
    Result = MM_DumpMemToFileCmd(&DumpMemToFileCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_SYMNAME_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Symbolic address can't be resolved: Name = '%s'");
}

void Test_MM_DumpMemToFileCmd_NoVerifyDumpParams(void)
{
    CFE_Status_t          Result;
    MM_DumpMemToFileCmd_t DumpMemToFileCmd;

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), CFE_PSP_ERROR);

    /* Execute the function being tested */
    Result = MM_DumpMemToFileCmd(&DumpMemToFileCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_DumpMemToFileCmd_NoWriteHeaders(void)
{
    CFE_Status_t          Result;
    MM_DumpMemToFileCmd_t DumpMemToFileCmd;

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), CFE_PSP_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(OS_OpenCreate), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_WriteFileHeaders), OS_ERR_INVALID_SIZE);

    /* Execute the function being tested */
    Result = MM_DumpMemToFileCmd(&DumpMemToFileCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);
}

void Test_MM_DumpMemToFileCmd_BadType(void)
{
    CFE_Status_t          Result;
    MM_DumpMemToFileCmd_t DumpMemToFileCmd;

    memset(&(DumpMemToFileCmd.Payload), 0, sizeof(MM_DumpMemToFileCmd_Payload_t));

    DumpMemToFileCmd.Payload.MemType = 99;

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), CFE_PSP_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(OS_OpenCreate), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_WriteFileHeaders), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(OS_close), OS_SUCCESS);

    /* Execute the function being tested */
    Result = MM_DumpMemToFileCmd(&DumpMemToFileCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_DumpMemToFileCmd_ReWriteHeadersErr(void)
{
    CFE_Status_t          Result;
    MM_DumpMemToFileCmd_t DumpMemToFileCmd;
    cpuaddr               SymAddr;

    memset(&(DumpMemToFileCmd.Payload), 0, sizeof(MM_DumpMemToFileCmd_Payload_t));

    SymAddr                             = 0x42;
    DumpMemToFileCmd.Payload.MemType    = MM_MemType_RAM;
    DumpMemToFileCmd.Payload.NumOfBytes = 50;
    strncpy(DumpMemToFileCmd.Payload.FileName, "filename_RAM", sizeof(DumpMemToFileCmd.Payload.FileName) - 1);

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), CFE_PSP_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(OS_OpenCreate), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_WriteFileHeaders), OS_SUCCESS);
    UT_SetDeferredRetcode(UT_KEY(MM_WriteFileHeaders), 2, OS_ERR_INVALID_SIZE);
    UT_SetDefaultReturnValue(UT_KEY(MM_DumpMemToFile), CFE_PSP_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(OS_lseek), sizeof(CFE_FS_Header_t) + sizeof(MM_LoadDumpFileHeader_t));
    UT_SetDefaultReturnValue(UT_KEY(MM_ComputeCRCFromFile), OS_SUCCESS);

    UT_SetHandlerFunction(UT_KEY(MM_ResolveSymAddr), UT_Handler_MM_ResolveSymAddr, &SymAddr);

    /* Execute the function being tested */
    Result = MM_DumpMemToFileCmd(&(DumpMemToFileCmd));

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_DumpInEventCmd_Nominal(void)
{
    CFE_Status_t        Result;
    MM_DumpInEventCmd_t DumpInEventCmd;
    cpuaddr             SymAddr;

    memset(&(DumpInEventCmd.Payload), 0, sizeof(MM_DumpInEventCmd_Payload_t));

    SymAddr                           = 0x42;
    DumpInEventCmd.Payload.NumOfBytes = 1;
    DumpInEventCmd.Payload.MemType    = MM_MemType_EEPROM;

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), CFE_PSP_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_FillDumpInEventBuffer), CFE_PSP_SUCCESS);

    UT_SetHandlerFunction(UT_KEY(MM_ResolveSymAddr), UT_Handler_MM_ResolveSymAddr, &SymAddr);

    /* Execute the function being tested */
    Result = MM_DumpInEventCmd(&DumpInEventCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 1);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 0);

    UtAssert_UINT8_EQ(MM_AppData.HkTlm.Payload.LastAction, MM_LastAction_DUMP_INEVENT);
    UtAssert_EQ(MM_MemType_Enum_t, MM_AppData.HkTlm.Payload.MemType, DumpInEventCmd.Payload.MemType);
    UtAssert_ADDRESS_EQ(CFE_ES_MEMADDRESS_TO_PTR(MM_AppData.HkTlm.Payload.Address), SymAddr);
    UtAssert_EQ(size_t, MM_AppData.HkTlm.Payload.BytesProcessed, DumpInEventCmd.Payload.NumOfBytes);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0, MM_DUMP_INEVENT_INF_EID, CFE_EVS_EventType_INFORMATION, "%s");
}

void Test_MM_DumpInEventCmd_SymNameError(void)
{
    CFE_Status_t        Result;
    MM_DumpInEventCmd_t DumpInEventCmd;

    /* Set to generate error message MM_SYMNAME_ERR_EID */
    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), OS_ERROR_NAME_LENGTH);

    /* Execute the function being tested */
    Result = MM_DumpInEventCmd(&DumpInEventCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_SYMNAME_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Symbolic address can't be resolved: Name = '%s'");
}

void Test_MM_DumpInEventCmd_NoVerifyDumpParams(void)
{
    CFE_Status_t        Result;
    MM_DumpInEventCmd_t DumpInEventCmd;

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), CFE_PSP_ERROR);

    /* Execute the function being tested */
    Result = MM_DumpInEventCmd(&DumpInEventCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_DumpInEventCmd_FillDumpInvalid(void)
{
    CFE_Status_t        Result;
    MM_DumpInEventCmd_t DumpInEventCmd;

    UT_SetDefaultReturnValue(UT_KEY(MM_ResolveSymAddr), OS_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_VerifyLoadDumpParams), CFE_PSP_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(MM_FillDumpInEventBuffer), CFE_PSP_ERROR_NOT_IMPLEMENTED);

    /* Execute the function being tested */
    Result = MM_DumpInEventCmd(&DumpInEventCmd);

    /* Verify results */
    UtAssert_EQ(CFE_Status_t, Result, CFE_SUCCESS);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandCounter, 0);
    UtAssert_EQ(uint8, MM_AppData.HkTlm.Payload.CommandErrorCounter, 1);

    /* Error event is issued in MM_FillDumpInEventBuffer and thus not visible here
     */
}

/*
 * Register the test cases to execute with the unit test tool
 */
void UtTest_Setup(void)
{
    ADD_TEST(Test_MM_SendHkCmd_Nominal);
    ADD_TEST(Test_MM_NoopCmd_Nominal);
    ADD_TEST(Test_MM_ResetCountersCmd_Nominal);
    ADD_TEST(Test_MM_LookupSymCmd_Nominal);
    ADD_TEST(Test_MM_LookupSymCmd_SymbolNameNull);
    ADD_TEST(Test_MM_LookupSymCmd_SymbolLookupError);
    ADD_TEST(Test_MM_SymTblToFileCmd_Nominal);
    ADD_TEST(Test_MM_SymTblToFileCmd_SymbolFilenameNull);
    ADD_TEST(Test_MM_SymTblToFileCmd_SymbolTableDumpError);
    ADD_TEST(Test_MM_EepromWriteEnaCmd_Nominal);
    ADD_TEST(Test_MM_EepromWriteEnaCmd_Error);
    ADD_TEST(Test_MM_EepromWriteDisCmd_Nominal);
    ADD_TEST(Test_MM_EepromWriteDisCmd_Error);
    ADD_TEST(Test_MM_PokeCmd_EEPROM);
    ADD_TEST(Test_MM_PokeCmd_NonEEPROM);
    ADD_TEST(Test_MM_PokeCmd_SymNameError);
    ADD_TEST(Test_MM_PokeCmd_NoVerifyPeekPokeParams);
    ADD_TEST(Test_MM_PokeCmd_PokeErr);
    ADD_TEST(Test_MM_LoadMemWIDCmd_Nominal);
    ADD_TEST(Test_MM_LoadMemWIDCmd_CRCError);
    ADD_TEST(Test_MM_LoadMemWIDCmd_SymNameErr);
    ADD_TEST(Test_MM_LoadMemWIDCmd_NoVerifyLoadWIDParams);
    ADD_TEST(Test_MM_LoadMemFromFileCmd_RAM);
    ADD_TEST(Test_MM_LoadMemFromFileCmd_BadType);
    ADD_TEST(Test_MM_LoadMemFromFileCmd_EEPROM);
    ADD_TEST(Test_MM_LoadMemFromFileCmd_MEM32);
    ADD_TEST(Test_MM_LoadMemFromFileCmd_MEM32Invalid);
    ADD_TEST(Test_MM_LoadMemFromFileCmd_MEM16);
    ADD_TEST(Test_MM_LoadMemFromFileCmd_MEM8);
    ADD_TEST(Test_MM_LoadMemFromFileCmd_NoReadFileHeaders);
    ADD_TEST(Test_MM_LoadMemFromFileCmd_NoVerifyLoadFileSize);
    ADD_TEST(Test_MM_LoadMemFromFileCmd_lseekError);
    ADD_TEST(Test_MM_LoadMemFromFileCmd_LoadParamsError);
    ADD_TEST(Test_MM_LoadMemFromFileCmd_SymNameError);
    ADD_TEST(Test_MM_LoadMemFromFileCmd_LoadFileCRCError);
    ADD_TEST(Test_MM_LoadMemFromFileCmd_ComputeCRCError);
    ADD_TEST(Test_MM_LoadMemFromFileCmd_CloseError);
    ADD_TEST(Test_MM_LoadMemFromFileCmd_OpenError);
    ADD_TEST(Test_MM_FillMemCmd_RAM);
    ADD_TEST(Test_MM_FillMemCmd_EEPROM);
    ADD_TEST(Test_MM_FillMemCmd_MEM32);
    ADD_TEST(Test_MM_FillMemCmd_MEM16);
    ADD_TEST(Test_MM_FillMemCmd_MEM8);
    ADD_TEST(Test_MM_FillMemCmd_SymNameError);
    ADD_TEST(Test_MM_FillMemCmd_NoLastActionFill);
    ADD_TEST(Test_MM_FillMemCmd_NoVerifyLoadDump);
    ADD_TEST(Test_MM_FillMemCmd_BadType);
    ADD_TEST(Test_MM_PeekCmd_Nominal);
    ADD_TEST(Test_MM_PeekCmd_SymNameError);
    ADD_TEST(Test_MM_PeekCmd_NoVerifyPeekPokeParams);
    ADD_TEST(Test_MM_PeekCmd_PeekErr);
    ADD_TEST(Test_MM_DumpMemToFileCmd_RAM);
    ADD_TEST(Test_MM_DumpMemToFileCmd_EEPROM);
    ADD_TEST(Test_MM_DumpMemToFileCmd_MEM32);
    ADD_TEST(Test_MM_DumpMemToFileCmd_MEM16);
    ADD_TEST(Test_MM_DumpMemToFileCmd_MEM8);
    ADD_TEST(Test_MM_DumpMemToFileCmd_ComputeCRCError);
    ADD_TEST(Test_MM_DumpMemToFileCmd_CloseError);
    ADD_TEST(Test_MM_DumpMemToFileCmd_CreatError);
    ADD_TEST(Test_MM_DumpMemToFileCmd_InvalidDumpResult);
    ADD_TEST(Test_MM_DumpMemToFileCmd_lseekError);
    ADD_TEST(Test_MM_DumpMemToFileCmd_SymNameError);
    ADD_TEST(Test_MM_DumpMemToFileCmd_NoVerifyDumpParams);
    ADD_TEST(Test_MM_DumpMemToFileCmd_NoWriteHeaders);
    ADD_TEST(Test_MM_DumpMemToFileCmd_BadType);
    ADD_TEST(Test_MM_DumpMemToFileCmd_ReWriteHeadersErr);
    ADD_TEST(Test_MM_DumpInEventCmd_Nominal);
    ADD_TEST(Test_MM_DumpInEventCmd_SymNameError);
    ADD_TEST(Test_MM_DumpInEventCmd_NoVerifyDumpParams);
    ADD_TEST(Test_MM_DumpInEventCmd_FillDumpInvalid);
}
