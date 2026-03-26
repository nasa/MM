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

#ifndef MM_FCNCODES_H
#define MM_FCNCODES_H

/* ======== */
/* Includes */
/* ======== */

#include "mm_fcncode_values.h"

/* ====== */
/* Macros */
/* ====== */

/**
 * \defgroup cfsmmcmdcodes CFS Memory Manager Command Codes
 * \{
 */

/**
 * \brief Noop
 *
 *  \par Description
 *       Implements the Noop command that insures the MM task is alive
 *
 *  \par Command Structure
 *       #MM_NoopCmd_t
 *
 *  \par Command Verification
 *       Successful execution of this command may be verified with
 *       the following telemetry:
 *       - #MM_HkTlm_Payload_t.CommandCounter will increment
 *       - The #MM_NOOP_INF_EID informational event message will be
 *         generated when the command is received
 *
 *  \par Error Conditions
 *       This command may fail for the following reason(s):
 *       - Command packet length not as expected
 *
 *  \par Evidence of failure may be found in the following telemetry:
 *       - #MM_HkTlm_Payload_t.CommandErrorCounter will increment
 *       - Error specific event message #MM_CMD_LEN_ERR_EID
 *
 *  \par Criticality
 *       None
 *
 *  \sa #MM_RESET_COUNTERS_CC
 */
#define MM_NOOP_CC MM_CCVAL(NOOP)

/**
 * \brief Reset Counters
 *
 *  \par Description
 *       Resets the MM housekeeping counters
 *
 *  \par Command Structure
 *       #MM_ResetCountersCmd_t
 *
 *  \par Command Verification
 *       Successful execution of this command may be verified with
 *       the following telemetry:
 *       - #MM_HkTlm_Payload_t.CommandCounter will be cleared
 *       - #MM_HkTlm_Payload_t.CommandErrorCounter will be cleared
 *       - The #MM_RESET_INF_EID informational event message will be
 *         generated when the command is executed
 *
 *  \par Error Conditions
 *       This command may fail for the following reason(s):
 *       - Command packet length not as expected
 *
 *  \par Evidence of failure may be found in the following telemetry:
 *       - #MM_HkTlm_Payload_t.CommandErrorCounter will increment
 *       - Error specific event message #MM_CMD_LEN_ERR_EID
 *
 *  \par Criticality
 *       None
 *
 *  \sa #MM_NOOP_CC
 */
#define MM_RESET_COUNTERS_CC MM_CCVAL(RESET)

/**
 * \brief Memory Peek
 *
 *  \par Description
 *       Reads 8,16, or 32 bits of data from any given input address
 *
 *  \par Command Structure
 *       #MM_PeekCmd_t
 *
 *  \par Command Verification
 *       Successful execution of this command may be verified with
 *       the following telemetry:
 *       - #MM_HkTlm_Payload_t.CommandCounter will increment
 *       - #MM_HkTlm_Payload_t.LastAction will be set to #MM_LastAction_PEEK
 *       - #MM_HkTlm_Payload_t.MemType will be set to the commanded memory type
 *       - #MM_HkTlm_Payload_t.Address will be set to the fully resolved
 * destination memory address
 *       - #MM_HkTlm_Payload_t.DataValue will be set to the value contained in
 * the requested address
 *       - #MM_HkTlm_Payload_t.BytesProcessed will be set to the byte size of
 * the peek operation (1, 2, or 4)
 *       - The #MM_PEEK_BYTE_INF_EID informational event message will
 *         be generated with the peek data if the data size was 8 bits
 *       - The #MM_PEEK_WORD_INF_EID informational event message will
 *         be generated with the peek data if the data size was 16 bits
 *       - The #MM_PEEK_DWORD_INF_EID informational event message will
 *         be generated with the peek data if the data size was 32 bits
 *
 *  \par Error Conditions
 *       This command may fail for the following reason(s):
 *       - Command packet length not as expected
 *       - A symbol name was specified that can't be resolved
 *       - The specified data size is invalid
 *       - The specified memory type is invalid
 *       - The address range fails validation check
 *       - The address and data size are not properly aligned
 *
 *  \par Evidence of failure may be found in the following telemetry:
 *       - #MM_HkTlm_Payload_t.CommandErrorCounter will increment
 *       - Error specific event message #MM_CMD_LEN_ERR_EID
 *       - Error specific event message #MM_SYMNAME_ERR_EID
 *       - Error specific event message #MM_DATA_SIZE_BITS_ERR_EID
 *       - Error specific event message #MM_MEMTYPE_ERR_EID
 *       - Error specific event message #MM_OS_MEMVALIDATE_ERR_EID
 *       - Error specific event message #MM_ALIGN16_ERR_EID
 *       - Error specific event message #MM_ALIGN32_ERR_EID
 *
 *  \par Criticality
 *       It is the responsibility of the user to verify the <i> DestSymAddress
 * </i> and <i> MemType </i> in the command.  It is possible to generate a
 * machine check exception when accessing I/O memory addresses/registers and
 * other types of memory. The user is cautioned to use extreme care.
 *
 *       Note: Valid memory ranges are defined within a hardcoded structure
 * contained in the PSP layer (CFE_PSP_MemoryTable) however, not every address
 * within the defined ranges may be valid.
 *
 *  \sa #MM_POKE_CC
 */
