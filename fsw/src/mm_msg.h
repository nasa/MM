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
 *   Specification for the CFS Memory Manager command and telemetry
 *   message data types.
 *
 * @note
 *   Constant and enumerated types related to these message structures
 *   are defined in mm_msgdefs.h.
 **/
#ifndef MM_MSG_H
#define MM_MSG_H

/************************************************************************
 * Includes
 ************************************************************************/
#include "mm_platform_cfg.h"
#include "cfe.h"
#include "mm_msgdefs.h"

/************************************************************************
 * Type Definitions
 ************************************************************************/

/**
 *  \brief Memory Types
 */
typedef enum
{
    MM_NOMEMTYPE = 0, /**< \brief Used to indicate that no memtype specified          */
    MM_RAM       = 1, /**< \brief Normal RAM, no special access required              */
    MM_EEPROM    = 2, /**< \brief EEPROM, requires special access for writes          */
    MM_MEM8      = 3, /**< \brief Optional memory type that is only 8-bit read/write  */
    MM_MEM16     = 4, /**< \brief Optional memory type that is only 16-bit read/write */
    MM_MEM32     = 5  /**< \brief Optional memory type that is only 32-bit read/write */
} MM_MemType_t;

/**
 * \defgroup cfsmmcmdstructs CFS Memory Manager Command Structures
 * \{
 */

/**
 *  \brief Symbolic Address Type
 */
typedef struct
{
    cpuaddr Offset;               /**< \brief Optional offset that is used as the
                                     absolute address if the SymName string is NUL */
    char SymName[OS_MAX_SYM_LEN]; /**< \brief Symbol name string  */
} MM_SymAddr_t;

/**
 *  \brief No Arguments Command
 *
 *  For command details see #MM_NOOP_CC, #MM_RESET_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHeader; /**< \brief Command header */
} MM_NoArgsCmd_t;

/**
 *  \brief Memory Peek Command
 *
 *  For command details see #MM_PEEK_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHeader; /**< \brief Command header */

    uint8        DataSize;      /**< \brief Size of the data to be read     */
    uint8        Padding[3];    /**< \brief Structure padding               */
    MM_MemType_t MemType;       /**< \brief Memory type to peek data from   */
    MM_SymAddr_t SrcSymAddress; /**< \brief Symbolic source peek address    */
} MM_PeekCmd_t;

/**
 *  \brief Memory Poke Command
 *
 *  For command details see #MM_POKE_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHeader; /**< \brief Command header */

    uint8        DataSize;       /**< \brief Size of the data to be written     */
    uint8        Padding1[3];    /**< \brief Structure padding                  */
    MM_MemType_t MemType;        /**< \brief Memory type to poke data to        */
    uint32       Data;           /**< \brief Data to be written                 */
    uint8        Padding2[4];    /**< \brief Structure padding                  */
    MM_SymAddr_t DestSymAddress; /**< \brief Symbolic destination poke address  */
} MM_PokeCmd_t;

/**
 *  \brief Memory Load With Interrupts Disabled Command
 *
 *  For command details see #MM_LOAD_MEM_WID_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHeader; /**< \brief Command header */

    uint8        NumOfBytes;                             /**< \brief Number of bytes to be loaded       */
    uint8        Padding[3];                             /**< \brief Structure padding                  */
    uint32       Crc;                                    /**< \brief Data check value                   */
    MM_SymAddr_t DestSymAddress;                         /**< \brief Symbolic destination load address  */
    uint8        DataArray[MM_MAX_UNINTERRUPTIBLE_DATA]; /**< \brief Data to be loaded           */
} MM_LoadMemWIDCmd_t;

/**
 *  \brief Dump Memory In Event Message Command
 *
 *  For command details see #MM_DUMP_IN_EVENT_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHeader; /**< \brief Command header */

    MM_MemType_t MemType;       /**< \brief Memory dump type             */
    uint8        NumOfBytes;    /**< \brief Number of bytes to be dumped */
    uint8        Padding[3];    /**< \brief Structure padding            */
    MM_SymAddr_t SrcSymAddress; /**< \brief Symbolic source address      */
} MM_DumpInEventCmd_t;

