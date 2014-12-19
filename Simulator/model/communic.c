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

#include <define.h>
#include <types.h>
#include <extern.h>
#include <assert.h>
#include <dlfcn.h>

#include <float.h>
#include <math.h>

#include "communic.h"
#include "eee_configuration.h"


#include "events.h"
#include "simulator.h"
#include "sched_vars.h"
// #include "machine.h"
#include "node.h"
#include "paraver.h"
#include "random.h"
#include "cpu.h"
#include "links.h"

#ifdef USE_EQUEUE
#include "listE.h"
#else
#include "list.h"
#endif

#ifdef VENUS_ENABLED
#include "venusclient.h"
#endif

#define PERIODIC_TRAFFIC 10e9
// For venus, 10e9 is too much
#ifdef VENUS_ENABLED
#define VENUS_PERIODIC_TRAFFIC 1e9
#endif

/******************************************************************************
 * Global variables                                                           *
 *****************************************************************************/

t_boolean DATA_COPY_enabled;      /* True if data copy latency is enabled */
int       DATA_COPY_message_size; /* Maximun message size to compute data copy
                                   * latency */

t_boolean RTT_enabled; /* True if Round Trip Time is enabled */
t_nano   RTT_time;    /* Round Trip Time for messages greater than eager */

/*
 * To compute the traffic on external network
 */
double suma_missatges_xarxa_externa      = 0.0;
double increment_missatges_xarxa_externa = 0.0;

/*
 * To check the external traffic computation
 */
double param_external_net_alfa    = 0.1;              /* Ha de ser < 1 */
double param_external_net_periode = 86400000000.0;   /* En microsegons */
double param_external_net_beta    = 0.0; /* Coeficients que determinen */
double param_external_net_gamma   = 1.0; /* la influencia dels traffics*/

/******************************************************************************
 * External communications library management
 *****************************************************************************/

t_boolean external_comm_library_loaded = FALSE;
void     *external_comm_library = NULL;

int (* external_get_communication_type) (int sender_nodeid,
                                         int receiver_nodeid,
                                         int sender_taskid,
                                         int receiver_taskid,
                                         int mess_tag,
                                         int mess_size);

t_nano (* get_startup_value) (int sender_nodeid,
                              int receiver_nodeid,
                              int sender_taskid,
                              int receiver_taskid,
                              int mess_tag,
                              int mess_size);

t_nano (* get_bandwidth_value) (int sender_nodeid,
                                int receiver_nodeid,
                                int sender_taskid,
                                int receiver_taskid,
                                int mess_tag,
                                int mess_size);

int (* external_get_global_op_type) (int comm_id,
                                     int global_op_id,
                                     int bytes_send,
                                     int bytes_recv);

void (* external_compute_global_operation_time) (int     comm_id,
                                                 int     global_op_id,
                                                 int     bytes_send,
                                                 int     bytes_recv,
                                                 t_nano *latency_time,
                                                 t_nano *op_time);


/******************************************************************************
 * MACROS per accounting del temps esperant busos                             *
 *****************************************************************************/
#include "task.h"

/* FEC: Macro per assignar l'inici del temps d'un thread bloquejat esperant
 * un bus */
#define START_BUS_WAIT_TIME(thread) \
  { \
    struct t_account *account; \
    /* double aux; */ \
    account = current_account(thread); \
    ASS_ALL_TIMER ((account->initial_wait_bus_time), current_time); \
/* \
    TIMER_TO_FLOAT(current_time,aux); \
    fprintf(stderr,"\nS'activa temps wait bus P%d, T%d, t%d instant: %f\n", \
            IDENTIFIERS (thread), aux); \
*/ \
  }


/* FEC: Macro per acumular el temps d'un thread bloquejat esperant un bus */
#define ACCUMULATE_BUS_WAIT_TIME(thread) \
{ \
  dimemas_timer tmp_timer; \
  struct t_account *account; \
/*    double aux, aux2, aux3;*/ \
  account = current_account(thread); \
  FLOAT_TO_TIMER(0,tmp_timer); \
  if (!EQ_TIMER((account->initial_wait_bus_time),tmp_timer)) \
  { \
    SUB_TIMER(current_time, (account->initial_wait_bus_time), tmp_timer); \
    ADD_TIMER (tmp_timer, (account->block_due_buses),(account->block_due_buses)); \
/* \
    TIMER_TO_FLOAT(tmp_timer,aux); \
    TIMER_TO_FLOAT(current_time,aux2); \
    TIMER_TO_FLOAT((account->initial_wait_bus_time),aux3); \
    fprintf(stderr,"\n%f: Accumula P%d, T%d, t%d - %f = bus wait time: %f\n", \
            aux2,IDENTIFIERS (thread), aux3, aux); \
*/ \
    FLOAT_TO_TIMER(0,(account->initial_wait_bus_time)); \
  } \
}

/******************************************************************************
 * CAPÇALERES DE LES FUNCIONS INTERNES                                        *
 *****************************************************************************/

static t_nano bw_ns_per_byte (t_nano bandw);

static void periodic_external_network_traffic_init (void);

static void global_op_get_all_buses (struct t_thread *thread);

static void global_op_get_all_out_links (struct t_thread *thread);

static void global_op_get_all_in_links (struct t_thread *thread);

/* void
close_global_communication(struct t_thread *thread); */

static void start_global_op (struct t_thread *thread, int kind);

static void free_global_communication_resources (struct t_thread *thread);

static void close_global_communication (struct t_thread *thread);

static int get_communication_type (struct t_task  *task,
                                   struct t_task  *task_partner,
                                   int             mess_tag,
                                   int             mess_size,
                                   struct t_dedicated_connection **connection);

static int from_rank_to_taskid (struct t_communicator *comm, int root_rank);

// struct t_queue Global_op;

/*
static t_nano compute_startup (struct t_thread               *thread,
                                int                            kind,
                                struct t_node                 *node,
                                struct t_dedicated_connection *connection);
*/

t_nano compute_startup (struct t_thread                 *thread,
                        int                              send_taskid,
                        int                              recv_taskid,
                        struct t_node                   *send_node,
                        struct t_node                   *recv_node,
                        int                              mess_tag,
                        int                              mess_size,
                        int                              kind,
                        struct t_dedicated_connection   *connection);

static t_nano compute_copy_latency (struct t_thread *thread,
                                     struct t_node   *node,
                                     int              mess_size);

static dimemas_timer get_logical_receive(struct t_thread *thread,
                                         struct t_queue  *irecv_not_queue,
                                         int              ori,
                                         int              ori_thread,
                                         int              mess_tag,
                                         long long int    mess_size,
                                         int              communic_id,
                                         dimemas_timer    stored_logical_recv);

/*****************************************************************************
 * Initialization/Finalization of the communications module
 ****************************************************************************/

void COMMUNIC_Init (void)
{
  struct t_machine *machine;
  size_t            machines_it;

  char             *external_comm_library_name, *dlsym_error;


  if (debug & D_COMM)
  {
    // PRINT_TIMER (current_time);
    printf ("-> COMMUNICATIONS initialization\n");
  }

  /* JGG (2012/01/17): new ways to navigate through machines
  for (machine  = (struct t_machine *) head_queue(&Machine_queue);
       machine != MA_NIL;
       machine  = (struct t_machine *) next_queue(&Machine_queue))
  {
  */
  for (machines_it = 0; machines_it < Simulator.number_machines; machines_it++)
  {
    machine = &Machines[machines_it];

    machine->network.utilization         = 0;
    machine->network.total_time_in_queue = 0;
    ASS_ALL_TIMER (machine->network.last_actualization, current_time);
    machine->network.curr_on_network     = 0;
    machine->communication.policy        = COMMUNIC_FIFO;
  }

  /*
   * JGG (2012/01/13): Loads the communications explicit configuration, if file
   * exists
   */
  CONFIGURATION_Load_Communications_Configuration();

  /*
  if ((fichero_comm != (char *) 0) && (strcmp(fichero_comm,"") != 0))
  {
    free_reserved_pointer(); /* To ensure that 'fichero_comm' can be opened

    fi = IO_fopen (fichero_comm, "r");

    if (fi == (FILE *) 0)
    {
      panic ("Can't open communication configuration file %s\n",
             fichero_comm);
    }

    read_communication_config_file(fi, fichero_comm);

    IO_fclose (fi);
  }
  */

  /* S'inicialitza el calcul del traffic de la xarxa externa */

  periodic_external_network_traffic_init();

  /* Initialization of the possible external library */

  if ((external_comm_library_name = getenv("DIMEMAS_EXTERNAL_COMM_LIBRARY")) != NULL)
  {
    external_comm_library = dlopen(external_comm_library_name, RTLD_LAZY);

    if (!external_comm_library)
    {
      warning("-> WARN: Unable to open external communication library %s %s\n",
              external_comm_library_name,
              dlerror());
    }
    else
    { /* Check for all symbols required in the external comms library */
      external_get_communication_type = dlsym(external_comm_library, "external_get_communication_type");
      if ((dlsym_error = dlerror()) != NULL)
      {
        warning("-> WARN: Unable to load function \"external_get_communication_type\" from library \"%s\": %s\n",
                external_comm_library_name,
                dlsym_error);
        return;
      }

      get_startup_value = dlsym(external_comm_library, "get_startup_value");
      if ((dlsym_error = dlerror()) != NULL)
      {
        warning("-> WARN: Unable to load function \"get_startup_value\" from library \"%s\": %s\n",
                external_comm_library_name,
                dlsym_error);
        return;
      }

      get_bandwidth_value = dlsym(external_comm_library, "get_bandwidth_value");
      if ((dlsym_error = dlerror()) != NULL)
      {
        warning("-> WARN: Unable to load function \"get_bandwidth_value\" from library \"%s\": %s\n",
                external_comm_library_name,
                dlsym_error);
        return;
      }

      external_get_global_op_type = dlsym(external_comm_library, "external_get_global_op_type");
      if ((dlsym_error = dlerror()) != NULL)
      {
        warning("-> WARN: Unable to load function \"external_get_global_op_type\" from library \"%s\": %s\n",
                external_comm_library_name,
                dlsym_error);
        return;
      }

      external_compute_global_operation_time = dlsym(external_comm_library, "external_compute_global_operation_time");
      if ((dlsym_error = dlerror()) != NULL)
      {
        warning("-> WARN: Unable to load function \"external_compute_global_operation_time\" from library \"%s\": %s\n",
                external_comm_library_name,
                dlsym_error);
        return;
      }

      external_comm_library_loaded = TRUE;

      printf ("-> External communications modelling library loaded\n");
    }
  }
}

void COMMUNIC_End()
{

  struct t_Ptask  *Ptask;
  struct t_task   *task;
  struct t_thread *thread;
  struct t_node   *node;
  struct t_cpu    *cpu;
  struct t_send   *mess_source;
  struct t_action *action;
  struct t_recv   *mess;
  struct t_communicator *communicator;
  struct t_global_op_definition * glop;

  t_boolean pending_comm_msg_shown = FALSE;

  if (debug & D_COMM)
  {
    PRINT_TIMER (current_time);
    printf (": COMMUNIC_end called\n");
  }

  /* PENDING LINKS INFORMATION */

#ifdef USE_EQUEUE
  for (
    node  = (struct t_node *) head_Equeue (&Node_queue);
    node != N_NIL;
    node  = (struct t_node *) next_Equeue (&Node_queue)
  )
#else
  for (
    node  = (struct t_node *) head_queue (&Node_queue);
    node != N_NIL;
    node  = (struct t_node *) next_queue (&Node_queue)
  )
#endif
  {
    if (count_queue (& (node->th_for_in) ) != 0)
    {
      if (debug & D_LINKS)
      {
        PRINT_TIMER (current_time);
        printf (
          ": COMMUNIC_end Warning! %d threads waiting in link on node %d\n",
          count_queue (& (node->th_for_in) ),
          node->nodeid
        );
      }
      for (thread  = (struct t_thread *) head_queue (& (node->th_for_in) );
           thread != TH_NIL;
           thread  = (struct t_thread *) next_queue (& (node->th_for_in) ) )
      {
        PARAVER_Wait (0,
                      IDENTIFIERS (thread),
                      thread->last_paraver,
                      current_time,
                      PRV_BLOCKING_RECV_ST);

        new_cp_node (thread, CP_BLOCK);

        /*
        if (debug&D_LINKS)
        {
          printf ("             P%02d T%02d (t%02d)\n", IDENTIFIERS (thread));
        }
        */
      }
    }
    if (count_queue (& (node->th_for_out) ) != 0)
    {
      if (debug & D_LINKS)
      {
        PRINT_TIMER (current_time);
        printf (
          ": COMMUNIC_end %d threads waiting out link on node %d\n",
          count_queue (& (node->th_for_out) ),
          node->nodeid
        );
      }
      for (thread  = (struct t_thread *) head_queue (& (node->th_for_out) );
           thread != TH_NIL;
           thread  = (struct t_thread *) next_queue (& (node->th_for_out) ) )
      {
        PARAVER_Wait (0,
                      IDENTIFIERS (thread),
                      thread->last_paraver, current_time,
                      PRV_BLOCKING_RECV_ST);

        new_cp_node (thread, CP_BLOCK);

        /*
        if (debug&D_LINKS)
        {
          printf ("             P%02d T%02d (t%02d)\n", IDENTIFIERS (thread));
        }
        */
      }
    }
  }

  /* PENDING COMMUNICATIONS INFORMATION */

  for(Ptask  = (struct t_Ptask *) head_queue (&Ptask_queue);
      Ptask != P_NIL;
      Ptask  = (struct t_Ptask *) next_queue (&Ptask_queue))
  {
    for (communicator  = (struct t_communicator *)head_queue(&Ptask->Communicator);
         communicator != (struct t_communicator *)0;
         communicator  = (struct t_communicator *)next_queue(&Ptask->Communicator))
    {
      /* JGG (07/07/2010): Easy way to check if it is a global operation in flight */
      /* if (count_queue(&communicator->threads)!=0) */
      if (communicator->in_flight_op == TRUE)
      {
        size_t involved_tasks = 0;

        warning(": COMMUNIC_end: Application finished with a pending global operation\n");

        for(thread  = (struct t_thread *)head_queue(&communicator->threads);
            thread != (struct t_thread *)0;
            thread  = (struct t_thread *)next_queue(&communicator->threads))
        {
          glop = (struct t_global_op_definition *)
            query_prio_queue (&Global_op,
                              (t_priority)thread->action->desc.global_op.glop_id);

          /*
          warning(": COMMUNIC_end: tasks P%02d T%02d (t%02d) waiting for global operation %s\n",
                  IDENTIFIERS(thread),
                  glop->name);
          */

          involved_tasks++;
        }

        warning(": * %d tasks involved\n", involved_tasks);
      }
    }

    /* JGG (2012/01/12): New way to navigate through tasks
    for(task  = (struct t_task *) head_queue (&(Ptask->tasks));
        task != T_NIL;
        task  = (struct t_task *) next_queue (&(Ptask->tasks)))
    {
    */
    size_t i;
    for(i = 0; i < Ptask->tasks_count; i++)
    {
      task = &(Ptask->tasks[i]);

      if (count_queue (&(task->recv)) != 0)
      {
        warning (": COMMUNIC_end: Task %02d ends with %d threads waiting to recv a message:\n",
                 task->taskid,
                 count_queue (&(task->recv)));

        for(thread  = (struct t_thread *) head_queue (&(task->recv));
            thread != TH_NIL;
            thread  = (struct t_thread *) next_queue (&(task->recv)))
        {
          printf ("          * Thread %02d <-",
                  thread->threadid);
          action = thread->action;
          mess = &(action->desc.recv);

          printf ("          * Sender: T%02d  t%02d, Destination T%02d  t%02d  Tag: %02d CommId: %02d Size: %lld\n",
                  mess->ori,
                  mess->ori_thread,
                  thread->task->taskid,
                  thread->threadid,
                  mess->mess_tag,
                  mess->communic_id,
                  mess->mess_size);

          /* Avoid the generation if this last IDLE state in 'backaground'
           * threads*/
          if (thread->original_thread)
          {
            node = get_node_of_thread (thread);
            cpu  = get_cpu_of_thread (thread);
            PARAVER_Idle(0,
                         IDENTIFIERS (thread),
                         thread->last_paraver,
                         current_time);
          }
        }
      }

      if (count_queue (&(task->send)) != 0)
      {
        warning (": COMMUNIC_end: Task %02d ends with %d message(s) pending to send:\n",
          task->taskid,
          count_queue (&(task->send)));

        for(thread  = (struct t_thread *) head_queue (&(task->send));
            thread != TH_NIL;
            thread  = (struct t_thread *) next_queue (&(task->send)))
        {
          /* Avoid the generation if this last IDLE state in 'backaground'
           * threads*/
          if (thread->original_thread)
          {
            node = get_node_of_thread (thread);
            cpu = get_cpu_of_thread (thread);
            PARAVER_Idle(0,
                         IDENTIFIERS (thread),
                         thread->last_paraver,
                         current_time);
          }

          printf ("          * Thread %02d ->",
                  thread->threadid);
          action = thread->action;
          // Vladimir: it has to be action->desc.send
          mess_source = &(action->desc.send);
          // mess_source = &(action->desc.recv);

          printf (" Dest-: T%02d Tag: %02d CommId: %02d Size: %lld\n",
                  mess_source->dest,
                  mess_source->mess_tag,
                  mess_source->communic_id,
                  mess_source->mess_size);

        }
      }

      if (count_queue (&(task->mess_recv)) != 0)
      {
        warning (": COMMUNIC_end: Task %02d ends with %d message(s) in reception queue:\n",
          task->taskid,
          count_queue (&(task->mess_recv)));

        for(thread  = (struct t_thread *) head_queue (&(task->mess_recv));
            thread != TH_NIL;
            thread  = (struct t_thread *) next_queue (&(task->mess_recv)))
        {
          action = thread->action;
          mess_source = &(action->desc.send);
          printf ("          -> Sender: T%02d t%02d  destT%02d  destt%02d  Tag: %02d CommId: %d Size: %lld\n",
                  thread->task->taskid,
                  thread->threadid,
                  mess_source->dest,
                  mess_source->dest_thread,
                  mess_source->mess_tag,
                  mess_source->communic_id,
                  mess_source->mess_size);
        }
      }
    }
  }
}

/******************************************************************************
 * FUNCIÓ 'compute_startup'                                                   *
 *****************************************************************************/
t_nano compute_startup (struct t_thread                 *thread,
                        int                              send_taskid,
                        int                              recv_taskid,
                        struct t_node                   *send_node,
                        struct t_node                   *recv_node,
                        int                              mess_tag,
                        int                              mess_size,
                        int                              kind,
                        struct t_dedicated_connection   *connection)
{
  t_nano startup = (t_nano) 0;

  switch (kind)
  {
  case MEMORY_COMMUNICATION_TYPE:
    /* Es un missatge local al node */
    startup  = send_node->local_startup;
    startup += RANDOM_GenerateRandom (&randomness.memory_latency);
    break;
  case INTERNAL_NETWORK_COM_TYPE:
    /* Es un missatge de la xarxa interna a la maquina */
    startup  = send_node->remote_startup;
    startup += RANDOM_GenerateRandom (&randomness.network_latency);
    break;
  case EXTERNAL_NETWORK_COM_TYPE:
    /* Es un missatge entre dues maquines diferents per la xarxa externa. */
    startup  = send_node->external_net_startup;
    startup += RANDOM_GenerateRandom (&randomness.external_network_latency);
    break;
  case DEDICATED_CONNECTION_COM_TYPE:
    /* Es un missatge entre dues maquines diferents per una connexio
     * dedicada. */
    if (connection == NULL)
    {
      panic ("Error computing startup (P%02 T%02d t%02d) : void connection \n",
             IDENTIFIERS (thread));

    }
    startup = connection->startup;
    break;
  case EXTERNAL_MODEL_COM_TYPE:
    /* This message will use an external modelling */
    if (external_comm_library_loaded == FALSE)
    {
      panic("Error computing startup through the external library (not loaded)\n");
    }
    startup = get_startup_value(send_node->nodeid,
                                recv_node->nodeid,
                                send_taskid,
                                recv_taskid,
                                mess_tag,
                                mess_size);
    break;

  default:
    panic ("Error computing startup (P%02 T%02d t%02d): unknown comm type %d\n",
           IDENTIFIERS (thread),
           kind);
    break;
  }

  /* If the startup is smaller than the minimum granularity, we consider it 0,
     to avoid possible overlapped states in Paraver generation
  */
  if (startup < 1)
  {
    startup = 0;
  }

  return (startup);
}
/******************************************************************************
 * FUNCIÓ 'compute_copy_time'                                                 *
 *****************************************************************************/
t_nano compute_copy_latency (struct t_thread *thread,
                              struct t_node   *node,
                              int              mess_size)
{
  t_nano bw;
  t_nano copy_latency = (t_nano) 0;

  // just to avoid warning for not used parameter
  assert (thread != NULL);

  if (node == NULL)
  {
    panic ("Error computing copy latency: void node descriptor\n");
  }

  if (node->bandwidth != (t_nano) 0)
  {
    bw = bw_ns_per_byte(node->bandwidth);
    copy_latency = bw * mess_size;
  }

  return (copy_latency);
}

static dimemas_timer get_logical_receive(struct t_thread *thread,
                                         struct t_queue  *irecv_not_queue,
                                         int              ori,
                                         int              ori_thread,
                                         int              mess_tag,
                                         long long int    mess_size,
                                         int              communic_id,
                                         dimemas_timer    stored_logical_recv)
{
  /* Check if the partner IRecv to detect the actual 'logical_receive'
   * time */
  dimemas_timer  result;
  struct t_recv* irecv_not;

  if (wait_logical_recv)
  {
    result = stored_logical_recv;
  }
  else
  {
    if (debug & D_COMM)
    {
      PRINT_TIMER (current_time);
      printf (
        ": \t\tP%02d T%02d (t%02d) Looking for a possible IRecv notification \n",
        IDENTIFIERS (thread));
      PRINT_TIMER(current_time);
      printf (
        ": \t\tLookup Sender: T%02d (t%02d) Tag: %d Size: %lld Comm.Id: %d\n",
        ori,
        ori_thread,
        mess_tag,
        mess_size,
        communic_id);
    }

    for (irecv_not  = (struct t_recv*) head_queue(irecv_not_queue);
         irecv_not != NULL;
         irecv_not  = (struct t_recv*) next_queue(irecv_not_queue))
    {
      if (debug & D_COMM)
      {
        PRINT_TIMER (current_time);
        printf (
          ": \t\t\tCurrent: Sender: T%02d (t%02d) Tag: %d Size: %lld Comm.Id: %d",
          irecv_not->ori,
          irecv_not->ori_thread,
          irecv_not->mess_tag,
          irecv_not->mess_size,
          irecv_not->communic_id);
      }

      if ( ((irecv_not->ori         == ori)        || (irecv_not->ori        == -1)) &&
           ((irecv_not->ori_thread  == ori_thread) || (irecv_not->ori_thread == -1)) &&
           ( irecv_not->mess_tag    == mess_tag)                                     &&
           ( irecv_not->communic_id == communic_id))
      {
        if (debug & D_COMM)
        {
          printf(" <- Found!\n");
        }
        break;
      }
      else
      {
        if (debug & D_COMM)
        {
          printf("\n");
        }
      }
    }

    if (irecv_not != NULL)
    {
      if (debug & D_COMM)
      {
        PRINT_TIMER (current_time);
        printf (
          ": \t\tP%02d T%02d (t%02d) IRecv notification Comm.Id: %d restored. LOGICAL RECEIVE = ",
          IDENTIFIERS (thread),
          communic_id);
      }
      PRINT_TIMER(irecv_not->logical_recv);
      printf("\n");

      result = irecv_not->logical_recv;

      extract_from_queue(irecv_not_queue, (char*) irecv_not);
      free(irecv_not);
    }
    else
    {
      result = stored_logical_recv;
    }
  }

  return result;
}

/******************************************************************************
 * FUNCIÓ 'recompute_memory_bandwidth'                                        *
 *****************************************************************************/
static t_nano recompute_memory_bandwidth (struct t_thread *thread)
{
  t_nano         bandw;
  t_nano         ratio;
  struct t_node *node;

  node = get_node_of_thread (thread);

  bandw  = (t_nano) node->bandwidth;
  bandw += RANDOM_GenerateRandom (&randomness.memory_bandwidth);

  // printf("Recompute_memory_bandwidth bandw+random = %f\n", bandw);

  if (bandw != 0)
  {
    bandw = bw_ns_per_byte(bandw);
  }

  if (node->cur_memory_messages <= node->max_memory_messages)
  {
    ratio = 1.0;
  }
  else
  {
    ratio = ( (t_nano) node->max_memory_messages) /
            ( (t_nano) node->cur_memory_messages);
  }

  if (ratio != 0)
  {
    bandw = bandw / ratio;
  }
  else
  {
    panic ("bandw = 0 !\n");
  }

  /*
  printf("Recompute_memory_bandwidth node->bandwidth = %f bandw = %f ratio = %f\n",
         bw_ns_per_byte(node->bandwidth),
         bandw,
         ratio);
  */
  return bandw;
}


/******************************************************************************
 * FUNCIÓ 'recompute_internal_network_bandwidth'                                                *
 *****************************************************************************/
static t_nano recompute_internal_network_bandwidth (struct t_thread *thread)
{
  t_nano bandw;
  t_nano ratio;
//   t_nano interm;
//   struct t_thread *pending;
//   dimemas_timer tmp_timer;
//   dimemas_timer inter;
//   int pending_bytes;
//   struct t_bus_utilization *bus_utilization;
  struct t_node    *node;
  struct t_machine *machine;

  node = get_node_of_thread (thread);
  machine = node->machine;

  bandw = (t_nano) machine->communication.remote_bandwidth;
  bandw += RANDOM_GenerateRandom (&randomness.network_bandwidth);

  if (bandw != 0)
  {
    bandw = (t_nano) ( (t_nano) (1e9) / (1 << 20) / bandw);
  }

  if (machine->network.curr_on_network <= machine->communication.num_messages_on_network)
  {
    ratio = 1.0;
  }
  else
  {
    ratio = ( (t_nano) machine->communication.num_messages_on_network) /
            ( (t_nano) machine->network.curr_on_network);
  }

  if (ratio != 0)
  {
    bandw = bandw / ratio;
  }
  else
  {
    panic ("bandw = 0 !\n");
  }

  /* Aixo dona problemes perque crec que recalcula temps d'events
   * COM_TIMER_OUT_RESOURCES en lloc de COM_TIMER_OUT.
   * De totes maneres, com que tal com esta el ratio sempre sera 1 era com
   * si ja no s'esigues fent perque quedava sempre igual.

    for (bus_utilization =
          (struct t_bus_utilization *)
          head_queue(&machine->network.threads_on_network);
         bus_utilization != BU_NIL;
         bus_utilization =
          (struct t_bus_utilization *)
          next_queue(&machine->network.threads_on_network)
    )
    {
      if (bus_utilization->sender != thread)
      {
        pending = bus_utilization->sender;
        EVENT_extract_timer (M_COM, pending, &tmp_timer);
        SUB_TIMER (current_time, pending->last_comm.ti, inter);
        TIMER_TO_FLOAT (inter, interm);
        pending_bytes =
          pending->last_comm.bytes - (interm/pending->last_comm.bandwidth);

        pending->last_comm.bandwidth = bandw;
        pending->last_comm.bytes = pending_bytes;
        ASS_ALL_TIMER (pending->last_comm.ti, current_time);

        interm = pending_bytes*bandw;
        FLOAT_TO_TIMER (interm, tmp_timer);
        ADD_TIMER (current_time, tmp_timer, tmp_timer);
        pending->event =
          EVENT_timer (tmp_timer, NOT_DAEMON, M_COM, pending, COM_TIMER_OUT);
      }
    }
  *******************************************************************************/
  return (bandw);
}

/******************************************************************************
 * PROCEDURE 'periodic_external_network_traffic_init'                         *
 *****************************************************************************/
/*
 * Aquesta funcio inicialitza el calcul del traffic de la xarxa externa
 */
void periodic_external_network_traffic_init()
{

  dimemas_timer tmp_timer;

  suma_missatges_xarxa_externa      = 0.0;
  increment_missatges_xarxa_externa = 0.0;

  /* Es prepara la primera execucio de la rutina que cal executar periodicament
   * per recalcular el traffic de la xarxa externa no produit per l'aplicacio
   * que s'esta simulant. */
#ifdef VENUS_ENABLED
  if (VC_is_enabled() ) {
    ADD_TIMER (VENUS_PERIODIC_TRAFFIC, current_time, tmp_timer); /* grodrigu: 10e6 is too much when interfaced with Venus */
  }
  else {
    ADD_TIMER (PERIODIC_TRAFFIC, current_time, tmp_timer); /* grodrigu: 10e6 is too much when interfaced with Venus */
  }
#else
  ADD_TIMER (PERIODIC_TRAFFIC, current_time, tmp_timer);
#endif

  EVENT_timer (tmp_timer, NOT_DAEMON, M_COM, NULL, COM_EXT_NET_TRAFFIC_TIMER);
}

/******************************************************************************
 * PROCEDURE 'recompute_external_network_traffic'                             *
 *****************************************************************************/
/*
 * Aquesta funcio s'hauria de cridar cada vegada que s'envia un missatge nou a
 * traves de la xarxa externa.
 */
void recompute_external_network_traffic (int mess_size)
{
  increment_missatges_xarxa_externa += mess_size;
}

/******************************************************************************
 * PROCEDURE 'recompute_external_network_traffic'                             *
 *****************************************************************************/
/*
 * Aquesta funcio s'hauria d'executar periodicament. En principi cada 10 segons.
 */
