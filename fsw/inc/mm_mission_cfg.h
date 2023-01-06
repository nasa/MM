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
 *   Specification for the CFS Memory Manager constants that can
 *   be configured from one mission to another
 */
#ifndef MM_MISSION_CFG_H
#define MM_MISSION_CFG_H

/************************************************************************
 * Includes
 ************************************************************************/
#include <cfe_mission_cfg.h>

/**
 * \defgroup cfsmmmissioncfg CFS Memory Manager Mission Configuration
 * \{
 */

/**
 * \brief CRC type for interrupts disabled loads
 *
 *  \par Description:
 *       CFE CRC type to use when processing the "memory load with
 *       interrupts disabled" (#MM_LOAD_MEM_WID_CC) command.
 *
 *  \par Limits:
 *       This must be one of the CRC types supported by the
 *       #CFE_ES_CalculateCRC function.
 */
#define MM_LOAD_WID_CRC_TYPE CFE_MISSION_ES_DEFAULT_CRC

/**
 * \brief CRC type for load files
 *
 *  \par Description:
 *       CFE CRC type to use when processing memory loads
 *       from a file.
 *
 *  \par Limits:
 *       This must be one of the CRC types supported by the
 *       #CFE_ES_CalculateCRC function.
 */
#define MM_LOAD_FILE_CRC_TYPE CFE_MISSION_ES_DEFAULT_CRC

/**
 * \brief CRC type for dump files
 *
 *  \par Description:
 *       CFE CRC type to use when processing memory dumps
 *       to a file.
 *
 *  \par Limits:
 *       This must be one of the CRC types supported by the
 *       #CFE_ES_CalculateCRC function.
 */
#define MM_DUMP_FILE_CRC_TYPE CFE_MISSION_ES_DEFAULT_CRC

/**\}*/

#endif
