#ifndef __SCH_FIFO_h
#define __SCH_FIFO_h
static char SCH_fifo_h_rcsid[]="$Id: SCH_fifo.h,v 1.1 2004/11/18 15:05:39 paraver Exp $";
static char *__a_SCH_fifo_h=SCH_fifo_h_rcsid;
/**
 * External routines defined in file SCH_fifo.c
 **/
extern void FIFO_thread_to_ready(struct t_thread *thread);
extern t_micro FIFO_get_execution_time(struct t_thread *thread);
extern struct t_thread *FIFO_next_thread_to_run(struct t_node *node);
extern void FIFO_init_scheduler_parameters(struct t_thread *thread);
extern void FIFO_clear_parameters(struct t_thread *thread);
extern int FIFO_info(int info);
extern void FIFO_init(char *filename, struct t_machine *machine);
extern void FIFO_copy_parameters(struct t_thread *th_o, struct t_thread *th_d);
extern void FIFO_free_parameters (struct t_thread *thread);

#endif
