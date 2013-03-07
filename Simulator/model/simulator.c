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

#include <stdlib.h>
#include <string.h>

#include <list.h>

#include "simulator.h"
#include "machine.h"
#include "node.h"
#include "task.h"

/*
 * Private functions
 */

void Initialize_Empty_Machines(void);
void Initialize_Empty_Dedicated_Connection(void);

/*****************************************************************************
 * Global variables
 ****************************************************************************/
struct t_simulator  Simulator;

struct t_machine              *Machines;
struct t_dedicated_connection *Dedicated_Connections;

#ifdef USE_EQUEUE
Equeue  Node_queue;
#else
struct t_queue  Node_queue;
#endif

struct t_queue  Global_op;
struct t_queue  Port_queue;

dimemas_timer   current_time;
dimemas_timer   final_statistical_time;

/*****************************************************************************
 * Public functions implementation
 ****************************************************************************/

void SIMULATOR_Init(void)
{
  Simulator.number_machines       = 0;

  Simulator.wan.name              = NULL;
  Simulator.wan.traffic_function  = 0;
  Simulator.wan.max_traffic_value = 0;
  Simulator.wan.bandwidth         = 0;
  Simulator.wan.global_op_model   = GOP_MODEL_CTE;
  create_queue (&(Simulator.wan.threads_on_network));
  Simulator.wan.flight_times = NULL;
  create_queue(&Simulator.wan.global_ops_info);

  Simulator.dedicated_connections_count = 0;
  Simulator.threads_count               = 0;
  Simulator.finished_threads_count      = 0;

  /* Initialize global queues */
#ifndef USE_EQUEUE
  create_queue (&Node_queue);
  create_queue (&Ptask_queue);
#else
  create_Equeue (&Node_queue);
  create_Equeue (&Ptask_queue);
#endif

  create_queue (&Port_queue);
  create_queue (&Global_op);
}

void SIMULATOR_set_number_of_machines(int number_of_machines)
{
  size_t i;
  Simulator.number_machines = number_of_machines;

  /* Flight times matrix */
  Simulator.wan.flight_times =
    (double**) malloc( number_of_machines*number_of_machines*sizeof(double));

  if (Simulator.wan.flight_times == NULL)
  {
    die("Not enough memory to initialize machine flight times\n");
  }

  /* Initialize the machines container */
  Machines = (struct t_machine*) malloc(number_of_machines*sizeof(struct t_machine));

  /* And each container */
  for (i = 0; i < Simulator.number_machines; i++)
  {
    MACHINE_Init_Empty_Machine(&Machines[i], i);
  }

  /* Set flight times to 0 */
  bzero(Simulator.wan.flight_times,
        number_of_machines*number_of_machines*sizeof(double));
}

void SIMULATOR_set_wan_name(char* wan_name)
{
  Simulator.wan.name = strdup(wan_name);
}

void SIMULATOR_set_number_of_dedicated_connections(int dedicated_connections_count)
{
  Simulator.dedicated_connections_count = dedicated_connections_count;

  Initialize_Empty_Dedicated_Connection();
}

void SIMULATOR_set_wan_traffic_function(int traffic_function)
{
  Simulator.wan.traffic_function = traffic_function;
}

void SIMULATOR_set_wan_max_traffic_value(double max_traffic_value)
{
  Simulator.wan.max_traffic_value = max_traffic_value;
}

void SIMULATOR_set_wan_bandwidth(double bandwidth)
{
  Simulator.wan.bandwidth = bandwidth;
}

void SIMULATOR_set_wan_global_op_model(int global_op_model)
{
  size_t i;

  Simulator.wan.global_op_model = global_op_model;

  for (i = 0; i < Simulator.number_machines; i++)
  {
    Machines[i].communication.global_op_model = global_op_model;
  }

}

void SIMULATOR_set_wan_flight_times(double** flight_times)
{

}

