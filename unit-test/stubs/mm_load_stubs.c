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
 *   Unit test stubs for mm_load.c
 */

/************************************************************************
** Includes
*************************************************************************/
#include "mm_app.h"
#include "mm_load.h"
#include "mm_perfids.h"
#include "mm_events.h"
#include "mm_utils.h"
#include "mm_mem32.h"
#include "mm_mem16.h"
#include "mm_mem8.h"
#include "mm_mission_cfg.h"

/************************************************************************
** UT Includes
*************************************************************************/
#include "uttest.h"
#include "utassert.h"
#include "utstubs.h"

bool MM_PokeMem(const MM_PokeCmd_t *CmdPtr, cpuaddr DestAddress)
{
    UT_Stub_RegisterContext(UT_KEY(MM_PokeMem), CmdPtr);
    UT_Stub_RegisterContextGenericArg(UT_KEY(MM_PokeMem), DestAddress);
    return UT_DEFAULT_IMPL(MM_PokeMem);
}

bool MM_PokeEeprom(const MM_PokeCmd_t *CmdPtr, cpuaddr DestAddress)
{
    UT_Stub_RegisterContext(UT_KEY(MM_PokeEeprom), CmdPtr);
    UT_Stub_RegisterContextGenericArg(UT_KEY(MM_PokeEeprom), DestAddress);
    return UT_DEFAULT_IMPL(MM_PokeEeprom);
}

bool MM_LoadMemWID(const MM_LoadMemWIDCmd_t *CmdPtr, cpuaddr DestAddress)
{
    UT_Stub_RegisterContext(UT_KEY(MM_LoadMemWID), CmdPtr);
    UT_Stub_RegisterContextGenericArg(UT_KEY(MM_LoadMemWID), DestAddress);
    return UT_DEFAULT_IMPL(MM_LoadMemWID);
}

bool MM_LoadMemFromFile(osal_id_t FileHandle, const char *FileName, const MM_LoadDumpFileHeader_t *FileHeader,
                        cpuaddr DestAddress)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(MM_LoadMemFromFile), FileHandle);
    UT_Stub_RegisterContext(UT_KEY(MM_LoadMemFromFile), FileName);
    UT_Stub_RegisterContext(UT_KEY(MM_LoadMemFromFile), FileHeader);
    UT_Stub_RegisterContextGenericArg(UT_KEY(MM_LoadMemFromFile), DestAddress);
    return UT_DEFAULT_IMPL(MM_LoadMemFromFile);
}

bool MM_VerifyLoadFileSize(const char *FileName, const MM_LoadDumpFileHeader_t *FileHeader)
{
    UT_Stub_RegisterContext(UT_KEY(MM_VerifyLoadFileSize), FileName);
    UT_Stub_RegisterContext(UT_KEY(MM_VerifyLoadFileSize), FileHeader);
    return UT_DEFAULT_IMPL(MM_VerifyLoadFileSize);
}

bool MM_ReadFileHeaders(const char *FileName, osal_id_t FileHandle, CFE_FS_Header_t *CFEHeader,
                        MM_LoadDumpFileHeader_t *MMHeader)
{
    UT_Stub_RegisterContext(UT_KEY(MM_ReadFileHeaders), FileName);
    UT_Stub_RegisterContextGenericArg(UT_KEY(MM_ReadFileHeaders), FileHandle);
    UT_Stub_RegisterContext(UT_KEY(MM_ReadFileHeaders), CFEHeader);
    UT_Stub_RegisterContext(UT_KEY(MM_ReadFileHeaders), MMHeader);
    return UT_DEFAULT_IMPL(MM_ReadFileHeaders);
}

bool MM_FillMem(cpuaddr DestAddr, const MM_FillMemCmd_t *CmdPtr)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(MM_FillMem), DestAddr);
    UT_Stub_RegisterContext(UT_KEY(MM_FillMem), CmdPtr);
    return UT_DEFAULT_IMPL(MM_FillMem);
}

bool MM_PokeCmd(const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContext(UT_KEY(MM_PokeCmd), BufPtr);
    return UT_DEFAULT_IMPL(MM_PokeCmd);
    CFE_EVS_SendEvent(MM_SYMNAME_ERR_EID, CFE_EVS_EventType_ERROR, "Symbolic address can't be resolved: Name = 'name'");
}

bool MM_LoadMemWIDCmd(const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContext(UT_KEY(MM_LoadMemWIDCmd), BufPtr);
    return UT_DEFAULT_IMPL(MM_LoadMemWIDCmd);
    CFE_EVS_SendEvent(MM_SYMNAME_ERR_EID, CFE_EVS_EventType_ERROR, "Symbolic address can't be resolved: Name = 'name'");
}

bool MM_LoadMemFromFileCmd(const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContext(UT_KEY(MM_LoadMemFromFileCmd), BufPtr);
    return UT_DEFAULT_IMPL(MM_LoadMemFromFileCmd);
}

bool MM_FillMemCmd(const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContext(UT_KEY(MM_FillMemCmd), BufPtr);
    return UT_DEFAULT_IMPL(MM_FillMemCmd);
    CFE_EVS_SendEvent(MM_SYMNAME_ERR_EID, CFE_EVS_EventType_ERROR, "Symbolic address can't be resolved: Name = 'name'");
}
