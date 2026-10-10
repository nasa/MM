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

#ifndef DEFAULT_MM_MSGSTRUCT_H
#define DEFAULT_MM_MSGSTRUCT_H

/**
 * \defgroup cfsmmcmdstructs cFS MM Message Structs
 * \{
 */

/* ======== */
/* Includes */
/* ======== */

#include "cfe.h"
#include "mm_msgdefs.h"

/* ======== */
/* Typedefs */
/* ======== */

/**
 *  \brief Noop Command
 *
 *  For command details see #MM_NOOP_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} MM_NoopCmd_t;

/**
 *  \brief Reset Counters Command
 *
 *  For command details see #MM_RESET_COUNTERS_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} MM_ResetCountersCmd_t;

/**
 *  \brief Memory Peek Command
 *
 *  For command details see #MM_PEEK_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
    MM_PeekCmd_Payload_t    Payload;
} MM_PeekCmd_t;

/**
 *  \brief Memory Poke Command
 *
 *  For command details see #MM_POKE_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
    MM_PokeCmd_Payload_t    Payload;
} MM_PokeCmd_t;

/**
 *  \brief Memory Load With Interrupts Disabled Command
 *
 *  For command details see #MM_LOAD_MEM_WID_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t    CommandHeader; /**< \brief Command header */
    MM_LoadMemWIDCmd_Payload_t Payload;
} MM_LoadMemWIDCmd_t;

/**
 *  \brief Dump Memory In Event Message Command
 *
 *  For command details see #MM_DUMP_IN_EVENT_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t     CommandHeader; /**< \brief Command header */
    MM_DumpInEventCmd_Payload_t Payload;
} MM_DumpInEventCmd_t;

/**
 *  \brief Memory Load From File Command
 *
 *  For command details see #MM_LOAD_MEM_FROM_FILE_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t         CommandHeader; /**< \brief Command header */
    MM_LoadMemFromFileCmd_Payload_t Payload;
} MM_LoadMemFromFileCmd_t;

/**
 *  \brief Memory Dump To File Command
 *
 *  For command details see #MM_DUMP_MEM_TO_FILE_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t       CommandHeader; /**< \brief Command header */
    MM_DumpMemToFileCmd_Payload_t Payload;
} MM_DumpMemToFileCmd_t;

/**
 *  \brief Memory Fill Command
 *
 *  For command details see #MM_FILL_MEM_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
    MM_FillMemCmd_Payload_t Payload;
} MM_FillMemCmd_t;

/**
 *  \brief Symbol Table Lookup Command
 *
 *  For command details see #MM_LOOKUP_SYM_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t   CommandHeader; /**< \brief Command header */
    MM_LookupSymCmd_Payload_t Payload;
} MM_LookupSymCmd_t;

/**
 *  \brief Save Symbol Table To File Command
 *
 *  For command details see #MM_SYM_TBL_TO_FILE_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t      CommandHeader; /**< \brief Command header */
    MM_SymTblToFileCmd_Payload_t Payload;
} MM_SymTblToFileCmd_t;

/**
 *  \brief EEPROM Write Enable Command
 *
 *  For command details see #MM_EEPROM_WRITE_ENA_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t        CommandHeader; /**< \brief Command header */
    MM_EepromWriteEnaCmd_Payload_t Payload;
} MM_EepromWriteEnaCmd_t;

/**
 *  \brief EEPROM Write Disable Command
 *
 *  For command details see #MM_EEPROM_WRITE_DIS_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t        CommandHeader; /**< \brief Command header */
    MM_EepromWriteDisCmd_Payload_t Payload;
} MM_EepromWriteDisCmd_t;

/**
 *  \brief Housekeeping Packet Structure
 */
typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader; /**< \brief Telemetry header */
    MM_HkTlm_Payload_t        Payload;
} MM_HkTlm_t;

/**
 * \brief Housekeeping Request Command
 */
typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
} MM_SendHkCmd_t;

/**
 *  \brief Symbol Lookup Packet Structure
 */
typedef struct
{
    CFE_MSG_TelemetryHeader_t    TelemetryHeader; /**< \brief Telemetry header */
    MM_SymLookupPacket_Payload_t Payload;         /**< \brief Symbol Lookup Packet Payload */
} MM_SymLookupPacket_t;

/**
 *  \brief Peek Packet Structure
 */
typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader; /**< \brief Telemetry header */
    MM_PeekPacket_Payload_t   Payload;         /**< \brief Peek Packet Payload */
} MM_PeekPacket_t;

#endif /* DEFAULT_MM_MSGSTRUCT_H */

/**\}*/