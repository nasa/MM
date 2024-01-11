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
 *   Functions used for processing CFS Memory Manager memory dump commands
 */

/*************************************************************************
** Includes
*************************************************************************/
#include "mm_app.h"
#include "mm_dump.h"
#include "mm_events.h"
#include "mm_mem32.h"
#include "mm_mem16.h"
#include "mm_mem8.h"
#include "mm_utils.h"
#include "mm_mission_cfg.h"
#include <string.h>

/*************************************************************************
** External Data
*************************************************************************/
extern MM_AppData_t MM_AppData;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Memory peek command                                             */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
bool MM_PeekCmd(const CFE_SB_Buffer_t *BufPtr)
{
    bool                Valid;
    const MM_PeekCmd_t *CmdPtr;
    cpuaddr             SrcAddress     = 0;
    bool                Result         = false;
    MM_SymAddr_t        SrcSymAddress;

        CmdPtr = ((MM_PeekCmd_t *)BufPtr);

        SrcSymAddress = CmdPtr->Payload.SrcSymAddress;

        /* Resolve the symbolic address in command message */
        Valid = MM_ResolveSymAddr(&(SrcSymAddress), &SrcAddress);

        if (Valid == true)
        {
            /* Run necessary checks on command parameters */
            Valid = MM_VerifyPeekPokeParams(SrcAddress, CmdPtr->Payload.MemType, CmdPtr->Payload.DataSize);

            /* Check the specified memory type and call the appropriate routine */
            if (Valid == true)
            {
                /*
                ** We use this single peek routine for all memory types
                ** (including the optional ones)
                */
                Result = MM_PeekMem(CmdPtr, SrcAddress);
            }

        } /* end MM_ResolveSymAddr if */
        else
        {
            CFE_EVS_SendEvent(MM_SYMNAME_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Symbolic address can't be resolved: Name = '%s'", CmdPtr->Payload.SrcSymAddress.SymName);
        }

    return Result;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Read 8,16, or 32 bits of data from any given input address      */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
bool MM_PeekMem(const MM_PeekCmd_t *CmdPtr, cpuaddr SrcAddress)
{
    bool   ValidPeek      = false;
    uint8  ByteValue      = 0;
    uint16 WordValue      = 0;
    uint32 DWordValue     = 0;
    int32  PSP_Status     = 0;
    size_t BytesProcessed = 0;
    uint32 DataValue      = 0;
    size_t DataSize       = 0;
    uint32 EventID        = 0;

    /*
    ** Read the requested number of bytes and report in an event message
    */
    switch (CmdPtr->Payload.DataSize)
    {
        case MM_BYTE_BIT_WIDTH:

            PSP_Status = CFE_PSP_MemRead8(SrcAddress, &ByteValue);
            DataSize   = 8;
            if (PSP_Status == CFE_PSP_SUCCESS)
            {
                DataValue      = (uint32)ByteValue;
                BytesProcessed = sizeof(uint8);
                EventID        = MM_PEEK_BYTE_INF_EID;
                ValidPeek      = true;
            }
            break;

        case MM_WORD_BIT_WIDTH:

            PSP_Status = CFE_PSP_MemRead16(SrcAddress, &WordValue);
            DataSize   = 16;
            if (PSP_Status == CFE_PSP_SUCCESS)
            {
                DataValue      = (uint32)WordValue;
                BytesProcessed = sizeof(uint16);
                EventID        = MM_PEEK_WORD_INF_EID;
                ValidPeek      = true;
            }
            break;

        case MM_DWORD_BIT_WIDTH:

            PSP_Status = CFE_PSP_MemRead32(SrcAddress, &DWordValue);
            DataSize   = 32;
            if (PSP_Status == CFE_PSP_SUCCESS)
            {
                DataValue      = DWordValue;
                BytesProcessed = sizeof(uint32);
                EventID        = MM_PEEK_DWORD_INF_EID;
                ValidPeek      = true;
            }
            break;

        /*
        ** We don't need a default case, a bad DataSize will get caught
        ** in the MM_VerifyPeekPokeParams function and we won't get here
        */
        default:
            break;
    }

    if (ValidPeek)
    {
        MM_AppData.HkPacket.Payload.LastAction     = MM_PEEK;
        MM_AppData.HkPacket.Payload.MemType        = CmdPtr->Payload.MemType;
        MM_AppData.HkPacket.Payload.Address        = SrcAddress;
        MM_AppData.HkPacket.Payload.BytesProcessed = BytesProcessed;
        MM_AppData.HkPacket.Payload.DataValue      = DataValue;

        CFE_EVS_SendEvent(EventID, CFE_EVS_EventType_INFORMATION,
                          "Peek Command: Addr = %p Size = %u bits Data = 0x%08X", (void *)SrcAddress,
                          (unsigned int)DataSize, (unsigned int)DataValue);
    }
    else
    {
        CFE_EVS_SendEvent(MM_PSP_READ_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PSP read memory error: RC=%d, Address=%p, MemType=MEM%u", (int)PSP_Status, (void *)SrcAddress,
                          (unsigned int)DataSize);
    }

    return ValidPeek;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Dump memory to file command                                      */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
bool MM_DumpMemToFileCmd(const CFE_SB_Buffer_t *BufPtr)
{
    bool                         Valid = false;
    int32                        OS_Status;
    osal_id_t                    FileHandle = OS_OBJECT_ID_UNDEFINED;
    cpuaddr                      SrcAddress = 0;
    char                         FileName[OS_MAX_PATH_LEN];
    MM_SymAddr_t                 SrcSymAddress;
    const MM_DumpMemToFileCmd_t *CmdPtr;
    CFE_FS_Header_t              CFEFileHeader;
    MM_LoadDumpFileHeader_t      MMFileHeader;

        CmdPtr = ((MM_DumpMemToFileCmd_t *)BufPtr);

        SrcSymAddress = CmdPtr->Payload.SrcSymAddress;

        /* Make sure strings are null terminated before attempting to process them */
        CFE_SB_MessageStringGet(FileName, CmdPtr->Payload.FileName, NULL, sizeof(FileName), sizeof(CmdPtr->Payload.FileName));

        /* Resolve the symbolic address in command message */
        Valid = MM_ResolveSymAddr(&(SrcSymAddress), &SrcAddress);

        if (Valid == true)
        {
            /* Run necessary checks on command parameters */
            Valid = MM_VerifyLoadDumpParams(SrcAddress, CmdPtr->Payload.MemType, CmdPtr->Payload.NumOfBytes, MM_VERIFY_DUMP);

            if (Valid == true)
            {
                /*
                ** Initialize the cFE primary file header structure
                */
                CFE_FS_InitHeader(&CFEFileHeader, MM_CFE_HDR_DESCRIPTION, MM_CFE_HDR_SUBTYPE);

                /*
                ** Initialize the MM secondary file header structure
                */
                memset(&MMFileHeader, 0, sizeof(MMFileHeader));
                MMFileHeader.SymAddress.SymName[0] = MM_CLEAR_SYMNAME;

                /*
                ** Copy command data to file secondary header
                */
                MMFileHeader.SymAddress.Offset = SrcAddress;
                MMFileHeader.MemType           = CmdPtr->Payload.MemType;
                MMFileHeader.NumOfBytes        = CmdPtr->Payload.NumOfBytes;

                /*
                ** Create and open dump file
                */
                OS_Status =
                    OS_OpenCreate(&FileHandle, FileName, OS_FILE_FLAG_CREATE | OS_FILE_FLAG_TRUNCATE, OS_READ_WRITE);
                if (OS_Status == OS_SUCCESS)
                {
                    /* Write the file headers */
                    Valid = MM_WriteFileHeaders(FileName, FileHandle, &CFEFileHeader, &MMFileHeader);
                    if (Valid == true)
                    {
                        switch (MMFileHeader.MemType)
                        {
                            case MM_RAM:
                            case MM_EEPROM:
                                Valid = MM_DumpMemToFile(FileHandle, FileName, &MMFileHeader);
                                break;

#ifdef MM_OPT_CODE_MEM32_MEMTYPE
                            case MM_MEM32:
                                Valid = MM_DumpMem32ToFile(FileHandle, FileName, &MMFileHeader);
                                break;
#endif /* MM_OPT_CODE_MEM32_MEMTYPE */

#ifdef MM_OPT_CODE_MEM16_MEMTYPE
                            case MM_MEM16:
                                Valid = MM_DumpMem16ToFile(FileHandle, FileName, &MMFileHeader);
                                break;
#endif /* MM_OPT_CODE_MEM16_MEMTYPE */

#ifdef MM_OPT_CODE_MEM8_MEMTYPE
                            case MM_MEM8:
                                Valid = MM_DumpMem8ToFile(FileHandle, FileName, &MMFileHeader);
                                break;
#endif /* MM_OPT_CODE_MEM8_MEMTYPE */
                            default:
                                /* This branch will never be executed. MMFileHeader.MemType will always
                                 * be valid value for this switch statement it is verified via
                                 * MM_VerifyFileLoadDumpParams */
                                Valid = false;
                                break;
                        }

                        if (Valid == true)
                        {
                            /*
                            ** Compute CRC of dumped data
                            */
                            OS_Status = OS_lseek(
                                FileHandle, (sizeof(CFE_FS_Header_t) + sizeof(MM_LoadDumpFileHeader_t)), OS_SEEK_SET);
                            if (OS_Status != (sizeof(CFE_FS_Header_t) + sizeof(MM_LoadDumpFileHeader_t)))
                            {
                                Valid = false;
                            }
                            else
                            {
                                OS_Status = MM_ComputeCRCFromFile(FileHandle, &MMFileHeader.Crc, MM_DUMP_FILE_CRC_TYPE);
                                if (OS_Status == OS_SUCCESS)
                                {
                                    /*
                                    ** Rewrite the file headers. The subfunctions will take care of moving
                                    ** the file pointer to the beginning of the file so we don't need to do it
                                    ** here.
                                    */
                                    Valid = MM_WriteFileHeaders(FileName, FileHandle, &CFEFileHeader, &MMFileHeader);

                                } /* end MM_ComputeCRCFromFile if */
                                else
                                {
                                    Valid = false;
                                    CFE_EVS_SendEvent(MM_COMPUTECRCFROMFILE_ERR_EID, CFE_EVS_EventType_ERROR,
                                                      "MM_ComputeCRCFromFile error received: RC = 0x%08X File = '%s'",
                                                      (unsigned int)OS_Status, FileName);
                                }
                            }

                        } /* end Valid == true if */

                        if (Valid == true)
                        {
                            CFE_EVS_SendEvent(
                                MM_DMP_MEM_FILE_INF_EID, CFE_EVS_EventType_INFORMATION,
                                "Dump Memory To File Command: Dumped %d bytes from address %p to file '%s'",
                                (int)MM_AppData.HkPacket.Payload.BytesProcessed, (void *)SrcAddress, FileName);
                            /*
                            ** Update last action statistics
                            */
                            MM_AppData.HkPacket.Payload.LastAction = MM_DUMP_TO_FILE;
                            strncpy(MM_AppData.HkPacket.Payload.FileName, FileName, OS_MAX_PATH_LEN);
                            MM_AppData.HkPacket.Payload.MemType        = CmdPtr->Payload.MemType;
                            MM_AppData.HkPacket.Payload.Address        = SrcAddress;
                            MM_AppData.HkPacket.Payload.BytesProcessed = CmdPtr->Payload.NumOfBytes;
                        }

                    } /* end MM_WriteFileHeaders if */

                    /* Close dump file */
                    if ((OS_Status = OS_close(FileHandle)) != OS_SUCCESS)
                    {
                        Valid = false;
                        CFE_EVS_SendEvent(MM_OS_CLOSE_ERR_EID, CFE_EVS_EventType_ERROR,
                                          "OS_close error received: RC = 0x%08X File = '%s'", (unsigned int)OS_Status,
                                          FileName);
                    }

                } /* end OS_OpenCreate if */
                else
                {
                    Valid = false;
                    CFE_EVS_SendEvent(MM_OS_CREAT_ERR_EID, CFE_EVS_EventType_ERROR,
                                      "OS_OpenCreate error received: RC = %d File = '%s'", (int)OS_Status, FileName);
                }

            } /* end MM_VerifyFileLoadDumpParams if */

        } /* end MM_ResolveSymAddr if */
        else
        {
            Valid = false;
            CFE_EVS_SendEvent(MM_SYMNAME_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Symbolic address can't be resolved: Name = '%s'", SrcSymAddress.SymName);
        }

    return Valid;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Dump the requested number of bytes from memory to a file        */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
bool MM_DumpMemToFile(osal_id_t FileHandle, const char *FileName, const MM_LoadDumpFileHeader_t *FileHeader)
{
    bool   ValidDump = false;
    int32  OS_Status;
    uint32 BytesRemaining = FileHeader->NumOfBytes;
    size_t BytesProcessed = 0;
    size_t SegmentSize    = MM_MAX_DUMP_DATA_SEG;
    uint8 *SourcePtr      = (uint8 *)(FileHeader->SymAddress.Offset);
    uint8 *ioBuffer       = (uint8 *)&MM_AppData.DumpBuffer[0];

    while (BytesRemaining != 0)
    {
        if (BytesRemaining < MM_MAX_DUMP_DATA_SEG)
        {
            SegmentSize = BytesRemaining;
        }

        memcpy(ioBuffer, SourcePtr, SegmentSize);

        OS_Status = OS_write(FileHandle, ioBuffer, SegmentSize);
        if (OS_Status == SegmentSize)
        {
            SourcePtr += SegmentSize;
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
            BytesRemaining = 0;
            CFE_EVS_SendEvent(MM_OS_WRITE_EXP_ERR_EID, CFE_EVS_EventType_ERROR,
                              "OS_write error received: RC = %d, Expected = %u, File = '%s'", (int)OS_Status,
                              (unsigned int)SegmentSize, FileName);
        }
    }

    /* Update last action statistics */
    if (BytesProcessed == FileHeader->NumOfBytes)
    {
        ValidDump                          = true;
        MM_AppData.HkPacket.Payload.LastAction     = MM_DUMP_TO_FILE;
        MM_AppData.HkPacket.Payload.MemType        = FileHeader->MemType;
        MM_AppData.HkPacket.Payload.Address        = FileHeader->SymAddress.Offset;
        MM_AppData.HkPacket.Payload.BytesProcessed = BytesProcessed;
        strncpy(MM_AppData.HkPacket.Payload.FileName, FileName, OS_MAX_PATH_LEN);
    }

    return ValidDump;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Write the cFE primary and MM secondary file headers         */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
bool MM_WriteFileHeaders(const char *FileName, osal_id_t FileHandle, CFE_FS_Header_t *CFEHeader,
                         const MM_LoadDumpFileHeader_t *MMHeader)
{
    bool  Valid = true;
    int32 OS_Status;

    /*
    ** Write out the primary cFE file header
    */
    OS_Status = CFE_FS_WriteHeader(FileHandle, CFEHeader);
    if (OS_Status != sizeof(CFE_FS_Header_t))
    {
        /* We either got an error or didn't write as much data as expected */
        Valid = false;
        CFE_EVS_SendEvent(MM_CFE_FS_WRITEHDR_ERR_EID, CFE_EVS_EventType_ERROR,
                          "CFE_FS_WriteHeader error received: RC = %d Expected = %d File = '%s'", (int)OS_Status,
                          (int)sizeof(CFE_FS_Header_t), FileName);

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
            Valid = false;
            CFE_EVS_SendEvent(MM_OS_WRITE_EXP_ERR_EID, CFE_EVS_EventType_ERROR,
                              "OS_write error received: RC = %d Expected = %u File = '%s'", (int)OS_Status,
                              (unsigned int)sizeof(MM_LoadDumpFileHeader_t), FileName);

        } /* end OS_write if */

    } /* end CFE_FS_WriteHeader else */

    return Valid;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Dump memory in event message command                            */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
bool MM_DumpInEventCmd(const CFE_SB_Buffer_t *BufPtr)
{
    bool                       Valid = false;
    const MM_DumpInEventCmd_t *CmdPtr;
    uint32                     i;
    int32                      EventStringTotalLength = 0;
    cpuaddr                    SrcAddress             = 0;
    uint8 *                    BytePtr;
    char                       TempString[MM_DUMPINEVENT_TEMP_CHARS];
    const char                 HeaderString[] = "Memory Dump: ";
    static char                EventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    MM_SymAddr_t               SrcSymAddress;

    /*
    ** Allocate a dump buffer. It's declared this way to ensure it stays
    ** longword aligned since MM_MAX_DUMP_INEVENT_BYTES can be adjusted
    ** by changing the maximum event message string size.
    */
    uint32 DumpBuffer[(MM_MAX_DUMP_INEVENT_BYTES + 3) / 4];

        CmdPtr = ((MM_DumpInEventCmd_t *)BufPtr);

        SrcSymAddress = CmdPtr->Payload.SrcSymAddress;

        /* Resolve the symbolic source address in the command message */
        Valid = MM_ResolveSymAddr(&(SrcSymAddress), &SrcAddress);

        if (Valid == true)
        {
            /* Run necessary checks on command parameters */
            Valid = MM_VerifyLoadDumpParams(SrcAddress, CmdPtr->Payload.MemType, CmdPtr->Payload.NumOfBytes, MM_VERIFY_EVENT);

            if (Valid == true)
            {
                /* Fill a local data buffer with the dump words */
                Valid = MM_FillDumpInEventBuffer(SrcAddress, CmdPtr, (uint8 *)DumpBuffer);

                if (Valid == true)
                {
                    /*
                    ** Prepare event message string header
                    ** 13 characters, not counting NUL terminator
                    */
                    CFE_SB_MessageStringGet(&EventString[EventStringTotalLength], HeaderString, NULL,
                                            sizeof(EventString) - EventStringTotalLength, sizeof(HeaderString));
                    EventStringTotalLength = strlen(EventString);

                    /*
                    ** Build dump data string
                    ** Each byte of data requires 5 characters of string space
                    ** Note this really only allows up to ~15 bytes using default config
                    */
                    BytePtr = (uint8 *)DumpBuffer;
                    for (i = 0; i < CmdPtr->Payload.NumOfBytes; i++)
                    {
                        snprintf(TempString, MM_DUMPINEVENT_TEMP_CHARS, "0x%02X ", *BytePtr);
                        CFE_SB_MessageStringGet(&EventString[EventStringTotalLength], TempString, NULL,
                                                sizeof(EventString) - EventStringTotalLength, sizeof(TempString));
                        EventStringTotalLength = strlen(EventString);
                        BytePtr++;
                    }

                    /*
                    ** Append tail
                    ** This adds up to 33 characters depending on pointer representation including the NUL terminator
                    */
                    snprintf(TempString, MM_DUMPINEVENT_TEMP_CHARS, "from address: %p", (void *)SrcAddress);
                    CFE_SB_MessageStringGet(&EventString[EventStringTotalLength], TempString, NULL,
                                            sizeof(EventString) - EventStringTotalLength, sizeof(TempString));

                    /* Send it out */
                    CFE_EVS_SendEvent(MM_DUMP_INEVENT_INF_EID, CFE_EVS_EventType_INFORMATION, "%s", EventString);

                    /* Update telemetry */
                    MM_AppData.HkPacket.Payload.LastAction     = MM_DUMP_INEVENT;
                    MM_AppData.HkPacket.Payload.MemType        = CmdPtr->Payload.MemType;
                    MM_AppData.HkPacket.Payload.Address        = SrcAddress;
                    MM_AppData.HkPacket.Payload.BytesProcessed = CmdPtr->Payload.NumOfBytes;
                } /* end MM_FillDumpInEventBuffer if */
            }     /* end MM_VerifyFileLoadDumpParams if */
        }         /* end MM_ResolveSymAddr if */
        else
        {
            CFE_EVS_SendEvent(MM_SYMNAME_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Symbolic address can't be resolved: Name = '%s'", SrcSymAddress.SymName);
        }

    return Valid;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Fill a buffer with data to be dumped in an event message string */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
bool MM_FillDumpInEventBuffer(cpuaddr SrcAddress, const MM_DumpInEventCmd_t *CmdPtr, void *DumpBuffer)
{
#if defined(MM_OPT_CODE_MEM8_MEMTYPE) || defined(MM_OPT_CODE_MEM16_MEMTYPE) || defined(MM_OPT_CODE_MEM32_MEMTYPE)
    uint32 i;
#endif
    /* cppcheck-suppress unusedVariable */
    int32 PSP_Status;
    bool  Valid = true;

    /* Initialize buffer */
    memset(DumpBuffer, 0, MM_MAX_DUMP_INEVENT_BYTES);

    switch (CmdPtr->Payload.MemType)
    {
        case MM_RAM:
        case MM_EEPROM:
            memcpy((void *)DumpBuffer, (void *)SrcAddress, CmdPtr->Payload.NumOfBytes);
            break;

#ifdef MM_OPT_CODE_MEM32_MEMTYPE
        case MM_MEM32:
            for (i = 0; i < (CmdPtr->Payload.NumOfBytes / 4); i++)
            {
                PSP_Status = CFE_PSP_MemRead32(SrcAddress, (uint32 *)DumpBuffer);
                if (PSP_Status == CFE_PSP_SUCCESS)
                {
                    SrcAddress += sizeof(uint32);
                    DumpBuffer = (uint8 *)DumpBuffer + sizeof(uint32);
                }
                else
                {
                    /* CFE_PSP_MemRead32 error */
                    Valid = false;
                    CFE_EVS_SendEvent(MM_PSP_READ_ERR_EID, CFE_EVS_EventType_ERROR,
                                      "PSP read memory error: RC=%d, Src=%p, Tgt=%p, Type=MEM32", (int)PSP_Status,
                                      (void *)SrcAddress, (void *)DumpBuffer);
                    /* Stop load dump buffer loop */
                    break;
                }
            }
            break;
#endif /* MM_OPT_CODE_MEM32_MEMTYPE */

#ifdef MM_OPT_CODE_MEM16_MEMTYPE
        case MM_MEM16:
            for (i = 0; i < (CmdPtr->Payload.NumOfBytes / 2); i++)
            {
                PSP_Status = CFE_PSP_MemRead16(SrcAddress, (uint16 *)DumpBuffer);
                if (PSP_Status == CFE_PSP_SUCCESS)
                {
                    SrcAddress += sizeof(uint16);
                    DumpBuffer = (uint8 *)DumpBuffer + sizeof(uint16);
                }
                else
                {
                    /* CFE_PSP_MemRead16 error */
                    Valid = false;
                    CFE_EVS_SendEvent(MM_PSP_READ_ERR_EID, CFE_EVS_EventType_ERROR,
                                      "PSP read memory error: RC=%d, Src=%p, Tgt=%p, Type=MEM16", (int)PSP_Status,
                                      (void *)SrcAddress, (void *)DumpBuffer);
                    /* Stop load dump buffer loop */
                    break;
                }
            }
            break;
#endif /* MM_OPT_CODE_MEM16_MEMTYPE */

#ifdef MM_OPT_CODE_MEM8_MEMTYPE
        case MM_MEM8:
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
                    Valid = false;
                    CFE_EVS_SendEvent(MM_PSP_READ_ERR_EID, CFE_EVS_EventType_ERROR,
                                      "PSP read memory error: RC=%d, Src=%p, Tgt=%p, Type=MEM8", (int)PSP_Status,
                                      (void *)SrcAddress, (void *)DumpBuffer);
                    /* Stop load dump buffer loop */
                    break;
                }
            }
            break;
#endif /* MM_OPT_CODE_MEM8_MEMTYPE */
        default:
            /* This branch will never be executed. CmdPtr->Payload.MemType will always
             * be valid value for this switch statement it is verified via
             * MM_VerifyFileLoadDumpParams */
            Valid = false;
            break;

    } /* end CmdPtr->Payload.MemType switch */

    return Valid;
}
