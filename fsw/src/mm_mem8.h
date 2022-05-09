/*************************************************************************
** File: mm_mem8.h 
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
**   Specification for the CFS Memory Manager functions that are used
**   for the conditionally compiled MM_MEM8 optional memory type.
**
*************************************************************************/
#ifndef _mm_mem8_
#define _mm_mem8_

/*************************************************************************
** Includes
*************************************************************************/
#include "mm_filedefs.h"

/*************************************************************************
** Exported Functions
*************************************************************************/
/************************************************************************/
/** \brief Memory8 load from file
**
**  \par Description
**       Support function for #MM_LoadMemFromFileCmd. This routine will
**       read a file and write the data to memory that is defined to
**       only be 8 bit accessible
**
**  \par Assumptions, External Events, and Notes:
**       This function is specific to the optional #MM_MEM8 memory
**       type
**
**  \param [in]   FileHandle   The open file handle of the load file
**
**  \param [in]   FileName     A pointer to a character string holding
**                             the load file name
**
**  \param [in]   FileHeader   A #MM_LoadDumpFileHeader_t pointer to
**                             the load file header structure
**
**  \param [in]   DestAddress  The destination address for the requested
**                             load operation
**
**  \returns
**  \retstmt Returns TRUE if the load completed successfully  \endcode
**  \retstmt Returns FALSE if the load failed due to an error \endcode
**  \endreturns
**
*************************************************************************/
bool MM_LoadMem8FromFile(uint32 FileHandle, const char *FileName, const MM_LoadDumpFileHeader_t *FileHeader,
                         cpuaddr DestAddress);

/************************************************************************/
/** \brief Memory8 dump to file
**
**  \par Description
**       Support function for #MM_DumpMemToFileCmd. This routine will
**       read an address range that is defined to only be 8 bit
**       accessible and store the data in a file
**
**  \par Assumptions, External Events, and Notes:
**       This function is specific to the optional #MM_MEM8 memory
**       type
**
**  \param [in]   FileHandle   The open file handle of the dump file
**
**  \param [in]   FileName     A pointer to a character string holding
**                             the dump file name
**
**  \param [in]   FileHeader   A #MM_LoadDumpFileHeader_t pointer to
**                             the dump file header structure initialized
**                             with data based upon the command message
**                             parameters
**
**  \returns
**  \retstmt Returns TRUE if the dump completed successfully  \endcode
**  \retstmt Returns FALSE if the dump failed due to an error \endcode
**  \endreturns
**
*************************************************************************/
bool MM_DumpMem8ToFile(uint32 FileHandle, const char *FileName, const MM_LoadDumpFileHeader_t *FileHeader);

/************************************************************************/
/** \brief Fill memory8
**
**  \par Description
**       Support function for #MM_FillMemCmd. This routine will
**       load memory that is defined to only be 8 bit accessible
**       with a command specified fill pattern
**
**  \par Assumptions, External Events, and Notes:
**       This function is specific to the optional #MM_MEM8 memory
**       type
**
**  \param [in]   DestAddress   The destination address for the fill
**                              operation
**
**  \param [in]   CmdPtr     A #MM_FillMemCmd_t pointer to the fill
**                           command message
**
*************************************************************************/
bool MM_FillMem8(cpuaddr DestAddress, const MM_FillMemCmd_t *CmdPtr);

#endif /* _mm_mem8_ */

/************************/
/*  End of File Comment */
/************************/
