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
#include "list.h"
#include "mallocame.h"
#include "subr.h"


#ifdef VENUS_ENABLED
#include "venusclient.h"
#endif


/*
 * Create empty queue
 */
void
create_queue(struct t_queue *q)
{
   q->first = Q_NIL;
   q->last = Q_NIL;
   q->curr = Q_NIL;
   q->count = (t_count) 0;
}

/*
 * Insert last
 */
void
inFIFO_queue(struct t_queue *queue, char  *content)
{
   register struct t_item *tmp;
   register struct t_item *new_item;

   new_item = (struct t_item *) mallocame (sizeof (struct t_item));
   new_item->content = content;

   tmp = queue->last;		/* The ancient last */
   queue->last = new_item;	/* The young last */

   new_item->next = Q_NIL;	/* don't have next */
   if (tmp != Q_NIL)
   {				/* If no top of queue */
      tmp->next = new_item;	/* set the next */
      new_item->prev = tmp;	/* and the previous */
   }
   else
   {
      queue->first = new_item;	/* set the first */
      new_item->prev = Q_NIL;	/* has no previous */
   }
   queue->count++;		/* One more in queue */
}

/*
 * Insert first
 */
void
inLIFO_queue(struct t_queue *queue, char *content)
{
   register struct t_item *tmp;
   register struct t_item *new_item;


   tmp = queue->first;		/* The ancient first */

   new_item = (struct t_item *) mallocame (sizeof (struct t_item));
   new_item->content = content;
   new_item->prev = Q_NIL;
   new_item->next = tmp;	/* have next */

   queue->first = new_item;	/* The young first */

   if (tmp == Q_NIL)
      queue->last = new_item;
   else
   {				/* If no top of queue */
      tmp->prev = new_item;	/* set the next */
   }
   queue->count++;		/* One more in queue */
}

/*
 * Extract first
 */
char *
outFIFO_queue(struct t_queue *q)
{
   register struct t_item *e;
   register char  *res;

   e = q->first;
   if (e == Q_NIL)		/* Empty queue */
      return (A_NIL);

   q->first = e->next;		/* second take first place */
   if (q->last == e)
      q->last = Q_NIL;

   if (e->next != Q_NIL)
      (e->next)->prev = Q_NIL;	/* Has no precessor */
   q->count--;			/* One less in queue */
   res = e->content;
   freeame ((char *) e, sizeof (struct t_item));
   return (res);
}

/*
 * Extract last
 */
char *
outLIFO_queue(struct t_queue *q)
{
   register struct t_item *e;
   register char  *res;

   e = q->last;
   if (e == Q_NIL)		/* Empty queue */
      return (A_NIL);

   q->last = e->prev;		/* second take first place */
   if (q->first == e)
      q->first = Q_NIL;

   if (e->prev != Q_NIL)
      (e->prev)->next = Q_NIL;	/* Has no precessor */
   q->count--;			/* One less in queue */
   res = e->content;
   freeame ((char *) e, sizeof (struct t_item));
   return (res);
}

/*
 * Get number of elements in queue
 */
t_count
count_queue(struct t_queue *q)
{
   return (q->count);
}

/*
 * Ask for empty queue
 */
t_boolean
empty_queue(struct t_queue *q)
{
   return (q->count == 0 ? TRUE : FALSE);
}

/*
 * Query first element in queue
 */
char *
head_queue(struct t_queue *q)
{
   register struct t_item *e;

   e = q->curr = q->first;
   if (e == Q_NIL)
      return (A_NIL);
   return ((char *) (e->content));
}

/*
 * Query next element in queue
 */
char *
next_queue(struct t_queue *q)
{
   register struct t_item *e;

   e = q->curr;
   if (e == Q_NIL)
      return (A_NIL);
   e = q->curr = e->next;
   if (e == Q_NIL)
      return (A_NIL);
   return ((char *) (e->content));
}

/*
 * Query last element in queue
 */
char *
tail_queue(struct t_queue *q)
{
   register struct t_item *e;

   e = q->curr = q->last;
   if (e == Q_NIL)
      return (A_NIL);
   return ((char *) (e->content));
}

/*
 * Query previous element in queue
 */
