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
#include <CommonMacros.h>
#include <EventEncoding.h>
#include <Macros.h>
#include <errno.h>

/* ---------------------------------------------------- Constants -----------*/
#define TYPE_LABEL   "EVENT_TYPE"
#define VALUES_LABEL "VALUES"

#define FALSE 0
#define TRUE  1

/* ---------------------------------------------------- Data Types ----------*/
typedef int Boolean;

/* ---------------------------------------------------- Data Types ----------*/
typedef struct
{
  MPIType Type;
  MPI_Event_Values Op;
  char *Label;
  DimBlock Block;
  DimCollectiveOp GlobalOpId;
  Boolean Enabled;
} MPI_Enable;

/* ---------------------------------------------------- Data Types ----------*/
typedef struct
{
  MPIType Type;
  char *Label;

} MPITypeInfo;

/* ---------------------------------------------------- Global Variables ----*/
#define NUM_MPITYPES 4
MPITypeInfo MPIType_Table[ NUM_MPITYPES ] = {

  { MPITYPE_PTOP, MPITYPE_PTOP_LABEL },
  { MPITYPE_COLLECTIVE, MPITYPE_COLLECTIVE_LABEL },
  { MPITYPE_OTHER, MPITYPE_OTHER_LABEL },
  { MPITYPE_RMA, MPITYPE_RMA_LABEL } /*,
{ MPITYPE_COMM,        MPITYPE_COMM_LABEL },
{ MPITYPE_GROUP,       MPITYPE_GROUP_LABEL },
{ MPITYPE_TOPOLOGIES,  MPITYPE_TOPOLOGIES_LABEL },
{ MPITYPE_TYPE,        MPITYPE_TYPE_LABEL }*/
};

