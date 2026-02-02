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
 * Auto-Generated stub implementations for functions defined in mm_utils header
 */

#include "mm_utils.h"
#include "utgenstub.h"

/*
 * ----------------------------------------------------
 * Generated stub function for MM_ComputeCRCFromFile()
 * ----------------------------------------------------
 */
int32 MM_ComputeCRCFromFile(osal_id_t FileHandle, uint32 *CrcPtr, uint32 TypeCRC)
{
    UT_GenStub_SetupReturnBuffer(MM_ComputeCRCFromFile, int32);

    UT_GenStub_AddParam(MM_ComputeCRCFromFile, osal_id_t, FileHandle);
    UT_GenStub_AddParam(MM_ComputeCRCFromFile, uint32 *, CrcPtr);
    UT_GenStub_AddParam(MM_ComputeCRCFromFile, uint32, TypeCRC);

    UT_GenStub_Execute(MM_ComputeCRCFromFile, Basic, NULL);

    return UT_GenStub_GetReturnValue(MM_ComputeCRCFromFile, int32);
}

/*
 * ----------------------------------------------------
 * Generated stub function for MM_ResetHk()
 * ----------------------------------------------------
 */
void MM_ResetHk(void)
{
    UT_GenStub_Execute(MM_ResetHk, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for MM_ResolveSymAddr()
 * ----------------------------------------------------
 */
int32 MM_ResolveSymAddr(MM_SymAddr_t *SymAddr, cpuaddr *ResolvedAddr)
{
    UT_GenStub_SetupReturnBuffer(MM_ResolveSymAddr, int32);

    UT_GenStub_AddParam(MM_ResolveSymAddr, MM_SymAddr_t *, SymAddr);
    UT_GenStub_AddParam(MM_ResolveSymAddr, cpuaddr *, ResolvedAddr);

    UT_GenStub_Execute(MM_ResolveSymAddr, Basic, NULL);

    return UT_GenStub_GetReturnValue(MM_ResolveSymAddr, int32);
}

/*
 * ----------------------------------------------------
 * Generated stub function for MM_SegmentBreak()
 * ----------------------------------------------------
 */
void MM_SegmentBreak(void)
{
    UT_GenStub_Execute(MM_SegmentBreak, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for MM_Verify16Aligned()
 * ----------------------------------------------------
 */
bool MM_Verify16Aligned(cpuaddr Address, size_t Size)
{
    UT_GenStub_SetupReturnBuffer(MM_Verify16Aligned, bool);

    UT_GenStub_AddParam(MM_Verify16Aligned, cpuaddr, Address);
    UT_GenStub_AddParam(MM_Verify16Aligned, size_t, Size);

    UT_GenStub_Execute(MM_Verify16Aligned, Basic, NULL);

    return UT_GenStub_GetReturnValue(MM_Verify16Aligned, bool);
}

/*
 * ----------------------------------------------------
 * Generated stub function for MM_Verify32Aligned()
 * ----------------------------------------------------
 */
bool MM_Verify32Aligned(cpuaddr Address, size_t Size)
{
    UT_GenStub_SetupReturnBuffer(MM_Verify32Aligned, bool);

    UT_GenStub_AddParam(MM_Verify32Aligned, cpuaddr, Address);
    UT_GenStub_AddParam(MM_Verify32Aligned, size_t, Size);

    UT_GenStub_Execute(MM_Verify32Aligned, Basic, NULL);

    return UT_GenStub_GetReturnValue(MM_Verify32Aligned, bool);
}

/*
 * ----------------------------------------------------
 * Generated stub function for MM_VerifyLoadDumpParams()
 * ----------------------------------------------------
 */
int32 MM_VerifyLoadDumpParams(cpuaddr Address, MM_MemType_Enum_t MemType, size_t SizeInBytes, uint8 VerifyType)
{
    UT_GenStub_SetupReturnBuffer(MM_VerifyLoadDumpParams, int32);

    UT_GenStub_AddParam(MM_VerifyLoadDumpParams, cpuaddr, Address);
    UT_GenStub_AddParam(MM_VerifyLoadDumpParams, MM_MemType_Enum_t, MemType);
    UT_GenStub_AddParam(MM_VerifyLoadDumpParams, size_t, SizeInBytes);
    UT_GenStub_AddParam(MM_VerifyLoadDumpParams, uint8, VerifyType);

    UT_GenStub_Execute(MM_VerifyLoadDumpParams, Basic, NULL);

    return UT_GenStub_GetReturnValue(MM_VerifyLoadDumpParams, int32);
}

/*
 * ----------------------------------------------------
 * Generated stub function for MM_VerifyPeekPokeParams()
 * ----------------------------------------------------
 */
int32 MM_VerifyPeekPokeParams(cpuaddr Address, MM_MemType_Enum_t MemType, size_t SizeInBits)
{
    UT_GenStub_SetupReturnBuffer(MM_VerifyPeekPokeParams, int32);

    UT_GenStub_AddParam(MM_VerifyPeekPokeParams, cpuaddr, Address);
    UT_GenStub_AddParam(MM_VerifyPeekPokeParams, MM_MemType_Enum_t, MemType);
    UT_GenStub_AddParam(MM_VerifyPeekPokeParams, size_t, SizeInBits);

    UT_GenStub_Execute(MM_VerifyPeekPokeParams, Basic, NULL);

    return UT_GenStub_GetReturnValue(MM_VerifyPeekPokeParams, int32);
}
