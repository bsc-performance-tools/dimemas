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

#include <math.h>
#include <string.h>
#include <assert.h>
#include <float.h>

#include <EventEncoding.h>

#include <define.h>
#include <types.h>
#include <aleatorias.h>
#include <cpu.h>
#include <events.h>
#include <extern.h>
#include <list.h>
#include <memory.h>
#include <paraver.h>
#include <ports.h>
#include <read.h>
#include <schedule.h>
#include <subr.h>
#include <task.h>
#include <random.h>
#include <modules_map.h>
#include <simulator.h>
#include <machine.h>
#include <node.h>
#include <file_data_access.h>
#include <communic.h>

#define MIN_SERVICE_TIME (t_nano)7000
#define MAX_SERVICE_TIME (t_nano)13000

/*****************************************************************************
* Global variables
*****************************************************************************/

/*
 * Ptask queue
 */
struct t_queue Ptask_queue;

/*
 * Ptask Unique IDs
 */
int   Ptask_ids = 0;

/*
 * Preemption overhead variables
 */
t_boolean PREEMP_initialized = FALSE;
t_boolean PREEMP_enabled;
int       PREEMP_cycle;
t_nano    PREEMP_time;
int      *PREEMP_table; /* Array to manage preemption cycles for each task */

/*
 * Synthetic burst generation variables
 */
t_boolean      synthetic_bursts = FALSE;
struct t_queue burst_categories;

/*
 * Private functions
 */
void new_task_in_Ptask(struct t_Ptask *Ptask, int taskid, int nodeid);

void TASK_add_thread_to_task (struct t_task *task, int threadid);
void TASK_Delete_Task (struct t_task *task);

void TASK_module_new_general (unsigned long int module_type,
                              unsigned long int module_value,
                              double        ratio,
                              double        const_burst_duration);

void TASK_Initialize_Ptask_Mapping(struct t_Ptask *Ptask);


int *TASK_Map_Filling_Nodes(int task_count);
void Update_Node_Info(struct t_task *tasks, int task_count, int * task_mapping);
int *TASK_Map_N_Tasks_Per_Node(int n_tasks_per_node);
int *TASK_Map_Interleaved(int task_count);

/*****************************************************************************
* Public functions implementation
*****************************************************************************/

/*
 * This function initializes the applications (Ptasks) and also their tasks
 */
void TASK_Init(int sintetic_io_applications)
{
  struct t_Ptask        *Ptask;
  struct t_queue        *comms_queue;
  struct t_communicator *comm;

  struct ptask_structure *ptask_structure;

  struct t_task   *task;
  struct t_thread *thread;
  struct t_action *new_action;

  size_t tasks_it, threads_it;

  /* Add predefined global ops identificators */
  add_global_ops();

  if (sintetic_io_applications > 0)
  {
    /* Initialize synthetic applications */
    create_sintetic_applications(sintetic_io_applications);
  }

  // For multiple tasks, first do the IO initializations, then get the "next actions"
  // Otherwise, the IO streams cannot be shared among the apps.
  for (Ptask  = (struct t_Ptask *) head_queue (&Ptask_queue);
       Ptask != P_NIL;
       Ptask  = (struct t_Ptask *) next_queue (&Ptask_queue))
  {
    if (debug)
    {
      printf ("* Loading Ptask %02d", Ptask->Ptaskid);
    }

    if (!DATA_ACCESS_init(Ptask->Ptaskid, Ptask->tracefile))
    {
      die("Error accessing trace (%s): %s",
          Ptask->tracefile,
          DATA_ACCESS_get_error());
    }
    /*
     * Initalize ptask threads
     */

    if (!DATA_ACCESS_get_ptask_structure(Ptask->Ptaskid, &ptask_structure))
    {
      die("Error retrieving application %d information from trace: %s",
          Ptask->Ptaskid,
          DATA_ACCESS_get_error());
    }

    if (Ptask->map_definition != MAP_NO_PREDEFINED)
    { // We must initialize the tasks mapping here
      Ptask->tasks_count = ptask_structure->tasks_count;      
      TASK_Initialize_Ptask_Mapping(Ptask);
    }
    else
    {
      if (debug)
      {
        printf(" (Using task mapping present on the configuration file)\n");
      }
    }

    if (ptask_structure->tasks_count != Ptask->tasks_count)
    {
      die("Different number of tasks defined on configuration file (%d) than the present in trace '%s' (%d)\n",
          Ptask->tasks_count,
          Ptask->tracefile,
          ptask_structure->tasks_count);
    }

    for (tasks_it = 0; tasks_it < Ptask->tasks_count; tasks_it++)
    {
      task = &(Ptask->tasks[tasks_it]);
      /* Allocate memory for the task threads */
      task->threads_count = ptask_structure->threads_per_task[tasks_it];

      Simulator.threads_count += task->threads_count;

      task->threads = (struct t_thread**)
        malloc(task->threads_count*sizeof(struct t_thread*));

      for (threads_it = 0;
           threads_it < task->threads_count;
           threads_it++)
      {
        TASK_add_thread_to_task(task, threads_it);
      }
    }
    /*
     * Initialize application communicators
     */
    if (!DATA_ACCESS_get_communicators(Ptask->Ptaskid, &comms_queue))
    {
      die("Error retrieving communicators information from trace: %s", DATA_ACCESS_get_error());
    }

    for (comm  = (struct t_communicator*) head_queue (comms_queue);
         comm != COM_NIL;
         comm  = (struct t_communicator*) next_queue(comms_queue))
    {
      insert_queue(&Ptask->Communicator,
                   (char*) comm,
                   (t_priority)comm->communicator_id);
    }

  } // end of loading the Ptasks.

  // Load "next" action for each thread of each task of each Ptask.
  for (Ptask  = (struct t_Ptask *) head_queue (&Ptask_queue);
       Ptask != P_NIL;
       Ptask  = (struct t_Ptask *) next_queue (&Ptask_queue))
  {

    for (tasks_it = 0; tasks_it < Ptask->tasks_count; tasks_it++)
    {
      task = &(Ptask->tasks[tasks_it]);
      for (threads_it = 0; threads_it < task->threads_count; threads_it++ )
      {
        thread = task->threads[threads_it];
        READ_get_next_action(thread);
      }
    }
  }
}

void TASK_End()
{
  struct t_Ptask  *Ptask;
  struct t_task   *task;
  struct t_thread *thread;
  struct t_node   *node;
  struct t_cpu    *cpu;
  struct t_send   *mess_source;
  struct t_action *action;
  struct t_recv   *mess;
  struct t_communicator *communicator;
  struct t_global_op_definition * glop;


  for(Ptask  = (struct t_Ptask *) head_queue (&Ptask_queue);
      Ptask != P_NIL;
      Ptask  = (struct t_Ptask *) next_queue (&Ptask_queue))
  {
    for (communicator  = (struct t_communicator *)head_queue(&Ptask->Communicator);
         communicator != (struct t_communicator *)0;
         communicator  = (struct t_communicator *)next_queue(&Ptask->Communicator))
    {
      /* JGG (07/07/2010): Easy way to check if it is a global operation in flight */
      /* if (count_queue(&communicator->threads)!=0) */
      if (communicator->in_flight_op == TRUE)
      {
        for(thread  = (struct t_thread *)head_queue(&communicator->threads);
            thread != (struct t_thread *)0;
            thread  = (struct t_thread *)next_queue(&communicator->threads))
        {
          glop = (struct t_global_op_definition *)
            query_prio_queue (&Global_op,
                              (t_priority)thread->action->desc.
                              global_op.glop_id);
          warning("Some task P%02d T%02d (t%02d) waiting for global operation %s\n",
                  IDENTIFIERS(thread),
                  glop->name);
        }
      }
    }
    size_t i;
    for(i = 0; i < Ptask->tasks_count; i++)
    {
      task = &(Ptask->tasks[i]);
      /* DELETE ALL MEMORY STRUCTURES */

      TASK_Delete_Task(task);
    }

    free(Ptask->tasks);
  }

  DATA_ACCESS_end();
}
/*
 * Create a new Ptask. Ptask is a queue whose elements are tasks
 */
