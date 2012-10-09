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

#include "TaskTranslationInfo.hpp"

#include "Dimemas2Prv.h"
#include "EventEncoding.h"
#include "Dimemas_Generation.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <sys/stat.h>

#include <Macros.h>

#include <algorithm>
using std::sort;

#include <iostream>
using std::cout;

#include <sstream>
using std::ostringstream;

// #define DEBUG 1

/*****************************************************************************
 * Public functions
 ****************************************************************************/

TaskTranslationInfo::TaskTranslationInfo(INT32   TaskId,
                                         double  TimeFactor,
                                         UINT64  InitialTime,
                                         bool    GenerateFirstIdle,
                                         bool    EmptyTask,
                                         double  IprobeMissesThreshold,
                                         bool    BurstCounterGeneration,
                                         INT32   BurstCounterType,
                                         double  BurstCounterFactor,
                                         char*   TemporaryFileName,
                                         FILE*   TemporaryFile)
{
  const char* tmp_dir_default = "/tmp";
  char* tmp_dir;

  this->TaskId = TaskId;

  /* Burst Counter initialization */
  this->BurstCounterGeneration = BurstCounterGeneration;
  this->BurstCounterType       = BurstCounterType;
  this->BurstCounterFactor     = BurstCounterFactor;

  if (TemporaryFile == NULL)
  {
    FilePointerAvailable    = false;
    this->TemporaryFileName = TemporaryFileName;
    this->TemporaryFile     = NULL;
  }
  else
  {
    FilePointerAvailable = true;
    this->TemporaryFile     = TemporaryFile;
    this->TemporaryFileName = TemporaryFileName;
  }

  PendingGlobalOp       = false;
  this->TimeFactor      = TimeFactor;
  LastBlockEnd          = 0;
  FirstPrint            = false;
  FlushClusterStack     = false;
  // FirstPrint       = true;

  if (!FilePointerAvailable)
  {
    if ( (TemporaryFile = fopen(TemporaryFileName, "a")) == NULL)
    {
      SetError(true);
      SetErrorMessage("unable to open output temporary file",
                      strerror(errno));
    }
  }

#ifdef DEBUG
  cout << "Printing Initial CPU Burst [";

  cout.width(3);
  cout.fill('0');
  cout << TaskId << ":";

  cout.width(2);
  cout.fill('0');
  cout << 0 << "]";
  cout << endl;
#endif

  if (Dimemas_CPU_Burst(TemporaryFile,
                        TaskId-1, 0,
                        0.0) < 0)
  {
    SetError(true);
    SetErrorMessage("error writing output trace", strerror(errno));
  }

  this->GenerateFirstIdle = GenerateFirstIdle;

  if (!EmptyTask && GenerateFirstIdle)
  {

#ifdef DEBUG
    cout << "Printing Initial Idle Block Begin [";
    cout.width(3);
    cout.fill('0');
    cout << TaskId << ":";

    cout.width(2);
    cout.fill('0');
    cout << 0 << "]";
    cout << endl;
#endif

#ifdef NEW_DIMEMAS_TRACE
    if (Dimemas_Block_Begin(TemporaryFile,
                            TaskId-1, 0,
                            (INT64) 0, (INT64) 1) < 0)
#else
    if (Dimemas_Block_Begin(TemporaryFile,
                            TaskId-1, 0,
                            (INT64) BLOCK_ID_NULL) < 0)
#endif
    {
      SetError(true);
      SetErrorMessage("error writing output trace", strerror(errno));
    }

#ifdef DEBUG
    cout << "Printing Initial CPU burst [";
    cout.width(3);
    cout.fill('0');
    cout << TaskId << ":";

    cout.width(2);
    cout.fill('0');
    cout << 0 << "]";
    cout << endl;
#endif

    if (Dimemas_CPU_Burst(TemporaryFile,
                          TaskId-1, 0,
                          1.0*InitialTime*TimeFactor) < 0)
    {
      SetError(true);
      SetErrorMessage("error writing output trace", strerror(errno));
    }

#ifdef DEBUG
    cout << "Printing Initial CPU Block End [";
    cout.width(3);
    cout.fill('0');
    cout << TaskId << ":";

    cout.width(2);
    cout.fill('0');
    cout << 0 << "]";
    cout << endl;
#endif

    if (Dimemas_Block_End(TemporaryFile,
                          TaskId-1, 0,
                          (INT64) BLOCK_ID_NULL) < 0)
    {
      SetError(true);
      SetErrorMessage("error writing output trace", strerror(errno));
    }
    LastBlockEnd = InitialTime;
  }

  if (!FilePointerAvailable)
  {
    if (fclose(TemporaryFile) != 0)
    {
      SetError(true);
      SetErrorMessage("Error closing Dimemas trace file to share descriptors",
                      strerror(errno));
      return;
    }
  }

  /* DEBUG
  cout << "Initial Time = " << InitialTime << endl; */

  if (!EmptyTask && !GenerateFirstIdle)
  {
    LastBlockEnd = InitialTime;
  }

  this->IprobeMissesThreshold = IprobeMissesThreshold;

  OngoingIprobe      = false;
  IprobeBurstFlushed = false;
  OutsideComms       = false;
  WrongComms         = false;
  DisorderedRecords  = false;
  // RecordStack.empty();
}


TaskTranslationInfo::~TaskTranslationInfo(void)
{
  {
    unlink(TemporaryFileName);
    free(TemporaryFileName);
  }
  return;
}

