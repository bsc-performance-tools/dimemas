#include "Dimemas2Prv.h"

#ifndef OLD_PRV_FORMAT

struct t_block_dimemas2prv block_dimemas2prv[NUM_MPI_ELEMENTS]={
  {MPITYPE_COLLECTIVE, MPI_ALLGATHER_VAL},             /*   1 */
  {MPITYPE_COLLECTIVE, MPI_ALLGATHERV_VAL},            /*   2 */
  {MPITYPE_COLLECTIVE, MPI_ALLREDUCE_VAL},             /*   3 */
  {MPITYPE_COLLECTIVE, MPI_ALLTOALL_VAL},              /*   4 */
  {MPITYPE_COLLECTIVE, MPI_ALLTOALLV_VAL},             /*   5 */
  {MPITYPE_COLLECTIVE, MPI_BARRIER_VAL},               /*   6 */
  {MPITYPE_COLLECTIVE, MPI_BCAST_VAL},                 /*   7 */
  {MPITYPE_COLLECTIVE, MPI_GATHER_VAL},                /*   8 */
  {MPITYPE_COLLECTIVE, MPI_GATHERV_VAL},               /*   9 */
  {MPITYPE_OTHER,      MPI_OP_CREATE_VAL},             /*  10 */
  {MPITYPE_OTHER,      MPI_OP_FREE_VAL},               /*  11 */
  {MPITYPE_COLLECTIVE, MPI_REDUCE_SCATTER_VAL},        /*  12 */
  {MPITYPE_COLLECTIVE, MPI_REDUCE_VAL},                /*  13 */
  {MPITYPE_COLLECTIVE, MPI_SCAN_VAL},                  /*  14 */
  {MPITYPE_COLLECTIVE, MPI_SCATTER_VAL},               /*  15 */
  {MPITYPE_COLLECTIVE, MPI_SCATTERV_VAL},              /*  16 */
  {MPITYPE_OTHER,      MPI_ATTR_DELETE_VAL},           /*  17 */
  {MPITYPE_OTHER,      MPI_ATTR_GET_VAL},              /*  18 */
  {MPITYPE_OTHER,      MPI_ATTR_PUT_VAL},              /*  19 */
  {MPITYPE_COMM,       MPI_COMM_CREATE_VAL},           /*  20 */
  {MPITYPE_COMM,       MPI_COMM_DUP_VAL},              /*  21 */
  {MPITYPE_COMM,       MPI_COMM_FREE_VAL},             /*  22 */
  {MPITYPE_COMM,       MPI_COMM_GROUP_VAL},            /*  23 */
  {MPITYPE_COMM,       MPI_COMM_RANK_VAL},             /*  24 */
  {MPITYPE_COMM,       MPI_COMM_REMOTE_GROUP_VAL},     /*  25 */
  {MPITYPE_COMM,       MPI_COMM_REMOTE_SIZE_VAL},      /*  26 */
  {MPITYPE_COMM,       MPI_COMM_SIZE_VAL},             /*  27 */
  {MPITYPE_COMM,       MPI_COMM_SPLIT_VAL},            /*  28 */
  {MPITYPE_COMM,       MPI_COMM_TEST_INTER_VAL},       /*  29 */
  {MPITYPE_COMM,       MPI_COMM_COMPARE_VAL},          /*  30 */
  {MPITYPE_GROUP,      MPI_GROUP_DIFFERENCE_VAL},      /*  31 */
  {MPITYPE_GROUP,      MPI_GROUP_EXCL_VAL},            /*  32 */
  {MPITYPE_GROUP,      MPI_GROUP_FREE_VAL},            /*  33 */
  {MPITYPE_GROUP,      MPI_GROUP_INCL_VAL},            /*  34 */
  {MPITYPE_GROUP,      MPI_GROUP_INTERSECTION_VAL},    /*  35 */
  {MPITYPE_GROUP,      MPI_GROUP_RANK_VAL},            /*  36 */
  {MPITYPE_GROUP,      MPI_GROUP_RANGE_EXCL_VAL},      /*  37 */
  {MPITYPE_GROUP,      MPI_GROUP_RANGE_INCL_VAL},      /*  38 */
  {MPITYPE_GROUP,      MPI_GROUP_SIZE_VAL},            /*  39 */
  {MPITYPE_GROUP,      MPI_GROUP_TRANSLATE_RANKS_VAL}, /*  40 */
  {MPITYPE_GROUP,      MPI_GROUP_UNION_VAL},           /*  41 */
  {MPITYPE_GROUP,      MPI_GROUP_COMPARE_VAL},         /*  42 */
  {MPITYPE_COMM,       MPI_INTERCOMM_CREATE_VAL},      /*  43 */
  {MPITYPE_COMM,       MPI_INTERCOMM_MERGE_VAL},       /*  44 */
  {MPITYPE_OTHER,      MPI_KEYVAL_FREE_VAL},           /*  45 */
  {MPITYPE_OTHER,      MPI_KEYVAL_CREATE_VAL},         /*  46 */
  {MPITYPE_OTHER,      MPI_ABORT_VAL},                 /*  47 */
  {MPITYPE_OTHER,      MPI_ERROR_CLASS_VAL},           /*  48 */
  {MPITYPE_OTHER,      MPI_ERRHANDLER_CREATE_VAL},     /*  49 */
  {MPITYPE_OTHER,      MPI_ERRHANDLER_FREE_VAL},       /*  50 */
  {MPITYPE_OTHER,      MPI_ERRHANDLER_GET_VAL},        /*  51 */
  {MPITYPE_OTHER,      MPI_ERROR_STRING_VAL},          /*  52 */
  {MPITYPE_OTHER,      MPI_ERRHANDLER_SET_VAL},        /*  53 */
  {MPITYPE_OTHER,      MPI_FINALIZE_VAL},              /*  54 */
  {MPITYPE_OTHER,      MPI_GET_PROCESSOR_NAME_VAL},    /*  55 */
  {MPITYPE_OTHER,      MPI_INIT_VAL},                  /*  56 */
  {MPITYPE_OTHER,      MPI_INITIALIZED_VAL},           /*  57 */
  {MPITYPE_OTHER,      MPI_WTICK_VAL},                 /*  58 */
  {MPITYPE_OTHER,      MPI_WTIME_VAL},                 /*  59 */
  {MPITYPE_OTHER,      MPI_ADDRESS_VAL},               /*  60 */
  {MPITYPE_PTOP,       MPI_BSEND_VAL},                 /*  61 */
  {MPITYPE_PTOP,       MPI_BSEND_INIT_VAL},            /*  62 */
  {MPITYPE_OTHER,      MPI_BUFFER_ATTACH_VAL},         /*  63 */
  {MPITYPE_OTHER,      MPI_BUFFER_DETACH_VAL},         /*  64 */
  {MPITYPE_PTOP,       MPI_CANCEL_VAL},                /*  65 */
  {MPITYPE_OTHER,      MPI_REQUEST_FREE_VAL},          /*  66 */
  {MPITYPE_PTOP,       MPI_RECV_INIT_VAL},             /*  67 */
  {MPITYPE_PTOP,       MPI_SEND_INIT_VAL},             /*  68 */
  {MPITYPE_OTHER,      MPI_GET_COUNT_VAL},             /*  69 */
  {MPITYPE_OTHER,      MPI_GET_ELEMENTS_VAL},          /*  70 */
  {MPITYPE_PTOP,       MPI_IBSEND_VAL},                /*  71 */
  {MPITYPE_PTOP,       MPI_IPROBE_VAL},                /*  72 */
  {MPITYPE_PTOP,       MPI_IRECV_VAL},                 /*  73 */
  {MPITYPE_PTOP,       MPI_IRSEND_VAL},                /*  74 */
  {MPITYPE_PTOP,       MPI_ISEND_VAL},                 /*  75 */
  {MPITYPE_PTOP,       MPI_ISSEND_VAL},                /*  76 */
  {MPITYPE_OTHER,      MPI_PACK_VAL},                  /*  77 */
  {MPITYPE_OTHER,      MPI_PACK_SIZE_VAL},             /*  78 */
  {MPITYPE_PTOP,       MPI_PROBE_VAL},                 /*  79 */
  {MPITYPE_PTOP,       MPI_RECV_VAL},                  /*  80 */
  {MPITYPE_PTOP,       MPI_RSEND_VAL},                 /*  81 */
  {MPITYPE_PTOP,       MPI_RSEND_INIT_VAL},            /*  82 */
  {MPITYPE_PTOP,       MPI_SEND_VAL},                  /*  83 */
  {MPITYPE_PTOP,       MPI_SENDRECV_VAL},              /*  84 */
  {MPITYPE_PTOP,       MPI_SENDRECV_REPLACE_VAL},      /*  85 */
  {MPITYPE_PTOP,       MPI_SSEND_VAL},                 /*  86 */
  {MPITYPE_PTOP,       MPI_SSEND_INIT_VAL},            /*  87 */
  {MPITYPE_OTHER,      MPI_START_VAL},                 /*  88 */
  {MPITYPE_OTHER,      MPI_STARTALL_VAL},              /*  89 */
  {MPITYPE_PTOP,       MPI_TEST_VAL},                  /*  90 */
  {MPITYPE_PTOP,       MPI_TESTALL_VAL},               /*  91 */
  {MPITYPE_PTOP,       MPI_TESTANY_VAL},               /*  92 */
  {MPITYPE_PTOP,       MPI_TEST_CANCELLED_VAL},        /*  93 */
  {MPITYPE_PTOP,       MPI_TESTSOME_VAL},              /*  94 */
  {MPITYPE_TYPE,       MPI_TYPE_COMMIT_VAL},           /*  95 */
  {MPITYPE_TYPE,       MPI_TYPE_CONTIGUOUS_VAL},       /*  96 */
  {MPITYPE_TYPE,       MPI_TYPE_EXTENT_VAL},           /*  97 */
  {MPITYPE_TYPE,       MPI_TYPE_FREE_VAL},             /*  98 */
  {MPITYPE_TYPE,       MPI_TYPE_HINDEXED_VAL},         /*  99 */
  {MPITYPE_TYPE,       MPI_TYPE_HVECTOR_VAL},          /* 100 */
  {MPITYPE_TYPE,       MPI_TYPE_INDEXED_VAL},          /* 101 */
  {MPITYPE_TYPE,       MPI_TYPE_LB_VAL},               /* 102 */
  {MPITYPE_TYPE,       MPI_TYPE_SIZE_VAL},             /* 103 */
  {MPITYPE_TYPE,       MPI_TYPE_STRUCT_VAL},           /* 104 */
  {MPITYPE_TYPE,       MPI_TYPE_UB_VAL},               /* 105 */
  {MPITYPE_TYPE,       MPI_TYPE_VECTOR_VAL},           /* 106 */
  {MPITYPE_OTHER,      MPI_UNPACK_VAL},                /* 107 */
  {MPITYPE_PTOP,       MPI_WAIT_VAL},                  /* 108 */
  {MPITYPE_PTOP,       MPI_WAITALL_VAL},               /* 109 */
  {MPITYPE_PTOP,       MPI_WAITANY_VAL},               /* 110 */
  {MPITYPE_PTOP,       MPI_WAITSOME_VAL},              /* 111 */
  {MPITYPE_TOPOLOGIES, MPI_CART_COORDS_VAL},           /* 112 */
  {MPITYPE_TOPOLOGIES, MPI_CART_CREATE_VAL},           /* 113 */
  {MPITYPE_TOPOLOGIES, MPI_CART_GET_VAL},              /* 114 */
  {MPITYPE_TOPOLOGIES, MPI_CART_MAP_VAL},              /* 115 */
  {MPITYPE_TOPOLOGIES, MPI_CART_RANK_VAL},             /* 116 */
  {MPITYPE_TOPOLOGIES, MPI_CART_SHIFT_VAL},            /* 117 */
  {MPITYPE_TOPOLOGIES, MPI_CART_SUB_VAL},              /* 118 */
  {MPITYPE_TOPOLOGIES, MPI_CARTDIM_GET_VAL},           /* 119 */
  {MPITYPE_TOPOLOGIES, MPI_DIMS_CREATE_VAL},           /* 120 */
  {MPITYPE_TOPOLOGIES, MPI_GRAPH_GET_VAL},             /* 121 */
  {MPITYPE_TOPOLOGIES, MPI_GRAPH_MAP_VAL},             /* 122 */
  {MPITYPE_TOPOLOGIES, MPI_GRAPH_NEIGHBORS_VAL},       /* 123 */
  {MPITYPE_TOPOLOGIES, MPI_GRAPH_CREATE_VAL},          /* 124 */
  {MPITYPE_TOPOLOGIES, MPI_GRAPHDIMS_GET_VAL},         /* 125 */
  {MPITYPE_TOPOLOGIES, MPI_GRAPH_NEIGHBORS_COUNT_VAL}, /* 126 */
  {MPITYPE_TOPOLOGIES, MPI_TOPO_TEST_VAL},             /* 127 */
  {USER_BLOCK, 128},                                   /* 128 = TRACE_ON */
  {IO_READ_EV,  1},                                    /* 129 - 1/0 = Read */
  {IO_WRITE_EV, 1},                                    /* 130 - 1/0 = Write */
  {IO_EV, 1},                                          /* 131 - 1/0 = Call */

