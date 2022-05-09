/************************************************************************
** File: mm_load_stubs.c 
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
**   Unit test stubs for mm_load.c
**
*************************************************************************/

/************************************************************************
** Includes
*************************************************************************/
#include "mm_app.h"
#include "mm_load.h"
#include "mm_perfids.h"
#include "mm_events.h"
#include "mm_utils.h"
#include "mm_mem32.h"
#include "mm_mem16.h"
#include "mm_mem8.h"
#include "mm_mission_cfg.h"
#include "cfs_utils.h"

/************************************************************************
** UT Includes
*************************************************************************/
#include "uttest.h"
#include "utassert.h"
#include "utstubs.h"

bool MM_PokeMem(const MM_PokeCmd_t *CmdPtr, cpuaddr DestAddress)
{
    UT_Stub_RegisterContext(UT_KEY(MM_PokeMem), CmdPtr);
    UT_Stub_RegisterContextGenericArg(UT_KEY(MM_PokeMem), DestAddress);
    return UT_DEFAULT_IMPL(MM_PokeMem);
}

bool MM_PokeEeprom(const MM_PokeCmd_t *CmdPtr, cpuaddr DestAddress)
{
    UT_Stub_RegisterContext(UT_KEY(MM_PokeEeprom), CmdPtr);
    UT_Stub_RegisterContextGenericArg(UT_KEY(MM_PokeEeprom), DestAddress);
    return UT_DEFAULT_IMPL(MM_PokeEeprom);
}

bool MM_LoadMemWID(const MM_LoadMemWIDCmd_t *CmdPtr, cpuaddr DestAddress)
{
    UT_Stub_RegisterContext(UT_KEY(MM_LoadMemWID), CmdPtr);
    UT_Stub_RegisterContextGenericArg(UT_KEY(MM_LoadMemWID), DestAddress);
    return UT_DEFAULT_IMPL(MM_LoadMemWID);
}

bool MM_LoadMemFromFile(uint32 FileHandle, const char *FileName, const MM_LoadDumpFileHeader_t *FileHeader,
                        cpuaddr DestAddress)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(MM_LoadMemFromFile), FileHandle);
    UT_Stub_RegisterContext(UT_KEY(MM_LoadMemFromFile), FileName);
    UT_Stub_RegisterContext(UT_KEY(MM_LoadMemFromFile), FileHeader);
    UT_Stub_RegisterContextGenericArg(UT_KEY(MM_LoadMemFromFile), DestAddress);
    return UT_DEFAULT_IMPL(MM_LoadMemFromFile);
}

bool MM_VerifyLoadFileSize(const char *FileName, const MM_LoadDumpFileHeader_t *FileHeader)
{
    UT_Stub_RegisterContext(UT_KEY(MM_VerifyLoadFileSize), FileName);
    UT_Stub_RegisterContext(UT_KEY(MM_VerifyLoadFileSize), FileHeader);
    return UT_DEFAULT_IMPL(MM_VerifyLoadFileSize);
}

bool MM_ReadFileHeaders(const char *FileName, int32 FileHandle, CFE_FS_Header_t *CFEHeader,
                        MM_LoadDumpFileHeader_t *MMHeader)
{
    UT_Stub_RegisterContext(UT_KEY(MM_ReadFileHeaders), FileName);
    UT_Stub_RegisterContextGenericArg(UT_KEY(MM_ReadFileHeaders), FileHandle);
    UT_Stub_RegisterContext(UT_KEY(MM_ReadFileHeaders), CFEHeader);
    UT_Stub_RegisterContext(UT_KEY(MM_ReadFileHeaders), MMHeader);
    return UT_DEFAULT_IMPL(MM_ReadFileHeaders);
}

bool MM_FillMem(cpuaddr DestAddr, const MM_FillMemCmd_t *CmdPtr)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(MM_FillMem), DestAddr);
    UT_Stub_RegisterContext(UT_KEY(MM_FillMem), CmdPtr);
    return UT_DEFAULT_IMPL(MM_FillMem);
}

bool MM_PokeCmd(const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContext(UT_KEY(MM_PokeCmd), BufPtr);
    return UT_DEFAULT_IMPL(MM_PokeCmd);
    CFE_EVS_SendEvent(MM_SYMNAME_ERR_EID, CFE_EVS_EventType_ERROR, "Symbolic address can't be resolved: Name = 'name'");
}

bool MM_LoadMemWIDCmd(const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContext(UT_KEY(MM_LoadMemWIDCmd), BufPtr);
    return UT_DEFAULT_IMPL(MM_LoadMemWIDCmd);
    CFE_EVS_SendEvent(MM_SYMNAME_ERR_EID, CFE_EVS_EventType_ERROR, "Symbolic address can't be resolved: Name = 'name'");
}

bool MM_LoadMemFromFileCmd(const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContext(UT_KEY(MM_LoadMemFromFileCmd), BufPtr);
    return UT_DEFAULT_IMPL(MM_LoadMemFromFileCmd);
    CFE_EVS_SendEvent(MM_CMD_FNAME_ERR_EID, CFE_EVS_EventType_ERROR,
                      "Command specified filename invalid: Name = 'file'");
}

bool MM_FillMemCmd(const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContext(UT_KEY(MM_FillMemCmd), BufPtr);
    return UT_DEFAULT_IMPL(MM_FillMemCmd);
    CFE_EVS_SendEvent(MM_SYMNAME_ERR_EID, CFE_EVS_EventType_ERROR, "Symbolic address can't be resolved: Name = 'name'");
}
