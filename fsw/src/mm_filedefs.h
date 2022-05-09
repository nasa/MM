/************************************************************************
 * NASA Docket No. GSC-18,923-1, and identified as “Core Flight
 * System (cFS) Memory Manager Application version 2.5.0”
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
 *   Specification for the CFS Memory Manager file structures.
 */
#ifndef MM_FILEDEFS_H
#define MM_FILEDEFS_H

/*************************************************************************
 * Includes
 *************************************************************************/
#include "mm_msg.h"

/*************************************************************************
 * Type Definitions
 *************************************************************************/

/**
 * \brief MM Load and Dump file header structure
 * We use the same header structure for both dump and load files so a dump
 * file can be reloaded back into memory if desired (providing a memory save
 * and restore capability). This MM header is the secondary header, the
 * standard cFE file header is primary header for all load and dump files.
 */
typedef struct
{
    MM_SymAddr_t SymAddress; /**< \brief Symbolic load address or fully resolved dump address */
    uint32       NumOfBytes; /**< \brief Bytes to load or bytes dumped      */
    uint32       Crc;        /**< \brief CRC value for load or dump data    */
    uint8        MemType;    /**< \brief Memory type used                   */
    uint8        Spare[3];   /**< \brief Structure Padding                  */
} MM_LoadDumpFileHeader_t;

#endif
