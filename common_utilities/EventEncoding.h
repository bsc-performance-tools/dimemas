#ifndef _EVENT_ENCODING_H
#define _EVENT_ENCODING_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include "../include/types.h"

#if !defined( LONG64_T )
typedef long long long64_t;
#define LONG64_T
#endif

/* ==========================================================================
 * ==== Special event 0
 * ========================================================================== */

/* This event is used in Dimemas new trace to mark special behavior regions   */

#define SPECIAL_EVENT_TYPE            0
#define IDLE_EVENT_TYPE               0

/* ==========================================================================
   ==== User Function/Calls
   ========================================================================== */
#define USER_FUNCTION                 60000019
#define USER_CALL                     60000020
#define USER_BLOCK                    60000021

#define USER_FUNCTION_LABEL           "User functions"
#define USER_CALL_LABEL               "User calls"
#define USER_BLOCK_LABEL              "User Block"

#define BASE_USERFUNCTION             200
#define BASE_CLUSTER_BLOCK            1000
#define BASE_USERCALL                 10000
#define BASE_USERBLOCK                20000

/* ==========================================================================
   ==== MPI Event Types
   ========================================================================== */
typedef int MPIType;

#define MPITYPE_PTOP                  50000001
#define MPITYPE_COLLECTIVE            50000002

#define MPITYPE_OTHER                 50000003
#define MPITYPE_RMA                   50000004
#define MPITYPE_COMM                  MPITYPE_OTHER
#define MPITYPE_GROUP                 MPITYPE_OTHER
#define MPITYPE_TOPOLOGIES            MPITYPE_OTHER
#define MPITYPE_TYPE                  MPITYPE_OTHER
#define MPITYPE_IO										50000005

#define MPITYPE_PROBE_SOFTCOUNTER     50000300
#define MPITYPE_PROBE_TIMECOUNTER     50000301
#define MPITYPE_TEST_SOFTCOUNTER      50000080
#define MPI_GLOBAL_OP_SENDSIZE        (MPITYPE_PTOP+100000)
#define MPI_GLOBAL_OP_RECVSIZE        (MPITYPE_PTOP+100001)
#define MPI_GLOBAL_OP_ROOT            (MPITYPE_PTOP+100002)
#define MPI_GLOBAL_OP_COMM            (MPITYPE_PTOP+100003)

#define MPITYPE_COLLECTIVE_LABEL      "MPI Collective Comm"
#define MPITYPE_PTOP_LABEL            "MPI Point-to-point"

#define MPITYPE_OTHER_LABEL         "MPI Other"
#define MPITYPE_RMA_LABEL           "MPI One-sided"
#define MPITYPE_COMM_LABEL          MPITYPE_OTHER_LABEL
#define MPITYPE_GROUP_LABEL         MPITYPE_OTHER_LABEL
#define MPITYPE_TOPOLOGIES_LABEL    MPITYPE_OTHER_LABEL
#define MPITYPE_TYPE_LABEL          MPITYPE_OTHER_LABEL

#define MPITYPE_PROBE_SOFTCOUNTER_LABEL "MPI Probe software counter"
#define MPITYPE_TEST_SOFTCOUNTER_LABEL  "MPI Test/Testsome software counter"

#define FLUSHING_EV                   40000003
#define IO_READ_EV                    40000016
#define IO_WRITE_EV                   40000017
#define IO_EV                         40000018

#define IO_READ_LABEL                 "I/O Read"
#define IO_WRITE_LABEL                "I/O Write"
#define IO_LABEL                      "I/O Call"

/* ==========================================================================
   ==== MISC Event Types
   ========================================================================== */

#define SET_TRACE_EV           40000014

#define MPI_CALLER_EV          70000000
#define MPI_CALLER_EV_END      79999999
#define MPI_CALLER_LINE_EV     80000000
#define MPI_CALLER_LINE_EV_END 89999999

#define LAPI_EV                30000000

#define CLUSTER_ID_EV          90000001
#define CLUSTER_ID_LABEL       "Cluster ID"
#define CLUSTEREND_VAL         0

#define CLUSTER_GROUP_EV       90000002
#define CLUSTER_GROUP_LABEL    "Cluster Group"

/* ==========================================================================
   ==== CUDA Event Types
   ========================================================================== */

#define CUDA_LIB_CALL_EV        63000001
#define CUDA_LIB_CALL_LABEL     "CUDA library call"
#define CUDA_MEMCPY_SIZE_EV     63000002
#define CUDA_MEMCPY_SIZE_LABEL  "cudaMemcpy size"
#define CUDA_KERNEL_EV					63000019
#define CUDA_KERNEL_LABEL				"CUDA Kernel"
#define CUDA_KERNEL_SOURCE_EV		63000119
#define CUDA_KERNEL_SOURCE_LABEL "CUDA Kernel Source Code"
#define CUDA_SYNCH_STREAM_EV    63300000
#define CUDA_SYNCH_STREAM_LABEL "Synchronized stream (on thread)"
#define CUDA_TAG								49370

typedef enum
{
  CUDA_END_VAL        = 0,
  CUDA_LAUNCH_VAL,
  CUDA_CONFIGURECALL_VAL,
  CUDA_MEMCPY_VAL,
  CUDA_THREADSYNCHRONIZE_VAL,
  CUDA_STREAMSYNCHRONIZE_VAL,
  CUDA_MEMCPY_ASYNC_VAL,
  CUDA_DEVICE_RESET_VAL,
  CUDA_THREADEXIT_VAL
} CUDA_Event_Values;

/* ==========================================================================
   ==== OpenCL Event Types
   ========================================================================== */

#define OCL_HOST_CALL_EV        64000000
#define OCL_HOST_CALL_LABEL     "OpenCL host call"

