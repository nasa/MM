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
 *   Specification for the CFS Memory Manager constants that can
 *   be configured from one platform to another
 */
#ifndef MM_PLATFORM_CFG_H
#define MM_PLATFORM_CFG_H

/**
 * \defgroup cfsmmplatformcfg CFS Memory Manager Platform Configuration
 * \{
 */

/**
 *  \brief Memory Managment File -- cFE file header sub-type
 *
 *  \par Description:
 *       This parameter defines the value that is used
 *       to identify a Memory Management file.
 *
 *  \par Limits:
 *       The file header data type for the value is 32 bits unsigned,
 *       thus the value can be anything from zero to 4,294,967,295.
 */
#define MM_CFE_HDR_SUBTYPE 0x4D4D5354

/**
 *  \brief Memory Management File -- cFE file header description
 *
 *  \par Description:
 *       This parameter defines the text string that
 *       may be used to identify Memory Management files.
 *
 *  \par Limits:
 *       The string length (including string terminator) cannot exceed
 *       #CFE_FS_HDR_DESC_MAX_LEN.  (limit is not verified)
 */
#define MM_CFE_HDR_DESCRIPTION "Memory Manager dump file"

/**
 * \brief Maximum number of bytes for a file load to RAM memory
 *
 *  \par Description:
 *       Maximum number of bytes that can be loaded into RAM from a
 *       single load file.
 *
 *  \par Limits:
 *       This parameter is limited only by the maximum value of the
 *       uint32 type.
 *       However, setting this value to a large number will increase
 *       the likelihood of MM being late responding to housekeeping
 *       requests since it cannot process such a request while a load
 *       or dump is in progress.
 */
#define MM_MAX_LOAD_FILE_DATA_RAM (1024 * 1024)

/**
 * \brief Maximum number of bytes for a file load to EEPROM memory
 *
 *  \par Description:
 *       Maximum number of bytes that can be loaded into EEPROM from a
 *       single load file.
 *
 *  \par Limits:
 *       This parameter is limited only by the maximum value of the
 *       uint32 type.
 *       However, setting this value to a large number will increase
 *       the likelihood of MM being late responding to housekeeping
 *       requests since it cannot process such a request while a load
 *       or dump is in progress.
 */
#define MM_MAX_LOAD_FILE_DATA_EEPROM (128 * 1024)

/**
 * \brief Maximum number of bytes for an uninterruptable load
 *
 *  \par Description:
 *       Maximum number of bytes that can be loaded with the
 *       "memory load with interrupts disabled" (#MM_LOAD_MEM_WID_CC)
 *       command.
 *
 *  \par Limits:
 *       This parameter is limited to the size of an uint8 which
 *       is the data type used to specify the number of bytes to
 *       load in the command message.
 *
 *       If this data type is made bigger, changing this value to a
 *       large number will increase the amount of time interrupts are
 *       disabled during the load. It should also be kept small enough
 *       to avoid packet segmentation for the command protocal being
 *       used.
 */
#define MM_MAX_UNINTERRUPTIBLE_DATA 200

/**
 * \brief Maximum number of bytes per load data segment
 *
 *  \par Description:
 *       Maximum number of bytes MM will load per task cycle
 *       to prevent CPU hogging (segmented load).
 *
 *  \par Limits:
 *       The MM app does not place a limit on this parameter.
 *       However, setting this value to a large number will decrease
 *       the amount of time available for other tasks to execute and
 *       increase MM CPU utilization during load operations.
 */
#define MM_MAX_LOAD_DATA_SEG 200

/**
 * \brief Maximum number of bytes for a file dump from RAM memory
 *
 *  \par Description:
 *       Maximum number of bytes that can be dumped from RAM into a
 *       single dump file.
 *
 *  \par Limits:
 *       This parameter is limited only by the maximum value of the
 *       uint32 type.
 *       However, setting this value to a large number will increase
 *       the likelihood of MM being late responding to housekeeping
 *       requests since it cannot process such a request while a load
 *       or dump is in progress.
 */
#define MM_MAX_DUMP_FILE_DATA_RAM (1024 * 1024)

/**
 * \brief Maximum number of bytes for a file dump from EEPROM memory
 *
 *  \par Description:
 *       Maximum number of bytes that can be dumped from EEPROM into a
 *       single dump file.
 *
 *  \par Limits:
 *       This parameter is limited only by the maximum value of the
 *       uint32 type.
 *       However, setting this value to a large number will increase
 *       the likelihood of MM being late responding to housekeeping
 *       requests since it cannot process such a request while a load
 *       or dump is in progress.
 */
#define MM_MAX_DUMP_FILE_DATA_EEPROM (128 * 1024)

/**
 * \brief Maximum number of bytes for a symbol table file dump
 *
 *  \par Description:
 *       Maximum number of bytes that can be dumped from the symbol table
 *       into a single dump file.
 *
 *  \par Limits:
 *       This parameter is limited only by the maximum value of the
 *       uint32 type.
 *       However, setting this value to a large number will impact
 *       the OSAL since it is responsible for generating the dump file.
 */
