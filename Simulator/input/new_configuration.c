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

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <float.h>
#include <list.h>
#include <generate_error.h>
#include <sddf_records.h>
#include <configuration.h>
#include <check.h>
#include <dimemas_io.h>
#include <simulator.h>
#include <machine.h>
#include <random.h>
#include <sched_vars.h>
#include <task.h>
#include <subr.h>
#include <fs.h>
#include <file_data_access.h>

static int   current_line;
static char *error_message;

static char   *NEW_CONFIGURATION_parameter_tracefile      = (char*) 0;
static double  NEW_CONFIGURATION_parameter_bw             = DBL_MIN;
static double  NEW_CONFIGURATION_parameter_lat            = DBL_MIN;
static int     NEW_CONFIGURATION_parameter_predefined_map = MAP_NO_PREDEFINED;
static int     NEW_CONFIGURATION_parameter_tasks_per_node;
static int     NEW_CONFIGURATION_mappings_read   = 0;
t_boolean      NEW_CONFIGURATION_old_nodes_defined = FALSE;

/*
 * Private function definitions
 */
t_boolean parse_record          (char* current_record);

t_boolean parse_wan_info        (char* current_record);
t_boolean parse_env_info        (char* current_record);
t_boolean parse_node_info       (char* current_record);
t_boolean parse_multinode_info  (char* current_record);
t_boolean parse_map_info        (char* current_record);
t_boolean parse_predef_map_info (char* current_record);
t_boolean parse_conf_files      (char* current_record);
t_boolean parse_mod_info        (char* current_record);
t_boolean parse_fs_params       (char* current_record);
t_boolean parse_d_conn          (char* current_record);
t_boolean parse_acc_nodes				(char *record_fields);

t_boolean NEW_CONFIGURATION_load_mapping(char* tracefile,
                                         int*  tasks_mapping,
                                         int   tasks_mapping_length,
                                         int   predefined_map,
                                         int   tasks_per_node);

t_boolean NEW_CONFIGURATION_load_tasks_mapping_array(char* tracefile,
                                                     int*  tasks_mapping,
                                                     int   tasks_mapping_length);

t_boolean is_empty(char *str);
char     *erase_quotations(char* in_str);
char     *trim_string(char *in_str);

size_t trimwhitespace(char *out, size_t len, const char *str);
char** str_split( char* str, char delim, int* numSplits );


/*****************************************************************************
 * Public functions implementation
 ****************************************************************************/
t_boolean NEW_CONFIGURATION_parse(FILE  *configuration_file,
                                  const char  *parameter_tracefile,
                                  double parameter_bw,
                                  double parameter_lat,
                                  int    parameter_predefined_map,
                                  int    parameter_tasks_per_node)
{
  t_boolean reading_records_definition = FALSE;

  char   *line_buffer = NULL, *trim_line = NULL;
  size_t  line_length = 0;
  ssize_t bytes_read;

  current_line = 1;

  if (parameter_tracefile != (char*) 0)
  {
    NEW_CONFIGURATION_parameter_tracefile = strdup(parameter_tracefile);
  }

  NEW_CONFIGURATION_parameter_bw             = parameter_bw;
  NEW_CONFIGURATION_parameter_lat            = parameter_lat;
  NEW_CONFIGURATION_parameter_predefined_map = parameter_predefined_map;
  NEW_CONFIGURATION_parameter_tasks_per_node = parameter_tasks_per_node;

  // Header
  getline(&line_buffer, &line_length, configuration_file);

  while ( (bytes_read = getline(&line_buffer, &line_length, configuration_file)) != -1)
  {
    current_line++;

    if (line_length == 1 && line_buffer[0] == '\n')
    {
      // blank line!
      continue;
    }
    else if (line_length > 0)
    {
      trim_line = trim_string(line_buffer);

      if (trim_line == NULL)
      { // blank line
      }
      else if (strlen(trim_line) == 0)
      {
        // blank line
      }
      else if(strstr(trim_line, "//") != NULL)
      {
        // comment
      }
      else if (strstr(trim_line, "/*") != NULL)
      {
        reading_records_definition = TRUE;
      }
      else if (strstr(trim_line, "*/") != NULL)
      {
        reading_records_definition = FALSE;
      }
      else
      {
        if (!reading_records_definition)
        {
          // actual record line
          if (!parse_record(trim_line))
          {
            free(trim_line);
            return FALSE;
          }
        }
      }
    }
  }

  if (bytes_read == -1 && !feof(configuration_file))
  {
    generate_error(&error_message, "Error reading configuration file: %s", strerror(errno));
    return FALSE;
  }

  SIMULATOR_check_correct_definitions();

  if (NEW_CONFIGURATION_old_nodes_defined)
  {
    warning("\n");
    warning("*************************** INFORMATION ********************************\n");
    warning("Old nodes definition (no intra node contention available). This\n");
    warning("simulation will use default values (intra node buses = # of processors\n");
    warning("and 1 half-duplex link per CPU).\n");
    warning("\n");
    warning("Please contact with 'tools@bsc.es' to check how to update this\n");
    warning("configuration file\n");
    warning("************************************************************************\n");
    warning("\n");
  }

  return TRUE;
}

char* NEW_CONFIGURATION_get_last_error(void)
{
  return error_message;
}

/*****************************************************************************
 * Private functions implementation
 ****************************************************************************/
