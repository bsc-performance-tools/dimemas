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
  #include "dim_acc.h"
  #include "define.h"
  #include "EventEncoding.h"
  #include "paraver.h"
  #include "schedule.h"
}

#include "event_sync.h"

/***************************************************************
 ** treat_acc_event
 ************************
 ** if action->desc.even is an accelerator event (CUDA or OpenCL), affects
 ** to cpu (or gpu) states, communications.
 ** Updates acc_in_block_event

 ***************************************************************/
void treat_acc_event( struct t_thread *thread, struct t_even *event )
{
  if ( !CUDAEventEncoding_Is_CUDABlock( event->type ) && !OCLEventEncoding_Is_OCLBlock( event->type ) &&
       !( CUDAEventEncoding_Is_Kernel( event->type ) && thread->stream ) )
    return;

  int block_begin = CUDAEventEncoding_Is_BlockBegin( event->value );

  if ( !thread->first_acc_event_read )
  { /*	when in stream thread first CUDA/OpenCL event  occurred,
        indicated to stop generating NOT_CREATED states in CPU	*/
    thread->first_acc_event_read = TRUE;
  }

  if( thread->captured_events->treatAccEventBehavior == t_treat_acc_events_behavior::STATES_AND_BLOCK ||
      thread->captured_events->treatAccEventBehavior == t_treat_acc_events_behavior::ALL )
  {
    struct t_cpu *cpu = get_cpu_of_thread( thread );


    if ( CUDAEventEconding_Is_CUDAStreamCreate( event ) )
    {
      struct t_thread * created_stream = thread->task->threads[ ++thread->task->totalCreatedStreams ];
      created_stream->stream_created = TRUE;
      SCHEDULER_thread_to_ready( created_stream );
    }

    /* CUDA cpu states */
    if ( !block_begin && CUDAEventEconding_Is_CUDAConfigCall( thread->acc_in_block_event ) )
    { /* If ending a Config Call event, cpu state is Others	*/
      PARAVER_Others( cpu->unique_number, IDENTIFIERS( thread ), thread->acc_in_block_event.paraver_time, current_time );
    }

    else if ( !block_begin && !thread->stream && CUDAEventEconding_Is_CUDALaunch( thread->acc_in_block_event ) )
    { /* If ending a Launch event, cpu state is Thread Scheduling	*/
      PARAVER_Thread_Sched( cpu->unique_number, IDENTIFIERS( thread ), thread->acc_in_block_event.paraver_time, current_time );
    }

    else if ( !block_begin && CUDAEventEconding_Is_CUDASync( thread->acc_in_block_event ) )
    {
      PARAVER_Thread_Sync( cpu->unique_number, IDENTIFIERS( thread ), thread->acc_in_block_event.paraver_time, current_time );
    }

    else if ( !block_begin && CUDAEventEncoding_Is_CUDATransferBlock( thread->acc_in_block_event ) )
    {
      PARAVER_Mem_Transf( cpu->unique_number, IDENTIFIERS( thread ), thread->acc_in_block_event.paraver_time, current_time );
    }
    else if ( !block_begin && CUDAEventEncoding_Is_CUDAMemset( thread->acc_in_block_event ) )
    {
      PARAVER_Mem_Transf( cpu->unique_number, IDENTIFIERS( thread ), thread->acc_in_block_event.paraver_time, current_time );
    }
    else if ( thread->stream && ( CUDAEventEncoding_Is_Kernel_Block( thread->acc_in_block_event ) ||
                                  OCLEventEncoding_Is_OCLKernelRunning( thread->acc_in_block_event ) ) )
    {
      PARAVER_Running( cpu->unique_number, IDENTIFIERS( thread ), thread->acc_in_block_event.paraver_time, current_time );
    }
    /* CUDA cpu states */

    /* OpenCL cpu states */
    else if ( !block_begin && OCLEventEncoding_Is_OCLSchedBlock( thread->acc_in_block_event ) && thread->host )
    {
      PARAVER_Thread_Sched( cpu->unique_number, IDENTIFIERS( thread ), thread->acc_in_block_event.paraver_time, current_time );
    }

    else if ( !block_begin && OCLEventEncoding_Is_OCLSyncBlock( thread->acc_in_block_event ) )
    {
      PARAVER_Thread_Sync( cpu->unique_number, IDENTIFIERS( thread ), thread->acc_in_block_event.paraver_time, current_time );
    }

    else if ( !block_begin && OCLEventEncoding_Is_OCLTransferBlock( thread->acc_in_block_event ) )
    {
      PARAVER_Mem_Transf( cpu->unique_number, IDENTIFIERS( thread ), thread->acc_in_block_event.paraver_time, current_time );
    }


    /* Update the current accelerator block	*/
    thread->acc_in_block_event.type  = event->type;
    thread->acc_in_block_event.value = event->value;
  }

  if( thread->captured_events->treatAccEventBehavior == t_treat_acc_events_behavior::PARAVER_TIME ||
      thread->captured_events->treatAccEventBehavior == t_treat_acc_events_behavior::ALL )
  {
    // if ( CUDAEventEconding_Is_CUDASync( thread->acc_in_block_event ) && thread->stream )
    // { /* Event waits when receive comm to be written	*/
    //   thread->acc_recv_sync = TRUE;
    // }
    // else
    // { /* Do not get current_time if block is going to start later */
    // }
      thread->acc_in_block_event.paraver_time = current_time;
  }
}
