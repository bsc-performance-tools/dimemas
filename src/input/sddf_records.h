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

/* Configuration file */

/* Record 0: WAN information */
#define SDDFA_0C_NAME "wide area network information"
#define SDDFA_0C_1A   "wan_name"
#define SDDFA_0C_2A   "number_of_machines"
#define SDDFA_0C_3A   "number_dedicated_connections"
#define SDDFA_0C_4A   "function_of_traffic"
#define SDDFA_0C_5A   "max_traffic_value"
#define SDDFA_0C_6A   "external_net_bandwidth"
#define SDDFA_0C_7A   "communication_group_model"

/* Record 1: Environment information */
#define SDDFA_1C_NAME "environment information"
#define SDDFA_1C_1A "machine_name"
#define SDDFA_1C_2A "machine_id"
#define SDDFA_1C_3A "instrumented_architecture"
#define SDDFA_1C_4A "number_of_nodes"
#define SDDFA_1C_5A "network_bandwidth"
#define SDDFA_1C_6A "number_of_buses_on_network"
#define SDDFA_1C_7A "communication_group_model"

/* Record 2: Node information */
#define SDDFA_2C_NAME  "node information"
#define SDDFA_2C_1A  "machine_id"
#define SDDFA_2C_2A  "node_id"
#define SDDFA_2C_3A  "simulated_architecture"
#define SDDFA_2C_4A  "number_of_processors"
#define SDDFA_2C_5A  "number_of_input_links"
#define SDDFA_2C_6A  "number_of_output_links"
#define SDDFA_2C_7A  "startup_on_local_communication"
#define SDDFA_2C_8A  "startup_on_remote_communication"
#define SDDFA_2C_9A  "speed_ratio_instrumented_vs_simulated"
#define SDDFA_2C_10A "memory_bandwidth"
#define SDDFA_2C_11A "external_net_startup"
#define SDDFA_2C_12A "local_port_startup"
#define SDDFA_2C_13A "remote_port_startup"
#define SDDFA_2C_14A "local_memory_startup"
#define SDDFA_2C_15A "remote_memory_startup"
  
  
/* Record 3 */
#define SDDFA_3C_NAME "mapping information"
#define SDDFA_3C_1A   "tracefile"
#define SDDFA_3C_2A   "number_of_tasks"
#define SDDFA_3C_3A   "mapping_tasks_to_nodes"
#define SDDFA_3C_4A   "priority"

/* Record 4 */
#define SDDFA_4C_NAME "configuration files"
#define SDDFA_4C_1A   "scheduler"
#define SDDFA_4C_2A   "file_system"
#define SDDFA_4C_3A   "communication"
#define SDDFA_4C_4A   "sensitivity"

/* Record 5 */
#define SDDFA_5C_NAME   "modules information"
#define SDDFA_5C_1A     "type"
#define SDDFA_5C_2A     "value"
#define SDDFA_5C_3A     "execution_ratio"

/* Record 6 */
#define SDDFA_6C_NAME   "file system parameters"
#define SDDFA_6C_1A     "disk latency"
#define SDDFA_6C_2A     "disk bandwidth"
#define SDDFA_6C_3A     "block size"
#define SDDFA_6C_4A     "concurrent requests"
#define SDDFA_6C_5A     "hit ratio"

/* Record 7 */
#define SDDFA_7C_NAME "dedicated connection information"
#define SDDFA_7C_1A   "connection_id"
#define SDDFA_7C_2A   "source_machine"
#define SDDFA_7C_3A   "destination_machine"
#define SDDFA_7C_4A   "connection_bandwidth"
#define SDDFA_7C_5A   "tags_list"
#define SDDFA_7C_6A   "first_message_size"
#define SDDFA_7C_7A   "first_size_condition"
#define SDDFA_7C_8A   "operation"
#define SDDFA_7C_9A   "second_message_size"
#define SDDFA_7C_10A  "second_size_condition"
#define SDDFA_7C_11A  "list_communicators"
#define SDDFA_7C_12A  "connection_startup"
#define SDDFA_7C_13A  "flight_time"