typedef enum
{
/* 0 */		OCL_OUTSIDE_VAL	= 0,
/* 1 */		OCL_CREATE_BUFF_VAL,
/* 2 */		OCL_CREATE_COMMAND_QUEUE_VAL,
/* 3 */		OCL_CREATE_CONTEXT_VAL,
/* 4 */		OCL_CREATE_CONTEXT_FROM_TYPE_VAL,
/* 5 */		OCL_CREATE_SUBBUFFER_VAL,
/* 6 */		OCL_CREATE_KERNEL_VAL,
/* 7 */		OCL_CREATE_KERNELS_IN_PROGRAM_VAL,
/* 8 */		OCL_SET_KERNEL_ARGS_VAL,
/* 9 */		OCL_CREATE_PROGRAM_WITH_SOURCE_VAL,
/* 10 */	OCL_CREATE_PROGRAM_WITH_BINARY_VAL,
/* 11 */	OCL_CREATE_PROGRAM_WITH_BUILTIN_KERNELS_VAL,
/* 12 */	OCL_ENQUEUE_FILL_BUFFER_VAL,
/* 13 */	OCL_ENQUEUE_COPY_BUFFER_VAL,
/* 14 */	OCL_ENQUEUE_COPY_BUFFER_RECT_VAL,
/* 15 */	OCL_ENQUEUE_NDRANGE_KERNEL_VAL,
/* 16 */	OCL_ENQUEUE_TASK_VAL,
/* 17 */	OCL_ENQUEUE_NATIVE_KERNEL_VAL,
/* 18 */	OCL_ENQUEUE_READ_BUFF_VAL,
/* 19 */	OCL_ENQUEUE_READ_BUFF_RECT_VAL,
/* 20 */	OCL_ENQUEUE_WRITE_BUFF_VAL,
/* 21 */	OCL_ENQUEUE_WRITE_BUFF_RECT_VAL,
/* 22 */	OCL_BUILD_PROGRAM_VAL,
/* 23 */	OCL_COMPILE_PROGRAM_VAL,
/* 24 */	OCL_LINK_PROGRAM_VAL,
/* 25 */	OCL_FINISH_VAL,
/* 26 */	OCL_FLUSH_VAL,
/* 27 */	OCL_WAIT_FOR_EVENTS_VAL,
/* 28 */	OCL_ENQUEUE_MARKER_WITH_WAIT_LIST_VAL,
/* 29 */	OCL_ENQUEUE_BARRIER_WITH_WAIT_LIST_VAL,
/* 30 */	OCL_ENQUEUE_MAP_BUFFER_VAL,
/* 31 */	OCL_ENQUEUE_UNMAP_MEM_OBJ_VAL,
/* 32 */	OCL_ENQUEUE_MIGRATE_MEM_OBJ_VAL,
/* 33 */	OCL_ENQUEUE_MARKER_VAL,
/* 34 */	OCL_ENQUEUE_BARRIER_VAL,
/* 35 */	OCL_RETAIN_COMMAND_QUEUE_VAL,
/* 36 */	OCL_RELEASE_COMMAND_QUEUE_VAL,
/* 37 */	OCL_RETAIN_CONTEXT_VAL,
/* 38 */	OCL_RELEASE_CONTEXT_VAL,
/* 39 */	OCL_RETAIN_DEVICE_VAL,
/* 40 */	OCL_RELEASE_DEVICE_VAL,
/* 41 */	OCL_RETAIN_EVENT_VAL,
/* 42 */	OCL_RELEASE_EVENT_VAL,
/* 43 */	OCL_RETAIN_KERNEL_VAL,
/* 44 */	OCL_RELEASE_KERNEL_VAL,
/* 45 */	OCL_RETAIN_MEM_OBJ_VAL,
/* 46 */	OCL_RELEASE_MEM_OBJ_VAL,
/* 47 */	OCL_RETAIN_PROGRAM_VAL,
/* 48 */	OCL_RELEASE_PROGRAM_VAL
} OCL_Host_Event_Values;

#define OCL_TRANSF_SIZE_EV     			64099999
#define OCL_TRANSF_SIZE_LABEL   		"OpenCL transfer size"

#define OCL_ACCELERATOR_CALL_EV			64100000
#define OCL_ACCELERATOR_CALL_LABEL	"OpenCL Accelerator call"

typedef enum
{
/* 0 */		OCL_OUTSIDE_ACC_VAL = 0,
/* 12 */	OCL_ENQUEUE_FILL_BUFFER_ACC_VAL = 12,
/* 13 */	OCL_ENQUEUE_COPY_BUFFER_ACC_VAL,
/* 14 */	OCL_ENQUEUE_COPY_BUFFER_RECT_ACC_VAL,
/* 15 */	OCL_ENQUEUE_NDRANGE_KERNEL_ACC_VAL,
/* 16 */	OCL_ENQUEUE_TASK_ACC_VAL,
/* 17 */	OCL_ENQUEUE_NATIVE_KERNEL_ACC_VAL,
/* 18 */	OCL_ENQUEUE_READ_BUFF_ACC_VAL,
/* 19 */	OCL_ENQUEUE_READ_BUFF_RECT_ACC_VAL,
/* 20 */	OCL_ENQUEUE_WRITE_BUFF_ACC_VAL,
/* 21 */	OCL_ENQUEUE_WRITE_BUFF_RECT_ACC_VAL,
/* 25 */	OCL_FINISH_ACC_VAL = 25,
/* 28 */	OCL_ENQUEUE_MARKER_WITH_WAIT_LIST_ACC_VAL = 28,
/* 29 */	OCL_ENQUEUE_BARRIER_WITH_WAIT_LIST_ACC_VAL,
/* 30 */	OCL_ENQUEUE_MAP_BUFFER_ACC_VAL,
/* 31 */	OCL_ENQUEUE_UNMAP_MEM_OBJ_ACC_VAL,
/* 32 */	OCL_ENQUEUE_MIGRATE_MEM_OBJ_ACC_VAL,
/* 33 */	OCL_ENQUEUE_MARKER_ACC_VAL,
/* 34 */	OCL_ENQUEUE_BARRIER_ACC_VAL
} OCL_Accelerator_Event_Values;

#define OCL_KERNEL_NAME_EV					64200000
#define OCL_KERNEL_NAME_LABEL				"OpenCL kernel name"
#define OCL_SYNCH_STREAM_EV    			64300000
#define OCL_SYNCH_STREAM_LABEL 			"OpenCL Synchronized stream (on thread)"
#define OCL_TAG											3121

/*
EVENT_TYPE
0		64000000    OpenCL host call
VALUES
0 Outside OpenCL
1 clCreateBuffer
2 clCreateCommandQueue
3 clCreateContext
4 clCreateContextFromType
5 clCreateSubBuffer
6 clCreateKernel
7 clCreateKernelsInProgram
8 clSetKernelArg
9 clCreateProgramWithSource
10 clCreateProgramWithBinary
11 clCreateProgramWithBuiltInKernels
12 clEnqueueFillBuffer
13 clEnqueueCopyBuffer
14 clEnqueueCopyBufferRect
15 clEnqueueNDRangeKernel
16 clEnqueueTask
17 clEnqueueNativeKernel
18 clEnqueueReadBuffer
19 clEnqueueReadBufferRect
20 clEnqueueWriteBuffer
21 clEnqueueWriteBufferRect
22 clBuildProgram
23 clCompileProgram
24 clLinkProgram
25 clFinish
26 clFlush
27 clWaitForEvents
28 clEnqueueMarkerWithWaitList
29 clEnqueueBarrierWithWaitList
30 clEnqueueMapBuffer
31 clEnqueueUnmapMemObject
32 clEnqueueMigrateMemObjects
33 clEnqueueMarker
34 clEnqueueBarrier
35 clRetainCommandQueue
36 clReleaseCommandQueue
37 clRetainContext
38 clReleaseContext
39 clRetainDevice
40 clReleaseDevice
41 clRetainEvent
42 clReleaseEvent
43 clRetainKernel
44 clReleaseKernel
45 clRetainMemObject
46 clReleaseMemObject
47 clRetainProgram
48 clReleaseProgram

EVENT_TYPE
0   64099999    OpenCL transfer size

EVENT_TYPE
0		64100000		Accelerator OpenCL call
VALUES
0 Outside OpenCL
12 clEnqueueFillBuffer
13 clEnqueueCopyBuffer
14 clEnqueueCopyBufferRect
15 clEnqueueNDRangeKernel
16 clEnqueueTask
17 clEnqueueNativeKernel
18 clEnqueueReadBuffer
19 clEnqueueReadBufferRect
20 clEnqueueWriteBuffer
21 clEnqueueWriteBufferRect
25 clFinish
28 clEnqueueMarkerWithWaitList
29 clEnqueueBarrierWithWaitList
30 clEnqueueMapBuffer
31 clEnqueueUnmapMemObject
32 clEnqueueMigrateMemObjects
33 clEnqueueMarker
34 clEnqueueBarrier

EVENT_TYPE
0    64200000    OpenCL kernel name
VALUES
1      kernel_name
*/

