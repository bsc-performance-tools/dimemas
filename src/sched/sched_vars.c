char c_c_rcsid[]="$Id: sched_vars.c,v 1.1 2004/11/18 15:05:39 paraver Exp $";
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
 * Schedulers policies declaration
 * 
 * Sergi Girona (sergi@ac.upc.es)
 * 
 * (c) DAC-UPC 1993-94
 * 
 */


#include "define.h"
#include "types.h"
#include "sched_vars.h"

#include "extern.h"
#include "SCH_boost.h"
#include "SCH_prio_fifo.h"
#include "SCH_svr4.h"
#include "SCH_fifo.h"
#include "SCH_rr.h"

struct t_scheduler_actions SCH[] =
{
  "FIFO",                         /* Politica FIFO */
  FIFO_thread_to_ready,
  FIFO_get_execution_time,
  FIFO_next_thread_to_run,
  FIFO_init_scheduler_parameters,
  FIFO_clear_parameters,
  FIFO_info,
  FIFO_init,
  FIFO_copy_parameters,
  FIFO_free_parameters,
  "RR",                           /* Politica Round Robin */
  RR_thread_to_ready,
  RR_get_execution_time,
  RR_next_thread_to_run,
  RR_init_scheduler_parameters,
  RR_clear_parameters,
  RR_info,
  RR_init,
  RR_copy_parameters,
  RR_free_parameters,
  "FIXED_PRIORITY_FIFO",          /* Politica FIFO con prioridades fijas */
  PRIO_FIFO_thread_to_ready,
  PRIO_FIFO_get_execution_time,
  PRIO_FIFO_next_thread_to_run,
  PRIO_FIFO_init_scheduler_parameters,
  PRIO_FIFO_clear_parameters,
  PRIO_FIFO_info,
  PRIO_FIFO_init,
  PRIO_FIFO_copy_parameters,
  PRIO_FIFO_free_parameters,
  "UNIX_SVR4",                    /* Politica tipo System V */
  svr4_thread_to_ready,
  svr4_get_execution_time,
  svr4_next_thread_to_run,
  svr4_init_scheduler_parameters,
  svr4_clear_parameters,
  svr4_info,
  svr4_init,
  svr4_copy_parameters,
  svr4_free_parameters,
  "BOOST",                        /* Politica Boost */
  SCH_boost_thread_to_ready,
  SCH_boost_get_execution_time,
  SCH_boost_next_thread_to_run,
  SCH_boost_init_scheduler_parameters,
  SCH_boost_clear_parameters,
  SCH_boost_info,
  SCH_boost_init,
  SCH_boost_copy_parameters,
  SCH_boost_free_parameters,
  0
};

struct t_communic_actions COMMUNIC[] =
{
  "FIFO",
  "RR",
  "BOOST",
  0
};
