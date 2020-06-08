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

#include "define.h"
#include "types.h"

#include "extern.h"
#include "list.h"
#include "subr.h"
#include "task.h"


#ifdef VENUS_ENABLED
#include "listE.h"
#include "venusclient.h"
#endif


#include<assert.h>

/*
 * Create empty queue
 */
void create_queue(struct t_queue *q)
{
    q->first = (struct t_item *) Q_NIL;
    q->last  = (struct t_item *) Q_NIL;
    q->curr  = (struct t_item *) Q_NIL;
    q->count = (t_count) 0;
}

/*
 * Insert last
 */
void inFIFO_queue(struct t_queue *queue, char  *content)
{
    register struct t_item *tmp;
    register struct t_item *new_item;

    new_item = (struct t_item *) malloc (sizeof (struct t_item));
    new_item->content = content;

    tmp = queue->last;          
    queue->last = new_item;    
    new_item->next = NULL;    
    if (tmp != NULL)
    {                         
        tmp->next = new_item; 
        new_item->prev = tmp; 
    }
    else
    {
        queue->first   = new_item; 
        new_item->prev = NULL;  
    }

    queue->count++;            
}

/*
 * Insert first
 */
void inLIFO_queue(struct t_queue *queue, char *content)
{
    register struct t_item *tmp;
    register struct t_item *new_item;

    tmp = queue->first;	

    new_item = (struct t_item*) malloc (sizeof (struct t_item));
    new_item->content = content;
    new_item->prev = NULL;
    new_item->next = tmp; 

    queue->first = new_item;

    if (tmp == NULL)
    {
        queue->last = new_item;
    }
    else
    {                      
        tmp->prev = new_item;
    }

    queue->count++;
}

/*
 * Extract first
 */
char* outFIFO_queue(struct t_queue *q)
{
    register struct t_item *e;
    register char  *res;

    e = q->first;
    if (e == NULL ) 
    {
        return (A_NIL);
    }

    q->first = e->next;

    if (q->last == e)
    {
        q->last = NULL;
    }

    if (e->next != NULL)
    {
        (e->next)->prev = NULL; 
    }

    q->count--;
        
    res = e->content;
    free (e);
    if(res == NULL)
        return (A_NIL);
    e = NULL;
    return (res);
}

/*
 * Extract last
 */
char* outLIFO_queue(struct t_queue *q)
{
    register struct t_item *e;
    register char  *res;

    e = q->last;
    if (e == NULL)
    {
        return (A_NIL);
    }

    q->last = e->prev;
    if (q->first == e)
    {
        q->first = NULL;
    }

    if (e->prev != NULL)
    {
        (e->prev)->next = NULL;
    }

    q->count--;
    res = e->content;
    free (e);
    return (res);
}

/*
 * Get number of elements in queue
 */
t_count count_queue(struct t_queue *q)
{
    return (q->count);
}

/*
 * Ask for empty queue
 */
t_boolean empty_queue(struct t_queue *q)
{
    return (q->count == 0 ? TRUE : FALSE);
}

/*
 * Query first element in queue
 */
char* head_queue(struct t_queue *q)
{
    register struct t_item *e;

    e = q->curr = q->first;

    if (e == NULL)
    {
        return (A_NIL);
    }

    return ((char*) (e->content));
}

/*
 * Query next element in queue
 */
char* next_queue(struct t_queue *q)
{
    register struct t_item *e;

    e = q->curr;
    if (e == NULL)
    {
        return (A_NIL);
    }
    e = q->curr = e->next;
    if (e == NULL)
    {
        return (A_NIL);
    }
    return ((char *) (e->content));
}

/*
 * Query last element in queue
 */
char* tail_queue(struct t_queue *q)
{
    register struct t_item *e;

    e = q->curr = q->last;
    if (e == NULL)
    {
        return (A_NIL);
    }

    return ((char *) (e->content));
}

/**
 * Query previous element in queue
 */
char* prev_queue(struct t_queue *q)
{
    register struct t_item *e;

    e = q->curr;
    if (e == NULL)
    {
        return (A_NIL);
    }
    e = q->curr = e->prev;
    return ((char *) (e->content)); /* one queue */
}

/**
 * Insert element in priority queue (1 preceeds 2)
 */
