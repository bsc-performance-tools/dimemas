#include <deadlock_analysis.h>
#include <assert.h>
#include <string.h> 
#include <graph.h>
#include <simulator.h>
#include <file_data_access.h>
#include <list.h>
#include <communic.h>

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


int _get_earlier_task_index(int * dep_chain_queue, int deepness, t_boolean ignore_glops);

//int still_analyzing();
//struct t_action * action_to_inject(struct t_thread * partner);


/*
 * Initialization of the deadlock analyzer
 */
void DEADLOCK_init_deadlock_analysis(int ranks, const char * parameter_tracefile, float end_analysis_tpercent)
{
  char * to = strstr(parameter_tracefile, ".dim");
  int name_size = (to-parameter_tracefile);
  char * extension = ".estats\0";

  extra_stats_filename = (char *)malloc(sizeof(char)*name_size+8); // ".estats"
  memcpy(extra_stats_filename, parameter_tracefile, (to-parameter_tracefile));
  memcpy(extra_stats_filename+name_size, extension, 8);

  total_ranks = ranks;
  are_estats_parsed = FALSE;
  _MAX_RECVS = _MAX_SENDS = _MAX_GLOPS = 0;

  _end_analysis_tpercent = end_analysis_tpercent;

  GRAPH_init(total_ranks);

  printf("   * Deadlock analysis: Deactivation at %.2f\n", end_analysis_tpercent*100);
}

/**
 * Manager of new communications
 * It should be called when a new communication has been performed
 */
