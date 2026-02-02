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
 *   Unit tests for mm_dispatch.c
 */

/* ======== */
/* Includes */
/* ======== */

#include "mm_cmds.h"
#include "mm_dispatch.h"
#include "mm_eventids.h"
#include "mm_fcncodes.h"
#include "mm_msgdefs.h"
#include "mm_msgids.h"
#include "mm_test_utils.h"

#include "cfe.h"
#include "utassert.h"
#include "utstubs.h"
#include "uttest.h"

#include "mm_eds_dispatcher.h"

/*
**********************************************************************************
**          TEST CASE FUNCTIONS
**********************************************************************************
*/

void Test_MM_TaskPipe(void)
{
    /*
     * Test Case For:
     * void MM_TaskPipe
     */
    CFE_SB_Buffer_t UtBuf;

    UT_SetDeferredRetcode(UT_KEY(CFE_EDSMSG_Dispatch), 1, CFE_SUCCESS);

    memset(&UtBuf, 0, sizeof(UtBuf));
    UtAssert_VOIDCALL(MM_TaskPipe(&UtBuf));
}

/*
 * Register the test cases to execute with the unit test tool
 */
void UtTest_Setup(void)
{
    UtTest_Add(Test_MM_TaskPipe, MM_Test_Setup, MM_Test_TearDown, "Test_MM_TaskPipe");
}
