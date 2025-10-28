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

#include <comm_configuration.h>
#include <communic.h> // To configure the communications
#include <dimemas_io.h>
#include <errno.h>
#include "EventEncoding.h"
#include <list.h>
#include <sched_vars.h> // To access to COMMUNIC table
#include <simulator.h>
#include <string.h>
#include <subr.h>
#include <task.h> // To configure the preemptions
#include <types.h>

/*
 * External definitions
 */
extern int debug;

/*
 *Preemption overhead variables
 */
extern t_boolean PREEMP_initialized;
extern t_boolean PREEMP_enabled;
extern int PREEMP_cycle;
extern t_nano PREEMP_time;
extern int *PREEMP_table;

/*
 * Private variables
 */
// Possible buffer overflow :D
char comm_conf_error_message[ 200 ];
t_boolean expected_quantum_definition;


/*
 * Private functions
 */

t_boolean load_machine_comm_policy( int machine_id, t_boolean all_machines, char *policy_str );

t_boolean load_scheduling_quantum( int machine_id, t_boolean all_machines, int quantum );

t_boolean load_machine_flight_times( int machine_id, char *flight_times_str );

t_boolean load_global_op_parameters( t_boolean external_network,
                                     int machine_id,
                                     int global_op_id,
                                     char *FIN_model,
                                     char *FIN_size,
                                     char *FOUT_model,
                                     char *FOUT_size );

int get_global_op_model( char *model );

int get_global_op_size( char *size );

void load_external_network_parameters( void );


/******************************************************************************
 * Main function
 *****************************************************************************/

