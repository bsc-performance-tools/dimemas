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

#include "EventEncoding.h"
#include "communic.h"
#include "configuration.h"
#include "cp.h"
#include "cpu.h"
#include "deadlock_analysis.h"
#include "define.h"
#include "events.h"
#include "extern.h"
#include "fs.h"
#include "list.h"
#include "sched_vars.h"
#include "types.h"
#include "event_sync.h"
#include "dim_acc.h"

#include <math.h>
#ifdef USE_EQUEUE
#  include "listE.h"
#endif
#include "dim_omp.h"
#include "machine.h"
#include "memory.h"
#include "node.h"
#include "paraver.h"
#include "ports.h"
#include "read.h"
#include "schedule.h"
#include "semaphore.h"
#include "simulator.h"
#include "subr.h"
#include "task.h"

#include <assert.h>

static int progress          = 0;
static int last_node_id_used = 0;
static int SCH_prio          = 0;

t_boolean monitorize_event = FALSE;
int event_to_monitorize    = 0;


extern t_boolean is_ideal_openmp;
// void SCHEDULER_thread_to_gpu(struct t_node *node, struct t_thread *kernel_thread);

void SCHEDULER_Init()
{
  struct t_node *node;
  struct t_cpu *cpu;
  struct t_Ptask *Ptask;
  struct t_task *task;
  struct t_thread *thread;
  struct t_action *action;
  int j;
  struct t_machine *machine;

  size_t tasks_it, threads_it;

  CONFIGURATION_Load_Scheduler_Configuration();

  /* Prepare cpu ready queues */
  for ( Ptask = (struct t_Ptask *)head_queue( &Ptask_queue ); Ptask != P_NIL; Ptask = (struct t_Ptask *)next_queue( &Ptask_queue ) )
  {
    for ( tasks_it = 0; tasks_it < Ptask->tasks_count; tasks_it++ )
    {
      task    = &( Ptask->tasks[ tasks_it ] );
      node    = get_node_of_task( task );
      machine = node->machine;
      for ( threads_it = 0; threads_it < task->threads_count; threads_it++ )
      {
        thread = task->threads[ threads_it ];

        action = thread->action;
        if ( action == AC_NIL )
        {
          warning ("P%02d T%02d (t%02d) will not be simulated: empty thread\n",
                  IDENTIFIERS (thread));
          continue;
        }

        if ( action->action != WORK && action->action != GPU_BURST )
        {
          warning ("P%02d T%02d (t%02d) will not be simulated: not begins execution with work\n",
                  IDENTIFIERS (thread));
          continue;
        }

        if ( thread->task->accelerator && thread->stream )
        {
          thread->loose_cpu = FALSE;
        }
        else
        {
          thread->loose_cpu = TRUE;
        }

        ( *SCH[ machine->scheduler.policy ].init_scheduler_parameters )( thread );
        if ( thread->stream == FALSE || thread->stream_created == TRUE )
        {
          SCHEDULER_thread_to_ready( thread );
        }
      }
    }
  }
  /* Prepare the the first Scheduler Events */
  int node_id;
  for ( node_id = 0; node_id < SIMULATOR_get_number_of_nodes(); ++node_id )
  {
    node = &nodes[ node_id ];
    j    = MIN( count_queue( &( node->ready ) ), count_queue( &( node->Cpus ) ) );
    if ( j > 0 )
    {
      for ( ; j; j-- )
      {
        SCHEDULER_next_thread_to_run( node );
      }
    }
  }
}

void SCHEDULER_End()
{
  struct t_cpu *cpu;
  struct t_node *node;

  if ( debug & D_SCH )
  {
    PRINT_TIMER( current_time );
    printf( ": SCHEDULER end routine called\n" );
  }

  int node_id;
  for ( node_id = 0; node_id < SIMULATOR_get_number_of_nodes(); ++node_id )
  {
    node = &nodes[ node_id ];
    if ( count_queue( &( node->ready ) ) != 0 )
    {
      if ( debug & D_SCH )
      {
        PRINT_TIMER( current_time );
        printf( ": Warning, %d threads on ready queue for node %d\n", count_queue( &( node->ready ) ), node->nodeid );
      }
    }

    for ( cpu = (struct t_cpu *)head_queue( &( node->Cpus ) ); cpu != C_NIL; cpu = (struct t_cpu *)next_queue( &( node->Cpus ) ) )
    {
      if ( cpu->current_thread != TH_NIL )
      {
        if ( debug & D_SCH )
        {
          PRINT_TIMER( current_time );
          printf( ": Warning, cpu %d node %d is running P%02d T%02d (t%02d)\n", cpu->cpuid, node->nodeid, IDENTIFIERS( cpu->current_thread ) );
        }
      }
    }
  }
}

void SCHEDULER_reload( struct t_Ptask *Ptask )
{
  struct t_task *task;
  struct t_thread *thread;
  struct t_node *node;
  struct t_machine *machine;
  struct t_cpu *cpu;

  size_t tasks_it, threads_it;

  for ( tasks_it = 0; tasks_it < Ptask->tasks_count; tasks_it++ )
  {
    task = &( Ptask->tasks[ tasks_it ] );

    /* All threads from a given machine belong to the same node/machine */
    // thread = (struct t_thread *) head_queue (&(task->threads));
    thread = task->threads[ 0 ];

    if ( thread != TH_NIL )
    {
      node    = get_node_of_thread( thread );
      machine = node->machine;
    }

    for ( threads_it = 0; threads_it < task->threads_count; threads_it++ )
    {
      thread = task->threads[ threads_it ];

      if ( thread->action == AC_NIL )
      {
        panic( "P%02d T%02d (t%02d) without initial actions\n", IDENTIFIERS( thread ) );
      }
      if ( thread->task->accelerator && thread->stream )
      {
        thread->loose_cpu = FALSE;
      }
      else
      {
        thread->loose_cpu = TRUE;
      }
      ( *SCH[ machine->scheduler.policy ].init_scheduler_parameters )( thread );
      SCHEDULER_thread_to_ready( thread );
    }
  }
}

