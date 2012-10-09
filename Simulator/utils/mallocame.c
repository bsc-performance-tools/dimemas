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

#include <stdlib.h>

#include <types.h>
#include <extern.h>
#include <assert.h>

// #include "mallocame.h"

#include <stdio.h>

static size_t memory_requests        = 0;
static size_t memory_requested_space = 0;
static size_t memory_frees           = 0;

void MALLOC_Init()
{
}

void MALLOC_End()
{
  PRINT_TIMER (current_time);
  printf(": MALLOC_end called. Memory usage statistics:\n");
  printf("        * Total requests            = %zd\n", memory_requests);
  printf("        * Total space requested (B) = %zd\n", memory_requested_space);
  printf("        * Total frees               = %zd\n", memory_frees);

  return;
}

char* malloc(size_t size)
{
  char* result_address;

  if ( (result_address = (char*) malloc(size)) == NULL)
  {
    fprintf(stderr, "Out of Memory\n");
    exit(EXIT_FAILURE);
  }

  memory_requests++;
  memory_requested_space += size;

  // printf("@ allocated = %p (", result_address);
  /*
  if (MemoryRequests.count(result_address) > 0)
  {
    MemoryRequests[result_address]++;
  }
  else
  {
    MemoryRequests[result_address] = 1;
  }
  */

  // printf("%d times)\n", MemoryRequests[result_address]);

  return(result_address);
}

void MALLOC_free_memory(char *a)
{

  /*
  if (MemoryRequests.count(a) == 1)
  {
    // printf("@ freeing = %p (", a);

    MemoryRequests[a]--;

    // printf("%d times)\n", MemoryRequests[a]);


    assert (MemoryRequests[a] >= 0);

    if (MemoryRequests[a] == 0)
    {
      MemoryRequests.erase(a);
    }
  }
  else
  {
    // printf("@ freeing = %p (allocated externally)\n", a);
    memory_frees++;
  }
  */
  memory_frees++;

  free(a);
}