/**
 *  \brief Memory Load From File Command
 *
 *  For command details see #MM_LOAD_MEM_FROM_FILE_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHeader; /**< \brief Command header */

    char FileName[OS_MAX_PATH_LEN]; /**< \brief Name of memory load file */
} MM_LoadMemFromFileCmd_t;

/**
 *  \brief Memory Dump To File Command
 *
 *  For command details see #MM_DUMP_MEM_TO_FILE_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHeader; /**< \brief Command header */

    MM_MemType_t MemType;                   /**< \brief Memory dump type */
    uint32       NumOfBytes;                /**< \brief Number of bytes to be dumped */
    MM_SymAddr_t SrcSymAddress;             /**< \brief Symbol plus optional offset  */
    char         FileName[OS_MAX_PATH_LEN]; /**< \brief Name of memory dump file */
} MM_DumpMemToFileCmd_t;

/**
 *  \brief Memory Fill Command
 *
 *  For command details see #MM_FILL_MEM_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHeader; /**< \brief Command header */

    MM_MemType_t MemType;        /**< \brief Memory type                  */
    uint32       NumOfBytes;     /**< \brief Number of bytes to fill      */
    uint32       FillPattern;    /**< \brief Fill pattern to use          */
    uint8        Padding[4];     /**< \brief Structure padding            */
    MM_SymAddr_t DestSymAddress; /**< \brief Symbol plus optional offset  */
} MM_FillMemCmd_t;

/**
 *  \brief Symbol Table Lookup Command
 *
 *  For command details see #MM_LOOKUP_SYM_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHeader; /**< \brief Command header */

    char SymName[OS_MAX_SYM_LEN]; /**< \brief Symbol name string           */
} MM_LookupSymCmd_t;

/**
 *  \brief Save Symbol Table To File Command
 *
 *  For command details see #MM_SYMTBL_TO_FILE_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHeader; /**< \brief Command header */

    char FileName[OS_MAX_PATH_LEN]; /**< \brief Name of symbol dump file */
} MM_SymTblToFileCmd_t;

/**
 *  \brief EEPROM Write Enable Command
 *
 *  For command details see #MM_ENABLE_EEPROM_WRITE_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHeader; /**< \brief Command header */

    uint32 Bank; /**< \brief EEPROM bank number to write-enable */
} MM_EepromWriteEnaCmd_t;

/**
 *  \brief EEPROM Write Disable Command
 *
 *  For command details see #MM_DISABLE_EEPROM_WRITE_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHeader; /**< \brief Command header */

    uint32 Bank; /**< \brief EEPROM bank number to write-disable */
} MM_EepromWriteDisCmd_t;

/**\}*/

/**
 * \defgroup cfsmmtlm CFS Memory Manager Telemetry
 * \{
 */

/**
 *  \brief Housekeeping Packet Structure
 */
typedef struct
{
    CFE_MSG_TelemetryHeader_t TlmHeader; /**< \brief Telemetry header */

    uint8        CmdCounter;                /**< \brief MM Application Command Counter */
    uint8        ErrCounter;                /**< \brief MM Application Command Error Counter */
    uint8        LastAction;                /**< \brief Last command action executed */
    uint8        Padding;                   /**< \brief Last command action executed */
    MM_MemType_t MemType;                   /**< \brief Memory type for last command */
    cpuaddr      Address;                   /**< \brief Fully resolved address used for last command */
    uint32       DataValue;                 /**< \brief Last command data (fill pattern or peek/poke value) */
    uint32       BytesProcessed;            /**< \brief Bytes processed for last command */
    char         FileName[OS_MAX_PATH_LEN]; /**< \brief Name of the data file used for last command, where applicable */
} MM_HkPacket_t;

/**\}*/

#endif
