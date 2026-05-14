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

extern "C"
{
#include "dim_acc.h"

#include "EventEncoding.h"
#include "define.h"
#include "paraver.h"
#include "schedule.h"
#include "types.h"

  extern t_boolean simulate_cuda;
}

#include "event_sync.h"

#include <algorithm>
#include <map>
#include <vector>

using std::map;
using std::vector;

struct TCUDAEventInfo
{
  size_t gpu_requests;

  std::vector<struct t_thread *> streams_waiting;
};

struct TCUDAEventID_Info
{
  map<int, struct TCUDAEventInfo> eventID_info;

  map<int, vector<int> > eventIDs_per_stream;

  std::map<int, int> streams_called_for_wait_eventID;
};


struct TCUDAEventID_Info *createCUDAEventID_StreamID()
{
  return new struct TCUDAEventID_Info;
}

void insertCUDAEventID_info( struct TCUDAEventID_Info *whichMap, int eventID, int streamID, size_t gpu_requests )
{
  whichMap->eventID_info[ eventID ].gpu_requests = gpu_requests;

  auto eventIt = std::find( whichMap->eventIDs_per_stream[ streamID ].begin(), whichMap->eventIDs_per_stream[ streamID ].end(), eventID );
  if ( eventIt == whichMap->eventIDs_per_stream[ streamID ].end() )
    whichMap->eventIDs_per_stream[ streamID ].push_back( eventID );
}

size_t getGPURequests_from_CUDAEventID( struct TCUDAEventID_Info *whichMap, int eventID )
{
  return whichMap->eventID_info[ eventID ].gpu_requests;
}

size_t substract_GPURequests_from_CUDAEventID( struct TCUDAEventID_Info *whichMap, int eventID )
{
  --whichMap->eventID_info[ eventID ].gpu_requests;
  return whichMap->eventID_info[ eventID ].gpu_requests;
}

void remove_EventID_info( struct TCUDAEventID_Info *whichMap, int eventID )
{
  whichMap->eventID_info.erase( eventID );
}

void remove_EventID_forStream( struct TCUDAEventID_Info *whichMap, int streamID, std::vector<int>::iterator whichEvent )
{
  whichMap->eventIDs_per_stream[ streamID ].erase( whichEvent );
}

void insert_stream_called_for_wait_eventID( struct TCUDAEventID_Info *whichMap, int eventID, int streamID )
{
  whichMap->streams_called_for_wait_eventID[ streamID ] = eventID;
}

std::pair<bool, int> is_stream_called_for_wait_eventID( struct TCUDAEventID_Info *whichMap, int streamID )
{
  auto it = whichMap->streams_called_for_wait_eventID.find( streamID );
  if ( it == whichMap->streams_called_for_wait_eventID.end() )
    return { false, 0 };

  return { true, it->second };
}

void remove_stream_called_for_wait_eventID( struct TCUDAEventID_Info *whichMap, int streamID )
{
  whichMap->streams_called_for_wait_eventID.erase( streamID );
}

void insert_stream_waiting_for_eventID( struct TCUDAEventID_Info *whichMap, int eventID, struct t_thread *stream )
{
  whichMap->eventID_info[ eventID ].streams_waiting.push_back( stream );
}


