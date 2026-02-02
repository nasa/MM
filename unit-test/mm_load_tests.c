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
 *   Unit tests for mm_load.c
 */

/************************************************************************
** Includes
*************************************************************************/
#include "mm_eventids.h"
#include "mm_filedefs.h"
#include "mm_load.h"
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

/*
 * Function Definitions
 */

void Test_MM_PokeMem_NoDataSize(void)
{
    MM_PokeCmd_t CmdPacket;
    cpuaddr      DestAddress;
    CFE_Status_t Result;

    CmdPacket.Payload.MemType  = MM_MemType_RAM;
    CmdPacket.Payload.DataSize = 0;
    DestAddress                = 1;

    /* Execute the function being tested */
    Result = MM_PokeMem(&CmdPacket, DestAddress);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR_NOT_IMPLEMENTED, "Result == CFE_PSP_ERROR_NOT_IMPLEMENTED");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_PSP_WRITE_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "PSP write memory error: RC=0x%08X, Address=%p, MemType=MEM%u");
}

void Test_MM_PokeMem_8bit(void)
{
    MM_PokeCmd_t CmdPacket;
    cpuaddr      DestAddress;
    CFE_Status_t Result;

    CmdPacket.Payload.MemType  = MM_MemType_RAM;
    CmdPacket.Payload.DataSize = MM_INTERNAL_BYTE_BIT_WIDTH;
    CmdPacket.Payload.Data     = (uint8)(5);
    DestAddress                = 1;

    UT_SetDefaultReturnValue(UT_KEY(CFE_PSP_MemWrite8), CFE_PSP_SUCCESS);

    /* Execute the function being tested */
    Result = MM_PokeMem(&CmdPacket, DestAddress);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_SUCCESS, "Result == CFE_PSP_SUCCESS");

    UtAssert_True(MM_AppData.HkTlm.Payload.LastAction == MM_LastAction_POKE,
                  "MM_AppData.HkTlm.Payload.LastAction == MM_LastAction_POKE");
    UtAssert_True(MM_AppData.HkTlm.Payload.MemType == MM_MemType_RAM,
                  "MM_AppData.HkTlm.Payload.MemType == MM_MemType_RAM");
    UtAssert_ADDRESS_EQ(CFE_ES_MEMADDRESS_TO_PTR(MM_AppData.HkTlm.Payload.Address), DestAddress);
    UtAssert_True(MM_AppData.HkTlm.Payload.DataValue == 5, "MM_AppData.HkTlm.Payload.DataValue  == 5");
    UtAssert_True(MM_AppData.HkTlm.Payload.BytesProcessed == 1, "MM_AppData.HkTlm.Payload.BytesProcessed == 1");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_POKE_BYTE_INF_EID,
                         CFE_EVS_EventType_INFORMATION,
                         "Poke Command: Addr = %p, Size = %u bits, Data = 0x%08X");
}

void Test_MM_PokeMem_8bitError(void)
{
    MM_PokeCmd_t CmdPacket;
    cpuaddr      DestAddress;
    CFE_Status_t Result;

    CmdPacket.Payload.MemType  = MM_MemType_RAM;
    CmdPacket.Payload.DataSize = MM_INTERNAL_BYTE_BIT_WIDTH;
    CmdPacket.Payload.Data     = (uint8)(5);

    /* CFE_PSP_MemWrite8 stub returns success with non-zero address */
    DestAddress = 0;

    /* Set to generate error message MM_PSP_WRITE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemWrite8), 1, CFE_PSP_ERROR_NOT_IMPLEMENTED);

    /* Execute the function being tested */
    Result = MM_PokeMem(&CmdPacket, DestAddress);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR_NOT_IMPLEMENTED, "Result == CFE_PSP_ERROR_NOT_IMPLEMENTED");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_PSP_WRITE_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "PSP write memory error: RC=0x%08X, Address=%p, MemType=MEM%u");
}

