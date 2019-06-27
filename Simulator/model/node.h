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

#ifndef _NODE_H_
#define _NODE_H_

#include <types.h>

struct t_accelerator {
	dimemas_timer		startup;
	dimemas_timer		memory_startup;
	t_bandwidth			bandwidth;
	struct t_queue	threads_in_link;
	struct t_queue	wait_for_link;
	int							max_messages;
	int 						cur_messages;
};

#define	ACCELERATOR_NIL (struct t_accelerator*) 0

struct t_node
{
  char             *arch;
  int               nodeid;
  struct t_queue    Cpus;
  t_nano            relative;
  t_nano            bandwidth;
  t_nano            local_startup;
  t_nano            remote_startup;
  t_nano            external_net_startup; /* Latencia de la xarxa externa */
  t_nano            local_port_startup;
  t_nano            remote_port_startup;
  t_nano            local_memory_startup;
  t_nano            remote_memory_startup;
  struct t_queue    ready;

  /* Network links */
  t_boolean         infinite_net_links;   /* TRUE if there are infinite links */
  t_boolean         half_duplex_links;    /* TRUE if links are half duplex */
  struct t_queue    free_in_links;        /* Free input links */
  struct t_queue    free_out_links;       /* Free output link */
  struct t_queue    busy_in_links;        /* Busy input links */
  struct t_queue    busy_out_links;       /* Busy output links */
  struct t_queue    th_for_in;            /* Awaiting for input link */
  struct t_queue    th_for_out;           /* Awaiting for output link */

  /* Memory buses */
  int               max_memory_messages;
  int               cur_memory_messages;
  struct t_queue    wait_for_mem_bus;
  struct t_queue    threads_in_memory;

  t_boolean         infinite_mem_links;
  int               in_mem_links;
  int               out_mem_links;

  struct t_queue    wait_outlink_port;
  struct t_queue    wait_inlink_port;
  struct t_queue    wait_in_copy_segment;
  struct t_queue    wait_out_copy_segment;
  struct t_queue    IO_disks;
  struct t_queue    IO_disks_threads;

  struct t_machine *machine;

  t_boolean         initialized;
  t_boolean         used_node;

  //EEE
  int messages_in_flight;

  //accelerator
  int acc_nodes_count;
  t_boolean accelerator;
  struct t_accelerator 	acc;
  double acc_relative;
  t_boolean has_accelerated_task;
};

void NODE_Init_Empty_Node(struct t_machine* machine,
                          struct t_node* node);


void NODE_Fill_Node_Fields(struct t_node *node,
                           char          *node_name,
                           int            no_processors,
                           int            no_mem_buses,
                           int            no_mem_in_link,
                           int            no_mem_out_links,
                           int            no_input,
                           int            no_output,
                           double         local_startup,
                           double         remote_startup,
                           double         relative,
                           double         local_bandwidth,
                           double         external_net_startup,
                           double         local_port_startup,
                           double         remote_port_startup,
                           double         local_memory_startup,
                           double         remote_memory_startup);

void	NODE_set_acc(int node_id,
									 double latency,
									 double memory_latency,
									 double bandwith,
									 int num_acc_buses,
									 double relative);

t_boolean NODE_get_acc(struct t_node *node);
int NODE_get_acc_node(struct t_node *node);
#endif