char *
prev_queue(struct t_queue *q)
{
   register struct t_item *e;

   e = q->curr;
   if (e == Q_NIL)
      return (A_NIL);
   e = q->curr = e->prev;
   return ((char *) (e->content));	/* one queue */
}

/*
 * Insert element in priority queue (1 preceeds 2)
 */
void
insert_queue(struct t_queue *queue, char *content, t_priority prio)
{
   register struct t_item *new_element;
   register struct t_item *tmp1,
                  *tmp2;

   new_element = (struct t_item *) mallocame (sizeof (struct t_item));

   new_element->order.priority = prio;
   new_element->content = content;
   new_element->next = Q_NIL;
   new_element->prev = Q_NIL;

   tmp1 = queue->first;
   tmp2 = Q_NIL;
   if (tmp1 == Q_NIL)
   {
      queue->first = new_element;
      queue->last = new_element;
   }
   else
   {
      while (tmp1->order.priority <= prio)
      {
	 tmp2 = tmp1;
	 tmp1 = tmp1->next;
	 if (tmp1 == Q_NIL)
	    break;
      }
      if (tmp2 == Q_NIL)
      {				/* Insertion on queue top */
	 queue->first = new_element;
	 new_element->next = tmp1;
	 tmp1->prev = new_element;
      }
      else
      {
      /* Insertion between tmp2 and tmp1 */
	 tmp2->next = new_element;
	 new_element->prev = tmp2;
	 new_element->next = tmp1;
	 if (tmp1 == Q_NIL)
	    queue->last = new_element;
	 else
	    tmp1->prev = new_element;
      }
   }
   queue->count++;
}

/*
 * Insert element in priority queue (1 preceeds 2) but if same priority
 * found, insert first
 */
void
insert_first_queue(struct t_queue *queue, char *content, t_priority prio)
{
   register struct t_item *new_element;
   register struct t_item *tmp1,
                  *tmp2;

   new_element = (struct t_item *) mallocame (sizeof (struct t_item));

   new_element->order.priority = prio;
   new_element->content = content;
   new_element->next = Q_NIL;
   new_element->prev = Q_NIL;

   tmp1 = queue->first;
   tmp2 = Q_NIL;
   if (tmp1 == Q_NIL)
   {
      queue->first = new_element;
      queue->last = new_element;
   }
   else
   {
      while (tmp1->order.priority < prio)
      {
	 tmp2 = tmp1;
	 tmp1 = tmp1->next;
	 if (tmp1 == Q_NIL)
	    break;
      }
      if (tmp2 == Q_NIL)
      {				/* Insertion on queue top */
	 queue->first = new_element;
	 new_element->next = tmp1;
	 tmp1->prev = new_element;
      }
      else
      {
      /* Insertion between tmp2 and tmp1 */
	 tmp2->next = new_element;
	 new_element->prev = tmp2;
	 new_element->next = tmp1;
	 if (tmp1 == Q_NIL)
	    queue->last = new_element;
	 else
	    tmp1->prev = new_element;
      }
   }
   queue->count++;
}

/*
 * Query element with priority from queue
 */
char *
query_prio_queue(struct t_queue *queue, t_priority prio)
{
   register struct t_item *element;

   for (element = queue->first; element != Q_NIL; element = element->next)
      if (element->order.priority == prio)
	 break;

   if (element != Q_NIL)
      return (element->content);
   else
      return (A_NIL);
}

/*
 * Extract element from queue
 */
void
extract_from_queue(struct t_queue *queue, char * content)
{
   register struct t_item *tmp1,
                  *tmp2;

   tmp1 = queue->first;
   tmp2 = Q_NIL;
   if (tmp1 == Q_NIL)
      panic ("Can't find element to extract from queue %s\n", content);

   while (tmp1->content != content)
   {
      tmp2 = tmp1;
      tmp1 = tmp1->next;
      if (tmp1 == Q_NIL)
	 panic ("Can't find element to extract from queue %s\n", content);
   }
   if (tmp2 == Q_NIL)
   {				/* Was queue head */
      queue->first = tmp1->next;
      if (tmp1->next != Q_NIL)
	 (tmp1->next)->prev = Q_NIL;
      else
	 queue->last = Q_NIL;
   }
   else
   {
      if (tmp1->next != Q_NIL)
      {
	 tmp2->next = tmp1->next;
	 (tmp1->next)->prev = tmp2;
      }
      else
      {
	 tmp2->next = Q_NIL;
	 queue->last = tmp2;
      }
   }
   
   if (tmp1==queue->curr)
   { 
     /* Si ens carreguem l'element actual, cal posar com a actual l'anterior */
     queue->curr=tmp2;
   }
   
   queue->count--;
   freeame ((char *) tmp1, sizeof (struct t_item));
}


