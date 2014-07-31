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

#ifndef _DIMEMASSDDFTRANSLATOR_H
#define _DIMEMASSDDFTRANSLATOR_H

#include "DimemasSDDFTraceParser.hpp"

#include <basic_types.h>
#include <Error.hpp>
using cepba_tools::Error;

#include <cerrno>
#include <cstdio>

#include <iostream>
using std::cout;
using std::endl;

#include <string>
using std::string;


class DimemasSDDFTranslator: public Error
{
  private:
  
    string SDDFTraceName;
    string OutputTraceName;

    FILE* SDDFTraceFile;
    FILE* OutputTraceFile;
  
    DimemasSDDFTraceParser_t Parser;
  
  
  public:
  
    DimemasSDDFTranslator(){};

    DimemasSDDFTranslator(string SDDFTraceName, string OutputTraceName);
    
    bool InitTranslator(void);
      
    bool EndTranslator(void);
      
    bool Translate(void);
  
  private:
    
    bool WriteHeader(ApplicationDescription_t AppDescription,
                     bool                     InitialHeader = true,
                     off_t                    OffsetsLength = 0);
  
    bool WriteCommunicators(ApplicationDescription_t AppDescription);
  
    bool ShareDescriptor(void);
};

#endif /* _DIMEMASSDDFTRANSLATOR_H */
