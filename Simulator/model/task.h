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

/*****************************************************************************
 * Global variables
 *****************************************************************************/

/*
 * Global Applications (Ptask) Queue
 */
extern struct t_queue  Ptask_queue;

/*****************************************************************************
 * Public functions
 *****************************************************************************/

void TASK_init(int sintetic_io_applications);

void TASK_end (void);

void TASK_New_Ptask(char *trace_name,
                    int   tasks_count,
                    int  *tasks_mapping);

void TASK_New_Task(struct t_Ptask *Ptask, int taskid, int nodeid);

/* Synthetic burst generation functions */
void SYNT_BURST_add_new_burst_category(int    burst_category_id,
                                              double burst_category_mean,
                                              double burst_category_std_dev);

double SYNT_BURST_get_burst_value( int burst_category_id);

void new_communicator_definition (struct t_Ptask *Ptask, int communicator_id);

void new_identificator_to_communicator(struct t_Ptask *, int, int);

void clear_account(struct t_account *account);

struct t_account *new_accounter(void);

void new_account(struct t_queue *acc, int nodeid);

void add_account (struct t_account *to, struct t_account *from);

void min_account (struct t_account *to, struct t_account *from);

void max_account (struct t_account *to, struct t_account *from);

struct t_thread *locate_thread(struct t_Ptask *Ptask, int taskid, int thid);

struct t_thread *locate_thread_of_task (struct t_task *task, int thid);

void new_action_to_thread(struct t_Ptask *Ptask,
                          int taskid,
                          int thid,
                          struct t_action *action);

struct t_account *current_account(struct t_thread *thread);

struct t_task *locate_task (struct t_Ptask *Ptask, int taskid);

struct t_thread *duplicate_thread_fs (struct t_thread *thread);

void delete_duplicate_thread_fs (struct t_thread *thread);

struct t_thread *duplicate_thread (struct t_thread *thread);

void delete_duplicate_thread (struct t_thread *thread);

t_boolean more_actions_on_Ptask (struct t_Ptask *Ptask);

void clear_last_actions (struct t_Ptask *Ptask);

void get_operation (struct t_thread *thread, struct t_fs_op *fs_op);


t_nano work_time_for_sintetic(void);

void First_action_to_sintetic_application (struct t_Ptask *Ptask);

void create_sintetic_applications (int num);

t_boolean more_actions_to_sintetic (struct t_thread *thread);

struct t_node *get_node_for_task_by_name (struct t_Ptask *Ptask, int taskid);

void TASK_module_new (long long int type,
                      long long int value,
                      double        ratio);

void file_name (struct t_Ptask *Ptask, int file_id, char *location);

/*
void module_name(struct t_Ptask *Ptask,
                 long long       module_type,
                 long long       module_value,
                 char           *module_name,
                 char           *activity_name,
                 int             src_file,
                 int             src_line);
*/

void module_entrance(struct t_thread *thread,
                     long long        module_type,
                     long long        module_value);

int module_exit(struct t_thread *thread,
                long long        module_type);

void user_event_type_name(struct t_Ptask *Ptask,
                          int   type,
                          char *name,
                          int   color);

void user_event_value_name(struct t_Ptask *Ptask,
                           int   type,
                           int   value,
                           char *name);

void recompute_work_upon_modules(struct t_thread *thread, struct t_action *action);


void add_identificator_to_communicator(struct t_Ptask *Ptask,
                                  int communicator_id,
                                  int taskid);

void no_more_identificator_to_communicator(struct t_Ptask *Ptask,
                                      int communicator_id);

void new_window_definition (struct t_Ptask *Ptask, int window_id);

void add_identificator_to_window(struct t_Ptask *Ptask, int window_id, int taskid);

void no_more_identificator_to_window(struct t_Ptask *Ptask, int window_id);

t_nano PREEMP_overhead(struct t_task* task);


#endif