void COMM_CONFIGURATION_Load_General_Comms_Definition( char *comm_conf_filename, FILE *comm_conf_file )
{
  ssize_t read;
  char *current_line = NULL;
  size_t lenght = 0, line = 0;

  int matches, machine_id, quantum, global_OP;

  t_boolean apply_to_all_machines;


  char mini_buffer[ BUFSIZE ];
  char FIN_model[ 256 ];
  char FIN_size[ 256 ];
  char FOUT_model[ 256 ];
  char FOUT_size[ 256 ];
  double contention; /* Per motius historics. Actualment no s'utilitza. */

  info( "   * Loading communications configuration from file %s\n", comm_conf_filename );

  expected_quantum_definition = FALSE;
  while ( ( read = getline( &current_line, &lenght, comm_conf_file ) ) != -1 )
  {
    line++;

    /*
     * Scheduling algorithm
     */
    matches = sscanf( current_line, "Machine policy: %d %s", &machine_id, mini_buffer );
    if ( matches == 2 )
    {
      /* Assign scheduling only to the selected machine */
      apply_to_all_machines = FALSE;

      if ( load_machine_comm_policy( machine_id, apply_to_all_machines, mini_buffer ) == FALSE )
      {
        die( "Error loading machine policy on (%s:%d): %s", comm_conf_filename, line, comm_conf_error_message );
      }

      continue;
    }

    matches = sscanf( current_line, "Policy: %s", mini_buffer );
    if ( matches == 1 && apply_to_all_machines == TRUE )
    {
      /* Assign scheduling to ALL machines */
      apply_to_all_machines = TRUE;

      if ( load_machine_comm_policy( machine_id, apply_to_all_machines, mini_buffer ) == FALSE )
      {
        die( "Error loading machine policy (%s:%d): %s", comm_conf_filename, line, comm_conf_error_message );
      }

      continue;
    }

    /*
     * Scheduling quantum
     */
    matches = sscanf( current_line, "Quantum size (bytes): %d", &quantum );
    if ( matches != 1 && expected_quantum_definition == TRUE )
    {
      die( "Error loading communications configuration (%s:%d): expected quantum size", comm_conf_filename, line );
    }
    else if ( matches == 1 && expected_quantum_definition == TRUE )
    {
      if ( !load_scheduling_quantum( machine_id, apply_to_all_machines, quantum ) )
      {
        die( "Error loading scheduling quantum (%s:%d): %s", comm_conf_filename, line, comm_conf_error_message );
      }

      expected_quantum_definition = FALSE;

      continue;
    }


    /*
     * Machines flight times
     */
    matches = sscanf( current_line, "Flight time: %d %[^\n]", &machine_id, mini_buffer );
    if ( matches == 2 )
    {
      if ( ( machine_id < 0 ) || ( machine_id >= Simulator.number_machines ) )
      {
        die( "Invalid machine id %d (%s:%d)", machine_id, comm_conf_filename, line );
      }

      // machine_id++; /* Tenim els identificadors guardats a de 1 a N */
      /* Cal llegir els flight time d'aquesta maquina */
      if ( !load_machine_flight_times( machine_id, mini_buffer ) )
      {
        die( "Error loading machine flight times (%s:%d): %s", comm_conf_filename, line, comm_conf_error_message );
      }

      continue;
    }

    /*
     * Single machine global operation definition
     */
    matches = sscanf( current_line, "Machine globalop: %d %d %s %s %s %s", &machine_id, &global_OP, FIN_model, FIN_size, FOUT_model, FOUT_size );
    if ( matches == 6 )
    {
      if( global_OP == GLOP_ID_MPI_Reduce )
        continue;

      if ( ( machine_id < 0 ) || ( machine_id >= Simulator.number_machines ) )
      {
        die( "Invalid machine id %d (%s:%d)", machine_id, comm_conf_filename, line );
      }

      // machine_id++; /* Tenim els identificadors guardats a de 1 a N */
      /* Cal llegir els parametres de l'operació col.lectiva
       * global_OP de la maquina machine_id. */
      if ( !load_global_op_parameters( FALSE, // NO external network
                                       machine_id,
                                       global_OP,
                                       FIN_model,
                                       FIN_size,
                                       FOUT_model,
                                       FOUT_size ) )
      {
        die( "Error loading machine %d global operation definition (%s:%d): %s", machine_id, comm_conf_filename, line, comm_conf_error_message );
      }

      continue;
    }

    /*
     * External network global operation definition
     */
    matches = sscanf( current_line, "External globalop: %d %s %s %s %s", &global_OP, FIN_model, FIN_size, FOUT_model, FOUT_size );
    if ( matches == 5 )
    {
      if( global_OP == GLOP_ID_MPI_Reduce )
        continue;

      /* Cal llegir els parametres de l'operació col.lectiva
       * global_OP per la xarxa externa. */
      if ( !load_global_op_parameters( TRUE, // External network
                                       machine_id,
                                       global_OP,
                                       FIN_model,
                                       FIN_size,
                                       FOUT_model,
                                       FOUT_size ) )
      {
        die( "Error loading external network global operation definition (%s:%d): %s", comm_conf_filename, line, comm_conf_error_message );
      }

      continue;
    }

    /*
     * General global operation definition
     */
    matches = sscanf( current_line, "%d %s %s %s %s %le", &global_OP, FIN_model, FIN_size, FOUT_model, FOUT_size, &contention );
    if ( matches == 6 )
    {
      /* External network */
      if ( !load_global_op_parameters( TRUE, machine_id, global_OP, FIN_model, FIN_size, FOUT_model, FOUT_size ) )
      {
        die( "Error loading external network global operation definition (%s:%d): %s", comm_conf_filename, line, comm_conf_error_message );
      }

      for ( machine_id = 0; machine_id <= Simulator.number_machines; machine_id++ )
      {
        /* Rest of machines */
        if ( !load_global_op_parameters( FALSE, machine_id, global_OP, FIN_model, FIN_size, FOUT_model, FOUT_size ) )
        {
          die( "Error loading machine %d global operation definition (%s:%d): %s", machine_id, comm_conf_filename, line, comm_conf_error_message );
        }
      }
      /* El camp contention actualment s'ignora. */

      continue;
    }
  }
}

