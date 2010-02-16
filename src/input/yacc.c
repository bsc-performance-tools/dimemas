
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.4.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
#line 35 "../src/input/yacc.y"


#include "define.h"
#include "types.h"

#include "extern.h"
#include "list.h"
#include "mallocame.h"
#include "subr.h"

#include "ts.h"
#include "check.h"

#define YYSTYPE long
#define YYMAXDEPTH 5000

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



/* Line 189 of yacc.c  */
#line 113 "../src/input/yacc.c"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     SDDFA = 258,
     DECIMAL = 259,
     STRING = 260,
     NUMTYPE = 261,
     REAL = 262,
     IDENTIFICADOR = 263,
     SEPARADOR = 264,
     OPEN_K = 265,
     CLOSE_K = 266,
     PUNTO_I_COMA = 267,
     COMA = 268,
     ARRAY = 269,
     OPEN_B = 270,
     CLOSE_B = 271
   };
#endif
/* Tokens.  */
#define SDDFA 258
#define DECIMAL 259
#define STRING 260
#define NUMTYPE 261
#define REAL 262
#define IDENTIFICADOR 263
#define SEPARADOR 264
#define OPEN_K 265
#define CLOSE_K 266
#define PUNTO_I_COMA 267
#define COMA 268
#define ARRAY 269
#define OPEN_B 270
#define CLOSE_B 271




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 187 "../src/input/yacc.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  4
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   60

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  17
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  13
/* YYNRULES -- Number of rules.  */
#define YYNRULES  27
/* YYNRULES -- Number of states.  */
#define YYNSTATES  55

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   271

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint8 yyprhs[] =
{
       0,     0,     3,     4,     5,    12,    14,    16,    19,    26,
      28,    31,    33,    37,    39,    43,    46,    48,    50,    53,
      59,    62,    66,    68,    70,    72,    74,    84
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      18,     0,    -1,    -1,    -1,    19,     3,     9,    21,    20,
      26,    -1,     1,    -1,    22,    -1,    21,    22,    -1,     8,
       5,    10,    23,    11,     9,    -1,     9,    -1,    24,    23,
      -1,    24,    -1,     6,    25,    12,    -1,     1,    -1,     5,
      14,    14,    -1,     5,    14,    -1,     5,    -1,    27,    -1,
      26,    27,    -1,     5,    10,    28,    11,     9,    -1,     1,
       9,    -1,    29,    13,    28,    -1,    29,    -1,     4,    -1,
       7,    -1,     5,    -1,    15,     4,    16,    15,     4,    16,
      10,    28,    11,    -1,    15,     4,    16,    10,    28,    11,
      -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,    81,    81,    89,    81,   100,   107,   107,   109,   131,
     133,   139,   149,   161,   167,   179,   191,   203,   203,   205,
     224,   231,   237,   247,   257,   268,   278,   292
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "SDDFA", "DECIMAL", "STRING", "NUMTYPE",
  "REAL", "IDENTIFICADOR", "SEPARADOR", "OPEN_K", "CLOSE_K",
  "PUNTO_I_COMA", "COMA", "ARRAY", "OPEN_B", "CLOSE_B", "$accept", "input",
  "$@1", "$@2", "declaraciones", "declaracion", "componentes", "tipo",
  "string_a", "registros", "registro", "campos", "campo", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    17,    19,    20,    18,    18,    21,    21,    22,    22,
      23,    23,    24,    24,    25,    25,    25,    26,    26,    27,
      27,    28,    28,    29,    29,    29,    29,    29
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     0,     6,     1,     1,     2,     6,     1,
       2,     1,     3,     1,     3,     2,     1,     1,     2,     5,
       2,     3,     1,     1,     1,     1,     9,     6
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     5,     0,     0,     1,     0,     0,     0,     9,     3,
       6,     0,     0,     7,     0,     0,     0,     0,    17,    13,
       0,     0,     0,    20,     0,    18,    16,     0,     0,    10,
      23,    25,    24,     0,     0,    22,    15,    12,     8,     0,
       0,     0,    14,     0,    19,    21,     0,     0,     0,     0,
      27,     0,     0,     0,    26
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     2,     3,    12,     9,    10,    21,    22,    27,    17,
      18,    34,    35
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -40
static const yytype_int8 yypact[] =
{
      17,   -40,     8,    19,   -40,    16,    15,    21,   -40,    15,
     -40,    18,    14,   -40,    11,    20,    22,     5,   -40,   -40,
      25,    23,     3,   -40,    -4,   -40,    13,    24,    26,   -40,
     -40,   -40,   -40,    27,    28,    29,    30,   -40,   -40,    31,
      32,    -4,   -40,     6,   -40,   -40,    -4,    33,    34,    35,
     -40,    36,    -4,    37,   -40
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -40,   -40,   -40,   -40,   -40,    40,    38,   -40,   -40,   -40,
      39,   -39,   -40
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -12
static const yytype_int8 yytable[] =
{
      30,    31,    45,    32,    19,    -4,    15,    48,     4,    20,
      16,    33,    19,    53,   -11,    15,    46,    20,     1,    16,
      -2,    47,     5,     7,     8,     6,    11,    36,    14,    23,
      26,    39,    24,     0,    28,    38,    37,    49,     0,    40,
       0,    44,    41,     0,    42,    50,    52,    43,    54,    13,
       0,    51,     0,     0,     0,     0,    25,     0,     0,     0,
      29
};

static const yytype_int8 yycheck[] =
{
       4,     5,    41,     7,     1,     0,     1,    46,     0,     6,
       5,    15,     1,    52,    11,     1,    10,     6,     1,     5,
       3,    15,     3,     8,     9,     9,     5,    14,    10,     9,
       5,     4,    10,    -1,    11,     9,    12,     4,    -1,    11,
      -1,     9,    13,    -1,    14,    11,    10,    16,    11,     9,
      -1,    16,    -1,    -1,    -1,    -1,    17,    -1,    -1,    -1,
      22
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,    18,    19,     0,     3,     9,     8,     9,    21,
      22,     5,    20,    22,    10,     1,     5,    26,    27,     1,
       6,    23,    24,     9,    10,    27,     5,    25,    11,    23,
       4,     5,     7,    15,    28,    29,    14,    12,     9,     4,
      11,    13,    14,    16,     9,    28,    10,    15,    28,     4,
      11,    16,    10,    28,    11
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}

/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*-------------------------.
| yyparse or yypush_parse.  |
`-------------------------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{


    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:

/* Line 1455 of yacc.c  */
#line 81 "../src/input/yacc.y"
    {
  line_no          = 1;
  first_str        = (struct Str *) 0;
  max_task_id      = 0;
  definition_error = FALSE;
}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 89 "../src/input/yacc.y"
    {
  if (check_all_definitions())
    return (ERROR);
  if ((load_interactive) && (configuration_file == FALSE))
    return (NO_ERROR);
}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 96 "../src/input/yacc.y"
    {
  return (free_memory());
}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 101 "../src/input/yacc.y"
    {
  yyerrok;
  yyclearin;
  return (ERROR);
}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 110 "../src/input/yacc.y"
    {
  struct t_entry* new_entry;

  if (exist_identifier_or_string((int) (yyvsp[(1) - (6)]), (char *) (yyvsp[(2) - (6)]), (struct t_queue *) (yyvsp[(4) - (6)])))
  {
    fill_parse_error("Duplicated identifier #%d: %s\n", (yyvsp[(1) - (6)]), (char *) (yyvsp[(2) - (6)]));
    near_line();
    definition_error = TRUE;
    (yyval) = (YYSTYPE) 0;
  } else
  {
    new_entry = (struct t_entry *) mallocame(sizeof(struct t_entry));

    new_entry->entryid = (yyvsp[(1) - (6)]);
    new_entry->name    = (char *) (yyvsp[(2) - (6)]);
    new_entry->types   = (struct t_queue *) (yyvsp[(4) - (6)]);
    inFIFO_queue(Registers, (char *) new_entry);

    (yyval) = (YYSTYPE) new_entry;
  }
}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 134 "../src/input/yacc.y"
    {
  inLIFO_queue((struct t_queue *) (yyvsp[(2) - (2)]), (char *) (yyvsp[(1) - (2)]));
  (yyval) = (yyvsp[(2) - (2)]);
}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 140 "../src/input/yacc.y"
    {
  struct t_queue* types;

  types = (struct t_queue *) mallocame(sizeof(struct t_queue));
  create_queue(types);
  inFIFO_queue(types, (char *) (yyvsp[(1) - (1)]));
  (yyval) = (YYSTYPE) types;
}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 150 "../src/input/yacc.y"
    {
  struct t_element* elem;

  elem = (struct t_element *) mallocame(sizeof(struct t_element));

  elem->i3   = (struct t_array *) (yyvsp[(2) - (3)]);
  elem->type = (yyvsp[(1) - (3)]);

  (yyval) = (YYSTYPE) elem;
}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 162 "../src/input/yacc.y"
    {
  yyerrok;
  yyclearin;
}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 168 "../src/input/yacc.y"
    {
  struct t_array* a;

  a = (struct t_array *) mallocame(sizeof(struct t_array));

  a->i1        = (char *) (yyvsp[(1) - (3)]);
  a->dimension = 2;

  (yyval) = (YYSTYPE) a;
}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 180 "../src/input/yacc.y"
    {
  struct t_array* a;

  a = (struct t_array *) mallocame(sizeof(struct t_array));
  
  a->i1        = (char *) (yyvsp[(1) - (2)]);
  a->dimension = 1;
  
  (yyval) = (YYSTYPE) a;
}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 192 "../src/input/yacc.y"
    {
  struct t_array* a;

  a = (struct t_array *) mallocame(sizeof(struct t_array));

  a->i1        = (char *) (yyvsp[(1) - (1)]);
  a->dimension = 0;

  (yyval) = (YYSTYPE) a;
}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 206 "../src/input/yacc.y"
    {
  struct t_entry *en;

  en = exist_identifier((char *) (yyvsp[(1) - (5)]), Registers);
  if (en != (struct t_entry *) 0)
  {
    if (configuration_file)
      check_fields_for_configuration((struct t_queue *) (yyvsp[(3) - (5)]), en);
    else
      check_fields_with_structure((struct t_queue *) (yyvsp[(3) - (5)]), en);
  } else
  {
    fill_parse_error("Unkown register name: %s\n", (char *) (yyvsp[(1) - (5)]));
    near_line();
    definition_error = TRUE;
  }
}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 225 "../src/input/yacc.y"
    {
  yyerrok;
  yyclearin;
  return (ERROR);
}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 232 "../src/input/yacc.y"
    {
  inLIFO_queue((struct t_queue *) (yyvsp[(3) - (3)]), (char *) (yyvsp[(1) - (3)]));
  (yyval) = (yyvsp[(3) - (3)]);
}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 238 "../src/input/yacc.y"
    {
  struct t_queue *campos;

  campos = (struct t_queue *) mallocame(sizeof(struct t_queue));
  create_queue(campos);
  inFIFO_queue(campos, (char *) (yyvsp[(1) - (1)]));
  (yyval) = (YYSTYPE) campos;
}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 248 "../src/input/yacc.y"
    {
  struct t_field *f;

  f = (struct t_field *) mallocame(sizeof(struct t_field));
  f->tipo = 0;
  f->value.dec = (yyval);
  (yyval) = (YYSTYPE) f;
}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 258 "../src/input/yacc.y"
    {
  struct t_field *f;

  f = (struct t_field *) mallocame(sizeof(struct t_field));
  f->tipo = 1;
  f->value.real = *(double *) (yyval);
  freeame((char *) (yyval), sizeof(double));
  (yyval) = (YYSTYPE) f;
}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 269 "../src/input/yacc.y"
    {
  struct t_field *f;

  f = (struct t_field *) mallocame(sizeof(struct t_field));
  f->tipo = 2;
  f->value.string = (char *) (yyval);
  (yyval) = (YYSTYPE) f;
}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 279 "../src/input/yacc.y"
    {
  struct t_field *f;

  f = (struct t_field *) mallocame(sizeof(struct t_field));

  f->tipo           = 3;
  f->value.arr.q    = (struct t_queue *) (yyvsp[(8) - (9)]);
  f->value.arr.dim1 = (yyvsp[(2) - (9)]);
  f->value.arr.dim2 = (yyvsp[(5) - (9)]);
  
  (yyval) = (YYSTYPE) f;
}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 293 "../src/input/yacc.y"
    {
  struct t_field* f;

  f = (struct t_field *) mallocame(sizeof(struct t_field));
  
  f->tipo           = 3;
  f->value.arr.q    = (struct t_queue *) (yyvsp[(5) - (6)]);
  f->value.arr.dim1 = (yyvsp[(2) - (6)]);
  f->value.arr.dim2 = 0;
  
  (yyval) = (YYSTYPE) f;
}
    break;



/* Line 1455 of yacc.c  */
#line 1722 "../src/input/yacc.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 1675 of yacc.c  */
#line 306 "../src/input/yacc.y"


static struct t_entry*
exist_identifier(char *c, struct t_queue* q)
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

static t_boolean
exist_identifier_or_string(int i, char *c, struct t_queue* q)
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

