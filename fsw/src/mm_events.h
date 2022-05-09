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
 *   Specification for the CFS Memory Manger event identifers.
 */
#ifndef MM_EVENTS_H
#define MM_EVENTS_H

/**
 * \defgroup cfsmmevents CFS Memory Manager Event IDs
 * \{
 */

/**
 * \brief MM Initialization Event ID
 *
 *  \par Type: INFORMATIONAL
 *
 *  \par Cause:
 *
 *  This event message is issued when the CFS Memory Manager has
 *  completed initialization.
 */
#define MM_INIT_INF_EID 1

/**
 * \brief MM No-op Command Event ID
 *
 *  \par Type: INFORMATIONAL
 *
 *  \par Cause:
 *
 *  This event message is issued when a NOOP command has been received.
 */
#define MM_NOOP_INF_EID 2

/**
 * \brief MM Reset Counters Command Event ID
 *
 *  \par Type: INFORMATIONAL
 *
 *  \par Cause:
 *
 *  This event message is issued when a reset counters command has
 *  been received.
 */
#define MM_RESET_INF_EID 3

/**
 * \brief MM Load With Interrupts Disabled Command Event ID
 *
 *  \par Type: INFORMATIONAL
 *
 *  \par Cause:
 *
 *  This event message is issued when a memory load with interrupts disabled
 *  command has been executed.
 */
#define MM_LOAD_WID_INF_EID 4

/**
 * \brief MM Load From File Command Event ID
 *
 *  \par Type: INFORMATIONAL
 *
 *  \par Cause:
 *
 *  This event message is issued when a load memory from file command has
 *  been executed.
 */
#define MM_LD_MEM_FILE_INF_EID 5

/**
 * \brief MM Fill Command Event ID
 *
 *  \par Type: INFORMATIONAL
 *
 *  \par Cause:
 *
 *  This event message is issued when a fill memory command has been executed.
 */
#define MM_FILL_INF_EID 6

/**
 * \brief MM 8-bit Peek Command Event ID
 *
 *  \par Type: INFORMATIONAL
 *
 *  \par Cause:
 *
 *  This event message is issued when an 8 bit memory peek command has been
 *  executed.
 */
#define MM_PEEK_BYTE_INF_EID 7

/**
 * \brief MM 16-bit Peek Command Event ID
 *
 *  \par Type: INFORMATIONAL
 *
 *  \par Cause:
 *
 *  This event message is issued when a 16 bit memory peek command has been
 *  executed.
 */
#define MM_PEEK_WORD_INF_EID 8

/**
 * \brief MM 32-bit Peek Command Event ID
 *
 *  \par Type: INFORMATIONAL
 *
 *  \par Cause:
 *
 *  This event message is issued when a 32 bit memory peek command has been
 *  executed.
 */
#define MM_PEEK_DWORD_INF_EID 9

/**
 * \brief MM 8-bit Poke Command Event ID
 *
 *  \par Type: INFORMATIONAL
 *
 *  \par Cause:
 *
 *  This event message is issued when an 8 bit memory poke command has been
 *  executed.
 */
#define MM_POKE_BYTE_INF_EID 10

/**
 * \brief MM 16-bit Poke Command Event ID
 *
 *  \par Type: INFORMATIONAL
 *
 *  \par Cause:
 *
 *  This event message is issued when an 16 bit memory poke command has been
 *  executed.
 */
#define MM_POKE_WORD_INF_EID 11

/**
 * \brief MM 32-bit Poke Command Event ID
 *
 *  \par Type: INFORMATIONAL
 *
 *  \par Cause:
 *
 *  This event message is issued when an 32 bit memory poke command has been
 *  executed.
 */
#define MM_POKE_DWORD_INF_EID 12

/**
 * \brief MM Dump To File Command Event ID
 *
 *  \par Type: INFORMATIONAL
 *
 *  \par Cause:
 *
 *  This event message is issued when a dump memory to file command has
 *  been executed.
 */
#define MM_DMP_MEM_FILE_INF_EID 13

/**
 * \brief MM Dump To Event Message Command ID
 *
 *  \par Type: INFORMATIONAL
 *
 *  \par Cause:
 *
 *  This event message is issued in response to a dump memory in event message
 *  command.
 */
#define MM_DUMP_INEVENT_INF_EID 14

/**
 * \brief MM Pipe Read Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when a call to #CFE_SB_ReceiveBuffer fails.
 */
#define MM_PIPE_ERR_EID 15

/**
 * \brief MM Command Pipe Message ID Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when a software bus message is received
 *  with an invalid message ID.
 */
#define MM_MID_ERR_EID 16

/**
 * \brief MM Ground Command Code Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when a software bus message is received
 *  with an invalid command code.
 */
#define MM_CC1_ERR_EID 17

/**
 * \brief MM Command Message Length Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when command message is received with a message
 *  length that doesn't match the expected value.
 */
#define MM_LEN_ERR_EID 18

/**
 * \brief MM Commanded Memory Type Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when a command is received with an
 *  unrecognized or unsupported memory type specified.
 */
#define MM_MEMTYPE_ERR_EID 19