// struct t_Ptask* create_Ptask (char *tracefile, char *configfile)
void TASK_New_Ptask(char *trace_name,
                    int   tasks_count,
                    int  *tasks_mapping)
{
  struct t_Ptask *Ptask;
  size_t          new_taskid;
  int 						i;

  Ptask = (struct t_Ptask*) malloc (sizeof (struct t_Ptask));
  Ptask->Ptaskid    = Ptask_ids++;

  Ptask->tracefile  = strdup(trace_name);

  /* Not used anywhere
  Ptask->configfile = configfile;
  */
  Ptask->map_definition = MAP_NO_PREDEFINED;

  Ptask->n_rerun    = 0;
//mmap: changing to mmap
//   Ptask->file = (FILE *) NULL;
  Ptask->mmapped_file          = (char*) NULL;
  Ptask->mmap_position         = 0;
  Ptask->synthetic_application = FALSE;

  // create_queue (&(Ptask->tasks));

  create_queue (&(Ptask->global_operation));
  create_queue (&(Ptask->Communicator));
  create_queue (&(Ptask->Window));
  create_queue (&(Ptask->MPI_IO_fh_to_commid));
  create_queue (&(Ptask->MPI_IO_request_thread));
  // create_queue (&(Ptask->Modules));
  create_modules_map(&(Ptask->Modules));
  create_queue (&(Ptask->Filesd));
  create_queue (&(Ptask->UserEventsInfo));

  /* Task creation */
  if (trace_name == NULL)
  {
    /* It is a synthetic application */
    /* JGG (2012/01/16): a synthetic application will use all nodes with a
     * linear mapping! */

    int synth_task;

#ifdef USE_EQUEUE
    Ptask->tasks_count = count_Equeue(&Node_queue);
#else
    Ptask->tasks_count = count_queue(&Node_queue);
#endif

    Ptask->tasks = (struct t_task*) malloc(Ptask->tasks_count*sizeof(struct t_task));

    for (synth_task = 0; synth_task < Ptask->tasks_count; synth_task++)
    {
      TASK_New_Task(Ptask, synth_task, /*synth_task,*/ FALSE);
    }
  }
  else
  {
    Ptask->tasks_count = tasks_count;
    Ptask->tasks       = (struct t_task*) malloc(Ptask->tasks_count*sizeof(struct t_task));

    /*	gets accelerator tasks info for mapping	*/
    get_acc_tasks_info(Ptask);

    for (new_taskid = 0; new_taskid < tasks_count; new_taskid++)
    {	/*	find if any accelerator task is not going to map to an accelerator node	*/
			for (i = 0; i < Ptask->acc_tasks_count && Ptask->acc_tasks != NULL; i++)
			{
				if (Ptask->acc_tasks[i] == new_taskid)
				{
					TASK_New_Task(Ptask, new_taskid, /*tasks_mapping[new_taskid],*/ TRUE);
					break;
				}
			}
      if (i == Ptask->acc_tasks_count)
      	TASK_New_Task(Ptask, new_taskid, /*tasks_mapping[new_taskid],*/ FALSE);
    }
  }

  insert_queue(&Ptask_queue, (char*) Ptask, (t_priority) Ptask->Ptaskid);
}
/*
 * Create a new Ptask, using a predefined map information
 */
void TASK_New_Ptask_predefined_map(char* trace_name,
                                   int   map_definition,
                                   int   tasks_per_node)
{
  struct t_Ptask *Ptask;
  size_t          new_taskid;

  Ptask = (struct t_Ptask*) malloc (sizeof (struct t_Ptask));
  Ptask->Ptaskid    = Ptask_ids++;

  Ptask->tracefile  = strdup(trace_name);
  /* Unknown number of total tasks */
  Ptask->tasks_count    = 0;

  Ptask->map_definition = map_definition;
  Ptask->tasks_per_node = tasks_per_node;
  /* Not used anywhere
  Ptask->configfile = configfile;
  */
  Ptask->n_rerun    = 0;
//mmap: changing to mmap
//   Ptask->file = (FILE *) NULL;
  Ptask->mmapped_file          = (char*) NULL;
  Ptask->mmap_position         = 0;
  Ptask->synthetic_application = FALSE;

  Ptask->acc_tasks_count = -1;	//-1:	search for acc_tasks not done, >= 0 otherwise
  Ptask->acc_tasks = (int *) NULL;

  create_queue (&(Ptask->Communicator));
  create_queue (&(Ptask->Window));
  create_queue (&(Ptask->MPI_IO_fh_to_commid));
  create_queue (&(Ptask->MPI_IO_request_thread));
  create_modules_map(&(Ptask->Modules));
  create_queue (&(Ptask->Filesd));
  create_queue (&(Ptask->UserEventsInfo));

  insert_queue(&Ptask_queue, (char*) Ptask, (t_priority) Ptask->Ptaskid);
}
/**
 * This method is used when reading the configuration, so the task structure
 * is not fully defined. When the tracefile is read each task definition is
 * completed with the rest of information (threads count, communicators, etc.)
 */
//void TASK_New_Task(struct t_Ptask *Ptask, int taskid, /*int nodeid, */t_boolean acc_task)
void TASK_New_Task(struct t_Ptask *Ptask, int taskid, t_boolean acc_task)
{
  struct t_task *task;
  struct t_node *node;
  int nodeid;
  int  i;
  struct t_link *link;
  
  node = get_node_by_id(nodeid);
  if (NODE_get_acc_node(node) < Ptask->acc_tasks_count)// && !NODE_get_acc(node))
  { /*  when mapping a task with accelerator (indicated in Dimemas header)
     *  in a non-accelerator node (indicated in configuration file)
     */
    die("Cannot map %d accelerator tasks in %d accelerator nodes \n PLEASE CHECK THE CONFIGURATIOIN", 
         Ptask->acc_tasks_count, NODE_get_acc_node(node));
  }

  task = &(Ptask->tasks[taskid]);
  task->taskid    = taskid;
  task->nodeid    = nodeid;
  task->Ptask     = Ptask;
  task->accelerator = acc_task;
  task->io_thread = FALSE;
  task->threads_count = 0;
  task->threads       = NULL;

  create_queue (&(task->mess_recv));
  create_queue (&(task->recv));
  create_queue (&(task->send));
  create_queue (&(task->recv_without_send));
  create_queue (&(task->send_without_recv));
  create_queue (&(task->irecvs_executed));

/********************************/
/* Rest of the function is defined in Update_Node_Info () 
*********************************/
  create_queue (&(task->busy_in_links));
  create_queue (&(task->busy_out_links));
  create_queue (&(task->th_for_in));
  create_queue (&(task->th_for_out));

  task->KernelSync = TH_NIL;	//Means no thread is waiting
  task->HostSync = TH_NIL;	//Means no root is waiting
  task->KernelByComm	= -1;	//Means no root is waiting for kernel

  return;
}

void TASK_Delete_Task(struct t_task *task)
{
  struct t_thread *thread;
  size_t i;

  /* Delete all threads on the task */
  for (i = 0; i < task->threads_count; i++)
  {
    thread = task->threads[i];

    free(thread);
  }

  return;
}

/* Synthetic burst generation functions */
void SYNT_BURST_add_new_burst_category(int    burst_category_id,
                                       double burst_category_mean,
                                       double burst_category_std_dev)
{
  burst_category_t new_category;

  if (!synthetic_bursts)
  {
    synthetic_bursts = TRUE;
    create_queue(&(burst_categories));
  }

  new_category = (burst_category_t) malloc(sizeof(struct _burst_category));

  if (new_category == NULL)
  {
    panic("Unable to allocate memory to burst category structure");
  }

  new_category->id                      = burst_category_id;
  new_category->values.distribution     = NORMAL_DISTRIBUTION;
  new_category->values.parameters.normal.mean  = burst_category_mean;
  new_category->values.parameters.normal.stdev = burst_category_std_dev;

  insert_queue(&(burst_categories),
               (char*) new_category,
               (t_priority) burst_category_id);

  return;
}

double SYNT_BURST_get_burst_value( int burst_category_id)
{
  burst_category_t burst_category;

  burst_category =
    (burst_category_t) query_prio_queue(&(burst_categories),
                                        (t_priority) burst_category_id);

  if (burst_category == NULL)
    return 0.0;

  return RANDOM_GenerateRandom(&(burst_category->values));
}

/* Preemption overhead functions */
void PREEMP_init(struct t_Ptask *Ptask)
{
  size_t task_num = Ptask->tasks_count;

  PREEMP_table = (int*) malloc (task_num * sizeof(int));

  srand((unsigned int) time(NULL));

  size_t  i;
  for (i = 0; i < task_num; i++)
  {
    PREEMP_table[i] = rand();
  }
}

t_nano PREEMP_overhead(struct t_task* task)
{
  /* JGG: tasks id minus 1 */
  PREEMP_table[task->taskid]++;

  if ((PREEMP_table[task->taskid] % PREEMP_cycle) == 0)
  {
    if (debug)
    {
      PRINT_TIMER(current_time);
      printf(": Task %02d Preemption\n", task->taskid);
    }
    return PREEMP_time;
  }
  else
  {
    return (t_nano) 0;
  }
}

/*
void new_communicator_definition (struct t_Ptask *Ptask, int communicator_id)
{
  register struct t_communicator *comm;
  comm = (struct t_communicator *)query_prio_queue (&Ptask->Communicator,
      (t_priority)communicator_id);
  if (comm!=(struct t_communicator *)0)
  {
      panic("Redefinition of communicator %d for P%d\n",
            communicator_id,
            Ptask->Ptaskid);
  }

  comm = (struct t_communicator *)malloc(sizeof(struct t_communicator));

  comm->communicator_id = communicator_id;
  comm->size            = 0;
  comm->global_ranks    = NULL;

  create_queue (&comm->threads);
  create_queue (&comm->machines_threads);
  create_queue (&comm->m_threads_with_links);

  create_queue (&comm->nonblock_global_op_threads);
  create_queue (&comm->nonblock_global_op_machine_threads);
  create_queue (&comm->nonblock_m_threads_with_links);
  create_queue (&comm->nonblock_current_root);

  comm->current_root = TH_NIL;
  comm->in_flight_op = FALSE;

  insert_queue (&Ptask->Communicator, (char *)comm, (t_priority)communicator_id);
}
*/