void periodic_recompute_external_network_traffic()
{

  dimemas_timer tmp_timer;

  double periodic_traffic_interval = PERIODIC_TRAFFIC;
#ifdef VENUS_ENABLED
  if (VC_is_enabled()) {
    periodic_traffic_interval = VENUS_PERIODIC_TRAFFIC;
  }
#endif

  suma_missatges_xarxa_externa =
    suma_missatges_xarxa_externa * param_external_net_alfa +
    (increment_missatges_xarxa_externa / (periodic_traffic_interval) ) *
    (1 - param_external_net_alfa);

  if (debug & D_COMM)
  {
    PRINT_TIMER (current_time);
    printf (
      ": COMMUNIC\tPeriodic external network recompute - Tn=%.4f Inc=%.4f\n",
      suma_missatges_xarxa_externa,
      increment_missatges_xarxa_externa
    );
  }

  increment_missatges_xarxa_externa = 0;

  /*
    PRINT_TIMER (current_time);
    printf(": S'ha recalculat el traffic de la xarxa externa\n");
  */

  /* Es prepara la seguen execucio d'aquesta rutina */
#ifdef USE_EQUEUE
#ifdef VENUS_ENABLED
  if ( (top_Eevent (&Event_queue) != E_NIL) || (VC_is_enabled() && (top_Eevent (&Interactive_event_queue) != E_NIL) ) ) /* grodrigu: venus! */
#else
  if (top_Eevent (&Event_queue) != E_NIL)
#endif
#else
#ifdef VENUS_ENABLED
  if ( (top_event (&Event_queue) != E_NIL) || (VC_is_enabled() && (top_event (&Interactive_event_queue) != E_NIL) ) ) /* grodrigu: venus! */
#else
    if (top_event (&Event_queue) != E_NIL)
#endif
#endif
  {
    /* Si queden events es que cal seguir simulant, per tant, es pot encuar
       aquest event. Si no en quedessin no el podriem encuar perque es
       seguiria simulant sense que hi hagues cap altre event que aquests. */
#ifdef VENUS_ENABLED
    if (VC_is_enabled() ) {
      ADD_TIMER (VENUS_PERIODIC_TRAFFIC, current_time, tmp_timer); /* grodrigu: 10e6 is too much when interfaced with Venus */
    }
    else {
      ADD_TIMER (PERIODIC_TRAFFIC, current_time, tmp_timer); /* grodrigu: 10e6 is too much when interfaced with Venus */
    }
#else
    ADD_TIMER (PERIODIC_TRAFFIC, current_time, tmp_timer);
#endif

    EVENT_timer (tmp_timer, NOT_DAEMON, M_COM, NULL, COM_EXT_NET_TRAFFIC_TIMER);
  }
}


/******************************************************************************
 * FUNCIÓ 'external_network_general_traffic'                                  *
 *****************************************************************************/
/*
 * La funcio que defineix el comportament del traffic de la xarxa externa
 * independentment de la nostra aplicació hauria de ser definida per l'usuari.
 */
double external_network_general_traffic (dimemas_timer temps)
{
  double traffic;

  // fprintf (stderr, "\nFunction disabled (external_network_general_traffic) because of compilation problem - Vladimir,14-07-2009!\n\n");

  // exit (EXIT_FAILURE);
  // return 0;

  /* traffic = (sin(aux * 2 * M_PI) + 1) / 2; /* Aqui traffic esta entre 0 i 1
  /* Aquesta funcio ha de retornar un numero entre 0 i
     l'ample de banda maxim de la xarxa externa.
  traffic = traffic *
            (Simulator.wan.bandwidth * (1 << 20) / (t_nano) (1e9));

  // to avoid warning for unused parameter
  // temps = temps;

  //
  //
  //   /* Per fer alguna cosa hi poso aixo: */
//   double aux;
//   /* Per tenir un periode d'un dia */
//   unsigned long long temp1 = (unsigned long long) temps;
//   long temp2               = (long) param_external_net_periode;
//   double temp3 =  (double)(temp1 % (temp2 + 1));
//   aux = ((temp3) / param_external_net_periode);
//
// //  aux = ((double)(temp1 %
// //            (temp2 + 1)) / param_external_net_periode);
//
// //  aux = ((double)((unsigned long long) temps %
// //        ((long) param_external_net_periode + 1)) / param_external_net_periode);
//
//   traffic = (sin(aux * 2 * M_PI) + 1) / 2; /* Aqui traffic esta entre 0 i 1 */
//   /* Aquesta funcio ha de retornar un numero entre 0 i
//      l'ample de banda maxim de la xarxa externa. */
//   traffic = traffic *
//             (Simulator.wan.bandwidth * (1 << 20) / (t_nano) (1e9));
//
//   if (debug&D_COMM)
//   {
//     PRINT_TIMER (current_time);
//     printf (
//       ": COMMUNIC\tExternal Network Traffic = %.4f\n",
//       traffic
//     );
//   }
//  return(traffic);

  return 0;

}

/******************************************************************************
 * FUNCIÓ 'external_network_application_traffic'                              *
 *****************************************************************************/
/*
 * Retorna el valor del número de missatges a la xarxa externa
 */
double
external_network_application_traffic()
{
  /* Hauria de retornar un numero a partir de suma_missatges_xarxa_externa. */
  return (suma_missatges_xarxa_externa);
}

/******************************************************************************
 * FUNCIÓ 'external_network_bandwidth_ratio'                                  *
 *****************************************************************************/
/*
 * Retorna el valor del ratio de l'ample de banda en funció del tràfic
 * existent, i la modelització del mateix
 */
double external_network_bandwidth_ratio (double traffic)
{
  double ratio;

  /* El parametre traffic pot ser mes gran que l'ample de banda
     maxim de la xarxa externa. */

  switch (Simulator.wan.traffic_function)
  {
  case EXP: /* Comportament Exponencial */
    /*ratio=pow(M_E, (-5*traffic/Simulator.wan.max_traffic_value));*/
    if (Simulator.wan.max_traffic_value != 0)
    {
      ratio = exp (-5 * traffic / Simulator.wan.max_traffic_value);
    }
    else
    {
      ratio = 1;
    }
    break;
  case LOG: /* Comportament logaritmic */
    if (Simulator.wan.max_traffic_value != 0)
    {
      ratio = log10 (10 - (10 * traffic / Simulator.wan.max_traffic_value) );
    }
    else
    {
      ratio = 1;
    }
    break;
  case LINEAL: /* Comportament linial */
    if ( (Simulator.wan.max_traffic_value != 0) && (traffic != 0) )
    {
      if (traffic < Simulator.wan.max_traffic_value)
      {
        ratio = (Simulator.wan.max_traffic_value - traffic) /
                Simulator.wan.max_traffic_value;
      }
      else
      {
        ratio = 0;
      }
    }
    else
    {
      ratio = 1;
    }
    break;
  case CONST: /* Comportament constant */
    ratio = 1;
    break;
  default:
    ratio = 1;
    break;
  }

  /* Aquesta funcio ha de retornar un numero entre 0 i 1 */
  return ratio;
}


/******************************************************************************
 * FUNCIÓ 'bw_ns_per_byte'                                           *
 *****************************************************************************/
/*
 * Aquesta funció passa de Mbytes a bytes, de segons a microsegons i calcula la
 * inversa. Aixi nomes cal multiplicar per la mida del missatge per obtenir el
 * temps estimat de transferencia. El resultat sera microsegons/byte.
 */
static t_nano bw_ns_per_byte (t_nano bandw)
{
  t_nano bandw_ms_per_byte = 0;

  if (bandw != 0)
  {
    bandw_ms_per_byte = (t_nano) ( (t_nano) (1e9) / (1 << 20) / bandw);
  }

  return bandw_ms_per_byte;
}

/******************************************************************************
 * FUNCIÓ 'recompute_external_network_bandwidth'                              *
 *****************************************************************************/
static t_nano
recompute_external_network_bandwidth (struct t_thread *thread)
{
  t_nano bandw;
  t_nano bandw_ms_per_byte;
  t_nano ratio;
  /*
    t_nano interm;
    struct t_thread *pending;
    dimemas_timer tmp_timer;
    dimemas_timer inter;
    int pending_bytes;
    struct t_bus_utilization *bus_utilization;
  */
  double traffic, traffic_indep, traffic_aplic;

  //to avoid warning about the unused parameter
  assert (thread);

  /* El primer que cal fer es calcular el traffic per poder indexar la funcio
   * que ens determinara el coeficient de l'ample de banda maxim que cal agafar
   * (el ratio). */

  /* Es calcula el traffic de la xarxa externa independent de nosaltres */
  traffic_indep = external_network_general_traffic (current_time);

  /* S'obte el traffic que hi ha a la xarxa per culpa de la propia
     aplicacio que estem simulant */
  traffic_aplic = external_network_application_traffic();

  /* Es calcula el traffic total */
  traffic = param_external_net_gamma * traffic_indep +
            param_external_net_beta  * traffic_aplic;

  /* Es calcula el ratio en funcio del traffic */
  ratio = external_network_bandwidth_ratio (traffic);

  /* S'obte l'ample de banda maxim */
  bandw = (t_nano) Simulator.wan.bandwidth;

  bandw += RANDOM_GenerateRandom (&randomness.external_network_bandwidth);

  /* Es calcula l'ample de banda final */
  if (ratio != 0)
  {
    bandw = bandw * ratio;
  }
  else
  {
    panic ("External network ratio equals 0\n");
  }

  bandw_ms_per_byte = bw_ns_per_byte (bandw);

  if (debug & D_COMM)
  {
    PRINT_TIMER (current_time);
    printf (
      ": COMMUNIC\tNew External Network Bandwidth = %.4f\n",
      bandw_ms_per_byte
    );
  }

  /* Aqui es recalcularien els temps estimats de totes les transferencies que
   * s'estiguessin fent per la xarxa externa en aquest instant. Pero aixo esta
   * desactivat perque no es vol aplicar. Nomes es vol fer que es calculi el
   * temps estimat una vegada, al comenc,ar la comunicacio.
   * Si es volgues recalcular aixo tambe caldria descomentar del
   * 'really_send_external_network' on es guarden les utilitzacions de bus i del
   * 'COMMUNIC_external_network_COM_TIMER_OUT' descomentar on s'alliberen els
   * busos. */
  /*
    for (
      bus_utilization = (struct t_bus_utilization *)
                        head_queue(&Simulator.wan.threads_on_network);
      bus_utilization != BU_NIL;
      bus_utilization = (struct t_bus_utilization *)
                        next_queue(&Simulator.wan.threads_on_network)
    )
    {
      if (bus_utilization->sender != thread)
      {
        pending = bus_utilization->sender;
        EVENT_extract_timer (M_COM, pending, &tmp_timer);
        SUB_TIMER (current_time, pending->last_comm.ti, inter);
        TIMER_TO_FLOAT (inter, interm);
        pending_bytes =
          pending->last_comm.bytes -(interm/pending->last_comm.bandwidth);

        pending->last_comm.bandwidth = bandw_ms_per_byte;
        pending->last_comm.bytes    = pending_bytes;
        ASS_ALL_TIMER (pending->last_comm.ti, current_time);

        interm = pending_bytes*bandw_ms_per_byte;
        FLOAT_TO_TIMER (interm, tmp_timer);
        ADD_TIMER (current_time, tmp_timer, tmp_timer);
        pending->event =
          EVENT_timer (tmp_timer, NOT_DAEMON, M_COM, pending, COM_TIMER_OUT);
      }
    }
  */
  return (bandw);
}

/******************************************************************************
 * FUNCIÓ 'locate_receiver'                                                   *
 *****************************************************************************/
/*
 * Find a receiver thread in queue asking for mess_tag
 */
static struct t_thread* locate_receiver_real_MPI_transfer (
  struct t_queue *threads,
  int taskid_ori,
  int threadid_ori,
  int dest_threadid,
  int mess_tag,
  int communic_id
)
{
  struct t_thread *thread;
  struct t_action *action;
  struct t_recv  *mess;

  assert (dest_threadid == -1);

  for (thread  = (struct t_thread *) head_queue (threads);
       thread != TH_NIL;
       thread  = (struct t_thread *) next_queue (threads))
  {
    action = thread->action;
    mess   = & (action->desc.recv);

    /* rod: following checks should be added. Sometimes actions is WORK, or other */
    //if (action->action != RECV || action->action != SEND)
    //   continue;
    assert (mess->ori_thread == -1); // rod: True for SS?
    /* if (debug)
      PRINT_TIMER (current_time);
      printf(": Locating a message for %d, %d, %d, %d, %d\n", taskid_ori, threadid_ori, dest_threadid, communic_id, mess_tag);
    } */

//     assert(thread->task->taskid == dest);
//     assert(dest > 0);
    if ( ( (thread->threadid      == dest_threadid) || (dest_threadid == -1) )    &&
         /*((thread->task->taskid == dest)          || (dest          == -1)      && */
         (mess->mess_tag          == mess_tag)                                    &&
         (mess->communic_id       == communic_id)                                 &&
         ( (mess->ori             == taskid_ori)    || (mess->ori        == -1) ) &&
         ( (mess->ori_thread      == threadid_ori)  || (mess->ori_thread == -1) )
       )
    {
      return (thread);
    }
  }
  return (TH_NIL);
}

static struct t_thread* locate_receiver_dependencies_synchronization (struct t_queue *threads,
                                                                      int taskid_ori,
                                                                      int threadid_ori,
                                                                      int dest_threadid,
                                                                      int mess_tag,
                                                                      int communic_id)
{
  struct t_thread *thread;
  struct t_action *action;
  struct t_recv  *mess;

  assert (dest_threadid != -1);
  for (
    thread  = (struct t_thread *) head_queue (threads);
    thread != TH_NIL;
    thread  = (struct t_thread *) next_queue (threads)
  )
  {
    action = thread->action;
    mess   = & (action->desc.recv);
    assert (mess->ori_thread != -1);

//     assert(thread->task->taskid == dest);
//     assert(dest > 0);
    if ( ( (thread->threadid     == dest_threadid) || (dest_threadid == -1) )    &&
         /*((thread->task->taskid == dest)          || (dest          == -1)     && */
         (mess->mess_tag        == mess_tag)                                   &&
         (mess->communic_id     == communic_id)                                &&
         ( (mess->ori            == taskid_ori)    || (mess->ori == -1) )        &&
         ( (mess->ori_thread     == threadid_ori)  || (mess->ori_thread == -1) )
       )
    {
      return (thread);
    }
  }
  return (TH_NIL);
}

/******************************************************************************
 * PROCEDURE 'message_received'                                               *
 *****************************************************************************/
static void message_received (struct t_thread *thread)
{
  struct t_node    *node, *node_partner;
  struct t_task    *task, *task_partner;
  struct t_thread  *thread_partner;
  struct t_action  *action;
  struct t_send    *mess;
  struct t_thread  *partner;
  struct t_account *account;
  dimemas_timer     tmp_timer, actual_logical_recv;
  struct t_recv    *mess_recv;
  struct t_cpu     *cpu_partner, *cpu;

  node   = get_node_of_thread (thread);
  task   = thread->task;
  action = thread->action;
  mess   = & (action->desc.send);

  task_partner = locate_task (task->Ptask, mess->dest);
  node_partner = get_node_of_task (task_partner);

  thread->physical_recv = current_time;

  if (mess->dest_thread == -1)
  {
    /* real MPI transfer - we look for it at the level of tasks  */
    partner = locate_receiver_real_MPI_transfer (&(task_partner->recv),
                                                 task->taskid,
                                                 thread->threadid,
                                                 mess->dest_thread,
                                                 mess->mess_tag,
                                                 mess->communic_id);
  }
  else
  {
    /* low level transfer - we look for it at the level of threads  */
    thread_partner = locate_thread_of_task (task_partner, mess->dest_thread);

    assert (thread_partner != TH_NIL);
    partner = locate_receiver_dependencies_synchronization (& (thread_partner->recv),
                                                            task->taskid,
                                                            thread->threadid,
                                                            mess->dest_thread,
                                                            mess->mess_tag,
                                                            mess->communic_id);
  }

  if (partner == TH_NIL)
  {
//  Vladimir: this is a case that I NEVER SAW: thread in doing_busy_wait
//  I MAKE IT AS AN ASSERTATION BECAUSE IT IS NOT SCALABLE
    /* El thread corresponent no esta bloquejat esperant a rebre.
     * Cal mirar si esta fent espera activa. */
    if (assert)
    {
      size_t i;

      /* JGG (2012/01/16): new way to navigate through threads
      for (partner  = (struct t_thread *) head_queue (&(task_partner->threads));
           partner != TH_NIL;
           partner  = (struct t_thread *) next_queue (&(task_partner->threads)))
      {
      */

      for (i = 0; i < task_partner->threads_count; i++)
      {
        partner = task_partner->threads[i];

        if ( (is_thread_running (partner) ) && (partner->doing_busy_wait) )
        {
          panic ("does this happen at all - espera activa ???? I BELIEVE NOT\n");
          action    = partner->action;
          mess_recv = & (action->desc.recv);

          assert (partner->task->taskid == mess->dest);
          assert (mess->dest > 0);
          if ( ( (partner->threadid      == mess->dest_thread) || (mess->dest_thread == -1) )       &&
               /* (partner->task->taskid  == mess->dest)        || (mess->dest        == -1))       && */
               (mess_recv->mess_tag    == mess->mess_tag)                                        &&
               (mess_recv->communic_id == mess->communic_id)                                     &&
               ( (mess_recv->ori         == task->taskid)      || (mess_recv->ori == -1) )          &&
               ( (mess_recv->ori_thread  == thread->threadid)  || (mess_recv->ori_thread == -1) ) )
          {
            partner->doing_busy_wait    = FALSE;
            cpu_partner                 = get_cpu_of_thread (partner);
            cpu_partner->current_thread = TH_NIL;

            EVENT_extract_timer (M_SCH, partner, &tmp_timer);

            PARAVER_Busy_Wait (cpu_partner->unique_number,
                               IDENTIFIERS (partner),
                               partner->last_paraver,
                               current_time);
            partner->last_paraver = current_time;
            if (debug & D_COMM)
            {
              PRINT_TIMER (current_time);
              printf (": COMMUNIC_send\tP%02d T%02d (t%02d) -> T%02d Busy Wait\n",
                      IDENTIFIERS (thread),
                      mess->dest);
            }

            account = current_account (partner);
            account->n_bytes_recv += mess->mess_size;
            cpu = get_cpu_of_thread (thread);

            /**
             * Look for a possible IRecv notification!
             */
            actual_logical_recv = get_logical_receive(thread,
                                                      &(task_partner->irecvs_executed),
                                                      mess_recv->ori,
                                                      mess_recv->ori_thread,
                                                      mess_recv->mess_tag,
                                                      mess_recv->mess_size,
                                                      mess_recv->communic_id,
                                                      partner->logical_recv);

            if (debug & D_PRV)
            {
              PRINT_TIMER(current_time);
              printf(": COMMUNIC_send\tP%02d T%02d (t%02d) Printing communication with Comm.Id: %d\n",
                     IDENTIFIERS(thread),
                     mess->communic_id);
            }

            PARAVER_P2P_Comm (cpu->unique_number,
                              IDENTIFIERS (thread),
                              thread->logical_send,
                              thread->physical_send,
                              cpu_partner->unique_number,
                              IDENTIFIERS (partner),
                              actual_logical_recv,
                              thread->physical_recv,
                              mess->mess_size,
                              mess->mess_tag);

            partner->last_paraver = current_time;
            action = partner->action;
            partner->action = action->next;
            READ_free_action(action);

            if (more_actions (partner) )
            {
              partner->loose_cpu = FALSE;
              SCHEDULER_thread_to_ready (partner);
              SCHEDULER_general (SCH_NEW_JOB, partner);
            }
            /* FEC: No es pot borrar el thread aqui perque encara es necessitara
            * quan es retorni a la crida d'on venim. Nomes es marca com a pendent
            * d'eliminar i ja s'esborrara mes tard. */
            thread->marked_for_deletion = 1;
            return;
          }
        }
      }
    }
    /* El thread corresponent encara no esta a punt per rebre el missatge.
     * El que cal fer es encuar el thread a la cua de missatges rebuts de
     * la task corresponent. */
    if (debug & D_COMM)
    {
      PRINT_TIMER (current_time);
      printf (
        ": COMMUNIC_send\tP%02d T%02d (t%02d) -> T%02d Tag(%d) Receiver Not Ready\n",
        IDENTIFIERS (thread),
        mess->dest,
        mess->mess_tag
      );
    }
    if (mess->dest_thread == -1)
    {
      /* this is a real MPI transfer */
      inFIFO_queue (& (task_partner->mess_recv), (char *) thread);
    }
    else
    {
      /* this is a dependency synchronization  */
      assert (thread_partner != TH_NIL);
      inFIFO_queue (& (thread_partner->mess_recv), (char *) thread);
    }

  }
  else
  {
    /* El thread corresponent esta bloquejat esperant a rebre. Per tant,
     * s'haura de desbloquejar. */

    account = current_account (partner);
    account->n_bytes_recv += mess->mess_size;
    SUB_TIMER (current_time, partner->start_wait_for_message, tmp_timer);
    ADD_TIMER (
      account->time_waiting_for_message,
      tmp_timer,
      account->time_waiting_for_message
    );

    if (mess->dest_thread == -1)
    {
      /* this is a real MPI transfer - it was at the queue inside task */
      extract_from_queue (& (task_partner->recv), (char *) partner);
    }
    else
    {
      /* this is a dependency - it was at the queue inside thread */
      assert (thread_partner != TH_NIL);
      extract_from_queue (& (thread_partner->recv), (char *) partner);
    }

    PARAVER_Wait (0,
                  IDENTIFIERS (partner),
                  partner->last_paraver,
                  current_time,
                  PRV_BLOCKING_RECV_ST);

    new_cp_node (partner, CP_BLOCK);
    new_cp_relation (partner, thread);
    cpu = get_cpu_of_thread (thread);
    cpu_partner = get_cpu_of_thread (partner);



    /**
     * Look for a possible IRecv notification!
     */
    actual_logical_recv = get_logical_receive(thread,
                                              &(partner->task->irecvs_executed),
                                              thread->task->taskid,
                                              thread->threadid,
                                              mess->mess_tag,
                                              mess->mess_size,
                                              mess->communic_id,
                                              partner->logical_recv);

    if (debug & D_PRV)
    {
      PRINT_TIMER(current_time);
      printf(": COMMUNIC_send\tP%02d T%02d (t%02d) Printing communication with Comm.Id: %d\n",
             IDENTIFIERS(thread),
             mess->communic_id);
    }

    PARAVER_P2P_Comm (cpu->unique_number,
                      IDENTIFIERS (thread),
                      thread->logical_send,
                      thread->physical_send,
                      cpu_partner->unique_number,
                      IDENTIFIERS (partner),
                      actual_logical_recv,
                      thread->physical_recv,
                      mess->mess_size,
                      mess->mess_tag );

    partner->last_paraver = current_time;

    action          = partner->action;
    partner->action = action->next;
    READ_free_action(action);
    if (more_actions (partner) )
    {
      partner->loose_cpu = TRUE;
      SCHEDULER_thread_to_ready (partner);
      SCHEDULER_general (SCH_NEW_JOB, partner);
    }
    /* FEC: No es pot borrar el thread aqui perque encara es necessitara
            quan es retorni a la crida d'on venim. Nomes es marca com a
            pendent d'eliminar i ja s'esborrara mes tard. */
    thread->marked_for_deletion = 1;

    if (debug & D_COMM)
    {
      PRINT_TIMER (current_time);
      printf (
        ": COMMUNIC_send\tP%02d T%02d (t%02d) -> T%02d Tag(%d) Receiver Unlocked\n",
        IDENTIFIERS (thread),
        mess->dest,
        mess->mess_tag
      );
    }
  }
}

/******************************************************************************
 * FUNCTION  'is_message_awaiting'                                            *
 *****************************************************************************/
static t_boolean is_message_awaiting_real_MPI_transfer (struct t_task   *task,
                                                        struct t_recv   *mess,
                                                        struct t_thread *thread)
{
  struct t_thread  *thread_source;
  struct t_task    *task_source;
  struct t_action  *action;
  struct t_send    *mess_source;
  struct t_account *account;
  struct t_node    *s_node, *r_node;
  struct t_cpu     *cpu, *cpu_source;
  t_boolean         result = FALSE, found;

  assert (mess->ori_thread == -1);

  if (debug & D_COMM)
  {
    PRINT_TIMER (current_time);
    printf (
      ": COMMUNIC_wait/recv\tP%02d T%02d (t%02d) <- T%02d Tag(%02d)  Size: %lld, Comm.Id: %d\n",
      IDENTIFIERS (thread),
      mess->ori,
      mess->mess_tag,
      mess->mess_size,
      mess->communic_id);
    PRINT_TIMER (current_time);
    printf(": COMMUNIC_wait/recv\t** Inspecting received messaged queue **\n");
  }

  for (thread_source = (struct t_thread *) head_queue (& (task->mess_recv) ), found = FALSE;
       thread_source != TH_NIL && !found;
       thread_source = (struct t_thread *) next_queue (& (task->mess_recv) ))
  {
    task_source = thread_source->task;
    action      = thread_source->action;
    mess_source = & (action->desc.send);

    if (debug & D_COMM)
    {
      PRINT_TIMER (current_time);
      printf (
        ": \t\t\t-> Message Sender: T%02d Tag(%02d)  Size: %lld, Comm.Id: %d\n",
        task_source->taskid,
        mess_source->mess_tag,
        mess_source->mess_size,
        mess_source->communic_id
      );
    }

    // JGG (2012/01/17): now, the numbering is 0..(n-1)
    // assert(mess_source->dest > 0);
    assert (mess_source->dest_thread == -1);
    assert (mess_source->dest == thread->task->taskid);
    if ( ( (thread_source->threadid  == mess->ori_thread)     || (mess->ori_thread == -1)         ) &&
         ( (task_source->taskid      == mess->ori)            || (mess->ori == -1)                ) &&
         (  mess->mess_tag           == mess_source->mess_tag                                     ) &&
         (  mess->communic_id        == mess_source->communic_id                                  ) &&
         ( (mess_source->dest_thread == thread->threadid)     || (mess_source->dest_thread == -1) ) )
    {
      dimemas_timer actual_logical_recv;
      account = current_account (thread);
      account->n_bytes_recv += mess_source->mess_size;
      extract_from_queue (& (task->mess_recv), (char *) thread_source);
      s_node = get_node_of_thread (thread_source);
      r_node = get_node_of_thread (thread);

      SCHEDULER_info (COMMUNICATION_INFO,
                      SCH_INFO_RECV_HIT,
                      thread_source,
                      thread);

      cpu        = get_cpu_of_thread (thread);
      cpu_source = get_cpu_of_thread (thread_source);


      actual_logical_recv = get_logical_receive(thread,
                                                &(task->irecvs_executed),
                                                mess->ori,
                                                mess->ori_thread,
                                                mess->mess_tag,
                                                mess->mess_size,
                                                mess->communic_id,
                                                thread->logical_recv);

      if (debug & D_PRV)
      {
        PRINT_TIMER(current_time);
        printf(": COMMUNIC_wait/recv\tP%02d T%02d (t%02d) Printing communication with Comm.Id: %d\n",
               IDENTIFIERS(thread),
               mess_source->communic_id);
      }

      PARAVER_P2P_Comm (cpu_source->unique_number,
                        IDENTIFIERS (thread_source),
                        thread_source->logical_send,
                        thread_source->physical_send,
                        cpu->unique_number,
                        IDENTIFIERS (thread),
                        actual_logical_recv,
                        thread_source->physical_recv, // The physical recv. is only annotated in the sender
                        mess_source->mess_size,
                        mess_source->mess_tag);

      /* In order to guarantee the cyclic simulation, we have to
         generate a special event to warn the 'prv2dim' translator the
         communication match (the PHYSICAL_RECV side appears in the
         middle of a computation burst) */


      /*
      CPU             = DstCPU;
      AppId           = DstAppId;
      TaskId          = DstTaskId;
      ThreadId        = DstThreadId;
      PartnerCPU      = SrcCPU;
      PartnerAppId    = SrcAppId;
      PartnerTaskId   = SrcTaskId;
      PartnerThreadId = SrcThreadId;
      */

      /*
      long long int types[7]  = { 10, 11, 12, 13, 14, 15, 16 };
      long long int values[7] = { cpu_source->unique_number,
                                  thread_source->task->Ptask->Ptaskid+1,
                                  thread_source->task->taskid+1,
                                  thread_source->threadid+1,
                                  mess_source->communic_id,
                                  mess_source->mess_size,
                                  mess_source->mess_tag };

      PARAVER_Multievent(cpu->unique_number,
                         IDENTIFIERS(thread),
                         current_time,
                         7,
                         types,
                         values);
      */

      new_cp_relation (thread, thread_source);
      thread->last_paraver = current_time;

      delete_duplicate_thread (thread_source);

      result = TRUE;
      found  = TRUE;
    }
    /* else
       printf("messages NOT matching\n");*/
  }

  if (!found)
  {
    /***************************************************/
    /* Say, the asynchronous message was not received! */

  }

  task_source   = locate_task (task->Ptask, mess->ori);
  /*
   * JGG (2012/01/16): first thread
  thread_source = (struct t_thread *) head_queue (&(task_source->threads));
  */
  thread_source = task_source->threads[0];

  SCHEDULER_info (COMMUNICATION_INFO,
                  SCH_INFO_RECV_MISS,
                  thread_source,
                  thread);

  return result;
}

