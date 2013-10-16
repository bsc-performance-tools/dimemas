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

#include "define.h"
#include "types.h"

#include "node.h"

#include "communic.h"
#include "cpu.h"
#include "extern.h"
#include "events.h"
#include "fs.h"
#include "links.h"
#include "list.h"
#ifdef USE_EQUEUE
#include "listE.h"
#endif
#include "mallocame.h"
#include "paraver.h"
#include "ports.h"
#include "schedule.h"
#include "subr.h"


static struct t_port *
locate_port (int portid)
{
  struct t_port  *port;

  for (port = (struct t_port *) head_queue (&Port_queue);
       port != PO_NIL;
       port = (struct t_port *) next_queue (&Port_queue) )
  {
    if (port->portid == portid)
    {
      return (port);
    }
  }

  return (PO_NIL);
}

static void
port_to_next_module (struct t_thread *thread)
{
  int             value;
  struct t_action *action;

  switch (thread->to_module)
  {
    case M_FS:

      if ( (thread->port_send_link != L_NIL) || (thread->port_send_link == L_NUL) )
      {
        thread->port_send_link = L_NIL;
        value = FS_PORT_SEND_END;
      }
      else
      {
        thread->port_recv_link = L_NIL;
        value = FS_PORT_RECV_END;
      }

      thread->size_port = 0;
      thread->to_module = 0;
      action = thread->action;

      if (action != AC_NIL)
        if ( (action->action == PORT_SEND) || (action->action == PORT_RECV) )
          thread->action = action->next;

      FS_general (value, thread);

      break;
    default:
      panic ("Invalid module identifier in Port %d\n", thread->to_module);
  }
}

void
PORT_general (int value, struct t_thread *thread)
{
  switch (value)
  {
    case PORT_TIMER_OUT:

      if (thread->port_send_link == L_NUL)
      {
        thread->port->sending--;
        thread->port = PO_NIL;
        port_to_next_module (thread);
        break;
      }

      if (thread->port_recv_link == L_NUL)
      {
        thread->port = PO_NIL;
        port_to_next_module (thread);
        break;
      }

      if (thread->port_recv_link == L_NIL)
      {
        /* Sender thread  */
        if (debug & D_PORT)
        {
          PRINT_TIMER (current_time);
          printf (": PORT_general\tP%02d T%02d (t%02d) End PORT send\n",
                  IDENTIFIERS (thread) );
        }

        free_link (thread->port_send_link, thread);
        thread->port->sending--;
        thread->port = PO_NIL;
        port_to_next_module (thread);
      }
      else
      {
        /* Receiver thread */
        if (debug & D_PORT)
        {
          PRINT_TIMER (current_time);
          printf (": PORT_general P%02d T%02d (t%02d) End PORT recv\n",
                  IDENTIFIERS (thread) );
        }

        free_link (thread->port_recv_link, thread);
        port_to_next_module (thread);
      }

      break;
    default:
      panic ("Invalid interface value %d in ports\n", value);
  }
}

void
PORT_init()
{
  struct t_node  *node;

  if (debug & D_PORT)
  {
    PRINT_TIMER (current_time);
    printf (": PORT initial routine called\n");
  }

#ifdef USE_EQUEUE

  for (node  = (struct t_node*) head_Equeue (&Node_queue);
       node != N_NIL;
       node  = (struct t_node*) next_Equeue (&Node_queue) )
#else
  for (node  = (struct t_node*) head_queue (&Node_queue);
       node != N_NIL;
       node  = (struct t_node*) next_queue (&Node_queue) )
#endif
  {
    create_queue (& (node->wait_outlink_port) );
    create_queue (& (node->wait_inlink_port) );
  }
}

