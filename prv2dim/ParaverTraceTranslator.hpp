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

#ifndef _PARAVERTRACETRANSLATOR_H
#define _PARAVERTRACETRANSLATOR_H

#include <Error.hpp>
#include <cerrno>
#include <cstdio>
using cepba_tools::Error;

#include <iostream>
using std::cout;
using std::endl;

#include <string>
using std::string;

#include <map>

#include "ParaverTraceParser.hpp"
#include "TaskTranslationInfo.hpp"
#include "TranslatorRecord.hpp"

class ParaverTraceTranslator : public Error
{
 private:
  string ParaverTraceName;
  string DimemasTraceName;
  string ExtraStatsName;

  FILE* RowTraceFile;
  FILE* PcfTraceFile;

  FILE* ParaverTraceFile;
  FILE* DimemasTraceFile;

  bool withExtraStats;
  bool DescriptorShared;

  vector<PartialCommunication_t> Communications;
  vector<TranslationCommunicator_t> Communicators;
  vector<vector<TaskTranslationInfo_t> > TranslationInfo;
  std::map< std::tuple<INT32, UINT32>, INT32> MPICollectiveRoots; // index->CommunicatorId,CollectiveCount data->RootTaskId

  ParaverTraceParser_t Parser;

  bool MultiThreadTrace;

  bool PreviouslySimulatedTrace;

  INT32 WrongRecordsFound;
  bool is_openmp_trace;
  /* Indicates if OpenMP will be translated */
  vector<bool> omp_tasks;
  INT32 omp_tasks_count;
  /* Indicates if CUDA or OpenCL will be translated */
  INT32 AcceleratorType;
  vector<bool> acc_tasks;
  INT32 acc_tasks_count;

 public:
  ParaverTraceTranslator( void ){};
  ParaverTraceTranslator( string ParaverTraceName, string DimemasTraceName, string ExtraStatsName );
  ParaverTraceTranslator( string ParaverTraceName, string DimemasTraceName );
  ParaverTraceTranslator( FILE* ParaverTraceFile, FILE* DimemasTraceFile );

  bool InitTranslator( void );
  bool EndTranslator( void );
  bool SplitCommunications( void );
  bool Translate( bool GenerateFirstIdle,
                  double IprobeMissesThreshold,
                  double TestMissesThreshold,
                  INT32 BurstCounterType,
                  double BurstCounterFactor,
                  bool GenerateMPIInitBarrier );

 private:
  ParaverRecord_t SelectNextRecord( void );
  TranslationCommunicator_t GetCommunicator( INT32 CommId );

  bool InitTranslationStructures( ApplicationDescription_t AppDescription,
                                  double TimeFactor,
                                  bool GenerateFirstIdle,
                                  double IprobeMissesThreshold,
                                  double TestMissesThreshold,
                                  INT32 BurstCounterType,
                                  double BurstCounterFactor,
                                  bool GenerateMPIInitBarrier );

  void treatMultiEvent( const Event_t& CurrentEvent );
  void createPartialCommunication( const Event_t& CurrentEvent,
                                   INT32 Type,
                                   INT32 PartnerTaskId,
                                   INT32 PartnerThreadId,
                                   INT32 Size,
                                   INT32 Tag,
                                   INT32 CommId );

  void createMPICollectiveRoots( const Event_t& CurrentEvent,
                                 INT32 CommunicatorId );

  bool TranslateCommunicators( ApplicationDescription_t AppDescription );
  bool IsDimemasBlockBegin( Event_t EventRecord );
  bool IsDimemasBlockEnd( Event_t EventRecord );
  bool ShareDescriptor( void );
  bool WriteNewFormatHeader( ApplicationDescription_t AppDescription,
                             int acc_tasks_count,
                             const vector<bool>& acc_tasks,
                             int omp_tasks_count,
                             const vector<bool>& omp_tasks,
                             off_t OffsetsOffset = 0 );
  bool AcceleratorTasksInfo( INT32 tasks_count );
  bool OpenMPTasksInfo( INT32 tasks_count );
};

#endif /* _PARAVERTRACETRANSLATOR_H */
