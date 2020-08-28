/*****************************************************************************\
 *                        ANALYSIS PERFORMANCE TOOLS                         *
 *                                                                           *
 *           Paraver to Dimemas trace translator (old and new format)        *
 *****************************************************************************
 *     ___        This tool is free software; you can redistribute it and/or *
 *    /  __         modify it under the terms of the GNU LGPL as published   *
 *   /  /  _____    by the Free Software Foundation; either version 2.12.1    *
 *  /  /  /     \   of the License, or (at your option) any later version.   *
 * (  (  ( B S C )                                                           *
 *  \  \  \_____/   This tool is distributed in hope that it will be         *
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

/* ---------------------------------------------------- Include Files -------*/
#include <Macros.h>
#include <CommonMacros.h>
#include <EventEncoding.h>
#include <errno.h>

/* ---------------------------------------------------- Constants -----------*/
#define TYPE_LABEL           "EVENT_TYPE"
#define VALUES_LABEL         "VALUES"

#define FALSE 0
#define TRUE  1

/* ---------------------------------------------------- Data Types ----------*/
typedef int Boolean;

/* ---------------------------------------------------- Data Types ----------*/
typedef struct
{
  MPIType    Type;
  MPI_Event_Values     Op;
  char       *Label;
  DimBlock   Block;
  Boolean    Enabled;
  
} MPI_Enable;

/* ---------------------------------------------------- Data Types ----------*/
typedef struct
{
  MPIType  Type;
  char     *Label;
  
} MPITypeInfo;

/* ---------------------------------------------------- Global Variables ----*/
#define NUM_MPITYPES  4
MPITypeInfo MPIType_Table[ NUM_MPITYPES ] = {

  { MPITYPE_PTOP,        MPITYPE_PTOP_LABEL },
  { MPITYPE_COLLECTIVE,  MPITYPE_COLLECTIVE_LABEL },
  { MPITYPE_OTHER,       MPITYPE_OTHER_LABEL },
  { MPITYPE_RMA,         MPITYPE_RMA_LABEL }/*,
  { MPITYPE_COMM,        MPITYPE_COMM_LABEL },
  { MPITYPE_GROUP,       MPITYPE_GROUP_LABEL },
  { MPITYPE_TOPOLOGIES,  MPITYPE_TOPOLOGIES_LABEL },
  { MPITYPE_TYPE,        MPITYPE_TYPE_LABEL }*/
};

