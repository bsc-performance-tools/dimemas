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

#ifndef __read_h
#define __read_h
/**
 * External routines defined in file read.c
 **/

extern struct t_action* get_next_action_from_file_binary(FILE *file,
                                                         int *tid, int *th_id,
                                                         struct t_node  *node);

extern struct t_action*
get_next_action_from_file_sddf(struct t_thread *thread,
                               int *tid, int *th_id,
                               struct t_node  *node,
                               struct t_Ptask *Ptask);

extern void reload_new_Ptask(struct t_Ptask *Ptask);

extern void init_simul_char(void);

extern void show_statistics(int pallas_output);

extern void load_configuration_file(char *filename);

extern void Read_Ptasks(void);

extern void Ptasks_init(void);

extern void get_next_action(struct t_thread* thread);

extern t_boolean is_it_new_format(void);

extern void get_next_action_wrapper(struct t_thread *thread);

void calculate_execution_end_time();

/* 
EXTERN void     read_configuration_file (T (char *) L (char *));
EXTERN void     read_simul_char (L (char *));
EXTERN void     show_results (L (void));
EXTERN int      yyparse (L (void));
*/

#endif
