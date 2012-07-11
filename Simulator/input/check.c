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

#include <unistd.h>

#include <define.h>
#include <types.h>

#include "yacc.h"
#include "list.h"
#include "sddf_records.h"
#include "check.h"

#include "configuration.h"


static int one_more_record;
static int aprox_number_of_records = 0;
static double last_value_load_in_curse = 0;
void  update_loading (double);
#ifdef LIBLEXYACC
#include "config.h"
#endif

struct t_queue Record_Definition;

struct t_queue   Registers_queue;
struct Str      *first_str;

/*
 * External variables defined in 'yacc.y'
 */
extern int       line_no;
extern int       yydebug;
extern char      yy_near_line[];
extern t_boolean yy_near_line_filled;
extern t_boolean dimemas_GUI;

char *types[] = {"integer", "double", "char", "array"};

t_boolean       definition_error = FALSE;
extern struct t_Ptask *Ptask_current;
extern int      max_task_id;

/*
 * Private functions
 */
static void check_wan_info_definition  (char *c, struct t_queue *q);
static void check_env_info_definition  (char *c, struct t_queue *q);
static void check_node_info_definition (char *c, struct t_queue *q);
static void check_mapping_definition   (char *c, struct t_queue *q);
static void check_conf_files_definition (char *c, struct t_queue *q);
static void check_mod_info_definition  (char *c, struct t_queue *q);
static void check_fs_params_definition (char *c, struct t_queue *q);
static void check_ded_conn_definition  (char *c, struct t_queue *q);

void near_line()
{
  warning ("\n* Parse error near line %d in file '%s':\n",
           line_no,
           CONFIGURATION_Get_Configuration_FileName() );
}

static void Invalid_attribute (int   field_number,
                               int   record_type,
                               char *record_name,
                               char *actual_field_name,
                               char *expected_field_name)
{
  near_line ();

  warning ("Invalid name for field number %d in record #%d: %s\n",
       field_number,
       record_type,
       record_name);

  warning("It is '%s' and should be '%s'\n",
          actual_field_name,
          expected_field_name);

  definition_error = TRUE;
}

static void Invalid_type (int   field_number,
                          char *field_name,
                          int   record_type,
                          char *record_name,
                          int   actual_field_type,
                          int   expected_field_type,
                          int   actual_dimensions,
                          int   expected_dimensions)
{
  near_line();

  warning ("Invalid type for field number % d ( % s) in record #%d: %s\n",
          field_number,
          field_name,
          record_type,
          record_name);

  warning("It is '%s' with %d dimension(s) and should be '%s' with %d dimension(s)\n",
          types[actual_field_type],
          actual_dimensions,
          types[expected_field_type],
          expected_dimensions);

  definition_error = TRUE;
}

int free_memory (void)
{
  struct t_entry   *entry;
  struct t_element *el;
  struct t_array   *ar;
  struct Str       *str, *n;

  for (entry  = (struct t_entry *) outFIFO_queue (&Registers_queue);
       entry != (struct t_entry *) 0;
       entry  = (struct t_entry *) outFIFO_queue (&Registers_queue) )
  {
    for (el  = (struct t_element *) outFIFO_queue (entry->types);
         el != (struct t_element *) 0;
         el  = (struct t_element *) outFIFO_queue (entry->types) )
    {
      ar = el->i3;
      MALLOC_free_memory ( (char *) ar);
      MALLOC_free_memory ( (char *) el);
    }
    MALLOC_free_memory ( (char *) entry->types);
    MALLOC_free_memory ( (char *) entry);
  }

  str = first_str;
  while (str != (struct Str *) 0)
  {
    n = str->next_str;
    MALLOC_free_memory (str->str);
    MALLOC_free_memory ( (char *) str);
    str = n;
  }

  if (definition_error)
    return (ERROR);

  return (NO_ERROR);
}


/*****************************************************************************
 * CONFIGURATION FILE DEFINITION RECORDS
 ****************************************************************************/

struct t_entry* exist_identifier (char           *register_name,
                                  struct t_queue *q)
{
  struct t_entry *entry;

  if (&Registers_queue == NULL)
  {
    return FALSE;
  }

  for (entry  = (struct t_entry *) head_queue (q);
       entry != (struct t_entry *) 0;
       entry  = (struct t_entry *) next_queue (q) )
  {
    if (strcmp (entry->name, register_name) == 0)
    {
      return (entry);
    }
  }

  return NULL;
}