/**
 * \brief MM Symbolic Address Resolution Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when a symbol name string can't be
 *  resolved by the OSAPI.
 */
#define MM_SYMNAME_ERR_EID 20

/**
 * \brief MM Data Size In Bytes Invalid Or Exceeds Limits Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when a command or load file specifies a number
 *  of bytes that is either zero or exceeds the limits specified by the
 *  MM configuration parameters
 */
#define MM_DATA_SIZE_BYTES_ERR_EID 21

/**
 * \brief MM Data Size In Bits Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when a command specified bit width
 *  for a peek or poke operation is either undefined or not valid for the
 *  specified memory type.
 */
#define MM_DATA_SIZE_BITS_ERR_EID 22

/**
 * \brief MM Data Or Address Not 32-bit Aligned Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when command execution requires 32 bit wide
 *  memory access and the data size and address specified are not both 32 bit
 *  aligned.
 */
#define MM_ALIGN32_ERR_EID 23

/**
 * \brief MM Data Or Address Not 16-bit Aligned Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when command execution requires 16 bit wide
 *  memory access and the data size and address specified are not both 16 bit
 *  aligned.
 */
#define MM_ALIGN16_ERR_EID 24

/**
 * \brief MM Memory Range Validation Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when a call to the #CFE_PSP_MemValidateRange routine that
 *  is used to check address parameters fails.
 */
#define MM_OS_MEMVALIDATE_ERR_EID 25

/**
 * \brief MM Load File CRC Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when a CRC computation on the data in
 *  a load file does not return the expected result that is specified in
 *  the load file header.
 */
#define MM_LOAD_FILE_CRC_ERR_EID 26

/**
 * \brief MM Interrupt Disabled Load CRC Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when a CRC computation on the data in
 *  a load with interrupts disabled command message does not return the
 *  expected result that is specified in the command message header.
 */
#define MM_LOAD_WID_CRC_ERR_EID 27

/**
 * \brief MM 8-bit EEPROM Write Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when the 8-bit EEPROM write function
 *  returns some value other than #OS_SUCCESS.
 */
#define MM_OS_EEPROMWRITE8_ERR_EID 28

/**
 * \brief MM 16-bit EEPROM Write Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when the 16-bit EEPROM write function
 *  returns some value other than #OS_SUCCESS.
 */
#define MM_OS_EEPROMWRITE16_ERR_EID 29

/**
 * \brief MM 32-bit EEPROM Write Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when the 32-bit EEPROM write function
 *  returns some value other than #OS_SUCCESS.
 */
#define MM_OS_EEPROMWRITE32_ERR_EID 30

/**
 * \brief MM File Create Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when a call to the #OS_OpenCreate function
 *  returns some value other than #OS_SUCCESS.
 */
#define MM_OS_CREAT_ERR_EID 31

/**
 * \brief MM File Open Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when a call to the #OS_OpenCreate function
 *  returns some value other than #OS_SUCCESS.
 */
#define MM_OS_OPEN_ERR_EID 32

/**
 * \brief MM File Close Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when a call to the #OS_close function
 *  returns some value other than #OS_SUCCESS.
 */
#define MM_OS_CLOSE_ERR_EID 33

/**
 * \brief MM File Read Returned Error Code Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when a call to the #OS_read function
 *  returns a negative error code.
 */
#define MM_OS_READ_ERR_EID 34

/**
 * \brief MM File Read Byte Read Mismatch Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when a call to the #OS_read function
 *  returns some value other than the expected number of bytes read.
 */
#define MM_OS_READ_EXP_ERR_EID 35

/**
 * \brief MM File Write Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when a call to the #OS_read function
 *  returns some value other than the expected number of bytes written.
 */
#define MM_OS_WRITE_EXP_ERR_EID 36

/**
 * \brief MM File Stat Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when a call to the #OS_stat function
 *  returns some value other than #OS_SUCCESS.
 */
#define MM_OS_STAT_ERR_EID 37

/**
 * \brief MM Compute CRC From File Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when a call to the #MM_ComputeCRCFromFile
 *  function returns some value other than #OS_SUCCESS.
 */
#define MM_COMPUTECRCFROMFILE_ERR_EID 38

/**
 * \brief MM Load File Size Mismatch Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when a load memory from file command
 *  is processed and the size of the load file in bytes (as reported by the
 *  filesystem) doesn't match what would be expected based upon the load byte
 *  count specified in the file header.
 */
#define MM_LD_FILE_SIZE_ERR_EID 40

/**
 * \brief MM Load File Parameter Check Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when a load file fails one of a series of
 *  parameter checks on the Destination Address, Memory Type, and Byte Size
 *  specified in the load file header. Another error event will be issued
 *  with the specific error, this is a supplemental message that echos the
 *  name of the file that failed.
 */
#define MM_FILE_LOAD_PARAMS_ERR_EID 41

/**
 * \brief MM Read File Header Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when a call to the #CFE_FS_ReadHeader function
 *  returns some value other than the expected number of bytes read.
 */
#define MM_CFE_FS_READHDR_ERR_EID 42

/**
 * \brief MM Write File Header Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when a call to the #CFE_FS_WriteHeader function
 *  returns some value other than the expected number of bytes written.
 */
