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

#define _GNU_SOURCE // for getline
#include <stdio.h>

/* JGG: to use 'USE_RENDEZ_VOUS' macro */
#include <define.h>
#include <types.h>
#include <extern.h>

#include <sys/stat.h>
#include <assert.h>

#include <string.h>
#include <errno.h>

#include "file_data_access.h"
#include <dimemas_io.h>
#include <list.h>

// Definition and configuartion parameters of data access api
#define DATA_ACCESS_DIMEMAS_HELLO_SIGN "#DIMEMAS"

// Configures maximim number of applications supported by API
#define DATA_ACCESS_MAX_NUM_APP 100

// Configures maximum error message length
#define DATA_ACCESS_MAX_ERROR_LENGTH 256

// Thereshold value that deterimnates sequential or advanced metrhod for offsets search
#define DATA_ACCESS_SEEK_THRESHOLD 20*1024*1024


#define DEFINITION d
#define DEFINITION_REGEXP "d:%d:%[^\n]\n" // d:object_type:{specific_fields}

#define OFFSET        s
#define OFFSET_REGEXP "s:%d:%[^\n]\n"  // s:task_id:offset_thread_1:(...):offset_thread_n

#define OFFSET_NOT_PRESENT 0
#define OFFSET_PRESENT     1

#define DEF_COMMUNICATOR 1
#define COMMUNICATOR_REGEXP "%d:%d:%s" // comm_id:tasks_count:task_id_1:(...):task_id_n
#define DEF_FILE         2
#define DEF_ONESIDED_WIN 3

#define ACTION_REGEXP    "%d:%d:%d:%[^\n]\n" // op_id:task_id:thread_id:{specific_fields}

#define RECORD_NOOP      0
#define NOOP_REGEXP      "0:%d:%d\n"

#define RECORD_CPU_BURST 1
#define CPU_BURST_REGEXP "%lf" // burst_duration

#define RECORD_MSG_SEND  2
#define MSG_SEND_REGEXP_MPI  "%d:%ld:%d:%d:%d"     // dest_task_id:msg_size:tag:comm_id:synchronism
#define MSG_SEND_REGEXP_SS   "%d:%d:%ld:%d:%d:%d"  // dest_task_id:dest_thread_id:msg_size:tag:comm_id:synchronism

#define RECORD_MSG_RECV  3
#define MSG_RECV_REGEXP_MPI "%d:%ld:%d:%d:%d"     // src_task_id:msg_size:tag:comm_id:recv_type
#define MSG_RECV_REGEXP_SS  "%d:%d:%ld:%d:%d:%d"  // ssrc_task_id:src_thread_id:msg_size:tag:comm_id:recv_type

#define RECORD_GLOBAL_OP 10
#define GLOBAL_OP_REGEXP    "%d:%d:%d:%d:%ld:%ld" // global_op_id:comm_id:root_task:root_th:bytes_send:bytes_recv

#define RECORD_EVENT     20
#define EVENT_REGEXP     "%ld:%ld" // event_type:event_value

#define RECVTYPE_RECV  0
#define RECVTYPE_IRECV 1
#define RECVTYPE_WAIT  2

#define DATA_ACCESS_ERROR -1
#define DATA_ACCESS_OK     1

/*****************************************************************************
 * Internal structures
 ****************************************************************************/

/**
 * 'fp_share' handles the FILE* shared for each application and stores the
 * 'task_id' and 'thread_id' of the last access so as to avoid useless seeks
 */
typedef struct _fp_share
{

  count_t  last_task_id;
  count_t  last_thread_id;
  FILE *fp;

} fp_share;

/**
 * 'app_struct'  to store basic information about applications (ptasks), as
 * well as data for efficient data extraction from trace file and resource
 * sharing
 */
typedef struct _app_struct
{
  int             ptask_id;
  char           *app_name;
  char           *trace_file_name;
  count_t         tasks_count;
  count_t        *threads_count;
  count_t         total_threads_count;
  int             offset_type;
  off_t           offsets_offset;
  off_t           records_offset;
  off_t         **threads_offsets;

  count_t         streams_count;
  fp_share       *streams;
  size_t        **streams_idxs; // Index of the 'fp_share' assigned to each thread
  off_t         **current_threads_offsets;

  count_t         comms_count;
  struct t_queue  comms;

} app_struct;

/**
 * 'main_struct', main container to keep track of the DATA_ACCESS_API state
 */
struct data_access_layer_struct
{
  int          num_apps;
  app_struct **apps;

  char         error_string[DATA_ACCESS_MAX_ERROR_LENGTH];

  FILE*        current_stream;

  t_boolean    init_flag;
  t_boolean    io_init;

} main_struct;


/*****************************************************************************
 * Private functions declaration
 ****************************************************************************/
void DAP_report_error (const char *format, ...);

t_boolean   DAP_init_data_access_layer (void);

app_struct* DAP_locate_app_struct (int ptask_id);

t_boolean DAP_add_ptask         (int ptask_id, char *trace_file_name, int index);

void      DAP_end_ptask (app_struct *app);

t_boolean DAP_initialize_app (app_struct *app,
                              int         ptask_id,
                              char       *app_name,
                              count_t     tasks_count,
                              count_t    *threads_count,
                              count_t     comms_count,
                              off_t       offsets_offset);

/*
 * Descriptive sections operations
 */

t_boolean DAP_read_header       (app_struct *ptask, int ptask_id);

t_boolean DAP_read_definitions  (app_struct *app);
t_boolean DAP_read_communicator (app_struct *app, const char *comm_fields);

t_boolean DAP_read_offsets         (app_struct *app);
t_boolean DAP_locate_offsets       (app_struct *app);
off_t     DAP_locate_thread_offset (app_struct *app,
                                    int         task_id,
                                    int         thread_id,
                                    off_t       lower_bound,
                                    off_t       upper_bound);



/*
 * File descriptors sharing
 */
t_boolean DAP_io_init(void);
t_boolean DAP_allocate_streams(app_struct *app, size_t assigned_streams);

/*
 * Operation records parsing
 */
t_boolean DAP_read_action (app_struct       *app,
                           int               task_id,
                           int               thread_id,
                           struct t_action **action);

t_boolean DAP_read_CPU_burst (const char      *cpu_burst_str,
                              struct t_action *action);

t_boolean DAP_read_msg_send  (const char      *msg_send_str,
                              struct t_action *action);

t_boolean DAP_read_msg_recv  (const char      *msg_recv_str,
                              struct t_action *action);

t_boolean DAP_read_global_op (const char      *global_op_str,
                              struct t_action *action);

