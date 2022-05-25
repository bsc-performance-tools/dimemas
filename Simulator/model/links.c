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

#include "Macros.h" /* Per l'ASSERT */

#include <communic.h>
#include <cpu.h>
#include <define.h>
#include <extern.h>
#include <links.h>
#include <list.h>
#include <machine.h>
#include <math.h>
#include <memory.h>
#include <node.h>
#include <paraver.h>
#include <ports.h>
#include <sched_vars.h>
#include <subr.h>
#include <types.h>


/***************************************************************************/
/**** MACROS per accounting del temps esperant links                    ****/
/***************************************************************************/
#include "task.h"

/* FEC: Macro per assignar l'inici del temps d'un thread bloquejat esperant
        un link */
#define START_LINK_WAIT_TIME( thread )                                                      \
  {                                                                                         \
    struct t_account *account;                                                              \
    /*    double aux;  */                                                                   \
    account = current_account( thread );                                                    \
    ASS_ALL_TIMER( ( account->initial_wait_link_time ), current_time );                     \
    /*                                                                                      \
        TIMER_TO_FLOAT(current_time,aux);                                                   \
        fprintf(stderr,"\nS'activa temps wait link P%d, T%d, t%d instant: %f thread: %X\n", \
                      IDENTIFIERS (thread), aux, thread);                                   \
    */                                                                                      \
  }


/* FEC: Macro per acumular el temps d'un thread bloquejat esperant un link */
#define ACCUMULATE_LINK_WAIT_TIME( thread )                                               \
  {                                                                                       \
    dimemas_timer tmp_timer;                                                              \
    struct t_account *account;                                                            \
    /*    double aux, aux2, aux3; */                                                      \
    account = current_account( thread );                                                  \
    FLOAT_TO_TIMER( 0, tmp_timer );                                                       \
    if ( !EQ_TIMER( ( account->initial_wait_link_time ), tmp_timer ) )                    \
    {                                                                                     \
      SUB_TIMER( current_time, ( account->initial_wait_link_time ), tmp_timer );          \
      ADD_TIMER( tmp_timer, ( account->block_due_link ), ( account->block_due_link ) );   \
      /*                                                                                  \
            TIMER_TO_FLOAT(tmp_timer,aux);                                                \
            TIMER_TO_FLOAT(current_time,aux2);                                            \
            TIMER_TO_FLOAT((account->initial_wait_link_time),aux3);                       \
            fprintf(stderr,"\n%f: Accumula P%d, T%d, t%d - %f = link wait time: %f %X\n", \
                        aux2,IDENTIFIERS (thread), aux3, aux, thread);                    \
      */                                                                                  \
      FLOAT_TO_TIMER( 0, ( account->initial_wait_link_time ) );                           \
    }                                                                                     \
  }

/*****************************************************************************
******************************************************************************
                    ###
                     #     #    #   #####  ######  #####
                     #     ##   #     #    #       #    #
                     #     # #  #     #    #####   #    #
                     #     #  # #     #    #       #####
                     #     #   ##     #    #       #   #
                    ###    #    #     #    ######  #    #


             #####   #####    ####    ####   ######   ####    ####
             #    #  #    #  #    #  #    #  #       #       #
             #    #  #    #  #    #  #       #####    ####    ####
             #####   #####   #    #  #       #            #       #
             #       #   #   #    #  #    #  #       #    #  #    #
             #       #    #   ####    ####   ######   ####    ####


                     #          #    #    #  #    #   ####
                     #          #    ##   #  #   #   #
                     #          #    # #  #  ####     ####
                     #          #    #  # #  #  #         #
                     #          #    #   ##  #   #   #    #
                     ######     #    #    #  #    #   ####

*****************************************************************************
*****************************************************************************/

void mem_link_busy( struct t_thread *thread, struct t_task *task, int in_out )
{
  struct t_node *node;
  struct t_machine *machine;

  node    = get_node_of_thread( thread );
  machine = node->machine;

  switch ( machine->communication.policy )
  {
    case COMMUNIC_FIFO:
    case COMMUNIC_RR:
    case COMMUNIC_BOOST:
      if ( in_out == OUT_LINK )
      {
        inFIFO_queue( &( task->th_for_out ), (char *)thread );
      }
      else
      {
        inFIFO_queue( &( task->th_for_in ), (char *)thread );
      }
      break;
    default:
      panic( "Communication policy not implemented\n" );
  }

  START_LINK_WAIT_TIME( thread );
}

t_boolean LINKS_get_mem_links( struct t_thread *thread_snd, struct t_task *task_snd, struct t_task *task_rcv )
{
  struct t_link *link;
  struct t_node *node_snd, *node_rcv;

  node_snd = get_node_of_task( task_snd );
  node_rcv = get_node_of_task( task_rcv );

  if ( ( task_snd->taskid == task_rcv->taskid ) )
  {
    return TRUE;
  }

  if ( ( task_snd->half_duplex_links ) || ( task_rcv->half_duplex_links ) )
  {
    if ( task_snd->taskid > task_rcv->taskid )
    {
      goto first_dest;
    }
  }

second_src_link:

  if ( node_snd->infinite_mem_links != TRUE )
  {
    // Check if the Output Link has been acquired

    link = thread_snd->local_link;

    if ( link == L_NIL )
    {
      // Not yet! Check if there are links available
      link = (struct t_link *)outFIFO_queue( &( task_snd->free_out_links ) );
    }

    if ( link == L_NIL )
    {
      mem_link_busy( thread_snd, task_snd, OUT_LINK );

      if ( debug & D_LINKS )
      {
        PRINT_TIMER( current_time );
        printf( ": P%02d T%02d (t%02d) no local (OUT) write permissions to task %d\n", IDENTIFIERS( thread_snd ), task_rcv->taskid );
      }

      return FALSE;
    }
    else
    {
      /* The link has just been extracted from the 'free_out_links' */
      if ( thread_snd->local_link == L_NIL )
      {
        if ( task_snd->half_duplex_links )
        {
          /* If the output has been obtained, it MUST be an input free */
          thread_snd->local_hd_link = (struct t_link *)outFIFO_queue( &( task_snd->free_in_links ) );

          if ( thread_snd->local_hd_link == L_NIL )
          {
            panic( "P%02d T%02d (t%02d) unable to obtain destination inter-process memory\n", IDENTIFIERS( thread_snd ) );
          }

          inFIFO_queue( &( task_snd->busy_in_links ), (char *)thread_snd->local_hd_link );
        }
        ASS_ALL_TIMER( link->assigned_on, current_time );
        inFIFO_queue( &( task_snd->busy_out_links ), (char *)link );
      }
    }

    thread_snd->local_link = link;

    if ( ( task_snd->half_duplex_links ) || ( task_rcv->half_duplex_links ) )
    {
      if ( task_snd->taskid > task_rcv->taskid )
      {
        goto end_get_links;
      }
    }
  }

first_dest:

  if ( node_rcv->infinite_mem_links != TRUE )
  {
    link = thread_snd->partner_link;
    if ( link == L_NIL )
    {
      link = (struct t_link *)outFIFO_queue( &( task_rcv->free_in_links ) );
    }

    if ( link == L_NIL )
    {
      mem_link_busy( thread_snd, task_rcv, IN_LINK );
      if ( debug & D_LINKS )
      {
        PRINT_TIMER( current_time );
        printf( ": P%02d T%02d (t%02d) no local (IN) write permissions to %d\n", IDENTIFIERS( thread_snd ), task_rcv->taskid );
      }
      return ( FALSE );
    }
    else
    {
      if ( thread_snd->partner_link == L_NIL )
      {
        if ( task_rcv->half_duplex_links )
        {
          /* If the input has been obtained, it MUST be an output free */
          thread_snd->partner_hd_link = (struct t_link *)outFIFO_queue( &( task_rcv->free_out_links ) );

          if ( thread_snd->partner_hd_link == L_NIL )
          {
            panic( "P%02d T%02d (t%02d) unable to obtain destination inter-process memory\n", IDENTIFIERS( thread_snd ) );
          }
          inFIFO_queue( &( task_rcv->busy_out_links ), (char *)thread_snd->partner_hd_link );
        }

        ASS_ALL_TIMER( link->assigned_on, current_time );
        inFIFO_queue( &( task_rcv->busy_in_links ), (char *)link );
      }
    }

    thread_snd->partner_link = link;

    if ( ( task_snd->half_duplex_links ) || ( task_rcv->half_duplex_links ) )
    {
      if ( task_snd->taskid > task_rcv->taskid )
      {
        goto second_src_link;
      }
    }
  }

end_get_links:

  if ( debug & D_LINKS )
  {
    PRINT_TIMER( current_time );
    printf( ": P%02d T%02d (t%02d)  Inter-process communication available to task %d\n", IDENTIFIERS( thread_snd ), task_rcv->taskid );
  }

  return TRUE;
}

