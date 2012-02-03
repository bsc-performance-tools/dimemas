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

#include "sched_vars.h"
#include "communic.h"
#include "cpu.h"
#include "events.h"
#include "extern.h"
#include "fs.h"
#include "links.h"
#include "list.h"
#ifdef USE_EQUEUE
#include "listE.h"
#endif
#include "mallocame.h"
#include "memory.h"
#include "paraver.h"
#include "random.h"
#include "schedule.h"
#include "subr.h"
#include "task.h"

#include "machine.h"
#include "node.h"


static void os_post (struct t_thread *thread,
                     struct t_mpi_os *mpi_os, struct t_window *win);

static void
mem_to_next_module (struct t_thread *thread)
{
  switch (thread->to_module)
  {
  case M_FS:
    FS_general (FS_COPY_SEGMENT_END, thread);
    break;
  default:
    panic ("Invalid module identifier %d in MEMORY\n", thread->to_module);
  }
}

void
MEMORY_general (int value, struct t_thread *thread)
{
  switch (value)
  {
  case MEMORY_TIMER_OUT:
    if (thread->copy_segment_link_source != L_NUL)
    {

      if (debug)
      {
        PRINT_TIMER (current_time);
        printf (": MEMORY general P%02d T%02d (t%02d) end copy from %02d to %02d\n",
                IDENTIFIERS (thread),
                thread->copy_segment_link_source->info.node->nodeid,
                thread->copy_segment_link_dest->info.node->nodeid);
      }
      free_link (thread->copy_segment_link_source, thread);
      free_link (thread->copy_segment_link_dest, thread);
    }
    else
    {
      if (debug)
      {
        PRINT_TIMER (current_time);
        printf (": MEMORY general P%02d T%02d (t%02d) end copy local\n",
                IDENTIFIERS (thread) );
      }
    }
    mem_to_next_module (thread);
    break;
  default:
    panic ("Invalid interface value %d in MEMORY_general", value);
  }
}

void MEMORY_init()
{
  struct t_node  *node;

  printf ("-> Loding initial memory status\n");

#ifdef USE_EQUEUE
  for (node  = (struct t_node*) head_Equeue (&Node_queue);
       node != N_NIL;
       node  = (struct t_node*) next_Equeue (&Node_queue) )
#else
  for (node  = (struct t_node*) head_queue (&Node_queue);
       node != N_NIL;
       node  = (struct t_node*) next_queue (&Node_queue) )
#endif
  {
    create_queue (& (node->wait_out_copy_segment) );
    create_queue (& (node->wait_in_copy_segment) );
  }
}

void MEMORY_end()
{
  /*
  if (debug)
  {
    PRINT_TIMER (current_time);
    printf (": MEMORY final routine called\n");
  }
  */
}

void
MEMORY_copy_segment (int              module,
                     struct t_thread *thread,
                     struct t_node   *node_s,
                     struct t_node   *node_d,
                     int si)
{
  struct t_action *action;
  float           startup;

  startup = (node_s == node_d ? node_s->local_memory_startup :
             node_s->remote_memory_startup);

  if ( (startup != (t_nano) 0) && (thread->startup_done == FALSE) )
  {
    if (debug)
    {
      PRINT_TIMER (current_time);
      printf (": MEMORY startup for P%d T%d t%d\n", IDENTIFIERS (thread) );
    }
    action = (struct t_action *) MALLOC_get_memory (sizeof (struct t_action) );
    action->next = thread->action;
    action->action = MEMORY_COPY;
    action->desc.memory.size = si;
    action->desc.memory.source = node_s;
    action->desc.memory.dest = node_d;
    action->desc.memory.module = module;
    thread->action = action;
    /* thread->startup_done = TRUE; */
    thread->doing_startup = TRUE;
    SCHEDULER_thread_to_ready_return (M_MEM, thread, startup, 0);
    return;
  }

  if (debug)
  {
    PRINT_TIMER (current_time);
    printf (": MEMORY copy segment P%d T%d t%d from %d to %d size %d\n",
            IDENTIFIERS (thread),
            node_s->nodeid, node_d->nodeid, si);
  }

  thread->startup_done = FALSE;

  thread->copy_segment_link_source = L_NIL;
  thread->copy_segment_link_dest = L_NIL;
  thread->copy_segment_size = si;
  thread->to_module = module;

  thread->loose_cpu = TRUE;
  really_copy_segment (thread, node_s, node_d, si);
}

void
really_copy_segment (struct t_thread *thread, struct t_node *node_s,
                     struct t_node *node_d, int si)
{
  struct t_copyseg *copyseg;
  t_nano         ti;
  t_boolean       remote_comm;
  dimemas_timer   tmp_timer;

