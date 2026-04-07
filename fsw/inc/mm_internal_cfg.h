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

#ifndef MM_INTERNAL_CFG_H
#define MM_INTERNAL_CFG_H

/* ======== */
/* Includes */
/* ======== */

#include "mm_internal_cfg_values.h"

/* ====== */
/* Macros */
/* ====== */

/** \brief MM command pipe depth */
#define MM_INTERNAL_CMD_PIPE_DEPTH         MM_INTERNAL_CFGVAL(CMD_PIPE_DEPTH)
#define DEFAULT_MM_INTERNAL_CMD_PIPE_DEPTH (3 * CFE_PLATFORM_SB_DEFAULT_MSG_LIMIT)

/** \brief MM command pipe name */
#define MM_INTERNAL_CMD_PIPE_NAME         MM_INTERNAL_CFGVAL(CMD_PIPE_NAME)
#define DEFAULT_MM_INTERNAL_CMD_PIPE_NAME "MM_CMD_PIPE"

/**
 * \brief Wakeup for MM
 *
 * \par Description
 *      Wakes up MM every 1 second for routine maintenance whether a
 *      message was received or not.
 */
#define MM_INTERNAL_SB_TIMEOUT         MM_INTERNAL_CFGVAL(SB_TIMEOUT)
#define DEFAULT_MM_INTERNAL_SB_TIMEOUT 1000

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
#define MM_INTERNAL_CFE_HDR_SUBTYPE         MM_INTERNAL_CFGVAL(CFE_HDR_SUBTYPE)
#define DEFAULT_MM_INTERNAL_CFE_HDR_SUBTYPE 0x4D4D5354

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
#define MM_INTERNAL_CFE_HDR_DESCRIPTION         MM_INTERNAL_CFGVAL(CFE_HDR_DESCRIPTION)
#define DEFAULT_MM_INTERNAL_CFE_HDR_DESCRIPTION "Memory Manager dump file"

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
#define MM_INTERNAL_MAX_LOAD_FILE_DATA_RAM         MM_INTERNAL_CFGVAL(MAX_LOAD_FILE_DATA_RAM)
#define DEFAULT_MM_INTERNAL_MAX_LOAD_FILE_DATA_RAM (1024 * 1024)

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
#define MM_INTERNAL_MAX_LOAD_FILE_DATA_EEPROM         MM_INTERNAL_CFGVAL(MAX_LOAD_FILE_DATA_EEPROM)
#define DEFAULT_MM_INTERNAL_MAX_LOAD_FILE_DATA_EEPROM (128 * 1024)

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
#define MM_INTERNAL_MAX_LOAD_DATA_SEG         MM_INTERNAL_CFGVAL(MAX_LOAD_DATA_SEG)
#define DEFAULT_MM_INTERNAL_MAX_LOAD_DATA_SEG 200

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
#define MM_INTERNAL_MAX_DUMP_FILE_DATA_RAM         MM_INTERNAL_CFGVAL(MAX_DUMP_FILE_DATA_RAM)
#define DEFAULT_MM_INTERNAL_MAX_DUMP_FILE_DATA_RAM (1024 * 1024)

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
#define MM_INTERNAL_MAX_DUMP_FILE_DATA_EEPROM         MM_INTERNAL_CFGVAL(MAX_DUMP_FILE_DATA_EEPROM)
#define DEFAULT_MM_INTERNAL_MAX_DUMP_FILE_DATA_EEPROM (128 * 1024)

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
#define MM_INTERNAL_MAX_DUMP_FILE_DATA_SYMTBL         MM_INTERNAL_CFGVAL(MAX_DUMP_FILE_DATA_SYMTBL)
#define DEFAULT_MM_INTERNAL_MAX_DUMP_FILE_DATA_SYMTBL (128 * 1024)

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
#define MM_INTERNAL_MAX_DUMP_DATA_SEG         MM_INTERNAL_CFGVAL(MAX_DUMP_DATA_SEG)
#define DEFAULT_MM_INTERNAL_MAX_DUMP_DATA_SEG 200

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
#define MM_INTERNAL_MAX_FILL_DATA_RAM         MM_INTERNAL_CFGVAL(MAX_FILL_DATA_RAM)
#define DEFAULT_MM_INTERNAL_MAX_FILL_DATA_RAM (1024 * 1024)

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
#define MM_INTERNAL_MAX_FILL_DATA_EEPROM         MM_INTERNAL_CFGVAL(MAX_FILL_DATA_EEPROM)
#define DEFAULT_MM_INTERNAL_MAX_FILL_DATA_EEPROM (128 * 1024)

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
#define MM_INTERNAL_MAX_FILL_DATA_SEG         MM_INTERNAL_CFGVAL(MAX_FILL_DATA_SEG)
#define DEFAULT_MM_INTERNAL_MAX_FILL_DATA_SEG 200