struct t_cpu *select_free_cpu( struct t_node *node, struct t_thread *thread )
{
  struct t_cpu *cpu;

  /* Select a free processor with cache affinity */
  for ( cpu = (struct t_cpu *)head_queue( &( node->Cpus ) ); cpu != C_NIL; cpu = (struct t_cpu *)next_queue( &( node->Cpus ) ) )
  {
    if ( ( cpu->current_thread == TH_NIL ) && ( cpu->current_thread_context == thread ) && ( cpu->is_gpu == FALSE ) )
      return ( cpu );
  }
  /* Select a free processor */
  for ( cpu = (struct t_cpu *)head_queue( &( node->Cpus ) ); cpu != C_NIL; cpu = (struct t_cpu *)next_queue( &( node->Cpus ) ) )
  {
    if ( ( cpu->current_thread == TH_NIL ) && ( cpu->is_gpu == FALSE ) )
      return ( cpu );
  }
  return ( C_NIL );
}

struct t_cpu * assign_gpu( struct t_thread *thread, struct t_node *node )
{
  struct t_cpu *cpu = NULL;
  struct t_account *account = current_account( thread );
  
  if (thread->cpu != NULL)
    return thread->cpu;
  // 1. Look for CPU with is_gpu = 1
  for ( cpu = (struct t_cpu *)head_queue( &( node->Cpus ) ); cpu != C_NIL; cpu = (struct t_cpu *)next_queue( &( node->Cpus ) ) )
  {
    // check either the cpu has thread or not.
    if ( cpu->is_gpu == TRUE && cpu->current_thread == TH_NIL )
    {
      // 2.Assigning thread to this GPU
      cpu->current_thread = thread;
      thread->cpu         = cpu;
      // context switch
      if ( cpu->current_thread_context != thread )
      {
        account->n_th_in_run++;
        cpu->current_thread_context = thread;
      }
      break;
    }
  }
  return cpu;
} 

static void put_thread_on_run( struct t_thread *thread, struct t_node *node )
{
  t_nano ti;
  struct t_account *account;
  struct t_cpu *cpu;
  dimemas_timer tmp_timer, new_time;
  struct t_machine *machine;

  machine = node->machine;

  if ( !thread->stream && !thread->host && !thread->event_sync_reentry)
  { /* If is a wait in accelerator event block, no wait thrown to fill original trace */
    PARAVER_Wait( 0, IDENTIFIERS( thread ), thread->last_paraver, current_time, PRV_SYNC_ST );
    thread->last_paraver = current_time;
  }
  account = current_account( thread );
  SUB_TIMER( current_time, thread->put_into_ready, tmp_timer );
  ADD_TIMER( tmp_timer, account->time_ready_without_cpu, account->time_ready_without_cpu );

  if ( thread->task->accelerator && thread->stream == TRUE )
  {
    // 0. Ensure that this is an heterogeneous node
    cpu = assign_gpu( thread, node );

/*    if( cpu == C_NIL )
      printf("task %d thread %d without cpu in node %d\n",thread->task->taskid,thread->threadid,node->nodeid);
    if( thread->task->taskid == 7)
      printf("task 7 thread %d with cpu %d in node %d\n",thread->threadid,thread->cpu->cpuid,node->nodeid);*/
  }
  else if ( !thread->task->accelerator || ( thread->task->accelerator && thread->host == TRUE ) )
  {
    cpu = select_free_cpu( node, thread );

    if ( cpu == C_NIL )
    {
      panic( "Can't get free processor on node %d\n", node->nodeid );
    }
    assert( cpu->is_gpu == FALSE );

    cpu->current_thread = thread;
    thread->cpu         = cpu;

    if ( cpu->current_thread_context != thread )
    {
      /* Context switch */
      account->n_th_in_run++;
      cpu->current_thread_context = thread;
    }
  }

  if ( machine->scheduler.context_switch != (t_nano)NO_CONTEXT_SWITCH )
  {
    thread->doing_context_switch = TRUE;
    thread->to_be_preempted      = FALSE;
    FLOAT_TO_TIMER( machine->scheduler.context_switch, tmp_timer );
    ADD_TIMER( current_time, machine->scheduler.minimum_quantum, thread->min_time_to_be_preempted );
    ADD_TIMER( tmp_timer, thread->min_time_to_be_preempted, thread->min_time_to_be_preempted );
    ADD_TIMER( current_time, tmp_timer, new_time );

    if ( debug & D_SCH )
    {
      PRINT_TIMER( current_time );
      printf( ": Thread context swicth P%02d T%02d (t%02d) for %.0f on CPU %d node %d\n",
              IDENTIFIERS( thread ),
              machine->scheduler.context_switch,
              cpu->cpuid,
              node->nodeid );
    }
    thread->event = EVENT_timer( new_time, NOT_DAEMON, M_SCH, thread, 0 );
    return;
  }
  ADD_TIMER( current_time, machine->scheduler.minimum_quantum, thread->min_time_to_be_preempted );

  ti = SCHEDULER_get_execution_time( thread );

  FLOAT_TO_TIMER( ti, tmp_timer );
  ADD_TIMER( current_time, tmp_timer, new_time );

  if ( debug & D_SCH )
  {
    PRINT_TIMER( current_time );
    printf( ": Thread to run P%02d T%02d (t%02d) for %.6f on CPU %d node %d\n", IDENTIFIERS( thread ), ti, cpu->cpuid, node->nodeid );
  }

  thread->event = EVENT_timer( new_time, NOT_DAEMON, M_SCH, thread, 0 );
}