  if (get_links_memory_copy (thread, node_s, node_d) )
  {
    if (debug)
    {
      PRINT_TIMER (current_time);
      printf (": Memory real copy P%d T%d t%d start copy from %d to %d\n",
              IDENTIFIERS (thread),
              node_s->nodeid, node_d->nodeid);
    }
    remote_comm = (node_s == node_d ? FALSE : TRUE);
    /* ti = transferencia (si, remote_comm, thread, NULL, NULL); */
    transferencia (si, remote_comm, thread, NULL, &ti, NULL);
    FLOAT_TO_TIMER (ti, tmp_timer);
    ADD_TIMER (current_time, tmp_timer, tmp_timer);
    thread->event = EVENT_timer (tmp_timer, NOT_DAEMON, M_MEM, thread, 0);
  }
  else
  {
    if (debug)
    {
      PRINT_TIMER (current_time);
      printf (": MEMORY real copy P%d T%d t%d wait for links on %d or %d\n",
              IDENTIFIERS (thread),
              node_s->nodeid, node_d->nodeid);
    }
    copyseg = (struct t_copyseg *) MALLOC_get_memory (sizeof (struct t_copyseg) );

    copyseg->thread = thread;
    copyseg->node_s = node_s;
    copyseg->node_d = node_d;

    if (thread->copy_segment_link_source == L_NIL)
    {
      inFIFO_queue (& (node_s->wait_out_copy_segment), (char *) copyseg);
    }
    else
    {
      inFIFO_queue (& (node_d->wait_in_copy_segment), (char *) copyseg);
    }
  }
}

static int
from_rank_to_task (struct t_window *win, int rank)
{
  int *task;
  int  i;

  task = (int *) head_queue (&win->global_ranks);
  i = 0;
  while (i != rank)
  {
    task = (int *) next_queue (&win->global_ranks);
    if (task == (int *) 0)
      panic ("Unable to locate  rank %d in window %d\n",
             rank, win->window_id);
    i++;
  }
  return (*task);
}

void
really_RMA (register struct t_thread *thread)
{
  register struct t_action *action;
  register struct t_window *win;
  register struct t_mpi_os *mpi_os;
  struct t_node *node_s, *node_d;
  t_boolean remote_comm;
  struct t_bus_utilization *bus_utilization;
  t_nano ti;
  dimemas_timer tmp_timer;
  struct t_machine  *machine;

  action = thread->action;
  mpi_os = &action->desc.mpi_os;

  win = (struct t_window *) query_prio_queue (& (thread->task->Ptask->Window),
        (t_priority) mpi_os->window_id);
  if (win == (struct t_window *) 0)
  {
    panic ("Unable to locate window with identificator %d\n",
           mpi_os->window_id);
  }

  node_s = get_node_of_thread (thread);
  node_d = get_node_for_task_by_name (thread->task->Ptask,
                                      from_rank_to_task (win, mpi_os->target_rank) );
  /* Se suposa que els dos nodes son de la mateixa maquina. Per tant, es
     igual quin agafem. */
  machine = node_s->machine;

  if (get_links (thread, node_s, node_d) )
  {
    remote_comm = (node_s == node_d ? FALSE : TRUE);
    if (remote_comm)
    {
      if (machine->communication.num_messages_on_network)
      {
        if (machine->communication.policy == COMMUNIC_FIFO)
        {
          if (machine->network.curr_on_network >= machine->communication.num_messages_on_network)
          {
            if (debug & D_COMM)
            {
              PRINT_TIMER (current_time);
              printf (": Really RMA for P%d T%d th%d block due bus contention\n",
                      IDENTIFIERS (thread) );
            }
            inFIFO_queue (&machine->network.queue, (char *) thread);
            /*             PARAVER_Event (1, 1, 1, 1, current_time,
                                70, count_queue (&network.queue)); */
            return;
          }
          if (debug & D_COMM)
          {
            PRINT_TIMER (current_time);
            printf (": Really RMA for P%d T%d th%d obtain bus\n",
                    IDENTIFIERS (thread) );
          }
        }

        /* Annotate this thread is using the bus */
        machine->network.curr_on_network++;
        bus_utilization = (struct t_bus_utilization *) MALLOC_get_memory (sizeof (struct t_bus_utilization) );
        bus_utilization->sender = thread;
        ASS_ALL_TIMER (bus_utilization->initial_time, current_time);
        inFIFO_queue (&machine->network.threads_on_network, (char *) bus_utilization);
      }
    }

    /* ready for sending */
    if (debug & D_COMM)
    {
      PRINT_TIMER (current_time);
      printf (": Really RMA for P%d T%d th%d start RMA operation size %d\n",
              IDENTIFIERS (thread), mpi_os->size);
    }
    /* ti = transferencia (mpi_os->size, remote_comm, thread, NULL, NULL); */
    transferencia (mpi_os->size, remote_comm, thread, NULL, &ti, NULL);

    FLOAT_TO_TIMER (ti, tmp_timer);
    ADD_TIMER (current_time, tmp_timer, tmp_timer);
    thread->event = EVENT_timer (tmp_timer, NOT_DAEMON, M_COM, thread, RMA_TIMER_OUT);
  }

}

