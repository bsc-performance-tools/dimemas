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

#include "cpu.h"
#include "node.h"
#include "extern.h"
#include "list.h"
#ifdef USE_EQUEUE
#include "listE.h"
#endif
#include "subr.h"
#include "task.h"

void CPU_Get_Unique_CPU_IDs (void)
{
  struct t_node *no;
  struct t_cpu *cpu;
  int number = 1;
// add one more cpu as a gpu if t_boolean == TRUE;
#ifdef USE_EQUEUE
  for (no  = (struct t_node*) head_Equeue (&Node_queue);
       no != NULL;
       no  = (struct t_node*) next_Equeue (&Node_queue) )
#else
  for (no  = (struct t_node*) head_queue (&Node_queue);
       no != NULL;
       no  = (struct t_node*) next_queue (&Node_queue) )
#endif
  {
    for (cpu  = (struct t_cpu*) head_queue (& (no->Cpus) );
         cpu != NULL;
         cpu  = (struct t_cpu*) next_queue (& (no->Cpus) ) )
    {
      cpu->unique_number = number;
      number++;                     /*counting the number of cpu's and put them in the queue */        
    }
  }
}
/*need to count the number of GPU's if we have more than one gpu in input*/
struct t_node *get_node_of_thread (struct t_thread *thread)
{
  register struct t_account *account;
  register struct t_node *node;

  account = current_account (thread);
  if (account == ACC_NIL)
    panic ("Thread without account P%d T%d t%d\n", IDENTIFIERS (thread) );
  #ifdef USE_EQUEUE
  node = (struct t_node *) query_prio_Equeue (&Node_queue,
         (t_priority) account->nodeid);
  #else
  node = (struct t_node *) query_prio_queue (&Node_queue,
         (t_priority) account->nodeid);
  #endif
  if (node == (struct t_node *) 0)
    panic ("Unable to locate node %d for P%d T%d t%d\n",
           account->nodeid, IDENTIFIERS (thread) );
  return (node);
}

struct t_node *get_node_of_task (struct t_task *task)
{
  struct t_thread *thread;

  /* JGG (2012/01/16): first thread
  thread = (struct t_thread *) head_queue (&(task->threads)); */
  thread = task->threads[0];
  return (get_node_of_thread (thread) );
}

struct t_node *get_node_by_id (int nodeid)
{
#ifdef USE_EQUEUE
  return ( (struct t_node *) query_prio_Equeue (&Node_queue, (t_priority) nodeid) );
#else
  return ( (struct t_node *) query_prio_queue (&Node_queue, (t_priority) nodeid) );
#endif
}

void check_full_nodes()
{
  struct t_node  *node;

#ifdef USE_EQUEUE
  for (node = (struct t_node *) head_Equeue (&Node_queue);
       node != N_NIL;
       node = (struct t_node *) next_Equeue (&Node_queue) )
#else
  for (node = (struct t_node *) head_queue (&Node_queue);
       node != N_NIL;
       node = (struct t_node *) next_queue (&Node_queue) )
#endif
  {
    if (count_queue (& (node->Cpus) ) == 0)
    {
      panic ("Node %d non initialized\n", node->nodeid);
    }
  }
}

int num_free_cpu (struct t_node *node)
{
  register struct t_cpu *cpu;
  int i = 0;
  for (cpu = (struct t_cpu *) head_queue (& (node->Cpus) );
       cpu != C_NIL;
       cpu = (struct t_cpu *) next_queue (& (node->Cpus) ) )
  {
    if (cpu->current_thread == TH_NIL && cpu->is_gpu == FALSE)
      i++;
  }
  return (i);
}

t_boolean is_thread_running (struct t_thread *thread)
{
  register struct t_cpu *cpu;
  register struct t_node *node;
  struct t_thread *kernel_thread;

  node = get_node_of_thread (thread);
  for (cpu = (struct t_cpu *) head_queue (& (node->Cpus) );
       cpu != C_NIL;
       cpu = (struct t_cpu *) next_queue (& (node->Cpus) ) )
  {
    if (cpu->current_thread == thread || cpu->current_thread ==  kernel_thread)
      return (TRUE);
  }
  return (FALSE);
}
