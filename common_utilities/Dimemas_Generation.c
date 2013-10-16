/* ---------------------------------------------------- Include Files -------*/
#include <CommonMacros.h>
#include <Dimemas_Generation.h>
#include <EventEncoding.h>

#ifdef HAVE_CONFIG_H
#ifdef CLUSTERING_SUITE
#include "clustering_suite_config.h"
#else
#include "config.h"
#endif
#endif

/* ---------------------------------------------------- Constants -----------*/

/* ---------------------------------------------------- Global Variables ----*/
#define DIMEMASSDDF_LINES 108
static char *DimemasSDDF[ DIMEMASSDDF_LINES ] =
{
  "SDDFA;;",
  "",
  "#1:",
  "\"CPU burst\" {",
      "\tint      \"taskid\";",
      "\tint      \"thid\";",
      "\tdouble   \"time\";",
  "};;\n\n",
  /* ----------------------------------------------------------------------- */
  "#2:",
  "\"NX send\" {",
      "\tint      \"taskid\";",
      "\tint      \"thid\";",
      "\tint      \"dest taskid\";",
      "\tint      \"msg length\";",
      "\tint      \"tag\";",
      "\tint      \"commid\";",
      "\tint      \"synchronism\";",
  "};;\n\n",
  /* ----------------------------------------------------------------------- */
  "#3:",
  "\"NX recv\" {",
      "\tint      \"taskid\";",
      "\tint      \"thid\";",
      "\tint      \"source taskid\";",
      "\tint      \"msg length\";",
      "\tint      \"tag\";",
      "\tint      \"commid\";",
      "\tint      \"type\";",
  "};;\n\n",
  /* ----------------------------------------------------------------------- */
  "#40:",
  "\"block begin\" {",
      "\tint      \"taskid\";",
      "\tint      \"thid\";",
      "\tint      \"blockid\";",
  "};;\n\n",
  /* ----------------------------------------------------------------------- */
  "#41:",
  "\"block end\" {",
      "\tint      \"taskid\";",
      "\tint      \"thid\";",
      "\tint      \"blockid\";",
  "};;\n\n",
  /* ----------------------------------------------------------------------- */
  "#50:",
  "\"user event\" {",
      "\tint      \"taskid\";",
      "\tint      \"thid\";",
      "\tint      \"type\";",
      "\tint      \"value\";",
  "};;\n\n",
  /* ----------------------------------------------------------------------- */
  "#42:",
  "\"block definition\" {",
      "\tint      \"block_id\";",
      "\tchar     \"block_name\"[];",
      "\tchar     \"activity_name\"[];",
      "\tint      \"src_file\";",
      "\tint      \"src_line\";",
  "};;\n\n",
  /* ----------------------------------------------------------------------- */
  "#43:",
  "\"file definition\" {",
      "\tint      \"file_id\";",
      "\tchar     \"location\"[];",
  "};;\n\n",
  /* ----------------------------------------------------------------------- */
  "#100:",
  "\"communicator definition\" {",
      "\tint     \"comm_id\";",
      "\tint     \"comm_size\";",
      "\tint     \"global_ranks\"[];",
  "};;\n\n",
  /* ----------------------------------------------------------------------- */
  "#200:",
  "\"global OP definition\" {",
      "\tint     \"glop_id\";",
      "\tchar     \"glop_name\"[];",
  "};;\n\n",
  /* ----------------------------------------------------------------------- */
  "#48:",
  "\"user event type definition\" {",
      "\tint     \"type\";",
      "\tchar    \"name\"[];",
      "\tint     \"color\";",
  "};;\n\n",
  /* ----------------------------------------------------------------------- */
  "#49:",
  "\"user event value definition\" {",
      "\tint     \"type\";",
      "\tint     \"value\";",
      "\tchar    \"name\"[];",
  "};;\n\n",
  /* ----------------------------------------------------------------------- */
  "#201:",
  "\"global OP\" {",
      "\tint     \"rank\";",
      "\tint     \"thid\";",
      "\tint     \"glop_id\";",
      "\tint     \"comm_id\";",
      "\tint     \"root_rank\";",
      "\tint     \"root_thid\";",
      "\tint     \"bytes_sent\";",
      "\tint     \"bytes_recvd\";",
  "};;\n\n",
  /* ----------------------------------------------------------------------- */
  "#500:",
  "\"1sided OP definition\" {",
      "\tint     \"1sided_op_id\";",
      "\tchar    \"1sided_op_name\"[];",
  "};;\n\n",
  /* ----------------------------------------------------------------------- */
  "#501:",
  "\"1sided OP\" {",
      "\tint     \"taskid\";",
      "\tint     \"thid\";",
      "\tint     \"1sided_op_id\";",
      "\tint     \"handle\";",
      "\tint     \"tgt_thid\";",
      "\tint     \"msg_size\";",
  "};;\n\n"
};