/* RECORD TYPES */
static const char WAN_TEXT[]        = "\"wide area network information\" {%[^}]};;\n";
static const char ENV_TEXT[]        = "\"environment information\" {%[^}]};;\n";
static const char NODE_TEXT[]       = "\"node information\" {%[^}]};;\n";
static const char MULTINODE_TEXT[]  = "\"multi node information\" {%[^}]};;\n";
static const char MAP_TEXT[]        = "\"mapping information\" {%[^}]};;\n";
static const char PREDEF_MAP_TEXT[] = "\"predefined mapping information\" {%[^}]};;\n";
static const char CONF_FILES_TEXT[] = "\"configuration files\" {%[^}]};;\n";
static const char MOD_INFO_TEXT[]   = "\"modules information\" {%[^}]};;\n";
static const char FS_PARAMS_TEXT[]  = "\"file system parameters\" {%[^}]};;\n";
static const char D_CONN_TEXT[]     = "\"dedicated connection information\" {%[^}]};;\n";
static const char ACC_NODES_TEXT[]  = "\"accelerator node information\" {%[^}]};;\n";

t_boolean parse_record(char* current_record)
{
  char record_fields[strlen(current_record)];

  if (sscanf(current_record, WAN_TEXT, record_fields) == 1)
  {
    return parse_wan_info(record_fields);
  }
  else if (sscanf(current_record, ENV_TEXT, record_fields) == 1)
  {
    return parse_env_info(record_fields);
  }
  else if (sscanf(current_record, NODE_TEXT, record_fields) == 1)
  {
    return parse_node_info(record_fields);
  }
  else if (sscanf(current_record, MULTINODE_TEXT, record_fields) == 1)
  {
    return parse_multinode_info(record_fields);
  }
  else if (sscanf(current_record, MAP_TEXT, record_fields) == 1)
  {
    return parse_map_info(record_fields);
  }
  else if (sscanf(current_record, PREDEF_MAP_TEXT, record_fields) == 1)
  {
    return parse_predef_map_info(record_fields);
  }
  else if (sscanf(current_record, CONF_FILES_TEXT, record_fields) == 1)
  {
    return parse_conf_files(record_fields);
  }
  else if (sscanf(current_record, MOD_INFO_TEXT, record_fields) == 1)
  {
    return parse_mod_info(record_fields);
  }
  else if (sscanf(current_record, FS_PARAMS_TEXT, record_fields) == 1)
  {
    return parse_fs_params(record_fields);
  }
  else if (sscanf(current_record, D_CONN_TEXT, record_fields) == 1)
  {
    return parse_d_conn(record_fields);
  }
  else if (sscanf(current_record, ACC_NODES_TEXT, record_fields) == 1)
  {
  	return parse_acc_nodes(record_fields);
  }

  generate_error(&error_message,
                 "Unkwon configuration record type at line %d",
                 current_line);

  return FALSE;
}

/*
"wide area network information" {
  char "wan_name"[];
  int "number_of_machines";
  int "number_dedicated_connections";
  // "options: 1 EXP, 2 LOG, 3 LIN, 4 CT"
  int "function_of_traffic";
  double "max_traffic_value";
  double "external_net_bandwidth";
  // "1 Constant, 2 Lineal, 3 Logarithmic"
  int "communication_group_model";
};;
*/
t_boolean parse_wan_info (char* record_fields)
{
  char  *fields_array;
  int    fields_count;

  int    matches;
  char   wan_name[strlen(record_fields)];
  int    number_of_machines, number_of_dedicated_connections, function_of_traffic;
  double max_traffic_value, external_net_bandwidth;
  int    communication_group_model;

  matches = sscanf (record_fields,
                    "%[^,], %d, %d, %d, %lf, %lf, %d",
                    wan_name,
                    &number_of_machines,
                    &number_of_dedicated_connections,
                    &function_of_traffic,
                    &max_traffic_value,
                    &external_net_bandwidth,
                    &communication_group_model);
  if (matches == 7)
  {
    if (!SIMULATOR_set_wan_definition(erase_quotations(trim_string(wan_name)),
                                      number_of_machines,
                                      number_of_dedicated_connections,
                                      function_of_traffic,
                                      max_traffic_value,
                                      external_net_bandwidth,
                                      communication_group_model))
    {
      generate_error(&error_message,
                     "Wrong wide area network information record values at line %d (%s)",
                     current_line,
                     SIMULATOR_get_last_error());

      return FALSE;
    }

    return TRUE;
  }
  else
  {
    printf("MATCHES = %d \n", matches);
    generate_error(&error_message,
                   "Wrong wide area network information record structure at line %d",
                   current_line);

    return FALSE;
  }
}

/*
"environment information" {
char "machine_name"[];
int "machine_id";
char    "instrumented_architecture"[];
int     "number_of_nodes";
double  "network_bandwidth";
int "number_of_buses_on_network";
	// "1 Constant, 2 Lineal, 3 Logarithmic"
int "communication_group_model";
};;
*/
t_boolean parse_env_info (char* record_fields)
{
  int    matches;
  char   machine_name[strlen(record_fields)];
  int    machine_id;
  char   instrumented_architecture[strlen(record_fields)];
  int    number_of_nodes;
  double network_bandwidth;
  int    number_of_buses_on_network, communication_group_model;

  matches = sscanf(record_fields, "%[^,], %d,%[^,], %d, %lf, %d, %d",
                    machine_name,
                   &machine_id,
                    instrumented_architecture,
                   &number_of_nodes,
                   &network_bandwidth,
                   &number_of_buses_on_network,
                   &communication_group_model);

  if (matches == 7)
  {
    /* Check the possible parameter override of the bandwidth */
    if (NEW_CONFIGURATION_parameter_bw != DBL_MIN)
    {
      network_bandwidth = NEW_CONFIGURATION_parameter_bw;
    }

    if (!SIMULATOR_set_machine_definition(machine_id,
                                          erase_quotations(trim_string(machine_name)),
                                          erase_quotations(trim_string(instrumented_architecture)),
                                          number_of_nodes,
                                          network_bandwidth,
                                          number_of_buses_on_network,
                                          communication_group_model))
    {
      generate_error(&error_message,
                     "Wrong environment information record values at line %d",
                     current_line);

      return FALSE;
    }
    return TRUE;
  }
  else
  {
    generate_error(&error_message,
                   "Wrong enviroment information record field count at line %d",
                   current_line);

    return FALSE;
  }

}

