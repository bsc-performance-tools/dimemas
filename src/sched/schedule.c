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
#include "subr.h"
#include "task.h"

struct t_queue  Machine_queue;
struct t_queue  Dedicated_Connections_queue;
struct t_queue  Node_queue;
int last_node_id_used=0;
struct t_queue  Ptask_queue;

int             SCH_prio = 0;

struct t_cpu*
select_free_cpu(struct t_node *node, struct t_thread *thread)
{
  struct t_cpu   *cpu;

  /* Select a free processor with cache affinity */
  for (cpu = (struct t_cpu *) head_queue (&(node->Cpus));
       cpu != C_NIL;
       cpu = (struct t_cpu *) next_queue (&(node->Cpus)))
  {
    if ((cpu->current_thread == TH_NIL) && 
        (cpu->current_thread_context == thread))
      return (cpu);
  }

  /* Select a free processor */
  for (cpu = (struct t_cpu *) head_queue (&(node->Cpus));
       cpu != C_NIL;
       cpu = (struct t_cpu *) next_queue (&(node->Cpus)))
  {
    if (cpu->current_thread == TH_NIL)
      return (cpu);
  }
  return (C_NIL);
}

static void
put_thread_on_run (struct t_thread *thread, struct t_node *node)
{
  t_micro           ti;
  struct t_account *account;
  struct t_cpu     *cpu;
  dimemas_timer     tmp_timer,new_time;
  struct t_machine *machine;
  struct t_action  *action;

  machine = node->machine;

  Paraver_thread_wait (0,
                       IDENTIFIERS (thread),
                       thread->last_paraver, current_time,
                       PRV_THREAD_SYNC_ST);
  thread->last_paraver = current_time;

  account = current_account (thread);
  SUB_TIMER (current_time, thread->put_into_ready, tmp_timer);
  ADD_TIMER (tmp_timer,
             account->time_ready_without_cpu,
             account->time_ready_without_cpu);

  cpu = select_free_cpu (node, thread);
  if (cpu == C_NIL)
    panic ("Can't get free processor on node %d\n", node->nodeid);

  cpu->current_thread = thread;
  thread->cpu = cpu;

  if (cpu->current_thread_context != thread)
  {
     /* Context switch */
    account->n_th_in_run++;
    cpu->current_thread_context = thread;
    if (machine->scheduler.context_switch != (t_micro) NO_CONTEXT_SWITCH)
    {
      thread->doing_context_switch = TRUE;
      thread->to_be_preempted = FALSE;
      FLOAT_TO_TIMER (machine->scheduler.context_switch, tmp_timer);
      ADD_TIMER (current_time,
                 machine->scheduler.minimum_quantum,
                 thread->min_time_to_be_preempted);
      ADD_TIMER (tmp_timer,
                 thread->min_time_to_be_preempted,
                 thread->min_time_to_be_preempted);
      ADD_TIMER (current_time, tmp_timer, new_time);

      if (debug&D_SCH)
      {
        PRINT_TIMER (current_time);
        printf (
          ": Thread context swicth P%02d T%02d (t%02d) for %.0f on CPU %d node %d\n",
          IDENTIFIERS (thread),
          machine->scheduler.context_switch, cpu->cpuid,
          node->nodeid);
      }
      thread->event = EVENT_timer (new_time, NOT_DAEMON, M_SCH, thread, 0);
      return;
    }
  }
  ADD_TIMER (current_time,
             machine->scheduler.minimum_quantum,
             thread->min_time_to_be_preempted);

  ti = SCHEDULER_get_execution_time (thread);

  FLOAT_TO_TIMER (ti, tmp_timer);
  ADD_TIMER (current_time, tmp_timer, new_time);

  if (debug&D_SCH)
  {
    PRINT_TIMER (current_time);
    printf (": Thread to run P%02d T%02d (t%02d) for %.6f on CPU %d node %d\n",
            IDENTIFIERS (thread),
            ti, cpu->cpuid, node->nodeid);
  }

  thread->event = EVENT_timer (new_time, NOT_DAEMON, M_SCH, thread, 0);
}