#define MM_PEEK_CC MM_CCVAL(PEEK)

/**
 * \brief Memory Poke
 *
 *  \par Description
 *       Writes 8, 16, or 32 bits of data to any memory address
 *
 *  \par Command Structure
 *       #MM_PokeCmd_t
 *
 *  \par Command Verification
 *       Successful execution of this command may be verified with
 *       the following telemetry:
 *       - #MM_HkTlm_Payload_t.CommandCounter will increment
 *       - #MM_HkTlm_Payload_t.LastAction will be set to #MM_LastAction_POKE
 *       - #MM_HkTlm_Payload_t.MemType will be set to the commanded memory type
 *       - #MM_HkTlm_Payload_t.Address will be set to the fully resolved source
 * memory address
 *       - #MM_HkTlm_Payload_t.DataValue will be set to the commanded poke data
 * value
 *       - #MM_HkTlm_Payload_t.BytesProcessed will be set to the byte size of
 * the poke operation (1, 2, or 4)
 *       - The #MM_POKE_BYTE_INF_EID informational event message will
 *         be generated if the data size was 8 bits
 *       - The #MM_POKE_WORD_INF_EID informational event message will
 *         be generated if the data size was 16 bits
 *       - The #MM_POKE_DWORD_INF_EID informational event message will
 *         be generated if the data size was 32 bits
 *
 *  \par Error Conditions
 *       This command may fail for the following reason(s):
 *       - Command packet length not as expected
 *       - A symbol name was specified that can't be resolved
 *       - The specified data size is invalid
 *       - The specified memory type is invalid
 *       - The address range fails validation check
 *       - The address and data size are not properly aligned
 *       - An EEPROM write error occured
 *
 *  \par Evidence of failure may be found in the following telemetry:
 *       - #MM_HkTlm_Payload_t.CommandErrorCounter will increment
 *       - Error specific event message #MM_CMD_LEN_ERR_EID
 *       - Error specific event message #MM_SYMNAME_ERR_EID
 *       - Error specific event message #MM_DATA_SIZE_BITS_ERR_EID
 *       - Error specific event message #MM_MEMTYPE_ERR_EID
 *       - Error specific event message #MM_OS_MEMVALIDATE_ERR_EID
 *       - Error specific event message #MM_ALIGN16_ERR_EID
 *       - Error specific event message #MM_ALIGN32_ERR_EID
 *       - Error specific event message #MM_OS_EEPROMWRITE8_ERR_EID
 *       - Error specific event message #MM_OS_EEPROMWRITE16_ERR_EID
 *       - Error specific event message #MM_OS_EEPROMWRITE32_ERR_EID
 *
 *  \par Criticality
 *       It is the responsibility of the user to verify the
 * <i>DestSymAddress</i>, <i>MemType</i>, and <i>Data</i> in the command.  It is
 * highly recommended to verify the success or failure of the memory poke.  The
 * poke may be verified by issuing a subsequent peek command and evaluating the
 * returned value.  It is possible to destroy critical information with this
 * command causing unknown consequences. In addition, it is possible to generate
 * a machine check exception when accessing I/O memory addresses/registers and
 * other types of memory. The user is cautioned to use extreme care.
 *
 *       Note: Valid memory ranges are defined within a hardcoded structure
 * contained in the PSP layer (CFE_PSP_MemoryTable) however, not every address
 * within the defined ranges may be valid.
 *
 *  \sa #MM_PEEK_CC
 */
#define MM_POKE_CC MM_CCVAL(POKE)

