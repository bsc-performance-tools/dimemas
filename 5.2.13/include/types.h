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

#ifndef __types_h
#define __types_h

#include <stdio.h>
#include <limits.h>
#include <sys/types.h>
#include "define.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>

#ifdef ENABLE_LARGE_TRACES
  #ifdef OS_CYGWIN
    #define t_off_fitxer _off64_t
  #else
    #ifdef OS_MACOSX
      #define t_off_fitxer off_t
    #else
      #define t_off_fitxer off64_t
    #endif
  #endif
#else
  #define t_off_fitxer off_t
#endif /* ENABLE_LARGE_TRACES */

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_RESET   "\x1b[0m"


typedef int     t_boolean;
typedef double  t_priority; /* priority for queue elements */
typedef int     t_count;    /* number of elements in queue */

typedef double  t_nano;

typedef int modules_map;    /* see 'modules_map.h' */

#ifdef PACA

typedef struct
{
  unsigned int  hours;
  unsigned int  micro;
} dimemas_timer;

#else   /* !PACA */
typedef double  dimemas_timer;
//stypedef unsigned long long dimemas_timer;
#endif  /* PACA */

typedef int Equeue;

/*
 * Item for queues and lists Double chained item previous item of first in
 * queue is NIL next item of last in queue is NIL
 */
struct t_item
{
  struct t_item  *next;      /* Next item in queue */
  struct t_item  *prev;      /* Previous item in queue */
  union
  {
    t_priority    priority;  /* priority on sorted priority queues */
    dimemas_timer list_time; /* time for next event in event queue */
  } order;
  void           *content;   /* Real content of item */
};

struct t_queue
{
  struct t_item  *first; /* First item in queue */
  struct t_item  *last;  /* Last item in queue */
  struct t_item  *curr;  /* Current item in sequential search */
  t_count         count; /* Number of items */
};

struct t_list
{
  struct t_list  *next;
};

struct t_module
{
  unsigned long int type;
  unsigned long int value;
  double            ratio;
  double            const_burst_duration; /* JGG (2012/10/19): to substitute bursts durations */
  int               src_file;
  int               src_line;
  int               used;     /* Indica si el bloc ha estat utilitzat (a part de definit) */
  char             *module_name;
  char             *activity_name;
};

struct t_filed
{
  int   file_id;
  char *location;
};


#define CP_WORK 0
#define CP_BLOCK 1
#define CP_OVERHEAD 2

struct t_cp_node
{
  struct t_cp_node *prev;
  struct t_cp_node *next;
  struct t_cp_node *recv;
  struct t_cp_node *send;
  int               status; /* Work, block, communication overhead */
  long long         module_type;
  long long         module_value;
  dimemas_timer     final_time;
  struct t_thread  *thread;
};

struct t_module_cp
{
  long long module_type;
  long long module_value;

  dimemas_timer timer;
  dimemas_timer timer_comm;
};
/* This structers was previously found on 'random.h' */
/* Distribution parameters for gaussian and uniform */
struct t_normal
{
  double mean;
  double stdev;
};

struct t_uniform
{
  double left;
  double right;
};

/* Distibution constants */
#define NO_DISTRIBUTION      0
#define NORMAL_DISTRIBUTION  1
#define UNIFORM_DISTRIBUTION 2

struct t_rand_type
{
  int distribution;

  union
  {
    struct t_normal  normal;
    struct t_uniform uniform;
  } parameters;
};

/* JGG (31/03/2006): Structure to store burst categories, used on synthetic
 * burst generation */
struct _burst_category
{
  int                id;
  struct t_rand_type values;
};
typedef struct _burst_category* burst_category_t;

/* JGG (24/12/2004): Estructuras para marcar las notificaciones de sends y
 * receives producidos, y pendientes de ser tratados */
struct send_notification
{
  dimemas_timer    t_stamp;     /* TimeStamp */
  int              id;
  int              dest_taskid;
  int              msg_tag;
  int              msg_size;
  int              communic_id;
  int              send_type;
  struct t_thread *sender;
};
typedef struct send_notification* t_send_notification;
#define SNDN_NIL (struct send_notification*)0
/* JGG: Buscar en 'define.h'
#define BSEND 0
#define ISEND 1
*/

