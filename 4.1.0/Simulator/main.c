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

  $URL::                                          $:  File
  $Rev::                                          $:  Revision of last commit
  $Author::                                       $:  Author of last commit
  $Date::                                         $:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include "define.h"
#include "subr.h"
#include "types.h"
#include "extern.h"

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

#include "mallocame.h"
#include "memory.h"
#include "paraver.h"
#include "ports.h"
#include "read.h"
#include "schedule.h"
#include "semaphore.h"
#include "task.h"
#include "random.h"

#include "dimemas_io.h"

#include "configuration.h"
#include "simulator.h"

#ifdef VENUS_ENABLED
#include "listE.h"
#include "venusclient.h"
#endif

#include <time.h>
#include <errno.h>

#include <execinfo.h>
#include <signal.h>
#include <sys/types.h>
#include <float.h>

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

  printf ("%d\n", a);
  printf("Stack frames (%d):\n", size);

  for (i = 0; i < size; i++)
    printf( "%s\n", strings[i]);

  free (strings);
};

/// Segmentation fault signal handler.
void
segfaultHandler(int sigtype)
{
    printf("Segmentation fault \n");
    print_backtrace(sigtype);
    exit(-1);
}
*/

t_boolean Pallas_output = TRUE;

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

FILE     *salida_datos;
char     *fichero_salida     = NULL;
char     *paraver_file       = NULL;
char     *paraver_pcf_insert = NULL;

char *file_for_event_to_monitorize = (char *)0;
FILE *File_for_Event;

dimemas_timer         paraver_start = -DBL_MAX;
dimemas_timer         paraver_end = -DBL_MAX;
t_boolean             paraver_priorities;

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

int  RD_SYNC_message_size;   /* Minimum message size to use Rendez-vous
                              * <0 = Asynchronous unless specified: */
int  RD_SYNC_use_trace_sync; /* Use the synchronous field of the sends
                              * in the trace? */

#ifdef VENUS_ENABLED
#define USAGE \
"Usage: %s [-h] [-v] [-d] [-x[s|e|p]] [-o[l] output-file] [-T time] [-l] [-C]\n"\
"\t[-p[a|b] paraver-file [-y time] -z time]] [-x[s|e]]\n" \
"\t[-e event_type] [-g event_output_info] [-A sddf-file]\n" \
"\t[-F] [-S sync_size] [-venus] [-venusconn host:port] config-file\n"
#else
#define USAGE \
"Usage: %s [-h] [-v] [-d] [-x[s|e|p]] [-o[l] output-file] [-T time] [-l] [-C]\n"\
"\t[-p[a|b] paraver-file [-y time] -z time]] [-x[s|e]]\n" \
"\t[-e event_type] [-g event_output_info] [-A sddf-file]\n" \
"\t[-F] [-S sync_size] config-file\n"
#endif


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
}

static dimemas_timer read_timer(char *c);
static int read_size(char *c);


void parse_arguments(int argc, char *argv[])
{
  int   j;
  char *conn_url;

  if (argc == 1)
  {
    fprintf (stderr, USAGE, argv[0]);
    exit (1);
  }

  if ((argc == 2) && (strcmp (argv[1], "-v") == 0))
  {
    show_version (argv[0]);
    exit (0);
  }

  if ((argc == 2) &&
      ((strcmp(argv[1], "-h") == 0) || (strcmp(argv[1], "-?") == 0) ||
      (strcmp(argv[1], "-help") == 0)))
  {
    help_message(argv[0]);
    exit (0);
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
              exit (1);
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
              exit (1);
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
            output_level = 0;
          if (output_level > MAX_OUTPUT_LEVEL)
            output_level = MAX_OUTPUT_LEVEL;
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
              exit (1);
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
        default:
          fprintf (stderr, USAGE, argv[0]);
          exit (1);
      }
    }
  }

  if (j >= argc)
  {
    fprintf (stderr, USAGE, argv[0]);
    exit (1);
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

static int read_size(char *c)
{
  int i, mida_tmp, mida;
  char unitats;

  /* Un int es massa petit si les unitats son > kb */

  i = sscanf (c, "%d%c", &mida_tmp,&unitats);
  if (i==0)
  {
    fprintf(stderr,"Incorrect minimum message size to use Rendez vous!\n");
    exit(1);
  }
  else if (i==1)
    mida=mida_tmp; /* La mida es en bytes */
  else /* S'ha indicat les unitats */
  {
    switch (unitats)
    {
      case 'k': /* La mida es en Kb */
        mida=mida_tmp<<10;
        break;

      case 'm': /* La mida es en Mb */
        mida=mida_tmp<<20;
        break;

      case 'g': /* La mida es en Gb */
        mida=mida_tmp<<30;
        break;

      case 'b':
      default: /* La mida es en bytes */
        mida=mida_tmp;
        break;
    }
  }

  return(mida);
}

int
main (int argc, char *argv[])
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
  PARAVER_init(paraver_file,
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
      exit (1);
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
      exit (1);
    }

    if ((paraver_initial_timer) || (paraver_final_timer))
    {
      fprintf (stderr, USAGE, argv[0]);
      exit (1);
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
        exit (1);
      }
    }
    else
    {
      File_for_Event = stdout;
    }
  }

  /* Initial routines */

  MALLOC_Init();

  /* Modules Initial routines */
  SIMULATOR_Init();

  /* Configuration file (target machine description and application mapping)
   * parsing */
  CONFIGURATION_Init(config_file);
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

  PORT_init ();

#ifdef VENUS_ENABLED
  VC_Init(); /* VENUS CLIENT */
#endif

  EVENT_init ();
  SCHEDULER_init ();
  COMMUNIC_init ();
  MEMORY_init ();
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

#ifdef USE_EQUEUE

#ifdef VENUS_ENABLED
  while ((top_Eevent (&Event_queue) != E_NIL) || (VC_is_enabled() && (top_Eevent(&Interactive_event_queue) != E_NIL)))
#else /* !VENUS_ENABLED */
  while (top_Eevent (&Event_queue) != E_NIL)
#endif

#else /* !USE_EQUEUE */

#ifdef VENUS_ENABLED
  while ((top_event (&Event_queue) != E_NIL) || (VC_is_enabled() && (top_event(&Interactive_event_queue) != E_NIL)))
#else /* !VENUS_ENABLED */
  while (top_event (&Event_queue) != E_NIL)
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

  if (debug)
  {
    printf ("\n");
    PRINT_TIMER (current_time);
    printf (": END SIMULATION\n\n");
  }

  /* Modules final routines */
  TASK_end ();
  FS_End ();
  PORT_end ();
  EVENT_end ();
  SCHEDULER_end ();
  COMMUNIC_end ();
  MEMORY_end ();
  SEMAPHORE_end ();

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

  PARAVER_end();

  strcpy (message_buffer, "");
  for (i = 0; i < argc; i++)
  {
    strcat (message_buffer, argv[i]);
    strcat (message_buffer, " ");
  }

  /* Free reserved pointers
  free_all_reserved_pointers(); */

  MALLOC_End();

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
