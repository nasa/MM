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
 *   Specification for the CFS Memory Manager functions that are used
 *   for the conditionally compiled MM_MEM32 optional memory type.
 */
#ifndef MM_MEM32_H
#define MM_MEM32_H

/*************************************************************************
 * Includes
 *************************************************************************/
#include "mm_filedefs.h"

/*************************************************************************
 * Exported Functions
 *************************************************************************/

/**
 * \brief Memory32 load from file
 *
 *  \par Description
 *       Support function for #MM_LoadMemFromFileCmd. This routine will
 *       read a file and write the data to memory that is defined to
 *       only be 32 bit accessible
 *
 *  \par Assumptions, External Events, and Notes:
 *       This function is specific to the optional #MM_MEM32 memory
 *       type
 *
 *  \param [in]   FileHandle   The open file handle of the load file
 *  \param [in]   FileName     A pointer to a character string holding
 *                             the load file name
 *  \param [in]   FileHeader   Pointer to file header
 *  \param [in]   DestAddress  The destination address for the requested
 *                             load operation
 *
 *  \return Boolean execution status
 *  \retval true  Load successful
 *  \retval false Load failed
 */
bool MM_LoadMem32FromFile(osal_id_t FileHandle, const char *FileName, const MM_LoadDumpFileHeader_t *FileHeader,
                          cpuaddr DestAddress);

/**
 * \brief Memory32 dump to file
 *
 *  \par Description
 *       Support function for #MM_DumpMemToFileCmd. This routine will
 *       read an address range that is defined to only be 32 bit
 *       accessible and store the data in a file
 *
 *  \par Assumptions, External Events, and Notes:
 *       This function is specific to the optional #MM_MEM32 memory
 *       type
 *
 *  \param [in]   FileHandle   The open file handle of the dump file
 *  \param [in]   FileName     A pointer to a character string holding
 *                             the dump file name
 *  \param [in]   FileHeader   Pointer to file header
 *
 *  \return Boolean execution status
 *  \retval true  Dump successful
 *  \retval false Dump failed
 */
bool MM_DumpMem32ToFile(osal_id_t FileHandle, const char *FileName, const MM_LoadDumpFileHeader_t *FileHeader);

/**
 * \brief Fill memory32
 *
 *  \par Description
 *       Support function for #MM_FillMemCmd. This routine will
 *       load memory that is defined to only be 32 bit accessible
 *       with a command specified fill pattern
 *
 *  \par Assumptions, External Events, and Notes:
 *       This function is specific to the optional #MM_MEM32 memory
 *       type
 *
 *  \param [in]   DestAddress   The destination address for the fill
 *                              operation
 *  \param [in]   CmdPtr        Pointer to command
 */
bool MM_FillMem32(cpuaddr DestAddress, const MM_FillMemCmd_t *CmdPtr);

#endif
