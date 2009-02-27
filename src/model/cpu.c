char cpu_c_rcsid[]="$Id: cpu.c,v 1.8 2005/05/24 15:41:40 paraver Exp $";
/*
 * Processor (resource) routines
 * 
 * Sergi Girona (sergi@ac.upc.es)
 * 
 * (c) DAC-UPC 1993-94
 * 
 */

#include "define.h"
#include "types.h"

#include "cpu.h"
#include "extern.h"
#include "list.h"
#include "mallocame.h"
#include "subr.h"
#include "task.h"



void
Initialize_empty_machine(struct t_queue *machines)
{
  int               i;
  struct t_machine *machine;
  struct t_link    *link;
  
  /* FEC: El numero de maquines s'obte de les dades del simulador,
         que ja haurien de ser les correctes.*/

  create_queue (machines);
  
  for (i = 0; i < sim_char.number_machines; i++)
  {
    machine = (struct t_machine *) mallocame (sizeof (struct t_machine));
    machine->id = i + 1;
  
    /* Assign default values */
    machine->number_nodes = MAX_NODES;
    machine->instrumented_arch = (char *) 0;
    
    /* Informacio del scheduler */
    machine->scheduler.policy = 0;
    machine->scheduler.quantum = 3000;
    machine->scheduler.priority_preemptive = FALSE;
    machine->scheduler.lost_cpu_on_send = FALSE;
    machine->scheduler.context_switch = (t_micro) NO_CONTEXT_SWITCH;
    machine->scheduler.busywait_before_block = FALSE;
    ASS_TIMER (machine->scheduler.minimum_quantum, 0);
  
    /* Parametres de la xarxa interna */
    machine->communication.remote_bandwith = (t_micro) REMOTE_BANDWITH;
    machine->communication.port_startup = (t_micro) PORT_STARTUP;
    machine->communication.memory_startup = (t_micro) MEMORY_STARTUP;
    machine->communication.num_messages_on_network = 0;
    machine->communication.global_operation = sim_char.general_net.global_operation;
    create_queue (&(machine->communication.global_ops_info));
    
    /* Dades variables de l'estat de la xarxa interna */
    create_queue (&(machine->network.threads_on_network));
    create_queue (&(machine->network.queue));
    
    /* Dades variables de l'estat de la xarxa externa */
    create_queue (&(machine->external_net.free_in_links));
    create_queue (&(machine->external_net.free_out_links));
    create_queue (&(machine->external_net.busy_in_links));
    create_queue (&(machine->external_net.busy_out_links));
    create_queue (&(machine->external_net.th_for_in));
    create_queue (&(machine->external_net.th_for_out));
    machine->external_net.half_duplex_links = FALSE;
    /* Creem un link d'entrada */
    link = (struct t_link *) mallocame (sizeof (struct t_link));
    link->linkid = 1;
    link->info.machine = machine;
    link->kind = MACHINE_LINK;
    link->type = IN_LINK;
    link->thread = TH_NIL;
    ASS_ALL_TIMER (link->assigned_on, current_time);
    inFIFO_queue (&(machine->external_net.free_in_links), (char *) link);
    /* I un link de sortida */
    link = (struct t_link *) mallocame (sizeof (struct t_link));
    link->linkid = 2;
    link->info.machine = machine;
    link->kind = MACHINE_LINK;
    link->type = OUT_LINK;
    link->thread = TH_NIL;
    ASS_ALL_TIMER (link->assigned_on, current_time);
    inFIFO_queue (&(machine->external_net.free_out_links), (char *) link);
  
    /* Dades de les connexions dedicades */
    create_queue (&(machine->dedicated_connections.connections));
  
    /* S'afegeix la maquina a la llista */
    insert_queue (machines, (char *) machine, (t_priority) (machine->id));
  }
  
  /* Es crea i s'inicialitza la matriu dels flight time de la xarxa externa */
  sim_char.general_net.flight_times = 
    (double **) malloc(sizeof(double*) * sim_char.number_machines);
  
  if (sim_char.general_net.flight_times == NULL)
  {
    panic("Not enough memory");
  }
  
  for (i = 0; i < sim_char.number_machines; i++)
  {
    sim_char.general_net.flight_times[i] = 
      (double*) malloc(sizeof(double) * sim_char.number_machines);
    if (sim_char.general_net.flight_times[i] == NULL)
    {
      panic("Not enough memory");
    }
    bzero(
      sim_char.general_net.flight_times[i],
      sizeof(double)* sim_char.number_machines
    );
  }
}