/* ---------------------------------------------------- Global Variables ----*/
#define BLOCKDEF_LINES 155
static char *DimemasBlockDef[ BLOCKDEF_LINES ] =
{
  "\"block definition\" {   1, \"MPI_Allgather\", \"MPI\", 0, 0 };;",
  "\"block definition\" {   2, \"MPI_Allgatherv\", \"MPI\", 0, 0 };;",
  "\"block definition\" {   3, \"MPI_Allreduce\", \"MPI\", 0, 0 };;",
  "\"block definition\" {   4, \"MPI_Alltoall\", \"MPI\", 0, 0 };;",
  "\"block definition\" {   5, \"MPI_Alltoallv\", \"MPI\", 0, 0 };;",
  "\"block definition\" {   6, \"MPI_Barrier\", \"MPI\", 0, 0 };;",
  "\"block definition\" {   7, \"MPI_Bcast\", \"MPI\", 0, 0 };;",
  "\"block definition\" {   8, \"MPI_Gather\", \"MPI\", 0, 0 };;",
  "\"block definition\" {   9, \"MPI_Gatherv\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  10, \"MPI_Op_create\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  11, \"MPI_Op_free\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  12, \"MPI_Reduce_scatter\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  13, \"MPI_Reduce\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  14, \"MPI_Scan\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  15, \"MPI_Scatter\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  16, \"MPI_Scatterv\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  17, \"MPI_Attr_delete\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  18, \"MPI_Attr_get\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  19, \"MPI_Attr_put\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  20, \"MPI_Comm_create\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  21, \"MPI_Comm_dup\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  22, \"MPI_Comm_free\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  23, \"MPI_Comm_group\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  24, \"MPI_Comm_rank\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  25, \"MPI_Comm_remote_group\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  26, \"MPI_Comm_remote_size\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  27, \"MPI_Comm_size\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  28, \"MPI_Comm_split\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  29, \"MPI_Comm_test_inter\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  30, \"MPI_Comm_compare\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  31, \"MPI_Group_difference\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  32, \"MPI_Group_excl\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  33, \"MPI_Group_free\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  34, \"MPI_Group_incl\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  35, \"MPI_Group_intersection\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  36, \"MPI_Group_rank\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  37, \"MPI_Group_range_excl\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  38, \"MPI_Group_range_incl\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  39, \"MPI_Group_size\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  40, \"MPI_Group_translate_ranks\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  41, \"MPI_Group_union\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  42, \"MPI_Group_compare\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  43, \"MPI_Intercomm_create\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  44, \"MPI_Intercomm_merge\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  45, \"MPI_Keyval_free\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  46, \"MPI_Keyval_create\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  47, \"MPI_Abort\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  48, \"MPI_Error_class\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  49, \"MPI_Errhandler_create\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  50, \"MPI_Errhandler_free\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  51, \"MPI_Errhandler_get\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  52, \"MPI_Error_string\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  53, \"MPI_Errhandler_set\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  54, \"MPI_Finalize\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  55, \"MPI_Get_processor_name\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  56, \"MPI_Init\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  57, \"MPI_Initialized\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  58, \"MPI_Wtick\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  59, \"MPI_Wtime\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  60, \"MPI_Address\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  61, \"MPI_Bsend\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  62, \"MPI_Bsend_init\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  63, \"MPI_Buffer_attach\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  64, \"MPI_Buffer_detach\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  65, \"MPI_Cancel\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  66, \"MPI_Request_free\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  67, \"MPI_Recv_init\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  68, \"MPI_Send_init\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  69, \"MPI_Get_count\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  70, \"MPI_Get_elements\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  71, \"MPI_Ibsend\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  72, \"MPI_Iprobe\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  73, \"MPI_Irecv\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  74, \"MPI_Irsend\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  75, \"MPI_Isend\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  76, \"MPI_Issend\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  77, \"MPI_Pack\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  78, \"MPI_Pack_size\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  79, \"MPI_Probe\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  80, \"MPI_Recv\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  81, \"MPI_Rsend\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  82, \"MPI_Rsend_init\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  83, \"MPI_Send\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  84, \"MPI_Sendrecv\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  85, \"MPI_Sendrecv_replace\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  86, \"MPI_Ssend\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  87, \"MPI_Ssend_init\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  88, \"MPI_Start\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  89, \"MPI_Startall\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  90, \"MPI_Test\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  91, \"MPI_Testall\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  92, \"MPI_Testany\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  93, \"MPI_Test_cancelled\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  94, \"MPI_Testsome\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  95, \"MPI_Type_commit\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  96, \"MPI_Type_contiguous\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  97, \"MPI_Type_extent\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  98, \"MPI_Type_free\", \"MPI\", 0, 0 };;",
  "\"block definition\" {  99, \"MPI_Type_hindexed\", \"MPI\", 0, 0 };;",
  "\"block definition\" { 100, \"MPI_Type_hvector\", \"MPI\", 0, 0 };;",
  "\"block definition\" { 101, \"MPI_Type_indexed\", \"MPI\", 0, 0 };;",
  "\"block definition\" { 102, \"MPI_Type_lb\", \"MPI\", 0, 0 };;",
  "\"block definition\" { 103, \"MPI_Type_size\", \"MPI\", 0, 0 };;",
  "\"block definition\" { 104, \"MPI_Type_struct\", \"MPI\", 0, 0 };;",
  "\"block definition\" { 105, \"MPI_Type_ub\", \"MPI\", 0, 0 };;",
  "\"block definition\" { 106, \"MPI_Type_vector\", \"MPI\", 0, 0 };;",
  "\"block definition\" { 107, \"MPI_Unpack\", \"MPI\", 0, 0 };;",
  "\"block definition\" { 108, \"MPI_Wait\", \"MPI\", 0, 0 };;",
  "\"block definition\" { 109, \"MPI_Waitall\", \"MPI\", 0, 0 };;",
  "\"block definition\" { 110, \"MPI_Waitany\", \"MPI\", 0, 0 };;",
  "\"block definition\" { 111, \"MPI_Waitsome\", \"MPI\", 0, 0 };;",
  "\"block definition\" { 112, \"MPI_Cart_coords\", \"MPI\", 0, 0 };;",
  "\"block definition\" { 113, \"MPI_Cart_create\", \"MPI\", 0, 0 };;",
  "\"block definition\" { 114, \"MPI_Cart_get\", \"MPI\", 0, 0 };;",
  "\"block definition\" { 115, \"MPI_Cart_map\", \"MPI\", 0, 0 };;",
  "\"block definition\" { 116, \"MPI_Cart_rank\", \"MPI\", 0, 0 };;",
  "\"block definition\" { 117, \"MPI_Cart_shift\", \"MPI\", 0, 0 };;",
  "\"block definition\" { 118, \"MPI_Cart_sub\", \"MPI\", 0, 0 };;",
  "\"block definition\" { 119, \"MPI_Cartdim_get\", \"MPI\", 0, 0 };;",
  "\"block definition\" { 120, \"MPI_Dims_create\", \"MPI\", 0, 0 };;",
  "\"block definition\" { 121, \"MPI_Graph_get\", \"MPI\", 0, 0 };;",
  "\"block definition\" { 122, \"MPI_Graph_map\", \"MPI\", 0, 0 };;",
  "\"block definition\" { 123, \"MPI_Graph_neighbors\", \"MPI\", 0, 0 };;",
  "\"block definition\" { 124, \"MPI_Graph_create\", \"MPI\", 0, 0 };;",
  "\"block definition\" { 125, \"MPI_Graphdims_get\", \"MPI\", 0, 0 };;",
  "\"block definition\" { 126, \"MPI_Graph_neighbors_count\", \"MPI\", 0, 0 };;",
  "\"block definition\" { 127, \"MPI_Topo_test\", \"MPI\", 0, 0 };;",
  "\"block definition\" { 128, \"TRACE_ON\", \"DimemasTrace_API\", 0, 0 };;",
  "\"block definition\" { 129, \"I/O Read\", \"I/O\", 0, 0 };;",
  "\"block definition\" { 130, \"I/O Write\", \"I/O\", 0, 0 };;",
  "\"block definition\" { 131, \"I/O Call\", \"I/O\", 0, 0 };;",
  "\"block definition\" { 132, \"MPI_Win_create\", \"MPI\", 0, 0 };;",
  "\"block definition\" { 133, \"MPI_Win_free\", \"MPI\", 0, 0 };;",
  "\"block definition\" { 134, \"MPI_Put\", \"MPI\", 0, 0 };;",
  "\"block definition\" { 135, \"MPI_Get\", \"MPI\", 0, 0 };;",
  "\"block definition\" { 136, \"MPI_Accumulate\", \"MPI\", 0, 0 };;",
  "\"block definition\" { 137, \"MPI_Win_fence\", \"MPI\", 0, 0 };;",
  "\"block definition\" { 138, \"MPI_Win_complete\", \"MPI\", 0, 0 };;",
  "\"block definition\" { 139, \"MPI_Win_start\", \"MPI\", 0, 0 };;",
  "\"block definition\" { 140, \"MPI_Win_post\", \"MPI\", 0, 0 };;",
  "\"block definition\" { 141, \"MPI_Win_wait\", \"MPI\", 0, 0 };;",
  "\"block definition\" { 142, \"MPI_Win_test\", \"MPI\", 0, 0 };;",
  "\"block definition\" { 143, \"MPI_Win_lock\", \"MPI\", 0, 0 };;",
  "\"block definition\" { 144, \"MPI_Win_unlock\", \"MPI\", 0, 0 };;",
  "\"block definition\" { 145, \"MPI_Init_thread\", \"MPI\", 0, 0 };;",
  "\"block definition\" { 146, \"LAPI_Init\", \"LAPI\", 0, 0 };;",
  "\"block definition\" { 147, \"LAPI_Term\", \"LAPI\", 0, 0 };;",
  "\"block definition\" { 148, \"LAPI_Put\", \"LAPI\", 0, 0 };;",
  "\"block definition\" { 149, \"LAPI_Get\", \"LAPI\", 0, 0 };;",
  "\"block definition\" { 150, \"LAPI_Fence\", \"LAPI\", 0, 0 };;",
  "\"block definition\" { 151, \"LAPI_Gfence\", \"LAPI\", 0, 0 };;",
  "\"block definition\" { 152, \"LAPI_Address_init\", \"LAPI\", 0, 0 };;",
  "\"block definition\" { 153, \"LAPI_Amsend\", \"LAPI\", 0, 0 };;",
  "\"block definition\" { 154, \"LAPI_Rmw\", \"LAPI\", 0, 0 };;",
  "\"block definition\" { 155, \"LAPI_Waitcntr\", \"LAPI\", 0, 0 };"
};