/* ==========================================================================
   ==== MPI Event Values
   ========================================================================== */
typedef enum
{
  MPIEND_VAL,

  MPI_SEND_VAL,                           /* 001 */
  MPI_RECV_VAL,                           /* 002 */
  MPI_ISEND_VAL,                          /* 003 */
  MPI_IRECV_VAL,                          /* 004 */
  MPI_WAIT_VAL,                           /* 005 */
  MPI_WAITALL_VAL,                        /* 006 */

  MPI_BCAST_VAL,                          /* 007 */
  MPI_BARRIER_VAL,                        /* 008 */
  MPI_REDUCE_VAL,                         /* 009 */
  MPI_ALLREDUCE_VAL,                      /* 010 */

  MPI_ALLTOALL_VAL,                       /* 011 */
  MPI_ALLTOALLV_VAL,                      /* 012 */
  MPI_GATHER_VAL,                         /* 013 */
  MPI_GATHERV_VAL,                        /* 014 */
  MPI_SCATTER_VAL,                        /* 015 */
  MPI_SCATTERV_VAL,                       /* 016 */
  MPI_ALLGATHER_VAL,                      /* 017 */
  MPI_ALLGATHERV_VAL,                     /* 018 */

  MPI_COMM_RANK_VAL,                      /* 019 */
  MPI_COMM_SIZE_VAL,                      /* 020 */
  MPI_COMM_CREATE_VAL,                    /* 021 */
  MPI_COMM_DUP_VAL,                       /* 022 */
  MPI_COMM_SPLIT_VAL,                     /* 023 */
  MPI_COMM_GROUP_VAL,                     /* 024 */
  MPI_COMM_FREE_VAL,                      /* 025 */
  MPI_COMM_REMOTE_GROUP_VAL,              /* 026 */
  MPI_COMM_REMOTE_SIZE_VAL,               /* 027 */
  MPI_COMM_TEST_INTER_VAL,                /* 028 */
  MPI_COMM_COMPARE_VAL,                   /* 029 */

  MPI_SCAN_VAL,                           /* 030 */

  MPI_INIT_VAL,                           /* 031 */
  MPI_FINALIZE_VAL,                       /* 032 */
  MPI_BSEND_VAL,                          /* 033 */
  MPI_SSEND_VAL,                          /* 034 */
  MPI_RSEND_VAL,                          /* 035 */
  MPI_IBSEND_VAL,                         /* 036 */
  MPI_ISSEND_VAL,                         /* 037 */
  MPI_IRSEND_VAL,                         /* 038 */
  MPI_TEST_VAL,                           /* 039 */
  MPI_CANCEL_VAL,                         /* 040 */
  MPI_SENDRECV_VAL,                       /* 041 */
  MPI_SENDRECV_REPLACE_VAL,               /* 042 */
  MPI_CART_CREATE_VAL,                    /* 043 */
  MPI_CART_SHIFT_VAL,                     /* 044 */
  MPI_CART_COORDS_VAL,                    /* 045 */
  MPI_CART_GET_VAL,                       /* 046 */
  MPI_CART_MAP_VAL,                       /* 047 */
  MPI_CART_RANK_VAL,                      /* 048 */
  MPI_CART_SUB_VAL,                       /* 049 */
  MPI_CARTDIM_GET_VAL,                    /* 050 */
  MPI_DIMS_CREATE_VAL,                    /* 051 */
  MPI_GRAPH_GET_VAL,                      /* 052 */
  MPI_GRAPH_MAP_VAL,                      /* 053 */
  MPI_GRAPH_CREATE_VAL,                   /* 054 */
  MPI_GRAPH_NEIGHBORS_VAL,                /* 055 */
  MPI_GRAPHDIMS_GET_VAL,                  /* 056 */
  MPI_GRAPH_NEIGHBORS_COUNT_VAL,          /* 057 */
  MPI_TOPO_TEST_VAL,                      /* 058 */

  MPI_WAITANY_VAL,                        /* 059 */
  MPI_WAITSOME_VAL,                       /* 060 */
  MPI_PROBE_VAL,                          /* 061 */
  MPI_IPROBE_VAL,                         /* 062 */

  MPI_WIN_CREATE_VAL,                     /* 063 */
  MPI_WIN_FREE_VAL,                       /* 064 */
  MPI_PUT_VAL,                            /* 065 */
  MPI_GET_VAL,                            /* 066 */
  MPI_ACCUMULATE_VAL,                     /* 067 */
  MPI_WIN_FENCE_VAL,                      /* 068 */
  MPI_WIN_START_VAL,                      /* 069 */
  MPI_WIN_COMPLETE_VAL,                   /* 070 */
  MPI_WIN_POST_VAL,                       /* 071 */
  MPI_WIN_WAIT_VAL,                       /* 072 */
  MPI_WIN_TEST_VAL,                       /* 073 */
  MPI_WIN_LOCK_VAL,                       /* 074 */
  MPI_WIN_UNLOCK_VAL,                     /* 075 */

  MPI_PACK_VAL,                           /* 076 */
  MPI_UNPACK_VAL,                         /* 077 */

  MPI_OP_CREATE_VAL,                      /* 078 */
  MPI_OP_FREE_VAL,                        /* 079 */
  MPI_REDUCE_SCATTER_VAL,                 /* 080 */

  MPI_ATTR_DELETE_VAL,                    /* 081 */
  MPI_ATTR_GET_VAL,                       /* 082 */
  MPI_ATTR_PUT_VAL,                       /* 083 */

  MPI_GROUP_DIFFERENCE_VAL,               /* 084 */
  MPI_GROUP_EXCL_VAL,                     /* 085 */
  MPI_GROUP_FREE_VAL,                     /* 086 */
  MPI_GROUP_INCL_VAL,                     /* 087 */
  MPI_GROUP_INTERSECTION_VAL,             /* 088 */
  MPI_GROUP_RANK_VAL,                     /* 089 */
  MPI_GROUP_RANGE_EXCL_VAL,               /* 090 */
  MPI_GROUP_RANGE_INCL_VAL,               /* 091 */
  MPI_GROUP_SIZE_VAL,                     /* 092 */
  MPI_GROUP_TRANSLATE_RANKS_VAL,          /* 093 */
  MPI_GROUP_UNION_VAL,                    /* 094 */
  MPI_GROUP_COMPARE_VAL,                  /* 095 */

  MPI_INTERCOMM_CREATE_VAL,               /* 096 */
  MPI_INTERCOMM_MERGE_VAL,                /* 097 */
  MPI_KEYVAL_FREE_VAL,                    /* 098 */
  MPI_KEYVAL_CREATE_VAL,                  /* 099 */
  MPI_ABORT_VAL,                          /* 100 */
  MPI_ERROR_CLASS_VAL,                    /* 101 */
  MPI_ERRHANDLER_CREATE_VAL,              /* 102 */
  MPI_ERRHANDLER_FREE_VAL,                /* 103 */
  MPI_ERRHANDLER_GET_VAL,                 /* 104 */
  MPI_ERROR_STRING_VAL,                   /* 105 */
  MPI_ERRHANDLER_SET_VAL,                 /* 106 */
  MPI_GET_PROCESSOR_NAME_VAL,             /* 107 */
  MPI_INITIALIZED_VAL,                    /* 108 */
  MPI_WTICK_VAL,                          /* 109 */
  MPI_WTIME_VAL,                          /* 110 */
  MPI_ADDRESS_VAL,                        /* 111 */
  MPI_BSEND_INIT_VAL,                     /* 112 */
  MPI_BUFFER_ATTACH_VAL,                  /* 113 */
  MPI_BUFFER_DETACH_VAL,                  /* 114 */
  MPI_REQUEST_FREE_VAL,                   /* 115 */
  MPI_RECV_INIT_VAL,                      /* 116 */
  MPI_SEND_INIT_VAL,                      /* 117 */
  MPI_GET_COUNT_VAL,                      /* 118 */
  MPI_GET_ELEMENTS_VAL,                   /* 119 */
  MPI_PACK_SIZE_VAL,                      /* 120 */
  MPI_RSEND_INIT_VAL,                     /* 121 */
  MPI_SSEND_INIT_VAL,                     /* 122 */
  MPI_START_VAL,                          /* 123 */
  MPI_STARTALL_VAL,                       /* 124 */
  MPI_TESTALL_VAL,                        /* 125 */
  MPI_TESTANY_VAL,                        /* 126 */
  MPI_TEST_CANCELLED_VAL,                 /* 127 */
  MPI_TESTSOME_VAL,                       /* 128 */
  MPI_TYPE_COMMIT_VAL,                    /* 129 */
  MPI_TYPE_CONTIGUOUS_VAL,                /* 130 */
  MPI_TYPE_EXTENT_VAL,                    /* 131 */
  MPI_TYPE_FREE_VAL,                      /* 132 */
  MPI_TYPE_HINDEXED_VAL,                  /* 133 */
  MPI_TYPE_HVECTOR_VAL,                   /* 134 */
  MPI_TYPE_INDEXED_VAL,                   /* 135 */
  MPI_TYPE_LB_VAL,                        /* 136 */
  MPI_TYPE_SIZE_VAL,                      /* 137 */
  MPI_TYPE_STRUCT_VAL,                    /* 138 */
  MPI_TYPE_UB_VAL,                        /* 139 */
  //MPI_INIT_THREAD_VAL,                    /* 140 */

  MPI_TYPE_VECTOR_VAL,                     /* 141 */

	MPI_FILE_OPEN_VAL,					
  MPI_FILE_CLOSE_VAL,
  MPI_FILE_READ_VAL,
  MPI_FILE_READ_ALL_VAL,
  MPI_FILE_WRITE_VAL,
  MPI_FILE_WRITE_ALL_VAL,
  MPI_FILE_READ_AT_VAL,
  MPI_FILE_READ_AT_ALL_VAL,
  MPI_FILE_WRITE_AT_VAL,									/* 150 */

  MPI_FILE_WRITE_AT_ALL_VAL,							
  MPI_COMM_SPAWN_VAL,
  MPI_COMM_SPAWN_MULTIPLE_VAL,
  MPI_REQUEST_GET_STATUS_VAL,

  MPI_IREDUCE_VAL,												/* 155 */
  MPI_IALLREDUCE_VAL,											
  MPI_IBARRIER_VAL,
  MPI_IBCAST_VAL,
  MPI_IALLTOALL_VAL,
  MPI_IALLTOALLV_VAL,											/* 160 */
  MPI_IALLGATHER_VAL,									
  MPI_IALLGATHERV_VAL,
  MPI_IGATHER_VAL,
  MPI_IGATHERV_VAL,
  MPI_ISCATTER_VAL,												/* 165 */
  MPI_ISCATTERV_VAL,
  MPI_IREDUCESCAT_VAL,
  MPI_ISCAN_VAL,													/* 168 */

	// In order to have the same Ids than extrae I've moved this MPI
	// from 141 to here
  MPI_INIT_THREAD_VAL                    /* 140 */
} MPI_Event_Values;