t_boolean all_definitions_exist (void)
{
  struct t_entry   *en;
  struct t_element *el;
  t_boolean     id0 = FALSE, id1 = FALSE, id2 = FALSE, id3 = FALSE, id4 = FALSE,
                                   id5 = FALSE, id6 = FALSE, id7 = FALSE;

  for (en  = (struct t_entry *) head_queue (&Registers_queue);
       en != (struct t_entry *) 0;
       en  = (struct t_entry *) next_queue (&Registers_queue) )
  {
    switch (en->entryid)
    {
    case 0:
      id0 = TRUE;
      break;
    case 1:
      id1 = TRUE;
      break;
    case 2:
      id2 = TRUE;
      break;
    case 3:
      id3 = TRUE;
      break;
    case 4:
      id4 = TRUE;
      break;
    case 5:
      id5 = TRUE;
      break;
    case 6:
      id6 = TRUE;
      break;
    case 7:
      id7 = TRUE;
      break;

    default:
      warning ("Error, not recognized field definition\n");
      break;
    }

    if (yydebug)
    {
      warning ("Entry id: %d, Name: %s\n", en->entryid, en->name);

      for (el = (struct t_element *) head_queue (en->types);
           el != (struct t_element *) 0;
           el = (struct t_element *) next_queue (en->types) )
      {
        warning ("\t %s(%d), %d\n",
                 el->i3->i1,
                 el->i3->dimension,
                 el->type);
      }
    }
  }

  if ( (definition_error) )
  {
    near_line();
    warning ("Some mandatory definition missed or incorrect\n");

    if (!id0)
      warning ("Definition for #0: \"wide area network information\"\n");

    if (!id1)
      warning ("Definition for #1: \"environment information\"\n");

    if (!id2)
      warning ("Definition for #2: \"node information\"\n");

    if (!id3)
      warning ("Definition for #3: \"mapping information\"\n");

    if (!id4)
      warning ("Definition for #4: \"configuration files\"\n");

    if (!id5)
      warning ("Definition for #5: \"module information\"\n");

    if (!id6)
      warning ("Definition for #6: \"file system parameters\"\n");

    if (!id7)
      warning ("Definition for #7: \"dedicated connection information\"\n");

    return (FALSE);
  }

  return TRUE;
}

t_boolean add_register_definition (int             definition_type,
                                   char           *definition_name,
                                   struct t_queue *q)
{
  struct t_entry *entry;

  switch (definition_type)
  {
  case 0:
    check_wan_info_definition (definition_name, q);
    break;
  case 1:
    check_env_info_definition (definition_name, q);
    break;
  case 2:
    check_node_info_definition (definition_name, q);
    break;
  case 3:
    check_mapping_definition (definition_name, q);
    break;
  case 4:
    check_conf_files_definition (definition_name, q);
    break;
  case 5:
    check_mod_info_definition (definition_name, q);
    break;
  case 6:
    check_fs_params_definition (definition_name, q);
    break;
  case 7:
    check_ded_conn_definition (definition_name, q);
    break;
  default:
    near_line();
    warning ("Invalid record number %d: %s\n",
             definition_type,
             definition_name);

    definition_error = TRUE;
    return FALSE;
  }

  if (&Registers_queue == NULL)
  {
    return (FALSE);
  }

  for (entry  = (struct t_entry *) head_queue (&Registers_queue);
       entry != (struct t_entry *) 0;
       entry  = (struct t_entry *) next_queue (&Registers_queue) )
  {
    if ( (entry->entryid == definition_type) && (entry->name == definition_name) )
    {
      return TRUE;
    }
  }

  return (FALSE);
}

