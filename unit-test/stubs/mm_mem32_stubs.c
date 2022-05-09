/************************************************************************
** File: mm_mem32_stubs.c 
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
**   Unit test stubs for mm_mem32.c
**
*************************************************************************/

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

bool MM_LoadMem32FromFile(uint32 FileHandle, const char *FileName, const MM_LoadDumpFileHeader_t *FileHeader,
                          cpuaddr DestAddress)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(MM_LoadMem32FromFile), FileHandle);
    UT_Stub_RegisterContext(UT_KEY(MM_LoadMem32FromFile), FileName);
    UT_Stub_RegisterContext(UT_KEY(MM_LoadMem32FromFile), FileHeader);
    UT_Stub_RegisterContextGenericArg(UT_KEY(MM_LoadMem32FromFile), DestAddress);
    return UT_DEFAULT_IMPL(MM_LoadMem32FromFile);
}

bool MM_DumpMem32ToFile(uint32 FileHandle, const char *FileName, const MM_LoadDumpFileHeader_t *FileHeader)
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
