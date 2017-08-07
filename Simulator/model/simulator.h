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

#ifndef _SIMULATOR_H_
#define _SIMULATOR_H_

#include <types.h>
#include "machine.h"

#define NO_NODE_ID -1

struct t_simulator
{
  int             number_machines;      /* Number of machines in wan */

  struct t_wan                         /* General external network */
  {
    char           *name;               /* Name of the Wide Area Network
                                         * simulated */
    int             traffic_function;   /* Function of traffic */
    double          max_traffic_value;  /* Maximum value of the traffic
                                         * function */
    double          bandwidth;          /* Maximum value of external net
                                         * bandwidth */
    int             global_op_model;    /* Global communication model */
    struct t_queue  threads_on_network; /* Cua dels threads que estan utilitzant
                                         * la xarxa externa */
    double        **flight_times;       /* Matriu del flight time entre cada
                                         * parell de maquines */
    struct t_queue  global_ops_info;    /* Parametres de les operacions
                                         * col.lectives */
  } wan;

  int dedicated_connections_count;      /* Number of dedicated connections between
                                         * machines */

  size_t threads_count;                 /* Number of total threads to be
                                         * simulated */

  size_t finished_threads_count;        /* Number of finished simulated threads */

};

/*
 * Global structures used in the simulation
 */
extern struct t_simulator  Simulator;

extern struct t_machine              *Machines;
extern struct t_dedicated_connection *Dedicated_Connections;

#ifdef USE_EQUEUE
extern Equeue  Node_queue;
#else
extern struct t_queue  Node_queue;
#endif

extern struct t_queue  Global_op;
extern struct t_queue  Port_queue;

/*
 * Simulation timers
 */
extern dimemas_timer   current_time;
extern dimemas_timer   final_statistical_time;

void SIMULATOR_Init(const char  *simulator_configuration_filename,
                    const char  *input_tracefile,
                    double parameter_bw,
                    double parameter_lat,
                    int    parameter_predefined_map,
                    int    parameter_tasks_per_node);

char *SIMULATOR_get_configuration_filename(void);
int   SIMULATOR_get_number_of_nodes(void);
int  *SIMULATOR_get_cpus_per_node(void);

void SIMULATOR_set_number_of_machines(int number_of_machines);
void SIMULATOR_set_wan_name(char*);
void SIMULATOR_set_wan_traffic_function(int traffic_function);
void SIMULATOR_set_wan_max_traffic_value(double max_traffic_value);
void SIMULATOR_set_wan_bandwidth(double bandwidth);
void SIMULATOR_set_wan_global_op_model(int global_op_model);
void SIMULATOR_set_wan_flight_times(double** flight_times);
void SIMULATOR_set_number_of_dedicated_connections(int dedicated_connections_count);

t_boolean SIMULATOR_set_wan_definition(char  *wan_name,
                                       int    number_of_machines,
                                       int    number_of_dedicated_connections,
                                       int    function_of_traffic,
                                       double max_traffic_value,
                                       double external_net_bandwidth,
                                       int    communication_group_model);

t_boolean         SIMULATOR_machine_exists(int machine_id);
struct t_machine* SIMULATOR_get_machine(int machine_id);

t_boolean SIMULATOR_set_machine_definition(int    machine_id,
                                           char  *machine_name,
                                           char  *instrumented_architecture,
                                           int    number_of_nodes,
                                           double network_bandwidth,
                                           int    number_of_buses,
                                           int    global_operation_model);

t_boolean SIMULATOR_node_exists(int node_id);
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
                                        double         remote_memory_startup);

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
                                                 double         remote_memory_startup);

t_boolean SIMULATOR_dedicated_connection_exists(int d_conn_id);
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
                                                        double flight_time);

void SIMULATOR_check_correct_definitions(void);

char* SIMULATOR_get_last_error(void);

void SIMULATOR_set_acc_nodes(int node_id,
														 double latency,
														 double memory_latency,
														 double bandwith,
														 int num_acc_buses,
														 double relative);

void SIMULATOR_reset_state();

#endif