void
PORT_end()
{
  struct t_port  *port;
  struct t_node  *node;
  struct t_thread *thread;

  if (debug & D_PORT)
  {
    PRINT_TIMER (current_time);
    printf (": PORT end routine called\n");
  }

  for (port  = (struct t_port *) head_queue (&Port_queue);
       port != PO_NIL;
       port  = (struct t_port *) next_queue (&Port_queue) )
  {
    if (empty_queue (& (port->recv) ) == FALSE)
    {
      if (debug & D_PORT)
      {
        PRINT_TIMER (current_time);
        printf (": Warning PORT end with %d threads recv on port %d\n",
                count_queue (& (port->recv) ),
                port->portid);
      }

      for (thread  = (struct t_thread *) head_queue (& (port->recv) );
           thread != TH_NIL;
           thread  = (struct t_thread *) next_queue (& (port->recv) ) )
      {
        node = get_node_of_thread (thread);
        PARAVER_Idle (0,
                      IDENTIFIERS (thread),
                      thread->last_paraver,
                      current_time);
      }
    }

    if (empty_queue (& (port->send) ) == FALSE)
    {
      if (debug & D_PORT)
      {
        PRINT_TIMER (current_time);
        printf (": PORT end with %d threads send on port %d\n",
                count_queue (& (port->send) ), port->portid);
      }

      for (thread = (struct t_thread *) head_queue (& (port->send) );
           thread != TH_NIL;
           thread = (struct t_thread *) next_queue (& (port->send) ) )
      {
        node = get_node_of_thread (thread);
        PARAVER_Idle (0, IDENTIFIERS (thread),
                      thread->last_paraver, current_time);
      }
    }

    if (port->sending != 0)
    {

      if (debug & D_PORT)
      {
        PRINT_TIMER (current_time);
        printf (": PORT end of portid %d but is doing %d communications\n",
                port->portid, port->sending);
      }
    }
  }
}

t_boolean
PORT_create (int portid, struct t_thread *thread)
{
  struct t_port  *port;

  if (debug & D_PORT)
  {
    PRINT_TIMER (current_time);
    printf (": PORT create called for portid %d\n", portid);
  }

  if (!assert)
    port = PO_NIL;
  else
  {
//  Vladimir: this locate_port is an EXPENSIVE ASSERT - I ELIMINATED IT
//  -> ASSUME THAT IT WILL NEVER FIND THE OVERLAP OF PORT_IDs
    port = locate_port (portid);

    if (port != PO_NIL)
    {
      assert (0);

      if (debug & D_PORT)
      {
        PRINT_TIMER (current_time);
        printf (": PORT create duplication of portid %d created by Ptask %d\n",
                portid, port->thread->task->Ptask->Ptaskid);
      }

      return (FALSE);
    }

    return (FALSE);
  }

  port = (struct t_port *) MALLOC_get_memory (sizeof (struct t_port) );
  port->portid = portid;
  port->thread = thread;
  create_queue (& (port->send) );
  create_queue (& (port->recv) );
  port->sending = 0;

  inFIFO_queue (&Port_queue, (char *) port);

  return (TRUE);
}

t_boolean
PORT_delete (int portid)
{
  struct t_port  *port;

  if (debug & D_PORT)
  {
    PRINT_TIMER (current_time);
    printf (": PORT delete called for portid %d\n", portid);
  }

  port = locate_port (portid);

  if (port == PO_NIL)
  {
    if (debug & D_PORT)
    {
      PRINT_TIMER (current_time);
      printf (": PORT delete of inexistent port %d\n",
              portid);
    }

    return (FALSE);
  }

  if (empty_queue (& (port->send) ) == FALSE)
  {
    if (debug & D_PORT)
    {
      PRINT_TIMER (current_time);
      printf (": PORT delete of portid %d but have %d senders blocked\n",
              portid, count_queue (& (port->send) ) );
    }

    return (FALSE);
  }

  if (empty_queue (& (port->recv) ) == FALSE)
  {
    if (debug & D_PORT)
    {
      PRINT_TIMER (current_time);
      printf (": PORT delete of portid %d but have %d receivers blocked\n",
              portid, count_queue (& (port->recv) ) );
    }

    return (FALSE);
  }

  if (port->sending != 0)
  {

    if (debug & D_PORT)
    {
      PRINT_TIMER (current_time);
      printf (": PORT delete of portid %d but is doing %d communications\n",
              portid, port->sending);
    }

    return (FALSE);
  }

  extract_from_queue (&Port_queue, (char *) port);
  MALLOC_free_memory ( (char*) port);
  return (TRUE);
}

