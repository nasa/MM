/************************************************************************
** File: mm_mem16_stubs.c 
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
**   Unit test stubs for mm_mem16.c
**
*************************************************************************/

/************************************************************************
** Includes
*************************************************************************/
#include "mm_mem16.h"
#include "mm_app.h"
#include "mm_events.h"
#include "mm_utils.h"

/************************************************************************
** UT Includes
*************************************************************************/
#include "uttest.h"
#include "utassert.h"
#include "utstubs.h"

bool MM_LoadMem16FromFile(uint32 FileHandle, const char *FileName, const MM_LoadDumpFileHeader_t *FileHeader,
                          cpuaddr DestAddress)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(MM_LoadMem16FromFile), FileHandle);
    UT_Stub_RegisterContext(UT_KEY(MM_LoadMem16FromFile), FileName);
    UT_Stub_RegisterContext(UT_KEY(MM_LoadMem16FromFile), FileHeader);
    UT_Stub_RegisterContextGenericArg(UT_KEY(MM_LoadMem16FromFile), DestAddress);
    return UT_DEFAULT_IMPL(MM_LoadMem16FromFile);
}

bool MM_DumpMem16ToFile(uint32 FileHandle, const char *FileName, const MM_LoadDumpFileHeader_t *FileHeader)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(MM_DumpMem16ToFile), FileHandle);
    UT_Stub_RegisterContext(UT_KEY(MM_DumpMem16ToFile), FileName);
    UT_Stub_RegisterContext(UT_KEY(MM_DumpMem16ToFile), FileHeader);
    return UT_DEFAULT_IMPL(MM_DumpMem16ToFile);
}

bool MM_FillMem16(cpuaddr DestAddress, const MM_FillMemCmd_t *CmdPtr)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(MM_FillMem16), DestAddress);
    UT_Stub_RegisterContext(UT_KEY(MM_FillMem16), CmdPtr);
    return UT_DEFAULT_IMPL(MM_FillMem16);
}
