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

#include <dimemas_io.h>

/* Include propio */
#include "SCH_svr4.h"

/* Dependencias con otros fuentes */
#include "cpu.h"
#include "extern.h"
#include "list.h"
#include "paraver.h"
#include "schedule.h"
#include "subr.h"

static tsdpent_t           config_ts_dptbl[] = {
/*  glbpri   qntm   tqexp   slprt   mxwt   lwt  */
    0,       200000,     0,     50,     0,      50,
    1,       200000,     0,     50,     0,      50,
    2,       200000,     0,     50,     0,      50,
    3,       200000,     0,     50,     0,      50,
    4,       200000,     0,     50,     0,      50,
    5,       200000,     0,     50,     0,      50,
    6,       200000,     0,     50,     0,      50,
    7,       200000,     0,     50,     0,      50,
    8,       200000,     0,     50,     0,      50,
    9,       200000,     0,     50,     0,      50,
   10,       160000,     0,     51,     0,      51,
   11,       160000,     1,     51,     0,      51,
   12,       160000,     2,     51,     0,      51,
   13,       160000,     3,     51,     0,      51,
   14,       160000,     4,     51,     0,      51,
   15,       160000,     5,     51,     0,      51,
   16,       160000,     6,     51,     0,      51,
   17,       160000,     7,     51,     0,      51,
   18,       160000,     8,     51,     0,      51,
   19,       160000,     9,     51,     0,      51,
   20,       120000,    10,     52,     0,      52,
   21,       120000,    11,     52,     0,      52,
   22,       120000,    12,     52,     0,      52,
   23,       120000,    13,     52,     0,      52,
   24,       120000,    14,     52,     0,      52,
   25,       120000,    15,     52,     0,      52,
   26,       120000,    16,     52,     0,      52,
   27,       120000,    17,     52,     0,      52,
   28,       120000,    18,     52,     0,      52,
   29,       120000,    19,     52,     0,      52,
   30,        80000,    20,     53,     0,      53,
   31,        80000,    21,     53,     0,      53,
   32,        80000,    22,     53,     0,      53,
   33,        80000,    23,     53,     0,      53,
   34,        80000,    24,     53,     0,      53,
   35,        80000,    25,     54,     0,      54,
   36,        80000,    26,     54,     0,      54,
   37,        80000,    27,     54,     0,      54,
   38,        80000,    28,     54,     0,      54,
   39,        80000,    29,     54,     0,      54,
   40,        40000,    30,     55,     0,      55,
   41,        40000,    31,     55,     0,      55,
   42,        40000,    32,     55,     0,      55,
   43,        40000,    33,     55,     0,      55,
   44,        40000,    34,     55,     0,      55,
   45,        40000,    35,     56,     0,      56,
   46,        40000,    36,     57,     0,      57,
   47,        40000,    37,     58,     0,      58,
   48,        40000,    38,     58,     0,      58,
   49,        40000,    39,     58,     0,      58,
   50,        40000,    40,     58,     0,      58,
   51,        40000,    41,     58,     0,      58,
   52,        40000,    42,     58,     0,      58,
   53,        40000,    43,     58,     0,      58,
   54,        40000,    44,     58,     0,      58,
   55,        40000,    45,     58,     0,      58,
   56,        40000,    46,     58,     0,      58,
   57,        40000,    47,     58,     0,      58,
   58,        40000,    48,     58,     0,      58,
   59,        20000,    49,     59, 32000,      59,
};

#define BEST_PRIO 59

static svr4_t sch_svr4_params[MAX_MACHINES];

#define SCH_SVR4_PARAMS sch_svr4_params[machine->id]

#include "simulator.h"
#include "machine.h"
#include "node.h"