t_boolean DAP_read_event     (const char      *event_str,
                              struct t_action *action);

/*
 * DEBUG routine
 */
void DAP_print_app_structure(app_struct *app);

int DAP_get_thread_ptask_num(struct ptask_structure* ptask, int task);

int DAP_int_field_read (char *buffer,
                               int buffer_size,
                               char *field,
                               int field_size,
                               int offset,
                               int *rec);

/*****************************************************************************
 * API routines implementation
 ****************************************************************************/
char* DATA_ACCESS_get_error (void)
{
  return main_struct.error_string;
}


// API SPECIFIC ROUTINES
// INIT OF APPLICATION
t_boolean DATA_ACCESS_init (int ptask_id, char *trace_file_location)
{
  // Init main structure if necessary
  if (main_struct.init_flag != 1)
  {
    if (!DAP_init_data_access_layer())
    {
      return FALSE;
    }
  }

  /* The third parameter indicates that the position of the application is
   * free to choice */
  return DAP_add_ptask (ptask_id, trace_file_location, -1);
}

// API SPECIFIC ROUTINES
// INIT OF APPLICATION
t_boolean DATA_ACCESS_init_index (int   ptask_id,
                                  char *trace_file_location,
                                  int   index)
{
  // Init main structure if necessary
  if (main_struct.init_flag != 1)
  {
    if (DAP_init_data_access_layer() < 0)
    {
      return FALSE;
    }
  }

  if (DAP_add_ptask (ptask_id, trace_file_location, index) == -1)
  {
    return FALSE;
  }

  return TRUE;
}

// RETURNS APLLICATION INFOFRMATION
t_boolean DATA_ACCESS_get_ptask_structure (int                      ptask_id,
                                           struct ptask_structure **ptask_info)
{
  app_struct *app = NULL;


  if ( (app = DAP_locate_app_struct (ptask_id)) == NULL)
  {
    *ptask_info = NULL;
    DAP_report_error ("invalid Ptask id %d", ptask_id);
    return FALSE;
  }
  else
  {
    *ptask_info =
      (struct ptask_structure*) malloc (sizeof (struct ptask_structure));

    (*ptask_info)->ptask_id      = app->ptask_id;
    (*ptask_info)->tasks_count   = app->tasks_count;

    (*ptask_info)->threads_per_task =
      (count_t*) malloc(app->tasks_count*sizeof(count_t));

    memcpy((*ptask_info)->threads_per_task,
           app->threads_count,
           app->tasks_count*sizeof(count_t));

    return TRUE;
  }
}

// RETURNS COMMUNICATORS
t_boolean DATA_ACCESS_get_communicators (int              ptask_id,
                                         struct t_queue **communicators)
{
  app_struct *app = NULL;

  if ( (app = DAP_locate_app_struct (ptask_id)) == NULL)
  {
    *communicators = NULL;
    DAP_report_error ("invalid Ptask id %d", ptask_id);
    return FALSE;
  }
  else
  {
    *communicators = &(app->comms);
    return TRUE;
  }
}

// RETURNS ACCTION
t_boolean DATA_ACCESS_get_next_action (int               ptask_id,
                                       int               task_id,
                                       int               thread_id,
                                       struct t_action **action)
{
  app_struct *app = NULL;

  /* Initialization of the file sharing */
  if (!main_struct.io_init)
  {
    if (!DAP_io_init())
    {
      return FALSE;
    }
  }

  /*
  printf("%s -> Action requested for P%02d T%02d (t%02d)\n",
         __FUNCTION__,
         ptask_id,
         task_id,
         thread_id);
  */

  if ( (app = DAP_locate_app_struct (ptask_id)) == NULL)
  {
    DAP_report_error ("invalid Ptask id %d", ptask_id);
    return FALSE;
  }
  else
  {
    return DAP_read_action (app, task_id, thread_id, action);
  }
}

// ENDS API
t_boolean DATA_ACCESS_end (void)
{
  int i = 0;

  for (i = 0; i < main_struct.num_apps; i++)
  {
    if (main_struct.apps[i] != NULL)
    {
      DAP_end_ptask(main_struct.apps[i]);
    }
  }

  free (main_struct.apps);

  main_struct.init_flag = 0;
  main_struct.num_apps  = 0;

  return TRUE;
}

// ENDS API for ptask_id
// returns the index in the "apps" structure
int DATA_ACCESS_ptask_id_end (int ptask_id)
{
  int i = 0;

  for (i = 0; i < main_struct.num_apps; i++)
  {
    if (main_struct.apps[i] != NULL)
    {
      if (main_struct.apps[i]->ptask_id == ptask_id)
      {
        /*printf("RELOAD: %d == %d ; i = %d\n", main_struct.apps[i]->ptask_id, ptask_id, i);*/
        DAP_end_ptask (main_struct.apps[i]);
        return i;
      }
    }
  }

  return -1;
}
/*
 * PROBE AND INTERNAL TESTING ROUTINES
 */

// Prints all basic data relevant to appliction identified by ptask_id
// At this point DATA_ACCESS_Init should be invoiced previosly in order to have data to print.
int DATA_ACCESS_test_routine (int ptask_id)
{
  int i;

  for (i = 0; i < DATA_ACCESS_MAX_NUM_APP; i++)
  {
    if (main_struct.apps[i] != NULL && main_struct.apps[i]->ptask_id == ptask_id)
    {
      DAP_print_app_structure(main_struct.apps[i]);
    }
  }
  return i;
}

/******************************************************************************
 * Private functions implementation
 *****************************************************************************/

/**
 * Processes error messages in a 'printf' style
 */
void DAP_report_error (const char *format, ...)
{
  va_list args;
  va_start (args, format);
  vsprintf(main_struct.error_string, format, args);
  va_end(args);
}

/**
 * Creates the API main structure
 */
t_boolean DAP_init_data_access_layer(void)
{
  int i = 0;

  main_struct.apps =
    (app_struct **) malloc (DATA_ACCESS_MAX_NUM_APP *
                                       sizeof (app_struct *));

  if (main_struct.apps == NULL)
  {
    DAP_report_error("no memory available to data access management structure");
    return FALSE;
  }
  // Inialize apps to avoid problems
  for (i = 0; i < DATA_ACCESS_MAX_NUM_APP; i++)
  {
    main_struct.apps[i] = NULL;
  }

  main_struct.num_apps  = 0;
  main_struct.init_flag = TRUE;
  main_struct.io_init   = FALSE;

  return TRUE;
}

/**
 * Returns the 'app_struct' for a given 'ptask_id' if it exists, NULL otherwise
 */
