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
#include <math.h>
#include <float.h>

#include <list.h>

#include "sddf_records.h"
#include "configuration.h"
#include "check.h"
#include "dimemas_io.h"
#include "file_data_access.h" /* risky, to read the number of tasks from the
                                 parameter trace */

/* That shouldn't be used here... */
#include "simulator.h"
#include "machine.h"
#include "task.h"
#include "random.h"
#include "sched_vars.h"

#define BAD_TYPES(a,b) \
{die ("Incompatible types between definition and usage\n");\
 die ("It is %s and must be %s\n", types[a], types[b]); \
 near_line ();\
 return;\
}

extern int   yyparse();
extern FILE *yyin;


/*
 * External definitions
 */
extern int debug; // Global debug

extern struct t_queue Registers_queue; // Defined in 'check.c'

extern char *types[];

/*
 * Global variables
 */


/*
 * Extra configuration files that can be included in the main configuration file
 */

static char *sch_conf_filename    = NULL;
static FILE *sch_conf_file        = NULL;

static char *fs_conf_filename     = NULL;
static FILE *fs_conf_file         = NULL;

static char *comm_conf_filename   = NULL;
static FILE *comm_conf_file       = NULL;

static char *random_conf_filename = NULL;
static FILE *random_conf_file     = NULL;

/*
 * Configuration elemets that can be overridden per parameter
 */
static char  *CONFIGURATION_parameter_tracefile = (char*) 0;
static double CONFIGURATION_parameter_bw;
static double CONFIGURATION_parameter_lat;
static int    CONFIGURATION_parameter_predefined_map;
static int    CONFIGURATION_parameter_tasks_per_node;

static int    CONFIGURATION_mappings_read   = 0;

/*
 * Private function definitions
 */
void new_wan_info      (struct t_queue *q, struct t_entry* en);
void new_env_info      (struct t_queue *q, struct t_entry* en);
void new_node_info     (struct t_queue *q, struct t_entry* en);
void new_OLD_node_info (struct t_queue *q, struct t_entry* en);
void new_map_info      (struct t_queue *q, struct t_entry* en);
void new_conf_files    (struct t_queue *q, struct t_entry* en);
void new_mod_info      (struct t_queue *q, struct t_entry* en);
void new_fs_params     (struct t_queue *q, struct t_entry* en);
void new_d_conn        (struct t_queue *q, struct t_entry* en);

// Code replication from 'new_configuration.c'
void CONFIGURATION_load_mapping(char* tracefile,
                                int*  tasks_mapping,
                                int   tasks_mapping_length);

t_boolean CONFIGURATION_load_tasks_mapping_array(char* tracefile,
                                                 int*  tasks_mapping,
                                                 int   tasks_mapping_length);

  /*****************************************************************************
   * Public functions implementation
   ****************************************************************************/

t_boolean CONFIGURATION_parse(FILE  *configuration_file,
                              char  *parameter_tracefile,
                              double parameter_bw,
                              double parameter_lat,
                              int    parameter_predefined_map,
                              int    parameter_tasks_per_node)
{
  create_queue (&Registers_queue);

  if (parameter_tracefile != (char*) 0)
  {
    CONFIGURATION_parameter_tracefile = strdup(parameter_tracefile);
  }
  CONFIGURATION_parameter_bw             = parameter_bw;
  CONFIGURATION_parameter_lat            = parameter_lat;
  CONFIGURATION_parameter_predefined_map = parameter_predefined_map;
  CONFIGURATION_parameter_tasks_per_node = parameter_tasks_per_node;

  yyin = configuration_file;

  if (yyparse())
  {
    return FALSE;
  }

  SIMULATOR_check_correct_definitions();

  return TRUE;
}

void CONFIGURATION_Set_Scheduling_Configuration_File(char *sch_filename)
{
  if (sch_filename != NULL)
  {
    warning("Using the '-s' scheduling parametrization (%s) instead of the indicated in the configuration file\n",
              sch_conf_filename);
  }

  sch_conf_filename = strdup(sch_filename);

  if ( (sch_conf_file = IO_fopen(sch_conf_filename, "r")) == NULL)
  {
    die("Unable to set and open scheduling configuration file (%s): %s\n",
        sch_filename,
        IO_get_error());
  }
  return;
}

void CONFIGURATION_Set_FileSystem_Configuration_File(char *fs_filename)
{
  fs_conf_filename = strdup(fs_filename);

  if ( (fs_conf_file = IO_fopen(fs_conf_filename, "r")) == NULL)
  {
    die("Unable to set and open file system configuration file (%s): %s\n",
        fs_filename,
        IO_get_error());
  }
  return;
}

void CONFIGURATION_Set_Communications_Configuration_File(char *comm_filename)
{
  comm_conf_filename = strdup(comm_filename);

  if ( (comm_conf_file = IO_fopen(comm_conf_filename, "r")) == NULL)
  {
    die("Unable to set and open communications configuration file (%s): %s\n",
        comm_filename,
        IO_get_error());
  }
  return;
}


void  CONFIGURATION_Set_RandomValues_Configuration_File(char* rand_filename)
{
  random_conf_filename = strdup(rand_filename);

  if ( (random_conf_file = IO_fopen(random_conf_filename, "r")) == NULL)
  {
    die("Unable to set and open random values configuration file (%s): %s\n",
        rand_filename,
        IO_get_error());
  }
  return;
}



void CONFIGURATION_New_Definition(struct t_queue *definition_fields,
                                  struct t_entry *definition_structure)
{
  struct t_field   *f;

  if (count_queue(definition_fields) != count_queue (definition_structure->types))
  {
    near_line();
    die("Insufficient fields on register #%d: %s (Expected %d - Read %d)\n",
                     definition_structure->entryid,
                     definition_structure->name,
                     count_queue(definition_fields),
                     count_queue (definition_structure->types));
    return;
  }

