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

#ifndef __dim_omp_h
#define __dim_omp_h

#include <stdbool.h>
#include "types.h"
#include "define.h"

#ifdef __cplusplus
extern "C" {
#endif

struct t_omp_worker_info;
struct t_omp_queue;

struct t_omp_queue *create_omp_queue();

void set_omp_master_time( struct t_omp_queue *q, int omp_it, dimemas_timer master_time );
void set_omp_worker_info_iteration(struct t_omp_queue *queue, int omp_it, int iteration); 
void set_omp_worker_info_init_master_time(struct t_omp_worker_info * info, dimemas_timer master_time); 
void set_omp_worker_info_duration(struct t_omp_queue *q, int omp_it, int thread_id, dimemas_timer duration);
void set_omp_worker_printed(struct t_omp_queue *q, int omp_it, int thread_id);

dimemas_timer get_omp_master_time(struct t_omp_queue *q, int omp_it);
dimemas_timer get_omp_worker_duration( struct t_omp_queue *q, int omp_it, int thread_id );

bool is_omp_worker_printed(struct t_omp_queue *q, int omp_it, int thread_id);
bool is_omp_worker_info_ready( struct t_omp_queue *q, int omp_it, int thread_id );

/**************************
 * For OMP syncronization*
 **************************/
/*struct t_omp_worker_synchro_info;
struct t_omp_queue_synchro;

struct t_omp_queue_synchro *create_omp_queue_synchro();

void set_omp_synchro_end_time( struct t_omp_queue_synchro *q, int omp_it, dimemas_timer syncro_end_time );
void set_omp_worker_synchro_printed(struct t_omp_queue_synchro *q, int omp_it, int thread_id);
void set_omp_worker_synchro_iteration(struct t_omp_queue_synchro *queue, int omp_it, int iteration); 

dimemas_timer get_omp_synchro_end_time( struct t_omp_queue_synchro *q, int omp_it );

bool is_omp_worker_synchro_info_ready( struct t_omp_queue_synchro *q, int omp_it, int thread_id );
bool is_omp_worker_synchro_printed(struct t_omp_queue_synchro *q, int omp_it, int thread_id);
*/
/* After Barrier Info*/
/*
void set_omp_worker_after_barrier_run_printed(struct t_omp_queue_synchro *q, int omp_it, int thread_id);

bool is_omp_worker_after_barrier_run_printed(struct t_omp_queue_synchro *q, int omp_it, int thread_id);
*/

#ifdef __cplusplus
}
#endif

#endif // __dim_omp_h
