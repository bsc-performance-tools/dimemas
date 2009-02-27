#ifndef __cp_h
#define __cp_h
/**
 * External routines defined in file cp.c
 **/
extern void new_cp_node (struct t_thread *thread, int status);
extern void new_cp_relation (struct t_thread *dest, struct t_thread *src);
extern void show_CP_graph(void);
#endif