  switch (definition_structure->entryid)
  {
    case WAN_INFORMATION:
      new_wan_info(definition_fields, definition_structure);
      break;
    case ENV_INFORMATION:
      new_env_info(definition_fields, definition_structure);
      break;
    case NODE_INFORMATION:
      new_node_info(definition_fields, definition_structure);
      break;
    case MAP_INFORMATION:
      new_map_info(definition_fields, definition_structure);
      break;
    case CONFIG_FILES:
      new_conf_files(definition_fields, definition_structure);
      break;
    case MOD_INFORMATION:
      new_mod_info(definition_fields, definition_structure);
      break;
    case FS_PARAMETERS:
      new_fs_params(definition_fields, definition_structure);
      break;
    case DED_CONNECTION:
      new_d_conn(definition_fields, definition_structure);
      break;
    default:
      near_line();
      die("Wrong configuration register %d\n",
                       definition_structure->entryid);
      break;
  }

  for (f  = (struct t_field *) outFIFO_queue (definition_fields);
       f != (struct t_field *) 0;
       f  = (struct t_field *) outFIFO_queue (definition_fields))
  {
    if (f->tipo == 3)
    {
      struct t_field* f2;

      for (f2  = (struct t_field *) outFIFO_queue (f->value.arr.q);
           f2 != (struct t_field *) 0;
           f2  = (struct t_field *) outFIFO_queue (f->value.arr.q))
      {
        //free ((char *) f2, sizeof (struct t_field));
        free(f2);
      }
      //free ((char *) f->value.arr.q, sizeof (struct t_queue));
      free(f->value.arr.q);
    }
    // free ((char *) f, sizeof (struct t_field));
    free(f);
  }
  //free ((char *) q, sizeof (struct t_queue));
  free(definition_fields);

  return;
}

void CONFIGURATION_Load_Communications_Configuration(void)
{
  /*
   * Load the communications definition file indicated in the configuration
   * file.
   *
   * NOTE: the file pointer is closed inside!
   */
  if (comm_conf_filename != NULL)
  {
    COMM_CONFIGURATION_Load_General_Comms_Definition(comm_conf_filename,
                                                     comm_conf_file);
  }
  else
  {
    printf ("   * Loading default communications configuration\n");
  }

  /*
   * Load the special file 'params_traffic.cfg' that defines the external
   * network traffic parameters
   */
  COMM_CONFIGURATION_Load_External_Network_Parameters();

  return;
}

void CONFIGURATION_Load_Scheduler_Configuration(void)
{
  int    matches;
  char  *current_line = NULL;
  size_t length;
  char  *policy_str;
  int    policy = -1;

  size_t machine_it;

  if (sch_conf_file != NULL)
  {
    printf ("-> Loading scheduler configuration from file %s\n",
            sch_conf_filename);

    if ( getline(&current_line, &length, sch_conf_file) == -1)
    {
      die ("Can't read scheduler configuration file %s: %s \n",
           sch_conf_filename,
           IO_get_error());
    }

    policy_str = (char*) malloc(strlen(current_line));

    matches = sscanf (current_line, "Policy: %s\n", policy_str);
    if (matches != 1)
    {
      die("Invalid format in scheduler configuration file %s. Invalid policy name %s\n",
         sch_conf_filename,
         current_line);
    }

    IO_fclose (sch_conf_file);

    /* TODO: The same policy is applied to all machines! */
    if ( (policy = SCHEDULER_get_policy (policy_str)) != -1)
    {
      printf ("   * Scheduling policy selected %s\n", policy_str);
    }
    else
    {
      printf ("   * Invalid policy %s defined on file %s. Using default\n",
              policy_str,
              sch_conf_filename);
    }
  }
  else
  {
    printf ("-> Loading default scheduler configuration\n");

  }

  /* Load the scheduler for each machine */
  for (machine_it = 0; machine_it < Simulator.number_machines; machine_it++)
  {
    if (policy != -1)
    {
      /* Set the scheduler policy to the machine */
      Machines[machine_it].scheduler.policy = policy;
    }

    /* Initialize the scheduler using the 'SCH' function table */
    (*SCH[Machines[machine_it].scheduler.policy].scheduler_init) (sch_conf_filename, &Machines[machine_it]);
    printf ("   * Machine %zu. Policy: %s\n",
            machine_it,
            SCH[Machines[machine_it].scheduler.policy].name);
  }

  return;
}

void CONFIGURATION_Load_Random_Configuration(void)
{
  char   dist_name[BUFSIZE]; /* Abans era: 256 */
  double param1, param2;

  if (random_conf_file != NULL)
  {
    printf ("-> Loading random configuration from file %s\n",
            random_conf_filename);

    if ( fscanf (random_conf_file,
                 "Processor: %s %le %le\n",
                 dist_name,
                 &param1,
                 &param2) !=3)
    {
      die("Invalid format in random configuration file %s (processor)\n",
          random_conf_filename);
    }

    if (!RANDOM_Init_Distribution(dist_name,
                                  param1,
                                  param2,
                                  &randomness.processor_ratio))
    {
      die("Invalid random distribution '%s' in configuration file %s (processor)\n",
          dist_name,
          random_conf_filename);
    }

    if ( fscanf (random_conf_file,
                 "Network bandwidth: %s %le %le\n",
                 dist_name,
                 &param1,
                 &param2) !=3)
    {
      die ("Invalid format in random configuration file %s (network bandwidth)\n",
           random_conf_filename);
    }
    if (!RANDOM_Init_Distribution(dist_name,
                                  param1,
                                  param2,
                                  &randomness.network_bandwidth))
    {
      die("Invalid random distribution '%s' in configuration file %s (network bandwidth)\n",
          dist_name,
          random_conf_filename);
    }

    if (fscanf (random_conf_file,
                "Network latency: %s %le %le\n",
                dist_name,
                &param1,
                &param2) != 3)
    {
      die ("Invalid format in random configuration file %s (network latency)\n",
           random_conf_filename);
    }
    if (!RANDOM_Init_Distribution(dist_name,
                                  param1,
                                  param2,
                                  &randomness.network_latency))
    {
      die("Invalid random distribution '%s' in configuration file %s (network latency)\n",
          dist_name,
          random_conf_filename);
    }

    if (fscanf (random_conf_file,
                "Memory bandwidth: %s %le %le\n",
                dist_name,
                &param1,
                &param2) !=3)
    {
      die ("Invalid format in random configuration file %s (memory bandwidth)\n",
           random_conf_filename);
    }
    if (!RANDOM_Init_Distribution(dist_name,
                                 param1,
                                 param2,
                                 &randomness.memory_bandwidth))
    {
      die("Invalid random distribution '%s' in configuration file %s (memory bandwidth)\n",
          dist_name,
          random_conf_filename);
    }

    if (fscanf (random_conf_file,
                "Memory latency: %s %le %le\n",
                dist_name,
                &param1,
                &param2) !=3)
    {
      die ("Invalid format in random configuration file %s (memory latency)\n",
           random_conf_filename);
    }
    if (!RANDOM_Init_Distribution(dist_name,
                                  param1,
                                  param2,
                                  &randomness.memory_latency))
    {
      die("Invalid random distribution '%s' in configuration file %s (memory latency)\n",
          dist_name,
          random_conf_filename);
    }

    if (fscanf (random_conf_file,
                "External Network bandwidth: %s %le %le\n",
                dist_name,
                &param1,
                &param2) != 3)
    {
      die ("Invalid format in random configuration file %s (external network bandwidth)\n",
           random_conf_filename);
    }
    if (!RANDOM_Init_Distribution(dist_name,
                                  param1,
                                  param2,
                                  &randomness.external_network_bandwidth))
    {
      die ("Invalid random distribution '%s' in configuration file %s (external network bandwidth)\n",
           dist_name,
           random_conf_filename);
    }

    if (fscanf (random_conf_file,
                "External Network latency: %s %le %le\n",
                dist_name,
                &param1,
                &param2) != 3)
    {
      die ("Invalid format in random configuration file %s (external network latency)\n",
           random_conf_filename);
    }
    if (!RANDOM_Init_Distribution(dist_name,
                                  param1,
                                  param2,
                                  &randomness.external_network_latency))
    {
      die("Invalid random distribution '%s' in configuration file %s (external network latency)\n",
          dist_name,
          random_conf_filename);
    }
  }
  else
  {
    printf ("-> Loading default random values\n");
  }
}

