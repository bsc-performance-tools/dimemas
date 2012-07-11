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
#include <string.h>
#include <errno.h>

#include <Dimemas2Prv.h>
#include <EventEncoding.h>

#include <define.h>
#include <types.h>
#include "extern.h"

#include "list.h"
#include "pcf_defines.h"

//#define PUT_ALL_VALUES 1

/*****************************************************************************
 * Private functions
 ****************************************************************************/

static t_boolean PCF_copy_existing(FILE* input_pcf,
                                   FILE* output_pcf,
                                   FILE* pcf_insert);

static t_boolean PCF_generate_default(FILE* output_pcf, FILE* pcf_insert);

/* JGG: La generación de los colores asociados a los estados debería ser más
 * dinámica...
 * HABRÁ QUE REVISARLO */



/******************************************************************************
 ***  MakeParaverPCF
 ******************************************************************************/

t_boolean MakeParaverPCF(const char *output_trace_name,
                         const char *pcf_insert_name)
{
  struct t_Ptask* ptask;

  FILE *input_pcf_file, *output_pcf_file, *pcf_insert_file;
  char *input_pcf_name,  *output_pcf_name;

  /* Generate the output PCF name */
  output_pcf_name = strdup(output_trace_name);

  if (strcmp(&output_pcf_name[strlen(output_trace_name)-4],".prv") != 0)
  {
    warning ("Wrong extension in output Paraver trace. PCF file would be named wrong\n");
  }
  strcpy(&output_pcf_name[strlen(output_trace_name)-4],".pcf");

  if ( (output_pcf_file = IO_fopen(output_pcf_name, "w")) == NULL)
  {
    warning ("Unable to open output PCF file (%s): %s\n",
             output_pcf_name,
             IO_get_error());

    return FALSE;
  }

  /* Open the possible PCF insert */
  if (pcf_insert_name != NULL)
  {
    if ( (pcf_insert_file = IO_fopen(pcf_insert_name, "r")) == NULL)
    {
      warning ("Unable to open supplied PCF insert file (%s): %s\n",
               pcf_insert_name,
               IO_get_error());

      pcf_insert_file = NULL;
    }
  }
  else
  {
    pcf_insert_file = NULL;
  }

  /* The PCF would be a copy of the first trace PCF, if exists! */
  ptask  = (struct t_Ptask *) head_queue(&Ptask_queue);

  input_pcf_name = strdup(ptask->tracefile);

  if (input_pcf_name == NULL)
  {
    warning ("Unable to locate input PCF. A default would be generated\n");
    input_pcf_file = NULL;
  }
  else
  {
    strcpy(&input_pcf_name[strlen(input_pcf_name)-3],"pcf");

    printf("PCF to be copied = %s\n",
           input_pcf_name);

    if ( ( input_pcf_file = IO_fopen(input_pcf_name, "r")) == NULL)
    {
      warning ("Unable to open input PCF. A default would be generated\n");
      input_pcf_file = NULL;
    }
  }

  if (input_pcf_file != NULL)
  {
    return PCF_copy_existing(input_pcf_file, output_pcf_file, pcf_insert_file);
  }
  else
  {
    return PCF_generate_default(output_pcf_file, pcf_insert_file);
  }
}