#define MM_MAX_DUMP_FILE_DATA_SYMTBL (128 * 1024)

/**
 * \brief Maximum number of bytes per dump data segment
 *
 *  \par Description:
 *       Maximum number of bytes MM will dump per task cycle
 *       to prevent CPU hogging (segmented dump).
 *
 *  \par Limits:
 *       This parameter is limited only by the maximum value of the
 *       uint32 type.
 *       However, setting this value to a large number will decrease
 *       the amount of time available for other tasks to execute and
 *       increase MM CPU utilization during dump operations.
 */
#define MM_MAX_DUMP_DATA_SEG 200

/**
 * \brief Maximum number of bytes for a fill to RAM memory
 *
 *  \par Description:
 *       Maximum number of bytes that can be loaded into RAM with a
 *       single memory fill command.
 *
 *  \par Limits:
 *       This parameter is limited only by the maximum value of the
 *       uint32 type.
 *       However, setting this value to a large number will increase
 *       the likelihood of MM being late responding to housekeeping
 *       requests since it cannot process such a request while a fill
 *       operation is in progress.
 */
#define MM_MAX_FILL_DATA_RAM (1024 * 1024)

/**
 * \brief Maximum number of bytes for a fill to EEPROM memory
 *
 *  \par Description:
 *       Maximum number of bytes that can be loaded into EEPROM with a
 *       single memory fill command.
 *
 *  \par Limits:
 *       This parameter is limited only by the maximum value of the
 *       uint32 type.
 *       However, setting this value to a large number will increase
 *       the likelihood of MM being late responding to housekeeping
 *       requests since it cannot process such a request while a fill
 *       operation is in progress.
 */
#define MM_MAX_FILL_DATA_EEPROM (128 * 1024)

/**
 * \brief Maximum number of bytes per fill data segment
 *
 *  \par Description:
 *       Maximum number of bytes MM will fill per task cycle
 *       to prevent CPU hogging (segmented fill).
 *
 *  \par Limits:
 *       This parameter is limited only by the maximum value of the
 *       uint32 type.
 *       However, setting this value to a large number will decrease
 *       the amount of time available for other tasks to execute and
 *       increase MM CPU utilization during memory fill operations.
 */
#define MM_MAX_FILL_DATA_SEG 200

/**
 * \brief Optional MEM32 compile switch
 *
 *  \par Description:
 *       Compile switch to include code for the optional MM_MEM32 memory.
 *       If defined the code will be included.  Otherwise the code will be
 *       excluded.
 *
 *  \par Limits:
 *       n/a
 */
#define MM_OPT_CODE_MEM32_MEMTYPE

/**
 * \brief Maximum number of bytes for a file load to MEM32 memory
 *
 *  \par Description:
 *       Maximum number of bytes that can be loaded into the optional
 *       MEM32 memory type from a single load file.
 *
 *  \par Limits:
 *       This value should be longword aligned.
 *       This parameter is limited by the maximum value of the
 *       uint32 type.
 *       Setting this value to a large number will increase the likelihood
 *       of MM being late responding to housekeeping requests since it
 *       cannot process such a request while a load or dump is in progress.
 */
#define MM_MAX_LOAD_FILE_DATA_MEM32 (1024 * 1024)

/**
 * \brief Maximum number of bytes for a file dump from MEM32 memory
 *
 *  \par Description:
 *       Maximum number of bytes that can be dumped from the optional
 *       MEM32 memory type to a single dump file.
 *
 *  \par Limits:
 *       This value should be longword aligned.
 *       This parameter is limited by the maximum value of the
 *       uint32 type.
 *       Setting this value to a large number will increase the likelihood
 *       of MM being late responding to housekeeping requests since it
 *       cannot process such a request while a load or dump is in progress.
 */
#define MM_MAX_DUMP_FILE_DATA_MEM32 (1024 * 1024)

/**
 * \brief Maximum number of bytes for a fill to MEM32 memory
 *
 *  \par Description:
 *       Maximum number of bytes that can be loaded into the optional
 *       MEM32 memory type with a single memory fill command.
 *
 *  \par Limits:
 *       This value should be longword aligned.
 *       This parameter is limited by the maximum value of the
 *       uint16 type.
 *       Setting this value to a large number will increase the likelihood
 *       of MM being late responding to housekeeping requests since it
 *       cannot process such a request while a memory fill operation
 *       is in progress.
 */
#define MM_MAX_FILL_DATA_MEM32 (1024 * 1024)

/**
 * \brief Optional MEM16 compile switch
 *
 *  \par Description:
 *       Compile switch to include code for the optional MM_MEM16 memory.
 *       If defined the the code will be included.  Otherwise it will be
 *       excluded.
 *
 *  \par Limits:
 *       n/a
 */
#define MM_OPT_CODE_MEM16_MEMTYPE

