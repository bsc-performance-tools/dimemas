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

/*#define BLOCK_SPECIAL 1*/

#include <math.h>
#include <string.h>
#include <assert.h>

#include <EventEncoding.h>

#if defined(OS_MACOSX) || defined(OS_CYGWIN)
#include "macosx_limits.h"
#else
#include <values.h>
#endif

#include "define.h"
#include "types.h"

#include "aleatorias.h"
#include "cpu.h"
#include "events.h"
#include "extern.h"
#include "list.h"
#include "mallocame.h"
#include "memory.h"
#include "paraver.h"
#include "ports.h"
#include "read.h"
#include "schedule.h"
#include "subr.h"
#include "task.h"
#include "random.h"

#define MIN_SERVICE_TIME (t_micro)7000
#define MAX_SERVICE_TIME (t_micro)13000

/* Preemption overhead variables */
t_boolean PREEMP_initialized = FALSE;
t_boolean PREEMP_enabled;
int       PREEMP_cycle;
t_micro   PREEMP_time;
int      *PREEMP_table; /* Array to manage preemption cycles for each task */

/* Synthetic burst generation variables */
t_boolean      synthetic_bursts = FALSE;
struct t_queue burst_categories;

/* Synthetic burst generation functions */
void
SYNT_BURST_add_new_burst_category( int    burst_category_id,
                                   double burst_category_mean,
                                   double burst_category_std_dev)
{
  burst_category_t new_category;

  if (!synthetic_bursts)
  {
    synthetic_bursts = TRUE;
    create_queue(&(burst_categories));
  }
  
  new_category = (burst_category_t) mallocame(sizeof(struct _burst_category));
  
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

double
SYNT_BURST_get_burst_value( int burst_category_id)
{
  burst_category_t burst_category;
  
  burst_category = 
    (burst_category_t) query_prio_queue(&(burst_categories),
                                        (t_priority) burst_category_id);
  
  if (burst_category == NULL)
    return 0.0;
  
  return random_dist(&(burst_category->values));
}


/* Preemption overhead functions */
void
PREEMP_init(struct t_Ptask *Ptask)
{
  int i, task_num = count_queue(&(Ptask->tasks));

  PREEMP_table = (int*) mallocame (task_num * sizeof(int));
  
  srand((unsigned int) time(NULL));
  
  for (i = 0; i < task_num; i++)
  {
    PREEMP_table[i] = rand();
  }
}

t_micro
PREEMP_overhead(struct t_task* task)
{
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
    return (t_micro) 0;
}

static char BIGPOINTER2[BUFSIZE];

void
TASK_end()
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
	//mmap: Ptask->file will be closed as soon as I put it into mmap
	//     if (Ptask->file != NULL)
	//     {
	//       fclose (Ptask->file);
	//       Ptask->file = NULL;
	//     }
    
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
          printf (
            "WARNING: Some task P%d T%d th%d waiting for global operation %s\n",
            IDENTIFIERS(thread),
            glop->name);
        }
      }
    }

    for(task  = (struct t_task *) head_queue (&(Ptask->tasks));
        task != T_NIL;
        task  = (struct t_task *) next_queue (&(Ptask->tasks)))
    {
      if (count_queue (&(task->recv)) != 0)
      {
        printf (
        "WARNING: Task %02d ends with %d threads waiting to recv message:\n",
          task->taskid,
          count_queue (&(task->recv)));
        
        for(thread  = (struct t_thread *) head_queue (&(task->recv));
            thread != TH_NIL;
            thread  = (struct t_thread *) next_queue (&(task->recv)))
        {
          /* Data access is now located on DATA ACCESS API *
          if (thread->file_shared == FALSE)
          {
            fclose (thread->file);
          } */
          
          printf ("\t* Thread %02d <-",
                  IDENTIFIERS(thread));
          action = thread->action;
          mess = &(action->desc.recv);
          
          printf (" Sender: T%02d  t%02d, Destination T%02d  t%02d  Tag: %02d CommId: %02d Size: %d\n",
                  mess->ori,
                  mess->ori_thread,
                  thread->task->taskid,
                  thread->threadid,
                  mess->mess_tag,
                  mess->communic_id,
                  mess->mess_size);

          node = get_node_of_thread (thread);
          cpu  = get_cpu_of_thread (thread);
          Paraver_thread_idle(0,
                              IDENTIFIERS (thread),
                              thread->last_paraver,
                              current_time);
        }
      }

      if (count_queue (&(task->send)) != 0)
      {
        printf (
          "WARNING: Task %02d ends with %d message(s) pending to send:\n",
          task->taskid,
          count_queue (&(task->send)));
        
        for(thread  = (struct t_thread *) head_queue (&(task->send));
            thread != TH_NIL;
            thread  = (struct t_thread *) next_queue (&(task->send)))
        {
          node = get_node_of_thread (thread);
          cpu = get_cpu_of_thread (thread);
          Paraver_thread_idle(0,
                              IDENTIFIERS (thread),
                              thread->last_paraver,
                              current_time);
          
          printf ("\t* Thread %02d ->",
                  thread->threadid);
          action = thread->action;
          // Vladimir: it has to be action->desc.send
          mess_source = &(action->desc.send);
          // mess_source = &(action->desc.recv);
          
          printf (" Dest-: T%02d Tag: %02d CommId: %02d Size: %d\n",
                  mess_source->dest,
                  mess_source->mess_tag,
                  mess_source->communic_id,
                  mess_source->mess_size);
          
        }
      }

      if (count_queue (&(task->mess_recv)) != 0)
      {
        printf (
          "WARNING: Task %02d ends with %d message(s) in reception queue:\n",
          task->taskid,
          count_queue (&(task->mess_recv)));
        for(thread  = (struct t_thread *) head_queue (&(task->mess_recv));
            thread != TH_NIL;
            thread  = (struct t_thread *) next_queue (&(task->mess_recv)))
        {
          action = thread->action;
          mess_source = &(action->desc.send);
          printf ("\t-> Sender: T%02d t%02d  destT%02d  destt%02d  Tag: %02d CommId: %d Size: %d\n",
                  thread->task->taskid,
                  thread->threadid,
                  mess_source->dest,
                  mess_source->dest_thread,
                  mess_source->mess_tag,
                  mess_source->communic_id,
                  mess_source->mess_size);
        }
      }
    }
  }
}

void
new_communicator_definition (struct t_Ptask *Ptask, int communicator_id)
{
  register struct t_communicator *comm;
//   printf("called once\n");
  comm = (struct t_communicator *)query_prio_queue (&Ptask->Communicator,
      (t_priority)communicator_id);
  if (comm!=(struct t_communicator *)0)
  {
      panic("Redefinition of communicator %d for P%d\n",
            communicator_id,
            Ptask->Ptaskid);
  }
  
  comm = (struct t_communicator *)mallocame(sizeof(struct t_communicator));
  comm->communicator_id = communicator_id;
  create_queue (&comm->global_ranks);
  create_queue (&comm->threads);
  create_queue (&comm->machines_threads);
  create_queue (&comm->m_threads_with_links);
  comm->current_root = TH_NIL;
  comm->in_flight_op = FALSE;
  
  insert_queue (&Ptask->Communicator, (char *)comm, (t_priority)communicator_id);
}

void
add_identificator_to_communicator(struct t_Ptask *Ptask,
                                  int communicator_id,
                                  int taskid)
{
  register struct t_communicator *comm;
  int *mtaskid;
  int  i;

  comm = (struct t_communicator *)query_prio_queue(&Ptask->Communicator,
                                                  (t_priority)communicator_id);
  
  if (comm==(struct t_communicator *)0)
  {
      panic("Unable to locate communicator %d for P%d\n",
            communicator_id,
            Ptask->Ptaskid);
  }
  if (taskid==-1)
  {
    for (i=0; i<count_queue(&Ptask->tasks); i++)
    {
      mtaskid = (int *)mallocame(sizeof(int));
      *mtaskid = i+1;
      inFIFO_queue (&comm->global_ranks, (char *)mtaskid);
    }
    return;
  }
  if (taskid>count_queue(&Ptask->tasks))
    panic(
      "Communicator definition: Specified rank %d not valid in this Ptask\n",
      taskid);
  mtaskid = (int *)mallocame(sizeof(int));
  *mtaskid = taskid+1;
  inFIFO_queue (&comm->global_ranks, (char *)mtaskid);
}

void
no_more_identificator_to_communicator(struct t_Ptask *Ptask,
                                      int communicator_id)
{
  register struct t_communicator *comm;
  int *mtaskid;
  int  i;
  int *trips;

  comm = (struct t_communicator *)query_prio_queue (&Ptask->Communicator,
      (t_priority)communicator_id);
  
  if (comm==(struct t_communicator *)0)
  {
      panic ("Unable to locate communicator %d for P%d\n",
             communicator_id,
             Ptask->Ptaskid);
  }
  trips = (int *)mallocame (3*count_queue(&comm->global_ranks)*sizeof(int));
  i=0;
  for (mtaskid=(int *)head_queue(&comm->global_ranks);
       mtaskid!=(int *)0;
       mtaskid=(int *)next_queue(&comm->global_ranks))
  {
    trips[i] = *mtaskid;
    i++;
    trips[i] = *mtaskid;
    i++;
    trips[i] = 1;
    i++;
  }
  freeame ((char *)trips, count_queue(&comm->global_ranks)*sizeof(int));
}

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
  
  win = (struct t_window *)mallocame(sizeof(struct t_window));
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

