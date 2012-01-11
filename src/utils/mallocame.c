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

#define NORMAL_MALLOC

#ifdef NORMAL_MALLOC

#include<stdlib.h>
#include<stdio.h>

void malloc_init()
{
}

char *mallocame(size_t s)
{
  char *adreca;

  adreca = (char*) malloc(s);
  if (adreca == NULL)
  {
    fprintf(stderr,"Out of Memory\n");
    exit(1);
  }
  return(adreca);
}

void freeame(char *a, int s)
{
  free(a);
}

void malloc_end()
{
}


#else


#include "define.h"
#include "types.h"

#include "extern.h"
#include "mallocame.h"
#include "subr.h"

static struct t_list SPACE[1024];
static int      veces_malloc = 0;
static int      malo[1024];

void
malloc_init()
{
   register int    i;

   for (i = sizeof (struct t_list); i < 1024; i++)
   {
      SPACE[i].next = LI_NIL;
      malo[i] = 0;
   }
}

char * mallocame(int s)
{
   register char  *add;

   veces_malloc++;
   s = s+1280;
   if (s < sizeof (struct t_list))
      s = (sizeof (struct t_list));
   if (s < 1024)
   {
     malo[s]++;
     if (SPACE[s].next != LI_NIL)
      {
	 add = (char *) SPACE[s].next;
	 SPACE[s].next = ((struct t_list *) add)->next;
	 return (add);
      }
   }
   add = (char *) malloc (s);
   if (add == (char *) 0)
   {
      panic ("Can't get more space %s. Malloc error\n", s);
      exit (1);
   }
   return (add);
}

void freeame(char  *a, int s)
{
   veces_malloc--;

   s = s+1280;
   if (s < sizeof (struct t_list))
      s = sizeof (struct t_list);
   if (s < 1024)
   {
      malo[s]--;
      ((struct t_list *) a)->next = SPACE[s].next;
      SPACE[s].next = (struct t_list *) a;
   }
   else
      free (a);

}

void
malloc_end()
{
   register int    i;

   if (debug)
   {
      for (i = 0; i < 1024; i++)
	 if (malo[i] != 0)
	    printf ("Malloc size %d still have %d\n", i, malo[i]);
   }
}


#endif
