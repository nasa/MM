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
 *   Specification for the CFS Memory Manager routines that process
 *   memory load and fill ground commands
 */
#ifndef MM_LOAD_H
#define MM_LOAD_H

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
 * \brief Memory poke
 *
 *  \par Description
 *       Support function for #MM_PokeCmd. This routine will write
 *       8, 16, or 32 bits of data to a single ram address.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param [in]   CmdPtr        Pointer to command
 *  \param [in]   DestAddress   The destination address for the poke
 *                              operation
 */
CFE_Status_t MM_PokeMem(const MM_PokeCmd_t *CmdPtr, cpuaddr DestAddress);

/**
 * \brief Eeprom poke
 *
 *  \par Description
 *       Support function for #MM_PokeCmd. This routine will write
 *       8, 16, or 32 bits of data to a single EEPROM address.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param [in]   CmdPtr        Pointer to command
 *  \param [in]   DestAddress   The destination address for the poke
 *                              operation
 *
 * \return Execution status
 */
CFE_Status_t MM_PokeEeprom(const MM_PokeCmd_t *CmdPtr, cpuaddr DestAddress);

/**
 * \brief Load memory with interrupts disabled
 *
 *  \par Description
 *       Support function for #MM_LoadMemWIDCmd. This routine will
 *       load up to #MM_INTERFACE_MAX_UNINTERRUPTIBLE_DATA bytes into
 *       ram with interrupts disabled during the actual load
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param [in]   CmdPtr        Pointer to command
 *  \param [in]   DestAddress   The destination address for the load
 *                              operation
 */
bool MM_LoadMemWID(const MM_LoadMemWIDCmd_t *CmdPtr, cpuaddr DestAddress);

/**
 * \brief Memory load from file
 *
 *  \par Description
 *       Support function for #MM_LoadMemFromFileCmd. This routine will
 *       read a file and write the data to memory
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param [in]   FileHandle   The open file handle of the load file
 *  \param [in]   FileName     A pointer to a character string holding
 *                             the load file name
 *  \param [in]   FileHeader   Pointer to file header
 *  \param [in]   DestAddress  The destination address for the requested
 *                             load operation
 *
 *  \return Execution status
 */
int32 MM_LoadMemFromFile(osal_id_t                      FileHandle,
                         const char                    *FileName,
                         const MM_LoadDumpFileHeader_t *FileHeader,
                         cpuaddr                        DestAddress);

/**
 * \brief Verify load file size
 *
 *  \par Description
 *       Support function for #MM_LoadMemFromFileCmd. This routine will
 *       check if the size of a load file as reported by the filesystem
 *       is what it should be based upon the number of load bytes
 *       specified in the MM file header.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param [in]   FileName     A pointer to a character string holding
 *                             the load file name
 *  \param [in]   FileHeader   Pointer to file header
 *
 *  \return Execution status
 */
int32 MM_VerifyLoadFileSize(const char *FileName, const MM_LoadDumpFileHeader_t *FileHeader);

/**
 * \brief Read the cFE primary and MM secondary file headers
 *
 *  \par Description
 *       Support function for #MM_LoadMemFromFileCmd. This routine will
 *       read the cFE primary and MM secondary headers from the
 *       file specified by the FileHandle.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param [in]   FileName     A pointer to a character string holding
 *                             the file name (used only for error event
 *                             messages).
 *  \param [in]   FileHandle   File handle to read header from
 *  \param [out]  CFEHeader    Contents of the cFE primary file header
 *                             structure for the specified file.
 *  \param [out]  MMHeader     Contents of the MM secondary file header
 *                             structure for the specified file.
 *
 *  \return Execution status
 */
int32 MM_ReadFileHeaders(const char              *FileName,
                         osal_id_t                FileHandle,
                         CFE_FS_Header_t         *CFEHeader,
                         MM_LoadDumpFileHeader_t *MMHeader);

/**
 * \brief Fill memory
 *
 *  \par Description
 *       Support function for #MM_FillMemCmd. This routine will
 *       load memory with a command specified fill pattern
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param [in]   DestAddress The destination address for the fill operation
 *  \param [in]   CmdPtr      Pointer to command
 */
void MM_FillMem(cpuaddr DestAddress, const MM_FillMemCmd_t *CmdPtr);

#endif