//#define NUM_MPICALLS  (MPI_TYPE_VECTOR_VAL+1)
#define NUM_MPICALLS (MPI_INIT_THREAD_VAL+1)

#define BLOCK_END_VAL MPIEND_VAL          /* BLOCK_END_VAL == MPIEND_VAL */

typedef enum
{
/* 000 */  LAPI_END_VAL,
/* 001 */  LAPI_INIT_VAL,
/* 002 */  LAPI_TERM_VAL,
/* 003 */  LAPI_PUT_VAL,
/* 004 */  LAPI_GET_VAL,
/* 005 */  LAPI_FENCE_VAL,
/* 006 */  LAPI_GFENCE_VAL,
/* 007 */  LAPI_ADDRESS_INIT_VAL,
/* 008 */  LAPI_AMSEND_VAL,
/* 009 */  LAPI_RMW_VAL,
/* 010 */  LAPI_WAITCNTR_VAL
}LAPIVal;

/* ==========================================================================
   ==== MPI Dimemas Block Numbers
   ========================================================================== */

typedef enum
{
	BLOCK_ID_NULL,									/* 000 */
	BLOCK_ID_MPI_Allgather,					   
	BLOCK_ID_MPI_Allgatherv,
	BLOCK_ID_MPI_Allreduce,
	BLOCK_ID_MPI_Alltoall,
	BLOCK_ID_MPI_Alltoallv,
	BLOCK_ID_MPI_Barrier,
	BLOCK_ID_MPI_Bcast,
	BLOCK_ID_MPI_Gather,
	BLOCK_ID_MPI_Gatherv,
	BLOCK_ID_MPI_Reduce_scatter,		/* 010 */
  BLOCK_ID_MPI_Reduce,
  BLOCK_ID_MPI_Scan,
  BLOCK_ID_MPI_Scatter,
  BLOCK_ID_MPI_Scatterv,
  
	BLOCK_ID_MPI_Iallgather,
  BLOCK_ID_MPI_Iallgatherv,
  BLOCK_ID_MPI_Iallreduce,
	BLOCK_ID_MPI_Ialltoall,
  BLOCK_ID_MPI_Ialltoallv,
  BLOCK_ID_MPI_Ibarrier,					/* 020 */
  BLOCK_ID_MPI_Ibcast,
  BLOCK_ID_MPI_Igather,
  BLOCK_ID_MPI_Igatherv,
	BLOCK_ID_MPI_Ireducescat,
  BLOCK_ID_MPI_Ireduce,
  BLOCK_ID_MPI_Iscan,						
  BLOCK_ID_MPI_Iscatter,
  BLOCK_ID_MPI_Iscatterv,

	BLOCK_ID_MPI_Op_create,					
  BLOCK_ID_MPI_Op_free,						/* 030 */

  BLOCK_ID_MPI_Attr_delete,
  BLOCK_ID_MPI_Attr_get,
  BLOCK_ID_MPI_Attr_put,

  BLOCK_ID_MPI_Comm_create,
  BLOCK_ID_MPI_Comm_dup,
  BLOCK_ID_MPI_Comm_free,
  BLOCK_ID_MPI_Comm_group,
  BLOCK_ID_MPI_Comm_rank,
  BLOCK_ID_MPI_Comm_remote_group,
  BLOCK_ID_MPI_Comm_remote_size,	 /* 040 */
  BLOCK_ID_MPI_Comm_size,
  BLOCK_ID_MPI_Comm_split,
  BLOCK_ID_MPI_Comm_test_inter,
  BLOCK_ID_MPI_Comm_compare,
  BLOCK_ID_MPI_Group_difference,
  BLOCK_ID_MPI_Group_excl,
  BLOCK_ID_MPI_Group_free,
  BLOCK_ID_MPI_Group_incl,
  BLOCK_ID_MPI_Group_intersection,
  BLOCK_ID_MPI_Group_rank,				/* 050 */
  BLOCK_ID_MPI_Group_range_excl,
  BLOCK_ID_MPI_Group_range_incl,
  BLOCK_ID_MPI_Group_size,
  BLOCK_ID_MPI_Group_translate_ranks,
  BLOCK_ID_MPI_Group_union,
  BLOCK_ID_MPI_Group_compare,
  BLOCK_ID_MPI_Intercomm_create,
  BLOCK_ID_MPI_Intercomm_merge,
  BLOCK_ID_MPI_Keyval_free,
  BLOCK_ID_MPI_Keyval_create,			/* 060 */
  BLOCK_ID_MPI_Abort,
  BLOCK_ID_MPI_Error_class,
  BLOCK_ID_MPI_Errhandler_create,
  BLOCK_ID_MPI_Errhandler_free,
  BLOCK_ID_MPI_Errhandler_get,
  BLOCK_ID_MPI_Error_string,
  BLOCK_ID_MPI_Errhandler_set,
  BLOCK_ID_MPI_Finalize,
  BLOCK_ID_MPI_Get_processor_name,
  BLOCK_ID_MPI_Init,							/* 070 */
  BLOCK_ID_MPI_Initialized,
  BLOCK_ID_MPI_Wtick,
  BLOCK_ID_MPI_Wtime,
  BLOCK_ID_MPI_Address,
  BLOCK_ID_MPI_Bsend,
  BLOCK_ID_MPI_Bsend_init,
  BLOCK_ID_MPI_Buffer_attach,
  BLOCK_ID_MPI_Buffer_detach,
  BLOCK_ID_MPI_Cancel,
  BLOCK_ID_MPI_Request_free,			/* 080 */
  BLOCK_ID_MPI_Recv_init,
  BLOCK_ID_MPI_Send_init,
  BLOCK_ID_MPI_Get_count,
  BLOCK_ID_MPI_Get_elements,
  BLOCK_ID_MPI_Ibsend,
  BLOCK_ID_MPI_Iprobe,
  BLOCK_ID_MPI_Irecv,
  BLOCK_ID_MPI_Irsend,
  BLOCK_ID_MPI_Isend,
  BLOCK_ID_MPI_Issend,						/* 090 */
  BLOCK_ID_MPI_Pack,
  BLOCK_ID_MPI_Pack_size,
  BLOCK_ID_MPI_Probe,
  BLOCK_ID_MPI_Recv,
  BLOCK_ID_MPI_Rsend,
  BLOCK_ID_MPI_Rsend_init,
  BLOCK_ID_MPI_Send,
  BLOCK_ID_MPI_Sendrecv,
  BLOCK_ID_MPI_Sendrecv_replace,
  BLOCK_ID_MPI_Ssend,							/* 100 */
  BLOCK_ID_MPI_Ssend_init,
  BLOCK_ID_MPI_Start,
  BLOCK_ID_MPI_Startall,
  BLOCK_ID_MPI_Test,
  BLOCK_ID_MPI_Testall,
  BLOCK_ID_MPI_Testany,
  BLOCK_ID_MPI_Test_cancelled,
  BLOCK_ID_MPI_Test_some,
  BLOCK_ID_MPI_Type_commit,
  BLOCK_ID_MPI_Type_contiguous,		/* 110 */
  BLOCK_ID_MPI_Type_extent,
  BLOCK_ID_MPI_Type_free,
  BLOCK_ID_MPI_Type_hindexed,
  BLOCK_ID_MPI_Type_hvector,
  BLOCK_ID_MPI_Type_indexed,
  BLOCK_ID_MPI_Type_lb,
  BLOCK_ID_MPI_Type_size,
  BLOCK_ID_MPI_Type_struct,
  BLOCK_ID_MPI_Type_ub,
  BLOCK_ID_MPI_Type_vector,				/* 120 */
  BLOCK_ID_MPI_Unpack,
  BLOCK_ID_MPI_Wait,
  BLOCK_ID_MPI_Waitall,
  BLOCK_ID_MPI_Waitany,
  BLOCK_ID_MPI_Waitsome,
  BLOCK_ID_MPI_Cart_coords,
  BLOCK_ID_MPI_Cart_create,
  BLOCK_ID_MPI_Cart_get,
  BLOCK_ID_MPI_Cart_map,
  BLOCK_ID_MPI_Cart_rank,					/* 130 */
  BLOCK_ID_MPI_Cart_shift,
  BLOCK_ID_MPI_Cart_sub,
  BLOCK_ID_MPI_Cartdim_get,
  BLOCK_ID_MPI_Dims_create,
  BLOCK_ID_MPI_Graph_get,
  BLOCK_ID_MPI_Graph_map,
  BLOCK_ID_MPI_Graph_create,
  BLOCK_ID_MPI_Graph_neighbors,
  BLOCK_ID_MPI_Graphdims_get,
  BLOCK_ID_MPI_Graph_neighbors_count,	/* 140 */
  BLOCK_ID_MPI_Topo_test,

  BLOCK_ID_TRACE_ON,
  BLOCK_ID_IO_Read,
  BLOCK_ID_IO_Write,
  BLOCK_ID_IO,

  BLOCK_ID_MPI_Win_create,
  BLOCK_ID_MPI_Win_free,
  BLOCK_ID_MPI_Put,
  BLOCK_ID_MPI_Get,
  BLOCK_ID_MPI_Accumulate,					/* 150 */
  BLOCK_ID_MPI_Win_fence,
  BLOCK_ID_MPI_Win_complete,
  BLOCK_ID_MPI_Win_start,
  BLOCK_ID_MPI_Win_post,
  BLOCK_ID_MPI_Win_wait,
  BLOCK_ID_MPI_Win_test,
  BLOCK_ID_MPI_Win_lock,
  BLOCK_ID_MPI_Win_unlock,

  BLOCK_ID_MPI_Init_thread,

  BLOCK_ID_LAPI_Init,								/* 160 */
  BLOCK_ID_LAPI_Term,
  BLOCK_ID_LAPI_Put,
  BLOCK_ID_LAPI_Get,
  BLOCK_ID_LAPI_Fence,
  BLOCK_ID_LAPI_Gfence,
  BLOCK_ID_LAPI_Address_init,
  BLOCK_ID_LAPI_Amsend,
  BLOCK_ID_LAPI_Rmw,
  BLOCK_ID_LAPI_Waitcntr,

  BLOCK_ID_MPI_File_open,						/* 170 */
  BLOCK_ID_MPI_File_close,
  BLOCK_ID_MPI_File_read,
  BLOCK_ID_MPI_File_read_all,
  BLOCK_ID_MPI_File_write,
  BLOCK_ID_MPI_File_write_all,
  BLOCK_ID_MPI_File_read_at,
  BLOCK_ID_MPI_File_read_at_all,
  BLOCK_ID_MPI_File_write_at,
  BLOCK_ID_MPI_File_write_at_all,
  BLOCK_ID_MPI_Comm_spawn,					/* 180 */
  BLOCK_ID_MPI_Comm_spawn_multiple,
  BLOCK_ID_MPI_Request_get_status,	/* 182 */

}DimBlock;