void Test_MM_PokeMem_16bit(void)
{
    MM_PokeCmd_t CmdPacket;
    cpuaddr      DestAddress;
    CFE_Status_t Result;

    CmdPacket.Payload.MemType  = MM_MemType_RAM;
    CmdPacket.Payload.DataSize = MM_INTERNAL_WORD_BIT_WIDTH;
    CmdPacket.Payload.Data     = (uint16)(5);

    DestAddress = 1;

    UT_SetDefaultReturnValue(UT_KEY(CFE_PSP_MemWrite16), CFE_PSP_SUCCESS);

    /* Execute the function being tested */
    Result = MM_PokeMem(&CmdPacket, DestAddress);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_SUCCESS, "Result == CFE_PSP_SUCCESS");

    UtAssert_True(MM_AppData.HkTlm.Payload.LastAction == MM_LastAction_POKE,
                  "MM_AppData.HkTlm.Payload.LastAction == MM_LastAction_POKE");
    UtAssert_True(MM_AppData.HkTlm.Payload.MemType == MM_MemType_RAM,
                  "MM_AppData.HkTlm.Payload.MemType == MM_MemType_RAM");
    UtAssert_ADDRESS_EQ(CFE_ES_MEMADDRESS_TO_PTR(MM_AppData.HkTlm.Payload.Address), DestAddress);
    UtAssert_True(MM_AppData.HkTlm.Payload.DataValue == 5, "MM_AppData.HkTlm.Payload.DataValue  == 5");
    UtAssert_True(MM_AppData.HkTlm.Payload.BytesProcessed == 2, "MM_AppData.HkTlm.Payload.BytesProcessed == 2");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_POKE_WORD_INF_EID,
                         CFE_EVS_EventType_INFORMATION,
                         "Poke Command: Addr = %p, Size = %u bits, Data = 0x%08X");
}

void Test_MM_PokeMem_16bitError(void)
{
    MM_PokeCmd_t CmdPacket;
    cpuaddr      DestAddress;
    CFE_Status_t Result;

    CmdPacket.Payload.MemType  = MM_MemType_RAM;
    CmdPacket.Payload.DataSize = MM_INTERNAL_WORD_BIT_WIDTH;
    CmdPacket.Payload.Data     = (uint16)(5);

    /* CFE_PSP_MemWrite16 stub returns success with non-zero address */
    DestAddress = 0;

    /* Set to generate error message MM_PSP_WRITE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemWrite16), 1, CFE_PSP_ERROR_NOT_IMPLEMENTED);

    /* Execute the function being tested */
    Result = MM_PokeMem(&CmdPacket, DestAddress);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR_NOT_IMPLEMENTED, "Result == CFE_PSP_ERROR_NOT_IMPLEMENTED");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_PSP_WRITE_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "PSP write memory error: RC=0x%08X, Address=%p, MemType=MEM%u");
}

void Test_MM_PokeMem_32bit(void)
{
    MM_PokeCmd_t CmdPacket;
    cpuaddr      DestAddress;
    CFE_Status_t Result;

    CmdPacket.Payload.MemType  = MM_MemType_RAM;
    CmdPacket.Payload.DataSize = MM_INTERNAL_DWORD_BIT_WIDTH;
    CmdPacket.Payload.Data     = (uint32)(5);

    DestAddress = 1;

    /* Execute the function being tested */
    Result = MM_PokeMem(&CmdPacket, DestAddress);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_SUCCESS, "Result == CFE_PSP_SUCCESS");

    UtAssert_True(MM_AppData.HkTlm.Payload.LastAction == MM_LastAction_POKE,
                  "MM_AppData.HkTlm.Payload.LastAction == MM_LastAction_POKE");
    UtAssert_True(MM_AppData.HkTlm.Payload.MemType == MM_MemType_RAM,
                  "MM_AppData.HkTlm.Payload.MemType == MM_MemType_RAM");
    UtAssert_ADDRESS_EQ(CFE_ES_MEMADDRESS_TO_PTR(MM_AppData.HkTlm.Payload.Address), DestAddress);
    UtAssert_True(MM_AppData.HkTlm.Payload.DataValue == 5, "MM_AppData.HkTlm.Payload.DataValue  == 5");
    UtAssert_True(MM_AppData.HkTlm.Payload.BytesProcessed == 4, "MM_AppData.HkTlm.Payload.BytesProcessed == 4");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_POKE_DWORD_INF_EID,
                         CFE_EVS_EventType_INFORMATION,
                         "Poke Command: Addr = %p, Size = %u bits, Data = 0x%08X");
}

void Test_MM_PokeMem_32bitError(void)
{
    MM_PokeCmd_t CmdPacket;
    cpuaddr      DestAddress;
    CFE_Status_t Result;

    CmdPacket.Payload.MemType  = MM_MemType_RAM;
    CmdPacket.Payload.DataSize = MM_INTERNAL_DWORD_BIT_WIDTH;
    CmdPacket.Payload.Data     = (uint32)(5);

    /* CFE_PSP_MemWrite32 stub returns success with non-zero address */
    DestAddress = 0;

    /* Set to generate error message MM_PSP_WRITE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_MemWrite32), 1, CFE_PSP_ERROR_NOT_IMPLEMENTED);

    /* Execute the function being tested */
    Result = MM_PokeMem(&CmdPacket, DestAddress);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR_NOT_IMPLEMENTED, "Result == CFE_PSP_ERROR_NOT_IMPLEMENTED");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_PSP_WRITE_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "PSP write memory error: RC=0x%08X, Address=%p, MemType=MEM%u");
}

