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

/* ======== */
/* Includes */
/* ======== */

#include "mm_app.h"
#include "mm_cmds.h"
#include "mm_dispatch.h"
#include "mm_eventids.h"
#include "mm_msg.h"
#include "mm_msgids.h"

#include "mm_eds_dictionary.h"
#include "mm_eds_dispatcher.h"

/*
 * Define a lookup table for MM command codes
 */
/* clang-format off */
static const EdsDispatchTable_EdsComponent_MM_Application_CFE_SB_Telecommand_t MM_TC_DISPATCH_TABLE =
{
    .CMD =
    {
        .NoopCmd_indication            = MM_NoopCmd,
        .ResetCountersCmd_indication   = MM_ResetCountersCmd,
        .LookupSymCmd_indication       = MM_LookupSymCmd,
        .SymTblToFileCmd_indication    = MM_SymTblToFileCmd,
        .EepromWriteEnaCmd_indication  = MM_EepromWriteEnaCmd,
        .EepromWriteDisCmd_indication  = MM_EepromWriteDisCmd,
        .PokeCmd_indication            = MM_PokeCmd,
        .LoadMemWIDCmd_indication      = MM_LoadMemWIDCmd,
        .LoadMemFromFileCmd_indication = MM_LoadMemFromFileCmd,
        .FillMemCmd_indication         = MM_FillMemCmd,
        .PeekCmd_indication            = MM_PeekCmd,
        .DumpMemToFileCmd_indication   = MM_DumpMemToFileCmd,
        .DumpInEventCmd_indication     = MM_DumpInEventCmd,
    },
    .SEND_HK =
    {
        .indication = MM_SendHkCmd
    }
};
/* clang-format on */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*     This routine will process any packet that is received on the MM        */
/*     command pipe.                                                          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void MM_TaskPipe(const CFE_SB_Buffer_t *BufPtr)
{
    CFE_Status_t      Status;
    CFE_SB_MsgId_t    MsgId;
    CFE_MSG_Size_t    MsgSize;
    CFE_MSG_FcnCode_t MsgFc;

    Status = EdsDispatch_EdsComponent_MM_Application_Telecommand(BufPtr, &MM_TC_DISPATCH_TABLE);

    if (Status != CFE_SUCCESS)
    {
        CFE_MSG_GetMsgId(&BufPtr->Msg, &MsgId);
        CFE_MSG_GetSize(&BufPtr->Msg, &MsgSize);
        CFE_MSG_GetFcnCode(&BufPtr->Msg, &MsgFc);
        ++MM_AppData.HkTlm.Payload.CommandErrorCounter;

        if (Status == CFE_STATUS_UNKNOWN_MSG_ID)
        {
            CFE_EVS_SendEvent(MM_MID_ERR_EID,
                              CFE_EVS_EventType_ERROR,
                              "MM: invalid command packet,MID = 0x%x",
                              (unsigned int)CFE_SB_MsgIdToValue(MsgId));
        }
        else if (Status == CFE_STATUS_WRONG_MSG_LENGTH)
        {
            CFE_EVS_SendEvent(MM_CMD_LEN_ERR_EID,
                              CFE_EVS_EventType_ERROR,
                              "MM: Invalid Msg length: ID = 0x%X, CC = %u, Len = %u",
                              (unsigned int)CFE_SB_MsgIdToValue(MsgId),
                              (unsigned int)MsgFc,
                              (unsigned int)MsgSize);
        }
        else
        {
            CFE_EVS_SendEvent(MM_CC_ERR_EID,
                              CFE_EVS_EventType_ERROR,
                              "MM: Invalid ground command code: CC = %d",
                              (int)MsgFc);
        }
    }
}
