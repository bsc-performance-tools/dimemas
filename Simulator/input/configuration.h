/*****************************************************************************\
 *                        ANALYSIS PERFORMANCE TOOLS                         *
 *                                  Dimemas                                  *
 *       Simulation tool for the parametric analysis of the behaviour of     *
 *       message-passing applications on a configurable parallel platform    *
 *                                                                           *
 *****************************************************************************
 *     ___     This library is free software; you can redistribute it and/or *
 *    /  __         modify it under the terms of the GNU LGPL as published   *
 *   /  /  _____    by the Free Software Foundation; either version 2.1      *
 *  /  /  /     \   of the License, or (at your option) any later version.   *
 * (  (  ( B S C )                                                           *
 *  \  \  \_____/   This library is distributed in hope that it will be      *
 *   \  \__         useful but WITHOUT ANY WARRANTY; without even the        *
 *    \___          implied warranty of MERCHANTABILITY or FITNESS FOR A     *
 *                  PARTICULAR PURPOSE. See the GNU LGPL for more details.   *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public License  *
 * along with this library; if not, write to the Free Software Foundation,   *
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA          *
 * The GNU LEsser General Public License is contained in the file COPYING.   *
 *                                 ---------                                 *
 *   Barcelona Supercomputing Center - Centro Nacional de Supercomputacion   *
\*****************************************************************************/

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- *\

  $URL:: https://svn.bsc.#$:  File
  $Rev:: 35               $:  Revision of last commit
  $Author:: jgonzale      $:  Author of last commit
  $Date:: 2012-01-11 19:4#$:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef _CONFIGURATION_H_
#define _CONFIGURATION_H_

#include "types.h"
#include "ts.h"

/*
 * Definitions in a configuration file
 */
typedef enum { WAN_INFORMATION = 0,
               ENV_INFORMATION,
               NODE_INFORMATION,
               MAPPING,
               CONFIG_FILES,
               MOD_INFORMATION,
               FS_PARAMETERS,
               DED_CONNECTION } record_types;

#define WAN_INFORMATION  0
#define ENV_INFORMATION  1
#define NODE_INFORMATION 2
#define MAP_INFORMATION  3
#define CONFIG_FILES     4
#define MOD_INFORMATION  5
#define FS_PARAMETERS    6
#define DED_CONNECTION   7


extern void CONFIGURATION_Init(char *configuration_filename);

extern void CONFIGURATION_Set_Scheduling_Configuration_File(char *sch_file);

extern void CONFIGURATION_Set_FileSystem_Configuration_File(char *fs_file);

extern char* CONFIGURATION_Get_Configuration_FileName(void);

extern void CONFIGURATION_New_Definition(struct t_queue *definition_fields,
                                         struct t_entry *definition_structure);

extern void CONFIGURATION_Load_Communications_Configuration(void);
extern void CONFIGURATION_Load_Scheduler_Configuration(void);
extern void CONFIGURATION_Load_Random_Configuration(void);
extern void CONFIGURATION_Load_FS_Configuration(void);

#endif