void Test_MM_PokeEeprom_NoDataSize(void)
{
    MM_PokeCmd_t CmdPacket;
    cpuaddr      DestAddress;
    CFE_Status_t Result;

    CmdPacket.Payload.MemType  = MM_MemType_EEPROM;
    CmdPacket.Payload.DataSize = 0;

    DestAddress = 1;

    /* Execute the function being tested */
    Result = MM_PokeEeprom(&CmdPacket, DestAddress);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR_NOT_IMPLEMENTED, "Result == CFE_PSP_ERROR_NOT_IMPLEMENTED");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_PokeEeprom_8bit(void)
{
    MM_PokeCmd_t CmdPacket;
    cpuaddr      DestAddress;
    CFE_Status_t Result;

    CmdPacket.Payload.MemType  = MM_MemType_EEPROM;
    CmdPacket.Payload.DataSize = MM_INTERNAL_BYTE_BIT_WIDTH;
    CmdPacket.Payload.Data     = (uint8)(5);

    DestAddress = 1;

    /* Execute the function being tested */
    Result = MM_PokeEeprom(&CmdPacket, DestAddress);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_SUCCESS, "Result == CFE_PSP_SUCCESS");

    UtAssert_True(MM_AppData.HkTlm.Payload.LastAction == MM_LastAction_POKE,
                  "MM_AppData.HkTlm.Payload.LastAction == MM_LastAction_POKE");
    UtAssert_True(MM_AppData.HkTlm.Payload.MemType == MM_MemType_EEPROM,
                  "MM_AppData.HkTlm.Payload.MemType == MM_MemType_EEPROM");
    UtAssert_ADDRESS_EQ(CFE_ES_MEMADDRESS_TO_PTR(MM_AppData.HkTlm.Payload.Address), DestAddress);
    UtAssert_True(MM_AppData.HkTlm.Payload.DataValue == 5, "MM_AppData.HkTlm.Payload.DataValue  == 5");
    UtAssert_True(MM_AppData.HkTlm.Payload.BytesProcessed == 1, "MM_AppData.HkTlm.Payload.BytesProcessed == 1");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_POKE_BYTE_INF_EID,
                         CFE_EVS_EventType_INFORMATION,
                         "Poke Command: Addr = %p, Size = 8 bits, Data = 0x%02X");
}

void Test_MM_PokeEeprom_8bitError(void)
{
    MM_PokeCmd_t CmdPacket;
    cpuaddr      DestAddress;
    CFE_Status_t Result;

    CmdPacket.Payload.MemType  = MM_MemType_EEPROM;
    CmdPacket.Payload.DataSize = MM_INTERNAL_BYTE_BIT_WIDTH;
    CmdPacket.Payload.Data     = (uint8)(5);

    /* CFE_PSP_MemWrite8 stub returns success with non-zero address */
    DestAddress = 0;

    /* Set to generate error message MM_OS_EEPROMWRITE8_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_EepromWrite8), 1, CFE_PSP_ERROR_NOT_IMPLEMENTED);

    /* Execute the function being tested */
    Result = MM_PokeEeprom(&CmdPacket, DestAddress);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR_NOT_IMPLEMENTED, "Result == CFE_PSP_ERROR_NOT_IMPLEMENTED");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_OS_EEPROMWRITE8_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "CFE_PSP_EepromWrite8 error received: RC = 0x%08X, Addr = %p");
}

void Test_MM_PokeEeprom_16bit(void)
{
    MM_PokeCmd_t CmdPacket;
    cpuaddr      DestAddress;
    CFE_Status_t Result;

    CmdPacket.Payload.MemType  = MM_MemType_EEPROM;
    CmdPacket.Payload.DataSize = MM_INTERNAL_WORD_BIT_WIDTH;
    CmdPacket.Payload.Data     = (uint16)(5);

    DestAddress = 1;

    /* Execute the function being tested */
    Result = MM_PokeEeprom(&CmdPacket, DestAddress);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_SUCCESS, "Result == CFE_PSP_SUCCESS");

    UtAssert_True(MM_AppData.HkTlm.Payload.LastAction == MM_LastAction_POKE,
                  "MM_AppData.HkTlm.Payload.LastAction == MM_LastAction_POKE");
    UtAssert_True(MM_AppData.HkTlm.Payload.MemType == MM_MemType_EEPROM,
                  "MM_AppData.HkTlm.Payload.MemType == MM_MemType_EEPROM");
    UtAssert_ADDRESS_EQ(CFE_ES_MEMADDRESS_TO_PTR(MM_AppData.HkTlm.Payload.Address), DestAddress);
    UtAssert_True(MM_AppData.HkTlm.Payload.DataValue == 5, "MM_AppData.HkTlm.Payload.DataValue  == 5");
    UtAssert_True(MM_AppData.HkTlm.Payload.BytesProcessed == 2, "MM_AppData.HkTlm.Payload.BytesProcessed == 2");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_POKE_WORD_INF_EID,
                         CFE_EVS_EventType_INFORMATION,
                         "Poke Command: Addr = %p, Size = 16 bits, Data = 0x%04X");
}

