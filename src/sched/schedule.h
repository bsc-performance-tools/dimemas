#ifndef __schedule_h
#define __schedule_h
/**
 * External routines defined in file schedule.c
 **/
extern struct t_cpu*
select_free_cpu(struct t_node *node, struct t_thread *thread);
  
extern void
SCHEDULER_thread_to_ready(struct t_thread *thread);
  
extern t_micro
SCHEDULER_get_execution_time(struct t_thread *thread);

extern void
SCHEDULER_next_thread_to_run(struct t_node *node);
  
extern void
SCHEDULER_general(int value, struct t_thread *thread);
  
extern void
SCHEDULER_thread_to_busy_wait(struct t_thread *thread);
  
extern void
SCHEDULER_init(char *filename);

extern void
SCHEDULER_end(void);

extern void
SCHEDULER_reload(struct t_Ptask *Ptask);
  
extern void
SCHEDULER_copy_parameters(struct t_thread *th_o, struct t_thread *th_d);
  
extern void
SCHEDULER_free_parameters(struct t_thread *thread);
  
extern int
SCHEDULER_info(
  int              value,
  int              info,
  struct t_thread *th_s,
  struct t_thread *th_r
);

extern void
SCHEDULER_thread_to_ready_return(
  int              module, 
  struct t_thread *thread, 
  t_micro          ti,
  int              id
);

extern struct t_thread*
SCHEDULER_preemption (struct t_thread *thread, struct t_cpu *cpu);
  
extern int
SCHEDULER_get_policy (char *s);

extern t_boolean
more_actions (struct t_thread *thread);

/* JGG: Intenta replanificar el thread que le pasamos por parámetro */
extern void
SCHEDULER_reschedule (struct t_thread *thread);

#endif
