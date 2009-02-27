char communic_c_rcsid[]="$Id: communic.c,v 1.41 2007/11/28 13:48:34 jgonzale Exp $";
/*
 * Communication routines
 *
 * Sergi Girona (sergi@ac.upc.es)
 *
 * (c) DAC-UPC 1993-94
 *
 */

#include "define.h"
#include "types.h"

#if defined(ARCH_MACOSX) || defined(ARCH_CYGWIN)
#include "macosx_limits.h"
#else
#include <values.h>
#endif
#include <math.h>

#include "sched_vars.h"
#include "cp.h"
#include "cpu.h"
#include "communic.h"
#include "extern.h"
#include "events.h"
#include "links.h"
#include "list.h"
#include "mallocame.h"
#include "paraver.h"
#include "random.h"
#include "schedule.h"
#include "sddf.h"
#include "subr.h"
#include "task.h"
#include "vampir.h"

#include "venusclient.h"


/******************************************************************************
 * Global variables                                                           *
 *****************************************************************************/

t_boolean DATA_COPY_enabled;      /* True if data copy latency is enabled */
int       DATA_COPY_message_size; /* Maximun message size to compute data copy
                                   * latency */

t_boolean RTT_enabled; /* True if Round Trip Time is enabled */
t_micro   RTT_time;    /* Round Trip Time for messages greater than eager */

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

static void
global_op_get_all_buses (struct t_thread *thread);

static void
global_op_get_all_out_links (struct t_thread *thread);

static void
global_op_get_all_in_links (struct t_thread *thread);

/* void
close_global_communication(struct t_thread *thread); */

static void
read_communication_config_file(FILE *fi, char *filename);

static void
get_machine_flight_times(int machine_id, char *buffer, char *filename);

static void
get_global_OP_parameters (
  char *filename,
  int machine_id,
  int global_OP,
  char *FIN_model,
  char *FIN_size,
  char *FOUT_model,
  char *FOUT_size
);

static void
start_global_op (struct t_thread *thread);

static void
free_global_communication_resources (struct t_thread *thread);

static void
close_global_communication(struct t_thread *thread);

struct t_queue Global_op;

t_micro
compute_startup(struct t_thread               *thread,
                int                            kind,
                struct t_node                 *node,
                struct t_dedicated_connection *connection);

t_micro
compute_copy_latency(struct t_thread *thread,
                     struct t_node   *node,
                     int              mess_size);


/******************************************************************************
 * FUNCIÓ 'compute_startup'                                                   *
 *****************************************************************************/
t_micro
compute_startup(struct t_thread                *thread,
                int                             kind,
                struct t_node                  *node,
                struct t_dedicated_connection  *connection)
{
  t_micro startup = (t_micro) 0;

  switch (kind)
  {
    case LOCAL_COMMUNICATION_TYPE:
      /* Es un missatge local al node */
      startup = node->local_startup;
      if (randomness.memory_latency.distribution != NO_DISTRIBUTION)
        startup += random_dist(&randomness.memory_latency);
      break;
    case INTERNAL_NETWORK_COM_TYPE:
      /* Es un missatge de la xarxa interna a la maquina */
      startup = node->remote_startup;
      if (randomness.network_latency.distribution != NO_DISTRIBUTION)
        startup += random_dist(&randomness.network_latency);
      break;
    case EXTERNAL_NETWORK_COM_TYPE:
      /* Es un missatge entre dues maquines diferents per la xarxa externa. */
      startup = node->external_net_startup;
      if (randomness.external_network_latency.distribution != NO_DISTRIBUTION)
        startup += random_dist(&randomness.external_network_latency);
      break;
    case DEDICATED_CONNECTION_COM_TYPE:
      /* Es un missatge entre dues maquines diferents per una connexio
       * dedicada. */
      if (connection == NULL)
      {
        panic("Error computing startup (P%02 T%02d t%02d) : void connection \n",
              IDENTIFIERS(thread));
      }
      startup = connection->startup;
      break;
    default:
      panic ("Error computing startup (P%02 T%02d t%02d): unknown comm type %d\n",
             IDENTIFIERS(thread),
             kind);
      break;
  }
  return (startup);
}
/******************************************************************************
 * FUNCIÓ 'compute_copy_time'                                                 *
 *****************************************************************************/
t_micro
compute_copy_latency(struct t_thread *thread,
                     struct t_node   *node,
                     int              mess_size)
{
  t_micro bw;
  t_micro copy_latency = (t_micro) 0;

  if (node == NULL)
  {
    panic("Error computing copy latency: void node descriptor\n");
  }

  if (node->bandwith != (t_micro) 0)
  {
    bw = (t_micro) ((t_micro) (1000000) / (1 << 20) / node->bandwith);
    copy_latency = bw * mess_size;
  }

  return (copy_latency);
}

/******************************************************************************
 * FUNCIÓ 'recompute_bandwith'                                                *
 *****************************************************************************/