void Initialize_empty_node(struct t_queue *nodes, struct t_machine *machine)
{
   int               i;
   struct t_node    *node;

   for (i = 0; i < machine->number_nodes; i++)
   {
      node = (struct t_node *) mallocame (sizeof (struct t_node));
      node->nodeid = last_node_id_used + 1;
      last_node_id_used++;
      create_queue (&(node->Cpus));
      create_queue (&(node->ready));
      create_queue (&(node->free_out_links));
      create_queue (&(node->free_in_links));
      create_queue (&(node->busy_out_links));
      create_queue (&(node->busy_in_links));
      create_queue (&(node->th_for_in));
      create_queue (&(node->th_for_out));
      create_queue (&(node->wait_outlink_port));
      create_queue (&(node->wait_inlink_port));
      create_queue (&(node->wait_in_copy_segment));
      create_queue (&(node->wait_out_copy_segment));
      create_queue (&(node->IO_disks));
      create_queue (&(node->IO_disks_threads));

      /* FEC: Ens passen la maquina que correspon als nodes */
      node->machine=machine;

      insert_queue (nodes, (char *) node, (t_priority) (node->nodeid));
   }
}



int fill_node(int no_number, char * node_name, int no_processors, int no_input,
	  int no_output, double local_startup, double remote_startup, 
	  double relative, double local_bandwith, double external_net_startup,
          double local_port_startup, double remote_port_startup,
          double local_memory_startup, double remote_memory_startup)
{
   register struct t_node *node;
   int             j;
   struct t_cpu   *cpu;
   struct t_link  *link;

   node = (struct t_node *) query_prio_queue (&Node_queue, (t_priority) no_number);
   if (node == N_NIL)
   {
      fill_parse_error ("Invalid node identifier %d\n", no_number);
      return (1);
   }
   if (count_queue (&(node->Cpus)) != 0)
   {
      fprintf (stderr, "Warning: redefinition of node %d\n", no_number - 1);
   }
   for (j = 0; j < no_processors; j++)
   {
      cpu = (struct t_cpu *) mallocame (sizeof (struct t_cpu));
      cpu->cpuid = j + 1;
      cpu->current_thread = TH_NIL;
      cpu->current_thread_context = TH_NIL;
      cpu->current_load = (double) 0;
      cpu->io = QU_NIL;
      insert_queue (&(node->Cpus), (char *) cpu, (t_priority) (j + 1));
   }
   if ((no_input==0) || (no_output==0))
   {
     node->half_duplex_links = TRUE;
     j = MAX(no_input, no_output);
     no_input = j;
     no_output = j;
   }
   else
     node->half_duplex_links = FALSE;
   for (j = 0; j < no_input; j++)
   {
      link = (struct t_link *) mallocame (sizeof (struct t_link));
      link->linkid = j + 1;
      link->info.node = node;
      link->kind = NODE_LINK;
      link->type = IN_LINK;
      link->thread = TH_NIL;
      ASS_ALL_TIMER (link->assigned_on, current_time);
      inFIFO_queue (&(node->free_in_links), (char *) link);
   }
   for (j = 0; j < no_output; j++)
   {
      link = (struct t_link *) mallocame (sizeof (struct t_link));
      link->linkid = j + 1;
      link->info.node = node;
      link->kind = NODE_LINK;
      link->type = OUT_LINK;
      link->thread = TH_NIL;
      ASS_ALL_TIMER (link->assigned_on, current_time);
      inFIFO_queue (&(node->free_out_links), (char *) link);
   }
   node->local_startup = local_startup;
   node->remote_startup = remote_startup;
   node->relative = relative;
   node->bandwith = local_bandwith;
   node->arch = node_name;
   node->external_net_startup = external_net_startup;
   node->local_port_startup = local_port_startup;
   node->remote_port_startup = remote_port_startup;
   node->local_memory_startup = local_memory_startup;
   node->remote_memory_startup = remote_memory_startup;
   return (0);
}



