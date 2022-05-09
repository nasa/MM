/************************************************************************
** File: mm_test_utils.c 
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

/************************************************************************
** Includes
*************************************************************************/
#include "mm_test_utils.h"
#include "mm_app.h"

/************************************************************************
** UT Includes
*************************************************************************/
#include "uttest.h"
#include "utassert.h"
#include "utstubs.h"

int32 UT_Utils_stub_reporter_hook(void *UserObj, int32 StubRetcode, uint32 CallCount, const UT_StubContext_t *Context)
{
    uint8 i            = 0;    /* i is index */
    uint8 size_used    = 0;    /* determines size of argument to be saved */
    void *val_location = NULL; /* pointer to arg value to be saved */
    void *obj_ptr;             /* tracker indicates where to push data into UserObj */

    /* Determine where in the UserObj we should be located dependent upon CallCount */
    if (CallCount == 0)
    {
        obj_ptr = UserObj;
    }
    else
    {
        uint8 context_size = 0;

        for (i = 0; i < Context->ArgCount; ++i)
        {
            /* A UT_STUBCONTEXT_ARG_TYPE_DIRECT type indicates the arg itself is the ptr argument, add a (void*) size */
            if (Context->Meta[i].Type == UT_STUBCONTEXT_ARG_TYPE_DIRECT)
            {
                context_size += sizeof(void *);
            }
            else /* UT_STUBCONTEXT_ARG_TYPE_INDIRECT indicates the arg is pointing to the value to be saved, add its
                    size */
            {
                context_size += Context->Meta[i].Size;
            }
        }

        /* obj_ptr moves a full context_size for every call (initial value is 0) -- user object for calls > 1 must be an
         * array of contexts */
        obj_ptr = (uint8 *)UserObj + (context_size * CallCount);
    }

    for (i = 0; i < Context->ArgCount; ++i)
    {
        /* UT_STUBCONTEXT_ARG_TYPE_DIRECT indicates the arg is the ptr that is to be saved */
        if (Context->Meta[i].Type == UT_STUBCONTEXT_ARG_TYPE_DIRECT)
        {
            val_location = (void *)&Context->ArgPtr[i];
            size_used    = sizeof(void *);
        }
        else /* UT_STUBCONTEXT_ARG_TYPE_INDIRECT indicates the arg is pointing to the value to be saved */
        {
            val_location = (void *)Context->ArgPtr[i];
            size_used    = Context->Meta[i].Size;
        }
        /* put the argument value into the user object */
        memcpy(obj_ptr, val_location, size_used);
        /* move to end of this size item in the user object */
        obj_ptr = (uint8 *)obj_ptr + size_used;
    }

    return StubRetcode;
}

/*
 * Function Definitions
 */

void MM_Test_Setup(void)
{
    /* initialize test environment to default state for every test */
    UT_ResetState(0);

    CFE_PSP_MemSet(&MM_AppData, 0, sizeof(MM_AppData_t));

} /* end HS_Test_Setup */

void MM_Test_TearDown(void)
{
    /* cleanup test environment */
} /* end MM_Test_TearDown */

/*
 * Placeholder stubs until PSP UT implementation is available
 */

int32 CFE_PSP_EepromWriteEnable(uint32 Bank)
{
    if (Bank == 0)
        return (CFE_PSP_SUCCESS);
    else
        return (CFE_PSP_ERROR_NOT_IMPLEMENTED);
}

int32 CFE_PSP_EepromWriteDisable(uint32 Bank)
{
    if (Bank == 0)
        return (CFE_PSP_SUCCESS);
    else
        return (CFE_PSP_ERROR_NOT_IMPLEMENTED);
}

int32 CFE_PSP_EepromWrite32(cpuaddr MemoryAddress, uint32 uint32Value)
{
    if (!MemoryAddress)
        return (-1);
    else
        return (CFE_PSP_SUCCESS);
}

int32 CFE_PSP_EepromWrite16(cpuaddr MemoryAddress, uint16 uint16Value)
{
    if (!MemoryAddress)
        return (-1);
    else
        return (CFE_PSP_SUCCESS);
}

int32 CFE_PSP_EepromWrite8(cpuaddr MemoryAddress, uint8 ByteValue)
{
    if (!MemoryAddress)
        return (-1);
    else
        return (CFE_PSP_SUCCESS);
}

int32 CFE_PSP_MemWrite8(cpuaddr MemoryAddress, uint8 ByteValue)
{
    if (!MemoryAddress)
        return (-1);
    else
        return (CFE_PSP_SUCCESS);
}

int32 CFE_PSP_MemRead16(cpuaddr MemoryAddress, uint16 *uint16Value)
{
    if (!MemoryAddress)
        return (-1);
    else
        return (CFE_PSP_SUCCESS);
}

int32 CFE_PSP_MemWrite16(cpuaddr MemoryAddress, uint16 uint16Value)
{
    if (!MemoryAddress)
        return (-1);
    else
        return (CFE_PSP_SUCCESS);
}

int32 CFE_PSP_MemRead32(cpuaddr MemoryAddress, uint32 *uint32Value)
{
    if (!MemoryAddress)
        return (-1);
    else
        return (CFE_PSP_SUCCESS);
}

int32 CFE_PSP_MemWrite32(cpuaddr MemoryAddress, uint32 uint32Value)
{
    if (!MemoryAddress)
        return (-1);
    else
        return (CFE_PSP_SUCCESS);
}

/************************/
/*  End of File Comment */
/************************/
