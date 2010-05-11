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
#include "fs.h"
#include "list.h"
#include "paraver.h"
#include "subr.h"
#include "paraver_pcf.h"

/* Incloc el fitxer on es defineix la taula i les macros
   de conversio de formats. */
#include <Dimemas2Prv.h>
/******************************************************/

#include <ParaverBinaryTrace.h>
#include "Macros.h"



int             PARAVER_cpu;
t_boolean       paraver_binary;
static int      paraver_line_number = 1;

static t_boolean generar_paraver=FALSE;
static BinHandle **bin_handles=NULL;



#define VERIFICA_GENERACIO_PARAVER {if (!generar_paraver) return;}


void
Activar_Generacio_Paraver()
{
  generar_paraver = TRUE;
}

void
Paraver_init()
{
  struct t_Ptask *ptask;
  struct t_task  *task;
  
  VERIFICA_GENERACIO_PARAVER;
  
  bin_handles = 
    (BinHandle**) malloc(count_queue(&Ptask_queue)*sizeof(BinHandle *));
  
  if (bin_handles == NULL)
  {
    panic("Not enough memory!\n");
  }

  for(
    ptask  = (struct t_Ptask *) head_queue (&(Ptask_queue));
    ptask != P_NIL;
    ptask  = (struct t_Ptask *) next_queue (&(Ptask_queue))
  )
  {
    ASSERT(ptask->Ptaskid>=1);
    
    bin_handles[ptask->Ptaskid-1] = 
      (BinHandle*) malloc(count_queue(&(ptask->tasks))*sizeof(BinHandle));
    
    if (bin_handles[ptask->Ptaskid-1]==NULL)
    {
      panic("Not enough memory!\n");
    }
    
    for(
      task  = (struct t_task*) head_queue (&(ptask->tasks));
      task != T_NIL;
      task  = (struct t_task*) next_queue (&(ptask->tasks))
    )
    {
      ASSERT(task->taskid>=1);
      bin_handles[ptask->Ptaskid-1][task->taskid-1] = 
        ParaverTrace_AllocBinaryFile();
    }
  }
}

void
Paraver_fini()
{
  int               ii;
  struct t_Ptask   *ptask;
  struct t_node    *node;
  ProcessHierarchy  ph;
  ResourceHierarchy rh;
  double            tmp1;
  dimemas_timer     tmp_timer;

  struct t_communicator *communicator;
  int num_tasks_comm, mida_tasks_comm = 0;
  int *global_ranks, *tasks_comm      = NULL;


  VERIFICA_GENERACIO_PARAVER;

  /* Cal muntar unes estructures temporals per poder generar la capçalera
   * de la traça paraver. */
  ph.nappls = count_queue(&Ptask_queue);
  ph.appls  = (Application *)malloc(ph.nappls*sizeof(Application));
  
  if (ph.appls == NULL)
  {
    panic("Not enough memory!\n");
  }     

  for(
    ptask  = (struct t_Ptask *) head_queue (&(Ptask_queue));
    ptask != P_NIL;
    ptask  = (struct t_Ptask *) next_queue (&(Ptask_queue))
  )
  {
    ASSERT(ptask->Ptaskid >= 1);
    ph.appls[ptask->Ptaskid-1].ntasks   = count_queue(&(ptask->tasks));
    ph.appls[ptask->Ptaskid-1].nthreads =
      (int*) malloc(ph.appls[ptask->Ptaskid-1].ntasks*sizeof(int));
    
    if (ph.appls[ptask->Ptaskid-1].nthreads == NULL)
    {
      panic("Not enough memory!\n");
    }
    
    /* Tots tenen un sol thread */
    for( ii = 0; ii < ph.appls[ptask->Ptaskid-1].ntasks; ii++)
    {
      ph.appls[ptask->Ptaskid-1].nthreads[ii] = 1;
    }
  }
  
  rh.nnodes = count_queue(&Node_queue);
  rh.ncpus  = (int*) malloc(rh.nnodes*sizeof(int));
  
  for(
    node  = (struct t_node*) head_queue (&Node_queue);
    node != N_NIL;
    node  = (struct t_node *) next_queue (&Node_queue)
  )
  {
    ASSERT(node->nodeid>=1);
    rh.ncpus[node->nodeid-1]=count_queue(&(node->Cpus));
  }

  /* Cal calcular el temps final */
  if ((paraver_initial_timer) || (paraver_final_timer))
  {
    if (paraver_final_timer)
    {
      tmp_timer = stop_paraver;
    }
    else
    {
      tmp_timer = execution_end_time;
    }
    SUB_TIMER (tmp_timer, start_paraver, tmp_timer);
  }
  else
  {
    tmp_timer = execution_end_time;
  }
  
  TIMER_TO_DOUBLE (tmp_timer, tmp1);

  /* Cal passar els comunicadors amb una altra estructura temporal */
  for (
    ptask  = (struct t_Ptask*) head_queue (&Ptask_queue);
    ptask != P_NIL;
    ptask  = (struct t_Ptask*) next_queue (&Ptask_queue)
  )
  {    
    if (count_queue(&ptask->Communicator) != 0)
    {
      for (
        communicator = (struct t_communicator*)head_queue(&ptask->Communicator);
        communicator!= COM_NIL;
        communicator = (struct t_communicator*)next_queue(&ptask->Communicator)
      )
      {
        num_tasks_comm = count_queue(&communicator->global_ranks);
        
        if (mida_tasks_comm < num_tasks_comm)
        {
          if (tasks_comm != NULL)
          {
            free(tasks_comm);
          }
          
          tasks_comm = (int*) malloc(num_tasks_comm*sizeof(int));
          
          if (tasks_comm == NULL)
          {
            panic("Not enough memory!\n");
          }
          mida_tasks_comm = num_tasks_comm;
        }

        for (
          ii = 0, global_ranks = (int*)head_queue(&communicator->global_ranks);
          global_ranks != (int*)0;
          ii++, global_ranks = (int *)next_queue(&communicator->global_ranks)
        )
        {
          tasks_comm[ii] = *global_ranks;
        }
        ParaverTrace_AddCommunicator(
          ptask->Ptaskid,
          communicator->communicator_id,
          num_tasks_comm,
          tasks_comm
        );
      }
    }
  }
  
  if (tasks_comm != NULL)
  {
    free(tasks_comm);
  }
  
  /* S'ajunten tots els fitxers temporals */
  ParaverTrace_MergeBinaryFiles (paraver_file, (long long) tmp1, &ph, &rh );

  /* S'allibera la memoria utilitzada */
  for(
    ptask  = (struct t_Ptask*) head_queue (&Ptask_queue);
    ptask != P_NIL;
    ptask = (struct t_Ptask *) next_queue (&Ptask_queue))
  {
    free(ph.appls[ptask->Ptaskid-1].nthreads);
    free(bin_handles[ptask->Ptaskid-1]);
  }

  free(ph.appls);
  free(bin_handles);
  
  /* Es genera el fitxer .pcf corresponent */
  MakeParaverPCF(paraver_file);
}

