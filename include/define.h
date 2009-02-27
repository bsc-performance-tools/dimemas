#ifndef __define_h
#define __define_h

/*
 * Constants
 * 
 * Sergi Girona (sergi@ac.upc.es)
 * 
 * (c) DAC-UPC 1993-94
 * 
 */

#if defined(ENABLE_LARGE_TRACES) && !defined(ARCH_MACOSX)
  #define MYOPEN    open64
  #define MYFOPEN   fopen64
  #define MYFREOPEN freopen64
  #define MYFSEEK   fseeko64
  #define MYFTELL   ftello64
  #define MYFSTAT   fstat64
  #define MYLSEEK   lseek64
#else
  #define MYOPEN    open
  #define MYFOPEN   fopen
  #define MYFREOPEN freopen
  #define MYFSEEK   fseeko
  #define MYFTELL   ftello
  #define MYFSTAT   fstat
  #define MYLSEEK   lseek
#endif /* ENABLE_LARGE_TRACES */

#if defined(ARCH_DEC) && defined(__alpha)
  #define alpha
#endif

#define VERSION    2
#define SUBVERSION 3

#define TRUE  1
#define FALSE 0

#define ERROR    1
#define NO_ERROR 0

#ifndef MIN
  #define MIN(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef MAX
  #define MAX(a,b) (((a)>(b))?(a):(b))
#endif

#define IDENTIFIERS(x) x->task->Ptask->Ptaskid,x->task->taskid,x->threadid
#define get_cpu_of_thread(x) x->cpu

/*
 * Maximum number of processors that must be created at initialitation time
 * of simulator
 */
#define MAX_MACHINES 1024 /* S'utilitza als schedulings */
#define MAX_NODES    256  /* Nomes es fa servir per inicialitzar, pero
                           * un cop es llegeix el .cfg ja s'agafen els
                           * valors correctes. Per tant, no hauria
                           * de tenir cap importancia. */
#define MAX_CPU      256  /* No es fa servir per res */

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

/*
 * Possible actions in trace files.
 */
#define DEAD        0 /* Thread wants to die */
#define WORK        1 /* Thread wants to work */
#define SEND        2 /* Thread wants to send a message */
#define RECV        3 /* Thread wants to receive a message */
#define EVEN        4 /* User or Compiler Event */
#define PRIO        5 /* Change thread priority */
#define FS          6 /* Disk operation */
#define SEM         7 /* Semaphore operation */
#define PORT_SEND   8
#define PORT_RECV   9
#define MEMORY_COPY 10
#define GLOBAL_OP   11
#define MPI_IO      12
#define MPI_OS      13
#define IRECV       14  /* Immediate recieve (without blocking) */
#define WAIT        15  /* Block until wait */

/*
 * JGG (29/12/2004): Communication types for notifications 
 */
 
#define ISEND 1001
#define BSEND 1002
/* #define IRECV 1003 --> Para no entrar en contradicción con el anterior */
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
#define MPI_IO_Metadata                        0
#define MPI_IO_Block_NonCollective             1
#define MPI_IO_Block_Collective                2
#define MPI_IO_NonBlock_NonCollective_Begin    3
#define MPI_IO_NonBlock_NonCollective_End      4
#define MPI_IO_NonBlock_Collective_Begin       5
#define MPI_IO_NonBlock_Collective_End         6

#define MPI_OS_GETPUT     0
#define MPI_OS_FENCE      1
#define MPI_OS_LOCK       2
#define MPI_OS_POST       3

#define MPI_OS_LOCK_LOCK    0
#define MPI_OS_LOCK_UNLOCK  1
#define MPI_OS_LOCK_SHARED  0
#define MPI_OS_LOCK_EXCLUS  1

#define MPI_OS_POST_POST     0
#define MPI_OS_POST_START    1
#define MPI_OS_POST_COMPLETE 2
#define MPI_OS_POST_WAIT     3


#define IO_Metadata                 100
#define IO_Block_NonCollective      101
#define IO_Block_Collective         102
#define IO_NonBlock_NonCollective   103
#define IO_NonBlock_Collective      104

/*
 * Us events reserved
 */
#define BLOCK_BEGIN 40
#define BLOCK_END   41

/* Two types of links */
#define OUT_LINK  1
#define IN_LINK   2

/* Three kinds of links */
#define NODE_LINK       1
#define MACHINE_LINK    2
#define CONNECTION_LINK 3

/* Five communication types */
#define LOCAL_COMMUNICATION_TYPE       0
#define INTERNAL_NETWORK_COM_TYPE      1
#define EXTERNAL_NETWORK_COM_TYPE      2
#define DEDICATED_CONNECTION_COM_TYPE  3

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

#define COM_TIMER_OUT             1
#define COM_TIMER_GROUP           2
#define RMA_TIMER_OUT             3
#define COM_EXT_NET_TRAFFIC_TIMER 4
#define COM_TIMER_GROUP_RESOURCES 5
#define COM_TIMER_OUT_RESOURCES   6

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

#define MIN_PORT_NUMBER  1001

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

#define ADD_MICRO_TIMER(x,y,z) \
        z = x+y

#define GT_TIMER(x,y) \
        (x > y)

#define LE_TIMER(x,y) \
        (x <= y)

#define EQ_TIMER(x,y) \
        (x == y)

#define ADD_TIMER(x,y,z) \
        z = x+y

#define SUB_TIMER(x,y,z) \
        z = x-y

#define MIN_TIMER(x,y,z) \
        z = MIN(x,y)

