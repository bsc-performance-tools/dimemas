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
 \****************************************************************************/

#include <stdio.h>
#include <cmath>
#include <map>
#include <set>
#include <tuple>

#include "event_sync.h"

extern "C" {
  #include "EventEncoding.h"
#ifndef PRV2DIM
  #include "cpu.h"
  #include "schedule.h"
  #include "define.h"
  #include "events.h"
  #include "extern.h"
  #include "list.h"
  #include "node.h"
  #include "read.h"
#endif // PRV2DIM
}

using std::map;
using std::set;
using std::unordered_set;
using std::vector;

#ifndef PRV2DIM

bool operator<( const struct t_even& a, const struct t_even& b )
{
  return a.type < b.type ||
         ( a.type == b.type && a.value < b.value );
}

bool operator==( const struct t_even& a, const struct t_even& b )
{
  return a.type == b.type && a.value == b.value;
}

struct EventTraitIndex
{
  bool isHost;
  struct t_even event;

  bool operator<( const EventTraitIndex& b ) const
  {
    return event < b.event ||
           ( event == b.event && isHost < b.isHost );
  }
};

struct EventTrait
{
  struct t_even eventHost;
  struct t_even eventRest;

  int partnerThreadID;

  bool restThreadsCanResume;
  bool capturePreviousEvents;
  t_boolean rewriteLogicalReceive;

  int numParticipants;
  mutable int numArrived;

  bool operator<( const EventTrait& b ) const
  {
    return std::tie( eventHost, eventRest, partnerThreadID ) < std::tie( b.eventHost, b.eventRest, b.partnerThreadID );
  }
};

struct TEventSyncQueue
{
  set<EventTrait> insertedTraits;
};

extern int debug;
extern dimemas_timer current_time;

map<EventTraitIndex, EventTrait> syncEvents;

#endif // PRV2DIM

unordered_set<unsigned long long> validSyncTypes;

const unordered_set<unsigned long long>& getValidSyncTypes()
{
  return validSyncTypes;
}