void SCHEDULER_thread_to_ready( struct t_thread *thread )
{
  struct t_node *node;
  struct t_cpu *cpu;
  struct t_thread *thread_current;
  struct t_machine *machine;

  node    = get_node_of_thread( thread );
  machine = node->machine;

  if ( !more_actions( thread ) )
    panic( "Trying to run P%02d T%02d (t%02d) without actions\n", IDENTIFIERS( thread ) );

  /* FEC: Faig aixo per no haver de posar necessariament un CPU_BURST a zero
   * entre cada accio de la trac,a. */
  if ( thread->action->action != WORK && thread->action->action != GPU_BURST )
  {
    /* Estic suposant que en aquest cas es prove d'un lloc on s'ha cridat a
     * SCHEDULER_thread_to_ready just despres d'acabar una altra accio sense
     * mirar-ne el tipus. Per tant, en lloc de donar una error afegeixo una
     * accio de WORK de durada 0, mitjanc,ant la rutina
     * SCHEDULER_thread_to_ready_return.
     * Segurament en aquest cas thread->lose_cpu estara a FALSE. */
    if ( debug & D_SCH )
    {
      PRINT_TIMER( current_time );
      printf( ": Thread P%02d T%02d (t%02d) to ready and action isn't WORK! Rescheduling...\n", IDENTIFIERS( thread ) );
    }
    SCHEDULER_thread_to_ready_return( M_SCH, thread, (t_nano)0, 0 );
    return;
  }
  if ( debug & D_SCH )
  {
    PRINT_TIMER( current_time );
    printf( ": Threads on ready queue of node %d\n", node->nodeid );
    for ( cpu = (struct t_cpu *)head_queue( &( node->Cpus ) ); cpu != C_NIL; cpu = (struct t_cpu *)next_queue( &( node->Cpus ) ) )
    {
      if ( cpu->current_thread != TH_NIL )
      {
        thread_current = cpu->current_thread;
        if ( thread_current->doing_busy_wait )
        {
          printf( "\t\t  (W) P%02d T%02d (t%02d) node %d cpu %d\n", IDENTIFIERS( thread_current ), node->nodeid, cpu->cpuid );
        }
        else
        {
          printf( "\t\t  (R) P%02d T%02d (t%02d) node %d cpu %d\n", IDENTIFIERS( thread_current ), node->nodeid, cpu->cpuid );
        }
      }
    }

    for ( thread_current = (struct t_thread *)head_queue( &( node->ready ) ); thread_current != TH_NIL;
          thread_current = (struct t_thread *)next_queue( &( node->ready ) ) )
    {
      printf( "\t\t  (O) P%02d T%02d (t%02d)\n", IDENTIFIERS( thread_current ) );
    }
  }

  if ( debug & D_SCH )
    printf( "\t\t  (N) P%02d T%02d (t%02d)\n", IDENTIFIERS( thread ) );

  if ( thread->action->action == FS )
  {
    FS_general( thread->action->desc.fs_op.fs_o.fs_user_event.id, thread );
  }
  else
  {
    thread->put_into_ready = current_time;
    ( *SCH[ machine->scheduler.policy ].thread_to_ready )( thread );
  }
  reload_done = TRUE;
}

t_nano SCHEDULER_get_execution_time( struct t_thread *thread )
{
  t_nano ti;
  struct t_account *account;
  dimemas_timer tmp_timer;
  struct t_action *action;
  struct t_node *node;
  struct t_machine *machine;

  node    = get_node_of_thread( thread );
  machine = node->machine;

  action = thread->action;

  if ( action->action != WORK && action->action != GPU_BURST ) // WORK = 1 && GPU_BRUST = 18 in define.h
  {
    panic( "Trying to work when innaproppiate P%02d T%02d (t%02d) (%d)\n", IDENTIFIERS( thread ), thread->original_thread );
  }

  /* the following call " .get_execution_time " is mandatory */
  ti = ( *SCH[ machine->scheduler.policy ].get_execution_time )( thread );
  
  /* for ideal configuration ignore the burst inside the configure call */
  if ( ( thread->stream && CUDAEventEconding_Is_CUDAConfigCall ( thread->acc_in_block_event ) && thread->task->node->acc.startup == 0 ) ||
       ( is_ideal_openmp == TRUE &&
         !OMPEventEncoding_Is_OMPWorker_Running( thread->omp_in_block_event ) &&
         ( OMPEventEncoding_Is_OMPBlock( thread->omp_in_block_event.type ) && thread->omp_in_block_event.value > 0 ||
           OMPEventEncoding_Is_OMPWorker_Running_End( thread->omp_in_block_event ) ) 
       )
     )
    ti = 0;

  account = current_account( thread );
  FLOAT_TO_TIMER( ti, tmp_timer );
  ADD_TIMER( account->cpu_time, tmp_timer, account->cpu_time );

  return ( ti );
}

void SCHEDULER_next_thread_to_run( struct t_node *node )
{
  struct t_thread *thread;
  struct t_cpu *cpu;
  struct t_machine *machine;

  machine = node->machine;

  if ( debug & D_SCH )
  {
    PRINT_TIMER( current_time );
    printf( ": Threads on ready queue of node %d to RUN\n", node->nodeid );
    for ( cpu = (struct t_cpu *)head_queue( &( node->Cpus ) ); cpu != C_NIL; cpu = (struct t_cpu *)next_queue( &( node->Cpus ) ) )
    {
      if ( cpu->current_thread != TH_NIL )
      {
        thread = cpu->current_thread;
        printf( "\t\t   Running P%02d T%02d (t%02d) node %d cpu %d\n", IDENTIFIERS( thread ), node->nodeid, cpu->cpuid );
      }
    }
    for ( thread = (struct t_thread *)head_queue( &( node->ready ) ); thread != TH_NIL; thread = (struct t_thread *)next_queue( &( node->ready ) ) )
    {
      printf( "\t\t     (O) P%02d T%02d (t%02d)\n", IDENTIFIERS( thread ) );
    }
  }

  thread = ( *SCH[ machine->scheduler.policy ].next_thread_to_run )( node );

  /* FEC: Per evitar haver de tenir CPU_BURSTS a zero afegeixo aquest if
   * tot i que crec que no es possible que s'arribi aqui i no hi
   * hagi cap thread a ready (per exemple despres d'un message_receive
   * es crida a SCHEDULER_thread_to_ready i a SCHEDULER_general
   * consecutivament) perque en aquest cas s'hauria d'haver afegit
   * una accio de WORK amb durada 0.
   */
  if ( thread == TH_NIL )
  {
    panic( "Null thread scheduled to run!\n" );
  }
  /*Fi FEC: canvi per evitar CPU_BURST a zero *************************/

  if ( debug & D_SCH )
  {
    PRINT_TIMER( current_time );
    printf( ": Selected thread P%02d T%02d (t%02d)\n", IDENTIFIERS( thread ) );
  }

  put_thread_on_run( thread, node );
}