/* ---------------------------------------------------- Global Variables ----*/
#define COLLECTIVEDEF_LINES   14
static char *DimemasCollectiveDef[ COLLECTIVEDEF_LINES ] =
{
  "\"global OP definition\" { 0, \"MPI_Barrier\" };;",
  "\"global OP definition\" { 1, \"MPI_Bcast\" };;",
  "\"global OP definition\" { 2, \"MPI_Gather\" };;",
  "\"global OP definition\" { 3, \"MPI_Gatherv\" };;",
  "\"global OP definition\" { 4, \"MPI_Scatter\" };;",
  "\"global OP definition\" { 5, \"MPI_Scatterv\" };;",
  "\"global OP definition\" { 6, \"MPI_Allgather\" };;",
  "\"global OP definition\" { 7, \"MPI_Allgatherv\" };;",
  "\"global OP definition\" { 8, \"MPI_Alltoall\" };;",
  "\"global OP definition\" { 9, \"MPI_Alltoallv\" };;",
  "\"global OP definition\" {10, \"MPI_Reduce\" };;",
  "\"global OP definition\" {11, \"MPI_Allreduce\" };;",
  "\"global OP definition\" {12, \"MPI_Reduce_Scatter\" };;",
  "\"global OP definition\" {13, \"MPI_Scan\" };;"
};