void Test_MM_PokeEeprom_16bitError(void)
{
    MM_PokeCmd_t CmdPacket;
    cpuaddr      DestAddress;
    CFE_Status_t Result;

    CmdPacket.Payload.MemType  = MM_MemType_EEPROM;
    CmdPacket.Payload.DataSize = MM_INTERNAL_WORD_BIT_WIDTH;
    CmdPacket.Payload.Data     = (uint16)(5);

    /* CFE_PSP_MemWrite16 stub returns success with non-zero address */
    DestAddress = 0;

    /* Set to generate error message MM_OS_EEPROMWRITE16_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_EepromWrite16), 1, CFE_PSP_ERROR_NOT_IMPLEMENTED);

    /* Execute the function being tested */
    Result = MM_PokeEeprom(&CmdPacket, DestAddress);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR_NOT_IMPLEMENTED, "Result == CFE_PSP_ERROR_NOT_IMPLEMENTED");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_OS_EEPROMWRITE16_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "CFE_PSP_EepromWrite16 error received: RC = 0x%08X, Addr = %p");
}

void Test_MM_PokeEeprom_32bit(void)
{
    MM_PokeCmd_t CmdPacket;
    cpuaddr      DestAddress;
    CFE_Status_t Result;

    CmdPacket.Payload.MemType  = MM_MemType_EEPROM;
    CmdPacket.Payload.DataSize = MM_INTERNAL_DWORD_BIT_WIDTH;
    CmdPacket.Payload.Data     = (uint32)(5);

    DestAddress = 1;

    /* Execute the function being tested */
    Result = MM_PokeEeprom(&CmdPacket, DestAddress);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_SUCCESS, "Result == CFE_PSP_SUCCESS");

    UtAssert_True(MM_AppData.HkTlm.Payload.LastAction == MM_LastAction_POKE,
                  "MM_AppData.HkTlm.Payload.LastAction == MM_LastAction_POKE");
    UtAssert_True(MM_AppData.HkTlm.Payload.MemType == MM_MemType_EEPROM,
                  "MM_AppData.HkTlm.Payload.MemType == MM_MemType_EEPROM");
    UtAssert_ADDRESS_EQ(CFE_ES_MEMADDRESS_TO_PTR(MM_AppData.HkTlm.Payload.Address), DestAddress);
    UtAssert_True(MM_AppData.HkTlm.Payload.DataValue == 5, "MM_AppData.HkTlm.Payload.DataValue  == 5");
    UtAssert_True(MM_AppData.HkTlm.Payload.BytesProcessed == 4, "MM_AppData.HkTlm.Payload.BytesProcessed == 4");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_POKE_DWORD_INF_EID,
                         CFE_EVS_EventType_INFORMATION,
                         "Poke Command: Addr = %p, Size = 32 bits, Data = 0x%08X");
}

void Test_MM_PokeEeprom_32bitError(void)
{
    MM_PokeCmd_t CmdPacket;
    cpuaddr      DestAddress;
    CFE_Status_t Result;

    CmdPacket.Payload.MemType  = MM_MemType_EEPROM;
    CmdPacket.Payload.DataSize = MM_INTERNAL_DWORD_BIT_WIDTH;
    CmdPacket.Payload.Data     = (uint32)(5);

    /* CFE_PSP_MemWrite32 stub returns success with non-zero address */
    DestAddress = 0;

    /* Set to generate error message MM_OS_EEPROMWRITE32_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_EepromWrite32), 1, CFE_PSP_ERROR_NOT_IMPLEMENTED);

    /* Execute the function being tested */
    Result = MM_PokeEeprom(&CmdPacket, DestAddress);

    /* Verify results */
    UtAssert_True(Result == CFE_PSP_ERROR_NOT_IMPLEMENTED, "Result == CFE_PSP_ERROR_NOT_IMPLEMENTED");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_OS_EEPROMWRITE32_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "CFE_PSP_EepromWrite32 error received: RC = 0x%08X, Addr = %p");
}

