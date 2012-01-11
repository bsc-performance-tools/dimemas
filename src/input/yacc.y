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
#include "mallocame.h"
#include "subr.h"

#include "ts.h"
#include "check.h"

#define YYSTYPE long
#define YYMAXDEPTH 30000

  static void     yyerror(char *);
  static int      free_memory(void);
  static int      check_all_definitions(void);
  static t_boolean exist_identifier_or_string(int, char *, struct t_queue *);
  static struct t_entry* exist_identifier(char *, struct t_queue *);

  int              yydebug;
  int              line_no;
  int              max_task_located;
  t_boolean        yy_error_filled = FALSE;
  char             yy_error_string[1024];
  t_boolean        yy_near_line_filled = FALSE;
  char             yy_near_line[80];
  struct t_queue  *Registers = (struct t_queue *) 0;
  extern t_boolean definition_error;
  extern t_boolean configuration_file;
  extern t_boolean dimemas_GUI;
  extern char     *current_file;
  int              max_task_id;
  struct Str      *first_str;

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
  if (check_all_definitions())
    return (ERROR);
  if ((load_interactive) && (configuration_file == FALSE))
    return (NO_ERROR);
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

  if (exist_identifier_or_string((int) $1, (char *) $2, (struct t_queue *) $4))
  {
    fill_parse_error("Duplicated identifier #%d: %s\n", $1, (char *) $2);
    near_line();
    definition_error = TRUE;
    $$ = (YYSTYPE) 0;
  }
  else
  {
    new_entry = (struct t_entry *) mallocame(sizeof(struct t_entry));

    new_entry->entryid = $1;
    new_entry->name    = (char *) $2;
    new_entry->types   = (struct t_queue *) $4;
    inFIFO_queue(Registers, (char *) new_entry);

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

  types = (struct t_queue *) mallocame(sizeof(struct t_queue));
  create_queue(types);
  inFIFO_queue(types, (char *) $1);
  $$ = (YYSTYPE) types;
};

tipo: NUMTYPE string_a PUNTO_I_COMA
{
  struct t_element* elem;

  elem = (struct t_element *) mallocame(sizeof(struct t_element));

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

  a = (struct t_array *) mallocame(sizeof(struct t_array));

  a->i1        = (char *) $1;
  a->dimension = 2;

  $$ = (YYSTYPE) a;
}

| STRING ARRAY
{
  struct t_array* a;

  a = (struct t_array *) mallocame(sizeof(struct t_array));
  
  a->i1        = (char *) $1;
  a->dimension = 1;
  
  $$ = (YYSTYPE) a;
}

| STRING
{
  struct t_array* a;

  a = (struct t_array *) mallocame(sizeof(struct t_array));

  a->i1        = (char *) $1;
  a->dimension = 0;

  $$ = (YYSTYPE) a;
};

registros: registro | registros registro;

