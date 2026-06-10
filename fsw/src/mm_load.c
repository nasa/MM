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
 *   Provides functions for the execution of the CFS Memory Manager
 *   load and fill ground commands
 */

/*************************************************************************
** Includes
*************************************************************************/
#include "mm_load.h"
#include "mm_app.h"
#include "mm_eventids.h"
#include "mm_mem16.h"
#include "mm_mem32.h"
#include "mm_mem8.h"
#include "mm_mission_cfg.h"
#include "mm_perfids.h"
#include "mm_utils.h"
#include <string.h>

/*************************************************************************
** External Data
*************************************************************************/
extern MM_AppData_t MM_AppData;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Write 8, 16, or 32 bits of data to any RAM memory address       */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CFE_Status_t MM_PokeMem(const MM_PokeCmd_t *CmdPtr, cpuaddr DestAddress)
{
    uint8        ByteValue;
    uint16       WordValue;
    CFE_Status_t PSP_Status     = CFE_PSP_ERROR_NOT_IMPLEMENTED;
    uint32       DataValue      = 0;
    size_t       BytesProcessed = 0;
    size_t       DataSize       = 0; /* only used for giving MEM type/size in events */
    uint32       EventID        = 0;

    /*
     * Defense-in-depth: re-validate the DataSize field before
     * dispatching to the platform write primitives.  Today the
     * validity of this field is also enforced upstream in
     * MM_VerifyPeekPokeParams() before MM_PokeMem() is called,
     * but this function is the one that holds the
     * CFE_PSP_MemWrite{8,16,32}() calls and should not depend on
     * a cross-translation-unit invariant for memory safety.  Any
     * future code path that reaches MM_PokeMem() without going
     * through MM_VerifyPeekPokeParams() (refactor of MM_PokeCmd
     * dispatch, new ground-command entry that delegates to
     * MM_PokeMem, etc.) would otherwise drop straight into the
     * switch with PSP_Status left at its initialised value of
     * CFE_PSP_ERROR_NOT_IMPLEMENTED and the command counter
     * unchanged, but no diagnostic event sent --- silently
     * failing to update HK and silently failing to log.
     */
    if (CmdPtr->Payload.DataSize != MM_INTERNAL_BYTE_BIT_WIDTH &&
        CmdPtr->Payload.DataSize != MM_INTERNAL_WORD_BIT_WIDTH &&
        CmdPtr->Payload.DataSize != MM_INTERNAL_DWORD_BIT_WIDTH)
    {
        CFE_EVS_SendEvent(MM_DATA_SIZE_BITS_ERR_EID,
                          CFE_EVS_EventType_ERROR,
                          "MM_PokeMem invalid data size: %u",
                          (unsigned int)CmdPtr->Payload.DataSize);
        return CFE_PSP_ERROR;
    }

    /* Write input number of bits to destination address */
    switch (CmdPtr->Payload.DataSize)
    {
        case MM_INTERNAL_BYTE_BIT_WIDTH:
            ByteValue      = (uint8)CmdPtr->Payload.Data;
            DataValue      = (uint32)ByteValue;
            BytesProcessed = sizeof(uint8);
            DataSize       = 8;
            if ((PSP_Status = CFE_PSP_MemWrite8(DestAddress, ByteValue)) == CFE_PSP_SUCCESS)
            {
                EventID = MM_POKE_BYTE_INF_EID;
            }
            break;

        case MM_INTERNAL_WORD_BIT_WIDTH:
            WordValue      = (uint16)CmdPtr->Payload.Data;
            DataValue      = (uint32)WordValue;
            BytesProcessed = sizeof(uint16);
            DataSize       = 16;
            if ((PSP_Status = CFE_PSP_MemWrite16(DestAddress, WordValue)) == CFE_PSP_SUCCESS)
            {
                EventID = MM_POKE_WORD_INF_EID;
            }
            break;

        case MM_INTERNAL_DWORD_BIT_WIDTH:
            DataValue      = CmdPtr->Payload.Data;
            BytesProcessed = sizeof(uint32);
            DataSize       = 32;
            if ((PSP_Status = CFE_PSP_MemWrite32(DestAddress, DataValue)) == CFE_PSP_SUCCESS)
            {
                EventID = MM_POKE_DWORD_INF_EID;
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
        /* Update cmd counter and last action stats */
        MM_AppData.HkTlm.Payload.LastAction     = MM_LastAction_POKE;
        MM_AppData.HkTlm.Payload.MemType        = CmdPtr->Payload.MemType;
        MM_AppData.HkTlm.Payload.Address        = CFE_ES_MEMADDRESS_C(DestAddress);
        MM_AppData.HkTlm.Payload.DataValue      = DataValue;
        MM_AppData.HkTlm.Payload.BytesProcessed = BytesProcessed;

        CFE_EVS_SendEvent(EventID,
                          CFE_EVS_EventType_INFORMATION,
                          "Poke Command: Addr = %p, Size = %u bits, Data = 0x%08X",
                          (void *)DestAddress,
                          (unsigned int)DataSize,
                          (unsigned int)DataValue);
    }
    else
    {
        CFE_EVS_SendEvent(MM_PSP_WRITE_ERR_EID,
                          CFE_EVS_EventType_ERROR,
                          "PSP write memory error: RC=0x%08X, Address=%p, MemType=MEM%u",
                          (unsigned int)PSP_Status,
                          (void *)DestAddress,
                          (unsigned int)DataSize);
    }

    return PSP_Status;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Write 8, 16, or 32 bits of data to any EEPROM memory address    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CFE_Status_t MM_PokeEeprom(const MM_PokeCmd_t *CmdPtr, cpuaddr DestAddress)
{
    uint8        ByteValue;
    uint16       WordValue;
    CFE_Status_t PSP_Status     = CFE_PSP_ERROR_NOT_IMPLEMENTED;
    uint32       DataValue      = 0;
    size_t       BytesProcessed = 0;

    CFE_ES_PerfLogEntry(MM_EEPROM_POKE_PERF_ID);

    /* Write input number of bits to destination address */
    switch (CmdPtr->Payload.DataSize)
    {
        case MM_INTERNAL_BYTE_BIT_WIDTH:
            ByteValue      = (uint8)CmdPtr->Payload.Data;
            DataValue      = (uint32)ByteValue;
            BytesProcessed = sizeof(uint8);
            PSP_Status     = CFE_PSP_EepromWrite8(DestAddress, ByteValue);
            if (PSP_Status != CFE_PSP_SUCCESS)
            {
                CFE_EVS_SendEvent(MM_OS_EEPROMWRITE8_ERR_EID,
                                  CFE_EVS_EventType_ERROR,
                                  "CFE_PSP_EepromWrite8 error received: RC = 0x%08X, Addr = %p",
                                  (unsigned int)PSP_Status,
                                  (void *)DestAddress);
            }
            else
            {
                CFE_EVS_SendEvent(MM_POKE_BYTE_INF_EID,
                                  CFE_EVS_EventType_INFORMATION,
                                  "Poke Command: Addr = %p, Size = 8 bits, Data = 0x%02X",
                                  (void *)DestAddress,
                                  ByteValue);
            }
            break;

        case MM_INTERNAL_WORD_BIT_WIDTH:
            WordValue      = (uint16)CmdPtr->Payload.Data;
            DataValue      = (uint32)WordValue;
            BytesProcessed = sizeof(uint16);
            PSP_Status     = CFE_PSP_EepromWrite16(DestAddress, WordValue);
            if (PSP_Status != CFE_PSP_SUCCESS)
            {
                CFE_EVS_SendEvent(MM_OS_EEPROMWRITE16_ERR_EID,
                                  CFE_EVS_EventType_ERROR,
                                  "CFE_PSP_EepromWrite16 error received: RC = 0x%08X, Addr = %p",
                                  (unsigned int)PSP_Status,
                                  (void *)DestAddress);
            }
            else
            {
                CFE_EVS_SendEvent(MM_POKE_WORD_INF_EID,
                                  CFE_EVS_EventType_INFORMATION,
                                  "Poke Command: Addr = %p, Size = 16 bits, Data = 0x%04X",
                                  (void *)DestAddress,
                                  WordValue);
            }
            break;

        case MM_INTERNAL_DWORD_BIT_WIDTH:
            DataValue      = CmdPtr->Payload.Data;
            BytesProcessed = sizeof(uint32);
            PSP_Status     = CFE_PSP_EepromWrite32(DestAddress, CmdPtr->Payload.Data);
            if (PSP_Status != CFE_PSP_SUCCESS)
            {
                CFE_EVS_SendEvent(MM_OS_EEPROMWRITE32_ERR_EID,
                                  CFE_EVS_EventType_ERROR,
                                  "CFE_PSP_EepromWrite32 error received: RC = 0x%08X, Addr = %p",
                                  (unsigned int)PSP_Status,
                                  (void *)DestAddress);
            }
            else
            {
                CFE_EVS_SendEvent(MM_POKE_DWORD_INF_EID,
                                  CFE_EVS_EventType_INFORMATION,
                                  "Poke Command: Addr = %p, Size = 32 bits, Data = 0x%08X",
                                  (void *)DestAddress,
                                  (unsigned int)(CmdPtr->Payload.Data));
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
        /* Update cmd counter and last action stats */
        MM_AppData.HkTlm.Payload.LastAction     = MM_LastAction_POKE;
        MM_AppData.HkTlm.Payload.MemType        = CmdPtr->Payload.MemType;
        MM_AppData.HkTlm.Payload.Address        = CFE_ES_MEMADDRESS_C(DestAddress);
        MM_AppData.HkTlm.Payload.DataValue      = DataValue;
        MM_AppData.HkTlm.Payload.BytesProcessed = BytesProcessed;
    }

    CFE_ES_PerfLogExit(MM_EEPROM_POKE_PERF_ID);

    return PSP_Status;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Loads memory from a file                                        */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 MM_LoadMemFromFile(osal_id_t                      FileHandle,
                         const char                    *FileName,
                         const MM_LoadDumpFileHeader_t *FileHeader,
                         cpuaddr                        DestAddress)
{
    int32  BytesRemaining = FileHeader->NumOfBytes;
    size_t BytesProcessed = 0;
    size_t SegmentSize    = MM_INTERNAL_MAX_LOAD_DATA_SEG;
    uint8 *ioBuffer       = (uint8 *)&MM_AppData.LoadBuffer[0];
    uint8 *TargetPointer  = (uint8 *)DestAddress;
    int32  Status         = OS_SUCCESS;
    int32  ReadLength;

    if (FileHeader->MemType == MM_MemType_EEPROM)
    {
        CFE_ES_PerfLogEntry(MM_EEPROM_FILELOAD_PERF_ID);
    }

    while (BytesRemaining != 0)
    {
        if (BytesRemaining < MM_INTERNAL_MAX_LOAD_DATA_SEG)
        {
            SegmentSize = BytesRemaining;
        }

        if ((ReadLength = OS_read(FileHandle, ioBuffer, SegmentSize)) == SegmentSize)
        {
            memcpy(TargetPointer, ioBuffer, SegmentSize);

            BytesRemaining -= SegmentSize;
            BytesProcessed += SegmentSize;
            TargetPointer  += SegmentSize;

            /* Prevent CPU hogging between load segments */
            if (BytesRemaining != 0)
            {
                MM_SegmentBreak();
            }
        }
        else
        {
            BytesRemaining = 0;
            Status         = OS_ERROR;
            CFE_EVS_SendEvent(MM_OS_READ_ERR_EID,
                              CFE_EVS_EventType_ERROR,
                              "OS_read error received: RC = 0x%08X Expected = %u File = '%s'",
                              (unsigned int)ReadLength,
                              (unsigned int)SegmentSize,
                              FileName);
        }
    }

    if (FileHeader->MemType == MM_MemType_EEPROM)
    {
        CFE_ES_PerfLogExit(MM_EEPROM_FILELOAD_PERF_ID);
    }

    /* Update last action statistics */
    if (BytesProcessed == FileHeader->NumOfBytes)
    {
        MM_AppData.HkTlm.Payload.LastAction     = MM_LastAction_LOAD_FROM_FILE;
        MM_AppData.HkTlm.Payload.MemType        = FileHeader->MemType;
        MM_AppData.HkTlm.Payload.Address        = CFE_ES_MEMADDRESS_C(DestAddress);
        MM_AppData.HkTlm.Payload.BytesProcessed = BytesProcessed;
        strncpy(MM_AppData.HkTlm.Payload.FileName, FileName, CFE_MISSION_MAX_PATH_LEN);
    }
    else
    {
        Status = OS_ERROR;
    }

    return Status;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Verify load file size                                           */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 MM_VerifyLoadFileSize(const char *FileName, const MM_LoadDumpFileHeader_t *FileHeader)
{
    int32      OS_Status;
    size_t     ExpectedSize;
    int32      ActualSize; /* The size returned by OS_stat is signed */
    os_fstat_t FileStats;

    memset(&FileStats, 0, sizeof(FileStats));

    /*
    ** Get the filesystem statistics on our load file
    */
    OS_Status = OS_stat(FileName, &FileStats);
    if (OS_Status != OS_SUCCESS)
    {
        CFE_EVS_SendEvent(MM_OS_STAT_ERR_EID,
                          CFE_EVS_EventType_ERROR,
                          "OS_stat error received: RC = 0x%08X File = '%s'",
                          (unsigned int)OS_Status,
                          FileName);
    }
    else
    {
        /*
        ** Check the reported size of the file against what it should be based
        ** upon the number of load bytes specified in the file header
        */
        ActualSize   = OS_FILESTAT_SIZE(FileStats);
        ExpectedSize = FileHeader->NumOfBytes + sizeof(CFE_FS_Header_t) + sizeof(MM_LoadDumpFileHeader_t);
        if (ActualSize != ExpectedSize)
        {
            OS_Status = OS_ERR_INVALID_SIZE;

            /*
            ** Note: passing FileStats.st_size in this event message will cause
            ** a segmentation fault under cygwin during unit testing, so we added
            ** the variable ActualSize to this function.
            */
            CFE_EVS_SendEvent(MM_LD_FILE_SIZE_ERR_EID,
                              CFE_EVS_EventType_ERROR,
                              "Load file size error: Reported by OS = %d Expected = %u File = '%s'",
                              (int)ActualSize,
                              (unsigned int)ExpectedSize,
                              FileName);
        }
    }

    return OS_Status;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Read the cFE primary and MM secondary file headers          */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 MM_ReadFileHeaders(const char              *FileName,
                         osal_id_t                FileHandle,
                         CFE_FS_Header_t         *CFEHeader,
                         MM_LoadDumpFileHeader_t *MMHeader)
{
    int32 OS_Status;

    /*
    ** Read in the primary cFE file header
    */
    OS_Status = CFE_FS_ReadHeader(CFEHeader, FileHandle);
    if (OS_Status != sizeof(CFE_FS_Header_t))
    {
        /* We either got an error or didn't read as much data as expected */
        CFE_EVS_SendEvent(MM_CFE_FS_READHDR_ERR_EID,
                          CFE_EVS_EventType_ERROR,
                          "CFE_FS_ReadHeader error received: RC = 0x%08X Expected "
                          "= %u File = '%s'",
                          (unsigned int)OS_Status,
                          (unsigned int)sizeof(CFE_FS_Header_t),
                          FileName);

        OS_Status = OS_ERR_INVALID_SIZE;
    } /* end CFE_FS_ReadHeader if */
    else
    {
        /*
        ** Read in the secondary MM file header
        */
        OS_Status = OS_read(FileHandle, MMHeader, sizeof(MM_LoadDumpFileHeader_t));
        if (OS_Status != sizeof(MM_LoadDumpFileHeader_t))
        {
            /* We either got an error or didn't read as much data as expected */
            CFE_EVS_SendEvent(MM_OS_READ_EXP_ERR_EID,
                              CFE_EVS_EventType_ERROR,
                              "OS_read error received: RC = 0x%08X Expected = %u File = '%s'",
                              (unsigned int)OS_Status,
                              (unsigned int)sizeof(MM_LoadDumpFileHeader_t),
                              FileName);

            OS_Status = OS_ERR_INVALID_SIZE;
        }
        else
        {
            OS_Status = OS_SUCCESS;
        } /* end OS_read if */
    } /* end CFE_FS_ReadHeader else */

    return OS_Status;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Fill memory with the command specified fill pattern             */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void MM_FillMem(cpuaddr DestAddress, const MM_FillMemCmd_t *CmdPtr)
{
    uint16 i;
    size_t BytesProcessed = 0;
    uint32 BytesRemaining = CmdPtr->Payload.NumOfBytes;
    uint32 SegmentSize    = MM_INTERNAL_MAX_FILL_DATA_SEG;
    uint8 *TargetPointer  = (uint8 *)DestAddress;
    uint8 *FillBuffer     = (uint8 *)&MM_AppData.FillBuffer[0];

    /* Create a scratch buffer with one fill segment */
    for (i = 0; i < (MM_INTERNAL_MAX_FILL_DATA_SEG / sizeof(uint32)); i++)
    {
        FillBuffer[i] = CmdPtr->Payload.FillPattern;
    }

    /* Start EEPROM performance monitor */
    if (CmdPtr->Payload.MemType == MM_MemType_EEPROM)
    {
        CFE_ES_PerfLogEntry(MM_EEPROM_FILL_PERF_ID);
    }

    /* Fill memory one segment at a time */
    while (BytesRemaining != 0)
    {
        /* Last fill segment may be partial size */
        if (BytesRemaining < MM_INTERNAL_MAX_FILL_DATA_SEG)
        {
            SegmentSize = BytesRemaining;
        }

        memcpy(TargetPointer, FillBuffer, SegmentSize);

        TargetPointer  += SegmentSize;
        BytesProcessed += SegmentSize;
        BytesRemaining -= SegmentSize;

        /* Prevent CPU hogging between load segments */
        if (BytesRemaining != 0)
        {
            MM_SegmentBreak();
        }
    }

    /* Stop EEPROM performance monitor */
    if (CmdPtr->Payload.MemType == MM_MemType_EEPROM)
    {
        CFE_ES_PerfLogExit(MM_EEPROM_FILL_PERF_ID);
    }

    /* Update last action statistics */
    MM_AppData.HkTlm.Payload.LastAction     = MM_LastAction_FILL;
    MM_AppData.HkTlm.Payload.MemType        = CmdPtr->Payload.MemType;
    MM_AppData.HkTlm.Payload.Address        = CFE_ES_MEMADDRESS_C(DestAddress);
    MM_AppData.HkTlm.Payload.DataValue      = CmdPtr->Payload.FillPattern;
    MM_AppData.HkTlm.Payload.BytesProcessed = BytesProcessed;

    return;
}
