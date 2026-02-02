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
 * \file
 *   This file contains the source code for the Memory Manager
 */

/*
** Include Files:
*/
#include "mm_dispatch.h"
#include "mm_app.h"
#include "mm_cmds.h"
#include "mm_eventids.h"
#include "mm_fcncodes.h"
#include "mm_msg.h"
#include "mm_msgids.h"
#include "mm_utils.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Verify command packet length                                               */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
bool MM_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength)
{
    bool              result       = true;
    size_t            ActualLength = 0;
    CFE_SB_MsgId_t    MsgId        = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t FcnCode      = 0;

    CFE_MSG_GetSize(MsgPtr, &ActualLength);

    /*
    ** Verify the command packet length.
    */
    if (ExpectedLength != ActualLength)
    {
        CFE_MSG_GetMsgId(MsgPtr, &MsgId);
        CFE_MSG_GetFcnCode(MsgPtr, &FcnCode);

        CFE_EVS_SendEvent(MM_CMD_LEN_ERR_EID,
                          CFE_EVS_EventType_ERROR,
                          "Invalid Msg length: ID = 0x%X,  CC = %u, Len = %u, Expected = %u",
                          (unsigned int)CFE_SB_MsgIdToValue(MsgId),
                          (unsigned int)FcnCode,
                          (unsigned int)ActualLength,
                          (unsigned int)ExpectedLength);

        result = false;

        MM_AppData.HkTlm.Payload.ErrCounter++;
    }

    return result;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Process a command pipe message                                  */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void MM_ProcessGroundCommand(const CFE_SB_Buffer_t *BufPtr)
{
    CFE_MSG_FcnCode_t CommandCode = 0;

    MM_ResetHk(); /* Clear all "Last Action" data */

    CFE_MSG_GetFcnCode(&BufPtr->Msg, &CommandCode);
    switch (CommandCode)
    {
        case MM_NOOP_CC:
            if (MM_VerifyCmdLength(&BufPtr->Msg, sizeof(MM_NoopCmd_t)))
            {
                MM_NoopCmd((MM_NoopCmd_t *)BufPtr);
            }
            break;

        case MM_RESET_COUNTERS_CC:
            if (MM_VerifyCmdLength(&BufPtr->Msg, sizeof(MM_ResetCountersCmd_t)))
            {
                MM_ResetCountersCmd((MM_ResetCountersCmd_t *)BufPtr);
            }
            break;

        case MM_PEEK_CC:
            if (MM_VerifyCmdLength(&BufPtr->Msg, sizeof(MM_PeekCmd_t)))
            {
                MM_PeekCmd((MM_PeekCmd_t *)BufPtr);
            }
            break;

        case MM_POKE_CC:
            if (MM_VerifyCmdLength(&BufPtr->Msg, sizeof(MM_PokeCmd_t)))
            {
                MM_PokeCmd((MM_PokeCmd_t *)BufPtr);
            }
            break;

        case MM_LOAD_MEM_WID_CC:
            if (MM_VerifyCmdLength(&BufPtr->Msg, sizeof(MM_LoadMemWIDCmd_t)))
            {
                MM_LoadMemWIDCmd((MM_LoadMemWIDCmd_t *)BufPtr);
            }
            break;

        case MM_LOAD_MEM_FROM_FILE_CC:
            if (MM_VerifyCmdLength(&BufPtr->Msg, sizeof(MM_LoadMemFromFileCmd_t)))
            {
                MM_LoadMemFromFileCmd((MM_LoadMemFromFileCmd_t *)BufPtr);
            }
            break;

        case MM_DUMP_MEM_TO_FILE_CC:
            if (MM_VerifyCmdLength(&BufPtr->Msg, sizeof(MM_DumpMemToFileCmd_t)))
            {
                MM_DumpMemToFileCmd((MM_DumpMemToFileCmd_t *)BufPtr);
            }
            break;

        case MM_DUMP_IN_EVENT_CC:
            if (MM_VerifyCmdLength(&BufPtr->Msg, sizeof(MM_DumpInEventCmd_t)))
            {
                MM_DumpInEventCmd((MM_DumpInEventCmd_t *)BufPtr);
            }
            break;

        case MM_FILL_MEM_CC:
            if (MM_VerifyCmdLength(&BufPtr->Msg, sizeof(MM_FillMemCmd_t)))
            {
                MM_FillMemCmd((MM_FillMemCmd_t *)BufPtr);
            }
            break;

        case MM_LOOKUP_SYM_CC:
            if (MM_VerifyCmdLength(&BufPtr->Msg, sizeof(MM_LookupSymCmd_t)))
            {
                MM_LookupSymCmd((MM_LookupSymCmd_t *)BufPtr);
            }
            break;

        case MM_SYM_TBL_TO_FILE_CC:
            if (MM_VerifyCmdLength(&BufPtr->Msg, sizeof(MM_SymTblToFileCmd_t)))
            {
                MM_SymTblToFileCmd((MM_SymTblToFileCmd_t *)BufPtr);
            }
            break;

        case MM_EEPROM_WRITE_ENA_CC:
            if (MM_VerifyCmdLength(&BufPtr->Msg, sizeof(MM_EepromWriteEnaCmd_t)))
            {
                MM_EepromWriteEnaCmd((MM_EepromWriteEnaCmd_t *)BufPtr);
            }
            break;

        case MM_EEPROM_WRITE_DIS_CC:
            if (MM_VerifyCmdLength(&BufPtr->Msg, sizeof(MM_EepromWriteDisCmd_t)))
            {
                MM_EepromWriteDisCmd((MM_EepromWriteDisCmd_t *)BufPtr);
            }
            break;

        default:
            MM_AppData.HkTlm.Payload.ErrCounter++;
            CFE_EVS_SendEvent(MM_CC_ERR_EID, CFE_EVS_EventType_ERROR, "Invalid ground command code %d", CommandCode);
            break;
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*     This routine will process any packet that is received on the SAMPLE    */
/*     command pipe.                                                          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void MM_TaskPipe(const CFE_SB_Buffer_t *BufPtr)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;

    CFE_MSG_GetMsgId(&BufPtr->Msg, &MsgId);

    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        case MM_CMD_MID:
            MM_ProcessGroundCommand(BufPtr);
            break;

        case MM_SEND_HK_MID:
            MM_SendHkCmd((MM_SendHkCmd_t *)BufPtr);
            break;

        default:
            /*
            ** Unrecognized Message ID
            */
            MM_AppData.HkTlm.Payload.ErrCounter++;
            CFE_EVS_SendEvent(MM_MID_ERR_EID,
                              CFE_EVS_EventType_ERROR,
                              "Invalid command pipe message ID: 0x%08lX",
                              (unsigned long)CFE_SB_MsgIdToValue(MsgId));
            break;
    }
}