void 
Paraver_thread_buwa(
  int           cpu,
  int           ptask,
  int           task,
  int           thread,
  dimemas_timer init,
  dimemas_timer end
)
{
  double ini_time, end_time;

  VERIFICA_GENERACIO_PARAVER;

  if (paraver_different_waits == FALSE)
  {
    Paraver_thread_running(cpu, ptask, task, thread, init, end);
    return;
  }

  if EQ_TIMER (init, end)
  {
    return;
  }
  else
  {
    if (paraver_final_timer)
    {
      if (GT_TIMER (init, stop_paraver))
      {
        return;
      }
      
      if (GT_TIMER (end, stop_paraver))
      {
        ASS_ALL_TIMER (end, stop_paraver);
      }
    }
    
    if (paraver_initial_timer)
    {
      if ((GT_TIMER (start_paraver, init)) && (GT_TIMER (start_paraver, end)))
      {
        return;
      }
      if (GT_TIMER (start_paraver, init))
      {
        ASS_ALL_TIMER (init, start_paraver);
      }

      SUB_TIMER (init, start_paraver, init);
      SUB_TIMER (end, start_paraver, end);
    }
    
    TIMER_TO_DOUBLE (init, ini_time);
    TIMER_TO_DOUBLE (end, end_time);

    /*
    fprintf (
      paraver,
      PA_STATE_STRING,
      cpu, ptask, task, thread,
      ini_time, end_time,
      PRV_BLOCKED_ST
    );
    */
    
    if (debug&D_PRV)
    {
      PRINT_TIMER (current_time);
      printf (
        ": Paraver Busy Wait Printed\n\tOBJECT   CPU %d P%02d T%02d (t%02d)\n",
        cpu, ptask, task, thread
      );
      
      printf(
        "\tInitial Time: %.6f End Time: %.6f\n",
        init/1e6,
        end/1e6
      ); 
    }
    
    /* JGG: Ahora se usa BLOCKED (9) en vez de IO (12), hay que revisarlo */
    ParaverTrace_BinaryState(
      bin_handles[ptask-1][task-1],
      cpu, ptask, task, thread,
      (long long) ini_time,
      (long long) end_time,
      PRV_BLOCKED_ST
    );
  }
}

void 
Paraver_thread_wait_links(
  int           cpu,
  int           ptask,
  int           task,
  int           thread,
  dimemas_timer init,
  dimemas_timer end
)
{
  double ini_time, end_time;

  VERIFICA_GENERACIO_PARAVER;

  if (paraver_different_waits == FALSE)
  {
    Paraver_thread_running(cpu, ptask, task, thread, init, end);
    return;
  }

  if EQ_TIMER (init, end)
  {
    return;
  }
  else
  {
    if (paraver_final_timer)
    {
      if (GT_TIMER (init, stop_paraver))
      {
        return;
      }
      
      if (GT_TIMER (end, stop_paraver))
      {
        ASS_ALL_TIMER (end, stop_paraver);
      }
    }
    
    if (paraver_initial_timer)
    {
      if ((GT_TIMER (start_paraver, init)) && (GT_TIMER (start_paraver, end)))
      {
        return;
      }
      if (GT_TIMER (start_paraver, init))
      {
        ASS_ALL_TIMER (init, start_paraver);
      }

      SUB_TIMER (init, start_paraver, init);
      SUB_TIMER (end, start_paraver, end);
    }
    
    TIMER_TO_DOUBLE (init, ini_time);
    TIMER_TO_DOUBLE (end, end_time);

    /*
    fprintf (
      paraver,
      PA_STATE_STRING,
      cpu, ptask, task, thread,
      ini_time, end_time,
      PRV_BLOCKED_ST
    );
    */
    
    if (debug&D_PRV)
    {
      PRINT_TIMER (current_time);
      printf (
        ": Paraver Wait Links Printed\n\tOBJECT   CPU %d P%02d T%02d (t%02d)\n",
        cpu, ptask, task, thread
      );
      
      printf(
        "\tInitial Time: %.6f End Time: %.6f\n",
        init/1e6,
        end/1e6
      ); 
    }
    
    /* JGG: Ahora se usa BLOCKED (9) en vez de IO (12), hay que revisarlo */
    ParaverTrace_BinaryState(
      bin_handles[ptask-1][task-1],
      cpu, ptask, task, thread,
      (long long) ini_time,
      (long long) end_time,
      PRV_WAIT_LINKS_ST
    );
  }
}

/* JGG: OJO! Ahora está pintando un Test/Probe, mirar con calma cuando se usará
 * este estado */
void
Paraver_thread_consw(
  int           cpu,
  int           ptask,
  int           task,
  int           thread,
  dimemas_timer f,
  dimemas_timer g
)
{

  double  ini_time, end_time;

  VERIFICA_GENERACIO_PARAVER;

  if EQ_TIMER(f, g)
  {
    return;
  }
  else
  {
    if (paraver_final_timer)
    {
      if (GT_TIMER (f, stop_paraver))
      {
        return;
      }
      
      if (GT_TIMER (g, stop_paraver))
      {
        ASS_ALL_TIMER (g, stop_paraver);
      }
    }
    
    if (paraver_initial_timer)
    {
      if ((GT_TIMER (start_paraver, f)) && (GT_TIMER (start_paraver, g)))
      {
        return;
      }
      
      if (GT_TIMER (start_paraver, f))
      {
        ASS_ALL_TIMER (f, start_paraver);
      }

      SUB_TIMER (f, start_paraver, f);
      SUB_TIMER (g, start_paraver, g);
    }

    TIMER_TO_DOUBLE (f, ini_time);
    TIMER_TO_DOUBLE (g, end_time);
    
    if (debug&D_PRV)
    {
      PRINT_TIMER (current_time);
      printf (
        ": Paraver Test/Probe Printed\n\tOBJECT   CPU %d P%02d T%02d (t%02d)\n",
        cpu, ptask, task, thread
      );
      
      printf(
        "\tInitial Time: %.6f End Time: %.6f\n",
        f/1e6,
        g/1e6
      ); 
    }

    /*
    fprintf (
      paraver,
      PA_STATE_STRING,
      cpu, ptask, task, thread,
      ini_time, end_time,
      PRV_TEST_PROBE_ST
    );
    */

    ParaverTrace_BinaryState(
      bin_handles[ptask-1][task-1],
      cpu,
      ptask,
      task,
      thread,
      (long long) ini_time,
      (long long) end_time,
      PRV_TEST_PROBE_ST
    );
  }
}