void checkSyncAndSetHostToReady( struct t_thread *thread )
{
  struct t_thread *tmpHostThread = thread->task->hostThreadWaiting;
  auto eventInfoMap              = thread->task->eventID_To_Info;
  auto streamID                  = thread->threadid;
  bool putHostToReady            = false;

  auto do_put_thread_to_ready = [ & ]( struct t_thread *hostThread )
  {
    hostThread->event_sync_reentry  = TRUE;
    hostThread->loose_cpu           = TRUE;
    thread->task->hostThreadWaiting = TH_NIL;
    SCHEDULER_thread_to_ready( hostThread );
  };

  // cudaEvent synchronization
  std::vector<std::vector<int>::iterator> eventsToErase;
  for ( auto itEvent = eventInfoMap->eventIDs_per_stream[ streamID ].begin(); itEvent != eventInfoMap->eventIDs_per_stream[ streamID ].end();
        ++itEvent )
  {
    int current_gpu_requests = substract_GPURequests_from_CUDAEventID( eventInfoMap, *itEvent );
    if ( current_gpu_requests == 0 )
    {
      if ( tmpHostThread != TH_NIL && CUDAEventEncoding_Is_CUDAEventSyncBlock( tmpHostThread->acc_in_block_event ) &&
           thread->task->lastEventID == *itEvent )
        putHostToReady = true;

      for ( auto &th : eventInfoMap->eventID_info[ *itEvent ].streams_waiting )
        do_put_thread_to_ready( th );

      remove_EventID_info( thread->task->eventID_To_Info, *itEvent );
      eventsToErase.push_back( itEvent );
    }
  }

  for ( auto &el : eventsToErase )
    remove_EventID_forStream( eventInfoMap, streamID, el );

  // cudaStream/cudaDevice synchronization
  if ( thread->task->gpu_requests[ thread->threadid ] == 1 || thread->task->gpu_requests[ 0 ] == 1 )
  {
    if ( tmpHostThread != TH_NIL )
      putHostToReady = true;
  }

  if ( putHostToReady )
    do_put_thread_to_ready( tmpHostThread );

  --thread->task->gpu_requests[ thread->threadid ];
  --thread->task->gpu_requests[ 0 ];
}

/***************************************************************
 ** treat_acc_event
 ************************
 ** if action->desc.even is an accelerator event (CUDA or OpenCL), affects
 ** to cpu (or gpu) states, communications.
 ** Updates acc_in_block_event

 ***************************************************************/