t_boolean DEADLOCK_new_communic_event(struct t_thread * thread)
{
  t_boolean res = FALSE;
  int action = thread->action->action;
  int from_taskid = thread->task->taskid;

  #if DEBUG
  //GRAPH_print_state(from_taskid);
  #endif // DEBUG

  // In principle if a task has a strong dependency with other, it can not advance
  // in the simulation, but if it does, it means that we have a lack of coordination
  // between Dimemas and the analyzer.
  int str_deps_size = 0;
  struct dependency ** str_deps =
    GRAPH_get_dependencies(from_taskid, GRAPH_TO_ALL,
		  STRG_DEP, GRAPH_ALL_ACTIONS, TAG_ALL, COMMUNIC_ID_ALL, &str_deps_size);

  if (str_deps_size > 0)
  {
#if DEBUG
    printf("**************************************************************\n"\
           "* WARNING: Lack of coordination between Dimemas and analyzer *\n"\
           "**************************************************************\n"\
           "Rank %d wants to %s => ", from_taskid, get_name_of_action(action));
#endif

    char msg[100];
    sprintf(msg, "*Rank %d wants to %s\n", from_taskid, get_name_of_action(action));
    GRAPH_debug(from_taskid, -1, msg);
    GRAPH_print_state(from_taskid);
    printf("\n");

    GRAPH_fini();
    //assert(FALSE);
  }

  ///////////////////////////////////////////////////////////////////////////////
  // TODO: Mirar si vamos por el 25-50% de la traza y que además no tenga nada //
  // pendiente. En ese caso with_deadlock_analysis = 0 !!!!                    //
  ///////////////////////////////////////////////////////////////////////////////

  int Ptaskid = thread->task->Ptask->Ptaskid;
  int taskid = thread->task->taskid;
  int threadid = thread->threadid;

  float prog = DAP_get_progression(Ptaskid, taskid, threadid);

  if (prog >= _end_analysis_tpercent)
  {
      //res = DEADLOCK_check_end();
      //if (res == FALSE)
      //{
	    printf("**** Deadlock analyzer has been deactivated. Progression %.2f\n", prog*100);
	    with_deadlock_analysis = 0;
	    return FALSE;
      //}
      return res;
  }


  // A new state of the graph is computed in function of the last state and
  // the new communication event.
  switch(action)
  {
    case SEND:
    {
      int to_taskid = thread->action->desc.send.dest;
      int mess_tag = thread->action->desc.send.mess_tag;
      int communic_id = thread->action->desc.send.communic_id;

      // For now, Dimemas only manage rendez vous or not rendez vous communications
      // independently whether is immediate or not as we can see at communic.c::3937
      t_boolean is_isend = !thread->action->desc.send.rendez_vous;

#if DEBUG
      char msg[100];
      if (is_isend)
      {
        sprintf(msg, "New ISEND (%d, %d)", mess_tag, communic_id);
      }
      else
      {
        sprintf(msg, "New SEND (%d, %d)", mess_tag, communic_id);
      }
      GRAPH_debug(from_taskid, to_taskid, msg);
#endif // DEBUG

      struct dependency * dep  = GRAPH_get_dependency(to_taskid, from_taskid,
                                                      STRG_DEP, RECV|WAIT, mess_tag, communic_id);

      int dep_set_size;
      struct dependency ** deps = GRAPH_get_dependencies(to_taskid, from_taskid,
		      SOFT_DEP, IRECV, mess_tag, communic_id, &dep_set_size);

      if (dep != NULL && dep->action == RECV) // Send Recv matching
      {
        GRAPH_remove_dependency_p(dep);
      }
      else if(dep != NULL && dep->action == WAIT) // Send Wait matching
      {
        if (dep_set_size > 0)
        {
	      assert(deps[0]->action == IRECV);

          GRAPH_remove_dependency_p(dep); // WAIT
          GRAPH_remove_dependency_p(deps[0]); // IRECV
        }
        else // the partner performs a wait before irecv -> something wrong
        {
          GRAPH_add_dependency(from_taskid, to_taskid, STRG_DEP, thread);
          res = _is_deadlocked(from_taskid);

          assert(res == TRUE);
        }
      }
      else if(dep_set_size > 0) // Send Irecv matching
      {
        // All WAITS are waiting recvs but no one for sends
        GRAPH_change_action(deps[0], SOFT_ACTION_RESOLVED);
      }
      else // It is a new dependecy arc
      {
        if (!is_isend)
        {
          GRAPH_add_dependency(from_taskid, to_taskid, STRG_DEP, thread);
          res = _is_deadlocked(from_taskid);
        }
        else
        {
          // In Dimemas, when it is an isend, it does not waits for the recv
          // then, the notification is performed asynchronously. Therefore,
          // when a Wait is performed, the communication has to be resolved.
          // If it is done in this way we have to take into account the RECV/IRECV
          // counterpart. It has to be resolved with this SOFT_ACTION_RESOLVED
          // WORKARROUND
          struct dependency * new_d = GRAPH_add_dependency(from_taskid, to_taskid, SOFT_DEP, thread);
          GRAPH_change_action(new_d, SOFT_ACTION_RESOLVED);

          // Theorically the good way
          //GRAPH_add_dependency(from_taskid, to_taskid, SOFT_DEP, thread);
        }
      }
      free(deps);
      break;
    }
    case RECV:
    {
      int to_taskid = thread->action->desc.recv.ori;
      int mess_tag =  thread->action->desc.recv.mess_tag;
      int communic_id = thread->action->desc.recv.communic_id;

      #if DEBUG
      char msg[100];
      sprintf(msg, "New RECV (%d, %d)", mess_tag, communic_id);
      GRAPH_debug(from_taskid, to_taskid, msg);
      #endif // DEBUG

      struct dependency * dep  = GRAPH_get_dependency(to_taskid, from_taskid,
		      STRG_DEP, SEND, mess_tag, communic_id);

      if (dep != NULL)
        GRAPH_remove_dependency_p(dep);
      else
      {
        // Now, the Isend directly is a soft_action_Resolved. WORKARROUND
        struct dependency * dep_soft = GRAPH_get_dependency(to_taskid, from_taskid,
              SOFT_DEP, SOFT_ACTION_RESOLVED, mess_tag, communic_id);

        if (dep_soft != NULL) // ISEND
          GRAPH_change_action(dep_soft, SOFT_ACTION_RESOLVED);
        else
        {
          GRAPH_add_dependency(from_taskid, to_taskid, STRG_DEP, thread);
          res = _is_deadlocked(from_taskid);
        }
      }
      break;
    }
    case WAIT:
    {
      int to_taskid = thread->action->desc.recv.ori;
      int mess_tag =  thread->action->desc.recv.mess_tag;
      int communic_id = thread->action->desc.recv.communic_id;

      #if DEBUG
      char msg[100];
      sprintf(msg, "New WAIT (%d, %d)", mess_tag, communic_id);
      GRAPH_debug(from_taskid, to_taskid, msg);
      #endif // DEBUG

      struct dependency * deps = GRAPH_get_dependency(from_taskid, to_taskid, SOFT_DEP,
		      SOFT_ACTION_RESOLVED, mess_tag, communic_id);

      // When an Isend is performed, it is directly an soft_action_resolved. WORKARROUND
      struct dependency * deps_aux = GRAPH_get_dependency(to_taskid, from_taskid, SOFT_DEP,
		      SOFT_ACTION_RESOLVED, mess_tag, communic_id);

      if (deps != NULL)
      {
        GRAPH_remove_dependency_p(deps);
      }
      else if(deps_aux != NULL) // WORKARROUND
      {
        GRAPH_remove_dependency_p(deps_aux);
      }
      else
      {
        GRAPH_add_dependency(from_taskid, to_taskid, STRG_DEP, thread);
        res = _is_deadlocked(from_taskid);
      }
      break;
    }
    case GLOBAL_OP:
    {
      #if DEBUG
      GRAPH_debug(from_taskid, -1, "New GLOP");
      #endif // DEBUG

      t_boolean blocks = DEADLOCK_manage_global_dependency(thread);
      if (blocks)
        res = _is_deadlocked(from_taskid);

      break;
    }
    case IRECV:
    {
      int to_taskid = thread->action->desc.recv.ori;
      int mess_tag =  thread->action->desc.recv.mess_tag;
      int communic_id = thread->action->desc.recv.communic_id;

      #if DEBUG
      char msg[100];
      sprintf(msg, "New IRECV (%d, %d)", mess_tag, communic_id);
      GRAPH_debug(from_taskid, to_taskid, msg);
      #endif // DEBUG

      struct dependency * dep  = GRAPH_get_dependency(to_taskid, from_taskid, STRG_DEP, SEND, mess_tag, communic_id);
      // Now, the Isend directly is a soft_action_Resolved. WORKARROUND
      struct dependency * dep_soft = GRAPH_get_dependency(to_taskid, from_taskid, SOFT_DEP, SOFT_ACTION_RESOLVED, mess_tag, communic_id);

      if (dep != NULL) // Irecv Send matching
      {
        struct dependency * new_d = GRAPH_add_dependency(from_taskid, to_taskid, SOFT_DEP, thread);
        GRAPH_change_action(new_d, SOFT_ACTION_RESOLVED);
        GRAPH_remove_dependency_p(dep);
      }
      else if (dep_soft != NULL) // Irecv Isend matching
      {
        struct dependency * new_d = GRAPH_add_dependency(from_taskid, to_taskid, SOFT_DEP, thread);
        GRAPH_change_action(new_d, SOFT_ACTION_RESOLVED);
        GRAPH_remove_dependency_p(dep_soft);
      }
      else
        GRAPH_add_dependency(from_taskid, to_taskid, SOFT_DEP, thread);

      break;
    }
    default:
    {
      assert(FALSE);
    }
  }

  return res;
}

