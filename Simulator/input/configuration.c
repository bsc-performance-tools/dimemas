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

#include "configuration.h"

#include "dimemas_io.h"
#include "file_data_access.h" /* risky, to read the number of tasks from the parameter trace */

#include <comm_configuration.h>
#include <errno.h>
#include <float.h>
#include <fs.h>
#include <list.h>
#include <math.h>
#include <schedule.h>
#include <stdlib.h>
#include <string.h>
#include <subr.h>
                                 

/* That shouldn't be used here... */
#include "machine.h"
#include "random.h"
#include "sched_vars.h"
#include "simulator.h"
#include "task.h"


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

static char *sch_conf_filename = NULL;
static FILE *sch_conf_file     = NULL;

static char *fs_conf_filename = NULL;
static FILE *fs_conf_file     = NULL;

static char *comm_conf_filename = NULL;
static FILE *comm_conf_file     = NULL;

static char *random_conf_filename = NULL;
static FILE *random_conf_file     = NULL;

/*
 * Configuration elemets that can be overridden per parameter
 */
static char *CONFIGURATION_parameter_tracefile    = (char *)0;
static double CONFIGURATION_parameter_bw          = DBL_MIN;
static double CONFIGURATION_parameter_lat         = DBL_MIN;
static int CONFIGURATION_parameter_predefined_map = MAP_NO_PREDEFINED;
static int CONFIGURATION_parameter_tasks_per_node;

static int CONFIGURATION_mappings_read = 0;


/*****************************************************************************
 * Public functions implementation
 ****************************************************************************/

// t_boolean CONFIGURATION_parse( FILE *configuration_file,
//                                char *parameter_tracefile,
//                                double parameter_bw,
//                                double parameter_lat,
//                                int parameter_predefined_map,
//                                int parameter_tasks_per_node )
// {
//   create_queue( &Registers_queue );

//   if ( parameter_tracefile != (char *)0 )
//   {
//     CONFIGURATION_parameter_tracefile = strdup( parameter_tracefile );
//   }
//   CONFIGURATION_parameter_bw             = parameter_bw;
//   CONFIGURATION_parameter_lat            = parameter_lat;
//   CONFIGURATION_parameter_predefined_map = parameter_predefined_map;
//   CONFIGURATION_parameter_tasks_per_node = parameter_tasks_per_node;

//   yyin = configuration_file;

//   if ( yyparse() )
//   {
//     return FALSE;
//   }

//   SIMULATOR_check_correct_definitions();

//   return TRUE;
// }

void CONFIGURATION_Set_Scheduling_Configuration_File( const char *sch_filename )
{
  if ( sch_conf_filename != NULL )
  {
    warning( "Using the '-s' scheduling parametrization (%s) instead of the indicated in the configuration file\n", sch_conf_filename );
  }
  else
    sch_conf_filename = strdup( sch_filename );

  if ( ( sch_conf_file = IO_fopen( sch_conf_filename, "r" ) ) == NULL )
  {
    die( "Unable to set and open scheduling configuration file (%s): %s\n", sch_filename, IO_get_error() );
  }
  return;
}

void CONFIGURATION_Set_FileSystem_Configuration_File( const char *fs_filename )
{
  if ( fs_conf_filename != NULL )
  {
    warning( "Using the '-f' file system parametrization (%s) instead of the indicated in the configuration file\n", fs_conf_filename );
  }
  else
    fs_conf_filename = strdup( fs_filename );

  if ( ( fs_conf_file = IO_fopen( fs_conf_filename, "r" ) ) == NULL )
  {
    die( "Unable to set and open file system configuration file (%s): %s\n", fs_filename, IO_get_error() );
  }
  return;
}

