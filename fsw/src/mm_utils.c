/*************************************************************************
** File: mm_utils.c 
**
**   Copyright © 2007-2014 United States Government as represented by the
**   Administrator of the National Aeronautics and Space Administration.
**   All Other Rights Reserved.
**
**   This software was created at NASA's Goddard Space Flight Center.
**   This software is governed by the NASA Open Source Agreement and may be
**   used, distributed and modified only pursuant to the terms of that
**   agreement.
**
** Purpose:
**   Utility functions used for processing CFS memory manager commands
**
*************************************************************************/

/*************************************************************************
** Includes
*************************************************************************/
#include "mm_app.h"
#include "mm_utils.h"
#include "mm_perfids.h"
#include "mm_msgids.h"
#include "mm_events.h"
#include "cfs_utils.h"
#include "mm_dump.h"
#include <string.h>

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

    MM_AppData.HkPacket.LastAction     = MM_NOACTION;
    MM_AppData.HkPacket.MemType        = MM_NOMEMTYPE;
    MM_AppData.HkPacket.Address        = MM_CLEAR_ADDR;
    MM_AppData.HkPacket.DataValue      = MM_CLEAR_PATTERN;
    MM_AppData.HkPacket.BytesProcessed = 0;
    MM_AppData.HkPacket.FileName[0]    = MM_CLEAR_FNAME;

    return;

} /* end MM_ResetHk */

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

    return;

} /* End of MM_SegmentBreak */

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

        if (MessageID == MM_SEND_HK_MID)
        {
            /*
            ** For a bad HK request, just send the event. We only increment
            ** the error counter for ground commands and not internal messages.
            */
            CFE_EVS_SendEvent(MM_HKREQ_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Invalid HK request msg length: ID = 0x%08X, CC = %d, Len = %d, Expected = %d", MessageID,
                              CommandCode, (int)ActualLength, (int)ExpectedLength);
        }
        else
        {
            CFE_EVS_SendEvent(MM_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Invalid msg length: ID = 0x%08X, CC = %d, Len = %d, Expected = %d", MessageID,
                              CommandCode, (int)ActualLength, (int)ExpectedLength);
        }

        result = false;
    }

    return (result);

} /* End of MM_VerifyCmdLength */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Verify peek and poke command parameters                         */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
bool MM_VerifyPeekPokeParams(cpuaddr Address, uint8 MemType, uint8 SizeInBits)
{
    bool  Valid = true;
    uint8 SizeInBytes;
    int32 OS_Status;

    switch (SizeInBits)
    {
        case MM_BYTE_BIT_WIDTH:
            SizeInBytes = 1;
            break;

        case MM_WORD_BIT_WIDTH:
            SizeInBytes = 2;
            if (CFS_Verify16Aligned(Address, SizeInBytes) != true)
            {
                Valid = false;
                CFE_EVS_SendEvent(MM_ALIGN16_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "Data and address not 16 bit aligned: Addr = 0x%08X Size = %d", (unsigned int)Address,
                                  SizeInBytes);
            }
            break;

        case MM_DWORD_BIT_WIDTH:
            SizeInBytes = 4;
            if (CFS_Verify32Aligned(Address, SizeInBytes) != true)
            {
                Valid = false;
                CFE_EVS_SendEvent(MM_ALIGN32_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "Data and address not 32 bit aligned: Addr = 0x%08X Size = %d", (unsigned int)Address,
                                  SizeInBytes);
            }
            break;

        default:
            Valid = false;
            CFE_EVS_SendEvent(MM_DATA_SIZE_BITS_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Data size in bits invalid: Data Size = %d", SizeInBits);
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
                                      "CFE_PSP_MemValidateRange error received: RC = 0x%08X Addr = 0x%08X Size = %d "
                                      "MemType = MEM_RAM",
                                      (unsigned int)OS_Status, (unsigned int)Address, SizeInBytes);
                }
                break;

            case MM_EEPROM:
                OS_Status = CFE_PSP_MemValidateRange(Address, SizeInBytes, CFE_PSP_MEM_EEPROM);

                if (OS_Status != CFE_PSP_SUCCESS)
                {
                    Valid = false;
                    CFE_EVS_SendEvent(MM_OS_MEMVALIDATE_ERR_EID, CFE_EVS_EventType_ERROR,
                                      "CFE_PSP_MemValidateRange error received: RC = 0x%08X Addr = 0x%08X Size = %d "
                                      "MemType = MEM_EEPROM",
                                      (unsigned int)OS_Status, (unsigned int)Address, SizeInBytes);
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
                        "CFE_PSP_MemValidateRange error received: RC = 0x%08X Addr = 0x%08X Size = %d MemType = MEM32",
                        (unsigned int)OS_Status, (unsigned int)Address, SizeInBytes);
                }
                /*
                ** Peeks and Pokes must be 32 bits wide for this memory type
                */
                else if (SizeInBytes != 4)
                {
                    Valid = false;
                    CFE_EVS_SendEvent(MM_DATA_SIZE_BITS_ERR_EID, CFE_EVS_EventType_ERROR,
                                      "Data size in bits invalid: Data Size = %d", SizeInBits);
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
                        "CFE_PSP_MemValidateRange error received: RC = 0x%08X Addr = 0x%08X Size = %d MemType = MEM16",
                        (unsigned int)OS_Status, (unsigned int)Address, SizeInBytes);
                }
                /*
                ** Peeks and Pokes must be 16 bits wide for this memory type
                */
                else if (SizeInBytes != 2)
                {
                    Valid = false;
                    CFE_EVS_SendEvent(MM_DATA_SIZE_BITS_ERR_EID, CFE_EVS_EventType_ERROR,
                                      "Data size in bits invalid: Data Size = %d", SizeInBits);
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
                        "CFE_PSP_MemValidateRange error received: RC = 0x%08X Addr = 0x%08X Size = %d MemType = MEM8",
                        (unsigned int)OS_Status, (unsigned int)Address, SizeInBytes);
                }
                /*
                ** Peeks and Pokes must be 8 bits wide for this memory type
                */
                else if (SizeInBytes != 1)
                {
                    Valid = false;
                    CFE_EVS_SendEvent(MM_DATA_SIZE_BITS_ERR_EID, CFE_EVS_EventType_ERROR,
                                      "Data size in bits invalid: Data Size = %d", SizeInBits);
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

    return (Valid);

} /* end MM_VerifyPeekPokeParams */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Verify load/dump memory parameters                              */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
bool MM_VerifyLoadDumpParams(cpuaddr Address, uint8 MemType, uint32 SizeInBytes, uint8 VerifyType)
{
    bool   Valid = true;
    int32  PSP_Status;
    uint32 MaxSize     = 0;
    uint32 PSP_MemType = 0;
    char   MemTypeStr[MM_MAX_MEM_TYPE_STR_LEN];

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
                if (CFS_Verify32Aligned(Address, SizeInBytes) != true)
                {
                    Valid = false;
                    CFE_EVS_SendEvent(MM_ALIGN32_ERR_EID, CFE_EVS_EventType_ERROR,
                                      "Data and address not 32 bit aligned: Addr = 0x%08X Size = %d",
                                      (unsigned int)Address, (int)SizeInBytes);
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
                if (CFS_Verify16Aligned(Address, SizeInBytes) != true)
                {
                    Valid = false;
                    CFE_EVS_SendEvent(MM_ALIGN16_ERR_EID, CFE_EVS_EventType_ERROR,
                                      "Data and address not 16 bit aligned: Addr = 0x%08X Size = %d",
                                      (unsigned int)Address, (int)SizeInBytes);
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
                              "Data size in bytes invalid or exceeds limits: Data Size = %d", (int)SizeInBytes);
        }
    }

    if (Valid)
    {
        PSP_Status = CFE_PSP_MemValidateRange(Address, SizeInBytes, PSP_MemType);

        if (PSP_Status != CFE_PSP_SUCCESS)
        {
            Valid = false;
            CFE_EVS_SendEvent(
                MM_OS_MEMVALIDATE_ERR_EID, CFE_EVS_EventType_ERROR,
                "CFE_PSP_MemValidateRange error received: RC = 0x%08X Addr = 0x%08X Size = %d MemType = %s",
                (unsigned int)PSP_Status, (unsigned int)Address, (int)SizeInBytes, MemTypeStr);
        }
    }

    return (Valid);

} /* end MM_VerifyFileLoadDumpParams */

/************************/
/*  End of File Comment */
/************************/