t_boolean
PORT_send (int module, int portid, struct t_thread *thread, int siz)
{
  struct t_port   *port;
  struct t_thread *receiver;
  struct t_action *action;
  t_nano          startup;
  struct t_node   *node;

  port = locate_port (portid);

  if (port == PO_NIL)
  {
    if (debug & D_PORT)
    {
      PRINT_TIMER (current_time);
      printf (": PORT send to an invalid port %d on P%d T%d t%d\n",
              portid,
              IDENTIFIERS (thread) );
    }

    return (FALSE);
  }

  node = get_node_of_thread (thread);

  if (empty_queue (& (port->recv) ) )
    startup = node->remote_port_startup;
  else
  {
    receiver = (struct t_thread *) head_queue (& (port->recv) );

    if (node == get_node_of_thread (receiver) )
      startup = node->local_port_startup;
    else
      startup = node->remote_port_startup;
  }

  if ( (startup != (t_nano) 0) && (thread->startup_done == FALSE) )
  {
    if (debug & D_PORT)
    {
      PRINT_TIMER (current_time);
      printf (": PORT %d startup send for P%d T%d t%d\n",
              portid,
              IDENTIFIERS (thread) );
    }

    action = (struct t_action *) MALLOC_get_memory (sizeof (struct t_action) );
    action->next = thread->action;
    action->action = PORT_SEND;
    action->desc.port.portid = portid;
    action->desc.port.size = siz;
    action->desc.port.module = module;
    thread->action = action;
    thread->doing_startup = TRUE;
    SCHEDULER_thread_to_ready_return (M_PORT, thread, startup, 0);
    return (TRUE);
  }

  thread->startup_done = FALSE;
  thread->loose_cpu = TRUE;

  thread->size_port = siz;
  thread->to_module = module;

  if (empty_queue (& (port->recv) ) )
  {
    if (debug & D_PORT)
    {
      PRINT_TIMER (current_time);
      printf (": PORT send on %d from P%d T%d t%d must wait\n",
              portid,
              IDENTIFIERS (thread) );
    }

    inFIFO_queue (& (port->send), (char *) thread);
    return (TRUE);
  }
  else
  {
    receiver = (struct t_thread *) outFIFO_queue (& (port->recv) );

    if (debug & D_PORT)
    {
      PRINT_TIMER (current_time);
      printf (": PORT send on %d send(P%d T%d t%d) recv(P%d T%d t%d)\n",
              portid,
              IDENTIFIERS (thread),
              IDENTIFIERS (receiver) );
    }

    /* Data inicialitation */
    thread->port_send_link = L_NIL;
    thread->port_recv_link = L_NIL;
    receiver->port_recv_link = L_NIL;
    receiver->port_send_link = L_NIL;

    really_port_send (port, thread, receiver);
    return (TRUE);
  }
}