app_struct* DAP_locate_app_struct (int ptask_id)
{
  int i = 0;

  for (i = 0; i < main_struct.num_apps; i++)
  {
    if (main_struct.apps[i] != NULL)
    {
      if (ptask_id == main_struct.apps[i]->ptask_id)
      {
        return main_struct.apps[i];
      }
    }
  }

  return NULL;
}

/**
 * Reload of an application
 */
t_boolean DATA_ACCESS_reload_ptask (int ptask_id)
{
  app_struct *app;
  count_t     tasks_it;
  count_t     threads_it;
  if (debug) {
    PRINT_TIMER(current_time);
    printf(": Reloading task %d\n", ptask_id);
  }

  if ( (app = DAP_locate_app_struct (ptask_id)) == NULL)
  {
    DAP_report_error ("invalid Ptask id %d", ptask_id);
    return FALSE;
  }

  for (tasks_it = 0; tasks_it < app->tasks_count; tasks_it++)
  {
    for (threads_it = 0;
         threads_it < app->threads_count[tasks_it];
         threads_it++)
    {
      app->current_threads_offsets[tasks_it][threads_it] =
        app->threads_offsets[tasks_it][threads_it];
    }
  }
  DAP_reset_app_stream_fps (app);

  return TRUE;
}


/**
 * Initialization of an application
 */
t_boolean DAP_add_ptask (int ptask_id, char *trace_file_name, int index)
{
  int num_a      = 0;

  app_struct *app;

  if (index == -1)
    num_a = main_struct.num_apps;
  else
    num_a = index;

  if (num_a < DATA_ACCESS_MAX_NUM_APP)
  {
    app = (app_struct*) malloc (sizeof (app_struct));


    if (app == NULL)
    {
      DAP_report_error("not enough memory");
      return DATA_ACCESS_ERROR;
    }
    main_struct.apps[num_a] = app;

    app->ptask_id        = ptask_id;
    app->trace_file_name = strdup(trace_file_name);

    // Open the main stream for current ptask
    if ( (main_struct.current_stream = IO_fopen(trace_file_name, "r")) == NULL)
    {
      DAP_report_error("unable to open trace file: %s",
                       IO_get_error());
      return FALSE;
    }

    /* Strat reading procedures: header, definitions, offsets */
    if (DAP_read_header (app,
                         ptask_id) == DATA_ACCESS_ERROR)
    {
      return FALSE;
    }

    if (!DAP_read_definitions (app))
    {
      return FALSE;
    }

    /* After reading the definitions, we know where the operation records
     * start. Now read/locate the threads offsets */
    if (app->offsets_offset == 0)
    {
      /* Offsets not available. Locate them on trace file */
      if (!DAP_locate_offsets(app))
      {
        return FALSE;
      }
    }
    else
    {
      /* Offsets available. Load offsets from the end of file */
      if (!DAP_read_offsets(app))
      {
        return FALSE;
      }
    }

    if ( IO_fclose(main_struct.current_stream) != 0)
    {
      DAP_report_error("error closing trace '%s' of ptask %d: %s",
                       trace_file_name,
                       ptask_id,
                       IO_get_error());
      return FALSE;
    }
    main_struct.current_stream = NULL;
    main_struct.num_apps++;
  }
  else
  {
    DAP_report_error("max number of applications reached");
    return FALSE;
  }

  return TRUE;
}

/**
 * Frees resource of an application
 */
void DAP_end_ptask (app_struct *app)
{
  int i = 0;

  free (app->app_name);
  free (app->trace_file_name);
  free (app->threads_count);

  for (i = 0; i < app->tasks_count; i++)
  {
    free (app->threads_offsets[i]);
    free (app->streams_idxs[i]);
    free (app->current_threads_offsets[i]);
  }

  free (app->threads_offsets);
  free (app->streams_idxs);
  free (app->current_threads_offsets);


  /* Close all streams used by the application */
  for (i = 0; i < app->streams_count; i++)
  {
    if (app->streams[i].fp != NULL)
    {
      IO_fclose (app->streams[i].fp);
    }
  }
  free (app->streams);

  // Free array of streams

  // Deletes queue of comunicators

  /* TO DO: clear all elements in the queue
  res = empty_queue (&app->comms); */

  // Free queue DBG
  // free (app->comms);
  // free (app->comms);
  // Frees list of avaluable streams
}

/**
 * Initalization of the application structure
 */
t_boolean DAP_initialize_app (app_struct *app,
                              int         ptask_id,
                              char       *app_name,
                              count_t     tasks_count,
                              count_t    *threads_count,
                              count_t     comms_count,
                              off_t       offsets_offset)
{
  // Local variables
  int i;        // Local iterators


  app->ptask_id      = ptask_id;
  app->app_name      = strdup(app_name);
  app->tasks_count   = tasks_count;

  app->threads_count = (count_t*) malloc(tasks_count*sizeof(count_t));
  memcpy(app->threads_count, threads_count, tasks_count*sizeof(count_t));

  app->comms_count    = comms_count;
  app->offsets_offset = offsets_offset;

  /* Check the total number of threads */
  app->total_threads_count = 0;
  for (i = 0; i < tasks_count; i++)
  {
    app->total_threads_count += app->threads_count[i];
  }

  /* Allocate space for the threads offsets positions */
  app->threads_offsets         = (off_t**)  malloc(tasks_count*sizeof(off_t*));
  app->current_threads_offsets = (off_t**)  malloc(tasks_count*sizeof(off_t*));

  for (i = 0; i < tasks_count; i++)
  {
    app->threads_offsets[i] =
      (off_t*) malloc(threads_count[i]*sizeof(off_t));

    app->current_threads_offsets[i] =
      (off_t*) malloc(threads_count[i]*sizeof(off_t));
  }

  create_queue(&app->comms);


  return TRUE;
}

/**
 * Main header reading procedure
 */