void
new_window_definition (struct t_Ptask *Ptask, int window_id)
{
  register struct t_window *win;

  win = (struct t_window *)query_prio_queue (&Ptask->Window,
      (t_priority)window_id);
  if (win!=(struct t_window *)0)
  {
      panic("Redefinition of window %d for P%d\n",
            window_id,
            Ptask->Ptaskid);
  }

  win = (struct t_window*) malloc(sizeof(struct t_window));
  win->window_id = window_id;
  win->mode = WINDOW_MODE_NONE;
  create_queue (&win->global_ranks);
  create_queue (&win->fence_tasks);
  create_queue (&win->fence_operations);

  win->task_with_lock = (struct t_task *)0;
  win->lock_mode = MPI_OS_LOCK_SHARED;
  create_queue (&win->threads_with_lock);
  create_queue (&win->threads_waiting_lock);
  create_queue (&win->threads_waiting_unlock);
  create_queue (&win->lock_operations);

  create_queue (&win->pending_of_post);
  create_queue (&win->pending_rma_to_complete);
  create_queue (&win->complete_done);
  create_queue (&win->pending_wait_to_complete);
  create_queue (&win->post_operations);
  create_queue (&win->post_done);
  create_queue (&win->start_done);

  insert_queue (&Ptask->Window, (char *)win, (t_priority)window_id);
}

void add_identificator_to_window(struct t_Ptask *Ptask, int window_id, int taskid)
{
  register struct t_window *win;
  int *mtaskid;
  size_t  i;

  win = (struct t_window *)query_prio_queue (&Ptask->Window,
                                            (t_priority)window_id);

  if (win==(struct t_window *)0)
  {
      panic("Unable to locate window %d for P%d\n",
            window_id,
            Ptask->Ptaskid);
  }

  if (taskid == -1)
  {
    for (i=0; i < Ptask->tasks_count; i++)
    {
      mtaskid = (int*) malloc(sizeof(int));
      *mtaskid = i+1;
      inFIFO_queue (&win->global_ranks, (char *)mtaskid);
    }
    return;
  }

  if (taskid > Ptask->tasks_count)
  {
    panic("Specified rank %d not valid in this Ptask\n", taskid);
  }

  mtaskid = (int*) malloc(sizeof(int));
  *mtaskid = taskid+1;
  inFIFO_queue (&win->global_ranks, (char *)mtaskid);
}

void no_more_identificator_to_window(struct t_Ptask *Ptask, int window_id)
{
  register struct t_window *win;
  win = (struct t_window *)query_prio_queue (&Ptask->Window,
      (t_priority)window_id);

  if (win==(struct t_window *)0)
  {
    panic("Unable to locate window %d for P%d\n",
          window_id,
          Ptask->Ptaskid);
  }
}
/*
 * Set to 0 an account entry
 */
void clear_account (struct t_account *account)
{
  /* FEC: Hi havia problemes perque no tots els camps havien estat inicialitzats
   * Per tant, el primer que faig es posar-ho tot a zero. */
  bzero(account,sizeof(struct t_account));

  /* FEC: Es segueixen inicialitzant manualment alguns camps */
  ASS_TIMER (account->initial_time, DBL_MAX);
  ASS_TIMER (account->final_time, 0);
  account->nodeid = 0;

  ASS_TIMER (account->cpu_time, 0);
  account->n_th_in_run = 0;
  account->n_preempt_to_me = 0;
  account->n_preempt_to_other = 0;
  ASS_TIMER (account->time_ready_without_cpu, 0);

  account->n_sends = 0;
  account->n_bytes_send = 0;
  account->n_recvs = 0;
  account->n_bytes_recv = 0;
  account->n_recvs_on_processor = 0;
  account->n_recvs_must_wait = 0;
  ASS_TIMER (account->time_waiting_for_message, 0);

  ASS_TIMER (account->block_due_link, 0);
  ASS_TIMER (account->block_due_buses, 0);
}

/*
 * New and local create account
 */
struct t_account* new_accounter()
{
  struct t_account *res;

  res = (struct t_account*) malloc (sizeof (struct t_account));
  clear_account (res);
  return (res);
}
/*
 * Create new account entry Initializes to 0
 */
void new_account (struct t_queue *acc, int nodeid)
{
  struct t_account *new_acc;

  new_acc = (struct t_account*) malloc (sizeof (struct t_account));
  clear_account (new_acc);
  new_acc->nodeid = nodeid;
  new_acc->iteration = count_queue (acc) + 1;
  new_acc->initial_time = current_time;
  inFIFO_queue (acc, (char *) new_acc);
}
/*
 * Sumarize from account to account
 */
void add_account (struct t_account *to, struct t_account *from)
{
  MIN_TIMER (to->initial_time, from->initial_time, to->initial_time);
  MAX_TIMER (to->final_time, from->final_time, to->final_time);

  to->n_group_operations += from->n_group_operations;
  ADD_TIMER (to->block_due_group_operations,
             from->block_due_group_operations,
             to->block_due_group_operations);

  ADD_TIMER (to->group_operations_time,
             from->group_operations_time,
             to->group_operations_time);

  ADD_TIMER (to->cpu_time, from->cpu_time, to->cpu_time);
  to->n_th_in_run        += from->n_th_in_run;
  to->n_preempt_to_me    += from->n_preempt_to_me;
  to->n_preempt_to_other += from->n_preempt_to_other;
  ADD_TIMER (to->time_ready_without_cpu,
             from->time_ready_without_cpu,
             to->time_ready_without_cpu);

  to->n_sends += from->n_sends;
  to->n_bytes_send += from->n_bytes_send;

  to->n_recvs              += from->n_recvs;
  to->n_bytes_recv         += from->n_bytes_recv;
  to->n_recvs_on_processor += from->n_recvs_on_processor;
  to->n_recvs_must_wait    += from->n_recvs_must_wait;

  ADD_TIMER (to->time_waiting_for_message,
             from->time_waiting_for_message,
             to->time_waiting_for_message);
  ADD_TIMER (to->latency_time, from->latency_time, to->latency_time);
  ADD_TIMER (to->block_due_resources,
             from->block_due_resources,
             to->block_due_resources);

  ADD_TIMER (to->block_due_link, from->block_due_link, to->block_due_link);
  ADD_TIMER (to->block_due_buses, from->block_due_buses, to->block_due_buses);

}

void min_account (struct t_account *to, struct t_account *from)
{
  to->n_group_operations =
    MIN(to->n_group_operations, from->n_group_operations);

  MIN_TIMER(to->block_due_group_operations,
            from->block_due_group_operations,
            to->block_due_group_operations);
  MIN_TIMER(to->group_operations_time,
            from->group_operations_time,
            to->group_operations_time);

  MIN_TIMER(to->cpu_time, from->cpu_time, to->cpu_time);
  MIN_TIMER(to->latency_time, from->latency_time, to->latency_time);
  MIN_TIMER(to->block_due_resources,
            from->block_due_resources,
            to->block_due_resources);
  to->n_th_in_run = MIN(from->n_th_in_run,to->n_th_in_run);

  to->n_sends = MIN (to->n_sends, from->n_sends);
  to->n_bytes_send = MIN (to->n_bytes_send, from->n_bytes_send);

  to->n_recvs = MIN (to->n_recvs, from->n_recvs);
  to->n_bytes_recv = MIN (to->n_bytes_recv, from->n_bytes_recv);
  to->n_recvs_on_processor =
    MIN (to->n_recvs_on_processor, from->n_recvs_on_processor);

  to->n_recvs_must_wait = MIN (to->n_recvs_must_wait, from->n_recvs_must_wait);

  MIN_TIMER(to->time_waiting_for_message,
            from->time_waiting_for_message,
            to->time_waiting_for_message);

  MIN_TIMER(to->time_ready_without_cpu,
            from->time_ready_without_cpu,
            to->time_ready_without_cpu);

  MIN_TIMER(to->block_due_link, from->block_due_link, to->block_due_link);
  MIN_TIMER(to->block_due_buses, from->block_due_buses, to->block_due_buses);

}

void max_account (struct t_account *to, struct t_account *from)
{
  to->n_group_operations =
    MAX(to->n_group_operations, from->n_group_operations);

  MAX_TIMER(to->block_due_group_operations,
            from->block_due_group_operations,
            to->block_due_group_operations);
  MAX_TIMER(to->group_operations_time,
            from->group_operations_time,
            to->group_operations_time);

  MAX_TIMER(to->cpu_time, from->cpu_time, to->cpu_time);
  MAX_TIMER(to->latency_time, from->latency_time, to->latency_time);
  MAX_TIMER(to->block_due_resources,
            from->block_due_resources,
            to->block_due_resources);

  to->n_th_in_run = MAX(from->n_th_in_run,to->n_th_in_run);

  to->n_sends = MAX (to->n_sends, from->n_sends);
  to->n_bytes_send = MAX (to->n_bytes_send, from->n_bytes_send);

  to->n_recvs = MAX (to->n_recvs, from->n_recvs);
  to->n_bytes_recv = MAX (to->n_bytes_recv, from->n_bytes_recv);
  to->n_recvs_on_processor =
    MAX (to->n_recvs_on_processor, from->n_recvs_on_processor);
  to->n_recvs_must_wait = MAX (to->n_recvs_must_wait, from->n_recvs_must_wait);
  MAX_TIMER(to->time_waiting_for_message,
            from->time_waiting_for_message,
            to->time_waiting_for_message);
  MAX_TIMER(to->time_ready_without_cpu,
            from->time_ready_without_cpu,
            to->time_ready_without_cpu);

  MAX_TIMER (to->block_due_link, from->block_due_link, to->block_due_link);
  MAX_TIMER (to->block_due_buses, from->block_due_buses, to->block_due_buses);
}

/*
 * Create a thread and put in on the queue in the task
   Vladimir: this is used for hardcoding
   Dimemas expects 1 thread per task
   and I let it start thinking that it is going to be that way
   but when I read trf file I can see how many threads there is
   and then I augment this number
 */