bool
TaskTranslationInfo::PushRecord ( ParaverRecord_t Record )
{
  ParaverRecord_t LastRecord;
  Event_t         NewEvent;

  /* Just translate records of Thread 1 */
  if (Record->GetThreadId() != 1)
  {

    return true;
  }

  if (RecordStack.size() > 0)
  {
    LastRecord = RecordStack.back();

    /* Where we need to flush */
    if (LastRecord->operator<(*Record))
    {
      if (!ReorderAndFlush())
        return false;

      RecordStack.clear();
      RecordStack.push_back(Record);
    }
    else if (LastRecord->operator==(*Record))
    {
      RecordStack.push_back(Record);

      if ( (NewEvent = dynamic_cast<Event_t>(Record)) != NULL)
      { /* Check if there is block end */
        if (NewEvent->IsDimemasBlockEnd())
        {
          if (!ReorderAndFlush())
            return false;

          RecordStack.clear();
        }
      }
    }
    else if (LastRecord->operator>(*Record))
    {
      DisorderedRecords = true;
    }
  }
  else
  {
    RecordStack.push_back(Record);
  }
  return true;
}

bool
TaskTranslationInfo::LastFlush(void)
{
  bool result;

  result = ReorderAndFlush();

  if (!result)
    return false;
  else
  {
    if (FilePointerAvailable)
    {
      if (fclose(TemporaryFile) != 0)
      {
        SetError(true);
        SetErrorMessage("Error closing Dimemas trace file to share descriptors",
                        strerror(errno));
        return false;
      }
    }
  }

  return true;
}

bool
TaskTranslationInfo::Merge(FILE* DimemasFile)
{
  struct  stat FileStat;
  char         MergeMessage[128];
  off_t        TemporaryFileSize = 0, CurrentPosition;

  bool         SizeAvailable;
  INT32 CurrentPercentage = 0;
  INT32 PercentageRead    = 0;

  char   Buffer[1048576];
  size_t EffectiveBytes;

  if ( (TemporaryFile = fopen(TemporaryFileName, "r")) == NULL)
  {
    SetError(true);
    SetErrorMessage("unable to open output temporary file",
                      strerror(errno));
    return false;
  }

  if (fstat(fileno(TemporaryFile), &FileStat) < 0)
  {
    SizeAvailable = false;
  }
  else
  {
    SizeAvailable = true;
    TemporaryFileSize = FileStat.st_size;
    sprintf(MergeMessage, "   * Merging task %4d", this->TaskId);
  }

  fseek(TemporaryFile, 0, SEEK_SET);

  while(!feof(TemporaryFile))
  {
    EffectiveBytes = fread( Buffer,
                            sizeof(char),
                            sizeof(Buffer),
                            TemporaryFile);

    if (EffectiveBytes != sizeof(Buffer) && ferror(TemporaryFile))
    {
      LastError = strerror(errno);
      if (!FilePointerAvailable);
        fclose(TemporaryFile);
      return false;
    }

    if (fwrite(Buffer,
               sizeof(char),
               EffectiveBytes,
               DimemasFile) != EffectiveBytes)
    {
      LastError = strerror(errno);
      if (!FilePointerAvailable)
        fclose(TemporaryFile);
      return false;
    }

    if (SizeAvailable)
    {
      CurrentPosition = ftello(TemporaryFile);
      PercentageRead = lround(100.0*CurrentPosition/TemporaryFileSize);

      if (PercentageRead > CurrentPercentage)
      {
        CurrentPercentage = PercentageRead;
        SHOW_PROGRESS(stdout, MergeMessage, CurrentPercentage);
      }
    }
  }

  fclose(TemporaryFile);

  return true;
}

/*****************************************************************************
 * Private functions
 ****************************************************************************/

bool TaskTranslationInfo::ReorderAndFlush(void)
{
  UINT32 i;

  if (RecordStack.size() == 0)
    return true;

#ifdef DEBUG
  fprintf(stdout,"[%03d:%02d] BEFORE sort stack\n",TaskId,1);
  for (i = 0; i < RecordStack.size(); i++)
    cout << (*RecordStack[i]);
#endif

  /* There is a communication on the stack, we can flush it! */
  if (RecordStack.size() > 1)
  {
    if (MPIBlockIdStack.size() > 0)
    {
      sort(RecordStack.begin(), RecordStack.end(), InBlockComparison());
    }
    else
    {
      sort(RecordStack.begin(), RecordStack.end(), OutBlockComparison());
    }
  }

#ifdef DEBUG
  fprintf(stdout,"[%03d:%02d] AFTER sort stack\n",TaskId,1);
  for (i = 0; i < RecordStack.size(); i++)
    cout << (*RecordStack[i]);
#endif


  if (!FilePointerAvailable)
  {
    if ( (TemporaryFile = fopen(TemporaryFileName, "a")) == NULL)
    {
      char CurrentError[128];
      sprintf(CurrentError,
              "Error opening temporary file to task %02d",
              TaskId);
      SetErrorMessage(CurrentError, strerror(errno));
    }
  }

  for (i = 0; i < RecordStack.size(); i++)
  {
    ParaverRecord_t CurrentRecord;
    CurrentRecord = RecordStack[i];
    if (!ToDimemas(CurrentRecord))
    {
      if (!FilePointerAvailable)
        fclose(TemporaryFile);

      return false;
    }

    delete CurrentRecord;
  }

  /* TEST */
  if (PendingGlobalOp)
    FinalizeGlobalOp();

  if (!FilePointerAvailable)
    fclose(TemporaryFile);

  return true;
}

/* ToDimemas (GENERIC) ********************************************************/
bool TaskTranslationInfo::ToDimemas(ParaverRecord_t Record)
{
  Event_t                CurrentEvent;
  PartialCommunication_t CurrentComm;
  GlobalOp_t             CurrentGlobOp;
  bool                   InnerResult;

  /*
  if (FirstPrint)
  { // Must be a 0 timed CPU burst
    if (Dimemas_CPU_Burst(TemporaryFile,
                          Record->GetTaskId()-1, Record->GetThreadId()-1,
                          0) < 0)
    {
      SetError(true);
      SetErrorMessage("error writing output trace", strerror(errno));
      return false;
    }
    FirstPrint = false;
  }
  */

  if ( (CurrentEvent = dynamic_cast<Event_t>(Record)) != NULL)
    return ToDimemas(CurrentEvent);
  else if (
    (CurrentComm = dynamic_cast<PartialCommunication_t>(Record)) != NULL)
    return ToDimemas(CurrentComm);
  else if ( (CurrentGlobOp = dynamic_cast<GlobalOp_t>(Record)) != NULL)
    return ToDimemas(CurrentGlobOp);
  else
    return false;

  return true;
}

