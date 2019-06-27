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

#include <cmath>
#include <ctime>
#include <cerrno>
#include <csignal>
#include <cfloat>
#include <assert.h>
#include <iostream>
#include <boost/program_options.hpp>
#include <boost/program_options/positional_options.hpp>

extern "C"
{
#include <execinfo.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <deadlock_analysis.h>
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
#include "memory.h"
#include "paraver.h"
#include "ports.h"
#include "read.h"
#include "schedule.h"
#include "semaphore.h"
#include "task.h"
#include "random.h"
#include "eee_configuration.h"
#include "dimemas_io.h"
#include "new_configuration.h"
#include "configuration.h"
#include "simulator.h"

#ifdef USE_EQUEUE
    #include "listE.h"
#endif

#ifdef VENUS_ENABLED
    #include "listE.h"
    #include "venusclient.h"
#endif
}

// Not exposed functions
static dimemas_timer read_timer(char *c);
static long long int read_size(const char *c);
double ddiff(struct timespec start, struct timespec end);

int port_ids = MIN_PORT_NUMBER;

// Variables from params

bool critical_path_enabled;
const char *parameter_tracefile;

double parameter_bw = DBL_MIN;
double parameter_lat = DBL_MIN;

int parameter_predefined_map = MAP_NO_PREDEFINED;
int parameter_tasks_per_node;

const char *config_file;
int debug = 0;

int reload_limit = 10;
t_boolean reload_Ptasks = FALSE;
t_boolean reload_done = FALSE;
t_boolean reload_while_longest_running = FALSE;
t_boolean full_out_info = FALSE;
t_boolean short_out_info = FALSE; 
t_boolean wait_logical_recv = FALSE;

FILE *salida_datos;
const char *fichero_salida     = NULL;
const char *paraver_file       = NULL;
const char *paraver_pcf_insert = NULL;

const char *file_for_event_to_monitorize = (char *)0;
FILE *File_for_Event;

static dimemas_timer paraver_start = -DBL_MAX;
static dimemas_timer paraver_end = -DBL_MAX;
static t_boolean paraver_priorities;

dimemas_timer time_limit;
int sintetic_io_applications = 0;

struct t_queue CP_NODES;
struct t_queue *CP_nodes = &CP_NODES;

long long int  RD_SYNC_message_size;
int  RD_SYNC_use_trace_sync; 

int with_deadlock_analysis = 0;
int reboots_counter = 0;
float danalysis_deactivation_percent = 1;
int asynch_read = FALSE;
int asynch_buffer_size_mb = 10;
t_boolean simulation_rebooted = FALSE;

t_boolean extra_assert=FALSE;
t_boolean Critical_Path_Analysis = FALSE;

using namespace std;

void print_dimemas_header()
{
    cout << endl;
    cout << "Dimemas - DIstributed MEmory MAchine Simulator" << endl;
    cout << "Barcelona Supercomputing Center - Centro Nacional de"\
        " Supercomputacion" << endl;
    cout << endl;
}

static long long int read_size(const char *c)
{
    int i;
    long long int mida_tmp, mida;
    char unitats;

    i = sscanf (c, "%lld%c", &mida_tmp, &unitats);
    if (i == 0)
    {
        fprintf(stderr,"Incorrect minimum message size"\
                " to use Rendez vous!\n");
        exit(EXIT_FAILURE);
    }
    else if (i==1)
        mida = mida_tmp; 
    else 
    {
        switch (unitats)
        {
            case 'k': 
            case 'K':
                mida=mida_tmp<<10;
                break;
            case 'm': 
            case 'M':
                mida=mida_tmp<<20;
                break;
            case 'g': 
            case 'G':
                mida=mida_tmp<<30;
                break;
            case 'b':
            case 'B':
            default: 
                mida=mida_tmp;
                break;
        }
    }
    return(mida);
}

