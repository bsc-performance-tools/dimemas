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
#include "extern.h"
#include "list.h"
#include "mallocame.h"
#include "paraver.h"
#include "schedule.h"
#include "semaphore.h"
#include "subr.h"

void
SEMAPHORE_general(int value, struct t_thread *thread)
{
   if (debug)
   {
      PRINT_TIMER (current_time);
      printf (": SEMAPHORE General %d, P%d T%d t%d\n",
	      value, IDENTIFIERS (thread));
   }
}

void
SEMAPHORE_init()
{
   struct t_Ptask *Ptask;
   struct t_task  *task;
   struct t_semaphore *sem;
   int             sem_id;

   if (debug)
   {
      PRINT_TIMER (current_time);
      printf (": SEMAPHORE initial routine called\n");
   }

   for (Ptask = (struct t_Ptask *) head_queue (&Ptask_queue);
	Ptask != P_NIL;
	Ptask = (struct t_Ptask *) next_queue (&Ptask_queue))
   {
      for (task = (struct t_task *) head_queue (&(Ptask->tasks));
	   task != T_NIL;
	   task = (struct t_task *) next_queue (&(Ptask->tasks)))
      {
	 create_queue (&(task->semaphores));
	 for (sem_id = 0; sem_id < MAX_SEMAPHORES; sem_id++)
	 {
	    sem = (struct t_semaphore *) mallocame (sizeof (struct t_semaphore));
	    sem->sem_id = sem_id + 1;
	    sem->counter = 0;
	    create_queue (&(sem->threads));

	    insert_queue (&(task->semaphores), (char *) sem, (t_priority) (sem_id + 1));
	 }
      }
   }
}

void
SEMAPHORE_end()
{
  struct t_Ptask     *Ptask;
  struct t_task      *task;
  struct t_semaphore *sem;
  struct t_thread    *thread;
  struct t_node      *node;

   if (debug)
   {
      PRINT_TIMER (current_time);
      printf (": SEMAPHORE final routine called\n");
   }

  for (
    Ptask  = (struct t_Ptask *) head_queue (&Ptask_queue);
    Ptask != P_NIL;
    Ptask  = (struct t_Ptask *) next_queue (&Ptask_queue)
  )
  {
    for (
      task  = (struct t_task *) head_queue (&(Ptask->tasks));
      task != T_NIL;
      task  = (struct t_task *) next_queue (&(Ptask->tasks))
    )
    {
      for (
        sem  = (struct t_semaphore *) head_queue (&(task->semaphores));
        sem != S_NIL;
        sem  = (struct t_semaphore *) next_queue (&(task->semaphores))
      )
      {
        if (empty_queue (&(sem->threads)) == FALSE)
        {
          if (debug)
          {
            PRINT_TIMER (current_time);
            printf (
              ": Warning SEMAPHORE end with %d threads waiting S%d, P%d T%d\n",
              count_queue (&(sem->threads)),
              sem->sem_id,
              Ptask->Ptaskid,
              task->taskid
            );
          }
          for (
            thread  = (struct t_thread *) head_queue (&(sem->threads));
            thread != TH_NIL;
            thread  = (struct t_thread *) next_queue (&(sem->threads))
          )
          {
            node = get_node_of_thread (thread);
            /* JGG: OJO! Aquí este Wait no tiene mucho sentido ... */
            Paraver_thread_wait (
              0,
              IDENTIFIERS (thread),
              thread->last_paraver,
              current_time,
              PRV_WAIT_ST
            );
          }
        }
      }
    }
  }
}

void
SEMAPHORE_signal(int sem_id, struct t_thread *thread)
{
   struct t_semaphore *sem;
   struct t_task  *task;
   struct t_thread *awaked;
   struct t_node  *node;
   struct t_cpu *cpu;

   node = get_node_of_thread (thread);
   cpu = get_cpu_of_thread (thread);
#ifdef PARAVER_ALL
   Paraver_event (cpu->unique_number, IDENTIFIERS (thread), current_time,
		  PARAVER_SEM_SIGNAL, sem_id);
#endif

   task = thread->task;
   sem = (struct t_semaphore *) query_prio_queue (&(task->semaphores), (t_priority) sem_id);
   if (sem == S_NIL)
   {
      panic ("Invalid semaphore identification %d\n", sem_id);
   }
   if (sem->counter >= 0)
   {

   /*
    * No threads waiting for this signal
    */

      sem->counter++;
      if (debug)
      {
	 PRINT_TIMER (current_time);
	 printf (": SEMAPHORE signal sem %d to %d by P%d T%d t%d\n",
		 sem_id, sem->counter, IDENTIFIERS (thread));
      }
   }
   else
   {

   /*
    * There is, at least, one thread waiting for this signal
    */
      sem->counter++;
      awaked = (struct t_thread *) outFIFO_queue (&(sem->threads));
      if (debug)
      {
	 PRINT_TIMER (current_time);
	 printf (": SEMAPHORE signal sem %d to %d by P%d T%d t%d awakes t%d\n",
	      sem_id, sem->counter, IDENTIFIERS (thread), awaked->threadid);
      }
      if (more_actions (thread))
      {
	 SCHEDULER_thread_to_ready (awaked);
      }
   }
}

void
SEMAPHORE_wait(int sem_id, struct t_thread *thread)
{
   struct t_task  *task;
   struct t_semaphore *sem;
   struct t_node  *node;
   struct t_cpu *cpu;

   node = get_node_of_thread (thread);
   cpu = get_cpu_of_thread (thread);
#ifdef PARAVER_ALL
   Paraver_event (cpu->unique_number, IDENTIFIERS (thread), current_time,
		  PARAVER_SEM_WAIT, sem_id);
#endif

   task = thread->task;

   task = thread->task;
   sem = (struct t_semaphore *) query_prio_queue (&(task->semaphores), (t_priority) sem_id);
   if (sem == S_NIL)
   {
      panic ("Invalid semaphore identification %d\n", sem_id);
   }

   if (sem->counter > 0)
   {

   /*
    * Free way
    */
      sem->counter--;
      if (debug)
      {
	 PRINT_TIMER (current_time);
	 printf (": SEMAPHORE wait sem %d to %d free pass to P%d T%d t%d\n",
		 sem_id, sem->counter, IDENTIFIERS (thread));
      }
      if (more_actions (thread))
      {
	 thread->loose_cpu = FALSE;
	 SCHEDULER_thread_to_ready (thread);
      }
   }
   else
   {

   /*
    * Must block
    */
      sem->counter--;
      if (debug)
      {
	 PRINT_TIMER (current_time);
	 printf (": SEMAPHORE wait sem %d to %d must wait P%d T%d t%d\n",
		 sem_id, sem->counter, IDENTIFIERS (thread));
      }
      Paraver_thread_running (cpu->unique_number, IDENTIFIERS (thread),
			   thread->last_paraver, current_time);
      thread->last_paraver = current_time;
      thread->loose_cpu = TRUE;
      inFIFO_queue (&(sem->threads), (char *) thread);
   }
}

void
SEMAPHORE_signal_n(int sem_id, int n, struct t_thread *thread)
{
   int             i;

   for (i = 0; i < n; i++)
      SEMAPHORE_signal (sem_id, thread);
}