void
SCHEDULER_thread_to_ready (struct t_thread *thread)
{
  struct t_node    *node;
  struct t_cpu     *cpu;
  struct t_thread  *thread_current;
  struct t_machine *machine;

  node = get_node_of_thread (thread);
  machine = node->machine;

  if (more_actions (thread) == FALSE)
    panic ("Trying to run P%02d T%02d (t%02d) without actions\n",
           IDENTIFIERS (thread));

  /* FEC: Faig aixo per no haver de posar necessariament un CPU_BURST a zero 
   * entre cada accio de la trac,a. */
  if (thread->action->action != WORK)
  {
    /* Estic suposant que en aquest cas es prove d'un lloc on s'ha cridat a 
     * SCHEDULER_thread_to_ready just despres d'acabar una altra accio sense 
     * mirar-ne el tipus. Per tant, en lloc de donar una error afegeixo una 
     * accio de WORK de durada 0, mitjanc,ant la rutina 
     * SCHEDULER_thread_to_ready_return.
     * Segurament en aquest cas thread->lose_cpu estara a FALSE. */
    if (debug&D_SCH)
    {
      PRINT_TIMER (current_time);
      printf (": Thread P%02d T%02d (t%02d) to ready and action isn't WORK! Rescheduling...\n",
              IDENTIFIERS (thread));
    }
     
    SCHEDULER_thread_to_ready_return(M_SCH, thread, (t_micro) 0, 0);
    return;
  }
  
  if (debug&D_SCH)
  {
    PRINT_TIMER (current_time);
    printf (": Threads on ready queue of node %d\n", node->nodeid);
    for (cpu = (struct t_cpu *) head_queue (&(node->Cpus));
         cpu != C_NIL;
         cpu = (struct t_cpu *) next_queue (&(node->Cpus)))
    {
      if (cpu->current_thread != TH_NIL)
      {
        thread_current = cpu->current_thread;
        if (thread_current->doing_busy_wait)
        {
          printf ("\t\t  (W) P%02d T%02d (t%02d) node %d cpu %d\n",
                  IDENTIFIERS (thread_current),
                  node->nodeid, cpu->cpuid);
        }
        else
        {
          printf ("\t\t  (R) P%02d T%02d (t%02d) node %d cpu %d\n",
                  IDENTIFIERS (thread_current),
                  node->nodeid, cpu->cpuid);
        }
      }
    }

    for (thread_current = (struct t_thread *) head_queue (&(node->ready));
         thread_current != TH_NIL;
         thread_current = (struct t_thread *) next_queue (&(node->ready)))
    {
      printf ("\t\t  (O) P%02d T%02d (t%02d)\n", IDENTIFIERS (thread_current));
    }
    printf ("\t\t  (N) P%02d T%02d (t%02d)\n", IDENTIFIERS (thread));
  }

  if (thread->action->action==FS)
  {
    FS_general (thread->action->desc.fs_op.fs_o.fs_user_event.id, thread); 
  }
  else
  {
    thread->put_into_ready = current_time;
    (*SCH[machine->scheduler.policy].thread_to_ready) (thread);
  }
  reload_done = TRUE;
}

t_micro
SCHEDULER_get_execution_time (struct t_thread *thread)
{
  t_micro           ti;
  struct t_account *account;
  dimemas_timer     tmp_timer;
  struct t_action  *action;
  struct t_cpu       *cpu;
  struct t_node    *node;
  struct t_machine *machine;
   
  node = get_node_of_thread (thread);
  machine = node->machine;
   
  action = thread->action;
  if (action->action != WORK)
    panic ("Trying to work when innaproppiate P%02d T%02d (t%02d)\n",
           IDENTIFIERS (thread));
  
  ti = (*SCH[machine->scheduler.policy].get_execution_time) (thread);
  
#ifndef IDLE_ACCOUNTING
  /* In idle blocks, time isn't added to account */
  if (thread->idle_block == FALSE)
  {
#endif
    account = current_account (thread);
    FLOAT_TO_TIMER (ti, tmp_timer);
    ADD_TIMER (account->cpu_time, tmp_timer, account->cpu_time);
#ifndef IDLE_ACCOUNTING
  }
#endif
  return (ti);
}

void
SCHEDULER_next_thread_to_run (struct t_node *node)
{
  struct t_thread *thread;
  struct t_cpu   *cpu;
  struct t_machine *machine;
   
  machine = node->machine;

  if (debug&D_SCH)
  {
    PRINT_TIMER (current_time);
    printf (": Threads on ready queue of node %d to RUN\n", node->nodeid);
    for (
      cpu = (struct t_cpu *) head_queue (&(node->Cpus));
      cpu != C_NIL;
      cpu = (struct t_cpu *) next_queue (&(node->Cpus))
    )
    {
      if (cpu->current_thread != TH_NIL)
      {
        thread = cpu->current_thread;
        printf (
          "\t\t   Running P%02d T%02d (t%02d) node %d cpu %d\n",
          IDENTIFIERS (thread),
          node->nodeid, cpu->cpuid
        );
      }
    }
    for (thread = (struct t_thread *) head_queue (&(node->ready));
         thread != TH_NIL;
         thread = (struct t_thread *) next_queue (&(node->ready)))
    {
      printf ("\t\t     (O) P%02d T%02d (t%02d)\n", IDENTIFIERS (thread));
    }
  }
  
  thread = (*SCH[machine->scheduler.policy].next_thread_to_run) (node);
   
  /* FEC: Per evitar haver de tenir CPU_BURSTS a zero afegeixo aquest if
   * tot i que crec que no es possible que s'arribi aqui i no hi
   * hagi cap thread a ready (per exemple despres d'un message_receive
   * es crida a SCHEDULER_thread_to_ready i a SCHEDULER_general
   * consecutivament) perque en aquest cas s'hauria d'haver afegit
   * una accio de WORK amb durada 0.
   */
  if (thread == TH_NIL)
  {
    panic("Null thread scheduled to run!\n");
  }
  /*Fi FEC: canvi per evitar CPU_BURST a zero *************************/
   
  if (debug&D_SCH)
  {
    PRINT_TIMER (current_time);
    printf (": Selected thread P%02d T%02d (t%02d)\n", IDENTIFIERS (thread));
  }
  
  put_thread_on_run (thread, node);
}