void
Paraver_thread_running(
  int           cpu,
  int           ptask,
  int           task,
  int           thread,
  dimemas_timer f,
  dimemas_timer g
)
{

  double ini_time, end_time;

  VERIFICA_GENERACIO_PARAVER;

  if EQ_TIMER(f, g)
  {
    return;
  }
  else
  {
    if (paraver_final_timer)
    {
      if (GT_TIMER (f, stop_paraver))
      {
        return;
      }

      if (GT_TIMER (g, stop_paraver))
      {
        ASS_ALL_TIMER (g, stop_paraver);
      }
    }

    if (paraver_initial_timer)
    {
      if ((GT_TIMER (start_paraver, f)) && (GT_TIMER (start_paraver, g)))
      {
        return;
      }

      if (GT_TIMER (start_paraver, f))
      {
        ASS_ALL_TIMER (f, start_paraver);
      }
      
      SUB_TIMER (f, start_paraver, f);
      SUB_TIMER (g, start_paraver, g);
    }
    
    TIMER_TO_DOUBLE (f, ini_time);
    TIMER_TO_DOUBLE (g, end_time);
    
    if (debug&D_PRV)
    {
      PRINT_TIMER (current_time);
      printf (
        ": Paraver Running State Printed\n\tOBJECT   CPU %d P%02d T%02d (t%02d)\n",
        cpu, ptask, task,thread
      );
      
      printf(
        "\tInitial Time: %.6f End Time: %.6f\n",
        f/1e6,
        g/1e6
      ); 
    }


    /*
    fprintf (
      paraver,
      PA_STATE_STRING,
      cpu, ptask, task, thread,
      ini_time, end_time,
      PRV_RUNNING_ST
    );
    */

    ParaverTrace_BinaryState(
      bin_handles[ptask-1][task-1],
      cpu,
      ptask,
      task,
      thread,
      (long long) ini_time,
      (long long) end_time,
      PRV_RUNNING_ST
    );
  }
}


void 
Paraver_thread_startup(
  int           cpu,
  int           ptask,
  int           task,
  int           thread,
  dimemas_timer f, 
  dimemas_timer g
)
{
  double ini_time, end_time;

  VERIFICA_GENERACIO_PARAVER;

  if EQ_TIMER(f, g)
  {
    return;
  }
  else
  {
    if (paraver_final_timer)
    {
      if (GT_TIMER (f, stop_paraver))
      {
        return;
      }

      if (GT_TIMER (g, stop_paraver))
      {
        ASS_ALL_TIMER (g, stop_paraver);
      }
    }

    if (paraver_initial_timer)
    {
      if ((GT_TIMER (start_paraver, f)) && (GT_TIMER (start_paraver, g)))
      {
        return;
      }
      
      if (GT_TIMER (start_paraver, f))
      {
        ASS_ALL_TIMER (f, start_paraver);
      }
      
      SUB_TIMER (f, start_paraver, f);
      SUB_TIMER (g, start_paraver, g);
    }
 
    TIMER_TO_DOUBLE (f, ini_time);
    TIMER_TO_DOUBLE (g, end_time);
    
    if (debug&D_PRV)
    {
      PRINT_TIMER (current_time);
      printf (
        ": Paraver Startup Printed\n\tOBJECT   CPU %d P%02d T%02d (t%02d)\n",
        cpu, ptask, task,thread
      );
      
      printf(
        "\tInitial Time: %.6f End Time: %.6f\n",
        f/1e6,
        g/1e6
      ); 
    }

    
    /*
    fprintf(
      paraver,
      PA_STATE_STRING, 
      cpu, ptask, task, thread,
      ini_time, end_time,
      PRV_STARTUP_ST
    );
    */
    
    ParaverTrace_BinaryState(
      bin_handles[ptask-1][task-1],
      cpu, ptask, task, thread,
      (long long) ini_time,
      (long long) end_time,
      PRV_STARTUP_ST
    );

    /* FEC: Aixo s'haura de treure: */
#ifdef DEBUG_STARTUPS
    if ((end_time-ini_time)!=10000)
    {
      printf(
        "\tEstat PRV_STARTUP_ST %f -> %f = %f msec\n",
        ini_time,
        end_time,
        (end_time-ini_time)
      );
    }
#endif
    /********************************/
  }
}

void
Paraver_thread_data_copy(
  int           cpu,
  int           ptask,
  int           task,
  int           thread,
  dimemas_timer f, 
  dimemas_timer g
)
{
  double ini_time, end_time;

  VERIFICA_GENERACIO_PARAVER;

  if EQ_TIMER(f, g)
  {
    return;
  }
  else
  {
    if (paraver_final_timer)
    {
      if (GT_TIMER (f, stop_paraver))
      {
        return;
      }

      if (GT_TIMER (g, stop_paraver))
      {
        ASS_ALL_TIMER (g, stop_paraver);
      }
    }

    if (paraver_initial_timer)
    {
      if ((GT_TIMER (start_paraver, f)) && (GT_TIMER (start_paraver, g)))
      {
        return;
      }
      
      if (GT_TIMER (start_paraver, f))
      {
        ASS_ALL_TIMER (f, start_paraver);
      }
      
      SUB_TIMER (f, start_paraver, f);
      SUB_TIMER (g, start_paraver, g);
    }
 
    TIMER_TO_DOUBLE (f, ini_time);
    TIMER_TO_DOUBLE (g, end_time);
    
    if (debug&D_PRV)
    {
      PRINT_TIMER (current_time);
      printf (
        ": Paraver Data Copy Printed\n\tOBJECT   CPU %d P%02d T%02d (t%02d)\n",
        cpu, ptask, task,thread
      );
      
      printf(
        "\tInitial Time: %.6f End Time: %.6f\n",
        f/1e6,
        g/1e6
      ); 
    }

    
    /*
    fprintf(
      paraver,
      PA_STATE_STRING, 
      cpu, ptask, task, thread,
      ini_time, end_time,
      PRV_STARTUP_ST
    );
    */
    
    ParaverTrace_BinaryState(
      bin_handles[ptask-1][task-1],
      cpu, ptask, task, thread,
      (long long) ini_time,
      (long long) end_time,
      PRV_DATA_COPY_ST
    );

    /* FEC: Aixo s'haura de treure: */
#ifdef DEBUG_STARTUPS
    if ((end_time-ini_time)!=10000)
    {
      printf(
        "\tEstat PRV_DATA_COPY_ST %f -> %f = %f msec\n",
        ini_time,
        end_time,
        (end_time-ini_time)
      );
    }
#endif
    /********************************/
  }
}

