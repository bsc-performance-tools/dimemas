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
#include "ParaverTraceTranslator.hpp"

#include "Dimemas_Generation.h"
#include "EventEncoding.h"
#include "define.h"

#include <limits.h>
// #include "UIParaverTraceConfig.h"

#include <Macros.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
using std::cout;
using std::endl;
using std::flush;

#include <algorithm>
using std::sort;

#include <sstream>
using std::ostringstream;

/*****************************************************************************
 * Public functions
 ****************************************************************************/

ParaverTraceTranslator::ParaverTraceTranslator( string ParaverTraceName, string DimemasTraceName, string ExtraStatsName )
{
  if ( ( ParaverTraceFile = fopen( ParaverTraceName.c_str(), "r" ) ) == NULL )
  {
    SetError( true );
    SetErrorMessage( "Unable to open Paraver trace", strerror( errno ) );
    return;
  }

  Parser = new ParaverTraceParser( ParaverTraceName, ParaverTraceFile );

  this->ParaverTraceName = ParaverTraceName;

  if ( Parser->GetError() )
  {
    SetError( true );
    LastError = Parser->GetLastError();
  }

  if ( ( DimemasTraceFile = fopen( DimemasTraceName.c_str(), "w" ) ) == NULL )
  {
    SetError( true );
    SetErrorMessage( "Unable to create Dimemas trace", strerror( errno ) );
    return;
  }

  this->DimemasTraceName = DimemasTraceName;

  DescriptorShared         = false;
  MultiThreadTrace         = false;
  PreviouslySimulatedTrace = false;
  WrongRecordsFound        = 0;

  this->ExtraStatsName = ExtraStatsName;
  this->withExtraStats = ExtraStatsName.size() > 0;
}

ParaverTraceTranslator::ParaverTraceTranslator( string ParaverTraceName, string DimemasTraceName )
{
  if ( ( ParaverTraceFile = fopen( ParaverTraceName.c_str(), "r" ) ) == NULL )
  {
    SetError( true );
    SetErrorMessage( "Unable to open Paraver trace", strerror( errno ) );
    return;
  }

  Parser = new ParaverTraceParser( ParaverTraceName, ParaverTraceFile );

  this->ParaverTraceName = ParaverTraceName;

  if ( Parser->GetError() )
  {
    SetError( true );
    LastError = Parser->GetLastError();
  }

  if ( ( DimemasTraceFile = fopen( DimemasTraceName.c_str(), "w" ) ) == NULL )
  {
    SetError( true );
    SetErrorMessage( "Unable to create Dimemas trace", strerror( errno ) );
    return;
  }

  this->DimemasTraceName = DimemasTraceName;

  this->withExtraStats     = false;
  DescriptorShared         = false;
  MultiThreadTrace         = false;
  PreviouslySimulatedTrace = false;
  WrongRecordsFound        = 0;
}

ParaverTraceTranslator::ParaverTraceTranslator( FILE* ParaverTraceFile, FILE* DimemasTraceFile )
{
  this->ParaverTraceFile   = ParaverTraceFile;
  this->DimemasTraceFile   = DimemasTraceFile;
  PreviouslySimulatedTrace = false;
  WrongRecordsFound        = 0;
}

bool ParaverTraceTranslator::InitTranslator( void )
{
  cout << "INITIALIZING PARSER... ";
  if ( !Parser->InitTraceParsing() )
  {
    cout << "NOK" << endl;
    SetErrorMessage( "Unable to initilize parsing", Parser->GetLastError().c_str() );
    return false;
  }

  cout << "OK!" << endl;
  return true;
}

bool ParaverTraceTranslator::EndTranslator( void )
{
  if ( this->ParaverTraceFile != NULL )
  {
    if ( fclose( this->ParaverTraceFile ) != 0 )
    {
      SetErrorMessage( "Error closing Paraver trace file", strerror( errno ) );
      return false;
    }
  }

  if ( !DescriptorShared && this->DimemasTraceFile != NULL )
  {
    if ( fclose( this->DimemasTraceFile ) != 0 )
    {
      SetErrorMessage( "Error closing Dimemas trace file", strerror( errno ) );
      return false;
    }
  }

  if ( this->CommunicationsFile != NULL )
  {
    if ( fclose( this->CommunicationsFile ) != 0 )
    {
      SetErrorMessage( "Error closing communications temporal file", strerror( errno ) );
      return false;
    }

    unlink( this->CommunicationsFileName );
  }

  for ( unsigned int i = 0; i < TranslationInfo.size(); i++ )
  {
    for ( unsigned int j = 0; j < TranslationInfo[ i ].size(); j++ )
      delete TranslationInfo[ i ][ j ];
  }

  return true;
}