/* ToDimemas (EVENT) **********************************************************/
bool TaskTranslationInfo::ToDimemas(Event_t CurrentEvent)
{
  Block_t CurrentBlock;

  INT32  TaskId, ThreadId;
  INT32  Type;
  INT64  Value;
  UINT64 Timestamp;
  double OnTraceTime;

  /* We must ensure that current event only has one type/value pair */
  if (CurrentEvent->GetTypeValueCount() != 1)
    return false;

  TaskId    = CurrentEvent->GetTaskId()-1;
  ThreadId  = CurrentEvent->GetThreadId()-1;
  Type      = CurrentEvent->GetFirstType();
  Value     = CurrentEvent->GetFirstValue();
  Timestamp = CurrentEvent->GetTimestamp();

  if (PendingGlobalOp)
  {
    Event2GlobalOp(CurrentEvent);
  }

  if (Type == MPITYPE_COLLECTIVE && Value != 0)
  {

    DimBlock        CurrentBlock;
    DimCollectiveOp GlobalOpId;

    PendingGlobalOp = true;

    CurrentBlock = MPIEventEncoding_DimemasBlockId((MPIVal)Value);
    GlobalOpId   = MPIEventEncoding_GlobalOpId(CurrentBlock);

    PartialGlobalOp =
      new GlobalOp(Timestamp,
                   CurrentEvent->GetCPU(),
                   CurrentEvent->GetAppId(),
                   CurrentEvent->GetTaskId(),
                   CurrentEvent->GetThreadId(),
                   0, 0, 0, /* CommunicatorId, SendSize, RecvSize */
                   GlobalOpId,
                   false);

    GlobalOpFields  = 1;
  }

  if (!CheckIprobeCounters(CurrentEvent))
  {
    return false;
  }

  if (!MPIEventEncoding_Is_MPIBlock( (INT64) Type ) &&
      !MPIEventEncoding_Is_UserBlock( (INT64) Type ) &&
      !ClusterEventEncoding_Is_ClusterBlock( (INT64) Type))
  { /* It's a generic event! */


#ifdef DEBUG
    cout << "Printing User Event: " << *CurrentEvent;
#endif

    /* Test: if event appears in the middle of a CPU burst it breaks the
     * burst */
    // if (RecordStack.size() == 1)
    // {
      if (LastBlockEnd != Timestamp && MPIBlockIdStack.size() == 0)
      {
        if (Type != FLUSHING_EV &&
            Type != MPITYPE_PROBE_SOFTCOUNTER &&
            Type != MPITYPE_PROBE_TIMECOUNTER)
        {
          if (!GenerateBurst(TaskId, ThreadId, Timestamp))
          {
            return false;
          }

          LastBlockEnd = Timestamp;
        }
      }

    // }

    if (Dimemas_User_Event(TemporaryFile,
                           TaskId,
                           ThreadId,
                           (INT64) Type,
                           Value) < 0)
    {
      SetError(true);
      SetErrorMessage("error writing output trace", strerror(errno));
      return false;
    }

//    return true;
  }


  if ( ClusterEventEncoding_Is_ClusterBlock(Type) )
  {
    /* It's a cluster block */
    if (ClusterEventEncoding_Is_BlockBegin(Value))
    {
      /* It's a cluster block begin */
#ifndef NEW_DIMEMAS_TRACE
      CurrentBlock = ClusterEventEncoding_DimemasBlockId(Value);
#endif

      if( MPIBlockIdStack.size() == 0 )
      { /* No MPI call in proces */
#ifdef NEW_DIMEMAS_TRACE
        if (Dimemas_Block_Begin(TemporaryFile,
                                TaskId,
                                ThreadId,
                                (INT64) Type, (INT64)Value) < 0)
#else

        if (Dimemas_Block_Begin(TemporaryFile,
                                TaskId,
                                ThreadId,
                                (INT64) CurrentBlock) < 0)
#endif
        {
          SetError(true);
          SetErrorMessage("error writing output trace", strerror(errno));
          return false;
        }

        FlushClusterStack = false;
      }
      else
      {
        /* we can not put the record on trace yet */
        FlushClusterStack = true;
      }

      /* Block management */
#ifdef NEW_DIMEMAS_TRACE
      ClusterBlockIdStack.push_back(std::make_pair(Type,Value));
#else
      ClusterBlockIdStack.push_back(CurrentBlock);
#endif
      /* In order to generate burst at block end */
      LastBlockEnd = Timestamp;
    }
    else
    {
      /* It's a cluster block end */
      if (ClusterBlockIdStack.size() == 0)
      {
        cout << "WARNING: unbalanced cluster blocks on original trace" << endl;
        cout <<  "Task "<< TaskId + 1 << " Thread: " << ThreadId + 1 << " ";
        cout << "Time " << Timestamp << endl;
      }
      else
      {
        CurrentBlock = ClusterBlockIdStack.back();
        ClusterBlockIdStack.pop_back();

        /* Generate burst associated with this cluster */
        if(Timestamp > LastBlockEnd)
        {
          if (!GenerateBurst(TaskId, ThreadId, Timestamp))
          {
            return false;
          }

#ifdef NEW_DIMEMAS_TRACE
          if (Dimemas_Block_End(TemporaryFile,
                                TaskId,
                                ThreadId,
                                (INT64) Type) < 0)
#else
          if (Dimemas_Block_End(TemporaryFile,
                                TaskId,
                                ThreadId,
                                (INT64) CurrentBlock) < 0)
#endif
          {
            SetError(true);
            SetErrorMessage("error writing output trace", strerror(errno));
            return false;
          }

          LastBlockEnd = Timestamp;
        }
      }
    }
  }

  if (MPIEventEncoding_Is_MPIBlock(Type))
  {
    /* It's a MPI function */
    if (MPIEventEncoding_Is_BlockBegin(Value))
    {
      /* It's a Dimemas MPI block begin */

      /* Iprobe checks! */
      if ( OngoingIprobe )
      {
        OngoingIprobe = false;
      }

      MPIVal MPIValue = (MPIVal) Value;
#ifndef NEW_DIMEMAS_TRACE
      CurrentBlock = MPIEventEncoding_DimemasBlockId(MPIValue);

#ifdef DEBUG
      fprintf(
          stdout,
          "[%03d:%02d %lld] Opening Block %03d (%s)\n",
          CurrentEvent->GetTaskId(),
          CurrentEvent->GetThreadId(),
          CurrentEvent->GetTimestamp(),
          (INT64) CurrentBlock,
          MPIEventEncoding_GetBlockLabel((MPIVal)BLOCK_TRF2PRV_VALUE(CurrentBlock)));
#endif

#else
      CurrentBlock.first  = Type;
      CurrentBlock.second = Value;

#ifdef DEBUG
      fprintf(
          stdout,
          "[%03d:%02d %lld] Opening Block %03lld (%s)\n",
          CurrentEvent->GetTaskId(),
          CurrentEvent->GetThreadId(),
          CurrentEvent->GetTimestamp(),
          (INT64) CurrentBlock.second,
          MPIEventEncoding_GetBlockLabel((MPIVal)BLOCK_TRF2PRV_VALUE(CurrentBlock.second)));
#endif

#endif



      /* CPU Burst */
      if (Value != MPI_IPROBE_VAL || !IprobeBurstFlushed)
      {
        /*
        OnTraceTime = (double) 1.0*(Timestamp-LastBlockEnd)*TimeFactor;

        if (Dimemas_CPU_Burst(TemporaryFile,
                             TaskId, ThreadId,
                             OnTraceTime) < 0)
        {
          SetError(true);
          SetErrorMessage("error writing output trace", strerror(errno));
          return false;
        }
        */

        if(Timestamp > LastBlockEnd)
        {
          if (!GenerateBurst(TaskId, ThreadId, Timestamp))
          {
            return false;
          }
        }
      }
      IprobeBurstFlushed = false;

#ifdef DEBUG
      cout << "Printing Block Begin " << *CurrentEvent;
#endif

#ifdef NEW_DIMEMAS_TRACE
      if (Dimemas_Block_Begin(TemporaryFile,
                              TaskId,
                              ThreadId,
                              (INT64) Type, (INT64)Value) < 0)
#else
      if (Dimemas_Block_Begin(TemporaryFile,
                              TaskId,
                              ThreadId,
                              (INT64) CurrentBlock) < 0)
#endif
      {
        SetError(true);
        SetErrorMessage("error writing output trace", strerror(errno));
        return false;
      }

      /* Block management */
      MPIBlockIdStack.push_back(CurrentBlock);
      CommunicationPrimitivePrinted = false;

    }
    else
    {
      /* It's a Dimemas MPI block end */

      if (MPIBlockIdStack.size() == 0)
      {
        /* Relax this constraint. Unbalanced MPI blocks are not common
        MPIVal MPIValue = (MPIVal) Value;
        CurrentBlock    = MPIEventEncoding_DimemasBlockId(MPIValue);

        char CurrentError[128];

        PrintStack();

        sprintf(CurrentError,
                "unbalanced MPI event on original trace (task %02d, block %02d) ",
                TaskId,
                (long int) Value);

        SetError(true);
        // SetErrorMessage(CurrentError, "");
        this->LastError = CurrentError;

        return false;
        */
        return true;
      }

      // CurrentBlock = MPIEventEncoding_DimemasBlockId(BlockIdStack.back());
      CurrentBlock = MPIBlockIdStack.back();
      MPIBlockIdStack.pop_back();

#ifdef NEW_DIMEMAS_TRACE

#ifdef DEBUG
      fprintf(
        stdout,
        "[%03d:%02d %lld] Closing Block %03lld (%s)\n",
        CurrentEvent->GetTaskId(),
        CurrentEvent->GetThreadId(),
        CurrentEvent->GetTimestamp(),
        (INT64) CurrentBlock.second,
        MPIEventEncoding_GetBlockLabel( (MPIVal) CurrentBlock.second));
#endif

#else

#ifdef DEBUG
      fprintf(
        stdout,
        "[%03d:%02d %lld] Closing Block %03lld (%s)\n",
        CurrentEvent->GetTaskId(),
        CurrentEvent->GetThreadId(),
        CurrentEvent->GetTimestamp(),
        (INT32) CurrentBlock,
        MPIEventEncoding_GetBlockLabel((MPIVal)BLOCK_TRF2PRV_VALUE(CurrentBlock)));
#endif

#endif

      /* CurrentBlock = MPIEventEncoding_DimemasBlockId(MPIValue); */

      LastBlockEnd = Timestamp;

#ifdef DEBUG
      cout << "Printing Block End " << *CurrentEvent;
#endif

      if (!CommunicationPrimitivePrinted)
      {
        if (Dimemas_NOOP(TemporaryFile, TaskId, ThreadId) < 0)
        {
          SetError(true);
          SetErrorMessage("error writing output trace", strerror(errno));
          return false;
        }
      }

#ifdef NEW_DIMEMAS_TRACE
      if (Dimemas_Block_End(TemporaryFile,
                            TaskId,
                            ThreadId,
                            (INT64) Type) < 0)
#else
      if (Dimemas_Block_End(TemporaryFile,
                            TaskId,
                            ThreadId,
                            (INT64) CurrentBlock) < 0)
#endif
      {
        SetError(true);
        SetErrorMessage("error writing output trace", strerror(errno));
        return false;
      }

      /* If there are some block not flushed in the stack */

      if( FlushClusterStack )
      {
#ifdef NEW_DIMEMAS_TRACE
        if (Dimemas_Block_Begin(TemporaryFile,
                                TaskId,
                                ThreadId,
                                (INT64) ClusterBlockIdStack.back().first,
                                (INT64) ClusterBlockIdStack.back().second) < 0)
#else
        if (Dimemas_Block_Begin(TemporaryFile,
                                TaskId,
                                ThreadId,
                                (INT64) ClusterBlockIdStack.back()) < 0)
#endif
        {
          SetError(true);
          SetErrorMessage("error writing output trace", strerror(errno));
          return false;
        }
        FlushClusterStack = false;
      }

    }
  }

  if (MPIEventEncoding_Is_UserBlock(Type))
  { /* It's a User function */
    if (MPIEventEncoding_Is_BlockBegin(Value))
    { /* It's a Dimemas User block begin */

#ifndef NEW_DIMEMAS_TRACE
      CurrentBlock = (DimBlock) MPIEventEncoding_UserBlockId(Type, Value);
#endif

      /* Iprobe checks! */
      if ( OngoingIprobe )
      {
        OngoingIprobe = false;
      }

      /* Generate the CPU burst previous to this user-function */
      if(Timestamp > LastBlockEnd)
      {
        if (!GenerateBurst(TaskId, ThreadId, Timestamp))
        {
          return false;
        }
      }

      /* This 'trick' is done to ensure the generation of the next CPU burst */
      LastBlockEnd = Timestamp;

      #ifdef DEBUG
      cout << "Printing Block Begin " << *CurrentEvent;
      #endif

      /* Print the Block Begin */
#ifdef NEW_DIMEMAS_TRACE
      if (Dimemas_Block_Begin(TemporaryFile,
                              TaskId,
                              ThreadId,
                              (INT64) Type, (INT64)Value) < 0)
#else
      if (Dimemas_Block_Begin(TemporaryFile,
                              TaskId,
                              ThreadId,
                              CurrentBlock) < 0)
#endif
      {
        SetError(true);
        SetErrorMessage("error writing output trace", strerror(errno));
        return false;
      }

      /* Block management */
#ifdef NEW_DIMEMAS_TRACE
      UserBlockIdStack.push_back(std::make_pair(Type, Value));
#else
      UserBlockIdStack.push_back(CurrentBlock);
#endif
    }
    else
    { /* It's a Dimemas User block end */
      if (UserBlockIdStack.size() == 0)
      {
        /* Relax this constraint. Unbalanced blocks are not common */
        return true;
      }

      /* Get last block */
      CurrentBlock = UserBlockIdStack.back();
      UserBlockIdStack.pop_back();

      /* Generate the CPU burst up to this user-function end */
      if(Timestamp > LastBlockEnd)
      {
        if (!GenerateBurst(TaskId, ThreadId, Timestamp))
        {
          return false;
        }
      }

      /* This 'trick' is done to ensure the generation of the next CPU burst */
      LastBlockEnd = Timestamp;

      #ifdef DEBUG
      cout << "Printing Block End " << *CurrentEvent;
      #endif

      /* Print the Block Begin */
#ifdef NEW_DIMEMAS_TRACE
      if (Dimemas_Block_End(TemporaryFile,
                            TaskId,
                            ThreadId,
                            Type) < 0)
#else
      if (Dimemas_Block_End(TemporaryFile,
                            TaskId,
                            ThreadId,
                            CurrentBlock) < 0)
#endif
      {
        SetError(true);
        SetErrorMessage("error writing output trace", strerror(errno));
        return false;
      }
    }
  }

  return true;
}