static void
os_fence (register struct t_thread *thread, register struct t_mpi_os *mpi_os,
          register struct t_window *win, t_boolean end_operation)
{
  register struct t_thread *others;
  register struct t_node *node;
  struct t_cpu *cpu;
  register struct t_action *action;

  node = get_node_of_thread (thread);
  cpu = get_cpu_of_thread (thread);
  if (end_operation == FALSE)
    PARAVER_Event (cpu->unique_number, IDENTIFIERS (thread), current_time,
                   PARAVER_OS, PARAVER_OS_FENCE_START);

  if ( (end_operation) ||
       ( ( (count_queue (&win->fence_tasks) + 1) == count_queue (&win->global_ranks) )
         && (count_queue (&win->fence_operations) == 0) ) )
  {
    PARAVER_Event (cpu->unique_number, IDENTIFIERS (thread), current_time,
                   PARAVER_OS, PARAVER_OS_FENCE_END);
    action = thread->action;
    thread->action = action->next;
    MALLOC_free_memory ( (char *) action, sizeof (struct t_action) );
    if (more_actions (thread) )
    {
      thread->loose_cpu = FALSE;
      SCHEDULER_thread_to_ready (thread);
    }
    if (end_operation)
      extract_from_queue (&win->fence_tasks, (char *) thread);

    for (others = (struct t_thread *) outFIFO_queue (&win->fence_tasks);
         others != (struct t_thread *) 0;
         others = (struct t_thread *) outFIFO_queue (&win->fence_tasks) )
    {
      node = get_node_of_thread (others);
      cpu = get_cpu_of_thread (others);
      PARAVER_Event (cpu->unique_number, IDENTIFIERS (others), current_time,
                     PARAVER_OS, PARAVER_OS_FENCE_END);
      PARAVER_OS_Blocked (0, IDENTIFIERS (others),
                          others->last_paraver, current_time);
      others->last_paraver = current_time;
      action = others->action;
      others->action = action->next;
      MALLOC_free_memory ( (char *) action, sizeof (struct t_action) );
      if (more_actions (others) )
      {
        thread->loose_cpu = FALSE;
        SCHEDULER_thread_to_ready (others);
      }
    }
    win->mode = WINDOW_MODE_NONE;
  }
  else
  {
    thread->last_paraver = current_time;
    inFIFO_queue (&win->fence_tasks, (char *) thread);
  }
}

static void
os_getput (register struct t_thread *thread, register struct t_mpi_os *mpi_os,
           register struct t_window *win)
{
  struct t_node *node_s, *node_d;
  t_nano        startup = 0;
  struct t_account *account;
  struct t_action *action;
  struct t_thread *copy_thread;
  dimemas_timer tmp_timer;
  struct t_cpu *cpu;

  /* Compute latency */
  if (thread->startup_done == FALSE)
  {
    /* register this as a current operation thread */
    if (win->mode ==  WINDOW_MODE_FENCE)
      inFIFO_queue (&win->fence_operations, (char *) thread);
    if (win->mode ==  WINDOW_MODE_LOCK)
      inFIFO_queue (&win->lock_operations, (char *) thread);
    if (win->mode ==  WINDOW_MODE_POST)
      inFIFO_queue (&win->post_operations, (char *) thread);

    node_s = get_node_of_thread (thread);
    node_d = get_node_for_task_by_name (thread->task->Ptask,
                                        from_rank_to_task (win, mpi_os->target_rank) );

    cpu = get_cpu_of_thread (thread);
    PARAVER_Event (cpu->unique_number,
                   IDENTIFIERS (thread),
                   current_time,
                   PARAVER_OS,
                   PARAVER_OS_START);

    if (node_s == node_d)
    {
      startup  = node_s->local_startup;
      startup += RANDOM_GenerateRandom (&randomness.memory_latency);
    }
    else
    {
      startup  = node_s->remote_startup;
      startup += RANDOM_GenerateRandom (&randomness.network_latency);
    }

    if (startup != 0)
    {
      if (debug & D_COMM)
      {
        PRINT_TIMER (current_time);
        printf (": RMA startup for P%d T%d t%d\n", IDENTIFIERS (thread) );
      }
      thread->loose_cpu     = FALSE;
      thread->doing_startup = TRUE;

      account = current_account (thread);
      FLOAT_TO_TIMER (startup, tmp_timer);
      ADD_TIMER (account->latency_time, tmp_timer, account->latency_time);
      SUB_TIMER (account->cpu_time, tmp_timer, account->cpu_time);

      SCHEDULER_thread_to_ready_return (M_RMA, thread, startup, 0);
      return;
    }
  }

  node_s = get_node_of_thread (thread);
  cpu = get_cpu_of_thread (thread);
  PARAVER_Event (cpu->unique_number,
                 IDENTIFIERS (thread),
                 current_time,
                 PARAVER_OS,
                 PARAVER_OS_LATENCY);

  thread->startup_done = FALSE;
  if (debug & D_COMM)
  {
    PRINT_TIMER (current_time);
    printf (": RMA for P%d T%d t%d to T%d %d bytes\n",
            IDENTIFIERS (thread),
            mpi_os->target_rank,
            mpi_os->size);
  }
  copy_thread = duplicate_thread (thread);
  really_RMA (copy_thread);

  action = thread->action;
  thread->action = action->next;
  MALLOC_free_memory ( (char *) action, sizeof (struct t_action) );
  if (more_actions (thread) )
  {
    thread->loose_cpu = FALSE;
    SCHEDULER_thread_to_ready (thread);
  }
}

