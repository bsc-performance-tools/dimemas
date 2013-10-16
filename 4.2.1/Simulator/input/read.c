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

#include <define.h>
#include <types.h>
#include <assert.h>

#include <dimemas_io.h>

#include "read.h"

// #include "file_data_access.h"

#include "node.h"

#include "cpu.h"
// #include "extern.h"
#include "list.h"
// #include "random.h"
//
// #include "schedule.h"
// #include "subr.h"
#include "task.h"
#include "communic.h"
#include "fs.h"
#include "yacc.h"
#include "paraver.h"

#if defined(OS_MACOSX) || defined(OS_CYGWIN)
#include "macosx_limits.h"
#else
#include <values.h> /* Ho necessito per la constant MAXDOUBLE */
#endif
#include <math.h>   /* Per l'arrel quadrada */

struct t_Ptask        *Ptask_current;
extern t_boolean       NEW_MODIFICATIONS;
int                    stop_restarts;
extern t_boolean       reload_while_longest_running;

int             greatest_cpuid   = 0;

dimemas_timer execution_end_time = 0; /* Temps final de l'execucio */

/* Function it_is_gzip_file
 * This function detects if current tracefile is compressed with GZIP
 *
 * @return TRUE value if current tracefile is compressed with GZIP
 */

static char const gz_magic[2] = {0x1f, 0x8b}; /* Needed to detect GZIP files */

static t_boolean it_is_gzip_file()
{
  t_boolean result = FALSE;
  char      magic[2];

  scanf ("%c%c", &magic[0], &magic[1]);

  if (magic[0] == gz_magic[0] && magic[1] == gz_magic[1])
  {
    result = TRUE;
  }
  else
  {
    result = FALSE;
  }
  return result;
}


/* This function detects if current tracefile is in new format or not */
t_boolean check_new_trace_format()
{
  struct t_Ptask* current_Ptask;
  char buffer[32];
  t_boolean new_format;

  for(current_Ptask  = (struct t_Ptask *) head_queue (&Ptask_queue);
      current_Ptask != NULL;
      current_Ptask  = (struct t_Ptask *) next_queue (&Ptask_queue))
  {
    FILE *trace_file;
    char *trace_name = current_Ptask->tracefile;

    if((trace_file = IO_fopen(trace_name, "r")) == NULL)
    {
#ifdef ENABLE_LARGE_TRACES
      panic ("\n\nTracefile %s can't be opened\n\n\n", trace_name);
#else
      panic ("\n\nTracefile %s can't be opened - if the file is larger than 2GBs   =>   configure Dimemas with --enable-large-traces\n\n\n", trace_name);
#endif
    }

    while(strlen(fgets(buffer, 32, trace_file)) == 1);

    if(strstr(buffer, "#DIMEMAS:") == NULL)
    {
      die("\n\nUnsupported tracefile %s of application %d. Please translate it using 'trf2dim'\n\n",
          trace_name,
          current_Ptask->Ptaskid);
    }
    IO_fclose(trace_file);
  }

  return TRUE;
}

static void Ptask_reload (struct t_Ptask *Ptask,
                          char           *TraceFile)
{

  struct t_task   *task;
  struct t_thread *thread;

  int tasks_it, threads_it;


  if (debug)
  {
    printf ("* RELOADing Ptask %02d\n", Ptask->Ptaskid);
  }

  DATA_ACCESS_reload_ptask (Ptask->Ptaskid);

  for (tasks_it = 0; tasks_it < Ptask->tasks_count; tasks_it++)
  {
    task = &(Ptask->tasks[tasks_it]);

    for (threads_it = 0; threads_it < task->threads_count; threads_it++ )
    {
      thread = task->threads[threads_it];
      assert(thread->action == NULL);
      READ_get_next_action(thread);
    }
  }

  return;
}

int        stop_restarts       = 0;
static int count_finished_apps = 0;
static int orig_apps_count     = 0;

