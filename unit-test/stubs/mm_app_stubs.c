/************************************************************************
** File: mm_app_stubs.c 
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
**   Unit test stubs for mm_app.c
**
*************************************************************************/

/************************************************************************
** Includes
*************************************************************************/
#include "mm_app.h"
#include "mm_perfids.h"
#include "mm_msgids.h"
#include "mm_load.h"
#include "mm_dump.h"
#include "mm_utils.h"
#include "mm_events.h"
#include "mm_verify.h"
#include "mm_version.h"
#include "mm_platform_cfg.h"

/************************************************************************
** UT Includes
*************************************************************************/
#include "uttest.h"
#include "utassert.h"
#include "utstubs.h"

MM_AppData_t MM_AppData;

void MM_AppMain(void)
{
    UT_DEFAULT_IMPL(MM_AppMain);
}

int32 MM_AppInit(void)
{
    return UT_DEFAULT_IMPL(MM_AppInit);
}

void MM_AppPipe(const CFE_SB_Buffer_t *msg)
{
    UT_Stub_RegisterContext(UT_KEY(MM_AppPipe), msg);
    UT_DEFAULT_IMPL(MM_AppPipe);
}

void MM_HousekeepingCmd(const CFE_SB_Buffer_t *msg)
{
    UT_Stub_RegisterContext(UT_KEY(MM_HousekeepingCmd), msg);
    UT_DEFAULT_IMPL(MM_HousekeepingCmd);
}

bool MM_NoopCmd(const CFE_SB_Buffer_t *msg)
{
    UT_Stub_RegisterContext(UT_KEY(MM_NoopCmd), msg);
    return UT_DEFAULT_IMPL(MM_NoopCmd);
}

bool MM_ResetCmd(const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContext(UT_KEY(MM_ResetCmd), BufPtr);
    return UT_DEFAULT_IMPL(MM_ResetCmd);
}

bool MM_LookupSymbolCmd(const CFE_SB_Buffer_t *msg)
{
    UT_Stub_RegisterContext(UT_KEY(MM_LookupSymbolCmd), msg);
    return UT_DEFAULT_IMPL(MM_LookupSymbolCmd);
}

bool MM_SymTblToFileCmd(const CFE_SB_Buffer_t *msg)
{
    UT_Stub_RegisterContext(UT_KEY(MM_SymTblToFileCmd), msg);
    return UT_DEFAULT_IMPL(MM_SymTblToFileCmd);
}

bool MM_EepromWriteEnaCmd(const CFE_SB_Buffer_t *msg)
{
    UT_Stub_RegisterContext(UT_KEY(MM_EepromWriteEnaCmd), msg);
    return UT_DEFAULT_IMPL(MM_EepromWriteEnaCmd);
}

bool MM_EepromWriteDisCmd(const CFE_SB_Buffer_t *msg)
{
    UT_Stub_RegisterContext(UT_KEY(MM_EepromWriteDisCmd), msg);
    return UT_DEFAULT_IMPL(MM_EepromWriteDisCmd);
}