MPI_Enable MPI_Table[ NUM_MPICALLS ] = {
  /* 000 */
  { 0, MPIEND_VAL, MPIEND_LABEL, BLOCK_ID_NULL, GLOP_ID_NULL, FALSE },
  /* 001 */
  { MPITYPE_PTOP, MPI_SEND_VAL, MPI_SEND_LABEL, BLOCK_ID_MPI_Send, GLOP_ID_NULL, FALSE },
  /* 002 */
  { MPITYPE_PTOP, MPI_RECV_VAL, MPI_RECV_LABEL, BLOCK_ID_MPI_Recv, GLOP_ID_NULL, FALSE },
  /* 003 */
  { MPITYPE_PTOP, MPI_ISEND_VAL, MPI_ISEND_LABEL, BLOCK_ID_MPI_Isend, GLOP_ID_NULL, FALSE },
  /* 004 */
  { MPITYPE_PTOP, MPI_IRECV_VAL, MPI_IRECV_LABEL, BLOCK_ID_MPI_Irecv, GLOP_ID_NULL, FALSE },
  /* 005 */
  { MPITYPE_PTOP, MPI_WAIT_VAL, MPI_WAIT_LABEL, BLOCK_ID_MPI_Wait, GLOP_ID_NULL, FALSE },
  /* 006 */
  { MPITYPE_PTOP, MPI_WAITALL_VAL, MPI_WAITALL_LABEL, BLOCK_ID_MPI_Waitall, GLOP_ID_NULL, FALSE },
  /* 007 */
  { MPITYPE_COLLECTIVE, MPI_BCAST_VAL, MPI_BCAST_LABEL, BLOCK_ID_MPI_Bcast, GLOP_ID_MPI_Bcast, FALSE },
  /* 008 */
  { MPITYPE_COLLECTIVE, MPI_BARRIER_VAL, MPI_BARRIER_LABEL, BLOCK_ID_MPI_Barrier, GLOP_ID_MPI_Barrier, FALSE },
  /* 009 */
  { MPITYPE_COLLECTIVE, MPI_REDUCE_VAL, MPI_REDUCE_LABEL, BLOCK_ID_MPI_Reduce, GLOP_ID_MPI_Reduce, FALSE },
  /* 010 */
  { MPITYPE_COLLECTIVE, MPI_ALLREDUCE_VAL, MPI_ALLREDUCE_LABEL, BLOCK_ID_MPI_Allreduce, GLOP_ID_MPI_Allreduce, FALSE },
  /* 011 */
  { MPITYPE_COLLECTIVE, MPI_ALLTOALL_VAL, MPI_ALLTOALL_LABEL, BLOCK_ID_MPI_Alltoall, GLOP_ID_MPI_Alltoall, FALSE },
  /* 012 */
  { MPITYPE_COLLECTIVE, MPI_ALLTOALLV_VAL, MPI_ALLTOALLV_LABEL, BLOCK_ID_MPI_Alltoallv, GLOP_ID_MPI_Alltoallv, FALSE },
  /* 013 */
  { MPITYPE_COLLECTIVE, MPI_GATHER_VAL, MPI_GATHER_LABEL, BLOCK_ID_MPI_Gather, GLOP_ID_MPI_Gather, FALSE },
  /* 014 */
  { MPITYPE_COLLECTIVE, MPI_GATHERV_VAL, MPI_GATHERV_LABEL, BLOCK_ID_MPI_Gatherv, GLOP_ID_MPI_Gatherv, FALSE },
  /* 015 */
  { MPITYPE_COLLECTIVE, MPI_SCATTER_VAL, MPI_SCATTER_LABEL, BLOCK_ID_MPI_Scatter, GLOP_ID_MPI_Scatter, FALSE },
  /* 016 */
  { MPITYPE_COLLECTIVE, MPI_SCATTERV_VAL, MPI_SCATTERV_LABEL, BLOCK_ID_MPI_Scatterv, GLOP_ID_MPI_Scatterv, FALSE },
  /* 017 */
  { MPITYPE_COLLECTIVE, MPI_ALLGATHER_VAL, MPI_ALLGATHER_LABEL, BLOCK_ID_MPI_Allgather, GLOP_ID_MPI_Iallgather, FALSE },
  /* 018 */
  { MPITYPE_COLLECTIVE, MPI_ALLGATHERV_VAL, MPI_ALLGATHERV_LABEL, BLOCK_ID_MPI_Allgatherv, GLOP_ID_MPI_Iallgatherv, FALSE },
  /* 019 */
  { MPITYPE_COMM, MPI_COMM_RANK_VAL, MPI_COMM_RANK_LABEL, BLOCK_ID_MPI_Comm_rank, GLOP_ID_NULL, FALSE },
  /* 020 */
  { MPITYPE_COMM, MPI_COMM_SIZE_VAL, MPI_COMM_SIZE_LABEL, BLOCK_ID_MPI_Comm_size, GLOP_ID_NULL, FALSE },
  /* 021 */
  { MPITYPE_COMM, MPI_COMM_CREATE_VAL, MPI_COMM_CREATE_LABEL, BLOCK_ID_MPI_Comm_create, GLOP_ID_NULL, FALSE },
  /* 022 */
  { MPITYPE_COMM, MPI_COMM_DUP_VAL, MPI_COMM_DUP_LABEL, BLOCK_ID_MPI_Comm_dup, GLOP_ID_NULL, FALSE },
  /* 023 */
  { MPITYPE_COMM, MPI_COMM_SPLIT_VAL, MPI_COMM_SPLIT_LABEL, BLOCK_ID_MPI_Comm_split, GLOP_ID_NULL, FALSE },
  /* 024 */
  { MPITYPE_COMM, MPI_COMM_GROUP_VAL, MPI_COMM_GROUP_LABEL, BLOCK_ID_MPI_Comm_group, GLOP_ID_NULL, FALSE },
  /* 025 */
  { MPITYPE_COMM, MPI_COMM_FREE_VAL, MPI_COMM_FREE_LABEL, BLOCK_ID_MPI_Comm_free, GLOP_ID_NULL, FALSE },
  /* 026 */
  { MPITYPE_COMM, MPI_COMM_REMOTE_GROUP_VAL, MPI_COMM_REMOTE_GROUP_LABEL, BLOCK_ID_MPI_Comm_remote_group, GLOP_ID_NULL, FALSE },
  /* 027 */
  { MPITYPE_COMM, MPI_COMM_REMOTE_SIZE_VAL, MPI_COMM_REMOTE_SIZE_LABEL, BLOCK_ID_MPI_Comm_remote_size, GLOP_ID_NULL, FALSE },
  /* 028 */
  { MPITYPE_COMM, MPI_COMM_TEST_INTER_VAL, MPI_COMM_TEST_INTER_LABEL, BLOCK_ID_MPI_Comm_test_inter, GLOP_ID_NULL, FALSE },
  /* 029 */
  { MPITYPE_COMM, MPI_COMM_COMPARE_VAL, MPI_COMM_COMPARE_LABEL, BLOCK_ID_MPI_Comm_compare, GLOP_ID_NULL, FALSE },
  /* 030 */
  { MPITYPE_COLLECTIVE, MPI_SCAN_VAL, MPI_SCAN_LABEL, BLOCK_ID_MPI_Scan, GLOP_ID_MPI_Scan, FALSE },
  /* 031 */
  { MPITYPE_OTHER, MPI_INIT_VAL, MPI_INIT_LABEL, BLOCK_ID_MPI_Init, GLOP_ID_NULL, FALSE },
  /* 032 */
  { MPITYPE_OTHER, MPI_FINALIZE_VAL, MPI_FINALIZE_LABEL, BLOCK_ID_MPI_Finalize, GLOP_ID_NULL, FALSE },
  /* 033 */
  { MPITYPE_PTOP, MPI_BSEND_VAL, MPI_BSEND_LABEL, BLOCK_ID_MPI_Bsend, GLOP_ID_NULL, FALSE },
  /* 034 */
  { MPITYPE_PTOP, MPI_SSEND_VAL, MPI_SSEND_LABEL, BLOCK_ID_MPI_Ssend, GLOP_ID_NULL, FALSE },
  /* 035 */
  { MPITYPE_PTOP, MPI_RSEND_VAL, MPI_RSEND_LABEL, BLOCK_ID_MPI_Rsend, GLOP_ID_NULL, FALSE },
  /* 036 */
  { MPITYPE_PTOP, MPI_IBSEND_VAL, MPI_IBSEND_LABEL, BLOCK_ID_MPI_Ibsend, GLOP_ID_NULL, FALSE },
  /* 037 */
  { MPITYPE_PTOP, MPI_ISSEND_VAL, MPI_ISSEND_LABEL, BLOCK_ID_MPI_Issend, GLOP_ID_NULL, FALSE },
  /* 038 */
  { MPITYPE_PTOP, MPI_IRSEND_VAL, MPI_IRSEND_LABEL, BLOCK_ID_MPI_Irsend, GLOP_ID_NULL, FALSE },
  /* 039 */
  { MPITYPE_PTOP, MPI_TEST_VAL, MPI_TEST_LABEL, BLOCK_ID_MPI_Test, GLOP_ID_NULL, FALSE },
  /* 040 */
  { MPITYPE_PTOP, MPI_CANCEL_VAL, MPI_CANCEL_LABEL, BLOCK_ID_MPI_Cancel, GLOP_ID_NULL, FALSE },
  /* 041 */
  { MPITYPE_PTOP, MPI_SENDRECV_VAL, MPI_SENDRECV_LABEL, BLOCK_ID_MPI_Sendrecv, GLOP_ID_NULL, FALSE },
  /* 042 */
  { MPITYPE_PTOP, MPI_SENDRECV_REPLACE_VAL, MPI_SENDRECV_REPLACE_LABEL, BLOCK_ID_MPI_Sendrecv_replace, GLOP_ID_NULL, FALSE },
  /* 043 */
  { MPITYPE_TOPOLOGIES, MPI_CART_CREATE_VAL, MPI_CART_CREATE_LABEL, BLOCK_ID_MPI_Cart_create, GLOP_ID_NULL, FALSE },
  /* 044 */
  { MPITYPE_TOPOLOGIES, MPI_CART_SHIFT_VAL, MPI_CART_SHIFT_LABEL, BLOCK_ID_MPI_Cart_shift, GLOP_ID_NULL, FALSE },
  /* 045 */
  { MPITYPE_TOPOLOGIES, MPI_CART_COORDS_VAL, MPI_CART_COORDS_LABEL, BLOCK_ID_MPI_Cart_coords, GLOP_ID_NULL, FALSE },
  /* 046 */
  { MPITYPE_TOPOLOGIES, MPI_CART_GET_VAL, MPI_CART_GET_LABEL, BLOCK_ID_MPI_Cart_get, GLOP_ID_NULL, FALSE },
  /* 047 */
  { MPITYPE_TOPOLOGIES, MPI_CART_MAP_VAL, MPI_CART_MAP_LABEL, BLOCK_ID_MPI_Cart_map, GLOP_ID_NULL, FALSE },
  /* 048 */
  { MPITYPE_TOPOLOGIES, MPI_CART_RANK_VAL, MPI_CART_RANK_LABEL, BLOCK_ID_MPI_Cart_rank, GLOP_ID_NULL, FALSE },
  /* 049 */
  { MPITYPE_TOPOLOGIES, MPI_CART_SUB_VAL, MPI_CART_SUB_LABEL, BLOCK_ID_MPI_Cart_sub, GLOP_ID_NULL, FALSE },
  /* 050 */
  { MPITYPE_TOPOLOGIES, MPI_CARTDIM_GET_VAL, MPI_CARTDIM_GET_LABEL, BLOCK_ID_MPI_Cartdim_get, GLOP_ID_NULL, FALSE },
  /* 051 */
  { MPITYPE_TOPOLOGIES, MPI_DIMS_CREATE_VAL, MPI_DIMS_CREATE_LABEL, BLOCK_ID_MPI_Dims_create, GLOP_ID_NULL, FALSE },
  /* 052 */
  { MPITYPE_TOPOLOGIES, MPI_GRAPH_GET_VAL, MPI_GRAPH_GET_LABEL, BLOCK_ID_MPI_Graph_get, GLOP_ID_NULL, FALSE },
  /* 053 */
  { MPITYPE_TOPOLOGIES, MPI_GRAPH_MAP_VAL, MPI_GRAPH_MAP_LABEL, BLOCK_ID_MPI_Graph_map, GLOP_ID_NULL, FALSE },
  /* 054 */
  { MPITYPE_TOPOLOGIES, MPI_GRAPH_CREATE_VAL, MPI_GRAPH_CREATE_LABEL, BLOCK_ID_MPI_Graph_create, GLOP_ID_NULL, FALSE },
  /* 055 */
  { MPITYPE_TOPOLOGIES, MPI_GRAPH_NEIGHBORS_VAL, MPI_GRAPH_NEIGHBORS_LABEL, BLOCK_ID_MPI_Graph_neighbors, GLOP_ID_NULL, FALSE },
  /* 056 */
  { MPITYPE_TOPOLOGIES, MPI_GRAPHDIMS_GET_VAL, MPI_GRAPHDIMS_GET_LABEL, BLOCK_ID_MPI_Graphdims_get, GLOP_ID_NULL, FALSE },
  /* 057 */
  { MPITYPE_TOPOLOGIES, MPI_GRAPH_NEIGHBORS_COUNT_VAL, MPI_GRAPH_NEIGHBORS_COUNT_LABEL, BLOCK_ID_MPI_Graph_neighbors_count, GLOP_ID_NULL, FALSE },
  /* 058 */
  { MPITYPE_TOPOLOGIES, MPI_TOPO_TEST_VAL, MPI_TOPO_TEST_LABEL, BLOCK_ID_MPI_Topo_test, GLOP_ID_NULL, FALSE },
  /* 059 */
  { MPITYPE_PTOP, MPI_WAITANY_VAL, MPI_WAITANY_LABEL, BLOCK_ID_MPI_Waitany, GLOP_ID_NULL, FALSE },
  /* 060 */
  { MPITYPE_PTOP, MPI_WAITSOME_VAL, MPI_WAITSOME_LABEL, BLOCK_ID_MPI_Waitsome, GLOP_ID_NULL, FALSE },
  /* 061 */
  { MPITYPE_PTOP, MPI_PROBE_VAL, MPI_PROBE_LABEL, BLOCK_ID_MPI_Probe, GLOP_ID_NULL, FALSE },
  /* 062 */
  { MPITYPE_PTOP, MPI_IPROBE_VAL, MPI_IPROBE_LABEL, BLOCK_ID_MPI_Iprobe, GLOP_ID_NULL, FALSE },
  /*
   * RMA MPI calls (Remote Memory Access calls)
   */
  /* 063 */
  { MPITYPE_RMA, MPI_WIN_CREATE_VAL, MPI_WIN_CREATE_LABEL, BLOCK_ID_MPI_Win_create, GLOP_ID_NULL, FALSE },
  /* 064 */
  { MPITYPE_RMA, MPI_WIN_FREE_VAL, MPI_WIN_FREE_LABEL, BLOCK_ID_MPI_Win_free, GLOP_ID_NULL, FALSE },
  /* 065 */
  { MPITYPE_RMA, MPI_PUT_VAL, MPI_PUT_LABEL, BLOCK_ID_MPI_Put, GLOP_ID_NULL, FALSE },
  /* 066 */
  { MPITYPE_RMA, MPI_GET_VAL, MPI_GET_LABEL, BLOCK_ID_MPI_Get, GLOP_ID_NULL, FALSE },
  /* 067 */
  { MPITYPE_RMA, MPI_ACCUMULATE_VAL, MPI_ACCUMULATE_LABEL, BLOCK_ID_MPI_Accumulate, GLOP_ID_NULL, FALSE },
  /* 068 */
  { MPITYPE_RMA, MPI_WIN_FENCE_VAL, MPI_WIN_FENCE_LABEL, BLOCK_ID_MPI_Win_fence, GLOP_ID_NULL, FALSE },
  /* 069 */
  { MPITYPE_RMA, MPI_WIN_START_VAL, MPI_WIN_START_LABEL, BLOCK_ID_MPI_Win_start, GLOP_ID_NULL, FALSE },
  /* 070 */
  { MPITYPE_RMA, MPI_WIN_COMPLETE_VAL, MPI_WIN_COMPLETE_LABEL, BLOCK_ID_MPI_Win_complete, GLOP_ID_NULL, FALSE },
  /* 071 */
  { MPITYPE_RMA, MPI_WIN_POST_VAL, MPI_WIN_POST_LABEL, BLOCK_ID_MPI_Win_post, GLOP_ID_NULL, FALSE },
  /* 072 */
  { MPITYPE_RMA, MPI_WIN_WAIT_VAL, MPI_WIN_WAIT_LABEL, BLOCK_ID_MPI_Win_wait, GLOP_ID_NULL, FALSE },
  /* 073 */
  { MPITYPE_RMA, MPI_WIN_TEST_VAL, MPI_WIN_TEST_LABEL, BLOCK_ID_MPI_Win_test, GLOP_ID_NULL, FALSE },
  /* 074 */
  { MPITYPE_RMA, MPI_WIN_LOCK_VAL, MPI_WIN_LOCK_LABEL, BLOCK_ID_MPI_Win_lock, GLOP_ID_NULL, FALSE },
  /* 075 */
  { MPITYPE_RMA, MPI_WIN_UNLOCK_VAL, MPI_WIN_UNLOCK_LABEL, BLOCK_ID_MPI_Win_unlock, GLOP_ID_NULL, FALSE },
  /* 076 */
  { MPITYPE_OTHER, MPI_PACK_VAL, MPI_PACK_LABEL, BLOCK_ID_MPI_Pack, GLOP_ID_NULL, FALSE },
  /* 077 */
  { MPITYPE_OTHER, MPI_UNPACK_VAL, MPI_UNPACK_LABEL, BLOCK_ID_MPI_Unpack, GLOP_ID_NULL, FALSE },
  /* 078 */
  { MPITYPE_OTHER, MPI_OP_CREATE_VAL, MPI_OP_CREATE_LABEL, BLOCK_ID_MPI_Op_create, GLOP_ID_NULL, FALSE },
  /* 079 */
  { MPITYPE_OTHER, MPI_OP_FREE_VAL, MPI_OP_FREE_LABEL, BLOCK_ID_MPI_Op_free, GLOP_ID_NULL, FALSE },
  /* 080 */
  { MPITYPE_COLLECTIVE, MPI_REDUCE_SCATTER_VAL, MPI_REDUCE_SCATTER_LABEL, BLOCK_ID_MPI_Reduce_scatter, GLOP_ID_MPI_Reduce_scatter, FALSE },
  /* 081 */
  { MPITYPE_OTHER, MPI_ATTR_DELETE_VAL, MPI_ATTR_DELETE_LABEL, BLOCK_ID_MPI_Attr_delete, GLOP_ID_NULL, FALSE },
  /* 082 */
  { MPITYPE_OTHER, MPI_ATTR_GET_VAL, MPI_ATTR_GET_LABEL, BLOCK_ID_MPI_Attr_get, GLOP_ID_NULL, FALSE },
  /* 083 */
  { MPITYPE_OTHER, MPI_ATTR_PUT_VAL, MPI_ATTR_PUT_LABEL, BLOCK_ID_MPI_Attr_put, GLOP_ID_NULL, FALSE },
  /* 084 */
  { MPITYPE_GROUP, MPI_GROUP_DIFFERENCE_VAL, MPI_GROUP_DIFFERENCE_LABEL, BLOCK_ID_MPI_Group_difference, GLOP_ID_NULL, FALSE },
  /* 085 */
  { MPITYPE_GROUP, MPI_GROUP_EXCL_VAL, MPI_GROUP_EXCL_LABEL, BLOCK_ID_MPI_Group_excl, GLOP_ID_NULL, FALSE },
  /* 086 */
  { MPITYPE_GROUP, MPI_GROUP_FREE_VAL, MPI_GROUP_FREE_LABEL, BLOCK_ID_MPI_Group_free, GLOP_ID_NULL, FALSE },
  /* 087 */
  { MPITYPE_GROUP, MPI_GROUP_INCL_VAL, MPI_GROUP_INCL_LABEL, BLOCK_ID_MPI_Group_incl, GLOP_ID_NULL, FALSE },
  /* 088 */
  { MPITYPE_GROUP, MPI_GROUP_INTERSECTION_VAL, MPI_GROUP_INTERSECTION_LABEL, BLOCK_ID_MPI_Group_intersection, GLOP_ID_NULL, FALSE },
  /* 089 */
  { MPITYPE_GROUP, MPI_GROUP_RANK_VAL, MPI_GROUP_RANK_LABEL, BLOCK_ID_MPI_Group_rank, GLOP_ID_NULL, FALSE },
  /* 090 */
  { MPITYPE_GROUP, MPI_GROUP_RANGE_EXCL_VAL, MPI_GROUP_RANGE_EXCL_LABEL, BLOCK_ID_MPI_Group_range_excl, GLOP_ID_NULL, FALSE },
  /* 091 */
  { MPITYPE_GROUP, MPI_GROUP_RANGE_INCL_VAL, MPI_GROUP_RANGE_INCL_LABEL, BLOCK_ID_MPI_Group_range_incl, GLOP_ID_NULL, FALSE },
  /* 092 */
  { MPITYPE_GROUP, MPI_GROUP_SIZE_VAL, MPI_GROUP_SIZE_LABEL, BLOCK_ID_MPI_Group_size, GLOP_ID_NULL, FALSE },
  /* 093 */
  { MPITYPE_GROUP, MPI_GROUP_TRANSLATE_RANKS_VAL, MPI_GROUP_TRANSLATE_RANKS_LABEL, BLOCK_ID_MPI_Group_translate_ranks, GLOP_ID_NULL, FALSE },
  /* 094 */
  { MPITYPE_GROUP, MPI_GROUP_UNION_VAL, MPI_GROUP_UNION_LABEL, BLOCK_ID_MPI_Group_union, GLOP_ID_NULL, FALSE },
  /* 095 */
  { MPITYPE_GROUP, MPI_GROUP_COMPARE_VAL, MPI_GROUP_COMPARE_LABEL, BLOCK_ID_MPI_Group_compare, GLOP_ID_NULL, FALSE },
  /* 096 */
  { MPITYPE_OTHER, MPI_INTERCOMM_CREATE_VAL, MPI_INTERCOMM_CREATE_LABEL, BLOCK_ID_MPI_Intercomm_create, GLOP_ID_NULL, FALSE },
  /* 097 */
  { MPITYPE_OTHER, MPI_INTERCOMM_MERGE_VAL, MPI_INTERCOMM_MERGE_LABEL, BLOCK_ID_MPI_Intercomm_merge, GLOP_ID_NULL, FALSE },
  /* 098 */
  { MPITYPE_OTHER, MPI_KEYVAL_FREE_VAL, MPI_KEYVAL_FREE_LABEL, BLOCK_ID_MPI_Keyval_free, GLOP_ID_NULL, FALSE },
  /* 099 */
  { MPITYPE_OTHER, MPI_KEYVAL_CREATE_VAL, MPI_KEYVAL_CREATE_LABEL, BLOCK_ID_MPI_Keyval_create, GLOP_ID_NULL, FALSE },
  /* 100 */
  { MPITYPE_OTHER, MPI_ABORT_VAL, MPI_ABORT_LABEL, BLOCK_ID_MPI_Abort, GLOP_ID_NULL, FALSE },
  /* 101 */
  { MPITYPE_OTHER, MPI_ERROR_CLASS_VAL, MPI_ERROR_CLASS_LABEL, BLOCK_ID_MPI_Error_class, GLOP_ID_NULL, FALSE },
  /* 102 */
  { MPITYPE_OTHER, MPI_ERRHANDLER_CREATE_VAL, MPI_ERRHANDLER_CREATE_LABEL, BLOCK_ID_MPI_Errhandler_create, GLOP_ID_NULL, FALSE },
  /* 103 */
  { MPITYPE_OTHER, MPI_ERRHANDLER_FREE_VAL, MPI_ERRHANDLER_FREE_LABEL, BLOCK_ID_MPI_Errhandler_free, GLOP_ID_NULL, FALSE },
  /* 104 */
  { MPITYPE_OTHER, MPI_ERRHANDLER_GET_VAL, MPI_ERRHANDLER_GET_LABEL, BLOCK_ID_MPI_Errhandler_get, GLOP_ID_NULL, FALSE },
  /* 105 */
  { MPITYPE_OTHER, MPI_ERROR_STRING_VAL, MPI_ERROR_STRING_LABEL, BLOCK_ID_MPI_Error_string, GLOP_ID_NULL, FALSE },
  /* 106 */
  { MPITYPE_OTHER, MPI_ERRHANDLER_SET_VAL, MPI_ERRHANDLER_SET_LABEL, BLOCK_ID_MPI_Errhandler_set, GLOP_ID_NULL, FALSE },
  /* 107 */
  { MPITYPE_OTHER, MPI_GET_PROCESSOR_NAME_VAL, MPI_GET_PROCESSOR_NAME_LABEL, BLOCK_ID_MPI_Get_processor_name, GLOP_ID_NULL, FALSE },
  /* 108 */
  { MPITYPE_OTHER, MPI_INITIALIZED_VAL, MPI_INITIALIZED_LABEL, BLOCK_ID_MPI_Initialized, GLOP_ID_NULL, FALSE },
  /* 109 */
  { MPITYPE_OTHER, MPI_WTICK_VAL, MPI_WTICK_LABEL, BLOCK_ID_MPI_Wtick, GLOP_ID_NULL, FALSE },
  /* 110 */
  { MPITYPE_OTHER, MPI_WTIME_VAL, MPI_WTIME_LABEL, BLOCK_ID_MPI_Wtime, GLOP_ID_NULL, FALSE },
  /* 111 */
  { MPITYPE_OTHER, MPI_ADDRESS_VAL, MPI_ADDRESS_LABEL, BLOCK_ID_MPI_Address, GLOP_ID_NULL, FALSE },
  /* 112 */
  { MPITYPE_OTHER, MPI_BSEND_INIT_VAL, MPI_BSEND_INIT_LABEL, BLOCK_ID_MPI_Bsend_init, GLOP_ID_NULL, FALSE },
  /* 113 */
  { MPITYPE_OTHER, MPI_BUFFER_ATTACH_VAL, MPI_BUFFER_ATTACH_LABEL, BLOCK_ID_MPI_Buffer_attach, GLOP_ID_NULL, FALSE },
  /* 114 */
  { MPITYPE_OTHER, MPI_BUFFER_DETACH_VAL, MPI_BUFFER_DETACH_LABEL, BLOCK_ID_MPI_Buffer_detach, GLOP_ID_NULL, FALSE },
  /* 115 */
  { MPITYPE_OTHER, MPI_REQUEST_FREE_VAL, MPI_REQUEST_FREE_LABEL, BLOCK_ID_MPI_Request_free, GLOP_ID_NULL, FALSE },
  /* 116 */
  { MPITYPE_OTHER, MPI_RECV_INIT_VAL, MPI_RECV_INIT_LABEL, BLOCK_ID_MPI_Recv_init, GLOP_ID_NULL, FALSE },
  /* 117 */
  { MPITYPE_OTHER, MPI_SEND_INIT_VAL, MPI_SEND_INIT_LABEL, BLOCK_ID_MPI_Send_init, GLOP_ID_NULL, FALSE },
  /* 118 */
  { MPITYPE_OTHER, MPI_GET_COUNT_VAL, MPI_GET_COUNT_LABEL, BLOCK_ID_MPI_Get_count, GLOP_ID_NULL, FALSE },
  /* 119 */
  { MPITYPE_OTHER, MPI_GET_ELEMENTS_VAL, MPI_GET_ELEMENTS_LABEL, BLOCK_ID_MPI_Get_elements, GLOP_ID_NULL, FALSE },
  /* 120 */
  { MPITYPE_OTHER, MPI_PACK_SIZE_VAL, MPI_PACK_SIZE_LABEL, BLOCK_ID_MPI_Pack_size, GLOP_ID_NULL, FALSE },
  /* 121 */
  { MPITYPE_OTHER, MPI_RSEND_INIT_VAL, MPI_RSEND_INIT_LABEL, BLOCK_ID_MPI_Rsend_init, GLOP_ID_NULL, FALSE },
  /* 122 */
  { MPITYPE_OTHER, MPI_SSEND_INIT_VAL, MPI_SSEND_INIT_LABEL, BLOCK_ID_MPI_Ssend_init, GLOP_ID_NULL, FALSE },
  /* 123 */
  { MPITYPE_OTHER, MPI_START_VAL, MPI_START_LABEL, BLOCK_ID_MPI_Start, GLOP_ID_NULL, FALSE },
  /* 124 */
  { MPITYPE_OTHER, MPI_STARTALL_VAL, MPI_STARTALL_LABEL, BLOCK_ID_MPI_Startall, GLOP_ID_NULL, FALSE },
  /* 125 */
  { MPITYPE_PTOP, MPI_TESTALL_VAL, MPI_TESTALL_LABEL, BLOCK_ID_MPI_Testall, GLOP_ID_NULL, FALSE },
  /* 126 */
  { MPITYPE_PTOP, MPI_TESTANY_VAL, MPI_TESTANY_LABEL, BLOCK_ID_MPI_Testany, GLOP_ID_NULL, FALSE },
  /* 127 */
  { MPITYPE_OTHER, MPI_TEST_CANCELLED_VAL, MPI_TEST_CANCELLED_LABEL, BLOCK_ID_MPI_Test_cancelled, GLOP_ID_NULL, FALSE },
  /* 128 */
  { MPITYPE_PTOP, MPI_TESTSOME_VAL, MPI_TESTSOME_LABEL, BLOCK_ID_MPI_Testsome, GLOP_ID_NULL, FALSE },
  /* 129 */
  { MPITYPE_TYPE, MPI_TYPE_COMMIT_VAL, MPI_TYPE_COMMIT_LABEL, BLOCK_ID_MPI_Type_commit, GLOP_ID_NULL, FALSE },
  /* 130 */
  { MPITYPE_TYPE, MPI_TYPE_CONTIGUOUS_VAL, MPI_TYPE_CONTIGUOUS_LABEL, BLOCK_ID_MPI_Type_contiguous, GLOP_ID_NULL, FALSE },
  /* 131 */
  { MPITYPE_TYPE, MPI_TYPE_EXTENT_VAL, MPI_TYPE_EXTENT_LABEL, BLOCK_ID_MPI_Type_extent, GLOP_ID_NULL, FALSE },
  /* 132 */
  { MPITYPE_TYPE, MPI_TYPE_FREE_VAL, MPI_TYPE_FREE_LABEL, BLOCK_ID_MPI_Type_free, GLOP_ID_NULL, FALSE },
  /* 133 */
  { MPITYPE_TYPE, MPI_TYPE_HINDEXED_VAL, MPI_TYPE_HINDEXED_LABEL, BLOCK_ID_MPI_Type_hindexed, GLOP_ID_NULL, FALSE },
  /* 134 */
  { MPITYPE_TYPE, MPI_TYPE_HVECTOR_VAL, MPI_TYPE_HVECTOR_LABEL, BLOCK_ID_MPI_Type_hvector, GLOP_ID_NULL, FALSE },
  /* 135 */
  { MPITYPE_TYPE, MPI_TYPE_INDEXED_VAL, MPI_TYPE_INDEXED_LABEL, BLOCK_ID_MPI_Type_indexed, GLOP_ID_NULL, FALSE },
  /* 136 */
  { MPITYPE_TYPE, MPI_TYPE_LB_VAL, MPI_TYPE_LB_LABEL, BLOCK_ID_MPI_Type_lb, GLOP_ID_NULL, FALSE },
  /* 137 */
  { MPITYPE_TYPE, MPI_TYPE_SIZE_VAL, MPI_TYPE_SIZE_LABEL, BLOCK_ID_MPI_Type_size, GLOP_ID_NULL, FALSE },
  /* 138 */
  { MPITYPE_TYPE, MPI_TYPE_STRUCT_VAL, MPI_TYPE_STRUCT_LABEL, BLOCK_ID_MPI_Type_struct, GLOP_ID_NULL, FALSE },
  /* 139 */
  { MPITYPE_TYPE, MPI_TYPE_UB_VAL, MPI_TYPE_UB_LABEL, BLOCK_ID_MPI_Type_ub, GLOP_ID_NULL, FALSE },
  /* 140 */
  { MPITYPE_TYPE, MPI_TYPE_VECTOR_VAL, MPI_TYPE_VECTOR_LABEL, BLOCK_ID_MPI_Type_vector, GLOP_ID_NULL, FALSE },
  /* 141 */
  { MPITYPE_IO, MPI_FILE_OPEN_VAL, MPI_FILE_OPEN_LABEL, BLOCK_ID_MPI_File_open, GLOP_ID_NULL, FALSE },
  /* 142 */
  { MPITYPE_IO, MPI_FILE_CLOSE_VAL, MPI_FILE_CLOSE_LABEL, BLOCK_ID_MPI_File_close, GLOP_ID_NULL, FALSE },
  /* 143 */
  { MPITYPE_IO, MPI_FILE_READ_VAL, MPI_FILE_READ_LABEL, BLOCK_ID_MPI_File_read, GLOP_ID_NULL, FALSE },
  /* 144 */
  { MPITYPE_IO, MPI_FILE_READ_ALL_VAL, MPI_FILE_READ_ALL_LABEL, BLOCK_ID_MPI_File_read_all, GLOP_ID_NULL, FALSE },
  /* 145 */
  { MPITYPE_IO, MPI_FILE_WRITE_VAL, MPI_FILE_WRITE_LABEL, BLOCK_ID_MPI_File_write, GLOP_ID_NULL, FALSE },
  /* 146 */
  { MPITYPE_IO, MPI_FILE_WRITE_ALL_VAL, MPI_FILE_WRITE_ALL_LABEL, BLOCK_ID_MPI_File_write_all, GLOP_ID_NULL, FALSE },
  /* 147 */
  { MPITYPE_IO, MPI_FILE_READ_AT_VAL, MPI_FILE_READ_AT_LABEL, BLOCK_ID_MPI_File_read_at, GLOP_ID_NULL, FALSE },
  /* 148 */
  { MPITYPE_IO, MPI_FILE_READ_AT_ALL_VAL, MPI_FILE_READ_AT_ALL_LABEL, BLOCK_ID_MPI_File_read_at_all, GLOP_ID_NULL, FALSE },
  /* 149 */
  { MPITYPE_IO, MPI_FILE_WRITE_AT_VAL, MPI_FILE_WRITE_AT_LABEL, BLOCK_ID_MPI_File_write_at, GLOP_ID_NULL, FALSE },
  /* 150 */
  { MPITYPE_IO, MPI_FILE_WRITE_AT_ALL_VAL, MPI_FILE_WRITE_AT_ALL_LABEL, BLOCK_ID_MPI_File_write_at_all, GLOP_ID_NULL, FALSE },
  /* 151 */
  { MPITYPE_COMM, MPI_COMM_SPAWN_VAL, MPI_COMM_SPAWN_LABEL, BLOCK_ID_MPI_Comm_spawn, GLOP_ID_NULL, FALSE },
  /* 152 */
  { MPITYPE_COMM, MPI_COMM_SPAWN_MULTIPLE_VAL, MPI_COMM_SPAWN_MULTIPLE_LABEL, BLOCK_ID_MPI_Comm_spawn_multiple, GLOP_ID_NULL, FALSE },
  /* 153 */
  { MPITYPE_OTHER, MPI_REQUEST_GET_STATUS_VAL, MPI_REQUEST_GET_STATUS_LABEL, BLOCK_ID_MPI_Request_get_status, GLOP_ID_NULL, FALSE },
  /* 154 */
  { MPITYPE_COLLECTIVE, MPI_IREDUCE_VAL, MPI_IREDUCE_LABEL, BLOCK_ID_MPI_Ireduce, GLOP_ID_MPI_Ireduce, FALSE },
  /* 155 */
  { MPITYPE_COLLECTIVE, MPI_IALLREDUCE_VAL, MPI_IALLREDUCE_LABEL, BLOCK_ID_MPI_Iallreduce, GLOP_ID_MPI_Iallreduce, FALSE },
  /* 156 */
  { MPITYPE_COLLECTIVE, MPI_IBARRIER_VAL, MPI_IBARRIER_LABEL, BLOCK_ID_MPI_Ibarrier, GLOP_ID_MPI_Ibarrier, FALSE },
  /* 157 */
  { MPITYPE_COLLECTIVE, MPI_IBCAST_VAL, MPI_IBCAST_LABEL, BLOCK_ID_MPI_Ibcast, GLOP_ID_MPI_Ibcast, FALSE },
  /* 158 */
  { MPITYPE_COLLECTIVE, MPI_IALLTOALL_VAL, MPI_IALLTOALL_LABEL, BLOCK_ID_MPI_Ialltoall, GLOP_ID_MPI_Ialltoall, FALSE },
  /* 159 */
  { MPITYPE_COLLECTIVE, MPI_IALLTOALLV_VAL, MPI_IALLTOALLV_LABEL, BLOCK_ID_MPI_Ialltoallv, GLOP_ID_MPI_Ialltoallv, FALSE },
  /* 160 */
  { MPITYPE_COLLECTIVE, MPI_IALLGATHER_VAL, MPI_IALLGATHER_LABEL, BLOCK_ID_MPI_Iallgather, GLOP_ID_MPI_Iallgather, FALSE },
  /* 161 */
  { MPITYPE_COLLECTIVE, MPI_IALLGATHERV_VAL, MPI_IALLGATHERV_LABEL, BLOCK_ID_MPI_Iallgatherv, GLOP_ID_MPI_Iallgatherv, FALSE },
  /* 162 */
  { MPITYPE_COLLECTIVE, MPI_IGATHER_VAL, MPI_IGATHER_LABEL, BLOCK_ID_MPI_Igather, GLOP_ID_MPI_Igather, FALSE },
  /* 163 */
  { MPITYPE_COLLECTIVE, MPI_IGATHERV_VAL, MPI_IGATHERV_LABEL, BLOCK_ID_MPI_Igatherv, GLOP_ID_MPI_Igatherv, FALSE },
  /* 164 */
  { MPITYPE_COLLECTIVE, MPI_ISCATTER_VAL, MPI_ISCATTER_LABEL, BLOCK_ID_MPI_Iscatter, GLOP_ID_MPI_Iscatter, FALSE },
  /* 165 */
  { MPITYPE_COLLECTIVE, MPI_ISCATTERV_VAL, MPI_ISCATTERV_LABEL, BLOCK_ID_MPI_Iscatterv, GLOP_ID_MPI_Iscatterv, FALSE },
  /* 166 */
  { MPITYPE_COLLECTIVE, MPI_IREDUCESCAT_VAL, MPI_IREDUCESCAT_LABEL, BLOCK_ID_MPI_Ireduce_scatter, GLOP_ID_MPI_Ireduce_scatter, FALSE },
  /* 167 */
  { MPITYPE_COLLECTIVE, MPI_ISCAN_VAL, MPI_ISCAN_LABEL, BLOCK_ID_MPI_Iscan, GLOP_ID_MPI_Iscan, FALSE },
  /* 168 */
  { MPITYPE_COLLECTIVE, MPI_REDUCE_SCATTER_BLOCK_VAL, MPI_REDUCE_SCATTER_BLOCK_LABEL, BLOCK_ID_MPI_Reduce_scatter_block, GLOP_ID_MPI_Reduce_scatter_block, FALSE },
  /*169 */
  { MPITYPE_COLLECTIVE, MPI_IREDUCE_SCATTER_BLOCK_VAL, MPI_IREDUCE_SCATTER_BLOCK_LABEL, BLOCK_ID_MPI_Ireduce_scatter_block, GLOP_ID_MPI_Ireduce_scatter_block, FALSE },
  /*170 */
  { MPITYPE_COLLECTIVE, MPI_ALLTOALLW_VAL, MPI_ALLTOALLW_LABEL, BLOCK_ID_MPI_Alltoallw, GLOP_ID_MPI_Alltoallw, FALSE },
  /*171 */
  { MPITYPE_COLLECTIVE, MPI_IALLTOALLW_VAL, MPI_IALLTOALLW_LABEL, BLOCK_ID_MPI_Ialltoallw, GLOP_ID_MPI_Ialltoallw, FALSE },
  /*172 */
  { MPITYPE_RMA, MPI_GET_ACCUMULATE_VAL, MPI_GET_ACCUMULATE_LABEL, BLOCK_ID_MPI_Get_accumulate, GLOP_ID_NULL, FALSE },
  /*173 */
  { MPITYPE_TOPOLOGIES, MPI_DIST_GRAPH_CREATE_VAL, MPI_DIST_GRAPH_CREATE_LABEL, BLOCK_ID_MPI_Dist_graph_create, GLOP_ID_NULL, FALSE },
  /*174 */
  { MPITYPE_COLLECTIVE, MPI_NEIGHBOR_ALLGATHER_VAL, MPI_NEIGHBOR_ALLGATHER_LABEL, BLOCK_ID_MPI_Neighbor_allgather, GLOP_ID_NULL, FALSE },
  /*175 */
  { MPITYPE_COLLECTIVE, MPI_INEIGHBOR_ALLGATHER_VAL, MPI_INEIGHBOR_ALLGATHER_LABEL, BLOCK_ID_MPI_Ineighbor_allgather, GLOP_ID_NULL, FALSE },
  /*176*/
  { MPITYPE_COLLECTIVE, MPI_NEIGHBOR_ALLGATHERV_VAL, MPI_NEIGHBOR_ALLGATHERV_LABEL, BLOCK_ID_MPI_Neighbor_allgatherv, GLOP_ID_NULL, FALSE },
  /* 177*/
  { MPITYPE_COLLECTIVE, MPI_INEIGHBOR_ALLGATHERV_VAL, MPI_INEIGHBOR_ALLGATHERV_LABEL, BLOCK_ID_MPI_Ineighbor_allgatherv, GLOP_ID_NULL, FALSE },
  /* 178*/
  { MPITYPE_COLLECTIVE, MPI_NEIGHBOR_ALLTOALL_VAL, MPI_NEIGHBOR_ALLTOALL_LABEL, BLOCK_ID_MPI_Neighbor_alltoall, GLOP_ID_NULL, FALSE },
  /* 179*/
  { MPITYPE_COLLECTIVE, MPI_INEIGHBOR_ALLTOALL_VAL, MPI_NEIGHBOR_ALLTOALL_LABEL, BLOCK_ID_MPI_Ineighbor_alltoall, GLOP_ID_NULL, FALSE },
  /* 180*/
  { MPITYPE_COLLECTIVE, MPI_NEIGHBOR_ALLTOALLV_VAL, MPI_NEIGHBOR_ALLTOALLV_LABEL, BLOCK_ID_MPI_Neighbor_alltoallv, GLOP_ID_NULL, FALSE },
  /* 181 */
  { MPITYPE_COLLECTIVE, MPI_INEIGHBOR_ALLTOALLV_VAL, MPI_INEIGHBOR_ALLTOALLV_LABEL, BLOCK_ID_MPI_Ineighbor_alltoallv, GLOP_ID_NULL, FALSE },
  /* 182*/
  { MPITYPE_COLLECTIVE, MPI_NEIGHBOR_ALLTOALLW_VAL, MPI_NEIGHBOR_ALLTOALLW_LABEL, BLOCK_ID_MPI_Neighbor_alltoallw, GLOP_ID_NULL, FALSE },
  /*183 */
  { MPITYPE_COLLECTIVE, MPI_INEIGHBOR_ALLTOALLW_VAL, MPI_INEIGHBOR_ALLTOALLW_LABEL, BLOCK_ID_MPI_Ineighbor_alltoallw, GLOP_ID_NULL, FALSE },
  /*184*/
  { MPITYPE_RMA, MPI_FETCH_AND_OP_VAL, MPI_FETCH_AND_OP_LABEL, BLOCK_ID_MPI_Fetch_and_op, GLOP_ID_NULL, FALSE },
  /*185 */
  { MPITYPE_RMA, MPI_COMPARE_AND_SWAP_VAL, MPI_COMPARE_AND_SWAP_LABEL, BLOCK_ID_MPI_Compare_and_swap, GLOP_ID_NULL, FALSE },
  /* 186 */
  { MPITYPE_RMA, MPI_WIN_FLUSH_VAL, MPI_WIN_FLUSH_LABEL, BLOCK_ID_MPI_Win_flush, GLOP_ID_NULL, FALSE },
  /*187 */
  { MPITYPE_RMA, MPI_WIN_FLUSH_ALL_VAL, MPI_WIN_FLUSH_ALL_LABEL, BLOCK_ID_MPI_Win_flush_all, GLOP_ID_NULL, FALSE },
  /*188 */
  { MPITYPE_RMA, MPI_WIN_FLUSH_LOCAL_VAL, MPI_WIN_FLUSH_LOCAL_LABEL, BLOCK_ID_MPI_Win_flush_local, GLOP_ID_NULL, FALSE },
  /*189 */
  { MPITYPE_RMA, MPI_WIN_FLUSH_LOCAL_ALL_VAL, MPI_WIN_FLUSH_LOCAL_ALL_LABEL, BLOCK_ID_MPI_Win_flush_local_all, GLOP_ID_NULL, FALSE },
  /*190 */
  { MPITYPE_PTOP, MPI_MPROBE_VAL, MPI_MPROBE_LABEL, BLOCK_ID_MPI_Mprobe, GLOP_ID_NULL, FALSE },
  /*191 */
  { MPITYPE_PTOP, MPI_IMPROBE_VAL, MPI_IMPROBE_LABEL, BLOCK_ID_MPI_Improbe, GLOP_ID_NULL, FALSE },
  /*192*/
  { MPITYPE_PTOP, MPI_MRECV_VAL, MPI_MRECV_LABEL, BLOCK_ID_MPI_Mrecv, GLOP_ID_NULL, FALSE },
  /*193 */
  { MPITYPE_PTOP, MPI_IMRECV_VAL, MPI_IMRECV_LABEL, BLOCK_ID_MPI_Imrecv, GLOP_ID_NULL, FALSE },
  // In order to have the same Ids than extrae I've moved this
  // MPI from 141 to here.
  /*194*/
  { MPITYPE_OTHER, MPI_INIT_THREAD_VAL, MPI_INIT_THREAD_LABEL, BLOCK_ID_MPI_Init_thread, GLOP_ID_NULL, FALSE }
};


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