void check_wan_info_definition (char *c, struct t_queue *q)
{
  struct t_element *el;

  if (strcmp (c, SDDFA_0C_NAME) != 0)
  {
    near_line ();
    warning ("Incorrect name for register #0\n");
    warning ("It is %s and must be %s\n", c, SDDFA_0C_NAME);
    return;
  }
  if (count_queue (q) != 7)
  {
    near_line ();
    warning ("Incorrect number of elements in register #0: %s\n", c);;
    warning ("There are %d and must be 7\n", count_queue (q) );
    return;
  }

  el = (struct t_element *) head_queue (q);
  if (strcmp (el->i3->i1, SDDFA_0C_1A) != 0)
  {
    Invalid_attribute (1, 0, c, el->i3->i1, SDDFA_0C_1A);
  }

  if ( (el->type != TYPE_CHAR) || (el->i3->dimension != 1) )
  {

    Invalid_type (1, el->i3->i1, 0, c, el->type, TYPE_CHAR, el->i3->dimension, 1);
  }

  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_0C_2A) != 0)
  {

    Invalid_attribute (2, 0, c, el->i3->i1, SDDFA_0C_2A);
  }

  if ( (el->type != TYPE_INTEGER) || (el->i3->dimension != 0) )
  {

    Invalid_type (2, el->i3->i1, 0, c, el->type, TYPE_INTEGER, el->i3->dimension, 0);

  }

  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_0C_3A) != 0)
  {

    Invalid_attribute (3, 0, c, el->i3->i1, SDDFA_0C_3A);
  }

  if ( (el->type != TYPE_INTEGER) || (el->i3->dimension != 0) )
  {

    Invalid_type (3, el->i3->i1, 0, c, el->type, TYPE_INTEGER, el->i3->dimension, 0);
  }

  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_0C_4A) != 0)
  {

    Invalid_attribute (4, 0, c, el->i3->i1, SDDFA_0C_4A);
  }

  if ( (el->type != TYPE_INTEGER) || (el->i3->dimension != 0) )
  {

    Invalid_type (4, el->i3->i1, 0, c, el->type, TYPE_INTEGER, el->i3->dimension, 0);
  }

  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_0C_5A) != 0)
  {

    Invalid_attribute (5, 0, c, el->i3->i1, SDDFA_0C_5A);
  }

  if ( (el->type != TYPE_DOUBLE) || (el->i3->dimension != 0) )
  {

    Invalid_type (5, el->i3->i1, 0, c, el->type, TYPE_DOUBLE, el->i3->dimension, 0);
  }

  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_0C_6A) != 0)
  {
    Invalid_attribute (6, 0, c, el->i3->i1, SDDFA_0C_6A);

  }
  if ( (el->type != TYPE_DOUBLE) || (el->i3->dimension != 0) )
  {
    Invalid_type (6, el->i3->i1, 0, c, el->type, TYPE_DOUBLE, el->i3->dimension, 0);

  }

  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_0C_7A) != 0)
  {
    Invalid_attribute (7, 0, c, el->i3->i1, SDDFA_0C_7A);

  }
  if ( (el->type != TYPE_INTEGER) || (el->i3->dimension != 0) )
  {
    Invalid_type (7, el->i3->i1, 0, c, el->type, TYPE_INTEGER, el->i3->dimension, 0);

  }
}

void check_env_info_definition (char *c, struct t_queue *q)
{
  struct t_element *el;
  if (strcmp (c, SDDFA_1C_NAME) != 0)
  {
    near_line();
    warning ("Incorrect name for register #1\n");
    warning ("It is %s and must be %s\n", c, SDDFA_1C_NAME);
    return;
  }

  /* Es comproven els camps segons el format nou de DAMIEN */
  el = (struct t_element *) head_queue (q);
  if (strcmp (el->i3->i1, SDDFA_1C_1A) != 0)
  {

    Invalid_attribute (1, 1, c, el->i3->i1, SDDFA_1C_1A);
  }

  if ( (el->type != TYPE_CHAR) || (el->i3->dimension != 1) )
  {

    Invalid_type (1, el->i3->i1, 1, c, el->type, TYPE_CHAR, el->i3->dimension, 1);
  }

  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_1C_2A) != 0)
  {

    Invalid_attribute (2, 1, c, el->i3->i1, SDDFA_1C_2A);
  }

  if ( (el->type != TYPE_INTEGER) || (el->i3->dimension != 0) )
  {

    Invalid_type (2, el->i3->i1, 1, c, el->type, TYPE_INTEGER, el->i3->dimension, 0);
  }

  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_1C_3A) != 0)
  {
    Invalid_attribute (3, 1, c, el->i3->i1, SDDFA_1C_3A);

  }
  if ( (el->type != TYPE_CHAR) || (el->i3->dimension != 1) )
  {
    Invalid_type (3, el->i3->i1, 1, c, el->type, TYPE_CHAR, el->i3->dimension, 1);

  }

  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_1C_4A) != 0)
  {
    Invalid_attribute (4, 1, c, el->i3->i1, SDDFA_1C_4A);

  }
  if ( (el->type != TYPE_INTEGER) || (el->i3->dimension != 0) )
  {
    Invalid_type (4, el->i3->i1, 1, c, el->type, TYPE_INTEGER, el->i3->dimension, 0);

  }

  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_1C_5A) != 0)
  {
    Invalid_attribute (5, 1, c, el->i3->i1, SDDFA_1C_5A);

  }
  if ( (el->type != TYPE_DOUBLE) || (el->i3->dimension != 0) )
  {
    Invalid_type (5, el->i3->i1, 1, c, el->type, TYPE_DOUBLE, el->i3->dimension, 0);

  }

  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_1C_6A) != 0)
  {
    Invalid_attribute (6, 1, c, el->i3->i1, SDDFA_1C_6A);

  }
  if ( (el->type != TYPE_INTEGER) || (el->i3->dimension != 0) )
  {
    Invalid_type (6, el->i3->i1, 1, c, el->type, TYPE_INTEGER, el->i3->dimension, 0);

  }

  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_1C_7A) != 0)
  {
    Invalid_attribute (7, 1, c, el->i3->i1, SDDFA_1C_7A);

  }
  if ( (el->type != TYPE_INTEGER) || (el->i3->dimension != 0) )
  {
    Invalid_type (7, el->i3->i1, 1, c, el->type, TYPE_INTEGER, el->i3->dimension, 0);

  }

}