/**
 * \brief Memory Load With Interrupts Disabled
 *
 *  \par Description
 *       Reprogram processor memory with input data.  Loads up to
 *       #MM_INTERFACE_MAX_UNINTERRUPTIBLE_DATA data bytes into RAM with
 *       interrupts disabled
 *
 *  \par Command Structure
 *       #MM_LoadMemWIDCmd_t
 *
 *  \par Command Verification
 *       Successful execution of this command may be verified with
 *       the following telemetry:
 *       - #MM_HkTlm_Payload_t.CommandCounter will increment
 *       - #MM_HkTlm_Payload_t.LastAction will be set to #MM_LastAction_LOAD_WID
 *       - #MM_HkTlm_Payload_t.Address will be set to the fully resolved
 * destination memory address
 *       - #MM_HkTlm_Payload_t.BytesProcessed will be set to the number of bytes
 * loaded
 *       - The #MM_LOAD_WID_INF_EID information event message will be
 *         generated when the command is executed
 *
 *  \par Error Conditions
 *       This command may fail for the following reason(s):
 *       - Command packet length not as expected
 *       - A symbol name was specified that can't be resolved
 *       - The computed CRC doesn't match the command message value
 *       - The address range fails validation check
 *       - Invalid data size specified in command message
 *
 *  \par Evidence of failure may be found in the following telemetry:
 *       - #MM_HkTlm_Payload_t.CommandErrorCounter will increment
 *       - Error specific event message #MM_CMD_LEN_ERR_EID
 *       - Error specific event message #MM_SYMNAME_ERR_EID
 *       - Error specific event message #MM_LOAD_WID_CRC_ERR_EID
 *       - Error specific event message #MM_OS_MEMVALIDATE_ERR_EID
 *       - Error specific event message #MM_DATA_SIZE_BYTES_ERR_EID
 *
 *  \par Criticality
 *       It is the responsibility of the user to verify the
 * <i>DestSymAddress</i>, <i>NumOfBytes</i>, and <i>DataArray</i> contents in
 * the command.  It is highly recommended to verify the success or failure of
 * the memory load.  The load may be verified by dumping memory and evaluating
 * the dump contents.  It is possible to destroy critical information with this
 * command causing unknown consequences. In addition, it is possible to generate
 * a machine check exception when accessing I/O memory addresses/registers and
 * other types of memory. The user is cautioned to use extreme care.
 *
 *       Note: Valid memory ranges are defined within a hardcoded structure
 * contained in the PSP layer (CFE_PSP_MemoryTable) however, not every address
 * within the defined ranges may be valid.
 *
 */
#define MM_LOAD_MEM_WID_CC MM_CCVAL(LOAD_MEM_WID)