bool ParaverTraceTranslator::SplitCommunications( void )
{
  // vector<PartialCommunication_t> CommunicationVector;
  vector<ApplicationDescription_t> AppDescription;
  ParaverRecord_t CurrentRecord;
  Event_t CurrentEvent;
  Communication_t CurrentCommunication;
  PartialCommunication_t SplittedCommunication;
  UINT64 i;
  INT32 COMM_WORLD_Id;
  INT32 PseudoCommId;

  INT32 CurrentPercentage = 0;
  INT32 PercentageRead    = 0;

  const char* tmp_dir_default = "/tmp";
  char* tmp_dir;

  if ( ( tmp_dir = getenv( "TMPDIR" ) ) == NULL )
  {
    tmp_dir = strdup( tmp_dir_default );
  }

  CommunicationsFileName = (char*)malloc( strlen( tmp_dir ) + 1 + 50 );

  if ( CommunicationsFileName == NULL )
  {
    SetErrorMessage( "unable to allocate memory to communications filename", strerror( errno ) );
    return false;
  }

  srand( time( NULL ) );
  sprintf( CommunicationsFileName, "%s/ParaverComms_%06d", tmp_dir, rand() % 999999 );
  AppDescription = Parser->GetApplicationsDescription();
  COMM_WORLD_Id  = AppDescription[ 0 ]->GetCOMM_WORLD_Id();

  CommunicationsFile = fopen( CommunicationsFileName, "w+" );

  /* Uncomment this section when testing finishes
    CommunicationsFile = tmpfile();
    */

  if ( CommunicationsFile == NULL )
  {
    ostringstream ErrorMessage;

    ErrorMessage << "Unable to create temporal communications file : ";
    ErrorMessage << strerror( errno ) << endl;
    ErrorMessage << "Please, check permissions on '/tmp' or update 'TMPDIR' environment variable";

    SetErrorMessage( ErrorMessage.str() );
    return false;
  }

  if ( !Parser->Reload() )
  {
    SetErrorMessage( "Unable to reload Paraver trace", Parser->GetLastError().c_str() );
    return false;
  }

  i = 0;

  SHOW_PERCENTAGE_PROGRESS( stdout, "SPLITTING COMMUNICATIONS", CurrentPercentage );

  PseudoCommId = 0;
  // CurrentCommunication = Parser->GetNextCommunication();
  CurrentRecord = Parser->GetNextRecord( EVENT_REC | COMM_REC );
  while ( CurrentRecord != NULL )
  {
    PseudoCommId++;

    // Current Record is an event. We split it, if needed
    //
    if ( ( CurrentEvent = dynamic_cast<Event_t>( CurrentRecord ) ) != NULL )
    {
      INT32 Type;
      INT32 PartnerTaskId, PartnerThreadId;
      INT32 Size, Tag, CommId;

      Type          = -1;
      PartnerTaskId = PartnerThreadId = -1;
      Size = Tag = CommId = -1;

      if ( CurrentEvent->GetTypeValueCount() > 1 )
      {
        /* Capture the 'pseudo-physical receive' events generated by Dimemas */

        for ( unsigned int i = 0; i < CurrentEvent->GetTypeValueCount(); i++ )
        {
          switch ( CurrentEvent->GetType( i ) )
          {
            case 9:
              Type = CurrentEvent->GetValue( i );
              break;
            case 10:
              PartnerTaskId = CurrentEvent->GetValue( i );
              break;
            case 11:
              PartnerThreadId = CurrentEvent->GetValue( i );
              break;
            case 12:
              Size = CurrentEvent->GetValue( i );
              break;
            case 13:
              Tag = CurrentEvent->GetValue( i );
              break;
            case 14:
              CommId = CurrentEvent->GetValue( i );
              break;
          }
        }

        /* If all events are available, generate the the PartialCommunication record */
        // if (SrcCPU != -1 && SrcAppId != -1 && SrcTaskId != -1 && SrcThreadId != -1 &&
        if ( Type != -1 && PartnerTaskId != -1 && Size != -1 && Tag != -1 && CommId != -1 )
        {
          INT32 SrcCPU, SrcAppId, SrcTaskId, SrcThreadId;
          INT32 DstCPU, DstAppId, DstTaskId, DstThreadId;

          if ( Type == PHYSICAL_RECV || Type == LOGICAL_RECV )
          {
            SrcCPU      = 0;
            SrcAppId    = CurrentEvent->GetAppId();
            SrcTaskId   = PartnerTaskId;
            SrcThreadId = PartnerThreadId;
            DstCPU      = 0;
            DstAppId    = CurrentEvent->GetAppId();
            DstTaskId   = CurrentEvent->GetTaskId();
            DstThreadId = CurrentEvent->GetThreadId();
          }
          else if ( Type == PHYSICAL_SEND || Type == LOGICAL_SEND )
          {
            SrcCPU      = 0;
            SrcAppId    = CurrentEvent->GetAppId();
            SrcTaskId   = CurrentEvent->GetTaskId();
            SrcThreadId = CurrentEvent->GetThreadId();
            DstCPU      = 0;
            DstAppId    = CurrentEvent->GetAppId();
            DstTaskId   = PartnerTaskId;
            DstThreadId = PartnerThreadId;
          }

          PreviouslySimulatedTrace = true;
          SplittedCommunication    = new PartialCommunication( Type,
                                                            CurrentEvent->GetTimestamp(),
                                                            SrcCPU,
                                                            SrcAppId,
                                                            SrcTaskId,
                                                            SrcThreadId,
                                                            DstCPU,
                                                            DstAppId,
                                                            DstTaskId,
                                                            DstThreadId,
                                                            Size,
                                                            Tag,
                                                            CommId,
                                                            CurrentEvent->GetRecordCount() );
          Communications.push_back( SplittedCommunication );
        }
      }
    }
    else
    {
      /**
       * JGG (2014/12/17): PseudoCommId is now the CommID of the original
       * execution!
       */
      CurrentCommunication = dynamic_cast<Communication_t>( CurrentRecord );

      SplittedCommunication = new PartialCommunication( LOGICAL_SEND, CurrentCommunication, PseudoCommId );
      Communications.push_back( SplittedCommunication );

      SplittedCommunication = new PartialCommunication( LOGICAL_RECV, CurrentCommunication, PseudoCommId );
      Communications.push_back( SplittedCommunication );

      SplittedCommunication = new PartialCommunication( PHYSICAL_SEND, CurrentCommunication, PseudoCommId );
      Communications.push_back( SplittedCommunication );

      SplittedCommunication = new PartialCommunication( PHYSICAL_RECV, CurrentCommunication, PseudoCommId );
      Communications.push_back( SplittedCommunication );
    }

    delete CurrentRecord;

    if ( PreviouslySimulatedTrace )
    {
      CurrentRecord = Parser->GetNextRecord( EVENT_REC );
    }
    else
    {
      CurrentRecord = Parser->GetNextRecord( COMM_REC | EVENT_REC );
    }

    i++;

    PercentageRead = Parser->GetFilePercentage();

    if ( PercentageRead > CurrentPercentage )
    {
      CurrentPercentage = PercentageRead;
      if ( debug )
        SHOW_PERCENTAGE_PROGRESS( stdout, "SPLITTING COMMUNICATIONS", CurrentPercentage );
    }
  }

  cout << endl;
  cout << "-> Trace first pass finished (communications split)" << endl;
  cout << "   * Records parsed:          " << i << endl;
  ;
  cout << "   * Splitted communications " << Communications.size();
  cout << endl;

  if ( Parser->GetError() )
  {
    SetError( true );
    SetErrorMessage( "Error reading communication records", Parser->GetLastError().c_str() );
    return false;
  }

  cout << "SORTING PARTIAL COMMUNICATIONS" << endl;
  sort( Communications.begin(), Communications.end(), InBlockComparison() );
  cout << "COMMUNICATIONS SORTED" << endl;

  return true;
}

