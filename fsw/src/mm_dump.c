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
 *   Functions used for processing CFS Memory Manager memory dump commands
 */

/*************************************************************************
** Includes
*************************************************************************/
#include "mm_dump.h"
#include "mm_app.h"
#include "mm_eventids.h"
#include "mm_mem16.h"
#include "mm_mem32.h"
#include "mm_mem8.h"
#include "mm_mission_cfg.h"
#include "mm_utils.h"
#include <string.h>

/*************************************************************************
** External Data
*************************************************************************/
extern MM_AppData_t MM_AppData;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Read 8,16, or 32 bits of data from any given input address      */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 MM_PeekMem(const MM_PeekCmd_t *CmdPtr, cpuaddr SrcAddress)
{
    uint8  ByteValue      = 0;
    uint16 WordValue      = 0;
    uint32 DWordValue     = 0;
    int32  PSP_Status     = CFE_PSP_ERROR_NOT_IMPLEMENTED;
    size_t BytesProcessed = 0;
    uint32 DataValue      = 0;
    size_t DataSize       = 0;
    uint32 EventID        = 0;

    /*
    ** Read the requested number of bytes and report in an event message
    */
    switch (CmdPtr->Payload.DataSize)
    {
        case MM_INTERNAL_BYTE_BIT_WIDTH:

            PSP_Status = CFE_PSP_MemRead8(SrcAddress, &ByteValue);
            DataSize   = 8;
            if (PSP_Status == CFE_PSP_SUCCESS)
            {
                DataValue      = (uint32)ByteValue;
                BytesProcessed = sizeof(uint8);
                EventID        = MM_PEEK_BYTE_INF_EID;
            }
            break;

        case MM_INTERNAL_WORD_BIT_WIDTH:

            PSP_Status = CFE_PSP_MemRead16(SrcAddress, &WordValue);
            DataSize   = 16;
            if (PSP_Status == CFE_PSP_SUCCESS)
            {
                DataValue      = (uint32)WordValue;
                BytesProcessed = sizeof(uint16);
                EventID        = MM_PEEK_WORD_INF_EID;
            }
            break;

        case MM_INTERNAL_DWORD_BIT_WIDTH:

            PSP_Status = CFE_PSP_MemRead32(SrcAddress, &DWordValue);
            DataSize   = 32;
            if (PSP_Status == CFE_PSP_SUCCESS)
            {
                DataValue      = DWordValue;
                BytesProcessed = sizeof(uint32);
                EventID        = MM_PEEK_DWORD_INF_EID;
            }
            break;

        /*
        ** We don't need a default case, a bad DataSize will get caught
        ** in the MM_VerifyPeekPokeParams function and we won't get here
        */
        default:
            break;
    }

    if (PSP_Status == CFE_PSP_SUCCESS)
    {
        MM_AppData.HkTlm.Payload.LastAction     = MM_LastAction_PEEK;
        MM_AppData.HkTlm.Payload.MemType        = CmdPtr->Payload.MemType;
        MM_AppData.HkTlm.Payload.Address        = CFE_ES_MEMADDRESS_C(SrcAddress);
        MM_AppData.HkTlm.Payload.BytesProcessed = BytesProcessed;
        MM_AppData.HkTlm.Payload.DataValue      = DataValue;

        CFE_EVS_SendEvent(EventID,
                          CFE_EVS_EventType_INFORMATION,
                          "Peek Command: Addr = %p Size = %u bits Data = 0x%08X",
                          (void *)SrcAddress,
                          (unsigned int)DataSize,
                          (unsigned int)DataValue);
    }
    else
    {
        CFE_EVS_SendEvent(MM_PSP_READ_ERR_EID,
                          CFE_EVS_EventType_ERROR,
                          "PSP read memory error: RC=%d, Address=%p, MemType=MEM%u",
                          (int)PSP_Status,
                          (void *)SrcAddress,
                          (unsigned int)DataSize);
    }

    return PSP_Status;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Dump the requested number of bytes from memory to a file        */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 MM_DumpMemToFile(osal_id_t FileHandle, const char *FileName, const MM_LoadDumpFileHeader_t *FileHeader)
{
    int32  OS_Status      = OS_SUCCESS;
    uint32 BytesRemaining = FileHeader->NumOfBytes;
    size_t BytesProcessed = 0;
    size_t SegmentSize    = MM_INTERNAL_MAX_DUMP_DATA_SEG;
    uint8 *SourcePtr      = CFE_ES_MEMADDRESS_TO_PTR(FileHeader->SymAddress.Offset);
    uint8 *ioBuffer       = (uint8 *)&MM_AppData.DumpBuffer[0];

    while (BytesRemaining != 0)
    {
        if (BytesRemaining < MM_INTERNAL_MAX_DUMP_DATA_SEG)
        {
            SegmentSize = BytesRemaining;
        }

        memcpy(ioBuffer, SourcePtr, SegmentSize);

        OS_Status = OS_write(FileHandle, ioBuffer, SegmentSize);
        if (OS_Status == SegmentSize)
        {
            SourcePtr      += SegmentSize;
            BytesRemaining -= SegmentSize;
            BytesProcessed += SegmentSize;

            /* Prevent CPU hogging between dump segments */
            if (BytesRemaining != 0)
            {
                MM_SegmentBreak();
            }

            OS_Status = OS_SUCCESS;
        }
        else
        {
            BytesRemaining = 0;
            CFE_EVS_SendEvent(MM_OS_WRITE_EXP_ERR_EID,
                              CFE_EVS_EventType_ERROR,
                              "OS_write error received: RC = %d, Expected = %u, File = '%s'",
                              (int)OS_Status,
                              (unsigned int)SegmentSize,
                              FileName);
        }
    }

    /* Update last action statistics */
    if (BytesProcessed == FileHeader->NumOfBytes)
    {
        MM_AppData.HkTlm.Payload.LastAction     = MM_LastAction_DUMP_TO_FILE;
        MM_AppData.HkTlm.Payload.MemType        = FileHeader->MemType;
        MM_AppData.HkTlm.Payload.Address        = FileHeader->SymAddress.Offset;
        MM_AppData.HkTlm.Payload.BytesProcessed = BytesProcessed;
        strncpy(MM_AppData.HkTlm.Payload.FileName, FileName, CFE_MISSION_MAX_PATH_LEN);
    }
    else
    {
        OS_Status = OS_ERROR;
    }

    return OS_Status;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Write the cFE primary and MM secondary file headers         */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 MM_WriteFileHeaders(const char                    *FileName,
                          osal_id_t                      FileHandle,
                          CFE_FS_Header_t               *CFEHeader,
                          const MM_LoadDumpFileHeader_t *MMHeader)
{
    int32 OS_Status;

    /*
    ** Write out the primary cFE file header
    */
    OS_Status = CFE_FS_WriteHeader(FileHandle, CFEHeader);
    if (OS_Status != sizeof(CFE_FS_Header_t))
    {
        /* We either got an error or didn't write as much data as expected */
        OS_Status = OS_ERR_INVALID_SIZE;
        CFE_EVS_SendEvent(MM_CFE_FS_WRITEHDR_ERR_EID,
                          CFE_EVS_EventType_ERROR,
                          "CFE_FS_WriteHeader error received: RC = %d Expected = %d File = '%s'",
                          (int)OS_Status,
                          (int)sizeof(CFE_FS_Header_t),
                          FileName);

    } /* end CFE_FS_WriteHeader if */
    else
    {
        /*
        ** Write out the secondary MM file header
        */
        OS_Status = OS_write(FileHandle, MMHeader, sizeof(MM_LoadDumpFileHeader_t));
        if (OS_Status != sizeof(MM_LoadDumpFileHeader_t))
        {
            /* We either got an error or didn't read as much data as expected */
            OS_Status = OS_ERR_INVALID_SIZE;
            CFE_EVS_SendEvent(MM_OS_WRITE_EXP_ERR_EID,
                              CFE_EVS_EventType_ERROR,
                              "OS_write error received: RC = %d Expected = %u File = '%s'",
                              (int)OS_Status,
                              (unsigned int)sizeof(MM_LoadDumpFileHeader_t),
                              FileName);
        }
        else
        {
            OS_Status = OS_SUCCESS;
        }
    } /* end CFE_FS_WriteHeader else */

    return OS_Status;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Fill a buffer with data to be dumped in an event message string */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 MM_FillDumpInEventBuffer(cpuaddr SrcAddress, const MM_DumpInEventCmd_t *CmdPtr, void *DumpBuffer)
{
#if defined(MM_INTERNAL_OPT_CODE_MEM8_MEMTYPE) || defined(MM_INTERNAL_OPT_CODE_MEM16_MEMTYPE) \
    || defined(MM_INTERNAL_OPT_CODE_MEM32_MEMTYPE)
    uint32 i;
#endif
    /* cppcheck-suppress unusedVariable */
    int32 PSP_Status = CFE_PSP_ERROR;

    /* Initialize buffer */
    memset(DumpBuffer, 0, MM_INTERNAL_MAX_DUMP_INEVENT_BYTES);

    switch (CmdPtr->Payload.MemType)
    {
        case MM_MemType_RAM:
        case MM_MemType_EEPROM:
            memcpy((void *)DumpBuffer, (void *)SrcAddress, CmdPtr->Payload.NumOfBytes);
            PSP_Status = CFE_PSP_SUCCESS;

            break;

#ifdef MM_INTERNAL_OPT_CODE_MEM32_MEMTYPE
        case MM_MemType_MEM32:
            for (i = 0; i < (CmdPtr->Payload.NumOfBytes / 4); i++)
            {
                PSP_Status = CFE_PSP_MemRead32(SrcAddress, (uint32 *)DumpBuffer);
                if (PSP_Status == CFE_PSP_SUCCESS)
                {
                    SrcAddress += sizeof(uint32);
                    DumpBuffer  = (uint8 *)DumpBuffer + sizeof(uint32);
                }
                else
                {
                    /* CFE_PSP_MemRead32 error */
                    CFE_EVS_SendEvent(MM_PSP_READ_ERR_EID,
                                      CFE_EVS_EventType_ERROR,
                                      "PSP read memory error: RC=%d, Src=%p, Tgt=%p, Type=MEM32",
                                      (int)PSP_Status,
                                      (void *)SrcAddress,
                                      (void *)DumpBuffer);
                    /* Stop load dump buffer loop */
                    break;
                }
            }
            break;
#endif /* MM_INTERNAL_OPT_CODE_MEM32_MEMTYPE */

#ifdef MM_INTERNAL_OPT_CODE_MEM16_MEMTYPE
        case MM_MemType_MEM16:
            for (i = 0; i < (CmdPtr->Payload.NumOfBytes / 2); i++)
            {
                PSP_Status = CFE_PSP_MemRead16(SrcAddress, (uint16 *)DumpBuffer);
                if (PSP_Status == CFE_PSP_SUCCESS)
                {
                    SrcAddress += sizeof(uint16);
                    DumpBuffer  = (uint8 *)DumpBuffer + sizeof(uint16);
                }
                else
                {
                    /* CFE_PSP_MemRead16 error */
                    CFE_EVS_SendEvent(MM_PSP_READ_ERR_EID,
                                      CFE_EVS_EventType_ERROR,
                                      "PSP read memory error: RC=%d, Src=%p, Tgt=%p, Type=MEM16",
                                      (int)PSP_Status,
                                      (void *)SrcAddress,
                                      (void *)DumpBuffer);
                    /* Stop load dump buffer loop */
                    break;
                }
            }
            break;
#endif /* MM_INTERNAL_OPT_CODE_MEM16_MEMTYPE */

#ifdef MM_INTERNAL_OPT_CODE_MEM8_MEMTYPE
        case MM_MemType_MEM8:
            for (i = 0; i < CmdPtr->Payload.NumOfBytes; i++)
            {
                PSP_Status = CFE_PSP_MemRead8(SrcAddress, (uint8 *)DumpBuffer);
                if (PSP_Status == CFE_PSP_SUCCESS)
                {
                    SrcAddress++;
                    DumpBuffer = (uint8 *)DumpBuffer + 1;
                }
                else
                {
                    /* CFE_PSP_MemRead8 error */
                    CFE_EVS_SendEvent(MM_PSP_READ_ERR_EID,
                                      CFE_EVS_EventType_ERROR,
                                      "PSP read memory error: RC=%d, Src=%p, Tgt=%p, Type=MEM8",
                                      (int)PSP_Status,
                                      (void *)SrcAddress,
                                      (void *)DumpBuffer);
                    /* Stop load dump buffer loop */
                    break;
                }
            }
            break;
#endif /* MM_INTERNAL_OPT_CODE_MEM8_MEMTYPE */
        default:
            /* This branch will never be executed. CmdPtr->Payload.MemType will always
             * be valid value for this switch statement it is verified via
             * MM_VerifyFileLoadDumpParams */
            break;

    } /* end CmdPtr->Payload.MemType switch */

    return PSP_Status;
}