/**
 * \brief Maximum number of bytes for a file load to MEM16 memory
 *
 *  \par Description:
 *       Maximum number of bytes that can be loaded into the optional
 *       MEM16 memory type from a single load file.
 *
 *  \par Limits:
 *       This value should be word aligned.
 *       This parameter is limited by the maximum value of the
 *       uint16 type.
 *       Setting this value to a large number will increase the likelihood
 *       of MM being late responding to housekeeping requests since it
 *       cannot process such a request while a load or dump is in progress.
 */
#define MM_MAX_LOAD_FILE_DATA_MEM16 (1024 * 1024)

/**
 * \brief Maximum number of bytes for a file dump from MEM16 memory
 *
 *  \par Description:
 *       Maximum number of bytes that can be dumped from the optional
 *       MEM16 memory type to a single dump file.
 *
 *  \par Limits:
 *       This value should be word aligned.
 *       Setting this value to a large number will increase the likelyhood
 *       of MM being late responding to housekeeping requests since it
 *       cannot process such a request while a load or dump is in progress.
 */
#define MM_MAX_DUMP_FILE_DATA_MEM16 (1024 * 1024)

/**
 * \brief Maximum number of bytes for a fill to MEM16 memory
 *
 *  \par Description:
 *       Maximum number of bytes that can be loaded into the optional
 *       MEM16 memory type with a single memory fill command.
 *
 *  \par Limits:
 *       This value should be word aligned.
 *       This parameter is limited by the maximum value of the
 *       uint16 type.
 *       Setting this value to a large number will increase the likelihood
 *       of MM being late responding to housekeeping requests since it
 *       cannot process such a request while a memory fill operation
 *       is in progress.
 */
#define MM_MAX_FILL_DATA_MEM16 (1024 * 1024)

/**
 * \brief Optional MEM8 compile switch
 *
 *  \par Description:
 *       Compile switch to include code for the optional MM_MEM8 memory.
 *       If defined the code will be included.  Otherwise the code will be
 *       excluded.
 *
 *  \par Limits:
 *       n/a
 */
#define MM_OPT_CODE_MEM8_MEMTYPE

/**
 * \brief Maximum number of bytes for a file load to MEM8 memory
 *
 *  \par Description:
 *       Maximum number of bytes that can be loaded into the optional
 *       MEM8 memory type from a single load file.
 *
 *  \par Limits:
 *       This parameter is limited by the maximum value of the uint8 type.
 *       Setting this value to a large number will increase the likelihood
 *       of MM being late responding to housekeeping requests since it
 *       cannot process such a request while a load or dump is in progress.
 */
#define MM_MAX_LOAD_FILE_DATA_MEM8 (1024 * 1024)

/**
 * \brief Maximum number of bytes for a file dump from MEM8 memory
 *
 *  \par Description:
 *       Maximum number of bytes that can be dumped from the optional
 *       MEM8 memory type to a single dump file.
 *
 *  \par Limits:
 *       This parameter is limited by the maximum value of the uint8 type.
 *       Setting this value to a large number will increase the likelihood
 *       of MM being late responding to housekeeping requests since it
 *       cannot process such a request while a load or dump is in progress.
 */
#define MM_MAX_DUMP_FILE_DATA_MEM8 (1024 * 1024)

/**
 * \brief Maximum number of bytes for a fill to MEM8 memory
 *
 *  \par Description:
 *       Maximum number of bytes that can be loaded into the optional
 *       MEM8 memory type with a single memory fill command.
 *
 *  \par Limits:
 *       This parameter is limited by the maximum value of the uint8 type.
 *       Setting this value to a large number will increase the likelihood
 *       of MM being late responding to housekeeping requests since it
 *       cannot process such a request while a memory fill operation
 *       is in progress.
 */
#define MM_MAX_FILL_DATA_MEM8 (1024 * 1024)

/**
 * \brief Segment break processor delay
 *
 *  \par Description:
 *       How many milliseconds to delay between segments for dump, load,
 *       and fill operations. A value of zero cycles through the
 *       OS scheduler, giving up what's left of the current timeslice.
 *
 *  \par Limits:
 *       The MM app does not place a limit on this parameter.
 *       However, setting this value to a large number will increase the
 *       time required to process load, dump, and fill requests.
 *       It will also increase the likelyhood of MM being late responding
 *       to housekeeping requests since it cannot process such a request
 *       while a memory operation is in progress.
 */
#define MM_PROCESSOR_CYCLE 0

/**
 * \brief Mission specific version number for MM application
 *
 *  \par Description:
 *       An application version number consists of four parts:
 *       major version number, minor version number, revision
 *       number and mission specific revision number. The mission
 *       specific revision number is defined here and the other
 *       parts are defined in "mm_version.h".
 *
 *  \par Limits:
 *       Must be defined as a numeric value that is greater than
 *       or equal to zero.
 */
#define MM_MISSION_REV 0

/**\}*/

#endif