void reload_new_Ptask (struct t_Ptask *Ptask)
{
  struct t_Ptask *P;
  int             i = 0;
  int             sin = 0;
  struct t_task   *task;
  struct t_thread *thread;
  struct t_cpu    *cpu;

  if (orig_apps_count == 0)
  {
    orig_apps_count = count_queue (&(Ptask_queue));
    if (debug) {
      PRINT_TIMER(current_time);
      printf(": RELOAD: # apps: %d\n", orig_apps_count);
    }
  }

  if (reload_while_longest_running && !stop_restarts)
  {
    if (Ptask->n_rerun == 0)
    {
      count_finished_apps++;
      if (debug)
      {
        PRINT_TIMER(current_time);
        printf(": RELOAD: # app %d finished; count_finished_apps=%d\n", Ptask->Ptaskid, count_finished_apps);
      }
    }

    if (count_finished_apps >= orig_apps_count)
    {
      if (debug) {
        printf("RELOAD: count_finished_apps=%d, orig_apps_count=%d - STOPPING RESTARTS\n", count_finished_apps, orig_apps_count);
      }
      stop_restarts = 1;
    }
  }

  for (P  = (struct t_Ptask *) head_queue (&Ptask_queue);
       P != P_NIL;
       P  = (struct t_Ptask *) next_queue (&Ptask_queue))
  {
    if (P->synthetic_application)
    {
      sin++;
      continue;
    }

    if (  (P->Ptaskid == Ptask->Ptaskid)  /* this solves a bug */
       && (P->n_rerun < reload_limit)  )
    {
      break;
    }
    i++;
  }

  if ((i == count_queue (&(Ptask_queue)) - sin) || stop_restarts)
  {
    if (EQ_0_TIMER(final_statistical_time))
    {
      ASS_ALL_TIMER (final_statistical_time, current_time);
    }
    return;
  }

  if (debug)
  {
    PRINT_TIMER (current_time);
    printf (": RELOAD  P%d iteration %d out of %d\n",
            Ptask->Ptaskid, Ptask->n_rerun + 1, reload_limit);
    PRINT_TIMER (current_time);
    printf (": RELOAD  i = %d, count_queue = %d, sin = %d\n", i, count_queue (&(Ptask_queue)), sin);
  }

  Ptask->n_rerun++;
  clear_last_actions (Ptask);

  Ptask_reload (Ptask, Ptask->tracefile); /* new format */

  SCHEDULER_reload (Ptask);
  reload_done = TRUE;

  /* Putting and initial event in every Ptask simulation */

  /* JGG (2012/01/13): new way to navigate through tasks
  for (task  = (struct t_task *) head_queue (&(Ptask->tasks));
       task != T_NIL;
       task  = (struct t_task *) next_queue (&(Ptask->tasks)))
  {
  */
  for (i = 0; i < Ptask->tasks_count; i++)
  {
    task = &(Ptask->tasks[i]);
    // thread = (struct t_thread *) head_queue (&(task->threads));


    // Thread 0 always exists */
    thread = task->threads[0];

    cpu = get_cpu_of_thread(thread);

    PARAVER_Event (cpu->unique_number,
                  Ptask->Ptaskid,
                  task->taskid,
                  thread->threadid,
                  current_time,
                  (long int)40000000,
                  (long int)Ptask->n_rerun);

  }
}

static void print_statistics(char             *buf,
                             struct t_account *account,
                             dimemas_timer     total_time)
{

  t_nano  cpu_time, latency_time, block_due_resources, block_time, ready_time,
           tot_time;

  TIMER_TO_FLOAT (account->cpu_time, cpu_time);
  TIMER_TO_FLOAT (account->latency_time, latency_time);
  TIMER_TO_FLOAT (account->block_due_resources, block_due_resources);
  TIMER_TO_FLOAT (account->time_waiting_for_message, block_time);
  TIMER_TO_FLOAT (account->time_ready_without_cpu, ready_time);
  TIMER_TO_FLOAT (total_time, tot_time);

  fprintf (salida_datos, "%s\t", buf);
  FPRINT_TIMER (salida_datos, account->cpu_time);

  fprintf (salida_datos, "\t%03.02f\t", cpu_time * 100 / tot_time);
  FPRINT_TIMER (salida_datos, account->time_waiting_for_message);

  fprintf (salida_datos, "\t%03.02f\t", block_time * 100 / tot_time);

  if (NEW_MODIFICATIONS)
  {
    FPRINT_TIMER (salida_datos, account->latency_time);
    fprintf (salida_datos, "\t%03.02f\t", latency_time * 100 / tot_time);

    FPRINT_TIMER (salida_datos, account->block_due_resources);
    fprintf (salida_datos, "\t%03.02f\t", block_due_resources * 100 / tot_time);
  }

  if ( full_out_info )
  {
    FPRINT_TIMER (salida_datos, account->time_ready_without_cpu);
    fprintf (salida_datos, "\t%03.02f\t", ready_time * 100 / tot_time);
  }

  fprintf (salida_datos, "%le\t%le\t", account->n_sends, account->n_bytes_send);

  fprintf (salida_datos,
           "%le\t%le\t%le",
           account->n_recvs_on_processor,
           account->n_recvs_must_wait,
           account->n_bytes_recv);

  fprintf (salida_datos, "%le\t", account->n_group_operations);
  FPRINT_TIMER (salida_datos, account->block_due_group_operations);

  fprintf (salida_datos, "\t");
  FPRINT_TIMER (salida_datos, account->group_operations_time);

  if (full_out_info)
  {
    fprintf (salida_datos, "\t%.0f",account->n_th_in_run);
  }
  fprintf (salida_datos,"\n");
}

