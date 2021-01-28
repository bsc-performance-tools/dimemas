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

  $URL:: https://svn.bsc.es/repos/ptools/prv2dim/#$:  File
  $Rev:: 563                                      $:  Revision of last commit
  $Author:: jgonzale                              $:  Author of last commit
  $Date:: 2011-03-22 16:27:21 +0100 (Tue, 22 Mar #$:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include "dimemas_io.h"

#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <sys/resource.h> // GETRLIMIT(2)
#include <sys/stat.h>
#include <sys/types.h>
#include <types.h>  // t_boolean
#include <unistd.h> // FSTAT(2)

// #define DEBUG_IO

static size_t IO_MaximumFileDescriptors;
static size_t IO_OpenedFileDescriptors;

t_boolean IO_error_state;
static char IO_error_string[ 256 ]; // Hardcoded :(

void IO_report_error( const char *format, ... );

/**
 * Initializes the control structures and variables to guarantee the IO
 * scalability
 */
void IO_Init( void )
{
  struct rlimit nofile_limits;
  int i;

  if ( getrlimit( RLIMIT_NOFILE, &nofile_limits ) == -1 )
  {
    perror( "Error trying to get the OS resources available" );
    exit( EXIT_FAILURE );
  }

#ifdef DEBUG_IO
  printf( "No. Files. Hard limit %d. Sof Limit %d\n", (int)nofile_limits.rlim_max, (int)nofile_limits.rlim_cur );
#endif

  IO_MaximumFileDescriptors = nofile_limits.rlim_cur;
  IO_OpenedFileDescriptors  = 0;

  for ( i = 0; i <= IO_MaximumFileDescriptors; i++ )
  {
    if ( fcntl( i, F_GETFD ) != -1 )
    {
      IO_OpenedFileDescriptors++;
    }
  }

#ifdef DEBUG_IO
  printf( "%d used file descriptors at IO_Init\n", IO_OpenedFileDescriptors );
#endif

  IO_error_state = FALSE;
  sprintf( IO_error_string, "no I/O error" );
}

void IO_End( void )
{
  return;
}

size_t IO_available_streams( void )
{
  return ( IO_MaximumFileDescriptors - IO_OpenedFileDescriptors );
}
/**
 * FOPEN(3) wrapper to keep track of the number of descriptors available
 */
FILE *IO_fopen( const char *path, const char *mode )
{
  FILE *result;

#ifdef DEBUG_IO
  // PRINT_TIMER(current_time);
  printf( "IO_fopen('%s', '%s') [%d/%d]\n", path, mode, IO_OpenedFileDescriptors, IO_MaximumFileDescriptors );
#endif

  if ( IO_OpenedFileDescriptors == IO_MaximumFileDescriptors )
  {
    printf( " here a return null report \n" );
    IO_report_error( "no file pointers available, please check OS limits (total: %zu)", IO_MaximumFileDescriptors );
    result = NULL;
  }
  else
  {
    if ( ( result = MYFOPEN( path, mode ) ) == NULL )
    {
      IO_report_error( "%s", strerror( errno ) );
    }
    else
    {
      IO_OpenedFileDescriptors++;
    }
  }

  return result;
}

/**
 * FCLOSE(3) wrapper to keep track of the number of descriptors available
 */
int IO_fclose( FILE *fp )
{
  int result = 0;

  if ( fp == NULL )
  {
    IO_report_error( "unable to close an already closed file" );
    result = EOF;
  }
  else
  {
    if ( ( result = MYFCLOSE( fp ) ) == EOF )
    {
      IO_report_error( "%s", strerror( errno ) );
    }
    else
    {
      IO_OpenedFileDescriptors--;
    }
  }

  return result;
}

/**
 * FTELLO(3) wrapper to keep uniform the I/O
 */
off_t IO_ftello( FILE *stream )
{
  off_t result;

  if ( ( result = MYFTELLO( stream ) ) == -1 )
  {
    IO_report_error( "%s", strerror( errno ) );
  }

  return result;
}

/**
 * FSEEKO(3) wrapper to keep uniform the I/O
 */
int IO_fseeko( FILE *stream, off_t offset, int whence )
{
  int result;

  if ( ( result = MYFSEEKO( stream, offset, whence ) ) == -1 )
  {
    IO_report_error( "%s", strerror( errno ) );
  }

  return result;
}

/**
 * FSEEK(3) wrapper to keep uniform the I/O
 */
int IO_fseek( FILE *stream, long offset, int whence )
{
  int result;

  if ( ( result = MYFSEEK( stream, offset, whence ) ) == -1 )
  {
    IO_report_error( "%s", strerror( errno ) );
  }

  return result;
}

/**
 * Return the error message
 */
const char *IO_get_error( void )
{
  return IO_error_string;
}

/**
 * Checks if the given path corresponds to an existing file
 */
t_boolean IO_file_exists( const char *path )
{
  struct stat stats;

  if ( stat( path, &stats ) == -1 )
  {
    return FALSE;
  }

  if ( S_ISREG( stats.st_mode ) )
  {
    return TRUE;
  }

  return FALSE;
}

/**
 * Processes error messages in a 'printf' style
 */
void IO_report_error( const char *format, ... )
{
  va_list args;
  va_start( args, format );
  vsprintf( IO_error_string, format, args );
  va_end( args );

  IO_error_state = TRUE;
}