/******************************************************************
 ** Aquesta funcio serveix per fer algun tractament extra en alguns
 ** events concrets. Per exemple, si l'event es un block d'entrada/
 ** sortida, cal contar el temps que el thread passa dins d'aquest
 ** block a l'accounting.
 ******************************************************************/
void TractaEvent( struct t_thread *thread, struct t_action *action)
{
  struct t_account *account;
  dimemas_timer tmp_timer;
  #define IDLE_BLOCK     0
  #define IO_READ_BLOCK  129
  #define IO_WRITE_BLOCK 130
  #define IO_CALL_BLOCK  131

  if (action->desc.even.type == BLOCK_BEGIN)
  {
    if (action->desc.even.value == IO_READ_BLOCK)
    {
      /* Comença una lectura de disc */
      account = current_account(thread);
      ASS_ALL_TIMER ((account->initial_io_read_time), current_time);
    }
    else if (action->desc.even.value==IO_WRITE_BLOCK)
    {
      /* Comença una escriptura de disc */
      account = current_account(thread);
      ASS_ALL_TIMER ((account->initial_io_write_time), current_time);
    }
    else if (action->desc.even.value == IO_CALL_BLOCK)
    {
      /* Comença una lectura/escriptura de disc */
      account = current_account(thread);
      ASS_ALL_TIMER ((account->initial_io_call_time), current_time);
    }
    else if (action->desc.even.value == IDLE_BLOCK)
    {
      thread->idle_block = TRUE;
    }
  }
  else if (action->desc.even.type==BLOCK_END)
  {
    if (action->desc.even.value==IO_READ_BLOCK)
    {
      /* Acaba una lectura de disc */
      account = current_account(thread);
      FLOAT_TO_TIMER(0,tmp_timer);
      if (!EQ_TIMER((account->initial_io_read_time),tmp_timer))
      {
        SUB_TIMER(current_time, (account->initial_io_read_time), tmp_timer);
        ADD_TIMER(tmp_timer, (account->io_read_time),(account->io_read_time));
        FLOAT_TO_TIMER(0,(account->initial_io_read_time));
      }
    }
    else if (action->desc.even.value==IO_WRITE_BLOCK)
    {
      /* Acaba una escriptura de disc */
      account = current_account(thread);
      FLOAT_TO_TIMER(0,tmp_timer);
      if (!EQ_TIMER((account->initial_io_write_time),tmp_timer))
      {
        SUB_TIMER(current_time, (account->initial_io_write_time), tmp_timer);
        ADD_TIMER(tmp_timer, (account->io_write_time),(account->io_write_time));
        FLOAT_TO_TIMER(0,(account->initial_io_write_time));
      }
    }
    else if (action->desc.even.value==IO_CALL_BLOCK)
    {
      /* Acaba una lectura/escriptura de disc */
      account = current_account(thread);
      FLOAT_TO_TIMER(0,tmp_timer);
      if (!EQ_TIMER((account->initial_io_call_time),tmp_timer))
      {
        SUB_TIMER(current_time, (account->initial_io_call_time), tmp_timer);
        ADD_TIMER(tmp_timer, (account->io_call_time),(account->io_call_time));
        FLOAT_TO_TIMER(0,(account->initial_io_call_time));
      }
    }
    else if (action->desc.even.value == IDLE_BLOCK)
    {
      thread->idle_block = FALSE;
    }
  }
}

