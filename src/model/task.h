/*****************************************************************************\
 *                        ANALYSIS PERFORMANCE TOOLS                         *
 *                                  Dimemas                                  *
 *       Simulation tool for the parametric analysis of the behaviour of     *
 *       message-passing applications on a configurable parallel platform    *
 *                                                                           * 
 *****************************************************************************
 *     ___     This library is free software; you can redistribute it and/or *
 *    /  __         modify it under the terms of the GNU LGPL as published   *
 *   /  /  _____    by the Free Software Foundation; either version 2.1      *
 *  /  /  /     \   of the License, or (at your option) any later version.   *
 * (  (  ( B S C )                                                           *
 *  \  \  \_____/   This library is distributed in hope that it will be      *
 *   \  \__         useful but WITHOUT ANY WARRANTY; without even the        *
 *    \___          implied warranty of MERCHANTABILITY or FITNESS FOR A     *
 *                  PARTICULAR PURPOSE. See the GNU LGPL for more details.   *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public License  *
 * along with this library; if not, write to the Free Software Foundation,   *
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA          *
 * The GNU LEsser General Public License is contained in the file COPYING.   *
 *                                 ---------                                 *
 *   Barcelona Supercomputing Center - Centro Nacional de Supercomputacion   *
\*****************************************************************************/

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- *\

  $URL::                  $:  File
  $Rev::                  $:  Revision of last commit
  $Author::               $:  Author of last commit
  $Date::                 $:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef __task_h
#define __task_h
/**
 * External routines defined in file task.c
 **/

/* Synthetic burst generation functions */
extern void
SYNT_BURST_add_new_burst_category(
  int    burst_category_id,
  double burst_category_mean,
  double burst_category_std_dev
);

extern double
SYNT_BURST_get_burst_value( int burst_category_id);

extern void
TASK_end (void);

extern void
new_communicator_definition(struct t_Ptask *, int);

extern void
new_identificator_to_communicator(struct t_Ptask *, int, int);

extern struct t_Ptask*
create_Ptask(char *tracefile, char *configfile);

extern void
clear_account(struct t_account *account);

extern struct t_account*
new_accounter(void);

extern void
new_account(struct t_queue *acc, int nodeid);

extern void
add_account (struct t_account *to, struct t_account *from);

extern void
min_account (struct t_account *to, struct t_account *from);

extern void
max_account (struct t_account *to, struct t_account *from);

extern void
new_thread_in_Ptask(
  struct t_Ptask *Ptask,
  int taskid,
  int nodeid,
  int number_of_threads,
  int priority,
  int where
);

extern struct t_thread*
locate_thread(struct t_Ptask *Ptask, int taskid, int thid);

extern void
new_action_to_thread(
  struct t_Ptask *Ptask,
  int taskid, int thid,
  struct t_action *action
);
  
extern struct t_account*
current_account(struct t_thread *thread);

extern struct t_task*
locate_task (struct t_Ptask *Ptask, int taskid);

extern struct t_thread*
duplicate_thread_fs (struct t_thread *thread);

extern void
delete_duplicate_thread_fs (struct t_thread *thread);

extern struct t_thread*
duplicate_thread (struct t_thread *thread);

extern void
delete_duplicate_thread (struct t_thread *thread);

extern t_boolean
more_actions_on_Ptask (struct t_Ptask *Ptask);

extern void
clear_last_actions (struct t_Ptask *Ptask);

extern void
get_operation (struct t_thread *thread, struct t_fs_op *fs_op);

extern void
sddf_seek_next_action_to_thread (struct t_thread *thread);

extern void
sddf_load_initial_work_to_threads (struct t_Ptask *Ptask);

extern t_micro
work_time_for_sintetic(void);

extern void
First_action_to_sintetic_application (struct t_Ptask *Ptask);

extern void
create_sintetic_applications (int num);

extern t_boolean
more_actions_to_sintetic (struct t_thread *thread);

extern struct t_node*
get_node_for_task_by_name (struct t_Ptask *Ptask, int taskid);

extern void
module_new (struct t_Ptask *Ptask, int identificator, double ratio);

extern void
file_name (struct t_Ptask *Ptask, int file_id, char *location);

extern void
module_name(
  struct t_Ptask *Ptask,
  int   block_id,
  char *block_name,
  char *activity_name,
  int   src_file,
  int   src_line
);

extern void
module_entrance(
  struct t_Ptask *Ptask,
  int tid,
  int thid,
  int identificator
);

extern int
module_exit (
  struct t_Ptask *Ptask,
  int tid,
  int thid,
  int identificator
);

void
user_event_type_name(
  struct t_Ptask *Ptask,
  int   type,
  char *name,
  int   color
);

void
user_event_value_name(
  struct t_Ptask *Ptask,
  int   type,
  int   value,
  char *name
);

extern void
recompute_work_upon_modules(struct t_thread *thread, struct t_action *action);
#endif
