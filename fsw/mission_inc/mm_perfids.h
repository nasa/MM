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
 *   CFS Memory Manager (MM) Application Performance IDs
 */
#ifndef MM_PERFIDS_H
#define MM_PERFIDS_H

/**
 * \ingroup cfsmmmissioncfg
 * \{
 */

#define MM_APPMAIN_PERF_ID         30 /**< \brief Application main performance ID */
#define MM_SEGBREAK_PERF_ID        31 /**< \brief Memory processing segment break performance ID */
#define MM_EEPROM_POKE_PERF_ID     32 /**< \brief EEPROM poke performance ID */
#define MM_EEPROM_FILELOAD_PERF_ID 33 /**< \brief EEPROM file load performance ID */
#define MM_EEPROM_FILL_PERF_ID     34 /**< \brief EEPROM fill performance ID */

/**\}*/

#endif
