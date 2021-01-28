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


#include <cpu.h>
#include <define.h>
#include <extern.h>
#include <list.h>
#include <node.h>
#include <simulator.c>
#include <subr.h>
#include <task.h>
#include <types.h>

void CPU_Get_Unique_CPU_IDs( void )
{
  struct t_node *no;
  struct t_cpu *cpu;
  int number = 1;


  int node_id;
  for ( node_id = 0; node_id < SIMULATOR_get_number_of_nodes(); ++node_id )
  {
    struct t_node *node = &nodes[ node_id ];
    for ( cpu = (struct t_cpu *)head_queue( &( node->Cpus ) ); cpu != NULL; cpu = (struct t_cpu *)next_queue( &( node->Cpus ) ) )
    {
      cpu->unique_number = number;
      printf( "cpu->uniquenumber is %d\n", number );
      number++;
    }
  }
}

/*need to count the number of GPU's if we have more than one gpu in input*/
struct t_node *get_node_of_thread( struct t_thread *thread )
{
  struct t_node *node = thread->task->node;

  if ( node == (struct t_node *)0 )
  {
    panic( "Unable to locate node %d for P%d T%d t%d\n", node->nodeid, IDENTIFIERS( thread ) );
  }
  return node;
}

struct t_node *get_node_of_task( struct t_task *task )
{
  struct t_thread *thread;

  /* JGG (2012/01/16): first thread
     thread = (struct t_thread *) head_queue (&(task->threads)); */
  thread = task->threads[ 0 ];
  return ( get_node_of_thread( thread ) );
}

struct t_node *get_node_by_id( int node_id )
{
  return &nodes[ node_id ];
}

void check_full_nodes()
{
  struct t_node *node;

  int node_id;
  for ( node_id = 0; node_id < SIMULATOR_get_number_of_nodes(); ++node_id )
  {
    struct t_node *node = &nodes[ node_id ];
    if ( count_queue( &( node->Cpus ) ) == 0 )
    {
      panic( "Node %d non initialized\n", node->nodeid );
    }
  }
}

int num_free_cpu( struct t_node *node )
{
  register struct t_cpu *cpu;
  int i = 0;
  for ( cpu = (struct t_cpu *)head_queue( &( node->Cpus ) ); cpu != C_NIL; cpu = (struct t_cpu *)next_queue( &( node->Cpus ) ) )
  {
    if ( cpu->current_thread == TH_NIL && cpu->is_gpu == FALSE )
      i++;
  }
  return ( i );
}

t_boolean is_thread_running( struct t_thread *thread )
{
  register struct t_cpu *cpu;
  register struct t_node *node;
  register struct t_thread *kernel_thread;

  node = get_node_of_thread( thread );
  for ( cpu = (struct t_cpu *)head_queue( &( node->Cpus ) ); cpu != C_NIL; cpu = (struct t_cpu *)next_queue( &( node->Cpus ) ) )
  {
    if ( cpu->current_thread == thread || cpu->current_thread == kernel_thread )
      return ( TRUE );
  }
  return ( FALSE );
}