void CONFIGURATION_Set_Communications_Configuration_File( const char *comm_filename )
{
  if ( comm_conf_filename != NULL )
  {
    warning( "Using the '-c' communication parametrization (%s) instead of the indicated in the configuration file\n", comm_conf_filename );
  }
  else
    comm_conf_filename = strdup( comm_filename );

  if ( ( comm_conf_file = IO_fopen( comm_conf_filename, "r" ) ) == NULL )
  {
    die( "Unable to set and open communications configuration file (%s): %s\n", comm_filename, IO_get_error() );
  }
  return;
}


void CONFIGURATION_Set_RandomValues_Configuration_File( const char *rand_filename )
{
  if ( random_conf_filename != NULL )
  {
    warning( "Using the '-r' random parametrization (%s) instead of the indicated in the configuration file\n", random_conf_filename );
  }
  else
    random_conf_filename = strdup( rand_filename );

  if ( ( random_conf_file = IO_fopen( random_conf_filename, "r" ) ) == NULL )
  {
    die( "Unable to set and open random values configuration file (%s): %s\n", rand_filename, IO_get_error() );
  }
  return;
}

void CONFIGURATION_Load_Communications_Configuration( void )
{
  /*
   * Load the communications definition file indicated in the configuration
   * file.
   *
   * NOTE: the file pointer is closed inside!
   */
  if ( comm_conf_filename != NULL )
  {
    COMM_CONFIGURATION_Load_General_Comms_Definition( comm_conf_filename, comm_conf_file );
  }
  else
  {
    info( "   * Loading default communications configuration\n" );
  }

  /*
   * Load the special file 'params_traffic.cfg' that defines the external
   * network traffic parameters
   */
  COMM_CONFIGURATION_Load_External_Network_Parameters();

  return;
}

void CONFIGURATION_Load_Scheduler_Configuration( void )
{
  int matches;
  char *current_line = NULL;
  size_t length;
  char *policy_str;
  int policy = -1;

  size_t machine_it;

  if ( sch_conf_file != NULL )
  {
    info( "-> Loading scheduler configuration from file %s\n", sch_conf_filename );

    if ( getline( &current_line, &length, sch_conf_file ) == -1 )
    {
      die( "Can't read scheduler configuration file %s: %s \n", sch_conf_filename, IO_get_error() );
    }

    policy_str = (char *)malloc( strlen( current_line ) );

    matches = sscanf( current_line, "Policy: %s\n", policy_str );
    if ( matches != 1 )
    {
      die( "Invalid format in scheduler configuration file %s. Invalid policy name %s\n", sch_conf_filename, current_line );
    }

    IO_fclose( sch_conf_file );

    /* TODO: The same policy is applied to all machines! */
    if ( ( policy = SCHEDULER_get_policy( policy_str ) ) != -1 )
    {
      info( "   * Scheduling policy selected %s\n", policy_str );
    }
    else
    {
      info( "   * Invalid policy %s defined on file %s. Using default\n", policy_str, sch_conf_filename );
    }
  }
  else
  {
    info( "-> Loading default scheduler configuration\n" );
  }

  /* Load the scheduler for each machine */
  for ( machine_it = 0; machine_it < Simulator.number_machines; machine_it++ )
  {
    if ( policy != -1 )
    {
      /* Set the scheduler policy to the machine */
      Machines[ machine_it ].scheduler.policy = policy;
    }

    /* Initialize the scheduler using the 'SCH' function table */
    ( *SCH[ Machines[ machine_it ].scheduler.policy ].scheduler_init )( sch_conf_filename, &Machines[ machine_it ] );
    info( "   * Machine %zu. Policy: %s\n", machine_it, SCH[ Machines[ machine_it ].scheduler.policy ].name );
  }

  return;
}