MPI_Enable MPI_Table[ NUM_MPICALLS ] = {
/* 000 */
{ 0,
  MPIEND_VAL, MPIEND_LABEL,
  BLOCK_ID_NULL,
  FALSE },
/* 001 */
{ MPITYPE_PTOP,
  MPI_SEND_VAL, MPI_SEND_LABEL,
  BLOCK_ID_MPI_Send,
  FALSE },
/* 002 */
{ MPITYPE_PTOP,
  MPI_RECV_VAL, MPI_RECV_LABEL,
  BLOCK_ID_MPI_Recv,
  FALSE },
/* 003 */
{ MPITYPE_PTOP,
  MPI_ISEND_VAL, MPI_ISEND_LABEL,
  BLOCK_ID_MPI_Isend,
  FALSE },
/* 004 */
{ MPITYPE_PTOP,
  MPI_IRECV_VAL, MPI_IRECV_LABEL,
  BLOCK_ID_MPI_Irecv,
  FALSE },
/* 005 */
{ MPITYPE_PTOP,
  MPI_WAIT_VAL, MPI_WAIT_LABEL,
  BLOCK_ID_MPI_Wait,
  FALSE },
/* 006 */
{ MPITYPE_PTOP,
  MPI_WAITALL_VAL, MPI_WAITALL_LABEL,
  BLOCK_ID_MPI_Waitall,
  FALSE },
/* 007 */
{ MPITYPE_COLLECTIVE,
  MPI_BCAST_VAL, MPI_BCAST_LABEL,
  BLOCK_ID_MPI_Bcast,
  FALSE },
/* 008 */
{ MPITYPE_COLLECTIVE,
  MPI_BARRIER_VAL, MPI_BARRIER_LABEL,
  BLOCK_ID_MPI_Barrier,
  FALSE },
/* 009 */
{ MPITYPE_COLLECTIVE,
  MPI_REDUCE_VAL, MPI_REDUCE_LABEL,
  BLOCK_ID_MPI_Reduce,
  FALSE },
/* 010 */
{ MPITYPE_COLLECTIVE, MPI_ALLREDUCE_VAL,
  MPI_ALLREDUCE_LABEL,
  BLOCK_ID_MPI_Allreduce,
  FALSE },
/* 011 */
{ MPITYPE_COLLECTIVE,
  MPI_ALLTOALL_VAL, MPI_ALLTOALL_LABEL,
  BLOCK_ID_MPI_Alltoall,
  FALSE },
/* 012 */
{ MPITYPE_COLLECTIVE,
  MPI_ALLTOALLV_VAL, MPI_ALLTOALLV_LABEL,
  BLOCK_ID_MPI_Alltoallv,
  FALSE },
/* 013 */
{ MPITYPE_COLLECTIVE,
  MPI_GATHER_VAL, MPI_GATHER_LABEL,
  BLOCK_ID_MPI_Gather,
  FALSE },
/* 014 */
{ MPITYPE_COLLECTIVE,
  MPI_GATHERV_VAL, MPI_GATHERV_LABEL,
  BLOCK_ID_MPI_Gatherv,
  FALSE },
/* 015 */
{ MPITYPE_COLLECTIVE,
  MPI_SCATTER_VAL, MPI_SCATTER_LABEL,
  BLOCK_ID_MPI_Scatter,
  FALSE },
/* 016 */
{ MPITYPE_COLLECTIVE,
  MPI_SCATTERV_VAL, MPI_SCATTERV_LABEL,
  BLOCK_ID_MPI_Scatterv,
  FALSE },
/* 017 */
{ MPITYPE_COLLECTIVE,
  MPI_ALLGATHER_VAL, MPI_ALLGATHER_LABEL,
  BLOCK_ID_MPI_Allgather,
  FALSE },
/* 018 */
{ MPITYPE_COLLECTIVE,
  MPI_ALLGATHERV_VAL, MPI_ALLGATHERV_LABEL,
  BLOCK_ID_MPI_Allgatherv,
  FALSE },
/* 019 */
{ MPITYPE_COMM,
  MPI_COMM_RANK_VAL, MPI_COMM_RANK_LABEL,
  BLOCK_ID_MPI_Comm_rank,
  FALSE },
/* 020 */
{ MPITYPE_COMM,
  MPI_COMM_SIZE_VAL, MPI_COMM_SIZE_LABEL,
  BLOCK_ID_MPI_Comm_size,
  FALSE },
/* 021 */
{ MPITYPE_COMM,
  MPI_COMM_CREATE_VAL, MPI_COMM_CREATE_LABEL,
  BLOCK_ID_MPI_Comm_create,
  FALSE },
/* 022 */
{ MPITYPE_COMM,
  MPI_COMM_DUP_VAL, MPI_COMM_DUP_LABEL,
  BLOCK_ID_MPI_Comm_dup,
  FALSE },
/* 023 */
{ MPITYPE_COMM,
  MPI_COMM_SPLIT_VAL, MPI_COMM_SPLIT_LABEL,
  BLOCK_ID_MPI_Comm_split,
  FALSE },
/* 024 */
{ MPITYPE_COMM,
  MPI_COMM_GROUP_VAL, MPI_COMM_GROUP_LABEL,
  BLOCK_ID_MPI_Comm_group,
  FALSE },
/* 025 */
{ MPITYPE_COMM,
  MPI_COMM_FREE_VAL, MPI_COMM_FREE_LABEL,
  BLOCK_ID_MPI_Comm_free,
  FALSE },
/* 026 */
{ MPITYPE_COMM,
  MPI_COMM_REMOTE_GROUP_VAL, MPI_COMM_REMOTE_GROUP_LABEL,
  BLOCK_ID_MPI_Comm_remote_group,
  FALSE },
/* 027 */
{ MPITYPE_COMM,
  MPI_COMM_REMOTE_SIZE_VAL, MPI_COMM_REMOTE_SIZE_LABEL,
  BLOCK_ID_MPI_Comm_remote_size,
  FALSE },
/* 028 */
{ MPITYPE_COMM,
  MPI_COMM_TEST_INTER_VAL, MPI_COMM_TEST_INTER_LABEL,
  BLOCK_ID_MPI_Comm_test_inter,
  FALSE },
/* 029 */
{ MPITYPE_COMM,
  MPI_COMM_COMPARE_VAL, MPI_COMM_COMPARE_LABEL,
  BLOCK_ID_MPI_Comm_compare,
  FALSE },
/* 030 */
{ MPITYPE_COLLECTIVE,
  MPI_SCAN_VAL, MPI_SCAN_LABEL,
  BLOCK_ID_MPI_Scan,
  FALSE },
/* 031 */
{ MPITYPE_OTHER,
  MPI_INIT_VAL, MPI_INIT_LABEL,
  BLOCK_ID_MPI_Init,
  FALSE },
/* 032 */
{ MPITYPE_OTHER,
  MPI_FINALIZE_VAL, MPI_FINALIZE_LABEL,
  BLOCK_ID_MPI_Finalize,
  FALSE },
/* 033 */
{ MPITYPE_PTOP,
  MPI_BSEND_VAL, MPI_BSEND_LABEL,
  BLOCK_ID_MPI_Bsend,
  FALSE },
/* 034 */
{ MPITYPE_PTOP,
  MPI_SSEND_VAL, MPI_SSEND_LABEL,
  BLOCK_ID_MPI_Ssend,
  FALSE },
/* 035 */
{ MPITYPE_PTOP,
  MPI_RSEND_VAL, MPI_RSEND_LABEL,
  BLOCK_ID_MPI_Rsend,
  FALSE },
/* 036 */
{ MPITYPE_PTOP,
  MPI_IBSEND_VAL, MPI_IBSEND_LABEL,
  BLOCK_ID_MPI_Ibsend,
  FALSE },
/* 037 */
{ MPITYPE_PTOP,
  MPI_ISSEND_VAL, MPI_ISSEND_LABEL,
  BLOCK_ID_MPI_Issend,
  FALSE },
/* 038 */
{ MPITYPE_PTOP,
  MPI_IRSEND_VAL, MPI_IRSEND_LABEL,
  BLOCK_ID_MPI_Irsend,
  FALSE },
/* 039 */
{ MPITYPE_PTOP,
  MPI_TEST_VAL, MPI_TEST_LABEL,
  BLOCK_ID_MPI_Test,
  FALSE },
/* 040 */
{ MPITYPE_PTOP,
  MPI_CANCEL_VAL, MPI_CANCEL_LABEL,
  BLOCK_ID_MPI_Cancel,
  FALSE },
/* 041 */
{ MPITYPE_PTOP,
  MPI_SENDRECV_VAL, MPI_SENDRECV_LABEL,
  BLOCK_ID_MPI_Sendrecv,
  FALSE },
/* 042 */
{ MPITYPE_PTOP,
  MPI_SENDRECV_REPLACE_VAL, MPI_SENDRECV_REPLACE_LABEL,
  BLOCK_ID_MPI_Sendrecv_replace,
  FALSE },
/* 043 */
{ MPITYPE_TOPOLOGIES,
  MPI_CART_CREATE_VAL, MPI_CART_CREATE_LABEL,
  BLOCK_ID_MPI_Cart_create,
  FALSE },
/* 044 */
{ MPITYPE_TOPOLOGIES,
  MPI_CART_SHIFT_VAL, MPI_CART_SHIFT_LABEL,
  BLOCK_ID_MPI_Cart_shift,
  FALSE },
/* 045 */
{ MPITYPE_TOPOLOGIES,
  MPI_CART_COORDS_VAL, MPI_CART_COORDS_LABEL,
  BLOCK_ID_MPI_Cart_coords,
  FALSE },
/* 046 */
{ MPITYPE_TOPOLOGIES,
  MPI_CART_GET_VAL, MPI_CART_GET_LABEL,
  BLOCK_ID_MPI_Cart_get,
  FALSE },
/* 047 */
{ MPITYPE_TOPOLOGIES,
  MPI_CART_MAP_VAL, MPI_CART_MAP_LABEL,
  BLOCK_ID_MPI_Cart_map,
  FALSE },
/* 048 */
{ MPITYPE_TOPOLOGIES,
  MPI_CART_RANK_VAL, MPI_CART_RANK_LABEL,
  BLOCK_ID_MPI_Cart_rank,
  FALSE },
/* 049 */
{ MPITYPE_TOPOLOGIES,
  MPI_CART_SUB_VAL, MPI_CART_SUB_LABEL,
  BLOCK_ID_MPI_Cart_sub,
  FALSE },
/* 050 */
{ MPITYPE_TOPOLOGIES,
  MPI_CARTDIM_GET_VAL, MPI_CARTDIM_GET_LABEL,
  BLOCK_ID_MPI_Cartdim_get,
  FALSE },
/* 051 */
{ MPITYPE_TOPOLOGIES,
  MPI_DIMS_CREATE_VAL, MPI_DIMS_CREATE_LABEL,
  BLOCK_ID_MPI_Dims_create,
  FALSE },
/* 052 */
{ MPITYPE_TOPOLOGIES,
  MPI_GRAPH_GET_VAL, MPI_GRAPH_GET_LABEL,
  BLOCK_ID_MPI_Graph_get,
  FALSE },
/* 053 */
{ MPITYPE_TOPOLOGIES,
  MPI_GRAPH_MAP_VAL, MPI_GRAPH_MAP_LABEL,
  BLOCK_ID_MPI_Graph_map,
  FALSE },
/* 054 */
{ MPITYPE_TOPOLOGIES,
  MPI_GRAPH_CREATE_VAL, MPI_GRAPH_CREATE_LABEL,
  BLOCK_ID_MPI_Graph_create,
  FALSE },
/* 055 */
{ MPITYPE_TOPOLOGIES,
  MPI_GRAPH_NEIGHBORS_VAL, MPI_GRAPH_NEIGHBORS_LABEL,
  BLOCK_ID_MPI_Graph_neighbors,
  FALSE },
/* 056 */
{ MPITYPE_TOPOLOGIES,
  MPI_GRAPHDIMS_GET_VAL, MPI_GRAPHDIMS_GET_LABEL,
  BLOCK_ID_MPI_Graphdims_get,
  FALSE },
/* 057 */
{ MPITYPE_TOPOLOGIES,
  MPI_GRAPH_NEIGHBORS_COUNT_VAL, MPI_GRAPH_NEIGHBORS_COUNT_LABEL,
  BLOCK_ID_MPI_Graph_neighbors_count,
  FALSE },
/* 058 */
{ MPITYPE_TOPOLOGIES,
  MPI_TOPO_TEST_VAL, MPI_TOPO_TEST_LABEL,
  BLOCK_ID_MPI_Topo_test,
  FALSE },
/* 059 */
{ MPITYPE_PTOP,
  MPI_WAITANY_VAL, MPI_WAITANY_LABEL,
  BLOCK_ID_MPI_Waitany,
  FALSE },
/* 060 */
{ MPITYPE_PTOP,
  MPI_WAITSOME_VAL, MPI_WAITSOME_LABEL,
  BLOCK_ID_MPI_Waitsome,
  FALSE },
/* 061 */
{ MPITYPE_PTOP,
  MPI_PROBE_VAL, MPI_PROBE_LABEL,
  BLOCK_ID_MPI_Probe,
  FALSE },
/* 062 */
{ MPITYPE_PTOP,
  MPI_IPROBE_VAL, MPI_IPROBE_LABEL,
  BLOCK_ID_MPI_Iprobe,
  FALSE },
/*
 * RMA MPI calls (Remote Memory Access calls)
 */
/* 063 */
{ MPITYPE_RMA,
  MPI_WIN_CREATE_VAL, MPI_WIN_CREATE_LABEL,
  BLOCK_ID_MPI_Win_create,
  FALSE },
/* 064 */
{ MPITYPE_RMA,
  MPI_WIN_FREE_VAL, MPI_WIN_FREE_LABEL,
  BLOCK_ID_MPI_Win_free,
  FALSE },
/* 065 */
{ MPITYPE_RMA,
  MPI_PUT_VAL, MPI_PUT_LABEL,
  BLOCK_ID_MPI_Put,
  FALSE },
/* 066 */
{ MPITYPE_RMA,
  MPI_GET_VAL, MPI_GET_LABEL,
  BLOCK_ID_MPI_Get,
  FALSE },
/* 067 */
{ MPITYPE_RMA,
  MPI_ACCUMULATE_VAL, MPI_ACCUMULATE_LABEL,
  BLOCK_ID_MPI_Accumulate,
  FALSE },
/* 068 */
{ MPITYPE_RMA,
  MPI_WIN_FENCE_VAL, MPI_WIN_FENCE_LABEL,
  BLOCK_ID_MPI_Win_fence,
  FALSE },
/* 069 */
{ MPITYPE_RMA,
  MPI_WIN_START_VAL, MPI_WIN_START_LABEL,
  BLOCK_ID_MPI_Win_start,
  FALSE },
/* 070 */
{ MPITYPE_RMA,
  MPI_WIN_COMPLETE_VAL, MPI_WIN_COMPLETE_LABEL,
  BLOCK_ID_MPI_Win_complete,
  FALSE },
/* 071 */
{ MPITYPE_RMA,
  MPI_WIN_POST_VAL, MPI_WIN_POST_LABEL,
  BLOCK_ID_MPI_Win_post,
  FALSE },
/* 072 */
{ MPITYPE_RMA,
  MPI_WIN_WAIT_VAL, MPI_WIN_WAIT_LABEL,
  BLOCK_ID_MPI_Win_wait,
  FALSE },
/* 073 */
{ MPITYPE_RMA,
  MPI_WIN_TEST_VAL, MPI_WIN_TEST_LABEL,
  BLOCK_ID_MPI_Win_test,
  FALSE },
/* 074 */
{ MPITYPE_RMA,
  MPI_WIN_LOCK_VAL, MPI_WIN_LOCK_LABEL,
  BLOCK_ID_MPI_Win_lock,
  FALSE },
/* 075 */
{ MPITYPE_RMA,
  MPI_WIN_UNLOCK_VAL, MPI_WIN_UNLOCK_LABEL,
  BLOCK_ID_MPI_Win_unlock,
  FALSE }, 
/* 076 */
{ MPITYPE_OTHER,
  MPI_PACK_VAL, MPI_PACK_LABEL,
  BLOCK_ID_MPI_Pack,
  FALSE },
/* 077 */
{ MPITYPE_OTHER,
  MPI_UNPACK_VAL, MPI_UNPACK_LABEL,
  BLOCK_ID_MPI_Unpack,
  FALSE },
/* 078 */
{ MPITYPE_OTHER,
  MPI_OP_CREATE_VAL, MPI_OP_CREATE_LABEL,
  BLOCK_ID_MPI_Op_create,
  FALSE },
/* 079 */
{ MPITYPE_OTHER,
  MPI_OP_FREE_VAL, MPI_OP_FREE_LABEL,
  BLOCK_ID_MPI_Op_free,
  FALSE },
/* 080 */
{ MPITYPE_COLLECTIVE,
  MPI_REDUCE_SCATTER_VAL, MPI_REDUCE_SCATTER_LABEL,
  BLOCK_ID_MPI_Reduce_scatter,
  FALSE },
/* 081 */
{ MPITYPE_OTHER,
  MPI_ATTR_DELETE_VAL, MPI_ATTR_DELETE_LABEL,
  BLOCK_ID_MPI_Attr_delete,
  FALSE },
/* 082 */
{ MPITYPE_OTHER,
  MPI_ATTR_GET_VAL, MPI_ATTR_GET_LABEL,
  BLOCK_ID_MPI_Attr_get,
  FALSE },
/* 083 */
{ MPITYPE_OTHER,
  MPI_ATTR_PUT_VAL, MPI_ATTR_PUT_LABEL,
  BLOCK_ID_MPI_Attr_put,
  FALSE },
/* 084 */
{ MPITYPE_GROUP,
  MPI_GROUP_DIFFERENCE_VAL, MPI_GROUP_DIFFERENCE_LABEL,
  BLOCK_ID_MPI_Group_difference,
  FALSE },
/* 085 */
{ MPITYPE_GROUP,
  MPI_GROUP_EXCL_VAL, MPI_GROUP_EXCL_LABEL,
  BLOCK_ID_MPI_Group_excl,
  FALSE },
/* 086 */
{ MPITYPE_GROUP,
  MPI_GROUP_FREE_VAL, MPI_GROUP_FREE_LABEL,
  BLOCK_ID_MPI_Group_free,
  FALSE },
/* 087 */
{ MPITYPE_GROUP,
  MPI_GROUP_INCL_VAL, MPI_GROUP_INCL_LABEL,
  BLOCK_ID_MPI_Group_incl,
  FALSE },
/* 088 */
{ MPITYPE_GROUP,
  MPI_GROUP_INTERSECTION_VAL, MPI_GROUP_INTERSECTION_LABEL,
  BLOCK_ID_MPI_Group_intersection,
  FALSE },
/* 089 */
{ MPITYPE_GROUP,
  MPI_GROUP_RANK_VAL, MPI_GROUP_RANK_LABEL,
  BLOCK_ID_MPI_Group_rank,
  FALSE },
/* 090 */
{ MPITYPE_GROUP,
  MPI_GROUP_RANGE_EXCL_VAL, MPI_GROUP_RANGE_EXCL_LABEL,
  BLOCK_ID_MPI_Group_range_excl,
  FALSE },
/* 091 */
{ MPITYPE_GROUP,
  MPI_GROUP_RANGE_INCL_VAL, MPI_GROUP_RANGE_INCL_LABEL,
  BLOCK_ID_MPI_Group_range_incl,
  FALSE },
/* 092 */
{ MPITYPE_GROUP,
  MPI_GROUP_SIZE_VAL, MPI_GROUP_SIZE_LABEL,
  BLOCK_ID_MPI_Group_size,
  FALSE },
/* 093 */
{ MPITYPE_GROUP,
  MPI_GROUP_TRANSLATE_RANKS_VAL, MPI_GROUP_TRANSLATE_RANKS_LABEL,
  BLOCK_ID_MPI_Group_translate_ranks,
  FALSE },
/* 094 */
{ MPITYPE_GROUP,
  MPI_GROUP_UNION_VAL, MPI_GROUP_UNION_LABEL,
  BLOCK_ID_MPI_Group_union,
  FALSE },
/* 095 */
{ MPITYPE_GROUP,
  MPI_GROUP_COMPARE_VAL, MPI_GROUP_COMPARE_LABEL,
  BLOCK_ID_MPI_Group_compare,
  FALSE },
/* 096 */
{ MPITYPE_OTHER,
  MPI_INTERCOMM_CREATE_VAL, MPI_INTERCOMM_CREATE_LABEL, 
  BLOCK_ID_MPI_Intercomm_create,
  FALSE },
/* 097 */
{ MPITYPE_OTHER,
  MPI_INTERCOMM_MERGE_VAL, MPI_INTERCOMM_MERGE_LABEL, 
  BLOCK_ID_MPI_Intercomm_merge,
  FALSE },
/* 098 */
{ MPITYPE_OTHER,
  MPI_KEYVAL_FREE_VAL, MPI_KEYVAL_FREE_LABEL, 
  BLOCK_ID_MPI_Keyval_free,
  FALSE },
/* 099 */
{ MPITYPE_OTHER,
  MPI_KEYVAL_CREATE_VAL, MPI_KEYVAL_CREATE_LABEL, 
  BLOCK_ID_MPI_Keyval_create,
  FALSE },
/* 100 */
{ MPITYPE_OTHER,
  MPI_ABORT_VAL, MPI_ABORT_LABEL, 
  BLOCK_ID_MPI_Abort,
  FALSE },
/* 101 */
{ MPITYPE_OTHER,
  MPI_ERROR_CLASS_VAL, MPI_ERROR_CLASS_LABEL, 
  BLOCK_ID_MPI_Error_class,
  FALSE },
/* 102 */
{ MPITYPE_OTHER,
  MPI_ERRHANDLER_CREATE_VAL, MPI_ERRHANDLER_CREATE_LABEL, 
  BLOCK_ID_MPI_Errhandler_create,
  FALSE },
/* 103 */
{ MPITYPE_OTHER,
  MPI_ERRHANDLER_FREE_VAL, MPI_ERRHANDLER_FREE_LABEL, 
  BLOCK_ID_MPI_Errhandler_free,
  FALSE },
/* 104 */
{ MPITYPE_OTHER,
  MPI_ERRHANDLER_GET_VAL, MPI_ERRHANDLER_GET_LABEL, 
  BLOCK_ID_MPI_Errhandler_get,
  FALSE },
/* 105 */
{ MPITYPE_OTHER,
  MPI_ERROR_STRING_VAL, MPI_ERROR_STRING_LABEL, 
  BLOCK_ID_MPI_Error_string,
  FALSE },
/* 106 */
{ MPITYPE_OTHER,
  MPI_ERRHANDLER_SET_VAL, MPI_ERRHANDLER_SET_LABEL, 
  BLOCK_ID_MPI_Errhandler_set,
  FALSE },
/* 107 */
{ MPITYPE_OTHER,
  MPI_GET_PROCESSOR_NAME_VAL, MPI_GET_PROCESSOR_NAME_LABEL, 
  BLOCK_ID_MPI_Get_processor_name,
  FALSE },
/* 108 */
{ MPITYPE_OTHER,
  MPI_INITIALIZED_VAL, MPI_INITIALIZED_LABEL, 
  BLOCK_ID_MPI_Initialized,
  FALSE },
/* 109 */
{ MPITYPE_OTHER,
  MPI_WTICK_VAL, MPI_WTICK_LABEL,
  BLOCK_ID_MPI_Wtick,
  FALSE },
/* 110 */
{ MPITYPE_OTHER,
  MPI_WTIME_VAL, MPI_WTIME_LABEL,
  BLOCK_ID_MPI_Wtime,
  FALSE },
/* 111 */
{ MPITYPE_OTHER,
  MPI_ADDRESS_VAL, MPI_ADDRESS_LABEL, 
  BLOCK_ID_MPI_Address,
  FALSE },
/* 112 */
{ MPITYPE_OTHER,
  MPI_BSEND_INIT_VAL, MPI_BSEND_INIT_LABEL,
  BLOCK_ID_MPI_Bsend_init,
  FALSE },
/* 113 */
{ MPITYPE_OTHER,
  MPI_BUFFER_ATTACH_VAL, MPI_BUFFER_ATTACH_LABEL, 
  BLOCK_ID_MPI_Buffer_attach,
  FALSE },
/* 114 */
{ MPITYPE_OTHER,
  MPI_BUFFER_DETACH_VAL, MPI_BUFFER_DETACH_LABEL, 
  BLOCK_ID_MPI_Buffer_detach,
  FALSE },
/* 115 */
{ MPITYPE_OTHER,
  MPI_REQUEST_FREE_VAL, MPI_REQUEST_FREE_LABEL, 
  BLOCK_ID_MPI_Request_free,
  FALSE },
/* 116 */
{ MPITYPE_OTHER,
  MPI_RECV_INIT_VAL, MPI_RECV_INIT_LABEL, 
  BLOCK_ID_MPI_Recv_init,
  FALSE },
/* 117 */
{ MPITYPE_OTHER,
  MPI_SEND_INIT_VAL, MPI_SEND_INIT_LABEL, 
  BLOCK_ID_MPI_Send_init,
  FALSE },
/* 118 */
{ MPITYPE_OTHER,
  MPI_GET_COUNT_VAL, MPI_GET_COUNT_LABEL, 
  BLOCK_ID_MPI_Get_count,
  FALSE },
/* 119 */
{ MPITYPE_OTHER,
  MPI_GET_ELEMENTS_VAL, MPI_GET_ELEMENTS_LABEL, 
  BLOCK_ID_MPI_Get_elements,
  FALSE },
/* 120 */
{ MPITYPE_OTHER,
  MPI_PACK_SIZE_VAL, MPI_PACK_SIZE_LABEL, 
  BLOCK_ID_MPI_Pack_size,
  FALSE },
/* 121 */
{ MPITYPE_OTHER,
  MPI_RSEND_INIT_VAL, MPI_RSEND_INIT_LABEL, 
  BLOCK_ID_MPI_Rsend_init,
  FALSE },
/* 122 */
{ MPITYPE_OTHER,
  MPI_SSEND_INIT_VAL, MPI_SSEND_INIT_LABEL, 
  BLOCK_ID_MPI_Ssend_init,
  FALSE },
/* 123 */
{ MPITYPE_OTHER,
  MPI_START_VAL, MPI_START_LABEL, 
  BLOCK_ID_MPI_Start,
  FALSE },
/* 124 */
{ MPITYPE_OTHER,
  MPI_STARTALL_VAL, MPI_STARTALL_LABEL, 
  BLOCK_ID_MPI_Startall,
  FALSE },
/* 125 */
{ MPITYPE_PTOP,
  MPI_TESTALL_VAL, MPI_TESTALL_LABEL, 
  BLOCK_ID_MPI_Testall,
  FALSE },
/* 126 */
{ MPITYPE_PTOP,
  MPI_TESTANY_VAL, MPI_TESTANY_LABEL, 
  BLOCK_ID_MPI_Testany,
  FALSE },
/* 127 */
{ MPITYPE_OTHER,
  MPI_TEST_CANCELLED_VAL, MPI_TEST_CANCELLED_LABEL, 
  BLOCK_ID_MPI_Test_cancelled,
  FALSE },
/* 128 */
{ MPITYPE_PTOP,
  MPI_TESTSOME_VAL, MPI_TESTSOME_LABEL,
  BLOCK_ID_MPI_Testsome,
  FALSE },
/* 129 */
{ MPITYPE_TYPE,
  MPI_TYPE_COMMIT_VAL, MPI_TYPE_COMMIT_LABEL,
  BLOCK_ID_MPI_Type_commit,
  FALSE },
/* 130 */
{ MPITYPE_TYPE,
  MPI_TYPE_CONTIGUOUS_VAL, MPI_TYPE_CONTIGUOUS_LABEL,
  BLOCK_ID_MPI_Type_contiguous,
  FALSE },
/* 131 */
{ MPITYPE_TYPE,
  MPI_TYPE_EXTENT_VAL, MPI_TYPE_EXTENT_LABEL,
  BLOCK_ID_MPI_Type_extent,
  FALSE },
/* 132 */
{ MPITYPE_TYPE,
  MPI_TYPE_FREE_VAL, MPI_TYPE_FREE_LABEL,
  BLOCK_ID_MPI_Type_free,
  FALSE },
/* 133 */
{ MPITYPE_TYPE,
  MPI_TYPE_HINDEXED_VAL, MPI_TYPE_HINDEXED_LABEL,
  BLOCK_ID_MPI_Type_hindexed,
  FALSE },
/* 134 */
{ MPITYPE_TYPE,
  MPI_TYPE_HVECTOR_VAL, MPI_TYPE_HVECTOR_LABEL,
  BLOCK_ID_MPI_Type_hvector,
  FALSE },
/* 135 */
{ MPITYPE_TYPE,
  MPI_TYPE_INDEXED_VAL, MPI_TYPE_INDEXED_LABEL,
  BLOCK_ID_MPI_Type_indexed,
  FALSE },
/* 136 */
{ MPITYPE_TYPE,
  MPI_TYPE_LB_VAL, MPI_TYPE_LB_LABEL,
  BLOCK_ID_MPI_Type_lb,
  FALSE },
/* 137 */
{ MPITYPE_TYPE,
  MPI_TYPE_SIZE_VAL, MPI_TYPE_SIZE_LABEL,
  BLOCK_ID_MPI_Type_size,
  FALSE },
/* 138 */
{ MPITYPE_TYPE,
  MPI_TYPE_STRUCT_VAL, MPI_TYPE_STRUCT_LABEL,
  BLOCK_ID_MPI_Type_struct,
  FALSE },
/* 139 */
{ MPITYPE_TYPE,
  MPI_TYPE_UB_VAL, MPI_TYPE_UB_LABEL,
  BLOCK_ID_MPI_Type_ub,
  FALSE },
/* 140 */
{ MPITYPE_TYPE,
  MPI_TYPE_VECTOR_VAL, MPI_TYPE_VECTOR_LABEL,
  BLOCK_ID_MPI_Type_vector,
  FALSE },
/* 141 */
{ MPITYPE_IO,
  MPI_FILE_OPEN_VAL, MPI_FILE_OPEN_LABEL,
  BLOCK_ID_MPI_File_open,
  FALSE },
/* 142 */
{ MPITYPE_IO,
  MPI_FILE_CLOSE_VAL, MPI_FILE_CLOSE_LABEL,
  BLOCK_ID_MPI_File_close,
  FALSE },
/* 143 */
{ MPITYPE_IO,
  MPI_FILE_READ_VAL, MPI_FILE_READ_LABEL,
  BLOCK_ID_MPI_File_read,
  FALSE },
/* 144 */
{ MPITYPE_IO,
  MPI_FILE_READ_ALL_VAL, MPI_FILE_READ_ALL_LABEL,
  BLOCK_ID_MPI_File_read_all,
  FALSE },
/* 145 */
{ MPITYPE_IO,
  MPI_FILE_WRITE_VAL, MPI_FILE_WRITE_LABEL,
  BLOCK_ID_MPI_File_write,
  FALSE },
/* 146 */
{ MPITYPE_IO,
  MPI_FILE_WRITE_ALL_VAL, MPI_FILE_WRITE_ALL_LABEL,
  BLOCK_ID_MPI_File_write_all,
  FALSE },
/* 147 */
{ MPITYPE_IO,
  MPI_FILE_READ_AT_VAL, MPI_FILE_READ_AT_LABEL,
  BLOCK_ID_MPI_File_read_at,
  FALSE },
/* 148 */
{ MPITYPE_IO,
  MPI_FILE_READ_AT_ALL_VAL, MPI_FILE_READ_AT_ALL_LABEL,
  BLOCK_ID_MPI_File_read_at_all,
  FALSE },
/* 149 */
{ MPITYPE_IO,
  MPI_FILE_WRITE_AT_VAL, MPI_FILE_WRITE_AT_LABEL,
  BLOCK_ID_MPI_File_write_at,
  FALSE },
/* 150 */
{ MPITYPE_IO,
  MPI_FILE_WRITE_AT_ALL_VAL, MPI_FILE_WRITE_AT_ALL_LABEL,
  BLOCK_ID_MPI_File_write_at_all,
  FALSE },
/* 151 */
{ MPITYPE_COMM,
  MPI_COMM_SPAWN_VAL, MPI_COMM_SPAWN_LABEL,
  BLOCK_ID_MPI_Comm_spawn,
  FALSE },
/* 152 */
{ MPITYPE_COMM,
  MPI_COMM_SPAWN_MULTIPLE_VAL, MPI_COMM_SPAWN_MULTIPLE_LABEL,
  BLOCK_ID_MPI_Comm_spawn_multiple,
  FALSE },
/* 153 */
{ MPITYPE_OTHER,
  MPI_REQUEST_GET_STATUS_VAL, MPI_REQUEST_GET_STATUS_LABEL,
  BLOCK_ID_MPI_Request_get_status,
  FALSE },
/* 154 */
{ MPITYPE_COLLECTIVE,
  MPI_IREDUCE_VAL, MPI_IREDUCE_LABEL,
  BLOCK_ID_MPI_Ireduce,
  FALSE },
/* 155 */
{ MPITYPE_COLLECTIVE,
  MPI_IALLREDUCE_VAL, MPI_IALLREDUCE_LABEL,
  BLOCK_ID_MPI_Iallreduce,
  FALSE },
/* 156 */
{ MPITYPE_COLLECTIVE,
  MPI_IBARRIER_VAL, MPI_IBARRIER_LABEL,
  BLOCK_ID_MPI_Ibarrier,
  FALSE },
/* 157 */
{ MPITYPE_COLLECTIVE,
  MPI_IBCAST_VAL, MPI_IBCAST_LABEL,
  BLOCK_ID_MPI_Ibcast,
  FALSE },
/* 158 */
{ MPITYPE_COLLECTIVE,
  MPI_IALLTOALL_VAL, MPI_IALLTOALL_LABEL,
  BLOCK_ID_MPI_Ialltoall,
  FALSE },
/* 159 */
{ MPITYPE_COLLECTIVE,
  MPI_IALLTOALLV_VAL, MPI_IALLTOALLV_LABEL,
  BLOCK_ID_MPI_Ialltoallv,
  FALSE },
/* 160 */
{ MPITYPE_COLLECTIVE,
  MPI_IALLGATHER_VAL, MPI_IALLGATHER_LABEL,
  BLOCK_ID_MPI_Iallgather,
  FALSE },
/* 161 */
{ MPITYPE_COLLECTIVE,
  MPI_IALLGATHERV_VAL, MPI_IALLGATHERV_LABEL,
  BLOCK_ID_MPI_Iallgatherv,
  FALSE },
/* 162 */
{ MPITYPE_COLLECTIVE,
  MPI_IGATHER_VAL, MPI_IGATHER_LABEL,
  BLOCK_ID_MPI_Igather,
  FALSE },
/* 163 */
{ MPITYPE_COLLECTIVE,
  MPI_IGATHERV_VAL, MPI_IGATHERV_LABEL,
  BLOCK_ID_MPI_Igatherv,
  FALSE },
/* 164 */
{ MPITYPE_COLLECTIVE,
  MPI_ISCATTER_VAL, MPI_ISCATTER_LABEL,
  BLOCK_ID_MPI_Iscatter,
  FALSE },
/* 165 */
{ MPITYPE_COLLECTIVE,
  MPI_ISCATTERV_VAL, MPI_ISCATTERV_LABEL,
  BLOCK_ID_MPI_Iscatterv,
  FALSE },
/* 166 */
{ MPITYPE_COLLECTIVE,
  MPI_IREDUCESCAT_VAL, MPI_IREDUCESCAT_LABEL,
  BLOCK_ID_MPI_Ireduce_scatter,
  FALSE },
/* 167 */
{ MPITYPE_COLLECTIVE,
  MPI_ISCAN_VAL, MPI_ISCAN_LABEL,
  BLOCK_ID_MPI_Iscan,
  FALSE },
/* 168 */
{ MPITYPE_COLLECTIVE,
  MPI_REDUCE_SCATTER_BLOCK_VAL, MPI_REDUCE_SCATTER_BLOCK_LABEL,
  BLOCK_ID_MPI_Reduce_scatter_block,
  FALSE},
/*169 */
{ MPITYPE_COLLECTIVE,
  MPI_IREDUCE_SCATTER_BLOCK_VAL, MPI_IREDUCE_SCATTER_BLOCK_LABEL,
  BLOCK_ID_MPI_Ireduce_scatter_block,
  FALSE},
/*170 */
{ MPITYPE_COLLECTIVE,
  MPI_ALLTOALLW_VAL, MPI_ALLTOALLW_LABEL,
  BLOCK_ID_MPI_Alltoallw,
  FALSE},
/*171 */
{ MPITYPE_COLLECTIVE,
  MPI_IALLTOALLW_VAL, MPI_IALLTOALLW_LABEL,
  BLOCK_ID_MPI_Ialltoallw,
  FALSE},
/*172 */
{ MPITYPE_RMA,
  MPI_GET_ACCUMULATE_VAL, MPI_GET_ACCUMULATE_LABEL,
  BLOCK_ID_MPI_Get_accumulate,
  FALSE},
/*173 */
{ MPITYPE_TOPOLOGIES,
  MPI_DIST_GRAPH_CREATE_VAL, MPI_DIST_GRAPH_CREATE_LABEL,
  BLOCK_ID_MPI_Dist_graph_create,
  FALSE},
/*174 */
{ MPITYPE_COLLECTIVE,
  MPI_NEIGHBOR_ALLGATHER_VAL, MPI_NEIGHBOR_ALLGATHER_LABEL,
  BLOCK_ID_MPI_Neighbor_allgather,
  FALSE},
/*175 */
{ MPITYPE_COLLECTIVE,
  MPI_INEIGHBOR_ALLGATHER_VAL, MPI_INEIGHBOR_ALLGATHER_LABEL,
  BLOCK_ID_MPI_Ineighbor_allgather,
  FALSE},
/*176*/
{ MPITYPE_COLLECTIVE,
  MPI_NEIGHBOR_ALLGATHERV_VAL, MPI_NEIGHBOR_ALLGATHERV_LABEL,
  BLOCK_ID_MPI_Neighbor_allgatherv,
  FALSE},
/* 177*/
{ MPITYPE_COLLECTIVE, 
  MPI_INEIGHBOR_ALLGATHERV_VAL,MPI_INEIGHBOR_ALLGATHERV_LABEL, 
  BLOCK_ID_MPI_Ineighbor_allgatherv,
  FALSE},
/* 178*/
{ MPITYPE_COLLECTIVE,
  MPI_NEIGHBOR_ALLTOALL_VAL, MPI_NEIGHBOR_ALLTOALL_LABEL,
  BLOCK_ID_MPI_Neighbor_alltoall,
  FALSE},
/* 179*/
{ MPITYPE_COLLECTIVE,
  MPI_INEIGHBOR_ALLTOALL_VAL,MPI_NEIGHBOR_ALLTOALL_LABEL,
  BLOCK_ID_MPI_Ineighbor_alltoall,
  FALSE},
/* 180*/
{ MPITYPE_COLLECTIVE, 
  MPI_NEIGHBOR_ALLTOALLV_VAL, MPI_NEIGHBOR_ALLTOALLV_LABEL,
  BLOCK_ID_MPI_Neighbor_alltoallv,
  FALSE}, 
/* 181 */
{ MPITYPE_COLLECTIVE,
  MPI_INEIGHBOR_ALLTOALLV_VAL, MPI_INEIGHBOR_ALLTOALLV_LABEL, 
  BLOCK_ID_MPI_Ineighbor_alltoallv,
  FALSE},
/* 182*/
{ MPITYPE_COLLECTIVE, 
  MPI_NEIGHBOR_ALLTOALLW_VAL, MPI_NEIGHBOR_ALLTOALLW_LABEL, 
  BLOCK_ID_MPI_Neighbor_alltoallw,
  FALSE},
/*183 */
{ MPITYPE_COLLECTIVE,
  MPI_INEIGHBOR_ALLTOALLW_VAL, MPI_INEIGHBOR_ALLTOALLW_LABEL, 
  BLOCK_ID_MPI_Ineighbor_alltoallw,
  FALSE},
/*184*/
{ MPITYPE_RMA,
  MPI_FETCH_AND_OP_VAL, MPI_FETCH_AND_OP_LABEL,
  BLOCK_ID_MPI_Fetch_and_op,
  FALSE},
/*185 */
{ MPITYPE_RMA, 
  MPI_COMPARE_AND_SWAP_VAL, MPI_COMPARE_AND_SWAP_LABEL, 
  BLOCK_ID_MPI_Compare_and_swap,
  FALSE}, 
/* 186 */
{ MPITYPE_RMA, 
  MPI_WIN_FLUSH_VAL, MPI_WIN_FLUSH_LABEL, 
  BLOCK_ID_MPI_Win_flush,
  FALSE},
/*187 */
{ MPITYPE_RMA,
  MPI_WIN_FLUSH_ALL_VAL, MPI_WIN_FLUSH_ALL_LABEL, 
  BLOCK_ID_MPI_Win_flush_all,
  FALSE},
/*188 */
{ MPITYPE_RMA, 
  MPI_WIN_FLUSH_LOCAL_VAL, MPI_WIN_FLUSH_LOCAL_LABEL, 
  BLOCK_ID_MPI_Win_flush_local,
  FALSE},
/*189 */
{ MPITYPE_RMA, 
  MPI_WIN_FLUSH_LOCAL_ALL_VAL, MPI_WIN_FLUSH_LOCAL_ALL_LABEL, 
  BLOCK_ID_MPI_Win_flush_local_all,
  FALSE},
/*190 */
{ MPITYPE_PTOP,
  MPI_MPROBE_VAL, MPI_MPROBE_LABEL,
  BLOCK_ID_MPI_Mprobe,
  FALSE },
/*191 */
{ MPITYPE_PTOP,
  MPI_IMPROBE_VAL, MPI_IMPROBE_LABEL,
  BLOCK_ID_MPI_Improbe,
  FALSE },
 /*192*/
{ MPITYPE_PTOP,
  MPI_MRECV_VAL, MPI_MRECV_LABEL,
  BLOCK_ID_MPI_Mrecv,
  FALSE },
/*193 */
{ MPITYPE_PTOP,
  MPI_IMRECV_VAL, MPI_IMRECV_LABEL,
  BLOCK_ID_MPI_Imrecv,
  FALSE },
// In order to have the same Ids than extrae I've moved this
// MPI from 141 to here.
/*194*/
  { MPITYPE_OTHER,
  MPI_INIT_THREAD_VAL, MPI_INIT_THREAD_LABEL,
  BLOCK_ID_MPI_Init_thread,
  FALSE }
};