/**
 * \brief Memory Load From File
 *
 *  \par Description
 *       Reprograms processor memory with the data contained within the given
 *       input file
 *
 *  \par Command Structure
 *       #MM_LoadMemFromFileCmd_t
 *
 *  \par Command Verification
 *       Successful execution of this command may be verified with
 *       the following telemetry:
 *       - #MM_HkTlm_Payload_t.CommandCounter will increment
 *       - #MM_HkTlm_Payload_t.LastAction will be set to
 * #MM_LastAction_LOAD_FROM_FILE
 *       - #MM_HkTlm_Payload_t.MemType will be set to the commanded memory type
 *       - #MM_HkTlm_Payload_t.Address will be set to the fully resolved
 * destination memory address
 *       - #MM_HkTlm_Payload_t.BytesProcessed will be set to the number of bytes
 * loaded
 *       - #MM_HkTlm_Payload_t.FileName will be set to the load file name
 *       - The #MM_LD_MEM_FILE_INF_EID informational event message will
 *         be generated
 *
 *  \par Error Conditions
 *       This command may fail for the following reason(s):
 *       - Command packet length not as expected
 *       - Command specified filename is invalid
 *       - #OS_OpenCreate call fails
 *       - #OS_close call fails
 *       - #OS_read doesn't read the expected number of bytes
 *       - The #MM_ComputeCRCFromFile call fails
 *       - The computed CRC doesn't match the load file value
 *       - A symbol name was specified that can't be resolved
 *       - #CFE_FS_ReadHeader call fails
 *       - #OS_read call fails
 *       - The address range fails validation check
 *       - The specified data size is invalid
 *       - The address and data size are not properly aligned
 *       - The specified memory type is invalid
 *
 *  \par Evidence of failure may be found in the following telemetry:
 *       - #MM_HkTlm_Payload_t.CommandErrorCounter will increment
 *       - Error specific event message #MM_CMD_LEN_ERR_EID
 *       - Error specific event message #MM_OS_OPEN_ERR_EID
 *       - Error specific event message #MM_OS_CLOSE_ERR_EID
 *       - Error specific event message #MM_OS_READ_EXP_ERR_EID
 *       - Error specific event message #MM_COMPUTECRCFROMFILE_ERR_EID
 *       - Error specific event message #MM_LOAD_FILE_CRC_ERR_EID
 *       - Error specific event message #MM_SYMNAME_ERR_EID
 *       - Error specific event message #MM_FILE_LOAD_PARAMS_ERR_EID
 *       - Error specific event message #MM_CFE_FS_READHDR_ERR_EID
 *       - Error specific event message #MM_OS_READ_ERR_EID
 *       - Error specific event message #MM_OS_MEMVALIDATE_ERR_EID
 *       - Error specific event message #MM_DATA_SIZE_BYTES_ERR_EID
 *       - Error specific event message #MM_ALIGN32_ERR_EID
 *       - Error specific event message #MM_ALIGN16_ERR_EID
 *       - Error specific event message #MM_MEMTYPE_ERR_EID
 *
 *  \par Criticality
 *       It is the responsibility of the user to verify the  contents of the
 * load file in the command.  It is highly recommended to verify the success or
 * failure of the memory load.  The load may be verified by dumping memory and
 * evaluating the dump contents.  It is possible to destroy critical information
 * with this command causing unknown consequences. In addition, it is possible
 * to generate a machine check exception when accessing I/O memory
 * addresses/registers and other types of memory. The user is cautioned to use
 * extreme care.
 *
 *       Note: Valid memory ranges are defined within a hardcoded structure
 * contained in the PSP layer (CFE_PSP_MemoryTable) however, not every address
 * within the defined ranges may be valid.
 *
 *  \sa #MM_DUMP_MEM_TO_FILE_CC
 */
#define MM_LOAD_MEM_FROM_FILE_CC MM_CCVAL(LOAD_MEM_FROM_FILE)

/**
 * \brief Memory Dump To File
 *
 *  \par Description
 *       Dumps the input number of bytes from processor memory
 *       to a file
 *
 *  \par Command Structure
 *       #MM_DumpMemToFileCmd_t
 *
 *  \par Command Verification
 *       Successful execution of this command may be verified with
 *       the following telemetry:
 *       - #MM_HkTlm_Payload_t.CommandCounter will increment
 *       - #MM_HkTlm_Payload_t.LastAction will be set to
 * #MM_LastAction_DUMP_TO_FILE
 *       - #MM_HkTlm_Payload_t.MemType will be set to the commanded memory type
 *       - #MM_HkTlm_Payload_t.Address will be set to the fully resolved source
 * memory address
 *       - #MM_HkTlm_Payload_t.BytesProcessed will be set to the number of bytes
 * dumped
 *       - #MM_HkTlm_Payload_t.FileName will be set to the dump file name
 *       - The #MM_DMP_MEM_FILE_INF_EID informational event message will
 *         be generated
 *
 *  \par Error Conditions
 *       This command may fail for the following reason(s):
 *       - Command packet length not as expected
 *       - Command specified filename is invalid
 *       - A symbol name was specified that can't be resolved
 *       - #OS_OpenCreate call fails
 *       - #CFE_FS_WriteHeader call fails
 *       - #OS_close call fails
 *       - #OS_write doesn't write the expected number of bytes
 *         or returns an error code
 *       - The #MM_ComputeCRCFromFile call fails
 *       - The address range fails validation check
 *       - The specified data size is invalid
 *       - The address and data size are not properly aligned
 *       - The specified memory type is invalid
 *
 *  \par Evidence of failure may be found in the following telemetry:
 *       - #MM_HkTlm_Payload_t.CommandErrorCounter will increment
 *       - Error specific event message #MM_CMD_LEN_ERR_EID
 *       - Error specific event message #MM_SYMNAME_ERR_EID
 *       - Error specific event message #MM_OS_CREAT_ERR_EID
 *       - Error specific event message #MM_CFE_FS_WRITEHDR_ERR_EID
 *       - Error specific event message #MM_OS_CLOSE_ERR_EID
 *       - Error specific event message #MM_OS_WRITE_EXP_ERR_EID
 *       - Error specific event message #MM_COMPUTECRCFROMFILE_ERR_EID
 *       - Error specific event message #MM_OS_MEMVALIDATE_ERR_EID
 *       - Error specific event message #MM_DATA_SIZE_BYTES_ERR_EID
 *       - Error specific event message #MM_ALIGN32_ERR_EID
 *       - Error specific event message #MM_ALIGN16_ERR_EID
 *       - Error specific event message #MM_MEMTYPE_ERR_EID
 *
 *  \par Criticality
 *       It is the responsibility of the user to verify the
 * <i>SrcSymAddress</i>, <i>NumOfBytes</i>, and <i>MemType</i> in the command.
 * It is possible to generate a machine check exception when accessing I/O
 * memory addresses/registers and other types of memory.  The user is cautioned
 * to use extreme care.
 *
 *       Note: Valid memory ranges are defined within a hardcoded structure
 * contained in the PSP layer (CFE_PSP_MemoryTable) however, not every address
 * within the defined ranges may be valid.
 *
 *  \sa #MM_LOAD_MEM_FROM_FILE_CC
 */
