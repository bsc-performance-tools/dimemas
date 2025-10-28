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

#ifndef MACROS_H
#define MACROS_H

#include <stdio.h>
#include <stdlib.h>
#include "define.h"

#define MSG_DEBUG(D_MASK, thread, format, ...)             \
{                                                      \
  { \
    if ( debug & D_MASK )                                \
    {                                                    \
      PRINT_TIMER( current_time );                       \
      printf( ": [P:%d T:%d THD:%d] " format "\n", IDENTIFIERS( thread ), ##__VA_ARGS__ );\
    }                                                    \
  } \
}

#define ASSERT_ERROR( x )                                                            \
  {                                                                                  \
    fflush( NULL );                                                                  \
    fprintf( stderr, "\n" );                                                         \
    fprintf( stderr, "* Assertion failed: %s at %s (%d)\n", x, __FILE__, __LINE__ ); \
    fprintf( stderr, "*\n" );                                                        \
    fflush( NULL );                                                                  \
    exit( EXIT_FAILURE );                                                            \
  }

#define ASSERT( x ) \
  if ( !( x ) )     \
  ASSERT_ERROR( #x )

/* PROGRESS MESSAGE */
#define SHOW_PROGRESS( channel, message, current, total )         \
    fprintf( channel, "\r%s %03d/%03ld", message, current, total ); \
    fflush( channel )


#define SHOW_PROGRESS_END( channel, message, total )             \
    fprintf( channel, "\r%s %03ld/%03ld", message, total, total ); \
    fflush( channel )

#define SHOW_PERCENTAGE_PROGRESS( channel, message, percentage ) \
    fprintf( channel, "\r%s %03d %%", message, percentage );       \
    fflush( channel )


#define SHOW_PERCENTAGE_PROGRESS_END( channel, message ) \
    fprintf( channel, "\r%s 100 %%", message );            \
    fflush( channel )

#endif