#define BLOCKID_TOGLOBALOPID_VALUES 33
DimCollectiveOp BlockId2GlobalOpId[BLOCKID_TOGLOBALOPID_VALUES] =
{
/* -1 */  GLOP_ID_NULL,
/* 00 */  GLOP_ID_MPI_Barrier,
/* 01 */  GLOP_ID_MPI_Bcast,
/* 02 */  GLOP_ID_MPI_Gather,
/* 03 */  GLOP_ID_MPI_Gatherv,
/* 04 */  GLOP_ID_MPI_Scatter,
/* 05 */  GLOP_ID_MPI_Scatterv,
/* 06 */  GLOP_ID_MPI_Allgather,
/* 07 */  GLOP_ID_MPI_Allgatherv,
/* 08 */  GLOP_ID_MPI_Alltoall,
/* 09 */  GLOP_ID_MPI_Alltoallv,
/* 10 */  GLOP_ID_MPI_Reduce,
/* 11 */  GLOP_ID_MPI_Allreduce,
/* 12 */  GLOP_ID_MPI_Reduce_scatter,
/* 13 */  GLOP_ID_MPI_Reduce_scatter_block,
/* 14 */  GLOP_ID_MPI_Scan,
/* 15 */  GLOP_ID_MPI_Alltoallw,
/* 16 */  GLOP_ID_MPI_Ibarrier,
/* 17 */  GLOP_ID_MPI_Ibcast,
/* 18 */  GLOP_ID_MPI_Igather,
/* 19 */  GLOP_ID_MPI_Igatherv,
/* 20 */  GLOP_ID_MPI_Iscatter,
/* 21 */  GLOP_ID_MPI_Iscatterv,	
/* 22 */  GLOP_ID_MPI_Iallgather,
/* 23 */  GLOP_ID_MPI_Iallgatherv,
/* 24 */  GLOP_ID_MPI_Ialltoall,
/* 25 */  GLOP_ID_MPI_Ialltoallv,
/* 26 */  GLOP_ID_MPI_Ialltoallw,
/* 27 */  GLOP_ID_MPI_Ireduce,
/* 28 */  GLOP_ID_MPI_Iallreduce,
/* 29 */  GLOP_ID_MPI_Ireduce_scatter,
/* 30 */  GLOP_ID_MPI_Ireduce_scatter_block,
/* 31 */  GLOP_ID_MPI_Iscan
};