static t_micro
recompute_bandwith(struct t_thread *thread)
{
  t_micro bandw;
  t_micro ratio;
  t_micro interm;
  struct t_thread *pending;
  dimemas_timer tmp_timer;
  dimemas_timer inter;
  int pending_bytes;
  struct t_bus_utilization *bus_utilization;
  struct t_node    *node;
  struct t_machine *machine;

  node = get_node_of_thread (thread);
  machine = node->machine;

  bandw = (t_micro) machine->communication.remote_bandwith;
  if (randomness.network_bandwidth.distribution != NO_DISTRIBUTION)
  {
    bandw += random_dist(&randomness.network_bandwidth);
  }

  if (bandw!=0)
  {
    bandw = (t_micro) ((t_micro) (1000000) / (1 << 20) / bandw);
  }

  if (machine->network.curr_on_network <=
      machine->communication.num_messages_on_network)
  {
    ratio = 1.0;
  }
  else
  {
    ratio = ((t_micro)machine->communication.num_messages_on_network)/
            ((t_micro)machine->network.curr_on_network);
  }

  if (ratio != 0)
  {
    bandw = bandw / ratio;
  }
  else
  {
    panic("bandw = 0 !\n");
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
        pending->last_comm.bytes - (interm/pending->last_comm.bandwith);

      pending->last_comm.bandwith = bandw;
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
  return(bandw);
}

/******************************************************************************
 * VARIABLES GLOBALS PEL CALCUL DEL COMPORTAMENT DE LA XARXA EXTERNA          *
 *****************************************************************************/

/* Variables per poder anar calculant el trafic que generem a la
 * xarxa externa. */
static double suma_missatges_xarxa_externa      =0.0;
static double increment_missatges_xarxa_externa =0.0;


/* Parametres per poder provar el calcul del trafic de la xarxa externa.
 * Un cop establerts correctament, es podrien convertir en defines i eliminar
 * les rutines que els llegeixen d'un fitxer. */
static double param_external_net_alfa    = 0.1;              /* Ha de ser < 1 */
static double param_external_net_periode = 86400000000.0;   /* En microsegons */
static double param_external_net_beta    = 0.0; /* Coeficients que determinen */
static double param_external_net_gamma   = 1.0; /* la influencia dels traffics*/

/******************************************************************************
 * PROCEDURE 'get_param_external_net_traffic'                                 *
 *****************************************************************************/
/*
 * Intenta obtenir els parametres que definexen el traffic de la xarxa externa.
 * Això només s'ha de fer així temporalment. Un cop definits correctament,
 * s'haurien de posar en un #DEFINE i no tornar-los a tocar.
 */
static void
get_param_external_net_traffic()
{
  FILE  *fd;
  char   buff[BUFSIZE];
  int    i;
  double aux;
  char  *sub_buff;

  /* S'obre el fitxer dels parametres */
  fd = MYFOPEN("parametres_traffic.cfg","r");
  if (fd == NULL) return;

  /* Es llegeixen caracters suficients per tots els parametres */
  i       = fread(buff, 1, 255, fd);
  buff[i] = '\0';

  /* S'identifiquen els parametres existents */

  /* alfa */
  sub_buff = strstr(buff, "alfa=");
  if (sub_buff != 0)
  {
    i = sscanf(sub_buff, "alfa=%lf", &aux);
    if (i == 1)
    {
      param_external_net_alfa = aux;
    }
  }
  /* periode */
  sub_buff = strstr(buff, "periode=");
  if (sub_buff != 0)
  {
    i = sscanf(sub_buff, "periode=%lf", &aux);
    if (i == 1)
    {
      param_external_net_periode = aux;
    }
  }
  /* beta */
  sub_buff = strstr(buff, "beta=");
  if (sub_buff != 0)
  {
    i = sscanf(sub_buff, "beta=%lf", &aux);
    if (i == 1)
    {
      param_external_net_beta = aux;
    }
  }
  /* gamma */
  sub_buff = strstr(buff, "gamma=");
  if (sub_buff != 0)
  {
    i = sscanf(sub_buff, "gamma=%lf", &aux);
    if (i == 1)
    {
      param_external_net_gamma = aux;
    }
  }

  /* Es tanca el fitxer */
  fclose(fd);

  /* Es mosta com han quedat els parametres */
  if (debug&D_COMM)
  {
    PRINT_TIMER (current_time);
    printf (
      ": COMMUNIC\t'get_param_external_net_traffic' alfa=%.2f, periode=%.2f, beta=%.2f, gamma=%.2f\n",
      param_external_net_alfa,
      param_external_net_periode,
      param_external_net_beta,
      param_external_net_gamma
    );
  }
}

/******************************************************************************
 * PROCEDURE 'periodic_external_network_traffic_init'                         *
 *****************************************************************************/
/*
 * Aquesta funcio inicialitza el calcul del traffic de la xarxa externa
 */
void
periodic_external_network_traffic_init()
{

  dimemas_timer tmp_timer;

  suma_missatges_xarxa_externa      = 0.0;
  increment_missatges_xarxa_externa = 0.0;

  /* Es prepara la primera execucio de la rutina que cal executar periodicament
   * per recalcular el traffic de la xarxa externa no produit per l'aplicacio
   * que s'esta simulant. */
  if (venus_enabled) {
    ADD_TIMER (1e6, current_time, tmp_timer); /* grodrigu: 10e6 is too much when interfaced with Venus */
  }
  else {
    ADD_TIMER (10e6, current_time, tmp_timer); /* grodrigu: 10e6 is too much when interfaced with Venus */
  }
  EVENT_timer (tmp_timer, NOT_DAEMON, M_COM, NULL, COM_EXT_NET_TRAFFIC_TIMER);

  /* Intenta llegir els parametres que determinen el traffic */
  get_param_external_net_traffic();
}

/******************************************************************************
 * PROCEDURE 'recompute_external_network_traffic'                             *
 *****************************************************************************/
/*
 * Aquesta funcio s'hauria de cridar cada vegada que s'envia un missatge nou a
 * traves de la xarxa externa.
 */
void
recompute_external_network_traffic(int mess_size)
{
  increment_missatges_xarxa_externa += mess_size;
}

/******************************************************************************
 * PROCEDURE 'recompute_external_network_traffic'                             *
 *****************************************************************************/
/*
 * Aquesta funcio s'hauria d'executar periodicament. En principi cada 10 segons.
 */
void
periodic_recompute_external_network_traffic()
{

  dimemas_timer tmp_timer;

  suma_missatges_xarxa_externa =
    suma_missatges_xarxa_externa * param_external_net_alfa +
    (increment_missatges_xarxa_externa / (10*1000000)) *
    (1 - param_external_net_alfa);

  if (debug&D_COMM)
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
  if ((top_event (&Event_queue) != E_NIL) || (venus_enabled && (top_event(&Interactive_event_queue) != E_NIL))) /* grodrigu: venus! */
  {
    /* Si queden events es que cal seguir simulant, per tant, es pot encuar
       aquest event. Si no en quedessin no el podriem encuar perque es
       seguiria simulant sense que hi hagues cap altre event que aquests. */
    if (venus_enabled) {
      ADD_TIMER (1e6, current_time, tmp_timer); /* grodrigu: 10e6 is too much when interfaced with Venus */
    }
    else {
      ADD_TIMER (10e6, current_time, tmp_timer); /* grodrigu: 10e6 is too much when interfaced with Venus */
    }
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
double
external_network_general_traffic(dimemas_timer temps)
{
  double traffic;


  /* Per fer alguna cosa hi poso aixo: */
  double aux;
  /* Per tenir un periode d'un dia */
  aux = ((double)((unsigned long long) temps %
        ((long) param_external_net_periode + 1)) / param_external_net_periode);

  traffic = (sin(aux * 2 * M_PI) + 1) / 2; /* Aqui traffic esta entre 0 i 1 */
  /* Aquesta funcio ha de retornar un numero entre 0 i
     l'ample de banda maxim de la xarxa externa. */
  traffic = traffic *
            (sim_char.general_net.bandwidth * (1 << 20) / (t_micro) (1000000));

  if (debug&D_COMM)
  {
    PRINT_TIMER (current_time);
    printf (
      ": COMMUNIC\tExternal Network Traffic = %.4f\n",
      traffic
    );
  }
  return(traffic);
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
  return(suma_missatges_xarxa_externa);
}

/******************************************************************************
 * FUNCIÓ 'external_network_bandwidth_ratio'                                  *
 *****************************************************************************/
/*
 * Retorna el valor del ratio de l'ample de banda en funció del tràfic
 * existent, i la modelització del mateix
 */
double
external_network_bandwidth_ratio(double traffic)
{
  double ratio;

  /* El parametre traffic pot ser mes gran que l'ample de banda
     maxim de la xarxa externa. */

  switch (sim_char.general_net.traffic_function)
  {
    case EXP: /* Comportament Exponencial */
      /*ratio=pow(M_E, (-5*traffic/sim_char.general_net.max_traffic_value));*/
      if (sim_char.general_net.max_traffic_value != 0)
      {
        ratio = exp(-5*traffic/sim_char.general_net.max_traffic_value);
      }
      else
      {
        ratio = 1;
      }
      break;
    case LOG: /* Comportament logaritmic */
      if (sim_char.general_net.max_traffic_value != 0)
      {
        ratio = log10(10-(10*traffic/sim_char.general_net.max_traffic_value));
      }
      else
      {
        ratio=1;
      }
      break;
    case LINEAL: /* Comportament linial */
      if ((sim_char.general_net.max_traffic_value != 0) && (traffic != 0))
      {
        if (traffic < sim_char.general_net.max_traffic_value)
        {
          ratio = (sim_char.general_net.max_traffic_value - traffic) /
                   sim_char.general_net.max_traffic_value;
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
 * FUNCIÓ 'bandwidth_2_ms_per_byte'                                           *
 *****************************************************************************/
/*
 * Aquesta funció passa de Mbytes a bytes, de segons a microsegons i calcula la
 * inversa. Aixi nomes cal multiplicar per la mida del missatge per obtenir el
 * temps estimat de transferencia. El resultat sera microsegons/byte.
 */
static t_micro
bandwidth_2_ms_per_byte(t_micro bandw)
{
  t_micro bandw_ms_per_byte = 0;

  if (bandw != 0)
  {
    bandw_ms_per_byte = (t_micro) ((t_micro) (1000000) / (1 << 20) / bandw);
  }

  return bandw_ms_per_byte;
}

/******************************************************************************
 * FUNCIÓ 'recompute_external_network_bandwidth'                              *
 *****************************************************************************/
static t_micro
recompute_external_network_bandwidth(struct t_thread *thread)
{
  t_micro bandw;
  t_micro bandw_ms_per_byte;
  t_micro ratio;
/*
  t_micro interm;
  struct t_thread *pending;
  dimemas_timer tmp_timer;
  dimemas_timer inter;
  int pending_bytes;
  struct t_bus_utilization *bus_utilization;
*/
  double traffic, traffic_indep, traffic_aplic;

  /* El primer que cal fer es calcular el traffic per poder indexar la funcio
   * que ens determinara el coeficient de l'ample de banda maxim que cal agafar
   * (el ratio). */

  /* Es calcula el traffic de la xarxa externa independent de nosaltres */
  traffic_indep = external_network_general_traffic(current_time);

  /* S'obte el traffic que hi ha a la xarxa per culpa de la propia
     aplicacio que estem simulant */
  traffic_aplic = external_network_application_traffic();

  /* Es calcula el traffic total */
  traffic = param_external_net_gamma * traffic_indep +
            param_external_net_beta  * traffic_aplic;

  /* Es calcula el ratio en funcio del traffic */
  ratio = external_network_bandwidth_ratio(traffic);

  /* S'obte l'ample de banda maxim */
  bandw = (t_micro) sim_char.general_net.bandwidth;

  /* FEC: Aixo segurament ja no cal. */
  if (randomness.external_network_bandwidth.distribution != NO_DISTRIBUTION)
  {
    bandw += random_dist(&randomness.external_network_bandwidth);
  }

  /* Es calcula l'ample de banda final */
  if (ratio != 0)
  {
    bandw = bandw * ratio;
  }
  else
  {
    panic("External network ratio equals 0\n");
  }

  bandw_ms_per_byte = bandwidth_2_ms_per_byte(bandw);

  if (debug&D_COMM)
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
                      head_queue(&sim_char.general_net.threads_on_network);
    bus_utilization != BU_NIL;
    bus_utilization = (struct t_bus_utilization *)
                      next_queue(&sim_char.general_net.threads_on_network)
  )
  {
    if (bus_utilization->sender != thread)
    {
      pending = bus_utilization->sender;
      EVENT_extract_timer (M_COM, pending, &tmp_timer);
      SUB_TIMER (current_time, pending->last_comm.ti, inter);
      TIMER_TO_FLOAT (inter, interm);
      pending_bytes =
        pending->last_comm.bytes -(interm/pending->last_comm.bandwith);

      pending->last_comm.bandwith = bandw_ms_per_byte;
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
  return(bandw);
}

/******************************************************************************
 * FUNCIÓ 'locate_receiver'                                                   *
 *****************************************************************************/
/*
 * Find a receiver thread in queue asking for mess_tag
 */
static struct t_thread*
locate_receiver(
  struct t_queue *threads,
  int taskid_ori,
  int mess_tag,
  int communic_id
)
{
  struct t_thread *thread;
  struct t_action *action;
  struct t_recv  *mess;

  for (
    thread  = (struct t_thread *) head_queue (threads);
    thread != TH_NIL;
    thread  = (struct t_thread *) next_queue (threads)
  )
  {
    action = thread->action;
    mess   = &(action->desc.recv);

    if ( (mess->mess_tag    == mess_tag)  &&
         (mess->communic_id == communic_id) &&
         ((mess->ori == taskid_ori) || (mess->ori == -1))
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
static void
message_received (struct t_thread *thread)
{
  struct t_node    *node, *node_partner;
  struct t_task    *task, *task_partner;
  struct t_action  *action;
  struct t_send    *mess;
  struct t_thread  *partner;
  struct t_account *account;
  dimemas_timer     tmp_timer;
  struct t_recv    *mess_recv;
  struct t_cpu     *cpu_partner, *cpu;

  node   = get_node_of_thread (thread);
  task   = thread->task;
  action = thread->action;
  mess   = &(action->desc.send);

  task_partner = locate_task (task->Ptask, mess->dest);
  node_partner = get_node_of_task (task_partner);

  thread->physical_recv = current_time;

  partner =
    locate_receiver (
      &(task_partner->recv),
      task->taskid,
      mess->mess_tag,
      mess->communic_id
    );

  if (partner == TH_NIL)
  {
    /* El thread corresponent no esta bloquejat esperant a rebre.
     * Cal mirar si esta fent espera activa. */
    for (
      partner  = (struct t_thread *) head_queue (&(task_partner->threads));
      partner != TH_NIL;
      partner  = (struct t_thread *) next_queue (&(task_partner->threads))
    )
    {
      if ((is_thread_running (partner)) && (partner->doing_busy_wait))
      {
        action    = partner->action;
        mess_recv = &(action->desc.recv);
        if (
          (mess_recv->mess_tag == mess->mess_tag) &&
          (mess_recv->communic_id == mess->communic_id) &&
          ((mess_recv->ori == task->taskid) || (mess_recv->ori == -1))
        )
        {
          partner->doing_busy_wait    = FALSE;
          cpu_partner                 = get_cpu_of_thread (partner);
          cpu_partner->current_thread = TH_NIL;

          EVENT_extract_timer (M_SCH, partner, &tmp_timer);

          Paraver_thread_buwa (
            cpu_partner->unique_number,
            IDENTIFIERS (partner),
            partner->last_paraver,
            current_time
          );
          partner->last_paraver = current_time;
          if (debug&D_COMM)
          {
            PRINT_TIMER (current_time);
            printf (
              ": COMMUNIC_send\tP%02d T%02d (t%02d) -> T%d Busy Wait\n",
              IDENTIFIERS (thread),
              mess->dest
            );
          }
          account = current_account (partner);
          account->n_bytes_recv += mess->mess_size;
          cpu = get_cpu_of_thread(thread);

          Paraver_comm (
            cpu->unique_number,
            IDENTIFIERS (thread),
            thread->logical_send,
            thread->physical_send,
            cpu_partner->unique_number,
            IDENTIFIERS (partner),
            partner->logical_recv,
            thread->physical_recv,
            mess->mess_size,
            mess->mess_tag
          );

          partner->last_paraver = current_time;
          action = partner->action;
          partner->action = action->next;
          freeame ((char *) action, sizeof (struct t_action));

          if (more_actions (partner))
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
    /* El thread corresponent encara no esta a punt per rebre el missatge.
     * El que cal fer es encuar el thread a la cua de missatges rebuts de
     * la task corresponent. */
    if (debug&D_COMM)
    {
      PRINT_TIMER (current_time);
      printf (
        ": COMMUNIC_send\tP%02d T%02d (t%02d) -> T%d Tag(%d) Receiver Not Ready\n",
        IDENTIFIERS (thread),
        mess->dest,
        mess->mess_tag
      );
    }
    inFIFO_queue (&(task_partner->mess_recv), (char *) thread);
    SDDF_in_message (mess->mess_size);
  }
  else
  {
    /* El thread corresponent esta bloquejat esperant a rebre. Per tant,
     * s'haura de desbloquejar. */
    Activity_Exit (FALSE,partner, 2);
    Vampir_Receive (
      task_partner->taskid,
      thread->task->taskid,
      mess->communic_id,
      mess->mess_tag,
      mess->mess_size
    );

    SDDF_recv_stop (
      node_partner->nodeid,
      task_partner->taskid,
      current_time,
      mess->mess_tag,
      mess->mess_size,
      node->nodeid,
      thread->task->taskid
    );
    SDDF_in_message (mess->mess_size);

    account = current_account (partner);
    account->n_bytes_recv += mess->mess_size;
    SUB_TIMER (current_time, partner->start_wait_for_message,tmp_timer);
    ADD_TIMER (
      account->time_waiting_for_message,
      tmp_timer,
      account->time_waiting_for_message
    );
    extract_from_queue (&(task_partner->recv), (char *) partner);

    Paraver_thread_wait (
      0,
      IDENTIFIERS (partner),
      partner->last_paraver,
      current_time,
      PRV_BLOCKING_RECV_ST
    );

    new_cp_node (partner, CP_BLOCK);
    new_cp_relation (partner, thread);
    cpu = get_cpu_of_thread(thread);
    cpu_partner = get_cpu_of_thread (partner);

    Paraver_comm (
      cpu->unique_number,
      IDENTIFIERS (thread),
      thread->logical_send,
      thread->physical_send,
      cpu_partner->unique_number,
      IDENTIFIERS (partner),
      partner->logical_recv,
      thread->physical_recv,
      mess->mess_size,
      mess->mess_tag
    );
    partner->last_paraver = current_time;

    action          = partner->action;
    partner->action = action->next;
    freeame ((char *) action, sizeof (struct t_action));
    if (more_actions (partner))
    {
      partner->loose_cpu = TRUE;
      SCHEDULER_thread_to_ready (partner);
      SCHEDULER_general (SCH_NEW_JOB, partner);
    }
    /* FEC: No es pot borrar el thread aqui perque encara es necessitara
            quan es retorni a la crida d'on venim. Nomes es marca com a
            pendent d'eliminar i ja s'esborrara mes tard. */
    thread->marked_for_deletion = 1;

    if (debug&D_COMM)
    {
      PRINT_TIMER (current_time);
      printf (
        ": COMMUNIC_send\tP%02d T%02d (t%02d) -> T%d Tag(%d) Receiver Unlocked\n",
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
static t_boolean
is_message_awaiting(
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

  for (
    thread_source = (struct t_thread *) head_queue (&(task->mess_recv)),
      found = FALSE;
	  thread_source != TH_NIL && !found;
	  thread_source = (struct t_thread *) next_queue (&(task->mess_recv))
  )
  {
    task_source = thread_source->task;
    action      = thread_source->action;
    mess_source = &(action->desc.send);
    if (debug&D_COMM)
    {
      PRINT_TIMER (current_time);
      printf (
        ": COMMUNIC_wait/recv\tP%02d T%02d (t%02d) <- %d Tag(%d)\n",
        IDENTIFIERS (thread),
        task_source->taskid,
        mess_source->mess_tag
      );
    }

    if ( ((mess->ori == -1) || (task_source->taskid == mess->ori)) &&
	       (mess_source->mess_tag == mess->mess_tag) &&
	       (mess_source->communic_id == mess->communic_id)
    )
    {
      account = current_account (thread);
      account->n_bytes_recv += mess_source->mess_size;
      extract_from_queue (&(task->mess_recv), (char *) thread_source);
      s_node = get_node_of_thread (thread_source);
      r_node = get_node_of_thread (thread);

      SCHEDULER_info (
        COMMUNICATION_INFO,
        SCH_INFO_RECV_HIT,
        thread_source, thread
      );
      cpu         = get_cpu_of_thread(thread_source);
      cpu_partner = get_cpu_of_thread(thread);

      Paraver_comm (
        cpu->unique_number,
        IDENTIFIERS (thread_source),
        thread_source->logical_send,
        thread_source->physical_send,
        cpu_partner->unique_number,
        IDENTIFIERS (thread),
        thread->logical_recv,
        thread_source->physical_recv,
        mess_source->mess_size,
        mess_source->mess_tag
      );

      new_cp_relation (thread, thread_source);
      thread->last_paraver = current_time;

      SDDF_recv_stop (
        r_node->nodeid,
        thread->task->taskid,
        current_time,
        mess->mess_tag,
        mess->mess_size,
        s_node->nodeid,
        thread_source->task->taskid
      );
      SDDF_in_message (mess_source->mess_size);

      delete_duplicate_thread (thread_source);

      result = TRUE;
      found  = TRUE;
    }
  }

  task_source   = locate_task (task->Ptask, mess->ori);
  thread_source = (struct t_thread *) head_queue (&(task_source->threads));
  SCHEDULER_info (
    COMMUNICATION_INFO,
    SCH_INFO_RECV_MISS,
    thread_source,
    thread
  );

  return result;
}




/******************************************************************************
 * PROCEDURE 'Start_communication_if_partner_ready_for_rendez_vous'           *
 *****************************************************************************/
static void
Start_communication_if_partner_ready_for_rendez_vous (
  struct t_thread *thread,
  struct t_recv *mess
)
{
  register struct t_thread *sender;
  register struct t_task *task_sender;
  struct t_send *mess_sender;
  register struct t_action *action;
  struct t_thread *copy_thread;
  struct t_Ptask *ptask;
  int trobat;


  if (mess->ori != -1)
  {
    task_sender = locate_task (thread->task->Ptask, mess->ori);
    if (task_sender==T_NIL)
    {
      panic (
        "Unable to locate task %d in Ptask %d\n",
        mess->ori,
        thread->task->Ptask->Ptaskid
      );
    }
    for (sender = (struct t_thread *) head_queue (&(task_sender->send));
         sender != TH_NIL;
         sender = (struct t_thread *) next_queue (&(task_sender->send)))
    {
      action = sender->action;
      mess_sender = &(action->desc.send);
      if ((mess_sender->mess_tag == mess->mess_tag)  &&
          (mess_sender->communic_id==mess->communic_id) &&
          ((mess_sender->dest == -1) || (mess_sender->dest == thread->task->taskid))) /* match receiver also! */
      {
         break;
      }
    }
  }
  else /* mess->ori == ANY */
  {
    ptask  = thread->task->Ptask;
    trobat = FALSE;
    for(task_sender=(struct t_task *) head_queue (&(ptask->tasks));
        task_sender!=T_NIL && trobat == FALSE;
        task_sender=(struct t_task *) next_queue (&(ptask->tasks)))
    {
      for (sender = (struct t_thread *) head_queue (&(task_sender->send));
           sender != TH_NIL && trobat == FALSE;
           sender = (struct t_thread *) next_queue (&(task_sender->send)))
      {
        action = sender->action;
        mess_sender = &(action->desc.send);
        if ((mess_sender->mess_tag == mess->mess_tag)  &&
            (mess_sender->communic_id==mess->communic_id) &&
            (mess->ori == task_sender->taskid))
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

  extract_from_queue (&(task_sender->send), (char *)sender);
  /*  Si el thread és una copia és que s'està fent el rendez vous en
      un altre thread, per tant, no s'ha de generar res a la traça. */
  if (sender->original_thread)
  {
    Paraver_thread_wait (
      0,
      IDENTIFIERS (sender),
      sender->last_paraver,
      current_time,
      PRV_BLOCKING_RECV_ST
    );

    sender->last_paraver = current_time;
    copy_thread = duplicate_thread (sender);
  }
  else
  {
    copy_thread = sender;
  }

  if (debug&D_COMM)
  {
    PRINT_TIMER(current_time);
    printf(
      ": RENDEZ VOUS\tP%02d T%02d (t%02d) <- P%02d T%02d (t%02d)\n",
      IDENTIFIERS(thread),
      IDENTIFIERS(copy_thread)
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
    freeame ((char *) action, sizeof (struct t_action));
    if (more_actions (sender))
    {
      sender->loose_cpu = FALSE;
      SCHEDULER_thread_to_ready (sender);
    }
  }
}


/******************************************************************************
 * FUNCTION 'COMMUNIC_internal_network_COM_TIMER_OUT'                         *
 *****************************************************************************/
struct t_thread*
COMMUNIC_internal_network_COM_TIMER_OUT(struct t_thread *thread)
{
  /* FEC: Es treu aixo per separar l'alliberació dels recursos utilitzats ******

  register struct t_thread *wait_thread;
  register struct t_bus_utilization *bus_utilization;
  struct t_node *node;
  struct t_machine *machine;
  int aux;
  *****************************************************************************/
  struct t_thread *copy_thread;

  /* FEC: Es treu aixo per separar l'alliberació dels recursos utilitzats ******
  node = get_node_of_thread (thread);
  machine = node->machine;

  if (machine->communication.num_messages_on_network)
  {
    for (
      bus_utilization  = (struct t_bus_utilization *)
        head_queue(&machine->network.threads_on_network);
      bus_utilization != BU_NIL;
      bus_utilization  = (struct t_bus_utilization *)
        next_queue(&machine->network.threads_on_network)
    )
    {
      if (bus_utilization->sender==thread) break;
    }
    if (bus_utilization==BU_NIL)
    {
      panic ("Unable to locate in bus utilization queue\n");
    }

    extract_from_queue (
      &machine->network.threads_on_network,
      (char *)bus_utilization
    );
    freeame ((char *) bus_utilization, sizeof(struct t_bus_utilization));
  }

  free_link (thread->local_link, thread);

  if (debug&D_COMM)
  {
    PRINT_TIMER (current_time);
    printf (
      ": COMMUNIC\tP%02d T%02d (t%02d) Free local link\n",
      IDENTIFIERS (thread)
    );
	}
  *****************************************************************************/

  if (thread->original_thread)
  {
    copy_thread = duplicate_thread(thread);
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

  /* FEC: Es treu aixo per separar l'alliberació dels recursos utilitzats ******

  free_link (thread->partner_link, thread);

  if (debug&D_COMM)
  {
    PRINT_TIMER (current_time);
    printf (
      ": COMMUNIC\tP%02d T%02d (t%02d) Free Remote Link\n",
      IDENTIFIERS (thread)
    );
  }

  if (machine->network.curr_on_network > 0)
  {
    machine->network.curr_on_network--;
  }
  *****************************************************************************/

  switch (thread->action->action)
  {
    case SEND:
      message_received (copy_thread);
      if (venus_enabled) {
        venusmsgs_in_flight--;
      }
      break;
    case MPI_OS:
      os_completed (copy_thread);
      break;
  }

  /* FEC: Es treu aixo per separar l'alliberació dels recursos utilitzats
   * recompute_bandwith(thread);

  if ((machine->communication.num_messages_on_network) &&
      (count_queue (&machine->network.queue) > 0))
  {
    wait_thread = (struct t_thread *) head_queue (&machine->network.queue);
#ifdef PARAVER_ALL
    Paraver_event (
      1,
      1,
      1,
      1,
      current_time,
      70,
      count_queue (&machine->network.queue)
    );
#endif
	  if (debug&D_COMM)
    {
      PRINT_TIMER (current_time);
      printf(
        ": COMMUNIC\tP%02d T%02d (t%02d) Obtain Bus!\n",
        IDENTIFIERS (wait_thread)
      );
	  }

    switch (wait_thread->action->action)
    {
      case SEND:
        extract_from_queue(&machine->network.queue, (char *) wait_thread);
        really_send (wait_thread);
        break;
      case MPI_OS:
        extract_from_queue(&machine->network.queue, (char *) wait_thread);
        really_RMA (wait_thread);
        break;
      case GLOBAL_OP:
        /* aux sempre hauria de ser 1 */
        /* FEC: Es treu aixo x separar l'alliberació dels recursos utilitzats
        aux = machine->communication.num_messages_on_network-
              machine->network.curr_on_network;
        wait_thread->number_buses+=aux;
        machine->network.curr_on_network+=aux;

        if (wait_thread->number_buses ==
            machine->communication.num_messages_on_network
        )
        {
          extract_from_queue(&machine->network.queue, (char *) wait_thread);
          global_op_get_all_buses(wait_thread);
        }
        break;
		}
	}
  *****************************************************************************/
  return(copy_thread);
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
struct t_thread*
COMMUNIC_external_network_COM_TIMER_OUT(struct t_thread *thread)
{
  struct t_thread *copy_thread;

  /* FEC: Es treu aixo per separar l'alliberació dels recursos utilitzats	******

  register struct t_bus_utilization *bus_utilization;

  for (
    bus_utilization  = (struct t_bus_utilization *)
      head_queue(&sim_char.general_net.threads_on_network);
      bus_utilization !=BU_NIL;
      bus_utilization  = (struct t_bus_utilization *)
      next_queue(&sim_char.general_net.threads_on_network)
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
    &sim_char.general_net.threads_on_network,
    (char *)bus_utilization
  );
  freeame ((char *) bus_utilization, sizeof(struct t_bus_utilization));

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
    copy_thread = duplicate_thread(thread);
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

  return(copy_thread);
}



/******************************************************************************
 * FUNCTION 'COMMUNIC_dedicated_connection_COM_TIMER_OUT'                     *
 *****************************************************************************/
struct t_thread*
COMMUNIC_dedicated_connection_COM_TIMER_OUT(struct t_thread *thread)
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
    copy_thread = duplicate_thread(thread);
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

  return(copy_thread);
}




/******************************************************************************
 * PROCEDURE 'COMMUNIC_COM_TIMER_OUT'                                         *
 *****************************************************************************/
void
COMMUNIC_COM_TIMER_OUT(struct t_thread *thread)
{
  struct t_action *action;

  /* JGG (12/11/2004): Creo que todas estas comparaciones se podrian ahorrar
   * haciendo un switch, sobre el mensaje que que esta esperando o enviado
   * el thread */
  if ((thread->partner_link == L_NIL) && (thread->local_link == L_NIL))
  {
    /* Es una comunicació local al node */
    if (debug&D_COMM)
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
     * s'ha de fer aqui. */
    if (thread->marked_for_deletion)
    {
      delete_duplicate_thread (thread);
    }
    return;
  }

  if ((thread->partner_link == L_NIL) && (thread->local_link == L_NUL))
  {
     /* Es una comunicació amb PORTS */
    if (debug&D_COMM)
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

      if (debug&D_COMM)
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

  /* Si estem aqui es que es una comunicacio entre nodes o entre maquines.
   * Per saber el tipus de comunicacio nomes cal que mirem el tipus de
   * qualsevol dels dos links: */

  switch (thread->local_link->kind)
  {
    case NODE_LINK:
      COMMUNIC_internal_network_COM_TIMER_OUT(thread);
      /*if (venus_enabled) {
        venusmsgs_in_flight--;
      }*/
      break;
    case MACHINE_LINK:
      COMMUNIC_external_network_COM_TIMER_OUT(thread);
      break;
    case CONNECTION_LINK:
      COMMUNIC_dedicated_connection_COM_TIMER_OUT(thread);
      break;
    default:
      panic("Unknown link type!\n");
  }

  if (thread->original_thread)
  {
    action         = thread->action;
    thread->action = action->next;
    freeame ((char *) action, sizeof (struct t_action));

    if (more_actions (thread))
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
 * PROCEDURE 'COMMUNIC_internal_resources_COM_TIMER_OUT'                      *
 *****************************************************************************/
/*
 * Alliberació dels recursos d'una comunicació punt a punt
 */
static void
COMMUNIC_internal_resources_COM_TIMER_OUT(struct t_thread *thread)
{
  register struct t_thread *wait_thread;
  register struct t_bus_utilization *bus_utilization;
  struct t_node *node;
  struct t_machine *machine;
  int aux;

  node = get_node_of_thread (thread);
  machine = node->machine;

  if (machine->communication.num_messages_on_network)
  {
    for (
      bus_utilization  = (struct t_bus_utilization *)
        head_queue(&machine->network.threads_on_network);
      bus_utilization !=BU_NIL;
      bus_utilization  = (struct t_bus_utilization *)
        next_queue(&machine->network.threads_on_network)
    )
    {
      if (bus_utilization->sender == thread) break;
    }

    if (bus_utilization==BU_NIL)
    {
      panic ("Unable to locate in bus utilization queue\n");
    }

    extract_from_queue (
      &machine->network.threads_on_network,
      (char *)bus_utilization
    );

    freeame ((char *) bus_utilization, sizeof(struct t_bus_utilization));
  }

  if (debug&D_COMM)
  {
    PRINT_TIMER (current_time);
    printf (
      ": COMMUNIC\tP%02d T%02d (t%02d) Free Local Link\n",
      IDENTIFIERS (thread)
    );
  }

  free_link (thread->local_link, thread);

  if (debug&D_COMM)
  {
    PRINT_TIMER (current_time);
    printf (": COMMUNIC\tP%02d T%02d (t%02d) Free Remote Link\n",
            IDENTIFIERS (thread));
  }

  free_link (thread->partner_link, thread);

  if (machine->network.curr_on_network > 0)
  {
    machine->network.curr_on_network--;
  }

  recompute_bandwith(thread);

  if ((machine->communication.num_messages_on_network) &&
      (count_queue (&machine->network.queue) > 0)
  )
  {
    wait_thread = (struct t_thread *) head_queue (&machine->network.queue);
#ifdef PARAVER_ALL
    Paraver_event (
      1,
      1,
      1,
      1,
      current_time,
      70,
      count_queue (&machine->network.queue)
    );
#endif
    if (debug&D_COMM)
    {
      PRINT_TIMER (current_time);
      printf(
        ": COMMUNIC\tP%02d T%02d (t%02d) Obtain Bus\n",
        IDENTIFIERS (wait_thread)
      );
    }

    switch (wait_thread->action->action)
    {
      case SEND:
        /* FEC: S'acumula el temps que ha estat esperant busos */
        ACCUMULATE_BUS_WAIT_TIME(wait_thread);

        extract_from_queue(&machine->network.queue, (char *) wait_thread);
        really_send (wait_thread);
        break;
      case MPI_OS:
        /* FEC: S'acumula el temps que ha estat esperant busos */
        ACCUMULATE_BUS_WAIT_TIME(wait_thread);

        extract_from_queue(&machine->network.queue, (char *) wait_thread);
        really_RMA (wait_thread);
        break;
      case GLOBAL_OP:
        /* aux sempre hauria de ser 1 */
        aux = machine->communication.num_messages_on_network -
              machine->network.curr_on_network;
        wait_thread->number_buses+=aux;
        machine->network.curr_on_network+=aux;

        if (wait_thread->number_buses ==
            machine->communication.num_messages_on_network
        )
        {
          /* FEC: S'acumula el temps que ha estat esperant busos */
          ACCUMULATE_BUS_WAIT_TIME(wait_thread);

          extract_from_queue(&machine->network.queue, (char *) wait_thread);
          global_op_get_all_buses(wait_thread);
        }
        break;
    }
  }
  return;
}



/******************************************************************************
 * PROCEDURE 'COMMUNIC_internal_resources_COM_TIMER_OUT'                      *
 *****************************************************************************/
static void
COMMUNIC_external_resources_COM_TIMER_OUT(struct t_thread *thread)
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
      head_queue(&sim_char.general_net.threads_on_network);
    bus_utilization != BU_NIL;
    bus_utilization  = (struct t_bus_utilization *)
      next_queue(&sim_char.general_net.threads_on_network)
  )
  {
	  if (bus_utilization->sender==thread) break;
	}
  if (bus_utilization==BU_NIL)
  {
    panic ("Unable to locate in external network bus utilization queue\n");
  }

  extract_from_queue (
    &sim_char.general_net.threads_on_network,
    (char *)bus_utilization
  );
	freeame ((char *) bus_utilization, sizeof(struct t_bus_utilization));
  *****************************************************************************/

  free_machine_link (thread->local_link, thread);

  if (debug&D_COMM)
  {
    PRINT_TIMER (current_time);
    printf (
      ": COMMUNIC\tP%02d T%02d (t%02d) Free Local Machine Link\n",
      IDENTIFIERS (thread)
    );
  }

  free_machine_link (thread->partner_link, thread);

  if (debug&D_COMM)
  {
    PRINT_TIMER (current_time);
    printf (
      ": COMMUNIC\tP%02d T%02d (t%02d) Free Remote Machine Link\n",
      IDENTIFIERS (thread)
    );
  }

  recompute_external_network_bandwidth(thread);
}



/******************************************************************************
 * PROCEDURE 'COMMUNIC_dedicated_resources_COM_TIMER_OUT'                     *
 *****************************************************************************/
static void
COMMUNIC_dedicated_resources_COM_TIMER_OUT(struct t_thread *thread)
{
  free_connection_link (thread->local_link, thread);

  if (debug&D_COMM)
  {
    PRINT_TIMER (current_time);
    printf (
      ": COMMUNIC\tP%02d T%02d (t%02d) Free Local Connection Link\n",
      IDENTIFIERS (thread)
    );
  }

  free_connection_link (thread->partner_link, thread);

  if (debug&D_COMM)
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
void
COMMUNIC_resources_COM_TIMER_OUT(struct t_thread *thread)
{

	if ((thread->partner_link == L_NIL) && (thread->local_link == L_NIL))
	{
    /* Es una comunicació local al node */
    /* Es retorna directament perque no s'ha reservat res! */
    return;
	}

	if ((thread->partner_link == L_NIL) && (thread->local_link == L_NUL))
	{
     /* Es una comunicació amb PORTS */
	   return;
	}


  /* Si estem aqui es que es una comunicacio entre nodes o entre maquines */
  /* Per saber el tipus de comunicacio nomes cal que mirem el tipus de
     qualsevol dels dos links: */
  switch (thread->local_link->kind)
  {
    case NODE_LINK:
      COMMUNIC_internal_resources_COM_TIMER_OUT(thread);
      break;

    case MACHINE_LINK:
      COMMUNIC_external_resources_COM_TIMER_OUT(thread);
      break;

    case CONNECTION_LINK:
      COMMUNIC_dedicated_resources_COM_TIMER_OUT(thread);
      break;

    default:
      panic("Unknown link type!\n");
  }
  /***** FEC: Fi alliberació dels recursos d'una comunicació punt a punt ******/
}










void COMMUNIC_general(int value, struct t_thread *thread)
{

  switch (value)
  {
    case RMA_TIMER_OUT:
    case COM_TIMER_OUT:
      COMMUNIC_COM_TIMER_OUT(thread);
      break;

    case COM_TIMER_OUT_RESOURCES:
      /* L'operació punt a punt encara no s'ha acabat, però ja es poden
          alliberar els recursos que té reservats. */
      COMMUNIC_resources_COM_TIMER_OUT(thread);
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




int
COMMUNIC_get_policy(char *s, int machine_id, FILE *fi, char *filename)
{
   int             i = 0;
   int j,k;
   char buf[BUFSIZE];
   struct t_machine  *machine;

  /* Si machine_id==0 s'assigna a totes les maquines, sino nomes
     a la maquina indicada. */

   while (COMMUNIC[i].name != 0)
   {
      if (strcmp (s, COMMUNIC[i].name) == 0)
      {
         if (debug&D_COMM)
         {
            PRINT_TIMER (current_time);
            if (machine_id==0)
              printf (": Communication policy selected %s\n", s);
            else
              printf (": Machine %d Communication policy selected %s\n",
                      (machine_id-1), s);
         }
	 switch (i)
	 {
	   case COMMUNIC_FIFO:
		break;
	   case COMMUNIC_RR:
           case COMMUNIC_BOOST:
/*		fgets (buf,128,fi); */
    i = fscanf (fi, "%[^\n]\n", buf);
    if (i == -1) return;

		j = sscanf(buf, "Quantum size (bytes): %d",&k);
                if (j!=1)
                   panic ("Invalid format in file %s.\nExpected Quantum size\n",filename);
                if (k<0)
                    k = 0;
                for (machine=(struct t_machine *)head_queue(&Machine_queue);
                     machine!=MA_NIL;
                     machine=(struct t_machine *)next_queue(&Machine_queue))
                {
                  if (machine->id==machine_id)
                  {
                    machine->communication.quantum = k;
                    break; /* ja s'ha trobat la maquina */
                  }
                  if (machine_id==0)
                  {
                    /* S'ha d'assignar a TOTES les maquines! */
                    machine->communication.quantum = k;
                  }
                }
                break;
	   default:
		panic ("Communication policy not implemented\n");
	 }

         for (machine=(struct t_machine *)head_queue(&Machine_queue);
              machine!=MA_NIL;
              machine=(struct t_machine *)next_queue(&Machine_queue))
         {
           if (machine->id==machine_id)
           {
             machine->communication.policy = i;
             break; /* ja s'ha trobat la maquina */
           }
           if (machine_id==0)
           {
             /* S'ha d'assignar a TOTES les maquines! */
             machine->communication.policy = i;
           }
         }
         return (i);
      }
      i++;
   }
   panic ("Invalid communication policy name %s\n", s);
   return (-1);
}




void COMMUNIC_init(char *fichero_comm)
{
  FILE *fi;
  struct t_machine  *machine;


  if (debug&D_COMM)
  {
    PRINT_TIMER (current_time);
    printf (": COMMUNIC initial routine called with file %s\n",fichero_comm);
  }

  for (machine  = (struct t_machine *)head_queue(&Machine_queue);
       machine != MA_NIL;
       machine  = (struct t_machine *)next_queue(&Machine_queue))
  {
    machine->network.utilization = 0;
    machine->network.total_time_in_queue = 0;
    ASS_ALL_TIMER (machine->network.last_actualization, current_time);
    machine->network.curr_on_network = 0;
    machine->communication.policy = COMMUNIC_FIFO;
  }

  if ((fichero_comm != (char *) 0) && (strcmp(fichero_comm,"")!=0))
  {
    free_reserved_pointer(); /* To ensure that 'fichero_comm' can be opened */
    fi = MYFOPEN (fichero_comm, "r");
    if (fi == (FILE *) 0)
    {
      panic ("Can't open communication configuration file %s\n",
             fichero_comm);
    }

    read_communication_config_file(fi, fichero_comm);

    fclose (fi);
  }

  /* S'inicialitza el calcul del traffic de la xarxa externa */
  periodic_external_network_traffic_init();
}




void COMMUNIC_end()
{
  struct t_node  *node;
  struct t_thread *thread;

  if (debug&D_COMM)
  {
    PRINT_TIMER (current_time);
    printf (": COMMUNIC_end called\n");
  }

  for (
    node  = (struct t_node *) head_queue (&Node_queue);
    node != N_NIL;
	  node  = (struct t_node *) next_queue (&Node_queue)
  )
  {
    if (count_queue (&(node->th_for_in)) != 0)
    {
      if (debug&D_LINKS)
	    {
        PRINT_TIMER (current_time);
        printf (
          ": COMMUNIC_end Warning! %d threads waiting in link on node %d\n",
		      count_queue (&(node->th_for_in)),
          node->nodeid
        );
      }
	    for (
        thread  = (struct t_thread *) head_queue (&(node->th_for_in));
	      thread != TH_NIL;
	      thread  = (struct t_thread *) next_queue (&(node->th_for_in))
      )
      {
        Paraver_thread_wait (
          0,
          IDENTIFIERS (thread),
          thread->last_paraver,
          current_time,
          PRV_BLOCKING_RECV_ST
        );

        new_cp_node (thread, CP_BLOCK);

        /*
        if (debug&D_LINKS)
        {
	        printf ("             P%d T%d th%d\n", IDENTIFIERS (thread));
	      }
        */
      }
    }
    if (count_queue (&(node->th_for_out)) != 0)
    {
      if (debug&D_LINKS)
      {
        PRINT_TIMER (current_time);
        printf (
          ": COMMUNIC_end %d threads waiting out link on node %d\n",
          count_queue (&(node->th_for_out)),
          node->nodeid
        );
      }
	    for (
        thread  = (struct t_thread *) head_queue (&(node->th_for_out));
	      thread != TH_NIL;
	      thread  = (struct t_thread *) next_queue (&(node->th_for_out))
      )
	    {
        Paraver_thread_wait (
          0,
          IDENTIFIERS (thread),
          thread->last_paraver, current_time,
          PRV_BLOCKING_RECV_ST
        );

        new_cp_node (thread, CP_BLOCK);

        /*
        if (debug&D_LINKS)
	      {
	        printf ("             P%d T%d th%d\n", IDENTIFIERS (thread));
        }
        */
      }
    }
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
int COMMUNIC_recv_reached(struct t_thread *thread, struct t_recv *mess)
{
  struct t_task   *task, *source_task;
  struct t_thread *partner_send;
  struct t_action *action;
  struct t_send   *mess_send;
  struct t_thread *copia_thread;
  int              res   = 0;

  task = thread->task;
  for (
    partner_send = (struct t_thread *) head_queue(&(task->send_without_recv));
    partner_send != TH_NIL;
    partner_send = (struct t_thread *) next_queue(&(task->send_without_recv))
  )
  {
    source_task = partner_send->task;
    action      = partner_send->action;
    mess_send   = &(action->desc.send);
    if (((mess->ori == source_task->taskid)      || (mess->ori == -1))      &&
        ((mess->mess_tag == mess_send->mess_tag) || (mess->mess_tag == -1)) &&
        (mess->communic_id == mess_send->communic_id))
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
    extract_from_queue(&(task->send_without_recv), (char *)partner_send);
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
    inFIFO_queue(&(task->recv_without_send), (char *)copia_thread);
  }

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
int COMMUNIC_send_reached(struct t_thread *thread, struct t_send *mess)
{
  struct t_task   *task, *dest_task;
  struct t_thread *partner_recv;
  struct t_action *action;
  struct t_recv   *mess_recv;
  struct t_thread *copia_thread;
  int res=0;

  task=thread->task;
  dest_task=locate_task(task->Ptask, mess->dest);
  for (partner_recv=(struct t_thread *)head_queue(&(dest_task->recv_without_send));
       partner_recv!=TH_NIL;
       partner_recv=(struct t_thread *)next_queue(&(dest_task->recv_without_send)))
  {
    action=partner_recv->action;
    mess_recv=&(action->desc.recv);
    if (((task->taskid == mess_recv->ori) || (mess_recv->ori == -1)) &&
        ((mess->mess_tag == mess_recv->mess_tag) || (mess_recv->mess_tag == -1)) &&
        (mess->communic_id == mess_recv->communic_id))
    {
      /* Ja s'havia arribat al recv corresponent */
      break;
    }
  }

  if (partner_recv!=TH_NIL)
  {
    if (action->action==IRECV)
    {
      /* Caldra retornar que s'ha trobat un Irecv corresponent i, per tant,
       * aquest thread no s'haura d'esperar encara que sigui sincron i el
       * seu partner no hagi arribat al wait. */
      res = 1;
    }
    /* Ja s'havia arribat al recv/Irecv corresponent, per tant, cal treure'l de la
     * cua de recv sense el send que li correspon */
    extract_from_queue(&(dest_task->recv_without_send),(char *)partner_recv);
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
    copia_thread=duplicate_thread (thread);
    inFIFO_queue(&(dest_task->send_without_recv), (char *)copia_thread);
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

void
COMMUNIC_send (struct t_thread *thread)
{
  struct t_action  *action;
  struct t_send    *mess;         /* Message */
  struct t_task    *task,         /* Sender task */
                   *task_partner; /* Receiver task */
  int               comm_kind;
  struct t_thread  *copy_thread;
  struct t_thread  *partner;
  struct t_account *account;
  t_micro           startup, copy_latency, roundtriptime;
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

  mess = &(action->desc.send);

  task = thread->task;
  task_partner = locate_task (task->Ptask, mess->dest);
  if (task_partner == T_NIL)
  {
    panic ("P%02d T%02d (t%02d) trying to send message to inexistent T%d\n",
           IDENTIFIERS (thread),
           mess->dest);
  }

  node_s = get_node_of_task (task);
  node_r = get_node_of_task (task_partner);

  /* S'obte el tipus de communicació */
  kind = get_communication_type(task,
                                task_partner,
                                mess->mess_tag,
                                mess->mess_size,
                                &connection);

  mess->comm_type = kind;

  /* Compute startup duration and re-schedule thread if needed */
  if (thread->startup_done == FALSE)
  {
    startup = compute_startup(thread, kind, node_s, connection);

    Vampir_Send (task->taskid,
                 task_partner->taskid,
                 mess->communic_id,
                 mess->mess_tag,
                 mess->mess_size);
    SDDF_send_start (node_s->nodeid, task->taskid, current_time);

    if (startup != (t_micro) 0)
    {
      Activity_Enter (FALSE,thread, 1);
      thread->logical_send = current_time;

      thread->loose_cpu     = FALSE;
      thread->doing_startup = TRUE;

      account = current_account (thread);
      FLOAT_TO_TIMER (startup, tmp_timer);
      ADD_TIMER (account->latency_time, tmp_timer, account->latency_time);
      SUB_TIMER (account->cpu_time, tmp_timer, account->cpu_time);

      SCHEDULER_thread_to_ready_return (M_COM, thread, startup, 0);

      if (debug&D_COMM)
      {
        PRINT_TIMER (current_time);
        printf (": COMMUNIC_send\tP%02d T%02d (t%02d) Initiate startup (%f)\n",
                IDENTIFIERS (thread),
                (double) startup / 1e6);
      }
      return;
    }
    else /* (startup == (t_micro) 0) */
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
      copy_latency = compute_copy_latency(thread, node_s, mess->mess_size);

      if (copy_latency != (t_micro) 0)
      {
        thread->loose_cpu     = FALSE;
        thread->doing_copy    = TRUE;

        account = current_account (thread);
        FLOAT_TO_TIMER (copy_latency, tmp_timer);
        ADD_TIMER (account->latency_time, tmp_timer, account->latency_time);
        SUB_TIMER (account->cpu_time, tmp_timer, account->cpu_time);

        SCHEDULER_thread_to_ready_return (M_COM, thread, copy_latency, 0);

        if (debug&D_COMM)
        {
          PRINT_TIMER (current_time);
          printf (
            ": COMMUNIC_send\tP%02d T%02d (t%02d) Initiate copy latency (%f)\n",
            IDENTIFIERS (thread),
            (double) copy_latency / 1e6,
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

  /* Round Trip Time for sends */
  if (RTT_enabled && mess->rendez_vous && (kind == INTERNAL_NETWORK_COM_TYPE))
  {
    if (thread->roundtrip_done == FALSE)
    {
      roundtriptime = RTT_time/2.0;

      if (RTT_time != (t_micro) 0)
      {
        thread->loose_cpu       = FALSE;
        thread->doing_roundtrip = TRUE;

        account = current_account (thread);
        FLOAT_TO_TIMER (roundtriptime, tmp_timer);
        ADD_TIMER (account->latency_time, tmp_timer, account->latency_time);
        SUB_TIMER (account->cpu_time, tmp_timer, account->cpu_time);

        SCHEDULER_thread_to_ready_return (M_COM, thread, roundtriptime, 0);

        if (debug&D_COMM)
        {
          PRINT_TIMER (current_time);
          printf (
          ": COMMUNIC_send\tP%02d T%02d (t%02d) Initiate round trip time(%f)\n",
            IDENTIFIERS (thread),
            (double) roundtriptime / 1e6,
            mess->mess_size);
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

  SDDF_send_stop (node_s->nodeid,
                  task->taskid,
                  current_time,
                  mess->mess_tag,
                  mess->mess_size,
                  node_r->nodeid,
                  task_partner->taskid);
  account = current_account (thread);
  account->n_sends++;

  account->n_bytes_send += mess->mess_size;

  if (debug&D_COMM)
  {
    PRINT_TIMER (current_time);
    printf (
      ": COMMUNIC_send\tP%02d T%02d (t%02d) -> T%d Tag: %d Communicator: %d Size: %db\n",
      IDENTIFIERS (thread),
      action->desc.send.dest,
      mess->mess_tag,
      mess->communic_id,
      action->desc.send.mess_size
    );
  }

  /* FEC: S'avisa que s'ha arribat a aquest send i es comprova si s'ha arribat
   * a un Irecv que permeti continuar a aquest send encara que sigui sincron. */
  hi_ha_irecv = COMMUNIC_send_reached(thread,mess);

  if (venus_enabled && (kind == INTERNAL_NETWORK_COM_TYPE)) {
      double dtime;
      TIMER_TO_FLOAT(current_time, dtime);
      if (mess->rendez_vous) {
        vc_command_rdvz_send(dtime, node_s->nodeid - 1, node_r->nodeid - 1, mess->mess_tag, mess->mess_size);
      }
  }

  partner = locate_receiver (&(task_partner->recv),
                             task->taskid,
                             mess->mess_tag,
                             mess->communic_id);

  if (partner != TH_NIL)
  {
    SCHEDULER_info (COMMUNICATION_INFO, SCH_INFO_SEND, thread, TH_NIL);
  }

  comm_kind = (mess->immediate << 1) + mess->rendez_vous;
  switch (comm_kind)
  {
    /* Con RD estamos en un Send */
    case RD_SYNC:
    case RD_ASYNC:
    {
      if ((!hi_ha_irecv) && (partner == TH_NIL))
      {
        if (mess->immediate)
        {
          /* El Rendez vous s'ha de fer en "background". Cal utilitzar
           * una copia del thread */
          copy_thread = duplicate_thread (thread);
          inFIFO_queue (&(task->send), (char *)copy_thread);
          /* El thread original pot continuar */
          action = thread->action;
          thread->action = action->next;
          freeame ((char *) action, sizeof (struct t_action));
          if (more_actions (thread))
          {
            thread->loose_cpu = FALSE;
            SCHEDULER_thread_to_ready (thread);
          }
        } /* Send, con Rendezvous */
        else
        {
          /* El thread s'ha de bloquejar per esperar el Irecv/recv */
          inFIFO_queue (&(task->send), (char *)thread);
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
        freeame ((char *) action, sizeof (struct t_action));
        if (more_actions (thread))
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
      freeame ((char *) action, sizeof (struct t_action));
      if (more_actions (thread))
      {
        thread->loose_cpu = FALSE;
        SCHEDULER_thread_to_ready (thread);
      }
      break;
    }
    default:
      panic("Impossible send type %d\n", comm_kind);
      break;
  } /* switch */
}

/******************************************************************************
 * PROCEDURE 'COMMUNIC_recv'                                                  *
 * ÚLTIMA MODIFICACIÓN: 29/10/2004 (Juan González García)                     *
 *****************************************************************************/

void
COMMUNIC_recv(struct t_thread *thread)
{
  struct t_action  *action;
  struct t_recv    *mess;
  struct t_task    *task, *task_source;
  struct t_account *account;
  dimemas_timer     tmp_timer;
  struct t_node    *node_s, *node_r;
  t_micro           startup, copy_latency;
  int               kind;
  struct t_dedicated_connection *connection;


  action = thread->action;
  if (action->action != RECV)
  {
    panic ("Calling COMMUNIC_recv and action is not receive (%d)\n",
           action->action);
  }

  mess = &(action->desc.recv);
  task = thread->task;

  node_r = get_node_of_thread (thread);
  node_s = get_node_for_task_by_name (thread->task->Ptask, mess->ori);

  task_source = locate_task (task->Ptask, mess->ori);

  /* S'obte el tipus de communicació */
  kind = get_communication_type(task_source,
                                task,
                                mess->mess_tag,
                                mess->mess_size,
                                &connection);
  mess->comm_type = kind;

  if (thread->startup_done == FALSE)
  { /* Compute startup duration and re-schedule thread if needed */
    startup = compute_startup(thread, kind, node_s, connection);

    SDDF_recv_start (node_r->nodeid, task->taskid, current_time);

    if (startup > (t_micro) 0)
    {

      Activity_Enter (FALSE, thread, 1);    /* Esto es para Vampir! */
      thread->logical_recv = current_time;

      thread->loose_cpu     = FALSE;
      thread->doing_startup = TRUE;

      account = current_account (thread);
      FLOAT_TO_TIMER (startup, tmp_timer);
      ADD_TIMER (account->latency_time, tmp_timer, account->latency_time);
      SUB_TIMER (account->cpu_time, tmp_timer, account->cpu_time);

      SCHEDULER_thread_to_ready_return (M_COM, thread, startup, 0);

      if (debug&D_COMM)
      {
        PRINT_TIMER (current_time);
        printf (": COMMUNIC_recv\tP%02d T%02d (t%02d) Initiate startup (%f)\n",
                IDENTIFIERS (thread),
                (double) startup / 1e6);
      }

      return;
    }
    else if (startup == (t_micro) 0)
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
      copy_latency = compute_copy_latency(thread, node_s, mess->mess_size);

      if (copy_latency != (t_micro) 0)
      {
        thread->loose_cpu     = FALSE;
        thread->doing_copy    = TRUE;

        account = current_account (thread);
        FLOAT_TO_TIMER (copy_latency, tmp_timer);
        ADD_TIMER (account->latency_time, tmp_timer, account->latency_time);
        SUB_TIMER (account->cpu_time, tmp_timer, account->cpu_time);

        SCHEDULER_thread_to_ready_return (M_COM, thread, copy_latency, 0);

        if (debug&D_COMM)
        {
          PRINT_TIMER (current_time);
          printf (
            ": COMMUNIC_recv\tP%02d T%02d (t%02d) Initiate copy latency (%f)\n",
            IDENTIFIERS (thread),
            (double) copy_latency / 1e6,
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

  /* Startup and Copy checks reset */
  thread->startup_done = FALSE;
  thread->copy_done    = FALSE;

  SDDF_recv_block (node_r->nodeid, task->taskid, current_time);
  account = current_account (thread);
  account->n_recvs++;

  /* FEC: S'avisa que s'ha arribat a aquest recv */
  /* JGG: No se tiene en cuenta el tipo de send ?¿?¿ */
  COMMUNIC_recv_reached(thread, mess);

  if (is_message_awaiting (task, mess, thread)) /* 'is_message_awaiting'      */
  {                                             /* desencola a los que esperan*/
    Vampir_Receive (task->taskid,
                    action->desc.recv.ori,
                    mess->communic_id,
                    mess->mess_tag,
                    mess->mess_size);
    account->n_recvs_on_processor++;
    if (debug&D_COMM)
    {
      PRINT_TIMER (current_time);
      printf (
        ": COMMUNIC_recv\tP%02d T%02d (t%02d) <- T%d Tag: %d Communicator: %d (Local Message)\n",
        IDENTIFIERS (thread),
        action->desc.recv.ori,
        mess->mess_tag,
        mess->communic_id
      );
    }

    thread->action = action->next;
    freeame ((char *) action, sizeof (struct t_action));
    if (more_actions (thread))
    {
      thread->loose_cpu = FALSE;
      SCHEDULER_thread_to_ready (thread);
    }
  }
  else /* !is_message_awaiting(...) */
  {
    Start_communication_if_partner_ready_for_rendez_vous(thread, mess);
    if (node_r->machine->scheduler.busywait_before_block)
    {
      SCHEDULER_thread_to_busy_wait (thread);
    }
    else
    {
      account->n_recvs_must_wait++;
      thread->start_wait_for_message = current_time;
      if (debug&D_COMM)
      {
        PRINT_TIMER (current_time);
        printf (
          ": COMMUNIC_recv\tP%02d T%02d (t%02d) <- T%d Tag: %d Communicator: %d (Waiting)\n",
          IDENTIFIERS (thread),
          action->desc.recv.ori,
          mess->mess_tag,
          mess->communic_id
        );
      }
      Activity_Enter (FALSE,thread, 2);
      inFIFO_queue (&(task->recv), (char *) thread);
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

void
COMMUNIC_Irecv(struct t_thread *thread)
{
  struct t_action  *action;
  struct t_recv    *mess;
#ifdef STARTUP_ALS_IRECV
  struct t_task    *task, *task_source;
  struct t_account *account;
  dimemas_timer     tmp_timer;
  struct t_node    *node, *node_s, *node_r;
  t_micro           startup;
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

  mess = &(action->desc.recv);

#ifdef STARTUP_ALS_IRECV /*****************************************************/

  task = thread->task;
  node_r = get_node_of_thread (thread);
  node_s = get_node_for_task_by_name (thread->task->Ptask, mess->ori);

  task_source = locate_task (task->Ptask, mess->ori);
  /* S'obte el tipus de communicació */
  kind = get_communication_type(task_source,
                                task,
                                mess->mess_tag,
                                mess->mess_size,
                                &connection);
  mess->comm_type = kind;

  if (thread->startup_done == FALSE)
  { /* Compute startup duration and re-schedule thread if needed */
    startup = compute_startup(thread, kind, node_s, connection);

    SDDF_recv_start (node_r->nodeid, task->taskid, current_time);

    if (startup > (t_micro) 0) /* Change != with > */
    { /* Positive startup time. Thread must be re-scheduled */
      Activity_Enter (FALSE,thread, 1);

#ifndef LOGICAL_RECEIVE_ALS_WAIT
      /* Tan aviat ho volen als Irecv com als Wait */
      thread->logical_recv = current_time;
#endif /* LOGICAL_RECEIVE_ALS_WAIT */

      thread->loose_cpu     = FALSE;
      thread->doing_startup = TRUE;

      account = current_account (thread);
      FLOAT_TO_TIMER (startup, tmp_timer);
      ADD_TIMER (account->latency_time, tmp_timer, account->latency_time);
      SUB_TIMER (account->cpu_time, tmp_timer, account->cpu_time);

      SCHEDULER_thread_to_ready_return (M_COM, thread, startup, 0);

      if (debug&D_COMM)
      {
        PRINT_TIMER (current_time);
        printf (": COMMUNIC_Irecv\tP%02d T%02d (t%02d) Initiate startup (%f)\n",
                IDENTIFIERS (thread),
                (double) startup / 1e6);
      }

      return;
    }
    else if (startup == (t_micro) 0)
    {
#ifndef LOGICAL_RECEIVE_ALS_WAIT
      /* Tan aviat ho volen als Irecv com als Wait */
      thread->logical_recv = current_time;
#endif /* LOGICAL_RECEIVE_ALS_WAIT */
      SDDF_recv_block (node->nodeid, task->taskid, current_time);
      thread->startup_done = TRUE;
    }
  }


  /* Copy latency operations */
  if (DATA_COPY_enabled && mess->mess_size <= DATA_COPY_message_size)
  {
    if (thread->copy_done == FALSE)
    {
      copy_latency = compute_copy_latency(thread, node_s, mess->mess_size);

      if (copy_latency != (t_micro) 0)
      {
        thread->loose_cpu     = FALSE;
        thread->doing_copy    = TRUE;

        account = current_account (thread);
        FLOAT_TO_TIMER (copy_latency, tmp_timer);
        ADD_TIMER (account->latency_time, tmp_timer, account->latency_time);
        SUB_TIMER (account->cpu_time, tmp_timer, account->cpu_time);

        SCHEDULER_thread_to_ready_return (M_COM, thread, copy_latency, 0);

        if (debug&D_COMM)
        {
          PRINT_TIMER (current_time);
          printf (
            ": COMMUNIC_Irecv\tP%02d T%02d (t%02d) Initiate copy latency (%f)\n",
            IDENTIFIERS (thread),
            (double) copy_latency / 1e6,
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

  SDDF_recv_block (node_r->nodeid, task->taskid, current_time);
  account = current_account (thread);

#else /* STARTUP_ALS_IRECV */

#ifndef LOGICAL_RECEIVE_ALS_WAIT
  /* Tan aviat ho volen als Irecv com als Wait */
  thread->logical_recv = current_time;

#endif /* LOGICAL_RECEIVE_ALS_WAIT */

#endif /* STARTUP_ALS_IRECV ***************************************************/


  /* FEC: S'avisa que s'ha arribat a aquest Irecv i s'obte si
   * cal desbloqejar un send o no. */
  hi_ha_send_sync = COMMUNIC_recv_reached(thread,mess);
  if (hi_ha_send_sync)
  {
    /* Per poder executar aixo cal haver comprovat que el send que correspon
     * a aquest Irecv es realment sincron, perque si no es aixi, pot ser que
     * el Irecv ja hagi rebut el missatge a traves d'un send asincron i,
     * per tant, estariem desbloquejant el send corresponent a algun Irecv
     * futur. */
    Start_communication_if_partner_ready_for_rendez_vous(thread, mess);
  }

  thread->action = action->next;
  freeame ((char *) action, sizeof (struct t_action));
  if (more_actions (thread))
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
void
COMMUNIC_wait(struct t_thread *thread)
{
  struct t_action               *action;
  struct t_recv                 *mess;
  struct t_task                 *task, *task_source;
  struct t_account              *account;
  dimemas_timer                  tmp_timer;
  struct t_node                 *node_r, *node_s;
  t_micro                        startup, copy_latency;
  int                            kind;
  struct t_dedicated_connection *connection;


  action = thread->action;
  if (action->action != WAIT)
  {
    panic("Calling COMMUNIC_wait and action is not Wait (%d)\n",
          action->action);
  }
  mess    = &(action->desc.recv);
  task   = thread->task;
  node_r = get_node_of_thread (thread);
  node_s = get_node_for_task_by_name (thread->task->Ptask, mess->ori);

  task_source = locate_task (task->Ptask, mess->ori);
  /* S'obte el tipus de communicació */
  kind = get_communication_type(task_source,
                                task,
                                mess->mess_tag,
                                mess->mess_size,
                                &connection);
  mess->comm_type = kind;

  if (thread->startup_done == FALSE)
  {
    startup = compute_startup(thread, kind, node_s, connection);

    SDDF_recv_start (node_r->nodeid, task->taskid, current_time);

    if (startup != (t_micro) 0)
    {
      Activity_Enter (FALSE,thread, 1);
      if (debug&D_COMM)
      {
        PRINT_TIMER (current_time);
        printf (
          ": COMMUNIC_wait\tP%02d T%02d (t%02d) Initiate startup (%f)\n",
          IDENTIFIERS (thread),
          (double) startup / 1e6
        );
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
      SDDF_recv_block (node_r->nodeid, task->taskid, current_time);
    }
  }
  else
  {
    SDDF_recv_block (node_r->nodeid, task->taskid, current_time);
  }

  /* Copy latency operations */
  if (DATA_COPY_enabled && mess->mess_size <= DATA_COPY_message_size)
  {
    if (thread->copy_done == FALSE)
    {
      copy_latency = compute_copy_latency(thread, node_s, mess->mess_size);

      if (copy_latency != (t_micro) 0)
      {
        thread->loose_cpu     = FALSE;
        thread->doing_copy    = TRUE;

        account = current_account (thread);
        FLOAT_TO_TIMER (copy_latency, tmp_timer);
        ADD_TIMER (account->latency_time, tmp_timer, account->latency_time);
        SUB_TIMER (account->cpu_time, tmp_timer, account->cpu_time);

        SCHEDULER_thread_to_ready_return (M_COM, thread, copy_latency, 0);

        if (debug&D_COMM)
        {
          PRINT_TIMER (current_time);
          printf (
            ": COMMUNIC_wait\tP%02d T%02d (t%02d) Initiate copy latency (%f)\n",
            IDENTIFIERS (thread),
            (double) copy_latency / 1e6,
            mess->mess_size);
        }
        return;
      }
    }
  }

  /* Startup and Copy checks reset */
  thread->startup_done = FALSE;
  thread->copy_done    = FALSE;

  SDDF_recv_block (node_r->nodeid, task->taskid, current_time);

  account = current_account (thread);
  account->n_recvs++;
  account = current_account (thread);

#ifdef LOGICAL_RECEIVE_ALS_WAIT
  /* Tan aviat ho volen als Irecv com als Wait */
  thread->logical_recv = current_time;
#endif /* LOGICAL_RECEIVE_ALS_WAIT */

  if (is_message_awaiting (task, mess, thread))
  {
    /* El mensaje ya se ha recibido. 'is_message_awaiting' ya se encarga de
     * desencolar los elementos en consecuencia, por lo que el 'wait' ya se
     * considera finalizado */

    Vampir_Receive (
      task->taskid,
      action->desc.recv.ori,
      mess->communic_id,
      mess->mess_tag,
      mess->mess_size
    );
    account->n_recvs_on_processor++;
    if (debug&D_COMM)
    {
      PRINT_TIMER (current_time);
      printf (
        ": COMMUNIC_wait\tP%02d T%02d (t%02d) <- T%d Tag: %d Communicator: %d (Local message)\n",
        IDENTIFIERS (thread),
        action->desc.recv.ori,
        mess->mess_tag,
        mess->communic_id
      );
    }

    thread->action = action->next;
    freeame ((char *) action, sizeof (struct t_action));
    if (more_actions (thread))
    {
      thread->loose_cpu = FALSE;
      SCHEDULER_thread_to_ready (thread);
    }
  }
  else
  {
    /* Start_communication_if_partner_ready_for_rendez_vous assumes
     * thread is receiver, so locate receiver task */
    Start_communication_if_partner_ready_for_rendez_vous(thread,mess);
    if (node_r->machine->scheduler.busywait_before_block)
    {
      SCHEDULER_thread_to_busy_wait (thread);
    }
    else
    {
      account->n_recvs_must_wait++;
      thread->start_wait_for_message = current_time;
      if (debug&D_COMM)
      {
        PRINT_TIMER (current_time);
        printf (
          ": COMMUNIC_wait\tP%02d T%02d (t%02d) <- T%d Tag: %d Communicator: %d (Waiting)\n",
          IDENTIFIERS (thread),
          action->desc.recv.ori,
          mess->mess_tag,
          mess->communic_id
        );
      }
      Activity_Enter (FALSE,thread, 2);
      inFIFO_queue (&(task->recv), (char *) thread);
    }
  }
}

void
COMMUNIC_block_after_busy_wait(struct t_thread *thread)
{
  struct t_task    *task;
  struct t_account *account;
  struct t_action  *action;
  struct t_recv    *mess;

  task = thread->task;
  action = thread->action;
  mess = &(action->desc.recv);
  account = current_account (thread);
  account->n_recvs_must_wait++;
  thread->start_wait_for_message = current_time;

  if (debug&D_COMM)
  {
    PRINT_TIMER (current_time);
    printf (
      ": COMMUNIC_recv/wait P%02d T%02d (t%02d) <- T%d Tag(%d) Block after Busy Wait\n",
	    IDENTIFIERS (thread),
      mess->ori,
      mess->mess_tag
    );
  }
  inFIFO_queue (&(task->recv), (char *) thread);
}

int
get_global_op_id_by_name (char *name)
{
  struct t_global_op_definition * glop;

  for (glop=(struct t_global_op_definition *)head_queue(&Global_op);
       glop!=(struct t_global_op_definition *)0;
       glop=(struct t_global_op_definition *)next_queue(&Global_op))
  {
    if (strcmp(glop->name, name)==0)
      return(glop->identificator);
  }
  return(-1);
}


static void
inicialitza_info_nova_globalop(
  int model,
  struct t_global_op_definition *glop,
  struct t_queue *cua
)
{
  struct t_global_op_information *glop_info;

  switch (model) /* Aixo és absurd */
  {
    case GOP_MODEL_CTE:
      model = GOP_MODEL_CTE;
      break;
    case GOP_MODEL_LIN:
      model = GOP_MODEL_LIN;
      break;
    case GOP_MODEL_LOG:
      model = GOP_MODEL_LOG;
      break;
    default:
      panic ("Unexpected Global operation model\n");
      break;
  }

  glop_info = (struct t_global_op_information *) query_prio_queue(cua,
              (t_priority)glop->identificator);

  if (glop_info != (struct t_global_op_information *)0)
  { /* Aixo no hauria de passar mai! */
    if (debug&D_COMM)
    {
      PRINT_TIMER(current_time);
      printf (
       ": WARNING ('inicialitza_info_nova_globalop'): Global operation %s (%d) already exists\n",
        glop->name,
        glop->identificator
      );
    }
  }

  glop_info = (struct t_global_op_information *)
    mallocame (sizeof(struct t_global_op_information));

  glop_info->identificator = glop->identificator;
  glop_info->FIN_model     = model;
  glop_info->FIN_size      = GOP_SIZE_CURR;
  glop_info->FOUT_model    = model;
  glop_info->FOUT_size     = GOP_SIZE_CURR;

  insert_queue (cua, (char *)glop_info, (t_priority)glop->identificator);
}

void
new_global_op (int identificator, char *name)
{
  struct t_global_op_definition  *glop;
  struct t_machine  *machine;

  /* Es guarda la definicio d'aquesta operació col.lectiva */
  glop = (struct t_global_op_definition *)
         query_prio_queue(&Global_op, (t_priority)identificator);

  if (glop != GOPD_NIL)
  {
    if (debug&D_COMM)
    {
      PRINT_TIMER(current_time);
      printf (
       ": WARNING ('new_global_op'): Global operation %s (%d) already exists\n",
        glop->name,
        glop->identificator
      );
    }
  }

  glop = (struct t_global_op_definition *)
         mallocame (sizeof(struct t_global_op_definition));

  glop->name          = (char *) mallocame(strlen(name) + 1);
  glop->identificator = identificator;

  strcpy (glop->name, name);

  insert_queue (&Global_op, (char *)glop, (t_priority)identificator);
  Vampir_GlobalOpToken(identificator, name);

  /* S'hauria de guardar informació d'aquesta operació per cada màquina */
  for(
    machine  = (struct t_machine *)head_queue(&Machine_queue);
    machine != MA_NIL;
    machine  = (struct t_machine *)next_queue(&Machine_queue)
  )
  {
    inicialitza_info_nova_globalop(
      machine->communication.global_operation,
      glop,
      &machine->communication.global_ops_info
    );
  }

  /* Es fa el mateix per la xarxa externa */
  inicialitza_info_nova_globalop(
    sim_char.general_net.global_operation,
    glop,
    &sim_char.general_net.global_ops_info
  );
}


/*
 * Transfer message time function
 */
t_micro
transferencia(
  int                            size,
  int                            communication_type,
  struct t_thread               *thread,
  struct t_dedicated_connection *connection,
  t_micro                       *temps_recursos
)
{
  struct t_node    *node, *node_partner;
  struct t_machine *machine;
  struct t_task    *task, *task_partner;
  t_micro           bandw, temps, t_recursos;

  node    = get_node_of_thread (thread);
  machine = node->machine;
  task_partner = locate_task (thread->task->Ptask,
                              thread->action->desc.send.dest);

  switch(communication_type)
  {
    case LOCAL_COMMUNICATION_TYPE:  /* Es un missatge local al node */
      if (node->bandwith == (t_micro) 0)
      {
        temps      = 0;
        t_recursos = 0;
      }
      else
      {
        temps = ((int) ((t_micro) (1000000) / (1 << 20) * size /
                        (t_micro) node->bandwith));
        t_recursos = temps;
      }
      break;

    case INTERNAL_NETWORK_COM_TYPE:
      if (machine->communication.remote_bandwith == (t_micro) 0)
      {
        temps      = 0;
        t_recursos = 0;
      }
      else
      {
        bandw = recompute_bandwith (thread);
        thread->last_comm.bandwith = bandw;
        thread->last_comm.bytes = size;
        ASS_ALL_TIMER (thread->last_comm.ti, current_time);
        temps = (bandw * size);
        /* Es calcula el temps d'utilització dels recursos amb l'ample
           de banda maxim possible */
        bandw = (t_micro) machine->communication.remote_bandwith;

        if (bandw != 0)
        {
          bandw = (t_micro) ((t_micro) (1000000) / (1 << 20) / bandw);
        }

        t_recursos = (bandw * size);
      }
      break;

    case EXTERNAL_NETWORK_COM_TYPE:
      if (sim_char.general_net.bandwidth == (t_micro) 0)
      {
        temps      = 0;
        t_recursos = 0;
      }
      else
      {
        recompute_external_network_traffic(size);
        bandw = recompute_external_network_bandwidth (thread);
        /* Aqui es passa de Mbytes/sec a microsegons/byte. */
        bandw = bandwidth_2_ms_per_byte(bandw);
        thread->last_comm.bandwith = bandw;
        thread->last_comm.bytes    = size;
        ASS_ALL_TIMER (thread->last_comm.ti, current_time);
        temps = (bandw * size);

        node_partner = get_node_of_task (task_partner);
        temps +=
          sim_char.general_net.flight_times[node->machine->id-1][node_partner->machine->id-1];

        /* Es calcula el temps d'utilització dels recursos amb l'ample
           de banda maxim possible */
        bandw = (t_micro) sim_char.general_net.bandwidth;
        if (bandw != 0)
        {
          bandw = (t_micro) ((t_micro) (1000000) / (1 << 20) / bandw);
        }
        t_recursos = (bandw * size);
      }
      break;

    case DEDICATED_CONNECTION_COM_TYPE:
      /* En aquest cas el parametre connection no pot ser NULL */
      if (connection->bandwidth == (t_micro) 0)
      {
        temps      = 0;
        t_recursos = 0;
      }
      else
      {
        /* Se suposa que els "busos" de la connexio no seran mai compartits
           i, per tant, no caldra mai recalcular l'ample de banda */
        bandw =
          (t_micro) ((t_micro) (1000000) / (1 << 20) / connection->bandwidth);
        thread->last_comm.bandwith = bandw;
        thread->last_comm.bytes = size;
        ASS_ALL_TIMER (thread->last_comm.ti, current_time);
        temps = (bandw * size);
        /* Falta sumar-hi el flight time*/
        temps += connection->flight_time;
        /* Es calcula el temps d'utilització dels recursos */
        t_recursos = (bandw * size);
      }
      break;
    default:
      panic("Unknown communication type!\n");
      break;
  }

  /* Si es volia calcular el temps d'ocupacio dels recursos, tambe s'ha
     de retornar. */
  if (temps_recursos != NULL)
  {
    *temps_recursos = t_recursos;
  }

  if (debug&D_COMM)
  {
    PRINT_TIMER(current_time);
    printf
    (
      ": TRANSFERENCIA\tP%02d T%02d (t%02d) -> T%d Size: %db Time: %f Rsrc_Time: %f\n",
      IDENTIFIERS(thread),
      task_partner->taskid,
      size,
      temps / 1e6,
      t_recursos / 1e6
    );
  }

  /* Es retorna el temps necessari estimat per fer la transferencia */
  return(temps);
}



#define COMPROVA_CONDICIO_MIDA_CONNEXIO(c_size,c_cond, m_size) \
  ( \
    (c_cond==0)? (m_size < c_size) : /* < */\
                 ((c_cond==1)? (m_size == c_size) : /* = */\
                               (m_size > c_size))   /* > */\
  )

int
connection_can_be_used(
  struct t_dedicated_connection *connection,
  int mess_tag,
  int mess_size
)
{
  int i;
  int res, res_size1, res_size2;

  /* Es comprova si el tag donat pot utilitzar la connexio dedicada */
  for(i=0;i<connection->number_of_tags;i++)
  {
    if (connection->tags[i]==mess_tag) break;
  }

  if (i==connection->number_of_tags) /* El tag no es permes */
    return(0);

  /* Es comprova si la mida del missatge esta dins del marge permes */
  res_size1=COMPROVA_CONDICIO_MIDA_CONNEXIO(connection->first_message_size,
                                            connection->first_size_condition,
                                            mess_size);
  res_size2=COMPROVA_CONDICIO_MIDA_CONNEXIO(connection->second_message_size,
                                            connection->second_size_condition,
                                            mess_size);

  res=( (connection->operation==0)? (res_size1 && res_size2) : /* AND */
        (res_size1 || res_size2)); /* OR */

/*
  printf("Connexio %d tag %d mida %d: res1 %d res2 %d res %d\n",
         connection->id, mess_tag, mess_size, res_size1, res_size2, res);
*/
  return(res);
}


int
get_communication_type(
  struct t_task  *task,
  struct t_task  * task_partner,
  int mess_tag, int mess_size,
  struct t_dedicated_connection **connection
)
{
  int result_type;
  struct t_node  *node, *node_partner;
  struct t_machine *s_machine, *d_machine;
  struct t_dedicated_connection *d_con;



  node         = get_node_of_task(task);
  node_partner = get_node_of_task(task_partner);

  if (node == node_partner)
  {
    /* Es un missatge local al node */
    result_type = LOCAL_COMMUNICATION_TYPE;
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
    for(
      d_con = (struct t_dedicated_connection *)
                head_queue(&(s_machine->dedicated_connections.connections));
      d_con != DC_NIL;
      d_con = (struct t_dedicated_connection *)
                next_queue(&(s_machine->dedicated_connections.connections)))
    {
      if (
        ((d_con->source_id == s_machine->id) &&
         (d_con->destination_id == d_machine->id)) || /* Sentit normal */
        ((d_con->source_id == d_machine->id) &&
         (d_con->destination_id == s_machine->id)))   /* Sentit invers */
      {
        /* La connexio es entre les dues maquines correctes. Nomes
           cal mirar si es pot utilitzar per aquest missatge concret. */
        if (connection_can_be_used(d_con, mess_tag, mess_size))
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

void
really_send_single_machine(struct t_thread *thread)
{
  struct t_node            *node, *node_partner;
  struct t_task            *task, *task_partner;
  struct t_thread          *partner = TH_NIL;  /* GRH (25/05/2008) */
  struct t_action          *action;
  struct t_account         *account;
  struct t_send            *mess;
  struct t_bus_utilization *bus_utilization;
  struct t_machine         *machine;
  t_micro                   ti, t_recursos;
  dimemas_timer             tmp_timer, tmp_timer2;
  int                       comm_type;

  node   = get_node_of_thread (thread);
  task   = thread->task;
  action = thread->action;
  mess   = &(action->desc.send);

  task_partner = locate_task (task->Ptask, mess->dest);
  node_partner = get_node_of_task (task_partner);

  /* Tots dos nodes son de la mateixa maquina */
  machine = node->machine;

  if (get_links (thread, node, node_partner))
  {

    /* JGG (05/11/2004): El 'comm_type' lo cogemos del mensaje, es más rápido
    comm_type =
      (node == node_partner ? LOCAL_COMMUNICATION_TYPE : INTERNAL_NETWORK_COM_TYPE);
    */

    comm_type = mess->comm_type;

    if (comm_type == INTERNAL_NETWORK_COM_TYPE)
    {
      if (machine->communication.num_messages_on_network)
      {
        if (machine->communication.policy == COMMUNIC_FIFO)
        {
          if (
            machine->network.curr_on_network >=
            machine->communication.num_messages_on_network
          )
          {
            if (debug&D_COMM)
            {
              PRINT_TIMER (current_time);
              printf(
               ": COMMUNIC_send\tP%02d T%02d (t%02d) Blocked (Bus Waiting)\n",
                IDENTIFIERS (thread)
              );
	          }
            inFIFO_queue (&machine->network.queue, (char *) thread);
            /* FEC: Comenc,a el temps que el thread passa esperant un bus */
            START_BUS_WAIT_TIME(thread);
            /**************************************************************/
#ifdef PARAVER_ALL
            Paraver_event (
              1,
              1,
              1,
              1,
              current_time,
              70,
              count_queue (&machine->network.queue)
            );
#endif
            return;
          }

          if (debug&D_COMM)
	        {
            PRINT_TIMER (current_time);
	          printf(": COMMUNIC_send\tP%02d T%02d (t%02d) Obtains bus\n",
	           IDENTIFIERS (thread));
	        }
        }
        machine->network.curr_on_network++;
        bus_utilization = (struct t_bus_utilization *)
          mallocame(sizeof(struct t_bus_utilization));
        bus_utilization->sender = thread;
        ASS_ALL_TIMER (bus_utilization->initial_time, current_time);
        inFIFO_queue (
          &machine->network.threads_on_network,
          (char *)bus_utilization
        );
      }
    }

    if (debug&D_COMM)
    {
      PRINT_TIMER (current_time);
      printf (
      ": COMMUNIC_send\tP%02d T%02d (t%02d) -> T%d Tag(%d) SEND (INTERNAL)\n",
		    IDENTIFIERS (thread),
        mess->dest,
        mess->mess_tag
      );
    }
    account = current_account(thread);
    SUB_TIMER(current_time, thread->initial_communication_time, tmp_timer);
    ADD_TIMER (tmp_timer,
    account->block_due_resources,account->block_due_resources);
    thread->physical_send = current_time;
    thread->last_paraver = current_time;
    ti = transferencia(mess->mess_size, comm_type, thread,NULL, &t_recursos);
    if (t_recursos>ti)
    {
      panic("resources > transmission time!\n");
    }
    /* Abans de programar la fi de la comunicacio, es programa la fi de la
     * utilització dels recursos reservats. */
    FLOAT_TO_TIMER (t_recursos, tmp_timer);
    ADD_TIMER (current_time, tmp_timer, tmp_timer);

    /* GRH (25/06/2008) RTT modification for rendez-vous protocol */
    /* Round Trip Time for receives - recvs*/
    if (RTT_enabled && mess->rendez_vous && (comm_type == INTERNAL_NETWORK_COM_TYPE))
    {
       struct t_thread          *partner; 
       t_micro                  roundtriptime;
       dimemas_timer            rtt_timer;
     
       if (thread->roundtrip_done == FALSE)
       {
             roundtriptime = RTT_time/2.0;
             if (RTT_time != (t_micro) 0) {
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

    if ((!venus_enabled) || (mess->comm_type != INTERNAL_NETWORK_COM_TYPE)) {
      EVENT_timer (tmp_timer, NOT_DAEMON, M_COM, thread, COM_TIMER_OUT_RESOURCES);
      thread->event =
        EVENT_timer (tmp_timer2, NOT_DAEMON, M_COM, thread, COM_TIMER_OUT);
    }
    else {
      double dtime;
      struct t_event *out_resources_ev;

      out_resources_ev = EVENT_venus_timer (tmp_timer, NOT_DAEMON, M_COM, thread, COM_TIMER_OUT_RESOURCES);
      thread->event =
        EVENT_venus_timer (tmp_timer2, NOT_DAEMON, M_COM, thread, COM_TIMER_OUT);

      TIMER_TO_FLOAT(current_time, dtime);
      dtime = thread->physical_send;
      if (mess->rendez_vous) {
        vc_command_rdvz_ready(dtime, node->nodeid - 1, node_partner->nodeid - 1, mess->mess_tag, mess->mess_size, thread->event, out_resources_ev);
      }
      else {
        vc_command_send(dtime, node->nodeid - 1, node_partner->nodeid - 1, mess->mess_size, thread->event, out_resources_ev);
      }
    }
  }
}

void
really_send_external_network (struct t_thread *thread)
{
  struct t_node    *node, *node_partner;
  struct t_task    *task, *task_partner;
  struct t_action  *action;
  struct t_account *account;
  struct t_send    *mess;
  t_micro           ti, t_recursos;
  dimemas_timer     tmp_timer;

  /* Si es descomenta el codi que reserva busos, tambe cal descomentar aixo:
  struct t_bus_utilization *bus_utilization; */

  node = get_node_of_thread (thread);
  task = thread->task;
  action = thread->action;
  mess = &(action->desc.send);

  task_partner = locate_task (task->Ptask, mess->dest);
  node_partner = get_node_of_task (task_partner);

  if (get_machine_links (thread, node->machine, node_partner->machine))
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
      (struct t_bus_utilization *) mallocame(sizeof(struct t_bus_utilization));
    bus_utilization->sender = thread;
    ASS_ALL_TIMER (bus_utilization->initial_time, current_time);
    inFIFO_queue (
      &sim_char.general_net.threads_on_network,
      (char *)bus_utilization
    );
  */
    if (debug&D_COMM)
    {
      PRINT_TIMER (current_time);
      printf (
      ": COMMUNIC_send\tP%02d T%02d (t%02d) -> T%d Tag(%d) SEND (EXTERNAL) message\n",
        IDENTIFIERS (thread),
        mess->dest,
        mess->mess_tag
      );
    }
    account = current_account(thread);
    SUB_TIMER(current_time, thread->initial_communication_time, tmp_timer);
    ADD_TIMER (
      tmp_timer,
      account->block_due_resources,
      account->block_due_resources
    );
    thread->physical_send = current_time;
    thread->last_paraver  = current_time;
    ti = transferencia(
           mess->mess_size,
           EXTERNAL_NETWORK_COM_TYPE,
           thread,
           NULL,
           &t_recursos
         );

    if (t_recursos>ti) panic("resources > transmission time!\n");
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

void
really_send_dedicated_connection(
  struct t_thread *thread,
  struct t_dedicated_connection *connection
)
{
  struct t_action  *action;
  struct t_account *account;
  struct t_send    *mess;
  t_micro           ti, t_recursos;
  dimemas_timer     tmp_timer;

  action = thread->action;
  mess   = &(action->desc.send);

  if (get_connection_links (thread, connection))
  {
    account = current_account(thread);
    SUB_TIMER(current_time, thread->initial_communication_time, tmp_timer);
    ADD_TIMER (
      tmp_timer,
      account->block_due_resources,
      account->block_due_resources
    );
    thread->physical_send = current_time;
    thread->last_paraver = current_time;
    ti = transferencia(
           mess->mess_size,
           DEDICATED_CONNECTION_COM_TYPE,
           thread,
           connection,
           &t_recursos
         );
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
        ": COMMUNIC\tP%02d T%02d (t%02d) -> T%d Tag(%d) SEND (DEDICATED) message\n",
        IDENTIFIERS (thread),
        mess->dest,
        mess->mess_tag
      );
    }
  }
}

void
really_send (struct t_thread *thread)
{
  struct t_task   *task,
                  *task_partner;
  struct t_action *action;
  struct t_send   *mess;
  int              kind;
  struct t_dedicated_connection *connection;

  task = thread->task;
  action = thread->action;
  mess = &(action->desc.send);

  task_partner = locate_task (task->Ptask, mess->dest);
  if (task_partner == T_NIL)
  {
    panic("Task partner not found!\n");
  }

  /* JGG: Ahora obtenemos el 'kind' por a partir del mensaje
  kind = get_communication_type(task,task_partner,mess->mess_tag,
                                mess->mess_size,&connection);
  */

  kind = mess->comm_type;


  if (debug&D_COMM)
  {
    PRINT_TIMER (current_time);
    printf (
      ": COMMUNIC_send\tP%02d T%02d (t%02d) -> T%d Tag: %d Type: %d (Really Send)\n",
	    IDENTIFIERS (thread),
      mess->dest,
      mess->mess_tag,
      mess->comm_type,
      kind
    );
  }


  switch (kind)
  {
    case LOCAL_COMMUNICATION_TYPE:
    case INTERNAL_NETWORK_COM_TYPE:
      really_send_single_machine(thread);
      break;
    case EXTERNAL_NETWORK_COM_TYPE:
      really_send_external_network(thread);
      break;
    case DEDICATED_CONNECTION_COM_TYPE:
      really_send_dedicated_connection(thread, connection);
      break;
    default:
      panic ("Incorrect communication type!");
      break;
  } /* end switch */
  return;
}

struct t_communicator *
locate_communicator(struct t_queue *communicator_queue, int commid)
{
  register struct t_communicator *communicator;

  for (communicator  = (struct t_communicator *)head_queue(communicator_queue);
       communicator != (struct t_communicator *)0;
       communicator  = (struct t_communicator *)next_queue(communicator_queue))
  {
    if (communicator->communicator_id==commid)
    {
      break;
    }
  }
  return (communicator);
}

static int
provide_log (int n)
{
    int i;

    for (i=0; i<n; i++)
        if ((1<<i)>=n)
            return(i);
    return (0);
}


static int
compute_contention_stage (int ntasks, int num_busos)
{
  int i;
  int j;
  int loga;
  int parallel_comm;
  int total = 0;

  loga = provide_log(ntasks);

  /* If unlimited number of buses, contention is number of stages */
  if (num_busos == 0)
  {
    return (loga);
  }

  for (i = 0; i<loga; i++)
  {
    parallel_comm = 0;
    for (j = (1<<i)-1; j<ntasks-1; j = j+(1<<i+1))
    {
#ifdef DEBUG
      if (j+(1<<i)<ntasks)
      {
        printf ("From %d to %d\n", j+1, j+(1<<i)+1);
      }
      else
      {
        printf ("From %d to %d\n", j+1, ntasks);
      }
#endif /* DEBUG */
      parallel_comm ++;
    }
    if (parallel_comm%num_busos==0)
    {
      total = total + parallel_comm / num_busos;
    }
    else
    {
      total = total + 1 + parallel_comm / num_busos;
    }
#ifdef DEBUG
    printf ("Stage %d parallel communications %d\n", i+1, parallel_comm);
#endif /* DEBUG */
  }
  return (total);
}


/**************************************************************************
 ** Ara aquesta funcio nomes reserva tot el que cal a la maquina
 ** corresponent al thread donat (si es pot i es la primera vegada que es
 ** crida. Sino suposa que ja s'han reservat tots els busos). I, si era
 ** l'ultim thread que quedava, crida a la funcio corresponent a l'inici
 ** de l'operacio col.lectiva.
 **************************************************************************/
static void
global_op_get_all_buses (struct t_thread *thread)
{
  struct t_Ptask *Ptask;
  struct t_communicator *communicator;
  struct t_global_op_definition * glop;
  struct t_bus_utilization *bus_utilization;
  struct t_action *action;
  int i, comm_id, glop_id;
  struct t_node *node;
  struct t_machine *machine;

  node    = get_node_of_thread (thread);
  machine = node->machine;

  if ((machine->communication.num_messages_on_network) &&
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
      START_BUS_WAIT_TIME(thread);
      /*************************************************************/
#ifdef PARAVER_ALL
      Paraver_event (
        1,
        1,
        1,
        1,
        current_time,
        70,
        count_queue (&machine->network.queue)
      );
#endif
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
            query_prio_queue(&Global_op, (t_priority)glop_id);

  if (glop == (struct t_global_op_definition *) 0 )
  {
    panic(
      "Global operation %d undefined to P%02d T%02d (t%02d)\n",
      glop_id,
      IDENTIFIERS (thread)
    );
  }

  machine->network.curr_on_network =
    machine->communication.num_messages_on_network;

  bus_utilization =
    (struct t_bus_utilization *) mallocame(sizeof(struct t_bus_utilization));

  bus_utilization->sender = thread;
  ASS_ALL_TIMER (bus_utilization->initial_time, current_time);
  for (i = 0; i<machine->communication.num_messages_on_network; i++)
  {
    inFIFO_queue (&machine->network.threads_on_network, (char*)bus_utilization);
  }

  if (debug&D_COMM)
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
  if (count_queue(&communicator->machines_threads) > 1)
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

    if (get_one_machine_link(thread, machine, OUT_LINK))
    {
      /* S'han pogut obtenir el link de sortida a la xarxa externa */
      /* A partir d'aqui aixo s'ha de fer en una funcio diferent perque
       * no es torni a fer la reserva dels busos anterior. */
      global_op_get_all_out_links(thread);
    }
    /* Si no s'han pogut obtenir s'haura bloquejat el thread i si s'han
     * pogut obtenir ja s'haura fet el que cal. En qualsevol cas, ja es
     * pot retornar. */
    return;
  }
  else /* Si no calen els links, ja es pot passar al seguent pas */
  {
    global_op_get_all_out_links(thread);
  }
}


/**************************************************************************
 ** Reserva els links necessaris i despres crida a global_op_get_all_links.
 **************************************************************************/
void
global_op_reserva_links (struct t_thread *thread)
{
  struct t_node *node;
  struct t_machine *machine;
  struct t_Ptask *Ptask;
  struct t_communicator *communicator;
  struct t_action *action;
  int comm_id;

  node = get_node_of_thread (thread);
  machine = node->machine;

  Ptask = thread->task->Ptask;
  action = thread->action;
  comm_id = action->desc.global_op.comm_id;
  communicator = locate_communicator (&Ptask->Communicator, comm_id);
  if (communicator==(struct t_communicator *)0)
    panic ("Global op get links trough an invalid communicator %d to P%02d T%02d (t%02d)\n",
         comm_id, IDENTIFIERS (thread));

  /* Cal saber si s'estan reservant els links d'entrada o de sortida */
  if (count_queue(&communicator->machines_threads) !=
      count_queue(&communicator->m_threads_with_links))
  {
    /* Encara falten algunes maquines per tenir tots els out_links */
    if (get_one_machine_link(thread, machine, OUT_LINK))
    {
      /* S'han pogut obtenir els links de sortida a la xarxa externa*/
      global_op_get_all_out_links(thread);
    }
  }
  else
  {
    /* Ja es tenen tots els links de sortida, es busquen els d'entrada */
    if (get_one_machine_link(thread, machine, IN_LINK))
    {
      /* S'han pogut obtenir els links d'entrada de la xarxa externa*/
      global_op_get_all_in_links(thread);
    }
  }

}



/**************************************************************************
 ** Aquesta funcio suposa que ja s'han reservat tots els links de sortida
 ** de la maquina corresponent, cap a la xarxa externa. I, si era l'ultim
 ** thread que quedava, crida a la funcio corresponent a l'inici de la
 ** fase de reserva dels links d'entrada de la xarxa externa.
 **************************************************************************/
static void
global_op_get_all_out_links (struct t_thread *thread)
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
    panic (
      "Communication get_links trough an invalid communicator %d to P%02d T%02d (t%02d)\n",
      comm_id,
      IDENTIFIERS (thread)
    );
  }

  glop_id = action->desc.global_op.glop_id;
  glop    = (struct t_global_op_definition *)
            query_prio_queue(&Global_op, (t_priority)glop_id);

  if (glop == GOPD_NIL)
  {
    panic(
      "Global operation %d undefined to P%02d T%02d (t%02d)\n",
      glop_id,
      IDENTIFIERS (thread)
    );
  }

  /* Si cal obtenir mes links de la xarxa externa, cal fer-ho aqui */
  if (count_queue(&communicator->machines_threads) > 1)
  {
    /* D'aquesta maquina ja es disposa del link de sortida i dels busos,
       per tant, s'ha d'indicar aixo i, si era l'ultima, ja es pot
       comenc,ar la seguent fase. En cas contrari,
       s'ha d'encuar aquest thread i seguir esperant. */

    /* S'afegeix el thread a la cua d'aquest communicador de threads que ja
       tenen el link de sortida reservat. */
    inFIFO_queue (&communicator->m_threads_with_links, (char *)thread);

    /* Es mira si ja es tenen tots */
    if (count_queue(&communicator->m_threads_with_links) !=
        count_queue(&communicator->machines_threads))
    {
      /* Encara falten alguns threads per acabar d'obtenir tots els
         links de sortida. */
      return;
    }

    /* Ja es pot comenc,ar a la fase de reserva de links d'entrada */
    for(
      others  = (struct t_thread *) head_queue(&communicator->machines_threads);
      others != TH_NIL;
      others  = (struct t_thread *) next_queue(&communicator->machines_threads)
    )
    {
      /* Aquest thread reserva els recursos necessaris que falten de la
         seva maquina */
      node    = get_node_of_thread (others);
      machine = node->machine;

      if (get_one_machine_link(others, machine, IN_LINK))
      {
        /* S'han pogut obtenir els links d'entrada de la xarxa externa*/
        global_op_get_all_in_links(others);
      }
    }
    /* Si no s'han pogut obtenir s'haura bloquejat el thread i si s'han
       pogut obtenir ja s'haura fet el que cal. En qualsevol cas, ja es
       pot retornar. */
    return;

  }
  else /* Si no calen els links, ja es pot passar al seguent pas */
  {
    global_op_get_all_in_links(thread);
  }
}


/**************************************************************************
 ** Aquesta funcio suposa que ja s'han reservat tots els links de la
 ** maquina corresponent, cap a la xarxa externa. I, si era l'ultim thread
 ** que quedava, crida a la funcio corresponent a l'inici de l'operacio
 ** col.lectiva.
 **************************************************************************/
static void
global_op_get_all_in_links (struct t_thread *thread)
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
            query_prio_queue(&Global_op, (t_priority)glop_id);

  if (glop == GOPD_NIL)
  {
    panic(
      "Global operation %d undefined to P%02d T%02d (t%02d)\n",
      glop_id,
      IDENTIFIERS (thread)
    );
  }

  /* D'aquesta maquina ja es disposa de tot el que es necessita, per tant,
     si era l'ultima, ja es pot comenc,ar la col.lectiva. En cas contrari,
     s'ha d'encuar aquest thread i seguir esperant. */

  /* S'afegeix el thread a la cua de threads del communicador */
  inFIFO_queue (&communicator->threads, (char *)thread);

  if (count_queue(&communicator->threads) !=
      count_queue(&communicator->global_ranks))
  {
    /* Encara falten alguns threads per acabar d'obtenir tot el que
       es necessita. */
    return;
  }

  /* Ja es pot comenc,ar a realitzar l'operacio col.lectiva */

  /* Es buida la cua m_threads_with_links perque ja no es necessita */
  for (
    others =
      (struct t_thread *)outFIFO_queue(&communicator->m_threads_with_links);
    others != TH_NIL;
    others =
      (struct t_thread *)outFIFO_queue(&communicator->m_threads_with_links)
  )
  {}

  /* Search the root task */
  root_task =
    from_rank_to_taskid (communicator, action->desc.global_op.root_rank);

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
    exit(1);
  }
  /* NO ES TREU el thread de root de la cua. Els tracto tots igual! */

  /* S'inicia l'operacio col.lectiva !! */
  start_global_op(others);
}


/**************************************************************************
 ** Aquesta funcio calcula els temps estimats de latenica i temps total
 ** per una etapa d'una operacio col.lectiva.
 **************************************************************************/
void
calcula_fan(
  t_micro bandw,    /* MBytes per segon */
  int num_tasks,    /* Numero de tasks d'aquesta etapa */
  int num_busos,    /* Numero de busos disponibles */
  int tipus_de_fan, /* 0 = IN / 1 = OUT */
  int model,        /* 0, CONSTANT, LINEAL, LOGARITHMIC */
  int size_type,    /* MIN, MAX, average, 2*MAX, send+recv */
  int bytes_send,   /* Number of bytes send */
  int bytes_recvd,  /* Number of bytes received */
  t_micro startup,
  t_micro *temps,
  t_micro *latencia
)
{
  int mes_si;
  float fo;

  /* Aqui es passa de Mbytes/sec a microsegons/byte. */
  bandw = bandwidth_2_ms_per_byte(bandw);

  /* Computation of global communication model */
  switch (size_type)
  {
    case GOP_SIZE_CURR:
      if (tipus_de_fan== FAN_IN)
      {
        mes_si = bytes_send;
      }
      else if (tipus_de_fan == FAN_OUT)
      {
        mes_si = bytes_recvd;
      }
      break;
    case GOP_SIZE_MIN:
      mes_si = MIN(bytes_send,bytes_recvd);
      break;
    case GOP_SIZE_MAX:
      mes_si = MAX(bytes_send,bytes_recvd);
      break;
    case GOP_SIZE_MEAN:
      mes_si = (bytes_send+bytes_recvd) / 2;
      break;
    case GOP_SIZE_2MAX:
      mes_si = 2*MAX(bytes_send,bytes_recvd);
      break;
    case GOP_SIZE_SIR:
      mes_si = (bytes_send+bytes_recvd);
      break;
    default:
      panic ("Invalid size FIN/OUT %d for global operation\n", size_type);
      exit(1);
      break;
  }
  switch (model)
  {
    case GOP_MODEL_0:
      *temps    = 0;
      *latencia = 0;
      break;
    case GOP_MODEL_CTE:
      fo        = 1.0;
      *temps    = startup + mes_si * bandw;
      *latencia = startup;
      break;
    case GOP_MODEL_LIN:
      fo        = (t_micro) num_tasks;
      *temps    = (startup + mes_si * bandw) * fo;
      *latencia = startup * fo;
      break;
    case GOP_MODEL_LOG:
      fo        = compute_contention_stage(num_tasks,num_busos);
      *temps    = (startup + mes_si * bandw) * fo;
      *latencia = startup * fo;
      break;
    default:
      panic ("Invalid model FIN/OUT %d for global operation\n", model);
      exit(1);
  }

}



/**************************************************************************
 ** Es calculen els temps dins de cada node de la maquina i
 ** s'agafen els maxims.
 **************************************************************************/
static void
calcula_temps_maxim_intra_nodes(
  struct t_machine *machine,
  struct t_global_op_information *glop_info,
  int bytes_send,   /* Number of bytes send */
  int bytes_recvd,  /* Number of bytes received */
  t_micro *max_tnode_in,
  t_micro *max_lnode_in,
  t_micro *max_tnode_out,
  t_micro *max_lnode_out
)
{
  struct t_node  *node;
  t_micro tauxn_in, lauxn_in, tauxn_out, lauxn_out;
  int num_cpus;
  double suma_aux, suma_maxim;

  /* S'inicialitzen els maxims */
  *max_tnode_in=*max_lnode_in=*max_tnode_out=*max_lnode_out=0;
  suma_maxim=0;

  /* Es calcula a tots els nodes de la maquina */
  for (node  = (struct t_node *)head_queue(&Node_queue);
       node != (struct t_node *)0;
       node  = (struct t_node *)next_queue(&Node_queue))
  {
    /* Nomes s'agafen els nodes d'aquesta maquina. */
    if (node->machine != machine) continue;

    num_cpus = count_queue(&(node->Cpus));

    /* Cal calcular els temps dins d'aquest node */
    calcula_fan(
      node->bandwith,
      num_cpus,
      0, /* Infinits busos */
      0, /* FAN IN */
      glop_info->FIN_model,
      glop_info->FIN_size,
      bytes_send,
      bytes_recvd,
      node->local_startup,
      &tauxn_in,
      &lauxn_in
    );

    calcula_fan(
      node->bandwith,
      num_cpus,
      0, /* Infinits busos */
      1, /* FAN OUT */
      glop_info->FOUT_model, glop_info->FOUT_size,
      bytes_send,
      bytes_recvd,
      node->local_startup,
      &tauxn_out,
      &lauxn_out
    );

    /* S'agafen els del node amb la suma mes gran */
    suma_aux = (double)(tauxn_in+lauxn_in+tauxn_out+lauxn_out);
    if (suma_aux>suma_maxim)
    {
      /* S'agafa aquest node com a node de temps maxim d'aquesta maquina */
      suma_maxim=suma_aux;
      *max_tnode_in=tauxn_in;
      *max_lnode_in=lauxn_in;
      *max_tnode_out=tauxn_out;
      *max_lnode_out=lauxn_out;
    }
  }

  /* Es retornen els temps del node de temps total maxim */
}


/**************************************************************************
 ** Es calcula el flight time maxims d'entrada i el maxim de sortida
 ** del thread donat.
 **************************************************************************/
static void
calcula_maxim_flight_times(
  struct t_thread *thread,
  struct t_communicator *communicator,
  t_micro *maxflight_in,
  t_micro *maxflight_out
)
{
  t_micro max_flight_in, max_flight_out;
  struct t_node  *node;
  struct t_machine *machine;
  int index_origen;
  struct t_thread *others;
  double flight_time_in, flight_time_out;

  node         = get_node_of_thread (thread);
  machine      = node->machine;
  index_origen = machine->id-1;

  max_flight_in  = 0;
  max_flight_out = 0;

  /* Es calcula el temps de cada maquina */
  for (others=(struct t_thread *)head_queue(&communicator->machines_threads);
       others!=(struct t_thread *)0;
       others=(struct t_thread *)next_queue(&communicator->machines_threads))
  {
    /* No te sentit calcular el flight time a si mateixa */
    if (others == thread) continue;

    /* S'obte el node i la maquina corresponent al thread */
    node    = get_node_of_thread (others);
    machine = node->machine;

    /* S'agafen els flight times */
    flight_time_in  =
      sim_char.general_net.flight_times[machine->id-1][index_origen];
    flight_time_out =
      sim_char.general_net.flight_times[index_origen][machine->id-1];

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
void
calcula_temps_operacio_global(
  struct t_thread *thread,
  dimemas_timer   *temps_latencia,
  dimemas_timer   *temps_recursos,
  dimemas_timer   *temps_final
)
{
  struct t_Ptask                 *Ptask;
  struct t_communicator          *communicator;
  struct t_global_op_definition  *glop;
  struct t_thread                *others;
  struct t_action                *action;
  struct t_node                  *node;
  struct t_machine               *machine;
  struct t_global_op_information *glop_info;
  int comm_id, glop_id, num_maquines;

  t_micro tfin_node, lfin_node, tfout_node, lfout_node;
  t_micro tfin_int,  lfin_int,  tfout_int,  lfout_int;
  t_micro tfin_ext,  lfin_ext,  tfout_ext,  lfout_ext;
  t_micro flightin_ext, flightout_ext;
  t_micro temps, latencia, t_recursos; /* Temps totals */
  t_micro taux_in, laux_in, taux_out, laux_out;
  t_micro tauxn_in, lauxn_in, tauxn_out, lauxn_out;
  t_micro bandw_externa;
  double  suma_aux, suma_maxim;

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
    query_prio_queue(&Global_op, (t_priority)glop_id);

  if (glop == GOPD_NIL)
  {
    panic(
      "Global operation %d undefined to P%02d T%02d (t%02d)\n",
      glop_id,
      IDENTIFIERS (thread)
    );
  }

  /* S'inicialitzen els temps */
  tfin_node    = lfin_node = tfout_node = lfout_node = 0;
  tfin_int     = lfin_int  = tfout_int  = lfout_int  = 0;
  tfin_ext     = lfin_ext  = tfout_ext  = lfout_ext  = 0;
  flightin_ext = flightout_ext = 0;
  suma_maxim   = 0;

  /* Es calcula el temps de cada maquina */
  for (others  =(struct t_thread *)head_queue(&communicator->machines_threads);
       others != TH_NIL;
       others  =(struct t_thread *)next_queue(&communicator->machines_threads))
  {
    /* S'obte el node i la maquina corresponent al thread */
    node    = get_node_of_thread (others);
    machine = node->machine;

    /* S'obte la informacio de les operacions col.lectives dins
       d'aquesta maquina. */
    glop_info = (struct t_global_op_information *)
      query_prio_queue(
        &machine->communication.global_ops_info,
        (t_priority)glop_id
      );

    if (glop_info == (struct t_global_op_information *)0)
    {
      panic(
        "Global operation %d undefined to P%02d T%02d (t%02d)\n",
        glop_id,
        IDENTIFIERS (others)
      );
    }

    /* Es calculen els temps entre nodes d'aquesta maquina */
    calcula_fan(
      machine->communication.remote_bandwith,
      machine->number_nodes,
      machine->communication.num_messages_on_network,
      FAN_IN,
      glop_info->FIN_model,
      glop_info->FIN_size,
      action->desc.global_op.bytes_send,
      action->desc.global_op.bytes_recvd,
      node->remote_startup,
      &taux_in,
      &laux_in
    );

    calcula_fan(
      machine->communication.remote_bandwith,
      machine->number_nodes,
      machine->communication.num_messages_on_network,
      FAN_OUT,
      glop_info->FOUT_model,
      glop_info->FOUT_size,
      action->desc.global_op.bytes_send,
      action->desc.global_op.bytes_recvd,
      node->remote_startup,
      &taux_out,
      &laux_out
    );

    /* Es calculen els temps dins de cada node de la maquina i
       s'agafen els maxims. */
    calcula_temps_maxim_intra_nodes(
      machine,
      glop_info,
      action->desc.global_op.bytes_send,
      action->desc.global_op.bytes_recvd,
      &tauxn_in,
      &lauxn_in,
      &tauxn_out,
      &lauxn_out
    );

    /* Es calcula la suma total de temps entre nodes d'aquesta maquina
       mes els temps del node d'aquesta maquina de mes durada */
    suma_aux  = (double)(taux_in+laux_in+taux_out+laux_out);
    suma_aux += (double)(tauxn_in+lauxn_in+tauxn_out+lauxn_out);

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
*/

    if (debug&D_COMM)
	  {
	    PRINT_TIMER (current_time);
	    printf (
        ": Machine %d: tfin_int=%f lfin_int=%f tfout_int=%f lfout_int=%f\n",
        machine->id,
        taux_in,
        laux_in,
        taux_out,
        laux_out
      );
	  }
  }

  /* Cal contar el temps a la xarxa externa */
  num_maquines = count_queue(&(communicator->machines_threads));
  if (num_maquines > 1)
  {
    /* S'obte la informacio de les operacions col.lectives a la
       xarxa externa. */
    glop_info = (struct t_global_op_information *)
      query_prio_queue(
        &sim_char.general_net.global_ops_info,
        (t_priority)glop_id
      );

    if (glop_info == (struct t_global_op_information *)0)
    {
      panic(
        "Global operation %d undefined to P%02d T%02d (t%02d)\n",
        glop_id,
        IDENTIFIERS (thread)
      );
    }

    /* Aqui hi ha d'haver el recompute_external_netwrok_bandewidth */
    if (sim_char.general_net.bandwidth != 0)
    {
      /*recompute_external_network_traffic(size???); */
      bandw_externa = recompute_external_network_bandwidth (thread);
    }
    else
    {
      bandw_externa = (t_micro) 0;
    }

    /* Es calculen els temps */
    calcula_fan(
      bandw_externa,
      num_maquines,
      0, /* Infinits busos */
      FAN_IN,
      glop_info->FIN_model, glop_info->FIN_size,
      action->desc.global_op.bytes_send,
      action->desc.global_op.bytes_recvd,
      node->external_net_startup,
      &tfin_ext,
      &lfin_ext
    );

    calcula_fan(
      bandw_externa,
      num_maquines,
      0, /* Infinits busos */
      FAN_OUT, /* FAN OUT */
      glop_info->FOUT_model, glop_info->FOUT_size,
      action->desc.global_op.bytes_send,
      action->desc.global_op.bytes_recvd,
      node->external_net_startup,
      &tfout_ext,
      &lfout_ext
    );

    /* Es calculen els flight times */
    calcula_maxim_flight_times(
      thread,
      communicator,
      &flightin_ext,
      &flightout_ext
    );

    if (debug&D_COMM)
	  {
	    PRINT_TIMER (current_time);
	    printf (
        ": External net: tfin_ext=%f lfin_ext=%f tfout_ext=%f lfout_ext=%f\n",
        tfin_ext,
        lfin_ext,
        tfout_ext,
        lfout_ext
      );
	    PRINT_TIMER (current_time);
	    printf (
        ": Num maquines %d startup xarxa externa %f\n",
        num_maquines,
        node->external_net_startup
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

  if (debug&D_COMM)
  {
    PRINT_TIMER (current_time);
    printf(": COMMUNIC calcula_temps_operacio_global temps ");
    PRINT_TIMER (*temps_final);
    printf(" recursos ");
    PRINT_TIMER (*temps_recursos);
    printf(" latencia ");
    PRINT_TIMER (*temps_latencia);
    printf("\n");
  }
}

/**************************************************************************
 ** Aquesta funcio no reserva res. Dona per suposat que ja s'ha reservat
 ** tot el que calia. Simplement engega l'operacio col.lectiva.
 **************************************************************************/
static void
start_global_op (struct t_thread *thread)
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
  dimemas_timer temps_latencia, temps_recursos, temps_final;

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
    query_prio_queue(&Global_op, (t_priority)glop_id);

  if (glop == GOPD_NIL)
  {
    panic(
      "Global operation %d undefined to P%02d T%02d (t%02d)\n",
      glop_id,
      IDENTIFIERS (thread)
    );
  }


  if (debug&D_COMM)
  {
    PRINT_TIMER (current_time);
    printf (
      ": GLOBAL_operation P%02d T%02d (t%02d) (Root Task) Unlocks '%s'\n",
      IDENTIFIERS (thread),
      glop->name
    );
  }


  calcula_temps_operacio_global(
    thread,
    &temps_latencia,
    &temps_recursos,
    &temps_final
  );

  /* Es programa l'event de final de la reserva dels recursos */
  ADD_TIMER (current_time, temps_recursos, tmp_timer);
  EVENT_timer (
    tmp_timer,
    NOT_DAEMON,
    M_COM,
    thread,
    COM_TIMER_GROUP_RESOURCES
  );



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
  for (
    others  = (struct t_thread *) head_queue(&communicator->threads);
    others != TH_NIL;
    others  = (struct t_thread *) next_queue(&communicator->threads)
  )
  {
    ASS_ALL_TIMER(others->collective_timers.with_resources, current_time);

    Vampir_GlobalOp (
      current_time,
      tmp_timer,
      glop_id, others,
      comm_id,
      thread,
      action->desc.global_op.bytes_send,
      action->desc.global_op.bytes_recvd
    );

    Paraver_thread_wait (
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
      cpu = get_cpu_of_thread(others);
      Paraver_thread_startup (
        0,
        IDENTIFIERS (others),
        current_time,
        tmp_timer2
      );
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
            query_prio_queue(&Global_op, (t_priority)glop_id);

  if (glop  ==  GOPD_NIL)
  {
    panic(
      "Global operation %d undefined to P%02d T%02d (t%02d)\n",
      glop_id,
      IDENTIFIERS (thread)
    );
  }

  if (debug&D_COMM)
  {
    PRINT_TIMER (current_time);
    printf (
      ": GLOBAL_operation P%02d T%02d (t%02d) Free resources used in '%s'\n",
      IDENTIFIERS(thread),
      glop->name
    );
  }

  /* Mirem el numero de maquines implicades abans de comenc,ar a
     desencuar threads. */
  num_maquines = count_queue(&communicator->machines_threads);

  /* Per cada maquina utilitzada s'alliberen tots els recursos reservats i
   * es treu el thread de la cua de maquines utilitzades per tal que la
   * cua ja quedi buida per la propera vegada. */
/*  for (others = (struct t_thread *)head_queue(&communicator->machines_threads);
       others != TH_NIL;
       others = (struct t_thread *)next_queue(&communicator->machines_threads))
*/
  for (
    others  = (struct t_thread *)outFIFO_queue(&communicator->machines_threads);
    others != TH_NIL;
    others  = (struct t_thread *)outFIFO_queue(&communicator->machines_threads)
  )
  {
    /* S'obte la maquina corresponent al thread */
    node    = get_node_of_thread (others);
    machine = node->machine;

    /* S'alliberen els busos */
    others->number_buses             = 0;
    machine->network.curr_on_network = 0;

    for (i=0; i<machine->communication.num_messages_on_network;i++)
    {
      bus_utilization = (struct t_bus_utilization *)
                        outFIFO_queue (&machine->network.threads_on_network);
    }
    /* Nomes s'ha reservat memoria una vegada i, per tant, nomes s'ha
       d'alliberar una vegada. */
    if (i>0)
    {
      freeame ((char *)bus_utilization, sizeof(struct t_bus_utilization));
    }

    /* Si hi havia més d'una màquina, aqui cal alliberar els links
       de la xarxa externa d'aquesta maquina. */
    if (num_maquines > 1)
    {
      free_machine_link(others->local_link, others);
      free_machine_link(others->partner_link, others);
      /* En aquest cas cal posar explicitament els links a L_NIL perque
         en les operacions col.lectives no es treballa amb copies dels
         threads, sino amb els threads originals. */
      others->local_link=L_NIL;
      others->partner_link=L_NIL;
    }


    /* Intent d'alliberar els threads (No d'aquesta col.lectiva) que podien
       estar bloquejats esperant a que hi hagui busos lliures en aquesta
       maquina */
    if (count_queue (&machine->network.queue)> 0)
    {
      num_elements=count_queue (&machine->network.queue);
      for(
        i=0;
        ((i<num_elements) &&
        (machine->network.curr_on_network<machine->communication.num_messages_on_network));
        i++
      )
      {
        wait_thread = (struct t_thread *) head_queue (&machine->network.queue);

#ifdef PARAVER_ALL
        Paraver_event (1, 1, 1, 1, current_time, 70, count_queue (&machine->network.queue));
#endif

        if (debug&D_COMM)
        {
          PRINT_TIMER (current_time);
          printf(
            ": GLOBAL_operation P%02d T%02d (t%02d) goes unlock and obtain bus\n",
            IDENTIFIERS (wait_thread)
          );
        }
        action = wait_thread->action;
        switch (action->action)
        {
          case SEND:
             /* FEC: S'acumula el temps que ha estat esperant busos */
             ACCUMULATE_BUS_WAIT_TIME(wait_thread);
             /*******************************************************/
             extract_from_queue(&machine->network.queue, (char *) wait_thread);
             really_send (wait_thread);
             break;

          case MPI_OS:
             /* FEC: S'acumula el temps que ha estat esperant busos */
             ACCUMULATE_BUS_WAIT_TIME(wait_thread);
             /*******************************************************/
             extract_from_queue(&machine->network.queue, (char *) wait_thread);
             really_RMA (wait_thread);
             break;

          case GLOBAL_OP:
             aux=machine->communication.num_messages_on_network-
                 machine->network.curr_on_network;
             wait_thread->number_buses+=aux;
             machine->network.curr_on_network+=aux;
             if (wait_thread->number_buses==machine->communication.num_messages_on_network)
             {
               /* FEC: S'acumula el temps que ha estat esperant busos */
               ACCUMULATE_BUS_WAIT_TIME(wait_thread);
               /*******************************************************/
               extract_from_queue(&machine->network.queue, (char *) wait_thread);
               global_op_get_all_buses(wait_thread);
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
static void
close_global_communication(struct t_thread *thread)
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

  Ptask        = thread->task->Ptask;
  action       = thread->action;
  comm_id      = action->desc.global_op.comm_id;
  communicator = locate_communicator (&Ptask->Communicator, comm_id);

  if (communicator == COM_NIL)
  {
    panic (
      "Communication close trough an invalid communicator %d to P%02d T%02d (t%02d)\n",
      comm_id,
      IDENTIFIERS (thread)
    );
  }

  glop_id = action->desc.global_op.glop_id;
  glop    = (struct t_global_op_definition *)
            query_prio_queue(&Global_op, (t_priority)glop_id);

  if (glop == GOPD_NIL)
  {
    panic(
      "Global operation %d undefined to P%02d T%02d (t%02d)\n",
      glop_id,
      IDENTIFIERS (thread)
    );
  }

  if (debug&D_COMM)
  {
    PRINT_TIMER (current_time);
    printf (
      ": GLOBAL_operation P%02d T%02d (t%02d) ends '%s'\n",
      IDENTIFIERS (thread),
      glop->name
    );
  }

  /* Unblock all threads involved in communication */
  for (
    others  = (struct t_thread *)outFIFO_queue(&communicator->threads);
    others != TH_NIL;
    others  = (struct t_thread *)outFIFO_queue(&communicator->threads)
  )
  {
    if (debug&D_COMM)
    {
      PRINT_TIMER (current_time);
      printf (
        ": GLOBAL_operation P%02d T%02d (t%02d) ends '%s'\n",
        IDENTIFIERS (others),
        glop->name
      );
    }

    node = get_node_of_thread (others);
    new_cp_node (others, CP_BLOCK);
    new_cp_relation (others, thread);

    Paraver_thread_wait (
      0,
      IDENTIFIERS (others),
      others->last_paraver,
      current_time,
      PRV_BLOCKING_RECV_ST
    );

    ASS_ALL_TIMER(
      others->collective_timers.conclude_communication,
      current_time
    );

    action = others->action;

    Paraver_Global_Op (
      node->nodeid,
      IDENTIFIERS (others),
      others->collective_timers.arrive_to_collective,
      others->collective_timers.sync_time,
      others->collective_timers.with_resources,
      others->collective_timers.conclude_communication,
      comm_id,
      action->desc.global_op.bytes_send,
      action->desc.global_op.bytes_recvd,
      glop_id,
      action->desc.global_op.root_rank
    );

    account = current_account (others);

    SUB_TIMER (
      others->collective_timers.with_resources,
      others->collective_timers.arrive_to_collective,
      tmp_timer
    );

    ADD_TIMER (
      account->block_due_group_operations,
      tmp_timer,
      account->block_due_group_operations
    );

    SUB_TIMER (
      others->collective_timers.conclude_communication,
      others->collective_timers.with_resources,
      tmp_timer
    );

    ADD_TIMER (
      account->group_operations_time,
      tmp_timer,
      account->group_operations_time
    );

    others->last_paraver = current_time;
    cpu                  = get_cpu_of_thread(others);

    Paraver_event (
      cpu->unique_number,
      IDENTIFIERS (others),
      current_time,
      PARAVER_GROUP_FREE,
      glop_id
    );

    action         = others->action;
    others->action = action->next;
    freeame ((char *) action, sizeof (struct t_action));

    if (more_actions (others))
    {
       others->loose_cpu = TRUE;
       SCHEDULER_thread_to_ready (others);
       reload_done=TRUE;
    }
  }
}

static t_boolean
thread_in_communicator (struct t_communicator *comm, struct t_thread *thread)
{
  int *taskid;

  for (
    taskid  = (int *) head_queue (&comm->global_ranks);
    taskid != (int *)0;
    taskid  = (int *) next_queue (&comm->global_ranks)
  )
  {
    if (*taskid == (thread->task->taskid))
    {
      return (TRUE);
    }
  }
  return(FALSE);
}

int
from_rank_to_taskid (struct t_communicator *comm, int root_rank)
{
  int *root_task;
  int  i;

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
}


void
GLOBAL_operation (
  struct t_thread *thread,
  int glop_id,
  int comm_id,
  int root_rank,
  int root_thid,
  int bytes_send,
  int bytes_recv
)
{
  struct t_Ptask                *Ptask;
  struct t_communicator         *communicator;
  struct t_global_op_definition *glop;
  struct t_thread               *others, *root_th;
  struct t_account              *account;
  struct t_cpu                  *cpu;
  struct t_node                 *node_usat;
  struct t_machine              *maquina_usada;

  int Total_threads_involved;
  int root_task;
  int i, kind;

  t_boolean trobat;

  account = current_account (thread);
  account->n_group_operations++;

  Ptask = thread->task->Ptask;
  communicator = locate_communicator (&Ptask->Communicator, comm_id);
  if (communicator == COM_NIL)
  {
    panic (
      "Communication start trough an invalid communicator %d to P%02d T%02d (t%02d)\n",
      comm_id,
      IDENTIFIERS (thread)
    );
  }

  if (thread_in_communicator (communicator, thread) == FALSE)
  {
    panic ("Thread P%02d T%02d (t%02d) not valid for communicator %d\n",
           IDENTIFIERS (thread),
           comm_id);
  }

  glop = (struct t_global_op_definition *)
    query_prio_queue(&Global_op, (t_priority)glop_id);

  if (glop == GOPD_NIL)
  {
    panic(
      "Global operation %d undefined to P%02d T%02d (t%02d)\n",
      glop_id,
      IDENTIFIERS (thread)
    );
  }

  if (debug&D_COMM)
  {
    PRINT_TIMER (current_time);
    printf (
      ": GLOBAL_operation P%02d T%02d (t%02d) Starting %s\n",
      IDENTIFIERS (thread),
      glop->name
    );
  }

  Total_threads_involved = count_queue(&communicator->global_ranks);

  /* FEC: Debugant! */
/*
  printf(
    "El comid de l'operacio global es %d, te %d threads i %d estan bloquejats\n",
    comm_id,
    Total_threads_involved,
    count_queue(&communicator->threads)
  );
*/
  ASS_ALL_TIMER(thread->collective_timers.arrive_to_collective, current_time);

  if (Total_threads_involved != count_queue(&communicator->threads)+1)
  {
    /* This is not the last thread arriving to the communication point,
     * simply block */

    cpu = get_cpu_of_thread(thread);
    Paraver_event (
      cpu->unique_number,
      IDENTIFIERS (thread),
      current_time,
      PARAVER_GROUP_BLOCK,
      glop_id
    );
    thread->last_paraver = current_time;

    inFIFO_queue (&communicator->threads, (char *)thread);

    if (debug&D_COMM)
    {
      PRINT_TIMER (current_time);
      printf (
        ": GLOBAL_operation P%02d T%02d (t%02d) Blocked on '%s' (%d: %dw / %dT)\n",
        IDENTIFIERS (thread),
        glop->name,
        comm_id,
        count_queue(&communicator->threads),
        Total_threads_involved
      );
    }

    Activity_Enter (FALSE,thread, 3);
    return;
  }

  /* Ja jan arribat tots els threads a aquest punt */
  if (debug&D_COMM)
  {
    PRINT_TIMER (current_time);
    printf (
      ": GLOBAL_operation P%02d T%02d (t%02d) Initiate '%s' (%d: %dT)\n",
      IDENTIFIERS (thread),
      glop->name,
      comm_id,
      Total_threads_involved
    );
  }

  cpu = get_cpu_of_thread(thread);
  Paraver_event (
    cpu->unique_number,
    IDENTIFIERS (thread),
    current_time,
    PARAVER_GROUP_LAST,
    glop_id
  );

  thread->number_buses = 0;
  thread->last_paraver = current_time;

  ASS_ALL_TIMER(thread->collective_timers.sync_time, current_time);

  for (
    others  = (struct t_thread *)head_queue(&communicator->threads);
    others != TH_NIL;
    others  = (struct t_thread *)next_queue(&communicator->threads)
  )
  {
    ASS_ALL_TIMER(others->collective_timers.sync_time, current_time);
    Paraver_thread_wait (
      0,
      IDENTIFIERS (others),
      others->last_paraver,
      current_time,
      PRV_BLOCKING_RECV_ST
    );
    others->last_paraver = current_time;
    Activity_Exit (FALSE,others, 3);
  }

  Activity_Enter (FALSE,thread, 3);
  Activity_Exit  (FALSE,thread, 3);

  /* Search the root task */
  inFIFO_queue (&communicator->threads, (char *)thread);
  root_task = from_rank_to_taskid (communicator, root_rank);

  /* Es busca el thread de la root task */
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
    exit(1);
  }
  /* El thread 'others' pertany a la 'root_task' */
  root_th = others;


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
  insert_queue(
    &communicator->machines_threads,
    (char *)others,
    maquina_usada->id
  );
  /* Es treu de la llista de threads per poder detectar quan esta tot reservat*/
  extract_from_queue (&communicator->threads, (char *)others);

  /* Es fa el mateix amb el primer thread trobat de cada nova maquina */
  /* Aquest altre tipus de bucle no funcionava perque hi ha problemes si es
   * treu de la llista el primer element. En aquest cas el next retornava
   * NULL abans d'hora.
    for (others=(struct t_thread *)head_queue(&communicator->threads);
         others!=(struct t_thread *)0;
         others=(struct t_thread *)next_queue(&communicator->threads))
   */
  others = (struct t_thread *) head_queue(&communicator->threads);

  for (i=0; i < (Total_threads_involved - 1); i++)
  {
    /* S'obte la maquina corresponent a aquest thread */
    node_usat     = get_node_of_thread (others);
    maquina_usada = node_usat->machine;
    /* Es mira si ja hem trobat un thread d'aquesta maquina */
    if
    ( query_prio_queue(&communicator->machines_threads, maquina_usada->id) ==
      A_NIL
    )
    {
      /* Es una maquina nova */
      /* S'afegeix el thread a la llista de maquines utilitzades */
      insert_queue(
        &communicator->machines_threads,
        (char *)others,
        maquina_usada->id
      );
      /* Es treu de la llista de threads per poder detectar quan esta tot reservat */
      extract_from_queue (&communicator->threads, (char *)others);
    }
    others = (struct t_thread *) next_queue(&communicator->threads);
    if ( (others == NULL) && (i < (Total_threads_involved - 2)) )
    {
      /* En aquest cas es deu haver extret el primer element de la llista,
       * pero encara hauria de quedar algun thread. Per tant, agafo el
       * primer que queda. */
      others = (struct t_thread *) head_queue(&communicator->threads);
    }
  }
  /* Un thread de cada maquina reserva els recursos */
  for(
    others  =(struct t_thread *)head_queue(&communicator->machines_threads);
    others != TH_NIL;
    others  = (struct t_thread *)next_queue(&communicator->machines_threads)
  )
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


static int
get_GLOP_MODEL (char *model)
{
  if (strcmp(model, "0")==0)
      return (GOP_MODEL_0);
  if (strcmp(model, "CTE")==0)
      return (GOP_MODEL_CTE);
  if (strcmp(model, "LIN")==0)
      return (GOP_MODEL_LIN);
  if (strcmp(model, "LOG")==0)
      return (GOP_MODEL_LOG);
  panic ("Invalid Communication model for Global Operation %s\n", model);
  return (-1);
}

static int
get_GLOP_SIZE (char *si)
{
  if (strcmp(si, "MIN")==0)
      return (GOP_SIZE_MIN);
  if (strcmp(si, "MAX")==0)
      return (GOP_SIZE_MAX);
  if (strcmp(si, "MEAN")==0)
      return (GOP_SIZE_MEAN);
  if (strcmp(si, "2MAX")==0)
      return (GOP_SIZE_2MAX);
  if (strcmp(si, "S+R")==0)
      return (GOP_SIZE_SIR);
  panic ("Invalid Communication size for Global Operation %s\n", si);
  return (-1);
}


static char BIGPOINTER[BUFSIZE];


static void read_communication_config_file(FILE *fi, char *filename)
{
  int i, machine_id, global_OP;
  char mini_buffer[BUFSIZE];
  char FIN_model[256];
  char FIN_size[256];
  char FOUT_model[256];
  char FOUT_size[256];
  double contention;  /* Per motius historics. Actualment no s'utilitza. */


  while (!(feof(fi)))
  {
    i = fscanf (fi, "%[^\n]\n", BIGPOINTER);
    if (i == -1) return;

    /* SCHEDULING */
    i = sscanf (BIGPOINTER, "Machine policy: %d %s", &machine_id, mini_buffer);
    if (i==2)
    {
      /* S'ha pogut llegir la politica de scheduling per una maquina */
      machine_id++; /* Tenim els identificadors guardats a de 1 a N */
      COMMUNIC_get_policy (mini_buffer, machine_id, fi, filename);
    }



    /* MACHINE FLIGHT TIME */
    i = sscanf (BIGPOINTER, "Flight time: %d %[^\n]",
                &machine_id, mini_buffer);
    if (i==2)
    {
      if ((machine_id<0) || (machine_id>=sim_char.number_machines))
        panic ("Invalid machine id %d in file %s\n", machine_id, filename);
      machine_id++; /* Tenim els identificadors guardats a de 1 a N */
      /* Cal llegir els flight time d'aquesta maquina */
      get_machine_flight_times( machine_id, mini_buffer, filename);
    }


    /* MACHINE GLOBAL OPERATION INFO */
    i = sscanf (BIGPOINTER, "Machine globalop: %d %d %s %s %s %s",
                &machine_id, &global_OP, FIN_model, FIN_size,
                FOUT_model, FOUT_size);
    if (i==6)
    {
      if ((machine_id<0) || (machine_id>=sim_char.number_machines))
        panic ("Invalid machine id %d in file %s\n", machine_id, filename);
      machine_id++; /* Tenim els identificadors guardats a de 1 a N */
      /* Cal llegir els parametres de l'operació col.lectiva
       * global_OP de la maquina machine_id. */
      get_global_OP_parameters (filename, machine_id, global_OP,
                                FIN_model, FIN_size, FOUT_model,
                                FOUT_size);
    }


    /* EXTERNAL NETWORK GLOBAL OPERATION INFO */
    i = sscanf (BIGPOINTER, "External globalop: %d %s %s %s %s",
                &global_OP, FIN_model, FIN_size,
                FOUT_model, FOUT_size);
    if (i==5)
    {
      /* Cal llegir els parametres de l'operació col.lectiva
       * global_OP per la xarxa externa. */
      get_global_OP_parameters (filename, 0, global_OP,
                                FIN_model, FIN_size, FOUT_model,
                                FOUT_size);
    }


    /* OLD FORMAT!!!! SCHEDULING */
    i = sscanf (BIGPOINTER, "Policy: %s", mini_buffer);
    if (i==1)
    {
      /* S'ha pogut llegir la politica de scheduling cal assignar-la
         a totes les maquines. */
      COMMUNIC_get_policy (mini_buffer, 0, fi, filename);
    }



    /* OLD FORMAT!!!! GLOBAL OPERATION INFO */
    i = sscanf (BIGPOINTER, "%d %s %s %s %s %le",
                &global_OP, FIN_model, FIN_size,
                FOUT_model, FOUT_size, &contention);
    if (i==6)
    {
      /* Cal llegir els parametres de l'operació col.lectiva
       * global_OP per totes les maquines i per la xarxa externa! */
      for(i=0;i<=sim_char.number_machines;i++)
      {
        /* El cas i=0 serveix per canviar els parametres de la
           xarxa externa. Els i=1..N son les diferents maquines. */
        get_global_OP_parameters (filename, i, global_OP,
                                  FIN_model, FIN_size, FOUT_model,
                                  FOUT_size);
      }
      /* El camp contention actualment s'ignora. */
    }
  }
}




/************************************************************************
 ** Aquesta funcio llegeix del string donat tots els flight time de la
 ** maquina indicada.
 ************************************************************************/
static void get_machine_flight_times( int machine_id, char *buffer,
                                      char *filename)
{
  int num_maquines, i, final=0;
  double flight_value;
  char mini_buffer[BUFSIZE];

  num_maquines=0;
  i = sscanf (buffer, "%le %[^\n]", &flight_value, mini_buffer);
  if (i!=2)
  {
    i=sscanf (buffer, "%le", &flight_value);
    final=(i!=1);
  }
  while ((!final) && (num_maquines<sim_char.number_machines))
  {
    flight_value*=1e6;
    sim_char.general_net.flight_times[machine_id-1][num_maquines]=flight_value;
/*    fprintf(stderr,"Flight_time[%d][%d]=%f us\n", machine_id-1, num_maquines,
            flight_value);*/
    num_maquines++;
    if (i==2)
    { /* Encara haurien de quedar maquines per llegir */
      buffer=&buffer[strlen(buffer)-strlen(mini_buffer)];
      i = sscanf (buffer, "%le %[^\n]", &flight_value, mini_buffer);
      if (i!=2)
      {
        i=sscanf (buffer, "%le", &flight_value);
        final=(i!=1);
      }
    }
    else final=1;
  }

  if (num_maquines!=sim_char.number_machines) /* No n'hi havia prous */
    panic ("Invalid number of flight times for machine %d in file %s\n",
           machine_id-1, filename);

  if (!final) /* En sobraven */
    fprintf(stderr,"WARNING: Too many flight times for machine %d in file %s\n",
           machine_id-1, filename);
}




/************************************************************************
 ** Posa a la maquina corresponent o a la xarxa externa, els parametres
 ** donats a l'operació col.lectiva indicada.
 ************************************************************************/
static void  get_global_OP_parameters (char *filename,
                                       int machine_id, int global_OP,
                                       char *FIN_model, char *FIN_size,
                                       char *FOUT_model, char *FOUT_size)
{
  struct t_global_op_definition *glop;
  struct t_machine *machine;
  struct t_global_op_information *glop_info;
  int FIN_model_value;
  int FIN_size_value;
  int FOUT_model_value;
  int FOUT_size_value;
  struct t_queue *cua;


  glop = (struct t_global_op_definition *) query_prio_queue(&Global_op,
         (t_priority)global_OP);
  if (glop==(struct t_global_op_definition *)0)
    panic("Global operation %d undefined and reading file %s\n",
          global_OP, filename);


  FIN_model_value = get_GLOP_MODEL(FIN_model);
  FIN_size_value = get_GLOP_SIZE(FIN_size);
  FOUT_model_value = get_GLOP_MODEL(FOUT_model);
  FOUT_size_value = get_GLOP_SIZE(FOUT_size);


  /* Es determina de quina cua cal modificar les dades */
  if (machine_id==0)
  {
    /* Es tracta de la xarxa externa */
    cua=&sim_char.general_net.global_ops_info;
  }
  else
  {
    /* És una de les màquines */
    for(machine=(struct t_machine *)head_queue(&Machine_queue);
        machine!=MA_NIL;
        machine=(struct t_machine *)next_queue(&Machine_queue))
    {
      if (machine->id == machine_id) break;
    }
    if (machine==MA_NIL) panic("Machine %d not found!\n",machine_id);

    cua=&machine->communication.global_ops_info;
  }

  /* S'obte l'estructura amb la informació d'aquesta operació global */
  glop_info=(struct t_global_op_information *)
            query_prio_queue(cua, (t_priority)global_OP);
  if (glop_info==(struct t_global_op_information *)0)
    panic("Global operation %d undefined and reading file %s\n",
          global_OP, filename);

  /* Es modifiquen les dades*/
  glop_info->FIN_model = FIN_model_value;
  glop_info->FIN_size = FIN_size_value;
  glop_info->FOUT_model = FOUT_model_value;
  glop_info->FOUT_size = FOUT_size_value;

/*
if (machine_id==0)
  fprintf(stderr,"EXTERNAL NET GLOP %d = INM %d, INS %d, OUTM %d, OUTS %d\n",
          global_OP, FIN_model_value, FIN_size_value, FOUT_model_value,
          FOUT_size_value);
else
  fprintf(stderr,"MACHINE %d GLOP %d = INM %d, INS %d, OUTM %d, OUTS %d\n",
          machine_id, global_OP, FIN_model_value, FIN_size_value,
          FOUT_model_value, FOUT_size_value);
*/
}
