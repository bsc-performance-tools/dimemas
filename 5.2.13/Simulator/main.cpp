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

  $URL:: https://svn.bsc.es/repos/DIMEMAS/trunk/S#$:  File
  $Rev:: 162                                      $:  Revision of last commit
  $Author:: jgonzale                              $:  Author of last commit
  $Date:: 2014-07-31 13:17:31 +0200 (jue, 31 jul #$:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include <cmath>
#include <ctime>
#include <cerrno>
#include <csignal>
#include <cfloat>

#include <ezOptionParser.hpp>

#include <assert.h>

extern "C"
{
#include <deadlock_analysis.h>

#include <execinfo.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "extern.h"

#include "define.h"
#include "subr.h"
#include "types.h"

#include "aleatorias.h"
#include "communic.h"
#include "cp.h"
#include "cpu.h"
#include "events.h"
#include "fs.h"
#include "list.h"
#ifdef USE_EQUEUE
#include "listE.h"
#endif

#include "memory.h"
#include "paraver.h"
#include "ports.h"
#include "read.h"
#include "schedule.h"
#include "semaphore.h"
#include "task.h"
#include "random.h"

// EEE
#include "eee_configuration.h"

#include "dimemas_io.h"

#include "configuration.h"
#include "simulator.h"

#ifdef VENUS_ENABLED
#include "listE.h"
#include "venusclient.h"
#endif

}

// Compute time differences
double ddiff(struct timespec start, struct timespec end)
{
  return ( (double)(end.tv_sec-start.tv_sec) + (double)1e-9*(end.tv_nsec-start.tv_nsec) );
}

/*
void print_backtrace (int a) {

  // prevent infinite recursion if print_backtrace() causes another segfault
  signal(SIGSEGV, SIG_DFL);
  signal(SIGABRT, SIG_DFL);

  void *array[100];
  int size;
  char **strings;
  int i;

  size = backtrace (array, 100);
  strings = backtrace_symbols (array, size);

  printf("Signal type = %d\n", a);
  printf("Stack frames (%d):\n", size);

  for (i = 0; i < size; i++)
    printf( "%s\n", strings[i]);

  free (strings);
};

/// Segmentation fault signal handler.
void segfaultHandler(int sigtype)
{
    printf("Segmentation fault \n");
    print_backtrace(sigtype);
    exit(EXIT_FAILURE);
}
*/

t_boolean Pallas_output = TRUE;

char      *parameter_tracefile = (char*) 0;

double    parameter_bw  = DBL_MIN;
double    parameter_lat = DBL_MIN;

int       parameter_predefined_map = MAP_NO_PREDEFINED;
int       parameter_tasks_per_node;

char *config_file  = "rc.dimemas";
int   output_level = 0;
int   debug        = 0;
int   assert       = 0;

int   port_ids  = MIN_PORT_NUMBER;


char  *fichero_fs     = (char*)0;
char  *fichero_sch    = (char*)0;
char  scheduling_policy[50];
char  *fichero_comm   = (char*)0;
char  *fichero_random = (char*)0;

t_boolean reload_Ptasks = FALSE;
t_boolean reload_done   = FALSE;
int       reload_limit  = 10;
t_boolean reload_while_longest_running = FALSE;

t_boolean full_out_info  = FALSE;
t_boolean short_out_info = FALSE; /* Only simulation time */

t_boolean Critical_Path_Analysis=FALSE;

t_boolean wait_logical_recv = FALSE;

FILE     *salida_datos;
char     *fichero_salida     = NULL;
char     *paraver_file       = NULL;
char     *paraver_pcf_insert = NULL;

char *file_for_event_to_monitorize = (char *)0;
FILE *File_for_Event;

static dimemas_timer  paraver_start = -DBL_MAX;
static dimemas_timer  paraver_end = -DBL_MAX;
static t_boolean      paraver_priorities;

/* Tracefile detectitons
t_boolean      binary_files = FALSE; /* Currently unused
t_boolean      gzip_files   = FALSE; /* Compressed traces
t_boolean      new_trace_format = FALSE;  /* New dimemas trace format
*/

static t_boolean HOT_cache = FALSE;

char            message_buffer[1024];

dimemas_timer   time_limit;

int             sintetic_io_applications = 0;

struct t_queue CP_NODES;
struct t_queue *CP_nodes = &CP_NODES;

t_boolean NEW_MODIFICATIONS = FALSE;

long long int  RD_SYNC_message_size;   /* Minimum message size to use Rendez-vous
                              * <0 = Asynchronous unless specified: */
int  RD_SYNC_use_trace_sync; /* Use the synchronous field of the sends
                              * in the trace? */

#ifdef VENUS_ENABLED
#define USAGE \
"Usage: %s [-h] [-v] [-d] [-x[s|e|p]] [-o[l] output-file] [-T time] [-l] [-C]\n"\
"\t[-p[a|b] paraver-file [-y time] -z time]] [-x[s|e]]\n" \
"\t[-e event_type] [-g event_output_info] [-a input-trace]\n" \
"\t[-F] [-S sync_size] [-venus] [-venusconn host:port]\n"\
"\t[-w] [-ES] [-Eo eee_network_definition] [-Ef eee_frame_size]\n"\
"\t[--dim input-trace] [--bw bandwidth] [--lat latency]\n"\
"\t[--ppn processors_per_node] [--fill] [--interlvd]\n"\
"\t[--clean-deadlocks <until-trace-progression> ]\n"\
"\tconfig-file\n"
#else
#define USAGE \
"Usage: %s [-h] [-v] [-d] [-x[s|e|p]] [-o[l] output-file] [-T time] [-l] [-C]\n"\
"\t[-p[a|b] paraver-file [-y time] -z time]] [-x[s|e]]\n" \
"\t[-e event_type] [-g event_output_info] [-F] [-S sync_size]\n" \
"\t[-w] [-ES] [-Eo eee_network_definition] [-Ef eee_frame_size]\n"\
"\t[--dim input-trace] [--bw bandwidth] [--lat latency]\n"\
"\t[--ppn processors_per_node] [--fill] [--interlvd]\n"\
"\t[--clean-deadlocks <until-trace-progression> ]\n"\
"\t config-file\n"

#endif

int with_deadlock_analysis = 0;
int reboots_counter = 0;
float danalysis_deactivation_percent = 1;
t_boolean simulation_rebooted = FALSE;

static bool ParseArguments(const int argc, const char* argv)
{
  return true;
}

static void
show_version(char *name)
{
  int             i = 0;

  printf ("Dimemas v%s \n",     VERSION);
  printf ("Binary file  %s \n", name);
  printf ("Compiled  %s \n",    DATE);


  printf ("Implemented CPU Scheduling policies:\n");
  while (SCH[i].name)
  {
    printf ("    %s\n", SCH[i].name);
    i++;
  }

  FS_show_version();

/*
 * printf ("(c) DAC-UPC 1993-95\n"); printf ("Authors: Sergi Girona, Jesus
 * Labarta and Toni Cortes\n");
 */
}

static void
help_message(char *tname)
{
  printf ("Dimemas version %s\n\n", VERSION);
#ifndef NO_COMPILATION_INFO
  printf ("Compiled on %s \n", DATE);
#endif
  printf (USAGE, tname);
  printf ("\nRequired arguments:\n");
  printf ("\tconfig-file\tDimemas configuration file\n");
  printf ("\nSupported options:\n");
  printf ("\t-h\t\tDisplay this help message\n");
  printf ("\t-v\t\tDisplay Dimemas version information\n");
  printf ("\t-d\t\tEnable debug output\n");
  printf ("\t-xa\t\tForce assertations - Vladimir: check if optimizations caused some errors\n");
  printf ("\t-xs\t\tEnable extra scheduling debug output\n");
  printf ("\t-xe\t\tEnable extra event manager debug output\n");
  printf ("\t-xp\t\tEnable extra Paraver trace generation debug output\n");
  printf ("\t-o[l] file\tSet output file (default: stdout) (l:long info) \n");
  printf ("\t-t\t\tShow only simulation time as output\n");
  printf ("\t-T time\t\tSet simulation stop time\n");
  printf ("\t-l\t\tEnable in-core operation\n");
  printf ("\t-C\t\tPerform critical path analysis\n");
  printf ("\t-p file\t\tGenerate Paraver tracefile (ASCII)\n");
  printf ("\t-pc file\tUse the given file as output Paraver configuration file\n");
  printf ("\t-y time\t\tSet Paraver tracefile start time\n");
  printf ("\t-z time\t\tSet Paraver tracefile stop time\n");
  printf ("\t-e event_type\tShow time distance between events occurrence\n");
  printf ("\t-g event_output\tFile for output information on events occurrence\n");
  printf ("\t-F\t\tIgnore synchronism send trace field\n");
  printf ("\t-S sync_size\tMinimum message size to use Rendez vous\n");
#ifdef VENUS_ENABLED
  printf ("\t-venus\tConnect to a Venus server at localhost:'default venus port'\n");
  printf ("\t-venusconn host:port\tConnect to a Venus server at host:port\n");
#endif
  printf ("\t-w\tWhen generating Paraver trace, output the LOGICAL_RECV times when Wait primitives take place (Default: at IRecv execution time)\n");
  printf ("\t-ES\tEnables the EEE network model (you must use '-Eo' and '-Ef'\n");
  printf ("\t-Eo eee_network_definition\tSet the filename where the EEE network is defined\n");
  printf ("\t-Ef frame_size\tSets the EEE network frame size (in bytes)\n");
  printf ("\t--dim input-trace\tSet input trace (overrides the configuration file)\n");
  printf ("\t--bw  bandwidth\tSet inter-node bandwidth (MBps, overrides the configuration file)\n");
  printf ("\t--lat latency\tSet inter-node latency for all nodes (seconds, overrides the configuration file)\n");
  printf ("\t--fill\tSet node filling task mapping (overrides the configuration file)\n");
  printf ("\t--ppn tasks_per_node\tSet 'n' tasks per node mapping (overrides the configuration file)\n");
  printf ("\t--interlvd\tSet interleaved node tasks mapping (overrides the configuration file)\n");
  printf ("\t--clean-deadlocks <until-trace-progression> \tA deadlock analysis is performed and if detects, tries to solve it.\n");



}

static dimemas_timer read_timer(char *c);
static long long int read_size(char *c);


void parse_arguments(int argc, char *argv[])
{
  int   j;
  char *conn_url, *endptr;

  if (argc == 1)
  {
    fprintf (stderr, USAGE, argv[0]);
    exit (EXIT_FAILURE);
  }

  if ((argc == 2) && (strcmp (argv[1], "-v") == 0))
  {
    show_version (argv[0]);
    exit (EXIT_SUCCESS);
  }

  if ((argc == 2) &&
      ((strcmp(argv[1], "-h") == 0) || (strcmp(argv[1], "-?") == 0) ||
      (strcmp(argv[1], "-help") == 0)))
  {
    help_message(argv[0]);
    exit (EXIT_SUCCESS);
  }

  j = 1;


  if (argv[1][0] == '-')
  {
    for (j = 1; (j < argc - 1) && (argv[j][0] == '-'); j++)
    {
      switch (argv[j][1])
      {
        case 'o':
          if (argv[j][2] == '1')
            full_out_info = TRUE;
          j++;
          fichero_salida = argv[j];
          break;
        case 'P':
          paraver_priorities = TRUE;
          break;
        case 'd':
          debug = D_LINKS|D_COMM;
          #ifdef DEBUG_STARTUPS
          debug |= D_SCH; /* FEC: He afegit aquesta linia! */
          #endif
          break;

        /* JGG (13/04/2005): Debug avanzado */
        case 'x':
          switch (argv[j][2])
          {
            case 's':
              debug |= D_SCH; /* FEC: He afegit aquesta linia! */
              break;
            case 'e':
              debug |= D_EV;
              break;
            case 'p':
              debug |= D_PRV;
              break;
            case 't':
              debug |= D_TASK;
              break;
            case 'a':
              assert = 1;
              break;
            default:
              fprintf (stderr, USAGE, argv[0]);
              exit (EXIT_FAILURE);
              break;
          }
          break;
        case 'T':
            j++;
            time_limit = read_timer (argv[j]);
            break;
        case 'z':
            j++;
            stop_paraver        = read_timer (argv[j]);
            paraver_final_timer = TRUE;
            break;
        case 'y':
            j++;
            start_paraver         = read_timer (argv[j]);
            paraver_initial_timer = TRUE;
            break;
        case 'p':
          switch (argv[j][2])
          {
            case 0:
            case 'a':
              j++;
              paraver_file = argv[j];
              break;
            case 'c':
              j++;
              paraver_pcf_insert = argv[j];
              break;
            default:
              fprintf (stderr, USAGE, argv[0]);
              exit (EXIT_FAILURE);
          }
          break;
#ifdef VENUS_ENABLED
        case 'v':
          if (!strcmp(argv[j], "-venus")) {
            conn_url = NULL;
          }
          else if (!strcmp(argv[j], "-venusconn")) {
            j++;
            conn_url = argv[j];
          }
          else {
            show_version (argv[0]);
            exit (EXIT_FAILURE);
          }

          VC_enable(conn_url); // Enable the venus client!
          break;
#endif
        case 'i':
          j++;
          output_level = atoi (argv[j]);
          if (output_level < 0)
          {
            output_level = 0;
          }
          if (output_level > MAX_OUTPUT_LEVEL)
          {
            output_level = MAX_OUTPUT_LEVEL;
          }
          break;
        case 'f':
          j++;
          CONFIGURATION_Set_FileSystem_Configuration_File(argv[j]);
          break;
        case 's':
          j++;
          CONFIGURATION_Set_Scheduling_Configuration_File(argv[j]);
          break;
        case 'I':
          j++;
          sintetic_io_applications = atoi (argv[j]);
          if (sintetic_io_applications < 0)
          {
            sintetic_io_applications = 0;
          }
          break;
        case 'N':
          NEW_MODIFICATIONS = TRUE;
          break;
        case 'W':
          HOT_cache = TRUE;
          break;
        case 'r':
          j++;
          reload_Ptasks = TRUE;
          reload_limit = atoi (argv[j]);
          if (reload_limit == 0) {
            /* Changed behaviour: 0 is "inifinite" until all have executed at least once */
            reload_limit = MAX_RELOAD_LIMIT;
            reload_while_longest_running = TRUE;
          }
          if (reload_limit < 0) {
              fprintf (stderr, "# reloads must be >= 0, given: -r %d\n", reload_limit);
              fprintf (stderr, USAGE, argv[0]);
              exit (EXIT_FAILURE);
          }
          break;
        case 'O':
          Pallas_output = FALSE;
          break;
        case 'C':
          Critical_Path_Analysis = TRUE;
          break;
        case 'e':
          j++;
          event_to_monitorize = atoi (argv[j]);
          monitorize_event    = TRUE;
          break;
        case 'g':
          j++;
          file_for_event_to_monitorize = argv[j];
          break;
        case 'F':
          RD_SYNC_use_trace_sync = FALSE;
          break;
        case 'S':
          j++;
          RD_SYNC_use_trace_sync = FALSE;
          RD_SYNC_message_size = read_size (argv[j]);
          break;
        case 't':
          short_out_info = TRUE; /* Only simulation time */
          break;
        case 'w':
          wait_logical_recv = TRUE;
          break;
        case '-':
          if (strncmp(argv[j], "--clean-deadlocks", 11) == 0)
          {
            with_deadlock_analysis = 1;
            j++;
            danalysis_deactivation_percent = atof(argv[j]);
          }
          else if (strncmp(argv[j], "--dim", 5) == 0)
          {
            j++;
            parameter_tracefile = argv[j];
            break;
          }
          else if (strncmp(argv[j], "--bw", 4) == 0)
          {
            j++;

            parameter_bw = strtod(argv[j], &endptr);

            if (*endptr != 0 || parameter_bw < 0)
            {
              die("Wrong bandwidth (--bw) value: %s\n", argv[j]);
            }
          }
          else if (strncmp(argv[j], "--lat", 5) == 0)
          {
            j++;
            parameter_lat = strtod(argv[j], &endptr);

            if (*endptr != 0 || parameter_lat < 0)
            {
              die("Wrong latency (--lat) value: %s\n", argv[j]);
            }

            /* This adjustment is done internally!!
             parameter_lat = parameter_lat*1e9;
            */
          }
          else if (strncmp(argv[j], "--ppn", 5) == 0)
          {
            if (parameter_predefined_map != MAP_NO_PREDEFINED)
            {
              warning("More than one predefined map provided, using the last one ('n' tasks per node)\n");
            }

            j++;
            parameter_predefined_map = MAP_N_TASKS_PER_NODE;
            parameter_tasks_per_node = atoi(argv[j]);
          }
          else if (strncmp(argv[j], "--fill", 6) == 0)
          {
            if (parameter_predefined_map != MAP_NO_PREDEFINED)
            {
              warning("More than one predefined map provided, using the last one (fill nodes)\n");
            }
            parameter_predefined_map = MAP_FILL_NODES;
          }
          else if (strncmp(argv[j], "--interlvd", 8) == 0)
          {
            if (parameter_predefined_map != MAP_NO_PREDEFINED)
            {
              warning("More than one predefined map provided, using the last one (interleaved)\n");
            }
            parameter_predefined_map = MAP_INTERLEAVED;
          }
          else
          {
            fprintf (stderr, USAGE, argv[0]);
            exit (EXIT_FAILURE);
          }
          break;

        case 'E': // Karthikeyan 3L Network Code
          switch (argv[j][2])
          {
              case 'S': // State of EEE 1 for ON
                j++;
                eee_enabled = atof(argv[j]);
                if(eee_enabled) {
                    printf("EEE Enabled!\nFrame Header Size and Cfg FileName MUST be specified!\n");
                }
                break;
              case 'o':
                j++;
                if(eee_enabled){
                    eee_config_file = argv[j];
                } else {fprintf (stderr, USAGE, argv[0]); exit(EXIT_FAILURE);}
                break;
              case 'f':
                j++;
                if(eee_enabled){
                    eee_frame_header_size = atof(argv[j]);
                    printf("Frame Header Size:%d\n",eee_frame_header_size);
                } else {fprintf (stderr, USAGE, argv[0]); exit(EXIT_FAILURE);}
                break;
              default:
                {fprintf (stderr, USAGE, argv[0]); exit(EXIT_FAILURE);}
          }
          break;

        default:
          fprintf (stderr, USAGE, argv[0]);
          exit (EXIT_FAILURE);
      }
    }
  }

  if (j >= argc)
  {
    fprintf (stderr, USAGE, argv[0]);
    exit (EXIT_FAILURE);
  }

  config_file = argv[j];

  return;
}

static dimemas_timer read_timer(char *c)
{
  int           h, u;
  int           i;
  float         tmp_float;
  dimemas_timer tmp_timer;;

  i = sscanf (c, "%dH%du", &h, &u);
  if (i == 2)
  {
    HOUR_MICRO_TO_TIMER (h, u, tmp_timer);
  }
  else
  {
    tmp_float = (float) atof (c);
    tmp_float = tmp_float * 1e9;
    FLOAT_TO_TIMER (tmp_float, tmp_timer);
  }
  return (tmp_timer);
}

static long long int read_size(char *c)
{
  int i;
  long long int mida_tmp, mida;
  char unitats;

  /* Un int es massa petit si les unitats son > kb */

  i = sscanf (c, "%lld%c", &mida_tmp, &unitats);
  if (i == 0)
  {
    fprintf(stderr,"Incorrect minimum message size to use Rendez vous!\n");
    exit(EXIT_FAILURE);
  }
  else if (i==1)
  {
    mida = mida_tmp; /* La mida es en bytes */
  }
  else /* S'ha indicat les unitats */
  {
    switch (unitats)
    {
      case 'k': /* La mida es en Kb */
      case 'K':
        mida=mida_tmp<<10;
        break;

      case 'm': /* La mida es en Mb */
      case 'M':
        mida=mida_tmp<<20;
        break;

      case 'g': /* La mida es en Gb */
      case 'G':
        mida=mida_tmp<<30;
        break;

      case 'b':
      case 'B':
      default: /* La mida es en bytes */
        mida=mida_tmp;
        break;
    }
  }

  return(mida);
}

int main (int argc, char *argv[])
{

  int             i;
  int             j;
  dimemas_timer   to_parent;
  char            nom1[128];
  char            *c;

  unsigned long long event_counter = 0;
  struct timespec time_start;

  //   int               venusconn = "";
// Vladimir:  this is to eliminate the warning
// should not break the program??
  char *            venusconn = "";

  // signal(SIGSEGV, segfaultHandler);

  ASS_TIMER (current_time, 0);
  ASS_TIMER (final_statistical_time, 0);
  ASS_TIMER (time_limit, TIME_LIMIT); /* Simulation restricted to 8 hours */

  /* Per defecte les comunicacions son asincrones independentment de la mida */
  RD_SYNC_message_size = -1;
  /* Per defecte les comunicacions poden ser sincrones si s'indica a la traca */
  RD_SYNC_use_trace_sync = TRUE;

  /* Before performing any operation, we have to set up the file pointers
   * availability */
  IO_Init();

  parse_arguments(argc, argv);


  /* Paraver trace initialization if needed */
  PARAVER_Init(paraver_file,
               paraver_pcf_insert,
               paraver_start,
               paraver_end,
               paraver_priorities);

  if (fichero_salida != (char*) 0)
  {
    salida_datos = IO_fopen (fichero_salida, "w");
    if (salida_datos == (FILE *) 0)
    {
      fprintf (stderr, USAGE, argv[0]);
      printf ("Can't open %s for Output\n", fichero_salida);
      exit (EXIT_FAILURE);
    }
  }
  else
  {
    salida_datos = stdout;
  }

  if ((paraver_file == NULL ))
  {
    if (paraver_priorities)
    {
      fprintf (stderr, USAGE, argv[0]);
      exit (EXIT_FAILURE);
    }

    if ((paraver_initial_timer) || (paraver_final_timer))
    {
      fprintf (stderr, USAGE, argv[0]);
      exit (EXIT_FAILURE);
    }
  }

  if (monitorize_event)
  {
    if (file_for_event_to_monitorize != NULL )
    {
      File_for_Event = IO_fopen (file_for_event_to_monitorize, "w");
      if (File_for_Event == NULL)
      {
        fprintf (stderr, USAGE, argv[0]);
        fprintf (stderr,
                "Can't open %s for Event monitor\n",
                file_for_event_to_monitorize);
        exit (EXIT_FAILURE);
      }
    }
    else
    {
      File_for_Event = stdout;
    }
  }

  /* Initial routines */

  // MALLOC_init();

  /* Modules Initial routines */
  SIMULATOR_Init(config_file,
                 parameter_tracefile,
                 parameter_bw,
                 parameter_lat,
                 parameter_predefined_map,
                 parameter_tasks_per_node);

  /* Configuration file (target machine description and application mapping)
   * parsing */
  // CONFIGURATION_Init(config_file);
  RANDOM_Init ();
  TASK_Init(sintetic_io_applications);

  if (Critical_Path_Analysis)
  {
    create_queue (CP_nodes);
    /* Critical path only allowed if a single task is used */
    if (count_queue(&Ptask_queue) != 1)
    {
      printf("Warning: Critical Path analysis not allowed with %d Ptask\n",
             count_queue(&Ptask_queue));
      Critical_Path_Analysis = FALSE;
    }
  }

  // check_full_nodes ();

  PORT_Init ();

#ifdef VENUS_ENABLED
  VC_Init(); /* VENUS CLIENT */
#endif

  /* Code to set-up an Energy Efficient Ethernet (EEE) */
  EEE_Init();

  EVENT_Init ();
  SCHEDULER_Init ();
  COMMUNIC_Init (parameter_tracefile, danalysis_deactivation_percent);
  MEMORY_Init ();
  SEMAPHORE_Init ();
  FS_Init();

  CPU_Get_Unique_CPU_IDs();

  reload_events ();

  if (debug)
  {
    printf ("\n");
    PRINT_TIMER (current_time);
    printf (": START SIMULATION\n\n");
  }

  ASS_ALL_TIMER (to_parent, current_time);

#ifdef SHOW_PERFORMANCE
  clock_gettime(CLOCK_MONOTONIC, &time_start);
#endif

REBOOT:

#ifdef USE_EQUEUE

#ifdef VENUS_ENABLED
  while ((top_Eevent (&Event_queue) != E_NIL) || (VC_is_enabled() && (top_Eevent(&Interactive_event_queue) != E_NIL)) && !simulation_rebooted)
#else /* !VENUS_ENABLED */
  while (top_Eevent (&Event_queue) != E_NIL && !simulation_rebooted)
#endif

#else /* !USE_EQUEUE */

#ifdef VENUS_ENABLED
  while ((top_event (&Event_queue) != E_NIL) || (VC_is_enabled() && (top_event(&Interactive_event_queue) != E_NIL)) && !simulation_rebooted)
#else /* !VENUS_ENABLED */
  while (top_event (&Event_queue) != E_NIL && !simulation_rebooted)
#endif

#endif /* USE_EQUEUE */

  {
    struct t_event* current_event;

#ifdef SHOW_PERFORMANCE
    if ((event_counter % 100000UL) == 0)
    {
      struct timespec time_now;

      clock_gettime(CLOCK_MONOTONIC, &time_now);
      double real_time = ddiff(time_start, time_now);

      printf("** Event #%llu   T=%.7f  ( %.2fus)   Elapsed: %.7fs  (%.2fs)  ev/sec=%.0f\n",
             event_counter,
             current_time/1e9,
             current_time,
             real_time,
             real_time,
             (real_time > 0.0) ? ((double)event_counter)/real_time : 0.0);
    }
    event_counter++;
#endif /* SHOW_PERFORMANCE */

    /*
    if ((NEQ_0_TIMER (time_limit)) && GT_TIMER (current_time, time_limit))
    {
    */
    if (OUT_OF_LIMIT(current_time))
    {
      printf ("\n");
      PRINT_TIMER (current_time);
      printf (": END SIMULATION due time limit\n\n");
      break;
    }

#ifdef USE_EQUEUE
    current_event = outFIFO_Eevent(&Event_queue);
#else
    current_event = outFIFO_event(&Event_queue);
#endif
    /* DEBUG
    if (current_event->thread != NULL)
    {
      printf ("Event selected from Thread %02d\n",
              current_event->thread->threadid);
    }
    */

    event_manager(current_event);
  }

  if (with_deadlock_analysis)
  {
    if (simulation_rebooted || DEADLOCK_check_end())
    {
      // this events must be freed
      remove_queue_elements(&Event_queue);

      SIMULATOR_reset_state();
      COMMUNIC_reset_deadlock();

      reboots_counter++;

      /*if (reboots_counter == 0)
      {
        char * to = strstr(paraver_file, ".prv");
        int name_size = (to-paraver_file);

        char * new_paraver_file;

        new_paraver_file = (char *)calloc(sizeof(char),name_size+18);
        memcpy(new_paraver_file, paraver_file, (to-paraver_file));
        char * extension = ".undeadlocked.prv\0";
        memcpy(new_paraver_file+name_size, extension, 18);
        paraver_file = new_paraver_file;
      }*/

      // Ends the actual erroneous paraver trace
      // Starts new paraver trace with ".undeadlocked."

      PARAVER_End(FALSE);
      PARAVER_Init(paraver_file,
                 paraver_pcf_insert,
                 paraver_start,
                 paraver_end,
                 paraver_priorities);
#if DEBUG
      printf("\n-> " ANSI_COLOR_RED "RESTARTING SIMULATION(%d) " ANSI_COLOR_RESET "\n\n", reboots_counter);
#endif

      // This call load the events that threads have in actions
      // for this reason, before this we have to read the new actions
      reload_events();

      simulation_rebooted = FALSE;
      goto REBOOT;
    }
  }

  if (reboots_counter > 0)
  {
    printf("\n**** Deadlocks ****\n\n");
    printf("%d deadlocks has been successfully cleaned.\n", reboots_counter);
    printf("\n");
  }

  // Finalizing simulation

  if (!short_out_info)
  {
    printf ("\n");
    PRINT_TIMER (current_time);
    printf (": END SIMULATION\n\n");
  }
  /* Modules final routines */
  FS_End ();
  PORT_End ();
  EVENT_End ();
  SCHEDULER_End ();
  COMMUNIC_End ();
  MEMORY_End ();
  SEMAPHORE_End ();

#ifdef VENUS_ENABLED
  VC_End();
#endif


  /* Es calcula el temps final de la simulacio */
  calculate_execution_end_time();

  /* Statistical results */
  if (Critical_Path_Analysis == FALSE)
  {
    show_statistics (Pallas_output);
  }
  else
  {
    fprintf (salida_datos,"Execution time:\t");
    FPRINT_TIMER (salida_datos, execution_end_time);
    fprintf (salida_datos, "\n\n");
    show_CP_graph();
  }

  PARAVER_End(TRUE);
  TASK_End ();


  struct rusage usage;

  if (debug)
  {
    if (getrusage(RUSAGE_SELF, &usage) == -1)
    {
      printf("Unable to get memory usage statistics\n");
    }
    else
    {
      printf("Maximum memory used: %ld\n", usage.ru_maxrss);
    }
  }

  // printf("Possible living actions: %ld\n", READ_get_living_actions());

  // exit(0);

  strcpy (message_buffer, "");
  for (i = 0; i < argc; i++)
  {
    strcat (message_buffer, argv[i]);
    strcat (message_buffer, " ");
  }

  /* Free reserved pointers
  free_all_reserved_pointers(); */

  // MALLOC_End();

  IO_fclose (salida_datos);

  /* Change output tabs per spaces */
  if (fichero_salida != NULL)
  {
    /* c = (char *)tempnam (".", nom1);*/
    c = "Dimemas_OUTPUT.tmp";
    rename (fichero_salida, c);

    close (0);
    MYOPEN (c, O_RDONLY);
    unlink (c);
    close (1);
    MYOPEN (fichero_salida, O_WRONLY| O_TRUNC| O_CREAT,0600);
    execlp ("col", "dimemas_col", "-x", (char *)0);
  }

  exit(EXIT_SUCCESS);
}
