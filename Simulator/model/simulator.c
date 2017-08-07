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
#include <errno.h>
#include <assert.h>

#include <generate_error.h>
#include <list.h>

#include "dimemas_io.h"
#include "simulator.h"
#include "machine.h"
#include "node.h"
#include "task.h"
#include "configuration.h"
#include "new_configuration.h"

#define UNKNOWN_CONFIGURATION   -1
#define OLD_CONFIGURATION_MAGIC "SDDF"
#define OLD_CONFIGURATION       0
#define NEW_CONFIGURATION_MAGIC "#DIMEMAS_CONFIGURATION"
#define NEW_CONFIGURATION       1

/*****************************************************************************
 * Private functions
 ****************************************************************************/

void Initialize_Empty_Machines(void);
void Initialize_Empty_Dedicated_Connection(void);

int       check_configuration_file_type(FILE* configuration_file);
t_boolean d_conn_check_operand  (char oper, int* out_oper);
t_boolean d_conn_check_condition(char cond, int *out_cond);

/*****************************************************************************
 * Private variables
 ****************************************************************************/
static char *configuration_filename;

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

/* Number of machines actually defined in the configuration file */
static int  SIMULATOR_machines_loaded;

/* Number of nodes loaded on each machine */
static int *SIMULATOR_nodes_loaded;

/* Number of dedicated connections actually defined in the configuration file */
static int SIMULATOR_dedicated_connections_loaded;

/* To control the multiple node declaration */
static int   SIMULATOR_last_node_id = 0;

static size_t SIMULATOR_error_size;
static char  *SIMULATOR_error_message;

/*****************************************************************************
 * Public functions implementation
 ****************************************************************************/

void SIMULATOR_Init(const char  *simulator_configuration_filename,
                    const char  *parameter_tracefile,
                    double parameter_bw,
                    double parameter_lat,
                    int    parameter_predefined_map,
                    int    parameter_tasks_per_node)
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

  Simulator.dedicated_connections_count  = 0;
  Simulator.threads_count                = 0;
  Simulator.finished_threads_count       = 0;

  SIMULATOR_machines_loaded              = 0;
  SIMULATOR_dedicated_connections_loaded = 0;

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

  info ("-> Simulator configuration to be read %s\n",
        simulator_configuration_filename);

  FILE *configuration_file;

  configuration_filename = strdup(simulator_configuration_filename);

  if ( (configuration_file = IO_fopen(configuration_filename, "r")) == NULL)
  {
    die("Can't open configuration file %s\n", simulator_configuration_filename);
  }

  switch(check_configuration_file_type(configuration_file))
  {
    case OLD_CONFIGURATION:


      if (IO_fseeko(configuration_file, (off_t) 0, SEEK_SET) == -1)
      {
        die ("Error initializing configuration file %s", strerror(errno));
      }

      if (!CONFIGURATION_parse(configuration_file,
                               parameter_tracefile,
                               parameter_bw,
                               parameter_lat,
                               parameter_predefined_map,
                               parameter_tasks_per_node))
      {
        die("Error parsing configuration file");
      }
      break;

    case NEW_CONFIGURATION:

      if (IO_fseeko(configuration_file, (off_t) 0, SEEK_SET) == -1)
      {
        die ("Error initializing configuration file: %s", strerror(errno));
      }

      if (!NEW_CONFIGURATION_parse(configuration_file,
                                   parameter_tracefile,
                                   parameter_bw,
                                   parameter_lat,
                                   parameter_predefined_map,
                                   parameter_tasks_per_node))
      {
        die("Error parsing configuration file: %s", NEW_CONFIGURATION_get_last_error());
      }
      break;

    default:
      die("Unknown configuration file type %s\n", simulator_configuration_filename);
      break;
  }

  IO_fclose(configuration_file);
}

char* SIMULATOR_Get_Configuration_FileName(void)
{
  return configuration_filename;
}

int  SIMULATOR_get_number_of_nodes(void)
{
  #ifndef USE_EQUEUE
    return (int) count_queue(&Node_queue);
  #else
    return (int) count_Equeue(&Node_queue);
  #endif
}

