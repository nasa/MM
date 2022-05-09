/************************************************************************
** File: mm_test_utils.h
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
**   Test utility functions for MM unit tests
**
*************************************************************************/

#ifndef _MM_TEST_UTILS_H_
#define _MM_TEST_UTILS_H_

/************************************************************************
** Includes
*************************************************************************/
#include "mm_app.h"
#include "utstubs.h"

extern MM_AppData_t MM_AppData;

/*
 * Global context structures
 */
typedef struct
{
    uint16      EventID;
    uint16      EventType;
    const char *Spec;
} __attribute__((packed)) CFE_EVS_SendEvent_context_t;

typedef struct
{
    const char *Spec;
} __attribute__((packed)) CFE_ES_WriteToSysLog_context_t;

int32 UT_Utils_stub_reporter_hook(void *UserObj, int32 StubRetcode, uint32 CallCount, const UT_StubContext_t *Context);

/*
 * Function Definitions
 */

void MM_Test_Setup(void);
void MM_Test_TearDown(void);

#endif

/************************/
/*  End of File Comment */
/************************/
