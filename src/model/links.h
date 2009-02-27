#ifndef __links_h
#define __links_h
/**
 * External routines defined in file links.c
 **/

extern void link_busy(struct t_thread *thread, struct t_node *node, 
		      int in_out);
extern t_boolean get_links(struct t_thread *thread, struct t_node *node,
			   struct t_node *node_partner);
extern t_boolean get_links_port(struct t_thread *thread_s, 
				struct t_node *node_s, 
				struct t_thread *thread_r, 
				struct t_node *node_r);
extern t_boolean get_links_memory_copy(struct t_thread *thread, 
				       struct t_node *node_s,
				       struct t_node *node_d);
extern void free_link(struct t_link *link,
    struct t_thread *thread);


EXTERN          t_boolean
get_links_port (T (struct t_thread *) T (struct t_node *)
		T (struct t_thread *) L (struct t_node *));
EXTERN          t_boolean
get_links_memory (T (struct t_thread *) T (struct t_node *)
		  L (struct t_node *));
EXTERN          t_boolean
get_links_memory_copy (T (struct t_thread *) T (struct t_node *)
		       L (struct t_node *));
EXTERN void     free_link (L (struct t_link *));



extern void machine_link_busy(struct t_thread *thread,
                              struct t_machine *machine, int in_out);
extern t_boolean get_machine_links(struct t_thread *thread,
                                   struct t_machine *s_machine,
	                                 struct t_machine *d_machine);
extern t_boolean get_one_machine_link(struct t_thread *thread,
                                      struct t_machine *machine,
                                      int in_out);


extern void connection_link_busy(struct t_thread *thread,
                                 struct t_dedicated_connection *connection,
                                 int in_out);
extern t_boolean get_connection_links(struct t_thread *thread,
                                      struct t_dedicated_connection *connection);
extern void free_connection_link(struct t_link *link, struct t_thread *thread);

#endif
