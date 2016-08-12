/*****************************************************************************\
 *                        ANALYSIS PERFORMANCE TOOLS                         *
 *                                  prv2dim                                  *
 *           Paraver to Dimemas trace translator (old and new format)        *
 *****************************************************************************
 *     ___        This tool is free software; you can redistribute it and/or *
 *    /  __         modify it under the terms of the GNU LGPL as published   *
 *   /  /  _____    by the Free Software Foundation; either version 2.12.1    *
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

  $URL:: https://svn.bsc.es/repos/ptools/prv2dim/#$:  File
  $Rev:: 478                                      $:  Revision of last commit
  $Author:: jgonzale                              $:  Author of last commit
  $Date:: 2010-10-28 13:58:59 +0200 (jue, 28 oct #$:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef _TRACEPARSER_H
#define _TRACEPARSER_H

#include <string>
using std::string;

#include <vector>
using std::vector;

#include <stdio.h>
// Required for 'off_t' in modern GNU compilers 
#include <sys/types.h>

#include "ParaverRecord.hpp"
#include "ParaverHeader.hpp"

/* Record type codification for GetNextRecord function */
#define STATE_REC  2
#define EVENT_REC  4
#define COMM_REC   8
#define GLOBOP_REC 16
#define ANY_REC    STATE_REC | EVENT_REC | COMM_REC | GLOBOP_REC

class ParaverTraceParser: public Error
{
  private:

    string ParaverTraceName;
    FILE*  ParaverTraceFile;
    off_t  TraceSize;
    off_t  FirstCommunicatorOffset; /* Offset of first communicator */
    off_t  FirstRecordOffset;       /* Offset of first record (without header
                                     * and communicators) */

    bool   ParsingInitialized;

    UINT32 CurrentLine;
    UINT32 FirstRecordLine;

    ParaverHeader_t Header;

  public:
    ParaverTraceParser(){ ParsingInitialized = false; };

    ParaverTraceParser(string ParaverTraceName,
                       FILE*  ParaverTraceFile = NULL);

    UINT32 GetCurrentLine(void) { return CurrentLine; };

    bool InitTraceParsing(void);

    vector<ApplicationDescription_t> GetApplicationsDescription(void);
    INT32 GetTimeUnits(void);

    ParaverRecord_t GetNextRecord(void);
    ParaverRecord_t GetNextRecord(UINT32 RecordTypeMask);
    ParaverRecord_t GetNextTaskRecord(INT32 TaskId);
    ParaverRecord_t GetNextThreadRecord(INT32 TaskId, INT32 ThreadId);

    State_t         GetNextState(void);
    Event_t         GetNextEvent(void);
    Communication_t GetNextCommunication(void);
    GlobalOp_t      GetNextGlobalOp(void);

    INT32           GetFilePercentage(void);

    bool Reload(void);

  private:

    ParaverRecord_t NextTraceRecord(UINT32 RecordType);

    bool   GetAppCommunicators(ApplicationDescription_t AppDescription);

    INT32  GetLongLine(char** Line);
  
    bool   GetTraceLine(char* Line, int LineSize);

    State_t         ParseState(char* ASCIIState);
    Event_t         ParseEvent(char* ASCIIEvent);
    Communication_t ParseCommunication(char* ASCIICommunication);
    GlobalOp_t      ParseGlobalOp(char* ASCIIGlobalOp);

};
typedef ParaverTraceParser* ParaverTraceParser_t;

#endif /* _TRACEPARSER_H */
