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
 *   Unit test stubs for mm_app.c
 */

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