void insert_queue(struct t_queue *queue, char *content, t_priority prio)
{
    register struct t_item *new_element;
    register struct t_item *tmp1, *tmp2;

    new_element = (struct t_item *) malloc (sizeof (struct t_item));

    new_element->order.priority = prio;
    new_element->content        = content;
    new_element->next           = NULL;
    new_element->prev           = NULL;

    tmp1 = queue->first;
    tmp2 = NULL;

    if (tmp1 == NULL)
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

            if (tmp1 == NULL)
            {
                break;
            }
        }

        if (tmp2 == NULL)
        { 
            queue->first      = new_element;
            new_element->next = tmp1;
            tmp1->prev        = new_element;
        }
        else
        {
            tmp2->next        = new_element;
            new_element->prev = tmp2;
            new_element->next = tmp1;

            if (tmp1 == NULL)
            {
                queue->last = new_element;
            }
            else
            {
                tmp1->prev = new_element;
            }
        }
    }
    queue->count++;
}

/*
 * Insert element in priority queue (1 preceeds 2)
 * this routine inserts the element starting from the back of the queue
 * this optimizes SS_MPI_CP scheduling time
 */
void insert_queue_from_back(struct t_queue *queue, char *content, t_priority prio)
{
    register struct t_item *new_element;
    register struct t_item *tmp1, *tmp2;

    new_element = (struct t_item *) malloc (sizeof (struct t_item));

    new_element->order.priority = prio;
    new_element->content        = content;
    new_element->next           = NULL;
    new_element->prev           = NULL;

    tmp1 = queue->last;
    tmp2 = NULL;

    if (tmp1 == NULL)
    {
        queue->first = new_element;
        queue->last = new_element;
    }
    else
    {
        while (tmp1->order.priority > prio)
        {
            tmp2 = tmp1;
            tmp1 = tmp1->prev;

            if (tmp1 == NULL)
            {
                break;
            }
        }

        if (tmp2 == NULL)
        {
            /* Insertion on queue back */
            tmp1->next        = new_element;
            new_element->prev = tmp1;
            queue->last       = new_element;
        }
        else
        {
            /* Insertion between tmp1 and tmp2 */
            tmp2->prev        = new_element;
            new_element->next = tmp2;
            new_element->prev = tmp1;

            if (tmp1 == NULL)
            {
                queue->first = new_element;
            }
            else
            {
                tmp1->next = new_element;
            }
        }
    }
    queue->count++;
}

/*
 * Insert element in priority queue (1 preceeds 2) but if same priority
 * found, insert first
 */
void insert_first_queue(struct t_queue *queue, char *content, t_priority prio)
{
    register struct t_item *new_element;
    register struct t_item *tmp1,
             *tmp2;

    new_element = (struct t_item *) malloc (sizeof (struct t_item));

    new_element->order.priority = prio;
    new_element->content        = content;
    new_element->next           = NULL;
    new_element->prev           = NULL;

    tmp1 = queue->first;
    tmp2 = NULL;

    if (tmp1 == NULL)
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
            if (tmp1 == NULL)
            {
                break;
            }
        }
        if (tmp2 == NULL)
        { /* Insertion on queue top */
            queue->first      = new_element;
            new_element->next = tmp1;
            tmp1->prev        = new_element;
        }
        else
        { /* Insertion between tmp2 and tmp1 */
            tmp2->next = new_element;
            new_element->prev = tmp2;
            new_element->next = tmp1;
            if (tmp1 == NULL)
            {
                queue->last = new_element;
            }
            else
            {
                tmp1->prev = new_element;
            }
        }
    }
    queue->count++;
}

/*
 * Query element with priority from queue
 */
char* query_prio_queue(struct t_queue *queue, t_priority prio)
{
    register struct t_item *element;

    for (element = queue->first; element != NULL; element = element->next)
    {
        if (element->order.priority == prio)
        {
            break;
        }
    }

    if (element != NULL)
    {
        return (element->content);
    }
    else
    {
        return (A_NIL);
    }
}

/*
 * Extract element from queue
 */
void extract_from_queue(struct t_queue *queue, char* content)
{
    register struct t_item *tmp1, *tmp2;

    tmp1 = queue->first;
    tmp2 = NULL;

    if (tmp1 == NULL)
    {
        panic ("Can't find element to extract from queue %s\n", content);
    }

    while (tmp1->content != content)
    {
        tmp2 = tmp1;
        tmp1 = tmp1->next;
        if (tmp1 == NULL)
        {
            panic ("Can't find element to extract from queue %s\n", content);
        }
    }

    if (tmp2 == NULL)
    { /* Was queue head */
        queue->first = tmp1->next;
        if (tmp1->next != NULL)
        {
            (tmp1->next)->prev = NULL;
        }
        else
        {
            queue->last = NULL;
        }
    }
    else
    {
        if (tmp1->next != NULL)
        {
            tmp2->next         = tmp1->next;
            (tmp1->next)->prev = tmp2;
        }
        else
        {
            tmp2->next  = NULL;
            queue->last = tmp2;
        }
    }

    if (tmp1 == queue->curr)
    { /* Si ens carreguem l'element actual, cal posar com a actual l'anterior */
        queue->curr=tmp2;
    }

    queue->count--;
    free (tmp1);
}


