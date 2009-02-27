#define NORMAL_MALLOC

#ifdef NORMAL_MALLOC

#include<stdlib.h>
#include<stdio.h>

void malloc_init()
{
}

char *mallocame(int s)
{
  char *adreca;
  
  adreca=(char*)malloc(s);
  if (adreca==NULL)
  {
    fprintf(stderr,"Not enough MEMORY!!!!\n");
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

char malloc_c_rcsid[]="$Id: mallocame.c,v 1.7 2005/06/28 10:58:56 paraver Exp $";

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

char *
mallocame(int s)
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


void
freeame(char  *a, int s)
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
