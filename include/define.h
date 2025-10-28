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

#ifndef __define_h
#define __define_h

#include <config.h>


/*
 * Constants
 *
 */

#if defined( ENABLE_LARGE_TRACES ) && !defined( ARCH_MACOSX )
#  define MYOPEN    open64
#  define MYFOPEN   fopen64
#  define MYFREOPEN freopen64
#  define MYFCLOSE  fclose
#  define MYFSEEKO  fseeko64
#  define MYFSEEK   fseek64
#  define MYFTELLO  ftello64
#  define MYFSTAT   fstat64
#  define MYLSEEK   lseek64
#else
#  define MYOPEN    open
#  define MYFOPEN   fopen
#  define MYFREOPEN freopen
#  define MYFCLOSE  fclose
#  define MYFSEEKO  fseeko
#  define MYFSEEK   fseek
#  define MYFTELLO  ftello
#  define MYFSTAT   fstat
#  define MYLSEEK   lseek
#endif /* ENABLE_LARGE_TRACES */

#if defined( ARCH_DEC ) && defined( __alpha )
#  define alpha
#endif

/*
#define VERSION    2
#define SUBVERSION 3
*/

#define TRUE  1
#define FALSE 0

#define ERROR    1
#define NO_ERROR 0

#ifndef MIN
#  define MIN( a, b ) ( ( ( a ) < ( b ) ) ? ( a ) : ( b ) )
#endif
#ifndef MAX
#  define MAX( a, b ) ( ( ( a ) > ( b ) ) ? ( a ) : ( b ) )
#endif

#define IDENTIFIERS( x )       x->task->Ptask->Ptaskid, x->task->taskid, x->threadid
#define get_cpu_of_thread( x ) x->cpu

/*
 * Maximum number of processors that must be created at initialitation time
 * of simulator
 */
#define MAX_MACHINES 1024 /* S'utilitza als schedulings */
#define MAX_NODES                                                \
  256               /* Nomes es fa servir per inicialitzar, pero \
                     * un cop es llegeix el .cfg ja s'agafen els \
                     * valors correctes. Per tant, no hauria     \
                     * de tenir cap importancia. */
#define MAX_CPU 256 /* No es fa servir per res */

/*
 * Maximun number of links per processor
 */
#define MAX_LINKS 4

/*
 * Maximun number of semaphores per task
 */
#define MAX_SEMAPHORES 100

/*
 * Base priority Unix like. Schedulers with round_robin and multiprogramation
 * doesn't use this value.
 */
#define BASE_PRIO 120

/* MAX RELOAD LIMIT */
#define MAX_RELOAD_LIMIT 1000

/* SHOW PERFORMANCE BANNERS: requires clock_gettime(), in library -lrt */
/*
#define SHOW_PERFORMANCE
*/

/* USE NEW QUEUE CONTAINERS */
//#define USE_EQUEUE

/*
 * Possible actions in trace files.
 */
#define NOOP          -1 /* Dummy action record! */
#define DEAD          0  /* Thread wants to die */
#define WORK          1  /* Thread wants to work */
#define SEND          2  /* Thread wants to send a message */
#define RECV          3  /* Thread wants to receive a message */
#define EVENT         4  /* User or Compiler Event */
#define PRIO          5  /* Change thread priority */
#define FS            6  /* Disk operation */
#define SEM           7  /* Semaphore operation */
#define PORT_SEND     8
#define PORT_RECV     9
#define MEMORY_COPY   10
#define GLOBAL_OP     11
#define MPI_IO        12
#define MPI_OS        13
#define IRECV         14 /* Immediate recieve (without blocking) */
#define WAIT          15 /* Block until wait */
#define WAIT_FOR_SEND 16 /* Added by Vladimir */
#define ACC_SYNC      17
#define GPU_BURST     18

/*
 * Types of global ops. With the introduction of the non-blocking global ops
 * we need a way to determine if the glop is blocking or not. The last field
 * of the record is saying it to us
 */

#define GLOBAL_OP_ASYNC     0
#define GLOBAL_OP_SYNC      1
#define GLOBAL_OP_WAIT      2
#define GLOBAL_OP_ROOT_SYNC 3


/*#define FS               32
#define SEM              64
#define PORT_SEND        128
#define PORT_RECV        256
#define MEMORY_COPY      512
#define GLOBAL_OP        1024
#define MPI_IO           2048
#define MPI_OS           4096
#define IRECV            8192
#define WAIT             16384
#define WAIT_FOR_SEND    32768*/

#define WAIT_NORMAL 0
#define WAIT_ALL    1


/*
 * JGG (29/12/2004): Communication types for notifications
 */

#define ISEND 1001
#define BSEND 1002
/* #define IRECV 1003 --> Para no entrar en contradiccion con el anterior */
#define BRECV 1004
/* #define WAIT 1005 --> Igual que con el IRECV */