  {MPITYPE_RMA, MPI_WIN_CREATE_VAL},                   /* 132 */
  {MPITYPE_RMA, MPI_WIN_FREE_VAL},                     /* 133 */
  {MPITYPE_RMA, MPI_PUT_VAL},                          /* 134 */
  {MPITYPE_RMA, MPI_GET_VAL},                          /* 135 */
  {MPITYPE_RMA, MPI_ACCUMULATE_VAL},                   /* 136 */
  {MPITYPE_RMA, MPI_WIN_FENCE_VAL},                    /* 137 */
  {MPITYPE_RMA, MPI_WIN_COMPLETE_VAL},                 /* 138 */
  {MPITYPE_RMA, MPI_WIN_START_VAL},                    /* 139 */
  {MPITYPE_RMA, MPI_WIN_POST_VAL},                     /* 140 */
  {MPITYPE_RMA, MPI_WIN_WAIT_VAL},                     /* 141 */
  {MPITYPE_RMA, MPI_WIN_TEST_VAL},                     /* 142 */
  {MPITYPE_RMA, MPI_WIN_LOCK_VAL},                     /* 143 */
  {MPITYPE_RMA, MPI_WIN_UNLOCK_VAL},                   /* 144 */

  {MPITYPE_OTHER, MPI_INIT_THREAD_VAL},                /* 145 */
  
