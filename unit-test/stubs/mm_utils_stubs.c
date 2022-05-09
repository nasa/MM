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
 *   Unit test stubs for mm_utils.c
 */

/************************************************************************
** Includes
*************************************************************************/
#include "mm_app.h"
#include "mm_utils.h"
#include "mm_perfids.h"
#include "mm_msgids.h"
#include "mm_events.h"

/************************************************************************
** UT Includes
*************************************************************************/
#include "uttest.h"
#include "utassert.h"
#include "utstubs.h"

void MM_ResetHk(void)
{
    UT_DEFAULT_IMPL(MM_ResetHk);
}

void MM_SegmentBreak(void)
{
    UT_DEFAULT_IMPL(MM_SegmentBreak);
}

bool MM_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength)
{
    UT_Stub_RegisterContext(UT_KEY(MM_VerifyCmdLength), MsgPtr);
    UT_Stub_RegisterContextGenericArg(UT_KEY(MM_VerifyCmdLength), ExpectedLength);
    return UT_DEFAULT_IMPL(MM_VerifyCmdLength);
}

bool MM_VerifyPeekPokeParams(cpuaddr Address, uint8 MemType, uint8 SizeInBits)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(MM_VerifyPeekPokeParams), Address);
    UT_Stub_RegisterContextGenericArg(UT_KEY(MM_VerifyPeekPokeParams), MemType);
    UT_Stub_RegisterContextGenericArg(UT_KEY(MM_VerifyPeekPokeParams), SizeInBits);
    return UT_DEFAULT_IMPL(MM_VerifyPeekPokeParams);
}

bool MM_VerifyLoadDumpParams(cpuaddr Address, uint8 MemType, uint32 SizeInBytes, uint8 VerifyType)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(MM_VerifyLoadDumpParams), Address);
    UT_Stub_RegisterContextGenericArg(UT_KEY(MM_VerifyLoadDumpParams), MemType);
    UT_Stub_RegisterContextGenericArg(UT_KEY(MM_VerifyLoadDumpParams), SizeInBytes);
    UT_Stub_RegisterContextGenericArg(UT_KEY(MM_VerifyLoadDumpParams), VerifyType);
    return UT_DEFAULT_IMPL(MM_VerifyLoadDumpParams);
}

bool MM_Verify32Aligned(cpuaddr Address, uint32 Size)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(MM_Verify32Aligned), Address);
    UT_Stub_RegisterContextGenericArg(UT_KEY(MM_Verify32Aligned), Size);
    return UT_DEFAULT_IMPL(MM_Verify32Aligned) != 0;
}

bool MM_Verify16Aligned(cpuaddr Address, uint32 Size)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(MM_Verify16Aligned), Address);
    UT_Stub_RegisterContextGenericArg(UT_KEY(MM_Verify16Aligned), Size);
    return UT_DEFAULT_IMPL(MM_Verify16Aligned) != 0;
}

bool MM_ResolveSymAddr(MM_SymAddr_t *SymAddr, cpuaddr *ResolvedAddr)
{
    UT_Stub_RegisterContext(UT_KEY(MM_ResolveSymAddr), SymAddr);
    UT_Stub_RegisterContext(UT_KEY(MM_ResolveSymAddr), ResolvedAddr);
    return UT_DEFAULT_IMPL(MM_ResolveSymAddr) != 0;
}

int32 MM_ComputeCRCFromFile(osal_id_t FileHandle, uint32 *CrcPtr, uint32 TypeCRC)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(MM_ComputeCRCFromFile), FileHandle);
    UT_Stub_RegisterContext(UT_KEY(MM_ComputeCRCFromFile), CrcPtr);
    UT_Stub_RegisterContextGenericArg(UT_KEY(MM_ComputeCRCFromFile), TypeCRC);
    return UT_DEFAULT_IMPL(MM_ComputeCRCFromFile);
}