/* NEW FORMAT
"node information" {
int    "machine_id";
int    "node_id";
char   "simulated_architecture"[];
int    "number_of_processors";
int    "number_of_input_links";
int    "number_of_output_links";
double "startup_on_local_communication";
double "startup_on_remote_communication";
double "speed_ratio_instrumented_vs_simulated";
double "memory_bandwidth";
double "external_net_startup";
};;
*/

/* NEW FORMAT
"node information" {
int     "machine_id";
int     "node_id";
char    "simulated_architecture"[];
int     "number_of_processors";
double  "speed_ratio_instrumented_vs_simulated";
double  "intra_node_startup";
double  "intra_node_bandwidth";
int     "intra_node_buses";
int     "intra_node_input_links";
int     "intra_node_output_links";
double  "inter_node_startup";
int     "inter_node_input_links";
int     "inter_node_output_links";
double  "wan_startup";
};;
*/

t_boolean parse_node_info (char* record_fields)
{
  int     matches;

  int     machine_id, node_id;
  char    simulated_architecture[strlen(record_fields)];
  int     number_of_processors;

  double  speed_ratio_instrumented_vs_simulated, intra_node_startup, intra_node_bandwidth;
  int     intra_node_buses, intra_node_input_links, intra_node_output_links;
  double  inter_node_startup;
  int     inter_node_input_links, inter_node_output_links;
  double  wan_startup;

  int     number_of_input_links, number_of_output_links;
  double  startup_on_local_communication;
  double  startup_on_remote_communication;
  double  memory_bandwidth, external_net_startup;

  // First, check the old version fields (including the node id)
  matches = sscanf(record_fields,
                   "%d, %d, %[^,], %d, %lf, %lf, %lf, %d, %d, %d, %lf, %d, %d, %lf",
                   &machine_id,
                   &node_id,
                   simulated_architecture,
                   &number_of_processors,
                   &speed_ratio_instrumented_vs_simulated,
                   &intra_node_startup,
                   &intra_node_bandwidth,
                   &intra_node_buses,
                   &intra_node_input_links,
                   &intra_node_output_links,
                   &inter_node_startup,
                   &inter_node_input_links,
                   &inter_node_output_links,
                   &wan_startup);

  if (matches == 14)
  {
    if (NEW_CONFIGURATION_parameter_lat != DBL_MIN)
    {
      inter_node_startup = NEW_CONFIGURATION_parameter_lat;
    }

    inter_node_startup = inter_node_startup*1e9;
    intra_node_startup = intra_node_startup*1e9;

    /* The node_id is then ignored (NO_NODE_ID) */
    if (!SIMULATOR_set_node_definition (NO_NODE_ID,
                                        machine_id,
                                        erase_quotations(trim_string(simulated_architecture)),
                                        number_of_processors,
                                        intra_node_buses,
                                        intra_node_input_links,
                                        intra_node_output_links,
                                        inter_node_input_links,
                                        inter_node_output_links,
                                        intra_node_startup,
                                        inter_node_startup,
                                        speed_ratio_instrumented_vs_simulated,
                                        intra_node_bandwidth,
                                        wan_startup,
                                        0,
                                        0,
                                        0,
                                        0))
    {
      generate_error(&error_message,
                     "Wrong node information record at line %d (%s)",
                     current_line,
                     SIMULATOR_get_last_error());

      return FALSE;
    }
    else
    {
      return TRUE;
    }
  }

  // First, we check the new version, as it has a large number of fields
  matches = sscanf(record_fields,
                   "%d, %[^,], %d, %lf, %lf, %lf, %d, %d, %d, %lf, %d, %d, %lf",
                   &machine_id,
                   simulated_architecture,
                   &number_of_processors,
                   &speed_ratio_instrumented_vs_simulated,
                   &intra_node_startup,
                   &intra_node_bandwidth,
                   &intra_node_buses,
                   &intra_node_input_links,
                   &intra_node_output_links,
                   &inter_node_startup,
                   &inter_node_input_links,
                   &inter_node_output_links,
                   &wan_startup);

  if (matches == 13)
  {
    if (NEW_CONFIGURATION_parameter_lat != DBL_MIN)
    {
      inter_node_startup = NEW_CONFIGURATION_parameter_lat;
    }

    inter_node_startup = inter_node_startup*1e9;
    intra_node_startup = intra_node_startup*1e9;

    if (!SIMULATOR_set_node_definition (NO_NODE_ID,
                                        machine_id,
                                        erase_quotations(trim_string(simulated_architecture)),
                                        number_of_processors,
                                        intra_node_buses,
                                        intra_node_input_links,
                                        intra_node_output_links,
                                        inter_node_input_links,
                                        inter_node_output_links,
                                        intra_node_startup,
                                        inter_node_startup,
                                        speed_ratio_instrumented_vs_simulated,
                                        intra_node_bandwidth,
                                        wan_startup,
                                        0,
                                        0,
                                        0,
                                        0))
    {
      generate_error(&error_message,
                     "Wrong node information record at line %d (%s)",
                     current_line,
                     SIMULATOR_get_last_error());

      return FALSE;
    }
    else
    {
      return TRUE;
    }
  }

  /* In new configurations, this old version of the records must never appear! */
  matches = sscanf(record_fields,
                   "%d, %d,%[^,], %d, %d, %d, %lf, %lf, %lf, %lf, %lf",
                   &machine_id,
                   &node_id,
                   simulated_architecture,
                   &number_of_processors,
                   &number_of_input_links,
                   &number_of_output_links,
                   &startup_on_local_communication,
                   &startup_on_remote_communication,
                   &speed_ratio_instrumented_vs_simulated,
                   &memory_bandwidth,
                   &external_net_startup);

  if (matches == 11)
  {
    if (is_empty(trim_string(simulated_architecture)))
    {
      simulated_architecture[0] = '\0';
    }

    if (NEW_CONFIGURATION_parameter_lat != DBL_MIN)
    {
      inter_node_startup = NEW_CONFIGURATION_parameter_lat;
    }

    inter_node_startup = inter_node_startup*1e9;
    intra_node_startup = intra_node_startup*1e9;

    if (!SIMULATOR_set_node_definition (NO_NODE_ID,
                                        machine_id,
                                        erase_quotations(trim_string(simulated_architecture)),
                                        number_of_processors,
                                        number_of_processors, // mem_buses = processors
                                        1,             // 1 mem_in_link
                                        0,             // 0 mem_out_link (half-duplex link)
                                        number_of_input_links,
                                        number_of_output_links,
                                        startup_on_local_communication,
                                        startup_on_remote_communication,
                                        speed_ratio_instrumented_vs_simulated,
                                        memory_bandwidth,
                                        external_net_startup,
                                        0,
                                        0,
                                        0,
                                        0))
    {
      generate_error(&error_message,
                     "wrong node information record values at line %d (%s)",
                     current_line,
                     SIMULATOR_get_last_error());
      return FALSE;
    }

    NEW_CONFIGURATION_old_nodes_defined = TRUE;
  }
  else
  {
    generate_error(&error_message,
                   "wrong node information record field count at line %d",
                   current_line);

    return FALSE;
  }

  return TRUE;
}