void event_sync_init( void )
{
#ifndef PRV2DIM
  EventTraitIndex tmpEventIndex;
  EventTrait tmpEventTrait;
#endif // PRV2DIM

  validSyncTypes.insert( CUDA_LIB_CALL_EV );
  validSyncTypes.insert( NEW_CUDA_LIB_CALL_EV );
  validSyncTypes.insert( OMP_BARRIER );
  validSyncTypes.insert( OMP_EXECUTED_PARALLEL_FXN );
  validSyncTypes.insert( OMP_PARALLEL_EV );

#ifndef PRV2DIM
  // ------- CUDA_LIB_CALL_EV + CUDA_CONFIGURECALL_VAL -------
  tmpEventIndex.event.type = CUDA_LIB_CALL_EV;
  tmpEventIndex.event.value = CUDA_CONFIGURECALL_VAL;
  tmpEventIndex.isHost = true;

  tmpEventTrait.eventHost.type = CUDA_LIB_CALL_EV;
  tmpEventTrait.eventHost.value = CUDA_CONFIGURECALL_VAL;
  tmpEventTrait.eventRest.type = CUDA_LIB_CALL_EV;
  tmpEventTrait.eventRest.value = CUDA_CONFIGURECALL_VAL;
  tmpEventTrait.restThreadsCanResume = false;
  tmpEventTrait.capturePreviousEvents = true;
  tmpEventTrait.rewriteLogicalReceive = TRUE;
  syncEvents[ tmpEventIndex ] = tmpEventTrait;

  tmpEventIndex.isHost = false;
  syncEvents[ tmpEventIndex ] = tmpEventTrait;


  // NEW_CUDA_LIB_CALL_EV
  tmpEventIndex.event.type = NEW_CUDA_LIB_CALL_EV;
  tmpEventIndex.isHost = true;

  tmpEventTrait.eventHost.type = NEW_CUDA_LIB_CALL_EV;
  tmpEventTrait.eventRest.type = NEW_CUDA_LIB_CALL_EV;
  syncEvents[ tmpEventIndex ] = tmpEventTrait;

  tmpEventIndex.isHost = false;
  syncEvents[ tmpEventIndex ] = tmpEventTrait;

  // ------- CUDA_LIB_CALL_EV + CUDA_LAUNCH_VAL -------
  tmpEventIndex.event.type = CUDA_LIB_CALL_EV;
  tmpEventIndex.event.value = CUDA_LAUNCH_VAL;
  tmpEventIndex.isHost = true;

  tmpEventTrait.eventHost.type = CUDA_LIB_CALL_EV;
  tmpEventTrait.eventHost.value = CUDA_LAUNCH_VAL;
  tmpEventTrait.eventRest.type = CUDA_LIB_CALL_EV;
  tmpEventTrait.eventRest.value = CUDA_LAUNCH_VAL;
  tmpEventTrait.restThreadsCanResume = false;
  tmpEventTrait.capturePreviousEvents = true;
  tmpEventTrait.rewriteLogicalReceive = TRUE;
  syncEvents[ tmpEventIndex ] = tmpEventTrait;

  tmpEventIndex.isHost = false;
  syncEvents[ tmpEventIndex ] = tmpEventTrait;

  // NEW_CUDA_LIB_CALL_EV
  tmpEventIndex.event.type = NEW_CUDA_LIB_CALL_EV;
  tmpEventIndex.isHost = true;

  tmpEventTrait.eventHost.type = NEW_CUDA_LIB_CALL_EV;
  tmpEventTrait.eventRest.type = NEW_CUDA_LIB_CALL_EV;
  syncEvents[ tmpEventIndex ] = tmpEventTrait;

  tmpEventIndex.isHost = false;
  syncEvents[ tmpEventIndex ] = tmpEventTrait;
  

  // ------- OMP_BARRIER + OMP_END_VAL -------
  tmpEventIndex.event.type = OMP_BARRIER;
  tmpEventIndex.event.value = OMP_END_VAL;
  tmpEventIndex.isHost = true;
  
  tmpEventTrait.eventHost.type = OMP_BARRIER;
  tmpEventTrait.eventHost.value = OMP_END_VAL;
  tmpEventTrait.eventRest.type = OMP_BARRIER;
  tmpEventTrait.eventRest.value = OMP_END_VAL;
  tmpEventTrait.restThreadsCanResume = false;
  tmpEventTrait.capturePreviousEvents = false;
  tmpEventTrait.rewriteLogicalReceive = FALSE;
  syncEvents[ tmpEventIndex ] = tmpEventTrait;

  tmpEventIndex.isHost = false;
  syncEvents[ tmpEventIndex ] = tmpEventTrait;

  // ------- OMP_EXECUTED_PARALLEL_FXN + OMP_BEGIN_VAL -------
  tmpEventIndex.event.type = OMP_EXECUTED_PARALLEL_FXN;
  tmpEventIndex.event.value = OMP_BEGIN_VAL;
  tmpEventIndex.isHost = true;

  tmpEventTrait.eventHost.type = OMP_EXECUTED_PARALLEL_FXN;
  tmpEventTrait.eventHost.value = OMP_BEGIN_VAL;
  tmpEventTrait.eventRest.type = OMP_EXECUTED_PARALLEL_FXN;
  tmpEventTrait.eventRest.value = OMP_BEGIN_VAL;
  tmpEventTrait.restThreadsCanResume = false;
  tmpEventTrait.capturePreviousEvents = false;
  tmpEventTrait.rewriteLogicalReceive = FALSE;
  syncEvents[ tmpEventIndex ] = tmpEventTrait;

  tmpEventIndex.isHost = false;
  syncEvents[ tmpEventIndex ] = tmpEventTrait;
  
  // ------- OMP_PARALLEL_EV + OMP_END_VAL -------
  tmpEventIndex.event.type = OMP_PARALLEL_EV;
  tmpEventIndex.event.value = OMP_END_VAL;
  tmpEventIndex.isHost = true;
  
  tmpEventTrait.eventHost.type = OMP_PARALLEL_EV;
  tmpEventTrait.eventHost.value = OMP_END_VAL;
  tmpEventTrait.eventRest.type = OMP_EXECUTED_PARALLEL_FXN;
  tmpEventTrait.eventRest.value = OMP_END_VAL;
  tmpEventTrait.restThreadsCanResume = true;
  tmpEventTrait.capturePreviousEvents = false;
  tmpEventTrait.rewriteLogicalReceive = FALSE;
  syncEvents[ tmpEventIndex ] = tmpEventTrait;

  tmpEventIndex.event.type = OMP_EXECUTED_PARALLEL_FXN;
  tmpEventIndex.event.value = OMP_END_VAL;
  tmpEventIndex.isHost = false;
  syncEvents[ tmpEventIndex ] = tmpEventTrait;
#endif // PRV2DIM
}