#define MM_CFE_FS_WRITEHDR_ERR_EID 43

/**
 * \brief MM Housekeeping Request Message Length Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when housekeeping request message is received
 *  with a message length that doesn't match the expected value.
 */
#define MM_HKREQ_LEN_ERR_EID 44

/**
 * \brief MM Symbol Lookup Command Event ID
 *
 *  \par Type: INFORMATIONAL
 *
 *  \par Cause:
 *
 *  This event message is issued when an symbol lookup command has been
 *  successfully executed.
 */
#define MM_SYM_LOOKUP_INF_EID 45

/**
 * \brief MM Symbol Name NULL Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when a NUL string has been specified
 *  as the symbol name in a lookup symbol command
 */
#define MM_SYMNAME_NUL_ERR_EID 46

/**
 * \brief MM Symbol Table Dump To File Started Event ID
 *
 *  \par Type: INFORMATIONAL
 *
 *  \par Cause:
 *
 *  This event message is issued when a dump symbol table fo file command has been
 *  successfully executed.
 */
#define MM_SYMTBL_TO_FILE_INF_EID 47

/**
 * \brief MM Symbol Table Dump To File Filename NULL Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when a NUL string has been specified
 *  as the dump file name in a dump symbol table to file command
 */
#define MM_SYMFILENAME_NUL_ERR_EID 48

/**
 * \brief MM Symbol Table Dump To File Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when the dump to symbol table fails.
 */
#define MM_SYMTBL_TO_FILE_FAIL_ERR_EID 49

/**
 * \brief MM EEPROM Bank Write Enable Event ID
 *
 *  \par Type: INFORMATIONAL
 *
 *  \par Cause:
 *
 *  This event message is issued when a request to enable writing to a specified
 *  EEPROM bank results in a success status from the PSP.
 */
#define MM_EEPROM_WRITE_ENA_INF_EID 51

/**
 * \brief MM EEPROM Bank Write Enable Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when a request to enable writing to a specified
 *  EEPROM bank results in an error status from the PSP.
 */
#define MM_EEPROM_WRITE_ENA_ERR_EID 52

/**
 * \brief MM EEPROM Bank Write Disable Event ID
 *
 *  \par Type: INFORMATIONAL
 *
 *  \par Cause:
 *
 *  This event message is issued when a request to disable writing to a specified
 *  EEPROM bank results in a success status from the PSP.
 */
#define MM_EEPROM_WRITE_DIS_INF_EID 53

/**
 * \brief MM EEPROM Bank Write Disable Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when a request to disable writing to a specified
 *  EEPROM bank results in an error status from the PSP.
 */
#define MM_EEPROM_WRITE_DIS_ERR_EID 54

/**
 * \brief MM File Read Zero Bytes Returned Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when a call to the #OS_read function
 *  returns zero total bytes read.
 */
#define MM_OS_ZERO_READ_ERR_EID 55

/**
 * \brief MM Memory Read Error Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when a call to one of the CFE_PSP_MemRead functions
 *  (#CFE_PSP_MemRead8, #CFE_PSP_MemRead16, #CFE_PSP_MemRead32) returns something
 *  other than CFE_PSP_SUCCESS.
 */
#define MM_PSP_READ_ERR_EID 56

/**
 * \brief MM Memory Write Error Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when a call to one of the CFE_PSP_MemWrite functions
 *  (#CFE_PSP_MemWrite8, #CFE_PSP_MemWrite16, #CFE_PSP_MemWrite32) returns something
 *  other than CFE_PSP_SUCCESS.
 */
#define MM_PSP_WRITE_ERR_EID 57

/**
 * \brief MM Create Pipe Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when a call to #CFE_SB_CreatePipe returns
 *  something other than CFE_SUCCESS.
 */
#define MM_CR_PIPE_ERR_EID 60

/**
 * \brief MM Housekeeping Request Subscription Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when a call to #CFE_SB_Subscribe returns
 *  something other than CFE_SUCCESS when called for MM_SEND_HK_MID.
 */
#define MM_HK_SUB_ERR_EID 61

/**
 * \brief MM Command Subscription Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when a call to #CFE_SB_Subscribe returns
 *  something other than CFE_SUCCESS when called for MM_CMD_MID.
 */
#define MM_CMD_SUB_ERR_EID 62

/**
 * \brief MM 32-bit Fill Memory Not Aligned Event ID
 *
 *  \par Type: INFORMATION
 *
 *  \par Cause:
 *
 *  This event message is issued when a call to #MM_FillMem32 is called with a
 *  NumOfBytes value that is not divisible by 4.
 */
#define MM_FILL_MEM32_ALIGN_WARN_INF_EID 63

/**
 * \brief MM 16-bit Fill Memory Not Aligned Event ID
 *
 *  \par Type: INFORMATION
 *
 *  \par Cause:
 *
 *  This event message is issued when a call to #MM_FillMem16 is called with a
 *  NumOfBytes value that is not divisible by 2.
 */
#define MM_FILL_MEM16_ALIGN_WARN_INF_EID 64

/**\}*/

#endif
