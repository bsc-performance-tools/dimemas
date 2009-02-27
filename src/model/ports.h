#ifndef __ports_h
#define __ports_h
/**
 * External routines defined in file ports.c
 **/
extern void PORT_general (int value, struct t_thread *thread);
extern void PORT_init(void);
extern void PORT_end(void);
extern t_boolean PORT_create(int portid, struct t_thread *thread);
extern t_boolean PORT_delete(int portid);
extern t_boolean PORT_send (int module, int portid, struct t_thread *thread,
			    int siz);
extern t_boolean PORT_receive(int module, int portid, struct t_thread *thread,
			      int siz);
extern void really_port_send(struct t_port *port, struct t_thread *thread_s,
			     struct t_thread *thread_r);

#endif