void COMM_CONFIGURATION_Load_External_Network_Parameters( void )
{
  FILE *fd;
  char buff[ BUFSIZE ];
  int i;
  double aux;
  char *sub_buff;

  /* S'obre el fitxer dels parametres */
  if ( !IO_file_exists( "traffic_parameters.cfg" ) )
  {
    return;
  }

  if ( ( fd = (FILE *)IO_fopen( "traffic_parameters.cfg", "r" ) ) == NULL )
  {
    die( "Unable to open 'traffic_parameters.cfg' file: %s\n", IO_get_error() );
  }

  info( "-> Loading external network traffic parameters file (parametres_traffic.cfg)\n" );


  /* Es llegeixen caracters suficients per tots els parametres */
  i         = fread( buff, 1, 255, fd );
  buff[ i ] = '\0';

  /* S'identifiquen els parametres existents */

  /* alfa */
  sub_buff = strstr( buff, "alfa=" );
  if ( sub_buff != 0 )
  {
    i = sscanf( sub_buff, "alfa=%lf", &aux );
    if ( i == 1 )
    {
      param_external_net_alfa = aux;
    }
  }
  /* periode */
  sub_buff = strstr( buff, "periode=" );
  if ( sub_buff != 0 )
  {
    i = sscanf( sub_buff, "periode=%lf", &aux );
    if ( i == 1 )
    {
      param_external_net_periode = aux;
    }
  }
  /* beta */
  sub_buff = strstr( buff, "beta=" );
  if ( sub_buff != 0 )
  {
    i = sscanf( sub_buff, "beta=%lf", &aux );
    if ( i == 1 )
    {
      param_external_net_beta = aux;
    }
  }
  /* gamma */
  sub_buff = strstr( buff, "gamma=" );
  if ( sub_buff != 0 )
  {
    i = sscanf( sub_buff, "gamma=%lf", &aux );
    if ( i == 1 )
    {
      param_external_net_gamma = aux;
    }
  }

  /* Es tanca el fitxer */
  IO_fclose( fd );

  /* Es mosta com han quedat els parametres */
  info( "   * External network traffic parameters: alfa=%.2f, periode=%.2f, beta=%.2f, gamma=%.2f\n",
        param_external_net_alfa,
        param_external_net_periode,
        param_external_net_beta,
        param_external_net_gamma );
}

/******************************************************************************
 * Private functions
 *****************************************************************************/

t_boolean load_machine_comm_policy( int machine_id, t_boolean all_machines, char *policy_str )
{
  int scheduling_types = 0;
  struct t_machine *machine;
  size_t machines_it;

  while ( COMMUNIC[ scheduling_types ].name != 0 )
  {
    if ( strcmp( policy_str, COMMUNIC[ scheduling_types ].name ) == 0 )
    {
      info( "   * Machine %d Communication policy selected %s\n", machine_id, policy_str );

      switch ( scheduling_types )
      {
        case COMMUNIC_FIFO:
          break;
        case COMMUNIC_RR:
        case COMMUNIC_BOOST:
          /* Quantum required */
          expected_quantum_definition = TRUE;
          break;
        default:
          sprintf( comm_conf_error_message, "Communication policy \"%s\" not implemented yet", policy_str );
          return FALSE;
      }

      for ( machines_it = 0; machines_it < Simulator.number_machines; machines_it++ )
      {
        machine = &( Machines[ machines_it ] );

        if ( all_machines )
        {
          machine->communication.policy = scheduling_types;
        }
        else if ( machine->id == machine_id )
        {
          machine->communication.policy = scheduling_types;
          break;
        }
      }
      return TRUE;
    }
    scheduling_types++;
  }

  sprintf( comm_conf_error_message, "invalid communication policy name %s", policy_str );

  return FALSE;
}

t_boolean load_scheduling_quantum( int machine_id, t_boolean all_machines, int quantum )
{
  struct t_machine *machine;
  size_t machines_it;

  if ( quantum < 0 )
  {
    quantum = 0;
  }

  for ( machines_it = 0; machines_it < Simulator.number_machines; machines_it++ )
  {
    machine = &( Machines[ machines_it ] );

    if ( all_machines )
    {
      machine->communication.quantum = quantum;
    }
    else if ( machine->id == machine_id )
    {
      machine->communication.quantum = quantum;
      break;
    }
  }
  return TRUE;
}

t_boolean load_machine_flight_times( int machine_id, char *flight_times_str )
{
  int current_dest_machine;
  char *flight_value_str;
  double flight_value;

  current_dest_machine = 0;

  flight_value_str = strtok( flight_times_str, " " );
  while ( flight_value_str != NULL )
  {
    flight_value = strtod( flight_value_str, NULL );

    if ( current_dest_machine == Simulator.number_machines )
    {
      fprintf( stderr, "   * WARNING: Too many flight times for machine %d\n", machine_id );
      break;
    }

    Simulator.wan.flight_times[ machine_id ][ current_dest_machine ] = flight_value * 1e9;

    current_dest_machine++;
    flight_value_str = strtok( NULL, " " );
  }

  if ( current_dest_machine != Simulator.number_machines )
  {
    sprintf( comm_conf_error_message, "expected %d flight times and just %d present", Simulator.number_machines, current_dest_machine );
    return FALSE;
  }

  return TRUE;
}

