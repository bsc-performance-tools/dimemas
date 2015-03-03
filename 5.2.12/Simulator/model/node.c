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

  $URL:: https://svn.bsc.#$:  File
  $Rev:: 35               $:  Revision of last commit
  $Author:: jgonzale      $:  Author of last commit
  $Date:: 2012-01-11 19:4#$:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include <stdlib.h>
#include <string.h>

#include "simulator.h"
#include "node.h"

/* Thats to generate an unique node id */
static int unique_nodeid = 0;

void NODE_Init_Empty_Node(struct t_machine* machine,
                          struct t_node*    node)
{
  node->nodeid = unique_nodeid++;

  create_queue (&(node->Cpus));
  create_queue (&(node->ready));

  node->infinite_mem_links = FALSE;
  node->infinite_net_links = FALSE;

  /* Internal network management queues */
  create_queue (&(node->free_out_links));
  create_queue (&(node->free_in_links));
  create_queue (&(node->busy_out_links));
  create_queue (&(node->busy_in_links));
  create_queue (&(node->th_for_in));
  create_queue (&(node->th_for_out));

  /* Memory message queue */
  create_queue (&(node->wait_for_mem_bus));
  create_queue (&(node->threads_in_memory));


  create_queue (&(node->wait_outlink_port));
  create_queue (&(node->wait_inlink_port));
  create_queue (&(node->wait_in_copy_segment));
  create_queue (&(node->wait_out_copy_segment));
  create_queue (&(node->IO_disks));
  create_queue (&(node->IO_disks_threads));

  node->initialized = FALSE;
  node->machine     = machine;

#ifdef USE_EQUEUE
    insert_Equeue (&Node_queue, (char *) node, (t_priority) (node->nodeid));
#else
    insert_queue (&Node_queue, (char *) node, (t_priority) (node->nodeid));
#endif

}

void NODE_Fill_Node_Fields(struct t_node *node,
                           char          *node_name,
                           int            no_processors,
                           int            no_mem_buses,
                           int            no_mem_in_links,
                           int            no_mem_out_links,
                           int            no_input,
                           int            no_output,
                           double         local_startup,
                           double         remote_startup,
                           double         relative,
                           double         local_bandwidth,
                           double         external_net_startup,
                           double         local_port_startup,
                           double         remote_port_startup,
                           double         local_memory_startup,
                           double         remote_memory_startup)
{
  int             j;
  struct t_cpu   *cpu;
  struct t_link  *link;

  /* Thats a useless warning
  if (count_queue (&(node->Cpus)) != 0)
  {
    fprintf (stderr, "   * Warning: redefinition of node %d\n", no_number - 1);
  }
  */

  for (j = 0; j < no_processors; j++)
  {
    cpu = (struct t_cpu *) malloc (sizeof (struct t_cpu));

    cpu->cpuid                  = j + 1;
    cpu->current_thread         = TH_NIL;
    cpu->current_thread_context = TH_NIL;
    cpu->current_load           = (double) 0;
    cpu->io                     = QU_NIL;
    insert_queue (&(node->Cpus), (char *) cpu, (t_priority) (j + 1));
  }

  if (no_input == 0 && no_output == 0)
  {
    node->infinite_net_links = TRUE;
  }

  if (no_input == 0 || no_output == 0)
  {
    node->half_duplex_links = TRUE;
    j = MAX(no_input, no_output);

    no_input  = j;
    no_output = j;
  }
  else
  {
    node->half_duplex_links = FALSE;
  }

  for (j = 0; j < no_input; j++)
  {
    link = (struct t_link *) malloc (sizeof (struct t_link));

    link->linkid    = j + 1;
    link->info.node = node;
    link->kind      = NODE_LINK;
    link->type      = IN_LINK;
    link->thread    = TH_NIL;

    ASS_ALL_TIMER (link->assigned_on, current_time);
    inFIFO_queue (&(node->free_in_links), (char *) link);
  }

  for (j = 0; j < no_output; j++)
  {
    link = (struct t_link *) malloc (sizeof (struct t_link));

    link->linkid    = j + 1;
    link->info.node = node;
    link->kind      = NODE_LINK;
    link->type      = OUT_LINK;
    link->thread    = TH_NIL;

    ASS_ALL_TIMER (link->assigned_on, current_time);
    inFIFO_queue (&(node->free_out_links), (char *) link);
  }

  node->max_memory_messages = no_mem_buses;
  node->in_mem_links        = no_mem_in_links;
  node->out_mem_links       = no_mem_out_links;

  /*
  for (j = 0; j < no_mem_links; j++)
  {
    link = (struct t_link*) malloc (sizeof(struct t_link));

    link->linkid    = j + 1;
    link->info.node = node;
    link->kind      = MEM_LINK;
    // link->type      = OUT_LINK;
    link->thread    = TH_NIL;

    ASS_ALL_TIMER (link->assigned_on, current_time);
    inFIFO_queue (&(node->free_mem_links), (char *) link);
  }
  */

  node->arch                  = strdup(node_name);

  node->local_startup         = local_startup;
  node->remote_startup        = remote_startup;
  node->relative              = relative;
  node->bandwidth             = local_bandwidth;

  node->external_net_startup  = external_net_startup;
  node->local_port_startup    = local_port_startup;
  node->remote_port_startup   = remote_port_startup;
  node->local_memory_startup  = local_memory_startup;
  node->remote_memory_startup = remote_memory_startup;

  node->initialized = TRUE;

  return;
}