/*
#2b:
"multi node information" {
  int     "machine_id";
  int     "node_count";
  char    "simulated_architecture"[];
  int     "number_of_processors";
  double  "speed_ratio_instrumented_vs_simulated";
  double  "intra_node_startup";
  double  "intra_node_bandwidth";
  int     "intra_node_buses";
  int     "intra_node_input_links";
  int     "intra_node_output_links";
  double  "inter_node_startup";
  int     "inter_node_input_links";
  int     "inter_node_output_links";
  double  "wan_startup";
};;
*/
t_boolean parse_multinode_info (char* record_fields)
{
  int     matches;

  int     machine_id, node_count;
  char    simulated_architecture[strlen(record_fields)];
  int     number_of_processors;
  double  speed_ratio_instrumented_vs_simulated, intra_node_startup, intra_node_bandwidth;
  int     intra_node_buses, intra_node_input_links, intra_node_output_links;
  double  inter_node_startup;
  int     inter_node_input_links, inter_node_output_links;
  double  wan_startup;

  matches = sscanf(record_fields,
                   "%d, %d,%[^,], %d, %lf, %lf, %lf, %d, %d, %d, %lf, %d, %d, %lf",
                   &machine_id,
                   &node_count,
                   simulated_architecture,
                   &number_of_processors,
                   &speed_ratio_instrumented_vs_simulated,
                   &intra_node_startup,
                   &intra_node_bandwidth,
                   &intra_node_buses,
                   &intra_node_input_links,
                   &intra_node_output_links,
                   &inter_node_startup,
                   &inter_node_input_links,
                   &inter_node_output_links,
                   &wan_startup);

  if (matches == 14)
  {
    if (NEW_CONFIGURATION_parameter_lat != DBL_MIN)
    {
      inter_node_startup = NEW_CONFIGURATION_parameter_lat;
    }

    inter_node_startup = inter_node_startup*1e9;
    intra_node_startup = intra_node_startup*1e9;
    wan_startup = wan_startup*1e9;

    if (!SIMULATOR_set_multiple_node_definition(node_count,
                                                machine_id,
                                                erase_quotations(trim_string(simulated_architecture)),
                                                number_of_processors,
                                                intra_node_buses,
                                                intra_node_input_links,
                                                intra_node_output_links,
                                                inter_node_input_links,
                                                inter_node_output_links,
                                                intra_node_startup,
                                                inter_node_startup,
                                                speed_ratio_instrumented_vs_simulated,
                                                intra_node_bandwidth,
                                                wan_startup,
                                                0,
                                                0,
                                                0,
                                                0))
    {
      generate_error(&error_message,
                    "wrong multi node information record values at line %d (%s)",
                    current_line,
                    SIMULATOR_get_last_error());

      return FALSE;
    }
  }
  else
  {
    generate_error(&error_message,
                   "wrong multi node information record field count at line %d",
                   current_line);

    return FALSE;
  }

  return TRUE;
}