void Test_MM_LoadMemFromFile_PreventCPUHogging(void)
{
    int32                   Result;
    MM_LoadDumpFileHeader_t FileHeader;
    char                    FileName[] = "filename";
    cpuaddr                 DestAddress;
    uint8                   OutBuff[(2 * MM_INTERNAL_MAX_LOAD_DATA_SEG) + 2];

    memset(MM_AppData.LoadBuffer, 0, sizeof(uint32) * (MM_INTERNAL_MAX_LOAD_DATA_SEG / 4));
    FileHeader.MemType    = MM_MemType_EEPROM;
    FileHeader.NumOfBytes = 2 * MM_INTERNAL_MAX_LOAD_DATA_SEG;
    DestAddress           = (cpuaddr)OutBuff;

    /* Set to satisfy condition "(ReadLength = OS_read(FileHandle, ioBuffer,
     * SegmentSize)) == SegmentSize" */
    UT_SetDefaultReturnValue(UT_KEY(OS_read), MM_INTERNAL_MAX_LOAD_DATA_SEG);

    /* Execute the function being tested */
    Result = MM_LoadMemFromFile(MM_UT_OBJID_1, FileName, &FileHeader, DestAddress);

    /* Verify results */
    UtAssert_True(Result == OS_SUCCESS, "Result == OS_SUCCESS");
    UtAssert_True(MM_AppData.HkTlm.Payload.LastAction == MM_LastAction_LOAD_FROM_FILE,
                  "MM_AppData.HkTlm.Payload.LastAction == MM_LastAction_LOAD_FROM_FILE");
    UtAssert_True(MM_AppData.HkTlm.Payload.MemType == MM_MemType_EEPROM,
                  "MM_AppData.HkTlm.Payload.MemType == MM_MemType_EEPROM");
    UtAssert_ADDRESS_EQ(CFE_ES_MEMADDRESS_TO_PTR(MM_AppData.HkTlm.Payload.Address), DestAddress);
    UtAssert_True(MM_AppData.HkTlm.Payload.BytesProcessed == 2 * MM_INTERNAL_MAX_LOAD_DATA_SEG,
                  "MM_AppData.HkTlm.Payload.BytesProcessed == "
                  "2*MM_INTERNAL_MAX_LOAD_DATA_SEG");
    UtAssert_STRINGBUF_EQ(MM_AppData.HkTlm.Payload.FileName,
                          sizeof(MM_AppData.HkTlm.Payload.FileName),
                          FileName,
                          sizeof(FileName));

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_LoadMemFromFile_ReadError(void)
{
    int32                   Result;
    MM_LoadDumpFileHeader_t FileHeader;
    cpuaddr                 DestAddress;

    FileHeader.MemType    = MM_MemType_EEPROM;
    FileHeader.NumOfBytes = 2;
    DestAddress           = 0;

    /* Set to generate error message MM_OS_READ_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(OS_read), 1, 0);

    /* Execute the function being tested */
    Result = MM_LoadMemFromFile(MM_UT_OBJID_1, (char *)"filename", &FileHeader, DestAddress);

    /* Verify results */
    UtAssert_INT32_EQ(Result, OS_ERROR);

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_OS_READ_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "OS_read error received: RC = 0x%08X Expected = %u File = '%s'");
}