/******************************************************************************
 **      Function name : MPIEventEncoding_EnableOperation
 **      
 **      Description : 
 ******************************************************************************/

void MPIEventEncoding_EnableOperation( MPI_Event_Values Op )
{
  /* 
   * if the table typedef enum{...}MPI_Event_Values; from the EventEncoding.h is 
   * changed then we must have to made same changes on MPI_Table.
  */
  ASSERT( Op < NUM_MPICALLS );
  ASSERT( MPI_Table[ Op ].Op == Op );

  MPI_Table[ Op ].Enabled = TRUE;
}

/******************************************************************************
 **      Function name : MPIEventEncoding_DimemasBlockId
 **      
 **      Description : 
 ******************************************************************************/

DimBlock MPIEventEncoding_DimemasBlockId( MPI_Event_Values Op )
{
  /* 
   * if the table typedef enum{...}MPI_Event_Values; from the EventEncoding.h is 
   * changed then we must have to made same changes on MPI_Table.
  */
  ASSERT( Op < NUM_MPICALLS );
  ASSERT( MPI_Table[ Op ].Op == Op );
  
  return( MPI_Table[ Op ].Block );
}

/******************************************************************************
 **      Function name : MPIEventEncoding_GlobalOpId
 **      
 **      Description : 
 ******************************************************************************/