/******************************************************************************
 **      Function name : DimemasHeader_WriteHeader
 **
 **      Description :
 ******************************************************************************/

int DimemasHeader_WriteHeader( FILE *fd )
{
  int ii;

  ASSERT( fd != NULL );

  if (fprintf( fd, "\n" ) < 0)
    return -1;

  for (ii= 0; ii< DIMEMASSDDF_LINES; ii++)
  {
    if (fprintf( fd, "%s\n", DimemasSDDF[ ii ] ) < 0)
      return -1;
  }

  if (fprintf( fd, "\n" ) < 0)
    return -1;

  for (ii= 0; ii< BLOCKDEF_LINES; ii++)
  {
    if (fprintf( fd, "%s\n", DimemasBlockDef[ ii ]) < 0)
      return -1;
  }

  if (fprintf( fd, "\n" ) < 0)
    return -1;

  for (ii= 0; ii< COLLECTIVEDEF_LINES; ii++)
  {
    if (fprintf( fd, "%s\n", DimemasCollectiveDef[ ii ]) < 0)
      return -1;
  }

  if (fprintf( fd, "\n" ) < 0)
    return -1;

  return 1;
}


/******************************************************************************
 **      Function name : DimemasHeader_WriteHeader_withAdditionalBlocks
 **
 **      Description :
 ******************************************************************************/

