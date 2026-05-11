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

#ifndef MM_INTERFACE_CFG_H
#define MM_INTERFACE_CFG_H

/* ======== */
/* Includes */
/* ======== */

#include "mm_interface_cfg_values.h"

/* ====== */
/* Macros */
/* ====== */

/**
 * \brief Maximum number of bytes for an uninterruptable load
 *
 *  \par Description:
 *       Maximum number of bytes that can be loaded with the
 *       "memory load with interrupts disabled" (#MM_LOAD_MEM_WID_CC)
 *       command.
 *
 *  \par Limits:
 *       This parameter is limited to the size of an uint8 which
 *       is the data type used to specify the number of bytes to
 *       load in the command message.
 *
 *       If this data type is made bigger, changing this value to a
 *       large number will increase the amount of time interrupts are
 *       disabled during the load. It should also be kept small enough
 *       to avoid packet segmentation for the command protocal being
 *       used.
 */
#define MM_INTERFACE_MAX_UNINTERRUPTIBLE_DATA         MM_INTERFACE_CFGVAL(MAX_UNINTERRUPTIBLE_DATA)
#define DEFAULT_MM_INTERFACE_MAX_UNINTERRUPTIBLE_DATA 200

#endif /* MM_INTERFACE_CFG_H */