DimCollectiveOp MPIEventEncoding_GlobalOpId (DimBlock BlockId)
{
    ASSERT (BlockId < NUM_MPICALLS);
    int GlobalOpId=BlockId2GlobalOpId[BlockId]; 
    
    return GlobalOpId;
}

/******************************************************************************
 **      Function name : MPIEventEncoding_Is_MPIBlock
 **      
 **      Description : 
 ******************************************************************************/

int MPIEventEncoding_Is_MPIBlock( long64_t Type )
{
  int ii;
  
  for (ii= 0; ii< NUM_MPITYPES; ii++)
  {
    if (Type == (long64_t) MPIType_Table[ ii ].Type)
      return( TRUE );
  }
  
  return( FALSE );
}

/******************************************************************************
 **      Function name : MPIEventEncoding_Is_UserBlock
 **      
 **      Description : 
 ******************************************************************************/

int MPIEventEncoding_Is_UserBlock( long64_t Type )
{
  if (Type == (long64_t) USER_FUNCTION || Type == (long64_t) USER_CALL ||
      Type == (long64_t) USER_BLOCK)
    return( TRUE );
  
  return( FALSE );
}

/******************************************************************************
 **      Function name : EventEncoding_Is_IO
 **      
 **      Description : 
 ******************************************************************************/

