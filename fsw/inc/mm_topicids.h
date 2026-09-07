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

#ifndef MM_TOPICIDS_H
#define MM_TOPICIDS_H

#include "mm_topicid_values.h"

/* Command topic IDs */
#define MM_MISSION_CMD_TOPICID         MM_MISSION_TIDVAL(CMD)
#define DEFAULT_MM_MISSION_CMD_TOPICID 0x88

#define MM_MISSION_SEND_HK_TOPICID         MM_MISSION_TIDVAL(SEND_HK)
#define DEFAULT_MM_MISSION_SEND_HK_TOPICID 0x89

/* Telemetry topic IDs */
#define MM_MISSION_HK_TLM_TOPICID         MM_MISSION_TIDVAL(HK_TLM)
#define DEFAULT_MM_MISSION_HK_TLM_TOPICID 0x87

#define MM_MISSION_PEEK_TLM_TOPICID         MM_MISSION_TIDVAL(PEEK_TLM)
#define DEFAULT_MM_MISSION_PEEK_TLM_TOPICID 0x88

#define MM_MISSION_SYM_LOOKUP_TLM_TOPICID         MM_MISSION_TIDVAL(SYM_LOOKUP_TLM)
#define DEFAULT_MM_MISSION_SYM_LOOKUP_TLM_TOPICID 0x89

#endif /* MM_TOPICIDS_H */