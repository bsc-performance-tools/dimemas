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

  $URL:: https://svn.bsc.#$:  File
  $Rev:: 15               $:  Revision of last commit
  $Author:: jgonzale      $:  Author of last commit
  $Date:: 2010-02-16 16:5#$:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef PCF_DEFINES_H
#define PCF_DEFINES_H

#include "ParaverColors.h"

/*******************************************************************************
 * PARAVER STATES DEFINITIONS                                                  *
 ******************************************************************************/
/* Type to define the characteristics of any Paraver state */
typedef struct paraver_state
{
  char         *name;
  unsigned char color[3];
} paraver_state_info_t;

/* STATE DEFINES **************************************************************/

/* Every state has three components:
 *
 * 1. PRV_<STATE_NAME>_ST:  Constant that identifies the states
 *
 * 2. PRV_<STATE_NAME>_LBL: String label with the state description
 *
 * 3. PRV_<STATE_NAME>_CLR: State color using RGB codifications
 */

/* Internal Dimemas states. It's not necessary to match private Dimemas colors
 * with not used Paraver colors defined on 'ParaverColors.h', from 'common-files'
 */

#define PRV_SENDRECV_ST        16
#define PRV_SENDRECV_LBL       "SendRecv"
#define PRV_SENDRECV_CLR       { 170,   0,   0 }

#define PRV_MPI2_ONESIDED_ST   17
#define PRV_MPI2_ONESIDED_LBL  "One Sided OP"
#define PRV_MPI2_ONESIDED_CLR  { 189, 168, 100 }

#define PRV_STARTUP_ST         18
#define PRV_STARTUP_LBL        "Startup Latency"
#define PRV_STARTUP_CLR        {  95, 200,   0 }

#define PRV_WAIT_LINKS_ST      19
#define PRV_WAIT_LINKS_LBL     "Waiting links"
#define PRV_WAIT_LINKS_CLR     { 203,  60,  69 }

/* New state to show data copy on sends */
#define PRV_DATA_COPY_ST       20
#define PRV_DATA_COPY_LBL      "Data copy"
#define PRV_DATA_COPY_CLR      {   0, 109, 255 }

/* New state to show round trip time on sends */
#define PRV_RTT_ST              21
#define PRV_RTT_LBL             "Round Trip Time"
#define PRV_RTT_CLR             {  55,  55,  55 }

#define LAST_STATE PRV_RTT_ST

/* 'global_paraver_states' DEFINITION *****************************************/

/* This define must be uptdated when new state is added */
#define PRV_STATE_COUNT     PRV_DEFAULT_STATES_NUM
#define DIMEMAS_STATE_COUNT (LAST_STATE - PRV_STATE_COUNT)+1
/* This define is used on PCF generation */
#define PCF_COLOR_COUNT_LBL "NUM_OF_STATE_COLORS 21"
/* This define must be nomber of states minus 1 */
#define PCF_YMAX_LBL        "YMAX_SCALE          20"

/* The global_paraver_states. Must be updated on addition of new states */
static DefaultPalette_t DimemasDefaultPalette[DIMEMAS_STATE_COUNT] =
{
/* Internal Dimemas states (use not defined labels) */

/* 16 */  {PRV_SENDRECV_CLR,
           PRV_SENDRECV_LBL},

/* 17 */  {PRV_MPI2_ONESIDED_CLR,
           PRV_MPI2_ONESIDED_LBL},

/* 18 */  {PRV_STARTUP_CLR,
           PRV_STARTUP_LBL},

/* 19 */  {PRV_WAIT_LINKS_CLR,
           PRV_WAIT_LINKS_LBL},

/* 20 */  {PRV_DATA_COPY_CLR,
           PRV_DATA_COPY_LBL},

/* 21 */  {PRV_RTT_CLR,
           PRV_RTT_LBL}
};

#define PCF_HEAD_LINES 14
static const char *pcf_head[PCF_HEAD_LINES]=
{
  "DEFAULT_OPTIONS",
  "" ,
  "LEVEL               TASK",
  "UNITS               NANOSEC",
  "LOOK_BACK           100",
  "SPEED               1",
  "FLAG_ICONS          ENABLED",
  PCF_COLOR_COUNT_LBL,/* OJO: ESTA CONSTANTE ESTÁ EN 'pcf_defines.h' */
  PCF_YMAX_LBL,       /* OJO: ESTA CONSTANTE ESTÁ EN 'pcf_defines.h' */
  "\n",
  "DEFAULT_SEMANTIC",
  "",
  "THREAD_FUNC         State As Is",
  "\n"
};