t_boolean DAP_read_header (app_struct *ptask,
                           int         ptask_id)
{
  char*   header        = NULL;
  size_t  header_length = 0;
  ssize_t bytes_read;

  char   *app_name_str, *offsets_str, *ptask_info_str;

  int     offset_present;
  size_t  offsets_offset = 0;

  count_t tasks_count;
  char   *threads_str;
  count_t comms_count;

  count_t *threads_count;
  char    *current_thread_count_str;
  count_t  current_task;

  /* Obtain the header line */
  if ( (bytes_read = getline(&header,
                             &header_length,
                             main_struct.current_stream )) == -1)
  {
    DAP_report_error("unable to retrieve header line");
    return DATA_ACCESS_ERROR;
  }

  app_name_str   = malloc(bytes_read+1);
  offsets_str    = malloc(bytes_read+1);
  ptask_info_str = malloc(bytes_read+1);

  /* Scan the general header structure */

  // Header format: #DIMEMAS:trace_name:offsets[,offsets_offset]:ptask_info
  if (sscanf(header,
             "#DIMEMAS:%[^:]:%[^:]:%s",
             app_name_str,
             offsets_str,
             ptask_info_str) != 3)
  {
    DAP_report_error("wrong header format");
    return DATA_ACCESS_ERROR;
  }

  /* Check if offsets are correctly defined */
  if (sscanf(offsets_str, "%d,%zu", &offset_present, &offsets_offset) != 2)
  {
    if (sscanf(offsets_str, "%d", &offset_present) != 1)
    {
      DAP_report_error("wrong offsets definition on trace header");
      return DATA_ACCESS_ERROR;
    }
  }

  /* Check if offset definitions are correctly set */
  switch(offset_present)
  {
    case OFFSET_PRESENT:
      if (offsets_offset == 0)
      {
        DAP_report_error("wrong offsets offset position on trace header");
        return DATA_ACCESS_ERROR;
      }
      break;

    case OFFSET_NOT_PRESENT:
      if (offsets_offset != 0)
      {
        DAP_report_error("offsets offset wrong set");
        return DATA_ACCESS_ERROR;
      }
      break;

    default:
      DAP_report_error("unknown offsets hint value on header \'%d\'", offset_present);
      return DATA_ACCESS_ERROR;
  }

  /* Scan the ptask information substring
   * format: task_num(task_1_thread_num,...,task_n_thread_num),comms_num */
  threads_str = malloc(bytes_read+1);

  if (sscanf(ptask_info_str,
             "%d(%[^)]),%d",
             &tasks_count,
             threads_str,
             &comms_count) != 3)
  {
    DAP_report_error("wrong application definition on trace header: %s",
                     ptask_info_str);
    return DATA_ACCESS_ERROR;
  }

  threads_count = (count_t *) malloc(tasks_count*sizeof(count_t));

  /* 'Tokenize' the tasks definition to obtain the threads per task */
  current_task = 0;
  current_thread_count_str = strtok(threads_str, ",");
  while (current_thread_count_str != NULL)
  {
    threads_count[current_task] = atoi(current_thread_count_str);
    current_task++;
    current_thread_count_str = strtok(NULL, ",");
  }

  if (current_task != tasks_count)
  {
    DAP_report_error("number of threads per task list different from total number of tasks on trace header");
    return DATA_ACCESS_ERROR;
  }

  if (!DAP_initialize_app(ptask,
                         ptask_id,
                         app_name_str,
                         tasks_count,
                         threads_count,
                         comms_count,
                         offsets_offset))
  {
    return FALSE;
  }

  free (current_thread_count_str);
  free (app_name_str);
  free (offsets_str);
  free (ptask_info_str);
  free (header);

  return DATA_ACCESS_OK;
}

/**
 * Main procedure for object definition reading
 */
t_boolean DAP_read_definitions (app_struct *app)
{
  char*   line        = NULL;
  size_t  line_length = 0;
  ssize_t bytes_read;

  t_boolean first_record_found;
  int   object_type;
  char* object_fields;

  size_t total_definitions_found = 0;

  first_record_found = FALSE;
  while (!first_record_found)
  {
    line        = NULL;
    line_length = 0;

    if ( (bytes_read = getline(&line,
                               &line_length,
                               main_struct.current_stream )) == -1)
    {
      DAP_report_error("error reading object definitions: %s",
                       strerror(errno));

      return FALSE;
    }

    object_fields = (char*) malloc(strlen(line)+1);

    if (sscanf(line,
               DEFINITION_REGEXP,
               &object_type,
               object_fields) != 2)
    {
      first_record_found = TRUE;
    }
    else
    {
      total_definitions_found++;

      switch (object_type)
      {
        case DEF_COMMUNICATOR:
          DAP_read_communicator(app, object_fields);
          break;
        case DEF_FILE:
          /* Not yet implemented :( */
          break;
        case DEF_ONESIDED_WIN:
          /* Not yet implemented :( */
          break;
        default:
          DAP_report_error("unknown object definition type: %s", line);
          return FALSE;
      }
    }
    free(object_fields);
  }

  /* Set the records starting offset */
  app->records_offset = (IO_ftello(main_struct.current_stream)) - bytes_read;

  /* DEBUG
  printf ("Total definitions found = %zu\n", total_definitions_found);
  printf ("Current file offset %zu\n", IO_ftello(main_struct.current_stream));
  printf ("Bytes read = %zu\n", bytes_read);
  printf ("Records offset = %zu\n", app->records_offset);
  */

  return TRUE;
}

/**
 * Communicator definition parsing
 */
t_boolean DAP_read_communicator(app_struct *app, const char *comm_fields)
{
  struct t_communicator *new_communicator;

  int   comm_id;
  int   comm_tasks_count;
  char* comm_tasks_str;

  char* current_task_str;
  int   actual_comm_tasks_count;

  comm_tasks_str = malloc(strlen(comm_fields)+1);

  if (comm_tasks_str == NULL)
  {
    DAP_report_error("unable to allocate memory for application communicator");
    return FALSE;
  }

  if ( sscanf(comm_fields,
              COMMUNICATOR_REGEXP,
              &comm_id,
              &comm_tasks_count,
              comm_tasks_str) != 3)
  {
    DAP_report_error("wrong communicator definition: %s", comm_fields);
    return FALSE;
  }

  new_communicator =
    (struct t_communicator*) malloc(sizeof(struct t_communicator));

  if (new_communicator == NULL)
  {
    DAP_report_error("unable to allocate memory for application communicators");
    return FALSE;
  }

  new_communicator->communicator_id = comm_id;
  new_communicator->current_root    = NULL;

  create_queue (&new_communicator->global_ranks);
  create_queue (&new_communicator->threads);
  create_queue (&new_communicator->machines_threads);
  create_queue (&new_communicator->m_threads_with_links);

  actual_comm_tasks_count = 0;

  current_task_str = strtok(comm_tasks_str, ":");
  while (current_task_str != NULL)
  {
    count_t* current_task_ptr = (count_t *) malloc(sizeof(count_t));

    if (current_task_ptr == NULL)
    {
      DAP_report_error("unable to allocate memory for communicator ranks");
    }

    (*current_task_ptr) = atoi(current_task_str);

    inFIFO_queue(&new_communicator->global_ranks, (char*) current_task_ptr);
    actual_comm_tasks_count++;
    current_task_str = strtok(NULL, ":");
  }

  if (actual_comm_tasks_count != comm_tasks_count)
  {
    DAP_report_error("number ranks of communicator %d are different from the defined %d",
                     actual_comm_tasks_count,
                     comm_id,
                     comm_tasks_count);

    free(new_communicator);
    return FALSE;
  }

  insert_queue(&(app->comms), (char*) new_communicator, (t_priority) comm_id);

  return TRUE;
}

