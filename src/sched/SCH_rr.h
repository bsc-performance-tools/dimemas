#ifndef __SCH_rr_h
#define __SCH_rr_h
static char SCH_rr_h_rcsid[]="$Id: SCH_rr.h,v 1.1 2004/11/18 15:05:39 paraver Exp $";
static char *__a_SCH_rr_h=SCH_rr_h_rcsid;
/*
 * Round Robin scheduler structures
 * 
 * Sergi Girona (sergi@ac.upc.es)
 * 
 * (c) DAC-UPC 1993-94
 * 
 */


struct t_SCH_rr
{
   t_micro         last_quantum;
};

/**
 * External routines defined in file SCH_rr.c
 **/
extern void RR_thread_to_ready(struct t_thread *thread);
extern t_micro RR_get_execution_time(struct t_thread *thread);
extern struct t_thread *RR_next_thread_to_run(struct t_node *node);
extern void RR_init_scheduler_parameters(struct t_thread *thread);
extern void RR_clear_parameters(struct t_thread *thread);
extern int RR_info(int info);
extern void RR_init(char *filename, struct t_machine *machine);
extern void RR_copy_parameters(struct t_thread *th_o, struct t_thread *th_d);
extern void RR_free_parameters (struct t_thread *thread);

#endif