/* ==========================================================================
   ==== MPI Dimemas Collective Communications Identifiers
   ========================================================================== */

typedef enum
{
  GLOP_ID_NULL               = -1,
  GLOP_ID_MPI_Barrier        = 0,
  GLOP_ID_MPI_Bcast          = 1,
  GLOP_ID_MPI_Gather         = 2,
  GLOP_ID_MPI_Gatherv        = 3,
  GLOP_ID_MPI_Scatter        = 4,
  GLOP_ID_MPI_Scatterv       = 5,
  GLOP_ID_MPI_Allgather      = 6,
  GLOP_ID_MPI_Allgatherv     = 7,
  GLOP_ID_MPI_Alltoall       = 8,
  GLOP_ID_MPI_Alltoallv      = 9,
  GLOP_ID_MPI_Reduce         = 10,
  GLOP_ID_MPI_Allreduce      = 11,
  GLOP_ID_MPI_Reduce_scatter = 12,
  GLOP_ID_MPI_Scan           = 13,

  GLOP_ID_MPI_Ibarrier       = 14,
  GLOP_ID_MPI_Ibcast         = 15,
	GLOP_ID_MPI_Igather        = 16,
  GLOP_ID_MPI_Igatherv       = 17,
	GLOP_ID_MPI_Iscatter       = 18,
  GLOP_ID_MPI_Iscatterv      = 19,
  GLOP_ID_MPI_Iallgather     = 20,
  GLOP_ID_MPI_Iallgatherv    = 21, 
  GLOP_ID_MPI_Ialltoall      = 22,
  GLOP_ID_MPI_Ialltoallv     = 23,
  GLOP_ID_MPI_Ireduce        = 24,
  GLOP_ID_MPI_Iallreduce     = 25,
  GLOP_ID_MPI_Ireduce_scatter= 26,
  GLOP_ID_MPI_Iscan          = 27
  

}DimCollectiveOp;

