/************************************************************************
** File: mm_app.h 
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
**   Unit specification for the Core Flight System (CFS)
**   Memory Manger (MM) Application.
**
*************************************************************************/
#ifndef _mm_app_
#define _mm_app_

/************************************************************************
** Includes
*************************************************************************/
#include "mm_msg.h"
#include "cfe.h"

/************************************************************************
** Macro Definitions
*************************************************************************/
/**
** \name MM Command Pipe Parameters */
/** \{ */
#define MM_CMD_PIPE_DEPTH (3 * CFE_PLATFORM_SB_DEFAULT_MSG_LIMIT)
/** \} */

/**
** \name MM Command Pipe Parameters */
/** \{ */
#define MM_VERIFY_DUMP  0
#define MM_VERIFY_LOAD  1
#define MM_VERIFY_EVENT 2
#define MM_VERIFY_FILL  3
#define MM_VERIFY_WID   4
#define MM_VERIFY_TYPES 5
/** \} */

#define MM_MAX_MEM_TYPE_STR_LEN 11 /* length of MM_EEPROM + \0 */

/**
** \brief Wakeup for MM
**
** \par Description
**      Wakes up MM every 1 second for routine maintenance whether a
**      message was received or not.
*/

#define MM_SB_TIMEOUT 1000

/************************************************************************
** Type Definitions
*************************************************************************/
/**
**  \brief MM global data structure
*/
typedef struct
{
    MM_HkPacket_t HkPacket; /**< \brief Housekeeping telemetry packet  */

    CFE_SB_PipeId_t CmdPipe; /**< \brief Command pipe ID                */

    uint32 RunStatus; /**< \brief Application run status         */

    uint32 LoadBuffer[MM_MAX_LOAD_DATA_SEG / 4]; /**< \brief Load file i/o buffer */
    uint32 DumpBuffer[MM_MAX_DUMP_DATA_SEG / 4]; /**< \brief Dump file i/o buffer */
    uint32 FillBuffer[MM_MAX_FILL_DATA_SEG / 4]; /**< \brief Fill memory buffer   */

} MM_AppData_t;

/************************************************************************
** Exported Functions
*************************************************************************/
/************************************************************************/
/** \brief CFS Memory Manager (MM) application entry point
**
**  \par Description
**       Memory Manager application entry point and main process loop.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
*************************************************************************/
void MM_AppMain(void);

/************************************************************************
** Function prototypes
*************************************************************************/
/************************************************************************/
/** \brief Initialize the memory manager CFS application
**
**  \par Description
**       Memory manager application initialization routine. This
**       function performs all the required startup steps to
**       get the application registered with the cFE services so
**       it can begin to receive command messages.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \returns
**  \retcode #CFE_SUCCESS  \retdesc \copydoc CFE_SUCCESS \endcode
**  \retstmt Return codes from #CFE_EVS_Register         \endcode
**  \retstmt Return codes from #CFE_SB_CreatePipe        \endcode
**  \retstmt Return codes from #CFE_SB_Subscribe         \endcode
**  \endreturns
**
*************************************************************************/
int32 MM_AppInit(void);

/************************************************************************/
/** \brief Process a command pipe message
**
**  \par Description
**       Processes a single software bus command pipe message. Checks
**       the message and command IDs and calls the appropriate routine
**       to handle the command.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   msg   A #CFE_SB_Buffer_t* pointer that
**                      references the software bus message
**
**  \sa #CFE_SB_RcvMsg
**
*************************************************************************/
void MM_AppPipe(const CFE_SB_Buffer_t *msg);

/************************************************************************/
/** \brief Process housekeeping request
**
**  \par Description
**       Processes an on-board housekeeping request message.
**
**  \par Assumptions, External Events, and Notes:
**       This command does not affect the command execution counter
**
**  \param [in]   msg   A #CFE_SB_Buffer_t* pointer that
**                      references the software bus message
**
*************************************************************************/
void MM_HousekeepingCmd(const CFE_SB_Buffer_t *msg);

/************************************************************************/
/** \brief Process noop command
**
**  \par Description
**       Processes a noop ground command.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   msg   A #CFE_SB_Buffer_t* pointer that
**                      references the software bus message
**
**  \sa #MM_NOOP_CC
**
*************************************************************************/
bool MM_NoopCmd(const CFE_SB_Buffer_t *msg);

/************************************************************************/
/** \brief Process reset counters command
**
**  \par Description
**       Processes a reset counters ground command which will reset
**       the memory manager commmand error and command execution counters
**       to zero.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   BufPtr   A #CFE_SB_Buffer_t* pointer that
**                             references the software bus message
**
**  \sa #MM_RESET_CC
**
*************************************************************************/
bool MM_ResetCmd(const CFE_SB_Buffer_t *BufPtr);

/************************************************************************/
/** \brief Process lookup symbol command
**
**  \par Description
**       Processes a lookup symbol ground command which takes a
**       symbol name and tries to resolve it to an address using the
**       #OS_SymbolLookup OSAL function.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   msg          A #CFE_SB_Buffer_t* pointer that
**                             references the software bus message
**
**  \sa #MM_LOOKUP_SYM_CC
**
*************************************************************************/
bool MM_LookupSymbolCmd(const CFE_SB_Buffer_t *msg);

/************************************************************************/
/** \brief Dump symbol table to file command
**
**  \par Description
**       Processes a dump symbol table to file ground command which calls
**       the #OS_SymbolTableDump OSAL function using the specified dump
**       file name.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   msg          A #CFE_SB_Buffer_t* pointer that
**                             references the software bus message
**
**  \sa #MM_SYMTBL_TO_FILE_CC
**
*************************************************************************/
bool MM_SymTblToFileCmd(const CFE_SB_Buffer_t *msg);

/************************************************************************/
/** \brief Write-enable EEPROM command
**
**  \par Description
**       Processes a EEPROM write enable ground command which calls
**       the #CFE_PSP_EepromWriteEnable cFE function using the specified
**       bank number.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   msg          A #CFE_SB_Buffer_t* pointer that
**                             references the software bus message
**
**  \sa #MM_ENABLE_EEPROM_WRITE_CC
**
*************************************************************************/
bool MM_EepromWriteEnaCmd(const CFE_SB_Buffer_t *msg);

/************************************************************************/
/** \brief Write-disable EEPROM command
**
**  \par Description
**       Processes a EEPROM write disable ground command which calls
**       the #CFE_PSP_EepromWriteDisable cFE function using the specified
**       bank number.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   msg          A #CFE_SB_Buffer_t* pointer that
**                             references the software bus message
**
**  \sa #MM_DISABLE_EEPROM_WRITE_CC
**
*************************************************************************/
bool MM_EepromWriteDisCmd(const CFE_SB_Buffer_t *msg);

/************************************************************************
** Exported Data
*************************************************************************/
extern MM_AppData_t MM_AppData;

#endif /* _mm_app_ */

/************************/
/*  End of File Comment */
/************************/
