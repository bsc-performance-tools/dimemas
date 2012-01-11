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

  $URL:: https://svn.bsc.es/repos/ptools/prv2dim/#$:  File
  $Rev:: 563                                      $:  Revision of last commit
  $Author:: jgonzale                              $:  Author of last commit
  $Date:: 2011-03-22 16:27:21 +0100 (Tue, 22 Mar #$:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include <sys/resource.h>
#include <stdio.h>
#include <errno.h>

#include <stdlib.h>
/**
 * Initializes the control structures and variables to guarantee the IO
 * scalability
 */
void IO_init(void)
{
  struct rlimit nofile_limits;

  if (getrlimit(RLIMIT_NOFILE, &nofile_limits) == -1)
  {
    perror("Error trying to get the OS resources available");
    exit(EXIT_FAILURE);
  }

  /* DEBUG */
  printf("No. Files. Hard limit %d. Sof Limit %d\n",
         nofile_limits.rlim_max,
         nofile_limits.rlim_cur);

  exit(EXIT_SUCCESS);
}

