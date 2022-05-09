/************************************************************************
** File: mm_dump_stubs.c 
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
**   Unit test stubs for mm_dump.c
**
*************************************************************************/

/************************************************************************
** Includes
*************************************************************************/
#include "mm_app.h"
#include "mm_dump.h"
#include "mm_events.h"
#include "mm_mem32.h"
#include "mm_mem16.h"
#include "mm_mem8.h"
#include "mm_utils.h"
#include "mm_mission_cfg.h"
#include "cfs_utils.h"

/************************************************************************
** UT Includes
*************************************************************************/
#include "uttest.h"
#include "utassert.h"
#include "utstubs.h"

bool MM_PeekMem(const MM_PeekCmd_t *CmdPtr, cpuaddr SrcAddress)
{
    UT_Stub_RegisterContext(UT_KEY(MM_PeekMem), CmdPtr);
    UT_Stub_RegisterContextGenericArg(UT_KEY(MM_PeekMem), SrcAddress);
    return UT_DEFAULT_IMPL(MM_PeekMem);
}

bool MM_DumpMemToFile(uint32 FileHandle, const char *FileName, const MM_LoadDumpFileHeader_t *FileHeader)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(MM_DumpMemToFile), FileHandle);
    UT_Stub_RegisterContext(UT_KEY(MM_DumpMemToFile), FileName);
    UT_Stub_RegisterContext(UT_KEY(MM_DumpMemToFile), FileHeader);
    return UT_DEFAULT_IMPL(MM_DumpMemToFile);
}

bool MM_WriteFileHeaders(const char *FileName, int32 FileHandle, CFE_FS_Header_t *CFEHeader,
                         const MM_LoadDumpFileHeader_t *MMHeader)
{
    UT_Stub_RegisterContext(UT_KEY(MM_WriteFileHeaders), FileName);
    UT_Stub_RegisterContextGenericArg(UT_KEY(MM_WriteFileHeaders), FileHandle);
    UT_Stub_RegisterContext(UT_KEY(MM_WriteFileHeaders), CFEHeader);
    UT_Stub_RegisterContext(UT_KEY(MM_WriteFileHeaders), MMHeader);
    return UT_DEFAULT_IMPL(MM_WriteFileHeaders);
}

bool MM_FillDumpInEventBuffer(cpuaddr SrcAddress, const MM_DumpInEventCmd_t *CmdPtr, void *DumpBuffer)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(MM_FillDumpInEventBuffer), SrcAddress);
    UT_Stub_RegisterContext(UT_KEY(MM_FillDumpInEventBuffer), CmdPtr);
    UT_Stub_RegisterContext(UT_KEY(MM_FillDumpInEventBuffer), DumpBuffer);
    return UT_DEFAULT_IMPL(MM_FillDumpInEventBuffer);
}

bool MM_PeekCmd(const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContext(UT_KEY(MM_PeekCmd), BufPtr);
    return UT_DEFAULT_IMPL(MM_PeekCmd);
    CFE_EVS_SendEvent(MM_SYMNAME_ERR_EID, CFE_EVS_EventType_ERROR, "Symbolic address can't be resolved: Name = 'name'");
}

bool MM_DumpMemToFileCmd(const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContext(UT_KEY(MM_DumpMemToFileCmd), BufPtr);
    return UT_DEFAULT_IMPL(MM_DumpMemToFileCmd);
    CFE_EVS_SendEvent(MM_CMD_FNAME_ERR_EID, CFE_EVS_EventType_ERROR,
                      "Command specified filename invalid: Name = 'file'");
}

bool MM_DumpInEventCmd(const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContext(UT_KEY(MM_DumpInEventCmd), BufPtr);
    return UT_DEFAULT_IMPL(MM_DumpInEventCmd);
    CFE_EVS_SendEvent(MM_SYMNAME_ERR_EID, CFE_EVS_EventType_ERROR, "Symbolic address can't be resolved: Name = 'name'");
}