/*
 * Different disk operations
 */
#define OPEN          0
#define READ          1
#define WRITE         2
#define SEEK          3
#define CLOSE         4
#define FS_USER_EVENT 5
#define DUP           6
#define TIMER         7
#define UNLINK        8

/*
 * Different semaphore operations
 */
#define SEM_WAIT     0
#define SEM_SIGNAL   1
#define SEM_SIGNAL_N 2

/*
 * Different MPI IO operations
 */
#define MPI_IO_Metadata                     0
#define MPI_IO_Block_NonCollective          1
#define MPI_IO_Block_Collective             2
#define MPI_IO_NonBlock_NonCollective_Begin 3
#define MPI_IO_NonBlock_NonCollective_End   4
#define MPI_IO_NonBlock_Collective_Begin    5
#define MPI_IO_NonBlock_Collective_End      6

#define MPI_OS_GETPUT 0
#define MPI_OS_FENCE  1
#define MPI_OS_LOCK   2
#define MPI_OS_POST   3

#define MPI_OS_LOCK_LOCK   0
#define MPI_OS_LOCK_UNLOCK 1
#define MPI_OS_LOCK_SHARED 0
#define MPI_OS_LOCK_EXCLUS 1

#define MPI_OS_POST_POST     0
#define MPI_OS_POST_START    1
#define MPI_OS_POST_COMPLETE 2
#define MPI_OS_POST_WAIT     3

#define IO_Metadata               100
#define IO_Block_NonCollective    101
#define IO_Block_Collective       102
#define IO_NonBlock_NonCollective 103
#define IO_NonBlock_Collective    104

/*
 * Us events reserved
 */
#define BLOCK_BEGIN 40
#define BLOCK_END   41


#define USER_EVENT_TYPE_TASKID_START_TASK   7001
#define USER_EVENT_TYPE_TASKID_END_TASK     7001
#define USER_EVENT_TYPE_TASKTYPE_START_TASK 7003
#define USER_EVENT_TYPE_TASKTYPE_END_TASK   7003
#define PRIORITY_SET_EVENT                  6999
#define PREEMPTION_SET_EVENT                6998

/* Two types of links */
#define OUT_LINK 1
#define IN_LINK  2

/* Five kinds of links */
#define MEM_LINK         1
#define NODE_LINK        2
#define MACHINE_LINK     3
#define CONNECTION_LINK  4
#define ACCELERATOR_LINK 5

/* Six communication types */
#define MEMORY_COMMUNICATION_TYPE       0
#define INTERNAL_NETWORK_COM_TYPE       1
#define EXTERNAL_NETWORK_COM_TYPE       2
#define DEDICATED_CONNECTION_COM_TYPE   3
#define EXTERNAL_MODEL_COM_TYPE         4 // Used by the external communications model
#define ACCELERATOR_COM_TYPE            5
#define NON_BLOCKING_GLOBAL_OP_COM_TYPE 6

/* Type of global Ops used by the external communications models */
#define DIMEMAS_GLOBAL_OP_MODEL  0
#define EXTERNAL_GLOBAL_OP_MODEL 1

/* Communication kinds */
#define NORD_SYNC  0 /* 0 IMMD / 0 RENDEZVOUS */
#define RD_SYNC    1 /* 0 IMMD / 1 RENDEZVOUS */
#define NORD_ASYNC 2 /* 1 IMMD / 0 RENDEZVOUS */
#define RD_ASYNC   3 /* 1 IMMD / 1 RENDEZVOUS */

/* Traffic functions */
#define EXP    1
#define LOG    2
#define LINEAL 3
#define CONST  4

/* Modules ID */
#define M_SCH     1
#define M_COM     2
#define M_FS      3
#define M_EV      4
#define M_PORT    5
#define M_MEM     6
#define M_CTXT_SW 7
#define M_RMA     8

/*
 * General interfaces values
 */
#define SCH_TIMER_OUT 1
#define SCH_NEW_JOB   2

#define COM_TIMER_OUT                     1
#define COM_TIMER_GROUP                   2
#define RMA_TIMER_OUT                     3
#define COM_EXT_NET_TRAFFIC_TIMER         4
#define COM_TIMER_GROUP_RESOURCES         5
#define COM_TIMER_OUT_RESOURCES_MEM       6
#define COM_TIMER_OUT_RESOURCES_NET       7
#define COM_TIMER_OUT_RESOURCES_WAN       8
#define COM_TIMER_OUT_RESOURCES_DED       9
#define COM_TIMER_OUT_RESOURCES_EXT_MODEL 10
#define COM_TIMER_OUT_RESOURCES_ACC       11


