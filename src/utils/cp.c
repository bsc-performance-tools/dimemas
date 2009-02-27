char cp_c_rcsid[]="$Id: cp.c,v 1.4 2005/03/01 08:57:54 paraver Exp $";
/******************************************************************************
*******************************************************************************
*
*  (c) Copyright 1994, 1995, 1996 - CENTRO EUROPEO DE PARALELISMO DE BARCELONA
*				  - UNIVERSITAT POLITECNICA DE CATALUNYA
*  ALL RIGHTS RESERVED
*  
*  	THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED
*  AND COPIED ONLY IN ACCORDANCE WITH THE TERMS OF SUCH LICENSE AND
*  WITH THE INCLUSION OF THE ABOVE COPYRIGHT NOTICE. THIS SOFTWARE OR
*  ANY OTHER COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE
*  AVAILABLE TO ANY OTHER PERSON. NO TITLE TO AND OWNERSHIP OF THE
*  SOFTWARE IS HEREBY TRANSFERRED.
*  
*  	THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT
*  NOTICE AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY CEPBA-UPC
*  OR ITS THIRD PARTY SUPPLIERS  
*  
*  	CEPBA-UPC AND ITS THIRD PARTY SUPPLIERS,
*  ASSUME NO RESPONSIBILITY FOR THE USE OR INABILITY TO USE ANY OF ITS
*  SOFTWARE . DIMEMAS AND ITS GUI IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*  KIND, AND CEPBA-UPC EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES, INCLUDING
*  BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
*  FITNESS FOR A PARTICULAR PURPOSE.
* 
*  
*******************************************************************************
******************************************************************************/

#include "define.h"
#include "types.h"

#include "extern.h"
#include "list.h"
#include "mallocame.h"
#include "paraver.h"
#include "cpu.h"

#include <stdlib.h>

extern struct t_queue *CP_nodes;
extern dimemas_timer current_time;

extern t_boolean Critical_Path_Analysis;

void
new_cp_node(struct t_thread *thread, int status)
{
  register struct t_cp_node *cpn;
  register struct t_module *mod;
  
  if (Critical_Path_Analysis==FALSE)
    return;

  mod = (struct t_module *)head_queue(&(thread->modules));
  if (mod==M_NIL)
  {
    return;
  }
  
  cpn = (struct t_cp_node *)mallocame(sizeof(struct t_cp_node));
  
  cpn->prev = thread->last_cp_node;
  if (thread->last_cp_node!=(struct t_cp_node *)0)
    thread->last_cp_node->next = cpn;
  cpn->next = (struct t_cp_node *)0;
  thread->last_cp_node = cpn;
  cpn->status = status;
  ASS_ALL_TIMER (cpn->final_time, current_time);
  
  cpn->block_identificator = mod->identificator;
  
  cpn->send = cpn->recv = (struct t_cp_node *)0;
  cpn->thread = thread;
  
  inFIFO_queue (CP_nodes, (char *)cpn);
}

void
new_cp_relation (struct t_thread *dest, struct t_thread *src)
{
  if (Critical_Path_Analysis==FALSE)
  {
    return;
  }
  if (dest->last_cp_node == (struct t_cp_node *)0)
  {
    return;
  }
  dest->last_cp_node->send = src->last_cp_node;
}

static int
compar (const void *p1, const void *p2)
{
  register struct t_module_cp *mocp1, *mocp2;
  float f1, f2, f3, f4;
  
  mocp1 = (struct t_module_cp *)p1;
  mocp2 = (struct t_module_cp *)p2;
  TIMER_TO_FLOAT (mocp1->timer, f1);    
  TIMER_TO_FLOAT (mocp1->timer_comm, f2);    
  TIMER_TO_FLOAT (mocp2->timer, f3);    
  TIMER_TO_FLOAT (mocp2->timer_comm, f4);    
  if (f1+f2<f3+f4) return (1);
  else return (-1);
}    

