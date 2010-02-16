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

#include "define.h"
#include "types.h"

#include "extern.h"
#include "subr.h"

extern char     yy_error_string[];
extern t_boolean yy_error_filled;
extern t_boolean dimemas_GUI;

#ifdef LIBLEXYACC /* LIBLEXYACC */
dimemas_timer current_time;
#endif

void
panic(char *fmt, ...)
{
  va_list args;

  va_start(args, fmt);

  // (void) 
  fprintf (stdout, "Fatal error at time ");
  FPRINT_TIMER (stdout, current_time);
  //(void)
  fprintf (stdout, "\n");
  //(void)
  vfprintf (stdout, fmt, args);
  va_end(args);
  
  /* JGG: Aquí se debe controlar que las trazas parciales de Paraver, si las
   * hay, se deben destruir */
  exit (-1);
}

void
fill_parse_error(char *fmt, ...)
{
  va_list  args;

  if (dimemas_GUI)
    if (yy_error_filled)
      return;

  va_start(args, fmt);

  if (dimemas_GUI)
    vsprintf (yy_error_string, fmt, args);
  else
    vfprintf (stdout, fmt, args);
  va_end(args);
  yy_error_filled = TRUE;
}
