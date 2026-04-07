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
 *   Default values of the cFS Memory Manager (MM) command/function codes
 *
 * @note
 *   This file should be strictly limited to the command/function code (CC)
 *   values. The values from this file are used in mm_fcncodes.h
 */

#ifndef DEFAULT_MM_FCNCODE_VALUES_H
#define DEFAULT_MM_FCNCODE_VALUES_H

/* ====== */
/* Macros */
/* ====== */

#define MM_CCVAL(x) MM_FunctionCode_##x

/* ======== */
/* Typedefs */
/* ======== */

enum MM_FunctionCode
{
    MM_FunctionCode_NOOP                 = 0,
    MM_FunctionCode_RESET                = 1,
    MM_FunctionCode_PEEK                 = 2,
    MM_FunctionCode_POKE                 = 3,
    MM_FunctionCode_LOAD_MEM_WID         = 4,
    MM_FunctionCode_LOAD_MEM_FROM_FILE   = 5,
    MM_FunctionCode_DUMP_MEM_TO_FILE     = 6,
    MM_FunctionCode_DUMP_IN_EVENT        = 7,
    MM_FunctionCode_FILL_MEM             = 8,
    MM_FunctionCode_LOOKUP_SYM           = 9,
    MM_FunctionCode_SYMTBL_TO_FILE       = 10,
    MM_FunctionCode_ENABLE_EEPROM_WRITE  = 11,
    MM_FunctionCode_DISABLE_EEPROM_WRITE = 12,
};

#endif /* DEFAULT_MM_FCNCODE_VALUES_H */