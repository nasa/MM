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
 *   The CFS Memory Manager (MM) Application provides onboard hardware
 *   and software maintenance services by processing commands for memory
 *   operations and read and write accesses to memory mapped hardware.
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
    int32            Status = CFE_SUCCESS;
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
        Status = CFE_SB_ReceiveBuffer(&BufPtr, MM_AppData.CmdPipe, MM_SB_TIMEOUT);
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
            MM_AppPipe(BufPtr);
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
            CFE_EVS_SendEvent(MM_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "SB Pipe Read Error, App will exit. RC = 0x%08X", (unsigned int)Status);

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

} /* end MM_AppMain */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* MM initialization                                               */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 MM_AppInit(void)
{
    int32 Status = CFE_SUCCESS;

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
    MM_AppData.HkPacket.CmdCounter = 0;
    MM_AppData.HkPacket.ErrCounter = 0;

    /*
    ** Register for event services
    */
    Status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);

    if (Status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("MM App: Error Registering For Event Services, RC = 0x%08X\n", (unsigned int)Status);
        return (Status);
    }

    /*
    ** Initialize the local housekeeping telemetry packet (clear user data area)
    */
    CFE_MSG_Init(&MM_AppData.HkPacket.TlmHeader.Msg, CFE_SB_ValueToMsgId(MM_HK_TLM_MID), sizeof(MM_HkPacket_t));

    /*
    ** Create Software Bus message pipe
    */
    Status = CFE_SB_CreatePipe(&MM_AppData.CmdPipe, MM_CMD_PIPE_DEPTH, "MM_CMD_PIPE");
    if (Status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(MM_CR_PIPE_ERR_EID, CFE_EVS_EventType_ERROR, "Error Creating SB Pipe, RC = 0x%08X", Status);
        return (Status);
    }

    /*
    ** Subscribe to Housekeeping request commands
    */
    Status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(MM_SEND_HK_MID), MM_AppData.CmdPipe);
    if (Status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(MM_HK_SUB_ERR_EID, CFE_EVS_EventType_ERROR, "Error Subscribing to HK Request, RC = 0x%08X",
                          Status);
        return (Status);
    }

    /*
    ** Subscribe to MM ground command packets
    */
    Status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(MM_CMD_MID), MM_AppData.CmdPipe);
    if (Status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(MM_CMD_SUB_ERR_EID, CFE_EVS_EventType_ERROR, "Error Subscribing to MM Command, RC = 0x%08X",
                          Status);
        return (Status);
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
    CFE_EVS_SendEvent(MM_INIT_INF_EID, CFE_EVS_EventType_INFORMATION, "MM Initialized. Version %d.%d.%d.%d",
                      MM_MAJOR_VERSION, MM_MINOR_VERSION, MM_REVISION, MM_MISSION_REV);

    return (CFE_SUCCESS);

} /* end MM_AppInit */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Process a command pipe message                                  */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void MM_AppPipe(const CFE_SB_Buffer_t *BufPtr)
{
    CFE_SB_MsgId_t    MessageID   = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t CommandCode = 0;

    CFE_MSG_GetMsgId(&BufPtr->Msg, &MessageID);

    bool CmdResult = false;

    switch (CFE_SB_MsgIdToValue(MessageID))
    {
        /*
        ** Housekeeping telemetry request
        */
        case MM_SEND_HK_MID:
            MM_HousekeepingCmd(BufPtr);
            break;

        /*
        ** MM ground commands
        */
        case MM_CMD_MID:
            MM_ResetHk(); /* Clear all "Last Action" data */

            CFE_MSG_GetFcnCode(&BufPtr->Msg, &CommandCode);
            switch (CommandCode)
            {
                case MM_NOOP_CC:
                    CmdResult = MM_NoopCmd(BufPtr);
                    break;

                case MM_RESET_CC:
                    CmdResult = MM_ResetCmd(BufPtr);
                    break;

                case MM_PEEK_CC:
                    CmdResult = MM_PeekCmd(BufPtr);
                    break;

                case MM_POKE_CC:
                    CmdResult = MM_PokeCmd(BufPtr);
                    break;

                case MM_LOAD_MEM_WID_CC:
                    CmdResult = MM_LoadMemWIDCmd(BufPtr);
                    break;

                case MM_LOAD_MEM_FROM_FILE_CC:
                    CmdResult = MM_LoadMemFromFileCmd(BufPtr);
                    break;

                case MM_DUMP_MEM_TO_FILE_CC:
                    CmdResult = MM_DumpMemToFileCmd(BufPtr);
                    break;

                case MM_DUMP_IN_EVENT_CC:
                    CmdResult = MM_DumpInEventCmd(BufPtr);
                    break;

                case MM_FILL_MEM_CC:
                    CmdResult = MM_FillMemCmd(BufPtr);
                    break;

                case MM_LOOKUP_SYM_CC:
                    CmdResult = MM_LookupSymbolCmd(BufPtr);
                    break;

                case MM_SYMTBL_TO_FILE_CC:
                    CmdResult = MM_SymTblToFileCmd(BufPtr);
                    break;

                case MM_ENABLE_EEPROM_WRITE_CC:
                    CmdResult = MM_EepromWriteEnaCmd(BufPtr);
                    break;

                case MM_DISABLE_EEPROM_WRITE_CC:
                    CmdResult = MM_EepromWriteDisCmd(BufPtr);
                    break;

                default:
                    CmdResult = false;
                    CFE_EVS_SendEvent(MM_CC1_ERR_EID, CFE_EVS_EventType_ERROR,
                                      "Invalid ground command code: ID = 0x%08lX, CC = %d",
                                      (unsigned long)CFE_SB_MsgIdToValue(MessageID), CommandCode);
                    break;
            }

            if (CommandCode != MM_RESET_CC)
            {
                if (CmdResult == true)
                {
                    MM_AppData.HkPacket.CmdCounter++;
                }
                else
                {
                    MM_AppData.HkPacket.ErrCounter++;
                }
            }
            break;

        /*
        ** Unrecognized Message ID
        */
        default:
            MM_AppData.HkPacket.ErrCounter++;
            CFE_EVS_SendEvent(MM_MID_ERR_EID, CFE_EVS_EventType_ERROR, "Invalid command pipe message ID: 0x%08lX",
                              (unsigned long)CFE_SB_MsgIdToValue(MessageID));
            break;

    } /* end switch */

    return;

} /* End MM_AppPipe */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Housekeeping request                                            */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void MM_HousekeepingCmd(const CFE_SB_Buffer_t *BufPtr)
{
    size_t ExpectedLength = sizeof(MM_NoArgsCmd_t);

    /*
    ** Verify command packet length
    */
    if (MM_VerifyCmdLength(&BufPtr->Msg, ExpectedLength))
    {

        /*
        ** Send housekeeping telemetry packet
        */
        CFE_SB_TimeStampMsg(&MM_AppData.HkPacket.TlmHeader.Msg);
        CFE_SB_TransmitMsg(&MM_AppData.HkPacket.TlmHeader.Msg, true);

        /*
        ** This command does not affect the command execution counter
        */

    } /* end if */

    return;

} /* end MM_HousekeepingCmd */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Noop command                                                    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
bool MM_NoopCmd(const CFE_SB_Buffer_t *BufPtr)
{
    size_t ExpectedLength = sizeof(MM_NoArgsCmd_t);
    bool   Result         = false;
    /*
    ** Verify command packet length
    */
    if (MM_VerifyCmdLength(&BufPtr->Msg, ExpectedLength))
    {
        MM_AppData.HkPacket.LastAction = MM_NOOP;
        Result                         = true;

        CFE_EVS_SendEvent(MM_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "No-op command. Version %d.%d.%d.%d",
                          MM_MAJOR_VERSION, MM_MINOR_VERSION, MM_REVISION, MM_MISSION_REV);
    }

    return Result;

} /* end MM_NoopCmd */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Reset counters command                                          */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
bool MM_ResetCmd(const CFE_SB_Buffer_t *BufPtr)
{
    size_t ExpectedLength = sizeof(MM_NoArgsCmd_t);
    bool   Result         = false;
    /*
    ** Verify command packet length
    */
    if (MM_VerifyCmdLength(&BufPtr->Msg, ExpectedLength))
    {
        MM_AppData.HkPacket.LastAction = MM_RESET;
        MM_AppData.HkPacket.CmdCounter = 0;
        MM_AppData.HkPacket.ErrCounter = 0;

        CFE_EVS_SendEvent(MM_RESET_INF_EID, CFE_EVS_EventType_INFORMATION, "Reset counters command received");
        Result = true;
    }

    return Result;

} /* end MM_ResetCmd */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Lookup symbol name command                                      */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
bool MM_LookupSymbolCmd(const CFE_SB_Buffer_t *BufPtr)
{
    int32              OS_Status = OS_ERROR; /* Set to error instead of success since we explicitly test for success */
    cpuaddr            ResolvedAddr   = 0;
    MM_LookupSymCmd_t *CmdPtr         = NULL;
    size_t             ExpectedLength = sizeof(MM_LookupSymCmd_t);
    bool               Result         = false;

    /*
    ** Verify command packet length
    */
    if (MM_VerifyCmdLength(&BufPtr->Msg, ExpectedLength))
    {
        CmdPtr = ((MM_LookupSymCmd_t *)BufPtr);

        /*
        ** NUL terminate the very end of the symbol name string as a
        ** safety measure
        */
        CmdPtr->SymName[OS_MAX_SYM_LEN - 1] = '\0';

        /*
        ** Check if the symbol name string is a nul string
        */
        if (strlen(CmdPtr->SymName) == 0)
        {
            CFE_EVS_SendEvent(MM_SYMNAME_NUL_ERR_EID, CFE_EVS_EventType_ERROR,
                              "NUL (empty) string specified as symbol name");
        }
        else
        {
            /*
            ** If symbol name is not an empty string look it up using the OSAL API
            */
            OS_Status = OS_SymbolLookup(&ResolvedAddr, CmdPtr->SymName);
            if (OS_Status == OS_SUCCESS)
            {
                /* Update telemetry */
                MM_AppData.HkPacket.LastAction = MM_SYM_LOOKUP;
                MM_AppData.HkPacket.Address    = ResolvedAddr;

                CFE_EVS_SendEvent(MM_SYM_LOOKUP_INF_EID, CFE_EVS_EventType_INFORMATION,
                                  "Symbol Lookup Command: Name = '%s' Addr = %p", CmdPtr->SymName,
                                  (void *)ResolvedAddr);
                Result = true;
            }
            else
            {
                CFE_EVS_SendEvent(MM_SYMNAME_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "Symbolic address can't be resolved: Name = '%s'", CmdPtr->SymName);
            }

        } /* end strlen(CmdPtr->SymName) == 0 else */

    } /* end MM_VerifyCmdLength if */

    return Result;

} /* end MM_LookupSymbolCmd */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Dump symbol table to file command                               */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
bool MM_SymTblToFileCmd(const CFE_SB_Buffer_t *BufPtr)
{
    int32 OS_Status              = OS_ERROR; /* Set to error instead of success since we explicitly test for success */
    MM_SymTblToFileCmd_t *CmdPtr = NULL;
    size_t                ExpectedLength = sizeof(MM_SymTblToFileCmd_t);
    bool                  Result         = false;

    /*
    ** Verify command packet length
    */
    if (MM_VerifyCmdLength(&BufPtr->Msg, ExpectedLength))
    {
        CmdPtr = ((MM_SymTblToFileCmd_t *)BufPtr);

        /*
        ** NUL terminate the very end of the filename string as a
        ** safety measure
        */
        CmdPtr->FileName[OS_MAX_PATH_LEN - 1] = '\0';

        /*
        ** Check if the filename string is a nul string
        */
        if (strlen(CmdPtr->FileName) == 0)
        {
            CFE_EVS_SendEvent(MM_SYMFILENAME_NUL_ERR_EID, CFE_EVS_EventType_ERROR,
                              "NUL (empty) string specified as symbol dump file name");
        }
        else
        {
            OS_Status = OS_SymbolTableDump(CmdPtr->FileName, MM_MAX_DUMP_FILE_DATA_SYMTBL);
            if (OS_Status == OS_SUCCESS)
            {
                /* Update telemetry */
                MM_AppData.HkPacket.LastAction = MM_SYMTBL_SAVE;
                strncpy(MM_AppData.HkPacket.FileName, CmdPtr->FileName, OS_MAX_PATH_LEN);

                CFE_EVS_SendEvent(MM_SYMTBL_TO_FILE_INF_EID, CFE_EVS_EventType_INFORMATION,
                                  "Symbol Table Dump to File Started: Name = '%s'", CmdPtr->FileName);
                Result = true;
            }
            else
            {
                CFE_EVS_SendEvent(MM_SYMTBL_TO_FILE_FAIL_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "Error dumping symbol table, OS_Status= 0x%X, File='%s'", (unsigned int)OS_Status,
                                  CmdPtr->FileName);
            }

        } /* end strlen(CmdPtr->FileName) == 0 else */

    } /* end MM_VerifyCmdLength if */

    return Result;

} /* end MM_SymTblToFileCmd */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* EEPROM write-enable command                                     */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
bool MM_EepromWriteEnaCmd(const CFE_SB_Buffer_t *BufPtr)
{
    int32                   cFE_Status     = CFE_PSP_ERROR; /* Set to error since we explicitly test for success */
    MM_EepromWriteEnaCmd_t *CmdPtr         = NULL;
    size_t                  ExpectedLength = sizeof(MM_EepromWriteEnaCmd_t);
    bool                    Result         = false;

    /*
    ** Verify command packet length
    */
    if (MM_VerifyCmdLength(&BufPtr->Msg, ExpectedLength))
    {
        CmdPtr = ((MM_EepromWriteEnaCmd_t *)BufPtr);

        /*
        ** Call the cFE to write-enable the requested bank
        */
        cFE_Status = CFE_PSP_EepromWriteEnable(CmdPtr->Bank);
        if (cFE_Status == CFE_PSP_SUCCESS)
        {
            /* Update telemetry */
            MM_AppData.HkPacket.LastAction = MM_EEPROMWRITE_ENA;
            MM_AppData.HkPacket.MemType    = MM_EEPROM;

            CFE_EVS_SendEvent(MM_EEPROM_WRITE_ENA_INF_EID, CFE_EVS_EventType_INFORMATION,
                              "EEPROM bank %d write enabled, cFE_Status= 0x%X", (int)CmdPtr->Bank,
                              (unsigned int)cFE_Status);
            Result = true;
        }
        else
        {
            CFE_EVS_SendEvent(MM_EEPROM_WRITE_ENA_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Error requesting EEPROM bank %d write enable, cFE_Status= 0x%X", (int)CmdPtr->Bank,
                              (unsigned int)cFE_Status);
        }

    } /* end MM_VerifyCmdLength if */

    return Result;

} /* end MM_EepromWriteEnaCmd */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* EEPROM write-disable command                                    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
bool MM_EepromWriteDisCmd(const CFE_SB_Buffer_t *BufPtr)
{
    int32                   cFE_Status     = CFE_PSP_ERROR; /* Set to error since we explicitly test for success */
    MM_EepromWriteDisCmd_t *CmdPtr         = NULL;
    size_t                  ExpectedLength = sizeof(MM_EepromWriteDisCmd_t);
    bool                    Result         = false;

    /*
    ** Verify command packet length
    */
    if (MM_VerifyCmdLength(&BufPtr->Msg, ExpectedLength))
    {
        CmdPtr = ((MM_EepromWriteDisCmd_t *)BufPtr);

        /*
        ** Call the cFE to write-enable the requested bank
        */
        cFE_Status = CFE_PSP_EepromWriteDisable(CmdPtr->Bank);
        if (cFE_Status == CFE_PSP_SUCCESS)
        {
            /* Update telemetry */
            MM_AppData.HkPacket.LastAction = MM_EEPROMWRITE_DIS;
            MM_AppData.HkPacket.MemType    = MM_EEPROM;
            Result                         = true;
            CFE_EVS_SendEvent(MM_EEPROM_WRITE_DIS_INF_EID, CFE_EVS_EventType_INFORMATION,
                              "EEPROM bank %d write disabled, cFE_Status= 0x%X", (int)CmdPtr->Bank,
                              (unsigned int)cFE_Status);
        }
        else
        {
            CFE_EVS_SendEvent(MM_EEPROM_WRITE_DIS_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Error requesting EEPROM bank %d write disable, cFE_Status= 0x%X", (int)CmdPtr->Bank,
                              (unsigned int)cFE_Status);
        }

    } /* end MM_VerifyCmdLength if */

    return Result;

} /* end MM_EepromWriteDisCmd */

/************************/
/*  End of File Comment */
/************************/