void
Paraver_thread_round_trip(
  int           cpu,
  int           ptask,
  int           task,
  int           thread,
  dimemas_timer f, 
  dimemas_timer g
)
{
  double ini_time, end_time;

  VERIFICA_GENERACIO_PARAVER;

  if EQ_TIMER(f, g)
  {
    return;
  }
  else
  {
    if (paraver_final_timer)
    {
      if (GT_TIMER (f, stop_paraver))
      {
        return;
      }

      if (GT_TIMER (g, stop_paraver))
      {
        ASS_ALL_TIMER (g, stop_paraver);
      }
    }

    if (paraver_initial_timer)
    {
      if ((GT_TIMER (start_paraver, f)) && (GT_TIMER (start_paraver, g)))
      {
        return;
      }
      
      if (GT_TIMER (start_paraver, f))
      {
        ASS_ALL_TIMER (f, start_paraver);
      }
      
      SUB_TIMER (f, start_paraver, f);
      SUB_TIMER (g, start_paraver, g);
    }
 
    TIMER_TO_DOUBLE (f, ini_time);
    TIMER_TO_DOUBLE (g, end_time);
    
    if (debug&D_PRV)
    {
      PRINT_TIMER (current_time);
      printf (
        ": Paraver Startup Printed\n\tOBJECT   CPU %d P%02d T%02d (t%02d)\n",
        cpu, ptask, task,thread
      );
      
      printf(
        "\tInitial Time: %.6f End Time: %.6f\n",
        f/1e6,
        g/1e6
      ); 
    }

    ParaverTrace_BinaryState(
      bin_handles[ptask-1][task-1],
      cpu, ptask, task, thread,
      (long long) ini_time,
      (long long) end_time,
      PRV_RTT_ST
    );

    /* FEC: Aixo s'haura de treure: */
#ifdef DEBUG_STARTUPS
    if ((end_time-ini_time)!=10000)
    {
      printf(
        "\tState PRV_RTT_ST %f -> %f = %f msec\n",
        ini_time,
        end_time,
        (end_time-ini_time)
      );
    }
#endif
    /********************************/
  }
}

void 
Paraver_thread_dead(
  int cpu,
  int ptask,
  int task,
  int thread
)
{
  double        ini_time, end_time;
  dimemas_timer f;

  VERIFICA_GENERACIO_PARAVER;

  ASS_ALL_TIMER (f, current_time);

  if (paraver_final_timer)
  {
    if (GT_TIMER (f, stop_paraver))
    {      
      return;
    }
  }
  
  if (paraver_initial_timer)
  {
    if (GT_TIMER (start_paraver, f))
    {
      return;
    }

    SUB_TIMER (f, start_paraver, f);
  }
  
  
  TIMER_TO_DOUBLE (f, ini_time);
  end_time = ini_time + 1;
  
  if (debug&D_PRV)
  {
    PRINT_TIMER (current_time);
    printf (
      ": Paraver thread dead Printed\n\tOBJECT   CPU %d P%02d T%02d (t%02d)\n",
      cpu, ptask, task,thread
    );
  }
  
  /*
  fprintf (
    paraver,
    PA_STATE_STRING,
    cpu, ptask, task, thread,
    ini_time, ini_time+1,
    PRV_NOT_CREATED_ST
  );
  */

  ParaverTrace_BinaryState(
    bin_handles[ptask-1][task-1],
    cpu,
    ptask,
    task,
    thread,
    (long long) ini_time,
    (long long) end_time,
    PRV_NOT_CREATED_ST
  );
}

void
Paraver_thread_idle(
  int           cpu,
  int           ptask,
  int           task,
  int           thread,
  dimemas_timer f,
  dimemas_timer g
)
{
  double ini_time, end_time;

  VERIFICA_GENERACIO_PARAVER;

  if EQ_TIMER(f, g)
  {
    return;
  }
  
  if (paraver_final_timer)
  {
    if (GT_TIMER (f, stop_paraver))
    {
      return;
    }
    
    if (GT_TIMER (g, stop_paraver))
    {
      ASS_ALL_TIMER (g, stop_paraver);
    }
  }
 
  if (paraver_initial_timer)
  {
    if ((GT_TIMER (start_paraver, f)) && (GT_TIMER (start_paraver, g)))
    {
      return;
    }
    
    if (GT_TIMER (start_paraver, f))
    {
      ASS_ALL_TIMER (f, start_paraver);
    }
    
    SUB_TIMER (f, start_paraver, f);
    SUB_TIMER (g, start_paraver, g);
  }

  TIMER_TO_DOUBLE (f, ini_time);
  TIMER_TO_DOUBLE (g, end_time);
  
  if (debug&D_PRV)
  {
    PRINT_TIMER (current_time);
    printf (
      ": Paraver Idle State Printed\n\tOBJECT   CPU %d P%02d T%02d (t%02d)\n",
      cpu, ptask, task,thread
    );
    
    printf(
      "\tInitial Time: %.6f End Time: %.6f\n",
      f/1e6,
      g/1e6
    ); 
  }

  /*
    fprintf (
      paraver,
      PA_STATE_STRING,
      cpu, ptask, task, thread,
      ini_time, end_time,
      PRV_IDLE_ST
    );
  */
  
  ParaverTrace_BinaryState(
    bin_handles[ptask-1][task-1],
    cpu,
    ptask,
    task,
    thread,
    (long long) ini_time,
    (long long) end_time,
    PRV_IDLE_ST
  );
}


void 
Paraver_thread_wait(
  int           cpu,
  int           ptask,
  int           task,
  int           thread,
  dimemas_timer f,
  dimemas_timer g,
  int           type
)
{
  int tipus;
  double ini_time, end_time;

  VERIFICA_GENERACIO_PARAVER;


  if EQ_TIMER(f, g)
  {
    return;
  }

  if (paraver_final_timer)
  {
    if (GT_TIMER (f, stop_paraver))
    {
      return;
    }
    
    if (GT_TIMER (g, stop_paraver))
    {
      ASS_ALL_TIMER (g, stop_paraver);
    }
  }
  
  if (paraver_initial_timer)
  {
    if ((GT_TIMER (start_paraver, f)) && (GT_TIMER (start_paraver, g)))
    {
      return;
    }
    
    if (GT_TIMER (start_paraver, f))
    {
      ASS_ALL_TIMER (f, start_paraver);
    }

    SUB_TIMER (f, start_paraver, f);
    SUB_TIMER (g, start_paraver, g);
  }
  
  TIMER_TO_DOUBLE (f, ini_time);
  TIMER_TO_DOUBLE (g, end_time);
  
  if (paraver_different_waits)
  {
    tipus = type;
    /*
    fprintf (
      paraver,
      PA_STATE_STRING,
      cpu, ptask, task, thread,
      ini_time, end_time,
      type
    );
    */
  }
  else
  {
    tipus = PRV_IDLE_ST;
    /*
    fprintf (
      paraver,
      PA_STATE_STRING,
      cpu, ptask, task, thread,
      ini_time, end_time,
      PRV_IDLE_ST
    );
    */
  }
  
  if (debug&D_PRV)
  {
    PRINT_TIMER (current_time);
    printf (
      ": Paraver Wait State Printed\n\tOBJECT   CPU %d P%02d T%02d (t%02d)\n",
      cpu, ptask, task,thread
    );
    
    printf(
      "\tInitial Time: %.6f End Time: %.6f Type: %d\n",
      f/1e6,
      g/1e6,
      tipus
    ); 
  }

  ParaverTrace_BinaryState(
    bin_handles[ptask-1][task-1],
    cpu,
    ptask,
    task,
    thread,
    (long long) ini_time,
    (long long) end_time,
    tipus
  );
}