static t_boolean is_message_awaiting_dependency_synchronization (
  struct t_task   *task,
  struct t_recv   *mess,
  struct t_thread *thread
)
{
  struct t_thread  *thread_source;
  struct t_task    *task_source;
  struct t_action  *action;
  struct t_send    *mess_source;
  struct t_account *account;
  struct t_node    *s_node, *r_node;
  struct t_cpu     *cpu, *cpu_partner;
  t_boolean         result = FALSE, found;

  assert (mess->ori_thread != -1);
  assert (mess->ori == task->taskid);

  for ( thread_source = (struct t_thread *) head_queue (& (thread->mess_recv) ),
        found = FALSE;
        thread_source != TH_NIL && !found;
        thread_source = (struct t_thread *) next_queue (& (thread->mess_recv) ) )
  {
    task_source = thread_source->task;
    action      = thread_source->action;
    mess_source = & (action->desc.send);

    if (debug & D_COMM)
    {
      PRINT_TIMER (current_time);
      printf (
        ": COMMUNIC_wait/recv\tP%02d T%02d (t%02d) <- T%02d Tag(%02d)  Size: %lld, Comm.Id: %d -  this is only matching - looking for the correct message\n",
        IDENTIFIERS (thread),
        task_source->taskid,
        mess_source->mess_tag,
        mess_source->mess_size,
        mess_source->communic_id
      );
    }

    assert (mess_source->dest_thread != -1);
    assert (mess_source->dest == task->taskid);

    if ( ( (thread_source->threadid  == mess->ori_thread)     || (mess->ori_thread == -1) )          &&
         ( (task_source->taskid      == mess->ori)            || (mess->ori == -1) )                 &&
           (mess->mess_tag           == mess_source->mess_tag)                                       &&
           (mess->communic_id        == mess_source->communic_id)                                    &&
         /*      ((mess_source->dest        == thread->task->taskid) || (mess_source->dest == -1))         &&   */
         ( (mess_source->dest_thread == thread->threadid)     || (mess_source->dest_thread == -1) ) )
    {
      dimemas_timer actual_logical_recv;
      account = current_account (thread);
      account->n_bytes_recv += mess_source->mess_size;
      extract_from_queue (& (thread->mess_recv), (char *) thread_source);
      s_node = get_node_of_thread (thread_source);
      r_node = get_node_of_thread (thread);

      SCHEDULER_info (
        COMMUNICATION_INFO,
        SCH_INFO_RECV_HIT,
        thread_source, thread
      );
      cpu         = get_cpu_of_thread (thread_source);
      cpu_partner = get_cpu_of_thread (thread);

      actual_logical_recv = get_logical_receive(thread,
                                                &(thread->irecvs_executed),
                                                mess->ori,
                                                mess->ori_thread,
                                                mess->mess_tag,
                                                mess->mess_size,
                                                mess->communic_id,
                                                thread->logical_recv);

      /*
      if (wait_logical_recv)
      {
        actual_logical_recv = thread->logical_recv;
      }
      else
      {
        /* Check if the partner IRecv to detect the actual 'logical_receive'
         * time
        struct t_recv* irecv_not;

        for (irecv_not  = (struct t_recv*) head_queue(& (thread->irecvs_executed));
             irecv_not != NULL;
             irecv_not  = (struct t_recv*) next_queue(& (thread->irecvs_executed)))
        {
          if ( ((irecv_not->ori_thread  == mess->ori_thread) || (mess->ori_thread == -1) ) &&
               ((irecv_not->ori         == mess->ori)        || (mess->ori == -1) )        &&
                (irecv_not->mess_tag    == mess_source->mess_tag)                          &&
                (irecv_not->communic_id == mess_source->communic_id))
          {
            break;
          }
        }

        if (irecv_not != NULL)
        {
          extract_from_queue(&(thread->irecvs_executed), (char*) irecv_not);
          actual_logical_recv = irecv_not->logical_recv;
          free(irecv_not);
        }
        else
        {
          actual_logical_recv = thread->logical_recv;
        }
      }
      */

      if (debug & D_PRV)
      {
        PRINT_TIMER(current_time);
        printf(": COMMUNIC_wait/recv\tP%02d T%02d (t%02d) Printing communication with Comm.Id: %d\n",
               IDENTIFIERS(thread),
               mess->communic_id);
      }

      PARAVER_P2P_Comm ( cpu->unique_number,
                         IDENTIFIERS (thread_source),
                         thread_source->logical_send,
                         thread_source->physical_send,
                         cpu_partner->unique_number,
                         IDENTIFIERS (thread),
                         actual_logical_recv,
                         thread_source->physical_recv,
                         mess_source->mess_size,
                         mess_source->mess_tag);

      new_cp_relation (thread, thread_source);
      thread->last_paraver = current_time;

      delete_duplicate_thread (thread_source);

      result = TRUE;
      found  = TRUE;
    }
    /* else
       printf("messages NOT matching\n");*/
  }

  task_source   = locate_task (task->Ptask, mess->ori);

  /* JGG (2012/01/16): first thread
  thread_source = (struct t_thread *) head_queue (&(task_source->threads));
  */
  thread_source = task_source->threads[0];
  SCHEDULER_info (COMMUNICATION_INFO,
                  SCH_INFO_RECV_MISS,
                  thread_source,
                  thread);

  return result;
}

/******************************************************************************
 * PROCEDURE 'Start_communication_if_partner_ready_for_rendez_vous'           *
 *****************************************************************************/

static void
Start_communication_if_partner_ready_for_rendez_vous_real_MPI_transfer (
  struct t_thread *thread,
  struct t_recv *mess
)
{
  register struct t_thread *sender;
  register struct t_task *task_sender;
  struct t_send *mess_sender;
  register struct t_action *action;
  struct t_thread *copy_thread;
  struct t_Ptask *Ptask;
  int trobat;

  assert (mess->ori_thread == -1);

  if (mess->ori != -1)
  {
    task_sender = locate_task (thread->task->Ptask, mess->ori);
    if (task_sender == T_NIL)
    {
      panic (
        "Unable to locate task %d in Ptask %d\n",
        mess->ori,
        thread->task->Ptask->Ptaskid
      );
    }
    for (sender = (struct t_thread *) head_queue (& (task_sender->send) );
         sender != TH_NIL;
         sender = (struct t_thread *) next_queue (& (task_sender->send) ) )
    {
      action = sender->action;
      mess_sender = & (action->desc.send);
      assert (mess_sender->dest_thread == -1);
      assert (sender->task->taskid == mess->ori);
      assert (mess->ori >= 0);
      assert (mess_sender->dest >= 0); /* no WILDCARDING WITH sends */
      if (
        ( (sender->threadid         == mess->ori_thread)     || (mess->ori_thread == -1) )
        /* &&((sender->task->taskid     == mess->ori)            || (mess->ori        == -1))    */
        && (mess_sender->mess_tag    == mess->mess_tag)
        && (mess_sender->communic_id == mess->communic_id)
        && ( (mess_sender->dest        == thread->task->taskid) || (mess_sender->dest == -1) )
        && ( (mess_sender->dest_thread == thread->threadid)     || (mess_sender->dest_thread == -1) )
      )
      {
        break;
      }
    }
  }
  else /* mess->ori == ANY, wildcarding -- ACCEPT FROM ANY SOURCE */
  {
    size_t tasks_it;
    Ptask  = thread->task->Ptask;
    trobat = FALSE;

    /* JGG (2012/01/16): new way to navigate through tasks
    for(task_sender=(struct t_task *) head_queue (&(ptask->tasks));
        task_sender!=T_NIL && trobat == FALSE;
        task_sender=(struct t_task *) next_queue (&(ptask->tasks)))
    {
    */
    for (tasks_it = 0; tasks_it < Ptask->tasks_count; tasks_it++)
    {
      task_sender = & (Ptask->tasks[tasks_it]);

      for (sender = (struct t_thread *) head_queue (& (task_sender->send) );
           sender != TH_NIL && trobat == FALSE;
           sender = (struct t_thread *) next_queue (& (task_sender->send) ) )
      {
        action = sender->action;
        mess_sender = & (action->desc.send);
        assert (mess_sender->dest_thread == -1);
        if (
          ( (sender->threadid         == mess->ori_thread)     || (mess->ori_thread == -1) )
          && ( (sender->task->taskid     == mess->ori)             || (mess->ori        == -1) )
          && (mess_sender->mess_tag    == mess->mess_tag)
          && (mess_sender->communic_id == mess->communic_id)
          && ( (mess_sender->dest        == thread->task->taskid) || (mess_sender->dest == -1) )
          && ( (mess_sender->dest_thread == thread->threadid)     || (mess_sender->dest_thread == -1) )
        )
        {
          trobat = TRUE;
        }
      }
    }
  } /* endif */

  if (sender == TH_NIL)
  {
    /* Unable to locate a message pending of being send. Do nothing */
    return;
  }

  extract_from_queue (& (task_sender->send), (char *) sender);
  /*  Si el thread és una copia és que s'està fent el rendez vous en
      un altre thread, per tant, no s'ha de generar res a la traça. */
  if (sender->original_thread)
  {
    PARAVER_Wait ( 0,
                   IDENTIFIERS (sender),
                   sender->last_paraver,
                   current_time,
                   PRV_BLOCKING_RECV_ST );

//    printf("start communication if there is the original thread on the sends list\n");

    sender->last_paraver = current_time;
    copy_thread          = duplicate_thread (sender);
  }
  else
  {
    copy_thread = sender;
//    printf("start communication if there is a copy thread on the sends list\n");

  }

  if (debug & D_COMM)
  {
    PRINT_TIMER (current_time);
    printf (
      ": RENDEZ VOUS\tP%02d T%02d (t%02d) <- P%02d T%02d (t%02d)\n",
      IDENTIFIERS (thread),
      IDENTIFIERS (copy_thread)
    );
  }

  ASS_ALL_TIMER (copy_thread->initial_communication_time, current_time);
  copy_thread->last_paraver = current_time;
  /* !!! */

  really_send (copy_thread);

  /* En cas que el thread sigui una copia no hi haura mes accions */
  if (sender->original_thread)
  {
    action = sender->action;
    sender->action = action->next;
    READ_free_action(action);
    if (more_actions (sender) )
    {
      sender->loose_cpu = FALSE;
      SCHEDULER_thread_to_ready (sender);
    }
  }
}

static void Start_communication_if_partner_ready_for_rendez_vous_dependency_synchronization (
  struct t_thread *thread,
  struct t_recv *mess
)
{
  register struct t_thread *sender, *thread_sender;
  struct t_send *mess_sender;
  register struct t_action *action;
  struct t_thread *copy_thread;

  assert (mess->ori_thread != -1);
  assert (mess->ori == thread->task->taskid);



  thread_sender = locate_thread_of_task (thread->task, mess->ori_thread);
  if (thread_sender == (struct t_thread*) T_NIL)
  {
    panic (
      "Unable to locate task %d in Ptask %d\n",
      mess->ori,
      thread->task->Ptask->Ptaskid
    );
  }

  for (sender = (struct t_thread *) head_queue (& (thread_sender->send) );
       sender != TH_NIL;
       sender = (struct t_thread *) next_queue (& (thread_sender->send) ) )
  {
    action = sender->action;
    mess_sender = & (action->desc.send);
    assert (mess_sender->dest == mess->ori);
    assert (sender->task->taskid == mess->ori);

    if (
      ( (sender->threadid         == mess->ori_thread)     || (mess->ori_thread == -1) )
//       &&((sender->task->taskid     == mess->ori)             || (mess->ori        == -1))
      && (mess_sender->mess_tag    == mess->mess_tag)
      && (mess_sender->communic_id == mess->communic_id)
//       &&((mess_sender->dest        == thread->task->taskid) || (mess_sender->dest == -1))
      && ( (mess_sender->dest_thread == thread->threadid)     || (mess_sender->dest_thread == -1) )
    )
    {
      break;
    }
  }


  if (sender == TH_NIL)
  {
    /* Unable to locate a message pending of being send. Do nothing */
    return;
  }

  assert (count_queue (& (thread_sender->send) ) > 0);
  extract_from_queue (& (thread_sender->send), (char *) sender);
  /*  Si el thread és una copia és que s'està fent el rendez vous en
      un altre thread, per tant, no s'ha de generar res a la traça. */
  if (sender->original_thread)
  {
    PARAVER_Wait (
      0,
      IDENTIFIERS (sender),
      sender->last_paraver,
      current_time,
      PRV_BLOCKING_RECV_ST
    );

//    printf("start communication if there is the original thread on the sends list\n");

    sender->last_paraver = current_time;
    copy_thread = duplicate_thread (sender);
  }
  else
  {
    copy_thread = sender;
//    printf("start communication if there is a copy thread on the sends list\n");

  }

  if (debug & D_COMM)
  {
    PRINT_TIMER (current_time);
    printf (
      ": RENDEZ VOUS\tP%02d T%02d (t%02d) <- P%02d T%02d (t%02d)\n",
      IDENTIFIERS (thread),
      IDENTIFIERS (copy_thread)
    );
  }

  ASS_ALL_TIMER (copy_thread->initial_communication_time, current_time);
  copy_thread->last_paraver = current_time;
  /* !!! */

  really_send (copy_thread);

  /* En cas que el thread sigui una copia no hi haura mes accions */
  if (sender->original_thread)
  {
    action = sender->action;
    sender->action = action->next;
    READ_free_action(action);
    if (more_actions (sender) )
    {
      sender->loose_cpu = FALSE;
      SCHEDULER_thread_to_ready (sender);
    }
  }
}

/******************************************************************************
 * FUNCTION 'COMMUNIC_memory_COM_TIMER_OUT'                                   *
 *****************************************************************************/
struct t_thread* COMMUNIC_memory_COM_TIMER_OUT (struct t_thread *thread)
{
  /* FEC: Es treu aixo per separar l'alliberació dels recursos utilitzats ******

  register struct t_thread *wait_thread;
  register struct t_bus_utilization *bus_utilization;
  struct t_node *node;
  struct t_machine *machine;
  int aux;
  *****************************************************************************/
  struct t_thread *copy_thread;


  if (thread->original_thread)
  {
    copy_thread = duplicate_thread (thread);
  }
  /* FEC: Jo crec que actualment mai no es dona aquest cas. Aixo nomes passaria
   * si els send realment siguesin sincrons durant la transferencia del missatge
   * i no simplement rendez vous. Es a dir, si estiguessin implementats els
   * casos RD_SYNC i NORD_SYNC. En aquest cas, jo crec que al final d'aquesta
   * funcio caldria posar el thread->local_link i el thread->partner_link a
   * L_NIL. Actualment no cal perque com que sempre es treballa amb una copia
   * del thread, s'acaba destruint. Es a dir, actualment el thread original mai
   * no te cap link assignat. */
  else
  {
    copy_thread = thread;
  }

  switch (thread->action->action)
  {
  case SEND:
    message_received (copy_thread);
    break;
  case MPI_OS:
    os_completed (copy_thread);
    break;
  }

  return (copy_thread);
}


/******************************************************************************
 * FUNCTION 'COMMUNIC_internal_network_COM_TIMER_OUT'                         *
 *****************************************************************************/
struct t_thread* COMMUNIC_internal_network_COM_TIMER_OUT (struct t_thread *thread)
{
  /* FEC: Es treu aixo per separar l'alliberació dels recursos utilitzats ******

  register struct t_thread *wait_thread;
  register struct t_bus_utilization *bus_utilization;
  struct t_node *node;
  struct t_machine *machine;
  int aux;
  *****************************************************************************/
  struct t_thread *copy_thread;


  if (thread->original_thread)
  {
    copy_thread = duplicate_thread (thread);
  }
  /* FEC: Jo crec que actualment mai no es dona aquest cas. Aixo nomes passaria
   * si els send realment siguesin sincrons durant la transferencia del missatge
   * i no simplement rendez vous. Es a dir, si estiguessin implementats els
   * casos RD_SYNC i NORD_SYNC. En aquest cas, jo crec que al final d'aquesta
   * funcio caldria posar el thread->local_link i el thread->partner_link a
   * L_NIL. Actualment no cal perque com que sempre es treballa amb una copia
   * del thread, s'acaba destruint. Es a dir, actualment el thread original mai
   * no te cap link assignat. */
  else
  {
    copy_thread = thread;
  }

  switch (thread->action->action)
  {
  case SEND:
    message_received (copy_thread);
#ifdef VENUS_ENABLED
    if (VC_is_enabled() ) {
      venusmsgs_in_flight--;
    }
#endif
    break;
  case MPI_OS:
    os_completed (copy_thread);
    break;
  }

  return (copy_thread);
}

/******************************************************************************
 * FUNCTION 'COMMUNIC_external_network_COM_TIMER_OUT'                         *
 *****************************************************************************/
/*
 * Aqui s'allibrerarien els busos reservats per poder recalcular els temps
 * estimats de totes les transferencies que s'estiguessin fent per la xarxa
 * externa quan comenc,a o acaba una transferencia. Pero aixo esta desactivat
 * perque no es vol aplicar. Nomes es vol fer que es calculi el temps estimat
 * una vegada, al comenc,ar la comunicacio. Si es volgues recalcular aixo tambe
 * caldria descomentar del really_send_external_network on es guarden les
 * utilitzacions de bus i del recompute_external_network_bandwidth el recalcul
 * de temps.
 */
struct t_thread* COMMUNIC_external_network_COM_TIMER_OUT (struct t_thread *thread)
{
  struct t_thread *copy_thread;

  /* FEC: Es treu aixo per separar l'alliberació dels recursos utilitzats	******

  register struct t_bus_utilization *bus_utilization;

  for (
    bus_utilization  = (struct t_bus_utilization *)
      head_queue(&Simulator.wan.threads_on_network);
      bus_utilization !=BU_NIL;
      bus_utilization  = (struct t_bus_utilization *)
      next_queue(&Simulator.wan.threads_on_network)
  )
  {
    if (bus_utilization->sender == thread)
    {
      break;
    }
  }

  if (bus_utilization == BU_NIL)
  {
    panic ("Unable to locate in external network bus utilization queue\n");
  }

  extract_from_queue (
    &Simulator.wan.threads_on_network,
    (char *)bus_utilization
  );
  free ( bus_utilization);

  if (debug&D_COMM)
  {
    PRINT_TIMER (current_time);
    printf (
      ": COMMUNIC\tP%02d T%02d (t%02d) free local machine link\n",
      IDENTIFIERS (thread)
    );
  }

  free_machine_link (thread->local_link, thread);
  *****************************************************************************/

  if (thread->original_thread)
  {
    copy_thread = duplicate_thread (thread);
  }
  /* FEC: Jo crec que actualment mai no es dona aquest cas. Aixo nomes
     * passaria si els send realment siguesin sincrons durant la transferencia
     * del missatge i no simplement rendez vous. Es a dir, si estiguessin
     * implementats els casos RD_SYNC i NORD_SYNC.
     * En aquest cas, jo crec que al final d'aquesta funcio caldria posar el
     * thread->local_link i el thread->partner_link a L_NIL. Actualment no
     * cal perque com que sempre es treballa amb una copia del thread, s'acaba
     * destruint. Es a dir, actualment el thread original mai no te cap link
     * assignat. */
  else
  {
    copy_thread = thread;
  }

  /* FEC: Es treu aixo per separar l'alliberació dels recursos utilitzats ******

  free_machine_link (thread->partner_link, thread);

  if (debug&D_COMM)
  {
    PRINT_TIMER (current_time);
    printf (
      ": COMMUNIC\tP%02d T%02d (t%02d) Free Remote Machine Link\n",
      IDENTIFIERS (thread)
    );
  }

  *****************************************************************************/

  switch (thread->action->action)
  {
  case SEND:
    message_received (copy_thread);
    break;
  case MPI_OS:
    os_completed (copy_thread);
    break;
  }

  /* FEC: Es treu aixo per separar l'alliberació dels recursos utilitzats ******

  recompute_external_network_bandwidth(thread);

  *****************************************************************************/

  /* Caldria mirar aqui alguna cosa per activar algun thread? */

  return (copy_thread);
}



/******************************************************************************
 * FUNCTION 'COMMUNIC_dedicated_connection_COM_TIMER_OUT'                     *
 *****************************************************************************/
struct t_thread* COMMUNIC_dedicated_connection_COM_TIMER_OUT (struct t_thread *thread)
{
  struct t_thread *copy_thread;

  /* FEC: Es treu aixo per separar l'alliberació dels recursos utilitzats ******

  free_connection_link (thread->local_link, thread);

  if (debug&D_COMM)
  {
    PRINT_TIMER (current_time);
    printf (
      ": COMMUNIC\tP%02d T%02d (t%02d) Free Local Connection Link\n",
      IDENTIFIERS (thread)
    );
  }
  *****************************************************************************/

  if (thread->original_thread)
  {
    copy_thread = duplicate_thread (thread);
  }
  /* FEC: Jo crec que actualment mai no es dona aquest cas. Aixo nomes
   * passaria si els send realment siguesin sincrons durant la transferencia
   * del missatge i no simplement rendez vous. Es a dir, si estiguessin
   * implementats els casos RD_SYNC i NORD_SYNC.
   * En aquest cas, jo crec que al final d'aquesta funcio caldria posar el
   * thread->local_link i el thread->partner_link a L_NIL. Actualment no
   * cal perque com que sempre es treballa amb una copia del thread, s'acaba
   * destruint. Es a dir, actualment el thread original mai no te cap link
   * assignat. */
  else
  {
    copy_thread = thread;
  }

  /* FEC: Es treu aixo per separar l'alliberació dels recursos utilitzats ******

  free_connection_link (thread->partner_link, thread);

  if (debug&D_COMM)
  {
    PRINT_TIMER (current_time);
    printf (
      ": COMMUNIC\tP%02d T%02d (t%02d) Free Remote Connection Link\n",
      IDENTIFIERS (thread)
    );
  }
  *****************************************************************************/

  switch (thread->action->action)
  {
  case SEND:
    message_received (copy_thread);
    break;
  case MPI_OS:
    os_completed (copy_thread);
    break;
  }

  /* Caldria mirar aqui alguna cosa per activar algun thread? */

  return (copy_thread);
}


/******************************************************************************
 * PROCEDURE 'COMMUNIC_COM_TIMER_OUT'                                         *
 *****************************************************************************/
void COMMUNIC_COM_TIMER_OUT (struct t_thread *thread)
{
  struct t_action *action;

  /* JGG (2013/03/27): it is a self-message! */
  if ( (thread->partner_link == L_NIL) && (thread->local_link == L_NIL) )
  {
    COMMUNIC_memory_COM_TIMER_OUT (thread);
    return;
    /* Es una comunicació local al node
    if (debug & D_COMM)
    {
      PRINT_TIMER (current_time);
      printf (
        ": COMMUNIC\tP%02d T%02d (t%02d) Local Channel Communication (BUSES)\n",
        IDENTIFIERS (thread)
      );
    }

    if (thread->original_thread)
    {
      panic ("Thread with syncronous communication, not implemented\n");
    }
    else
    {
      if (thread->action->action == SEND)
      {
        message_received (thread);
      }
      else if (thread->action->action == MPI_OS)
      {
        os_completed (thread);
      }
    }
    /* FEC: Com que al message_received no es pot eliminar el thread, si cal,
     * s'ha de fer aqui.
    if (thread->marked_for_deletion)
    {
      delete_duplicate_thread (thread);
    }
    return;
    */
  }

  /*
  if ( (thread->partner_link == L_NIL) && (thread->local_link == L_NUL) )
  {
    /* Es una comunicació amb PORTS
    if (debug & D_COMM)
    {
      PRINT_TIMER (current_time);
      printf (
        ": COMMUNIC\tP%02d T%02d (t%02d) Local Channel Communication (PORTS)\n",
        IDENTIFIERS (thread)
      );
    }

    if (thread->original_thread)
    {
      panic ("Thread with syncronous communication, not implemented\n");
    }
    else
    {

      if (debug & D_COMM)
      {
        PRINT_TIMER (current_time);
        printf (
          ": COMMUNIC\tP%02d T%02d (t%02d) Duplicated Thread Deleted\n",
          IDENTIFIERS (thread)
        );
      }
      delete_duplicate_thread (thread);
    }
    return;
  }
  */

  /* Si estem aqui es que es una comunicacio entre nodes o entre maquines.
   * Per saber el tipus de comunicacio nomes cal que mirem el tipus de
   * qualsevol dels dos links: */

  switch (thread->local_link->kind)
  {
    case MEM_LINK:
      COMMUNIC_memory_COM_TIMER_OUT (thread);
      break;

    case NODE_LINK:
      COMMUNIC_internal_network_COM_TIMER_OUT (thread);
      /*if (VC_is_enabled()) {
        venusmsgs_in_flight--;
      }*/
      break;
    case MACHINE_LINK:
      COMMUNIC_external_network_COM_TIMER_OUT (thread);
      break;
    case CONNECTION_LINK:
      COMMUNIC_dedicated_connection_COM_TIMER_OUT (thread);
      break;
    default:
      panic ("Unknown link type!\n");
  }

  if (thread->original_thread)
  {
    action         = thread->action;
    thread->action = action->next;
    READ_free_action(action);

    if (more_actions (thread) )
    {
      thread->loose_cpu = FALSE;
      SCHEDULER_thread_to_ready (thread);
    }
  }
  else
  {
    /* FEC: Com que al message_received no es pot eliminar el thread, si cal,
     * s'ha de fer aqui. */
    if (thread->marked_for_deletion)
    {
      delete_duplicate_thread (thread);
    }
  }
}

/******************************************************************************
 * PROCEDURE 'COMMUNIC_mem_resources_COM_TIMER_OUT'                           *
 *****************************************************************************/

static void COMMUNIC_mem_resources_COM_TIMER_OUT (struct t_thread *thread)
{
  register struct t_thread          *wait_thread;
  register struct t_bus_utilization *bus_utilization;
  struct t_node                     *node;
  struct t_machine                  *machine;

  int aux;

  node    = get_node_of_thread (thread);
  machine = node->machine;

  if (node->max_memory_messages != 0)
  {
    for (
      bus_utilization  = (struct t_bus_utilization *)
                         head_queue (&node->threads_in_memory);
      bus_utilization != BU_NIL;
      bus_utilization  = (struct t_bus_utilization *)
                         next_queue (&node->threads_in_memory)
    )
    {
      if (bus_utilization->sender == thread) break;
    }

    if (bus_utilization == BU_NIL)
    {
      panic ("Unable to locate in bus utilization queue\n");
    }

    extract_from_queue (&node->threads_in_memory, (char*) bus_utilization);

    free (bus_utilization);
  }

  if (thread->local_link != L_NIL && thread->partner_link != L_NIL)
  {
    if (debug & D_COMM)
    {
      PRINT_TIMER (current_time);
      printf (
        ": COMMUNIC\tP%02d T%02d (t%02d) Free memory writing permissions (OUT)\n",
        IDENTIFIERS (thread)
      );
    }

    LINKS_free_mem_link(thread->local_link, thread);

    if (debug & D_COMM)
    {
      PRINT_TIMER (current_time);
      printf (": COMMUNIC\tP%02d T%02d (t%02d) Free memory writing permissions (IN)\n",
              IDENTIFIERS (thread) );
    }

    LINKS_free_mem_link(thread->partner_link, thread);
  }

  if (node->cur_memory_messages > 0)
  {
    node->cur_memory_messages--;
  }

  recompute_memory_bandwidth (thread);

  if ( (node->max_memory_messages != 0) && (count_queue (&node->wait_for_mem_bus) > 0)
     )
  {
    wait_thread = (struct t_thread *) head_queue (&node->wait_for_mem_bus);

    if (debug & D_COMM)
    {
      PRINT_TIMER (current_time);
      printf (
        ": COMMUNIC\tP%02d T%02d (t%02d) Obtain Bus\n",
        IDENTIFIERS (wait_thread)
      );
    }

//    printf("\nResources are freed and now I should take the network for the pending transfer P%d, T%d t%d  action == %d\n\n", IDENTIFIERS (wait_thread), wait_thread->action->action);

    switch (wait_thread->action->action)
    {
    case SEND:
    case WAIT_FOR_SEND:
      /* FEC: S'acumula el temps que ha estat esperant busos */
      ACCUMULATE_BUS_WAIT_TIME (wait_thread);

      extract_from_queue (&node->wait_for_mem_bus, (char *) wait_thread);
//        printf("\nAGAIN THE REALLY_SEND FOR THE WAIT_THREAD   P%d, T%d t%d\n\n", IDENTIFIERS (wait_thread));
      really_send (wait_thread);
      break;

    case MPI_OS:
      /* FEC: S'acumula el temps que ha estat esperant busos */
      ACCUMULATE_BUS_WAIT_TIME (wait_thread);

      extract_from_queue (&node->wait_for_mem_bus, (char *) wait_thread);
      really_RMA (wait_thread);
      break;
    case GLOBAL_OP:
      /* aux sempre hauria de ser 1 */
      aux = node->max_memory_messages - node->cur_memory_messages;
      wait_thread->number_buses += aux;
      node->cur_memory_messages += aux;

      if (wait_thread->number_buses == node->max_memory_messages)
      {
        /* FEC: S'acumula el temps que ha estat esperant busos */
        ACCUMULATE_BUS_WAIT_TIME (wait_thread);

        extract_from_queue (&node->wait_for_mem_bus, (char *) wait_thread);
        global_op_get_all_buses (wait_thread);
      }
      break;
    }
  }

  return;
}


/******************************************************************************
 * PROCEDURE 'COMMUNIC_internal_resources_COM_TIMER_OUT'                      *
 *****************************************************************************/
/*
 * Alliberació dels recursos d'una comunicació punt a punt
 */
static void COMMUNIC_internal_resources_COM_TIMER_OUT (struct t_thread *thread)
{
  register struct t_thread          *wait_thread;
  register struct t_bus_utilization *bus_utilization;
  struct t_node                     *node;
  struct t_machine                  *machine;

  int aux;

  node    = get_node_of_thread (thread);
  machine = node->machine;

  if (machine->communication.num_messages_on_network)
  {
    for (
      bus_utilization  = (struct t_bus_utilization *)
                         head_queue (&machine->network.threads_on_network);
      bus_utilization != BU_NIL;
      bus_utilization  = (struct t_bus_utilization *)
                         next_queue (&machine->network.threads_on_network)
    )
    {
      if (bus_utilization->sender == thread) break;
    }

    if (bus_utilization == BU_NIL)
    {
      panic ("Unable to locate in bus utilization queue\n");
    }

    extract_from_queue (
      &machine->network.threads_on_network,
      (char *) bus_utilization
    );

    free (bus_utilization);
  }

  if (debug & D_COMM)
  {
    PRINT_TIMER (current_time);
    printf (
      ": COMMUNIC\tP%02d T%02d (t%02d) Free Local Link\n",
      IDENTIFIERS (thread)
    );
  }

  LINKS_free_network_link(thread->local_link, thread);

  if (debug & D_COMM)
  {
    PRINT_TIMER (current_time);
    printf (": COMMUNIC\tP%02d T%02d (t%02d) Free Remote Link\n",
            IDENTIFIERS (thread) );
  }

  LINKS_free_network_link(thread->partner_link, thread);

  if (machine->network.curr_on_network > 0)
  {
    machine->network.curr_on_network--;
  }

  recompute_internal_network_bandwidth (thread);

  if ( (machine->communication.num_messages_on_network) &&
       (count_queue (&machine->network.queue) > 0)
     )
  {
    wait_thread = (struct t_thread *) head_queue (&machine->network.queue);

    if (debug & D_COMM)
    {
      PRINT_TIMER (current_time);
      printf (
        ": COMMUNIC\tP%02d T%02d (t%02d) Obtain Bus\n",
        IDENTIFIERS (wait_thread)
      );
    }

//    printf("\nResources are freed and now I should take the network for the pending transfer P%d, T%d t%d  action == %d\n\n", IDENTIFIERS (wait_thread), wait_thread->action->action);

    switch (wait_thread->action->action)
    {
    case SEND:
    case WAIT_FOR_SEND:
      /* FEC: S'acumula el temps que ha estat esperant busos */
      ACCUMULATE_BUS_WAIT_TIME (wait_thread);

      extract_from_queue (&machine->network.queue, (char *) wait_thread);
//        printf("\nAGAIN THE REALLY_SEND FOR THE WAIT_THREAD   P%d, T%d t%d\n\n", IDENTIFIERS (wait_thread));
      really_send (wait_thread);
      break;

    case MPI_OS:
      /* FEC: S'acumula el temps que ha estat esperant busos */
      ACCUMULATE_BUS_WAIT_TIME (wait_thread);

      extract_from_queue (&machine->network.queue, (char *) wait_thread);
      really_RMA (wait_thread);
      break;
    case GLOBAL_OP:
      /* aux sempre hauria de ser 1 */
      aux = machine->communication.num_messages_on_network -
            machine->network.curr_on_network;
      wait_thread->number_buses += aux;
      machine->network.curr_on_network += aux;

      if (wait_thread->number_buses ==
          machine->communication.num_messages_on_network
         )
      {
        /* FEC: S'acumula el temps que ha estat esperant busos */
        ACCUMULATE_BUS_WAIT_TIME (wait_thread);

        extract_from_queue (&machine->network.queue, (char *) wait_thread);
        global_op_get_all_buses (wait_thread);
      }
      break;
    }
  }
  return;
}

/******************************************************************************
 * PROCEDURE 'COMMUNIC_internal_resources_COM_TIMER_OUT'                      *
 *****************************************************************************/
