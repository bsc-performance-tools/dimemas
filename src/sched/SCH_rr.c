char SCH_rr_c_rcsid[]="$Id: SCH_rr.c,v 1.2 2005/12/23 10:44:14 jgonzale Exp $";
/*
 * Round Robin scheduling policy routines
 * 
 * Sergi Girona (sergi@ac.upc.es)
 * 
 * (c) DAC-UPC 1993-94
 * 
 */

/* Includes 'por defecto' */
#include "define.h"
#include "types.h"

/* Include propio */
#include "SCH_rr.h"

/* Dependencias con otros fuentes */
#include "cpu.h"
#include "extern.h"
#include "list.h"
#include "mallocame.h"
#include "subr.h"

void
RR_thread_to_ready(struct t_thread *thread)
{
   struct t_node  *node;
   struct t_machine *machine;

   node = get_node_of_thread (thread);
   machine = node->machine;
   
   if (thread->action == AC_NIL)
      panic ("Queuing thread  P%d T%d t%d without actions", IDENTIFIERS (thread));
   if ((thread->loose_cpu) || (machine->scheduler.lost_cpu_on_send))
      inFIFO_queue (&(node->ready), (char *) thread);
   else
      inLIFO_queue (&(node->ready), (char *) thread);
}

t_micro
RR_get_execution_time(struct t_thread *thread)
{
   struct t_action *action;
   t_micro         ex_time;
   struct t_SCH_rr *sch_rr;
   struct t_node    *node;
   struct t_machine *machine;
   
   node = get_node_of_thread (thread);
   machine = node->machine;
   sch_rr = (struct t_SCH_rr *) thread->sch_parameters;
   action = thread->action;
   if (action->action != WORK)
   {
      panic ("Trying to work when innaproppiate P%d T%d t%d action(%d)\n",
	     IDENTIFIERS (thread), action->action);
   }
   if ((thread->loose_cpu) || (sch_rr->last_quantum == machine->scheduler.quantum))
   {
      ex_time = MIN (action->desc.compute.cpu_time, machine->scheduler.quantum);
      sch_rr->last_quantum = ex_time;
   }
   else
   {
      ex_time = MIN (machine->scheduler.quantum - sch_rr->last_quantum,
		     action->desc.compute.cpu_time);
      sch_rr->last_quantum += ex_time;
   }
   action->desc.compute.cpu_time -= ex_time;
   if (action->desc.compute.cpu_time == 0)
   {
      thread->action = action->next;
      freeame ((char *) action, sizeof (struct t_action));
   }

   thread->loose_cpu = TRUE;
   return (ex_time);
}

struct t_thread *
RR_next_thread_to_run(struct t_node *node)
{
   return ((struct t_thread *) outFIFO_queue (&(node->ready)));
}

void
RR_init_scheduler_parameters(struct t_thread *thread)
{
   struct t_SCH_rr *sch_rr;

   sch_rr = (struct t_SCH_rr *) mallocame (sizeof (struct t_SCH_rr));
   sch_rr->last_quantum = (t_micro) 0;
   thread->sch_parameters = (char *) sch_rr;
}

void
RR_clear_parameters(struct t_thread *thread)
{
   struct t_SCH_rr *sch_rr;

   sch_rr = (struct t_SCH_rr *) thread->sch_parameters;
   sch_rr->last_quantum = (t_micro) 0;
}

int
RR_info (int info)
{
  return (0);
}

void
RR_init(char *filename, struct t_machine  *machine)
{
  FILE *fp;
  int j;
  char buf[256];
  char str[256];
  double fl;


  if (debug)
  {
    PRINT_TIMER (current_time);
    printf (": RR scheduler init with file %s\n", filename);
  }
  
  if (filename==(char *)0)
    return;
  
  if (strcmp(filename,"")==0)
    return;
  
  fp = MYFOPEN (filename,"r");
  if (fp==(FILE *)0)
  {
    if (filename!=(char *)0)
      fprintf(stderr,"Can open Round Robin configuration file %s\n",filename);
    return;
  }

  fgets (str, 256, fp);
  j = sscanf (str, "Policy: %s", buf);
  if (j!=1)
    panic ("Invalid format in file %s.\nInvalid policy name %s\n",
	   filename, buf);
  
  fgets (str, 256, fp);
  j = sscanf (str, "Context switch cost (seconds): %le",&fl);
  if ((j!=1) || (fl<0))
    panic ("Invalid format in file %s.\nInvalid context switch cost %f\n",
	   filename, fl);
  machine->scheduler.context_switch = fl*1e6;
  
  fgets (str, 256, fp);
  j = sscanf (str, "Busy wait (seconds):  %le", &fl);
  if ((j!=1) || (fl<0))
    panic ("Invalid format in file %s.\nInvalid busy wait %f\n",
	   filename, fl);
  machine->scheduler.busywait_before_block = fl*1e6;

  fgets (str, 256, fp);
  j = sscanf (str, "Quantum (seconds):  %le", &fl);
  if ((j!=1) || (fl<0))
    panic ("Invalid format in file %s.\nInvalid quantum %f\n",
	   filename, fl);
  machine->scheduler.quantum = fl*1e6;
  
  fclose(fp);
}

void
RR_copy_parameters(struct t_thread *th_o, struct t_thread *th_d)
{
   struct t_SCH_rr *sch_rr1;
   struct t_SCH_rr *sch_rr2;

   sch_rr1 = (struct t_SCH_rr *) th_o->sch_parameters;
   sch_rr2 = (struct t_SCH_rr *) th_d->sch_parameters;
   sch_rr2->last_quantum = sch_rr1->last_quantum;
}

void
RR_free_parameters(struct t_thread *thread)
{
   struct t_SCH_rr *sch_rr;

   sch_rr = (struct t_SCH_rr *) thread->sch_parameters;
   freeame ((char *) sch_rr, sizeof (struct t_SCH_rr));
}
