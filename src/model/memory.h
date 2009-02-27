#ifndef __memory_h
#define __memory_h
/**
 * External routines defined in file memory.c
 **/
extern void MEMORY_general (int value, struct t_thread *thread);
extern void MEMORY_init(void);
extern void MEMORY_end(void);
extern void MEMORY_copy_segment(int module, struct t_thread *thread, 
				struct t_node *node_s, struct t_node *node_d,
				int si);
extern void really_copy_segment(struct t_thread *thread, 
				struct t_node *node_s, 
				struct t_node *node_d, int si);

extern void RMA_general (int value, struct t_thread *thread);
extern void ONE_SIDED_operation (struct t_thread *thread);

/* Different window modes */
#define WINDOW_MODE_NONE  0
#define WINDOW_MODE_FENCE 1
#define WINDOW_MODE_LOCK  2 
#define WINDOW_MODE_POST  3

#endif
