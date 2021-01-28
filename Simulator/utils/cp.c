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

#include "cpu.h"
#include "define.h"
#include "extern.h"
#include "list.h"
#include "modules_map.h"
#include "paraver.h"
#include "types.h"

#include <stdlib.h>

extern struct t_queue *CP_nodes;
extern dimemas_timer current_time;

extern t_boolean Critical_Path_Analysis;

void new_cp_node( struct t_thread *thread, int status )
{
  register struct t_cp_node *cpn;
  register struct t_module *mod;

  if ( Critical_Path_Analysis == FALSE )
  {
    return;
  }

  mod = (struct t_module *)head_queue( &( thread->modules ) );
  if ( mod == M_NIL )
  {
    return;
  }

  cpn = (struct t_cp_node *)malloc( sizeof( struct t_cp_node ) );

  cpn->prev = thread->last_cp_node;
  if ( thread->last_cp_node != (struct t_cp_node *)0 )
  {
    thread->last_cp_node->next = cpn;
  }

  cpn->next            = (struct t_cp_node *)0;
  thread->last_cp_node = cpn;
  cpn->status          = status;
  ASS_ALL_TIMER( cpn->final_time, current_time );

  cpn->module_type  = mod->type;
  cpn->module_value = mod->value;

  cpn->send = cpn->recv = (struct t_cp_node *)0;
  cpn->thread           = thread;

  inFIFO_queue( CP_nodes, (char *)cpn );
}

void new_cp_relation( struct t_thread *dest, struct t_thread *src )
{
  if ( Critical_Path_Analysis == FALSE )
  {
    return;
  }
  if ( dest->last_cp_node == (struct t_cp_node *)0 )
  {
    return;
  }
  dest->last_cp_node->send = src->last_cp_node;
}

static int compar( const void *p1, const void *p2 )
{
  register struct t_module_cp *mocp1, *mocp2;
  float f1, f2, f3, f4;

  mocp1 = (struct t_module_cp *)p1;
  mocp2 = (struct t_module_cp *)p2;

  TIMER_TO_FLOAT( mocp1->timer, f1 );
  TIMER_TO_FLOAT( mocp1->timer_comm, f2 );
  TIMER_TO_FLOAT( mocp2->timer, f3 );
  TIMER_TO_FLOAT( mocp2->timer_comm, f4 );

  if ( f1 + f2 < f3 + f4 )
  {
    return ( 1 );
  }
  else
  {
    return ( -1 );
  }
}

