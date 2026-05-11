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
 *   Specification for the CFS Memory Manager memory dump ground commands.
 */
#ifndef MM_DUMP_H
#define MM_DUMP_H

/*************************************************************************
 * Includes
 *************************************************************************/
#include "cfe.h"
#include "mm_filedefs.h"
#include "mm_msg.h"

/*************************************************************************
 * Exported Functions
 *************************************************************************/

/**
 * \brief Memory peek
 *
 * \par Description
 *      Support function for #MM_PeekCmd. This routine will read
 *      8, 16, or 32 bits of data and send it in an event message.
 *
 * \par Assumptions, External Events, and Notes:
 *      None
 *
 * \param [in]   CmdPtr       Pointer to the command
 * \param [in]   SrcAddress   The source address for the peek operation
 *
 * \return Execution status
 */
int32 MM_PeekMem(const MM_PeekCmd_t *CmdPtr, cpuaddr SrcAddress);

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
 *  \param [in]   FileHeader   Pointer to the dump file header structure
 * initialized
 *
 *  \return Execution status
 */
int32 MM_DumpMemToFile(osal_id_t FileHandle, const char *FileName, const MM_LoadDumpFileHeader_t *FileHeader);

/**
 * \brief Write the cFE primary and MM secondary file headers
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
 *  \param [in]   CFEHeader    Pointer to cFE primary file header structure to
 * be written.
 *  \param [in]   MMHeader     Pointer to MM secondary file header structure
 *                             to be written.
 *
 *  \return Execution status
 */
int32 MM_WriteFileHeaders(const char                    *FileName,
                          osal_id_t                      FileHandle,
                          CFE_FS_Header_t               *CFEHeader,
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
 *  \param [out]  DumpBuffer   Byte array holding the dump data
 *
 *  \return Execution status
 */
int32 MM_FillDumpInEventBuffer(cpuaddr SrcAddress, const MM_DumpInEventCmd_t *CmdPtr, void *DumpBuffer);

#endif