#define FS_OPERATION        1
#define FS_TIMER_OUT        2
#define FS_PORT_SEND_END    3
#define FS_PORT_RECV_END    4
#define FS_COPY_SEGMENT_END 5
#define FS_RETURN           6

#define PORT_TIMER_OUT           1
#define MEMORY_TIMER_OUT         1
#define CONTEXT_SWITCH_TIMER_OUT 1
#define COMMUNICATION_INFO       2

/* Scheduling default values */
#define NO_CONTEXT_SWITCH  0
#define NO_MINIMUM_QUANTUM 0

/* Communication default values */
#define LOCAL_BANDWITH  0
#define REMOTE_BANDWITH 0
#define CHANNEL_STARTUP 0
#define PORT_STARTUP    0
#define MEMORY_STARTUP  0

/* Output level max limit */
#define MAX_OUTPUT_LEVEL 6

#define MIN_PORT_NUMBER 1001

#define SEEK_NIL -1


#define DAEMON     TRUE
#define NOT_DAEMON FALSE

/* SCH_info constants */
#define INC_CPU_UTIL       1
#define DECAY_CPU_UTIL     2
#define RECOMPUTE_LOAD     3
#define SCH_INFO_RECV_HIT  4
#define SCH_INFO_SEND      5
#define MESS_RECV          6
#define SCH_INFO_RECV_MISS 7
#define SCH_GANG_XCHG_PRIO 8
#define SCH_PRIORITY       9

/* Global operations models and sizes */
#define GOP_MODEL_0   0
#define GOP_MODEL_CTE 1
#define GOP_MODEL_LIN 2
#define GOP_MODEL_LOG 3

#define GOP_SIZE_CURR 0 /* send or recv */
#define GOP_SIZE_MIN  1 /* Min(send, recv) */
#define GOP_SIZE_MAX  2 /* Max(send, recv) */
#define GOP_SIZE_MEAN 3 /* (send+recv)/2 */
#define GOP_SIZE_2MAX 4 /* 2*Max(send, recv) */
#define GOP_SIZE_SIR  5 /* send+recv */

/* Collective and blocking possible stages */
#define IN_BARRIER 0
#define IN_IO      1
#define IN_SCATTER 2

/* Debug tags */
#define D_NONE  0
#define D_FS    2
#define D_COMM  4
#define D_EV    8
#define D_SCH   16
#define D_LINKS 32
#define D_TASK  64
#define D_PORT  128
#define D_TH    256 /* JGG: Thread copy and destroy */
#define D_PRV   512 /* JGG: Paraver trace generation */

/* FS constants */
#define FILEMAX 16

/* Timer operations */

#ifndef PACA

#  define ADD_MICRO_TIMER( x, y, z ) z = x + y

#  define GT_TIMER( x, y ) ( x > y )

#  define LE_TIMER( x, y ) ( x <= y )

#  define EQ_TIMER( x, y ) ( x == y )

#  define ADD_TIMER( x, y, z ) z = x + y

#  define ADD_MIN_INCR_TO_TIMER( x, y ) y = x + 1

#  define SUB_TIMER( x, y, z ) z = x - y

#  define MIN_TIMER( x, y, z ) z = MIN( x, y )

#  define MAX_TIMER( x, y, z ) z = MAX( x, y )

#  define TIMER_TO_FLOAT( x, y ) y = x

#  define FLOAT_TO_TIMER( x, y ) y = x

#  define TIMER_TO_DOUBLE( x ) x

#  define NEQ_0_TIMER( x ) ( x != 0 )

#  define EQ_0_TIMER( x ) ( x == 0 )

#  define HOUR_MICRO_TO_TIMER( x, y, z ) z = x * 3.6e9 + y

#  define ASS_TIMER( x, y ) x = ( y )

#  define ASS_ALL_TIMER( x, y ) x = ( y )

#  define PRINT_TIMER( x ) printf( "%.9f", round( x ) / 1e9 )

#  define FPRINT_TIMER( x, y ) fprintf( x, "%.9f", round( y ) / 1e9 )

#  define OUT_OF_LIMIT( x ) ( x > TIME_LIMIT )

/* JGG: para tratar marcas de tiempo negativo */
#  define NIL_TIME( x ) x = -1;

#else /* PACA */

#  define GT_TIMER( x, y ) ( ( x.hours > y.hours ) || ( ( x.hours == y.hours ) && ( x.micro > y.micro ) ) )

#  define LE_TIMER( x, y ) ( ( x.hours < y.hours ) || ( ( x.hours == y.hours ) && ( x.micro <= y.micro ) ) )

#  define EQ_TIMER( x, y ) ( ( x.hours == y.hours ) && ( x.micro == y.micro ) )