void check_node_info_definition (char *c, struct t_queue *q)
{
  struct t_element *el;

  if (strcmp (c, SDDFA_2C_NAME) != 0)
  {
    warning ("Incorrect name for register #2\n");
    warning ("It is %s and must be %s\n", c, SDDFA_2C_NAME);

    return;
  }

  el = (struct t_element *) head_queue (q);
  if (strcmp (el->i3->i1, SDDFA_2C_1A) != 0)
  {
    Invalid_attribute (1, 2, c, el->i3->i1, SDDFA_2C_1A);
  }
  if ( (el->type != TYPE_INTEGER) || (el->i3->dimension != 0) )
  {
    Invalid_type (1, el->i3->i1, 2, c, el->type, TYPE_INTEGER, el->i3->dimension, 0);
  }

  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_2C_2A) != 0)
  {
    Invalid_attribute (2, 2, c, el->i3->i1, SDDFA_2C_2A);
  }
  if ( (el->type != TYPE_INTEGER) || (el->i3->dimension != 0) )
  {
    Invalid_type (2, el->i3->i1, 2, c, el->type, TYPE_INTEGER, el->i3->dimension, 0);

  }

  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_2C_3A) != 0)
  {
    Invalid_attribute (3, 2, c, el->i3->i1, SDDFA_2C_3A);

  }
  if ( (el->type != TYPE_CHAR) || (el->i3->dimension != 1) )
  {
    Invalid_type (3, el->i3->i1, 2, c, el->type, TYPE_CHAR, el->i3->dimension, 1);

  }

  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_2C_4A) != 0)
  {
    Invalid_attribute (4, 2, c, el->i3->i1, SDDFA_2C_4A);

  }
  if ( (el->type != TYPE_INTEGER) || (el->i3->dimension != 0) )
  {
    Invalid_type (4, el->i3->i1, 2, c, el->type, TYPE_INTEGER, el->i3->dimension, 0);

  }

  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_2C_5A) != 0)
  {
    Invalid_attribute (5, 2, c, el->i3->i1, SDDFA_2C_5A);

  }
  if ( (el->type != TYPE_INTEGER) || (el->i3->dimension != 0) )
  {
    Invalid_type (5, el->i3->i1, 2, c, el->type, TYPE_INTEGER, el->i3->dimension, 0);

  }

  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_2C_6A) != 0)
  {
    Invalid_attribute (6, 2, c, el->i3->i1, SDDFA_2C_6A);

  }
  if ( (el->type != TYPE_INTEGER) || (el->i3->dimension != 0) )
  {
    Invalid_type (6, el->i3->i1, 2, c, el->type, TYPE_INTEGER, el->i3->dimension, 0);

  }

  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_2C_7A) != 0)
  {
    Invalid_attribute (7, 2, c, el->i3->i1, SDDFA_2C_7A);

  }
  if ( (el->type != TYPE_DOUBLE) || (el->i3->dimension != 0) )
  {
    Invalid_type (7, el->i3->i1, 2, c, el->type, TYPE_DOUBLE, el->i3->dimension, 0);

  }

  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_2C_8A) != 0)
  {
    Invalid_attribute (8, 2, c, el->i3->i1, SDDFA_2C_8A);

  }
  if ( (el->type != TYPE_DOUBLE) || (el->i3->dimension != 0) )
  {
    Invalid_type (8, el->i3->i1, 2, c, el->type, TYPE_DOUBLE, el->i3->dimension, 0);

  }

  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_2C_9A) != 0)
  {
    Invalid_attribute (9, 2, c, el->i3->i1, SDDFA_2C_9A);

  }
  if ( (el->type != TYPE_DOUBLE) || (el->i3->dimension != 0) )
  {
    Invalid_type (9, el->i3->i1, 2, c, el->type, TYPE_DOUBLE, el->i3->dimension, 0);

  }

  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_2C_10A) != 0)
  {
    Invalid_attribute (10, 2, c, el->i3->i1, SDDFA_2C_10A);

  }
  if ( (el->type != TYPE_DOUBLE) || (el->i3->dimension != 0) )
  {
    Invalid_type (10, el->i3->i1, 2, c, el->type, TYPE_DOUBLE, el->i3->dimension, 0);
  }

  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_2C_11A) != 0)
  {
    Invalid_attribute (11, 2, c, el->i3->i1, SDDFA_2C_11A);

  }
  if ( (el->type != TYPE_DOUBLE) || (el->i3->dimension != 0) )
  {
    Invalid_type (11, el->i3->i1, 2, c, el->type, TYPE_DOUBLE, el->i3->dimension, 0);

  }

  if (count_queue (q) == 15)
  {
    el = (struct t_element *) next_queue (q);
    if (strcmp (el->i3->i1, SDDFA_2C_12A) != 0)
    {
      Invalid_attribute (12, 2, c, el->i3->i1, SDDFA_2C_12A);

    }
    if ( (el->type != TYPE_DOUBLE) || (el->i3->dimension != 0) )
    {
      Invalid_type (12, el->i3->i1, 2, c, el->type, TYPE_DOUBLE, el->i3->dimension, 0);

    }

    el = (struct t_element *) next_queue (q);
    if (strcmp (el->i3->i1, SDDFA_2C_13A) != 0)
    {
      Invalid_attribute (13, 2, c, el->i3->i1, SDDFA_2C_13A);

    }
    if ( (el->type != TYPE_DOUBLE) || (el->i3->dimension != 0) )
    {
      Invalid_type (13, el->i3->i1, 2, c, el->type, TYPE_DOUBLE, el->i3->dimension, 0);

    }

    el = (struct t_element *) next_queue (q);
    if (strcmp (el->i3->i1, SDDFA_2C_14A) != 0)
    {
      Invalid_attribute (14, 2, c, el->i3->i1, SDDFA_2C_14A);

    }
    if ( (el->type != TYPE_DOUBLE) || (el->i3->dimension != 0) )
    {
      Invalid_type (14, el->i3->i1, 2, c, el->type, TYPE_DOUBLE, el->i3->dimension, 0);

    }

    el = (struct t_element *) next_queue (q);
    if (strcmp (el->i3->i1, SDDFA_2C_15A) != 0)
    {
      Invalid_attribute (15, 2, c, el->i3->i1, SDDFA_2C_15A);

    }
    if ( (el->type != TYPE_DOUBLE) || (el->i3->dimension != 0) )
    {
      Invalid_type (15, el->i3->i1, 2, c, el->type, TYPE_DOUBLE, el->i3->dimension, 0);
    }
  }
}