void
Paraver_thread_IO(
  int           cpu,
  int           ptask,
  int           task,
  int           thread,
  dimemas_timer f,
  dimemas_timer g
)
{
  double ini_time, end_time;

  VERIFICA_GENERACIO_PARAVER;

  if EQ_TIMER(f, g)
  {
    return;
  }

  if (paraver_final_timer)
  {
    if (GT_TIMER (f, stop_paraver))
    {
      return;
    }
    if (GT_TIMER (g, stop_paraver))
    {
      ASS_ALL_TIMER (g, stop_paraver);
    }
  }
  
  if (paraver_initial_timer)
  {
    if ((GT_TIMER (start_paraver, f)) && (GT_TIMER (start_paraver, g)))
    {
      return;
    }
    
    if (GT_TIMER (start_paraver, f))
    {
      ASS_ALL_TIMER (f, start_paraver);
    }

    SUB_TIMER (f, start_paraver, f);
    SUB_TIMER (g, start_paraver, g);
  }
  
  TIMER_TO_DOUBLE (f, ini_time);
  TIMER_TO_DOUBLE (g, end_time);
  /*
  fprintf (
    paraver,
    PA_STATE_STRING,
    cpu, ptask, task, thread,
    ini_time, end_time,
    PRV_IO_ST
  );
  */
  
  if (debug&D_PRV)
  {
    PRINT_TIMER (current_time);
    printf (
      ": Paraver IO State Printed\n\tOBJECT   CPU %d P%02d T%02d (t%02d)\n",
      cpu, ptask, task,thread
    );
    
    printf(
      "\tInitial Time: %.6f End Time: %.6f\n",
      f/1e6,
      g/1e6
    ); 
  }
  
  ParaverTrace_BinaryState(
    bin_handles[ptask-1][task-1],
    cpu, ptask, task, thread,
    (long long) ini_time,
    (long long) end_time,
    PRV_IO_ST
  );
}




void
Paraver_thread_block_IO(
  int           cpu,
  int           ptask,
  int           task,
  int           thread,
  dimemas_timer f,
  dimemas_timer g
)
{
  double ini_time, end_time;

  VERIFICA_GENERACIO_PARAVER;

  if EQ_TIMER(f, g)
  {
    return;
  }

  if (paraver_final_timer)
  {
    if (GT_TIMER (f, stop_paraver))
    {
      return;
    }
    
    if (GT_TIMER (g, stop_paraver))
    {
      ASS_ALL_TIMER (g, stop_paraver);
    }
  }
  
  if (paraver_initial_timer)
  {
    if ((GT_TIMER (start_paraver, f)) && (GT_TIMER (start_paraver, g)))
    {
      return;
    }
    
    if (GT_TIMER (start_paraver, f))
    {
      ASS_ALL_TIMER (f, start_paraver);
    }

    SUB_TIMER (f, start_paraver, f);
    SUB_TIMER (g, start_paraver, g);
  }
  
  TIMER_TO_DOUBLE (f, ini_time);
  TIMER_TO_DOUBLE (g, end_time);
  /*
  fprintf(
    paraver,
    PA_STATE_STRING,
    cpu, ptask, task, thread,
    ini_time, end_time,
    PRV_IO_ST
  );
  */
  
  if (debug&D_PRV)
  {
    PRINT_TIMER (current_time);
    printf (
      ": Paraver IO Block Printed\n\tOBJECT   CPU %d P%02d T%02d (t%02d)\n",
      cpu, ptask, task,thread
    );
    
    printf(
      "\tInitial Time: %.6f End Time: %.6f\n",
      f/1e6,
      g/1e6
    ); 
  }
  
  /* JGG: Antes se representaba por el estado 15, sin equivalente en el PCF, 
   * ahora se represeta por PRV_IO_ST (12) */
  ParaverTrace_BinaryState(
    bin_handles[ptask-1][task-1],
    cpu, ptask, task, thread,
    (long long) ini_time,
    (long long) end_time,
    PRV_IO_ST
  );
}




void
Paraver_thread_block_OS(
  int           cpu,
  int           ptask,
  int           task,
  int           thread,
  dimemas_timer f,
  dimemas_timer g
)
{
  double ini_time, end_time;

  VERIFICA_GENERACIO_PARAVER;

  if EQ_TIMER(f, g)
  {
    return;
  }

  if (paraver_final_timer)
  {
    if (GT_TIMER (f, stop_paraver))
    {
      return;
    }
    
    if (GT_TIMER (g, stop_paraver))
    {
      ASS_ALL_TIMER (g, stop_paraver);
    }
  }
  
  if (paraver_initial_timer)
  {
    if ((GT_TIMER (start_paraver, f)) && (GT_TIMER (start_paraver, g)))
    {
      return;
    }
    
    if (GT_TIMER (start_paraver, f))
    {
      ASS_ALL_TIMER (f, start_paraver);
    }

    SUB_TIMER (f, start_paraver, f);
    SUB_TIMER (g, start_paraver, g);
  }
  
  TIMER_TO_DOUBLE (f, ini_time);
  TIMER_TO_DOUBLE (g, end_time);
  
  /*
  fprintf(
    paraver,
    PA_STATE_STRING,
    cpu, ptask, task, thread,
    ini_time, end_time,
    PRV_BLOCKED_ST
  );
  */
  
  if (debug&D_PRV)
  {
    PRINT_TIMER (current_time);
    printf (
      ": Paraver OS Block Printed\n\tOBJECT   CPU %d P%02d T%02d (t%02d)\n",
      cpu, ptask, task,thread
    );
    
    printf(
      "\tInitial Time: %.6f End Time: %.6f\n",
      f/1e6,
      g/1e6
    ); 
  }
  
  ParaverTrace_BinaryState(
    bin_handles[ptask-1][task-1],
    cpu, ptask, task, thread,
    (long long) ini_time,
    (long long) end_time,
    PRV_BLOCKED_ST
  );
}




