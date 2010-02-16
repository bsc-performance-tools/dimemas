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
#include <stdio.h>
#include <fcntl.h>
#include "define.h"
#include "types.h"
#include "extern.h"
#include <Dimemas2Prv.h>
#include "list.h"
#include "pcf_defines.h"
#include "EventEncoding.h"

//#define PUT_ALL_VALUES 1

/* JGG: La generación de los colores asociados a los estados debería ser más
 * dinámica... 
 * HABRÁ QUE REVISARLO */
#define PCF_HEAD_LINES 14
static char *pcf_head[PCF_HEAD_LINES]=
{
  { "DEFAULT_OPTIONS" },
  { "" },
  { "LEVEL               TASK"},
  { "UNITS               NANOSEC"},
  { "LOOK_BACK           100"},
  { "SPEED               1"},
  { "FLAG_ICONS          ENABLED"},
  { PCF_COLOR_COUNT_LBL},/* OJO: ESTA CONSTANTE ESTÁ EN 'pcf_defines.h' */
  { PCF_YMAX_LBL},       /* OJO: ESTA CONSTANTE ESTÁ EN 'pcf_defines.h' */
  { "\n"},
  { "DEFAULT_SEMANTIC" },
  { "" },
  { "THREAD_FUNC         State As Is" },
  { "\n" }
};
                
#define PCF_MIDDLE_LINES 64
static char *pcf_middle[PCF_MIDDLE_LINES]=
{
  { "EVENT_TYPE" },
  { "10  90  Critical Path Block" },
  { "VALUES" },
  { "1  Begin" },
  { "0  End" },
  { "\n" },
  { "EVENT_TYPE" },
  { "10  89  I/O" },
  { "VALUES" },
  { "0      IO METADATA BEGIN" },
  { "1      IO METADATA END" },
  { "2      IO BLOCK NONCOLLECTIVE READ BEGIN" },
  { "3      IO BLOCK NONCOLLECTIVE READ END" },
  { "4      IO BLOCK NONCOLLECTIVE WRITE BEGIN" },
  { "5      IO BLOCK NONCOLLECTIVE WRITE END" },
  { "6      IO BLOCK COLLECTIVE READ BEGIN" },
  { "7      IO BLOCK COLLECTIVE READ END" },
  { "8      IO BLOCK COLLECTIVE WRITE BEGIN" },
  { "9      IO BLOCK COLLECTIVE WRITE END" },
  { "10     IO NONBLOCK NONCOLLECTIVE READ BEGIN" },
  { "11     IO NONBLOCK NONCOLLECTIVE END" },
  { "12     IO NONBLOCK NONCOLLECTIVE WRITE BEGIN" },
  { "13     IO NONBLOCK COLLECTIVE READ BEGIN" },
  { "14     IO NONBLOCK COLLECTIVE WRITE BEGIN" },
  { "15     IO NONBLOCK COLLECTIVE END" },
  { "\n" },
  { "EVENT_TYPE" },
  { "11  88   One Sided Operations" },
  { "VALUES" },
  { "0      OS START" },
  { "1      OS LATENCY" },
  { "2      OS END" },
  { "3      OS FENCE START" },
  { "4      OS FENCE END" },
  { "5      OS GET LOCK" },
  { "6      OS WAIT LOCK" },
  { "7      OS UNLOCK BEGIN" },
  { "8      OS UNLOCK END" },
  { "9      OS POST POST" },
  { "10     OS POST START" },
  { "11     OS POST START BLOCK" },
  { "12     OS POST COMPLETE BLOCK" },
  { "13     OS POST COMPLETE" },
  { "14     OS POST WAIT" },
  { "15     OS POST WAIT BLOCK" },
  { "\n\n" },
  { "EVENT_TYPE" },
  { "8  91  GROUP_BLOCK" },
  { "VALUES" },
  { "0       END" },
  { "\n\n" },
  { "EVENT_TYPE" },
  { "8  92  GROUP_LAST" },
  { "VALUES" },
  { "0       END" },
  { "\n\n" },
  { "EVENT_TYPE" },
  { "8  93  GROUP_FREE" },
  { "VALUES" },
  { "0       END" },
  { "\n\n" },
  { "EVENT_TYPE" },
  { "8 40000000  ITERATION_BEGIN" },
  { "\n\n" }
};