struct recv_notification
{
  dimemas_timer    t_stamp;     /* TimeStamp */
  int              id;
  int              src_taskid;
  int              msg_tag;
  int              communic_id;
  int              recv_type;
  struct t_thread *receiver;
  dimemas_timer    logical_recv;
};
typedef struct recv_notification* t_recv_notification;
#define RCVN_NIL (struct recv_notification*)0
/* JGG: Buscar en 'define.h'
#define BRECV 0
#define IRECV 1
#define BWAIT 2
*/
struct mesg_notification
{
  dimemas_timer    t_stamp;     /* TimeStamp */
  int              id;
  int              src_taskid;
  int              msg_tag;
  int              size;
  int              communic_id;
  t_boolean        used;
  t_boolean        wait_arrived;
  struct t_thread *sender;
  dimemas_timer    logical_send;
  dimemas_timer    physical_send;
  dimemas_timer    physical_recv;
};
typedef struct mesg_notification* t_mesg_notification;
#define MSGN_NIL (struct mesg_notification*)0

#define ANY -1

/*****************************************************************************
 * JGG (15/02/2005): Estructura para gestionar el estado de los threads que se
 * ejecutan en la aplicacion
 *
 * Los estados posibles se corresponden a los estados Paraver definidos en el
 * fichero 'pcf_defines.h'
 */
struct thread_state_
{
  int           state;
  dimemas_timer init_time;
};
typedef struct thread_state_ thread_state_t;
#define STATE_NIL (struct thread_state*)0
/****************************************************************************/

struct t_dedicated_connection
{
  int    id;                /* Connection number */
  int    source_id;         /* Source machine number */
  int    destination_id;    /* Destination machine number */
  double bandwidth;         /* Bandwidth of the connection */
  int   *tags;              /* List of tags that will use the connection */
  int    number_of_tags;
  int    first_message_size;    /* Size of messages in bytes */
  int    first_size_condition;  /* Size condition that should meet messages to
                                 * use the connection. 0 <, 1 =, 2 > */
  int    operation; /*Operation between the conditions: 0 AND, 1 OR */
  int    second_message_size; /* Size of messages in bytes */
  int    second_size_condition; /* Size condition that should meet messages to
                                 * use the connection. 0 <, 1 =, 2 > */
  int   *communicators; /* List of communicators of collective op. that can use
                         * the connection */
  int    number_of_communicators;
  double startup;  /* Connection startup */
  double flight_time; /* Connection flight time */

  /* Les cues input_links, output_links, in_links i out_links son cues
   * per si mes endavant es vol tenir connexions amb "mes d'un bus" en
   * cada sentit. Pero actulament podrien ser punters perque nomes hi
   * ha un link d'entrada i un de sortida.
   */
  t_boolean      infinite_links;       /* TRUE if there are infinite links */
  t_boolean      half_duplex;    /* Half duplex connection? */
  struct t_queue free_in_links;  /* Free input links (destination -> source) */
  struct t_queue free_out_links; /* Free output links (source -> destination) */
  struct t_queue busy_in_links;  /* Busy input link */
  struct t_queue busy_out_links; /* Busy output link */
  struct t_queue th_for_in;      /* Threads awaiting for input link */
  struct t_queue th_for_out;     /* Threads awaiting for output link */
};

/*
 * Types for actions described in trace files
 */
struct t_compute
{
  t_nano cpu_time; /* Cpu time wanted */
};

struct t_send
{
  long long int       mess_size;    /* Size of message */
  int                 dest;         /* Taskid of partner (receiver) */
  int                 dest_thread;  /* Thread_id of partner (receiver) */
  int                 mess_tag;     /* Message tag */
  int                 communic_id;  /* Communicator id */

  t_boolean           rendez_vous;  /* Rendez vous message or not */
  t_boolean           immediate;    /* Immediate send or not */

  int                            comm_type;    /* Communication type (JGG) */
  struct t_dedicated_connection *connection;