void
Paraver_comm(
  int           cpu_s,
  int           ptask_s,
  int           task_s,
  int           thread_s,
  dimemas_timer e,
  dimemas_timer e1,
  int           cpu_r,
  int           ptask_r,
  int           task_r,
  int           thread_r,
  dimemas_timer j,
  dimemas_timer j1,
  int           size,
  int           tag
)
{

  double log_s, phy_s, log_r, phy_r;

  VERIFICA_GENERACIO_PARAVER;

  if (paraver_final_timer)
  {
    if (GT_TIMER (e, stop_paraver))
    {
      return;
    }
    
    if (GT_TIMER (j, stop_paraver))
    {
      ASS_ALL_TIMER (j, stop_paraver);
    }
  }
  
  if (paraver_initial_timer)
  {
    if ((GT_TIMER (start_paraver, e)) && (GT_TIMER (start_paraver, j)))
    {
      return;
    }
    
    if (GT_TIMER (start_paraver, e))
    {
      ASS_ALL_TIMER (e, start_paraver);
    }
    
    if (GT_TIMER (start_paraver, j))
    {
      ASS_ALL_TIMER (j, start_paraver);
    }

    SUB_TIMER (e, start_paraver, e);
    SUB_TIMER (j, start_paraver, j);
  }
  
  if (paraver_final_timer)
  {
    if (GT_TIMER (e1, stop_paraver))
    {
      return;
    }
    
    if (GT_TIMER (j1, stop_paraver))
    {
      ASS_ALL_TIMER (j1, stop_paraver);
    }
  }
  
  if (paraver_initial_timer)
  {
    if ((GT_TIMER (start_paraver, e1)) && (GT_TIMER (start_paraver, j1)))
    {
      return;
    }
    
    if (GT_TIMER (start_paraver, e1))
    {
      ASS_ALL_TIMER (e1, start_paraver);
    }
    
    if (GT_TIMER (start_paraver, j1))
    {
      ASS_ALL_TIMER (j1, start_paraver);
    }

    SUB_TIMER (e1, start_paraver, e1);
    SUB_TIMER (j1, start_paraver, j1);
  }
  TIMER_TO_DOUBLE (e, log_s);
  TIMER_TO_DOUBLE (e1, phy_s);
  TIMER_TO_DOUBLE (j, log_r);
  TIMER_TO_DOUBLE (j1, phy_r);
  /* 
  fprintf (
    paraver,
    PA_COMM_STRING,
    cpu_s,
    ptask_s,
    task_s,
    thread_s,
    log_s,
    phy_s,
    cpu_r,
    ptask_r,
    task_r,
    thread_r,
    log_r,
    phy_r,
    size,
    tag
  ); 
  */

  /* JGG: DEBUG DE LA GENERACIÓN DE LAS COMUNICACIONES PARAVER */
  if (debug&D_PRV)
  {
    PRINT_TIMER (current_time);
    printf (
      ": Paraver Communication Printed\n\tSENDER   CPU %d P%02d T%02d (t%02d)\n",
      cpu_s, ptask_s, task_s,thread_s
    );
    
    printf(
      "\tLgcSend: %.6f PhySend: %.6f\n",
      log_s/1e6,
      phy_s/1e6
    ); 
    
    printf(
      "\tRECEIVER CPU %d P%02d T%02d (t%02d)\n",
      cpu_r, ptask_r, task_r, thread_r
    );
    
    printf(
      "\tLgcRecv: %.6f PhyRecv: %.6f\n",
      log_r/1e6,
      phy_r/1e6
    );
  }

  ParaverTrace_BinaryComm(
    bin_handles[ptask_s-1][task_s-1],
    cpu_s, ptask_s, task_s,thread_s,
    (long long) log_s, (long long) phy_s,
    cpu_r, ptask_r, task_r, thread_r,
    (long long) log_r, (long long) phy_r,
    (long long) size,
    (long long) tag 
  );
}

void
Paraver_trace_event(
  int           cpu,
  int           ptask,
  int           task,
  int           thread,
  dimemas_timer temps,
  long long     tipus,
  long long     valor,
  t_boolean     translate
)
{
  double    temps_final;
  long long tipus_final, valor_final;

  VERIFICA_GENERACIO_PARAVER;

  if (paraver_final_timer)
  {
    if (GT_TIMER (temps, stop_paraver))
    {
      return;
    }
  }
  
  if (paraver_initial_timer)
  {
    if (GT_TIMER (start_paraver, temps))
    {
      return;
    }

    SUB_TIMER (temps, start_paraver, temps);
  }
  
  TIMER_TO_DOUBLE (temps, temps_final);

  /* Aqui cal fer la conversió entre el tipus d'event de DIMEMAS i
   * el format de traça de paraver. */
 
  if ((translate) && ((tipus == BLOCK_BEGIN) || (tipus == BLOCK_END)))
  {
    /* Pot ser un block MPI o block de l'usuari 
    if ((valor >= 1) && (valor <= 128))
    {
      /* Es el començament o final d'un block MPI */
      /* S'assigna el valor i el tipus que correspon 
      BLOCK_TRF2PRV(valor, tipus_final, valor_final);
      if (tipus == BLOCK_END)
      {
        valor_final=0;
      }
    }
    else
    {
      /* Es el començament o final d'un block d'usuari */
      /* S'assigna el valor i el tipus que correspon 
      BLOCK_TRF2PRV(valor, tipus_final, valor_final);
      if (tipus == BLOCK_END)
      {
        valor_final = 0;
      }
    }
    */
    /* Now we use the function instead of the macro 
    BLOCK_TRF2PRV(valor, tipus_final, valor_final); */
      
    Block_Dimemas2Paraver_TypeAndValue(valor, &tipus_final, &valor_final);
    
    if (tipus == BLOCK_END)
    {
      valor_final=0;
    }
  }
  else
  {
    /* Segur que es un event independent totalment definit per l'usuari */
    tipus_final = tipus;
    valor_final = valor;
  }
  /* fprintf (paraver, PA_EVENT_STRING, cpu, ptask, task, thread,
           temps_final, tipus_final, valor_final); */
  
  if (debug&D_PRV)
  {
    PRINT_TIMER (current_time);
    printf (
      ": Paraver Event Printed\n\tOBJECT   CPU %d P%02d T%02d (t%02d)\n",
      cpu, ptask, task,thread
    );
    
    printf(
      "\tTime: %.6f Final Type: %lld Final Value: %lld\n",
      temps_final/1e6,
      tipus_final,
      valor_final
    ); 
  }

  ParaverTrace_BinaryEvent(
    bin_handles[ptask-1][task-1],
    cpu, ptask, task, thread,
    (long long) temps_final,
    (long long) tipus_final,
    (long long) valor_final
  );
}


void
Paraver_translate_event(
  int           cpu,
  int           ptask,
  int           task,
  int           thread,
  dimemas_timer temps,
  long long     tipus,
  long long     valor
)
{
  Paraver_trace_event(cpu, ptask, task, thread, temps, tipus, valor, TRUE);
}

void 
Paraver_event(
  int           cpu,
  int           ptask,
  int           task,
  int           thread,
  dimemas_timer temps,
  long long     tipus,
  long long     valor
)
{
  Paraver_trace_event(cpu, ptask, task, thread, temps, tipus, valor, FALSE);
}