void scheduler_treat_event(struct t_thread *thread, struct t_even *event )
{
  if ( debug & D_SCH )
  {
    PRINT_TIMER( current_time );
    printf( ": SCHEDULER general P%02d T%02d (t%02d) User Event %llu (%llu)\n",
            IDENTIFIERS( thread ),
            event->type,
            event->value );
  }

  if ( monitorize_event )
  {
    if ( event_to_monitorize == event->type )
    {
      dimemas_timer tmp_timer;

      SUB_TIMER( current_time, thread->last_time_event_number, tmp_timer );
      fprintf( File_for_Event, "Event monitorized  P%02d T%02d (th%02d) distance ", IDENTIFIERS( thread ) );
      FPRINT_TIMER( File_for_Event, tmp_timer );
      fprintf( File_for_Event, "\n" );
      ASS_ALL_TIMER( thread->last_time_event_number, current_time );
    }
  }

  /* JGG (2012/01/10): New module management */
  if ( event->value != (unsigned long int)0 )
  {
    if ( debug & D_SCH )
    {
      PRINT_TIMER( current_time );
      printf( ": Checking 'module_entrance [%lld:%lld] for P%02d T%02d (t%02d)\n",
              event->type,
              event->value,
              IDENTIFIERS( thread ) );
    }
    module_entrance( thread, (unsigned long int)event->type, (unsigned long int)event->value );
  }
  else
  {
    module_exit( thread, (unsigned long int)event->type );
  }

  if ( event->type == PRIORITY_SET_EVENT )
  {
    struct t_machine *machine = get_node_of_thread( thread )->machine;

    if ( event->value == 0 )
    {
      /* this is the ultimate priority that a task can have
          highest priority + preempting tasks with smaller priorities */
      ( *SCH[ machine->scheduler.policy ].modify_priority )( thread, ( t_priority )( 0 ) );
      /* this is a task that can preempt*/
      ( *SCH[ machine->scheduler.policy ].modify_preemption )( thread, (t_priority)1 );
    }
    else
    {
      /* this is a regular priority of a task */
      t_priority new_priority = 1 / (t_priority)event->value;
      ( *SCH[ machine->scheduler.policy ].modify_priority )( thread, ( t_priority )( new_priority ) );
    }
  }

  if ( event->type == PREEMPTION_SET_EVENT )
  {
    printf( "warning - this was BEFORE used as an user event for PREEMPTION_SET_EVENT - BUT NOT ANY MORE???\n" );
  }

  /* remember the sstaskid of this thread */
  if ( event->type == USER_EVENT_TYPE_TASKID_START_TASK )
  {
    if ( event->type != 0 )
      thread->sstask_id = event->value;
  }

  /* remember the sstask_type of this thread */
  if ( event->type == USER_EVENT_TYPE_TASKTYPE_START_TASK )
  {
    if ( event->type != 0 )
      thread->sstask_type = event->value;
  }

  /* treat acc events */
  treat_acc_event( thread, event );
  /* treating OMP events */
  treat_omp_events( thread, event, current_time );

  /* Not printing block end if it is a clEnqueueNDRangeKernel because
    * it has been printed yet in COMMUNIC_SEND
    * Not printing event if stream needs a previous sync (in barrier)
    */
  int printing_event =
    !thread->acc_recv_sync &&
    !( OCLEventEncoding_Is_OCLKernelRunning( thread->acc_in_block_event ) && thread->stream && event->value == 0 );
  if ( printing_event )
  { /* If it is not an accelerator event that has to wait to be written */
    struct t_cpu *cpu;
    
    cpu = get_cpu_of_thread( thread );
    PARAVER_Event( cpu->unique_number, IDENTIFIERS( thread ), current_time, event->type, event->value );
  }
}

