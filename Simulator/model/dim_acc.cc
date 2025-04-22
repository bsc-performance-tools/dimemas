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

  extern t_boolean simulate_cuda;
}

#include "event_sync.h"

/***************************************************************
 ** treat_acc_event
 ************************
 ** if action->desc.even is an accelerator event (CUDA or OpenCL), affects
 ** to cpu (or gpu) states, communications.
 ** Updates acc_in_block_event

 ***************************************************************/
scheduler_synchronization treat_acc_event( struct t_thread *thread, struct t_even *event )
{
  if ( CUDAEventEncoding_Is_StreamSyncId_EV( event ) )
  {
    thread->task->streamid_to_synchronize = event->value-1;
  }

  if ( !CUDAEventEncoding_Is_CUDABlock( event->type ) && !OCLEventEncoding_Is_OCLBlock( event->type ) &&
       !( CUDAEventEncoding_Is_Kernel( event->type ) && thread->stream ) )
    return CONTINUE;

  int block_begin = CUDAEventEncoding_Is_BlockBegin( event->value );

  if ( !thread->first_acc_event_read )
  { /*	when in stream thread first CUDA/OpenCL event  occurred,
        indicated to stop generating NOT_CREATED states in CPU	*/
    thread->first_acc_event_read = TRUE;
  }

  auto checkSyncAndSetHostToReady = []( auto thread )
  {
    if( thread->task->gpu_requests[thread->threadid] == 1 || thread->task->gpu_requests[0] == 1 )
    {
      struct t_thread * tmpThread = thread->task->hostThreadWaiting;
      if ( tmpThread != TH_NIL )
      {
        tmpThread->event_sync_reentry = TRUE;
        tmpThread->loose_cpu = TRUE;
        thread->task->hostThreadWaiting = NULL;
        SCHEDULER_thread_to_ready( tmpThread );
      }
    }
    --thread->task->gpu_requests[thread->threadid];
    --thread->task->gpu_requests[0];
  };

  if( thread->captured_events->treatAccEventBehavior == t_treat_acc_events_behavior::STATES_AND_BLOCK ||
      thread->captured_events->treatAccEventBehavior == t_treat_acc_events_behavior::ALL )
  {
    struct t_cpu *cpu = get_cpu_of_thread( thread );

    if ( CUDAEventEconding_Is_CUDAStreamCreate( event ) && simulate_cuda == TRUE )
    {
      struct t_thread * created_stream = thread->task->threads[ ++thread->task->totalCreatedStreams ];
      created_stream->stream_created = TRUE;
      SCHEDULER_thread_to_ready( created_stream );
    }

    /* CUDA cpu states */
    if ( !block_begin && 
         ( CUDAEventEconding_Is_CUDAConfigCall( thread->acc_in_block_event ) || CUDAEventEconding_Is_CUDAStreamCreateBlock( thread->acc_in_block_event ) || CUDAEventEncoding_Is_CUDAStreamDestroy( thread->acc_in_block_event ) ) )
    { /* If ending a Config Call or Stream Create event, cpu state is Others	*/
      PARAVER_Others( cpu->unique_number, IDENTIFIERS( thread ), thread->acc_in_block_event.paraver_time, current_time );
    }

    else if ( !block_begin && !thread->stream && CUDAEventEconding_Is_CUDALaunch( thread->acc_in_block_event ) )
    { /* If ending a Launch event, cpu state is Thread Scheduling	*/
      PARAVER_Thread_Sched( cpu->unique_number, IDENTIFIERS( thread ), thread->acc_in_block_event.paraver_time, current_time );
    }

    else if ( !block_begin && !thread->stream && ( CUDAEventEncoding_Is_CUDAMalloc( thread->acc_in_block_event ) || CUDAEventEncoding_Is_CUDAFree( thread->acc_in_block_event ) ) )
    { /* If ending a Launch event, cpu state is Thread Scheduling	*/
      PARAVER_Mem_Alloc( cpu->unique_number, IDENTIFIERS( thread ), thread->acc_in_block_event.paraver_time, current_time );
    }

    else if ( !block_begin && CUDAEventEconding_Is_CUDASync( thread->acc_in_block_event ) )
    {
      if( simulate_cuda )
      {
        size_t num_cuda_calls = 0;

        if( CUDAEventEconding_Is_CUDAStreamSync( thread->acc_in_block_event ) )
          num_cuda_calls = thread->task->gpu_requests[ thread->task->streamid_to_synchronize ];
        else
          num_cuda_calls = thread->task->gpu_requests[0];
  
        if ( num_cuda_calls > 0 )
        {
          thread->task->hostThreadWaiting = thread;
          return WAIT_FOR_SYNC;
        }
      }

      PARAVER_Thread_Sync( cpu->unique_number, IDENTIFIERS( thread ), thread->acc_in_block_event.paraver_time, current_time );
    }

    else if ( !block_begin && CUDAEventEncoding_Is_CUDADeviceReset( thread->acc_in_block_event ) )
    {
      PARAVER_Thread_Sched( cpu->unique_number, IDENTIFIERS( thread ), thread->acc_in_block_event.paraver_time, current_time );
    }

    else if ( !block_begin && CUDAEventEncoding_Is_CUDATransferBlock( thread->acc_in_block_event ) )
    {
      if( simulate_cuda && !thread->host )
        checkSyncAndSetHostToReady( thread );

      PARAVER_Mem_Transf( cpu->unique_number, IDENTIFIERS( thread ), thread->acc_in_block_event.paraver_time, current_time );
    }
    else if ( !block_begin && CUDAEventEncoding_Is_CUDAMemset( thread->acc_in_block_event ) )
    {
      PARAVER_Mem_Transf( cpu->unique_number, IDENTIFIERS( thread ), thread->acc_in_block_event.paraver_time, current_time );
    }
    else if ( thread->stream && ( CUDAEventEncoding_Is_Kernel_Block( thread->acc_in_block_event ) ||
                                  OCLEventEncoding_Is_OCLKernelRunning( thread->acc_in_block_event ) ) )
    {
      if ( simulate_cuda )
        checkSyncAndSetHostToReady( thread );

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
    if( event->value == CUDA_END_VAL ) 
      thread->acc_in_block_event.type = 0;
    else
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
  return CONTINUE;
}