void TASK_add_thread_to_task (struct t_task *task, int thread_id)
{
  struct t_thread *thread;

  if (get_node_by_id (task->nodeid) == N_NIL)
  {
    panic("Incorrect node identifier %d in  T%d\n",
          task->nodeid,
          task->taskid);
  }

  /* Insert new thread in task */
  thread = (struct t_thread*) malloc (sizeof (struct t_thread));

  create_queue (&(thread->account));
  create_queue (&(thread->modules));
  create_queue (&(thread->Activity));

  thread->threadid                 = thread_id;
  thread->task                     = task;
  thread->put_into_ready           = current_time;
  thread->action                   = AC_NIL;
  thread->original_thread          = TRUE;
  thread->twin_thread              = TH_NIL;
  thread->doing_context_switch     = FALSE;
  thread->min_time_to_be_preempted = current_time;
  thread->doing_busy_wait          = FALSE;
  thread->last_action              = AC_NIL;
  thread->local_link               = L_NIL;
  thread->partner_link             = L_NIL;
  thread->local_hd_link            = L_NIL;
  thread->partner_hd_link          = L_NIL;
  // thread->in_mem_link              = L_NIL;
  // thread->out_mem_link             = L_NIL;
  thread->partner_node             = N_NIL;
  thread->doing_startup            = FALSE;
  thread->startup_done             = FALSE;
  thread->doing_copy               = FALSE;
  thread->copy_done                = FALSE; /* JGG New latency modelling */
  thread->doing_roundtrip          = FALSE;
  thread->roundtrip_done           = FALSE;
  thread->loose_cpu                = TRUE;
  thread->idle_block               = FALSE;
  thread->last_paraver             = current_time;
  thread->size_port                = 0;
  thread->port_send_link           = L_NIL;
  thread->port_recv_link           = L_NIL;
  thread->to_module                = 0;
  thread->port                     = PO_NIL;
  thread->copy_segment_link_source = L_NIL;
  thread->copy_segment_link_dest   = L_NIL;
  thread->copy_segment_size        = 0;
  thread->original_seek            = 0;
  thread->seek_position            = 0;
  thread->base_priority            = 0;

  thread->sstask_id                = 0;
  thread->sstask_type              = 0;

  thread->portid                   = port_ids++;
  PORT_create (thread->portid, thread); 

  thread->portid                   = port_ids++;
  PORT_create (thread->portid, thread);

  thread->portid                   = port_ids++;
  PORT_create (thread->portid, thread);

  /* making these queues separate for every thread
     only DEPENDENCIES go to this queues
     REAL MPI TRANSFERS go the the queues of the task */
  create_queue (&(thread->mess_recv));
  create_queue (&(thread->recv));
  create_queue (&(thread->send));
  /* FEC: Cal crear les dues noves cues pels Irecv*/
  create_queue (&(thread->recv_without_send));
  create_queue (&(thread->send_without_recv));
  create_queue (&(thread->irecvs_executed));

  // Initialization of queue of operations to be ignored if a deadlock arises
  // It is best to init them one time per execution than check if it is active 
  // every time that we are reading an action.
  //if (with_deadlock_analysis)
  thread->counter_ops_already_ignored = 0;
  thread->counter_ops_already_injected = 0;
  create_queue(&thread->ops_to_be_ignored);
  create_queue(&thread->ops_to_be_injected);


  new_account (&(thread->account), task->nodeid);

  thread->last_cp_node = (struct t_cp_node *)0;
  thread->global_op_done = FALSE;

  ASS_ALL_TIMER (thread->last_time_event_number,0);

  thread->marked_for_deletion = 0;
// thread->file_shared         = FALSE;

  /* JGG (2012/01/12): thread queue not needed anymore */
  // inFIFO_queue (&(task->threads), (char *) thread);
  /* and store it in the array of all threads in that task */
  assert(task->threads_count >= thread->threadid);
  task->threads[thread->threadid] = thread;

  /* Accelerator variables */
  if (task->accelerator && task->threads_count == thread_id + 1)
	{	/*	Kernel thread in accelerator task it's always last	*/
		thread->kernel = TRUE;
		thread->host	 = FALSE;
	}
	else if (task->accelerator && thread_id == 0)
	{	/*	It's not an accelerator task	*/
		thread->kernel = FALSE;
		thread->host	 = TRUE;
	}
	else
	{
		thread->kernel = FALSE;
		thread->host	 = FALSE;
	}

	thread->accelerator_link		= L_NIL;
	thread->first_acc_event_read		= FALSE;
	thread->acc_recv_sync			= FALSE;
	thread->acc_sndr_sync 			= FALSE;
	thread->doing_acc_comm			= FALSE;
	thread->acc_in_block_event.type = 0;
	thread->acc_in_block_event.value= 0;
	thread->acc_in_block_event.paraver_time = (dimemas_timer) 0;
	thread->blckd_in_global_op = FALSE;
	/* Accelerator variables */

    /* NON-Block global operations variables */
    thread->n_nonblock_glob_in_flight = 0;
    thread->n_nonblock_glob_waiting = 0;
    thread->n_nonblock_glob_done = 0;
    thread->nb_glob_index = 0;
    thread->nb_glob_index_master = 0;

    create_queue(&thread->nonblock_glop_done_threads);

}

struct t_thread *locate_thread_of_task (struct t_task *task, int thid)
{
  register struct t_thread *thread;

  assert(thid >= 0 && thid < task->threads_count);

  thread = task->threads[thid];

  assert (thread != NULL);
  assert (thread->threadid == thid);
  return (thread);
}

struct t_thread* locate_thread (struct t_Ptask *Ptask, int taskid, int thid)
{
  struct t_task   *task;
  struct t_thread *thread;

  if (taskid < 0 || taskid >= Ptask->tasks_count)
  {
    panic ("Incorrect task identifier %d in trace file %s for P%02d in locate_thread\n",
           taskid,
           Ptask->tracefile,
           Ptask->Ptaskid);
  }

  task = &(Ptask->tasks[taskid]);

  // this is optimized by indexing threads in tasks
  thread = locate_thread_of_task (task, thid);

  if (thread == TH_NIL)
    panic("Incorrect thread identifier %d in trace file %s for P%d T%d\n",
          thid,
          Ptask->tracefile,
          Ptask->Ptaskid,
          taskid);

  return (thread);
}
/*
 * Append an action to action list of specific thread.
 *
 * NOTE (2014/03/07): There are no references to this function around the source
 * code!
 *
 */
void new_action_to_thread (struct t_Ptask *Ptask,
                           int taskid,
                           int thid,
                           struct t_action *action)
{
  register struct t_thread *thread;
  struct t_action *last_action;

  thread = locate_thread (Ptask, taskid, thid);
  if (thread == TH_NIL)
  {
    panic("Incorrect thread identifier %d in trace file %s for P%d T%d\n",
          thid,
          Ptask->tracefile,
          Ptask->Ptaskid,
          taskid);
  }

  last_action = thread->last_action;
  if (last_action == AC_NIL)
  {
    thread->action = action;
  }
  else
  {
    if (action->action == WORK)
    {
      recompute_work_upon_modules(thread, action);
    }

    if ((action->action == WORK) && (last_action->action == WORK))
    {
      last_action->desc.compute.cpu_time += action->desc.compute.cpu_time;
      READ_free_action(action);
      return;
    }
    last_action->next = action;
  }

  thread->last_action = action;
}

struct t_account* current_account(struct t_thread *thread)
{
  struct t_account *temp;
  temp = ((struct t_account*) tail_queue (&(thread->account)));

  return temp;
}
/*
 * Find task into Ptask with taskid
 */
struct t_task *locate_task (struct t_Ptask *Ptask, int taskid)
{
  if (taskid < 0 || taskid >= Ptask->tasks_count)
  {
    panic ("Incorrect task identifier %d in trace file %s for P%02d in locate_task\n",
           taskid,
           Ptask->tracefile,
           Ptask->Ptaskid);
  }

  return &(Ptask->tasks[taskid]);
}

struct t_thread *duplicate_thread_fs (struct t_thread *thread)
{
  struct t_thread *copy_thread;
  struct t_node    *node;
  struct t_machine *machine;

  node = get_node_of_thread (thread);
  machine = node->machine;

  copy_thread = (struct t_thread *) malloc (sizeof (struct t_thread));
  // bcopy (thread, copy_thread, sizeof(struct t_thread));
  memcpy(copy_thread, thread, sizeof(struct t_thread));

  copy_thread->original_thread = FALSE;
  copy_thread->twin_thread = thread;
  copy_thread->doing_context_switch = FALSE;
  copy_thread->min_time_to_be_preempted = thread->min_time_to_be_preempted;
  copy_thread->doing_busy_wait = FALSE;
  copy_thread->threadid = thread->threadid;
  copy_thread->task = thread->task;
  copy_thread->put_into_ready = thread->put_into_ready;
  copy_thread->last_action = AC_NIL;
  copy_thread->account = thread->account;
  copy_thread->local_link = thread->local_link;
  copy_thread->partner_link = thread->partner_link;
  copy_thread->local_hd_link = thread->local_hd_link;
  copy_thread->partner_hd_link = thread->partner_hd_link;
  copy_thread->last_paraver = thread->last_paraver;
  copy_thread->base_priority = thread->base_priority;
  (*SCH[machine->scheduler.policy].init_scheduler_parameters) (copy_thread);
  SCHEDULER_copy_parameters (thread, copy_thread);
  copy_thread->seek_position = SEEK_NIL;
  copy_thread->logical_send = thread->logical_send;
  copy_thread->logical_recv = thread->logical_recv;
  copy_thread->physical_send = thread->physical_send;
  copy_thread->physical_recv = thread->physical_recv;
  copy_thread->last_cp_node = thread->last_cp_node;