bool ParaverTraceTranslator::WriteNewFormatHeader( ApplicationDescription_t AppDescription,
                                                   int acc_tasks_count,
                                                   const vector<bool>& acc_tasks,
                                                   int omp_tasks_count,
                                                   const vector<bool>& omp_tasks,
                                                   off_t OffsetsOffset )
{
#define OFFSETS_OFFSET_RESERVE 15

  UINT32 OffsetsLength;
  bool InitialHeader = false;

  if ( OffsetsOffset == 0 )
  {
    InitialHeader = true;
  }

  if ( !InitialHeader )
  {
    /* rewind(DimemasTraceFile); */
    if ( fseek( DimemasTraceFile, 0, SEEK_SET ) != 0 )
    {
      SetError( true );
      SetErrorMessage( "error setting position to write definitive header", strerror( errno ) );
      return false;
    }
  }

  if ( fprintf( DimemasTraceFile, "#DIMEMAS:\"%s\":1,", DimemasTraceName.c_str() ) < 0 )
  {
    SetErrorMessage( "error writing header", strerror( errno ) );
    return false;
  }

  if ( InitialHeader )
  {
    /* Reserve characters for future information */
    /* !!! Ensure that the number of crosses is the same that
       OFFSETS_OFFSET_RESERVE value */
    if ( fprintf( DimemasTraceFile, "XXXXXXXXXXXXXXX" ) < 0 )
    {
      SetErrorMessage( "error writing header", strerror( errno ) );
      return false;
    }
  }
  else
  {
    if ( ( OffsetsLength = fprintf( DimemasTraceFile, "%zu", OffsetsOffset ) ) <= 0 )
    {
      SetErrorMessage( "error writing header", strerror( errno ) );
      return false;
    }
  }

  if ( fprintf( DimemasTraceFile, ":%d(", AppDescription->GetTaskCount() ) < 0 )
  {
    SetErrorMessage( "error writing header", strerror( errno ) );

    return false;
  }

  const vector<TaskDescription_t>& TasksInfo = AppDescription->GetTaskInfo();

  for ( UINT32 i = 0; i < TasksInfo.size(); i++ )
  {
    if ( fprintf( DimemasTraceFile, "%d", TasksInfo[ i ]->GetThreadCount() ) < 0 )
    {
      SetErrorMessage( "error writing header", strerror( errno ) );
      return false;
    }

    if ( i < TasksInfo.size() - 1 )
    {
      if ( fprintf( DimemasTraceFile, "," ) < 0 )
      {
        SetErrorMessage( "error writing header", strerror( errno ) );
        return false;
      }
    }
  }

  if ( fprintf( DimemasTraceFile, "),%d", AppDescription->GetCommunicatorCount() ) < 0 )
  {
    SetErrorMessage( "error writing header", strerror( errno ) );
    return false;
  }

  if ( fprintf( DimemasTraceFile, ",%d", acc_tasks_count ) < 0 )
  { /*	if no accelerator tasks, just 0 is printed	*/
    SetErrorMessage( "error writing header", strerror( errno ) );
    return false;
  }

  if ( acc_tasks_count > 0 )
  {
    if ( fprintf( DimemasTraceFile, "(" ) < 0 )
    {
      SetErrorMessage( "error writing header", strerror( errno ) );
      return false;
    }
    for ( UINT32 i = 0; i < acc_tasks.size() && acc_tasks_count > 0; ++i )
    {
      if ( acc_tasks[ i ] )
      {
        if ( fprintf( DimemasTraceFile, "%d", i ) < 0 )
        { // prints task_id
          SetErrorMessage( "error writing header", strerror( errno ) );
          return false;
        }

        --acc_tasks_count;

        if ( acc_tasks_count > 0 )
        { // not last element
          if ( fprintf( DimemasTraceFile, "," ) < 0 )
          {
            SetErrorMessage( "error writing header", strerror( errno ) );
            return false;
          }
        }
      }
    }

    if ( fprintf( DimemasTraceFile, ")" ) < 0 )
    {
      SetErrorMessage( "error writing header", strerror( errno ) );
      return false;
    }
  }

  if ( fprintf( DimemasTraceFile, ",%d", omp_tasks_count ) < 0 )
  { /*	if no omp tasks, just 0 is printed	*/
    SetErrorMessage( "error writing header", strerror( errno ) );
    return false;
  }
  if ( omp_tasks_count > 0 )
  {
    if ( fprintf( DimemasTraceFile, "(" ) < 0 )
    {
      SetErrorMessage( "error writing header", strerror( errno ) );
      return false;
    }

    for ( UINT32 i = 0; i < omp_tasks.size() && omp_tasks_count > 0; ++i )
    {
      if ( omp_tasks[ i ] )
      {
        if ( fprintf( DimemasTraceFile, "%d", i ) < 0 )
        { // prints task_id
          SetErrorMessage( "error writing header", strerror( errno ) );
          return false;
        }

        --omp_tasks_count;

        if ( omp_tasks_count > 0 )
        { // not last element
          if ( fprintf( DimemasTraceFile, "," ) < 0 )
          {
            SetErrorMessage( "error writing header", strerror( errno ) );
            return false;
          }
        }
      }
    }

    if ( fprintf( DimemasTraceFile, ")" ) < 0 )
    {
      SetErrorMessage( "error writing header", strerror( errno ) );
      return false;
    }
  }

  if ( !InitialHeader )
  {
    for ( UINT32 i = 0; i < OFFSETS_OFFSET_RESERVE - OffsetsLength; i++ )
    {
      if ( fprintf( DimemasTraceFile, " " ) < 0 )
      {
        SetErrorMessage( "error writing header", strerror( errno ) );
        return false;
      }
    }
  }

  if ( fprintf( DimemasTraceFile, "\n" ) < 0 )
  {
    SetErrorMessage( "error writing header", strerror( errno ) );
    cout << "Error!" << endl;
    return false;
  }

  return true;
}

void CheckIdleState( UINT64 CurrentTaskId,
                     UINT64 CurrentThreadId,
                     UINT64 max_nthreads,
                     State_t* current_state,
                     ParaverRecord_t CurrentRecord,
                     TaskTranslationInfo_t tti )
{
  State_t last_state = current_state[ CurrentTaskId * max_nthreads + CurrentThreadId ];
  if ( last_state != NULL )
  {
    if ( last_state->GetEndTime() < CurrentRecord->GetTimestamp() )
    {
      UINT64 state_begin_time = last_state->GetEndTime();
      UINT64 state_end_time   = CurrentRecord->GetTimestamp();

      // Adding idle block
      Event_t BeginIdleEvent =
        new Event( state_begin_time, CurrentRecord->GetCPU(), CurrentRecord->GetAppId(), CurrentRecord->GetTaskId(), CurrentRecord->GetThreadId() );

      Event_t EndIdleEvent =
        new Event( state_end_time, CurrentRecord->GetCPU(), CurrentRecord->GetAppId(), CurrentRecord->GetTaskId(), CurrentRecord->GetThreadId() );

      BeginIdleEvent->AddTypeValue( 0, 1 );
      EndIdleEvent->AddTypeValue( 0, 0 );

      tti->PushRecord( BeginIdleEvent );
      tti->PushRecord( EndIdleEvent );

      // Add the new state to last state
      State_t new_state = new State( CurrentRecord->GetCPU(),
                                     CurrentRecord->GetAppId(),
                                     CurrentRecord->GetTaskId(),
                                     CurrentRecord->GetThreadId(),
                                     state_begin_time,
                                     state_end_time,
                                     0 );

      free( current_state[ CurrentTaskId * max_nthreads + CurrentThreadId ] );
      current_state[ CurrentTaskId * max_nthreads + CurrentThreadId ] = new_state;
    }
  }
}