  struct t_thread    *receiver;
  dimemas_timer       logical_send;
  dimemas_timer       physical_send;
  dimemas_timer       physical_recv;
};

struct t_recv
{
  int           ori;         /* Taskid of partner(sender) */
  int           ori_thread;  /* Thread_id of partner(sender) */
  int           mess_tag;    /* Message tag */
  long long int mess_size;   /* Size of message */
  int           communic_id; /* Communicator id */
  int           comm_type;   /* Communication type */
  int		wait_type;

  struct t_thread *sender;
  dimemas_timer    logical_recv;
};

struct t_even
{
  unsigned long long type;  /* Event type user or compiler */
  unsigned long long value; /* Value for this event */
};

struct t_prio
{
  int value; /* New priority value for this thread */
};

struct t_open
{
  char filename[FILEMAX + 1];
  int  flags;                 /* Open 2nd parameter */
  int  initial_size;          /* Number of bytes of file when opened */
  int  fd;                    /* fd associated to opened file or error */
};

struct t_read
{
  int           fd;
  int           requested_size;
  int           delivered_size; /* number of bytes read or error */
  unsigned long pos;            /* Used by the File System module */
};

struct t_write
{
  int           fd;
  int           requested_size;
  int           delivered_size; /* number of bytes write or error */
  unsigned long pos;            /* Used by the File System module */
};

struct t_seek
{
  int fd;
  int offset; /* Position requested */
  int whence; /* Offset relative position */
  int result; /* Final position or error */
};

struct t_close
{
  int fd;
};

struct t_dup
{
  int old_fd;
  int new_fd;
};

struct t_unlink
{
  char filename[FILEMAX + 1]; /* Space must be allocated on reader */
};

struct t_user_event
{
  int id;
};

struct t_fs_op
{
  int                   which_fsop;

  union
  {
    struct t_open       fs_open;
    struct t_read       fs_read;
    struct t_write      fs_write;
    struct t_seek       fs_seek;
    struct t_close      fs_close;
    struct t_dup        fs_dup;
    struct t_user_event fs_user_event;
    struct t_unlink     fs_unlink;
  } fs_o;
};

struct t_sem_op
{
  int op;     /* SEM_WAIT/ SEM_SIGNAL / SEM_SIGNAL_N */
  int sem_id;
  int n;      /* Only when SEM_SIGNAL_N */
};

struct t_portac
{
  int portid;
  int size;
  int module;
};

struct t_mem
{
  int             module;
  int             size;
  struct t_node  *source;
  struct t_node  *dest;
};

struct t_global_op
{
  int           glop_id;     /* Global operation identificator */
  int           comm_id;     /* Communicator identificator */
  int           root_rank;   /* Identificator of root task */
  int           root_thid;   /* Identificator of thread root task */
  long long int bytes_send;  /* Number of bytes send */
  long long int bytes_recvd; /* Number of bytes received */
};

struct t_mpi_io
{
  int which_io;
  int commid;      /* Only valid for metadata operation */
  int fh;          /* File handle */
  int Oop;         /* Operation */
  int size;        /* Requested size */
  int request;     /* Match point for non-blocking operations */
};

struct t_mpi_os
{
  int which_os;
  int Oop;
  int target_rank;
  int window_id;
  int size;
  int mode;
  int post_size;
  struct t_queue post_ranks;
};

struct t_action
{
  struct t_action *next;   /* Next action for thread */
  int              action; /* DEAD/WORK/SEND/RECV/IRECV/WAIT/
                              PRIO/FS/SEM/GLOBAL_OP/MPI_IO/MPI_OS*/
  union
  {
    struct t_compute   compute;
    struct t_send      send;
    struct t_recv      recv;
    struct t_even      even;
    struct t_prio      prio;
    struct t_fs_op     fs_op;
    struct t_sem_op    sem_op;
    struct t_portac    port;
    struct t_mem       memory;
    struct t_global_op global_op;
    struct t_mpi_io    mpi_io;
    struct t_mpi_os    mpi_os;
  } desc;
};

struct t_link
{
  union
  {
    struct t_task                 *task;       /* Task of the link */
    struct t_node                 *node;       /* Node of the link */
    struct t_machine              *machine;    /* Machine link */
    struct t_dedicated_connection *connection; /* Dedicated connection */

  } info;