void parse_arguments(int argc, char *argv[])
{
#ifdef VENUS_ENABLED
    string venus_conn_url;
#endif

    string fine_conf_fs;
    string fine_conf_sched;
    string fine_conf_comm;
    string fine_conf_rand;
    string event_to_monitorize;
    string eager_limit;

    string str_parameter_tracefile;
    string str_eee_config_file;
    string str_paraver_file;
    string str_config_file;
    string str_fichero_salida;
    string str_paraver_pcf_insert;
    string str_file_for_event_to_monitorize;

    bool paraver_priorities_enabled;
    bool debug_enabled;
    bool extra_asserts_enabled;
    bool extra_sched_debug_enabled;
    bool extra_paraver_debug_enabled;
    bool extra_task_debug_enabled = false;
    bool extra_event_debug_enabled;
    bool venus_enabled;
    bool critical_path_enabled;
    bool asynch_sends_enabled;
    bool b_short_out_info;
    bool map_fill_enabled;
    bool map_interleaved_enabled;
    bool wait_logical_recv_enabled;
    bool b_eee_enabled;
    bool asynch_read_bool;

    int sintetic_io_applications;

    namespace po = boost::program_options;

    po::options_description general("Miscellany options");
    general.add_options()
        ("help,h", "Show this help message")
        ("version,v", "Show the version")
    ;

    po::options_description conf("Configuration options: This options are going to"\
            " rewrite the options defined in the configuration file");
    conf.add_options()
        ("asynch-sends,F", po::bool_switch(&asynch_sends_enabled),
            "All sends are asynchronous")
        ("eager-limit,S", po::value<string>(&eager_limit),
            "Eager limit is set to this value")
        ("sched,s", po::value<string>(&fine_conf_sched), 
            "Scheduler definition filename")
        ("fs,f", po::value<string>(&fine_conf_fs), 
            "File-system definition filename")
        ("comms,c", po::value<string>(&fine_conf_comm), 
            "Communications fine-tunning definition filename")
        ("random,r", po::value<string>(&fine_conf_rand), 
            "Random definition filename")
        ("bandwidth,bw", po::value<double>(&parameter_bw), 
            "Bandwidth for the modeled machine")
        ("latency,lat", po::value<double>(&parameter_lat), 
            "Latency for the modeled machine")
        ("fill-nodes,fill", po::bool_switch(&map_fill_enabled),
            "Mapping policy where all processors in nodes are filled up")
        ("process-per-node", po::value<int>(&parameter_tasks_per_node), 
            "Processes per node for the modeled machine")
        ("interleaved", po::bool_switch(&map_interleaved_enabled),
            "Mapping policy where the processes are mapped"\
            " in an interleaved fashion")
        ("dim", po::value<string>(&str_parameter_tracefile), 
            "Dimemas input tracefile")
    ;

    po::options_description simulation("Simulation options: This options"\
            " are going to affect the simulation but not the model");
    simulation.add_options()
        ("prv-start-time,y", po::value<dimemas_timer>(&paraver_start),
            "Set paraver tracefile start time")
        ("prv-end-time,z", po::value<dimemas_timer>(&paraver_end),
            "Set paraver tracefile stop time")
        ("prv-priorities,P", po::bool_switch(&paraver_priorities_enabled), 
            "Paraver priorities")
        ("stop-time,T", po::value<dimemas_timer>(&time_limit), 
            "Set the simulation stop time")
        ("critical-path-analysis,C", po::bool_switch(&critical_path_enabled),
            "Performs critical path analysis")
        ("logic-irecv-at-wait,w", po::bool_switch(&wait_logical_recv_enabled),
            "Logic receive of Irecv when MPI_Wait take place")
        ("synthetic-io-app,I", po::value<int>(&sintetic_io_applications),
            "Adds synthetic applications with I/O workloads")
        ("reload,R", po::value<int>(&reload_limit),
            "Reload simulation the indicated times")
        /*
        ("eee-enable", po::bool_switch(&b_eee_enabled),
            "Enable EEE network model")
        ("eee-network", po::value<string>(&str_eee_config_file), 
            "EEE network definition filename")
        ("eee-framesize", po::value<int>(&eee_frame_header_size), 
            "EEE network frame size")
        */
        ("clean-deadlocks", po::value<float>(&danalysis_deactivation_percent), 
            "Try to recover from deadlocks if any")
#ifdef VENUS_ENABLED
        ("venus", po::bool_switch(&venus_enabled),
            "Enable venus (default conn localhost:default_port)")
        ("venuscon", po::value<string>(&venus_conn_url)->default_value(NULL), 
            "Connect to venus server at host:port")
#endif
        ("asynch-read", po::bool_switch(&asynch_read_bool), 
            "Wakes up a new thread for read the input trace")
        ("asynch-max-buffer", po::value<int>(&asynch_buffer_size_mb), 
            "Max size of the asynch read buffer in MB (default: 10MB)")
    ;

    po::options_description mandatory("Mandatory options");
    mandatory.add_options()
       // ("dim", po::value<string>(&str_parameter_tracefile)->required(), 
         //   "Dimemas Input Trace File")
        //("prv-trace,p", po::value<string>(&str_paraver_file),//->required(), 
        //    "Generated paraver trace(OPTIONAL)")
        ("config-file", po::value<string>(&str_config_file)->required(), 
            "Dimemas configuration file")
    ;

    po::options_description optional("Optional options :"\
    " To generate simulated paraver trace file");
    optional.add_options()
        ("prv-trace,p", po::value<string>(&str_paraver_file), 
            "Generate simulated paraver trace file")
    ;
    po::options_description output("Output options");
    output.add_options()
        //("output,o", po::value<string>(&str_fichero_salida), "Set output file")
        ("only-time,t", po::bool_switch(&b_short_out_info),
            "Shows just timing information as output")
        ("monitorize-event,e", po::value<string>(&event_to_monitorize),
            "Show time distance between event occurrences")
        ("monitorize-event-output,g", po::value<string>(&str_file_for_event_to_monitorize),
            "File for output info about event monitoring")
        ("pcf-file", po::value<string>(&str_paraver_pcf_insert),
            "Set the pcf output file")
    ;

    po::options_description debug_args("Debug options");
    debug_args.add_options()
        ("debug,d", po::bool_switch(&debug_enabled), 
            "Show debug information during the simulation")
        ("xtra-asserts", po::bool_switch(&extra_asserts_enabled),
            "Extra assertations")
        ("xtra-sched-debug", po::bool_switch(&extra_sched_debug_enabled),
            "Extra scheduler debug information")
        ("xtra-event-debug", po::bool_switch(&extra_event_debug_enabled),
            "Extra events debug information")
        ("xtra-paraver-debug", po::bool_switch(&extra_paraver_debug_enabled),
            "Extra paraver tracefile generation debug information")
        //("xtra-task-debug", po::bool_switch(&extra_task_debug_enabled),
        //    "Extra task debug information")
    ;

    po::options_description all("Allowed options");
    all.add(mandatory)
        .add(optional)
        .add(conf)
        .add(simulation)
        .add(output)
        .add(debug_args)
        .add(general);

    po::positional_options_description pd;
    pd.add("config-file", 1);

    po::variables_map varmap;
   try{
    po::store(po::command_line_parser(argc, argv)
            .options(all)
            .positional(pd)
            .run(), varmap);
   }

   catch(...)
   {
        cout << "Error in the arguments. Please check it again." << endl;
        cout << "To see help, use -h flag" << endl;
        cout<< endl;
        cout << "USAGE: " << argv[0] << 
            " --dim [--dim-trace] ARG -p [--prv-trace] ARG [--config-file] CONFIG" << endl;
        exit(EXIT_FAILURE);
    }
    if (varmap.count("help"))
    {
        print_dimemas_header();
        cout << "USAGE: " << argv[0] << 
            " --dim [--dim-trace] ARG -p [--prv-trace] ARG [--config-file] CONFIG" << endl;
            //" -p [--prv-trace] ARG [--config-file] CONFIG" << endl;
        cout << endl;
        cout << all << endl;
        exit(EXIT_SUCCESS);
    }
    else if(varmap.count("version"))
    {
        cout << "Dimemas"  << " "  VERSION << " (" << DATE << ")"<< endl;
        exit(EXIT_SUCCESS);
    }

    try
    {
        po::notify(varmap);
    }
    catch(boost::program_options::required_option& e)
    {
        cout << "Error parsing arguments" << endl;
        cout << endl;
        //cout << "plase specify the dim trace file " << endl;
        cout<< "USAGE: " << argv[0] <<
            " --dim [--dim-trace] ARG -p [--prv-trace] ARG [--config-file] CONFIG" << endl;
        cout << endl;
        cout << e.what() << endl;
        exit(EXIT_FAILURE);
    }

    // Parameters extra treatment
    // Needed because the kernel is c and can not use C++ datatypes
    //
    
    if (varmap.count("dim"))
    {
        parameter_tracefile = (char *)malloc(str_parameter_tracefile.size()+1);
        memcpy( (void *)parameter_tracefile, 
            (const void *)str_parameter_tracefile.c_str(),
            str_parameter_tracefile.size()+1);
    }

    if (varmap.count("eee-network"))
    {
        eee_config_file = (char *)malloc(str_eee_config_file.size()+1);
        memcpy( (void *)eee_config_file, 
            (const void *)str_eee_config_file.c_str(),
            str_eee_config_file.size()+1);
    }

    if (varmap.count("output"))
    {
        fichero_salida = (char *)malloc(str_fichero_salida.size()+1);
        memcpy( (void *)fichero_salida, 
            (const void *)str_fichero_salida.c_str(),
            str_fichero_salida.size()+1);
    }

    if (varmap.count("pcf-file"))
    {
        paraver_pcf_insert = (char *)malloc(str_paraver_pcf_insert.size()+1);
        memcpy( (void *)paraver_pcf_insert, 
            (const void *)str_paraver_pcf_insert.c_str(),
            str_paraver_pcf_insert.size()+1);
    }

    if (varmap.count("monitorize-event-output"))
    {
        file_for_event_to_monitorize = 
            (char *)malloc(str_file_for_event_to_monitorize.size()+1);
        memcpy( (void *)file_for_event_to_monitorize, 
            (const void *)str_file_for_event_to_monitorize.c_str(),
            str_file_for_event_to_monitorize.size()+1);
    }

    config_file = (char *)malloc(str_config_file.size()+1);
    memcpy( (void *)config_file, 
            (const void *)str_config_file.c_str(),
            str_config_file.size()+1);
    paraver_file = (char *)malloc(str_paraver_file.size()+1);
    memcpy( (void *)paraver_file, 
            (const void *)str_paraver_file.c_str(),
            str_paraver_file.size()+1);

    if (debug_enabled)
        debug = D_LINKS|D_COMM;
    if (extra_sched_debug_enabled)
        debug |= D_SCH;
    if (extra_paraver_debug_enabled)
        debug |= D_PRV;
    if (extra_task_debug_enabled)
        debug |= D_TASK;
    if (extra_event_debug_enabled)
        debug |= D_EV;

    if (asynch_read_bool)
        asynch_read = TRUE;

    paraver_final_timer = (varmap.count("prv-stop-time") != 0);
    paraver_initial_timer = (varmap.count("prv-start-time") != 0);

#if VENUS_ENABLED
    if (venus_enabled)
        VC_enable(venus_conn_url.c_str());
#endif
    if (varmap.count("sched"))
        CONFIGURATION_Set_Scheduling_Configuration_File(fine_conf_sched.c_str());
    if (varmap.count("fs"))
        CONFIGURATION_Set_FileSystem_Configuration_File(fine_conf_fs.c_str());
    if (varmap.count("comm"))
        CONFIGURATION_Set_Communications_Configuration_File(fine_conf_comm.c_str());
    if (varmap.count("random"))
        CONFIGURATION_Set_RandomValues_Configuration_File(fine_conf_rand.c_str());

    if (varmap.count("reload"))
        if (reload_limit == 0)
        {
            reload_limit = MAX_RELOAD_LIMIT;
            reload_while_longest_running = TRUE;
        }

    if (asynch_sends_enabled)
        RD_SYNC_use_trace_sync = FALSE;
    else if (varmap.count("eager-limit"))
    {
        RD_SYNC_use_trace_sync = FALSE;
        RD_SYNC_message_size = read_size(eager_limit.c_str());
    }

    if (varmap.count("clean-deadlocks"))
        with_deadlock_analysis = TRUE;

    if (varmap.count("process-per-node"))
        parameter_predefined_map = MAP_N_TASKS_PER_NODE;
    else if (map_fill_enabled)
        parameter_predefined_map = MAP_FILL_NODES;
    else if (map_interleaved_enabled)
        parameter_predefined_map = MAP_INTERLEAVED;

    if (paraver_priorities_enabled)
        paraver_priorities = TRUE;
    else
        paraver_priorities = FALSE;

    if (wait_logical_recv_enabled)
        wait_logical_recv = TRUE;
    else
        wait_logical_recv = FALSE;

    if (b_eee_enabled)
        eee_enabled = TRUE;
    else
        eee_enabled = FALSE;

    if (extra_asserts_enabled)
        extra_assert = TRUE;
    else
        extra_assert = FALSE;

    if (b_short_out_info)
        short_out_info = TRUE;
    else
        short_out_info = FALSE;
}

