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

#ifndef __communic_h
#define __communic_h

#include "types.h"

/*****************************************************************************
 * Global variables
 *****************************************************************************/

/*
 * P2P communications fine tuning
 */
extern t_boolean DATA_COPY_enabled;
extern int DATA_COPY_message_size;
extern t_boolean RTT_enabled;
extern t_nano RTT_time;

/*
 * External network status
 */
extern double suma_missatges_xarxa_externa;
extern double increment_missatges_xarxa_externa;

/*
 * External network parameters
 */
extern double param_external_net_alfa;    /* Ha de ser < 1 */
extern double param_external_net_periode; /* En microsegons */
extern double param_external_net_beta;    /* Coeficients que determinen */
extern double param_external_net_gamma;   /* la influencia dels traffics*/


/*****************************************************************************
 * Public functions
 *****************************************************************************/

void COMMUNIC_Init( const char *parameter_tracefile, float end_analysis_tpercent );
void COMMUNIC_End( void );

void COMMUNIC_general( int value, struct t_thread *thread );
void COMMUNIC_send( struct t_thread *thread );
void COMMUNIC_recv( struct t_thread *thread );
void COMMUNIC_Irecv( struct t_thread *thread );
void COMMUNIC_wait( struct t_thread *thread );
void COMMUNIC_block_after_busy_wait( struct t_thread *thread );

/* It is used also in 'memory' and 'ports' */
void transferencia( long long int size,
                    t_boolean remote,
                    struct t_thread *thread,
                    struct t_dedicated_connection *connection,
                    t_nano *temps_total,
                    t_nano *temps_recursos );

void really_send( struct t_thread *thread );

void new_global_op( int identificator, const char *name );

int get_global_op_id_by_name( char *name );

void GLOBAL_wait_operation( struct t_thread *thread );
void GLOBAL_operation( struct t_thread *thread,
                       int glop_id,
                       int comm_id,
                       int is_root,
                       int root_thid,
                       int bytes_send,
                       int bytes_recv,
                       int synch_type );

struct t_communicator *locate_communicator( struct t_queue *communicator_queue, int commid );

void global_op_reserva_links( struct t_thread *thread );

/* JGG: Constantes para marcar el FAN_IN y el FAN_OUT */
#define FAN_IN  0
#define FAN_OUT 1

void COMMUNIC_reset_deadlock();

void add_global_ops( void );

#endif