  int              kind;        /* NODE_LINK/MACHINE_LINK/CONNECTION_LINK */
  int              type;        /* IN_LINK/OUT_LINK */
  int              linkid;      /* Identifier */
  struct t_thread *thread;      /* Thread belonging the link */
  dimemas_timer    assigned_on; /* Time when the thread was assigned */
};

// Kar EEE -------------------------- 3 LEVEL NETWORK --------------- 3LEEE----------------


struct eee_link
{
    int level;
    int switch_id;
    int link_id;

    t_nano inlink_next_free_time;  // Duplex links: incoming messages
    t_nano outlink_next_free_time; // Duplex links: outgoing messages

    t_nano last_inlink_off_time;
    t_nano last_outlink_off_time;

    t_nano stats_total_inlink_on_time;     // STATS
    t_nano stats_total_outlink_on_time;    // STATS

    struct eee_link *partner_link;
};

struct switches
{

    int switch_level;
    int switch_id;

// in_link ==> lower level to current level -------- UP LINK
// out_link ==> current level to higher level ------ OUT LINK

    int N_in_links; // in_link ==> lower level to current level -------- UP LINK
    int N_out_links; // out_link ==> current level to higher level ------ OUT LINK

    // These nics connect to other routers or nodes
    struct eee_link *in_links;
    struct eee_link *out_links;

    // Routing logic inside the switch -- fully connected bus (assumed)
    struct eee_link *bus;
};

// ---------------------------------------- END OF Karthikeyan 3LEEE Code --------------------

struct t_global_op_definition
{
  int identificator;
  char *name;
};

struct t_global_op_information
{
  int identificator;
  int FIN_model;                /* 0, CONSTANT, LINEAL, LOGARITHMIC*/
  int FIN_size;                 /* MIN, MAX, average, 2*MAX, send+recv */
  int FOUT_model;               /* 0, CONSTANT, LINEAL, LOGARITHMIC*/
  int FOUT_size;                /* MIN, MAX, average, 2*MAX, send+recv */
};

struct t_communicator
{
  int            communicator_id;
  int            size;
  int*           global_ranks;
  // struct t_queue global_ranks;         /* Queue of integers, rank of tasks */

  struct t_queue threads;              /* Threads block until syncronization */
  struct t_queue machines_threads;     /* One thread from each machine used */
  struct t_queue m_threads_with_links; /* Els anteriors que ja tenen links
                                        * reservats */

  // struct t_queue machines_nodes;
  struct t_queue* nodes_per_machine;    /* Nodes involved on each machine */
  struct t_queue  tasks_per_node;
  // struct t_queue nodes_used;           /* List of nodes used */
  // struct t_queue tasks_per_node;       /* Number of tasks on each node */

  struct t_thread* current_root;       /* Root thread of the 'in-flight'
                                          operation */
  t_boolean in_flight_op;              /* True when simulating an operation */
};



struct t_window
{
  int window_id;
  struct t_queue global_ranks;            /* Queue of integers, rank of tasks */
  int mode;                               /* Indicates the current mode of this
                                           * window */

  struct t_queue fence_tasks;             /* Queue with blocked threads  due
                                           * fence operation */
  struct t_queue fence_operations;        /* Queue with threads performing
                                           * operations and the mode is fence */

  struct t_task *task_with_lock;           /* Task with lock */
  int    lock_mode;                        /* Indicates if it is shared/
                                            * exclusive */
  struct t_queue threads_with_lock;        /* Threads holding lock */
  struct t_queue threads_waiting_lock;
  struct t_queue threads_waiting_unlock;
  struct t_queue lock_operations;          /* Threads performing a communiction
                                            * and the mode is lock */

  struct t_queue post_done;
  struct t_queue start_done;
  struct t_queue pending_of_post;
  struct t_queue pending_rma_to_complete;  /* block complete until all RMA
                                            * finish */
  struct t_queue complete_done;
  struct t_queue pending_wait_to_complete; /* block wait until complete is done */
  struct t_queue post_operations;          /* Threads performing a communiction
                                            * and the mode is post */
};

