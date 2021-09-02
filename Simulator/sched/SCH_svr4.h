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

#ifndef __SCH_svr4_h
#define __SCH_svr4_h

typedef short prio_t;

/* These lines comes from Solaris file /usr/include/sys/ts.h */

typedef struct tsdpent
{
  prio_t ts_globpri; /* global (class independent) priority        */
  long ts_quantum;   /* time quantum given to procs at this level  */
  prio_t ts_tqexp;   /* ts_umdpri assigned when proc at this level */
                     /*                   exceeds its time quantum */
  prio_t ts_slpret;  /* ts_umdpri assigned when proc at this level */
                     /*        returns to user mode after sleeping */
  short ts_maxwait;  /* bumped to ts_lwait if more than ts_maxwait */
                     /* s ecs elapse before receiving full quantum */
  short ts_lwait;    /* ts_umdpri assigned if ts_dispwait exceeds  */
                     /*                                 ts_maxwait */
} tsdpent_t;

typedef struct tsproc
{
  prio_t ts_umdpri; /* user mode priority within ts class       */
  long ts_timeleft; /* time remaining in procs quantum           */
  long ts_dispwait; /* number of wall clock seconds since start  */
                    /*    of quantum (not reset upon preemption  */
  t_boolean full_quantum;
  long last_quantum;
} tsproc_t;

typedef struct svr4
{
  double context_switch; /* Context Switch cost */
  double busy_wait;      /* Busy wait when message not ready */
  t_boolean priority_preemptive;
  double minimum_time_preemption;
  double quantum_factor;
} svr4_t;

/**
 * External routines defined in file SCH_fifo.c
 **/
extern void svr4_thread_to_ready( struct t_thread *thread );
extern t_nano svr4_get_execution_time( struct t_thread *thread );
extern struct t_thread *svr4_next_thread_to_run( struct t_node *node );
extern void svr4_init_scheduler_parameters( struct t_thread *thread );
extern void svr4_clear_parameters( struct t_thread *thread );
extern int svr4_info( int info, struct t_thread *thread_s, struct t_thread *thread_r );
extern void svr4_init( char *filename, struct t_machine *machine );
extern void svr4_copy_parameters( struct t_thread *th_o, struct t_thread *th_d );
extern void svr4_free_parameters( struct t_thread *thread );

#endif