/**
 * A new state of the graph is computed for the special case of
 * global operations.
 */
t_boolean DEADLOCK_manage_global_dependency(struct t_thread * thread)
{
  t_boolean blocks = FALSE;

  struct t_communicator * communicator;
  int from_collective = thread->task->taskid;
  struct t_action * from_action = thread->action;
  int comm_id = from_action->desc.global_op.comm_id;

  communicator = locate_communicator(&thread->task->Ptask->Communicator, comm_id);
  int * comm_threads = communicator->global_ranks;
  int size_comm = communicator->size;

  for ( int j = 0; j < size_comm; ++j)
  {
    int to_collective = comm_threads[j];
    if (from_collective == to_collective)
        continue;

    struct dependency * dep = GRAPH_get_dependency(
            to_collective, 
            from_collective,
            GLOP_DEP, 
            from_action->action, TAG_ALL, COMMUNIC_ID_ALL);

    if (dep != NULL && dep->global_op == from_action->desc.global_op.glop_id)
    {
      GRAPH_remove_dependency_p(dep);
    }
    else
    {
      GRAPH_add_dependency(from_collective, to_collective, GLOP_DEP, thread);
      blocks = TRUE;
    }
  }
  return blocks;
}

/**
 * This method should be called when the simulation of a thread
 * is finalized. It is possible that another thread are in a waiting
 * state but w/o having a deadlock. Then, the thread finnishing is
 * considered a dependency with the rest of ranks and force the deadlock
 */