struct t_request_thread
{
  int request;
  struct t_thread *thread;
};

struct t_fh_commid
{
  int fh;
  struct t_communicator *communicator;
  struct t_queue threads;
  struct t_queue copy;
  int stage;                    /* For different IO stages */
  int counter;
};


struct t_Ptask
{
  int             Ptaskid;
  char           *tracefile;
  char           *configfile;

  int             map_definition;
  int             tasks_per_node;
/*
  HERE I WANT TO AVOID USING A FILE POINTER FOR EACH THREAD BUT TO USE MMAP
  AND THAN READ THE TRF FILE AS IT WAS A STRING
*/
//   FILE           *file;
  char           *mmapped_file;
  unsigned long   mmap_position;
  struct stat     sb;
  int             is_there_seek_info;


  int             n_rerun;
  t_boolean       synthetic_application;
  // struct t_queue  tasks;
  size_t         tasks_count;
  struct t_task *tasks;
  struct t_queue global_operation; /* Threads pending for sincronization
                                     * in global operation */
  struct t_queue      Communicator;
  struct t_queue      Window;
  struct t_queue      MPI_IO_fh_to_commid;
  struct t_queue      MPI_IO_request_thread;
  modules_map         Modules;
  struct t_queue      Filesd;
  struct t_queue      UserEventsInfo; /* Cua amb les informacions dels possibles
                                       * events d'usuari */
};

struct t_task
{
  struct t_Ptask *Ptask;
  int             taskid;
  int             nodeid;
  // struct t_queue  threads;

/*Vladimir: for optimization, when a task has many threads*/
  int               threads_count;
  struct t_thread **threads;


  struct t_queue  mess_recv;     /* Queue for received messages */
  struct t_queue  recv;          /* Queue of threads waiting for message */
  struct t_queue  send;          /* Queue of threads waiting for a partner
                                  * when sending a message */
  struct t_thread *current_wait; /* Current WAIT operation, only for IRECV *
                                     * ES SOLO TEMPORAL, PORQUE SI HAY OpenMP *
                                     * PUEDE HABER + DE 1 WAIT */

  /****************************************************************************/
  /* Les dues cues seguents s'han hagut d'afegir per poder tractar
   * correctament els Irecv/Wait */
   struct t_queue  recv_without_send; /* Cua de threads (d'aquesta task) que
                                       * han arribat a un recv o Irecv
                                       * sense que s'hagi arribat al send
                                       * corresponent. La diferencia
                                       * principal amb la cua recv es que el
                                       * thread original no te perque estar
                                       * bloquejat (si era irecv). Una altra
                                       * diferencia es que els threads d'aquesta
                                       * cua es treuen quan s'arriba al send,
                                       * no quan s'acaba la transmisio fisica.*/
   struct t_queue  send_without_recv; /* Aquesta cua es la inversa de
                                       * l'anterior. Hi ha els threads de
                                       * qualsevol task que han arribat a un
                                       * send sense cap a aquesta task que
                                       * s'hagi arribat al recv o Irecv
                                       * corresponent.
                                       * Els threads d'aquesta cua es treuen de
                                       * seguida que s'arriba al recv o Irecv
                                       * corresponent, no quan es fa realment
                                       * la transmisio. */
  /****************************************************************************/

  /* JGG (2014/03/19): this queue stores the t_recv structures from received
   * messages to mark where the logical_recv time of the possible Irecv
   * executed */
  struct t_queue irecvs_executed;

  struct t_queue  semaphores;

  t_boolean         infinite_links;       /* TRUE if there are infinite links */
  t_boolean         half_duplex_links;    /* TRUE if links are half duplex */


  struct t_queue    free_in_links;        /* Free input links */
  struct t_queue    free_out_links;       /* Free output link */
  struct t_queue    busy_in_links;        /* Busy input links */
  struct t_queue    busy_out_links;       /* Busy output links */
  struct t_queue    th_for_in;            /* Awaiting for input link */
  struct t_queue    th_for_out;           /* Awaiting for output link */

  t_boolean       io_thread;
};

