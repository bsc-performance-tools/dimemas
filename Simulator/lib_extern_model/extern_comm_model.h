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
  $Rev:: 81               $:  Revision of last commit
  $Author:: jgonzale      $:  Author of last commit
  $Date:: 2012-12-17 17:3#$:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */


#ifndef __extern_comm_model_h
#define __extern_comm_model_h

#ifndef __define_h

/* Allowed point-to-point communication types */
#  define DIMEMAS_P2P_COMM_MODEL  1
#  define EXTERNAL_P2P_COMM_MODEL 4

/* Allowed global operation communication types */
#  define DIMEMAS_GLOBAL_OP  0
#  define EXTERNAL_GLOBAL_OP 1
#endif

/* Identifiers of the global operations simulated */
#define MPI_BARRIER_ID        0
#define MPI_BCAST_ID          1
#define MPI_GATHER_ID         2
#define MPI_GATHERV_ID        3
#define MPI_SCATTER_ID        4
#define MPI_SCATTERV_ID       5
#define MPI_ALLGATHER_ID      6
#define MPI_ALLGATHERV_ID     7
#define MPI_ALLTOALL_ID       8
#define MPI_ALLTOALLV_ID      9
#define MPI_REDUCE_ID         10
#define MPI_ALLREDUCE_ID      11
#define MPI_REDUCE_SCATTER_ID 12
#define MPI_SCAN_ID           13
#define MPI_ALLTOALLW_ID      14

#ifndef __types_h
/* Time resolution, microseconds */
typedef double t_nano;
#endif

/******************************************************************************
 * FUNCTION:  'external_get_communication_type'                               *
 *****************************************************************************/
/*
 * DESCRIPTION: Evaluates if a p2p communication that uses the internal cluster
 * network should be modeled externally
 *
 * inputs: identifiers of the nodes and tasks involved in the communication plus
 * the message tag and the size of the message to be transmitted
 *
 * return value: DIMEMAS_P2P_COMM_MODEL if the communication must be modelled
 * using the Dimemas model. EXTERNAL_P2P_COMM_MODEL if the communication will
 * be modeled using the current library.
 */
int external_get_communication_type( int sender_nodeid, int receiver_nodeid, int sender_taskid, int receiver_taskid, int mess_tag, int mess_size );

/******************************************************************************
 * FUNCTION:  'get_startup_value'                                             *
 *****************************************************************************/
/*
 * DESCRIPTION: Returns the startup value of a p2p communication in
 * *nanoseconds*
 *
 * inputs: identifiers of the nodes and tasks involved in the communication plus
 * the message tag and the size of the message to be transmitted
 *
 * return value: startup latency of the p2p communication in nanoseconds
 */
t_nano get_startup_value( int sender_nodeid, int receiver_nodeid, int sender_taskid, int receiver_taskid, int mess_tag, int mess_size );

/******************************************************************************
 * FUNCTION:  'get_bandwidth_value'                                           *
 *****************************************************************************/
/*
 * DESCRIPTION: Computes the effective bandwidth to be used in a p2p
 * communication
 *
 * inputs: identifiers of the nodes and tasks involved in the communication plus
 * the message tag and the size of the message to be transmitted
 *
 * return value: bandwidth in MB/s to be used in the p2p communication
 */
t_nano get_bandwidth_value( int sender_nodeid, int receiver_nodeid, int sender_taskid, int receiver_taskid, int mess_tag, int mess_size );

/******************************************************************************
 * FUNCTION:  'external_get_global_op_type'                                   *
 *****************************************************************************/
/*
 * DESCRIPTION: Evaluates if a collective communication should be modeled
 * externally
 *
 * inputs:
 *   - 'comm_id': identifier of the communicator used in the collective
 *                communication
 *
 *   - 'global_op_id': identifier of collective communication (see definitions
 *                     at the beginning of this file)
 *
 *   - 'bytes_send': total bytes to be scattered in the collective operation
 *
 *   - 'bytes_recv': total bytes to be gathered in the collective operation
 *
 * return value: DIMEMAS_GLOBAL_OP if the collective communication should be
 * simulated using the Dimemas model. EXTERNAL_GLOBAL_OP if the collective
 * communication will use an external modellization defined in this library
 */
int external_get_global_op_type( int comm_id, int global_op_id, int bytes_send, int bytes_recv );

/******************************************************************************
 * FUNCTION:  'external_compute_global_operation_time'                        *
 *****************************************************************************/
/*
 * DESCRIPTION: computes the startup latency and the transmission time of a
 * collective communication
 *
 * inputs:
 *   - 'comm_id': identifier of the communicator used in the collective
 *                communication
 *
 *   - 'global_op_id': identifier of collective communication (see definitions
 *                     at the beginning of this file)
 *
 *   - 'bytes_send': total bytes to be scattered in the collective operation
 *
 *   - 'bytes_recv': total bytes to be gathered in the collective operation
 *
 * outputs:
 *   - latency_time: startup latency of the collective communication in
 *                   *nanoseconds*
 *
 *   - op_time: transmission time of the collective communication in
 *              *nanoseconds*
 */
void external_compute_global_operation_time( int comm_id, int global_op_id, int bytes_send, int bytes_recv, t_nano *latency_time, t_nano *op_time );

#endif