DimBlock MPIEventEncoding_DimemasBlockId( MPI_Event_Values Op )
{
  /*
   * if the table typedef enum{...}MPI_Event_Values; from the EventEncoding.h is
   * changed then we must have to made same changes on MPI_Table.
   */
  ASSERT( Op < NUM_MPICALLS );
  ASSERT( MPI_Table[ Op ].Op == Op );

  return ( MPI_Table[ Op ].Block );
}


DimCollectiveOp MPIEventEncoding_GlobalOpId( MPI_Event_Values Op )
{
  ASSERT( Op < NUM_MPICALLS );
  ASSERT( MPI_Table[ Op ].Op == Op );

  return ( MPI_Table[ Op ].GlobalOpId );
}


int MPIEventEncoding_Is_MPIBlock( long64_t Type )
{
  for ( int ii = 0; ii < NUM_MPITYPES; ii++ )
  {
    if ( Type == (long64_t)MPIType_Table[ ii ].Type )
      return ( TRUE );
  }

  return ( FALSE );
}


int MPIEventEncoding_Is_UserBlock( long64_t Type )
{
  if ( Type == (long64_t)USER_FUNCTION || Type == (long64_t)USER_CALL || Type == (long64_t)USER_BLOCK )
    return ( TRUE );

  return ( FALSE );
}


