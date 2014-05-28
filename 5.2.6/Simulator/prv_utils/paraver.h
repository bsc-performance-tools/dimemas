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

  $URL::                                          $:  File
  $Rev::                                          $:  Revision of last commit
  $Author::                                       $:  Author of last commit
  $Date::                                         $:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef _paraver_h_
#define _paraver_h_

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"
#include "extern.h"

#include "pcf_defines.h"

/* Record identifier */
#define PRV_STATE     1
#define PRV_EVENT     2
#define PRV_COMM      3
#define PRV_STATE_RUN 4// Special state

/* #define PA_STATE_IDLE       0  */
/* #define PA_STATE_BUSY       1  */
/* #define PA_STATE_DEAD       2  */
/* #define PA_STATE_WAIT_MESS  3  */
/* #define PA_STATE_BLOCK_SEND 4  */
/* #define PA_STATE_WAIT_CPU   5  */
/* #define PA_STATE_CTXT       6  */
/* #define PA_STATE_WAIT_SEM   8  */
/* #define PA_STATE_WAIT_LINK  9  */
/* #define PA_STATE_IO         11 */
/* #define PA_BUSY_WAIT        12 */
/* #define PA_GLOBAL_OP_SYNC   13 */ /* NO SE USA EN TODO EL CÓDIGO */
/* #define PA_GLOBAL_OP_PERF   14 */ /* "                         " */
/* #define PA_STATE_BLOCK_IO   15 */
/* #define PA_STATE_BLOCK_OS   16 */ /* NO LISTADA EN EL PCF.         *
                                      * Sustituida por PRV_BLOCKED_ST */
/* #define PA_STATE_STARTUP    18 */

#define PA_STATE_STRING "%d:1:%d:%d:%d:%d:%.0f:%.0f:%d\n",paraver_line_number++
#define PA_EVENT_STRING "%d:2:%d:%d:%d:%d:%.0f:%d:%d\n",paraver_line_number++
#define PA_COMM_STRING  "%d:3:%d:%d:%d:%d:%.0f:%.0f:%d:%d:%d:%d:%.0f:%.0f:%d:%d\n",paraver_line_number++

#ifdef  FORMAT_VELL_RECORDS_4_PRV
  #define PA_GLOB_STRING  "%d:4:%d:%d:%d:%d:%.0f:%.0f:%.0f:%.0f:%d:%d:%d:%d\n",paraver_line_number++
#else
  #define PA_GLOB_STRING  "%d:4:%d:%d:%d:%d:%.0f:%d:%d:%d:%d:%d\n",paraver_line_number++
#endif /* FORMAT_VELL_RECORDS_4_PRV */

#define PARAVER_SEM_WAIT   98
#define PARAVER_SEM_SIGNAL 99

#define PARAVER_MINMAX_PRIORITY 96
#define PARAVER_PRIORITY        97

#define PARAVER_GROUP_BLOCK 91
#define PARAVER_GROUP_LAST  92
#define PARAVER_GROUP_FREE  93

#define PARAVER_CP_BLOCK 90

#define PARAVER_CP_ENTER 1
#define PARAVER_CP_EXIT  0

#define PARAVER_IO 89

#define PARAVER_IO_METADATA_BEGIN                          0
#define PARAVER_IO_METADATA_END                            1
#define PARAVER_IO_BLOCK_NONCOLLECTIVE_READ_BEGIN          2
#define PARAVER_IO_BLOCK_NONCOLLECTIVE_READ_END            3
#define PARAVER_IO_BLOCK_NONCOLLECTIVE_WRITE_BEGIN         4
#define PARAVER_IO_BLOCK_NONCOLLECTIVE_WRITE_END           5
#define PARAVER_IO_BLOCK_COLLECTIVE_READ_BEGIN             6
#define PARAVER_IO_BLOCK_COLLECTIVE_READ_END               7
#define PARAVER_IO_BLOCK_COLLECTIVE_WRITE_BEGIN            8
#define PARAVER_IO_BLOCK_COLLECTIVE_WRITE_END              9
#define PARAVER_IO_NONBLOCK_NONCOLLECTIVE_READ_BEGIN      10
#define PARAVER_IO_NONBLOCK_NONCOLLECTIVE_END             11
#define PARAVER_IO_NONBLOCK_NONCOLLECTIVE_WRITE_BEGIN     12
#define PARAVER_IO_NONBLOCK_COLLECTIVE_READ_BEGIN         14
#define PARAVER_IO_NONBLOCK_COLLECTIVE_WRITE_BEGIN        15
#define PARAVER_IO_NONBLOCK_COLLECTIVE_END                16