bool ParaverTraceTranslator::Translate( bool GenerateFirstIdle,
                                        double IprobeMissesThreshold,
                                        double TestMissesThreshold,
                                        INT32 BurstCounterType,
                                        double BurstCounterFactor,
                                        bool GenerateMPIInitBarrier )
{
  /* unsigned int CurrentCommunication; */
  vector<ApplicationDescription_t> AppsDescription;
  ParaverRecord_t CurrentRecord;
  INT32 TimeUnits;
  double TimeFactor;

  INT32 CurrentPercentage = 0;
  INT32 PercentageRead    = 0;

  vector<INT32> TasksWithOutsideComms;
  bool OutsideCommsPresent = false;

  vector<INT32> TasksWithWrongComms;
  bool WrongCommsPresent = false;

  vector<INT32> TasksWithNonDeterministicComms;
  bool NonDeterministicCommsPresent = false;

  vector<INT32> TasksWithDisorderedRecords;
  bool DisorderedRecordsPresent = false;

  /* For writting new header format with offsets */
  off_t OffsetsOffset;
  vector<vector<off_t> > OutputOffsets;

  /* For obtaining cluster labels from .pcf
     using namespace libparaver;
     UIParaverTraceConfig *pcfTrace;
     string               InputPCFName;

     vector<unsigned int> ClusteringValues;
     vector<unsigned int> UserFunctionValues;

     int                 *pcfValues;
     char               **pcfLabels;
     */

  cout << flush;
  cout << "INITIALIZING TRANSLATION... ";

  if ( !Parser->Reload() )
  {
    cout << " NOK!" << endl;

    SetErrorMessage( "Unable to reload Paraver trace", Parser->GetLastError().c_str() );
    return false;
  }

  AppsDescription = Parser->GetApplicationsDescription();

  if ( AppsDescription.size() == 0 && Parser->GetError() )
  {
    cout << " NOK!" << endl;

    SetErrorMessage( "Unable to read applications description", Parser->GetLastError().c_str() );
    return false;
  }
  else if ( AppsDescription.size() == 0 )
  {
    cout << " NOK!" << endl;
    SetErrorMessage( "Trace without applications description", Parser->GetLastError().c_str() );
    return false;
  }
  else if ( AppsDescription.size() != 1 )
  {
    cout << " NOK!" << endl;
    SetErrorMessage( "This translator is only able to translate"
                     " Paraver traces with 1 application",
                     Parser->GetLastError().c_str() );
    return false;
  }

  if ( ( TimeUnits = Parser->GetTimeUnits() ) < 0 )
  {
    cout << " NOK!" << endl;
    SetErrorMessage( "Unable to get trace time units", Parser->GetLastError().c_str() );
    return false;
  }

  switch ( TimeUnits )
  {
    case MICROSECONDS:
      TimeFactor = 1e-6;
      break;
    case NANOSECONDS:
      TimeFactor = 1e-9;
      break;
    default:
      cout << " NOK!" << endl;
      SetErrorMessage( "Error reading trace, time units are unknown", Parser->GetLastError().c_str() );
      return false;
      break;
  }
  cout << " OK" << endl;

  cout << "LOOKING FOR OpenMP TASK...  ";
  if ( OpenMPTasksInfo( ( AppsDescription[ 0 ] )->GetTaskCount() ) )
  {
    cout << "FOUND" << endl;
  }
  else
  {
    cout << "NOT FOUND" << endl;
  }

  cout << "LOOKING FOR ACCELERATOR THREADS...  ";
  if ( AcceleratorTasksInfo( ( AppsDescription[ 0 ] )->GetTaskCount() ) )
  {
    cout << "FOUND" << endl;
    if ( debug )
    {
      cout << "\t Accelerator tasks: ";
      for ( INT32 i = 0; i < acc_tasks_count; i++ )
      {
        if ( acc_tasks[ i ] )
          cout << i + 1;
        if ( i + 1 < acc_tasks_count )
          cout << ", ";
      }
      cout << endl;
    }
  }
  else
  {
    cout << "NOT FOUND" << endl;
  }

  if ( !InitTranslationStructures( AppsDescription[ 0 ],
                                   TimeFactor,
                                   GenerateFirstIdle,
                                   IprobeMissesThreshold,
                                   TestMissesThreshold,
                                   BurstCounterType,
                                   BurstCounterFactor,
                                   GenerateMPIInitBarrier ) )
    return false;

  if ( DescriptorShared )
  {
    DimemasTraceFile = fopen( DimemasTraceName.c_str(), "a" );

    if ( DimemasTraceFile == NULL )
    {
      SetErrorMessage( "Error opening output tracefile", strerror( errno ) );
      return false;
    }
  }

  if ( MultiThreadTrace )
  {
    cout << endl << "WARNING! Tasks with multiple threads. ";
    cout << "Just first thread will be translated" << endl;
  }
  else
  {
    cout << endl;
  }

  cout << "WRITING HEADER...";

  /* Temporary header! */
  if ( !WriteNewFormatHeader( AppsDescription[ 0 ], acc_tasks_count, acc_tasks, omp_tasks_count, omp_tasks ) )
    return false;

  cout << " OK" << endl;

  cout << "TRANSLATING COMMUNICATORS...";
  if ( !TranslateCommunicators( AppsDescription[ 0 ] ) )
    return false;

  if ( DescriptorShared )
  {
    if ( fclose( DimemasTraceFile ) != 0 )
    {
      SetError( true );
      SetErrorMessage( "Error closing Dimemas trace file to share descriptors", strerror( errno ) );
      return false;
    }
  }
  cout << " OK" << endl;

  printf( "RECORD TRANSLATION\n" );

  if ( debug )
    SHOW_PERCENTAGE_PROGRESS( stdout, "TRANSLATING RECORDS", CurrentPercentage );

  Parser->Reload();

  CurrentRecord = SelectNextRecord();

  unsigned int max_nthreads = 0;
  for ( UINT32 task = 0; task < TranslationInfo.size(); ++task )
    if ( TranslationInfo[ task ].size() > max_nthreads )
      max_nthreads = TranslationInfo[ task ].size();

  State_t* current_state = (State_t*)calloc( TranslationInfo.size() * max_nthreads, sizeof( State_t ) );

  while ( CurrentRecord != NULL )
  {
    Event_t CurrentEvent;
    State_t CurrentState;
    UINT32 CurrentTaskId   = CurrentRecord->GetTaskId() - 1;
    UINT32 CurrentThreadId = CurrentRecord->GetThreadId() - 1;

    if ( CurrentTaskId < 0 || CurrentTaskId >= TranslationInfo.size() )
    {
      WrongRecordsFound++;
      delete CurrentRecord;
    }
    else if ( CurrentThreadId < 0 || CurrentThreadId >= TranslationInfo[ CurrentTaskId ].size() )
    {
      WrongRecordsFound++;
      delete CurrentRecord;
    }
    else
    {
      if ( debug )
        cout << "SELECTED RECORD: " << endl << *CurrentRecord;

      if ( ( CurrentEvent = dynamic_cast<Event_t>( CurrentRecord ) ) != NULL )
      {
        CheckIdleState( CurrentTaskId,
                        CurrentThreadId,
                        max_nthreads,
                        current_state,
                        CurrentRecord,
                        TranslationInfo[ CurrentTaskId ][ CurrentThreadId ] );
        if ( CurrentEvent->GetTypeValueCount() > 1 )
        {
          for ( unsigned int i = 0; i < CurrentEvent->GetTypeValueCount(); i++ )
          {
            Event_t SubEvent;

            SubEvent = new Event( CurrentEvent->GetTimestamp(),
                                  CurrentEvent->GetCPU(),
                                  CurrentEvent->GetAppId(),
                                  CurrentEvent->GetTaskId(),
                                  CurrentEvent->GetThreadId() );

            SubEvent->AddTypeValue( CurrentEvent->GetType( i ), CurrentEvent->GetValue( i ) );

            bool success = TranslationInfo[ CurrentTaskId ][ CurrentThreadId ]->PushRecord( SubEvent );

            if ( !success )
            {
              SetError( true );
              this->LastError = TranslationInfo[ CurrentTaskId ][ CurrentThreadId ]->GetLastError();
              return false;
            }
          }

          // Here we can delete the Event record, as we have pushed all the
          // Type/Values in sub-events
          delete CurrentRecord;
        }
        else
        {
          if ( !TranslationInfo[ CurrentTaskId ][ CurrentThreadId ]->PushRecord( CurrentRecord ) )
          {
            SetError( true );
            this->LastError = TranslationInfo[ CurrentTaskId ][ CurrentThreadId ]->GetLastError();
            return false;
          }
        }
      }
      else if ( ( CurrentState = dynamic_cast<State_t>( CurrentRecord ) ) != NULL )
      {
        INT32 state_value       = CurrentState->GetStateValue();
        UINT64 state_begin_time = CurrentState->GetBeginTime();
        UINT64 state_end_time   = CurrentState->GetEndTime();

        /*  f(state_value > 1)
        {

        }*/
        if ( state_value == 0 ) // idle
        {
          Event_t BeginIdleEvent =
            new Event( state_begin_time, CurrentState->GetCPU(), CurrentState->GetAppId(), CurrentState->GetTaskId(), CurrentState->GetThreadId() );

          Event_t EndIdleEvent =
            new Event( state_end_time, CurrentState->GetCPU(), CurrentState->GetAppId(), CurrentState->GetTaskId(), CurrentState->GetThreadId() );

          BeginIdleEvent->AddTypeValue( 0, 1 );
          EndIdleEvent->AddTypeValue( 0, 0 );

          bool success = TranslationInfo[ CurrentTaskId ][ CurrentThreadId ]->PushRecord( BeginIdleEvent );

          success = TranslationInfo[ CurrentTaskId ][ CurrentThreadId ]->PushRecord( EndIdleEvent );
        }
        free( current_state[ CurrentTaskId * max_nthreads + CurrentThreadId ] );
        current_state[ CurrentTaskId * max_nthreads + CurrentThreadId ] = CurrentState;
      }
      else
      {
        CheckIdleState( CurrentTaskId,
                        CurrentThreadId,
                        max_nthreads,
                        current_state,
                        CurrentRecord,
                        TranslationInfo[ CurrentTaskId ][ CurrentThreadId ] );
        if ( !TranslationInfo[ CurrentTaskId ][ CurrentThreadId ]->PushRecord( CurrentRecord ) )
        {
          SetError( true );
          this->LastError = TranslationInfo[ CurrentTaskId ][ CurrentThreadId ]->GetLastError();
          return false;
        }
      }
    }

    CurrentRecord  = SelectNextRecord();
    PercentageRead = Parser->GetFilePercentage();

    if ( PercentageRead > CurrentPercentage )
    {
      CurrentPercentage = PercentageRead;
      SHOW_PERCENTAGE_PROGRESS( stdout, "TRANSLATING RECORDS", CurrentPercentage );
    }
  }

  for ( int i = 0; i < TranslationInfo.size() * max_nthreads; ++i )
    if ( current_state[ i ] != NULL )
      free( current_state[ i ] );
  free( current_state );

  if ( this->withExtraStats )
  {
    /* Flush extra statistics on file */
    FILE* extra_statistics_file = fopen( this->ExtraStatsName.c_str(), "w" );
    assert( extra_statistics_file != NULL );

    fprintf( extra_statistics_file, "// #send,#isend,#recv, #irecv, #wait, #glop\n" );

    unsigned int min_glops = UINT_MAX;
    for ( unsigned int i = 0; i < TranslationInfo.size(); ++i )
    {
      for ( unsigned int j = 0; j < TranslationInfo[ i ].size(); j++ )
      {
        if ( TranslationInfo[ i ][ j ]->glop_counter < min_glops )
          min_glops = TranslationInfo[ i ][ j ]->glop_counter;
      }
    }

    for ( unsigned int i = 0; i < TranslationInfo.size(); ++i )
    {
      for ( unsigned int j = 0; j < TranslationInfo[ i ].size(); j++ )
      {
        fprintf( extra_statistics_file,
                 "%u,%u,%u,%u,%u,%u,%u,%u,%u\n",
                 TranslationInfo[ i ][ j ]->send_counter,
                 TranslationInfo[ i ][ j ]->isend_counter,
                 TranslationInfo[ i ][ j ]->recv_counter,
                 TranslationInfo[ i ][ j ]->irecv_counter,
                 TranslationInfo[ i ][ j ]->wait_counter,
                 TranslationInfo[ i ][ j ]->glop_counter,
                 TranslationInfo[ i ][ j ]->pendent_i_Send_counter,
                 TranslationInfo[ i ][ j ]->pendent_i_Recv_counter,
                 TranslationInfo[ i ][ j ]->glop_counter - min_glops );
        // TranslationInfo[i]->pendent_Glop_counter);
      }
    }

    fclose( extra_statistics_file );
  }

  if ( Parser->GetError() )
  {
    SetError( true );
    SetErrorMessage( Parser->GetLastError() );
    return false;
  }

  if ( DescriptorShared )
  {
    if ( ( DimemasTraceFile = fopen( DimemasTraceName.c_str(), "a" ) ) == NULL )
    {
      SetError( true );
      SetErrorMessage( "Unable to re-open Dimemas trace", strerror( errno ) );
      return false;
    }
  }

  cout << endl;
  cout << "MERGING PARTIAL OUTPUT TRACES" << endl;
  unsigned int TotalMPIInitBarriersWritten = 0;
  for ( UINT32 i = 0; i < TranslationInfo.size(); i++ )
  {
    for ( UINT32 j = 0; j < TranslationInfo[ i ].size(); j++ )
    {
      /* Generate last records and close temporary files to guarantee the
       * availability of file descriptors */
      if ( !( TranslationInfo[ i ][ j ]->LastFlush() ) )
      {
        char CurrentError[ 128 ];

        sprintf( CurrentError, "Error flushing last records from task %02d : ", i + 1 );
        SetErrorMessage( CurrentError, TranslationInfo[ i ][ j ]->GetLastError().c_str() );
        return false;
      }
    }
  }

  OutputOffsets.resize( TranslationInfo.size() );

  for ( UINT32 i = 0; i < TranslationInfo.size(); i++ )
  {
    OutputOffsets[ i ].resize( TranslationInfo[ i ].size() );

    for ( UINT32 j = 0; j < TranslationInfo[ i ].size(); j++ )
    {
      OutputOffsets[ i ][ j ] = ftello( DimemasTraceFile );
      if ( !( TranslationInfo[ i ][ j ]->Merge( DimemasTraceFile ) ) )
      {
        char CurrentError[ 128 ];

        sprintf( CurrentError, "Error merging file from task %02d : ", i + 1 );
        SetErrorMessage( CurrentError, TranslationInfo[ i ][ j ]->GetLastError().c_str() );
        return false;
      }

      if ( TranslationInfo[ i ][ j ]->GetOutsideComms() )
      {
        OutsideCommsPresent = true;
        TasksWithOutsideComms.push_back( TranslationInfo[ i ][ j ]->GetTaskId() );
      }

      if ( TranslationInfo[ i ][ j ]->GetWrongComms() )
      {
        WrongCommsPresent = true;
        TasksWithWrongComms.push_back( TranslationInfo[ i ][ j ]->GetTaskId() );
      }

      if ( TranslationInfo[ i ][ j ]->GetNonDeterministicComms() )
      {
        NonDeterministicCommsPresent = true;
        TasksWithNonDeterministicComms.push_back( TranslationInfo[ i ][ j ]->GetTaskId() );
      }

      if ( TranslationInfo[ i ][ j ]->GetDisorderedRecords() )
      {
        DisorderedRecordsPresent = true;
        TasksWithDisorderedRecords.push_back( TranslationInfo[ i ][ j ]->GetTaskId() );
      }


      if ( TranslationInfo[ i ][ j ]->GetMPIInitBarrierWritten() )
      {
        TotalMPIInitBarriersWritten++;
      }
    }
  }
  printf( "   * All task merged!         \r\n" );

  if ( OutsideCommsPresent )
  {
    cout << endl;
    cout << "********************************************************************************" << endl;
    cout << "*                               WARNING                                        *" << endl;
    cout << "********************************************************************************" << endl;
    cout << TasksWithOutsideComms.size() << " ";
    cout << "tasks have communications records outside a communication block" << endl;
    cout << "WARNING: The simulation of this trace could be inconsistent" << endl;
    cout << "NOTE: If the Paraver trace comes from a trace cut, check the cut limits" << endl;
    cout << "********************************************************************************" << endl;
    cout << endl;
  }

  if ( WrongCommsPresent )
  {
    cout << endl;
    cout << "********************************************************************************" << endl;
    cout << "*                               WARNING                                        *" << endl;
    cout << "********************************************************************************" << endl;
    cout << TasksWithWrongComms.size() << " ";
    cout << "tasks have communications records wrapped with not matching blocks" << endl;
    cout << "WARNING: The simulation of this trace could be inconsistent" << endl;
    cout << "NOTE: Please check the Paraver trace time resolution" << endl;
    cout << "********************************************************************************" << endl;
    cout << endl;
  }

  if ( NonDeterministicCommsPresent )
  {
    cout << endl;
    cout << "********************************************************************************" << endl;
    cout << "*                               WARNING                                        *" << endl;
    cout << "********************************************************************************" << endl;
    cout << TasksWithNonDeterministicComms.size() << " ";
    cout << "tasks of this application execute 'non-deterministic' communications " << endl;
    cout << "primitives (MPI_Test[*] | MPI_Waitany | MPI_Waitall | MPI_Waitsome)" << endl;
    cout << "The simulation of this trace will not capture the possible indeterminism " << endl;
    cout << "********************************************************************************" << endl;
    cout << endl;
  }

  if ( DisorderedRecordsPresent )
  {
    cout << "********************************************************************************" << endl;
    cout << "*                               WARNING                                        *" << endl;
    cout << "********************************************************************************" << endl;
    cout << TasksWithDisorderedRecords.size() << " ";
    cout << "tasks have disordered records" << endl;
    cout << "WARNING: The simulation of this trace could be inconsistent" << endl;
    cout << "NOTE: Contact tools@bsc.es to check how to solve this problem" << endl;
    cout << "********************************************************************************" << endl;
    cout << endl;
  }

  if ( WrongRecordsFound > 0 )
  {
    cout << "********************************************************************************" << endl;
    cout << "*                               WARNING                                        *" << endl;
    cout << "********************************************************************************" << endl;
    cout << WrongRecordsFound << " wrong records found in the input trace.  " << endl;
    cout << "WARNING: The simulation of this trace could be inconsistent" << endl;
    cout << "NOTE: Contact tools@bsc.es to check how to solve this problem" << endl;
    cout << "********************************************************************************" << endl;
    cout << endl;
  }

  if ( TotalMPIInitBarriersWritten != 0 && TotalMPIInitBarriersWritten != TranslationInfo.size() && GenerateMPIInitBarrier )
  {
    cout << "********************************************************************************" << endl;
    cout << "*                               ERROR                                          *" << endl;
    cout << "********************************************************************************" << endl;
    cout << "ERROR: The 'MPI_Init' primitive does not appear at the beggining of the all" << endl;
    cout << "ERROR: tasks in this trace. Current translation will fail" << endl;
    cout << endl;
    cout << "ERROR: Please, re-run the translator using the '-s' flags" << endl;
    cout << "********************************************************************************" << endl;
    cout << endl;
  }


#ifdef NEW_DIMEMAS_TRACE

  /* Writting offsets */

  OffsetsOffset = ftello( DimemasTraceFile );
  if ( OffsetsOffset == -1 )
  {
    SetErrorMessage( "Error getting offsets offset on output trace file", strerror( errno ) );
    return false;
  }

  for ( INT32 Task = 0; Task < ( AppsDescription[ 0 ] )->GetTaskCount(); Task++ )
  {
    if ( fprintf( DimemasTraceFile, "s:%d", Task ) < 0 )
    {
      SetErrorMessage( "Error printing offsets line", strerror( errno ) );
      return false;
    }

    const vector<TaskDescription_t>& TasksInfo = ( AppsDescription[ 0 ] )->GetTaskInfo();

    for ( INT32 Thread = 0; Thread < TasksInfo[ Task ]->GetThreadCount(); Thread++ )
    {
      if ( fprintf( DimemasTraceFile, ":%zu", OutputOffsets[ Task ][ Thread ] ) < 0 )
      {
        SetErrorMessage( "Error printing offsets line", strerror( errno ) );
        return false;
      }
    }

    if ( fprintf( DimemasTraceFile, "\n" ) < 0 )
    {
      SetErrorMessage( "Error printing offsets line", strerror( errno ) );
      return false;
    }
  }

  if ( DescriptorShared )
  {
    fclose( DimemasTraceFile );

    DimemasTraceFile = fopen( DimemasTraceName.c_str(), "r+" );

    if ( DimemasTraceFile == NULL )
    {
      SetErrorMessage( "Error opening output tracefile", strerror( errno ) );
      return false;
    }
  }

  if ( !WriteNewFormatHeader( AppsDescription[ 0 ], acc_tasks_count, acc_tasks, omp_tasks_count, omp_tasks, OffsetsOffset ) )
  {
    return false;
  }

#endif

  cout << "TRANSLATION FINISHED" << endl;

  return true;
}

