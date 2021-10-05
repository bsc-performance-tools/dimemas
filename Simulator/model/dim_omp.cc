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
#include <iostream>

#include "types.h"
#include "dim_omp.h"
#include "define.h"
#include "EventEncoding.h"
#include "paraver.h"

using std::vector;
using std::map;
using namespace std; 

struct t_omp_worker_info 
{
    t_omp_worker_info()
    {
        iteration = -1;
        init_master_time = -1.0;
    }

    int                               iteration;
    dimemas_timer                     init_master_time;

    map< int, dimemas_timer >         worker_duration;
    map< int, bool>                   worker_printed;
    map< int, bool>                   worker_event_printed;
    
    map< int, vector< t_omp_event > > events_sequence;
};

struct t_omp_queue
{
    map< int, struct t_omp_worker_info > winfo;
};

struct t_omp_queue *create_omp_queue()
{
    return new t_omp_queue();
}

void set_omp_worker_info_iteration(struct t_omp_queue *q, int omp_it, int iteration) 
{
    struct t_omp_worker_info &omp_worker = q->winfo[omp_it];
    omp_worker.iteration = iteration;
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

dimemas_timer get_omp_master_time( struct t_omp_queue *q, int omp_it )
{
    return q->winfo[omp_it].init_master_time;
}

dimemas_timer get_omp_worker_duration( struct t_omp_queue *q, int omp_it, int thread_id )
{
    return q->winfo[omp_it].worker_duration[thread_id];
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

bool is_omp_master_info_ready( struct t_omp_queue *q, int omp_it )
{
    map< int, struct t_omp_worker_info >::iterator itr = q->winfo.find( omp_it );

    if( itr == q->winfo.end() )
        return false;
 
    struct t_omp_worker_info &omp_worker = itr->second;

    if( omp_worker.init_master_time == -1.0 )
        return false;

    return true;
}

void add_omp_worker_event( struct t_omp_queue *q, int omp_it, int thread_id, struct t_omp_event *event )
{
    struct t_omp_worker_info &omp_worker = q->winfo[omp_it];
    omp_worker.events_sequence[ thread_id ].push_back( *event );
}

vector< t_omp_event >& get_omp_worker_events_sequence( struct t_omp_queue *q, int omp_it, int thread_id )
{
    struct t_omp_worker_info &omp_worker = q->winfo[omp_it];
    return omp_worker.events_sequence[ thread_id ];
}

void omp_print_event( struct t_thread *thread, struct t_even *event, int iteration )
{
    struct t_cpu *cpu = get_cpu_of_thread(thread);
    dimemas_timer tmp_event_time = 0;
    
    if( OMPEventEncoding_Is_OMPWorker_Running( thread->omp_in_block_event ) )
    {
        tmp_event_time = get_omp_master_time( thread->task->omp_queue, iteration );
    }
    else if(OMPEventEncoding_Is_OMPSync( thread->omp_in_block_event ) )
    {
        tmp_event_time = thread->omp_last_running_end;
    }
    else if( OMPEventEncoding_Is_OMPWorker_After_Synchro( thread->omp_in_block_event ) )
    {
        tmp_event_time = thread->omp_last_synchro_end;
    }
    else if( OMPEventEncoding_Is_OMPWorker_Running_End( thread->omp_in_block_event ) )
    {
        tmp_event_time = thread->omp_last_running_end;
    }
    else
    {
        tmp_event_time = current_time;
    }

    PARAVER_Event( cpu->unique_number,
                   IDENTIFIERS(thread),
                   tmp_event_time,
                   event->type,
                   event->value );
}

/**
 * Treating the OpenMP events
 * If any OpenMP states exist we have to print as they were
 * else we will print the events as they were.
 */
void treat_omp_events( struct t_thread *thread, struct t_even *event, dimemas_timer current_time, int iteration )
{
    struct t_cpu *cpu;
    cpu = get_cpu_of_thread( thread );
    
    if( !OMPEventEncoding_Is_OMPBlock( event->type ) )
    {
        return;
    }

    if( thread->omp_master_thread )
    {
        if( OMPEventEncoding_Is_OMP_Running( thread->omp_in_block_event ) )
        {
            PARAVER_Running( cpu->unique_number,
                            IDENTIFIERS( thread ),
                            thread->omp_in_block_event.paraver_time,
                            current_time );
        }
        else if(OMPEventEncoding_Is_OMPSync(thread->omp_in_block_event))
        {
            PARAVER_Thread_Sync(cpu->unique_number,
                    IDENTIFIERS(thread),
                    thread->omp_in_block_event.paraver_time,
                    current_time);
        }
        else if(OMPEventEncoding_Is_OMPWork_Dist(thread->omp_in_block_event))
        {
            PARAVER_Thread_Sched(cpu->unique_number,
                    IDENTIFIERS(thread),
                    thread->omp_in_block_event.paraver_time,
                    current_time);
        }
        else if( OMPEventEncoding_Is_OMPIdle(thread->omp_in_block_event.type)
                 && !OMPEventEncoding_Is_BlockBegin(thread->omp_in_block_event.value))
        {
            PARAVER_Thread_Sched(cpu->unique_number,
                    IDENTIFIERS(thread),
                    thread->omp_in_block_event.paraver_time,
                    current_time);
        }
        else if(OMPEventEncoding_Is_OMPSched(thread->omp_in_block_event))
        {
            PARAVER_Thread_Sched(cpu->unique_number,
                    IDENTIFIERS(thread),
                    thread->omp_in_block_event.paraver_time,
                    current_time);
        }
        
        thread->omp_in_block_event.type = event->type;
        thread->omp_in_block_event.value = event->value;
        thread->omp_in_block_event.paraver_time = current_time;
    }
    else if( thread->omp_worker_thread )
    {
        if( thread->omp_in_block_event.type == 0 )
        {
            PARAVER_Not_Created( cpu->unique_number,
                                 IDENTIFIERS (thread),
                                 0,
                                 current_time );
        }
        else if ( OMPEventEncoding_Is_OMPWorker_Running( thread->omp_in_block_event ) )
        {
            PARAVER_Running( cpu->unique_number,
                             IDENTIFIERS (thread),
                             thread->omp_in_block_event.paraver_time,
                             current_time );
        }
        else if(OMPEventEncoding_Is_OMPSched(thread->omp_in_block_event))
        {
            PARAVER_Thread_Sched( cpu->unique_number,
                                  IDENTIFIERS(thread),
                                  thread->omp_in_block_event.paraver_time,
                                  current_time );
        }
        else if( OMPEventEncoding_Is_OMPSync( thread->omp_in_block_event ) )
        {
            PARAVER_Thread_Sync( cpu->unique_number,
                                 IDENTIFIERS(thread),
                                 thread->omp_in_block_event.paraver_time,
                                 current_time );
        }
        else if (OMPEventEncoding_Is_OMPWorker_After_Synchro(thread->omp_in_block_event))
        {
            PARAVER_Running( cpu->unique_number,
                             IDENTIFIERS (thread),
                             thread->omp_in_block_event.paraver_time,
                             current_time );
        }

        thread->omp_in_block_event.type = event->type;
        thread->omp_in_block_event.value = event->value;
        thread->omp_in_block_event.paraver_time = current_time;
    }
}