t_boolean SIMULATOR_machine_exists(int machine_id)
{
  if (machine_id < 0 || machine_id >= Simulator.number_machines)
  {
    return FALSE;
  }

  return TRUE;
}

t_boolean SIMULATOR_set_machine_definition(int    machine_id,
                                           char  *machine_name,
                                           char  *instrumented_architecture,
                                           int    number_of_nodes,
                                           double network_bandwidth,
                                           int    number_of_buses,
                                           int    global_operation_model)
{
  /* We guarantee that the machine id is correct */
  // assert(machine_id >= 0 && machine_id < Simulator.number_machines);

  if (!SIMULATOR_machine_exists(machine_id))
  {
    return FALSE;
  }

  MACHINE_Fill_Machine_Fields(&Machines[machine_id],
                              machine_name,
                              instrumented_architecture,
                              number_of_nodes,
                              network_bandwidth,
                              number_of_buses,
                              global_operation_model);

  return TRUE;
}

t_boolean SIMULATOR_node_exists(int node_id)
{
#ifdef USE_EQUEUE
  if ( (struct t_node*) query_prio_Equeue(&Node_queue, (t_priority) node_id) == N_NIL)
#else
  if ( (struct t_node*) query_prio_queue(&Node_queue, (t_priority) node_id ) == N_NIL)
#endif
  {
    return FALSE;
  }

  return TRUE;
}

t_boolean SIMULATOR_set_node_definition(int            node_id,
                                        char          *node_name,
                                        int            no_processors,
                                        int            no_input,
                                        int            no_output,
                                        double         local_startup,
                                        double         remote_startup,
                                        double         relative,
                                        double         local_bandwith,
                                        double         external_net_startup,
                                        double         local_port_startup,
                                        double         remote_port_startup,
                                        double         local_memory_startup,
                                        double         remote_memory_startup)
{
  struct t_node* node;

  #ifdef USE_EQUEUE
  if ( (node = (struct t_node*) query_prio_Equeue(&Node_queue, (t_priority) node_id)) == NULL)
#else
  if ( (node = (struct t_node*) query_prio_queue(&Node_queue, (t_priority) node_id )) == NULL)
#endif
  {
    return FALSE;
  }

  NODE_Fill_Node_Fields(node,
                        node_name,
                        no_processors,
                        no_input,
                        no_output,
                        local_startup,
                        remote_startup,
                        relative,
                        local_bandwith,
                        external_net_startup,
                        local_port_startup,
                        remote_port_startup,
                        local_memory_startup,
                        remote_memory_startup);

  return TRUE;
}

t_boolean SIMULATOR_dedicated_connection_exists(int d_conn_id)
{
  if (d_conn_id < 0 || d_conn_id >= Simulator.dedicated_connections_count)
  {
    return FALSE;
  }

  return TRUE;
}

/*
 * That shouldn't be here. A new file to manage dedicated connections should
 * be included
 */
t_boolean SIMULATOR_set_dedicated_connection_definition(int    d_conn_id,
                                                  int    s_machine_id,
                                                  int    d_machine_id,
                                                  double bandwidth,
                                                  int    tags_size,
                                                  int   *tags,
                                                  int    first_message_size,
                                                  char   first_size_cond,
                                                  int    operation,
                                                  int    second_message_size,
                                                  char   second_size_cond,
                                                  int    comms_size,
                                                  int   *comm_ids,
                                                  double startup,
                                                  double flight_time)
{
  struct t_dedicated_connection *d_con;

  /* We guarantee that this connection exists */
  assert (d_conn_id < 0 || d_conn_id >= Simulator.dedicated_connections_count);

  d_con = &Dedicated_Connections[d_conn_id];

  d_con->source_id               = s_machine_id;
  d_con->destination_id          = d_machine_id;
  d_con->bandwidth               = bandwidth;
  d_con->number_of_tags          = tags_size;

  d_con->tags                    = (int*) malloc(tags_size*sizeof(int));
  memcpy(d_con->tags, tags, tags_size*sizeof(int));

  d_con->first_message_size      = first_message_size;
  d_con->first_size_condition    = first_size_cond;
  d_con->operation               = operation;
  d_con->second_message_size     = second_message_size;
  d_con->second_size_condition   = second_size_cond;

  d_con->number_of_communicators = comms_size;
  d_con->communicators           = (int*) malloc(comms_size*sizeof(int));
  memcpy(d_con->communicators, comm_ids, comms_size*sizeof(int));

  d_con->startup                 = startup;
  d_con->flight_time             = flight_time;

  insert_queue (&(Machines[s_machine_id].dedicated_connections.connections),
                (char*) d_con,
                (t_priority) (d_con->id));

  insert_queue (&(Machines[d_machine_id].dedicated_connections.connections),
                (char *) d_con,
                (t_priority) (d_con->id));

  return TRUE;
}