/*****************************************************************************
 * Private functions
 ****************************************************************************/

bool ParaverTraceTranslator::InitTranslationStructures( ApplicationDescription_t AppDescription,
                                                        double TimeFactor,
                                                        bool GenerateFirstIdle,
                                                        double IprobeMissesThreshold,
                                                        double TestMissesThreshold,
                                                        INT32 BurstCounterType,
                                                        double BurstCounterFactor,
                                                        bool GenerateMPIInitBarrier )
{
  vector<Communicator_t> TraceCommunicators;
  UINT32 CurrentCommunicator, CurrentTask, CurrentThread;
  TaskTranslationInfo_t NewTranslationInfo;
  FILE* TemporaryFile;

  bool BurstCounterGeneration;

  if ( BurstCounterType != -1 )
    BurstCounterGeneration = true;
  else
    BurstCounterGeneration = false;

  /* Temporal directory management */
  char* TemporaryFileName;
  const char* tmp_dir_default = "/tmp";
  char* tmp_dir;

  if ( ( tmp_dir = getenv( "TMPDIR" ) ) == NULL )
  {
    tmp_dir = strdup( tmp_dir_default );
  }

  /* Translation communicator initialization */
  TraceCommunicators = AppDescription->GetCommunicators();

  for ( CurrentCommunicator = 0; CurrentCommunicator < TraceCommunicators.size(); CurrentCommunicator++ )
  {
    TranslationCommunicator_t NewTransalationComm;

    NewTransalationComm = new TranslationCommunicator( TraceCommunicators[ CurrentCommunicator ] );

    Communicators.push_back( NewTransalationComm );
  }

  const vector<TaskDescription_t>& TaskInfo = AppDescription->GetTaskInfo();

  TranslationInfo.resize( TaskInfo.size() );


  if ( !Parser->Reload() )
  {
    SetErrorMessage( "Unable to reload Paraver trace", Parser->GetLastError().c_str() );
    return false;
  }

  int max_nthreads           = 0;
  unsigned int total_threads = 0;
  for ( unsigned int task = 0; task < TaskInfo.size(); ++task )
  {
    total_threads += TaskInfo[ task ]->GetThreadCount();
    if ( TaskInfo[ task ]->GetThreadCount() > max_nthreads )
      max_nthreads = TaskInfo[ task ]->GetThreadCount();
  }

  INT64* first_record_times = (INT64*)calloc( TaskInfo.size() * max_nthreads, sizeof( INT64 ) );
  for ( unsigned int i = 0; i < TaskInfo.size() * max_nthreads; ++i )
    first_record_times[ i ] = -1;

  UINT64 done = 0;
  while ( done < total_threads )
  {
    ParaverRecord_t Record = Parser->GetNextRecord( EVENT_REC | STATE_REC );
    if ( Record == NULL )
      break;

    UINT64 taskid   = Record->GetTaskId() - 1;
    UINT64 threadid = Record->GetThreadId() - 1;
    if ( first_record_times[ taskid * max_nthreads + threadid ] == -1 )
    {
      first_record_times[ taskid * max_nthreads + threadid ] = Record->GetTimestamp();
      done++;
    }
  }

  for ( CurrentTask = 0; CurrentTask < TaskInfo.size(); CurrentTask++ )
  {
    if ( debug )
      SHOW_PROGRESS( stdout, "CREATING TRANSLATION STRUCTURES ", CurrentTask + 1, TaskInfo.size() );

    UINT32 TaskThreadCount = TaskInfo[ CurrentTask ]->GetThreadCount();

    bool is_acc_task = false;
    if ( !acc_tasks.empty() && acc_tasks[ CurrentTask ] )
      is_acc_task = true;

    bool is_omp_task = false;
    if ( !omp_tasks.empty() && omp_tasks[ CurrentTask ] )
      is_omp_task = true;

    TranslationInfo[ CurrentTask ].resize( TaskThreadCount );

    for ( CurrentThread = 0; CurrentThread < TaskThreadCount; CurrentThread++ )
    {
      ParaverRecord_t FirstRecord;
      bool EmptyTask         = false;
      UINT64 FirstRecordTime = 0;

      INT32 AcceleratorThread = ACCELERATOR_NULL;
      INT32 OpenMP_thread     = OpenMP_NULL;

      if ( is_acc_task && CurrentThread != 0 )
        AcceleratorThread = ACCELERATOR_KERNEL;
      else if ( is_acc_task && CurrentThread == 0 )
        AcceleratorThread = ACCELERATOR_HOST;

      if ( is_omp_task && CurrentThread == 0 )
        OpenMP_thread = MASTER;
      else if ( is_omp_task && CurrentThread != 0 )
        OpenMP_thread = WORKER;

      TemporaryFileName = (char*)malloc( strlen( tmp_dir ) + 1 + 50 );

      if ( TemporaryFileName == NULL )
      {
        SetErrorMessage( "unable to allocate memory to temporay filename", strerror( errno ) );
        return false;
      }

      // Added PID of the prv2dim process in order to allow multiple
      // instances working together
      sprintf( TemporaryFileName, "%s/P%d_TmpTask_%04d_%04d_%06d", tmp_dir, getpid(), CurrentTask, CurrentThread, rand() % 999999 );

      if ( !DescriptorShared )
        TemporaryFile = fopen( TemporaryFileName, "a" );
      else
        TemporaryFile = NULL;

      if ( first_record_times[ CurrentTask * max_nthreads + CurrentThread ] == -1 )
        EmptyTask = true;
      else
        FirstRecordTime = first_record_times[ CurrentTask * max_nthreads + CurrentThread ];

      if ( DescriptorShared || ( TemporaryFile == NULL && ( errno == EMFILE || errno == ENFILE ) ) )
      {
        if ( !DescriptorShared )
          if ( !ShareDescriptor() )
            return false;

        NewTranslationInfo = new TaskTranslationInfo( TaskInfo[ CurrentTask ]->GetTaskId(),
                                                      CurrentThread + 1,
                                                      TimeFactor,
                                                      FirstRecordTime,
                                                      GenerateFirstIdle,
                                                      EmptyTask,
                                                      IprobeMissesThreshold,
                                                      TestMissesThreshold,
                                                      BurstCounterGeneration,
                                                      BurstCounterType,
                                                      BurstCounterFactor,
                                                      GenerateMPIInitBarrier,
                                                      PreviouslySimulatedTrace,
                                                      &TranslationInfo,
                                                      AcceleratorThread,
                                                      OpenMP_thread,
                                                      TemporaryFileName );
      }
      else if ( TemporaryFile == NULL )
      {
        char CurrentError[ 128 ];
        sprintf( CurrentError, "Error opening temporary file to task %02d", TaskInfo[ CurrentTask ]->GetTaskId() );
        SetErrorMessage( CurrentError, strerror( errno ) );
        return false;
      }
      else
      {
        NewTranslationInfo = new TaskTranslationInfo( TaskInfo[ CurrentTask ]->GetTaskId(),
                                                      CurrentThread + 1,
                                                      TimeFactor,
                                                      FirstRecordTime,
                                                      GenerateFirstIdle,
                                                      EmptyTask,
                                                      IprobeMissesThreshold,
                                                      TestMissesThreshold,
                                                      BurstCounterGeneration,
                                                      BurstCounterType,
                                                      BurstCounterFactor,
                                                      GenerateMPIInitBarrier,
                                                      PreviouslySimulatedTrace,
                                                      &TranslationInfo,
                                                      AcceleratorThread,
                                                      OpenMP_thread,
                                                      TemporaryFileName,
                                                      TemporaryFile );
      }

      TranslationInfo[ CurrentTask ][ CurrentThread ] = NewTranslationInfo;
      // delete FirstRecord;
    }
    if ( TaskThreadCount > 1 && !is_acc_task && !is_omp_task )
    { /* if task has an accelerator or openmp thread it's not a MultiThreadTrace error */
      MultiThreadTrace = true;
    }
    else if ( is_omp_task )
    {
      if ( debug )
        cout << " OpenMP TASK TRANSLATION IN TASK:" << CurrentTask << endl;
    }
    else if ( is_acc_task )
    {
      if ( debug )
        cout << "ACCELERATOR TASK TRANSLATION IN TASK:" << CurrentTask << endl;
    }
  }
  free( first_record_times );
  if ( debug )
    SHOW_PROGRESS_END( stdout, "CREATING TRANSLATION STRUCTURES ", TaskInfo.size() );
  return true;
}

