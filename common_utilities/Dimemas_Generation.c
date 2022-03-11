/* ---------------------------------------------------- Include Files -------*/
#include <CommonMacros.h>
#include <Dimemas_Generation.h>
#include <EventEncoding.h>

#ifdef HAVE_CONFIG_H
#  ifdef CLUSTERING_SUITE
#    include "clustering_suite_config.h"
#  else
#    include "config.h"
#  endif
#endif


int Dimemas_NX_Generic_Send( FILE *fd,
                             int task,
                             int thread,
                             int task_r,
                             int thread_r, /* receiver */
                             int commid,
                             int size,
                             long64_t tag,
                             int synchronism )
{
  return fprintf( fd, NX_GENERIC_SEND_STRING, task, thread, task_r, thread_r, size, tag, commid, synchronism );
}


int Dimemas_NX_Send( FILE *fd,
                     int task,
                     int thread,
                     int task_r,
                     int thread_r, /* sender */
                     int commid,
                     int size,
                     long64_t tag )
{
  /* synchronism: NO immediate + NO rendezvous = 0 */
  return fprintf( fd, NX_SEND_STRING, task, thread, task_r, thread_r, size, tag, commid );
}


/******************************************************************************
 **      Function name : Dimemas_NX_Immediate_Send
 **
 **      Description :
 ******************************************************************************/

int Dimemas_NX_ImmediateSend( FILE *fd,
                              int task,
                              int thread,
                              int task_r,
                              int thread_r, /* receiver */
                              int commid,
                              int size,
                              long64_t tag )
{
  /* synchronism: immediate + NO rendezvous = 2 */
  return fprintf( fd, NX_ISEND_STRING, task, thread, task_r, thread_r, size, tag, commid );
}

/******************************************************************************
 **      Function name : Dimemas_NX_ImmediateReady_Send
 **
 **      Description :
 ******************************************************************************/

int Dimemas_NX_ImmediateReadySend( FILE *fd,
                                   int task,
                                   int thread,
                                   int task_r,
                                   int thread_r, /* receiver */
                                   int commid,
                                   int size,
                                   long64_t tag )
{
  /* synchronism: immediate + rendezvous = 3 */
  return fprintf( fd, NX_ISEND_READY_STRING, task, thread, task_r, thread_r, size, tag, commid );
}


/******************************************************************************
 **      Function name : Dimemas_NX_BlockingSend
 **
 **      Description :
 ******************************************************************************/

int Dimemas_NX_BlockingSend( FILE *fd,
                             int task,
                             int thread,
                             int task_r,
                             int thread_r, /* receiver */
                             int commid,
                             int size,
                             long64_t tag )
{
  /* synchronism: NO immediate + rendezvous = 1 */
  return fprintf( fd, NX_BSEND_STRING, task, thread, task_r, thread_r, size, tag, commid );
}

/******************************************************************************
 **      Function name : Dimemas_NX_Generic_Recv
 **
 **      Description :
 ******************************************************************************/

int Dimemas_NX_Generic_Recv( FILE *fd,
                             int task,
                             int thread,
                             int task_s,
                             int thread_s, /* sender */
                             int commid,
                             int size,
                             long64_t tag,
                             int type )
{
  return fprintf( fd, NX_GENERIC_RECV_STRING, task, thread, task_s, thread_s, size, tag, commid, type );
}

/******************************************************************************
 **      Function name : Dimemas_NX_Recv
 **
 **      Description :
 ******************************************************************************/

int Dimemas_NX_Recv( FILE *fd,
                     int task,
                     int thread,
                     int task_s,
                     int thread_s, /* sender */
                     int commid,
                     int size,
                     long64_t tag )
{
  return fprintf( fd, NX_RECV_STRING, task, thread, task_s, thread_s, size, tag, commid );
}


int Dimemas_NX_Irecv( FILE *fd,
                      int task,
                      int thread,
                      int task_s,
                      int thread_s, /* sender */
                      int commid,
                      int size,
                      long64_t tag )
{
  return fprintf( fd, NX_IRECV_STRING, task, thread, task_s, thread_s, size, tag, commid );
}


int Dimemas_NX_Wait( FILE *fd,
                     int task,
                     int thread,
                     int task_s,
                     int thread_s, /* sender */
                     int commid,
                     int size,
                     long64_t tag )
{
  return fprintf( fd, NX_WAIT_STRING, task, thread, task_s, thread_s, size, tag, commid );
}


int Dimemas_CPU_Burst( FILE *fd, int task, int thread, double burst_time )
{
  return fprintf( fd, CPU_BURST_STRING, task, thread, burst_time );
}


int Dimemas_Communicator_Definition( FILE *fd, long long commid, int Ntasks, int *TaskList )
{
  int ii;
  if ( fprintf( fd, COMMUNICATOR_STRING, commid, Ntasks ) < 0 )
    return -1;

  for ( ii = 0; ii < Ntasks; ii++ )
  {
    if ( fprintf( fd, ":%d", TaskList[ ii ] ) < 0 )
      return -1;
  }

  if ( fprintf( fd, "\n" ) < 0 )
    return -1;

  return 1;
}


int Dimemas_User_Event( FILE *fd, int task, int thread, long64_t type, long64_t value )
{
  return fprintf( fd, USER_EVENT_STRING, task, thread, type, value );
}


int Dimemas_Block_Begin( FILE *fd, int task, int thread, long64_t type, long64_t value )
{
  return fprintf( fd, BLOCK_BEGIN_STRING, task, thread, type, value );
}


int Dimemas_Block_End( FILE *fd, int task, int thread, long64_t block )
{
  return fprintf( fd, BLOCK_END_STRING, task, thread, block );
}


int Dimemas_Global_OP( FILE *fd,
                       int task,
                       int thread,
                       int opid,
                       int commid,
                       int root_rank,
                       int root_thd,
                       long64_t sendsize,
                       long64_t recvsize,
                       int synchronize )
{
  return fprintf( fd, GLOBAL_OP_STRING, task, thread, opid, commid, root_rank, root_thd, sendsize, recvsize, synchronize );
}


int Dimemas_Global_OP_Wait( FILE *fd, int task, int thread, int opid, int commid, int root_rank, int root_thd, long64_t sendsize, long64_t recvsize )
{
  return fprintf( fd, GLOBAL_OP_WAIT_STRING, task, thread, opid, commid, root_rank, root_thd, sendsize, recvsize );
}


int Dimemas_NOOP( FILE *fd, int task, int thread )
{
  return fprintf( fd, NOOP_STRING, task, thread );
}


int Dimemas_GPU_Burst( FILE *fd, int task, int thread, double burst_time )
{
  return fprintf( fd, GPU_BURST_STRING, task, thread, burst_time );
}