/* One sided operations*/
#define PARAVER_OS 88

#define PARAVER_OS_START               0
#define PARAVER_OS_LATENCY             1
#define PARAVER_OS_END                 2
#define PARAVER_OS_FENCE_START         3
#define PARAVER_OS_FENCE_END           4
#define PARAVER_OS_GET_LOCK            5
#define PARAVER_OS_WAIT_LOCK           6
#define PARAVER_OS_UNLOCK_BEGIN        7
#define PARAVER_OS_UNLOCK_END          8
#define PARAVER_OS_POST_POST           9
#define PARAVER_OS_POST_START          10
#define PARAVER_OS_POST_START_BLOCK    11
#define PARAVER_OS_POST_COMPLETE_BLOCK 12
#define PARAVER_OS_POST_COMPLETE       13
#define PARAVER_OS_POST_WAIT           14
#define PARAVER_OS_POST_WAIT_BLOCK     15

/* Record types */
#define PARADIS_WORK  1
#define PARADIS_EVENT 2
#define PARADIS_COMM  3





typedef char    p_ids;

/**
 * External routines defined in file paraver.c
 **/

extern void PARAVER_Enable_Trace_Generation (void);

extern void PARAVER_init(const char   *output_paraver_trace,
                         const char   *pcf_name,
                         dimemas_timer paraver_start_time,
                         dimemas_timer paraver_end_time,
                         t_boolean     paraver_priorities);

extern void PARAVER_end(void);

/* Hints to avoid 'over-collapsing' of events */
extern void PARAVER_Start_Op (int cpu, int ptask, int task, int thread,
                              dimemas_timer time);

extern void PARAVER_Idle (int cpu, int ptask, int task, int thread,
                          dimemas_timer init_time,
                          dimemas_timer end_time);

extern void PARAVER_Running (int cpu, int ptask, int task, int thread,
                             dimemas_timer init_time,
                             dimemas_timer end_time);

extern void PARAVER_Startup (int cpu, int ptask, int task, int thread,
                             dimemas_timer init_time,
                             dimemas_timer end_time);

extern void PARAVER_Wait (int cpu, int ptask, int task, int thread,
                          dimemas_timer init_time,
                          dimemas_timer end_time,
                          int type);

extern void PARAVER_Busy_Wait (int cpu, int ptask, int task, int thread,
                               dimemas_timer init_time,
                               dimemas_timer end_time);


extern void PARAVER_Waiting_Links (int cpu, int ptask, int task, int thread,
                                   dimemas_timer init_time,
                                  dimemas_timer end_time);

extern void PARAVER_Ctx_Switch (int cpu, int ptask, int task, int thread,
                                dimemas_timer init_time,
                                dimemas_timer end_time);

extern void PARAVER_Data_Copy (int cpu, int ptask, int task, int thread,
                               dimemas_timer init_time,
                               dimemas_timer end_time);

extern void PARAVER_RTT (int cpu, int ptask, int task, int thread,
                         dimemas_timer init_time,
                         dimemas_timer end_time);

extern void PARAVER_IO_Op (int cpu, int ptask, int task, int thread,
                           dimemas_timer init_time,
                          dimemas_timer end_time);

extern void PARAVER_IO_Blocked (int cpu, int ptask, int task, int thread,
                                dimemas_timer init_time,
                                dimemas_timer end_time);

extern void PARAVER_OS_Blocked (int cpu, int ptask, int task, int thread,
                                dimemas_timer init_time,
                                dimemas_timer end_time);

extern void PARAVER_Dead (int cpu, int ptask, int task, int thread,
                          dimemas_timer time);

extern void PARAVER_P2P_Comm (int cpu_s,  int ptask_s, int task_s, int thread_s,
                              dimemas_timer log_s, dimemas_timer phy_s,
                              int cpu_r, int ptask_r, int task_r, int thread_r,
                              dimemas_timer log_r, dimemas_timer phy_r,
                              int size, int tag);

extern void PARAVER_Event (int cpu, int ptask, int task, int thread,
                           dimemas_timer     time,
                           unsigned long long type,
                           unsigned long long value);

#ifdef __cplusplus
}
#endif

#endif