int DimemasHeader_WriteHeader_withAdditionalBlocks( FILE *fd, char **labels, int *values, int size )
{
  int ii;

  ASSERT( fd != NULL );

  if (fprintf( fd, "\n" ) < 0)
    return -1;

  for (ii= 0; ii< DIMEMASSDDF_LINES; ii++)
  {
    if (fprintf( fd, "%s\n", DimemasSDDF[ ii ] ) < 0)
      return -1;
  }

  if (fprintf( fd, "\n" ) < 0)
    return -1;


  for (ii = 0; ii < size; ii++)
  {
    if(values[ii] != 0)
       if (fprintf( fd, "\"block definition\" { %d, \"%s\", \"Cluster\", 0, 0 };;\n", values[ii] + BASE_CLUSTER_BLOCK, labels[ii]) < 0)
         return -1;
  }

  if (fprintf( fd, "\n" ) < 0)
    return -1;


  for (ii= 0; ii< BLOCKDEF_LINES; ii++)
  {
    if (fprintf( fd, "%s\n", DimemasBlockDef[ ii ]) < 0)
      return -1;
  }

  if (fprintf( fd, "\n" ) < 0)
    return -1;

  for (ii= 0; ii< COLLECTIVEDEF_LINES; ii++)
  {
    if (fprintf( fd, "%s\n", DimemasCollectiveDef[ ii ]) < 0)
      return -1;
  }

  if (fprintf( fd, "\n" ) < 0)
    return -1;

  return 1;
}

/******************************************************************************
 **      Function name : Dimemas_NX_Generic_Send
 **
 **      Description :
 ******************************************************************************/

int Dimemas_NX_Generic_Send( FILE *fd,
                             int task,   int thread,
                             int task_r, int thread_r, /* receiver */
                             int commid,
                             int size, long64_t tag,
                             int synchronism )
{
#ifdef NEW_DIMEMAS_TRACE
  #define NX_GENERIC_SEND_STRING "2:%d:%d:%d:%d:%d:%lld:%d:%d\n"
#else
  #define NX_GENERIC_SEND_STRING "\"NX send\" { %d, %d, %d, %d, %lld, %d, %d };;\n"
#endif
  return fprintf(fd,
                 NX_GENERIC_SEND_STRING,
                 task, thread, task_r, thread_r, size, tag, commid, synchronism);
}

/******************************************************************************
 **      Function name : Dimemas_NX_Send
 **
 **      Description :
 ******************************************************************************/