static void show_individual_statistics (struct t_Ptask *Ptask)
{

  struct t_task  *task;
  struct t_thread *thread;
  struct t_account *acc_Ptask_sum, *acc_Ptask_min, *acc_Ptask_max, *account,
                   *acc_th;
  dimemas_timer   total_time;
  int             num_task;
  struct t_node  *node;
  char            buf[100];
  t_nano         tmp_micro;
  size_t          i;

  fprintf (salida_datos, "\nInformation for application %d\n", Ptask->Ptaskid);

  acc_Ptask_sum = new_accounter ();
  acc_Ptask_min = new_accounter ();
  acc_Ptask_max = new_accounter ();

  acc_Ptask_min->cpu_time = current_time;
  acc_Ptask_min->latency_time             = current_time;
  acc_Ptask_min->block_due_resources      = current_time;
  acc_Ptask_min->n_th_in_run              = INT_MAX;
  acc_Ptask_min->n_sends                  = INT_MAX;
  acc_Ptask_min->n_bytes_send             = INT_MAX;
  acc_Ptask_min->n_recvs                  = INT_MAX;
  acc_Ptask_min->n_bytes_recv             = INT_MAX;
  acc_Ptask_min->n_recvs_on_processor     = INT_MAX;
  acc_Ptask_min->n_recvs_must_wait        = INT_MAX;
  acc_Ptask_min->time_waiting_for_message = current_time;
  acc_Ptask_min->time_ready_without_cpu   = current_time;

  num_task = Ptask->tasks_count;

  ASS_TIMER (total_time, 0);

  /* JGG (2012/01/13): new way to navigate through tasks
  for (task  = (struct t_task *) head_queue (&(Ptask->tasks));
       task != T_NIL;
       task  = (struct t_task *) next_queue (&(Ptask->tasks)))
  {
  */
  for (i = 0; i < Ptask->tasks_count; i++)
  {
    task = &(Ptask->tasks[i]);
    // thread = (struct t_thread *) head_queue (&(task->threads));
    // Thread 0 always exists */
    thread = task->threads[0];

    if (reload_Ptasks==FALSE)
    {
      account = current_account (thread);
    }
    else
    {
      for (account  = (struct t_account*)head_queue (&(thread->account));
           account != ACC_NIL;
           account  = (struct t_account*)next_queue(&(thread->account)))
      {
        if (account->iteration == Ptask->n_rerun-1)
          break;
      }
    }
    MAX_TIMER (account->final_time, total_time, total_time);
  }

  if (EQ_0_TIMER(total_time))
    ASS_ALL_TIMER (total_time, current_time);

  if ((full_out_info) && (reload_Ptasks))
  {
    fprintf (salida_datos,
             "Total rerun %d and application time: ",
             Ptask->n_rerun-1);

    FPRINT_TIMER (salida_datos, total_time);
    TIMER_TO_FLOAT (total_time, tmp_micro);
    tmp_micro = tmp_micro / (Ptask->n_rerun-1);
    fprintf (salida_datos, "\tAverage time: %.6f\n",tmp_micro/1e9);
  }

  /* JGG (2012/01/13): new way to navigate through tasks
  for (task  = (struct t_task *) head_queue (&(Ptask->tasks));
       task != T_NIL;
       task  = (struct t_task *) next_queue (&(Ptask->tasks)))
  {
  */
  for (i = 0; i < Ptask->tasks_count; i++)
  {
    task = &(Ptask->tasks[i]);
    // thread = (struct t_thread *) head_queue (&(task->threads));


    // Thread 0 always exists */
    thread = task->threads[0];
    // thread = (struct t_thread *) head_queue (&(task->threads));

    if (reload_Ptasks == FALSE)
    {
      account = current_account (thread);
    }
    else
    {
      acc_th = new_accounter();

      for (account  = (struct t_account*) head_queue (&(thread->account));
           account != ACC_NIL;
           account  = (struct t_account*) next_queue (&(thread->account)))
      {
        if (account->iteration<Ptask->n_rerun)
        {
          add_account (acc_th, account);
        }
      }
      account = acc_th;
    }

    node = get_node_of_thread (thread);
    add_account (acc_Ptask_sum, account);
    min_account (acc_Ptask_min, account);
    max_account (acc_Ptask_max, account);

    if (full_out_info)
      sprintf (buf, "%d (%d)", task->taskid, node->nodeid );
    else
      sprintf (buf, "%d", task->taskid);

    print_statistics (buf, account, total_time);
  }

  fprintf (salida_datos, "\n");
  print_statistics ("Sum", acc_Ptask_sum, total_time);

  print_statistics ("Min", acc_Ptask_min, total_time);

  print_statistics ("Max", acc_Ptask_max, total_time);

  TIMER_TO_FLOAT (acc_Ptask_sum->cpu_time, tmp_micro);
  tmp_micro = tmp_micro / num_task;
  FLOAT_TO_TIMER (tmp_micro, acc_Ptask_sum->cpu_time);

  TIMER_TO_FLOAT (acc_Ptask_sum->latency_time, tmp_micro);
  tmp_micro = tmp_micro / num_task;
  FLOAT_TO_TIMER (tmp_micro, acc_Ptask_sum->latency_time);

  TIMER_TO_FLOAT (acc_Ptask_sum->block_due_resources, tmp_micro);
  tmp_micro = tmp_micro / num_task;
  FLOAT_TO_TIMER (tmp_micro, acc_Ptask_sum->block_due_resources);

  TIMER_TO_FLOAT (acc_Ptask_sum->time_waiting_for_message, tmp_micro);
  tmp_micro = tmp_micro / num_task;
  FLOAT_TO_TIMER (tmp_micro, acc_Ptask_sum->time_waiting_for_message);

  TIMER_TO_FLOAT (acc_Ptask_sum->time_ready_without_cpu, tmp_micro);
  tmp_micro = tmp_micro / num_task;
  FLOAT_TO_TIMER (tmp_micro, acc_Ptask_sum->time_ready_without_cpu);

  acc_Ptask_sum->n_sends =
    acc_Ptask_sum->n_sends / num_task;

  acc_Ptask_sum->n_bytes_send =
    acc_Ptask_sum->n_bytes_send / num_task;

  acc_Ptask_sum->n_recvs_on_processor =
    acc_Ptask_sum->n_recvs_on_processor / num_task;

  acc_Ptask_sum->n_recvs_must_wait =
    acc_Ptask_sum->n_recvs_must_wait / num_task;

  acc_Ptask_sum->n_bytes_recv =
    acc_Ptask_sum->n_bytes_recv / num_task;

  print_statistics ("Aver.", acc_Ptask_sum, total_time);
}