void SCHEDULER_general( int value, struct t_thread *thread )
{
  register struct t_action *action;
  register struct t_cpu *cpu;
  register struct t_node *node;
  register t_nano ti;
  dimemas_timer tmp_timer;
  struct t_machine *machine;

  cpu     = get_cpu_of_thread( thread );
  node    = get_node_of_thread( thread );
  machine = node->machine;
  if ( debug & D_SCH )
  {
    PRINT_TIMER( current_time );
    printf( ": SCHEDULER general %d for P%02d, T%02d, (t%02d) on CPU %d node %d\n", value, IDENTIFIERS( thread ), cpu->cpuid, node->nodeid );
  }

  switch ( value )
  {
    case SCH_TIMER_OUT:
    {
      if ( thread->doing_acc_comm )
      {
        thread->doing_acc_comm = FALSE;
        thread->startup_done   = TRUE;
        thread->doing_startup  = FALSE;
        new_cp_node( thread, CP_OVERHEAD );
      }
      else if ( thread->doing_startup )
      {
        PARAVER_Startup( cpu->unique_number, IDENTIFIERS( thread ), thread->last_paraver, current_time );
        new_cp_node( thread, CP_OVERHEAD );
        thread->startup_done  = TRUE;
        thread->doing_startup = FALSE;
      }
      else if ( thread->doing_copy )
      { /* At this point thread has done its data copy to internal library */
        PARAVER_Data_Copy( cpu->unique_number, IDENTIFIERS( thread ), thread->last_paraver, current_time );
        new_cp_node( thread, CP_OVERHEAD );
        thread->copy_done  = TRUE;
        thread->doing_copy = FALSE;
      }
      else if ( thread->doing_roundtrip )
      { /* At this point thread has done send round trip */
        PARAVER_RTT( cpu->unique_number, IDENTIFIERS( thread ), thread->last_paraver, current_time );
        new_cp_node( thread, CP_OVERHEAD );
        thread->roundtrip_done  = TRUE;
        thread->doing_roundtrip = FALSE;
      }
      else
      {
        if ( thread->doing_context_switch )
        {
          PARAVER_Ctx_Switch( cpu->unique_number, IDENTIFIERS( thread ), thread->last_paraver, current_time );
        }
        else
        {
          if ( thread->doing_busy_wait )
          {
            PARAVER_Busy_Wait( cpu->unique_number, IDENTIFIERS( thread ), thread->last_paraver, current_time );
          }
          else
          {
            action = thread->action;

            if ( thread->stream && !thread->first_acc_event_read )
            {
              /* Previous at accelerator events in stream thread must be NOT_CREATED state in CPU	*/
              PARAVER_Not_Created( cpu->unique_number, IDENTIFIERS( thread ), thread->last_paraver, current_time );
            }
            else if ( thread->idle_block )
            {
              PARAVER_Idle( cpu->unique_number, IDENTIFIERS( thread ), thread->last_paraver, current_time );
            }
            else
            {
              if ( !thread->task->accelerator && !thread->task->openmp )
              { /*	It's a CPU burst	*/
                PARAVER_Running( cpu->unique_number, IDENTIFIERS( thread ), thread->last_paraver, current_time );
              }
              else if ( ( thread->host || thread->stream ) &&
                        ( CUDAEventEncoding_Is_CUDABlock( thread->acc_in_block_event.type ) ||
                          OCLEventEncoding_Is_OCLBlock( thread->acc_in_block_event.type ) ) &&
                        CUDAEventEncoding_Is_BlockBegin( thread->acc_in_block_event.value ) )
              {
                /* Do not throw anything if host or stream is inside a CUDA or OpenCL event block	*/
              }
              else if ( thread->omp_worker_thread || ( thread->omp_master_thread && OMPEventEncoding_Is_OMPType( thread->omp_in_block_event.type ) ) )
              {
                // Do not throw anything if it is inside ompblock
              }
              else
              {
                /*It's a CPU burst*/
                PARAVER_Running( cpu->unique_number, IDENTIFIERS( thread ), thread->last_paraver, current_time );
              }
            }
            new_cp_node( thread, CP_WORK );
          }
        }
      }
      thread->last_paraver = current_time;
      // Accelerator stream threads always in same gpu
      if( !( thread->task->accelerator == TRUE && thread->stream == TRUE ) )
        cpu->current_thread  = TH_NIL;

      if ( thread->doing_context_switch )
      {
        thread->doing_context_switch = FALSE;
        if ( thread->to_be_preempted )
        {
          thread->loose_cpu = FALSE;
          SCHEDULER_thread_to_ready( thread );
          SCHEDULER_next_thread_to_run( node );
        }
        else
        {
          cpu->current_thread = thread;
          ti                  = SCHEDULER_get_execution_time( thread );

          ADD_TIMER( machine->scheduler.minimum_quantum, current_time, thread->min_time_to_be_preempted );

          FLOAT_TO_TIMER( ti, tmp_timer );
          ADD_TIMER( current_time, tmp_timer, tmp_timer );
          if ( debug & D_SCH )
          {
            PRINT_TIMER( current_time );
            printf( ": Thread run context P%02d T%02d (t%02d) for %.0f on CPU %d node %d\n", IDENTIFIERS( thread ), ti, cpu->cpuid, node->nodeid );
          }
          thread->event = EVENT_timer( tmp_timer, NOT_DAEMON, M_SCH, thread, 0 );
        }
        break;
      }
      if ( thread->doing_busy_wait )
      {
        thread->doing_busy_wait = FALSE;
        COMMUNIC_block_after_busy_wait( thread );
        if ( count_queue( &( node->ready ) ) != 0 )
          SCHEDULER_next_thread_to_run( node );
        break;
      }
    next_op:
      if ( more_actions( thread ) )
      {
        action = thread->action;

        switch ( action->action )
        {
          case NOOP:
          {
            /* Special action to separate simultaneous events */
            PARAVER_Start_Op( cpu->unique_number, IDENTIFIERS( thread ), current_time );

            thread->action = action->next;
            READ_free_action( action );

            if ( more_actions( thread ) )
            {
              action = thread->action;
              if ( action->action != WORK && action->action != GPU_BURST )
                goto next_op;

              thread->loose_cpu = FALSE;
              SCHEDULER_thread_to_ready( thread );
            }

            break;
          }
          case SEND:
          {
            PARAVER_Start_Op( cpu->unique_number, IDENTIFIERS( thread ), current_time );

            COMMUNIC_send( thread );
            break;
          }
          case RECV:
          {
            PARAVER_Start_Op( cpu->unique_number, IDENTIFIERS( thread ), current_time );

            COMMUNIC_recv( thread );
            break;
          }
          case IRECV:
          {
            PARAVER_Start_Op( cpu->unique_number, IDENTIFIERS( thread ), current_time );

            COMMUNIC_Irecv( thread );
            break;
          }
          case WAIT:
          {
            PARAVER_Start_Op( cpu->unique_number, IDENTIFIERS( thread ), current_time );

            COMMUNIC_wait( thread );
            break;
          }
          case WAIT_FOR_SEND:
          {
            PARAVER_Start_Op( cpu->unique_number, IDENTIFIERS( thread ), current_time );

            panic( "wait for send, I did not expect that" );
            break;
          }
          case WORK:
          {
            PARAVER_Start_Op( cpu->unique_number, IDENTIFIERS( thread ), current_time );

            thread->loose_cpu = TRUE;
            SCHEDULER_thread_to_ready( thread );
            break;
          }
          case GPU_BURST:
          {
            PARAVER_Start_Op( cpu->unique_number, IDENTIFIERS( thread ), current_time );

            thread->loose_cpu = TRUE;
            SCHEDULER_thread_to_ready( thread );
            break;
          }
          case EVENT:
          {
            if( capture_previous_events( thread, &action->desc.even, thread->threadid) == FALSE )
            {
              if( event_sync_add( thread->task, &action->desc.even, thread->threadid, PARTNER_ID_BARRIER, FALSE ) )
                return;

              scheduler_treat_event( thread, &action->desc.even );
            }

            thread->action = action->next;
            READ_free_action( action );

            if ( more_actions( thread ) )
            {
              action = thread->action;
              if ( action->action != WORK && action->action != GPU_BURST )
                goto next_op;

              if ( thread->stream == TRUE )
                thread->loose_cpu = FALSE;
              else
                thread->loose_cpu = TRUE;
              SCHEDULER_thread_to_ready( thread );
            }
            break;
          }
          case FS:
          {
            PARAVER_Start_Op( cpu->unique_number, IDENTIFIERS( thread ), current_time );

            thread->loose_cpu = FALSE;
            FS_general( action->desc.fs_op.fs_o.fs_user_event.id, thread );
            break;
          }
          case SEM:
          {
            PARAVER_Start_Op( cpu->unique_number, IDENTIFIERS( thread ), current_time );
            switch ( action->desc.sem_op.op )
            {
              case SEM_WAIT:
                thread->action = action->next;
                SEMAPHORE_wait( action->desc.sem_op.sem_id, thread );
                READ_free_action( action );
                break;
              case SEM_SIGNAL:
                SEMAPHORE_signal( action->desc.sem_op.sem_id, thread );
                thread->action = action->next;
                READ_free_action( action );
                if ( more_actions( thread ) )
                {
                  thread->loose_cpu = FALSE;
                  SCHEDULER_thread_to_ready( thread );
                }
                break;
              case SEM_SIGNAL_N:
                SEMAPHORE_signal_n( action->desc.sem_op.sem_id, action->desc.sem_op.n, thread );
                thread->action = action->next;
                READ_free_action( action );
                if ( more_actions( thread ) )
                {
                  thread->loose_cpu = FALSE;
                  SCHEDULER_thread_to_ready( thread );
                }
                break;
              default:
                panic( "Invalid semaphore action\n" );
            }
            break;
          }
          case PORT_SEND:
          {
            PARAVER_Start_Op( cpu->unique_number, IDENTIFIERS( thread ), current_time );

            PORT_send( action->desc.port.module, action->desc.port.portid, thread, action->desc.port.size );
            thread->action = action->next;
            READ_free_action( action );
            break;
          }
          case PORT_RECV:
          {
            PARAVER_Start_Op( cpu->unique_number, IDENTIFIERS( thread ), current_time );

            PORT_receive( action->desc.port.module, action->desc.port.portid, thread, action->desc.port.size );
            thread->action = action->next;
            READ_free_action( action );
            break;
          }
          case MEMORY_COPY:
          {
            PARAVER_Start_Op( cpu->unique_number, IDENTIFIERS( thread ), current_time );

            MEMORY_copy_segment( action->desc.memory.module, thread, action->desc.memory.source, action->desc.memory.dest, action->desc.memory.size );

            thread->action = action->next;
            READ_free_action( action );
            break;
          }
          case GLOBAL_OP:
          {
            if ( action->desc.global_op.comm_id >= 0 )
            { /* Regular execution of a global operation */


              if ( action->desc.global_op.synch_type != GLOBAL_OP_WAIT )
              {
                PARAVER_Start_Op( cpu->unique_number, IDENTIFIERS( thread ), current_time );

                GLOBAL_operation( thread,
                                  action->desc.global_op.glop_id,
                                  action->desc.global_op.comm_id,
                                  action->desc.global_op.is_root,
                                  action->desc.global_op.root_thid,
                                  action->desc.global_op.bytes_send,
                                  action->desc.global_op.bytes_recvd,
                                  action->desc.global_op.synch_type );
              }
              else if ( action->desc.global_op.synch_type == GLOBAL_OP_WAIT )
              {
                GLOBAL_wait_operation( thread );
                // Ignore
                /*
                   READ_action_free(action);
                   goto next_op;
                   */
              }
            }
            else
            { /* Pseudo global operation to implement accelerator
               * synchronizations */
              ACCELERATOR_synchronization( thread, action->desc.global_op.comm_id );
            }
            break;
          }
          case MPI_IO:
          {
            PARAVER_Start_Op( cpu->unique_number, IDENTIFIERS( thread ), current_time );

            FS_general( FS_OPERATION, thread );
            break;
          }
          case MPI_OS:
          {
            PARAVER_Start_Op( cpu->unique_number, IDENTIFIERS( thread ), current_time );

            ONE_SIDED_operation( thread );
            break;
          }
          default:
            panic( "Unknown action %d to P%02d T%02d (t%02d)\n", action->action, IDENTIFIERS( thread ) );
        }
      }

      if ( thread->stream == TRUE )
        thread->loose_cpu = FALSE;
      /*assert (thread->loose_cpu == FALSE);*/

      if ( thread->acc_put_on_run == TRUE )
      {
        thread->acc_put_on_run = FALSE;
        // put_thread_on_run( thread, node );
        SCHEDULER_next_thread_to_run( node );
      }
      else if ( ( count_queue( &( node->ready ) ) != 0 ) && ( num_free_cpu( node ) > 0 ) )
      {
        SCHEDULER_next_thread_to_run( node );
      }
      break;
    }
    case SCH_NEW_JOB:
    {
      if ( num_free_cpu( node ) > 0 || thread->stream == TRUE )
      {
        /* The new one is the unique one */
        SCHEDULER_next_thread_to_run( node );
      }
      break;
    }
    default:
      panic( "Invalid interface value %d in scheduler routines\n", value );
  }
}

