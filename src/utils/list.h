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

#ifndef __list_h
#define __list_h

#ifdef DIMEMAS_GUI

#define FALSE 0
#define TRUE 1

typedef int     t_boolean;
typedef double  t_priority;	/* priority for queue elements */
typedef int     t_count;	/* number of elements in queue */

typedef double  t_micro;

/*
 * Item for queues and lists Double chained item previous item of first in
 * queue is NIL next item of last in queue is NIL
 */
struct t_item
{
   struct t_item  *next;	/* Next item in queue */
   struct t_item  *prev;	/* Previous item in queue */
   union
   {
      t_priority      priority;	/* priority on sorted priority queues */
      double          list_time;/* time for next event in event queue */
   }               order;
   char           *content;	/* Real content of item */
};

struct t_queue
{
   struct t_item  *first;	/* First item in queue */
   struct t_item  *last;	/* Last item in queue */
   struct t_item  *curr;	/* Current item in sequential search */
   t_count         count;	/* Number of items */
};

struct t_list
{
   struct t_list  *next;
};

#define QU_NIL  (struct t_queue *)0
#define Q_NIL   (struct t_item *)0
#define LI_NIL   (struct t_list *)0
#define A_NIL   (char *)0

#endif

/**
 * External routines defined in file task.c
 **/
extern void  create_queue(struct t_queue *q);

extern void  inFIFO_queue(struct t_queue *queue, char *content);

extern void  inLIFO_queue(struct t_queue *queue, char *content);

extern char* outFIFO_queue(struct t_queue *q);

extern char* outLIFO_queue(struct t_queue *q);

extern t_count   count_queue(struct t_queue *q);

extern t_boolean empty_queue(struct t_queue *q);

extern char* head_queue(struct t_queue *q);

extern char* next_queue(struct t_queue *q);

extern char* tail_queue(struct t_queue *q);

extern char* prev_queue(struct t_queue *q);

extern void  insert_queue( struct t_queue *queue,
                           char           *content,
                           t_priority      prio);

extern void insert_first_queue( struct t_queue *queue,
                                char           *content, 
                                t_priority      prio);

extern char *query_prio_queue(struct t_queue *queue, t_priority prio);

extern void extract_from_queue(struct t_queue *queue, char * content);

extern void create_event(struct t_queue *q);

#ifndef DIMEMAS_GUI
extern void insert_event(struct t_queue *q, struct t_event *e);
#endif
extern struct t_event *top_event(struct t_queue *q);
extern t_boolean empty_event(struct t_queue *q);
extern struct t_event *outFIFO_event(struct t_queue *q);
extern struct t_event *head_event(struct t_queue *q);
extern struct t_event *next_event(struct t_queue *q);
extern struct t_queue *new_queue_accounter(void);

#endif
