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

#ifndef DEFAULT_MM_MSGDEFS_H
#define DEFAULT_MM_MSGDEFS_H

/**
 * \defgroup cfsmmtlm cFS MM Telemetry
 * \{
 */

/* ======== */
/* Includes */
/* ======== */

#include "cfe_mission_cfg.h"
#include "common_types.h"
#include "mm_extern_typedefs.h"
#include "mm_mission_cfg.h"

/* ======== */
/* Typedefs */
/* ======== */

/**
 *  \brief Memory Peek Command Payload
 */
typedef struct
{
    MM_MemSize_t      DataSize;      /**< \brief Size of the data to be read     */
    MM_MemType_Enum_t MemType;       /**< \brief Memory type to peek data from   */
    MM_SymAddr_t      SrcSymAddress; /**< \brief Symbolic source peek address    */
} MM_PeekCmd_Payload_t;

/**
 *  \brief Memory Poke Command Payload
 */
typedef struct
{
    MM_MemSize_t      DataSize;       /**< \brief Size of the data to be written     */
    MM_MemType_Enum_t MemType;        /**< \brief Memory type to poke data to        */
    uint32            Data;           /**< \brief Data to be written                 */
    uint8             Padding2[4];    /**< \brief Structure padding                  */
    MM_SymAddr_t      DestSymAddress; /**< \brief Symbolic destination poke address  */
} MM_PokeCmd_Payload_t;

/**
 *  \brief Memory Load With Interrupts Disabled Command Payload
 */
typedef struct
{
    uint8        NumOfBytes;                                       /**< \brief Number of bytes to be loaded       */
    uint8        Padding[3];                                       /**< \brief Structure padding                  */
    uint32       Crc;                                              /**< \brief Data check value                   */
    MM_SymAddr_t DestSymAddress;                                   /**< \brief Symbolic destination load address  */
    uint8        DataArray[MM_INTERFACE_MAX_UNINTERRUPTIBLE_DATA]; /**< \brief Data to be
                                                                      loaded           */
} MM_LoadMemWIDCmd_Payload_t;

/**
 *  \brief Dump Memory In Event Message Command Payload
 */
typedef struct
{
    MM_MemType_Enum_t MemType;       /**< \brief Memory dump type             */
    uint8             NumOfBytes;    /**< \brief Number of bytes to be dumped */
    uint8             Padding[3];    /**< \brief Structure padding            */
    MM_SymAddr_t      SrcSymAddress; /**< \brief Symbolic source address      */
} MM_DumpInEventCmd_Payload_t;

/**
 *  \brief Memory Load From File Command Payload
 */
typedef struct
{
    char FileName[CFE_MISSION_MAX_PATH_LEN]; /**< \brief Name of memory load file
                                              */
} MM_LoadMemFromFileCmd_Payload_t;

/**
 *  \brief Memory Dump To File Command Payload
 */
typedef struct
{
    MM_MemType_Enum_t MemType;                            /**< \brief Memory dump type */
    MM_MemSize_t      NumOfBytes;                         /**< \brief Number of bytes to be dumped */
    MM_SymAddr_t      SrcSymAddress;                      /**< \brief Symbol plus optional offset  */
    char              FileName[CFE_MISSION_MAX_PATH_LEN]; /**< \brief Name of memory dump file
                                                           */
} MM_DumpMemToFileCmd_Payload_t;

/**
 *  \brief Memory Fill Command Payload
 */
typedef struct
{
    MM_MemType_Enum_t MemType;        /**< \brief Memory type                  */
    MM_MemSize_t      NumOfBytes;     /**< \brief Number of bytes to fill      */
    uint32            FillPattern;    /**< \brief Fill pattern to use          */
    uint8             Padding[4];     /**< \brief Structure padding            */
    MM_SymAddr_t      DestSymAddress; /**< \brief Symbol plus optional offset  */
} MM_FillMemCmd_Payload_t;

/**
 *  \brief Symbol Table Lookup Command Payload
 */
typedef struct
{
    char SymName[CFE_MISSION_MAX_PATH_LEN]; /**< \brief Symbol name string */
} MM_LookupSymCmd_Payload_t;

/**
 *  \brief Save Symbol Table To File Command Payload
 */
typedef struct
{
    char FileName[CFE_MISSION_MAX_PATH_LEN]; /**< \brief Name of symbol dump file
                                              */
} MM_SymTblToFileCmd_Payload_t;

/**
 *  \brief EEPROM Write Enable Command Payload
 */
typedef struct
{
    uint32 Bank; /**< \brief EEPROM bank number to write-enable */
} MM_EepromWriteEnaCmd_Payload_t;

/**
 *  \brief EEPROM Write Disable Command Payload
 */
typedef struct
{
    uint32 Bank; /**< \brief EEPROM bank number to write-disable */
} MM_EepromWriteDisCmd_Payload_t;

/**
 *  \brief Housekeeping Packet Payload Structure
 */
typedef struct
{
    uint8             CmdCounter;                         /**< \brief MM Application Command Counter */
    uint8             ErrCounter;                         /**< \brief MM Application Command Error Counter */
    uint8             LastAction;                         /**< \brief Last command action executed */
    uint8             Padding;                            /**< \brief Last command action executed */
    MM_MemType_Enum_t MemType;                            /**< \brief Memory type for last command */
    MM_MemAddress_t   Address;                            /**< \brief Fully resolved address used for last command */
    uint32            DataValue;                          /**< \brief Last command data (fill pattern or peek/poke
                                                             value) */
    MM_MemSize_t      BytesProcessed;                     /**< \brief Bytes processed for last command */
    char              FileName[CFE_MISSION_MAX_PATH_LEN]; /**< \brief Name of the data file
                                                             used for last command, where
                                                             applicable */
} MM_HkTlm_Payload_t;

#endif /* DEFAULT_MM_MSGDEFS_H */

/**\}*/