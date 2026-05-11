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

#include "mm_cmds.h"
#include "mm_app.h"
#include "mm_dump.h"
#include "mm_eventids.h"
#include "mm_extern_typedefs.h"
#include "mm_filedefs.h"
#include "mm_interface_cfg.h"
#include "mm_internal_cfg.h"
#include "mm_load.h"
#include "mm_msg.h"
#include "mm_utils.h"
#include "mm_version.h"

#include "cfe.h"

#ifdef MM_INTERNAL_OPT_CODE_MEM32_MEMTYPE
#include "mm_mem32.h"
#endif /* MM_INTERNAL_OPT_CODE_MEM32_MEMTYPE */

#ifdef MM_INTERNAL_OPT_CODE_MEM16_MEMTYPE
#include "mm_mem16.h"
#endif /* MM_INTERNAL_OPT_CODE_MEM16_MEMTYPE */

#ifdef MM_INTERNAL_OPT_CODE_MEM8_MEMTYPE
#include "mm_mem8.h"
#endif /* MM_INTERNAL_OPT_CODE_MEM8_MEMTYPE */

/* ==================== */
/* Function Definitions */
/* ==================== */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Housekeeping request                                            */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CFE_Status_t MM_SendHkCmd(const MM_SendHkCmd_t *Msg)
{
    /*
    ** Send housekeeping telemetry packet
    */
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(MM_AppData.HkTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(MM_AppData.HkTlm.TelemetryHeader), true);

    /*
    ** This command does not affect the command execution counter
    */

    /* Nothing atypical needs to be done so return success */
    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Noop command                                                    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CFE_Status_t MM_NoopCmd(const MM_NoopCmd_t *Msg)
{
    MM_AppData.HkTlm.Payload.LastAction = MM_LastAction_NOOP;
    MM_AppData.HkTlm.Payload.CommandCounter++;

    CFE_EVS_SendEvent(MM_NOOP_INF_EID,
                      CFE_EVS_EventType_INFORMATION,
                      "No-op command. Version %d.%d.%d.%d",
                      MM_MAJOR_VERSION,
                      MM_MINOR_VERSION,
                      MM_REVISION,
                      MM_INTERNAL_MISSION_REV);

    /* Nothing atypical needs to be done so return success */
    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Reset counters command                                          */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CFE_Status_t MM_ResetCountersCmd(const MM_ResetCountersCmd_t *Msg)
{
    MM_AppData.HkTlm.Payload.LastAction          = MM_LastAction_RESET;
    MM_AppData.HkTlm.Payload.CommandCounter      = 0;
    MM_AppData.HkTlm.Payload.CommandErrorCounter = 0;

    CFE_EVS_SendEvent(MM_RESET_INF_EID, CFE_EVS_EventType_INFORMATION, "Reset counters command received");

    /* Nothing atypical needs to be done so return success */
    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Lookup symbol name command                                      */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CFE_Status_t MM_LookupSymCmd(const MM_LookupSymCmd_t *Msg)
{
    int32   OS_Status;
    cpuaddr ResolvedAddr = 0;
    char    SymName[CFE_MISSION_MAX_PATH_LEN];

    /* Make sure string is null terminated before attempting to process it */
    CFE_SB_MessageStringGet(SymName, Msg->Payload.SymName, NULL, sizeof(SymName), sizeof(Msg->Payload.SymName));

    /*
    ** Check if the symbol name string is a nul string
    */
    if (OS_strnlen(SymName, CFE_MISSION_MAX_PATH_LEN) == 0)
    {
        MM_AppData.HkTlm.Payload.CommandErrorCounter++;
        CFE_EVS_SendEvent(MM_SYMNAME_NUL_ERR_EID,
                          CFE_EVS_EventType_ERROR,
                          "NUL (empty) string specified as symbol name");
    }
    else
    {
        /*
        ** If symbol name is not an empty string look it up using the OSAL API
        */
        OS_Status = OS_SymbolLookup(&ResolvedAddr, SymName);
        if (OS_Status == OS_SUCCESS)
        {
            /* Update telemetry */
            MM_AppData.HkTlm.Payload.LastAction = MM_LastAction_SYM_LOOKUP;
            MM_AppData.HkTlm.Payload.Address    = CFE_ES_MEMADDRESS_C(ResolvedAddr);

            MM_AppData.HkTlm.Payload.CommandCounter++;
            CFE_EVS_SendEvent(MM_SYM_LOOKUP_INF_EID,
                              CFE_EVS_EventType_INFORMATION,
                              "Symbol Lookup Command: Name = '%s' Addr = %p",
                              SymName,
                              (void *)ResolvedAddr);
        }
        else
        {
            MM_AppData.HkTlm.Payload.CommandErrorCounter++;
            CFE_EVS_SendEvent(MM_SYMNAME_ERR_EID,
                              CFE_EVS_EventType_ERROR,
                              "Symbolic address can't be resolved: Name = '%s'",
                              SymName);
        }

    } /* end OS_strnlen(Msg->Payload.SymName, CFE_MISSION_MAX_PATH_LEN) == 0 else
       */

    /* Nothing atypical needs to be done so return success */
    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Dump symbol table to file command                               */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CFE_Status_t MM_SymTblToFileCmd(const MM_SymTblToFileCmd_t *Msg)
{
    char  FileName[CFE_MISSION_MAX_PATH_LEN];
    int32 OS_Status;

    /* Make sure string is null terminated before attempting to process it */
    CFE_SB_MessageStringGet(FileName, Msg->Payload.FileName, NULL, sizeof(FileName), sizeof(Msg->Payload.FileName));

    /*
    ** Check if the filename string is a nul string
    */
    if (OS_strnlen(FileName, CFE_MISSION_MAX_PATH_LEN) == 0)
    {
        MM_AppData.HkTlm.Payload.CommandErrorCounter++;
        CFE_EVS_SendEvent(MM_SYMFILENAME_NUL_ERR_EID,
                          CFE_EVS_EventType_ERROR,
                          "NUL (empty) string specified as symbol dump file name");
    }
    else
    {
        OS_Status = OS_SymbolTableDump(FileName, MM_INTERNAL_MAX_DUMP_FILE_DATA_SYMTBL);
        if (OS_Status == OS_SUCCESS)
        {
            /* Update telemetry */
            MM_AppData.HkTlm.Payload.LastAction = MM_LastAction_SYMTBL_SAVE;
            snprintf(MM_AppData.HkTlm.Payload.FileName, CFE_MISSION_MAX_PATH_LEN, "%s", FileName);

            MM_AppData.HkTlm.Payload.CommandCounter++;
            CFE_EVS_SendEvent(MM_SYMTBL_TO_FILE_INF_EID,
                              CFE_EVS_EventType_INFORMATION,
                              "Symbol Table Dump to File Started: Name = '%s'",
                              FileName);
        }
        else
        {
            MM_AppData.HkTlm.Payload.CommandErrorCounter++;
            CFE_EVS_SendEvent(MM_SYMTBL_TO_FILE_FAIL_ERR_EID,
                              CFE_EVS_EventType_ERROR,
                              "Error dumping symbol table, OS_Status= 0x%X, File='%s'",
                              (unsigned int)OS_Status,
                              FileName);
        }
    } /* end OS_strnlen(FileName, CFE_MISSION_MAX_PATH_LEN) == 0 else */

    /* Nothing atypical needs to be done so return success */
    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* EEPROM write-enable command                                     */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CFE_Status_t MM_EepromWriteEnaCmd(const MM_EepromWriteEnaCmd_t *Msg)
{
    CFE_Status_t cFE_Status;

    /*
    ** Call the cFE to write-enable the requested bank
    */
    cFE_Status = CFE_PSP_EepromWriteEnable(Msg->Payload.Bank);
    if (cFE_Status == CFE_PSP_SUCCESS)
    {
        /* Update telemetry */
        MM_AppData.HkTlm.Payload.LastAction = MM_LastAction_EEPROMWRITE_ENA;
        MM_AppData.HkTlm.Payload.MemType    = MM_MemType_EEPROM;

        MM_AppData.HkTlm.Payload.CommandCounter++;
        CFE_EVS_SendEvent(MM_EEPROM_WRITE_ENA_INF_EID,
                          CFE_EVS_EventType_INFORMATION,
                          "EEPROM bank %d write enabled, cFE_Status= 0x%X",
                          (int)Msg->Payload.Bank,
                          (unsigned int)cFE_Status);
    }
    else
    {
        MM_AppData.HkTlm.Payload.CommandErrorCounter++;
        CFE_EVS_SendEvent(MM_EEPROM_WRITE_ENA_ERR_EID,
                          CFE_EVS_EventType_ERROR,
                          "Error requesting EEPROM bank %d write enable, cFE_Status= 0x%X",
                          (int)Msg->Payload.Bank,
                          (unsigned int)cFE_Status);
    }

    /* Nothing atypical needs to be done so return success */
    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* EEPROM write-disable command                                    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CFE_Status_t MM_EepromWriteDisCmd(const MM_EepromWriteDisCmd_t *Msg)
{
    CFE_Status_t cFE_Status;

    /*
    ** Call the cFE to write-enable the requested bank
    */
    cFE_Status = CFE_PSP_EepromWriteDisable(Msg->Payload.Bank);
    if (cFE_Status == CFE_PSP_SUCCESS)
    {
        /* Update telemetry */
        MM_AppData.HkTlm.Payload.LastAction = MM_LastAction_EEPROMWRITE_DIS;
        MM_AppData.HkTlm.Payload.MemType    = MM_MemType_EEPROM;

        MM_AppData.HkTlm.Payload.CommandCounter++;
        CFE_EVS_SendEvent(MM_EEPROM_WRITE_DIS_INF_EID,
                          CFE_EVS_EventType_INFORMATION,
                          "EEPROM bank %d write disabled, cFE_Status= 0x%X",
                          (int)Msg->Payload.Bank,
                          (unsigned int)cFE_Status);
    }
    else
    {
        MM_AppData.HkTlm.Payload.CommandErrorCounter++;
        CFE_EVS_SendEvent(MM_EEPROM_WRITE_DIS_ERR_EID,
                          CFE_EVS_EventType_ERROR,
                          "Error requesting EEPROM bank %d write disable, cFE_Status= 0x%X",
                          (int)Msg->Payload.Bank,
                          (unsigned int)cFE_Status);
    }

    /* Nothing atypical needs to be done so return success */
    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Memory poke ground command                                      */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CFE_Status_t MM_PokeCmd(const MM_PokeCmd_t *Msg)
{
    cpuaddr      DestAddress = 0;
    MM_SymAddr_t DestSymAddress;
    int32        Status;

    DestSymAddress = Msg->Payload.DestSymAddress;

    /* Resolve the symbolic address in command message */
    Status = MM_ResolveSymAddr(&(DestSymAddress), &DestAddress);

    if (Status == OS_SUCCESS)
    {
        /* Run necessary checks on command parameters */
        Status = MM_VerifyPeekPokeParams(DestAddress, Msg->Payload.MemType, Msg->Payload.DataSize);

        /* Check the specified memory type and call the appropriate routine */
        if (Status == OS_SUCCESS)
        {
            /* Check if we need special EEPROM processing */
            if (Msg->Payload.MemType == MM_MemType_EEPROM)
            {
                Status = MM_PokeEeprom(Msg, DestAddress);
            }
            else
            {
                /*
                ** We can use this routine for all other memory types
                *  (including the optional ones)
                */
                Status = MM_PokeMem(Msg, DestAddress);
            }

            if (Status == CFE_PSP_SUCCESS)
            {
                MM_AppData.HkTlm.Payload.CommandCounter++;
            }
            else
            {
                MM_AppData.HkTlm.Payload.CommandErrorCounter++;
            }
        }
        else
        {
            MM_AppData.HkTlm.Payload.CommandErrorCounter++;
        } /* end MM_VerifyPeekPokeParams if */
    } /* end MM_ResolveSymAddr */
    else
    {
        MM_AppData.HkTlm.Payload.CommandErrorCounter++;
        CFE_EVS_SendEvent(MM_SYMNAME_ERR_EID,
                          CFE_EVS_EventType_ERROR,
                          "Symbolic address can't be resolved: Name = '%s'",
                          DestSymAddress.SymName);
    }

    /* Nothing atypical needs to be done so return success */
    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Load memory with interrupts disabled                            */
/* Only valid for RAM addresses                                    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CFE_Status_t MM_LoadMemWIDCmd(const MM_LoadMemWIDCmd_t *Msg)
{
    uint32       ComputedCRC;
    cpuaddr      DestAddress = 0;
    MM_SymAddr_t DestSymAddress;

    DestSymAddress = Msg->Payload.DestSymAddress;

    /* Resolve the symbolic address in command message */
    if (MM_ResolveSymAddr(&(DestSymAddress), &DestAddress) == OS_SUCCESS)
    {
        /*
        ** Run some necessary checks on command parameters
        ** NOTE: A load with interrupts disabled command is only valid for RAM
        *addresses
        */
        if (MM_VerifyLoadDumpParams(DestAddress, MM_MemType_RAM, Msg->Payload.NumOfBytes, MM_VERIFY_WID)
            == CFE_PSP_SUCCESS)
        {
            /* Verify data integrity check value */
            ComputedCRC =
                CFE_ES_CalculateCRC(Msg->Payload.DataArray, Msg->Payload.NumOfBytes, 0, MM_INTERNAL_LOAD_WID_CRC_TYPE);
            /*
            ** If the CRC matches do the load
            */
            if (ComputedCRC == Msg->Payload.Crc)
            {
                /* Load input data to input memory address */
                memcpy((void *)DestAddress, Msg->Payload.DataArray, Msg->Payload.NumOfBytes);

                MM_AppData.HkTlm.Payload.CommandCounter++;
                CFE_EVS_SendEvent(MM_LOAD_WID_INF_EID,
                                  CFE_EVS_EventType_INFORMATION,
                                  "Load Memory WID Command: Wrote %d bytes to address: %p",
                                  (int)Msg->Payload.NumOfBytes,
                                  (void *)DestAddress);

                /* Update last action statistics */
                MM_AppData.HkTlm.Payload.LastAction     = MM_LastAction_LOAD_WID;
                MM_AppData.HkTlm.Payload.Address        = CFE_ES_MEMADDRESS_C(DestAddress);
                MM_AppData.HkTlm.Payload.MemType        = MM_MemType_RAM;
                MM_AppData.HkTlm.Payload.BytesProcessed = Msg->Payload.NumOfBytes;
            }
            else
            {
                MM_AppData.HkTlm.Payload.CommandErrorCounter++;
                CFE_EVS_SendEvent(MM_LOAD_WID_CRC_ERR_EID,
                                  CFE_EVS_EventType_ERROR,
                                  "Interrupts Disabled Load CRC failure: Expected = "
                                  "0x%X Calculated = 0x%X",
                                  (unsigned int)Msg->Payload.Crc,
                                  (unsigned int)ComputedCRC);
            }
        }
        else
        {
            MM_AppData.HkTlm.Payload.CommandErrorCounter++;
        } /* end MM_VerifyLoadWIDParams */
    } /* end MM_ResolveSymAddr if */
    else
    {
        MM_AppData.HkTlm.Payload.CommandErrorCounter++;
        CFE_EVS_SendEvent(MM_SYMNAME_ERR_EID,
                          CFE_EVS_EventType_ERROR,
                          "Symbolic address can't be resolved: Name = '%s'",
                          DestSymAddress.SymName);
    }

    /* Nothing atypical needs to be done so return success */
    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Load memory from a file command                                 */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CFE_Status_t MM_LoadMemFromFileCmd(const MM_LoadMemFromFileCmd_t *Msg)
{
    int32                   Status;
    osal_id_t               FileHandle  = OS_OBJECT_ID_UNDEFINED;
    cpuaddr                 DestAddress = 0;
    char                    FileName[CFE_MISSION_MAX_PATH_LEN];
    CFE_FS_Header_t         CFEFileHeader;
    MM_LoadDumpFileHeader_t MMFileHeader;
    uint32                  ComputedCRC;
    int32                   LSeekSize;

    memset(&MMFileHeader, 0, sizeof(MMFileHeader));

    /* Make sure string is null terminated before attempting to process it */
    CFE_SB_MessageStringGet(FileName, Msg->Payload.FileName, NULL, sizeof(FileName), sizeof(Msg->Payload.FileName));

    /* Open load file for reading */
    Status = OS_OpenCreate(&FileHandle, FileName, OS_FILE_FLAG_NONE, OS_READ_ONLY);
    if (Status == OS_SUCCESS)
    {
        /* Read in the file headers */
        Status = MM_ReadFileHeaders(FileName, FileHandle, &CFEFileHeader, &MMFileHeader);
        if (Status == OS_SUCCESS)
        {
            /* Verify the file size is correct */
            Status = MM_VerifyLoadFileSize(FileName, &MMFileHeader);
            if (Status == OS_SUCCESS)
            {
                /* Verify data integrity check value */
                Status = MM_ComputeCRCFromFile(FileHandle, &ComputedCRC, MM_INTERNAL_LOAD_FILE_CRC_TYPE);
                if (Status == OS_SUCCESS)
                {
                    /*
                    ** Reset the file pointer to the start of the load data, need to do
                    *this
                    ** because MM_ComputeCRCFromFile reads to the end of file
                    */
                    LSeekSize =
                        OS_lseek(FileHandle, (sizeof(CFE_FS_Header_t) + sizeof(MM_LoadDumpFileHeader_t)), OS_SEEK_SET);
                    if (LSeekSize != (sizeof(CFE_FS_Header_t) + sizeof(MM_LoadDumpFileHeader_t)))
                    {
                        Status = OS_ERROR;
                    }

                    /* Check the computed CRC against the file header CRC */
                    if ((ComputedCRC == MMFileHeader.Crc) && (Status == OS_SUCCESS))
                    {
                        /* Resolve symbolic address in file header */
                        Status = MM_ResolveSymAddr(&(MMFileHeader.SymAddress), &DestAddress);

                        if (Status == OS_SUCCESS)
                        {
                            /* Run necessary checks on command parameters */
                            Status = MM_VerifyLoadDumpParams(DestAddress,
                                                             MMFileHeader.MemType,
                                                             MMFileHeader.NumOfBytes,
                                                             MM_VERIFY_LOAD);
                            if (Status == CFE_PSP_SUCCESS)
                            {
                                /* Call the load routine for the specified memory type */
                                switch (MMFileHeader.MemType)
                                {
                                    case MM_MemType_RAM:
                                    case MM_MemType_EEPROM:
                                        Status = MM_LoadMemFromFile(FileHandle, FileName, &MMFileHeader, DestAddress);
                                        break;

#ifdef MM_INTERNAL_OPT_CODE_MEM32_MEMTYPE
                                    case MM_MemType_MEM32:
                                        Status = MM_LoadMem32FromFile(FileHandle, FileName, &MMFileHeader, DestAddress);
                                        break;
#endif /* MM_INTERNAL_OPT_CODE_MEM32_MEMTYPE */

#ifdef MM_INTERNAL_OPT_CODE_MEM16_MEMTYPE
                                    case MM_MemType_MEM16:
                                        Status = MM_LoadMem16FromFile(FileHandle, FileName, &MMFileHeader, DestAddress);
                                        break;
#endif /* MM_INTERNAL_OPT_CODE_MEM16_MEMTYPE */

#ifdef MM_INTERNAL_OPT_CODE_MEM8_MEMTYPE
                                    case MM_MemType_MEM8:
                                        Status = MM_LoadMem8FromFile(FileHandle, FileName, &MMFileHeader, DestAddress);
                                        break;
#endif /* MM_INTERNAL_OPT_CODE_MEM8_MEMTYPE */

                                    /*
                                    ** We don't need a default case, a bad MemType will get caught
                                    ** in the MM_VerifyFileLoadParams function and we won't get here
                                    */
                                    default:
                                        Status = OS_ERROR;
                                        break;
                                }

                                if (Status == OS_SUCCESS)
                                {
                                    MM_AppData.HkTlm.Payload.CommandCounter++;
                                    CFE_EVS_SendEvent(MM_LD_MEM_FILE_INF_EID,
                                                      CFE_EVS_EventType_INFORMATION,
                                                      "Load Memory From File Command: Loaded %d bytes to "
                                                      "address %p from file '%s'",
                                                      (int)MM_AppData.HkTlm.Payload.BytesProcessed,
                                                      (void *)DestAddress,
                                                      FileName);
                                }
                                else
                                {
                                    MM_AppData.HkTlm.Payload.CommandErrorCounter++;
                                }
                            } /* end MM_VerifyFileLoadParams if */
                            else
                            {
                                MM_AppData.HkTlm.Payload.CommandErrorCounter++;
                                CFE_EVS_SendEvent(MM_FILE_LOAD_PARAMS_ERR_EID,
                                                  CFE_EVS_EventType_ERROR,
                                                  "Load file failed parameters check: File = '%s'",
                                                  FileName);
                            }
                        } /* end MM_ResolveSymAddr if */
                        else
                        {
                            MM_AppData.HkTlm.Payload.CommandErrorCounter++;
                            CFE_EVS_SendEvent(MM_SYMNAME_ERR_EID,
                                              CFE_EVS_EventType_ERROR,
                                              "Symbolic address can't be resolved: Name = '%s'",
                                              MMFileHeader.SymAddress.SymName);
                        }

                    } /* end ComputedCRC == MMFileHeader.Crc if */
                    else
                    {
                        MM_AppData.HkTlm.Payload.CommandErrorCounter++;
                        CFE_EVS_SendEvent(MM_LOAD_FILE_CRC_ERR_EID,
                                          CFE_EVS_EventType_ERROR,
                                          "Load file CRC failure: Expected = 0x%X "
                                          "Calculated = 0x%X File = '%s'",
                                          (unsigned int)MMFileHeader.Crc,
                                          (unsigned int)ComputedCRC,
                                          FileName);
                    }

                } /* end MM_ComputeCRCFromFile if */
                else
                {
                    MM_AppData.HkTlm.Payload.CommandErrorCounter++;
                    CFE_EVS_SendEvent(MM_COMPUTECRCFROMFILE_ERR_EID,
                                      CFE_EVS_EventType_ERROR,
                                      "MM_ComputeCRCFromFile error received: RC = 0x%08X File = '%s'",
                                      (unsigned int)Status,
                                      FileName);
                }
            }
            else
            {
                MM_AppData.HkTlm.Payload.CommandErrorCounter++;
            } /* end MM_VerifyLoadFileSize */
        }
        else
        {
            MM_AppData.HkTlm.Payload.CommandErrorCounter++;
        } /* end MM_ReadFileHeaders if */

        /* Close the load file for all cases after the open call succeeds */
        Status = OS_close(FileHandle);
        if (Status != OS_SUCCESS)
        {
            MM_AppData.HkTlm.Payload.CommandErrorCounter++;
            CFE_EVS_SendEvent(MM_OS_CLOSE_ERR_EID,
                              CFE_EVS_EventType_ERROR,
                              "OS_close error received: RC = 0x%08X File = '%s'",
                              (unsigned int)Status,
                              FileName);
        }

    } /* end OS_OpenCreate if */
    else
    {
        MM_AppData.HkTlm.Payload.CommandErrorCounter++;
        CFE_EVS_SendEvent(MM_OS_OPEN_ERR_EID,
                          CFE_EVS_EventType_ERROR,
                          "OS_OpenCreate error received: RC = %d File = '%s'",
                          (int)Status,
                          Msg->Payload.FileName);
    }

    /* Nothing atypical needs to be done so return success */
    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Fill memory command                                             */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CFE_Status_t MM_FillMemCmd(const MM_FillMemCmd_t *Msg)
{
    cpuaddr      DestAddress    = 0;
    CFE_Status_t Status         = CFE_SUCCESS;
    MM_SymAddr_t DestSymAddress = Msg->Payload.DestSymAddress;

    /* Resolve symbolic address */
    if (MM_ResolveSymAddr(&(DestSymAddress), &DestAddress) == OS_SUCCESS)
    {
        /* Run necessary checks on command parameters */
        if (MM_VerifyLoadDumpParams(DestAddress, Msg->Payload.MemType, Msg->Payload.NumOfBytes, MM_VERIFY_FILL)
            == CFE_SUCCESS)
        {
            switch (Msg->Payload.MemType)
            {
                case MM_MemType_RAM:
                case MM_MemType_EEPROM:
                    MM_FillMem(DestAddress, Msg);
                    break;

#ifdef MM_INTERNAL_OPT_CODE_MEM32_MEMTYPE
                case MM_MemType_MEM32:
                    Status = MM_FillMem32(DestAddress, Msg);
                    break;
#endif

#ifdef MM_INTERNAL_OPT_CODE_MEM16_MEMTYPE
                case MM_MemType_MEM16:
                    Status = MM_FillMem16(DestAddress, Msg);
                    break;
#endif

#ifdef MM_INTERNAL_OPT_CODE_MEM8_MEMTYPE
                case MM_MemType_MEM8:
                    Status = MM_FillMem8(DestAddress, Msg);
                    break;
#endif

                    /*
                    ** We don't need a default case, a bad MemType will get caught
                    ** in the MM_VerifyLoadDumpParams function and we won't get here
                    */
                default:
                    Status = CFE_PSP_ERROR;
                    break;
            }

            if (Status == CFE_PSP_SUCCESS)
            {
                if (MM_AppData.HkTlm.Payload.LastAction == MM_LastAction_FILL)
                {
                    MM_AppData.HkTlm.Payload.CommandCounter++;
                    CFE_EVS_SendEvent(MM_FILL_INF_EID,
                                      CFE_EVS_EventType_INFORMATION,
                                      "Fill Memory Command: Filled %d bytes at address: "
                                      "%p with pattern: 0x%08X",
                                      (int)MM_AppData.HkTlm.Payload.BytesProcessed,
                                      (void *)DestAddress,
                                      (unsigned int)MM_AppData.HkTlm.Payload.DataValue);
                }
                else
                {
                    MM_AppData.HkTlm.Payload.CommandErrorCounter++;
                }
            }
            else
            {
                MM_AppData.HkTlm.Payload.CommandCounter++;
            }
        }
        else
        {
            MM_AppData.HkTlm.Payload.CommandErrorCounter++;
        }
    }
    else
    {
        MM_AppData.HkTlm.Payload.CommandErrorCounter++;
        CFE_EVS_SendEvent(MM_SYMNAME_ERR_EID,
                          CFE_EVS_EventType_ERROR,
                          "Symbolic address can't be resolved: Name = '%s'",
                          DestSymAddress.SymName);
    }

    /* Nothing atypical needs to be done so return success */
    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Memory peek command                                             */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CFE_Status_t MM_PeekCmd(const MM_PeekCmd_t *Msg)
{
    cpuaddr      SrcAddress = 0;
    MM_SymAddr_t SrcSymAddress;
    CFE_Status_t Status = OS_SUCCESS;

    SrcSymAddress = Msg->Payload.SrcSymAddress;

    /* Resolve the symbolic address in command message */
    Status = MM_ResolveSymAddr(&(SrcSymAddress), &SrcAddress);

    if (Status == OS_SUCCESS)
    {
        /* Run necessary checks on command parameters */
        Status = MM_VerifyPeekPokeParams(SrcAddress, Msg->Payload.MemType, Msg->Payload.DataSize);

        /* Check the specified memory type and call the appropriate routine */
        if (Status == OS_SUCCESS)
        {
            /*
            ** We use this single peek routine for all memory types
            ** (including the optional ones)
            */
            Status = MM_PeekMem(Msg, SrcAddress);
            if (Status == CFE_PSP_SUCCESS)
            {
                MM_AppData.HkTlm.Payload.CommandCounter++;
            }
            else
            {
                MM_AppData.HkTlm.Payload.CommandErrorCounter++;
            }
        }
        else
        {
            MM_AppData.HkTlm.Payload.CommandErrorCounter++;
        }
    } /* end MM_ResolveSymAddr if */
    else
    {
        MM_AppData.HkTlm.Payload.CommandErrorCounter++;
        CFE_EVS_SendEvent(MM_SYMNAME_ERR_EID,
                          CFE_EVS_EventType_ERROR,
                          "Symbolic address can't be resolved: Name = '%s'",
                          Msg->Payload.SrcSymAddress.SymName);
    }

    /* Nothing atypical needs to be done so return success */
    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Dump memory to file command                                      */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CFE_Status_t MM_DumpMemToFileCmd(const MM_DumpMemToFileCmd_t *Msg)
{
    int32                   Status;
    osal_id_t               FileHandle = OS_OBJECT_ID_UNDEFINED;
    cpuaddr                 SrcAddress = 0;
    char                    FileName[CFE_MISSION_MAX_PATH_LEN];
    MM_SymAddr_t            SrcSymAddress;
    CFE_FS_Header_t         CFEFileHeader;
    MM_LoadDumpFileHeader_t MMFileHeader;

    SrcSymAddress = Msg->Payload.SrcSymAddress;

    /* Make sure strings are null terminated before attempting to process them */
    CFE_SB_MessageStringGet(FileName, Msg->Payload.FileName, NULL, sizeof(FileName), sizeof(Msg->Payload.FileName));

    /* Resolve the symbolic address in command message */
    Status = MM_ResolveSymAddr(&(SrcSymAddress), &SrcAddress);

    if (Status == OS_SUCCESS)
    {
        /* Run necessary checks on command parameters */
        Status = MM_VerifyLoadDumpParams(SrcAddress, Msg->Payload.MemType, Msg->Payload.NumOfBytes, MM_VERIFY_DUMP);

        if (Status == OS_SUCCESS)
        {
            /*
            ** Initialize the cFE primary file header structure
            */
            CFE_FS_InitHeader(&CFEFileHeader, MM_INTERNAL_CFE_HDR_DESCRIPTION, MM_INTERNAL_CFE_HDR_SUBTYPE);

            /*
            ** Initialize the MM secondary file header structure
            */
            memset(&MMFileHeader, 0, sizeof(MMFileHeader));
            MMFileHeader.SymAddress.SymName[0] = MM_INTERNAL_CLEAR_SYMNAME;

            /*
            ** Copy command data to file secondary header
            */
            MMFileHeader.SymAddress.Offset = CFE_ES_MEMADDRESS_C(SrcAddress);
            MMFileHeader.MemType           = Msg->Payload.MemType;
            MMFileHeader.NumOfBytes        = Msg->Payload.NumOfBytes;

            /*
            ** Create and open dump file
            */
            Status = OS_OpenCreate(&FileHandle, FileName, OS_FILE_FLAG_CREATE | OS_FILE_FLAG_TRUNCATE, OS_READ_WRITE);
            if (Status == OS_SUCCESS)
            {
                /* Write the file headers */
                Status = MM_WriteFileHeaders(FileName, FileHandle, &CFEFileHeader, &MMFileHeader);
                if (Status == OS_SUCCESS)
                {
                    switch (MMFileHeader.MemType)
                    {
                        case MM_MemType_RAM:
                        case MM_MemType_EEPROM:
                            Status = MM_DumpMemToFile(FileHandle, FileName, &MMFileHeader);
                            break;

#ifdef MM_INTERNAL_OPT_CODE_MEM32_MEMTYPE
                        case MM_MemType_MEM32:
                            Status = MM_DumpMem32ToFile(FileHandle, FileName, &MMFileHeader);
                            break;
#endif /* MM_INTERNAL_OPT_CODE_MEM32_MEMTYPE */

#ifdef MM_INTERNAL_OPT_CODE_MEM16_MEMTYPE
                        case MM_MemType_MEM16:
                            Status = MM_DumpMem16ToFile(FileHandle, FileName, &MMFileHeader);
                            break;
#endif /* MM_INTERNAL_OPT_CODE_MEM16_MEMTYPE */

#ifdef MM_INTERNAL_OPT_CODE_MEM8_MEMTYPE
                        case MM_MemType_MEM8:
                            Status = MM_DumpMem8ToFile(FileHandle, FileName, &MMFileHeader);
                            break;
#endif /* MM_INTERNAL_OPT_CODE_MEM8_MEMTYPE */
                        default:
                            /* This branch will never be executed. MMFileHeader.MemType will
                             * always be valid value for this switch statement it is verified
                             * via MM_VerifyFileLoadDumpParams */
                            Status = CFE_PSP_ERROR;
                            break;
                    }

                    if (Status == CFE_PSP_SUCCESS)
                    {
                        /*
                        ** Compute CRC of dumped data
                        */
                        Status = OS_lseek(FileHandle,
                                          (sizeof(CFE_FS_Header_t) + sizeof(MM_LoadDumpFileHeader_t)),
                                          OS_SEEK_SET);
                        if (Status != (sizeof(CFE_FS_Header_t) + sizeof(MM_LoadDumpFileHeader_t)))
                        {
                            MM_AppData.HkTlm.Payload.CommandErrorCounter++;
                        }
                        else
                        {
                            Status =
                                MM_ComputeCRCFromFile(FileHandle, &MMFileHeader.Crc, MM_INTERNAL_DUMP_FILE_CRC_TYPE);
                            if (Status == OS_SUCCESS)
                            {
                                /*
                                ** Rewrite the file headers. The subfunctions will take care of
                                *moving
                                ** the file pointer to the beginning of the file so we don't
                                *need to do it
                                ** here.
                                */
                                Status = MM_WriteFileHeaders(FileName, FileHandle, &CFEFileHeader, &MMFileHeader);
                                if (Status == OS_SUCCESS)
                                {
                                    /*
                                    ** Update last action statistics
                                    */
                                    MM_AppData.HkTlm.Payload.LastAction     = MM_LastAction_DUMP_TO_FILE;
                                    MM_AppData.HkTlm.Payload.MemType        = Msg->Payload.MemType;
                                    MM_AppData.HkTlm.Payload.Address        = CFE_ES_MEMADDRESS_C(SrcAddress);
                                    MM_AppData.HkTlm.Payload.BytesProcessed = Msg->Payload.NumOfBytes;
                                    snprintf(MM_AppData.HkTlm.Payload.FileName,
                                             CFE_MISSION_MAX_PATH_LEN,
                                             "%s",
                                             FileName);

                                    MM_AppData.HkTlm.Payload.CommandCounter++;
                                    CFE_EVS_SendEvent(MM_DMP_MEM_FILE_INF_EID,
                                                      CFE_EVS_EventType_INFORMATION,
                                                      "Dump Memory To File Command: Dumped %d bytes from "
                                                      "address %p to file '%s'",
                                                      (int)MM_AppData.HkTlm.Payload.BytesProcessed,
                                                      (void *)SrcAddress,
                                                      FileName);
                                }
                                else
                                {
                                    MM_AppData.HkTlm.Payload.CommandErrorCounter++;
                                }

                            } /* end MM_ComputeCRCFromFile if */
                            else
                            {
                                MM_AppData.HkTlm.Payload.CommandErrorCounter++;
                                CFE_EVS_SendEvent(MM_COMPUTECRCFROMFILE_ERR_EID,
                                                  CFE_EVS_EventType_ERROR,
                                                  "MM_ComputeCRCFromFile error received: RC = "
                                                  "0x%08X File = '%s'",
                                                  (unsigned int)Status,
                                                  FileName);
                            }
                        }
                    }
                    else
                    {
                        MM_AppData.HkTlm.Payload.CommandErrorCounter++;
                    }
                }
                else
                {
                    MM_AppData.HkTlm.Payload.CommandErrorCounter++;
                } /* end MM_WriteFileHeaders if */

                /* Close dump file */
                if ((Status = OS_close(FileHandle)) != OS_SUCCESS)
                {
                    MM_AppData.HkTlm.Payload.CommandErrorCounter++;
                    CFE_EVS_SendEvent(MM_OS_CLOSE_ERR_EID,
                                      CFE_EVS_EventType_ERROR,
                                      "OS_close error received: RC = 0x%08X File = '%s'",
                                      (unsigned int)Status,
                                      FileName);
                }

            } /* end OS_OpenCreate if */
            else
            {
                MM_AppData.HkTlm.Payload.CommandErrorCounter++;
                CFE_EVS_SendEvent(MM_OS_CREAT_ERR_EID,
                                  CFE_EVS_EventType_ERROR,
                                  "OS_OpenCreate error received: RC = %d File = '%s'",
                                  (int)Status,
                                  FileName);
            }
        }
        else
        {
            MM_AppData.HkTlm.Payload.CommandErrorCounter++;
        } /* end MM_VerifyFileLoadDumpParams if */
    } /* end MM_ResolveSymAddr if */
    else
    {
        MM_AppData.HkTlm.Payload.CommandErrorCounter++;
        CFE_EVS_SendEvent(MM_SYMNAME_ERR_EID,
                          CFE_EVS_EventType_ERROR,
                          "Symbolic address can't be resolved: Name = '%s'",
                          SrcSymAddress.SymName);
    }

    /* Nothing atypical needs to be done so return success */
    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Dump memory in event message command                            */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CFE_Status_t MM_DumpInEventCmd(const MM_DumpInEventCmd_t *Msg)
{
    uint32       i;
    int32        EventStringTotalLength = 0;
    cpuaddr      SrcAddress             = 0;
    uint8       *BytePtr;
    char         TempString[MM_INTERNAL_DUMPINEVENT_TEMP_CHARS];
    const char   HeaderString[] = "Memory Dump: ";
    static char  EventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    MM_SymAddr_t SrcSymAddress;
    int32        Status;

    /*
    ** Allocate a dump buffer. It's declared this way to ensure it stays
    ** longword aligned since MM_INTERNAL_MAX_DUMP_INEVENT_BYTES can be adjusted
    ** by changing the maximum event message string size.
    */
    uint32 DumpBuffer[(MM_INTERNAL_MAX_DUMP_INEVENT_BYTES + 3) / 4];

    SrcSymAddress = Msg->Payload.SrcSymAddress;

    /* Resolve the symbolic source address in the command message */
    Status = MM_ResolveSymAddr(&(SrcSymAddress), &SrcAddress);

    if (Status == OS_SUCCESS)
    {
        /* Run necessary checks on command parameters */
        Status = MM_VerifyLoadDumpParams(SrcAddress, Msg->Payload.MemType, Msg->Payload.NumOfBytes, MM_VERIFY_EVENT);

        if (Status == CFE_PSP_SUCCESS)
        {
            /* Fill a local data buffer with the dump words */
            Status = MM_FillDumpInEventBuffer(SrcAddress, Msg, (uint8 *)DumpBuffer);

            if (Status == CFE_PSP_SUCCESS)
            {
                /*
                ** Prepare event message string header
                ** 13 characters, not counting NUL terminator
                */
                CFE_SB_MessageStringGet(&EventString[EventStringTotalLength],
                                        HeaderString,
                                        NULL,
                                        sizeof(EventString) - EventStringTotalLength,
                                        sizeof(HeaderString));
                EventStringTotalLength = OS_strnlen(EventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

                /*
                ** Build dump data string
                ** Each byte of data requires 5 characters of string space
                ** Note this really only allows up to ~15 bytes using default config
                */
                BytePtr = (uint8 *)DumpBuffer;
                for (i = 0; i < Msg->Payload.NumOfBytes; i++)
                {
                    /* SAD: No need to check snprintf return; CFE_SB_MessageStringGet()
                     * handles safe concatenation and prevents overflow */
                    snprintf(TempString, MM_INTERNAL_DUMPINEVENT_TEMP_CHARS, "0x%02X ", *BytePtr);
                    CFE_SB_MessageStringGet(&EventString[EventStringTotalLength],
                                            TempString,
                                            NULL,
                                            sizeof(EventString) - EventStringTotalLength,
                                            sizeof(TempString));
                    EventStringTotalLength = OS_strnlen(EventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);
                    BytePtr++;
                }

                /*
                ** Append tail
                ** This adds up to 33 characters depending on pointer representation
                *including the NUL terminator
                */
                /* SAD: No need to check snprintf return; CFE_SB_MessageStringGet()
                 * handles safe concatenation and prevents overflow */
                snprintf(TempString, MM_INTERNAL_DUMPINEVENT_TEMP_CHARS, "from address: %p", (void *)SrcAddress);
                CFE_SB_MessageStringGet(&EventString[EventStringTotalLength],
                                        TempString,
                                        NULL,
                                        sizeof(EventString) - EventStringTotalLength,
                                        sizeof(TempString));

                /* Send it out */
                CFE_EVS_SendEvent(MM_DUMP_INEVENT_INF_EID, CFE_EVS_EventType_INFORMATION, "%s", EventString);

                /* Update telemetry */
                MM_AppData.HkTlm.Payload.LastAction     = MM_LastAction_DUMP_INEVENT;
                MM_AppData.HkTlm.Payload.MemType        = Msg->Payload.MemType;
                MM_AppData.HkTlm.Payload.Address        = CFE_ES_MEMADDRESS_C(SrcAddress);
                MM_AppData.HkTlm.Payload.BytesProcessed = Msg->Payload.NumOfBytes;
                MM_AppData.HkTlm.Payload.CommandCounter++;
            }
            else
            {
                MM_AppData.HkTlm.Payload.CommandErrorCounter++;
            } /* end MM_FillDumpInEventBuffer if */
        }
        else
        {
            MM_AppData.HkTlm.Payload.CommandErrorCounter++;
        } /* end MM_VerifyFileLoadDumpParams if */
    } /* end MM_ResolveSymAddr if */
    else
    {
        MM_AppData.HkTlm.Payload.CommandErrorCounter++;
        CFE_EVS_SendEvent(MM_SYMNAME_ERR_EID,
                          CFE_EVS_EventType_ERROR,
                          "Symbolic address can't be resolved: Name = '%s'",
                          SrcSymAddress.SymName);
    }

    /* Nothing atypical needs to be done so return success */
    return CFE_SUCCESS;
}
