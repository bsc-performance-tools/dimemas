#ifndef __SCH_svr4_h
#define __SCH_svr4_h
static char SCH_svr4_h_rcsid[]="$Id: SCH_svr4.h,v 1.1 2004/11/18 15:05:39 paraver Exp $";
static char *__a_SCH_svr4_h=SCH_svr4_h_rcsid;

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
 * Unix SVR4 scheduler structures
 * 
 * Sergi Girona (sergi@ac.upc.es)
 * 
 * (c) DAC-UPC 1996
 * 
 */

typedef short   prio_t;

/* These lines comes from Solaris file /usr/include/sys/ts.h */

typedef struct tsdpent {
  prio_t   ts_globpri;    /* global (class independent) priority        */
  long     ts_quantum;    /* time quantum given to procs at this level  */
  prio_t   ts_tqexp;      /* ts_umdpri assigned when proc at this level */
                          /*                   exceeds its time quantum */
  prio_t   ts_slpret;     /* ts_umdpri assigned when proc at this level */
                          /*        returns to user mode after sleeping */
  short    ts_maxwait;    /* bumped to ts_lwait if more than ts_maxwait */
                          /* s ecs elapse before receiving full quantum */
  short    ts_lwait;      /* ts_umdpri assigned if ts_dispwait exceeds  */
                          /*                                 ts_maxwait */
} tsdpent_t;

typedef struct tsproc {
   prio_t   ts_umdpri;      /* user mode priority within ts class       */
   long    ts_timeleft;    /* time remaining in procs quantum           */
   long    ts_dispwait;    /* number of wall clock seconds since start  */
                           /*    of quantum (not reset upon preemption  */
   t_boolean full_quantum;
   long    last_quantum;
}tsproc_t;

typedef struct svr4 {
   double  context_switch; /* Context Switch cost */
   double  busy_wait;      /* Busy wait when message not ready */
   t_boolean priority_preemptive;
   double minimum_time_preemption;
   double quantum_factor;
}svr4_t;

/**
 * External routines defined in file SCH_fifo.c
 **/
extern void svr4_thread_to_ready(struct t_thread *thread);
extern t_micro svr4_get_execution_time(struct t_thread *thread);
extern struct t_thread *svr4_next_thread_to_run(struct t_node *node);
extern void svr4_init_scheduler_parameters(struct t_thread *thread);
extern void svr4_clear_parameters(struct t_thread *thread);
extern int svr4_info(int info, struct t_thread *thread_s, 
		     struct t_thread *thread_r);
extern void svr4_init(char *filename, struct t_machine *machine);
extern void svr4_copy_parameters(struct t_thread *th_o, struct t_thread *th_d);
extern void svr4_free_parameters (struct t_thread *thread);

#endif
