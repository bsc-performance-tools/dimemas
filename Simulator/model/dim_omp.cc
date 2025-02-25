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

extern "C" {
  #include "types.h"
  #include "dim_omp.h"
  #include "define.h"
  #include "EventEncoding.h"
  #include "paraver.h"
  #include "schedule.h"
}

#include <map>
#include <vector>

using namespace std; 

struct OMPTasks
{
  std::map< unsigned long long, struct t_thread * > waitingExeTasks;
};

struct OMPTasks_ThreadInfo
{
  bool keep_capturing_events = false;
  std::vector< t_even > captured_events;
};

struct OMPTasks *createOpenMPTasks()
{
  return new struct OMPTasks;
}

struct OMPTasks_ThreadInfo *createOpenMPTasks_ThreadInfo()
{
  return new struct OMPTasks_ThreadInfo;
}

void dumpOMPTaskCapturedEvents( struct t_thread * thread )
{
  struct t_cpu *cpu;
  cpu = get_cpu_of_thread( thread );
  
  for( auto ev: thread->omp_tasks_thread_info->captured_events )
    PARAVER_Event( cpu->unique_number, IDENTIFIERS( thread ), current_time, ev.type, ev.value );

  thread->omp_tasks_thread_info->captured_events.clear();
}


/**
 * Treating the OpenMP events
 * If any OpenMP states exist we have to print as they were
 * else we will print the events as they were.
 */
scheduler_synchronization treat_omp_events( struct t_thread *thread, struct t_even *event, dimemas_timer current_time )
{
  struct t_cpu *cpu;
  cpu = get_cpu_of_thread( thread );
  
  if( thread->omp_tasks_thread_info->keep_capturing_events && event->type != OMP_TASK_IDENTIFIER )
  {
    thread->omp_tasks_thread_info->captured_events.push_back( *event );
    return NO_PRINT_EVENT;
  }

  if( OMPEventEncoding_Is_InitTask( thread->omp_in_block_event ) && event->type == OMP_TASK_IDENTIFIER )
  {
    auto tmpThread = thread->task->omp_tasks->waitingExeTasks.find( event->value );
    if( tmpThread != thread->task->omp_tasks->waitingExeTasks.end() )
    {
      tmpThread->second->event_sync_reentry = TRUE;
      tmpThread->second->loose_cpu = TRUE;
      SCHEDULER_thread_to_ready( tmpThread->second );
    }
    else
      thread->task->omp_tasks->waitingExeTasks.insert( { event->value, nullptr } );

    return CONTINUE;
  }

  if( OMPEventEncoding_Is_ExeTask( thread->omp_in_block_event ) && event->type == OMP_TASK_IDENTIFIER )
  {
    auto tmpThread = thread->task->omp_tasks->waitingExeTasks.find( event->value );
    if( tmpThread != thread->task->omp_tasks->waitingExeTasks.end() )
    {
      thread->task->omp_tasks->waitingExeTasks.erase( tmpThread );
      dumpOMPTaskCapturedEvents( thread );
      thread->omp_tasks_thread_info->keep_capturing_events = false;
      thread->omp_in_block_event.paraver_time = current_time;
      return CONTINUE;
    }
    else
    {
      thread->task->omp_tasks->waitingExeTasks.insert( { event->value, thread } );
      return WAIT_FOR_SYNC;
    }
  }


  if( !OMPEventEncoding_Is_OMPBlock( event->type ) )
  {
    return CONTINUE;
  }

  if( OMPEventEncoding_Is_OMP_Running( thread->omp_in_block_event ) )
  {
    PARAVER_Running( cpu->unique_number,
                     IDENTIFIERS( thread ),
                     thread->omp_in_block_event.paraver_time,
                     current_time );
  }
  else if(OMPEventEncoding_Is_OMPSync(thread->omp_in_block_event))
  {
    PARAVER_Thread_Sync( cpu->unique_number,
                         IDENTIFIERS( thread ),
                         thread->omp_in_block_event.paraver_time,
                         current_time );
  }
  else if(OMPEventEncoding_Is_OMPSched(thread->omp_in_block_event))
  {
    PARAVER_Thread_Sched( cpu->unique_number,
                          IDENTIFIERS(thread),
                          thread->omp_in_block_event.paraver_time,
                          current_time);
  }
  else if( thread->omp_master_thread )
  {
    if( OMPEventEncoding_Is_OMP_fork_begin( thread->omp_in_block_event ) || 
        OMPEventEncoding_Is_OMP_fork_end( thread->omp_in_block_event ) )
    {
      PARAVER_Thread_Sched( cpu->unique_number,
                            IDENTIFIERS( thread ),
                            thread->omp_in_block_event.paraver_time,
                            current_time );
    }
  }

  if( ( event->type == OMP_TASKWAIT && event->value == OMP_BEGIN_VAL ) ||
      ( event->type == OMP_BARRIER && event->value == OMP_BEGIN_VAL ) )
    thread->omp_in_block_event.inWaitBlock = TRUE;
  else if( ( event->type == OMP_TASKWAIT && event->value == OMP_END_VAL ) ||
           ( event->type == OMP_BARRIER && event->value == OMP_END_VAL ) )
    thread->omp_in_block_event.inWaitBlock = FALSE;

  if( event->type == OMP_PARALLEL_EV && event->value == OMP_END_VAL )
  {
    thread->omp_in_block_event.type = 0;
  }
  else
    thread->omp_in_block_event.type = event->type;
  
  thread->omp_in_block_event.value = event->value;
  thread->omp_in_block_event.paraver_time = current_time;

  if( OMPEventEncoding_Is_ExeTask( thread->omp_in_block_event ) )
  {
    thread->omp_tasks_thread_info->keep_capturing_events = true;
    thread->omp_tasks_thread_info->captured_events.push_back( *event );
    return NO_PRINT_EVENT;
  }

  return CONTINUE;
}
