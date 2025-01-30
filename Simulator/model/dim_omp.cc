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
}

using namespace std; 

/**
 * Treating the OpenMP events
 * If any OpenMP states exist we have to print as they were
 * else we will print the events as they were.
 */
void treat_omp_events( struct t_thread *thread, struct t_even *event, dimemas_timer current_time )
{
  struct t_cpu *cpu;
  cpu = get_cpu_of_thread( thread );
  
  if( !OMPEventEncoding_Is_OMPBlock( event->type ) )
  {
    return;
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
  else if( thread->omp_worker_thread ) 
  {
    if( thread->omp_in_block_event.type == 0 )
    {
      PARAVER_Not_Created( cpu->unique_number,
                           IDENTIFIERS (thread),
                           0,
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
}
