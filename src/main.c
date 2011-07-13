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

#include "define.h"
#include "types.h"

#include "aleatorias.h"
#include "communic.h"
#include "cp.h"
#include "cpu.h"
#include "events.h"
#include "extern.h"
#include "fs.h"
#include "list.h"
#include "mallocame.h"
#include "memory.h"
#include "paraver.h"
#include "ports.h"
#include "read.h"
#include "schedule.h"
#include "sddf.h"
#include "semaphore.h"
#include "task.h"

#ifdef VENUS_ENABLED
#include "venusclient.h"
#endif


#include <errno.h>

dimemas_timer   current_time;
dimemas_timer   final_statistical_time;

struct t_simulator sim_char;

char *config_file  = "rc.dimemas";
int   output_level = 0;
int   debug        = 0;

int   port_ids = MIN_PORT_NUMBER;
int   Ptask_ids = 1;

char  *fichero_fs     = (char*)0;
char  *fichero_sch    = (char*)0;
char  *fichero_comm   = (char*)0;
char  *fichero_random = (char*)0;

t_boolean reload_Ptasks = FALSE;
t_boolean reload_done   = FALSE;
int       reload_limit  = 10;

t_boolean dimemas_GUI   = FALSE;

t_boolean full_out_info  = FALSE;
t_boolean short_out_info = FALSE; /* Only simulation time */

t_boolean Critical_Path_Analysis=FALSE;

FILE     *salida_datos;
char     *fichero_salida = (char*) 0;
char     *paraver_file   = (char*) 0;

char *file_for_event_to_monitorize = (char *)0;
FILE *File_for_Event;

t_boolean      load_interactive        = FALSE;
t_boolean      seek_info               = FALSE;
t_boolean      sorted_files            = FALSE;
t_boolean      paraver_priorities      = FALSE;
t_boolean      paraver_different_waits = TRUE;
dimemas_timer  start_paraver;
dimemas_timer  stop_paraver;
t_boolean      paraver_initial_timer = FALSE;
t_boolean      paraver_final_timer   = FALSE;

/* Tracefile detectitons */
t_boolean      binary_files = FALSE; /* Currently unused */
t_boolean      gzip_files   = FALSE; /* Compressed traces */
t_boolean      new_trace_format = FALSE;  /* New dimemas trace format */


static t_boolean HOT_cache = FALSE;

char            message_buffer[1024];

dimemas_timer   time_limit;

int event_to_monitorize =0;
t_boolean monitorize_event = FALSE;


int             sintetic_io_applications = 0;

struct t_queue CP_NODES;
struct t_queue *CP_nodes = &CP_NODES;

t_boolean NEW_MODIFICATIONS = FALSE;

int  RD_SYNC_message_size;   /* Minimum message size to use Rendez-vous
                              * <0 = Asynchronous unless specified: */
int  RD_SYNC_use_trace_sync; /* Use the synchronous field of the sends
                              * in the trace? */

/* Function to reserve pointers to mandatory files */
int    reserved_pointers_count = 0;
FILE**   reserved_pointers;
/* char** reserved_names; */

void
reserve_pointers(int num_reserves)
{
  int  i;
  char tempnam[13];
  
  srandom((unsigned int) time(NULL));
  
  reserved_pointers = (FILE*) mallocame(num_reserves*sizeof(FILE*));
  /* reserved_names    = (char*) mallocame(num_reserves*sizeof(char*)); */
  if (reserved_pointers == NULL)
    panic("Error reserving mandatory file descriptors memory\n");

  for(i = 0; i < num_reserves; i++)
  {
    /* Now 'tmpfile' function is used */
    /*
    reserved_names[i] = (char*) mallocame(13*sizeof(char*));
    if (reserved_names[i] == NULL)
      panic("Error reserving mandatory file descriptors memory\n");
    */
    
    
    /* sprintf(reserved_names[i], "dmmtmp%06d", random()%999999); 
    if ((reserved_pointers[i] = 
         MYOPEN(reserved_names[i],  O_CREAT|O_WRONLY|O_TRUNC)) == -1)
    {
    */
    if ( (reserved_pointers[i] = tmpfile()) == NULL)
      panic("Error reserving mandatory file descriptors %s\n",
            strerror(errno));
  }
  
  reserved_pointers_count = num_reserves;
}

void
free_reserved_pointer(void)
{
  if (reserved_pointers_count == 0)
    panic("No more pointers to free\n");
  
  reserved_pointers_count--;
  
  if (fclose(reserved_pointers[reserved_pointers_count]) == -1)
    panic("Error using reserved file pointer (close)\n");
  
  /* Temporal filenames not needed
  if (remove(reserved_names[reserved_pointers_count]) == -1)
    panic("Error using reserved file pointer (unlink)\n");
  
  freeame(reserved_names[reserved_pointers_count], 13*sizeof(char*));
  */
}

