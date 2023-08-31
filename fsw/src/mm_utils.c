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
 *   Utility functions used for processing CFS memory manager commands
 */

/*************************************************************************
** Includes
*************************************************************************/
#include "mm_app.h"
#include "mm_utils.h"
#include "mm_perfids.h"
#include "mm_msgids.h"
#include "mm_events.h"
#include "mm_dump.h"
#include <string.h>

/*************************************************************************
** Macro Definitions
*************************************************************************/

#define FILE_CRC_BUFFER_SIZE 200 /**< \brief Number of bytes per read when computing file CRC */

/*************************************************************************
** External Data
*************************************************************************/
extern MM_AppData_t MM_AppData;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Reset the local housekeeping variables to default parameters    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void MM_ResetHk(void)
{
    MM_AppData.HkPacket.Payload.LastAction     = MM_NOACTION;
    MM_AppData.HkPacket.Payload.MemType        = MM_NOMEMTYPE;
    MM_AppData.HkPacket.Payload.Address        = MM_CLEAR_ADDR;
    MM_AppData.HkPacket.Payload.DataValue      = MM_CLEAR_PATTERN;
    MM_AppData.HkPacket.Payload.BytesProcessed = 0;
    MM_AppData.HkPacket.Payload.FileName[0]    = MM_CLEAR_FNAME;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Process a load, dump, or fill segment break                     */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void MM_SegmentBreak(void)
{
    /*
    ** Performance Log entry stamp
    */
    CFE_ES_PerfLogEntry(MM_SEGBREAK_PERF_ID);

    /*
    ** Give something else the chance to run
    */
    OS_TaskDelay(MM_PROCESSOR_CYCLE);

    /*
    ** Performance Log exit stamp
    */
    CFE_ES_PerfLogExit(MM_SEGBREAK_PERF_ID);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Verify command packet length                                    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
bool MM_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength)
{
    bool              result       = true;
    size_t            ActualLength = 0;
    CFE_MSG_FcnCode_t CommandCode  = 0;
    CFE_SB_MsgId_t    MessageID    = CFE_SB_INVALID_MSG_ID;

    /*
    ** Verify the message packet length...
    */

    CFE_MSG_GetSize(MsgPtr, &ActualLength);
    if (ExpectedLength != ActualLength)
    {
        CFE_MSG_GetMsgId(MsgPtr, &MessageID);
        CFE_MSG_GetFcnCode(MsgPtr, &CommandCode);

        if (CFE_SB_MsgIdToValue(MessageID) == MM_SEND_HK_MID)
        {
            /*
            ** For a bad HK request, just send the event. We only increment
            ** the error counter for ground commands and not internal messages.
            */
            CFE_EVS_SendEvent(MM_HKREQ_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Invalid HK request msg length: ID = 0x%08lX, CC = %d, Len = %d, Expected = %d",
                              (unsigned long)CFE_SB_MsgIdToValue(MessageID), CommandCode, (int)ActualLength,
                              (int)ExpectedLength);
        }
        else
        {
            CFE_EVS_SendEvent(MM_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Invalid msg length: ID = 0x%08lX, CC = %d, Len = %d, Expected = %d",
                              (unsigned long)CFE_SB_MsgIdToValue(MessageID), CommandCode, (int)ActualLength,
                              (int)ExpectedLength);
        }

        result = false;
    }

    return result;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Verify peek and poke command parameters                         */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
bool MM_VerifyPeekPokeParams(cpuaddr Address, MM_MemType_t MemType, size_t SizeInBits)
{
    bool   Valid = true;
    size_t SizeInBytes;
    int32  OS_Status;

    switch (SizeInBits)
    {
        case MM_BYTE_BIT_WIDTH:
            SizeInBytes = 1;
            break;

        case MM_WORD_BIT_WIDTH:
            SizeInBytes = 2;
            if (MM_Verify16Aligned(Address, SizeInBytes) != true)
            {
                Valid = false;
                CFE_EVS_SendEvent(MM_ALIGN16_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "Data and address not 16 bit aligned: Addr = %p Size = %u", (void *)Address,
                                  (unsigned int)SizeInBytes);
            }
            break;

        case MM_DWORD_BIT_WIDTH:
            SizeInBytes = 4;
            if (MM_Verify32Aligned(Address, SizeInBytes) != true)
            {
                Valid = false;
                CFE_EVS_SendEvent(MM_ALIGN32_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "Data and address not 32 bit aligned: Addr = %p Size = %u", (void *)Address,
                                  (unsigned int)SizeInBytes);
            }
            break;

        default:
            Valid = false;
            CFE_EVS_SendEvent(MM_DATA_SIZE_BITS_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Data size in bits invalid: Data Size = %u", (unsigned int)SizeInBits);
            break;
    }

    /* Do other checks if this one passed */
    if (Valid == true)
    {
        switch (MemType)
        {
            case MM_RAM:
                OS_Status = CFE_PSP_MemValidateRange(Address, SizeInBytes, CFE_PSP_MEM_RAM);

                if (OS_Status != CFE_PSP_SUCCESS)
                {
                    Valid = false;
                    CFE_EVS_SendEvent(MM_OS_MEMVALIDATE_ERR_EID, CFE_EVS_EventType_ERROR,
                                      "CFE_PSP_MemValidateRange error received: RC = 0x%08X Addr = %p Size = %u "
                                      "MemType = MEM_RAM",
                                      (unsigned int)OS_Status, (void *)Address, (unsigned int)SizeInBytes);
                }
                break;

            case MM_EEPROM:
                OS_Status = CFE_PSP_MemValidateRange(Address, SizeInBytes, CFE_PSP_MEM_EEPROM);

                if (OS_Status != CFE_PSP_SUCCESS)
                {
                    Valid = false;
                    CFE_EVS_SendEvent(MM_OS_MEMVALIDATE_ERR_EID, CFE_EVS_EventType_ERROR,
                                      "CFE_PSP_MemValidateRange error received: RC = 0x%08X Addr = %p Size = %u "
                                      "MemType = MEM_EEPROM",
                                      (unsigned int)OS_Status, (void *)Address, (unsigned int)SizeInBytes);
                }
                break;

#ifdef MM_OPT_CODE_MEM32_MEMTYPE
            case MM_MEM32:
                OS_Status = CFE_PSP_MemValidateRange(Address, SizeInBytes, CFE_PSP_MEM_RAM);

                if (OS_Status != CFE_PSP_SUCCESS)
                {
                    Valid = false;
                    CFE_EVS_SendEvent(
                        MM_OS_MEMVALIDATE_ERR_EID, CFE_EVS_EventType_ERROR,
                        "CFE_PSP_MemValidateRange error received: RC = 0x%08X Addr = %p Size = %u MemType = MEM32",
                        (unsigned int)OS_Status, (void *)Address, (unsigned int)SizeInBytes);
                }
                /*
                ** Peeks and Pokes must be 32 bits wide for this memory type
                */
                else if (SizeInBytes != 4)
                {
                    Valid = false;
                    CFE_EVS_SendEvent(MM_DATA_SIZE_BITS_ERR_EID, CFE_EVS_EventType_ERROR,
                                      "Data size in bits invalid: Data Size = %u", (unsigned int)SizeInBits);
                }
                break;
#endif /* MM_OPT_CODE_MEM32_MEMTYPE */

#ifdef MM_OPT_CODE_MEM16_MEMTYPE
            case MM_MEM16:
                OS_Status = CFE_PSP_MemValidateRange(Address, SizeInBytes, CFE_PSP_MEM_RAM);

                if (OS_Status != CFE_PSP_SUCCESS)
                {
                    Valid = false;
                    CFE_EVS_SendEvent(
                        MM_OS_MEMVALIDATE_ERR_EID, CFE_EVS_EventType_ERROR,
                        "CFE_PSP_MemValidateRange error received: RC = 0x%08X Addr = %p Size = %u MemType = MEM16",
                        (unsigned int)OS_Status, (void *)Address, (unsigned int)SizeInBytes);
                }
                /*
                ** Peeks and Pokes must be 16 bits wide for this memory type
                */
                else if (SizeInBytes != 2)
                {
                    Valid = false;
                    CFE_EVS_SendEvent(MM_DATA_SIZE_BITS_ERR_EID, CFE_EVS_EventType_ERROR,
                                      "Data size in bits invalid: Data Size = %u", (unsigned int)SizeInBits);
                }
                break;
#endif /* MM_OPT_CODE_MEM16_MEMTYPE */

#ifdef MM_OPT_CODE_MEM8_MEMTYPE
            case MM_MEM8:
                OS_Status = CFE_PSP_MemValidateRange(Address, SizeInBytes, CFE_PSP_MEM_RAM);

                if (OS_Status != CFE_PSP_SUCCESS)
                {
                    Valid = false;
                    CFE_EVS_SendEvent(
                        MM_OS_MEMVALIDATE_ERR_EID, CFE_EVS_EventType_ERROR,
                        "CFE_PSP_MemValidateRange error received: RC = 0x%08X Addr = %p Size = %u MemType = MEM8",
                        (unsigned int)OS_Status, (void *)Address, (unsigned int)SizeInBytes);
                }
                /*
                ** Peeks and Pokes must be 8 bits wide for this memory type
                */
                else if (SizeInBytes != 1)
                {
                    Valid = false;
                    CFE_EVS_SendEvent(MM_DATA_SIZE_BITS_ERR_EID, CFE_EVS_EventType_ERROR,
                                      "Data size in bits invalid: Data Size = %u", (unsigned int)SizeInBits);
                }
                break;
#endif /* MM_OPT_CODE_MEM8_MEMTYPE */

            default:
                Valid = false;
                CFE_EVS_SendEvent(MM_MEMTYPE_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "Invalid memory type specified: MemType = %d", MemType);
                break;

        } /* end switch */

    } /* end Valid == true if */

    return Valid;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Verify load/dump memory parameters                              */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
bool MM_VerifyLoadDumpParams(cpuaddr Address, MM_MemType_t MemType, size_t SizeInBytes, uint8 VerifyType)
{
    bool         Valid = true;
    CFE_Status_t PSP_Status;
    size_t       MaxSize     = 0;
    uint32       PSP_MemType = 0;
    char         MemTypeStr[MM_MAX_MEM_TYPE_STR_LEN];

    if ((VerifyType != MM_VERIFY_LOAD) && (VerifyType != MM_VERIFY_DUMP) && (VerifyType != MM_VERIFY_EVENT) &&
        (VerifyType != MM_VERIFY_FILL) && (VerifyType != MM_VERIFY_WID))
    {
        Valid = false;
    }

    /* The DumpInEvent and LoadMemWID commands use the same max size for all
     * memory types. Therefore if one of these is the command being verified,
     * the max size can be set here rather than in the switch statement */
    if (VerifyType == MM_VERIFY_EVENT)
    {
        MaxSize = MM_MAX_DUMP_INEVENT_BYTES;
    }
    else if (VerifyType == MM_VERIFY_WID)
    {
        MaxSize = MM_MAX_UNINTERRUPTIBLE_DATA;
    }

    if (Valid)
    {
        /* All memory types and verification types do fundamentally the same set
           of checks.  This switch-case statement sets up the values used to
           perform the checks at the end of the function.  Two memory types
           also require special handling which is performed in the appropriate
           case. */
        switch (MemType)
        {
            /* else clauses are not needed in the VerifyType checks because the
                VerifyType is checked above and if the MaxSize is left unchanged,
                it will force an error when the size is checked as it should */
            case MM_RAM:
                if (VerifyType == MM_VERIFY_LOAD)
                {
                    MaxSize = MM_MAX_LOAD_FILE_DATA_RAM;
                }
                else if (VerifyType == MM_VERIFY_DUMP)
                {
                    MaxSize = MM_MAX_DUMP_FILE_DATA_RAM;
                }
                else if (VerifyType == MM_VERIFY_FILL)
                {
                    MaxSize = MM_MAX_FILL_DATA_RAM;
                }
                PSP_MemType = CFE_PSP_MEM_RAM;
                snprintf(MemTypeStr, MM_MAX_MEM_TYPE_STR_LEN, "%s", "MEM_RAM");
                break;
            case MM_EEPROM:
                if (VerifyType == MM_VERIFY_LOAD)
                {
                    MaxSize = MM_MAX_LOAD_FILE_DATA_EEPROM;
                }
                else if (VerifyType == MM_VERIFY_DUMP)
                {
                    MaxSize = MM_MAX_DUMP_FILE_DATA_EEPROM;
                }
                else if (VerifyType == MM_VERIFY_FILL)
                {
                    MaxSize = MM_MAX_FILL_DATA_EEPROM;
                }
                PSP_MemType = CFE_PSP_MEM_EEPROM;
                snprintf(MemTypeStr, MM_MAX_MEM_TYPE_STR_LEN, "%s", "MEM_EEPROM");
                break;
#ifdef MM_OPT_CODE_MEM32_MEMTYPE
            case MM_MEM32:
                if (VerifyType == MM_VERIFY_LOAD)
                {
                    MaxSize = MM_MAX_LOAD_FILE_DATA_MEM32;
                }
                else if (VerifyType == MM_VERIFY_DUMP)
                {
                    MaxSize = MM_MAX_DUMP_FILE_DATA_MEM32;
                }
                else if (VerifyType == MM_VERIFY_FILL)
                {
                    MaxSize = MM_MAX_FILL_DATA_MEM32;
                }
                PSP_MemType = CFE_PSP_MEM_RAM;
                snprintf(MemTypeStr, MM_MAX_MEM_TYPE_STR_LEN, "%s", "MEM32");
                if (MM_Verify32Aligned(Address, SizeInBytes) != true)
                {
                    Valid = false;
                    CFE_EVS_SendEvent(MM_ALIGN32_ERR_EID, CFE_EVS_EventType_ERROR,
                                      "Data and address not 32 bit aligned: Addr = %p Size = %u", (void *)Address,
                                      (unsigned int)SizeInBytes);
                }
                break;
#endif
#ifdef MM_OPT_CODE_MEM16_MEMTYPE
            case MM_MEM16:
                if (VerifyType == MM_VERIFY_LOAD)
                {
                    MaxSize = MM_MAX_LOAD_FILE_DATA_MEM16;
                }
                else if (VerifyType == MM_VERIFY_DUMP)
                {
                    MaxSize = MM_MAX_DUMP_FILE_DATA_MEM16;
                }
                else if (VerifyType == MM_VERIFY_FILL)
                {
                    MaxSize = MM_MAX_FILL_DATA_MEM16;
                }
                PSP_MemType = CFE_PSP_MEM_RAM;
                snprintf(MemTypeStr, MM_MAX_MEM_TYPE_STR_LEN, "%s", "MEM16");
                if (MM_Verify16Aligned(Address, SizeInBytes) != true)
                {
                    Valid = false;
                    CFE_EVS_SendEvent(MM_ALIGN16_ERR_EID, CFE_EVS_EventType_ERROR,
                                      "Data and address not 16 bit aligned: Addr = %p Size = %u", (void *)Address,
                                      (unsigned int)SizeInBytes);
                }
                break;
#endif
#ifdef MM_OPT_CODE_MEM8_MEMTYPE
            case MM_MEM8:
                if (VerifyType == MM_VERIFY_LOAD)
                {
                    MaxSize = MM_MAX_LOAD_FILE_DATA_MEM8;
                }
                else if (VerifyType == MM_VERIFY_DUMP)
                {
                    MaxSize = MM_MAX_DUMP_FILE_DATA_MEM8;
                }
                else if (VerifyType == MM_VERIFY_FILL)
                {
                    MaxSize = MM_MAX_FILL_DATA_MEM8;
                }
                PSP_MemType = CFE_PSP_MEM_RAM;
                snprintf(MemTypeStr, MM_MAX_MEM_TYPE_STR_LEN, "%s", "MEM8");
                break;
#endif
            default:
                Valid = false;
                CFE_EVS_SendEvent(MM_MEMTYPE_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "Invalid memory type specified: MemType = %d", MemType);

                break;
        } /* end MemType switch */
    }

    if (Valid)
    {
        if ((SizeInBytes == 0) || (SizeInBytes > MaxSize))
        {
            Valid = false;
            CFE_EVS_SendEvent(MM_DATA_SIZE_BYTES_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Data size in bytes invalid or exceeds limits: Data Size = %u",
                              (unsigned int)SizeInBytes);
        }
    }

    if (Valid)
    {
        PSP_Status = CFE_PSP_MemValidateRange(Address, SizeInBytes, PSP_MemType);

        if (PSP_Status != CFE_PSP_SUCCESS)
        {
            Valid = false;
            CFE_EVS_SendEvent(MM_OS_MEMVALIDATE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "CFE_PSP_MemValidateRange error received: RC = 0x%08X Addr = %p Size = %u MemType = %s",
                              (unsigned int)PSP_Status, (void *)Address, (unsigned int)SizeInBytes, MemTypeStr);
        }
    }

    return Valid;
}

/******************************************************************************/

bool MM_Verify32Aligned(cpuaddr Address, size_t Size)
{
    bool IsAligned = false;

    if (Address % sizeof(uint32) == 0 && Size % sizeof(uint32) == 0)
    {
        IsAligned = true;
    }

    return IsAligned;
}

/******************************************************************************/

bool MM_Verify16Aligned(cpuaddr Address, size_t Size)
{
    bool IsAligned = false;

    if (Address % sizeof(uint16) == 0 && Size % sizeof(uint16) == 0)
    {
        IsAligned = true;
    }

    return IsAligned;
}

/******************************************************************************/

bool MM_ResolveSymAddr(MM_SymAddr_t *SymAddr, cpuaddr *ResolvedAddr)
{
    bool  Valid = false;
    int32 OS_Status;

    /*
    ** NUL terminate the very end of the symbol name string array as a
    ** safety measure
    */
    SymAddr->SymName[OS_MAX_SYM_LEN - 1] = '\0';

    /*
    ** If the symbol name string is a nul string
    ** we use the offset as the absolute address
    */
    if (strlen(SymAddr->SymName) == 0)
    {
        *ResolvedAddr = SymAddr->Offset;
        Valid         = true;
    }
    else
    {
        /*
        ** If symbol name is not an empty string look it up
        ** using the OSAL API and add the offset if it succeeds
        */
        OS_Status = OS_SymbolLookup(ResolvedAddr, SymAddr->SymName);
        if (OS_Status == OS_SUCCESS)
        {
            *ResolvedAddr += SymAddr->Offset;
            Valid = true;
        }
        else
            Valid = false;
    }
    return Valid;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Compute the CRC of data in a file                               */
/* Operates from the current location of the file poiner to EOF    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 MM_ComputeCRCFromFile(osal_id_t FileHandle, uint32 *CrcPtr, uint32 TypeCRC)
{
    int32        ByteCntr;
    int32        OS_Status = OS_SUCCESS;
    uint32       TempCrc   = 0;
    static uint8 DataArray[FILE_CRC_BUFFER_SIZE];

    do
    {
        /*
        ** Read in some data
        */
        ByteCntr = OS_read(FileHandle, DataArray, sizeof(DataArray));

        /*
        ** If we didn't hit end of file on the last read...
        */
        if (ByteCntr > 0)
        {
            /*
            ** Calculate the CRC based upon the previous CRC calculation
            */
            TempCrc = CFE_ES_CalculateCRC(DataArray, ByteCntr, TempCrc, TypeCRC);
        }

    } while (ByteCntr > 0);

    /*
    ** Check if we broke out of the loop because of an error return
    ** from the OS_read call
    */
    if (ByteCntr < 0)
    {
        OS_Status = ByteCntr;
    }
    else
    {
        *CrcPtr = TempCrc;
    }

    return OS_Status;
}
