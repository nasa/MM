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
 *   Specification for the CFS Memory Manager utility functions.
 */
#ifndef MM_UTILS_H
#define MM_UTILS_H

/*************************************************************************
 * Includes
 *************************************************************************/
#include "mm_msg.h"

/*************************************************************************
 * Exported Functions
 *************************************************************************/

/**
 * \brief Reset housekeeping variables
 *
 *  \par Description
 *       Sets the local memory manager housekeeping variables to default
 *       values. This routine gets called before each command is
 *       processed to verify that all the variables are properly cleared
 *
 *  \par Assumptions, External Events, and Notes:
 *       This function does not zero the command execution counter
 *       or the command error counter
 */
void MM_ResetHk(void);

/**
 * \brief Segment break
 *
 *  \par Description
 *       This routine gets called during each segment break in a load,
 *       dump, or memory fill operation and handles any processing
 *       that needs to be done during those breaks
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 */
void MM_SegmentBreak(void);

/**
 * \brief Verify command message length
 *
 *  \par Description
 *       This routine will check if the actual length of a software bus
 *       command message matches the expected length and send an
 *       error event message if a mismatch occurs
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param [in]   MsgPtr           Pointer to message
 *  \param [in]   ExpectedLength   The expected length of the message
 *                                 based upon the command code
 *
 *  \return Boolean length validation status
 *  \retval true  Length matches expected
 *  \retval false Length does not match expected
 *
 *  \sa #MM_LEN_ERR_EID
 */
bool MM_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength);

/**
 * \brief Verify memory peek and poke parameters
 *
 *  \par Description
 *       This routine will run various checks on the specified address,
 *       memory type, and data size (in bits) for a memory peek or
 *       memory poke command
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param [in]   Address      The source or destination address for the
 *                             requested peek or poke operation
 *  \param [in]   MemType      The source or destination memory type for
 *                             the requested peek or poke operation
 *  \param [in]   SizeInBits   The bit width for the requested
 *                             peek or poke operation
 *
 *  \return Boolean peek/poke parameter validation status
 *  \retval true  Validation passed
 *  \retval false Validation failed
 */
bool MM_VerifyPeekPokeParams(cpuaddr Address, MM_MemType_t MemType, size_t SizeInBits);

/**
 * \brief Verify memory load and dump parameters
 *
 *  \par Description
 *       This routine will run various checks on the specified address,
 *       memory type, and data size (in bits) for a memory load or
 *       memory dump command.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param [in]   Address      The source or destination address for the
 *                             requested load or dump operation
 *  \param [in]   MemType      The source or destination memory type for
 *                             the requested load or dump operation
 *  \param [in]   SizeInBytes  Number of bytes for load or dump operation
 *  \param [in]   VerifyType   Flag indicating whether the requested
 *                             operation is a load or a dump.
 *
 *  \return Boolean load/dump parameter validation status
 *  \retval true  Validation passed
 *  \retval false Validation failed
 */
bool MM_VerifyLoadDumpParams(cpuaddr Address, MM_MemType_t MemType, size_t SizeInBytes, uint8 VerifyType);

/**
 * \brief Verify 32 bit alignment
 *
 *  \par Description
 *       This routine will check an address and data size argument pair
 *       for correct 32 bit alignment
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param [in]   Address   The address to check for proper alignment
 *
 *  \param [in]   Size      The size in bytes to check for proper
 *                          alignment
 *
 *  \return Boolean alignment validation status
 *  \retval true  Validation passed
 *  \retval false Validation failed
 *
 *  \sa #MM_Verify16Aligned
 */
bool MM_Verify32Aligned(cpuaddr Address, size_t Size);

/**
 * \brief Verify 16 bit alignment
 *
 *  \par Description
 *       This routine will check an address and data size argument pair
 *       for correct 16 bit alignment
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param [in]   Address   The address to check for proper alignment
 *  \param [in]   Size      The size in bytes to check for proper
 *                          alignment
 *
 *  \return Boolean alignment validation status
 *  \retval true  Validation passed
 *  \retval false Validation failed
 *
 *  \sa #MM_Verify32Aligned
 */
bool MM_Verify16Aligned(cpuaddr Address, size_t Size);

/**
 * \brief Resolve symbolic address
 *
 *  \par Description
 *       This routine will resolve a symbol name and optional address
 *       offset to an absolute address
 *
 *  \par Assumptions, External Events, and Notes:
 *       If the symbol name is a NUL (empty) string, then the offset
 *       becomes the absolute address
 *
 *  \param [in]   SymAddr          A #MM_SymAddr_t pointer that holds
 *                                 the symbol name and optional offset
 *  \param [out]  ResolvedAddr     The fully resolved address. Only valid
 *                                 if the return value is TRUE
 *
 *  \return Boolean execution status
 *  \retval true  Symbolic address resolved
 *  \retval false Symbolic address not resolved
 *
 *  \sa #OS_SymbolLookup
 */
bool MM_ResolveSymAddr(MM_SymAddr_t *SymAddr, cpuaddr *ResolvedAddr);

/**
 * \brief Compute CRC from a file
 *
 *  \par Description
 *       This function will compute the cyclic redundancy check (CRC)
 *       value of data in a file
 *
 *  \par Assumptions, External Events, and Notes:
 *       The computation starts from the current location of the
 *       file pointer to the end of file
 *
 *  \param [in]   FileHandle   The open file handle of the file to scan
 *  \param [in]   TypeCRC      CRC type to compute
 *  \param [out]  CrcPtr       The computed CRC. Only updated if the return
 *                             value is #OS_SUCCESS
 *
 *  \return Execution status, see \ref OSReturnCodes
 *  \retval #OS_SUCCESS \copybrief OS_SUCCESS
 *
 *  \sa #CFE_ES_CalculateCRC, #OS_read
 */
int32 MM_ComputeCRCFromFile(osal_id_t FileHandle, uint32 *CrcPtr, uint32 TypeCRC);

#endif