void check_mapping_definition (char *c, struct t_queue *q)
{
  struct t_element *el;

  if (strcmp (c, SDDFA_3C_NAME) != 0)
  {
    near_line ();
    warning ("Incorrect name for register #3\n");
    warning ("It is %s and must be %s\n", c, SDDFA_3C_NAME);
    return;
  }

  if ( (count_queue (q) != 4) && (count_queue (q) != 3) )
  {
    near_line ();
    warning ("Incorrect number of elements in register #3: %s\n", c);
    warning ("There are %d and must be 3 or 4\n", count_queue (q) );
    return;
  }

  el = (struct t_element *) head_queue (q);
  if (strcmp (el->i3->i1, SDDFA_3C_1A) != 0)
  {
    Invalid_attribute (1, 3, c, el->i3->i1, SDDFA_3C_1A);

  }
  if ( (el->type != TYPE_CHAR) || (el->i3->dimension != 1) )
  {
    Invalid_type (1, el->i3->i1, 3, c, el->type, 2, el->i3->dimension, 1);

  }

  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_3C_2A) != 0)
  {
    Invalid_attribute (2, 3, c, el->i3->i1, SDDFA_3C_2A);

  }
  if ( (el->type != TYPE_INTEGER) || (el->i3->dimension != 0) )
  {
    Invalid_type (2, el->i3->i1, 3, c, el->type, 0, el->i3->dimension, 0);

  }

  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_3C_3A) != 0)
  {
    Invalid_attribute (3, 3, c, el->i3->i1, SDDFA_3C_3A);

  }
  if ( (el->type != TYPE_INTEGER) || (el->i3->dimension != 1) )
  {
    Invalid_type (3, el->i3->i1, 3, c, el->type, 0, el->i3->dimension, 1);

  }

  if (count_queue (q) == 4)
  {
    el = (struct t_element *) next_queue (q);
    if (strcmp (el->i3->i1, SDDFA_3C_4A) != 0)
    {
      Invalid_attribute (3, 3, c, el->i3->i1, SDDFA_3C_4A);

    }
    if ( (el->type != TYPE_INTEGER) || (el->i3->dimension != 1) )
    {
      Invalid_type (3, el->i3->i1, 3, c, el->type, 0, el->i3->dimension, 1);

    }
  }

}