int Dimemas_NX_Send( FILE *fd,
                     int task,   int thread,
                     int task_r, int thread_r, /* sender */
                     int commid,
                     int size, long64_t tag )
{
  /* synchronism: NO immediate + NO rendezvous = 0 */

#ifdef NEW_DIMEMAS_TRACE
  #define NX_SEND_STRING "2:%d:%d:%d:%d:%d:%lld:%d:0\n"
#else
  #define NX_SEND_STRING "\"NX send\" { %d, %d, %d, %d, %lld, %d, 0 };;\n"
#endif

  return fprintf(fd, NX_SEND_STRING, task, thread, task_r, thread_r, size, tag, commid);
}



/******************************************************************************
 **      Function name : Dimemas_NX_Immediate_Send
 **
 **      Description :
 ******************************************************************************/

int Dimemas_NX_ImmediateSend( FILE *fd,
                              int task,   int thread,
                              int task_r, int thread_r, /* receiver */
                              int commid,
                              int size, long64_t tag )
{
  /* synchronism: immediate + NO rendezvous = 2 */

#ifdef NEW_DIMEMAS_TRACE
  #define NX_ISEND_STRING "2:%d:%d:%d:%d:%d:%lld:%d:2\n"
#else
  #define NX_ISEND_STRING "\"NX send\" { %d, %d, %d, %d, %lld, %d, 2 };;\n"
#endif

  return fprintf(fd, NX_ISEND_STRING, task, thread, task_r, thread_r, size, tag, commid);
}



/******************************************************************************
 **      Function name : Dimemas_NX_BlockingSend
 **
 **      Description :
 ******************************************************************************/

int Dimemas_NX_BlockingSend( FILE *fd,
                             int task,   int thread,
                             int task_r, int thread_r, /* receiver */
                             int commid,
                             int size, long64_t tag )
{
  /* synchronism: NO immediate + rendezvous = 1 */

#ifdef NEW_DIMEMAS_TRACE
  #define NX_BSEND_STRING "2:%d:%d:%d:%d:%d:%lld:%d:1\n"
#else
  #define NX_BSEND_STRING "\"NX send\" { %d, %d, %d, %d, %lld, %d, 1 };;\n"
#endif

  return fprintf(fd, NX_BSEND_STRING, task, thread, task_r, thread_r, size, tag, commid);
}

/******************************************************************************
 **      Function name : Dimemas_NX_Generic_Recv
 **
 **      Description :
 ******************************************************************************/

int Dimemas_NX_Generic_Recv( FILE *fd,
                             int task,   int thread,
                             int task_s, int thread_s, /* sender */
                             int commid,
                             int size, long64_t tag,
                             int type )
{

#ifdef NEW_DIMEMAS_TRACE
  #define NX_GENERIC_RECV_STRING "3:%d:%d:%d:%d:%d:%lld:%d:%d\n"
#else
  #define NX_GENERIC_RECV_STRING "\"NX recv\" { %d, %d, %d, %d, %lld, %d, %d };;\n"
#endif

  return fprintf(fd,
                 NX_GENERIC_RECV_STRING,
                 task, thread, task_s, thread_s, size, tag, commid, type);
}

/******************************************************************************
 **      Function name : Dimemas_NX_Recv
 **
 **      Description :
 ******************************************************************************/

int Dimemas_NX_Recv( FILE *fd,
                     int task,   int thread,
                     int task_s, int thread_s, /* sender */
                     int commid,
                     int size, long64_t tag )
{

#ifdef NEW_DIMEMAS_TRACE
  #define NX_RECV_STRING "3:%d:%d:%d:%d:%d:%lld:%d:0\n"
#else
  #define NX_RECV_STRING "\"NX recv\" { %d, %d, %d, %d, %lld, %d, 0 };;\n"
#endif

  return fprintf(fd, NX_RECV_STRING, task, thread, task_s, thread_s, size, tag, commid);
}

/******************************************************************************
 **      Function name : Dimemas_NX_Irecv
 **
 **      Description :
 ******************************************************************************/

int Dimemas_NX_Irecv( FILE *fd,
                      int task,   int thread,
                      int task_s, int thread_s, /* sender */
                      int commid,
                      int size, long64_t tag )
{

#ifdef NEW_DIMEMAS_TRACE
  #define NX_IRECV_STRING "3:%d:%d:%d:%d:%d:%lld:%d:1\n"
#else
  #define NX_IRECV_STRING "\"NX recv\" { %d, %d, %d, %d, %lld, %d, 1 };;\n"
#endif

  return fprintf(fd, NX_IRECV_STRING, task, thread, task_s, thread_s, size, tag, commid);
}

