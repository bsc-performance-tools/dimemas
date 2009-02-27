/*
 *	Author: Tomislav Ziavnovic
 *	PFC: NEW DIMEMAS DATA ACCES LAYER API 
 */

#include <stdio.h>
#include "types.h"

// API PUBLIC STRUCTURES 
struct ptask_structure
{
  int  ptask_id;
  int  task_count;
  int *threads_per_task;
};

// PTASK STRUCTURE FUNCTIONS
int 
DAP_get_ptask_task_num(struct ptask_structure* ptask);

int
DAP_get_thread_ptask_num(struct ptask_structure* ptask, int task);

// API MAIN FUNCTIONS
extern t_boolean
DATA_ACCESS_init(int ptask_id, char* trace_file_location);

extern t_boolean
DATA_ACCESS_get_ptask_structure(int                      ptask_id,
                                struct ptask_structure** ptask_info);

extern t_boolean
DATA_ACCESS_get_communicators(int ptask_id, struct t_queue** communicators);

extern t_boolean
DATA_ACCESS_get_next_action(int               ptask_id, 
                            int               task_id, 
                            int               thread_id, 
                            struct t_action** action);

int
DATA_ACCESS_test_routine(int ptask_id);

extern char*
DATA_ACCESS_get_error();

extern void
DATA_ACCESS_end();

#define OFFSET_NOT_PRESENT 0
#define OFFSET_PRESENT     1

#define DEF_COMMUNICATOR 1
#define DEF_FILE         2
#define DEF_ONESIDED_WIN 3

#define RECORD_CPU_BURST 1
#define RECORD_MSG_SEND  2
#define RECORD_MSG_RECV  3
#define RECORD_GLOBAL_OP 10
#define RECORD_EVENT     20

#define RECVTYPE_RECV  0
#define RECVTYPE_IRECV 1
#define RECVTYPE_WAIT  2