/**
 * Function to read offsets already present on trace file
 */
t_boolean DAP_read_offsets (app_struct *app)
{
  char*   line        = NULL;
  size_t  line_length = 0;
  ssize_t bytes_read;

  count_t  current_task;

  count_t current_task_read;
  char*   threads_offsets_str;
  char*   current_thread_offset_str;

  // Set the stream to the offsets offset position
  if ( IO_fseeko(main_struct.current_stream, app->offsets_offset, SEEK_SET) < 0)
  {
    DAP_report_error ("error on locating the position of tasks offsets");
    return FALSE;
  }

  // Read the offset of each task/thread of the application
  for (current_task = 0; current_task < app->tasks_count; current_task++)
  {
    count_t current_thread;

    line        = NULL;
    line_length = 0;
    line_length = 0;

    if ( (bytes_read = getline(&line,
                               &line_length,
                               main_struct.current_stream )) == -1)
    {
      if (feof(main_struct.current_stream))
      {
        DAP_report_error("number of tasks offsets lower than application tasks (%d expected, %d read)",
                         app->tasks_count,
                         current_task);
      }
      else
      {
        DAP_report_error("error reading offsets line");
      }

      return FALSE;
    }

    threads_offsets_str = malloc(strlen(line)+1);

    if (sscanf(line,
               OFFSET_REGEXP,
               &current_task_read,
               threads_offsets_str) != 2)
    {
      DAP_report_error("wrong offset definition %s",line);
      return FALSE;
    }

    /*
    if (offset_record_id != DATA_ACCESS_OFFDEF)
    {
      DAP_report_error("wrong offset definition %s",line);
      return FALSE;
    }
    */

    if (current_task_read != current_task)
    {
      DAP_report_error("wrong task sequence when reading offsets (%d expected, %d read)",
                       current_task,
                       current_task_read);
      return FALSE;
    }

    current_thread = 0;
    current_thread_offset_str  = strtok(threads_offsets_str, ":");
    while (current_thread_offset_str != NULL)
    {
      if (current_thread >= app->threads_count[current_task])
      {
        DAP_report_error("number of threads offsets bigger than task threads (%d expected, %d read)",
                         app->threads_count[current_task],
                         current_thread);
        return FALSE;
      }

      app->threads_offsets[current_task][current_thread] =
        (off_t) strtoul(current_thread_offset_str, NULL, 0);

      app->current_threads_offsets[current_task][current_thread] =
        app->threads_offsets[current_task][current_thread];

      current_thread++;
      current_thread_offset_str = strtok(NULL, ":");
    }

    if (current_thread < app->threads_count[current_task])
    {
      DAP_report_error("number of threads offsets smaller than task threads (%d expected, %d read)",
                       app->threads_count[current_task],
                       current_thread);
      return FALSE;
    }

    free(threads_offsets_str);
    free(line);
  }

  return TRUE;
}

/**
 * Function to locate the thread offsets when not present on trace file
 */
t_boolean DAP_locate_offsets(app_struct *app)
{
  count_t task, thread;

  off_t last_thread_offset;
  off_t file_size;

  if (fseek(main_struct.current_stream, 0, SEEK_END) < 0)
  {
    DAP_report_error("unable to locate end of trace");
    return FALSE;
  }
  file_size = ftello(main_struct.current_stream);

  if ( IO_fseeko(main_struct.current_stream, app->records_offset, SEEK_SET) < 0)
  {
    DAP_report_error("unable to reposition trace at initial record %zu (%s)",
                     app->records_offset,
                     strerror(errno));
    return FALSE;
  }

  last_thread_offset = app->records_offset;

  for (task = 0; task < app->tasks_count; task++)
  {
    for (thread = 0; thread < app->threads_count[task]; thread++)
    {
      if (task == 0 && thread == 0)
      {
        last_thread_offset = app->records_offset;
      }
      else
      {
        last_thread_offset = DAP_locate_thread_offset(app,
                                                      task,
                                                      thread,
                                                      last_thread_offset,
                                                      file_size);
      }

      if (last_thread_offset == 0)
      {
        return FALSE;
      }

      app->current_threads_offsets[task][thread] = last_thread_offset;
      app->threads_offsets[task][thread]         = last_thread_offset;
    }
  }

  return TRUE;
}

/**
 * Dicotomic search to locate the offset of a single thread
 */