double ddiff(struct timespec start, struct timespec end)
{
  return ( (double)(end.tv_sec-start.tv_sec) 
          + (double)1e-9*(end.tv_nsec-start.tv_nsec) );
}

static dimemas_timer read_timer(char *c)
{
    int h, u, i;
    float tmp_float;
    dimemas_timer tmp_timer;;

    i = sscanf (c, "%dH%du", &h, &u);
    if (i == 2)
        HOUR_MICRO_TO_TIMER (h, u, tmp_timer);
    else
    {
        tmp_float = (float) atof (c);
        tmp_float = tmp_float * 1e9;
        FLOAT_TO_TIMER (tmp_float, tmp_timer);
    }
    return (tmp_timer);
}

int main (int argc, char *argv[])
{
    IO_Init();
    ASS_TIMER (current_time, 0);
    ASS_TIMER (final_statistical_time, 0);
    ASS_TIMER (time_limit, TIME_LIMIT); 

    RD_SYNC_message_size = -1;
    RD_SYNC_use_trace_sync = TRUE;

    parse_arguments(argc, argv);

    PARAVER_Init(paraver_file, paraver_pcf_insert, paraver_start,
        paraver_end, paraver_priorities);

    if (fichero_salida != (char*) 0)
    {
        salida_datos = IO_fopen (fichero_salida, "w");
        if (salida_datos == (FILE *) 0)
        {
            printf ("Can't open %s for Output\n", fichero_salida);
            exit (EXIT_FAILURE);
        }
    }
    else
        salida_datos = stdout;

    if (monitorize_event)
    {
        if (file_for_event_to_monitorize != NULL )
        {
            File_for_Event = IO_fopen (file_for_event_to_monitorize, "w");
            if (File_for_Event == NULL)
            {
                fprintf (stderr, "Can't open %s for Event monitor\n",
                    file_for_event_to_monitorize);
                exit (EXIT_FAILURE);
            }
        }
        else
            File_for_Event = stdout;
    }

    SIMULATOR_Init(config_file,
            parameter_tracefile,
            parameter_bw,
            parameter_lat,
            parameter_predefined_map,
            parameter_tasks_per_node);
    RANDOM_Init();
    TASK_Init(sintetic_io_applications);
    PORT_Init();
    EEE_Init();
    EVENT_Init ();
    SCHEDULER_Init ();
    COMMUNIC_Init (parameter_tracefile, danalysis_deactivation_percent);
    MEMORY_Init ();
    SEMAPHORE_Init ();
    FS_Init();
    CPU_Get_Unique_CPU_IDs();
#ifdef VENUS_ENABLED
     VC_Init(); /* VENUS CLIENT */
#endif

    if (critical_path_enabled)
    {
        Critical_Path_Analysis = TRUE;
        create_queue (CP_nodes);
        if (count_queue(&Ptask_queue) != 1)
        {
            printf("Warning: Critical Path analysis not"\
                    " allowed with %d Ptask\n", count_queue(&Ptask_queue));
            Critical_Path_Analysis = FALSE;
        }
    }
    else
        Critical_Path_Analysis = FALSE;

    
    reload_events();

    printf ("\n");
    PRINT_TIMER (current_time);
    printf (": START SIMULATION\n\n");

REBOOT:

#ifdef VENUS_ENABLED
    while ((top_event (&Event_queue) != E_NIL) || (VC_is_enabled() 
                && (top_event(&Interactive_event_queue) != E_NIL)) 
            && !simulation_rebooted)
#else 
    while (top_event (&Event_queue) != E_NIL && !simulation_rebooted)
#endif
    {
        struct t_event* current_event;
        if (OUT_OF_LIMIT(current_time))
        {
            PRINT_TIMER (current_time);
            printf (": END SIMULATION due time limit\n\n");
            break;
        }

        current_event = outFIFO_event(&Event_queue);
        event_manager(current_event);
    }

    if (with_deadlock_analysis)
        if (simulation_rebooted || DEADLOCK_check_end())
        {
            // this events must be freed
            remove_queue_elements(&Event_queue);

            SIMULATOR_reset_state();
            COMMUNIC_reset_deadlock();

            reboots_counter++;

            // Ends the actual erroneous paraver trace
            // Starts new paraver trace with ".undeadlocked."
            PARAVER_End(FALSE);
            PARAVER_Init(paraver_file,
                 paraver_pcf_insert,
                 paraver_start,
                 paraver_end,
                 paraver_priorities);

            // This call load the events that threads have in actions
            // for this reason, before this we have to read the new actions
            reload_events();

            simulation_rebooted = FALSE;
            goto REBOOT;
        }

    if (reboots_counter > 0)
    {
        printf("\n**** Deadlocks ****\n\n");
        printf("%d deadlocks has been successfully cleaned.\n", 
                reboots_counter);
        printf("\n");
    }

    // Finalizing simulation
    if (!short_out_info)
    {
        PRINT_TIMER (current_time);
        printf (": END SIMULATION\n\n");
    }

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

    calculate_execution_end_time();
    if (critical_path_enabled == FALSE)
        show_statistics();
    else
    {
        fprintf (salida_datos,"Execution time:\t");
        FPRINT_TIMER (salida_datos, execution_end_time);
        fprintf (salida_datos, "\n\n");
        show_CP_graph();
    }

    PARAVER_End(TRUE);


    std::string  row_file(paraver_file);
    row_file.replace(row_file.end()-4, row_file.end(), ".row");
    SIMULATOR_Generate_row(row_file.c_str());
    TASK_End();

    struct rusage usage;
    if (debug)
    {
        if (getrusage(RUSAGE_SELF, &usage) == -1)
            printf("Unable to get memory usage statistics\n");
        else
            printf("Maximum memory used: %ld\n", usage.ru_maxrss);
    }

    char message_buffer[1024];
    strcpy (message_buffer, "");

    int i;
    for (i = 0; i < argc; i++)
    {
        strcat (message_buffer, argv[i]);
        strcat (message_buffer, " ");
    }

    IO_fclose (salida_datos);

    if (fichero_salida != NULL)
    {
        const char *c = "Dimemas_OUTPUT.tmp";
        rename (fichero_salida, c);

        close (0);
        MYOPEN (c, O_RDONLY);
        unlink (c);
        close (1);
        MYOPEN (fichero_salida, O_WRONLY| O_TRUNC| O_CREAT,0600);
        execlp ("col", "dimemas_col", "-x", (char *)0);
    }
    return 0;
}
