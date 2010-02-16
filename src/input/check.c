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
#include "ts.h"
#include "sddf_records.h"

#include "check.h"
#include "cpu.h"
#include "extern.h"
#include "fs.h"
#include "list.h"
#include "mallocame.h"
#include "random.h"
#include "subr.h"
#include "task.h"
#include "yacc.h"

static int one_more_record;
static int aprox_number_of_records = 0;
static double last_value_load_in_curse=0;
void  update_loading(double);
#ifdef LIBLEXYACC
#include "config.h"
#endif

struct t_queue Record_Definition;

extern int      line_no;
extern int      yydebug;
extern char     yy_near_line[];
extern t_boolean yy_near_line_filled;
extern t_boolean dimemas_GUI;

t_boolean       definition_error = FALSE;
extern t_boolean configuration_file;
extern struct t_Ptask *Ptask_current;
extern char    *current_file;
extern int      max_task_id;

char           *s[] = {"integer", "double", "char", "array"};

#ifndef LIBLEXYACC
void  update_loading(double a)
{
}

static void 
new_block_identifier(int i)
{
}
static void 
new_block_name(int i, char *n, char *a, int f, int l)
{
}
static void 
new_file_name (int f, char *l)
{
}
#endif

static void
Invalid_attribute (int i, int j, char *c, char *d, char *e)
{
  fill_parse_error ("Invalid name for field number %d in record #%d: %s\n", 
                    i, j, c);

  fill_parse_error ("It is %s and should be %s\n", d, e);
}

static void
Invalid_type (int i, char *h, int j, char *c, int d, int e, int g, int l)
{
  fill_parse_error ("Invalid type for field number %d (%s) in record #%d: %s\n",
                    i, h, j, c);

  fill_parse_error ("It is %s with %d dimension and should be %s with %d dimension\n",
                    s[d], g, s[e], l);
}

void
near_line()
{
   if (dimemas_GUI)
      if (yy_near_line_filled)
	 return;
   if (dimemas_GUI)
      sprintf (yy_near_line, "Near line %d in file %s\n", line_no, current_file);
   else
      fprintf (stderr, "Near line %d in file %s\n", line_no, current_file);
   definition_error = TRUE;
}

void
check_record_1(char *c, struct t_queue *q)
{
  struct t_element *el;
  
  if (strcmp (c, SDDFA_1_NAME) != 0)
  {
    fill_parse_error ("Incorrect name for register #1\n");
    fill_parse_error ("It is %s and must be %s\n", c, SDDFA_1_NAME);
    near_line ();
    return;
  }

  if (count_queue (q) != 3)
  {
    fill_parse_error ("Incorrect number of elements in register #1: %s\n", c);
    fill_parse_error ("There are %d and must be 3\n", count_queue (q));
    near_line ();
  }
  
  el = (struct t_element *) head_queue (q);
  if (strcmp (el->i3->i1, SDDFA_1_1A) != 0)
  {
    Invalid_attribute (1, 1, c, el->i3->i1, SDDFA_1_1A);
    near_line ();
  }

  if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
  {
    Invalid_type (1, el->i3->i1, 1, c, el->type, 0, el->i3->dimension, 0);
    near_line ();
  }
  
  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_1_2A) != 0)
  {
    Invalid_attribute (2, 1, c, el->i3->i1, SDDFA_1_2A);
    near_line ();
  }

  if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
  {
    Invalid_type (2, el->i3->i1, 1, c, el->type, 0, el->i3->dimension, 0);
    near_line ();
  }
  
  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_1_3A) != 0)
  {
    Invalid_attribute (3, 1, c, el->i3->i1, SDDFA_1_3A);
    near_line ();
  }

  if ((el->type != TYPE_DOUBLE) || (el->i3->dimension != 0))
  {
    Invalid_type (3, el->i3->i1, 1, c, el->type, 1, el->i3->dimension, 0);
    near_line ();
  }
}

void
check_record_2(char *c, struct t_queue *q)
{
  struct t_element *el;
  
  if (strcmp (c, SDDFA_2_NAME) != 0)
  {
    fill_parse_error ("Incorrect name for register #2\n");
    fill_parse_error ("It is %s and must be %s\n", c, SDDFA_2_NAME);
    near_line ();
    return;
  }
  if ((count_queue (q) < 5) || (count_queue (q) >7))
  {
    fill_parse_error ("Incorrect number of elements in register #2: %s\n", c);
    fill_parse_error ("There are %d and must be 5, 6 or 7\n", count_queue (q));
    near_line ();
  }
  
  el = (struct t_element *) head_queue (q);
  if (strcmp (el->i3->i1, SDDFA_2_1A) != 0)
  {
    Invalid_attribute (1, 2, c, el->i3->i1, SDDFA_2_1A);
    near_line ();
  }
  if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
  {
    Invalid_type (1, el->i3->i1, 2, c, el->type, 0, el->i3->dimension, 0);
    near_line ();
  }
  
  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_2_2A) != 0)
  {
    Invalid_attribute (2, 2, c, el->i3->i1, SDDFA_2_2A);
    near_line ();
  }
  if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
  {
    Invalid_type (2, el->i3->i1, 2, c, el->type, 0, el->i3->dimension, 0);
    near_line ();
  }
  
  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_2_3A) != 0)
  {
    Invalid_attribute (3, 2, c, el->i3->i1, SDDFA_2_3A);
    near_line ();
  }
  if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
  {
    Invalid_type (3, el->i3->i1, 2, c, el->type, 0, el->i3->dimension, 0);
    near_line ();
  }
  
  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_2_4A) != 0)
  {
    Invalid_attribute (4, 2, c, el->i3->i1, SDDFA_2_4A);
    near_line ();
  }
  if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
  {
    Invalid_type (4, el->i3->i1, 2, c, el->type, 0, el->i3->dimension, 0);
    near_line ();
  }
  
  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_2_5A) != 0)
  {
    Invalid_attribute (5, 2, c, el->i3->i1, SDDFA_2_5A);
    near_line ();
  }
  if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
  {
    Invalid_type (5, el->i3->i1, 2, c, el->type, 0, el->i3->dimension, 0);
    near_line ();
  }
  
  
  el = (struct t_element *) next_queue (q);
  if (el==(struct t_element *)0) return;
  if (strcmp (el->i3->i1, SDDFA_2_6A) != 0)
  {
    Invalid_attribute (6, 2, c, el->i3->i1, SDDFA_2_6A);
    near_line ();
  }
  if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
  {
    Invalid_type (6, el->i3->i1, 2, c, el->type, 0, el->i3->dimension, 0);
    near_line ();
  }
  
  
  el = (struct t_element *) next_queue (q);
  if (el==(struct t_element *)0) return;
  
  /* (20/04/2005): Compatibilidad con el formato nuevo de traza */
  if (
  strcmp (el->i3->i1, SDDFA_2_7A) != 0 &&
  strcmp (el->i3->i1, SDDFA_2_7A_OLD) != 0
  )
  {
    char *message;
    sprintf (message, "%s or %s", SDDFA_2_7A, SDDFA_2_7A_OLD);
   
    Invalid_attribute (7, 2, c, el->i3->i1, message);
    near_line ();
  }
  if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
  {
    Invalid_type (7, el->i3->i1, 2, c, el->type, 0, el->i3->dimension, 0);
    near_line ();
  }
}

void
check_record_3(char *c, struct t_queue *q)
{
  struct t_element *el;
  
  if (strcmp (c, SDDFA_3_NAME) != 0)
  {
    fill_parse_error ("Incorrect name for register #3\n");
    fill_parse_error ("It is %s and must be %s\n", c, SDDFA_3_NAME);
    near_line ();
    return;
  }
  if ((count_queue (q) < 5) || (count_queue (q) >7))
  {
    fill_parse_error ("Incorrect number of elements in register #3: %s\n", c);
    fill_parse_error ("There are %d and must be 5, 6 or 7\n", count_queue (q));
    near_line ();
  }
  
  el = (struct t_element *) head_queue (q);
  if (strcmp (el->i3->i1, SDDFA_3_1A) != 0)
  {
    Invalid_attribute (1, 3, c, el->i3->i1, SDDFA_3_1A);
    near_line ();
  }
  if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
  {
    Invalid_type (1, el->i3->i1, 3, c, el->type, 0, el->i3->dimension, 0);
    near_line ();
  }
  
  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_3_2A) != 0)
  {
    Invalid_attribute (2, 3, c, el->i3->i1, SDDFA_3_2A);
    near_line ();
  }
  if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
  {
    Invalid_type (2, el->i3->i1, 3, c, el->type, 0, el->i3->dimension, 0);
    near_line ();
  }
  
  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_3_3A) != 0)
  {
    Invalid_attribute (3, 3, c, el->i3->i1, SDDFA_3_3A);
    near_line ();
  }
  if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
  {
    Invalid_type (3, el->i3->i1, 3, c, el->type, 0, el->i3->dimension, 0);
    near_line ();
  }
  
  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_3_4A) != 0)
  {
    Invalid_attribute (4, 3, c, el->i3->i1, SDDFA_3_4A);
    near_line ();
  }
  if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
  {
    Invalid_type (4, el->i3->i1, 3, c, el->type, 0, el->i3->dimension, 0);
    near_line ();
  }
  
  el = (struct t_element *) next_queue (q);
  if (strcmp (el->i3->i1, SDDFA_3_5A) != 0)
  {
    Invalid_attribute (5, 3, c, el->i3->i1, SDDFA_3_5A);
    near_line ();
  }
  if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
  {
    Invalid_type (5, el->i3->i1, 3, c, el->type, 0, el->i3->dimension, 0);
    near_line ();
  }
  
  el = (struct t_element *) next_queue (q);
  if (el==(struct t_element *)0) return;
  if (strcmp (el->i3->i1, SDDFA_3_6A) != 0)
  {
    Invalid_attribute (6, 3, c, el->i3->i1, SDDFA_3_6A);
    near_line ();
  }
  if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
  {
    Invalid_type (6, el->i3->i1, 3, c, el->type, 0, el->i3->dimension, 0);
    near_line ();
  }
  
  el = (struct t_element *) next_queue (q);
  if (el==(struct t_element *)0) return;
  if (strcmp (el->i3->i1, SDDFA_3_7A) != 0)
  {
    Invalid_attribute (7, 3, c, el->i3->i1, SDDFA_3_7A);
    near_line ();
  }
  if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
  {
    Invalid_type (7, el->i3->i1, 3, c, el->type, 0, el->i3->dimension, 0);
    near_line ();
  }
}