registro: STRING OPEN_K campos CLOSE_K SEPARADOR
{
  struct t_entry *en;

  en = exist_identifier((char *) $1, Registers);
  if (en != (struct t_entry *) 0)
  {
    if (configuration_file)
      check_fields_for_configuration((struct t_queue *) $3, en);
    else
      check_fields_with_structure((struct t_queue *) $3, en);
  } else
  {
    fill_parse_error("Unkown register name: %s\n", (char *) $1);
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

  campos = (struct t_queue *) mallocame(sizeof(struct t_queue));
  create_queue(campos);
  inFIFO_queue(campos, (char *) $1);
  $$ = (YYSTYPE) campos;
};

campo:DECIMAL
{
  struct t_field *f;

  f = (struct t_field *) mallocame(sizeof(struct t_field));
  f->tipo = 0;
  f->value.dec = $$;
  $$ = (YYSTYPE) f;
}

|REAL
{
  struct t_field *f;

  f = (struct t_field *) mallocame(sizeof(struct t_field));
  f->tipo = 1;
  f->value.real = * (double*) $$;
  freeame((char *) $$, sizeof(double));
  $$ = (YYSTYPE) f;
}

|STRING
{
  struct t_field *f;

  f = (struct t_field *) mallocame(sizeof(struct t_field));
  f->tipo = 2;
  f->value.string = (char *) $$;
  $$ = (YYSTYPE) f;
}

|OPEN_B DECIMAL CLOSE_B OPEN_B DECIMAL CLOSE_B OPEN_K campos CLOSE_K
{
  struct t_field *f;

  f = (struct t_field *) mallocame(sizeof(struct t_field));

  f->tipo           = 3;
  f->value.arr.q    = (struct t_queue *) $8;
  f->value.arr.dim1 = $2;
  f->value.arr.dim2 = $5;
  
  $$ = (YYSTYPE) f;
}

| OPEN_B DECIMAL CLOSE_B OPEN_K campos CLOSE_K
{
  struct t_field* f;

  f = (struct t_field *) mallocame(sizeof(struct t_field));
  
  f->tipo           = 3;
  f->value.arr.q    = (struct t_queue *) $5;
  f->value.arr.dim1 = $2;
  f->value.arr.dim2 = 0;
  
  $$ = (YYSTYPE) f;
};

%%

static struct t_entry* exist_identifier(char *c, struct t_queue* q)
{
  struct t_entry *entry;

  if (q == (struct t_queue *) 0)
    return (FALSE);

  for (entry = (struct t_entry *) head_queue(q);
       entry != (struct t_entry *) 0;
       entry = (struct t_entry *) next_queue(q))
  {
    if (entry->name == c)
      return (entry);
  }

  return ((struct t_entry *) 0);
}

static t_boolean exist_identifier_or_string(int i, char *c, struct t_queue* q)
{
  struct t_entry *entry;

  if (configuration_file)
  {
    switch (i)
    {
    case 0:
      check_record_0c(c, q);
      break;
    case 1:
      check_record_1c(c, q);
      break;
    case 2:
      check_record_2c(c, q);
      break;
    case 3:
      check_record_3c(c, q);
      break;
    case 4:
      check_record_4c(c, q);
      break;
    case 5:
      check_record_5c(c, q);
      break;
    case 6:
      check_record_6c(c, q);
      break;
    case 7:
      check_record_7c(c, q);
      break;
    default:
      fill_parse_error("Invalid record number %d: %s\n", i, c);
      near_line();
      definition_error = TRUE;
      return (FALSE);
    }
  } else
  {
    switch (i)
    {
    case 1:
      check_record_1(c, q);
      break;
    case 2:
      check_record_2(c, q);
      break;
    case 3:
      check_record_3(c, q);
      break;
    case 40:
      check_record_40(c, q);
      break;
    case 41:
      check_record_41(c, q);
      break;
    case 42:
      check_record_42(c, q);
      break;
    case 43:
      check_record_43(c, q);
      break;
    case 48:
      check_record_48(c, q);
      break;
    case 49:
      check_record_49(c, q);
      break;
    case 50:
      check_record_50(c, q);
      break;
    case 101:
      check_record_101(c, q);
      break;
    case 102:
      check_record_102(c, q);
      break;
    case 103:
      check_record_103(c, q);
      break;
    case 104:
      check_record_104(c, q);
      break;
    case 105:
      check_record_105(c, q);
      break;
    case 106:
      check_record_106(c, q);
      break;
    case 107:
      check_record_107(c, q);
      break;
    case 100:
      check_record_100(c, q);
      break;
    case 200:
      check_record_200(c, q);
      break;
    case 201:
      check_record_201(c, q);
      break;
    case 300:
      check_record_300(c, q);
      break;
    case 301:
      check_record_301(c, q);
      break;
    case 302:
      check_record_302(c, q);
      break;
    case 303:
      check_record_303(c, q);
      break;
    case 304:
      check_record_304(c, q);
      break;
    case 305:
      check_record_305(c, q);
      break;
    case 306:
      check_record_306(c, q);
      break;
    case 307:
      check_record_307(c, q);
      break;
    case 400:
      check_record_400(c, q);
      break;
    case 401:
      check_record_401(c, q);
      break;
    case 402:
      check_record_402(c, q);
      break;
    case 403:
      check_record_403(c, q);
      break;
    case 404:
      check_record_404(c, q);
      break;
    default:
      fill_parse_error("Invalid record number %d(%s)\n", i, c);
      near_line();
      definition_error = TRUE;
      return (FALSE);
    }
  }

  if (Registers == (struct t_queue *) 0)
    return (FALSE);

  for (entry = (struct t_entry *) head_queue(Registers);
       entry != (struct t_entry *) 0;
       entry = (struct t_entry *) next_queue(Registers))
  {
    if ((entry->entryid == i) && (entry->name == c))
      return (TRUE);
  }

  return (FALSE);
}


static int
check_all_definitions()
{
  struct t_entry   *en;
  struct t_element *el;
  t_boolean     id0 = FALSE, id1 = FALSE, id2 = FALSE, id3 = FALSE, id4 = FALSE,
                id5 = FALSE, id6 = FALSE, id7 = FALSE, id40 = FALSE,
                id41 = FALSE, id42 = FALSE, id43 = FALSE, id48 = FALSE,
                id49 = FALSE, id50 = FALSE, id100 = FALSE, id101 = FALSE,
                id102 = FALSE, id103 = FALSE, id104 = FALSE, id105 = FALSE,
                id106 = FALSE, id107 = FALSE, id200 = FALSE, id201 = FALSE,
                id300 = FALSE, id301 = FALSE, id302 = FALSE, id303 = FALSE,
                id304 = FALSE, id305 = FALSE, id306 = FALSE, id307 = FALSE,
                id400 = FALSE, id401 = FALSE, id402 = FALSE, id403 = FALSE,
                id404 = FALSE;

  for (en = (struct t_entry *) head_queue(Registers);
       en != (struct t_entry *) 0;
       en = (struct t_entry *) next_queue(Registers))
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
    case 40:
      id40 = TRUE;
      break;
    case 41:
      id41 = TRUE;
      break;
    case 42:
      id42 = TRUE;
      break;
    case 43:
      id43 = TRUE;
      break;
    case 48:
      id48 = TRUE;
      break;
    case 49:
      id49 = TRUE;
      break;
    case 50:
      id50 = TRUE;
      break;
    case 100:
      id101 = TRUE;
      break;
    case 101:
      id101 = TRUE;
      break;
    case 102:
      id102 = TRUE;
      break;
    case 103:
      id103 = TRUE;
      break;
    case 104:
      id104 = TRUE;
      break;
    case 105:
      id105 = TRUE;
      break;
    case 106:
      id106 = TRUE;
      break;
    case 107:
      id107 = TRUE;
      break;
    case 200:
      id200 = TRUE;
      break;
    case 201:
      id201 = TRUE;
      break;
    case 300:
      id300 = TRUE;
      break;
    case 301:
      id301 = TRUE;
      break;
    case 302:
      id302 = TRUE;
      break;
    case 303:
      id303 = TRUE;
      break;
    case 304:
      id304 = TRUE;
      break;
    case 305:
      id305 = TRUE;
      break;
    case 306:
      id306 = TRUE;
      break;
    case 307:
      id307 = TRUE;
      break;
    case 400:
      id400 = TRUE;
      break;
    case 401:
      id401 = TRUE;
      break;
    case 402:
      id402 = TRUE;
      break;
    case 403:
      id403 = TRUE;
      break;
    case 404:
      id404 = TRUE;
      break;
    default:
      fill_parse_error("Error, not recognized field definition\n");
      break;
    }
    if (yydebug)
    {
      fill_parse_error("Entry id: %d, Name: %s\n", en->entryid, en->name);
      
      for(el = (struct t_element *) head_queue(en->types);
          el != (struct t_element *) 0;
          el = (struct t_element *) next_queue(en->types))
      {
        fill_parse_error("\t %s(%d), %d\n",
                         el->i3->i1,
                         el->i3->dimension,
                         el->type);
      }
    }
  }
  
  if (configuration_file)
  {
    if ((definition_error))
    {
      fill_parse_error("Some mandatory definition missed or incorrect\n");
      
      if (!id0)
        fill_parse_error("Definition for #0: \"wide area network information\"\n");
      
      if (!id1)
        fill_parse_error("Definition for #1: \"environment information\"\n");
      
      if (!id2)
        fill_parse_error("Definition for #2: \"node information\"\n");
      
      if (!id3)
        fill_parse_error("Definition for #3: \"mapping information\"\n");
      
      if (!id4)
        fill_parse_error("Definition for #4: \"configuration files\"\n");
      
      if (!id5)
        fill_parse_error("Definition for #5: \"module information\"\n");
      
      if (!id6)
        fill_parse_error("Definition for #6: \"file system parameters\"\n");
      
      if (!id7)
        fill_parse_error("Definition for #7: \"dedicated connection information\"\n");
      
      if (!id40)
        fill_parse_error("Can't proceed with simulation\n");
      
      return (ERROR);
    }
  }
  else
  {
    if ((definition_error))
    {
      fill_parse_error(
        "Some mandatory definition missed or incorrect in file %s\n", 
        current_file);
      
      if (!id1)
        fill_parse_error("Definition for #1: \"CPU burst\"\n");
      
      if (!id2)
        fill_parse_error("Definition for #2: \"NX send\"\n");
      
      if (!id3)
        fill_parse_error("Definition for #3: \"NX recv\"\n");
      
      if (!id40)
        fill_parse_error("Definition for #40: \"block begin\"\n");
      
      if (!id41)
        fill_parse_error("Definition for #41: \"block end\"\n");
      
      if (!id42)
        fill_parse_error("Definition for #42: \"block definition\"\n");
      
      if (!id43)
        fill_parse_error("Definition for #43: \"file definition\"\n");
      
      if (!id48)
        fill_parse_error("Definition for #48: \"user event type definition\"\n");
      
      if (!id49)
        fill_parse_error("Definition for #49: \"user event value definition\"\n");
      
      if (!id50)
        fill_parse_error("Definition for #50: \"user event\"\n");
      
      if (!id100)
        fill_parse_error("Definition for #100: \"communicator definition\"\n");
      
      if (!id101)
        fill_parse_error("Definition for #101: \"FS open\"\n");
      
      if (!id102)
        fill_parse_error("Definition for #102: \"FS read\"\n");
      
      if (!id103)
        fill_parse_error("Definition for #103: \"FS write\"\n");
      
      if (!id104)
        fill_parse_error("Definition for #104: \"FS seek\"\n");
      
      if (!id105)
        fill_parse_error("Definition for #105: \"FS close\"\n");
      
      if (!id106)
        fill_parse_error("Definition for #106: \"FS dup\"\n");
      
      if (!id107)
        fill_parse_error("Definition for #107: \"FS unlink\"\n");
      
      if (!id200)
        fill_parse_error("Definition for #200: \"global OP definition\"\n");
      
      if (!id201)
        fill_parse_error("Definition for #201: \"global OP\"\n");
      
      if (!id300)
        fill_parse_error("Definition for #300: \"IO OP definition\"\n");
      
      if (!id301)
        fill_parse_error("Definition for #301: \"IO Collective Metadata\"\n");
      
      if (!id302)
        fill_parse_error("Definition for #302: \"IO Blocking NonCollective\"\n");
      
      if (!id303)
        fill_parse_error("Definition for #303: \"IO Blocking Collective\"\n");
      
      if (!id304)
        fill_parse_error("Definition for #304: \"IO NonBlocking NonCollective begin\"\n");
      
      if (!id305)
        fill_parse_error("Definition for #305: \"IO NonBlocking NonCollective end\"\n");
      
      if (!id306)
        fill_parse_error("Definition for #306: \"IO NonBlocking Collective begin\"\n");
      
      if (!id307)
        fill_parse_error("Definition for #307: \"IO NonBlocking Collective end\"\n");
      
      if (!id400)
        fill_parse_error("Definition for #400: \"OS window definition\"\n");
      
      if (!id401)
        fill_parse_error("Definition for #401: \"OS operation\"\n");
      
      if (!id402)
        fill_parse_error("Definition for #402: \"OS fence\"\n");
      
      if (!id403)
        fill_parse_error("Definition for #403: \"OS lock\"\n");
      
      if (!id404)
        fill_parse_error("Definition for #404: \"OS post\"\n");
      
      fill_parse_error("Can't proceed with simulation\n");
      
      return (ERROR);
    }
  }
  return (NO_ERROR);
}

static int
free_memory()
{
  struct t_entry   *entry;
  struct t_element *el;
  struct t_array   *ar;
  struct Str       *str, *n;

  for (entry  = (struct t_entry *) outFIFO_queue(Registers);
       entry != (struct t_entry *) 0;
       entry  = (struct t_entry *) outFIFO_queue(Registers))
  {
    for(el  = (struct t_element *) outFIFO_queue(entry->types);
        el != (struct t_element *) 0;
        el  = (struct t_element *) outFIFO_queue(entry->types))
    {
      ar = el->i3;
      freeame((char *) ar, sizeof(struct t_array));
      freeame((char *) el, sizeof(struct t_element));
    }
    freeame((char *) entry->types, sizeof(struct t_queue));
    freeame((char *) entry, sizeof(struct t_entry));
  }
  
  str = first_str;
  while (str != (struct Str *) 0)
  {
    n = str->next_str;
    freeame(str->str, strlen(str->str) + 1);
    freeame((char *) str, sizeof(struct Str));
    str = n;
  }
  
  if (definition_error)
    return (ERROR);
  
  return (NO_ERROR);
}

static void
yyerror(char* s)
{
  fill_parse_error("Error: %s\n", s);
  near_line();
  definition_error = TRUE;
}
