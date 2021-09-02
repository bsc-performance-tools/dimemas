#ifndef _DIMEMAS_HEADER_H
#define _DIMEMAS_HEADER_H


#define NX_GENERIC_SEND_STRING "2:%d:%d:%d:%d:%d:%lld:%d:%d\n"
#define NX_SEND_STRING         "2:%d:%d:%d:%d:%d:%lld:%d:0\n"
#define NX_ISEND_STRING        "2:%d:%d:%d:%d:%d:%lld:%d:2\n"
#define NX_ISEND_READY_STRING  "2:%d:%d:%d:%d:%d:%lld:%d:3\n"
#define NX_BSEND_STRING        "2:%d:%d:%d:%d:%d:%lld:%d:1\n"
#define NX_GENERIC_RECV_STRING "3:%d:%d:%d:%d:%d:%lld:%d:%d\n"
#define NX_RECV_STRING         "3:%d:%d:%d:%d:%d:%lld:%d:0\n"
#define NX_IRECV_STRING        "3:%d:%d:%d:%d:%d:%lld:%d:1\n"
#define NX_WAIT_STRING         "3:%d:%d:%d:%d:%d:%lld:%d:2\n"
#define CPU_BURST_STRING       "1:%d:%d:%.9f\n"
#define COMMUNICATOR_STRING    "d:1:%lld:%d"
#define USER_EVENT_STRING      "20:%d:%d:%lld:%lld\n"
#define BLOCK_BEGIN_STRING     "20:%d:%d:%lld:%lld\n"
#define BLOCK_END_STRING       "20:%d:%d:%lld:0\n"
#define GLOBAL_OP_STRING       "10:%d:%d:%d:%d:%d:%d:%lld:%lld:%d\n"
#define GLOBAL_OP_WAIT_STRING  "10:%d:%d:%d:%d:%d:%d:%lld:%lld:2\n"
#define NOOP_STRING            "0:%d:%d\n"
#define GPU_BURST_STRING       "11:%d:%d:%.9f\n"

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>

/* ---------------------------------------------------- Data Types ----------*/
#if !defined( LONG64_T )
  typedef long long long64_t;
#  define LONG64_T
#endif

  /* ---------------------------------------------------- Function Prototypes -*/
  int DimemasHeader_WriteHeader( FILE *fd );

  int DimemasHeader_WriteHeader_withAdditionalBlocks( FILE *fd, char **labels, int *values, int size );

  int Dimemas_NX_Generic_Send( FILE *fd,
                               int task,
                               int thread,
                               int task_r,
                               int thread_r, /* receiver */
                               int commid,
                               int size,
                               long64_t tag,
                               int synchronism );

  int Dimemas_NX_Send( FILE *fd,
                       int task,
                       int thread,
                       int task_s,
                       int thread_s, /* sender */
                       int commid,
                       int size,
                       long64_t tag );

  int Dimemas_NX_ImmediateSend( FILE *fd,
                                int task,
                                int thread,
                                int task_r,
                                int thread_r, /* receiver */
                                int commid,
                                int size,
                                long64_t tag );

  int Dimemas_NX_ImmediateReadySend( FILE *fd,
                                     int task,
                                     int thread,
                                     int task_r,
                                     int thread_r, /* receiver */
                                     int commid,
                                     int size,
                                     long64_t tag );

  int Dimemas_NX_BlockingSend( FILE *fd,
                               int task,
                               int thread,
                               int task_r,
                               int thread_r, /* receiver */
                               int commid,
                               int size,
                               long64_t tag );

  int Dimemas_NX_Generic_Recv( FILE *fd,
                               int task,
                               int thread,
                               int task_s,
                               int thread_s, /* sender */
                               int commid,
                               int size,
                               long64_t tag,
                               int type );

  int Dimemas_NX_Recv( FILE *fd,
                       int task,
                       int thread,
                       int task_s,
                       int thread_s, /* sender */
                       int commid,
                       int size,
                       long64_t tag );

  int Dimemas_NX_Irecv( FILE *fd,
                        int task,
                        int thread,
                        int task_s,
                        int thread_s, /* sender */
                        int commid,
                        int size,
                        long64_t tag );

  int Dimemas_NX_Wait( FILE *fd,
                       int task,
                       int thread,
                       int task_s,
                       int thread_s, /* sender */
                       int commid,
                       int size,
                       long64_t tag );

  int Dimemas_Communicator_Definition( FILE *fd, long long commid, int Ntasks, int *TaskList );

  int Dimemas_CPU_Burst( FILE *fd, int task, int thread, double burst_time );

  int Dimemas_User_Event( FILE *fd, int task, int thread, long64_t type, long64_t value );

  int Dimemas_Block_Definition( FILE *fd, long64_t block, const char *Label );


#ifdef NEW_DIMEMAS_TRACE
  int Dimemas_Block_Begin( FILE *fd, int task, int thread, long64_t Type, long64_t Value );
#else
int Dimemas_Block_Begin( FILE *fd, int task, int thread, long64_t block );
#endif

  int Dimemas_Block_End( FILE *fd, int task, int thread, long64_t block );

  int Dimemas_Global_OP( FILE *fd,
                         int task,
                         int thread,
                         int opid,
                         int commid,
                         int root_rank,
                         int root_thd,
                         long64_t sendsize,
                         long64_t recvsize,
                         int synchronize );

  int Dimemas_Global_OP_Wait( FILE *fd,
                              int task,
                              int thread,
                              int opid,
                              int commid,
                              int root_rank,
                              int root_thd,
                              long64_t sendsize,
                              long64_t recvsize );

  int Dimemas_User_EventType_Definition( FILE *fd, long64_t type, char *Label, int color );

  int Dimemas_User_EventValue_Definition( FILE *fd, long64_t type, long64_t value, char *Label );

  int Dimemas_NOOP( FILE *fd, int task, int thread );

  int Dimemas_GPU_Burst( FILE *fd, int task, int thread, double burst_time );

#ifdef __cplusplus
}
#endif

#endif /* _DIMEMAS_HEADER_H */