void LINKS_free_mem_link( struct t_link *link, struct t_thread *thread )
{
  struct t_thread *first;
  struct t_task *task;
  struct t_node *node;
  struct t_cpu *cpu;
  struct t_both *both;
  struct t_copyseg *copyseg;

  // task = thread->task;
  // cpu  = get_cpu_of_thread(thread);
  // node = link->info.node;
  task = link->info.task;

  if ( link->type == IN_LINK )
  {
    if ( debug & D_LINKS )
    {
      PRINT_TIMER( current_time );
      printf( ": P%02d T%02d (t%02d) frees writing memory permissions (IN) to task %d\n", IDENTIFIERS( thread ), task->taskid );
    }
    inFIFO_queue( &( task->free_in_links ), (char *)link );
    extract_from_queue( &( task->busy_in_links ), (char *)link );

    if ( task->half_duplex_links )
    {
      if ( debug & D_LINKS )
      {
        PRINT_TIMER( current_time );
        printf( ": P%02d T%02d (t%02d) frees memory writing permissions (OUT) to task %d\n", IDENTIFIERS( thread ), task->taskid );
      }

      inFIFO_queue( &( task->free_out_links ), (char *)thread->partner_hd_link );

      extract_from_queue( &( task->busy_out_links ), (char *)thread->partner_hd_link );

      first = (struct t_thread *)outFIFO_queue( &( task->th_for_out ) );

      if ( first != TH_NIL )
      {
        /* FEC: El primer que faig es acumular el temps que el thread ha
                  estat bloquejat esperant el link */
        ACCUMULATE_LINK_WAIT_TIME( first );
        /***************************************************************/

        if ( first->original_thread )
        {
          first->last_paraver = current_time;
        }
        if ( debug & D_LINKS )
        {
          PRINT_TIMER( current_time );
          printf( ": free memory writing permissions (OUT) to task %d wakes-up P%02d T%02d (t%02d)\n", task->taskid, IDENTIFIERS( first ) );
        }

        if ( first->action->action == SEND || first->action->action == GLOBAL_OP )
        {
          really_send( first );
        }
        else if ( first->action->action == MPI_OS )
        {
          really_RMA( first );
        }
      }
    }

    first = (struct t_thread *)outFIFO_queue( &( task->th_for_in ) );

    if ( first != TH_NIL )
    {
      /* FEC: El primer que faig es acumular el temps que el thread ha estat
       * bloquejat esperant el link */
      ACCUMULATE_LINK_WAIT_TIME( first );
      /***************************************************************/

      if ( first->original_thread )
      {
        first->last_paraver = current_time;
      }
      if ( debug & D_LINKS )
      {
        PRINT_TIMER( current_time );
        printf( ": Free memory writing permissions (IN) to task %d wakes-up P%02d T%02d (t%02d)\n", task->taskid, IDENTIFIERS( first ) );
      }
      if ( first->action->action == SEND || first->action->action == GLOBAL_OP )
      {
        really_send( first );
      }
      else if ( first->action->action == MPI_OS )
      {
        really_RMA( first );
      }

      return;
    }

    /*
    both = (struct t_both *) outFIFO_queue (&(node->wait_inlink_port));
    if (both != B_NIL)
    {
      if (debug&D_LINKS)
      {
        PRINT_TIMER (current_time);
        printf (": free link wakeup port communication\n");
      }
      really_port_send (both->port, both->thread_s, both->thread_r);
      free (both);
      return;
    }

    copyseg = (struct t_copyseg *) outFIFO_queue (&(node->wait_in_copy_segment));
    if (copyseg != CO_NIL)
    {
      if (debug&D_LINKS)
      {
        PRINT_TIMER (current_time);
        printf (": free link wakeup copy segment\n");
      }
      really_copy_segment (copyseg->thread,
                           copyseg->node_s,
                           copyseg->node_d,
                           copyseg->thread->copy_segment_size);
      free (copyseg);
      return;
    }
    */
  }
  else
  {
    if ( debug & D_LINKS )
    {
      PRINT_TIMER( current_time );
      printf( ": P%02d T%02d (t%02d) frees writing memory permissions (OUT) to task %d\n", IDENTIFIERS( thread ), task->taskid );
    }
    inFIFO_queue( &( task->free_out_links ), (char *)link );

    extract_from_queue( &( task->busy_out_links ), (char *)link );

    if ( task->half_duplex_links )
    {
      if ( debug & D_LINKS )
      {
        PRINT_TIMER( current_time );
        printf( ": P%02d T%02d (t%02d) frees writing memory permissions (IN) to task %d\n", IDENTIFIERS( thread ), task->taskid );
      }
      inFIFO_queue( &( task->free_in_links ), (char *)thread->local_hd_link );
      extract_from_queue( &( task->busy_in_links ), (char *)thread->local_hd_link );
      first = (struct t_thread *)outFIFO_queue( &( task->th_for_in ) );

      if ( first != TH_NIL )
      {
        /* FEC: El primer que faig es acumular el temps que el thread ha estat
         * bloquejat esperant el link */
        ACCUMULATE_LINK_WAIT_TIME( first );
        /***************************************************************/

        if ( first->original_thread )
        {
          first->last_paraver = current_time;
        }
        if ( debug & D_LINKS )
        {
          PRINT_TIMER( current_time );
          printf( ": free memory writing permissions (IN) to task %d wakes-up P%02d T%02d (t%02d)\n", task->taskid, IDENTIFIERS( first ) );
        }
        if ( first->action->action == SEND || first->action->action == GLOBAL_OP ) //TODO: ADD GLOBAL OP TO THE OTHER FREE LINKS CASES
          really_send( first );
        else if ( first->action->action == MPI_OS )
          really_RMA( first );
      }
    }
    first = (struct t_thread *)outFIFO_queue( &( task->th_for_out ) );

    if ( first != TH_NIL )
    {
      /* FEC: El primer que faig es acumular el temps que el thread ha estat
       * bloquejat esperant el link */
      ACCUMULATE_LINK_WAIT_TIME( first );
      /***************************************************************/

      if ( first->original_thread )
      {
        first->last_paraver = current_time;
      }
      if ( debug & D_LINKS )
      {
        PRINT_TIMER( current_time );
        printf( ": free memory writing permissions (OUT) to task %d wakes-up P%02d T%02d (t%02d)\n", task->taskid, IDENTIFIERS( first ) );
      }
      if ( first->action->action == SEND || first->action->action == GLOBAL_OP )
        really_send( first );
      else if ( first->action->action == MPI_OS )
        really_RMA( first );

      return;
    }

    /*
    both = (struct t_both *) outFIFO_queue (&(node->wait_outlink_port));
    if (both != B_NIL)
    {
      if (debug&D_LINKS)
      {
        PRINT_TIMER (current_time);
        printf (": free link wakeup port communication\n");
      }
      really_port_send (both->port, both->thread_s, both->thread_r);
      free (both);
      return;
    }

    copyseg = (struct t_copyseg *) outFIFO_queue (&(node->wait_out_copy_segment));
    if (copyseg != CO_NIL)
    {
      if (debug&D_LINKS)
      {
        PRINT_TIMER (current_time);
        printf (": free link wakeup copy segment\n");
      }
      really_copy_segment (copyseg->thread,
                           copyseg->node_s,
                           copyseg->node_d,
                           copyseg->thread->copy_segment_size);
      free (copyseg);
      return;
    }
    */
  }
}


/*****************************************************************************
******************************************************************************
          ###
           #     #    #   #####  ######  #####   #    #    ##    #
           #     ##   #     #    #       #    #  ##   #   #  #   #
           #     # #  #     #    #####   #    #  # #  #  #    #  #
           #     #  # #     #    #       #####   #  # #  ######  #
           #     #   ##     #    #       #   #   #   ##  #    #  #
          ###    #    #     #    ######  #    #  #    #  #    #  ######

            #     #
            ##    #  ######   #####  #    #   ####   #####   #    #
            # #   #  #          #    #    #  #    #  #    #  #   #
            #  #  #  #####      #    #    #  #    #  #    #  ####
            #   # #  #          #    # ## #  #    #  #####   #  #
            #    ##  #          #    ##  ##  #    #  #   #   #   #
            #     #  ######     #    #    #   ####   #    #  #    #


                   #          #    #    #  #    #   ####
                   #          #    ##   #  #   #   #
                   #          #    # #  #  ####     ####
                   #          #    #  # #  #  #         #
                   #          #    #   ##  #   #   #    #
                   ######     #    #    #  #    #   ####

*****************************************************************************
*****************************************************************************/

/***************************************************************************/

void link_busy_prioritat( struct t_thread *thread, struct t_node *node, int in_out, t_boolean prioritari )
{
  register struct t_link *older_link;
  dimemas_timer ti, quantum;
  struct t_node *n2;
  struct t_cpu *cpu;
  struct t_machine *machine;

  /* El thread no te perque ser del node donat, pero com que al menys
     de moment tots dos son de la mateixa maquina es igual */
  machine = node->machine;

  cpu = get_cpu_of_thread( thread );
  switch ( machine->communication.policy )
  {
    case COMMUNIC_FIFO:
    case COMMUNIC_RR:
    case COMMUNIC_BOOST:

      if ( in_out == OUT_LINK )
      {
        if ( prioritari )
          inLIFO_queue( &( node->th_for_out ), (char *)thread );
        else
          inFIFO_queue( &( node->th_for_out ), (char *)thread );
      }
      else
      {
        if ( prioritari )
          inLIFO_queue( &( node->th_for_in ), (char *)thread );
        else
          inFIFO_queue( &( node->th_for_in ), (char *)thread );
      }
      break;

    default:
      panic( "Communication policy not implemented\n" );
  }

  /* Link wait accounting */
  START_LINK_WAIT_TIME( thread );
}


void link_busy( struct t_thread *thread, struct t_node *node, int in_out )
{
  /* Aquesta es la versio normal, es a dir, insertant amb FIFO */
  link_busy_prioritat( thread, node, in_out, FALSE );
}

void link_busy_primer( struct t_thread *thread, struct t_node *node, int in_out )
{
  /* Aquesta es la versio insertant amb LIFO */
  link_busy_prioritat( thread, node, in_out, TRUE );
}

#ifndef GESTIO_LINKS_ORIGINAL
/* Nou protocol de gestio de links dins d'una maquina */

/*******************************************************************
 **  Aquesta funcio intenta prendre un link de sortida a alguna
 **  comunicaci� que surti d'aquest node i estigui bloquejada.  Si
 **  en troba alguna li roba el link i l'assigna al thread donat com
 **  a link d'entrada o de sortida en funcio del parametre. En
 **  funcio de "test_only" es fa el que cal o simplement es retorna
 **  un boolea indicant si es podia fer.
 *******************************************************************/