void DEADLOCK_thread_finalized(struct t_thread * thread)
{
  int to_taskid = thread->task->taskid;
  int dep_set_size = 0;
  struct dependency ** dep_tome;

  for (int i = 0; i < total_ranks; ++i)
  {
    if (i == to_taskid) 
        continue;

    dep_tome = GRAPH_get_dependencies(i, to_taskid, STRG_DEP|GLOP_DEP,
		    GRAPH_ALL_ACTIONS, TAG_ALL, COMMUNIC_ID_ALL, &dep_set_size);

    for (int j=0; j < dep_set_size; ++j)
    {
        struct t_thread * thread_to_reverse = get_thread_by_task_id(i);
        struct trace_operation * op_to_be_ignored = 
            (struct trace_operation *)malloc(sizeof(struct trace_operation));

        op_to_be_ignored->Ptask_id = thread_to_reverse->task->Ptask->Ptaskid;
        op_to_be_ignored->task_id = thread_to_reverse->task->taskid;
        op_to_be_ignored->thread_id = thread_to_reverse->threadid;
        op_to_be_ignored->file_offset = dep_tome[j]->file_offset;

        double prio = (double)op_to_be_ignored->file_offset;
        insert_queue(&thread_to_reverse->ops_to_be_ignored, (char *)op_to_be_ignored,prio);

        simulation_rebooted = TRUE;
    }
  }
}

t_boolean _is_deadlocked(int from)
{
  int * dep_chain_queue = malloc(sizeof(int)*total_ranks+1);
  int deepness = 0;

  int res = GRAPH_look_for_cycle(from, dep_chain_queue, &deepness);

        printf("\nthe deepness is ::::::  %d and res is :: %d\n", deepness, res);
  if (res)
  {
      printf("-> [%f] Deadlock detected\n", current_time);
#if DEBUG
    char msg[512];
    sprintf(msg,"*******************************\n" \
          "-> [%f ns] Deadlock has been detected\n" \
          "-> Dependency chain (%d) [ ", current_time, deepness);
    GRAPH_debug_msg(msg);

    int i;
    for (i=0; i < deepness; ++i)
    {
      sprintf(msg, "%d", dep_chain_queue[i]);
      GRAPH_debug_msg(msg);
      if (i < deepness-1)
      {
        struct dependency * d;
        d = GRAPH_get_dependency(dep_chain_queue[i],dep_chain_queue[i+1],
                STRG_DEP|GLOP_DEP, GRAPH_ALL_ACTIONS, TAG_ALL, COMMUNIC_ID_ALL);

        sprintf(msg,"-(%s)->", get_name_of_action(d->action));
        GRAPH_debug_msg(msg);
      }
    }
    GRAPH_debug_msg("]\n");
    //printf("-> Looking for file \"%s\" ...", extra_stats_filename);
#endif

    struct stat file_stat;
    int task_to_mod_pos;

    int ret = stat(extra_stats_filename, &file_stat);
    if (ret != 0)
    {
#if DEBUG
      //printf("FAIL\n  -> Breaking dependency chain randomly.\n");
      //printf("  --> Should consider to execute prv2dim with '-e' flag.\n");
#endif
      task_to_mod_pos = _get_earlier_task_index(dep_chain_queue, deepness, FALSE);
    }
    else
    {
#if DEBUG
        GRAPH_debug_msg("SUCCESS\n");
        //printf("SUCCESS\n");
#endif
      task_to_mod_pos = breakChainFrom(dep_chain_queue, deepness);
    }

    // The deadlock only could be breaked if we ignore one of the dependencies
    // that defines the relations between the tasks of the 'dep_chain_queue'.
    struct dependency * _dep = GRAPH_get_dependency(dep_chain_queue[task_to_mod_pos],
                                                           dep_chain_queue[task_to_mod_pos+1],
                                                           STRG_DEP|GLOP_DEP,
                                                           GRAPH_ALL_ACTIONS,
                                                           TAG_ALL,
                                                           COMMUNIC_ID_ALL);

    // This thread is the thread that performs the operation that becomes the dependency
    // that we want to ignore, therefore, the info has to be taken from it
    struct t_thread * thread_to_mod = get_thread_by_task_id(dep_chain_queue[task_to_mod_pos]);
    struct trace_operation * op_to_be_ignored = (struct trace_operation *)malloc(sizeof(struct trace_operation));
    
    op_to_be_ignored->Ptask_id = thread_to_mod->task->Ptask->Ptaskid;
    op_to_be_ignored->task_id = thread_to_mod->task->taskid;
    op_to_be_ignored->thread_id = thread_to_mod->threadid;
    op_to_be_ignored->file_offset = _dep->file_offset;

    double prio = (double)op_to_be_ignored->file_offset;

    if (query_prio_queue(&thread_to_mod->ops_to_be_ignored, prio) != A_NIL)
    {
      printf("ERROR: Has been an incoherence between Dimemas"
             " models and deadlock analyzer. Try to simulate without --clean-deadlocks flag.\n\n");
      //assert(FALSE);
    }
#if DEBUG
    sprintf(msg,"-> Action of task %d will be ignored. (file offset: %u)\n",
           op_to_be_ignored->task_id, op_to_be_ignored->file_offset);
    GRAPH_debug_msg(msg);
#endif
    insert_queue(&thread_to_mod->ops_to_be_ignored, (char *)op_to_be_ignored, prio);
    simulation_rebooted = TRUE;
  }
  free(dep_chain_queue);
  return res;
}