bool ParaverTraceTranslator::TranslateCommunicators( ApplicationDescription_t AppDescription )
{
  unsigned int Index;
  vector<Communicator_t> Communicators;
  Communicator_t CurrentCommunicator;
  set<INT32> CommunicatorTasks;
  int* TaskIdList;


  Communicators = AppDescription->GetCommunicators();

  for ( Index = 0; Index < Communicators.size(); Index++ )
  {
    CurrentCommunicator = Communicators[ Index ];
    TaskIdList          = (int*)calloc( CurrentCommunicator->GetCommunicatorSize(), sizeof( int ) );

    CommunicatorTasks = CurrentCommunicator->GetCommunicatorTasks();

    set<INT32>::iterator it = CommunicatorTasks.begin();
    INT32 i                 = 0;
    while ( it != CommunicatorTasks.end() )
    {
      TaskIdList[ i ] = (int)( *it );

      if ( TaskIdList[ i ] != -1 )
        TaskIdList[ i ]--;

      ++it;
      ++i;
    }

    if ( Dimemas_Communicator_Definition( DimemasTraceFile,
                                          CurrentCommunicator->GetCommunicatorId(),
                                          CurrentCommunicator->GetCommunicatorSize(),
                                          TaskIdList ) < 0 )
    {
      char CurrentError[ 128 ];
      sprintf( CurrentError, "error writing communicator %02d", CurrentCommunicator->GetCommunicatorId() );
      SetErrorMessage( CurrentError, strerror( errno ) );

      free( TaskIdList );
      return false;
    }

    free( TaskIdList );
  }

  return true;
}

