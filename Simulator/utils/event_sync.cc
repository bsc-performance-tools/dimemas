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
#include <unordered_set>

#include "event_sync.h"

#include "define.h"
#include "EventEncoding.h"

using std::map;
using std::set;
using std::unordered_set;

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
           ( event == b.event && isHost == b.isHost ? false : isHost );
  }
};

struct EventTrait
{
  struct t_even eventHost;
  struct t_even eventRest;

  bool operator<( const EventTrait& b ) const
  {
    return eventHost < b.eventHost ||
           ( eventHost == b.eventHost && eventRest < b.eventRest );
  }
};

struct TEventSyncQueue
{
  set<EventTrait> insertedTraits;
};

#define debug  1
// extern int debug;
extern dimemas_timer current_time;

map<EventTraitIndex, EventTrait> syncEvents;
unordered_set<unsigned long long> validSyncTypes;


void event_sync_init( void )
{
  EventTraitIndex tmpEventIndex;
  EventTrait tmpEventTrait;

  validSyncTypes.insert( CUDA_LIB_CALL_EV );
  validSyncTypes.insert( OMP_BARRIER );
  validSyncTypes.insert( OMP_EXECUTED_PARALLEL_FXN );
  validSyncTypes.insert( OMP_PARALLEL_EV );

  // ------- CUDA_LIB_CALL_EV + CUDA_CONFIGURECALL_VAL -------
  tmpEventIndex.event.type = CUDA_LIB_CALL_EV;
  tmpEventIndex.event.value = CUDA_CONFIGURECALL_VAL;
  tmpEventIndex.isHost = true;

  tmpEventTrait.eventHost.type = CUDA_LIB_CALL_EV;
  tmpEventTrait.eventHost.value = CUDA_CONFIGURECALL_VAL;
  tmpEventTrait.eventRest.type = CUDA_LIB_CALL_EV;
  tmpEventTrait.eventRest.value = CUDA_CONFIGURECALL_VAL;
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
  syncEvents[ tmpEventIndex ] = tmpEventTrait;

  tmpEventIndex.event.type = OMP_EXECUTED_PARALLEL_FXN;
  tmpEventIndex.event.value = OMP_END_VAL;
  tmpEventIndex.isHost = false;
  syncEvents[ tmpEventIndex ] = tmpEventTrait;
}

struct TEventSyncQueue *createEventSyncQueue()
{
  return new struct TEventSyncQueue;
}

t_boolean event_sync_add( struct t_task *whichTask,
                          struct t_even *whichEvent,
                          int threadID,
                          int partnerThreadID,
                          t_boolean isCommCall )
{
  if( validSyncTypes.find( whichEvent->type ) == validSyncTypes.end() ||
      ( whichEvent->type == CUDA_LIB_CALL_EV && !isCommCall ) )
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

  EventTraitIndex tmpEventTraitIndex;

  tmpEventTraitIndex.event.type = whichEvent->type;
  if( whichEvent->type == OMP_EXECUTED_PARALLEL_FXN && whichEvent->value != 0 )
    tmpEventTraitIndex.event.value = OMP_BEGIN_VAL;
  tmpEventTraitIndex.isHost = ( threadID == 0 );

  map<EventTraitIndex, EventTrait>::iterator tmpItTrait = syncEvents.find( tmpEventTraitIndex );
  if( tmpItTrait == syncEvents.end() )
    return FALSE;

  EventTrait tmpEventTrait = tmpItTrait->second;
  if( whichEvent->type == OMP_EXECUTED_PARALLEL_FXN && whichEvent->value != 0 )
  {
    tmpEventTrait.eventHost.value = whichEvent->value;
    tmpEventTrait.eventRest.value = whichEvent->value;
  }

  set<EventTrait>::iterator tmpIt = whichTask->event_sync_queue->insertedTraits.find( tmpEventTrait );
  if ( tmpIt != whichTask->event_sync_queue->insertedTraits.end() )
  {
    // found: apuntarse al contador del "barrier" y comprobar si ya están todos
    if( debug )
      printf( "FOUND in task queue\n" );
  }
  else
  {
    // not found: insertar el nuevo trait y apuntarse a si mismo
    if( debug )
      printf( "NOT FOUND in task queue\n" );
  }

  // las colectivas: barrier, apertura y cierre no llevan identificador pero, los p2p si.
  // 

  return FALSE;
}