void CONFIGURATION_Load_FS_Configuration(void)
{
  if (fs_conf_file != NULL)
  {
    printf ("-> Loading file system configuration from file %s. NOT IMPLEMENTED YET\n",
            fs_conf_filename);

    if (IO_fclose(fs_conf_file) == EOF)
    {
      die ("Error closing File System parameters file (%s): %s\n",
           fs_conf_filename,
           IO_get_error());
    }
  }
  else
  {
    printf ("-> Loading default file sytem configuration\n");
  }

  return;
}

/*****************************************************************************
 * Private functions implementation
 ****************************************************************************/

void new_wan_info  (struct t_queue *q, struct t_entry* en)
{
  struct t_field   *f;
  struct t_element *el;

  /* WAN Name */
  f  = (struct t_field *)   head_queue (q);
  el = (struct t_element *) head_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  SIMULATOR_set_wan_name(f->value.string);

  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if (f->value.dec <= 0)
  {
    near_line ();
    die ("Invalid number of machines: %d \n", f->value.dec);
    return;
  }

  SIMULATOR_set_number_of_machines(f->value.dec);

  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if (f->value.dec < 0)
  {
    near_line ();
    die ("Invalid number of dedicated connections: %d \n",
                      f->value.dec);
    return;
  }

  SIMULATOR_set_number_of_dedicated_connections(f->value.dec);

  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if ((f->value.dec < 1) || (f->value.dec > 4))
  {
    near_line ();
    die ("Invalid function of traffic: %d \n", f->value.dec);
    return;
  }

  SIMULATOR_set_wan_traffic_function(f->value.dec);

  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if (f->value.real < 0)
  {
    near_line ();
    die ("Invalid maximmum traffic value: %le \n", f->value.real);
    return;
  }

  SIMULATOR_set_wan_max_traffic_value(f->value.real);

  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if (f->value.real < 0)
  {
    near_line ();
    die ("Invalid external network bandwidth: %le \n",
                      f->value.real);
    return;
  }

  SIMULATOR_set_wan_bandwidth(f->value.real);

  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if ((f->value.dec < 1) || (f->value.dec > 3))
  {
    near_line ();
    die ("Invalid model for global communication: %d \n",
                      f->value.dec);
    return;
  }
  SIMULATOR_set_wan_global_op_model(f->value.dec);
}

void new_env_info (struct t_queue *q, struct t_entry* en)
{
  struct t_field   *f;
  struct t_element *el;

  int               machine_id, number_of_nodes, number_of_buses, global_operation_model;
  char             *machine_name, *instrumented_architecture;
  double            bandwidth;

  /*
   * Machine name
   */
  f  = (struct t_field *) head_queue (q);
  el = (struct t_element *) head_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  machine_name = strdup(f->value.string);

  /*
   * Machine id
   */
  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if (f->value.dec < 0)
  {
    near_line ();
    die ("Machine identifier cannot be negative (%d)\n",
                      f->value.dec);
    return;
  }
  machine_id = f->value.dec;

  if (!SIMULATOR_machine_exists(machine_id))
  {
    near_line();
    die("Machine id %d does not match the environment information\n",
                     machine_id);
    return;
  }

  /*
   * Instrumented architecture name
   */
  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  instrumented_architecture = strdup(f->value.string);

  /*
   * Number of nodes
   */
  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if (f->value.dec <= 0)
  {
    near_line ();
    die ("Invalid number of nodes on machine: %d \n",
                      f->value.dec);
    return;
  }
  number_of_nodes = f->value.dec;

  /*
   * Network bandwidth
   */
  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
    BAD_TYPES(f->tipo, el->type);
  if (f->value.real < 0)
  {
    near_line ();
    die ("Invalid network communication bandwidth: %le\n",
                      f->value.real);
    return;
  }
  bandwidth = f->value.real;

  /*
   * Number of network buses
   */
  f = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);
  if (f->tipo != el->type)
    BAD_TYPES(f->tipo, el->type);
  if (f->value.dec < 0)
  {
    near_line ();
    die ("Invalid number of buses on internal network: %d \n",
                      f->value.dec);
    return;
  }
  number_of_buses = f->value.dec;

  /*
   * Global operations model
   */
  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if ((f->value.dec < 1) || (f->value.dec > 3)) // HARCODE!! 3 = different global op models!
  {
    near_line ();
    die ("Invalid model for global communication: %d \n",
                      f->value.dec);
    return;
  }
  global_operation_model = f->value.dec;

  if (CONFIGURATION_parameter_bw != DBL_MIN)
  {
    bandwidth = CONFIGURATION_parameter_bw;
  }

  SIMULATOR_set_machine_definition(machine_id,
                                   machine_name,
                                   instrumented_architecture,
                                   number_of_nodes,
                                   bandwidth,
                                   number_of_buses,
                                   global_operation_model);


  free(machine_name);
  free(instrumented_architecture);

  // Initialize_empty_node (&Node_queue, machine);
}