void check_conf_files_definition (char *c, struct t_queue *q)
{
  struct t_element *el;

  if (strcmp (c, SDDFA_4C_NAME) != 0)
  {
    near_line ();
    warning ("Incorrect name for register #4\n");
    warning ("It is %s and must be %s\n", c, SDDFA_4C_NAME);
    return;
  }

  if (count_queue (q) != 4)
  {
    near_line ();
    warning ("Incorrect number of elements in register #4: %s\n", c);
    warning ("There are %d and must be 4\n", count_queue (q) );
    return;
  }

  el = (struct t_element *) head_queue (q);
  if (strcmp (el->i3->i1, SDDFA_4C_1A) != 0)
  {
    Invalid_attribute (1, 4, c, el->i3->i1, SDDFA_4C_1A);

  }
  if ( (el->type != TYPE_CHAR) || (el->i3->dimension != 1) )
  {
    Invalid_type (1, el->i3->i1, 4, c, el->type, 2, el->i3->dimension, 1);

  }

  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_4C_2A) != 0)
  {
    Invalid_attribute (2, 4, c, el->i3->i1, SDDFA_4C_2A);

  }
  if ( (el->type != TYPE_CHAR) || (el->i3->dimension != 1) )
  {
    Invalid_type (2, el->i3->i1, 4, c, el->type, 2, el->i3->dimension, 1);

  }

  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_4C_3A) != 0)
  {
    Invalid_attribute (3, 4, c, el->i3->i1, SDDFA_4C_3A);

  }
  if ( (el->type != TYPE_CHAR) || (el->i3->dimension != 1) )
  {
    Invalid_type (3, el->i3->i1, 4, c, el->type, 2, el->i3->dimension, 1);

  }

  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_4C_4A) != 0)
  {
    Invalid_attribute (4, 4, c, el->i3->i1, SDDFA_4C_4A);

  }
  if ( (el->type != TYPE_CHAR) || (el->i3->dimension != 1) )
  {
    Invalid_type (4, el->i3->i1, 4, c, el->type, 2, el->i3->dimension, 1);

  }

}

void check_mod_info_definition (char           *record_name,
                                struct t_queue *record_attributes)
{
  struct t_element *el;

  if (strcmp (record_name, SDDFA_5C_NAME) != 0)
  {
    near_line ();
    warning ("Incorrect name for register #5\n");
    warning ("It is '%s' and must be '%s'\n",
         record_name,
         SDDFA_5C_NAME);
    return;
  }

  if (count_queue (record_attributes) != 3)
  {
    near_line ();
    warning ("Incorrect number of elements in register #5: %s\n",
         record_name);
    warning ("There are %d and must be 3\n",
         count_queue (record_attributes) );
    return;
  }

  /* Type */
  el = (struct t_element*) head_queue (record_attributes);
  if (strcmp (el->i3->i1, SDDFA_5C_1A) != 0)
  {
    Invalid_attribute (1, 5, record_name, el->i3->i1, SDDFA_5C_1A);
    // Invalid_attribute (1, 1, c, el->i3->i1, SDDFA_5C_1A);
    near_line();
  }

  if ( (el->type != TYPE_INTEGER) || (el->i3->dimension != 0) )
  {
    Invalid_type (1,
                  el->i3->i1,
                  5,
                  record_name,
                  el->type,
                  0,
                  el->i3->dimension,
                  0);
    // Invalid_type (1, el->i3->i1, 1, c, el->type, 0, el->i3->dimension, 0);

  }

  /* Value */
  el = (struct t_element *) next_queue (record_attributes);
  if (strcmp (el->i3->i1, SDDFA_5C_2A) != 0)
  {
    Invalid_attribute (2, 5, record_name, el->i3->i1, SDDFA_5C_2A);
    // Invalid_attribute (1, 1, c, el->i3->i1, SDDFA_5C_1A);

  }

