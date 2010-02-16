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


#endif
