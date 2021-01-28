#ifndef _PARAVERCOLORS
#define _PARAVERCOLORS

#ifdef __cplusplus
extern "C"
{
#endif

#define MAX_NAME 512

  /* ---------------------------------------------------- Data Types ----------*/
  typedef struct
  {
    unsigned char RGB[ 3 ];
    char name[ MAX_NAME ];

  } DefaultPalette_t;

  /* Definition of each Default Palette entry */

#define PRV_IDLE_ST  0
#define PRV_IDLE_LBL "Idle"
#define PRV_IDLE_CLR \
  {                  \
    117, 195, 255    \
  }

#define PRV_RUNNING_ST  1
#define PRV_RUNNING_LBL "Running"
#define PRV_RUNNING_CLR \
  {                     \
    0, 0, 255           \
  }

#define PRV_NOT_CREATED_ST  2
#define PRV_NOT_CREATED_LBL "Not created"
#define PRV_NOT_CREATED_CLR \
  {                         \
    255, 255, 255           \
  }

#define PRV_WAITING_MESG_ST  3
#define PRV_WAITING_MESG_LBL "Waiting a message"
#define PRV_WAITING_MESG_CLR \
  {                          \
    255, 0, 0                \
  }

#define PRV_BLOCKING_SEND_ST  4
#define PRV_BLOCKING_SEND_LBL "Blocking send"
#define PRV_BLOCKING_SEND_CLR \
  {                           \
    255, 0, 174               \
  }

#define PRV_SYNC_ST  5
#define PRV_SYNC_LBL "Synchronization"
#define PRV_SYNC_CLR \
  {                  \
    179, 0, 0        \
  }

#define PRV_TEST_PROBE_ST  6
#define PRV_TEST_PROBE_LBL "Test/Probe"
#define PRV_TEST_PROBE_CLR \
  {                        \
    0, 255, 0              \
  }

#define PRV_THREAD_SCHED_ST  7
#define PRV_THREAD_SCHED_LBL "Sched. and Fork/Join"
#define PRV_THREAD_SCHED_CLR \
  {                          \
    255, 255, 0              \
  }

#define PRV_WAIT_ST  8
#define PRV_WAIT_LBL "Wait/WaitAll"
#define PRV_WAIT_CLR \
  {                  \
    235, 0, 0        \
  }

#define PRV_BLOCKED_ST  9
#define PRV_BLOCKED_LBL "Blocked"
#define PRV_BLOCKED_CLR \
  {                     \
    0, 162, 0           \
  }

#define PRV_ISEND_ST  10
#define PRV_ISEND_LBL "Immediate Send"
#define PRV_ISEND_CLR \
  {                   \
    255, 0, 255       \
  }

#define PRV_IRECV_ST  11
#define PRV_IRECV_LBL "Immediate Receive"
#define PRV_IRECV_CLR \
  {                   \
    100, 100, 177     \
  }

#define PRV_IO_ST  12
#define PRV_IO_LBL "I/O"
#define PRV_IO_CLR \
  {                \
    172, 174, 41   \
  }

#define PRV_GLOBAL_OP_ST  13
#define PRV_GLOBAL_OP_LBL "Group Communication"
#define PRV_GLOBAL_OP_CLR \
  {                       \
    255, 144, 26          \
  }

#define PRV_NOT_TRACING_ST  14
#define PRV_NOT_TRACING_LBL "Tracing Disabled"
#define PRV_NOT_TRACING_CLR \
  {                         \
    2, 255, 177             \
  }

#define PRV_OTHERS_ST  15
#define PRV_OTHERS_LBL "Others"
#define PRV_OTHERS_CLR \
  {                    \
    192, 224, 0        \
  }

#define PRV_SENDRECV_ST  16
#define PRV_SENDRECV_LBL "Send Receive"
#define PRV_SENDRECV_CLR \
  {                      \
    66, 66, 66           \
  }

#define PRV_MEMORY_TRNSF_ST  17
#define PRV_MEMORY_TRNSF_LBL "Memory transfer"
#define PRV_MEMORY_TRNSF_CLR \
  {                          \
    255, 0, 96               \
  }

#define PRV_PROFILING_ST  18
#define PRV_PROFILING_LBL "Profiling"
#define PRV_PROFILING_CLR \
  {                       \
    169, 169, 169         \
  }

#define PRV_ONLINE_ANALYSIS_ST  19
#define PRV_ONLINE_ANALYSIS_LBL "On-line analysis"
#define PRV_ONLINE_ANALYSIS_CLR \
  {                             \
    169, 0, 0                   \
  }

#define PRV_REMOTE_MEM_ACC_ST  20
#define PRV_REMOTE_MEM_ACC_LBL "Remote memory access"
#define PRV_REMOTE_MEM_ACC_CLR \
  {                            \
    0, 109, 255                \
  }

#define PRV_ATOMIC_MEM_OP_ST  21
#define PRV_ATOMIC_MEM_OP_LBL "Atomic memory operation"
#define PRV_ATOMIC_MEM_OP_CLR \
  {                           \
    200, 61, 68               \
  }

#define PRV_MEM_ORDER_OP_ST  22
#define PRV_MEM_ORDER_OP_LBL "Memory ordering operation"
#define PRV_MEM_ORDER_OP_CLR \
  {                          \
    200, 66, 0               \
  }

#define PRV_DISTRIB_LCK_ST  23
#define PRV_DISTRIB_LCK_LBL "Distributed locking"
#define PRV_DISTRIB_LCK_CLR \
  {                         \
    0, 41, 0                \
  }

#define PRV_OVERHEAD_ST  24
#define PRV_OVERHEAD_LBL "Overhead"
#define PRV_OVERHEAD_CLR \
  {                      \
    139, 121, 177        \
  }

#define PRV_ONESIDED_OP_ST  25
#define PRV_ONESIDED_OP_LBL "One Sided OP"
#define PRV_ONESIDED_OP_CLR \
  {                         \
    116, 116, 116           \
  }

#define PRV_STARTUP_LAT_ST  26
#define PRV_STARTUP_LAT_LBL "Startup Latency"
#define PRV_STARTUP_LAT_CLR \
  {                         \
    200, 50, 89             \
  }

#define PRV_WAITING_LNKS_ST  27
#define PRV_WAITING_LNKS_LBL "Waiting links"
#define PRV_WAITING_LNKS_CLR \
  {                          \
    255, 171, 98             \
  }

#define PRV_DATA_COPY_ST  28
#define PRV_DATA_COPY_LBL "Data copy"
#define PRV_DATA_COPY_CLR \
  {                       \
    0, 68, 189            \
  }

#define PRV_RTT_ST  29
#define PRV_RTT_LBL "Round Trip Time"
#define PRV_RTT_CLR \
  {                 \
    52, 43, 0       \
  }

#define PRV_NOT_USED_LBL "Not used"

/* ---------------------------------------------------- Global Variables ----*/
#define DEF_NB_COLOR_STATE     49
#define PRV_DEFAULT_STATES_NUM 30
  static DefaultPalette_t ParaverDefaultPalette[ DEF_NB_COLOR_STATE ] = {
    /* 00 */ { PRV_IDLE_CLR, PRV_IDLE_LBL },
    /* 01 */ { PRV_RUNNING_CLR, PRV_RUNNING_LBL },
    /* 02 */ { PRV_NOT_CREATED_CLR, PRV_NOT_CREATED_LBL },
    /* 03 */ { PRV_WAITING_MESG_CLR, PRV_WAITING_MESG_LBL },
    /* 04 */ { PRV_BLOCKING_SEND_CLR, PRV_BLOCKING_SEND_LBL },
    /* 05 */ { PRV_SYNC_CLR, PRV_SYNC_LBL },
    /* 06 */ { PRV_TEST_PROBE_CLR, PRV_TEST_PROBE_LBL },
    /* 07 */ { PRV_THREAD_SCHED_CLR, PRV_THREAD_SCHED_LBL },
    /* 08 */ { PRV_WAIT_CLR, PRV_WAIT_LBL },
    /* 09 */ { PRV_BLOCKED_CLR, PRV_BLOCKED_LBL },
    /* 10 */ { PRV_ISEND_CLR, PRV_ISEND_LBL },
    /* 11 */ { PRV_IRECV_CLR, PRV_IRECV_LBL },
    /* 12 */ { PRV_IO_CLR, PRV_IO_LBL },
    /* 13 */ { PRV_GLOBAL_OP_CLR, PRV_GLOBAL_OP_LBL },
    /* 14 */ { PRV_NOT_TRACING_CLR, PRV_NOT_TRACING_LBL },
    /* 15 */ { PRV_OTHERS_CLR, PRV_OTHERS_LBL },
    /* 16 */ { PRV_SENDRECV_CLR, PRV_SENDRECV_LBL },
    /* 17 */ { PRV_MEMORY_TRNSF_CLR, PRV_MEMORY_TRNSF_LBL },
    /* 18 */ { PRV_PROFILING_CLR, PRV_PROFILING_LBL },
    /* 19 */ { PRV_ONLINE_ANALYSIS_CLR, PRV_ONLINE_ANALYSIS_LBL },
    /* 20 */ { PRV_REMOTE_MEM_ACC_CLR, PRV_REMOTE_MEM_ACC_LBL },
    /* 21 */ { PRV_ATOMIC_MEM_OP_CLR, PRV_ATOMIC_MEM_OP_LBL },
    /* 22 */ { PRV_MEM_ORDER_OP_CLR, PRV_MEM_ORDER_OP_LBL },
    /* 23 */ { PRV_DISTRIB_LCK_CLR, PRV_DISTRIB_LCK_LBL },
    /* 24 */ { PRV_OVERHEAD_CLR, PRV_OVERHEAD_LBL },
    /* 25 */ { PRV_ONESIDED_OP_CLR, PRV_ONESIDED_OP_LBL },
    /* 26 */ { PRV_STARTUP_LAT_CLR, PRV_STARTUP_LAT_LBL },
    /* 27 */ { PRV_WAITING_LNKS_CLR, PRV_WAITING_LNKS_LBL },
    /* 28 */ { PRV_DATA_COPY_CLR, PRV_DATA_COPY_LBL },
    /* 29 */ { PRV_RTT_CLR, PRV_RTT_LBL },
    /* 30 */ { { 255, 46, 0 }, PRV_NOT_USED_LBL },
    /* 31 */ { { 100, 216, 32 }, PRV_NOT_USED_LBL },
    /* 32 */ { { 0, 0, 112 }, PRV_NOT_USED_LBL },
    /* 33 */ { { 105, 105, 0 }, PRV_NOT_USED_LBL },
    /* 34 */ { { 132, 75, 255 }, PRV_NOT_USED_LBL },
    /* 35 */ { { 184, 232, 0 }, PRV_NOT_USED_LBL },
    /* 36 */ { { 0, 109, 112 }, PRV_NOT_USED_LBL },
    /* 37 */ { { 189, 168, 100 }, PRV_NOT_USED_LBL },
    /* 38 */ { { 132, 75, 75 }, PRV_NOT_USED_LBL },
    /* 39 */ { { 255, 75, 75 }, PRV_NOT_USED_LBL },
    /* 40 */ { { 255, 20, 0 }, PRV_NOT_USED_LBL },
    /* 41 */ { { 52, 0, 0 }, PRV_NOT_USED_LBL },
    /* 42 */ { { 0, 66, 0 }, PRV_NOT_USED_LBL },
    /* 43 */ { { 184, 132, 0 }, PRV_NOT_USED_LBL },
    /* 44 */ { { 100, 16, 32 }, PRV_NOT_USED_LBL },
    /* 45 */ { { 146, 255, 255 }, PRV_NOT_USED_LBL },
    /* 46 */ { { 0, 23, 37 }, PRV_NOT_USED_LBL },
    /* 47 */ { { 146, 0, 255 }, PRV_NOT_USED_LBL },
    /* 48 */ { { 0, 138, 119 }, PRV_NOT_USED_LBL }
  };

#ifdef __cplusplus
}
#endif

#endif /* _PARAVERCOLORS */