  copy_thread->sstask_id                = thread->sstask_id;
  copy_thread->sstask_type              = thread->sstask_type;

  /* Accelerator variables */
  copy_thread->host											= thread->host;
  copy_thread->kernel										= thread->kernel;
	copy_thread->accelerator_link					= thread->accelerator_link;
	copy_thread->first_acc_event_read			= thread->first_acc_event_read;
	copy_thread->acc_in_block_event       = thread->acc_in_block_event;
	copy_thread->acc_recv_sync						= thread->acc_recv_sync;
	copy_thread->acc_sndr_sync        		= thread->acc_sndr_sync;
	copy_thread->doing_acc_comm						= thread->doing_acc_comm;
	copy_thread->blckd_in_global_op				= thread->blckd_in_global_op;
	/* Accelerator variables */

  return (copy_thread);
}

void delete_duplicate_thread_fs (struct t_thread *thread)
{
  SCHEDULER_free_parameters (thread);
  free (thread);
}

struct t_thread *duplicate_thread (struct t_thread *thread)
{
  struct t_thread  *copy_thread;
  struct t_action  *action, *ac;
  struct t_node    *node;
  struct t_machine *machine;

  node = get_node_of_thread (thread);
  machine = node->machine;

  copy_thread = (struct t_thread *) malloc (sizeof (struct t_thread));
  // bcopy (thread, copy_thread, sizeof(struct t_thread));
  memcpy(copy_thread, thread, sizeof(struct t_thread));

  copy_thread->original_thread          = FALSE;
  copy_thread->twin_thread              = thread;
  copy_thread->doing_context_switch     = FALSE;
  copy_thread->min_time_to_be_preempted = thread->min_time_to_be_preempted;
  copy_thread->doing_busy_wait          = FALSE;
  copy_thread->threadid                 = thread->threadid;
  copy_thread->task                     = thread->task;
  copy_thread->put_into_ready           = thread->put_into_ready;
  copy_thread->last_action              = AC_NIL;
  copy_thread->local_link               = thread->local_link;
  copy_thread->partner_link             = thread->partner_link;
  copy_thread->local_hd_link            = thread->local_hd_link;
  copy_thread->partner_hd_link          = thread->partner_hd_link;

  copy_thread->last_paraver             = thread->last_paraver;
  copy_thread->base_priority            = thread->base_priority;
  copy_thread->sch_parameters           = A_NIL;
  copy_thread->seek_position            = SEEK_NIL;
  copy_thread->logical_send             = thread->logical_send;
  copy_thread->logical_recv             = thread->logical_recv;
  copy_thread->physical_send            = thread->physical_send;
  copy_thread->physical_recv            = thread->physical_recv;
  copy_thread->last_cp_node             = thread->last_cp_node;

  copy_thread->sstask_id                = thread->sstask_id;
  copy_thread->sstask_type              = thread->sstask_type;

  (*SCH[machine->scheduler.policy].init_scheduler_parameters) (copy_thread);
  SCHEDULER_copy_parameters (thread, copy_thread);

  READ_create_action(&copy_thread->action);
  READ_copy_action(thread->action, copy_thread->action);

  copy_thread->action->next = AC_NIL;

  // Intent de crear un nou accounting pel thread nou, que al destruir-se s'afegira al thread original.
  create_queue (&(copy_thread->account));
  new_account (&(copy_thread->account), node->nodeid);

  copy_thread->host                     = thread->host;
  copy_thread->kernel                   = thread->kernel;
  copy_thread->accelerator_link         = thread->accelerator_link;
  copy_thread->first_acc_event_read     = thread->first_acc_event_read;
  copy_thread->acc_in_block_event       = thread->acc_in_block_event;
  copy_thread->acc_recv_sync            = thread->acc_recv_sync;
  copy_thread->acc_sndr_sync            = thread->acc_sndr_sync;
  copy_thread->doing_acc_comm           = thread->doing_acc_comm;
  copy_thread->blckd_in_global_op       = thread->blckd_in_global_op;
  copy_thread->n_nonblock_glob_in_flight= thread->n_nonblock_glob_in_flight;

  return copy_thread;
}

// this is used for making MPI_Isend
struct t_thread *promote_to_original (struct t_thread *copy_thread, struct t_thread *thread)
{

   struct t_node    *node;
   struct t_machine *machine;

   node = get_node_of_thread (thread);
   machine = node->machine;

   copy_thread->original_thread          = TRUE;
   thread->original_thread               = FALSE;

   copy_thread->twin_thread              = NULL;
   thread->twin_thread                   = copy_thread;

   copy_thread->doing_context_switch     = thread->doing_context_switch;
   copy_thread->min_time_to_be_preempted = thread->min_time_to_be_preempted;
   copy_thread->doing_busy_wait          = thread->doing_busy_wait;
   copy_thread->threadid                 = thread->threadid;
   copy_thread->task                     = thread->task;
   copy_thread->put_into_ready           = thread->put_into_ready;
   copy_thread->last_action              = thread->last_action;
   copy_thread->account                  = thread->account;
   copy_thread->local_link               = thread->local_link;
   copy_thread->partner_link             = thread->partner_link;
   copy_thread->local_hd_link            = thread->local_hd_link;
   copy_thread->partner_hd_link          = thread->partner_hd_link;
   copy_thread->last_paraver             = thread->last_paraver;
   copy_thread->base_priority            = thread->base_priority;
   copy_thread->sch_parameters           = thread->sch_parameters;
   copy_thread->seek_position            = thread->seek_position;
   copy_thread->last_cp_node             = thread->last_cp_node;

//   (*SCH[machine->scheduler.policy].init_scheduler_parameters) (copy_thread);
  SCHEDULER_copy_parameters (thread, copy_thread);

  copy_thread->action = thread->action;

/* Intent de crear un nou accounting pel thread nou, que al destruir-se
 * s'afegira al thread original. */
  return (copy_thread);
}


struct t_thread *promote_to_original2 (struct t_thread *copy_thread, struct t_thread *thread)
{
//   struct t_thread *copy_thread;
  printf("\n copy_thread->action == %p and the next action is %p \n\n", copy_thread->action, copy_thread->action->next);

  struct t_thread *temp_thread;
  printf("step 1\n");
  temp_thread = thread;
   printf("step 2\n");
  *thread = *copy_thread;
   printf("step 3\n");
  *copy_thread = *temp_thread;
   printf("step 4\n");

  struct t_action *action, *ac;
  struct t_node    *node;
  struct t_machine *machine;

  node = get_node_of_thread (thread);
  machine = node->machine;

  printf("\n copy_thread->action == %p and the next action is %p \n\n", thread->action, thread->action->next);

  thread->logical_send                    = copy_thread->logical_send;
  thread->logical_recv                    = copy_thread->logical_recv;
  thread->physical_send                   = copy_thread->physical_send;
  thread->physical_recv                   = copy_thread->physical_recv;


  ac = copy_thread->action;

  // action = (struct t_action *) malloc (sizeof (struct t_action));
  READ_create_action(&action);

  memcpy (action, ac, sizeof(struct t_action));
  action->next = thread->action;
  thread->action = action;

  register struct t_account *account;
  account = current_account (thread);

  account = current_account (thread);
  printf ("Thread without account in promote to original P%d T%d t%d\n", IDENTIFIERS (thread));
#ifdef USE_EQUEUE
  node = (struct t_node *) query_prio_Equeue (&Node_queue,
            (t_priority) account->nodeid);
#else
  node = (struct t_node *) query_prio_queue (&Node_queue,
            (t_priority) account->nodeid);
#endif
  printf ("Unable to locate node in promote_to_original %d for P%d T%d t%d\n",
             account->nodeid, IDENTIFIERS (thread));

  printf("END PROMOTE_TO_ORIGINAL2 \n");

  /* Intent de crear un nou accounting pel thread nou, que al destruir-se
  * s'afegira al thread original. */
  return (thread);
}

void delete_duplicate_thread (struct t_thread *thread)
{
  struct t_thread *twin_thread;
  twin_thread = thread->twin_thread;
  struct t_account *acc_th_copia, *acc_th_original;

  SCHEDULER_free_parameters (thread);
  acc_th_copia = current_account (thread);
  acc_th_original = current_account (thread->twin_thread);
  add_account(acc_th_original, acc_th_copia);

  extract_from_queue(&(thread->account), (char*) acc_th_copia);

  free(acc_th_copia);

  READ_free_action(thread->action);

  free (thread);
}

static t_boolean more_actions_on_task (struct t_task  *task)
{
  struct t_thread *thread;
  size_t thread_it;

  for (thread_it = 0; thread_it < task->threads_count; thread_it++)
  {
    thread = task->threads[thread_it];

    if (thread->action != AC_NIL)
      return TRUE;

    if (events_for_thread (thread))
      return TRUE;
  }

  return FALSE;
}

t_boolean more_actions_on_Ptask (struct t_Ptask *Ptask)
{
  struct t_task  *task;
  size_t          tasks_it;

  for(tasks_it = 0; tasks_it < Ptask->tasks_count; tasks_it++)
  {
    task = &(Ptask->tasks[tasks_it]);
    if (more_actions_on_task (task))
    {
      return (TRUE);
    }
  }
  return (FALSE);
}