void
Paraver_Global_Op (
  int           cpu,
  int           ptask,
  int           task,
  int           thread,
  dimemas_timer e,
  dimemas_timer f,
  dimemas_timer g,
  dimemas_timer h,
  int           commid,
  int           sendsize,
  int           recvsize,
  int           gOP,
  int           root
)
{
  double           time, tmp2, tmp3, tmp4;

  VERIFICA_GENERACIO_PARAVER;

  if (paraver_final_timer)
  {
    if (GT_TIMER (e, stop_paraver))
      return;
    if (GT_TIMER (f, stop_paraver))
      return;
    if (GT_TIMER (g, stop_paraver))
      return;
    if (GT_TIMER (h, stop_paraver))
      return;
  }
  
  if (paraver_initial_timer)
  {
    if (
      GT_TIMER (start_paraver, e) && 
      GT_TIMER (start_paraver, f) && 
      GT_TIMER (start_paraver, g) && 
      GT_TIMER (start_paraver, h)
    )
    {
      return;
    }
      
    if GT_TIMER (start_paraver, e)
      ASS_ALL_TIMER (e, start_paraver);
    
    if GT_TIMER (start_paraver, f)
      ASS_ALL_TIMER (f, start_paraver);
    
    if GT_TIMER (start_paraver, g)
      ASS_ALL_TIMER (g, start_paraver);
    
    if GT_TIMER (start_paraver, h)
      ASS_ALL_TIMER (h, start_paraver);

    SUB_TIMER (e, start_paraver, e);
    SUB_TIMER (f, start_paraver, f);
    SUB_TIMER (g, start_paraver, g);
    SUB_TIMER (h, start_paraver, h);
  }
  
  TIMER_TO_DOUBLE (e, time);
  TIMER_TO_DOUBLE (f, tmp2);
  TIMER_TO_DOUBLE (g, tmp3);
  TIMER_TO_DOUBLE (h, tmp4);

#ifdef FORMAT_VELL_RECORDS_4_PRV   
   /*fprintf (paraver, PA_GLOB_STRING, cpu, ptask, task, thread,
            time, tmp2, tmp3, tmp4, commid, sendsize, recvsize, gOP);*/
#else
   /*fprintf (paraver, PA_GLOB_STRING, cpu, ptask, task, thread,
            time, commid, sendsize, recvsize, gOP, root);*/
#endif /* FORMAT_VELL_RECORDS_4_PRV */

  ParaverTrace_BinaryGlobalComm(
    bin_handles[ptask-1][task-1],
    cpu, ptask, task, thread,
    (long long) time,
    commid,
    (long long) sendsize,
    (long long) recvsize,
    gOP,
    root
  );
}




static char *priorities_set = (char *) 0;

void
paraver_set_priorities(char *c)
{
  priorities_set = c;
}

/* Exit status */
#define USAGE 1
#define FILES_ACCES 2
#define RECORD_FORMAT 3

struct t_paradis_record pr;
static char     buf[BUFSIZE]; /*Abans era 2560 */

static void
to_bin (char *s, char *d)
{
   FILE           *sour,
                  *dest;
   int             i,
                   j;
   int             linenum = 1;
   int             cpu,
                   Ptask,
                   task,
                   thread;
   int             cpu_s,
                   Ptask_s,
                   task_s,
                   thread_s;
   int             cpu_r,
                   Ptask_r,
                   task_r,
                   thread_r;

   sour = MYFOPEN (s, "r");
   if (sour == (FILE *) 0)
   {
      printf ("Can't open source file %s\n", s);
      perror ((char *) 0);
      exit (FILES_ACCES);
   }
   dest = MYFOPEN (d, "w");
   if (dest == (FILE *) 0)
   {
      printf ("Can't open destination file %s\n", d);
      perror ((char *) 0);
      exit (FILES_ACCES);
   }

   i = fscanf (sour, "%[^\n]\n", buf);
   *buf = '%';
   fprintf (dest, "%s\n", buf);
   while (feof (sour) == FALSE)
   {
      linenum++;
      i = fscanf (sour, "%d:", &j);
      pr.record_type = (char) j;
      if (i == -1)
	 break;
      switch (j)
      {
	 case PARADIS_WORK:
	    i = fscanf (sour, "%d:%d:%d:%d:%le:%le:%d\n",
			&cpu,
			&Ptask,
			&task,
			&thread,
			&pr.d.work.begin,
			&pr.d.work.end,
			&pr.d.work.action);
	    if (i != 7)
	    {
	       printf ("Invalid record for work on line %d, file %s\n",
		       linenum, s);
	       exit (RECORD_FORMAT);
	    }
	    pr.d.work.cpu = (p_ids) cpu;
	    pr.d.work.Ptask = (p_ids) Ptask;
	    pr.d.work.task = (p_ids) task;
	    pr.d.work.thread = (p_ids) thread;
	    break;
	 case PARADIS_EVENT:
	    i = fscanf (sour, "%d:%d:%d:%d:%le:%d:%d\n",
			&cpu,
			&Ptask,
			&task,
			&thread,
			&pr.d.event.time,
			&pr.d.event.event,
			&pr.d.event.value);
	    if (i != 7)
	    {
	       printf ("Invalid record for event on line %d, file %s\n",
		       linenum, s);
	       exit (RECORD_FORMAT);
	    }
	    pr.d.event.cpu = (p_ids) cpu;
	    pr.d.event.Ptask = (p_ids) Ptask;
	    pr.d.event.task = (p_ids) task;
	    pr.d.event.thread = (p_ids) thread;
	    break;
	 case PARADIS_COMM:
	    i = fscanf (sour, "%d:%d:%d:%d:%le:%le:%d:%d:%d:%d:%le:%le:%d:%d\n",
			&cpu_s,
			&Ptask_s,
			&task_s,
			&thread_s,
			&pr.d.comm.log_s,
			&pr.d.comm.phys_s,
			&cpu_r,
			&Ptask_r,
			&task_r,
			&thread_r,
			&pr.d.comm.log_r,
			&pr.d.comm.phys_r,
			&pr.d.comm.size,
			&pr.d.comm.tag);
	    if (i != 14)
	    {
	       printf ("Invalid record for communication on line %d, file %s\n",
		       linenum, s);
	       exit (RECORD_FORMAT);
	    }
	    pr.d.comm.cpu_s = (p_ids) cpu_s;
	    pr.d.comm.Ptask_s = (p_ids) Ptask_s;
	    pr.d.comm.task_s = (p_ids) task_s;
	    pr.d.comm.thread_s = (p_ids) thread_s;
	    pr.d.comm.cpu_r = (p_ids) cpu_r;
	    pr.d.comm.Ptask_r = (p_ids) Ptask_r;
	    pr.d.comm.task_r = (p_ids) task_r;
	    pr.d.comm.thread_r = (p_ids) thread_r;
	    break;
	 default:
	    printf ("Invalid record in line %d, file %s: record type %d\n",
		    linenum, s, pr.record_type);
	    exit (3);
      }
      fwrite (&pr, sizeof (struct t_paradis_record), 1, dest);
   }
   fclose (sour);
   fclose (dest);
}


static char nom1[BUFSIZE]; /*Abans era 1250 */
static char nom2[BUFSIZE]; /*Abans era 1250 */
static char buffer[BUFSIZE]; /*Abans era 2560 */
static char buffer2[BUFSIZE]; /*Abans era 2560 */
static char content[BUFSIZE]; /*Abans era 2560 */


