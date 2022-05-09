/*************************************************************************
** File: mm_utils.h 
**
**   Copyright © 2007-2014 United States Government as represented by the
**   Administrator of the National Aeronautics and Space Administration.
**   All Other Rights Reserved.
**
**   This software was created at NASA's Goddard Space Flight Center.
**   This software is governed by the NASA Open Source Agreement and may be
**   used, distributed and modified only pursuant to the terms of that
**   agreement.
**
** Purpose:
**   Specification for the CFS Memory Manager utility functions.
**
*************************************************************************/
#ifndef _mm_utils_
#define _mm_utils_

/*************************************************************************
** Includes
*************************************************************************/
#include "mm_msg.h"

/*************************************************************************
** Exported Functions
*************************************************************************/

/************************************************************************/
/** \brief Reset housekeeping variables
**
**  \par Description
**       Sets the local memory manager housekeeping variables to default
**       values. This routine gets called before each command is
**       processed to verify that all the variables are properly cleared
**
**  \par Assumptions, External Events, and Notes:
**       This function does not zero the command execution counter
**       or the command error counter
**
*************************************************************************/
void MM_ResetHk(void);

/************************************************************************/
/** \brief Segment break
**
**  \par Description
**       This routine gets called during each segment break in a load,
**       dump, or memory fill operation and handles any processing
**       that needs to be done during those breaks
**
**  \par Assumptions, External Events, and Notes:
**       None
**
*************************************************************************/
void MM_SegmentBreak(void);

/************************************************************************/
/** \brief Verify command message length
**
**  \par Description
**       This routine will check if the actual length of a software bus
**       command message matches the expected length and send an
**       error event message if a mismatch occurs
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   MsgPtr           A #CFE_MSG_Message_t* pointer that
**                                 references the software bus message
**
**  \param [in]   ExpectedLength   The expected length of the message
**                                 based upon the command code
**
**  \returns
**  \retstmt Returns true if the length is as expected      \endcode
**  \retstmt Returns false if the length is not as expected \endcode
**  \endreturns
**
**  \sa #MM_LEN_ERR_EID
**
*************************************************************************/
bool MM_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength);

/************************************************************************/
/** \brief Verify memory peek and poke parameters
**
**  \par Description
**       This routine will run various checks on the specified address,
**       memory type, and data size (in bits) for a memory peek or
**       memory poke command
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   Address      The source or destination address for the
**                             requested peek or poke operation
**
**  \param [in]   MemType      The source or destination memory type for
**                             the requested peek or poke operation
**
**  \param [in]   SizeInBits   The bit width for the requested
**                             peek or poke operation
**
**  \returns
**  \retstmt Returns true if all the parameter checks passed  \endcode
**  \retstmt Returns false any parameter check failed         \endcode
**  \endreturns
**
*************************************************************************/
bool MM_VerifyPeekPokeParams(cpuaddr Address, uint8 MemType, uint8 SizeInBits);

/************************************************************************/
/** \brief Verify memory load and dump parameters
**
**  \par Description
**       This routine will run various checks on the specified address,
**       memory type, and data size (in bits) for a memory load or
**       memory dump command.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   Address      The source or destination address for the
**                             requested load or dump operation
**
**  \param [in]   MemType      The source or destination memory type for
**                             the requested load or dump operation
**
**  \param [in]   SizeInBits   The bit width for the requested
**                             load or dump operation
**
**  \param [in]   VerifyType   Flag indicating whether the requested
**                             operation is a load or a dump.
**
**  \returns
**  \retstmt Returns true if all the parameter checks passed  \endcode
**  \retstmt Returns false any parameter check failed         \endcode
**  \endreturns
**
*************************************************************************/

bool MM_VerifyLoadDumpParams(cpuaddr Address, uint8 MemType, uint32 SizeInBytes, uint8 VerifyType);

#endif /* _mm_utils_ */

/************************/
/*  End of File Comment */
/************************/