void
free_all_reserved_pointers(void)
{
  while(reserved_pointers_count != 0)
  {
    free_reserved_pointer();
  }
}


/* JGG (06/02/2006): Function to parse special configuration file, used to fine
 * tuning */

void
parse_special_cfg(FILE* special_cfg)
{
  int i, size, cycle, time;
  char   units;
  
  { /* LIBRARY COPY LATENCY */
  i = fscanf (special_cfg, "copy:%d%c\n", &size, &units);
  if (i == -1)
  {
    printf(
      "WARNING: error parsing special configuration file (copy latency)\n");
  }
  
  if (i == 1)
  {
    if (size != 0)
    {
      DATA_COPY_enabled      = TRUE;
      DATA_COPY_message_size = size;
    }
  }
  else /* S'ha indicat les unitats */
  {
    if (size != 0)
    {
      DATA_COPY_enabled = TRUE;
      switch (units)
      {
        case 'k': /* La mida es en Kb */
          DATA_COPY_message_size = size<<10; 
          break;
  
        case 'm': /* La mida es en Mb */
          DATA_COPY_message_size = size<<20;
          break;
          
        case 'g': /* La mida es en Gb */
          DATA_COPY_message_size = size<<30;
          break;
          
        case 'b':
        default: /* La mida es en bytes */
          DATA_COPY_message_size = size;
          break;
      }
    }
  }
  }
  { /* ROUND TRIP TIME LATENCY */
  i = fscanf (special_cfg, "rtt:%d\n", &time);
  if (i <= 0 || i > 1)
  {
    printf(
      "WARNING: error parsing special configuration file (round trip time)\n");
  }
  else /* i == 1 */
  {
    if (time != 0)
    {
      RTT_enabled = TRUE;
      RTT_time    = (t_micro) (time * 1.0);
    }
  }
  }
  { /* PREEMPTIONS */
  i = fscanf (special_cfg, "preemp:%d:%d\n", &cycle, &time);
  
  if (i != 2)
  {
    printf(
      "WARNING: error parsing special configuration file (preemptions info)\n");
  }
  else
  {
    if (cycle != 0)
    {
      PREEMP_enabled = TRUE;
      PREEMP_cycle   = cycle;
      PREEMP_time    = (t_micro) (time*1.0);
    }
  }
  }
  if (debug)
  {
    printf("Special configuration file correctly open\n");
  }
}


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

   printf ("Dimemas v%d.%d \n", VERSION, SUBVERSION);
   printf ("Binary file  %s \n", name);
   printf ("Compiled  %s \n", DATE);
   printf ("Implemented CPU Scheduling policies:\n");
   while (SCH[i].name)
   {
      printf ("    %s\n", SCH[i].name);
      i++;
   }

   show_fs_version ();

/*
 * printf ("(c) DAC-UPC 1993-95\n"); printf ("Authors: Sergi Girona, Jesus
 * Labarta and Toni Cortes\n");
 */
}