/*
"mapping information" {
  char    "tracefile"[];
  int     "number_of_tasks";
  int     "mapping_tasks_to_nodes"[];
};;
*/
t_boolean parse_map_info (char* record_fields)
{
  int   matches;
  char *tracefile = (char*) malloc (strlen(record_fields)*sizeof(char));
  int   number_of_tasks, array_length, actual_array_length;
  char  mapping_tasks_to_nodes[strlen(record_fields)];

  int *tasks_mapping;


  matches = sscanf(record_fields,
                   "%[^,], %d, [%d] {%[^}]}};;\n",
                   tracefile,
                   &number_of_tasks,
                   &array_length,
                   mapping_tasks_to_nodes);

  if (matches == 4)
  {
    char **mapping_nodes_array;
    int    current_node;
    char  *translate_ptr;
    int    i;

    tasks_mapping = (int*) malloc(array_length*sizeof(int));

    // Mapping tokening
    mapping_nodes_array = str_split(mapping_tasks_to_nodes,
                                   ',',
                                   &actual_array_length);

    if (actual_array_length != array_length)
    {
      generate_error(&error_message,
                     "wrong mapping information record values (incorrect map length) at line %d",
                     current_line);

      return FALSE;
    }

    for (i = 0; i < actual_array_length; i++)
    {
      current_node = strtod(mapping_nodes_array[i], &translate_ptr);

      if (current_node == 0 && translate_ptr == mapping_nodes_array[i])
      {
        generate_error(&error_message,
                       "wrong mapping information record value (node mapping) at line %d",
                       current_line);

        return FALSE;
      }
      tasks_mapping[i] = current_node;
    }
  }
  else
  {
    generate_error(&error_message,
                   "Wrong mapping information record field count at line %d",
                   current_line);

    return FALSE;
  }

  return NEW_CONFIGURATION_load_mapping(erase_quotations(trim_string(tracefile)),
                                        tasks_mapping,
                                        actual_array_length,
                                        MAP_NO_PREDEFINED,
                                        0);
}

/*
"predefined mapping information" {
  char "tracefile"[];
  char "predefined_map"[];
};;
*/
t_boolean parse_predef_map_info (char* record_fields)
{

  int   matches;
  char *tracefile = (char*) malloc(strlen(record_fields)*sizeof(char));
  char  predefined_map[strlen(record_fields)];
  t_boolean tracefile_read = TRUE;
  int tasks_per_node;
  int map_definition;

  matches = sscanf(record_fields,
                   "%[^,]%*[, ]\"%[^\"]\"",
                   tracefile,
                   predefined_map);

  if (matches != 2)
  {
    free(tracefile);
    tracefile = (char*) 0;

    strcpy(predefined_map, erase_quotations(trim_string(record_fields)));
  }

  if (strstr(predefined_map, MAP_FILL_NODES_TXT) != NULL)
  {
    map_definition = MAP_FILL_NODES;
  }
  else if (strstr(predefined_map, MAP_N_TASKS_PER_NODE_TXT) != NULL)
  {
    char *tasks_per_node_str;
    map_definition = MAP_N_TASKS_PER_NODE;
    // get the number of tasks!

    tasks_per_node_str = strtok(predefined_map, " ");

    if (tasks_per_node_str == NULL)
    {
      generate_error(&error_message,
                     "wrong predefined mapping information record values (no tasks per node specified) at line %d",
                     current_line);

      return FALSE;
    }
    else
    {
      if (sscanf(tasks_per_node_str, "%d", &tasks_per_node) != 1)
      {
        generate_error(&error_message,
                       "wrong predefined mapping information record (tasks per node value) at line %d",
                       current_line);

        return FALSE;
      }
    }
  }
  else if (strstr(predefined_map, MAP_INTERLEAVED_TXT) != NULL)
  {
    map_definition = MAP_INTERLEAVED;
  }
  else
  {
    generate_error(&error_message,
                   "wrong predefined mapping information record values (unknown map type %s) at line %d",
                   predefined_map,
                   current_line);

    return FALSE;
  }


  /*
  if (NEW_CONFIGURATION_parameter_tracefile != (char*) 0)
  {
    free(tracefile);
    tracefile = strdup(NEW_CONFIGURATION_parameter_tracefile);
  }

  if (NEW_CONFIGURATION_parameter_predefined_map != MAP_NO_PREDEFINED)
  {
    map_definition = NEW_CONFIGURATION_parameter_predefined_map;
    tasks_per_node = NEW_CONFIGURATION_parameter_tasks_per_node;
  }


  TASK_New_Ptask_predefined_map(erase_quotations(trim_string(tracefile)),
                                map_definition,
                                tasks_per_node);

  */

  return NEW_CONFIGURATION_load_mapping(erase_quotations(trim_string(tracefile)),
                                       (int*) 0,
                                       0,
                                       map_definition,
                                       tasks_per_node);
}

/*
"configuration files" {
  char       "scheduler"[];
  char       "file_system"[];
  char       "communication"[];
  char       "sensitivity"[];
};;
*/
t_boolean parse_conf_files (char* record_fields)
{
  int  matches;
  char scheduler[strlen(record_fields)];
  char file_system[strlen(record_fields)];
  char communication[strlen(record_fields)];
  char random[strlen(record_fields)];

  matches = sscanf(record_fields,
                   "%[^,],%[^,],%[^,],%s",
                   scheduler,
                   file_system,
                   communication,
                   random);

  if (matches == 4)
  {
    if (!is_empty(trim_string(scheduler)))
    {
      CONFIGURATION_Set_Scheduling_Configuration_File(trim_string(scheduler));
    }

    if (!is_empty(trim_string(file_system)))
    {
    	CONFIGURATION_Set_FileSystem_Configuration_File(trim_string(file_system));
    }

    if (!is_empty(trim_string(communication)))
    {
      CONFIGURATION_Set_Communications_Configuration_File(erase_quotations(trim_string(communication)));
    }

    if (!is_empty(trim_string(random)))
    {
      CONFIGURATION_Set_RandomValues_Configuration_File(erase_quotations(trim_string(random)));
    }
  }
  else
  {
    generate_error(&error_message,
                   "wrong configuration files record field count at line %d",
                   current_line);

    return FALSE;
  }

  return TRUE;
}