off_t DAP_locate_thread_offset(app_struct *app,
                               count_t     task_id,
                               count_t     thread_id,
                               off_t       lower_bound,
                               off_t       upper_bound)
{
  char*   line        = NULL;
  size_t  line_length = 0;
  ssize_t bytes_read;

  off_t result;

  int     op_id;
  count_t read_task_id, read_thread_id;
  char   *op_fields;

  t_boolean found = FALSE;

  if (upper_bound <= lower_bound)
  {
    DAP_report_error("error locating offset for task %d, thread %d",
                     task_id,
                     thread_id);
    return 0;
  }

  if (upper_bound - lower_bound < DATA_ACCESS_SEEK_THRESHOLD)
  {
    /* Line by line seek. We use -1 to check if we need to read a new full
     * line */
    if (fseek(main_struct.current_stream, lower_bound-1, SEEK_SET) < 0)
    {
      DAP_report_error("unable to reposition trace (%s)",
                       strerror(errno));
      return FALSE;
    }

    /* First line is discarded to avoid possible seek missplacements */
    if ( (bytes_read = getline(&line,
                               &line_length,
                               main_struct.current_stream )) == -1)
    {
      DAP_report_error("error locating offset for task %d, thread %d: %s",
                       task_id,
                       thread_id,
                       strerror(errno));
      return 0;
    }
    free (line);

    while (!found)
    {
      line        = NULL;
      line_length = 0;

      if ( (bytes_read = getline(&line,
                                 &line_length,
                                 main_struct.current_stream )) == -1)
      {
        DAP_report_error("error locating offset for task %d, thread %d: %s",
                         task_id,
                         thread_id,
                         strerror(errno));
        return 0;
      }

      op_fields = malloc(strlen(line)+1);

      if (sscanf(line,
                 ACTION_REGEXP,
                 &op_id,
                 &read_task_id,
                 &read_thread_id,
                 op_fields) != 4)
      {
        DAP_report_error("error accessing to an operation record when locating offsets: %s",
                         line);

        free(op_fields);
        free(line);

        return 0;
      }

      free(op_fields);
      free(line);

      if (task_id == read_task_id && thread_id == read_thread_id)
      {
        result  = ftello(main_struct.current_stream);
        result -= bytes_read;

        found = TRUE;
      }
    }
  }
  else
  {
    /* Dicotomic search */
    off_t half_bound = (upper_bound-lower_bound)/2;

    if (fseek(main_struct.current_stream, half_bound, SEEK_SET) < 0)
    {
      DAP_report_error("unable to reposition trace at %zu (%s)",
                       half_bound,
                       strerror(errno));
      return FALSE;
    }

    /* First line is discarded to avoid possible seek missplacements */
    if ( (bytes_read = getline(&line,
                               &line_length,
                               main_struct.current_stream )) == -1)
    {
      DAP_report_error("error locating offset for task %d, thread %d: %s",
                       task_id,
                       thread_id,
                       strerror(errno));
      return 0;
    }
    free(line);

    line        = NULL;
    line_length = 0;
    if ( (bytes_read = getline(&line,
                               &line_length,
                               main_struct.current_stream )) == -1)
    {
      DAP_report_error("error locating offset for task %d, thread %d: %s",
                       task_id,
                       thread_id,
                       strerror(errno));
      return 0;
    }

    op_fields = malloc(strlen(line)+1);

    if (sscanf(line,
               ACTION_REGEXP,
               &op_id,
               &read_task_id,
               &read_thread_id,
               op_fields) != 4)
    {
      DAP_report_error("error accessing to an operation record when locating offsets: %s",
                       line);

      free(op_fields);
      free(line);

      return 0;
    }

    free(op_fields);
    free(line);

    if (task_id > read_task_id)
    {
      result = DAP_locate_thread_offset(app, task_id, thread_id, half_bound, upper_bound);
    }
    else if (task_id < read_task_id)
    {
      result = DAP_locate_thread_offset(app, task_id, thread_id, lower_bound, half_bound);
    }
    else
    {
      if (thread_id > read_thread_id)
      {
        result = DAP_locate_thread_offset(app, task_id, thread_id, half_bound, upper_bound);
      }
      else
      {
        result = DAP_locate_thread_offset(app, task_id, thread_id, lower_bound, half_bound);
      }
    }
  }

  return result;
}

/**
 * Initialization of API I/O file sharing
 */
t_boolean DAP_io_init(void)
{
  size_t total_available_streams;

  int         apps_it;
  app_struct *app;

  size_t total_api_threads;

  size_t streams_per_app[DATA_ACCESS_MAX_NUM_APP];

  if (main_struct.io_init)
  {
    return TRUE;
  }

  total_api_threads = 0;


  for (apps_it = 0; apps_it < main_struct.num_apps; apps_it++)
  {
    app = main_struct.apps[apps_it];
    total_api_threads += app->total_threads_count;
  }

  /* Get the total available streams from the I/O manager */
  total_available_streams = IO_available_streams();

  for (apps_it = 0; apps_it < main_struct.num_apps; apps_it++)
  {
    app = main_struct.apps[apps_it];

    if (total_api_threads <= total_available_streams)
    {
      /* Number of available pointers is smaller than total threads count
       * each application receives as much streams as it needs */
      streams_per_app[apps_it] = app->total_threads_count;
    }
    else
    {
      streams_per_app[apps_it] =
        (app->total_threads_count/total_api_threads)*total_available_streams;
    }

    if (!streams_per_app[apps_it]) {
      DAP_report_error("streams per app is 0 for app=%d", apps_it);
      return FALSE;
    }

    if (!DAP_allocate_streams(app,
                              streams_per_app[apps_it]))
    {
      DAP_report_error("Could not allocate streams for apps_it=%d", apps_it);
      return FALSE;
    }
  }

  /* DEBUG
  printf("Total available streams %zu\n", total_available_streams);
  printf("Streams per application: ");
  for (apps_it = 0; apps_it < main_struct.num_apps; apps_it++)
  {
    printf(" %d:%zu ", apps_it, streams_per_app[apps_it]);
  }
  printf("\n");
  */

  /* I/O has been correctly initialized! */
  main_struct.io_init = TRUE;

  return TRUE;
}


/**
 * Initialization of I/O file sharing for each application
 */
t_boolean DAP_allocate_streams(app_struct *app, size_t assigned_streams)
{
  size_t i;
  count_t tasks_it, threads_it;
  count_t current_thread;

  app->streams_count = assigned_streams;

  /* Initialize the 'fp_share' array */
  app->streams =
    (fp_share*) malloc(assigned_streams*sizeof(fp_share));

  if (app->streams == NULL)
  {
    DAP_report_error ("unable to allocate memory for I/O streams");
    return FALSE;
  }

  /* Open all streams */
  for (i = 0; i < assigned_streams; i++)
  {
    /* Deferred open to guarantee access from task_0, thread_0 doesn't break
     * the algorithm
    if ( (app->streams[i].fp = IO_fopen(app->trace_file_name, "r")) == NULL)
    {
      DAP_report_error ("unable to open trace '%s': %s",
                        app->trace_file_name,
                        IO_get_error());
      return FALSE;
    }
    */
    app->streams[i].fp = NULL;
    app->streams[i].last_task_id   = 0;
    app->streams[i].last_thread_id = 0;
  }

  /* Initialize the thread stream indexes */
  app->streams_idxs =
    (size_t**) malloc(app->tasks_count*sizeof(size_t*));

  /* DEBUG
  printf ("Ptask %d assigned streams = %d\n", app->ptask_id, assigned_streams);
  printf ("Stream idx per thread for Ptask %d: ", app->ptask_id);
  */

  current_thread = 0;
  for (tasks_it = 0; tasks_it < app->tasks_count; tasks_it++)
  {
    app->streams_idxs[tasks_it] =
      (size_t*) malloc(app->threads_count[tasks_it]*sizeof(size_t));

    for (threads_it = 0;
         threads_it < app->threads_count[tasks_it];
         threads_it++)
    {
      /* DEBUG
      printf("Stream (app %d, assigned %d) [%d:%d] -> %d (%d)\n",
             app->ptask_id,
             assigned_streams,
             tasks_it,
             threads_it,
             (current_thread*assigned_streams)/app->total_threads_count,
             (current_thread*assigned_streams)/app->threads_count[tasks_it]);
      */

      app->streams_idxs[tasks_it][threads_it] =
        (current_thread*assigned_streams)/app->total_threads_count;
      current_thread++;
    }
  }

  return TRUE;
}

/**
 * Returns the stream of a given thread ready to access
 */
