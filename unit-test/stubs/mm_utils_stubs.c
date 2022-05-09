/************************************************************************
** File: mm_utils_stubs.c 
**
**   Copyright © 2007-2014 United States Government as represented by the
**   Administrator of the National Aeronautics and Space Administration.
**   All Other Rights Reserved.
**
**   This software was created at NASA's Goddard Space Flight Center.
**   This software is governed by the NASA Open Source Agreement and may be
**   used, distributed and modified only pursuant to the terms of that
**   agreement.
**
** Purpose:
**   Unit test stubs for mm_utils.c
**
*************************************************************************/

/************************************************************************
** Includes
*************************************************************************/
#include "mm_app.h"
#include "mm_utils.h"
#include "mm_perfids.h"
#include "mm_msgids.h"
#include "mm_events.h"
#include "cfs_utils.h"

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
