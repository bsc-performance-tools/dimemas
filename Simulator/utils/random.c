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

#include "random.h"

#include <aleatorias.h>
#include <configuration.h>
#include <define.h>
#include <extern.h>
#include <float.h>
#include <stdio.h>
#include <subr.h>
#include <types.h>

struct t_randomness randomness;

void RANDOM_Init()
{
  FILE *fi;
  int j;

  /* srandomine (time (0), time (0) + time (0)); */
  srandomine( 16994, 18996 );
  srandomine( time( 0 ), time( 0 ) + time( 0 ) );

  /* Initialize randomness to have proper default values */
  randomness.processor_ratio.distribution            = NO_DISTRIBUTION;
  randomness.network_bandwidth.distribution          = NO_DISTRIBUTION;
  randomness.network_latency.distribution            = NO_DISTRIBUTION;
  randomness.memory_bandwidth.distribution           = NO_DISTRIBUTION;
  randomness.memory_latency.distribution             = NO_DISTRIBUTION;
  randomness.external_network_bandwidth.distribution = NO_DISTRIBUTION;
  randomness.external_network_latency.distribution   = NO_DISTRIBUTION;
  randomness.acc_memory_latency.distribution         = NO_DISTRIBUTION;
  randomness.acc_memory_bandwith.distribution        = NO_DISTRIBUTION;

  CONFIGURATION_Load_Random_Configuration();

  return;
}

t_boolean RANDOM_Init_Distribution( char *dist_name, float param1, float param2, struct t_rand_type *rt )
{
  if ( strcmp( dist_name, "NONE" ) == 0 )
  {
    rt->distribution = NO_DISTRIBUTION;
    return TRUE;
  }

  if ( strcmp( dist_name, "NORMAL" ) == 0 )
  {
    rt->distribution            = NORMAL_DISTRIBUTION;
    rt->parameters.normal.mean  = param1;
    rt->parameters.normal.stdev = param2;
    return TRUE;
  }

  if ( strcmp( dist_name, "UNIFORM" ) == 0 )
  {
    rt->distribution             = UNIFORM_DISTRIBUTION;
    rt->parameters.uniform.left  = param1;
    rt->parameters.uniform.right = param2;
    return TRUE;
  }

  return FALSE;
}

double RANDOM_GenerateRandom( struct t_rand_type *rt )
{
  double result;
  switch ( rt->distribution )
  {
    case NORMAL_DISTRIBUTION:
      result = ( (double)rnorm( (float)rt->parameters.normal.mean, (float)-INT_MAX, (float)INT_MAX, (float)rt->parameters.normal.stdev ) );
      return result;
    case UNIFORM_DISTRIBUTION:
      return ( (double)unform( (float)rt->parameters.uniform.left, (float)rt->parameters.uniform.right ) );
    default:
      return (double)0;
  }
}