/************************************************************************
 ** Posa a la maquina corresponent o a la xarxa externa, els parametres
 ** donats a l'operació col.lectiva indicada.
 ************************************************************************/
t_boolean load_global_op_parameters( t_boolean external_network,
                                     int machine_id,
                                     int global_op_id,
                                     char *FIN_model,
                                     char *FIN_size,
                                     char *FOUT_model,
                                     char *FOUT_size )
{
  struct t_global_op_definition *glop;
  struct t_machine *machine;
  struct t_global_op_information *glop_info;

  int FIN_model_value;
  int FIN_size_value;
  int FOUT_model_value;
  int FOUT_size_value;
  struct t_queue *cua;


  glop = (struct t_global_op_definition *)query_prio_queue( &Global_op, (t_priority)global_op_id );
  if ( glop == NULL )
  {
    sprintf( comm_conf_error_message, "undefined global operation %d", global_op_id );

    return FALSE;
  }

  if ( ( FIN_model_value = get_global_op_model( FIN_model ) ) == -1 )
  {
    sprintf( comm_conf_error_message, "undefined Fan In model (%s)", FIN_model );

    return FALSE;
  }

  if ( ( FIN_size_value = get_global_op_size( FIN_size ) ) == -1 )
  {
    sprintf( comm_conf_error_message, "undefined Fan In size (%s)", FIN_size );

    return FALSE;
  }

  if ( ( FOUT_model_value = get_global_op_model( FOUT_model ) ) == -1 )
  {
    sprintf( comm_conf_error_message, "undefined Fan Out model (%s)", FOUT_model );

    return FALSE;
  }

  if ( ( FOUT_size_value = get_global_op_size( FOUT_size ) ) == -1 )
  {
    sprintf( comm_conf_error_message, "undefined Fan Out size (%s)", FOUT_size );

    return FALSE;
  }


  /* Es determina de quina cua cal modificar les dades */
  if ( external_network == TRUE )
  {
    /* Es tracta de la xarxa externa */
    cua = &Simulator.wan.global_ops_info;
  }
  else
  {
    /* És una de les màquines */
    if ( machine_id < 0 || machine_id > Simulator.number_machines )
    {
      sprintf( comm_conf_error_message, "machine %d not found", machine_id );

      return FALSE;
    }

    cua = &( Machines[ machine_id ].communication.global_ops_info );
  }

  /* S'obte l'estructura amb la informació d'aquesta operació global */
  glop_info = (struct t_global_op_information *)query_prio_queue( cua, (t_priority)global_op_id );

  if ( glop_info == NULL )
  {
    sprintf( comm_conf_error_message, "global operation %d undefined", global_op_id );

    return FALSE;
  }

  /* Es modifiquen les dades*/
  glop_info->FIN_model  = FIN_model_value;
  glop_info->FIN_size   = FIN_size_value;
  glop_info->FOUT_model = FOUT_model_value;
  glop_info->FOUT_size  = FOUT_size_value;

  return TRUE;
}

int get_global_op_model( char *model )
{
  if ( strcmp( model, "0" ) == 0 )
    return ( GOP_MODEL_0 );

  if ( strcmp( model, "CTE" ) == 0 )
    return ( GOP_MODEL_CTE );

  if ( strcmp( model, "LIN" ) == 0 )
    return ( GOP_MODEL_LIN );

  if ( strcmp( model, "LOG" ) == 0 )
    return ( GOP_MODEL_LOG );

  return -1;
}

int get_global_op_size( char *size )
{
  if ( strcmp( size, "MIN" ) == 0 )
    return ( GOP_SIZE_MIN );

  if ( strcmp( size, "MAX" ) == 0 )
    return ( GOP_SIZE_MAX );

  if ( strcmp( size, "MEAN" ) == 0 )
    return ( GOP_SIZE_MEAN );

  if ( strcmp( size, "2MAX" ) == 0 )
    return ( GOP_SIZE_2MAX );

  if ( strcmp( size, "S+R" ) == 0 )
    return ( GOP_SIZE_SIR );

  return -1;
}

/******************************************************************************
 * PROCEDURE 'get_param_external_net_traffic'                                 *
 *****************************************************************************/
/*
 * Intenta obtenir els parametres que definexen el traffic de la xarxa externa.
 * Això només s'ha de fer així temporalment. Un cop definits correctament,
 * s'haurien de posar en un #DEFINE i no tornar-los a tocar.
 */
