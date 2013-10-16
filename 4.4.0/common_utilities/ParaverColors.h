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
  char          name[ MAX_NAME ];
  
} DefaultPalette_t;

/* Definition of each Default Palette entry */

#define PRV_IDLE_ST            0
#define PRV_IDLE_LBL           "Idle"
#define PRV_IDLE_CLR           { 117, 195, 255 }

#define PRV_RUNNING_ST         1
#define PRV_RUNNING_LBL        "Running"
#define PRV_RUNNING_CLR        {   0,   0, 255 }

#define PRV_NOT_CREATED_ST     2
#define PRV_NOT_CREATED_LBL    "Not created"
#define PRV_NOT_CREATED_CLR    { 255, 255, 255 }

#define PRV_BLOCKING_RECV_ST   3
#define PRV_BLOCKING_RECV_LBL  "Waiting a message"
#define PRV_BLOCKING_RECV_CLR  { 255,   0,   0 }

#define PRV_BLOCKED_ST         4
#define PRV_BLOCKED_LBL        "Blocked"
#define PRV_BLOCKED_CLR        { 255,   0, 174 }

#define PRV_THREAD_SYNC_ST     5
#define PRV_THREAD_SYNC_LBL    "Thd. Synchr."
#define PRV_THREAD_SYNC_CLR    { 179,   0,   0 }

#define PRV_TEST_PROBE_ST      6
#define PRV_TEST_PROBE_LBL     "Test/Probe"
#define PRV_TEST_PROBE_CLR     { 0,   255,   0 }

#define PRV_THREAD_SCHED_ST    7
#define PRV_THREAD_SCHED_LBL   "Sched. and Fork/Join"
#define PRV_THREAD_SCHED_CLR   { 255, 255,   0 }

#define PRV_WAIT_ST            8
#define PRV_WAIT_LBL           "Wait/WaitAll"
#define PRV_WAIT_CLR           { 235,   0,   0 }


#define PRV_BLOCKING_SEND_ST   9
#define PRV_BLOCKING_SEND_LBL  "Blocked"
#define PRV_BLOCKING_SEND_CLR  {   0, 162,   0 }

#define PRV_ISEND_ST           10
#define PRV_ISEND_LBL          "Immediate Send"
#define PRV_ISEND_CLR          { 255,   0, 255 }

#define PRV_IRECV_ST           11
#define PRV_IRECV_LBL          "Immediate Receive"
#define PRV_IRECV_CLR          { 100, 100,  177 }

#define PRV_IO_ST              12
#define PRV_IO_LBL             "I/O"
#define PRV_IO_CLR             { 172, 174,  41 }

#define PRV_GLOBAL_OP_ST       13
#define PRV_GLOBAL_OP_LBL      "Group Communication"
#define PRV_GLOBAL_OP_CLR      { 255, 144,  26 }

#define PRV_NOT_TRACING_ST     14
#define PRV_NOT_TRACING_LBL    "Tracing Disabled"
#define PRV_NOT_TRACING_CLR    {   2, 255, 177 }

#define PRV_OVERHEAD_ST        15
#define PRV_OVERHEAD_LBL       "Overhead"
#define PRV_OVERHEAD_CLR       { 192, 224,   0 }

#define PRV_NOT_USED_LBL    "Not used"

