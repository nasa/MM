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

#ifndef DEFAULT_MM_EXTERN_TYPEDEFS_H
#define DEFAULT_MM_EXTERN_TYPEDEFS_H

/* ======== */
/* Includes */
/* ======== */

#include "cfe_es_extern_typedefs.h"
#include "cfe_mission_cfg.h"
#include "common_types.h"

/* ======== */
/* Typedefs */
/* ======== */

typedef CFE_ES_MemAddress_t MM_MemAddress_t;
typedef uint32              MM_MemSize_t;

/**
 * @brief Last Action Identifiers
 */
enum
{
    MM_LastAction_NOACTION        = 0,  /**< \brief Used to clear out HK action variable */
    MM_LastAction_PEEK            = 1,  /**< \brief Peek action */
    MM_LastAction_POKE            = 2,  /**< \brief Poke action */
    MM_LastAction_LOAD_FROM_FILE  = 3,  /**< \brief Load from file action */
    MM_LastAction_LOAD_WID        = 4,  /**< \brief Load with interrupts disabled action */
    MM_LastAction_DUMP_TO_FILE    = 5,  /**< \brief Dump to file action */
    MM_LastAction_DUMP_INEVENT    = 6,  /**< \brief Dump in event action */
    MM_LastAction_FILL            = 7,  /**< \brief Fill action */
    MM_LastAction_SYM_LOOKUP      = 8,  /**< \brief Symbol lookup action */
    MM_LastAction_SYMTBL_SAVE     = 9,  /**< \brief Dump symbol table to file action */
    MM_LastAction_EEPROMWRITE_ENA = 10, /**< \brief EEPROM write enable action */
    MM_LastAction_EEPROMWRITE_DIS = 11, /**< \brief EEPROM write disable action */
    MM_LastAction_NOOP            = 12, /**< \brief No-op action */
    MM_LastAction_RESET           = 13  /**< \brief Reset counters action */
};

typedef uint8 MM_LastAction_Enum_t;

/**
 *  \brief Memory Types
 */
typedef enum
{
    MM_MemType_NOMEMTYPE = 0, /**< \brief Used to indicate that no memtype specified          */
    MM_MemType_RAM       = 1, /**< \brief Normal RAM, no special access required */
    MM_MemType_EEPROM    = 2, /**< \brief EEPROM, requires special access for writes          */
    MM_MemType_MEM8      = 3, /**< \brief Optional memory type that is only 8-bit read/write  */
    MM_MemType_MEM16     = 4, /**< \brief Optional memory type that is only 16-bit read/write */
    MM_MemType_MEM32     = 5  /**< \brief Optional memory type that is only 32-bit read/write */
} MM_MemType_Enum_t;

/**
 *  \brief Symbolic Address Type
 */
typedef struct
{
    MM_MemAddress_t Offset;                            /**< \brief Optional offset that is used as the
                                                           absolute address if the SymName string is NUL */
    char            SymName[CFE_MISSION_MAX_PATH_LEN]; /**< \brief Symbol name string  */
} MM_SymAddr_t;

#endif /* DEFAULT_MM_EXTERN_TYPEDEFS_H */