void new_node_info (struct t_queue *q, struct t_entry* en)
{
  struct t_field   *f, *f2, *f3, *f4;
  struct t_element *el;
  struct t_machine *machine;

  int               machine_id;
  int               node_id;
  char             *node_name;
  int               no_processors;
  int               no_mem_buses;
  int               no_mem_in_links;
  int               no_mem_out_links;
  int               no_input,
                    no_output;
  double            local_startup,
                    remote_startup;
  double            relative;
  double            local_bandwidth;
  double            external_net_startup;
  double            local_port_startup,
                    remote_port_startup;
  double            local_memory_startup,
                    remote_memory_startup;


  /* (2013/04/29): v4 CFG parsing. Should be erased as soon as possible */
  if (count_queue(q) == SDDFA_2C_OLD_FIELD_COUNT)
  {
    return new_OLD_node_info(q, en);
  }

  /*
   * Machine id
   */
  f  = (struct t_field *) head_queue (q);
  el = (struct t_element *) head_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if (f->value.dec < 0)
  {
    near_line ();
    die ("Machine identifier cannot be negative (%d)\n",
                      f->value.dec);
    return;
  }
  machine_id = f->value.dec;

  if (!SIMULATOR_machine_exists(machine_id))
  {
    near_line();
    die("Machine id %d does not exist when defining new node\n",
                     machine_id);
    return;
  }

  /*
   * Node id
   */
  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }
  node_id = f->value.dec;

  if (!SIMULATOR_node_exists(node_id))
  {
    near_line();
    die("Node id %d does not belong to any of the created machines\n",
                     machine_id);
    return;
  }

  /*
   * Node name ("simulated_architecture")
   */
  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }
  node_name = strdup(f->value.string);

  /*
   * Number of processors
   */
  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if (f->value.dec <= 0)
  {
    near_line ();
    die ("Invalid number of processors (%d) for node %d\n",
                      f->value.dec,
                      node_id);
    return;
  }
  no_processors = f->value.dec;

  /*
   * CPU ratio
   */
  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if (f->value.real < 0)
  {
    near_line ();
    die ("Invalid relative processor speed (%le) for node %d\n",
                      f->value.real,
                      node_id);
    return;
  }
  relative = f->value.real;


  /*
   * Local startup
   */
  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if (f->value.real < 0)
  {
    near_line ();
    die ("Invalid startup on local communication (%le) for node %d\n",
                      f->value.real,
                      node_id);
    return;
  }
  local_startup = f->value.real * 1e9;

  /*
   * Internal network bandwidth
   */
  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if (f->value.real < 0)
  {
    near_line ();
    die ("Invalid memory bandwidth (%le) for node %d\n",
                      f->value.real,
                      node_id);
    return;
  }
  local_bandwidth = f->value.real;

  /*
   * Memory contention
  f  = (struct t_field *)   next_queue (q);
  el = (struct t_element *) next_queue (en->types);
  // el = (struct t_element *) next_queue (en->types);

  if (( f->value.arr.dim1 != 3) || (count_queue (f->value.arr.q) != 3 ))
  {
    near_line ();
    die("Incorrect number of fields (%d) in memory contention definition of node %d. Should be 3\n",
        node_id,
        f->value.arr.dim1 );
    return;
  }

  f2 = (struct t_field*) head_queue (f->value.arr.q);
  no_mem_buses = f2->value.dec;

  f2 = (struct t_field*) next_queue (f->value.arr.q);
  no_mem_in_links = f2->value.dec;

  f2 = (struct t_field*) next_queue (f->value.arr.q);
  no_mem_out_links = f2->value.dec;

  */

  /*
   * Number of node buses
   */

  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if (f->value.dec < 0)
  {
    near_line ();
    die ("Invalid number of memory buses (%d) for node %d\n",
                      f->value.dec,
                      node_id);
    return;
  }
  no_mem_buses = f->value.dec;

  /*
   * Number of intra-node INPUT links
   */
  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if (f->value.dec < 0)
  {
    near_line ();
    die ("Invalid number of intra-node INPUT links (%d) for node %d\n",
                      f->value.dec,
                      node_id);
    return;
  }
  no_mem_in_links = f->value.dec;

  /*
   * Number of intra-node OUTPUT links
   */
  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if (f->value.dec < 0)
  {
    near_line ();
    die ("Invalid number of intra-node OUT links (%d) for node %d\n",
                      f->value.dec,
                      node_id);
    return;
  }
  no_mem_out_links = f->value.dec;

  if ((no_mem_in_links == 0) && (no_mem_out_links == 0))
  {
    near_line ();
    die ("Invalid number of intra-node half duplex links (%d) for node %d\n",
         f->value.dec,
         node_id);
    return;
  }

  /*
   * Inter-node startup
   */
  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if (f->value.real < 0)
  {
    near_line ();
    die ("Invalid startup on inter-node startup (%le) for node %d\n",
                      f->value.real,
                      node_id);
    return;
  }
  remote_startup = f->value.real * 1e9;

  /*
   * Inter-node Input links
   */
  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if (f->value.dec < 0)
  {
    near_line ();
    die ("Invalid number of inter-node INPUT links (%d) for node %d\n",
          f->value.dec,
          node_id);
    return;
  }
  no_input = f->value.dec;

  /*
   * Inter-node output links
   */
  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if (f->value.dec < 0)
  {
    near_line ();
    die ("Invalid number of inter-node OUTPUT links (%d) for node %d\n",
                      f->value.dec,
                      node_id);
    return;
  }
  no_output = f->value.dec;

  if ((no_input == 0) && (no_output == 0))
  {
    near_line ();
    die ("Invalid number of network half duplex links (%d) for node %d\n",
                      f->value.dec,
                      node_id);
    return;
  }

  /*
   * Inter-machines (WAN) startup
   */
  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if (f->value.real < 0)
  {
    near_line ();
    die ("Invalid inter-machines (WAN) startup (%le) for node %d\n",
                      f->value.real,
                      node_id);
    return;
  }
  external_net_startup = f->value.real * 1e9;

  /*
   * Local port startup
   */
  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (el != NULL)
  {
    if (f->tipo != el->type)
    {
      BAD_TYPES(f->tipo, el->type);
    }

    if (f->value.real < 0)
    {
      near_line ();
      die ("Invalid local port startup (%le) for node %d\n",
                        f->value.real,
                        node_id);
      return;
    }
    local_port_startup = f->value.real * 1e9;
  }
  else
  {
    local_port_startup = 0.0;
  }

  /*
   * Remote port startup
   */
  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (el != NULL)
  {
    if (f->tipo != el->type)
    {
      BAD_TYPES(f->tipo, el->type);
    }

    if (f->value.real < 0)
    {
      near_line ();
      die ("Invalid remote port startup (%le) for node %d\n",
                        f->value.real,
                        node_id);
      return;
    }
    remote_port_startup = f->value.real * 1e9;
  }
  else
  {
    remote_port_startup = 0.0;
  }

  /*
   * Local memory startup
   */
  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (el != NULL)
  {
    if (f->tipo != el->type)
    {
      BAD_TYPES(f->tipo, el->type);
    }

    if (f->value.real < 0)
    {
      near_line ();
      die ("Invalid local memory startup (%le) for node %d\n",
                        f->value.real,
                        node_id);
      return;
    }
    local_memory_startup = f->value.real * 1e9;
  }
  else
  {
    local_memory_startup = 0.0;
  }

  /*
   * Remote memory startup
   */
  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (el != NULL)
  {
    if (f->tipo != el->type)
    {
      BAD_TYPES(f->tipo, el->type);
    }

    if (f->value.real < 0)
    {
      near_line ();
      die ("Invalid remote memory startup (%le) for node %d\n",
                        f->value.real,
                        node_id);
      return;
    }
    remote_memory_startup = f->value.real * 1e9;
  }
  else
  {
    remote_memory_startup = 0.0;
  }

  if (CONFIGURATION_parameter_lat != DBL_MIN)
  {
    remote_startup = CONFIGURATION_parameter_lat;
  }

  SIMULATOR_set_node_definition (node_id,
                                 machine_id,
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
  return;
}