t_boolean
PORT_receive (int module, int portid, struct t_thread *thread, int siz)
{
  struct t_port   *port;
  struct t_thread *sender;
  struct t_action *action;
  struct t_node   *node;
  t_nano          startup;

  port = locate_port (portid);

  if (port == PO_NIL)
  {
    if (debug & D_PORT)
    {
      PRINT_TIMER (current_time);
      printf (": PORT recv to an invalid port %d on P%d T%d t%d\n",
              portid,
              IDENTIFIERS (thread) );
    }

    return (FALSE);
  }

  node = get_node_of_thread (thread);

  if (empty_queue (& (port->send) ) )
    startup = node->remote_port_startup;
  else
  {
    sender = (struct t_thread *) head_queue (& (port->send) );

    if (node == get_node_of_thread (sender) )
      startup = node->local_port_startup;
    else
      startup = node->remote_port_startup;
  }

  if ( (startup != 0) && (thread->startup_done == FALSE) )
  {
    if (debug & D_PORT)
    {
      PRINT_TIMER (current_time);
      printf (": PORT %d startup recv for P%d T%d t%d\n",
              portid,
              IDENTIFIERS (thread) );
    }

    action = (struct t_action *) MALLOC_get_memory (sizeof (struct t_action) );
    action->next = thread->action;
    action->action = PORT_RECV;
    action->desc.port.portid = portid;
    action->desc.port.size = siz;
    action->desc.port.module = module;
    thread->action = action;
    thread->doing_startup = TRUE;
    SCHEDULER_thread_to_ready_return (M_PORT, thread, startup, 0);
    return (TRUE);
  }

  thread->startup_done = FALSE;
  thread->loose_cpu    = TRUE;
  thread->size_port    = siz;
  thread->to_module    = module;

  if (empty_queue (& (port->send) ) )
  {
    if (debug & D_PORT)
    {
      PRINT_TIMER (current_time);
      printf (": PORT recv on %d from P%d T%d t%d must wait\n",
              portid,
              IDENTIFIERS (thread) );
    }

    inFIFO_queue (& (port->recv), (char *) thread);
    return (TRUE);
  }
  else
  {
    sender = (struct t_thread *) outFIFO_queue (& (port->send) );

    if (debug & D_PORT)
    {
      PRINT_TIMER (current_time);
      printf (": PORT recv on %d send(P%d T%d t%d) recv(P%d T%d t%d)\n",
              portid,
              IDENTIFIERS (sender),
              IDENTIFIERS (thread) );
    }

    /* Data inicialitation */
    thread->port_send_link = L_NIL;
    thread->port_recv_link = L_NIL;
    sender->port_recv_link = L_NIL;
    sender->port_send_link = L_NIL;

    really_port_send (port, sender, thread);
    return (TRUE);
  }
}


void
really_port_send (struct t_port *port, struct t_thread *thread_s,
                  struct t_thread *thread_r)
{
  struct t_node  *node_s,
      *node_r;
  t_nano         ti;
  struct t_both  *both;
  t_boolean       remote_comm;
  dimemas_timer   tmp_timer;

  node_s = get_node_of_thread (thread_s);
  node_r = get_node_of_thread (thread_r);

  if (get_links_port (thread_s, node_s, thread_r, node_r) )
{
    remote_comm = (node_s == node_r ? FALSE : TRUE);

    /* ti = transferencia (thread_s->size_port, remote_comm, thread_s, NULL, NULL); */
    transferencia (thread_s->size_port, remote_comm, thread_s, NULL, &ti, NULL);

    thread_s->port = port;
    port->sending++;

    if (debug & D_PORT)
    {
      PRINT_TIMER (current_time);
      printf (": PORT send P%d T%d t%d to P%d T%d t%d start comm.\n",
              IDENTIFIERS (thread_s), IDENTIFIERS (thread_r) );
    }

    FLOAT_TO_TIMER (ti, tmp_timer);
    ADD_TIMER (current_time, tmp_timer, tmp_timer);
    thread_s->event = EVENT_timer (tmp_timer, NOT_DAEMON, M_PORT, thread_s, 0);
    thread_r->event = EVENT_timer (tmp_timer, NOT_DAEMON, M_PORT, thread_r, 0);
  }
  else
  {

    /*
     * Must wait for links
     */
    both = (struct t_both *) MALLOC_get_memory (sizeof (struct t_both) );
    both->port = port;
    both->thread_s = thread_s;
    both->thread_r = thread_r;

    if (thread_s->port_send_link == L_NIL)
    {
      inFIFO_queue (& (node_s->wait_outlink_port), (char *) both);
    }
    else
    {
      inFIFO_queue (& (node_r->wait_inlink_port), (char *) both);
    }

    if (debug & D_PORT)
    {
      PRINT_TIMER (current_time);
      printf (": PORT send P%d T%d t%d to P%d T%d t%d waits for links\n",
              IDENTIFIERS (thread_s), IDENTIFIERS (thread_r) );
    }
  }
}