FILE* DAP_get_stream (app_struct* app, int task_id, int thread_id)
{
  FILE     *result = NULL;
  fp_share *assigned_fp;

  size_t assigned_stream_idx = app->streams_idxs[task_id][thread_id];

  assigned_fp = &(app->streams[assigned_stream_idx]);

  if (assigned_fp->fp == NULL)
  {
    /* First utilization: open the stream... */
    if ( (assigned_fp->fp = IO_fopen(app->trace_file_name, "r")) == NULL)
    {
      DAP_report_error("unable to open trace '%s' to access task %d thread %d information",
                       app->trace_file_name,
                       task_id,
                       thread_id);
      return NULL;
    }

    /* ... and set the position */
    if ( IO_fseeko(assigned_fp->fp,
                   app->threads_offsets[task_id][thread_id], // Initial offset!
                   SEEK_SET) == -1)
    {
      DAP_report_error("unable to position the trace '%s' to access task %d thread %d information: %s",
                       app->trace_file_name,
                       task_id,
                       thread_id,
                       IO_get_error());
      return NULL;
    }

    result = assigned_fp->fp;

    assigned_fp->last_task_id   = task_id;
    assigned_fp->last_thread_id = thread_id;
  }
  else
  {
    if (assigned_fp->last_task_id   == task_id &&
        assigned_fp->last_thread_id == thread_id)
    {
      /* The stream can be used 'as is' because it keeps the position */
      result = assigned_fp->fp;
    }
    else
    {
      /* Reposition to pointer to current offset */
       if ( IO_fseeko(assigned_fp->fp,
                      app->current_threads_offsets[task_id][thread_id],
                      SEEK_SET) == -1)
      {
        DAP_report_error("unable to position the trace '%s' to access task %d thread %d information: %s",
                       app->trace_file_name,
                       task_id,
                       thread_id,
                       IO_get_error());
        return NULL;
      }

      result = assigned_fp->fp;

      assigned_fp->last_task_id   = task_id;
      assigned_fp->last_thread_id = thread_id;
    }
  }

  return result;
}

/**
 * This function resests the "fp" for a ptask's streams
 * So that the "next_action" is forced to reposition the offset.
 */
t_boolean DAP_reset_app_stream_fps (app_struct *app) {
  count_t task, thread;

  /* PRINT_TIMER(current_time);
  printf(": Resetting app stream fps for %s\n", app->trace_file_name); */

  for (task = 0; task < app->tasks_count; task++)
  {
    for (thread = 0; thread < app->threads_count[task]; thread++)
    {
      fp_share *assigned_fp;
      size_t assigned_stream_idx = app->streams_idxs[task][thread];
      assigned_fp = &(app->streams[assigned_stream_idx]);
      if (assigned_fp) { // Forces repositioning of file descriptor pointer
        assigned_fp->last_task_id   = -1;
        assigned_fp->last_thread_id = -1;
      }
    }
  }

  return TRUE;
}

/**
 * Reads an operation (action) record from the given task/thread, if available
 */
t_boolean DAP_read_action (app_struct       *app,
                           int               task_id,
                           int               thread_id,
                           struct t_action **action)
{
  t_boolean result;

  FILE *stream;

  char*   line        = NULL;
  size_t  line_length = 0;
  ssize_t bytes_read;

  int   op_id, read_task_id, read_thread_id;
  char *op_fields;

  if ( (stream = DAP_get_stream(app, task_id, thread_id)) == NULL)
  {
    printf("DAP_get_stream failed %d,%d\n", task_id, thread_id);
    return FALSE;
  }

  if ( (bytes_read = getline(&line,
                             &line_length,
                             stream )) == -1)
  {
    if (feof(stream))
    {
      /* EOF is not an error */
      /* printf("EOF reached %d,%d, read %d bytes\n", task_id, thread_id, bytes_read); */
      (*action) = NULL;
      return TRUE;
    }

    DAP_report_error("error reading operation record definitions: %s",
                     strerror(errno));

    return FALSE;
  }

  /*
  PRINT_TIMER (current_time);
  printf(": App %d read: %s", app->ptask_id, line);
  */

  if ( (op_fields = malloc(bytes_read+1)) == NULL)
  {
    DAP_report_error("unable to allocate memory to parse an action record");

    free(line);

    return FALSE;
  }

  if (sscanf(line,
             ACTION_REGEXP,
             &op_id,
             &read_task_id,
             &read_thread_id,
             op_fields) != 4)
  {
    /* Check if it is a NOOP! */
    if (sscanf(line,
               NOOP_REGEXP,
               &read_task_id,
               &read_thread_id) == 2)
    {
      if (read_task_id != task_id || read_thread_id != thread_id)
      {
        free(op_fields);
        free(line);
        (*action) = NULL;

        return TRUE;
      }
      else
      {
        (*action) = (struct t_action*) malloc(sizeof(struct t_action));

        if ( (*action) == NULL)
        {
          DAP_report_error("unable to allocate space for new action");

          free(op_fields);
          free(line);

          return FALSE;
        }

        (*action)->next   = NULL;
        (*action)->action = NOOP;

        app->current_threads_offsets[task_id][thread_id] += (off_t) bytes_read;

        free(op_fields);
        free(line);

        return TRUE;

      }
    }

    if (sscanf(line,
               OFFSET_REGEXP,
               &read_task_id,
               op_fields) == 2)
    {
      /* 'line' contains an offset record. We have finished the records for
       * last task/thread */
      (*action) = NULL;

      free(op_fields);
      free(line);

      return TRUE;
    }

    DAP_report_error("wrong operation record format: %s", line);

    free(op_fields);
    free(line);

    return FALSE;
  }

  if (read_task_id != task_id || read_thread_id != thread_id)
  {
    /* no more actions for current task/thread */
    free(op_fields);
    free(line);

    (*action) = NULL;

    return TRUE;
  }
  else
  {
    /* Allocate memory for new action */
    (*action) = (struct t_action*) malloc(sizeof(struct t_action));

    if ( (*action) == NULL)
    {
      DAP_report_error("unable to allocate space for new action");
      return FALSE;
    }

    (*action)->next = NULL;

    /* check the operation id, and proceed */
    switch (op_id)
    {
      case RECORD_CPU_BURST:
        result = DAP_read_CPU_burst(op_fields, *action);
        break;
      case RECORD_MSG_SEND:
        result = DAP_read_msg_send(op_fields, *action);
        break;
      case RECORD_MSG_RECV:
        result = DAP_read_msg_recv(op_fields, *action);
        break;
      case RECORD_GLOBAL_OP:
        result = DAP_read_global_op(op_fields, *action);
        break;
      case RECORD_EVENT:
        result = DAP_read_event(op_fields, *action);
        break;
      default:
        DAP_report_error("unknown action: %s", line);
        result = FALSE;
        break;
    }

    if (result == FALSE)
    {
      READ_free_action( (*action) );
      // free( (char*) (*action) );
      (*action) = NULL;
    }
    else
    {
      app->current_threads_offsets[task_id][thread_id] += (off_t) bytes_read;

      /* DEBUG
      PRINT_TIMER (current_time);
      printf(": [P%02d:T%02d:t%02d] Offset: %jd\n",
             app->ptask_id,
             task_id,
             thread_id,
             (intmax_t) app->current_threads_offsets[task_id][thread_id]);
      */
    }
  }

