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
 *   CFS Memory Manager (MM) Application Message IDs
 */
#ifndef MM_MSGIDS_H
#define MM_MSGIDS_H

/**
 * \defgroup cfsmmcmdmid CFS Memory Manager Command Message IDs
 * \{
 */

#define MM_CMD_MID     0x1888 /**< \brief Msg ID for cmds to mm     */
#define MM_SEND_HK_MID 0x1889 /**< \brief Msg ID to request mm HK   */

/**\}*/

/**
 * \defgroup cfsmmtlmmid CFS Memory Manager Telemetry Message IDs
 * \{
 */

#define MM_HK_TLM_MID 0x0887 /**< \brief MM Housekeeping Telemetry */

/**\}*/

#endif