void SCHEDULER_thread_to_busy_wait( struct t_thread *thread )
{
  struct t_cpu *cpu;
  dimemas_timer tmp_timer, new_time;
  struct t_node *node;
  struct t_machine *machine;

  node    = get_node_of_thread( thread );
  machine = node->machine;

  if ( debug & D_SCH )
  {
    PRINT_TIMER( current_time );
    printf( ": Thread to busy wait P%02d T%02d (t%02d) for %f\n", IDENTIFIERS( thread ), machine->scheduler.context_switch );
  }

  cpu                              = get_cpu_of_thread( thread );
  cpu->current_thread              = thread;
  thread->doing_busy_wait          = TRUE;
  thread->min_time_to_be_preempted = current_time;
  FLOAT_TO_TIMER( machine->scheduler.context_switch, tmp_timer );
  ADD_TIMER( current_time, tmp_timer, new_time );
  thread->event = EVENT_timer( new_time, NOT_DAEMON, M_SCH, thread, 0 );
}

void SCHEDULER_copy_parameters( struct t_thread *th_o, struct t_thread *th_d )
{
  struct t_node *node;
  struct t_machine *machine;

  node    = get_node_of_thread( th_o );
  machine = node->machine;
  ( *SCH[ machine->scheduler.policy ].scheduler_copy_parameters )( th_o, th_d );
}