void
SCHEDULER_general (int value, struct t_thread *thread)
{
  register struct t_action *action;
  register struct t_cpu    *cpu;
  register struct t_node   *node;
  register t_micro          ti;
  dimemas_timer             tmp_timer;
  struct t_machine         *machine;

  cpu = get_cpu_of_thread (thread);
  node = get_node_of_thread (thread);
  machine = node->machine;
  if (debug&D_SCH)
  {
    PRINT_TIMER (current_time);
    printf (": SCHEDULER general %d for P%02d, T%02d, (t%02d) on CPU %d node %d\n",
            value,
            IDENTIFIERS (thread),
            cpu->cpuid,
            node->nodeid);
  }

  switch (value)
  {
    case SCH_TIMER_OUT:
    {
      if (thread->doing_startup)
      {
        Paraver_thread_startup (cpu->unique_number,
                                IDENTIFIERS (thread),
                                thread->last_paraver,
                                current_time);
        new_cp_node (thread, CP_OVERHEAD);
        thread->startup_done  = TRUE;
        thread->doing_startup = FALSE;
      }
      else if (thread->doing_copy)
      { /* At this point thread has done its data copy to internal library */
        Paraver_thread_data_copy(cpu->unique_number,
                                IDENTIFIERS (thread),
                                thread->last_paraver,
                                current_time);
        new_cp_node(thread, CP_OVERHEAD);
        thread->copy_done  = TRUE;
        thread->doing_copy = FALSE;
      }
      else if (thread->doing_roundtrip)
      { /* At this point thread has done send round trip */
        Paraver_thread_round_trip(cpu->unique_number,
                                  IDENTIFIERS (thread),
                                  thread->last_paraver,
                                  current_time);
        new_cp_node(thread, CP_OVERHEAD);
        thread->roundtrip_done  = TRUE;
        thread->doing_roundtrip = FALSE;
      }
      else
      {
        if (thread->doing_context_switch)
        {
          Paraver_thread_consw (cpu->unique_number,
                                IDENTIFIERS (thread),
                                thread->last_paraver,
                                current_time);
        }
        else
        {
          if (thread->doing_busy_wait)
          {
            Paraver_thread_buwa (cpu->unique_number,
                                 IDENTIFIERS (thread),
                                 thread->last_paraver, current_time);
          }
          else
          {
            action = thread->action;
            
            if (thread->idle_block == TRUE)
            {
              Paraver_thread_idle ( cpu->unique_number,
                                    IDENTIFIERS (thread),
                                    thread->last_paraver,
                                    current_time);
            }
            else
            {
              Paraver_thread_running (cpu->unique_number,
                                      IDENTIFIERS (thread),
                                      thread->last_paraver,
                                      current_time);
            }

            new_cp_node (thread, CP_WORK);
          }
        }
      }
      thread->last_paraver = current_time;
      cpu->current_thread  = TH_NIL;

      if (thread->doing_context_switch)
      {
        thread->doing_context_switch = FALSE;
        if (thread->to_be_preempted)
        {
          thread->loose_cpu = FALSE;
          SCHEDULER_thread_to_ready (thread);
          SCHEDULER_next_thread_to_run (node);
        }
        else
        {
          cpu->current_thread = thread;
          ti = SCHEDULER_get_execution_time (thread);

          ADD_TIMER (machine->scheduler.minimum_quantum,
                     current_time,
                     thread->min_time_to_be_preempted);

          FLOAT_TO_TIMER (ti, tmp_timer);
          ADD_TIMER (current_time, tmp_timer, tmp_timer);
          if (debug&D_SCH)
          {
            PRINT_TIMER (current_time);
            printf (
              ": Thread run context P%02d T%02d (t%02d) for %.0f on CPU %d node %d\n",
              IDENTIFIERS (thread),
              ti,
              cpu->cpuid,
              node->nodeid);
          }
          thread->event = EVENT_timer (tmp_timer, NOT_DAEMON, M_SCH, thread, 0);
        }
        break;
      }

      if (thread->doing_busy_wait)
      {
        thread->doing_busy_wait = FALSE;
        COMMUNIC_block_after_busy_wait (thread);
        if (count_queue (&(node->ready)) != 0)
          SCHEDULER_next_thread_to_run (node);
        break;
      }

next_op:
      if (more_actions (thread))
      {
        action = thread->action;
        switch (action->action)
        {
          case SEND:
            COMMUNIC_send (thread);
            break;
          case RECV:
            COMMUNIC_recv (thread);
            break;
          case IRECV:
            COMMUNIC_Irecv (thread);
            break;
          case WAIT:
            COMMUNIC_wait (thread);
            break;
          case WORK:
            thread->loose_cpu = TRUE;
            SCHEDULER_thread_to_ready (thread);
            break;
          case EVEN:
          {
            if (debug&D_SCH)
            {
              PRINT_TIMER (current_time);
              printf (": SCHEDULER general P%02d T%02d (t%02d) User Event %d (%d)\n",
                      IDENTIFIERS (thread),
              action->desc.even.type, action->desc.even.value);
            }

            if (monitorize_event)
            {
              if (event_to_monitorize==action->desc.even.type)
              {
                SUB_TIMER (current_time,
                           thread->last_time_event_number,
                           tmp_timer);
                fprintf (File_for_Event,
                         "Event monitorized  P%02d T%02d (th%02d) distance ",
                         IDENTIFIERS(thread));
                FPRINT_TIMER(File_for_Event, tmp_timer);
                fprintf (File_for_Event,"\n");
                ASS_ALL_TIMER (thread->last_time_event_number, current_time);
              }
            }
            
            if (action->desc.even.type == BLOCK_BEGIN)
            {
              module_entrance (thread->task->Ptask, 
                               thread->task->taskid, 
                               thread->threadid, 
                               action->desc.even.value);
              cpu = get_cpu_of_thread (thread);
              
              if (action->desc.even.value != IDLE_BLOCK)
              {
                Paraver_translate_event (cpu->unique_number,
                                         IDENTIFIERS (thread),
                                         current_time,
                                         action->desc.even.type,
                                         action->desc.even.value);
              }
            }

            /* Depenent de l'event que sigui potser cal un tractament especial */
            TractaEvent(thread, action);

            if (action->desc.even.type == BLOCK_END)
            {
              if (module_exit (thread->task->Ptask, 
                               thread->task->taskid, 
                               thread->threadid, 
                               action->desc.even.value))
                break;
              cpu = get_cpu_of_thread(thread);
              if (action->desc.even.value != IDLE_BLOCK)
              {
                Paraver_translate_event (cpu->unique_number,
                                         IDENTIFIERS (thread),
                                         current_time,
                                         BLOCK_END,
                                         action->desc.even.value);
              }
              /*
              Paraver_event (cpu->unique_number,
                             IDENTIFIERS (thread),
                             current_time,
                             BLOCK_BEGIN,
                             0);
              */
            }
            if ((action->desc.even.type!=BLOCK_BEGIN) && 
                (action->desc.even.type!=BLOCK_END))
            {
              cpu = get_cpu_of_thread(thread);
              
              /* DEBUG
              printf("Printing event: P%02d T%02d (t%02d) Type: %lld Val: %lld\n",
                     IDENTIFIERS(thread),
                     action->desc.even.type,
                     action->desc.even.value);
              */
              
              Paraver_event (cpu->unique_number,
                             IDENTIFIERS (thread),
                             current_time,
                             action->desc.even.type,
                             action->desc.even.value);
            }
            thread->action = action->next;
            freeame ((char *) action, sizeof (struct t_action));
            if (more_actions (thread))
            {
              action = thread->action;
              if (action->action != WORK)
                goto next_op;

              thread->loose_cpu = FALSE;
              SCHEDULER_thread_to_ready (thread);
            }
            break;
          }
          case FS:
            thread->loose_cpu = FALSE;
            FS_general (action->desc.fs_op.fs_o.fs_user_event.id, thread);
            break;
          case SEM:
            switch (action->desc.sem_op.op)
            {
              case SEM_WAIT:
                thread->action = action->next;
                SEMAPHORE_wait (action->desc.sem_op.sem_id, thread);
                freeame ((char *) action, sizeof (struct t_action));
                break;
              case SEM_SIGNAL:
                SEMAPHORE_signal (action->desc.sem_op.sem_id, thread);
                thread->action = action->next;
                freeame ((char *) action, sizeof (struct t_action));
                if (more_actions (thread))
                {
                   thread->loose_cpu = FALSE;
                   SCHEDULER_thread_to_ready (thread);
                }
                break;
              case SEM_SIGNAL_N:
                SEMAPHORE_signal_n (action->desc.sem_op.sem_id,
                        action->desc.sem_op.n,
                        thread);
                thread->action = action->next;
                freeame ((char *) action, sizeof (struct t_action));
                if (more_actions (thread))
                {
                   thread->loose_cpu = FALSE;
                   SCHEDULER_thread_to_ready (thread);
                }
                break;
              default:
                panic ("Invalid semaphore action\n");
            }
            break;
          case PORT_SEND:
            PORT_send (action->desc.port.module,
                       action->desc.port.portid,
                       thread,
                       action->desc.port.size);
            thread->action = action->next;
            freeame ((char *) action, sizeof (struct t_action));
            break;
          case PORT_RECV:
            PORT_receive (action->desc.port.module,
                           action->desc.port.portid,
                           thread, action->desc.port.size);
            thread->action = action->next;
            freeame ((char *) action, sizeof (struct t_action));
            break;
          case MEMORY_COPY:
            MEMORY_copy_segment (action->desc.memory.module,
                                 thread,
                                 action->desc.memory.source,
                                 action->desc.memory.dest,
                                 action->desc.memory.size);
            thread->action = action->next;
            freeame ((char *) action, sizeof (struct t_action));
            break;
          case GLOBAL_OP:
            GLOBAL_operation (thread,
                              action->desc.global_op.glop_id,
                              action->desc.global_op.comm_id,
                              action->desc.global_op.root_rank,
                              action->desc.global_op.root_thid,
                              action->desc.global_op.bytes_send,
                              action->desc.global_op.bytes_recvd);
            break;
          case  MPI_IO:
            FS_general (FS_OPERATION, thread);
            break;
          case  MPI_OS:
            ONE_SIDED_operation (thread);
            break;
          default:
            panic ("Unkown action %d to P%02d T%02d (t%02d)\n",
                   action->action,
                   IDENTIFIERS (thread));
        }
      }

      if ((count_queue (&(node->ready)) != 0) &&
          (num_free_cpu (node) > 0))
      {
        SCHEDULER_next_thread_to_run (node);
      }
      break;
    }
    case SCH_NEW_JOB:
    {
      if (num_free_cpu (node) > 0)
      {
        /* The new one is the unique one */
        SCHEDULER_next_thread_to_run (node);
      }
      break;
    }
    default:
      panic ("Invalid interface value %d in scheduler routines\n", value);
  }
}