#define PCF_TAIL_LINES 33
static char *pcf_tail[PCF_TAIL_LINES]={
  { "GRADIENT_COLOR" },
  { "0     {0,255,2}" },
  { "1     {0,244,13}" },
  { "2     {0,232,25}" },
  { "3     {0,220,37}" },
  { "4     {0,209,48}" },
  { "5     {0,197,60}" },
  { "6     {0,185,72}" },
  { "7     {0,173,84}" },
  { "8     {0,162,95}" },
  { "9     {0,150,107}" },
  { "10    {0,138,119}" },
  { "11    {0,127,130}" },
  { "12    {0,115,142}" },
  { "13    {0,103,154}" },
  { "14    {0,91,166}" },
  { "\n" },
  { "GRADIENT_NAMES" },
  { "0     Gradient 0" },
  { "1     Grad. 1/MPI Events" },
  { "2     Grad. 2/OMP Events" },
  { "3     Grad. 3/OMP locks" },
  { "4     Grad. 4/User func" },
  { "5     Grad. 5/User Events" },
  { "6     Grad. 6/General Events" },
  { "7     Gradient 7/Hard. Counters" },
  { "8     Gradient 8" },
  { "9     Gradient 9" },
  { "10    Gradient 10" },
  { "11    Gradient 11" },
  { "12    Gradient 12" },
  { "13    Gradient 13" },
  { "14    Gradient 14" }
};


/******************************************************************************
 ***  MakeParaverPCF
 ******************************************************************************/