  {LAPI_EV, LAPI_INIT_VAL},                            /* 146 */
  {LAPI_EV, LAPI_TERM_VAL},                            /* 147 */
  {LAPI_EV, LAPI_PUT_VAL},                             /* 148 */
  {LAPI_EV, LAPI_GET_VAL},                             /* 149 */
  {LAPI_EV, LAPI_FENCE_VAL},                           /* 150 */
  {LAPI_EV, LAPI_GFENCE_VAL},                          /* 151 */
  {LAPI_EV, LAPI_ADDRESS_INIT_VAL},                    /* 152 */
  {LAPI_EV, LAPI_AMSEND_VAL},                          /* 153 */
  {LAPI_EV, LAPI_RMW_VAL},                             /* 154 */
  {LAPI_EV, LAPI_WAITCNTR_VAL}                         /* 155 */
};


/* Dels 14, de moment nomes 11 son diferents */
struct t_prv_type_info prv_block_groups[ NUM_BLOCK_GROUPS ] = {
  { MPITYPE_PTOP,        MPITYPE_PTOP_LABEL,        MPITYPE_FLAG_COLOR},
  { MPITYPE_COLLECTIVE,  MPITYPE_COLLECTIVE_LABEL,  MPITYPE_FLAG_COLOR},
  { MPITYPE_OTHER,       MPITYPE_OTHER_LABEL,       MPITYPE_FLAG_COLOR},
  { MPITYPE_RMA,         MPITYPE_RMA_LABEL,         MPITYPE_FLAG_COLOR},
/*{ MPITYPE_COMM,        MPITYPE_COMM_LABEL,        MPITYPE_FLAG_COLOR},
  { MPITYPE_GROUP,       MPITYPE_GROUP_LABEL,       MPITYPE_FLAG_COLOR},
  { MPITYPE_TOPOLOGIES,  MPITYPE_TOPOLOGIES_LABEL,  MPITYPE_FLAG_COLOR},
  { MPITYPE_TYPE,        MPITYPE_TYPE_LABEL,        MPITYPE_FLAG_COLOR},*/
  { USER_FUNCTION,       USER_FUNCTION_LABEL,       MPITYPE_FLAG_COLOR},
  { USER_CALL,           USER_CALL_LABEL,           MPITYPE_FLAG_COLOR},
  { USER_BLOCK,          USER_BLOCK_LABEL,          MPITYPE_FLAG_COLOR},
  { IO_READ_EV,          IO_READ_LABEL,             MPITYPE_FLAG_COLOR},
  { IO_WRITE_EV,         IO_WRITE_LABEL,            MPITYPE_FLAG_COLOR},
  { IO_EV,               IO_LABEL,                  MPITYPE_FLAG_COLOR},
  { CLUSTER_ID_EV,       CLUSTER_ID_LABEL,          MPITYPE_FLAG_COLOR}
};