/*
 * (2013/04/29) Old declaration of a node, using default values for the
 * memory contention parameters. Should be erased as soon as possible
 */
void new_OLD_node_info (struct t_queue *q, struct t_entry* en)
{
  struct t_field   *f;
  struct t_element *el;
  struct t_machine *machine;

  int               machine_id;
  int               node_id;
  char             *node_name;
  int               no_processors;
  int               no_input,
                    no_output;
  double            local_startup,
                    remote_startup;
  double            relative;
  double            local_bandwith;
  double            external_net_startup;
  double            local_port_startup,
                    remote_port_startup;
  double            local_memory_startup,
                    remote_memory_startup;

  /*
   * Machine id
   */
  f  = (struct t_field *) head_queue (q);
  el = (struct t_element *) head_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if (f->value.dec < 0)
  {
    near_line ();
    die ("Machine identifier cannot be negative (%d)\n",
                      f->value.dec);
    return;
  }
  machine_id = f->value.dec;

  if (!SIMULATOR_machine_exists(machine_id))
  {
    near_line();
    die("Machine id %d does not exist when defining new node\n",
                     machine_id);
    return;
  }

  /*
   * Node id
   */
  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }
  node_id = f->value.dec;

  if (!SIMULATOR_node_exists(node_id))
  {
    near_line();
    die("Node id %d does not belong to any of the created machines\n",
                     machine_id);
    return;
  }

  /*
   * Node name
   */
  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  node_name = strdup(f->value.string);

  /*
   * Number of processors
   */
  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if (f->value.dec <= 0)
  {
    near_line ();
    die ("Invalid number of processors (%d) for node %d\n",
                      f->value.dec,
                      node_id);
    return;
  }
  no_processors = f->value.dec;

  /*
   * Input links
   */
  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if (f->value.dec < 0)
  {
    near_line ();
    die ("Invalid number of input links (%d) for node %d\n",
                      f->value.dec,
                      node_id);
    return;
  }
  no_input = f->value.dec;

  /*
   * Output links
   */
  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if (f->value.dec < 0)
  {
    near_line ();
    die ("Invalid number of output links (%d) for node %d\n",
                      f->value.dec,
                      node_id);
    return;
  }
  no_output = f->value.dec;

  if ((no_input == 0) && (no_output == 0))
  {
    near_line ();
    die ("Invalid number of half duplex  links (%d) for node %d\n",
                      f->value.dec,
                      node_id);
    return;
  }

  /*
   * Local startup
   */
  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if (f->value.real < 0)
  {
    near_line ();
    die ("Invalid startup on local communication (%le) for node %d\n",
                      f->value.real,
                      node_id);
    return;
  }
  local_startup = f->value.real * 1e9;

  /*
   * Remote startup
   */
  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if (f->value.real < 0)
  {
    near_line ();
    die ("Invalid startup on remote communication (%le) for node %d\n",
                      f->value.real,
                      node_id);
    return;
  }
  remote_startup = f->value.real * 1e9;

  /*
   * CPU ratio
   */
  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if (f->value.real < 0)
  {
    near_line ();
    die ("Invalid relative processor speed (%le) for node %d\n",
                      f->value.real,
                      node_id);
    return;
  }
  relative = f->value.real;

  /*
   * Internal network bandwidth
   */
  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if (f->value.real < 0)
  {
    near_line ();
    die ("Invalid memory bandwidth (%le) for node %d\n",
                      f->value.real,
                      node_id);
    return;
  }
  local_bandwith = f->value.real;

  /*
   * Remote network startup
   */
  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if (f->value.real < 0)
  {
    near_line ();
    die ("Invalid external net startup (%le) for node %d\n",
                      f->value.real,
                      node_id);
    return;
  }
  external_net_startup = f->value.real * 1e9;

  /*
   * Local port startup
   */
  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (el != NULL)
  {
    if (f->tipo != el->type)
    {
      BAD_TYPES(f->tipo, el->type);
    }

    if (f->value.real < 0)
    {
      near_line ();
      die ("Invalid local port startup (%le) for node %d\n",
                        f->value.real,
                        node_id);
      return;
    }
    local_port_startup = f->value.real * 1e9;
  }
  else
  {
    local_port_startup = 0.0;
  }

  /*
   * Remote port startup
   */
  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (el != NULL)
  {
    if (f->tipo != el->type)
    {
      BAD_TYPES(f->tipo, el->type);
    }

    if (f->value.real < 0)
    {
      near_line ();
      die ("Invalid remote port startup (%le) for node %d\n",
                        f->value.real,
                        node_id);
      return;
    }
    remote_port_startup = f->value.real * 1e9;
  }
  else
  {
    remote_port_startup = 0.0;
  }

  /*
   * Local memory startup
   */
  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (el != NULL)
  {
    if (f->tipo != el->type)
    {
      BAD_TYPES(f->tipo, el->type);
    }

    if (f->value.real < 0)
    {
      near_line ();
      die ("Invalid local memory startup (%le) for node %d\n",
                        f->value.real,
                        node_id);
      return;
    }
    local_memory_startup = f->value.real * 1e9;
  }
  else
  {
    local_memory_startup = 0.0;
  }

  /*
   * Remote memory startup
   */
  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (el != NULL)
  {
    if (f->tipo != el->type)
    {
      BAD_TYPES(f->tipo, el->type);
    }

    if (f->value.real < 0)
    {
      near_line ();
      die ("Invalid remote memory startup (%le) for node %d\n",
                        f->value.real,
                        node_id);
      return;
    }
    remote_memory_startup = f->value.real * 1e9;
  }
  else
  {
    remote_memory_startup = 0.0;
  }

  if (CONFIGURATION_parameter_lat != DBL_MIN)
  {
    remote_startup = CONFIGURATION_parameter_lat;
  }

  SIMULATOR_set_node_definition (node_id,
                                 machine_id,
                                 node_name,
                                 no_processors,
                                 no_processors, // mem_buses = processors
                                 1,             // 1 mem_in_link
                                 0,             // 0 mem_out_link (half-duplex link)
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

  return;
}