#define PCF_MIDDLE_LINES 64
static const char *pcf_middle[PCF_MIDDLE_LINES] =
{
  "EVENT_TYPE",
  "10  90  Critical Path Block",
  "VALUES",
  "1  Begin",
  "0  End",
  "\n",
  "EVENT_TYPE",
  "10  89  I/O",
  "VALUES",
  "0      IO METADATA BEGIN",
  "1      IO METADATA END",
  "2      IO BLOCK NONCOLLECTIVE READ BEGIN",
  "3      IO BLOCK NONCOLLECTIVE READ END",
  "4      IO BLOCK NONCOLLECTIVE WRITE BEGIN",
  "5      IO BLOCK NONCOLLECTIVE WRITE END",
  "6      IO BLOCK COLLECTIVE READ BEGIN",
  "7      IO BLOCK COLLECTIVE READ END",
  "8      IO BLOCK COLLECTIVE WRITE BEGIN",
  "9      IO BLOCK COLLECTIVE WRITE END",
  "10     IO NONBLOCK NONCOLLECTIVE READ BEGIN",
  "11     IO NONBLOCK NONCOLLECTIVE END",
  "12     IO NONBLOCK NONCOLLECTIVE WRITE BEGIN",
  "13     IO NONBLOCK COLLECTIVE READ BEGIN",
  "14     IO NONBLOCK COLLECTIVE WRITE BEGIN",
  "15     IO NONBLOCK COLLECTIVE END",
  "\n",
  "EVENT_TYPE",
  "11  88   One Sided Operations",
  "VALUES",
  "0      OS START",
  "1      OS LATENCY",
  "2      OS END",
  "3      OS FENCE START",
  "4      OS FENCE END",
  "5      OS GET LOCK",
  "6      OS WAIT LOCK",
  "7      OS UNLOCK BEGIN",
  "8      OS UNLOCK END",
  "9      OS POST POST",
  "10     OS POST START",
  "11     OS POST START BLOCK",
  "12     OS POST COMPLETE BLOCK",
  "13     OS POST COMPLETE",
  "14     OS POST WAIT",
  "15     OS POST WAIT BLOCK",
  "\n\n",
  "EVENT_TYPE",
  "8  91  GROUP_BLOCK",
  "VALUES",
  "0       END",
  "\n\n",
  "EVENT_TYPE",
  "8  92  GROUP_LAST",
  "VALUES",
  "0       END",
  "\n\n",
  "EVENT_TYPE",
  "8  93  GROUP_FREE",
  "VALUES",
  "0       END",
  "\n\n",
  "EVENT_TYPE",
  "8 40000000  ITERATION_BEGIN",
  "\n\n"
};



#define PCF_TAIL_LINES 33
static const char* pcf_tail[PCF_TAIL_LINES]={
  "GRADIENT_COLOR",
  "0     {0,255,2}",
  "1     {0,244,13}",
  "2     {0,232,25}",
  "3     {0,220,37}",
  "4     {0,209,48}",
  "5     {0,197,60}",
  "6     {0,185,72}",
  "7     {0,173,84}",
  "8     {0,162,95}",
  "9     {0,150,107}",
  "10    {0,138,119}",
  "11    {0,127,130}",
  "12    {0,115,142}",
  "13    {0,103,154}",
  "14    {0,91,166}",
  "\n",
  "GRADIENT_NAMES",
  "0     Gradient 0",
  "1     Grad. 1/MPI Events",
  "2     Grad. 2/OMP Events",
  "3     Grad. 3/OMP locks",
  "4     Grad. 4/User func",
  "5     Grad. 5/User Events",
  "6     Grad. 6/General Events",
  "7     Gradient 7/Hard. Counters",
  "8     Gradient 8",
  "9     Gradient 9",
  "10    Gradient 10",
  "11    Gradient 11",
  "12    Gradient 12",
  "13    Gradient 13",
  "14    Gradient 14"
};

#endif
