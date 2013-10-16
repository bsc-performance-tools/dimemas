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

#define FALSE 0
#define TRUE 1

#include <types.h>

/**
 * External routines defined in file task.c
 **/
void  create_queue(struct t_queue *q);

void  inFIFO_queue(struct t_queue *queue, char *content);

void  inLIFO_queue(struct t_queue *queue, char *content);

char* outFIFO_queue(struct t_queue *q);

char* outLIFO_queue(struct t_queue *q);

t_count count_queue(struct t_queue *q);

t_boolean empty_queue(struct t_queue *q);

char* head_queue(struct t_queue *q);

char* next_queue(struct t_queue *q);

char* tail_queue(struct t_queue *q);

char* prev_queue(struct t_queue *q);

void  insert_queue( struct t_queue *queue,
                    char           *content,
                    t_priority      prio);

void insert_queue_from_back(struct t_queue *queue,
                            char           *content,
                            t_priority      prio);

void insert_first_queue( struct t_queue *queue,
                         char           *content,
                         t_priority      prio);

char* query_prio_queue(struct t_queue *queue, t_priority prio);

void extract_from_queue(struct t_queue *queue, char * content);

struct t_thread *find_thread_in_queue(struct t_queue *queue, struct t_thread *thread);

void create_event(struct t_queue *q);

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