void calculate_execution_end_time()
{
  struct t_Ptask   *Ptask;
  struct t_task    *task;
  struct t_thread  *thread;
  struct t_account *account;
  size_t            i;

  for (Ptask  = (struct t_Ptask *) head_queue (&Ptask_queue);
       Ptask != P_NIL;
       Ptask  = (struct t_Ptask *) next_queue (&Ptask_queue))
  {
    ASS_TIMER (execution_end_time, 0);
    /* JGG (2012/01/13): new way to navigate through tasks
    for (task  = (struct t_task *) head_queue (&(Ptask->tasks));
         task != T_NIL;
         task  = (struct t_task *) next_queue (&(Ptask->tasks)))
    {
    */
    for (i = 0; i < Ptask->tasks_count; i++)
    {
      task = &(Ptask->tasks[i]);
      // thread = (struct t_thread *) head_queue (&(task->threads));


      // Thread 0 always exists */
      thread = task->threads[0];
      // thread = (struct t_thread *) head_queue (&(task->threads));

      account = current_account (thread);
      MAX_TIMER (account->final_time, execution_end_time, execution_end_time);
    }
  }

  if (EQ_0_TIMER(execution_end_time))
    ASS_ALL_TIMER (execution_end_time, current_time);
}

/*******************************************************************************
 * MACROS PARA LA FUNCION 'show_individual_statistics_pallas'
 ******************************************************************************/
#define SP_TOTAL_INDEX 0
#define SP_AVG_INDEX   1
#define SP_MAX_INDEX   2
#define SP_MIN_INDEX   3
#define SP_STDEV_INDEX 4

/* Macro que inicialitza totes les estadistiques totals */
#define SP_INIT_ESTADISTIQUES() \
{ \
  int i; \
  \
  bzero(totals,sizeof(double)*NUM_COLS_EST*NUM_ESTADISTICS); \
  for(i=0;i<NUM_COLS_EST;i++) \
  { \
    totals[i][SP_MIN_INDEX]=MAXDOUBLE; \
    totals[i][SP_MAX_INDEX]=-MAXDOUBLE; \
  } \
}

