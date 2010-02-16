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

#include <stdio.h>

#if defined(ARCH_MACOSX) || defined(ARCH_CYGWIN)
#include "macosx_limits.h"
#else
#include <values.h>
#endif

#include "define.h"
#include "types.h"
#include "extern.h"

#include "aleatorias.h"

#include "random.h"

struct t_randomness randomness;

static char dist[BUFSIZE]; /* Abans era: 256 */
double p1, p2;

static void
distribution_name (char *d , float p1, float p2,  struct t_rand_type *rt)
{
  if (strcmp(d, "NONE")==0)
  {
    rt->distribution = NO_DISTRIBUTION;
    return;
  }
  if (strcmp(d, "NORMAL")==0)
  {
    rt->distribution = NORMAL_DISTRIBUTION;
    rt->parameters.normal.mean = p1;
    rt->parameters.normal.stdev = p2;
    return;
  }
  if (strcmp(d, "UNIFORM")==0)
  {
    rt->distribution = UNIFORM_DISTRIBUTION;
    rt->parameters.uniform.left = p1;
    rt->parameters.uniform.right = p2;
    return;
  }
  panic ("Incorrect distribution name %s in random file\n", d);
}

void
RANDOM_init (char *fichero_random)
{
  FILE *fi;
  int j;

  /* Initialize randomness to have proper default values */
  randomness.processor_ratio.distribution            = NO_DISTRIBUTION;
  randomness.network_bandwidth.distribution          = NO_DISTRIBUTION;
  randomness.network_latency.distribution            = NO_DISTRIBUTION;
  randomness.memory_bandwidth.distribution           = NO_DISTRIBUTION;
  randomness.memory_latency.distribution             = NO_DISTRIBUTION;
  randomness.external_network_bandwidth.distribution = NO_DISTRIBUTION;
  randomness.external_network_latency.distribution   = NO_DISTRIBUTION;
  
  if ((fichero_random == (char *) 0) || (strcmp(fichero_random,"") == 0))
  {
    if (debug)
    {
      printf ("* Random initial routine called. Using default values\n");
    }
  }

  if ((fichero_random != (char *) 0) && (strcmp(fichero_random,"")!=0))
  {
    if (debug)
    {
       PRINT_TIMER (current_time);
       printf (": RANDOM initial routine called with file %s\n",fichero_random);
    }

    fi = MYFOPEN(fichero_random, "r");
    if (fi==(FILE *)0)
      panic ("Can't open random configuration file %s\n", fichero_random);
  
    j = fscanf (fi, "Processor: %s %le %le\n", dist, &p1, &p2);
    if (j!=3)
      panic ("Invalid format in random configuration file %s (processor)\n", 
             fichero_random);
    distribution_name(dist, p1, p2, &randomness.processor_ratio);

    j = fscanf (fi, "Network bandwidth: %s %le %le\n", dist, &p1, &p2);
    if (j!=3)
      panic ("Invalid format in random configuration file %s (network bandwidth)\n", 
             fichero_random);
    distribution_name(dist, p1, p2, &randomness.network_bandwidth);

    j = fscanf (fi, "Network latency: %s %le %le\n", dist, &p1, &p2);
    if (j!=3)
      panic ("Invalid format in random configuration file %s (network latency)\n", 
             fichero_random);
    distribution_name(dist, p1, p2, &randomness.network_latency);

    j = fscanf (fi, "Memory bandwidth: %s %le %le\n", dist, &p1, &p2);
    if (j!=3)
      panic ("Invalid format in random configuration file %s (memory bandwidth)\n", 
             fichero_random);
    distribution_name(dist, p1, p2, &randomness.memory_bandwidth);

    j = fscanf (fi, "Memory latency: %s %le %le\n", dist, &p1, &p2);
    if (j!=3)
      panic ("Invalid format in random configuration file %s (memory latency)\n", 
             fichero_random);
    distribution_name(dist, p1, p2, &randomness.memory_latency);

    j = fscanf (fi, "External Network bandwidth: %s %le %le\n", dist, &p1, &p2);
    if (j!=3)
      panic ("Invalid format in random configuration file %s (external network bandwidth)\n",
             fichero_random);
    distribution_name(dist, p1, p2, &randomness.external_network_bandwidth);

    j = fscanf (fi, "External Network latency: %s %le %le\n", dist, &p1, &p2);
    if (j!=3)
      panic ("Invalid format in random configuration file %s (external network latency)\n",
             fichero_random);
    distribution_name(dist, p1, p2, &randomness.external_network_latency);
  }
  return;
}

double 
random_dist(struct t_rand_type *rt)
{
  double t;
  switch (rt->distribution)
  {
    case NORMAL_DISTRIBUTION:
      t =  ((double)rnorm((float)rt->parameters.normal.mean,
                             (float)-MAXINT,
                             (float)MAXINT,
                             (float)rt->parameters.normal.stdev));
      return (t);
    case UNIFORM_DISTRIBUTION:
      return ((double)unform((float)rt->parameters.uniform.left,
                             (float)rt->parameters.uniform.right));
    default:
      return ((double)0);
  }
}
