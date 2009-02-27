char SCH_fifo_c_rcsid[]="$Id: SCH_fifo.c,v 1.3 2005/12/23 10:44:14 jgonzale Exp $";
/*
 * FIFO scheduling policy routines
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
#include "SCH_fifo.h"

/* Dependencias con otros fuentes */
#include "cpu.h"
#include "extern.h"
#include "list.h"
#include "mallocame.h"
#include "subr.h"

void
FIFO_thread_to_ready(struct t_thread *thread)
{
   struct t_node    *node;
   struct t_machine *machine;
   
   node = get_node_of_thread (thread);
   machine = node->machine;
   
   if ((thread->loose_cpu) || (machine->scheduler.lost_cpu_on_send))
      inFIFO_queue (&(node->ready), (char *) thread);
   else
      inLIFO_queue (&(node->ready), (char *) thread);
}

t_micro
FIFO_get_execution_time(struct t_thread *thread)
{
  struct t_action *action;
  t_micro         ex_time;

  action = thread->action;
  if (action->action != WORK)
  {
    printf(
      "Must be work for P%d T%d t%d but it was %d\n",
      IDENTIFIERS (thread),
      action->action
    );
  }
  ex_time = action->desc.compute.cpu_time;
  thread->action = action->next;
  freeame ((char *) action, sizeof (struct t_action));
  return (ex_time);
}

struct t_thread *
FIFO_next_thread_to_run(struct t_node *node)
{
   return ((struct t_thread *) outFIFO_queue (&(node->ready)));
}

void
FIFO_init_scheduler_parameters(struct t_thread *thread)
{
   thread->sch_parameters = A_NIL;
}

void
FIFO_clear_parameters(struct t_thread *thread)
{
}

int
FIFO_info(int info)
{
  return (0);
}

void
FIFO_init(char *filename, struct t_machine  *machine)
{
  FILE *fp;
  int j;
  char buf[256];
  char str[256];
  double fl;


  if (debug==D_SCH)
  {
    PRINT_TIMER (current_time);
    printf (": FIFO scheduler init with file %s\n", filename);
  }
  
  if (filename==(char *)0)
    return;
  
  if (strcmp(filename,"")==0)
    return;
  
  fp = MYFOPEN (filename,"r");
  if (fp==(FILE *)0)
  {
    if (filename!=(char *)0)
      fprintf(stderr,"Can open FIFO configuration file %s\n",filename);
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
  
  fclose(fp);
}

void
FIFO_copy_parameters(struct t_thread *th_o, struct t_thread *th_d)
{
}

void
FIFO_free_parameters (struct t_thread *thread)
{
}
