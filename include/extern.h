#ifndef RCS_extern_h
#define RCS_extern_h
#endif
/******************************************************************************
*******************************************************************************
*
*  (c) Copyright 1994, 1995, 1996 - CENTRO EUROPEO DE PARALELISMO DE BARCELONA
*				  - UNIVERSITAT POLITECNICA DE CATALUNYA
*  ALL RIGHTS RESERVED
*  
*  	THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED
*  AND COPIED ONLY IN ACCORDANCE WITH THE TERMS OF SUCH LICENSE AND
*  WITH THE INCLUSION OF THE ABOVE COPYRIGHT NOTICE. THIS SOFTWARE OR
*  ANY OTHER COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE
*  AVAILABLE TO ANY OTHER PERSON. NO TITLE TO AND OWNERSHIP OF THE
*  SOFTWARE IS HEREBY TRANSFERRED.
*  
*  	THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT
*  NOTICE AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY CEPBA-UPC
*  OR ITS THIRD PARTY SUPPLIERS  
*  
*  	CEPBA-UPC AND ITS THIRD PARTY SUPPLIERS,
*  ASSUME NO RESPONSIBILITY FOR THE USE OR INABILITY TO USE ANY OF ITS
*  SOFTWARE . DIMEMAS AND ITS GUI IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*  KIND, AND CEPBA-UPC EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES, INCLUDING
*  BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
*  FITNESS FOR A PARTICULAR PURPOSE.
* 
*  
*******************************************************************************
******************************************************************************/

/*
 * External declarations
 * 
 * Sergi Girona (sergi@ac.upc.es)
 * 
 * (c) DAC-UPC 1993-94
 * 
 */

#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <stdarg.h>

#if (ALPHA|SUN4|SGI64)
#include <sys/time.h>
#include <sys/resource.h>
#endif

#if (HPPA)
#include <unistd.h>
#include <sys/times.h>
#endif

#ifdef CHECK
#define T(x) x,
#define L(x) x
#define EXTERN
#else   /* CHECK */
#define T(x)
#define L(x)
#define EXTERN extern
#endif  /* CHECK */

extern struct t_queue Ptask_queue;
extern struct t_queue Event_queue;
extern struct t_queue Interactive_event_queue;
extern struct t_queue Port_queue;
extern struct t_queue Node_queue;
extern struct t_queue Network_queue;
extern struct t_queue Global_op;

/* FEC */
extern struct t_queue Machine_queue;
extern struct t_queue Dedicated_Connections_queue;
extern int last_node_id_used;
extern dimemas_timer execution_end_time; /* Temps final de l'execucio */



extern dimemas_timer current_time;
extern dimemas_timer final_statistical_time;

extern struct t_simulator sim_char;

extern int      debug;
extern int      output_level;

extern int      SCH_prio;

extern int      Ptask_ids;
extern int      port_ids;

extern t_boolean reload_Ptasks;
extern int      reload_limit;
extern t_boolean reload_done;

extern t_boolean full_out_info;
extern t_boolean short_out_info; /* defined on 'main.c' */

extern char     message_buffer[];

extern char    *paraver_file;
extern t_boolean paraver_binary;
extern dimemas_timer start_paraver;
extern dimemas_timer stop_paraver;
extern t_boolean paraver_initial_timer;
extern t_boolean paraver_final_timer;

extern FILE    *sddf_file;
extern char    *sddf_filename;

extern FILE    *salida_datos;

extern t_boolean load_interactive;
extern t_boolean seek_info;
extern t_boolean sorted_files;
extern t_boolean paraver_priorities;
extern t_boolean paraver_different_waits;

extern t_boolean monitorize_event;
extern int event_to_monitorize;
extern FILE    *File_for_Event;

extern t_boolean binary_files;
extern t_boolean gzip_files;
extern t_boolean new_trace_format;

extern struct t_scheduler_actions SCH[];

extern struct t_communic_actions COMMUNIC[];

extern int      PARAVER_cpu;

extern char    *fichero_sch;
extern char    *fichero_fs;
extern char    *fichero_comm;
extern char    *fichero_random;

extern int      greatest_cpuid;

extern int  RD_SYNC_message_size;
extern int  RD_SYNC_use_trace_sync;

/* Data copy latency global variables. Defined in 'communic.c' */

extern t_boolean DATA_COPY_enabled;
extern int       DATA_COPY_message_size;

/* Round trip time global variables. Defined in 'communic.c' */
extern t_boolean RTT_enabled;
extern t_micro   RTT_time;

/* Preemptio overheads global variables. Defined in 'task.c' */
extern t_boolean PREEMP_enabled;
extern int       PREEMP_cycle;
extern t_micro   PREEMP_time;

/* Synthetic burst detection. Defined in 'task.c' */
extern t_boolean synthetic_bursts;

/* File pointer reserving facility */
extern void
free_reserved_pointer(void);

/*
 * Prototypes
 */

/* File events.c */
/* File list.c */
/* File main.c */
/* File aleatorias.c */
/* File cpu.c */
/* File task.c */
/* File schedule.c */
/* File fs.c */
/* File communic.c */
/* File ports.c */
/* File memory.c */
/* File paraver.c */
/* File sddf.c */
/* File semaphore.c */
/* File read.c */
/* File links.c */
/* File check.c */
/* File SCH_fifo.c */
/* File SCH_rr.c */
/* File SCH_prio_fifo.c */
/* File SCH_svr4.c */
/* File SCH_boost.c */
