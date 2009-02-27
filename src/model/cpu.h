#ifndef __cpu_h
#define __cpu_h
/**
 * External routines defined in file cpu.c
 **/
extern void Initialize_empty_machine(struct t_queue *machines);
extern void Initialize_empty_node(struct t_queue *nodes, struct t_machine *machine);
extern void Initialize_empty_connection(struct t_queue *connections);
extern int fill_node(int no_number, char * node_name, int no_processors,
		     int no_input, int no_output, double local_startup,
		     double remote_startup, double relative, 
		     double local_bandwith, double external_net_startup,
                     double local_port_startup, double remote_port_startup,
                     double local_memory_startup, double remote_memory_startup);
extern struct t_node *get_node_of_thread(struct t_thread *thread);
extern struct t_node *get_node_of_task(struct t_task *task);
extern struct t_node *get_node_by_id(int nodeid);
extern void check_full_nodes(void);
extern int num_free_cpu(struct t_node *node);
extern t_boolean is_thread_running (struct t_thread *thread);

#endif
