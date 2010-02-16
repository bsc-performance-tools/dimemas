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

#ifndef __SCH_prio_fifo_h
#define __SCH_prio_fifo_h

#ifdef BASE_PRIORITY
#undef BASE_PRIORITY
#endif
#define BASE_PRIORITY (t_priority)0
struct t_SCH_prio_fifo
{
   t_priority      priority;
};

/**
 * External routines defined in file SCH_prio_fifo.c
 **/
extern void PRIO_FIFO_thread_to_ready(struct t_thread *thread);
extern t_micro PRIO_FIFO_get_execution_time(struct t_thread *thread);
extern struct t_thread *PRIO_FIFO_next_thread_to_run(struct t_node *node);
extern void PRIO_FIFO_init_scheduler_parameters(struct t_thread *thread);
extern void PRIO_FIFO_clear_parameters(struct t_thread *thread);
extern int PRIO_FIFO_info(int info);
extern void PRIO_FIFO_init(char *filename, struct t_machine *machine);
extern void PRIO_FIFO_copy_parameters(struct t_thread *th_o, 
				      struct t_thread *th_d);
extern void PRIO_FIFO_free_parameters(struct t_thread *thread);
#endif