#  define ADD_TIMER( x, y, x_plus_y )                                   \
    if ( ( 3.6e9 - y.micro ) >= x.micro )                               \
    {                                                                   \
      x_plus_y.micro = x.micro + y.micro;                               \
      x_plus_y.hours = x.hours + y.hours;                               \
    }                                                                   \
    else                                                                \
    {                                                                   \
      x_plus_y.hours = x.hours + y.hours + (unsigned int)1;             \
      x_plus_y.micro = y.micro - ( (unsigned int)( 3.6e9 - x.micro ) ); \
    }

#  define ADD_MIN_INCR_TO_TIMER( x, y ) \
    y.hours = x.hours;                  \
    y.micro = x.micro + 1;

#  define SUB_TIMER( x, y, x_minus_y )                               \
    if ( x.micro >= y.micro )                                        \
    {                                                                \
      x_minus_y.hours = x.hours - y.hours;                           \
      x_minus_y.micro = x.micro - y.micro;                           \
    }                                                                \
    else                                                             \
    {                                                                \
      x_minus_y.hours = x.hours - y.hours - 1;                       \
      x_minus_y.micro = (unsigned int)( 3.6e9 - y.micro + x.micro ); \
    }

#  define MIN_TIMER( x, y, min_x_y ) \
    if GT_TIMER ( x, y )             \
      min_x_y = y;                   \
    else                             \
      min_x_y = x;

#  define MAX_TIMER( x, y, max_x_y ) \
    if GT_TIMER ( x, y )             \
      max_x_y = x;                   \
    else                             \
      max_x_y = y;

#  define TIMER_TO_FLOAT( timer_a, float_a ) float_a = (float)( 3.6e9 * timer_a.hours + timer_a.micro )

#  define FLOAT_TO_TIMER( float_a, timer_a )                  \
    timer_a.hours = (unsigned int)( float_a / (float)3.6e9 ); \
    timer_a.micro = float_a - timer_a.hours * 3.6e9;

#  define TIMER_TO_DOUBLE( timer_a ) (double)( 3.6e12 * timer_a.hours + timer_a.micro )

#  define NEQ_0_TIMER( timer_a ) ( ( timer_a.hours != 0 ) || ( timer_a.micro != 0 ) )

#  define EQ_0_TIMER( timer_a ) ( ( x.hours == 0 ) && ( x.micro == 0 ) )

#  define HOUR_MICRO_TO_TIMER( hours_uint, micro_uint, timer_z ) \
    timer_z.hours = (unsigned int)hours_uint;                    \
    timer_z.micro = (unsigned int)micro_uint;

#  define ASS_TIMER( timer_x, value_y ) \
    timer_x.hours = value_y;            \
    timer_x.micro = value_y;

#  define ASS_ALL_TIMER( x, y ) \
    x.hours = y.hours;          \
    x.micro = y.micro;

#  define PRINT_TIMER( x ) printf( "%dH %010u us", x.hours, x.micro )

#  define FPRINT_TIMER( x, y ) fprintf( x, "%dH %010u us", y.hours, y.micro )

#  define OUT_OF_LIMIT( x )
( 0 )

/* JGG: para tratar marcas de tiempo negativo */
#  define NIL_TIME( x ) x.hours = -1;
  x.micro = -1;

#endif /* PACA */


/* Mida dels buffers utilitzats (basicament per llegir fitxers) */
#define BUFSIZE 100000

#define GLOBAL_OPS_COUNT 16

enum
{
  MPI_Barrier = 0,
  MPI_Bcast,
  MPI_Gather,
  MPI_Gatherv,
  MPI_Scatter,
  MPI_Scatterv, /* 5 */
  MPI_Allgather,
  MPI_Allgatherv,
  MPI_Alltoall,
  MPI_Alltoallv,
  MPI_Reduce, /* 10 */
  MPI_Allreduce,
  MPI_Reduce_Scatter,
  MPI_Reduce_Scatter_block,
  MPI_Scan,
  MPI_Alltoallw
};

static const char* Global_Ops_Labels[ GLOBAL_OPS_COUNT ] = {
  "MPI_Barrier",    "MPI_Bcast",    "MPI_Gather",    "MPI_Gatherv", "MPI_Scatter",   "MPI_Scatterv",       "MPI_Allgather",
  "MPI_Allgatherv", "MPI_Alltoall", "MPI_Alltoallv", "MPI_Reduce",  "MPI_Allreduce", "MPI_Reduce_Scatter", "MPI_Reduce_Scatter_block",
  "MPI_Scan",       "MPI_Alltoallw"
};

#define ACCELERATOR_NULL   0 // No accelerator tracing
#define ACCELERATOR_HOST   1 // Accelerator host thread
#define ACCELERATOR_KERNEL 2 // Accelerator kernel thread
#define OpenMP_NULL        0 // No open tracing
#define MASTER             1 // OpenMP master thread
#define WORKER             2 // OpenMp worker thread

#endif
