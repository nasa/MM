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
 *
 * Auto-Generated stub implementations for functions defined in mm_load header
 */

#include "mm_load.h"
#include "utgenstub.h"

/*
 * ----------------------------------------------------
 * Generated stub function for MM_FillMem()
 * ----------------------------------------------------
 */
void MM_FillMem(cpuaddr DestAddress, const MM_FillMemCmd_t *CmdPtr)
{
    UT_GenStub_AddParam(MM_FillMem, cpuaddr, DestAddress);
    UT_GenStub_AddParam(MM_FillMem, const MM_FillMemCmd_t *, CmdPtr);

    UT_GenStub_Execute(MM_FillMem, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for MM_LoadMemFromFile()
 * ----------------------------------------------------
 */
int32 MM_LoadMemFromFile(osal_id_t                      FileHandle,
                         const char                    *FileName,
                         const MM_LoadDumpFileHeader_t *FileHeader,
                         cpuaddr                        DestAddress)
{
    UT_GenStub_SetupReturnBuffer(MM_LoadMemFromFile, int32);

    UT_GenStub_AddParam(MM_LoadMemFromFile, osal_id_t, FileHandle);
    UT_GenStub_AddParam(MM_LoadMemFromFile, const char *, FileName);
    UT_GenStub_AddParam(MM_LoadMemFromFile, const MM_LoadDumpFileHeader_t *, FileHeader);
    UT_GenStub_AddParam(MM_LoadMemFromFile, cpuaddr, DestAddress);

    UT_GenStub_Execute(MM_LoadMemFromFile, Basic, NULL);

    return UT_GenStub_GetReturnValue(MM_LoadMemFromFile, int32);
}

/*
 * ----------------------------------------------------
 * Generated stub function for MM_LoadMemWID()
 * ----------------------------------------------------
 */
bool MM_LoadMemWID(const MM_LoadMemWIDCmd_t *CmdPtr, cpuaddr DestAddress)
{
    UT_GenStub_SetupReturnBuffer(MM_LoadMemWID, bool);

    UT_GenStub_AddParam(MM_LoadMemWID, const MM_LoadMemWIDCmd_t *, CmdPtr);
    UT_GenStub_AddParam(MM_LoadMemWID, cpuaddr, DestAddress);

    UT_GenStub_Execute(MM_LoadMemWID, Basic, NULL);

    return UT_GenStub_GetReturnValue(MM_LoadMemWID, bool);
}

/*
 * ----------------------------------------------------
 * Generated stub function for MM_PokeEeprom()
 * ----------------------------------------------------
 */
CFE_Status_t MM_PokeEeprom(const MM_PokeCmd_t *CmdPtr, cpuaddr DestAddress)
{
    UT_GenStub_SetupReturnBuffer(MM_PokeEeprom, CFE_Status_t);

    UT_GenStub_AddParam(MM_PokeEeprom, const MM_PokeCmd_t *, CmdPtr);
    UT_GenStub_AddParam(MM_PokeEeprom, cpuaddr, DestAddress);

    UT_GenStub_Execute(MM_PokeEeprom, Basic, NULL);

    return UT_GenStub_GetReturnValue(MM_PokeEeprom, CFE_Status_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for MM_PokeMem()
 * ----------------------------------------------------
 */
CFE_Status_t MM_PokeMem(const MM_PokeCmd_t *CmdPtr, cpuaddr DestAddress)
{
    UT_GenStub_SetupReturnBuffer(MM_PokeMem, CFE_Status_t);

    UT_GenStub_AddParam(MM_PokeMem, const MM_PokeCmd_t *, CmdPtr);
    UT_GenStub_AddParam(MM_PokeMem, cpuaddr, DestAddress);

    UT_GenStub_Execute(MM_PokeMem, Basic, NULL);

    return UT_GenStub_GetReturnValue(MM_PokeMem, CFE_Status_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for MM_ReadFileHeaders()
 * ----------------------------------------------------
 */
int32 MM_ReadFileHeaders(const char              *FileName,
                         osal_id_t                FileHandle,
                         CFE_FS_Header_t         *CFEHeader,
                         MM_LoadDumpFileHeader_t *MMHeader)
{
    UT_GenStub_SetupReturnBuffer(MM_ReadFileHeaders, int32);

    UT_GenStub_AddParam(MM_ReadFileHeaders, const char *, FileName);
    UT_GenStub_AddParam(MM_ReadFileHeaders, osal_id_t, FileHandle);
    UT_GenStub_AddParam(MM_ReadFileHeaders, CFE_FS_Header_t *, CFEHeader);
    UT_GenStub_AddParam(MM_ReadFileHeaders, MM_LoadDumpFileHeader_t *, MMHeader);

    UT_GenStub_Execute(MM_ReadFileHeaders, Basic, NULL);

    return UT_GenStub_GetReturnValue(MM_ReadFileHeaders, int32);
}

/*
 * ----------------------------------------------------
 * Generated stub function for MM_VerifyLoadFileSize()
 * ----------------------------------------------------
 */
int32 MM_VerifyLoadFileSize(const char *FileName, const MM_LoadDumpFileHeader_t *FileHeader)
{
    UT_GenStub_SetupReturnBuffer(MM_VerifyLoadFileSize, int32);

    UT_GenStub_AddParam(MM_VerifyLoadFileSize, const char *, FileName);
    UT_GenStub_AddParam(MM_VerifyLoadFileSize, const MM_LoadDumpFileHeader_t *, FileHeader);

    UT_GenStub_Execute(MM_VerifyLoadFileSize, Basic, NULL);

    return UT_GenStub_GetReturnValue(MM_VerifyLoadFileSize, int32);
}
