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

#ifndef __schedule_h
#define __schedule_h
/**
 * External routines defined in file schedule.c
 **/

void SCHEDULER_Init( void );

void SCHEDULER_End( void );

struct t_cpu *select_free_cpu( struct t_node *node, struct t_thread *thread );

void SCHEDULER_thread_to_ready( struct t_thread *thread );

t_nano SCHEDULER_get_execution_time( struct t_thread *thread );

void SCHEDULER_next_thread_to_run( struct t_node *node );

void SCHEDULER_general( int value, struct t_thread *thread );

void SCHEDULER_thread_to_busy_wait( struct t_thread *thread );

void SCHEDULER_reload( struct t_Ptask *Ptask );

void SCHEDULER_copy_parameters( struct t_thread *th_o, struct t_thread *th_d );

void SCHEDULER_free_parameters( struct t_thread *thread );

int SCHEDULER_info( int value, int info, struct t_thread *th_s, struct t_thread *th_r );

void SCHEDULER_thread_to_ready_return( int module, struct t_thread *thread, t_nano ti, int id );

struct t_thread *SCHEDULER_preemption( struct t_thread *thread, struct t_cpu *cpu );

int SCHEDULER_get_policy( char *s );

t_boolean more_actions( struct t_thread *thread );

/* JGG: Intenta replanificar el thread que le pasamos por parámetro */
void SCHEDULER_reschedule( struct t_thread *thread );

void treat_acc_event( struct t_thread *thread, struct t_action *action );

#endif
