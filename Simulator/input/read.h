

#ifndef __read_h
#define __read_h
/**
 * External routines defined in file read.c
 **/

void reload_new_Ptask(struct t_Ptask *Ptask);

void show_statistics(int pallas_output);

void READ_get_next_action(struct t_thread* thread);

void calculate_execution_end_time(void);

#endif
