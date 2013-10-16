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

%{

#include "define.h"
#include "types.h"

#include "extern.h"
#include "list.h"
#include "modules_map.h"
#include "subr.h"

#include "ts.h"
#include "check.h"

#define YYSTYPE long
#define YYMAXDEPTH 30000

  static void      yyerror(char *);

  int              yydebug;
  int              line_no;
  int              max_task_located;
  t_boolean        yy_error_filled = FALSE;
  char             yy_error_string[1024];
  t_boolean        yy_near_line_filled = FALSE;
  char             yy_near_line[80];

  extern struct t_queue  Registers_queue;
  extern struct Str      *first_str;

  // extern char *types[] = {"integer", "double", "char", "array"};

  extern t_boolean definition_error;
  int              max_task_id;

%}

%token SDDFA DECIMAL STRING NUMTYPE REAL IDENTIFICADOR SEPARADOR
%token OPEN_K CLOSE_K PUNTO_I_COMA COMA ARRAY OPEN_B CLOSE_B

%start input

%%
input:
{
  line_no          = 1;
  first_str        = (struct Str *) 0;
  max_task_id      = 0;
  definition_error = FALSE;
}

SDDFA SEPARADOR declaraciones
{
  if (!all_definitions_exist())
  {
    return (ERROR);
  }
}
registros
{
  return (free_memory());
}

| error
{
  yyerrok;
  yyclearin;
  return (ERROR);
};

declaraciones: declaracion | declaraciones declaracion;

declaracion: IDENTIFICADOR STRING OPEN_K componentes CLOSE_K SEPARADOR
{
  struct t_entry* new_entry;

  /* $1: Record identifier
     $2: Record name
     $3: Record components */

  if (add_register_definition((int) $1, (char *) $2, (struct t_queue *) $4))
  {
    near_line();
    warning("Duplicated identifier #%d: %s\n", $1, (char *) $2);
    definition_error = TRUE;
    $$ = (YYSTYPE) 0;
  }
  else
  {
    new_entry = (struct t_entry *) malloc(sizeof(struct t_entry));

    new_entry->entryid = $1;
    new_entry->name    = (char *) $2;
    new_entry->types   = (struct t_queue *) $4;
    inFIFO_queue(&Registers_queue, (char *) new_entry);

    $$ = (YYSTYPE) new_entry;
  }
}
| SEPARADOR;

componentes: tipo componentes
{
  inLIFO_queue((struct t_queue *) $2, (char *) $1);
  $$ = $2;
}

| tipo
{
  struct t_queue* types;

  types = (struct t_queue *) malloc(sizeof(struct t_queue));
  create_queue(types);
  inFIFO_queue(types, (char *) $1);
  $$ = (YYSTYPE) types;
};

tipo: NUMTYPE string_a PUNTO_I_COMA
{
  struct t_element* elem;

  elem = (struct t_element *) malloc(sizeof(struct t_element));

  elem->i3   = (struct t_array *) $2;
  elem->type = $1;

  $$ = (YYSTYPE) elem;
}

| error
{
  yyerrok;
  yyclearin;
};

string_a: STRING ARRAY ARRAY
{
  struct t_array* a;

  a = (struct t_array *) malloc(sizeof(struct t_array));

  a->i1        = (char *) $1;
  a->dimension = 2;

  $$ = (YYSTYPE) a;
}

| STRING ARRAY
{
  struct t_array* a;

  a = (struct t_array *) malloc(sizeof(struct t_array));

  a->i1        = (char *) $1;
  a->dimension = 1;

  $$ = (YYSTYPE) a;
}

| STRING
{
  struct t_array* a;

  a = (struct t_array *) malloc(sizeof(struct t_array));

  a->i1        = (char *) $1;
  a->dimension = 0;

  $$ = (YYSTYPE) a;
};

/*****************************************************************************
 * INFORMATION REGISTERS
 ****************************************************************************/

registros: registro | registros registro;

registro: STRING OPEN_K campos CLOSE_K SEPARADOR
{
  struct t_entry *en;

  en = exist_identifier((char*) $1, &Registers_queue);

  if (en != NULL)
  {
    CONFIGURATION_New_Definition((struct t_queue*) $3, en);
  }
  else
  {
    die("Unknown register name: %s\n", (char *) $1);
    near_line();
    definition_error = TRUE;
  }
}

| error SEPARADOR
{
  yyerrok;
  yyclearin;
  return (ERROR);
};

campos: campo COMA campos
{
  inLIFO_queue((struct t_queue *) $3, (char *) $1);
  $$ = $3;
}

| campo
{
  struct t_queue *campos;

  campos = (struct t_queue *) malloc(sizeof(struct t_queue));
  create_queue(campos);
  inFIFO_queue(campos, (char *) $1);
  $$ = (YYSTYPE) campos;
};

campo:DECIMAL
{
  struct t_field *f;

  f = (struct t_field *) malloc(sizeof(struct t_field));
  f->tipo = 0;
  f->value.dec = $$;
  $$ = (YYSTYPE) f;
}

|REAL
{
  struct t_field *f;

  f = (struct t_field *) malloc(sizeof(struct t_field));
  f->tipo = 1;
  f->value.real = * (double*) $$;
  free((void*) $$);
  $$ = (YYSTYPE) f;
}

|STRING
{
  struct t_field *f;

  f = (struct t_field *) malloc(sizeof(struct t_field));
  f->tipo = 2;
  f->value.string = (char *) $$;
  $$ = (YYSTYPE) f;
}

|OPEN_B DECIMAL CLOSE_B OPEN_B DECIMAL CLOSE_B OPEN_K campos CLOSE_K
{
  struct t_field *f;

  f = (struct t_field *) malloc(sizeof(struct t_field));

  f->tipo           = 3;
  f->value.arr.q    = (struct t_queue *) $8;
  f->value.arr.dim1 = $2;
  f->value.arr.dim2 = $5;

  $$ = (YYSTYPE) f;
}

| OPEN_B DECIMAL CLOSE_B OPEN_K campos CLOSE_K
{
  struct t_field* f;

  f = (struct t_field *) malloc(sizeof(struct t_field));

  f->tipo           = 3;
  f->value.arr.q    = (struct t_queue *) $5;
  f->value.arr.dim1 = $2;
  f->value.arr.dim2 = 0;

  $$ = (YYSTYPE) f;
};

%%

static void
yyerror(char* s)
{
  die ("Error: %s\n", s);
  near_line();
  definition_error = TRUE;
}