void
check_record_40(char *c, struct t_queue *q)
{
   struct t_element *el;

   if (strcmp (c, SDDFA_40_NAME) != 0)
   {
      fill_parse_error ("Incorrect name for register #4\n");
      fill_parse_error ("It is %s and must be %s\n", c, SDDFA_40_NAME);
      near_line ();
      return;
   }
   if (count_queue (q) != 3)
   {
      fill_parse_error ("Incorrect number of elements in register #40: %s\n", c);
      fill_parse_error ("There are %d and must be 3\n", count_queue (q));
      near_line ();
   }

   el = (struct t_element *) head_queue (q);
   if (strcmp (el->i3->i1, SDDFA_40_1A) != 0)
   {
      Invalid_attribute (1, 40, c, el->i3->i1, SDDFA_40_1A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (1, el->i3->i1, 40, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_40_2A) != 0)
   {
      Invalid_attribute (2, 40, c, el->i3->i1, SDDFA_40_2A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (2, el->i3->i1, 40, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_40_3A) != 0)
   {
      Invalid_attribute (3, 40, c, el->i3->i1, SDDFA_40_3A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (3, el->i3->i1, 40, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }
}

void
check_record_41(char *c, struct t_queue *q)
{
   struct t_element *el;

   if (strcmp (c, SDDFA_41_NAME) != 0)
   {
      fill_parse_error ("Incorrect name for register #41\n");
      fill_parse_error ("It is %s and must be %s\n", c, SDDFA_41_NAME);
      near_line ();
      return;
   }
   if (count_queue (q) != 3)
   {
      fill_parse_error ("Incorrect number of elements in register #41: %s\n", c);
      fill_parse_error ("There are %d and must be 3\n", count_queue (q));
      near_line ();
   }

   el = (struct t_element *) head_queue (q);
   if (strcmp (el->i3->i1, SDDFA_41_1A) != 0)
   {
      Invalid_attribute (1, 41, c, el->i3->i1, SDDFA_41_1A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (1, el->i3->i1, 41, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_41_2A) != 0)
   {
      Invalid_attribute (2, 41, c, el->i3->i1, SDDFA_41_2A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (2, el->i3->i1, 41, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_41_3A) != 0)
   {
      Invalid_attribute (3, 41, c, el->i3->i1, SDDFA_41_3A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (3, el->i3->i1, 41, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

}

void
check_record_42(char *c, struct t_queue *q)
{
   struct t_element *el;

   if (strcmp (c, SDDFA_42_NAME) != 0)
   {
      fill_parse_error ("Incorrect name for register #42\n");
      fill_parse_error ("It is %s and must be %s\n", c, SDDFA_42_NAME);
      near_line ();
      return;
   }
   if (count_queue (q) != 5)
   {
      fill_parse_error ("Incorrect number of elements in register #42: %s\n", c);
      fill_parse_error ("There are %d and must be 5\n", count_queue (q));
      near_line ();
   }

   el = (struct t_element *) head_queue (q);
   if (strcmp (el->i3->i1, SDDFA_42_1A) != 0)
   {
      Invalid_attribute (1, 42, c, el->i3->i1, SDDFA_42_1A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (1, el->i3->i1, 42, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_42_2A) != 0)
   {
      Invalid_attribute (2, 42, c, el->i3->i1, SDDFA_42_2A);
      near_line ();
   }
   if ((el->type != TYPE_CHAR) || (el->i3->dimension != 1))
   {
      Invalid_type (2, el->i3->i1, 42, c, el->type, 2, el->i3->dimension, 1);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_42_3A) != 0)
   {
      Invalid_attribute (3, 42, c, el->i3->i1, SDDFA_42_3A);
      near_line ();
   }
   if ((el->type != TYPE_CHAR) || (el->i3->dimension != 1))
   {
      Invalid_type (3, el->i3->i1, 42, c, el->type, 2, el->i3->dimension, 1);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_42_4A) != 0)
   {
      Invalid_attribute (4, 42, c, el->i3->i1, SDDFA_42_4A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (4, el->i3->i1, 42, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_42_5A) != 0)
   {
      Invalid_attribute (5, 42, c, el->i3->i1, SDDFA_42_5A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (5, el->i3->i1, 42, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

}

void
check_record_43(char *c, struct t_queue *q)
{
   struct t_element *el;

   if (strcmp (c, SDDFA_43_NAME) != 0)
   {
      fill_parse_error ("Incorrect name for register #43\n");
      fill_parse_error ("It is %s and must be %s\n", c, SDDFA_43_NAME);
      near_line ();
      return;
   }
   if (count_queue (q) != 2)
   {
      fill_parse_error ("Incorrect number of elements in register #43: %s\n", c);
      fill_parse_error ("There are %d and must be 2\n", count_queue (q));
      near_line ();
   }

   el = (struct t_element *) head_queue (q);
   if (strcmp (el->i3->i1, SDDFA_43_1A) != 0)
   {
      Invalid_attribute (1, 43, c, el->i3->i1, SDDFA_43_1A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (1, el->i3->i1, 43, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_43_2A) != 0)
   {
      Invalid_attribute (2, 43, c, el->i3->i1, SDDFA_43_2A);
      near_line ();
   }
   if ((el->type != TYPE_CHAR) || (el->i3->dimension != 1))
   {
      Invalid_type (2, el->i3->i1, 43, c, el->type, 2, el->i3->dimension, 1);
      near_line ();
   }
}



void
check_record_48(char *c, struct t_queue *q)
{
   struct t_element *el;

   if (strcmp (c, SDDFA_48_NAME) != 0)
   {
      fill_parse_error ("Incorrect name for register #48\n");
      fill_parse_error ("It is %s and must be %s\n", c, SDDFA_48_NAME);
      near_line ();
      return;
   }
   if (count_queue (q) != 3)
   {
      fill_parse_error ("Incorrect number of elements in register #48: %s\n", c);
      fill_parse_error ("There are %d and must be 3\n", count_queue (q));
      near_line ();
   }

   el = (struct t_element *) head_queue (q);
   if (strcmp (el->i3->i1, SDDFA_48_1A) != 0)
   {
      Invalid_attribute (1, 48, c, el->i3->i1, SDDFA_48_1A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (1, el->i3->i1, 48, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_48_2A) != 0)
   {
      Invalid_attribute (2, 48, c, el->i3->i1, SDDFA_48_2A);
      near_line ();
   }
   if ((el->type != TYPE_CHAR) || (el->i3->dimension != 1))
   {
      Invalid_type (2, el->i3->i1, 48, c, el->type, 2, el->i3->dimension, 1);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_48_3A) != 0)
   {
      Invalid_attribute (3, 48, c, el->i3->i1, SDDFA_48_3A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (3, el->i3->i1, 48, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }
}




void
check_record_49(char *c, struct t_queue *q)
{
   struct t_element *el;

   if (strcmp (c, SDDFA_49_NAME) != 0)
   {
      fill_parse_error ("Incorrect name for register #49\n");
      fill_parse_error ("It is %s and must be %s\n", c, SDDFA_49_NAME);
      near_line ();
      return;
   }
   if (count_queue (q) != 3)
   {
      fill_parse_error ("Incorrect number of elements in register #49: %s\n", c);
      fill_parse_error ("There are %d and must be 3\n", count_queue (q));
      near_line ();
   }

   el = (struct t_element *) head_queue (q);
   if (strcmp (el->i3->i1, SDDFA_49_1A) != 0)
   {
      Invalid_attribute (1, 49, c, el->i3->i1, SDDFA_49_1A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (1, el->i3->i1, 49, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_49_2A) != 0)
   {
      Invalid_attribute (2, 49, c, el->i3->i1, SDDFA_49_2A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (2, el->i3->i1, 49, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_49_3A) != 0)
   {
      Invalid_attribute (3, 49, c, el->i3->i1, SDDFA_49_3A);
      near_line ();
   }
   if ((el->type != TYPE_CHAR) || (el->i3->dimension != 1))
   {
      Invalid_type (3, el->i3->i1, 49, c, el->type, 2, el->i3->dimension, 1);
      near_line ();
   }
}




void
check_record_50(char *c, struct t_queue *q)
{
   struct t_element *el;

   if (strcmp (c, SDDFA_50_NAME) != 0)
   {
      fill_parse_error ("Incorrect name for register #50\n");
      fill_parse_error ("It is %s and must be %s\n", c, SDDFA_50_NAME);
      near_line ();
      return;
   }
   if (count_queue (q) != 4)
   {
      fill_parse_error ("Incorrect number of elements in register #50: %s\n", c);
      fill_parse_error ("There are %d and must be 4\n", count_queue (q));
      near_line ();
   }

   el = (struct t_element *) head_queue (q);
   if (strcmp (el->i3->i1, SDDFA_50_1A) != 0)
   {
      Invalid_attribute (1, 50, c, el->i3->i1, SDDFA_50_1A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (1, el->i3->i1, 50, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_50_2A) != 0)
   {
      Invalid_attribute (2, 50, c, el->i3->i1, SDDFA_50_2A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (2, el->i3->i1, 50, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_50_3A) != 0)
   {
      Invalid_attribute (3, 50, c, el->i3->i1, SDDFA_50_3A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (3, el->i3->i1, 50, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_50_4A) != 0)
   {
      Invalid_attribute (4, 50, c, el->i3->i1, SDDFA_50_4A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (4, el->i3->i1, 50, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

}

void
check_record_101(char *c, struct t_queue *q)
{
   struct t_element *el;

   if (strcmp (c, SDDFA_101_NAME) != 0)
   {
      fill_parse_error ("Incorrect name for register #101\n");
      fill_parse_error ("It is %s and must be %s\n", c, SDDFA_101_NAME);
      near_line ();
      return;
   }
   if (count_queue (q) != 6)
   {
      fill_parse_error ("Incorrect number of elements in register #101: %s\n", c);
      fill_parse_error ("There are %d and must be 6\n", count_queue (q));
      near_line ();
   }

   el = (struct t_element *) head_queue (q);
   if (strcmp (el->i3->i1, SDDFA_101_1A) != 0)
   {
      Invalid_attribute (1, 101, c, el->i3->i1, SDDFA_101_1A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (1, el->i3->i1, 101, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_101_2A) != 0)
   {
      Invalid_attribute (2, 101, c, el->i3->i1, SDDFA_101_2A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (2, el->i3->i1, 101, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }
   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_101_3A) != 0)
   {
      Invalid_attribute (3, 101, c, el->i3->i1, SDDFA_101_3A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (3, el->i3->i1, 101, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_101_4A) != 0)
   {
      Invalid_attribute (4, 101, c, el->i3->i1, SDDFA_101_4A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (4, el->i3->i1, 101, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_101_5A) != 0)
   {
      Invalid_attribute (5, 101, c, el->i3->i1, SDDFA_101_5A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (5, el->i3->i1, 101, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_101_6A) != 0)
   {
      Invalid_attribute (6, 101, c, el->i3->i1, SDDFA_101_6A);
      near_line ();
   }
   if ((el->type != TYPE_CHAR) || (el->i3->dimension != 1))
   {
      Invalid_type (6, el->i3->i1, 101, c, el->type, 2, el->i3->dimension, 1);
      near_line ();
   }

}

void
check_record_102(char *c, struct t_queue *q)
{
   struct t_element *el;

   if (strcmp (c, SDDFA_102_NAME) != 0)
   {
      fill_parse_error ("Incorrect name for register #102\n");
      fill_parse_error ("It is %s and must be %s\n", c, SDDFA_102_NAME);
      near_line ();
      return;
   }
   if (count_queue (q) != 5)
   {
      fill_parse_error ("Incorrect number of elements in register #102: %s\n", c);
      fill_parse_error ("There are %d and must be 5\n", count_queue (q));
      near_line ();
   }

   el = (struct t_element *) head_queue (q);
   if (strcmp (el->i3->i1, SDDFA_102_1A) != 0)
   {
      Invalid_attribute (1, 102, c, el->i3->i1, SDDFA_102_1A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (1, el->i3->i1, 102, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_102_2A) != 0)
   {
      Invalid_attribute (2, 102, c, el->i3->i1, SDDFA_102_2A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (2, el->i3->i1, 102, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }
   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_102_3A) != 0)
   {
      Invalid_attribute (3, 102, c, el->i3->i1, SDDFA_102_3A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (3, el->i3->i1, 102, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_102_4A) != 0)
   {
      Invalid_attribute (4, 102, c, el->i3->i1, SDDFA_102_4A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (4, el->i3->i1, 102, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_102_5A) != 0)
   {
      Invalid_attribute (5, 102, c, el->i3->i1, SDDFA_102_5A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (5, el->i3->i1, 102, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

}

void
check_record_103(char *c, struct t_queue *q)
{
   struct t_element *el;

   if (strcmp (c, SDDFA_103_NAME) != 0)
   {
      fill_parse_error ("Incorrect name for register #103\n");
      fill_parse_error ("It is %s and must be %s\n", c, SDDFA_103_NAME);
      near_line ();
      return;
   }
   if (count_queue (q) != 5)
   {
      fill_parse_error ("Incorrect number of elements in register #103: %s\n", c);
      fill_parse_error ("There are %d and must be 5\n", count_queue (q));
      near_line ();
   }

   el = (struct t_element *) head_queue (q);
   if (strcmp (el->i3->i1, SDDFA_103_1A) != 0)
   {
      Invalid_attribute (1, 103, c, el->i3->i1, SDDFA_103_1A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (1, el->i3->i1, 103, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_103_2A) != 0)
   {
      Invalid_attribute (2, 103, c, el->i3->i1, SDDFA_103_2A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (2, el->i3->i1, 103, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }
   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_103_3A) != 0)
   {
      Invalid_attribute (3, 103, c, el->i3->i1, SDDFA_103_3A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (3, el->i3->i1, 103, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_103_4A) != 0)
   {
      Invalid_attribute (4, 103, c, el->i3->i1, SDDFA_103_4A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (4, el->i3->i1, 103, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_103_5A) != 0)
   {
      Invalid_attribute (5, 103, c, el->i3->i1, SDDFA_103_5A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (5, el->i3->i1, 103, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

}

void
check_record_104(char *c, struct t_queue *q)
{
   struct t_element *el;

   if (strcmp (c, SDDFA_104_NAME) != 0)
   {
      fill_parse_error ("Incorrect name for register #104\n");
      fill_parse_error ("It is %s and must be %s\n", c, SDDFA_104_NAME);
      near_line ();
      return;
   }
   if (count_queue (q) != 6)
   {
      fill_parse_error ("Incorrect number of elements in register #104: %s\n", c);
      fill_parse_error ("There are %d and must be 6\n", count_queue (q));
      near_line ();
   }

   el = (struct t_element *) head_queue (q);
   if (strcmp (el->i3->i1, SDDFA_104_1A) != 0)
   {
      Invalid_attribute (1, 104, c, el->i3->i1, SDDFA_104_1A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (1, el->i3->i1, 104, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_104_2A) != 0)
   {
      Invalid_attribute (2, 104, c, el->i3->i1, SDDFA_104_2A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (2, el->i3->i1, 104, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }
   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_104_3A) != 0)
   {
      Invalid_attribute (3, 104, c, el->i3->i1, SDDFA_104_3A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (3, el->i3->i1, 104, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_104_4A) != 0)
   {
      Invalid_attribute (4, 104, c, el->i3->i1, SDDFA_104_4A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (4, el->i3->i1, 104, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_104_5A) != 0)
   {
      Invalid_attribute (5, 104, c, el->i3->i1, SDDFA_104_5A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (5, el->i3->i1, 104, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_104_6A) != 0)
   {
      Invalid_attribute (6, 104, c, el->i3->i1, SDDFA_104_6A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (6, el->i3->i1, 104, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

}

void
check_record_105(char *c, struct t_queue *q)
{
   struct t_element *el;

   if (strcmp (c, SDDFA_105_NAME) != 0)
   {
      fill_parse_error ("Incorrect name for register #105\n");
      fill_parse_error ("It is %s and must be %s\n", c, SDDFA_105_NAME);
      near_line ();
      return;
   }
   if (count_queue (q) != 3)
   {
      fill_parse_error ("Incorrect number of elements in register #105: %s\n", c);
      fill_parse_error ("There are %d and must be 3\n", count_queue (q));
      near_line ();
   }

   el = (struct t_element *) head_queue (q);
   if (strcmp (el->i3->i1, SDDFA_105_1A) != 0)
   {
      Invalid_attribute (1, 105, c, el->i3->i1, SDDFA_105_1A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (1, el->i3->i1, 105, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_105_2A) != 0)
   {
      Invalid_attribute (2, 105, c, el->i3->i1, SDDFA_105_2A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (2, el->i3->i1, 105, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }
   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_105_3A) != 0)
   {
      Invalid_attribute (3, 105, c, el->i3->i1, SDDFA_105_3A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (3, el->i3->i1, 105, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

}

void
check_record_106(char *c, struct t_queue *q)
{
   struct t_element *el;

   if (strcmp (c, SDDFA_106_NAME) != 0)
   {
      fill_parse_error ("Incorrect name for register #106\n");
      fill_parse_error ("It is %s and must be %s\n", c, SDDFA_106_NAME);
      near_line ();
      return;
   }
   if (count_queue (q) != 4)
   {
      fill_parse_error ("Incorrect number of elements in register #106: %s\n", c);
      fill_parse_error ("There are %d and must be 4\n", count_queue (q));
      near_line ();
   }

   el = (struct t_element *) head_queue (q);
   if (strcmp (el->i3->i1, SDDFA_106_1A) != 0)
   {
      Invalid_attribute (1, 106, c, el->i3->i1, SDDFA_106_1A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (1, el->i3->i1, 106, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_106_2A) != 0)
   {
      Invalid_attribute (2, 106, c, el->i3->i1, SDDFA_106_2A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (2, el->i3->i1, 106, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }
   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_106_3A) != 0)
   {
      Invalid_attribute (3, 106, c, el->i3->i1, SDDFA_106_3A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (3, el->i3->i1, 106, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_106_4A) != 0)
   {
      Invalid_attribute (4, 106, c, el->i3->i1, SDDFA_106_4A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (4, el->i3->i1, 106, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

}

void
check_record_107(char *c, struct t_queue *q)
{
   struct t_element *el;

   if (strcmp (c, SDDFA_107_NAME) != 0)
   {
      fill_parse_error ("Incorrect name for register #107\n");
      fill_parse_error ("It is %s and must be %s\n", c, SDDFA_107_NAME);
      near_line ();
      return;
   }
   if (count_queue (q) != 3)
   {
      fill_parse_error ("Incorrect number of elements in register #107: %s\n", c);
      fill_parse_error ("There are %d and must be 3\n", count_queue (q));
      near_line ();
   }

   el = (struct t_element *) head_queue (q);
   if (strcmp (el->i3->i1, SDDFA_107_1A) != 0)
   {
      Invalid_attribute (1, 107, c, el->i3->i1, SDDFA_107_1A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (1, el->i3->i1, 107, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_107_2A) != 0)
   {
      Invalid_attribute (2, 107, c, el->i3->i1, SDDFA_107_2A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (2, el->i3->i1, 107, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }
   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_107_3A) != 0)
   {
      Invalid_attribute (3, 107, c, el->i3->i1, SDDFA_107_3A);
      near_line ();
   }
   if ((el->type != TYPE_CHAR) || (el->i3->dimension != 1))
   {
      Invalid_type (3, el->i3->i1, 107, c, el->type, 2, el->i3->dimension, 1);
      near_line ();
   }

}

void
check_record_100(char *c, struct t_queue *q)
{
   struct t_element *el;

   if (strcmp (c, SDDFA_100_NAME) != 0)
   {
      fill_parse_error ("Incorrect name for register #100\n");
      fill_parse_error ("It is %s and must be %s\n", c, SDDFA_100_NAME);
      near_line ();
      return;
   }
   if (count_queue (q) != 3)
   {
      fill_parse_error ("Incorrect number of elements in register #100: %s\n", c);
      fill_parse_error ("There are %d and must be 3\n", count_queue (q));
      near_line ();
   }

   el = (struct t_element *) head_queue (q);
   if (strcmp (el->i3->i1, SDDFA_100_1A) != 0)
   {
      Invalid_attribute (1, 100, c, el->i3->i1, SDDFA_100_1A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (1, el->i3->i1, 100, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_100_2A) != 0)
   {
      Invalid_attribute (2, 100, c, el->i3->i1, SDDFA_100_2A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (2, el->i3->i1, 100, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }
   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_100_3A) != 0)
   {
      Invalid_attribute (3, 100, c, el->i3->i1, SDDFA_100_3A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 1))
   {
      Invalid_type (3, el->i3->i1, 100, c, el->type, 0, el->i3->dimension, 1);
      near_line ();
   }

}

void
check_record_200 (char *c, struct t_queue *q)
{
   struct t_element *el;

   if (strcmp (c, SDDFA_200_NAME) != 0)
   {
      fill_parse_error ("Incorrect name for register #200\n");
      fill_parse_error ("It is %s and must be %s\n", c, SDDFA_200_NAME);
      near_line ();
      return;
   }
   if (count_queue (q) != 2)
   {
      fill_parse_error ("Incorrect number of elements in register #200: %s\n", c);
      fill_parse_error ("There are %d and must be 3\n", count_queue (q));
      near_line ();
   }

   el = (struct t_element *) head_queue (q);
   if (strcmp (el->i3->i1, SDDFA_200_1A) != 0)
   {
      Invalid_attribute (1, 200, c, el->i3->i1, SDDFA_200_1A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (1, el->i3->i1, 200, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_200_2A) != 0)
   {
      Invalid_attribute (2, 200, c, el->i3->i1, SDDFA_200_2A);
      near_line ();
   }
   if ((el->type != TYPE_CHAR) || (el->i3->dimension != 1))
   {
      Invalid_type (2, el->i3->i1, 200, c, el->type, 2, el->i3->dimension, 1);
      near_line ();
   }
}

void
check_record_201 (char *c, struct t_queue *q)
{
   struct t_element *el;

   if (strcmp (c, SDDFA_201_NAME) != 0)
   {
      fill_parse_error ("Incorrect name for register #201\n");
      fill_parse_error ("It is %s and must be %s\n", c, SDDFA_201_NAME);
      near_line ();
      return;
   }
   if (count_queue (q) != 8)
   {
      fill_parse_error ("Incorrect number of elements in register #201: %s\n", c);
      fill_parse_error ("There are %d and must be 8\n", count_queue (q));
      near_line ();
   }

   el = (struct t_element *) head_queue (q);
   if (strcmp (el->i3->i1, SDDFA_201_1A) != 0)
   {
      Invalid_attribute (1, 201, c, el->i3->i1, SDDFA_201_1A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (1, el->i3->i1, 201, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }
   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_201_2A) != 0)
   {
      Invalid_attribute (2, 201, c, el->i3->i1, SDDFA_201_2A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (2, el->i3->i1, 202, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }
   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_201_3A) != 0)
   {
      Invalid_attribute (3, 201, c, el->i3->i1, SDDFA_201_3A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (3, el->i3->i1, 201, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }
   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_201_4A) != 0)
   {
      Invalid_attribute (4, 201, c, el->i3->i1, SDDFA_201_4A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (4, el->i3->i1, 201, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }
   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_201_5A) != 0)
   {
      Invalid_attribute (5, 201, c, el->i3->i1, SDDFA_201_5A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (5, el->i3->i1, 201, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }
   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_201_6A) != 0)
   {
      Invalid_attribute (6, 201, c, el->i3->i1, SDDFA_201_6A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (6, el->i3->i1, 201, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }
   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_201_7A) != 0)
   {
      Invalid_attribute (7, 201, c, el->i3->i1, SDDFA_201_7A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (7, el->i3->i1, 201, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }
   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_201_8A) != 0)
   {
      Invalid_attribute (8, 201, c, el->i3->i1, SDDFA_201_8A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (8, el->i3->i1, 201, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }
}

void
check_record_300 (char *c, struct t_queue *q)
{
   struct t_element *el;

   if (strcmp (c, SDDFA_300_NAME) != 0)
   {
      fill_parse_error ("Incorrect name for register #300\n");
      fill_parse_error ("It is %s and must be %s\n", c, SDDFA_300_NAME);
      near_line ();
      return;
   }
   if (count_queue (q) != 2)
   {
      fill_parse_error ("Incorrect number of elements in register #300: %s\n", c);
      fill_parse_error ("There are %d and must be 2\n", count_queue (q));
      near_line ();
   }

   el = (struct t_element *) head_queue (q);
   if (strcmp (el->i3->i1, SDDFA_300_1A) != 0)
   {
      Invalid_attribute (1, 300, c, el->i3->i1, SDDFA_300_1A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (1, el->i3->i1, 300, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }
   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_300_2A) != 0)
   {
      Invalid_attribute (2, 300, c, el->i3->i1, SDDFA_300_2A);
      near_line ();
   }
   if ((el->type != TYPE_CHAR) || (el->i3->dimension != 1))
   {
      Invalid_type (2, el->i3->i1, 300, c, el->type, TYPE_CHAR, el->i3->dimension, 0);
      near_line ();
   }
}

void
check_record_301 (char *c, struct t_queue *q)
{
   struct t_element *el;

   if (strcmp (c, SDDFA_301_NAME) != 0)
   {
      fill_parse_error ("Incorrect name for register #301\n");
      fill_parse_error ("It is %s and must be %s\n", c, SDDFA_301_NAME);
      near_line ();
      return;
   }
   if (count_queue (q) != 5)
   {
      fill_parse_error ("Incorrect number of elements in register #301: %s\n", c);
      fill_parse_error ("There are %d and must be 5\n", count_queue (q));
      near_line ();
   }

   el = (struct t_element *) head_queue (q);
   if (strcmp (el->i3->i1, SDDFA_301_1A) != 0)
   {
      Invalid_attribute (1, 301, c, el->i3->i1, SDDFA_301_1A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (1, el->i3->i1, 301, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }
   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_301_2A) != 0)
   {
      Invalid_attribute (2, 300, c, el->i3->i1, SDDFA_301_2A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (2, el->i3->i1, 301, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }
   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_301_3A) != 0)
   {
      Invalid_attribute (3, 301, c, el->i3->i1, SDDFA_301_3A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (3, el->i3->i1, 301, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }
   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_301_4A) != 0)
   {
      Invalid_attribute (4, 301, c, el->i3->i1, SDDFA_301_4A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (4, el->i3->i1, 301, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }
   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_301_5A) != 0)
   {
      Invalid_attribute (5, 301, c, el->i3->i1, SDDFA_301_5A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (5, el->i3->i1, 301, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }
}

void
check_record_302 (char *c, struct t_queue *q)
{
   struct t_element *el;

   if (strcmp (c, SDDFA_302_NAME) != 0)
   {
      fill_parse_error ("Incorrect name for register #302\n");
      fill_parse_error ("It is %s and must be %s\n", c, SDDFA_302_NAME);
      near_line ();
      return;
   }
   if (count_queue (q) != 4)
   {
      fill_parse_error ("Incorrect number of elements in register #302: %s\n", c);
      fill_parse_error ("There are %d and must be 4\n", count_queue (q));
      near_line ();
   }

   el = (struct t_element *) head_queue (q);
   if (strcmp (el->i3->i1, SDDFA_302_1A) != 0)
   {
      Invalid_attribute (1, 302, c, el->i3->i1, SDDFA_302_1A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (1, el->i3->i1, 302, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_302_2A) != 0)
   {
      Invalid_attribute (2, 302, c, el->i3->i1, SDDFA_302_2A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (2, el->i3->i1, 302, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_302_3A) != 0)
   {
      Invalid_attribute (3, 302, c, el->i3->i1, SDDFA_302_3A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (3, el->i3->i1, 302, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_302_4A) != 0)
   {
      Invalid_attribute (4, 302, c, el->i3->i1, SDDFA_302_4A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (4, el->i3->i1, 302, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }
}

void
check_record_303 (char *c, struct t_queue *q)
{
   struct t_element *el;

   if (strcmp (c, SDDFA_303_NAME) != 0)
   {
      fill_parse_error ("Incorrect name for register #303\n");
      fill_parse_error ("It is %s and must be %s\n", c, SDDFA_303_NAME);
      near_line ();
      return;
   }
   if (count_queue (q) != 5)
   {
      fill_parse_error ("Incorrect number of elements in register #303: %s\n", c);
      fill_parse_error ("There are %d and must be 5\n", count_queue (q));
      near_line ();
   }

   el = (struct t_element *) head_queue (q);
   if (strcmp (el->i3->i1, SDDFA_303_1A) != 0)
   {
      Invalid_attribute (1, 303, c, el->i3->i1, SDDFA_303_1A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (1, el->i3->i1, 303, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_303_2A) != 0)
   {
      Invalid_attribute (2, 303, c, el->i3->i1, SDDFA_303_2A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (2, el->i3->i1, 303, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_303_3A) != 0)
   {
      Invalid_attribute (3, 303, c, el->i3->i1, SDDFA_303_3A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (3, el->i3->i1, 303, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_303_4A) != 0)
   {
      Invalid_attribute (4, 303, c, el->i3->i1, SDDFA_303_4A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (4, el->i3->i1, 303, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_303_5A) != 0)
   {      
      Invalid_attribute (5, 303, c, el->i3->i1, SDDFA_303_5A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (5, el->i3->i1, 303, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }
}

void
check_record_304 (char *c, struct t_queue *q)
{
   struct t_element *el;

   if (strcmp (c, SDDFA_304_NAME) != 0)
   {
      fill_parse_error ("Incorrect name for register #304\n");
      fill_parse_error ("It is %s and must be %s\n", c, SDDFA_304_NAME);
      near_line ();
      return;
   }
   if (count_queue (q) != 5)
   {
      fill_parse_error ("Incorrect number of elements in register #304: %s\n", c);
      fill_parse_error ("There are %d and must be 5\n", count_queue (q));
      near_line ();
   }

   el = (struct t_element *) head_queue (q);
   if (strcmp (el->i3->i1, SDDFA_304_1A) != 0)
   {
      Invalid_attribute (1, 304, c, el->i3->i1, SDDFA_304_1A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (1, el->i3->i1, 304, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_304_2A) != 0)
   {
      Invalid_attribute (2, 304, c, el->i3->i1, SDDFA_304_2A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (2, el->i3->i1, 304, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_304_3A) != 0)
   {
      Invalid_attribute (3, 304, c, el->i3->i1, SDDFA_304_3A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (3, el->i3->i1, 304, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_304_4A) != 0)
   {
      Invalid_attribute (4, 304, c, el->i3->i1, SDDFA_304_4A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (4, el->i3->i1, 304, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_304_5A) != 0)
   {
      Invalid_attribute (5, 304, c, el->i3->i1, SDDFA_304_5A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (5, el->i3->i1, 304, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }
}

void
check_record_305 (char *c, struct t_queue *q)
{
   struct t_element *el;

   if (strcmp (c, SDDFA_305_NAME) != 0)
   {
      fill_parse_error ("Incorrect name for register #305\n");
      fill_parse_error ("It is %s and must be %s\n", c, SDDFA_305_NAME);
      near_line ();
      return;
   }
   if (count_queue (q) != 4)
   {
      fill_parse_error ("Incorrect number of elements in register #305: %s\n", c);
      fill_parse_error ("There are %d and must be 4\n", count_queue (q));
      near_line ();
   }

   el = (struct t_element *) head_queue (q);
   if (strcmp (el->i3->i1, SDDFA_305_1A) != 0)
   {
      Invalid_attribute (1, 305, c, el->i3->i1, SDDFA_305_1A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (1, el->i3->i1, 305, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_305_2A) != 0)
   {
      Invalid_attribute (2, 305, c, el->i3->i1, SDDFA_305_2A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (2, el->i3->i1, 305, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_305_3A) != 0)
   {
      Invalid_attribute (3, 305, c, el->i3->i1, SDDFA_305_3A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (3, el->i3->i1, 305, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_305_4A) != 0)
   {
      Invalid_attribute (4, 305, c, el->i3->i1, SDDFA_305_4A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (4, el->i3->i1, 305, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }
}

void
check_record_306 (char *c, struct t_queue *q)
{
   struct t_element *el;

   if (strcmp (c, SDDFA_306_NAME) != 0)
   {
      fill_parse_error ("Incorrect name for register #306\n");
      fill_parse_error ("It is %s and must be %s\n", c, SDDFA_306_NAME);
      near_line ();
      return;
   }
   if (count_queue (q) != 5)
   {
      fill_parse_error ("Incorrect number of elements in register #306: %s\n", c);
      fill_parse_error ("There are %d and must be 5\n", count_queue (q));
      near_line ();
   }

   el = (struct t_element *) head_queue (q);
   if (strcmp (el->i3->i1, SDDFA_306_1A) != 0)
   {
      Invalid_attribute (1, 306, c, el->i3->i1, SDDFA_306_1A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (1, el->i3->i1, 306, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_306_2A) != 0)
   {
      Invalid_attribute (2, 306, c, el->i3->i1, SDDFA_306_2A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (2, el->i3->i1, 306, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_306_3A) != 0)
   {
      Invalid_attribute (3, 306, c, el->i3->i1, SDDFA_306_3A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (3, el->i3->i1, 306, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_306_4A) != 0)
   {
      Invalid_attribute (4, 306, c, el->i3->i1, SDDFA_306_4A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (4, el->i3->i1, 306, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_306_5A) != 0)
   {
      Invalid_attribute (5, 306, c, el->i3->i1, SDDFA_306_5A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (5, el->i3->i1, 306, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }
}

void
check_record_307 (char *c, struct t_queue *q)
{
   struct t_element *el;

   if (strcmp (c, SDDFA_307_NAME) != 0)
   {
      fill_parse_error ("Incorrect name for register #307\n");
      fill_parse_error ("It is %s and must be %s\n", c, SDDFA_307_NAME);
      near_line ();
      return;
   }
   if (count_queue (q) != 4)
   {
      fill_parse_error ("Incorrect number of elements in register #307: %s\n", c);
      fill_parse_error ("There are %d and must be 4\n", count_queue (q));
      near_line ();
   }

   el = (struct t_element *) head_queue (q);
   if (strcmp (el->i3->i1, SDDFA_307_1A) != 0)
   {
      Invalid_attribute (1, 307, c, el->i3->i1, SDDFA_307_1A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (1, el->i3->i1, 307, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_307_2A) != 0)
   {
      Invalid_attribute (2, 307, c, el->i3->i1, SDDFA_307_2A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (2, el->i3->i1, 307, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_307_3A) != 0)
   {
      Invalid_attribute (3, 307, c, el->i3->i1, SDDFA_307_3A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (3, el->i3->i1, 307, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_307_4A) != 0)
   {
      Invalid_attribute (4, 307, c, el->i3->i1, SDDFA_307_4A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (4, el->i3->i1, 307, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }
}

void
check_record_400 (char *c, struct t_queue *q)
{
   struct t_element *el;

   if (strcmp (c, SDDFA_400_NAME) != 0)
   {
      fill_parse_error ("Incorrect name for register #400\n");
      fill_parse_error ("It is %s and must be %s\n", c, SDDFA_400_NAME);
      near_line ();
      return;
   }
   if (count_queue (q) != 3)
   {
      fill_parse_error ("Incorrect number of elements in register #400: %s\n", c);
      fill_parse_error ("There are %d and must be 3\n", count_queue (q));
      near_line ();
   }

   el = (struct t_element *) head_queue (q);
   if (strcmp (el->i3->i1, SDDFA_400_1A) != 0)
   {
      Invalid_attribute (1, 400, c, el->i3->i1, SDDFA_400_1A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (1, el->i3->i1, 400, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_400_2A) != 0)
   {
      Invalid_attribute (2, 400, c, el->i3->i1, SDDFA_400_2A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (2, el->i3->i1, 400, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }
   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_400_3A) != 0)
   {
      Invalid_attribute (3, 400, c, el->i3->i1, SDDFA_400_3A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 1))
   {
      Invalid_type (3, el->i3->i1, 400, c, el->type, 0, el->i3->dimension, 1);
      near_line ();
   }
} 

void
check_record_401 (char *c, struct t_queue *q)
{
   struct t_element *el;

   if (strcmp (c, SDDFA_401_NAME) != 0)
   {
      fill_parse_error ("Incorrect name for register #401\n");
      fill_parse_error ("It is %s and must be %s\n", c, SDDFA_401_NAME);
      near_line ();
      return;
   }
   if (count_queue (q) != 6)
   {
      fill_parse_error ("Incorrect number of elements in register #401: %s\n", c);
      fill_parse_error ("There are %d and must be 6\n", count_queue (q));
      near_line ();
   }

   el = (struct t_element *) head_queue (q);
   if (strcmp (el->i3->i1, SDDFA_401_1A) != 0)
   {
      Invalid_attribute (1, 401, c, el->i3->i1, SDDFA_401_1A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (1, el->i3->i1, 401, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_401_2A) != 0)
   {
      Invalid_attribute (2, 401, c, el->i3->i1, SDDFA_401_2A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (2, el->i3->i1, 401, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_401_3A) != 0)
   {
      Invalid_attribute (3, 401, c, el->i3->i1, SDDFA_401_3A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (3, el->i3->i1, 401, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_401_4A) != 0)
   {
      Invalid_attribute (4, 401, c, el->i3->i1, SDDFA_401_4A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (4, el->i3->i1, 401, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_401_5A) != 0)
   {      
      Invalid_attribute (5, 401, c, el->i3->i1, SDDFA_401_5A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (5, el->i3->i1, 401, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_401_6A) != 0)
   {      
      Invalid_attribute (6, 401, c, el->i3->i1, SDDFA_401_6A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (6, el->i3->i1, 401, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }
}

void
check_record_402 (char *c, struct t_queue *q)
{
   struct t_element *el;

   if (strcmp (c, SDDFA_402_NAME) != 0)
   {
      fill_parse_error ("Incorrect name for register #402\n");
      fill_parse_error ("It is %s and must be %s\n", c, SDDFA_402_NAME);
      near_line ();
      return;
   }
   if (count_queue (q) != 3)
   {
      fill_parse_error ("Incorrect number of elements in register #402: %s\n", c);
      fill_parse_error ("There are %d and must be 3\n", count_queue (q));
      near_line ();
   }

   el = (struct t_element *) head_queue (q);
   if (strcmp (el->i3->i1, SDDFA_402_1A) != 0)
   {
      Invalid_attribute (1, 402, c, el->i3->i1, SDDFA_402_1A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (1, el->i3->i1, 402, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_402_2A) != 0)
   {
      Invalid_attribute (2, 402, c, el->i3->i1, SDDFA_402_2A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (2, el->i3->i1, 402, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_402_3A) != 0)
   {
      Invalid_attribute (3, 402, c, el->i3->i1, SDDFA_402_3A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (3, el->i3->i1, 402, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }
}

void
check_record_403 (char *c, struct t_queue *q)
{
   struct t_element *el;

   if (strcmp (c, SDDFA_403_NAME) != 0)
   {
      fill_parse_error ("Incorrect name for register #403\n");
      fill_parse_error ("It is %s and must be %s\n", c, SDDFA_403_NAME);
      near_line ();
      return;
   }
   if (count_queue (q) != 5)
   {
      fill_parse_error ("Incorrect number of elements in register #403: %s\n", c);
      fill_parse_error ("There are %d and must be 5\n", count_queue (q));
      near_line ();
   }

   el = (struct t_element *) head_queue (q);
   if (strcmp (el->i3->i1, SDDFA_403_1A) != 0)
   {
      Invalid_attribute (1, 403, c, el->i3->i1, SDDFA_403_1A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (1, el->i3->i1, 403, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_403_2A) != 0)
   {
      Invalid_attribute (2, 403, c, el->i3->i1, SDDFA_403_2A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (2, el->i3->i1, 403, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_403_3A) != 0)
   {
      Invalid_attribute (3, 403, c, el->i3->i1, SDDFA_403_3A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (3, el->i3->i1, 403, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_403_4A) != 0)
   {
      Invalid_attribute (4, 403, c, el->i3->i1, SDDFA_403_4A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (4, el->i3->i1, 403, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_403_5A) != 0)
   {      
      Invalid_attribute (5, 403, c, el->i3->i1, SDDFA_403_5A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (5, el->i3->i1, 403, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }
}

void
check_record_404 (char *c, struct t_queue *q)
{
   struct t_element *el;

   if (strcmp (c, SDDFA_404_NAME) != 0)
   {
      fill_parse_error ("Incorrect name for register #404\n");
      fill_parse_error ("It is %s and must be %s\n", c, SDDFA_404_NAME);
      near_line ();
      return;
   }
   if (count_queue (q) != 6)
   {
      fill_parse_error ("Incorrect number of elements in register #404: %s\n", c);
      fill_parse_error ("There are %d and must be 6\n", count_queue (q));
      near_line ();
   }

   el = (struct t_element *) head_queue (q);
   if (strcmp (el->i3->i1, SDDFA_404_1A) != 0)
   {
      Invalid_attribute (1, 404, c, el->i3->i1, SDDFA_404_1A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (1, el->i3->i1, 404, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_404_2A) != 0)
   {
      Invalid_attribute (2, 404, c, el->i3->i1, SDDFA_404_2A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (2, el->i3->i1, 404, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_404_3A) != 0)
   {
      Invalid_attribute (3, 404, c, el->i3->i1, SDDFA_404_3A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (3, el->i3->i1, 404, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_404_4A) != 0)
   {
      Invalid_attribute (4, 404, c, el->i3->i1, SDDFA_404_4A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (4, el->i3->i1, 404, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_404_5A) != 0)
   {      
      Invalid_attribute (5, 404, c, el->i3->i1, SDDFA_404_5A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (5, el->i3->i1, 404, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_404_6A) != 0)
   {      
      Invalid_attribute (6, 404, c, el->i3->i1, SDDFA_404_6A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 1))
   {
      Invalid_type (6, el->i3->i1, 404, c, el->type, 0, el->i3->dimension, 1);
      near_line ();
   }
}




void
check_record_0c(char *c, struct t_queue *q)
{
  struct t_element *el;
   
  if (strcmp (c, SDDFA_0C_NAME) != 0)
  {
     fill_parse_error ("Incorrect name for register #0\n");
     fill_parse_error ("It is %s and must be %s\n", c, SDDFA_0C_NAME);
     near_line ();
     return;
  }
  if (count_queue (q) != 7)
  {
     fill_parse_error ("Incorrect number of elements in register #0: %s\n", c);;
     fill_parse_error ("There are %d and must be 7\n", count_queue (q));
     near_line ();
     return;
  }
  
   el = (struct t_element *) head_queue (q);
   if (strcmp (el->i3->i1, SDDFA_0C_1A_D) != 0)
   {
      Invalid_attribute (1, 0, c, el->i3->i1, SDDFA_0C_1A_D);
      near_line ();
   }
   if ((el->type != TYPE_CHAR) || (el->i3->dimension != 1))
   {
      Invalid_type (1, el->i3->i1, 0, c, el->type, TYPE_CHAR, el->i3->dimension, 1);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_0C_2A_D) != 0)
   {
      Invalid_attribute (2, 0, c, el->i3->i1, SDDFA_0C_2A_D);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (2, el->i3->i1, 0, c, el->type, TYPE_INTEGER, el->i3->dimension, 0);
      near_line ();
   }
 
   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_0C_3A_D) != 0)
   {
      Invalid_attribute (3, 0, c, el->i3->i1, SDDFA_0C_3A_D);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (3, el->i3->i1, 0, c, el->type, TYPE_INTEGER, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_0C_4A_D) != 0)
   {
      Invalid_attribute (4, 0, c, el->i3->i1, SDDFA_0C_4A_D);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (4, el->i3->i1, 0, c, el->type, TYPE_INTEGER, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_0C_5A_D) != 0)
   {
      Invalid_attribute (5, 0, c, el->i3->i1, SDDFA_0C_5A_D);
      near_line ();
   }
   if ((el->type != TYPE_DOUBLE) || (el->i3->dimension != 0))
   {
      Invalid_type (5, el->i3->i1, 0, c, el->type, TYPE_DOUBLE, el->i3->dimension, 0);
      near_line ();
   }
   
   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_0C_6A_D) != 0)
   {
      Invalid_attribute (6, 0, c, el->i3->i1, SDDFA_0C_6A_D);
      near_line ();
   }
   if ((el->type != TYPE_DOUBLE) || (el->i3->dimension != 0))
   {
      Invalid_type (6, el->i3->i1, 0, c, el->type, TYPE_DOUBLE, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_0C_7A_D) != 0)
   {
      Invalid_attribute (7, 0, c, el->i3->i1, SDDFA_0C_7A_D);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (7, el->i3->i1, 0, c, el->type, TYPE_INTEGER, el->i3->dimension, 0);
      near_line ();
   }
}





void
check_record_1c_old_format(char *c, struct t_queue *q)
{
   struct t_element *el;
   
  /* Es comproven els camps segons el format antic */
   el = (struct t_element *) head_queue (q);
   if (strcmp (el->i3->i1, SDDFA_1C_1A) != 0)
   {
      Invalid_attribute (1, 1, c, el->i3->i1, SDDFA_1C_1A);
      near_line ();
   }
   if ((el->type != TYPE_CHAR) || (el->i3->dimension != 1))
   {
      Invalid_type (1, el->i3->i1, 1, c, el->type, 2, el->i3->dimension, 1);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_1C_2A) != 0)
   {
      Invalid_attribute (2, 1, c, el->i3->i1, SDDFA_1C_2A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (2, el->i3->i1, 1, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_1C_3A) != 0)
   {
      Invalid_attribute (3, 1, c, el->i3->i1, SDDFA_1C_3A);
      near_line ();
   }
   if ((el->type != TYPE_DOUBLE) || (el->i3->dimension != 0))
   {
      Invalid_type (3, el->i3->i1, 1, c, el->type, 1, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_1C_4A) != 0)
   {
      Invalid_attribute (4, 1, c, el->i3->i1, SDDFA_1C_4A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (4, el->i3->i1, 1, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_1C_5A) != 0)
   {
      Invalid_attribute (5, 1, c, el->i3->i1, SDDFA_1C_5A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (5, el->i3->i1, 1, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }
}



void
check_record_1c_DAMIEN_format(char *c, struct t_queue *q)
{
   struct t_element *el;
   
  /* Es comproven els camps segons el format nou de DAMIEN */
   el = (struct t_element *) head_queue (q);
   if (strcmp (el->i3->i1, SDDFA_1C_1A_D) != 0)
   {
      Invalid_attribute (1, 1, c, el->i3->i1, SDDFA_1C_1A_D);
      near_line ();
   }
   if ((el->type != TYPE_CHAR) || (el->i3->dimension != 1))
   {
      Invalid_type (1, el->i3->i1, 1, c, el->type, TYPE_CHAR, el->i3->dimension, 1);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_1C_2A_D) != 0)
   {
      Invalid_attribute (2, 1, c, el->i3->i1, SDDFA_1C_2A_D);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (2, el->i3->i1, 1, c, el->type, TYPE_INTEGER, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_1C_3A_D) != 0)
   {
      Invalid_attribute (3, 1, c, el->i3->i1, SDDFA_1C_3A_D);
      near_line ();
   }
   if ((el->type != TYPE_CHAR) || (el->i3->dimension != 1))
   {
      Invalid_type (3, el->i3->i1, 1, c, el->type, TYPE_CHAR, el->i3->dimension, 1);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_1C_4A_D) != 0)
   {
      Invalid_attribute (4, 1, c, el->i3->i1, SDDFA_1C_4A_D);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (4, el->i3->i1, 1, c, el->type, TYPE_INTEGER, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_1C_5A_D) != 0)
   {
      Invalid_attribute (5, 1, c, el->i3->i1, SDDFA_1C_5A_D);
      near_line ();
   }
   if ((el->type != TYPE_DOUBLE) || (el->i3->dimension != 0))
   {
      Invalid_type (5, el->i3->i1, 1, c, el->type, TYPE_DOUBLE, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_1C_6A_D) != 0)
   {
      Invalid_attribute (6, 1, c, el->i3->i1, SDDFA_1C_6A_D);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (6, el->i3->i1, 1, c, el->type, TYPE_INTEGER, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_1C_7A_D) != 0)
   {
      Invalid_attribute (7, 1, c, el->i3->i1, SDDFA_1C_7A_D);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (7, el->i3->i1, 1, c, el->type, TYPE_INTEGER, el->i3->dimension, 0);
      near_line ();
   }

}


void
check_record_1c(char *c, struct t_queue *q)
{
   if (strcmp (c, SDDFA_1C_NAME) != 0)
   {
      fill_parse_error ("Incorrect name for register #1\n");
      fill_parse_error ("It is %s and must be %s\n", c, SDDFA_1C_NAME);
      near_line ();
      return;
   }
   if (count_queue (q) == 5)
   {
     check_record_1c_old_format(c,q);
   }
   else if (count_queue (q) == 7)
   {
     check_record_1c_DAMIEN_format(c,q);
   }
   else
   {
      fill_parse_error ("Incorrect number of elements in register #1: %s\n", c);
      fill_parse_error ("There are %d and must be 5 or 7\n", count_queue (q));
      near_line ();
   }
}




void
check_record_2c_old_format(char *c, struct t_queue *q)
{
   struct t_element *el;

   el = (struct t_element *) head_queue (q);
   if (strcmp (el->i3->i1, SDDFA_2C_1A) != 0)
   {
      Invalid_attribute (1, 2, c, el->i3->i1, SDDFA_2C_1A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (1, el->i3->i1, 2, c, el->type, TYPE_INTEGER, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_2C_2A) != 0)
   {
      Invalid_attribute (2, 2, c, el->i3->i1, SDDFA_2C_2A);
      near_line ();
   }
   if ((el->type != TYPE_CHAR) || (el->i3->dimension != 1))
   {
      Invalid_type (2, el->i3->i1, 2, c, el->type, TYPE_CHAR, el->i3->dimension, 1);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_2C_3A) != 0)
   {
      Invalid_attribute (3, 2, c, el->i3->i1, SDDFA_2C_3A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (3, el->i3->i1, 2, c, el->type, TYPE_INTEGER, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_2C_4A) != 0)
   {
      Invalid_attribute (4, 2, c, el->i3->i1, SDDFA_2C_4A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (4, el->i3->i1, 2, c, el->type, TYPE_INTEGER, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_2C_5A) != 0)
   {
      Invalid_attribute (5, 2, c, el->i3->i1, SDDFA_2C_5A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (5, el->i3->i1, 2, c, el->type, TYPE_INTEGER, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_2C_6A) != 0)
   {
      Invalid_attribute (6, 2, c, el->i3->i1, SDDFA_2C_6A);
      near_line ();
   }
   if ((el->type != TYPE_DOUBLE) || (el->i3->dimension != 0))
   {
      Invalid_type (6, el->i3->i1, 2, c, el->type, TYPE_DOUBLE, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_2C_7A) != 0)
   {
      Invalid_attribute (7, 2, c, el->i3->i1, SDDFA_2C_7A);
      near_line ();
   }
   if ((el->type != TYPE_DOUBLE) || (el->i3->dimension != 0))
   {
      Invalid_type (7, el->i3->i1, 2, c, el->type, TYPE_DOUBLE, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_2C_8A) != 0)
   {
      Invalid_attribute (8, 2, c, el->i3->i1, SDDFA_2C_8A);
      near_line ();
   }
   if ((el->type != TYPE_DOUBLE) || (el->i3->dimension != 0))
   {
      Invalid_type (8, el->i3->i1, 2, c, el->type, TYPE_DOUBLE, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_2C_9A) != 0)
   {
      Invalid_attribute (9, 2, c, el->i3->i1, SDDFA_2C_9A);
      near_line ();
   }
   if ((el->type != TYPE_DOUBLE) || (el->i3->dimension != 0))
   {
      Invalid_type (9, el->i3->i1, 2, c, el->type, TYPE_DOUBLE, el->i3->dimension, 0);
      near_line ();
   }

   if (count_queue (q) == 13)
   {
     el = (struct t_element *) next_queue (q);
     if (strcmp (el->i3->i1, SDDFA_2C_10A) != 0)
     {
        Invalid_attribute (10, 2, c, el->i3->i1, SDDFA_2C_10A);
        near_line ();
     }
     if ((el->type != TYPE_DOUBLE) || (el->i3->dimension != 0))
     {
        Invalid_type (10, el->i3->i1, 2, c, el->type, TYPE_DOUBLE, el->i3->dimension, 0);
        near_line ();
     }
  
     el = (struct t_element *) next_queue (q);
     if (strcmp (el->i3->i1, SDDFA_2C_11A) != 0)
     {
        Invalid_attribute (11, 2, c, el->i3->i1, SDDFA_2C_11A);
        near_line ();
     }
     if ((el->type != TYPE_DOUBLE) || (el->i3->dimension != 0))
     {
        Invalid_type (11, el->i3->i1, 2, c, el->type, TYPE_DOUBLE, el->i3->dimension, 0);
        near_line ();
     }
  
     el = (struct t_element *) next_queue (q);
     if (strcmp (el->i3->i1, SDDFA_2C_12A) != 0)
     {
        Invalid_attribute (12, 2, c, el->i3->i1, SDDFA_2C_12A);
        near_line ();
     }
     if ((el->type != TYPE_DOUBLE) || (el->i3->dimension != 0))
     {
        Invalid_type (12, el->i3->i1, 2, c, el->type, TYPE_DOUBLE, el->i3->dimension, 0);
        near_line ();
     }
  
     el = (struct t_element *) next_queue (q);
     if (strcmp (el->i3->i1, SDDFA_2C_13A) != 0)
     {
        Invalid_attribute (13, 2, c, el->i3->i1, SDDFA_2C_13A);
        near_line ();
     }
     if ((el->type != TYPE_DOUBLE) || (el->i3->dimension != 0))
     {
        Invalid_type (13, el->i3->i1, 2, c, el->type, TYPE_DOUBLE, el->i3->dimension, 0);
        near_line ();
     }
   }
}



void
check_record_2c_DAMIEN_format(char *c, struct t_queue *q)
{
   struct t_element *el;

   el = (struct t_element *) head_queue (q);
   if (strcmp (el->i3->i1, SDDFA_2C_1A_D) != 0)
   {
      Invalid_attribute (1, 2, c, el->i3->i1, SDDFA_2C_1A_D);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (1, el->i3->i1, 2, c, el->type, TYPE_INTEGER, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_2C_2A_D) != 0)
   {
      Invalid_attribute (2, 2, c, el->i3->i1, SDDFA_2C_2A_D);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (2, el->i3->i1, 2, c, el->type, TYPE_INTEGER, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_2C_3A_D) != 0)
   {
      Invalid_attribute (3, 2, c, el->i3->i1, SDDFA_2C_3A_D);
      near_line ();
   }
   if ((el->type != TYPE_CHAR) || (el->i3->dimension != 1))
   {
      Invalid_type (3, el->i3->i1, 2, c, el->type, TYPE_CHAR, el->i3->dimension, 1);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_2C_4A_D) != 0)
   {
      Invalid_attribute (4, 2, c, el->i3->i1, SDDFA_2C_4A_D);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (4, el->i3->i1, 2, c, el->type, TYPE_INTEGER, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_2C_5A_D) != 0)
   {
      Invalid_attribute (5, 2, c, el->i3->i1, SDDFA_2C_5A_D);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (5, el->i3->i1, 2, c, el->type, TYPE_INTEGER, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_2C_6A_D) != 0)
   {
      Invalid_attribute (6, 2, c, el->i3->i1, SDDFA_2C_6A_D);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (6, el->i3->i1, 2, c, el->type, TYPE_INTEGER, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_2C_7A_D) != 0)
   {
      Invalid_attribute (7, 2, c, el->i3->i1, SDDFA_2C_7A_D);
      near_line ();
   }
   if ((el->type != TYPE_DOUBLE) || (el->i3->dimension != 0))
   {
      Invalid_type (7, el->i3->i1, 2, c, el->type, TYPE_DOUBLE, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_2C_8A_D) != 0)
   {
      Invalid_attribute (8, 2, c, el->i3->i1, SDDFA_2C_8A_D);
      near_line ();
   }
   if ((el->type != TYPE_DOUBLE) || (el->i3->dimension != 0))
   {
      Invalid_type (8, el->i3->i1, 2, c, el->type, TYPE_DOUBLE, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_2C_9A_D) != 0)
   {
      Invalid_attribute (9, 2, c, el->i3->i1, SDDFA_2C_9A_D);
      near_line ();
   }
   if ((el->type != TYPE_DOUBLE) || (el->i3->dimension != 0))
   {
      Invalid_type (9, el->i3->i1, 2, c, el->type, TYPE_DOUBLE, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_2C_10A_D) != 0)
   {
      Invalid_attribute (10, 2, c, el->i3->i1, SDDFA_2C_10A_D);
      near_line ();
   }
   if ((el->type != TYPE_DOUBLE) || (el->i3->dimension != 0))
   {
      Invalid_type (10, el->i3->i1, 2, c, el->type, TYPE_DOUBLE, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_2C_11A_D) != 0)
   {
      Invalid_attribute (11, 2, c, el->i3->i1, SDDFA_2C_11A_D);
      near_line ();
   }
   if ((el->type != TYPE_DOUBLE) || (el->i3->dimension != 0))
   {
      Invalid_type (11, el->i3->i1, 2, c, el->type, TYPE_DOUBLE, el->i3->dimension, 0);
      near_line ();
   }

   if (count_queue (q) == 15)
   {
     el = (struct t_element *) next_queue (q);
     if (strcmp (el->i3->i1, SDDFA_2C_12A_D) != 0)
     {
        Invalid_attribute (12, 2, c, el->i3->i1, SDDFA_2C_12A_D);
        near_line ();
     }
     if ((el->type != TYPE_DOUBLE) || (el->i3->dimension != 0))
     {
        Invalid_type (12, el->i3->i1, 2, c, el->type, TYPE_DOUBLE, el->i3->dimension, 0);
        near_line ();
     }
  
     el = (struct t_element *) next_queue (q);
     if (strcmp (el->i3->i1, SDDFA_2C_13A_D) != 0)
     {
        Invalid_attribute (13, 2, c, el->i3->i1, SDDFA_2C_13A_D);
        near_line ();
     }
     if ((el->type != TYPE_DOUBLE) || (el->i3->dimension != 0))
     {
        Invalid_type (13, el->i3->i1, 2, c, el->type, TYPE_DOUBLE, el->i3->dimension, 0);
        near_line ();
     }
  
     el = (struct t_element *) next_queue (q);
     if (strcmp (el->i3->i1, SDDFA_2C_14A_D) != 0)
     {
        Invalid_attribute (14, 2, c, el->i3->i1, SDDFA_2C_14A_D);
        near_line ();
     }
     if ((el->type != TYPE_DOUBLE) || (el->i3->dimension != 0))
     {
        Invalid_type (14, el->i3->i1, 2, c, el->type, TYPE_DOUBLE, el->i3->dimension, 0);
        near_line ();
     }
  
     el = (struct t_element *) next_queue (q);
     if (strcmp (el->i3->i1, SDDFA_2C_15A_D) != 0)
     {
        Invalid_attribute (15, 2, c, el->i3->i1, SDDFA_2C_15A_D);
        near_line ();
     }
     if ((el->type != TYPE_DOUBLE) || (el->i3->dimension != 0))
     {
        Invalid_type (15, el->i3->i1, 2, c, el->type, TYPE_DOUBLE, el->i3->dimension, 0);
        near_line ();
     }
   }
}



void
check_record_2c(char *c, struct t_queue *q)
{
   if (strcmp (c, SDDFA_2C_NAME) != 0)
   {
      fill_parse_error ("Incorrect name for register #2\n");
      fill_parse_error ("It is %s and must be %s\n", c, SDDFA_2C_NAME);
      near_line ();
      return;
   }

   if ((count_queue (q) == 9) || (count_queue (q) == 13))
     check_record_2c_old_format(c,q);
   else if ((count_queue (q) == 11) || (count_queue (q) == 15))
     check_record_2c_DAMIEN_format(c,q);
   else
   {
      fill_parse_error ("Incorrect number of elements in register #2: %s\n", c);
      fill_parse_error ("There are %d and must be 9, 11, 13 or 15\n", count_queue (q));
      near_line ();
   }
}




void
check_record_3c(char *c, struct t_queue *q)
{
   struct t_element *el;

   if (strcmp (c, SDDFA_3C_NAME) != 0)
   {
      fill_parse_error ("Incorrect name for register #3\n");
      fill_parse_error ("It is %s and must be %s\n", c, SDDFA_3C_NAME);
      near_line ();
      return;
   }

   if ((count_queue (q) != 4) && (count_queue (q) != 3))  
   {
      fill_parse_error ("Incorrect number of elements in register #3: %s\n", c);
      fill_parse_error ("There are %d and must be 3 or 4\n", count_queue (q));
      near_line ();
   }

   el = (struct t_element *) head_queue (q);
   if (strcmp (el->i3->i1, SDDFA_3C_1A) != 0)
   {
      Invalid_attribute (1, 3, c, el->i3->i1, SDDFA_3C_1A);
      near_line ();
   }
   if ((el->type != TYPE_CHAR) || (el->i3->dimension != 1))
   {
      Invalid_type (1, el->i3->i1, 3, c, el->type, 2, el->i3->dimension, 1);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_3C_2A) != 0)
   {
      Invalid_attribute (2, 3, c, el->i3->i1, SDDFA_3C_2A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (2, el->i3->i1, 3, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_3C_3A) != 0)
   {
      Invalid_attribute (3, 3, c, el->i3->i1, SDDFA_3C_3A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 1))
   {
      Invalid_type (3, el->i3->i1, 3, c, el->type, 0, el->i3->dimension, 1);
      near_line ();
   }

   if (count_queue (q) == 4)
   {
     el = (struct t_element *) next_queue (q);
     if (strcmp (el->i3->i1, SDDFA_3C_4A) != 0)
     {
        Invalid_attribute (3, 3, c, el->i3->i1, SDDFA_3C_4A);
        near_line ();
     }
     if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 1))
     {
        Invalid_type (3, el->i3->i1, 3, c, el->type, 0, el->i3->dimension, 1);
        near_line ();
     }
   }

}

void
check_record_4c (char *c, struct t_queue *q)
{
   struct t_element *el;

   if (strcmp (c, SDDFA_4C_NAME) != 0)
   {
      fill_parse_error ("Incorrect name for register #4\n");
      fill_parse_error ("It is %s and must be %s\n", c, SDDFA_4C_NAME);
      near_line ();
      return;
   }
   if (count_queue (q) != 4)
   {
      fill_parse_error ("Incorrect number of elements in register #4: %s\n", c);
      fill_parse_error ("There are %d and must be 4\n", count_queue (q));
      near_line ();
      return;
   }

   el = (struct t_element *) head_queue (q);
   if (strcmp (el->i3->i1, SDDFA_4C_1A) != 0)
   {
      Invalid_attribute (1, 4, c, el->i3->i1, SDDFA_4C_1A);
      near_line ();
   }
   if ((el->type != TYPE_CHAR) || (el->i3->dimension != 1))
   {
      Invalid_type (1, el->i3->i1, 4, c, el->type, 2, el->i3->dimension, 1);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_4C_2A) != 0)
   {
      Invalid_attribute (2, 4, c, el->i3->i1, SDDFA_4C_2A);
      near_line ();
   }
   if ((el->type != TYPE_CHAR) || (el->i3->dimension != 1))
   {
      Invalid_type (2, el->i3->i1, 4, c, el->type, 2, el->i3->dimension, 1);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_4C_3A) != 0)
   {
      Invalid_attribute (3, 4, c, el->i3->i1, SDDFA_4C_3A);
      near_line ();
   }
   if ((el->type != TYPE_CHAR) || (el->i3->dimension != 1))
   {
      Invalid_type (3, el->i3->i1, 4, c, el->type, 2, el->i3->dimension, 1);
      near_line ();
   }
   
   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_4C_4A) != 0)
   {
      Invalid_attribute (4, 4, c, el->i3->i1, SDDFA_4C_4A);
      near_line ();
   }
   if ((el->type != TYPE_CHAR) || (el->i3->dimension != 1))
   {
      Invalid_type (4, el->i3->i1, 4, c, el->type, 2, el->i3->dimension, 1);
      near_line ();
   }

}

void
check_record_5c(char *c, struct t_queue *q)
{
   struct t_element *el;

   if (strcmp (c, SDDFA_5C_NAME) != 0)
   {
      fill_parse_error ("Incorrect name for register #5\n");
      fill_parse_error ("It is %s and must be %s\n", c, SDDFA_5C_NAME);
      near_line ();
      return;
   }
   if (count_queue (q) != 2)
   {
      fill_parse_error ("Incorrect number of elements in register #5: %s\n", c);
      fill_parse_error ("There are %d and must be 2\n", count_queue (q));
      near_line ();
   }

   el = (struct t_element *) head_queue (q);
   if (strcmp (el->i3->i1, SDDFA_5C_1A) != 0)
   {
      Invalid_attribute (1, 1, c, el->i3->i1, SDDFA_5C_1A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (1, el->i3->i1, 1, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_5C_2A) != 0)
   {
      Invalid_attribute (2, 1, c, el->i3->i1, SDDFA_5C_2A);
      near_line ();
   }
   if ((el->type != TYPE_DOUBLE) || (el->i3->dimension != 0))
   {
      Invalid_type (2, el->i3->i1, 1, c, el->type, 1, el->i3->dimension, 0);
      near_line ();
   }
}


void
check_record_6c (char *c, struct t_queue *q)
{
   struct t_element *el;

   if (strcmp (c, SDDFA_6C_NAME) != 0)
   {
      fill_parse_error ("Incorrect name for register #6\n");
      fill_parse_error ("It is %s and must be %s\n", c, SDDFA_6C_NAME);
      near_line ();
      return;
   }
   if (count_queue (q) != 5)
   {
      fill_parse_error ("Incorrect number of elements in register #6: %s\n", c);
      fill_parse_error ("There are %d and must be 5\n", count_queue (q));
      near_line ();
   }

   el = (struct t_element *) head_queue (q);
   if (strcmp (el->i3->i1, SDDFA_6C_1A) != 0)
   {
      Invalid_attribute (1, 6, c, el->i3->i1, SDDFA_6C_1A);
      near_line ();
   }
   if ((el->type != TYPE_DOUBLE) || (el->i3->dimension != 0))
   {
      Invalid_type (1, el->i3->i1, 6, c, el->type, TYPE_DOUBLE, el->i3->dimension, 0);
      near_line ();
   }
   
   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_6C_2A) != 0)
   {
      Invalid_attribute (2, 6, c, el->i3->i1, SDDFA_6C_2A);
      near_line ();
   }
   if ((el->type != TYPE_DOUBLE) || (el->i3->dimension != 0))
   {
      Invalid_type (2, el->i3->i1, 6, c, el->type, TYPE_DOUBLE, el->i3->dimension, 0);
      near_line ();
   }
   
   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_6C_3A) != 0)
   {
      Invalid_attribute (3, 6, c, el->i3->i1, SDDFA_6C_3A);
      near_line ();
   }
   if ((el->type != TYPE_DOUBLE) || (el->i3->dimension != 0))
   {
      Invalid_type (3, el->i3->i1, 6, c, el->type, TYPE_DOUBLE, el->i3->dimension, 0);
      near_line ();
   }
   
   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_6C_4A) != 0)
   {
      Invalid_attribute (4, 6, c, el->i3->i1, SDDFA_6C_4A);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (4, el->i3->i1, 6, c, el->type, 0, el->i3->dimension, 0);
      near_line ();
   }
   
   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_6C_5A) != 0)
   {
      Invalid_attribute (5, 6, c, el->i3->i1, SDDFA_6C_5A);
      near_line ();
   }
   if ((el->type != TYPE_DOUBLE) || (el->i3->dimension != 0))
   {
      Invalid_type (5, el->i3->i1, 6, c, el->type, TYPE_DOUBLE, el->i3->dimension, 0);
      near_line ();
   }
}



void check_record_7c(char *c, struct t_queue *q)
{
  struct t_element *el;
   
  if (strcmp (c, SDDFA_7C_NAME) != 0)
  {
     fill_parse_error ("Incorrect name for register #7\n");
     fill_parse_error ("It is %s and must be %s\n", c, SDDFA_7C_NAME);
     near_line ();
     return;
  }
  if (count_queue (q) != 13)
  {
     fill_parse_error ("Incorrect number of elements in register #7: %s\n", c);;
     fill_parse_error ("There are %d and must be 13\n", count_queue (q));
     near_line ();
     return;
  }
  
   el = (struct t_element *) head_queue (q);
   if (strcmp (el->i3->i1, SDDFA_7C_1A_D) != 0)
   {
      Invalid_attribute (1, 7, c, el->i3->i1, SDDFA_7C_1A_D);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (1, el->i3->i1, 7, c, el->type, TYPE_INTEGER, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_7C_2A_D) != 0)
   {
      Invalid_attribute (2, 7, c, el->i3->i1, SDDFA_7C_2A_D);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (2, el->i3->i1, 7, c, el->type, TYPE_INTEGER, el->i3->dimension, 0);
      near_line ();
   }
 
   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_7C_3A_D) != 0)
   {
      Invalid_attribute (3, 7, c, el->i3->i1, SDDFA_7C_3A_D);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (3, el->i3->i1, 7, c, el->type, TYPE_INTEGER, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_7C_4A_D) != 0)
   {
      Invalid_attribute (4, 7, c, el->i3->i1, SDDFA_7C_4A_D);
      near_line ();
   }
   if ((el->type != TYPE_DOUBLE) || (el->i3->dimension != 0))
   {
      Invalid_type (4, el->i3->i1, 7, c, el->type, TYPE_DOUBLE, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_7C_5A_D) != 0)
   {
      Invalid_attribute (5, 7, c, el->i3->i1, SDDFA_7C_5A_D);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 1))
   {
      Invalid_type (5, el->i3->i1, 7, c, el->type, TYPE_INTEGER, el->i3->dimension, 1);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_7C_6A_D) != 0)
   {
      Invalid_attribute (6, 7, c, el->i3->i1, SDDFA_7C_6A_D);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (6, el->i3->i1, 7, c, el->type, TYPE_INTEGER, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_7C_7A_D) != 0)
   {
      Invalid_attribute (7, 7, c, el->i3->i1, SDDFA_7C_7A_D);
      near_line ();
   }
   if ((el->type != TYPE_CHAR) || (el->i3->dimension != 1))
   {
      Invalid_type (7, el->i3->i1, 7, c, el->type, TYPE_CHAR, el->i3->dimension, 1);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_7C_8A_D) != 0)
   {
      Invalid_attribute (8, 7, c, el->i3->i1, SDDFA_7C_8A_D);
      near_line ();
   }
   if ((el->type != TYPE_CHAR) || (el->i3->dimension != 1))
   {
      Invalid_type (8, el->i3->i1, 7, c, el->type, TYPE_CHAR, el->i3->dimension, 1);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_7C_9A_D) != 0)
   {
      Invalid_attribute (9, 7, c, el->i3->i1, SDDFA_7C_9A_D);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 0))
   {
      Invalid_type (9, el->i3->i1, 7, c, el->type, TYPE_INTEGER, el->i3->dimension, 0);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_7C_10A_D) != 0)
   {
      Invalid_attribute (10, 7, c, el->i3->i1, SDDFA_7C_10A_D);
      near_line ();
   }
   if ((el->type != TYPE_CHAR) || (el->i3->dimension != 1))
   {
      Invalid_type (10, el->i3->i1, 7, c, el->type, TYPE_CHAR, el->i3->dimension, 1);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_7C_11A_D) != 0)
   {
      Invalid_attribute (11, 7, c, el->i3->i1, SDDFA_7C_11A_D);
      near_line ();
   }
   if ((el->type != TYPE_INTEGER) || (el->i3->dimension != 1))
   {
      Invalid_type (11, el->i3->i1, 7, c, el->type, TYPE_INTEGER, el->i3->dimension, 1);
      near_line ();
   }

   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_7C_12A_D) != 0)
   {
      Invalid_attribute (12, 7, c, el->i3->i1, SDDFA_7C_12A_D);
      near_line ();
   }
   if ((el->type != TYPE_DOUBLE) || (el->i3->dimension != 0))
   {
      Invalid_type (12, el->i3->i1, 7, c, el->type, TYPE_DOUBLE, el->i3->dimension, 0);
      near_line ();
   }


   el = (struct t_element *) next_queue (q);
   if (strcmp (el->i3->i1, SDDFA_7C_13A_D) != 0)
   {
      Invalid_attribute (13, 7, c, el->i3->i1, SDDFA_7C_13A_D);
      near_line ();
   }
   if ((el->type != TYPE_DOUBLE) || (el->i3->dimension != 0))
   {
      Invalid_type (13, el->i3->i1, 7, c, el->type, TYPE_DOUBLE, el->i3->dimension, 0);
      near_line ();
   }
}






void
check_fields_with_structure(struct t_queue *q, struct t_entry *en)
{
  struct t_element *el;
  struct t_field *f, *f1;
  int             k;
  int             i[10];
  double          j[10];
  char           *s[10];
  register struct t_action *action;
  struct t_node  *node;
  char *bn, *fn, *an;
  register double tmp_double;
  double relative;
  int *tm;

  k = 0;
  bzero(&i[0],10*sizeof(int));
  bzero(&j[0],10*sizeof(double));
  bzero(&s[0],10*sizeof(char *));
  f = (struct t_field *) head_queue (q);
  
  for (
    el  = (struct t_element *) head_queue (en->types);
    el != (struct t_element *) 0;
    el  = (struct t_element *) next_queue (en->types),
    f   = (struct t_field *) next_queue (q)
  )
  {
    if (f == (struct t_field *) 0)
    {
      fill_parse_error ("Invalid register. Not enough fields.");
      near_line ();
      return;
    }
    if ((f->tipo != el->type) && (f->tipo!=3))
    {
      fill_parse_error ("Incompatible types between definition and usage\n");
      fill_parse_error ("It is %s %d and must be %s %d\n", s[f->tipo],f->tipo, s[el->type], el->type);
      near_line ();
      return;
    }

    if (f->tipo == TYPE_INTEGER)
    {
      i[k++] = f->value.dec;
    }
    else
    {
      if (f->tipo == TYPE_DOUBLE)
      {
        j[k++] = f->value.real;
      }
      else
      {
        if (f->tipo == TYPE_CHAR)
        {
          s[k++] = f->value.string;
        }
        else
        {
          f->tipo = TYPE_INTEGER;
        }
      }
    }

    switch (el->i3->dimension)
    {
      case 0:
        break;
      case 1:
        break;
      case 2:
        break;
      default:
        fill_parse_error ("More than two dimensions\n");
        exit (1);
        break;
    }
  }
  if (dimemas_GUI)
  {
    one_more_record++;
    tmp_double = (double)one_more_record/aprox_number_of_records;
    
    if (tmp_double>(last_value_load_in_curse+0.05))
    {
      last_value_load_in_curse = tmp_double;
      update_loading (last_value_load_in_curse);
    }
    if (
      (en->entryid !=  42) && (en->entryid !=  43) &&
      (en->entryid != 100) && (en->entryid != 200) && 
      (en->entryid != 300) && (en->entryid != 400)
    )
    {
      max_task_id = MAX (i[0] + 1, max_task_id);
    }
    
    switch (en->entryid)
    {
      case 40:
      case 41:
        new_block_identifier (i[2]);
        break;
      case 42:
        bn = mallocame (strlen(s[1])+1);
        strcpy(bn, s[1]);
        an = mallocame (strlen(s[2])+1);
        strcpy(an, s[2]);
        new_block_name(i[0],bn,an,i[3],i[4]);
        break;
      case 43:
        fn = mallocame (strlen(s[1])+1);
        strcpy(fn,s[1]); 
        new_file_name(i[0],fn);
        break;
      default:
        break;
    }
  }

  #ifndef LIBLEXYACC
  else
  {
    action = (struct t_action *) mallocame (sizeof (struct t_action));
    action->action = en->entryid;
    action->next   = AC_NIL;
    switch (en->entryid)
    {
      case 1:
        action->desc.compute.cpu_time = j[2] * 1e6;
        node = get_node_for_task_by_name (Ptask_current, i[0] + 1);
        relative = node->relative;
        /*
        if (randomness.processor_ratio.distribution!=NO_DISTRIBUTION)
        {
          relative += random_dist(&randomness.processor_ratio);
        }
        */
        if (relative<=0)
        {
          (action->desc).compute.cpu_time = 0.0;
        }
        else
        {
          (action->desc).compute.cpu_time =
             (t_micro) (((action->desc).compute.cpu_time) / relative);
          (action->desc).compute.cpu_time += random_dist(&randomness.processor_ratio);
          
          if ((action->desc).compute.cpu_time <(float)0)
          {
            (action->desc).compute.cpu_time = 0.0;
          }
        }
        /*
        if ((action->desc).compute.cpu_time <= 1.0)
        {
         (action->desc).compute.cpu_time = 1.0;
        }
        */
        break;
      case 2:
        action->desc.send.dest        = i[2] + 1;
        action->desc.send.mess_size   = i[3];
        action->desc.send.mess_tag    = i[4];
        action->desc.send.communic_id = i[5];
        /* action->desc.send.immediate   = 1; */  
        /* El bit mes baix del camp i[6] indica si el send es sincron o no. */
        action->desc.send.rendez_vous = USE_RENDEZ_VOUS((i[6] & ((int)1)),i[3]);
        /* El segon bit mes baix de i[6] indica si es un immediate send o no */
        if (i[6] & ((int)2))
        {  
          action->desc.send.immediate = TRUE;
        }
        else
        {
          action->desc.send.immediate = FALSE;
        }
        /*
        fprintf(
          stderr,
          "Un SEND amb i[6]=%d, i[3]=%d i rendez_vous =%d\n",
          i[6],i[3],action->desc.send.rendez_vous
        );
        */
        if (locate_task (Ptask_current, i[2] + 1) == T_NIL)
        {
          near_line ();
          panic ("Invalid destination taskid %d for message\n", i[2] + 1);
        }
        break;
      case 3:
        action->desc.recv.ori = i[2] + 1;
        action->desc.recv.mess_size = i[3];
        action->desc.recv.mess_tag = i[4];
        action->desc.recv.communic_id = i[5];
        /* Per defecte l'accio ja es un RECV */
        if (i[6]==1)
        {
          action->action=IRECV;
        }
        else if (i[6]==2)
        {
          action->action=WAIT;
        }
        if (
          (locate_task (Ptask_current, i[2] + 1) == T_NIL) &&
          (!(action->action==IRECV && i[4]==-1))
        ) /* No es Irecv amb MPI_ANY_TAG */
        {
          near_line ();
          panic ("Invalid source taskid %d for message\n", i[2] + 1);
        }
        break;
      case 40:
        action->action = EVEN;
        action->desc.even.type = BLOCK_BEGIN;
        action->desc.even.value = i[2];
        module_entrance (Ptask_current, i[0] + 1, i[1] + 1, i[2]);
        break;
      case 41:
        action->action = EVEN;
        action->desc.even.type = BLOCK_END;
        action->desc.even.value = i[2];
        module_exit (Ptask_current, i[0] + 1, i[1] + 1, i[2]);
        break;
      case 42:
        bn = mallocame (strlen(s[1])+1);
        strcpy(bn, s[1]);
        an = mallocame (strlen(s[2])+1);
        strcpy(an, s[2]);
        module_name (Ptask_current,i[0],bn,an,i[3],i[4]);
        break;
      case 43:
        fn = mallocame (strlen(s[1])+1);
        strcpy(fn, s[1]);
        file_name (Ptask_current,i[0],fn);
        break;
      case 48:
        bn = mallocame (strlen(s[1])+1);
        strcpy(bn, s[1]);
        user_event_type_name (Ptask_current,i[0],bn,i[2]);
        break;
      case 49:
        bn = mallocame (strlen(s[2])+1);
        strcpy(bn, s[2]);
        user_event_value_name (Ptask_current,i[0],i[1],bn);
        break;
      case 50:
        action->action = EVEN;
        action->desc.even.type = i[2];
        action->desc.even.value = i[3];
        break;
      case 101:
        action->action = FS;
        action->desc.fs_op.which_fsop                = OPEN;
        action->desc.fs_op.fs_o.fs_open.fd           = i[2];
        action->desc.fs_op.fs_o.fs_open.initial_size = i[3];
        action->desc.fs_op.fs_o.fs_open.flags        = i[4];
        strncpy (action->desc.fs_op.fs_o.fs_open.filename, s[5], FILEMAX);
        action->desc.fs_op.fs_o.fs_open.filename[FILEMAX] = (char) 0;
        break;
      case 102:
        action->action = FS;
        action->desc.fs_op.which_fsop                  = READ;
        action->desc.fs_op.fs_o.fs_read.fd             = i[2];
        action->desc.fs_op.fs_o.fs_read.requested_size = i[3];
        action->desc.fs_op.fs_o.fs_read.delivered_size = i[4];
        break;
      case 103:
        action->action = FS;
        action->desc.fs_op.which_fsop                   = WRITE;
        action->desc.fs_op.fs_o.fs_write.fd             = i[2];
        action->desc.fs_op.fs_o.fs_write.requested_size = i[3];
        action->desc.fs_op.fs_o.fs_write.delivered_size = i[4];
        break;
      case 104:
        action->action = FS;
        action->desc.fs_op.which_fsop          = SEEK;
        action->desc.fs_op.fs_o.fs_seek.fd     = i[2];
        action->desc.fs_op.fs_o.fs_seek.offset = i[3];
        action->desc.fs_op.fs_o.fs_seek.whence = i[4];
        action->desc.fs_op.fs_o.fs_seek.result = i[5];
        break;
      case 105:
        action->action = FS;
        action->desc.fs_op.which_fsop = CLOSE;
        action->desc.fs_op.fs_o.fs_close.fd = i[2];
        break;
      case 106:
        action->action = FS;
        action->desc.fs_op.which_fsop         = DUP;
        action->desc.fs_op.fs_o.fs_dup.old_fd = i[2];
        action->desc.fs_op.fs_o.fs_dup.new_fd = i[3];
        break;
      case 107:
        action->action = FS;
        action->desc.fs_op.which_fsop = UNLINK;
        strncpy (action->desc.fs_op.fs_o.fs_unlink.filename, s[2], FILEMAX);
        action->desc.fs_op.fs_o.fs_unlink.filename[FILEMAX] = (char) 0;
        break;
      case 100:
        new_communicator_definition (Ptask_current, i[0]);
        f = (struct t_field *) tail_queue (q);
        if (i[1]!=count_queue(f->value.arr.q))
        {
          fill_parse_error(
            "Not matching number of ranks %d vs %d\n",
            i[1], count_queue(f->value.arr.q)
          );
          near_line();
          return;
        }

        f = (struct t_field *) tail_queue (q);
        for (
          f1  = (struct t_field *)head_queue(f->value.arr.q);
          f1 != (struct t_field *)0;
          f1  = (struct t_field *)next_queue(f->value.arr.q))
        {
          add_identificator_to_communicator(Ptask_current, i[0], f1->value.dec);
        }
        no_more_identificator_to_communicator(Ptask_current, i[0]);
        break;
      case 200:
        new_global_op(i[0], s[1]);
        break;
      case 201:
        action->action = GLOBAL_OP;
        action->desc.global_op.glop_id = i[2];
        action->desc.global_op.comm_id = i[3];
        action->desc.global_op.root_rank = i[4];
        action->desc.global_op.root_thid = i[5];
        action->desc.global_op.bytes_send = i[6];
        action->desc.global_op.bytes_recvd = i[7];
        break;
      case 300:
        new_io_operation(i[0], s[1]);
        break;
      case 301:
        action->action = MPI_IO;
        action->desc.mpi_io.which_io = MPI_IO_Metadata;
        action->desc.mpi_io.commid = i[2];
        action->desc.mpi_io.fh = i[3];
        action->desc.mpi_io.Oop = i[4];
        break;
      case 302:
        action->action = MPI_IO;
        action->desc.mpi_io.which_io = MPI_IO_Block_NonCollective;
        action->desc.mpi_io.size = i[2];
        action->desc.mpi_io.Oop = i[3];
        break;
      case 303:
        action->action = MPI_IO;
        action->desc.mpi_io.which_io = MPI_IO_Block_Collective;
        action->desc.mpi_io.fh = i[2];
        action->desc.mpi_io.size = i[3];
        action->desc.mpi_io.Oop = i[4];
        break;
      case 304:
        action->action = MPI_IO;
        action->desc.mpi_io.which_io = MPI_IO_NonBlock_NonCollective_Begin;
        action->desc.mpi_io.size = i[2];
        action->desc.mpi_io.request = i[3];
        action->desc.mpi_io.Oop = i[4];
        break;
      case 305:
        action->action = MPI_IO;
        action->desc.mpi_io.which_io = MPI_IO_NonBlock_NonCollective_End;
        action->desc.mpi_io.request = i[2];
        action->desc.mpi_io.Oop = i[3];
        break;
      case 306:
        action->action = MPI_IO;
        action->desc.mpi_io.which_io = MPI_IO_NonBlock_Collective_Begin;
        action->desc.mpi_io.fh = i[2];
        action->desc.mpi_io.size = i[3];
        action->desc.mpi_io.Oop = i[4];
        break;
      case 307:
        action->action = MPI_IO;
        action->desc.mpi_io.which_io = MPI_IO_NonBlock_Collective_End;
        action->desc.mpi_io.fh = i[2];
        action->desc.mpi_io.Oop = i[3];
        break;
      case 400:
        new_window_definition (Ptask_current, i[0]);
        f = (struct t_field *) tail_queue (q);
        if (i[1]!=count_queue(f->value.arr.q))
        {
          fill_parse_error(
            "window error: Not matching number of ranks %d vs %d\n",
            i[1], count_queue(f->value.arr.q)
          );
          near_line();
          return;
        }
      
        f = (struct t_field *) tail_queue (q);
        for (
          f1  = (struct t_field *)head_queue(f->value.arr.q);
          f1 != (struct t_field *)0;
          f1  = (struct t_field *)next_queue(f->value.arr.q)
        )
        {
          add_identificator_to_window (Ptask_current, i[0], f1->value.dec);
        }
        no_more_identificator_to_window(Ptask_current, i[0]);
        break;
      case 401:
        action->action = MPI_OS;
        action->desc.mpi_os.which_os = MPI_OS_GETPUT;
        action->desc.mpi_os.Oop = i[2];
        action->desc.mpi_os.target_rank = i[3];
        action->desc.mpi_os.window_id = i[4];
        action->desc.mpi_os.size = i[5];
        break;
      case 402:
        action->action = MPI_OS;
        action->desc.mpi_os.which_os = MPI_OS_FENCE;
        action->desc.mpi_os.window_id = i[2];
        break;
      case 403:
        action->action = MPI_OS;
        action->desc.mpi_os.which_os = MPI_OS_LOCK;
        action->desc.mpi_os.window_id = i[2];
        action->desc.mpi_os.Oop = i[3];
        action->desc.mpi_os.mode = i[4];
        break;
      case 404:
        action->action = MPI_OS;
        action->desc.mpi_os.which_os = MPI_OS_POST;
        action->desc.mpi_os.window_id = i[2];
        action->desc.mpi_os.Oop = i[3];
        action->desc.mpi_os.post_size = i[4];
        
        create_queue (&action->desc.mpi_os.post_ranks);
        
        if ((i[3]==MPI_OS_POST_POST) || (i[3]==MPI_OS_POST_START))
        {
          f = (struct t_field *) tail_queue (q);
          if (i[4]!=count_queue(f->value.arr.q))
          {
            fill_parse_error(
              "window error: Not matching number of ranks %d vs %d\n",
              i[4], count_queue(f->value.arr.q)
            );
            near_line();
            return;
          }
      
          f = (struct t_field *) tail_queue (q);
          for (
            f1  = (struct t_field *)head_queue(f->value.arr.q);
            f1 != (struct t_field *)0;
            f1  = (struct t_field *)next_queue(f->value.arr.q)
          )
          {
            tm  = (int *)mallocame (sizeof(int));
            *tm = f1->value.dec;
            inFIFO_queue (&action->desc.mpi_os.post_ranks, (char *)tm);
          }
        }
        break;
      default:
        fill_parse_error ("Undefined register number %d\n", en->entryid);
        near_line ();
        en->entryid = -1;
        break;
    }

    if (
      (en->entryid != -1)  &&
      (en->entryid != 42)  &&
      (en->entryid != 43)  &&
      (en->entryid != 48)  &&
      (en->entryid != 49)  &&
      (en->entryid != 100) &&
      (en->entryid != 200) &&
      (en->entryid != 300) &&
      (en->entryid != 400)
    )
    {
      new_action_to_thread (Ptask_current, i[0] + 1, i[1] + 1, action);
    }
    else
    {
      freeame ((char *)action, sizeof (struct t_action));
    }

  }
  #endif  /* LIBLEXYACC */

  for (
    f  = (struct t_field *) outFIFO_queue (q);
    f != (struct t_field *) 0;
    f  = (struct t_field *) outFIFO_queue (q)
  )
  {
    freeame ((char *) f, sizeof (struct t_field));
  }
  freeame ((char *) q, sizeof (struct t_queue));
}





#define tipos_incompatibles(a,b) \
{fill_parse_error ("Incompatible types between definition and usage\n");\
 fill_parse_error ("It is %s and must be %s\n", s[a], s[b]); \
 near_line ();\
 return;\
}



void new_record_0c_DAMIEN_format(struct t_queue *q, struct t_entry *en)
{
  struct t_field   *f;
  struct t_element *el;

  f  = (struct t_field *) head_queue (q);
  el = (struct t_element *) head_queue (en->types);
  if (f->tipo != el->type)
    tipos_incompatibles (f->tipo, el->type);

  sim_char.general_net.name = (char *) mallocame (strlen (f->value.string) + 1);
  strcpy (sim_char.general_net.name, f->value.string);

  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
    tipos_incompatibles (f->tipo, el->type);

  if (f->value.dec <= 0)
  {
    near_line ();
    fill_parse_error ("Invalid number of machines: %d \n", f->value.dec);
    return;
  }
  sim_char.number_machines = f->value.dec;
  
  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
    tipos_incompatibles (f->tipo, el->type);

  if (f->value.dec < 0)
  {
    near_line ();
    fill_parse_error ("Invalid number of dedicated connections: %d \n",
                      f->value.dec);
    return;
  }
  sim_char.dedicated_connections.number_connections = f->value.dec;
   
  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);
 
  if (f->tipo != el->type)
    tipos_incompatibles (f->tipo, el->type);

  if ((f->value.dec < 1) || (f->value.dec > 4))
  {
    near_line ();
    fill_parse_error ("Invalid function of traffic: %d \n", f->value.dec);
    return;
  }
  sim_char.general_net.traffic_function = f->value.dec;
  
  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
    tipos_incompatibles (f->tipo, el->type);
  
  if (f->value.real < 0)
  {
    near_line ();
    fill_parse_error ("Invalid maximmum traffic value: %le \n", f->value.real);
    return;
  }
  sim_char.general_net.max_traffic_value = f->value.real;
  
  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);
  
  if (f->tipo != el->type)
    tipos_incompatibles (f->tipo, el->type);

  if (f->value.real < 0)
  {
    near_line ();
    fill_parse_error ("Invalid external network bandwidth: %le \n", 
                      f->value.real);
    return;
  }
  sim_char.general_net.bandwidth = f->value.real;

  f  = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);

  if (f->tipo != el->type)
    tipos_incompatibles (f->tipo, el->type);

  if ((f->value.dec < 1) || (f->value.dec > 3))
  {
    near_line ();
    fill_parse_error ("Invalid model for global communication: %d \n",
                      f->value.dec);
    return;
  }
  sim_char.general_net.global_operation = f->value.dec;

  /* FEC: Un cop tenim totes les dades cal crear aqui el numero 
          de maquines que ens han indicat. */
  Initialize_empty_machine(&Machine_queue);
  /* Un cop creades les maquines ja es poden crear les connexions
     dedicades que ens han indicat */
  Initialize_empty_connection(&Dedicated_Connections_queue);
}

void
new_record_1c_old_format(struct t_queue *q, struct t_entry *en)
{
  struct t_field   *f;
  struct t_element *el;
  struct t_machine  *machine;

  /* FEC: Com que utilitza el format vell, segur que hi ha una
          sola maquina. Per tant, s'ha de crear aqui i 
          nomes caldra agafar la primera. */
  Initialize_empty_machine(&Machine_queue);
  machine=(struct t_machine *)head_queue(&Machine_queue);
  if (machine==MA_NIL) panic("No hi ha cap maquina!\n");
  
	f = (struct t_field *) head_queue (q);
	el = (struct t_element *) head_queue (en->types);
	if (f->tipo != el->type)
	  tipos_incompatibles (f->tipo, el->type);
	machine->instrumented_arch = (char *) mallocame (strlen (f->value.string) + 1);
	strcpy (machine->instrumented_arch, f->value.string);

	f = (struct t_field *) next_queue (q);
	el = (struct t_element *) next_queue (en->types);
	if (f->tipo != el->type)
	   tipos_incompatibles (f->tipo, el->type);
	if (f->value.dec <= 0)
  {
	  near_line ();
	  fill_parse_error ("Invalid number of nodes on virutal machine: %d \n",
		                  f->value.dec);
	  return;
	}
	machine->number_nodes = f->value.dec;

	f = (struct t_field *) next_queue (q);
	el = (struct t_element *) next_queue (en->types);
	if (f->tipo != el->type)
	  tipos_incompatibles (f->tipo, el->type);
	if (f->value.real < 0)
	{
	  near_line ();
	  fill_parse_error ("Invalid network communication bandwidth: %le\n",
			      f->value.real);
	  return;
	}
	machine->communication.remote_bandwith = f->value.real;
    
	f = (struct t_field *) next_queue (q);
	el = (struct t_element *) next_queue (en->types);
	if (f->tipo != el->type)
	  tipos_incompatibles (f->tipo, el->type);
	if (f->value.dec < 0)
	{
	  near_line ();
	  fill_parse_error ("Invalid number of buses on network: %d \n",
			      f->value.dec);
	  return;
	}
  machine->communication.num_messages_on_network = f->value.dec;
       
	f = (struct t_field *) next_queue (q);
	el = (struct t_element *) next_queue (en->types);
	if (f->tipo != el->type)
	  tipos_incompatibles (f->tipo, el->type);
	if ((f->value.dec < 1) || (f->value.dec >3))
	{
	  near_line ();
	  fill_parse_error ("Invalid model for global communication: %d \n",
			      f->value.dec);
	  return;
	}
  machine->communication.global_operation = f->value.dec;

	Initialize_empty_node (&Node_queue, machine);
}




void new_record_1c_DAMIEN_format(struct t_queue *q, struct t_entry *en)
{
  struct t_field   *f;
  struct t_element *el;
  struct t_machine  *machine;
  int ma_number;
  char *str_aux;

  
	f = (struct t_field *) head_queue (q);
	el = (struct t_element *) head_queue (en->types);
	if (f->tipo != el->type)
	  tipos_incompatibles (f->tipo, el->type);
  str_aux = (char *) mallocame (strlen (f->value.string) + 1);
	strcpy (str_aux, f->value.string);

	f = (struct t_field *) next_queue (q);
	el = (struct t_element *) next_queue (en->types);
	if (f->tipo != el->type)
	   tipos_incompatibles (f->tipo, el->type);
	if (f->value.dec < 0)
  {
	  near_line ();
	  fill_parse_error ("Invalid number of machine identifier: %d \n",
		                  f->value.dec);
	  return;
	}
  ma_number = f->value.dec + 1;
  /* FEC: Com que utilitza el format nou, s'ha d'obtenir la maquina amb
          l'identificador donat. I se suposa que ja haura estat creada
          al llegir el "wide area network information", que es on diu
          el numero de maquines. */
  machine=(struct t_machine *)query_prio_queue(&Machine_queue, (t_priority)ma_number);
  if (machine==MA_NIL) panic("Machine not found!\n");
  /* Ara que ja tenim la maquina ja li podem assignar el nom */
  machine->name =str_aux;


	f = (struct t_field *) next_queue (q);
	el = (struct t_element *) next_queue (en->types);
	if (f->tipo != el->type)
	  tipos_incompatibles (f->tipo, el->type);
	machine->instrumented_arch = (char *) mallocame (strlen (f->value.string) + 1);
	strcpy (machine->instrumented_arch, f->value.string);

	f = (struct t_field *) next_queue (q);
	el = (struct t_element *) next_queue (en->types);
	if (f->tipo != el->type)
	   tipos_incompatibles (f->tipo, el->type);
	if (f->value.dec <= 0)
  {
	  near_line ();
	  fill_parse_error ("Invalid number of nodes on virutal machine: %d \n",
		                  f->value.dec);
	  return;
	}
	machine->number_nodes = f->value.dec;

	f = (struct t_field *) next_queue (q);
	el = (struct t_element *) next_queue (en->types);
	if (f->tipo != el->type)
	  tipos_incompatibles (f->tipo, el->type);
	if (f->value.real < 0)
	{
	  near_line ();
	  fill_parse_error ("Invalid network communication bandwidth: %le\n",
			      f->value.real);
	  return;
	}
	machine->communication.remote_bandwith = f->value.real;
    
	f = (struct t_field *) next_queue (q);
	el = (struct t_element *) next_queue (en->types);
	if (f->tipo != el->type)
	  tipos_incompatibles (f->tipo, el->type);
	if (f->value.dec < 0)
	{
	  near_line ();
	  fill_parse_error ("Invalid number of buses on internal network: %d \n",
			      f->value.dec);
	  return;
	}
  machine->communication.num_messages_on_network = f->value.dec;
       
	f = (struct t_field *) next_queue (q);
	el = (struct t_element *) next_queue (en->types);
	if (f->tipo != el->type)
	  tipos_incompatibles (f->tipo, el->type);
	if ((f->value.dec < 1) || (f->value.dec >3))
	{
	  near_line ();
	  fill_parse_error ("Invalid model for global communication: %d \n",
			      f->value.dec);
	  return;
	}
  machine->communication.global_operation = f->value.dec;

	Initialize_empty_node (&Node_queue, machine);
}





void new_record_2c_old_format(struct t_queue *q, struct t_entry *en)
{
  struct t_field   *f;
  struct t_element *el;
  struct t_machine *machine;
  int               no_number;
  char             *node_name;
  int               no_processors;
  int               no_input,
                    no_output;
  double            local_startup,
                    remote_startup;
  double            relative;
  double            local_bandwith;
  double            external_net_startup;
  double            local_port_startup,
                    remote_port_startup;
  double            local_memory_startup,
                    remote_memory_startup;
  

  /* FEC: Com que utilitza el format vell, segur que hi ha una
          sola maquina. Per tant, nomes caldra agafar la primera. */
  machine=(struct t_machine *)head_queue(&Machine_queue);
  if (machine==MA_NIL) panic("No hi ha cap maquina!\n");

  external_net_startup=0;

	f = (struct t_field *) head_queue (q);
  el = (struct t_element *) head_queue (en->types);
  if (f->tipo != el->type)
    tipos_incompatibles (f->tipo, el->type);
  no_number = f->value.dec + 1;

  f = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);
  if (f->tipo != el->type)
    tipos_incompatibles (f->tipo, el->type);
  node_name = (char *) mallocame (strlen (f->value.string) + 1);
  strcpy (node_name, f->value.string);

  f = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);
  if (f->tipo != el->type)
    tipos_incompatibles (f->tipo, el->type);
  if (f->value.dec <= 0)
  {
    near_line ();
    fill_parse_error ("Invalid number of processors (%d) for node %d\n",
                      f->value.dec, no_number - 1);
    return;
  }
  no_processors = f->value.dec;

  f = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);
  if (f->tipo != el->type)
    tipos_incompatibles (f->tipo, el->type);
  if (f->value.dec < 0)
  {
    near_line ();
    fill_parse_error ("Invalid number of input links (%d) for node %d\n",
                      f->value.dec, no_number - 1);
    return;
  }
  no_input = f->value.dec;

  f = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);
  if (f->tipo != el->type)
    tipos_incompatibles (f->tipo, el->type);
  if (f->value.dec < 0)
  {
    near_line ();
    fill_parse_error ("Invalid number of output links (%d) for node %d\n",
                      f->value.dec, no_number - 1);
    return;
  }
  no_output = f->value.dec;
         
  if ((no_input==0) && (no_output==0))
  {
    near_line ();
    fill_parse_error ("Invalid number of half duplex  links (%d) for node %d\n",
			                f->value.dec, no_number - 1);
    return;
  }

  f = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);
  if (f->tipo != el->type)
    tipos_incompatibles (f->tipo, el->type);
  if (f->value.real < 0)
  {
    near_line ();
    fill_parse_error ("Invalid startup on local communication (%le) for node %d\n",
		          	      f->value.real, no_number - 1);
    return;
  }
  local_startup = f->value.real * 1e6;

  f = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);
  if (f->tipo != el->type)
    tipos_incompatibles (f->tipo, el->type);
  if (f->value.real < 0)
  {
    near_line ();
    fill_parse_error ("Invalid startup on remote communication (%le) for node %d\n",
                      f->value.real, no_number - 1);
    return;
  }
  remote_startup = f->value.real * 1e6;

  f = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);
  if (f->tipo != el->type)
    tipos_incompatibles (f->tipo, el->type);
  if (f->value.real <= 0)
  {
    near_line ();
    fill_parse_error ("Invalid relative processor speed (%le) for node %d\n",
                      f->value.real, no_number - 1);
    return;
  }
  relative = f->value.real;

  f = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);
  if (f->tipo != el->type)
    tipos_incompatibles (f->tipo, el->type);
  if (f->value.real < 0)
  {
    near_line ();
    fill_parse_error ("Invalid memory bandwidth (%le) for node %d\n",
                      f->value.real, no_number - 1);
    return;
  }
  local_bandwith = f->value.real;

  f = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);
  if (el!=(struct t_element *)0)
  {
    if (f->tipo != el->type)
      tipos_incompatibles (f->tipo, el->type);
    if (f->value.real < 0)
    {
      near_line ();
      fill_parse_error ("Invalid local port startup (%le) for node %d\n",
                        f->value.real, no_number - 1);
      return;
    }
    local_port_startup = f->value.real * 1e6;
  }
  else
    local_port_startup = 0.0;

  f = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);
  if (el!=(struct t_element *)0)
  {
    if (f->tipo != el->type)
      tipos_incompatibles (f->tipo, el->type);
    if (f->value.real < 0)
    {
      near_line ();
      fill_parse_error ("Invalid remote port startup (%le) for node %d\n",
                        f->value.real, no_number - 1);
      return;
    }
    remote_port_startup = f->value.real * 1e6;
  }
  else
    remote_port_startup = 0.0;

  f = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);
  if (el!=(struct t_element *)0)
  {
    if (f->tipo != el->type)
      tipos_incompatibles (f->tipo, el->type);
    if (f->value.real < 0)
    {
      near_line ();
      fill_parse_error ("Invalid local memory startup (%le) for node %d\n",
      f->value.real, no_number - 1);
      return;
    }
    local_memory_startup = f->value.real * 1e6;
  }
  else
    local_memory_startup = 0.0;

  f = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);
  if (el!=(struct t_element *)0)
  {
    if (f->tipo != el->type)
      tipos_incompatibles (f->tipo, el->type);
    if (f->value.real < 0)
    {
      near_line ();
      fill_parse_error ("Invalid remote memory startup (%le) for node %d\n",
      f->value.real, no_number - 1);
      return;
    }
    remote_memory_startup = f->value.real * 1e6;
  }
  else
    remote_memory_startup = 0.0;

  if (fill_node (no_number, node_name, no_processors, no_input,
			no_output, local_startup, remote_startup,
			relative, local_bandwith, external_net_startup,
			local_port_startup, remote_port_startup,
			local_memory_startup, remote_memory_startup))
    return;
}




void new_record_2c_DAMIEN_format(struct t_queue *q, struct t_entry *en)
{
  struct t_field   *f;
  struct t_element *el;
  struct t_machine *machine;
  int               ma_number;
  int               no_number;
  char             *node_name;
  int               no_processors;
  int               no_input,
                    no_output;
  double            local_startup,
                    remote_startup;
  double            relative;
  double            local_bandwith;
  double            external_net_startup;
  double            local_port_startup,
                    remote_port_startup;
  double            local_memory_startup,
                    remote_memory_startup;


	f = (struct t_field *) head_queue (q);
  el = (struct t_element *) head_queue (en->types);
  if (f->tipo != el->type)
    tipos_incompatibles (f->tipo, el->type);
  ma_number = f->value.dec + 1;
  
  /* FEC: Com que utilitza el format nou, s'ha d'obtenir la maquina amb
          l'identificador donat. I se suposa que ja haura estat creada
          al llegir el "wide area network information", que es on diu
          el numero de maquines. */
  machine=(struct t_machine *)query_prio_queue(&Machine_queue, (t_priority)ma_number);
  if (machine==MA_NIL) panic("Machine not found!\n");

	f = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);
  if (f->tipo != el->type)
    tipos_incompatibles (f->tipo, el->type);
  no_number = f->value.dec + 1;

  f = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);
  if (f->tipo != el->type)
    tipos_incompatibles (f->tipo, el->type);
  node_name = (char *) mallocame (strlen (f->value.string) + 1);
  strcpy (node_name, f->value.string);

  f = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);
  if (f->tipo != el->type)
    tipos_incompatibles (f->tipo, el->type);
  if (f->value.dec <= 0)
  {
    near_line ();
    fill_parse_error ("Invalid number of processors (%d) for node %d\n",
                      f->value.dec, no_number - 1);
    return;
  }
  no_processors = f->value.dec;

  f = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);
  if (f->tipo != el->type)
    tipos_incompatibles (f->tipo, el->type);
  if (f->value.dec < 0)
  {
    near_line ();
    fill_parse_error ("Invalid number of input links (%d) for node %d\n",
                      f->value.dec, no_number - 1);
    return;
  }
  no_input = f->value.dec;

  f = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);
  if (f->tipo != el->type)
    tipos_incompatibles (f->tipo, el->type);
  if (f->value.dec < 0)
  {
    near_line ();
    fill_parse_error ("Invalid number of output links (%d) for node %d\n",
                      f->value.dec, no_number - 1);
    return;
  }
  no_output = f->value.dec;
         
  if ((no_input==0) && (no_output==0))
  {
    near_line ();
    fill_parse_error ("Invalid number of half duplex  links (%d) for node %d\n",
			                f->value.dec, no_number - 1);
    return;
  }

  f = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);
  if (f->tipo != el->type)
    tipos_incompatibles (f->tipo, el->type);
  if (f->value.real < 0)
  {
    near_line ();
    fill_parse_error ("Invalid startup on local communication (%le) for node %d\n",
		          	      f->value.real, no_number - 1);
    return;
  }
  local_startup = f->value.real * 1e6;

  f = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);
  if (f->tipo != el->type)
    tipos_incompatibles (f->tipo, el->type);
  if (f->value.real < 0)
  {
    near_line ();
    fill_parse_error ("Invalid startup on remote communication (%le) for node %d\n",
                      f->value.real, no_number - 1);
    return;
  }
  remote_startup = f->value.real * 1e6;

  f = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);
  if (f->tipo != el->type)
    tipos_incompatibles (f->tipo, el->type);
  if (f->value.real <= 0)
  {
    near_line ();
    fill_parse_error ("Invalid relative processor speed (%le) for node %d\n",
                      f->value.real, no_number - 1);
    return;
  }
  relative = f->value.real;

  f = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);
  if (f->tipo != el->type)
    tipos_incompatibles (f->tipo, el->type);
  if (f->value.real < 0)
  {
    near_line ();
    fill_parse_error ("Invalid memory bandwidth (%le) for node %d\n",
                      f->value.real, no_number - 1);
    return;
  }
  local_bandwith = f->value.real;


  f = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);
  if (f->tipo != el->type)
    tipos_incompatibles (f->tipo, el->type);
  if (f->value.real < 0)
  {
    near_line ();
    fill_parse_error ("Invalid external net startup (%le) for node %d\n",
                      f->value.real, no_number - 1);
    return;
  }
  external_net_startup = f->value.real * 1e6;


  f = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);
  if (el!=(struct t_element *)0)
  {
    if (f->tipo != el->type)
      tipos_incompatibles (f->tipo, el->type);
    if (f->value.real < 0)
    {
      near_line ();
      fill_parse_error ("Invalid local port startup (%le) for node %d\n",
                        f->value.real, no_number - 1);
      return;
    }
    local_port_startup = f->value.real * 1e6;
  }
  else
    local_port_startup = 0.0;

  f = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);
  if (el!=(struct t_element *)0)
  {
    if (f->tipo != el->type)
      tipos_incompatibles (f->tipo, el->type);
    if (f->value.real < 0)
    {
      near_line ();
      fill_parse_error ("Invalid remote port startup (%le) for node %d\n",
                        f->value.real, no_number - 1);
      return;
    }
    remote_port_startup = f->value.real * 1e6;
  }
  else
    remote_port_startup = 0.0;

  f = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);
  if (el!=(struct t_element *)0)
  {
    if (f->tipo != el->type)
      tipos_incompatibles (f->tipo, el->type);
    if (f->value.real < 0)
    {
      near_line ();
      fill_parse_error ("Invalid local memory startup (%le) for node %d\n",
      f->value.real, no_number - 1);
      return;
    }
    local_memory_startup = f->value.real * 1e6;
  }
  else
    local_memory_startup = 0.0;

  f = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);
  if (el!=(struct t_element *)0)
  {
    if (f->tipo != el->type)
      tipos_incompatibles (f->tipo, el->type);
    if (f->value.real < 0)
    {
      near_line ();
      fill_parse_error ("Invalid remote memory startup (%le) for node %d\n",
      f->value.real, no_number - 1);
      return;
    }
    remote_memory_startup = f->value.real * 1e6;
  }
  else
    remote_memory_startup = 0.0;

  if (fill_node (no_number, node_name, no_processors, no_input,
			no_output, local_startup, remote_startup,
			relative, local_bandwith, external_net_startup,
			local_port_startup, remote_port_startup,
			local_memory_startup, remote_memory_startup))
    return;
}




void new_record_7c_DAMIEN_format(struct t_queue *q, struct t_entry *en)
{
  struct t_field   *f, *f2;
  struct t_element *el;
  int i, d_con_id;
  struct t_dedicated_connection *d_con;
  int error=0;
  struct t_machine *s_machine, *d_machine;
  
  f = (struct t_field *) head_queue (q);
	el = (struct t_element *) head_queue (en->types);
	if (f->tipo != el->type)
	  tipos_incompatibles (f->tipo, el->type);
  if (f->value.dec < 0)
  {
	  near_line ();
	  fill_parse_error ("Invalid connection id: %d \n",
		                  f->value.dec);
	  return;
	}  
  d_con_id=f->value.dec+1;
  
  /* Es busca la connexio dedicada, que ja hauria s'estar creada */
  d_con=(struct t_dedicated_connection *)query_prio_queue(&Dedicated_Connections_queue, (t_priority) d_con_id);
  if (d_con==DC_NIL) panic("Dedicated connection not found!\n");

  f = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);
  if (f->tipo != el->type)
    tipos_incompatibles (f->tipo, el->type);
  if (f->value.dec < 0)
  {
    near_line ();
    fill_parse_error ("Invalid number of source machine identifier: %d for connection %d\n",
                      f->value.dec, d_con_id - 1);
    return;
  }
  d_con->source_id = f->value.dec + 1;
 
  f = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);
  if (f->tipo != el->type)
    tipos_incompatibles (f->tipo, el->type);
  if (f->value.dec < 0)
  {
    near_line ();
    fill_parse_error ("Invalid number of destination machine identifier: %d for connection %d\n",
                      f->value.dec, d_con_id - 1);
    return;
  }
  d_con->destination_id = f->value.dec + 1;
 
  f = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);
  if (f->tipo != el->type)
    tipos_incompatibles (f->tipo, el->type);
  if (f->value.real < 0)
  {
    near_line ();
    fill_parse_error ("Invalid dedicated connection bandwidth (%le) for connection %d\n",
                      f->value.real, d_con_id - 1);
    return;
  }
  d_con->bandwidth = f->value.real;

  /* Cal llegir els TAGS */
  f = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);
  if (count_queue (f->value.arr.q) != f->value.arr.dim1)
  {
    near_line ();
    fill_parse_error ("Incorrect dedicated connection number of tags for connection %d\n",
                      d_con_id - 1);
    return;
  }
  d_con->number_of_tags=f->value.arr.dim1;
  /* Es reserva memoria pels tags */
  d_con->tags=(int *) mallocame(d_con->number_of_tags * sizeof(int));
  /* Es llegeixen els tags */
  for (i=0, f2 = (struct t_field *) head_queue (f->value.arr.q);
       i<d_con->number_of_tags;
       i++, f2 = (struct t_field *) next_queue (f->value.arr.q))
  {
    d_con->tags[i]=f2->value.dec;
  }
    
  f = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);
  if (f->tipo != el->type)
    tipos_incompatibles (f->tipo, el->type);
  if (f->value.dec < 0)
  {
    near_line ();
    fill_parse_error ("Invalid first message size: %d for connection %d\n",
                      f->value.dec, d_con_id - 1);
    return;
  }
  d_con->first_message_size = f->value.dec;
  
  f = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);
  if (f->tipo != el->type)
    tipos_incompatibles (f->tipo, el->type);
  if (strlen(f->value.string)!=1) error=1;
  else if (f->value.string[0]=='<') d_con->first_size_condition = 0;
  else if (f->value.string[0]=='=') d_con->first_size_condition = 1;
  else if (f->value.string[0]=='>') d_con->first_size_condition = 2;
  else error=1;
  if (error)
  {
    near_line ();
    fill_parse_error ("Invalid dedicated connection first condition: \"%s\" for connection %d\n",
                      f->value.string, d_con_id - 1);
    return;
  }

  f = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);
  if (f->tipo != el->type)
    tipos_incompatibles (f->tipo, el->type);
  if (strlen(f->value.string)!=1) error=1;
  else if (f->value.string[0]=='&') d_con->operation = 0;
  else if (f->value.string[0]=='|') d_con->operation = 1;
  else error=1;
  if (error)
  {
    near_line ();
    fill_parse_error ("Invalid dedicated connection operation: \"%s\" for connection %d\n",
                      f->value.string, d_con_id - 1);
    return;
  }
    
  f = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);
  if (f->tipo != el->type)
    tipos_incompatibles (f->tipo, el->type);
  if (f->value.dec < 0)
  {
    near_line ();
    fill_parse_error ("Invalid second message size: %d for connection %d\n",
                      f->value.dec, d_con_id - 1);
    return;
  }
  d_con->second_message_size = f->value.dec;
 
  f = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);
  if (f->tipo != el->type)
    tipos_incompatibles (f->tipo, el->type);
  if (strlen(f->value.string)!=1) error=1;
  else if (f->value.string[0]=='<') d_con->second_size_condition = 0;
  else if (f->value.string[0]=='=') d_con->second_size_condition = 1;
  else if (f->value.string[0]=='>') d_con->second_size_condition = 2;
  else error=1;
  if (error)
  {
    near_line ();
    fill_parse_error ("Invalid dedicated connection second condition: \"%s\" for connection %d\n",
                      f->value.string, d_con_id - 1);
    return;
  }
 
  /* Cal llegir els Comunicadors */
  f = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);
  if (count_queue (f->value.arr.q) != f->value.arr.dim1)
  {
    near_line ();
    fill_parse_error ("Incorrect dedicated connection number of communicators for connection %d\n",
                      d_con_id - 1);
    return;
  }
  d_con->number_of_communicators=f->value.arr.dim1;
  /* Es reserva memoria pels comunicadors */
  d_con->communicators=(int *) mallocame(d_con->number_of_communicators * sizeof(int));
  /* Es llegeixen els tags */
  for (i=0, f2 = (struct t_field *) head_queue (f->value.arr.q);
       i<d_con->number_of_communicators;
       i++, f2 = (struct t_field *) next_queue (f->value.arr.q))
  {
    d_con->communicators[i]=f2->value.dec;
  }

  f = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);
  if (f->tipo != el->type)
    tipos_incompatibles (f->tipo, el->type);
  if (f->value.real < 0)
  {
    near_line ();
    fill_parse_error ("Invalid dedicated connection startup (%le) for connection %d\n",
                      f->value.real, d_con_id - 1);
    return;
  }
  d_con->startup = f->value.real * 1e6;


  f = (struct t_field *) next_queue (q);
  el = (struct t_element *) next_queue (en->types);
  if (f->tipo != el->type)
    tipos_incompatibles (f->tipo, el->type);
  if (f->value.real < 0)
  {
    near_line ();
    fill_parse_error ("Invalid dedicated connection flight_time (%le) for connection %d\n",
                      f->value.real, d_con_id - 1);
    return;
  }
  d_con->flight_time = f->value.real * 1e6;

  
  /* Cal buscar les dues maquines (origen i desti) per afegir la connexio a la
     cua de connexions de cada maquina. Aixo permetra trobar la connexio que
     cal utilitzar molt mes rapidament que haver de recorrer la cua de totes
     les connexions. */
  s_machine=(struct t_machine *)query_prio_queue(&Machine_queue, (t_priority)d_con->source_id);
  if (s_machine==MA_NIL) panic("Source connection machine not found!\n");
  d_machine=(struct t_machine *)query_prio_queue(&Machine_queue, (t_priority)d_con->destination_id);
  if (d_machine==MA_NIL) panic("Destination connection machine not found!\n");
  insert_queue (&(s_machine->dedicated_connections.connections),
                (char *) d_con, (t_priority) (d_con->id));
  insert_queue (&(d_machine->dedicated_connections.connections),
                (char *) d_con, (t_priority) (d_con->id));
}



void
check_fields_for_configuration(struct t_queue *q, struct t_entry *en)
{
  struct t_element *el;
  struct t_field   *f, *f2, *f3, *f4;
  char             *node_name;
  int               no_processors;

  int    module;
  double ratio;
  double disk_latency, disk_bandwidth, block_size, hit_ratio;
  int    concurrent_requests;

#ifdef LIBLEXYACC
  char *fichero_sch, *fichero_comm, *fichero_fs, *fichero_random;   
#endif /* LIBLEXYACC */
  struct t_Ptask *Ptask;
  int             j;


  switch (en->entryid)
  {
    case 0:
      if (count_queue (q) != count_queue (en->types))
      {
        near_line ();
        fill_parse_error ("Insufficient fields on register #0: %s\n", en->name);
        return;
      }

      new_record_0c_DAMIEN_format(q,en);
      break;

    case 1: /* environment information */
      if (count_queue (q) != count_queue (en->types))
      {
        near_line ();
        fill_parse_error ("Insufficient fields on register #1: %s\n", en->name);
        return;
      }

      if (count_queue (q) == 5)
        new_record_1c_old_format(q,en);
      else if (count_queue (q) == 7)
        new_record_1c_DAMIEN_format(q,en);
      break;

    case 2:
      if (count_queue (q) != count_queue (en->types))
      {
        near_line ();
        fill_parse_error ("Insufficient fields on register #2: %s\n", en->name);
        return;
      }

      if ((count_queue (q)==9) || (count_queue (q)==13))
        new_record_2c_old_format(q,en);
      else if ((count_queue (q)==11) || (count_queue (q)==15))
        new_record_2c_DAMIEN_format(q,en);
      break;

    case 3:
      if (count_queue (q) != count_queue (en->types))
      {
        near_line ();
        fill_parse_error ("Insufficient fields on register #3: %s\n", en->name);
        return;
      }

      f  = (struct t_field *) head_queue (q);
      el = (struct t_element *) head_queue (en->types);

      if (f->tipo != el->type)
        tipos_incompatibles (f->tipo, el->type);

      node_name = (char *) mallocame (strlen (f->value.string) + 1);
      strcpy (node_name, f->value.string);
      
      f  = (struct t_field *) next_queue (q);
      el = (struct t_element *) next_queue (en->types);

      if (f->tipo != el->type)
        tipos_incompatibles (f->tipo, el->type);

      if (f->value.dec <= 0)
      {
        near_line ();
        fill_parse_error ("Invalid number of tasks (%le) in application %s\n",
                          f->value.dec,
                          node_name);
        return;
      }
      no_processors = f->value.dec;

#ifndef LIBLEXYACC
      Ptask_current = Ptask = create_Ptask (node_name, (char *) 0);
#else   /* LIBLEXYACC */
      new_application (node_name, no_processors);
#endif  /* LIBLEXYACC */

      f  = (struct t_field *) next_queue (q);
      el = (struct t_element *) next_queue (en->types);
      el = (struct t_element *) next_queue (en->types);
      
      if (el!=(struct t_element *)0)
      {
        f3 = (struct t_field *) next_queue (q);
        f4 = (struct t_field *) head_queue (f3->value.arr.q);
      }

      if ((no_processors != count_queue (f->value.arr.q)) ||
         (no_processors != f->value.arr.dim1))
      {
        near_line ();
        fill_parse_error (
          "Number of tasks (%d) greater than mapping info (%d)\n",
          no_processors,
          count_queue (f->value.arr.q));
        return;
      }

      for (j = 0, f2 = (struct t_field *) head_queue (f->value.arr.q);
           j < no_processors;
           j++, f2 = (struct t_field *) next_queue (f->value.arr.q))
      {

#ifndef LIBLEXYACC

        if (el==(struct t_element *)0)
         new_thread_in_Ptask (Ptask, j + 1, f2->value.dec + 1, 1, 0, 0);
        else
        {
          new_thread_in_Ptask (Ptask,
                               j + 1,
                               f2->value.dec + 1,
                               1,
                               f4->value.dec,
                               0);

          f4 = (struct t_field *) next_queue (f3->value.arr.q);
        }

#else  /* LIBLEXYACC */
        map_task_to_node (j + 1, f2->value.dec + 1);
#endif /* LIBLEXYACC */
      }

#ifndef LIBLEXYACC
      inFIFO_queue (&Ptask_queue, (char *) Ptask);
#endif  /* LIBLEXYACC */

      break;

    case 4:
      if (count_queue (q) != count_queue (en->types))
      {
        near_line ();
        fill_parse_error ("Insufficient fields on register #4: %s\n", en->name);
      }

      f  = (struct t_field *) head_queue (q);
      el = (struct t_element *) head_queue (en->types);

      if (f->tipo != el->type)
        tipos_incompatibles (f->tipo, el->type);

      fichero_sch = (char *) mallocame (strlen (f->value.string) + 1);
      strcpy (fichero_sch, f->value.string);

      f  = (struct t_field *) next_queue (q);
      el = (struct t_element *) head_queue (en->types);

      if (f->tipo != el->type)
        tipos_incompatibles (f->tipo, el->type);

      fichero_fs = (char *) mallocame (strlen (f->value.string) + 1);
      strcpy (fichero_fs, f->value.string);
      
      f  = (struct t_field *) next_queue (q);
      el = (struct t_element *) head_queue (en->types);

      if (f->tipo != el->type)
        tipos_incompatibles (f->tipo, el->type);

      fichero_comm = (char *) mallocame (strlen (f->value.string) + 1);
      strcpy (fichero_comm, f->value.string);

      f  = (struct t_field *) next_queue (q);
      el = (struct t_element *) head_queue (en->types);

      if (f->tipo != el->type)
        tipos_incompatibles (f->tipo, el->type);

      fichero_random = (char *) mallocame (strlen (f->value.string) + 1);
      strcpy (fichero_random, f->value.string);

#ifdef LIBLEXYACC
      configuration_files(fichero_sch,
                          fichero_fs,
                          fichero_comm,
                          fichero_random);
#endif /* LIBLEXYACC */

      break;
    case 5:
      if (count_queue (q) != count_queue (en->types))
      {
        near_line ();
        fill_parse_error ("Insufficient fields on register #5: %s\n", en->name);
        return;
      }

      f  = (struct t_field *) head_queue (q);
      el = (struct t_element *) head_queue (en->types);

      if (f->tipo != el->type)
        tipos_incompatibles (f->tipo, el->type);

      if (f->value.dec < 0)
      {
        near_line ();
        fill_parse_error ("Invalid module number: %d \n", f->value.dec);
        return;
      }

      module = f->value.dec;

      f  = (struct t_field *) next_queue (q);
      el = (struct t_element *) next_queue (en->types);

      if (f->tipo != el->type)
        tipos_incompatibles (f->tipo, el->type);
      
      if (f->value.real < 0)
      {
        near_line ();
        fill_parse_error ("Invalid ratio for module %d: %le\n", 
                          f->value.real,
                          module);
        return;
      }

      ratio = f->value.real;
#ifdef LIBLEXYACC
      module_new ((struct t_Ptask *)0, module, ratio);
#else
      module_new (Ptask_current, module, ratio);
#endif
      break;

    case 6:
      if (count_queue (q) != count_queue (en->types))
      {
        near_line ();
        fill_parse_error ("Insufficient fields on register #6: %s\n",
                          en->name);
        return;
      }

      f  = (struct t_field *) head_queue (q);
      el = (struct t_element *) head_queue (en->types);

      if (f->tipo != el->type)
        tipos_incompatibles (f->tipo, el->type);

      if (f->value.real < 0)
      {
        near_line ();
        fill_parse_error ("Invalid disk latency number: %le \n", f->value.real);
        return;
      }

      disk_latency = f->value.real;

      f  = (struct t_field *) next_queue (q);
      el = (struct t_element *) next_queue (en->types);

      if (f->tipo != el->type)
        tipos_incompatibles (f->tipo, el->type);

      if (f->value.real < 0)
      {
        near_line ();
        fill_parse_error ("Invalid disk bandwidth: %le\n", f->value.real);
        return;
      }

      disk_bandwidth = f->value.real;

      f  = (struct t_field *) next_queue (q);
      el = (struct t_element *) next_queue (en->types);

      if (f->tipo != el->type)
        tipos_incompatibles (f->tipo, el->type);

      if (f->value.real <= 0)
      {
        near_line ();
        fill_parse_error ("Invalid block_size: %le\n", f->value.real);
        return;
      }

      block_size = f->value.real;

      f  = (struct t_field *) next_queue (q);
      el = (struct t_element *) next_queue (en->types);

      if (f->tipo != el->type)
        tipos_incompatibles (f->tipo, el->type);

      if (f->value.dec < 0)
      {
        near_line ();
        fill_parse_error ("Invalid value for concurrent requests: %le\n",
                          f->value.dec);
        return;
      }

      concurrent_requests = f->value.dec;

      f  = (struct t_field *) next_queue (q);
      el = (struct t_element *) next_queue (en->types);

      if (f->tipo != el->type)
        tipos_incompatibles (f->tipo, el->type);

      if ((f->value.real < 0.0) || (f->value.real > 1.0 ))
      {
        near_line ();
        fill_parse_error ("Invalid hit ratio: %le\n",f->value.real);
        return;
      }

      hit_ratio = f->value.real;

      FS_parameters (disk_latency,
                     disk_bandwidth,
                     block_size,
                     concurrent_requests,
                     hit_ratio);
      break;

    case 7:
      if (count_queue (q) != count_queue (en->types))
      {
        near_line ();
        fill_parse_error ("Insufficient fields on register #7: %s\n", en->name);
        return;
      }
      new_record_7c_DAMIEN_format(q,en);
      break;

    default:
      fill_parse_error ("Undefined register number %d\n", en->entryid);
      near_line ();
      break;
  }

  for (f  = (struct t_field *) outFIFO_queue (q);
       f != (struct t_field *) 0;
       f  = (struct t_field *) outFIFO_queue (q))
  {
    if (f->tipo == 3)
    {
      for (f2  = (struct t_field *) outFIFO_queue (f->value.arr.q);
           f2 != (struct t_field *) 0;
           f2  = (struct t_field *) outFIFO_queue (f->value.arr.q))
      {
        freeame ((char *) f2, sizeof (struct t_field));
      }
      freeame ((char *) f->value.arr.q, sizeof (struct t_queue));
    }
    freeame ((char *) f, sizeof (struct t_field));
  }
  freeame ((char *) q, sizeof (struct t_queue));
}


#ifdef LIBLEXYACC
static struct t_queue Register_queue;
extern struct t_queue *Registers;


int 
get_max_tasks_in_tracefile (char *filename)
{
   struct t_disk_action da;
   FILE           *fp;
   char            b[12];
   int             max_task = 0;
   struct stat buf;

   stat (filename, &buf);

   fp = MYFREOPEN (filename, "r", stdin);
   if (fp == (FILE *) 0)
      return (0);
   fscanf (fp, "%5s", b);
   if (strcmp ("SDDFA", b) == 0)
   {
      rewind (fp);
      current_file = filename;
      configuration_file = FALSE;
      load_interactive = FALSE;
      dimemas_GUI = TRUE;
      Registers = &Register_queue;
      create_queue (Registers);

      create_wait_window ("Getting number of tasks");
      aprox_number_of_records = buf.st_size/30;
      one_more_record = 0;
      last_value_load_in_curse = 0;
#ifndef AIXV3
      yyrestart (fp);
#endif
      if (yyparse () != 0)
	 max_task_id = -1;
      destroy_wait_window();
      configuration_file = TRUE;
      max_task = max_task_id;
   }
   else
   {
      if (strcmp("SDDFB",b) !=0)
	return(-1);
      while (fread (&da, sizeof (struct t_disk_action), 1, fp) == 1)
	 max_task = MAX (max_task, da.taskid);
   }
   return (max_task);
}

void
complete_list_of_modules(char *filename)
{
   struct t_disk_action da;
   FILE           *fp;
   char            b[12];
   struct stat buf;

   stat (filename, &buf);
   fp = MYFREOPEN (filename, "r", stdin);
   if (fp == (FILE *) 0)
      return ;
   fscanf (fp, "%5s", b);
   if (strcmp ("SDDFA", b) == 0)
   {
      rewind (fp);
      current_file = filename;
      configuration_file = FALSE;
      load_interactive = FALSE;
      dimemas_GUI = TRUE;
      Registers = &Register_queue;
      create_queue (Registers);

#ifndef AIXV3
      yyrestart (fp);
#endif
      create_wait_window ("Getting modules identifiers");
      aprox_number_of_records = buf.st_size/30;
      one_more_record = 0;
      last_value_load_in_curse = 0;
      yyparse ();
      destroy_wait_window();
      configuration_file = TRUE;
   }
   else
   {
      if (strcmp("SDDFB",b) !=0)
	return;
      while (fread (&da, sizeof (struct t_disk_action), 1, fp) == 1)
	 if ((da.action.action==EVEN) && 
	     ((da.action.desc.even.type==BLOCK_BEGIN) ||
	      (da.action.desc.even.type==BLOCK_END)))
	 {
	   new_block_identifier(da.action.desc.even.value);
	 }
   }

}
#endif				/* LIBLEXYACC */
