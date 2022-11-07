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
 *   for the conditionally compiled MM_MEM16 optional memory type.
 */

/*************************************************************************
** Includes
*************************************************************************/
#include "mm_mem16.h"
#include "mm_app.h"
#include "mm_events.h"
#include "mm_utils.h"
#include <string.h>

/*
** The code in this file is optional.
** See mm_platform_cfg.h to set this compiler switch.
*/
#ifdef MM_OPT_CODE_MEM16_MEMTYPE

/*************************************************************************
** External Data
*************************************************************************/
extern MM_AppData_t MM_AppData;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Load memory from a file using only 16 bit wide writes           */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
bool MM_LoadMem16FromFile(osal_id_t FileHandle, const char *FileName, const MM_LoadDumpFileHeader_t *FileHeader,
                          cpuaddr DestAddress)
{
    uint32  i;
    int32   ReadLength;
    int32   PSP_Status     = CFE_PSP_SUCCESS;
    int32   BytesProcessed = 0;
    int32   BytesRemaining = FileHeader->NumOfBytes;
    uint16 *DataPointer16  = (uint16 *)DestAddress;
    uint16 *ioBuffer16     = (uint16 *)&MM_AppData.LoadBuffer[0];
    uint32  SegmentSize    = MM_MAX_LOAD_DATA_SEG;
    bool    Valid          = false;

    while (BytesRemaining != 0)
    {
        if (BytesRemaining < MM_MAX_LOAD_DATA_SEG)
        {
            SegmentSize = BytesRemaining;
        }

        /* Read file data into i/o buffer */
        if ((ReadLength = OS_read(FileHandle, ioBuffer16, SegmentSize)) != SegmentSize)
        {
            BytesRemaining = 0;
            CFE_EVS_SendEvent(MM_OS_READ_ERR_EID, CFE_EVS_EventType_ERROR,
                              "OS_read error received: RC = 0x%08X Expected = %d File = '%s'", (unsigned int)ReadLength,
                              (int)SegmentSize, FileName);
        }
        else
        {
            /* Load memory from i/o buffer using 16 bit wide writes */
            for (i = 0; i < (SegmentSize / sizeof(uint16)); i++)
            {
                PSP_Status = CFE_PSP_MemWrite16((cpuaddr)DataPointer16, ioBuffer16[i]);
                if (PSP_Status == CFE_PSP_SUCCESS)
                {
                    DataPointer16++;
                }
                else
                {
                    /* CFE_PSP_MemWrite16 error */
                    BytesRemaining = 0;
                    CFE_EVS_SendEvent(MM_PSP_WRITE_ERR_EID, CFE_EVS_EventType_ERROR,
                                      "PSP write memory error: RC=0x%08X, Address=%p, MemType=MEM16",
                                      (unsigned int)PSP_Status, (void *)DataPointer16);
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
        Valid                              = true;
        MM_AppData.HkPacket.LastAction     = MM_LOAD_FROM_FILE;
        MM_AppData.HkPacket.MemType        = MM_MEM16;
        MM_AppData.HkPacket.Address        = DestAddress;
        MM_AppData.HkPacket.BytesProcessed = BytesProcessed;
        strncpy(MM_AppData.HkPacket.FileName, FileName, OS_MAX_PATH_LEN);
    }

    return Valid;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Dump the requested number of bytes from memory to a file using  */
/* only 16 bit wide reads                                          */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
bool MM_DumpMem16ToFile(osal_id_t FileHandle, const char *FileName, const MM_LoadDumpFileHeader_t *FileHeader)
{
    bool    Valid = true;
    int32   OS_Status;
    int32   PSP_Status = CFE_PSP_SUCCESS;
    uint32  i;
    uint32  BytesProcessed = 0;
    uint32  BytesRemaining = FileHeader->NumOfBytes;
    uint16 *DataPointer16  = (uint16 *)(FileHeader->SymAddress.Offset);
    uint16 *ioBuffer16     = (uint16 *)&MM_AppData.DumpBuffer[0];
    uint32  SegmentSize    = MM_MAX_DUMP_DATA_SEG;

    while (BytesRemaining != 0)
    {
        if (BytesRemaining < MM_MAX_DUMP_DATA_SEG)
        {
            SegmentSize = BytesRemaining;
        }

        /* Load RAM data into i/o buffer */
        for (i = 0; i < (SegmentSize / sizeof(uint16)); i++)
        {
            if ((PSP_Status = CFE_PSP_MemRead16((cpuaddr)DataPointer16, &ioBuffer16[i])) == CFE_PSP_SUCCESS)
            {
                DataPointer16++;
            }
            else
            {
                /* CFE_PSP_MemRead16 error */
                Valid          = false;
                BytesRemaining = 0;
                CFE_EVS_SendEvent(MM_PSP_READ_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "PSP read memory error: RC=0x%08X, Src=%p, Tgt=%p, Type=MEM16",
                                  (unsigned int)PSP_Status, (void *)DataPointer16, (void *)&ioBuffer16[i]);
                /* Stop load i/o buffer loop */
                break;
            }
        }

        /* Check for error loading i/o buffer */
        if (PSP_Status == CFE_PSP_SUCCESS)
        {
            /* Write i/o buffer contents to file */
            if ((OS_Status = OS_write(FileHandle, ioBuffer16, SegmentSize)) == SegmentSize)
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
                                  "OS_write error received: RC = 0x%08X Expected = %d File = '%s'",
                                  (unsigned int)OS_Status, (int)SegmentSize, FileName);
            }
        }
    }

    if (Valid)
    {
        /* Update last action statistics */
        MM_AppData.HkPacket.LastAction = MM_DUMP_TO_FILE;
        MM_AppData.HkPacket.MemType    = MM_MEM16;
        MM_AppData.HkPacket.Address    = FileHeader->SymAddress.Offset;
        strncpy(MM_AppData.HkPacket.FileName, FileName, OS_MAX_PATH_LEN);
        MM_AppData.HkPacket.BytesProcessed = BytesProcessed;
    }

    return Valid;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Fill memory with the command specified fill pattern using only  */
/* 16 bit wide writes                                              */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
bool MM_FillMem16(cpuaddr DestAddress, const MM_FillMemCmd_t *CmdPtr)
{
    uint32  i;
    int32   PSP_Status     = CFE_PSP_SUCCESS;
    uint32  BytesProcessed = 0;
    uint32  BytesRemaining = CmdPtr->NumOfBytes;
    uint32  NewBytesRemaining;
    uint16  FillPattern16 = (uint16)CmdPtr->FillPattern;
    uint16 *DataPointer16 = (uint16 *)DestAddress;
    uint32  SegmentSize   = MM_MAX_FILL_DATA_SEG;
    bool    Result        = true;

    /* Check fill size and warn if not a multiple of 2 */
    if ((BytesRemaining % 2) != 0)
    {
        NewBytesRemaining = BytesRemaining - (BytesRemaining % 2);
        CFE_EVS_SendEvent(MM_FILL_MEM16_ALIGN_WARN_INF_EID, CFE_EVS_EventType_INFORMATION,
                          "MM_FillMem16 NumOfBytes not multiple of 2. Reducing from %d to %d.", BytesRemaining,
                          NewBytesRemaining);
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
        for (i = 0; i < (SegmentSize / sizeof(uint16)); i++)
        {
            PSP_Status = CFE_PSP_MemWrite16((cpuaddr)DataPointer16, FillPattern16);
            if (PSP_Status == CFE_PSP_SUCCESS)
            {
                DataPointer16++;
            }
            else
            {
                /* CFE_PSP_MemWrite16 error */
                Result         = false;
                BytesRemaining = 0;
                CFE_EVS_SendEvent(MM_PSP_WRITE_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "PSP write memory error: RC=0x%08X, Address=%p, MemType=MEM16",
                                  (unsigned int)PSP_Status, (void *)DataPointer16);
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
    if (BytesProcessed == CmdPtr->NumOfBytes)
    {
        MM_AppData.HkPacket.LastAction     = MM_FILL;
        MM_AppData.HkPacket.MemType        = MM_MEM16;
        MM_AppData.HkPacket.Address        = DestAddress;
        MM_AppData.HkPacket.DataValue      = (uint32)FillPattern16;
        MM_AppData.HkPacket.BytesProcessed = BytesProcessed;
    }

    return Result;
}

#endif /* MM_OPT_CODE_MEM16_MEMTYPE */