/* Rutinas de events (Ordenacion por tiempo) */

/*
 * Create event queue
 */
void
create_event(struct t_queue *q)
{
   q->first = Q_NIL;
   q->last = Q_NIL;
   q->curr = Q_NIL;
   q->count = (t_count) 0;
}

/*
 * Insert event preserving time sort
 */
void
insert_event(struct t_queue *q, struct t_event *e)
{
   register struct t_item *new_element;
   register struct t_item *tmp1,
                  *tmp2;

   new_element = (struct t_item *) mallocame (sizeof (struct t_item));

   new_element->order.list_time = e->event_time;
   new_element->content = (char *) e;
   new_element->next = Q_NIL;
   new_element->prev = Q_NIL;

   tmp1 = q->first;
   tmp2 = Q_NIL;
   if (tmp1 == Q_NIL)
   {
      q->first = new_element;
      q->last = new_element;
   }
   else
   {
      while LE_TIMER
	 (tmp1->order.list_time, e->event_time)
      {
	 tmp2 = tmp1;
	 tmp1 = tmp1->next;
	 if (tmp1 == Q_NIL)
	    break;
      }
      if (tmp2 == Q_NIL)
      {				/* Insertion on queue top */
	 q->first = new_element;
	 new_element->next = tmp1;
	 tmp1->prev = new_element;
      }
      else
      {
	 tmp2->next = new_element;
	 new_element->prev = tmp2;
	 new_element->next = tmp1;
	 if (tmp1 == Q_NIL)
	    q->last = new_element;
	 else
	    tmp1->prev = new_element;
      }
   }
   q->count++;
}

/*
 * Ask first event element from queue
 */
struct t_event *
top_event(struct t_queue *q)
{
   if (q->count == 0)
      return (E_NIL);
   return ((struct t_event *) (q->first)->content);
}

/*
 * Empty event queue
 */

t_boolean
empty_event(struct t_queue *q)
{
   if (top_event (q) == E_NIL)
      return (TRUE);
   return (FALSE);
}

/*
 * Get first event element from queue
 */
struct t_event *
outFIFO_event(struct t_queue *q)
{
   register struct t_item *e = Q_NIL;
   register struct t_event *event = E_NIL;

#ifdef VENUS_ENABLED
   if (venus_enabled) {
   	venus_outFIFO_event(q,e);
   }
#endif

   e = q->first;
   if (e == Q_NIL)
      return (E_NIL);

   q->first = e->next;
   if (e->next != Q_NIL)
      (e->next)->prev = Q_NIL;
   q->count--;
   if (q->last == e)
      q->last = Q_NIL;
   event = (struct t_event *) e->content;
   freeame ((char *) e, sizeof (struct t_item));
   return (event);
}

/*
 * Query first element in event queue
 */
struct t_event *
head_event(struct t_queue *q)
{
   register struct t_item *e;

   e = q->curr = q->first;
   if (e == Q_NIL)
      return (E_NIL);
   return ((struct t_event *) (e->content));
}

/*
 * Query next element from event queue
 */
struct t_event *
next_event(struct t_queue *q)
{
   register struct t_item *e;

   e = q->curr;
   if (e == Q_NIL)
      return (E_NIL);
   e = q->curr = e->next;
   if (e == Q_NIL)
      return (E_NIL);
   return ((struct t_event *) (e->content));
}

/*
 * Allocates and creates queue
 */
struct t_queue *
new_queue_accounter()
{
   register struct t_queue *res;

   res = (struct t_queue *) mallocame (sizeof (struct t_queue));
   create_queue (res);
   return (res);
}