t_boolean roba_out_link( struct t_thread *thread, struct t_node *node, int in_out, t_boolean test_only )
{
  struct t_link *link;
  t_boolean trobat          = FALSE;
  struct t_thread *candidat = TH_NIL, *candidat_encuat = TH_NIL;

  if ( in_out == IN_LINK )
  {
    /* Si es vol robar un link de sortida per aconseguir un link
       d'entrada al node, el node ha de ser HALF Duplex per for�a. */
    if ( node->half_duplex_links == FALSE )
      return ( FALSE );
  }


  /* El primer que cal �s trobar un link que compleixi les
     condicions necessaries per ser robat. */

  /* Es recorren els links de sortida ocupats */
  for ( link = (struct t_link *)head_queue( &( node->busy_out_links ) ); link != (struct t_link *)0;
        link = (struct t_link *)next_queue( &( node->busy_out_links ) ) )
  {
    candidat = link->thread;

    /* Nomes s'agafen els links origen d'una comunicaci�, que seran
       els que tinguin un propietari associat (per evitar prendre
       el link de desti en un node half duplex). */
    if ( candidat == TH_NIL )
      continue;

    /* Una verificacio mes que el link es l'origen */
    if ( candidat->local_link != link )
      continue;

    /* Es comprova que esta bloquejat esperant el desti */

    if ( candidat->partner_link != L_NIL )
      continue;

    /* El thread candidat ha d'estar encuat esperant link
       d'entrada a algun node. Cal treure'l d'aquesta cua i
       afegir-lo a l'espera de sortida perqu� li pendrem el link*/
    for ( candidat_encuat = (struct t_thread *)head_queue( &( candidat->partner_node->th_for_in ) ); candidat_encuat != (struct t_thread *)0;
          candidat_encuat = (struct t_thread *)next_queue( &( candidat->partner_node->th_for_in ) ) )
    {
      if ( candidat_encuat == candidat )
        break;
    }
    /* S'ha d'haver trobat per for�a */
    ASSERT( candidat_encuat == candidat );

    /* Si nomes era un test cal retornar sense tocar res */
    if ( test_only )
      return ( TRUE );

    /* Es treu el thread de la cua d'espera d'in_link */
    extract_from_queue( &( candidat->partner_node->th_for_in ), (char *)candidat_encuat );

    /* S'assigna el link robat al thread que el volia */
    if ( in_out == OUT_LINK )
    {
      link->thread          = thread;
      thread->local_link    = link;
      thread->local_hd_link = candidat->local_hd_link; /* Pot ser L_NIL */
    }
    else
    {
      link->thread            = TH_NIL; /* Perque ha estat robat com a hd d'un d'entrada */
      thread->partner_hd_link = link;
      thread->partner_link    = candidat->local_hd_link; /* Ha de ser d'entrada i existir */
    }

    /* Se li pren el link de sortida (i el seu possible partner) */
    candidat->local_link    = L_NIL;
    candidat->local_hd_link = L_NIL;

    /* S'encua en espera, pero passa al davant dels altres de la cua */
    link_busy_primer( candidat, node, OUT_LINK );

    /* Es mostra que s'ha robat el link */
    if ( debug & D_LINKS )
    {
      PRINT_TIMER( current_time );
      printf( ": Link from P%02d T%02d (t%02d) borrowed by P%02d T%02d (t%02d) on node %d\n",
              IDENTIFIERS( candidat ),
              IDENTIFIERS( thread ),
              node->nodeid );
    }

    /* Cal trencar el bucle perqu� ja s'ha pogut robar un link */
    trobat = TRUE;
    break;
  }

  return ( trobat );
}


/*******************************************************************
 **  Aquesta funcio intenta obtenir els links entre nodes d'una
 **  mateixa m�quina per una comunicaci�. Ha estat modificada per
 **  tal que el link de sortida pugui ser robat en cas que s'estigui
 **  esperant link d'entrada. Aquesta funci� �s la que fa �s del
 **  robatori de links en cas que en faltin.
 *******************************************************************/
t_boolean LINKS_get_network_links( struct t_thread *thread, struct t_node *node, struct t_node *node_partner )
{
  struct t_link *link;
  struct t_cpu *cpu_partner;
  struct t_cpu *cpu;
  t_boolean link_robat;
  t_boolean out_link_robable = FALSE, in_link_robable = FALSE;


  cpu         = get_cpu_of_thread( thread );
  cpu_partner = (struct t_cpu *)head_queue( &node_partner->Cpus );

  if ( node == node_partner )
  {
    return ( TRUE );
  }

  /* Em guardo el node desti de la comunicacio per facilitar el robatori de
    links de comunicacions en espera. */
  thread->partner_node = node_partner;

  /* Com que s'utilitza el robatori de links de sortida, sempre s'agafen
    primer els links de sortida encara que un dels nodes sigui half duplex. */

  /* S'obt� primer el LINK de SORTIDA */
  /************************************/
  if ( node->infinite_net_links != TRUE )
  {
    link = thread->local_link;

    if ( link == L_NIL )
    {
      link = (struct t_link *)outFIFO_queue( &( node->free_out_links ) );
    }

    if ( link == L_NIL )
    {
      /* Si no hi ha link lliure es mira si es pot robar un link ocupat */
      out_link_robable = roba_out_link( thread, node, OUT_LINK, TRUE );

      if ( out_link_robable == FALSE )
      {
        /* Si no hi ha link ens hem d'esperar */
        link_busy( thread, node, OUT_LINK );

        if ( debug & D_LINKS )
        {
          PRINT_TIMER( current_time );
          printf( ": GET LINKS\tP%02d T%02d (t%02d) Unable to get OUT link for node %d and %d\n",
                  IDENTIFIERS( thread ),
                  node->nodeid,
                  node_partner->nodeid );
        }
        return ( FALSE );
      }
    }
    else
    {
      /* Hem obtingut un link lliure o ja el teniem d'abans */
      if ( thread->local_link == L_NIL )
      {
        /* Ens apropiem el link que estava lliure */
        link->thread = thread;

        if ( node->half_duplex_links )
        {
          /* If the output has been obtained, it MUST be an input free */
          thread->local_hd_link = (struct t_link *)outFIFO_queue( &( node->free_in_links ) );

          if ( thread->local_hd_link == L_NIL )
          {
            panic( "Unable to obtain the Half Duplex link for input for P%02d T%02d (t%02d)\n", IDENTIFIERS( thread ) );
          }

          thread->local_hd_link->thread = TH_NIL;

          inFIFO_queue( &( node->busy_in_links ), (char *)thread->local_hd_link );
        }

        ASS_ALL_TIMER( link->assigned_on, current_time );
        inFIFO_queue( &( node->busy_out_links ), (char *)link );

        thread->local_link = link;
      }
    }
  }

  /* Despr�s s'obt� el LINK de d'ENTRADA al dest� */
  /************************************************/
  if ( node_partner->infinite_net_links != TRUE )
  {
    link = thread->partner_link;

    if ( link == L_NIL )
    {
      link = (struct t_link *)outFIFO_queue( &( node_partner->free_in_links ) );
    }

    if ( link == L_NIL )
    {
      /* Si no hi ha link lliure es mira si es pot robar un link ocupat */
      in_link_robable = roba_out_link( thread, node_partner, IN_LINK, TRUE );

      if ( in_link_robable == FALSE )
      {
        /* No es pot obtenir link d'entrada! */
        if ( !out_link_robable )
        {
          /* Si teniem el link de sortida cal esperar el link d'entrada */
          link_busy( thread, node_partner, IN_LINK );

          if ( debug & D_LINKS )
          {
            PRINT_TIMER( current_time );
            printf( ": GET LINKS\tP%02d T%02d (t%02d) Unable to get IN link for nodes %d and %d\n",
                    IDENTIFIERS( thread ),
                    node->nodeid,
                    node_partner->nodeid );
          }
          return ( FALSE );
        }
        else
        {
          /* Si no el teniem, pero es podia robar, no val la pena fer-ho.
            Cal esperar link de sortida. */
          link_busy( thread, node, OUT_LINK );
          if ( debug & D_LINKS )
          {
            PRINT_TIMER( current_time );
            printf( ": GET LINKS\tP%02d T%02d (t%02d) Unable to get OUT link for node %d and %d\n",
                    IDENTIFIERS( thread ),
                    node->nodeid,
                    node_partner->nodeid );
          }
          return ( FALSE );
        }
      }
      else
      {
        /* Es pot robar link d'entrada! */
        if ( out_link_robable )
        {
          /* No tenim link de sortida, pero es pot robar */
          link_robat = roba_out_link( thread, node, OUT_LINK, FALSE );
          ASSERT( link_robat == TRUE );
        }

        /* Ara que tenim link de sortida ja es pot robar el d'entrada */
        link_robat = roba_out_link( thread, node_partner, IN_LINK, FALSE );
        ASSERT( link_robat == TRUE );
      }
    }
    else
    {
      /* Hem obtingut un link lliure o ja el teniem d'abans */
      if ( out_link_robable )
      {
        /* No tenim link de sortida, pero es pot robar */
        link_robat = roba_out_link( thread, node, OUT_LINK, FALSE );
        ASSERT( link_robat == TRUE );
      }

      if ( thread->partner_link == L_NIL )
      {
        link->thread = TH_NIL;

        if ( node_partner->half_duplex_links )
        {
          /* If the output has been obtained, it MUST be an input free */
          thread->partner_hd_link = (struct t_link *)outFIFO_queue( &( node_partner->free_out_links ) );

          if ( thread->partner_hd_link == L_NIL )
          {
            panic( "Unable to obtain the Half Duplex link for output for P%02d T%02d (t%02d)\n", IDENTIFIERS( thread ) );
          }

          thread->partner_hd_link->thread = TH_NIL;
          inFIFO_queue( &( node_partner->busy_out_links ), (char *)thread->partner_hd_link );
        }
        ASS_ALL_TIMER( link->assigned_on, current_time );
        inFIFO_queue( &( node_partner->busy_in_links ), (char *)link );
        thread->partner_link = link;
      }
    }
  }

  /* Es mostra que s'han obtingut els links */
  if ( debug & D_LINKS )
  {
    PRINT_TIMER( current_time );
    printf( ": GET LINKS\tP%02d T%02d (t%02d) Links reserved between node %d and node %d\n",
            IDENTIFIERS( thread ),
            node->nodeid,
            node_partner->nodeid );
  }

  /* Ja no cal guardar el node desti de la comunicacio perque si ja te els dos
    links no poden ser robats. */
  thread->partner_node = N_NIL;

  return ( TRUE );
}


#else /* GESTIO_LINKS_ORIGINAL */
/* Protocol original de gestio de links dins d'una maquina */