static t_boolean PCF_copy_existing(FILE* input_pcf,
                                   FILE* output_pcf,
                                   FILE* pcf_insert)
{
  char*   line  = NULL;
  size_t  current_line_length = 0;
  ssize_t bytes_read;

  while ( (bytes_read = getline(&line, &current_line_length, input_pcf)) != -1)
  {
    if ( (pcf_insert != NULL) && ( strcmp (line, "GRADIENT COLOR\n") == 0))
    {
      char ch;

      while(!feof(pcf_insert))
      {
        ch = fgetc(pcf_insert);

        if(ferror(pcf_insert))
        {
          die("Error reading source cfg file to be included: %s\n",
              strerror(errno));
        }

        if(!feof(pcf_insert))
        {
          fputc(ch, output_pcf);
        }

        if(ferror(output_pcf))
        {
            printf("Error writing destination file when including the cfg file\n");
            exit(1);
        }
      }
      fprintf(output_pcf, "\n\n");

      IO_fclose(pcf_insert);
      pcf_insert = NULL;
    }

    fprintf(output_pcf, "%s", line);
    free(line);
    line = NULL;
  }

  if (!feof(input_pcf))
  {
    die ("Error reading input PCF: %s\n",
         strerror(errno));
  }

  /* Copy the PCF insert supplied if it wasn't before */
  if (pcf_insert != NULL)
  {
    char ch;

    while(!feof(pcf_insert))
    {
      ch = fgetc(pcf_insert);

      if(ferror(pcf_insert))
      {
        die("Error reading source cfg file to be included: %s\n",
            strerror(errno));
      }

      if(!feof(pcf_insert))
      {
        fputc(ch, output_pcf);
      }

      if(ferror(output_pcf))
      {
          die ("Error writing destination file when including the cfg file: %s\n",
               strerror(errno));
      }
    }
    fprintf(output_pcf, "\n\n");

    IO_fclose(pcf_insert);
    pcf_insert = NULL;
  }

  IO_fclose(output_pcf);

  return TRUE;
}