/* ToDimemas (COMMUNICATION) **************************************************/
bool TaskTranslationInfo::ToDimemas(PartialCommunication_t CurrentComm)
{
  Block_t CurrentBlock;
  INT64   CurrentBlockValue;
  INT32   TaskId, ThreadId, PartnerTaskId, CommId, Size, Tag;

  if (MPIBlockIdStack.size() == 0)
  { /* Error! Communications must be inside a block! */

    /* Communications outside a block only emit a Warning

    cout << "WARNING: Communication outside a block on Task ";
    cout << CurrentComm->GetTaskId() << "(TimeStamp = " << CurrentComm->GetTimestamp() << ")";
    cout << ". The simulation of this trace could be inconsistent" << endl;

    LastError = "Communication outside a block";
    return false;
    */

    OutsideComms = true;
    return true;
  }

#ifdef NEW_DIMEMAS_TRACE
  CurrentBlock      = MPIBlockIdStack.back();
  CurrentBlockValue = MPIEventEncoding_DimemasBlockId((MPIVal) CurrentBlock.second);
#else
  CurrentBlockValue = MPIBlockIdStack.back();
#endif

  TaskId        = CurrentComm->GetTaskId()-1;
  ThreadId      = CurrentComm->GetThreadId()-1;
  PartnerTaskId = CurrentComm->GetPartnerTaskId()-1;
  CommId        = CurrentComm->GetCommId();
  Size          = CurrentComm->GetSize();
  Tag           = CurrentComm->GetTag();

  switch(CurrentBlockValue)
  {
    case BLOCK_ID_MPI_Recv:
      /* DEBUG
      fprintf(stdout, "MPI_RECV\n");
      */
      if (CurrentComm->GetType() == LOGICAL_RECV)
      {
#ifdef DEBUG
        cout << "Printing NX Recv " << *CurrentComm;
#endif
        CommunicationPrimitivePrinted = true;

        if (Dimemas_NX_Recv(TemporaryFile,
                            TaskId, ThreadId,
                            PartnerTaskId,
                            CommId, Size, (INT64) Tag) < 0)
        {
          SetError(true);
          SetErrorMessage("error writing output trace", strerror(errno));
          return false;
        }
      }
      else if (CurrentComm->GetType() != PHYSICAL_RECV)
      {
        WrongComms = true;
      }
      break;
    case BLOCK_ID_MPI_Send:
    case BLOCK_ID_MPI_Ssend:
      /* DEBUG
      fprintf(stdout, "MPI_SSEND\n");
      */
      if (CurrentComm->GetType() == LOGICAL_SEND)
      {
#ifdef DEBUG
        cout << "Printing NX BSend " << *CurrentComm;
#endif
        CommunicationPrimitivePrinted = true;

        if (Dimemas_NX_BlockingSend(TemporaryFile,
                                    TaskId, ThreadId,
                                    PartnerTaskId,
                                    CommId, Size, (INT64) Tag) < 0)
        {
          SetError(true);
          SetErrorMessage("error writing output trace", strerror(errno));
          return false;
        }
      }
      else if (CurrentComm->GetType() != PHYSICAL_SEND)
      {
        WrongComms = true;
      }
      break;
    case BLOCK_ID_MPI_Bsend:
    case BLOCK_ID_MPI_Isend:
    case BLOCK_ID_MPI_Issend:
      /* DEBUG
      fprintf(stdout, "MPI_ISEND\n");
      */
      if (CurrentComm->GetType() == LOGICAL_SEND)
      {
#ifdef DEBUG
        cout << "Printing NX ISend " << *CurrentComm;
#endif
        CommunicationPrimitivePrinted = true;

        if (Dimemas_NX_ImmediateSend(TemporaryFile,
                                     TaskId, ThreadId,
                                     PartnerTaskId,
                                     CommId, Size, (INT64) Tag) < 0)
        {
          SetError(true);
          SetErrorMessage("error writing output trace", strerror(errno));
          return false;
        }
      }
      else if (CurrentComm->GetType() != PHYSICAL_SEND)
      {
        WrongComms = true;
      }
      break;
    case BLOCK_ID_MPI_Irecv:
      /* DEBUG
      fprintf(stdout, "MPI_IRECV\n");
      */
      if (CurrentComm->GetType() == LOGICAL_RECV)
      {
#ifdef DEBUG
        cout << "Printing NX IRecv " << *CurrentComm;
#endif
        CommunicationPrimitivePrinted = true;

        if (Dimemas_NX_Irecv(TemporaryFile,
                             TaskId, ThreadId,
                             PartnerTaskId,
                             CommId, Size, (INT64) Tag) < 0)
        {
          SetError(true);
          SetErrorMessage("error writing output trace", strerror(errno));
          return false;
        }
      }
      else if (CurrentComm->GetType() != PHYSICAL_RECV)
      {
        WrongComms = true;
      }
      break;
    case BLOCK_ID_MPI_Waitany:
    case BLOCK_ID_MPI_Wait:
    case BLOCK_ID_MPI_Waitall:
    case BLOCK_ID_MPI_Waitsome:
    case BLOCK_ID_MPI_Test:

      /* DEBUG
      fprintf(stdout, "MPI_WAIT\n");
      */
      if (CurrentComm->GetType() == PHYSICAL_RECV)
      {
#ifdef DEBUG
        cout << "Printing NX Wait " << *CurrentComm;
#endif
        CommunicationPrimitivePrinted = true;

        if (Dimemas_NX_Wait(TemporaryFile,
                            TaskId, ThreadId,
                            PartnerTaskId,
                            CommId, Size, (INT64) Tag) < 0)
        {
          SetError(true);
          SetErrorMessage("error writing output trace", strerror(errno));
          return false;
        }
      }
      else if (CurrentComm->GetType() != LOGICAL_RECV)
      {
        WrongComms = true;
      }
      break;
    case BLOCK_ID_MPI_Sendrecv:
    case BLOCK_ID_MPI_Sendrecv_replace:
      /* DEBUG
      fprintf(stdout, "MPI_SENDRECV\n");
      */
      if (CurrentComm->GetType() == LOGICAL_RECV)
      {
#ifdef DEBUG
        cout << "Printing NX IRecv " << *CurrentComm;
#endif
        CommunicationPrimitivePrinted = true;

        if (Dimemas_NX_Irecv(TemporaryFile,
                             TaskId, ThreadId,
                             PartnerTaskId,
                             CommId, Size, (INT64) Tag) < 0)
        {
          SetError(true);
          SetErrorMessage("error writing output trace", strerror(errno));
          return false;
        }
      }
      else if (CurrentComm->GetType() == LOGICAL_SEND)
      {
#ifdef DEBUG
        cout << "Printing NX Send " << *CurrentComm;
#endif
        CommunicationPrimitivePrinted = true;

        if (Dimemas_NX_Send(TemporaryFile,
                            TaskId, ThreadId,
                            PartnerTaskId,
                            CommId, Size, (INT64) Tag) < 0)
        {
          SetError(true);
          SetErrorMessage("error writing output trace", strerror(errno));
          return false;
        }
      }
      else if (CurrentComm->GetType() == PHYSICAL_RECV)
      {
#ifdef DEBUG
        cout << "Printing NX Wait " << *CurrentComm;
#endif
        CommunicationPrimitivePrinted = true;

        if (Dimemas_NX_Wait(TemporaryFile,
                            TaskId, ThreadId,
                            PartnerTaskId,
                            CommId, Size, (INT64) Tag) < 0)
        {
          SetError(true);
          SetErrorMessage("error writing output trace", "");
          return false;
        }
      }
      break;
    default:
#ifdef NEW_DIMEMAS_TRACE
      MPIVal CurrentBlockMPIVal = (MPIVal) CurrentBlock.second;
#else
      MPIVal CurrentBlockMPIVal =
        (MPIVal) BLOCK_TRF2PRV_VALUE(CurrentBlockValue);
#endif

      fprintf(
        stdout,
        "[%03d:%02d %lld] Communication wrapped with unknown communication call (%d:%s)\n",
        CurrentComm->GetTaskId(),
        CurrentComm->GetThreadId(),
        CurrentComm->GetTimestamp(),
        (INT32) CurrentBlockValue,
        MPIEventEncoding_GetBlockLabel(CurrentBlockMPIVal));
      break;
  }

  /*
  fprintf(stdout, "Communication printed!!\n");
  */

  return true;
}

