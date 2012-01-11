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

#ifndef __memory_h
#define __memory_h
/**
 * External routines defined in file memory.c
 **/
extern void MEMORY_general (int value, struct t_thread *thread);
extern void MEMORY_init(void);
extern void MEMORY_end(void);
extern void MEMORY_copy_segment(int module, struct t_thread *thread, 
				struct t_node *node_s, struct t_node *node_d,
				int si);
extern void really_copy_segment(struct t_thread *thread, 
				struct t_node *node_s, 
				struct t_node *node_d, int si);

extern void RMA_general (int value, struct t_thread *thread);
extern void ONE_SIDED_operation (struct t_thread *thread);


extern void os_completed (struct t_thread *thread);
extern void really_RMA (register struct t_thread *thread);

/* Different window modes */
#define WINDOW_MODE_NONE  0
#define WINDOW_MODE_FENCE 1
#define WINDOW_MODE_LOCK  2 
#define WINDOW_MODE_POST  3

#endif