static struct t_task *
from_taskid_to_task (struct t_Ptask *Ptask, int taskid)
{
  size_t         tasks_it;
  struct t_task *task;

  /* JGG (2012/01/16): new way to navigate through tasks
  for (task=(struct t_task *)head_queue(&Ptask->tasks);
       task!=(struct t_task *)0;
       task=(struct t_task *)next_queue(&Ptask->tasks))
  */
  for (tasks_it = 0; tasks_it < Ptask->tasks_count; tasks_it++)
  {
    task = & (Ptask->tasks[tasks_it]);

    if (task->taskid == taskid)
      return (task);
  }
  panic ("Unable to locate task %d in P%d\n", taskid, Ptask->Ptaskid);

  return NULL;
}

static void
os_lock (struct t_thread *thread, struct t_mpi_os *mpi_os,  struct t_window *win)
{
  int taskid;
  struct t_task *task_d;
  struct t_node *node_s;
  struct t_action *action;
  struct t_cpu *cpu;

  taskid = from_rank_to_task (win, mpi_os->target_rank);
  task_d = from_taskid_to_task (thread->task->Ptask, taskid);
  if ( (win->task_with_lock == (struct t_task *) 0) ||
       ( (win->task_with_lock == task_d) && (mpi_os->mode == MPI_OS_LOCK_SHARED) && (win->lock_mode == MPI_OS_LOCK_SHARED) ) )
  {
    /* No lock ongoing or shared lock on the same rank */
    /* Add a new thread to the queue with handling locks */
    win->task_with_lock = task_d;
    win->lock_mode = mpi_os->mode;
    inFIFO_queue (&win->threads_with_lock, (char *) thread);

    node_s = get_node_of_thread (thread);
    cpu = get_cpu_of_thread (thread);
    PARAVER_Event (cpu->unique_number, IDENTIFIERS (thread), current_time,
                   PARAVER_OS, PARAVER_OS_GET_LOCK);

    action = thread->action;
    thread->action = action->next;
    MALLOC_free_memory ( (char *) action, sizeof (struct t_action) );
    if (more_actions (thread) )
    {
      thread->loose_cpu = FALSE;
      SCHEDULER_thread_to_ready (thread);
    }

  }
  else
  {
    /* Block thread until lock is released */
    node_s = get_node_of_thread (thread);
    cpu = get_cpu_of_thread (thread);
    PARAVER_Event (cpu->unique_number, IDENTIFIERS (thread), current_time,
                   PARAVER_OS, PARAVER_OS_WAIT_LOCK);

    thread->last_paraver = current_time;
    inFIFO_queue (&win->threads_waiting_lock, (char *) thread);
  }
}