void Test_MM_LoadMemFromFile_NotEepromMemType(void)
{
    int32                   Result;
    MM_LoadDumpFileHeader_t FileHeader;
    char                    FileName[] = "filename";
    cpuaddr                 DestAddress;
    uint8                   OutBuff[(2 * MM_INTERNAL_MAX_LOAD_DATA_SEG) + 2];

    FileHeader.MemType    = MM_MemType_MEM8;
    FileHeader.NumOfBytes = 2 * MM_INTERNAL_MAX_LOAD_DATA_SEG;
    DestAddress           = (cpuaddr)OutBuff;

    /* Set to satisfy condition "(ReadLength = OS_read(FileHandle, ioBuffer,
     * SegmentSize)) == SegmentSize" */
    UT_SetDefaultReturnValue(UT_KEY(OS_read), MM_INTERNAL_MAX_LOAD_DATA_SEG);

    /* Execute the function being tested */
    Result = MM_LoadMemFromFile(MM_UT_OBJID_1, FileName, &FileHeader, DestAddress);

    /* Verify results */
    UtAssert_True(Result == OS_SUCCESS, "Result == OS_SUCCESS");
    UtAssert_True(MM_AppData.HkTlm.Payload.LastAction == MM_LastAction_LOAD_FROM_FILE,
                  "MM_AppData.HkTlm.Payload.LastAction == MM_LastAction_LOAD_FROM_FILE");
    UtAssert_True(MM_AppData.HkTlm.Payload.MemType == MM_MemType_MEM8,
                  "MM_AppData.HkTlm.Payload.MemType == MM_MemType_MEM8");
    UtAssert_ADDRESS_EQ(CFE_ES_MEMADDRESS_TO_PTR(MM_AppData.HkTlm.Payload.Address), DestAddress);
    UtAssert_True(MM_AppData.HkTlm.Payload.BytesProcessed == 2 * MM_INTERNAL_MAX_LOAD_DATA_SEG,
                  "MM_AppData.HkTlm.Payload.BytesProcessed == "
                  "2*MM_INTERNAL_MAX_LOAD_DATA_SEG");
    UtAssert_STRINGBUF_EQ(MM_AppData.HkTlm.Payload.FileName,
                          sizeof(MM_AppData.HkTlm.Payload.FileName),
                          FileName,
                          sizeof(FileName));

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_VerifyLoadFileSize_Nominal(void)
{
    int32                   Status;
    MM_LoadDumpFileHeader_t FileHeader;
    os_fstat_t              FileStats;

    memset(&FileHeader, 0, sizeof(MM_LoadDumpFileHeader_t));
    FileHeader.NumOfBytes = 1;

    memset(&FileStats, 0, sizeof(os_fstat_t));
    FileStats.FileSize = FileHeader.NumOfBytes + sizeof(CFE_FS_Header_t) + sizeof(MM_LoadDumpFileHeader_t);

    UT_SetDefaultReturnValue(UT_KEY(OS_stat), OS_SUCCESS);
    UT_SetDataBuffer(UT_KEY(OS_stat), &FileStats, sizeof(os_fstat_t), false);

    /* Execute the function being tested */
    Status = MM_VerifyLoadFileSize(NULL, &FileHeader);

    /* Verify results */
    UtAssert_EQ(int32, Status, OS_SUCCESS);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_VerifyLoadFileSize_StatError(void)
{
    int32                   Result;
    MM_LoadDumpFileHeader_t FileHeader;

    /* Set to generate error message MM_OS_MEMVALIDATE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(OS_stat), 1, OS_FS_ERR_PATH_INVALID);

    /* Execute the function being tested */
    Result = MM_VerifyLoadFileSize((char *)"filename", &FileHeader);

    /* Verify results */
    UtAssert_True(Result == OS_FS_ERR_PATH_INVALID, "Result == OS_FS_ERR_PATH_INVALID");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_OS_STAT_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "OS_stat error received: RC = 0x%08X File = '%s'");
}

void Test_MM_VerifyLoadFileSize_SizeError(void)
{
    int32                   Result;
    MM_LoadDumpFileHeader_t FileHeader;

    FileHeader.NumOfBytes = 99;

    /* Execute the function being tested */
    Result = MM_VerifyLoadFileSize((char *)"filename", &FileHeader);

    /* Verify results */
    UtAssert_True(Result == OS_ERR_INVALID_SIZE, "Result == OS_ERR_INVALID_SIZE");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_LD_FILE_SIZE_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "Load file size error: Reported by OS = %d Expected = %u File = '%s'");
}

void Test_MM_ReadFileHeaders_Nominal(void)
{
    int32                   Status;
    osal_id_t               FileHandle = MM_UT_OBJID_1;
    CFE_FS_Header_t         CFEHeader;
    MM_LoadDumpFileHeader_t MMHeader;

    UT_SetDefaultReturnValue(UT_KEY(CFE_FS_ReadHeader), sizeof(CFE_FS_Header_t));
    UT_SetDefaultReturnValue(UT_KEY(OS_read), sizeof(MM_LoadDumpFileHeader_t));

    /* Execute the function being tested */
    Status = MM_ReadFileHeaders(NULL, FileHandle, &CFEHeader, &MMHeader);

    /* Verify results */
    UtAssert_EQ(int32, Status, OS_SUCCESS);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void Test_MM_ReadFileHeaders_ReadHeaderError(void)
{
    int32                   Result;
    osal_id_t               FileHandle = MM_UT_OBJID_1;
    CFE_FS_Header_t         CFEHeader;
    MM_LoadDumpFileHeader_t MMHeader;

    /* Set to satisfy condition "OS_Status != sizeof(CFE_FS_Header_t)" */
    UT_SetDeferredRetcode(UT_KEY(CFE_FS_ReadHeader), 1, CFE_FS_BAD_ARGUMENT);

    /* Execute the function being tested */
    Result = MM_ReadFileHeaders((char *)"filename", FileHandle, &CFEHeader, &MMHeader);

    /* Verify results */
    UtAssert_True(Result == OS_ERR_INVALID_SIZE, "Result == OS_ERR_INVALID_SIZE");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_CFE_FS_READHDR_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "CFE_FS_ReadHeader error received: RC = 0x%08X Expected "
                         "= %u File = '%s'");
}

