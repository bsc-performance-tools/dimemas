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

/*
 * NOTE: example implementation of the 'external_comm_model' library used by
 * Dimemas to externally adapt the behaviour of selected p2p and collective
 * communications.
 */

#include "extern_comm_model.h"

/******************************************************************************
 * FUNCTION:  'external_get_communication_type'                               *
 *****************************************************************************/
int external_get_communication_type (int sender_nodeid,
                                     int receiver_nodeid,
                                     int sender_taskid,
                                     int receiver_taskid,
                                     int mess_tag,
                                     int mess_size)
{
  /* p2p communications are going to be modelled using the operations defined
   * in this library  */
  int result = EXTERNAL_P2P_COMM_MODEL;

  return result;
}

/******************************************************************************
 * FUNCTION:  'get_startup_value'                                             *  
 *****************************************************************************/
t_nano get_startup_value(int sender_nodeid,
                         int receiver_nodeid,
                         int sender_taskid,
                         int receiver_taskid,
                         int mess_tag, 
                         int mess_size)
{
  /* Selectively chose different startup values depending of the MPI task
   * sender or receiver of the operation */
  if (sender_taskid == 0 || receiver_taskid == 0)
  {
    return (t_nano) 10000;
  }
  else
  {
    return (t_nano) 8000;
  }


}

/******************************************************************************
 * FUNCTION:  'get_bandwidth_value'                                           *  
 *****************************************************************************/
t_nano get_bandwidth_value(int sender_nodeid,
                           int receiver_nodeid,
                           int sender_taskid,
                           int receiver_taskid,
                           int mess_tag, 
                           int mess_size)
{
  /* Selectively chose different bandwidth values depending of the MPI task 
   * sender of the operation */
  if (sender_taskid == 0)
  {
    return (t_nano) 1000;
  }
  else
  {
    return (t_nano) 500;
  }
}

/******************************************************************************
 * FUNCTION:  'external_get_global_op_type'                                   *  
 *****************************************************************************/
int external_get_global_op_type(int comm_id,
                                int global_op_id,
                                int bytes_send,
                                int bytes_recv)
{
  /* Collective communications are going to be modelled using the operations 
   * defined in this library  */
  return EXTERNAL_GLOBAL_OP;
}

/******************************************************************************
 * FUNCTION:  'external_compute_global_operation_time'                        *  
 *****************************************************************************/
void external_compute_global_operation_time(int     comm_id,
                                             int     global_op_id,
                                             int     bytes_send,
                                             int     bytes_recv,
                                             t_nano *latency_time,
                                             t_nano *op_time)
{
  /* Fixed values of the latency to 1000 nanoseconds */
  *latency_time = (t_nano) 1000.0;

  /* Transference time computed using a fixed value of 300 MB/s */
  *op_time      = (t_nano) (1e9 / (1 << 20) * (bytes_send + bytes_recv) / 300);
}