static void COMMUNIC_external_resources_COM_TIMER_OUT (struct t_thread *thread)
{
  /* Aqui s'allibrerarien els busos reservats per poder recalcular els
   * temps estimats de totes les transferencies que s'estiguessin fent
   * per la xarxa externa quan comenc,a o acaba una transferencia. Pero
   * aixo esta desactivat perque no es vol aplicar. Nomes es vol fer que
   * es calculi el temps estimat una vegada, al comenc,ar la comunicacio.
   * Si es volgues recalcular aixo tambe caldria descomentar del
   * really_send_external_network on es guarden les utilitzacions de bus
   * i del recompute_external_network_bandwidth el recalcul de temps.

  register struct t_bus_utilization *bus_utilization;

  for (
    bus_utilization  = (struct t_bus_utilization *)
      head_queue(&Simulator.wan.threads_on_network);
    bus_utilization != BU_NIL;
    bus_utilization  = (struct t_bus_utilization *)
      next_queue(&Simulator.wan.threads_on_network)
  )
  {
    if (bus_utilization->sender==thread) break;
  }
  if (bus_utilization==BU_NIL)
  {
    panic ("Unable to locate in external network bus utilization queue\n");
  }

  extract_from_queue (
    &Simulator.wan.threads_on_network,
    (char *)bus_utilization
  );
  free (bus_utilization);
  *****************************************************************************/

  LINKS_free_wan_link(thread->local_link, thread);

  if (debug & D_COMM)
  {
    PRINT_TIMER (current_time);
    printf (
      ": COMMUNIC\tP%02d T%02d (t%02d) Free Local Machine Link\n",
      IDENTIFIERS (thread)
    );
  }

  LINKS_free_wan_link(thread->partner_link, thread);

  if (debug & D_COMM)
  {
    PRINT_TIMER (current_time);
    printf (
      ": COMMUNIC\tP%02d T%02d (t%02d) Free Remote Machine Link\n",
      IDENTIFIERS (thread)
    );
  }

  recompute_external_network_bandwidth (thread);
}

/******************************************************************************
 * PROCEDURE 'COMMUNIC_dedicated_resources_COM_TIMER_OUT'                     *
 *****************************************************************************/
static void COMMUNIC_dedicated_resources_COM_TIMER_OUT (struct t_thread *thread)
{
  LINKS_free_dedicated_connection_link(thread->local_link, thread);

  if (debug & D_COMM)
  {
    PRINT_TIMER (current_time);
    printf (
      ": COMMUNIC\tP%02d T%02d (t%02d) Free Local Connection Link\n",
      IDENTIFIERS (thread)
    );
  }

  LINKS_free_dedicated_connection_link(thread->partner_link, thread);

  if (debug & D_COMM)
  {
    PRINT_TIMER (current_time);
    printf (
      ": COMMUNIC\tP%02d T%02d (t%02d) Free Remote Connection Link\n",
      IDENTIFIERS (thread)
    );
  }
}

/******************************************************************************
 * PROCEDURE 'COMMUNIC_resources_COM_TIMER_OUT'                               *
 *****************************************************************************/
void COMMUNIC_resources_COM_TIMER_OUT (struct t_thread *thread)
{

  if ( (thread->partner_link == L_NIL) && (thread->local_link == L_NIL) )
  {
    /* It is a self-message! */
    COMMUNIC_mem_resources_COM_TIMER_OUT (thread);
    return;
  }

  /*
  if ( (thread->partner_link == L_NIL) && (thread->local_link == L_NUL) )
  {
    /* Es una comunicació amb PORTS
    return;
  }
  */

  /* Si estem aqui es que es una comunicacio entre nodes o entre maquines */
  /* Per saber el tipus de comunicacio nomes cal que mirem el tipus de
     qualsevol dels dos links: */
  switch (thread->local_link->kind)
  {
  case MEM_LINK:
    COMMUNIC_mem_resources_COM_TIMER_OUT (thread);
    break;

  case NODE_LINK:
    COMMUNIC_internal_resources_COM_TIMER_OUT (thread);
    break;

  case MACHINE_LINK:
    COMMUNIC_external_resources_COM_TIMER_OUT (thread);
    break;

  case CONNECTION_LINK:
    COMMUNIC_dedicated_resources_COM_TIMER_OUT (thread);
    break;

  default:
    panic ("Unknown link type!\n");
  }
  /***** FEC: Fi alliberació dels recursos d'una comunicació punt a punt ******/
}

void COMMUNIC_general (int value, struct t_thread *thread)
{
  switch (value)
  {
  case RMA_TIMER_OUT:
  case COM_TIMER_OUT:
    COMMUNIC_COM_TIMER_OUT (thread);
    break;

  case COM_TIMER_OUT_RESOURCES:
    /* L'operació punt a punt encara no s'ha acabat, però ja es poden
        alliberar els recursos que té reservats. */
    COMMUNIC_resources_COM_TIMER_OUT (thread);
    break;

  case COM_TIMER_GROUP_RESOURCES:
    /* L'operació col.lectiva encara no s'ha acabat, però ja es poden
       alliberar els recursos que té reservats. */
    free_global_communication_resources (thread);
    break;

  case COM_TIMER_GROUP:
    /* The global operation is completed, restart blocked threads */
    close_global_communication (thread);
    break;

  case COM_EXT_NET_TRAFFIC_TIMER:
    periodic_recompute_external_network_traffic();
    break;
  default:
    panic ("Incorrect command %d to routine COMMUNIC_general\n", value);
  }
}


/******************************************************************************
 * PROCEDURE 'COMMUNIC_recv_reached                                           *
 * ÚLTIMA MODIFICACIÓN: 29/10/2004 (Juan González García)                     *
 *****************************************************************************/
/* Serveix per indicar que s'ha arribat a un recv/Irecv. Cal comprovar si
 * s'havia arribat al send corresponent i actuar en consequencia. Si es troba
 * que s'ha fet un Send SINCRON corresponent a aquest Irecv, es retorna 1 per
 * indicar que cal desbloquejar el send immediatament. En cas contrari es
 * retorna 0.
 */
int COMMUNIC_recv_reached_real_MPI_transfer (struct t_thread *thread,
                                             struct t_recv   *mess)
{
  struct t_task   *task, *source_task;
  struct t_thread *partner_send;
  struct t_action *action;
  struct t_send   *mess_send;
  struct t_thread *copia_thread;
  int              res   = 0;

  assert (mess->ori_thread == -1);
  task = thread->task;
  for (
    partner_send  = (struct t_thread *) head_queue (& (task->send_without_recv) );
    partner_send != TH_NIL;
    partner_send  = (struct t_thread *) next_queue (& (task->send_without_recv) )
  )
  {
    source_task = partner_send->task;
    action      = partner_send->action;
    mess_send   = &(action->desc.send);

    assert (mess_send->dest_thread == -1);
    assert (mess_send->dest == thread->task->taskid);
    /* JGG (2012/01/18): now, internal structures are indexed from 0 to n-1
    assert(mess_send->dest > 0); */
    if ( ( (partner_send->threadid     == mess->ori_thread)     || (mess->ori_thread == -1) )  &&
         ( (partner_send->task->taskid == mess->ori)            || (mess->ori == -1) )         &&
         (mess_send->mess_tag        == mess->mess_tag)                                     &&
         (mess_send->communic_id     == mess->communic_id)                                  &&
         /*    ((mess_send->dest            == thread->task->taskid) || (mess_send->dest == -1))   &&   */
         ( (mess_send->dest_thread     == thread->threadid)     || (mess_send->dest_thread == -1) ) )
    {
      /* Ja s'havia arribat al send corresponent */
      break;
    }
  }



  if (partner_send != TH_NIL)
  {
    /* Cal comprovar si el Send corresponent era realment sincron o no */
    if (mess_send->rendez_vous)
    {
      /* Caldra retornar que s'ha trobat un Send corresponent que era SINCRON,
       * per tant, caldra desbloquejar aquest send abans d'arribar al wait. */
      res = 1;
    }
    /* Ja s'havia arribat al send corresponent, per tant, cal treure'l de la
     * cua de sends sense el recv que li correspon */
    extract_from_queue (& (task->send_without_recv), (char *) partner_send);
    /* Cal carregar-se aquest thread perque nomes era una copia per informar
     * que s'havia arribat a aquest send */
    delete_duplicate_thread (partner_send);
  }
  else
  {
    /* S'ha arribat a un recv o Irecv sense que s'hagi arribat al send que li
     * correspon. Per tant, cal encuar una copia d'aquest thread a la cua
     * recv_without_send perque quan s'arribi al send corresponent, aquest
     * pugui saber que ja s'havia arribat aqui. */
    copia_thread = duplicate_thread (thread);
    inFIFO_queue (& (task->recv_without_send), (char *) copia_thread);
  }

  /* Es retorna si s'ha trobat el Send corresponent o no */
  return res;
}


int COMMUNIC_recv_reached_dependency_synchronization (struct t_thread *thread,
                                                      struct t_recv   *mess)
{
  struct t_task   *task, *source_task;
  struct t_thread *partner_send;
  struct t_action *action;
  struct t_send   *mess_send;
  struct t_thread *copia_thread;
  int              res   = 0;

  assert (mess->ori_thread != -1);
  task = thread->task;
  assert (mess->ori == task->taskid);
  for (
    partner_send = (struct t_thread *) head_queue (& (thread->send_without_recv) );
    partner_send != TH_NIL;
    partner_send = (struct t_thread *) next_queue (& (thread->send_without_recv) )
  )
  {
    source_task = partner_send->task;
    action      = partner_send->action;
    mess_send   = & (action->desc.send);
    assert (mess_send->dest            == thread->task->taskid);
    assert (mess_send->dest_thread != -1);
    if ( ( (partner_send->threadid     == mess->ori_thread)     || (mess->ori_thread == -1) )  &&
         ( (partner_send->task->taskid == mess->ori)            || (mess->ori == -1) )         &&
         (mess_send->mess_tag        == mess->mess_tag)                                     &&
         (mess_send->communic_id     == mess->communic_id)                                  &&
         /*    ((mess_send->dest            == thread->task->taskid) || (mess_send->dest == -1))   &&   */
         ( (mess_send->dest_thread     == thread->threadid)     || (mess_send->dest_thread == -1) ) )
    {
      /* Ja s'havia arribat al send corresponent */
      break;
    }
  }

  if (partner_send != TH_NIL)
  {
    /* Cal comprovar si el Send corresponent era realment sincron o no */
    if (mess_send->rendez_vous)
    {
      /* Caldra retornar que s'ha trobat un Send corresponent que era SINCRON,
       * per tant, caldra desbloquejar aquest send abans d'arribar al wait. */
      res = 1;
    }
    /* Ja s'havia arribat al send corresponent, per tant, cal treure'l de la
     * cua de sends sense el recv que li correspon */
    extract_from_queue (& (thread->send_without_recv), (char *) partner_send);
    /* Cal carregar-se aquest thread perque nomes era una copia per informar
     * que s'havia arribat a aquest send */
    delete_duplicate_thread (partner_send);
  }
  else
  {
    /* S'ha arribat a un recv o Irecv sense que s'hagi arribat al send que li
     * correspon. Per tant, cal encuar una copia d'aquest thread a la cua
     * recv_without_send perque quan s'arribi al send corresponent, aquest
     * pugui saber que ja s'havia arribat aqui. */
    copia_thread = duplicate_thread (thread);
    inFIFO_queue (& (thread->recv_without_send), (char *) copia_thread);
  }

  /* Es retorna si s'ha trobat el Send corresponent o no */
  return res;
}


int COMMUNIC_debug_the_senders_list (struct t_thread *thread)
{
  struct t_task   *task;
  struct t_thread *partner_send;
  int              res   = 0;

  task = thread->task;
  panic ("I believe this is never happening, COMMUNIC_debug_the_senders_list\n");
  for (
    partner_send = (struct t_thread *) head_queue (& (task->send) );
    partner_send != TH_NIL;
    partner_send = (struct t_thread *) next_queue (& (task->send) )
  )
  {
    printf ("checking one entry of the sends list in DEBUG_THE_SENDERS_LIST\n");

    register struct t_account *account;


    account = current_account (partner_send);
    printf ("Thread without IDENTITY P%d T%02d t%d is original %d address account first  %p and last %p\n",
            IDENTIFIERS (partner_send), partner_send->original_thread, (partner_send->account).first, (partner_send->account).last);
    struct t_node *node;
    printf ("was is able to locate node in promote_to_original %d for P%d T%d t%d\n",
            account->nodeid, IDENTIFIERS (partner_send) );
#ifdef USE_EQUEUE
    node = (struct t_node *) query_prio_Equeue (&Node_queue,
           (t_priority) account->nodeid);
#else
    node = (struct t_node *) query_prio_queue (&Node_queue,
           (t_priority) account->nodeid);
#endif
    printf ("yes it was able NODE %d  IDENTITY  for P%d T%d t%d\n",
            account->nodeid, IDENTIFIERS (partner_send) );

  }

  printf ("FINISHED DEBUG_THE_SENDERS_LIST\n");
  /* Es retorna si s'ha trobat el Send corresponent o no */
  return res;
}


/***************************************************************
 ** COMMUNIC_send_reached
 ************************
 ** Serveix per indicar que s'ha arribat a un send. Cal
 ** comprovar si s'havia arribat al recv/Irecv corresponent i
 ** actuar en consequencia.
 ** Si es troba que s'ha fet un Irecv corresponent a aquest send,
 ** es retorna 1 per indicar que el send podra comenc,ar
 ** immediatament. En cas contrari es retorna 0.
 ***************************************************************/
int COMMUNIC_send_reached_real_MPI_transfer (struct t_thread *thread, struct t_send *mess)
{
  struct t_task   *task, *dest_task;
  struct t_thread *partner_recv;
  struct t_action *action;
  struct t_recv   *mess_recv;
  struct t_thread *copia_thread;
  int res = 0;

  assert (mess->dest_thread == -1);
  task = thread->task;

  dest_task = locate_task (task->Ptask, mess->dest);
  for (partner_recv = (struct t_thread *) head_queue (& (dest_task->recv_without_send) );
       partner_recv != TH_NIL;
       partner_recv = (struct t_thread *) next_queue (& (dest_task->recv_without_send) ) )
  {
    action = partner_recv->action;
    mess_recv = & (action->desc.recv);

    assert (partner_recv->task->taskid == mess->dest);
    assert (mess->dest_thread == -1);
    if ( ( (partner_recv->threadid     == mess->dest_thread)    || (mess->dest_thread == -1) ) &&
         /*   ((partner_recv->task->taskid == mess->dest)           || (mess->dest == -1))        &&   */
         (mess->mess_tag             == mess_recv->mess_tag)                                &&
         (mess->communic_id          == mess_recv->communic_id)                             &&
         ( (mess_recv->ori           == thread->task->taskid) || (mess_recv->ori == -1) )    &&
         ( (mess_recv->ori_thread    == thread->threadid)     || (mess_recv->ori_thread == -1) ) )
    {
      /* Ja s'havia arribat al recv corresponent */
      break;
    }
  }

  if (partner_recv != TH_NIL)
  {
    if (action->action == IRECV)
    {
      /* Caldra retornar que s'ha trobat un Irecv corresponent i, per tant,
       * aquest thread no s'haura d'esperar encara que sigui sincron i el
       * seu partner no hagi arribat al wait. */
      res = 1;
    }
    /* Ja s'havia arribat al recv/Irecv corresponent, per tant, cal treure'l de la
     * cua de recv sense el send que li correspon */
    extract_from_queue (& (dest_task->recv_without_send), (char *) partner_recv);
    /* Cal carregar-se aquest thread perque nomes era una copia per informar
     * que s'havia arribat a aquest recv/Irecv */
    delete_duplicate_thread (partner_recv);
  }
  else
  {
    /* S'ha arribat a un send sense que s'hagi arribat al recv/Irecv que li
     * correspon. Per tant, cal encuar una copia d'aquest thread a la cua
     * send_without_recv perque quan s'arribi al recv/Irecv corresponent, aquest
     * pugui saber que ja s'havia arribat aqui. */
    copia_thread = duplicate_thread (thread);
    inFIFO_queue (& (dest_task->send_without_recv), (char *) copia_thread);
  }

  /* Es retorna indicant si s'ha trobat un Irecv associat a aquest send o no */
  return res;
}


int COMMUNIC_send_reached_dependency_synchronization (struct t_thread *thread, struct t_send *mess)
{
  struct t_task   *task, *dest_task;
  struct t_thread *partner_recv, *dest_thread;
  struct t_action *action;
  struct t_recv   *mess_recv;
  struct t_thread *copia_thread;
  int res = 0;

  assert (mess->dest_thread != -1);

  task = thread->task;
  dest_task = locate_task (task->Ptask, mess->dest);
  dest_thread = locate_thread_of_task (dest_task, mess->dest_thread);
  for (partner_recv = (struct t_thread *) head_queue (& (dest_thread->recv_without_send) );
       partner_recv != TH_NIL;
       partner_recv = (struct t_thread *) next_queue (& (dest_thread->recv_without_send) ) )
  {
    action = partner_recv->action;
    mess_recv = & (action->desc.recv);
    assert (partner_recv->task->taskid == mess->dest);
    assert (mess->dest_thread != -1);
    if ( ( (partner_recv->threadid     == mess->dest_thread)    || (mess->dest_thread == -1) ) &&
         /*   ((partner_recv->task->taskid == mess->dest)           || (mess->dest == -1))        &&   */
         (mess->mess_tag             == mess_recv->mess_tag)                                &&
         (mess->communic_id          == mess_recv->communic_id)                             &&
         ( (mess_recv->ori             == thread->task->taskid) || (mess_recv->ori == -1) )    &&
         ( (mess_recv->ori_thread      == thread->threadid)     || (mess_recv->ori_thread == -1) ) )
    {
      /* Ja s'havia arribat al recv corresponent */
      break;
    }
  }

  if (partner_recv != TH_NIL)
  {
    if (action->action == IRECV)
    {
      /* Caldra retornar que s'ha trobat un Irecv corresponent i, per tant,
       * aquest thread no s'haura d'esperar encara que sigui sincron i el
       * seu partner no hagi arribat al wait. */
      res = 1;
    }
    /* Ja s'havia arribat al recv/Irecv corresponent, per tant, cal treure'l de la
     * cua de recv sense el send que li correspon */
    extract_from_queue (& (dest_thread->recv_without_send), (char *) partner_recv);
    /* Cal carregar-se aquest thread perque nomes era una copia per informar
     * que s'havia arribat a aquest recv/Irecv */
    delete_duplicate_thread (partner_recv);
  }
  else
  {
    /* S'ha arribat a un send sense que s'hagi arribat al recv/Irecv que li
     * correspon. Per tant, cal encuar una copia d'aquest thread a la cua
     * send_without_recv perque quan s'arribi al recv/Irecv corresponent, aquest
     * pugui saber que ja s'havia arribat aqui. */
    copia_thread = duplicate_thread (thread);
    inFIFO_queue (& (dest_thread->send_without_recv), (char *) copia_thread);
  }

  /* Es retorna indicant si s'ha trobat un Irecv associat a aquest send o no */
  return res;
}

/******************************************************************************
 * PROCEDURE 'COMMUNIC_send'                                                  *
 * ÚLTIMA MODIFICACIÓN: 28/10/2004 (Juan González García)                     *
 ******************************************************************************
 *
 * Inicia el tratamiento de una operación MPI_Send. En primer lugar asegura que
 * se cumple el tiempo de 'startup'. Posteriormente, en función de cada tipo de
 * 'send' (Send o ISend, con o sin Rendezvous), inicia o no la reserva de
 * recursos, llamando a 'really_send'.
 */

void COMMUNIC_send (struct t_thread *thread)
{
  struct t_action  *action;
  struct t_send    *mess;         /* Message */
  struct t_task    *task,         /* Sender task */
      *task_partner; /* Receiver task */
  struct t_thread  *thread_partner;
  int               comm_kind;
  struct t_thread  *copy_thread;
  struct t_thread  *partner;
  struct t_account *account;
  t_nano           startup, copy_latency, roundtriptime;
  dimemas_timer     tmp_timer;
  struct t_node    *node_s,       /* Sender node */
      *node_r;       /* Receiver node */
  int               hi_ha_irecv;  /* Indica si s'ha arribat a un Irecv que
                                   * permet continuar el send encara que sigui
                                   *  sincron */
  int               kind;         /* Communication type */
  struct t_dedicated_connection *connection;

  action = thread->action;
  if (action->action != SEND)
{
    panic ("Calling COMMUNIC_send and action is not Send (%d)\n",
           action->action);
  }

  mess = & (action->desc.send);

  /* DEBUG */
   if (debug&D_COMM)
   {
    PRINT_TIMER (current_time);
    printf (":----calling COMMUNIC_send   P%02d T%02d (t%02d) -> T%02d Tag(%d) Size: %lld  Comm.Id: %d \n",
            IDENTIFIERS (thread),
            mess->dest,
            mess->mess_tag,
            mess->mess_size,
            mess->communic_id);
   }

  task = thread->task;
  task_partner = locate_task (task->Ptask, mess->dest);
  if (task_partner == T_NIL)
  {
    panic ("P%02d T%02d (t%02d) trying to send message to inexistent T%d\n",
           IDENTIFIERS (thread),
           mess->dest);
  }


  if (mess->dest_thread == -1)
    /* real MPI transfer - we look for it at the level of tasks  */
    thread_partner = TH_NIL;
  else
  {
    /* real MPI transfer - we look for it at the level of threads  */
    thread_partner = locate_thread_of_task (task_partner, mess->dest_thread /* thid */);
  }


  node_s = get_node_of_task (task);
  node_r = get_node_of_task (task_partner);

  /* S'obte el tipus de communicació */
  kind = get_communication_type (task,
                                 task_partner,
                                 mess->mess_tag,
                                 mess->mess_size,
                                 &connection);

  if ( kind == INTERNAL_NETWORK_COM_TYPE &&
      external_comm_library_loaded == TRUE)
  {
    /* JGG (28/10/2004): if the communication occurs between nodes and
    the external library is loaded, we check which model is going to be
    used, a regular Dimemas model or an external model defined in the library
    (EXTERNAL_MODEL_COM_TYPE).*/

    kind = external_get_communication_type(node_s->nodeid,
                                           node_r->nodeid,
                                           task->taskid,
                                           task_partner->taskid,
                                           mess->mess_tag,
                                           mess->mess_size);
  }

  mess->comm_type = kind;

   /**************************************************************************************
  *
  *                     Karthikeyan: EEE CODE for 3 Level Network
  *
  ***************************************************************************************/

  if(eee_enabled){
    if(EEE_DEBUG)
    {
      PRINT_TIMER(current_time);
      printf("::Message at COMMUNIC_send function--\n");
    }
    //TODO REMOVE THIS PRINTF!
    //printf("------------------MESSINFLIGHT:T:%d,N_S:%d,N_R:%d\n"
    //                                                      ,thread->messages_in_flight
    //                                                      ,node_s->messages_in_flight
    //                                                      ,node_r->messages_in_flight);
    if (thread->eee_done_reset_var == TRUE) {
        if(EEE_DEBUG) {
            PRINT_TIMER(current_time);
            printf("::EEE_SEND Finished! Resetting Variables!");
        }
        thread->doing_startup = FALSE;
        thread->startup_done = FALSE;
        thread->loose_cpu = FALSE;
        thread->link_transmit_done = FALSE;
        thread->nw_switch_done = TRUE;
        thread->eee_done_reset_var = FALSE;
    }

    if (thread->eee_send_done == FALSE) {

        if(EEE_DEBUG) {PRINT_TIMER(current_time); printf("::At EEE Network Code\n");}

        t_nano eee_nw_delay;
        eee_nw_delay = eee_network(thread);

        if(EEE_DEBUG) {
            PRINT_TIMER(current_time);
            printf("::Delay Returned:%f\n",(double)eee_nw_delay);
        }

        if (thread->eee_send_done == TRUE) {
            if(EEE_DEBUG) {
                PRINT_TIMER(current_time);
                printf("::Last Transmit - Activing reset variables after send!\n\n");
            }
            thread->eee_done_reset_var = TRUE;
            if(EEE_DEBUG) {
                PRINT_TIMER(current_time);
                printf("::Event Added to Scheduler with delay::%f\n"
                                              ,(double)eee_nw_delay);
            }
            SCHEDULER_thread_to_ready_return (M_COM, thread, eee_nw_delay, 0);
            return;

        } else {

            thread->doing_startup = TRUE;
            thread->startup_done = FALSE;
            thread->loose_cpu = FALSE;
            if(EEE_DEBUG) {
                PRINT_TIMER(current_time);
                printf("::Event Added to Scheduler with delay::%f\n"
                                              ,(double)eee_nw_delay);
            }
            SCHEDULER_thread_to_ready_return (M_COM, thread, eee_nw_delay, 0);
            return;
        }

        if(EEE_DEBUG) {
            PRINT_TIMER(current_time);
            printf("::Leaving EEE Network Code\n");
        }
    }
  }
  /**************************************************************************************
  *                     Karthikeyan: End of EEE code
  ***************************************************************************************/

  /* Compute startup duration and re-schedule thread if needed */
  if (thread->startup_done == FALSE)
  {
    // startup = compute_startup (thread, kind, node_s, connection);
    startup = compute_startup (thread,
                               thread->task->taskid,  /* Sender */
                               task_partner->taskid,  /* Receiver */
                               node_s,
                               node_r,
                               mess->mess_tag,
                               mess->mess_size,
                               kind,
                               connection);

    if (startup != (t_nano) 0)
    {
      thread->logical_send = current_time;

      thread->loose_cpu     = FALSE;
      thread->doing_startup = TRUE;

      account = current_account (thread);
      FLOAT_TO_TIMER (startup, tmp_timer);
      ADD_TIMER (account->latency_time, tmp_timer, account->latency_time);
      SUB_TIMER (account->cpu_time, tmp_timer, account->cpu_time);

      SCHEDULER_thread_to_ready_return (M_COM, thread, startup, 0);

      if (debug & D_COMM)
      {
        PRINT_TIMER (current_time);
        printf (": COMMUNIC_send\tP%02d T%02d (t%02d) Initiate startup (%f)\n",
                IDENTIFIERS (thread),
                (double) startup / 1e9);
      }
      return;
    }
    else /* (startup == (t_nano) 0) */
    {
      thread->startup_done = TRUE;
      thread->logical_send = current_time;
    }
  } /* thread->startup_done == TRUE */

  /* Copy latency operations */
  if (DATA_COPY_enabled && mess->mess_size <= DATA_COPY_message_size)
  {
    if (thread->copy_done == FALSE)
    {
      copy_latency = compute_copy_latency (thread, node_s, mess->mess_size);

      if (copy_latency != (t_nano) 0)
      {
        thread->loose_cpu     = FALSE;
        thread->doing_copy    = TRUE;

        account = current_account (thread);
        FLOAT_TO_TIMER (copy_latency, tmp_timer);
        ADD_TIMER (account->latency_time, tmp_timer, account->latency_time);
        SUB_TIMER (account->cpu_time, tmp_timer, account->cpu_time);

        SCHEDULER_thread_to_ready_return (M_COM, thread, copy_latency, 0);

        if (debug & D_COMM)
        {
          PRINT_TIMER (current_time);
          printf (
            ": COMMUNIC_send\tP%02d T%02d (t%02d) Initiate copy latency (%f)\n",
            IDENTIFIERS (thread),
            (double) copy_latency / 1e9);
        }
        return;
      }
      else
      {
        thread->copy_done = TRUE;
      }
    }
  }

  /* Round Trip Time for sends */
  if (RTT_enabled && mess->rendez_vous && (kind == INTERNAL_NETWORK_COM_TYPE) )
  {
    if (thread->roundtrip_done == FALSE)
    {
      roundtriptime = RTT_time / 2.0;

      if (RTT_time != (t_nano) 0)
      {
        thread->loose_cpu       = FALSE;
        thread->doing_roundtrip = TRUE;

        account = current_account (thread);
        FLOAT_TO_TIMER (roundtriptime, tmp_timer);
        ADD_TIMER (account->latency_time, tmp_timer, account->latency_time);
        SUB_TIMER (account->cpu_time, tmp_timer, account->cpu_time);

        SCHEDULER_thread_to_ready_return (M_COM, thread, roundtriptime, 0);

        if (debug & D_COMM)
        {
          PRINT_TIMER (current_time);
          printf (
            ": COMMUNIC_send\tP%02d T%02d (t%02d) Initiate round trip time(%f)\n",
            IDENTIFIERS (thread),
            (double) roundtriptime / 1e9);
        }
        return;
      }
      else
      {
        thread->roundtrip_done = TRUE;
      }
    }
  }
  /* Startup, Copy and RTT checks reset */
  thread->startup_done   = FALSE;
  thread->copy_done      = FALSE;
  thread->roundtrip_done = FALSE;
  // Karthikeyan EEE Code - Resetting variables
  thread->eee_send_done = FALSE;
  // Karthikeyan EEE Code END


  account = current_account (thread);
  account->n_sends++;

  account->n_bytes_send += mess->mess_size;

  if (debug & D_COMM)
  {
    PRINT_TIMER (current_time);
    printf (
      ": COMMUNIC_send\tP%02d T%02d (t%02d) -> T%02d t%d Tag: %d Comm.Id: %d Size: %lldb\n",
      IDENTIFIERS (thread),
      action->desc.send.dest,
      action->desc.send.dest_thread,
      mess->mess_tag,
      mess->communic_id,
      action->desc.send.mess_size
    );
  }

  /* FEC: S'avisa que s'ha arribat a aquest send i es comprova si s'ha arribat
   * a un Irecv que permeti continuar a aquest send encara que sigui sincron. */

  if (mess->dest_thread == -1)
  {
    /* this is a real MPI transfer */
    hi_ha_irecv = COMMUNIC_send_reached_real_MPI_transfer (thread, mess);
  }
  else
  {
    /* this is a dependency synchronization */
    hi_ha_irecv = COMMUNIC_send_reached_dependency_synchronization (thread, mess);
  }

#ifdef VENUS_ENABLED
  if (VC_is_enabled() && (kind == INTERNAL_NETWORK_COM_TYPE) ) {
    double dtime;
    TIMER_TO_FLOAT (current_time, dtime);
    if (mess->rendez_vous) {
      VC_command_rdvz_send (dtime, node_s->nodeid, node_r->nodeid, mess->mess_tag, mess->mess_size, task->Ptask->Ptaskid, task_partner->Ptask->Ptaskid);
    }
  }
#endif


  if (mess->dest_thread == -1) {
    /* real MPI transfer
       look for it at task->recv   */
    partner =
      locate_receiver_real_MPI_transfer (
        & (task_partner->recv),
        task->taskid,
        thread->threadid,
        mess->dest_thread,
        mess->mess_tag,
        mess->communic_id
      );
  } else  {
    /* real MPI transfer
       look for it at thread->recv   */
    assert (thread_partner != TH_NIL);
    partner =
      locate_receiver_dependencies_synchronization (
        & (thread_partner->recv),
        task->taskid,
        thread->threadid,
        mess->dest_thread,
        mess->mess_tag,
        mess->communic_id
      );
  }


//TODO - think about this - what is what?????
//   if (partner != TH_NIL)
//      assert(hi_ha_irecv);


  if (partner != TH_NIL)
  {
    SCHEDULER_info (COMMUNICATION_INFO, SCH_INFO_SEND, thread, TH_NIL);
  }

  comm_kind = (mess->immediate << 1) + mess->rendez_vous;

  // printf("Communication kind = %d\n", comm_kind);

  switch (comm_kind)
  {
    /* Con RD estamos en un Send */
  case RD_SYNC:
  case RD_ASYNC:
  {
    if ( (!hi_ha_irecv) && (partner == TH_NIL) )
    {
      if (mess->immediate)
      {
        /* El Rendez vous s'ha de fer en "background". Cal utilitzar
         * una copia del thread */
        copy_thread = duplicate_thread (thread);
        if (mess->dest_thread == -1) {
          /* this is for a real MPI transfer */
          inFIFO_queue (& (task->send), (char *) copy_thread);
        } else {
          /* this is for a dependency synchronization */
          inFIFO_queue (& (thread->send), (char *) copy_thread);
        }

        /* El thread original pot continuar */
        action = thread->action;
        thread->action = action->next;
        READ_free_action(action);
        if (more_actions (thread) )
        {
          thread->loose_cpu = FALSE;
          SCHEDULER_thread_to_ready (thread);
        }
      } /* Send, con Rendezvous */
      else
      {
        /* El thread s'ha de bloquejar per esperar el Irecv/recv */
        if (mess->dest_thread == -1) {
          /* this is for a real MPI transfer */
          inFIFO_queue (& (task->send), (char *) thread);
        } else {
          /* this is for a dependency synchronization */
          inFIFO_queue (& (thread->send), (char *) thread);
        }
      }
    }
    else /* hi_ha_irecv || partner != TH_NIL */
    {
      copy_thread = duplicate_thread (thread);
      ASS_ALL_TIMER (copy_thread->initial_communication_time, current_time);
      copy_thread->last_paraver = current_time;

      /* !!! */
      really_send (copy_thread);

      action = thread->action;
      thread->action = action->next;
      READ_free_action(action);
      if (more_actions (thread) )
      {
        thread->loose_cpu = FALSE;
        SCHEDULER_thread_to_ready (thread);
      }
    }
    break;
  }
  case NORD_SYNC:
    /* De momento solo el tipo  de comunicacion NORD_ASYNC */
  case NORD_ASYNC:
  {
    copy_thread = duplicate_thread (thread);
    ASS_ALL_TIMER (copy_thread->initial_communication_time, current_time);
    copy_thread->last_paraver = current_time;
    /* !!!! */
    really_send (copy_thread);

    action = thread->action;
    thread->action = action->next;
    READ_free_action(action);
    if (more_actions (thread) )
    {
      thread->loose_cpu = FALSE;
      SCHEDULER_thread_to_ready (thread);
    }
    break;
  }
  default:
    panic ("Impossible send type %d\n", comm_kind);
    break;
  } /* switch */
}

