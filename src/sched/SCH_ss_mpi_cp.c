char SCH_ss_mpi_cp_c_rcsid[]="$Id: SCH_ss_mpi_cp.c,v 1.3 2010/12/21 16:46:31 paraver Exp $";
/*
 * Scheduling for MPI/SMPSs execution
 * It is fifo with priorities, with preempting for specific threads
 *
 * Vladimir Subotic
 *
 * (c) BSC-UPC 2010-2011
 *
 */

#include <assert.h>

/* Includes 'por defecto' */
#include "define.h"
#include "types.h"
#include "extern.h"

/* Include propio */
#include "SCH_ss_mpi_cp.h"

/* Dependencias con otros fuentes */
#include "cpu.h"
#include "list.h"
#include "mallocame.h"
#include "schedule.h"
#include "subr.h"

#include "simulator.h"
#include "machine.h"
#include "node.h"

void SS_MPI_CP_thread_to_ready(struct t_thread *thread)
{
  struct t_node          *node;
  struct t_cpu           *cpu, *cpu_to_preemp;
  struct t_SCH_ss_mpi_cp *sch_ss_mpi_cp, *sch_ss_mpi_cp_cur;
  t_priority              priority;
  t_priority              forces_preemption;
  struct t_thread        *thread_current;
  struct t_machine       *machine;
  t_boolean               preempted;

  node    = get_node_of_thread (thread);
  machine = node->machine;

  /* a little bit dirty
   * for this scheduler - make sure there is no preempting
   * if there are two consecutive CPU burst
   * so keep the same thread on the same cpu
   */
  if (thread->cpu != NULL) {
     inLIFO_queue (&(node->ready), (char *) thread);
     return;
  }


  preempted = FALSE;

  sch_ss_mpi_cp     = (struct t_SCH_ss_mpi_cp *) thread->sch_parameters;
  priority          = (t_priority) sch_ss_mpi_cp->priority;
  forces_preemption = (t_priority) sch_ss_mpi_cp->forces_preemption;

  if (
    (forces_preemption == 1) &&
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
      sch_ss_mpi_cp_cur =
        (struct t_SCH_ss_mpi_cp *) thread_current->sch_parameters;

      if (
         (sch_ss_mpi_cp_cur->priority > priority)
      )
      {
        priority      = sch_ss_mpi_cp_cur->priority;
        cpu_to_preemp = cpu;
      }
    }

    if (cpu_to_preemp != C_NIL)
    {
      thread = SCHEDULER_preemption (thread, cpu_to_preemp);
      /* thread is now the one that is removed from the CPU */
      preempted = TRUE;
    }

    if (thread == TH_NIL)
    {
      return;
    }
  }

  /* Puede ser que haya cambiado de thread */
  sch_ss_mpi_cp = (struct t_SCH_ss_mpi_cp *) thread->sch_parameters;
  priority      = sch_ss_mpi_cp->priority;

  if (thread->action == AC_NIL)
  {
    panic (
      "Thread P%02d T%02d (t%02d) to ready without actions\n",
      IDENTIFIERS (thread)
    );
  }

  if ((thread->loose_cpu) || (machine->scheduler.lost_cpu_on_send))
  {
     if (preempted) {
        /* this is the thread that is removed from CPU, put close to the top of the queue*/
        insert_queue (&(node->ready), (char *) thread, (t_priority) priority);
     } else {
        /* this is a new thread */
        if (priority == 0.0)
           /* if it is a task with MPI  -  put close to the top of the queue*/
           insert_first_queue (&(node->ready), (char *) thread, (t_priority) priority);
        else
           /* if it is an ordinary task  -  put close to the back of the queue*/
           insert_queue_from_back (&(node->ready), (char *) thread, (t_priority) priority);
     }
  }
  else
  {
    insert_first_queue (&(node->ready), (char *) thread, (t_priority) priority);
  }
}

t_nano
SS_MPI_CP_get_execution_time(struct t_thread *thread)
{
   struct t_action *action;
   t_nano         ex_time;

   action = thread->action;
   if (action->action != WORK)
      panic ("Trying to work when innaproppiate P%d T%d t%d", IDENTIFIERS (thread));
   ex_time = action->desc.compute.cpu_time;
   thread->action = action->next;
   MALLOC_free_memory ((char *) action, sizeof (struct t_action));
   return (ex_time);
}

struct t_thread *
SS_MPI_CP_next_thread_to_run(struct t_node *node)
{
   return ((struct t_thread *) outFIFO_queue (&(node->ready)));
}

void
SS_MPI_CP_init_scheduler_parameters(struct t_thread *thread)
{
   struct t_SCH_ss_mpi_cp *sch_ss_mpi_cp;

   sch_ss_mpi_cp = (struct t_SCH_ss_mpi_cp *) MALLOC_get_memory (sizeof(struct t_SCH_ss_mpi_cp));
//    sch_ss_mpi_cp->priority          = thread->base_priority;

   /* this is the highest priority that the thread can have
      in one of the first trace records this value is decresed */
   sch_ss_mpi_cp->priority          = 0;
   sch_ss_mpi_cp->forces_preemption = (t_priority) 0;
   thread->sch_parameters           = (char *) sch_ss_mpi_cp;
}

void
SS_MPI_CP_clear_parameters(struct t_thread *thread)
{
   struct t_SCH_ss_mpi_cp *sch_ss_mpi_cp;

   sch_ss_mpi_cp = (struct t_SCH_ss_mpi_cp *) thread->sch_parameters;
   sch_ss_mpi_cp->priority = BASE_PRIO;
   sch_ss_mpi_cp->forces_preemption = (t_priority) 0;

}

int
SS_MPI_CP_info(int info)
{
  return (0);
}

void
SS_MPI_CP_init (char *filename, struct t_machine *machine)
{
  assert(filename != NULL);
  assert(machine  != NULL);
}

void
SS_MPI_CP_copy_parameters(struct t_thread *th_o, struct t_thread *th_d)
{
   struct t_SCH_ss_mpi_cp *or, *de;

   or = (struct t_SCH_ss_mpi_cp *) th_o->sch_parameters;
   de = (struct t_SCH_ss_mpi_cp *) th_d->sch_parameters;
   de->priority = or->priority;
   de->forces_preemption = or->forces_preemption;
}

void
SS_MPI_CP_free_parameters(struct t_thread *thread)
{
   struct t_SCH_ss_mpi_cp *sch_ss_mpi_cp;

   sch_ss_mpi_cp = (struct t_SCH_ss_mpi_cp *) thread->sch_parameters;
   MALLOC_free_memory ((char *) sch_ss_mpi_cp, sizeof (struct t_SCH_ss_mpi_cp));
}

/* priority by which the thread is scheduled is
   1 / value_critical_path
*/

void
SS_MPI_CP_modify_priority(struct t_thread *thread, t_priority priority)
{
   struct t_SCH_ss_mpi_cp *sch_ss_mpi_cp;
   sch_ss_mpi_cp = (struct t_SCH_ss_mpi_cp *) thread->sch_parameters;
   sch_ss_mpi_cp->priority = priority;
}

void
SS_MPI_CP_modify_preemption(struct t_thread *thread, t_priority preemption)
{
   struct t_SCH_ss_mpi_cp *sch_ss_mpi_cp;
   sch_ss_mpi_cp = (struct t_SCH_ss_mpi_cp *) thread->sch_parameters;
   sch_ss_mpi_cp->forces_preemption = preemption;
}

