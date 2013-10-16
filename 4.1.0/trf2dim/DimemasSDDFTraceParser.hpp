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

#ifndef _DIMEMASSDDFTRACEPARSER_H
#define _DIMEMASSDDFTRACEPARSER_H

#include <basic_types.h>
#include "ApplicationDescription.hpp"
#include "DimemasRecord.hpp"
#include "Communicator.hpp"
#include "PCFGenerator.hpp"

#include <string>
using std::string;

#include <vector>
using std::vector;

#include <cstdio>


#define PARSER_BUFFER 10000

class DimemasSDDFTraceParser: public Error
{
  private:
  
    string TraceName;
    FILE*  TraceFile;
    off_t  TraceSize;
    
    INT32  TaskCount;
    
    bool   ParsingInitialized;
    bool   FirstTaskOffsetFound;

    UINT32 FirstRecordLine;
    
    ApplicationDescription_t ApplicationStructure;

    PCFGenerator   PCFGeneration;
    
    vector<FILE*> TaskFile;
    vector<off_t> OriginalOffset;
    vector<off_t> CurrentOffset;
    vector<bool>  SharingDescriptors;

  public:
    DimemasSDDFTraceParser(){ ParsingInitialized = false; };

    DimemasSDDFTraceParser(string TraceName,
                           FILE*  TraceFile = NULL);
    
    bool InitTraceParsing(void);
  
    off_t GetTraceSize(void)
    {
      return ParsingInitialized ? TraceSize : (off_t) 0;
    };
  
    off_t GetCurrentOffset(INT32 TaskId)
    {
      if (!ParsingInitialized)
        return (off_t) 0;
      
      return CurrentOffset[TaskId];
    };
    
    ApplicationDescription_t GetApplicationsDescription(void);
      
    DimemasRecord_t GetNextRecord(INT32 TaskId, INT32 ThreadId);
      
    bool Reload(void);

  private:
    bool   GetTaskCount(void);
      
    bool   LocateFirstCPUBurst(INT32 TaskId);
    
    bool   FastLocateInitialOffset(INT32 TaskId);
  
    INT32  GetLongLine(FILE* File, char** Line);
  
    bool   GetTraceLine(FILE* File, char* Line, int LineSize);
  
    CPUBurst_t      ParseSDDFCPUBurst(char* SDDFCPUBurst);
    Send_t          ParseSDDFSend(char* SDDFSend);
    Receive_t       ParseSDDFReceive(char* SDDFReceive);
    Event_t         ParseEvent(char* SDDFEvent);
    Event_t         AppendEvents(Event_t CurrentEvent, char* SDDFEvent);
    GlobalOp_t      ParseSDDFGlobalOp(char* SDDFGlobalOp);
};
typedef DimemasSDDFTraceParser* DimemasSDDFTraceParser_t;

#endif /* _DIMEMASSDDFTRACEPARSER_H */