/*****************************************************************************
 * Private functions implementation
 ****************************************************************************/

void Initialize_Empty_Machines(void)
{
  int               i;
  struct t_machine *machine;
  struct t_link    *link;

  Machines = (struct t_machine*) malloc(Simulator.number_machines * sizeof(struct t_machine));

  /* Initialize all machine structures with default values */
  for (i = 0; i < Simulator.number_machines; i++)
  {
    MACHINE_Init_Empty_Machine(&Machines[i], i);
  }
}

/*
 * That shouldn't be here. A new file to manage dedicated connections should
 * be included
 */
void Initialize_Empty_Dedicated_Connection(void)
{
   int                            i;
   struct t_dedicated_connection *d_con;
   struct t_link                 *link;

  Dedicated_Connections = (struct t_dedicated_connection*)
    malloc(Simulator.dedicated_connections_count*sizeof(struct t_dedicated_connection));

   for (i = 0; i < Simulator.dedicated_connections_count; i++)
   {
    d_con = &Dedicated_Connections[i];
    d_con->id = i;

    /* Assign default values */
    d_con->source_id               = 0;
    d_con->destination_id          = 0;
    d_con->bandwidth               = 0;
    d_con->tags                    = NULL;
    d_con->number_of_tags          = 0;
    d_con->first_message_size      = 0;
    d_con->first_size_condition    ='\0';
    d_con->operation               = 0;
    d_con->second_message_size     = 0;
    d_con->second_size_condition   ='\0';
    d_con->communicators           = NULL;
    d_con->number_of_communicators = 0;
    d_con->startup                 = 0;
    d_con->flight_time             = 0;
    d_con->half_duplex             = FALSE;

    /* Connection status queues */
    create_queue (&(d_con->free_in_links));
    create_queue (&(d_con->free_out_links));
    create_queue (&(d_con->busy_in_links));
    create_queue (&(d_con->busy_out_links));
    create_queue (&(d_con->th_for_in));
    create_queue (&(d_con->th_for_out));

    /* Input link */
    link = (struct t_link *) malloc (sizeof (struct t_link));
    link->linkid          = 0;
    link->info.connection = d_con;
    link->kind            = CONNECTION_LINK;
    link->type            = IN_LINK;
    link->thread          = TH_NIL;
    ASS_ALL_TIMER (link->assigned_on, current_time);
    insert_queue (&(d_con->free_in_links),
                  (char *)link,
                  (t_priority)(link->linkid));

    /* Output link */
    link = (struct t_link *) malloc (sizeof (struct t_link));
    link->linkid          = 1;
    link->info.connection = d_con;
    link->kind            = CONNECTION_LINK;
    link->type            = OUT_LINK;
    link->thread          = TH_NIL;
    ASS_ALL_TIMER (link->assigned_on, current_time);
    insert_queue (&(d_con->free_out_links), (char *)link, (t_priority)(link->linkid));
    /* S'afegeix la connexio a la llista */
    // insert_queue (connections, (char *) d_con, (t_priority) (d_con->id));
  }
}