void
add_identificator_to_window(struct t_Ptask *Ptask, int window_id, int taskid)
{
  register struct t_window *win;
  int *mtaskid;
  int  i;

  win = (struct t_window *)query_prio_queue (&Ptask->Window,
                                            (t_priority)window_id);
  
  if (win==(struct t_window *)0)
  {
      panic("Unable to locate window %d for P%d\n",
            window_id,
            Ptask->Ptaskid);
  }

  if (taskid==-1)
  {
    for (i=0; i<count_queue(&Ptask->tasks); i++)
    {
      mtaskid = (int *)mallocame(sizeof(int));
      *mtaskid = i+1;
      inFIFO_queue (&win->global_ranks, (char *)mtaskid);
    }
    return;
  }
  if (taskid>count_queue(&Ptask->tasks))
    panic("Specified rank %d not valid in this Ptask\n", taskid);
  mtaskid = (int *)mallocame(sizeof(int));
  *mtaskid = taskid+1;
  inFIFO_queue (&win->global_ranks, (char *)mtaskid);
}

void
no_more_identificator_to_window(struct t_Ptask *Ptask, int window_id)
{
  register struct t_window *win;
//   int *mtaskid;
//   int  i;
//   int *trips;

  win = (struct t_window *)query_prio_queue (&Ptask->Window,
      (t_priority)window_id);
  
  if (win==(struct t_window *)0)
  {
    panic("Unable to locate window %d for P%d\n",
          window_id,
          Ptask->Ptaskid);
  }
  /* FEC: Aqui no s'esta fent res!!!!!!! 
   * Per fer aixo ja es pot retornar! Per tant, ho comento.
  trips = (int *)mallocame (3*count_queue(&win->global_ranks)*sizeof(int));
  i=0;
  for (mtaskid=(int *)head_queue(&win->global_ranks);
       mtaskid!=(int *)0;
       mtaskid=(int *)next_queue(&win->global_ranks))
  {
    trips[i] = *mtaskid;
    i++;
    trips[i] = *mtaskid;
    i++;
    trips[i] = 1;
    i++;
  }
  freeame ((char *)trips, count_queue(&win->global_ranks)*sizeof(int));
  */
}

/*
 * Create a new Ptask. Ptask is a queue whose elements are tasks
 */