/* Macro que calcula totes les estadistiques d'una columna */
#define SP_ESTADISTIQUES(index,valor) \
{ \
  totals[index][SP_TOTAL_INDEX]+=(valor); \
  totals[index][SP_AVG_INDEX]++; /* De moment conta el numero de files */ \
  if ((valor)<totals[index][SP_MIN_INDEX]) \
    totals[index][SP_MIN_INDEX]=(valor); \
  if ((valor)>totals[index][SP_MAX_INDEX]) \
    totals[index][SP_MAX_INDEX]=(valor); \
  /* Acumulem els valors al quadrat */ \
  totals[index][SP_STDEV_INDEX] += ((valor)*(valor)); \
}

/* Macro que fa els ultims tractaments a totes les estadistiques totals */
#define SP_FINI_ESTADISTIQUES() \
{ \
  int i; \
  double num_files, aux; \
  \
  for(i=0;i<NUM_COLS_EST;i++) \
  { \
    num_files=totals[i][SP_AVG_INDEX]; \
    /* Es calcula la mitjana */ \
    totals[i][SP_AVG_INDEX]=totals[i][SP_TOTAL_INDEX] / num_files; \
    /* Es calcula la variança */ \
    totals[i][SP_STDEV_INDEX]/=num_files; \
    aux=(totals[i][SP_AVG_INDEX] * totals[i][SP_AVG_INDEX]); \
    totals[i][SP_STDEV_INDEX]-=aux; \
    /* Si el valor es -0 es passa a positiu */ \
    if (totals[i][SP_STDEV_INDEX]==0.0) \
      totals[i][SP_STDEV_INDEX]=0.0; \
    totals[i][SP_STDEV_INDEX]=sqrt(totals[i][SP_STDEV_INDEX]); \
  } \
}