static t_boolean PCF_generate_default(FILE* output_pcf, FILE* pcf_insert)
{
  int i;
  struct t_Ptask *ptask;

  /* JGG: Escritura de la cabecera del PCF, estatica*/
  for (i = 0; i < PCF_HEAD_LINES; i++)
  {
    fprintf(output_pcf,"%s\n",pcf_head[i]);
  }

  /* JGG: Parte dinámica referente a los estados */
  fprintf(output_pcf, "STATES\n");
  for ( i = 0; i < PRV_STATE_COUNT; i++)
  {
    fprintf(
      output_pcf,
      "%d\t%s\n",
      i,
      ParaverDefaultPalette[i].name
    );
  }

  /* Dimemas private states */
  for ( i = 0; i < DIMEMAS_STATE_COUNT; i++)
  {
    fprintf(
      output_pcf,
      "%d\t%s\n",
      i + PRV_STATE_COUNT,
      DimemasDefaultPalette[i].name
    );
  }

  fprintf(output_pcf, "\nSTATES_COLOR\n");
  for ( i = 0; i < PRV_STATE_COUNT; i++)
  {
    fprintf(
      output_pcf,
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
      output_pcf,
      "%d\t{%d,%d,%d}\n",
      i + PRV_STATE_COUNT,
      DimemasDefaultPalette[i].RGB[0],
      DimemasDefaultPalette[i].RGB[1],
      DimemasDefaultPalette[i].RGB[2]
    );
  }

  fprintf(output_pcf, "\n");

  /* JGG: Escritura de la parte central estática */
  for(i = 0; i < PCF_MIDDLE_LINES; i++)
  {
    fprintf(output_pcf, "%s\n", pcf_middle[i]);
  }

//#ifdef PUT_ALL_VALUES
  /* JGG: That should be changed at certain point... */
  for (i = 0; i < NUM_MPICALLS; i++) {
     MPIEventEncoding_EnableOperation(i);
  }
  MPIEventEncoding_WriteEnabledOperations(output_pcf);
// #endif

  /* S'escriuen els events que ha definit l'usuari manualment
  for(ptask  = (struct t_Ptask *) head_queue(&Ptask_queue);
      ptask != P_NIL;
      ptask  = (struct t_Ptask *) next_queue(&Ptask_queue))
  {

#ifndef PUT_ALL_VALUES
    /* Es recorren tots els possibles grups de blocs
    for (i = 0; i < NUM_BLOCK_GROUPS; i++)
    {
      /* Es posa la capc,alera d'aquest tipus
      fprintf(output_pcf,
              "EVENT_TYPE\n%d\t%d\t%s\nVALUES\n0\tEnd\n",
              PRV_BLOCK_COLOR(i),
              PRV_BLOCK_TYPE(i),
              PRV_BLOCK_LABEL(i));

      /* DEBUG
      printf ("Printing %d\t%d\t%s\n",
              PRV_BLOCK_COLOR(i),
              PRV_BLOCK_TYPE(i),
              PRV_BLOCK_LABEL(i));
      */

      /* S'afegeixen com a valors tots els blocs utilitzats d'aquest grup
      for(mod  = (struct t_module*) head_queue (&ptask->Modules);
          mod != (struct t_module*) 0;
          mod  = (struct t_module*) next_queue (&ptask->Modules)
      )
      {
        /*if ((mod->activity_name != NULL) && (mod->block_name != NULL) &&
            (mod->used) &&
            (Block_Dimemas2Paraver_Type(mod->identificator) == PRV_BLOCK_TYPE(i)) */

        /* DEBUG
        printf("Current Type = %lld  CurrentName = %s\n",
               Block_Dimemas2Paraver_Type(mod->identificator),
               mod->block_name);
        */

        /*
        if ((mod->block_name != NULL) && (mod->used) &&
            (Block_Dimemas2Paraver_Type(mod->identificator) == PRV_BLOCK_TYPE(i)))
        {

          char* local_block_name;
          /* Es un block correctament definit i que ha estat utilitzat */
          /* No es mostra l'activitat per tal que la trac,a .prv quedi
           * exactament com si hagues estat creada directament amb el mpitrace.
          fprintf(output_pcf,"%d\t%s: %s\n",
                  BLOCK_TRF2PRV_VALUE(mod->identificator),
                  mod->activity_name,
                  mod->block_name);
        */

          /* DEBUG
          printf ("Printing Module %lld with name %s\n",
                  Block_Dimemas2Paraver_Value(mod->identificator),
                  mod->block_name);

          fprintf(output_pcf,
                  "%lld\t%s \n",
                  Block_Dimemas2Paraver_Value(mod->identificator),
                  mod->block_name);

        }
      }
      fprintf(output_pcf,"\n\n");

    }
#endif

    /* DEPRECATED: no longer available on DIM trace
    for(
      evinfo  = (struct t_user_event_info *) head_queue(&ptask->UserEventsInfo);
      evinfo != (struct t_user_event_info *) 0;
      evinfo  = (struct t_user_event_info *) next_queue(&ptask->UserEventsInfo)
    )
    {
      fprintf(
        output_pcf,
        "EVENT_TYPE\n%d\t%d\t%s\nVALUES\n",
        evinfo->color,
        evinfo->type,
        evinfo->name
      );

      for(evinfo_val  = (struct t_user_event_value_info *) head_queue (&evinfo->values);
          evinfo_val != (struct t_user_event_value_info *) 0;
          evinfo_val  = (struct t_user_event_value_info *) next_queue (&evinfo->values))
      {
        fprintf(output_pcf, "%d\t%s\n", evinfo_val->value, evinfo_val->name);
      }
      fprintf(output_pcf,"\n\n");
    }

  }
  */

  /* Copy the PCF insert supplied */
 if (pcf_insert != NULL)
 {
    char ch;

    while(!feof(pcf_insert))
    {
      ch = fgetc(pcf_insert);

      if(ferror(pcf_insert))
      {
        die("Error reading source cfg file to be included: %s\n",
            strerror(errno));
      }

      if(!feof(pcf_insert))
      {
        fputc(ch, output_pcf);
      }

      if(ferror(output_pcf))
      {
          printf("Error writing destination file when including the cfg file\n");
          exit(1);
      }
    }
    fprintf(output_pcf, "\n\n");

    IO_fclose(pcf_insert);
  }


  /* JGG: Parte final del PCF, también estática */
  for (i = 0; i < PCF_TAIL_LINES; i++)
  {
    fprintf(output_pcf,"%s\n",pcf_tail[i]);
  }

  /* Es tanca el fitxer i es retorna que ha anat bé */
  fflush(output_pcf);
  IO_fclose(output_pcf);

  // chmod (pcf_name, 0600);
  // free(pcf_name);
  return TRUE;
}
