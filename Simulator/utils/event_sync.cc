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
#include <set>

#include "event_sync.h"

#include "define.h"
#include "EventEncoding.h"

using std::set;

#define debug  1
// extern int debug;
extern dimemas_timer current_time;

struct EventTrait
{
  struct t_even eventHost;
  struct t_even eventRest;
  bool checkEventValueNotZero;
};

auto compareEvent = []( const struct EventTrait& a, const struct EventTrait& b )
                    { 
                      if ( b.checkEventValueNotZero ) // a??
                      {
                        // Host || Rest
                        return ( a.eventRest.type == b.eventRest.type ? a.eventRest.value < b.eventRest.value : a.eventRest.type < b.eventRest.type ) ||
                              ( a.eventHost.type == b.eventHost.type ? a.eventHost.value < b.eventHost.value : a.eventHost.type < b.eventHost.type );
                      }
                      else
                      {
                        // Host || rest
                        return ( a.eventRest.type == b.eventRest.type ? a.eventRest.value > 0 && b.eventRest.value > 0:
                                                                        a.eventRest.type < b.eventRest.type );

                      }
                    };
                      
                      
                     // return a.type == b.type ? a.value < b.value : a.type < b.type; };



set<EventTrait, decltype( compareEvent ) > syncEvents( compareEvent );

void event_sync_init( void )
{
  EventTrait tmpEvent;


  tmpEvent.eventHost.type = CUDA_LIB_CALL_EV;
  tmpEvent.eventHost.value = CUDA_CONFIGURECALL_VAL;
  tmpEvent.eventRest.type = CUDA_LIB_CALL_EV;
  tmpEvent.eventRest.value = CUDA_CONFIGURECALL_VAL;
  tmpEvent.checkEventValueNotZero = true;
  syncEvents.insert( tmpEvent );

  tmpEvent.eventHost.type = CUDA_LIB_CALL_EV;
  tmpEvent.eventHost.value = CUDA_LAUNCH_VAL;
  tmpEvent.eventRest.type = CUDA_LIB_CALL_EV;
  tmpEvent.eventRest.value = CUDA_LAUNCH_VAL;
  tmpEvent.checkEventValueNotZero = true;
  syncEvents.insert( tmpEvent );

  tmpEvent.eventHost.type = OMP_BARRIER;
  tmpEvent.eventHost.value = OMP_END_VAL;
  tmpEvent.eventRest.type = OMP_BARRIER;
  tmpEvent.eventRest.value = OMP_END_VAL;
  tmpEvent.checkEventValueNotZero = true;
  syncEvents.insert( tmpEvent );

  tmpEvent.eventHost.type = OMP_EXECUTED_PARALLEL_FXN;
  tmpEvent.eventHost.value = OMP_BEGIN_VAL;
  tmpEvent.eventRest.type = OMP_EXECUTED_PARALLEL_FXN;
  tmpEvent.eventRest.value = OMP_BEGIN_VAL;
  tmpEvent.checkEventValueNotZero = false;
  syncEvents.insert( tmpEvent );

  tmpEvent.eventHost.type = OMP_PARALLEL_EV;
  tmpEvent.eventHost.value = OMP_END_VAL;
  tmpEvent.eventRest.type = OMP_EXECUTED_PARALLEL_FXN;
  tmpEvent.eventRest.value = OMP_END_VAL;
  tmpEvent.checkEventValueNotZero = true;
  syncEvents.insert( tmpEvent );
}

t_boolean event_sync_add( struct t_task *whichTask, 
                          struct t_even *whichEvent,
                          int threadID,
                          int partnerThreadID,
                          t_boolean isCommCall )
{
  EventTrait tmpEvent;

  if (threadID == 0)
  {
    tmpEvent.eventHost.type = whichEvent->type;
    tmpEvent.eventHost.value = whichEvent->value;
  }
  else
  {
    tmpEvent.eventRest.type = whichEvent->type;
    tmpEvent.eventRest.value = whichEvent->value;
  }

  if( syncEvents.find( tmpEvent ) == syncEvents.end() )
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

  return FALSE;
}