/* ToDimemas (GLOBAL_OP) ******************************************************/
bool
TaskTranslationInfo::ToDimemas(GlobalOp_t CurrentGlobOp)
{
  INT32 RootTaskId;

  /*
  if (CurrentGlobOp->GetRootTaksId() == -1)
    RootTaskId = 0;
  else
    RootTaskId = CurrentGlobOp->GetTaskId()-1;
  */

  if (CurrentGlobOp->GetIsRoot())
  {
    RootTaskId = 1;
  }
  else
  {
    RootTaskId = 0;
  }

#ifdef DEBUG
  cout << "Printing GlobalOP " << *CurrentGlobOp;
#endif
  if (Dimemas_Global_OP(TemporaryFile,
                        CurrentGlobOp->GetTaskId()-1,
                        CurrentGlobOp->GetThreadId()-1,
                        CurrentGlobOp->GetGlobalOpId(),
                        CurrentGlobOp->GetCommunicatorId(),
                        RootTaskId, 0,
                        CurrentGlobOp->GetSendSize(),
                        CurrentGlobOp->GetRecvSize()) < 0)
  {
    SetError(true);
    SetErrorMessage("error writing output trace", strerror(errno));
    return false;
  }

  /* DEBUG
  fprintf(stdout, "GlobalOP printed!!\n");
  */
  return true;
}