ParaverRecord_t ParaverTraceTranslator::SelectNextRecord( void )
{
  static ParaverRecord_t CurrentTraceRecord = NULL;
  static UINT32 CurrentCommunicationIndex   = 0;
  static bool LastRecordTrace               = true;

  PartialCommunication_t CurrentCommunication;
  Event_t PrvEventRecord;

  if ( LastRecordTrace )
  {
    /* Only events or global ops are needed */
    // CurrentTraceRecord = Parser->GetNextRecord(EVENT_REC | GLOBOP_REC);
    CurrentTraceRecord = Parser->GetNextRecord( STATE_REC | EVENT_REC | GLOBOP_REC );
  }

  if ( CurrentTraceRecord == NULL && Parser->GetError() )
  {
    SetError( true );
    SetErrorMessage( "Error reading trace record", Parser->GetLastError().c_str() );
    return NULL;
  }

  if ( CurrentTraceRecord == NULL )
  {
    if ( ( CurrentCommunicationIndex == Communications.size() ) )
      return NULL;
    else
    {
      LastRecordTrace      = false;
      CurrentCommunication = Communications[ CurrentCommunicationIndex ];
      CurrentCommunicationIndex++;
      return CurrentCommunication;
    }
  }

  if ( CurrentCommunicationIndex == Communications.size() )
  {
    LastRecordTrace = true;
    return CurrentTraceRecord;
  }

  /* Here, both kind of records are available */
  CurrentCommunication = Communications[ CurrentCommunicationIndex ];

  if ( CurrentTraceRecord->operator<( static_cast<ParaverRecord>( *CurrentCommunication ) ) )
  {
    LastRecordTrace = true;
    return CurrentTraceRecord;
  }

  if ( CurrentTraceRecord->operator>( static_cast<ParaverRecord>( *CurrentCommunication ) ) )
  {
    LastRecordTrace = false;
    CurrentCommunicationIndex++;
    return CurrentCommunication;
  }

  /* Both records are simultaneous */
  if ( CurrentTraceRecord->GetTaskId() != CurrentCommunication->GetTaskId() )
  { /* They come from different tasks. Communication is return */
    LastRecordTrace = false;
    CurrentCommunicationIndex++;
    return CurrentCommunication;
  }


  PrvEventRecord = dynamic_cast<Event_t>( CurrentTraceRecord );

  if ( PrvEventRecord != NULL ) /* Current trace record is an event */
  {
    if ( PrvEventRecord->GetTypeValueCount() >= 1 )
    {
      if ( PrvEventRecord->IsMPIBlockBegin() )
      {
        LastRecordTrace = true;
        return CurrentTraceRecord;
      }
      else if ( PrvEventRecord->IsMPIBlockEnd() )
      {
        LastRecordTrace = false;
        CurrentCommunicationIndex++;
        return CurrentCommunication;
      }
      else
      {
        LastRecordTrace = true;
        return CurrentTraceRecord;
      }
    }
    else
    { /* ¿Invalid? event record */
      LastRecordTrace = true;
      CurrentCommunicationIndex++;
      return CurrentCommunication;
    }
  }
  else /* Current trace record is a global op */
  {
    // LastRecordTrace = false;
    LastRecordTrace = true;
    return CurrentTraceRecord;
  }
}