void new_map_info  (struct t_queue *q, struct t_entry* en)
{
  struct t_field   *f, *f2, *f3, *f4;
  struct t_element *el;
  int               j;

  char*  trace_name;
  int    tasks_count, tasks_mapping_length, *tasks_mapping;

  if (count_queue (q) != count_queue (en->types))
  {
    near_line ();
    die ("Insufficient fields on register #3: %s\n", en->name);
    return;
  }

  /*
   * Trace file
   */
  f  = (struct t_field *)   head_queue (q);
  el = (struct t_element *) head_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES (f->tipo, el->type);
  }

  trace_name = strdup(f->value.string);

  /*
   * Number of tasks
   */
  f  = (struct t_field *)   next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if (f->value.dec <= 0)
  {
    near_line ();
    die ("Invalid number of tasks (%d) in application %s\n",
                      f->value.dec,
                      trace_name);
    return;
  }
  tasks_count = f->value.dec;

  /*
   * Tasks mapping
   */
  f  = (struct t_field *)   next_queue (q);
  el = (struct t_element *) next_queue (en->types);
  el = (struct t_element *) next_queue (en->types);

  if (el != (struct t_element *)0)
  {
    f3 = (struct t_field *) next_queue (q);
    f4 = (struct t_field *) head_queue (f3->value.arr.q);
  }

  tasks_mapping_length = count_queue (f->value.arr.q);

  tasks_mapping = (int*) malloc(tasks_mapping_length*sizeof(int));

  for (j = 0, f2 = (struct t_field *) head_queue (f->value.arr.q);
       j < tasks_mapping_length;
       j++,   f2 = (struct t_field *) next_queue (f->value.arr.q))
  {
    int current_node_id = f2->value.dec;
    if (!SIMULATOR_node_exists(current_node_id))
    {
      near_line ();
      die("Node %d does not exist",
          current_node_id);
    }
    tasks_mapping[j] = current_node_id;
  }

  CONFIGURATION_load_mapping(trace_name, tasks_mapping, tasks_mapping_length);

  free(tasks_mapping);


  // inFIFO_queue (&Ptask_queue, (char *) Ptask);
}

void new_conf_files(struct t_queue *q, struct t_entry* en)
{
  struct t_field   *f;
  struct t_element *el;

  f  = (struct t_field *) head_queue (q);
  el = (struct t_element *) head_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if (strlen(f->value.string) != 0)
  {
    if (sch_conf_filename == NULL)
    {
      sch_conf_filename = strdup(f->value.string);

      if ( (sch_conf_file = IO_fopen(sch_conf_filename, "r")) == NULL)
      {
        die("Unable to set and open scheduling configuration file (%s): %s\n",
            sch_conf_filename,
            IO_get_error());
      }
    }
    else
    {
      warning("Using the '-s' scheduling parametrization (%s) instead of the indicated in the configuration file\n",
              sch_conf_filename);
    }
  }

  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) head_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if (strlen(f->value.string) != 0)
  {
    if (fs_conf_filename == NULL)
    {
      fs_conf_filename = strdup(f->value.string);

      if ( (fs_conf_file = IO_fopen(fs_conf_filename, "r")) == NULL)
      {
        die("Unable to set and open file system configuration file (%s): %s\n",
            fs_conf_filename,
            IO_get_error());
      }
    }
    else
    {
      warning ("Using the '-f' file system parametrization (%s) instead of the indicated in the configuration file\n",
               fs_conf_filename);
    }
  }

  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) head_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if (strlen(f->value.string) != 0)
  {
    comm_conf_filename = strdup(f->value.string);

    if ( (comm_conf_file = IO_fopen(comm_conf_filename, "r")) == NULL)
    {
      die("Unable to set and open communications configuration file (%s): %s\n",
          comm_conf_filename,
          IO_get_error());
    }
  }
  else
  {
    comm_conf_filename = NULL;
  }

  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) head_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if (strlen(f->value.string) != 0)
  {
    random_conf_filename = strdup(f->value.string);

    if ( (random_conf_file = IO_fopen(random_conf_filename, "r")) == NULL)
    {
      die("Unable to set and open random numbers configuration file (%s): %s\n",
          random_conf_filename,
          IO_get_error());
    }
  }
  else
  {
    random_conf_filename = NULL;
  }

  return;
}