static void
help_message(char *tname)
{
  printf ("Dimemas version %d.%d\n\n", VERSION, SUBVERSION);
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
  printf ("\t-xs\t\tEnable extra scheduling debug output\n");
  printf ("\t-xe\t\tEnable extra event manager debug output\n");
  printf ("\t-xp\t\tEnable extra Paraver trace generation debug output\n");
  printf ("\t-o[l] file\tSet output file (default: stdout) (l:long info) \n");
  printf ("\t-t\t\tShow only simulation time as output\n");
  printf ("\t-T time\t\tSet simulation stop time\n");
  printf ("\t-l\t\tEnable in-core operation\n");
  printf ("\t-C\t\tPerform critical path analysis\n");
  printf ("\t-A file\t\tGenerate Paragraph+ SDDF tracefile\n");
  printf ("\t-p file\t\tGenerate Paraver tracefile (ASCII)\n");
  printf ("\t-pa file\tGenerate Paraver ASCII tracefile\n");
  printf ("\t-pb file\tGenerate Paraver binary tracefile\n");
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


/*
void
show_all_information()
{
  double f,g;
  double ct;
  struct t_machine  *machine;


  FLOAT_TO_TIMER (final_statistical_time, ct);
  
  for (machine=(struct t_machine *)head_queue(&Machine_queue);
       machine!=MA_NIL;
       machine=(struct t_machine *)next_queue(&Machine_queue))
  {
    f = machine->network.total_time_in_queue/ ct;
    g = machine->network.utilization/ct;
  /*  fprintf (salida_datos,"Mean queue lenght for machine %d is: %le and utilization %le\n",machine->id,f,g);
  }
}
*/

/*
static void
cotilleo (char *comando)
{
  char            hostname[255];
  char            username[L_cuserid + 1];
  int             p[2];
  char            buffer[1024];
  int             i;
  char            hora[80];
  time_t          t;

  char           *date = DATE;

  cuserid (username);
  username[L_cuserid] = (char) 0;
  if (strcmp ("sergi", username) == 0)
  {
    show_all_information();
    return;
  }
}
*/

static dimemas_timer
read_timer(char *c)
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
    tmp_float = tmp_float * 1e6;
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
  t_boolean Pallas_output = TRUE;
  struct t_machine  *machine;
  FILE              *special_cfg;
  int               venusconn = 0;


  ASS_TIMER (current_time, 0);
  ASS_TIMER (final_statistical_time, 0);
  ASS_TIMER (time_limit, 0);

  #ifndef PACA
  ASS_TIMER (time_limit, TIME_LIMIT);	/* Simulation restricted to 8 hours */
  #endif                                /* PACA */

  /* Per defecte les comunicacions son asincrones independentment de la mida */
  RD_SYNC_message_size = -1;
  /* Per defecte les comunicacions poden ser sincrones si s'indica a la traca */
  RD_SYNC_use_trace_sync = TRUE;

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
  paraver_binary = FALSE;
  
  /* Load interactive is the default option */
  load_interactive = TRUE;
  sorted_files     = TRUE;

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
            stop_paraver = read_timer (argv[j]);
            paraver_final_timer = TRUE;
            break;
        case 'y':
            j++;
            start_paraver = read_timer (argv[j]);
            paraver_initial_timer = TRUE;
            break;
        case 'p':
          switch (argv[j][2])
          {
            case 0:
            case 'a':
                paraver_binary = FALSE;
                break;
            case 'b':
                paraver_binary = TRUE;
                break;
            default:
                fprintf (stderr, USAGE, argv[0]);
                exit (1);
          }
          j++;
          paraver_file = argv[j];
          break;
        /* -l now means 'in-core' */
        case 'l':
          load_interactive = FALSE;
          /* sorted_files     = TRUE; */
          break;
        case 'A':
          j++;
          sddf_filename = argv[j];
          break;
#ifdef VENUS_ENABLED
        case 'v':
          if (!strcmp(argv[j], "-venus")) {
            venus_enabled = 1;
            venusconn = "";
          }
          else if (!strcmp(argv[j], "-venusconn")) {
            venus_enabled = 1;
            j++;
            venusconn = strdup(argv[j]);
          }
          else {
            show_version (argv[0]);
            exit (0);
          }
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
          fichero_fs = argv[j];
          break;
        case 's':
          j++;
          fichero_sch = argv[j];
          break;
        case 'b':
          binary_files = TRUE;
          break;
        case 'k':
          seek_info = TRUE;
          break;
        case 'I':
          j++;
          sintetic_io_applications = atoi (argv[j]);
          if (sintetic_io_applications < 0)
            sintetic_io_applications = 0;
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
          if (reload_limit <= 0)
            reload_limit = 1;
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
          monitorize_event = TRUE;
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

  if (fichero_salida != (char*) 0)
  {
    salida_datos = MYFOPEN (fichero_salida, "w");
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

  if (paraver_file != (char *) 0)
  {
    Activar_Generacio_Paraver();
    if ((paraver_initial_timer) && (paraver_final_timer))
    {
      if LE_TIMER (stop_paraver, start_paraver)
      {
        fprintf (stderr, "Final paraver time must be greater than initial\n");
        fprintf (stderr, USAGE, argv[0]);
        exit (1);
      }
    }
  }
  
  if ((paraver_file == (char*)0))
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
    if (file_for_event_to_monitorize!=(char *)0)
    {
      File_for_Event = MYFOPEN (file_for_event_to_monitorize, "w");
      if (File_for_Event == (FILE *) 0)
      {
        fprintf (stderr, USAGE, argv[0]);
        fprintf (stderr,
                "Can't open %s for Event monitor\n",
                file_for_event_to_monitorize);
        exit (1);
      }
    }
    else
      File_for_Event = stdout;
  }
   
  if (j>=argc)
  {
    fprintf (stderr, USAGE, argv[0]);
    exit (1);
  }

  config_file = argv[j];
   
  /* JGG (06/02/2006): special configuration file open */
  if ((special_cfg = MYFOPEN("./special.cfg", "r")) != NULL)
  {
    parse_special_cfg(special_cfg);
  }
  else
  {
    if (debug)
    {
      printf("Special configuration file not present\n");
    }
  }
  
  /* Initial routines */
  
  /* Pointer reservation */
  reserve_pointers(1);
  
  malloc_init ();

  /* srandomine (time (0), time (0) + time (0)); */
  srandomine (16994, 18996);
  srandomine (time (0), time (0) + time (0));

  /* Modules Initial routines */
  init_simul_char ();
  create_queue (&Node_queue);
  create_queue (&Port_queue);
  create_queue (&Global_op);
  
  if (debug)
  {
    PRINT_TIMER (current_time);
    printf (": Simulator config to be read %s\n", config_file);
  }

  /* Configuration file (target machine description and application mapping)
   * parsing */
  load_configuration_file (config_file);

  if (debug)
  {
    PRINT_TIMER (current_time);
    printf (": Reading trace files\n");
  }

  RANDOM_init (fichero_random);
  

  new_trace_format = is_it_new_format();

  if(!new_trace_format)
	Read_Ptasks();
  else  
  	Ptasks_init();


  if (Critical_Path_Analysis)
  {
    create_queue (CP_nodes);
    /* Critical path only allowed if a single task is used */
    if (count_queue(&Ptask_queue)!=1)
    {
      printf("Warning: Critical Path analysis not allowed with %d Ptask\n",
             count_queue(&Ptask_queue));
      Critical_Path_Analysis = FALSE;
    }
  }

  if (debug)
  {
    PRINT_TIMER (current_time);
    printf (": Simulator config file readed %s\n", config_file);
  }

  check_full_nodes ();

  PORT_init ();


  seek_info = FALSE;

  if (sintetic_io_applications)
  {
    create_sintetic_applications (sintetic_io_applications);
  }


#ifdef VENUS_ENABLED
  if (venus_enabled) {
    int rc = 1;
    rc = vc_initialize(venusconn); /* VENUS CLIENT */
    if (!rc) {
            fprintf(stderr, "Could not connect to Venus, exiting\n");
            return -100;
    }
  }
#endif


  EVENT_init ();
  SCHEDULER_init (fichero_sch);

  /* S'inicialitza l'scheduler de cada maquina */     
  for(machine  = (struct t_machine *)head_queue(&Machine_queue);
      machine != MA_NIL;
      machine  = (struct t_machine *)next_queue(&Machine_queue))
  {
    (*SCH[machine->scheduler.policy].scheduler_init) (fichero_sch, machine);
  }
  
  COMMUNIC_init (fichero_comm);
  MEMORY_init ();
  SEMAPHORE_init ();
  FS_init (fichero_fs, HOT_cache);

  Give_number_to_CPU();

  /* Paraver trace initialization if needed */
  Paraver_init();

  reload_events ();

  if (debug)
  {
    printf ("\n");
    PRINT_TIMER (current_time);
    printf (": START SIMULATION\n\n");
  }

  ASS_ALL_TIMER (to_parent, current_time);

#ifdef VENUS_ENABLED
  while ((top_event (&Event_queue) != E_NIL) || (venus_enabled && (top_event(&Interactive_event_queue) != E_NIL)))
#else
while (top_event (&Event_queue) != E_NIL)
#endif
  {
    /*
    if ((NEQ_0_TIMER (time_limit)) && GT_TIMER (current_time, time_limit))
    {
    */
    if (OUT_OF_LIMIT(current_time))
    {
      /*
      if (debug)
      {
      */
      printf ("\n");
      PRINT_TIMER (current_time);
      printf (": END SIMULATION due time limit\n\n");
      /*
      }
      */
      break;
    }
    event_manager (outFIFO_event (&Event_queue));
  }

  if (debug)
  {
    printf ("\n");
    PRINT_TIMER (current_time);
    printf (": END SIMULATION\n\n");
  }

  /* Modules final routines */
  TASK_end ();
  FS_end ();
  PORT_end ();
  EVENT_end ();
  SCHEDULER_end ();
  COMMUNIC_end ();
  MEMORY_end ();
  SEMAPHORE_end ();

  SDDF_do ();


#ifdef VENUS_ENABLED
  if (venus_enabled) {
    vc_finish();
  }
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

  Paraver_fini();

  strcpy (message_buffer, "");
  for (i = 0; i < argc; i++)
  {
    strcat (message_buffer, argv[i]);
    strcat (message_buffer, " ");
  }

  /* Free reserved pointers */
  free_all_reserved_pointers();
  
  malloc_end();
  fclose (salida_datos);

  /* Change output tabs per spaces */
  if (fichero_salida != (char *)0)
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
  
  exit(0);
}