t_boolean LINKS_get_network_links( struct t_thread *thread, struct t_node *node, struct t_node *node_partner )
{
  struct t_link *link;
  struct t_cpu *cpu_partner;
  struct t_cpu *cpu;

  cpu         = get_cpu_of_thread( thread );
  cpu_partner = (struct t_cpu *)head_queue( &node_partner->Cpus );

  if ( node == node_partner )
  {
    return ( TRUE );
  }


  if ( ( node->half_duplex_links ) || ( node_partner->half_duplex_links ) )
  {
    if ( node->nodeid > node_partner->nodeid )
    {
      goto first_dest;
    }
  }

second_src_link:

  link = thread->local_link;

  if ( link == L_NIL )
  {
    link = (struct t_link *)outFIFO_queue( &( node->free_out_links ) );
  }

  if ( link == L_NIL )
  {
    link_busy( thread, node, OUT_LINK );
    if ( debug & D_LINKS )
    {
      PRINT_TIMER( current_time );
      printf( ": GET LINKS\tP%02d T%02d (t%02d) Unable to get OUT link for nodes %d and %d\n",
              IDENTIFIERS( thread ),
              node->nodeid,
              node_partner->nodeid );
    }
    return ( FALSE );
  }
  else
  {
    if ( thread->local_link == L_NIL )
    {
      if ( node->half_duplex_links )
      {
        /* If the output has been obtained, it MUST be an input free */
        thread->local_hd_link = (struct t_link *)outFIFO_queue( &( node->free_in_links ) );

        if ( thread->local_hd_link == L_NIL )
        {
          panic( "Unable to obtain the input Half Duplex link for P%02d T%02d (t%02d)\n", IDENTIFIERS( thread ) );
        }
        inFIFO_queue( &( node->busy_in_links ), (char *)thread->local_hd_link );
      }
      ASS_ALL_TIMER( link->assigned_on, current_time );
      inFIFO_queue( &( node->busy_out_links ), (char *)link );
    }
    thread->local_link = link;

    if ( ( node->half_duplex_links ) || ( node_partner->half_duplex_links ) )
    {
      if ( node->nodeid > node_partner->nodeid )
      {
        goto end_get_links;
      }
    }

  first_dest:
    link = thread->partner_link;

    if ( link == L_NIL )
    {
      link = (struct t_link *)outFIFO_queue( &( node_partner->free_in_links ) );
    }

    if ( link == L_NIL )
    {
      link_busy( thread, node_partner, IN_LINK );

      if ( debug & D_LINKS )
      {
        PRINT_TIMER( current_time );
        printf( ": GET LINKS\tP%02d T%02d (t%02d) Unable to get in link for nodes %d and %d\n",
                IDENTIFIERS( thread ),
                node->nodeid,
                node_partner->nodeid );
      }
      return ( FALSE );
    }
    else
    {
      if ( thread->partner_link == L_NIL )
      {
        if ( node_partner->half_duplex_links )
        {
          /* If the output has been obtained, it MUST be an input free */
          thread->partner_hd_link = (struct t_link *)outFIFO_queue( &( node_partner->free_out_links ) );

          if ( thread->partner_hd_link == L_NIL )
          {
            panic( "Unable to obtain the output Half Duplex link for P%02d T%02d (t%02d)\n", IDENTIFIERS( thread ) );
          }

          inFIFO_queue( &( node_partner->busy_out_links ), (char *)thread->partner_hd_link );
        }
        ASS_ALL_TIMER( link->assigned_on, current_time );
        inFIFO_queue( &( node_partner->busy_in_links ), (char *)link );
      }
    }
    thread->partner_link = link;

    if ( ( node->half_duplex_links ) || ( node_partner->half_duplex_links ) )
    {
      if ( node->nodeid > node_partner->nodeid )
      {
        goto second_src_link;
      }
    }

  end_get_links:

    if ( debug & D_LINKS )
    {
      PRINT_TIMER( current_time );
      printf( ": GET LINKS\tP%02d T%02d (t%02d) Links reserved between node %d and node %d\n",
              IDENTIFIERS( thread ),
              node->nodeid,
              node_partner->nodeid );
    }

    /* Ja no cal guardar el node desti de la comunicacio perque si ja te els dos
     * links no poden ser robats. */
    thread->partner_node = N_NIL;
    return ( TRUE );
  }
}

#endif /* GESTIO_LINKS_ORIGINAL */

t_boolean LINKS_get_port_links( struct t_thread *thread_s, struct t_node *node_s, struct t_thread *thread_r, struct t_node *node_r )
{
  if ( node_s == node_r )
  {
    thread_s->port_send_link = L_NUL;
    thread_r->port_recv_link = L_NUL;
    return ( TRUE );
  }

  if ( thread_s->port_send_link == L_NIL )
  {
    thread_s->port_send_link = (struct t_link *)outFIFO_queue( &( node_s->free_out_links ) );
    if ( thread_s->port_send_link == L_NIL )
    {
      return ( FALSE );
    }
  }

  if ( thread_r->port_recv_link == L_NIL )
  {
    thread_r->port_recv_link = (struct t_link *)outFIFO_queue( &( node_r->free_in_links ) );
    if ( thread_r->port_recv_link == L_NIL )
    {
      return ( FALSE );
    }
  }
  return ( TRUE );
}

t_boolean LINKS_get_memory_copy_links( struct t_thread *thread, struct t_node *node_s, struct t_node *node_d )
{
  if ( node_s == node_d )
  {
    thread->copy_segment_link_source = L_NUL;
    thread->copy_segment_link_dest   = L_NUL;
    return ( TRUE );
  }

  if ( thread->copy_segment_link_source == L_NIL )
  {
    thread->copy_segment_link_source = (struct t_link *)outFIFO_queue( &( node_s->free_out_links ) );
    if ( thread->copy_segment_link_source == L_NIL )
    {
      return ( FALSE );
    }
  }
  if ( thread->copy_segment_link_dest == L_NIL )
  {
    thread->copy_segment_link_dest = (struct t_link *)outFIFO_queue( &( node_d->free_in_links ) );
    if ( thread->copy_segment_link_dest == L_NIL )
    {
      return ( FALSE );
    }
  }
  return ( TRUE );
}

void LINKS_free_network_link( struct t_link *link, struct t_thread *thread )
{
  struct t_thread *first;
  struct t_node *node;
  struct t_cpu *cpu;
  struct t_both *both;
  struct t_copyseg *copyseg;

  cpu  = get_cpu_of_thread( thread );
  node = link->info.node;
  if ( link->type == IN_LINK )
  {
    if ( debug & D_LINKS )
    {
      PRINT_TIMER( current_time );
      printf( ": Free input links for P%02d T%02d (t%02d) node %d\n", IDENTIFIERS( thread ), node->nodeid );
    }
    inFIFO_queue( &( node->free_in_links ), (char *)link );
    extract_from_queue( &( node->busy_in_links ), (char *)link );

    if ( node->half_duplex_links )
    {
      if ( debug & D_LINKS )
      {
        PRINT_TIMER( current_time );
        printf( ": Free output links for P%02d T%02d (t%02d) node %d\n", IDENTIFIERS( thread ), node->nodeid );
      }
      inFIFO_queue( &( node->free_out_links ), (char *)thread->partner_hd_link );
      extract_from_queue( &( node->busy_out_links ), (char *)thread->partner_hd_link );
      first = (struct t_thread *)outFIFO_queue( &( node->th_for_out ) );

      if ( first != TH_NIL )
      {
        /* FEC: El primer que faig es acumular el temps que el thread ha
                  estat bloquejat esperant el link */
        ACCUMULATE_LINK_WAIT_TIME( first );
        /***************************************************************/

        if ( first->original_thread )
        {
          first->last_paraver = current_time;
        }
        if ( debug & D_LINKS )
        {
          PRINT_TIMER( current_time );
          printf( ": free out link full duplex wakeup P%02d T%02d (t%02d) node %d\n", IDENTIFIERS( first ), node->nodeid );
        }

        if ( first->action->action == SEND )
        {
          really_send( first );
        }
        else if ( first->action->action == MPI_OS )
        {
          really_RMA( first );
        }
      }
    }

    first = (struct t_thread *)outFIFO_queue( &( node->th_for_in ) );

    if ( first != TH_NIL )
    {
      /* FEC: El primer que faig es acumular el temps que el thread ha estat
       * bloquejat esperant el link */
      ACCUMULATE_LINK_WAIT_TIME( first );
      /***************************************************************/

      if ( first->original_thread )
      {
        first->last_paraver = current_time;
      }
      if ( debug & D_LINKS )
      {
        PRINT_TIMER( current_time );
        printf( ": free in link wakeup P%02d T%02d (t%02d) node %d\n", IDENTIFIERS( first ), node->nodeid );
      }
      if ( first->action->action == SEND )
      {
        really_send( first );
      }
      else if ( first->action->action == MPI_OS )
      {
        really_RMA( first );
      }

      return;
    }

    both = (struct t_both *)outFIFO_queue( &( node->wait_inlink_port ) );
    if ( both != B_NIL )
    {
      if ( debug & D_LINKS )
      {
        PRINT_TIMER( current_time );
        printf( ": free link wakeup port communication\n" );
      }
      really_port_send( both->port, both->thread_s, both->thread_r );
      free( both );
      return;
    }

    copyseg = (struct t_copyseg *)outFIFO_queue( &( node->wait_in_copy_segment ) );
    if ( copyseg != CO_NIL )
    {
      if ( debug & D_LINKS )
      {
        PRINT_TIMER( current_time );
        printf( ": free link wakeup copy segment\n" );
      }
      really_copy_segment( copyseg->thread, copyseg->node_s, copyseg->node_d, copyseg->thread->copy_segment_size );
      free( copyseg );
      return;
    }
  }
  else
  {
    if ( debug & D_LINKS )
    {
      PRINT_TIMER( current_time );
      printf( ": Free output links for P%02d T%02d (t%02d) node %d\n", IDENTIFIERS( thread ), node->nodeid );
    }
    inFIFO_queue( &( node->free_out_links ), (char *)link );

    extract_from_queue( &( node->busy_out_links ), (char *)link );
    if ( node->half_duplex_links )
    {
      if ( debug & D_LINKS )
      {
        PRINT_TIMER( current_time );
        printf( ": Free input links for P%02d T%02d (t%02d) node %d\n", IDENTIFIERS( thread ), node->nodeid );
      }
      inFIFO_queue( &( node->free_in_links ), (char *)thread->local_hd_link );
      extract_from_queue( &( node->busy_in_links ), (char *)thread->local_hd_link );
      first = (struct t_thread *)outFIFO_queue( &( node->th_for_in ) );

      if ( first != TH_NIL )
      {
        /* FEC: El primer que faig es acumular el temps que el thread ha estat
         * bloquejat esperant el link */
        ACCUMULATE_LINK_WAIT_TIME( first );
        /***************************************************************/

        if ( first->original_thread )
        {
          first->last_paraver = current_time;
        }
        if ( debug & D_LINKS )
        {
          PRINT_TIMER( current_time );
          printf( ": free in link full duplex wakeup P%02d T%02d (t%02d) node %d\n", IDENTIFIERS( first ), node->nodeid );
        }
        if ( first->action->action == SEND )
          really_send( first );
        else if ( first->action->action == MPI_OS )
          really_RMA( first );
      }
    }
    first = (struct t_thread *)outFIFO_queue( &( node->th_for_out ) );

    if ( first != TH_NIL )
    {
      /* FEC: El primer que faig es acumular el temps que el thread ha estat
       * bloquejat esperant el link */
      ACCUMULATE_LINK_WAIT_TIME( first );
      /***************************************************************/

      if ( first->original_thread )
      {
        first->last_paraver = current_time;
      }
      if ( debug & D_LINKS )
      {
        PRINT_TIMER( current_time );
        printf( ": free out link wakeup P%02d T%02d (t%02d) node %d\n", IDENTIFIERS( first ), node->nodeid );
      }
      if ( first->action->action == SEND ) 
        really_send( first );
      else if ( first->action->action == MPI_OS )
        really_RMA( first );

      return;
    }

    both = (struct t_both *)outFIFO_queue( &( node->wait_outlink_port ) );
    if ( both != B_NIL )
    {
      if ( debug & D_LINKS )
      {
        PRINT_TIMER( current_time );
        printf( ": free link wakeup port communication\n" );
      }
      really_port_send( both->port, both->thread_s, both->thread_r );
      free( both );
      return;
    }

    copyseg = (struct t_copyseg *)outFIFO_queue( &( node->wait_out_copy_segment ) );
    if ( copyseg != CO_NIL )
    {
      if ( debug & D_LINKS )
      {
        PRINT_TIMER( current_time );
        printf( ": free link wakeup copy segment\n" );
      }
      really_copy_segment( copyseg->thread, copyseg->node_s, copyseg->node_d, copyseg->thread->copy_segment_size );
      free( copyseg );
      return;
    }
  }
}