/******************************************************************************
 **      Function name : Dimemas_NX_Wait
 **
 **      Description :
 ******************************************************************************/

int Dimemas_NX_Wait( FILE *fd,
                     int task,   int thread,
                     int task_s, int thread_s, /* sender */
                     int commid,
                     int size, long64_t tag )
{
#ifdef NEW_DIMEMAS_TRACE
  #define NX_WAIT_STRING "3:%d:%d:%d:%d:%d:%lld:%d:2\n"
#else
  #define NX_WAIT_STRING "\"NX recv\" { %d, %d, %d, %d, %lld, %d, 2 };;\n"
#endif
  return fprintf(fd, NX_WAIT_STRING, task, thread, task_s, thread_s, size, tag, commid);
}

/******************************************************************************
 **      Function name : Dimemas_CPU_Burst
 **
 **      Description :
 ******************************************************************************/

int Dimemas_CPU_Burst( FILE *fd,
                       int task, int thread,
                       double burst_time )
{

#ifdef NEW_DIMEMAS_TRACE

  #ifdef NANOSECOND_TRACE
    #define CPU_BURST_STRING "1:%d:%d:%.9f\n"
  #else
    #define CPU_BURST_STRING "1:%d:%d:%.6f\n"
  #endif

#else

  #ifdef NANOSECOND_TRACE
    #define CPU_BURST_STRING "\"CPU burst\" { %d, %d, %.9f };;\n"
  #else
    #define CPU_BURST_STRING "\"CPU burst\" { %d, %d, %.6f };;\n"
  #endif

#endif

  return fprintf(fd, CPU_BURST_STRING, task, thread, burst_time);
}

/******************************************************************************
 **      Function name : Dimemas_CPU_Burst
 **
 **      Description :
 ******************************************************************************/

int Dimemas_Communicator_Definition( FILE *fd,
                                     long long commid,
                                     int Ntasks,
                                     int *TaskList )
{
  int ii;

#ifdef NEW_DIMEMAS_TRACE
  #define COMMUNICATOR_STRING "d:1:%lld:%d"

  if ( fprintf( fd, COMMUNICATOR_STRING, commid, Ntasks) < 0)
    return -1;

  for (ii= 0; ii< Ntasks; ii++)
  {
    if (fprintf( fd, ":%d", TaskList[ ii ] ) < 0)
      return -1;
  }

  if (fprintf(fd, "\n") < 0)
    return -1;

#else
  #define COMMUNICATOR_STRING "\"communicator definition\" { %lld, %d, [%d] { "

  if ( fprintf( fd, COMMUNICATOR_STRING, commid, Ntasks, Ntasks ) < 0)
    return -1;

  for (ii= 0; ii< Ntasks-1; ii++)
  {
    if (fprintf( fd, "%d,", TaskList[ ii ] ) < 0)
      return -1;
  }

  if (fprintf( fd, "%d }};;\n", TaskList[ Ntasks-1 ] ) < 0)
    return -1;

#endif

  return 1;
}

/******************************************************************************
 **      Function name : Dimemas_User_Event
 **
 **      Description :
 ******************************************************************************/

int Dimemas_User_Event( FILE *fd,
                        int task, int thread,
                        long64_t type, long64_t value )
{
#ifdef NEW_DIMEMAS_TRACE
  #define USER_EVENT_STRING "20:%d:%d:%lld:%lld\n"
#else
  #define USER_EVENT_STRING "\"user event\" { %d, %d, %lld, %lld };;\n"
#endif

  return fprintf(fd, USER_EVENT_STRING, task, thread, type, value);
}

/******************************************************************************
 **      Function name : Dimemas_User_EventType_Definition
 **
 **      Description :
 ******************************************************************************/

int Dimemas_User_EventType_Definition( FILE *fd,
                                       long64_t type,
                                       char *Label,
                                       int color )
{
  #define USER_EVENT_TYPE_DEF_STRING "\"user event type definition\" { %lld, \"%s\", %d };;\n"
  ASSERT( Label != NULL );

  return fprintf(fd, USER_EVENT_TYPE_DEF_STRING, type, Label, color);
}

