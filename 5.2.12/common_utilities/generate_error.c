#include "generate_error.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#if !HAVE_OPEN_MEMSTREAM
FILE * open_memstream (char **buf, size_t *len);
#endif

int generate_error(char**__restrict __buffer, __const char*__restrict __format, ...)
{
  int    ret_status = 0;
  size_t size;
  FILE *__stream;
  char *__in_buffer;

  if ( (__stream = open_memstream(&__in_buffer, &size)) == NULL)
    return -1;


  va_list args;
  va_start(args, __format);
  ret_status = vfprintf(__stream, __format, args);
  va_end(args);

  fclose(__stream);

  if (ret_status > 0)
  {
    *__buffer = strdup(__in_buffer);
    free(__in_buffer);
  }
  else
  {
    *__buffer = NULL;
  }

  return ret_status;
}
