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

#include <assert.h>

/* Includes 'por defecto' */
#include "define.h"
#include "types.h"

/* Include propio */
#include "SCH_prio_fifo.h"

/* Dependencias con otros fuentes */
#include "cpu.h"
#include "extern.h"
#include "list.h"
#include "mallocame.h"
#include "schedule.h"
#include "subr.h"

void
PRIO_FIFO_thread_to_ready(struct t_thread *thread)
{
  struct t_node          *node;
  struct t_cpu           *cpu, *cpu_to_preemp;
  struct t_SCH_prio_fifo *sch_prio_fifo, *sch_prio_fifo_cur;
  t_priority              priority;
  struct t_thread        *thread_current;
  struct t_machine       *machine;

  node    = get_node_of_thread (thread);
  machine = node->machine;

  sch_prio_fifo = (struct t_SCH_prio_fifo *) thread->sch_parameters;
  priority      = sch_prio_fifo->priority;
   
  if (
    (machine->scheduler.priority_preemptive) && 
    (select_free_cpu (node, thread) == C_NIL)
  )
  {
    cpu_to_preemp = C_NIL;
    /* Select processor to preemp */
    for (
      cpu  = (struct t_cpu*) head_queue (&(node->Cpus));
      cpu != C_NIL;
      cpu  = (struct t_cpu*) next_queue (&(node->Cpus))
    )
    {
      thread_current    = cpu->current_thread;
      sch_prio_fifo_cur = 
        (struct t_SCH_prio_fifo *) thread_current->sch_parameters;

      if (
        (sch_prio_fifo_cur->priority > sch_prio_fifo->priority) &&
        (sch_prio_fifo_cur->priority > priority)
      )
      {
        priority      = sch_prio_fifo_cur->priority;
        cpu_to_preemp = cpu;
      }
    }
    
    if (cpu_to_preemp != C_NIL)
    {
      thread = SCHEDULER_preemption (thread, cpu_to_preemp);
    }
    
    if (thread == TH_NIL)
    {
      return;
    }
  }

  /* Puede ser que haya cambiado de thread */
  sch_prio_fifo = (struct t_SCH_prio_fifo *) thread->sch_parameters;
  priority      = sch_prio_fifo->priority;

  if (thread->action == AC_NIL)
  {
    panic (
      "Thread P%02d T%02d (t%02d) to ready without actions\n",
      IDENTIFIERS (thread)
    );
  }

  if ((thread->loose_cpu) || (machine->scheduler.lost_cpu_on_send))
  {
    insert_queue (&(node->ready), (char *) thread, (t_priority) priority);
  }
  else
  {
    insert_first_queue (&(node->ready), (char *) thread, priority);
  }
}

t_micro
PRIO_FIFO_get_execution_time(struct t_thread *thread)
{
   struct t_action *action;
   t_micro         ex_time;

   action = thread->action;
   if (action->action != WORK)
      panic ("Trying to work when innaproppiate P%d T%d t%d", IDENTIFIERS (thread));
   ex_time = action->desc.compute.cpu_time;
   thread->action = action->next;
   freeame ((char *) action, sizeof (struct t_action));
   return (ex_time);
}

struct t_thread *
PRIO_FIFO_next_thread_to_run(struct t_node *node)
{
   return ((struct t_thread *) outFIFO_queue (&(node->ready)));
}

void
PRIO_FIFO_init_scheduler_parameters(struct t_thread *thread)
{
   struct t_SCH_prio_fifo *sch_prio_fifo;

   sch_prio_fifo = (struct t_SCH_prio_fifo *) mallocame (sizeof(struct t_SCH_prio_fifo));
   sch_prio_fifo->priority = thread->base_priority;
   thread->sch_parameters = (char *) sch_prio_fifo;
}

void
PRIO_FIFO_clear_parameters(struct t_thread *thread)
{
   struct t_SCH_prio_fifo *sch_prio_fifo;

   sch_prio_fifo = (struct t_SCH_prio_fifo *) thread->sch_parameters;
   sch_prio_fifo->priority = BASE_PRIO;
}

int
PRIO_FIFO_info(int info)
{
  return (0);
}

void
PRIO_FIFO_init (char *filename, struct t_machine *machine)
{
   assert(filename != NULL);
   assert(machine  != NULL);
}

void
PRIO_FIFO_copy_parameters(struct t_thread *th_o, struct t_thread *th_d)
{
   struct t_SCH_prio_fifo *or,
                  *de;

   or = (struct t_SCH_prio_fifo *) th_o->sch_parameters;
   de = (struct t_SCH_prio_fifo *) th_d->sch_parameters;
   de->priority = or->priority;
}

void
PRIO_FIFO_free_parameters(struct t_thread *thread)
{
   struct t_SCH_prio_fifo *sch_prio_fifo;

   sch_prio_fifo = (struct t_SCH_prio_fifo *) thread->sch_parameters;
   freeame ((char *) sch_prio_fifo, sizeof (struct t_SCH_prio_fifo));
}