void Test_MM_ReadFileHeaders_ReadError(void)
{
    int32                   Result;
    osal_id_t               FileHandle = MM_UT_OBJID_1;
    CFE_FS_Header_t         CFEHeader;
    MM_LoadDumpFileHeader_t MMHeader;

    /* Set to generate error message MM_OS_READ_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(OS_read), 1, OS_ERROR);

    /* Execute the function being tested */
    Result = MM_ReadFileHeaders((char *)"filename", FileHandle, &CFEHeader, &MMHeader);

    /* Verify results */
    UtAssert_True(Result == OS_ERR_INVALID_SIZE, "Result == OS_ERR_INVALID_SIZE");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    MM_Test_Verify_Event(0,
                         MM_OS_READ_EXP_ERR_EID,
                         CFE_EVS_EventType_ERROR,
                         "OS_read error received: RC = 0x%08X Expected = %u File = '%s'");
}

void Test_MM_FillMem_Eeprom(void)
{
    MM_FillMemCmd_t CmdPacket;
    uint8           OutBuff[(2 * MM_INTERNAL_MAX_LOAD_DATA_SEG) + 2];
    cpuaddr         DestAddress;

    memset(&CmdPacket, 0, sizeof(CmdPacket));

    CmdPacket.Payload.MemType    = MM_MemType_EEPROM;
    CmdPacket.Payload.NumOfBytes = 2;
    DestAddress                  = (cpuaddr)OutBuff;

    /* Execute the function being tested */
    MM_FillMem(DestAddress, &CmdPacket);

    /* Verify results */
    UtAssert_True(MM_AppData.HkTlm.Payload.LastAction == MM_LastAction_FILL,
                  "MM_AppData.HkTlm.Payload.LastAction == MM_LastAction_FILL");
    UtAssert_True(MM_AppData.HkTlm.Payload.MemType == CmdPacket.Payload.MemType,
                  "MM_AppData.HkTlm.Payload.MemType == CmdPacket.Payload.MemType");
    UtAssert_ADDRESS_EQ(CFE_ES_MEMADDRESS_TO_PTR(MM_AppData.HkTlm.Payload.Address), DestAddress);
    UtAssert_True(MM_AppData.HkTlm.Payload.DataValue == CmdPacket.Payload.FillPattern,
                  "MM_AppData.HkTlm.Payload.DataValue == CmdPacket.Payload.FillPattern");
    UtAssert_True(MM_AppData.HkTlm.Payload.BytesProcessed == 2, "MM_AppData.HkTlm.Payload.BytesProcessed == 2");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
    UtAssert_STUB_COUNT(CFE_ES_PerfLogAdd, 2);
}

void Test_MM_FillMem_NonEeprom(void)
{
    MM_FillMemCmd_t CmdPacket;
    uint8           OutBuff[(2 * MM_INTERNAL_MAX_LOAD_DATA_SEG) + 2];
    cpuaddr         DestAddress;

    memset(&CmdPacket, 0, sizeof(CmdPacket));

    CmdPacket.Payload.MemType    = MM_MemType_RAM;
    CmdPacket.Payload.NumOfBytes = 2;
    DestAddress                  = (cpuaddr)OutBuff;

    /* Execute the function being tested */
    MM_FillMem(DestAddress, &CmdPacket);

    /* Verify results */
    UtAssert_True(MM_AppData.HkTlm.Payload.LastAction == MM_LastAction_FILL,
                  "MM_AppData.HkTlm.Payload.LastAction == MM_LastAction_FILL");
    UtAssert_True(MM_AppData.HkTlm.Payload.MemType == CmdPacket.Payload.MemType,
                  "MM_AppData.HkTlm.Payload.MemType == CmdPacket.Payload.MemType");
    UtAssert_ADDRESS_EQ(CFE_ES_MEMADDRESS_TO_PTR(MM_AppData.HkTlm.Payload.Address), DestAddress);
    UtAssert_True(MM_AppData.HkTlm.Payload.DataValue == CmdPacket.Payload.FillPattern,
                  "MM_AppData.HkTlm.Payload.DataValue == CmdPacket.Payload.FillPattern");
    UtAssert_True(MM_AppData.HkTlm.Payload.BytesProcessed == 2, "MM_AppData.HkTlm.Payload.BytesProcessed == 2");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
    UtAssert_STUB_COUNT(CFE_ES_PerfLogAdd, 0);
}

