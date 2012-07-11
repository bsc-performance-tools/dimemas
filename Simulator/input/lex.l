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
char lex_l_rcsid[]="$Id$";
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
/*#include "y.tab.h"*/
#include "ts.h"
#include "mallocame.h"


/* A les Itanium(al menys les 2)  aixo ha d'estar abans del y.tab.h */
#ifndef YYSTYPE
#define YYSTYPE long
#endif
#include "yacc.h"


extern int line_no;
extern struct Str *first_str;
extern YYSTYPE yylval;
extern YYSTYPE insert_string();
extern YYSTYPE deci();
extern YYSTYPE rela();
extern YYSTYPE NUMtype();

%}

delim				[ \t]
ws				({delim})+
strchar				[^"\n]
digit				[0-9]
decimal				({digit})+
real				{decimal}\.({decimal})?

%%

{ws}				;
"\n"				line_no++;
"char"				{yylval=(YYSTYPE)NUMtype();return(NUMTYPE);};
"int"				{yylval=(YYSTYPE)NUMtype();return(NUMTYPE);};
"double"			{yylval=(YYSTYPE)NUMtype();return(NUMTYPE);};
"SDDFA"				return (SDDFA);

"/*"                    {       char c;
                                int i;
				i = line_no;
				loop:
				while ((c=input())!='*')
				{
				  if (c==(char)0)
				  {
				    fprintf (stderr,"Non closed comment started on line %d\n",i);
				    return (0);
				  }
				  if (c=='\n') line_no++;
				}
                                switch (c=input())
				{
				case '/': break;
				default: unput (c);
				         goto loop;
				}
                         };

["]({strchar})*["]		{yylval=insert_string();return (STRING);};
"#"{decimal}":"			{yylval=(YYSTYPE)deci(&yytext[1]);return (IDENTIFICADOR);};
{decimal}			{yylval=(YYSTYPE)deci(yytext);return (DECIMAL);};
-{decimal}			{yylval=(YYSTYPE)deci(yytext);return (DECIMAL);};
{real}				{yylval=(YYSTYPE)rela(); return (REAL);};
";;"				return (SEPARADOR);
"//"				{
				   while (input()!='\n');
				   line_no++;
				};
"{"				return (OPEN_K);
"}"				return (CLOSE_K);
";"				return (PUNTO_I_COMA);
","				return (COMA);
"[]"				return (ARRAY);
"["				return (OPEN_B);
"]"				return (CLOSE_B);

.				return(yytext[0]);

%%

YYSTYPE
deci(v)
char *v;
{
  int i,j;
  int negative=1;

  j = 0;
  if (v[0]=='-')
  {
     v++;
     negative = -1;
  }
  for (i=0;i<yyleng && v[i]>='0' && v[i]<='9'; j=j*10+v[i++]-'0');
  return ((YYSTYPE)j*negative);
}

YYSTYPE
rela()
{
  char *v;
  int i,j;
  double k,l;
  double *r;

  r = (double *)MALLOC_get_memory (sizeof (double));
  v = (char *)yytext;
  j = 0;
  for (i=0;i<yyleng && v[i]!='.'; j=j*10+v[i++]-'0');
  k = (double)j;
  j = 10;
  l=0;
  for (i++;i<yyleng && v[i]>='0' && v[i]<='9'; i++)
  {
    l=l+(double)(v[i]-'0')/j;
    j *= 10;
  }
  *r = k+l;
  return ((YYSTYPE)r);
}

YYSTYPE
insert_string()
{
  char *v;
  int i;
  struct Str *new_str;
  char *new;

  i=yyleng-1;
  new = (char *)MALLOC_get_memory (i);
  strncpy (new, &yytext[1],i);
  new[i-1] = (char)0;

  for (new_str = first_str; new_str!=(struct Str *)0; new_str=new_str->next_str)
    if (strcmp(new_str->str, new)==0)
  {
      MALLOC_free_memory (new);
      return ((YYSTYPE)(new_str->str));
  }

  new_str      = (struct Str*) MALLOC_get_memory(sizeof(struct Str));
  new_str->str = strdup(new);
  new_str->next_str = first_str;
  first_str = new_str;
  MALLOC_free_memory (new);
  return ((YYSTYPE)(new_str->str));
}

YYSTYPE
NUMtype ()
{
  if (yytext[0]=='i') return ((YYSTYPE)0);
  if (yytext[0]=='d') return ((YYSTYPE)1);
  if (yytext[0]=='c') return ((YYSTYPE)2);
  return (-1);
}
