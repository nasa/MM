/*************************************************************************
** File: mm_load.h 
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
**   Specification for the CFS Memory Manager routines that process
**   memory load and fill ground commands
**
**************************************************************************/

#ifndef _mm_load_
#define _mm_load_

/*************************************************************************
** Includes
*************************************************************************/
#include "cfe.h"
#include "mm_msg.h"
#include "mm_filedefs.h"

/*************************************************************************
** Exported Functions
*************************************************************************/
/************************************************************************/
/** \brief Memory poke
**
**  \par Description
**       Support function for #MM_PokeCmd. This routine will write
**       8, 16, or 32 bits of data to a single ram address.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   CmdPtr        A #MM_PokeCmd_t pointer to the poke
**                              command message
**
**  \param [in]   DestAddress   The destination address for the poke
**                              operation
**
*************************************************************************/
bool MM_PokeMem(const MM_PokeCmd_t *CmdPtr, cpuaddr DestAddress);

/************************************************************************/
/** \brief Eeprom poke
**
**  \par Description
**       Support function for #MM_PokeCmd. This routine will write
**       8, 16, or 32 bits of data to a single EEPROM address.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   CmdPtr        A #MM_PokeCmd_t pointer to the poke
**                              command message
**
**  \param [in]   DestAddress   The destination address for the poke
**                              operation
**
*************************************************************************/
bool MM_PokeEeprom(const MM_PokeCmd_t *CmdPtr, cpuaddr DestAddress);

/************************************************************************/
/** \brief Load memory with interrupts disabled
**
**  \par Description
**       Support function for #MM_LoadMemWIDCmd. This routine will
**       load up to #MM_MAX_UNINTERRUPTIBLE_DATA bytes into
**       ram with interrupts disabled during the actual load
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   CmdPtr        A #MM_LoadMemWIDCmd_t pointer to the
**                              command message
**
**  \param [in]   DestAddress   The destination address for the load
**                              operation
**
*************************************************************************/
bool MM_LoadMemWID(const MM_LoadMemWIDCmd_t *CmdPtr, cpuaddr DestAddress);

/************************************************************************/
/** \brief Memory load from file
**
**  \par Description
**       Support function for #MM_LoadMemFromFileCmd. This routine will
**       read a file and write the data to memory
**
**  \par Assumptions, External Events, and Notes:
**       None
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
bool MM_LoadMemFromFile(uint32 FileHandle, const char *FileName, const MM_LoadDumpFileHeader_t *FileHeader,
                        cpuaddr DestAddress);

/************************************************************************/
/** \brief Verify load file size
**
**  \par Description
**       Support function for #MM_LoadMemFromFileCmd. This routine will
**       check if the size of a load file as reported by the filesystem
**       is what it should be based upon the number of load bytes
**       specified in the MM file header.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   FileName     A pointer to a character string holding
**                             the load file name
**
**  \param [in]   FileHeader   A #MM_LoadDumpFileHeader_t pointer to
**                             the load file header structure
**
**  \returns
**  \retstmt Returns TRUE if the load file size is as expected \endcode
**  \retstmt Returns FALSE if the load file size is not as expected \endcode
**  \endreturns
**
*************************************************************************/
bool MM_VerifyLoadFileSize(const char *FileName, const MM_LoadDumpFileHeader_t *FileHeader);

/************************************************************************/
/** \brief Read the cFE primary and and MM secondary file headers
**
**  \par Description
**       Support function for #MM_LoadMemFromFileCmd. This routine will
**       read the cFE primary and MM secondary headers from the
**       file specified by the FileHandle.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   FileName     A pointer to a character string holding
**                             the file name (used only for error event
**                             messages).
**
**  \param [in]   FileHandle   File Descriptor obtained from a previous
**                             call to #OS_open that is associated with
**                             the file whose headers are to be read.
**
**  \param [in]   CFEHeader    A #CFE_FS_Header_t pointer to
**                             the cFE primary file header structure.
**
**  \param [in]   MMHeader     A #MM_LoadDumpFileHeader_t pointer to
**                             the MM secondary file header structure
**
**  \param [out]  *CFEHeader   Contents of the cFE primary file header
**                             structure for the specified file.
**
**  \param [out]  *MMHeader    Contents of the MM secondary file header
**                             structure for the specified file.
**
**  \returns
**  \retstmt Returns TRUE if the headers were read successfully \endcode
**  \retstmt Returns FALSE if a read error occurred \endcode
**  \endreturns
**
*************************************************************************/
bool MM_ReadFileHeaders(const char *FileName, int32 FileHandle, CFE_FS_Header_t *CFEHeader,
                        MM_LoadDumpFileHeader_t *MMHeader);

/************************************************************************/
/** \brief Fill memory
**
**  \par Description
**       Support function for #MM_FillMemCmd. This routine will
**       load memory with a command specified fill pattern
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   DestAddr   The destination address for the fill
**                           operation
**
**  \param [in]   CmdPtr     A #MM_FillMemCmd_t pointer to the fill
**                           command message
**
*************************************************************************/
bool MM_FillMem(cpuaddr DestAddr, const MM_FillMemCmd_t *CmdPtr);

/************************************************************************/
/** \brief Process memory poke command
**
**  \par Description
**       Processes the memory poke command that will load a memory
**       location with data specified in the command message.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   BufPtr   A #CFE_SB_Buffer_t* pointer that
**                             references the software bus message
**
**  \sa #MM_POKE_CC
**
*************************************************************************/
bool MM_PokeCmd(const CFE_SB_Buffer_t *BufPtr);

/************************************************************************/
/** \brief Process load memory with interrupts disabled command
**
**  \par Description
**       Processes the load memory with interrupts disabled command
**       that will load up to #MM_MAX_UNINTERRUPTIBLE_DATA bytes into
**       ram with interrupts disabled during the actual load
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   BufPtr   A #CFE_SB_Buffer_t* pointer that
**                             references the software bus message
**
**  \sa #MM_LOAD_MEM_WID_CC
**
*************************************************************************/
bool MM_LoadMemWIDCmd(const CFE_SB_Buffer_t *BufPtr);

/************************************************************************/
/** \brief Process memory load from file command
**
**  \par Description
**       Processes the memory load from file command that will read a
**       file and store the data in the command specified address range
**       of memory.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   BufPtr   A #CFE_SB_Buffer_t* pointer that
**                             references the software bus message
**
**  \sa #MM_LOAD_MEM_FROM_FILE_CC
**
*************************************************************************/
bool MM_LoadMemFromFileCmd(const CFE_SB_Buffer_t *BufPtr);

/************************************************************************/
/** \brief Process memory fill command
**
**  \par Description
**       Processes the memory fill command that will load an address
**       range of memory with the command specified fill pattern
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   BufPtr   A #CFE_SB_Buffer_t* pointer that
**                             references the software bus message
**
**  \sa #MM_FILL_MEM_CC
**
*************************************************************************/
bool MM_FillMemCmd(const CFE_SB_Buffer_t *BufPtr);

#endif /* _mm_load_ */

/************************/
/*  End of File Comment */
/************************/