/////////////////////    AUXILIAR FUNCTIONS    /////////////////////////////////


/** \brief Creates a communicator that involves all thes tasks on parameter
 *         \p tasks_involved. The new communicator is automatically inserted in the
 *         communicators list of the first task of the \p tasks_involved.
 *
 * \param tasks_involved int* Is an array with all tasks that wants to be involved in the
 *        new communicator.
 * \param size int Is the size of the array tasks_involved.
 * \return int Returns the communicator_id of the new communicator.
 *
 */
int create_communicator(int * tasks_involved, int size)
{
  // Assume all tasks in same Ptask
  struct t_Ptask * my_ptask = get_thread_by_task_id(tasks_involved[0])->task->Ptask;

  struct t_communicator * new_c = malloc(sizeof(struct t_communicator));
  new_c->communicator_id = my_ptask->Communicator.count+1; // Starts to enumerate with 1?
  new_c->size = size-1;
  new_c->global_ranks = (int *)malloc(sizeof(int)*size);
  new_c->current_root = NULL;

  create_queue (&new_c->threads);
  create_queue (&new_c->machines_threads);
  create_queue (&new_c->m_threads_with_links);

  new_c->nodes_per_machine =
    (struct t_queue*) malloc(Simulator.number_machines*sizeof(struct t_queue));

  int i;
  for (i = 0; i < Simulator.number_machines; i++)
  {
    create_queue(&new_c->nodes_per_machine[i]);
  }

  create_queue(&new_c->tasks_per_node);

  memcpy(new_c->global_ranks, (void *)tasks_involved, size*sizeof(int));
  insert_queue(&my_ptask->Communicator,(char*) new_c,(t_priority)new_c->communicator_id);

  return new_c->communicator_id;
}


/** 
 * brief Returns the first thread of task that has the id equals to the parameter
 *         \p taskid
 *  \param taskid int Identification number of the task,
 *  \return struct t_thread* Pointer to the thread of task with \p taskid
 */
struct t_thread * get_thread_by_task_id(int taskid)
{
  struct t_Ptask *Ptask;
  for (   Ptask  = (struct t_Ptask *) head_queue (&Ptask_queue);
          Ptask != P_NIL;
          Ptask  = (struct t_Ptask *) next_queue (&Ptask_queue))
  {
    for (int tasks_it = 0; tasks_it < Ptask->tasks_count; tasks_it++)
    {
      struct t_task * task = &(Ptask->tasks[tasks_it]);
      //assert(task->threads_count == 1);

      if (task->taskid == taskid)
      {
        return task->threads[0];
      }
    }
  }
  return NULL;
}