  free(op_fields);
  free(line);
  return result;
}

t_boolean DAP_read_CPU_burst (const char      *cpu_burst_str,
                              struct t_action *action)
{
  double duration;

  if (sscanf(cpu_burst_str,
             CPU_BURST_REGEXP,
             &duration) != 1)
  {
    DAP_report_error("wrong CPU burst duration: %s", cpu_burst_str);
    return FALSE;
  }

  action->action                = WORK;
  action->desc.compute.cpu_time = duration;

  return TRUE;
}

t_boolean DAP_read_msg_send  (const char      *msg_send_str,
                              struct t_action *action)
{
  int      dest_task_id;
  int      dest_thread_id;
  long int msg_size;
  int      tag;
  int      comm_id;
  int      sync;


  if (sscanf(msg_send_str,
             MSG_SEND_REGEXP_SS,
             &dest_task_id,
             &dest_thread_id,
             &msg_size,
             &tag,
             &comm_id,
             &sync) != 6)
  {
    if (sscanf(msg_send_str,
                   MSG_SEND_REGEXP_MPI,
                  &dest_task_id,
                  &msg_size,
                  &tag,
                  &comm_id,
                  &sync) == 5)
    {
      /* Regular MPI message */
      dest_thread_id = -1;
    }
    else
    {
      DAP_report_error("wrong message sending operation: %s", msg_send_str);
      return FALSE;
    }
  }

  /* Bad translation correction */
  if (dest_thread_id == -2)
  {
    dest_thread_id = -1;
  }

  action->action                = SEND;
  action->desc.send.mess_size   = msg_size;
  action->desc.send.dest        = dest_task_id;
  action->desc.send.dest_thread = dest_thread_id;
  action->desc.send.mess_tag    = tag;
  action->desc.send.communic_id = comm_id;

  action->desc.send.rendez_vous = USE_RENDEZ_VOUS((sync & ((int)1)), msg_size);

  if (sync & ((int)2))
  {
    action->desc.send.immediate = TRUE;
  }
  else
  {
    action->desc.send.immediate = FALSE;
  }

  return TRUE;
}

t_boolean DAP_read_msg_recv  (const char      *msg_recv_str,
                              struct t_action *action)
{
  int      src_task_id;
  int      src_thread_id;
  long int msg_size;
  int      tag;
  int      comm_id;
  int      rcvt;

  if (sscanf(msg_recv_str,
             MSG_RECV_REGEXP_SS,
             &src_task_id,
             &src_thread_id,
             &msg_size,
             &tag,
             &comm_id,
             &rcvt) != 6)
  {
    if (sscanf(msg_recv_str,
                   MSG_RECV_REGEXP_MPI,
                  &src_task_id,
                  &msg_size,
                  &tag,
                  &comm_id,
                  &rcvt) == 5)
    {
      /* Regular MPI message */
      src_thread_id = -1;
    }
    else
    {
      DAP_report_error("wrong message reception operation: %s", msg_recv_str);
      return FALSE;
    }
  }

    /* Bad translation correction */
  if (src_thread_id == -2)
  {
    src_thread_id = -1;
  }

  action->desc.recv.mess_size   = msg_size;
  action->desc.recv.ori         = src_task_id;
  action->desc.recv.ori_thread  = src_thread_id;
  action->desc.recv.mess_tag    = tag;
  action->desc.recv.communic_id = comm_id;

  switch (rcvt)
  {
  case RECVTYPE_RECV:
    action->action = RECV;
    break;
  case RECVTYPE_IRECV:
    action->action = IRECV;
    break;
  case RECVTYPE_WAIT:
    action->action = WAIT;
    break;
  }

  return TRUE;
}

t_boolean DAP_read_global_op (const char      *global_op_str,
                              struct t_action *action)
{
  int      global_op_id;
  int      comm_id;
  int      root_task_id;
  int      root_thread_id;
  long int bytes_sent;
  long int bytes_recv;

  if (sscanf(global_op_str,
             GLOBAL_OP_REGEXP,
             &global_op_id,
             &comm_id,
             &root_task_id,
             &root_thread_id,
             &bytes_sent,
             &bytes_recv) != 6)
  {
    DAP_report_error("wrong global operation: %", global_op_str);
    return FALSE;
  }


  action->action                     = GLOBAL_OP;
  action->desc.global_op.glop_id     = global_op_id;
  action->desc.global_op.comm_id     = comm_id;
  action->desc.global_op.root_rank   = root_task_id;
  action->desc.global_op.root_thid   = root_thread_id;
  action->desc.global_op.bytes_send  = bytes_sent;
  action->desc.global_op.bytes_recvd = bytes_recv;

  return TRUE;
}

t_boolean DAP_read_event     (const char      *event_str,
                              struct t_action *action)
{
  long int type;
  long int value;

  if (sscanf(event_str,
             EVENT_REGEXP,
             &type,
             &value) != 2)
  {
    DAP_report_error("wrong event in trace: %", event_str);
    return FALSE;
  }

  action->action          = EVENT;
  action->desc.even.type  = type;
  action->desc.even.value = value;

  return TRUE;
}

void DAP_print_app_structure(app_struct *app)
{
  int i, j;

  printf ("*****************************************************\n");
  printf ("Application: %d\n", app->ptask_id);
  printf ("Num. of tasks: %d\n", app->tasks_count);

  printf ("Num. of threads:");
  for (i = 0; i < app->tasks_count; i++)
  {
    printf(" %d ", app->threads_count[i]);
  }
  printf ("\n");

  printf ("Threads offsets: ");
  for (i = 0; i < app->tasks_count; i++)
  {
    for (j = 0; j < app->threads_count[i]; j++)
    {
      printf(" %ju ", app->threads_offsets[i][j]);
    }
  }
  printf ("\n");

  printf ("Number of communicators: %d\n", count_queue(&app->comms));

  return;
}