#define MM_DUMP_MEM_TO_FILE_CC MM_CCVAL(DUMP_MEM_TO_FILE)

/**
 * \brief Dump In Event Message
 *
 *  \par Description
 *       Dumps up to #MM_INTERNAL_MAX_DUMP_INEVENT_BYTES of memory in an event
 * message
 *
 *  \par Command Structure
 *       #MM_DumpInEventCmd_t
 *
 *  \par Command Verification
 *       Successful execution of this command may be verified with
 *       the following telemetry:
 *       - #MM_HkTlm_Payload_t.CommandCounter will increment
 *       - #MM_HkTlm_Payload_t.LastAction will be set to
 * #MM_LastAction_DUMP_INEVENT
 *       - #MM_HkTlm_Payload_t.MemType will be set to the commanded memory type
 *       - #MM_HkTlm_Payload_t.Address will be set to the fully resolved source
 * memory address
 *       - #MM_HkTlm_Payload_t.BytesProcessed will be set to the number of bytes
 * dumped
 *       - The #MM_DUMP_INEVENT_INF_EID informational event message will
 *         be generated with the dump data
 *
 *  \par Error Conditions
 *       This command may fail for the following reason(s):
 *       - Command packet length not as expected
 *       - A symbol name was specified that can't be resolved
 *       - The address range fails validation check
 *       - The specified data size is invalid
 *       - The address and data size are not properly aligned
 *       - The specified memory type is invalid
 *
 *  \par Evidence of failure may be found in the following telemetry:
 *       - #MM_HkTlm_Payload_t.CommandErrorCounter will increment
 *       - Error specific event message #MM_CMD_LEN_ERR_EID
 *       - Error specific event message #MM_SYMNAME_ERR_EID
 *       - Error specific event message #MM_OS_MEMVALIDATE_ERR_EID
 *       - Error specific event message #MM_DATA_SIZE_BYTES_ERR_EID
 *       - Error specific event message #MM_ALIGN32_ERR_EID
 *       - Error specific event message #MM_ALIGN16_ERR_EID
 *       - Error specific event message #MM_MEMTYPE_ERR_EID
 *
 *  \par Criticality
 *       It is the responsibility of the user to verify the
 * <i>SrcSymAddress</i>, <i>NumOfBytes</i>, and <i>MemType</i> in the command.
 * It is possible to generate a machine check exception when accessing I/O
 * memory addresses/registers and other types of memory.  The user is cautioned
 * to use extreme care.
 *
 *       Note: Valid memory ranges are defined within a hardcoded structure
 * contained in the PSP layer (CFE_PSP_MemoryTable) however, not every address
 * within the defined ranges may be valid.
 *
 */
#define MM_DUMP_IN_EVENT_CC MM_CCVAL(DUMP_IN_EVENT)