#ifndef PRV2DIM

map<EventTraitIndex, EventTrait>::iterator find_event_trait(struct t_even *whichEvent, int threadID )
{
  EventTraitIndex tmpEventTraitIndex;

  tmpEventTraitIndex.event.type = whichEvent->type;
  tmpEventTraitIndex.event.value = whichEvent->value;
  if( whichEvent->type == OMP_EXECUTED_PARALLEL_FXN && whichEvent->value != 0 )
    tmpEventTraitIndex.event.value = OMP_BEGIN_VAL;
  tmpEventTraitIndex.isHost = ( threadID == 0 );

  return syncEvents.find( tmpEventTraitIndex );
}

struct TEventSyncQueue *createEventSyncQueue()
{
  return new struct TEventSyncQueue;
}

struct TCapturedEvents *createCapturedEvents()
{
  TCapturedEvents *tmp = new struct TCapturedEvents;
  tmp->captureEvents = false;
  tmp->treatAccEventBehavior = t_treat_acc_events_behavior::ALL;

  return tmp;
}


t_boolean capture_previous_events( struct t_thread *whichThread,
                                   struct t_even *whichEvent,
                                   int threadID )
{
  if( whichThread->host )
    return FALSE;

  if( !whichThread->captured_events->captureEvents )
  {
    auto it = find_event_trait( whichEvent, threadID );
    if( it == syncEvents.end() || !it->second.capturePreviousEvents )
      return FALSE;

    if( debug )
      printf( "capture_previous_events-->after RET false\n");
  
    whichThread->captured_events->treatAccEventBehavior = t_treat_acc_events_behavior::STATES_AND_BLOCK;
    treat_acc_event( whichThread, whichEvent );

    whichThread->captured_events->captureEvents = true;
  }

  if( debug )
  {
    printf("\t event sync capture events:tid %d event type %llu val %llu\n", threadID, whichEvent->type, whichEvent->value);
  }
  
  whichThread->captured_events->events.push_back( *whichEvent );

  return TRUE;
}

void resumeCapturedEvents( struct t_thread *thread )
{
  thread->captured_events->captureEvents = false;
  if( !thread->host )
    thread->captured_events->treatAccEventBehavior = t_treat_acc_events_behavior::PARAVER_TIME;
  for( auto it : thread->captured_events->events )
  {
    scheduler_treat_event( thread, &it );
  }
  if( !thread->host )
    thread->captured_events->treatAccEventBehavior = t_treat_acc_events_behavior::ALL;
  thread->captured_events->events.clear();
}