/* Dimemas to Paraver Type/Value conversion functions */

void
Block_Dimemas2Paraver_TypeAndValue(long long  DimemasBlockId,
                                   long long* ParaverType,
                                   long long* ParaverValue)
{
  if ((DimemasBlockId >= MIN_TIPUS_MPI_TRF) && 
      (DimemasBlockId <= MAX_TIPUS_MPI_TRF))
  {
    *ParaverType  = block_dimemas2prv[DimemasBlockId - MIN_TIPUS_MPI_TRF].tipus_prv;
    *ParaverValue = block_dimemas2prv[DimemasBlockId-MIN_TIPUS_MPI_TRF].valor_prv;
  }
  else if (DimemasBlockId < BASE_CLUSTER_BLOCK)
  {
    *ParaverType  = USER_FUNCTION;
    *ParaverValue = DimemasBlockId - BASE_USERFUNCTION;
  }
  else if (DimemasBlockId < BASE_USERCALL)
  {
    *ParaverType  = CLUSTER_ID_EV;
    *ParaverValue = DimemasBlockId - BASE_CLUSTER_BLOCK;
  }
  else if (DimemasBlockId < BASE_USERBLOCK)
  {
    *ParaverType  = USER_CALL;
    *ParaverValue = DimemasBlockId - BASE_USERCALL;
  }
  else
  { 
    *ParaverType  = USER_BLOCK;
    *ParaverValue = DimemasBlockId - BASE_USERBLOCK;
  }

  return;
}