void clear_last_actions (struct t_Ptask *Ptask)
{
  struct t_task    *task;
  struct t_thread  *thread;
  struct t_node    *node;
  struct t_machine *machine;
  size_t            tasks_it, thread_it;

  for(tasks_it = 0; tasks_it < Ptask->tasks_count; tasks_it++)
  {
    task = &(Ptask->tasks[tasks_it]);

    for (thread_it = 0; thread_it < task->threads_count; thread_it++)
    {
      thread = task->threads[thread_it];
      assert(thread->last_action == AC_NIL);
      assert(thread->action == AC_NIL);
      thread->last_paraver  = current_time;
      thread->last_action   = AC_NIL;
      thread->seek_position = thread->original_seek;

      node = get_node_of_thread (thread);
      machine = node->machine;
      (SCH[machine->scheduler.policy].clear_parameters) (thread);
      new_account (&(thread->account), node->nodeid);
    }
  }
}

void get_operation (struct t_thread *thread, struct t_fs_op *fs_op)
{
  struct t_action *action;

  action = thread->action;
  if (action != AC_NIL)
  {
    *fs_op = action->desc.fs_op;
    thread->action = action->next;
    READ_free_action(action);
  }
}

void file_name (struct t_Ptask *Ptask, int file_id, char *location)
{
  register struct t_filed *filed;

  filed =
    (struct t_filed*) query_prio_queue (&Ptask->Filesd, (t_priority)file_id);

  if (filed == F_NIL)
  {
    filed = (struct t_filed*) malloc (sizeof(struct t_filed));
    filed->file_id = file_id;
    filed->location = location;
    insert_queue (&Ptask->Filesd, (char *)filed, (t_priority) file_id);
  }
  else
  {
    printf ("Warning: redefinition of file %d\n",file_id);
  }
}

t_nano work_time_for_sintetic ()
{
  t_nano f;

  f = (t_nano) unform (MIN_SERVICE_TIME,
                        MAX_SERVICE_TIME);
  return (f);
}

void First_action_to_sintetic_application (struct t_Ptask *Ptask)
{
  struct t_task   *task;
  struct t_thread *thread;
  struct t_action *action;

  size_t tasks_it, threads_it;

  for(tasks_it = 0; tasks_it < Ptask->tasks_count; tasks_it++)
  {
    task = &(Ptask->tasks[tasks_it]);
    for (threads_it = 0; threads_it < task->threads_count; threads_it++)
    {
      thread = task->threads[threads_it];

      // action = (struct t_action *) malloc (sizeof (struct t_action));

      READ_create_action(&action);

      action->action = WORK;
      (action->desc).compute.cpu_time = work_time_for_sintetic ();
      (action->desc).compute.cpu_time = (t_nano) unform ((float) 0,
                                                          MAX_SERVICE_TIME);
      thread->action = action;
      thread->seek_position = 0;
    }
  }
}

void create_sintetic_applications (int num)
{
  int             i, j;
  struct t_Ptask *Ptask;

  for (i = 0; i < num; i++)
  {
    TASK_New_Ptask(NULL, 0, 0); // NULL -> implies creation of synthetic applications
    First_action_to_sintetic_application (Ptask);
    // inFIFO_queue (&Ptask_queue, (char *) Ptask);
  }
}
t_boolean more_actions_to_sintetic (struct t_thread *thread)
{
  struct t_action *action;
  struct t_Ptask *P;
  int             i = 0;
  int             sin = 0;

  if (thread->action != AC_NIL)
    return (TRUE);

  for(P  = (struct t_Ptask *) head_queue (&Ptask_queue);
      P != P_NIL;
      P  = (struct t_Ptask *) next_queue (&Ptask_queue))
  {
    if (P->synthetic_application)
    {
      sin++;
      continue;
    }
    if (P->n_rerun < reload_limit)
      break;
    i++;
  }

  if ((thread->seek_position == 0) && (i == count_queue (&(Ptask_queue)) - sin))
    return (FALSE);

  // action = (struct t_action *) malloc (sizeof (struct t_action));
  READ_create_action(&action);

  if (thread->seek_position == -1)
  {
    action->action = WORK;
    action->next = AC_NIL;
    (action->desc).compute.cpu_time = work_time_for_sintetic ();
    thread->action = action;
    thread->seek_position = 0;
  }
  else
  {
    action->action = FS;
    action->next = AC_NIL;
    thread->action = action;
    thread->seek_position = -1;
  }
  return (TRUE);
}

struct t_node *get_node_for_task_by_name (struct t_Ptask *Ptask, int taskid)
{
  struct t_task  *task;
  struct t_node  *node;

  if (taskid < 0 || taskid >= Ptask->tasks_count)
  {
    panic ("Incorrect task identifier %d in trace file %s for P%02d in get_node_for_task_by_name\n",
           taskid,
           Ptask->tracefile,
           Ptask->Ptaskid);
  }

  task = &(Ptask->tasks[taskid]);   
  node = get_node_of_task(task);

  return (node);
}

void TASK_module_new_ratio (unsigned long int module_type,
                            unsigned long int module_value,
                            double ratio)
{
  TASK_module_new_general(module_type,
                          module_value,
                          ratio,
                          (double) -1);
}

void TASK_module_new_duration (unsigned long int module_type,
                               unsigned long int module_value,
                               double const_burst_duration)
{
  TASK_module_new_general(module_type,
                          module_value,
                          (double) -1,
                          const_burst_duration);

  return;
}

void TASK_module_new_general (unsigned long int module_type,
                              unsigned long int module_value,
                              double            ratio,
                              double            const_burst_duration)
{
  unsigned long int module_type_ll, module_value_ll;

  struct t_Ptask  *current_Ptask;
  struct t_module *mod;

  module_type_ll  = module_type;
  module_value_ll = module_value;

  if (Ptask_ids == 0)
  {
    fprintf(stderr,
            "   * Warning: No applications defined when adding module [%lu:%lu]. Module not added\n",
            module_value_ll,
            module_type_ll);

    fprintf(stderr, "   * Please guarantee order ('mapping_information' -> 'modules information') in configuration file\n");
    return;
  }

  /*
   * Last Ptask is Ptasks_ids - 1
   */
  current_Ptask = (struct t_Ptask*) query_prio_queue(&Ptask_queue,
                                                     (t_priority) Ptask_ids-1);

  if (current_Ptask == NULL)
  {
    fprintf(stderr,
            "   * Warning: No applications defined when adding module [%lu:%lu]. Module not added\n",
            module_value_ll,
            module_type_ll);
    fprintf(stderr, "   * Please guarantee order ('mapping_information' -> 'modules information') in configuration file\n");
    return;
  }

  if (module_value == 0)
  {
    fprintf(stderr,
            "   * Unable to add new module [%lu] with value 0\n",
            module_type_ll);
    return;
  }

  if (ratio == -1)
  {
    printf ("   * New module: [%lu:%lu] with constant duration %f\n",
            module_type_ll,
            module_value_ll,
            const_burst_duration);
  }
  else
  {
    printf ("   * New module: [%lu:%lu] with ratio %f\n",
            module_type_ll,
            module_value_ll,
            ratio);
  }
  // mod = (struct t_module *) query_prio_queue (&Ptask->Modules, (t_priority)identificator);

  mod = (struct t_module*) find_module (&(current_Ptask->Modules),
                                          module_type_ll,
                                          module_value_ll);

  if (mod == M_NIL)
  {
    mod = (struct t_module*) malloc (sizeof(struct t_module));
    mod->type                 = module_type_ll;
    mod->value                = module_value_ll;
    mod->ratio                = ratio;
    mod->const_burst_duration = const_burst_duration * 1e9;
    mod->module_name          = (char*) 0;
    mod->activity_name        = (char*) 0;
    mod->src_file             = -1;
    mod->src_line             = -1;
    mod->used                 =  0; /* De moment no ha estat utilitzat */

    insert_module (&current_Ptask->Modules,
                   module_type_ll,
                   module_value_ll,
                   mod);
  }
  else
  {
    fprintf (stdout, "   * Module [%lu:%lu] already exists. Overwriting! (Old ratio %f - New Ratio %f)\n",
             module_type,
             module_value,
             mod->ratio, ratio);

    mod->ratio = ratio;
  }
  return;
}