long64_t MPIEventEncoding_UserBlockId( long64_t Type, long64_t Value )
{
  if ( Type == (long64_t)USER_FUNCTION )
    return ( Value + BASE_USERFUNCTION );
  if ( Type == (long64_t)USER_CALL )
    return ( Value + BASE_USERCALL );
  if ( Type == (long64_t)USER_BLOCK )
    return ( Value + BASE_USERBLOCK );
  if ( Type == (long64_t)CLUSTER_ID_EV )
    return ( Value + BASE_CLUSTER_BLOCK );

  assert( "MPIEventEncoding_UserBlockId: Invalid Type" );
}

/******************************************************************************
 **      Function name : MPIEventEncoding_Is_BlockBegin
 **
 **      Description :
 ******************************************************************************/

int MPIEventEncoding_Is_BlockBegin( long64_t Op )
{
  return ( ( Op == (long64_t)MPIEND_VAL ) ? FALSE : TRUE );
}

/******************************************************************************
 **      Function name : MPIEventEncoding_WriteEnabledSoftcounters
 **      Author : HSG
 **      Description : Genera informacio al PCF sobre rutines que poden
 **                    donar valors sobre software counters
 ******************************************************************************/

static void MPIEventEncoding_WriteEnabledSoftcounters( FILE *fd )
{
  ASSERT( fd != NULL );

  /* MPI_Test i MPI_Testsome generen un software counter comu! */
  if ( MPI_Table[ MPI_TEST_VAL ].Enabled || MPI_Table[ MPI_TESTSOME_VAL ].Enabled )
  {
    fprintf( fd, "%s\n", TYPE_LABEL );
    fprintf( fd, "%d  %d   %s\n", 0, MPITYPE_TEST_SOFTCOUNTER, MPITYPE_TEST_SOFTCOUNTER_LABEL );
    fprintf( fd, "\n\n" );
  }

  /* HSG, codi que hauria d'emprar-se si es fan soft counters per MPI_Iprobe */
}