scheduler_synchronization treat_acc_event( struct t_thread *thread, struct t_even *event )
{
  if ( CUDAEventEncoding_Is_CudaEventID( event ) )
  {
    thread->task->lastEventID = event->value;
  }

  if ( CUDAEventEncoding_Is_StreamSyncId_EV( event ) )
  {
    if ( CUDAEventEncoding_Is_CUDASync( thread->acc_in_block_event ) )
      thread->task->streamid_to_synchronize = event->value - 1;
    else if ( CUDAEventEncoding_Is_CUDAEventRecordBlock( thread->acc_in_block_event ) )
      insertCUDAEventID_info( thread->task->eventID_To_Info,
                              thread->task->lastEventID,
                              event->value - 1,
                              thread->task->gpu_requests[ event->value - 1 ] );
    else if ( CUDAEventEncoding_Is_CUDAStreamWaitEventBlock( thread->acc_in_block_event ) )
      insert_stream_called_for_wait_eventID( thread->task->eventID_To_Info, thread->task->lastEventID, event->value - 1 );
  }

  if ( !CUDAEventEncoding_Is_CUDABlock( event->type ) && !OCLEventEncoding_Is_OCLBlock( event->type ) &&
       !( CUDAEventEncoding_Is_Kernel( event->type ) && thread->stream ) )
    return CONTINUE;

  int block_begin = CUDAEventEncoding_Is_BlockBegin( event->value );

  if ( thread->captured_events->treatAccEventBehavior == t_treat_acc_events_behavior::STATES_AND_BLOCK ||
       thread->captured_events->treatAccEventBehavior == t_treat_acc_events_behavior::ALL )
  {
    struct t_cpu *cpu = get_cpu_of_thread( thread );

    /* CUDA cpu states */
    if ( !block_begin && ( CUDAEventEncoding_Is_CUDAConfigCall( thread->acc_in_block_event ) ||
                           CUDAEventEncoding_Is_CUDAStreamCreateBlock( thread->acc_in_block_event ) ||
                           CUDAEventEncoding_Is_CUDAStreamDestroy( thread->acc_in_block_event ) ) )
    { /* If ending a Config Call or Stream Create event, cpu state is Others	*/
      PARAVER_Others( cpu->unique_number, IDENTIFIERS( thread ), thread->acc_in_block_event.paraver_time, current_time );
    }

    else if ( !block_begin && !thread->stream && CUDAEventEncoding_Is_CUDALaunch( thread->acc_in_block_event ) )
    { /* If ending a Launch event, cpu state is Thread Scheduling	*/
      PARAVER_Thread_Sched( cpu->unique_number, IDENTIFIERS( thread ), thread->acc_in_block_event.paraver_time, current_time );
    }

    else if ( !block_begin && !thread->stream &&
              ( CUDAEventEncoding_Is_CUDAMalloc( thread->acc_in_block_event ) || CUDAEventEncoding_Is_CUDAFree( thread->acc_in_block_event ) ) )
    { /* If ending a Launch event, cpu state is Thread Scheduling	*/
      PARAVER_Mem_Alloc( cpu->unique_number, IDENTIFIERS( thread ), thread->acc_in_block_event.paraver_time, current_time );
    }

    else if ( !block_begin && CUDAEventEncoding_Is_CUDASync( thread->acc_in_block_event ) )
    {
      if ( simulate_cuda )
      {
        size_t num_cuda_calls = 0;

        if ( CUDAEventEncoding_Is_CUDAStreamSync( thread->acc_in_block_event ) )
          num_cuda_calls = thread->task->gpu_requests[ thread->task->streamid_to_synchronize ];
        else
          num_cuda_calls = thread->task->gpu_requests[ 0 ];

        if ( num_cuda_calls > 0 )
        {
          thread->task->hostThreadWaiting = thread;
          return WAIT_FOR_SYNC;
        }
      }

      PARAVER_Thread_Sync( cpu->unique_number, IDENTIFIERS( thread ), thread->acc_in_block_event.paraver_time, current_time );
    }

    else if ( !block_begin && CUDAEventEncoding_Is_CUDAEventSyncBlock( thread->acc_in_block_event ) )
    {
      if ( simulate_cuda )
      {
        if ( getGPURequests_from_CUDAEventID( thread->task->eventID_To_Info, thread->task->lastEventID ) > 0 )
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
      PARAVER_Mem_Transf( cpu->unique_number, IDENTIFIERS( thread ), thread->acc_in_block_event.paraver_time, current_time );
    }
    else if ( !block_begin && CUDAEventEncoding_Is_CUDAMemset( thread->acc_in_block_event ) )
    {
      PARAVER_Mem_Transf( cpu->unique_number, IDENTIFIERS( thread ), thread->acc_in_block_event.paraver_time, current_time );
    }
    else if ( !block_begin && CUDAEventEncoding_Is_CUDAEventRecordBlock( thread->acc_in_block_event ) )
    {
      PARAVER_Others( cpu->unique_number, IDENTIFIERS( thread ), thread->acc_in_block_event.paraver_time, current_time );
    }
    else if ( thread->stream && ( CUDAEventEncoding_Is_Kernel_Block( thread->acc_in_block_event ) ||
                                  OCLEventEncoding_Is_OCLKernelRunning( thread->acc_in_block_event ) ) )
    {
      if ( simulate_cuda )
        checkSyncAndSetHostToReady( thread );

      PARAVER_Running( cpu->unique_number, IDENTIFIERS( thread ), thread->acc_in_block_event.paraver_time, current_time );
    }
    else if ( thread->stream && block_begin && CUDAEventEncoding_Is_Kernel( event->type ) )
    {
      auto stream_for_wait_eventID = is_stream_called_for_wait_eventID( thread->task->eventID_To_Info, thread->threadid );
      if ( stream_for_wait_eventID.first )
      {
        remove_stream_called_for_wait_eventID( thread->task->eventID_To_Info, thread->threadid );
        if ( getGPURequests_from_CUDAEventID( thread->task->eventID_To_Info, stream_for_wait_eventID.second ) > 0 )
        {
          insert_stream_waiting_for_eventID( thread->task->eventID_To_Info, stream_for_wait_eventID.second, thread );
          return WAIT_FOR_SYNC;
        }
      }
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
    if ( event->value == CUDA_END_VAL )
      thread->acc_in_block_event.type = 0;
    else
      thread->acc_in_block_event.type = event->type;

    thread->acc_in_block_event.value = event->value;
  }

  if ( thread->captured_events->treatAccEventBehavior == t_treat_acc_events_behavior::PARAVER_TIME ||
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
