/*****************************************************************************\
 *                        ANALYSIS PERFORMANCE TOOLS                         *
 *                                  Dimemas                                  *
 *       Simulation tool for the parametric analysis of the behaviour of     *
 *       message-passing applications on a configurable parallel platform    *
 *                                                                           *
 *****************************************************************************
 *     ___     This library is free software; you can redistribute it and/or *
 *    /  __         modify it under the terms of the GNU LGPL as published   *
 *   /  /  _____    by the Free Software Foundation; either version 2.1      *
 *  /  /  /     \   of the License, or (at your option) any later version.   *
 * (  (  ( B S C )                                                           *
 *  \  \  \_____/   This library is distributed in hope that it will be      *
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

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- *\

  $URL::                  $:  File
  $Rev::                  $:  Revision of last commit
  $Author::               $:  Author of last commit
  $Date::                 $:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/*
 * External declarations
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

#include <types.h>

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

#ifdef USE_EQUEUE
extern Equeue Event_queue;
extern Equeue Interactive_event_queue;
#else
extern struct t_queue Event_queue;
extern struct t_queue Interactive_event_queue;
#endif

extern struct t_queue Port_queue;
#ifdef USE_EQUEUE
extern Equeue Node_queue;
#else
extern struct t_queue Node_queue;
#endif
extern struct t_queue Network_queue;
extern struct t_queue Global_op;

/* FEC */
extern struct t_queue Machine_queue;
extern struct t_queue Dedicated_Connections_queue;
extern dimemas_timer  execution_end_time; /* Temps final de l'execucio */

extern dimemas_timer current_time;
extern dimemas_timer final_statistical_time;

extern int      debug;
extern int      assert;
extern int      output_level;


extern int      Ptask_ids;
extern int      port_ids;

extern t_boolean reload_Ptasks;
extern int       reload_limit;
extern t_boolean reload_done;

extern t_boolean full_out_info;
extern t_boolean short_out_info; /* defined on 'main.c' */

extern char     message_buffer[];

extern char         *paraver_file;
extern char         *paraver_cfg_include_file;
extern t_boolean     paraver_binary;
extern t_boolean     paraver_cfg_include;
extern dimemas_timer start_paraver;
extern dimemas_timer stop_paraver;
extern t_boolean     paraver_initial_timer;
extern t_boolean     paraver_final_timer;

extern FILE    *salida_datos;

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

extern long long int  RD_SYNC_message_size;
extern int            RD_SYNC_use_trace_sync;

/* Data copy latency global variables. Defined in 'communic.c' */

extern t_boolean DATA_COPY_enabled;
extern int       DATA_COPY_message_size;

/* Round trip time global variables. Defined in 'communic.c' */
extern t_boolean RTT_enabled;
extern t_nano   RTT_time;

/* Preemptio overheads global variables. Defined in 'task.c' */
extern t_boolean PREEMP_enabled;
extern int       PREEMP_cycle;
extern t_nano   PREEMP_time;

/* Synthetic burst detection. Defined in 'task.c' */
extern t_boolean synthetic_bursts;

/* File pointer reserving facility */
extern void free_reserved_pointer(void);

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
/* File semaphore.c */
/* File read.c */
/* File links.c */
/* File check.c */
/* File SCH_fifo.c */
/* File SCH_rr.c */
/* File SCH_prio_fifo.c */
/* File SCH_svr4.c */
/* File SCH_boost.c */

