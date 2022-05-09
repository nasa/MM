/************************************************************************
** File: mm_mem8_stubs.c 
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
**   Unit test stubs for mm_mem8.c
**
*************************************************************************/

/************************************************************************
** Includes
*************************************************************************/
#include "mm_mem8.h"
#include "mm_app.h"
#include "mm_events.h"
#include "mm_utils.h"

/************************************************************************
** UT Includes
*************************************************************************/
#include "uttest.h"
#include "utassert.h"
#include "utstubs.h"

bool MM_LoadMem8FromFile(uint32 FileHandle, const char *FileName, const MM_LoadDumpFileHeader_t *FileHeader,
                         cpuaddr DestAddress)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(MM_LoadMem8FromFile), FileHandle);
    UT_Stub_RegisterContext(UT_KEY(MM_LoadMem8FromFile), FileName);
    UT_Stub_RegisterContext(UT_KEY(MM_LoadMem8FromFile), FileHeader);
    UT_Stub_RegisterContextGenericArg(UT_KEY(MM_LoadMem8FromFile), DestAddress);
    return UT_DEFAULT_IMPL(MM_LoadMem8FromFile);
}

bool MM_DumpMem8ToFile(uint32 FileHandle, const char *FileName, const MM_LoadDumpFileHeader_t *FileHeader)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(MM_DumpMem8ToFile), FileHandle);
    UT_Stub_RegisterContext(UT_KEY(MM_DumpMem8ToFile), FileName);
    UT_Stub_RegisterContext(UT_KEY(MM_DumpMem8ToFile), FileHeader);
    return UT_DEFAULT_IMPL(MM_DumpMem8ToFile);
}

bool MM_FillMem8(cpuaddr DestAddress, const MM_FillMemCmd_t *CmdPtr)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(MM_FillMem8), DestAddress);
    UT_Stub_RegisterContext(UT_KEY(MM_FillMem8), CmdPtr);
    return UT_DEFAULT_IMPL(MM_FillMem8);
}