/* ---------------------------------------------------- Global Variables ----*/
#define DEF_NB_COLOR_STATE     49
#define PRV_DEFAULT_STATES_NUM 16
static DefaultPalette_t ParaverDefaultPalette[ DEF_NB_COLOR_STATE ] =
{
/* 00 */  { PRV_IDLE_CLR,          PRV_IDLE_LBL},
/* 01 */  { PRV_RUNNING_CLR,       PRV_RUNNING_LBL       },
/* 02 */  { PRV_NOT_CREATED_CLR,   PRV_NOT_CREATED_LBL   },
/* 03 */  { PRV_BLOCKING_RECV_CLR, PRV_BLOCKING_RECV_LBL },
/* 04 */  { PRV_BLOCKED_CLR,       PRV_BLOCKED_LBL       },
/* 05 */  { PRV_THREAD_SYNC_CLR,   PRV_THREAD_SYNC_LBL   },
/* 06 */  { PRV_TEST_PROBE_CLR,    PRV_TEST_PROBE_LBL    },
/* 07 */  { PRV_THREAD_SCHED_CLR,  PRV_THREAD_SCHED_LBL  },
/* 08 */  { PRV_WAIT_CLR,          PRV_WAIT_LBL          },
/* 09 */  { PRV_BLOCKING_SEND_CLR, PRV_BLOCKING_SEND_LBL },
/* 10 */  { PRV_ISEND_CLR,         PRV_ISEND_LBL         },
/* 11 */  { PRV_IRECV_CLR,         PRV_IRECV_LBL         },
/* 12 */  { PRV_IO_CLR,            PRV_IO_LBL            },
/* 13 */  { PRV_GLOBAL_OP_CLR,     PRV_GLOBAL_OP_LBL     },
/* 14 */  { PRV_NOT_TRACING_CLR,   PRV_NOT_TRACING_LBL   },
/* 15 */  { PRV_OVERHEAD_CLR,      PRV_OVERHEAD_LBL      },
/* 16 */  { {  66,  66,  66 },     PRV_NOT_USED_LBL      },
/* 17 */  { { 189, 168, 100 },     PRV_NOT_USED_LBL      },
/* 18 */  { {  95, 200,   0 },     PRV_NOT_USED_LBL      },
/* 19 */  { { 203,  60,  69 },     PRV_NOT_USED_LBL      },
/* 20 */  { {   0, 109, 255 },     PRV_NOT_USED_LBL      },
/* 21 */  { { 200,  61,  68 },     PRV_NOT_USED_LBL      },
/* 22 */  { { 200,  66,   0 },     PRV_NOT_USED_LBL      },
/* 23 */  { {   0,  41,   0 },     PRV_NOT_USED_LBL      },
/* 24 */  { { 139, 121, 177 },     PRV_NOT_USED_LBL      },
/* 25 */  { { 116, 116, 116 },     PRV_NOT_USED_LBL      },
/* 26 */  { { 200,  50,  89 },     PRV_NOT_USED_LBL      },
/* 27 */  { { 255, 171,  98 },     PRV_NOT_USED_LBL      },
/* 28 */  { {   0,  68, 189 },     PRV_NOT_USED_LBL      },
/* 29 */  { {  52,  43,   0 },     PRV_NOT_USED_LBL      },
/* 30 */  { { 255,  46,   0 },     PRV_NOT_USED_LBL      },
/* 31 */  { { 100, 216,  32 },     PRV_NOT_USED_LBL      },
/* 32 */  { {   0,   0, 112 },     PRV_NOT_USED_LBL      },
/* 33 */  { { 105, 105,   0 },     PRV_NOT_USED_LBL      },
/* 34 */  { { 132,  75, 255 },     PRV_NOT_USED_LBL      },
/* 35 */  { { 184, 232,   0 },     PRV_NOT_USED_LBL      },
/* 36 */  { {   0, 109, 112 },     PRV_NOT_USED_LBL      },
/* 37 */  { { 189, 168, 100 },     PRV_NOT_USED_LBL      },
/* 38 */  { { 132,  75,  75 },     PRV_NOT_USED_LBL      },
/* 39 */  { { 255,  75,  75 },     PRV_NOT_USED_LBL      },
/* 40 */  { { 255,  20,   0 },     PRV_NOT_USED_LBL      },
/* 41 */  { {  52,   0,   0 },     PRV_NOT_USED_LBL      },
/* 42 */  { {   0,  66,   0 },     PRV_NOT_USED_LBL      },
/* 43 */  { { 184, 132,   0 },     PRV_NOT_USED_LBL      },
/* 44 */  { { 100,  16,  32 },     PRV_NOT_USED_LBL      },
/* 45 */  { { 146, 255, 255 },     PRV_NOT_USED_LBL      },
/* 46 */  { {   0,  23,  37 },     PRV_NOT_USED_LBL      },
/* 47 */  { { 146,   0, 255 },     PRV_NOT_USED_LBL      },
/* 48 */  { {   0, 138, 119 },     PRV_NOT_USED_LBL      }
};

#ifdef __cplusplus
}
#endif

#endif /* _PARAVERCOLORS */

