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

#ifndef __cpu_h
#define __cpu_h
/**
 * External routines defined in file cpu.c
 **/
extern void Initialize_empty_machine(struct t_queue *machines);
#ifdef USE_EQUEUE
extern void Initialize_empty_node(Equeue *nodes, struct t_machine *machine);
#else
extern void Initialize_empty_node(struct t_queue *nodes, struct t_machine *machine);
#endif
extern void Initialize_empty_connection(struct t_queue *connections);

extern int fill_node(int no_number,
                     char  *node_name,
                     int    no_processors,
                     int    no_input,
                     int    no_output,
                     double local_startup,
                     double remote_startup,
                     double relative,
                     double local_bandwith,
                     double external_net_startup,
                     double local_port_startup,
                     double remote_port_startup,
                     double local_memory_startup,
                     double remote_memory_startup);

extern struct t_node *get_node_of_thread(struct t_thread *thread);
extern struct t_node *get_node_of_task(struct t_task *task);
extern struct t_node *get_node_by_id(int nodeid);
extern void check_full_nodes(void);
extern int num_free_cpu(struct t_node *node);
extern t_boolean is_thread_running (struct t_thread *thread);

void
Give_number_to_CPU ();


#endif