/******************************************************************************
 **      Function name : Dimemas_User_EventValue_Definition
 **
 **      Description :
 ******************************************************************************/

int Dimemas_User_EventValue_Definition( FILE *fd,
                                        long64_t type,
                                        long64_t value,
                                        char *Label )
{
  #define USER_EVENT_VALUE_DEF_STRING "\"user event value definition\" { %lld, %lld, \"%s\" };;\n"
  ASSERT( Label != NULL );

  return fprintf(fd, USER_EVENT_VALUE_DEF_STRING, type, value, Label);
}

/******************************************************************************
 **      Function name : Dimemas_Block_Definition
 **
 **      Description :
 ******************************************************************************/

int Dimemas_Block_Definition( FILE *fd,
                              long64_t block,
                              const char *Label )
{
  #define BLOCK_DEFINITION_STRING "\"block definition\" { %lld, \"%s\", \" \", 0, 0 };;\n"
  ASSERT( Label != NULL );

  return fprintf(fd, BLOCK_DEFINITION_STRING, block, Label);
}

/******************************************************************************
 **      Function name : Dimemas_Block_Begin
 **
 **      Description :
 ******************************************************************************/

#ifndef NEW_DIMEMAS_TRACE
int Dimemas_Block_Begin( FILE *fd,
                         int task, int thread,
                         long64_t block )
{
  #define BLOCK_BEGIN_STRING "\"block begin\" { %d, %d, %lld };;\n"
  return fprintf(fd, BLOCK_BEGIN_STRING, task, thread, block);
}

#else
int Dimemas_Block_Begin( FILE *fd,
                         int task, int thread,
                         long64_t type, long64_t value )
{
  #define BLOCK_BEGIN_STRING "20:%d:%d:%lld:%lld\n"

  return fprintf(fd, BLOCK_BEGIN_STRING, task, thread, type, value);
}
#endif

/******************************************************************************
 **      Function name : Dimemas_Block_End
 **
 **      Description :
 ******************************************************************************/

int Dimemas_Block_End( FILE *fd,
                       int task, int thread,
                       long64_t block )
{
#ifdef NEW_DIMEMAS_TRACE
  #define BLOCK_END_STRING "20:%d:%d:%lld:0\n"
#else
  #define BLOCK_END_STRING "\"block end\" { %d, %d, %lld };;\n"
#endif
  return fprintf(fd, BLOCK_END_STRING, task, thread, block );
}

/******************************************************************************
 **      Function name : Dimemas_Global_OP
 **
 **      Description :
 ******************************************************************************/

int Dimemas_Global_OP( FILE *fd,
                       int task, int thread,
                       int opid, int commid,
                       int root_rank, int root_thd,
                       long64_t sendsize, long64_t recvsize )
{
#ifdef NEW_DIMEMAS_TRACE
  #define GLOBAL_OP_STRING "10:%d:%d:%d:%d:%d:%d:%lld:%lld\n"
#else
  #define GLOBAL_OP_STRING "\"global OP\" { %d, %d, %d, %d, %d, %d, %lld, %lld };;\n"
#endif

  return fprintf(fd, GLOBAL_OP_STRING,
                 task, thread, opid, commid,
                 root_rank, root_thd,
                 sendsize, recvsize );
}

/******************************************************************************
 **      Function name : Dimemas_NX_One_Sided
 **
 **      Description :
 ******************************************************************************/

int Dimemas_NX_One_Sided( FILE * fd,
                          int task, int thread,
                          int one_sided_opid,
                          int handle, int tgt_thid,
                          int msg_size)
{
  #define NX_ONE_SIDED_STRING "\"1sided OP\" { %d, %d, %d, %d, %d, %d };;\n"
  return fprintf( fd, NX_ONE_SIDED_STRING,
                  task, thread, one_sided_opid,
                  handle, tgt_thid, msg_size);
}

int Dimemas_NOOP ( FILE *fd, int task, int thread )
{
#ifdef NEW_DIMEMAS_TRACE
  #define NOOP_STRING "0:%d:%d\n"

  return fprintf(fd, NOOP_STRING, task, thread);
#else
  return 0;
#endif
}