int EventEncoding_Is_IO( long64_t Type )
{
  if (Type == (long64_t) IO_READ_EV || Type == (long64_t) IO_WRITE_EV ||
      Type == (long64_t) IO_EV)
    return( TRUE );
  
  return( FALSE );
}


long64_t EventEncoding_DimemasIO_Block( long64_t Type )
{
  if (Type == (long64_t) IO_READ_EV)
    return( BLOCK_ID_IO_Read );

  if (Type == (long64_t) IO_WRITE_EV)
    return( BLOCK_ID_IO_Write );

  if (Type == (long64_t) IO_EV)
    return( BLOCK_ID_IO );
  
  abort( );
}

/******************************************************************************
 **      Function name : EventEncoding_Is_Flushing
 **      
 **      Description : 
 ******************************************************************************/

int EventEncoding_Is_Flushing( long64_t Type )
{
  if (Type == (long64_t) IO_READ_EV || Type == (long64_t) IO_WRITE_EV ||
      Type == (long64_t) IO_EV)
    return( TRUE );
  
  return( FALSE );
}

/******************************************************************************
 **      Function name : MPIEventEncoding_UserBlockId
 **      
 **      Description : 
 ******************************************************************************/

long64_t MPIEventEncoding_UserBlockId( long64_t Type, long64_t Value )
{
  if (Type == (long64_t) USER_FUNCTION) 
      return( Value + BASE_USERFUNCTION );
  if (Type == (long64_t) USER_CALL)     
      return( Value + BASE_USERCALL );
  if (Type == (long64_t) USER_BLOCK)    
      return( Value + BASE_USERBLOCK );
  if (Type == (long64_t) CLUSTER_ID_EV) 
      return ( Value + BASE_CLUSTER_BLOCK );

  assert("MPIEventEncoding_UserBlockId: Invalid Type");
}

/******************************************************************************
 **      Function name : MPIEventEncoding_Is_BlockBegin
 **      
 **      Description : 
 ******************************************************************************/

int MPIEventEncoding_Is_BlockBegin( long64_t Op )
{
  return( (Op == (long64_t) MPIEND_VAL) ? FALSE : TRUE );
}

/******************************************************************************
 **      Function name : MPIEventEncoding_WriteEnabledSoftcounters
 **      Author : HSG
 **      Description : Genera informacio al PCF sobre rutines que poden
 **                    donar valors sobre software counters 
 ******************************************************************************/

static void MPIEventEncoding_WriteEnabledSoftcounters ( FILE *fd )
{
  ASSERT (fd != NULL);

  /* MPI_Test i MPI_Testsome generen un software counter comu! */
  if (MPI_Table[MPI_TEST_VAL].Enabled || MPI_Table[MPI_TESTSOME_VAL].Enabled)
  {
    fprintf ( fd, "%s\n", TYPE_LABEL );
    fprintf ( fd, "%d  %d   %s\n", 0, MPITYPE_TEST_SOFTCOUNTER, MPITYPE_TEST_SOFTCOUNTER_LABEL);
    fprintf ( fd, "\n\n" );
  }

  /* HSG, codi que hauria d'emprar-se si es fan soft counters per MPI_Iprobe */
}

/******************************************************************************
 **      Function name : MPIEventEncoding_WriteCollectiveInfo
 **      Author : HSG
 **      Description : Escriu informacio sobre rutines colectives.
 **                    
 *****************************************************************************/

static void MPIEventEncoding_WriteCollectiveInfo ( FILE *fd )
{
  ASSERT (fd != NULL);

  /* MPI_Test i MPI_Testsome generen un software counter comu! */
  /*
  if (MPI_Table[MPI_BCAST_VAL].Enabled ||
    MPI_Table[MPI_ALLGATHER_VAL].Enabled || 
    MPI_Table[MPI_ALLREDUCE_VAL].Enabled || 
    MPI_Table[MPI_BARRIER_VAL].Enabled || 
    MPI_Table[MPI_REDUCE_SCATTER_VAL].Enabled)
  */
  {
  fprintf ( fd, "%s\n", TYPE_LABEL );
  fprintf ( fd, "%d  %d   %s\n", 0, MPI_GLOBAL_OP_ROOT, "MPI Global OP is root?");
  fprintf ( fd, "%d  %d   %s\n", 0, MPI_GLOBAL_OP_COMM, "MPI Global OP communicator");
  fprintf ( fd, "%d  %d   %s\n", 0, MPI_GLOBAL_OP_SENDSIZE, "MPI Global OP send size");
  fprintf ( fd, "%d  %d   %s\n", 0, MPI_GLOBAL_OP_RECVSIZE, "MPI Global OP recv size");
  fprintf ( fd, "\n\n" );
  }

  /* HSG, codi que hauria d'emprar-se si es fan soft counters per MPI_Iprobe */
}