/* Event2GlobalOp *************************************************************/
void TaskTranslationInfo::Event2GlobalOp(Event_t CurrentEvent)
{
  bool  NoMoreEvents = false;

#ifdef DEBUG
  fprintf(
        stdout,
        "[%03d:%02d %lld] Event to global translation: ",
        CurrentEvent->GetTaskId(),
        CurrentEvent->GetThreadId(),
        CurrentEvent->GetTimestamp());
#endif

  switch (CurrentEvent->GetFirstType())
  {
    case MPI_GLOBAL_OP_SENDSIZE:
      PartialGlobalOp->SetSendSize( (INT32) CurrentEvent->GetFirstValue() );
      GlobalOpFields++;

#ifdef DEBUG
      fprintf(stdout, "SEND SIZE = %lld\n",CurrentEvent->GetFirstValue());
#endif
      break;
    case MPI_GLOBAL_OP_RECVSIZE:
      PartialGlobalOp->SetRecvSize( (INT32) CurrentEvent->GetFirstValue() );
      GlobalOpFields++;

#ifdef DEBUG
      fprintf(stdout, "RECEIVE SIZE = %lld\n",CurrentEvent->GetFirstValue());
#endif
      break;
    case MPI_GLOBAL_OP_ROOT:
      if (CurrentEvent->GetFirstValue() == 1)
      {
        PartialGlobalOp->SetIsRoot(true);
        // PartialGlobalOp->SetRootTaskId(CurrentEvent->GetTaskId());
      }

      GlobalOpFields++;
#ifdef DEBUG
      fprintf(stdout, "ROOT\n",CurrentEvent->GetFirstValue());
#endif
      break;
    case MPI_GLOBAL_OP_COMM:
      // DEBUG
      // cout << "Communicator in Global OP = " << CurrentEvent->GetFirstValue() << endl;
      PartialGlobalOp->SetCommunicatorId( (INT32) CurrentEvent->GetFirstValue() );
      GlobalOpFields++;

#ifdef DEBUG
      fprintf(stdout, "COMMUNICATOR = %lld\n",CurrentEvent->GetFirstValue());
#endif
      break;
    default:
      /* TEST */

#ifdef DEBUG
      fprintf(stdout, "NOT A GLOBAL OP EVENT (%d)\n",CurrentEvent->GetFirstType());
#endif

      return;
      //NoMoreEvents = true;
      break;
  }

  if (NoMoreEvents)
  {
    /* DEBUG */
    printf("FINALIZING GlobalOp Fields = %d\n", GlobalOpFields);

    /* if (GlobalOpFields == 4 || GlobalOpFields == 5) */
    if (GlobalOpFields >= 2)
    {
      if (CurrentEvent->GetFirstType() == MPITYPE_COLLECTIVE)
      {
        ToDimemas(PartialGlobalOp);
      }
      else
      {
        RecordStack.push_back(PartialGlobalOp);
      }

      /* PushRecord(PartialGlobalOp); */
      /* ToDimemas(PartialGlobalOp); */
      GlobalOpFields  = 0;
      PendingGlobalOp = false;
    }
    else
    {
      delete PartialGlobalOp;
      PendingGlobalOp = false;
    }
  }
}