/*****************************************************************************
 *****************************************************************************
 ****
 ****    ######  #    #   #####  ######  #####   #    #    ##    #
 ****    #        #  #      #    #       #    #  ##   #   #  #   #
 ****    #####     ##       #    #####   #    #  # #  #  #    #  #
 ****    #         ##       #    #       #####   #  # #  ######  #
 ****    #        #  #      #    #       #   #   #   ##  #    #  #
 ****    ######  #    #     #    ######  #    #  #    #  #    #  ######
 ****
 ****
 ****        #    #  ######   #####  #    #   ####   #####   #    #
 ****        ##   #  #          #    #    #  #    #  #    #  #   #
 ****        # #  #  #####      #    #    #  #    #  #    #  ####
 ****        #  # #  #          #    # ## #  #    #  #####   #  #
 ****        #   ##  #          #    ##  ##  #    #  #   #   #   #
 ****        #    #  ######     #    #    #   ####   #    #  #    #
 ****
 ****
 ****               ##   #     #    #    #     #   ##
 ****              #     #  #  #   # #   ##    #     #
 ****             #      #  #  #  #   #  # #   #      #
 ****             #      #  #  # #     # #  #  #      #
 ****             #      #  #  # ####### #   # #      #
 ****              #     #  #  # #     # #    ##     #
 ****               ##    ## ##  #     # #     #   ##
 ****
 ****
 ****               #          #    #    #  #    #   ####
 ****               #          #    ##   #  #   #   #
 ****               #          #    # #  #  ####     ####
 ****               #          #    #  # #  #  #         #
 ****               #          #    #   ##  #   #   #    #
 ****               ######     #    #    #  #    #   ####
 ****
 *****************************************************************************
 *****************************************************************************/

void machine_link_busy( struct t_thread *thread, struct t_machine *machine, int in_out )
{
  switch ( machine->communication.policy )
  {
    case COMMUNIC_FIFO:
    case COMMUNIC_RR:
    case COMMUNIC_BOOST:
      if ( in_out == OUT_LINK )
      {
        inFIFO_queue( &( machine->external_net.th_for_out ), (char *)thread );
      }
      else
      {
        inFIFO_queue( &( machine->external_net.th_for_in ), (char *)thread );
      }
      break;
    default:
      panic( "Communication policy not implemented\n" );
  }

  /* FEC: Afegeixo aixo per poder contar el temps que un thread passa
          bloquejat esperant un link */
  START_LINK_WAIT_TIME( thread );
  /*******************************************************************/
}


t_boolean LINKS_get_wan_links( struct t_thread *thread, struct t_machine *s_machine, struct t_machine *d_machine )
{
  struct t_link *link;

  /* Aixo ara ja no �s un error, perqu� a les operacions col.lectives que
     utilitzen la xarxa externa, els threads reserven tots els links de la
     seva mateixa maquina.
     if (s_machine == d_machine) panic("Trying to get links for the same machine!\n");
  */

  if ( ( s_machine->external_net.half_duplex_links ) || ( d_machine->external_net.half_duplex_links ) )
  {
    if ( s_machine->id > d_machine->id )
      goto first_dest;
  }

second_src_link:
  link = thread->local_link;
  if ( link == L_NIL )
    link = (struct t_link *)outFIFO_queue( &( s_machine->external_net.free_out_links ) );
  if ( link == L_NIL )
  {
    machine_link_busy( thread, s_machine, OUT_LINK );
    if ( debug & D_LINKS )
    {
      PRINT_TIMER( current_time );
      printf( ": Get machine links for P%02d T%02d (t%02d) unable to get out link for machine %d %d\n",
              IDENTIFIERS( thread ),
              s_machine->id,
              d_machine->id );
    }
    return ( FALSE );
  }
  else
  {
    if ( thread->local_link == L_NIL )
    {
      if ( s_machine->external_net.half_duplex_links )
      {
        /* If the output has been obtained, it MUST be an input free */
        thread->local_hd_link = (struct t_link *)outFIFO_queue( &( s_machine->external_net.free_in_links ) );
        if ( thread->local_hd_link == L_NIL )
          panic( "Unable to obtain the Half Duplex machine link for input for P%02d T%02d (t%02d)\n", IDENTIFIERS( thread ) );
        inFIFO_queue( &( s_machine->external_net.busy_in_links ), (char *)thread->local_hd_link );
      }
      ASS_ALL_TIMER( link->assigned_on, current_time );
      inFIFO_queue( &( s_machine->external_net.busy_out_links ), (char *)link );
    }
  }
  thread->local_link = link;

  if ( ( s_machine->external_net.half_duplex_links ) || ( d_machine->external_net.half_duplex_links ) )
  {
    if ( s_machine->id > d_machine->id )
      goto end_get_links;
  }

first_dest:
  link = thread->partner_link;
  if ( link == L_NIL )
    link = (struct t_link *)outFIFO_queue( &( d_machine->external_net.free_in_links ) );
  if ( link == L_NIL )
  {
    machine_link_busy( thread, d_machine, IN_LINK );
    if ( debug & D_LINKS )
    {
      PRINT_TIMER( current_time );
      printf( ": Get machine links for P%02d T%02d (t%02d) unable to get in link for machines %d %d\n",
              IDENTIFIERS( thread ),
              s_machine->id,
              d_machine->id );
    }
    return ( FALSE );
  }
  else
  {
    if ( thread->partner_link == L_NIL )
    {
      if ( d_machine->external_net.half_duplex_links )
      {
        /* If the input has been obtained, it MUST be an output free */
        thread->partner_hd_link = (struct t_link *)outFIFO_queue( &( d_machine->external_net.free_out_links ) );
        if ( thread->partner_hd_link == L_NIL )
          panic( "Unable to obtain the Half Duplex machine link for output for P%02d T%02d (t%02d)\n", IDENTIFIERS( thread ) );
        inFIFO_queue( &( d_machine->external_net.busy_out_links ), (char *)thread->partner_hd_link );
      }
      ASS_ALL_TIMER( link->assigned_on, current_time );
      inFIFO_queue( &( d_machine->external_net.busy_in_links ), (char *)link );
    }
  }
  thread->partner_link = link;

  if ( ( s_machine->external_net.half_duplex_links ) || ( d_machine->external_net.half_duplex_links ) )
  {
    if ( s_machine->id > d_machine->id )
      goto second_src_link;
  }

end_get_links:

  if ( debug & D_LINKS )
  {
    PRINT_TIMER( current_time );
    printf( ": Get machine links for P%02d T%02d (t%02d) machine %d  and machine %d\n", IDENTIFIERS( thread ), s_machine->id, d_machine->id );
  }

  return ( TRUE );
}