/******************************************************************************
 **      Function name : MPIEventEncoding_WriteEnabledOperations
 **      
 **      Description : 
 ******************************************************************************/

void MPIEventEncoding_WriteEnabledOperations( FILE *fd )
{
  int ii, jj;
  int cnt;
  
  MPIType Type;
  ASSERT( fd != NULL );

  for (ii= 0; ii< NUM_MPITYPES; ii++)
  {
    Type = MPIType_Table[ ii ].Type;
    
    /* Primer comptem si hi ha alguna operacio MPI del grup actual */
    cnt = 0;
    for (jj= 0; jj< NUM_MPICALLS; jj++)
    {
      if (Type == MPI_Table[ jj ].Type)
        if (MPI_Table[ jj ].Enabled)
          cnt++;
    }
    
    if (cnt)
    {
      fprintf( fd, "%s\n", TYPE_LABEL );
      fprintf( fd, "%d   %d    %s\n", 0, MPIType_Table[ ii ].Type,
                                         MPIType_Table[ ii ].Label );

      fprintf( fd, "%s\n", VALUES_LABEL );
      for (jj= 1; jj< NUM_MPICALLS; jj++)
      {
        if (Type == MPI_Table[ jj ].Type)
          if (MPI_Table[ jj ].Enabled)
            fprintf( fd, "%d   %s\n", MPI_Table[ jj ].Op,
                                      MPI_Table[ jj ].Label );
      }
      fprintf( fd, "%d   %s\n", MPI_Table[ 0 ].Op,
                                MPI_Table[ 0 ].Label );
      fprintf( fd, "\n\n" );
    }
  }
  MPIEventEncoding_WriteEnabledSoftcounters ( fd );
  MPIEventEncoding_WriteCollectiveInfo ( fd );
}

/******************************************************************************
 **      Function name : MPIEventEncoding_GetBlockLabel
 **      
 **      Description : 
 ******************************************************************************/

char* MPIEventEncoding_GetBlockLabel(MPI_Event_Values Op)
{
  ASSERT (Op < NUM_MPICALLS);
  
  return( MPI_Table[Op].Label );
}


/******************************************************************************
 **      Function name : ClusterEventEncoding_Is_ClusterBlock
 **
 **      Description :
 ******************************************************************************/

int ClusterEventEncoding_Is_ClusterBlock( long64_t type )
{
  return( (type == (long64_t) CLUSTER_ID_EV) ? TRUE : FALSE );
}

/******************************************************************************
 **      Function name : ClusterEventEncoding_Is_ClusterBlock
 **
 **      Description :
 ******************************************************************************/
int ClusterEventEncoding_Is_BlockBegin( long64_t Op )
{
  return( (Op == (long64_t) CLUSTEREND_VAL) ? FALSE : TRUE );
}

/******************************************************************************
 **      Function name : ClusterEventEncoding_DimemasBlockId
 **
 **      Description :
 ******************************************************************************/

DimBlock ClusterEventEncoding_DimemasBlockId( long64_t value)
{
  return ( (DimBlock) (value + BASE_CLUSTER_BLOCK) );
}


/* ---------------------------------------------------- Data Types ----------*/
typedef struct
{
  int  			Type;
  char     *Label;

} CUDATypeInfo;

/******************************************************************************
 **      Function name : CUDAEventEncoding_Is_CUDABlock
 **
 **      Description :
 ******************************************************************************/
Boolean CUDAEventEncoding_Is_CUDABlock ( long64_t type )
{
	return( (type == (long64_t) CUDA_LIB_CALL_EV) ? TRUE: FALSE);
}

/******************************************************************************
 **      Function name : CUDAEventEncoding_Is_BlockBegin
 **
 **      Description :
 ******************************************************************************/
Boolean CUDAEventEncoding_Is_BlockBegin ( long64_t Op )
{
  return( (Op == (long64_t) CUDA_END_VAL) ? FALSE : TRUE );
}

/******************************************************************************
 **      Function name : CUDAEventEncoding_Is_CUDAComm
 **
 **      Description : Returns true if it's a CUDA communication
 ******************************************************************************/

Boolean CUDAEventEncoding_Is_CUDAComm	 (long64_t tag)
{
	if (tag == CUDA_TAG) return TRUE;
	return FALSE;
}

/******************************************************************************
 **      Function name : CUDAEventEconding_Is_CUDAConfigCall
 **
 **      Description : Returns true if it's a Cuda_MemCpy event
 ******************************************************************************/
Boolean CUDAEventEncoding_Is_CUDATransferBlock (struct t_event_block event)
{
	if (event.type == CUDA_LIB_CALL_EV && event.value == CUDA_MEMCPY_VAL)
		return TRUE;
	return FALSE;
}

/******************************************************************************
 **      Function name : CUDAEventEconding_Is_CUDAConfigCall
 **
 **      Description : Returns true if it's a CudaConfigCall event
 ******************************************************************************/
Boolean CUDAEventEconding_Is_CUDAConfigCall(struct t_event_block event)
{
	if (event.type == CUDA_LIB_CALL_EV && event.value == CUDA_CONFIGURECALL_VAL)
		return TRUE;
	return FALSE;
}

/******************************************************************************
 **      Function name : CUDAEventEconding_Is_CUDALaunch
 **
 **      Description : Returns true if it's a CudaLaunch event
 ******************************************************************************/
Boolean CUDAEventEconding_Is_CUDALaunch(struct t_event_block event)
{
	if (event.type == CUDA_LIB_CALL_EV && event.value == CUDA_LAUNCH_VAL)
		return TRUE;
	return FALSE;
}

/******************************************************************************
 **      Function name : CUDAEventEconding_Is_CUDASync
 **
 **      Description : Returns true if it's a CudaThreadSync or CudaStreamSync
 ******************************************************************************/
Boolean CUDAEventEconding_Is_CUDASync(struct t_event_block event)
{
	if (event.type == CUDA_LIB_CALL_EV &&
			(event.value == CUDA_THREADSYNCHRONIZE_VAL ||
			 event.value == CUDA_STREAMSYNCHRONIZE_VAL))
		return TRUE;
	return FALSE;
}

/* ---------------------------------------------------- Global Variables ----*/
#define NUM_OCLTYPES  2
CUDATypeInfo OCLType_Table[ NUM_OCLTYPES ] = {

  { OCL_HOST_CALL_EV,  				OCL_HOST_CALL_LABEL },
  { OCL_ACCELERATOR_CALL_EV,	OCL_ACCELERATOR_CALL_LABEL }/*,
  { OCL_KERNEL_NAME_EV,  			OCL_KERNEL_NAME_LABEL }*/
	/*{ OCL_SYNCH_STREAM_EV,			OCL_SYNCH_STREAM_LABEL }*/
};

/******************************************************************************
 **      Function name : OCLEventEncoding_Is_OCLBlock
 **
 **      Description : Returns true if is an OCL event
 ******************************************************************************/

Boolean OCLEventEncoding_Is_OCLBlock ( long64_t type )
{
	for (int i=0; i< NUM_OCLTYPES; i++)
	{
		if (type == (long64_t) OCLType_Table[ i ].Type)
			return( TRUE );
	}

	return( FALSE );
}

/******************************************************************************
 **      Function name : OCLEventEncoding_Is_BlockBegin
 **
 **      Description : Returns true if OCL event is a block begin
 ******************************************************************************/
Boolean OCLEventEncoding_Is_BlockBegin ( long64_t Op )
{
  return( (Op == (long64_t) OCL_OUTSIDE_VAL) ? FALSE : TRUE );
}

/******************************************************************************
 **      Function name : OCLEventEncoding_Is_OCLComm
 **
 **      Description : Returns true if it's a OCL communication
 ******************************************************************************/
Boolean OCLEventEncoding_Is_OCLComm	 (long64_t tag)
{
	if (tag == OCL_TAG) return TRUE;
	return FALSE;
}

/******************************************************************************
 **      Function name : OCLEventEncoding_Is_OCLSchedBlock
 **
 **      Description : Returns true if it's a OCL event that generates
 **      							 a Sched & Fork/Join CPU state.
 ******************************************************************************/
Boolean OCLEventEncoding_Is_OCLSchedBlock (struct t_event_block event)
{
	if (event.type == OCL_KERNEL_NAME_EV) return TRUE;
	if (event.type != OCL_HOST_CALL_EV) return FALSE;

	switch (event.value)
	{
		case OCL_CREATE_BUFF_VAL:
		case OCL_CREATE_COMMAND_QUEUE_VAL:
		case OCL_CREATE_CONTEXT_VAL:
		case OCL_CREATE_CONTEXT_FROM_TYPE_VAL:
		case OCL_CREATE_KERNELS_IN_PROGRAM_VAL:
		case OCL_CREATE_KERNEL_VAL:
		case OCL_SET_KERNEL_ARGS_VAL:
		case OCL_CREATE_PROGRAM_WITH_BINARY_VAL:
		case OCL_CREATE_PROGRAM_WITH_BUILTIN_KERNELS_VAL:
		case OCL_CREATE_PROGRAM_WITH_SOURCE_VAL:
		case OCL_CREATE_SUBBUFFER_VAL:
		case OCL_BUILD_PROGRAM_VAL:
		case OCL_RELEASE_COMMAND_QUEUE_VAL:
		case OCL_RELEASE_CONTEXT_VAL:
		case OCL_RELEASE_DEVICE_VAL:
		case OCL_RELEASE_EVENT_VAL:
		case OCL_RELEASE_KERNEL_VAL:
		case OCL_RELEASE_MEM_OBJ_VAL:
		case OCL_RELEASE_PROGRAM_VAL:
		case OCL_ENQUEUE_NDRANGE_KERNEL_VAL:
			return TRUE;
		default:
			return FALSE;
	}
}

