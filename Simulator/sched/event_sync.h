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

#ifndef __event_sync_h
#define __event_sync_h

#include "types.h"

#define PARTNER_ID_BARRIER -1

#ifdef __cplusplus

#include <unordered_set>
#include <vector>

enum class t_treat_acc_events_behavior
{
  ALL = 0,
  STATES_AND_BLOCK,
  PARAVER_TIME
};

struct TCapturedEvents
{
  bool captureEvents;
  std::vector<struct t_even > events;
  t_treat_acc_events_behavior treatAccEventBehavior;
};

const std::unordered_set<unsigned long long>& getValidSyncTypes();

extern "C"
{
#else // __cplusplus
struct TCapturedEvents;
#endif // __cplusplus
struct TEventSyncQueue;

void event_sync_init( void );

#ifndef PRV2DIM
struct TEventSyncQueue *createEventSyncQueue();
struct TCapturedEvents *createCapturedEvents();

t_boolean event_sync_add( struct t_task *whichTask, struct t_even *whichEvent, int threadID, int partnerThreadID, t_boolean isCommCall );

t_boolean capture_previous_events( struct t_thread *whichThread,
                                   struct t_even *whichEvent,
                                   int threadID );

void print_pending_syncs( struct t_task *whichTask );

t_boolean is_openmp_treated_event( unsigned long long type );

t_boolean is_cuda_treated_event( unsigned long long type );

t_boolean is_cuda_comm ( unsigned long long commTag );

#endif // PRV2DIM

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __event_sync_h