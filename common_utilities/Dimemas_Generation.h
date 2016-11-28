#ifndef _DIMEMAS_HEADER_H
#define _DIMEMAS_HEADER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>

/* ---------------------------------------------------- Data Types ----------*/
#if !defined( LONG64_T )
typedef long long long64_t;
#define LONG64_T
#endif

/* ---------------------------------------------------- Function Prototypes -*/
int DimemasHeader_WriteHeader( FILE *fd );

int DimemasHeader_WriteHeader_withAdditionalBlocks( FILE  *fd,
                                                    char **labels,
                                                    int   *values,
                                                    int    size );

int Dimemas_NX_Generic_Send( FILE *fd,
                             int task,   int thread,
                             int task_r, int thread_r, /* receiver */
                             int commid,
                             int size, long64_t tag,
                             int synchronism );

int Dimemas_NX_Send( FILE *fd,
                     int task,   int thread,
                     int task_s, int thread_s, /* sender */
                     int commid,
                     int size, long64_t tag );

int Dimemas_NX_ImmediateSend( FILE *fd,
                     int task,   int thread,
                     int task_r, int thread_r, /* receiver */
                     int commid,
                     int size, long64_t tag );

int Dimemas_NX_ImmediateReadySend( FILE *fd,
                                  int task,   int thread,
                                  int task_r, int thread_r, /* receiver */
                                  int commid,
                                  int size, long64_t tag );

int Dimemas_NX_BlockingSend( FILE *fd,
                             int task,   int thread,
                             int task_r, int thread_r, /* receiver */
                             int commid,
                             int size, long64_t tag );

int Dimemas_NX_Generic_Recv( FILE *fd,
                             int task,   int thread,
                             int task_s, int thread_s, /* sender */
                             int commid,
                             int size, long64_t tag,
                             int type );

int Dimemas_NX_Recv( FILE *fd,
                     int task,   int thread,
                     int task_s, int thread_s, /* sender */
                     int commid,
                     int size, long64_t tag );

int Dimemas_NX_Irecv( FILE *fd,
                      int task,   int thread,
                      int task_s, int thread_s, /* sender */
                      int commid,
                      int size, long64_t tag );

int Dimemas_NX_Wait( FILE *fd,
                     int task,   int thread,
                     int task_s, int thread_s, /* sender */
                     int commid,
                     int size, long64_t tag );

int Dimemas_Communicator_Definition( FILE *fd,
                                     long long commid,
                                     int Ntasks,
                                     int *TaskList );

int Dimemas_CPU_Burst( FILE *fd,
                       int task, int thread,
                       double burst_time );

int Dimemas_User_Event( FILE *fd,
                        int task, int thread,
                        long64_t type, long64_t value );

int Dimemas_Block_Definition( FILE *fd,
                              long64_t block,
                              const char *Label );


#ifdef NEW_DIMEMAS_TRACE
int Dimemas_Block_Begin( FILE *fd,
                         int task, int thread,
                         long64_t Type, long64_t Value );
#else
int Dimemas_Block_Begin( FILE *fd,
                         int task, int thread,
                         long64_t block );
#endif

int Dimemas_Block_End( FILE *fd,
                       int task, int thread,
                       long64_t block );

int Dimemas_Global_OP( FILE *fd,
                       int task, int thread,
                       int opid, int commid,
                       int root_rank, int root_thd,
                       long64_t sendsize, long64_t recvsize, 
											 int synchronize );

int Dimemas_Global_OP_Wait( FILE *fd,
                       int task, int thread,
                       int opid, int commid,
                       int root_rank, int root_thd,
                       long64_t sendsize, long64_t recvsize );

int Dimemas_User_EventType_Definition( FILE *fd,
                                       long64_t type,
                                       char *Label,
                                       int color );

int Dimemas_User_EventValue_Definition( FILE *fd,
                                        long64_t type,
                                        long64_t value,
                                        char *Label );

int Dimemas_NOOP ( FILE *fd, int task, int thread );

int Dimemas_GPU_Burst( FILE *fd,
                       	 	 	 int task, int thread,
														 double burst_time );

#ifdef __cplusplus
}
#endif

#endif /* _DIMEMAS_HEADER_H */