void SCHEDULER_thread_to_busy_wait(struct t_thread *thread)
{
  struct t_cpu     *cpu;
  dimemas_timer     tmp_timer, new_time;
  struct t_node    *node;
  struct t_machine *machine;
  
  node = get_node_of_thread (thread);
  machine = node->machine;

  if (debug&D_SCH)
  {
    PRINT_TIMER (current_time);
    printf (": Thread to busy wait P%02d T%02d (t%02d) for %f\n",
            IDENTIFIERS (thread),
            machine->scheduler.context_switch);
  }

  cpu = get_cpu_of_thread (thread);
  cpu->current_thread = thread;
  thread->doing_busy_wait = TRUE;
  thread->min_time_to_be_preempted = current_time;
  FLOAT_TO_TIMER (machine->scheduler.context_switch, tmp_timer);
  ADD_TIMER (current_time, tmp_timer, new_time);
  thread->event = EVENT_timer (new_time, NOT_DAEMON, M_SCH, thread, 0);
}

void
SCHEDULER_init(char *filename)
{
  struct t_node    *node;
  struct t_Ptask   *Ptask;
  struct t_task    *task;
  struct t_thread  *thread;
  struct t_action  *action;
  int               j;
  FILE             *fi;
  char              buf[BUFSIZE];
  struct t_machine *machine;

  if (debug&D_SCH)
  {
    PRINT_TIMER (current_time);
    printf (": SCHEDULER initial routine called with file %s\n",filename);
  }

  if (filename != (char *) 0)
  {
    if (strcmp(filename,"")!=0)
    {
      fi = MYFOPEN (filename, "r");
      if (fi == (FILE *) 0)
      {
        panic ("Can't open scheduler configuration file %s\n", filename);
      }
      j = fscanf (fi, "Policy: %s\n", buf);

      if (j != 1)
        panic ("Invalid format in file %s. Invalid policy name %s\n",
               filename,
               buf);
      
      /* FEC pendent: Quan es canvii el format del fitxer de scheduling caldra 
       * canviar aixo. De moment s'aplica el mateix a totes les maquines. */
      j = SCHEDULER_get_policy (buf);
      for (machine=(struct t_machine *)head_queue(&Machine_queue);
           machine!=MA_NIL;
           machine=(struct t_machine *)next_queue(&Machine_queue))
      {
        machine->scheduler.policy = j;
      }
      fclose (fi);
    }
  }

  /* Prepare cpu ready queues */

  for (Ptask  = (struct t_Ptask *) head_queue (&Ptask_queue);
       Ptask != P_NIL;
       Ptask  = (struct t_Ptask *) next_queue (&Ptask_queue))
  {
    for (task  = (struct t_task *) head_queue (&(Ptask->tasks));
         task != T_NIL;
         task  = (struct t_task *) next_queue (&(Ptask->tasks)))
    {
      node = get_node_of_task (task);
      machine = node->machine;
      SDDF_start (node->nodeid, task->taskid, current_time);

      for (thread  = (struct t_thread *) head_queue (&(task->threads));
           thread != TH_NIL;
           thread  = (struct t_thread *) next_queue (&(task->threads)))
      {
        action = thread->action;
        if (action == AC_NIL)
        {
            panic ("P%02d T%02d (t%02d) without actions\n", IDENTIFIERS (thread));
        }

        if (action->action != WORK)
          panic ("P%02d T%02d (t%02d) must begin execution with work\n",
                 IDENTIFIERS (thread));

        thread->loose_cpu = TRUE;
        (*SCH[machine->scheduler.policy].init_scheduler_parameters) (thread);
        SCHEDULER_thread_to_ready (thread);
      }
    }
  }

  /* Prepare the the first Scheduler Events */
  for (node  = (struct t_node *) head_queue (&Node_queue);
       node != N_NIL;
       node  = (struct t_node *) next_queue (&Node_queue))
  {
    j = MIN (count_queue (&(node->ready)), count_queue (&(node->Cpus)));
    if (j > 0)
    {
      for (; j; j--)
      {
        SCHEDULER_next_thread_to_run (node);
      }
    }
  } 
}

