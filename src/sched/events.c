char events_c_rcsid[]="$Id: events.c,v 1.7 2009/02/18 10:27:43 xaguilar Exp $";
/*
 * Event routines
 * 
 * Sergi Girona (sergi@ac.upc.es)
 * 
 * (c) DAC-UPC 1993-94
 * 
 */


#include "define.h"
#include "types.h"
#include "extern.h"

#include "communic.h"
#include "cpu.h"
#include "events.h"
#include "fs.h"
#include "list.h"
#include "mallocame.h"
#include "memory.h"
#include "ports.h"
#include "schedule.h"
#include "subr.h"

#ifdef VENUS_CLIENT
#include "venusclient.h"
#endif

struct t_queue  Event_queue;
struct t_queue  Interactive_event_queue;

int             are_only_daemons = 0;

struct t_event*
EVENT_timer(
  dimemas_timer    when,
  int              daemon,
  int              module,
  struct t_thread *thread,
  int              info
)
{
  register struct t_event *event;

  if (daemon == DAEMON)
  {
    if ( 
      (are_only_daemons == count_queue (&Event_queue)) && 
       NEQ_0_TIMER (current_time)
    )
    {
      return(E_NIL);
    }
    are_only_daemons++;
  }
 
  event = (struct t_event *) mallocame (sizeof (struct t_event));
  
  event->event_time = when;
  event->module     = module;
  event->thread     = thread;
  event->info       = info;
  event->daemon     = daemon;
  
  if (
    ( module != M_CTXT_SW) &&
    ((module != M_COM) || (info != COM_EXT_NET_TRAFFIC_TIMER))
  )
  {
    thread->loose_cpu        = TRUE;
    thread->next_event_timer = when;
  }
  
  insert_event (&Event_queue, event);
  
  if (debug&D_EV)
  {
    PRINT_TIMER (current_time);
    printf (": EVENT Inserted At ");
    PRINT_TIMER (when);
    printf (" Module %d", module);
    if (event->thread == TH_NIL)
    {
      printf("\n");
    }
    else
    {
      printf(
        ". P%02d T%02d (t%02d)\n",
        IDENTIFIERS(event->thread)
      );
    }
  }
  
  return event;
}


void
EVENT_extract_timer(int module, struct t_thread *thread, dimemas_timer *when)
{
   register struct t_event *event;
   struct t_queue *q = &Event_queue;

   if (thread->event!=E_NIL)
   {
       event = thread->event;
       if ((event->module == module) && (event->thread == thread))
       {
           if (debug&D_EV)
           {
               PRINT_TIMER (current_time);
               printf (": Extracted event from module %d\n", module);
           }
           extract_from_queue (q, (char *) event);
           *when = event->event_time;
           freeame ((char *) event, sizeof (struct t_event));
           return;
       }
   }
   for (event = head_event (q); event != E_NIL; event = next_event (q))
   {
      if ((event->module == module) && (event->thread == thread))
      {
	 if (debug&D_EV)
	 {
	    PRINT_TIMER (current_time);
	    printf (": Extracted event from module %d\n", module);
	 }
	 extract_from_queue (q, (char *) event);
         *when = event->event_time;
         freeame ((char *) event, sizeof (struct t_event));
	 return;
      }
   }
   panic ("Can't extract requested event for thread P%02d T%02d t%02d\n",
	  IDENTIFIERS (thread));
   return;
}

/*
 * Local routines and simulator routines
 */
void
EVENT_init()
{
  if (debug&D_EV)
  {
    PRINT_TIMER (current_time);
    printf (": EVENT initial routine called\n");
  }
  create_queue (&Event_queue);
}

void
EVENT_end()
{
  if (debug&D_EV)
  {
    PRINT_TIMER (current_time);
    printf (": EVENT end routine called\n");
  }
}

t_boolean
events_for_thread (struct t_thread *thread)
{
  struct t_event *event;

  for (
    event  = head_event (&Event_queue);
    event != E_NIL;
    event  = next_event (&Event_queue)
  )
  {
    if (event->thread == thread)
    {
      return (TRUE);
    }
  }
  return (FALSE);
}

void
reload_events()
{
  register struct t_node *node;

  for (
    node  = (struct t_node *) head_queue (&Node_queue);
    node != N_NIL;
    node  = (struct t_node *) next_queue (&Node_queue)
  )
  {
    while ((count_queue (&(node->ready)) != 0) && (num_free_cpu (node) > 0))
    {
      SCHEDULER_next_thread_to_run(node);
    }
  }
}

/*
 * Main event managament routine
 */
void
event_manager (struct t_event *event)
{
  if GT_TIMER (current_time, event->event_time)
  {
    panic (
      "Time is going backwards old (%le) new(%le)\n",
      current_time,
      event->event_time
    );
  }

  if (debug&D_EV)
  {
    PRINT_TIMER (current_time);
    printf (": EVENT Selected ");
    /* PRINT_TIMER (event->event_time); */
    printf ("Module %d ", event->module);
    if (event->thread == TH_NIL)
    {
      printf("\n");
    }
    else
    {
      printf(
        ": P%02d T%02d (t%02d)\n",
        IDENTIFIERS(event->thread)
      );
    }
  }
  
  ASS_ALL_TIMER (current_time, event->event_time);

  if (OUT_OF_LIMIT(current_time))
  {
    /* Avoid underflow problem */
    panic("Time out of simulation limit\n");
  }
   
  if (event->daemon == DAEMON)
  {
    are_only_daemons--;
  }

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
  
  freeame ((char *) event, sizeof (struct t_event));
}