void
svr4_thread_to_ready(struct t_thread *thread)
{
  struct t_node    *node;
  tsproc_t         *sch_par, *sch_par_cur;
  struct t_thread  *thread_current;
  int               priority;
  struct t_cpu     *cpu, *cpu_to_preempt;
  struct t_machine *machine;


  sch_par = (tsproc_t *)thread->sch_parameters;
  node    = get_node_of_thread (thread);
  cpu     = get_cpu_of_thread(thread);
  machine = node->machine;

  sch_par = (tsproc_t *) thread->sch_parameters;

  if (sch_par->full_quantum)
  {
    if (sch_par->ts_umdpri!=config_ts_dptbl[sch_par->ts_umdpri].ts_tqexp)
    {
      sch_par->ts_umdpri=config_ts_dptbl[sch_par->ts_umdpri].ts_tqexp;

      PARAVER_Event (
        cpu->unique_number,
        IDENTIFIERS (thread),
        current_time,
        120,
        sch_par->ts_umdpri
      );
    }
    sch_par->full_quantum=FALSE;
  }

  if (machine->scheduler.priority_preemptive)
  {
    if (select_free_cpu (node, thread) == C_NIL)
    {
      priority       = sch_par->ts_umdpri;
      cpu_to_preempt = C_NIL;

      for (
        cpu  = (struct t_cpu *)head_queue (&(node->Cpus));
        cpu != C_NIL;
        cpu  = (struct t_cpu *)next_queue (&(node->Cpus))
      )
      {
        thread_current = cpu->current_thread;
        sch_par_cur    = (tsproc_t*) thread_current->sch_parameters;
        if (priority<sch_par_cur->ts_umdpri)
        {
          priority       = sch_par_cur->ts_umdpri;
          cpu_to_preempt = cpu;
        }
      }

      if (cpu_to_preempt != C_NIL)
      {
        thread = SCHEDULER_preemption (thread, cpu_to_preempt);
      }

      if (thread == TH_NIL)
      {
        return;
      }

      sch_par = (tsproc_t *) thread->sch_parameters;
    }
  }

  if (thread->loose_cpu)
  {
    insert_queue(&(node->ready), (void*) thread, BEST_PRIO-sch_par->ts_umdpri);
  }
  else
  {
    insert_first_queue(
      &(node->ready),
      (void*) thread, BEST_PRIO-sch_par->ts_umdpri
    );
  }
}

t_nano
svr4_get_execution_time(struct t_thread *thread)
{
  struct t_action *action;
  t_nano         ex_time;
  tsproc_t *sch_par;
  struct t_node *node = get_node_of_thread (thread);
  struct t_machine *machine;

  machine = node->machine;

  sch_par = (tsproc_t *)thread->sch_parameters;

  action = thread->action;

  if ((sch_par->last_quantum<
       config_ts_dptbl[sch_par->ts_umdpri].ts_quantum*
       SCH_SVR4_PARAMS.quantum_factor/100.0)
      && (thread->loose_cpu==FALSE))
  {
    ex_time = MIN (action->desc.compute.cpu_time,
		   config_ts_dptbl[sch_par->ts_umdpri].ts_quantum*SCH_SVR4_PARAMS.quantum_factor/100.0-
		   sch_par->last_quantum);
    sch_par->last_quantum += ex_time;
  }
  else
  {
    ex_time = MIN (action->desc.compute.cpu_time,
		   config_ts_dptbl[sch_par->ts_umdpri].ts_quantum*SCH_SVR4_PARAMS.quantum_factor/100.0);
    sch_par->last_quantum = ex_time;
  }

  if (sch_par->last_quantum == config_ts_dptbl[sch_par->ts_umdpri].ts_quantum*SCH_SVR4_PARAMS.quantum_factor/100.0)
    sch_par->full_quantum = TRUE;

  action->desc.compute.cpu_time -= ex_time;
  if (action->desc.compute.cpu_time == 0)
  {
    thread->action = action->next;
    READ_free_action(action);
  }
  return (ex_time);
}

struct t_thread *
svr4_next_thread_to_run(struct t_node *node)
{
  return ((struct t_thread *) outFIFO_queue (&(node->ready)));
}

void
svr4_init_scheduler_parameters(struct t_thread *thread)
{
  tsproc_t *sch_par;
  struct t_node *node = get_node_of_thread (thread);
  struct t_cpu *cpu;

  sch_par = (tsproc_t *) malloc (sizeof(tsproc_t));
  sch_par->ts_umdpri = BEST_PRIO;
  sch_par->ts_timeleft = 0;
  sch_par->ts_dispwait = 0;
  sch_par->full_quantum = FALSE;
  sch_par->last_quantum = 0;
  thread->sch_parameters = (char *)sch_par;

  cpu = get_cpu_of_thread(thread);
  PARAVER_Event (cpu->unique_number, IDENTIFIERS (thread),
                 current_time, 120, sch_par->ts_umdpri);
}

void
svr4_clear_parameters(struct t_thread *thread)
{
  tsproc_t *sch_par;

  sch_par = (tsproc_t *)thread->sch_parameters;
  sch_par->ts_umdpri = BEST_PRIO;
  sch_par->ts_timeleft = 0;
  sch_par->ts_dispwait = 0;
}

int
svr4_info(int info, struct t_thread *thread_s, struct t_thread *thread_r)
{
  tsproc_t *sch_par;
  struct t_node *node;
  struct t_cpu *cpu;

  assert(thread_s != NULL);

  if (info==SCH_INFO_RECV_MISS)
  {
    node = get_node_of_thread (thread_r);
    sch_par = (tsproc_t *)thread_r->sch_parameters;
    if (sch_par->ts_umdpri!=config_ts_dptbl[sch_par->ts_umdpri].ts_slpret)
    {
      sch_par->ts_umdpri = config_ts_dptbl[sch_par->ts_umdpri].ts_slpret;
      cpu = get_cpu_of_thread(thread_r);
      PARAVER_Event (cpu->unique_number, IDENTIFIERS (thread_r),
		     current_time, 120, sch_par->ts_umdpri);
   }
  }
  return (0);
}