void
SCHEDULER_end()
{
  struct t_cpu   *cpu;
  struct t_node  *node;
  
  if (debug&D_SCH)
  {
    PRINT_TIMER (current_time);
    printf (": SCHEDULER end routine called\n");
  }

  for (node  = (struct t_node *) head_queue (&Node_queue);
       node != N_NIL;
       node  = (struct t_node *) next_queue (&Node_queue))
  {
    if (count_queue (&(node->ready)) != 0)
    {
      if (debug&D_SCH)
      {
        PRINT_TIMER (current_time);
        printf (": Warning, %d threads on ready queue for node %d\n",
                count_queue (&(node->ready)),
                node->nodeid);
      }
    }

    for (cpu  = (struct t_cpu *) head_queue (&(node->Cpus));
         cpu != C_NIL;
         cpu = (struct t_cpu *) next_queue (&(node->Cpus)))
    {
      if (cpu->current_thread != TH_NIL)
      {
        if (debug&D_SCH)
        {
          PRINT_TIMER (current_time);
          printf (": Warning, cpu %d node %d is running P%02d T%02d (t%02d)\n",
                  cpu->cpuid,
                  node->nodeid,
                  IDENTIFIERS (cpu->current_thread));
        }
      }
    }
  }
}

void
SCHEDULER_reload(struct t_Ptask *Ptask)
{
  struct t_task  *task;
  struct t_thread *thread;
  struct t_node    *node;
  struct t_machine *machine;
  
  for (task  = (struct t_task *) head_queue (&(Ptask->tasks));
       task != T_NIL;
       task  = (struct t_task *) next_queue (&(Ptask->tasks)))
  {
    /* Una task tindra tots els threads al mateix node i, per tant, a la
       mateixa maquina */
    thread=  (struct t_thread *) head_queue (&(task->threads));
    if (thread != TH_NIL)
    {
      node = get_node_of_thread (thread);
      machine = node->machine;
    }

    for (thread = (struct t_thread *) head_queue (&(task->threads));
         thread != TH_NIL;
         thread = (struct t_thread *) next_queue (&(task->threads)))
    {
      if (thread->action == AC_NIL)
      {
        panic ("P%02d T%02d (t%02d) without initial actions\n",
               IDENTIFIERS (thread));
      }
      thread->loose_cpu = TRUE;
      SCHEDULER_thread_to_ready (thread);
      (*SCH[machine->scheduler.policy].clear_parameters) (thread);
    }
  }
}