/******************************************************************************
 * PROCEDURE 'COMMUNIC_recv'                                                  *
 * ÚLTIMA MODIFICACIÓN: 29/10/2004 (Juan González García)                     *
 *****************************************************************************/

void COMMUNIC_recv (struct t_thread *thread)
{
  struct t_action  *action;
  struct t_recv    *mess;
  struct t_task    *task, *task_source;
  struct t_account *account;
  dimemas_timer     tmp_timer;
  struct t_node    *node_s, *node_r;
  t_nano           startup, copy_latency;
  int               kind;
  struct t_dedicated_connection *connection;


  action = thread->action;
  if (action->action != RECV)
  {
    panic ("Calling COMMUNIC_recv and action is not receive (%d)\n",
           action->action);
  }

  mess = & (action->desc.recv);


  /* DEBUG */
  if (debug & D_COMM)
  {
    PRINT_TIMER (current_time);
    printf (
      ":----calling COMMUNIC_recv   P%02d T%02d (t%02d) <- T%02d (t%02d) Tag: %d Comm.Id: %d size: %lld\n",
      IDENTIFIERS (thread),
      action->desc.recv.ori,
      action->desc.recv.ori_thread,
      mess->mess_tag,
      mess->communic_id,
      action->desc.recv.mess_size);
  }

  task = thread->task;

  node_r = get_node_of_thread (thread);
  node_s = get_node_for_task_by_name (thread->task->Ptask, mess->ori);

  task_source = locate_task (task->Ptask, mess->ori);

  /* S'obte el tipus de communicació */
  kind = get_communication_type (task_source,
                                 task,
                                 mess->mess_tag,
                                 mess->mess_size,
                                 &connection);

  if (kind == INTERNAL_NETWORK_COM_TYPE &&
      external_comm_library_loaded == TRUE)
  {
    /* JGG (28/10/2004): if the communication occurs between nodes and
    the external library is loaded, we check which model is going to be
    used, a regular Dimemas model or an external model defined in the library
    (EXTERNAL_MODEL_COM_TYPE).*/
    kind = external_get_communication_type(node_s->nodeid,
                                           node_r->nodeid,
                                           mess->ori,
                                           task->taskid,
                                           mess->mess_tag,
                                           mess->mess_size);
  }

  mess->comm_type = kind;

  if (thread->startup_done == FALSE)
  { /* Compute startup duration and re-schedule thread if needed */
    // startup = compute_startup (thread, kind, node_s, connection);
    startup = compute_startup (thread,
                               mess->ori,            /* Sender */
                               thread->task->taskid, /* Receiver */
                               node_s,
                               node_r,
                               mess->mess_tag,
                               mess->mess_size,
                               kind,
                               connection);


    if (startup > (t_nano) 0)
    {

      thread->logical_recv = current_time;

      thread->loose_cpu     = FALSE;
      thread->doing_startup = TRUE;

      account = current_account (thread);
      FLOAT_TO_TIMER (startup, tmp_timer);
      ADD_TIMER (account->latency_time, tmp_timer, account->latency_time);
      SUB_TIMER (account->cpu_time, tmp_timer, account->cpu_time);

      SCHEDULER_thread_to_ready_return (M_COM, thread, startup, 0);

      if (debug & D_COMM)
      {
        PRINT_TIMER (current_time);
        printf (": COMMUNIC_recv\tP%02d T%02d (t%02d) Initiate startup (%f)\n",
                IDENTIFIERS (thread),
                (double) startup / 1e9);
      }

      return;
    }
    else if (startup == (t_nano) 0)
    {
      thread->logical_recv = current_time;
      thread->startup_done = TRUE;
    }
  }

  /* Startup has finished */

  /* Copy latency operations */
  if (DATA_COPY_enabled && mess->mess_size <= DATA_COPY_message_size)
  {
    if (thread->copy_done == FALSE)
    {
      copy_latency = compute_copy_latency (thread, node_s, mess->mess_size);

      if (copy_latency != (t_nano) 0)
      {
        thread->loose_cpu     = FALSE;
        thread->doing_copy    = TRUE;

        account = current_account (thread);
        FLOAT_TO_TIMER (copy_latency, tmp_timer);
        ADD_TIMER (account->latency_time, tmp_timer, account->latency_time);
        SUB_TIMER (account->cpu_time, tmp_timer, account->cpu_time);

        SCHEDULER_thread_to_ready_return (M_COM, thread, copy_latency, 0);

        if (debug & D_COMM)
        {
          PRINT_TIMER (current_time);
          printf (
            ": COMMUNIC_recv\tP%02d T%02d (t%02d) Initiate copy latency (%f)\n",
            IDENTIFIERS (thread),
            (double) copy_latency / 1e9);
        }
        return;
      }
      else
      {
        thread->copy_done = TRUE;
      }
    }
  }

  /* Startup and Copy checks reset */
  thread->startup_done = FALSE;
  thread->copy_done    = FALSE;

  account = current_account (thread);
  account->n_recvs++;


  /* FEC: S'avisa que s'ha arribat a aquest recv */
  /* JGG: No se tiene en cuenta el tipo de send ?¿?¿ */
  if (mess->ori_thread == -1) {
    /* this is a real MPI transfer */
    COMMUNIC_recv_reached_real_MPI_transfer (thread, mess);
  } else {
    /* this is a dependency synchronization */
    COMMUNIC_recv_reached_dependency_synchronization (thread, mess);
  }

  t_boolean Is_message_awaiting;
  if (mess->ori_thread == -1)
    Is_message_awaiting = is_message_awaiting_real_MPI_transfer (task, mess, thread);
  else
    Is_message_awaiting = is_message_awaiting_dependency_synchronization (task, mess, thread);
  if (Is_message_awaiting)                      /* 'is_message_awaiting'      */
  { /* desencola a los que esperan*/
    account->n_recvs_on_processor++;
    if (debug & D_COMM)
    {
      PRINT_TIMER (current_time);
      printf (
        ": COMMUNIC_recv_reached\tP%02d T%02d (t%02d) <- T%02d t%d Tag: %d Comm.Id: %d size: %lld (Local Message)\n",
        IDENTIFIERS (thread),
        action->desc.recv.ori,
        action->desc.recv.ori_thread,
        mess->mess_tag,
        mess->communic_id,
        action->desc.recv.mess_size);
    }

    thread->action = action->next;
    READ_free_action(action);
    if (more_actions (thread) )
    {
      thread->loose_cpu = FALSE;
      SCHEDULER_thread_to_ready (thread);
    }
  }
  else /* !is_message_awaiting(...) */
  {
    if (mess->ori_thread == -1) {
      /* this is for a real MPI transfer */
      Start_communication_if_partner_ready_for_rendez_vous_real_MPI_transfer (thread, mess);
    } else {
      /* this is for a dependency synchronization */
      Start_communication_if_partner_ready_for_rendez_vous_dependency_synchronization (thread, mess);
    }

    if (node_r->machine->scheduler.busywait_before_block)
    {
      SCHEDULER_thread_to_busy_wait (thread);
    }
    else
    {
      account->n_recvs_must_wait++;
      thread->start_wait_for_message = current_time;
      if (debug & D_COMM)
      {
        PRINT_TIMER (current_time);
        printf (
          ": COMMUNIC_recv\tP%02d T%02d (t%02d) <- T%02d t%d Tag: %d Comm.Id: %d  Size1: %lld  Size2: %lld (Waiting)\n",
          IDENTIFIERS (thread),
          action->desc.recv.ori,
          action->desc.recv.ori_thread,
          mess->mess_tag,
          mess->communic_id,
          action->desc.recv.mess_size,
          mess->mess_size
        );
      }

      if (mess->ori_thread == -1) {
        /* this is a regular MPI transfer
           put it on the recv list of the task */
        inFIFO_queue (& (task->recv), (char *) thread);
      }
      else {
        /* this is a dependency synchronization
           put it on the recv list of the thread  */
        inFIFO_queue (& (thread->recv), (char *) thread);
      }
    }
  }
}
/******************************************************************************
 * PROCEDURE 'COMMUNIC_Irecv'                                                 *
 * ÚLTIMA MODIFICACIÓN: 02/11/2004 (Juan González García)                     *
 ******************************************************************************
 *
 * Es el tractament que cal aplicar a un Irecv. Consisteix en aplicar el startup
 * i despres mirar si hi ha algun send bloqejat esperant aquest Irecv. Si es
 * aixi es desbloqueja. Tant en un cas com en l'altre cal informar que s'ha
 * arribat a un Irecv.
 */

void COMMUNIC_Irecv (struct t_thread *thread)
{
  struct t_action  *action;
  struct t_recv    *mess;
  struct t_task    *task;
  struct t_recv    *irecv_notification;
#ifdef STARTUP_ALS_IRECV
  struct t_task    *task_source;
  struct t_account *account;
  dimemas_timer     tmp_timer;
  struct t_node    *node, *node_s, *node_r;
  t_nano           startup;
  int               kind;
  struct t_dedicated_connection *connection;
#endif
  int               hi_ha_send_sync; /* Indica si s'ha arribat a un Send SINCRON
                                        que caldra desbloqejar. */
  action = thread->action;
  if (action->action != IRECV)
  {
    panic (
      "Calling COMMUNIC_Irecv and action is not Ireceive (%d)\n",
      action->action
    );
  }

  mess = & (action->desc.recv);

  /* DEBUG */
  if (debug & D_COMM)
  {
    PRINT_TIMER (current_time);
    printf (":-----calling COMMUNIC_Irecv   P%02d T%02d (t%02d)  <- T%02d (t%02d)  Tag: %d  Comm.Id: %d  Size: %lld \n",
            IDENTIFIERS (thread),
            action->desc.recv.ori,
            action->desc.recv.ori_thread,
            mess->mess_tag,
            mess->communic_id,
            action->desc.recv.mess_size);
  }

#ifdef STARTUP_ALS_IRECV /*****************************************************/

  task   = thread->task;
  node_r = get_node_of_thread (thread);
  node_s = get_node_for_task_by_name (thread->task->Ptask, mess->ori);

  task_source = locate_task (task->Ptask, mess->ori);
  /* S'obte el tipus de communicació */
  kind = get_communication_type (task_source,
                                 task,
                                 mess->mess_tag,
                                 mess->mess_size,
                                 &connection);

  if (kind == EXTERNAL_NETWORK_COM_TYPE &&
      external_comm_library_loaded == TRUE)
  {
    /* JGG (28/10/2004): if the communication occurs between nodes and
    the external library is loaded, we check which model is going to be
    used, a regular Dimemas model or an external model defined in the library
    (EXTERNAL_MODEL_COM_TYPE).*/
    kind = external_get_communication_type(node_s->nodeid,
                                           node_r->nodeid,
                                           mess->ori,
                                           task->taskid,
                                           mess->mess_tag,
                                           mess->mess_size);
  }

  mess->comm_type = kind;

  if (thread->startup_done == FALSE)
  { /* Compute startup duration and re-schedule thread if needed */
    // startup = compute_startup (thread, kind, node_s, connection);

    startup = compute_startup (thread,
                               mess->ori,            /* Sender */
                               thread->task->taskid, /* Receiver */
                               node_s,
                               node_r,
                               mess->mess_tag,
                               mess->mess_size,
                               kind,
                               connection);

    if (startup > (t_nano) 0) /* Change != with > */
    { /* Positive startup time. Thread must be re-scheduled */
      if (!wait_logical_recv)
      {
        thread->logical_recv = current_time;
      }

      thread->loose_cpu     = FALSE;
      thread->doing_startup = TRUE;

      account = current_account (thread);
      FLOAT_TO_TIMER (startup, tmp_timer);
      ADD_TIMER (account->latency_time, tmp_timer, account->latency_time);
      SUB_TIMER (account->cpu_time, tmp_timer, account->cpu_time);

      SCHEDULER_thread_to_ready_return (M_COM, thread, startup, 0);

      if (debug & D_COMM)
      {
        PRINT_TIMER (current_time);
        printf (": COMMUNIC_Irecv\tP%02d T%02d (t%02d) Initiate startup (%f)\n",
                IDENTIFIERS (thread),
                (double) startup / 1e9);
      }

      return;
    }
    else if (startup == (t_nano) 0)
    {
      if (!wait_logical_recv)
      {
        thread->logical_recv = current_time;
      }
      thread->startup_done = TRUE;
    }
  }


  /* Copy latency operations */
  if (DATA_COPY_enabled && mess->mess_size <= DATA_COPY_message_size)
  {
    if (thread->copy_done == FALSE)
    {
      copy_latency = compute_copy_latency (thread, node_s, mess->mess_size);

      if (copy_latency != (t_nano) 0)
      {
        thread->loose_cpu     = FALSE;
        thread->doing_copy    = TRUE;

        account = current_account (thread);
        FLOAT_TO_TIMER (copy_latency, tmp_timer);
        ADD_TIMER (account->latency_time, tmp_timer, account->latency_time);
        SUB_TIMER (account->cpu_time, tmp_timer, account->cpu_time);

        SCHEDULER_thread_to_ready_return (M_COM, thread, copy_latency, 0);

        if (debug & D_COMM)
        {
          PRINT_TIMER (current_time);
          printf (
            ": COMMUNIC_Irecv\tP%02d T%02d (t%02d) Initiate copy latency (%f)\n",
            IDENTIFIERS (thread),
            (double) copy_latency / 1e9,
            mess->mess_size);
        }
        return;
      }
      else
      {
        thread->copy_done = TRUE;
      }
    }
  }


  thread->startup_done = FALSE;
  thread->copy_done    = FALSE;

  account = current_account (thread);
#endif /* ! STARTUP_ALS_IRECV */

// #else

/*
  if (!wait_logical_recv)
  {
    thread->logical_recv = current_time;
  }
*/


// #endif /* STARTUP_ALS_IRECV ***************************************************/

  if (!wait_logical_recv)
  {
    irecv_notification = (struct t_recv*) malloc(sizeof(struct t_recv));
    memcpy(irecv_notification, mess, sizeof(struct t_recv));
    irecv_notification->logical_recv = current_time;

    if (debug & D_COMM)
    {
      PRINT_TIMER (current_time);
      printf (
        ": COMMUNIC_Irecv\tP%02d T%02d (t%02d) IRecv notification Comm.Id: %d saved\n",
        IDENTIFIERS (thread),
        mess->communic_id);
    }
  }

  /* FEC: S'avisa que s'ha arribat a aquest Irecv i s'obte si
   * cal desbloqejar un send o no. */
  if (mess->ori_thread == -1)
  {
    /* this is a real MPI transfer */
    hi_ha_send_sync = COMMUNIC_recv_reached_real_MPI_transfer (thread, mess);

    if (!wait_logical_recv)
    {
      inFIFO_queue(&(thread->task->irecvs_executed), (char*) irecv_notification);
    }
  }
  else
  {
    /* this is a dependency synchronization */
    hi_ha_send_sync = COMMUNIC_recv_reached_dependency_synchronization (thread, mess);

    if (!wait_logical_recv)
    {
      inFIFO_queue(&(thread->irecvs_executed), (char*) irecv_notification);
    }
  }

  if (hi_ha_send_sync)
  {
    /* Per poder executar aixo cal haver comprovat que el send que correspon
     * a aquest Irecv es realment sincron, perque si no es aixi, pot ser que
     * el Irecv ja hagi rebut el missatge a traves d'un send asincron i,
     * per tant, estariem desbloquejant el send corresponent a algun Irecv
     * futur. */
    if (mess->ori_thread == -1)
    {
      /* this is for a real MPI transfer */
      Start_communication_if_partner_ready_for_rendez_vous_real_MPI_transfer (thread, mess);
    }
    else
    {
      /* this is for a dependency synchronization */
      Start_communication_if_partner_ready_for_rendez_vous_dependency_synchronization (thread, mess);
    }
  }

  thread->action = action->next;
  READ_free_action(action);
  if (more_actions (thread) )
  {
    thread->loose_cpu = FALSE;
    SCHEDULER_thread_to_ready (thread);
  }
}


/******************************************************************************
 * PROCEDURE 'COMMUNIC_wait'                                                  *
 * ÚLTIMA MODIFICACIÓN: 04/11/2004 (Juan González García)                     *
 ******************************************************************************
 *
 * Es el tractament que cal aplicar a un Wait. Consisteix en
 * esperar fins que s'hagi rebut el missatge.
 */
void COMMUNIC_wait (struct t_thread *thread)
{
  struct t_action               *action;
  struct t_recv                 *mess;
  struct t_task                 *task, *task_source;
  struct t_account              *account;
  dimemas_timer                  tmp_timer;
  struct t_node                 *node_r, *node_s;
  t_nano                        startup, copy_latency;
  int                            kind;
  struct t_dedicated_connection *connection;


  action = thread->action;
  if (action->action != WAIT)
  {
    panic ("Calling COMMUNIC_wait and action is not Wait (%d)\n",
           action->action);
  }
  mess    = & (action->desc.recv);


  /* DEBUG */
  if (debug & D_COMM)
  {
    PRINT_TIMER (current_time);
    printf (
      ":----calling COMMUNIC_wait   P%02d T%02d (t%02d) <- T%02d t%d Tag: %d Comm.Id: %d size: %lld\n",
      IDENTIFIERS (thread),
      action->desc.recv.ori,
      action->desc.recv.ori_thread,
      mess->mess_tag,
      mess->communic_id,
      action->desc.recv.mess_size);
  }

  task   = thread->task;
  node_r = get_node_of_thread (thread);
  node_s = get_node_for_task_by_name (thread->task->Ptask, mess->ori);

  task_source = locate_task (task->Ptask, mess->ori);
  /* S'obte el tipus de communicació */
  kind = get_communication_type (task_source,
                                 task,
                                 mess->mess_tag,
                                 mess->mess_size,
                                 &connection);

  if (kind == EXTERNAL_NETWORK_COM_TYPE &&
      external_comm_library_loaded == TRUE)
  {
    /* JGG (28/10/2004): if the communication occurs between nodes and
    the external library is loaded, we check which model is going to be
    used, a regular Dimemas model or an external model defined in the library
    (EXTERNAL_MODEL_COM_TYPE).*/
    kind = external_get_communication_type(node_s->nodeid,
                                           node_r->nodeid,
                                           mess->ori,
                                           task->taskid,
                                           mess->mess_tag,
                                           mess->mess_size);
  }

  mess->comm_type = kind;

  if (thread->startup_done == FALSE)
  {
    // startup = compute_startup (thread, kind, node_s, connection);
    startup = compute_startup (thread,
                               mess->ori,            /* Sender */
                               thread->task->taskid, /* Receiver */
                               node_s,
                               node_r,
                               mess->mess_tag,
                               mess->mess_size,
                               kind,
                               connection);

    if (startup != (t_nano) 0)
    {
      if (debug & D_COMM)
      {
        PRINT_TIMER (current_time);
        printf (": COMMUNIC_wait\tP%02d T%02d (t%02d) Initiate startup (%f)\n",
                IDENTIFIERS (thread),
                (double) startup / 1e9);
      }
      thread->loose_cpu     = FALSE;
      thread->doing_startup = TRUE;

      account = current_account (thread);
      FLOAT_TO_TIMER (startup, tmp_timer);
      ADD_TIMER (account->latency_time, tmp_timer, account->latency_time);
      SUB_TIMER (account->cpu_time, tmp_timer, account->cpu_time);

      SCHEDULER_thread_to_ready_return (M_COM, thread, startup, 0);
      return;
    }
    else
    {
      thread->startup_done = TRUE;
    }
  }
  else
  {
  }

  /* Copy latency operations */
  if (DATA_COPY_enabled && mess->mess_size <= DATA_COPY_message_size)
  {
    if (thread->copy_done == FALSE)
    {
      copy_latency = compute_copy_latency (thread, node_s, mess->mess_size);

      if (copy_latency != (t_nano) 0)
      {
        thread->loose_cpu     = FALSE;
        thread->doing_copy    = TRUE;

        account = current_account (thread);
        FLOAT_TO_TIMER (copy_latency, tmp_timer);
        ADD_TIMER (account->latency_time, tmp_timer, account->latency_time);
        SUB_TIMER (account->cpu_time, tmp_timer, account->cpu_time);

        SCHEDULER_thread_to_ready_return (M_COM, thread, copy_latency, 0);

        if (debug & D_COMM)
        {
          PRINT_TIMER (current_time);
          printf (": COMMUNIC_wait\tP%02d T%02d (t%02d) Initiate copy latency (%f)\n",
                  IDENTIFIERS (thread),
                  (double) copy_latency / 1e9);
        }
        return;
      }
    }
  }

  /* Startup and Copy checks reset */
  thread->startup_done = FALSE;
  thread->copy_done    = FALSE;

  account = current_account (thread);
  account->n_recvs++;
  account = current_account (thread);

  if (wait_logical_recv)
  {
    thread->logical_recv = current_time;
  }

  t_boolean Is_message_awaiting;

  if (mess->ori_thread == -1)
  {
    Is_message_awaiting = is_message_awaiting_real_MPI_transfer (task, mess, thread);
  }
  else
  {
    Is_message_awaiting = is_message_awaiting_dependency_synchronization (task, mess, thread);
  }

  if (Is_message_awaiting)                      /* 'is_message_awaiting'      */
  {
    /* El mensaje ya se ha recibido. 'is_message_awaiting' ya se encarga de
     * desencolar los elementos en consecuencia, por lo que el 'wait' ya se
     * considera finalizado */

    account->n_recvs_on_processor++;
    if (debug & D_COMM)
    {
      PRINT_TIMER (current_time);
      printf (
        ": COMMUNIC_wait/recv\tP%02d T%02d (t%02d) <- T%02d t%d Tag: %d Comm.Id: %d (Local message)\n",
        IDENTIFIERS (thread),
        action->desc.recv.ori,
        action->desc.recv.ori_thread,
        mess->mess_tag,
        mess->communic_id
      );
    }

    thread->action = action->next;
    READ_free_action(action);
    if (more_actions (thread) )
    {
      thread->loose_cpu = FALSE;
      SCHEDULER_thread_to_ready (thread);
    }
  }
  else
  {
    /* Start_communication_if_partner_ready_for_rendez_vous assumes
     * thread is receiver, so locate receiver task */
    if (mess->ori_thread == -1)
    {
      /* this is for a real MPI transfer */
      Start_communication_if_partner_ready_for_rendez_vous_real_MPI_transfer (thread, mess);
    }
    else
    {
      /* this is for a dependency synchronization */
      Start_communication_if_partner_ready_for_rendez_vous_dependency_synchronization (thread, mess);
    }

    if (node_r->machine->scheduler.busywait_before_block)
    {
      SCHEDULER_thread_to_busy_wait (thread);
    }
    else
    {
      account->n_recvs_must_wait++;
      thread->start_wait_for_message = current_time;
      if (debug & D_COMM)
      {
        PRINT_TIMER (current_time);
        printf (
          ": COMMUNIC_wait/recv\tP%02d T%02d (t%02d) <- T%02d t%d Tag: %d Comm.Id: %d (Waiting)\n",
          IDENTIFIERS (thread),
          action->desc.recv.ori,
          action->desc.recv.ori_thread,
          mess->mess_tag,
          mess->communic_id
        );
      }

      if (mess->ori_thread == -1) {
        /* this is a regular MPI transfer
           put it on the recv list of the task */
        inFIFO_queue (& (task->recv), (char *) thread);
      }
      else {
        /* this is a dependency synchronization
           put it on the recv list of the thread  */
        inFIFO_queue (& (thread->recv), (char *) thread);
      }
    }
  }
}

void COMMUNIC_block_after_busy_wait (struct t_thread *thread)
{
  struct t_task    *task;
  struct t_account *account;
  struct t_action  *action;
  struct t_recv    *mess;

  task = thread->task;
  action = thread->action;
  mess = & (action->desc.recv);
  account = current_account (thread);
  account->n_recvs_must_wait++;
  thread->start_wait_for_message = current_time;

  if (debug & D_COMM)
  {
    PRINT_TIMER (current_time);
    printf (
      ": COMMUNIC_recv/wait P%02d T%02d (t%02d) <- T%02d Tag(%d) Block after Busy Wait\n",
      IDENTIFIERS (thread),
      mess->ori,
      mess->mess_tag
    );
  }

  if (mess->ori_thread == -1) {
    /* this is a regular MPI transfer
       put it on the recv list of the task */
    inFIFO_queue (& (task->recv), (char *) thread);
  }
  else {
    /* this is a dependency synchronization
       put it on the recv list of the thread  */
    inFIFO_queue (& (thread->recv), (char *) thread);
  }
}

int get_global_op_id_by_name (char *name)
{
  struct t_global_op_definition * glop;

  for (glop = (struct t_global_op_definition *) head_queue (&Global_op);
       glop != (struct t_global_op_definition *) 0;
       glop = (struct t_global_op_definition *) next_queue (&Global_op) )
  {
    if (strcmp (glop->name, name) == 0)
      return (glop->identificator);
  }
  return (-1);
}

void add_global_ops (void)
{
  int i;

  for (i = 0; i < GLOBAL_OPS_COUNT; i++)
  {
    new_global_op (i, Global_Ops_Labels[i]);
  }
}

static void inicialitza_info_nova_globalop (int                            model,
                                            struct t_global_op_definition *glop,
                                            struct t_queue                *cua)
{
  struct t_global_op_information *glop_info;

  switch (model) /* Aixo és absurd */
  {
    case GOP_MODEL_0:
    case GOP_MODEL_CTE:
    case GOP_MODEL_LIN:
    case GOP_MODEL_LOG:
      break;
    default:
      panic ("Unexpected Global operation model\n");
      break;
  }

  glop_info = (struct t_global_op_information *)
                query_prio_queue (cua,
                                  (t_priority) glop->identificator);

  if (glop_info != (struct t_global_op_information *) 0)
  { /* Aixo no hauria de passar mai! */
    if (debug & D_COMM)
    {
      PRINT_TIMER (current_time);
      printf (
        ": WARNING ('inicialitza_info_nova_globalop'): Global operation %s (%d) already exists\n",
        glop->name,
        glop->identificator
      );
    }
  }

  glop_info = (struct t_global_op_information *)
              malloc (sizeof (struct t_global_op_information) );

  glop_info->identificator = glop->identificator;
  glop_info->FIN_model     = model;
  glop_info->FIN_size      = GOP_SIZE_CURR;
  glop_info->FOUT_model    = model;
  glop_info->FOUT_size     = GOP_SIZE_CURR;

  insert_queue (cua, (char *) glop_info, (t_priority) glop->identificator);
}

void new_global_op (int         identificator,
                    const char *name)
{
  struct t_global_op_definition  *glop;
  struct t_machine  *machine;
  size_t             machines_it;

  /* Es guarda la definicio d'aquesta operació col.lectiva */
  glop = (struct t_global_op_definition *)
         query_prio_queue (&Global_op, (t_priority) identificator);

  if (glop != GOPD_NIL)
  {
    if (debug & D_COMM)
    {
      PRINT_TIMER (current_time);
      printf (
        ": WARNING ('new_global_op'): Global operation %s (%d) already exists\n",
        glop->name,
        glop->identificator
      );
    }
  }

  glop = (struct t_global_op_definition *)
         malloc (sizeof (struct t_global_op_definition) );

  glop->name          = strdup (name);
  glop->identificator = identificator;

  insert_queue (&Global_op, (char *) glop, (t_priority) identificator);

  /* S'hauria de guardar informació d'aquesta operació per cada màquina */
  /* JGG (2012/01/17): new ways to navigate through machines
  for (machine  = (struct t_machine *) head_queue(&Machine_queue);
       machine != MA_NIL;
       machine  = (struct t_machine *) next_queue(&Machine_queue))
  {
  */
  for (machines_it = 0; machines_it < Simulator.number_machines; machines_it++)
  {
    machine = &Machines[machines_it];

    inicialitza_info_nova_globalop (machine->communication.global_op_model,
                                    glop,
                                    &machine->communication.global_ops_info);
  }

  /* Es fa el mateix per la xarxa externa */
  inicialitza_info_nova_globalop (Simulator.wan.global_op_model,
                                  glop,
                                  &Simulator.wan.global_ops_info);
}

