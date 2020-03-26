#ifndef __graph_h
#define __graph_h

#include <types.h>
#include <define.h>
#include <extern.h>

#define STRG_DEP 1
#define SOFT_DEP 2
#define GLOP_DEP 4

#define NO_ACTION 0
#define NO_GLOP 0
#define NO_COMM NULL
#define SOFT_ACTION_RESOLVED 65536

#define GRAPH_TO_ALL -1
#define GRAPH_ALL_ACTIONS -1
#define GRAPH_ALL_TASKS -1

#define TAG_ALL -1
#define COMMUNIC_ID_ALL -1

#define DEBUG 1


struct dependency_queue
{
  struct dependency * first_dep;
  struct dependency * last_dep;
  unsigned int num_deps;
};

struct dependency
{
  struct dependency * previous;
  struct dependency * next;

  int from;
  int to;
  unsigned int action;

  int type;

  int p2p_tag;
  int p2p_communic_id;

  int global_op;
  struct t_communicator * communicator;

  unsigned int file_offset;
  dimemas_timer time;   // Instant when the dependency has been added
};

struct dependency_queue * _graph_map;
int _nodes;


void GRAPH_init(int _nodes);
void GRAPH_reset();
void GRAPH_fini();
void GRAPH_remove_dependency_p(struct dependency * d);
void GRAPH_print_state(int i);
void GRAPH_change_action(struct dependency *d, int new_action);
void GRAPH_debug(int from_taskid, int to_taskid, char * msg);
void GRAPH_debug_msg(char * msg);

t_boolean GRAPH_look_for_cycle(int root, int * chain, int * chain_size);

struct dependency *  GRAPH_add_dependency(int from_taskid, int to_taskid, int type, struct t_thread * thread);
struct dependency *  GRAPH_get_dependency(int from_taskid, int to_taskid, int type, int action, int mess_tag, int communic_id);
struct dependency ** GRAPH_get_dependencies(int from_taskid, int to_taskid, int type, int action, int mess_tag, int communic_id, int * dep_set_size);

//int GRAPH_get_independent(int from_task);
//int GRAPH_is_dependent(int from_taskid, int to_taskid, int type);
//int GRAPH_is_dependent_action(int from_taskid, int to_taskid, int type);
//struct dependency * GRAPH_get_dependency(int from_taskid, int to_taskid, int type, int action);
//struct dependency ** GRAPH_get_dependencies(int from_taskid, int to_taskid, int type, int action, int * res_size);

t_boolean __look_for_cycle(int from, int root, int * dep_chain_queue, int * deepness);



#endif