static void show_individual_statistics_pallas (struct t_Ptask *Ptask)
{
  struct t_task  *task;
  struct t_thread *thread;
  struct t_account *acc_Ptask_sum, *acc_Ptask_min, *acc_Ptask_max, *account,
                   *acc_th, **acc_Ptask_rerun;
  dimemas_timer   total_time, comm_time;
  int             num_task;
  struct t_node  *node;
  t_nano         tmp_micro;
  float x,y,z;
  int i, j;
  dimemas_timer  t_io;

  /* FEC: Afegeixo aixo per tenir linies de totals de les estadistiques/task */
  #define NUM_COLS_EST 11
  #define NUM_ESTADISTICS 5

  char *totals_names[NUM_ESTADISTICS]={
    "Total",
    "Average",
    "Maximum",
    "Minimum",
    "Stdev"
  };

  /* Les linies de totals de les estadistiques */
  double totals[NUM_COLS_EST][NUM_ESTADISTICS];

  acc_Ptask_sum = new_accounter ();

  num_task = Ptask->tasks_count;

  ASS_TIMER (total_time, 0);

  /* JGG (2012/01/13): new way to navigate through tasks
  for (task  = (struct t_task *) head_queue (&(Ptask->tasks));
       task != T_NIL;
       task  = (struct t_task *) next_queue (&(Ptask->tasks)))
  {
  */
  for (i = 0; i < Ptask->tasks_count; i++)
  {
    task = &(Ptask->tasks[i]);
    // thread = (struct t_thread *) head_queue (&(task->threads));


    // Thread 0 always exists */
    thread = task->threads[0];
    // thread = (struct t_thread *) head_queue (&(task->threads));
    account = current_account (thread);
    MAX_TIMER (account->final_time, total_time, total_time);
  }

  if (EQ_0_TIMER(total_time))
    ASS_ALL_TIMER (total_time, current_time);

  /* JGG: Show an small App information */
  fprintf(salida_datos,
          "**** Application %d (%s) ****\n",
          Ptask->Ptaskid,
          Ptask->tracefile);

  /* JGG: For short out info, only 'total_time' is needed */
  if (short_out_info)
  {
    FPRINT_TIMER(salida_datos, total_time);
    fprintf(salida_datos, "\n");
    return;
  }

  /* FEC: Inicialitzo les estadistiques totals */
  SP_INIT_ESTADISTIQUES();

  /* Initialization of accounters per reload */
  /* We have (n_rerun + 1) iterations */
  acc_Ptask_rerun =
    (struct t_account **)malloc(sizeof(struct t_account *) * (Ptask->n_rerun + 1));
  for(i = 0; i <= Ptask->n_rerun; i++)
  {
    acc_Ptask_rerun[i] = new_accounter();
  }

  /* Calcul Estadistiques per iteracio i totals */
  int n_rerun;
  /* JGG (2012/01/13): new way to navigate through tasks
  for (task  = (struct t_task *) head_queue (&(Ptask->tasks));
       task != T_NIL;
       task  = (struct t_task *) next_queue (&(Ptask->tasks)))
  {
  */
  for (i = 0; i < Ptask->tasks_count; i++)
  {
    task = &(Ptask->tasks[i]);
    // thread = (struct t_thread *) head_queue (&(task->threads));


    // Thread 0 always exists */
    thread = task->threads[0];
    // thread = (struct t_thread *) head_queue (&(task->threads));

    n_rerun = 0;
    for(account  = (struct t_account*)head_queue (&(thread->account));
        account != ACC_NIL;
        account  = (struct t_account*)next_queue(&(thread->account)))
    {

        add_account(acc_Ptask_rerun[n_rerun], account);
        n_rerun++;
        add_account(acc_Ptask_sum, account);

     }
  }

  /* Estadistiques per iteracio */
  if(Ptask->n_rerun > 0)
  {
    fprintf(salida_datos, "\n**** Statistics by iteration ****\n\n");
    for(n_rerun = 0; n_rerun <= Ptask->n_rerun; n_rerun++)
    {
      fprintf(salida_datos,"Execution time Iteration %d:\t", n_rerun);

      FPRINT_TIMER (salida_datos, (acc_Ptask_rerun[n_rerun]->final_time - acc_Ptask_rerun[n_rerun]->initial_time));

      TIMER_TO_FLOAT(acc_Ptask_rerun[n_rerun]->cpu_time, x);
      TIMER_TO_FLOAT((acc_Ptask_rerun[n_rerun]->final_time - acc_Ptask_rerun[n_rerun]->initial_time), y);
      fprintf (salida_datos,"\nSpeedup Iteration %d:\t\t%.2f \n",n_rerun, x/y);

      fprintf (salida_datos,"CPU Time Iteration %d:\t\t", n_rerun);
      FPRINT_TIMER (salida_datos, acc_Ptask_rerun[n_rerun]->cpu_time);
      fprintf (salida_datos,"\n\n");
    }
  }

  /* Estadistiques totals */
  fprintf(salida_datos, "\n**** Total Statistics ****\n\n");
  fprintf (salida_datos,"Execution time:\t");
  FPRINT_TIMER (salida_datos, total_time);

  TIMER_TO_FLOAT(acc_Ptask_sum->cpu_time, x);
  TIMER_TO_FLOAT(total_time, y);
  fprintf (salida_datos,"\nSpeedup:\t%.2f \n",x/y);

  /* FEC: Mostro el temps de cpu total */
  fprintf (salida_datos,"CPU Time:\t");
  FPRINT_TIMER (salida_datos, acc_Ptask_sum->cpu_time);
  fprintf (salida_datos,"\n\n");


  fprintf (salida_datos," Id.\tComputation\t%%time\tCommunication\n");
  /* JGG (2012/01/13): new way to navigate through tasks
  for (task  = (struct t_task *) head_queue (&(Ptask->tasks));
       task != T_NIL;
       task  = (struct t_task *) next_queue (&(Ptask->tasks)))
  {
  */
  for (i = 0; i < Ptask->tasks_count; i++)
  {
    task = &(Ptask->tasks[i]);
    // thread = (struct t_thread *) head_queue (&(task->threads));


    // Thread 0 always exists */
    thread = task->threads[0];
    //thread = (struct t_thread *) head_queue (&(task->threads));

    account = current_account (thread);
    node    = get_node_of_thread (thread);
    fprintf (salida_datos, "%3d\t", task->taskid);

    /* Computation time */
    FPRINT_TIMER (salida_datos, account->cpu_time);

    TIMER_TO_FLOAT(account->cpu_time, x);

    /* Communications time */
    TIMER_TO_FLOAT(account->latency_time, y);
    TIMER_TO_FLOAT(account->time_waiting_for_message,z);
    y = y+z;
    TIMER_TO_FLOAT(account->block_due_group_operations, z);
    y = y+z;
    TIMER_TO_FLOAT(account->group_operations_time, z);
    y = y+z;
    FLOAT_TO_TIMER(y, comm_time);

    fprintf (salida_datos, "\t%03.2f\t", (float)x*100/(x+y));
    FPRINT_TIMER (salida_datos, comm_time);
    fprintf (salida_datos,"\n");
  }

  fprintf (salida_datos,"\n");

  /* FEC: Afegeixo mes informacio */
  /*
  fprintf (salida_datos," Id.\tMess.sent\tBytes sent\tImmediate recv\tWaiting recv\tBytes recv\tColl.op.\tBlock time\tComm. time\n");*/

  fprintf (salida_datos,
           " Id.\tMess.sent\tBytes sent\tImmediate recv\tWaiting recv");
  fprintf (salida_datos,
           "\tBytes recv\tColl.op.\tBlock time\tComm. time\tWait link time");
  fprintf (salida_datos,
           "\tWait buses time\tI/O time\n");

  /* JGG (2012/01/13): new way to navigate through tasks
  for (task  = (struct t_task *) head_queue (&(Ptask->tasks));
       task != T_NIL;
       task  = (struct t_task *) next_queue (&(Ptask->tasks)))
  {
  */
  for (i = 0; i < Ptask->tasks_count; i++)
  {
    task = &(Ptask->tasks[i]);
    // thread = (struct t_thread *) head_queue (&(task->threads));


    // Thread 0 always exists */
    thread = task->threads[0];
    //thread = (struct t_thread *) head_queue (&(task->threads));

    account = current_account (thread);
    node = get_node_of_thread (thread);
    fprintf (salida_datos, "%3d\t%le\t%le\t",
             task->taskid,
             account->n_sends, account->n_bytes_send);

    fprintf (salida_datos, "%le\t%le\t%le",
             account->n_recvs_on_processor,
             account->n_recvs_must_wait,
             account->n_bytes_recv);

    fprintf (salida_datos, "\t%le\t",
             account->n_group_operations);

    FPRINT_TIMER (salida_datos, account->block_due_group_operations);
    fprintf (salida_datos, "\t");
    FPRINT_TIMER (salida_datos, account->group_operations_time);

    /* FEC: Afegeixo mes informacio */
    fprintf (salida_datos, "\t");
    FPRINT_TIMER (salida_datos, account->block_due_link);
    fprintf (salida_datos, "\t");
    FPRINT_TIMER (salida_datos, account->block_due_buses);

    fprintf (salida_datos, "\t");
    /* Si s'han comptat junts a la trac,a� trf els reads i writes, no s'han de
     * sumar perque ja s'han comptat junts tamb�al simular. */
    /* ADD_TIMER(account->io_read_time,account->io_write_time,t_io); */
    ASS_ALL_TIMER(t_io,account->io_call_time);
    FPRINT_TIMER (salida_datos, t_io);

    fprintf (salida_datos, "\n");

    /* FEC: Calculo les estadistiques totals d'aquesta fila */
    SP_ESTADISTIQUES(0,account->n_sends);
    SP_ESTADISTIQUES(1,account->n_bytes_send);
    SP_ESTADISTIQUES(2,account->n_recvs_on_processor);
    SP_ESTADISTIQUES(3,account->n_recvs_must_wait);
    SP_ESTADISTIQUES(4,account->n_bytes_recv);
    SP_ESTADISTIQUES(5,account->n_group_operations);
    TIMER_TO_FLOAT(account->block_due_group_operations,z);
    SP_ESTADISTIQUES(6,z);
    TIMER_TO_FLOAT(account->group_operations_time,z);
    SP_ESTADISTIQUES(7,z);
    TIMER_TO_FLOAT(account->block_due_link,z);
    SP_ESTADISTIQUES(8,z);
    TIMER_TO_FLOAT(account->block_due_buses,z);
    SP_ESTADISTIQUES(9,z);
    TIMER_TO_FLOAT(t_io,z);
    SP_ESTADISTIQUES(10,z);

  }

  /* FEC: Acabo les estadistiques totals i les mostro */
  SP_FINI_ESTADISTIQUES();
  fprintf(salida_datos,"----");

  for (j=0; j<NUM_COLS_EST; j++)
    fprintf(salida_datos,"----------------");

  fprintf(salida_datos,"\n");

  for (i=0; i<NUM_ESTADISTICS; i++)
  {
    fprintf(salida_datos,"%s\t",totals_names[i]);

    for (j=0; j<NUM_COLS_EST; j++)
    {
      fprintf(salida_datos,"%le\t",totals[j][i]);
    }
    fprintf(salida_datos,"\n");
  }

}

