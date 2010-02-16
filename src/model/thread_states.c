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

#include "thread_states.h"

t_boolean
init_thread_state (struct t_thread* thread, int state)
{
  t_thread_state new_state;
  t_boolean      result;
  
  result = TRUE;
  
  if (thread->current_state != STATE_NIL)
  {
    result   = FALSE;
    errorStr = "Initalizing new thread state with previous state open\n";
  }
  else
  {
    new_state = (t_thread_state) mallocame(sizeof(struct thread_state));
    
    new_state->state      = state;
    new_state->init_time  = current_time;
    thread->current_state = new_state;
  }
  return result;
}

t_boolean
end_thread_state (struct t_thread* thread, int state)
{
  t_boolean result = TRUE;
  
  if (thread->current_state == STATE_NIL)
  {
    result   = FALSE;
    errorStr = "Trying to close an uninitialized state\n";
  }
  else if (thread->current_state->state != state)
  {
    result   = FALSE;
    errorStr = "Trying to close different state that initialized one\n";
  }
  else
  {
    /* JGG (02/05/2005) */
    SUB_TIMER (current_time, thread->current_state->init_time, last_state_time);
    
    freeame(thread->current_state, sizeof(struct thread_state));
    thread->current_state = STATE_NIL;
  }
}

char*
get_last_state_error(void)
{
  return errorStr;
}

extern dimemas_timer
get_last_state_time(void)
{
  return last_state_time;
}
