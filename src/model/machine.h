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

#ifndef _MACHINE_H_
#define _MACHINE_H_

#include <types.h>

struct t_machine
{
  char          *name;              /* Name of the machine */
  int            id;                /* Machine number */
  char          *instrumented_arch; /* Architecture used to instrument */
  int            number_of_nodes;   /* Number of nodes on virtual machine */

  struct t_scheduler /* Informacio del scheduler */
  {
    int           policy;
    t_nano       quantum;
    t_boolean     priority_preemptive;
    t_boolean     lost_cpu_on_send;
    t_nano       context_switch;
    t_boolean     busywait_before_block;
    dimemas_timer minimum_quantum;
  } scheduler;

  struct t_communication /* Parametres de la xarxa interna */
  {
    int            policy;
    int            quantum;
    t_nano        remote_bandwidth;
    int            num_messages_on_network;
    t_nano        port_startup;
    t_nano        memory_startup;
    int            global_op_model;
    struct t_queue global_ops_info;
  } communication;

  struct t_network /* Dades variables de l'estat de la xarxa interna */
  {
    double         total_time_in_queue;
    double         utilization;
    dimemas_timer  last_actualization;
    int            curr_on_network;
    struct t_queue threads_on_network;
    struct t_queue queue;
  } network;

  struct t_external_net /* Dades variables de l'estat de la xarxa externa */
  {
    t_boolean       half_duplex_links; /* TRUE if links are half duplex */
    struct t_queue  free_in_links;     /* Free input links */
    struct t_queue  free_out_links;    /* Free output links */
    struct t_queue  busy_in_links;     /* Busy input links */
    struct t_queue  busy_out_links;    /* Busy output links */
    struct t_queue  th_for_in;         /* Awaiting for input link */
    struct t_queue  th_for_out;        /* Awaiting for output link */
  } external_net;

  struct t_dedicated_connections_net /* Dades variables de l'estat */
  {                                  /* de la xarxa externa */
    struct t_queue connections;      /* Cua de les connexions dedicades que
                                      * tenen aquesta maquina com a origen
                                      * o desti. */
  } dedicated_connections;
};

void MACHINE_Init_Empty_Machine(struct t_machine* machine,
                                int               machine_id);

void MACHINE_Fill_Machine_Fields(struct t_machine* machine,
                                 char  *machine_name,
                                 char  *instrumented_architecture,
                                 int    number_of_nodes,
                                 double network_bandwidth,
                                 int    number_of_buses,
                                 int    global_operation_model);

#endif