void SCHEDULER_free_parameters( struct t_thread *thread )
{
  struct t_node *node;
  struct t_machine *machine;

  node    = get_node_of_thread( thread );
  machine = node->machine;
  ( *SCH[ machine->scheduler.policy ].scheduler_free_parameters )( thread );
}

int SCHEDULER_info( int value, int info, struct t_thread *th_s, struct t_thread *th_r )
{
  int return_value = 0;
  struct t_node *node;
  struct t_machine *machine;

  node    = get_node_of_thread( th_s );
  machine = node->machine;
  switch ( value )
  {
    case CONTEXT_SWITCH_TIMER_OUT:
      ( *SCH[ machine->scheduler.policy ].info )( info );
      break;
    case COMMUNICATION_INFO:
      return_value = ( *SCH[ machine->scheduler.policy ].info )( info, th_s, th_r );
      break;
    default:
      panic( "Unknown info %d to scheduler value\n", value );
  }
  return ( return_value );
}

void SCHEDULER_thread_to_ready_return( int module, struct t_thread *thread, t_nano ti, int id )
{
  struct t_action *action, *cur_action;

  cur_action = thread->action;
  switch ( module )
  {
    case M_FS:
      // action = (struct t_action *) malloc (sizeof (struct t_action));
      READ_create_action( &action );

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
      panic( "Invalid module identifier %d\n", module );
  }

  // action = (struct t_action *) malloc (sizeof (struct t_action));
  READ_create_action( &action );

  action->next                  = cur_action;
  action->action                = WORK;
  action->desc.compute.cpu_time = ti;
  thread->action                = action;
  SCHEDULER_thread_to_ready( thread );
}

struct t_thread *SCHEDULER_preemption( struct t_thread *thread, struct t_cpu *cpu )
{
  struct t_node *node;
  struct t_thread *thread_current;
  struct t_account *account_current, *account;
  struct t_action *action;
  dimemas_timer when;
  t_nano ti;

  thread_current  = cpu->current_thread;
  account_current = current_account( thread_current );
  account         = current_account( thread );
  node            = get_node_of_thread( thread );


  if ( debug & D_SCH )
  {
    PRINT_TIMER( current_time );
    printf( ": Preemption from P%02d T%02d (t%02d) to P%02d T%02d (t%02d) ", IDENTIFIERS( thread ), IDENTIFIERS( thread_current ) );
  }

  if ( thread_current->doing_busy_wait )
  {
    if ( debug & D_SCH )
    {
      printf( "doing busy wait\n" );
    }
    printf( "SCHEDULER_preemption doing busy wait\n" );
    EVENT_extract_timer( M_SCH, thread_current, &when );

    PARAVER_Busy_Wait( cpu->unique_number, IDENTIFIERS( thread_current ), thread_current->last_paraver, current_time );

    thread_current->last_paraver = current_time;
    SUB_TIMER( when, current_time, when );
    TIMER_TO_FLOAT( when, ti );
    thread_current->doing_busy_wait = FALSE;
    COMMUNIC_block_after_busy_wait( thread_current );
    thread_current = TH_NIL;
  }
  else
  {
    if ( GT_TIMER( thread_current->min_time_to_be_preempted, current_time ) )
    {
      if ( GT_TIMER( thread_current->next_event_timer, thread_current->min_time_to_be_preempted ) )
      {
        EVENT_extract_timer( M_SCH, thread_current, &when );
        SUB_TIMER( when, current_time, when );
        TIMER_TO_FLOAT( when, ti );
        SUB_TIMER( account_current->cpu_time, when, account_current->cpu_time );
        if ( ( thread_current->action != AC_NIL ) && ( thread_current->action->action == WORK ) )
        {
          thread_current->action->desc.compute.cpu_time += ti;
        }
        else
        {
          READ_create_action( &action );
          action->next                  = thread_current->action;
          action->action                = WORK;
          action->desc.compute.cpu_time = ti;
          thread_current->action        = action;
        }
        printf( "I DONT EXPECT IT TO COME HERE - VLADIMIR\n" );

        thread->event = EVENT_timer( thread_current->min_time_to_be_preempted, NOT_DAEMON, M_SCH, thread_current, 0 );
      }

      if ( debug & D_SCH )
        printf( " min quantum requiered (delayed)\n" );
      return ( thread );
    }
  }

  cpu->current_thread = TH_NIL;

  if ( debug & D_SCH )
  {
    printf( "normal case\n" );
  }