void
SCHEDULER_copy_parameters (struct t_thread *th_o, struct t_thread *th_d)
{
  struct t_node    *node;
  struct t_machine *machine;
  
  node = get_node_of_thread (th_o);
  machine = node->machine;
  (*SCH[machine->scheduler.policy].scheduler_copy_parameters) (th_o, th_d);
}

void
SCHEDULER_free_parameters (struct t_thread *thread)
{
  struct t_node    *node;
  struct t_machine *machine;
  
  node = get_node_of_thread (thread);
  machine = node->machine;
  (*SCH[machine->scheduler.policy].scheduler_free_parameters) (thread);
}

int
SCHEDULER_info (int              value,
                int              info,
                struct t_thread *th_s,
                struct t_thread *th_r)
{
  int               return_value = 0;
  struct t_node    *node;
  struct t_machine *machine;
  
  node    = get_node_of_thread (th_s);
  machine = node->machine;
  switch (value)
  {
    case CONTEXT_SWITCH_TIMER_OUT:
      (*SCH[machine->scheduler.policy].info) (info);
      break;
    case COMMUNICATION_INFO:
      return_value = (*SCH[machine->scheduler.policy].info) (info, th_s, th_r);
      break;
    default:
      panic ("Unkown info %d to scheduler value\n", value);
  }
  return (return_value);
}

void
SCHEDULER_thread_to_ready_return(int              module,
                                 struct t_thread *thread, 
                                 t_micro          ti,
                                 int              id)
{
  struct t_action *action, *cur_action;

  cur_action = thread->action;
  switch (module)
  {
    case M_FS:
      action = (struct t_action *) mallocame (sizeof (struct t_action));

      action->next                             = cur_action;
      action->action                           = FS;
      action->desc.fs_op.which_fsop            = FS_USER_EVENT;
      action->desc.fs_op.fs_o.fs_user_event.id = id;
      cur_action                               = action;
      break;
    case M_COM:
      break;
    case M_PORT:
      break;
    case M_MEM:
      break;
    case M_RMA:
      break;
    case M_SCH: /* Ho afegeixo per eliminar CPUBURST a zero */
      break;
    default:
      panic ("Invalid module identifier %d\n", module);
  }

  action = (struct t_action *) mallocame (sizeof (struct t_action));

  action->next                  = cur_action;
  action->action                = WORK;
  action->desc.compute.cpu_time = ti;
  thread->action                = action;

  SCHEDULER_thread_to_ready (thread);
}