/*
"modules information" {
  int     "type";
  int     "value";
  double  "execution_ratio";
};;
*/
t_boolean parse_mod_info (char* record_fields)
{
  int    matches;
  int    type, value;
  double execution_ratio;

  matches = sscanf(record_fields,
                   "%d, %d, %lf",
                   &type,
                   &value,
                   &execution_ratio);

  if (matches == 3)
  {
    if (type < 0)
    {
      generate_error(&error_message,
                     "wrong modules files record value (negative type) at line %d",
                     current_line);

      return FALSE;
    }

    if (value < 0)
    {
      generate_error(&error_message,
                     "wrong modules files record value (negative value) at line %d",
                     current_line);

      return FALSE;
    }

    if (execution_ratio >= 0)
    {
      TASK_module_new_ratio (type, value, execution_ratio);
    }
    else
    {
      TASK_module_new_duration (type, value, fabs(execution_ratio));
    }
  }
  else
  {
    generate_error(&error_message,
                   "wrong modules files record field count at line %d",
                   current_line);

    return FALSE;
  }

  return TRUE;
}

/*
"file system parameters" {
  double     "disk latency";
  double     "disk bandwidth";
  double     "block size";
  int        "concurrent requests";
  double     "hit ratio";
};;
*/
t_boolean parse_fs_params (char* record_fields)
{
  int    matches;
  double disk_latency, disk_bandwidth,  block_size;
  int    concurrent_requests;
  double hit_ratio;

  matches = sscanf(record_fields,
                   "%lf, %lf, %lf, %d, %lf",
                   &disk_latency,
                   &disk_bandwidth,
                   &block_size,
                   &concurrent_requests,
                   &hit_ratio);

  if (matches == 5)
  {
    if (disk_latency < 0)
    {
      generate_error(&error_message,
                     "Wrong file system parameters record values (disk latency) at line %d",
                     current_line);

      return FALSE;
    }

    if (disk_bandwidth < 0)
    {
      generate_error(&error_message,
                     "Wrong file system parameters record values (disk bandwidth) at line %d",
                     current_line);

      return FALSE;
    }

    if (block_size < 0)
    {
      generate_error(&error_message,
                     "Wrong file system parameters record values (block size) at line %d",
                     current_line);

      return FALSE;
    }

    if (concurrent_requests < 0)
    {
      generate_error(&error_message,
                     "Wrong file system parameters record values (concurrent values) at line %d",
                     current_line);

      return FALSE;
    }

    if (hit_ratio < 0 || hit_ratio > 1)
    {
      generate_error(&error_message,
                     "Wrong file system parameters record values (hit ratio) at line %d",
                     current_line);

      return FALSE;
    }

    FS_Parameters (disk_latency,
                   disk_bandwidth,
                   block_size,
                   concurrent_requests,
                   hit_ratio);
  }
  else
  {
    generate_error(&error_message,
                   "Wrong file system parameters record field count at line %d",
                   current_line);

    return FALSE;
  }

  return TRUE;
}