/*
 * Transfer message time function
 */
/*t_nano */
void transferencia (long long int                  size,
                    int                            communication_type,
                    struct t_thread               *thread,
                    struct t_dedicated_connection *connection,
                    t_nano                        *temps_total,
                    t_nano                        *temps_recursos)
{
  struct t_node    *node, *node_partner;
  struct t_machine *machine;
  struct t_task    *task, *task_partner;
  t_nano           bandw, temps, t_recursos;

  node    = get_node_of_thread (thread);
  machine = node->machine;
  task_partner = locate_task (thread->task->Ptask,
                              thread->action->desc.send.dest);

  switch (communication_type)
  {
  case MEMORY_COMMUNICATION_TYPE:  /* Es un missatge local al node */
    if (node->bandwidth == (t_nano) 0)
    {
      temps      = 0;
      t_recursos = 0;
    }
    else
    {
      /* Transmission time with available bandwidth (including bus contention) */
      bandw = recompute_memory_bandwidth(thread);
      thread->last_comm.bandwidth = bandw;
      thread->last_comm.bytes     = size;
      ASS_ALL_TIMER (thread->last_comm.ti, current_time);

      // printf("Available mem. bandwidth = %f\n", bandw);

      temps = (bandw * size);

      /* Transmission time with full bandwidth (actual resource usage) */
      bandw = bw_ns_per_byte(node->bandwidth);

      // printf("Max. mem. bandwidth = %f\n", bandw);

      t_recursos = (bandw * size);
    }
    break;

  case INTERNAL_NETWORK_COM_TYPE:
    if (machine->communication.remote_bandwidth == (t_nano) 0)
    {
      temps      = 0;
      t_recursos = 0;
    }
    else
    {
      bandw = recompute_internal_network_bandwidth (thread);
      thread->last_comm.bandwidth = bandw;
      thread->last_comm.bytes    = size;
      ASS_ALL_TIMER (thread->last_comm.ti, current_time);
      temps = (bandw * size);
      /* Es calcula el temps d'utilització dels recursos amb l'ample
         de banda maxim possible */
      bandw = (t_nano) machine->communication.remote_bandwidth;

      if (bandw != 0)
      {
        bandw = bw_ns_per_byte (bandw);
      }

      t_recursos = (bandw * size);
    }
    break;

  case EXTERNAL_NETWORK_COM_TYPE:
    if (Simulator.wan.bandwidth == (t_nano) 0)
    {
      temps      = 0;
      t_recursos = 0;
    }
    else
    {
      recompute_external_network_traffic (size);
      bandw = recompute_external_network_bandwidth (thread);
      /* Aqui es passa de Mbytes/sec a microsegons/byte. */
      bandw = bw_ns_per_byte (bandw);
      thread->last_comm.bandwidth = bandw;
      thread->last_comm.bytes    = size;
      ASS_ALL_TIMER (thread->last_comm.ti, current_time);
      temps = (bandw * size);

      node_partner = get_node_of_task (task_partner);

      temps +=
        Simulator.wan.flight_times[node->machine->id][node_partner->machine->id];

      /* Es calcula el temps d'utilització dels recursos amb l'ample
         de banda maxim possible */
      bandw = (t_nano) Simulator.wan.bandwidth;
      if (bandw != 0)
      {
        bandw = bw_ns_per_byte (bandw);
      }
      t_recursos = (bandw * size);
    }
    break;

  case DEDICATED_CONNECTION_COM_TYPE:
    /* En aquest cas el parametre connection no pot ser NULL */
    if (connection->bandwidth == (t_nano) 0)
    {
      temps      = 0;
      t_recursos = 0;
    }
    else
    {
      /* Se suposa que els "busos" de la connexio no seran mai compartits
         i, per tant, no caldra mai recalcular l'ample de banda */
      bandw = bw_ns_per_byte (connection->bandwidth);
      thread->last_comm.bandwidth = bandw;
      thread->last_comm.bytes = size;
      ASS_ALL_TIMER (thread->last_comm.ti, current_time);
      temps = (bandw * size);
      /* Falta sumar-hi el flight time*/
      temps += connection->flight_time;
      /* Es calcula el temps d'utilització dels recursos */
      t_recursos = (bandw * size);
    }
    break;

  case EXTERNAL_MODEL_COM_TYPE:
      /* Logica para cálcular el tiempo de ocupación de la red NO Dimemas */
      if (external_comm_library_loaded == FALSE)
      {
        panic("Computing transfer time through external model not defined\n");
      }

      node_partner = get_node_of_task (task_partner);

      bandw = get_bandwidth_value(node->nodeid,
                                  node_partner->nodeid,
                                  thread->task->taskid,
                                  task_partner->taskid,
                                  thread->action->desc.send.mess_tag,
                                  size);

      if (bandw != 0)
      {
        bandw = bw_ns_per_byte (bandw);
      }
      temps = (bandw * size);
      t_recursos = (bandw * size);
      break;
  default:
    panic ("Unknown communication type!\n");
    break;
  }

  if (temps_total != NULL)
  {
    *temps_total = temps;
  }

  /* Si es volia calcular el temps d'ocupacio dels recursos, tambe s'ha
     de retornar. */
  if (temps_recursos != NULL)
  {
    *temps_recursos = t_recursos;
  }

  if (debug & D_COMM)
  {
    PRINT_TIMER (current_time);
    printf
    (
      ": TRANSFERENCIA\tP%02d T%02d (t%02d) -> T%d Size: %lldb Time: %.9f Rsrc_Time: %.9f\n",
      IDENTIFIERS (thread),
      task_partner->taskid,
      size,
      temps / 1e9,
      t_recursos / 1e9
    );
  }

  /* Es retorna el temps necessari estimat per fer la transferencia */
  /* return(temps); */
}



#define COMPROVA_CONDICIO_MIDA_CONNEXIO(c_size,c_cond, m_size) \
  ( \
    (c_cond==0)? (m_size < c_size) : /* < */\
                 ((c_cond==1)? (m_size == c_size) : /* = */\
                               (m_size > c_size))   /* > */\
  )

int connection_can_be_used (struct t_dedicated_connection *connection,
                            int mess_tag,
                            int mess_size)
{
  int i;
  int res, res_size1, res_size2;

  /* Es comprova si el tag donat pot utilitzar la connexio dedicada */
  for (i = 0; i < connection->number_of_tags; i++)
  {
    if (connection->tags[i] == mess_tag) break;
  }

  if (i == connection->number_of_tags) /* El tag no es permes */
    return (0);

  /* Es comprova si la mida del missatge esta dins del marge permes */
  res_size1 = COMPROVA_CONDICIO_MIDA_CONNEXIO (connection->first_message_size,
              connection->first_size_condition,
              mess_size);
  res_size2 = COMPROVA_CONDICIO_MIDA_CONNEXIO (connection->second_message_size,
              connection->second_size_condition,
              mess_size);

  res = ( (connection->operation == 0) ? (res_size1 && res_size2) : /* AND */
          (res_size1 || res_size2) ); /* OR */

  /*
    printf("Connexio %d tag %d mida %d: res1 %d res2 %d res %d\n",
           connection->id, mess_tag, mess_size, res_size1, res_size2, res);
  */
  return (res);
}


int get_communication_type (struct t_task *task,
                            struct t_task *task_partner,
                            int mess_tag,
                            int mess_size,
                            struct t_dedicated_connection **connection)
{
  int result_type;
  struct t_node  *node, *node_partner;
  struct t_machine *s_machine, *d_machine;
  struct t_dedicated_connection *d_con;

  node         = get_node_of_task (task);
  node_partner = get_node_of_task (task_partner);

  if (node == node_partner)
  {
    /* Es un missatge local al node */
    result_type = MEMORY_COMMUNICATION_TYPE;
  }
  else if (node->machine == node_partner->machine)
  {
    /* Es un missatge de la xarxa interna a la maquina */
    result_type = INTERNAL_NETWORK_COM_TYPE;
  }
  else
  {
    /* Es un missatge entre dues maquines diferents */
    /* Cal mirar si s'utilitza la xarxa externa o una connexio dedicada */
    s_machine = node->machine;
    d_machine = node_partner->machine;
    for (
      d_con = (struct t_dedicated_connection *)
              head_queue (& (s_machine->dedicated_connections.connections) );
      d_con != DC_NIL;
      d_con = (struct t_dedicated_connection *)
              next_queue (& (s_machine->dedicated_connections.connections) ) )
    {
      if (
        ( (d_con->source_id == s_machine->id) &&
          (d_con->destination_id == d_machine->id) ) || /* Sentit normal */
        ( (d_con->source_id == d_machine->id) &&
          (d_con->destination_id == s_machine->id) ) ) /* Sentit invers */
      {
        /* La connexio es entre les dues maquines correctes. Nomes
           cal mirar si es pot utilitzar per aquest missatge concret. */
        if (connection_can_be_used (d_con, mess_tag, mess_size) )
          break;
      }
    }
    if (d_con != DC_NIL)
    {
      /* Hem d'utilitzar una connexio dedicada, que s'ha de retornar
         al parametre connection a no ser que sigui NULL */
      if (connection != NULL)
      {
        *connection = d_con;
      }
      result_type = DEDICATED_CONNECTION_COM_TYPE;
    }
    else /* Hem d'utilitzar la xarxa externa */
    {
      result_type = EXTERNAL_NETWORK_COM_TYPE;
    }
  }
  return result_type;
}

void really_send_memory_message (struct t_thread *thread)
{
  struct t_node            *node;
  struct t_machine         *machine;
  struct t_task            *task, *task_partner;
  struct t_action          *action;
  struct t_account         *account;
  struct t_send            *mess;
  struct t_bus_utilization *bus_utilization;

  t_nano                    ti, t_recursos;
  dimemas_timer             tmp_timer, tmp_timer2;

  node    = get_node_of_thread (thread);
  machine = node->machine;
  task    = thread->task;
  action  = thread->action;
  mess    = & (action->desc.send);

  task_partner = locate_task (task->Ptask, mess->dest);

  if (LINKS_get_mem_links(thread, task, task_partner) )
  {
    if (node->max_memory_messages != 0)
    {
      if (machine->communication.policy == COMMUNIC_FIFO)
      {
        if (node->cur_memory_messages >= node->max_memory_messages)
        {
          if (debug & D_COMM)
          {
            PRINT_TIMER (current_time);
            printf (": COMMUNIC_send\tP%02d T%02d (t%02d) Blocked (Waiting intra-node)\n",
                    IDENTIFIERS (thread));
          }
          inFIFO_queue (&node->wait_for_mem_bus, (char *) thread);
          /* FEC: Comenc,a el temps que el thread passa esperant un bus */
          START_BUS_WAIT_TIME (thread);
          /**************************************************************/

            return;
        }

        if (debug & D_COMM)
        {
          PRINT_TIMER (current_time);
          printf (": COMMUNIC_send\tP%02d T%02d (t%02d) Obtains bus\n",
                  IDENTIFIERS (thread) );
          // printf ("\n\nNow I should proceed to transfering the message\n");
        }
      }
      node->cur_memory_messages++;
      bus_utilization = (struct t_bus_utilization *)
                        malloc (sizeof (struct t_bus_utilization) );
      bus_utilization->sender = thread;
      ASS_ALL_TIMER (bus_utilization->initial_time, current_time);
      inFIFO_queue (&node->threads_in_memory, (char*) bus_utilization);
    }

    if (debug & D_COMM)
    {
      PRINT_TIMER (current_time);
      printf (
        ": COMMUNIC_send\tP%02d T%02d (t%02d) -> T%02d Tag(%d) SEND (MEMORY)\n",
        IDENTIFIERS (thread),
        mess->dest,
        mess->mess_tag
      );
    }

    account = current_account (thread);
    SUB_TIMER (current_time, thread->initial_communication_time, tmp_timer);
    ADD_TIMER (tmp_timer,
               account->block_due_resources, account->block_due_resources);
    thread->physical_send = current_time;
    thread->last_paraver  = current_time;
    /* ti = transferencia(mess->mess_size, comm_type, thread,NULL, &t_recursos); */
    transferencia (mess->mess_size,
                   MEMORY_COMMUNICATION_TYPE,
                   thread,
                   NULL,
                   &ti,
                   &t_recursos);

    if (t_recursos > ti)
    {
      /* DEBUG */
      printf ("Memory communication. Resources time = %f - Transfer Time = %f\n",
              t_recursos,
              ti);

      panic ("resources > transmission time!\n");

    }
    /* Abans de programar la fi de la comunicacio, es programa la fi de la
     * utilització dels recursos reservats. */
    FLOAT_TO_TIMER (t_recursos, tmp_timer);
    ADD_TIMER (current_time, tmp_timer, tmp_timer);

        /* tmp_timer has the time for COM_TIMER_OUT_RESOURCES */
    /* Es programa el final de la comunicació punt a punt. */
    FLOAT_TO_TIMER (ti, tmp_timer2);
    ADD_TIMER (current_time, tmp_timer2, tmp_timer2);

    EVENT_timer (tmp_timer, NOT_DAEMON, M_COM, thread, COM_TIMER_OUT_RESOURCES);
    thread->event = EVENT_timer (tmp_timer2, NOT_DAEMON, M_COM, thread, COM_TIMER_OUT);

  }
}

void really_send_internal_network(struct t_thread *thread)
{
  struct t_node            *node, *node_partner;
  struct t_task            *task, *task_partner;
  struct t_action          *action;
  struct t_account         *account;
  struct t_send            *mess;
  struct t_bus_utilization *bus_utilization;
  struct t_machine         *machine;
  t_nano                   ti, t_recursos;
  dimemas_timer             tmp_timer, tmp_timer2;
  int                       comm_type;

  node   = get_node_of_thread (thread);
  task   = thread->task;
  action = thread->action;
  mess   = & (action->desc.send);

  task_partner = locate_task (task->Ptask, mess->dest);
  node_partner = get_node_of_task (task_partner);

  /* Tots dos nodes son de la mateixa maquina */
  machine = node->machine;

  if (LINKS_get_network_links(thread, node, node_partner) )
  {

    /* JGG (05/11/2004): El 'comm_type' lo cogemos del mensaje, es más rápido
    comm_type =
      (node == node_partner ? MEMORY_COMMUNICATION_TYPE : INTERNAL_NETWORK_COM_TYPE);
    */

    if (machine->communication.num_messages_on_network != 0)
    {
      if (machine->communication.policy == COMMUNIC_FIFO)
      {
        if (machine->network.curr_on_network >= machine->communication.num_messages_on_network)
        {
          if (debug & D_COMM)
          {
            PRINT_TIMER (current_time);
            printf (
              ": COMMUNIC_send\tP%02d T%02d (t%02d) Blocked (Bus Waiting)\n",
              IDENTIFIERS (thread)
            );
          }
          inFIFO_queue (&machine->network.queue, (char *) thread);
          /* FEC: Comenc,a el temps que el thread passa esperant un bus */
          START_BUS_WAIT_TIME (thread);
          /**************************************************************/

          return;
        }

        if (debug & D_COMM)
        {
          PRINT_TIMER (current_time);
          printf (": COMMUNIC_send\tP%02d T%02d (t%02d) Obtains bus\n",
                  IDENTIFIERS (thread) );
          // printf ("\n\nNow I should proceed to transfering the message\n");
        }
      }
      machine->network.curr_on_network++;
      bus_utilization = (struct t_bus_utilization*)
                        malloc (sizeof (struct t_bus_utilization) );
      bus_utilization->sender = thread;
      ASS_ALL_TIMER (bus_utilization->initial_time, current_time);
      inFIFO_queue (&machine->network.threads_on_network, (char*) bus_utilization
      );
    }


    if (debug & D_COMM)
    {
      PRINT_TIMER (current_time);
      printf (
        ": COMMUNIC_send\tP%02d T%02d (t%02d) -> T%02d Tag(%d) SEND (INTERNAL)\n",
        IDENTIFIERS (thread),
        mess->dest,
        mess->mess_tag
      );
    }

//    printf("\n\n Now I will call the actual sending of the message\n");

    account = current_account (thread);
    SUB_TIMER (current_time, thread->initial_communication_time, tmp_timer);
    ADD_TIMER (tmp_timer,
               account->block_due_resources, account->block_due_resources);
    thread->physical_send = current_time;
    thread->last_paraver = current_time;
    /* ti = transferencia(mess->mess_size, comm_type, thread,NULL, &t_recursos); */
    transferencia (mess->mess_size,
                   INTERNAL_NETWORK_COM_TYPE,
                   thread,
                   NULL,
                   &ti,
                   &t_recursos);

    if (t_recursos > ti)
    {
      /* DEBUG
      printf ("Internal Network. Resources time = %f - Transfer Time = %f\n",
              t_recursos,
              ti);
      */

      panic ("resources > transmission time!\n");

    }
    /* Abans de programar la fi de la comunicacio, es programa la fi de la
     * utilització dels recursos reservats. */
    FLOAT_TO_TIMER (t_recursos, tmp_timer);
    ADD_TIMER (current_time, tmp_timer, tmp_timer);

    /* GRH (25/06/2008) RTT modification for rendez-vous protocol */
    /* Round Trip Time for receives - recvs*/
    if (RTT_enabled && mess->rendez_vous && (comm_type == INTERNAL_NETWORK_COM_TYPE) )
    {
      struct t_thread          *partner;
      t_nano                  roundtriptime;
      dimemas_timer            rtt_timer;

      if (thread->roundtrip_done == FALSE)
      {
        roundtriptime = RTT_time / 2.0;
        if (RTT_time != (t_nano) 0) {
          account = current_account (thread);
          FLOAT_TO_TIMER (roundtriptime, rtt_timer);
          ADD_TIMER (account->latency_time, rtt_timer, account->latency_time);

          ti += roundtriptime;
          thread->physical_send += roundtriptime;
        }
      }
      /* partner must also so roundtrip time
      partner = locate_receiver (&(task_partner->recv),
                               task->taskid,
                               mess->mess_tag,
                               mess->communic_id);
      if (partner != TH_NIL) { } */
    }

    /* tmp_timer has the time for COM_TIMER_OUT_RESOURCES */
    /* Es programa el final de la comunicació punt a punt. */
    FLOAT_TO_TIMER (ti, tmp_timer2);
    ADD_TIMER (current_time, tmp_timer2, tmp_timer2);

#ifdef VENUS_ENABLED
    if ( (!VC_is_enabled() ) || (mess->comm_type != INTERNAL_NETWORK_COM_TYPE) ) {
      EVENT_timer (tmp_timer, NOT_DAEMON, M_COM, thread, COM_TIMER_OUT_RESOURCES);
      thread->event =
        EVENT_timer (tmp_timer2, NOT_DAEMON, M_COM, thread, COM_TIMER_OUT);
    }
    else
    {
      double dtime;
      struct t_event *out_resources_ev;

      out_resources_ev = EVENT_venus_timer (tmp_timer, NOT_DAEMON, M_COM, thread, COM_TIMER_OUT_RESOURCES);
      thread->event =
        EVENT_venus_timer (tmp_timer2, NOT_DAEMON, M_COM, thread, COM_TIMER_OUT);

      TIMER_TO_FLOAT (current_time, dtime);
      dtime = thread->physical_send;
      if (mess->rendez_vous)
      {
        VC_command_rdvz_ready (dtime, node->nodeid, node_partner->nodeid, mess->mess_tag, mess->mess_size, thread->event, out_resources_ev, task->Ptask->Ptaskid, task_partner->Ptask->Ptaskid);
      }
      else
      {
        VC_command_send (dtime, node->nodeid, node_partner->nodeid, mess->mess_tag, mess->mess_size, thread->event, out_resources_ev, task->Ptask->Ptaskid, task_partner->Ptask->Ptaskid);
      }
    }
#else
//      printf("\n\n And to put the event stating when the communication finishes\n");
    EVENT_timer (tmp_timer, NOT_DAEMON, M_COM, thread, COM_TIMER_OUT_RESOURCES);
    thread->event =
      EVENT_timer (tmp_timer2, NOT_DAEMON, M_COM, thread, COM_TIMER_OUT);
#endif
  }
}

void really_send_external_network (struct t_thread *thread)
{
  struct t_node    *node, *node_partner;
  struct t_task    *task, *task_partner;
  struct t_action  *action;
  struct t_account *account;
  struct t_send    *mess;
  t_nano           ti, t_recursos;
  dimemas_timer     tmp_timer;

  /* Si es descomenta el codi que reserva busos, tambe cal descomentar aixo:
  struct t_bus_utilization *bus_utilization; */

  node = get_node_of_thread (thread);
  task = thread->task;
  action = thread->action;
  mess = & (action->desc.send);

  task_partner = locate_task (task->Ptask, mess->dest);
  node_partner = get_node_of_task (task_partner);

  if (LINKS_get_wan_links(thread, node->machine, node_partner->machine) )
  {
    /* Aqui es guardaria l'utilitzacio del bus per despres poder recalcular els
     * temps estimats de totes les transferencies que s'estiguessin fent per la
     * xarxa externa quan es comença o s'acaba una transferencia; tot i que en
     * realitat la xarxa externa no te busos. Pero aixo esta desactivat perque no
     * es vol aplicar. Nomes es vol fer que es calculi el temps estimat una
     * vegada, al començar la comunicacio. Si es volgues recalcular aixo tambe
     * caldria descomentar del recompute_external_network_bandwidth la part
     * corresponent i del COMMUNIC_external_network_COM_TIMER_OUT descomentar
     * on s'alliberen els busos. */
    /*
      bus_utilization =
        (struct t_bus_utilization *) malloc(sizeof(struct t_bus_utilization));
      bus_utilization->sender = thread;
      ASS_ALL_TIMER (bus_utilization->initial_time, current_time);
      inFIFO_queue (
        &Simulator.wan.threads_on_network,
        (char *)bus_utilization
      );
    */
    if (debug & D_COMM)
    {
      PRINT_TIMER (current_time);
      printf (
        ": COMMUNIC_send\tP%02d T%02d (t%02d) -> T%02d Tag(%d) SEND (EXTERNAL) message\n",
        IDENTIFIERS (thread),
        mess->dest,
        mess->mess_tag
      );
    }
    account = current_account (thread);
    SUB_TIMER (current_time, thread->initial_communication_time, tmp_timer);
    ADD_TIMER (
      tmp_timer,
      account->block_due_resources,
      account->block_due_resources
    );
    thread->physical_send = current_time;
    thread->last_paraver  = current_time;
    /* ti = transferencia(
           mess->mess_size,
           EXTERNAL_NETWORK_COM_TYPE,
           thread,
           NULL,
           &t_recursos
         ); */

    transferencia (
      mess->mess_size,
      EXTERNAL_NETWORK_COM_TYPE,
      thread,
      NULL,
      &ti,
      &t_recursos
    );

    if (t_recursos > ti)
    {
      /* DEBUG */
      printf ("Resources time = %.20f - Transfer Time = %.20f\n",
              t_recursos,
              ti);

      panic ("resources > transmission time!\n");
    }
    /* Abans de programar la fi de la comunicacio, es programa la fi de la
       utilització dels recursos reservats. Però, de moment, ho deixo al
       mateix instant de temps. */
    FLOAT_TO_TIMER (t_recursos, tmp_timer);
    ADD_TIMER (current_time, tmp_timer, tmp_timer);
    EVENT_timer (tmp_timer, NOT_DAEMON, M_COM, thread, COM_TIMER_OUT_RESOURCES);
    /* Es programa el final de la comunicació punt a punt. */
    FLOAT_TO_TIMER (ti, tmp_timer);
    ADD_TIMER (current_time, tmp_timer, tmp_timer);
    thread->event =
      EVENT_timer (tmp_timer, NOT_DAEMON, M_COM, thread, COM_TIMER_OUT);
  } /* endif (get_machine_links(...)) */
}

void really_send_dedicated_connection (struct t_thread               *thread,
                                       struct t_dedicated_connection *connection)
{
  struct t_action  *action;
  struct t_account *account;
  struct t_send    *mess;
  t_nano           ti, t_recursos;
  dimemas_timer     tmp_timer;

  action = thread->action;
  mess   = & (action->desc.send);

  if (LINKS_get_dedicated_connection_links(thread, connection) )
  {
    account = current_account (thread);
    SUB_TIMER (current_time, thread->initial_communication_time, tmp_timer);
    ADD_TIMER (
      tmp_timer,
      account->block_due_resources,
      account->block_due_resources
    );
    thread->physical_send = current_time;
    thread->last_paraver = current_time;
    /* ti = transferencia(
           mess->mess_size,
           DEDICATED_CONNECTION_COM_TYPE,
           thread,
           connection,
           &t_recursos
         ); */

    transferencia (
      mess->mess_size,
      EXTERNAL_NETWORK_COM_TYPE,
      thread,
      NULL,
      &ti,
      &t_recursos
    );

    if (t_recursos > ti)
    {
      /* DEBUG */
      printf ("Resources time = %.20f - Transfer Time = %.20f\n",
              t_recursos,
              ti);

      panic ("resources > transmission time!\n");
    }
    /* Abans de programar la fi de la comunicacio, es programa la fi de la
       utilització dels recursos reservats. Però, de moment, ho deixo al
       mateix instant de temps. */
    FLOAT_TO_TIMER (t_recursos, tmp_timer);
    ADD_TIMER (current_time, tmp_timer, tmp_timer);
    EVENT_timer (tmp_timer,
                 NOT_DAEMON,
                 M_COM, thread,
                 COM_TIMER_OUT_RESOURCES);

    /* Es programa el final de la comunicació punt a punt. */
    FLOAT_TO_TIMER (ti, tmp_timer);
    ADD_TIMER (current_time, tmp_timer, tmp_timer);
    thread->event = EVENT_timer (tmp_timer,
                                 NOT_DAEMON,
                                 M_COM,
                                 thread,
                                 COM_TIMER_OUT);

    if (debug & D_COMM)
    {
      PRINT_TIMER (current_time);
      printf (
        ": COMMUNIC\tP%02d T%02d (t%02d) -> T%02d Tag(%d) SEND (DEDICATED) message\n",
        IDENTIFIERS (thread),
        mess->dest,
        mess->mess_tag
      );
    }
  }
}

void really_send_external_model_comm_type (struct t_thread *thread)
{
  struct t_action  *action;
  struct t_account *account;
  struct t_send    *mess;
  t_nano            ti, t_recursos;
  dimemas_timer     tmp_timer;

  action = thread->action;
  mess   = &(action->desc.send);

  account = current_account(thread);
  SUB_TIMER(current_time, thread->initial_communication_time, tmp_timer);
  ADD_TIMER (
    tmp_timer,
    account->block_due_resources,
    account->block_due_resources
  );
  thread->physical_send = current_time;
  thread->last_paraver = current_time;

  transferencia(mess->mess_size,
                EXTERNAL_MODEL_COM_TYPE,
                thread,
                NULL,
                &ti,
                &t_recursos);

  if (t_recursos>ti)
  {
    panic("resources > transmission time!\n");
  }
  /* Abans de programar la fi de la comunicacio, es programa la fi de la
     utilització dels recursos reservats. Però, de moment, ho deixo al
     mateix instant de temps. */
  FLOAT_TO_TIMER (t_recursos, tmp_timer);
  ADD_TIMER (current_time, tmp_timer, tmp_timer);
  EVENT_timer (
    tmp_timer,
    NOT_DAEMON,
    M_COM, thread,
    COM_TIMER_OUT_RESOURCES
  );
  /* Es programa el final de la comunicació punt a punt. */
  FLOAT_TO_TIMER (ti, tmp_timer);
  ADD_TIMER (current_time, tmp_timer, tmp_timer);
  thread->event =
    EVENT_timer (tmp_timer, NOT_DAEMON, M_COM, thread, COM_TIMER_OUT);

  if (debug&D_COMM)
  {
    PRINT_TIMER (current_time);
    printf (
      ": COMMUNIC_send P%02d T%02d (t%02d) -> T%d Tag(%d) SEND (OTHER) message\n",
      IDENTIFIERS (thread),
      mess->dest,
      mess->mess_tag
    );
  }
}

void really_send (struct t_thread *thread)
{
  struct t_task   *task,
      *task_partner;
  struct t_action *action;
  struct t_send   *mess;
  int              kind, kind2;
  struct t_dedicated_connection *connection;

  task = thread->task;
  action = thread->action;
  mess = & (action->desc.send);

  task_partner = locate_task (task->Ptask, mess->dest);
  if (task_partner == T_NIL)
{
    panic ("Task partner not found!\n");
  }

  /* JGG: Ahora obtenemos el 'kind' por a partir del mensaje
  kind2 = get_communication_type (task, task_partner, mess->mess_tag,
                                  mess->mess_size, &connection);
  */

  kind = mess->comm_type;


  if (debug & D_COMM)
  {
    PRINT_TIMER (current_time);
    printf (
      ": COMMUNIC_send\tP%02d T%02d (t%02d) -> T%02d Tag: %d Type: %d (Really Send)\n",
      IDENTIFIERS (thread),
      mess->dest,
      mess->mess_tag,
      mess->comm_type);
  }


  // switch (kind2)
  switch (kind)
  {
  case MEMORY_COMMUNICATION_TYPE:
    really_send_memory_message(thread);
    break;
  case INTERNAL_NETWORK_COM_TYPE:
    really_send_internal_network(thread);
    break;
  case EXTERNAL_NETWORK_COM_TYPE:
    really_send_external_network (thread);
    break;
  case DEDICATED_CONNECTION_COM_TYPE:
    really_send_dedicated_connection (thread, connection);
    break;
  case EXTERNAL_MODEL_COM_TYPE:
    really_send_external_model_comm_type(thread);
    break;
  default:
    panic ("Incorrect communication type! kind = %d, kind2 %d", kind, kind2);
    break;
  } /* end switch */
  return;
}

struct t_communicator*
locate_communicator (struct t_queue *communicator_queue, int commid)
{
  register struct t_communicator *communicator;

  for (communicator  = (struct t_communicator *) head_queue (communicator_queue);
       communicator != (struct t_communicator *) 0;
       communicator  = (struct t_communicator *) next_queue (communicator_queue) )
  {
    if (communicator->communicator_id == commid)
    {
      break;
    }
  }
  return (communicator);
}

static int provide_log (int n)
{
  int i;

  for (i = 0; i < n; i++)
  {
    if ( (1 << i) >= n)
    {
      return (i);
    }
  }

  return (0);
}