void
show_CP_graph ()
{
  register struct t_cp_node *cpn;
  register struct t_cp_node *cpnp;
  register struct t_cp_node *cpnn;
  dimemas_timer diff, diff2, diff3;
  struct t_queue visited_modules;
  struct t_module_cp *mocp;
  int i,n;
  struct t_module_cp *mocparr;
  float f1, f2, f0;
  struct t_node *node;
  struct t_cpu *cpu;
  struct t_queue CP_to_paraver;
  
  if (Critical_Path_Analysis==FALSE)
    return;
  
  create_queue (&CP_to_paraver);

  create_queue (&visited_modules);
  cpn = (struct t_cp_node *)tail_queue(CP_nodes);
  
  diff3 = 0;
  while (cpn!=(struct t_cp_node *)0)
  {
    cpnp = cpn->prev;
    ASS_ALL_TIMER (diff, cpn->final_time);

    if (cpnp != (struct t_cp_node *)0)
      ASS_ALL_TIMER (diff2, cpnp->final_time);
    else
      ASS_TIMER (diff2, 0);

    inFIFO_queue (&CP_to_paraver, (char *)cpn);

    SUB_TIMER (diff,diff2, diff);
    
    
    mocp = (struct t_module_cp *)query_prio_queue (&visited_modules, 
        (t_priority)cpn->block_identificator);
    if (mocp!=(struct t_module_cp *)0)
    {
      if (cpn->status==CP_OVERHEAD)
        ADD_TIMER (mocp->timer_comm, diff, mocp->timer_comm);
      else
        ADD_TIMER (mocp->timer, diff, mocp->timer);
      ADD_TIMER (diff3, mocp->timer_comm, mocp->timer_comm);
    }
    else
    {
      mocp = (struct t_module_cp *)mallocame(sizeof(struct t_module_cp));
      mocp->identifier = cpn->block_identificator;
      if (cpn->status==CP_OVERHEAD)
      {
        ASS_TIMER (mocp->timer, 0);
        ASS_ALL_TIMER (mocp->timer_comm, diff);
      }
      else
      {
        ASS_ALL_TIMER (mocp->timer, diff);
        ASS_TIMER (mocp->timer_comm, 0);
      }
      ADD_TIMER (mocp->timer_comm, diff3, mocp->timer_comm);
      insert_queue (&visited_modules,(char *)mocp,(t_priority)mocp->identifier);
    }

    if (NEQ_0_TIMER(diff3))
    {
      ADD_TIMER (diff, diff3, diff);
      ASS_ALL_TIMER (diff3,0);
    }
    
    if (cpnp!=(struct t_cp_node *)0)
    {
      if (cpnp->status==CP_BLOCK)
      {
        cpn = cpnp->send;
        ASS_ALL_TIMER(diff3, cpnp->final_time);
        SUB_TIMER (diff3, cpn->final_time, diff3);
      }
      else
        cpn = cpnp;
    }
    else
      cpn = cpnp;
  }

  ASS_TIMER (diff2, 0);
  cpnp = (struct t_cp_node *)0;
  cpn = (struct t_cp_node *)outLIFO_queue(&CP_to_paraver);
  while (cpn!=(struct t_cp_node *)0)
  {
    cpnn = (struct t_cp_node *)outLIFO_queue(&CP_to_paraver);
    node = get_node_of_thread (cpn->thread);

    if (cpnp==(struct t_cp_node *)0)
    {
      Paraver_event (0, IDENTIFIERS (cpn->thread),
              diff2, PARAVER_CP_BLOCK, PARAVER_CP_ENTER);
    }
    else
    {
      if (cpnp->thread!=cpn->thread)
	Paraver_event (0, IDENTIFIERS (cpn->thread),
		       diff2, PARAVER_CP_BLOCK, PARAVER_CP_ENTER);
    }
    if (cpnn==(struct t_cp_node *)0)
      Paraver_event (0, IDENTIFIERS (cpn->thread),
              cpn->final_time, PARAVER_CP_BLOCK, PARAVER_CP_EXIT);
    else
    {
      if (cpnn->thread!=cpn->thread)
        Paraver_event (0, IDENTIFIERS (cpn->thread),
                cpn->final_time, PARAVER_CP_BLOCK, PARAVER_CP_EXIT);
    }
    ASS_ALL_TIMER (diff2, cpn->final_time);
    cpnp = cpn;
    cpn = cpnn;
  }
  
  fprintf(salida_datos," Block\t %%CPU\t%%COMM\n");
  TIMER_TO_FLOAT (current_time, f0);
  
  n = count_queue(&visited_modules);
  mocparr = (struct t_module_cp *)mallocame(n*sizeof(struct t_module_cp));
  for (i=0;i<n;i++) 
  {
    mocp = (struct t_module_cp *)outFIFO_queue(&visited_modules);
    memcpy (&mocparr[i],mocp, sizeof(struct t_module_cp));
  }
  
  /* Sort output */
  qsort (mocparr, n, sizeof(struct t_module_cp), compar);
  for (i=0;i<n;i++) 
  {
    TIMER_TO_FLOAT (mocparr[i].timer, f1);    
    TIMER_TO_FLOAT (mocparr[i].timer_comm, f2);    
    fprintf (salida_datos, "%6d\t%5.2f\t%5.2f\n",mocparr[i].identifier,
        f1*100/f0,
        f2*100/f0);
  }  
}