/*
"dedicated connection information" {
  int "connection_id";
  int "source_machine";
  int "destination_machine";
  double "connection_bandwidth";
  int "tags_list"[];
  int "first_message_size";
  // "first_size_condition" "size condition that should meet messages to use the connection"
        // "it can be <, =, > and its is referent to message_size
  char "first_size_condition"[];
  // "operation" "& AND, | OR"
  char "operation"[];
  int "second_message_size";
  // "second_size_condition" "size condition that should meet messages to use the connection"
        // "it can be <, =, > and its is referent to message_size"
  char "second_size_condition"[];
  int "list_communicators"[];
  double "connection_startup";
  double "flight_time";
};;
*/
t_boolean parse_d_conn (char* record_fields)
{
  int    matches;
  int    connection_id, source_machine, destination_machine;
  double connection_bandwidth;
  int    tags_list_length;
  char   tags_list[strlen(record_fields)];
  int    first_message_size;
  char   first_size_condition;
  char   operation;
  int    second_message_size;
  char   second_size_condition;
  int    list_communicators_length;
  char   list_communicators[strlen(record_fields)];
  double connection_startup, flight_time;

  matches = sscanf(record_fields,
                   "%d, %d, %d, %lf, [%d] {%[^}]}, %d, %c, %c, %d, %c, [%d] {%[^}]}, %lf, %lf",
                   &connection_id,
                   &source_machine,
                   &destination_machine,
                   &connection_bandwidth,
                   &tags_list_length,
                   tags_list,
                   &first_message_size,
                   &first_size_condition,
                   &operation,
                   &second_message_size,
                   &second_size_condition,
                   &list_communicators_length,
                   list_communicators,
                   &connection_startup,
                   &flight_time);
  if (matches == 15)
  {
    char **tags_list_array;
    int    tags_list_array_length;
    int   *tags_list_int;
    char **list_communicators_array;
    int    list_communicators_array_length;
    int   *list_communicators_int;
    int    i;

    /* Translation of tags list to a integer vector*/
    tags_list_array = str_split(tags_list, ',', &tags_list_array_length);
    if (tags_list_array_length != tags_list_length)
    {
      generate_error(&error_message,
                     "Wrong dedicated connection information record values at line %d: tags list lenght do not match description",
                     current_line);
      return FALSE;
    }

    tags_list_int = malloc(tags_list_length*sizeof(int));

    for (i = 0; i < tags_list_length; i++)
    {
      char *translate_ptr;
      tags_list_int[i] = strtod(tags_list_array[i], &translate_ptr);

      if (tags_list_int[i] == 0 && translate_ptr == tags_list_array[i])
      {
        generate_error(&error_message,
                       "Wrong dedicated connection information record values at line %d: wrong tag value (%s)",
                       current_line,
                       tags_list_array[i]);

        return FALSE;
      }
    }

    /* Translation of communicators list to a integer vector */
    list_communicators_array = str_split(list_communicators, ',', &list_communicators_array_length);
    if (list_communicators_array_length != list_communicators_length)
    {
      generate_error(&error_message,
                     "Wrong dedicated connection information record values at line %d: communicators list length do not match description",
                     current_line);
      return FALSE;
    }

    list_communicators_int = malloc(list_communicators_length*sizeof(int));

    for (i = 0; i < list_communicators_length; i++)
    {
      char *translate_ptr;
      list_communicators_int[i] = strtod(list_communicators_array[i], &translate_ptr);

      if (list_communicators_int[i] == 0 && translate_ptr == list_communicators_array[i])
      {
        generate_error(&error_message,
                       "Wrong dedicated connection information record values at line %d: wrong communicator value (%s)",
                       current_line,
                       list_communicators_array[i]);

        return FALSE;
      }
    }

    if (!SIMULATOR_set_dedicated_connection_definition(connection_id,
                                                       source_machine,
                                                       destination_machine,
                                                       connection_bandwidth,
                                                       tags_list_length,
                                                       tags_list_int,
                                                       first_message_size,
                                                       first_size_condition,
                                                       operation,
                                                       second_message_size,
                                                       second_size_condition,
                                                       list_communicators_length,
                                                       list_communicators_int,
                                                       connection_startup,
                                                       flight_time))
    {
      generate_error(&error_message,
                     "Wrong dedicated connection information at line %d: %s",
                     current_line,
                     SIMULATOR_get_last_error());

      return FALSE;
    }
  }
  else
  {
    generate_error(&error_message,
                   "Wrong dedicated connection information record field count at line %d",
                   current_line);

    return FALSE;
  }

  return TRUE;
}


/*
"accelerator node information" {
  int  		"node ID"
  double  "latency time (s) of intra-node communications model"
  double  "memory_latency time (s) of intra-node communication model"
  double  "bandwidth (MBps) of intra-node communications model"
      		"0 means instantaneous communication"
  int			"num of accelerator buses"
  "if no accelerator nodes info and CUDA/OpenCL records -> error"
  double  "accelerator speed ratio wrt. original execution (divisive factor)"
          "0 means instantaneous | negative value means fixed duration in s."
};;
*/
t_boolean parse_acc_nodes(char *record_fields)
{
	int 		matches, node_id, i, num_acc_buses;
	double	bandwith, latency, memory_latency, relative;

  if (sscanf(record_fields, "%d, %lf, %lf, %lf, %d, %lf",
  		&node_id, &latency, &memory_latency, &bandwith, &num_acc_buses, &relative) == 6)
  {
  	latency = latency *1e9;
  	memory_latency = memory_latency * 1e9;
  	SIMULATOR_set_acc_nodes(node_id, latency, memory_latency, bandwith, num_acc_buses, relative);
  	//printf("%lf, %lf, %lf, %d", latency, memory_latency, bandwith, num_acc_buses);
  }
  else
	{
		generate_error(&error_message,
									 "Wrong accelerator nodes information record field count at line %d",
									 current_line);

		return FALSE;
	}
  return TRUE;
}

t_boolean NEW_CONFIGURATION_load_mapping(char* tracefile,
                                         int*  tasks_mapping,
                                         int   tasks_mapping_length,
                                         int   predefined_map,
                                         int   tasks_per_node)
{
  char* effective_tracefile;
  int   effective_tasks_count;

  if (NEW_CONFIGURATION_mappings_read == 0)
  {
    /* Check if there is a tracefile available */
    if (NEW_CONFIGURATION_parameter_tracefile != (char*) 0)
    {
      effective_tracefile = NEW_CONFIGURATION_parameter_tracefile;
    }
    else
    {
      if (tracefile != (char*) 0)
      {
        effective_tracefile = tracefile;
      }
      else if (tracefile == (char*) 0)
      {
        generate_error(&error_message,
                       "no tracefile provided, please use the '--dim' parameter to supply an application trace to simulate");
        return FALSE;
      }
    }

    /* Check the configuration precedence. In the first mapping, command line
     * mapping parameters override the ones present in the configuration file */
    if (NEW_CONFIGURATION_parameter_predefined_map != MAP_NO_PREDEFINED)
    {
      TASK_New_Ptask_predefined_map(effective_tracefile,
                                    NEW_CONFIGURATION_parameter_predefined_map,
                                    NEW_CONFIGURATION_parameter_tasks_per_node);
    }
    else if (tasks_mapping_length !=  0)
    {
      if (!NEW_CONFIGURATION_load_tasks_mapping_array(effective_tracefile,
                                                      tasks_mapping,
                                                      tasks_mapping_length))
      {
        return FALSE;
      }
    }
    else if (predefined_map != MAP_NO_PREDEFINED)
    {
      TASK_New_Ptask_predefined_map(effective_tracefile,
                                    predefined_map,
                                    tasks_per_node);
    }
    else
    {
      generate_error(&error_message,
                     "inconsistent task mapping at line %d",
                     current_line,
                     DATA_ACCESS_get_error());
      return FALSE;
    }
  }
  else
  {
    if (tracefile == (char*) 0)
    {
      generate_error(&error_message,
                     "no trace provided for mapping at line %d",
                     current_line);
      return FALSE;
    }

    if (tasks_mapping_length != 0)
    {
      if (!NEW_CONFIGURATION_load_tasks_mapping_array(tracefile,
                                                      tasks_mapping,
                                                      tasks_mapping_length))
      {
        return FALSE;
      }
    }
    else if (predefined_map != MAP_NO_PREDEFINED)
    {
      TASK_New_Ptask_predefined_map(tracefile,
                                    predefined_map,
                                    tasks_per_node);
    }
    else
    {
      generate_error(&error_message,
                     "inconsistent task mapping at line %d",
                     current_line,
                     DATA_ACCESS_get_error());
      return FALSE;
    }
  }

  NEW_CONFIGURATION_mappings_read++;

  return TRUE;
}


