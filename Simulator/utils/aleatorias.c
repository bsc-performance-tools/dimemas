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

#include "aleatorias.h"

#include "define.h"
#include "extern.h"
#include "types.h"

#include <math.h>

static int Q1 = 13, Q2 = 2, S1 = 12, S2 = 17, P1mS1 = 19, P2mS2 = 12, P1mP2 = 2;
static unsigned int I1, I2, b, Mask1 = 2147483647, Mask2 = 536870911;
double Norm = 4.656612873e-10;

void srandomine( unsigned int seed1, unsigned int seed2 )
{
  I1 = seed1 & Mask1;
  I2 = seed2 & Mask2;
}

float randomine()
{
  b  = ( ( I1 << Q1 ) ^ I1 ) & Mask1;
  I1 = ( ( I1 << S1 ) ^ ( b >> P1mS1 ) ) & Mask1;
  b  = ( ( I2 << Q2 ) ^ I2 ) & Mask2;
  I2 = ( ( I2 << S2 ) ^ ( b >> P2mS2 ) ) & Mask2;
  return ( (float)( I1 ^ ( I2 << P1mP2 ) ) * Norm );
}

/*****************************************************/
/* p1: mean     */
/* p2: xleft    */
/* p3: xrigth   */
/* p4: stdev    */
float rnorm( float p1, float p2, float p3, float p4 )
{
  static float v_funcion, v, ra, rb;

  ra        = randomine();
  rb        = randomine();
  v         = ( sqrt( (double)( -2.0 * log( ra ) ) ) ) * (float)cos( (double)( 6.283 * rb ) );
  v_funcion = v * p4 + p1;
  if ( v_funcion - p2 < 0 )
    goto L6;
  if ( v_funcion - p2 == 0 )
    goto L7;
  if ( v_funcion - p2 > 0 )
    goto L8;

L6:
  v_funcion = p2;
L7:
  return v_funcion;

L8:
  if ( v_funcion - p3 < 0 )
    goto L7;
  if ( v_funcion - p3 == 0 )
    goto L7;
  if ( v_funcion - p3 > 0 )
    goto L9;

L9:
  v_funcion = p3;

  return v_funcion;
}


float erlng( float p1, float p2, float p3, float p4 )
{
  static float v_funcion, r, rnum;
  static int i, k;

  k = p4;

  r = 1;
  for ( i = 1; i <= k; i++ )
  {
    rnum = randomine();
    r    = r * rnum;
  }

  v_funcion = -p1 * log( r );
  if ( v_funcion - p2 < 0 )
    goto L7;
  if ( v_funcion - p2 == 0 )
    goto L5;
  if ( v_funcion - p2 > 0 )
    goto L6;

L7:
  v_funcion = p2;

L5:
  return v_funcion;

L6:
  if ( v_funcion - p3 <= 0 )
    goto L5;
  if ( v_funcion - p3 > 0 )
    goto L4;

L4:
  v_funcion = p3;
  return v_funcion;
}

float expo( float mean, float min, float max )
{
  float v_funcion;
  float rnum;


  rnum      = randomine();
  v_funcion = -mean;
  v_funcion = v_funcion * (float)log( (double)rnum );
  if ( v_funcion < min )
    v_funcion = min;
  if ( v_funcion > max )
    v_funcion = max;
  return ( v_funcion );
}

float unform( float min, float max )
{
  float rnum, v_funcion;

  rnum      = randomine();
  v_funcion = min + ( ( max - min ) * rnum );
  return v_funcion;
}