/* ==========================================================================
   ==== MPI Event Labels
   ========================================================================== */

#define  MPIEND_LABEL                      "End"
#define  MPI_SEND_LABEL                    "MPI_Send"
#define  MPI_RECV_LABEL                    "MPI_Recv"
#define  MPI_ISEND_LABEL                   "MPI_Isend"
#define  MPI_IRECV_LABEL                   "MPI_Irecv"
#define  MPI_WAIT_LABEL                    "MPI_Wait"
#define  MPI_WAITALL_LABEL                 "MPI_Waitall"

#define  MPI_BCAST_LABEL                   "MPI_Bcast"
#define  MPI_BARRIER_LABEL                 "MPI_Barrier"
#define  MPI_REDUCE_LABEL                  "MPI_Reduce"
#define  MPI_ALLREDUCE_LABEL               "MPI_Allreduce"

#define  MPI_ALLTOALL_LABEL                "MPI_Alltoall"
#define  MPI_ALLTOALLV_LABEL               "MPI_Alltoallv"
#define  MPI_GATHER_LABEL                  "MPI_Gather"
#define  MPI_GATHERV_LABEL                 "MPI_Gatherv"
#define  MPI_SCATTER_LABEL                 "MPI_Scatter"
#define  MPI_SCATTERV_LABEL                "MPI_Scatterv"
#define  MPI_ALLGATHER_LABEL               "MPI_Allgather"
#define  MPI_ALLGATHERV_LABEL              "MPI_Allgatherv"
#define  MPI_SCAN_LABEL                    "MPI_Scan"

#define  MPI_IREDUCE_LABEL                 "MPI_Ireduce"                  
#define  MPI_IALLREDUCE_LABEL              "MPI_Iallreduce"                     
#define  MPI_IBARRIER_LABEL                "MPI_Ibarrier"                       
#define  MPI_IBCAST_LABEL                  "MPI_Ibcast"                         
#define  MPI_IALLTOALL_LABEL               "MPI_Ialltoall"                      
#define  MPI_IALLTOALLV_LABEL              "MPI_Ialltoallv"                     
#define  MPI_IALLGATHER_LABEL              "MPI_Iallgather"                     
#define  MPI_IALLGATHERV_LABEL             "MPI_Iallgatherv"                    
#define  MPI_IGATHER_LABEL                 "MPI_Igather"                        
#define  MPI_IGATHERV_LABEL                "MPI_Igatherv"                       
#define  MPI_ISCATTER_LABEL                "MPI_Iscatter"                       
#define  MPI_ISCATTERV_LABEL               "MPI_Iscatterv"                      
#define  MPI_IREDUCESCAT_LABEL             "MPI_Ireduce_scatter"                
#define  MPI_ISCAN_LABEL                   "MPI_Iscan" 