t_boolean NEW_CONFIGURATION_load_tasks_mapping_array(char* tracefile,
                                                     int*  tasks_mapping,
                                                     int   tasks_mapping_length)
{
  int tasks_count;

  if (DATA_ACCESS_get_number_of_tasks(tracefile, &tasks_count) == FALSE)
  {
    generate_error(&error_message,
                   "unable to retrieve number of tasks of trace '%s': %s",
                   tracefile,
                   DATA_ACCESS_get_error());
    return FALSE;
  }

  if (tasks_mapping_length >= tasks_count)
  {
    if (tasks_mapping_length > tasks_count)
    {
      warning("Number of tasks in trace '%s' (%d) is smaller than the mapping defined (%d), using only part of the map\n",
              tracefile,
              tasks_count,
              tasks_mapping_length);
    }

    TASK_New_Ptask(tracefile,
                   tasks_count,
                   tasks_mapping);
  }
  else
  {
    generate_error(&error_message,
                   "number of tasks in trace '%s' (%d) is larger than the tasks mapping (%d) defined at line %d",
                   tracefile,
                   tasks_count,
                   tasks_mapping_length,
                   current_line);
    return FALSE;
  }

  return TRUE;
}
// Checks if the string provided only contains two double quotes
t_boolean is_empty(char *str)
{
  if (str == NULL)
  {
    return TRUE;
  }

  if (strlen(str) == 2 && str[0] == '\"' && str[1] == '\"')
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

char* erase_quotations(char* in_str)
{
  char* result;

  if (in_str == NULL)
  {
    return NULL;
  }

  result = malloc( (strlen(in_str)+1)*sizeof(char) );

  if (strlen(in_str) == 2 && in_str[0] == '\"' && in_str[1] == '\"')
  {
    result[0] = '\0';
  }
  else if (sscanf(in_str, "\"%[^\"]\"", result) != 1)
  {
    result = strdup(in_str);
  }

  return result;
}

char *trim_string(char *in_str)
{
  size_t trim_length;
  char  *result, *trim_str, *buf_location;

  if (in_str == NULL)
  {
    return NULL;
  }

  trim_str     = malloc ( (strlen(in_str)+1)*sizeof(char));
  buf_location = trim_str;

  trim_length = trimwhitespace(trim_str, strlen(in_str)+1, in_str);

  if (trim_str == NULL)
  {
    free(buf_location);
    result = NULL;
  }
  else
  {
    result = strdup(trim_str);
    free(buf_location);
  }

  return result;
}

// Stores the trimmed input string into the given output buffer, which must be
// large enough to store the result.  If it is too small, the output is
// truncated.
//
// Obtained from: http://stackoverflow.com/questions/122616/how-do-i-trim-leading-trailing-whitespace-in-a-standard-way
size_t trimwhitespace(char *out, size_t len, const char *str)
{
  if(len == 0)
    return 0;

  const char *end;
  size_t out_size;

  // Trim leading space
  while(isspace(*str)) str++;

  if(*str == 0)  // All spaces?
  {
    *out = 0;
    return 1;
  }

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace(*end)) end--;
  end++;

  // Set output size to minimum of trimmed string length and buffer size minus 1
  out_size = (end - str) < len-1 ? (end - str) : len-1;

  // Copy trimmed string and add null terminator
  memcpy(out, str, out_size);
  out[out_size] = 0;

  return out_size;
}

// String split obtained from:
// http://stackoverflow.com/questions/9210528/split-string-with-delimiters-in-c (last answer)
char** str_split( char* str, char delim, int* numSplits )
{
    char** ret;
    int retLen;
    char* c;

    if ( ( str == NULL ) ||
        ( delim == '\0' ) )
    {
        /* Either of those will cause problems */
        ret = NULL;
        retLen = -1;
    }
    else
    {
        retLen = 0;
        c = str;

        /* Pre-calculate number of elements */
        do
        {
            if ( *c == delim )
            {
                retLen++;
            }

            c++;
        } while ( *c != '\0' );

        ret = malloc( ( retLen + 1 ) * sizeof( *ret ) );
        ret[retLen] = NULL;

        c = str;
        retLen = 1;
        ret[0] = str;

        do
        {
            if ( *c == delim )
            {
                ret[retLen++] = &c[1];
                *c = '\0';
            }

            c++;
        } while ( *c != '\0' );
    }

    if ( numSplits != NULL )
    {
        *numSplits = retLen;
    }

    return ret;
}
