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
 *   Provides CFS Memory Manager functions that are used
 *   for the conditionally compiled MM_MEM32 optional memory type.
 */

/*************************************************************************
** Includes
*************************************************************************/
#include "mm_mem32.h"
#include "mm_app.h"
#include "mm_events.h"
#include "mm_utils.h"
#include <string.h>

/*
** The code in this file is optional.
** See mm_platform_cfg.h to set this compiler switch.
*/
#ifdef MM_OPT_CODE_MEM32_MEMTYPE

/*************************************************************************
** External Data
*************************************************************************/
extern MM_AppData_t MM_AppData;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Load memory from a file using only 32 bit wide writes           */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
bool MM_LoadMem32FromFile(osal_id_t FileHandle, const char *FileName, const MM_LoadDumpFileHeader_t *FileHeader,
                          cpuaddr DestAddress)
{
    uint32       i;
    int32        ReadLength;
    CFE_Status_t PSP_Status     = CFE_PSP_SUCCESS;
    size_t       BytesProcessed = 0;
    int32        BytesRemaining = FileHeader->NumOfBytes;
    uint32 *     DataPointer32  = (uint32 *)DestAddress;
    uint32 *     ioBuffer32     = (uint32 *)&MM_AppData.LoadBuffer[0];
    size_t       SegmentSize    = MM_MAX_LOAD_DATA_SEG;
    bool         Valid          = false;

    while (BytesRemaining != 0)
    {
        if (BytesRemaining < MM_MAX_LOAD_DATA_SEG)
        {
            SegmentSize = BytesRemaining;
        }

        /* Read file data into i/o buffer */
        if ((ReadLength = OS_read(FileHandle, ioBuffer32, SegmentSize)) != SegmentSize)
        {
            BytesRemaining = 0;
            CFE_EVS_SendEvent(MM_OS_READ_ERR_EID, CFE_EVS_EventType_ERROR,
                              "OS_read error received: RC = 0x%08X Expected = %u File = '%s'", (unsigned int)ReadLength,
                              (unsigned int)SegmentSize, FileName);
        }
        else
        {
            /* Load memory from i/o buffer using 32 bit wide writes */
            for (i = 0; i < (SegmentSize / sizeof(uint32)); i++)
            {
                PSP_Status = CFE_PSP_MemWrite32((cpuaddr)DataPointer32, ioBuffer32[i]);
                if (PSP_Status == CFE_PSP_SUCCESS)
                {
                    DataPointer32++;
                }
                else
                {
                    /* CFE_PSP_MemWrite32 error */
                    BytesRemaining = 0;
                    CFE_EVS_SendEvent(MM_PSP_WRITE_ERR_EID, CFE_EVS_EventType_ERROR,
                                      "PSP write memory error: RC=%d, Address=%p, MemType=MEM32", (int)PSP_Status,
                                      (void *)DataPointer32);
                    /* Stop load segment loop */
                    break;
                }
            }

            if (PSP_Status == CFE_PSP_SUCCESS)
            {
                BytesProcessed += SegmentSize;
                BytesRemaining -= SegmentSize;

                /* Prevent CPU hogging between load segments */
                if (BytesRemaining != 0)
                {
                    MM_SegmentBreak();
                }
            }
        }
    }

    /* Update last action statistics */
    if (BytesProcessed == FileHeader->NumOfBytes)
    {
        Valid                                      = true;
        MM_AppData.HkPacket.Payload.LastAction     = MM_LOAD_FROM_FILE;
        MM_AppData.HkPacket.Payload.MemType        = MM_MEM32;
        MM_AppData.HkPacket.Payload.Address        = DestAddress;
        MM_AppData.HkPacket.Payload.BytesProcessed = BytesProcessed;
        strncpy(MM_AppData.HkPacket.Payload.FileName, FileName, OS_MAX_PATH_LEN);
    }

    return Valid;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Dump the requested number of bytes from memory to a file using  */
/* only 32 bit wide reads                                          */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
bool MM_DumpMem32ToFile(osal_id_t FileHandle, const char *FileName, const MM_LoadDumpFileHeader_t *FileHeader)
{
    bool         Valid = true;
    int32        OS_Status;
    CFE_Status_t PSP_Status = CFE_PSP_SUCCESS;
    uint32       i;
    size_t       BytesProcessed = 0;
    uint32       BytesRemaining = FileHeader->NumOfBytes;
    uint32 *     DataPointer32  = (uint32 *)(FileHeader->SymAddress.Offset);
    uint32 *     ioBuffer32     = (uint32 *)&MM_AppData.DumpBuffer[0];
    size_t       SegmentSize    = MM_MAX_DUMP_DATA_SEG;

    while (BytesRemaining != 0)
    {
        if (BytesRemaining < MM_MAX_DUMP_DATA_SEG)
        {
            SegmentSize = BytesRemaining;
        }

        /* Load RAM data into i/o buffer */
        for (i = 0; i < (SegmentSize / sizeof(uint32)); i++)
        {
            PSP_Status = CFE_PSP_MemRead32((cpuaddr)DataPointer32, &ioBuffer32[i]);
            if (PSP_Status == CFE_PSP_SUCCESS)
            {
                DataPointer32++;
            }
            else
            {
                /* CFE_PSP_MemRead32 error */
                Valid          = false;
                BytesRemaining = 0;
                CFE_EVS_SendEvent(MM_PSP_READ_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "PSP read memory error: RC=0x%08X, Src=%p, Tgt=%p, Type=MEM32",
                                  (unsigned int)PSP_Status, (void *)DataPointer32, (void *)&ioBuffer32[i]);
                /* Stop load i/o buffer loop */
                break;
            }
        }

        /* Check for error loading i/o buffer */
        if (PSP_Status == CFE_PSP_SUCCESS)
        {
            /* Write i/o buffer contents to file */
            if ((OS_Status = OS_write(FileHandle, ioBuffer32, SegmentSize)) == SegmentSize)
            {
                /* Update process counters */
                BytesRemaining -= SegmentSize;
                BytesProcessed += SegmentSize;

                /* Prevent CPU hogging between dump segments */
                if (BytesRemaining != 0)
                {
                    MM_SegmentBreak();
                }
            }
            else
            {
                /* OS_write error */
                Valid          = false;
                BytesRemaining = 0;
                CFE_EVS_SendEvent(MM_OS_WRITE_EXP_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "OS_write error received: RC = 0x%08X Expected = %u File = '%s'",
                                  (unsigned int)OS_Status, (unsigned int)SegmentSize, FileName);
            }
        }
    }

    if (Valid)
    {
        /* Update last action statistics */
        MM_AppData.HkPacket.Payload.LastAction = MM_DUMP_TO_FILE;
        MM_AppData.HkPacket.Payload.MemType    = MM_MEM32;
        MM_AppData.HkPacket.Payload.Address    = FileHeader->SymAddress.Offset;
        strncpy(MM_AppData.HkPacket.Payload.FileName, FileName, OS_MAX_PATH_LEN);
        MM_AppData.HkPacket.Payload.BytesProcessed = BytesProcessed;
    }

    return Valid;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Fill memory with the command specified fill pattern using only  */
/* 32 bit wide writes                                              */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
bool MM_FillMem32(cpuaddr DestAddress, const MM_FillMemCmd_t *CmdPtr)
{
    uint32       i;
    CFE_Status_t PSP_Status     = CFE_PSP_SUCCESS;
    size_t       BytesProcessed = 0;
    uint32       BytesRemaining = CmdPtr->Payload.NumOfBytes;
    uint32       NewBytesRemaining;
    uint32       FillPattern32 = CmdPtr->Payload.FillPattern;
    uint32 *     DataPointer32 = (uint32 *)(DestAddress);
    size_t       SegmentSize   = MM_MAX_FILL_DATA_SEG;
    bool         Result        = true;

    /* Check fill size and warn if not a multiple of 4 */
    if ((BytesRemaining % 4) != 0)
    {
        NewBytesRemaining = BytesRemaining - (BytesRemaining % 4);
        CFE_EVS_SendEvent(MM_FILL_MEM32_ALIGN_WARN_INF_EID, CFE_EVS_EventType_INFORMATION,
                          "MM_FillMem32 NumOfBytes not multiple of 4. Reducing from %d to %d.", (int)BytesRemaining,
                          (int)NewBytesRemaining);
        BytesRemaining = NewBytesRemaining;
    }

    while (BytesRemaining != 0)
    {
        /* Set size of next segment */
        if (BytesRemaining < MM_MAX_FILL_DATA_SEG)
        {
            SegmentSize = BytesRemaining;
        }

        /* Fill next segment */
        for (i = 0; i < (SegmentSize / sizeof(uint32)); i++)
        {
            PSP_Status = CFE_PSP_MemWrite32((cpuaddr)DataPointer32, FillPattern32);
            if (PSP_Status == CFE_PSP_SUCCESS)
            {
                DataPointer32++;
            }
            else
            {
                /* CFE_PSP_MemWrite32 error */
                BytesRemaining = 0;
                Result         = false;
                CFE_EVS_SendEvent(MM_PSP_WRITE_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "PSP write memory error: RC=0x%08X, Address=%p, MemType=MEM32",
                                  (unsigned int)PSP_Status, (void *)DataPointer32);
                /* Stop fill segment loop */
                break;
            }
        }

        if (PSP_Status == CFE_PSP_SUCCESS)
        {
            /* Update process counters */
            BytesRemaining -= SegmentSize;
            BytesProcessed += SegmentSize;

            /* Prevent CPU hogging between fill segments */
            if (BytesRemaining != 0)
            {
                MM_SegmentBreak();
            }
        }
    }

    /* Update last action statistics */
    if (BytesProcessed == CmdPtr->Payload.NumOfBytes)
    {
        MM_AppData.HkPacket.Payload.LastAction     = MM_FILL;
        MM_AppData.HkPacket.Payload.MemType        = MM_MEM32;
        MM_AppData.HkPacket.Payload.Address        = DestAddress;
        MM_AppData.HkPacket.Payload.DataValue      = FillPattern32;
        MM_AppData.HkPacket.Payload.BytesProcessed = BytesProcessed;
    }

    return Result;
}

#endif /* MM_OPT_CODE_MEM32_MEMTYPE */
