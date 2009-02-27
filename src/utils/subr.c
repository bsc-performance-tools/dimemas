char subr_c_rcsid[]="$Id: subr.c,v 1.2 2005/02/11 18:39:03 paraver Exp $";

#include "define.h"
#include "types.h"

#include "extern.h"
#include "subr.h"

extern char     yy_error_string[];
extern t_boolean yy_error_filled;
extern t_boolean dimemas_GUI;

#ifdef LIBLEXYACC /* LIBLEXYACC */
dimemas_timer current_time;
#endif

void
panic(char *fmt, ...)
{
  va_list args;

  va_start(args, fmt);

  // (void) 
  fprintf (stdout, "Fatal error at time ");
  FPRINT_TIMER (stdout, current_time);
  //(void)
  fprintf (stdout, "\n");
  //(void)
  vfprintf (stdout, fmt, args);
  va_end(args);
  
  /* JGG: Aquí se debe controlar que las trazas parciales de Paraver, si las
   * hay, se deben destruir */
  exit (-1);
}

void
fill_parse_error(char *fmt, ...)
{
  va_list  args;

  if (dimemas_GUI)
    if (yy_error_filled)
      return;

  va_start(args, fmt);

  if (dimemas_GUI)
    vsprintf (yy_error_string, fmt, args);
  else
    vfprintf (stdout, fmt, args);
  va_end(args);
  yy_error_filled = TRUE;
}