/**
 * \brief Optional MEM32 compile switch
 *
 *  \par Description:
 *       Compile switch to include code for the optional MM_MemType_MEM32
 * memory. If defined the code will be included.  Otherwise the code will be
 *       excluded.
 *
 *  \par Limits:
 *       n/a
 */
#define MM_INTERNAL_OPT_CODE_MEM32_MEMTYPE

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
#define MM_INTERNAL_MAX_LOAD_FILE_DATA_MEM32         MM_INTERNAL_CFGVAL(MAX_LOAD_FILE_DATA_MEM32)
#define DEFAULT_MM_INTERNAL_MAX_LOAD_FILE_DATA_MEM32 (1024 * 1024)

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
#define MM_INTERNAL_MAX_DUMP_FILE_DATA_MEM32         MM_INTERNAL_CFGVAL(MAX_DUMP_FILE_DATA_MEM32)
#define DEFAULT_MM_INTERNAL_MAX_DUMP_FILE_DATA_MEM32 (1024 * 1024)

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
#define MM_INTERNAL_MAX_FILL_DATA_MEM32         MM_INTERNAL_CFGVAL(MAX_FILL_DATA_MEM32)
#define DEFAULT_MM_INTERNAL_MAX_FILL_DATA_MEM32 (1024 * 1024)

/**
 * \brief Optional MEM16 compile switch
 *
 *  \par Description:
 *       Compile switch to include code for the optional MM_MemType_MEM16
 * memory. If defined the the code will be included.  Otherwise it will be
 *       excluded.
 *
 *  \par Limits:
 *       n/a
 */
#define MM_INTERNAL_OPT_CODE_MEM16_MEMTYPE

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
#define MM_INTERNAL_MAX_LOAD_FILE_DATA_MEM16         MM_INTERNAL_CFGVAL(MAX_LOAD_FILE_DATA_MEM16)
#define DEFAULT_MM_INTERNAL_MAX_LOAD_FILE_DATA_MEM16 (1024 * 1024)

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
#define MM_INTERNAL_MAX_DUMP_FILE_DATA_MEM16         MM_INTERNAL_CFGVAL(MAX_DUMP_FILE_DATA_MEM16)
#define DEFAULT_MM_INTERNAL_MAX_DUMP_FILE_DATA_MEM16 (1024 * 1024)

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
#define MM_INTERNAL_MAX_FILL_DATA_MEM16         MM_INTERNAL_CFGVAL(MAX_FILL_DATA_MEM16)
#define DEFAULT_MM_INTERNAL_MAX_FILL_DATA_MEM16 (1024 * 1024)

/**
 * \brief Optional MEM8 compile switch
 *
 *  \par Description:
 *       Compile switch to include code for the optional MM_MemType_MEM8 memory.
 *       If defined the code will be included.  Otherwise the code will be
 *       excluded.
 *
 *  \par Limits:
 *       n/a
 */
#define MM_INTERNAL_OPT_CODE_MEM8_MEMTYPE

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
#define MM_INTERNAL_MAX_LOAD_FILE_DATA_MEM8         MM_INTERNAL_CFGVAL(MAX_LOAD_FILE_DATA_MEM8)
#define DEFAULT_MM_INTERNAL_MAX_LOAD_FILE_DATA_MEM8 (1024 * 1024)

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
#define MM_INTERNAL_MAX_DUMP_FILE_DATA_MEM8         MM_INTERNAL_CFGVAL(MAX_DUMP_FILE_DATA_MEM8)
#define DEFAULT_MM_INTERNAL_MAX_DUMP_FILE_DATA_MEM8 (1024 * 1024)

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
#define MM_INTERNAL_MAX_FILL_DATA_MEM8         MM_INTERNAL_CFGVAL(MAX_FILL_DATA_MEM8)
#define DEFAULT_MM_INTERNAL_MAX_FILL_DATA_MEM8 (1024 * 1024)

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
#define MM_INTERNAL_PROCESSOR_CYCLE         MM_INTERNAL_CFGVAL(PROCESSOR_CYCLE)
#define DEFAULT_MM_INTERNAL_PROCESSOR_CYCLE 0

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
#define MM_INTERNAL_MISSION_REV         MM_INTERNAL_CFGVAL(MISSION_REV)
#define DEFAULT_MM_INTERNAL_MISSION_REV 0