void
svr4_init(char *filename, struct t_machine *machine)
{
  FILE *fp;
  int j;
  char buf[256];
  char str[256];
  double fl;


  bzero(&SCH_SVR4_PARAMS,sizeof(svr4_t));

  if (filename==(char *)0)
    return;

  if (strcmp(filename,"")==0)
    return;


  fp = IO_fopen (filename,"r");
  if (fp==(FILE *)0)
  {
    if (filename!=(char *)0)
      fprintf(stderr,"Can open Unix SVR4 configuration file %s\n",filename);
    return;
  }

  fgets (str, 256, fp);
  j = sscanf (str, "Policy: %s", buf);
  if (j!=1)
    panic ("Invalid format in file %s.\nInvalid policy name %s\n",
	   filename, buf);

  fgets (str, 256, fp);
  j = sscanf (str, "Context switch cost (seconds): %le",
	      &SCH_SVR4_PARAMS.context_switch);
  if ((j!=1) || (SCH_SVR4_PARAMS.context_switch<0))
    panic ("Invalid format in file %s.\nInvalid context switch cost %f\n",
	   filename, SCH_SVR4_PARAMS.context_switch);
  machine->scheduler.context_switch = SCH_SVR4_PARAMS.context_switch*1e9;

  fgets (str, 256, fp);
  j = sscanf (str, "Busy wait (seconds):  %le",
	      &SCH_SVR4_PARAMS.busy_wait);
  if ((j!=1) || (SCH_SVR4_PARAMS.busy_wait<0))
    panic ("Invalid format in file %s.\nInvalid busy wait %f\n",
	   filename, SCH_SVR4_PARAMS.busy_wait);
  machine->scheduler.busywait_before_block = SCH_SVR4_PARAMS.busy_wait*1e9;

  fgets (str, 256, fp);
  j = sscanf (str, "Priority Preemptive (YES/NO): %s", buf);
  if ((j!=1) || ((strcmp (buf,"YES")!=0) && (strcmp(buf,"NO")!=0)))
    panic ("Invalid format in file %s.\nInvalid priority preemptive %s\n",
	   filename, buf);
  if (strcmp(buf,"YES")==0)
    SCH_SVR4_PARAMS.priority_preemptive = TRUE;
  else
    SCH_SVR4_PARAMS.priority_preemptive = FALSE;
  machine->scheduler.priority_preemptive =
    SCH_SVR4_PARAMS.priority_preemptive;
  fgets (str, 256, fp);
  j = sscanf (str, "Minimum time preemption (seconds): %le",
	      &SCH_SVR4_PARAMS.minimum_time_preemption);
  if ((j!=1) || (SCH_SVR4_PARAMS.minimum_time_preemption<0)){
    panic ("Invalid format in file %s.\ni %d Invalid minimum time preemption %f\n",
	   filename, j, SCH_SVR4_PARAMS.minimum_time_preemption);
  }

  fgets (str, 256, fp);
  j = sscanf (str, "Quantum factor (%%): %le",
	      &SCH_SVR4_PARAMS.quantum_factor);
  if ((j!=1) || (SCH_SVR4_PARAMS.quantum_factor<0)){
    panic ("Invalid format in file %s.\nInvalid quantum factor %f\n",
	   filename, j, SCH_SVR4_PARAMS.quantum_factor);
  }
  IO_fclose (fp);

  if (debug)
  {
    PRINT_TIMER (current_time);
    printf (": SVR4 scheduler parameters\n");
    printf ("\t  Context switch cost %f\n",
	    SCH_SVR4_PARAMS.context_switch);
    printf ("\t  Busy wait %f\n",
	    SCH_SVR4_PARAMS.busy_wait);
    printf ("\t  Priority preemptive %s\n",
	    SCH_SVR4_PARAMS.priority_preemptive?"YES":"NO");
    printf ("\t  Minimum time preemttion %f\n",
	    SCH_SVR4_PARAMS.minimum_time_preemption);
    printf ("\t  Qauntum factor %f\n",
	    SCH_SVR4_PARAMS.quantum_factor);

  }
}

void
svr4_copy_parameters (struct t_thread *th_o, struct t_thread *th_d)
{
  tsproc_t *or,*de;

  or = (tsproc_t *)th_o->sch_parameters;
  de = (tsproc_t *)th_d->sch_parameters;

  de->ts_umdpri = or->ts_umdpri;
  de->ts_timeleft = or->ts_timeleft;
  de->ts_dispwait = or->ts_dispwait;
}

void
svr4_free_parameters (struct t_thread *thread)
{
  tsproc_t *sch_par;

  sch_par = (tsproc_t *)thread->sch_parameters;
  free (sch_par);
}
