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
 * Auto-Generated stub implementations for functions defined in mm_dump header
 */

#include "mm_dump.h"
#include "utgenstub.h"

/*
 * ----------------------------------------------------
 * Generated stub function for MM_DumpMemToFile()
 * ----------------------------------------------------
 */
int32 MM_DumpMemToFile(osal_id_t FileHandle, const char *FileName, const MM_LoadDumpFileHeader_t *FileHeader)
{
    UT_GenStub_SetupReturnBuffer(MM_DumpMemToFile, int32);

    UT_GenStub_AddParam(MM_DumpMemToFile, osal_id_t, FileHandle);
    UT_GenStub_AddParam(MM_DumpMemToFile, const char *, FileName);
    UT_GenStub_AddParam(MM_DumpMemToFile, const MM_LoadDumpFileHeader_t *, FileHeader);

    UT_GenStub_Execute(MM_DumpMemToFile, Basic, NULL);

    return UT_GenStub_GetReturnValue(MM_DumpMemToFile, int32);
}

/*
 * ----------------------------------------------------
 * Generated stub function for MM_FillDumpInEventBuffer()
 * ----------------------------------------------------
 */
int32 MM_FillDumpInEventBuffer(cpuaddr SrcAddress, const MM_DumpInEventCmd_t *CmdPtr, void *DumpBuffer)
{
    UT_GenStub_SetupReturnBuffer(MM_FillDumpInEventBuffer, int32);

    UT_GenStub_AddParam(MM_FillDumpInEventBuffer, cpuaddr, SrcAddress);
    UT_GenStub_AddParam(MM_FillDumpInEventBuffer, const MM_DumpInEventCmd_t *, CmdPtr);
    UT_GenStub_AddParam(MM_FillDumpInEventBuffer, void *, DumpBuffer);

    UT_GenStub_Execute(MM_FillDumpInEventBuffer, Basic, NULL);

    return UT_GenStub_GetReturnValue(MM_FillDumpInEventBuffer, int32);
}

/*
 * ----------------------------------------------------
 * Generated stub function for MM_PeekMem()
 * ----------------------------------------------------
 */
int32 MM_PeekMem(const MM_PeekCmd_t *CmdPtr, cpuaddr SrcAddress)
{
    UT_GenStub_SetupReturnBuffer(MM_PeekMem, int32);

    UT_GenStub_AddParam(MM_PeekMem, const MM_PeekCmd_t *, CmdPtr);
    UT_GenStub_AddParam(MM_PeekMem, cpuaddr, SrcAddress);

    UT_GenStub_Execute(MM_PeekMem, Basic, NULL);

    return UT_GenStub_GetReturnValue(MM_PeekMem, int32);
}

/*
 * ----------------------------------------------------
 * Generated stub function for MM_WriteFileHeaders()
 * ----------------------------------------------------
 */
int32 MM_WriteFileHeaders(const char                    *FileName,
                          osal_id_t                      FileHandle,
                          CFE_FS_Header_t               *CFEHeader,
                          const MM_LoadDumpFileHeader_t *MMHeader)
{
    UT_GenStub_SetupReturnBuffer(MM_WriteFileHeaders, int32);

    UT_GenStub_AddParam(MM_WriteFileHeaders, const char *, FileName);
    UT_GenStub_AddParam(MM_WriteFileHeaders, osal_id_t, FileHandle);
    UT_GenStub_AddParam(MM_WriteFileHeaders, CFE_FS_Header_t *, CFEHeader);
    UT_GenStub_AddParam(MM_WriteFileHeaders, const MM_LoadDumpFileHeader_t *, MMHeader);

    UT_GenStub_Execute(MM_WriteFileHeaders, Basic, NULL);

    return UT_GenStub_GetReturnValue(MM_WriteFileHeaders, int32);
}