void show_statistics (int pallas_output)
{
 struct t_Ptask *Ptask;
 t_nano         total_time;

 if (pallas_output)
 {
   for (Ptask  = (struct t_Ptask *) head_queue (&Ptask_queue);
        Ptask != P_NIL;
        Ptask = (struct t_Ptask *) next_queue (&Ptask_queue))
   {
     show_individual_statistics_pallas(Ptask);
   }
 }
 else
 {
   TIMER_TO_FLOAT (current_time, total_time);

   fprintf (salida_datos, "Final time: ");
   FPRINT_TIMER (salida_datos, current_time);
   fprintf (salida_datos, "\n");
   for (Ptask  = (struct t_Ptask *) head_queue (&Ptask_queue);
        Ptask != P_NIL;
        Ptask = (struct t_Ptask *) next_queue (&Ptask_queue))
   {
     show_individual_statistics (Ptask);
   }
 }
}

void MAIN_TEST_print_communicator(struct t_communicator* comm)
{
  char*           item = NULL;
  int*            task = NULL;
  struct t_queue* rank = NULL;

  rank = &comm->global_ranks;

  printf("Comunicator [%02d]: ", comm->communicator_id);

  // Print prcedure of global ranks
  item = head_queue(rank);
  while(item != NULL) {
    task = (int*)item;

    printf("%02d ", *task);

    item = next_queue(rank);
  }

  printf("\n");
}

