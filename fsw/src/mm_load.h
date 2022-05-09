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
 *   Specification for the CFS Memory Manager routines that process
 *   memory load and fill ground commands
 */
#ifndef MM_LOAD_H
#define MM_LOAD_H

/*************************************************************************
 * Includes
 *************************************************************************/
#include "cfe.h"
#include "mm_msg.h"
#include "mm_filedefs.h"

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
bool MM_PokeMem(const MM_PokeCmd_t *CmdPtr, cpuaddr DestAddress);

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
 */
bool MM_PokeEeprom(const MM_PokeCmd_t *CmdPtr, cpuaddr DestAddress);

/**
 * \brief Load memory with interrupts disabled
 *
 *  \par Description
 *       Support function for #MM_LoadMemWIDCmd. This routine will
 *       load up to #MM_MAX_UNINTERRUPTIBLE_DATA bytes into
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
 *  \return Boolean execution status
 *  \retval true Load from file successful
 *  \retval false Load from file failed
 */
bool MM_LoadMemFromFile(osal_id_t FileHandle, const char *FileName, const MM_LoadDumpFileHeader_t *FileHeader,
                        cpuaddr DestAddress);

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
 *  \return Boolean load file size verification result
 *  \retval true  Load file size verification success
 *  \retval false Load file size verification failure
 */
bool MM_VerifyLoadFileSize(const char *FileName, const MM_LoadDumpFileHeader_t *FileHeader);

/**
 * \brief Read the cFE primary and and MM secondary file headers
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
 *  \param [in]   CFEHeader    Pointer to CFE file header
 *  \param [in]   MMHeader     Pointer to MM file header
 *  \param [out]  *CFEHeader   Contents of the cFE primary file header
 *                             structure for the specified file.
 *  \param [out]  *MMHeader    Contents of the MM secondary file header
 *                             structure for the specified file.
 *
 *  \return Boolean execution status
 *  \retval true  Headers read successfully
 *  \retval false Headers read failed
 */
bool MM_ReadFileHeaders(const char *FileName, osal_id_t FileHandle, CFE_FS_Header_t *CFEHeader,
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
 *  \param [in]   DestAddr   The destination address for the fill
 *                           operation
 *  \param [in]   CmdPtr     Pointer to command
 */
bool MM_FillMem(cpuaddr DestAddr, const MM_FillMemCmd_t *CmdPtr);

/**
 * \brief Process memory poke command
 *
 *  \par Description
 *       Processes the memory poke command that will load a memory
 *       location with data specified in the command message.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param [in]   BufPtr   Pointer to Software Bus buffer
 *
 *  \sa #MM_POKE_CC
 */
bool MM_PokeCmd(const CFE_SB_Buffer_t *BufPtr);

/**
 * \brief Process load memory with interrupts disabled command
 *
 *  \par Description
 *       Processes the load memory with interrupts disabled command
 *       that will load up to #MM_MAX_UNINTERRUPTIBLE_DATA bytes into
 *       ram with interrupts disabled during the actual load
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param [in]   BufPtr   Pointer to Software Bus buffer
 *
 *  \sa #MM_LOAD_MEM_WID_CC
 */
bool MM_LoadMemWIDCmd(const CFE_SB_Buffer_t *BufPtr);

/**
 * \brief Process memory load from file command
 *
 *  \par Description
 *       Processes the memory load from file command that will read a
 *       file and store the data in the command specified address range
 *       of memory.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param [in]   BufPtr   Pointer to Software Bus buffer
 *
 *  \sa #MM_LOAD_MEM_FROM_FILE_CC
 */
bool MM_LoadMemFromFileCmd(const CFE_SB_Buffer_t *BufPtr);

/**
 * \brief Process memory fill command
 *
 *  \par Description
 *       Processes the memory fill command that will load an address
 *       range of memory with the command specified fill pattern
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param [in]   BufPtr   Pointer to Software Bus buffer
 *
 *  \sa #MM_FILL_MEM_CC
 */
bool MM_FillMemCmd(const CFE_SB_Buffer_t *BufPtr);

#endif
