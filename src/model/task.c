char task_c_rcsid[]="$Id: task.c,v 1.28 2007/04/20 12:42:46 jgonzale Exp $";
/*
 * Programming model routines
 * 
 * Sergi Girona (sergi@ac.upc.es)
 * 
 * (c) DAC-UPC 1993-94
 * 
 */
/*#define BLOCK_SPECIAL 1*/

#include <math.h>
#include <string.h>

#if defined(ARCH_MACOSX) || defined(ARCH_CYGWIN)
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
#include "vampir.h"
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
    if (Ptask->file != NULL)
    {
      fclose (Ptask->file);
      Ptask->file = NULL;
    }
    
    for (communicator=(struct t_communicator *)head_queue(&Ptask->Communicator);
         communicator!=(struct t_communicator *)0;
         communicator=(struct t_communicator *)next_queue(&Ptask->Communicator))
    {
      if (count_queue(&communicator->threads)!=0)
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
          
          printf (" Sender: T%02d Tag: %02d CommId: %02d Size: %d\n",
                  mess->ori,
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
          mess_source = &(action->desc.recv);
          
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
          printf ("\t-> Sender: T%02d Tag: %02d CommId: %d Size: %d\n",
                  thread->task->taskid, 
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
  Vampir_communicator(communicator_id,
                      count_queue(&comm->global_ranks),
                      count_queue(&comm->global_ranks),
                      trips);
  
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
  int *mtaskid;
  int  i;
  int *trips;

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
  Ptask->file = (FILE *) NULL;
  Ptask->synthetic_application = FALSE;
  create_queue (&(Ptask->tasks));
  create_queue (&(Ptask->global_operation));
  create_queue (&(Ptask->Communicator));
  create_queue (&(Ptask->Window));
  create_queue (&(Ptask->MPI_IO_fh_to_commid));
  create_queue (&(Ptask->MPI_IO_request_thread));
  create_queue (&(Ptask->Modules));
  create_queue (&(Ptask->Filesd));
  create_queue (&(Ptask->UserEventsInfo));
  return (Ptask);
}

/*
 * Set to 0 an account entry
 */
void
clear_account (struct t_account *account)
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

/*
 * Create a task and all threads in tasks
 */
void
new_thread_in_Ptask (struct t_Ptask *Ptask, int taskid, int nodeid, 
                     int number_of_threads, int priority, int where)
{
  struct t_thread *thread;
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
    task->Ptask = Ptask;
    task->io_thread = FALSE;
    create_queue (&(task->threads));
    create_queue (&(task->mess_recv));
    create_queue (&(task->recv));
    create_queue (&(task->send));
    /* FEC: Cal crear les dues noves cues pels Irecv*/
    create_queue (&(task->recv_without_send));
    create_queue (&(task->send_without_recv));
    inFIFO_queue (&(Ptask->tasks), (char *) task);
  }

  /* Insert new thread in task */
  for (i = 0; i < number_of_threads; i++)
  {
    thread = (struct t_thread*) mallocame (sizeof (struct t_thread));
    create_queue (&(thread->account));
    create_queue (&(thread->modules));
    create_queue (&(thread->Activity));
    thread->threadid = i + 1;
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
    thread->original_seek = where;
    thread->seek_position = where;
    thread->base_priority = priority;
    
    thread->portid = port_ids++;
    PORT_create (thread->portid, thread);
  
    thread->portid = port_ids++;
    PORT_create (thread->portid, thread);
  
    thread->portid = port_ids++;
    PORT_create (thread->portid, thread);
  
    new_account (&(thread->account), nodeid);
    inFIFO_queue (&(task->threads), (char *) thread);
    
    thread->last_cp_node = (struct t_cp_node *)0;
    thread->global_op_done = FALSE;
  
    ASS_ALL_TIMER (thread->last_time_event_number,0);
  
    thread->marked_for_deletion = 0;
    thread->file_shared         = FALSE;
  }
}

struct t_thread*
locate_thread (struct t_Ptask *Ptask, int taskid, int thid)
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

  /* Locate thread onto that task */
  for(thread  = (struct t_thread *) head_queue (&(task->threads));
      thread != TH_NIL;
      thread  = (struct t_thread *) next_queue (&(task->threads)))
  {
    if (thread->threadid == thid)
      break;
  }

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
void
new_action_to_thread (
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
  return ((struct t_account *) tail_queue (&(thread->account)));
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

void
delete_duplicate_thread (struct t_thread *thread)
{
  struct t_account *acc_th_copia, *acc_th_original;
  
  SCHEDULER_free_parameters (thread);

  /* Intent d'afegir les dades d'accounting al thread original. */
  acc_th_copia = current_account (thread);
  acc_th_original = current_account (thread->twin_thread);
  add_account(acc_th_original,acc_th_copia);
  extract_from_queue(&(thread->account), (char*) acc_th_copia);
  free(acc_th_copia);
 
  freeame ((char*) thread->action, sizeof (struct t_action));
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
  long             where;
  struct t_node   *node;

  node = get_node_of_thread(thread);
  file = thread->task->Ptask->file;
  file = thread->file;
  
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

  if (file == thread->task->Ptask->file)
  {
    MYFSEEK(file, thread->seek_position, SEEK_SET);
  }

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

  if (file==thread->task->Ptask->file)
  {
    thread->seek_position = MYFTELL(file);
  }
   
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
    if (i != 1)
        panic ("Can't locate actions for task %d\n", taskid);
    if (taskid == da.taskid)
        break;
    if (taskid < da.taskid)
        panic ("Trace file not properly sorted\n");
  }
  return (k);
}

void 
module_name(
  struct t_Ptask *Ptask,
  int   block_id,
  char *block_name,
  char *activity_name,
  int   src_file,
  int   src_line
)
{
  register struct t_module *mod;
    
  mod = 
    (struct t_module*) query_prio_queue (&Ptask->Modules, (t_priority)block_id);
  
  if (mod==M_NIL)
  {
    mod = (struct t_module *)mallocame (sizeof(struct t_module));
    mod->identificator = block_id;
    mod->ratio = 1.0;
    mod->block_name = block_name;
    mod->activity_name = activity_name;
    mod->src_file = src_file;
    mod->src_line = src_line;
    mod->used=0; /* De moment no ha estat utilitzat */
    insert_queue (&Ptask->Modules, (char *)mod, (t_priority) block_id);
  }
  else
  {
    if (mod->block_name!=(char *)0)
    {
      printf ("Warning: redefinition of module %d\n",block_id);
    }
    else
    {
      mod->block_name = block_name;
      mod->activity_name = activity_name;
      mod->src_file = src_file;
      mod->src_line = src_line;
    }
  }
  Vampir_new_application_symbol(activity_name,block_id,block_name);
}

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
locate_first_cpu_burst (
  FILE           *file,
  int             taskid, 
  struct t_Ptask *Ptask)
{
  char            buf[BUFSIZE];
  int             i;
  t_off_fitxer    k;
  int             a, b;
  int comm_id, root_rank, j;
  char *c;
  int block_id, src_file, src_line;
  char block_name[256], activity_name[256];
  char *bn, *an;
  int burst_category_id;
  double burst_category_mean, burst_category_std_dev;

  /*
  k = MYFTELL(file);
  fprintf(stderr,
          "Es comença la cerca a la posicio %llu (Task = %02d)\n",
          k,
          taskid);
  */
  
  while (1)
  {
    i = fscanf (file, "%[^\n]\n", buf);
    
    if (i == -1)
      panic ("Can't locate actions for task %d\n", taskid);

    /* DEBUG 
    fprintf(stderr,"REC: %s\n",buf); */

    i = sscanf (buf, "\"CPU burst\" { %d, %d,", &a, &b);
      
    if ((i == 2) && (taskid == a + 1))
      break;

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
    {
      bn = (char *)mallocame (strlen(block_name)+1);
      strcpy (bn, block_name);
      an = (char *)mallocame (strlen(activity_name)+1);
      strcpy (an, activity_name);
      module_name(Ptask, block_id, bn, an, src_file, src_line);
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
  }
   
  k = MYFTELL(file);
  k = k - (t_off_fitxer)(strlen (buf)+1);
  MYFSEEK (file, k, SEEK_SET);
  return (k);
}

static t_off_fitxer
fast_locate_first_cpu_burst (FILE *file,
                             int taskid,
                             struct t_Ptask *Ptask)
{
  char         buf[BUFSIZE];
  int          i;
  int          a, b;
  t_off_fitxer p_ini, p_inf, p_actual, p_sup;

/* Aixo es la maxima distancia que hi pot haver entre la cota inferior i la 
 superior per tal de comenc,ar a buscar record a record. No pot ser molt gran 
 perque no hi guanyariem res. Pero ha de ser prou gran per tal que hi hagui com 
 a minim un cpuburst perque sino la cerca no acabaria mai. */

#define DISTANCIA_CERCA_PAS_A_PAS (t_off_fitxer)(2*BUFSIZE)

#define SITUA_INICI_SEGUENT_POSICIO(pos) \
  MYFSEEK (file,pos,SEEK_SET); 


/* 
#define DEBUGA_CERCA_PRIMER_CPUBURST 1 */


#ifdef DEBUGA_CERCA_PRIMER_CPUBURST
  fprintf(stderr,"\tBuscant cpu burst de la task %d\n",taskid);
#endif  

  p_ini    = MYFTELL(file); /* Es guarda la posicio inicial */
  p_inf    = p_ini; /* Aquesta sera la cota inferior */
  p_actual = p_ini;

  MYFSEEK (file, 0, SEEK_END);
  p_sup = MYFTELL(file); /* S'obtre l'ultima posicio possible */

  /* Ens tornem a posar al comenc,amnent */
  MYFSEEK (file, p_ini, SEEK_SET);

#ifdef DEBUGA_CERCA_PRIMER_CPUBURST
  fprintf(stderr,"\t\tp_ini %llu, p_sup %llu\n",p_ini, p_sup);
#endif

  while (1)
  {
    /* 
    fprintf(stderr,"Abans de llegir, la posicio es %llu\n",FTELL(file));
    */
    i = fscanf (file, "%[^\n]\n", buf);
    if (i == -1)
      panic ("Can't locate actions for task %d\n", taskid);
    if (i==0)
    {
      SITUA_INICI_SEGUENT_POSICIO(MYFTELL(file)+1);
      p_actual++;
      continue;
    }
    /*
    fprintf(stderr,"Despres de llegir, la posicio es %llu\n",FTELL(file));
    */
    i = sscanf (buf, "\"CPU burst\" { %d, %d,", &a, &b);
    if (i == 2) 
    {
      if (taskid <= a + 1)
      {
        /* Hem trobat un cpu burst <= que la task buscada. Aixo es una cota 
           superior de la posicio buscada. */
        p_sup = p_actual;

        if ((p_sup - p_inf) <= DISTANCIA_CERCA_PAS_A_PAS)
        { 
          /* La distancia es prou petita per buscar pas a pas */
          break;
        }
        else
        {
          /* Cal provar amb una posicio anterior */
          p_actual = p_inf + (p_sup - p_inf)/2;
          SITUA_INICI_SEGUENT_POSICIO(p_actual);
#ifdef DEBUGA_CERCA_PRIMER_CPUBURST
          fprintf(stderr,"\t\t\tRecord: %s\n",buf);
          fprintf(stderr,
                    "\t\tInf: %llu, SUP: %llu Current: %llu\n",
                    p_inf,
                    p_sup,
                    p_actual);
#endif
        }
      }
      else /* taskid > a+1 */
      {
        /* Aixo es una cota inferior */
        p_inf = p_actual;

        if ((p_sup - p_inf) <= DISTANCIA_CERCA_PAS_A_PAS)
        { 
          /* La distancia es prou petita per buscar pas a pas */
          break;
        }
        else
        {
          /* Cal provar amb una posicio posterior */
          p_actual = p_inf + (p_sup - p_inf)/2;
          SITUA_INICI_SEGUENT_POSICIO(p_actual);
#ifdef DEBUGA_CERCA_PRIMER_CPUBURST
          fprintf(stderr,"\t\t\tRecord: %s\n",buf);
          fprintf(stderr,
                  "\t\tINF: %llu, Sup: %llu Current: %llu\n",
                  p_inf,
                  p_sup,
                  p_actual);
#endif
        }
      }
    }
    else
    {
      if (feof(file))
        break;

      /* Avancem pas a pas fins a trobar un cpu_burst */
    }

  }
   
#ifdef DEBUGA_CERCA_PRIMER_CPUBURST
  /*
  fprintf(stderr,"\t\tp_inf %llu, p_sup %llu\n",p_inf, p_sup);
  fprintf(stderr,"\t\t\tUltim Record: %s\n",buf);
  */
  fprintf(stderr, "\n\t\tLast record: %s\n", buf);
  fprintf(stderr, "\t\tSTEP BY STEP SEARCH ON [%llu - %llu]\n\n", p_inf, p_sup);
#endif
  /* Ens situem a la cota inferior i busquem d'un en un */
  MYFSEEK (file, p_inf, SEEK_SET);
  return(locate_first_cpu_burst (file, taskid, Ptask));
}

static t_boolean
is_there_lseek_info (FILE *fp, struct t_Ptask *Ptask)
{
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

void
sddf_load_initial_work_to_threads (struct t_Ptask *Ptask)
{
  struct t_task   *task;
  struct t_thread *thread;
  char            *local_io_buf;
  int              i;
  t_off_fitxer     j;
  char             buf[BUFSIZE];
  t_boolean        have_lseek_info = FALSE;


  if (Ptask->file == NULL)
  {
    Ptask->file = MYFOPEN (Ptask->tracefile, "r");

    if (Ptask->file == NULL)
    {
      panic ("Cant't open trace file %s\n", Ptask->tracefile);
    }
    
    /* Local io_buf for each file */
    local_io_buf = mallocame (BUFSIZE);
    setvbuf (Ptask->file, local_io_buf, _IOFBF, BUFSIZE);
    have_lseek_info = is_there_lseek_info (Ptask->file, Ptask);

    if ((!binary_files))
    {
      while (fgets (buf, BUFSIZE, Ptask->file) != NULL)
      {
        i = strlen (buf);
        if ((i > 3) && (buf[0] == '\"') && 
            (buf[i - 2] == ';') && (buf[i - 3] == ';'))
        {
          break;
        }
        j = MYFTELL (Ptask->file);
      }
      MYFSEEK (Ptask->file, j, SEEK_SET);
    }
  }

  /* To analyze all the block definition */
  /* FEC: Ara aixo s'ha de fer sempre per poder llegir les definicions:
  if (have_lseek_info==TRUE) */
  locate_first_cpu_burst (Ptask->file, 1, Ptask);

  /* Cal agafar la posicio on comenc,a la primera task per tal que els 
   * fast_locate_first_cpu_burst funcionin. */
  j = MYFTELL (Ptask->file);
  

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
        /* printf("   * Locating first record for Task %02d\n", task->taskid); */
        fprintf(stdout,
                "   * Locating first record for Task %02d\n",
                task->taskid);
      }
      thread->file = MYFOPEN (Ptask->tracefile, "r");

      if (thread->file == (FILE *)0)
      {
        thread->file        = Ptask->file;
        thread->file_shared = TRUE;
        fprintf (stderr,
                 "Warning!: T%02d t%02d is sharing trace pointer\n",
                 task->taskid,
                 thread->threadid);
      }

      if (have_lseek_info==FALSE)
      {
        MYFSEEK (thread->file, j, SEEK_SET);
        
        if (binary_files)
          thread->seek_position = locate_first_cpu_burst_binary(thread->file,
                                                                task->taskid);
        else
          thread->seek_position = fast_locate_first_cpu_burst (thread->file,
                                                               task->taskid,
                                                               Ptask);
        j = thread->seek_position;
      }
      else 
        MYFSEEK (thread->file, thread->seek_position, SEEK_SET);
    }
  }

  #ifdef WRITE_TRACE_SEEKS
  if ((have_lseek_info==FALSE) && (binary_files==FALSE))
  {
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
      new_thread_in_Ptask (Ptask, j, j, 1, 0, 0);
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

void
module_new (struct t_Ptask *Ptask, int identificator, double ratio)
{
  struct t_module *mod;

  if (debug&D_TASK) 
    printf ("New module: %d with ratio %f\n",identificator, ratio); 

  mod = (struct t_module *) query_prio_queue (&Ptask->Modules, (t_priority)identificator);
  if (mod==M_NIL)
  {
    mod = (struct t_module *)mallocame (sizeof(struct t_module));
    mod->identificator = identificator;
    mod->ratio = ratio;
    mod->block_name = (char *)0;
    mod->activity_name = (char *)0;
    mod->src_file = -1;
    mod->src_line = -1;
    mod->used=0; /* De moment no ha estat utilitzat */
    insert_queue (&Ptask->Modules, (char *)mod, (t_priority) identificator);
  }
  else
  {
    fprintf (stderr, "Module %d already exists. Old ratio %f, new one %f\n",
              identificator, mod->ratio, ratio);
    mod->ratio = ratio;
  }
}


void
module_entrance (struct t_Ptask *Ptask, int tid, int thid, int identificator)
{
  register struct t_thread *thread;
  register struct t_module *mod;
  
  if (debug&D_TASK)
  {
    PRINT_TIMER (current_time);
    printf (": Going into module: %d for P%d T%d th%d\n",identificator,
            Ptask->Ptaskid, tid, thid);
  }

  thread = locate_thread (Ptask, tid, thid);
  mod = (struct t_module*) query_prio_queue(&Ptask->Modules, 
                                            (t_priority)identificator);
  
  if (mod==M_NIL)
  {
    mod = (struct t_module *)mallocame (sizeof(struct t_module));
    mod->identificator = identificator;
    mod->ratio = 1.0;
    mod->block_name = (char *)0;
    mod->activity_name = (char *)0;
    mod->src_file = -1;
    mod->src_line = -1;
    insert_queue (&Ptask->Modules, (char *)mod, (t_priority) identificator);
  }
  mod->used=1; /* Ara ja ha estat utilitzat! */
  inLIFO_queue (&(thread->modules), (char *)mod);
}

int
module_exit (struct t_Ptask *Ptask, int tid, int thid, int identificator)
{
  register struct t_thread *thread;
  register struct t_module *mod;
  register struct t_thread *awaked;
  register struct t_action *action;
  register t_micro ti, bandw;
  struct t_node *node;
  float fo;
  struct t_machine *machine;

  if (debug&D_TASK)
  {
    PRINT_TIMER (current_time);
    printf (": Going out module: %d for P%d T%d th%d\n",identificator,
            Ptask->Ptaskid, tid, thid);
  }
  
  thread = locate_thread (Ptask, tid, thid);
  
  #ifdef BLOCK_SPECIAL
  if (((identificator==3) ||(identificator==6) || (identificator==7) || 
      (identificator==13)|| (identificator==4)) &&
      (thread->global_op_done==FALSE))
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
      return(1);
    }
  }
  else
  {
    thread->global_op_done = FALSE;
    #endif
    mod = (struct t_module *)outFIFO_queue(&(thread->modules));
    if (mod==M_NIL)
    {
      /*
       * FEC: Converteixo aquest panic en un Warning perque si la trac,a s'ha 
       * obtingut a partir de tallar una trac,a paraver, es possible que comenci 
       * amb alguns block end sense que hi haguin hagut els corresponents block
       * begin.
      panic("Exiting module %d, but no information recorderd P%d T%d th%d\n",
            identificator,IDENTIFIERS(thread));
      */
      fprintf(stderr,
              "WARNING: Exiting module %d, but no information recorderd P%d T%d th%d\n",
              identificator,IDENTIFIERS(thread));
    }
    else if (mod->identificator!=identificator)
    {
      panic("Exiting module %d, but this is not the current one (%d) for P%d T%d th%d\n",
            identificator, mod->identificator, IDENTIFIERS(thread));  
    }
  #ifdef BLOCK_SPECIAL  
  }
  #endif
  return(0);
}

void
recompute_work_upon_modules(struct t_thread *thread, struct t_action *action)
{
  register struct t_module *mod;
   
  if (action->action!=WORK)
    panic("Improper call to recompute_work_upon_modules\n");
   
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