struct t_Ptask*
create_Ptask (char *tracefile, char *configfile)
{
  struct t_Ptask *Ptask;
  
  Ptask = (struct t_Ptask *) mallocame (sizeof (struct t_Ptask));
  Ptask->Ptaskid = Ptask_ids;
  Ptask_ids++;
  Ptask->tracefile  = tracefile;
  Ptask->configfile = configfile;
  Ptask->n_rerun = 0;
//mmap: changing to mmap
//   Ptask->file = (FILE *) NULL;
  Ptask->mmapped_file = (char *) NULL;
  Ptask->mmap_position = 0;
  Ptask->synthetic_application = FALSE;
  create_queue (&(Ptask->tasks));
  create_queue (&(Ptask->global_operation));
  create_queue (&(Ptask->Communicator));
  create_queue (&(Ptask->Window));
  create_queue (&(Ptask->MPI_IO_fh_to_commid));
  create_queue (&(Ptask->MPI_IO_request_thread));
  // create_queue (&(Ptask->Modules));
  create_modules_map(&(Ptask->Modules));
  create_queue (&(Ptask->Filesd));
  create_queue (&(Ptask->UserEventsInfo));
  return (Ptask);
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
  ASS_TIMER (account->initial_time, LDBL_MAX);
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
struct t_account *
new_accounter ()
{
  struct t_account *res;
  
  res = (struct t_account *) mallocame (sizeof (struct t_account));
  clear_account (res);
  return (res);
}

/*
 * Create new account entry Initializes to 0
 */
void
new_account (struct t_queue *acc, int nodeid)
{
  struct t_account *new_acc;
  
  new_acc = (struct t_account *) mallocame (sizeof (struct t_account));
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

/*  set the number of threads and allocate the array for storing all threads_array
    for future fast serching by indexing with threadid
*/
void
set_tasks_number_of_threads (struct t_task *task, int number_of_threads)
{

   /*Vladimir: for optimizations when a task has many threads*/
   task->num_of_threads = number_of_threads;
   task->threads_array = (struct t_thread **) mallocame (sizeof (struct t_thread*) * number_of_threads);
   total_threads += number_of_threads;
}

/*
 * Create a task and all threads in tasks
 */
void
new_task_in_Ptask (struct t_Ptask *Ptask, int taskid, int nodeid)
{
  struct t_task  *task;
  int             num_tasks;
  int             i;

  if (get_node_by_id (nodeid) == N_NIL)
  {
    panic("Incorrect node identifier %d in config file %s for P%d T%d\n",
          nodeid - 1,
          Ptask->tracefile,
          Ptask->Ptaskid,
          taskid);
  }

  /* Search or insert task for these threads */
  num_tasks = count_queue (&(Ptask->tasks));
  task = T_NIL;
  if (num_tasks != 0)
  {
    task = (struct t_task *) head_queue (&(Ptask->tasks));
  }

  for (i = 0; i < num_tasks; i++)
  {
    if (task->taskid == taskid)
    {
      break;
    }
    task = (struct t_task *) next_queue (&(Ptask->tasks));
  }

  if ((i == num_tasks) && (task == T_NIL))
  {
    task = (struct t_task *) mallocame (sizeof (struct t_task));
    task->taskid = taskid;
    task->nodeid = nodeid;
    task->Ptask = Ptask;
    task->io_thread = FALSE;
    create_queue (&(task->threads));

   /*Vladimir: for optimizations when a task has many threads
     here set to 0, set to correct values in set_tasks_number_of_threads */
    task->num_of_threads = 0;
    task->threads_array = NULL;
    
    create_queue (&(task->mess_recv));
    create_queue (&(task->recv));
    create_queue (&(task->send));
    /* FEC: Cal crear les dues noves cues pels Irecv*/
    create_queue (&(task->recv_without_send));
    create_queue (&(task->send_without_recv));
    inFIFO_queue (&(Ptask->tasks), (char *) task);
  }

/*   threads inside tasks will be added later
     when I read from the trace 
     how many threads there is in every task      */
     
}


/*
 * Create a thread and put in on the queue in the task
   Vladimir: this is used for hardcoding
   Dimemas expects 1 thread per task
   and I let it start thinking that it is going to be that way
   but when I read trf file I can see how many threads there is
   and then I augment this number
 */
void add_thread_to_task (struct t_task *task, int threadid, int nodeid)
{
  struct t_thread *thread;

  if (get_node_by_id (nodeid) == N_NIL)
  {
    panic("Incorrect node identifier %d in  T%d\n",
          nodeid - 1,
//           Ptask->tracefile,
//           Ptask->Ptaskid,
          task->taskid);
  }

  /* Insert new thread in task */
    thread = (struct t_thread*) mallocame (sizeof (struct t_thread));
    create_queue (&(thread->account));
    create_queue (&(thread->modules));
    create_queue (&(thread->Activity));
    thread->threadid = threadid; //i + 1;
    thread->task = task;
    thread->put_into_ready = current_time;
    thread->action = AC_NIL;
    thread->original_thread = TRUE;
    thread->twin_thread = TH_NIL;
    thread->doing_context_switch = FALSE;
    thread->min_time_to_be_preempted = current_time;
    thread->doing_busy_wait = FALSE;
    thread->last_action = AC_NIL;
    thread->local_link = L_NIL;
    thread->partner_link = L_NIL;
    thread->local_hd_link = L_NIL;
    thread->partner_hd_link = L_NIL;
    thread->partner_node = N_NIL;
    thread->doing_startup = FALSE;
    thread->startup_done  = FALSE;
    thread->doing_copy    = FALSE;
    thread->copy_done     = FALSE; /* JGG New latency modelling */
    thread->loose_cpu     = TRUE;
    thread->last_paraver = current_time;
    thread->size_port = 0;
    thread->port_send_link = L_NIL;
    thread->port_recv_link = L_NIL;
    thread->to_module = 0;
    thread->port = PO_NIL;
    thread->copy_segment_link_source = L_NIL;
    thread->copy_segment_link_dest = L_NIL;
    thread->copy_segment_size = 0;
    thread->original_seek = 0;
    thread->seek_position = 0;
    thread->base_priority = 0;

    thread->sstask_id   = 0;
    thread->sstask_type = 0;    
           
    thread->portid = port_ids++;
    PORT_create (thread->portid, thread);
  
    thread->portid = port_ids++;
    PORT_create (thread->portid, thread);
  
    thread->portid = port_ids++;
    PORT_create (thread->portid, thread);

    /* making these queues separate for every thread
       only DEPENDENCIES go to this queues
       REAL MPI TRANSFERS go the the queues of the task    */
    create_queue (&(thread->mess_recv));
    create_queue (&(thread->recv));
    create_queue (&(thread->send));
    /* FEC: Cal crear les dues noves cues pels Irecv*/
    create_queue (&(thread->recv_without_send));
    create_queue (&(thread->send_without_recv));
    
    new_account (&(thread->account), nodeid);
    inFIFO_queue (&(task->threads), (char *) thread);
    /* and store it in the array of all threads in that task */
    assert(task->num_of_threads >= thread->threadid);
    task->threads_array[thread->threadid-1] = (struct t_thread*) thread;
    
    thread->last_cp_node = (struct t_cp_node *)0;
    thread->global_op_done = FALSE;
  
    ASS_ALL_TIMER (thread->last_time_event_number,0);
  
    thread->marked_for_deletion = 0;
//     thread->file_shared         = FALSE;
  
}

struct t_thread*
locate_thread_of_task (struct t_task *task, int thid)
{
  register struct t_thread *thread;

  assert(task->num_of_threads > thid-1);
  assert(thid-1 >= 0);

  thread = task->threads_array[thid-1];

  assert (thread != TH_NIL);
  assert (thread->threadid == thid);
  return (thread);
}



struct t_thread* locate_thread (struct t_Ptask *Ptask, int taskid, int thid)
{
  register struct t_task  *task;
  register struct t_thread *thread;
  
  /* Locate task in Ptask */
  for(task  = (struct t_task*) head_queue (&(Ptask->tasks));
      task != T_NIL;
      task  = (struct t_task *) next_queue (&(Ptask->tasks)))
  {
    if (task->taskid == taskid)
      break;
  }

  if (task == T_NIL)
  {
    panic (
     "Incorrect task identifier %d in trace file %s for P%d in locate_thread\n",
      taskid,
      Ptask->tracefile,
      Ptask->Ptaskid);
  }

//   /* Locate thread onto that task */
//   for(thread  = (struct t_thread *) head_queue (&(task->threads));
//       thread != TH_NIL;
//       thread  = (struct t_thread *) next_queue (&(task->threads)))
//   {
//     if (thread->threadid == thid)
//       break;
//   }

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
 * Append an action to action list of specific thread
 */
void new_action_to_thread (
  struct t_Ptask *Ptask,
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
    thread->action = action;
  else
  {
    if (action->action == WORK)
      recompute_work_upon_modules(thread, action);
    if ((action->action == WORK) && (last_action->action == WORK))
    {
      last_action->desc.compute.cpu_time += action->desc.compute.cpu_time;
      freeame ((char *) action, sizeof (struct t_action));
      return;
    }
    last_action->next = action;
  }
  thread->last_action = action;
}

struct t_account*
current_account(struct t_thread *thread)
{
  struct t_account *temp;
//  printf("current_account step 0, thread_account.first == %p\n", thread->account.first);
  temp = ((struct t_account *) tail_queue (&(thread->account)));
  return temp;
//((struct t_account *) tail_queue (&(thread->account)));
}

/*
 * Find task into Ptask with taskid
 */
struct t_task  *
locate_task (struct t_Ptask *Ptask, int taskid)
{
  struct t_task  *task;

  for(task  = (struct t_task *) head_queue (&(Ptask->tasks));
      task != T_NIL;
      task  = (struct t_task *) next_queue (&(Ptask->tasks)))
  {
    if (task->taskid == taskid)
      return (task);
  }
  printf("locate task returns no pointer ???? \n");
  return (T_NIL);
}

struct t_thread*
duplicate_thread_fs (struct t_thread *thread)
{
  struct t_thread *copy_thread;
  struct t_node    *node;
  struct t_machine *machine;
   
  node = get_node_of_thread (thread);
  machine = node->machine;
   
  copy_thread = (struct t_thread *) mallocame (sizeof (struct t_thread));
  bcopy (thread, copy_thread, sizeof(struct t_thread));
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
  
  return (copy_thread);
}

void
delete_duplicate_thread_fs (struct t_thread *thread)
{
  SCHEDULER_free_parameters (thread);
  freeame ((char *) thread, sizeof (struct t_thread));
}

struct t_thread*
duplicate_thread (struct t_thread *thread)
{
  struct t_thread *copy_thread;
  struct t_action *action, *ac;
  struct t_node    *node;
  struct t_machine *machine;
   
  node = get_node_of_thread (thread);
  machine = node->machine;
   
  copy_thread = (struct t_thread *) mallocame (sizeof (struct t_thread));
  bcopy (thread, copy_thread, sizeof(struct t_thread));

  copy_thread->original_thread          = FALSE;
  copy_thread->twin_thread              = thread;
  copy_thread->doing_context_switch     = FALSE;
  copy_thread->min_time_to_be_preempted = thread->min_time_to_be_preempted;
  copy_thread->doing_busy_wait          = FALSE;
  copy_thread->threadid                 = thread->threadid;
  copy_thread->task                     = thread->task;
  copy_thread->put_into_ready           = thread->put_into_ready;
  copy_thread->last_action              = AC_NIL;
  /* copy_thread->account               = thread->account;*/
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

  ac = thread->action;
  action = (struct t_action *) mallocame (sizeof (struct t_action));
  memcpy (action, ac,sizeof(struct t_action));
  action->next = AC_NIL;
  copy_thread->action = action;

  /* Intent de crear un nou accounting pel thread nou, que al destruir-se
  * s'afegira al thread original. */
  create_queue (&(copy_thread->account));
  new_account (&(copy_thread->account), node->nodeid);

  return (copy_thread);
}

// this is used for making MPI_Isend 
struct t_thread*
promote_to_original (struct t_thread *copy_thread, struct t_thread *thread)
{

   struct t_node    *node;
   struct t_machine *machine;

   node = get_node_of_thread (thread);
   machine = node->machine;
 
   copy_thread->original_thread          = TRUE;
   thread->original_thread               = FALSE;

   copy_thread->twin_thread              = NULL;
   thread->twin_thread              = copy_thread;

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
  
//I think this should not be changed
/*  copy_thread->logical_send             = thread->logical_send;
  copy_thread->logical_recv             = thread->logical_recv;
  copy_thread->physical_send            = thread->physical_send;
  copy_thread->physical_recv            = thread->physical_recv*/;
  copy_thread->last_cp_node             = thread->last_cp_node;

//   (*SCH[machine->scheduler.policy].init_scheduler_parameters) (copy_thread);
  SCHEDULER_copy_parameters (thread, copy_thread);

  copy_thread->action = thread->action;

/* Intent de crear un nou accounting pel thread nou, que al destruir-se
 * s'afegira al thread original. */
  return (copy_thread);
}


struct t_thread*
promote_to_original2 (struct t_thread *copy_thread, struct t_thread *thread)
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

 /* I'm not sure what to do with this

  copy_thread->original_thread          = TRUE;
  thread->original_thread               = FALSE;

  copy_thread->twin_thread              = NULL;
  thread->twin_thread              = copy_thread;
*/

/*
  copy_thread->doing_context_switch     = thread->doing_context_switch;
  copy_thread->min_time_to_be_preempted = thread->min_time_to_be_preempted;
  copy_thread->doing_busy_wait          = thread->doing_busy_wait;
  copy_thread->threadid                 = thread->threadid;
  copy_thread->task                     = thread->task;
  copy_thread->put_into_ready           = thread->put_into_ready;
  copy_thread->last_action              = thread->last_action;
  copy_thread->account               = thread->account;
  copy_thread->local_link               = thread->local_link;
  copy_thread->partner_link             = thread->partner_link;
  copy_thread->local_hd_link            = thread->local_hd_link;
  copy_thread->partner_hd_link          = thread->partner_hd_link;
  copy_thread->last_paraver             = thread->last_paraver;
  copy_thread->base_priority            = thread->base_priority;
  copy_thread->sch_parameters           = thread->sch_parameters;
  copy_thread->seek_position            = thread->seek_position;

*/

/* I do not know will I need this

  copy_thread->local_link               = thread->local_link;
  copy_thread->partner_link             = thread->partner_link;
  copy_thread->local_hd_link            = thread->local_hd_link;
  copy_thread->partner_hd_link          = thread->partner_hd_link;
*/

  thread->logical_send                    = copy_thread->logical_send;
  thread->logical_recv                    = copy_thread->logical_recv;
  thread->physical_send                   = copy_thread->physical_send;
  thread->physical_recv                   = copy_thread->physical_recv;


  ac = copy_thread->action;
  action = (struct t_action *) mallocame (sizeof (struct t_action));
  memcpy (action, ac,sizeof(struct t_action));
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


void
delete_duplicate_thread (struct t_thread *thread)
{
  struct t_thread *twin_thread;
  twin_thread = thread->twin_thread;
  struct t_account *acc_th_copia, *acc_th_original;
  
  SCHEDULER_free_parameters (thread);

  /* Intent d'afegir les dades d'accounting al thread original. */
  acc_th_copia = current_account (thread);

/*  printf("delete duplicate step 11 original thread %p is original %d  twin thread == %p is original %d  account original %p, account twin %p\n",
              thread, thread->original_thread, twin_thread, twin_thread->original_thread, &thread->account, &twin_thread->account);

  printf("WHAT HAPPENS HERE delete dupl account thread and twin 21\n");
  COMMUNIC_debug_the_senders_list(thread);
  COMMUNIC_debug_the_senders_list(twin_thread);
  printf("WHAT HAPPENS HERE delete dupl account thread and twin 22\n");
*/
 
//  if (&(thread->account) != &(thread->twin_thread->account))
//  {

  acc_th_original = current_account (thread->twin_thread);
  add_account(acc_th_original,acc_th_copia);

/*
  printf("WHAT HAPPENS HERE delete dupl account thread and twin 41\n");
  COMMUNIC_debug_the_senders_list(twin_thread);
  printf("WHAT HAPPENS HERE delete dupl account thread and twin 42\n");
  printf("delete duplicate thread step 13\n");
  printf("WHAT HAPPENS HERE delete dupl account only twin 511\n");
  COMMUNIC_debug_the_senders_list(twin_thread);
  printf("WHAT HAPPENS HERE delete dupl account only twin 512\n");
  printf("........twin_thread - original thread %p is original %d\n twin thread == %p account original first %p\n",
           thread->twin_thread, thread->twin_thread->original_thread,
           thread->twin_thread->twin_thread, thread->twin_thread->account.first);
  printf("........thread - original thread %p is original %d\n twin thread == %p account original first %p\n",
           thread, thread->original_thread, thread->twin_thread, thread->account.first);
*/

  extract_from_queue(&(thread->account), (char*) acc_th_copia);

/*
  printf("........twin_thread - original thread %p is original %d\n twin thread == %p account original first %p\n",
           thread->twin_thread, thread->twin_thread->original_thread, 
           thread->twin_thread->twin_thread, thread->twin_thread->account.first);
  printf(".... ...thread - original thread %p is original %d\n twin thread == %p account original first %p\n",
           thread, thread->original_thread, thread->twin_thread, thread->account.first);
  printf("WHAT HAPPENS HERE delete dupl account only twin 521\n");
  COMMUNIC_debug_the_senders_list(twin_thread);
  printf("WHAT HAPPENS HERE delete dupl account only twin 522\n");
  printf("delete duplicate thread step 3\n");
*/
  
  free(acc_th_copia);
  
//  }

  freeame ((char*) thread->action, sizeof (struct t_action));

/*
  printf("delete duplicate thread step 5\n");
  printf("WHAT HAPPENS HERE delete dupl account ONLY twin 81\n");
  COMMUNIC_debug_the_senders_list(twin_thread);
  printf("WHAT HAPPENS HERE delete dupl account thread ONLY twin 82\n");
*/

  freeame ((char*) thread, sizeof (struct t_thread));
}

static t_boolean
more_actions_on_task (struct t_task  *task)
{
  struct t_thread *thread;

  for (thread  = (struct t_thread *) head_queue (&(task->threads));
       thread != TH_NIL;
       thread  = (struct t_thread *) next_queue (&(task->threads)))
  {
    if (thread->action != AC_NIL)
      return (TRUE);

    if (events_for_thread (thread))
      return (TRUE);
  }

//    this task is finished
//    so check if the array of threads is freed, and if not, do it now
//    this here LEAKS -> BUT SO DO OTHER THINGS
//    EVERYTHING LEAKS -> WHO IS FREEING ALL THE THREADS????
//    TODO: I'm not freeing this array now -> because there might be a restart
//    if (task->num_of_threads != 0) {
//      freeame(/*(struct t_thread**)*/(char *) task->threads_array, sizeof(struct t_thread*) * task->num_of_threads);
//      task->num_of_threads = 0;
//   }
  
  return (FALSE);
}

t_boolean
more_actions_on_Ptask (struct t_Ptask *Ptask)
{
  struct t_task  *task;
  
  for (task  = (struct t_task *) head_queue (&(Ptask->tasks));
       task != T_NIL;
       task  = (struct t_task *) next_queue (&(Ptask->tasks)))
  {
    if (more_actions_on_task (task))
    {
      return (TRUE);
    }
  }
  return (FALSE);
}

void
clear_last_actions (struct t_Ptask *Ptask)
{
  struct t_task    *task;
  struct t_thread  *thread;
  struct t_node    *node;
  struct t_machine *machine;

  /* Locate task in Ptask */
  for (task  = (struct t_task *) head_queue (&(Ptask->tasks));
       task != T_NIL;
       task  = (struct t_task *) next_queue (&(Ptask->tasks)))
  {
    /* Locate thread onto that task */
    for (thread  = (struct t_thread *) head_queue (&(task->threads));
         thread != TH_NIL;
         thread  = (struct t_thread *) next_queue (&(task->threads)))
    {
      /*
      if (thread->last_action != AC_NIL)
        freeame (thread->last_action, sizeof (struct t_action));
      */
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

void
get_operation (struct t_thread *thread, struct t_fs_op *fs_op)
{
  struct t_action *action;
  
  action = thread->action;
  if (action != AC_NIL)
  {
    *fs_op = action->desc.fs_op;
    thread->action = action->next;
    freeame ((char *) action, sizeof (struct t_action));
  }
}

void
sddf_seek_next_action_to_thread (struct t_thread *thread)
{
  FILE            *file;
  struct t_action *action, *action2;
  int              thid, tid;
//   long             where;
  struct t_node   *node;

  node = get_node_of_thread(thread);
//mmap: kill these two lines
//   file = thread->task->Ptask->file;
//   file = thread->file;
  
  if (PREEMP_enabled && !PREEMP_initialized)
  {
    PREEMP_init(thread->task->Ptask);
    PREEMP_initialized = TRUE;
    if (debug)
    {
      PRINT_TIMER (current_time);
      printf("Preemption time initialization\n");
    }
  }

//mmap: kill this
//   if (file == thread->task->Ptask->file)
//   {
//     MYFSEEK(file, thread->seek_position, SEEK_SET);
//   }

  while (1)
  {
    if (binary_files)
    {
      action = get_next_action_from_file_binary (file, &tid, &thid, node);
    }
    else
    {
//mmap: I will pass thread instead of file and than see in get_next_action_from_file_sddf what to do
//       action = get_next_action_from_file_sddf (file, &tid, &thid, node,
//                                                thread->task->Ptask);
      action = get_next_action_from_file_sddf (thread, &tid, &thid, node,
                                               thread->task->Ptask);
//       printf("next action is %d", action->action);
    }
    
    if (action == AC_NIL)
    {
      thread->action = AC_NIL;
      break;
    }
    if ((tid == thread->task->taskid) && (thid == thread->threadid))
    {
      thread->action = action;

      if (action->action == WORK)
      {
        if (PREEMP_enabled)
          action->desc.compute.cpu_time += PREEMP_overhead(thread->task);
        recompute_work_upon_modules(thread, action);
      }
      else
        break;

      /*
      while (1)
      {
        if (binary_files)
        {
           action = get_next_action_from_file_binary (file, &tid, &thid, node);
        }
        else
        {
           action = get_next_action_from_file_sddf (file, &tid, &thid, node,
                                                    thread->task->Ptask);
        }

        if (action == AC_NIL)
          break;
        
        if ((tid == thread->task->taskid) && (thid == thread->threadid))
        {
          if (action->action == WORK)
          {
            if (PREEMP_enabled)
              action->desc.compute.cpu_time += PREEMP_overhead(thread->task);
            recompute_work_upon_modules(thread,action);
            thread->action->desc.compute.cpu_time += action->desc.compute.cpu_time;
          }
          else
          {
            thread->action->next = action;
            break;
          }
        }
        else
        {
          if (sorted_files)
          {
            freeame ((char *) action, sizeof (struct t_action));
            break;
          }
        }
        freeame ((char *) action, sizeof (struct t_action));
      }
      */
      break;
    }
    
    freeame ((char *) action, sizeof (struct t_action));
    if (sorted_files)
    {
      thread->action = AC_NIL;
      break;
    }
  }

//mmap: I have to setup pointers properly in get_next_action_from_file_sddf
//   if (file==thread->task->Ptask->file)
//   {
//     thread->seek_position = MYFTELL(file);
//   }
   
  /* Ensure last action is not a MPI IO action */
  action = thread->action;
  if (action == AC_NIL)
  {
    return;
  }
  
  while (action->next != AC_NIL)
    action = action->next;
  
  if ((action->action == MPI_IO))
  {
    action2 = thread->action;
    sddf_seek_next_action_to_thread (thread);
    action->next = thread->action;
    thread->action = action2;
  }
}

static int
//mmap: this I do not change because it is binary reading
locate_first_cpu_burst_binary (FILE *file, int taskid)
{
  struct t_disk_action da;
  int i, k;
  int a, b;

  MYFSEEK (file, 5, 0);  /* Byte number 5, because the SDDFB signature */
  while (1)
  {
    k = MYFTELL (file);
    i = fread (&da, sizeof (struct t_disk_action), 1, file);
    if (i != 1) {
        panic ("Can't locate actions for task %d\n", taskid);
    }

    if (taskid == da.taskid)
    {
     break;
    }

    if (taskid < da.taskid)
    {
    panic ("Trace file not properly sorted\n");
    }
  }
  return (k);
}

/*
void module_name(struct t_Ptask *Ptask,
                 long long       module_type,
                 long long       module_value,
                 char           *mod_name,
                 char           *activity_name,
                 int             src_file,
                 int             src_line)
{
  register struct t_module *mod;

  /*
  mod =
    (struct t_module*) find_module (&Ptask->Modules, (t_priority) block_id);

  mod = find_module(Ptask->Modules, module_type, module_value);
  
  if (mod == M_NIL)
  {
      
    mod = (struct t_module*) malloc (sizeof(struct t_module));
    mod->type  = module_type;
    mod->value = module_value;
    mod->ratio = 1.0;

		/*
    mod->block_name    = block_name;
    mod->activity_name = activity_name;
		
    
    mod->module_name    = (char*) malloc(strlen(module_name)+1);
    strcpy(mod->block_name, block_name);
    
    mod->activity_name = (char*) malloc(strlen(activity_name)+1);
    strcpy(mod->activity_name, activity_name);
    
    mod->src_file      = src_file;
    mod->src_line      = src_line;
    mod->used          = 0; /* De moment no ha estat utilitzat 

    /* DEBUG 
    printf("New module (%ld:%ld) Name = %s\n",
           mod->type,
           mod->value,
           mod->block_name);
    
    insert_queue (&Ptask->Modules, (char*) mod, (t_priority) block_id);
  }
  else
  {
    if (mod->block_name != (char*) 0)
    {
      printf ("Warning: redefinition of module %d\n",block_id);
    }
    else
    {
      free(mod->block_name);
      mod->block_name = strdup(block_name);
      /*
      mod->block_name    = (char*) malloc (strlen(block_name)+1);
      strcpy(mod->block_name, block_name);
      

      free(mod->activity_name);
      mod->activity_name = strdup(activity_name);
      /*
      mod->activity_name = (char*) malloc (strlen(activity_name)+1);
      strcpy(mod->activity_name, activity_name);
      
      
      mod->src_file      = src_file;
      mod->src_line      = src_line;
    }
  }
}
*/

void 
file_name (struct t_Ptask *Ptask, int file_id, char *location)
{
  register struct t_filed *filed;
  
  filed = 
    (struct t_filed*) query_prio_queue (&Ptask->Filesd, (t_priority)file_id);
  
  if (filed == F_NIL)
  {
    filed = (struct t_filed *)mallocame (sizeof(struct t_filed));
    filed->file_id = file_id;
    filed->location = location;
    insert_queue (&Ptask->Filesd, (char *)filed, (t_priority) file_id);
  }
  else
  {
    printf ("Warning: redefinition of file %d\n",file_id);
  }
}

static t_off_fitxer
//mmap: here I pass mmap_string instead of file
// locate_first_cpu_burst (
//   FILE           *file,
//   int             taskid,
//   int             th_id,
//   struct t_Ptask *Ptask)
locate_first_cpu_burst (
  char           *mmap_string,
  int             taskid,
  int             th_id,
  struct t_Ptask *Ptask)
//     t_boolean      *found_seek_info)
{
  char            buf[BUFSIZE];
//   int             i;
//   t_off_fitxer    k;
  int             a, b;
  int comm_id, root_rank, j;
  char *c;
  int block_id, src_file, src_line;
  char block_name[256], activity_name[256];
  char *bn, *an;
  int burst_category_id;
  double burst_category_mean, burst_category_std_dev;
  t_off_fitxer actual_seek_offset = 0;

  
  actual_seek_offset = 0;
  char *new_p;
  new_p = mmap_string;
  int pos = 0;
  
//   printf("to be put in the header of the .prv file \n____________________________________________\n\n(");
  while (1) {    
    int i = sscanf (new_p, "\n%[^\n]\n", buf);
    if (i==-1) {
      printf("%d:0)\n----------------------------------------------------------------------------------------\n", th_id - 1);
      return 0;
    }
    
    Ptask->is_there_seek_info = 0; // assume there is no seek info and see if there actually is
    
    i = sscanf (buf, "\"CPU burst\" { %d, %d,", &a, &b);

//if it is CPU burst and we are in the same task but we have found a new thread - THE ONE WE WERE LOOKING FOR!!!
    if ((i == 2) && ((taskid == a + 1) && (th_id == b + 1))) {
//       printf("we have found another thread and string is %s\n ", buf);
      break;
    }

//we have found an event from the next task - so we can conclude how many threads there is in the actual task
    if ((i == 2) && ((taskid == a))) {
      printf("%d:0,", th_id - 1);
      return 0;
    }   

    if ((i==2) && (taskid < a))
    {
      panic (
       "Trace file not properly sorted. Taskid %d and record with %d\nRecord: %s",
       taskid, a, buf
      );
    }

    { /* Burst category definition */
    i = sscanf ( buf,
                 "\"burst category definition\" { %d, %le, %le };;\n",
                 &burst_category_id,
                 &burst_category_mean,
                 &burst_category_std_dev);
    
    if (i == 3)
    {
      char *clusterName;
      
      if (synthetic_bursts == FALSE)
      {
        user_event_type_name(Ptask, 90000001, "Cluster ID", 9);
        user_event_value_name(Ptask,
                              90000001,
                              0,
                              "End");
      }
      
      SYNT_BURST_add_new_burst_category(burst_category_id,
                                        burst_category_mean,
                                        burst_category_std_dev);
      
      clusterName = malloc(128);
      
      if (clusterName == NULL)
      {
        panic("Unable to allocate memory to cluster names");
      }
      
      
      sprintf(clusterName, "Cluster %02d", burst_category_id+1);
      
      user_event_value_name(Ptask,
                            90000001,
                            burst_category_id+1,
                            clusterName);
      
    }
    }

    { /* Block definition */
    i = sscanf(buf, 
               "\"block definition\" { %d, \"%[^\"]\", \"%[^\"]\", %d, %d};;\n",
               &block_id, block_name, activity_name,
               &src_file, &src_line);
    
    if (i == 5)
    { /* DEPRECATED
      bn = (char *)mallocame (strlen(block_name)+1);
      strcpy (bn, block_name);
      an = (char *)mallocame (strlen(activity_name)+1);
      strcpy (an, activity_name);
      module_name(Ptask, block_id, bn, an, src_file, src_line);
      */
    }
    }

    { /* User event type definition */
    i = sscanf(buf,
               "\"user event type definition\" { %d, \"%[^\"]\", %d};;\n",
               &block_id,  /* user event type */
               block_name, /* user event type name */
               &src_line); /* user event type flag color */
    
    if (i == 3)
    {
      bn = (char *)mallocame (strlen(block_name)+1);
      strcpy (bn, block_name);
      user_event_type_name(Ptask, block_id, bn, src_line);
    }
    }

    { /* User event value definition */
    i = sscanf(buf,
               "\"user event value definition\" { %d, %d, \"%[^\"]\"};;\n",
               &block_id,   /* user event type */
               &src_line,   /* user event value */
               block_name); /* user event value name */
    
    if (i == 3)
    {
      bn = (char *)mallocame (strlen(block_name)+1);
      strcpy (bn, block_name);
      user_event_value_name(Ptask, block_id, src_line, bn);
    }
    }
    { /* File definition */
    i = sscanf (buf,
                "\"file definition\" { %d, \"%[^\"]\"};;\n",
                &block_id,
                block_name);

    if (i == 2)
    {
      bn = (char *)mallocame (strlen(block_name)+1);
      strcpy (bn, block_name);
      file_name(Ptask, block_id, bn);
    }
    }
    { /* Global OP definition */
    i = sscanf (buf,
                "\"global OP definition\" { %d, \"%[^\"]\"};;\n",
                &block_id,
                block_name);
    
    if (i == 2)
    {
      bn = (char *)mallocame (strlen(block_name)+1);
      strcpy (bn, block_name);
      new_global_op(block_id, bn);
    }
    }
    { /* Communicator definition */
    i = sscanf (buf,
                "\"communicator definition\" { %d, %d, [%*d] {%[^}]}};;\n",
                &comm_id,
                &root_rank,
                BIGPOINTER2);

    if (i == 3)
    {
      new_communicator_definition(Ptask, comm_id);
      c = strtok (BIGPOINTER2, ",}");
      for (i=0;i<root_rank; i++)
      {
        j = atoi(c);
        add_identificator_to_communicator(Ptask, comm_id, j);
        c = strtok (NULL, ",}");
      }
      no_more_identificator_to_communicator(Ptask, comm_id);
    }
    }
    { /* IO OP definition */
    i = sscanf (buf,
                "\"IO OP definition\" { %d, \"%[^\"]\"};;\n",
                &block_id,
                block_name);

    if (i == 2)
    {
      new_io_operation(block_id, block_name);
    }
    }
    
    
    
//     { /* seek info - Vladimir */
//     i = sscanf (buf,
//                 "\"seek info\" { %d, %d",
//                 &read_task,
//                 &read_threads_no);
// 
//     if (i == 2)
//     {
//       
//       strcpy(BIGPOINTER2, buf);
//       char *slider;
//       slider = buf;
//       char temp_string[BUFSIZE];
//       sprintf(temp_string, "\"seek info\" { %d, %d",
//                 read_task,
//                 read_threads_no);
// /*      printf("\"seek info\" { %d, %d\n",
//                 read_task,
//                 read_threads_no);	*/	
// //       printf("ceo buf je %s\n", slider);
//       unsigned long partial_offset = ((unsigned long long)strstr(slider, temp_string) + (unsigned long long)strlen(temp_string)) - (unsigned long long)slider;
//       slider = slider + partial_offset;
// //       printf("pomeren slider je sad %s\n", slider);
// 
//       struct t_task *task;
//       struct t_thread *thread;
//       t_off_fitxer partial_seek_offset;
// //       printf("start and actual_seek_offset is %lu\n", actual_seek_offset);
// //  THIS SHOULD BE MADE ASSUMING THAT THE SEEK INFO IS COMPLETE SO IT CAN READ No_TASKS LINES WHILE IT IS IN THE LOOP
// //    AND TO CHECK IF THE SEEK INFO WAS COMPLETE
//       *found_seek_info = TRUE;
//       for (task  = (struct t_task *) head_queue (&(Ptask->tasks));
// 	  task != T_NIL;
// 	  task  = (struct t_task *) next_queue (&(Ptask->tasks)))
//       {
// 	if (task->taskid != read_task + 1)
// 	    continue;
// 	printf("taskid is %d\n", task->taskid);	
// // 	int j;
// // 	for (j=0; j<read_threads_no - 1; j++)
// 	for (thread  = (struct t_thread *) head_queue (&(task->threads));
// 	    thread != TH_NIL;
// 	    thread  = (struct t_thread *) next_queue (&(task->threads)))
// 	{
//           thread->mmap_position = actual_seek_offset;
// 	  printf("task %d thread %d   has the offset %lu\n", task->taskid, thread->threadid, thread->mmap_position);
// 	  sscanf(slider, ", %lu", &partial_seek_offset);
// 	  actual_seek_offset += partial_seek_offset;
// 	  sprintf(temp_string, ", %d", partial_seek_offset);
// 	  partial_offset = ((unsigned long long)strstr(slider, temp_string) + (unsigned long long)strlen(temp_string)) - (unsigned long long)slider;
// 	  slider = slider + partial_offset;
// // 	  printf("pomeren slider je sad %s\n", slider);
// 	  
// 	  read_threads_no--;
// // 	  printf("threads_left is %d\n", read_threads_no);
// 	  if (read_threads_no == 0)
// 	    break;
// 	  add_thread_to_task (task, thread->threadid + 1, task->nodeid);
//   // 	  sscanf (bigpointer, "%llu %[^\n]\n", &thread->seek_position, d);
//   // 	  strcpy (bigpointer, d);
// 	}
//       }      
//       
// //       new_communicator_definition(Ptask, comm_id);
// //       c = strtok (BIGPOINTER2, ",}");
// //       for (i=0;i<root_rank; i++)
// //       {
// //         j = atoi(c);
// //         add_identificator_to_communicator(Ptask, comm_id, j);
// //         c = strtok (NULL, ",}");
// //       }
// //       no_more_identificator_to_communicator(Ptask, comm_id);
//     }
//     }
    
        
//move to the next line of the string
    pos += ((ptrdiff_t)strstr(new_p, buf) + (ptrdiff_t)strlen(buf)) - (ptrdiff_t)new_p;
    new_p = mmap_string + pos;    
  }    

//   printf("the first cpu burst is  ----------    %s\n", buf);	
  return pos;
}

static t_off_fitxer
fast_locate_first_cpu_burst (char *p,
                             int taskid,
                             int th_id,
                             struct t_Ptask *Ptask,
                   			 t_off_fitxer max_size)
{
  char         buf[BUFSIZE];
  int          a, b;
  t_off_fitxer p_ini, p_inf, p_actual, p_sup, slow_offset;

/* Aixo es la maxima distancia que hi pot haver entre la cota inferior i la 
 superior per tal de comenc,ar a buscar record a record. No pot ser molt gran 
 perque no hi guanyariem res. Pero ha de ser prou gran per tal que hi hagui com 
 a minim un cpuburst perque sino la cerca no acabaria mai. */
  
#define DISTANCIA_CERCA_PAS_A_PAS (t_off_fitxer)(BUFSIZE)

#define SITUA_INICI_SEGUENT_POSICIO(pos) \
  MYFSEEK (file,pos,SEEK_SET); 
/* 
#define DEBUGA_CERCA_PRIMER_CPUBURST 1 */


#ifdef DEBUGA_CERCA_PRIMER_CPUBURST
  fprintf(stderr,"\tBuscant cpu burst de la task %d\n",taskid);
#endif  

  p_ini    = 0; /* Es guarda la posicio inicial */
  p_inf    = 0; /* Aquesta sera la cota inferior */
  p_actual = 0;

  char *new_p;
  new_p = p;
  p_sup = max_size; /* S'obtre l'ultima posicio possible */

  /* this is from where to start searching */
  new_p = p + p_ini;

#if DEBUGA_CERCA_PRIMER_CPUBURST  
  printf("\t\tp_ini %lu, p_sup %lu, taskid %d, threadid %d  max_size %lu\n",p_ini, p_sup, taskid, th_id, max_size);
#endif

  while (1)
  {
    int i = sscanf (new_p, "\n%[^\n]\n", buf);

    if (i == -1)
    {
      printf("Can't locate actions for task %d thread %d in fast_locate_first_cpu_burst\n", taskid,th_id);
      return -1;
    }
    
    i = sscanf (buf, "\"CPU burst\" { %d, %d,", &a, &b);
    if (i != 2)
    {
       i = sscanf (buf, "\"NX recv\" { %d, %d,", &a, &b);
    }

    if (i != 2)
    {
      i = sscanf (buf, "\"NX send\" { %d, %d,", &a, &b);
    }

    if (i != 2)
    {
      i = sscanf (buf, "\"user event\" { %d, %d,", &a, &b);
    }

    if (i == 2) 
    {
      if ((taskid < a + 1) || ((taskid == a+1) && (th_id <= b + 1)))   //(taskid <= a + 1)
      {
        p_sup = p_actual;
        if ((p_sup - p_inf) <= DISTANCIA_CERCA_PAS_A_PAS)
        { 
          break;
        }
        else
        {
          p_actual = p_inf + (p_sup - p_inf)/16;
          new_p = p + p_actual;
        }
      }
      else /* taskid > a+1 */
      {
        p_inf = p_actual;
        if ((p_sup - p_inf) <= DISTANCIA_CERCA_PAS_A_PAS)
        { 
          break;
        }
        else
        {
          p_actual = p_inf + (p_sup - p_inf)/16;
          new_p = p + p_actual;
        }
      }
    }
    else
    {
    	//maybe in the middle of the line --- jump over the whole line and inspect the next one that is COMPLETE
      p_actual += ((ptrdiff_t)strstr(new_p, buf) + (ptrdiff_t)strlen(buf)) - (ptrdiff_t)new_p;
      new_p = p + p_actual;
      continue;
    }
  }
   
  slow_offset = locate_first_cpu_burst (p + p_inf, taskid, th_id, Ptask);
  if (slow_offset == 0)
    return 0;
  else return (p_inf + slow_offset);
//   return p_inf + locate_first_cpu_burst (p + p_inf, taskid, th_id, Ptask); //, &dummy_seek_info);
}

static t_boolean
is_there_lseek_info (FILE *fp, struct t_Ptask *Ptask)
{
  assert(fp != NULL);
  assert(Ptask != NULL);
  #ifdef READ_TRACE_SEEKS
  t_boolean exist_info = FALSE;
  char *bigpointer;
  char *c, *d;
  struct t_task *task;
  struct t_thread *thread;

  bigpointer = (char *)mallocame (BUFSIZE);
  d = (char *)mallocame (BUFSIZE);
  MYFSEEK (fp, -BUFSIZE, SEEK_END);
  fread (bigpointer, 1, BUFSIZE, fp);
  bigpointer [BUFSIZE-1] = (char)0;
  c = strstr (bigpointer, "SEEK ");

  if (c!=(char *)0)
  {
    sscanf (c, "SEEK %[^\n]\n", d);
    strcpy (bigpointer, d);
    for (task  = (struct t_task *) head_queue (&(Ptask->tasks));
         task != T_NIL;
         task  = (struct t_task *) next_queue (&(Ptask->tasks)))
    {
      for (thread  = (struct t_thread *) head_queue (&(task->threads));
           thread != TH_NIL;
           thread  = (struct t_thread *) next_queue (&(task->threads)))
      {
        /* FEC: Ara aixo ja no va.
        sscanf (bigpointer, "%d %[^\n]\n", &thread->seek_position, d);
        Ara s'hauria d'utilitzar aixo:
        sscanf (bigpointer, "%llu %[^\n]\n", &thread->seek_position, d);
        Pero aixo pot donar problemes a les compaq. */
        sscanf (bigpointer, "%llu %[^\n]\n", &thread->seek_position, d);
        strcpy (bigpointer, d);
        /* 
        fprintf(stderr,
                "Task %d thread %d posicio %d\n",
                task->taskid,
                thread->threadid,
                thread->seek_position);
        */
      }
    }
    exist_info = TRUE;
  }

  MYFSEEK (fp, 0, SEEK_SET);
  freeame (bigpointer, BUFSIZE);
  freeame (d, BUFSIZE);
  return (exist_info);
  #else
  return(FALSE);
  #endif
}

void sddf_load_initial_work_to_threads (struct t_Ptask *Ptask)
{
  struct t_task   *task;
  struct t_thread *thread, *thread_check;
  char             buf[BUFSIZE];
  int              trf_file;
  unsigned long    current_pos;
  
  trf_file = open (Ptask->tracefile, O_RDONLY);
  
  if (trf_file == -1) {
    panic ("Cant't open trace file %s\n", Ptask->tracefile);
  }    
  
  if (fstat (trf_file, &(Ptask->sb)) == -1) {
     panic ("fstat\n");
     return;
  }

  if (!S_ISREG (Ptask->sb.st_mode)) {
    panic ("%s is not a file\n", Ptask->tracefile);
    return;
  }

//Ptask->mmapped_file always points to the start of the ORIGINALY mmapped string - so we can munmap it at the endif
//Ptask->mmap_position points to the beginning of the last thread that we look for - so we do not start from the beginning every time
  Ptask->mmapped_file = mmap(0,
                             Ptask->sb.st_size,
                             PROT_READ,
                             MAP_SHARED,
                             trf_file,
                             0);
  Ptask->mmap_position = 0;
  if (Ptask->mmapped_file == MAP_FAILED) {
    panic ("mmap\n");
    return;
  }

  if (close (trf_file) == -1) {
    panic ("close in mmap\n");
    return;
  }

  /* Local io_buf for each file */
//mmap: i removed this three lines
//   char            *local_io_buf;
//   local_io_buf = mallocame (BUFSIZE);
//   setvbuf (Ptask->file, local_io_buf, _IOFBF, BUFSIZE);
//   have_lseek_info = is_there_lseek_info (Ptask->file, Ptask);

//mmap: this part has to skip the descriptive part of the header of trf_file
//   if ((!binary_files))
//   {
//     while (fgets (buf, BUFSIZE, Ptask->file) != NULL)
//     {
//       i = strlen (buf);
//       if ((i > 3) && (buf[0] == '\"') && 
// 	  (buf[i - 2] == ';') && (buf[i - 3] == ';'))
//       {
// 	break;
//       }
//       j = MYFTELL (Ptask->file);
//     }
//     MYFSEEK (Ptask->file, j, SEEK_SET);
//   }

//mmap: this is my change for the part that skips descriptive part of the trf_file
  if ((!binary_files))
  {
    current_pos=0;
    char *new_p = Ptask->mmapped_file;

    while (1) {    
    
      int i = sscanf (new_p, "\n%[^\n]\n", buf);
      
      if (i==-1) {
         printf("!!!!this is error, looked for the useful part of the header and came till the end of the file\n");
         break;
      }
      
// found the end of the descriptive part when there is 4 consecutive \n
      if (strstr(new_p, buf) - new_p == 4) {
         printf("skipped the beginning of the header and now to read the useful data\n\n");
         printf("this is to be put in the header of the .prv file \n----------------------------------------------------------------------------------------\n      (");
         break;
      }
      
//iterate - take another line of trf_file      
      current_pos += ((ptrdiff_t)strstr(new_p, buf) + (ptrdiff_t)strlen(buf)) - (ptrdiff_t)new_p;
      new_p = Ptask->mmapped_file + current_pos;
    }
  }
  Ptask->mmap_position = current_pos;

  /* To analyze all the block definition */
  /* FEC: Ara aixo s'ha de fer sempre per poder llegir les definicions:
  if (have_lseek_info==TRUE) */
//   printf("looking for the first cpu busrt for the first thread\n");
  Ptask->mmap_position += locate_first_cpu_burst (Ptask->mmapped_file + Ptask->mmap_position, 1, 1, Ptask); 
//   printf("found and found_seek_info is  %s\n",(found_seek_info)?"true":"false");
  
  
//this is a trick so if the first event is found the offset has to be higher than 0  
  Ptask->mmap_position-=3;
//put for every thread the mmap pointer to the first CPU burst event of that thread
  for ( 
    task  = (struct t_task *) head_queue (&(Ptask->tasks));
    task != T_NIL;
    task  = (struct t_task *) next_queue (&(Ptask->tasks)))
  {
    for (
      thread  = (struct t_thread *) head_queue (&(task->threads));
      thread != TH_NIL;
      thread  = (struct t_thread *) next_queue (&(task->threads)))
    {
      if (debug)
      {
        fprintf(stdout,
                "   * Locating first record for Task %02d\n",
                task->taskid);
      }
//thread->mmapped_file is pointing to the beginning of the line where the first CPU burst of that thread is
      thread->mmapped_file = Ptask->mmapped_file + Ptask->mmap_position;
      thread->mmap_position = 0;
      
//I know that only thread no 1 in every task is long, the others are always short
//       printf("looking for the start of thread no %d\n", thread->threadid);
      if (thread->threadid == 2)
        current_pos = fast_locate_first_cpu_burst (thread->mmapped_file,
						   task->taskid,
     						   thread->threadid,
						   Ptask,
						   Ptask->sb.st_size - Ptask->mmap_position);
      else
         current_pos = locate_first_cpu_burst (thread->mmapped_file,
						   task->taskid,
     						   thread->threadid,
						   Ptask); 
      thread->mmapped_file += current_pos;
      Ptask->mmap_position += current_pos;
      
      if (current_pos != 0) {
//we have found the thread that we supposed there is - validate it - leave it on the list of all threads in the task
//now try to see if there are more threads in this task - supose there is and than check it
         add_thread_to_task (task, thread->threadid + 1, task->nodeid);
      } else {
//we supposed that there will be more threads in this task - but this is not true - remove this anticipated thread
	  thread_check =  (struct t_thread *)outLIFO_queue(&(task->threads));
	  if (thread != thread_check)
	      panic("\nABORT --- (thread != thread_check)\n\n");
	  break;
      }
    }
  }

  printf("some write traces seek\n");

  #ifdef WRITE_TRACE_SEEKS
  t_boolean        have_lseek_info = FALSE;
  if ((have_lseek_info==FALSE) && (binary_files==FALSE))
  {
    FILE *tmp_file;
    tmp_file = MYFOPEN (Ptask->tracefile, "a");
    /* tmp_file=stderr; */
    if (tmp_file!=(FILE *)0)
    {
      fprintf (tmp_file, "/* SEEK ");
      for ( task  = (struct t_task *) head_queue (&(Ptask->tasks));
            task != T_NIL;
            task  = (struct t_task *) next_queue (&(Ptask->tasks)))
      {
        for ( thread  = (struct t_thread *) head_queue (&(task->threads));
              thread != TH_NIL;
              thread  = (struct t_thread *) next_queue (&(task->threads)))
        {
          fprintf (tmp_file,"%llu ", thread->seek_position);
        }
      }
      fprintf (tmp_file, " */\n\n");
      fclose(tmp_file);
    }
  }
  #endif


  for ( task  = (struct t_task*) head_queue (&(Ptask->tasks));
        task != T_NIL;
        task  = (struct t_task*) next_queue (&(Ptask->tasks)))
  {
    for ( thread  = (struct t_thread*) head_queue (&(task->threads));
          thread != TH_NIL;
          thread  = (struct t_thread*) next_queue (&(task->threads)))
    {
      sddf_seek_next_action_to_thread (thread);
      
      /* When using synthetic bursts, first action isn't work */
      if (synthetic_bursts == TRUE && thread->action->action != WORK)
      {
        struct t_action *tmp_action;
        tmp_action = thread->action;

        thread->action = tmp_action->next;

        free(tmp_action);
      }
    }
  }
  printf("FIHISHED loading initial work to threads\n");
}

t_micro
work_time_for_sintetic ()
{
  t_micro f;

  f = (t_micro) unform (MIN_SERVICE_TIME,
                        MAX_SERVICE_TIME);
  return (f);
}

void
First_action_to_sintetic_application (struct t_Ptask *Ptask)
{
  struct t_task   *task;
  struct t_thread *thread;
  struct t_action *action;

  /* Locate task in Ptask */
  for(task  = (struct t_task *) head_queue (&(Ptask->tasks));
      task != T_NIL;
      task = (struct t_task *) next_queue (&(Ptask->tasks)))
  {
    /* Locate thread onto that task */
    for(thread  = (struct t_thread *) head_queue (&(task->threads));
        thread != TH_NIL;
        thread  = (struct t_thread *) next_queue (&(task->threads)))
    {
      action = (struct t_action *) mallocame (sizeof (struct t_action));
      action->action = WORK;
      (action->desc).compute.cpu_time = work_time_for_sintetic ();
      (action->desc).compute.cpu_time = (t_micro) unform ((float) 0, 
                                                          MAX_SERVICE_TIME);
      thread->action = action;
      thread->seek_position = 0;
    }
  }
}

void
create_sintetic_applications (int num)
{
  int             i, j;
  struct t_Ptask *Ptask;

  for (i = 0; i < num; i++)
  {
    Ptask = create_Ptask ((char *) 0, (char *) 0);
    Ptask->synthetic_application = TRUE;
    for (j = 1; j <= greatest_cpuid; j++)
    {
      new_task_in_Ptask (Ptask, j, j);
      
    }
    First_action_to_sintetic_application (Ptask);
    inFIFO_queue (&Ptask_queue, (char *) Ptask);
  }
}

t_boolean
more_actions_to_sintetic (struct t_thread *thread)
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

  action = (struct t_action *) mallocame (sizeof (struct t_action));
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

struct t_node*
get_node_for_task_by_name (struct t_Ptask *Ptask, int taskid)
{
  struct t_task  *task;
  struct t_node  *node;

  /* Locate task in Ptask */
  for(task  = (struct t_task *) head_queue (&(Ptask->tasks));
      task != T_NIL;
      task  = (struct t_task *) next_queue (&(Ptask->tasks)))
  {
    if (task->taskid == taskid)
      break;
  }

  if (task == T_NIL)
    panic ("Incorrect task identifier %d in trace file %s for P%d in get_node_for_task_by_name\n",
           taskid, Ptask->tracefile, Ptask->Ptaskid);

  node = get_node_of_task (task);
  return (node);
}

void module_new(struct t_Ptask *Ptask,
                long long       module_type,
                long long       module_value,
                double          ratio)
{
  struct t_module *mod;

  if (module_value == 0)
  {
    fprintf(stderr,
            "Unable to add new module [%ld] with value 0\n",
            module_type);
    return;
  }
  
  if (debug&D_TASK)
  {
    printf ("New module: [%ld:%ld] with ratio %f\n",
            module_type,
            module_value,
            ratio);
  }

  // mod = (struct t_module *) query_prio_queue (&Ptask->Modules, (t_priority)identificator);

  mod = find_module (&Ptask->Modules, module_type, module_value);
  
  if (mod == M_NIL)
  {
    mod = (struct t_module *)mallocame (sizeof(struct t_module));
    mod->type          = module_type;
    mod->value         = module_value;
    mod->ratio         = ratio;
    mod->module_name   = (char *)0;
    mod->activity_name = (char *)0;
    mod->src_file      = -1;
    mod->src_line      = -1;
    mod->used          =  0; /* De moment no ha estat utilitzat */
    
    insert_module (&Ptask->Modules, module_type, module_value,  mod);
  }
  else
  {
    fprintf (stderr, "Module [%ld:%ld] already exists. Old ratio %f, new one %f\n",
              module_type, module_value, mod->ratio, ratio);
    
    mod->ratio = ratio;
  }
}

void module_entrance(struct t_thread *thread,
                     long long        module_type,
                     long long        module_value)
{
  register struct t_module *mod;
  register struct t_Ptask  *Ptask = thread->task->Ptask;

  /*
  if (debug&D_TASK)
  {
    PRINT_TIMER (current_time);
    printf (": Going into module: %d for P%d T%d th%d\n",identificator,
            Ptask->Ptaskid, tid, thid);
  }
  */

  // thread = locate_thread (Ptask, tid, thid);

  /*
  mod = (struct t_module*) query_prio_queue(&Ptask->Modules, 
                                            (t_priority)identificator);
  */

  /* Check if its an idle block */
  if (module_type == IDLE_EVENT_TYPE)
  {
    
    thread->idle_block = TRUE;
  }

  mod = find_module(&Ptask->Modules, module_type, module_value);
  
  if (mod != M_NIL)
  {
    mod->used = 1; /* Ara ja ha estat utilitzat! */
    inLIFO_queue (&(thread->modules), (char *)mod);
    
    if (debug&D_TASK)
    {
      PRINT_TIMER (current_time);
      printf (": Going into module [%ld:%ld] (%s) for P%d T%d th%d\n",
              mod->type,
              mod->value,
              mod->module_name,
              IDENTIFIERS(thread));
    }
  }

}

int module_exit (struct t_thread* thread,
                 long long        module_type)
{
  register struct t_Ptask  *Ptask = thread->task->Ptask;
  register struct t_module *mod;
  register struct t_thread *awaked;
  register struct t_action *action;
  register t_micro ti, bandw;
  struct t_node *node;
  float fo;
  struct t_machine *machine;
  
  // thread = locate_thread (Ptask, tid, thid);
  
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
      bandw = (t_micro) machine->communication.remote_bandwith;
      if (bandw!=0)
      {
        bandw = (t_micro) ((t_micro) (1000000) / (1 << 20) / bandw);
        /*ti += bandw*1024;*/
      }
      fo = log((t_micro)count_queue(&(thread->task->Ptask->tasks)))/log(2.0);
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
    
    mod = (struct t_module *) outFIFO_queue(&(thread->modules));

    /* JGG (2012/01/10): now that makes no sense 
    if (mod == M_NIL)
    {
      
      /*
       * FEC: Converteixo aquest panic en un Warning perque si la trac,a s'ha 
       * obtingut a partir de tallar una trac,a paraver, es possible que comenci 
       * amb alguns block end sense que hi haguin hagut els corresponents block
       * begin.
      panic("Exiting module %d, but no information recorderd P%d T%d th%d\n",
            identificator,IDENTIFIERS(thread));
     
      fprintf(stderr,
              "WARNING: Exiting module [%ld], but no information recorderd P%d T%d th%d\n",
              module_type,
              IDENTIFIERS(thread));
     
    }
    
    else */

    if (mod != M_NIL)
    {
      if (mod->type != module_type)
      {
        inFIFO_queue (&(thread->modules), mod);
      }
      /*
      if (mod->type != module_type)
      {
        panic("Exiting module [%ld], but this is not the current one [%ld:%ld] for P%d T%d th%d\n",
              module_type,
              mod->type,
              mod->value,
              IDENTIFIERS(thread));
      }
      else
      {
      */
      if (debug&D_TASK)
      {
        PRINT_TIMER (current_time);
        printf (": Going out module: [%ld] (Unknown) for P%02d T%02d th%02d\n",
                module_type,
                IDENTIFIERS(thread));
      }
      //}
    }
    
#ifdef BLOCK_SPECIAL
  }
#endif

  /*
  if (debug&D_TASK)
  {
    if (mod != M_NIL)
    {
      PRINT_TIMER (current_time);
      printf (": Going out module: [%ld:%ld] (%s) for P%02d T%02d Th%02d\n",
              mod->type,
              mod->value,
              mod->module_name,
              IDENTIFIERS(thread));
    }
    else
    {
      
      PRINT_TIMER (current_time);
      printf (": Going out module: [%ld] (Unknown) for P%02d T%02d th%02d\n",
              module_type,
              IDENTIFIERS(thread));
    }
  }
  */
  
  return(0);
}

void recompute_work_upon_modules(struct t_thread *thread, struct t_action *action)
{
  register struct t_module *mod;
   
  if (action->action != WORK)
  {
    panic("Improper call to recompute_work_upon_modules\n");
  }
   
  mod = (struct t_module*) head_queue(&(thread->modules));
  
  if (mod != M_NIL)
  {
    if (mod->ratio == 0.0)
    {
      action->desc.compute.cpu_time = 0.0;
    }
    else
    {
      action->desc.compute.cpu_time = action->desc.compute.cpu_time/mod->ratio;
    }
    /*
    if (action->desc.compute.cpu_time<1.0)
      action->desc.compute.cpu_time = 1.0
    */
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
      mallocame(sizeof(struct t_user_event_info));

    evinfo->type  = type;
    evinfo->color = color;
    evinfo->name  = name;
    create_queue (&(evinfo->values));
    insert_queue (&Ptask->UserEventsInfo, (char*)evinfo, (t_priority) type);
  }
  else
  {
    printf ("Warning: redefinition of user event type %d\n",type);
    freeame(name, strlen(name)+1);
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
    freeame(name, strlen(name)+1);
    return;
  }

  /* Es mira si ja te definit aquest valor */
  evinfo_val = (struct t_user_event_value_info*)
    query_prio_queue(&evinfo->values, (t_priority)value);

  if (evinfo_val == (struct t_user_event_value_info*) 0)
  {
    evinfo_val = (struct t_user_event_value_info*)
      mallocame (sizeof(struct t_user_event_value_info));

    evinfo_val->value = value;
    evinfo_val->name  = name;
    insert_queue (&evinfo->values, (char*) evinfo_val, (t_priority) value);
  }
  else
  {
    printf ("Warning: redefinition of user event value %d for type %d\n",
            value,
            type);
    freeame(name,strlen(name)+1);
  }
}