t_boolean LINKS_get_single_wan_link( struct t_thread *thread, struct t_machine *machine, int in_out )
{
  struct t_link *link;

  if ( machine->external_net.half_duplex_links )
  {
    PRINT_TIMER( current_time );
    printf( ": GLOBAL_operation P%02d T%02d (t%02d) get half duplex link on machine %d\n", IDENTIFIERS( thread ), machine->id );
  }

  if ( in_out == OUT_LINK ) /* Es vol un link de sortida (OUT_LINK) */
  {
    link = thread->local_link;

    if ( link == L_NIL )
    {
      link = (struct t_link *)outFIFO_queue( &( machine->external_net.free_out_links ) );
    }

    if ( link == L_NIL )
    {
      machine_link_busy( thread, machine, OUT_LINK );

      if ( debug & D_LINKS )
      {
        PRINT_TIMER( current_time );
        printf( ": GLOBAL_operation P%02d T%02d (t%02d) unable to get out link for machine %d\n", IDENTIFIERS( thread ), machine->id );
      }
      return ( FALSE );
    }
    else
    {
      if ( thread->local_link == L_NIL )
      {
        /* De moment no hi poden haver half duplex */
        if ( machine->external_net.half_duplex_links )
        {
          /* If the output has been obtained, it MUST be an input free */
          thread->local_hd_link = (struct t_link *)outFIFO_queue( &( machine->external_net.free_in_links ) );

          if ( thread->local_hd_link == L_NIL )
          {
            panic( "Unable to obtain the Half Duplex machine link for input for P%02d T%02d (t%02d)\n", IDENTIFIERS( thread ) );
          }
          inFIFO_queue( &( machine->external_net.busy_in_links ), (char *)thread->local_hd_link );
        }
        ASS_ALL_TIMER( link->assigned_on, current_time );
        inFIFO_queue( &( machine->external_net.busy_out_links ), (char *)link );
      }
    }
    thread->local_link = link;
  }
  else if ( in_out == IN_LINK ) /* Es vol el link d'entrada (IN_LINK)*/
  {
    link = thread->partner_link;

    if ( link == L_NIL )
    {
      link = (struct t_link *)outFIFO_queue( &( machine->external_net.free_in_links ) );
    }

    if ( link == L_NIL )
    {
      machine_link_busy( thread, machine, IN_LINK );
      if ( debug & D_LINKS )
      {
        PRINT_TIMER( current_time );
        printf( ": GLOBAL_operation P%02d T%02d (t%02d) unable to get out link for machine %d\n", IDENTIFIERS( thread ), machine->id );
      }
      return ( FALSE );
    }
    else
    {
      if ( thread->partner_link == L_NIL )
      {
        /* De moment no hi poden haver half duplex */
        if ( machine->external_net.half_duplex_links )
        {
          /* If the input has been obtained, it MUST be an output free */
          thread->partner_hd_link = (struct t_link *)outFIFO_queue( &( machine->external_net.free_out_links ) );

          if ( thread->partner_hd_link == L_NIL )
          {
            panic( "Unable to obtain the Half Duplex machine link for output for P%02d T%02d (t%02d)\n", IDENTIFIERS( thread ) );
          }
          inFIFO_queue( &( machine->external_net.busy_out_links ), (char *)thread->partner_hd_link );
        }
        ASS_ALL_TIMER( link->assigned_on, current_time );
        inFIFO_queue( &( machine->external_net.busy_in_links ), (char *)link );
      }
    }
    thread->partner_link = link;
  }

  if ( debug & D_LINKS )
  {
    PRINT_TIMER( current_time );
    if ( in_out == OUT_LINK )
    {
      printf( ": GLOBAL_operation P%02d T%02d (t%02d) gets output link on machine %d\n", IDENTIFIERS( thread ), machine->id );
    }
    else if ( in_out == IN_LINK )
    {
      printf( ": GLOBAL_operation P%02d T%02d (t%02d) gets input link on machine %d\n", IDENTIFIERS( thread ), machine->id );
    }
  }
  return ( TRUE );
}

void LINKS_free_wan_link( struct t_link *link, struct t_thread *thread )
{
  struct t_thread *first;
  struct t_machine *machine;


  machine = link->info.machine;
  if ( link->type == IN_LINK )
  {
    if ( debug & D_LINKS )
    {
      PRINT_TIMER( current_time );
      printf( ": Free input links for P%02d T%02d (t%02d) machine %d\n", IDENTIFIERS( thread ), machine->id );
    }
    inFIFO_queue( &( machine->external_net.free_in_links ), (char *)link );

    extract_from_queue( &( machine->external_net.busy_in_links ), (char *)link );
    if ( machine->external_net.half_duplex_links )
    {
      if ( debug & D_LINKS )
      {
        PRINT_TIMER( current_time );
        printf( ": Free output links for P%02d T%02d (t%02d) machine %d\n", IDENTIFIERS( thread ), machine->id );
      }
      inFIFO_queue( &( machine->external_net.free_out_links ), (char *)thread->partner_hd_link );
      extract_from_queue( &( machine->external_net.busy_out_links ), (char *)thread->partner_hd_link );
      first = (struct t_thread *)outFIFO_queue( &( machine->external_net.th_for_out ) );

      if ( first != TH_NIL )
      {
        /* FEC: El primer que faig es acumular el temps que el thread ha
                estat bloquejat esperant el link */
        ACCUMULATE_LINK_WAIT_TIME( first );
        /***************************************************************/

        if ( first->original_thread )
        {
          first->last_paraver = current_time;
        }

        if ( debug & D_LINKS )
        {
          PRINT_TIMER( current_time );
          printf( ": free out link full duplex wakeup P%02d T%02d (t%02d) machine %d\n", IDENTIFIERS( first ), machine->id );
        }

        if ( first->action->action == SEND )
        {
          really_send( first );
        }
        else if ( first->action->action == MPI_OS )
        {
          really_RMA( first );
        }
        else if ( first->action->action == GLOBAL_OP )
        {
          global_op_reserva_links( first );
        }
      }
    }

    first = (struct t_thread *)outFIFO_queue( &( machine->external_net.th_for_in ) );
    if ( first != TH_NIL )
    {
      /* FEC: El primer que faig es acumular el temps que el thread ha
              estat bloquejat esperant el link */
      ACCUMULATE_LINK_WAIT_TIME( first );
      /***************************************************************/

      if ( first->original_thread )
      {
        first->last_paraver = current_time;
      }

      if ( debug & D_LINKS )
      {
        PRINT_TIMER( current_time );
        printf( ": free in link wakeup P%02d T%02d (t%02d) machine %d\n", IDENTIFIERS( first ), machine->id );
      }

      if ( first->action->action == SEND )
      {
        really_send( first );
      }
      else if ( first->action->action == MPI_OS )
      {
        really_RMA( first );
      }
      else if ( first->action->action == GLOBAL_OP )
      {
        global_op_reserva_links( first );
      }

      return;
    }

    /* Si es vol poder utilitzar PORTS entre maquines, s'hauria de fer aqui */
  }
  else
  {
    if ( debug & D_LINKS )
    {
      PRINT_TIMER( current_time );
      printf( ": Free output links for P%02d T%02d (t%02d) machine %d\n", IDENTIFIERS( thread ), machine->id );
    }

    inFIFO_queue( &( machine->external_net.free_out_links ), (char *)link );
    extract_from_queue( &( machine->external_net.busy_out_links ), (char *)link );

    if ( machine->external_net.half_duplex_links )
    {
      if ( debug & D_LINKS )
      {
        PRINT_TIMER( current_time );
        printf( ": Free input links for P%02d T%02d (t%02d) machine %d\n", IDENTIFIERS( thread ), machine->id );
      }
      inFIFO_queue( &( machine->external_net.free_in_links ), (char *)thread->local_hd_link );
      extract_from_queue( &( machine->external_net.busy_in_links ), (char *)thread->local_hd_link );
      first = (struct t_thread *)outFIFO_queue( &( machine->external_net.th_for_in ) );

      if ( first != TH_NIL )
      {
        /* FEC: El primer que faig es acumular el temps que el thread ha
                estat bloquejat esperant el link */
        ACCUMULATE_LINK_WAIT_TIME( first );
        /***************************************************************/

        if ( first->original_thread )
        {
          first->last_paraver = current_time;
        }

        if ( debug & D_LINKS )
        {
          PRINT_TIMER( current_time );
          printf( ": free in link full duplex wakeup P%02d T%02d (t%02d) machine %d\n", IDENTIFIERS( first ), machine->id );
        }

        if ( first->action->action == SEND )
        {
          really_send( first );
        }
        else if ( first->action->action == MPI_OS )
        {
          really_RMA( first );
        }
        else if ( first->action->action == GLOBAL_OP )
        {
          global_op_reserva_links( first );
        }
      }
    }

    first = (struct t_thread *)outFIFO_queue( &( machine->external_net.th_for_out ) );

    if ( first != TH_NIL )
    {
      /* FEC: El primer que faig es acumular el temps que el thread ha
              estat bloquejat esperant el link */
      ACCUMULATE_LINK_WAIT_TIME( first );
      /***************************************************************/

      if ( first->original_thread )
      {
        first->last_paraver = current_time;
      }

      if ( debug & D_LINKS )
      {
        PRINT_TIMER( current_time );
        printf( ": free out link wakeup P%02d T%02d (t%02d) machine %d\n", IDENTIFIERS( first ), machine->id );
      }

      if ( first->action->action == SEND )
      {
        really_send( first );
      }
      else if ( first->action->action == MPI_OS )
      {
        really_RMA( first );
      }
      else if ( first->action->action == GLOBAL_OP )
      {
        global_op_reserva_links( first );
      }

      return;
    }

    /* Si es vol poder utilitzar PORTS entre maquines, s'hauria de fer aqui */
  }
}

/*****************************************************************************
******************************************************************************
*
*     #####   ######  #####      #     ####     ##     #####  ######  #####
*     #    #  #       #    #     #    #    #   #  #      #    #       #    #
*     #    #  #####   #    #     #    #       #    #     #    #####   #    #
*     #    #  #       #    #     #    #       ######     #    #       #    #
*     #    #  #       #    #     #    #    #  #    #     #    #       #    #
*     #####   ######  #####      #     ####   #    #     #    ######  #####
*
*
*  ####    ####   #    #  #    #  ######   ####    #####     #     ####   #    #
* #    #  #    #  ##   #  ##   #  #       #    #     #       #    #    #  ##   #
* #       #    #  # #  #  # #  #  #####   #          #       #    #    #  # #  #
* #       #    #  #  # #  #  # #  #       #          #       #    #    #  #  # #
* #    #  #    #  #   ##  #   ##  #       #    #     #       #    #    #  #   ##
*  ####    ####   #    #  #    #  ######   ####      #       #     ####   #    #
*
*
*                    #          #    #    #  #    #   ####
*                    #          #    ##   #  #   #   #
*                    #          #    # #  #  ####     ####
*                    #          #    #  # #  #  #         #
*                    #          #    #   ##  #   #   #    #
*                    ######     #    #    #  #    #   ####
*
*****************************************************************************
*****************************************************************************/


