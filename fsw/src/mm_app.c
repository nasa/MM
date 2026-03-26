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
 *   The CFS Memory Manager (MM) Application provides onboard hardware
 *   and software maintenance services by processing commands for memory
 *   operations and read and write accesses to memory mapped hardware.
 */

/************************************************************************
** Includes
*************************************************************************/
#include "mm_app.h"
#include "mm_dispatch.h"
#include "mm_dump.h"
#include "mm_eventids.h"
#include "mm_load.h"
#include "mm_msgids.h"
#include "mm_perfids.h"
#include "mm_platform_cfg.h"
#include "mm_utils.h"
#include "mm_verify.h"
#include "mm_version.h"
#include <string.h>

/************************************************************************
** MM global data
*************************************************************************/
MM_AppData_t MM_AppData;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* MM application entry point and main process loop                */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void MM_AppMain(void)
{
    CFE_Status_t     Status = CFE_SUCCESS;
    CFE_SB_Buffer_t *BufPtr = NULL;

    /*
    ** Create the first Performance Log entry
    */
    CFE_ES_PerfLogEntry(MM_APPMAIN_PERF_ID);

    /*
    ** Perform application specific initialization
    */
    Status = MM_AppInit();
    if (Status != CFE_SUCCESS)
    {
        MM_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    /*
    ** Application main loop
    */
    while (CFE_ES_RunLoop(&MM_AppData.RunStatus) == true)
    {
        /*
        ** Performance Log exit stamp
        */
        CFE_ES_PerfLogExit(MM_APPMAIN_PERF_ID);

        /*
        ** Pend on the arrival of the next Software Bus message
        */
        Status = CFE_SB_ReceiveBuffer(&BufPtr, MM_AppData.CmdPipe, MM_INTERNAL_SB_TIMEOUT);
        /*
        ** Performance Log entry stamp
        */
        CFE_ES_PerfLogEntry(MM_APPMAIN_PERF_ID);

        /*
        ** Check the return status from the software bus
        */
        if ((Status == CFE_SUCCESS) && (BufPtr != NULL))
        {
            /* Process Software Bus message */
            MM_TaskPipe(BufPtr);
        }
        else if (Status == CFE_SB_TIME_OUT)
        {
            /* No action, but also no error */
        }
        else
        {
            /*
            ** Exit on pipe read error
            */
            CFE_EVS_SendEvent(MM_PIPE_ERR_EID,
                              CFE_EVS_EventType_ERROR,
                              "SB Pipe Read Error, App will exit. RC = 0x%08X",
                              (unsigned int)Status);

            MM_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    } /* end CFS_ES_RunLoop while */

    /*
    ** Performance Log exit stamp
    */
    CFE_ES_PerfLogExit(MM_APPMAIN_PERF_ID);

    /*
    ** Exit the application
    */
    CFE_ES_ExitApp(MM_AppData.RunStatus);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* MM initialization                                               */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CFE_Status_t MM_AppInit(void)
{
    CFE_Status_t Status = CFE_SUCCESS;

    /*
    ** MM doesn't use the critical data store and
    ** doesn't need to identify power on vs. processor resets.
    ** If this changes add it here as shown in the qq_app.c template
    */

    /*
    ** Setup the RunStatus variable
    */
    MM_AppData.RunStatus = CFE_ES_RunStatus_APP_RUN;

    /*
    ** Initialize application command execution counters
    */
    MM_AppData.HkTlm.Payload.CommandCounter      = 0;
    MM_AppData.HkTlm.Payload.CommandErrorCounter = 0;

    /*
    ** Register for event services
    */
    Status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);

    if (Status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("MM App: Error Registering For Event Services, RC = 0x%08X\n", (unsigned int)Status);
        return Status;
    }

    /*
    ** Initialize the local housekeeping telemetry packet (clear user data area)
    */
    CFE_MSG_Init(CFE_MSG_PTR(MM_AppData.HkTlm.TelemetryHeader), CFE_SB_ValueToMsgId(MM_HK_TLM_MID), sizeof(MM_HkTlm_t));

    /*
    ** Create Software Bus message pipe
    */
    Status = CFE_SB_CreatePipe(&MM_AppData.CmdPipe, MM_INTERNAL_CMD_PIPE_DEPTH, MM_INTERNAL_CMD_PIPE_NAME);
    if (Status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(MM_CR_PIPE_ERR_EID,
                          CFE_EVS_EventType_ERROR,
                          "Error Creating SB Pipe, RC = 0x%08X",
                          (unsigned int)Status);
        return Status;
    }

    /*
    ** Subscribe to Housekeeping request commands
    */
    Status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(MM_SEND_HK_MID), MM_AppData.CmdPipe);
    if (Status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(MM_HK_SUB_ERR_EID,
                          CFE_EVS_EventType_ERROR,
                          "Error Subscribing to HK Request, RC = 0x%08X",
                          (unsigned int)Status);
        return Status;
    }

    /*
    ** Subscribe to MM ground command packets
    */
    Status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(MM_CMD_MID), MM_AppData.CmdPipe);
    if (Status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(MM_CMD_SUB_ERR_EID,
                          CFE_EVS_EventType_ERROR,
                          "Error Subscribing to MM Command, RC = 0x%08X",
                          (unsigned int)Status);
        return Status;
    }

    /*
    ** MM doesn't use tables. If this changes add table registration
    ** and initialization here as shown in the qq_app.c template
    */

    /*
    ** MM doesn't use the critical data store. If this changes add CDS
    ** creation here as shown in the qq_app.c template
    */

    /*
    ** Initialize MM housekeeping information
    */
    MM_ResetHk();

    /*
    ** Application startup event message
    */
    CFE_EVS_SendEvent(MM_INIT_INF_EID,
                      CFE_EVS_EventType_INFORMATION,
                      "MM Initialized. Version %d.%d.%d.%d",
                      MM_MAJOR_VERSION,
                      MM_MINOR_VERSION,
                      MM_REVISION,
                      MM_INTERNAL_MISSION_REV);

    return CFE_SUCCESS;
}
