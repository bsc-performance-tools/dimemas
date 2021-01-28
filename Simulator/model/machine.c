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

#include <list.h>
#include <machine.h>
#include <node.h>
#include <simulator.h>
#include <stdlib.h>
#include <string.h>
#include <subr.h>

void MACHINE_Init_Empty_Machine( struct t_machine *machine, int machine_id )
{
  struct t_link *link;

  machine->id = machine_id;

  machine->number_of_nodes   = MAX_NODES;
  machine->instrumented_arch = (char *)0;

  /* Scheduler information */
  machine->scheduler.policy                = 0;
  machine->scheduler.quantum               = 3000;
  machine->scheduler.priority_preemptive   = FALSE;
  machine->scheduler.lost_cpu_on_send      = FALSE;
  machine->scheduler.context_switch        = (t_nano)NO_CONTEXT_SWITCH;
  machine->scheduler.busywait_before_block = FALSE;
  ASS_TIMER( machine->scheduler.minimum_quantum, 0 );

  /* Internal network parameters */
  machine->communication.remote_bandwidth        = (t_nano)REMOTE_BANDWITH;
  machine->communication.port_startup            = (t_nano)PORT_STARTUP;
  machine->communication.memory_startup          = (t_nano)MEMORY_STARTUP;
  machine->communication.num_messages_on_network = 0;
  machine->communication.global_op_model         = Simulator.wan.global_op_model;
  create_queue( &( machine->communication.global_ops_info ) );

  /* Internal network status queues */
  create_queue( &( machine->network.threads_on_network ) );
  create_queue( &( machine->network.queue ) );

  /* External network status queues */
  create_queue( &( machine->external_net.free_in_links ) );
  create_queue( &( machine->external_net.free_out_links ) );
  create_queue( &( machine->external_net.busy_in_links ) );
  create_queue( &( machine->external_net.busy_out_links ) );
  create_queue( &( machine->external_net.th_for_in ) );
  create_queue( &( machine->external_net.th_for_out ) );
  machine->external_net.half_duplex_links = FALSE;

  /* Single external network out link  */
  if ( ( link = (struct t_link *)malloc( sizeof( struct t_link ) ) ) == NULL )
  {
    die( "Error reserving memory for machine %d input link", machine_id );
  }

  link->linkid       = 0;
  link->info.machine = machine;
  link->kind         = MACHINE_LINK;
  link->type         = IN_LINK;
  link->thread       = TH_NIL;
  ASS_ALL_TIMER( link->assigned_on, current_time );
  inFIFO_queue( &( machine->external_net.free_in_links ), (char *)link );

  /* Single external network input link */
  if ( ( link = (struct t_link *)malloc( sizeof( struct t_link ) ) ) == NULL )
  {
    die( "Error reserving memory for machine %d output link", machine_id );
  }

  link->linkid       = 1;
  link->info.machine = machine;
  link->kind         = MACHINE_LINK;
  link->type         = OUT_LINK;
  link->thread       = TH_NIL;
  ASS_ALL_TIMER( link->assigned_on, current_time );
  inFIFO_queue( &( machine->external_net.free_out_links ), (char *)link );

  /* Dedicated connections information */
  create_queue( &( machine->dedicated_connections.connections ) );
}

void MACHINE_Fill_Machine_Fields( struct t_machine *machine,
                                  char *machine_name,
                                  char *instrumented_architecture,
                                  int number_of_nodes,
                                  int first_node_id,
                                  double network_bandwidth,
                                  int number_of_buses,
                                  int global_operation_model )
{
  machine->name                                  = strdup( machine_name );
  machine->instrumented_arch                     = strdup( instrumented_architecture );
  machine->number_of_nodes                       = number_of_nodes;
  machine->loaded_nodes                          = 0;
  machine->communication.remote_bandwidth        = network_bandwidth;
  machine->communication.num_messages_on_network = number_of_buses;
  machine->communication.global_op_model         = global_operation_model;

  /* Super trick in order to allocate Nodes into a vector instead of list for performance
   * purposes. This is just a workarround, a good design would imply reprogram all
   * configuration module.
   */

  struct t_node *new_nodes = malloc( sizeof( struct t_node ) * ( nodes_size + number_of_nodes ) );
  if ( nodes_size > 0 )
  {
    memcpy( new_nodes, nodes, sizeof( struct t_node ) * nodes_size );
    free( nodes );
  }
  nodes      = new_nodes;
  nodes_size = nodes_size + number_of_nodes;

  machine->first_node_id = first_node_id;
  int node_id;
  for ( node_id = first_node_id; node_id < nodes_size; ++node_id )
  {
    struct t_node *node = &nodes[ node_id ];
    node->nodeid        = node_id;
    NODE_Init_Empty_Node( machine, node );
  }
}
