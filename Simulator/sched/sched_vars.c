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

#include "sched_vars.h"

#include "SCH_boost.h"
#include "SCH_fifo.h"
#include "SCH_prio_fifo.h"
#include "SCH_rr.h"
#include "SCH_ss_mpi_cp.h"
#include "SCH_svr4.h"
#include "define.h"
#include "extern.h"
#include "types.h"

#include <assert.h>

void empty_modify_priority( struct t_thread *thread )
{
  assert( thread != NULL );
}

void empty_modify_preemption( struct t_thread *thread )
{
  assert( thread != NULL );
}

struct t_scheduler_actions SCH[] = { { "FIFO", /* Politica FIFO */
                                       FIFO_thread_to_ready,
                                       FIFO_get_execution_time,
                                       FIFO_next_thread_to_run,
                                       FIFO_init_scheduler_parameters,
                                       FIFO_clear_parameters,
                                       FIFO_info,
                                       FIFO_init,
                                       FIFO_copy_parameters,
                                       FIFO_free_parameters,
                                       empty_modify_priority,
                                       empty_modify_preemption },
                                     { "RR", /* Politica Round Robin */
                                       RR_thread_to_ready,
                                       RR_get_execution_time,
                                       RR_next_thread_to_run,
                                       RR_init_scheduler_parameters,
                                       RR_clear_parameters,
                                       RR_info,
                                       RR_init,
                                       RR_copy_parameters,
                                       RR_free_parameters,
                                       empty_modify_priority,
                                       empty_modify_preemption },
                                     { "FIXED_PRIORITY_FIFO", /* Politica FIFO con prioridades fijas */
                                       PRIO_FIFO_thread_to_ready,
                                       PRIO_FIFO_get_execution_time,
                                       PRIO_FIFO_next_thread_to_run,
                                       PRIO_FIFO_init_scheduler_parameters,
                                       PRIO_FIFO_clear_parameters,
                                       PRIO_FIFO_info,
                                       PRIO_FIFO_init,
                                       PRIO_FIFO_copy_parameters,
                                       PRIO_FIFO_free_parameters,
                                       empty_modify_priority,
                                       empty_modify_preemption },
                                     { "UNIX_SVR4", /* Politica tipo System V */
                                       svr4_thread_to_ready,
                                       svr4_get_execution_time,
                                       svr4_next_thread_to_run,
                                       svr4_init_scheduler_parameters,
                                       svr4_clear_parameters,
                                       svr4_info,
                                       svr4_init,
                                       svr4_copy_parameters,
                                       svr4_free_parameters,
                                       empty_modify_priority,
                                       empty_modify_preemption },
                                     { "BOOST", /* Politica Boost */
                                       SCH_boost_thread_to_ready,
                                       SCH_boost_get_execution_time,
                                       SCH_boost_next_thread_to_run,
                                       SCH_boost_init_scheduler_parameters,
                                       SCH_boost_clear_parameters,
                                       SCH_boost_info,
                                       SCH_boost_init,
                                       SCH_boost_copy_parameters,
                                       SCH_boost_free_parameters,
                                       empty_modify_priority,
                                       empty_modify_preemption },
                                     { "SS_MPI_CP", /* Politica for SS, favorizing MPI and critical path, preemption only MPI */
                                       SS_MPI_CP_thread_to_ready,
                                       SS_MPI_CP_get_execution_time,
                                       SS_MPI_CP_next_thread_to_run,
                                       SS_MPI_CP_init_scheduler_parameters,
                                       SS_MPI_CP_clear_parameters,
                                       SS_MPI_CP_info,
                                       SS_MPI_CP_init,
                                       SS_MPI_CP_copy_parameters,
                                       SS_MPI_CP_free_parameters,
                                       SS_MPI_CP_modify_priority,
                                       SS_MPI_CP_modify_preemption },
                                     { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } };

struct t_communic_actions COMMUNIC[] = { { "FIFO" }, { "RR" }, { "BOOST" }, { 0 } };