struct t_event
{
  dimemas_timer    event_time;
  int              module;
  struct t_thread *thread;
  int              info;
  int              daemon;
};

struct t_activity
{
  char *name;
  int   number;
  char *symname;
};

struct t_thread
{
  int              threadid; /* Thread id within task */
  struct t_task   *task;
  struct t_queue   account;

  /* Vladimir: to remember sstask_id and sstask_type of the thread
     so these events could be marked again when preempting
  */
  unsigned long    sstask_id;
  unsigned long    sstask_type;

    // Karthikeyan: EEE CODE

  t_boolean eee_send_done;      // TRUE if comm_send_via_3L_network is over
  int current_level;            // level in 3L heirarchy -1 -> outside network; 0<1<2 => 3 Levels
  int routing_dir;              // 1 UP; -1 DOWN;
  t_boolean link_transmit_done;
  t_boolean nw_switch_done;
  t_boolean eee_done_reset_var;
  int eee_linkid;
  int eee_switchid;
  int messages_in_flight;

  // Karthikeyan: END OF EEE CODE

    /* making these queues separate for every thread
       only DEPENDENCIES go to this queues
       REAL MPI TRANSFERS go the the queues of the task    */
  struct t_queue  mess_recv; /* Queue for received messages */
  struct t_queue  recv;      /* Queue of threads waiting for message */
  struct t_queue  send;      /* Queue of threads waiting for a partner
                              * when sending a message */
  /****************************************************************************/
  /* Les dues cues seguents s'han hagut d'afegir per poder tractar
   * correctament els Irecv/Wait */
   struct t_queue  recv_without_send; /* Cua de threads (d'aquesta task) que
                                       * han arribat a un recv o Irecv
                                       * sense que s'hagi arribat al send
                                       * corresponent. La diferencia
                                       * principal amb la cua recv es que el
                                       * thread original no te perque estar
                                       * bloquejat (si era irecv). Una altra
                                       * diferencia es que els threads d'aquesta
                                       * cua es treuen quan s'arriba al send,
                                       * no quan s'acaba la transmisio fisica.*/
   struct t_queue  send_without_recv; /* Aquesta cua es la inversa de
                                       * l'anterior. Hi ha els threads de
                                       * qualsevol task que han arribat a un
                                       * send sense cap a aquesta task que
                                       * s'hagi arribat al recv o Irecv
                                       * corresponent.
                                       * Els threads d'aquesta cua es treuen de
                                       * seguida que s'arriba al recv o Irecv
                                       * corresponent, no quan es fa realment
                                       * la transmisio. */
  /****************************************************************************/
  /* JGG (2014/03/19): this queue stores the t_recv structures from received
   * messages to mark where the logical_recv time of the possible Irecv
   * executed */
  struct t_queue irecvs_executed;

  struct t_action *action;
  struct t_action *last_action;
  t_boolean        original_thread;
  struct t_thread *twin_thread;
  struct t_cpu    *cpu;
  struct t_link   *local_link,
                  *partner_link;
  struct t_link   *local_hd_link,
                  *partner_hd_link;   /* Pointers to non used links (HF-DPEX)*/
  struct t_node   *partner_node;   /* Cal guardar el node desti del missatge */

  /*
  struct t_link   *in_mem_link;
  struct t_link   *out_mem_link;
  */

  dimemas_timer    last_paraver;
  t_boolean        loose_cpu;
  int              to_module;
  char            *sch_parameters;
  t_off_fitxer     original_seek;
  t_off_fitxer     seek_position;
  int              base_priority;
  t_boolean        doing_context_switch;
  t_boolean        to_be_preempted;
  t_boolean        doing_busy_wait;
  t_boolean        doing_startup;
  t_boolean        startup_done;

  /* Library copy control fields */
  t_boolean        doing_copy;
  t_boolean        copy_done;
  /* Roundtrip time control fields */
  t_boolean        doing_roundtrip;
  t_boolean        roundtrip_done;

  /* JGG (22/04/2005): Communication values */
  int              comm_action;   /* Marks current Communication operation */
  dimemas_timer    logical_send;
  dimemas_timer    logical_recv;
  dimemas_timer    physical_send;
  dimemas_timer    physical_recv;

