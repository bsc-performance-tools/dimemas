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
 

#include <vector>
#include <map>
#include "types.h"
#include "dim_omp.h"
#include "define.h"
#include <iostream>

using std::vector;
using std::map;
using namespace std; 

struct t_omp_worker_info 
{
    t_omp_worker_info()
    {
        identifier = -1;
        init_master_time = -1.0;
    }
    int                         identifier;
    dimemas_timer               init_master_time;
    map< int, dimemas_timer >     worker_duration;
    map< int, bool>               worker_printed;
    map< int, vector< t_event > > init_worker_events;
    map< int,  vector< t_event > > end_worker_events;
};

struct t_omp_queue
{
    map< int, struct t_omp_worker_info > winfo;
    
};

//(omp_worker.init_worker_events[threadid]).push_back(event)
void set_omp_worker_info_identifier(struct t_omp_queue *q, int omp_it, int identifier) 
{
    struct t_omp_worker_info &omp_worker = q->winfo[omp_it];
    omp_worker.identifier = identifier;

}

struct t_omp_queue *create_omp_queue()
{
    return new t_omp_queue();
}

/** 
 *saving time information of the master_thread when the
 * work distribution events ends. 60000001:3
 */
void set_omp_master_time( struct t_omp_queue *q, int omp_it, dimemas_timer master_time )
{
    struct t_omp_worker_info &omp_worker = q->winfo[omp_it];
    omp_worker.init_master_time = master_time;

}

/* saving working information */
void set_omp_worker_info_duration(struct t_omp_queue *q, int omp_it, int thread_id, dimemas_timer duration)
{
    struct t_omp_worker_info &omp_worker = q->winfo[omp_it];
    omp_worker.worker_duration[thread_id] = duration;
}

void set_omp_worker_printed(struct t_omp_queue *q, int omp_it, int thread_id)
{
    struct t_omp_worker_info &omp_worker = q->winfo[omp_it];
    omp_worker.worker_printed[thread_id] = true;
}

bool is_omp_worker_printed(struct t_omp_queue *q, int omp_it, int thread_id)
{
    struct t_omp_worker_info &omp_worker = q->winfo[omp_it];
    
    if( omp_worker.worker_printed.find( thread_id ) == omp_worker.worker_printed.end() )
        return false;

    return true;
}

bool is_omp_worker_info_ready( struct t_omp_queue *q, int omp_it, int thread_id )
{
    map< int, struct t_omp_worker_info >::iterator itr = q->winfo.find( omp_it );

    if( itr == q->winfo.end() )
        return false;
 
    struct t_omp_worker_info &omp_worker = itr->second;

    if( omp_worker.init_master_time == -1.0 )
        return false;

    if( omp_worker.worker_duration.find( thread_id ) == omp_worker.worker_duration.end() )
        return false;

    return true;
}

dimemas_timer get_omp_master_time( struct t_omp_queue *q, int omp_it )
{
    return q->winfo[omp_it].init_master_time;
}

dimemas_timer get_omp_worker_duration( struct t_omp_queue *q, int omp_it, int thread_id )
{
    return q->winfo[omp_it].worker_duration[thread_id];
}