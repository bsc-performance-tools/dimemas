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
#include "assert.h"

extern char      yy_error_string[];
extern t_boolean yy_error_filled;
extern t_boolean dimemas_GUI;

void info   (const char *fmt, ...)
{
  if (!short_out_info)
  {
    va_list args;
    va_start (args, fmt);
    vfprintf (stdout, fmt, args);
    va_end (args);
    fflush(stdout);
  }
}

void die (const char *fmt, ...)
{
  va_list args;
  va_start (args, fmt);

  fprintf (stderr, "\n");
  fprintf (stderr, "UNRECOVERABLE ERROR -> ");
  vfprintf (stderr, fmt, args);
  fprintf (stderr, "\n");
  va_end (args);

  exit (EXIT_FAILURE);
}

void warning(const char *fmt, ...)
{
  va_list args;
  va_start (args, fmt);
  fprintf(stdout, "WARNING: ");
  vfprintf (stdout, fmt, args);
  va_end (args);
}

void panic (const char *fmt, ...)
{
  va_list args;
  va_start (args, fmt);

  fprintf (stdout, "Fatal error at time ");
  FPRINT_TIMER (stdout, current_time);
  fprintf (stdout, "\n");
  vfprintf (stdout, fmt, args);
  va_end (args);

  /* JGG: Aquí se debe controlar que las trazas parciales de Paraver, si las
   * hay, se deben destruir */
  // assert(0);

  // PARAVER_Cleanup();

  exit (EXIT_FAILURE);
}