static int compute_contention_stage (int ntasks, int num_busos)
{
  int i;
  int j;
  int loga;
  int parallel_comm;
  int total = 0;

  loga = provide_log (ntasks);

  /* If unlimited number of buses, contention is number of stages */
  if (num_busos == 0)
  {
    return (loga);
  }

  for (i = 0; i < loga; i++)
  {
    parallel_comm = 0;
    for (j = (1 << i) - 1; j < ntasks - 1; j = j + (1 << (i + 1) ) )
    {
#ifdef DEBUG
      if (j + (1 << i) < ntasks)
      {
        printf ("From %d to %d\n", j + 1, j + (1 << i) + 1);
      }
      else
      {
        printf ("From %d to %d\n", j + 1, ntasks);
      }
#endif /* DEBUG */
      parallel_comm ++;
    }

    if (parallel_comm % num_busos == 0)
    {
      total = total + parallel_comm / num_busos;
    }
    else
    {
      total = total + 1 + parallel_comm / num_busos;
    }
#ifdef DEBUG
    printf ("Stage %d parallel communications %d\n", i + 1, parallel_comm);
#endif /* DEBUG */
  }

  printf("Contention stages = %d\n", total);

  return (total);
}


/**************************************************************************
 ** Ara aquesta funcio nomes reserva tot el que cal a la maquina
 ** corresponent al thread donat (si es pot i es la primera vegada que es
 ** crida. Sino suposa que ja s'han reservat tots els busos). I, si era
 ** l'ultim thread que quedava, crida a la funcio corresponent a l'inici
 ** de l'operacio col.lectiva.
 **************************************************************************/
void global_op_get_all_buses (struct t_thread *thread)
{
  struct t_Ptask                *Ptask;
  struct t_communicator         *communicator;
  struct t_global_op_definition *glop;
  struct t_bus_utilization      *bus_utilization;
  struct t_action               *action;
  int                            i, comm_id, glop_id;
  struct t_node                 *node;
  struct t_machine              *machine;

  node    = get_node_of_thread (thread);
  machine = node->machine;

  if ( (machine->communication.num_messages_on_network) &&
       (thread->number_buses == 0)
     )
  {
    if (machine->network.curr_on_network != 0)
    {
      thread->number_buses =
        machine->communication.num_messages_on_network -
        machine->network.curr_on_network;

      machine->network.curr_on_network =
        machine->communication.num_messages_on_network;

      inFIFO_queue (&machine->network.queue, (char *) thread);

      /* FEC: Comenc,a el temps que el thread passa esperant busos */
      START_BUS_WAIT_TIME (thread);
      /*************************************************************/

      return;
    }
  }

  Ptask        = thread->task->Ptask;
  action       = thread->action;
  comm_id      = action->desc.global_op.comm_id;
  communicator = locate_communicator (&Ptask->Communicator, comm_id);

  if (communicator == COM_NIL )
  {
    panic (
      "Communication get_buses trough an invalid communicator %d to P%02d T%02d (t%02d)\n",
      comm_id,
      IDENTIFIERS (thread)
    );
  }

  glop_id = action->desc.global_op.glop_id;
  glop    = (struct t_global_op_definition *)
            query_prio_queue (&Global_op, (t_priority) glop_id);

  if (glop == (struct t_global_op_definition *) 0 )
  {
    panic (
      "Global operation %d undefined to P%02d T%02d (t%02d)\n",
      glop_id,
      IDENTIFIERS (thread)
    );
  }

  machine->network.curr_on_network =
    machine->communication.num_messages_on_network;

  bus_utilization =
    (struct t_bus_utilization *) malloc (sizeof (struct t_bus_utilization) );

  bus_utilization->sender = thread;
  ASS_ALL_TIMER (bus_utilization->initial_time, current_time);
  for (i = 0; i < machine->communication.num_messages_on_network; i++)
  {
    inFIFO_queue (&machine->network.threads_on_network, (char*) bus_utilization);
  }

  if (debug & D_COMM)
  {
    PRINT_TIMER (current_time);
    printf (
      ": GLOBAL_operation P%02d T%02d (t%02d) Gets machine %d buses to do '%s'\n",
      IDENTIFIERS (thread),
      machine->id,
      glop->name
    );
  }

  /* Si cal obtenir links de la xarxa externa, cal fer-ho aqui */
  if (count_queue (&communicator->machines_threads) > 1)
  {
    /* Com que hi ha mes d'una maquina cal obtenir els links cap a la
     * xarxa externa (tant el d'entrada com el de sortida). */

    /* Primer s'obtenen tots els de sortida. Quan es tenen tots s'intenten
     * aconseguir els d'entrada. S'ha de fer així per evitar deadlocks.
     * NO HI PODEN HAVER LINKS HALF-DUPLEX!!!!
     * Per poder-ne tenir caldria fer el que es fa només pels full duplex i,
     * entre les dues fases (quan ja es tenen tots els out, pero no ens in),
     * caldria agafar sequencialment per ordre de machine_id tots els
     * links half duplex. Despres (o potser paral.lelament) s'hauria de fer
     * l'altra fase (obtenir els in links de les full-duplex). */

    if (LINKS_get_single_wan_link(thread, machine, OUT_LINK) )
    {
      /* S'han pogut obtenir el link de sortida a la xarxa externa */
      /* A partir d'aqui aixo s'ha de fer en una funcio diferent perque
       * no es torni a fer la reserva dels busos anterior. */
      global_op_get_all_out_links (thread);
    }
    /* Si no s'han pogut obtenir s'haura bloquejat el thread i si s'han
     * pogut obtenir ja s'haura fet el que cal. En qualsevol cas, ja es
     * pot retornar. */
    return;
  }
  else /* Si no calen els links, ja es pot passar al seguent pas */
  {
    global_op_get_all_out_links (thread);
  }
}


/**************************************************************************
 ** Reserva els links necessaris i despres crida a global_op_get_all_links.
 **************************************************************************/
void global_op_reserva_links (struct t_thread *thread)
{
  struct t_node *node;
  struct t_machine *machine;
  struct t_Ptask *Ptask;
  struct t_communicator *communicator;
  struct t_action *action;
  int comm_id;

  node = get_node_of_thread (thread);
  machine = node->machine;

  Ptask        = thread->task->Ptask;
  action       = thread->action;
  comm_id      = action->desc.global_op.comm_id;
  communicator = locate_communicator (&Ptask->Communicator, comm_id);

  if (communicator == (struct t_communicator *) 0)
  {
    panic ("Global op get links trough an invalid communicator %d to P%02d T%02d (t%02d)\n",
           comm_id, IDENTIFIERS (thread) );
  }

  /* Cal saber si s'estan reservant els links d'entrada o de sortida */
  if (count_queue (&communicator->machines_threads) !=
      count_queue (&communicator->m_threads_with_links) )
  {
    /* Encara falten algunes maquines per tenir tots els out_links */
    if (LINKS_get_single_wan_link(thread, machine, OUT_LINK) )
    {
      /* S'han pogut obtenir els links de sortida a la xarxa externa*/
      global_op_get_all_out_links (thread);
    }
  }
  else
  {
    /* Ja es tenen tots els links de sortida, es busquen els d'entrada */
    if (LINKS_get_single_wan_link(thread, machine, IN_LINK) )
    {
      /* S'han pogut obtenir els links d'entrada de la xarxa externa*/
      global_op_get_all_in_links (thread);
    }
  }

}



/**************************************************************************
 ** Aquesta funcio suposa que ja s'han reservat tots els links de sortida
 ** de la maquina corresponent, cap a la xarxa externa. I, si era l'ultim
 ** thread que quedava, crida a la funcio corresponent a l'inici de la
 ** fase de reserva dels links d'entrada de la xarxa externa.
 **************************************************************************/
static void global_op_get_all_out_links (struct t_thread *thread)
{
  struct t_Ptask                *Ptask;
  struct t_communicator         *communicator;
  struct t_global_op_definition *glop;
  struct t_thread               *others;
  struct t_action               *action;
  struct t_node                 *node;
  struct t_machine              *machine;
  int                            comm_id, glop_id;


  Ptask        = thread->task->Ptask;
  action       = thread->action;
  comm_id      = action->desc.global_op.comm_id;
  communicator = locate_communicator (&Ptask->Communicator, comm_id);

  if (communicator == COM_NIL)
  {
    panic ("Communication get_links trough an invalid communicator %d to P%02d T%02d (t%02d)\n",
           comm_id,
           IDENTIFIERS (thread) );
  }

  glop_id = action->desc.global_op.glop_id;
  glop    = (struct t_global_op_definition *)
            query_prio_queue (&Global_op, (t_priority) glop_id);

  if (glop == GOPD_NIL)
  {
    panic ("Global operation %d undefined to P%02d T%02d (t%02d)\n",
           glop_id,
           IDENTIFIERS (thread) );
  }

  /* Si cal obtenir mes links de la xarxa externa, cal fer-ho aqui */
  if (count_queue (&communicator->machines_threads) > 1)
  {
    /* D'aquesta maquina ja es disposa del link de sortida i dels busos,
       per tant, s'ha d'indicar aixo i, si era l'ultima, ja es pot
       comenc,ar la seguent fase. En cas contrari,
       s'ha d'encuar aquest thread i seguir esperant. */

    /* S'afegeix el thread a la cua d'aquest communicador de threads que ja
       tenen el link de sortida reservat. */
    inFIFO_queue (&communicator->m_threads_with_links, (char *) thread);

    /* Es mira si ja es tenen tots */
    if (count_queue (&communicator->m_threads_with_links) !=
        count_queue (&communicator->machines_threads) )
    {
      /* Encara falten alguns threads per acabar d'obtenir tots els
         links de sortida. */
      return;
    }

    /* Ja es pot comenc,ar a la fase de reserva de links d'entrada */
    for (others  = (struct t_thread *) head_queue (&communicator->machines_threads);
         others != TH_NIL;
         others  = (struct t_thread *) next_queue (&communicator->machines_threads) )
    {
      /* Aquest thread reserva els recursos necessaris que falten de la
         seva maquina */
      node    = get_node_of_thread (others);
      machine = node->machine;

      if (LINKS_get_single_wan_link(others, machine, IN_LINK) )
      {
        /* S'han pogut obtenir els links d'entrada de la xarxa externa*/
        global_op_get_all_in_links (others);
      }
    }
    /* Si no s'han pogut obtenir s'haura bloquejat el thread i si s'han
       pogut obtenir ja s'haura fet el que cal. En qualsevol cas, ja es
       pot retornar. */
    return;

  }
  else /* Si no calen els links, ja es pot passar al seguent pas */
  {
    global_op_get_all_in_links (thread);
  }
}


/**************************************************************************
 ** Aquesta funcio suposa que ja s'han reservat tots els links de la
 ** maquina corresponent, cap a la xarxa externa. I, si era l'ultim thread
 ** que quedava, crida a la funcio corresponent a l'inici de l'operacio
 ** col.lectiva.
 **************************************************************************/
static void global_op_get_all_in_links (struct t_thread *thread)
{
  struct t_Ptask                *Ptask;
  struct t_communicator         *communicator;
  struct t_global_op_definition *glop;
  struct t_thread               *others;
  struct t_action               *action;
  int comm_id, glop_id, root_task;

  Ptask        = thread->task->Ptask;
  action       = thread->action;
  comm_id      = action->desc.global_op.comm_id;
  communicator = locate_communicator (&Ptask->Communicator, comm_id);

  if (communicator == COM_NIL)
  {
    panic (
      "Communication get_links trough an invalid communicator %d to P%02d T%02d (t%02d)\n",
      comm_id,
      IDENTIFIERS (thread)
    );
  }

  glop_id = action->desc.global_op.glop_id;
  glop    = (struct t_global_op_definition *)
            query_prio_queue (&Global_op, (t_priority) glop_id);

  if (glop == GOPD_NIL)
  {
    panic ("Global operation %d undefined to P%02d T%02d (t%02d)\n",
           glop_id,
           IDENTIFIERS (thread) );
  }

  /* D'aquesta maquina ja es disposa de tot el que es necessita, per tant,
     si era l'ultima, ja es pot comenc,ar la col.lectiva. En cas contrari,
     s'ha d'encuar aquest thread i seguir esperant. */

  /* S'afegeix el thread a la cua de threads del communicador */
  inFIFO_queue (&communicator->threads, (char *) thread);

  // if (count_queue (&communicator->threads) !=
  //    count_queue (&communicator->global_ranks) )
  if (count_queue(&communicator->threads) != communicator->size)
  {
    /* Encara falten alguns threads per acabar d'obtenir tot el que
       es necessita. */
    return;
  }

  /* Ja es pot comenc,ar a realitzar l'operacio col.lectiva */

  /* Es buida la cua m_threads_with_links perque ja no es necessita */
  for (others  = (struct t_thread *) outFIFO_queue (&communicator->m_threads_with_links);
       others != TH_NIL;
       others  = (struct t_thread *) outFIFO_queue (&communicator->m_threads_with_links) );
  // {}

  /* Search the root task
  root_task = from_rank_to_taskid (communicator, action->desc.global_op.root_rank);

  for (others  = (struct t_thread*) head_queue(&communicator->threads);
       others != TH_NIL;
       others  = (struct t_thread*) next_queue(&communicator->threads))
  {
    if (others->task->taskid == root_task)
    {
      break;
    }
  }

  if (others == TH_NIL)
  {
    panic("Unable to locate root T%02d for global operation\n", root_task);
    exit(EXIT_FAILURE);
  }
  */

  /* JGG (07/07/2010): New way to manage root thread */
  others = communicator->current_root;

  /* NO ES TREU el thread de root de la cua. Els tracto tots igual! */

  /* S'inicia l'operacio col.lectiva !! */
  start_global_op (others, DIMEMAS_GLOBAL_OP_MODEL);
}


/**************************************************************************
 ** Aquesta funcio calcula els temps estimats de latenica i temps total
 ** per una etapa d'una operacio col.lectiva.
 **************************************************************************/
void calcula_fan (t_nano bandw,     /* MB/s */
                  int num_partners, /* End-points of the current stage */
                  int num_busos,    /* Numero de busos disponibles */
                  int tipus_de_fan, /* 0 = IN / 1 = OUT */
                  int model,        /* 0, CONSTANT, LINEAL, LOGARITHMIC */
                  int size_type,    /* MIN, MAX, average, 2*MAX, send+recv */
                  int bytes_send,   /* Number of bytes send */
                  int bytes_recvd,  /* Number of bytes received */
                  t_nano startup,
                  t_nano *temps,
                  t_nano *latencia)
{
  int   mess_size;
  float factor;

  /* From MB/sec to ns/byte. */
  bandw = bw_ns_per_byte (bandw);

  /* Computation of global communication model */
  switch (size_type)
  {
  case GOP_SIZE_CURR:
    if (tipus_de_fan == FAN_IN)
    {
      mess_size = bytes_send;
    }
    else if (tipus_de_fan == FAN_OUT)
    {
      mess_size = bytes_recvd;
    }
    break;
  case GOP_SIZE_MIN:
    mess_size = MIN (bytes_send, bytes_recvd);
    break;
  case GOP_SIZE_MAX:
    mess_size = MAX (bytes_send, bytes_recvd);
    break;
  case GOP_SIZE_MEAN:
    mess_size = (bytes_send + bytes_recvd) / 2;
    break;
  case GOP_SIZE_2MAX:
    mess_size = 2 * MAX (bytes_send, bytes_recvd);
    break;
  case GOP_SIZE_SIR:
    mess_size = (bytes_send + bytes_recvd);
    break;
  default:
    panic ("Invalid size FIN/OUT %d for global operation\n", size_type);
    exit (EXIT_FAILURE);
    break;
  }

  if (debug & D_COMM)
  {
    PRINT_TIMER(current_time);
    printf(": FAN_CALCULATION Message size used %d\n", mess_size);
    PRINT_TIMER(current_time);
    printf(": FAN_CALCULATION startup %.9lf bandw %lf\n", startup, bandw);
  }

  switch (model)
  {
    case GOP_MODEL_0:
      *temps    = 0;
      *latencia = 0;

      if (debug & D_COMM)
      {
        PRINT_TIMER(current_time);
        printf(": FAN_CALCULATION Model 0\n");
      }

      break;
    case GOP_MODEL_CTE:
      factor    = 1.0;
      *temps    = startup + mess_size * bandw;
      *latencia = startup;

      if (debug & D_COMM)
      {
        PRINT_TIMER(current_time);
        printf(": FAN_CALCULATION CT Model\n");
      }

      break;
    case GOP_MODEL_LIN:
      factor    = (t_nano) num_partners - 1;
      *temps    = (startup + mess_size * bandw) * factor;
      *latencia = startup * factor;

      if (debug & D_COMM)
      {
        PRINT_TIMER(current_time);
        printf(": FAN_CALCULATION LIN Model\n");
      }

      break;
    case GOP_MODEL_LOG:
      factor    = compute_contention_stage (num_partners, num_busos);
      *temps    = (startup + mess_size * bandw) * factor;
      *latencia = startup * factor;

      if (debug & D_COMM)
      {
        PRINT_TIMER(current_time);
        printf(": FAN_CALCULATION LOG Model. Factor %d\n", factor);
      }

      break;

    default:
      panic ("Invalid model FIN/OUT %d for global operation\n", model);
      exit (EXIT_FAILURE);
  }
}



/**************************************************************************
 ** Es calculen els temps dins de cada node de la maquina i
 ** s'agafen els maxims.
 **************************************************************************/
static void calcula_temps_maxim_intra_nodes (struct t_machine      *machine,
                                             struct t_communicator *communicator,
                                             struct t_global_op_information *glop_info,
                                             int bytes_send,   /* Number of bytes send */
                                             int bytes_recvd,  /* Number of bytes received */
                                             t_nano *max_tnode_in,
                                             t_nano *max_lnode_in,
                                             t_nano *max_tnode_out,
                                             t_nano *max_lnode_out)
{
  struct t_node  *node;
  int*    nodeid;
  t_nano tauxn_in, lauxn_in, tauxn_out, lauxn_out;
  int num_cpus;
  double suma_aux, suma_maxim;

  /* S'inicialitzen els maxims */
  *max_tnode_in = *max_lnode_in = *max_tnode_out = *max_lnode_out = 0;
  suma_maxim = 0;

  /* Es calcula a tots els nodes de la maquina
#ifdef USE_EQUEUE
  for (node  = (struct t_node *) head_Equeue (&Node_queue);
       node != (struct t_node *) 0;
       node  = (struct t_node *) next_Equeue (&Node_queue) )
#else
  for (node  = (struct t_node *) head_queue (&Node_queue);
       node != (struct t_node *) 0;
       node  = (struct t_node *) next_queue (&Node_queue) )
#endif
  */
  for ( nodeid  = (int*) head_queue(&communicator->nodes_per_machine[machine->id]);
        nodeid != NULL;
        nodeid  = (int*) next_queue(&communicator->nodes_per_machine[machine->id]))
  {
    /* Nomes s'agafen els nodes d'aquesta maquina.
    if (node->machine != machine) continue;
    */
    node = get_node_by_id(*nodeid);

    int *tasks_per_node = (int*) query_prio_queue(&communicator->tasks_per_node,
                                                  (t_priority) *nodeid);

    /*
    if (debug & D_COMM)
    {
      PRINT_TIMER (current_time);
      printf (
        ": Machine %d Node: %d: Tasks involved = %d\n",
        machine->id,
        (*nodeid),
        (*tasks_per_node)
      );
    }
    */

    // num_cpus = count_queue (& (node->Cpus) );

    /* Cal calcular els temps dins d'aquest node */
    calcula_fan (node->bandwidth,
                 // num_tasks,
                 (*tasks_per_node),
                 0, /* Infinits busos */
                 FAN_IN,
                 glop_info->FIN_model,
                 glop_info->FIN_size,
                 bytes_send,
                 bytes_recvd,
                 node->local_startup,
                 &tauxn_in,
                 &lauxn_in);

    calcula_fan (node->bandwidth,
                 // num_tasks,
                 (*tasks_per_node),
                 0, /* Infinits busos */
                 FAN_OUT,
                 glop_info->FOUT_model,
                 glop_info->FOUT_size,
                 bytes_send,
                 bytes_recvd,
                 node->local_startup,
                 &tauxn_out,
                 &lauxn_out);

    if (debug & D_COMM)
    {
      PRINT_TIMER (current_time);
      printf (
        ": Machine %d Node: %d: tfin_int=%.9f lfin_int=%.9f tfout_int=%.9f lfout_int=%.9f\n",
        machine->id,
        (*nodeid),
        tauxn_in/1e9,
        lauxn_in/1e9,
        tauxn_out/1e9,
        lauxn_out/1e9
      );
    }

    /* S'agafen els del node amb la suma mes gran */
    suma_aux = (double) (tauxn_in + lauxn_in + tauxn_out + lauxn_out);
    if (suma_aux > suma_maxim)
    {
      /* S'agafa aquest node com a node de temps maxim d'aquesta maquina */
      suma_maxim     = suma_aux;
      *max_tnode_in  = tauxn_in;
      *max_lnode_in  = lauxn_in;
      *max_tnode_out = tauxn_out;
      *max_lnode_out = lauxn_out;
    }


  }

  /* Es retornen els temps del node de temps total maxim */
}


/**************************************************************************
 ** Es calcula el flight time maxims d'entrada i el maxim de sortida
 ** del thread donat.
 **************************************************************************/
static void calcula_maxim_flight_times (struct t_thread *thread,
                                        struct t_communicator *communicator,
                                        t_nano *maxflight_in,
                                        t_nano *maxflight_out)
{
  t_nano max_flight_in, max_flight_out;
  struct t_node  *node;
  struct t_machine *machine;
  int index_origen;
  struct t_thread *others;
  double flight_time_in, flight_time_out;

  node         = get_node_of_thread (thread);
  machine      = node->machine;
  index_origen = machine->id;

  max_flight_in  = 0;
  max_flight_out = 0;

  /* Es calcula el temps de cada maquina */
  for (others = (struct t_thread *) head_queue (&communicator->machines_threads);
       others != (struct t_thread *) 0;
       others = (struct t_thread *) next_queue (&communicator->machines_threads) )
  {
    /* No te sentit calcular el flight time a si mateixa */
    if (others == thread) continue;

    /* S'obte el node i la maquina corresponent al thread */
    node    = get_node_of_thread (others);
    machine = node->machine;

    /* S'agafen els flight times */
    flight_time_in  =
      Simulator.wan.flight_times[machine->id][index_origen];
    flight_time_out =
      Simulator.wan.flight_times[index_origen][machine->id];

    /* S'agafen els maxims */
    if (flight_time_in > max_flight_in)
    {
      max_flight_in = flight_time_in;
    }
    if (flight_time_out > max_flight_out)
    {
      max_flight_out = flight_time_out;
    }
  }

  /* Es retornen els maxims */
  *maxflight_in  = max_flight_in;
  *maxflight_out = max_flight_out;
}

/**************************************************************************
 ** Aquesta funcio calcula els temps estimats per realitzar l'operació
 ** col.lectiva.
 **************************************************************************/
void calcula_temps_operacio_global (struct t_thread *thread,
                                    dimemas_timer   *temps_latencia,
                                    dimemas_timer   *temps_recursos,
                                    dimemas_timer   *temps_final)
{
  struct t_Ptask                 *Ptask;
  struct t_communicator          *communicator;
  struct t_global_op_definition  *glop;
  struct t_thread                *others;
  struct t_action                *action;
  struct t_node                  *node;
  struct t_machine               *machine;
  struct t_global_op_information *glop_info;
  int                             comm_id, glop_id, num_maquines, num_tasks;
  int                             nodes_involved;

  t_nano tfin_node, lfin_node, tfout_node, lfout_node;
  t_nano tfin_int,  lfin_int,  tfout_int,  lfout_int;
  t_nano tfin_ext,  lfin_ext,  tfout_ext,  lfout_ext;
  t_nano taux_in, laux_in, taux_out, laux_out;
  t_nano tauxn_in, lauxn_in, tauxn_out, lauxn_out;

  t_nano flightin_ext, flightout_ext;
  t_nano temps, latencia, t_recursos; /* Temps totals */

  t_nano bandw_externa;
  double  suma_aux, suma_maxim;

  Ptask        = thread->task->Ptask;
  action       = thread->action;
  comm_id      = action->desc.global_op.comm_id;
  communicator = locate_communicator (&Ptask->Communicator, comm_id);

  if (communicator == COM_NIL)
  {
    panic ("Communication get_buses trough an invalid communicator %d to P%02d T%02d (t%02d)\n",
           comm_id, IDENTIFIERS (thread) );
  }

  glop_id = action->desc.global_op.glop_id;
  glop    = (struct t_global_op_definition *)
            query_prio_queue (&Global_op, (t_priority) glop_id);

  if (glop == GOPD_NIL)
  {
    panic (
      "Global operation %d undefined to P%02d T%02d (t%02d)\n",
      glop_id,
      IDENTIFIERS (thread)
    );
  }

  /* Communicator size */
  num_tasks = communicator->size;

  /* S'inicialitzen els temps */
  tfin_node    = lfin_node = tfout_node = lfout_node = 0;
  taux_in      = laux_in   = taux_out   = laux_out   = 0;
  tfin_int     = lfin_int  = tfout_int  = lfout_int  = 0;
  tauxn_in     = lauxn_in  = tauxn_out  = lauxn_out  = 0;
  tfin_ext     = lfin_ext  = tfout_ext  = lfout_ext  = 0;

  flightin_ext = flightout_ext = 0;
  suma_maxim   = 0;

  /* For each machine used we compute the inter-node communication time and
    the intra-node communication time */
  for (others  = (struct t_thread*) head_queue (&communicator->machines_threads);
       others != TH_NIL;
       others  = (struct t_thread*) next_queue (&communicator->machines_threads) )
  {
    /* S'obte el node i la maquina corresponent al thread */
    node    = get_node_of_thread (others);
    machine = node->machine;

    /* S'obte la informacio de les operacions col.lectives dins
       d'aquesta maquina. */
    glop_info = (struct t_global_op_information *)
                query_prio_queue (&machine->communication.global_ops_info,
                                  (t_priority) glop_id);

    if (glop_info == (struct t_global_op_information *) 0)
    {
      panic (
        "Global operation %d undefined to P%02d T%02d (t%02d)\n",
        glop_id,
        IDENTIFIERS (others)
      );
    }

    /*
    if (debug & D_COMM)
    {
      PRINT_TIMER (current_time);
      printf (
        ": Machine %d: GlobalOp = %d FIN_model = %d FIN_size = %d FOUT_model = %d FOUT_size = %d\n",
        machine->id,
        glop_info->identificator,
        glop_info->FIN_model,
        glop_info->FIN_size,
        glop_info->FOUT_model,
        glop_info->FOUT_size
      );
    }
    */

    /* Inter-node communication times */
    if (machine->number_of_nodes > 1)
    {
      // printf("machine->number_of_nodes = %d\n", machine->number_of_nodes);

      if (count_queue(&communicator->nodes_per_machine[machine->id]) > 0)
      {
        calcula_fan (machine->communication.remote_bandwidth,
                     count_queue(&communicator->nodes_per_machine[machine->id]),
                     machine->communication.num_messages_on_network,
                     FAN_IN,
                     glop_info->FIN_model,
                     glop_info->FIN_size,
                     action->desc.global_op.bytes_send,
                     action->desc.global_op.bytes_recvd,
                     node->remote_startup,
                     &taux_in,
                     &laux_in);

        calcula_fan (machine->communication.remote_bandwidth,
                     count_queue(&communicator->nodes_per_machine[machine->id]),
                     machine->communication.num_messages_on_network,
                     FAN_OUT,
                     glop_info->FOUT_model,
                     glop_info->FOUT_size,
                     action->desc.global_op.bytes_send,
                     action->desc.global_op.bytes_recvd,
                     node->remote_startup,
                     &taux_out,
                     &laux_out);

        if (debug & D_COMM)
        {
          PRINT_TIMER (current_time);
          printf (
            ": Machine %d (inter-nodes): tfin=%.9f lfin=%.9f tfout=%.9f lfout=%.9f\n",
            machine->id,
            taux_in/1e9,
            laux_in/1e9,
            taux_out/1e9,
            laux_out/1e9
          );

          /*
          PRINT_TIMER (current_time);
          printf (": Send_Size = %d, Recv_Size = %d, Nodes involved = %d\n",
                  action->desc.global_op.bytes_send,
                  action->desc.global_op.bytes_recvd,
                  count_queue(&communicator->nodes_per_machine[machine->id]));
          */
        }
      }
    }

    /* Intra-nodes communication times */
    calcula_temps_maxim_intra_nodes (machine,
                                     communicator,
                                     glop_info,
                                     action->desc.global_op.bytes_send,
                                     action->desc.global_op.bytes_recvd,
                                     &tauxn_in,
                                     &lauxn_in,
                                     &tauxn_out,
                                     &lauxn_out);

    if (debug & D_COMM)
    {
      PRINT_TIMER (current_time);
      printf (
        ": Machine %d (intra nodes): tfin=%.9f lfin=%.9f tfout=%.9f lfout=%.9f\n",
        machine->id,
        tauxn_in/1e9,
        lauxn_in/1e9,
        tauxn_out/1e9,
        lauxn_out/1e9
      );
    }

    /* Es calcula la suma total de temps entre nodes d'aquesta maquina
       mes els temps del node d'aquesta maquina de mes durada */
    suma_aux  = (double) (taux_in + laux_in + taux_out + laux_out);
    suma_aux += (double) (tauxn_in + lauxn_in + tauxn_out + lauxn_out);

    /* S'agafen els temps corresponents a la maquina de mes temps total */
    if (suma_aux > suma_maxim)
    {
      /* S'agafen tots els temps d'aquesta maquina */
      suma_maxim = suma_aux;

      tfin_node  = tauxn_in;
      lfin_node  = lauxn_in;
      tfout_node = tauxn_out;
      lfout_node = lauxn_out;

      tfin_int  = taux_in;
      lfin_int  = laux_in;
      tfout_int = taux_out;
      lfout_int = laux_out;
    }

    /* De moment s'ha decidit agafar el temps maxim de totes les maquines */
    /*    if (taux_in > tfin_int) tfin_int=taux_in;
        if (laux_in > lfin_int) lfin_int=laux_in;
        if (taux_out > tfout_int) tfout_int=taux_out;
        if (laux_out > lfout_int) lfout_int=laux_out;

    if (debug & D_COMM)
    {
      PRINT_TIMER (current_time);
      printf (
        ": Machine %d (Maximum):     tfin=%.9f lfin=%.9f tfout=%.9f lfout=%.9f\n",
        machine->id,
        taux_in/1e9,
        laux_in/1e9,
        taux_out/1e9,
        laux_out/1e9
      );
    }
    */
  }

  /* Cal contar el temps a la xarxa externa */
  num_maquines = count_queue (& (communicator->machines_threads) );
  if (num_maquines > 1)
  {

    /* S'obte la informacio de les operacions col.lectives a la
       xarxa externa. */
    glop_info = (struct t_global_op_information *)
                query_prio_queue (
                  &Simulator.wan.global_ops_info,
                  (t_priority) glop_id
                );

    if (glop_info == (struct t_global_op_information *) 0)
    {
      panic (
        "Global operation %d undefined to P%02d T%02d (t%02d)\n",
        glop_id,
        IDENTIFIERS (thread)
      );
    }

    /* Aqui hi ha d'haver el recompute_external_netwrok_bandewidth */
    if (Simulator.wan.bandwidth != 0)
    {
      /*recompute_external_network_traffic(size???); */
      bandw_externa = recompute_external_network_bandwidth (thread);
    }
    else
    {
      bandw_externa = (t_nano) 0;
    }

    /* Es calculen els temps */
    calcula_fan ( bandw_externa,
                  num_tasks,
                  0, /* Infinits busos */
                  FAN_IN,
                  glop_info->FIN_model, glop_info->FIN_size,
                  action->desc.global_op.bytes_send,
                  action->desc.global_op.bytes_recvd,
                  node->external_net_startup,
                  &tfin_ext,
                  &lfin_ext);

    calcula_fan (bandw_externa,
                 num_tasks,
                 0, /* Infinits busos */
                 FAN_OUT, /* FAN OUT */
                 glop_info->FOUT_model, glop_info->FOUT_size,
                 action->desc.global_op.bytes_send,
                 action->desc.global_op.bytes_recvd,
                 node->external_net_startup,
                 &tfout_ext,
                 &lfout_ext);

    /* Es calculen els flight times */
    calcula_maxim_flight_times (thread,
                                communicator,
                                &flightin_ext,
                                &flightout_ext);

    if (debug & D_COMM)
    {
      PRINT_TIMER (current_time);
      printf (
        ": External net: tfin_ext=%.9f lfin_ext=%.9f tfout_ext=%.9f lfout_ext=%.9f\n",
        tfin_ext/1e9,
        lfin_ext/1e9,
        tfout_ext/1e9,
        lfout_ext/1e9
      );

      PRINT_TIMER (current_time);
      printf (
        ": Num maquines %d startup xarxa externa %.9f\n",
        num_maquines,
        node->external_net_startup/1e9
      );
    }
  }

  /* S'acumulen els temps de totes les fases */
  temps    =
    tfin_node + tfout_node + tfin_int + tfout_int + tfin_ext + tfout_ext;

  latencia =
    lfin_node + lfout_node + lfin_int + lfout_int + lfin_ext + lfout_ext;

  /* S'agafa el temps amb els recursos ocupats */
  t_recursos = temps;

  /* Es sumen els flight times al temps final */
  temps += (flightin_ext + flightout_ext);

  /* Es retornen els temps estimats de durada de l'operacio col.lectiva */
  FLOAT_TO_TIMER (temps, *temps_final);
  FLOAT_TO_TIMER (latencia, *temps_latencia);
  FLOAT_TO_TIMER (t_recursos, *temps_recursos);

  if (debug & D_COMM)
  {
    PRINT_TIMER (current_time);
    printf (": GLOBAL_operation Total time=");
    PRINT_TIMER (*temps_final);
    printf (" ( Resources=");
    PRINT_TIMER (*temps_recursos);
    printf (" Startup=");
    PRINT_TIMER (*temps_latencia);
    printf (" ) \n");
  }
}