  dimemas_timer    put_into_ready;
  dimemas_timer    start_wait_for_message;
  dimemas_timer    min_time_to_be_preempted;
  dimemas_timer    next_event_timer;
  int              size_port;     /* Number of bytes to send/rec from/to port */
  struct t_link   *port_send_link,
                  *port_recv_link;
  struct t_port   *port;

  struct t_link   *copy_segment_link_source,
                  *copy_segment_link_dest;
  int              copy_segment_size;
  int              portid; /* thread user port */
  struct t_event  *event;

  struct t_last_comm
  {
    dimemas_timer ti;
    t_nano       bandwidth;
    int           bytes;
  } last_comm;

  dimemas_timer     initial_communication_time;

/*
  HERE I WANT TO AVOID USING A FILE POINTER FOR EACH THREAD BUT TO USE MMAP
  AND THAN READ THE TRF FILE AS IT WAS A STRING
*/
//   FILE           *file;
  char            *mmapped_file;
  unsigned long   mmap_position;

//   FILE             *file;
//   t_boolean         file_shared;               /* TRUE if sharing file pointer*/


  struct t_queue    modules;
  struct t_queue    Activity;
  struct t_cp_node *last_cp_node;
  t_boolean         global_op_done;
  int               number_buses;
  dimemas_timer     last_time_event_number;

  struct
  {
    dimemas_timer arrive_to_collective;
    dimemas_timer sync_time;
    dimemas_timer with_resources;
    dimemas_timer conclude_communication;
  } collective_timers;

  int       IO_blocking_point;
  t_boolean marked_for_deletion; /* Indica que s'ha d'eliminar quan es pugui */
  /* JGG IDENTIFICADOR */
  long int       th_copy_id;
  t_boolean      locked; /* INDICA SI EL THREAD SE HA BLOQUEADO */
  // t_thread_state current_state; /* Current state of thread (15/02/2005) */

  t_boolean      idle_block;  /* True if the thread has entered on NULL block */

  // When a deadlock is detected some operations could be ignored.
  struct t_queue ops_to_be_ignored;
  int counter_ops_already_ignored;

  // Or also the some operations can be injected.
  struct t_queue ops_to_be_injected;
  int counter_ops_already_injected;
};

struct t_semaphore
{
  int             sem_id;
  int             counter;
  struct t_queue  threads;
};

struct t_port
{
  int              portid; /* Port identifier */
  struct t_thread *thread; /* Creator thread */
  struct t_queue   send,   /* Blocked sender threads */
                   recv;   /* Blocked recv threads */
  int              sending;
};


struct t_file_system_parameters
{
  double disk_latency;
  double disk_bandwidth;
  double block_size;
  int    concurrent_requests;
  double hit_ratio;
};

/*
 * All account in based in this structure
 */
struct t_account
{
  int             nodeid;      /* Only when accounting on threads */
  int             iteration;   /* Iteration number */
  dimemas_timer   initial_time;
  dimemas_timer   final_time;

  /* Compute account time */
  dimemas_timer   cpu_time;            /* Cpu time spent */
  double          n_th_in_run;         /* Number of times thread goes to run */
  double          n_preempt_to_me;     /* Number times i've been preempted */
  double          n_preempt_to_other;  /* Number time i preempt others */
  dimemas_timer   time_ready_without_cpu; /* Time in ready queue with no
                                           * processor*/
  /* Send accounting */
  double          n_sends;        /* Total amount of sends */
  double          n_bytes_send;

  /* Recv accounting */
  double          n_recvs;
  double          n_bytes_recv;
  double          n_recvs_on_processor; /* Number of receives and message is on
                                         * processor (no rendez_vous) */
  double          n_recvs_must_wait;
  dimemas_timer   time_waiting_for_message;

  /* Misc. communication time accounting */
  dimemas_timer   latency_time;           /* Startup latencies */
  dimemas_timer   block_due_resources;    /* General Resource blocking */
  dimemas_timer   initial_wait_link_time; /* Inici del temps d'espera */
  dimemas_timer   block_due_link;         /* Temps esperant links */
  dimemas_timer   initial_wait_bus_time;  /* Inici del temps d'espera */
  dimemas_timer   block_due_buses;        /* Temps esperant busos */

