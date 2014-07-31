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

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- *\

  $URL::                  $:  File
  $Rev::                  $:  Revision of last commit
  $Author::               $:  Author of last commit
  $Date::                 $:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include "define.h"
#include "types.h"
#include "extern.h"

#include "communic.h"
#include "cpu.h"
#include "events.h"
#include "fs.h"
#include "list.h"
#ifdef  USE_EQUEUE
#include "listE.h"
#endif
#include "memory.h"
#include "ports.h"
#include "schedule.h"
#include "subr.h"

#include "node.h"

#ifdef VENUS_ENABLED
#include "venusclient.h"
#endif

#ifdef USE_EQUEUE
Equeue          Event_queue;
Equeue          Interactive_event_queue;
#else
struct t_queue  Event_queue;
struct t_queue  Interactive_event_queue;
#endif


int             are_only_daemons = 0;

struct t_event* EVENT_timer (dimemas_timer    when,
                             int              daemon,
                             int              module,
                             struct t_thread *thread,
                             int              info)
{
  register struct t_event *event;

  if (daemon == DAEMON)
  {
    if (
#ifdef USE_EQUEUE
      (are_only_daemons == count_Equeue (&Event_queue) ) &&
#else
      (are_only_daemons == count_queue (&Event_queue) ) &&
#endif
      NEQ_0_TIMER (current_time)
    )
    {
      return (E_NIL);
    }
    are_only_daemons++;
  }

  event = (struct t_event *) malloc (sizeof (struct t_event) );

  event->event_time = when;
  event->module     = module;
  event->thread     = thread;
  event->info       = info;
  event->daemon     = daemon;

  if (
    ( module != M_CTXT_SW) &&
    ( (module != M_COM) || (info != COM_EXT_NET_TRAFFIC_TIMER) )
  )
  {
    thread->loose_cpu        = TRUE;
    thread->next_event_timer = when;
  }

#ifdef USE_EQUEUE
  insert_Eevent (&Event_queue, event);
#else
  insert_event (&Event_queue, event);
#endif

  if (debug&D_EV)
  {
    PRINT_TIMER (current_time);
    printf (": EVENT Inserted At ");
    PRINT_TIMER (when);
    printf (" Module %d", module);
    if (event->thread == TH_NIL)
    {
      printf ("\n");
    }
    else
    {
      printf (
        ". P%02d T%02d (t%02d)\n",
        IDENTIFIERS (event->thread)
      );
    }
  }

  return event;
}


void EVENT_extract_timer (int              module,
                          struct t_thread *thread,
                          dimemas_timer   *when)
{
  register struct t_event *event;
#ifdef USE_EQUEUE
  Equeue *q = &Event_queue;
#else
  struct t_queue *q = &Event_queue;
#endif

  if (thread->event != E_NIL)
  {
    event = thread->event;
    if ( (event->module == module) && (event->thread == thread) )
    {
      if (debug & D_EV)
      {
        PRINT_TIMER (current_time);
        printf (": Extracted event from module %d\n", module);
      }
#ifdef USE_EQUEUE
      extract_from_Equeue (q, (char *) event);
#else
      extract_from_queue (q, (char *) event);
#endif
      *when = event->event_time;
      free (event);
      return;
    }
  }
#ifdef USE_EQUEUE
  for (event = head_Eevent (q); event != E_NIL; event = next_Eevent (q) )
#else
  for (event = head_event (q); event != E_NIL; event = next_event (q) )
#endif
  {
    if ( (event->module == module) && (event->thread == thread) )
    {
      if (debug & D_EV)
      {
        PRINT_TIMER (current_time);
        printf (": Extracted event from module %d\n", module);
      }
#ifdef USE_EQUEUE
      extract_from_Equeue (q, (char *) event);
#else
      extract_from_queue (q, (char *) event);
#endif
      *when = event->event_time;
      free (event);
      return;
    }
  }
  panic ("Can't extract requested event for thread P%02d T%02d t%02d\n",
         IDENTIFIERS (thread) );
  return;
}

/*
 * Local routines and simulator routines
 */
void EVENT_init()
{
  if (debug & D_EV)
  {
    PRINT_TIMER (current_time);
    printf (": EVENT initial routine called\n");
  }
#ifdef USE_EQUEUE
  create_Equeue (&Event_queue);
#else
  create_queue (&Event_queue);
#endif
}

void EVENT_end()
{
  if (debug & D_EV)
  {
    PRINT_TIMER (current_time);
    printf (": EVENT end routine called\n");
  }
}

t_boolean events_for_thread (struct t_thread *thread)
{
  struct t_event *event;

#ifdef USE_EQUEUE
  for (event  = head_Eevent (&Event_queue);
       event != E_NIL;
       event  = next_Eevent (&Event_queue))
#else
  for (event  = head_event (&Event_queue);
       event != E_NIL;
       event  = next_event (&Event_queue))
#endif
  {
    if (event->thread == thread)
    {
      return TRUE;
    }
  }
  return FALSE;
}

void reload_events()
{
  register struct t_node *node;

#ifdef USE_EQUEUE
  for (node  = (struct t_node *) head_Equeue (&Node_queue);
       node != N_NIL;
       node  = (struct t_node *) next_Equeue (&Node_queue))
#else
  for (node  = (struct t_node *) head_queue (&Node_queue);
       node != N_NIL;
       node  = (struct t_node *) next_queue (&Node_queue))
#endif
  {
    while ( (count_queue (& (node->ready) ) != 0) && (num_free_cpu (node) > 0) )
    {
      SCHEDULER_next_thread_to_run (node);
    }
  }
}

/*
 * Main event managament routine
 */
void event_manager (struct t_event *event)
{
  if GT_TIMER (current_time, event->event_time)
  {
    die ("Time is going backwards [Previous: %le New: %le]\n",
         current_time,
         event->event_time);
  }

  if (debug & D_EV)
  {
    PRINT_TIMER (current_time);
    printf (": EVENT Selected ");
    /* PRINT_TIMER (event->event_time); */
    printf ("Module %d ", event->module);
    if (event->thread == TH_NIL)
    {
      printf ("\n");
    }
    else
    {
      printf (
        ": P%02d T%02d (t%02d)\n",
        IDENTIFIERS (event->thread)
      );
    }
  }

  ASS_ALL_TIMER (current_time, event->event_time);

  if (OUT_OF_LIMIT (current_time) )
  {
    /* Avoid underflow problem */
    panic ("Time out of simulation limit\n");
  }

  if (event->daemon == DAEMON)
  {
    are_only_daemons--;
  }

//   printf("to check which type of event it is\n");
  switch (event->module)
  {
  case M_SCH:
    SCHEDULER_general (SCH_TIMER_OUT, event->thread);
    break;
  case M_COM:
    COMMUNIC_general (event->info, event->thread);
    break;
  case M_FS:
    FS_general (event->info, event->thread);
    break;
  case M_PORT:
    PORT_general (PORT_TIMER_OUT, event->thread);
    break;
  case M_MEM:
    MEMORY_general (MEMORY_TIMER_OUT, event->thread);
    break;
  case M_CTXT_SW:
    SCHEDULER_info (CONTEXT_SWITCH_TIMER_OUT, event->info, TH_NIL, TH_NIL);
    break;
  default:
    panic ("Invalid event type on queue (%d)\n", event->module);
    break;
  }

  if (reload_done)
  {
    reload_events();
    reload_done = FALSE;
  }

  free (event);
}