long long
Block_Dimemas2Paraver_Type(long long DimemasBlockId)
{
  if ((DimemasBlockId >= MIN_TIPUS_MPI_TRF) && 
      (DimemasBlockId <= MAX_TIPUS_MPI_TRF))
  {
    return block_dimemas2prv[DimemasBlockId - MIN_TIPUS_MPI_TRF].tipus_prv;
  }
  else if (DimemasBlockId < BASE_CLUSTER_BLOCK)
  {
    return USER_FUNCTION;
  }
  else if (DimemasBlockId < BASE_USERCALL)
  {
    return CLUSTER_ID_EV;
  }
  else if (DimemasBlockId < BASE_USERBLOCK)
  {
    return USER_CALL;
  }
  else
  { 
    return USER_BLOCK;
  }
}

long long
Block_Dimemas2Paraver_Value(long long DimemasBlockId)
{
  if ((DimemasBlockId >= MIN_TIPUS_MPI_TRF) && 
      (DimemasBlockId <= MAX_TIPUS_MPI_TRF))
  {
    return block_dimemas2prv[DimemasBlockId-MIN_TIPUS_MPI_TRF].valor_prv;
  }
  else if (DimemasBlockId < BASE_CLUSTER_BLOCK)
  {
    return (DimemasBlockId - BASE_USERFUNCTION);
  }
  else if (DimemasBlockId < BASE_USERCALL)
  {
    return (DimemasBlockId - BASE_CLUSTER_BLOCK);
  }
  else if (DimemasBlockId < BASE_USERBLOCK)
  {
    return (DimemasBlockId - BASE_USERCALL);
  }
  else
  { 
    return (DimemasBlockId - BASE_USERBLOCK);
  }
}


#endif