int *SIMULATOR_get_cpus_per_node(void)
{
  struct t_node  *node;
  struct t_cpu   *cpu;
  int             current_node = 0;
  int            *result = malloc(SIMULATOR_get_number_of_nodes()*sizeof(int));

  for (current_node = 0; current_node < SIMULATOR_get_number_of_nodes(); current_node++)
  {
    #ifndef USE_EQUEUE
      if ( (node = (struct t_node*) query_prio_queue(&Node_queue, (t_priority) current_node )) == NULL)
    #else
      if ( (node = (struct t_node*) query_prio_Equeue(&Node_queue, (t_priority) current_node)) == NULL)
    #endif
    {
        generate_error(&SIMULATOR_error_message, "invalid node id (%d)", current_node);
        return NULL;
    }

    result[current_node] = (int) count_queue (&(node->Cpus));
  }

  return result;
}

void  SIMULATOR_set_number_of_machines(int number_of_machines)
{
  size_t i;
  Simulator.number_machines = number_of_machines;

  /* Flight times matrix */
  Simulator.wan.flight_times = (double**) malloc(number_of_machines*sizeof(double*));

  for (i = 0; i < number_of_machines; i++)
  {
    Simulator.wan.flight_times[i] = (double*) malloc (number_of_machines*sizeof(double));
  }

  if (Simulator.wan.flight_times == NULL)
  {
    die("Not enough memory to initialize machine flight times\n");
  }

  /* Initialize the machines container */
  Machines = (struct t_machine*) malloc(number_of_machines*sizeof(struct t_machine));

  if (Machines == NULL)
  {
    die("Not enough memory to initialize machines description");
  }

  /* And each container */
  for (i = 0; i < Simulator.number_machines; i++)
  {
    MACHINE_Init_Empty_Machine(&Machines[i], i);
  }

  /* Allocate space to check the number of nodes initialized */
  SIMULATOR_nodes_loaded = (int*) malloc(number_of_machines*sizeof(int));
  bzero(SIMULATOR_nodes_loaded, number_of_machines*sizeof(int));

}