/**
 * \brief Memory Fill
 *
 *  \par Description
 *       Reprograms processor memory with the fill pattern contained
 *       within the command message
 *
 *  \par Command Structure
 *       #MM_FillMemCmd_t
 *
 *  \par Command Verification
 *       Successful execution of this command may be verified with
 *       the following telemetry:
 *       - #MM_HkTlm_Payload_t.CommandCounter will increment
 *       - #MM_HkTlm_Payload_t.LastAction will be set to #MM_LastAction_FILL
 *       - #MM_HkTlm_Payload_t.MemType will be set to the commanded memory type
 *       - #MM_HkTlm_Payload_t.Address will be set to the fully resolved
 * destination memory address
 *       - #MM_HkTlm_Payload_t.DataValue will be set to the fill pattern used
 *       - #MM_HkTlm_Payload_t.BytesProcessed will be set to the number of bytes
 * filled
 *       - The #MM_FILL_INF_EID informational event message will
 *         be generated when the command is executed
 *
 *  \par Error Conditions
 *       This command may fail for the following reason(s):
 *       - Command packet length not as expected
 *       - A symbol name was specified that can't be resolved
 *       - The address range fails validation check
 *       - The specified data size is invalid
 *       - The address and data size are not properly aligned
 *       - The specified memory type is invalid
 *
 *  \par Evidence of failure may be found in the following telemetry:
 *       - #MM_HkTlm_Payload_t.CommandErrorCounter will increment
 *       - Error specific event message #MM_CMD_LEN_ERR_EID
 *       - Error specific event message #MM_SYMNAME_ERR_EID
 *       - Error specific event message #MM_OS_MEMVALIDATE_ERR_EID
 *       - Error specific event message #MM_DATA_SIZE_BYTES_ERR_EID
 *       - Error specific event message #MM_ALIGN32_ERR_EID
 *       - Error specific event message #MM_ALIGN16_ERR_EID
 *       - Error specific event message #MM_MEMTYPE_ERR_EID
 *
 *  \par Criticality
 *       It is the responsibility of the user to verify the
 * <i>DestSymAddress</i>, and <i>NumOfBytes</i> in the command.  It is highly
 * recommended to verify the success or failure of the memory fill.  The fill
 * may be verified by dumping memory and evaluating the dump contents.  It is
 * possible to destroy critical information with this command causing unknown
 * consequences.  In addition, it is possible to generate a machine check
 * exception when accessing I/O memory addresses/registers and other types of
 * memory. The user is cautioned to use extreme care.
 *
 *       Note: Valid memory ranges are defined within a hardcoded structure
 * contained in the PSP layer (CFE_PSP_MemoryTable) however, not every address
 * within the defined ranges may be valid.
 *
 */
#define MM_FILL_MEM_CC MM_CCVAL(FILL_MEM)

/**
 * \brief Symbol Table Lookup
 *
 *  \par Description
 *       Queries the system symbol table and reports the resolved address
 *       in telemetry and an informational event message
 *
 *  \par Command Structure
 *       #MM_LookupSymCmd_t
 *
 *  \par Command Verification
 *       Successful execution of this command may be verified with
 *       the following telemetry:
 *       - #MM_HkTlm_Payload_t.CommandCounter will increment
 *       - #MM_HkTlm_Payload_t.LastAction will be set to
 * #MM_LastAction_SYM_LOOKUP
 *       - #MM_HkTlm_Payload_t.Address will be set to the fully resolved memory
 * address
 *
 *  \par Error Conditions
 *       This command may fail for the following reason(s):
 *       - Command packet length not as expected
 *       - An empty string was specified as the symbol name
 *       - A symbol name was specified that can't be resolved
 *
 *  \par Evidence of failure may be found in the following telemetry:
 *       - #MM_HkTlm_Payload_t.CommandErrorCounter will increment
 *       - Error specific event message #MM_CMD_LEN_ERR_EID
 *       - Error specific event message #MM_SYMNAME_NUL_ERR_EID
 *       - Error specific event message #MM_SYMNAME_ERR_EID
 *
 *  \par Criticality
 *       None
 *
 *  \sa #MM_SYM_TBL_TO_FILE_CC
 */
#define MM_LOOKUP_SYM_CC MM_CCVAL(LOOKUP_SYM)