static void
os_unlock (struct t_thread *thread, struct t_mpi_os *mpi_os,  struct t_window *win)
{
  struct t_node *node_s;
  struct t_action *action;
  struct t_thread *th;
  struct t_queue tmp_queue;
  struct t_cpu *cpu;

  node_s = get_node_of_thread (thread);
  cpu = get_cpu_of_thread (thread);
  PARAVER_Event (cpu->unique_number, IDENTIFIERS (thread), current_time,
                 PARAVER_OS, PARAVER_OS_UNLOCK_BEGIN);

  /* Check if there are pending operations */
  for (th = (struct t_thread *) head_queue (&win->lock_operations);
       th != TH_NIL;
       th = (struct t_thread *) next_queue (&win->lock_operations) )
  {
    if (th->task == thread->task)
      break;
  }

  if (th == TH_NIL)
  {
    /* There are no operations pending */
    extract_from_queue (&win->threads_with_lock, (char *) thread);
    node_s = get_node_of_thread (thread);
    cpu = get_cpu_of_thread (thread);
    PARAVER_Event (cpu->unique_number, IDENTIFIERS (thread), current_time,
                   PARAVER_OS, PARAVER_OS_UNLOCK_END);

    action = thread->action;
    thread->action = action->next;
    MALLOC_free_memory ( (char *) action, sizeof (struct t_action) );
    if (more_actions (thread) )
    {
      thread->loose_cpu = FALSE;
      SCHEDULER_thread_to_ready (thread);
    }

    if (count_queue (&win->threads_with_lock) == 0)
    {
      win->task_with_lock = (struct t_task *) 0;
      if (count_queue (&win->threads_waiting_lock) != 0)
      {
        /* Window is now free, start pending lock */
        create_queue (&tmp_queue);
        for (th = (struct t_thread *) outFIFO_queue (&win->threads_waiting_lock);
             th != TH_NIL;
             th = (struct t_thread *) outFIFO_queue (&win->threads_waiting_lock) )
        {
          node_s = get_node_of_thread (th);
          PARAVER_OS_Blocked (0, IDENTIFIERS (th),
                              th->last_paraver, current_time);
          th->last_paraver = current_time;

          inFIFO_queue (&tmp_queue, (char *) th);
        }
        for (th = (struct t_thread *) outFIFO_queue (&tmp_queue);
             th != TH_NIL;
             th = (struct t_thread *) outFIFO_queue (&tmp_queue) )
        {
          action = th->action;
          mpi_os = &action->desc.mpi_os;

          win = (struct t_window *) query_prio_queue (& (th->task->Ptask->Window),
                (t_priority) mpi_os->window_id);

          os_lock (th, mpi_os, win);
        }
      }
      else
        win->mode = WINDOW_MODE_NONE;
    }
    else
      win->mode = WINDOW_MODE_NONE;
  }
  else
  {
    inFIFO_queue (&win->threads_waiting_unlock, (char *) thread);
  }
}
static t_boolean
there_is_an_open_post (struct t_thread *thread, struct t_window *win)
{
  register struct t_thread *th;

  for (th = (struct t_thread *) head_queue (&win->post_done);
       th != (struct t_thread *) 0;
       th = (struct t_thread *) next_queue (&win->post_done) )
  {
    if (th->task == thread->task)
      return (TRUE);
  }
  return (FALSE);
}

static t_boolean
there_is_an_open_start (struct t_thread *thread, struct t_window *win)
{
  register struct t_thread *th;

  for (th = (struct t_thread *) head_queue (&win->start_done);
       th != (struct t_thread *) 0;
       th = (struct t_thread *) next_queue (&win->start_done) )
  {
    if (th->task == thread->task)
      return (TRUE);
  }
  return (FALSE);
}

static t_boolean
there_is_a_post_for_this_start (struct t_thread *thread, struct t_mpi_os *mpi_os,
                                struct t_window * win)
{
  register struct t_thread *th;
  struct t_action *action;
  struct t_mpi_os *mpi_os_2;
  int *current_rank;
  int *post_rank;
  int taskid;

  for (current_rank = (int *) head_queue (&mpi_os->post_ranks);
       current_rank != (int *) 0;
       current_rank = (int *) next_queue (&mpi_os->post_ranks) )
  {
    taskid = from_rank_to_task (win, *current_rank);
    for (th = (struct t_thread *) head_queue (&win->post_done);
         th != (struct t_thread *) 0;
         th = (struct t_thread *) next_queue (&win->post_done) )
    {
      if (th->task->taskid == taskid)
        break;
    }
    if (th == TH_NIL)
      return (FALSE); /* This task did not perform a post already */
    action = th->action;
    mpi_os_2 = &action->desc.mpi_os;
    for (post_rank = (int *) head_queue (&mpi_os_2->post_ranks);
         post_rank != (int *) 0;
         post_rank = (int *) next_queue (&mpi_os_2->post_ranks) )
    {
      if (thread->task->taskid == from_rank_to_task (win, *post_rank) )
        break;
    }
    if (post_rank == (int *) 0)
    {
      panic ("RMA, start with no corresponing post\n");
      return (FALSE);
    }
  }
  return (TRUE);
}

static void
are_there_pending_start (struct t_thread *thread, struct t_mpi_os *mpi_os,
                         struct t_window *win)
{
  struct t_thread *th, *th2;

  for (th = (struct t_thread *) head_queue (&win->pending_of_post);
       th != TH_NIL;
       th = (struct t_thread *) next_queue (&win->pending_of_post) )
  {
    os_post (th, &th->action->desc.mpi_os, win);
  }

  /* If no block any more, remove it from the block queue */
  for (th = (struct t_thread *) head_queue (&win->start_done);
       th != TH_NIL;
       th = (struct t_thread *) next_queue (&win->start_done) )
  {
    for (th2 = (struct t_thread *) head_queue (&win->pending_of_post);
         th2 != TH_NIL;
         th2 = (struct t_thread *) next_queue (&win->pending_of_post) )
    {
      if (th2 == th->twin_thread)
      {
        extract_from_queue (&win->pending_of_post, (char *) th2);
        break;
      }
    }
  }
}