/**
 * \brief Maximum dump bytes in an event string
 *
 * This macro defines the maximum number of bytes that can be dumped
 * in an event message string based upon the setting of the
 * CFE_MISSION_EVS_MAX_MESSAGE_LENGTH configuration parameter.
 *
 * The event message format is:
 *    Message head "Memory Dump: "             13 characters
 *    Message body "0xFF "                      5 characters per dump byte
 *    Message tail "from address: 0xFFFFFFFF"  33 characters including NUL on
 * 64-bit system
 */
#define MM_INTERNAL_MAX_DUMP_INEVENT_BYTES         MM_INTERNAL_CFGVAL(MAX_DUMP_INEVENT_BYTES)
#define DEFAULT_MM_INTERNAL_MAX_DUMP_INEVENT_BYTES ((CFE_MISSION_EVS_MAX_MESSAGE_LENGTH - (13 + 33)) / 5)

/**
 * \brief Dump in an event scratch string size
 *
 * This macro defines the size of the scratch buffer used to build
 * the dump in event message string. Set it to the size of the
 * largest piece shown above including room for a NUL terminator.
 */
#define MM_INTERNAL_DUMPINEVENT_TEMP_CHARS         MM_INTERNAL_CFGVAL(DUMPINEVENT_TEMP_CHARS)
#define DEFAULT_MM_INTERNAL_DUMPINEVENT_TEMP_CHARS 36

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
#define MM_INTERNAL_DUMP_FILE_CRC_TYPE         MM_INTERNAL_CFGVAL(DUMP_FILE_CRC_TYPE)
#define DEFAULT_MM_INTERNAL_DUMP_FILE_CRC_TYPE CFE_MISSION_ES_DEFAULT_CRC

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
#define MM_INTERNAL_LOAD_WID_CRC_TYPE         MM_INTERNAL_CFGVAL(LOAD_WID_CRC_TYPE)
#define DEFAULT_MM_INTERNAL_LOAD_WID_CRC_TYPE CFE_MISSION_ES_DEFAULT_CRC

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
#define MM_INTERNAL_LOAD_FILE_CRC_TYPE         MM_INTERNAL_CFGVAL(LOAD_FILE_CRC_TYPE)
#define DEFAULT_MM_INTERNAL_LOAD_FILE_CRC_TYPE CFE_MISSION_ES_DEFAULT_CRC

/**
 * \brief Misc Initialization Values
 */

#define MM_INTERNAL_CLEAR_SYMNAME         MM_INTERNAL_CFGVAL(CLEAR_SYMNAME)
#define DEFAULT_MM_INTERNAL_CLEAR_SYMNAME '\0' /**< \brief Used to clear out symbol name strings      */

#define MM_INTERNAL_CLEAR_FNAME         MM_INTERNAL_CFGVAL(CLEAR_FNAME)
#define DEFAULT_MM_INTERNAL_CLEAR_FNAME '\0' /**< \brief Used to clear out file name strings        */

#define MM_INTERNAL_CLEAR_ADDR         MM_INTERNAL_CFGVAL(CLEAR_ADDR)
#define DEFAULT_MM_INTERNAL_CLEAR_ADDR 0 /**< \brief Used to clear out memory address variables */

#define MM_INTERNAL_CLEAR_PATTERN         MM_INTERNAL_CFGVAL(CLEAR_PATTERN)
#define DEFAULT_MM_INTERNAL_CLEAR_PATTERN 0 /**< \brief Used to clear out fill and test patterns   */

/**
 * \brief MM Data Sizes for Peeks and Pokes
 */

#define MM_INTERNAL_BYTE_BIT_WIDTH         MM_INTERNAL_CFGVAL(BYTE_BIT_WIDTH)
#define DEFAULT_MM_INTERNAL_BYTE_BIT_WIDTH 8 /**< \brief Byte bit width */

#define MM_INTERNAL_WORD_BIT_WIDTH         MM_INTERNAL_CFGVAL(WORD_BIT_WIDTH)
#define DEFAULT_MM_INTERNAL_WORD_BIT_WIDTH 16 /**< \brief Word bit width */

#define MM_INTERNAL_DWORD_BIT_WIDTH         MM_INTERNAL_CFGVAL(DWORD_BIT_WIDTH)
#define DEFAULT_MM_INTERNAL_DWORD_BIT_WIDTH 32 /**< \brief Double word bit width */

#endif /* MM_INTERNAL_CFG_H */