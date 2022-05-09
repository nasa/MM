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
 *   Unit test stubs for mm_mem32.c
 */

/************************************************************************
** Includes
*************************************************************************/
#include "mm_mem32.h"
#include "mm_app.h"
#include "mm_events.h"
#include "mm_utils.h"

/************************************************************************
** UT Includes
*************************************************************************/
#include "uttest.h"
#include "utassert.h"
#include "utstubs.h"

bool MM_LoadMem32FromFile(osal_id_t FileHandle, const char *FileName, const MM_LoadDumpFileHeader_t *FileHeader,
                          cpuaddr DestAddress)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(MM_LoadMem32FromFile), FileHandle);
    UT_Stub_RegisterContext(UT_KEY(MM_LoadMem32FromFile), FileName);
    UT_Stub_RegisterContext(UT_KEY(MM_LoadMem32FromFile), FileHeader);
    UT_Stub_RegisterContextGenericArg(UT_KEY(MM_LoadMem32FromFile), DestAddress);
    return UT_DEFAULT_IMPL(MM_LoadMem32FromFile);
}

bool MM_DumpMem32ToFile(osal_id_t FileHandle, const char *FileName, const MM_LoadDumpFileHeader_t *FileHeader)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(MM_DumpMem32ToFile), FileHandle);
    UT_Stub_RegisterContext(UT_KEY(MM_DumpMem32ToFile), FileName);
    UT_Stub_RegisterContext(UT_KEY(MM_DumpMem32ToFile), FileHeader);
    return UT_DEFAULT_IMPL(MM_DumpMem32ToFile);
}

bool MM_FillMem32(cpuaddr DestAddress, const MM_FillMemCmd_t *CmdPtr)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(MM_FillMem32), DestAddress);
    UT_Stub_RegisterContext(UT_KEY(MM_FillMem32), CmdPtr);
    return UT_DEFAULT_IMPL(MM_FillMem32);
}