int breakChainFrom(int * dep_chain_queue, int size)
{
  if (!are_estats_parsed)
  {
    estats_per_rank = malloc(sizeof(struct t_estats)*total_ranks);
    parse_estats(estats_per_rank);
    are_estats_parsed = TRUE;
  }

  unsigned int * pendent_pointer = NULL;
  int max_pendent = 0;
  int position_chosed_thread = 0;
  int sum_pendent_glops = 0;

  int i;
  for (i = 0; i < size-1; i++)
  {
    int from_dep = dep_chain_queue[i];
    int to_dep = dep_chain_queue[i+1];

    struct dependency * d = GRAPH_get_dependency(from_dep, to_dep, STRG_DEP|GLOP_DEP,
            GRAPH_ALL_ACTIONS, TAG_ALL, COMMUNIC_ID_ALL);
    assert(d);
    switch (d->action)
    {
    case SEND:
    case ISEND:
#if DEBUG
      printf("  <%d> (I)SEND : %d -> ", from_dep, estats_per_rank[from_dep].pendent_i_Send);
#endif
      if (estats_per_rank[from_dep].pendent_i_Send > max_pendent)
      {
        max_pendent = estats_per_rank[from_dep].pendent_i_Send;
        position_chosed_thread = i;
        pendent_pointer = &estats_per_rank[from_dep].pendent_i_Send;
#if DEBUG
        printf("SET\n");
#endif
      }
#if DEBUG
      else
      {
        printf("NO SET\n");
      }
#endif
      break;
    case IRECV:
    case RECV:
    case WAIT:
#if DEBUG
      printf("  <%d> (I)RECV : %d ->", from_dep, estats_per_rank[from_dep].pendent_i_Recv);
#endif
      if (estats_per_rank[from_dep].pendent_i_Recv > max_pendent)
      {
        max_pendent = estats_per_rank[from_dep].pendent_i_Recv;
        position_chosed_thread = i;
        pendent_pointer = &estats_per_rank[from_dep].pendent_i_Recv;
#if DEBUG
        printf("SET\n");
#endif
      }
#if DEBUG
      else
      {
        printf("NO SET\n");
      }
#endif
      break;
    case GLOBAL_OP:
#if DEBUG
      printf("  <%d> GLOP : %d/%d ->", from_dep, estats_per_rank[from_dep].pendent_glop, max_pendent);
#endif
      sum_pendent_glops+=estats_per_rank[from_dep].pendent_glop;

      if (estats_per_rank[from_dep].pendent_glop > max_pendent)
      {
        max_pendent = estats_per_rank[from_dep].pendent_glop;
        position_chosed_thread = i;
        pendent_pointer = &estats_per_rank[from_dep].pendent_glop;
#if DEBUG
        printf("SET\n");
#endif
      }
#if DEBUG
      else
      {
        printf("NO SET\n");
      }
#endif
      break;
    default: assert(FALSE);
    }
  }

  if(pendent_pointer == NULL)
  {
    printf("WARNING: Lack of information in estats file.\n");
    return _get_earlier_task_index(dep_chain_queue, size, sum_pendent_glops == 0);
  }
  (*pendent_pointer)--;
  return position_chosed_thread;
}

void parse_estats(struct t_estats * epr)
{
  char * line = NULL;
  size_t len = 0;
  int cnt = 0;

  FILE * estats_fd = fopen(extra_stats_filename, "r");
  assert(estats_fd != NULL);

  while (getline(&line, &len, estats_fd) != -1)
  {
    if (strstr(line, "//") != NULL)
    {
      continue; // it is a comment
    }

    char * pch;
    int pos = 0;
    pch = strtok (line,",");

    while (pch != NULL)
    {
      switch (pos++)
      {
      case 0: epr[cnt].send_counter = atoi(pch); break;
      case 1:
        epr[cnt].isend_counter = atoi(pch);
        if (epr[cnt].send_counter + epr[cnt].isend_counter > _MAX_SENDS)
          _MAX_SENDS = epr[cnt].send_counter + epr[cnt].isend_counter;
        break;
      case 2: epr[cnt].recv_counter = atoi(pch); break;
      case 3: epr[cnt].irecv_counter = atoi(pch); break;
      case 4:
        epr[cnt].wait_counter = atoi(pch);
        if (epr[cnt].recv_counter + epr[cnt].irecv_counter > _MAX_RECVS)
          _MAX_RECVS = epr[cnt].recv_counter + epr[cnt].irecv_counter;
        break;
      case 5:
        epr[cnt].glop_counter = atoi(pch);
        if (epr[cnt].glop_counter > _MAX_GLOPS)
          _MAX_GLOPS = epr[cnt].glop_counter;
        break;
      case 6: epr[cnt].pendent_i_Send = atoi(pch); break;
      case 7: epr[cnt].pendent_i_Recv = atoi(pch); break;
      case 8: epr[cnt].pendent_glop = atoi(pch); break;
      case 9: epr[cnt].pendent_wait = atoi(pch); break;
        break;
      default: assert(FALSE);
      }
      pch = strtok (NULL, " ,");
    }
    cnt++; // next rank
  }

  fclose(estats_fd);
  if (line)
    free(line);
}