void new_mod_info  (struct t_queue *q, struct t_entry* en)
{
  struct t_field   *f;
  struct t_element *el;

  unsigned long int module_type, module_value;
  double            const_burst_duration, ratio;

  f  = (struct t_field *) head_queue (q);
  el = (struct t_element *) head_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if (f->value.dec < 0)
  {
    near_line ();
    die ("Invalid module type: %d \n", f->value.dec);
    return;
  }
  module_type = (unsigned long int) f->value.dec;

  f  = (struct t_field *)   next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if (f->value.dec < 0)
  {
    near_line ();
    die ("Invalid module value: %d \n", f->value.dec);
    return;
  }
  module_value = (unsigned long int) f->value.dec;

  f  = (struct t_field *)   next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  /* JGG (2012/10/19) New behavior: negative values indicate that bursts in
     the module will be susbtitued by the absolute value of the number
     indicated */
  if (f->value.real < 0)
  {
    const_burst_duration = fabs(f->value.real);
    TASK_module_new_duration (module_type,
                              module_value,
                              const_burst_duration);

    /* DEBUG
    printf("New Const Burst Duration Module");
    printf("(%d:%d): %le\n", module_type, module_value, const_burst_duration);
    */
    /*
    near_line ();
    die ("Invalid ratio for module (%d:%d): %le\n",
                      module_type,
                      module_value,
                      f->value.real);
    return;
    */
  }
  else
  {
    ratio = f->value.real;
    TASK_module_new_ratio (module_type, module_value, ratio);
  }

  return;
}

void new_fs_params (struct t_queue *q, struct t_entry* en)
{
  struct t_field   *f;
  struct t_element *el;

  double disk_latency, disk_bandwidth, block_size, hit_ratio;
  int    concurrent_requests;

  f  = (struct t_field *) head_queue (q);
  el = (struct t_element *) head_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if (f->value.real < 0)
  {
    near_line ();
    die ("Invalid disk latency number: %le \n", f->value.real);
    return;
  }
  disk_latency = f->value.real;

  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if (f->value.real < 0)
  {
    near_line ();
    die ("Invalid disk bandwidth: %le\n", f->value.real);
    return;
  }
  disk_bandwidth = f->value.real;

  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if (f->value.real <= 0)
  {
    near_line ();
    die ("Invalid block_size: %le\n", f->value.real);
    return;
  }
  block_size = f->value.real;

  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if (f->value.dec < 0)
  {
    near_line ();
    die ("Invalid value for concurrent requests: %le\n",
                      f->value.dec);
    return;
  }
  concurrent_requests = f->value.dec;

  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if ((f->value.real < 0.0) || (f->value.real > 1.0 ))
  {
    near_line ();
    die ("Invalid hit ratio: %le\n",f->value.real);
    return;
  }
  hit_ratio = f->value.real;

  FS_Parameters (disk_latency,
                 disk_bandwidth,
                 block_size,
                 concurrent_requests,
                 hit_ratio);

  return;
}