/**
 * \brief Save Symbol Table To File
 *
 *  \par Description
 *       Saves the system symbol table to a file that can be transfered
 *       to the ground
 *
 *  \par Command Structure
 *       #MM_SymTblToFileCmd_t
 *
 *  \par Command Verification
 *       Successful execution of this command may be verified with
 *       the following telemetry:
 *       - #MM_HkTlm_Payload_t.CommandCounter will increment
 *       - #MM_HkTlm_Payload_t.LastAction will be set to
 * #MM_LastAction_SYMTBL_SAVE
 *       - #MM_HkTlm_Payload_t.FileName will be set to the dump file name
 *       - The #MM_SYMTBL_TO_FILE_INF_EID informational event message will
 *         be generated when the command is executed
 *
 *  \par Error Conditions
 *       This command may fail for the following reason(s):
 *       - Command packet length not as expected
 *       - An empty string was specified as the dump filename
 *       - The OSAL returns a status other than success to the command
 *
 *  \par Evidence of failure may be found in the following telemetry:
 *       - #MM_HkTlm_Payload_t.CommandErrorCounter will increment
 *       - Error specific event message #MM_CMD_LEN_ERR_EID
 *       - Error specific event message #MM_SYMFILENAME_NUL_ERR_EID
 *       - Error specific event message #MM_SYMTBL_TO_FILE_FAIL_ERR_EID
 *
 *  \par Note:
 *       - Dump filenames #CFE_MISSION_MAX_PATH_LEN characters or longer are
 * truncated
 *
 *  \par Criticality
 *       None
 *
 *  \sa #MM_LOOKUP_SYM_CC
 */
#define MM_SYM_TBL_TO_FILE_CC MM_CCVAL(SYMTBL_TO_FILE)

/**
 * \brief EEPROM Write Enable
 *
 *  \par Description
 *       Enables writing to a specified EEPROM bank
 *
 *  \par Command Structure
 *       #MM_EepromWriteEnaCmd_t
 *
 *  \par Command Verification
 *       Successful execution of this command may be verified with
 *       the following telemetry:
 *       - #MM_HkTlm_Payload_t.CommandCounter will increment
 *       - #MM_HkTlm_Payload_t.LastAction will be set to
 * #MM_LastAction_EEPROMWRITE_ENA
 *       - The #MM_EEPROM_WRITE_ENA_INF_EID informational event message will
 *         be generated when the command is executed
 *
 *  \par Error Conditions
 *       This command may fail for the following reason(s):
 *       - Command packet length not as expected
 *       - Non-success return status from PSP write enable
 *
 *  \par Evidence of failure may be found in the following telemetry:
 *       - #MM_HkTlm_Payload_t.CommandErrorCounter will increment
 *       - Error specific event message #MM_CMD_LEN_ERR_EID
 *       - Error specific event message #MM_EEPROM_WRITE_ENA_ERR_EID
 *
 *  \par Criticality
 *       Extreme caution is advised in the use of this command. It is intended
 * to be used only as a maintence tool for patching the default FSW image. This
 * command will leave the EEPROM bank in a very vulnerable state. Once a patch
 * has been completed the #MM_EEPROM_WRITE_DIS_CC command must be issued to
 * protect the EEPROM bank from being inadvertently written.
 *
 *  \sa #MM_EEPROM_WRITE_DIS_CC
 */
#define MM_EEPROM_WRITE_ENA_CC MM_CCVAL(ENABLE_EEPROM_WRITE)

/**
 * \brief EEPROM Write Disable
 *
 *  \par Description
 *       Disables writing to a specified EEPROM bank
 *
 *  \par Command Structure
 *       #MM_EepromWriteDisCmd_t
 *
 *  \par Command Verification
 *       Successful execution of this command may be verified with
 *       the following telemetry:
 *       - #MM_HkTlm_Payload_t.CommandCounter will increment
 *       - #MM_HkTlm_Payload_t.LastAction will be set to
 * #MM_LastAction_EEPROMWRITE_DIS
 *       - The #MM_EEPROM_WRITE_DIS_INF_EID informational event message will
 *         be generated when the command is executed
 *
 *  \par Error Conditions
 *       This command may fail for the following reason(s):
 *       - Command packet length not as expected
 *       - Non-success return status from PSP write disable
 *
 *  \par Evidence of failure may be found in the following telemetry:
 *       - #MM_HkTlm_Payload_t.CommandErrorCounter will increment
 *       - Error specific event message #MM_CMD_LEN_ERR_EID
 *       - Error specific event message #MM_EEPROM_WRITE_DIS_ERR_EID
 *
 *  \par Criticality
 *       None
 *
 *  \sa #MM_EEPROM_WRITE_ENA_CC
 */
#define MM_EEPROM_WRITE_DIS_CC MM_CCVAL(DISABLE_EEPROM_WRITE)

/** \} */

#endif /* MM_FCNCODES_H */