TranslationCommunicator_t ParaverTraceTranslator::GetCommunicator( INT32 CommId )
{
  if ( Communicators.size() == 0 )
    return NULL;

  for ( UINT32 i = 0; i < Communicators.size(); i++ )
  {
    if ( Communicators[ i ]->GetCommunicatorId() == CommId )
      return Communicators[ i ];
  }

  return NULL;
}

bool ParaverTraceTranslator::ShareDescriptor( void )
{
  if ( DescriptorShared )
  {
    SetError( true );
    this->LastError = "Dimemas descriptor already share";
    return false;
  }
  else
  {
    if ( fclose( DimemasTraceFile ) != 0 )
    {
      SetError( true );
      SetErrorMessage( "Error closing Dimemas trace file to share descriptors", strerror( errno ) );
      return false;
    }
    DescriptorShared = true;

    cout << " (SHARING DESCRIPTORS)" << flush;

    return true;
  }
}

/**
 * Checks on .pcf file if it uses OpenMP or not!!!
 */
bool ParaverTraceTranslator::OpenMPTasksInfo( INT32 tasks_count )
{
  string PcfTraceName;
  string RowTraceName;
  string::size_type SubstrPosition;
  bool is_openmp_trace = FALSE;

  char* line = NULL;
  string Line;
  char* line1 = NULL;
  string Line1;
  size_t current_line_length = 0;
  INT32 task_id              = 0;

  string pattern_thread_lvl = "LEVEL THREAD SIZE";
  string pattern_thread_tag = "THREAD";
  SubstrPosition            = ParaverTraceName.rfind( ".prv" );

  if ( SubstrPosition == string::npos )
  {
    PcfTraceName = ParaverTraceName + ".pcf";
  }
  else
  {
    PcfTraceName = ParaverTraceName.substr( 0, SubstrPosition ) + ".pcf";
  }
  if ( ( PcfTraceFile = fopen( PcfTraceName.c_str(), "r" ) ) == NULL )
  {
    cout << "-> No input trace pcf file found" << endl;
    PcfTraceFile = NULL;
    return false;
  }
  if ( fseeko( PcfTraceFile, (size_t)0, SEEK_SET ) == -1 )
  {
    SetError( true );
    SetErrorMessage( "Error seeking position in pcf trace file", strerror( errno ) );
    return false;
  }

  omp_tasks_count = 0;
  while ( !is_openmp_trace && getline( &line, &current_line_length, PcfTraceFile ) != -1 )
  {
    if ( atoi( &line[ 2 ] ) == OMP_PARALLEL_EV )
    {
      omp_tasks.resize( tasks_count, true );
      omp_tasks_count = tasks_count;
      is_openmp_trace = TRUE;
    }
  }

  if ( !is_openmp_trace )
    omp_tasks.resize( tasks_count, false );

  return is_openmp_trace;
}
/**
 * Checks in .row file if there are CUDA or OpenCL devices and
 * gets the mapping info (task and thread)
 */
bool ParaverTraceTranslator::AcceleratorTasksInfo( INT32 tasks_count )
{
  string RowTraceName;
  string::size_type SubstrPosition;

  char* line = NULL;
  string Line;
  size_t current_line_length = 0;

  string pattern_thread_lvl = "LEVEL THREAD SIZE";
  string pattern_opencl_tag = "OpenCL";
  string pattern_cuda_tag   = "CUDA";

  INT32 task_id = -1;

  SubstrPosition = ParaverTraceName.rfind( ".prv" );

  if ( SubstrPosition == string::npos )
  {
    RowTraceName = ParaverTraceName + ".row";
  }
  else
  {
    RowTraceName = ParaverTraceName.substr( 0, SubstrPosition ) + ".row";
  }

  if ( ( RowTraceFile = fopen( RowTraceName.c_str(), "r" ) ) == NULL )
  {
    cout << "-> No input trace ROW file found" << endl;
    RowTraceFile = NULL;
    return false;
  }

  if ( fseeko( RowTraceFile, (size_t)0, SEEK_SET ) == -1 )
  {
    SetError( true );
    SetErrorMessage( "Error seeking position in row trace file", strerror( errno ) );
    return false;
  }

  acc_tasks.resize( tasks_count, false );
  acc_tasks_count = 0;

  while ( getline( &line, &current_line_length, RowTraceFile ) != -1 )
  {
    Line = (string)line;
    if ( Line.find( pattern_thread_lvl ) != std::string::npos )
    { /* Find Level Thread info line	*/
      while ( getline( &line, &current_line_length, RowTraceFile ) != -1 )
      { /* Application, task, thread info line	*/
        Line = (string)line;
        if ( Line.find( pattern_opencl_tag ) != std::string::npos || Line.find( pattern_cuda_tag ) != std::string::npos )
        {
          if ( task_id - 1 < tasks_count )
          {
            if ( !acc_tasks[ task_id - 1 ] )
            {
              acc_tasks[ task_id - 1 ] = true;
              acc_tasks_count++;
            }
          }
          else
          {
            SetError( true );
            SetErrorMessage( "Error in reading row line", strerror( errno ) );
            return false;
          }
        }
        else
        { /*	THREAD app_id.task_id.thread_id line	*/
          if ( atoi( &line[ 7 ] ) != 1 )
          { /*	App != 1, not be translated	*/
            if ( acc_tasks_count > 0 )
              return true;
            return false;
          }
          task_id = atoi( &line[ 9 ] ); // Task_id in 9th position
        }
      }
    }
  }
  if ( acc_tasks_count > 0 )
    return true;
  return false;
}