static long living_actions = 0;

void READ_get_next_action(struct t_thread *thread)
{
  int Ptask_id, task_id, thread_id;
  struct t_action *new_action, *new_action_aux;

  Ptask_id  = thread->task->Ptask->Ptaskid;
  task_id   = thread->task->taskid;
  thread_id = thread->threadid;

  /* Free previous action! */
  if (thread->action != NULL)
  { // When app (re)starts, it could be NULL
    assert(thread->action != NULL);
    new_action_aux = thread->action->next;

    READ_free_action(thread->action);

    // free(thread->action);


    thread->action = new_action_aux;
  }

  if (!DATA_ACCESS_get_next_action(Ptask_id,
                                   task_id,
                                   thread_id,
                                  &new_action))
  {
    panic(DATA_ACCESS_get_error());
  }

  living_actions++;

  if (new_action == AC_NIL)
  {
    /* DEBUG
    fprintf(stdout,"NULL ACTION on %d %d %d\n", Ptask_id, task_id, thread_id); */
    assert(thread->action == NULL);
    thread->action = AC_NIL;
    return;
  }

  if (new_action->action == WORK)
  {
    // JGG: NOW, time is ALWAYS in NANOSECONDS!!
    new_action->desc.compute.cpu_time *= 1e9;

    //Vladimir added to multiply with the relative cpu speed

    struct t_node *node;
    node = get_node_of_thread(thread);
    if (node->relative <= 0)
    {
      new_action->desc.compute.cpu_time = 0;
    }
    else
    {
      new_action->desc.compute.cpu_time /= node->relative;
    }


    if (PREEMP_enabled)
    {
      new_action->desc.compute.cpu_time += PREEMP_overhead(thread->task);
    }

    /* DEBUG
    struct t_module* mod;
    t_priority identificator = 0;

    mod = (struct t_module*) query_prio_queue(&Ptask_current->Modules, (t_priority)identificator);

    if (mod == M_NIL)
    {
      mod = (struct t_module*) malloc (sizeof(struct t_module));

      mod->identificator = identificator;
      mod->ratio         = 1.0;
      mod->block_name    = (char*)0;
      mod->activity_name = (char*)0;
      mod->src_file      = -1;
      mod->src_line      = -1;

      insert_queue (&Ptask_current->Modules, (char *)mod, (t_priority) identificator);
    }
    // inLIFO_queue (&(thread->modules), (char *)mod);
    */

    recompute_work_upon_modules(thread, new_action);
  }

  assert(thread->action == NULL);
  thread->action = new_action;

  /* Ensure last action is not a MPI IO action */
  new_action = thread->action;
  assert(thread->action != NULL);
  /*if (new_action == AC_NIL)
  {
    return;
  }*/

  while (new_action->next != AC_NIL)
  {
    new_action = new_action->next;
  }

  if ((new_action->action == MPI_IO))
  {
    new_action_aux = thread->action;

    if (DATA_ACCESS_get_next_action(Ptask_id,
                                    task_id,
                                    thread_id,
                                    &new_action) == FALSE)
    {
      panic(DATA_ACCESS_get_error());
    }

    living_actions++;

    new_action->next = thread->action;
    thread->action = new_action_aux;
  }

  return;
}

void READ_create_action(struct t_action **action)
{
  if ( action == NULL )
  {
    panic("Wrong parameter when creating a new action");
  }

  (*action) = (struct t_action*) malloc(sizeof(struct t_action));

  if ( (*action) == NULL )
  {
    panic("Unable to get memory for new actions!");
  }

  living_actions++;

  return;
}

void READ_copy_action(struct t_action *src_action,
                      struct t_action *dst_action)
{
  memcpy (dst_action, src_action, sizeof(struct t_action));

  return;
}

void READ_free_action(struct t_action *action)
{
  if (action != NULL)
  {
    free(action);
    living_actions--;
  }
}

long READ_get_living_actions(void)
{
  return living_actions;
}
