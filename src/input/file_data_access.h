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

  $URL::                  $:  File
  $Rev::                  $:  Revision of last commit
  $Author::               $:  Author of last commit
  $Date::                 $:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include <stdio.h>
#include "types.h"

typedef int count_t; // int or size_t?

// API PUBLIC STRUCTURES
struct ptask_structure
{
  int      ptask_id;
  count_t  tasks_count;
  count_t *threads_per_task;
};

// API MAIN FUNCTIONS
extern char* DATA_ACCESS_get_error();

extern t_boolean DATA_ACCESS_init(int   ptask_id,
                                  char* trace_file_location);

extern t_boolean DATA_ACCESS_end(void);

extern t_boolean DATA_ACCESS_get_ptask_structure(int                      ptask_id,
                                                 struct ptask_structure** ptask_info);

extern t_boolean DATA_ACCESS_get_communicators(int ptask_id, struct t_queue** communicators);

extern t_boolean DATA_ACCESS_get_next_action(int               ptask_id,
                                             int               task_id,
                                             int               thread_id,
                                             struct t_action** action);

extern int DATA_ACCESS_ptask_id_end (int ptask_id);

extern int DATA_ACCESS_test_routine(int ptask_id);

extern t_boolean   DATA_ACCESS_init_index(int   ptask_id,
                                          char *trace_file_location,
                                           int   index);