void DEADLOCK_reset()
{
  GRAPH_reset();
}

char * get_name_of_action(int action)
{
  char * res = malloc(sizeof(char)*5);
    res = "";
  switch(action)
  {
    case NOOP:
      res = "NOOP"; break;
    case DEAD:
      res = "DEAD"; break;
    case WORK:
      res = "WORK"; break;
    case SEND:
      res = "SEND"; break;
    case RECV:
      res = "RECV"; break;
    case EVENT:
      res = "EVENT"; break;
    case PRIO:
      res = "PRIO"; break;
    case FS:
      res = "FS"; break;
    case SEM:
      res = "SEM"; break;
    case PORT_SEND:
      res = "PSEN"; break;
    case PORT_RECV:
      res = "PREC"; break;
    case MEMORY_COPY:
      res = "MCPY"; break;
    case GLOBAL_OP:
      res = "GLOP"; break;
    case MPI_IO:
      res = "MPIO"; break;
    case MPI_OS:
      res = "MPIS"; break;
    case IRECV:
      res = "IREC"; break;
    case WAIT:
      res = "WAIT"; break;
    case WAIT_FOR_SEND:
      res = "WFSE"; break;
    case SOFT_ACTION_RESOLVED:
      res = "SOFT_ACTION_RESOLVED"; break;
    default:
      assert(FALSE);
  }
  return res;
}


t_boolean DEADLOCK_check_end()
{
  // This method is called when the simulation is finalizing, therefore, all
  // pendent dependencies have to be ignored in the next restart.
#if DEBUG
  printf("-> Looking for pendent operations ...\n");
#endif

  int res = FALSE;

  int i, j;
  for(i=0; i<total_ranks; ++i)
  {
    GRAPH_print_state(i);
    int pendent_size;
    struct dependency ** pendent = GRAPH_get_dependencies(i, GRAPH_TO_ALL, STRG_DEP|SOFT_DEP|GLOP_DEP,
                                                          GRAPH_ALL_ACTIONS, TAG_ALL, COMMUNIC_ID_ALL, &pendent_size);

    //if (pendent_size > 0)
     // res = TRUE;
    for (j=0; j < pendent_size; ++j)
    {
      struct dependency * dep = pendent[j];

      struct trace_operation * op_to_be_ignored = (struct trace_operation *)malloc(sizeof(struct trace_operation));
      struct t_thread * thread_to_mod = get_thread_by_task_id(pendent[j]->from);

      op_to_be_ignored->Ptask_id = thread_to_mod->task->Ptask->Ptaskid;
      op_to_be_ignored->task_id = thread_to_mod->task->taskid;
      op_to_be_ignored->thread_id = thread_to_mod->threadid;
      op_to_be_ignored->file_offset = pendent[j]->file_offset;
      
      double prio = (double)op_to_be_ignored->file_offset;
      insert_queue(&thread_to_mod->ops_to_be_ignored, (char *)op_to_be_ignored, prio);
      res = TRUE;
    }
  }
  return res;
}

int _get_earlier_task_index(int * dep_chain_queue, int deepness, t_boolean ignore_glops)
{
  // CHAPUCILLA... ignore_glos implies that a glop operation will not be proposed for ignoring.
  // it is because we can ensure with almost 100% that if the .estat returns 0 pending
  // glops, the deadlock is not because glops.

  int i = 0;
  dimemas_timer earlier;
  int earlier_index = -1;

  for(i=0; i<deepness-1; ++i)
  {
    struct dependency * _dep = GRAPH_get_dependency(dep_chain_queue[i], dep_chain_queue[i+1],
                                                       STRG_DEP|GLOP_DEP,GRAPH_ALL_ACTIONS,
                                                       TAG_ALL,COMMUNIC_ID_ALL);
    if (earlier_index == -1)
    {
      earlier = _dep->time;
      earlier_index = i;
    }
    else if (_dep->time < earlier && (_dep->action != GLOBAL_OP || !ignore_glops))
    {
      earlier = _dep->time;
      earlier_index = i;
    }
  }
  return earlier_index;
}