  if ( (el->type != TYPE_INTEGER) || (el->i3->dimension != 0) )
  {
    Invalid_type (2,
                  el->i3->i1,
                  5,
                  record_name,
                  el->type,
                  0,
                  el->i3->dimension,
                  0);
    // Invalid_type (1, el->i3->i1, 1, c, el->type, 0, el->i3->dimension, 0)

  }

  /* Ratio */
  el = (struct t_element *) next_queue (record_attributes);
  if (strcmp (el->i3->i1, SDDFA_5C_3A) != 0)
  {
    Invalid_attribute (3, 5, record_name, el->i3->i1, SDDFA_5C_3A);
    // Invalid_attribute (2, 1, c, el->i3->i1, SDDFA_5C_2A);

  }

  if ( (el->type != TYPE_DOUBLE) || (el->i3->dimension != 0) )
  {
    Invalid_type (3,
                  el->i3->i1,
                  5,
                  record_name,
                  el->type,
                  1,
                  el->i3->dimension,
                  0);

  }
}

void check_fs_params_definition (char *c, struct t_queue *q)
{
  struct t_element *el;

  if (strcmp (c, SDDFA_6C_NAME) != 0)
  {
    near_line ();
    warning ("Incorrect name for register #6\n");
    warning ("It is %s and must be %s\n", c, SDDFA_6C_NAME);
    return;
  }

  if (count_queue (q) != 5)
  {
    near_line ();
    warning ("Incorrect number of elements in register #6: %s\n", c);
    warning ("There are %d and must be 5\n", count_queue (q) );
    return;
  }

  el = (struct t_element *) head_queue (q);
  if (strcmp (el->i3->i1, SDDFA_6C_1A) != 0)
  {
    Invalid_attribute (1, 6, c, el->i3->i1, SDDFA_6C_1A);

  }
  if ( (el->type != TYPE_DOUBLE) || (el->i3->dimension != 0) )
  {
    Invalid_type (1, el->i3->i1, 6, c, el->type, TYPE_DOUBLE, el->i3->dimension, 0);

  }

  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_6C_2A) != 0)
  {
    Invalid_attribute (2, 6, c, el->i3->i1, SDDFA_6C_2A);

  }
  if ( (el->type != TYPE_DOUBLE) || (el->i3->dimension != 0) )
  {
    Invalid_type (2, el->i3->i1, 6, c, el->type, TYPE_DOUBLE, el->i3->dimension, 0);

  }

  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_6C_3A) != 0)
  {
    Invalid_attribute (3, 6, c, el->i3->i1, SDDFA_6C_3A);

  }
  if ( (el->type != TYPE_DOUBLE) || (el->i3->dimension != 0) )
  {
    Invalid_type (3, el->i3->i1, 6, c, el->type, TYPE_DOUBLE, el->i3->dimension, 0);

  }

  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_6C_4A) != 0)
  {
    Invalid_attribute (4, 6, c, el->i3->i1, SDDFA_6C_4A);

  }
  if ( (el->type != TYPE_INTEGER) || (el->i3->dimension != 0) )
  {
    Invalid_type (4, el->i3->i1, 6, c, el->type, 0, el->i3->dimension, 0);

  }

  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_6C_5A) != 0)
  {
    Invalid_attribute (5, 6, c, el->i3->i1, SDDFA_6C_5A);

  }
  if ( (el->type != TYPE_DOUBLE) || (el->i3->dimension != 0) )
  {
    Invalid_type (5, el->i3->i1, 6, c, el->type, TYPE_DOUBLE, el->i3->dimension, 0);

  }
}