void Test_MM_FillMem_MaxFillDataSegment(void)
{
    MM_FillMemCmd_t CmdPacket;
    uint8           OutBuff[(2 * MM_INTERNAL_MAX_LOAD_DATA_SEG) + 2];
    cpuaddr         DestAddress;

    memset(&CmdPacket, 0, sizeof(CmdPacket));

    CmdPacket.Payload.MemType    = MM_MemType_EEPROM;
    CmdPacket.Payload.NumOfBytes = MM_INTERNAL_MAX_FILL_DATA_SEG + 1;
    DestAddress                  = (cpuaddr)OutBuff;

    /* Execute the function being tested */
    MM_FillMem(DestAddress, &CmdPacket);

    /* Verify results */
    UtAssert_True(MM_AppData.HkTlm.Payload.LastAction == MM_LastAction_FILL,
                  "MM_AppData.HkTlm.Payload.LastAction == MM_LastAction_FILL");
    UtAssert_True(MM_AppData.HkTlm.Payload.MemType == CmdPacket.Payload.MemType,
                  "MM_AppData.HkTlm.Payload.MemType == CmdPacket.Payload.MemType");
    UtAssert_ADDRESS_EQ(CFE_ES_MEMADDRESS_TO_PTR(MM_AppData.HkTlm.Payload.Address), DestAddress);
    UtAssert_True(MM_AppData.HkTlm.Payload.DataValue == CmdPacket.Payload.FillPattern,
                  "MM_AppData.HkTlm.Payload.DataValue == CmdPacket.Payload.FillPattern");
    UtAssert_True(MM_AppData.HkTlm.Payload.BytesProcessed == MM_INTERNAL_MAX_FILL_DATA_SEG + 1,
                  "MM_AppData.HkTlm.Payload.BytesProcessed == "
                  "MM_INTERNAL_MAX_FILL_DATA_SEG + 1");

    /* No command-handling function should be updating the cmd or err counter
     * itself */
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.CmdCounter, 0);
    UtAssert_INT32_EQ(MM_AppData.HkTlm.Payload.ErrCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

/*
 * Register the test cases to execute with the unit test tool
 */
void UtTest_Setup(void)
{
    ADD_TEST(Test_MM_PokeMem_NoDataSize);
    ADD_TEST(Test_MM_PokeMem_8bit);
    ADD_TEST(Test_MM_PokeMem_8bitError);
    ADD_TEST(Test_MM_PokeMem_16bit);
    ADD_TEST(Test_MM_PokeMem_16bitError);
    ADD_TEST(Test_MM_PokeMem_32bit);
    ADD_TEST(Test_MM_PokeMem_32bitError);
    ADD_TEST(Test_MM_PokeEeprom_NoDataSize);
    ADD_TEST(Test_MM_PokeEeprom_8bit);
    ADD_TEST(Test_MM_PokeEeprom_8bitError);
    ADD_TEST(Test_MM_PokeEeprom_16bit);
    ADD_TEST(Test_MM_PokeEeprom_16bitError);
    ADD_TEST(Test_MM_PokeEeprom_32bit);
    ADD_TEST(Test_MM_PokeEeprom_32bitError);
    ADD_TEST(Test_MM_LoadMemFromFile_PreventCPUHogging);
    ADD_TEST(Test_MM_LoadMemFromFile_ReadError);
    ADD_TEST(Test_MM_LoadMemFromFile_NotEepromMemType);
    ADD_TEST(Test_MM_VerifyLoadFileSize_Nominal);
    ADD_TEST(Test_MM_VerifyLoadFileSize_StatError);
    ADD_TEST(Test_MM_VerifyLoadFileSize_SizeError);
    ADD_TEST(Test_MM_ReadFileHeaders_Nominal);
    ADD_TEST(Test_MM_ReadFileHeaders_ReadHeaderError);
    ADD_TEST(Test_MM_ReadFileHeaders_ReadError);
    ADD_TEST(Test_MM_FillMem_Eeprom);
    ADD_TEST(Test_MM_FillMem_NonEeprom);
    ADD_TEST(Test_MM_FillMem_MaxFillDataSegment);
}