struct t_thread *
SCHEDULER_preemption (struct t_thread *thread, struct t_cpu   *cpu)
{
  struct t_node    *node;
  struct t_thread  *thread_current;
  struct t_account *account_current, *account;
  struct t_action  *action;
  dimemas_timer     when;
  t_micro           ti;
  
  thread_current = cpu->current_thread;
  account_current = current_account (thread_current);
  account = current_account (thread);
  node = get_node_of_thread (thread);


  if (debug&D_SCH)
  {
    PRINT_TIMER (current_time);
    printf (": Preemption from P%02d T%02d (t%02d) to P%02d T%02d (t%02d) ",
            IDENTIFIERS (thread),
            IDENTIFIERS (thread_current));
  }

  if (thread_current->doing_busy_wait)
  {
    if (debug&D_SCH)
    {
      printf ("doing busy wait\n");
    }
    EVENT_extract_timer (M_SCH, thread_current, &when);
    Paraver_thread_buwa(cpu->unique_number,
                        IDENTIFIERS (thread_current),
                        thread_current->last_paraver,
                        current_time);
    thread_current->last_paraver = current_time;
    SUB_TIMER (when, current_time, when);
    TIMER_TO_FLOAT (when, ti);
    thread_current->doing_busy_wait = FALSE;
    COMMUNIC_block_after_busy_wait (thread_current);
    thread_current = TH_NIL;
  }
  else
  {
    if (GT_TIMER (thread_current->min_time_to_be_preempted, current_time))
    {
      if (GT_TIMER (thread_current->next_event_timer,
                    thread_current->min_time_to_be_preempted))
      {
        EVENT_extract_timer (M_SCH, thread_current, &when);
        SUB_TIMER (when, current_time, when);
        TIMER_TO_FLOAT (when, ti);
        SUB_TIMER (account_current->cpu_time, when, account_current->cpu_time);
        if ((thread_current->action != AC_NIL) &&
            (thread_current->action->action == WORK))
        {
          thread_current->action->desc.compute.cpu_time += ti;
        }
        else
        {
          action = (struct t_action *) mallocame (sizeof (struct t_action));
          action->next = thread_current->action;
          action->action = WORK;
          action->desc.compute.cpu_time = ti;
          thread_current->action = action;
        }
        thread->event = EVENT_timer (thread_current->min_time_to_be_preempted,
                                     NOT_DAEMON,
                                     M_SCH,
                                     thread_current,
                                     0);
      }

      /*
      if (thread_current->doing_context_switch)
      {
        thread_current->to_be_preempted = TRUE;
        account_current->n_preempt_to_me ++; account->n_preempt_to_other ++;
        if (debug&D_SCH)
        {
          printf ("doing context switch (preemption delayed)\n");
        }
        
        return (thread);
      }
      */

      if (debug&D_SCH)
        printf (" min quantum requiered (delayed)\n");
      return (thread);
    }
  }

  cpu->current_thread = TH_NIL;

  if (debug&D_SCH)
  {
    printf ("normal case\n");
  }

  if (thread_current != TH_NIL)
  {
    EVENT_extract_timer (M_SCH, thread_current, &when);

    action         = (struct t_action *) mallocame (sizeof (struct t_action));
    action->next   = thread_current->action;
    action->action = WORK;

    SUB_TIMER (when, current_time, when);
    TIMER_TO_FLOAT (when, ti);
    action->desc.compute.cpu_time  = ti;
    thread_current->action         = action;
    thread_current->put_into_ready = current_time;

    SUB_TIMER (account_current->cpu_time, when, account_current->cpu_time);

    Paraver_thread_running (cpu->unique_number,
                            IDENTIFIERS (thread_current),
                            thread_current->last_paraver,
                            current_time);

    thread_current->last_paraver = current_time;
  }

  if (thread->action->action != WORK)
    panic ("Next action for P%02d T%02d (t%02d) must be work\n",
           IDENTIFIERS (thread));

  put_thread_on_run (thread, node);

  account_current->n_preempt_to_me++;
  account->n_preempt_to_other++;
  return (thread_current);
}

int
SCHEDULER_get_policy (char *s)
{
  int i = 0;

  while (SCH[i].name != 0)
  {
    if (strcmp (s, SCH[i].name) == 0)
    {
      if (debug&D_SCH)
      {
        PRINT_TIMER (current_time);
        printf (": Scheduling policy selected %s\n", s);
      }
      return (i);
    }
    i++;
  }
  panic ("Invalid scheduling policy name %s\n", s);
  return (-1);
}

t_boolean
more_actions (struct t_thread *thread)
{
  struct t_action *action;
  struct t_account *account;
  struct t_Ptask *Ptask;
  struct t_cpu   *cpu;
  struct t_node  *node;

#ifdef PALLAS
  if ((load_interactive) && (thread->action == AC_NIL))
  {
    /* Using the wrapper ti mask whether we read old or new trace format */ 
    get_next_action_wrapper(thread);
  }
#else /* !PALLAS */
  if (thread->task->Ptask->synthetic_application)
  {
    more_actions_to_sintetic (thread);
  }
  else
  {
    if ((load_interactive) && (thread->action == AC_NIL))
    {
      /* Using the wrapper ti mask whether we read old or new trace format */ 
      get_next_action_wrapper(thread);
    }
  }
#endif /* PALLAS */

  action = thread->action;
  account = current_account (thread);
  Ptask = thread->task->Ptask;
  
  if (action == AC_NIL)
  {
    if (debug&D_SCH)
    {
      PRINT_TIMER (current_time);
      printf (": SCHEDULER no more actions P%02d T%02d (t%02d)\n",
              IDENTIFIERS (thread));
    }

    account->final_time = current_time;
    if ((reload_Ptasks) && (more_actions_on_Ptask (Ptask) == FALSE))
    {
      reload_new_Ptask (Ptask);
    }
    else
    {
      cpu = get_cpu_of_thread (thread);
      node = get_node_of_thread (thread);
      SDDF_stop (node->nodeid, thread->task->taskid, current_time);
      Paraver_thread_dead (0, IDENTIFIERS (thread));
    }
    return (FALSE);
  }

  if (action->action == DEAD)
  {
    if (debug&D_SCH)
    {
      PRINT_TIMER (current_time);
      printf (": SCHEDULER no more actions P%02d T%02d (t%02d)\n",
              IDENTIFIERS (thread));
    }

    account->final_time = current_time;
    if ((reload_Ptasks) && (more_actions_on_Ptask (Ptask) == FALSE))
    {
      reload_new_Ptask (Ptask);
    }
    else
    {
      cpu = get_cpu_of_thread (thread);
      node = get_node_of_thread (thread);
      SDDF_stop (node->nodeid, thread->task->taskid, current_time);
      Paraver_thread_dead (0, IDENTIFIERS (thread));
    }
    return (FALSE);
  }
  return (TRUE);
}