/**************************************************************************
 ** Aquesta funcio no reserva res. Dona per suposat que ja s'ha reservat
 ** tot el que calia. Simplement engega l'operacio col.lectiva.
 **************************************************************************/
static void start_global_op (struct t_thread *thread, int kind)
{
  struct t_Ptask *Ptask;
  struct t_communicator *communicator;
  struct t_global_op_definition * glop;
  struct t_thread *others;
  struct t_action *action;
  int comm_id, glop_id;
  dimemas_timer tmp_timer;
  dimemas_timer tmp_timer2;
  struct t_cpu *cpu;
  dimemas_timer temps_latencia, temps_recursos, temps_final, temps_operacio;

  Ptask        = thread->task->Ptask;
  action       = thread->action;
  comm_id      = action->desc.global_op.comm_id;
  communicator = locate_communicator (&Ptask->Communicator, comm_id);

  if (communicator == COM_NIL)
  {
    panic (
      "Communication get_buses trough an invalid communicator %d to P%02d T%02d (t%02d)\n",
      comm_id,
      IDENTIFIERS (thread)
    );
  }

  glop_id = action->desc.global_op.glop_id;
  glop    = (struct t_global_op_definition *)
            query_prio_queue (&Global_op, (t_priority) glop_id);

  if (glop == GOPD_NIL)
  {
    panic (
      "Global operation %d undefined to P%02d T%02d (t%02d)\n",
      glop_id,
      IDENTIFIERS (thread)
    );
  }


  if (debug & D_COMM)
  {
    PRINT_TIMER (current_time);
    printf (
      ": GLOBAL_operation P%02d T%02d (t%02d) (Root Task) Unlocks '%s'\n",
      IDENTIFIERS (thread),
      glop->name
    );
  }

  /* Calcula els temps estimats per realitzar l'operació col.lectiva */
  /* JGG (09/11/2004): En función al tipo de la operación, se calcula interna
   * o externamente */
  switch (kind)
  {
    case DIMEMAS_GLOBAL_OP_MODEL:

      calcula_temps_operacio_global (thread,
                                     &temps_latencia,
                                     &temps_recursos,
                                     &temps_final);

      /* Es programa l'event de final de la reserva dels recursos */
      ADD_TIMER (current_time, temps_recursos, tmp_timer);
      EVENT_timer (
        tmp_timer,
        NOT_DAEMON,
        M_COM,
        thread,
        COM_TIMER_GROUP_RESOURCES
      );
      break;

    case EXTERNAL_GLOBAL_OP_MODEL:
      if (external_comm_library_loaded == FALSE)
      {
        panic ("Executing global operation through external model library not loaded\n");
      }

      external_compute_global_operation_time(comm_id,
                                             glop_id,
                                             action->desc.global_op.bytes_send,
                                             action->desc.global_op.bytes_recvd,
                                             (t_nano*) &temps_latencia,
                                             (t_nano*) &temps_operacio);

      temps_final = temps_latencia + temps_operacio;

      if (debug&D_COMM)
      {
        PRINT_TIMER (current_time);
        printf (
          ": GLOBAL_operation P%02d T%02d (t%02d) '%s' USING EXTERNAL GLOBAL OPERATIONS MODEL\n",
          IDENTIFIERS (thread),
          glop->name
        );
      }

      /* JGG: There is no need to free the resources, because we use an external
       * model */
      break;

  }



  /* Es programa l'event de final de l'operacio col.lectiva */
  ADD_TIMER (current_time, temps_final, tmp_timer);

  EVENT_timer (
    tmp_timer,
    NOT_DAEMON,
    M_COM,
    thread,
    COM_TIMER_GROUP
  );

  /* Per cada thread del comunicador, es guarden les estadistiques i es
   * genera el que cal a la trac,a */
  for (others  = (struct t_thread *) head_queue (&communicator->threads);
       others != TH_NIL;
       others  = (struct t_thread *) next_queue (&communicator->threads) )
  {
    ASS_ALL_TIMER (others->collective_timers.with_resources, current_time);

    PARAVER_Wait (
      0,
      IDENTIFIERS (others),
      others->last_paraver,
      current_time,
      PRV_BLOCKED_ST
    );

    others->last_paraver = current_time;

    if (temps_latencia != 0)
    {
      ADD_TIMER (current_time, temps_latencia, tmp_timer2);
      cpu = get_cpu_of_thread (others);
      PARAVER_Startup (cpu->unique_number,
                       IDENTIFIERS (others),
                       current_time,
                       tmp_timer2);
      others->last_paraver = tmp_timer2;
    }
  }
}


/**************************************************************************
 ** Allibera tots els recursos reservat per fer l'operació col.lectiva,
 ** però no desbloqueja els threads perquè l'operació encara no ha acabat.
 **************************************************************************/
static void free_global_communication_resources (struct t_thread *thread)
{

  struct t_action               *action;
  struct t_bus_utilization      *bus_utilization;
  struct t_thread               *others, *wait_thread;
  struct t_communicator         *communicator;
  struct t_Ptask                *Ptask;
  struct t_global_op_definition *glop;
  struct t_node                 *node;
  struct t_machine              *machine;
  int comm_id, glop_id, i, aux, num_elements, num_maquines;


  Ptask        = thread->task->Ptask;
  action       = thread->action;
  comm_id      = action->desc.global_op.comm_id;
  communicator = locate_communicator (&Ptask->Communicator, comm_id);

  if (communicator == COM_NIL)
  {
    panic (
      "Communication free trough an invalid communicator %d to P%02d T%02d (t%02d)\n",
      comm_id,
      IDENTIFIERS (thread)
    );
  }

  glop_id = action->desc.global_op.glop_id;
  glop    = (struct t_global_op_definition *)
            query_prio_queue (&Global_op, (t_priority) glop_id);

  if (glop  ==  GOPD_NIL)
  {
    panic (
      "Global operation %d undefined to P%02d T%02d (t%02d)\n",
      glop_id,
      IDENTIFIERS (thread)
    );
  }

  if (debug & D_COMM)
  {
    PRINT_TIMER (current_time);
    printf (
      ": GLOBAL_operation P%02d T%02d (t%02d) Free resources used in '%s'\n",
      IDENTIFIERS (thread),
      glop->name
    );
  }

  /* Mirem el numero de maquines implicades abans de comenc,ar a
     desencuar threads. */
  num_maquines = count_queue (&communicator->machines_threads);

  /* Per cada maquina utilitzada s'alliberen tots els recursos reservats i
   * es treu el thread de la cua de maquines utilitzades per tal que la
   * cua ja quedi buida per la propera vegada. */
  /*  for (others = (struct t_thread *)head_queue(&communicator->machines_threads);
         others != TH_NIL;
         others = (struct t_thread *)next_queue(&communicator->machines_threads))
  */
  for (
    others  = (struct t_thread *) outFIFO_queue (&communicator->machines_threads);
    others != TH_NIL;
    others  = (struct t_thread *) outFIFO_queue (&communicator->machines_threads)
  )
  {
    /* S'obte la maquina corresponent al thread */
    node    = get_node_of_thread (others);
    machine = node->machine;

    /* S'alliberen els busos */
    others->number_buses             = 0;
    machine->network.curr_on_network = 0;

    for (i = 0; i < machine->communication.num_messages_on_network; i++)
    {
      bus_utilization = (struct t_bus_utilization *)
                        outFIFO_queue (&machine->network.threads_on_network);
    }
    /* Nomes s'ha reservat memoria una vegada i, per tant, nomes s'ha
       d'alliberar una vegada. */
    if (i > 0)
    {
      free (bus_utilization);
    }

    /* Si hi havia més d'una màquina, aqui cal alliberar els links
       de la xarxa externa d'aquesta maquina. */
    if (num_maquines > 1)
    {
      LINKS_free_wan_link(others->local_link, others);
      LINKS_free_wan_link(others->partner_link, others);
      /* En aquest cas cal posar explicitament els links a L_NIL perque
         en les operacions col.lectives no es treballa amb copies dels
         threads, sino amb els threads originals. */
      others->local_link = L_NIL;
      others->partner_link = L_NIL;
    }


    /* Intent d'alliberar els threads (No d'aquesta col.lectiva) que podien
       estar bloquejats esperant a que hi hagui busos lliures en aquesta
       maquina */
    if (count_queue (&machine->network.queue) > 0)
    {
      num_elements = count_queue (&machine->network.queue);
      for (
        i = 0;
        ( (i < num_elements) &&
          (machine->network.curr_on_network < machine->communication.num_messages_on_network) );
        i++
      )
      {
        wait_thread = (struct t_thread *) head_queue (&machine->network.queue);

        if (debug & D_COMM)
        {
          PRINT_TIMER (current_time);
          printf (
            ": GLOBAL_operation P%02d T%02d (t%02d) goes unlock and obtain bus\n",
            IDENTIFIERS (wait_thread)
          );
        }
        action = wait_thread->action;
        switch (action->action)
        {
        case SEND:
          /* FEC: S'acumula el temps que ha estat esperant busos */
          ACCUMULATE_BUS_WAIT_TIME (wait_thread);
          /*******************************************************/
          extract_from_queue (&machine->network.queue, (char *) wait_thread);
          really_send (wait_thread);
          break;

        case MPI_OS:
          /* FEC: S'acumula el temps que ha estat esperant busos */
          ACCUMULATE_BUS_WAIT_TIME (wait_thread);
          /*******************************************************/
          extract_from_queue (&machine->network.queue, (char *) wait_thread);
          really_RMA (wait_thread);
          break;

        case GLOBAL_OP:
          aux = machine->communication.num_messages_on_network -
                machine->network.curr_on_network;
          wait_thread->number_buses += aux;
          machine->network.curr_on_network += aux;
          if (wait_thread->number_buses == machine->communication.num_messages_on_network)
          {
            /* FEC: S'acumula el temps que ha estat esperant busos */
            ACCUMULATE_BUS_WAIT_TIME (wait_thread);
            /*******************************************************/
            extract_from_queue (&machine->network.queue, (char *) wait_thread);
            global_op_get_all_buses (wait_thread);
          }
          break;
        }
      }
    }
  }

  /* Ja s'han alliberat tots els recursos reservats, però encara NO s'han
     de desbloquejar els threads d'aquesta operació col.lectiva perquè
     l'operació encara no ha acabat. */
}

/**************************************************************************
 ** Desbloqueja tot els threads de l'operació col.lectiva. No allibera
 ** els recursos utilitzats, perquè ja s'ha d'haver fet a la rutina
 ** free_global_communication_resources.
 **************************************************************************/
static void close_global_communication (struct t_thread *thread)
{
  struct t_action               *action;
  struct t_thread               *others;
  struct t_communicator         *communicator;
  struct t_Ptask                *Ptask;
  struct t_global_op_definition *glop;
  struct t_node                 *node;
  struct t_account              *account;
  struct t_cpu                  *cpu;
  dimemas_timer                  tmp_timer;
  int comm_id, glop_id;
  int  machines;
  int *tasks_per_node;

  Ptask        = thread->task->Ptask;
  action       = thread->action;
  comm_id      = action->desc.global_op.comm_id;
  communicator = locate_communicator (&Ptask->Communicator, comm_id);

  if (communicator == COM_NIL)
  {
    panic ("Communication close trough an invalid communicator %d to P%02d T%02d (t%02d)\n",
           comm_id,
           IDENTIFIERS (thread) );
  }

  communicator->current_root = TH_NIL;
  communicator->in_flight_op = FALSE;

  /* Clean communicator queues */
  for (machines = 0; machines < Simulator.number_machines; machines++)
  {
    int *nodeid;

    for (nodeid  = (int*) outFIFO_queue (&communicator->nodes_per_machine[machines]);
         nodeid != NULL;
         nodeid  = (int*) outFIFO_queue (&communicator->nodes_per_machine[machines]))
    {
      free(nodeid);
    }

  }

  for (tasks_per_node  = (int*) outFIFO_queue(&communicator->tasks_per_node);
       tasks_per_node != NULL;
       tasks_per_node  =(int*) outFIFO_queue(&communicator->tasks_per_node))
  {
    free(tasks_per_node);
  }

  glop_id = action->desc.global_op.glop_id;
  glop    = (struct t_global_op_definition *)
            query_prio_queue (&Global_op, (t_priority) glop_id);

  if (glop == GOPD_NIL)
  {
    panic ("Global operation %d undefined to P%02d T%02d (t%02d)\n",
           glop_id,
           IDENTIFIERS (thread) );
  }

  if (debug & D_COMM)
  {
    PRINT_TIMER (current_time);
    printf (": GLOBAL_operation P%02d T%02d (t%02d) ends '%s'\n",
            IDENTIFIERS (thread),
            glop->name);
  }

  /* Unblock all threads involved in communication */
  for (others  = (struct t_thread *) outFIFO_queue (&communicator->threads);
       others != TH_NIL;
       others  = (struct t_thread *) outFIFO_queue (&communicator->threads) )
  {
    if (debug & D_COMM)
    {
      PRINT_TIMER (current_time);
      printf (": GLOBAL_operation P%02d T%02d (t%02d) ends '%s'\n",
              IDENTIFIERS (others),
              glop->name);
    }

    node = get_node_of_thread (others);
    new_cp_node (others, CP_BLOCK);
    new_cp_relation (others, thread);

    PARAVER_Wait (0,
                  IDENTIFIERS (others),
                  others->last_paraver,
                  current_time,
                  PRV_GLOBAL_OP_ST);

    ASS_ALL_TIMER (others->collective_timers.conclude_communication,
                   current_time);

    action = others->action;

    account = current_account (others);

    SUB_TIMER (others->collective_timers.with_resources,
               others->collective_timers.arrive_to_collective,
               tmp_timer);

    ADD_TIMER (account->block_due_group_operations,
               tmp_timer,
               account->block_due_group_operations);

    SUB_TIMER (others->collective_timers.conclude_communication,
               others->collective_timers.with_resources,
               tmp_timer);

    ADD_TIMER (account->group_operations_time,
               tmp_timer,
               account->group_operations_time);

    others->last_paraver = current_time;
    cpu                  = get_cpu_of_thread (others);

    /* JGG (2014/12/18): What is this event????
    PARAVER_Event (cpu->unique_number,
                   IDENTIFIERS (others),
                   current_time,
                   PARAVER_GROUP_FREE,
                   glop_id);
    */

    action         = others->action;
    others->action = action->next;
    READ_free_action(action);

    if (more_actions (others) )
    {
      others->loose_cpu = TRUE;
      SCHEDULER_thread_to_ready (others);
      reload_done       = TRUE;
    }
  }
}

static t_boolean thread_in_communicator (struct t_communicator *comm,
                                         struct t_thread       *thread)
{
  int  i;

  /*
  for (
    taskid  = (int *) head_queue (&comm->global_ranks);
    taskid != (int *) 0;
    taskid  = (int *) next_queue (&comm->global_ranks)
  )
  */
  for (i = 0; i < comm->size; i++)
  {
    // if (*taskid == (thread->task->taskid) )
    if ( thread->task->taskid == comm->global_ranks[i] )
    {
      return (TRUE);
    }
  }
  return (FALSE);
}

int from_rank_to_taskid (struct t_communicator *comm, int root_rank)
{
  int *root_task;
  int  i;

  /* TEST: Root_Rank is the Root Task ID
  root_task = (int *)head_queue(&comm->global_ranks);

  i = 0;
  while (i != root_rank)
  {
    root_task = (int *)next_queue(&comm->global_ranks);
    if (root_task==(int *)0)
    {
      panic(
        "Unable to localte root rank %d in communicator %d\n",
        root_rank,
        comm->communicator_id
      );
    }
    i++;
  }
  return (*root_task);
  */
  return root_rank;
}

void GLOBAL_operation (struct t_thread *thread,
                       int glop_id,
                       int comm_id,
                       int root_rank,
                       int root_thid,
                       int bytes_send,
                       int bytes_recv)
{
  struct t_Ptask                *Ptask;
  struct t_communicator         *communicator;
  struct t_global_op_definition *glop;
  struct t_thread               *others, *root_th;
  struct t_account              *account;
  struct t_cpu                  *cpu;
  struct t_node                 *node_usat;
  struct t_machine              *maquina_usada;

  struct t_queue                *nodes_per_machine;
  int                           *tasks_per_node;

  int root_task;
  int i, kind;


  /* not very clever mostly to eliminate warnings */
  assert (root_thid >= 0);
  assert (bytes_send >= 0);
  assert (bytes_recv >= 0);

  account = current_account (thread);
  account->n_group_operations++;

  Ptask = thread->task->Ptask;
  communicator = locate_communicator (&Ptask->Communicator, comm_id);
  if (communicator == COM_NIL)
  {
    panic ("Communication start trough an invalid communicator %d to P%02d T%02d (t%02d)\n",
           comm_id,
           IDENTIFIERS (thread) );
  }

  if (thread_in_communicator (communicator, thread) == FALSE)
  {
    panic ("Thread P%02d T%02d (t%02d) not valid for communicator %d\n",
           IDENTIFIERS (thread),
           comm_id);
  }

  glop = (struct t_global_op_definition *)
         query_prio_queue (&Global_op, (t_priority) glop_id);

  if (glop == GOPD_NIL)
  {
    panic ("Global operation %d undefined to P%02d T%02d (t%02d)\n",
           glop_id,
           IDENTIFIERS (thread) );
  }

  if (debug & D_COMM)
  {
    PRINT_TIMER (current_time);
    printf (
      ": GLOBAL_operation P%02d T%02d (t%02d) Starting %s (Root = %d)\n",
      IDENTIFIERS (thread),
      glop->name,
      root_rank
    );
  }

  if (communicator->in_flight_op != TRUE)
  {
    communicator->in_flight_op = TRUE;
  }

  /* Update the number of used nodes per machine and tasks per node */
  node_usat     = get_node_of_thread (thread);
  maquina_usada = node_usat->machine;
  // communicator->nodes_per_machine[maquina_usada->id]++;

  nodes_per_machine  = (struct t_queue*)
          query_prio_queue(&communicator->nodes_per_machine[maquina_usada->id],
                           (t_priority) node_usat->nodeid);

  if (nodes_per_machine == QU_NIL)
  {
    int* new_node     = (int*)            malloc(sizeof(int));

    (*new_node) = node_usat->nodeid;

    insert_queue(&communicator->nodes_per_machine[maquina_usada->id],
                 (char*) new_node,
                 (t_priority) node_usat->nodeid);
  }

  tasks_per_node = (int*) query_prio_queue(&communicator->tasks_per_node,
                                          (t_priority) node_usat->nodeid);
  if (tasks_per_node == NULL)
  {
    int* new_tasks_per_node = (int*) malloc(sizeof(int));

    (*new_tasks_per_node) = 1;
    insert_queue(&communicator->tasks_per_node,
                 (char*) new_tasks_per_node,
                 (t_priority) node_usat->nodeid);
  }
  else
  {
    (*tasks_per_node) += 1;
  }


  /*
  node_tasks = (int*) query_prio_queue (&communicator->node_tasks,
                                        (t_priority) node_usat->nodeid);

  if (node_tasks == NULL)
  {
    node_tasks  = malloc(sizeof(int));
    *node_tasks = 1;
    insert_queue(&communicator->node_tasks, node_tasks, node_usat->nodeid);
  }
  else
  {
    (*node_tasks) += 1;
  }
  */

  /* JGG (07/07/2010): New way to choose the root */
  if (root_rank == 1)
  {
    communicator->current_root = thread;

    if (debug & D_COMM)
    {
      PRINT_TIMER (current_time);
      printf (": GLOBAL_operation P%02d T%02d (t%02d) is the root\n",
              IDENTIFIERS (thread) );
    }
  }

  // Total_threads_involved = count_queue (&communicator->global_ranks);

  /* FEC: Debugant!

  printf(
    "El comid de l'operacio global es %d, te %d threads i %d estan bloquejats\n",
    comm_id,
    Total_threads_involved,
    count_queue(&communicator->threads)
  );
  */

  ASS_ALL_TIMER (thread->collective_timers.arrive_to_collective, current_time);

  // if (Total_threads_involved != count_queue (&communicator->threads) + 1)
  if ( communicator->size != count_queue (&communicator->threads) + 1)
  {
    /* This is not the last thread arriving to the communication point,
     * simply block */

    cpu = get_cpu_of_thread (thread);

    /* JGG (2014/12/18): What is this event????
    PARAVER_Event (cpu->unique_number,
                   IDENTIFIERS (thread),
                   current_time,
                   PARAVER_GROUP_BLOCK,
                   glop_id );
    */
    thread->last_paraver = current_time;

    inFIFO_queue (&communicator->threads, (char *) thread);

    if (debug & D_COMM)
    {
      PRINT_TIMER (current_time);
      printf (
        ": GLOBAL_operation P%02d T%02d (t%02d) Blocked on '%s' (Comm.%d: %dw / %dT)\n",
        IDENTIFIERS (thread),
        glop->name,
        comm_id,
        count_queue (&communicator->threads),
        communicator->size
      );
    }

    /* Add the current thread node */

    return;
  }

  /* All partners arrived. Start the contention stage */
  if (debug & D_COMM)
  {
    PRINT_TIMER (current_time);
    printf (
      ": GLOBAL_operation P%02d T%02d (t%02d) Initiate '%s' (Comm.%d: %dT)\n",
      IDENTIFIERS (thread),
      glop->name,
      comm_id,
      communicator->size
    );
  }

  cpu = get_cpu_of_thread (thread);

    /* JGG (2014/12/18): What is this event????
  PARAVER_Event (cpu->unique_number,
                 IDENTIFIERS (thread),
                 current_time,
                 PARAVER_GROUP_LAST,
                 glop_id);
  */

  thread->number_buses = 0;
  thread->last_paraver = current_time;

  ASS_ALL_TIMER (thread->collective_timers.sync_time, current_time);

  for (others  = (struct t_thread*) head_queue (&communicator->threads);
       others != TH_NIL;
       others  = (struct t_thread*) next_queue (&communicator->threads) )
  {
    ASS_ALL_TIMER (others->collective_timers.sync_time, current_time);
    PARAVER_Wait (0,
                  IDENTIFIERS (others),
                  others->last_paraver,
                  current_time,
                  PRV_BLOCKED_ST);

    others->last_paraver = current_time;
  }

  /*
  inFIFO_queue (&communicator->threads, (char*)thread);
  root_task = from_rank_to_taskid (communicator, root_rank);

  /* Es busca el thread de la root task
  for (
    others  = (struct t_thread *)head_queue(&communicator->threads);
    others != TH_NIL;
    others  = (struct t_thread *)next_queue(&communicator->threads)
  )
  {
    if (others->task->taskid == root_task)
    {
      break;
    }
  }

  if (others == TH_NIL)
  {
    panic("Unable to locate root %d for global operation\n", root_task);
    exit(EXIT_FAILURE);
  }
  /* El thread 'others' pertany a la 'root_task'
  root_th = others;
   */

  /* Insert current thread to the communicator list */
  inFIFO_queue (&communicator->threads, (char*) thread);

  /* Search the root task */
  if (communicator->current_root == TH_NIL)
  {
    /* Ja jan arribat tots els threads a aquest punt */
    if (debug & D_COMM)
    {
      PRINT_TIMER (current_time);
      printf (": GLOBAL_operation P%02d T%02d (t%02d) Root-less operation. Current thread will be the root\n",
              IDENTIFIERS (thread));
    }

    communicator->current_root = thread;

    root_th = thread;
    others  = thread;
  }
  else
  {
    root_th = communicator->current_root;
    others  = communicator->current_root;
  }

  if (external_comm_library_loaded == TRUE)
  {
    /* JGG (08/11/2004): Check 'external_get_global_op_type' to decide if
    its simulation will follow an external model */
    kind = external_get_global_op_type (root_th->action->desc.global_op.comm_id,
                                        root_th->action->desc.global_op.glop_id,
                                        root_th->action->desc.global_op.bytes_send,
                                        root_th->action->desc.global_op.bytes_recvd);

    if (kind == EXTERNAL_GLOBAL_OP_MODEL)
    {
      /* Aquí hay que iniciar el tratamiento del caso de una operación global
       * que sigue un modelo externo */
      /* SE DEBERÍA LLAMAR A UNA MODIFICACIÓN SOBRE EL 'start_global_op' PARA
       * EVITAR EL TIEMPO DE RESERVA DE RECURSOS */
      start_global_op(root_th, EXTERNAL_GLOBAL_OP_MODEL);
      return;
    }
  }

  /* JGG (08/11/2004): Comportamiento de una operación global según el
   * modelo Dimemas */


  /*****************************************************************************
    extract_from_queue (&communicator->threads, (char*)others);
    global_op_get_all_buses (others);

    En lloc de fer aixo (que root reservi tots el busos), a partir d'ara
    s'agafara un thread de cada maquina implicada per tal que agafi els
    busos de la seva maquina. De totes maneres, root sera l'encarregat
    d'agafar els de la seva maquina (tot i que no crec que calgui).
  *****************************************************************************/

  /* S'obte la maquina corresponent al thread de root */
  node_usat     = get_node_of_thread (others);
  maquina_usada = node_usat->machine;

  /* S'afegeix root a la llista de maquines utilitzades */
  insert_queue (&communicator->machines_threads,
                (char*) others,
                (t_priority) maquina_usada->id);

  /* Es treu de la llista de threads per poder detectar quan esta tot reservat*/
  extract_from_queue (&communicator->threads, (char *) others);

  /* Es fa el mateix amb el primer thread trobat de cada nova maquina */
  /* Aquest altre tipus de bucle no funcionava perque hi ha problemes si es
   * treu de la llista el primer element. En aquest cas el next retornava
   * NULL abans d'hora.
    for (others=(struct t_thread *)head_queue(&communicator->threads);
         others!=(struct t_thread *)0;
         others=(struct t_thread *)next_queue(&communicator->threads))
   */
  others = (struct t_thread*) head_queue (&communicator->threads);

  for (i = 0; i < (communicator->size - 1); i++)
  {
    /* S'obte la maquina corresponent a aquest thread */
    node_usat     = get_node_of_thread (others);
    maquina_usada = node_usat->machine;
    /* Es mira si ja hem trobat un thread d'aquesta maquina */
    if( query_prio_queue (&communicator->machines_threads,
                          (t_priority) maquina_usada->id) == A_NIL)
    {
      /* Es una maquina nova */
      /* S'afegeix el thread a la llista de maquines utilitzades */
      insert_queue (&communicator->machines_threads,
                    (char *) others,
                    maquina_usada->id);
      /* Es treu de la llista de threads per poder detectar quan esta tot reservat */
      extract_from_queue (&communicator->threads, (char *) others);
    }
    others = (struct t_thread*) next_queue (&communicator->threads);

    if ( (others == NULL) && (i < (communicator->size - 2) ) )
    {
      /* En aquest cas es deu haver extret el primer element de la llista,
       * pero encara hauria de quedar algun thread. Per tant, agafo el
       * primer que queda. */
      others = (struct t_thread*) head_queue (&communicator->threads);
    }
  }
  /* Un thread de cada maquina reserva els recursos */
  for (others  = (struct t_thread*) head_queue (&communicator->machines_threads);
       others != TH_NIL;
       others  = (struct t_thread*) next_queue (&communicator->machines_threads) )
  {
    /* Aquest thread reserva els recursos necessaris de la seva maquina */
    /* Aixo no es podia anar fent directament al bucle anterior perque
     * primer cal haver tret tots els threads que reserven busos, de la
     * cua "communicator->threads" perque sino no es pot detectar
     * correctament quan s'han reservat els recursos de totes les
     * maquines. */
    global_op_get_all_buses (others);
  }

  /* Aqui se suposa que ja haura intentat reservar tot el que cal i si
   * ho ha pogut fer, ja haura comenc,at la col.lectiva.  */

}
