/*****************************************************************************\
 *                        ANALYSIS PERFORMANCE TOOLS                         *
 *                             ClusteringSuite                               *
 *   Infrastructure and tools to apply clustering analysis to Paraver and    *
 *                              Dimemas traces                               *
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

  $Id:: Error.cpp 23 2011-05-17 09:47:12Z jgonzal#$:  Id
  $Rev:: 23                                       $:  Revision of last commit
  $Author:: jgonzale                              $:  Author of last commit
  $Date:: 2011-05-17 11:47:12 +0200 (Tue, 17 May #$:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include "bsc_utils"

#include <cstdlib>

bool bsc_tools::isDouble(const char* str)
{
  char* endptr = 0;
  strtod(str, &endptr);

  if(*endptr != '\0' || endptr == str)
  {
    return false;
  }
  return true;
};

bool bsc_tools::isDouble(const std::string& str)
{
  return bsc_tools::isDouble(str.c_str());
}

double bsc_tools::getDouble(const char* str)
{
  char* endptr = 0;
  double result = strtod(str, &endptr);

  if(*endptr != '\0' || endptr == str)
  {
    return 0;
  }
  return result;
};

double bsc_tools::getDouble(const std::string& str)
{
  return bsc_tools::getDouble(str.c_str());
}

bool bsc_tools::isLongInt(const char* str)
{
  char *p ;

  if(str == NULL || ((!isdigit(str[0])) && (str[0] != '-') && (str[0] != '+'))) return false ;

  strtol(str, &p, 10);

  return (*p == 0);
}

bool bsc_tools::isLongInt(const std::string& str)
{
  return bsc_tools::isLongInt(str.c_str());
}

long int bsc_tools::getLongInt(const char* str)
{
  if (isLongInt(str))
  {
    return (str, NULL, 10);
  }
  else
  {
    return 0;
  }
}

long int bsc_tools::getLongInt(const std::string& str)
{
  return bsc_tools::getLongInt(str.c_str());
}