/******************************************************************************
 **      Function name : MPIEventEncoding_WriteCollectiveInfo
 **      Author : HSG
 **      Description : Escriu informacio sobre rutines colectives.
 **
 *****************************************************************************/

static void MPIEventEncoding_WriteCollectiveInfo( FILE *fd )
{
  ASSERT( fd != NULL );

  /* MPI_Test i MPI_Testsome generen un software counter comu! */
  /*
  if (MPI_Table[MPI_BCAST_VAL].Enabled ||
    MPI_Table[MPI_ALLGATHER_VAL].Enabled ||
    MPI_Table[MPI_ALLREDUCE_VAL].Enabled ||
    MPI_Table[MPI_BARRIER_VAL].Enabled ||
    MPI_Table[MPI_REDUCE_SCATTER_VAL].Enabled)
  */
  {
    fprintf( fd, "%s\n", TYPE_LABEL );
    fprintf( fd, "%d  %d   %s\n", 0, MPI_GLOBAL_OP_ROOT, "MPI Global OP is root?" );
    fprintf( fd, "%d  %d   %s\n", 0, MPI_GLOBAL_OP_COMM, "MPI Global OP communicator" );
    fprintf( fd, "%d  %d   %s\n", 0, MPI_GLOBAL_OP_SENDSIZE, "MPI Global OP send size" );
    fprintf( fd, "%d  %d   %s\n", 0, MPI_GLOBAL_OP_RECVSIZE, "MPI Global OP recv size" );
    fprintf( fd, "\n\n" );
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

  for ( ii = 0; ii < NUM_MPITYPES; ii++ )
  {
    Type = MPIType_Table[ ii ].Type;

    /* Primer comptem si hi ha alguna operacio MPI del grup actual */
    cnt = 0;
    for ( jj = 0; jj < NUM_MPICALLS; jj++ )
    {
      if ( Type == MPI_Table[ jj ].Type )
        if ( MPI_Table[ jj ].Enabled )
          cnt++;
    }

    if ( cnt )
    {
      fprintf( fd, "%s\n", TYPE_LABEL );
      fprintf( fd, "%d   %d    %s\n", 0, MPIType_Table[ ii ].Type, MPIType_Table[ ii ].Label );

      fprintf( fd, "%s\n", VALUES_LABEL );
      for ( jj = 1; jj < NUM_MPICALLS; jj++ )
      {
        if ( Type == MPI_Table[ jj ].Type )
          if ( MPI_Table[ jj ].Enabled )
            fprintf( fd, "%d   %s\n", MPI_Table[ jj ].Op, MPI_Table[ jj ].Label );
      }
      fprintf( fd, "%d   %s\n", MPI_Table[ 0 ].Op, MPI_Table[ 0 ].Label );
      fprintf( fd, "\n\n" );
    }
  }
  MPIEventEncoding_WriteEnabledSoftcounters( fd );
  MPIEventEncoding_WriteCollectiveInfo( fd );
}


char *MPIEventEncoding_GetBlockLabel( MPI_Event_Values Op )
{
  ASSERT( Op < NUM_MPICALLS );

  return ( MPI_Table[ Op ].Label );
}


int ClusterEventEncoding_Is_ClusterBlock( long64_t type )
{
  return ( ( type == (long64_t)CLUSTER_ID_EV ) ? TRUE : FALSE );
}


int ClusterEventEncoding_Is_BlockBegin( long64_t Op )
{
  return ( ( Op == (long64_t)CLUSTEREND_VAL ) ? FALSE : TRUE );
}


DimBlock ClusterEventEncoding_DimemasBlockId( long64_t value )
{
  return ( ( DimBlock )( value + BASE_CLUSTER_BLOCK ) );
}


/* ---------------------------------------------------- Data Types ----------*/
typedef struct
{
  int Type;
  char *Label;

} CUDATypeInfo;

Boolean CUDAEventEconding_Is_OldLibType( long64_t type )
{
  return ( ( type == OLD_CUDA_LIB_CALL_EV || type == OLDEST_CUDA_LIB_CALL_EV ) ? TRUE : FALSE );
}

Boolean CUDAEventEconding_Is_OldKernelType( long64_t type )
{
  return ( ( type == OLD_CUDA_KERNEL_EV ) ? TRUE : FALSE );
}

Boolean CUDAEventEncoding_Is_CUDABlock( long64_t type )
{
  return ( type == CUDA_LIB_CALL_EV ? TRUE : FALSE );
}

Boolean CUDAEventEncoding_Is_Kernel( long64_t type )
{
  return ( ( type == CUDA_KERNEL_EV ) ? TRUE : FALSE );
}

Boolean CUDAEventEncoding_Is_Kernel_Block( struct t_event_block event )
{
  if ( CUDAEventEncoding_Is_Kernel( event.type ) == TRUE && event.value != CUDA_END_VAL )
    return TRUE;
  return FALSE;
}

Boolean CUDAEventEncoding_Is_BlockBegin( long64_t Op )
{
  return ( ( Op == (long64_t)CUDA_END_VAL ) ? FALSE : TRUE );
}

Boolean CUDAEventEncoding_Is_CUDAComm( struct t_thread *sender, struct t_thread *receiver )
{
  if( ( sender != NULL && sender->stream ) ||
      ( receiver != NULL && receiver->stream ) )
    return TRUE;
  return FALSE;
}

Boolean CUDAEventEncoding_Is_CUDATransferBlock( struct t_event_block event )
{
  if ( CUDAEventEncoding_Is_CUDABlock( event.type ) == TRUE && ( event.value == CUDA_MEMCPY_VAL || event.value == CUDA_MEMCPY_ASYNC_VAL ) )
    return TRUE;
  return FALSE;
}

Boolean CUDAEventEncoding_Is_CUDAMemcpy( struct t_event_block event )
{
  if ( CUDAEventEncoding_Is_CUDABlock( event.type ) == TRUE && event.value == CUDA_MEMCPY_VAL )
    return TRUE;
  return FALSE;
}

Boolean CUDAEventEconding_Is_CUDAConfigCall( struct t_event_block event )
{
  if ( CUDAEventEncoding_Is_CUDABlock( event.type ) == TRUE && event.value == CUDA_CONFIGURECALL_VAL )
    return TRUE;
  return FALSE;
}

Boolean CUDAEventEconding_Is_CUDALaunch( struct t_event_block event )
{
  if ( CUDAEventEncoding_Is_CUDABlock( event.type ) == TRUE && event.value == CUDA_LAUNCH_VAL )
    return TRUE;
  return FALSE;
}

Boolean CUDAEventEconding_Is_CUDASync( struct t_event_block event )
{
  if ( CUDAEventEncoding_Is_CUDABlock( event.type ) == TRUE && ( event.value == CUDA_THREADSYNCHRONIZE_VAL || event.value == CUDA_STREAMSYNCHRONIZE_VAL ) )
    return TRUE;
  return FALSE;
}

Boolean CUDAEventEconding_Is_CUDAStreamSync( struct t_event_block event )
{
  if ( CUDAEventEncoding_Is_CUDABlock( event.type ) == TRUE && event.value == CUDA_STREAMSYNCHRONIZE_VAL )
    return TRUE;
  return FALSE;
}

Boolean CUDAEventEconding_Is_CUDAStreamCreate( struct t_even *event )
{
  if ( CUDAEventEncoding_Is_CUDABlock( event->type ) == TRUE && event->value == CUDA_STREAM_CREATE_VAL )
    return TRUE;
  return FALSE;
}

/* ---------------------------------------------------- Global Variables ----*/
#define NUM_OCLTYPES 2
CUDATypeInfo OCLType_Table[ NUM_OCLTYPES ] = {

  { OCL_HOST_CALL_EV, OCL_HOST_CALL_LABEL },
  { OCL_ACCELERATOR_CALL_EV, OCL_ACCELERATOR_CALL_LABEL } /*,
{ OCL_KERNEL_NAME_EV,  			OCL_KERNEL_NAME_LABEL }*/
                                                          /*{ OCL_SYNCH_STREAM_EV,			OCL_SYNCH_STREAM_LABEL }*/
};

/******************************************************************************
 **      Function name : OCLEventEncoding_Is_OCLBlock
 **
 **      Description : Returns true if is an OCL event
 ******************************************************************************/

Boolean OCLEventEncoding_Is_OCLBlock( long64_t type )
{
  for ( int i = 0; i < NUM_OCLTYPES; i++ )
  {
    if ( type == (long64_t)OCLType_Table[ i ].Type )
      return ( TRUE );
  }

  return ( FALSE );
}

/******************************************************************************
 **      Function name : OCLEventEncoding_Is_BlockBegin
 **
 **      Description : Returns true if OCL event is a block begin
 ******************************************************************************/
Boolean OCLEventEncoding_Is_BlockBegin( long64_t Op )
{
  return ( ( Op == (long64_t)OCL_OUTSIDE_VAL ) ? FALSE : TRUE );
}

/******************************************************************************
 **      Function name : OCLEventEncoding_Is_OCLComm
 **
 **      Description : Returns true if it's a OCL communication
 ******************************************************************************/
Boolean OCLEventEncoding_Is_OCLComm( long64_t tag )
{
  if ( tag == OCL_TAG )
    return TRUE;
  return FALSE;
}

/******************************************************************************
 **      Function name : OCLEventEncoding_Is_OCLSchedBlock
 **
 **      Description : Returns true if it's a OCL event that generates
 **      							 a Sched & Fork/Join CPU state.
 ******************************************************************************/
Boolean OCLEventEncoding_Is_OCLSchedBlock( struct t_event_block event )
{
  if ( event.type == OCL_KERNEL_NAME_EV )
    return TRUE;
  if ( event.type != OCL_HOST_CALL_EV )
    return FALSE;

  switch ( event.value )
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
Boolean OCLEventEncoding_Is_OCLSchedblock( long64_t type, long64_t value )
{
  if ( type != OCL_HOST_CALL_EV )
    return FALSE;

  switch ( value )
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
Boolean OCLEventEncoding_Is_OCLTransferBlock( struct t_event_block event )
{
  if ( event.type != OCL_HOST_CALL_EV && event.type != OCL_ACCELERATOR_CALL_EV )
    return FALSE;

  switch ( event.value )
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
Boolean OCLEventEncoding_Is_OCLSyncBlock( struct t_event_block event )
{
  if ( event.type == OCL_HOST_CALL_EV || event.type == OCL_ACCELERATOR_CALL_EV )
  {
    if ( event.value == OCL_FINISH_VAL )
      return TRUE;
  }
  return FALSE;
}

/******************************************************************************
 **      Function name : OCLEventEncoding_Is_OCLKernelRunning
 **
 **      Description : Returns true if it's a OCL Kernel burst event
 ******************************************************************************/
Boolean OCLEventEncoding_Is_OCLKernelRunning( struct t_event_block event )
{
  if ( event.type == OCL_ACCELERATOR_CALL_EV && event.value == OCL_ENQUEUE_NDRANGE_KERNEL_ACC_VAL )
    return TRUE;

  return FALSE;
}

#define NUM_OMPTYPES   14
#define NUM_OMP_BLOCKS 10
CUDATypeInfo OMPType_Table[ NUM_OMPTYPES ] = {

  { OMP_PARALLEL_EV, OMP_PARALLEL_LABEL },
  { OMP_WORKSHARING_EV, OMP_WORKSHARING_LABEL },
  { OMP_BARRIER, OMP_BARRIER_LABEL },
  { OMP_WORK_EV, OMP_WORK_LABEL },
  { OMP_EXECUTED_PARALLEL_FXN, OMP_EXECUTED_PARALLEL_FXN_LABEL },
  { OMP_PTHREAD_FXN, OMP_PTHREAD_FXN_LABEL },
  { OMP_EXE_TASK_FXN, OMP_EXE_TASK_FXN_LABEL },
  { OMP_INIT_TASK_FXN, OMP_INIT_TASK_FXN_LABEL },
  { OMP_SET_NUM_THREADS, OMP_SET_NUM_THREADS_LABEL },
  { OMP_GET_NUM_THREADS, OMP_GET_NUM_THREADS_LABEL },
  { OMP_EXE_PARALLEL_FXN_LINE_N_FILE, OMP_EXE_PARALLEL_FXN_LINE_N_FILE_LABEL },
  { OMP_PTHREAD_FXN_LINE_N_FILE, OMP_PTHREAD_FXN_LINE_N_FILE_LABEL },
  { OMP_EXE_TASK_FXN_LINE_N_FILE, OMP_EXE_TASK_FXN_LINE_N_FILE_LABEL },
  { OMP_INIT_TASK_FXN_LINE_N_FILE, OMP_INIT_TASK_FXN_LINE_N_FILE_LABEL }
};

/**
 * To check either the event is of OMP Type
 */
Boolean OMPEventEncoding_Is_OMPType( long64_t type )
{
  for ( int i = 0; i < NUM_OMPTYPES; ++i )
  {
    if ( type == (long64_t)OMPType_Table[ i ].Type )
      return ( TRUE );
  }
  return ( FALSE );
}

/**
 * To check either the event is of OMP Block
 */
Boolean OMPEventEncoding_Is_OMPBlock( long64_t type )
{
  for ( int i = 0; i < NUM_OMP_BLOCKS; ++i )
  {
    if ( type == (long64_t)OMPType_Table[ i ].Type )
      return ( TRUE );
  }
  return ( FALSE );
}

Boolean OMPEventEncoding_Is_BlockBegin( long64_t Op )
{
  return ( ( Op == (long64_t)OMP_END_VAL ) ? FALSE : TRUE );
}


Boolean OMPEventEncoding_Is_OMP_fork_end( struct t_event_block event )
{
  if ( event.type == OMP_EXECUTED_PARALLEL_FXN && event.value == OMP_END_VAL )
    return TRUE;
  return FALSE;
}

Boolean OMPEventEncoding_Is_OMP_Running( struct t_event_block event )
{
  if ( ( event.type == OMP_EXECUTED_PARALLEL_FXN && event.value > OMP_END_VAL ) ||
       ( event.type == OMP_BARRIER && event.value == OMP_END_VAL ) ||
       ( event.type == OMP_WORK_EV && event.value == OMP_END_VAL ) )
    return TRUE;
  return FALSE;
}


Boolean OMPEventEncoding_Is_OMPWorker_Running( struct t_event_block event )
{
  if ( ( event.type == OMP_EXECUTED_PARALLEL_FXN ) && event.value > OMP_END_VAL )
    return TRUE;
  return FALSE;
}


Boolean OMPEventEncoding_Is_OMPWorker_Running_End( struct t_event_block event )
{
  if ( ( event.type == OMP_EXECUTED_PARALLEL_FXN ) && event.value == OMP_END_VAL )
    return TRUE;
  return FALSE;
}

/**
 * OMP synchronization
 */
Boolean OMPEventEncoding_Is_OMPSync( struct t_event_block event )
{
  if ( event.type == OMP_BARRIER && event.value == OMP_BEGIN_VAL )
    return TRUE;
  return FALSE;
}


/**
 * Scheduling and fork join
 */
Boolean OMPEventEncoding_Is_OMP_fork_begin( struct t_event_block event )
{
  if ( event.type == OMP_PARALLEL_EV && event.value == REGION_OPEN )
    return TRUE;
  return FALSE;
}


/**
 * Worksharing or other OpenMP calls translated to Scheduling State
 */
Boolean OMPEventEncoding_Is_OMPSched( struct t_event_block event )
{
  if ( ( event.type == OMP_WORKSHARING_EV && ( event.value == DO_WORKSHARE || event.value == SINGLE_WORKSHARE ) ) ||
       ( event.type == OMP_WORK_EV && event.value == OMP_BEGIN_VAL ) || ( event.type == OMP_SET_NUM_THREADS && event.value == OMP_BEGIN_VAL ) )
    return TRUE;
  return FALSE;
}
