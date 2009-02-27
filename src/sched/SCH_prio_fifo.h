#ifndef __SCH_prio_fifo_h
#define __SCH_prio_fifo_h
static char SCH_prio_fifo_h_rcsid[]="$Id: SCH_prio_fifo.h,v 1.2 2005/05/25 10:56:47 paraver Exp $";
static char *__a_SCH_prio_fifo_h=SCH_prio_fifo_h_rcsid;
/*
 * Fifo with priorities scheduler structures
 * 
 * Sergi Girona (sergi@ac.upc.es)
 * 
 * (c) DAC-UPC 1993-94
 * 
 */
#ifdef BASE_PRIORITY
#undef BASE_PRIORITY
#endif
#define BASE_PRIORITY (t_priority)0
struct t_SCH_prio_fifo
{
   t_priority      priority;
};

/**
 * External routines defined in file SCH_prio_fifo.c
 **/
extern void PRIO_FIFO_thread_to_ready(struct t_thread *thread);
extern t_micro PRIO_FIFO_get_execution_time(struct t_thread *thread);
extern struct t_thread *PRIO_FIFO_next_thread_to_run(struct t_node *node);
extern void PRIO_FIFO_init_scheduler_parameters(struct t_thread *thread);
extern void PRIO_FIFO_clear_parameters(struct t_thread *thread);
extern int PRIO_FIFO_info(int info);
extern void PRIO_FIFO_init(char *filename, struct t_machine *machine);
extern void PRIO_FIFO_copy_parameters(struct t_thread *th_o, 
				      struct t_thread *th_d);
extern void PRIO_FIFO_free_parameters(struct t_thread *thread);
#endif