#define  MPI_INIT_LABEL                    "MPI_Init"
#define  MPI_FINALIZE_LABEL                "MPI_Finalize"
#define  MPI_BSEND_LABEL                   "MPI_Bsend"
#define  MPI_SSEND_LABEL                   "MPI_Ssend"
#define  MPI_RSEND_LABEL                   "MPI_Rsend"
#define  MPI_IBSEND_LABEL                  "MPI_Ibsend"
#define  MPI_ISSEND_LABEL                  "MPI_Issend"
#define  MPI_IRSEND_LABEL                  "MPI_Irsend"
#define  MPI_TEST_LABEL                    "MPI_Test"
#define  MPI_CANCEL_LABEL                  "MPI_Cancel"
#define  MPI_SENDRECV_LABEL                "MPI_Sendrecv"
#define  MPI_SENDRECV_REPLACE_LABEL        "MPI_Sendrecv_replace"
#define  MPI_CART_CREATE_LABEL             "MPI_Cart_create"
#define  MPI_CART_SHIFT_LABEL              "MPI_Cart_shift"
#define  MPI_CART_COORDS_LABEL             "MPI_Cart_coords"
#define  MPI_CART_GET_LABEL                "MPI_Cart_get"
#define  MPI_CART_MAP_LABEL                "MPI_Cart_map"
#define  MPI_CART_RANK_LABEL               "MPI_Cart_rank"
#define  MPI_CART_SUB_LABEL                "MPI_Cart_sub"
#define  MPI_CARTDIM_GET_LABEL             "MPI_Cartdim_get"
#define  MPI_DIMS_CREATE_LABEL             "MPI_Dims_create"
#define  MPI_GRAPH_GET_LABEL               "MPI_Graph_get"
#define  MPI_GRAPH_MAP_LABEL               "MPI_Graph_map"
#define  MPI_GRAPH_CREATE_LABEL            "MPI_Graph_create"
#define  MPI_GRAPH_NEIGHBORS_LABEL         "MPI_Graph_neighbors"
#define  MPI_GRAPHDIMS_GET_LABEL           "MPI_Graphdims_get"
#define  MPI_GRAPH_NEIGHBORS_COUNT_LABEL   "MPI_Graph_neighbors_count"
#define  MPI_WAITANY_LABEL                 "MPI_Waitany"
#define  MPI_TOPO_TEST_LABEL               "MPI_Topo_test"
#define  MPI_WAITSOME_LABEL                "MPI_Waitsome"
#define  MPI_PROBE_LABEL                   "MPI_Probe"
#define  MPI_IPROBE_LABEL                  "MPI_Iprobe"

#define  MPI_WIN_CREATE_LABEL              "MPI_Win_create"
#define  MPI_WIN_FREE_LABEL                "MPI_Win_free"
#define  MPI_PUT_LABEL                     "MPI_Put"
#define  MPI_GET_LABEL                     "MPI_Get"
#define  MPI_ACCUMULATE_LABEL              "MPI_Accumulate"
#define  MPI_WIN_FENCE_LABEL               "MPI_Win_fence"
#define  MPI_WIN_START_LABEL               "MPI_Win_complete"
#define  MPI_WIN_COMPLETE_LABEL            "MPI_Win_start"
#define  MPI_WIN_POST_LABEL                "MPI_Win_post"
#define  MPI_WIN_WAIT_LABEL                "MPI_Win_wait"
#define  MPI_WIN_TEST_LABEL                "MPI_Win_test"
#define  MPI_WIN_LOCK_LABEL                "MPI_Win_lock"
#define  MPI_WIN_UNLOCK_LABEL              "MPI_Win_unlock"

#define  MPI_PACK_LABEL                    "MPI_Pack"
#define  MPI_UNPACK_LABEL                  "MPI_Unpack"

#define  MPI_OP_CREATE_LABEL               "MPI_Op_create"
#define  MPI_OP_FREE_LABEL                 "MPI_Op_free"
#define  MPI_REDUCE_SCATTER_LABEL          "MPI_Reduce_scatter"

#define  MPI_ATTR_DELETE_LABEL             "MPI_Attr_delete"
#define  MPI_ATTR_GET_LABEL                "MPI_Attr_get"
#define  MPI_ATTR_PUT_LABEL                "MPI_Attr_put"

#define  MPI_COMM_RANK_LABEL               "MPI_Comm_rank"
#define  MPI_COMM_SIZE_LABEL               "MPI_Comm_size"
#define  MPI_COMM_CREATE_LABEL             "MPI_Comm_create"
#define  MPI_COMM_DUP_LABEL                "MPI_Comm_dup"
#define  MPI_COMM_SPLIT_LABEL              "MPI_Comm_split"
#define  MPI_COMM_SPAWN_LABEL              "MPI_Comm_spawn"                 
#define  MPI_COMM_SPAWN_MULTIPLE_LABEL     "MPI_Comm_spawn_multiple"
#define  MPI_COMM_GROUP_LABEL              "MPI_Comm_group"
#define  MPI_COMM_FREE_LABEL               "MPI_Comm_free"
#define  MPI_COMM_REMOTE_GROUP_LABEL       "MPI_Comm_remote_group"
#define  MPI_COMM_REMOTE_SIZE_LABEL        "MPI_Comm_remote_size"
#define  MPI_COMM_TEST_INTER_LABEL         "MPI_Comm_test_inter"
#define  MPI_COMM_COMPARE_LABEL            "MPI_Comm_compare"

#define  MPI_GROUP_DIFFERENCE_LABEL        "MPI_Group_difference"
#define  MPI_GROUP_EXCL_LABEL              "MPI_Group_excl"
#define  MPI_GROUP_FREE_LABEL              "MPI_Group_free"
#define  MPI_GROUP_INCL_LABEL              "MPI_Group_incl"
#define  MPI_GROUP_INTERSECTION_LABEL      "MPI_Group_intersection"
#define  MPI_GROUP_RANK_LABEL              "MPI_Group_rank"
#define  MPI_GROUP_RANGE_EXCL_LABEL        "MPI_Group_range_excl"
#define  MPI_GROUP_RANGE_INCL_LABEL        "MPI_Group_range_incl"
#define  MPI_GROUP_SIZE_LABEL              "MPI_Group_size"
#define  MPI_GROUP_TRANSLATE_RANKS_LABEL   "MPI_Group_translate_ranks"
#define  MPI_GROUP_UNION_LABEL             "MPI_Group_union"
#define  MPI_GROUP_COMPARE_LABEL           "MPI_Group_compare"