void show_CP_graph()
{
  register struct t_cp_node *cpn;
  register struct t_cp_node *cpnp;
  register struct t_cp_node *cpnn;
  dimemas_timer diff, diff2, diff3;

  modules_map visited_modules;
  struct t_module_cp *mocp;
  int i, n;
  struct t_module_cp *mocparr;
  float f1, f2, f0;
  struct t_node *node;
  struct t_cpu *cpu;
  struct t_queue CP_to_paraver;

  if ( Critical_Path_Analysis == FALSE )
    return;

  create_queue( &CP_to_paraver );

  create_modules_map( &visited_modules );

  cpn = (struct t_cp_node *)tail_queue( CP_nodes );

  diff3 = 0;
  while ( cpn != (struct t_cp_node *)0 )
  {
    cpnp = cpn->prev;
    ASS_ALL_TIMER( diff, cpn->final_time );

    if ( cpnp != (struct t_cp_node *)0 )
      ASS_ALL_TIMER( diff2, cpnp->final_time );
    else
      ASS_TIMER( diff2, 0 );

    inFIFO_queue( &CP_to_paraver, (char *)cpn );

    SUB_TIMER( diff, diff2, diff );


    mocp = (struct t_module_cp *)find_module( &visited_modules, cpn->module_type, cpn->module_value );
    /*
    mocp = (struct t_module_cp *) query_prio_queue (&visited_modules,
                                                    (t_priority)cpn->block_identificator);
    */

    if ( mocp != NULL )
    {
      if ( cpn->status == CP_OVERHEAD )
      {
        ADD_TIMER( mocp->timer_comm, diff, mocp->timer_comm );
      }
      else
      {
        ADD_TIMER( mocp->timer, diff, mocp->timer );
      }
      ADD_TIMER( diff3, mocp->timer_comm, mocp->timer_comm );
    }
    else
    {
      mocp               = (struct t_module_cp *)malloc( sizeof( struct t_module_cp ) );
      mocp->module_type  = cpn->module_type;
      mocp->module_value = cpn->module_value;

      if ( cpn->status == CP_OVERHEAD )
      {
        ASS_TIMER( mocp->timer, 0 );
        ASS_ALL_TIMER( mocp->timer_comm, diff );
      }
      else
      {
        ASS_ALL_TIMER( mocp->timer, diff );
        ASS_TIMER( mocp->timer_comm, 0 );
      }
      ADD_TIMER( mocp->timer_comm, diff3, mocp->timer_comm );
      // insert_queue (&visited_modules,(char *)mocp,(t_priority)mocp->identifier);
      insert_module( &visited_modules, mocp->module_type, mocp->module_value, (void *)mocp );
    }

    if ( NEQ_0_TIMER( diff3 ) )
    {
      ADD_TIMER( diff, diff3, diff );
      ASS_ALL_TIMER( diff3, 0 );
    }

    if ( cpnp != (struct t_cp_node *)0 )
    {
      if ( cpnp->status == CP_BLOCK )
      {
        cpn = cpnp->send;
        ASS_ALL_TIMER( diff3, cpnp->final_time );
        SUB_TIMER( diff3, cpn->final_time, diff3 );
      }
      else
        cpn = cpnp;
    }
    else
      cpn = cpnp;
  }

  ASS_TIMER( diff2, 0 );
  cpnp = (struct t_cp_node *)0;
  cpn  = (struct t_cp_node *)outLIFO_queue( &CP_to_paraver );
  while ( cpn != (struct t_cp_node *)0 )
  {
    cpnn = (struct t_cp_node *)outLIFO_queue( &CP_to_paraver );
    node = get_node_of_thread( cpn->thread );

    if ( cpnp == (struct t_cp_node *)0 )
    {
      PARAVER_Event( 0, IDENTIFIERS( cpn->thread ), diff2, PARAVER_CP_BLOCK, PARAVER_CP_ENTER );
    }
    else
    {
      if ( cpnp->thread != cpn->thread )
      {
        PARAVER_Event( 0, IDENTIFIERS( cpn->thread ), diff2, PARAVER_CP_BLOCK, PARAVER_CP_ENTER );
      }
    }

    if ( cpnn == (struct t_cp_node *)0 )
    {
      PARAVER_Event( 0, IDENTIFIERS( cpn->thread ), cpn->final_time, PARAVER_CP_BLOCK, PARAVER_CP_EXIT );
    }
    else
    {
      if ( cpnn->thread != cpn->thread )
        PARAVER_Event( 0, IDENTIFIERS( cpn->thread ), cpn->final_time, PARAVER_CP_BLOCK, PARAVER_CP_EXIT );
    }
    ASS_ALL_TIMER( diff2, cpn->final_time );
    cpnp = cpn;
    cpn  = cpnn;
  }

  fprintf( salida_datos, " Block\t %%CPU\t%%COMM\n" );
  TIMER_TO_FLOAT( current_time, f0 );

  n       = count_map( &visited_modules );
  mocparr = (struct t_module_cp *)malloc( n * sizeof( struct t_module_cp ) );

  mocp = (struct t_module_cp *)head( &visited_modules );
  while ( mocp != NULL )
  {
    memcpy( &mocparr[ i ], mocp, sizeof( struct t_module_cp ) );
    mocp = next( &visited_modules );
  }

  /*
  for (i=0;i<n;i++)
  {
    mocp = (struct t_module_cp *)outFIFO_queue(&visited_modules);
    memcpy (&mocparr[i],mocp, sizeof(struct t_module_cp));
  }
  */

  /* Sort output */
  qsort( mocparr, n, sizeof( struct t_module_cp ), compar );

  for ( i = 0; i < n; i++ )
  {
    TIMER_TO_FLOAT( mocparr[ i ].timer, f1 );
    TIMER_TO_FLOAT( mocparr[ i ].timer_comm, f2 );
    fprintf( salida_datos, "[%lld:%lld]\t%5.2f\t%5.2f\n", mocparr[ i ].module_type, mocparr[ i ].module_value, f1 * 100 / f0, f2 * 100 / f0 );
  }
}