void Initialize_empty_connection(struct t_queue *connections)
{
   int               i;
   struct t_dedicated_connection    *d_con;
   struct t_link *link;

   /* FEC: El numero de connexions s'obte de les dades del simulador,
           que ja haurien de ser les correctes.*/

   create_queue (connections);
   for (i = 0; i < sim_char.dedicated_connections.number_connections; i++)
   {
      d_con = (struct t_dedicated_connection *) mallocame (sizeof (struct t_dedicated_connection));
      d_con->id = i + 1;

      /* Assign default values */
      d_con->source_id = 0;
      d_con->destination_id = 0;
      d_con->bandwidth = 0;
      d_con->tags=NULL;
      d_con->number_of_tags=0;
      d_con->first_message_size=0;
      d_con->first_size_condition='\0';
      d_con->operation=0;
      d_con->second_message_size=0;
      d_con->second_size_condition='\0';
      d_con->communicators=NULL;
      d_con->number_of_communicators=0;
      d_con->startup=0;
      d_con->flight_time=0;
      d_con->half_duplex = FALSE;
      
      /* Dades variables de l'estat de la connexio */
      /* Aquestes quatre cues actualment podrien ser punters perque
         nomes tindran 0 o 1 link. */
      create_queue (&(d_con->free_in_links));
      create_queue (&(d_con->free_out_links));
      create_queue (&(d_con->busy_in_links));
      create_queue (&(d_con->busy_out_links));
      /* Aquestes cues son necessaries */
      create_queue (&(d_con->th_for_in));
      create_queue (&(d_con->th_for_out));
      /* Creem un link d'entrada */
      link = (struct t_link *) mallocame (sizeof (struct t_link));
      link->linkid = 1;
      link->info.connection = d_con;
      link->kind = CONNECTION_LINK;
      link->type = IN_LINK;
      link->thread = TH_NIL;
      ASS_ALL_TIMER (link->assigned_on, current_time);
      insert_queue (&(d_con->free_in_links), (char *)link, (t_priority)(link->linkid));
      /* I un link de sortida */
      link = (struct t_link *) mallocame (sizeof (struct t_link));
      link->linkid = 2;
      link->info.connection = d_con;
      link->kind = CONNECTION_LINK;
      link->type = OUT_LINK;
      link->thread = TH_NIL;
      ASS_ALL_TIMER (link->assigned_on, current_time);
      insert_queue (&(d_con->free_out_links), (char *)link, (t_priority)(link->linkid));
      /* S'afegeix la connexio a la llista */
      insert_queue (connections, (char *) d_con, (t_priority) (d_con->id));
   }
}







void
Give_number_to_CPU ()
{
  struct t_node *no;
  struct t_cpu *cpu;
  int number=1;

  for (no=(struct t_node *)head_queue (&Node_queue);
       no!=N_NIL;
       no=(struct t_node *)next_queue (&Node_queue))
  {
    for (cpu=(struct t_cpu *)head_queue (&(no->Cpus));
         cpu!=(struct t_cpu *)0;
         cpu=(struct t_cpu *)next_queue (&(no->Cpus)))
    {
       cpu->unique_number = number;
       number++;
    }
  }
}

struct t_node *
get_node_of_thread(struct t_thread *thread)
{
   register struct t_account *account;
   register struct t_node *node;

   account = current_account (thread);
   if (account == ACC_NIL)
      panic ("Thread without account P%d T%d t%d\n", IDENTIFIERS (thread));
   node = (struct t_node *) query_prio_queue (&Node_queue, 
             (t_priority) account->nodeid);
   if (node==(struct t_node *)0)
      panic ("Unable to locate node %d for P%d T%d t%d\n",
              account->nodeid, IDENTIFIERS (thread));
   return (node);
}

struct t_node *
get_node_of_task(struct t_task *task)
{
   struct t_thread *thread;

   thread = (struct t_thread *) head_queue (&(task->threads));
   return (get_node_of_thread (thread));
}

struct t_node *
get_node_by_id(int nodeid)
{
   return ((struct t_node *) query_prio_queue (&Node_queue,
					       (t_priority) nodeid));
}

void
check_full_nodes()
{
   struct t_node  *node;

   for (node = (struct t_node *) head_queue (&Node_queue);
	node != N_NIL;
	node = (struct t_node *) next_queue (&Node_queue))
   {
      if (count_queue (&(node->Cpus)) == 0)
      {
	 panic ("Node %d non initialized\n", node->nodeid - 1);
      }
   }
}

int
num_free_cpu(struct t_node *node)
{
   register struct t_cpu *cpu;
   int             i = 0;


   for (cpu = (struct t_cpu *) head_queue (&(node->Cpus));
	cpu != C_NIL;
	cpu = (struct t_cpu *) next_queue (&(node->Cpus)))
   {
      if (cpu->current_thread == TH_NIL)
	 i++;
   }
   return (i);
}

t_boolean
is_thread_running (struct t_thread *thread)
{
   register struct t_cpu *cpu;
   register struct t_node *node;

   node = get_node_of_thread (thread);
   for (cpu = (struct t_cpu *) head_queue (&(node->Cpus));
	cpu != C_NIL;
	cpu = (struct t_cpu *) next_queue (&(node->Cpus)))
   {
      if (cpu->current_thread == thread)
	 return (TRUE);
   }
   return (FALSE);
}
