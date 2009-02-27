#ifndef __read_h
#define __read_h
/**
 * External routines defined in file read.c
 **/

extern struct t_action*
get_next_action_from_file_binary(FILE *file,
                                 int *tid, int *th_id, 
                                 struct t_node  *node);

extern struct t_action*
get_next_action_from_file_sddf(FILE *file,
                               int *tid, int *th_id,
                               struct t_node  *node,
                               struct t_Ptask *Ptask);

extern void
reload_new_Ptask(struct t_Ptask *Ptask);

extern void
init_simul_char(void);

extern void
show_statistics(int pallas_output);

extern void
load_configuration_file(char *filename);

extern void
Read_Ptasks(void);

extern void
Ptasks_init(void);

extern void
get_next_action(struct t_thread* thread);

extern t_boolean
is_it_new_format(void);

extern void 
get_next_action_wrapper(struct t_thread *thread);

void
calculate_execution_end_time();

/* 
EXTERN void     read_configuration_file (T (char *) L (char *));
EXTERN void     read_simul_char (L (char *));
EXTERN void     show_results (L (void));
EXTERN int      yyparse (L (void));
*/

#endif