#define  MPI_INTERCOMM_CREATE_LABEL        "MPI_Intercomm_create"
#define  MPI_INTERCOMM_MERGE_LABEL         "MPI_Intercomm_merge"
#define  MPI_KEYVAL_FREE_LABEL             "MPI_Keyval_free"
#define  MPI_KEYVAL_CREATE_LABEL           "MPI_Keyval_create"
#define  MPI_ABORT_LABEL                   "MPI_Abort"
#define  MPI_ERROR_CLASS_LABEL             "MPI_Error_class"
#define  MPI_ERRHANDLER_CREATE_LABEL       "MPI_Errhandler_create"
#define  MPI_ERRHANDLER_FREE_LABEL         "MPI_Errhandler_free"
#define  MPI_ERRHANDLER_GET_LABEL          "MPI_Errhandler_get"
#define  MPI_ERROR_STRING_LABEL            "MPI_Error_string"
#define  MPI_ERRHANDLER_SET_LABEL          "MPI_Errhandler_set"
#define  MPI_GET_PROCESSOR_NAME_LABEL      "MPI_Get_processor_name"
#define  MPI_INITIALIZED_LABEL             "MPI_Initialized"
#define  MPI_WTICK_LABEL                   "MPI_Wtick"
#define  MPI_WTIME_LABEL                   "MPI_Wtime"
#define  MPI_ADDRESS_LABEL                 "MPI_Address"
#define  MPI_BSEND_INIT_LABEL              "MPI_Bsend_init"
#define  MPI_BUFFER_ATTACH_LABEL           "MPI_Buffer_attach"
#define  MPI_BUFFER_DETACH_LABEL           "MPI_Buffer_detach"
#define  MPI_REQUEST_FREE_LABEL            "MPI_Request_free"
#define  MPI_RECV_INIT_LABEL               "MPI_Recv_init"
#define  MPI_SEND_INIT_LABEL               "MPI_Send_init"
#define  MPI_GET_COUNT_LABEL               "MPI_Get_count"
#define  MPI_GET_ELEMENTS_LABEL            "MPI_Get_elements"
#define  MPI_PACK_SIZE_LABEL               "MPI_Pack_size"
#define  MPI_RSEND_INIT_LABEL              "MPI_Rsend_init"
#define  MPI_SSEND_INIT_LABEL              "MPI_Ssend_init"
#define  MPI_START_LABEL                   "MPI_Start"
#define  MPI_STARTALL_LABEL                "MPI_Startall"
#define  MPI_TESTALL_LABEL                 "MPI_Testall"
#define  MPI_TESTANY_LABEL                 "MPI_Testany"
#define  MPI_TEST_CANCELLED_LABEL          "MPI_Test_cancelled"
#define  MPI_TESTSOME_LABEL                "MPI_Testsome"
#define  MPI_TYPE_COMMIT_LABEL             "MPI_Type_commit"
#define  MPI_TYPE_CONTIGUOUS_LABEL         "MPI_Type_contiguous"
#define  MPI_TYPE_EXTENT_LABEL             "MPI_Type_extent"
#define  MPI_TYPE_FREE_LABEL               "MPI_Type_free"
#define  MPI_TYPE_HINDEXED_LABEL           "MPI_Type_hindexed"
#define  MPI_TYPE_HVECTOR_LABEL            "MPI_Type_hvector"
#define  MPI_TYPE_INDEXED_LABEL            "MPI_Type_indexed"
#define  MPI_TYPE_LB_LABEL                 "MPI_Type_lb"
#define  MPI_TYPE_SIZE_LABEL               "MPI_Type_size"
#define  MPI_TYPE_STRUCT_LABEL             "MPI_Type_struct"
#define  MPI_TYPE_UB_LABEL                 "MPI_Type_ub"
#define  MPI_TYPE_VECTOR_LABEL             "MPI_Type_vector"

#define  MPI_FILE_OPEN_LABEL               "MPI_File_open"
#define  MPI_FILE_CLOSE_LABEL              "MPI_File_close"                     
#define  MPI_FILE_READ_LABEL               "MPI_File_read"                      
#define  MPI_FILE_READ_ALL_LABEL           "MPI_File_read_all"                  
#define  MPI_FILE_WRITE_LABEL              "MPI_File_write"                     
#define  MPI_FILE_WRITE_ALL_LABEL          "MPI_File_write_all"                 
#define  MPI_FILE_READ_AT_LABEL            "MPI_File_read_at"                   
#define  MPI_FILE_READ_AT_ALL_LABEL        "MPI_File_read_at_all"               
#define  MPI_FILE_WRITE_AT_LABEL           "MPI_File_write_at"                  
#define  MPI_FILE_WRITE_AT_ALL_LABEL       "MPI_File_write_at_all"              
#define  MPI_REQUEST_GET_STATUS_LABEL      "MPI_Request_get_status"

/*
 * MPI 2
 */
#define  MPI_INIT_THREAD_LABEL             "MPI_Init_thread"

/* ==========================================================================
   ==== LAPI Event Labels
   ========================================================================== */

#define LAPI_INIT_LABEL                    "LAPI_Init"
#define LAPI_TERM_LABEL                    "LAPI_Term"
#define LAPI_PUT_LABEL                     "LAPI_Put"
#define LAPI_GET_LABEL                     "LAPI_Get"
#define LAPI_FENCE_LABEL                   "LAPI_Fence"
#define LAPI_GFENCE_LABEL                  "LAPI_GFence"
#define LAPI_ADDRESS_INIT_LABEL            "LAPI_Address_Init"
#define LAPI_AMSEND_LABEL                  "LAPI_Amsend"
#define LAPI_RMW_LABEL                     "LAPI_Rmw"
#define LAPI_WAITCNTR_LABEL                "LAPI_Waitcntr"

/* ---------------------------------------------------- Function Prototypes -*/
void MPIEventEncoding_EnableOperation( MPI_Event_Values Op );

void MPIEventEncoding_WriteEnabledOperations( FILE *fd );

DimBlock MPIEventEncoding_DimemasBlockId( MPI_Event_Values Op );

DimCollectiveOp MPIEventEncoding_GlobalOpId (DimBlock BlockId);

int MPIEventEncoding_Is_MPIBlock  (long64_t Type );
int MPIEventEncoding_Is_BlockBegin(long64_t Op );


int MPIEventEncoding_Is_UserBlock( long64_t Type );
long64_t MPIEventEncoding_UserBlockId( long64_t Type, long64_t Value );

int EventEncoding_Is_IO( long64_t Type );
long64_t EventEncoding_DimemasIO_Block( long64_t Type );

int EventEncoding_Is_Flushing( long64_t Type );

char* MPIEventEncoding_GetBlockLabel(MPI_Event_Values Op);

int ClusterEventEncoding_Is_ClusterBlock(long64_t type);
int ClusterEventEncoding_Is_BlockBegin  (long64_t Op);


DimBlock ClusterEventEncoding_DimemasBlockId( long64_t value);

/* CUDA EventEncoding calls	*/
int      CUDAEventEncoding_Is_CUDABlock  				(long64_t type);
int      CUDAEventEncoding_Is_BlockBegin 				(long64_t Op);
int			 CUDAEventEncoding_Is_CUDAComm	 				(long64_t tag);
int			 CUDAEventEncoding_Is_CUDATransferBlock (struct t_event_block event);
int			 CUDAEventEconding_Is_CUDAConfigCall		(struct t_event_block event);
int			 CUDAEventEconding_Is_CUDALaunch				(struct t_event_block event);
int			 CUDAEventEconding_Is_CUDASync					(struct t_event_block event);
/* CUDA EventEncoding calls	*/

/* OpenCL EventEncoding calls	*/
int			 OCLEventEncoding_Is_OCLBlock		 			(long64_t type);
int			 OCLEventEncoding_Is_BlockBegin	 			(long64_t Op);
int			 OCLEventEncoding_Is_OCLComm	 	 			(long64_t tag);
int			 OCLEventEncoding_Is_OCLSyncBlock 		(struct t_event_block event);
int			 OCLEventEncoding_Is_OCLTransferBlock (struct t_event_block event);
int			 OCLEventEncoding_Is_OCLSchedBlock 		(struct t_event_block event);
int			 OCLEventEncoding_Is_OCLSchedblock 		(long64_t type, long64_t value);
int			 OCLEventEncoding_Is_OCLKernelRunning (struct t_event_block event);
/* OpenCL EventEncoding calls	*/

#ifdef __cplusplus
}
#endif

#endif /* _EVENT_ENCODING_H */