static t_boolean
there_are_rma_operations (struct t_thread *thread, struct t_window *win)
{
  struct t_thread *th;

  for (th = (struct t_thread *) head_queue (&win->post_operations);
       th != TH_NIL;
       th = (struct t_thread *) next_queue (&win->post_operations) )
  {
    if (th->task == thread->task)
      return (TRUE);
  }
  return (FALSE);
}

static void
are_there_pending_waits (struct t_thread *thread, struct t_window *win)
{
  struct t_queue tmp_q;
  struct t_action *action;
  struct t_mpi_os *mpi_os;
  struct t_thread *th;

  create_queue (&tmp_q);
  for (th = (struct t_thread *) outFIFO_queue (&win->pending_wait_to_complete);
       th != TH_NIL;
       th = (struct t_thread *) outFIFO_queue (&win->pending_wait_to_complete) )
  {
    inFIFO_queue (&tmp_q, (char *) th);
  }
  for (th = (struct t_thread *) outFIFO_queue (&tmp_q);
       th != TH_NIL;
       th = (struct t_thread *) outFIFO_queue (&tmp_q) )
  {
    action = th->action;
    mpi_os = &action->desc.mpi_os;

    os_post (th, mpi_os, win);
  }
}

static t_boolean
there_are_all_completes_for_this_wait (struct t_thread *thread, struct t_window *win)
{
  struct t_thread *th, *th2;
  struct t_action *action;
  struct t_mpi_os *mpi_os;
  int *post_rank;
  int taskid, located;

  for (th = (struct t_thread *) head_queue (&win->post_done);
       th != TH_NIL;
       th = (struct t_thread *) next_queue (&win->post_done) )
  {
    if (th->task == thread->task)
      break;
  }
  if (th == TH_NIL)
    panic ("Unable to locate post for a given wait P%d T%d th%d\n", IDENTIFIERS (thread) );

  action = th->action;
  mpi_os = &action->desc.mpi_os;
  located = count_queue (&mpi_os->post_ranks);
  for (post_rank = (int *) head_queue (&mpi_os->post_ranks);
       post_rank != (int *) 0;
       post_rank = (int *) next_queue (&mpi_os->post_ranks) )
  {
    taskid = from_rank_to_task (win, *post_rank);
    for (th2 = (struct t_thread *) head_queue (&win->complete_done);
         th2 != TH_NIL;
         th2 = (struct t_thread *) next_queue (&win->complete_done) )
    {
      if (th2->task->taskid == taskid)
        located--;
    }
  }
  return (located == 0 ? TRUE : FALSE);

}

