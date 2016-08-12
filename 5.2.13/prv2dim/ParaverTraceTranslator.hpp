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
  $Rev:: 1044                                     $:  Revision of last commit
  $Author:: jgonzale                              $:  Author of last commit
  $Date:: 2012-03-27 17:58:59 +0200 (Tue, 27 Mar #$:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef _PARAVERTRACETRANSLATOR_H
#define _PARAVERTRACETRANSLATOR_H

#include <cerrno>
#include <cstdio>

#include <Error.hpp>
using cepba_tools::Error;

#include <iostream>
using std::cout;
using std::endl;

#include <string>
using std::string;

#include "ParaverTraceParser.hpp"
#include "TranslatorRecord.hpp"
#include "TaskTranslationInfo.hpp"

class ParaverTraceTranslator: public Error
{
  private:

    string ParaverTraceName;
    string DimemasTraceName;
    string ExtraStatsName;

    FILE* ParaverTraceFile;
    FILE* DimemasTraceFile;

    bool withExtraStats;
    bool  DescriptorShared;

    char* CommunicationsFileName;
    FILE* CommunicationsFile;

    vector<PartialCommunication_t>    Communications;
    vector<TranslationCommunicator_t> Communicators;
    vector<TaskTranslationInfo_t>     TranslationInfo;

    ParaverTraceParser_t Parser;

    bool MultiThreadTrace;

    bool PreviouslySimulatedTrace;

    INT32 WrongRecordsFound;

  public:

    ParaverTraceTranslator(void){};

    ParaverTraceTranslator(string ParaverTraceName, string DimemasTraceName, string ExtraStatsName);

    ParaverTraceTranslator(string ParaverTraceName, string DimemasTraceName);

    ParaverTraceTranslator(FILE* ParaverTraceFile, FILE* DimemasTraceFile);

    bool InitTranslator(void);

    bool EndTranslator(void);

    bool SplitCommunications(void);

    bool Translate(bool   GenerateFirstIdle,
                   double IprobeMissesThreshold,
                   INT32  BurstCounterType,
                   double BurstCounterFactor,
                   bool   GenerateMPIInitBarrier);

  private:

    bool InitTranslationStructures(ApplicationDescription_t AppDescription,
                                   double TimeFactor,
                                   bool   GenerateFirstIdle,
                                   double IprobeMissesThreshold,
                                   INT32  BurstCounterType,
                                   double BurstCounterFactor,
                                   bool   GenerateMPIInitBarrier);

    bool TranslateCommunicators(ApplicationDescription_t AppDescription);

    ParaverRecord_t SelectNextRecord(void);

    bool IsDimemasBlockBegin(Event_t EventRecord);

    bool IsDimemasBlockEnd(Event_t EventRecord);

    TranslationCommunicator_t GetCommunicator(INT32 CommId);

    bool ShareDescriptor(void);

    bool WriteNewFormatHeader(ApplicationDescription_t AppDescription,
                              off_t                    OffsetsOffset = 0);
};

#endif /* _PARAVERTRACETRANSLATOR_H */