t_boolean event_sync_add( struct t_task *whichTask,
                          struct t_even *whichEvent,
                          int threadID,
                          int partnerThreadID,
                          t_boolean isCommCall )
{
  if( whichTask->threads[ threadID ]->event_sync_reentry )
  {
    whichTask->threads[ threadID ]->event_sync_reentry = FALSE;
    return FALSE;
  }

  // This piece of code fixes a possible extrae bug (types.h, task.c and event_sync.cc):
  //   nested parallel function calls after worksharing single
  if( whichEvent->type == OMP_EXECUTED_PARALLEL_FXN && whichEvent->value != OMP_END_VAL )
  {
    whichTask->threads[ threadID ]->omp_nesting_level++;
    if( whichTask->threads[ threadID ]->omp_nesting_level > 1 )
      return FALSE;
  }
  else if( whichEvent->type == OMP_EXECUTED_PARALLEL_FXN && whichEvent->value == OMP_END_VAL )
  {
    whichTask->threads[ threadID ]->omp_nesting_level--;
    if( whichTask->threads[ threadID ]->omp_nesting_level > 0 )
      return FALSE;
  }

  if( validSyncTypes.find( whichEvent->type ) == validSyncTypes.end() ||
      ( CUDAEventEncoding_Is_CUDABlock( whichEvent->type ) && !isCommCall ) )
    return FALSE;

  if ( debug )
  {
    PRINT_TIMER( current_time );
    printf( ": event sync add taskid %d, event type %lld value %lld, threadid %d, partnerid %d, isCommCall %d \n",
            whichTask->taskid,
            whichEvent->type,
            whichEvent->value,
            threadID,
            partnerThreadID,
            isCommCall );
  }

  auto tmpItTrait = find_event_trait( whichEvent, threadID );
  if ( tmpItTrait == syncEvents.end() )
    return FALSE;

  if( debug )
    printf( "\tevent sync add: EventTrait found.\n" );

  EventTrait tmpEventTrait = tmpItTrait->second;
  if( whichEvent->type == OMP_EXECUTED_PARALLEL_FXN && whichEvent->value != OMP_END_VAL )
  {
    tmpEventTrait.eventHost.value = whichEvent->value;
    tmpEventTrait.eventRest.value = whichEvent->value;
  }
  tmpEventTrait.partnerThreadID = partnerThreadID;

  set<EventTrait>::iterator tmpIt = whichTask->event_sync_queue->insertedTraits.find( tmpEventTrait );
  if ( tmpIt == whichTask->event_sync_queue->insertedTraits.end() )
  {
    if( partnerThreadID == PARTNER_ID_BARRIER )
      tmpEventTrait.numParticipants = whichTask->threads_count;
    else
      tmpEventTrait.numParticipants = 2;

    tmpEventTrait.numArrived = 0;

    std::tie( tmpIt, std::ignore ) = whichTask->event_sync_queue->insertedTraits.insert( tmpEventTrait );
  }

  ++tmpIt->numArrived;
  if( debug )
    printf( "\tevent sync add: num threads arrived to sync %d of %d participants.\n", tmpIt->numArrived, tmpIt->numParticipants );

  if( tmpIt->numArrived == tmpIt->numParticipants )
  {
    if( debug )
      printf( "\tevent sync add: all participants have arrived. Resume them.\n" );

    for( int iThread = 0; iThread < tmpIt->numParticipants; ++iThread )
    {
      if( iThread > 0 && tmpIt->restThreadsCanResume )
        break;

      int realThread = iThread;
      if( tmpIt->partnerThreadID != PARTNER_ID_BARRIER && iThread > 0 )
        realThread = tmpIt->partnerThreadID;

      struct t_thread * tmpThread = whichTask->threads[ realThread ];

      resumeCapturedEvents( tmpThread );
      if( tmpEventTrait.rewriteLogicalReceive )
      {
        tmpThread->logical_recv = current_time;
      }

      tmpThread->event_sync_reentry = TRUE;
      tmpThread->loose_cpu = TRUE;
      SCHEDULER_thread_to_ready( tmpThread );
      reload_done = TRUE;
    }

    whichTask->event_sync_queue->insertedTraits.erase( tmpIt );
  }

  if( threadID != 0 && tmpIt->restThreadsCanResume )
  {
    return FALSE;
  }

  return TRUE;
}

void print_pending_syncs( struct t_task *whichTask )
{
  for ( set<EventTrait>::iterator tmpIt = whichTask->event_sync_queue->insertedTraits.begin();
        tmpIt != whichTask->event_sync_queue->insertedTraits.end(); ++tmpIt )
  {
    if ( tmpIt->partnerThreadID == PARTNER_ID_BARRIER )
    {
      int num_remaining = tmpIt->numParticipants- tmpIt->numArrived;
      printf( "[Task %d] blocked on event %llu:%llu waiting for %d threads to arrive\n", 
              whichTask->taskid, tmpIt->eventHost.type, tmpIt->eventHost.value, num_remaining );
    }
    else
    {
      printf( "[Task %d] blocked on event %llu:%llu waiting for thread %d to arrive\n", 
              whichTask->taskid, tmpIt->eventHost.type, tmpIt->eventHost.value, tmpIt->partnerThreadID );
    }
  }
}


#endif // PRV2DIM