  /* Disk I/O time accounting */
  dimemas_timer   initial_io_read_time;  /* Inici del temps d'espera */
  dimemas_timer   io_read_time;          /* Temps llegint */
  dimemas_timer   initial_io_write_time; /* Inici del temps d'espera */
  dimemas_timer   io_write_time;         /* Temps escribint */
  dimemas_timer   initial_io_call_time;  /* Inici del temps d'espera */
  dimemas_timer   io_call_time;          /* Temps en IO (si no es separen reads
                                          * i writes) */

  /* Group operations */
  double          n_group_operations;
  dimemas_timer   block_due_group_operations;
  dimemas_timer   group_operations_time;
};

struct t_cpu
{
  int              cpuid;
  struct t_account account; /* Account for this cpu */
  struct t_thread *current_thread;
  struct t_thread *current_thread_context;
  double           current_load;
  dimemas_timer    last_actualization;
  double           utilization;
  struct t_queue  *io;
  int              unique_number;
};

struct t_bus_utilization
{
  dimemas_timer    initial_time;
  struct t_thread *sender;
};

struct t_user_event_value_info
{
  int   value;  /* Valor */
  char *name;   /* Nom d'aquest valor */
};


struct t_user_event_info
{
  int type;              /* Tipus de l'event */
  int color;             /* Color del flag al pcf */
  char *name;            /* Nom d'aquest tipus */
  struct t_queue values; /* Cua amb les informacions dels possibles valors */
};

struct t_both
{
  struct t_port   *port;
  struct t_thread *thread_s;
  struct t_thread *thread_r;
};

struct t_copyseg
{
  struct t_thread *thread;
  struct t_node  *node_s;
  struct t_node  *node_d;
  int             size;
};

#define E_NIL    (struct t_event*) 0
#define QU_NIL   (struct t_queue *)0
#define ITEM_NIL (struct t_item *)0
#define Q_NIL    (struct t_time *)0
#define A_NIL    (void *)0
#define AC_NIL   (struct t_action *)0
#define TH_NIL   (struct t_thread *)0
#define C_NIL    (struct t_cpu *)0
#define N_NIL    (struct t_node *)0
#define MA_NIL   (struct t_machine *)0
#define DC_NIL   (struct t_dedicated_connection *)0
#define COM_NIL  (struct t_communicator *) 0
#define GOPD_NIL (struct t_global_op_definition *) 0
#define T_NIL    (struct t_task *)0
#define P_NIL    (struct t_Ptask *)0
#define L_NUL    (struct t_link *)-1
#define L_NIL    (struct t_link *)0
#define ACC_NIL  (struct t_account *)0
#define B_NIL    (struct t_both *)0
#define LI_NIL   (struct t_list *)0
#define BU_NIL   (struct t_bus_utilization *)0
#define M_NIL    (struct t_module *)0
#define F_NIL    (struct t_filed *)0

#define CO_NIL   (struct t_copyseg *)0
#define PO_NIL   (struct t_port *)0
#define S_NIL    (struct t_semaphore *)0

struct t_scheduler_actions
{
  char              *name;
  void             (*thread_to_ready) ();
  t_nano          (*get_execution_time) ();
  struct t_thread *(*next_thread_to_run) ();
  void             (*init_scheduler_parameters) ();
  void             (*clear_parameters) ();
  int              (*info) ();
  void             (*scheduler_init) (char*, struct t_machine*);
  void             (*scheduler_copy_parameters) ();
  void             (*scheduler_free_parameters) ();
  void             (*modify_priority) ();
  void             (*modify_preemption) ();
};

struct t_communic_actions
{
  char *name;
};

struct t_disk_action
{
  int             taskid;
  int             threadid;
  struct t_action action;
};

struct trace_operation                                                                                   
{
   unsigned int Ptask_id;
   unsigned int task_id;
   unsigned int thread_id;
   unsigned int file_offset;
};


#endif
