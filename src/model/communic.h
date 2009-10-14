#ifndef __communic_h
#define __communic_h

/**
 * External routines defined in file communic.c
 **/
 
extern void 
COMMUNIC_general(int value, struct t_thread *thread);

extern int  
COMMUNIC_get_policy(char *s, int machine_id, FILE *fi, char *filename);

extern void 
COMMUNIC_init(char *fichero_comm);

extern void 
COMMUNIC_end(void);

extern void 
COMMUNIC_send(struct t_thread *thread);

extern void 
COMMUNIC_recv(struct t_thread *thread);

extern void 
COMMUNIC_block_after_busy_wait(struct t_thread *thread);

/*extern t_micro 
transferencia(
  int size, 
  t_boolean remote, 
  struct t_thread *thread,
  struct t_dedicated_connection *connection,
  t_micro *temps_recursos
);
*/

void
transferencia(
  int size, 
  t_boolean remote, 
  struct t_thread *thread,
  struct t_dedicated_connection *connection,
  t_micro *temps_total,
  t_micro *temps_recursos
);

extern void 
really_send (struct t_thread *thread);

extern void 
new_global_op (int identificator, char *name);

extern void 
  GLOBAL_operation (
  struct t_thread *thread, 
  int glop_id, 
  int comm_id, 
  int root_rank, 
  int root_thid,
  int bytes_send, 
  int bytes_recv
);

extern struct t_communicator* 
locate_communicator(struct t_queue *communicator_queue, int commid);

extern void 
global_op_reserva_links (struct t_thread *thread);

/* JGG: Constantes para marcar el FAN_IN y el FAN_OUT */
#define FAN_IN  0
#define FAN_OUT 1 


#endif
