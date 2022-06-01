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
#include <math.h>
#include <paraver.h>
#include <schedule.h>
#include <semaphore.h>
#include <subr.h>
#include <types.h>

void SEMAPHORE_Init()
{
  struct t_Ptask *Ptask;
  struct t_task *task;
  struct t_semaphore *sem;
  int sem_id;
  size_t tasks_it;

  info( "-> Loading initial semaphores status\n" );


  for ( Ptask = (struct t_Ptask *)head_queue( &Ptask_queue ); Ptask != P_NIL; Ptask = (struct t_Ptask *)next_queue( &Ptask_queue ) )
  {
    /* JGG (2012/01/12): new way to navigate through tasks
    for (task = (struct t_task *) head_queue (&(Ptask->tasks));
         task != T_NIL;
         task = (struct t_task *) next_queue (&(Ptask->tasks)))
    */
    for ( tasks_it = 0; tasks_it < Ptask->tasks_count; tasks_it++ )
    {
      task = &( Ptask->tasks[ tasks_it ] );

      create_queue( &( task->semaphores ) );
      for ( sem_id = 0; sem_id < MAX_SEMAPHORES; sem_id++ )
      {
        sem          = (struct t_semaphore *)malloc( sizeof( struct t_semaphore ) );
        sem->sem_id  = sem_id + 1;
        sem->counter = 0;

        create_queue( &( sem->threads ) );
        insert_queue( &( task->semaphores ), (char *)sem, ( t_priority )( sem_id + 1 ) );
      }
    }
  }
}

void SEMAPHORE_End()
{
  struct t_Ptask *Ptask;
  struct t_task *task;
  struct t_semaphore *sem;
  struct t_thread *thread;
  struct t_node *node;

  size_t tasks_it;

  /*
   if (debug)
   {
      PRINT_TIMER (current_time);
      printf (": SEMAPHORE final routine called\n");
   }
  */

  for ( Ptask = (struct t_Ptask *)head_queue( &Ptask_queue ); Ptask != P_NIL; Ptask = (struct t_Ptask *)next_queue( &Ptask_queue ) )
  {
    /* JGG (2012/01/12): new way to navigate through tasks
    for (task = (struct t_task *) head_queue (&(Ptask->tasks));
         task != T_NIL;
         task = (struct t_task *) next_queue (&(Ptask->tasks)))
    */
    for ( tasks_it = 0; tasks_it < Ptask->tasks_count; tasks_it++ )
    {
      task = &( Ptask->tasks[ tasks_it ] );

      for ( sem = (struct t_semaphore *)head_queue( &( task->semaphores ) ); sem != S_NIL;
            sem = (struct t_semaphore *)next_queue( &( task->semaphores ) ) )
      {
        if ( empty_queue( &( sem->threads ) ) == FALSE )
        {
          if ( debug )
          {
            PRINT_TIMER( current_time );
            printf( ": Warning SEMAPHORE end with %d threads waiting S%d, P%d T%d\n",
                    count_queue( &( sem->threads ) ),
                    sem->sem_id,
                    Ptask->Ptaskid,
                    task->taskid );
          }
          for ( thread = (struct t_thread *)head_queue( &( sem->threads ) ); thread != TH_NIL;
                thread = (struct t_thread *)next_queue( &( sem->threads ) ) )
          {
            node = get_node_of_thread( thread );
            /* JGG: OJO! Aquí este Wait no tiene mucho sentido ... */
            PARAVER_Wait( 0, IDENTIFIERS( thread ), thread->last_paraver, current_time, PRV_WAIT_ST );
          }
        }
      }
    }
  }
}

void SEMAPHORE_signal( int sem_id, struct t_thread *thread )
{
  struct t_semaphore *sem;
  struct t_task *task;
  struct t_thread *awaked;
  struct t_node *node;
  struct t_cpu *cpu;

  node = get_node_of_thread( thread );
  cpu  = get_cpu_of_thread( thread );

  task = thread->task;
  sem  = (struct t_semaphore *)query_prio_queue( &( task->semaphores ), (t_priority)sem_id );
  if ( sem == S_NIL )
  {
    panic( "Invalid semaphore identification %d\n", sem_id );
  }
  if ( sem->counter >= 0 )
  {
    /*
     * No threads waiting for this signal
     */

    sem->counter++;
    if ( debug )
    {
      PRINT_TIMER( current_time );
      printf( ": SEMAPHORE signal sem %d to %d by P%d T%d t%d\n", sem_id, sem->counter, IDENTIFIERS( thread ) );
    }
  }
  else
  {
    /*
     * There is, at least, one thread waiting for this signal
     */
    sem->counter++;
    awaked = (struct t_thread *)outFIFO_queue( &( sem->threads ) );
    if ( debug )
    {
      PRINT_TIMER( current_time );
      printf( ": SEMAPHORE signal sem %d to %d by P%d T%d t%d awakes t%d\n", sem_id, sem->counter, IDENTIFIERS( thread ), awaked->threadid );
    }
    if ( more_actions( thread ) )
    {
      SCHEDULER_thread_to_ready( awaked );
    }
  }
}

void SEMAPHORE_wait( int sem_id, struct t_thread *thread )
{
  struct t_task *task;
  struct t_semaphore *sem;
  struct t_node *node;
  struct t_cpu *cpu;

  node = get_node_of_thread( thread );
  cpu  = get_cpu_of_thread( thread );

  task = thread->task;

  task = thread->task;
  sem  = (struct t_semaphore *)query_prio_queue( &( task->semaphores ), (t_priority)sem_id );
  if ( sem == S_NIL )
  {
    panic( "Invalid semaphore identification %d\n", sem_id );
  }

  if ( sem->counter > 0 )
  {
    /* Free way  */
    sem->counter--;

    if ( debug )
    {
      PRINT_TIMER( current_time );
      printf( ": SEMAPHORE wait sem %d to %d free pass to P%d T%d t%d\n", sem_id, sem->counter, IDENTIFIERS( thread ) );
    }

    if ( more_actions( thread ) )
    {
      thread->loose_cpu = FALSE;
      SCHEDULER_thread_to_ready( thread );
    }
  }
  else
  {
    /* Must block */
    sem->counter--;
    if ( debug )
    {
      PRINT_TIMER( current_time );
      printf( ": SEMAPHORE wait sem %d to %d must wait P%d T%d t%d\n", sem_id, sem->counter, IDENTIFIERS( thread ) );
    }

    PARAVER_Running( cpu->unique_number, IDENTIFIERS( thread ), thread->last_paraver, current_time );

    thread->last_paraver = current_time;
    thread->loose_cpu    = TRUE;
    inFIFO_queue( &( sem->threads ), (char *)thread );
  }
}

void SEMAPHORE_signal_n( int sem_id, int n, struct t_thread *thread )
{
  int i;

  for ( i = 0; i < n; i++ )
    SEMAPHORE_signal( sem_id, thread );
}