/* Rutinas de events (Ordenacion por tiempo) */

/*
 * Create event queue
 */
void create_event(struct t_queue *q)
{
    q->first = NULL;
    q->last  = NULL;
    q->curr  = NULL;
    q->count = 0;
}

/*
 * Insert event preserving time sort
 */
void insert_event(struct t_queue *q, struct t_event *e)
{
    register struct t_item *new_element;
    register struct t_item *tmp1, *tmp2;

    new_element = (struct t_item *) malloc (sizeof (struct t_item));

    new_element->order.list_time = e->event_time;
    new_element->content         = (char*) e;
    new_element->next            = NULL;
    new_element->prev            = NULL;

    tmp1 = q->first;
    tmp2 = NULL;
    if (tmp1 == NULL)
    {
        q->first = new_element;
        q->last = new_element;
    }
    else
    {
        while LE_TIMER(tmp1->order.list_time, e->event_time)
        {
            tmp2 = tmp1;
            tmp1 = tmp1->next;
            if (tmp1 == NULL)
            {
                break;
            }
        }

        if (tmp2 == NULL)
        { /* Insertion on queue top */
            q->first          = new_element;
            new_element->next = tmp1;
            tmp1->prev        = new_element;
        }
        else
        {
            tmp2->next        = new_element;
            new_element->prev = tmp2;
            new_element->next = tmp1;


            if (tmp1 != NULL)
            {
                tmp1->prev = new_element;
            }
            else
            {
                q->last = new_element;
            }
        }
    }

    q->count++;
}

/*
 * Ask first event element from queue
 */
struct t_event* top_event(struct t_queue *q)
{
    if (q->count == 0)
    {
        return (E_NIL);
    }

    return ((struct t_event *) (q->first)->content);
}

/*
 * Empty event queue
 */

t_boolean empty_event(struct t_queue *q)
{
    if (top_event (q) == E_NIL)
    {
        return TRUE;
    }

    return FALSE;
}

/*
 * Get first event element from queue
 */
struct t_event* outFIFO_event(struct t_queue *q)
{
    register struct t_item  *e         = NULL;
    register struct t_event *event     = NULL;

#ifdef VENUS_ENABLED
    if (VC_is_enabled())
    {
        venus_outFIFO_event(q, event);
    }
#endif

    if (event == NULL)
    {
        e = q->first;
        if (e == NULL)
        {
            return (NULL);
        }

        q->first = e->next;

        if (e->next != NULL)
        {
            (e->next)->prev = NULL;
        }

        q->count--;

        if (q->last == e)
        {
            q->last = NULL;
        }

        event = (struct t_event*) e->content;
        free (e);
    }
    return (event);
}

/*
 * Query first element in event queue
 */
struct t_event* head_event(struct t_queue *q)
{
    register struct t_item *e;

    e = q->curr = q->first;
    if (e == NULL)
    {
        return (E_NIL);
    }

    return ((struct t_event *) (e->content));
}

/*
 * Query next element from event queue
 */
struct t_event* next_event(struct t_queue *q)
{
    register struct t_item *e;

    e = q->curr;
    if (e == NULL)
    {
        return (E_NIL);
    }

    e = q->curr = e->next;
    if (e == NULL)
    {
        return (E_NIL);
    }

    return ((struct t_event *) (e->content));
}

/*
 * Allocates and creates queue
 */
struct t_queue* new_queue_accounter()
{
    register struct t_queue *res;

    res = (struct t_queue *) malloc (sizeof (struct t_queue));
    create_queue (res);
    return (res);
}

/** 
 * Removes all the queue elements.
 * 
 * \param queue which needs to get empty
 * 
*/
void remove_queue_elements(struct t_queue * queue)
{
    
    if (queue->count == 0) 
        return;
    char * element;
    while ((element = outFIFO_queue(queue)) != NULL)
    {
            free(element);
            element = NULL;
    }

    queue->first = NULL;
    queue->last =NULL;
    queue->curr = NULL;
    assert(queue->count == 0);

}

void remove_queue_threads(struct t_queue * queue)
{
    if (queue->count == 0)
        return;

    struct t_thread *th;
    while ((th = (struct t_thread *)outFIFO_queue(queue)) != NULL)
    {
        if (!th->original_thread)
            delete_duplicate_thread(th);
    }

    queue->first = NULL;
    queue->last = NULL;
    queue->curr = NULL;
    assert(queue->count == 0);
}

void move_queue_elements(struct t_queue * from, struct t_queue * to)
{
    while (from->count > 0)
    {
        inFIFO_queue(to,outFIFO_queue(from));
    }
}