  if ( thread_current != TH_NIL )
  {
    EVENT_extract_timer( M_SCH, thread_current, &when );

    READ_create_action( &action );

    action->next   = thread_current->action;
    action->action = WORK;

    SUB_TIMER( when, current_time, when );
    TIMER_TO_FLOAT( when, ti );
    action->desc.compute.cpu_time  = ti;
    thread_current->action         = action;
    thread_current->put_into_ready = current_time;

    SUB_TIMER( account_current->cpu_time, when, account_current->cpu_time );

    if ( thread->idle_block == TRUE )
    {
      PARAVER_Idle( cpu->unique_number, IDENTIFIERS( thread ), thread->last_paraver, current_time );
    }
    else
    {
      PARAVER_Running( cpu->unique_number, IDENTIFIERS( thread ), thread->last_paraver, current_time );
    }

    // Vladimir: this is a fix to print good prv file with task names and sequence numbers
    if ( thread_current->sstask_id > 0 )
    {
      /* add user events to mark that this task is finished (PREEMPTED) - TASK_ID*/
      PARAVER_Event( cpu->unique_number, IDENTIFIERS( thread_current ), current_time, USER_EVENT_TYPE_TASKID_START_TASK, 0 );
      /* and that it will start again when it gets cpu (PREEMPTED) - TASK_ID*/
      // action = (struct t_action *) malloc (sizeof (struct t_action));
      READ_create_action( &action );

      action->next            = thread_current->action;
      action->action          = EVENT;
      action->desc.even.type  = USER_EVENT_TYPE_TASKID_START_TASK;
      action->desc.even.value = thread_current->sstask_id;
      thread_current->action  = action;
    }

    if ( thread_current->sstask_type > 0 )
    {
      /* add user events to mark that this task is finished (PREEMPTED) - TASK_TYPE*/
      PARAVER_Event( cpu->unique_number, IDENTIFIERS( thread_current ), current_time, USER_EVENT_TYPE_TASKTYPE_START_TASK, 0 );
      /* and that it will start again when it gets cpu (PREEMPTED) - TASK_TYPE*/
      // action = (struct t_action *) malloc (sizeof (struct t_action));

      READ_create_action( &action );

      action->next            = thread_current->action;
      action->action          = EVENT;
      action->desc.even.type  = USER_EVENT_TYPE_TASKTYPE_START_TASK;
      action->desc.even.value = thread_current->sstask_type;
      thread_current->action  = action;
    }

    // Add one empty burst so It could work fine
    if ( ( thread_current->sstask_id > 0 ) || ( thread_current->sstask_type > 0 ) )
    {
      READ_create_action( &action );

      action->next   = thread_current->action;
      action->action = WORK;

      action->desc.compute.cpu_time  = 0.0;
      thread_current->action         = action;
      thread_current->put_into_ready = current_time;
    }
    // Vladimir: DONE fix

    thread_current->last_paraver = current_time;
  }

  if ( thread->action->action != WORK && thread->action->action != GPU_BURST )
    panic( "Next action for P%02d T%02d (t%02d) must be work\n", IDENTIFIERS( thread ) );

  put_thread_on_run( thread, node );

  account_current->n_preempt_to_me++;
  account->n_preempt_to_other++;
  return ( thread_current );
}

int SCHEDULER_get_policy( char *s )
{
  int i = 0;

  while ( SCH[ i ].name != 0 )
  {
    if ( strcmp( s, SCH[ i ].name ) == 0 )
    {
      return i;
    }
    i++;
  }
  panic( "Invalid scheduling policy name %s\n", s );
  return ( -1 );
}

t_boolean more_actions( struct t_thread *thread )
{
  struct t_action *action;
  struct t_account *account;
  struct t_Ptask *Ptask;
  struct t_task *task;

  if ( thread->action == NULL )
  {
    READ_get_next_action( thread );
  }

  action  = thread->action;
  account = current_account( thread );
  Ptask   = thread->task->Ptask;
  task    = thread->task;

  if ( action == NULL )
  {
    if ( with_deadlock_analysis )
    {
      DEADLOCK_thread_finalized( thread );
    }

    if ( debug & D_SCH )
    {
      PRINT_TIMER( current_time );
      printf( ": SCHEDULER no more actions (NULL) P%02d T%02d (t%02d)\n", IDENTIFIERS( thread ) );
    }

    account->final_time = current_time;
    if ( ( reload_Ptasks ) && ( more_actions_on_Ptask( Ptask ) == FALSE ) )
    {
      reload_new_Ptask( Ptask );
    }
    else
    {
      struct t_cpu *cpu;
      if ( !thread->stream )
      { /* In stream thread last cpu state is Not_Created thrown by host (above) */
        cpu = get_cpu_of_thread( thread );
        PARAVER_Dead( cpu->unique_number, IDENTIFIERS( thread ), current_time );
      }

      /* Not_created state in stream from last paraver event to end of trace */
      if ( thread->host )
      {
        int threads_it;
        struct t_thread *tmp_thread;
        for ( threads_it = 0; threads_it < task->threads_count; threads_it++ )
        {
          tmp_thread = task->threads[ threads_it ];
          if ( tmp_thread->stream )
          {
            PARAVER_Not_Created( cpu->unique_number, IDENTIFIERS( tmp_thread ), tmp_thread->last_paraver, current_time );
          }
        }
      }
      thread->last_paraver = current_time;
    }

    Simulator.finished_threads_count++;

    if ( ( ( Simulator.finished_threads_count * 100 ) / Simulator.threads_count ) == (unsigned)progress + 10 )
    {
      progress = ( ( Simulator.finished_threads_count * 100 ) / Simulator.threads_count );
      info( "..%d%%..   ", progress );


      if ( progress == 100 )
      {
        info( "\n" );
      }
      // fflush(stdout);
    }

    return ( FALSE );
  }

  if ( action->action == DEAD )
  {
    if ( debug & D_SCH )
    {
      PRINT_TIMER( current_time );
      printf( ": SCHEDULER no more actions (DEAD) P%02d T%02d (t%02d)\n", IDENTIFIERS( thread ) );
    }

    account->final_time = current_time;
    if ( ( reload_Ptasks ) && ( more_actions_on_Ptask( Ptask ) == FALSE ) )
    {
      reload_new_Ptask( Ptask );
    }
    else
    {
      PARAVER_Dead( 0, IDENTIFIERS( thread ), current_time );
    }
    return ( FALSE );
  }
  return ( TRUE );
}