static void
os_post (struct t_thread *thread, struct t_mpi_os *mpi_os, struct t_window *win)
{
  struct t_node *node_s;
  struct t_action *action;
  struct t_thread *copy_thread;
  struct t_cpu *cpu;

  switch (mpi_os->Oop)
  {
  case MPI_OS_POST_POST:
    node_s = get_node_of_thread (thread);
    cpu = get_cpu_of_thread (thread);
    PARAVER_Event (cpu->unique_number, IDENTIFIERS (thread), current_time,
                   PARAVER_OS, PARAVER_OS_POST_POST);

    if (there_is_an_open_post (thread, win) )
    {
      panic ("Invalid RMA post operation. Recursive post not allowed\n");
      exit (1);
    }
    copy_thread = duplicate_thread (thread);
    inFIFO_queue (&win->post_done, (char *) copy_thread);

    are_there_pending_start (thread, mpi_os, win);

    action = thread->action;
    thread->action = action->next;
    MALLOC_free_memory ( (char *) action, sizeof (struct t_action) );
    if (more_actions (thread) )
    {
      thread->loose_cpu = FALSE;
      SCHEDULER_thread_to_ready (thread);
    }

    break;
  case MPI_OS_POST_START:

    if (there_is_an_open_start (thread, win) )
    {
      panic ("Invalid RMA start operation. Recursive start not allowed\n");
      exit (1);
    }

    if (there_is_a_post_for_this_start (thread, mpi_os, win) )
    {
      copy_thread = duplicate_thread (thread);
      inFIFO_queue (&win->start_done, (char *) copy_thread);
      node_s = get_node_of_thread (thread);
      cpu = get_cpu_of_thread (thread);
      PARAVER_Event (cpu->unique_number, IDENTIFIERS (thread), current_time,
                     PARAVER_OS, PARAVER_OS_POST_START);
      PARAVER_OS_Blocked (0, IDENTIFIERS (thread),
                          thread->last_paraver, current_time);
      thread->last_paraver = current_time;

      action = thread->action;
      thread->action = action->next;
      MALLOC_free_memory ( (char *) action, sizeof (struct t_action) );
      if (more_actions (thread) )
      {
        thread->loose_cpu = FALSE;
        SCHEDULER_thread_to_ready (thread);
      }
    }
    else
    {
      node_s = get_node_of_thread (thread);
      cpu = get_cpu_of_thread (thread);
      PARAVER_Event (cpu->unique_number, IDENTIFIERS (thread), current_time,
                     PARAVER_OS, PARAVER_OS_POST_START_BLOCK);
      thread->last_paraver = current_time;
      /* block until post */
      inFIFO_queue (&win->pending_of_post, (char *) thread);
    }

    break;
  case MPI_OS_POST_COMPLETE:
    if (there_are_rma_operations (thread, win) )
    {
      node_s = get_node_of_thread (thread);
      cpu = get_cpu_of_thread (thread);
      PARAVER_Event (cpu->unique_number, IDENTIFIERS (thread), current_time,
                     PARAVER_OS, PARAVER_OS_POST_COMPLETE_BLOCK);
      thread->last_paraver = current_time;
      inFIFO_queue (&win->pending_rma_to_complete, (char *) thread);
    }
    else
    {
      node_s = get_node_of_thread (thread);
      cpu = get_cpu_of_thread (thread);
      PARAVER_Event (cpu->unique_number, IDENTIFIERS (thread), current_time,
                     PARAVER_OS, PARAVER_OS_POST_COMPLETE);
      PARAVER_OS_Blocked (0, IDENTIFIERS (thread),
                          thread->last_paraver, current_time);
      thread->last_paraver = current_time;

      /* remove twin thread from the start_done queue*/
      for (copy_thread = (struct t_thread *) head_queue (&win->start_done);
           copy_thread != TH_NIL;
           copy_thread = (struct t_thread *) next_queue (&win->start_done) )
      {
        if (copy_thread->twin_thread == thread)
          break;
      }
      if (copy_thread == TH_NIL)
        panic ("Unable to locate a thread when complete in start list\n");
      extract_from_queue (&win->start_done, (char *) copy_thread);
      inFIFO_queue (&win->complete_done, (char *) copy_thread);

      /* check if wait call is done */
      are_there_pending_waits (thread, win);

      action = thread->action;
      thread->action = action->next;
      MALLOC_free_memory ( (char *) action, sizeof (struct t_action) );
      if (more_actions (thread) )
      {
        thread->loose_cpu = FALSE;
        SCHEDULER_thread_to_ready (thread);
      }
    }
    break;
  case MPI_OS_POST_WAIT:
    if (there_are_all_completes_for_this_wait (thread, win) == FALSE)
    {
      node_s = get_node_of_thread (thread);
      cpu = get_cpu_of_thread (thread);
      PARAVER_Event (cpu->unique_number, IDENTIFIERS (thread), current_time,
                     PARAVER_OS, PARAVER_OS_POST_WAIT_BLOCK);
      thread->last_paraver = current_time;
      inFIFO_queue (&win->pending_wait_to_complete, (char *) thread);
    }
    else
    {
      node_s = get_node_of_thread (thread);
      cpu = get_cpu_of_thread (thread);
      PARAVER_Event (cpu->unique_number, IDENTIFIERS (thread), current_time,
                     PARAVER_OS, PARAVER_OS_POST_WAIT);
      PARAVER_OS_Blocked (0, IDENTIFIERS (thread),
                          thread->last_paraver, current_time);
      thread->last_paraver = current_time;

      for (copy_thread = (struct t_thread *) head_queue (&win->post_done);
           copy_thread != TH_NIL;
           copy_thread = (struct t_thread *) next_queue (&win->post_done) )
      {
        if (copy_thread->task == thread->task)
          break;
      }
      if (copy_thread == TH_NIL)
        panic ("Unable to locate a thread when complete in start list\n");
      extract_from_queue (&win->post_done, (char *) copy_thread);

      if (count_queue (&win->post_done) == 0)
      {
        for (copy_thread = (struct t_thread *) outFIFO_queue (&win->complete_done);
             copy_thread != TH_NIL;
             copy_thread = (struct t_thread *) outFIFO_queue (&win->complete_done) )
        {
          MALLOC_free_memory ( (char *) copy_thread, sizeof (struct t_thread) );
        }
        win->mode = WINDOW_MODE_NONE;
      }
      action = thread->action;
      thread->action = action->next;
      MALLOC_free_memory ( (char *) action, sizeof (struct t_action) );
      if (more_actions (thread) )
      {
        thread->loose_cpu = FALSE;
        SCHEDULER_thread_to_ready (thread);
      }

    }
    break;
  default:
    panic ("Invalid operation for RMA post mode %d\n", mpi_os->Oop);
  }
}