/******************************************************************************
 **      Function name : OCLEventEncoding_Is_OCLSchedBlock
 **
 **      Description : Returns true if it's a OCL event that generates
 **      							 a Sched & Fork/Join CPU state.
 ******************************************************************************/
Boolean OCLEventEncoding_Is_OCLSchedblock (long64_t type, long64_t value)
{
	if (type != OCL_HOST_CALL_EV) return FALSE;

	switch (value)
	{
		case OCL_CREATE_BUFF_VAL:
		case OCL_CREATE_COMMAND_QUEUE_VAL:
		case OCL_CREATE_CONTEXT_VAL:
		case OCL_CREATE_CONTEXT_FROM_TYPE_VAL:
		case OCL_CREATE_KERNELS_IN_PROGRAM_VAL:
		case OCL_CREATE_KERNEL_VAL:
		case OCL_SET_KERNEL_ARGS_VAL:
		case OCL_CREATE_PROGRAM_WITH_BINARY_VAL:
		case OCL_CREATE_PROGRAM_WITH_BUILTIN_KERNELS_VAL:
		case OCL_CREATE_PROGRAM_WITH_SOURCE_VAL:
		case OCL_CREATE_SUBBUFFER_VAL:
		case OCL_BUILD_PROGRAM_VAL:
		case OCL_RELEASE_COMMAND_QUEUE_VAL:
		case OCL_RELEASE_CONTEXT_VAL:
		case OCL_RELEASE_DEVICE_VAL:
		case OCL_RELEASE_EVENT_VAL:
		case OCL_RELEASE_KERNEL_VAL:
		case OCL_RELEASE_MEM_OBJ_VAL:
		case OCL_RELEASE_PROGRAM_VAL:
			return TRUE;
		default:
			return FALSE;
	}
}

/******************************************************************************
 **      Function name : OCLEventEncoding_Is_OCLTransferBlock
 **
 **      Description : Returns true if it's a OCL memory transfer event
 ******************************************************************************/
Boolean OCLEventEncoding_Is_OCLTransferBlock (struct t_event_block event)
{
	if (event.type != OCL_HOST_CALL_EV && event.type != OCL_ACCELERATOR_CALL_EV)
		return FALSE;

	switch (event.value)
	{
		case OCL_ENQUEUE_WRITE_BUFF_RECT_VAL:
		case OCL_ENQUEUE_WRITE_BUFF_VAL:
		case OCL_ENQUEUE_READ_BUFF_RECT_VAL:
		case OCL_ENQUEUE_READ_BUFF_VAL:
			return TRUE;
		default:
			return FALSE;
	}
}

/******************************************************************************
 **      Function name : OCLEventEncoding_Is_OCLFinishBlock
 **
 **      Description : Returns true if it's a OCL synchronization event
 ******************************************************************************/
Boolean OCLEventEncoding_Is_OCLSyncBlock (struct t_event_block event)
{
	if (event.type == OCL_HOST_CALL_EV || event.type == OCL_ACCELERATOR_CALL_EV)
	{
		if (event.value == OCL_FINISH_VAL)
			return TRUE;
	}
	return FALSE;
}

/******************************************************************************
 **      Function name : OCLEventEncoding_Is_OCLKernelRunning
 **
 **      Description : Returns true if it's a OCL Kernel burst event
 ******************************************************************************/
Boolean OCLEventEncoding_Is_OCLKernelRunning (struct t_event_block event)
{
	if (event.type  == OCL_ACCELERATOR_CALL_EV &&
			event.value == OCL_ENQUEUE_NDRANGE_KERNEL_ACC_VAL)
		return TRUE;

	return FALSE;
}

#define NUM_OMPTYPES  14
CUDATypeInfo OMPType_Table[ NUM_OMPTYPES ] = {

    { OMP_CALL_EV,          		       OMP_CALL_LABEL },           
    { OMP_WORKSHARING_EV,                  OMP_WORKSHARING_LABEL },
    { OMP_BARRIER,                         OMP_BARRIER_LABEL },
    { OMP_WORK_EV,                         OMP_WORK_LABEL },
    { OMP_EXECUTED_PARALLEL_FXN,           OMP_EXECUTED_PARALLEL_FXN_LABEL },
    { OMP_PTHREAD_FXN,                     OMP_PTHREAD_FXN_LABEL },
    { OMP_EXE_TASK_FXN,                    OMP_EXE_TASK_FXN_LABEL },
    { OMP_INIT_TASK_FXN,                   OMP_INIT_TASK_FXN_LABEL },
    { OMP_SET_NUM_THREADS,                 OMP_SET_NUM_THREADS_LABEL },
    { OMP_GET_NUM_THREADS,                 OMP_GET_NUM_THREADS_LABEL },
    { OMP_EXE_PARALLEL_FXN_LINE_N_FILE,    OMP_EXE_PARALLEL_FXN_LINE_N_FILE_LABEL },
    { OMP_PTHREAD_FXN_LINE_N_FILE,         OMP_PTHREAD_FXN_LINE_N_FILE_LABEL },
    { OMP_EXE_TASK_FXN_LINE_N_FILE,        OMP_EXE_TASK_FXN_LINE_N_FILE_LABEL },
    { OMP_INIT_TASK_FXN_LINE_N_FILE,       OMP_INIT_TASK_FXN_LINE_N_FILE_LABEL }
};
/** 
 * To check either the event is of  OMP block or not
*/
Boolean OMPEventEncoding_Is_OMPBlock ( long64_t type )
{
    for(int i = 0; i < NUM_OMPTYPES; ++i)
    {
	    if(type == (long64_t) OMPType_Table[i].Type)    
            return (TRUE);  
    }
    return (FALSE);
}
/**
 * OMP blocking begining 
*/
Boolean OMPEventEncoding_Is_BlockBegin ( long64_t Op )
{
  return( (Op == (long64_t) OMP_END_VAL) ? FALSE : TRUE );
}
/**
 * OMP idle states
*/
Boolean OMPEventEncoding_Is_OMPIdle (long64_t Op)
{
	if (Op == OMP_EXECUTED_PARALLEL_FXN || Op == OMP_EXE_PARALLEL_FXN_LINE_N_FILE)
		return TRUE;
	return FALSE;
}
/**
 * OMP master running case
*/
Boolean OMPEventEncoding_Is_OMPMaster_Running (struct t_event_block event)
{
	if ((event.type == OMP_EXE_PARALLEL_FXN_LINE_N_FILE
                && event.value > OMP_END_VAL)
                || event.type == OMP_WORK_EV && event.value == OMP_END_VAL
                || event.type == OMP_BARRIER && event.value == OMP_END_VAL
                || event.type == OMP_CALL_EV && event.value == OMP_END_VAL)
		return TRUE;
	return FALSE;
}
/**
 * OMP worker running case
*/
Boolean OMPEventEncoding_Is_OMPWorker_Running (struct t_event_block event)
{
	if ((event.type == OMP_EXE_PARALLEL_FXN_LINE_N_FILE) 
                && event.value > OMP_END_VAL)
		return TRUE;
	return FALSE;
}
/**
 * OMP worker after barrier running case
*/
Boolean OMPEventEncoding_Is_OMPWorker_Afterbarrier_Running (struct t_event_block event)
{
    if(event.type == OMP_BARRIER && event.value == OMP_END_VAL)
		return TRUE;
	return FALSE;
}
Boolean OMPEventEncoding_Is_OMPTime (struct t_event_block event)
{
	if ((event.type == OMP_EXE_PARALLEL_FXN_LINE_N_FILE || event.type == OMP_EXECUTED_PARALLEL_FXN) 
                && event.value > OMP_END_VAL)
		return TRUE;
	return FALSE;
}
/**
 * OMP synchronization 
*/
Boolean OMPEventEncoding_Is_OMPSync (struct t_event_block event)
{
	if (event.type == OMP_BARRIER &&
			event.value == OMP_BEGIN_VAL)
		return TRUE;
	return FALSE;
}
/**
 * Scheduling and fork join 
*/
Boolean OMPEventEncoding_Is_OMPWork_Dist (struct t_event_block event)
{
    if(event.type == OMP_CALL_EV && event.value == REGION_OPEN)
		  return TRUE;
    return FALSE;
}
/**
 * Scheduling and fork join 
*/
Boolean OMPEventEncoding_Is_OMPSched (struct t_event_block event)
{
	if ((event.type == OMP_WORKSHARING_EV && (event.value == DO_WORKSHARE || event.value == SINGLE_WORKSHARE)) || 
            (event.type == OMP_WORK_EV && event.value == OMP_BEGIN_VAL) ||
            (event.type == OMP_SET_NUM_THREADS && event.value == OMP_BEGIN_VAL))
		return TRUE;
    return FALSE;
}