void connection_link_busy( struct t_thread *thread, struct t_dedicated_connection *connection, int in_out )
{
  /* La politica de les connexions dedicades sempre es FIFO. Si aixo es
     vol canviar alguna vegada, caldra modificar aixo. */

  if ( in_out == OUT_LINK )
  {
    inFIFO_queue( &( connection->th_for_out ), (char *)thread );
  }
  else
  {
    inFIFO_queue( &( connection->th_for_in ), (char *)thread );
  }

  /* FEC: Afegeixo aixo per poder contar el temps que un thread passa
          bloquejat esperant un link */
  START_LINK_WAIT_TIME( thread );
  /*******************************************************************/
}

t_boolean LINKS_get_dedicated_connection_links( struct t_thread *thread, struct t_dedicated_connection *connection )
{
  struct t_link *link;

  if ( ( connection->half_duplex ) && ( connection->source_id > connection->destination_id ) )
  {
    goto first_dest;
  }

second_src_link:

  link = thread->local_link;
  if ( link == L_NIL )
  {
    link = (struct t_link *)outFIFO_queue( &( connection->free_out_links ) );
  }

  if ( link == L_NIL )
  {
    connection_link_busy( thread, connection, OUT_LINK );
    if ( debug & D_LINKS )
    {
      PRINT_TIMER( current_time );
      printf( ": (DEDICATED) Get connection links for P%02d T%02d (t%02d) unable to get out link for connection %d\n",
              IDENTIFIERS( thread ),
              connection->id );
    }

    return FALSE;
  }
  else
  {
    if ( thread->local_link == L_NIL )
    {
      if ( connection->half_duplex )
      {
        /* If the output has been obtained, it MUST be an input free */
        thread->local_hd_link = (struct t_link *)outFIFO_queue( &( connection->free_in_links ) );

        if ( thread->local_hd_link == L_NIL )
        {
          panic( "Unable to obtain the Half Duplex connection link for input for P%02d T%02d (th%02d)\n", IDENTIFIERS( thread ) );
        }

        inFIFO_queue( &( connection->busy_in_links ), (char *)thread->local_hd_link );
      }
      ASS_ALL_TIMER( link->assigned_on, current_time );
      inFIFO_queue( &( connection->busy_out_links ), (char *)link );
    }
  }

  thread->local_link = link;

  if ( ( connection->half_duplex ) && ( connection->source_id > connection->destination_id ) )
  {
    goto end_get_links;
  }


first_dest:

  link = thread->partner_link;

  if ( link == L_NIL )
  {
    link = (struct t_link *)outFIFO_queue( &( connection->free_in_links ) );
  }

  if ( link == L_NIL )
  {
    connection_link_busy( thread, connection, IN_LINK );
    if ( debug & D_LINKS )
    {
      PRINT_TIMER( current_time );
      printf( ": (DEDICATED) Get connection links for P%02d T%02d (t%02d) unable to get in link for connection %d\n",
              IDENTIFIERS( thread ),
              connection->id );
    }

    return FALSE;
  }
  else
  {
    if ( thread->partner_link == L_NIL )
    {
      if ( connection->half_duplex )
      {
        /* If the input has been obtained, it MUST be an output free */
        thread->partner_hd_link = (struct t_link *)outFIFO_queue( &( connection->free_out_links ) );

        if ( thread->partner_hd_link == L_NIL )
        {
          panic( "Unable to obtain the Half Duplex connection link for output for P%02d T%02d (th%02d)\n", IDENTIFIERS( thread ) );
        }

        inFIFO_queue( &( connection->busy_out_links ), (char *)thread->partner_hd_link );
      }

      ASS_ALL_TIMER( link->assigned_on, current_time );
      inFIFO_queue( &( connection->busy_in_links ), (char *)link );
    }
  }

  thread->partner_link = link;

  if ( ( connection->half_duplex ) && ( connection->source_id > connection->destination_id ) )
  {
    goto second_src_link;
  }

end_get_links:

  if ( debug & D_LINKS )
  {
    PRINT_TIMER( current_time );
    printf( ": (DEDICATED) Get connection links for P%02d T%02d (t%02d) connection %d\n", IDENTIFIERS( thread ), connection->id );
  }

  return TRUE;
}

void LINKS_free_dedicated_connection_link( struct t_link *link, struct t_thread *thread )
{
  struct t_thread *first;
  struct t_dedicated_connection *connection;

  connection = link->info.connection;

  if ( link->type == IN_LINK )
  {
    if ( debug & D_LINKS )
    {
      PRINT_TIMER( current_time );
      printf( ": (DEDICATED) Free input links for P%02d T%02d (t%02d) connection %d\n", IDENTIFIERS( thread ), connection->id );
    }
    inFIFO_queue( &( connection->free_in_links ), (char *)link );

    extract_from_queue( &( connection->busy_in_links ), (char *)link );
    if ( connection->half_duplex )
    {
      if ( debug & D_LINKS )
      {
        PRINT_TIMER( current_time );
        printf( ": (DEDICATED) Free output links for P%02d T%02d (t%02d) connection %d\n", IDENTIFIERS( thread ), connection->id );
      }
      inFIFO_queue( &( connection->free_out_links ), (char *)thread->partner_hd_link );
      extract_from_queue( &( connection->busy_out_links ), (char *)thread->partner_hd_link );
      first = (struct t_thread *)outFIFO_queue( &( connection->th_for_out ) );
      if ( first != TH_NIL )
      {
        /* FEC: El primer que faig es acumular el temps que el thread ha
                estat bloquejat esperant el link */
        ACCUMULATE_LINK_WAIT_TIME( first );
        /***************************************************************/

        if ( first->original_thread )
        {
          first->last_paraver = current_time;
        }

        if ( debug & D_LINKS )
        {
          PRINT_TIMER( current_time );
          printf( ": (DEDICATED) free out link full duplex wakeup P%02d T%02d (t%02d) connection %d\n", IDENTIFIERS( first ), connection->id );
        }

        if ( first->action->action == SEND )
        {
          really_send( first );
        }
        else if ( first->action->action == MPI_OS )
        {
          really_RMA( first );
        }
      }
    }

    first = (struct t_thread *)outFIFO_queue( &( connection->th_for_in ) );

    if ( first != TH_NIL )
    {
      /* FEC: El primer que faig es acumular el temps que el thread ha
              estat bloquejat esperant el link */
      ACCUMULATE_LINK_WAIT_TIME( first );
      /***************************************************************/

      if ( first->original_thread )
      {
        first->last_paraver = current_time;
      }

      if ( debug & D_LINKS )
      {
        PRINT_TIMER( current_time );
        printf( ": (DEDICATED) free in link wakeup P%02d T%02d (t%02d) connection %d\n", IDENTIFIERS( first ), connection->id );
      }

      if ( first->action->action == SEND )
      {
        really_send( first );
      }
      else if ( first->action->action == MPI_OS )
      {
        really_RMA( first );
      }
      return;
    }

    /* Si es vol poder utilitzar PORTS amb connexions dedicades,
       s'hauria de fer aqui */
  }
  else
  {
    if ( debug & D_LINKS )
    {
      PRINT_TIMER( current_time );
      printf( ": (DEDICATED) Free output links for P%02d T%02d (t%02d) connection %d\n", IDENTIFIERS( thread ), connection->id );
    }
    inFIFO_queue( &( connection->free_out_links ), (char *)link );
    extract_from_queue( &( connection->busy_out_links ), (char *)link );
    if ( connection->half_duplex )
    {
      if ( debug & D_LINKS )
      {
        PRINT_TIMER( current_time );
        printf( ": (DEDICATED) Free input links for P%02d T%02d (t%02d) connection %d\n", IDENTIFIERS( thread ), connection->id );
      }
      inFIFO_queue( &( connection->free_in_links ), (char *)thread->local_hd_link );
      extract_from_queue( &( connection->busy_in_links ), (char *)thread->local_hd_link );
      first = (struct t_thread *)outFIFO_queue( &( connection->th_for_in ) );
      if ( first != TH_NIL )
      {
        /* FEC: El primer que faig es acumular el temps que el thread ha
                estat bloquejat esperant el link */
        ACCUMULATE_LINK_WAIT_TIME( first );
        /***************************************************************/

        if ( first->original_thread )
        {
          first->last_paraver = current_time;
        }

        if ( debug & D_LINKS )
        {
          PRINT_TIMER( current_time );
          printf( ": (DEDICATED) free in link full duplex wakeup P%02d T%02d (t%02d) connection %d\n", IDENTIFIERS( first ), connection->id );
        }

        if ( first->action->action == SEND )
        {
          really_send( first );
        }
        else if ( first->action->action == MPI_OS )
        {
          really_RMA( first );
        }
      }
    }

    first = (struct t_thread *)outFIFO_queue( &( connection->th_for_out ) );
    if ( first != TH_NIL )
    {
      /* FEC: El primer que faig es acumular el temps que el thread ha
              estat bloquejat esperant el link */
      ACCUMULATE_LINK_WAIT_TIME( first );
      /***************************************************************/

      if ( first->original_thread )
      {
        first->last_paraver = current_time;
      }

      if ( debug & D_LINKS )
      {
        PRINT_TIMER( current_time );
        printf( ": (DEDICATED) free out link wakeup P%02d T%02d (t%02d) connection %d\n", IDENTIFIERS( first ), connection->id );
      }

      if ( first->action->action == SEND )
      {
        really_send( first );
      }
      else if ( first->action->action == MPI_OS )
      {
        really_RMA( first );
      }
      return;
    }

    /* Si es vol poder utilitzar PORTS amb connexions dedicades,
       s'hauria de fer aqui */
  }
}