void
ONE_SIDED_operation (struct t_thread *thread)
{
  register struct t_action *action;
  register struct t_window *win;
  register struct t_mpi_os *mpi_os;

  action = thread->action;
  mpi_os = &action->desc.mpi_os;

  win = (struct t_window *) query_prio_queue (& (thread->task->Ptask->Window),
        (t_priority) mpi_os->window_id);
  if (win == (struct t_window *) 0)
  {
    panic ("Unable to locate window with identificator %d\n",
           mpi_os->window_id);
  }

  if (win->mode == WINDOW_MODE_NONE)
  {
    if ( (mpi_os->which_os == MPI_OS_GETPUT) || (mpi_os->which_os == MPI_OS_FENCE) )
      win->mode = WINDOW_MODE_FENCE;
    if (mpi_os->which_os == MPI_OS_LOCK)
      win->mode = WINDOW_MODE_LOCK;
    if (mpi_os->which_os == MPI_OS_POST)
      win->mode = WINDOW_MODE_POST;

    if (win->mode == WINDOW_MODE_NONE)
    {
      panic ("Unable to change to window mode based on operation %s",
             mpi_os->which_os);
    }
  }


  switch (win->mode)
  {
  case WINDOW_MODE_FENCE:
    switch (mpi_os->which_os)
    {
    case MPI_OS_GETPUT:
      os_getput (thread, mpi_os, win);
      break;
    case MPI_OS_FENCE:
      os_fence (thread, mpi_os, win, FALSE);
      break;
    default:
      panic ("Invalid window operation %d in this window mode %d\n",
             mpi_os->which_os,  win->mode);
    }
    break;
  case WINDOW_MODE_LOCK:
    switch (mpi_os->which_os)
    {
    case MPI_OS_GETPUT:
      os_getput (thread, mpi_os, win);
      break;
    case MPI_OS_LOCK:
      if (mpi_os->Oop == MPI_OS_LOCK_LOCK)
        os_lock (thread, mpi_os,  win);

      if (mpi_os->Oop == MPI_OS_LOCK_UNLOCK)
        os_unlock (thread, mpi_os,  win);
      break;
    default:
      panic ("Invalid operation in RMA lock mode\n");
    }
    break;
  case WINDOW_MODE_POST:
    switch (mpi_os->which_os)
    {
    case MPI_OS_GETPUT:
      /* Check if RMA is OK with Start and Post operations not perfomed */
      os_getput (thread, mpi_os, win);
      break;
    case MPI_OS_POST:
      os_post (thread, mpi_os, win);
      break;
    default:
      panic ("Invalid operation in POST mode\n");
    }
    break;
  }
}


void
os_completed (struct t_thread *thread)
{
  struct t_node *node_s;
  register struct t_action *action;
  register struct t_window *win;
  register struct t_mpi_os *mpi_os;
  struct t_thread *th;
  struct t_cpu *cpu;

  action = thread->action;
  mpi_os = &action->desc.mpi_os;

  win = (struct t_window *) query_prio_queue (& (thread->task->Ptask->Window),
        (t_priority) mpi_os->window_id);
  if (win == (struct t_window *) 0)
  {
    panic ("Unable to locate window with identificator %d\n",
           mpi_os->window_id);
  }

  node_s = get_node_of_thread (thread);
  cpu = get_cpu_of_thread (thread);
  PARAVER_Event (cpu->unique_number, IDENTIFIERS (thread), current_time,
                 PARAVER_OS, PARAVER_OS_END);

  /* This operation is finished */
  if (win->mode ==  WINDOW_MODE_FENCE)
  {
    extract_from_queue (&win->fence_operations, (char *) thread->twin_thread);
    if ( ( (count_queue (&win->fence_tasks) ) == count_queue (&win->global_ranks) ) &&
         (count_queue (&win->fence_operations) == 0) )
      os_fence (thread->twin_thread, mpi_os, win, TRUE);
  }

  if (win->mode ==  WINDOW_MODE_LOCK)
  {
    extract_from_queue (&win->lock_operations, (char *) thread->twin_thread);
    for (th = (struct t_thread *) head_queue (&win->threads_waiting_unlock);
         th != TH_NIL;
         th = (struct t_thread *) next_queue (&win->threads_waiting_unlock) )
    {
      if (th->task == thread->task)
        break;
    }
    if (th != TH_NIL)
    {
      /* Unlock pending */
      action = thread->action;
      mpi_os = &action->desc.mpi_os;
      win = (struct t_window *) query_prio_queue (& (thread->task->Ptask->Window),
            (t_priority) mpi_os->window_id);
      os_unlock (th, mpi_os, win);
    }
  }
  if (win->mode ==  WINDOW_MODE_POST)
  {
    extract_from_queue (&win->post_operations, (char *) thread->twin_thread);

    for (th = (struct t_thread *) head_queue (&win->pending_rma_to_complete);
         th != TH_NIL;
         th = (struct t_thread *) next_queue (&win->pending_rma_to_complete) )
    {
      if (thread->task == th->task)
      {
        extract_from_queue (&win->pending_rma_to_complete, (char *) th);
        action = th->action;
        mpi_os = &action->desc.mpi_os;
        win = (struct t_window *) query_prio_queue (& (thread->task->Ptask->Window),
              (t_priority) mpi_os->window_id);
        os_post (th, mpi_os, win);
        break;
      }
    }
  }
  delete_duplicate_thread (thread);
}