void check_ded_conn_definition (char *c, struct t_queue *q)
{
  struct t_element *el;

  if (strcmp (c, SDDFA_7C_NAME) != 0)
  {
    near_line ();
    warning ("Incorrect name for register #7\n");
    warning ("It is %s and must be %s\n", c, SDDFA_7C_NAME);
    return;
  }

  if (count_queue (q) != 13)
  {
    near_line ();
    warning ("Incorrect number of elements in register #7: %s\n", c);;
    warning ("There are %d and must be 13\n", count_queue (q) );
    return;
  }

  el = (struct t_element *) head_queue (q);
  if (strcmp (el->i3->i1, SDDFA_7C_1A) != 0)
  {
    Invalid_attribute (1, 7, c, el->i3->i1, SDDFA_7C_1A);

  }
  if ( (el->type != TYPE_INTEGER) || (el->i3->dimension != 0) )
  {
    Invalid_type (1, el->i3->i1, 7, c, el->type, TYPE_INTEGER, el->i3->dimension, 0);

  }

  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_7C_2A) != 0)
  {
    Invalid_attribute (2, 7, c, el->i3->i1, SDDFA_7C_2A);

  }
  if ( (el->type != TYPE_INTEGER) || (el->i3->dimension != 0) )
  {
    Invalid_type (2, el->i3->i1, 7, c, el->type, TYPE_INTEGER, el->i3->dimension, 0);

  }

  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_7C_3A) != 0)
  {
    Invalid_attribute (3, 7, c, el->i3->i1, SDDFA_7C_3A);

  }
  if ( (el->type != TYPE_INTEGER) || (el->i3->dimension != 0) )
  {
    Invalid_type (3, el->i3->i1, 7, c, el->type, TYPE_INTEGER, el->i3->dimension, 0);

  }

  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_7C_4A) != 0)
  {
    Invalid_attribute (4, 7, c, el->i3->i1, SDDFA_7C_4A);

  }
  if ( (el->type != TYPE_DOUBLE) || (el->i3->dimension != 0) )
  {
    Invalid_type (4, el->i3->i1, 7, c, el->type, TYPE_DOUBLE, el->i3->dimension, 0);

  }

  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_7C_5A) != 0)
  {
    Invalid_attribute (5, 7, c, el->i3->i1, SDDFA_7C_5A);

  }
  if ( (el->type != TYPE_INTEGER) || (el->i3->dimension != 1) )
  {
    Invalid_type (5, el->i3->i1, 7, c, el->type, TYPE_INTEGER, el->i3->dimension, 1);

  }

  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_7C_6A) != 0)
  {
    Invalid_attribute (6, 7, c, el->i3->i1, SDDFA_7C_6A);

  }
  if ( (el->type != TYPE_INTEGER) || (el->i3->dimension != 0) )
  {
    Invalid_type (6, el->i3->i1, 7, c, el->type, TYPE_INTEGER, el->i3->dimension, 0);

  }

  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_7C_7A) != 0)
  {
    Invalid_attribute (7, 7, c, el->i3->i1, SDDFA_7C_7A);

  }
  if ( (el->type != TYPE_CHAR) || (el->i3->dimension != 1) )
  {
    Invalid_type (7, el->i3->i1, 7, c, el->type, TYPE_CHAR, el->i3->dimension, 1);

  }

  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_7C_8A) != 0)
  {
    Invalid_attribute (8, 7, c, el->i3->i1, SDDFA_7C_8A);

  }
  if ( (el->type != TYPE_CHAR) || (el->i3->dimension != 1) )
  {
    Invalid_type (8, el->i3->i1, 7, c, el->type, TYPE_CHAR, el->i3->dimension, 1);

  }

  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_7C_9A) != 0)
  {
    Invalid_attribute (9, 7, c, el->i3->i1, SDDFA_7C_9A);

  }
  if ( (el->type != TYPE_INTEGER) || (el->i3->dimension != 0) )
  {
    Invalid_type (9, el->i3->i1, 7, c, el->type, TYPE_INTEGER, el->i3->dimension, 0);

  }

  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_7C_10A) != 0)
  {
    Invalid_attribute (10, 7, c, el->i3->i1, SDDFA_7C_10A);

  }
  if ( (el->type != TYPE_CHAR) || (el->i3->dimension != 1) )
  {
    Invalid_type (10, el->i3->i1, 7, c, el->type, TYPE_CHAR, el->i3->dimension, 1);

  }

  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_7C_11A) != 0)
  {
    Invalid_attribute (11, 7, c, el->i3->i1, SDDFA_7C_11A);

  }
  if ( (el->type != TYPE_INTEGER) || (el->i3->dimension != 1) )
  {
    Invalid_type (11, el->i3->i1, 7, c, el->type, TYPE_INTEGER, el->i3->dimension, 1);

  }

  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_7C_12A) != 0)
  {
    Invalid_attribute (12, 7, c, el->i3->i1, SDDFA_7C_12A);

  }
  if ( (el->type != TYPE_DOUBLE) || (el->i3->dimension != 0) )
  {
    Invalid_type (12, el->i3->i1, 7, c, el->type, TYPE_DOUBLE, el->i3->dimension, 0);

  }


  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_7C_13A) != 0)
  {
    Invalid_attribute (13, 7, c, el->i3->i1, SDDFA_7C_13A);

  }
  if ( (el->type != TYPE_DOUBLE) || (el->i3->dimension != 0) )
  {
    Invalid_type (13, el->i3->i1, 7, c, el->type, TYPE_DOUBLE, el->i3->dimension, 0);

  }
}