void SIMULATOR_set_wan_name(char* wan_name)
{
  if (wan_name == NULL)
  {
    Simulator.wan.name = NULL;
  }
  else
  {
    Simulator.wan.name = strdup(wan_name);
  }
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

t_boolean SIMULATOR_set_wan_definition(char  *wan_name,
                                       int    number_of_machines,
                                       int    number_of_dedicated_connections,
                                       int    function_of_traffic,
                                       double max_traffic_value,
                                       double external_net_bandwidth,
                                       int    communication_group_model)
{
  SIMULATOR_set_wan_name(wan_name);

  if (number_of_machines <= 0)
  {
    generate_error(&SIMULATOR_error_message, "number of machines less than 0");
    return FALSE;
  }

  SIMULATOR_set_number_of_machines(number_of_machines);

  if (number_of_dedicated_connections < 0)
  {
    generate_error(&SIMULATOR_error_message, "number of dedicated connections less than 0");
    return FALSE;
  }

  SIMULATOR_set_number_of_dedicated_connections(number_of_dedicated_connections);

  if (function_of_traffic < 1 || function_of_traffic > 4)
  {
    generate_error(&SIMULATOR_error_message, "unknown function of traffic (%d)", function_of_traffic);
    return FALSE;
  }

  SIMULATOR_set_wan_traffic_function(function_of_traffic);

  SIMULATOR_set_wan_max_traffic_value(max_traffic_value);

  SIMULATOR_set_wan_bandwidth(external_net_bandwidth);

  if (communication_group_model < 1 || communication_group_model > 4)
  {
    generate_error(&SIMULATOR_error_message, "unknown communication group model (%d)", communication_group_model);
    return FALSE;
  }

  SIMULATOR_set_wan_global_op_model(communication_group_model);

  return TRUE;
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

struct t_machine* SIMULATOR_get_machine(int machine_id)
{
  if (machine_id < 0 || machine_id >= Simulator.number_machines)
  {
    return NULL;
  }
  else
  {
    return &Machines[machine_id];
  }
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

  if (machine_id < 0)
  {
    generate_error(&SIMULATOR_error_message, "invalid machine id (%d)", machine_id);
    return FALSE;
  }

  if (!SIMULATOR_machine_exists(machine_id))
  {
    generate_error(&SIMULATOR_error_message, "machine with %d does not exist", machine_id);
    return FALSE;
  }

  if (number_of_nodes <= 0)
  {
    generate_error(&SIMULATOR_error_message, "invalid number of nodes (%d)", number_of_nodes);
    return FALSE;
  }

  if (network_bandwidth < 0)
  {
    generate_error(&SIMULATOR_error_message, "invalid network bandwidth (%lf)", network_bandwidth);
    return FALSE;
  }

  if (number_of_buses < 0)
  {
    generate_error(&SIMULATOR_error_message, "invalid number of buses (%d)", number_of_buses);
    return FALSE;
  }

  if (global_operation_model < 1 || global_operation_model > 3)
  {
    generate_error(&SIMULATOR_error_message, "invalid global operation model (%d)", global_operation_model);
    return FALSE;
  }

  MACHINE_Fill_Machine_Fields(&Machines[machine_id],
                              machine_name,
                              instrumented_architecture,
                              number_of_nodes,
                              network_bandwidth,
                              number_of_buses,
                              global_operation_model);

  SIMULATOR_machines_loaded++;

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
                                        int            machine_id,
                                        char          *node_name,
                                        int            no_processors,
                                        int            no_mem_buses,
                                        int            no_mem_in_links,
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
                                        double         remote_memory_startup)
{
  struct t_node    *node;
  struct t_machine *machine;
  int               effective_node_id;

  if (node_id == NO_NODE_ID)
  {
    effective_node_id = SIMULATOR_last_node_id;
    SIMULATOR_last_node_id++;
  }
  else
  {
    effective_node_id = node_id;
  }

  if ( (machine = SIMULATOR_get_machine(machine_id)) == NULL)
  {
    generate_error(&SIMULATOR_error_message, "invalid machine id (%d)", machine_id);
    return FALSE;
  }

  if ( (machine->loaded_nodes+1) > machine->number_of_nodes)
  {
    generate_error(&SIMULATOR_error_message, "more nodes defined than total machine nodes");
    return FALSE;
  }

  #ifdef USE_EQUEUE
  if ( (node = (struct t_node*) query_prio_Equeue(&Node_queue, (t_priority) node_id)) == NULL)
#else
  if ( (node = (struct t_node*) query_prio_queue(&Node_queue, (t_priority) effective_node_id )) == NULL)
#endif
  {
    generate_error(&SIMULATOR_error_message, "invalid node id (%d)", effective_node_id);
    return FALSE;
  }

  if (no_processors <= 0)
  {
    generate_error(&SIMULATOR_error_message, "invalid number of processors (%d)", no_processors);
    return FALSE;
  }

  if (no_mem_buses < 0)
  {
    generate_error(&SIMULATOR_error_message, "invalid intra-node buses (%d)", no_mem_buses);
    return FALSE;
  }

  if (no_mem_in_links < 0)
  {
    generate_error(&SIMULATOR_error_message, "invalid intra-node input links (%d)", no_mem_in_links);
    return FALSE;
  }

  if (no_mem_out_links < 0)
  {
    generate_error(&SIMULATOR_error_message, "invalid intra-node output links (%d)", no_mem_out_links);
    return FALSE;
  }

  if (no_input < 0)
  {
    generate_error(&SIMULATOR_error_message, "invalid inter-node input links (%d)", no_input);
    return FALSE;
  }

  if (no_output < 0)
  {
    generate_error(&SIMULATOR_error_message, "invalid inter-node output links (%d)", no_output);
    return FALSE;
  }

  if (local_startup < 0)
  {
    generate_error(&SIMULATOR_error_message, "invalid intra-node startup (%lf)", local_startup);
    return FALSE;
  }

  if (remote_startup < 0)
  {
    generate_error(&SIMULATOR_error_message, "invalid inter-node startup (%lf)", remote_startup);
    return FALSE;
  }

  if (relative < 0)
  {
    generate_error(&SIMULATOR_error_message, "invalid CPU ratio (%lf)", relative);
    return FALSE;
  }

  if (local_bandwidth < 0)
  {
    generate_error(&SIMULATOR_error_message, "invalid intra-node bandwidth (%lf)", local_bandwidth);
    return FALSE;
  }

  if (external_net_startup < 0)
  {
    generate_error(&SIMULATOR_error_message, "invalid WAN startup (%lf)", external_net_startup);
    return FALSE;
  }

  // SIMULATOR_last_node_id = effective_node_id;

  NODE_Fill_Node_Fields(node,
                        node_name,
                        no_processors,
                        no_mem_buses,
                        no_mem_in_links,
                        no_mem_out_links,
                        no_input,
                        no_output,
                        local_startup,
                        remote_startup,
                        relative,
                        local_bandwidth,
                        external_net_startup,
                        local_port_startup,
                        remote_port_startup,
                        local_memory_startup,
                        remote_memory_startup);

  machine->loaded_nodes++;

  return TRUE;
}

t_boolean SIMULATOR_set_multiple_node_definition(int            node_count,
                                                 int            machine_id,
                                                 char          *node_name,
                                                 int            no_processors,
                                                 int            no_mem_buses,
                                                 int            no_mem_in_links,
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
                                                 double         remote_memory_startup)
{
  struct t_node    *node;
  struct t_machine *machine;
  size_t         i;

  if ( (machine = SIMULATOR_get_machine(machine_id)) == NULL)
  {
    generate_error(&SIMULATOR_error_message, "invalid machine id (%d)", machine_id);
    return FALSE;
  }

  if ( (machine->loaded_nodes+node_count) > machine->number_of_nodes)
  {
    generate_error(&SIMULATOR_error_message, "more nodes defined than total machine nodes");
    return FALSE;
  }

  if (node_count  <= 0)
  {
    generate_error(&SIMULATOR_error_message, "invalid node count (%d)", node_count);
    return FALSE;
  }

  if (no_processors <= 0)
  {
    generate_error(&SIMULATOR_error_message, "invalid number of processors (%d)", no_processors);
    return FALSE;
  }

  if (no_mem_buses < 0)
  {
    generate_error(&SIMULATOR_error_message, "invalid intra-node buses (%d)", no_mem_buses);
    return FALSE;
  }

  if (no_mem_in_links < 0)
  {
    generate_error(&SIMULATOR_error_message, "invalid intra-node input links (%d)", no_mem_in_links);
    return FALSE;
  }

  if (no_mem_out_links < 0)
  {
    generate_error(&SIMULATOR_error_message, "invalid intra-node output links (%d)", no_mem_out_links);
    return FALSE;
  }

  if (no_input < 0)
  {
    generate_error(&SIMULATOR_error_message, "invalid inter-node input links (%d)", no_input);
    return FALSE;
  }

  if (no_output < 0)
  {
    generate_error(&SIMULATOR_error_message, "invalid inter-node output links (%d)", no_output);
    return FALSE;
  }

  if (local_startup < 0)
  {
    generate_error(&SIMULATOR_error_message, "invalid intra-node startup (%lf)", local_startup);
    return FALSE;
  }

  if (remote_startup < 0)
  {
    generate_error(&SIMULATOR_error_message, "invalid inter-node startup (%lf)", remote_startup);
    return FALSE;
  }

  if (relative < 0)
  {
    generate_error(&SIMULATOR_error_message, "invalid CPU ratio (%lf)", relative);
    return FALSE;
  }

  if (local_bandwidth < 0)
  {
    generate_error(&SIMULATOR_error_message, "invalid intra-node bandwidth (%lf)", local_bandwidth);
    return FALSE;
  }

  if (external_net_startup < 0)
  {
    generate_error(&SIMULATOR_error_message, "invalid WAN startup (%lf)", external_net_startup);
    return FALSE;
  }

  for (i = 0; i < node_count; i++)
  {
#ifdef USE_EQUEUE
    if ( (node = (struct t_node*) query_prio_Equeue(&Node_queue, (t_priority) node_id)) == NULL)
#else
    if ( (node = (struct t_node*) query_prio_queue(&Node_queue, (t_priority) SIMULATOR_last_node_id )) == NULL)
#endif
    {
      generate_error(&SIMULATOR_error_message, "invalid node identifier (%d)", SIMULATOR_last_node_id);
      return FALSE;
    }

    NODE_Fill_Node_Fields(node,
                          node_name,
                          no_processors,
                          no_mem_buses,
                          no_mem_in_links,
                          no_mem_out_links,
                          no_input,
                          no_output,
                          local_startup,
                          remote_startup,
                          relative,
                          local_bandwidth,
                          external_net_startup,
                          local_port_startup,
                          remote_port_startup,
                          local_memory_startup,
                          remote_memory_startup);

    SIMULATOR_last_node_id++;
  }

  machine->loaded_nodes += node_count;

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
                                                        char   operation,
                                                        int    second_message_size,
                                                        char   second_size_cond,
                                                        int    comms_size,
                                                        int   *comm_ids,
                                                        double startup,
                                                        double flight_time)
{
  struct t_dedicated_connection *d_con;
  int in_first_size_cond, in_op, in_second_size_cond;


  /* We guarantee that this connection exists */
  assert (d_conn_id < 0 || d_conn_id >= Simulator.dedicated_connections_count);

  if (!SIMULATOR_dedicated_connection_exists(d_conn_id))
  {
    generate_error(&SIMULATOR_error_message, "invalid identifier (%d)", d_conn_id);
    return FALSE;
  }

  if (!SIMULATOR_machine_exists(s_machine_id))
  {
    generate_error(&SIMULATOR_error_message, "invalid source machine id (%d)", s_machine_id);
    return FALSE;
  }

  if (!SIMULATOR_machine_exists(d_machine_id))
  {
    generate_error(&SIMULATOR_error_message, "invalid destination machine id (%d)", s_machine_id);
    return FALSE;
  }

  if (bandwidth < 0)
  {
    generate_error(&SIMULATOR_error_message, "invalid bandwidth (%lf)", bandwidth);
    return FALSE;
  }

  if (tags_size < 0)
  {
    generate_error(&SIMULATOR_error_message, "invalid tags list size (%d)", tags_size);
    return FALSE;
  }

  if (tags == NULL)
  {
    generate_error(&SIMULATOR_error_message, "invalid tags list");
    return FALSE;
  }

  if (first_message_size < 0)
  {
    generate_error(&SIMULATOR_error_message, "invalid size of first message (%d)", first_message_size);
    return FALSE;
  }

  if (!d_conn_check_condition(first_size_cond, &in_first_size_cond))
  {
    generate_error(&SIMULATOR_error_message, "invalid first message size condition (%c)", first_size_cond);
    return FALSE;
  }

  if (!d_conn_check_operand(operation, &in_op))
  {
    generate_error(&SIMULATOR_error_message, "invalid operation (%c)", operation);
    return FALSE;
  }

  if (second_message_size < 0)
  {
    generate_error(&SIMULATOR_error_message, "invalid size of second message (%d)", second_message_size);
    return FALSE;
  }

  if (!d_conn_check_condition(second_size_cond, &in_second_size_cond))
  {
    generate_error(&SIMULATOR_error_message, "invalid second message size condition (%c)", second_size_cond);
    return FALSE;
  }

  if (comms_size < 0)
  {
    generate_error(&SIMULATOR_error_message, "invalid communicators list size (%d)", comms_size);
    return FALSE;
  }

  if (comm_ids == NULL)
  {
    generate_error(&SIMULATOR_error_message, "invalid communicators list");
    return FALSE;
  }

  if (startup < 0)
  {
    generate_error(&SIMULATOR_error_message, "invalid startup (%lf)", startup);
    return FALSE;
  }

  if (flight_time < 0)
  {
    generate_error(&SIMULATOR_error_message, "invalid flight time (%lf)", flight_time);
    return FALSE;
  }

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

  SIMULATOR_dedicated_connections_loaded++;

  return TRUE;
}

void SIMULATOR_check_correct_definitions(void)
{
  struct t_machine *current_machine;
  int    i;

  if (Simulator.number_machines != SIMULATOR_machines_loaded)
  {
    die("%d machines listed but only %d defined. Check your configuration file\n",
        Simulator.number_machines,
        SIMULATOR_machines_loaded);
  }

  for ( i = 0; i < Simulator.number_machines; i++)
  {
    current_machine = &Machines[i];

    if (current_machine->loaded_nodes != current_machine->number_of_nodes)
    {
      die("Machine %d should have %d nodes, but only %d have been defined. Check your configuration file\n",
          i,
          current_machine->number_of_nodes,
          current_machine->loaded_nodes);
    }
  }

  if (Simulator.dedicated_connections_count != SIMULATOR_dedicated_connections_loaded)
  {
    die("%d dedicated connections but only %d defined. Check your configuration file\n",
        Simulator.dedicated_connections_count,
        SIMULATOR_dedicated_connections_loaded);
  }

  // return TRUE;
}

void SIMULATOR_set_acc_nodes(int node_id,
														 double latency,
														 double memory_latency,
														 double bandwith,
														 int num_acc_buses,
														 double relative) {
	if (!SIMULATOR_node_exists(node_id))
	{
		die("Wrong accelerator node id %d, does no exist. Check your configuration file\n", node_id);
	}
	if (latency < 0.0)
	{
		die("invalid latency value (%lf) in accelerator node (%d)", latency, node_id);
	}
	if (memory_latency < 0.0)
	{
		die("invalid memory latency value (%lf) in accelerator node (%d)", latency, node_id);
	}
	if (bandwith < 0.0)
	{
		die("invalid bandiwth (%lf) in accelerator node (%d)", bandwith, node_id);
	}

	if (num_acc_buses < 0)
	{
		die("invalid number of accelerator buses (%d) in accelerator node (%d)",
				num_acc_buses, node_id);
	}

	if (relative < 0)
	{
		die("invalid speed ratio value (%lf) in accelerator node (%d)",
				relative, num_acc_buses);
	}

	NODE_set_acc(node_id, latency, memory_latency, bandwith, num_acc_buses, relative);
}

char* SIMULATOR_get_last_error(void)
{
  return SIMULATOR_error_message;
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

int check_configuration_file_type(FILE* configuration_file)
{
  char buffer[32];

  if (fgets(buffer, 32, configuration_file) == NULL)
  {
    die("Error reading the MAGIC NUMBER of the configuration file");
  }

  if(strstr(buffer, NEW_CONFIGURATION_MAGIC) != NULL)
  {
    return NEW_CONFIGURATION;
  }
  else if(strstr(buffer, OLD_CONFIGURATION_MAGIC) != NULL)
  {
    return OLD_CONFIGURATION;
  }

  printf("Magic Buffer = %s\n", buffer);

  return UNKNOWN_CONFIGURATION;
}

t_boolean d_conn_check_operand  (char oper, int* out_oper)
{
  if (out_oper == NULL)
    return FALSE;

  if (oper =='<')
  {
    *out_oper = 0;
    return TRUE;
  }
  else if (oper == '=')
  {
    *out_oper = 1;
    return TRUE;
  }
  else if (oper =='>')
  {
    *out_oper = 2;
    return TRUE;
  }

  return FALSE;
}

t_boolean d_conn_check_condition(char cond, int *out_cond)
{
  if (out_cond == NULL)
    return FALSE;

  if (cond == '&')
  {
    *out_cond = 0;
    return TRUE;
  }
  else if (cond == '|')
  {
    *out_cond = 1;
    return TRUE;
  }

  return TRUE;
}

void SIMULATOR_reset_state()
{
  current_time = 0;
  remove_queue_elements(&Simulator.wan.threads_on_network);
  Simulator.finished_threads_count = 0;

  // Reset nodes info
  int node_id;
  for (node_id = 0; node_id < count_queue(&Node_queue); node_id++)
  {
    struct t_node * node = (struct t_node*) query_prio_queue(&Node_queue, (t_priority) node_id);

    move_queue_elements(&node->busy_in_links, &node->free_in_links);
    move_queue_elements(&node->busy_out_links, &node->free_out_links);

    remove_queue_elements(&node->th_for_in);
    remove_queue_elements(&node->th_for_out);

    remove_queue_elements(&node->wait_for_mem_bus);
    remove_queue_elements(&node->threads_in_memory);

    remove_queue_elements(&node->ready);

    struct t_cpu * cpu;
    for (cpu = (struct t_cpu *)head_queue(&node->Cpus); 
        cpu != C_NIL; 
        cpu = (struct t_cpu *)next_queue(&node->Cpus))
    {
      cpu->current_thread = TH_NIL;
    }
  }

  // Reset Ptask info
  struct t_Ptask *Ptask;
  for (Ptask  = (struct t_Ptask *) head_queue (&Ptask_queue);
       Ptask != P_NIL;
       Ptask  = (struct t_Ptask *) next_queue (&Ptask_queue))
  {
    struct t_communicator * communicator;
    for (communicator  = (struct t_communicator *)head_queue(&Ptask->Communicator);
         communicator != (struct t_communicator *)0;
         communicator  = (struct t_communicator *)next_queue(&Ptask->Communicator))
    {
      communicator->in_flight_op = FALSE;
      remove_queue_elements(&communicator->threads);
    }

    remove_queue_elements(&Ptask->global_operation);

    int tasks_it;
    for (tasks_it = 0; tasks_it < Ptask->tasks_count; tasks_it++)
    {
      struct t_task * task = &(Ptask->tasks[tasks_it]);

      move_queue_elements(&task->busy_in_links, &task->free_in_links);
      move_queue_elements(&task->busy_out_links, &task->busy_out_links);

      remove_queue_elements(&task->th_for_in);
      remove_queue_elements(&task->th_for_out);

      remove_queue_elements(&task->mess_recv);
      remove_queue_elements(&task->recv);
      remove_queue_elements(&task->send);
      remove_queue_elements(&task->recv_without_send);
      remove_queue_elements(&task->send_without_recv);
      remove_queue_elements(&task->irecvs_executed);
      remove_queue_elements(&task->semaphores); // ??

      task->current_wait = TH_NIL;

      int th_id;
      for (th_id = 0; th_id < task->threads_count; ++th_id)
      {
        struct t_thread * thread = task->threads[th_id];

        remove_queue_elements(&thread->recv_without_send);
        remove_queue_elements(&thread->send_without_recv);
        remove_queue_elements(&thread->irecvs_executed);

        if (thread->action != A_NIL)
        {
          if (thread->action->next != A_NIL)
          {
            free(thread->action->next);
            thread->action->next = A_NIL;
          }
          free(thread->action);
          thread->action = A_NIL;
        }
        if (thread->last_action != A_NIL)
        {
          free(thread->last_action);
          thread->last_action = A_NIL;
        }
        /*if (thread->pending_action != A_NIL)
        {
          free(thread->pending_action);
          thread->pending_action = A_NIL;
        }*/

        if (thread->original_thread && thread->twin_thread != TH_NIL)
        {
          free(thread->twin_thread);
          thread->twin_thread = TH_NIL;
        }

        thread->last_paraver = 0;
        thread->loose_cpu = FALSE;
        thread->doing_context_switch = FALSE;
        thread->to_be_preempted = FALSE;
        thread->doing_busy_wait = FALSE;
        thread->doing_startup = FALSE;
        thread->startup_done = FALSE;
        thread->doing_copy = FALSE;
        thread->copy_done = FALSE;
        thread->doing_roundtrip = FALSE;
        thread->roundtrip_done = FALSE;
        thread->counter_ops_already_ignored = 0;
        thread->counter_ops_already_injected = 0;

        // Move the file pointer to the beggining of the operations of this thread
        DAP_restart_fps(Ptask->Ptaskid, tasks_it, th_id);

        // The file pointer is now at the beggining of operations. We can
        // start again to read it.
        READ_get_next_action(thread);
        FIFO_thread_to_ready(thread);
      }
    }
  }
}