void CONFIGURATION_Load_Random_Configuration( void )
{
  char dist_name[ BUFSIZE ]; /* Abans era: 256 */
  double param1, param2;

  if ( random_conf_file != NULL )
  {
    info( "-> Loading random configuration from file %s\n", random_conf_filename );

    if ( fscanf( random_conf_file, "Processor: %s %le %le\n", dist_name, &param1, &param2 ) != 3 )
    {
      die( "Invalid format in random configuration file %s (processor)\n", random_conf_filename );
    }

    if ( !RANDOM_Init_Distribution( dist_name, param1, param2, &randomness.processor_ratio ) )
    {
      die( "Invalid random distribution '%s' in configuration file %s (processor)\n", dist_name, random_conf_filename );
    }

    if ( fscanf( random_conf_file, "Network bandwidth: %s %le %le\n", dist_name, &param1, &param2 ) != 3 )
    {
      die( "Invalid format in random configuration file %s (network bandwidth)\n", random_conf_filename );
    }
    if ( !RANDOM_Init_Distribution( dist_name, param1, param2, &randomness.network_bandwidth ) )
    {
      die( "Invalid random distribution '%s' in configuration file %s (network bandwidth)\n", dist_name, random_conf_filename );
    }

    if ( fscanf( random_conf_file, "Network latency: %s %le %le\n", dist_name, &param1, &param2 ) != 3 )
    {
      die( "Invalid format in random configuration file %s (network latency)\n", random_conf_filename );
    }
    if ( !RANDOM_Init_Distribution( dist_name, param1, param2, &randomness.network_latency ) )
    {
      die( "Invalid random distribution '%s' in configuration file %s (network latency)\n", dist_name, random_conf_filename );
    }

    if ( fscanf( random_conf_file, "Memory bandwidth: %s %le %le\n", dist_name, &param1, &param2 ) != 3 )
    {
      die( "Invalid format in random configuration file %s (memory bandwidth)\n", random_conf_filename );
    }
    if ( !RANDOM_Init_Distribution( dist_name, param1, param2, &randomness.memory_bandwidth ) )
    {
      die( "Invalid random distribution '%s' in configuration file %s (memory bandwidth)\n", dist_name, random_conf_filename );
    }

    if ( fscanf( random_conf_file, "Memory latency: %s %le %le\n", dist_name, &param1, &param2 ) != 3 )
    {
      die( "Invalid format in random configuration file %s (memory latency)\n", random_conf_filename );
    }
    if ( !RANDOM_Init_Distribution( dist_name, param1, param2, &randomness.memory_latency ) )
    {
      die( "Invalid random distribution '%s' in configuration file %s (memory latency)\n", dist_name, random_conf_filename );
    }

    if ( fscanf( random_conf_file, "External Network bandwidth: %s %le %le\n", dist_name, &param1, &param2 ) != 3 )
    {
      die( "Invalid format in random configuration file %s (external network bandwidth)\n", random_conf_filename );
    }
    if ( !RANDOM_Init_Distribution( dist_name, param1, param2, &randomness.external_network_bandwidth ) )
    {
      die( "Invalid random distribution '%s' in configuration file %s (external network bandwidth)\n", dist_name, random_conf_filename );
    }

    if ( fscanf( random_conf_file, "External Network latency: %s %le %le\n", dist_name, &param1, &param2 ) != 3 )
    {
      die( "Invalid format in random configuration file %s (external network latency)\n", random_conf_filename );
    }
    if ( !RANDOM_Init_Distribution( dist_name, param1, param2, &randomness.external_network_latency ) )
    {
      die( "Invalid random distribution '%s' in configuration file %s (external network latency)\n", dist_name, random_conf_filename );
    }
  }
  else
  {
    info( "-> Loading default random values\n" );
  }
}

void CONFIGURATION_Load_FS_Configuration( void )
{
  if ( fs_conf_file != NULL )
  {
    info( "-> Loading file system configuration from file %s. NOT IMPLEMENTED YET\n", fs_conf_filename );

    if ( IO_fclose( fs_conf_file ) == EOF )
    {
      die( "Error closing File System parameters file (%s): %s\n", fs_conf_filename, IO_get_error() );
    }
  }
  else
  {
    info( "-> Loading default file sytem configuration\n" );
  }

  return;
}