int MakeParaverPCF(const char *nom)
{
  int                             i;
  FILE                            *fd;
  struct t_Ptask                 *ptask;
  struct t_module                *mod;
  char                           *pcf_name;
  struct t_user_event_info       *evinfo;
  struct t_user_event_value_info *evinfo_val;


  pcf_name = (char *) malloc((strlen(nom)+5)*sizeof(char));
  if (pcf_name == NULL)
  {
    return(-1);
  }
  
  strcpy(pcf_name,nom);  
  
  if (strcmp(&nom[strlen(nom)-4],".prv") == 0)
  {
    strcpy(&pcf_name[strlen(nom)-3],"pcf");
  }
  else if (strcmp(&nom[strlen(nom)-4],".pcf") != 0)
  {
    strcpy(&pcf_name[strlen(nom)],".pcf");
  }
  

  /* Es crea el fitxer de sortida */
  if ((fd = MYFOPEN(pcf_name, "w")) == NULL)
  {
    fprintf(stderr,"DIMEMAS ERROR: Creating paraver PCF file : %s\n",pcf_name);
    return (-1);
  }

  /* JGG: Escritura de la cabecera del PCF, estática*/
  for (i = 0; i<PCF_HEAD_LINES; i++)
  {
    fprintf(fd,"%s\n",pcf_head[i]);
  }

  /* JGG: Parte dinámica referente a los estados */
  fprintf(fd, "STATES\n");
  for ( i = 0; i < PRV_STATE_COUNT; i++)
  {
    fprintf(
      fd,
      "%d\t%s\n",
      i,
      ParaverDefaultPalette[i].name
    );
  }
  
  /* Dimemas private states */
  for ( i = 0; i < DIMEMAS_STATE_COUNT; i++)
  {
    fprintf(
      fd,
      "%d\t%s\n",
      i + PRV_STATE_COUNT,
      DimemasDefaultPalette[i].name
    );
  }
  
  fprintf(fd, "\nSTATES_COLOR\n");
  for ( i = 0; i < PRV_STATE_COUNT; i++)
  {
    fprintf(
      fd,
      "%d\t{%d,%d,%d}\n",
      i,
      ParaverDefaultPalette[i].RGB[0],
      ParaverDefaultPalette[i].RGB[1],
      ParaverDefaultPalette[i].RGB[2]
    );
  }
  
  /* Dimemas private states */
  for ( i = 0; i < DIMEMAS_STATE_COUNT; i++)
  {
    fprintf(
      fd,
      "%d\t{%d,%d,%d}\n",
      i + PRV_STATE_COUNT,
      DimemasDefaultPalette[i].RGB[0],
      DimemasDefaultPalette[i].RGB[1],
      DimemasDefaultPalette[i].RGB[2]
    );
  }
  
  fprintf(fd, "\n");

  /* JGG: Escritura de la parte central estática */
  for(i = 0; i<PCF_MIDDLE_LINES; i++)
  {
    fprintf(fd,"%s\n",pcf_middle[i]);
  }

#ifdef PUT_ALL_VALUES
  for (i = 0; i < NUM_MPICALLS; i++) {
     MPIEventEncoding_EnableOperation(i);
  }
  MPIEventEncoding_WriteEnabledOperations(fd);
#endif
  
  /* S'escriuen els events que ha definit l'usuari manualment */
  for(
    ptask  = (struct t_Ptask *)head_queue(&Ptask_queue);
    ptask != P_NIL;
    ptask  = (struct t_Ptask *)next_queue(&Ptask_queue)
  )
  {
#ifndef PUT_ALL_VALUES
    /* Es recorren tots els possibles grups de blocs */
    for (i = 0; i<NUM_BLOCK_GROUPS; i++)
    {
      /* Es posa la capc,alera d'aquest tipus */
      fprintf(
        fd,"EVENT_TYPE\n%d\t%d\t%s\nVALUES\n0\tEnd\n",
        PRV_BLOCK_COLOR(i),
        PRV_BLOCK_TYPE(i),
        PRV_BLOCK_LABEL(i)
      );
      /* S'afegeixen com a valors tots els blocs utilitzats d'aquest grup */
      for (
        mod=(struct t_module *)head_queue(&ptask->Modules);
        mod!=(struct t_module *) 0;
        mod=(struct t_module *)next_queue(&ptask->Modules)
      )
      {
        if (
          (mod->activity_name!=NULL) &&
          (mod->block_name!=NULL) &&
          (mod->used) &&
          (BLOCK_TRF2PRV_TYPE(mod->identificator)==PRV_BLOCK_TYPE(i))
        )
        {
          /* Es un block correctament definit i que ha estat utilitzat */
          /* No es mostra l'activitat per tal que la trac,a .prv quedi 
           * exactament com si hagues estat creada directament amb el mpitrace.
          fprintf(fd,"%d\t%s: %s\n",
                  BLOCK_TRF2PRV_VALUE(mod->identificator),
                  mod->activity_name,
                  mod->block_name);
          */
          fprintf(
            fd,
            "%d\t%s\n",
            BLOCK_TRF2PRV_VALUE(mod->identificator),
            mod->block_name
          );
        }
      }
      fprintf(fd,"\n\n");
    }
#endif
    /* Es recorren tots els possibles tipus d'user event */
    for(
      evinfo  = (struct t_user_event_info *)head_queue(&ptask->UserEventsInfo);
      evinfo != (struct t_user_event_info *)0;
      evinfo  = (struct t_user_event_info *)next_queue(&ptask->UserEventsInfo)
    )
    {
      /* Es posa la capc,alera d'aquest tipus */
      fprintf(
        fd,
        "EVENT_TYPE\n%d\t%d\t%s\nVALUES\n",
        evinfo->color,
        evinfo->type,
        evinfo->name
      );
      
      /* Es recorren tots els possibles valors d'aquest tipus */
      for(
        evinfo_val  =
          (struct t_user_event_value_info *)head_queue(&evinfo->values);
        evinfo_val != (struct t_user_event_value_info *)0;
        evinfo_val  = 
          (struct t_user_event_value_info *)next_queue(&evinfo->values)
      )
      {
        /* Es posa l'etiqueta d'aquest valor */
        fprintf(fd, "%d\t%s\n", evinfo_val->value, evinfo_val->name);      
      }
      fprintf(fd,"\n\n");
    }
  }
  

  /* JGG: Parte final del PCF, también estática */
  for (i = 0; i < PCF_TAIL_LINES; i++)
  {
    fprintf(fd,"%s\n",pcf_tail[i]);
  }

  /* Es tanca el fitxer i es retorna que ha anat bé */
  fclose(fd);
  chmod (pcf_name, 0600);
  free(pcf_name);
  return(0);
}