/*****************************************************************************************************
******************************************************************************************************

*      ##      #####   #####   ######   #        ######    #####      ##     ######   ######   #####
*     #  #    #       #        #        #        #         #    #    #  #       #     #    #   #    #
*    #    #   #       #        ######   #        ######    ######   #    #      #     #    #   ######
*    ######   #       #        #        #        #         #    #   ######      #     #    #   #    #
*    #    #   #       #        #        #        #         #    #   #    #      #     #    #   #    #
*    #    #    #####   #####   ######   ######   ######    #    #   #	   #      #     ######   #    #
*
*
*                    #          #    #    #  #    #   ####
*                    #          #    ##   #  #   #   #
*                    #          #    # #  #  ####     ####
*                    #          #    #  # #  #  #         #
*                    #          #    #   ##  #   #   #    #
*                    ######     #    #    #  #    #   ####
*
******************************************************************************************************
******************************************************************************************************/
void acc_link_busy( struct t_thread *thread, struct t_task *task, int in_out )
{
  struct t_node *node;
  struct t_machine *machine;

  node    = get_node_of_thread( thread );
  machine = node->machine;

  switch ( machine->communication.policy )
  {
    case COMMUNIC_FIFO:
    case COMMUNIC_RR:
    case COMMUNIC_BOOST:
      if ( in_out == OUT_LINK )
      {
        inFIFO_queue( &( task->th_for_out ), (char *)thread );
      }
      else
      {
        inFIFO_queue( &( task->th_for_in ), (char *)thread );
      }
      break;
    default:
      panic( "Communication policy not implemented\n" );
  }

  START_LINK_WAIT_TIME( thread );
}

t_boolean LINKS_get_acc_links( struct t_thread *thread, struct t_task *task_snd, struct t_task *task_rcv )
{
  struct t_link *link;

  if ( ( task_snd->taskid == task_rcv->taskid ) )
  { // It always returns true, because it's inside and accelerator
    return TRUE;
  }
  return FALSE;
  /*if ((task_snd->half_duplex_links) || (task_rcv->half_duplex_links))
  {
    if (task_snd->taskid > task_rcv->taskid)
    {
      goto first_dest;
    }
  }

second_src_link:

  link = thread->local_link;

  if (link == L_NIL)
  {
    link = (struct t_link *) outFIFO_queue (&(task_snd->free_out_links));
  }

  if (link == L_NIL)
  {
    acc_link_busy (thread, task_snd, OUT_LINK);

    if (debug&D_LINKS)
    {
      PRINT_TIMER (current_time);
      printf (": P%02d T%02d (t%02d) no local (OUT) write permissions to task %d\n",
              IDENTIFIERS (thread),
              task_rcv->taskid);
    }
    return FALSE;
  }
  else
  {
    if (thread->local_link == L_NIL)
    {
      if (task_snd->half_duplex_links)
      {
        /* If the output has been obtained, it MUST be an input free */
  /*thread->local_hd_link =
    (struct t_link*) outFIFO_queue(&(task_snd->free_in_links));

  if (thread->local_hd_link==L_NIL)
  {
    panic ("P%02d T%02d (t%02d) unable to obtain destination accelerator process\n",
           IDENTIFIERS(thread));
  }

  inFIFO_queue (&(task_snd->busy_in_links), (char*) thread->local_hd_link);
}
link->assigned_on = current_time;

inFIFO_queue (&(task_snd->busy_out_links), (char*)link);
}
}

thread->local_link = link;

if ((task_snd->half_duplex_links) || (task_rcv->half_duplex_links))
{
if (task_snd->taskid > task_rcv->taskid)
{
goto end_get_links;
}
}

first_dest:

link = thread->partner_link;
if (link == L_NIL)
{
link = (struct t_link *) outFIFO_queue (&(task_rcv->free_in_links));
}

if (link == L_NIL)
{
acc_link_busy (thread, task_rcv, IN_LINK);
if (debug&D_LINKS)
{
PRINT_TIMER (current_time);
printf (": P%02d T%02d (t%02d) no local (IN) write permissions to %d\n",
        IDENTIFIERS (thread),
        task_rcv->taskid);
}
return (FALSE);
}
else
{
if (thread->partner_link == L_NIL)
{
if (task_rcv->half_duplex_links)
{
  /* If the input has been obtained, it MUST be an output free */
  /*thread->partner_hd_link = (struct t_link *) outFIFO_queue (&(task_rcv->free_out_links));

  if (thread->partner_hd_link==L_NIL)
  {
    panic ("P%02d T%02d (t%02d) unable to obtain destination  accelerator process\n",
           IDENTIFIERS(thread));
  }
  inFIFO_queue (&(task_rcv->busy_out_links), (char*)thread->partner_hd_link);
}

link->assigned_on = current_time;
inFIFO_queue (&(task_rcv->busy_in_links), (char*)link);
}
}

thread->partner_link = link;

if ((task_snd->half_duplex_links) || (task_rcv->half_duplex_links))
{
if (task_snd->taskid > task_rcv->taskid)
{
goto second_src_link;
}
}

end_get_links:

if (debug&D_LINKS)
{
PRINT_TIMER (current_time);
printf (": P%02d T%02d (t%02d)  Accelerator process communication available to task %d\n",
      IDENTIFIERS (thread),
      task_rcv->taskid);
}

return TRUE;*/
}

void LINKS_free_acc_link( struct t_link *link, struct t_thread *thread )
{
  struct t_thread *first;
  struct t_task *task;
  struct t_node *node;
  struct t_cpu *cpu;
  struct t_both *both;
  struct t_copyseg *copyseg;

  task = link->info.task;
  cpu  = get_cpu_of_thread( thread );
  node = link->info.node;

  if ( link->type == IN_LINK )
  {
    if ( debug & D_LINKS )
    {
      PRINT_TIMER( current_time );
      printf( ": P%02d T%02d (t%02d) frees writing memory permissions (IN) to task %d\n", IDENTIFIERS( thread ), task->taskid );
    }
    inFIFO_queue( &( task->free_in_links ), (char *)link );
    extract_from_queue( &( task->busy_in_links ), (char *)link );

    if ( task->half_duplex_links )
    {
      if ( debug & D_LINKS )
      {
        PRINT_TIMER( current_time );
        printf( ": P%02d T%02d (t%02d) frees memory writing permissions (OUT) to task %d\n", IDENTIFIERS( thread ), task->taskid );
      }

      inFIFO_queue( &( task->free_out_links ), (char *)thread->partner_hd_link );

      extract_from_queue( &( task->busy_out_links ), (char *)thread->partner_hd_link );

      first = (struct t_thread *)outFIFO_queue( &( task->th_for_out ) );

      if ( first != TH_NIL )
      {
        /* FEC: El primer que faig es acumular el temps que el thread ha
                  estat bloquejat esperant el link */
        ACCUMULATE_LINK_WAIT_TIME( first );
        /***************************************************************/

        if ( first->original_thread )
        {
          first->last_paraver = current_time;
        }
        if ( debug & D_LINKS )
        {
          PRINT_TIMER( current_time );
          printf( ": free memory writing permissions (OUT) to task %d wakes-up P%02d T%02d (t%02d)\n", task->taskid, IDENTIFIERS( first ) );
        }

        if ( first->action->action == SEND )
        {
          really_send( first );
        }
        else if ( first->action->action == MPI_OS )
        {
          really_RMA( first );
        }
      }
    }

    first = (struct t_thread *)outFIFO_queue( &( task->th_for_in ) );

    if ( first != TH_NIL )
    {
      /* FEC: El primer que faig es acumular el temps que el thread ha estat
       * bloquejat esperant el link */
      ACCUMULATE_LINK_WAIT_TIME( first );
      /***************************************************************/

      if ( first->original_thread )
      {
        first->last_paraver = current_time;
      }
      if ( debug & D_LINKS )
      {
        PRINT_TIMER( current_time );
        printf( ": Free memory writing permissions (IN) to task %d wakes-up P%02d T%02d (t%02d)\n", task->taskid, IDENTIFIERS( first ) );
      }
      if ( first->action->action == SEND )
      {
        really_send( first );
      }
      else if ( first->action->action == MPI_OS )
      {
        really_RMA( first );
      }

      return;
    }
  }
  else
  {
    if ( debug & D_LINKS )
    {
      PRINT_TIMER( current_time );
      printf( ": P%02d T%02d (t%02d) frees writing memory permissions (OUT) to task %d\n", IDENTIFIERS( thread ), task->taskid );
    }
    inFIFO_queue( &( task->free_out_links ), (char *)link );

    extract_from_queue( &( task->busy_out_links ), (char *)link );

    if ( task->half_duplex_links )
    {
      if ( debug & D_LINKS )
      {
        PRINT_TIMER( current_time );
        printf( ": P%02d T%02d (t%02d) frees writing memory permissions (IN) to task %d\n", IDENTIFIERS( thread ), task->taskid );
      }
      inFIFO_queue( &( task->free_in_links ), (char *)thread->local_hd_link );
      extract_from_queue( &( task->busy_in_links ), (char *)thread->local_hd_link );
      first = (struct t_thread *)outFIFO_queue( &( task->th_for_in ) );

      if ( first != TH_NIL )
      {
        /* FEC: El primer que faig es acumular el temps que el thread ha estat
         * bloquejat esperant el link */
        ACCUMULATE_LINK_WAIT_TIME( first );
        /***************************************************************/

        if ( first->original_thread )
        {
          first->last_paraver = current_time;
        }
        if ( debug & D_LINKS )
        {
          PRINT_TIMER( current_time );
          printf( ": free memory writing permissions (IN) to task %d wakes-up P%02d T%02d (t%02d)\n", task->taskid, IDENTIFIERS( first ) );
        }
        if ( first->action->action == SEND )
          really_send( first );
        else if ( first->action->action == MPI_OS )
          really_RMA( first );
      }
    }
    first = (struct t_thread *)outFIFO_queue( &( task->th_for_out ) );

    if ( first != TH_NIL )
    {
      /* FEC: El primer que faig es acumular el temps que el thread ha estat
       * bloquejat esperant el link */
      ACCUMULATE_LINK_WAIT_TIME( first );
      /***************************************************************/

      if ( first->original_thread )
      {
        first->last_paraver = current_time;
      }
      if ( debug & D_LINKS )
      {
        PRINT_TIMER( current_time );
        printf( ": free memory writing permissions (OUT) to task %d wakes-up P%02d T%02d (t%02d)\n", task->taskid, IDENTIFIERS( first ) );
      }
      if ( first->action->action == SEND )
        really_send( first );
      else if ( first->action->action == MPI_OS )
        really_RMA( first );

      return;
    }
  }
}
