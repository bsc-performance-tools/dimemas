/*****************************************************************************\
 *                        ANALYSIS PERFORMANCE TOOLS                         *
 *                                  trf2trf                                  *
 *       Dimemas TRF to Dimemas DIM trace translator (old to new format)     *
 *****************************************************************************
 *     ___        This tool is free software; you can redistribute it and/or *
 *    /  __         modify it under the terms of the GNU LGPL as published   *
 *   /  /  _____    by the Free Software Foundation; either version 2.12.1   *
 *  /  /  /     \   of the License, or (at your option) any later version.   *
 * (  (  ( B S C )                                                           *
 *  \  \  \_____/   This tool is distributed in hope that it will be         *
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

  $URL:: https://svn.bsc.es/repos/ptools/prv2dim/                          $:

  $Rev:: 478                        $:  Revision of last commit
  $Author:: jgonzale                $:  Author of last commit
  $Date:: 2010-10-28 13:58:59 +0200 $:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef _PCF_GENERATOR_HPP_
#define _PCF_GENERATOR_HPP_

#include <basic_types.h>

#include <map>
using std::map;
using std::pair;

#include <set>
using std::set;

#include <string>
using std::string;

class PCFGenerator
{
  private:
    map<pair<INT64, INT64>, string> DefinedBlocks;
    map<pair<INT64, INT64>, string> DefinedEvents;
    set<pair<INT64, INT64> >        UsedBlocks;
    
  public:

    void AddBlockDefinition(INT64 Type, INT64 Value, string Name);
    void AddEventDefinition(INT64 Type, INT64 Value, string Name);
    void SetUsedBlockUsed  (INT64 Type, INT64 Value);

    void FlushPCF(void);
};

#endif
