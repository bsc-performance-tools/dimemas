

#ifndef __read_h
#define __read_h
/**
 * External routines defined in file read.c
 **/

void reload_new_Ptask(struct t_Ptask *Ptask);

void show_statistics();

void calculate_execution_end_time(void);

void READ_get_next_action(struct t_thread* thread);

void READ_create_action(struct t_action **action);

void READ_copy_action(struct t_action *src_action,
                      struct t_action *dst_action);

void READ_free_action(struct t_action    *action);

long READ_get_living_actions(void);

#endif