void new_d_conn    (struct t_queue *q, struct t_entry* en)
{
  struct t_field   *f, *f2;
  struct t_element *el;

  int    i, d_conn_id, s_machine_id, d_machine_id;
  double bandwidth, startup, flight_time;
  int    tags_size, *tags;
  int    first_message_size;
  char   first_size_cond;
  int    second_message_size;
  char   second_size_cond;
  char   operation;
  int    comms_size, *comm_ids;

  t_boolean error = FALSE;

  /*
   * Connection ID
   */
  f  = (struct t_field *) head_queue (q);
  el = (struct t_element *) head_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if (f->value.dec < 0)
  {
    near_line ();
    die ("Invalid connection id: %d \n",
                      f->value.dec);
    return;
  }
  d_conn_id = f->value.dec;

  if (!SIMULATOR_dedicated_connection_exists(d_conn_id))
  {
    near_line();
    die("Dedicated connection %d not previously defined",
                     d_conn_id);
  }

  /*
   * Source machine ID
   */
  f   = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if (f->value.dec < 0)
  {
    near_line ();
    die ("Invalid number of source machine identifier: %d for connection %d\n",
                      f->value.dec,
                      d_conn_id);
    return;
  }
  s_machine_id = f->value.dec;

  if (!SIMULATOR_machine_exists(s_machine_id))
  {
    near_line();
    die("Source machine %d not found for connection %d!\n",
                     s_machine_id,
                     d_conn_id);
    return;
  }

  /*
   * Destination machine ID
   */
  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if (f->value.dec < 0)
  {
    near_line ();
    die ("Invalid number of destination machine identifier: %d for connection %d\n",
                      f->value.dec,
                      d_conn_id);
    return;
  }
  d_machine_id = f->value.dec;

  if (!SIMULATOR_machine_exists(d_machine_id))
  {
    near_line();
    die("Destination machine %d not found for connection %d!\n",
                     d_machine_id,
                     d_conn_id);
    return;
  }

  /*
   * Connection bandwidth
   */
  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if (f->value.real < 0)
  {
    near_line ();
    die ("Invalid dedicated connection bandwidth (%le) for connection %d\n",
                      f->value.real,
                      d_conn_id);
    return;
  }
  bandwidth = f->value.real;

  /*
   * Connection Tags
   */
  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (count_queue (f->value.arr.q) != f->value.arr.dim1)
  {
    near_line ();
    die ("Incorrect number of tags for connection %d\n",
                      d_conn_id);
    return;
  }
  tags_size = f->value.arr.dim1;


  tags = (int *) malloc(tags_size * sizeof(int));

  for (i = 0, f2 = (struct t_field *) head_queue (f->value.arr.q);
       i < tags_size;
       i++, f2 = (struct t_field *) next_queue (f->value.arr.q))
  {
    tags[i] = f2->value.dec;
  }

  /*
   * First message size
   */
  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if (f->value.dec < 0)
  {
    near_line ();
    die ("Invalid first message size: %d for connection %d\n",
                      f->value.dec,
                      d_conn_id);
    return;
  }
  first_message_size = f->value.dec;

  /*
   * First message condition
   */
  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if (strlen(f->value.string) != 1)
  {
    error = TRUE;
  }
  else if (f->value.string[0] == '<' || f->value.string[0]=='=' || f->value.string[0]=='>')
  {
    first_size_cond = f->value.string[0];
  }
  else
  {
    error = TRUE;
  }

  if (error)
  {
    near_line ();
    die ("Invalid dedicated connection first condition: \"%s\" for connection %d\n",
                      f->value.string,
                      d_conn_id);
    return;
  }

  /*
   * Connection operation
   */
  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if (strlen(f->value.string) != 1)
  {
    error = TRUE;
  }
  else if (f->value.string[0]=='&' || f->value.string[0]=='|')
  {
    operation = f->value.string[0];
  }
  else
  {
    error = TRUE;
  }

  if (error)
  {
    near_line ();
    die ("Invalid dedicated connection operation: \"%s\" for connection %d\n",
                      f->value.string,
                      d_conn_id);
    return;
  }

  /*
   * Second message size
   */
  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if (f->value.dec < 0)
  {
    near_line ();
    die ("Invalid second message size: %d for connection %d\n",
                      f->value.dec,
                      d_conn_id);
    return;
  }
  second_message_size = f->value.dec;

  /*
   * Second message size condition
   */
  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if (strlen(f->value.string) != 1)
  {
    error = TRUE;
  }
  else if (f->value.string[0] == '<' || f->value.string[0]=='=' || f->value.string[0]=='>')
  {
    second_size_cond = f->value.string[0];
  }
  else
  {
    error = TRUE;
  }

  if (error)
  {
    near_line ();
    die ("Invalid dedicated connection second condition: \"%s\" for connection %d\n",
                      f->value.string,
                      d_conn_id);
    return;
  }

  /*
   * Connection communicators
   */
  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (count_queue (f->value.arr.q) != f->value.arr.dim1)
  {
    near_line ();
    die ("Incorrect dedicated connection number of communicators for connection %d\n",
                      d_conn_id);
    return;
  }

  comms_size = f->value.arr.dim1;
  comm_ids   = (int*) malloc (comms_size*sizeof(int));

  for (i = 0, f2 = (struct t_field *) head_queue (f->value.arr.q);
       i  < comms_size;
       i++, f2 = (struct t_field *) next_queue (f->value.arr.q))
  {
    comm_ids[i] = f2->value.dec;
  }

  /*
   * Connection startup
   */
  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if (f->value.real < 0)
  {
    near_line ();
    die ("Invalid dedicated connection startup (%le) for connection %d\n",
                      f->value.real,
                      d_conn_id);
    return;
  }
  startup = f->value.real * 1e9;

  /*
   * Flight time
   */
  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
  {
    BAD_TYPES(f->tipo, el->type);
  }

  if (f->value.real < 0)
  {
    near_line ();
    die ("Invalid dedicated connection _time (%le) for connection %d\n",
                      f->value.real,
                      d_conn_id);
    return;
  }
  flight_time = f->value.real * 1e9;

  SIMULATOR_set_dedicated_connection_definition(d_conn_id,
                                                s_machine_id,
                                                d_machine_id,
                                                bandwidth,
                                                tags_size,
                                                tags,
                                                first_message_size,
                                                first_size_cond,
                                                operation,
                                                second_message_size,
                                                second_size_cond,
                                                comms_size,
                                                comm_ids,
                                                startup,
                                                flight_time);
}

void CONFIGURATION_load_mapping(char* tracefile,
                                int*  tasks_mapping,
                                int   tasks_mapping_length)
{
  char* effective_tracefile;
  int   effective_tasks_count;

  if (CONFIGURATION_mappings_read == 0)
  {
    /* Check if there is a tracefile available */
    if (CONFIGURATION_parameter_tracefile != (char*) 0)
    {
      effective_tracefile = CONFIGURATION_parameter_tracefile;
    }
    else if (tracefile == (char*) 0)
    {
      near_line ();
      die("No tracefile provided, please use the '--dim' parameter to supply an application trace to simulate");
    }
    else
    {
      effective_tracefile = tracefile;
    }

    /* Check the configuration precedence. In the first mapping, command line
     * mapping parameters override the ones present in the configuration file */
    if (CONFIGURATION_parameter_predefined_map != MAP_NO_PREDEFINED)
    {
      TASK_New_Ptask_predefined_map(effective_tracefile,
                                    CONFIGURATION_parameter_predefined_map,
                                    CONFIGURATION_parameter_tasks_per_node);
    }
    else if (tasks_mapping_length !=  0)
    {
      CONFIGURATION_load_tasks_mapping_array(effective_tracefile,
                                             tasks_mapping,
                                             tasks_mapping_length);
    }
    else
    {
      near_line ();
      die("Inconsistent task mapping");
    }
  }
  else
  {
    if (tracefile == (char*) 0)
    {
      near_line();
      die("no trace provided for mapping");
    }

    CONFIGURATION_load_tasks_mapping_array(tracefile,
                                           tasks_mapping,
                                           tasks_mapping_length);
  }

  CONFIGURATION_mappings_read++;

  return;
}

t_boolean CONFIGURATION_load_tasks_mapping_array(char* tracefile,
                                                 int*  tasks_mapping,
                                                 int   tasks_mapping_length)
{
  int tasks_count;

  if (DATA_ACCESS_get_number_of_tasks(tracefile, &tasks_count) == FALSE)
  {
    die("Unable to retrieve number of tasks of trace '%s': %s",
        tracefile,
        DATA_ACCESS_get_error());
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
    near_line();
    die("number of tasks in trace '%s' (%d) is larger than the tasks mapping (%d)",
        tracefile,
        tasks_count,
        tasks_mapping_length);
  }

  return TRUE;
}