void TASK_Initialize_Ptask_Mapping(struct t_Ptask *Ptask)
{
  int  new_taskid, i;
  int *task_mapping;

  Ptask->tasks = (struct t_task*) malloc(Ptask->tasks_count*sizeof(struct t_task));
  //Added here to initialize the task first
           
  get_acc_tasks_info(Ptask);
  for (new_taskid = 0; new_taskid < Ptask->tasks_count; new_taskid++)
  {
    for (i = 0; i < Ptask->acc_tasks_count; i++)
    {
      if (Ptask->acc_tasks[i] == new_taskid)
      {
        TASK_New_Task(Ptask, new_taskid, TRUE);      
        break;
      }
    }
    if (i == Ptask->acc_tasks_count)
    {       
      TASK_New_Task(Ptask, new_taskid, FALSE);       
    }
  }
  if (Ptask->map_definition == MAP_FILL_NODES)
  {
    if ( (task_mapping = TASK_Map_Filling_Nodes(Ptask->tasks_count)) == NULL)
    {
      die("Unable to apply the fill nodes mapping");
    }

    if (debug)
    {
      printf(" (Fill nodes mapping)\n");
    }   
  }
  else if (Ptask->map_definition == MAP_N_TASKS_PER_NODE)
  {
    task_mapping = TASK_Map_N_Tasks_Per_Node(Ptask->tasks_per_node);
    if (task_mapping == NULL)
    {
      die("'%d' tasks per node mapping not applicable (%d nodes per %d tasks is less than the application total tasks %d)",
          Ptask->tasks_per_node,
          SIMULATOR_get_number_of_nodes(),
          Ptask->tasks_per_node,
          Ptask->tasks_count);
    }
    if (debug)
    {
      printf(" (%d tasks per node mapping)\n", Ptask->tasks_per_node);
    }
  }
  else if (Ptask->map_definition == MAP_INTERLEAVED)
  {    
    task_mapping = TASK_Map_Interleaved(Ptask->tasks_count);
    
    if (debug)
    {
      printf(" (interleaved mapping)\n");
    }
  }
  else
  {
    die ("Unknow predefined map value when initializing task mapping (%d)",
         Ptask->map_definition);
  }
 
  get_acc_tasks_info(Ptask);
  if (Ptask->acc_tasks_count == -1)
  { // -1: search for acc_tasks not done yet

  	if ( (Ptask->acc_tasks == NULL && Ptask->acc_tasks_count > 0) ||
  			Ptask->acc_tasks_count < 0)
  	{
  		die ("Wrong accelerator tasks mapping definition in Ptask%d",
  				Ptask->Ptaskid);
  	}
  }
  Update_Node_Info(Ptask->tasks, Ptask->tasks_count, task_mapping);
  
  #if DEBUG
  printf("Mapping = { ");
  for (i = 0; i < Ptask->tasks_count; i++)
  {
    printf("%d ", task_mapping[i]);
  }
  printf("}\n");
  #endif

  free(task_mapping);
}

int* TASK_Map_Filling_Nodes(int task_count)
{
  int                *task_mapping;
  struct t_Ptask     *Ptask;
  int      *n_cpus_per_node;
  int       i_node, j_cpu, i_machines;
  size_t tasks_it;
  struct t_node *node;

  t_boolean end = FALSE, saturated_node = FALSE;
  task_mapping = malloc(task_count*sizeof(int));

  int i;
  for (i=0; i<task_count; ++i)
  {
    task_mapping[i] = -1;
  }
  int n_nodes = SIMULATOR_get_number_of_nodes();
  n_cpus_per_node = SIMULATOR_get_cpus_per_node();

  if ( (n_cpus_per_node = SIMULATOR_get_cpus_per_node()) == NULL)
  {
    return NULL;
  }
  // STEP 1: Map accelerated tasks  
  for(Ptask  = (struct t_Ptask *) head_queue (&Ptask_queue);
    Ptask != P_NIL;
    Ptask  = (struct t_Ptask *) next_queue (&Ptask_queue))
  {
    for(tasks_it = 0; tasks_it < Ptask->tasks_count; tasks_it++)
    {
      struct t_task *task = &(Ptask->tasks[tasks_it]);
      if(task->accelerator)
      { 

          for(i_node = 0; i_node < n_nodes && tasks_it < task_count; i_node++)
          {
            node = get_node_by_id(i_node);          
            if(node->accelerator && node->has_accelerated_task==FALSE)
            {
              task_mapping[tasks_it] = i_node;
              n_cpus_per_node[i_node]--; // One CPU is now occupied
              node->has_accelerated_task = TRUE; // One GPU is now occupied
              break;
            }
  } } } } 

  // STEP 2: Map no-accelerated tasks 
  int last_task_assigned = 0;

  for(i_node = 0; i_node < n_nodes && last_task_assigned < task_count; i_node++)
  {
    int n_cpus_node = n_cpus_per_node[i_node];
    node = get_node_by_id(i_node);
    if (node->accelerator)
    {
      n_cpus_node--;
    }
    for(j_cpu = 0; j_cpu < n_cpus_node && last_task_assigned < task_count; j_cpu++)
    { 
      while (last_task_assigned < task_count &&
        task_mapping[last_task_assigned] != -1) 
      {
        last_task_assigned++;
      }     
      if (task_mapping[last_task_assigned] == -1)
      {
        task_mapping[last_task_assigned] = i_node; 
        last_task_assigned++;   
      }
    }    
  }

  /*** If there is any task that could not be mapped, assign all of them
  to the last node.***/
  for (i=0; i < task_count; ++i)
  {
    if (task_mapping[i] == -1)
    {
      task_mapping[i] = n_nodes-1;
      saturated_node = TRUE;
    }
  }

  if (saturated_node)
  {
    printf("\nNode is saturated cannot do anything\n");
  }

  free(n_cpus_per_node);
  return task_mapping;
}

int* TASK_Map_N_Tasks_Per_Node(int n_tasks_per_node)
{
  int  last_task_assigned = 0;
  int  i, i_node, j_cpu;
  size_t tasks_it;

  int  n_nodes = SIMULATOR_get_number_of_nodes();
  
  int * tasks_in_node = calloc(n_nodes,sizeof(int));
  int * n_cpus_per_node = SIMULATOR_get_cpus_per_node();

  int total_task_count = 0;

  struct t_Ptask *Ptask;
  for ( Ptask  = (struct t_Ptask *) head_queue (&Ptask_queue);
        Ptask != P_NIL;
        Ptask  = (struct t_Ptask *) next_queue (&Ptask_queue))
  {
      total_task_count += Ptask->tasks_count;
  }
 
  int * task_mapping = malloc(total_task_count*sizeof(int));
   
  if (n_tasks_per_node * n_nodes < total_task_count)
    return NULL;

  for (i=0; i<total_task_count; ++i)
    task_mapping[i] = -1;

  if ( (n_cpus_per_node = SIMULATOR_get_cpus_per_node()) == NULL)
    return NULL;
    
  int task_in_node = 0;

  // STEP 1: Map accelerated tasks
  struct t_node *node;
  for ( Ptask  = (struct t_Ptask *) head_queue (&Ptask_queue);
        Ptask != P_NIL;
        Ptask  = (struct t_Ptask *) next_queue (&Ptask_queue))
  {
      int app_task_count = Ptask->tasks_count;
      for (tasks_it = 0; tasks_it < app_task_count; tasks_it++)
      {
          struct t_task *task = &(Ptask->tasks[tasks_it]);
          if(task->accelerator)
          {
              for (i_node = 0; i_node < n_nodes; ++i_node)
              {
                  if (tasks_in_node[i_node] < n_tasks_per_node)
                  {
                    node = get_node_by_id(i_node);
                    if(node->accelerator == TRUE 
                            && node->has_accelerated_task == FALSE)
                    {                
                      task_mapping[tasks_it] = i_node;
                      ++tasks_in_node[i_node];
                      //--n_cpus_per_node[i_node];
                      node->has_accelerated_task = TRUE;
                      break;
                    }
                  }
              }
          }
      }
  }

  // STEP 2: Map no-accelerated tasks 
  for(i_node = 0; i_node < n_nodes; i_node++)
  {
      if (tasks_in_node[i_node] == n_tasks_per_node)
          continue;

      int n_cpus_node = n_cpus_per_node[i_node];
      node = get_node_by_id(i_node);

      if (node->accelerator)
          n_cpus_node--; //GPU must be subtracted

      int i_task;
      for (i_task=0; i_task < total_task_count; ++i_task)
      {
          if (task_mapping[i_task] == -1)
          {
              task_mapping[i_task] = i_node;
              ++tasks_in_node[i_node];

              if (tasks_in_node[i_node] == n_tasks_per_node)
                  break;
          }
      }
  }
  return task_mapping;
}

int* TASK_Map_Interleaved(int task_count)
{
  int *task_mapping;
  int *n_cpus_per_node;
  struct t_Ptask *Ptask;
  int  i, last_task_assigned = 0;
  size_t tasks_it;
  int map;

  n_cpus_per_node = SIMULATOR_get_cpus_per_node();  
  int  n_nodes = SIMULATOR_get_number_of_nodes();
  task_mapping = malloc(task_count*sizeof(int));

  for (i=0; i<task_count; ++i)
  {
    task_mapping[i] = -1;
  }

  //MAP ACCELERATED TASKS
  struct t_node *node;
  int i_node;
  for (Ptask  = (struct t_Ptask *) head_queue (&Ptask_queue);
  Ptask != P_NIL;
  Ptask  = (struct t_Ptask *) next_queue (&Ptask_queue))
  {
    for (tasks_it = 0; tasks_it < Ptask->tasks_count; tasks_it++)
    {      
      struct t_task *task = &(Ptask->tasks[tasks_it]);
      if(task->accelerator)
      {
        for (i_node = 0; i_node < n_nodes && tasks_it < task_count; i_node++)
        {
          node = get_node_by_id(i_node);          
          if(node->accelerator == TRUE && node->has_accelerated_task == FALSE) 
          {                
            n_cpus_per_node [i_node]--; // One CPU is now occupied           
            task_mapping[tasks_it] = i_node % n_nodes;            
            last_task_assigned++;
            node->has_accelerated_task = TRUE; // One GPU is now occupied
            break;
          }
  } } } }
  //STEP 2 MAP NON ACCELERATED TASKS
  int j_cpu, task_per_node;
  //int n_cpus_node = n_cpus_per_node[i_node];
  for(i = 0; i < task_count && last_task_assigned < task_count; i++)
  {

    for(i_node = 0; i_node < n_nodes && last_task_assigned < task_count; i_node++)
    {
      node = get_node_by_id(i_node);
      if(node->accelerator)
      {
        n_cpus_per_node[i_node]--;
      }
      //one condition to subtract a cpu from a accelerated node.
      while (last_task_assigned < task_count &&
      task_mapping[last_task_assigned] != -1) 
      {
        last_task_assigned++;
      }
      if (task_mapping[last_task_assigned] == -1)
      {
        task_mapping[last_task_assigned] = i_node%n_nodes; 
        last_task_assigned++;
	//task_per_node;   
      }
    }  
  }
  return task_mapping;
}
//Added to update the acc node info after the fill/n-task-per-node/task-interleaved nodes
 /*Chetan*/
