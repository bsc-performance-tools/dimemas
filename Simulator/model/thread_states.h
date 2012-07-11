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

#include "paraver_pcf.h"
#include "define.h"
#include "types.h"

/* ESTADOS ********************************************************************/
/* Estos estados se definen en este punto, puesto que son internos a Dimemas
 * y no se 'exportan' a las trazas Paraver, por lo que no se incluyen en el
 * fichero 'paraver_pcf.h' 
 * Los estados se definen numericamente como incrementos sobre PRV_STATE_COUNT
 * para evitar, de esta manera, el solapamiento con los estados definidos para
 * las trazas Paraver
 */
 
#define WAIT_LINKS_ST  PRV_STATE_COUNT
#define BUSY_WAIT_ST   PRV_STATE_COUNT+1
#define CONTEXT_SWX_ST PRV_STATE_COUNT+2
 
#define TOTAL_THREAD_STATES PRV_STATE_COUNT+3


extern dimemas_timer current_time;

char          *errorStr;
dimemas_timer  last_state_time;

/* FUNCIONES PUBLICAS *********************************************************/
extern t_boolean
init_thread_state (struct t_thread* thread, int state);

extern t_boolean
end_thread_state (struct t_thread* thread, int state);

extern char*
get_last_state_error(void);

extern dimemas_timer
get_last_state_time(void);