void TaskTranslationInfo::FinalizeGlobalOp(void)
{
  /* if (GlobalOpFields == 4 || GlobalOpFields == 5) */

#ifdef DEBUG
  fprintf(
        stdout,
        "[%03d:%02d %lld] Finalizing global \n",
        PartialGlobalOp->GetTaskId(),
        PartialGlobalOp->GetThreadId(),
        PartialGlobalOp->GetTimestamp());
#endif

  if (GlobalOpFields >= 2)
  {
    ToDimemas(PartialGlobalOp);

    /* PushRecord(PartialGlobalOp); */
    /* ToDimemas(PartialGlobalOp); */
    GlobalOpFields  = 0;
    PendingGlobalOp = false;
  }
  else
  {
    delete PartialGlobalOp;
    PendingGlobalOp = false;
  }
}

/* CheckIprobeCounters ********************************************************/
bool TaskTranslationInfo::CheckIprobeCounters(Event_t CurrentEvent)
{

  INT32  TaskId, ThreadId;
  INT32  Type;
  INT64  Value;
  UINT64 Timestamp;
  double OnTraceTime;

  TaskId    = CurrentEvent->GetTaskId()-1;
  ThreadId  = CurrentEvent->GetThreadId()-1;
  Type      = CurrentEvent->GetFirstType();
  Value     = CurrentEvent->GetFirstValue();
  Timestamp = CurrentEvent->GetTimestamp();

  if ( Type  == MPITYPE_PROBE_SOFTCOUNTER && Value == 0)
  {
    /* CPU Burst */
    if (!GenerateBurst(TaskId, ThreadId, Timestamp))
      return false;
    /*
    OnTraceTime = (double) 1.0*(Timestamp-LastBlockEnd)*TimeFactor;

    if (Dimemas_CPU_Burst(TemporaryFile,
                          TaskId, ThreadId,
                          OnTraceTime) < 0)
    {
      SetError(true);
      SetErrorMessage("error writing output trace", strerror(errno));
      return false;
    }
    */

    LastBlockEnd          = Timestamp;
    OngoingIprobe         = true;
  }

  if ((Type == MPITYPE_PROBE_SOFTCOUNTER || Type == MPITYPE_PROBE_TIMECOUNTER)
       && Value != 0)
  {
    if (OngoingIprobe)
    {
      if (Type == MPITYPE_PROBE_TIMECOUNTER && IprobeMissesThreshold == 0.0)
      {
        /* if (Value < TimeCounterThreshold) */
#ifdef DEBUG
        cout << "Printing CPU Burst [";
        cout.width(3);
        cout.fill('0');
        cout << TaskId+1 << ":";

        cout.width(2);
        cout.fill('0');
        cout << ThreadId+1 << "]:" << Value/TimeFactor;
        cout << endl;
#endif
        if (Dimemas_CPU_Burst(TemporaryFile,
                              TaskId, ThreadId,
                              Value/TimeFactor) < 0)
        {
          SetError(true);
          SetErrorMessage("error writing output trace", strerror(errno));
          return false;
        }

        OngoingIprobe      = false;
        IprobeBurstFlushed = true;
        LastBlockEnd       = Timestamp;
      }
      else if (Type == MPITYPE_PROBE_SOFTCOUNTER)
      {
        /* CPU Burst */
        double IprobeAreaDuration = Timestamp - LastBlockEnd;
        double ActualIprobeRate = (Value / IprobeAreaDuration);

        if (TimeFactor == 1e-9)
        {
          ActualIprobeRate *= 1e6;
        }
        else
        {
          ActualIprobeRate *= 1e3;
        }

        if (IprobeMissesThreshold == 0.0 || ActualIprobeRate < IprobeMissesThreshold)
        {
          OnTraceTime = (double) 1.0*(Timestamp-LastBlockEnd)*TimeFactor;
        }
        else
        {
          /* Ereased burst! */
          OnTraceTime = 0.0;
        }

#ifdef DEBUG
        cout << "Printing CPU Burst [";
        cout.width(3);
        cout.fill('0');
        cout << TaskId+1 << ":";

        cout.width(2);
        cout.fill('0');
        cout << ThreadId+1 << "]:" << OnTraceTime;
        cout << endl;
#endif
        if (Dimemas_CPU_Burst(TemporaryFile,
                              TaskId, ThreadId,
                              OnTraceTime) < 0)
        {
          SetError(true);
          SetErrorMessage("error writing output trace", strerror(errno));
          return false;
        }

        OngoingIprobe      = false;
        IprobeBurstFlushed = true;
        LastBlockEnd       = Timestamp;
      }
    }
  }
  return true;
}
/* GenerateBurst **************************************************************/
bool TaskTranslationInfo::GenerateBurst(INT32  TaskId,
                                        INT32  ThreadId,
                                        UINT64 Timestamp)
{
  double OnTraceTime = 0.0;
  bool   found = false;

  if (FirstPrint || !BurstCounterGeneration)
  {
    if (FirstPrint)
      FirstPrint = false;

    OnTraceTime = (double) 1.0*(Timestamp-LastBlockEnd)*TimeFactor;
  }
  else
  { /* Burst duration computation based on counter value and factor */
    UINT32  i;
    Event_t CurrentEvent = NULL;

    for (i = 0; i < RecordStack.size(); i++)
    {
      CurrentEvent = dynamic_cast<Event_t>(RecordStack[i]);

      if (CurrentEvent == NULL)
        continue;

      if (CurrentEvent->GetFirstType() == BurstCounterType)
      {
        OnTraceTime = 1.0*CurrentEvent->GetFirstValue()*BurstCounterFactor;

        /*
        printf("Factor: %.9f Evt. Val: %ld Time: %.6f\n",
               BurstCounterFactor,
               (long int) CurrentEvent->GetFirstValue(),
               OnTraceTime );
        */

        found = true;
        break;
      }
    }

    if (!found)
    { /* If no counter found, burst duration is computed with the 'classical
         method' */
      OnTraceTime = (double) 1.0*(Timestamp-LastBlockEnd)*TimeFactor;
    }
  }

#ifdef DEBUG
  cout << "Printing CPU Burst [";
  cout.width(3);
  cout.fill('0');
  cout << TaskId+1 << ":";

  cout.width(2);
  cout.fill('0');
  cout << ThreadId+1 << "]:" << OnTraceTime;
  cout << endl;
#endif

  if (Dimemas_CPU_Burst(TemporaryFile,
                        TaskId, ThreadId,
                        OnTraceTime) < 0)
  {
    SetError(true);
    SetErrorMessage("error writing output trace", strerror(errno));
    return false;
  }

  return true;
}

void TaskTranslationInfo::PrintStack(void)
{
  int i;
  cout << "Record Stack for Task " << TaskId << endl;
  for (i = 0; i < RecordStack.size(); i++)
  {
    cout << *(RecordStack[i]);
  }
  return;
}

