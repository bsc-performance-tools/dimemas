#ifndef __deadlock_analysis_h
#define __deadlock_analysis_h

#include <define.h>
#include <extern.h>
#include <types.h>

#define NO_ACTION 0
#define NO_GLOP   0
#define NO_COMM   NULL

struct t_estats
{
  unsigned int send_counter;
  unsigned int isend_counter;
  unsigned int recv_counter;
  unsigned int irecv_counter;
  unsigned int wait_counter;
  unsigned int glop_counter;
  unsigned int pendent_i_Send;
  unsigned int pendent_i_Recv;
  unsigned int pendent_glop;
  unsigned int pendent_wait;
};

struct t_deadlock_descriptor
{
  int* dep_chain;
  int* dep_chain_actions;
  int discarded;
};

void DEADLOCK_init_deadlock_analysis( int ranks, const char* parameter_tracefile, float end_analysis_tpercent );
void DEADLOCK_reset();
void DEADLOCK_thread_finalized( struct t_thread* thread );
t_boolean DEADLOCK_new_communic_event( struct t_thread* thread );
t_boolean DEADLOCK_check_end();

t_boolean DEADLOCK_manage_global_dependency( struct t_thread* thread );
struct t_thread* get_thread_by_task_id( int taskid );
int breakChainFrom( int* dep_chain_queue, int size );
void parse_estats( struct t_estats* epr );
char* get_name_of_action( int action );
t_boolean _is_deadlocked( int from );

#endif
