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
 * Auto-Generated stub implementations for functions defined in mm_mem16 header
 */

#include "mm_mem16.h"
#include "utgenstub.h"

/*
 * ----------------------------------------------------
 * Generated stub function for MM_DumpMem16ToFile()
 * ----------------------------------------------------
 */
int32 MM_DumpMem16ToFile(osal_id_t FileHandle, const char *FileName, const MM_LoadDumpFileHeader_t *FileHeader)
{
    UT_GenStub_SetupReturnBuffer(MM_DumpMem16ToFile, int32);

    UT_GenStub_AddParam(MM_DumpMem16ToFile, osal_id_t, FileHandle);
    UT_GenStub_AddParam(MM_DumpMem16ToFile, const char *, FileName);
    UT_GenStub_AddParam(MM_DumpMem16ToFile, const MM_LoadDumpFileHeader_t *, FileHeader);

    UT_GenStub_Execute(MM_DumpMem16ToFile, Basic, NULL);

    return UT_GenStub_GetReturnValue(MM_DumpMem16ToFile, int32);
}

/*
 * ----------------------------------------------------
 * Generated stub function for MM_FillMem16()
 * ----------------------------------------------------
 */
int32 MM_FillMem16(cpuaddr DestAddress, const MM_FillMemCmd_t *CmdPtr)
{
    UT_GenStub_SetupReturnBuffer(MM_FillMem16, int32);

    UT_GenStub_AddParam(MM_FillMem16, cpuaddr, DestAddress);
    UT_GenStub_AddParam(MM_FillMem16, const MM_FillMemCmd_t *, CmdPtr);

    UT_GenStub_Execute(MM_FillMem16, Basic, NULL);

    return UT_GenStub_GetReturnValue(MM_FillMem16, int32);
}

/*
 * ----------------------------------------------------
 * Generated stub function for MM_LoadMem16FromFile()
 * ----------------------------------------------------
 */
int32 MM_LoadMem16FromFile(osal_id_t                      FileHandle,
                           const char                    *FileName,
                           const MM_LoadDumpFileHeader_t *FileHeader,
                           cpuaddr                        DestAddress)
{
    UT_GenStub_SetupReturnBuffer(MM_LoadMem16FromFile, int32);

    UT_GenStub_AddParam(MM_LoadMem16FromFile, osal_id_t, FileHandle);
    UT_GenStub_AddParam(MM_LoadMem16FromFile, const char *, FileName);
    UT_GenStub_AddParam(MM_LoadMem16FromFile, const MM_LoadDumpFileHeader_t *, FileHeader);
    UT_GenStub_AddParam(MM_LoadMem16FromFile, cpuaddr, DestAddress);

    UT_GenStub_Execute(MM_LoadMem16FromFile, Basic, NULL);

    return UT_GenStub_GetReturnValue(MM_LoadMem16FromFile, int32);
}
