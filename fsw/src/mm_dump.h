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
 *   Specification for the CFS Memory Manager memory dump ground commands.
 */
#ifndef MM_DUMP_H
#define MM_DUMP_H

/*************************************************************************
 * Includes
 *************************************************************************/
#include "cfe.h"
#include "mm_msg.h"
#include "mm_filedefs.h"

/************************************************************************
 * Macro Definitions
 ************************************************************************/

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
 *    Message tail "from address: 0xFFFFFFFF"  33 characters including NUL on 64-bit system
 */
#define MM_MAX_DUMP_INEVENT_BYTES ((CFE_MISSION_EVS_MAX_MESSAGE_LENGTH - (13 + 33)) / 5)

/**
 * \brief Dump in an event scratch string size
 *
 * This macro defines the size of the scratch buffer used to build
 * the dump in event message string. Set it to the size of the
 * largest piece shown above including room for a NUL terminator.
 */
#define MM_DUMPINEVENT_TEMP_CHARS 36

/*************************************************************************
 * Exported Functions
 *************************************************************************/

/**
 * \brief Memory peek
 *
 *  \par Description
 *       Support function for #MM_PeekCmd. This routine will read
 *       8, 16, or 32 bits of data and send it in an event message.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param [in]   CmdPtr       Pointer to the command
 *  \param [in]   SrcAddress   The source address for the peek operation
 */
bool MM_PeekMem(const MM_PeekCmd_t *CmdPtr, cpuaddr SrcAddress);

/**
 * \brief Memory dump to file
 *
 *  \par Description
 *       Support function for #MM_DumpMemToFileCmd. This routine will
 *       read an address range and store the data in a file.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param [in]   FileHandle   The open file handle of the dump file
 *  \param [in]   FileName     A pointer to a character string holding
 *                             the dump file name
 *  \param [in]   FileHeader   Pointer to the dump file header structure initialized
 *
 *  \return Boolean execution status
 *  \retval true Dump completed successfully
 *  \retval false Dump failed
 */
bool MM_DumpMemToFile(osal_id_t FileHandle, const char *FileName, const MM_LoadDumpFileHeader_t *FileHeader);

/**
 * \brief Write the cFE primary and and MM secondary file headers
 *
 *  \par Description
 *       Support function for #MM_DumpMemToFileCmd. This routine will
 *       write the cFE primary and MM secondary headers to the
 *       file specified by the FileHandle.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param [in]   FileName     A pointer to a character string holding
 *                             the file name (used only for error event
 *                             messages).
 *  \param [in]   FileHandle   File Descriptor to write headers to
 *  \param [in]   CFEHeader    Pointer to cFE primary file header structure to be
 *                             written.
 *  \param [in]   MMHeader     Pointer to MM secondary file header structure
 *                             to be written.
 *
 *  \return Boolean execution status
 *  \retval true  Header written successfully
 *  \retval false Header write failed
 */
bool MM_WriteFileHeaders(const char *FileName, osal_id_t FileHandle, CFE_FS_Header_t *CFEHeader,
                         const MM_LoadDumpFileHeader_t *MMHeader);

/**
 * \brief Fill dump memory in event message buffer
 *
 *  \par Description
 *       Support function for #MM_DumpInEventCmd. This routine will
 *       read an address range and store the data in a byte array.
 *       It will properly adjust for optional memory types that may
 *       require 16 or 32 bit reads.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param [in]   SrcAddress   The source address to read from
 *  \param [in]   CmdPtr       Pointer to dump in event command message
 *  \param [in]   DumpBuffer   A pointer to the byte array to store
 *                             the dump data in
 *  \param [out]  *DumpBuffer  A pointer to the byte array holding the
 *                             dump data
 *
 *  \return Boolean execution status
 *  \retval true  Dump in event buffer successful
 *  \retval false Dump in event buffer failed
 */
bool MM_FillDumpInEventBuffer(cpuaddr SrcAddress, const MM_DumpInEventCmd_t *CmdPtr, void *DumpBuffer);

/**
 * \brief Process memory peek command
 *
 *  \par Description
 *       Processes the memory peek command that will read a memory
 *       location and report the data in an event message.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param[in] BufPtr Pointer to Software Bus buffer
 *
 *  \sa #MM_PEEK_CC
 */
bool MM_PeekCmd(const CFE_SB_Buffer_t *BufPtr);

/**
 * \brief Process memory dump to file command
 *
 *  \par Description
 *       Processes the memory dump to file command that will read a
 *       address range of memory and store the data in a command
 *       specified file.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param[in] BufPtr Pointer to Software Bus buffer
 *
 *  \sa #MM_DUMP_MEM_TO_FILE_CC
 */
bool MM_DumpMemToFileCmd(const CFE_SB_Buffer_t *BufPtr);

/**
 * \brief Process memory dump in event command
 *
 *  \par Description
 *       Processes the memory dump in event command that will read
 *       up to #MM_MAX_DUMP_INEVENT_BYTES from memory and report
 *       the data in an event message.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param[in] BufPtr Pointer to Software Bus buffer
 *
 *  \sa #MM_DUMP_IN_EVENT_CC, #MM_MAX_DUMP_INEVENT_BYTES
 */
bool MM_DumpInEventCmd(const CFE_SB_Buffer_t *BufPtr);

#endif