void Update_Node_Info(
  struct t_task *tasks, /* vector of tasks */
  int tasks_count,
  int *task_mapping)
{
  
  int nodeid;
  t_boolean acc_task;
  struct t_task *task;
  struct t_node *node;
  int            in_mem_links, out_mem_links, i;
  struct t_link *link;
  int task_it;

  for (task_it = 0; task_it < tasks_count; task_it++)
  {
    task = &tasks[task_it];
    nodeid = task_mapping[task_it];
    node = get_node_by_id(nodeid);   
    task->nodeid = nodeid;

    if (node->in_mem_links == 0 && node->out_mem_links == 0)
    {
      node->infinite_mem_links = TRUE;
    }
    else if (node->in_mem_links == 0 || node->out_mem_links == 0)
    {
      int links;

      task->half_duplex_links  = TRUE;

      links = MAX(node->in_mem_links, node->out_mem_links);

      in_mem_links  = links;
      out_mem_links = links;
    }
    else
    {
      in_mem_links  = node->in_mem_links;
      out_mem_links = node->out_mem_links;
    }

    create_queue (&(task->free_in_links));
    for (i = 0; i < in_mem_links; i++)
    {
      link = (struct t_link*) malloc (sizeof(struct t_link));

      link->linkid    = i + 1;
      link->info.task = task;
      link->kind      = MEM_LINK;
      link->type      = IN_LINK;
      link->thread    = TH_NIL;

      ASS_ALL_TIMER (link->assigned_on, current_time);
      inFIFO_queue (&(task->free_in_links), (char*) link);
    }
    create_queue (&(task->free_out_links));
    for (i = 0; i < out_mem_links; i++)
    {
      link = (struct t_link*) malloc (sizeof(struct t_link));

      link->linkid    = i + 1;
      link->info.task = task;
      link->kind      = MEM_LINK;
      link->type      = OUT_LINK;
      link->thread    = TH_NIL;

      ASS_ALL_TIMER (link->assigned_on, current_time);
      inFIFO_queue (&(task->free_out_links), (char*) link);
    }
  }
}

void module_entrance(struct t_thread  *thread,
                     unsigned long int module_type,
                     unsigned long int module_value)
{
  register struct t_module *mod;
  register struct t_Ptask  *Ptask = thread->task->Ptask;

  if (module_type == IDLE_EVENT_TYPE)
  {
    thread->idle_block = TRUE;
  }

  mod = (struct t_module*) find_module(&Ptask->Modules,
                                       module_type,
                                       module_value);

  if (mod != M_NIL)
  {
    mod->used = 1; /* Ara ja ha estat utilitzat! */
    inLIFO_queue (&(thread->modules), (char *)mod);

    if (debug&D_TASK)
    {
      PRINT_TIMER (current_time);
      printf (": Going into module [%lu:%lu] (%s) for P%02d T%02d th%02d\n",
              mod->type,
              mod->value,
              mod->module_name,
              IDENTIFIERS(thread));
    }
  }
}

int module_exit (struct t_thread  *thread,
                 unsigned long int module_type)
{
  register struct t_Ptask  *Ptask = thread->task->Ptask;
  register struct t_module *mod;
  register struct t_thread *awaked;
  register struct t_action *action;
  register t_nano ti, bandw;
  struct t_node *node;
  float fo;
  struct t_machine *machine;

  #ifdef BLOCK_SPECIAL
  if (((identificator ==  3) || (identificator == 6) || (identificator == 7) ||
       (identificator == 13) || (identificator == 4))
      &&
      (thread->global_op_done == FALSE))
  {
    if (count_queue(&(thread->task->Ptask->tasks)) ==
        1 + count_queue (&(thread->task->Ptask->global_operation)))
    {
      node = get_node_of_task (thread->task);
      machine = node->machine;
      ti = node->remote_startup;
      bandw = (t_nano) machine->communication.remote_bandwidth;
      if (bandw!=0)
      {
        bandw = (t_nano) ((t_nano) (1e9) / (1 << 20) / bandw);
        /*ti += bandw*1024;*/
      }
      fo = log((t_nano)count_queue(&(thread->task->Ptask->tasks)))/log(2.0);
      ti = ti*fo;
      while (awaked=(struct t_thread *)outFIFO_queue(&(thread->task->Ptask->global_operation)))
      {
        SCHEDULER_thread_to_ready_return (M_COM, awaked, ti, 0);
      }
      SCHEDULER_thread_to_ready_return (M_COM, thread, ti, 0);
      thread->global_op_done = TRUE;
      return(1);
    }
    else
    {
      inFIFO_queue (&(thread->task->Ptask->global_operation), (char *)thread);
      thread->global_op_done = TRUE;
      return (1);
    }
  }
  else
  {
    thread->global_op_done = FALSE;
  #endif

    if (module_type == IDLE_EVENT_TYPE)
    {
      thread->idle_block = FALSE;
    }

    mod = (struct t_module *) outLIFO_queue(&(thread->modules));

  if (mod != M_NIL)
  {
    if (mod->type != module_type)
    {
      inLIFO_queue (&(thread->modules), (char*) mod);
    }

    if (debug&D_TASK)
    {
      PRINT_TIMER (current_time);
      printf (": Going out module: [%lu] (Unknown) for P%02d T%02d th%02d\n",
              module_type,
              IDENTIFIERS(thread));
    }
  }

  #ifdef BLOCK_SPECIAL
}
  #endif

  return(0);
}

void recompute_work_upon_modules(struct t_thread *thread,
                                 struct t_action *action)
{
  register struct t_module *mod;

  if (action->action != WORK)
  {
    panic("Improper call to recompute_work_upon_modules\n");
  }

  mod = (struct t_module*) head_queue(&(thread->modules));

  if (mod != M_NIL)
  {
    if (mod->ratio == (double) -1)
    {
      action->desc.compute.cpu_time = mod->const_burst_duration;
    }
    else if (mod->ratio == 0.0)
    {
      action->desc.compute.cpu_time = 0.0;
    }
    else
    {
      action->desc.compute.cpu_time = action->desc.compute.cpu_time / mod->ratio;
    }
  }
}

void user_event_type_name(struct t_Ptask *Ptask,
                          int             type,
                          char           *name,
                          int             color)
{
  struct t_user_event_info *evinfo;

  evinfo = (struct t_user_event_info *) query_prio_queue(&Ptask->UserEventsInfo,
                                                         (t_priority) type);

  if (evinfo == (struct t_user_event_info*) 0)
  {
    evinfo = (struct t_user_event_info*)
      malloc(sizeof(struct t_user_event_info));

    evinfo->type  = type;
    evinfo->color = color;
    evinfo->name  = name;
    create_queue (&(evinfo->values));
    insert_queue (&Ptask->UserEventsInfo, (char*)evinfo, (t_priority) type);
  }
  else
  {
    printf ("Warning: redefinition of user event type %d\n",type);
    free(name);
  }
}

void user_event_value_name (struct t_Ptask *Ptask,
                            int             type,
                            int             value,
                            char           *name)
{
  struct t_user_event_info       *evinfo;
  struct t_user_event_value_info *evinfo_val;

  /* Es busca la informacio d'aquest tipus d'event */
  evinfo = (struct t_user_event_info*) query_prio_queue(&Ptask->UserEventsInfo,
                                                        (t_priority) type);

  if (evinfo == (struct t_user_event_info*) 0)
  {
    printf ("Warning: User event type %d not defined for user event value %d definition\n",
            type,
            value);
    free(name);
    return;
  }

  /* Es mira si ja te definit aquest valor */
  evinfo_val = (struct t_user_event_value_info*)
    query_prio_queue(&evinfo->values, (t_priority)value);

  if (evinfo_val == (struct t_user_event_value_info*) 0)
  {
    evinfo_val = (struct t_user_event_value_info*)
      malloc (sizeof(struct t_user_event_value_info));

    evinfo_val->value = value;
    evinfo_val->name  = name;
    insert_queue (&evinfo->values, (char*) evinfo_val, (t_priority) value);
  }
  else
  {
    printf ("Warning: redefinition of user event value %d for type %d\n",
    value,
    type);
    free(name);
  }
}
/*
 * Gets accelerator task mapping info and stores it in Ptask
 */
void get_acc_tasks_info(struct t_Ptask *Ptask)
{
	int *acc_tasks_mapping;
	int acc_tasks_count;
	char *trace_file_name = Ptask->tracefile;
	acc_tasks_mapping = (int *) 0;
	if (DATA_ACCES_get_acc_tasks(trace_file_name, &acc_tasks_count, &acc_tasks_mapping) == FALSE)
	{
		die("unable to retrieve accelerator tasks of trace '%s': %s",
					 trace_file_name,
					 DATA_ACCESS_get_error());
	}
	Ptask->acc_tasks_count = acc_tasks_count;
	Ptask->acc_tasks = acc_tasks_mapping;
}