#define MAX_TIMER(x,y,z) \
        z = MAX(x,y)

#define TIMER_TO_FLOAT(x,y) \
        y = x

#define FLOAT_TO_TIMER(x,y) \
        y = x

#define TIMER_TO_DOUBLE(x,y) \
        y = (x*1e3)

#define NEQ_0_TIMER(x) \
        (x != 0)

#define EQ_0_TIMER(x) \
        (x == 0)

#define HOUR_MICRO_TO_TIMER(x,y,z) \
        z = x*3.6e9+y

#define ASS_TIMER(x,y) \
        x = (y)

#define ASS_ALL_TIMER(x,y) \
        x = (y)

#define PRINT_TIMER(x) \
        printf("%.6f",x/1e6)

#define FPRINT_TIMER(x,y) \
        fprintf (x,"%.6f",y/1e6)

#define OUT_OF_LIMIT(x) \
        (x > TIME_LIMIT)

/* JGG: para tratar marcas de tiempo negativo */
#define NIL_TIME(x) \
        x = -1;

#else   /* PACA */

#define GT_TIMER(x,y) \
        ((x.hours>y.hours) || ((x.hours==y.hours) && (x.micro>y.micro)))

#define LE_TIMER(x,y) \
        ((x.hours<y.hours) || ((x.hours==y.hours) && (x.micro<=y.micro)))

#define EQ_TIMER(x,y) \
        ((x.hours==y.hours)&& (x.micro==y.micro))

#define ADD_TIMER(x,y,x_plus_y) \
        if ((3.6e9-y.micro)>=x.micro) \
        { \
          x_plus_y.micro = x.micro + y.micro; \
          x_plus_y.hours = x.hours + y.hours; \
        } \
        else \
        { \
          x_plus_y.hours = x.hours + y.hours + (unsigned int)1; \
          x_plus_y.micro = y.micro - \
          ((unsigned int)(3.6e9 -x.micro)); \
        }

#define SUB_TIMER(x,y,x_minus_y) \
        if (x.micro>=y.micro) \
        { \
          x_minus_y.hours = x.hours - y.hours; \
          x_minus_y.micro = x.micro - y.micro; \
        } \
        else \
        { \
          x_minus_y.hours = x.hours - y.hours -1; \
          x_minus_y.micro = (unsigned int)(3.6e9 - y.micro+x.micro); \
        }

#define MIN_TIMER(x,y,min_x_y) \
        if GT_TIMER(x,y) \
          min_x_y = y; \
        else \
          min_x_y = x; \

#define MAX_TIMER(x,y,max_x_y) \
        if GT_TIMER(x,y) \
          max_x_y = x; \
        else \
          max_x_y = y;

#define TIMER_TO_FLOAT(timer_a, float_a) \
        float_a = (float)(3.6e9*timer_a.hours + timer_a.micro)

#define FLOAT_TO_TIMER(float_a, timer_a) \
        timer_a.hours = (unsigned int) (float_a/(float)3.6e9); \
        timer_a.micro = float_a - timer_a.hours*3.6e9;

#define TIMER_TO_DOUBLE(timer_a, double_a) \
        double_a = (double)(3.6e9*timer_a.hours*1e3 + timer_a.micro*1e3)

#define NEQ_0_TIMER(timer_a) \
        ((timer_a.hours !=0) || (timer_a.micro!=0))

#define EQ_0_TIMER(timer_a) \
        ((x.hours==0) && (x.micro==0))

#define HOUR_MICRO_TO_TIMER(hours_uint, micro_uint, timer_z) \
        timer_z.hours = (unsigned int) hours_uint; \
        timer_z.micro = (unsigned int) micro_uint;

#define ASS_TIMER(timer_x, value_y) \
        timer_x.hours = value_y; \
        timer_x.micro = value_y;

#define ASS_ALL_TIMER(x,y) \
        x.hours = y.hours; \
        x.micro = y.micro;

#define PRINT_TIMER(x) \
        printf("%dH %010u us",x.hours,x.micro)

#define FPRINT_TIMER(x,y) \
        fprintf (x,"%dH %010u us",y.hours,y.micro)
        
#define OUT_OF_LIMIT(x)
        (0)

/* JGG: para tratar marcas de tiempo negativo */
#define NIL_TIME(x) \
        x.hours = -1;
        x.micro = -1;

#endif /* PACA */


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


            
/* Mida dels buffers utilitzats (basicament per llegir fitxers) */
#define BUFSIZE 10000

#define GLOBAL_OPS_COUNT 14

enum
{
  MPI_Barrier = 0,
  MPI_Bcast,
  MPI_Gather,
  MPI_Gatherv,
  MPI_Scatter,
  MPI_Scatterv,
  MPI_Allgather,
  MPI_Allgatherv,
  MPI_Alltoall,
  MPI_Alltoallv,
  MPI_Reduce,
  MPI_Allreduce,
  MPI_Reduce_Scatter,
  MPI_Scan
};

static const char* Global_Ops_Labels[GLOBAL_OPS_COUNT] =
{
  "MPI_Barrier",
  "MPI_Bcast",
  "MPI_Gather",
  "MPI_Gatherv",
  "MPI_Scatter",
  "MPI_Scatterv",
  "MPI_Allgather",
  "MPI_Allgatherv",
  "MPI_Alltoall",
  "MPI_Alltoallv",
  "MPI_Reduce",
  "MPI_Allreduce",
  "MPI_Reduce_Scatter",
  "MPI_Scan"
};

#endif
