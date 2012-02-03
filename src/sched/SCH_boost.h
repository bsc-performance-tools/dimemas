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

#ifndef __SCH_boost_h
#define __SCH_boost_h

#ifdef BASE_PRIORITY
#undef BASE_PRIORITY
#endif

#define BASE_PRIORITY (t_priority)0


#define BOOST_RECV_FIRST -1
#define BOOST_RECV_LAST 1
#define BOOST_SENDER_LAST -1
#define BOOST_MAX_PRIO -5
#define BOOST_MIN_PRIO 5


struct t_SCH_boost
{
   t_priority      priority;
   t_nano         last_quantum;
};

/* Boost scheduler parameters */
struct t_boost
{
   int             best;
   int             worst;
   int             base_prio;
   int             recv_first;
   int             recv_last;
   int             sender_last;
};

#define PRV_PRIO 90

/**
 * External routines defined in file SCH_boost.c
 **/
extern void SCH_boost_thread_to_ready(struct t_thread *thread);
extern t_nano SCH_boost_get_execution_time(struct t_thread *thread);
extern struct t_thread *SCH_boost_next_thread_to_run(struct t_node *node);
extern void SCH_boost_init_scheduler_parameters(struct t_thread *thread);
extern void SCH_boost_clear_parameters(struct t_thread *thread);
extern int SCH_boost_info(int info, struct t_thread *thread_s,
		     struct t_thread *thread_r);
extern void SCH_boost_init(char *filename, struct t_machine  *machine);
extern void SCH_boost_copy_parameters(struct t_thread *th_o,
				      struct t_thread *th_d);
extern void SCH_boost_free_parameters (struct t_thread *thread);

#endif
