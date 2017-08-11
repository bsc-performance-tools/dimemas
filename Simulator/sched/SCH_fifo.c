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

#include <assert.h>

/* Includes 'por defecto' */
#include "define.h"
#include "types.h"

#include <dimemas_io.h>

/* Include propio */
#include "SCH_fifo.h"

/* Dependencias con otros fuentes */
#include "cpu.h"
#include "extern.h"
#include "list.h"
#include "subr.h"
#include "read.h"

#include "simulator.h"
#include "machine.h"
#include "node.h"

void FIFO_thread_to_ready (struct t_thread *thread)
{
  struct t_node    *node;
  struct t_machine *machine;

  node = get_node_of_thread (thread);
  machine = node->machine;

  if ( (thread->loose_cpu) || (machine->scheduler.lost_cpu_on_send) )
    inFIFO_queue (& (node->ready), (char *) thread);
  else
    inLIFO_queue (& (node->ready), (char *) thread);
}

t_nano FIFO_get_execution_time (struct t_thread *thread)
{
  struct t_action *action;
  t_nano           ex_time;

  action = thread->action;
  if (action->action != WORK && action->action != GPU_BURST)
  {
    printf ("Must be work for P%02d T%02d t%02d but it was %d\n",
            IDENTIFIERS (thread),
            action->action);
  }

  ex_time = action->desc.compute.cpu_time;

  thread->action = action->next;
  READ_free_action(action);

  return ex_time;
}

struct t_thread *
FIFO_next_thread_to_run (struct t_node *node)
{
  return ( (struct t_thread *) outFIFO_queue (& (node->ready) ) );
}

void
FIFO_init_scheduler_parameters (struct t_thread *thread)
{
  thread->sch_parameters = A_NIL;
}

void
FIFO_clear_parameters (struct t_thread *thread)
{
  assert (thread != NULL);
}

int
FIFO_info (int info)
{
  return (0);
}

void
FIFO_init (char *filename, struct t_machine  *machine)
{
  FILE *fp;
  int j;
  char buf[256];
  char str[256];
  double fl;

  if (filename == (char *) 0)
    return;

  if (strcmp (filename, "") == 0)
    return;

  printf ("   * FIFO scheduler init with file %s\n", filename);

  fp = IO_fopen (filename, "r");
  if (fp == NULL)
  {
    printf ("   * WARNING: Can't open FIFO configuration file %s\n", filename);
    return;
  }

  fgets (str, 256, fp);
  j = sscanf (str, "Policy: %s", buf);
  if (j != 1)
    die ("Invalid format in file %s.\nInvalid policy name %s\n",
         filename,
         buf);

  fgets (str, 256, fp);
  j = sscanf (str, "Context switch cost (seconds): %le", &fl);
  if ( (j != 1) || (fl < 0) )
    die ("Invalid format in file %s.\nInvalid context switch cost %f\n",
           filename, fl);
  machine->scheduler.context_switch = fl * 1e9;

  fgets (str, 256, fp);
  j = sscanf (str, "Busy wait (seconds):  %le", &fl);
  if ( (j != 1) || (fl < 0) )
    die ("Invalid format in file %s.\nInvalid busy wait %f\n",
           filename, fl);
  machine->scheduler.busywait_before_block = fl * 1e9;

  IO_fclose (fp);
}

void
FIFO_copy_parameters (struct t_thread *th_o, struct t_thread *th_d)
{
  assert (th_o != NULL);
  assert (th_d != NULL);
}

void
FIFO_free_parameters (struct t_thread *thread)
{
  assert (thread != NULL);
}
