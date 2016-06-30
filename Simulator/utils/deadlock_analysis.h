#ifndef __deadlock_analysis_h
#define __deadlock_analysis_h

#include <types.h>
#include <define.h>
#include <extern.h>

#define NO_ACTION 0
#define NO_GLOP 0
#define NO_COMM NULL

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
  int * dep_chain;
  int * dep_chain_actions;
  int discarded;
};

struct t_estats * estats_per_rank;

int are_estats_parsed;
int total_ranks;
char * extra_stats_filename;

// Keeps the maximum number of sends and recieves in all ranks. This info is
// gathered through estats file.
unsigned int _MAX_SENDS;
unsigned int _MAX_RECVS;
unsigned int _MAX_GLOPS;

float _end_analysis_tpercent;


void DEADLOCK_init_deadlock_analysis(int ranks, char * parameter_tracefile, float end_analysis_tpercent);
void DEADLOCK_reset();
void DEADLOCK_thread_finalized(struct t_thread * thread);
t_boolean DEADLOCK_new_communic_event(struct t_thread * thread);
t_boolean DEADLOCK_check_end();

#endif