/*********************** AIXO JA NO S'UTILITZA ******************************/
#if 0
void
sort_paraver()
{
   char           *c,
                  *d;
   int             i;
   int             pid;
   time_t          h;
   int             DISKS = 0;

   struct t_Ptask *Ptask;
   struct t_task  *task;
   struct t_thread *thread;
   struct t_node  *node;
   double           tmp1;
   dimemas_timer   tmp_timer;
   int             tot_th;
   struct t_communicator *communicator;
   int             *global_ranks;

   DISKS = FS_paraver ();

   strcpy (buffer2, "");
   if ((paraver_initial_timer) || (paraver_final_timer))
   {
      if (paraver_final_timer)
        tmp_timer = stop_paraver;
      else
	    tmp_timer = execution_end_time;
      SUB_TIMER (tmp_timer, start_paraver, tmp_timer);
   }
   else
      tmp_timer = execution_end_time;
      
   TIMER_TO_DOUBLE (tmp_timer, tmp1);
   /* FEC: Suposo que PARAVER_cpu es el numero total de nodes
           perque abans, quan nomes hi havia una maquina, es
           feia aixo:
           PARAVER_cpu = machine->number_nodes;
    */
   PARAVER_cpu = count_queue(&Node_queue);
   
   if (DISKS == 0)
   {
      sprintf (buffer, "%.0f:%d(", tmp1, PARAVER_cpu);
      strcat (buffer2, buffer);
      for (node = (struct t_node *) head_queue (&Node_queue);
        node != N_NIL; )
      {
        sprintf (buffer, "%d", (int)count_queue (&(node->Cpus)));
        strcat (buffer2, buffer);
        node = (struct t_node *) next_queue (&Node_queue);
        if (node!=N_NIL)
          strcat (buffer2, ",");
      }
      sprintf (buffer, "):%d",count_queue (&Ptask_queue));
      strcat (buffer2, buffer);
   }
   else
   {
      sprintf (buffer, "%.0f:%d:%d", tmp1, PARAVER_cpu + 1,
	       count_queue (&Ptask_queue) + 1);
      strcat (buffer2, buffer);
   }
   for (Ptask = (struct t_Ptask *) head_queue (&Ptask_queue);
	Ptask != P_NIL;
	Ptask = (struct t_Ptask *) next_queue (&Ptask_queue))
   {
      i = count_queue (&(Ptask->tasks));
      sprintf (buffer, ":%d(", i);
      strcat (buffer2, buffer);
      for (task = (struct t_task *) head_queue (&(Ptask->tasks));
	   task != T_NIL;
	   task = (struct t_task *) next_queue (&(Ptask->tasks)))
      {
	 i--;
	 thread = (struct t_thread *) head_queue (&(task->threads));
	 node = get_node_of_thread (thread);
         tot_th = count_queue (&(task->threads));
         if (task->io_thread)
           tot_th++;
	 if (i > 0)
	    sprintf (buffer, "%d:%d,", tot_th, node->nodeid);
	 else
	    sprintf (buffer, "%d:%d)", tot_th, node->nodeid);
	 strcat (buffer2, buffer);
      }
      sprintf (buffer, ",%d", count_queue (&Ptask->Communicator));
      strcat (buffer2, buffer);
   }
   if (DISKS != 0)
   {
      sprintf (buffer, "%d(", DISKS);
      strcat (buffer2, buffer);
      for (i = 0; i < DISKS; i++)
      {
	 sprintf (buffer, "%d:%d", 1, PARAVER_cpu + 1);
	 strcat (buffer2, buffer);
	 if (i != DISKS - 1)
	    strcat (buffer2, ",");
      }
      strcat (buffer2, ")");
   }
   strcpy (content, buffer2);

   fclose (paraver);
   
 
   c = (char *) tempnam (".", nom1);
   i = rename (paraver_file, c);
   d = tempnam (".", nom2);
   if ((pid = fork ()) == 0)
   {
      i = MYOPEN (d, O_WRONLY | O_TRUNC | O_CREAT, 0600);
      dup2 (i, 1);
      close (i);
#ifndef linux
/*
      execlp ("sort", "paraver_sort", "-T", ".", "-t:", "+6n", "-7", "+5n",
	    "-6", "+4n", "-5", "+3n", "-4", "+2n", "-3", "+1r", "-2", "+0n",
	      c, (char *) 0);
*/
      execlp ("sort", "paraver_sort", "-T", ".", "-t:", "+6n", "-7", "+5n",
	    "-6", "+4n", "-5", "+3n", "-4", "+2n", "-3", "+0n", "-1",
	      c, (char *) 0);
#else
/*
      execlp ("sort", "paraver_sort", "-g", "-T", ".", "-t:", "+6", "-7", "+5",
	    "-6", "+4", "-5", "+3", "-4", "+2", "-3", "+1r", "-2", "+0",
	      c, (char *) 0);
*/
      execlp ("sort", "paraver_sort", "-g", "-T", ".", "-t:", "+6", "-7", "+5",
	    "-6", "+4", "-5", "+3", "-4", "+2", "-3", "+0", "-1",
	      c, (char *) 0);
#endif
   }
   if (pid == -1)
      panic ("Can't fork process to sort paraver file\n");
   while (pid != wait (&i));

   unlink (c);
   c = (char *) tempnam (".", nom1);
   if ((pid = fork ()) == 0)
   {
      i = MYOPEN (c, O_WRONLY | O_TRUNC | O_CREAT, 0600);
      dup2 (i, 1);
      close (i);
      i = MYOPEN (d, O_RDONLY);
      dup2 (i, 0);
      close (i);
   /* Sign */
      time (&h);
      strftime (buffer, 80, "%d/%m/%y at %H:%M", localtime (&h));
      if ((paraver_priorities) && (priorities_set != (char *) 0))
	 sprintf (buffer2, "#Paraver (%s):%s%s\n", buffer, content, priorities_set);
      else
	 sprintf (buffer2, "#Paraver (%s):%s\n", buffer, content);
      write (1, buffer2, strlen (buffer2));
      for (Ptask = (struct t_Ptask *) head_queue (&Ptask_queue);
	   Ptask != P_NIL;
	   Ptask = (struct t_Ptask *) next_queue (&Ptask_queue))
      {
        if (count_queue(&Ptask->Communicator)!=0)
        {
          for (communicator = (struct t_communicator *)head_queue(&Ptask->Communicator);
               communicator!= (struct t_communicator *)0;
               communicator = (struct t_communicator *)next_queue(&Ptask->Communicator))
          {
            sprintf (buffer2, "C:%d:%d:%d", 
                    Ptask->Ptaskid, communicator->communicator_id,
                    count_queue(&communicator->global_ranks));
            write (1, buffer2, strlen(buffer2));
            for (global_ranks = (int *)head_queue(&communicator->global_ranks);
                 global_ranks!= (int *)0;
                 global_ranks = (int *)next_queue(&communicator->global_ranks))
            {
              sprintf (buffer2, ":%d", *global_ranks);
              write (1, buffer2, strlen(buffer2));
            }
            write (1,"\n", 1);
          }
        }
      }
      

      execlp ("cut", "paraver_cut", "-d:", "-f2-20", (char *) 0);
   }


   if (pid == -1)
      panic ("Can't fork process to sort paraver file\n");
   while (pid != wait (&i));

   unlink (d);
   if (paraver_binary)
   {
      to_bin (c, paraver_file);
      unlink (c);
   }
   else
   {
      rename (c, paraver_file);
   }
   chmod (paraver_file, 0600);
   
   /* Es genera el fitxer .pcf corresponent */
   MakeParaverPCF(paraver_file);
}
#endif
/****************************************************************************/
