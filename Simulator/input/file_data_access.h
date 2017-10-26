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

#include <stdio.h>
#include "types.h"

// Definition and configuartion parameters of data access api
#define DATA_ACCESS_DIMEMAS_HELLO_SIGN "#DIMEMAS"

// Configures maximim number of applications supported by API
#define DATA_ACCESS_MAX_NUM_APP 100

// Configures maximum error message length
#define DATA_ACCESS_MAX_ERROR_LENGTH 256

// Thereshold value that deterimnates sequential or advanced metrhod for offsets search (2KBytes)
#define DATA_ACCESS_SEEK_THRESHOLD 2*1024

#define DEFINITION d
#define DEFINITION_REGEXP "d:%d:%[^\n]\n" // d:object_type:{specific_fields}

#define OFFSET        s
#define OFFSET_REGEXP "s:%d:%[^\n]\n"  // s:task_id:offset_thread_1:(...):offset_thread_n

#define OFFSET_NOT_PRESENT 0
#define OFFSET_PRESENT     1

#define DEF_COMMUNICATOR 1
#define COMMUNICATOR_REGEXP "%d:%d:%s" // comm_id:tasks_count:task_id_1:(...):task_id_n
#define DEF_FILE         2
#define DEF_ONESIDED_WIN 3

#define ACTION_REGEXP    "%d:%d:%d:%[^\n]\n" // op_id:task_id:thread_id:{specific_fields}

#define RECORD_NOOP      0
#define NOOP_REGEXP      "0:%d:%d\n"

#define RECORD_CPU_BURST 1
#define CPU_BURST_REGEXP "%lf" // burst_duration

#define RECORD_MSG_SEND  2
#define MSG_SEND_REGEXP_MPI  "%d:%lld:%d:%d:%c"     // dest_task_id:msg_size:tag:comm_id:synchronism
#define MSG_SEND_REGEXP_SS   "%d:%d:%lld:%d:%d:%c"  // dest_task_id:dest_thread_id:msg_size:tag:comm_id:synchronism

#define RECORD_MSG_RECV  3
#define MSG_RECV_REGEXP_MPI "%d:%lld:%d:%d:%d"     // src_task_id:msg_size:tag:comm_id:recv_type
#define MSG_RECV_REGEXP_SS  "%d:%d:%lld:%d:%d:%d"  // ssrc_task_id:src_thread_id:msg_size:tag:comm_id:recv_type

#define RECORD_GLOBAL_OP 10
#define GLOBAL_OP_REGEXP    "%d:%d:%d:%d:%ld:%ld:%d" // global_op_id:comm_id:root_task:root_th:bytes_send:bytes_recv

#define RECORD_EVENT     20
#define EVENT_REGEXP     "%llu:%llu" // event_type:event_value

#define RECORD_GPU_BURST 11
#define GPU_BURST_REGEXP "%lf"

#define RECVTYPE_RECV  0
#define RECVTYPE_IRECV 1
#define RECVTYPE_WAIT  2
#define RECVTYPE_WAITALL  3

#define DATA_ACCESS_ERROR -1
#define DATA_ACCESS_OK     1

/*******************************************************************************
 * Macro per decidir si cal utilitzar rendez vous en un send.
 * Hi ha 4 possibilitats:
 *
 * - TOTES les comunicacions son Asincrones.
 *   Quan no s'hautoritza l'us del camp que indica sincronisme als sends de la
 *   traça (RD_SYNC_use_trace_sync == FALSE) i la mida minima del missatge per
 *   utilitzar sincronisme es negativa (RD_SYNC_message_size < 0).
 *
 * - Nomes s'utilitza comunicacio Sincrona si s'indica de forma explicita. Quan
 *   RD_SYNC_use_trace_sync == TRUE i RD_SYNC_message_size < 0.
 *
 * - Nomes s'utilitza comunicacio Sincrona si la mida del missatge es >= que la
 *   mida donada. Quan RD_SYNC_use_trace_sync == TRUE i RD_SYNC_message_size>=0.
 *
 * - S'utilitza comunicacio Sincrona si s'indica de forma explicita o la mida
 *   del missatge es >= que la mida donada. Quan RD_SYNC_use_trace_sync == TRUE
 *   i RD_SYNC_message_size >= 0.
 ******************************************************************************/
#define USE_RENDEZ_VOUS(rende, mida) \
    (( (RD_SYNC_use_trace_sync && (rende)) || \
       ((RD_SYNC_message_size >= 0) && ((mida)>=RD_SYNC_message_size)) \
       ) ? 1 : 0)

typedef int count_t; // int or size_t?

struct ptask_structure
{
    int      ptask_id;
    count_t  tasks_count;
    count_t *threads_per_task;
};

char* DATA_ACCESS_get_error();

t_boolean DATA_ACCESS_end();

t_boolean DATA_ACCESS_get_number_of_tasks(
        char *trace_file_location,
        int  *tasks_count);

t_boolean DATA_ACCESS_init(
        int   ptask_id,
        char* trace_file_location);

t_boolean DATA_ACCESS_get_ptask_structure(
        int ptask_id,
        struct ptask_structure** ptask_info);

t_boolean DATA_ACCESS_get_communicators(
        int ptask_id, 
        struct t_queue** communicators);

t_boolean DATA_ACCESS_get_next_action(
        int ptask_id,
        int task_id,
        int thread_id,
        struct t_action** action,
        int *no_more_actions);

int DATA_ACCESS_ptask_id_end(
        int ptask_id);

int DATA_ACCESS_test_routine(
        int ptask_id);

t_boolean   DATA_ACCESS_init_index(
        int ptask_id,
        char *trace_file_location,
        int index);

unsigned int DAP_get_offset(
        int Ptaskid, 
        int taskid, 
        int threadid);

void DAP_restart_fps(
        int Ptaskid, 
        int taskid, 
        int threadid);

float DAP_get_progression(
        int Ptaskid, 
        int taskid, 
        int threadid);

t_boolean DATA_ACCES_get_acc_tasks(
        char *trace_file_location,
        int *acc_tasks_count,
        int **acc_tasks);

t_boolean DATA_ACCESS_reload_ptask (
        int ptask_id);
